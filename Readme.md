# Notepad3

Notepad3 is free and open source. Your support helps keep development active.

[![Sponsor](https://img.shields.io/github/sponsors/rizonesoft?style=flat-square&logo=github)](https://github.com/sponsors/rizonesoft)
[![Donate](https://img.shields.io/badge/Donate-PayPal-blue.svg?style=flat-square)](https://www.paypal.com/donate/?hosted_button_id=7UGGCSDUZJPFE)

### Build Status
Continuous integration ensures code quality across compilers and platforms.

[![AppVeyor](https://img.shields.io/appveyor/ci/rizonesoft/notepad3/master.svg?style=flat-square&label=AppVeyor&color=1a7f37)](https://ci.appveyor.com/project/rizonesoft/notepad3/branch/master)
[![CI](https://img.shields.io/github/actions/workflow/status/rizonesoft/Notepad3/build.yml?style=flat-square&label=CI&color=1a7f37)](https://github.com/rizonesoft/Notepad3/actions/workflows/build.yml)
[![Latest Release](https://img.shields.io/github/v/release/rizonesoft/Notepad3?style=flat-square&label=Release)](https://rizonesoft.com/downloads/notepad3/)
[![Nightly](https://img.shields.io/github/v/release/rizonesoft/Notepad3?include_prereleases&style=flat-square&label=Nightly&color=orange)](https://github.com/rizonesoft/Notepad3/releases)

### Tech Stack
Built with modern C++ on the powerful Scintilla editing component.

[![Windows](https://img.shields.io/badge/Platform-Windows-0078D6?style=flat-square&logo=windows)](https://rizonesoft.com/downloads/notepad3/)
[![C++](https://img.shields.io/badge/Language-C%2B%2B-00599C?style=flat-square&logo=cplusplus)](https://github.com/rizonesoft/Notepad3)
[![Scintilla](https://img.shields.io/badge/Editor-Scintilla-4B8BBE?style=flat-square)](https://www.scintilla.org/)
[![Architecture](https://img.shields.io/badge/Arch-x86%20%7C%20x64%20%7C%20x64--AVX2%20%7C%20ARM64-informational?style=flat-square)](https://rizonesoft.com/downloads/notepad3/)

### Community
Join our growing community of contributors and users.

[![Contributors](https://img.shields.io/github/contributors/rizonesoft/Notepad3?style=flat-square&label=Contributors&color=1a7f37)](https://github.com/rizonesoft/Notepad3/graphs/contributors)
[![Last Commit](https://img.shields.io/github/last-commit/rizonesoft/Notepad3?style=flat-square&label=Last%20Commit&color=1a5fb4)](https://github.com/rizonesoft/Notepad3/commits)
[![Open Issues](https://img.shields.io/github/issues/rizonesoft/Notepad3?style=flat-square&label=Open%20Issues&color=9a6700)](https://github.com/rizonesoft/Notepad3/issues)
[![Closed Issues](https://img.shields.io/github/issues-closed/rizonesoft/Notepad3?style=flat-square&label=Closed%20Issues&color=6e40c9)](https://github.com/rizonesoft/Notepad3/issues?q=is%3Aissue+is%3Aclosed)
[![Pull Requests](https://img.shields.io/github/issues-pr/rizonesoft/Notepad3?style=flat-square&label=Pull%20Requests&color=0e7490)](https://github.com/rizonesoft/Notepad3/pulls)
[![Repo Size](https://img.shields.io/github/repo-size/rizonesoft/Notepad3?style=flat-square)](https://github.com/rizonesoft/Notepad3)
[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg?style=flat-square)](https://opensource.org/licenses/BSD-3-Clause)

Notepad3 is a fast and light-weight Scintilla-based text editor with syntax highlighting. It has a small memory footprint, but is powerful enough to handle most programming jobs. [Download Notepad3 here](https://rizonesoft.com/downloads/notepad3).

> *Notepad3 is based on code from Florian Balmer's Notepad2 and XhmikosR's Notepad2-mod. MiniPath is based on code from Florian Balmer's metapath.*

## Important links!
* Download page - https://rizonesoft.com/downloads/notepad3
* Latest changelog (release notes) - https://rizonesoft.com/downloads/notepad3/update
* Full changelog (all versions/builds) - [Notepad3 - Full Changelog](https://raw.githubusercontent.com/rizonesoft/Notepad3/master/Build/Changes.txt)
* Documentation - https://rizonesoft.com/documents/notepad3

## Rizonesoft Support

* **[GET IN TOUCH](https://rizonesoft.com/contact-us)**
* **Premium Support** - On Rizonesoft, support is free and we will assist you the best we can. Please be patient when contacting us; there are mainly volunteers working on Rizonesoft projects, and time is a precious commodity.

## Changes compared to Flo's official [Notepad2](https://www.flos-freeware.ch/notepad2.html) (made in [Notepad2-mod](https://xhmikosr.github.io/notepad2-mod/)):

* Code folding
* Support for bookmarks
* Option to mark all occurrences of a word
* Updated Scintilla component
* Word auto-completion
* Syntax highlighting support for AutoHotkey (AHK), AutoIt3, AviSynth, Bash, CMake, CoffeeScript, 
  Inno Setup, LaTeX, Lua, Markdown, NSIS, Ruby, Tcl, YAML and VHDL scripts.
* Improved support for NFO ANSI art
* Other various minor changes and tweaks

## Changes compared to the Notepad2-mod fork:

* Additional syntax highlighting support for Awk, D, golang, MATLAB
* Regular Expression search engine ([Oniguruma](https://github.com/kkos/oniguruma))
* New toolbar icons based on Yusuke Kamiyaman's Fugue Icons (purchased by [Rizonesoft](https://rizonesoft.com))
* Hyperlink hotspot highlighting (single-click Open in Browser (Ctrl) / Load in Editor (Alt)
* Syntax highlighting support for D Source Script, Go Source Script, JSON, Makefiles, MATLAB, Nim Source Code, Power Shell Script, Resource Script, Shell Script
* New program icon and other small cosmetic changes
* In-App support for AES-256 Rijndael encryption/decryption of files (incl. external command line tool for batch processing)
* Virtual space rectangular selection box (Alt-key down)
* High-DPI awareness, including high definition toolbar icons
* Undo/Redo preserves selection
* File history preserves caret position (optional) and remembers encoding of file
* Accelerated word navigation
* Preserve caret position of items in file history
* Count occurrences of a marked selection or word
* Count and mark occurrences of matching search/find expression
* Visual Studio style copy/paste current line (no selection)
* Insert GUIDs
* Dropped support for Windows XP
* Other various minor changes, tweaks, and bugfixes

## Supported Operating Systems:

* Windows 7, 8, 8.1, 10, and 11 (both 32-bit and 64-bit)

<hr/>

# References

Seen on Nsane Forums: [Notepad3 is an advanced text editor...](https://www.nsaneforums.com/topic/382910-guidereview-notepad3-is-an-advanced-text-editor-that-supports-many-programming-languages/), a review of **Notepad3** posted by the moderator [Karston](https://www.nsaneforums.com/profile/12756-karlston/) at [nsane.forums](https://www.nsaneforums.com/). 

**Notepad3's review**: **[Notepad3 is an advanced text editor that supports many programming languages](https://www.ghacks.net/2020/08/11/notepad3-is-an-advanced-text-editor-that-supports-many-programming-languages/)**.

<hr/>

# **Notepad3 Settings (Notepad3.ini)**


## **`[Notepad3]`**

This section can be used to redirect to a settings file which will be used by Notepad3.
If a non-elevated user is not allowed to write to the program directory of Notepad3.exe, 
the side-by-side Notepad3.ini can point to a place where the user is allowed to write their settings, 
for example: 

`Notepad3.ini=%APPDATA%\Rizonesoft\Notepad3\Notepad3.ini`

or a to have user-specific settings:

`Notepad3.ini=%WINDIR%\Notepad3-%USERNAME%.ini`


## **`[Settings]`**

These settings are read and written by Notepad3’s user interface.
For example, all Menu ? Settings will go here.

#### `SettingsVersion=5`

#### `Favorites=%APPDATA%\Rizonesoft\Notepad3\Favorites\`


## **`[Settings2]`**

This section offers some advanced Notepad3 program settings, and can only be edited manually. 
Press Ctrl+F7 to open the Notepad3 ini-file. Most changes only take effect upon restarting Notepad3.

#### `PreferredLanguageLocaleName=en-US`

The default value for the already supported languages is defined by the: “OS language setting”.
- The fallback is: “en-US”.

##### Available languages:

```
English/United States (en-US) (internal default)
Afrikaans/South Africa (af-ZA)
Belarusian/Belarus (be-BY)
German/Germany (de-DE)
Greek/Greece (el-GR)
English/United Kingdom (en-GB)
Spanish/Spain (es-ES)
French/France (fr-FR)
Hindi/India (hi-IN)
Hungarian/Hungary (hu-HU)
Indonesian/Indonesia (id-ID)
Italian/Italy (it-IT)
Japanese/Japan (ja-JP)
Korean/Korea (ko-KR)
Dutch/Netherlands (nl-NL)
Polish/Poland (pl-PL)
Portuguese/Brazil (pt-BR)
Portuguese/Portugal (pt-PT)
Russian/Russia (ru/RU)
Slovak/Slovakia (sk-SK)
Swedish/Sweden (sv-SE)
Turkish/Turkey (tr-TR)
Vietnamese/Vietnam (vi-VN)
Chinese Simplified/China (zh-CN)
Chinese Traditional/Taiwan (zh-TW)
```

#### `IMEInteraction=0`

#### `DateTimeFormat=`

- (-> (Locale dependent short format)

#### `DateTimeLongFormat=`

- (-> (Locale dependent long format)

Specify the short/long date and time formats. This is the format parameter passed to 
the `strftime()` function. 
Note that the locale will be set to English (because of the English Visual C++ Run-time 
Library used by Notepad3).  

#### `TimeStampRegEx=`

- (-> \$Date:[^\$]+\$) (Find-Pattern to Update Stamps)

#### `TimeStampFormat=`

- (-> \\$Date:[^\\$]+\\$ | $Date: %Y/%m/%d %H:%M:%S $
- (-> $Date: %s $) (Print format should fit to TimeStampRegEx)

This parameter is used as a regex pattern to match time-stamps which will be updated to 
current date-time by `Shift+F5`, e.g. `$Date: 2018/04/26 00:52:39 $`

- Default `DateTime` formats are:
    1. SHORT: `[Settings2] DateTimeFormat=` (empty) - Notepad3's language locale short '<time> <date>' format is used
    2. LONG:  `[Settings2] DateTimeLongFormat=` (empty) - Notepad3's language locale long '<time> <date>' format is used
    3. TIME_STAMP: `[Settings2] TimeStampFormat=` (empty) - "$Date: %s $" where '%s' is replaced by time/date in 
      `DateTimeFormat`. E.g. `[Settings2] TimeStampFormat=#TimeStamp=2020-07-21 16:02:23 #`
- All `DateTime` formats accept the [`strftime()`](https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strftime-wcsftime-strftime-l-wcsftime-l?view=vs-2019) format string.
  Addon: `TimeStampFormat` accepts '%s' (which is no valid `strftime()` formatting code) - a placeholder for a `DateTimeFormat` formatted current date/time string. (mixing of `strftime()` and '%s' is not allowed). 
- If you define your own `TimeStampFormat`, you should define the corresponding `TimeStampRegEx` regular expression pattern accordingly (for our example: `[Settings2] TimeStampRegEx="#TimeStamp=[^#]+#"`), so that` Update Timestamps` operation can find and update them correctly.
- Additional Menu Point: Insert Current Timestamp.

#### `DefaultDirectory=`

Specify the default directory for the open and save dialogs, used if no file is opened. 
- Path-names can be relative to the Notepad3 program directory.

#### `DefaultExtension=txt`

Specify the default extension for saved files (omit the leading dot, just like txt or html).

#### `DenyVirtualSpaceAccess=0`

#### `filebrowser.exe=minipath.exe`

Specify the path of an external program that is launched when pressing the Browse toolbar button. 
Defaults to `minipath.exe`, which is the file browser plugin. 

You can specify additional command line switches, and the file currently opened in Notepad3 will be appended as the last command line parameter. 

Note: Due to special treatment of quotes by the Win32 ini-file APIs, pathnames with spaces need to be quadruple-quoted (""path to/file.exe""), but only double-quoted if there's additional command line arguments ("path to/file.exe" /arg).

On the other hand, our preferred file browser is `minipath.exe` (Menu->File->Browse... Ctrl+M) + Toolbar-Button.

If you don't like it, you can configure e.g. 
- [Settings2] filebrowser.exe=explorer.exe (system's file explorer), or
- [Settings2] filebrowser.exe=Explorer++.exe (https://explorerplusplus.com/) (side-by-side Notepad3), or
- [Settings2] filebrowser.exe=Q-Dir_x64+.exe (https://www.softwareok.de/?seite=Freeware/Q-Dir/) (side-by-side Notepad3)

#### `grepWin.exe=grepWinNP3.exe`

We have integrated a powerful external tool called **grepWinNP3**. 
**grepWinNP3** is a search and replace tool which can use regular expressions to perform its job.
This allows you to perform much more powerful search and replace operations in files.

**grepWinNP3** can be launched:
- from "File --> Launch --> Search in Files"
- or from "Edit --> Search --> Search in Files"
- or simply with "`Ctrl+Shift+F`"

#### `FileCheckInterval=2000`

The interval (in milliseconds) to check for external modification of the currently opened file. 
- Defaults is 2000 msec.
- Min: 200[msec] - if equal or less, notify immediately.

#### `FileChangedIndicator=[@]`

#### `FileDeletedIndicator=[X]`

#### `FileDlgFilters=`

Specify filters for the open and save dialogs 
- (Example: `Text Files|*.txt;*.wtx;*.log;*.asc;*.doc;*.diz;*.nfo|All Files|*.*`).

#### `FileLoadWarningMB=4`

The size limit, in megabytes, to display a warning message for large files. 
- A value of 0 disables the warning.

#### `MultiFileArg=0`

Control if Notepad3 should allow multiple files on the command line (set to 1). 
The default behavior is to accept only a single file, without quoted spaces, like Windows Notepad (set to 0). 
The command line switches + and - can be used to override this setting on the fly, and the /z command-line switch has the same effect as the - switch.

#### `NoCGIGuess=0`

Set to 1 to disable simple language detection for cgi and fcgi files.

#### `NoCopyLineOnEmptySelection=0`

NoCopyLineOnEmptySelection=1 to avoid the copy line (`Ctrl+C`) on empty selection.

#### `NoCutLineOnEmptySelection=0`

NoCutLineOnEmptySelection=1 to avoid the cut line (`Ctrl+X`) on empty selection.

#### `NoFadeHidden=0`

Set to 1 to disable fading of hidden objects in file lists (such as Favorites, etc.).

#### `NoFileVariables=0`

Set to 1 to disable file variable parsing. 
Encoding tag parsing can be disabled in the Menu ? File ? Encoding ? Default dialog box.

Notepad3 can parse a few of the Emacs variables that can be used in source code files. 
The first 512 bytes of a file (and, if nothing is found, also the last 512 bytes) are checked for the following constructs (can be manually disabled in the ini-file, or the File, Encoding, Default dialog box, respectively):
```
coding: utf-8;
mode: python;
tab-width: 8;
c-basic-indent: 2;
indent-tabs-mode:  nil;
c-tab-always-indent: true;
fill-column: 64;
truncate-lines: false;
enable-local-variables: true;
```
`coding`: Serves as a file encoding tag. Details about using encoding tags are outlined in the 
Notepad2 Encoding Tutorial.      
`mode`: Indicates the syntax scheme to be used, and is either the name of a scheme, or a 
file name extension.
`tab-width`:       
`c-basic-indent`: Denote tab and indentation settings.      
`indent-tabs-mode`: Determines whether to insert tabs as spaces (nil, false or 0) 
or not (true or 1).      
`c-tab-always-indent`: Configures whether the tab key re-formats indenting white-space 
(true or 1) or not (nil, false or 0).      
`fill-column`: Sets the desired limit for long lines (but does not automatically display 
the visual marker).      
`truncate-lines`: Controls word wrap (enable: nil, false or 0; disable: true or 1).      
`enable-local-variables`: Disables file variable parsing (nil, false or 0), but keeps 
evaluating encoding tags.      

To bypass both file variable and encoding tag parsing, reload the file with Alt+F8. 
Adapt the settings mentioned above to permanently turn off file variables and encoding tags.

#### `NoHTMLGuess=0`

Set to 1 to disable simple HTML/XML detection for files without extensions.

#### `PortableMyDocs=1`

If set to 1, recent files and other path settings referring to the `My Documents` directory tree are stored relative to `My Documents`. 
This enhances USB stick portability between different versions of Windows, which are using different locations for `My Documents`. 
This setting has no effect if Notepad3.exe itself is located inside `My Documents` (or a sub-directory thereof). 
- The default is 1 (enabled) if `RelativeFileMRU` is enabled, and 0 (disabled) otherwise.

#### `OpacityLevel=75`

Opacity level (in %) of the Notepad3 window in transparent mode.

#### `FindReplaceOpacityLevel=50`

Opacity level (in %) of the Find/Replace window in transparent mode.

#### `RelativeFileMRU=1`

Set to 0 to disable recent files on the same drive or network share as Notepad3.exe being saved with relative path-names. 
- The default is 1 (enabled).

#### `ReuseWindow=0`

This items are managed by Notepad3. (`Menu->Settings->Window->Reuse Window  Ctrl+Shift+L`)
- If set, another started Notepad3 instance will try to give control to the currently opened Window and quit.

#### `SaveBlankNewFile=true`

New file (not exists on file system ("Untitled")) asking('true')/not asking('false') for file save if document contains any whitespace (blank/space, tab, line-break) character.

#### `SciFontQuality=3`

#### `SimpleIndentGuides=0`

Set to 1 to prevent indentation guides from jumping across empty lines.

#### `SingleFileInstance=1`

This items are managed by Notepad3.

#### `ShellAppUserModelID=Rizonesoft.Notepad3`

#### `ShellUseSystemMRU=1`

Application User Model IDs (AppUserModelIDs) are used extensively by the taskbar in Windows 7 and later systems to associate processes, files, and windows with a particular application. 
In some cases, it is sufficient to rely on the internal AppUserModelID assigned to a process by the system. 
However, an application that owns multiple processes or an application that is running in a host process might need to explicitly identify itself so that it can group its otherwise disparate windows under a single taskbar button and control the contents of that application's Jump List.

Most recently used (MRU) source lists are resident on the user's computer and contain information about source paths used in previous installations. 
This information can be used when prompting the user for a source path. 
Control system MRU, task-bar and jump list behavior. 
See Replacing Windows Notepad for detailed explanations.

#### `StickyWindowPosition=0`

This items are managed by Notepad3. 
- `Menu->View->Position->Sticky Window Position` (Will remember current window position on restart, instead of last closed position (save on exit))

#### `SubWrappedLineSelectOnMarginClick=false`

Set to `true` to revert to old selection behavior:
- 1 click on line-number margin selects the entire corresponding line
- 1 double click on line-number margin selects the entire line with all sub-lines.

#### `LaunchInstanceWndPosOffset=28`

#### `LaunchInstanceFullVisible=true`

#### `UseOldStyleBraceMatching=0`

UseOldStyleBraceMatching=1 to switch back to (not recommended) old style behavior

#### `WebTemplate1=https://google.com/search?q=%s`

#### `WebTmpl1MenuName=Open Web Action 1`

#### `WebTemplate2=https://en.wikipedia.org/w/index.php?search=%s`

#### `WebTmpl2MenuName=Open Web Action 2

#### `ExtendedWhiteSpaceChars=:`

Put in here all ASCII chars which will be word delimiters for "Accelerated Word Navigation".

#### `AutoCompleteWordCharSet=`

Is set automatically for CJK input languages (GetACP()). 

If you define your own character-set in AutoCompleteWordCharSet, Auto-Completion word list is limited to words composed of these chars only (case insensitive).)

#### `AutoCompleteFillUpChars=`

New configuration .ini-file: [Settings2] AutoCompleteFillUpChars=
To get the "Enter" completion behavior back, define: [Settings2] AutoCompleteFillUpChars=\r\n
I you like to allow more "fill-up" characters (accept completion item), just add them:
- e.g. [Settings2] AutoCompleteFillUpChars=\r\n[(. (will accept completion item & adds the char).

#### `LineCommentPostfixStrg=`

It will be appended/removed to the comment tag on line comment block toggle. 
If the string contains spaces, you have to double-quote it,
- e.g. [Settings2] LineCommentPostfixStrg=" " to add a space after the comment tag (origin and title of this feature request).

#### `UpdateDelayMarkAllOccurrences=50`

#### `CurrentLineHorizontalSlop=40`

#### `CurrentLineVerticalSlop=5`

#### `UndoTransactionTimeout=0`

- in [msec]

UndoTransactionTimeout=1 (will be clamped to 10msec min.) will separate nearly every keystroke as single undo action.
(UndoTransactionTimeout=0 will switch this timer OFF)

#### `AdministrationTool.exe=`  

This parameter is not used at the moment.

#### `DevDebugMode=0`

Encoding Detector information in Titlebar. This parameter is used to "debug" UCHARDET.

#### `AnalyzeReliableConfidenceLevel=90`

Confidence/Reliability level for reliability switch in encoding dialog.

#### `LocaleAnsiCodePageAnalysisBonus=33`

Bias/Bonus on top of Confidence/Reliability if current system's ANSI Code-Page is file encoding analysis result.
(This will push detection algorithm to like system's ANSI Code-Page more than other detection result)

#### `LexerSQLNumberSignAsComment=1`

The # (hash) is the start of a line comment in MySQL dialect.
But if this is confusing, it can be switched off by providing an option to Scintilla's SQL-Lexer
(set option: lexer.sql.numbersign.comment to 0 (zero)).

Unfortunately, in Notepad3, this can not be done by configuration, it can only be done hard coded.
- Ed.: The default is "OFF", it is set to "ON" explicitly in Notepad3 (hard coded) to preserve old behavior, 

#### `ExitOnESCSkipLevel=2`

The leveling of ESC behavior (msg-boxes -> selection -> exit) leads to following implementation:

New parameter "[Settings2] ExitOnESCSkipLevel = 2"
- Level 2 : ESC cancels every single state separately (the default).
- Level 1 : ESC cancels message-box and ignores Selection.
- Level 0 : ESC cancels all states and proceeds to Exit (if configured).

#### `ZoomTooltipTimeout=3200`

- in [msec]
- A value of zero (0) (or less than 100 msec) will disable the tooltip display.

#### `WrapAroundTooltipTimeout=2000`

- in [msec]
- A value of zero (0) (or less than 100 msec) will disable the tooltip display.

#### `LargeIconScalePrecent=150`

- `Screen/Display Scale Percent` threshold to switch to bigger file types icons (lexer style selections)

#### `DarkModeBkgColor=0x1F1F1F`

#### `DarkModeBtnFaceColor=0x333333`

#### `DarkModeTxtColor=0xEFEFEF`

#### `HyperlinkShellExURLWithApp=""`

- If not defined or empty the default behavior on `Ctrl+Click` URL is performed:
- The URL-String is sent to the OS via ShellExecute(), which will try open the URL using the registered protocol (e.g. http:// or file://) - in most cases the default browser resp. the file, if extension is known.
- If defined, e.g. "`D:\PortableApps\GoogleChromePortable\GoogleChromePortable.exe`", this application will be started on `Ctrl+Click`.

#### `HyperlinkShellExURLCmdLnArgs="${URL}"`

- (use ${URL} as place holder for clicked Hyperlink URL string)
- Defining the argument/parameter string for the above application (only taken into account if  `HyperlinkShellExURLWithApp` is defined). 
- If not defined, empty or set to "${URL}", the argument for the app will be the URL-String clicked.
You can specify more command line parameter for the app here. The token `${URL}` within the string will be replaced by the URL-String clicked. E.g. `HyperlinkShellExURLCmdLnArgs="--incognito "${URL}""` will start the Chrome-Browser (see `HyperlinkShellExURLWithApp`) in "incognito mode" trying to open the clicked URL.

#### `HyperlinkFileProtocolVerb=""`

- `ShellExecuteEx()::lpVerb (""=default, "edit", "explore", "find", "open", "print", "properties", "runas")`

#### `CodeFontPrefPrioList="Cascadia Code,Cascadia Mono,Cousine,Fira Code,Source Code Pro,Roboto Mono,DejaVu Sans Mono,Inconsolata,Consolas,Lucida Console"`

Configurable Fonts priority list for for "Common Base" Scheme.

#### `TextFontPrefPrioList="Cascadia Mono,Cousine,Roboto Mono,DejaVu Sans Mono,Inconsolata,Consolas,Lucida Console"`

Configurable Fonts priority list for "Text Files" Scheme.


## **`[Statusbar Settings]`**

This section provides the ability to set the number, order and width of columns, 
and the prefix text of the status bar fields.

#### `VisibleSections=0 1 15 14 2 4 5 6 7 8 9 10 11`  (internal default)

This parameter is used to define, which fields of the Status Bar should be visible. 
If used, this setting also defines the field ordering.
      
- Section  0 = Ln : Line number of Caret position / Number total of lines in the file
- Section  1 = Col : Column number of  Caret position / Limit for Long Line settings
- Section  2 = Sel : Number of characters selected
- Section  3 = Sb : Number of bytes (Bytes in [UTF-8]) selected
- Section  4 = SLn : Number of selected lines
- Section  5 = Occ : Number of Marked Occurrences 
- Section  6 = Size of file in [UTF-8] Mode
- Section  7 = Encoding Mode  (double click to open `Encoding F9` )
- Section  8 = EOL Mode (Toggle CR+LF, LF, CR)
- Section  9 = Toggle INS/OVR Mode
- Section 10 = Toggle STD/2ND Text Mode (Default Text or 2nd Default Text)
- Section 11 = Current Scheme  (double click to open `Select Scheme` )
- Section 12 = Character Count (per line from line start)
- Section 13 = Replaced Occurrences
- Section 14 = TinyExpr Evaluation
- Section 15 = Unicode point display (UTF-16 encoding) of current (caret pos) character.

#### `SectionPrefixes=Ln  ,Col  ,Sel  ,Sb  ,SLn  ,Occ  ,,,,,,,Ch  ,Repl  ,Eval  ,U+,`  (internal default)

This parameter is used to redefines the displayed Prefixes in the sections of the Status Bar
- A “,” (comma) is used as separator. Spaces are **NOT** ignored.

#### `SectionPostfixes=,,,,,,,,,,,,,,,,`  (internal default)

This parameter is used to redefines the displayed Postfixes in the sections of the Status Bar
- A “,” (comma) is used as separator. Spaces are **NOT** ignored.

#### `SectionWidthSpecs=30 20 20 20 20 20 20 0 0 0 0 0 0 0 20 24`  (internal default)

This parameter is used to define the relative width of each field of the Status Bar
- 0 = space optimized fit to text (dynamically adapted to width changes)
- -n (neg. val) = fixed width of section [pix] , longer text is truncated

Fine tuning: increase, decrease or modify the value of numbers,
- e.g.: `;;;;;;;;;;;;;;;;;  0  1  2  3  4  5 6 7 8 9 10 11 12 13 14 15`
- `SectionWidthSpecs=50 40 42 40 36 40 0 0 0 0 0 -10 40 40 -40 40`

#### `ZeroBasedColumnIndex=0`

This parameter is used to define start counting of column (`Col`) at 0 or 1.

#### `ZeroBasedCharacterCount=0`

This parameter is used to define start counting of characters (of current line) (`Ch`) at 0 or 1.


## **`[Toolbar Labels]`**

This section offers the possibility to display the name of the function to the right of its corresponding icon.

```
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


## **`[Window]`**

#### `<ResX>x<ResY> DefaultWindowPosition=`

This items are managed by Notepad3. (`Menu->View->Position->Save as Default Position`)
(Will set current window position as "Default Position" - can be recalled by `Ctrl+Shift+P` Hotkey)


<hr/>
