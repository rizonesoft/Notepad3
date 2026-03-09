# Notepad3 Configuration Reference

Notepad3 stores its settings in a portable INI file (`Notepad3.ini`). Press **Ctrl+F7** to open it directly in the editor. Most `[Settings2]` changes require restarting Notepad3.

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
- `TimeStampFormat` also accepts `%s` as a placeholder for a `DateTimeFormat`-formatted current date/time string (mixing `strftime()` codes with `%s` is not allowed).
- If you define a custom `TimeStampFormat`, define a matching `TimeStampRegEx` so that "Update Timestamps" can find and replace them correctly.

### Files & Directories

#### `DefaultDirectory=`

Default directory for open/save dialogs when no file is open. Paths can be relative to the Notepad3 program directory.

#### `DefaultExtension=txt`

Default extension for saved files (omit the leading dot).

#### `DenyVirtualSpaceAccess=0`

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

#### `FileChangedIndicator=[@]`

#### `FileDeletedIndicator=[X]`

#### `FileDlgFilters=`

Custom filters for open/save dialogs. Example:

```
Text Files|*.txt;*.wtx;*.log;*.asc;*.doc;*.diz;*.nfo|All Files|*.*
```

#### `FileLoadWarningMB=4`

Size limit (MB) for large-file warning. Set to `0` to disable.

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

#### `SciFontQuality=3`

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

#### `LaunchInstanceFullVisible=true`

### Appearance

#### `UseOldStyleBraceMatching=0`

Set to `1` for legacy brace-matching style.

#### `LargeIconScalePrecent=150`

Display scale percent threshold to switch to larger file-type icons.

#### `DarkModeBkgColor=0x1F1F1F`

#### `DarkModeBtnFaceColor=0x333333`

#### `DarkModeTxtColor=0xEFEFEF`

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

#### `CurrentLineHorizontalSlop=40`

#### `CurrentLineVerticalSlop=5`

#### `UndoTransactionTimeout=0`

Timeout in milliseconds. Set to `1` (clamped to 10 ms minimum) to make nearly every keystroke a separate undo action. `0` disables the timer.

#### `AdministrationTool.exe=`

Reserved for future use.

#### `DevDebugMode=0`

Show encoding detector (UCHARDET) info in the title bar for debugging.

#### `AnalyzeReliableConfidenceLevel=90`

Confidence threshold for the encoding reliability indicator.

#### `LocaleAnsiCodePageAnalysisBonus=33`

Bias added to confidence when the system's ANSI code page matches the detected encoding.

#### `LexerSQLNumberSignAsComment=1`

Enable `#` as a line-comment character in MySQL-dialect SQL. Set to `0` to disable.

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
| 9 | INS/OVR mode toggle |
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
