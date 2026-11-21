# Localization

When to read: UI strings, locale resources

There are 26 locales under `language\np3_<LANG>_<COUNTRY>\`; language packs build as separate DLLs.

## Resource invariants

- Set the thread UI language before caching strings from `LoadMUILibrary`. Loading the module alone can leave `LoadStringW` resolving the previous language. Keep `SetMuiLanguage` before the status label loads in [`LoadLanguageResources`](../../src/MuiLanguage.c#L467-L486). A headless probe using the built language packs got `Z` when switching from German to French before `SetThreadUILanguage`, and `Lgn` after.
- Every `IDS_MUI_*` in `language\common_res.h` must exist in all locale `strings_*.rc` files. A missing entry does not fail the build: `LoadLngStringW()` silently falls back to the built-in English string.
- Every `IDS_LEX_*` must exist in the corresponding locale `lexer_*.rc` files. See [Lexer development](lexer-development.md) for style-label localization.
- `.rc` files must be UTF-8 without BOM and use CRLF. Use `Build\rc_to_utf8.cmd` to strip accidental BOMs. In PowerShell use `[System.Text.UTF8Encoding]::new($false)`; in Python write `\r\n` explicitly.
- `Build\Notepad3.ini` and `Build\minipath.ini` are UTF-8 with BOM (`EF BB BF`); preserve it.
- For bulk locale edits, use the Python in the git-ignored `.venv` when available; `sed`/`perl` newline escaping is unreliable in Cygwin.

## Lazy language discovery

- Keep the full language-pack scan off the startup path: [`LoadLanguageResources`](../../src/MuiLanguage.c#L386-L403) probes only the preferred locale until the inventory is known. The first Settings popup or first use of preferred-locale date formatting calls [`EnsureLanguageDLLsChecked`](../../src/MuiLanguage.c#L328-L346), which caches the inventory and disables that date-format option if no additional pack exists. Keep the date-format call independent of the menu so shortcuts work before Settings opens ([date formatting](../../src/Edit.c#L2966-L2971)).
- Keep `IDM_SET_TABSASSPACES` first in every localized Settings menu. [`InitLanguageMenu`](../../src/MuiLanguage.c#L519-L557) uses it to recognize an unpopulated popup; insertion moves it down, preventing duplicates on later opens. Set the active-language and date-format checkmarks during insertion: [`WM_INITMENUPOPUP`](../../src/Notepad3.c#L2491-L2499) runs after the usual `WM_INITMENU` update. See the [English menu resource](../../language/np3_en_us/menu_en_us.rc#L477-L479).

## Adding a UI string

1. Define `IDS_MUI_XXX` in `language\common_res.h` with a free ID next to related entries; the IDs are grouped by feature, not by message kind.
2. Add the English string to `language\np3_en_us\strings_en_us.rc`.
3. Add the same English text as a placeholder to the other 25 locale string files.
4. Display it with `InfoBoxLng()` and check results with `IsYesOkay()` or the appropriate dialog-result helper. Beep through `SimpleBeep()` / `AttentionBeep()`, which honour `Settings.MuteMessageBeep`; a direct `MessageBeep()` ignores it.
