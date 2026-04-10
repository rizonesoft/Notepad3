# Copilot Instructions for Notepad3

## Build

Notepad3 is a Windows-only C/C++ application built with Visual Studio 2026 (toolset v145) and MSBuild. The solution file is `Notepad3.sln`.

### Build commands

```powershell
# Full build (all platforms: Win32, x64, x64_AVX2, ARM64)
Build\BuildAll.cmd [Release|Debug]

# Single platform
Build\Build_x64.cmd [Release|Debug]
Build\Build_Win32.cmd [Release|Debug]
Build\Build_ARM64.cmd [Release|Debug]
Build\Build_x64_AVX2.cmd [Release|Debug]

# MSBuild directly (used by CI)
msbuild Notepad3.sln /m /p:Configuration=Release /p:Platform=x64

# Clean
Build\Clean.cmd

# NuGet restore (required before first build)
nuget restore
```

Default configuration is Release. The build scripts delegate to PowerShell scripts in `Build\scripts\`.

### Versioning

Run `Version.ps1` before building to generate `src\VersionEx.h` from templates in `Versions\`. Version format is `Major.YY.Mdd.Build` where the build number is persisted in `Versions\build.txt`.

### Tests

Tests live in `test\` and run via GitHub Actions CI (Win32 and x64 jobs):

```cmd
cd test
TestFileVersion.cmd       # Verifies built binary version info
TestAhkNotepad3.cmd       # AutoHotkey-based GUI tests (requires AutoHotkey)
```

### CI

GitHub Actions workflow at `.github/workflows/build.yml` builds all four platforms (Win32, x64, x64_AVX2, ARM64) in Release on `windows-2022` runners, triggered on push/PR to master.

## Architecture

Notepad3 is a Win32 desktop text editor built on the **Scintilla** editing component with **Lexilla** for syntax highlighting. It ships with the companion tool **MiniPath** (file browser) and integrates with the external **grepWin** tool (file search/grep) via pre-built portable executables.

### Core modules (in `src\`)

- **Notepad3.c/h** — Application entry point (`wWinMain`), window procedure, global state structs (`GLOBALS_T`, `SETTINGS_T`, `FLAGS_T`, `PATHS_T`)
- **Edit.c/h** — Text manipulation: find/replace (PCRE2 regex), encoding conversion, clipboard, indentation, sorting, bookmarks, folding, auto-complete
- **Styles.c/h** — Scintilla styling, lexer selection, theme management
- **Dialogs.c/h** — All dialog boxes and UI interactions
- **Encoding.c/h** — Character encoding detection and conversion
- **SciCall.h** — Type-safe inline wrappers for Scintilla direct function calls (avoids `SendMessage` overhead)
- **DynStrg.c/h** — Custom dynamic wide-string type (`HSTRINGW`) with automatic buffer management
- **PathLib.c/h** — Path manipulation via opaque `HPATHL` handle
- **TypeDefs.h** — Core type definitions (`DocPos`, `DocLn`, `cpi_enc_t`), Windows version targeting, compiler macros
- **MuiLanguage.c/h** — Multi-language UI support, language DLL loading

### Vendored dependencies

- **`scintilla\`** — Scintilla editor component with Notepad3-specific patches in `np3_patches\`
- **`scintilla\doc\`** - Scintilla documentation offline
- **`lexilla\`** — Lexilla syntax highlighting engine with custom patches
- **`lexilla\doc\`** — Lexilla documentation offline
- **`scintilla\pcre2\`** — PCRE2 10.47 regex engine for find/replace (replaced archived Oniguruma)
- **`src\uchardet\`** — Character encoding detector
- **`src\tinyexpr\` / `src\tinyexprcpp\`** — Expression evaluator for statusbar
- **`src\uthash\`** — Hash table library (C macros)
- **`src\crypto\`** — Rijndael/SHA-256 for AES-256 encryption

### grepWin Integration (`grepWin\`)

grepWin is an **external** file search/grep tool — it is **not** built from source as part of Notepad3. Pre-built portable executables and translation files are stored in the repository:

- **`grepWin\portables\`** — `grepWin-x86_portable.exe`, `grepWin-x64_portable.exe`, `LICENSE.txt`
- **`grepWin\translations\`** — `*.lang` translation files (e.g. `German.lang`, `French.lang`)

At runtime (`src\Dialogs.c`), Notepad3 searches for grepWin in this order:
1. `Settings2.GrepWinPath` (user-configured INI setting)
2. `<ModuleDirectory>\grepWin\grepWin-x64_portable.exe` (or x86) — portable layout
3. `%APPDATA%\Rizonesoft\Notepad3\grepWin\` — installed layout

Language mapping (`src\MuiLanguage.c`): `grepWinLangResName[]` maps Notepad3 locale names (e.g. `de-DE`) to grepWin `.lang` filenames (e.g. `German.lang`). The language file path is written to `grepwin.ini` before launching.

Portable build scripts (`Build\make_portable_*.cmd`) package grepWin into a `grepWin\` subdirectory in the archive containing both portable executables, the license, and all `*.lang` translations.

### Syntax highlighting lexers (`src\StyleLexers\`)

Each language has its own `styleLexXXX.c` file (~50+ languages). All lexers follow the `EDITLEXER` struct pattern defined in `EditLexer.h`:

```c
EDITLEXER lexXXX = {
    SCLEX_XXX,          // Scintilla lexer ID
    "lexerName",        // Lexilla lexer name (case-sensitive)
    IDS_LEX_XXX_STR,    // Resource string ID
    L"Config Name",     // INI section name
    L"ext1; ext2",      // Default file extensions
    L"",                // Extension buffer (runtime)
    &KeyWords_XXX,      // Keyword lists
    { /* EDITSTYLE array — must be last member */ }
};
```

### Localization (`language\`)

Resource-based MUI system with 27+ locales. Each locale has a directory `np3_LANG_COUNTRY\` containing resource `.rc` files. Language DLLs are built as separate projects in the solution.

### Adding String Resources

1. Add `#define IDS_MUI_XXX <id>` to `language\common_res.h` (use next available ID in the appropriate range: 13xxx for errors/warnings, 14xxx for info/prompts)
2. Add the English string to `language\np3_en_us\strings_en_us.rc` in the matching `STRINGTABLE` block
3. Add the same English text as placeholder to all other 25 locale `strings_*.rc` files (translators update later)
4. Use `InfoBoxLng()` / `MessageBoxLng()` with `MB_YESNO`, `MB_ICONWARNING`, etc. to display; check result with `IsYesOkay()`
5. `Settings.MuteMessageBeep` controls whether to use `InfoBoxLng` (silent) or `MessageBoxLng` (with sound) — always provide both paths

### File I/O Flow

- **`FileSave()`** (`src\Notepad3.c`) — Main save dispatcher. Handles Save, Save As, Save Copy.
  Calls `FileIO()` → `EditSaveFile()` (`src\Edit.c`)
- **`FileLoad()`** (`src\Notepad3.c`) — Main load dispatcher.
  Calls `FileIO()` → `EditLoadFile()` (`src\Edit.c`)
- **`EditSaveFile()`** supports atomic save (temp file + `ReplaceFileW`) controlled by `Settings2.AtomicFileSave`
- **Error handling**: `Globals.dwLastError` holds the Win32 error code after failed I/O.
  `FileSave()` checks specific codes (`ERROR_ACCESS_DENIED`, `ERROR_PATH_NOT_FOUND`) before falling back to generic error.
- **File watching**: `InstallFileWatching()` uses `FindFirstChangeNotificationW` on the parent directory.
  Must be stopped before save (`InstallFileWatching(false)`) and restarted after (`InstallFileWatching(true)`).

### PCRE2 Regex Engine (`scintilla\pcre2\`)

PCRE2 10.47 replaced the archived Oniguruma library. The Scintilla integration lives in `scintilla\pcre2\scintilla\PCRE2RegExEngine.cxx`, compiled with `SCI_OWNREGEX` to override Scintilla's built-in regex.

Key components:
- **`PCRE2RegExEngine::FindText`** — Scintilla regex search (pattern matching via `pcre2_match`)
- **`PCRE2RegExEngine::SubstituteByPosition`** — Regex replacement with group references
- **`PCRE2RegExEngine::convertReplExpr`** — Normalizes replacement strings: converts `\1`-`\9` to `$1`-`$9`, processes escape sequences (`\n`, `\t`, `\xHH`, `\uHHHH`)
- **`PCRE2RegExEngine::translateRegExpr`** — Translates Scintilla regex extensions: `\<`/`\>` word boundaries → lookarounds, `\uHHHH` → `\x{HHHH}`
- **`RegExFind`** (exported C function) — Standalone regex find used by `EditURLDecode` in `Edit.c`; wraps `SimplePCRE2Engine`

Replacement string backreference syntax (both flavors supported for backward compatibility):
- `$0`-`$99` and `\0`-`\9` — numbered group references
- `${name}` / `${+name}` — named group references
- Escape sequences: `\n`, `\t`, `\r`, `\\`, `\xHH`, `\uHHHH`

URL hotspot regex is defined at `src\Edit.c:108` (`HYPLNK_REGEX_FULL` macro). It matches `https?://`, `ftp://`, `file:///`, `file://`, `mailto:`, `www.`, `ftp.` schemes. The trailing group excludes punctuation (`.,:?!`) so URLs don't absorb sentence-ending characters.

### DarkMode (`src\DarkMode\`)

Windows 10/11 dark mode via IAT (Import Address Table) hooks. Includes stub DLLs for uxtheme and user32.

### ARM64 Platform Considerations

**Supported platforms**: Win32 (x86), x64, x64_AVX2, ARM64. ARM 32-bit is **not** supported (the `Release|ARM` solution config maps to Win32).

#### Architecture detection

Use `#if defined(_M_ARM64)` or the helper macro `NP3_BUILD_ARM64` (defined in `src\TypeDefs.h`) for ARM64-specific code paths. **Important**: both ARM64 and x64 define `_WIN64`, so use `_M_ARM64` when you need to distinguish ARM64 from x64.

#### ARM64 rendering defaults

ARM64 defaults to `SC_TECHNOLOGY_DIRECTWRITERETAIN` (value 2) instead of `SC_TECHNOLOGY_DIRECTWRITE` (value 1) to preserve the Direct2D back buffer between frames. This avoids flickering on Qualcomm Adreno GPUs and the Win11 25H2 DWM compositor. The main window also uses `WS_EX_COMPOSITED` on ARM64 for system-level double-buffering. Users can override via `RenderingTechnology` in the INI file or the View menu.

#### ARM64 build configuration

- `CETCompat` must be `false` for ARM64 (CET is x86/x64 only)
- `TargetMachine` must be `MachineARM64` in all ARM64 linker sections
- `_WIN64` must be defined in preprocessor definitions for all ARM64 configurations
- Build fix scripts in `Build\scripts\`: `FixARM64CETCompat.ps1`, `FixARM64CrossCompile.ps1`, `FixARM64OutDir.ps1`

#### GrepWin on ARM64

No native ARM64 grepWin build exists. The ARM64 build uses `grepWin-x64_portable.exe` which runs via x64 emulation on Windows ARM64. The binary selection in `src\Notepad3.c` uses `#if defined(_M_ARM64)` to handle this explicitly.

#### Theme change flickering prevention

`MsgThemeChanged()` in `src\Notepad3.c` wraps all heavy operations (bar recreation, lexer reset, restyling) in `WM_SETREDRAW FALSE/TRUE` to suppress intermediate repaints and performs a single `RedrawWindow()` at the end. DarkMode `RedrawWindow()` calls in `ListViewUtil.hpp` omit `RDW_ERASE` to avoid background erase flashes during theme transitions.

## Conventions

### Code style

- **Formatting**: LLVM-based `.clang-format` in `src\` — 4-space indentation, Stroustrup brace style, left-aligned pointers, no column limit, no include sorting
- **Editor config**: `.editorconfig` enforces UTF-8/CRLF for source files, 4-space indentation for C/C++; Lexilla code uses tabs (preserved from upstream)
- **String safety**: Uses `strsafe.h` throughout; deprecated string functions are disabled

### Type conventions

- Use `DocPos` / `DocPosU` / `DocLn` for Scintilla document positions and line numbers (not raw `int`)
- Use `cpi_enc_t` for encoding identifiers
- Use `HSTRINGW` and `HPATHL` (opaque handle types) instead of raw `WCHAR*` buffers for dynamic strings and paths
- `NOMINMAX` is defined globally — use `min()`/`max()` macros or typed equivalents

### Scintilla interaction

Always use `SciCall.h` wrappers (e.g., `SciCall_GetTextLength()`) instead of raw `SendMessage(hwnd, SCI_XXX, ...)`. The wrappers use Scintilla's direct function pointer for performance.
Add missing wrapper calls to `SciCall.h` if needed.

#### SciCall.h wrapper macros

Wrappers are declared using macros. The naming convention is `DeclareSciCall{V|R}{0|01|1|2}`:

- **V** = void return, **R** = has return value
- **0** = no parameters, **1** = one parameter (wParam), **2** = two parameters (wParam + lParam)
- **01** = optional second parameter only (wParam=0, lParam=var) — used when the SCI message takes lParam but not wParam

```c
// Examples:
DeclareSciCallV0(Undo, UNDO);                                          // SciCall_Undo()
DeclareSciCallV1(SetTechnology, SETTECHNOLOGY, int, technology);       // SciCall_SetTechnology(int)
DeclareSciCallV2(ScrollVertical, SCROLLVERTICAL, DocLn, docLn, int, subLn); // SciCall_ScrollVertical(DocLn, int)
DeclareSciCallR0(GetTextLength, GETTEXTLENGTH, DocPos);                // DocPos SciCall_GetTextLength()
DeclareSciCallR1(SupportsFeature, SUPPORTSFEATURE, bool, int, feature); // bool SciCall_SupportsFeature(int)
```

The `msg` argument is the suffix after `SCI_` (e.g., `UNDO` for `SCI_UNDO`).

### Scintilla / Lexilla versions

The vendored Scintilla (5.5.8) and Lexilla (5.4.6) have Notepad3-specific patches in `scintilla\np3_patches\` and `lexilla\np3_patches\`. Version numbers are in `scintilla\version.txt` and `lexilla\version.txt`. When evaluating new Scintilla APIs, check the offline docs in `scintilla\doc\` and `lexilla\doc\`.

### Adding a new syntax lexer

1. Create `src\StyleLexers\styleLexNEW.c` following existing lexer patterns
2. Define `EDITLEXER` struct with lexer ID, name, extensions, keywords, and styles
3. Register it in `Styles.c` lexer array
4. Add localization string IDs to resource files

### Global state

Application state is centralized in global structs in `Notepad3.c` — `Globals`, `Settings`, `Settings2`, `Flags`, `Paths`. Prefer accessing these through their defined interfaces rather than adding new globals.

### INI file / portable-app design

Notepad3 follows a **portable-app** design for its configuration file (`Notepad3.ini`):

- **No auto-creation on first run**: If no INI file is found, the application runs with defaults. The path is stored in `Paths.IniFileDefault` (not `Paths.IniFile`) so the user can explicitly create it via "Save Settings Now".
- **Admin redirect**: An administrator can place `Notepad3.ini=<path>` in `[Notepad3]` section of the app-directory INI to redirect to a per-user path. Up to 2 levels of redirect are supported. Redirect targets **are** auto-created (the admin intended them to exist).
- **Key paths**: `Paths.IniFile` = active writable INI (empty if none exists), `Paths.IniFileDefault` = fallback path for "Save Settings Now" recovery.
- **Configuration code**: All INI init logic lives in `src\Config\Config.cpp` — `FindIniFile()` → `TestIniFile()` → `CreateIniFile()` → `LoadSettings()`.
- **MiniPath** follows the same portable INI and admin-redirect pattern (`minipath\src\Config.cpp`). Redirect targets are auto-created via `CreateIniFileEx()`.
- **New parameters**: When adding new `Settings2` (or other INI) parameters, always document them as commented entries in `Build\Notepad3.ini`

### Creating Directories

Use `Path_CreateDirectoryEx(hpth)` (PathLib wrapper around `SHCreateDirectoryExW`) to recursively create directory trees. Check result: `SUCCEEDED(hr) || (hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS))`. See `CreateIniFile()` in `src\Config\Config.cpp` for the reference pattern.

### Long-Path Support / Win32 File API Wrappers

Notepad3 supports paths exceeding `MAX_PATH` (260 chars) on systems without the Windows 10 long-path registry opt-in by conditionally prepending the `\\?\` prefix. The static function `PrependLongPathPrefix()` in PathLib.c handles this.

**Rule: Never call Win32 file APIs directly with `Path_Get(hpth)`.** Use the PathLib wrapper functions instead — they internally apply the copy-prefix-call-release pattern:

| Wrapper | Win32 API |
|---------|-----------|
| `Path_CreateFile(hpth, ...)` | `CreateFileW` |
| `Path_DeleteFile(hpth)` | `DeleteFileW` |
| `Path_GetFileAttributes(hpth)` | `GetFileAttributesW` |
| `Path_GetFileAttributesEx(hpth, ...)` | `GetFileAttributesExW` |
| `Path_SetFileAttributes(hpth, ...)` | `SetFileAttributesW` |
| `Path_ReplaceFile(hpth_dest, src)` | `ReplaceFileW` |
| `Path_MoveFileEx(src, hpth_dest, ...)` | `MoveFileExW` |
| `Path_FindFirstFile(hpth, ...)` | `FindFirstFileW` |
| `Path_CreateDirectoryEx(hpth)` | `SHCreateDirectoryExW` |
| `Path_IsExistingFile(hpth)` | `GetFileAttributesW` + check |
| `Path_IsExistingDirectory(hpth)` | `GetFileAttributesW` + check |

The prefix is only added when `RtlAreLongPathsEnabled()` returns false AND the path is ≥ 260 chars. On modern Windows 10+ with the opt-in, these wrappers are effectively pass-through.

### Path Comparison

Use `Path_StrgComparePath()` for comparing file paths — it supports optional normalization and uses `CompareStringOrdinal` (locale-independent, case-insensitive). For raw wide-string path comparison, use `CompareStringOrdinal(s1, -1, s2, -1, TRUE)` instead of `_wcsicmp` or `_wcsnicmp` which are locale-dependent.

### Undo/Redo transactions

Use `_BEGIN_UNDO_ACTION_` / `_END_UNDO_ACTION_` macros (defined in `Notepad3.h`) to group Scintilla operations into single undo steps. These also handle notification limiting during bulk edits.

### WriteAccessBuf — Dangling Pointer Anti-Pattern

**NEVER use a pointer obtained from `Path_WriteAccessBuf()` / `StrgWriteAccessBuf()` after ANY operation that may reallocate or swap the underlying buffer of the SAME handle.** The pointer becomes dangling (use-after-free).

Buffer-invalidating operations:
- `Path_CanonicalizeEx(h, ...)` — calls `Path_Swap` internally
- `Path_Swap(h, ...)` / `StrgSwap(h, ...)`
- `Path_ExpandEnvStrings(h)` / `ExpandEnvironmentStrgs(h, ...)` — may realloc
- `Path_Append(h, ...)` / `Path_Reset(h, ...)` — may realloc
- `StrgCat(h, ...)` / `StrgInsert(h, ...)` / `StrgFormat(h, ...)` / `StrgReset(h, ...)` — may realloc
- `Path_NormalizeEx(h, ...)` / `Path_AbsoluteFromApp(h, ...)` / `Path_RelativeToApp(h, ...)` — may realloc/swap

Safe patterns after invalidation:
- **Read-only**: use `Path_Get(h)` or `StrgGet(h)` — always returns current buffer
- **Read-write**: re-obtain via `ptr = Path_WriteAccessBuf(h, 0)` / `ptr = StrgWriteAccessBuf(h, 0)` (size 0 = no resize, just returns current pointer)
