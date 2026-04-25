# Notepad3 Command-Line Options

Complete reference for every option, switch, and positional argument accepted by `Notepad3.exe`. The in-app help dialog (`Notepad3 /?`) is the abbreviated form of this document; the parser source of truth is `ParseCommandLine()` / `ParseCmdLnOption()` in `src/Notepad3.c`.

> Notepad3 inherits its CLI surface from [Notepad2](https://www.flos-freeware.ch/notepad2.html) and [Notepad2-mod](https://xhmikosr.github.io/notepad2-mod/). Most switches behave identically to those predecessors; Notepad3-specific additions and behavioral differences are flagged with **(N3)**.

---

## Synopsis

```
Notepad3.exe [/?]
             [<encoding>] [<line-ending>]
             [/e <enc>] [/g <line>[,<col>]]
             [/m[modifiers] <text>]
             [/l|/l0|/l1] [/q[s]]
             [/s <ext>|/d|/h|/x]
             [/c] [/b]
             [/n[s]|/r[s|p]]
             [/p <pos>|/p0|/ps|/pd|/pf|/pl|/pt|/pr|/pb|/pm]
             [/t <title>]
             [/i] [/o|/o0]
             [/f <ini>|/f0]
             [/u] [/v|/vd]
             [/y] [/z <arg>]
             [appid=<id>] [sysmru={0|1}]
             [+|-]
             [[drive:][path]filename ...]
```

## Argument syntax

- **Switch prefix.** Both `/` and `-` are accepted (`/n` and `-n` are equivalent). The leading character is stripped before parsing.
- **Case.** Switch letters are **case-insensitive** (`/B`, `/b`, `-B`, `-b` all parse the same). Encoding/EOL keywords (`/utf8`, `/crlf`, …) are likewise case-insensitive.
- **Argument separators.** A switch that takes an argument (`/e`, `/g`, `/m`, `/s`, `/t`, `/f`, `/p`) consumes the **next** whitespace-separated token. Quote the argument if it contains spaces.
- **End of options.** The first token that does not look like a switch is treated as a file path; subsequent tokens are also treated as file paths. Switches placed after a file path are not parsed.
- **Multi-file mode.** By default Notepad3 accepts a **single** file argument (Windows-Notepad-compatible behavior — quoted spaces are part of the path). Override per invocation with `+` (multi-file, quoted spaces allowed) or `-` (single-file, no quoted spaces). The persistent default is `Settings2.MultiFileArg`. The `/z` switch implies `-`.

---

## Help

### `/?`
Displays the built-in command-line help dialog and exits without opening a document. The dialog content is the resource string `IDS_MUI_CMDLINEHELP` in `language/np3_<locale>/strings_<locale>.rc` — this Markdown file is the long form of that string.

---

## File arguments and multi-file mode

### `<filename>` (positional)
One or more files to open. Paths may be absolute, relative (resolved against the current working directory, or against `%PATH%` if `/y` is set), or shell links (`.lnk`). Must be the **last** arguments on the line.

### `+`
Treat all subsequent positional arguments as a **list** of file paths, with quoted spaces inside paths preserved. Use this when opening several files at once:

```
Notepad3.exe + "C:\My Docs\a.txt" "C:\My Docs\b.txt"
```

### `-`
Force **single-file** mode for this invocation, even if `Settings2.MultiFileArg=1` is persisted. Quoted spaces are *not* preserved as part of the path; the parser stops collecting after the first token. Equivalent to `/z` for the file-arg side.

### `/y`
If a positional filename is **relative** and not found in the current directory, search the directories in `%PATH%`. Useful when launching Notepad3 from a shell without changing directory.

### `/z <arg>`
Skip the next argument verbatim. Provided for **registry-based Notepad replacement** scenarios where Windows passes a leading flag (such as `-p`) that Notepad3 must consume but ignore. Implies single-file argument mode (`MultiFileArg = 1`) and marks the launch as a Notepad-replacement invocation.

---

## Encoding

The desired encoding may be given as a **positional keyword** (no argument needed) or via `/e <encoding>` for arbitrary code-page names.

### Positional keywords

| Keyword | Aliases | Meaning |
|---|---|---|
| `/ansi` | `/a`, `/mbcs` | System ANSI default code page |
| `/unicode` | `/w` | UTF-16 LE with BOM |
| `/unicodebe` | `/unicode-be` | UTF-16 BE with BOM |
| `/utf8` | `/utf-8` | UTF-8 (no BOM) |
| `/utf8sig` | `/utf-8sig`, `/utf8signature`, `/utf-8signature`, `/utf8-signature`, `/utf-8-signature` | UTF-8 with BOM (a.k.a. UTF-8 Signature) |

### `/e <encoding>`
Arbitrary encoding by name. Resolved via Notepad3's encoding database (uchardet-backed). Accepts code page names such as `cp1252`, `iso-8859-1`, `cp437`, `windows-1251`, `gb18030`, `shift-jis`, etc. Unknown names are ignored.

```
Notepad3.exe /e cp1252 d:\legacy\readme.txt
```

> The encoding switch only affects how the file is **interpreted on load**. It does not transcode the file. To change the saved encoding, use **File → Encoding → Recode** after opening, or pair the switch with a save action.

---

## Line-ending mode

### `/crlf` / `/cr+lf`
Windows line endings (CR LF).

### `/cr`
Classic Mac line endings (CR only).

### `/lf`
Unix / modern macOS line endings (LF only).

The line-ending switch sets the **target** EOL for the buffer; it triggers conversion on first save if the loaded file uses a different style.

---

## Navigation: `/g`

### `/g <line>[,<col>]`
Jump to the specified position after loading.

- `<line>` and `<col>` are **1-based**.
- `<line> = -1` jumps to the **end of file**.
- The column part is optional; column 1 is assumed if omitted.

```
Notepad3.exe /g 250 src\Notepad3.c
Notepad3.exe /g 1872,5 src\Notepad3.c
Notepad3.exe /g -1 log.txt
```

---

## Match / find: `/m`

### `/m[<modifiers>] <text>`
Find and select `<text>` immediately after the file is loaded. The match modifiers are appended to the switch letter as flags:

| Modifier | Effect |
|---|---|
| (none) | Plain literal match, case-insensitive, search forward from start of file |
| `-` | Search **backward** from end of file (find last match) |
| `c` | **Case-sensitive** match |
| `r` | **Regular expression** (PCRE2). Implies `b` (backslash escapes) |
| `a` | Dot-matches-all (`.` matches newlines in regex mode) |
| `b` | Enable backslash escape sequences in `<text>`: `\n`, `\t`, `\r`, `\xHH`, `\uHHHH`, … |

Modifiers combine: `/mrc`, `/mra`, `/m-rc`, `/mrb`, etc.

```
Notepad3.exe /m TODO main.c
Notepad3.exe /m- "}" main.c
Notepad3.exe /mrc "FIXME\(.+\)" notes.txt
Notepad3.exe /mb "tab\there" data.txt
```

---

## File-change watching: `/l`

Override the persisted **File Change Notification** mode for this session.

| Form | Effect |
|---|---|
| `/l` | Auto-reload externally modified files (silent reload) |
| `/l0` / `/lo` / `/l-` | Show a message box on external change (no auto-reload) |
| `/l1` / `/lx` / `/l+` | **Exclusive lock** — prevent external edits while open |

> Auto-reload uses `Settings2.AutoReloadTimeout` (default 2000 ms) to avoid read/write races. See [Configuration.md](../config/Configuration.md).

---

## New / scratch documents

### `/q`
Force creation of a new file at the given path **without prompting** if the file does not exist. Useful for shell integrations and scripts.

### `/qs`
Like `/q`, but also **save on relaunch** — used internally by elevated relaunch flows to persist the buffer when an instance restarts itself with higher privileges.

### `/c`
Open a new (untitled) Notepad3 window and **paste the current clipboard contents** into it. One-shot — clipboard is consumed once at startup. Compare with `/b`.

---

## Pasteboard (clipboard collector): `/b`

### `/b`
Activates **Pasteboard mode**: every clipboard change observed while the editor is running is appended as a new entry at the end of the document. Stop at any time via **Edit → Stop Clipboard Monitoring** or by pressing the Pasteboard toolbar toggle. The mode is *runtime-only* (process-local) and is not persisted to the INI.

#### Initial paste behaviour

When `/b` is passed at startup, Notepad3 will additionally **paste the current clipboard once** — but **only** if the buffer is otherwise empty: no file argument, no `/c`, and no MRU auto-load. This asymmetry is intentional. The runtime toggle (`Edit → Toggle Pasteboard`) never auto-pastes.

#### Separator between entries

Subsequent entries are separated according to `Settings2.PasteBoardSeparator`:

| Value | Meaning |
|---|---|
| `\x01` (default) | One document EOL between entries |
| `\0` | No separator |
| anything else | Verbatim character(s) used as separator |

The separator is suppressed for the first paste after enable and when the caret is already at the start of a line.

#### Interaction with file-change monitoring

`Pasteboard` and `FileWatching.MonitoringLog` are **mutually exclusive** at runtime: enabling one greys out the other in the View menu, the tail toolbar button, and the **Monitoring Log** checkbox in the Change-Notification dialog. If the INI persists `MonitoringLog=true` and `/b` is passed on the command line, **`/b` wins for the session** but the INI value is preserved untouched.

#### Combining with `/i` — deferred minimize **(N3)**

When `/b` and `/i` are passed together, Notepad3 keeps the window visible for `Settings2.PasteBoardInitialShowMs` (clamped to 500–5000 ms; default 2000 ms) before minimizing, so the user can briefly see what landed in the buffer. **Holding `Ctrl` at the moment the minimize-timer fires cancels the auto-minimize and keeps the window visible** — useful for inspecting or editing the captured clipboard immediately. Pasteboard mode itself remains active either way. Releasing `Ctrl` before the timer expires has no effect; the check is at the firing instant only.

---

## Syntax scheme / lexer

The following switches override the auto-detected lexer for the loaded file. See [readme/schema/CustomSchema.md](../schema/CustomSchema.md) for scheme management.

### `/s <extension>`
Force the syntax scheme that is associated with `<extension>` (e.g. `/s py` for Python, `/s rb` for Ruby). The argument is a filename extension *without* the leading dot.

### `/d`
Default text scheme (no syntax highlighting / plain text).

### `/h`
**Web Source Code** scheme (HTML).

### `/x`
**XML Document** scheme.

```
Notepad3.exe /s py script.txt
Notepad3.exe /h template.eex
```

---

## Window position and size: `/p`

### `/p <left>,<top>,<width>,<height>[,<dpi>[,<maximize>]]`
Place the window at explicit coordinates. All numbers are integers.

- `<dpi>` is optional; default `96`. Used to scale the geometry on high-DPI monitors.
- `<maximize>` is optional; non-zero zooms the window after placement.

```
Notepad3.exe /p 100,100,1280,800
Notepad3.exe /p 0,0,1920,1080,144,1
```

The alternate prefix `/pos:` is also accepted: `/pos:100,100,1280,800`.

### Position presets

| Switch | Effect |
|---|---|
| `/p0` / `/po` | System default placement |
| `/ps` / `/pd` | Notepad3 default ("snap to default") |
| `/pf` | Fit to foreground monitor |
| `/pl` | Align left |
| `/pr` | Align right |
| `/pt` | Align top |
| `/pb` | Align bottom |
| `/pm` | Maximize (combinable with align flags) |

---

## Window title: `/t`

### `/t <title>`
Override the title-bar text with `<title>` instead of using the filename. The title persists for the session.

```
Notepad3.exe /t "Code Review Notes" notes.md
```

---

## Window behaviour

### `/i`
Start **as a tray icon** — minimize-to-tray on launch. Combined with `/b`, see the deferred-minimize behaviour above.

### `/o`
Force window **always-on-top** for this session, regardless of the persisted setting.

### `/o0` / `/oo` / `/o-`
Explicitly **disable** always-on-top, regardless of the persisted setting.

---

## Window reuse / single-instance

| Switch | Reuse | Single-file instance |
|---|---|---|
| `/n` | No (always new window) | Off |
| `/ns` | No (always new window) | **On** (one window per file) |
| `/r` | **Yes** (reuse existing window) | Off |
| `/rs` | **Yes** (reuse existing window) | **On** (one window per file) |
| `/rp` | Reuse + **preserve file modification time** | Off |

Defaults are taken from `Settings.ReuseWindow` and `Settings.SingleFileInstance`; the command-line switches override per invocation.

---

## Printing

### `/v`
Print the file to the **default printer** immediately and exit. No UI shown. Headers/footers and margins follow the persisted print settings.

### `/vd`
Open the **Print** dialog before printing, allowing printer/range/copies selection. Notepad3 still exits after printing.

> `/v` and `/vd` are Notepad3-specific extensions over the base Notepad2 set (`/v` exists but the dialog-vs-silent split is Notepad3's).

---

## INI / configuration file: `/f`

### `/f <inipath>`
Load settings from `<inipath>` instead of the auto-discovered `Notepad3.ini`. Long paths and environment variables are accepted; the path is normalized and may be created if missing (subject to admin redirect — see [Configuration.md](../config/Configuration.md)).

### `/f0` / `/fo`
Disable INI handling **entirely** for this session. Notepad3 runs on built-in defaults; settings are not loaded and are not saved. Useful for clean-state testing.

```
Notepad3.exe /f "%USERPROFILE%\Documents\notepad3-presentation.ini"
Notepad3.exe /f0
```

---

## Elevation: `/u`

### `/u`
Relaunch with **administrator (UAC) privileges**. Useful for editing files in protected locations (`%ProgramFiles%`, `%SystemRoot%\System32`, …). The current document, position, and most state are preserved across the relaunch.

`/uc` is an internal marker used by the elevation handshake; do not pass it directly.

---

## Shell-integration tokens (no slash prefix)

These tokens are accepted **without** a leading `/` and influence Windows shell integration. Primarily used by the Notepad-replacement registration; see [Replacing Windows Notepad](https://rizonesoft.com/documents/notepad3/).

### `appid=<AppUserModelID>`
Override the AppUserModelID used for taskbar grouping and jump-list affinity. Sets `Settings2.AppUserModelID`.

### `sysmru=0` / `sysmru=1`
Toggle whether the system MRU (Recent Items) and jump list receive entries when Notepad3 opens a file. `sysmru=0` disables, `sysmru=1` enables. Sets `Globals.CmdLnFlag_ShellUseSystemMRU`.

---

## Examples

```bat
:: Open a UTF-8-BOM file with Windows line endings
Notepad3.exe /utf8sig /crlf d:\temp\Test.txt

:: Print silently
Notepad3.exe /v d:\reports\daily.log

:: Open at line 250 column 5, force single-instance per-file
Notepad3.exe /rs /g 250,5 src\Notepad3.c

:: Pasteboard collection, start in tray, hold Ctrl during the
:: ~2-second pre-minimize window to keep the editor visible
Notepad3.exe /b /i

:: Multi-file open with paths containing spaces
Notepad3.exe + "C:\Project Docs\spec.md" "C:\Project Docs\notes.md"

:: Run with no INI at all (clean defaults)
Notepad3.exe /f0

:: Open in elevated mode at a specific position, on top
Notepad3.exe /u /o /p 0,0,1280,800 C:\Windows\System32\drivers\etc\hosts

:: Force the Web Source Code lexer for an unrecognized extension
Notepad3.exe /h template.eex
```

---

## Notes on persistence

Most CLI switches are **session-only overrides** of an INI value:

| Switch | Persists in | Override scope |
|---|---|---|
| `/o`, `/o0` | `Settings.AlwaysOnTop` | Session only |
| `/n`, `/r`, `/ns`, `/rs` | `Settings.ReuseWindow`, `Settings.SingleFileInstance` | Session only |
| `/l`, `/l0`, `/l1` | `Settings2.FileWatchingMode` | Session only |
| `/b` | *(not persisted)* | Session only |
| `/i` | *(not persisted)* | Session only |
| `/p…` | `Settings.WindowPosition` | Session only |
| `/f <path>` | redirects active `Paths.IniFile` | Until next launch |

Cross-reference: [readme/config/Configuration.md](../config/Configuration.md) documents the underlying INI keys.

---

## Compatibility with Notepad2 / Notepad2-mod

Every switch documented in the original [Notepad2 4.2.25 readme](../../Build/Docs/Notepad3.txt) (the "Command Line Switches" block beginning around line 879 of `Build/Docs/Notepad3.txt`) is still recognized. Notepad3 adds:

- **`/vd`** — print with dialog (vs. `/v` silent print).
- **`/y`** — search `%PATH%` for relative file names.
- **`/qs`** — save-on-relaunch flag (used by the elevated-relaunch handshake).
- **`/rp`** — reuse window with file-modification-time preservation.
- **`/uc`** — internal elevation handshake marker (not for direct use).
- **`/b` deferred-minimize and Ctrl-skip** — see [Pasteboard](#pasteboard-clipboard-collector-b) above.
- **`appid=`** / **`sysmru=`** — shell-integration tokens.
- Extended `/m` modifier letters (`a`, `c`, `b`) and combinations.
- Extended `/p` placement presets (`/pf`, `/pl`, `/pt`, `/pr`, `/pb`, `/pm`).
- Extended encoding alias spellings (`/utf-8sig`, `/utf-8-signature`, etc.).

> **MiniPath** ships its own (much smaller) command-line surface and is documented separately. See `minipath/metapath.txt` for the historical metapath options that MiniPath inherits.

---
