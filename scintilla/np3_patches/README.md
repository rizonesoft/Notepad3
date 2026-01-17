# Notepad3 Scintilla Patches

This directory documents all modifications made to the upstream Scintilla library for Notepad3 compatibility.

## Patch Registry

| ID | Patch File | Target File | Description | Required |
|----|------------|-------------|-------------|----------|
| 001 | `001_Scintilla_h_NP3_exports.patch` | `include/Scintilla.h` | DLL exports, WRECT, IME, Strikethrough, Regex flags | ✅ Yes |
| 002 | `002_ScintillaTypes_DotMatchAll.patch` | `include/ScintillaTypes.h` | `FindOption::DotMatchAll` for Oniguruma | ✅ Yes |
| 003 | `003_ScintillaWin_exports.patch` | `win32/ScintillaWin.cxx` | DPI/CodePage/DirectFunction exports | ✅ Yes |
| 004 | `004_PlatWin_h_4param.patch` | `win32/PlatWin.h` | 4-param `AdjustWindowRectForDpi` signature | ✅ Yes |
| 005 | `005_PlatWin_cxx_4param.patch` | `win32/PlatWin.cxx` | 4-param `AdjustWindowRectForDpi` implementation | ✅ Yes |
| 006 | `006_ListBox_4param.patch` | `win32/ListBox.cxx` | Caller updated for 4-param API | ✅ Yes |

## Critical: DPI Scaling

> ⚠️ **WARNING**: Upstream Scintilla 5.5.8 changed `AdjustWindowRectForDpi` from 4 parameters to 3 parameters, hardcoding `WS_EX_WINDOWEDGE`. This breaks NP3 dialogs which require custom `dwExStyle` values. Patches 004 and 005 restore the 4-parameter version.

## Preserved Directories

- **`scintilla/oniguruma/`** - Entire directory is NP3 custom (regex engine)
- **`scintilla/*.vcxproj`** - Project files with NP3 defines (`SCI_OWNREGEX`, `NP3`, etc.)

## Upgrade Procedure

When upgrading Scintilla:

1. Upgrade `src/` files carefully (check Oniguruma compat)
2. Merge `include/Scintilla.h` (re-apply patches 001)
3. Merge `include/ScintillaTypes.h` (re-apply patch 002)
4. Merge `win32/` files (re-apply patches 003-005)
5. **TEST DPI SCALING** at 100%, 150%, 200%

## Version History

| Scintilla Version | Date | Notes |
|-------------------|------|-------|
| 5.5.7 | Current | Stable baseline |
| 5.5.8 | Pending | Incremental upgrade needed (DPI issues with bulk upgrade) |
