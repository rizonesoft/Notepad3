# Localization

There are 26 locales under `language\np3_LANG_COUNTRY\`; language packs build as separate DLLs.

## Resource invariants

- Every `IDS_MUI_*` in `language\common_res.h` must exist in all locale `strings_*.rc` files. A missing entry can break a language DLL build.
- Every `IDS_LEX_*` must exist in the corresponding locale `lexer_*.rc` files. See [Lexer development](lexer-development.md) for style-label localization.
- `.rc` files must be UTF-8 without BOM and use CRLF. Use `Build\rc_to_utf8.cmd` to strip accidental BOMs. In PowerShell use `[System.Text.UTF8Encoding]::new($false)`; in Python write `\r\n` explicitly.
- `Build\Notepad3.ini` and `Build\minipath.ini` are UTF-8 with BOM (`EF BB BF`); preserve it.
- For bulk locale edits, use `.venv\Scripts\python.exe` when available; `sed`/`perl` newline escaping is unreliable in Cygwin.

## Adding a UI string

1. Define `IDS_MUI_XXX` in `language\common_res.h` (13xxx for errors/warnings, 14xxx for information/prompts).
2. Add the English string to `language\np3_en_us\strings_en_us.rc`.
3. Add the same English text as a placeholder to the other 25 locale string files.
4. Display it with `InfoBoxLng()` / `MessageBoxLng()` and check results with `IsYesOkay()` or the appropriate dialog-result helper. `Settings.MuteMessageBeep` controls silent versus audible messages; provide both paths.
