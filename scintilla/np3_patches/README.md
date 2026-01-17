# Notepad3 Scintilla Patches

This folder contains NP3-specific patches applied to the upstream Scintilla library.

## Folder Structure

```
np3_patches/
├── README.md                          # This file
├── 001_scintilla_h_np3_exports.patch  # All Scintilla.h customizations
└── orig/
    └── Scintilla.h.558.orig           # Clean upstream Scintilla 5.5.8
```

## Patch Registry

| ID  | Patch File | Description | Status |
|-----|------------|-------------|--------|
| 001 | `001_scintilla_h_np3_exports.patch` | DLL exports, IME, Strikethrough, etc. | ✅ Applied |
| 008 | (pending) | DirectWrite font fix (#2080, #2262) | ⏳ Not applied |

## Applied Patches Detail

### Patch 001: Scintilla.h NP3 Exports

**File:** `include/Scintilla.h`

Contains all NP3-specific modifications to Scintilla.h:

1. **DLL Export Functions** (lines 18-30)
   - `WRECT` typedef for DPI handling
   - `Scintilla_RegisterClasses()` with `__declspec(dllexport)`
   - `Scintilla_ReleaseResources()`
   - `Scintilla_InputCodePage()`
   - `Scintilla_GetWindowDPI()`
   - `Scintilla_GetSystemMetricsForDpi()`
   - `Scintilla_AdjustWindowRectForDpi()`

2. **C++ Header Include** (lines 37-41)
   - Conditional `<cstdint>` vs `<stdint.h>`

3. **User-Defined Message Range**
   - `SCI_DEV_USER_DEFINED 6000`

4. **IME Detection Functions**
   - `SCI_ISIMEOPEN 6003`
   - `SCI_ISIMEMODECJK 6004`

5. **Strikethrough Style**
   - `SCI_STYLESETSTRIKE 6001`
   - `SCI_STYLEGETSTRIKE 6002`

6. **Regex Enhancement**
   - `SCFIND_DOT_MATCH_ALL 0x1000`

7. **Keyword Set Maximum**
   - Changed `KEYWORDSET_MAX` from 8 to 15

## Pending Patches

### Patch 008: DirectWrite Font Fix (NOT YET APPLIED)

**Upstream Bugs:** #2080, #2262, #2356
**Merge Request:** [MR #36](https://sourceforge.net/p/scintilla/code/merge-requests/36/)
**Files:** `win32/ScintillaWin.cxx`, `win32/PlatWin.cxx`

Fixes variable font and non-regular font style issues (Cascadia Mono Light, Iosevka, etc.)

**Related NP3 Issues:** #4150, #5455

## How to Apply Patches

```bash
# From scintilla/ directory:
git apply np3_patches/001_scintilla_h_np3_exports.patch
```

## How to Regenerate Patches

```bash
# After modifying Scintilla.h:
git diff --no-index np3_patches/orig/Scintilla.h.558.orig include/Scintilla.h > np3_patches/001_scintilla_h_np3_exports.patch
```

## Upgrade Process

See `todo/research/scintilla-lexilla-upgrade.md` for the complete upgrade checklist.

Quick steps:
1. Download new Scintilla
2. Copy new `include/Scintilla.h` to `np3_patches/orig/Scintilla.h.XXX.orig`
3. Apply patches or manually merge
4. Regenerate patch files
5. Update this README with new version numbers
