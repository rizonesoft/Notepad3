# Notepad3 — Menu Entries & Commands

A user-facing reference for every command exposed by Notepad3's menus and context menus. The structure of this document mirrors the in-app menu tree (`File`, `Edit`, `Search`, `View`, `Appearance`, `Settings`, `Help`, plus the context menus). The shortcut shown in each menu is the live source of truth — if a future build moves a binding, the menu wins.

> Related references
> - **All keyboard shortcuts** in one table: [`KeyboardShortcuts.md`](KeyboardShortcuts.md).
> - **Command-line switches**: [`cmdln/CmdLnOptions.md`](cmdln/CmdLnOptions.md).
> - **`Notepad3.ini` reference** (every `[Settings2]` key referenced below): [`config/Configuration.md`](config/Configuration.md).
> - **Focused View** in depth: [`focusedview/FocusedView.md`](focusedview/FocusedView.md).
> - **Encryption** in depth: [`encryption/Encryption.md`](encryption/Encryption.md).
> - **File-path handling** — hyperlinks, selection commands, line/column suffixes, anchor rules: [`paths/FilePathHandling.md`](paths/FilePathHandling.md).
> - **MiniPath** (the bundled file browser launched with `Ctrl+M`): [`minipath/`](minipath/).
> - **FAQ**, including the Notepad2 migration table: [`faq/FAQ.md`](faq/FAQ.md).

## Conventions used in this document

- **Shortcut** column shows the binding advertised by the menu. Modifier order in this document is `Ctrl` → `Shift` → `Alt`; Notepad3 itself accepts any order.
- A trailing **…** in the menu text indicates the command opens a dialog.
- "Selection or current line" means *if nothing is selected, the action targets the line under the caret* (Visual-Studio-style fallback).
- "Toggle" items are checkable — the menu shows a check mark when active.
- Greyed-out items are state-dependent (no selection, no document on disk, read-only file, conflicting mode active, etc.). The relevant precondition is noted in the description.

---

## File

File-system operations on the active document, plus a launcher for external tools and favorites.

### Open / save

| Menu | Shortcut | Description |
|---|---|---|
| **New** | `Ctrl+N` | Clear the current document. Prompts to save first if there are unsaved changes. |
| **Open…** | `Ctrl+O` | Open a file via the standard Windows file picker. |
| **Save** | `Ctrl+S` | Write the document to disk. For untitled documents this redirects to *Save As…*. |
| **Save As…** | `F6` | Save the document under a new path. |
| **Save Copy…** | `Ctrl+F6` | Save a copy without changing the editor's tracked path (the current document remains the original). |
| **Save with Original File Date/Time** | `Ctrl+Alt+F6` | Save like *Save*, but restore the original last-modified timestamp on disk afterwards. Useful when adjusting a file's contents without disturbing timestamp-based bookkeeping (build systems, backups). |
| **Read Only File Attribute** | — | Toggle the **Windows `FILE_ATTRIBUTE_READONLY`** bit on the file on disk. This is a persistent file-system flag — other applications see it too. (Distinct from *Document Read Only* in the Edit menu, which only affects this editor session.) |
| **Lock File (Read-Shared)** | — | Open the file with an exclusive write lock so that other processes cannot save over it while Notepad3 has it open (others may still read). Useful for streaming logs you want to read but not have rewritten under you. |
| **Set Encryption Passphrase…** | — | Set / change / clear the per-file passphrase used for `Crypt File` (.cryptedit) operations. See [`encryption/Encryption.md`](encryption/Encryption.md) for the full encryption workflow. |

### Path to Clipboard

Copies path information about the current file to the clipboard. The IDM constants are named `INSERT_*` for historical reasons (carried over from Notepad2), but the actions **copy** — they do not insert into the document.

| Menu | Shortcut | Description |
|---|---|---|
| **Copy Filename only** | `Ctrl+F9` | Just the filename, no directory. |
| **Copy Directory-Path only** | — | Just the parent directory. |
| **Copy Full-Path** | `Ctrl+Shift+F9` | Full absolute path. |

For untitled documents the bindings copy the localised "Untitled" placeholder.

### Launch

External actions that operate on the current document or its location.

| Menu | Shortcut | Description |
|---|---|---|
| **Search in Files…** | `Ctrl+Shift+F` | Open grepWin pre-targeted at the current file's directory. grepWin is a bundled portable exe — see *grepWin Integration* in [`config/Configuration.md`](config/Configuration.md) for path-resolution details. |
| **New Empty Window** | `Alt+N` | Launch a new Notepad3 process with an empty document. |
| **Duplicate Instance** | `Alt+Shift+N` | Launch a new Notepad3 process with a copy of the current document (only when *Single File Instance* is off). |
| **Relaunch Elevated** | — | Restart Notepad3 with administrator privileges, re-opening the current document with its current encoding and line-ending mode preserved. Greyed out if already running elevated. |
| **Execute Document** | `Ctrl+L` | Run the current document through its shell association (e.g. open a `.html` file in the browser). |
| **Open with…** | `Alt+L` | Show the Windows *"Open with"* picker for the current file. |
| **Command…** | `Ctrl+R` | Open a free-form command-line dialog pre-populated with the current file's directory. |
| **Web Template 1 / 2** | `Ctrl+Shift+1` / `Ctrl+Shift+2` | Run a user-configurable web action. Templates and their menu labels are set via `[Settings2] WebTmplFilePath1/2`, `WebTmpl1MenuName/WebTmpl2MenuName` and friends in `Notepad3.ini`. Selection / file path can be substituted into the URL. See `config/Configuration.md`. |
| **Open File Explorer** | — | Open Windows Explorer with the current file pre-selected. |

### Revert / encoding / line endings

| Menu | Shortcut | Description |
|---|---|---|
| **Revert from File** | `F5` | Reload the document from disk, discarding unsaved changes. Prompts before discarding. |
| **Ignore File-Vars** | — | Toggle parsing of in-file directives (e.g. Emacs `mode:` / Vim `vim:` modelines) that override tab width, indentation, encoding, and word-wrap. While on, the next load uses *only* the global defaults. |

#### Encoding → Revert from File as

Reload the file with the encoding interpreted differently.

| Menu | Shortcut | Description |
|---|---|---|
| **Default (Setting)** | — | Reload as the default encoding configured under *Set Encoding Defaults…*. |
| **ANSI** | `Ctrl+Shift+A` | Reload as the system ANSI code page. |
| **OEM** | `Ctrl+Shift+O` | Reload as the system OEM code page. |
| **GB18030** | `Ctrl+Shift+F10` | Reload as Chinese GB18030 (4-byte extended). |
| **ASCII as UTF-8** | `Ctrl+Shift+F8` | Force a file that detected as ASCII to be re-read as UTF-8 (useful for files whose only non-ASCII bytes appear far into the buffer and were missed by detection). |
| **Force UCHARDET Analysis** | — | Reload running the uchardet statistical encoding detector even on files where Notepad3 would normally trust a BOM or hint. |
| **Ignore Encoding Tags** | `Alt+F8` | Reload while ignoring embedded encoding directives (e.g. `# -*- coding: ... -*-` in Python sources). |
| **More Encodings…** | `F8` | Open the encoding picker dialog for full control. |

#### Encoding → Set Document as

Reinterpret the in-memory document as a different encoding (does not re-read from disk).

| Menu | Shortcut | Description |
|---|---|---|
| **ANSI** | — | Mark as system ANSI. |
| **Unicode** | — | Mark as UTF-16 LE (with BOM). |
| **Unicode Big Endian** | — | Mark as UTF-16 BE (with BOM). |
| **UTF-8** | `Shift+F8` | Mark as UTF-8 (without BOM). |
| **UTF-8 with Signature** | — | Mark as UTF-8 with BOM. |
| **More Encodings…** | `F9` | Open the full encoding picker. |
| **Set Encoding Defaults…** | — | Open the *Default Encoding* dialog: choose the default encoding for new files, what to do with files of unknown encoding, BOM rules, etc. |

#### Line Endings

Convert the document's line endings; the change applies to all existing lines and to subsequently typed `Enter` keys.

| Menu | Description |
|---|---|
| **Windows (CR+LF)** | `\r\n`. |
| **Mac (CR)** | `\r` (Classic Mac OS / "Mac line endings"). |
| **Unix (LF)** | `\n`. |
| **Default…** | Open dialog: choose default line ending for new files, and pick the policy for mixed-EOL files (warn / silently normalise / keep as-is). |

### Indentation / printing / properties

| Menu | Shortcut | Description |
|---|---|---|
| **Check Indentation…** | — | Analyse the document's indentation and report which mix of tabs and spaces is in use. Useful before applying *Tabify* / *Untabify*. |
| **Page Setup…** | — | Configure margins, headers/footers, and font for printing. |
| **Print…** | `Ctrl+P` | Print the document (or the selection, if any). |
| **Properties…** | — | Show a dialog with the current document's path, encoding, line endings, size, line/character/word counts, and file timestamps. |
| **Create Desktop Link** | — | Drop a `.lnk` shortcut to the current file on the desktop. |

### Navigation

| Menu | Shortcut | Description |
|---|---|---|
| **Browse…** | `Ctrl+M` | Launch the MiniPath file browser pre-targeted at the current file's directory. MiniPath is the bundled file-manager sidekick — see [`minipath/`](minipath/). |
| **Favorites → Open Favorites…** | `Alt+I` | Choose a file from the favorites list to open. |
| **Favorites → Add Current File…** | `Alt+K` | Add the current file (under a chosen display name) to the favorites list. |
| **Favorites → Manage…** | `Alt+F9` | Open the *Favorites* folder in MiniPath for direct management of the underlying `.lnk` files. |
| **Recent (History)…** | `Ctrl+Alt+H` | Pick from the MRU list. |
| **Exit** | `Alt+F4` | Close Notepad3 (prompts on unsaved changes). |

---

## Edit

Editing operations on the document's text. The **Edit** menu is split: the top of the menu holds the basic clipboard + undo block, while sub-menus group the rest by concern.

| Menu | Shortcut | Description |
|---|---|---|
| **Document Read Only** | — | Toggle the *session-only* read-only flag — typing is ignored. This is **not** the on-disk read-only attribute; see *File → Read Only File Attribute* for that. |

### Basic clipboard / undo

| Menu | Shortcut | Description |
|---|---|---|
| **Undo** | `Ctrl+Z` | Roll back the last edit. |
| **Redo** | `Ctrl+Y` | Replay the last undone edit. |
| **Cut** | `Ctrl+X` | Cut the selection — or the current line, if no selection. |
| **Copy** | `Ctrl+C` | Copy the selection — or the current line, if no selection. |
| **Copy All** | `Alt+C` | Copy the entire document to the clipboard. |
| **Copy Add** | `Ctrl+E` | **Append** the selection (or current line) to the existing clipboard contents instead of replacing them. Repeated presses accumulate. |
| **Paste** | `Ctrl+V` | Paste the clipboard at the caret. |
| **Swap** | `Ctrl+K` | Exchange clipboard contents with the current selection (selection goes to clipboard; previous clipboard text replaces the selection). |
| **Select All** | `Ctrl+A` | Select the whole document. |
| **Delete** | `Del` | Remove the selection (no clipboard write). |
| **Clear Clipboard** | — | Empty the Windows clipboard. |
| **Toggle Clipboard Monitoring** | — | "Pasteboard" mode — every clipboard change is auto-pasted into the document. Mutually exclusive with *Monitoring Log* (see *View → Display*). This is a session-only toggle and is not persisted. Can also be enabled at startup via the `/B` switch (see [`cmdln/CmdLnOptions.md`](cmdln/CmdLnOptions.md)). |

### Words

| Menu | Shortcut | Description |
|---|---|---|
| **Complete Word** | `Ctrl+Alt+Enter` | Buffer-based autocomplete — list words appearing elsewhere in the current document that start with the prefix at the caret. Independent of *Auto Complete Words* (always available on demand). |
| **Cursor Word Left** | `Ctrl+ ←` | Move caret one word left. |
| **Cursor Word Right** | `Ctrl+ →` | Move caret one word right. |
| **Delete Word Left** | `Ctrl+Back` | Delete the word before the caret. |
| **Delete Word Right** | `Ctrl+Del` | Delete the word after the caret. |

Word boundaries can be widened with *Settings → Alternate Word Separators* (`Ctrl+Alt+A`) — see that entry for the trade-off.

### Lines

Manipulation of whole lines, regardless of where the caret sits inside them.

| Menu | Shortcut | Description |
|---|---|---|
| **Move Up** | `Alt+ ↑` | Move current line (or selected block) up by one line. |
| **Move Down** | `Alt+ ↓` | Move current line (or selected block) down by one line. |
| **Cut Selection or Current Line** | `Ctrl+X` | Same as *Edit → Cut*. |
| **Cut Selection Whole Line(s)** | `Ctrl+Shift+X` | Cut the entire line(s) touched by the selection — even if the selection covers only part of them. |
| **Copy Selection or Current Line** | `Ctrl+C` | Same as *Edit → Copy*. |
| **Copy Selection Whole Line(s)** | `Ctrl+Shift+C` | Copy the entire line(s) touched by the selection. |
| **Duplicate Line/Selection** | `Ctrl+D` | Duplicate the selection, or the current line if nothing is selected. |
| **Transpose Lines** | `Alt+D` | Swap the current line with the line above. |
| **Delete Line** | `Ctrl+Shift+D` | Delete the current line. |
| **Delete Line Left** | `Ctrl+Shift+Back` | Delete from line start to caret. |
| **Delete Line Right** | `Ctrl+Shift+Del` | Delete from caret to line end. |
| **Column Wrap…** | `Ctrl+Shift+W` | Prompt for a column number and hard-wrap each line of the selection at that column. |
| **Unite Duplicate Lines** | `Ctrl+Alt+D` | Collapse runs of identical lines down to one occurrence (the first survives). |
| **Remove Duplicate Lines** | — | Like *Unite* but removes **all** copies — keeps only lines that occur exactly once. |
| **Merge Empty Lines** | `Alt+Y` | Collapse consecutive *empty* lines (length 0) into a single empty line. |
| **Merge Blank Lines** | `Ctrl+Alt+Y` | Collapse consecutive *blank* lines (empty **or** whitespace-only) into a single blank line. |
| **Remove Empty Lines** | `Alt+R` | Delete every empty line in the selection. |
| **Remove Blank Lines** | `Ctrl+Alt+B` | Delete every blank line (empty or whitespace-only). |

> **"Empty" vs "blank"**: *empty* = zero characters between line endings. *Blank* = empty **or** containing only spaces/tabs. The four merge/remove commands form a 2×2 grid (merge vs remove × empty vs blank).

### Advanced

A grab-bag of structural edits that go beyond per-line text manipulation.

#### Indent

| Menu | Description |
|---|---|
| **Indent** | Indent the selection one level (driven by *Tab Settings*). |
| **Unindent** | Unindent the selection one level. |

#### Line Comment

Line-style comments using the current lexer's comment token (`//`, `#`, `--`, etc.).

| Menu | Shortcut | Description |
|---|---|---|
| **Toggle** | `Ctrl+Q` | Toggle line comments on each selected line — if all are commented, uncomment; otherwise comment all. |
| **Add** | `Ctrl+Alt+Q` | Add a comment marker, even to already-commented lines (stack them). |
| **Remove** | `Ctrl+Alt+Shift+Q` | Strip one level of comment marker from each line. |
| **Block Edit** | `Alt+Shift+Q` | Place a multi-cursor at the comment marker position of each selected line, ready for in-place editing of the existing comments. |

#### Stream Comment

| Menu | Shortcut | Description |
|---|---|---|
| **Stream Comment** | `Ctrl+Shift+Q` | Toggle a block / stream comment around the selection using the current lexer's stream-comment delimiters (`/* */`, `<!-- -->`, etc.). |

#### Enclose Selection

Wrap the selection in matched delimiters.

| Menu | Shortcut |
|---|---|
| **Single Quotes** | `Ctrl+1` |
| **Double Quotes** | `Ctrl+2` |
| **( )** | `Ctrl+3` |
| **[ ]** | `Ctrl+4` |
| **{ }** | `Ctrl+5` |
| **Backticks** | `Ctrl+6` |
| **With…** | `Alt+Q` — Prompt for arbitrary *prefix* and *suffix* strings (e.g. enclose with `<b>` … `</b>`). |

#### Other advanced commands

| Menu | Shortcut | Description |
|---|---|---|
| **Duplicate Selection/Line** | `Ctrl+D` | Same as *Lines → Duplicate Line/Selection*. |
| **Pad With Blanks** | `Alt+B` | Pad every line of the selection with trailing spaces to the length of the longest line (turns ragged-right text into a column block). |
| **Strip First Character** | `Alt+Z` | Remove the first character from each line. |
| **Strip Last Character** | `Alt+U` | Remove the last character from each line. |
| **Strip Trailing Blanks** | `Alt+W` | Remove trailing whitespace from each line. |
| **Compress Blanks** | `Ctrl+Alt+P` | Collapse every run of spaces/tabs to a single space. |
| **Modify Lines…** | `Alt+M` | Open a dialog to add a *prefix* and/or *suffix* to each line of the selection (with optional `\n`, `\t`, line-number `$(L)` placeholders). |
| **Sort Lines…** | `Alt+O` | Open the sort dialog: ascending / descending, case-sensitive, numeric, by column N, remove duplicates, etc. |
| **Align Lines…** | `Alt+J` | Open a dialog to align the selection (left / right / centre / justify, plus column-N alignment). |
| **Split Lines** | `Ctrl+I` | Re-break wrapped paragraphs — splits at spaces so that no line exceeds the long-line column. |
| **Join Lines** | `Ctrl+J` | Join the selected lines into one, separated by single spaces. |
| **Fuse Lines** | `Ctrl+Alt+J` | Join the selected lines with **no** separator (concatenate). |
| **Preserve Paragraphs** | `Ctrl+Shift+J` | Join lines like *Join*, but keep blank lines (paragraph boundaries) intact. |
| **Jump to Selection Start** | `Ctrl+,` | Move the caret to the start of the current selection (without changing the selection). |
| **Jump to Selection End** | `Ctrl+.` | Move the caret to the end of the current selection. |

### Convert

Bulk conversions on the selection.

| Menu | Shortcut | Description |
|---|---|---|
| **Uppercase** | `Ctrl+Shift+U` | Selection → `UPPER`. |
| **Lowercase** | `Ctrl+U` | Selection → `lower`. |
| **Invert Case** | `Ctrl+Alt+U` | Flip each letter's case. |
| **Title Case** | `Ctrl+Alt+I` | Capitalise the first letter of each word. |
| **Sentence Case** | `Ctrl+Alt+O` | Capitalise the first letter after each sentence-ending punctuation. |
| **Tabify Selection** | `Ctrl+Shift+T` | Convert runs of spaces anywhere in the selection to tabs (using the configured tab width). |
| **Untabify Selection** | `Ctrl+Shift+S` | Convert tabs anywhere in the selection to spaces. |
| **Tabify Indent** | `Ctrl+Alt+T` | Like *Tabify*, but only acts on the leading indentation of each line — body whitespace is untouched. |
| **Untabify Indent** | `Ctrl+Alt+S` | Like *Untabify*, but only acts on leading indentation. |
| **Base64 Encode** | — | Replace selection with its Base64 encoding (using current encoding's bytes). |
| **Base64 Decode** | — | Replace selection with the Base64-decoded bytes, interpreted in the document's current encoding. |
| **Base64 Decode to codepage…** | — | Like *Base64 Decode*, but prompts for the target codepage so the decoded bytes can be interpreted independently. |
| **URL Encode** | `Ctrl+Shift+E` | Percent-encode the selection (e.g. ` ` → `%20`). |
| **URL Decode** | `Ctrl+Shift+R` | Reverse of *URL Encode*. |
| **File System Path to URL** | `Ctrl+F10` | `C:\path\to\file` → `file:///C:/path/to/file`. |
| **URL to File System Path** | `Alt+F10` | Reverse: `file:///C:/path/to/file` → `C:\path\to\file`. |
| **Invert Backslashes** | — | Replace every `\` with `/`. |
| **Invert Slashes** | — | Replace every `/` with `\`. |
| **Char To Hex** | `Ctrl+Alt+X` | Each character → its hex code point (e.g. `A` → `\x41`, `€` → `€`). |
| **Hex To Char** | `Ctrl+Alt+C` | Reverse of *Char To Hex* — interpret `\x..` / `\u....` escapes back into characters. |

### Insert

Insert dynamic content at the caret.

| Menu | Shortcut | Description |
|---|---|---|
| **New Line Above** | `Ctrl+Enter` | Insert an empty line above the current one and move the caret into it. |
| **HTML/XML Tag…** | `Alt+X` | Prompt for a tag name; wrap the selection in `<tag>` … `</tag>` (or insert a self-closing tag if no selection). |
| **Encoding Identifier** | `Ctrl+F8` | Insert a comment describing the current encoding (form depends on the active lexer: `# -*- coding: utf-8 -*-` for Python, `<?xml version="1.0" encoding="utf-8"?>` for XML, etc.). |
| **Time/Date (Short Form)** | `Ctrl+F5` | Insert short-form date/time per `Settings2.DateTimeFormat`. |
| **Time/Date (Long Form)** | `Ctrl+Shift+F5` | Insert long-form date/time per `Settings2.DateTimeLongFormat`. |
| **Current Timestamp** | — | Insert a timestamp at the caret. The format and regex are controlled by `[Settings2] TimeStampFormat` and `TimeStampRegEx`. |
| **Update Timestamps** | `Shift+F5` | Walk the document and rewrite every match of `TimeStampRegEx` to the current time. |

### Miscellaneous

Less-frequently-used utilities and selection commands.

| Menu | Shortcut | Description |
|---|---|---|
| **Copy GUID to Clipboard** | `Ctrl+Shift+.` | Generate a fresh GUID and copy it to the clipboard. (The IDM constant is `INSERT_GUID` for historical reasons — the action **copies**, it does not insert.) |
| **Escape C Chars** | `Ctrl+Alt+E` | C-escape the selection (`"` → `\"`, newline → `\n`, etc.). |
| **Unescape C Chars** | `Ctrl+Alt+R` | Reverse of *Escape C Chars*. |
| **Increase Number** | `Ctrl+Alt+NK+` | Add 1 to the number under the caret. |
| **Decrease Number** | `Ctrl+Alt+NK-` | Subtract 1 from the number under the caret. |
| **Find Matching Brace** | `Ctrl+B` | If the caret is at a brace `()[]{}` jump to its match; otherwise jump to the nearest enclosing opening brace. Shows a calltip if no match exists. |
| **Select To Matching Brace** | `Ctrl+Shift+B` | Select the enclosing brace pair (inclusive of both braces). Repeated presses expand outward one nesting level at a time. |
| **Select To Next** | `Ctrl+Alt+F2` | Extend the selection forward to the next match of the find pattern. |
| **Select To Previous** | `Ctrl+Alt+Shift+F2` | Extend the selection backward to the previous match. |
| **Select Word or Lines** | `Ctrl+Spc` | Select the word at the caret; press again to extend to the whole line. |
| **Multi-Select All Matches** | `Ctrl+Shift+Spc` | Place multi-carets at every occurrence of the word at the caret (true multi-selection, edits affect all). |
| **Open Containing Folder of Selection** | — | Resolve the selection (or token at the caret) to a path and reveal it in Windows Explorer. For relative paths the anchor is the current document's directory, falling back to the working directory. Greyed out when there is no selection and no word at the caret. See [`paths/FilePathHandling.md`](paths/FilePathHandling.md). |
| **Open File from Selection** | — | Same resolution as above; loads the file into Notepad3 (jumping to a parsed `:line` if present) or opens the file-open dialog rooted at the resolved directory. See [`paths/FilePathHandling.md`](paths/FilePathHandling.md). |
| **Split Undo Transaction at Line-Breaks** | — | Toggle: when on, every newline boundary becomes an undo checkpoint, so `Ctrl+Z` rolls back line-by-line instead of by typing run. |

### Bookmarks

Per-line user markers displayed in the bookmark margin.

| Menu | Shortcut | Description |
|---|---|---|
| **Toggle** | `Ctrl+F2` | Set/clear a bookmark on the current line. |
| **Goto Next** | `F2` | Jump to the next bookmarked line; falls through to change-history markers if none ahead (see *View → Change History*). |
| **Goto Previous** | `Shift+F2` | Jump to the previous bookmark; same fallback. |
| **Clear All** | `Alt+F2` | Remove every bookmark in the document. |

---

## Search

Find, replace, and goto commands. The *Find* and *Replace* dialogs are **modeless** — the editor remains usable while they're open. PCRE2 regular expressions are supported (see [`cmdln/CmdLnOptions.md`](cmdln/CmdLnOptions.md) for the syntax cheatsheet).

| Menu | Shortcut | Description |
|---|---|---|
| **Find…** | `Ctrl+F` | Open the Find dialog. |
| **Save Find Text** | `Alt+F3` | Save the current find text as the persistent default (so future runs start with it pre-filled). |
| **Find Next** | `F3` | Repeat the last find forward. |
| **Find Previous** | `Shift+F3` | Repeat the last find backward. |
| **Find Next Selected** | `Ctrl+F3` | Find next occurrence of the current selection (or word at caret). |
| **Find Previous Selected** | `Ctrl+Shift+F3` | Find previous occurrence of the selection. |
| **Replace…** | `Ctrl+H` | Open the Replace dialog. |
| **Replace Next** | `F4` | Replace the next match using the last-used find/replace pair. |
| **Goto…** | `Ctrl+G` | Open the Goto dialog (jump by line / column / offset). |
| **Search in Files…** | `Ctrl+Shift+F` | Launch grepWin pre-targeted at the current file's directory. Same as *File → Launch → Search in Files…*. |

Inside the Find/Replace dialogs an extra accelerator table applies (`Ctrl+F` switches to *Find* tab, `Ctrl+H` to *Replace*, `Ctrl+S`/`Ctrl+R` save/reset the dialog position, etc.) — see [`KeyboardShortcuts.md`](KeyboardShortcuts.md#inside-the-find--replace-dialog).

---

## View

Display, navigation, and visual-aid toggles. None of these change file contents.

### Visual aids

| Menu | Shortcut | Description |
|---|---|---|
| **Word Wrap** | `Ctrl+W` | Toggle soft word wrapping. Wrap style is configured via *Settings → Word Wrap Settings…*. |
| **Long Line Marker** | `Ctrl+Alt+L` | Toggle a vertical line at the configured column(s). Position(s) are set via *Settings → Long Line Settings…*. |
| **Indentation Guides** | `Ctrl+Shift+G` | Toggle the dotted indentation guide lines. |
| **Show Wrap Symbols** | `Ctrl+Shift+7` | Show arrow markers where soft-wrapped lines continue. |
| **Show Blanks** | `Ctrl+Shift+8` | Show whitespace as dots (spaces) and arrows (tabs). |
| **Show Line Endings** | `Ctrl+Shift+9` | Display `CR`, `LF`, `CR+LF` end-of-line glyphs. |
| **Show Non-Printable Characters** | — | Display the C0 / C1 / Unicode invisible / format / surrogate code points using their replacement glyphs. |

### Change History

Track which lines have been modified, saved, or reverted in the current session. Stored in the dedicated change-history margin and/or as inline marks on the line text.

| Menu | Description |
|---|---|
| **No Change Markers** | Disable change-history tracking. |
| **Show in Margin only** | Show the colored bar in the margin only. |
| **Show in Document only** | Show inline marks on the text only. |
| **Show All Change Markers** | Show both margin bar and inline marks. |
| **Clear Change/Undo History** | Reset both the change-history state and the undo/redo stack — useful after intentional bulk reformatting you do **not** want to roll back. |

### Hotspots & highlights

| Menu | Shortcut | Description |
|---|---|---|
| **Hyperlink Hotspots** | `Ctrl+Alt+W` | Toggle URL detection — when on, `Ctrl`-click opens the URL in the browser, `Alt`-click loads it as a file. |
| **Color Definition Hotspots → OFF** | — | Disable colour-swatch detection. |
| **Color Definition Hotspots → RGB+A** | — | Detect colour literals in `#RRGGBBAA` order. |
| **Color Definition Hotspots → A+RGB** | — | Detect colour literals in `#AARRGGBB` order. |
| **Color Definition Hotspots → BGRA** | — | Detect colour literals in `#BBGGRRAA` order. |
| **Highlight Unicode-Points** | — | Highlight surrogate pairs / non-BMP / unusual Unicode characters in the document text. |
| **Visual Brace Matching** | `Ctrl+Shift+V` | Toggle automatic highlighting of the brace matching the one under the caret. |

When *Color Definition Hotspots* is on, `Ctrl`-clicking a colour swatch in the editor opens the colour picker.

### Highlight Current Line

| Menu | Description |
|---|---|
| **No Highlight** | Do not highlight the current line. |
| **Background Color** | Tint the current line's background. |
| **Outline Frame** | Draw a thin frame around the current line. |

Cycled in code by `Ctrl+Shift+I`.

### Mark Occurrences

Automatically highlights every occurrence of the word at the caret (or the current selection). Markers are also what feeds *Focused View* (see *View → Display → Focused View*).

| Menu | Shortcut | Description |
|---|---|---|
| **Active** | `Alt+A` | Master toggle. When off, no occurrence marking happens. |
| **Use Bookmarks too** | — | Also drop bookmark icons in the margin for each occurrence — handy for jumping with `F2` / `Shift+F2`. |
| **Match Visible Only** | — | Only mark occurrences in the currently visible viewport. **Disables Focused View** (which needs full-document marks). |
| **Match Case Sensitive** | — | Match exact case only. |
| **Match Whole Word Only → OFF** | — | Match substrings anywhere. |
| **Match Whole Word Only → Match Selected Word** | — | Whole-word match using the **selection** as the search term. |
| **Match Whole Word Only → Match Current Word** | — | Whole-word match using the word **under the caret**. |

### Margins

| Menu | Shortcut | Description |
|---|---|---|
| **Line Numbers** | `Ctrl+Shift+N` | Show the line-number margin. |
| **Bookmark Margin** | `Ctrl+Shift+M` | Show the bookmark margin. |
| **Change History Margin** | — | Show the change-history margin only (independent of which markers are enabled in *Change History*). |

### Folding

Code folding visibility and navigation. Requires a lexer that defines folds.

| Menu | Shortcut | Description |
|---|---|---|
| **Code Folding** | — | Toggle visibility of the fold margin. |
| **Toggle Folds** | `Ctrl+Alt+F` | Collapse all folds if any are expanded; otherwise expand all. |
| **Jump Next Fold** | `Alt+ +` | Move caret to the next fold point. |
| **Jump Prev Fold** | `Alt+ –` | Move caret to the previous fold point. |
| **Collapse Fold** | `Alt+ ←` | Collapse the fold containing the caret. |
| **Expand Fold** | `Alt+ →` | Expand the fold containing the caret. |

### Display

| Menu | Shortcut | Description |
|---|---|---|
| **Focused View** | `Ctrl+Alt+V` | Hide all lines that don't contain an occurrence marker — see [`focusedview/FocusedView.md`](focusedview/FocusedView.md). Requires *Mark Occurrences → Active* on and *Match Visible Only* off. |
| **View Mode → Fold** | — | Focused View hides non-matches by folding. |
| **View Mode → Bookmark** | — | Focused View flags matches with bookmarks (no hiding). |
| **View Mode → Highlight** | — | Focused View highlights matches with a tinted background (no hiding). |
| **View Mode → Bookmark & Fold** | — | Combine bookmarks and folding. |
| **View Mode → Highlight & Fold** | — | Combine highlight and folding. |
| **Monitoring Log** | — | Tail mode — auto-reload the file when it changes on disk, follow scrolling at end. Mutually exclusive with *Toggle Clipboard Monitoring* (only one can be on at a time). |
| **Scroll Past End of File** | — | Allow scrolling beyond the last line of the document. |
| **Show Hyperlink Tooltip** | — | When *Hyperlink Hotspots* is on, also show a tooltip on hover with the resolved URL. |

### Panels

Show/hide the optional UI chrome.

| Menu | Description |
|---|---|
| **Show Titlebar** | Toggle the window title bar. |
| **Show Menubar** | Toggle the main menu. |
| **Show Toolbar** | Toggle the toolbar. |
| **Show Statusbar** | Toggle the status bar. |
| **Load Toolbar Theme…** | Browse to a `.bmp` toolbar-image set to use a custom theme. |
| **Customize Toolbar…** | Open the standard Windows toolbar customization dialog. |
| **Toggle Toolbars** | Cycle through the available built-in toolbar themes. |
| **DPI Scale Toolbar** | Toggle DPI-aware scaling of toolbar icons (off on some displays produces smaller, crisper icons). |

When the menu bar is hidden, press `Alt` to show it temporarily, or right-click anywhere on the title / toolbar / status bar to access the *Panels* context menu.

### Zoom

| Menu | Shortcut | Description |
|---|---|---|
| **Zoom In** | `Ctrl+ +` | Zoom in. |
| **Zoom Out** | `Ctrl+ –` | Zoom out. |
| **Reset Zoom** | `Ctrl+0` | Reset to 100 %. |

`Ctrl`-mousewheel also zooms.

### Position

Window position management — useful for power-user multi-monitor setups.

| Menu | Shortcut | Description |
|---|---|---|
| **Copy Position Args** | `Ctrl+Shift+K` | Copy the current window position to the clipboard as a `/pos x y w h dpi max` command-line switch — paste it into a shortcut to make that position the default for that shortcut. |
| **Snap to Default Position** | `Ctrl+Shift+P` | Restore the window to the user-saved default position on the current monitor. |
| **Save as Default Position** | — | Save the current window rect as the default. |
| **Clear saved Default Position** | — | Forget the saved default. |
| **Snap to Initial Position** | `Ctrl+F11` | Restore to the built-in initial-default position. |
| **Toggle Full Area Position** | `F11` | Toggle full-monitor (fills the work area). Distinct from a borderless fullscreen — chrome stays. |
| **Sticky Window Position** | — | Prevent the window position from being saved at exit — keeps the persisted default frozen even when you move the window during a session. |

---

## Appearance

Syntax-scheme and font configuration. See also the *Themes* section under `Build/Themes/` for shipped colour schemes.

| Menu | Shortcut | Description |
|---|---|---|
| **Customize Schemes…** | `Ctrl+F12` | Open the full styling editor: per-lexer per-token foreground / background / font / attribute, plus theme management. Persisted into `Notepad3.ini` (or the active theme `.ini`). |
| **Select Syntax Scheme…** | `F12` | Open the lexer picker — change which scheme the current file uses (e.g. force a `.txt` to be highlighted as JSON). Useful for files whose extension does not match their actual format. |
| **2nd Common Base** | `Shift+F12` | Swap between the primary and secondary *Common Base* schemes. Useful for keeping a dark and a light scheme one keypress apart. |
| **Global Default Font…** | `Alt+F12` | Open the font picker for the global *Default Text* style — affects every lexer that does not override it. |
| **Current Scheme's Default Font…** | `Ctrl+Alt+F12` | Open the font picker scoped to the **active lexer only**. |
| **Windows Dark Mode** | — | Toggle Windows 10/11 dark-mode chrome (title bar, scroll bars, menus). Editor colours come from the active scheme — pair with a dark theme for full coverage. |

---

## Settings

Editor behaviour, persistence, and platform integration. Most items are toggles persisted in `Notepad3.ini` (see [`config/Configuration.md`](config/Configuration.md) for every key's identifier and default).

### Typing & autocomplete

| Menu | Shortcut | Description |
|---|---|---|
| **Insert Tabs as Spaces** | — | When typing `Tab`, insert `N` spaces instead of `\t`. Width set via *Tab Settings…*. |
| **Tab Settings…** | `Ctrl+T` | Open dialog: tab width, indent width, *Tabs as Spaces*, back-tab behaviour. |
| **Word Wrap Settings…** | — | Open dialog: wrap mode (word / character / whitespace), continuation indent. |
| **Long Line Settings…** | — | Open dialog: long-line column(s) and marker style (line vs background). |
| **Auto Indent Text** | — | New lines inherit the indentation of the line above. |
| **Auto Close Quotes** | — | After typing `'` or `"`, automatically insert the matching closer. |
| **Auto Close Brackets** | — | After typing `(`, `[`, `{`, automatically insert the closer. |
| **Auto Close HTML/XML** | `Ctrl+Shift+H` | When typing `</` after an open tag, auto-complete the closing tag name. |
| **Auto Complete Words** | — | Show an autocomplete popup while typing using words already in the buffer. |
| **Auto Complete Lexer-Key-Words** | — | When autocomplete is active, also include the current lexer's keyword list (language built-ins). |
| **Alternate Word Separators** | `Ctrl+Alt+A` | Toggle "accelerated word navigation" — merges most punctuation into the word-character set, so `Ctrl+←/→` and `Ctrl+Backspace/Del` skip whole identifiers and operators. Trade-off: word-boundary text ops (find whole-word, double-click selection) widen too. Fine-tune via `[Settings2] ExtendedWhiteSpaceChars`. |

### File management

| Menu | Shortcut | Description |
|---|---|---|
| **Only One Instance per File** | — | If the requested file is already open in another Notepad3 instance, activate that one instead of opening a duplicate. |
| **File Change Notification…** | `Alt+F5` | Open dialog: configure what happens when the file on disk changes externally (ignore / prompt / auto-reload). Also controls *Monitoring Log* mode. |
| **Mute Message Beeps** | — | Silence `MessageBeep()` calls used for error / warning / info dialogs. |
| **Resolve to UNC-Paths** | — | When opening a file via a mapped drive letter, expand the path to its underlying UNC form (`\\server\share\…`). |

### Behaviour toggles

| Menu | Description |
|---|---|
| **Esc Key Function → None** | `Esc` does nothing at the top level (still closes popups / autocomplete). |
| **Esc Key Function → Minimize Notepad3** | `Esc` minimises the main window. |
| **Esc Key Function → Exit Notepad3** | `Esc` closes Notepad3 (prompts on unsaved changes). |
| **Save Before Running Tools** | Auto-save the document before *Execute Document*, *Open with…*, *Command…*, *Web Templates*. |
| **Calculate Tiny-Expressions** | When something arithmetic is selected (e.g. `1+2*3`), evaluate it and show the result in the status bar. See [`tinyexprcpp/`](tinyexprcpp/) for the supported function set. |
| **Enable Multiple Selection** | Allow Scintilla's multi-cursor mode (`Ctrl`-click adds an additional caret, `Alt`-drag creates rectangular multi-selection). |

### Default Directory

| Menu | Description |
|---|---|
| **Set to Current File's Directory** | Bind the *Open* / *Save As* default to the current file's directory. |
| **Reset to Default** | Restore the standard "Documents" default. |

### Remember

What state to persist between sessions. The menu items themselves are checkboxes (the underlying flag names contain `NO` because they negate the persistence — but each menu line works as a normal "remember this" toggle).

| Menu | Description |
|---|---|
| **Remember Recent Files** | Persist the MRU file list. |
| **Preserve Caret Position** | Restore the last caret position when re-opening a file. |
| **Remember Search Pattern** | Persist find/replace history across sessions. |
| **Autoload Recent File** | On startup with no file argument, open the most recently used file. |

### Window

| Menu | Shortcut | Description |
|---|---|---|
| **Window Title Display → Filename Only** | — | Title shows just `filename.ext`. |
| **Window Title Display → Filename and Directory** | — | Title shows `filename.ext  (path)`. |
| **Window Title Display → Full Pathname** | — | Title shows `C:\full\path\filename.ext`. |
| **Window Title Display → Text Excerpt** | `Ctrl+9` | Title shows the first non-empty line of the document (handy for untitled scratch documents). |
| **Reuse Window** | `Ctrl+Shift+L` | When a second Notepad3 launch happens, route the file into the **current** window instead of opening a new instance. Distinct from *Single File Instance* — *Reuse Window* always routes everything; *Single File Instance* only routes a re-open of an already-open file. |
| **Always On Top** | `Alt+T` | Keep Notepad3 above all other windows. |
| **Minimize To Tray** | — | Minimise to the system tray icon instead of the taskbar. |
| **Transparent Mode** | `Alt+G` | Apply window opacity (controlled by `Settings2.OpacityLevel`). |

### Technology

#### Rendering

The text engine used to draw the editor. **Direct2D modes (DirectWrite + Retain) are the default and the recommended choices** — they support font ligatures, smooth scaling, and correct DPI handling. GDI is the legacy fallback for systems where Direct2D misbehaves.

| Menu | Description |
|---|---|
| **DirectWrite (Direct2D)** | Direct2D + DirectWrite, default mode. Hardware-accelerated. |
| **DirectWrite Retain** | DirectWrite + retained back buffer. Default on ARM64 (avoids flicker on Qualcomm Adreno + Win11 DWM). |
| **DirectWrite GDI DC** | DirectWrite rendered into a GDI device context — interop fallback. |
| **GDI (no ligatures)** | Plain GDI rendering. No font ligatures. Use only when D2D modes cause artifacts on a specific machine. |

#### RTL Layout (GDI) / Bidirectional

| Menu | Description |
|---|---|
| **RTL Layout → Edit Area** | Right-to-left layout for the editor pane (RTL languages — Arabic, Hebrew). |
| **RTL Layout → Other Dialogs** | RTL layout for the surrounding dialogs and chrome. |
| **Bidirectional → None** | Disable bidirectional text shaping. |
| **Bidirectional → Left to Right** | Force LTR base direction with bidi shaping. |
| **Bidirectional → Right to Left** | Force RTL base direction with bidi shaping. |

### Advanced Configuration

| Menu | Shortcut | Description |
|---|---|---|
| **Save Settings On Exit** | — | When on, settings are written to `Notepad3.ini` on each exit. When off, settings are only saved when *Save Settings Now* is invoked. |
| **Save Settings Now** | `F7` | Write the current settings to `Notepad3.ini` immediately. |
| **Open Settings File** | `Ctrl+F7` | Open `Notepad3.ini` in the editor for manual editing. |

---

## Help

| Menu | Shortcut | Description |
|---|---|---|
| **Online Documentation** | `F1` | Open the project documentation site in the default browser. |
| **Launch Administration Tool…** | — | Open the Administration helper dialog — run programs from inside Notepad3 with elevated rights. |
| **Check Website for Update** | — | Open the project website's downloads page in the browser. |
| **Command Line Help…** | — | Show a dialog listing all supported command-line switches. See [`cmdln/CmdLnOptions.md`](cmdln/CmdLnOptions.md) for the full reference. |
| **About…** | `Shift+F1` | Show the *About* dialog: version, build info, credits, third-party components. |

---

## Context (right-click) menus

Notepad3 ships several context menus, each shown by right-clicking on a different region of the UI. They are smart subsets of the main menu, with state-aware enabling.

### Edit area (right-click in the document)

A condensed editing menu reachable via right-click on the text:

- **Open Hyperlink** — open the URL under the click position in the browser. Greyed out if the click did not land on a recognised URL.
- **Copy Hyperlink URL** — copy that URL to the clipboard.
- **Undo / Redo / Cut / Copy / Paste / Delete / Select All** — standard editing.
- **Convert Case → Uppercase / Lowercase / Invert Case / Title Case / Sentence Case** — same conversions as *Edit → Convert*, but apply to the selection **or** to the word at the right-click position when nothing is selected.
- **Toggle Current Fold** — collapse or expand the fold containing the right-click line. Equivalent to clicking the fold margin's `+` / `-` glyph.
- **Open Web Action 1 / 2** — same as *File → Launch → Web Template 1 / 2*. Menu labels are customised via `Settings2.WebTmpl1MenuName` / `WebTmpl2MenuName`.
- **Open File Explorer** — open the current document's directory in Windows Explorer with the file selected.

### Bar area (right-click on title / toolbar / status bar / non-client area)

Manage UI chrome — same items as *View → Panels*:

- **Show Titlebar / Menubar / Toolbar / Statusbar** — show or hide each panel.
- **Load Toolbar Theme… / Customize Toolbar… / Toggle Toolbars / DPI Scale Toolbar** — toolbar appearance controls.

### Margin (right-click on the line-number / bookmark / change-history / fold margin)

- **Clear Marker** — remove the marker (bookmark / change-history / fold) on the clicked line. Greyed out if no marker is present.
- **Cut Lines of Marker-Type** — cut every line that carries the same marker type as the click target.
- **Copy Lines of Marker-Type** — copy every line of that marker type.
- **Delete Lines of Marker-Type** — delete every line of that marker type.
- **Folding** — toggle the fold margin (mirrors *View → Margins*).
- **Line Numbers** — toggle the line-number margin.
- **Bookmark Margin** — toggle the bookmark margin.
- **Change History Margin** — toggle the change-history margin.

### Tray icon (right-click on the Notepad3 tray icon when *Minimize To Tray* is on)

- **Open Notepad3** — restore the main window. Also bound to a double-click on the tray icon.
- **Exit Notepad3** — close Notepad3 (prompts on unsaved changes).

---

## See also

- [`KeyboardShortcuts.md`](KeyboardShortcuts.md) — every shortcut in tabular form, plus cheatsheets for caret/view navigation and function keys.
- [`cmdln/CmdLnOptions.md`](cmdln/CmdLnOptions.md) — every command-line switch (regex syntax cheatsheet included).
- [`config/Configuration.md`](config/Configuration.md) — every `Notepad3.ini` key referenced in this document.
- [`focusedview/FocusedView.md`](focusedview/FocusedView.md) — Focused View modes and prerequisites.
- [`encryption/Encryption.md`](encryption/Encryption.md) — the `.cryptedit` workflow used by *Set Encryption Passphrase…*.
- [`faq/FAQ.md`](faq/FAQ.md) — *Migrating from Notepad2* covers every command whose behaviour or binding was redefined in Notepad3.
- [`minipath/`](minipath/) — MiniPath's own menu and shortcut reference.
