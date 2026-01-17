# Notepad3 Scintilla Patches

This directory documents **all** modifications made to upstream Scintilla 5.5.7 for Notepad3.

## Baseline

**Upstream Version**: Scintilla 5.5.7 (official release from scintilla.org)

## Patch Registry

| ID | Patch File | Target File | Description |
|----|------------|-------------|-------------|
| 001 | `001_Scintilla_h_exports.patch` | `include/Scintilla.h` | DLL exports: WRECT, DPI functions, IME, Strikethrough, KEYWORDSET_MAX, SCFIND_DOT_MATCH_ALL |
| 002 | `002_ScintillaTypes_DotMatchAll.patch` | `include/ScintillaTypes.h` | `FindOption::DotMatchAll = 0x1000` for Oniguruma regex |
| 003 | `003_Sci_Position_cstdint.patch` | `include/Sci_Position.h` | C++ `<cstdint>` include guard |
| 004 | `004_ILoader_cxx_guard.patch` | `include/ILoader.h` | C++ header compatibility |
| 005 | `005_Platform_h_mods.patch` | `src/Platform.h` | AlphaBlend typedef, platform compatibility |
| 006 | `006_PlatWin_h_4param.patch` | `win32/PlatWin.h` | 4-param `AdjustWindowRectForDpi` signature (DPI fix) |
| 007 | `007_PlatWin_cxx_4param.patch` | `win32/PlatWin.cxx` | 4-param `AdjustWindowRectForDpi` implementation |
| 008 | `008_ScintillaWin_exports.patch` | `win32/ScintillaWin.cxx` | DPI/CodePage/DirectFunction exports, ALT-key drag |
| 009 | `009_ListBox_4param.patch` | `win32/ListBox.cxx` | Calls 4-param AdjustWindowRectForDpi |
| 010 | `010_ScintillaDLL_entry.patch` | `win32/ScintillaDLL.cxx` | DLL entry point modifications |
| 011 | `011_SurfaceD2D_mods.patch` | `win32/SurfaceD2D.cxx` | DirectWrite font rendering adjustments |

## Critical: DPI Scaling (Patches 006, 007, 009)

> ⚠️ **WARNING**: Upstream Scintilla 5.5.8 changed `AdjustWindowRectForDpi` from 4 parameters to 3 parameters. NP3 requires the 4-param version for proper DPI handling.

## Preserved Directories (Not Patched)

These directories are entirely NP3-custom and must be preserved during upgrades:

- **`scintilla/oniguruma/`** - Custom regex engine (Oniguruma integration)
- **`scintilla/*.vcxproj`** - Project files with NP3 defines (`SCI_OWNREGEX`, `NP3`, `ONIG_STATIC`)

## Upgrade Procedure

When upgrading Scintilla to a new version:

1. Download new upstream release
2. Copy ALL files from upstream (src/, include/, win32/)
3. Re-apply patches 001-011 in order
4. Preserve oniguruma/ directory (do not overwrite)
5. Preserve .vcxproj files (NP3 build settings)
6. Clean build and test DPI scaling at 100%, 150%, 200%
