# Notepad3

[![Build status](https://img.shields.io/appveyor/ci/rizonesoft/notepad3/master.svg)](https://ci.appveyor.com/project/rizonesoft/notepad3/branch/master)
[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
[![](https://img.shields.io/badge/Donate-PayPal-blue.svg)](https://www.paypal.me/rizonesoft)

Notepad3 is a fast and light-weight Scintilla-based text editor with syntax highlighting. It has a small memory footprint, but is powerful enough to handle most programming jobs. [Download Notepad3 here](https://www.rizonesoft.com/downloads/notepad3).

> *Notepad3 is based on code from Florian Balmer's Notepad2 and XhmikosR's Notepad2-mod. MiniPath is based on code from Florian Balmer's metapath.*

## Important links!
* Notepad3/RC download page - https://www.rizonesoft.com/downloads/notepad3
* Latest changelog (release notes) - https://www.rizonesoft.com/downloads/notepad3/update
* Notepad3 changelog (all versions/builds) - [Notepad3 - Full Changelog](https://raw.githubusercontent.com/rizonesoft/Notepad3/master/Build/Changes.txt)
* Notepad3 Documentation - https://www.rizonesoft.com/documents/notepad3

## Rizonesoft Support

* **[GET IN TOUCH](https://www.rizonesoft.com/#contact)**
* **Premium Support** - On Rizonesoft, support is free and we will assist you the best we can. Please be patient when contacting us; there are mainly volunteers working on Rizonesoft projects and time is a precious commodity.

## Changes compared to Flo's official [Notepad2](http://www.flos-freeware.ch/notepad2.html) (made in [Notepad2-mod](https://xhmikosr.github.io/notepad2-mod/)):

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
* State of the art Regular Expression search engine ([Onigmu](https://github.com/k-takata/Onigmo))
* New toolbar icons based on Yusuke Kamiyaman's Fugue Icons (Purchased by [Rizonesoft](https://www.rizonesoft.com))
* Hyperlink Hotspot highlighting (single click Open in Browser (Ctrl) / Load in Editor (Alt)
* Syntax highlighting support for D Source Script, Go Source Script, JSON, Makefiles, MATLAB, Nim Source Code, Power Shell Script, Resource Script, Shell Script.
* New program icon and other small cosmetic changes
* In-App support for AES-256 Rijndael encryption/decryption of files (incl. external commandline tool for batch processing)
* Virtual Space rectangular selection box (Alt-Key down)
* High-DPI awareness, including high definition toolbar icons
* Undo/Redo preserves selection
* File History preserves Caret position (optional) and remembers encoding of file
* Accelerated word navigation
* Preserve caret position of items in file history
* Count occurrences of a marked selection or word
* Count and Mark occurrences of matching search/find expression
* Visual Studio style copy/paste current line (no selection)
* Insert GUIDs
* Dropped support for Windows XP version
* Other various minor changes, tweaks and bugfixes

## Supported Operating Systems:

* Windows 7, 8, 8.1 and 10 both 32-bit and 64-bit

<hr/>

# References

Seen on Nsane Forums: [Notepad3 is an advanced text editor...](https://www.nsaneforums.com/topic/382910-guidereview-notepad3-is-an-advanced-text-editor-that-supports-many-programming-languages/), a review of **Notepad3** posted by the moderator [Karston](https://www.nsaneforums.com/profile/12756-karlston/) at [nsane.forums](https://www.nsaneforums.com/). 

To be correct and complete, this **Notepad3's review** is written on 2020-08-11 by **[Ashwin](https://www.ghacks.net/author/ashwin/)** and posted on **[gHacks](https://www.ghacks.net/)**.

Original source:  **[Notepad3 is an advanced text editor that supports many programming languages](https://www.ghacks.net/2020/08/11/notepad3-is-an-advanced-text-editor-that-supports-many-programming-languages/)**.

<hr/>

# **Notepad3 Settings (Notepad3.ini)**


## **`[Notepad3]`**

This section can be used to redirect to a settings file which should be used by Notepad3.
If a non elevated user is not allowed to write to the program directory of Notepad3.exe, 
the side-by-side Notepad3.ini can point to a place, where the user is allowed to write his settings, 
for example : 

`Notepad3.ini=%APPDATA%\Rizonesoft\Notepad3\Notepad3.ini`

or a to have user specific settings:

`Notepad3.ini=%WINDIR%\Notepad3-%USERNAME%.ini`


## **`[Settings]`**

This settings are read and written by Notepad3’s user interface.
For examples all Menu ? Settings will go here.

#### `SettingsVersion=4`

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
English/United Kingdom (en-GB)
Spanish/Spain (es-ES)
Spanish/Mexico (es-MX)
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

#### `AutoReloadTimeout=2000`

The timeout (in milliseconds) to wait before automatically reloading modified files. 
- The default value of 2000 ms usually prevents read/write conflicts.

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

#### `DefaultWindowPosition=`

This items are managed by Notepad3. (`Menu->View->Position->Save as Default Position`)
(Will set current window position as "Default Position" - can be recalled by `Ctrl+Shift+P` Hotkey)


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

We have integrated of a Powerful External Tool called **grepWinNP3**. 
**grepWinNP3** is a simple search and replace tool which can use regular expressions to do its job. 

This allows to do much more powerful searches and replaces in Files.

**grepWinNP3** can be launched:
- from "File --> Launch --> Search in Files"
- or from "Edit --> Search --> Search in Files"
- or simply with "`Ctrl+Shift+F`"

#### `FileCheckInverval=2000`

The interval (in milliseconds) to check for external modification of the currently opened file. 
- Defaults is 2000 ms.

#### `FileDlgFilters=`

Specify filters for the open and save dialogs 
- (Example: `Text Files|*.txt;*.wtx;*.log;*.asc;*.doc;*.diz;*.nfo|All Files|*.*`).

#### `FileLoadWarningMB=64`

The size limit, in megabytes, to display a warning message for large files. 
- A value of 0 disables the warning.

#### `MultiFileArg=0`

Control if Notepad2 should allow multiple files on the command line (set to 1). 
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

#### `UseOldStyleBraceMatching=0`

UseOldStyleBraceMatching=1 to switch back to (not recommended) old style behavior

#### `WebTemplate1=https://google.com/search?q=%s`

#### `WebTemplate2=https://en.wikipedia.org/w/index.php?search=%s`

#### `ExtendedWhiteSpaceChars=:`

Put in here all ASCII chars which should be word delimiter in case of "Accelerated Word Navigation".

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

#### `AdministrationTool.exe=`  

This parameter is not used at the moment.

#### `DevDebugMode=0`

Encoding Detector information in Titlebar. This parameter is used to "debug"  UCHARDET

#### `AnalyzeReliableConfidenceLevel=92`

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

- A value of zero (0) (or less than 100 ms) will disable the Tooltip display.

#### `LargeIconScalePrecent=150`

- `Screen/Display Scale Percent` threshold to switch to bigger file types icons (lexer style selections)


## **`[Statusbar Settings]`**

This section provides the ability to set the number, order and width of columns, 
and the prefix text of the status bar fields.

#### `VisibleSections=0 1 12 14 2 4 5 6 7 8 9 10 11`  (internal default)

This parameter is used to define, which fields of the Status Bar should be visible. 
If used, this setting also defines the field ordering.
      
- Section  0 = Ln : Line number of Caret position / Number total of lines in the file
- Section  1 = Col : Column number of  Caret position / Limit for Long Line settings
- Section  2 = Sel : Number of characters selected
- Section  3 = Sb : Number of bytes (Bytes in [UTF-8]) selected
- Section  4 = SLn : Number of selected lines
- Section  5 =  Occ : Number of Marked Occurrences 
- Section  6 = Size of file in [UTF-8] Mode
- Section  7 = Encoding Mode  (double click to open `Encoding F9` )
- Section  8 = EOL Mode (Toggle CR+LF, LF, CR)
- Section  9 = Toggle INS/OVR Mode
- Section 10 = Toggle STD/2ND Text Mode (Default Text or 2nd Default Text)
- Section 11 = Current Scheme  (double click to open `Select Scheme` )
- Section 12 = Character Count (per line)
- Section 13 = Replaced Occurrences
- Section 14 = TinyExpr Evaluation

#### `SectionPrefixes=Ln  ,Col  ,Sel  ,Sb  ,SLn  ,Occ  ,,,,,,,Ch  ,Repl  ,Eval  ,`  (internal default)

This parameter is used to redefines the displayed Prefixes in the sections of the Status Bar
- A “,” (comma) is used as separator. Spaces are **NOT** ignored.

#### `SectionPostfixes=,,,,,,,,,,,,,,,`  (internal default)

This parameter is used to redefines the displayed Postfixes in the sections of the Status Bar
- A “,” (comma) is used as separator. Spaces are **NOT** ignored.

#### `SectionWidthSpecs=30 20 20 20 20 20 0 0 0 0 0 0 20 20 20`  (internal default)

This parameter is used to define the relative width of each field of the Status Bar
- 0 = space optimized fit to text (dynamically adapted to width changes)
- -n (neg. val) = fixed width of section [pix] , longer text is truncated

Fine tuning: increase, decrease or modify the value of numbers,
- e.g.: `;;;;;;;;;;;;;;;;;  0  1  2  3  4  5 6 7 8 9 10  11 12 13  14`
- `SectionWidthSpecs=50 40 42 40 36 40 0 0 0 0  0 -10 40 40 -40`

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
```

<hr/>
