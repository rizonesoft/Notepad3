# Configuration and INI

- INI files are beside the executable; there is no registry dependency. The application uses defaults if there is no INI; the user can create one with "Save Settings Now".
- Admin redirect: `Notepad3.ini=<path>` in `[Notepad3]` redirects to a per-user path (up to two levels). Redirect targets are auto-created by `CreateIniFileEx()`.
- `Paths.IniFile` is the active writable INI; `Paths.IniFileDefault` is the recovery fallback.
- Initialization in `src\Config\Config.cpp`: `FindIniFile()` → `TestIniFile()` → `CreateIniFile()` → `LoadSettings()`.
- Missing `SettingsVersion` defaults to `CFG_VER_CURRENT`; a new or empty INI gets current defaults.
- `bIniFileFromScratch` is set when the INI is 0 bytes and cleared after `SaveAllSettings()`. While set, `MuiLanguage.c` suppresses writing `PreferredLanguageLocaleName`.
- Empty lexer sections are pruned in `Style_ToIniSection()` via `IniSectionGetKeyCount()` after `Style_CanonicalSectionToIniCache()` establishes ordering. Only sections with non-default styles persist.
- MiniPath follows the same pattern in `minipath\src\Config.cpp`.
- Any new `Settings2` (or other INI) parameter must be documented in both places: a commented entry under `[Settings2]` in `Build\Notepad3.ini`, and a `#### \`Name=default\`` heading plus prose in the matching topical section of `readme\config\Configuration.md`.

## Save/load macros

- `Encoding_MapIniSetting` is asymmetric: CPI constants do not equal INI values. `Encoding_MapIniSetting(true, val)` maps INI→CPI for loading; `false` maps CPI→INI for saving. Passing CPI with `bLoad=true` produces incorrect results; see `MRU_Save()` for the save pattern.
- Defaults that depend on `DefaultEncoding` (for example, `SkipANSICodePageDetection` and `LoadASCIIasUTF8`) must be recalculated in `_SaveSettings()` before `SAVE_VALUE_IF_NOT_EQ_DEFAULT` runs. See the `bCurrentEncUTF8` block.
