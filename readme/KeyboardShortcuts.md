# Notepad3 — Keyboard Shortcuts

A complete reference for the shortcuts wired into Notepad3 (`src/Notepad3.rc` accelerator table). The in-menu hint after each command is the live source of truth — if a future build moves a binding, the menu shows the current key first.

> Looking for a different keyboard reference?
> - **MiniPath** (the file-browser plugin, Ctrl+M): see [`minipath/KeyboardShortcuts.md`](minipath/KeyboardShortcuts.md).
> - **Command-line switches**: see [`cmdln/CmdLnOptions.md`](cmdln/CmdLnOptions.md).
> - **Regex syntax** (used in Find / Replace): see the section *"Regular Expression Syntax"* of [`cmdln/CmdLnOptions.md`](cmdln/CmdLnOptions.md), or the bundled `Build/Docs/Oniguruma_RE.txt`.
> - **What changed since Notepad2**: see the *"Migrating from Notepad2"* section of the [FAQ](faq/FAQ.md).

---

## Conventions

- **Modifier order** in this document: `Ctrl` → `Shift` → `Alt`. Notepad3 itself accepts any order.
- **Key spellings:** `←` `→` `↑` `↓` are the cursor arrows. `+` and `-` mean the regular keyboard keys; numeric-keypad equivalents are noted as `Num+`, `Num-`, `Num0`, etc., where they have a separate binding.
- **NP2** in the *Notes* column means "carried over unchanged from Notepad2"; **(orig-NP2: …)** means "this slot held a different action in Notepad2 — see the FAQ migration table for the full list".
- **Customising shortcuts** is not supported in Notepad3 (the table is compiled in). See the FAQ for workarounds.

---

## File

### File operations

| Shortcut | Action | Notes |
|---|---|---|
| `Ctrl+N` | **Smart New** — clears the current document by default, but if the selection / token at caret resolves to an **existing file**, loads it into the current window instead; if it resolves to a **directory**, opens the file-open dialog rooted there. Path detection matches *Open File from Selection* (see [`paths/FilePathHandling.md`](paths/FilePathHandling.md#2-selection-based-commands)). | NP2+ |
| `Ctrl+F4` | Smart New (alias for `Ctrl+N`) | Inherits the same smart selection-aware behavior. (No MDI in Notepad3.) |
| `Ctrl+O` | Open file… | NP2 |
| `F5` | Reload current file from disk | NP2 |
| `Ctrl+S` | Save | NP2 |
| `F6` | Save as… | NP2 |
| `Ctrl+F6` | Save a copy… | NP2 |
| `Ctrl+Alt+F6` | Save preserving original modification time | (N3) |
| `Ctrl+P` | Print… | NP2 |
| `Ctrl+M` | Run MiniPath file browser | NP2 |
| `Ctrl+Alt+H` | Open recent file (history)… | (orig-NP2: `Alt+H`) |
| `Alt+F4` | Exit Notepad3 | Standard Windows |
| `Esc` | Cancel auto-completion popup or active selection. If neither is active, optionally minimise / exit (configurable). | NP2 |
| `Shift+Esc` | Save current file and exit Notepad3 | NP2 |

### Path to clipboard

| Shortcut | Action | Notes |
|---|---|---|
| `Ctrl+F9` | Copy filename only to clipboard | The IDM constant is named `INSERT_FILENAME` for historical reasons, but the action copies — it does not insert. |
| `Ctrl+Shift+F9` | Copy full path (directory + filename) to clipboard | Same naming caveat. |
| `Shift+F9` | Copy pathname to clipboard (legacy binding) | Identical to `Ctrl+Shift+F9` for saved files. For untitled documents, this one copies the localised "Untitled" placeholder. |

### Launch / external tools

| Shortcut | Action | Notes |
|---|---|---|
| `Ctrl+Shift+F` | Search in files (grepWin) | (orig-NP2: "Recode as default encoding") |
| `Alt+N` | **Smart New Window** — opens a new empty Notepad3 window by default, but if the selection / token at caret resolves to an **existing file**, spawns the new window with that file loaded (jumping to the line spec if any); if it resolves to a **directory**, the new window opens its file-open dialog there. Same path-detection rule as *Open File from Selection*. | NP2+ |
| `Alt+Shift+N` | Duplicate current document into a new window (only when *Single File Instance* is off) | (N3) |
| `Ctrl+L` | Launch document with its associated application | NP2 |
| `Alt+L` | "Open with…" external program | NP2 |
| `Ctrl+R` | Run command… | NP2 |
| `Ctrl+Shift+1` | Web template 1 (configurable URL — pass selection / file path) | (N3) |
| `Ctrl+Shift+2` | Web template 2 | (N3) |

### Encoding

| Shortcut | Action | Notes |
|---|---|---|
| `F8` | Recode (re-decode) — pick the source encoding to reload as | NP2. Internally loaded text is kept as UTF-8; this changes how the bytes were interpreted. |
| `F9` | Open the *Select Current Encoding* dialog | NP2 |
| `Shift+F8` | Reload as **UTF-8** | NP2 |
| `Ctrl+Shift+F8` | Reload an ASCII file as UTF-8 | NP2 |
| `Alt+F8` | Reload **without** parsing encoding tags or file variables | (N3) — useful when *mode: auto* etc. mis-triggers a lexer |
| `Ctrl+Shift+A` | Recode as system **ANSI** | NP2 |
| `Ctrl+Shift+O` | Recode as system **OEM** | NP2 |
| `Ctrl+Shift+F10` | Recode as **GB18030** | (N3) |

### Favorites

| Shortcut | Action | Notes |
|---|---|---|
| `Alt+I` | Open favorites… | NP2 |
| `Alt+K` | Add current file to favorites | NP2 |
| `Alt+F9` | Manage favorites… | NP2 |

---

## Edit

### Basic clipboard / undo

| Shortcut | Action | Notes |
|---|---|---|
| `Ctrl+Z` | Undo | NP2 |
| `Alt+Backspace` | Undo (alt) | NP2 |
| `Ctrl+Shift+Y` | Undo (alt) | NP2 |
| `Ctrl+Y` | Redo | NP2 |
| `Ctrl+Shift+Z` | Redo (alt) | NP2 |
| `Ctrl+X` | Cut selection — or **cut current line** if no selection (Visual-Studio style) | (orig-NP2: cut only when something selected) |
| `Shift+Del` | Cut (alt) | NP2 |
| `Ctrl+C` | Copy selection — or **copy current line** if no selection | (orig-NP2: copy only when something selected) |
| `Alt+C` | Copy entire document | NP2 |
| `Ctrl+E` | Copy add (append selection to clipboard) | NP2 |
| `Ctrl+V` | Paste | NP2 |
| `Shift+Ins` | Paste (alt) | NP2 |
| `Ctrl+K` | Swap clipboard with selection | NP2 |
| `Del` | Clear selection | NP2 |
| `Ctrl+A` | Select all | NP2 |
| `Insert` | Toggle Insert / Overtype mode | Standard Scintilla; not advertised by the menu |

### Words

| Shortcut | Action | Notes |
|---|---|---|
| `Ctrl+Alt+Enter` | Complete word (autocomplete from buffer) | (N3) |
| `Ctrl+←` | Move caret one word left | NP2 |
| `Ctrl+→` | Move caret one word right | NP2 |
| `Ctrl+Backspace` | Delete word **before** the caret | NP2 |
| `Ctrl+Del` | Delete word **after** the caret | NP2 |

### Lines

| Shortcut | Action | Notes |
|---|---|---|
| `Alt+↑` | Move current line / selected block **up** | NP2 |
| `Alt+↓` | Move current line / selected block **down** | NP2 |
| `Ctrl+Shift+X` | Cut whole line(s) — even when only part is selected | NP2 |
| `Ctrl+Shift+C` | Copy whole line(s) — even when only part is selected | NP2 |
| `Ctrl+D` | Duplicate selection — or current line if no selection | NP2 |
| `Alt+D` | Transpose lines (swap current with the one above) | NP2 |
| `Ctrl+Shift+D` | Delete current line | NP2 |
| `Ctrl+Shift+Backspace` | Delete from caret to start of line | NP2 |
| `Ctrl+Shift+Del` | Delete from caret to end of line | NP2 |
| `Ctrl+Shift+W` | Column wrap — break long lines at a column | NP2 |
| `Ctrl+I` | Split lines at spaces (rebreak text) | NP2 |
| `Ctrl+J` | Join lines (single-space joiner) | NP2 |
| `Ctrl+Alt+J` | Fuse lines (no space inserted) | (N3) |
| `Ctrl+Shift+J` | Join lines, preserve paragraph breaks | NP2 |
| `Ctrl+Alt+D` | Unite duplicate lines (remove duplicates within selection) | (N3) |
| `Alt+Y` | Merge consecutive empty lines into one | NP2 |
| `Ctrl+Alt+Y` | Merge consecutive blank-or-whitespace-only lines | (N3) |
| `Alt+R` | Remove all empty lines from selection | NP2 |
| `Ctrl+Alt+B` | Remove all blank-or-whitespace-only lines | (N3) |

### Block & indent

| Shortcut | Action | Notes |
|---|---|---|
| `Tab` | Indent selected block — or accept autocompletion when its popup is open, or insert a tab when caret is in plain text | NP2 |
| `Shift+Tab` | Unindent selected block | NP2 |
| `Ctrl+Tab` | Insert a literal tab character (bypasses indent reformat) | NP2 |
| `Alt+B` | Pad lines with trailing spaces to longest-line width | NP2 |
| `Alt+Z` | Strip first character of each line | NP2 |
| `Alt+U` | Strip last character of each line | NP2 |
| `Alt+W` | Strip trailing blanks | NP2 |
| `Ctrl+Alt+P` | Compress runs of blanks to single space | NP2 |
| `Alt+M` | Modify lines (prefix / suffix dialog)… | NP2 |
| `Alt+J` | Align lines… | NP2 |
| `Alt+O` | Sort lines… | NP2 |
| `Ctrl+,` | Jump to selection start | NP2 |
| `Ctrl+.` | Jump to selection end | NP2 |

### Comments

| Shortcut | Action | Notes |
|---|---|---|
| `Ctrl+Q` | Toggle line comment | NP2 |
| `Ctrl+/` (or `Ctrl+Num/`) | Toggle line comment (alternative binding) | (N3) |
| `Ctrl+Alt+Q` | Add line comment (without toggling) | (N3) |
| `Ctrl+Shift+Alt+Q` | Remove line comment (without toggling) | (N3) |
| `Alt+Shift+Q` | Block-edit line comments (multi-caret edit at comment start of each line) | (N3) |
| `Ctrl+Shift+Q` | Toggle stream / block comment | NP2 |
| `Ctrl+Shift+/` (or `Ctrl+Shift+Num/`) | Toggle stream comment (alternative binding) | (N3) |

### Enclose selection

| Shortcut | Action |
|---|---|
| `Ctrl+1` | Enclose within `''` (single quotes) |
| `Ctrl+2` | Enclose within `""` (double quotes) |
| `Ctrl+3` | Enclose within `()` |
| `Ctrl+4` | Enclose within `[]` |
| `Ctrl+5` | Enclose within `{}` |
| `Ctrl+6` | Enclose within `` `` `` (backticks) |
| `Alt+Q` | Enclose with custom strings (before / after dialog)… |

### Convert (case, encoding, hex)

| Shortcut | Action | Notes |
|---|---|---|
| `Ctrl+Shift+U` | Selection → UPPERCASE | NP2 |
| `Ctrl+U` | Selection → lowercase | NP2 |
| `Ctrl+Alt+U` | Invert case | NP2 |
| `Ctrl+Alt+I` | Title Case | NP2 |
| `Ctrl+Alt+O` | Sentence case | NP2 |
| `Ctrl+Shift+T` | Convert spaces to tabs (everywhere in selection) | NP2 |
| `Ctrl+Shift+S` | Convert tabs to spaces (everywhere in selection) | NP2 |
| `Ctrl+Alt+T` | Convert **indent** spaces to tabs only | NP2 |
| `Ctrl+Alt+S` | Convert **indent** tabs to spaces only | NP2 |
| `Ctrl+Shift+E` | URL-encode selection | NP2 |
| `Ctrl+Shift+R` | URL-decode selection | NP2 |
| `Ctrl+F10` | File-system path → URL (e.g. `C:\path` → `file:///C:/path`) | (N3) |
| `Alt+F10` | URL → file-system path | (N3) |
| `Ctrl+Alt+X` | Char → hex code | NP2 |
| `Ctrl+Alt+C` | Hex code → char | NP2 |
| `Ctrl+Alt+E` | Escape C special chars (`\n` etc.) | NP2 |
| `Ctrl+Alt+R` | Unescape C special chars | NP2 |

### Insert

| Shortcut | Action | Notes |
|---|---|---|
| `Ctrl+Enter` | Insert new line **above** current line | (orig-NP2: new line with toggled auto-indent) |
| `Ctrl+Shift+Enter` | New line with **toggled** auto-indent option | (orig-NP2: `Ctrl+Enter`) |
| `Alt+X` | Insert HTML / XML tag… | NP2 |
| `Ctrl+F8` | Insert encoding identifier comment | NP2 |
| `Ctrl+F5` | Insert short date/time | NP2 |
| `Ctrl+Shift+F5` | Insert long date/time | NP2 |
| `Shift+F5` | Update existing timestamps in document (regex-driven) | NP2. Pattern is `[Settings2] TimeStampRegEx`. |

### Numbers

| Shortcut | Action | Notes |
|---|---|---|
| `Ctrl+Alt++` (or `Ctrl+Alt+Num+`) | Increase the number under the caret | NP2 |
| `Ctrl+Alt+-` (or `Ctrl+Alt+Num-`) | Decrease the number under the caret | NP2 |

### Find, Replace, Goto

| Shortcut | Action | Notes |
|---|---|---|
| `Ctrl+F` | Find… | NP2 |
| `Alt+F3` | Save the current find text as the persistent default | NP2 |
| `F3` | Find next | NP2 |
| `Shift+F3` | Find previous | NP2 |
| `Ctrl+F3` | Find next occurrence of the current selection | NP2 |
| `Ctrl+Shift+F3` | Find previous occurrence of the current selection | NP2 |
| `Ctrl+H` | Replace… | NP2 |
| `F4` | Replace next | NP2 |
| `Ctrl+G` | Goto line / column dialog | NP2 |
| `Ctrl+Shift+F` | Search in files (grepWin) | (orig-NP2: "Recode as default") |
| `Ctrl+B` | **Find matching brace** — if the caret is at a brace `()[]{}` jump to its match; if not at a brace, jump to the nearest enclosing opening brace. If a brace-block selection is active, the caret toggles between the two brace endpoints while preserving the selection. Shows a calltip if no match or no enclosing brace exists. | NP2+ |
| `Ctrl+Shift+B` | **Select to matching brace** — selects the brace pair enclosing the caret (inclusive of both braces). Repeated presses expand the selection outward one nesting level at a time. Pressing when the caret is at the closing brace of the current selection pivots the caret to the opening brace (same selection, reversed direction); pressing again when at the opening brace expands to the next outer pair. Shows a calltip if no match or no enclosing brace exists. | NP2+ |
| `Ctrl+Alt+F2` | Expand selection to next match | (orig-NP2: `F2`) |
| `Ctrl+Alt+Shift+F2` | Expand selection to previous match | (orig-NP2: `Shift+F2`) |

#### Inside the Find / Replace dialog

A dedicated accelerator table applies while the modeless Find/Replace window has focus:

| Shortcut | Action |
|---|---|
| `Ctrl+F` | Switch to *Find* tab |
| `Ctrl+H` | Switch to *Replace* tab |
| `F3` | Find next (without leaving the dialog) |
| `Shift+F3` | Find previous |
| `Alt+F3` | Save current pattern as default |
| `F4` | Replace next |
| `Ctrl+S` | Save dialog window position |
| `Ctrl+R` | Reset dialog window position |

### Bookmarks

| Shortcut | Action | Notes |
|---|---|---|
| `F2` | Goto next bookmark | (orig-NP2: expand selection to next match — moved to `Ctrl+Alt+F2`) |
| `Shift+F2` | Goto previous bookmark | (orig-NP2: expand selection to previous match) |
| `Ctrl+F2` | Toggle bookmark on current line | (N3) |
| `Alt+F2` | Clear all bookmarks | (N3) |

### Selection

| Shortcut | Action | Notes |
|---|---|---|
| `Ctrl+Space` | Select word at caret — or whole line if word already selected | NP2 |
| `Ctrl+Shift+Space` | Multi-select all matches of word at caret | (N3) |
| `Alt+Shift+Arrows` | Rectangular selection | NP2 |
| `Alt+Left-mouse-drag` | Rectangular selection | NP2 |
| `Ctrl+Shift+.` | Copy GUID to clipboard | (N3). The IDM is named `INSERT_GUID` for historical reasons but the action copies. |

### Mouse

| Action | Effect |
|---|---|
| `Ctrl`-click on a hyperlink / URL hotspot | Open in default browser |
| `Alt`-click on a hyperlink | Load target as a file in Notepad3 |
| `Ctrl`-click on a colour swatch (when colour-hotspot highlighting is on) | Open the colour picker |
| `Ctrl`-mouse-wheel | Zoom in / out |

---

## View

### Visual aids

| Shortcut | Action | Notes |
|---|---|---|
| `Ctrl+W` | Toggle word wrap | NP2 |
| `Ctrl+Alt+L` | Toggle long-line marker | NP2 |
| `Ctrl+Shift+G` | Toggle indentation guides | NP2 |
| `Ctrl+Shift+7` | Toggle wrap symbols | NP2 |
| `Ctrl+Shift+8` | Toggle whitespace markers | NP2 |
| `Ctrl+Shift+9` | Toggle line-ending markers | NP2 |
| `Ctrl+Alt+W` | Toggle hyperlink hotspots | (N3) |
| `Ctrl+Shift+V` | Toggle visual brace matching | NP2 |
| `Ctrl+Shift+I` | Highlight current line | NP2 |
| `Ctrl+Shift+N` | Toggle line numbers | NP2 |
| `Ctrl+Shift+M` | Toggle bookmark margin | NP2 |

### Mark occurrences

| Shortcut | Action |
|---|---|
| `Alt+A` | Toggle "mark all occurrences" of current word |

### Folding

| Shortcut | Action |
|---|---|
| `Ctrl+Alt+F` | Toggle all folds (collapse / expand all) |
| `Alt++` | Jump to next fold point |
| `Alt+-` | Jump to previous fold point |
| `Alt+←` | Collapse current fold |
| `Alt+→` | Expand current fold |

### Display navigation

| Shortcut | Action | Notes |
|---|---|---|
| `Ctrl+Alt+V` | Focused View — show only lines matching word/selection | (N3) |
| `Ctrl+PgUp` / `Ctrl+PgDn` | Goto previous / next block (paragraph or fold) | NP2 |
| `Ctrl+Shift+PgUp` / `Ctrl+Shift+PgDn` | Select to previous / next block | NP2 |
| `Alt+PageUp` | Paragraph navigation up | (N3) |
| `Alt+PageDown` | Paragraph navigation down | (N3) |
| `Ctrl+↑` | Scroll one line up (without moving caret) | NP2 |
| `Ctrl+↓` | Scroll one line down (without moving caret) | NP2 |

### Zoom

| Shortcut | Action |
|---|---|
| `Ctrl++` (or `Ctrl+Num+`) | Zoom in |
| `Ctrl+-` (or `Ctrl+Num-`) | Zoom out |
| `Ctrl+0` (or `Ctrl+Num0`) | Reset zoom to 100 % |

### Window position

| Shortcut | Action | Notes |
|---|---|---|
| `Ctrl+Shift+K` | Copy current window position to clipboard as `/p x,y,cx,cy` switch | NP2 |
| `Ctrl+Shift+P` | Snap window to default position on current monitor | NP2 |
| `Ctrl+F11` | Snap window to internal initial-default position | (N3) |
| `F11` | Toggle full-area window (full-screen) | (N3) |

---

## Appearance

| Shortcut | Action | Notes |
|---|---|---|
| `F12` | Select syntax scheme for current file | NP2 |
| `Ctrl+F12` | Customize schemes (colours, fonts, …) | NP2 |
| `Shift+F12` | Switch to the secondary common base scheme | NP2 |
| `Shift+F11` | Force *Default Text* (no syntax) on current file | (N3) |
| `Alt+F12` | Change the global default font | NP2 |
| `Ctrl+Alt+F12` | Change the **active scheme's** default font | (N3) |

---

## Settings

| Shortcut | Action | Notes |
|---|---|---|
| `Ctrl+T` | Tab settings (tab width, indent width, tabs↔spaces)… | NP2 |
| `Ctrl+Shift+H` | Toggle automatic close of HTML / XML tags | NP2 |
| `Ctrl+Alt+A` | Toggle accelerated word navigation (alternate word separators) | (N3) |
| `Alt+F5` | File-change notification settings… | NP2 |

### Window-level toggles

| Shortcut | Action | Notes |
|---|---|---|
| `Ctrl+Shift+L` | Toggle "reuse window" for next launch | NP2 |
| `Alt+T` | Toggle "always on top" | NP2 |
| `Alt+G` | Toggle transparent mode | NP2 |
| `Ctrl+9` | Display selected text excerpt in window title | NP2 |

### Configuration file

| Shortcut | Action |
|---|---|
| `F7` | Save settings now (write `Notepad3.ini`) |
| `Ctrl+F7` | Open `Notepad3.ini` in the editor |

---

## Help

| Shortcut | Action |
|---|---|
| `F1` | Open online documentation |
| `Shift+F1` | About dialog |

---

## Cheatsheet — function keys at a glance

A compact summary of the function-key family (most frequent collisions):

| Key | Plain | Ctrl | Shift | Alt | Ctrl+Shift | Ctrl+Alt | Ctrl+Shift+Alt |
|---|---|---|---|---|---|---|---|
| `F1` | Online docs | — | About | — | — | — | — |
| `F2` | Goto next bookmark | Toggle bookmark | Goto prev bookmark | Clear all bookmarks | — | Expand sel to next match | Expand sel to prev match |
| `F3` | Find next | Find next selection | Find previous | Save find text | Find prev selection | — | — |
| `F4` | Replace next | New (clear) doc | — | Exit | — | — | — |
| `F5` | Reload file | Insert short date | Update timestamps | File-change notify | Insert long date | — | — |
| `F6` | Save as | Save copy | — | — | — | Save preserving mtime | — |
| `F7` | Save settings now | Open INI | — | — | — | — | — |
| `F8` | Recode (encoding) | Insert encoding ID | Reload as UTF-8 | Skip encoding tags | Reload ASCII as UTF-8 | — | — |
| `F9` | Encoding dialog | Copy filename | Copy path | Manage favorites | Copy full path | — | — |
| `F10` | (system menu) | Path → URL | — | URL → Path | Recode GB18030 | — | — |
| `F11` | Fullscreen toggle | Initial position | Default text scheme | — | — | — | — |
| `F12` | Pick scheme | Customize schemes | 2nd common base | Default font | — | Scheme default font | — |

---

## Cheatsheet — caret & view navigation

A single-stop reference for moving the caret, scrolling the view, and jumping between document landmarks. Repeats keys already listed in the topical sections above so the cheatsheet stands alone. Notepad3 inherits Scintilla's default keymap unchanged (no `SCI_ASSIGNCMDKEY` overrides), so the standard caret bindings below all work even though they are not part of Notepad3's accelerator table.

### Caret movement (basics)

| Shortcut | Action |
|---|---|
| `←` / `→` | Char left / right |
| `↑` / `↓` | Line up / down |
| `Home` / `End` | Line start / end |
| `Ctrl+Home` / `Ctrl+End` | Document start / end |
| `PageUp` / `PageDown` | Page up / down |
| `Shift+`*movement* | Extend selection in that direction |
| `Ctrl+Shift+`*movement* | Extend selection by word / document chunk |

### Word navigation

| Shortcut | Action |
|---|---|
| `Ctrl+←` / `Ctrl+→` | Caret one word left / right |
| `Ctrl+Backspace` | Delete word before caret |
| `Ctrl+Del` | Delete word after caret |
| `Ctrl+Alt+A` | Toggle **Accelerated Word Navigation** (see below) |
| `Ctrl+Space` | Select word at caret (full line if word already selected) |
| `Ctrl+Shift+Space` | Multi-select all matches of word at caret |

**Accelerated Word Navigation** (`Ctrl+Alt+A`) merges punctuation into the word-character set, so word-jump keys (`Ctrl+←`, `Ctrl+→`, `Ctrl+Backspace`, `Ctrl+Del`) skip whole code tokens at once.

Example — caret stops in `foo->bar.baz` walking with `Ctrl+→`:

- off: `foo` &nbsp;|&nbsp; `->` &nbsp;|&nbsp; `bar` &nbsp;|&nbsp; `.` &nbsp;|&nbsp; `baz`   (5 stops)
- on:  `foo->bar.baz`   (1 stop)

Fine-tune which characters still break words via `[Settings2] ExtendedWhiteSpaceChars=` — see [`config/Configuration.md`](config/Configuration.md). The toggle is persisted in `[Settings] AccelWordNavigation=`.

### View / scroll

| Shortcut | Action |
|---|---|
| `Ctrl+↑` / `Ctrl+↓` | Scroll one line up / down (caret stays put) |
| `Ctrl+PgUp` / `Ctrl+PgDn` | Goto previous / next block (paragraph or fold) |
| `Ctrl+Shift+PgUp` / `Ctrl+Shift+PgDn` | Select to previous / next block |
| `Alt+PageUp` / `Alt+PageDown` | Paragraph navigation up / down |
| `Ctrl+Alt+V` | Focused View — show only lines matching word/selection |
| `Ctrl+G` | Goto line / column dialog |

### Bookmarks & change history

| Shortcut | Action |
|---|---|
| `F2` | Goto next bookmark — falls through to next change-history marker if no further bookmark is found (sticky) |
| `Shift+F2` | Goto previous bookmark — same fallback |
| `Ctrl+F2` | Toggle bookmark on current line |
| `Alt+F2` | Clear all bookmarks |

If the document has no bookmarks (or none ahead of the caret in the search direction), `F2` / `Shift+F2` walk **change-history markers** instead — modified, saved, and reverted lines shown in the change-history margin (see *View → Change History*). Once a press has landed the caret on a change-history marker, repeated presses keep walking change-history markers until you reach a bookmarked line.

### Find navigation

| Shortcut | Action |
|---|---|
| `F3` | Find next |
| `Shift+F3` | Find previous |
| `Ctrl+F3` | Find next occurrence of current selection |
| `Ctrl+Shift+F3` | Find previous occurrence of current selection |
| `F4` | Replace next |
| `Ctrl+Alt+F2` | Expand selection to next match |
| `Ctrl+Alt+Shift+F2` | Expand selection to previous match |
| `Alt+A` | Toggle "mark all occurrences" of current word |

### Brace navigation

| Shortcut | Action |
|---|---|
| `Ctrl+B` | Find matching brace (or jump to nearest enclosing opening brace) |
| `Ctrl+Shift+B` | Select to matching brace; repeated presses expand the selection outward one nesting level at a time |

### Folding

| Shortcut | Action |
|---|---|
| `Alt+←` / `Alt+→` | Collapse / expand current fold |
| `Alt+-` / `Alt++` | Jump to previous / next fold point |
| `Ctrl+Alt+F` | Toggle all folds |

### Selection-boundary jumps

| Shortcut | Action |
|---|---|
| `Ctrl+,` | Jump caret to selection start |
| `Ctrl+.` | Jump caret to selection end |

---

## See also

- [`faq/FAQ.md`](faq/FAQ.md) — *"Migrating from Notepad2"* lists every shortcut whose meaning was reassigned, plus the **why**.
- [`cmdln/CmdLnOptions.md`](cmdln/CmdLnOptions.md) — full command-line reference.
- [`config/Configuration.md`](config/Configuration.md) — `[Settings2]` keys that affect editor behaviour.
- [`minipath/KeyboardShortcuts.md`](minipath/KeyboardShortcuts.md) — keyboard shortcuts for the bundled MiniPath file browser.
- `Build/Docs/KeyboardShortcuts.txt` — the plain-text version that ships with releases (auto-derived; this Markdown is the canonical edited copy).
