# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Notepad3 is a Windows-only Win32 desktop text editor (C/C++) built on **Scintilla** (editing component) and **Lexilla** (syntax highlighting). It ships with companion tools **MiniPath** (file browser, Ctrl+M) and **grepWinNP3** (file search/grep, Ctrl+Shift+F). Licensed under BSD 3-Clause.

## Build Commands

```powershell
# NuGet restore (required before first build)
nuget restore Notepad3.sln

# Single platform builds
Build\Build_x64.cmd [Release|Debug]
Build\Build_Win32.cmd [Release|Debug]
Build\Build_ARM64.cmd [Release|Debug]
Build\Build_x64_AVX2.cmd [Release|Debug]

# All platforms at once
Build\BuildAll.cmd [Release|Debug]

# MSBuild directly (used by CI)
msbuild Notepad3.sln /m /p:Configuration=Release /p:Platform=x64

# Clean all outputs
Build\Clean.cmd
```

Default configuration is Release. Build scripts delegate to PowerShell in `Build\scripts\`.

### Versioning

Run `Version.ps1` before building to generate `src\VersionEx.h` from templates in `Versions\`. Format: `Major.YY.Mdd.Build` (build number persisted in `Versions\build.txt`).

### Tests

```cmd
cd test
TestFileVersion.cmd       # Verifies built binary version info
TestAhkNotepad3.cmd       # AutoHotkey-based GUI tests (requires AutoHotkey)
```

### CI

GitHub Actions (`.github/workflows/build.yml`) builds all four platforms (Win32, x64, x64_AVX2, ARM64) in Release on `windows-2022` runners, triggered on push/PR to master.

## Architecture

### Core Modules (`src\`)

| File | Purpose |
|------|---------|
| **Notepad3.c/h** | Entry point (`wWinMain`), window procedure (`MainWndProc`), global state structs (`Globals`, `Settings`, `Settings2`, `Flags`, `Paths`) |
| **Edit.c/h** | Text manipulation: find/replace (Oniguruma regex), encoding conversion, clipboard, indentation, sorting, bookmarks, folding, auto-complete |
| **Styles.c/h** | Scintilla styling, lexer selection, theme management, margin configuration |
| **Dialogs.c/h** | All dialog boxes, DPI-aware UI interactions, window placement |
| **Config/Config.cpp/h** | INI file management, settings loading/saving, MRU list |
| **Encoding.c/h** | Encoding detection and conversion (integrates uchardet) |
| **SciCall.h** | Type-safe inline wrappers for Scintilla direct function calls (avoids `SendMessage` overhead) |
| **DynStrg.c/h** | Custom dynamic wide-string type (`HSTRINGW`) with automatic buffer management |
| **PathLib.c/h** | Path manipulation via opaque `HPATHL` handle |
| **TypeDefs.h** | Core type definitions (`DocPos`, `DocLn`, `cpi_enc_t`), Windows version targeting, compiler macros |
| **MuiLanguage.c/h** | Multi-language UI support, language DLL loading |

### Window Hierarchy

```
MainWndProc (Notepad3.c)
  +-- Scintilla Edit Control (hwndEdit / IDC_EDIT)
  +-- Toolbar (via Rebar control)
  +-- Status Bar (16 configurable fields)
```

### Vendored Dependencies

| Directory | Library | Purpose |
|-----------|---------|---------|
| `scintilla\` | Scintilla 5.5.8 | Editor component (NP3 patches in `np3_patches\`, docs in `doc\`) |
| `lexilla\` | Lexilla 5.4.6 | Syntax highlighting (NP3 patches in `np3_patches\`, docs in `doc\`) |
| `scintilla\oniguruma\` | Oniguruma | Regex engine for find/replace |
| `src\uchardet\` | uchardet | Mozilla encoding detection |
| `src\tinyexpr\` / `src\tinyexprcpp\` | TinyExpr | Expression evaluator (statusbar) |
| `src\uthash\` | uthash | Hash table / dynamic array macros |
| `src\crypto\` | Rijndael/SHA-256 | AES-256 encryption |
| Boost (via `vcpkg.json`) | Boost Regex & IOStreams | Used by grepWinNP3 |

### Syntax Lexers (`src\StyleLexers\`)

50+ languages, each in a `styleLexXXX.c` file. All follow the `EDITLEXER` struct pattern from `EditLexer.h`:

```c
EDITLEXER lexXXX = {
    SCLEX_XXX,          // Scintilla lexer ID
    "lexerName",        // Lexilla lexer name (case-sensitive)
    IDS_LEX_XXX_STR,    // Resource string ID
    L"Config Name",     // INI section name
    L"ext1; ext2",      // Default file extensions
    L"",                // Extension buffer (runtime)
    &KeyWords_XXX,      // Keyword lists
    { /* EDITSTYLE array */ }
};
```

To add a new lexer: create `styleLexNEW.c`, define the `EDITLEXER` struct, register in `Styles.c` lexer array, add localization string IDs to resource files.

### Localization (`language\`)

Resource-based MUI system with 27+ locales. Each locale has a `np3_LANG_COUNTRY\` directory with `.rc` files. Language packs are built as separate DLLs (separate projects in the solution).

### Configuration / Portable Design

- INI file alongside executable (`Notepad3.ini`), no registry usage
- **No auto-creation**: runs with defaults if no INI exists; user explicitly creates via "Save Settings Now"
- **Admin redirect**: `Notepad3.ini=<path>` in `[Notepad3]` section redirects to per-user path (up to 2 levels)
- Key paths: `Paths.IniFile` (active writable INI), `Paths.IniFileDefault` (fallback for recovery)
- INI init flow: `FindIniFile()` -> `TestIniFile()` -> `CreateIniFile()` -> `LoadSettings()`
- **MiniPath** follows the same portable INI and admin-redirect pattern (`minipath\src\Config.cpp`). Redirect targets are auto-created via `CreateIniFileEx()`.

### DarkMode (`src\DarkMode\`)

Windows 10/11 dark mode via IAT (Import Address Table) hooks. Includes stub DLLs for uxtheme and user32.

## Code Conventions

### Formatting

- LLVM-based `.clang-format` in `src\` — 4-space indentation, Stroustrup brace style, left-aligned pointers, no column limit, no include sorting
- `.editorconfig` enforces UTF-8/CRLF for source, 4-space indent for C/C++; Lexilla code uses tabs (preserved from upstream)
- String safety via `strsafe.h` throughout; deprecated string functions are disabled

### Type Conventions

- `DocPos` / `DocPosU` / `DocLn` for Scintilla document positions and line numbers (not raw `int`)
- `cpi_enc_t` for encoding identifiers
- `HSTRINGW` and `HPATHL` (opaque handle types) instead of raw `WCHAR*` buffers
- `NOMINMAX` is defined globally — use `min()`/`max()` macros or typed equivalents

### Scintilla Interaction

Always use `SciCall.h` wrappers (e.g. `SciCall_GetTextLength()`) instead of raw `SendMessage(hwnd, SCI_XXX, ...)`. Add missing wrappers to `SciCall.h` if needed.

Wrapper macros follow the naming `DeclareSciCall{V|R}{0|01|1|2}`:
- **V** = void return, **R** = has return value
- **0** = no params, **1** = one param (wParam), **2** = two params, **01** = lParam only (wParam=0)
- The `msg` argument is the suffix after `SCI_` (e.g. `UNDO` for `SCI_UNDO`)

```c
DeclareSciCallV0(Undo, UNDO);                                            // SciCall_Undo()
DeclareSciCallR0(GetTextLength, GETTEXTLENGTH, DocPos);                  // DocPos SciCall_GetTextLength()
DeclareSciCallV1(SetTechnology, SETTECHNOLOGY, int, technology);         // SciCall_SetTechnology(int)
DeclareSciCallR1(SupportsFeature, SUPPORTSFEATURE, bool, int, feature);  // bool SciCall_SupportsFeature(int)
```

### Global State

Application state is centralized in global structs (`Globals`, `Settings`, `Settings2`, `Flags`, `Paths`) defined in `Notepad3.c`. Access these through their defined interfaces rather than adding new globals.

### Undo/Redo Transactions

Use `_BEGIN_UNDO_ACTION_` / `_END_UNDO_ACTION_` macros (defined in `Notepad3.h`) to group Scintilla operations into single undo steps. These also handle notification limiting during bulk edits.
