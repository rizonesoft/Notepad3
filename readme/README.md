# Notepad3 Documentation

This folder collects the user-facing reference documentation for Notepad3 and its companion file browser **MiniPath**. The pages here cover end-user topics — configuration, command-line switches, shortcuts, file handling, encryption, theming, and FAQs — and are linked from the main project README and from the in-app *Help* menu.

> Looking for build instructions or contributor notes? See the project [README](../README.md) at the repository root and the [CLAUDE.md](../CLAUDE.md) developer guide. For installer / Windows-Notepad-replacement setup, see the [project website](https://rizonesoft.com/documents/notepad3/).

## Contents

### Quick references

- [KeyboardShortcuts.md](KeyboardShortcuts.md) — Complete keyboard-shortcut reference for the Notepad3 editor.
- [MenuEntriesAndCmds.md](MenuEntriesAndCmds.md) — Every command exposed by the menu tree and the context menus.

### Configuration & command line

- [config/Configuration.md](config/Configuration.md) — Full reference for the portable `Notepad3.ini` (`[Settings2]` keys, restart-required settings, etc.).
- [config/FileContentFlags.md](config/FileContentFlags.md) — Markers inside a file (mode lines, BOMs) that change how Notepad3 opens or styles it.
- [cmdln/CmdLnOptions.md](cmdln/CmdLnOptions.md) — Every command-line option, switch, and positional argument accepted by `Notepad3.exe`.

### File handling

- [paths/FilePathHandling.md](paths/FilePathHandling.md) — How Notepad3 turns text into a file: hyperlinks, dialogs, relative/UNC/long paths.
- [uchardet/EncodingDetection.md](uchardet/EncodingDetection.md) — Encoding auto-detection (uchardet + BOM/heuristic layering).
- [encryption/Encryption.md](encryption/Encryption.md) — Transparent AES-256 (Rijndael/CBC) encryption and the `np3encrypt` CLI.

### Editor features

- [focusedview/FocusedView.md](focusedview/FocusedView.md) — Folding-based filter view showing only lines that match the marked word or selection.
- [tinyexprcpp/TinyExprPP.md](tinyexprcpp/TinyExprPP.md) — The embedded TinyExpr++ math/expression evaluator and where it is wired into the UI.
- [schema/CustomSchema.md](schema/CustomSchema.md) — Schemas, lexer style sets, the style mini-language, and theme import/export.

### MiniPath

- [minipath/KeyboardShortcuts.md](minipath/KeyboardShortcuts.md) — Keyboard reference for the MiniPath file browser (launched with `Ctrl+M`).

### FAQ

- [faq/FAQ.md](faq/FAQ.md) — Frequently asked questions, including Notepad2 / Notepad2-mod migration notes.
