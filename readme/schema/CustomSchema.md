# Notepad3 Schemas, Styles & Themes

This document describes how Notepad3's appearance customization works: what a *schema* (a.k.a. *lexer style set*) is, how styles are layered, how the style mini-language reads, how to export / import / switch themes, and where the user-editable files live.

Audience: advanced users who want to tweak colours and fonts beyond the built-in dialog, share a theme with others, or understand why a particular setting isn't sticking.

---

## Table of contents

1. [Core concepts](#1-core-concepts)
2. [Layered override model](#2-layered-override-model)
3. [The style mini-language](#3-the-style-mini-language)
4. [The default schemas (`Common Base` & `2nd Common Base`)](#4-the-default-schemas-common-base--2nd-common-base)
5. [File → schema matching](#5-file--schema-matching)
6. [The Customize Schemes dialog](#6-the-customize-schemes-dialog)
7. [Themes collection (`View → Themes`)](#7-themes-collection-view--themes)
8. [**Quick workflow: save your customizations as a reusable theme**](#8-quick-workflow-save-your-customizations-as-a-reusable-theme)
9. [Export / Import](#9-export--import)
10. [Anatomy of a style INI file](#10-anatomy-of-a-style-ini-file)
11. [Toolbar themes (separate from colour schemes)](#11-toolbar-themes-separate-from-colour-schemes)
12. [References in the code](#references-in-the-code)

---

## 1. Core concepts

| Term | Meaning |
|------|---------|
| **Schema** / **Scheme** | A complete set of styles for one language (e.g. *C/C++ Source Code*, *Python Script*). Backed in code by an `EDITLEXER` record plus an array of `EDITSTYLE` entries. |
| **Style** | A single styleable element inside a schema (e.g. *Default Style*, *Keywords*, *Comments*, *Strings*). Each style has a plain-text *style string* describing its font, size, colours, etc. |
| **Theme** | A complete snapshot of all custom colours, schemas and per-schema extensions, stored as a standalone `.ini` file (e.g. `Dark.ini`). Switching themes replaces every schema's style strings at once. |
| **Lexilla lexer** | The underlying syntax-highlighting engine for a schema. Notepad3 ships with ~60 lexers (ANSYS APDL, C++, Python, Markdown, Rust, YAML, …). |
| **Custom colours** | A 16-slot colour palette stored per-installation. Used by the *Customize Schemes* colour-picker and exported with every theme. |

Notepad3 ships with roughly 60 schemas; the exact list appears in the *Select Scheme* dialog (**Ctrl+F11**) and in the left-hand tree of *Customize Schemes* (**F12**).

---

## 2. Layered override model

Every style string you see in the editor is the result of a four-level resolution chain, applied each time a file is opened or a theme is switched:

```
┌─────────────────────────────────────────────────────────────┐
│ 4. Runtime edits (Customize Schemes dialog)                │  ← highest priority, in-memory only
├─────────────────────────────────────────────────────────────┤
│ 3. User INI — either Notepad3.ini [SchemaSection]           │
│    or the currently loaded theme file (themes/<Name>.ini)   │
├─────────────────────────────────────────────────────────────┤
│ 2. 'Common Base' / '2nd Common Base' defaults               │
│    (font, size, fg/bg for everything unstyled)              │
├─────────────────────────────────────────────────────────────┤
│ 1. Hard-coded lexer defaults (compiled into Notepad3.exe)   │  ← lowest priority
└─────────────────────────────────────────────────────────────┘
```

- **Lower levels fill gaps only.** If a user INI defines `fore:#ff0000` for a style but omits `font:`, the font comes from the Common Base, and ultimately from the hard-coded default.
- **Only deltas are saved.** When Notepad3 writes your styles back, it stores *only* values that differ from the merged defaults. An exported theme, by contrast, can force-save everything — see [§9 Export/Import](#9-export--import).
- **Empty lexer sections are removed.** After saving, any `[Python Script]`-style section that ended up empty (because the user reset everything to default) is stripped so the file stays readable.
- **Dark mode is its own level 2.** When Windows is in dark mode, the *Common Base* defaults come from a dark-mode palette instead of the light one. Your custom deltas stack on top of whichever palette is active.

### Where do my custom changes land on disk?

| Active theme | Changes written to |
|---|---|
| *Standard Config* (index 0) | `Notepad3.ini`, sections `[Styles]`, `[Custom Colors]`, and per-schema sections like `[C/C++ Source Code]`, `[Python Script]`, … |
| A named theme (e.g. *Dark*) | `<IniFileDir>\themes\Dark.ini` (full dump — all styles, not just deltas) |

Saving occurs only if `Settings.SaveSettings = 1` (Menu **Settings → Save Settings On Exit**) or you choose **Settings → Save Settings Now** (**F7**).

---

## 3. The style mini-language

Every `EDITSTYLE::szValue` is a **semicolon-delimited list of `attribute:value` tokens**. Order is irrelevant, keys are case-insensitive, and spacing around `:` and `;` is tolerated:

```
font:Consolas; size:11; fore:#D7BA7D; back:#1E1E1E; bold
```

Buffer limit: **255 characters** per style string (`BUFSIZE_STYLE_VALUE` in `src/StyleLexers/EditLexer.h`). Longer strings are silently truncated on load.

### 3.1 Attributes

| Token | Purpose | Examples |
|-------|---------|----------|
| `font:<name>` | Font face. Literal face names or the aliases `Default`, `$Code0…$Code9`, `$Text0…$Text9` which resolve to entries of `CodeFontPrefPrioList` / `TextFontPrefPrioList` (`Settings2`). | `font:Consolas`, `font:$Code0` |
| `size:<pt>` or `size:±<pt>` | Absolute (`11`) or relative (`+2`, `-1`) font size in points. Relative sizes cumulate across the inheritance chain. | `size:10`, `size:+1` |
| `fore:#RRGGBB` | Foreground (text) colour, hex. `#AARRGGBB` also accepted on styles that respect alpha. | `fore:#569CD6` |
| `back:#RRGGBB` | Background colour, hex. `#AARRGGBB` also accepted. | `back:#1E1E1E` |
| `bold` / `italic` / `underline` / `strikeout` | Plain flags, no value. | `italic` |
| `eolfilled` | Extends the background colour to the end of the line. | `eolfilled` |
| `case:u` \| `case:l` \| `case:m` | Force upper / lower / mixed case (visual only). | `case:u` |
| `charset:<n>` | LOGFONT charset id (0 = ANSI, 128 = Shift-JIS, 186 = Baltic, 204 = Russian, …). Clamped to ≥ 0. | `charset:204` |
| `smoothing:<q>` | Font quality. Tokens: `default`, `standard`, `aliased`, `antialiased`, `cleartype`. | `smoothing:cleartype` |
| `alpha:<0-255>` | Primary alpha (indicator / overlay opacity). | `alpha:40` |
| `alpha2:<0-255>` | Secondary alpha (outline for box-type indicators). | `alpha2:220` |
| `weight:<name>` | Font weight. Accepted words: `thin`, `extralight`, `light`, `book`, `regular`, `medium`, `semibold`, `bold`, `extrabold`, `heavy`, `extrablack`, plus a handful of synonyms. | `weight:semibold` |
| `stretch:<name>` | Font stretch: `ultracondensed`, `extracondensed`, `condensed`, `semicondensed`, `normal`, `semiexpanded`, `expanded`, `extraexpanded`, `ultraexpanded`. | `stretch:condensed` |
| `block` | Draw caret as a block. | `block` |
| `hotspot` | Mark the style as a hotspot (clickable region). | `hotspot` |
| `indic_<shape>` | Indicator drawing style (used by hotspot / brace / occurrence markers). Common values: `indic_plain`, `indic_squiggle`, `indic_tt`, `indic_diagonal`, `indic_strike`, `indic_box`, `indic_roundbox`, `indic_straightbox`, `indic_fullbox`, `indic_dash`, `indic_dots`, `indic_squigglelow`, `indic_squigglepixmap`, `indic_compositionthick`, `indic_compositionthin`, `indic_textfore`, `indic_point`, `indic_pointcharacter`, `indic_gradient`, `indic_gradientcentre`. | `indic_roundbox` |

### 3.2 Colour syntax

- Always `#` followed by six hex digits (`#RRGGBB`) — **not** `0xBBGGRR` as in the dark-mode INI keys.
- Named colours are **not** supported; use hex.
- Colours stored in the *Custom Colors* palette (the 16 slots at the top of any theme file) are written as `#RRGGBB` strings with keys `01`…`16` and are just a convenience shared with the Windows colour-picker dialog — they are not referenced from style strings.

### 3.3 Example — styling comments in C++

```
Comment=italic; fore:#608B4E; back:#1E1E1E
```

- Italic, no explicit font (inherits Common Base), explicit fg and bg.
- Key (`Comment`) matches a fixed slot inside `styleLexCPP.c`. You cannot invent new style names; you edit the ones the lexer defines.

---

## 4. The default schemas (`Common Base` & `2nd Common Base`)

Two "super-schemas" act as the global fallback for *every* language-specific schema:

| Schema | INI section | Role |
|--------|-------------|------|
| `Common Base` | `[Common Base]` | Primary defaults — applied to every style that does not explicitly set an attribute. |
| `2nd Common Base` | `[2nd Common Base]` | Alternate set, activated by **Menu → View → Use 2nd Default Style** (`IDM_VIEW_USE2NDDEFAULT = 41002`). |

Both schemas also contain the universal editor chrome styles (they are not language specific):

- `Default Style` — base font, size, fg, bg.
- `Margins and Line Numbers` — line-number gutter.
- `Matching Braces (Indicator)` / `Matching Braces Error (Indicator)` — brace highlight.
- `Control Characters` — the `[NUL]`, `[BS]` etc. glyphs.
- `Indent Guide` — vertical dotted lines.
- `Selection` — selection highlight (foreground/background, with alpha).
- `Whitespace` — dot/arrow/EOL glyphs.
- `Current Line` — background highlight of the line containing the caret.
- `Caret` — colour + width, `block` modifier.
- `Long Line Marker` — visual for the right-margin line.
- `Bookmark` / `Folding` colours.
- `Mark Occurrences` — the highlight colour of *Mark All Occurrences*.
- `Hotspot Style` / `Unicode Hotspot` / `Multi-Edit` — interactive overlays.
- `Change History` markers (modified/saved/unsaved).
- `IME Input` colour.
- `Invisible` / `Read-only` overlays.

### Switching with `Use 2nd Default Style`

The status bar shows **STD** or **2ND** in the INS/OVR field. Double-click there (or use the menu) to toggle. The active state is saved as:

```ini
[Styles]
Use2ndDefaultStyle=1
```

Both base schemas coexist in the INI: toggling only flips which one Notepad3 reads.

---

## 5. File → schema matching

When you open a file, Notepad3 runs a pipeline to pick the right schema. It stops at the first match.

1. **File variables** (`vim:` / `emacs:` modelines inside the file). If the file declares `mode: python;` or `-*- mode: cpp -*-`, the declared mode name is matched against schema names (case-insensitive prefix) and then against extension lists. Disabled by `Settings2.NoFileVariables=1`.
2. **Shebang detection** for `.cgi` / `.fcgi` files or files flagged as CGI by mode. Recognised interpreters: `python`, `ruby`, `bash`/`sh`, `perl`, `tcl`, `node`/`js`, `php`. Disabled by `Settings2.NoCGIGuess=1`.
3. **Wildcard / filename match.** Any entry in a schema's extension list that contains `*`, `?`, or `.` is matched against the bare filename via the standard Win32 wildcard matcher — e.g. `Makefile*` matches `Makefile`, `Makefile.dev`, `cmakelists.txt` matches the literal filename. See the [Extension lists](#extension-lists) subsection below for syntax and worked examples.
4. **Plain extension match.** First lexer whose extension list contains the file's extension wins. Extension comparison is case-insensitive; separator is `;` (semicolon or space both work in practice).
5. **HTML/XML sniff** — if the first bytes start with `<`, classify as HTML or XML. Disabled by `Settings2.NoHTMLGuess=1`.
6. **Shebang fallback** for extension-less files (same recognisers as step 2).
7. **ANSI-art fallback** — DOS/OEM-encoded files fall back to the ANSI Art lexer.
8. **Default schema.** Whatever is set as *Default Scheme* in the *Select Scheme* dialog (default: *Pure Text Files*).

### The two toggles in the *Select Scheme* dialog

| Checkbox | INI key | Meaning |
|---|---|---|
| **Set as default** | `[Styles] DefaultScheme=<n>` | The schema used at startup and when nothing else matches. |
| **Automatic detection** | `[Styles] AutoSelect=0\|1` | When off, steps 3–7 above are skipped. File → schema association is based solely on extension. |

### Extension lists

Each schema carries a hard-coded default extension list (compiled into `Notepad3.exe`) plus an optional user override stored in the schema's INI section:

```ini
[Python Script]
FileNameExtensions=py;pyw;pyi;setup.py;test_*.py
```

The override **replaces** the default; the two are not merged. The field accepts both plain file extensions and filename wildcard patterns in a single semicolon-separated list.

#### Entry classification

Each entry is one of two kinds — Notepad3 picks based on what characters it contains:

| Entry contains… | Treated as… | Matched against |
|---|---|---|
| only letters / digits (no `*`, `?`, `.`) | plain extension token | the file's extension (the part after the last `.`, case-insensitive) |
| any of `*`, `?`, or `.` | filename wildcard pattern | the bare filename via Win32 `PathMatchSpecW` |

Examples:

| Entry | Kind | Matches |
|---|---|---|
| `py` | extension | `foo.py`, `foo.PY` |
| `*.py` | wildcard | any file ending in `.py` (same as `py`, just spelled out) |
| `Makefile*` | wildcard | `Makefile`, `Makefile.dev`, `Makefile_old` |
| `CMakeLists.txt` | wildcard (literal) | exactly `CMakeLists.txt` |
| `??.log` | wildcard | any two-character name + `.log` |
| `Makefile` | extension | files with extension `Makefile` (rare; not the literal extensionless filename) |

#### Syntax rules

| Aspect | Rule |
|---|---|
| Separator | `;` (semicolon) is canonical. A single space also works, and whitespace around `;` is tolerated. |
| Case | Both passes are **case-insensitive**. |
| `*` in a wildcard | Matches zero or more of any character. |
| `?` in a wildcard | Matches exactly one character. |
| `.` in a wildcard | Matches a literal dot — there is no escape character. |
| Buffer limit | **512 characters** per schema (counting all entries combined). Longer values are silently truncated when read from the INI; a debug-output warning is emitted. |

#### Worked examples

```ini
; Plain extensions (the common case)
[Python Script]
FileNameExtensions=py;pyw;pyi

; Match the literal extensionless file CMakeLists, plus its .txt sibling
[CMake]
FileNameExtensions=cmake;ctest;cmakelists.txt;CMakeLists*

; Dockerfile family — Dockerfile, Dockerfile.dev, Dockerfile.prod, …
[Docker]
FileNameExtensions=dockerfile;Dockerfile*

; Shell config files starting with a dot
[Bash Script]
FileNameExtensions=sh;bash;.bashrc;.bash_profile;.profile

; Route Python test files to a separate schema
[Python Test]
FileNameExtensions=test_*.py;*_test.py

; Single-character pattern: any 1-char-name file ending in .x
[Custom]
FileNameExtensions=?.x
```

Common pitfalls:

- `Makefile` (no `*`, `?`, `.`) is treated as a **plain extension**, so it matches files whose extension is `Makefile` (i.e. `foo.Makefile`) — **not** the literal extensionless `Makefile`. To catch the latter, use `Makefile*`.
- `Makefile*` is greedy — it also matches `Makefile_old` or `Makefilexxx`. If you need exactness, list each name explicitly (`Makefile;Makefile.dev`) or rely on the `.` form (`Makefile.*`) which still matches `Makefile.foo` but excludes `Makefilexxx`.
- `setup.py` (entry contains `.`) is a wildcard literal — it only matches the literal filename `setup.py`, not `mysetup.py`.

#### Precedence

Within `Style_SetLexerFromFile()` the auto-detect pipeline tries the **wildcard / filename pass across all schemas first**, and only then falls back to the plain-extension pass across all schemas. Within a single schema the two passes are independent: an entry's classification (plain vs wildcard) decides which pass it participates in.

When two schemas claim the same plain extension, or both match a file with their wildcards, the schema that appears first in the internal schema array wins. There is currently no UI to reorder schemas.

#### Legacy `\regex` syntax

Earlier versions of Notepad3 used a PCRE2 regex syntax for filename matching, marked by a leading backslash (e.g. `\^Makefile$`). This has been **removed**. Existing INIs migrate automatically on load:

- Patterns of the form `\^<name>$` where `<name>` is alphanumeric (with optional `_`, `-`, and `\.` escapes) are **translated** to a wildcard equivalent. Examples: `\^Makefile$` → `Makefile*`, `\^cmakelists\.txt$` → `cmakelists.txt`, `\^\.bashrc$` → `.bashrc`.
- Anything more complex (real regex metacharacters like `.+`, `[abc]`, `?`, missing anchors, etc.) is **dropped** — translation isn't safe.

In both cases the in-memory list is rewritten on the next save, so user INIs converge to wildcard-only over time. A debug-output line records the migration per schema. Hand-tuned entries that get dropped may need to be rewritten as wildcard patterns.

#### Editing in *Customize Schemes*

Select a schema node in the left-hand tree and edit the *FileNameExtensions* field. On save:

- **Empty field** (or whitespace only) → the compiled-in default is restored.
- **Field equals the default** → the INI key is removed entirely so `Notepad3.ini` stays clean.
- **Field differs from default** → the full value is written back to the schema's section.

Persistence still requires *Save Settings On Exit* or *Settings → Save Settings Now* (**F7**); see [§2](#2-layered-override-model).

---

## 6. The Customize Schemes dialog

- Launch: **Menu → View → Customize Schemes…** (`IDM_VIEW_SCHEMECONFIG = 41003`) or **F12**.
- Modeless — you can keep it open while editing files and see live preview.

### Tree

Every schema appears in the left-hand tree. Expand a schema to see its style slots. Selecting:

- **a schema node** lets you edit the *FileNameExtensions* field for that schema.
- **a style node** lets you edit the raw style string plus helpers for font, foreground, and background.

### Editing aids

| Control | Role |
|---|---|
| *Style* text field | Free-form style string — full mini-language from [§3](#3-the-style-mini-language). |
| *Font…* button | Opens the common ChooseFont dialog and writes `font:`, `size:`, `weight:`, `italic` back into the text field. |
| *Foreground* / *Background* | Colour pickers; write `fore:#…` / `back:#…` tokens. |
| *Default* | Resets the current style to its compiled-in default (discards the INI override for that style only). |
| *Preview* | Toggles live preview of your edits in the main editor. |
| *< Prev / Next >* | Step through the styles of the current schema. |
| *Import…* | See [§9](#9-export--import). |
| *Export…* | See [§9](#9-export--import). |
| *Dark Mode Highlight Contrast* (slider/field) | Adjusts how strongly foreground colours are re-mapped for legibility against the dark background. Stored as `Settings.DarkModeHiglightContrast`. |

### Cancel vs OK

Clicking **Cancel** reverts every style string that was modified during this dialog session (a backup is taken on open). Closing the window via the title-bar X behaves the same as Cancel.

### Drag-and-drop

Dragging a *style* node onto another *style* node copies the dragged style's string into the target. Dragging a *schema* node is blocked (the cursor changes to the *no-drop* glyph). Drag-and-drop is therefore a quick "use Comments' look for Documentation-Comments" shortcut — not a reordering tool.

---

## 7. Themes collection (`View → Themes`)

A theme is a standalone INI file holding a snapshot of every schema's styles. Themes live in:

```
<IniFileDir>\themes\*.ini
```

where `<IniFileDir>` is the directory that contains the active `Notepad3.ini`.

### Shipped themes

The release package includes three reference themes under `Build/themes/`:

- `Dark.ini`
- `Obsidian.ini`
- `Sombra.ini`

Copy any of them next to your `Notepad3.ini` (inside a `themes` folder) and they will appear in **View → Themes** on next launch.

### Menu layout (`View → Themes`)

```
  Factory Reset          ← reloads compiled-in defaults, discards INI/theme
  --------------
  ● Standard Config      ← Notepad3.ini itself (always present, always first)
    Dark
    Obsidian
    Sombra
    <your-theme>
    …
```

- **Factory Reset** (`IDM_THEMES_FACTORY_RESET = 37001`) discards the current session's style overrides and reloads the compiled-in defaults. It does *not* delete your INI file, but if you save afterwards, the deltas will be rewritten.
- **Standard Config** (`IDM_THEMES_STD_CFG = 37002`) is the internal name for "styles in Notepad3.ini".
- Each theme file adds a menu item with consecutive IDs (`STD_CFG + 1`, `+2`, …).

### Capacity

The theme table is allocated with **25 slots** total — slot 0 for *Standard Config*, slots 1-24 for theme files. If more than 24 `.ini` files exist in `themes\`, the extras are silently ignored (the ones scanned first by `FindFirstFile` win).

### Switching themes

Selecting a theme from the menu triggers this sequence:

1. If **Save Settings On Exit** is on, save the *current* theme first (to `Notepad3.ini` for Standard Config, to the theme file otherwise — with `bForceAll = true`, i.e. every style written in full).
2. Reset the INI file cache.
3. Load the new theme's `.ini` (or the compiled-in defaults for **Factory Reset**).
4. Update `Settings.CurrentThemeName` and tick the chosen menu radio.

`Settings.CurrentThemeName` persists the choice across restarts. If the file disappears before the next launch, Notepad3 falls back to *Standard Config*.

### Creating a new theme

Easiest flow:

1. Get your styles looking the way you want (Customize Schemes, regular save).
2. **View → Customize Schemes… → Export…**, save as `<IniFileDir>\themes\MyTheme.ini` (see [§9](#9-export--import)).
3. Restart Notepad3. **View → Themes → MyTheme** now appears.

For a fuller, step-by-step walkthrough aimed at first-time users, see the next section.

---

## 8. Quick workflow: save your customizations as a reusable theme

> **Read this first if you just want to tweak Notepad3's colours, keep the result, and maybe share it.**
> The other sections are reference material — this one is the recipe.

**The idea in one sentence:** any colour/font tweak you make is stored in `Notepad3.ini`. If you also want it as a named, switchable theme that shows up under **View → Themes** (and that you can copy to another PC), export it once into a folder called `themes\` next to your `Notepad3.ini`.

### Best practice: work on a copy, not on the original

Before you start customizing, make a **named theme you can edit freely**. That keeps your starting point (your current `Notepad3.ini`, or a shipped theme like *Dark*) safe — if you ever want to revert, you just switch back to it in **View → Themes**. Two equally good ways to set up that copy:

1. **Copy an existing theme file.** Pick a starting point — e.g. `Dark.ini` from the release package (`Build\themes\`), or any theme already in your own `themes\` folder. Make a copy of that file inside `<folder containing Notepad3.ini>\themes\` and **rename the copy** (e.g. `MyDark.ini`). Restart Notepad3, then switch to **View → Themes → MyDark**. All your edits now land in `MyDark.ini`; the original is untouched.
2. **Export "Standard Config" under a new name.** This is the step-by-step below: it produces a fresh theme file that mirrors your current look exactly, but is now a separate file you can change freely without touching `Notepad3.ini`.

> **The golden rule afterwards:** switch to your new theme **first**, *then* customize. *Save Settings* (**F7**) always writes to whichever theme is currently active — so as long as your theme has the radio dot in **View → Themes**, every edit lands in *your* file and not in the original.

### Step-by-step

1. **Customize.** Open **View → Customize Schemes…** (or press **F12**). On the left, expand a language (e.g. *C/C++ Source Code*). Click a style row (e.g. *Comment*). Use the **Font…**, **Foreground**, and **Background** buttons — or edit the text field directly — until the preview looks right. Repeat for whichever languages and styles you want to change. Click **OK** when done.
2. **Save to disk.** Press **F7** (same as **Settings → Save Settings Now**). Alternatively, tick **Settings → Save Settings On Exit** so Notepad3 saves automatically when you close it. Your tweaks now live in `Notepad3.ini`.
3. **Export as a theme file.** Reopen **View → Customize Schemes…** (**F12**) and click the **Export…** button at the bottom left. The file dialog **opens directly inside the `themes\` folder beside your `Notepad3.ini`** (Notepad3 creates that folder on the spot if it doesn't exist yet — no manual setup needed). Type a filename, e.g. `MyTheme.ini`, and save.
4. **Restart Notepad3.** Open the **View → Themes** menu — your theme now appears as **MyTheme**. Click it to switch.

That's the whole workflow. The rest of this section just shows you where things go and how to share them.

### Where exactly is "the themes folder"?

It is a folder named `themes` placed **right next to your active `Notepad3.ini`**:

```text
<folder containing your Notepad3.ini>\
├── Notepad3.ini
└── themes\
    ├── Dark.ini
    ├── Obsidian.ini
    ├── Sombra.ini
    └── MyTheme.ini      ← your exported file goes here
```

- If you don't know where your `Notepad3.ini` lives, open Notepad3 and use **File → Launch → Notepad3.ini** (or check **Help → About** for paths).
- The `themes\` folder is created automatically the first time you click **Export…** in *Customize Schemes*. You can also create it by hand and drop theme files into it — Notepad3 picks them up on the next launch.
- The three reference themes (`Dark.ini`, `Obsidian.ini`, `Sombra.ini`) ship inside the release package under `Build\themes\`. Copy them into your own `themes\` folder if you want them in the **View → Themes** menu too.

### Sharing your theme with someone else

An exported `.ini` is **self-contained** — it carries every style it needs. To share:

1. Send the recipient your `MyTheme.ini` file (email, USB stick, cloud share, whatever).
2. Tell them to drop it into their own `<folder containing Notepad3.ini>\themes\` folder.
3. They restart Notepad3 and pick **View → Themes → MyTheme**.

No other setup is required on their side.

### Switching back and forth

Once your theme is installed, **View → Themes** gives you three useful options:

| Menu item | What it does |
|---|---|
| **Standard Config** | Your styles as stored in `Notepad3.ini` (your "everyday" look). |
| **MyTheme** (or any named theme) | Apply the chosen theme file from `themes\`. |
| **Factory Reset** | Go back to Notepad3's built-in defaults. Does *not* delete your INI or theme files. |

### Tweaking your theme later

You don't need to re-export every time you change something:

1. **View → Themes → MyTheme** (make sure it's the active theme — the radio mark is next to it).
2. Open **Customize Schemes…** (**F12**), make your changes, click **OK**.
3. Press **F7**. Notepad3 writes the changes straight back into `themes\MyTheme.ini`.

> **Tip — make a backup before big experiments.** Before doing a large round of edits, copy `MyTheme.ini` somewhere safe (e.g. `MyTheme.bak.ini`). If you don't like the result you can simply replace the file and restart.

For the formal mechanics of Export / Import (what exactly is written, how Import differs from a theme switch, etc.), continue with [§9 Export / Import](#9-export--import).

---

## 9. Export / Import

Both actions are in the *Customize Schemes* dialog (**F12**), lower left.

### Export

- File dialog filter: `*.ini` only.
- **Default location** is the `themes\` folder beside your active `Notepad3.ini` — i.e. the same folder **View → Themes** scans for theme files. Notepad3 creates that folder on demand if it does not yet exist, so the first export "just works". If the folder is read-only or otherwise un-writable, the dialog still opens there — no silent fallback to a different directory — and you can navigate elsewhere manually before saving.
- Written with `bForceAll = true` — **every** style is written out in full, regardless of whether it equals the default. Exported files are therefore self-contained and can be shared with anyone.
- Sections written:
  - `[Custom Colors]` — 16 palette slots.
  - `[Styles]` — global settings: `Use2ndDefaultStyle`, `DefaultScheme`, `AutoSelect`, `SelectDlgSizeX`, `SelectDlgSizeY`.
  - `[Common Base]`, `[2nd Common Base]` — super-schemas.
  - One section per schema (~60 sections).
  - `FileNameExtensions` keys inside each schema section.

### Import

- File dialog filter: `*.ini`.
- **Default location** is the same `themes\` folder as Export (created on demand). Same no-fallback rule applies — you can navigate elsewhere from the dialog if you want to import an `.ini` that lives outside `themes\`.
- Loads the target file into the cache and re-reads style strings out of it. Existing in-memory styles are replaced slot-by-slot for keys present in the imported file; keys missing from the imported file keep their current value.
- Extensions are imported too — **your previous per-schema extension customisations will be overwritten** by whatever is in the imported file.
- Import does *not* persist by itself. Until you save settings (or a later auto-save fires), re-starting Notepad3 reverts to the previously persisted state. Hence the usual recipe: *Import → verify → Save Settings Now* (**F7**).

### Comparison table

| Operation | Scope | What it writes | Uses `bForceAll` |
|---|---|---|---|
| *Save Settings Now* (F7), theme = Standard Config | `Notepad3.ini` | Only deltas from defaults | ✘ |
| *Save Settings Now* (F7), theme = named theme | `themes/<Name>.ini` | Only deltas from defaults | ✘ |
| **View → Themes → <switch>** with SaveSettings on, leaving a *named* theme | `themes/<oldName>.ini` | Every style | ✔ |
| **View → Themes → <switch>** with SaveSettings on, leaving *Standard Config* | `Notepad3.ini` | Only deltas | ✘ |
| *Customize Schemes → Export…* | user-chosen `.ini` | Every style (self-contained) | ✔ |

---

## 10. Anatomy of a style INI file

Whether it is your `Notepad3.ini` or a shipped theme like `Dark.ini`, the schema-related sections follow the same layout:

```ini
[Custom Colors]
01=#252526
02=#1346CE
03=#A2C5D4
; … up to 16 slots

[Styles]
Use2ndDefaultStyle=0
DefaultScheme=2
AutoSelect=1
SelectDlgSizeX=0
SelectDlgSizeY=0

[Common Base]
Default Style=font:$Code0; size:11; fore:#DEDEDE; back:#1E1E1E
Margins and Line Numbers=font:Consolas; size:-2; fore:#A4FFFF; back:#2B2B2B
Matching Braces (Indicator)=fore:#00FF00; alpha:40; alpha2:220; indic_roundbox
Selection=fore:#000000; back:#3F84DB; alpha:50; alpha2:150; eolfilled
Current Line=back:#323232; alpha:60; alpha2:255
Caret=fore:#FFFFFF; block
; …

[2nd Common Base]
2nd Default Style=font:$Code1; size:10; fore:#DCDCDC; back:#000000
; …

[Text Files]
Default Style=font:$Text0; size:11; fore:#E0E0E0
FileNameExtensions=txt;text;log;asc;doc;diz;nfo

[C/C++ Source Code]
Default=fore:#D7D7D7
Comment=italic; fore:#608B4E
Keyword=bold; fore:#569CD6
Type Keyword=fore:#4EC9B0
String=fore:#CE9178
Number=fore:#B5CEA8
Operator=fore:#B4B4B4
Preprocessor=fore:#C586C0
FileNameExtensions=c;cc;cpp;cxx;h;hh;hpp;hxx;inl

; … one section per schema …
```

Observations:

- **Section names** (`[C/C++ Source Code]`, `[Python Script]`, …) are the schema display names as used internally; they are stable across versions but are localised copies of the names shown in the UI.
- **Style keys** inside a section (`Default`, `Comment`, `Keyword`, `Preprocessor`, …) are fixed per schema — you cannot add new ones.
- **Missing keys** fall through to the *Common Base* → hard-coded default chain.
- **`FileNameExtensions`** appears at most once per schema. The list is semicolon-separated; entries beginning with `\` are regexes matched against the full filename.
- **Custom Colors entries** are just hex colours; they populate the 16-slot picker, nothing more.

> **Encoding.** INI files are written in UTF-8. A UTF-8 BOM is tolerated on read. Do not hand-save as UTF-16 — Notepad3's INI parser is 8-bit.

---

## 11. Toolbar themes (separate from colour schemes)

The directories under the project's `themes/` folder (`Flat`, `b&w`, `professional`, `std_scaled`, `c6248`) are **toolbar bitmap themes**, not style themes. They contain `.bmp` files (`Toolbar.bmp`, `ToolbarDisabled.bmp`, `ToolbarHot.bmp`, plus 24- and 48-pixel variants).

The toolbar bitmap in use is chosen via the `[Toolbar Images]` section in `Notepad3.ini` — see [Configuration.md](../config/Configuration.md) for details. This mechanism is entirely independent of the schema/theme system described above.

---

## References in the code

For developers wanting to look up behaviour described above:

| Topic | Primary file(s) |
|---|---|
| Data structures (`EDITLEXER`, `EDITSTYLE`, `KEYWORDLIST`) | `src/StyleLexers/EditLexer.h` |
| Schema array, loading, saving, layering | `src/Styles.c` — `g_pLexArray[]`, `_ReadFromIniCache()`, `Style_ToIniSection()`, `Style_CanonicalSectionToIniCache()`, `Style_ExportToFile()` |
| Auto-detection pipeline | `src/Styles.c` — `Style_SetLexerFromFile()`, `Style_MatchLexer()`, `Style_WildcardMatchLexer()`, `Style_SniffShebang()` |
| Customize Schemes dialog | `src/Styles.c` — `Style_CustomizeSchemesDlg()`, `Style_CustomizeSchemesDlgProc()` |
| Select Scheme dialog | `src/Styles.c` — `Style_SelectLexerDlg()`, `Style_SelectLexerDlgProc()` |
| Theme menu & switching | `src/Styles.c` — `_FillThemesMenuTable()`, `Style_InsertThemesMenu()`, `Style_DynamicThemesMenuCmd()`, `Style_ImportTheme()` |
| Menu command IDs | `language/common_res.h` — `IDM_VIEW_SCHEME`, `IDM_VIEW_USE2NDDEFAULT`, `IDM_VIEW_SCHEMECONFIG`, `IDM_THEMES_STD_CFG`, `IDM_THEMES_FACTORY_RESET` |
| Shipped themes | `Build/themes/Dark.ini`, `Obsidian.ini`, `Sombra.ini` |
