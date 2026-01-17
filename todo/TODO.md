# Notepad3 TODO

## High Priority

- [ ] **Scintilla/Lexilla Update** - Upgrade to latest versions (5.5.8/5.4.6)
  - See [research/scintilla-lexilla-upgrade.md](research/scintilla-lexilla-upgrade.md)

## Medium Priority

- [ ] **DirectWrite Font Variant Bug** - Cascadia Mono Light breaks rendering
  - Known Scintilla limitation with WSS font family model
  - Not fixed in 5.5.8 - may need custom patch
  - See: https://github.com/notepad-plus-plus/notepad-plus-plus/issues/12602
- [ ] **SVG Toolbar Icons** - Resolution-independent toolbar with dark/light mode
  - Auto-adapts to any DPI (100%-300%)
  - Dark/light mode color switching
  - Keep classic bitmap as fallback (ToolBarTheme 0/1)
  - See [research/svg-toolbar.md](research/svg-toolbar.md)
- [ ] Installer testing on various Windows versions
- [ ] Language file updates

## Low Priority

- [ ] Documentation updates
