# CLAUDE.md

Guidance for Claude Code working in this repo. Read once per session; covers non-obvious mechanics and gotchas. Self-evident build/layout information is not duplicated here.

## Project

Notepad3 — Win32 C/C++ text editor on Scintilla/Lexilla. Ships with MiniPath (`Ctrl+M`) and integrates external grepWin (`Ctrl+Shift+F`) via pre-built portable exes. BSD-3. Windows-only.

## Build

Build scripts live in `Build\` (PowerShell under `Build\scripts\`):

- `Build\Build_x64.cmd [Release|Debug]` — single platform (also `_Win32`, `_ARM64`, `_x64_AVX2`)
- `Build\BuildAll.cmd` — all platforms
- `msbuild Notepad3.sln /m /p:Configuration=Release /p:Platform=x64` — CI equivalent
- `Build\Clean.cmd` — clean outputs
- Run `nuget restore Notepad3.sln` once before first build.
- Run `Version.ps1` before building to regenerate `src\VersionEx.h` (format `Major.YY.Mdd.Build`; build number in `Versions\build.txt`).
- Tests: `test\TestFileVersion.cmd`, `test\TestAhkNotepad3.cmd` (needs AutoHotkey). CI matrix in `.github/workflows/build.yml` (windows-2022).

Default configuration is Release.

## Code Map (`src\`)

| File | Purpose |
|------|---------|
| `Notepad3.c/h` | `wWinMain`, `MainWndProc`, global state structs (`Globals`, `Settings`, `Settings2`, `Flags`, `Paths`), `MsgCommand()` dispatcher |
| `Notepad3Util.c/h` | Image/toolbar helpers, word-wrap config, middle-click auto-scroll |
| `Edit.c/h` | Find/replace (PCRE2), encoding, clipboard, indent, sort, bookmarks, folding, autocomplete |
| `Styles.c/h` | Scintilla styling, lexer selection, themes, margins |
| `Dialogs.c/h` | Dialogs, DPI-aware UI, window placement |
| `Config/Config.cpp/h` | INI load/save, MRU |
| `Encoding.c/h` | Encoding detection/conversion (wraps uchardet) |
| `SciCall.h` | Type-safe wrappers for Scintilla direct calls (avoid `SendMessage`) |
| `DynStrg.c/h` | `HSTRINGW` dynamic wide-string handle |
| `PathLib.c/h` | `HPATHL` path handle + long-path-aware Win32 wrappers |
| `TypeDefs.h` | `DocPos`, `DocLn`, `cpi_enc_t`, OS targeting, compiler macros |
| `MuiLanguage.c/h` | MUI language DLL loading |
| `StyleLexers\styleLexXXX.c` | Per-language `EDITLEXER` definitions (~50 files) |

### Menu / Command Dispatch

- `MsgInitMenu()` (`WM_INITMENU`) — enable/check state.
- `MsgCommand()` (`WM_COMMAND`) — thin dispatcher delegating to static sub-handlers, each returning `true` if it handled:

| Handler | Scope |
|---------|-------|
| `_HandleFileCommands` | `IDM_FILE_*` |
| `_HandleEncodingCommands` | `IDM_ENCODING_*`, `IDM_LINEENDINGS_*` |
| `_HandleEditBasicCommands` | `IDM_EDIT_UNDO`..`CMD_VK_INSERT` |
| `_HandleEditLineManipulation` | `IDM_EDIT_ENCLOSESELECTION`..`IDM_EDIT_INSERT_GUID` |
| `_HandleEditTextTransform` | `IDM_EDIT_LINECOMMENT`..`IDM_EDIT_HEX2CHAR` |
| `_HandleEditFind` | `IDM_EDIT_FINDMATCHINGBRACE`..`IDM_EDIT_GOTOLINE` |
| `_HandleViewAndSettingsCommands` | `IDM_VIEW_*`, `IDM_SET_*` |
| `_HandleHelpCommands` | `IDM_HELP_*` |
| `_HandleCmdCommands` | `CMD_*` |
| `_HandleToolbarCommands` | `IDT_*` via `s_ToolbarDispatch[]` |

### Clipboard Monitoring (Pasteboard Mode)

Runtime-toggleable; external clipboard changes are pasted at the caret.

- Menu: `IDM_EDIT_STOP_PASTEBOARD` — "Toggle Clipboard Monitoring" (legacy identifier kept). Check mark reflects state.
- Helpers `PasteBoard_Start(HWND)` / `PasteBoard_Stop(HWND)` wrap `AddClipboardFormatListener` + `ID_PASTEBOARDTIMER`. Used at startup for `/B` and from the toggle handler.
- **Not persisted** — always OFF at startup unless `/B`.
- **Mutex with Tail** (`IDM_VIEW_CHASING_DOCTAIL` / `FileWatching.MonitoringLog`): each mode greys the other in `MsgInitMenu`; tail toolbar button (`IDT_VIEW_CHASING_DOCTAIL`) also greyed; "Monitoring Log" checkbox in `ChangeNotifyDlgProc` greyed while pasteboard active. `IsPasteBoardActive()` exposed in `Notepad3.h` for cross-TU use.
- Startup conflict (`/B` + persisted `MonitoringLog=true`): `/B` wins this session; `FileWatching.MonitoringLog` cleared in memory, `Settings.MonitoringLog` (INI) preserved.
- `PasteBoardTimerProc` pastes at current caret. `Settings2.PasteBoardSeparator` pre-pended before each new entry; suppressed on (1) first paste after enable and (2) caret at line start. `\x01` = one document EOL; `\0` = no separator.
- Status bar `STATUS_OVRMODE` shows `CBS` while active (passive indicator).

## Vendored Libraries

`scintilla\` (5.5.8), `lexilla\` (5.4.6), `scintilla\pcre2\` (PCRE2 10.47), `src\uchardet\`, `src\tinyexpr\` / `src\tinyexprcpp\`, `src\uthash\`, `src\crypto\` (Rijndael/SHA-256). NP3 patches under each `np3_patches\`; offline docs under `scintilla\doc\` and `lexilla\doc\`.

## grepWin Integration (`grepWin\`)

External tool, **not built from source**. Pre-built exes under `grepWin\portables\`; `.lang` files under `grepWin\translations\`. Runtime lookup order (in `src\Dialogs.c`): `Settings2.GrepWinPath` → `<ModuleDir>\grepWin\grepWin-x{64,86}_portable.exe` → `%APPDATA%\Rizonesoft\Notepad3\grepWin\`. `grepWinLangResName[]` in `MuiLanguage.c` maps Notepad3 locales → `.lang` filenames. ARM64 uses the x64 exe via emulation.

## Adding a Lexer

`styleLexXXX.c` defines an `EDITLEXER` (see any existing file for the struct), register in the `Styles.c` lexer array, add localization string IDs.

## Localization

- 26 locales under `language\np3_LANG_COUNTRY\`. Language DLLs are separate projects in the solution.
- **`.rc` files are UTF-8 WITHOUT BOM, CRLF line endings.** Never write with BOM. Use `Build\rc_to_utf8.cmd` to strip accidental BOMs. In PowerShell use `[System.Text.UTF8Encoding]::new($false)`; in Python write `\r\n` explicitly.
- **`Build\Notepad3.ini`, `Build\minipath.ini` are UTF-8 WITH BOM** (`EF BB BF`). Preserve it.

### Adding a string resource

1. `#define IDS_MUI_XXX <id>` in `language\common_res.h` (13xxx errors/warnings, 14xxx info/prompts).
2. Add the English string to `language\np3_en_us\strings_en_us.rc`.
3. Add the same English text as placeholder to all other 25 locale files (translators update later).
4. Display via `InfoBoxLng()` / `MessageBoxLng()`; check with `IsYesOkay()`. `Settings.MuteMessageBeep` controls silent vs. sound — provide both paths.

## Portable INI Design

- INI sits beside the exe. No registry. Runs on defaults if no INI exists (user creates via "Save Settings Now").
- **Admin redirect**: `Notepad3.ini=<path>` in `[Notepad3]` redirects to a per-user path (up to 2 levels). Redirect targets ARE auto-created via `CreateIniFileEx()`.
- `Paths.IniFile` = active writable INI; `Paths.IniFileDefault` = recovery fallback.
- Init flow: `FindIniFile()` → `TestIniFile()` → `CreateIniFile()` → `LoadSettings()`.
- **`SettingsVersion` defaults to `CFG_VER_CURRENT`** when missing — empty/new INI gets current defaults, not legacy treatment.
- **`bIniFileFromScratch`** is set when INI is 0 bytes, cleared after `SaveAllSettings()`. While set, `MuiLanguage.c` suppresses writing `PreferredLanguageLocaleName` to keep fresh INIs clean.
- **Empty lexer sections are pruned** in `Style_ToIniSection()` via `IniSectionGetKeyCount()` after `Style_CanonicalSectionToIniCache()` establishes order — only sections with non-default styles persist.
- MiniPath follows the same pattern (`minipath\src\Config.cpp`).
- **New `Settings2` (or other INI) params must be documented as commented entries in `Build\Notepad3.ini`.**

### Save/Load macros (`src\Config\Config.cpp`)

- **`Encoding_MapIniSetting` is asymmetric.** CPI constants (`CPI_UTF8=6`, `CPI_OEM=1`, …) don't equal INI values (`3`, `5`, …). `Encoding_MapIniSetting(true, val)` = INI→CPI (load); `(false, val)` = CPI→INI (save). Passing CPI with `bLoad=true` produces wrong results. Reference: `MRU_Save()` uses `false`.
- **Defaults that depend on `DefaultEncoding`** (e.g. `SkipANSICodePageDetection`, `LoadASCIIasUTF8`) must be recalculated in `_SaveSettings()` before `SAVE_VALUE_IF_NOT_EQ_DEFAULT` fires, since encoding can change at runtime. See the `bCurrentEncUTF8` block.

## File I/O

- `FileSave()` / `FileLoad()` (in `Notepad3.c`) → `FileIO()` → `EditSaveFile()` / `EditLoadFile()` (in `Edit.c`).
- Atomic save via temp file + `ReplaceFileW` controlled by `Settings2.AtomicFileSave`.
- Error codes land in `Globals.dwLastError`; check `ERROR_ACCESS_DENIED`, `ERROR_PATH_NOT_FOUND` before falling back to generic error.
- **File watching** (`InstallFileWatching()`) uses `FindFirstChangeNotificationW` on the parent directory. Must `InstallFileWatching(false)` before save and `InstallFileWatching(true)` after.

## Long-Path / PathLib wrappers

Never call Win32 file APIs directly with `Path_Get(hpth)`. Use the PathLib wrappers — they apply the `\\?\` prefix conditionally (only when `RtlAreLongPathsEnabled()` is false AND path ≥ 260 chars):

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

Use `Path_StrgComparePath()` (supports normalization; `CompareStringOrdinal` under the hood — locale-independent, case-insensitive). For raw wide strings use `CompareStringOrdinal(s1, -1, s2, -1, TRUE)`; **never** `_wcsicmp` / `_wcsnicmp` (locale-dependent).

### Creating directories

Use `Path_CreateDirectoryEx(hpth)`. Success: `SUCCEEDED(hr) || (hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS))`. Reference pattern in `CreateIniFile()` (`Config.cpp`).

## PCRE2

`scintilla\pcre2\scintilla\PCRE2RegExEngine.cxx` compiled with `SCI_OWNREGEX` overrides Scintilla's built-in regex. Entry points: `FindText`, `SubstituteByPosition`, `convertReplExpr` (normalizes `\1`-`\9`→`$1`-`$9`, processes `\n\t\xHH\uHHHH`), `translateRegExpr` (`\<`/`\>` → lookarounds, `\uHHHH` → `\x{HHHH}`). Standalone `RegExFind` (exported C) used by `EditURLDecode`.

Replacement backref syntax: `$0`-`$99`, `\0`-`\9`, `${name}`, `${+name}`.

URL hotspot regex: `HYPLNK_REGEX_FULL` macro in `src\Edit.c` — matches `https?://`, `ftp://`, `file:///`, `file://`, `mailto:`, `www.`, `ftp.`. Trailing group excludes `.,:?!` so URLs don't absorb sentence punctuation.

## DarkMode

`src\DarkMode\` — Windows 10/11 dark mode via IAT hooks on uxtheme/user32 (stub DLLs included).

## ARM64

Supported: Win32 (x86), x64, x64_AVX2, ARM64. **ARM 32-bit is not supported** — the `Release|ARM` solution config maps to Win32.

- Both ARM64 and x64 define `_WIN64`. Use `_M_ARM64` (or helper `NP3_BUILD_ARM64` in `TypeDefs.h`) to distinguish.
- Rendering default: `SC_TECHNOLOGY_DIRECTWRITERETAIN` (2) instead of `SC_TECHNOLOGY_DIRECTWRITE` (1) — preserves Direct2D back buffer, avoids flicker on Qualcomm Adreno + Win11 25H2 DWM. Main window also gets `WS_EX_COMPOSITED`. User can override via `RenderingTechnology` / View menu.
- Build config: `CETCompat=false` (CET is x86/x64 only), `TargetMachine=MachineARM64`, `_WIN64` defined. Fix scripts: `Build\scripts\FixARM64{CETCompat,CrossCompile,OutDir}.ps1`.
- grepWin: no native ARM64 — uses x64 exe via emulation (`#if defined(_M_ARM64)` in `Notepad3.c`).
- `MsgThemeChanged()` wraps bar recreate / lexer reset / restyle in `WM_SETREDRAW FALSE/TRUE` and does a single `RedrawWindow()` at end. DarkMode `RedrawWindow()` in `ListViewUtil.hpp` omits `RDW_ERASE` to avoid background flash.

## Conventions

- **Formatting**: LLVM-based `.clang-format` in `src\` — 4-space indent, Stroustrup braces, left-aligned pointers, no column limit, no include sorting. `.editorconfig` enforces UTF-8/CRLF; Lexilla code uses tabs (upstream preserved).
- **Strings**: `strsafe.h` throughout; deprecated string functions disabled.
- **Types**: `DocPos` / `DocPosU` / `DocLn` (not raw `int`). `cpi_enc_t` for encodings. `HSTRINGW` / `HPATHL` (opaque) instead of raw `WCHAR*`. `NOMINMAX` is global — use `min()`/`max()` or typed equivalents.
- **Scintilla**: always use `SciCall.h` wrappers. Add missing wrappers there. Naming: `DeclareSciCall{V|R}{0|01|1|2}` — V=void, R=return; 0/1/2 = param count; `01` = lParam-only. The `msg` arg is the suffix after `SCI_`.
- **Global state**: use existing structs (`Globals`, `Settings`, `Settings2`, `Flags`, `Paths`); don't add new globals.
- **Undo/redo**: use `_BEGIN_UNDO_ACTION_` / `_END_UNDO_ACTION_` macros (`Notepad3.h`) for grouping; they also throttle notifications during bulk edits.

### WriteAccessBuf — dangling pointer anti-pattern

**NEVER** use a pointer from `Path_WriteAccessBuf()` / `StrgWriteAccessBuf()` after any operation that may reallocate/swap the underlying buffer of the SAME handle. Buffer-invalidating ops include: `Path_CanonicalizeEx`, `Path_Swap` / `StrgSwap`, `Path_ExpandEnvStrings` / `ExpandEnvironmentStrgs`, `Path_Append` / `Path_Reset`, `StrgCat` / `StrgInsert` / `StrgFormat` / `StrgReset`, `Path_NormalizeEx` / `Path_AbsoluteFromApp` / `Path_RelativeToApp`.

After any of these:
- Read-only: use `Path_Get(h)` / `StrgGet(h)` (always current).
- Read-write: re-obtain via `Path_WriteAccessBuf(h, 0)` / `StrgWriteAccessBuf(h, 0)` (size 0 = no resize, returns current pointer).

## Python Environment

`.venv\` (Python 3.14) for scripting tasks — bulk locale edits, code generation, etc.

```bash
.venv/Scripts/python.exe <script.py>
.venv/Scripts/pip.exe install <package>
```

System Python is not installed; `python3` fails. Python beats sed/perl under Cygwin for literal string insertions (reliable `\r\n`).
