# Scintilla 5.5.8 Upstream Cherry-Pick Patches

All beneficial changes from Scintilla 5.5.8 have been applied to NP3's 5.5.7 baseline.

## ✅ All Changes Applied

| Patch | Description | Files Modified |
|-------|-------------|----------------|
| `001_undo_selection_redraw.patch` | Fix selection drawing after undo | `Editor.cxx` |
| `002_listbox_opaque_colors.patch` | Force autocomplete colors opaque | Already in NP3 |
| `003_listbox_bitmap_sizing.patch` | Fix bitmap sizing (no blank bg) | Already in NP3 |
| `004_unicode16_update.md` | Unicode 16 character categories | `CharacterCategoryMap.cxx` |
| `005_ptrdiff_alias.md` | PTRDIFF_DOESNT_ALIAS_INT option | `RunStyles.cxx` |
| `006_selection_serialization_format.patch` | New serialization format | `Selection.cxx`, `Selection.h` |
| `007_rectangular_indent_fix.patch` | Rectangular selection indent | `Editor.cxx` |

## Additional Minor Updates (Also Applied)

| File | Change |
|------|--------|
| `CallTip.cxx` | Removed macOS-specific default colors |
| `CaseConvert.cxx` | Unicode 16 case folding table updates |
| `CellBuffer.cxx` | `is_convertible_v` type check + `type_traits` include |
| `EditView.cxx` | Bidi data bounds check fix |

> [!NOTE]
> `PositionCache.cxx/h` was NOT updated - has `noexcept` specification changes
> that conflict with NP3's base class declarations.

## NP3-Protected Files (Not Updated)

These files have NP3 patches and retain their customizations:

- `win32/PlatWin.cxx` - 4-parameter `AdjustWindowRectForDpi`
- `win32/ScintillaWin.cxx` - DPI exports, popup menu control
- `src/Editor.cxx` - Zoom methods, strike style, drag policies (with 5.5.8 fixes merged in)

## Verification

Build completed successfully with exit code 0:
```
Notepad3.vcxproj -> R:\GitHub\Notepad3\Bin\Release_x64_v143\Notepad3.exe
```

