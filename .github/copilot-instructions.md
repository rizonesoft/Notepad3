# Copilot Instructions for Notepad3

Win32 C/C++ text editor on Scintilla/Lexilla. Built with Visual Studio 2026 (toolset v145) via MSBuild. Solution: `Notepad3.sln`. Windows-only.

## Build

Scripts under `Build\` (PowerShell under `Build\scripts\`):

- `Build\Build_x64.cmd [Release|Debug]` — single platform (also `_Win32`, `_ARM64`, `_x64_AVX2`)
- `Build\BuildAll.cmd` — all four platforms
- `Build\Clean.cmd` — clean outputs
- `nuget restore` once before first build
- `Version.ps1` regenerates `src\VersionEx.h` (`Major.YY.Mdd.Build`, build number in `Versions\build.txt`)
- Tests: `test\TestFileVersion.cmd`, `test\TestAhkNotepad3.cmd` (AutoHotkey). CI in `.github/workflows/build.yml` (windows-2022, Release × all four platforms).

Default config is Release.

### Invoking MSBuild from PowerShell

`msbuild` is **not** on the PATH in PowerShell. Do **not** call it directly — it will fail with "not recognized". Locate it via `vswhere.exe` first:

```powershell
$msbuild = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" `
    -latest -requires Microsoft.Component.MSBuild `
    -find MSBuild\**\Bin\MSBuild.exe | Select-Object -First 1
& $msbuild Notepad3.sln /m /p:Configuration=Release /p:Platform=x64 /nologo /v:minimal
Write-Host "Exit code: $LASTEXITCODE"
```

Check `$LASTEXITCODE` (0 = success) and scan the last output lines for `-> Notepad3.exe` or `error C` entries. The verbose output is long — use `Select-String` or tail filtering rather than reading it all.

## Core Modules (`src\`)

- `Notepad3.c/h` — `wWinMain`, `MainWndProc`, global state structs (`Globals`, `Settings`, `Settings2`, `Flags`, `Paths`), `MsgCommand()` dispatcher with static sub-handlers (`_HandleFileCommands`, `_HandleEditBasicCommands`, `_HandleViewAndSettingsCommands`, etc.).
- `Notepad3Util.c/h` — bitmap/toolbar loaders (`NP3Util_LoadBitmapFile`, `NP3Util_CreateScaledImageListFromBitmap`), wrap config (`NP3Util_SetWrapIndentMode`, `NP3Util_SetWrapVisualFlags`), middle-click auto-scroll (`NP3Util_AutoScrollStart/Stop`).
- `Edit.c/h` — find/replace (PCRE2), encoding, clipboard, indent, sort, bookmarks, folding, autocomplete.
- `Styles.c/h` — Scintilla styling, lexer selection, themes.
- `Dialogs.c/h` — dialogs, DPI-aware UI.
- `Encoding.c/h` — character encoding detection/conversion.
- `SciCall.h` — type-safe wrappers for Scintilla direct calls.
- `DynStrg.c/h` — `HSTRINGW` dynamic wide-string handle.
- `PathLib.c/h` — `HPATHL` path handle + long-path-aware Win32 wrappers.
- `TypeDefs.h` — `DocPos`, `DocLn`, `cpi_enc_t`, OS targeting, compiler macros.
- `MuiLanguage.c/h` — MUI language DLL loading.

## Vendored Libraries

`scintilla\` (5.5.8 + `np3_patches\`, docs in `scintilla\doc\`), `lexilla\` (5.4.6 + `np3_patches\`, docs in `lexilla\doc\`), `scintilla\pcre2\` (10.47, replaced archived Oniguruma), `src\uchardet\`, `src\tinyexpr\` / `src\tinyexprcpp\`, `src\uthash\`, `src\crypto\` (Rijndael/SHA-256 for AES-256).

## grepWin (`grepWin\`) — external tool, not built from source

Pre-built exes under `grepWin\portables\`; `.lang` files under `grepWin\translations\`. Runtime lookup in `src\Dialogs.c`: `Settings2.GrepWinPath` → `<ModuleDir>\grepWin\grepWin-x{64,86}_portable.exe` → `%APPDATA%\Rizonesoft\Notepad3\grepWin\`. `grepWinLangResName[]` in `MuiLanguage.c` maps Notepad3 locales to `.lang` filenames (written into `grepwin.ini` before launch). Portable build scripts (`Build\make_portable_*.cmd`) package both portable exes + LICENSE + all `.lang` files. ARM64 uses the x64 exe via emulation (`#if defined(_M_ARM64)` in `Notepad3.c`).

## Lexers (`src\StyleLexers\`)

Each language is one `styleLexXXX.c` defining an `EDITLEXER` struct (see existing files for template). To add: create the file, register in `Styles.c` lexer array, add localization string IDs.

Easy-to-miss touchpoints — derivable but only if you know to look:

- **Property setter arm in `EditLexer.c`.** If the lexer exposes Scintilla properties (e.g. `lexer.foo.allow.X`), add a `case SCLEX_FOO:` in the top-of-file dispatch that calls `SciCall_SetProperty(...)`. Without it the `OptionsXXX` constructor defaults silently apply — symptom is "feature looks broken" (e.g. comments highlighted as ERROR).
- **Shared `SCLEX_*` values**: `SCLEX_CPP` is the lexer for **C, C#, Java, JS, RC**; `SCLEX_D` for **D + Go**; `SCLEX_PYTHON` for **Python + Awk**. Property-arm or `EDITLEXER` table changes under those `SCLEX_*` cases ripple to every wiring using that lexer (e.g. adding `SCE_C_USERLITERAL` affects all SCLEX_CPP languages).
- **Comment-toggle arms.** If the lexer has comments, add `case SCLEX_FOO:` in BOTH `Lexer_GetStreamCommentStrgs` and `Lexer_GetLineCommentStrg` in `EditLexer.c`, or Edit > Toggle Block/Line Comment is a no-op.
- **Theme INI sections live under `pszName` (4th `EDITLEXER` field), not the lexer name string.** Each new lexer needs a `[<pszName>]` block in every theme INI: `Build\Notepad3.ini`, `Build\Themes\*.ini`, `res\StdDarkModeScheme.ini`, locale variants `Build\Notepad3_<locale>.ini`. Renaming `pszName` orphans existing user style customizations.
- **New style rows need theme INI entries too** — same rule as new lexers, just per-row. Each `EDITLEXER` row's label string (e.g. `L"User Literal"`) needs a matching `User Literal=<spec>` line in every theme INI's `[<pszName>]` block. Without it, the EDITLEXER inline default applies and the row is invisible to theme switching. `lexilla/wscite/*.properties` (one per language) are useful colour references.
- **Homebrew lexers in `lexilla/lexers_x/`** (6 files): `LexAHK`, `LexCSV`, `LexHTML` (NP3 fork — used by both `SCLEX_HTML` and `SCLEX_XML`), `LexJSON5`, `LexKotlin`, `LexVerilog` (`SCLEX_VERILOG` + `SCLEX_SYSVERILOG`). Their `SCE_*_*` enums live in `lexilla/lexers_x/SciXLexer.h`, not the stock `lexilla/include/SciLexer.h` — `#include "lexers_x/SciXLexer.h"` if you need the homebrew constants.

## Localization (`language\`)

26 locales under `np3_LANG_COUNTRY\`. Language packs build as separate DLLs.

**Invariant:** every `IDS_MUI_*` defined in `common_res.h` must exist in **all** `strings_*.rc` files. A missing entry breaks the corresponding language DLL build. For bulk insertions across locales, use a `.venv/Scripts/python.exe` script — `sed`/`perl` `\n` escaping is unreliable in Cygwin.

**Same rule for `IDS_LEX_*`** — these live in `language\np3_*\lexer_*.rc` (not `strings_*.rc`).

### Adding a string resource

1. `#define IDS_MUI_XXX <id>` in `language\common_res.h` (13xxx errors/warnings, 14xxx info/prompts).
2. English in `language\np3_en_us\strings_en_us.rc`.
3. Same English as placeholder in the 25 other `strings_*.rc` files (translators update later).
4. Display via `InfoBoxLng()` / `MessageBoxLng()`; check `IsYesOkay()`. `Settings.MuteMessageBeep` controls silent vs. sound — provide both paths.

## Clipboard Monitoring (Pasteboard Mode)

Runtime toggle `IDM_EDIT_TOGGLE_PASTEBOARD`; `/B` enables at startup. Not persisted. `IsPasteBoardActive()` in `Notepad3.h` is the cross-TU query.

- **Mutex with Tail** (`FileWatching.MonitoringLog`): each greys the other in `MsgInitMenu`, the tail toolbar button, and the "Monitoring Log" checkbox in `ChangeNotifyDlgProc`. `/B` + persisted `MonitoringLog=true` → `/B` wins the session, INI preserved.
- **`/B` initial auto-paste vs runtime toggle**: `/B` pastes the current clipboard once — only on an empty untitled buffer (no file arg, no `/c`, no auto-loaded MRU). Runtime toggle never auto-pastes. Preserve this asymmetry.
- `Settings2.PasteBoardSeparator`: `\x01` = one document EOL; `\0` = none; else verbatim. Also suppressed on first paste after enable and when caret is at line start.

## Taskbar Shift+Click / New Window

- Explorer runs Shift+Click via an implicit `.lnk` it creates once from the window's `RelaunchCommand` (`%APPDATA%\Microsoft\Internet Explorer\Quick Launch\User Pinned\ImplicitAppShortcuts\`) and never refreshes — the new process gets **no arguments**, only the link path in `STARTUPINFO.lpTitle` (`STARTF_TITLEISLINKNAME`). Changing `RelaunchCommand` does not affect existing links. `Start-Process <that .lnk>` emulates a Shift+Click for testing.
- A no-arg start whose launch link carries an AppID hands off via `WM_NEWWINDOWREQUEST` to `DialogNewWindow` (File > New Window) in the topmost enabled window of that AppID group, then exits. `DialogNewWindow` passes `-n`, so new-window requests deliberately bypass `ReuseWindow`. Preserve this asymmetry.
- Every property set via `SHGetPropertyStoreForWindow` must also be cleared in `ClearWindowAppUserModelID()` (`WM_DESTROY`) — required by the API.

## File I/O

- `FileSave()` / `FileLoad()` (`Notepad3.c`) → `FileIO()` → `EditSaveFile()` / `EditLoadFile()` (`Edit.c`).
- Atomic save (temp + `ReplaceFileW`) controlled by `Settings2.AtomicFileSave`.
- `Globals.dwLastError` holds Win32 error. `FileSave()` checks `ERROR_ACCESS_DENIED`, `ERROR_PATH_NOT_FOUND` before generic fallback.
- `InstallFileWatching()` uses `FindFirstChangeNotificationW` on the parent dir. `InstallFileWatching(false)` before save, `(true)` after.

## PCRE2 (`scintilla\pcre2\`)

`scintilla\pcre2\scintilla\PCRE2RegExEngine.cxx` compiled with `SCI_OWNREGEX` overrides Scintilla's built-in regex. Key points: `FindText`, `SubstituteByPosition`, `convertReplExpr` (normalizes `\1`-`\9`→`$1`-`$9`, processes `\n\t\xHH\uHHHH`), `translateRegExpr` (`\<`/`\>` → lookarounds, `\uHHHH` → `\x{HHHH}`). Standalone `RegExFind` used by `EditURLDecode`.

Replacement backref syntax: `$0`-`$99`, `\0`-`\9`, `${name}`, `${+name}`.

URL hotspot regex: `HYPLNK_REGEX_FULL` in `src\Edit.c` — matches `https?://`, `ftp://`, `file:///`, `file://`, `mailto:`, `www.`, `ftp.`. Trailing group excludes `.,:?!`.

## DarkMode (`src\DarkMode\`)

Windows 10/11 dark mode via IAT hooks on uxtheme/user32 (stub DLLs included).

## ARM64

Supported: Win32 (x86), x64, x64_AVX2, ARM64. **ARM 32-bit is not supported** — `Release|ARM` maps to Win32.

- Both ARM64 and x64 define `_WIN64`. Use `_M_ARM64` (or `NP3_BUILD_ARM64` in `TypeDefs.h`) to distinguish.
- Rendering: defaults to `SC_TECHNOLOGY_DIRECTWRITERETAIN` (2) — preserves Direct2D back buffer, avoids flicker on Qualcomm Adreno + Win11 25H2 DWM. Main window gets `WS_EX_COMPOSITED`. User can override via `RenderingTechnology` / View menu.
- Build: `CETCompat=false` (CET is x86/x64 only), `TargetMachine=MachineARM64`, `_WIN64` defined. Fix scripts: `Build\scripts\FixARM64{CETCompat,CrossCompile,OutDir}.ps1`.
- grepWin: uses x64 exe via emulation.
- `MsgThemeChanged()` wraps bar recreate / lexer reset / restyle in `WM_SETREDRAW FALSE/TRUE` + single `RedrawWindow()` at end. DarkMode `RedrawWindow()` in `ListViewUtil.hpp` omits `RDW_ERASE`.

## Conventions

### Formatting & encoding

- `.clang-format` in `src\` — 4-space indent, Stroustrup braces, left-aligned pointers, no column limit, no include sorting.
- `.editorconfig` — UTF-8/CRLF for source, 4-space indent C/C++. Lexilla uses tabs (upstream preserved).
- **`language\*\*.rc` = UTF-8 WITHOUT BOM, CRLF.** Never write with BOM. Use `Build\rc_to_utf8.cmd` to strip. PowerShell: `[System.Text.UTF8Encoding]::new($false)`. Python: write `\r\n` explicitly.
- **`Build\Notepad3.ini`, `Build\minipath.ini` = UTF-8 WITH BOM** (`EF BB BF`). Preserve it.
- `strsafe.h` throughout; deprecated string functions disabled.

### Types

- `DocPos` / `DocPosU` / `DocLn` for document positions/lines (not raw `int`).
- `cpi_enc_t` for encodings.
- `HSTRINGW` / `HPATHL` instead of raw `WCHAR*` buffers.
- `NOMINMAX` is global — use `min()`/`max()` or typed equivalents.

### Scintilla interaction

Always use `SciCall.h` wrappers (direct function pointer for performance). Add missing wrappers there. Naming: `DeclareSciCall{V|R}{0|01|1|2}` — V=void, R=return; 0/1/2 = param count; `01` = lParam-only. The `msg` arg is the suffix after `SCI_`.

Example wrappers:

```c
DeclareSciCallV0(Undo, UNDO);                                            // SciCall_Undo()
DeclareSciCallV1(SetTechnology, SETTECHNOLOGY, int, technology);         // SciCall_SetTechnology(int)
DeclareSciCallR0(GetTextLength, GETTEXTLENGTH, DocPos);                  // DocPos SciCall_GetTextLength()
DeclareSciCallR1(SupportsFeature, SUPPORTSFEATURE, bool, int, feature);  // bool SciCall_SupportsFeature(int)
```

### Scintilla/Lexilla versions

Vendored 5.5.8 / 5.4.6 with NP3 patches under `np3_patches\`. Versions in `scintilla\version.txt` / `lexilla\version.txt`. Check offline docs under `scintilla\doc\` and `lexilla\doc\` for new APIs.

### Global state

Use existing structs (`Globals`, `Settings`, `Settings2`, `Flags`, `Paths`). Don't add new globals.

### Dialog return values (`InfoBoxLng`)

`InfoBoxLng()` returns `MAKELONG(button, mode)` — never compare/switch on the raw return when the suppression key is non-NULL: a saved-answer replay sets HIWORD too, so `result == IDNO` is false. Use `IsYesOkay()` / `IsRetryContinue()` / `IsNoCancelClose()` (in `src\Dialogs.h`), or extract via `INFOBOX_ANSW(r)` (LOWORD) and `INFOBOX_MODE(r)` (HIWORD). Switching on the raw value is a latent bug if a suppression key is ever added.

### Portable INI design

- INI beside the exe. No registry. Runs on defaults if none exists (user creates via "Save Settings Now").
- **Admin redirect**: `Notepad3.ini=<path>` in `[Notepad3]` redirects to per-user path (up to 2 levels). Redirect targets auto-created via `CreateIniFileEx()`.
- `Paths.IniFile` = active writable INI; `Paths.IniFileDefault` = recovery fallback.
- Init flow (in `src\Config\Config.cpp`): `FindIniFile()` → `TestIniFile()` → `CreateIniFile()` → `LoadSettings()`.
- **`SettingsVersion` defaults to `CFG_VER_CURRENT`** when missing — empty/new INI gets current defaults.
- **`bIniFileFromScratch`** is set when INI is 0 bytes, cleared after `SaveAllSettings()`. While set, `MuiLanguage.c` suppresses writing `PreferredLanguageLocaleName`.
- **Empty lexer sections are pruned** in `Style_ToIniSection()` via `IniSectionGetKeyCount()` — only sections with non-default styles persist.
- MiniPath uses the same pattern (`minipath\src\Config.cpp`).
- **New `Settings2` (or other INI) params must be documented in BOTH places**: a commented entry under `[Settings2]` in `Build\Notepad3.ini` AND a `#### \`Name=default\`` heading + prose paragraph in the matching topical section of `readme\config\Configuration.md` (the user-facing reference). Updating only the INI is incomplete.

### Save/Load macros (`src\Config\Config.cpp`)

- **`Encoding_MapIniSetting` is asymmetric.** CPI constants (`CPI_UTF8=6`, `CPI_OEM=1`, …) don't equal INI values (`3`, `5`, …). `Encoding_MapIniSetting(true, val)` = INI→CPI (load); `(false, val)` = CPI→INI (save). CPI with `bLoad=true` produces wrong results. Reference: `MRU_Save()` uses `false`.
- **Defaults depending on `DefaultEncoding`** (e.g. `SkipANSICodePageDetection`, `LoadASCIIasUTF8`) must be recalculated in `_SaveSettings()` before `SAVE_VALUE_IF_NOT_EQ_DEFAULT` fires. See the `bCurrentEncUTF8` block.

### Creating directories

`Path_CreateDirectoryEx(hpth)` wraps `SHCreateDirectoryExW`. Success: `SUCCEEDED(hr) || (hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS))`. Reference: `CreateIniFile()` in `Config.cpp`.

### Long-path / PathLib wrappers

Never call Win32 file APIs directly with `Path_Get(hpth)`. Use PathLib wrappers — they apply `\\?\` prefix conditionally (only when `RtlAreLongPathsEnabled()` is false AND path ≥ 260 chars):

| Wrapper | Win32 |
|---------|-------|
| `Path_CreateFile` | `CreateFileW` |
| `Path_DeleteFile` | `DeleteFileW` |
| `Path_GetFileAttributes` / `Path_GetFileAttributesEx` | `GetFileAttributes[Ex]W` |
| `Path_SetFileAttributes` | `SetFileAttributesW` |
| `Path_ReplaceFile` | `ReplaceFileW` |
| `Path_MoveFileEx` | `MoveFileExW` |
| `Path_FindFirstFile` | `FindFirstFileW` |
| `Path_CreateDirectoryEx` | `SHCreateDirectoryExW` |
| `Path_IsExistingFile` / `Path_IsExistingDirectory` | `GetFileAttributesW` + check |

### Path comparison

Use `Path_StrgComparePath()` (normalization + `CompareStringOrdinal`, locale-independent, case-insensitive). For raw wide strings: `CompareStringOrdinal(s1, -1, s2, -1, TRUE)`. **Never** `_wcsicmp` / `_wcsnicmp` (locale-dependent).

### Undo/Redo transactions

Use `_BEGIN_UNDO_ACTION_` / `_END_UNDO_ACTION_` macros (`Notepad3.h`) for grouping; they also throttle notifications during bulk edits.

### WriteAccessBuf — dangling pointer anti-pattern

**NEVER** use a pointer from `Path_WriteAccessBuf()` / `StrgWriteAccessBuf()` after any operation that may reallocate/swap the same handle's buffer: `Path_CanonicalizeEx`, `Path_Swap` / `StrgSwap`, `Path_ExpandEnvStrings` / `ExpandEnvironmentStrgs`, `Path_Append` / `Path_Reset`, `StrgCat` / `StrgInsert` / `StrgFormat` / `StrgReset`, `Path_NormalizeEx` / `Path_AbsoluteFromApp` / `Path_RelativeToApp`.

After any of these:
- Read-only: `Path_Get(h)` / `StrgGet(h)`.
- Read-write: re-obtain via `Path_WriteAccessBuf(h, 0)` / `StrgWriteAccessBuf(h, 0)` (size 0 = no resize).
