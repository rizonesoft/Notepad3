# Notepad3 TODO

## High Priority

- [x] **Scintilla/Lexilla Update** - ✅ Upgraded to 5.5.8/5.4.6 parity
  - Applied incrementally to preserve NP3 patches (DPI, Oniguruma, DirectWrite)
  - See [scintilla/np3_patches/upstream_558/](../scintilla/np3_patches/upstream_558/)
- [x] **BUG: FileSave() nested call** - ✅ Fixed in commit 658fb2f19
  - Issue: [#5445](https://github.com/rizonesoft/Notepad3/issues/5445)
  - Code: `Notepad3.c:4788` - nested `FileSave()` returns bool, passed as FileSaveFlags
  - Fix: `FileSave((FileWatching.FileWatchingMode <= FWM_DONT_CARE) ? FSF_SaveAlways : FSF_None);`
- [ ] **BUG: Wrong working directory for new files** - Files created in Program Files instead of CWD
  - Issue: [#5306](https://github.com/rizonesoft/Notepad3/issues/5306)
  - NP3 changes CWD to exe directory during init
  - Fix: Resolve relative paths against original CWD before directory change
- [ ] **BUG: Encoding detection issue** - UTF-8 files detected as DOS-852
  - Issue: [#5310](https://github.com/rizonesoft/Notepad3/issues/5310)
  - Affects German Windows 11, possibly other locales
  - Improve UTF-8 detection for files without BOM
- [ ] **BUG: Print status message persistent** - "Printing page x..." stays after print
  - Issue: [#5313](https://github.com/rizonesoft/Notepad3/issues/5313)
  - Fix: Clear status bar after print completes
- [ ] **BUG: /m command line uses last search mode** - Should default to text mode
  - Issue: [#5060](https://github.com/rizonesoft/Notepad3/issues/5060)
- [ ] **BUG: Crash on Windows Server 2022** - File Open/SaveAs crashes
  - Issue: [#5066](https://github.com/rizonesoft/Notepad3/issues/5066), [#5080](https://github.com/rizonesoft/Notepad3/issues/5080)
  - Crash in ntdll.dll - needs investigation
- [ ] **BUG: Cannot save settings without folder** - Settings folder must exist
  - Issue: [#5075](https://github.com/rizonesoft/Notepad3/issues/5075)
  - Fix: Create settings folder if it doesn't exist
- [ ] **BUG: Replace dialog full-width caching** - Second replace uses wrong character
  - Issue: [#4268](https://github.com/rizonesoft/Notepad3/issues/4268)
  - CJK full-width replacement cached incorrectly
- [ ] **BUG: Initial window position not working** - Position settings ignored
  - Issue: [#4725](https://github.com/rizonesoft/Notepad3/issues/4725)

## Medium Priority

- [x] **DirectWrite Font Variant Bug** - ✅ Fixed with ResolveFontFace() patch
  - Uses IDWriteGdiInterop to resolve font face → family + weight/style/stretch
  - See [scintilla/np3_patches/001_directwrite_font_resolution.md](../scintilla/np3_patches/001_directwrite_font_resolution.md)
- [ ] **SVG Toolbar Icons** - Resolution-independent toolbar with dark/light mode
  - Auto-adapts to any DPI (100%-300%)
  - Dark/light mode color switching
  - Keep classic bitmap as fallback (ToolBarTheme 0/1)
  - Issues: [#5471](https://github.com/rizonesoft/Notepad3/issues/5471), [#5090](https://github.com/rizonesoft/Notepad3/issues/5090), [#4631](https://github.com/rizonesoft/Notepad3/issues/4631) (DPI scaling)
  - See [research/svg-toolbar.md](research/svg-toolbar.md)
- [ ] **EditorConfig Integration** - Apply `.editorconfig` settings on file open
  - Use [editorconfig-core-c](https://github.com/editorconfig/editorconfig-core-c) library
  - Support: indent_style, indent_size, tab_width, end_of_line, charset
  - See [research/editorconfig-integration.md](research/editorconfig-integration.md)
- [ ] **Autosave / Backup** - Periodic autosave and backup on save
  - Timer-based recovery to `%APPDATA%\Notepad3\recovery\`
  - Optional versioned backups: `.bak`, `.bak_1`, `.bak_2`, ...
  - **AutoSave Settings Dialog** - Built-in autosave configuration UI
  - Issues: [#1665](https://github.com/rizonesoft/Notepad3/issues/1665), [#370](https://github.com/rizonesoft/Notepad3/issues/370), [#512](https://github.com/rizonesoft/Notepad3/issues/512), [#4331](https://github.com/rizonesoft/Notepad3/issues/4331)
  - See [research/autosave-backup.md](research/autosave-backup.md)
- [ ] Installer testing on various Windows versions
- [ ] Language file updates
- [ ] **Force Save Option** - Allow saving unmodified files for strip trailing blanks
  - Issue: [#5444](https://github.com/rizonesoft/Notepad3/issues/5444)
  - Code: `Notepad3.c:12042` - `bSaveNeeded` check skips save on unmodified files
  - Fix: Check strip-blanks setting before early return at line 12044

## Low Priority

- [ ] Documentation updates
- [ ] **Additional Syntax Highlighting** - New language lexers
  - Haskell: [#3035](https://github.com/rizonesoft/Notepad3/issues/3035) - Lexilla `LexHaskell.cxx`
  - Racket: [#3035](https://github.com/rizonesoft/Notepad3/issues/3035) - Could use Lisp/Scheme lexer
  - Verilog HDL: [#4108](https://github.com/rizonesoft/Notepad3/issues/4108) - Lexilla `LexVerilog.cxx`
  - JSON5: [#5411](https://github.com/rizonesoft/Notepad3/issues/5411) - Extend JSON lexer
  - SourcePawn: [#5430](https://github.com/rizonesoft/Notepad3/issues/5430) - SourceMod scripting
  - Groovy: [#5093](https://github.com/rizonesoft/Notepad3/issues/5093)
  - Swift, Zig, Scala, F#, WASM, Vim, OCaml, Smali, GraphViz, Rebol
- [ ] **BATCH Code Folding** - Fold `if`/`for` blocks with parentheses
  - Issue: [#4484](https://github.com/rizonesoft/Notepad3/issues/4484)
- [ ] **Markdown: Highlight Trailing Double Spaces** - Show line break indicators
  - Issue: [#5312](https://github.com/rizonesoft/Notepad3/issues/5312)
- [ ] **Custom Hyperlink Schemes** - User-defined URL protocol recognition
  - ed2k:// links: [#5405](https://github.com/rizonesoft/Notepad3/issues/5405)
  - Customizable via INI settings
- [ ] **Smarter URL Recognition** - Improve URL boundary detection
  - Issue: [#5464](https://github.com/rizonesoft/Notepad3/issues/5464)
  - Don't include trailing `'` when URL is quoted
- [ ] **Display Hidden Characters** - Show invisible/control characters
  - Issue: [#5496](https://github.com/rizonesoft/Notepad3/issues/5496)
  - Scintilla `SCI_SETREPRESENTATION` for custom char display
- [ ] **Scrollbar Marks** - Highlights in scrollbar (search matches, bookmarks)
- [ ] **Enhanced Auto-Complete** - Language-aware auto-complete triggers

## Feature Ideas

### Text Processing
- [ ] **Strip Leading Blanks** - Trim leading whitespace (NP3 only has trailing)
- [ ] **LaTeX Input Method** - LaTeX character insertion (`\alpha` → α)
- [ ] **Base64 Encode/Decode** - Base64 conversion utilities
- [ ] **Insert Unicode Control Characters** - LRM, RLM, ZWJ, ZWNJ, etc.
- [ ] **Number Base Conversion** - Binary/Decimal/Octal/Hex
  - Issue: [#5059](https://github.com/rizonesoft/Notepad3/issues/5059) - TinyExpr output in hex/binary
- [ ] **Character Map Conversions** - Fullwidth↔Halfwidth, CJK transforms (`LCMapStringEx`)
- [ ] **Code Compress/Pretty** - Minify/beautify code
  - Issue: [#5515](https://github.com/rizonesoft/Notepad3/issues/5515) - JSON format, compress, escape/unescape
- [ ] **Insert GUID** - Generate and insert UUID
- [ ] **Insert Shebang** - Insert interpreter line
- [ ] **Insert Timestamps** - Various formats (UTC, Unix, milliseconds)

### Navigation
- [ ] **Navigate Backward/Forward** - VS Code-like history navigation
- [ ] **Go to Block Start/End** - Jump to enclosing block
- [ ] **Brace Find Enhancement** - Search backward for nearest brace when not at one
  - Issue: [#4863](https://github.com/rizonesoft/Notepad3/issues/4863)
- [ ] **Go to Sibling Block** - Navigate between sibling code blocks

### View Options
- [ ] **Toggle Fullscreen** - F11 fullscreen mode
- [ ] **Show Unicode Control Characters** - Toggle visibility
- [ ] **Line Selection Modes** - VS style, Normal, Old VS
- [ ] **Scroll Past Last Line Options** - half, third, quarter screen
- [ ] **Optimize Syntax Scheme Dialog** - Show detected scheme at top of list
  - Issue: [#5302](https://github.com/rizonesoft/Notepad3/issues/5302)
- [ ] **Alphabetical Common Base Settings** - Sort scheme settings alphabetically
  - Issue: [#4627](https://github.com/rizonesoft/Notepad3/issues/4627)

### Copy/Clipboard
- [ ] **Copy as RTF** - Rich text copy with syntax highlighting
- [ ] **Copy/Cut/Paste Binary** - Binary data handling
- [ ] **Copy Filename (no ext)** - Copy filename without extension

### Additional Micro Features
- [ ] **Increment/Decrement Number** - Modify number at cursor (Ctrl+Alt++/-)
- [ ] **Show Hex View** - Display hex representation of selection
- [ ] **CSV Options Dialog** - Configure CSV delimiter/qualifier
- [ ] **Large File Mode** - Optimized mode for files >2GB
  - Issue: [#4699](https://github.com/rizonesoft/Notepad3/issues/4699) - Disable mark occurrences for large files
- [ ] **Online Search** - Search selection via Google/Bing/Wiki
  - Issue: [#5096](https://github.com/rizonesoft/Notepad3/issues/5096) - More WebTemplates in INI
- [ ] **Open Path or Link** - Open file path or URL under cursor

### Windows Integration
- [ ] **Explorer Preview Handler** - Preview text files in Explorer pane
  - Issue: [#5510](https://github.com/rizonesoft/Notepad3/issues/5510)
  - COM DLL implementing `IPreviewHandler` interface
  - Optional installer component
