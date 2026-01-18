# Notepad3 TODO

## High Priority

- [ ] **Scintilla/Lexilla Update** - Upgrade to latest versions (5.5.8/5.4.6)
  - See [research/scintilla-lexilla-upgrade.md](research/scintilla-lexilla-upgrade.md)

## Medium Priority

- [x] **DirectWrite Font Variant Bug** - ✅ Fixed with ResolveFontFace() patch
  - Uses IDWriteGdiInterop to resolve font face → family + weight/style/stretch
  - See [scintilla/np3_patches/001_directwrite_font_resolution.md](../scintilla/np3_patches/001_directwrite_font_resolution.md)
- [ ] **SVG Toolbar Icons** - Resolution-independent toolbar with dark/light mode
  - Auto-adapts to any DPI (100%-300%)
  - Dark/light mode color switching
  - Keep classic bitmap as fallback (ToolBarTheme 0/1)
  - See [research/svg-toolbar.md](research/svg-toolbar.md)
- [ ] **EditorConfig Integration** - Apply `.editorconfig` settings on file open
  - Use [editorconfig-core-c](https://github.com/editorconfig/editorconfig-core-c) library
  - Support: indent_style, indent_size, tab_width, end_of_line, charset
  - Effort: ~2-4 weeks
  - See [research/editorconfig-integration.md](research/editorconfig-integration.md)
- [ ] Installer testing on various Windows versions
- [ ] Language file updates

## Low Priority

- [ ] Documentation updates
