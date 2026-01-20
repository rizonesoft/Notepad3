# Notepad3 TODO

## High Priority

- [x] **Scintilla/Lexilla Update** - ✅ Upgraded to 5.5.8/5.4.6 parity
  - Applied incrementally to preserve NP3 patches (DPI, Oniguruma, DirectWrite)
  - See [scintilla/np3_patches/upstream_558/](../scintilla/np3_patches/upstream_558/)
- [x] **BUG: FileSave() nested call** - ✅ Fixed in commit 658fb2f19
  - Issue: [#5445](https://github.com/rizonesoft/Notepad3/issues/5445)
  - Code: `Notepad3.c:4788` - nested `FileSave()` returns bool, passed as FileSaveFlags
  - Fix: `FileSave((FileWatching.FileWatchingMode <= FWM_DONT_CARE) ? FSF_SaveAlways : FSF_None);`
- [ ] **(Q2) BUG: Wrong working directory for new files** - Files created in Program Files instead of CWD
  - Issue: [#5306](https://github.com/rizonesoft/Notepad3/issues/5306)
  - NP3 changes CWD to exe directory during init
  - Fix: Resolve relative paths against original CWD before directory change
- [ ] **(Q2) BUG: Encoding detection issue** - UTF-8 files detected as DOS-852
  - Issue: [#5310](https://github.com/rizonesoft/Notepad3/issues/5310)
  - Affects German Windows 11, possibly other locales
  - Improve UTF-8 detection for files without BOM
- [x] **(Q1) BUG: Print status message persistent** - ✅ FIXED
  - Issue: [#5313](https://github.com/rizonesoft/Notepad3/issues/5313)
  - Fix: Added `StatusSetText()` call to clear status bar after `EditPrint()` completes
- [x] **(Q1) BUG: /m command line uses last search mode** - ✅ FIXED
  - Issue: [#5060](https://github.com/rizonesoft/Notepad3/issues/5060)
  - Fix: When `/m` is used without 'R' flag, explicitly clear SCFIND_REGEXP to force text mode
- [ ] **(Q3) BUG: Crash on Windows Server 2022** - File Open/SaveAs crashes
  - Issue: [#5066](https://github.com/rizonesoft/Notepad3/issues/5066), [#5080](https://github.com/rizonesoft/Notepad3/issues/5080)
  - Crash in ntdll.dll - needs investigation
- [ ] **(Q1) BUG: Cannot save settings without folder** - Settings folder must exist
  - Issue: [#5075](https://github.com/rizonesoft/Notepad3/issues/5075)
  - Fix: Create settings folder if it doesn't exist
- [ ] **(Q2) BUG: Replace dialog full-width caching** - Second replace uses wrong character
  - Issue: [#4268](https://github.com/rizonesoft/Notepad3/issues/4268)
  - CJK full-width replacement cached incorrectly
- [ ] **(Q2) BUG: Initial window position not working** - Position settings ignored
  - Issue: [#4725](https://github.com/rizonesoft/Notepad3/issues/4725)
- [ ] **(Q3) BUG: Regex replace issue** - Verify if still present
  - Issue: [#3531](https://github.com/rizonesoft/Notepad3/issues/3531)
  - Old bug from v5.21 - needs verification
- [ ] **(Q2) BUG: Minipath options don't save** - FullRowSelect/TrackSelect broken
  - Issue: [#4116](https://github.com/rizonesoft/Notepad3/issues/4116)
- [ ] **(Q1) BUG: Monitoring log not saved** - Setting not persisted
  - Issue: [#5037](https://github.com/rizonesoft/Notepad3/issues/5037)
- [ ] **(Q3) BUG: LAN file freeze** - Freezes when network host offline
  - Issue: [#5050](https://github.com/rizonesoft/Notepad3/issues/5050)
- [ ] **(Q1) BUG: Find/Replace patterns not updating** - Dropdown not refreshed immediately
  - Issue: [#5134](https://github.com/rizonesoft/Notepad3/issues/5134)
- [ ] **(Q3) BUG: Multiple file positions not saved** - Only last file's bookmarks/caret preserved
  - Issue: [#5151](https://github.com/rizonesoft/Notepad3/issues/5151)
- [ ] **(Q3) BUG: grepWinNP3 crash** - Right-click search results crashes
  - Issue: [#5158](https://github.com/rizonesoft/Notepad3/issues/5158)
- [ ] **(Q2) BUG: PHP comment toggle** - Ctrl+Q not working in Web Source Code
  - Issue: [#5163](https://github.com/rizonesoft/Notepad3/issues/5163)
- [ ] **(Q2) BUG: AltGr shortcut conflict** - Can't type `}` `@` on non-US keyboards
  - Issue: [#5220](https://github.com/rizonesoft/Notepad3/issues/5220)
- [ ] **(Q1) BUG: Mouse scroll settings not updated** - Needs restart to apply
  - Issue: [#5223](https://github.com/rizonesoft/Notepad3/issues/5223)
- [ ] **(Q2) BUG: Highlight current line broken** - Settings not respected (regression)
  - Issue: [#5270](https://github.com/rizonesoft/Notepad3/issues/5270)
- [ ] **(Q2) BUG: File lock held too long on save** - Blocks FileSystemWatcher
  - Issue: [#5301](https://github.com/rizonesoft/Notepad3/issues/5301)
- [ ] **(Q2) BUG: Folder handle leak** - Can't rename/delete folders with opened files
  - Issue: [#5342](https://github.com/rizonesoft/Notepad3/issues/5342)
- [ ] **(Q1) BUG: Black line in Language menu** - Visual glitch in submenu
  - Issue: [#5361](https://github.com/rizonesoft/Notepad3/issues/5361)

## Medium Priority

- [x] **DirectWrite Font Variant Bug** - ✅ Fixed with ResolveFontFace() patch
  - Uses IDWriteGdiInterop to resolve font face → family + weight/style/stretch
  - See [scintilla/np3_patches/001_directwrite_font_resolution.md](../scintilla/np3_patches/001_directwrite_font_resolution.md)
- [ ] **(Q3) SVG Toolbar Icons** - Resolution-independent toolbar with dark/light mode
  - Auto-adapts to any DPI (100%-300%)
  - Dark/light mode color switching
  - Keep classic bitmap as fallback (ToolBarTheme 0/1)
  - Issues: [#5471](https://github.com/rizonesoft/Notepad3/issues/5471), [#5090](https://github.com/rizonesoft/Notepad3/issues/5090), [#4631](https://github.com/rizonesoft/Notepad3/issues/4631) (DPI scaling), [#5316](https://github.com/rizonesoft/Notepad3/issues/5316) (dark mode readability), [#5390](https://github.com/rizonesoft/Notepad3/issues/5390) (encrypted dialog)
  - See [research/svg-toolbar.md](research/svg-toolbar.md)
- [ ] **(Q3) EditorConfig Integration** - Apply `.editorconfig` settings on file open
  - Use [editorconfig-core-c](https://github.com/editorconfig/editorconfig-core-c) library
  - Support: indent_style, indent_size, tab_width, end_of_line, charset
  - See [research/editorconfig-integration.md](research/editorconfig-integration.md)
- [/] **(Q3) Autosave / Backup** - ✅ **PARTIALLY IMPLEMENTED**
  - **Implemented**:
    - Settings dialog (`AutoSaveBackupSettingsDlg`) with full UI
    - Timer-based periodic autosave (`AutoSaveStart/Stop/DoWork`)
    - Options: Periodic save, Save on Suspend, Save on Shutdown
    - Backup options: Enable backup, On AutoSave, Side-by-side
    - Configurable interval (minimum 2 seconds)
  - **Remaining**:
    - Recovery folder (`%APPDATA%\Notepad3\recovery\`) not yet implemented
    - Versioned backups (`.bak_1`, `.bak_2`) not yet implemented
  - Issues: [#1665](https://github.com/rizonesoft/Notepad3/issues/1665), [#370](https://github.com/rizonesoft/Notepad3/issues/370), [#512](https://github.com/rizonesoft/Notepad3/issues/512), [#4331](https://github.com/rizonesoft/Notepad3/issues/4331), [#3652](https://github.com/rizonesoft/Notepad3/issues/3652), [#5399](https://github.com/rizonesoft/Notepad3/issues/5399)
  - See [research/autosave-backup.md](research/autosave-backup.md)
- [ ] (Q2) Installer testing on various Windows versions
- [x] **AVX2 Build** - ✅ Added x64_AVX2 to CI matrix
  - Issue: [#4240](https://github.com/rizonesoft/Notepad3/issues/4240)
- [ ] (Q2) Language file updates
- [ ] **(Q1) Move Beta Page to rizonesoft.com** - Host beta downloads on main site
  - Issue: [#1129](https://github.com/rizonesoft/Notepad3/issues/1129)
- [ ] **(Q2) Improve Temp File Handling** - Better UX for files from archives
  - Issue: [#5343](https://github.com/rizonesoft/Notepad3/issues/5343)
- [ ] **(Q1) Force Save Option** - Allow saving unmodified files for strip trailing blanks
  - Issue: [#5444](https://github.com/rizonesoft/Notepad3/issues/5444)
  - Code: `Notepad3.c:12042` - `bSaveNeeded` check skips save on unmodified files
  - Fix: Check strip-blanks setting before early return at line 12044

## GitHub Actions

- [x] **CI Workflow** - ✅ Added MSVC builds (Win32/x64 Release)
  - Runs on push/PR to master alongside AppVeyor
- [x] **ARM64 Build** - ✅ Added to CI matrix (Win32/x64/ARM64)
- [ ] **(Q1) Clang-cl Build** - Alternative compiler for better warnings
- [ ] **(Q1) Debug Build** - Catch debug-only assertions
- [ ] **(Q1) PR Labeler** - Auto-label PRs based on files changed
- [ ] **(Q2) Automatic Releases** - Create GitHub releases on tags with built artifacts
- [ ] **(Q2) Changelog Generator** - Auto-generate release notes from commits/PRs
- [ ] **(Q2) CodeQL Analysis** - Scans C++ code for security vulnerabilities
- [ ] **(Q1) Dependency Review** - Checks for vulnerable dependencies in PRs
- [ ] **(Q2) MSVC Code Analysis** - Microsoft's static analyzer (/analyze)
- [ ] **(Q3) Nightly Releases** - Automated nightly builds via GitHub Actions
  - Issue: [#5412](https://github.com/rizonesoft/Notepad3/issues/5412)
  - Schedule: Daily at 2 AM UTC
  - Package: Portable ZIP with exe, lng, docs, themes, ini files
  - See [research/nightly_releases.md](research/nightly_releases.md)

## Low Priority

- [ ] (Q1) Documentation updates
- [ ] **(Q2) Long Path Support** - Support paths >260 characters (Win10+)
  - Issue: [#3580](https://github.com/rizonesoft/Notepad3/issues/3580)
- [ ] **(Q3) Additional Syntax Highlighting** - New language lexers
  - Haskell: [#3035](https://github.com/rizonesoft/Notepad3/issues/3035) - Lexilla `LexHaskell.cxx`
  - Racket: [#3035](https://github.com/rizonesoft/Notepad3/issues/3035) - Could use Lisp/Scheme lexer
  - OpenCL: [#5374](https://github.com/rizonesoft/Notepad3/issues/5374) - C-like with extra keywords
  - Verilog HDL: [#4108](https://github.com/rizonesoft/Notepad3/issues/4108) - Lexilla `LexVerilog.cxx`
  - JSON5: [#5411](https://github.com/rizonesoft/Notepad3/issues/5411) - Extend JSON lexer
  - SourcePawn: [#5430](https://github.com/rizonesoft/Notepad3/issues/5430) - SourceMod scripting
  - Groovy: [#5093](https://github.com/rizonesoft/Notepad3/issues/5093)
  - Swift, Zig, Scala, F#, WASM, Vim, OCaml, Smali, GraphViz, Rebol
  - (Maybe) CSS in `<style>` tags: [#2061](https://github.com/rizonesoft/Notepad3/issues/2061) - Embedded language complexity
  - BUG: CSS keywords: [#4214](https://github.com/rizonesoft/Notepad3/issues/4214) - Update CSS property list
- [ ] **(Q3) BATCH Code Folding** - Fold `if`/`for` blocks with parentheses
  - Issue: [#4484](https://github.com/rizonesoft/Notepad3/issues/4484)
  - BUG: [#4959](https://github.com/rizonesoft/Notepad3/issues/4959) - Complex for loops highlighting
- [ ] **(Q1) Markdown: Highlight Trailing Double Spaces** - Show line break indicators
  - Issue: [#5312](https://github.com/rizonesoft/Notepad3/issues/5312)
- [ ] **(Q2) Custom Hyperlink Schemes** - User-defined URL protocol recognition
  - ed2k:// links: [#5405](https://github.com/rizonesoft/Notepad3/issues/5405)
  - Customizable via INI settings
- [ ] **(Q2) Smarter URL Recognition** - Improve URL boundary detection
  - Issue: [#5464](https://github.com/rizonesoft/Notepad3/issues/5464)
  - Don't include trailing `'` when URL is quoted
- [ ] **(Q3) Display Hidden Characters** - Show invisible/control characters
  - Issue: [#5496](https://github.com/rizonesoft/Notepad3/issues/5496)
  - Scintilla `SCI_SETREPRESENTATION` for custom char display
  - Issue: [#5035](https://github.com/rizonesoft/Notepad3/issues/5035) - Non-printing chars like Notepad++
- [ ] **(Q2) Trailing Whitespace Highlighting** - Distinct style for trailing blanks
  - Issue: [#1913](https://github.com/rizonesoft/Notepad3/issues/1913)
- [ ] **(Q3) Scrollbar Marks** - Highlights in scrollbar (search matches, bookmarks)
- [ ] **(Q3) Windows Spell Checker** - Spellcheck via Windows API
  - Issue: [#5157](https://github.com/rizonesoft/Notepad3/issues/5157)
- [ ] **(Q3) Enhanced Auto-Complete** - Language-aware auto-complete triggers
- [ ] **(Q3) Auto-Pair Brackets** - Auto-close `()`, `[]`, `{}`, `""`, `''`
  - Issue: [#4149](https://github.com/rizonesoft/Notepad3/issues/4149)
  - Issue: [#5285](https://github.com/rizonesoft/Notepad3/issues/5285) - Smart auto-dedent on closing bracket
- [ ] **(Q3) Custom Keyboard Shortcuts** - User-configurable shortcut keys
  - Issue: [#595](https://github.com/rizonesoft/Notepad3/issues/595)

## Feature Ideas

### Text Processing
- [ ] **(Q1) Strip Leading Blanks** - Trim leading whitespace (NP3 only has trailing)
- [ ] **(Q1) Right-Click Convert Case** - Add case conversion to context menu
  - Issue: [#5403](https://github.com/rizonesoft/Notepad3/issues/5403)
- [ ] **(Q2) LaTeX Input Method** - LaTeX character insertion (`\alpha` → α)
- [x] **(Q2) Base64 Encode/Decode** - ✅ IMPLEMENTED via `IDM_EDIT_BASE64ENCODE/DECODE`
- [ ] **(Q1) Insert Unicode Control Characters** - LRM, RLM, ZWJ, ZWNJ, etc.
- [ ] **(Q2) Number Base Conversion** - Binary/Decimal/Octal/Hex
  - Issue: [#5059](https://github.com/rizonesoft/Notepad3/issues/5059) - TinyExpr output in hex/binary
- [ ] **(Q2) Character Map Conversions** - Fullwidth↔Halfwidth, CJK transforms (`LCMapStringEx`)
- [ ] **(Q3) Code Compress/Pretty** - Minify/beautify code
  - Issue: [#5515](https://github.com/rizonesoft/Notepad3/issues/5515) - JSON format, compress, escape/unescape
  - Issue: [#1790](https://github.com/rizonesoft/Notepad3/issues/1790) - Call external formatter per scheme
  - Issue: [#2839](https://github.com/rizonesoft/Notepad3/issues/2839) - Tidy HTML/XML
  - (Maybe) Run Scripts: [#4045](https://github.com/rizonesoft/Notepad3/issues/4045) - Execute Python/Perl
  - Issue: [#4126](https://github.com/rizonesoft/Notepad3/issues/4126) - Custom Execute Document command
- [x] **(Q1) Insert GUID** - ✅ IMPLEMENTED via `IDM_EDIT_INSERT_GUID` (Ctrl+Shift+.)
- [ ] **(Q1) Insert Shebang** - Insert interpreter line
- [x] **(Q1) Insert Timestamps** - ✅ IMPLEMENTED via `IDM_EDIT_INSERT_SHORTDATE/LONGDATE` (Ctrl+F5, Shift+F5) and `CMD_INSERT_TIMESTAMP`

### Navigation
- [ ] **(Q3) Navigate Backward/Forward** - VS Code-like history navigation
- [ ] **(Q2) Go to Block Start/End** - Jump to enclosing block
- [ ] **(Q2) Brace Find Enhancement** - Search backward for nearest brace when not at one
  - Issue: [#4863](https://github.com/rizonesoft/Notepad3/issues/4863)
- [ ] **(Q2) Go to Sibling Block** - Navigate between sibling code blocks
- [ ] **(Q2) Touchpad Horizontal Scroll** - Direct touchpad left/right scrolling
  - Issue: [#3468](https://github.com/rizonesoft/Notepad3/issues/3468)
- [ ] **(Q1) Toggle Focus Find/Main** - Shortcut to switch focus
  - Issue: [#5395](https://github.com/rizonesoft/Notepad3/issues/5395)

### View Options
- [x] **(Q2) Toggle Fullscreen** - ✅ IMPLEMENTED via `SCR_FULL_SCREEN` mode
- [ ] **(Q3) Split View** - View two parts of same document
  - Issue: [#2577](https://github.com/rizonesoft/Notepad3/issues/2577)
- [ ] **(Q1) Show Unicode Control Characters** - Toggle visibility
- [ ] **(Q2) Line Selection Modes** - VS style, Normal, Old VS
- [x] **(Q1) Disable Multiple Cursors Option** - ✅ IMPLEMENTED via `IDM_SET_MULTIPLE_SELECTION` toggle
  - Issue: [#4033](https://github.com/rizonesoft/Notepad3/issues/4033) - ✅ Resolved
- [x] **(Q1) Scroll Past Last Line Options** - ✅ IMPLEMENTED via `Settings.ScrollPastEOF` (simple on/off, not multiple options)
- [ ] **(Q2) Optimize Syntax Scheme Dialog** - Show detected scheme at top of list
  - Issue: [#5302](https://github.com/rizonesoft/Notepad3/issues/5302)
- [ ] **(Q1) Alphabetical Common Base Settings** - Sort scheme settings alphabetically
  - Issue: [#4627](https://github.com/rizonesoft/Notepad3/issues/4627)
- [x] **(Q2) Find Dialog Position Persistence** - ✅ IMPLEMENTED via `FindReplaceDlgPosX/Y` saved to INI
  - Issue: [#3905](https://github.com/rizonesoft/Notepad3/issues/3905) - ✅ Resolved

### Copy/Clipboard
- [ ] **(Q2) Copy as RTF** - Rich text copy with syntax highlighting
  - Issue: [#5052](https://github.com/rizonesoft/Notepad3/issues/5052)
  - **Reference**: [Notepad4](https://github.com/zufuliu/notepad4) implements via `IDM_EDIT_COPYRTF`
- [ ] **(Q2) Copy/Cut/Paste Binary** - Binary data handling
- [x] **(Q1) Copy All** - ✅ IMPLEMENTED via `IDM_EDIT_COPYALL` (Alt+C)
- [x] **(Q1) Copy Add** - ✅ IMPLEMENTED via `IDM_EDIT_COPYADD` (Ctrl+E)
- [x] **(Q1) Copy Filename Options** - ✅ IMPLEMENTED
  - `IDM_EDIT_INSERT_FILENAME` - Copy filename (Ctrl+F9)
  - `IDM_EDIT_INSERT_DIRNAME` - Copy directory
  - `IDM_EDIT_INSERT_PATHNAME` - Copy full path

### Additional Micro Features
- [ ] **(Q1) Increment/Decrement Number** - Modify number at cursor (Ctrl+Alt++/-)
- [ ] **(Q2) Show Hex View** - Display hex representation of selection
- [ ] **(Q1) CSV Options Dialog** - Configure CSV delimiter/qualifier
- [ ] **(Q3) Large File Mode** - Optimized mode for files >2GB
  - Issue: [#4699](https://github.com/rizonesoft/Notepad3/issues/4699) - Disable mark occurrences for large files
  - Issue: [#4954](https://github.com/rizonesoft/Notepad3/issues/4954) - Can't open files >4GB
  - **Reference**: [Notepad4](https://github.com/zufuliu/notepad4) implements this via `SC_DOCUMENTOPTION_TEXT_LARGE` and `bLargeFileMode` flag - disables expensive features, enables reload-in-large-mode option
- [x] **(Q2) Online Search** - ✅ IMPLEMENTED via `WebTemplate1/2` (Google, Wikipedia)
  - Issue: [#5096](https://github.com/rizonesoft/Notepad3/issues/5096) - Custom templates via INI
  - BUG: [#4056](https://github.com/rizonesoft/Notepad3/issues/4056) - Caption not in File menu
  - BUG: [#4952](https://github.com/rizonesoft/Notepad3/issues/4952) - Template vs Action naming inconsistency
- [ ] **(Q2) Open Path or Link** - Open file path or URL under cursor
  - Issue: [#3926](https://github.com/rizonesoft/Notepad3/issues/3926) - Support relative file:// paths
- [ ] **(Q1) Open Containing Folder** - Open the folder containing the current file in Explorer
  - **Reference**: [Notepad4](https://github.com/zufuliu/notepad4) implements via `CMD_OPEN_CONTAINING_FOLDER` and `EditOpenSelection(OpenSelectionType_ContainingFolder)`

### Windows Integration
- [ ] **(Q3) Explorer Preview Handler** - Preview text files in Explorer pane
  - Issue: [#5510](https://github.com/rizonesoft/Notepad3/issues/5510)
  - COM DLL implementing `IPreviewHandler` interface
  - Optional installer component
