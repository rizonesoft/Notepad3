# Notepad3 Configuration Reference

Notepad3 stores its settings in a portable INI file (`Notepad3.ini`). Press **Ctrl+F7** to open it directly in the editor. Most `[Settings2]` changes require restarting Notepad3.

> Path-handling behaviour controlled by keys below (`AtomicFileSave`, `ResolveUNCPaths`, `DefaultDirectory`, `OpenWithDir`, `FavoritesDir`, `HyperlinkShellExURLWithApp`, `HyperlinkFileProtocolVerb`, `ShowHypLnkToolTip`, …) is summarised in [`../paths/FilePathHandling.md`](../paths/FilePathHandling.md).

---

## `[Notepad3]`

This section can redirect to a different settings file. Useful when a non-elevated user cannot write to the Notepad3 program directory:

```ini
Notepad3.ini=%APPDATA%\Rizonesoft\Notepad3\Notepad3.ini
```

Or for per-user settings:

```ini
Notepad3.ini=%WINDIR%\Notepad3-%USERNAME%.ini
```

---

## `[Settings]`

These settings are managed through Notepad3's user interface (Menu → Settings). For example:

#### `SettingsVersion=5`

#### `Favorites=%APPDATA%\Rizonesoft\Notepad3\Favorites\`

#### `AnalyzeReliableConfidenceLevel=50`

Confidence threshold (0–100) for the encoding analysis reliability indicator. Configurable in Menu → Settings → Encoding Defaults, next to "Perform ANSI Code Page analysis".

---

## `[Settings2]`

Advanced settings that can only be edited manually. Press **Ctrl+F7** to open the INI file.

### Localization

#### `PreferredLanguageLocaleName=en-US`

The default value is determined by the OS language setting. Fallback: `en-US`.

Available languages:

| Language | Locale Code |
|----------|-------------|
| English/United States | `en-US` (internal default) |
| Afrikaans/South Africa | `af-ZA` |
| Belarusian/Belarus | `be-BY` |
| German/Germany | `de-DE` |
| Greek/Greece | `el-GR` |
| English/United Kingdom | `en-GB` |
| Spanish/Spain | `es-ES` |
| Finnish/Finland | `fi-FI` |
| French/France | `fr-FR` |
| Hindi/India | `hi-IN` |
| Hungarian/Hungary | `hu-HU` |
| Indonesian/Indonesia | `id-ID` |
| Italian/Italy | `it-IT` |
| Japanese/Japan | `ja-JP` |
| Korean/Korea | `ko-KR` |
| Dutch/Netherlands | `nl-NL` |
| Polish/Poland | `pl-PL` |
| Portuguese/Brazil | `pt-BR` |
| Portuguese/Portugal | `pt-PT` |
| Russian/Russia | `ru-RU` |
| Slovak/Slovakia | `sk-SK` |
| Swedish/Sweden | `sv-SE` |
| Turkish/Turkey | `tr-TR` |
| Vietnamese/Vietnam | `vi-VN` |
| Chinese Simplified/China | `zh-CN` |
| Chinese Traditional/Taiwan | `zh-TW` |

### IME & Input

#### `IMEInteraction=0`

Scintilla IME composition mode:
- `-1` — Auto-detect (one-shot at startup): Korean code pages (`949`, `1361`) → inline (`1`); all other languages → windowed (`0`).
- `0` — `SC_IME_WINDOWED`: IME composition shown in a separate floating window.
- `1` — `SC_IME_INLINE`: composition happens inline at the caret. Recommended when working with Far East languages that emit long composition strings.

Range: `-1`–`1`.

### Date & Time

#### `DateTimeFormat=`

Locale-dependent short format. Specify the short date/time format using [`strftime()`](https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strftime-wcsftime-strftime-l-wcsftime-l?view=vs-2019) syntax.

#### `DateTimeLongFormat=`

Locale-dependent long format.

#### `TimeStampRegEx=`

Default: `\$Date:[^\$]+\$` — Find pattern to match timestamps for update.

#### `TimeStampFormat=`

Default: `$Date: %Y/%m/%d %H:%M:%S $`

This pattern is used by **Shift+F5** to update timestamps, e.g., `$Date: 2018/04/26 00:52:39 $`.

**Notes:**
- All `DateTime` formats accept [`strftime()`](https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strftime-wcsftime-strftime-l-wcsftime-l?view=vs-2019) format strings.
- `DateTimeFormat` is also the format used by the **`.LOG` auto-timestamp** feature (files starting with `.LOG`); see [File Content Flags](FileContentFlags.md#log-auto-timestamp).
- `TimeStampFormat` also accepts `%s` as a placeholder for a `DateTimeFormat`-formatted current date/time string (mixing `strftime()` codes with `%s` is not allowed).
- If you define a custom `TimeStampFormat`, define a matching `TimeStampRegEx` so that "Update Timestamps" can find and replace them correctly.

### Files & Directories

#### `DefaultDirectory=`

Default directory for open/save dialogs when no file is open. Paths can be relative to the Notepad3 program directory.

#### `DefaultExtension=txt`

Default extension for saved files (omit the leading dot).

#### `DenyVirtualSpaceAccess=0`

Set to `1` to disable virtual-space access (the area beyond each line's end). With access denied, rectangular (column) selections cannot extend past the shortest line end. Default (`0`) allows rectangular selection to span virtual space — useful for aligning columns across lines of varying length.

#### `filebrowser.exe=minipath.exe`

External program launched by the Browse toolbar button. Defaults to MiniPath.

Additional command line switches can be appended. The currently opened file is passed as the last argument.

> **Note:** Pathnames with spaces need quadruple quoting (`""path to/file.exe""`) when no arguments follow, or double quoting (`"path to/file.exe" /arg`) with arguments.

Alternative file browsers:
- `filebrowser.exe=explorer.exe` — Windows Explorer
- `filebrowser.exe=Explorer++.exe` — [Explorer++](https://explorerplusplus.com/)
- `filebrowser.exe=Q-Dir_x64+.exe` — [Q-Dir](https://www.softwareok.de/?seite=Freeware/Q-Dir/)

#### `grepWin.exe=grepWinNP3.exe`

Integrated search-and-replace tool with regex support. Launch via:
- File → Launch → Search in Files
- Edit → Search → Search in Files
- **Ctrl+Shift+F**

#### `FileCheckInterval=2000`

Interval in milliseconds to check for external file modification. Min: 200 ms (notify immediately if ≤ 200).

#### `FileWatchingMethod=0    ;(0=both[default], 1=poll-only, 2=push-only)`

#### `FileChangedIndicator=[@]`

#### `FileDeletedIndicator=[X]`

#### `FileDlgFilters=`

Custom filters for open/save dialogs. Example:

```
Text Files|*.txt;*.wtx;*.log;*.asc;*.doc;*.diz;*.nfo|All Files|*.*
```

#### `FileLoadWarningMB=4`

Size limit (MB) for large-file warning. Set to `0` to disable.

#### `FileVarScanBytes=512`

Number of bytes scanned at the file's head **and** (as fallback) at its tail for Emacs file variables, Vim modelines, and encoding tags. Increase if your project's headers (license blocks, banners) push the modeline past 512 bytes. Range: `256`–`2048`. See [File Content Flags](FileContentFlags.md).

#### `MultiFileArg=0`

Set to `1` to allow multiple files on the command line. Default (`0`) accepts a single file like Windows Notepad. Use `+` and `-` command-line switches to override; `/z` behaves like `-`.

### Language Detection

#### `NoCGIGuess=0`

Set to `1` to disable language detection for `.cgi` and `.fcgi` files.

#### `NoHTMLGuess=0`

Set to `1` to disable HTML/XML detection for files without extensions.

### Editing Behavior

#### `NoCopyLineOnEmptySelection=0`

Set to `1` to prevent **Ctrl+C** from copying the current line when nothing is selected.

#### `NoCutLineOnEmptySelection=0`

Set to `1` to prevent **Ctrl+X** from cutting the current line when nothing is selected.

#### `CopyMultiSelectionSeparator=`

String placed between selections when copying a multi-selection (**Ctrl+C** with multiple cursors/selections active). Default (key absent) uses the document's current EOL character. Set to empty for no separator. Supports escape sequences: `\r` (CR), `\n` (LF), `\t` (tab), `\xHH` (hex character). Common choices:

```ini
CopyMultiSelectionSeparator=       ;empty — selections joined without any separator
CopyMultiSelectionSeparator=\t     ;tab-separated
CopyMultiSelectionSeparator=\r\n   ;CRLF between each selection
```

### Clipboard Monitoring

Notepad3 can be launched in **clipboard monitoring mode** with the `/b` command-line switch. In this mode, every clipboard change is automatically appended as a new entry at the end of the document. Stop monitoring at any time via **Edit → Stop Clipboard Monitoring** without closing the editor. While active, the status bar shows **`CBS`** in the INS/OVR field.

#### `PasteBoardDebounceMs=200`

Debounce interval in milliseconds before a clipboard change is pasted into the document. Increase this value if rapid successive copies result in missed or doubled entries. Range: `0`–`5000`.

#### `PasteBoardSeparator=`

String inserted verbatim between pasted entries — all newlines must be included explicitly. Default (key absent) uses the document's current EOL character, placing entries on consecutive lines. Set to empty (`PasteBoardSeparator=`) for no separator (entries concatenated). Supports escape sequences: `\r` (CR), `\n` (LF), `\t` (tab), `\xHH` (hex character). Common choices:

```ini
PasteBoardSeparator=\r\n---\r\n   ; dashed line on its own line (CRLF doc)
PasteBoardSeparator=\n---\n       ; dashed line on its own line (LF doc)
PasteBoardSeparator=\r\n          ; one line break (consecutive lines, CRLF doc)
PasteBoardSeparator=\r\n\r\n      ; blank line between entries (CRLF doc)
PasteBoardSeparator=\r\n\r\n\r\n  ; two blank lines between entries
```

> **Note:** The separator string is inserted verbatim — exactly the number of newline characters you specify will appear between entries. To place a text separator (e.g. `---`) on its own line, include the surrounding newlines: `\n---\n`.

#### `PasteBoardAddTimestamp=0`

Set to `1` to prepend a `[HH:MM:SS]` timestamp to each pasted entry.

#### `PasteBoardInitialShowMs=1500`

When Notepad3 is launched with **both `/B` (clipboard monitoring) and `/I` (start minimized)**, the immediate minimize is deferred so the user can see the window populate with the one-shot auto-pasted clipboard content. The window stays visible for this many milliseconds, then minimizes (to the tray or taskbar, per `Settings.MinimizeToTray`). Range: `500`–`5000` ms (clamped at load time). Has no effect unless both `/B` and `/I` are passed on the command line — `/I` alone still minimizes immediately as before.

#### `NoFadeHidden=0`

Set to `1` to disable fading of hidden objects in file lists (Favorites, etc.).

#### `NoFileVariables=0`

Set to `1` to disable file variable parsing. Encoding tag parsing can be disabled separately in Menu → File → Encoding → Default.

Notepad3 parses Emacs-style variables in the first and last 512 bytes of a file:

| Variable | Effect |
|----------|--------|
| `coding: utf-8;` | File encoding tag |
| `mode: python;` | Syntax scheme (name or extension) |
| `tab-width: 8;` | Tab width |
| `c-basic-indent: 2;` | Indentation size |
| `indent-tabs-mode: nil;` | Insert tabs as spaces (`nil`/`false`/`0`) or not (`true`/`1`) |
| `c-tab-always-indent: true;` | Tab key reformats whitespace (`true`/`1`) or not |
| `fill-column: 64;` | Long-line limit (does not auto-show the visual marker) |
| `truncate-lines: false;` | Word wrap: enable (`nil`/`false`/`0`), disable (`true`/`1`) |
| `enable-local-variables: false;` | Disable variable parsing but keep encoding tags |

To bypass both file variable and encoding tag parsing, reload the file with **Alt+F8**.

### Portability & Window

#### `PortableMyDocs=1`

Store recent file paths relative to `My Documents` for USB portability across Windows versions. Enabled by default when `RelativeFileMRU=1`.

#### `OpacityLevel=75`

Window opacity (%) in transparent mode.

#### `FindReplaceOpacityLevel=50`

Find/Replace dialog opacity (%) in transparent mode.

#### `RelativeFileMRU=1`

Store recent file paths as relative when on the same drive/share as Notepad3.exe.

#### `ReuseWindow=0`

Managed by Notepad3 (Menu → Settings → Window → Reuse Window, **Ctrl+Shift+L**). When enabled, new instances hand off to the existing window.

#### `SaveBlankNewFile=true`

Whether to prompt for save when closing an untitled document that contains only whitespace.

#### `DiscardOnClosingUntitledPasteBoard=0`

Set to `1` to enable an opt-out variant of the close-modified prompt for **Untitled** documents (documents with no associated file path) **while clipboard-monitoring (PasteBoard) mode is active** — i.e. when Notepad3 was launched with `/B` or PasteBoard mode was toggled on via `Edit → Toggle Clipboard Monitoring`. With the default (`0`), or whenever PasteBoard mode is inactive, the standard Save / Discard / Cancel prompt is shown without a "Don't show this dialog again" checkbox.

When the gate matches (flag set + Untitled + PasteBoard active), closing a modified untitled document shows the same Save / Discard / Cancel prompt with a **"Don't show this dialog again"** checkbox and Discard pre-selected as the default button. If the box is checked alongside a Save or Discard answer, that choice is persisted under `[Suppressed Messages] MsgDiscardUntitled` in the INI and replays silently on subsequent close-untitled events — useful when Notepad3 is regularly used as a clipboard scratchpad. Cancel is never persisted. To re-enable the prompt, delete the `MsgDiscardUntitled` line from the `[Suppressed Messages]` section, or simply set `DiscardOnClosingUntitledPasteBoard=0` (the loader auto-clears the suppression entry on next start). Has no effect on documents that have a file path or when PasteBoard mode is not active.

#### `SciFontQuality=3`

Scintilla font rendering quality:
- `0` — Default (system-chosen)
- `1` — Non-antialiased
- `2` — Antialiased
- `3` — LCD-optimized (ClearType; default)

Range: `0`–`3`. A per-resolution override is supported in the `[Window]` section via a key named `<ResX>x<ResY> SciFontQuality=<n>` (e.g. `1920x1080 SciFontQuality=2`), so different monitor layouts can use different font quality settings.

#### `SimpleIndentGuides=0`

Set to `1` to prevent indentation guides from jumping across empty lines.

#### `SingleFileInstance=1`

Managed by Notepad3.

#### `ShellAppUserModelID=Rizonesoft.Notepad3`

#### `ShellUseSystemMRU=1`

Controls taskbar grouping, Jump List, and system MRU behavior. See [Replacing Windows Notepad](https://rizonesoft.com/documents/notepad3/) for details.

#### `StickyWindowPosition=0`

Managed by Notepad3 (Menu → View → Position → Sticky Window Position). Remembers current position on restart instead of last-closed position.

#### `SubWrappedLineSelectOnMarginClick=false`

Set to `true` for old selection behavior: single click selects the visible sub-line, double click selects the entire wrapped line.

#### `LaunchInstanceWndPosOffset=28`

Pixel offset cascade applied to each new Notepad3 window launched as an additional instance. Each successive window is shifted by this many pixels (both X and Y) relative to the previous one, so overlapping windows remain distinguishable. Range: `-10000`–`10000`. Set to `0` to spawn new windows at the exact same position.

#### `LaunchInstanceFullVisible=true`

When `true` (default), newly launched windows are clamped so they remain fully visible on the target monitor, even if the `LaunchInstanceWndPosOffset` cascade would push them off-screen. Set to `false` to allow partially off-screen positions (e.g. if you rely on the OS to handle multi-monitor placement).

#### `MaxFileDropInstances=20`

Maximum number of new Notepad3 windows spawned in a single drag-and-drop operation. When the current document has unsaved changes (or `Ctrl` is held during the drop), each dropped file opens in its own new window so the dirty document is never disturbed. This setting caps that fan-out — files beyond the cap are skipped with a single warning. Range: `-1`–`100`. Set to `-1` to remove the cap; default is `20`.

Drops originating from a Windows temp root (`%TEMP%`, `%LOCALAPPDATA%\Temp`, `%LOCALAPPDATA%\Microsoft\Windows\INetCache`) — e.g. files dragged from 7-Zip, Outlook attachments, downloads — are first snapshotted into `<%TEMP%>\Notepad3\drops\` so the source can be deleted by the originating process without breaking the load. Snapshots are pruned automatically (older than one hour on each subsequent drop, older than 24 hours at startup).

### Appearance

#### `RenderingTechnology=1`

Scintilla rendering back-end:

| Value | Name | Description |
|-------|------|-------------|
| `0` | GDI | Classic Win32 rendering |
| `1` | DirectWrite | Direct2D (default on x86/x64) |
| `2` | DirectWriteRetain | Direct2D preserving the back buffer between frames (default on ARM64 to prevent flicker on Qualcomm Adreno GPUs and the Win11 25H2 DWM compositor) |
| `3` | DirectWriteDC | DC-based Direct2D |

Also configurable via **Menu → View → Rendering Technology**. A per-resolution override is supported in the `[Window]` section, e.g. `1920x1080 RenderingTechnology=0`, so a docked laptop and an external 4K monitor can use different back-ends.

**Tip:** On ARM64 systems showing rendering glitches, try `0` (GDI) or `3` (DirectWriteDC) as a workaround.

#### `UseOldStyleBraceMatching=0`

Set to `1` for legacy brace-matching style.

#### `LargeIconScalePrecent=150`

Display scale percent threshold to switch to larger file-type icons.

#### `DarkModeBkgColor=0x1F1F1F`

Dark-mode window background color. The three `DarkMode*Color` entries use Win32 `COLORREF` byte order — `0xBBGGRR` (blue, green, red), **not** the usual HTML `0xRRGGBB`. So `0x1F1F1F` is a near-black neutral, and e.g. `0x0000FF` is pure red, not pure blue.

#### `DarkModeBtnFaceColor=0x333333`

Dark-mode button face color. Same `0xBBGGRR` byte order as above.

#### `DarkModeTxtColor=0xEFEFEF`

Dark-mode text color. Same `0xBBGGRR` byte order as above.

### Web Actions

#### `WebTemplate1=https://google.com/search?q=%s`

#### `WebTmpl1MenuName=Open Web Action 1`

#### `WebTemplate2=https://en.wikipedia.org/w/index.php?search=%s`

#### `WebTmpl2MenuName=Open Web Action 2`

### Auto-Completion & Editing

#### `ExtendedWhiteSpaceChars=:`

Additional ASCII characters treated as word delimiters for Accelerated Word Navigation.

#### `AutoCompleteWordCharSet=`

Set automatically for CJK input languages. Custom character sets limit the auto-completion word list to words composed of these characters only (case-insensitive).

#### `AutoCompleteFillUpChars=`

Characters that accept an auto-completion item. For Enter-to-complete behavior:

```ini
AutoCompleteFillUpChars=\r\n
```

To also accept on `(` or `.`:

```ini
AutoCompleteFillUpChars=\r\n(.
```

#### `LineCommentPostfixStrg=`

String appended/removed after comment tags on line-comment toggle. Double-quote if it contains spaces:

```ini
LineCommentPostfixStrg=" "
```

### Hyperlinks

#### `HyperlinkShellExURLWithApp=""`

External application for **Ctrl+Click** on URLs. If empty, the OS default handler is used.

#### `HyperlinkShellExURLCmdLnArgs="${URL}"`

Command-line arguments for the URL application. Use `${URL}` as placeholder:

```ini
HyperlinkShellExURLCmdLnArgs="--incognito "${URL}""
```

#### `HyperlinkFileProtocolVerb=""`

`ShellExecuteEx` verb: `""` (default), `"edit"`, `"explore"`, `"find"`, `"open"`, `"print"`, `"properties"`, `"runas"`.

### Fonts

#### `CodeFontPrefPrioList="Cascadia Code,Cascadia Mono,Cousine,Fira Code,Source Code Pro,Roboto Mono,DejaVu Sans Mono,Inconsolata,Consolas,Lucida Console"`

Font priority list for the "Common Base" scheme.

#### `TextFontPrefPrioList="Cascadia Mono,Cousine,Roboto Mono,DejaVu Sans Mono,Inconsolata,Consolas,Lucida Console"`

Font priority list for the "Text Files" scheme.

### Miscellaneous

#### `UpdateDelayMarkAllOccurrences=50`

Debounce interval (milliseconds) before the "mark all occurrences" highlight is recomputed after a document change. Lower values feel more responsive but cost more CPU on large documents; higher values reduce flicker while typing. Range: ~`20`–`10000` ms (the lower bound is clamped to `USER_TIMER_MINIMUM * 2`, typically around 20 ms).

#### `CurrentLineHorizontalSlop=40`

Horizontal caret slop in columns, passed to Scintilla's `SCI_SETXCARETPOLICY`. Defines the minimum horizontal gap (in character cells) the caret keeps from the left/right edge of the view before the view starts to scroll horizontally. Larger values keep more surrounding context visible; `0` disables the slop. Range: `0`–`240`.

#### `CurrentLineVerticalSlop=5`

Vertical caret slop in lines, passed to Scintilla's `SCI_SETYCARETPOLICY`. Defines the minimum vertical gap (in whole lines) the caret keeps from the top/bottom of the view before the view scrolls. `0` forces the "even caret" policy (caret always centered-ish); positive values set a fixed top/bottom margin. Range: `0`–`100`.

#### `UndoTransactionTimeout=0`

Timeout in milliseconds. Set to `1` (clamped to 10 ms minimum) to make nearly every keystroke a separate undo action. `0` disables the timer.

#### `AdministrationTool.exe=`

Reserved for future use.

#### `DevDebugMode=0`

Show encoding detector (UCHARDET) info in the title bar for debugging.

#### `LocaleAnsiCodePageAnalysisBonus=33`

Bias added to confidence when the system's ANSI code page matches the detected encoding.

#### UchardetLanguageFilter=31
```
;  Bitmask controlling which CJK charset probers are active in uchardet:
;    Bit 0 (0x01/1)  = Chinese Simplified (GB18030)
;    Bit 1 (0x02/2)  = Chinese Traditional (Big5, EUC-TW)
;    Bit 2 (0x04/4)  = Japanese (Shift_JIS, EUC-JP)
;    Bit 3 (0x08/8)  = Korean (EUC-KR)
;    Bit 4 (0x10/16) = Non-CJK (single-byte encodings)
;  Common values:
;    31 = All probers (default)
;    27 = Exclude Japanese (fix GB18030 misdetected as EUC-JP)
;    17 = Chinese Simplified + Non-CJK only
;     3 = Chinese only (simplified + traditional)
```

#### `LexerSQLNumberSignAsComment=1`

Enable `#` as a line-comment character in MySQL-dialect SQL. Set to `0` to disable.

#### `AtomicFileSave=true`

When `true` (default), `Save` writes to a temporary file alongside the target and then atomically replaces the original via `ReplaceFileW`. This preserves the original file's ACLs, alternate data streams, and attributes on failure (nothing is lost if the write is interrupted). Set to `false` to write in-place, which is faster on very large files but offers no rollback if the write fails mid-way. During Windows session end (`WM_ENDSESSION`) the atomic path is bypassed regardless of this setting.

#### `ExitOnESCSkipLevel=2`

Controls ESC key behavior:
- **2** (default): Cancel each state separately (message boxes → selection → exit)
- **1**: Cancel message boxes, skip selection state
- **0**: Cancel all states and proceed to exit (if configured)

#### `ZoomTooltipTimeout=3200`

Zoom tooltip display duration in milliseconds. Set to `0` to disable.

#### `WrapAroundTooltipTimeout=2000`

Search wrap-around tooltip duration in milliseconds. Set to `0` to disable.

---

## `[Statusbar Settings]`

Customize the status bar layout: number, order, width, and prefix/postfix text of fields.

#### `VisibleSections=0 1 15 14 2 4 5 6 7 8 9 10 11`

Defines visible fields and their order:

| Section | Content |
|---------|---------|
| 0 | Ln — Line number / Total lines |
| 1 | Col — Column number / Long-line limit |
| 2 | Sel — Selected characters |
| 3 | Sb — Selected bytes (UTF-8) |
| 4 | SLn — Selected lines |
| 5 | Occ — Marked occurrences |
| 6 | File size (UTF-8 mode) |
| 7 | Encoding (double-click to open Encoding dialog) |
| 8 | EOL mode (toggle CR+LF / LF / CR) |
| 9 | INS/OVR mode toggle (double-click); shows **CBS** when clipboard monitoring is active — double-click **CBS** stops monitoring |
| 10 | STD/2ND text mode toggle |
| 11 | Current scheme (double-click to open scheme selector) |
| 12 | Ch — Character count from line start |
| 13 | Repl — Replaced occurrences |
| 14 | Eval — TinyExpr evaluation |
| 15 | U+ — Unicode code point (UTF-16) at caret |

#### `SectionPrefixes=Ln  ,Col  ,Sel  ,Sb  ,SLn  ,Occ  ,,,,,,,Ch  ,Repl  ,Eval  ,U+,`

Redefine display prefixes. Comma-separated; spaces are significant.

#### `SectionPostfixes=,,,,,,,,,,,,,,,,`

Redefine display postfixes. Comma-separated; spaces are significant.

#### `SectionWidthSpecs=30 20 20 20 20 20 20 0 0 0 0 0 0 0 20 24`

Relative field widths. `0` = auto-fit, negative = fixed pixel width (text truncated).

#### `ZeroBasedColumnIndex=0`

Start column counting at 0 or 1.

#### `ZeroBasedCharacterCount=0`

Start character counting at 0 or 1.

---

## `[Toolbar Labels]`

Display function names next to toolbar icons:

```ini
01=New
02=Open
03=Browse
04=Save
05=Undo
06=Redo
07=Cut
08=Copy
09=Paste
10=Find
11=Replace
12=Word Wrap
13=Zoom In
14=Zoom Out
15=Scheme
16=Customize Schemes
17=Exit
18=Save As
19=Save Copy
20=Delete
21=Print
22=Favorites
23=Add to Favorites
24=Toggle Folds
25=Execute Document
26=Focused View
27=Monitoring Log
28=History
29=Always On Top
30=Search in Files
31=Reset Zoom
32=New Empty Window
```

---

## `[Window]`

#### `<ResX>x<ResY> DefaultWindowPosition=`

Managed by Notepad3 (Menu → View → Position → Save as Default Position). Recall with **Ctrl+Shift+P**.
