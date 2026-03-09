# Extended FontWeight and FontStyle Enums

## Description

Extends `ScintillaTypes.h` to include complete DirectWrite font weight and style enums,
enabling proper support for all font variants like "Cascadia Mono Light", "Iosevka Oblique", etc.

## Issues Addressed

- [#4150](https://github.com/rizonesoft/Notepad3/issues/4150) - Variable fonts support
- [#5455](https://github.com/rizonesoft/Notepad3/issues/5455) - Cascadia Mono Light rendering

## Changes

### FontWeight Enum (Extended)
Original had only: Normal(400), SemiBold(600), Bold(700)

Now includes all DWRITE_FONT_WEIGHT values:
- Thin (100), ExtraLight (200), Light (300), SemiLight (350)
- Normal (400), Medium (500)
- SemiBold (600), Bold (700), ExtraBold (800), Black (900), ExtraBlack (950)

### FontStyle Enum (New)
Matches DWRITE_FONT_STYLE:
- Normal (0) - Upright
- Oblique (1) - Mechanically slanted
- Italic (2) - True italic curves

## Files Modified

- `scintilla/include/ScintillaTypes.h`
