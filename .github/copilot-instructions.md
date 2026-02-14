# Copilot Instructions for Notepad3

## Build

Notepad3 is a Windows-only C/C++ application built with Visual Studio 2022 (toolset v145) and MSBuild. The solution file is `Notepad3.sln`.

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

Tests live in `test\` and run via AppVeyor CI:

```cmd
cd test
TestFileVersion.cmd       # Verifies built binary version info
TestAhkNotepad3.cmd       # AutoHotkey-based GUI tests (requires AutoHotkey)
```

### CI

GitHub Actions workflow at `.github/workflows/build.yml` builds all four platforms (Win32, x64, x64_AVX2, ARM64) in Release on `windows-2022` runners, triggered on push/PR to master.

## Architecture

Notepad3 is a Win32 desktop text editor built on the **Scintilla** editing component with **Lexilla** for syntax highlighting. It ships with two companion tools: **MiniPath** (file browser) and **grepWinNP3** (file search/grep).

### Core modules (in `src\`)

- **Notepad3.c/h** — Application entry point (`wWinMain`), window procedure, global state structs (`GLOBALS_T`, `SETTINGS_T`, `FLAGS_T`, `PATHS_T`)
- **Edit.c/h** — Text manipulation (find/replace, encoding conversion, clipboard, indentation, sorting)
- **Styles.c/h** — Scintilla styling, lexer selection, theme management
- **Dialogs.c/h** — All dialog boxes and UI interactions
- **Encoding.c/h** — Character encoding detection and conversion
- **SciCall.h** — Type-safe inline wrappers for Scintilla direct function calls (avoids `SendMessage` overhead)
- **DynStrg.c/h** — Custom dynamic wide-string type (`HSTRINGW`) with automatic buffer management
- **PathLib.c/h** — Path manipulation via opaque `HPATHL` handle
- **TypeDefs.h** — Core type definitions (`DocPos`, `DocLn`, `cpi_enc_t`), Windows version targeting, compiler macros

### Vendored dependencies

- **`scintilla\`** — Scintilla editor component with Notepad3-specific patches in `np3_patches\`
- **`lexilla\`** — Lexilla syntax highlighting engine with custom patches
- **`src\uchardet\`** — Character encoding detector
- **`src\tinyexpr\` / `src\tinyexprcpp\`** — Expression evaluator for statusbar
- **`src\uthash\`** — Hash table library (C macros)
- **Boost Regex & IOStreams** — Managed via `vcpkg.json`

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

### Adding a new syntax lexer

1. Create `src\StyleLexers\styleLexNEW.c` following existing lexer patterns
2. Define `EDITLEXER` struct with lexer ID, name, extensions, keywords, and styles
3. Register it in `Styles.c` lexer array
4. Add localization string IDs to resource files

### Global state

Application state is centralized in global structs in `Notepad3.c` — `Globals`, `Settings`, `Settings2`, `Flags`, `Paths`. Prefer accessing these through their defined interfaces rather than adding new globals.
