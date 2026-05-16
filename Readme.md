# Notepad3

**A fast, lightweight, Scintilla-based text editor for Windows**

[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg?style=flat-square)](https://opensource.org/licenses/BSD-3-Clause)
[![CI](https://img.shields.io/github/actions/workflow/status/rizonesoft/Notepad3/build.yml?style=flat-square&label=CI)](https://github.com/rizonesoft/Notepad3/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/rizonesoft/Notepad3?style=flat-square&label=Release&color=0e7490)](https://rizonesoft.com/downloads/notepad3/)
[![Nightly](https://img.shields.io/github/v/release/rizonesoft/Notepad3?include_prereleases&style=flat-square&label=Nightly&color=6e40c9)](https://github.com/rizonesoft/Notepad3/releases)

[Website](https://rizonesoft.com/downloads/notepad3/) · [Downloads](https://github.com/rizonesoft/Notepad3/releases) · [Documentation](https://rizonesoft.com/documents/notepad3/) · [FAQ](readme/faq/FAQ.md) · [Changelog](https://rizonesoft.com/downloads/notepad3/update) · [Sponsor](https://github.com/sponsors/rizonesoft)

---

Notepad3 is a free, open-source text editor with syntax highlighting for Windows. Built on the [Scintilla](https://www.scintilla.org/) editing component and [Lexilla](https://www.scintilla.org/Lexilla.html) lexer library, it has a small memory footprint while being powerful enough for professional programming and everyday text editing.

> Notepad3 is based on code from Florian Balmer's [Notepad2](https://www.flos-freeware.ch/notepad2.html) and XhmikosR's [Notepad2-mod](https://xhmikosr.github.io/notepad2-mod/). MiniPath is based on Florian Balmer's metapath.

<!-- TODO: Add a screenshot of the Notepad3 main window here.
     Save the image as doc/screenshot.png and uncomment the line below:
     ![Notepad3 Screenshot](doc/screenshot.png)
-->

## Features

### Editing
- **Code folding** with toggle-all support
- **Bookmarks** for quick navigation within files
- **Word auto-completion** with configurable fill-up characters
- **Mark all occurrences** of a selected word with occurrence count display
- **Find and Replace** with [PCRE2](https://github.com/PCRE2Project/pcre2) regular expression engine
- **[Focused View](readme/focusedview/FocusedView.md)** — filter display to show only lines matching the current word/selection (Ctrl+Alt+V), with fold, bookmark, and highlight modes
- **[TinyExpr++](readme/tinyexprcpp/TinyExprPP.md)** — inline math expression evaluation with 80+ functions (type `expression=?` to evaluate in-place, use `${expr}` in line numbering).
- **Undo/Redo** that preserves selection state
- **Visual Studio–style** copy/paste of the current line (when nothing is selected)
- **Virtual space** rectangular selection (Alt+Drag)
- **Accelerated word navigation** with configurable delimiters
- **Insert GUIDs** directly into your document
- **Clipboard monitoring mode** (`/b` command-line switch) — automatically appends every clipboard change as a new entry at the end of the document; stop at any time via **Edit → Stop Clipboard Monitoring** without closing the editor.

### Syntax Highlighting
Over 55 languages supported, including:

> ANSI Art, Apache Config, Assembly, AutoHotkey, AutoIt3, AviSynth, Awk, Batch, C/C++, C#, CMake, CoffeeScript, CSS, CSV, D, Dart, Diff, F77/Fortran, Go, HTML/XML, Java, JavaScript, JSON, Julia, Kotlin, KiXtart, LaTeX, Lua, Makefiles, Markdown, MATLAB, Nim, NSIS, Pascal, Perl, PHP, PowerShell, Python, R/S-Plus, Registry, Resource Script, Ruby, Rust, Shell Script, SQL, SystemVerilog, Tcl, TOML, VBScript, VHDL, Verilog, Visual Basic, YAML, and more.

### File Handling
- **[AES-256 Rijndael](readme/encryption/Encryption.md)** encryption/decryption (in-app and command-line batch tool)
- **[Encoding detection](readme/uchardet/EncodingDetection.md)** powered by [uchardet](https://www.freedesktop.org/wiki/Software/uchardet/)
- **File change monitoring** with configurable check intervals
- **[`.LOG` auto-timestamp](readme/config/FileContentFlags.md#log-auto-timestamp)** — files whose first line is `.LOG` get a fresh date + time appended on open, matching classic Notepad behaviour
- **[Emacs file variables](readme/config/FileContentFlags.md#emacs-file-variables)** support (encoding, mode, tab-width, etc.)
- **File history** that preserves caret position and encoding
- **Portable design** — runs from USB drives with relative path storage

### Search
- **grepWin** — integrated search-in-files tool with regex support (**Ctrl+Shift+F**)
- **Hyperlink hotspot highlighting** — Ctrl+Click to open in browser, Alt+Click to load in editor

### User Interface
- **High-DPI aware** with high-definition toolbar icons
- **Dark mode** support with customizable colors
- **Customizable status bar** with 16 configurable fields (line, column, encoding, [TinyExpr++](readme/tinyexprcpp/TinyExprPP.md) evaluation, Unicode code point, and more)
- **Customizable toolbar labels** — display function names next to icons
- **Zoom** from 10% to 1000% (Ctrl+Scroll or toolbar buttons)
- **Transparent window mode** with configurable opacity

### Localization
26 language translations:

> Afrikaans, Belarusian, Chinese (Simplified & Traditional), Dutch, English (US & UK), Finnish, French, German, Greek, Hindi, Hungarian, Indonesian, Italian, Japanese, Korean, Polish, Portuguese (Brazil & Portugal), Russian, Slovak, Spanish, Swedish, Turkish, Vietnamese

### Companion Tools
- **[MiniPath](minipath/)** — fast file browser plugin (Ctrl+M)
- **[grepWin](grepWin/)** — powerful search-and-replace across files

## System Requirements

- **OS:** Windows 8.1, 10, or 11
- **Architectures:** x86 (Win32), x64, x64 with AVX2, ARM64

## Download & Install

| Channel | Link |
|---------|------|
| **Stable Release** | [rizonesoft.com/downloads/notepad3](https://rizonesoft.com/downloads/notepad3/) |
| **GitHub Releases** | [github.com/rizonesoft/Notepad3/releases](https://github.com/rizonesoft/Notepad3/releases) |
| **Nightly Builds** | [Pre-releases on GitHub](https://github.com/rizonesoft/Notepad3/releases) |

Notepad3 is fully **portable** — no installation required. Extract the archive and run `Notepad3.exe`. Settings are stored in `Notepad3.ini` alongside the executable. See [Replacing Windows Notepad](https://rizonesoft.com/documents/notepad3/) for system-wide integration.

## Building from Source

Notepad3 is built with **Visual Studio 2022** (toolset v143) targeting the Windows SDK.

```powershell
# Restore NuGet packages (required once)
nuget restore Notepad3.sln

# Build a specific platform
Build\Build_x64.cmd Release

# Build all platforms (Win32, x64, x64_AVX2, ARM64)
Build\BuildAll.cmd Release

# Or use MSBuild directly
msbuild Notepad3.sln /m /p:Configuration=Release /p:Platform=x64
```

Run `Version.ps1` before building to generate version headers from templates in `Versions\`.

## Configuration

Notepad3 uses a portable INI file for all settings. Press **Ctrl+F7** to open it directly in the editor.

- **UI settings** (`[Settings]`) — managed through Menu → Settings
- **Advanced settings** (`[Settings2]`) — edit manually; most require a restart
- **Status bar** (`[Statusbar Settings]`) — customize field layout, order, and width
- **Toolbar labels** (`[Toolbar Labels]`) — show function names next to icons

📖 **Full configuration reference:** [readme/config/Configuration.md](readme/config/Configuration.md)

🎨 **Schemas, styles & themes:** [readme/schema/CustomSchema.md](readme/schema/CustomSchema.md) — the layered override model, the style mini-language, `View → Customize Schemes` (**F12**), and how to export / import / collect custom themes.

📄 **File content flags** (`.LOG`, Emacs file variables, encoding tags, shebang lexer hint): [readme/config/FileContentFlags.md](readme/config/FileContentFlags.md) — markers you can put inside a file to control how Notepad3 opens or styles it.

## Command-Line Options

Notepad3 accepts a rich set of command-line switches inherited from Notepad2 / Notepad2-mod, plus several Notepad3-specific extensions. All switches are case-insensitive and may be prefixed with either `/` or `-`.

```bat
Notepad3.exe /utf8sig /crlf d:\temp\Test.txt        :: Open as UTF-8 BOM + CRLF
Notepad3.exe /g 250,5 src\Notepad3.c                :: Jump to line 250, column 5
Notepad3.exe /m TODO main.c                         :: Open and find first "TODO"
Notepad3.exe /b /i                                  :: Pasteboard mode, start in tray
                                                     :: (hold Ctrl during the brief
                                                     :: pre-minimize window to keep
                                                     :: the editor visible)
Notepad3.exe /v report.log                          :: Print silently and exit
Notepad3.exe /?                                     :: Built-in help dialog
```

Coverage at a glance:

- **File arguments & multi-file** — positional paths, `+`, `-`, `/y`, `/z`
- **Encoding** — `/ansi`, `/unicode`, `/unicodebe`, `/utf8`, `/utf8sig`, `/e <name>`
- **Line endings** — `/crlf`, `/cr`, `/lf`
- **Navigation & search** — `/g`, `/m` with regex / case / backslash modifiers
- **File watching** — `/l`, `/l0`, `/l1`
- **New documents** — `/q`, `/qs`, `/c`
- **Pasteboard / clipboard collector** — `/b` (with deferred-minimize and Ctrl-skip when combined with `/i`)
- **Lexers** — `/s`, `/d`, `/h`, `/x`
- **Window placement & behaviour** — `/p` (with presets), `/t`, `/i`, `/o`
- **Window reuse / single-instance** — `/n`, `/ns`, `/r`, `/rs`, `/rp`
- **Printing** — `/v`, `/vd`
- **INI & elevation** — `/f`, `/f0`, `/u`
- **Shell integration** — `appid=`, `sysmru=`

⌨️ **Full reference:** [readme/cmdln/CmdLnOptions.md](readme/cmdln/CmdLnOptions.md) — every switch with arguments, examples, persistence notes, and Notepad2/Notepad2-mod compatibility.

## Menus & Commands

User-facing description of every entry in Notepad3's menus and context menus, structured to mirror the in-app menu tree. Covers File, Edit, Search, View, Appearance, Settings, Help, plus the four context (right-click) menus — with the keyboard shortcut, behaviour notes, and links into the relevant deep-dive docs for each command.

📋 **Menu reference:** [readme/MenuEntriesAndCmds.md](readme/MenuEntriesAndCmds.md)

## File-Path Handling

How Notepad3 turns clickable links, selected text, and path-shaped tokens into real files — including bare paths with line specs (`file.c:42`, `file.c(42)`), `file:///` URLs, relative-path anchor rules (current document's directory → working directory), environment-variable expansion, and the two new selection commands **Open Containing Folder of Selection** / **Open File from Selection**.

🗂️ **File-path reference:** [readme/paths/FilePathHandling.md](readme/paths/FilePathHandling.md)

## Keyboard Shortcuts

Every key combination Notepad3 understands — file, editing, view, search, customization — grouped by menu, with Notepad2 → Notepad3 reassignments flagged.

⌨️ **Notepad3 shortcuts:** [readme/KeyboardShortcuts.md](readme/KeyboardShortcuts.md)
🗂️ **MiniPath shortcuts:** [readme/minipath/KeyboardShortcuts.md](readme/minipath/KeyboardShortcuts.md)

## FAQ

Common questions inherited from Notepad2 / Notepad2-mod, updated for Notepad3 — installation, portable settings, regex (PCRE2), encoding, file change notification, `.LOG` and Emacs file variables, replacing Windows Notepad, and more.

❓ **Frequently Asked Questions:** [readme/faq/FAQ.md](readme/faq/FAQ.md)

## Contributing

Contributions are welcome! Here's how to help:

1. **Report bugs** — [Open an issue](https://github.com/rizonesoft/Notepad3/issues) with steps to reproduce
2. **Submit pull requests** — Fork the repo, create a feature branch, and submit a PR against `master`
3. **Translate** — Add or improve translations in the `language\` directory
4. **Spread the word** — Star the repo and share with others

For support, visit [rizonesoft.com/contact-us](https://rizonesoft.com/contact-us).

## Credits

Notepad3 builds upon the work of:

| Project | Author | Role |
|---------|--------|------|
| [Notepad2](https://www.flos-freeware.ch/notepad2.html) | Florian Balmer | Original editor |
| [Notepad2-mod](https://xhmikosr.github.io/notepad2-mod/) | XhmikosR | Extended fork |
| [Scintilla](https://www.scintilla.org/) 5.5.8 | Neil Hodgson | Editing component |
| [Lexilla](https://www.scintilla.org/Lexilla.html) 5.4.6 | Neil Hodgson | Syntax highlighting |
| [PCRE2](https://github.com/PCRE2Project/pcre2) 10.47 | PCRE2 Project | Regex engine |
| [TinyExpr++](https://blake-madden.github.io/tinyexpr-plusplus/) | Blake Madden | Expression evaluator |
| [uchardet](https://www.freedesktop.org/wiki/Software/uchardet/) | Mozilla / freedesktop | Encoding detection |
| Fugue Icons | Yusuke Kamiyamane | Toolbar icons |

## Reviews

- [Notepad3 is an advanced text editor that supports many programming languages](https://www.ghacks.net/2020/08/11/notepad3-is-an-advanced-text-editor-that-supports-many-programming-languages/) — gHacks
- [Notepad3 review on Nsane Forums](https://www.nsaneforums.com/topic/382910-guidereview-notepad3-is-an-advanced-text-editor-that-supports-many-programming-languages/) — Nsane Forums

## License

Notepad3 is licensed under the [BSD 3-Clause License](License.txt).

Copyright © 2008–2026 [Rizonesoft](https://rizonesoft.com). All rights reserved.

---

[![Sponsor](https://img.shields.io/badge/Sponsor-%E2%9D%A4-ea4aaa?style=flat-square&logo=github)](https://github.com/sponsors/rizonesoft)
[![Donate](https://img.shields.io/badge/Donate-PayPal-blue.svg?style=flat-square&logo=paypal)](https://www.paypal.com/donate/?hosted_button_id=7UGGCSDUZJPFE)
