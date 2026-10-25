# Notepad3 — Frequently Asked Questions

Answers to the questions Notepad3 users ask most often. Many are carried over from the original [Notepad2](https://www.flos-freeware.ch/notepad2.html) FAQ and the [Notepad2-mod](https://xhmikosr.github.io/notepad2-mod/) notes, updated for Notepad3 where behaviour has changed.

Looking for a specific switch, INI key, or keyboard shortcut? The detailed reference docs are linked below — this FAQ focuses on the **why** and the **what to do**, not on listing every option.

- 📖 Configuration reference: [`config/Configuration.md`](../config/Configuration.md)
- ⌨️ Command-line reference: [`cmdln/CmdLnOptions.md`](../cmdln/CmdLnOptions.md)
- 📄 File content flags (`.LOG`, Emacs vars, encoding tags, shebang): [`config/FileContentFlags.md`](../config/FileContentFlags.md)
- 🎨 Schemes, styles, themes: [`schema/CustomSchema.md`](../schema/CustomSchema.md)
- 🔑 Encryption: [`encryption/Encryption.md`](../encryption/Encryption.md)
- 🔍 Encoding detection (uchardet): [`uchardet/EncodingDetection.md`](../uchardet/EncodingDetection.md)

---

## General

### What is Notepad3?

A fast, lightweight, Scintilla/Lexilla-based text editor for Windows. Notepad3 is a fork of XhmikosR's [Notepad2-mod](https://xhmikosr.github.io/notepad2-mod/), which itself extends Florian Balmer's [Notepad2](https://www.flos-freeware.ch/notepad2.html). It is free software released under the [BSD 3-Clause License](../../License.txt).

### How does Notepad3 differ from Notepad2 and Notepad2-mod?

Notepad3 keeps the small footprint and most of the Notepad2 shortcut set, while adding plenty of new functionality on top of Notepad2-mod's code folding, bookmarks, mark-all-occurrences, and word auto-completion. The differences you're most likely to notice:

- **Modern regex engine.** Find/Replace uses **PCRE2** (full Perl-compatible regex) instead of Scintilla's basic subset — multi-line search, lookarounds, named groups, and Unicode all work.
- **More languages.** Over 55 syntax lexers, including AutoHotkey, AutoIt3, AviSynth, Awk, CMake, CoffeeScript, CSV, D, Dart, Go, JSON, Julia, Kotlin, LaTeX, Lua, Markdown, MATLAB, Nim, NSIS, R, Registry, Ruby, Rust, SystemVerilog, Tcl, TOML, VHDL, Verilog, YAML, and more.
- **Better encoding handling.** Automatic detection via [uchardet](https://www.freedesktop.org/wiki/Software/uchardet/), plus a much wider range of supported code pages.
- **Dark mode** and high-DPI awareness, with crisp toolbar icons at every scale factor.
- **Built-in AES-256 encryption** for sensitive files (with an optional command-line companion).
- **Inline math** via [TinyExpr++](https://blake-madden.github.io/tinyexpr-plusplus/) — type `expr=?` to evaluate; live result in the status bar.
- **Focused View** (Ctrl+Alt+V) — temporarily hide everything except the lines matching the current word or selection.
- **grepWin integration** (Ctrl+Shift+F) for fast searching across many files.
- **MiniPath** — a fast companion file browser (Ctrl+M), based on Florian Balmer's metapath.
- **Long-path support** (≥260 chars), a native **ARM64** build, and **26 UI translations**.

### Which Windows versions are supported?

Windows 8.1, 10, and 11 — both 32-bit and 64-bit, plus a native ARM64 build. Older Windows versions (XP / Vista / 7 / 8.0), which Notepad2-mod still supported, are no longer maintained.

### Will Notepad3 ever come to macOS or Linux?

No — Notepad3 is a native Windows application. Many of its building blocks (the Win32 GUI, DirectWrite text rendering, the DarkMode hooks, the `\\?\` long-path support, the IFileOperation save path, the ARM64 build) are tightly coupled to the Windows platform, so a port would essentially be a rewrite. There are no plans for one.

If you need Notepad3 on a non-Windows machine, the practical options are:

- Run it under **[Wine](https://www.winehq.org/)** on macOS or Linux — Notepad3 is reported to work, though some features (DarkMode, certain dialogs) may behave differently.
- Use it from inside a Windows VM.
- For Linux specifically, look at editors that share the same minimalist philosophy — Geany, gedit, or Kate.

### Will Notepad3 ever get tabs (multiple-document interface)?

Probably not — and the design rationale is more interesting than a flat "no".

When the original Notepad2 was written, the standard counter-argument was "the Windows taskbar already groups your open windows" — Florian Balmer's famous *No Tabs!* page. That argument has weakened: **Windows Notepad itself gained tabs in Windows 11**, and many users now genuinely expect tabs in any editor.

The deeper reason Notepad3 stays single-window is what tabs *invite*. Once you add a tab strip, users reasonably ask for the next layer: a project / file-tree pane, session save-and-restore, "find across all open tabs", split panes, an integrated terminal, drag-and-drop reordering between windows, etc. Each is a fair request on its own. **Together they push Notepad3 from a fast Notepad-replacement toward an IDE** — the niche it was deliberately built *not* to fill.

If you want tabs *and* project management, dedicated editors like VS Code, Sublime Text, or Notepad++ already do that very well. Notepad3 stays small and fast on purpose.

Workarounds within the current design:

- The Windows taskbar groups Notepad3 windows; hover for thumbnail previews.
- **Alt+Tab** / **Win+Number** for fast switching between open files.
- `/r` reuses the current window for the next launch; `SingleFileInstance=1` prevents accidentally opening the same file twice.

### Does Notepad3 have built-in source-code auto-formatting?

No. Adding it would mean integrating a different formatter for each language (clang-format for C/C++, Prettier for JS/CSS/HTML, Black for Python, gofmt for Go, …) and then exposing each one's options through the UI. The maintenance cost works against the lightweight design goal.

The recommended approach is to wire up a small external formatter:

- **HTML / XML / CSS** — [HTML Tidy](https://www.html-tidy.org/), [Prettier](https://prettier.io/)
- **C / C++ / Java / C#** — [Astyle](http://astyle.sourceforge.net/), [clang-format](https://clang.llvm.org/docs/ClangFormat.html)
- **Python** — [black](https://black.readthedocs.io/), [autopep8](https://pypi.org/project/autopep8/)
- **JavaScript / TypeScript / JSON** — [Prettier](https://prettier.io/), [js-beautify](https://beautifier.io/)
- **Go** — `gofmt` (bundled with the Go toolchain)

Configure the formatter via **Tools → Open with…** or wrap it in a tiny `.cmd` that takes the current file path. After running it, Notepad3's auto-reload picks up the formatted file.

### Should I use the 32-bit (Win32), 64-bit (x64), x64-AVX2, or ARM64 build?

Short answer: **use the 64-bit (x64) build on a 64-bit Windows install** — that's the right choice for almost everyone. Notepad3 ships four builds for completeness:

| Build | When to use |
|---|---|
| **x64** | Default for most users on 64-bit Windows 8.1 / 10 / 11. |
| **x64-AVX2** | Same as x64, compiled with AVX2 SIMD instructions. Slightly faster on Intel Haswell (2013) / AMD Excavator (2015) and newer CPUs. Won't run on older hardware. |
| **Win32 (x86)** | Only if you're on 32-bit Windows, or you specifically need a 32-bit process. |
| **ARM64** | Native build for Windows-on-ARM (Surface Pro X, Snapdragon laptops). |

Why x64 over Win32 on a 64-bit OS:

- **No WOW64 file-system redirection.** A 32-bit process running on 64-bit Windows silently rewrites paths under `%WINDIR%\System32` to `%WINDIR%\SysWOW64`, and 64-bit-only files become invisible. The 32-bit build of Notepad3 can therefore appear to "miss" files that Explorer (a 64-bit process) shows. The well-known hosts file (`%WINDIR%\System32\drivers\etc\hosts`) is in a directory the redirector skips, so both builds see it — but other system files require the 64-bit build, or the awkward virtual `%WINDIR%\Sysnative` path from 32-bit code.
- **Notepad replacement.** If you've registered Notepad3 as the system Notepad, use the 64-bit build — Explorer hands files to it as a 64-bit caller and 32-bit Notepad3 sees the wrong view of `System32`.

About maximum file size: switching to 64-bit does **not** raise Scintilla's internal document-size limit (it's a 32-bit signed-integer position internally). The practical multi-GB ceiling described under *Can Notepad3 open very large files* applies to all four builds.

### Where do I report a bug?

Please open an issue on GitHub: <https://github.com/rizonesoft/Notepad3/issues>. Include steps to reproduce and your Notepad3 version (Help → About).

Note: don't send Notepad3 bug reports to Florian Balmer — his work on Notepad2 ended with version 4.2.25 in 2011, and Notepad3 has diverged significantly since then.

---

## Installation

### How do I install Notepad3?

No installation needed — Notepad3 is fully **portable**. Extract the archive anywhere you like and double-click `Notepad3.exe`. When you choose to save settings, they are written to `Notepad3.ini` next to the executable.

### How do I uninstall it?

Just delete the Notepad3 folder. Notepad3 does not create any registry entries of its own, so nothing is left behind. (If you set up the system-wide Notepad replacement — see *How do I replace Windows Notepad with Notepad3?* below — the installer that performed the replacement also removes it.)

### Can I run Notepad3 from a USB stick?

Yes. The portable design uses relative paths for recent files, favorites, and "Open with…" entries, so the same stick works on different machines.

### How do I replace Windows Notepad with Notepad3?

Notepad3 supports a clean, registry-based replacement method (using Windows' "Image File Execution Options" mechanism) inherited from Notepad2-mod. The Notepad3 installer can set this up for you — see the [project website](https://rizonesoft.com/documents/notepad3/) for the current installer.

If you set this up by hand, note the `/z` command-line switch: it lets Notepad3 swallow the leading flag (`-p`) that Windows passes to a replacement Notepad, so files still open correctly. Details in [`cmdln/CmdLnOptions.md`](../cmdln/CmdLnOptions.md).

### Why does Windows show a security warning ("publisher could not be verified" / SmartScreen) when I run Notepad3?

When you download a ZIP from the web, every file inside it inherits a **Mark-of-the-Web** (MotW) — an alternate-data stream named `Zone.Identifier` that flags the file as "from an untrusted source". Windows then nags you each time you launch the executable. This is a Windows feature, not anything Notepad3 does.

Two ways to clear it:

1. **Per-file:** right-click `Notepad3.exe` → **Properties** → tick **Unblock** at the bottom of the **General** tab → **OK**.
2. **All extracted files at once (recommended):** **before** extracting, right-click the downloaded ZIP → **Properties** → **Unblock** → **OK**, then extract. The MotW does not propagate to the extracted files.

If SmartScreen specifically complains about an *unrecognized publisher*: the rizonesoft installer is signed and avoids this; the unsigned portable ZIPs may need a one-time approval via **More info → Run anyway**. Notepad3 is open-source and the source is on GitHub if you want to verify the build before allowing it.

---

## Settings & INI File

### Where is my settings file?

`Notepad3.ini`, by default in the same folder as `Notepad3.exe`. Press **Ctrl+F7** to open it directly in the editor.

### Why doesn't Notepad3 create an INI file the first time I run it?

That's intentional — inherited from Notepad2. Notepad3 starts with sensible built-in defaults and only writes an INI when you explicitly ask it to (press **F7** — "Save Settings Now") or change a setting that the UI auto-persists. This keeps a fresh download truly portable: you can try Notepad3 without it leaving anything on disk.

### My user account can't write to the Notepad3 program folder. What now?

This often happens when Notepad3 is installed under `Program Files`. Place a small "stub" INI next to the exe that redirects to a writable location, like your user profile:

```ini
[Notepad3]
Notepad3.ini=%APPDATA%\Rizonesoft\Notepad3\Notepad3.ini
```

Notepad3 will create the target folder automatically and use it for all settings. The redirect can chain up to two levels. Full details in [`config/Configuration.md`](../config/Configuration.md#notepad3).

### How do I keep a separate INI file for each Windows user on a shared machine?

Use the same redirect mechanism, but point it into a per-user path. The simplest approach — works with any number of accounts on the machine, no extra setup per user — is:

```ini
[Notepad3]
Notepad3.ini=%APPDATA%\Notepad3.ini
```

`%APPDATA%` resolves to each user's own roaming profile, so a single shared installation gives every account its own settings, MRU list, and theme.

For an even more explicit per-user file (handy if you'd rather keep the INIs together in `%WINDIR%`):

```ini
[Notepad3]
Notepad3.ini=%WINDIR%\Notepad3-%USERNAME%.ini
```

This produces files like `Notepad3-alice.ini`, `Notepad3-bob.ini`, etc. The redirect is followed on every launch and the target is created automatically.

### Can I keep settings out of `%APPDATA%` entirely?

Yes. Notepad3 always looks beside the exe first — `%APPDATA%` is only consulted as a fallback or when an explicit redirect points there. To force a specific INI for a single launch, pass `/f <path>`. To run completely stateless (no settings read, none saved), use `/f0`.

### How do I change the auto-reload polling interval?

`FileCheckInterval=2000` (ms) under `[Settings2]`. See [Configuration → File watching](../config/Configuration.md).

### How do I set a default directory or default extension for new files?

`DefaultDirectory=` and `DefaultExtension=txt` under `[Settings2]`.

You can also choose **Settings → Default Directory → Set to Current File's Directory** to store a fixed default folder. With no current file and no default folder defined, **Open** tries the most recent file's directory if **Remember Recent Files** is enabled, then the working directory. See [file-dialog starting directories](../paths/FilePathHandling.md#file-dialog-starting-directory).

### How do I customize the date/time format for "Update Timestamps" (Shift+F5)?

You need to define **both** of the following in `[Settings2]`:

- `TimeStampFormat=` — the text that gets inserted (using `strftime()` codes).
- `TimeStampRegEx=` — the pattern Notepad3 uses to find existing timestamps in the file.

The two must agree, otherwise "Update Timestamps" won't be able to locate the old stamp and replace it. See [Configuration → Time stamps](../config/Configuration.md) for examples.

### How do I move or restore the Notepad3 window?

- **Ctrl+Shift+P** — move the current window to the default position on the current monitor.
- **Ctrl+Shift+K** — copy the current window position to the clipboard as a `/p x,y,cx,cy` switch.
- Command line: `/p0` (system default), `/ps` (Notepad3 internal default), `/pf` (full), `/pl/pt/pr/pb` (left/top/right/bottom half), `/pm` (margin from screen edge). Full list in [`cmdln/CmdLnOptions.md`](../cmdln/CmdLnOptions.md).

### How do I bring back a notification I dismissed with "Don't show this again"?

Open the INI file with **Ctrl+F7** and find the `[Suppressed Messages]` section. To bring **everything** back, delete every line in that section. To restore just one specific notification, delete only the matching `Msg…=` line. Save the INI — the notifications will start appearing again on the next relevant event.

### How do I make sure a specific notification is always shown?

If you don't want to be able to suppress a particular message at all (handy on shared machines), set its value to `-1` in `[Suppressed Messages]`. For example:

```ini
[Suppressed Messages]
MsgNotFound=-1
```

With this in place, the "Don't display this message again" checkbox in that dialog is ignored — the message will always appear.

---

## Editing

### What regex syntax does Find/Replace use?

**PCRE2 10.47** — full Perl-compatible regex, including multi-line matches, lookarounds, named groups (`(?<name>…)`), Unicode property escapes, and non-greedy quantifiers. This is a major upgrade from Notepad2's single-line basic regex.

Replacement back-references: `$0`–`$99`, `\0`–`\9`, `${name}`, `${+name}`. Transform-backslashes also handles `\n`, `\t`, `\xHH`, `\uHHHH`. The legacy Notepad2 forms `\1`–`\9` and `\(…\)` keep working too.

### Can I find or replace across multiple lines?

Yes. PCRE2's `.` does not match newlines by default; either include explicit `\r?\n`, or enable single-line mode with `(?s)` at the start of the pattern.

### How do I find or replace a Tab character?

In the Find / Replace dialog, enable either **Regular Expression** or **Transform Backslashes** and use `\t`:

- Find: `\t` matches a literal Tab.
- Replace: `\t` inserts a Tab.

The same trick works for newlines (`\n`, `\r`, `\r\n`) and for arbitrary bytes (`\xHH` — e.g. `\x00` to find or remove NUL bytes).

### Why is Notepad3 slow with very long lines or huge files? Why isn't the horizontal scrollbar pixel-accurate?

These all stem from the same root cause: Notepad3 is built on the **Scintilla** editing component, which was designed for source-code editing — many short lines, modest file sizes, complex per-character syntax styling. That model is fast for code and very efficient on memory, but it doesn't scale gracefully to log-file or binary-dump workloads.

What this means in practice:

- **Very long single lines feel slow.** Scintilla styles each character independently, so one 1-MB line takes far longer to render than the same bytes spread across thousands of lines. Word-wrap helps the *display* but doesn't change the underlying cost. If you're about to do heavy edits on a long line, consider inserting line breaks first (Edit → Block → Column Wrap).
- **Multi-hundred-MB files become heavy.** The whole document, plus undo history and styling state, lives in memory. Editing remains correct, but find/replace passes can take seconds. Hard limits are discussed under *Can Notepad3 open very large files*.
- **Find/Replace can be slow in DBCS code pages.** If your default encoding is the system ANSI for a CJK locale, explicitly setting the corresponding Windows code page (e.g. `Windows-936` for Simplified Chinese rather than generic ANSI) sometimes speeds things up — internally it forces the buffer through UTF-8.
- **The horizontal scrollbar uses a fixed width estimate**, not the true longest-line width. Computing the actual maximum would require styling every line eagerly. End-of-line is always reachable via the **End** key or by clicking the scrollbar arrows.

These are Scintilla-level design choices and unlikely to change. For routine multi-GB log work, dedicated viewers like [klogg](https://klogg.filimonov.dev/) or [glogg](https://glogg.bonnefon.org/) are a better fit.

### How do I jump to a line / column?

`Ctrl+G` opens "Go to line". From the command line, `/g <line>[,<col>]` jumps after opening; `/g -1` jumps to end of file.

### How do I make Notepad3 reuse an existing window instead of opening a new one for each file?

You have two ways:

- **Persistent setting** (every launch): set `ReuseWindow=1` and, if you also want the same file to never open twice, `SingleFileInstance=1` — both under `[Settings]`.
- **Per-launch override:** `/r` reuses the current window, `/rs` reuses it and enforces single-file, `/rp` reuses and forces a paste. To go the other way, `/n` always opens a new window (`/ns` adds the single-file constraint).

### Can I set the tab width and indent width separately?

Yes — they have been separate since Notepad2 and still are. Open **Settings → Tab Settings** (Ctrl+T) to configure both, plus the "Tabs as spaces" toggle.

### Does pressing Tab or Backspace reformat the indentation in my file?

It depends on your settings. In **Settings → Tab Settings** (Ctrl+T) you can choose whether Tab and Backspace operate on whole indentation blocks (Notepad's traditional behaviour — one keypress moves a level) or strictly insert/delete a single character. Both are valid styles; pick whichever feels natural for your editing habits.

### Does Notepad3 lock the directory it's running from?

No. Only the directory that contains `Notepad3.exe` is held open while Notepad3 is running — every other directory you visit (recent files, Open dialogs, etc.) is released as soon as the operation finishes, so you can rename or move folders freely.

### Why does my extensionless file open with HTML or XML highlighting?

Notepad3 peeks at the first few bytes of files that have no recognised extension and tries to guess the language:

- A file starting with `<?xml …?>` is opened with the **XML** scheme.
- HTML-looking content is opened with the **Web Source Code** scheme.
- Files with `.cgi` / `.fcgi` extensions are sniffed for an embedded language (Perl, Python, PHP, etc.).

If the guess is wrong (or you simply don't want it), disable the heuristics with `NoHTMLGuess=1` and `NoCGIGuess=1` under `[Settings2]`. See [`config/Configuration.md`](../config/Configuration.md). You can always set the lexer manually with **F12** for the current file.

### Why does my plain `.txt` file open with AutoHotkey (or another unexpected) syntax highlighting?

This is the most common false positive from Notepad3's **Emacs-style file-variable parser**. The parser scans the first and last 512 bytes of every file for declarations like `mode: auto`, `coding: utf-8`, `tab-width: 4`, etc., and applies them automatically. If your file happens to contain a phrase such as *"wireless mode: auto"* or *"production mode: lite"*, the parser sees a `mode: …` declaration and switches the lexer accordingly — `mode: auto` famously triggers the **AutoHotkey** scheme (since `auto*` matches AutoHotkey's lexer name).

Three ways to deal with it:

- **One-off:** reload the file with **Alt+F8** (Reload without file-variable parsing) and pick the correct lexer manually with **F12**.
- **Per-file:** add `enable-local-variables: 0` near the top — it tells the parser to stop reading further variables in that file.
- **Globally:** set `NoFileVariables=1` under `[Settings]` to turn the parser off for everything.

See [`config/FileContentFlags.md`](../config/FileContentFlags.md) for the full list of recognised variables and how the scan window works.

---

## Files & Encoding

### How do I open a file with a specific encoding?

There are several ways, depending on whether the file is already open:

- **Already open?** Press **F8** to reload it under a different encoding (this is "recode"). For quick access to common code pages, use **Ctrl+Shift+A** (system ANSI) or **Ctrl+Shift+O** (system OEM). The full menu is at **File → Encoding**, including *Recode as Default*.
- **From the command line:** add `/e <encoding-name>` when launching — see [`cmdln/CmdLnOptions.md`](../cmdln/CmdLnOptions.md) for the list of keywords.

Note: in Notepad2, **Ctrl+Shift+F** triggered "Recode as default". In Notepad3 that shortcut now opens grepWin (search-in-files); use the menu instead to recode as the default encoding.

### What is the `.LOG` feature?

A neat carry-over from the original Windows Notepad: if the **very first line** of a text file is exactly `.LOG`, Notepad3 automatically appends the current date and time at the end of the file each time you open it. Great for keeping a running journal — just create a file whose first line is `.LOG`, and every time you open it you get a fresh timestamp ready for typing. See [`config/FileContentFlags.md`](../config/FileContentFlags.md) for details.

### Does Notepad3 support Emacs-style `-*- coding: … -*-` file variables?

Yes. Notepad3 scans the first and last 512 bytes of a file for Emacs file variables and applies the ones it recognises — `mode`, `coding`, `tab-width`, and several more. The scan window size can be tuned in `[Settings2]`.

If you want to load a file *without* this magic, use **Alt+F8** (Reload without file-variable parsing). The full list of recognised variables is in [`config/FileContentFlags.md`](../config/FileContentFlags.md).

### What does Notepad3 do when an open file is changed by another program?

Notepad3 watches the file in the background and reacts according to your preference. Open **Settings → File Change Notification** (Alt+F5) to choose between:

- **Silently reload** if you have no unsaved changes (and just notify if you do),
- **Always prompt** before reloading,
- **Ignore** external changes entirely.

The polling interval is `FileCheckInterval=2000` (ms) under `[Settings2]`. From the command line, `/l` forces auto-reload for that session, `/l0` disables notification entirely, and `/l1` enables it regardless of the persisted setting.

### Why does Notepad3 want to save the file after I just changed its encoding (F8 "Recode")?

Because changing the encoding is a real change, even though the visible text usually doesn't move. **F8 "Recode"** is shorthand for *"reload the bytes from disk and re-decode them as if they were written in this encoding"* — you're telling Notepad3 to override its automatic detection. Internally, the document is always held as UTF-8, and the encoding setting determines how it is **written back to disk**. Once you've picked a different encoding, saving the file will produce a different byte stream — so Notepad3 marks the buffer as modified and prompts you to save before closing.

What to do:

- If the new encoding is what you want, just **save** (Ctrl+S). The file will be written with the new encoding.
- If you triggered the recode by accident or chose the wrong encoding, press **F5** (Reload) to bring back the original bytes from disk.

The same logic applies to "Convert encoding" via the menu — it changes the on-disk representation, hence the prompt.

### Can Notepad3 open very large (multi-GB) files?

Notepad3 loads the entire file into memory, so practical limits depend on how much RAM your system has and whether you're running the 32-bit or 64-bit build:

- **32-bit (Win32) build:** roughly **1–2 GB** is the upper bound (process address space limit).
- **64-bit / x64 / ARM64 builds:** several GB is achievable but slow — every operation, syntax-highlighting pass, and undo step touches the whole buffer. Above ~500 MB the editor remains usable but feels heavy.

Before loading anything large, Notepad3 shows a warning when the file is bigger than `FileLoadWarningMB=4` (default 4 MB) — raise or set to `0` to silence it. There is no streaming or memory-mapped mode.

For multi-GB log files, dedicated viewers like **[klogg](https://klogg.filimonov.dev/)**, **[glogg](https://glogg.bonnefon.org/)**, or even Windows' built-in `more`/`type` are far better suited than a general-purpose editor.

### How do I save a file without an extension (like `hosts`)?

Notepad3 normally appends a default extension (controlled by `DefaultExtension=txt` in `[Settings2]`) when you save a file whose name has none. To override this on a per-file basis, **wrap the filename in double quotes** in the Save-As dialog:

```
"hosts"
```

The quotes tell Notepad3 to take the name literally and skip the default-extension logic. Useful for files like `hosts`, `Makefile`, `README`, `.gitignore`, etc.

### Why does Notepad3 prompt to save a file I opened but never modified?

The usual culprit is the **Strip trailing blanks on save** option (under **File → Line Endings → Default…**). When this is on, opening a file that already contains trailing whitespace puts Notepad3 in an awkward spot: closing the file without saving would silently throw away the cleanup that *should* happen on save. So Notepad3 conservatively asks whether you want to save, even though you didn't type anything.

You have two options:

- **Save once.** This rewrites the file in its cleaned-up form. From then on, opening it is silent.
- **Disable the option** if you'd rather view files without ever rewriting them. The matching INI key is `FixTrailingBlanks=` under `[Settings]`.

The same thing can happen with **Ensure consistent line endings** when a file mixes CRLF, LF, and CR — Notepad3 normalises on load and considers the buffer dirty. Save once, or turn the option off.

### Are files saved atomically? What happens if the power cuts out mid-save?

Yes — by default (`AtomicFileSave=1` under `[Settings2]`), Notepad3 writes the new content to a temporary file in the same folder and then atomically replaces the original. If the power cuts out, you either keep the old file or get the complete new file — never a half-written one. NTFS alternate data streams are also preserved across saves.

### Does Notepad3 handle paths longer than 260 characters?

Yes, transparently. Notepad3 detects when a path crosses the classic Windows length limit and uses the `\\?\` long-path prefix internally. You don't need to enable anything or change any setting — long paths just work.

---

## Customization

### How do I customize syntax colors?

A handful of keyboard shortcuts open the relevant dialogs:

- **F12** — pick the syntax scheme for the current file.
- **Ctrl+F12** — customize the colors and styles of the active scheme.
- **Shift+F12** — choose the secondary default scheme.
- **Alt+F12** — change the default font.

Notepad3 uses a layered theme model (built-in defaults → active theme → your overrides), and supports importing/exporting your customizations as `.ini` files. See [`schema/CustomSchema.md`](../schema/CustomSchema.md) for the complete guide.

### Can I export my color scheme customizations and re-import them later (or share them)?

Yes. Open **Customize Schemes** (Ctrl+F12) and use the **Import** / **Export** buttons. The format is a standard `.ini` with `[Default Text]`, `[C/C++ Source]`, and similar per-lexer sections.

A few useful properties:

- Only the sections you customised need to be exported — missing sections fall back to the active theme on import, so you can ship "delta" themes that only change a few colors.
- Re-importing is non-destructive in the same way: existing entries get overwritten, untouched ones stay.
- Theme INIs travel well across machines, since paths and font fall-backs are normalized.

Pre-made dark/light theme INIs and the layered override model are documented in [`schema/CustomSchema.md`](../schema/CustomSchema.md).

### How do I fine-tune the editor's appearance — caret style, line spacing, font smoothing, …?

Most cosmetic tweaks live in one place: **Customize Schemes** (Ctrl+F12). Open the dialog, expand the **Default Text** node, and edit the relevant style entry. Each style is a small mini-language; see [`schema/CustomSchema.md`](../schema/CustomSchema.md) for the full grammar.

The most-asked tweaks:

- **Caret color, width, block style, blinking.** Edit the **Caret** entry under Default Text. Valid size keywords: `size:1`, `size:2`, `size:3`, or `block`. Add `noblink` to stop the cursor from blinking.
- **Reduce line spacing.** The line height is set by the tallest glyph on each line — usually **Matching Braces** / **Matching Braces Error**. Reduce their `size:` to compact line height. Also reset any **Extra Line Spacing** value you may have set.
- **Selection that doesn't extend past end-of-line.** Edit **Selected Text** and remove the `eolfilled` attribute.
- **Smaller control-character glyphs (NUL, ACK, BEL …).** Notepad3 cannot hide them outright (a Scintilla limitation), but you can shrink them: edit **Control Characters (Font)** and set a smaller `size:`.
- **Font smoothing / anti-aliasing.** Set in `[Settings2]` via `SciFontQuality=`:
  - `0` system default
  - `1` no anti-aliasing
  - `2` standard anti-aliasing
  - `3` ClearType (LCD-optimized — Notepad3's default)

  An active scheme can override this with a `smoothing:` keyword on the **Default Style** entry.

After any change, press **F7** ("Save Settings Now") so your tweaks survive the next launch.

### Does Notepad3 support right-to-left / bidirectional text (Arabic, Hebrew, …)?

Yes — though it's still marked **experimental** in the source. Open **Settings → Bidirectional** and choose:

- **None** — left-to-right only (the default; matches Notepad2 behaviour).
- **Left to Right** — bidirectional rendering with LTR base direction.
- **Right to Left** — bidirectional rendering with RTL base direction.

This came with Scintilla 5.x and was not available in Notepad2 (whose FAQ said "not supported"). Performance and edge-case handling for complex bidi content are still being shaken out — if you hit display glitches, switching back to **None** is a safe fallback. Direct Write rendering is required, which is also Notepad3's default.

### Can I customize the toolbar?

Yes. **Settings → Customize Toolbar** lets you reorder, hide, or restore individual buttons. If you want to see the function name next to each icon, enable text labels in the `[Toolbar Labels]` section of the INI. High-DPI versions of the icons are included automatically.

### How do I switch the UI language?

Notepad3 ships with 26 translations. To pick one, edit the INI (Ctrl+F7) and set:

```ini
[Settings2]
PreferredLanguageLocaleName=de-DE
```

(Replace `de-DE` with your language code — the full list is in [`config/Configuration.md`](../config/Configuration.md).) The matching language DLL must be present in the `lng\` folder next to the exe; both the installer and the portable archive include all 26.

### Can I customize the keyboard shortcuts?

Not from inside the editor — Notepad3's shortcuts are compiled into the application's accelerator table (`src/Notepad3.rc`), so there is no settings dialog or keymap file to edit. This is a common request, but reworking the binding system to be user-configurable hasn't been done.

Workarounds:

- For a few hotkeys you don't use, the matching menu item in the resource file can be retargeted in a custom build.
- If you need system-wide remapping (e.g. swap **Ctrl+F** and **Ctrl+H**), tools like **[AutoHotkey](https://www.autohotkey.com/)** can intercept keys before they reach Notepad3.

The full list of bindings is browsable from the menus inside the editor; the keys reassigned compared to Notepad2 are in *Migrating from Notepad2* below.

### Can I use Notepad3's dark mode without enabling Windows' dark mode?

Partly. Notepad3's dark mode comes in two layers:

1. **The editing surface** (text area, line numbers, margins, status bar) — fully themed by the active syntax scheme. You can switch to a dark scheme at any time via **View → Schemes** or by importing a dark theme INI, regardless of what Windows is set to. See [`schema/CustomSchema.md`](../schema/CustomSchema.md).
2. **The window chrome** (title bar, menus, dialogs, scroll bars) — relies on Windows' dark-mode hooks (uxtheme/user32) and is only available **when Windows itself is in dark mode** (Settings → Personalization → Colors → Dark). This is a Windows API limitation, not a Notepad3 setting.

So: a dark editor surface on a light-themed Windows is fine. A fully dark window — including title bar and menus — requires Windows to be in dark mode too.

---

## Migrating from Notepad2

### Which keyboard shortcuts changed compared to Notepad2?

Most Notepad2 shortcuts are preserved, but a few were reassigned to make room for new Notepad3 / Notepad2-mod features (bookmarks, code folding, grepWin, etc.). Here are the ones that may surprise long-time Notepad2 users:

| Shortcut | In Notepad2 | In Notepad3 |
|---|---|---|
| **Ctrl+0** | Transparent mode | Reset zoom |
| **Alt+G** | — | Toggle transparency |
| **Ctrl+Shift+F** | Recode as default encoding | Open grepWin (search-in-files) |
| **F2** | Expand selection to next match | Go to next bookmark |
| **Shift+F2** | Expand selection to previous match | Go to previous bookmark |
| **Ctrl+F2** | — | Set / clear bookmark on current line |
| **Alt+F2** | — | Clear all bookmarks |
| **Ctrl+Alt+F2** | — | Expand selection to next match |
| **Ctrl+Alt+Shift+F2** | — | Expand selection to previous match |
| **Ctrl+Alt+F** | — | Toggle all folds |
| **Ctrl+Enter** | New line, toggled auto-indent | New line (normal) |
| **Ctrl+Shift+Enter** | — | New line, toggled auto-indent |
| **Ctrl+C / Ctrl+X** with no selection | (no effect) | Copy / cut current line (Visual Studio style) |
| **Ctrl+Shift+.** | — | Copy GUID to clipboard |

A dash (—) means the shortcut was unbound in Notepad2 and is now a Notepad3 addition. The complete, up-to-date binding list lives in the editor's menus.

### Which command-line switches are new in Notepad3?

The most useful additions compared to Notepad2 are the printing switches:

- `/v <file>` — print silently and exit.
- `/vd <file>` — show the print dialog first, then exit.

Plus several Notepad3-specific extensions and additional `/r` variants. See [`cmdln/CmdLnOptions.md`](../cmdln/CmdLnOptions.md) for the full reference — Notepad3-only items are flagged **(N3)**.

---

## Encryption

### Can Notepad3 encrypt files?

Yes — Notepad3 has built-in **AES-256 encryption** (the same standard used by full-disk encryption tools), with passphrases turned into 256-bit keys via SHA-256. The design is inherited from Dave Dyer's NotepadCrypt.

In practice: open or save a file as encrypted, type a passphrase, and Notepad3 takes care of the rest. Your passphrase is **never** written to disk — only the encrypted bytes go to the file. A separate command-line tool (`np3encrypt`) is also available for batch encryption / decryption from scripts. See [`encryption/Encryption.md`](../encryption/Encryption.md) for the full details.

### What is a "master key" and should I set one?

A *master key* is an optional **second** passphrase that gets stored alongside the encrypted file. It encrypts a copy of the file's own encryption key, so anyone who knows the master key can decrypt the file even without knowing the regular file passphrase. There are two practical reasons to use one:

1. **Disaster recovery.** If you ever forget the file passphrase, the master key still gets you in. Choose a master phrase you'll never forget and never use for anything else.
2. **Programs that need to read your file.** If an automated tool has to read configuration that humans edit, you can give the tool the master key while the humans rotate the file passphrase whenever they like.

If neither applies, you can safely skip the master key.

### Is Notepad3's encryption strong enough for my data?

The cryptography itself (AES-256 in CBC mode, SHA-256 for key derivation) is solid — there are no shortcuts to break it. The weak point is almost always the **passphrase** and the **human side**: shoulder-surfing, key-loggers, a sticky note on the monitor, a short word that a dictionary attack can guess.

Practical advice:

- Use a **long, sentence-length passphrase** (the original NotepadCrypt suggests something like *"When it rains in New York, even Chicago is a better place to be"*).
- Don't reuse passphrases across files.
- Don't write them down where someone can find them.

Notepad3 encryption is well-suited to protecting files on a laptop that might get lost or stolen, or shielding sensitive notes from casual snooping. It is **not** designed to resist a determined adversary who has physical access to you or your unlocked machine.

---

## Command Line

### How do I see the built-in command-line help?

Run `Notepad3.exe /?` from a command prompt — Notepad3 opens a help dialog listing every switch.

### Can I capture clipboard text into Notepad3?

Two modes are available:

- **`/c`** — open a new window pre-populated with the **current** clipboard contents. One-shot.
- **`/b`** — start in *pasteboard mode*: every time the clipboard changes, that text is appended as a new entry in the document. Useful for collecting snippets across many copy operations. Stop it any time via **Edit → Stop Clipboard Monitoring** (the editor stays open with the collected text).

Combine either with `/i` to start minimised to the tray.

### Can I run Notepad3 with administrator privileges?

Yes — pass `/u` and Notepad3 re-launches itself elevated (you'll get the standard UAC prompt). The window title shows an "(Administrator)" indicator while elevated, so it's easy to tell.

### How do I print a file from the command line?

- `/v <file>` — print silently with the current settings, then exit.
- `/vd <file>` — show the print dialog first (so you can choose printer, copies, etc.), then exit.

### Where is the full command-line reference?

[`cmdln/CmdLnOptions.md`](../cmdln/CmdLnOptions.md) — every switch with arguments, examples, persistence notes, and Notepad2 / Notepad2-mod compatibility flags.

---

## Building from Source

### What do I need to build Notepad3 from source?

Visual Studio 2022 (toolset v143) with the Windows SDK. Before your first build, run `nuget restore Notepad3.sln` once to fetch dependencies. Before each build, run `Version.ps1` to regenerate the version headers.

### How do I build for a single platform?

```powershell
Build\Build_x64.cmd Release
```

The other platform scripts follow the same pattern: `Build_Win32.cmd`, `Build_ARM64.cmd`, `Build_x64_AVX2.cmd`. To build everything in one go, use `Build\BuildAll.cmd Release`. CI uses the equivalent MSBuild invocation:

```powershell
msbuild Notepad3.sln /m /p:Configuration=Release /p:Platform=x64
```

### How do I add a new syntax lexer?

Most languages already have a lexer in Lexilla — your job is to wire it up to Notepad3's style table. Roughly:

1. Add a `styleLexXXX.c` under `src/StyleLexers/` with an `EDITLEXER` definition.
2. Register it in the `Styles.c` lexer array.
3. Add the localization string IDs for menu/UI text.

There are a few easy-to-miss touchpoints (comment-toggle handlers, property setters, theme INI sections) — see the **"Adding a Lexer"** section of [`CLAUDE.md`](../../CLAUDE.md) for the complete checklist.

---

## License

Notepad3 is free software released under the **BSD 3-Clause License** — see [License.txt](../../License.txt).

### Am I allowed to use Notepad3 commercially / modify it / redistribute it / bundle it?

Yes — the BSD 3-Clause License is one of the most permissive open-source licenses around. In short:

- ✅ **Private, educational, and commercial use** — no restrictions, no royalties.
- ✅ **Modify the source code** — change anything you like.
- ✅ **Redistribute** — original or modified, in source or binary form.
- ✅ **Bundle Notepad3 with your own application** — fine, with the conditions below.

The two requirements are simple:

1. Keep the copyright notice and the BSD-3-Clause text intact in any redistribution (source or binary).
2. Don't use the *Rizonesoft* name or the names of contributors to endorse or promote your derivative product without separate written permission. (This is the "no-endorsement" clause of BSD-3.)

Please don't charge distribution fees beyond reasonable media costs. If Notepad3 saves you time at work, consider [sponsoring the project](https://github.com/sponsors/rizonesoft) — that's how it stays maintained.

### What about the bundled libraries and tools?

The bundled tools and libraries (MiniPath, grepWin, Scintilla, Lexilla, PCRE2, uchardet, TinyExpr++, and more) each carry their own license — all of them permissive (BSD, MIT, MPL, or similar). Look for a `License.txt` (or equivalent) inside each component's folder for the exact terms. None of them impose copyleft on derivative work.
