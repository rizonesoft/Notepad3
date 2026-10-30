# TinyExpr++ and the C Wrapper

Notepad3's C code never includes `src\tinyexprcpp\tinyexpr.h` (C++20). It uses the C interface `src\tinyexprcpp\tinyexpr_cif.h` / `tinyexpr_cif.cpp`, which wraps `te_parser`. Do not edit the vendored `tinyexpr.cpp` / `tinyexpr.h` to fix Notepad3 behavior; adapt the wrapper. Library version is recorded in `src\tinyexprcpp\version.txt` and `TINYEXPR_CPP_*_VERSION`.

## Wrapper contract (`tinyexpr_cif.h`)

| Function | Notes |
|---|---|
| `te_interp(expr, &err)` | Parse + evaluate. NaN on error. `err` is `0` or the **1-based** error position in the caller's text. |
| `te_interp_ex(expr, &err, &isLogical)` | `te_interp` plus the `te_is_logical_expr` flag from the same parse; used by `_EvalTinyExpr`, which retries at successive start offsets. The position is clamped to the input length. |
| `te_compile` / `te_eval` / `te_free` | Bound variables are `{name, const double*}`; pointers must outlive `te_eval`. Returns NULL with `err` set for a bad expression, a NULL name/address, or a duplicate name. |
| `te_is_logical_expr(expr)` | Lexical check (depth 0 operators, `true`/`false`, `AND`/`OR`/`NOT`/`IS*` calls); never evaluates. |
| `te_interp_str` | Cooked string ("true"/"false" or number). Currently unused by Notepad3 (`TinyExprToStringA` in `Notepad3.c` formats itself for hex/bin modes). |
| `te_supports_64bit` | `0` for the default `double` te_type. |

`tinyexpr_cif.cpp` deliberately does not include `tinyexpr_cif.h` (the C `te_variable` / `te_expr` names clash with the C++ classes); keep signatures in sync by hand.

## Invariants in `tinyexpr_cif.cpp`

- **`te_cif_preprocess()` keeps the input length.** TinyExpr++ error positions index the text it was given, so every rewrite must be length-preserving, otherwise callers (`Edit.c` truncating at `err`, Go To dialog) point to the wrong character. It blanks a leading `=` (spreadsheet prefix the library would strip) and terminated comments (a comment acts as whitespace, so `1/**/2` is a parse error rather than `12`), rewrites `0b<bits>` to decimal padded with blanks (saturates above 64 significant bits), and copies `"..."` literals verbatim. An unterminated `/*` is left for the library to report.
- **String literals** (library 1.2.0) have no escapes; the next `"` ends them. `te_strip_outer_parens` and `te_expression_is_logical` must skip them.
- **`0b` token boundaries** are operators, `(`, `,`, `;`, whitespace, `~`, and the cp1252 `×` / `÷` (`0xD7` / `0xF7`, as in `te_is_op`).
- **`te_cif_init_parser()` registers the compat `log` (log10) and the locale separators.** In `te_compile` call it *after* `set_variables_and_functions()`, which replaces the whole set. A user variable named `log` takes precedence (`insert` does not overwrite).
- **Locale separators:** if the C runtime locale decimal point is `,`, decimal becomes `,` and list separator `;`.
- No exception may leave an `extern "C"` function; allocate inside the `try` and use `std::unique_ptr`.

## Callers

`Notepad3.c` `_EvalTinyExpr` (`=?` trigger), the status-bar selection evaluator (both via `te_interp_ex`) and `_InterpMultiSelectionTinyExpr`, `Edit.c` `EditModifyNumber`, `EditModifyLinesDlg` (compile once, `te_eval` per line with `L`, `I`, `N`), Go To dialog, `Dialogs.c` autosave interval. Results converted to line/column numbers go through `d2lnc()` (`Helpers.h`; NaN -> 0, out-of-range and Inf clamped, unlike `d2ln`) in `EditModifyLinesDlg` and the Go To dialog; `EditModifyNumber` evaluates `(sel
)+1` so a trailing `//` comment cannot swallow `)+1`. Input is converted with `te_cp()` (1252) and `te_invalid_chr()`.

## Test

No automated test exists. A throwaway harness linking `tinyexpr_cif.cpp` + `tinyexpr.cpp` (`cl /std:c++20 /EHsc`) is enough; check `0b` literals, `=1`, comments, error positions (e.g. `0b1111111111 + * 2` -> `16`), and variable binding.
