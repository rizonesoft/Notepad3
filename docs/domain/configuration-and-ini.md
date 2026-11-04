---
status: agent-observed
verified-at: b56c3e65dcc0eb6f783a1ef7e5a2289e3ec71adc
owner: "@RaiKoHoff"
anchors:
  - path: src/Config/Config.cpp
    symbol: _HandleIniFileRedirect
  - path: src/Config/Config.cpp
    symbol: FindIniFile
  - path: src/Config/Config.cpp
    symbol: bCurrentEncUTF8
  - path: src/Encoding.c
    symbol: Encoding_MapIniSetting
  - path: src/Notepad3.c
    symbol: wWinMain
  - path: src/Config/Config.cpp
    symbol: CmdSaveSettingsNow
  - path: src/Config/Config.cpp
    symbol: SaveAllSettings
  - path: src/MuiLanguage.c
    symbol: SetMuiLanguage
  - path: src/Styles.c
    symbol: Style_ToIniSection
---

# Configuration and INI

Evidence scope: code inspection of redirect creation/depth/fallback, initialization, from-scratch state, conditional lexer ordering/pruning, encoding-setting mapping, and encoding-dependent save defaults at the commit above. Other entries are existing guidance to verify when used; this pilot does not certify the whole page. Human review is pending.

- `FindIniFile()` in `src\Config\Config.cpp` searches beside the executable when no explicit INI path is supplied. The application uses defaults if there is no INI; the user can create one with "Save Settings Now".
- Admin redirect: `Notepad3.ini=<path>` in `[Notepad3]` redirects to a per-user path (up to two levels). In `src\Config\Config.cpp`, `FindIniFile()` limits `_HandleIniFileRedirect()` to two levels; missing targets are created with `CreateIniFile()`. Each redirecting INI can supply `Settings2.DefaultDirectory`, with later values overriding earlier ones. If target creation fails, its path is retained in `Paths.IniFileDefault` for "Save Settings Now".
- `Paths.IniFile` identifies the active INI; `Globals.bCanSaveIniFile` tracks save eligibility. `Paths.IniFileDefault` retains a fallback creation target.
- Initialization calls are in `src\Notepad3.c:wWinMain`: `FindIniFile()` → `TestIniFile()` → `CreateIniFile()` → `LoadSettings()`; the functions are defined in `src\Config\Config.cpp`.
- Missing `SettingsVersion` defaults to `CFG_VER_CURRENT`; a new or empty INI gets current defaults.
- `bIniFileFromScratch` is set from the INI size at startup and when `CmdSaveSettingsNow()` successfully creates/opens a fallback target; it is reset in `SaveAllSettings()`. While set, `src\MuiLanguage.c:SetMuiLanguage` suppresses writing `PreferredLanguageLocaleName`.
- Empty lexer sections are pruned in `src\Styles.c:Style_ToIniSection()` via `IniSectionGetKeyCount()`. Canonical ordering is rebuilt there only when `bForceAll` or `bIniFileFromScratch` is set; `SaveAllSettings()` also rebuilds it for a from-scratch INI. Forced saves can retain default styles, so persistence is not limited to non-default styles.
- Any new `Settings2` (or other INI) parameter must be documented in both places: a commented entry under `[Settings2]` in `Build\Notepad3.ini`, and a `#### \`Name=default\`` heading plus prose in the matching topical section of `readme\config\Configuration.md`.

## Save/load macros

- `Encoding_MapIniSetting` is asymmetric: CPI constants do not equal INI values. `Encoding_MapIniSetting(true, val)` maps INI→CPI for loading; `false` maps CPI→INI for saving. Passing CPI with `bLoad=true` produces incorrect results; see `MRU_Save()` for the save pattern.
- Defaults that depend on `DefaultEncoding` (for example, `SkipANSICodePageDetection` and `LoadASCIIasUTF8`) must be recalculated in `_SaveSettings()` before `SAVE_VALUE_IF_NOT_EQ_DEFAULT` runs. See the `bCurrentEncUTF8` block.
