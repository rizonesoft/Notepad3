# Notepad3 TODO

## Ideas / New Features (deliberated)
- [ ] Merge/Cleanup all old documentation (Build/Docs/*.txt, etc.) files
- [ ] PCRE2 backward search seems to be slow - ask Claude for analysis

## Epics (high effort)

- [X] **(Q3) SVG Toolbar Icons** - Resolution-independent toolbar with dark/light mode
  - Auto-adapts to any DPI (100%-300%)
  - Dark/light mode color switching
  - Keep classic bitmap as fallback (ToolBarTheme 0/1)
  - Issues: [#5471](https://github.com/rizonesoft/Notepad3/issues/5471), [#5090](https://github.com/rizonesoft/Notepad3/issues/5090), [#4631](https://github.com/rizonesoft/Notepad3/issues/4631) (DPI scaling), [#5316](https://github.com/rizonesoft/Notepad3/issues/5316) (dark mode readability) - ✅ FIXED, [#5390](https://github.com/rizonesoft/Notepad3/issues/5390) (encrypted dialog) - ✅ FIXED
  - See [research/svg-toolbar.md](research/svg-toolbar.md)

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

- [ ] **(Q3) EditorConfig Integration** - Apply `.editorconfig` settings on file open
  - Use [editorconfig-core-c](https://github.com/editorconfig/editorconfig-core-c) library
  - Support: indent_style, indent_size, tab_width, end_of_line, charset
  - See [research/editorconfig-integration.md](research/editorconfig-integration.md)

- [ ] **(Q3) Windows Spell Checker** - Spellcheck via Windows API - ✅ WON'T CHANGE
  - Issue: [#5157](https://github.com/rizonesoft/Notepad3/issues/5157)

- [ ] **(Q3) Scrollbar Marks** - Highlights in scrollbar (search matches, bookmarks)

- [ ] **(Q3) Plugin System** - Customizable / loadable plugin support
  - Issue: [#5071](https://github.com/rizonesoft/Notepad3/issues/5071)
  - Architectural: extension API surface, lifetime, sandboxing

- [ ] **(Q3) Code Compress/Pretty** - Minify/beautify code
  - Issue: [#5515](https://github.com/rizonesoft/Notepad3/issues/5515) - JSON format, compress, escape/unescape - ✅ WON'T CHANGE
  - Issue: [#1790](https://github.com/rizonesoft/Notepad3/issues/1790) - Call external formatter per scheme
  - Issue: [#2839](https://github.com/rizonesoft/Notepad3/issues/2839) - Tidy HTML/XML
  - (Maybe) Run Scripts: [#4045](https://github.com/rizonesoft/Notepad3/issues/4045) - Execute Python/Perl
  - Issue: [#4126](https://github.com/rizonesoft/Notepad3/issues/4126) - Custom Execute Document command

## High Priority

- [x] **Scintilla/Lexilla Update** - ✅ Upgraded to 5.5.8/5.4.6 parity
  - Applied incrementally to preserve NP3 patches (DPI, Oniguruma, DirectWrite)
  - See [scintilla/np3_patches/upstream_558/](../scintilla/np3_patches/upstream_558/)
- [x] **BUG: FileSave() nested call** - ✅ Fixed in commit 658fb2f19
  - Issue: [#5445](https://github.com/rizonesoft/Notepad3/issues/5445)
  - Code: `Notepad3.c:4788` - nested `FileSave()` returns bool, passed as FileSaveFlags
  - Fix: `FileSave((FileWatching.FileWatchingMode <= FWM_DONT_CARE) ? FSF_SaveAlways : FSF_None);`
- [x] **(Q2) BUG: Wrong working directory for new files** - ✅ FIXED
  - Issue: [#5306](https://github.com/rizonesoft/Notepad3/issues/5306)
  - Fix: Added else block in `Path_NormalizeEx()` to use canonicalized path for non-existent files
- [x] **(Q2) BUG: Encoding detection issue** - UTF-8 files detected as DOS-852
  - Issue: [#5310](https://github.com/rizonesoft/Notepad3/issues/5310)
  - Affects German Windows 11, possibly other locales
  - Improve UTF-8 detection for files without BOM
- [x] **(Q1) BUG: Print status message persistent** - ✅ FIXED
  - Issue: [#5313](https://github.com/rizonesoft/Notepad3/issues/5313)
  - Fix: Added `StatusSetText()` call to clear status bar after `EditPrint()` completes
- [x] **(Q1) BUG: /m command line uses last search mode** - ✅ FIXED
  - Issue: [#5060](https://github.com/rizonesoft/Notepad3/issues/5060)
  - Fix: When `/m` is used without 'R' flag, explicitly clear SCFIND_REGEXP to force text mode
- [x] **(Q3) Replace GetOpenFileNameW with IFileOpenDialog** - Modern file dialog API - ✅ IMPLEMENTED
  - Issue: [#5066](https://github.com/rizonesoft/Notepad3/issues/5066)
  - Issue: [#5080](https://github.com/rizonesoft/Notepad3/issues/5080)
  - Fixes crash on Windows Server 2022 (STATUS_STACK_BUFFER_OVERRUN in ntdll.dll)
  - See [research/server2022-file-dialog-crash.md](research/server2022-file-dialog-crash.md)
  - [ ] **Test on Windows Server 2022 and higer** - ⚠ Validation❗
- [x] **(Q1) BUG: Cannot save settings without folder**
  - Issue: [#5075](https://github.com/rizonesoft/Notepad3/issues/5075)
  - Fix: Changed `CreateDirectoryW` to `SHCreateDirectoryExW` to create all intermediate directories
- [x] **(Q2) BUG: Replace dialog full-width caching** - Second replace uses wrong character - ✅ FIXED
  - Issue: [#4268](https://github.com/rizonesoft/Notepad3/issues/4268)
  - CJK full-width replacement cached incorrectly - ✅ FIXED
- [x] **(Q2) BUG: Initial window position not working** - Position settings ignored - ✅ IMPLEMENTED
  - Issue: [#4725](https://github.com/rizonesoft/Notepad3/issues/4725)
  - [ ] **To be analyzed - works as designed ???** - ✅ FIXED
- [ ] **(Q2) BUG: Minipath options don't save** - FullRowSelect/TrackSelect broken
  - Issue: [#4116](https://github.com/rizonesoft/Notepad3/issues/4116)
- [x] **(Q1) BUG: Monitoring log not saved** - ✅ FIXED
  - Issue: [#5037](https://github.com/rizonesoft/Notepad3/issues/5037)
  - Fix: Added `MonitoringLog` to Settings struct with INI load/save in Config.cpp
- [ ] **(Q3) BUG: LAN file freeze** - Freezes when network host offline
  - Issue: [#5050](https://github.com/rizonesoft/Notepad3/issues/5050)
- [x] **(Q1) BUG: Find/Replace patterns not updating** - Dropdown not refreshed immediately - ✅ FIXED
  - Issue: [#5134](https://github.com/rizonesoft/Notepad3/issues/5134)
- [x] **(Q3) BUG: Multiple file positions not saved** - Only last file's bookmarks/caret preserved - ✅ FIXED
  - Issue: [#5151](https://github.com/rizonesoft/Notepad3/issues/5151) - ✅ DELEGATED (using grepWin orig)
- [x] **(Q3) BUG: grepWinNP3 crash** - Right-click search results crashes - ✅ IMPLEMENTED
  - Issue: [#5158](https://github.com/rizonesoft/Notepad3/issues/5158)
- [x] **(Q2) BUG: PHP comment toggle** - Ctrl+Q not working in Web Source Code - ✅ FIXED
  - Issue: [#5163](https://github.com/rizonesoft/Notepad3/issues/5163)
- [/] **(Q2) CHR: Ctrl+LAlt+V is used by NP3 - AltGr+V works** - Can't type `}` `@` on non-US keyboards - ❌ WON'T CHANGE
  - Issue: [#5220](https://github.com/rizonesoft/Notepad3/issues/5220)
- [x] **(Q1) BUG: Mouse scroll settings not updated** - ✅ FIXED
  - Issue: [#5223](https://github.com/rizonesoft/Notepad3/issues/5223)
  - Fix: Forward `WM_SETTINGCHANGE` to Scintilla to refresh cached scroll parameters
- [x] **(Q2) BUG: File lock held too long on save** - Blocks FileSystemWatcher - ✅ IMPLEMENTED
  - Issue: [#5301](https://github.com/rizonesoft/Notepad3/issues/5301)
- [x] **(Q2) BUG: Folder handle leak** - Can't rename/delete folders with opened files - ✅ IMPLEMENTED
  - Issue: [#5342](https://github.com/rizonesoft/Notepad3/issues/5342)
- [x] **(Q1) BUG: Black line in Language menu** - ✅ FIXED
  - Issue: [#5361](https://github.com/rizonesoft/Notepad3/issues/5361)
  - Fix: Removed `WM_UAHNCPAINTMENUPOPUP` from message interception - was using wrong window handle
- [x] **(Q1) BUG: Typing causes cursor to flash, disappear and reappear** - Regression
  - Issue: [#4942](https://github.com/rizonesoft/Notepad3/issues/4942)
- [x] **(Q1) BUG: Holding Ctrl+S inserts DC3 characters** - Save shortcut leaks into buffer when repeated
  - Issue: [#5178](https://github.com/rizonesoft/Notepad3/issues/5178)
- [x] **(Q1) BUG: Redo button stays active after redo stack exhausted**
  - Issue: [#5149](https://github.com/rizonesoft/Notepad3/issues/5149)
- [x] **(Q1) BUG: Ctrl+Shift+D redo deletes penultimate line and merges with next**
  - Issue: [#5150](https://github.com/rizonesoft/Notepad3/issues/5150)
- [x] **(Q1) BUG: "Only One Instance per File" doesn't always work**
  - Issue: [#5122](https://github.com/rizonesoft/Notepad3/issues/5122),
  - Issue: [#4636](https://github.com/rizonesoft/Notepad3/issues/4636)
- [x] **(Q1) BUG: New Empty Window prompts to save when nothing to save**
  - Issue: [#5164](https://github.com/rizonesoft/Notepad3/issues/5164)
- [x] **(Q1) BUG: New Empty Window button broken when AutoLoadMRUFile=true**
  - Issue: [#5174](https://github.com/rizonesoft/Notepad3/issues/5174)
- [x] **(Q1) BUG: Close-with-unsaved system dialog disappears on second instance close**
  - Issue: [#4945](https://github.com/rizonesoft/Notepad3/issues/4945)
- [x] **(Q1) BUG: Screen needs manual refresh after opening file with /g switch**
  - Issue: [#5103](https://github.com/rizonesoft/Notepad3/issues/5103)
- [x] **(Q2) BUG: Filename starting with "- " opens blank file from CLI**
  - Issue: [#5160](https://github.com/rizonesoft/Notepad3/issues/5160)
- [x] **(Q2) BUG: Selection (Sel) and Occurrences (Occ) status counts incorrect**
  - Issue: [#5176](https://github.com/rizonesoft/Notepad3/issues/5176)

## Medium Priority

- [x] **DirectWrite Font Variant Bug** - ✅ Fixed with ResolveFontFace() patch
  - Uses IDWriteGdiInterop to resolve font face → family + weight/style/stretch
  - See [scintilla/np3_patches/001_directwrite_font_resolution.md](../scintilla/np3_patches/001_directwrite_font_resolution.md)
- [ ] (Q2) Installer testing on various Windows versions
- [x] **AVX2 Build** - ✅ Added x64_AVX2 to CI matrix
  - Issue: [#4240](https://github.com/rizonesoft/Notepad3/issues/4240)
- [x] (Q2) Language file updates
- [ ] **(Q1) Move Beta Page to rizonesoft.com** - Host beta downloads on main site
  - Issue: [#1129](https://github.com/rizonesoft/Notepad3/issues/1129)
- [x] **(Q2) Improve Temp File Handling** - Better UX for files from archives - ✅ FIXED
  - Issue: [#5343](https://github.com/rizonesoft/Notepad3/issues/5343)
- [x] **(Q1) Force Save Option** - ✅ FIXED
  - Issue: [#5444](https://github.com/rizonesoft/Notepad3/issues/5444)
  - Fix: Added `Settings.FixTrailingBlanks` check to early return in `FileSave()`
- [ ] **(Q2) BUG: Korean IME composition box obscures the line of text being edited**
  - Issue: [#2982](https://github.com/rizonesoft/Notepad3/issues/2982)
- [x] **(Q2) BUG: Mixed tiny menu / huge dialog icons across screens with different DPI**
  - Issue: [#3150](https://github.com/rizonesoft/Notepad3/issues/3150)
- [ ] **(Q2) BUG: AutoCompleteWordCharSet setting has no effect**
  - Issue: [#4029](https://github.com/rizonesoft/Notepad3/issues/4029)
- [ ] **(Q2) BUG: Column Wrap doesn't work on Chinese / CJK text**
  - Issue: [#4940](https://github.com/rizonesoft/Notepad3/issues/4940)
- [x] **(Q2) BUG: Scrollbar can't reach bottom when processing very large files**
  - Issue: [#5092](https://github.com/rizonesoft/Notepad3/issues/5092)
- [ ] **(Q2) BUG: Accented vowels typed lowercase with Caps Lock on**
  - Issue: [#5097](https://github.com/rizonesoft/Notepad3/issues/5097)
- [x] **(Q2) BUG: Ctrl+Drag&Drop disabled while multi-cursor active**
  - Issue: [#5153](https://github.com/rizonesoft/Notepad3/issues/5153)
- [x] **(Q2) BUG: Taskbar window order changes after minimize**
  - Issue: [#5159](https://github.com/rizonesoft/Notepad3/issues/5159)
- [x] **(Q2) BUG: Menu Bar accelerator keys unreliable in non-English locales**
  - Issue: [#5169](https://github.com/rizonesoft/Notepad3/issues/5169)
- [x] **(Q2) BUG: Taskbar icon refuses to combine/group**
  - Issue: [#5175](https://github.com/rizonesoft/Notepad3/issues/5175)
- [x] **(Q2) BUG: Regex `{min,max}` quantifier not supported by current grammar**
  - Issue: [#5180](https://github.com/rizonesoft/Notepad3/issues/5180)
- [x] **(Q2) BUG: Color dialog RGB fields accept only 2 of 3 digits when typed**
  - Issue: [#5185](https://github.com/rizonesoft/Notepad3/issues/5185)

## GitHub Actions

- [x] **CI Workflow** - ✅ Added MSVC builds (Win32/x64 Release)
  - Runs on push/PR to master
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
- [x] **(Q2) Long Path Support** - Support paths >260 characters (Win10+) - ✅ FIXED
  - Issue: [#3580](https://github.com/rizonesoft/Notepad3/issues/3580)  - ✅ IMPLEMENTED

- [ ] **(Q3) Additional Syntax Highlighting** - New language lexers
  - [ ] Haskell: [#3035](https://github.com/rizonesoft/Notepad3/issues/3035) - Lexilla `LexHaskell.cxx`
  - [ ] Racket: [#3035](https://github.com/rizonesoft/Notepad3/issues/3035) - Could use Lisp/Scheme lexer
  - [ ] OpenCL: [#5374](https://github.com/rizonesoft/Notepad3/issues/5374) - C-like with extra keywords
  - [ ] Verilog HDL: [#4108](https://github.com/rizonesoft/Notepad3/issues/4108) - Lexilla `LexVerilog.cxx`
  - [x] JSON5: [#5411](https://github.com/rizonesoft/Notepad3/issues/5411) - ✅ IMPLEMENTED
  - [ ] SourcePawn: [#5430](https://github.com/rizonesoft/Notepad3/issues/5430) - SourceMod scripting
  - [ ] Groovy: [#5093](https://github.com/rizonesoft/Notepad3/issues/5093)
  - [ ] Swift, Zig, Scala, F#, WASM, Vim, OCaml, Smali, GraphViz, Rebol
  - (Maybe) CSS in `<style>` tags: [#2061](https://github.com/rizonesoft/Notepad3/issues/2061) - Embedded language complexity
  - [ ] BUG: CSS keywords: [#4214](https://github.com/rizonesoft/Notepad3/issues/4214) - Update CSS property list
  - [ ] BUG: CSS syntax highlighting incorrect: [#3572](https://github.com/rizonesoft/Notepad3/issues/3572)
  - [ ] HTML: [#3470](https://github.com/rizonesoft/Notepad3/issues/3470) - Expand HTML lexer coverage / "full HTML support"
- [ ] **(Q3) BATCH Code Folding** - Fold `if`/`for` blocks with parentheses
  - Issue: [#4484](https://github.com/rizonesoft/Notepad3/issues/4484)
  - BUG: [#4959](https://github.com/rizonesoft/Notepad3/issues/4959) - Complex for loops highlighting
- [ ] **(Q1) Markdown: Highlight Trailing Double Spaces** - Show line break indicators
  - Issue: [#5312](https://github.com/rizonesoft/Notepad3/issues/5312)
- [ ] **(Q2) Custom Hyperlink Schemes** - User-defined URL protocol recognition
  - ed2k:// links: [#5405](https://github.com/rizonesoft/Notepad3/issues/5405)
  - Customizable via INI settings
- [x] **(Q2) Smarter URL Recognition** - Improve URL boundary detection - ✅ FIXED
  - Issue: [#5464](https://github.com/rizonesoft/Notepad3/issues/5464)
  - Don't include trailing `'` when URL is quoted
- [x] **(Q3) Display Hidden Characters** - Show invisible/control characters - ✅ IMPLEMENTED
  - Issue: [#5035](https://github.com/rizonesoft/Notepad3/issues/5035) - Non-printing chars like Notepad++ - ✅ IMPLEMENTED
  - Issue: [#5496](https://github.com/rizonesoft/Notepad3/issues/5496) - ⚠ Validation ❗
  - Scintilla `SCI_SETREPRESENTATION` for custom char display
- [ ] **(Q2) Trailing Whitespace Highlighting** - Distinct style for trailing blanks
  - Issue: [#1913](https://github.com/rizonesoft/Notepad3/issues/1913)
- [ ] **(Q3) Enhanced Auto-Complete** - Language-aware auto-complete triggers
- [x] **(Q3) Auto-Pair Brackets** - Auto-close `()`, `[]`, `{}`, `""`, `''`
  - [x] Issue: [#4149](https://github.com/rizonesoft/Notepad3/issues/4149) - ✅ FIXED
  - [ ] Issue: [#5285](https://github.com/rizonesoft/Notepad3/issues/5285) - Smart auto-dedent on closing bracket
    - this will be out of scope, if not supported by lexers itself
- [ ] **(Q3) Custom Keyboard Shortcuts** - User-configurable shortcut keys
  - Issue: [#595](https://github.com/rizonesoft/Notepad3/issues/595)
  - [x] Workaround: Use Microsoft PowerToys's Keyboard-Manager to redirect

## Open Discussions

- [x] **(Q2) BUG: Highlight current line broken** - Settings not respected (regression) - ✅ WON'T CHANGE
  - Issue: [#5270](https://github.com/rizonesoft/Notepad3/issues/5270)
  - **This is a discussion, about limited line highlite rule language in schema definition **
- [x] **(Q3) QUE: Regex replace issue** - Verify if still present - ✅ FIXED
  - Issue: [#3531](https://github.com/rizonesoft/Notepad3/issues/3531)
  - Was no Bug but bad RegEx pattern design (expectation vs. what regex really does)

### Text Processing
- [ ] **(Q1) Strip Leading Blanks** - Trim leading whitespace (NP3 only has trailing)
- [x] **(Q1) Right-Click Convert Case** - Add case conversion to context menu - ✅ IMPLEMENTED
  - Issue: [#5403](https://github.com/rizonesoft/Notepad3/issues/5403)
- [ ] **(Q2) LaTeX Input Method** - LaTeX character insertion (`\alpha` → α)
- [x] **(Q2) Base64 Encode/Decode** - ✅ IMPLEMENTED via `IDM_EDIT_BASE64ENCODE/DECODE`
- [x] **(Q1) Insert Unicode Control Characters** - LRM, RLM, ZWJ, ZWNJ, etc.
- [x] **(Q2) Number Base Conversion** - Binary/Decimal/Octal/Hex - ✅ IMPLEMENTED
  - Issue: [#5059](https://github.com/rizonesoft/Notepad3/issues/5059) - TinyExpr output in hex/binary - ✅ IMPLEMENTED
- [ ] **(Q2) Character Map Conversions** - Fullwidth↔Halfwidth, CJK transforms (`LCMapStringEx`)
- [x] **(Q1) Insert GUID** - ✅ IMPLEMENTED via `IDM_EDIT_INSERT_GUID` (Ctrl+Shift+.)
- [ ] **(Q1) Insert Shebang** - Insert interpreter line
- [x] **(Q1) Insert Timestamps** - ✅ IMPLEMENTED via `IDM_EDIT_INSERT_SHORTDATE/LONGDATE` (Ctrl+F5, Shift+F5) and `CMD_INSERT_TIMESTAMP`
- [ ] **(Q2) Time/Date Expressions in TinyExpr** - Support `time()` / date arithmetic in expression evaluator
  - Issue: [#4760](https://github.com/rizonesoft/Notepad3/issues/4760)
- [ ] **(Q1) Clarify "Remove Duplicate Lines" naming** - Two submenu entries have different meanings, confusing UX
  - Issue: [#5154](https://github.com/rizonesoft/Notepad3/issues/5154)

### Navigation
- [ ] **(Q3) Navigate Backward/Forward** - VS Code-like history navigation
  - Remark: maybe description of change-history navigation is a start.
- [x] **(Q2) Brace Find Enhancement** - Search backward for nearest brace when not at one  - ✅ IMPLEMENTED
  - Issue: [#4863](https://github.com/rizonesoft/Notepad3/issues/4863) - ✅ FIXED
- [ ] **(Q2) Go to Block Start/End** - Jump to enclosing block
  - [x] Fixed by Bracket navigation, incl. Selection
  - [ ] No solution for Python-like "Block by Indent" navigation
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
  - [x] ✅ IMPLEMENTED (show non printing chars)
  - [ ] Show other Unicode Control Characters ???
- [ ] **(Q2) Line Selection Modes** - VS style, Normal, Old VS
- [x] **(Q1) Disable Multiple Cursors Option** - ✅ IMPLEMENTED via `IDM_SET_MULTIPLE_SELECTION` toggle
  - Issue: [#4033](https://github.com/rizonesoft/Notepad3/issues/4033) - ✅ Resolved
- [x] **(Q1) Scroll Past Last Line Options** - ✅ IMPLEMENTED via `Settings.ScrollPastEOF` (simple on/off, not multiple options)
- [x] **(Q2) Optimize Syntax Scheme Dialog** - Show detected scheme at top of list - ✅ WON'T CHANGE
  - Issue: [#5302](https://github.com/rizonesoft/Notepad3/issues/5302)
- [ ] **(Q1) Alphabetical Common Base Settings** - Sort scheme settings alphabetically
  - Issue: [#4627](https://github.com/rizonesoft/Notepad3/issues/4627)
- [x] **(Q2) Find Dialog Position Persistence** - ✅ IMPLEMENTED via `FindReplaceDlgPosX/Y` saved to INI
  - Issue: [#3905](https://github.com/rizonesoft/Notepad3/issues/3905) - ✅ RESOLVED
- [ ] **(Q2) Configurable Font Priority/Fallback List** - User-editable preferred Code/Text font chain
  - Issue: [#4611](https://github.com/rizonesoft/Notepad3/issues/4611)
  - Related: https://github.com/zufuliu/notepad4/issues/690
- [ ] **(Q2) Improve Selection & Convert Panel layout**
  - Issue: [#5074](https://github.com/rizonesoft/Notepad3/issues/5074)
- [ ] **(Q2) Separate Recent Files Menu** - Split MRU out of File menu
  - Issue: [#5177](https://github.com/rizonesoft/Notepad3/issues/5177)
- [x] **(Q3) Discussion: Gray out menu items when no selection**
  - Issue: [#4938](https://github.com/rizonesoft/Notepad3/issues/4938)
- [x] **(Q3) Keep current line visible after Word-Wrap toggle**
  - Issue: [#4944](https://github.com/rizonesoft/Notepad3/issues/4944)
- [ ] **(Q2) MiniPath: "Minimize on Close" option**
  - Issue: [#4946](https://github.com/rizonesoft/Notepad3/issues/4946)

### Copy/Clipboard
- [x] **(Q2) Copy as RTF** - Rich text copy with syntax highlighting - ✅ IMPLEMENTED
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
- [x] **(Q1) Increment/Decrement Number** - Modify number at cursor (Ctrl+Alt++/-)
  - implemented via TinyExpr enhancement 
- [x] **(Q2) Show Hex View** - Display hex representation of selection
  - implemented via TinyExpr output format switch
- [ ] **(Q1) CSV Options Dialog** - Configure CSV delimiter/qualifier
  - [x] CSV Rainbow Lexer (home-brew) has an auto-detect-delimiter
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
- [x] **(Q1) Open Containing Folder** - Open the folder containing the current file in Explorer - ✅ IMPLEMENTED
  - **Reference**: [Notepad4](https://github.com/zufuliu/notepad4) implements via `CMD_OPEN_CONTAINING_FOLDER` and `EditOpenSelection(OpenSelectionType_ContainingFolder)`

### Windows Integration
- [ ] **(Q3) Explorer Preview Handler** - Preview text files in Explorer pane
  - Issue: [#5510](https://github.com/rizonesoft/Notepad3/issues/5510)
  - COM DLL implementing `IPreviewHandler` interface
  - Optional installer component
