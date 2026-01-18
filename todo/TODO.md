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
- [ ] **Autosave / Backup** - Periodic autosave and backup on save
  - Timer-based recovery to `%APPDATA%\Notepad3\recovery\`
  - Optional versioned backups: `.bak`, `.bak_1`, `.bak_2`, ...
  - Issues: [#1665](https://github.com/rizonesoft/Notepad3/issues/1665), [#370](https://github.com/rizonesoft/Notepad3/issues/370), [#512](https://github.com/rizonesoft/Notepad3/issues/512)
  - Effort: ~1-4 weeks
  - See [research/autosave-backup.md](research/autosave-backup.md)
- [ ] Installer testing on various Windows versions
- [ ] Language file updates

## Low Priority

- [ ] Documentation updates
- [ ] **Additional Syntax Highlighting** - New language lexers
  - Haskell: [#3035](https://github.com/rizonesoft/Notepad3/issues/3035) - Lexilla has `LexHaskell.cxx`
  - Racket: [#3035](https://github.com/rizonesoft/Notepad3/issues/3035) - Could use Lisp/Scheme lexer
  - Verilog HDL: [#4108](https://github.com/rizonesoft/Notepad3/issues/4108) - Lexilla has `LexVerilog.cxx`
  - JSON5: [#5411](https://github.com/rizonesoft/Notepad3/issues/5411) - Extend JSON lexer or add `.json5` extension
- [ ] **Custom Hyperlink Schemes** - User-defined URL protocol recognition
  - ed2k:// links: [#5405](https://github.com/rizonesoft/Notepad3/issues/5405)
  - Customizable via INI settings
