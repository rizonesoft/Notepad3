# SVG Toolbar Implementation

## Overview

Replace bitmap toolbar icons with SVG for resolution-independent rendering and dark/light mode support.

## Problem

Current bitmap-based toolbar has issues:
- 24px icons render small on 200%+ DPI displays
- Scaling causes blur artifacts
- Separate bitmap files needed for dark mode
- Multiple sizes required (16, 24, 32, 48px)

## Solution

Use SVG icons rasterized at runtime to exact target DPI.

## Benefits

| Feature | Bitmap (Current) | SVG (Proposed) |
|---------|------------------|----------------|
| DPI Scaling | Stretched/blurry | Perfect at any DPI |
| Dark Mode | Separate bitmaps | Runtime color swap |
| File Size | Multiple sizes | Single file per icon |
| Quality at 200% | 24px stretched to 48px | Native 48px render |

## Implementation

### 1. Add NanoSVG Library

Single-header C library (~400 lines), MIT license.
- `nanosvg.h` - SVG parser
- `nanosvgrast.h` - Rasterizer

### 2. New Files

```
src/SvgLoader.h    - API declarations
src/SvgLoader.c    - SVG loading and rasterization
res/toolbar_svg/   - SVG icon files
```

### 3. Key Functions

```c
// Load SVG and render to HBITMAP at specified size
HBITMAP LoadSVGAsBitmap(LPCWSTR svgPath, int width, int height, COLORREF fillColor);

// Create ImageList from SVG files for toolbar
HIMAGELIST CreateImageListFromSVG(HWND hWnd, LPCWSTR svgDir, int baseSize);
```

### 4. Modified Files

| File | Changes |
|------|---------|
| `Notepad3.c` | Extend `CreateBars()` to support SVG loading |
| `TypeDefs.h` | Add `ToolBarTheme = 3` for modern SVG |
| `Dialogs.c` | Add "Modern (SVG)" option in toolbar settings |
| `resource.h` | New resource IDs if embedding SVG |

### 5. Theme Selection

Extend `Settings.ToolBarTheme`:
- `0` = Classic Small (16px bitmap)
- `1` = Classic Large (24px bitmap)
- `2` = Custom bitmap (user-provided)
- `3` = Modern SVG (auto dark/light)

### 6. Dark/Light Mode

```c
COLORREF GetToolbarIconColor() {
    return UseDarkMode() ? RGB(0xE0, 0xE0, 0xE0)  // Light icons on dark
                         : RGB(0x20, 0x20, 0x20); // Dark icons on light
}
```

SVG fill colors replaced at render time before rasterization.

## Effort Estimate

| Phase | Duration |
|-------|----------|
| NanoSVG integration | 2-3 days |
| SVG loader implementation | 3-4 days |
| CreateBars() modification | 2-3 days |
| SVG icon creation (31 icons) | 3-5 days |
| Dark mode support | 1-2 days |
| Testing & polish | 2-3 days |
| **Total** | **2-3 weeks** |

## Fallback Strategy

1. Check if SVG files exist in `res/toolbar_svg/`
2. If missing or theme < 3, use existing bitmap logic
3. User can always select Classic themes in settings

## References

- NanoSVG: https://github.com/memononen/nanosvg
- Current toolbar code: `Notepad3.c:3114` (`CreateBars()`)
- DPI handling: `Notepad3.c:3036` (`Scintilla_GetWindowDPI()`)
