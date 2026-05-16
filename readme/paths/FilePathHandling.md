# File-path handling in Notepad3

Everywhere Notepad3 turns a piece of text into a *file* — a clickable link in
the editor, a path you typed into a dialog, a reference you selected from a
build log — it goes through the same few rules. This page collects them in
one place.

> Related references
> - **Menu entries** for the commands below: [`../MenuEntriesAndCmds.md`](../MenuEntriesAndCmds.md).
> - **INI keys** that influence path handling: [`../config/Configuration.md`](../config/Configuration.md).
> - **Command-line switches** for opening files: [`../cmdln/CmdLnOptions.md`](../cmdln/CmdLnOptions.md).
> - **MiniPath** (the bundled file browser): [`../minipath/`](../minipath/).

## Quick reference

| Want to … | Do this |
|---|---|
| Open a `http://…` or `https://…` URL from the document | Ctrl+Click the highlighted link → opens in your default browser. |
| Open a `file:///…` URL in Notepad3 | Alt+Click the highlighted link → loads the target into the editor. |
| Open a bare path like `C:\foo\bar.c:42` in Notepad3 | Alt+Click — bare paths are highlighted only when they end in `:42`, `:42:7`, or `(42)`. |
| Reveal the file under the caret in Explorer | Place the caret on the path and choose **Edit → Miscellaneous → Open Containing Folder of Selection**, or right-click → same. |
| Open a file referenced by selection / token at caret | **Edit → Miscellaneous → Open File from Selection** (right-click also). |
| Reveal the *current* document in Explorer | **File → Open File Explorer**. |
| Copy the current path | **File → Path to Clipboard → Copy Filename / Directory / Full-Path**. |

---

## 1. Clickable hyperlinks in your document

When **View → Hyperlink Hotspots** is enabled (default), Notepad3 detects
links and paths in your text and marks them with an underline. Click
modifiers control what happens:

| Gesture | Action |
|---|---|
| **Ctrl+Click** | Open in the associated external app (browser for URLs, default opener for files). |
| **Ctrl+Shift+Click** | Open in the associated app *in the background* (Notepad3 stays in front). |
| **Alt+Click** | Open in Notepad3 itself — works for `file:///…` URLs and bare file paths. |
| **Alt+Shift+Click** | Open in a *new* Notepad3 instance. |
| **Middle-mouse click** | Same as Ctrl+Click. |
| **Right-click on a hotspot** | Context menu with explicit "Open Hyperlink" / "Copy Hyperlink URL" entries. |
| **Hover** (with `ShowHypLnkToolTip`) | Tooltip showing the link target. |

### URL forms Notepad3 recognises

Detected as a clickable URL with a scheme:

| Form | Example |
|---|---|
| HTTP(S) | `https://example.com/foo` |
| FTP | `ftp://example.com/foo` |
| `www.…` (no scheme) | `www.example.com` |
| `ftp.…` (no scheme) | `ftp.example.com` |
| `mailto:` | `mailto:nobody@example.com` |
| `file://` | `file://server/share/file.txt` |
| `file:///` | `file:///D:/projects/foo/readme.md` |

The matcher is a single PCRE2 regular expression — it stops at common
sentence punctuation (`. , ! ? :`) so a URL followed by punctuation in prose
doesn't absorb the period.

### File URLs (`file://` and `file:///`)

After the scheme is stripped, the rest is treated as a filesystem path.
**Relative `file:///` URLs** are resolved against:

1. The directory of the *current document*, if it has been saved.
2. Otherwise, the working directory (the directory Notepad3 was launched
   from, or set via `/y` on the command line).

It is *never* resolved against Notepad3's install directory — that was a bug
in older versions and is now fixed.

A trailing `#fragment` (e.g. `file:///foo.md#section-2`) is **stripped before
the file is opened**. Notepad3 doesn't resolve Markdown anchor names; the
file is opened cleanly without trying to interpret `#section-2`.

### Bare file paths (with mandatory line spec)

Plain filesystem paths are clickable **when they're followed by a line-number
indicator** — that's a strong signal the text is a build-error reference,
log entry, or stack-trace line rather than prose. Three prefix shapes are
recognised:

| Form | Example |
|---|---|
| Drive letter + separator | `C:\foo\bar.c:42` · `c:/foo/bar.c(42)` |
| UNC | `\\server\share\foo\bar.c:42` |
| Relative | `.\src\Helpers.c:42` · `../include/foo.h:42:7` |

A bare path without a line spec — for example `C:\Windows` typed in prose —
is **not** highlighted. That's intentional, to keep false positives down.
If you need to act on such a path, select it and use **Open Containing
Folder of Selection** or **Open File from Selection** (see §2).

### Line / column suffix patterns

Notepad3 recognises four trailing patterns and jumps to the parsed line
when opening:

| Pattern | Example | Meaning |
|---|---|---|
| `:N` | `Helpers.c:42` | Line 42 |
| `:N:M` | `Helpers.c:42:7` | Line 42, column 7 (column currently ignored) |
| `(N)` | `Helpers.c(42)` | Line 42 |
| `,N` | `Helpers.c,42` | Line 42 |

The parser is digit-only — `Helpers.c:42+1` is **not** interpreted as
"line 43"; it's treated as a non-match and the path is opened without a
jump.

A colon that follows a drive letter (`C:`) is never treated as a line
separator.

---

## 2. Selection-based commands

For paths that *aren't* highlighted as hotspots — or when you just want to
act on the path under the caret without targeting an indicator precisely —
use the two **Edit → Miscellaneous** entries (also in the editor right-click
menu when there's a selection or a token at the click point):

### Open Containing Folder of Selection

Resolves the selected text (or token at caret) to a filesystem path and
opens Windows Explorer with that file highlighted. If the target is a
folder, opens the folder itself.

### Open File from Selection

Resolves the same way, then loads the file into Notepad3 (jumps to the
parsed line if a line spec was present). If the target is a folder, opens
the file-open dialog rooted in it.

### How the token is extracted

If there's a non-empty selection:
- Use the selected text.
- Truncate at the first newline or tab character.

If the selection is empty:
- Expand from the caret outward on the same line.
- Stop at any of: whitespace, single quote, backtick, double quote, `<`,
  `>`, `|`, `*`, `,`, `;`.
- **Markdown link special-case**: if the caret sits inside `(…)` parens
  immediately preceded by `]`, the inner contents of the parens are used
  instead — so caret inside the URL of `[label](D:\projects\foo\readme.md)`
  picks up `D:\projects\foo\readme.md`, not the surrounding text.

Surrounding whitespace, `"`, `'`, `` ` ``, `<`, `>` are stripped from the
result.

Any trailing line/column suffix (`:N`, `:N:M`, `(N)`, `,N`) is split off
before path resolution. For *Open File from Selection*, the parsed line
number controls the caret jump after the file loads.

### Menu state

Both commands are greyed out when there's no selection *and* no word at the
caret / click position. They have no default keyboard shortcut; users can
bind one via custom keybindings if desired.

---

## 3. Path resolution rules

The same rules apply to file-URL clicks, bare-path clicks, and both
selection-based commands.

### Anchor priority for relative paths

A *relative* path (one without a drive letter, UNC `\\`, or leading slash)
is resolved against:

1. The directory of the **current document** (`Paths.CurrentFile`), if it
   has been saved.
2. Otherwise, the **working directory** (`Paths.WorkingDirectory`).

Notepad3's *install directory* (`Paths.ModuleDirectory`) is **never** used
as an anchor for in-document references.

### Environment-variable expansion

Tokens containing `%VAR%` are expanded against the current process
environment before resolution. So `%WINDIR%\notepad.exe` and
`%USERPROFILE%\Documents\foo.txt` resolve correctly.

### `file://` → Windows path conversion

`file://`-prefixed URLs go through the Windows shell's `PathCreateFromUrlW`
to convert URL-encoded characters (`%20` → space, etc.) into a real
filesystem path. If `PathCreateFromUrlW` fails on a malformed URL, the
prefix is stripped manually and the resolver tries the rest as a best
effort.

### Long paths (`\\?\` prefix)

When **Open Containing Folder of Selection** hands a path to the Windows
shell, the long-path prefix `\\?\` is stripped first so Explorer can accept
the path:

- `\\?\C:\Windows` → `C:\Windows`
- `\\?\UNC\server\share\file.txt` → `\\server\share\file.txt`

Internally Notepad3 uses long-path-aware Win32 wrappers throughout. If
Windows' long-path support (`LongPathsEnabled` group policy) is off and a
path is ≥ 260 characters, Notepad3 applies the `\\?\` prefix transparently
where necessary.

---

## 4. Path-related menu commands

### File menu

| Command | What it does |
|---|---|
| **Open…** (Ctrl+O) | Standard file-open dialog. |
| **Open with…** (Alt+L) | Hand the current file to an external editor — choose via `OpenWithDir` setting. |
| **Recent Files** (sub-menu) | Most-recently-used list. Paths are stored relative to Notepad3.exe by default — flip via the `Notepad3.ini` redirect mechanism. |
| **Save** / **Save As…** / **Save Copy…** | Standard write paths; respects `AtomicFileSave` (see §5). |
| **Open File Explorer** | Reveal the **current document** in Windows Explorer (or, if untitled, the working directory). Distinct from **Open Containing Folder of Selection**, which operates on text *inside* the document. |
| **Path to Clipboard → Copy Filename only / Copy Directory-Path only / Copy Full-Path** | Self-explanatory; for filling into other tools. |
| **Add to Favorites** / **Favorites…** | Pin a path to a quick-pick list, stored under `FavoritesDir`. |
| **Launch Default Application** (Ctrl+L) | ShellExecute the current document with its registered "open" verb. |
| **Run…** | ShellExecute an arbitrary command line, prefilled with the document's full path. |

### Edit menu

| Command | What it does |
|---|---|
| **Edit → Miscellaneous → Open Containing Folder of Selection** | See §2. |
| **Edit → Miscellaneous → Open File from Selection** | See §2. |
| **Edit → Insert → Filename / Directory / Path** | Insert the current document's name / parent dir / full path into the buffer at the caret. |

---

## 5. Background behavior

These mechanisms aren't user-triggered but are worth knowing about because
they affect what you'll see on disk.

### File watching

Notepad3 asks Windows to notify it of changes in the **parent directory**
of the open file. When the current file changes externally, the **File
Change Notification** dialog offers to reload (or you can configure silent
reload). The watch is released before *Save* and re-armed afterwards so
Notepad3 doesn't observe its own writes.

### Atomic save

With `AtomicFileSave=true` (the default), *Save* writes to a temporary file
alongside the target and then atomically replaces the original via
`ReplaceFileW`. ACLs, alternate data streams, and attributes are preserved
on failure — nothing is lost if the write is interrupted. See
[`../config/Configuration.md#atomicfilesave-true`](../config/Configuration.md#atomicfilesavetrue)
to disable on very large files where in-place writing is faster.

### MRU (Most-Recently-Used) list

The Recent Files list in the File menu is persisted in `Notepad3.ini` under
`[Recent Files]`. Disable persistence with the `NoSaveRecent` setting.
Entries can be removed individually from the MRU dialog.

### Portable & redirected INI

By default `Notepad3.ini` lives next to `Notepad3.exe` (portable mode). The
file `Notepad3.ini` itself can specify a `Notepad3.ini=<path>` redirect
under `[Notepad3]` (up to two levels) that points at a per-user location —
useful for shared installs. The redirected file is auto-created on first
write.

---

## 6. Configuration keys that affect path handling

All in `Notepad3.ini` under `[Settings2]` unless noted — see
[`../config/Configuration.md`](../config/Configuration.md) for full
descriptions.

| Key | Effect |
|---|---|
| `AtomicFileSave` | Two-phase Save (temp file + ReplaceFileW). |
| `ResolveUNCPaths` | When opening a UNC path that's actually a mapped drive's share, prefer the drive-letter form for display. |
| `DefaultDirectory` | Initial directory of the **Open…** dialog. |
| `OpenWithDir` | Directory used by **Open with…**. |
| `FavoritesDir` | Where the Favorites list lives. |
| `GrepWinPath` | Override the grepWin executable lookup. |
| `HyperlinkShellExURLWithApp` | Open URLs with a specific external app instead of the OS default. |
| `HyperlinkShellExURLCmdLnArgs` | Argument template for the above. |
| `HyperlinkFileProtocolVerb` | Override the verb used to ShellExecute `file:` URLs. |
| `ShowHypLnkToolTip` | Hover tooltip showing the link target. |
| `MonitoringLog` | `FileWatching.MonitoringLog`; tails the file by default. |

---

## 7. Troubleshooting

**Q. My path has spaces and the resolver only picked up part of it.**
Wrap it in double quotes or single quotes in the source text, or select the
full path manually before invoking the command.

**Q. `..\..\foo.c:42` opened the wrong file.**
Save your document first — relative paths anchor to the document's
directory only after it has been saved. Until then, they anchor to the
working directory.

**Q. A path is highlighted but Alt+Click does nothing.**
Notepad3 silently does nothing when the resolved path doesn't exist on
disk. Check the path with **Open Containing Folder of Selection** — it will
beep if the resolution fails.

**Q. `C:\Windows` isn't highlighted as a hotspot.**
By design — bare paths are only hotspotted when they end in a line spec
(`:42`, `:42:7`, or `(42)`). For paths without a line number, use the
selection-based commands instead.

**Q. The line jump for `foo.c:42:7` ignores `7`.**
The column is parsed and accepted but not currently acted on — only the
line is set. Tracked for a future enhancement.

**Q. `file:///D:/docs/readme.md#section-2` doesn't jump to the anchor.**
Heading-anchor resolution is a Markdown-specific convention and isn't
implemented — the fragment is stripped and the file opens at the top.
