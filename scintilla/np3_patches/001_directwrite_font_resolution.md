# DirectWrite Font Face Resolution Patch

## Description

Adds `ResolveFontFace()` function to resolve font face names (e.g., "Cascadia Mono Light", 
"Iosevka Oblique") into proper DirectWrite parameters (family name + weight/style/stretch).

Uses `IDWriteGdiInterop::CreateFontFromLOGFONT()` to correctly resolve font parameters that 
would otherwise cause DirectWrite to fail silently and fall back to a generic system font.

## Issues Fixed

- [#4150](https://github.com/rizonesoft/Notepad3/issues/4150) - Variable fonts (Iosevka) fallback
- [#5410](https://github.com/rizonesoft/Notepad3/issues/5410) - Global Default Font not applying  
- [#5455](https://github.com/rizonesoft/Notepad3/issues/5455) - Cascadia Mono Light breaks rendering

## Based On

- [Scintilla Bug #2080](https://sourceforge.net/p/scintilla/bugs/2080/) - D2D bug with font family/face name
- [Scintilla MR #36](https://sourceforge.net/p/scintilla/code/merge-requests/36/) - DirectWrite font resolution

## Files Modified

- `scintilla/win32/SurfaceD2D.cxx`

## Changes

1. Added `ResolveFontFace()` function (~70 lines) before `FontDirectWrite` struct
2. Modified `FontDirectWrite` constructor to call `ResolveFontFace()` first, with fallback
