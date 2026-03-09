#!/usr/bin/env python3
"""
check_scicall.py - Diff Scintilla.iface against SciCall.h

Parses Scintilla.iface to find all fun/get/set messages and compares
against SciCall.h to find:
  1. iface messages NOT wrapped in SciCall.h (candidates to add)
  2. SciCall.h wrappers that don't match any iface message (stale/custom)

Usage:
    python Build/scripts/check_scicall.py
    python Build/scripts/check_scicall.py --verbose
    python Build/scripts/check_scicall.py --category Basics
    python Build/scripts/check_scicall.py --generate
"""

import argparse
import os
import re
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.normpath(os.path.join(SCRIPT_DIR, "..", ".."))

IFACE_PATH = os.path.join(REPO_ROOT, "scintilla", "include", "Scintilla.iface")
SCICALL_PATH = os.path.join(REPO_ROOT, "src", "SciCall.h")

# iface type -> NP3 C type mapping
TYPE_MAP = {
    "void": "void",
    "position": "DocPos",
    "line": "DocLn",
    "colour": "COLORREF",
    "colouralpha": "COLORALPHAREF",
    "bool": "bool",
    "int": "int",
    "string": "const char*",
    "stringresult": "char*",
    "pointer": "sptr_t",
    "cells": "const char*",
    "textrange": "struct Sci_TextRange*",
    "textrangefull": "struct Sci_TextRangeFull*",
    "findtext": "struct Sci_TextToFind*",
    "findtextfull": "struct Sci_TextToFindFull*",
    "formatrange": "struct Sci_RangeToFormat*",
    "formatrangefull": "struct Sci_RangeToFormatFull*",
    "keymod": "size_t",
}


def map_type(iface_type):
    """Map an iface type to an NP3 C type."""
    if iface_type in TYPE_MAP:
        return TYPE_MAP[iface_type]
    # Capitalized enum types -> int
    if iface_type and iface_type[0].isupper():
        return "int"
    return "int"


def parse_iface(path):
    """Parse Scintilla.iface, returning a dict of uppercase_name -> info."""
    messages = {}
    current_category = "Unknown"

    # Match: fun/get/set <rettype> <Name>=<number>(<params>)
    msg_re = re.compile(
        r"^(fun|get|set)\s+"       # feature type
        r"(\S+)\s+"                # return type
        r"(\w+)"                   # function name
        r"=(\d+)"                  # message number
        r"\(([^)]*)\)"             # parameters
    )
    cat_re = re.compile(r"^cat\s+(\w+)")

    with open(path, "r", encoding="utf-8") as f:
        for line_no, line in enumerate(f, 1):
            line = line.rstrip()

            cat_m = cat_re.match(line)
            if cat_m:
                current_category = cat_m.group(1)
                continue

            if line.startswith("##") or line.startswith("#!"):
                continue

            msg_m = msg_re.match(line)
            if msg_m:
                feat_type = msg_m.group(1)
                ret_type = msg_m.group(2)
                name = msg_m.group(3)
                msg_num = int(msg_m.group(4))
                params_str = msg_m.group(5).strip()
                upper_name = name.upper()

                messages[upper_name] = {
                    "name": name,
                    "upper": upper_name,
                    "type": feat_type,
                    "ret": ret_type,
                    "num": msg_num,
                    "params": params_str,
                    "category": current_category,
                    "line": line_no,
                    "raw": line,
                }

    return messages


def parse_scicall(path):
    """Parse SciCall.h, returning a dict of uppercase_msg -> info."""
    wrappers = {}

    # Match: DeclareSciCall{R,V}{0,01,1,2}(fn, MSG, ...)
    decl_re = re.compile(
        r"DeclareSciCall([RV])(0|01|1|2)\s*\(\s*(\w+)\s*,\s*(\w+)"
    )
    # Also match commented-out declarations
    commented_re = re.compile(
        r"//~?\s*DeclareSciCall([RV])(0|01|1|2)\s*\(\s*(\w+)\s*,\s*(\w+)"
    )

    with open(path, "r", encoding="utf-8") as f:
        for line_no, line in enumerate(f, 1):
            line_stripped = line.rstrip()

            # Skip macro definitions (#define DeclareSciCall...)
            if line_stripped.startswith("#define"):
                continue

            # Check for commented-out declarations (track but mark)
            cm = commented_re.match(line_stripped)
            if cm:
                upper_msg = cm.group(4)
                wrappers[upper_msg] = {
                    "fn": cm.group(3),
                    "msg": upper_msg,
                    "macro_ret": cm.group(1),
                    "macro_params": cm.group(2),
                    "line": line_no,
                    "commented": True,
                    "raw": line_stripped,
                }
                continue

            dm = decl_re.search(line_stripped)
            if dm:
                upper_msg = dm.group(4)
                wrappers[upper_msg] = {
                    "fn": dm.group(3),
                    "msg": upper_msg,
                    "macro_ret": dm.group(1),
                    "macro_params": dm.group(2),
                    "line": line_no,
                    "commented": False,
                    "raw": line_stripped,
                }

    return wrappers


def parse_param(param_str):
    """Parse a single iface param like 'position pos' or '' into (type, name)."""
    param_str = param_str.strip()
    if not param_str:
        return None, None
    # Handle default values like 'int defaultValue'
    parts = param_str.split()
    if len(parts) >= 2:
        return parts[0], parts[1].split("=")[0]
    elif len(parts) == 1:
        return parts[0], "param"
    return None, None


def generate_wrapper(msg):
    """Generate a DeclareSciCall* line for an iface message."""
    name = msg["name"]
    upper = msg["upper"]
    ret_type = msg["ret"]
    params_str = msg["params"]

    # Parse return type
    c_ret = map_type(ret_type)
    is_void = (ret_type == "void")

    # Parse parameters
    if "," in params_str:
        wp_str, lp_str = params_str.split(",", 1)
    else:
        wp_str = params_str
        lp_str = ""

    wp_type, wp_name = parse_param(wp_str)
    lp_type, lp_name = parse_param(lp_str)

    has_wp = wp_type is not None
    has_lp = lp_type is not None

    # Determine macro variant
    rv = "V" if is_void else "R"

    if has_wp and has_lp:
        variant = "2"
    elif has_wp and not has_lp:
        variant = "1"
    elif not has_wp and has_lp:
        variant = "01"
    else:
        variant = "0"

    macro = f"DeclareSciCall{rv}{variant}"

    # Build arguments
    if variant == "0":
        if is_void:
            return f"{macro}({name}, {upper});"
        else:
            return f"{macro}({name}, {upper}, {c_ret});"
    elif variant == "1":
        c_wp = map_type(wp_type)
        if is_void:
            return f"{macro}({name}, {upper}, {c_wp}, {wp_name});"
        else:
            return f"{macro}({name}, {upper}, {c_ret}, {c_wp}, {wp_name});"
    elif variant == "01":
        c_lp = map_type(lp_type)
        if is_void:
            return f"{macro}({name}, {upper}, {c_lp}, {lp_name});"
        else:
            return f"{macro}({name}, {upper}, {c_ret}, {c_lp}, {lp_name});"
    elif variant == "2":
        c_wp = map_type(wp_type)
        c_lp = map_type(lp_type)
        if is_void:
            return f"{macro}({name}, {upper}, {c_wp}, {wp_name}, {c_lp}, {lp_name});"
        else:
            return f"{macro}({name}, {upper}, {c_ret}, {c_wp}, {wp_name}, {c_lp}, {lp_name});"

    return f"// TODO: {name}"


def main():
    parser = argparse.ArgumentParser(
        description="Diff Scintilla.iface against SciCall.h to find unwrapped messages"
    )
    parser.add_argument(
        "--verbose", "-v", action="store_true",
        help="Show detailed info for each unwrapped message"
    )
    parser.add_argument(
        "--category", "-c", type=str, default=None,
        help="Filter to a specific iface category (e.g. Basics, Provisional, Deprecated)"
    )
    parser.add_argument(
        "--show-wrapped", "-w", action="store_true",
        help="Also list messages that ARE wrapped (for completeness check)"
    )
    parser.add_argument(
        "--generate", "-g", action="store_true",
        help="Generate DeclareSciCall* lines for all unwrapped non-deprecated messages"
    )
    parser.add_argument(
        "--iface", type=str, default=IFACE_PATH,
        help=f"Path to Scintilla.iface (default: {IFACE_PATH})"
    )
    parser.add_argument(
        "--scicall", type=str, default=SCICALL_PATH,
        help=f"Path to SciCall.h (default: {SCICALL_PATH})"
    )
    args = parser.parse_args()

    if not os.path.isfile(args.iface):
        print(f"Error: {args.iface} not found", file=sys.stderr)
        sys.exit(1)
    if not os.path.isfile(args.scicall):
        print(f"Error: {args.scicall} not found", file=sys.stderr)
        sys.exit(1)

    all_iface_msgs = parse_iface(args.iface)
    scicall_wrappers = parse_scicall(args.scicall)

    # Apply category filter for unwrapped/wrapped analysis
    if args.category:
        iface_msgs = {
            k: v for k, v in all_iface_msgs.items()
            if v["category"].lower() == args.category.lower()
        }
    else:
        iface_msgs = all_iface_msgs

    iface_keys = set(iface_msgs.keys())
    all_iface_keys = set(all_iface_msgs.keys())
    scicall_keys = set(scicall_wrappers.keys())

    unwrapped = sorted(iface_keys - scicall_keys, key=lambda k: iface_msgs[k]["num"])
    # Stale check always uses the full iface (not filtered)
    stale = sorted(scicall_keys - all_iface_keys)
    wrapped = sorted(iface_keys & scicall_keys, key=lambda k: iface_msgs[k]["num"])

    # Generate mode: output DeclareSciCall* lines grouped by category
    if args.generate:
        non_deprecated = [
            k for k in unwrapped if iface_msgs[k]["category"] != "Deprecated"
        ]
        by_cat = {}
        for key in non_deprecated:
            cat = iface_msgs[key]["category"]
            by_cat.setdefault(cat, []).append(key)

        for cat in sorted(by_cat.keys()):
            keys = by_cat[cat]
            print(f"// --- [{cat}] ({len(keys)} wrappers) ---")
            for key in keys:
                msg = iface_msgs[key]
                line = generate_wrapper(msg)
                print(line)
            print()
        print(f"// Total: {len(non_deprecated)} generated wrappers")
        return 0

    # Group unwrapped by category
    unwrapped_by_cat = {}
    for key in unwrapped:
        cat = iface_msgs[key]["category"]
        unwrapped_by_cat.setdefault(cat, []).append(key)

    # Summary
    cat_label = f" (category: {args.category})" if args.category else ""
    print(f"=== SciCall.h Coverage Report{cat_label} ===")
    print(f"  iface messages (fun/get/set): {len(iface_msgs)}")
    print(f"  SciCall.h wrappers:           {len(scicall_wrappers)}")
    print(f"  Wrapped (matched):            {len(wrapped)}")
    print(f"  Unwrapped (missing):          {len(unwrapped)}")
    print(f"  Stale/custom (no iface):      {len(stale)}")
    print()

    # Unwrapped messages
    if unwrapped:
        print(f"--- Unwrapped iface messages ({len(unwrapped)}) ---")
        for cat in sorted(unwrapped_by_cat.keys()):
            keys = unwrapped_by_cat[cat]
            print(f"\n  [{cat}] ({len(keys)} messages)")
            for key in keys:
                msg = iface_msgs[key]
                if args.verbose:
                    print(f"    SCI_{key} = {msg['num']}")
                    print(f"      {msg['type']} {msg['ret']} {msg['name']}({msg['params']})")
                    print(f"      iface line {msg['line']}")
                else:
                    print(f"    SCI_{key:<45s} {msg['type']:<4s} {msg['ret']:<16s} {msg['name']}({msg['params']})")
        print()

    # Stale wrappers (in SciCall.h but not in iface)
    if stale:
        print(f"--- Stale/custom wrappers ({len(stale)}) ---")
        print("  (In SciCall.h but no matching iface fun/get/set)")
        for key in stale:
            w = scicall_wrappers[key]
            status = " [commented]" if w["commented"] else ""
            print(f"    SCI_{key:<45s} SciCall_{w['fn']:<30s} line {w['line']}{status}")
        print()

    # Wrapped messages (optional)
    if args.show_wrapped and wrapped:
        print(f"--- Wrapped messages ({len(wrapped)}) ---")
        for key in wrapped:
            msg = iface_msgs[key]
            w = scicall_wrappers[key]
            status = " [commented]" if w["commented"] else ""
            print(f"    SCI_{key:<45s} -> SciCall_{w['fn']}{status}")
        print()

    # Exit code: 0 if no unwrapped non-deprecated messages, 1 otherwise
    non_deprecated_unwrapped = [
        k for k in unwrapped if iface_msgs[k]["category"] != "Deprecated"
    ]
    if non_deprecated_unwrapped:
        print(f"({len(non_deprecated_unwrapped)} unwrapped non-deprecated messages)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
