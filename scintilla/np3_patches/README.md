# Notepad3 Scintilla Patches

This directory contains complete documentation of all NP3 modifications to Scintilla.

## Current Baseline

- **NP3 Scintilla**: Based on 5.5.7 with NP3 patches
- **Target Upgrade**: 5.5.8

## Patch Files

### Upgrade Patches (vs 5.5.8)

These patches transform official Scintilla 5.5.8 → NP3 implementation:

| File | Patch | Lines | Purpose |
|------|-------|-------|---------|
| `include/Scintilla.h` | `upgrade_001_*` | 99 | DLL exports, DPI, IME, Strikethrough |
| `include/ScintillaTypes.h` | `upgrade_002_*` | 23 | DotMatchAll flag |
| `src/AutoComplete.cxx` | `upgrade_020_*` | 19 | Custom autocomplete |
| `src/CellBuffer.cxx` | `upgrade_021_*` | 29 | Buffer mods |
| `src/CellBuffer.h` | `upgrade_022_*` | 15 | Buffer header |
| `src/Document.h` | `upgrade_023_*` | 134 | Document types |
| `src/Editor.cxx` | `upgrade_024_*` | **247** | Core editor (24 patch sections) |
| `src/Editor.h` | `upgrade_025_*` | 54 | Editor header |
| `src/EditView.cxx` | `upgrade_026_*` | 39 | Edit view |
| `src/Partitioning.h` | `upgrade_027_*` | 14 | Partitioning |
| `src/Platform.h` | `upgrade_028_*` | 52 | Platform defs |
| `src/ScintillaBase.cxx` | `upgrade_029_*` | 41 | Base class |
| `src/ScintillaBase.h` | `upgrade_030_*` | 27 | Base header |
| `src/SplitVector.h` | `upgrade_031_*` | 15 | Vector util |
| `src/Style.cxx` | `upgrade_032_*` | 14 | Style impl |
| `src/Style.h` | `upgrade_033_*` | 14 | Style header |
| `src/ViewStyle.cxx` | `upgrade_034_*` | 116 | ViewStyle impl |
| `src/ViewStyle.h` | `upgrade_035_*` | 58 | ViewStyle header |
| `win32/ListBox.cxx` | `upgrade_050_*` | 15 | 4-param DPI |
| `win32/PlatWin.cxx` | `upgrade_051_*` | 37 | 4-param impl |
| `win32/PlatWin.h` | `upgrade_052_*` | 18 | 4-param sig |
| `win32/ScintillaWin.cxx` | `upgrade_053_*` | **380** | Exports (34 patch sections) |

## Preserved Directories

Do NOT overwrite during upgrades:

- **`scintilla/oniguruma/`** - Custom regex engine
- **`scintilla/*.vcxproj`** - NP3 build settings

## Upgrade Procedure (5.5.7 → 5.5.8)

1. Copy ALL files from official 5.5.8 to `scintilla/`
2. Apply upgrade patches: `git apply upgrade_*.patch`
3. Preserve `oniguruma/` directory
4. Preserve `.vcxproj` files
5. Clean build and test DPI scaling

## NP3 Patch Markers

All intentional patches are marked with:
```
// >>>>>>>>>>>>>>>   BEG NON STD SCI PATCH   >>>>>>>>>>>>>>>
// code here
// <<<<<<<<<<<<<<<   END NON STD SCI PATCH   <<<<<<<<<<<<<<<
```
