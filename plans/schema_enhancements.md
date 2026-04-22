# Schema / Style / Theme System — Enhancements & Fixes

> Companion plan to the user-facing reference at
> [`readme/schema/CustomSchema.md`](../readme/schema/CustomSchema.md).
> Use this document to scope a future Claude Code session focused on the
> schema/style subsystem.

## 1. Background

Notepad3's schema system (the `EDITLEXER` array, per-schema `EDITSTYLE` slots,
the *Common Base* / *2nd Common Base* super-schemas, and the `themes\*.ini`
collection) is stable and user-visible, but has accumulated small rough
edges over time. This plan consolidates:

- issues surfaced while writing the user documentation,
- follow-up ideas marked as `//~` in the code,
- UX gaps reported during the review,
- plus a few additional findings that emerged from cross-checking the
  code paths.

Nothing here is a correctness bug blocking release. All items are
low-to-moderate effort; several are trivial single-call-site fixes.

## 2. Key source locations

| File | What lives there |
|------|------------------|
| `src/Styles.c` | `g_pLexArray[]`, load/save, mini-DSL parsers, Customize / Select / Themes dialogs, Import / Export |
| `src/StyleLexers/EditLexer.h` | `EDITLEXER`, `EDITSTYLE`, `KEYWORDLIST`, `BUFSIZE_STYLE_VALUE = 256` |
| `src/TypeDefs.h` | `STYLE_EXTENTIONS_BUFFER = 512`, `EXTENTIONS_FILTER_BUFFER = 1024` |
| `src/StyleLexers/*.c` | 60+ per-language lexer definitions |
| `src/Config/Config.cpp` | Settings → theme name persistence (`Settings.CurrentThemeName`) |
| `language/common_res.h` | Menu IDs: `IDM_VIEW_SCHEME=41001`, `IDM_VIEW_USE2NDDEFAULT=41002`, `IDM_VIEW_SCHEMECONFIG=41003`, `IDM_THEMES_FACTORY_RESET=37001`, `IDM_THEMES_STD_CFG=37002` |
| `Build/themes/Dark.ini`, `Obsidian.ini`, `Sombra.ini` | Shipped themes |

## 3. Inventory of issues

| # | Severity | Area | Issue |
|---|----------|------|-------|
| I-01 | Medium | Buffers | 255-char style-string cap; silent truncation on load |
| I-02 | Low | Buffers | 511-char `FileNameExtensions` cap; silent truncation |
| I-03 | Medium | Themes | 24-theme hard cap (fixed-size `Theme_Files[25]`); silently drops extras |
| I-04 | Medium | UX | No "Reset this whole schema" button — user must edit INI by hand or factory-reset everything |
| I-05 | Medium | UX | Import overwrites `FileNameExtensions` — users lose extension tweaks when switching themes |
| I-06 | Low | UX | No mid-session revert: `pStylesBackup` is only taken on dialog open |
| I-07 | Low | UX | No "import from URL" and no way to distribute themes via anything but a file copy |
| I-08 | Low | Round-trip | `smoothing:` token parsed on load but never written back by the dialog |
| I-09 | Low | Round-trip | `//~` dead code around `Style_StrGetAlpha` for Whitespace background layer (alpha2 never exposed) |
| I-10 | Low | Round-trip | `SelectDlgSizeX/Y` travels inside `[Styles]` and leaks into every exported theme |
| I-11 | Medium | Matching | First-in-array wins for extension conflicts — no per-user priority mechanism |
| I-12 | Low | Matching | `\regex` extension syntax is entirely undocumented in-UI |
| I-13 | Low | Validation | Silent load of malformed theme files — no minimum-viable-theme check |
| I-14 | Low | Validation | Stale `Settings.CurrentThemeName` silently falls back to Standard Config |
| I-15 | Low | Consistency | Dialog-cancel restores styles but *not* extension-list edits (they hit the array directly via `_ApplyDialogItemText`) — verify & fix if confirmed |
| I-16 | Low | Dead code | `//~bool ChooseFontDirectWrite(...)` (`Styles.c:~50`) — decide delete vs enable |
| I-17 | Low | Feature | Dark-mode 16-colour defaults (`s_colorLightDefault[]`, `s_colorDarkDefault[]`) are hard-coded with no load/save path |
| I-18 | Low | Feature | Style names are compile-time fixed — no user-defined styles, no extension lists on custom detection beyond extension/regex |
| I-19 | Low | Performance | Schema auto-detect is O(n) per file open across ~60 schemas × multi-token extension lists |
| I-20 | Low | Performance | All `themes\*.ini` are stat-scanned at startup even if user never switches themes |

## 4. Enhancement proposals

Each proposal below lists the motivation, concrete change, files to touch,
and likely risk level. They are grouped into tiers so a session can pick
up an easy batch without stepping on the harder ones.

---

### Tier 1 — Quick wins (half-day each)

#### E-01 — Warn on style-string truncation (closes I-01)

**Motivation.** When a user hand-edits a style in `Notepad3.ini` and the
result exceeds 255 chars (easy with long font names plus many
attributes), the tail is silently dropped on load. No signal is given.

**Change.**
- In `_ReadFromIniCache()` (`src/Styles.c`), after each
  `IniSectionGetString(..., dst, BUFSIZE_STYLE_VALUE)`, compare the
  returned length against `BUFSIZE_STYLE_VALUE - 1`. If equal, the value
  was almost certainly clipped.
- Accumulate a list of offending `<section> / <key>` pairs in a static
  buffer.
- After `_ReadFromIniCache()` completes, if any clipped values were
  found, show a single `InfoBoxLng(MB_ICONWARNING, ...)` listing the
  first few (e.g. up to 5) and hinting that the cap is 255 chars.
- Add string IDs `IDS_MUI_STYLE_TRUNCATED` to `language/common_res.h`
  and all 26 `strings_*.rc` files with an English placeholder (per
  memory note about locale files).

**Risk.** Low. Read-only addition.

#### E-02 — Reset-whole-schema button (closes I-04)

**Motivation.** Customize Schemes has a *Default* button that resets the
currently-selected style. Users who want to revert an entire schema
(say, C++) to defaults currently have no one-click path.

**Change.**
- Add a new button to `IDD_MUI_STYLECONFIG` next to the existing
  *Default* button, e.g. `IDC_STYLERESETSCHEMA`, labelled
  "Reset &Scheme".
- When the selected tree node is a schema (not a style), enable the
  button; otherwise disable. Current wiring in
  `Style_CustomizeSchemesDlgProc` already distinguishes the two cases.
- On click: for every `EDITSTYLE` in `pLexCurrent->Styles[]`, copy
  `pszDefault` into `szValue`; also reset `szExtensions` to
  `pszDefExt`. Refresh the preview.
- No backup/undo required — the existing `pStylesBackup` captured on
  dialog open already covers Cancel.

**Risk.** Low. UI addition only.

#### E-03 — Drop `SelectDlgSize*` from theme exports (closes I-10)

**Motivation.** `SelectDlgSizeX` / `SelectDlgSizeY` are remembered
ListView dimensions for the *Select Scheme* dialog. They are unrelated
to styling and pollute every exported theme.

**Change.**
Two reasonable variants — pick one:

1. **Move them out of `[Styles]`** into `[Settings]` where other UI
   remembered-size values already live. Keeps the INI cleaner.
2. **Skip writing them** inside `Style_ToIniSection()` when invoked by
   the *Export* path. Easiest: add a boolean flag to
   `Style_ToIniSection(bool bForceAll, bool bIncludeUIState)` and pass
   `false` from `Style_Export()`.

Variant 2 is minimally invasive; Variant 1 is cleaner long-term. A
future session should pick one and stick with it.

**Risk.** Low. Either variant is easy to revert if feedback is
negative.

#### E-04 — Tooltip documenting `\regex` extension syntax (closes I-12)

**Motivation.** `\^CMakeLists$` style regex entries in `FileNameExtensions`
are a real feature, implemented in `Style_RegExMatchLexer()`
(`src/Styles.c:2543`), but the UI never mentions it.

**Change.**
- Add a tooltip (`TOOLTIPS_CLASS`) to the `IDC_STYLEEDIT_ROOT` control
  in `Style_CustomizeSchemesDlgProc`.
- Text (new localised string `IDS_MUI_EXTLIST_TOOLTIP`): "Semicolon-
  separated. Prefix with `\` for a regex matched against the full file
  name, e.g. `\^CMakeLists$`."
- Add the string to all 26 locale files.

**Risk.** Trivial.

#### E-05 — Theme-file header comment (closes part of I-07)

**Motivation.** Shared theme files currently have no authorship or
version metadata. A single leading comment line is cheap.

**Change.**
- In `Style_ExportToFile()` (when `bForceAll=true` path), before calling
  `SaveIniFileCache()`, prepend a single comment line to the cache via
  the existing IniSection helpers — e.g. via a new helper
  `IniFileSetHeaderComment()` if one doesn't exist, otherwise write the
  line as a key in a throwaway `[; meta]` section ordering hack.
- Line format: `; Notepad3 theme — exported YYYY-MM-DD from Notepad3
  <version>`.
- Check that existing `LoadIniFileCache()` tolerates a leading comment
  before the first section header (it should — standard INI).

**Risk.** Low; verify that Windows INI APIs (if used anywhere — the code
uses a custom parser) don't choke.

#### E-06 — Remove documented dead code (closes I-16, part of I-09)

**Motivation.** `//~`-prefixed lines accumulate without a decision.

**Change.**
- Delete the commented `ChooseFontDirectWrite` declaration near
  `Styles.c:50` if the MUI/ChooseFont path is the intended permanent
  solution.
- Either implement the commented `Style_StrGetAlpha` alpha2 branch for
  the Whitespace style (`src/Styles.c:~1626`, `~4489`) or delete it
  along with the matching "expose alpha2 for Whitespace" TODO. A
  comment in the source explaining *why* it's not supported (e.g.
  "Scintilla does not honour alpha2 on SCE_STANDARDWHITESPACE") would
  also close the loop.

**Risk.** Low. Cleanup only.

---

### Tier 2 — Medium features (1–2 days each)

#### E-07 — Decouple `FileNameExtensions` from theme Import (closes I-05)

**Motivation.** The current Import behaviour forces users to choose
between colours and their accumulated extension tweaks. Most real-world
"themes" shared online are colour-only.

**Change.**
- In `Style_Import()` (`src/Styles.c:714`), before the
  `FileOpenDlg` call, show a small confirmation dialog or use
  `FOS_` custom-place extension with a checkbox: *"Also import file-name
  extensions"*, default **off**.
- Propagate the flag into `Style_ImportFromFile(HPATHL, bool
  bImportExtensions)`.
- Inside `_ReadFromIniCache()`, gate the call to
  `Style_FileExtFromIniSection()` on this flag.
- Keep the old signature as a `static` inline caller that passes the
  legacy `true` for startup / theme-switch paths where the extension
  set must follow.
- **Symmetry:** add the mirror "include extensions" checkbox to Export
  too — see E-09.

**Risk.** Medium. The Import path also runs on theme switch; ensure
that Factory Reset and Theme switching still reset extensions. Default
`true` on those paths, `false` only on explicit *Import…* button.

#### E-08 — Dynamic `Theme_Files[]` (closes I-03)

**Motivation.** 24-theme cap is arbitrary and not even documented.
Some users have 40+ curated themes.

**Change.**
- Replace the fixed `THEMEFILES Theme_Files[25]` (`src/Styles.c:374-400`)
  with a heap-allocated grow-on-demand array. Suggested: a local helper
  `ThemeList` wrapping a `THEMEFILES*` pointer and a size_t count.
- Update every reference: `ThemeItems_CountOf()`, `ThemesItems_Init()`,
  `ThemesItems_Release()`, `ThemesItems_MaxIndex()`, the loop in
  `_FillThemesMenuTable()`, and `Style_InsertThemesMenu()`.
- Keep `extern "C" THEMEFILES Theme_Files[]` compatibility by exposing
  a `Theme_Files_Get(unsigned i)` accessor, or by turning the external
  reference (`src/Config/Config.cpp:77`) into a function call.
- Bound at something sane (e.g. 256) to avoid accidental DoS on a
  poisoned `themes\` directory.

**Risk.** Medium. Touches many call sites; needs care with menu
command ID ranges (`IDM_THEMES_STD_CFG + i`) — decide whether to keep
consecutive IDs (cap at a known range) or switch to a single
reserved range checked in the WM_COMMAND handler.

#### E-09 — Export filter: styles vs styles+extensions (pairs with E-07)

**Motivation.** Symmetry with the Import change; authors can now
publish *colours only* themes cleanly.

**Change.**
- Add a checkbox "Include file-name extensions" to the Export dialog
  (use `CustomizeSaveFileDialog` or bundle it into a small pre-dialog
  that then launches `FileSaveDlg`).
- Propagate into `Style_ExportToFile(HPATHL, bool bForceAll, bool
  bIncludeExtensions)`.
- Inside that function, gate the `Style_FileExtToIniSection()` call.

**Risk.** Low.

#### E-10 — Import validation (closes I-13)

**Motivation.** Loading a random INI as a theme silently succeeds,
overwriting in-memory styles with partial or unrelated data.

**Change.**
- Before calling `_ReadFromIniCache()`, peek at the cache to confirm
  the imported file contains at least one of: `[Common Base]`,
  `[Styles]`, or any per-schema section that matches a known
  `g_pLexArray[]` name.
- If none found, show `InfoBoxLng(MB_ICONWARNING, ...,
  IDS_MUI_IMPORT_NOT_A_THEME)` and abort without touching in-memory
  state.
- Add the string to all 26 locale files.

**Risk.** Low.

#### E-11 — Handle missing theme file gracefully (closes I-14)

**Motivation.** If a theme file referenced by `Settings.CurrentThemeName`
was deleted between sessions, Notepad3 silently falls back to Standard
Config without informing the user — changes made "to the active theme"
after that point land in `Notepad3.ini`, not the expected file.

**Change.**
- In `_FillThemesMenuTable()` (`src/Styles.c:438`), remember whether
  the stored `Settings.CurrentThemeName` was actually matched.
- If not matched and non-empty, show a one-shot info message at
  startup: *"Theme '<name>' was not found in `themes\`. Reverting to
  Standard Config."*
- Gate under a `Settings2.NoThemeMissingWarning` flag so power users
  can suppress it.

**Risk.** Low. Add a new `Settings2` key — remember to document it per
the project's memory note on new parameters.

---

### Tier 3 — Larger features (multi-day)

#### E-12 — Per-schema extension priority (closes I-11)

**Motivation.** Today, if two schemas claim `.h`, whichever appears
first in `g_pLexArray[]` wins. Users who prefer C over C++ for `.h`
(or vice versa) can't express it.

**Change.**
- Add an optional integer key `Priority=<n>` inside each per-schema
  INI section. Default 0 when absent.
- In `Style_MatchLexer()` (`src/Styles.c:2517`), when searching by
  extension with `bCheckNames=false`, collect all matches and return
  the one with highest priority; tie-break by array index as before.
- Surface in Customize Schemes: a numeric spinner on the schema node
  page.

**Risk.** Medium. Touches the hot detection path; add a short-circuit
fast path for "exactly one match" (today's common case) to avoid a
double scan.

#### E-13 — Named custom-colour references (closes I-17 partially)

**Motivation.** The 16-slot custom-colour palette is currently isolated
from style strings; it's only used by the colour-picker dialog. Letting
users reference slots from style strings (`fore:@3`) would enable
trivial "palette-swap" themes: change 16 lines, re-theme everything.

**Change.**
- Extend `Style_StrGetColor()` (`src/Styles.c:~3476`) to accept `@N`
  (1-16) as an alternative to `#RRGGBB`. Resolve via
  `Globals.crCustom[N-1]`.
- When writing styles via the dialog, continue to emit `#RRGGBB` (so
  pre-existing themes stay pre-existing). Only users who opt in by
  hand-editing get the `@N` form.
- Add documentation (both in `CustomSchema.md` and an in-dialog
  tooltip) that `@N` resolves at draw time and follows changes to the
  custom palette.

**Risk.** Medium. Watch for cases where a style is evaluated before
`Globals.crCustom` is populated (early startup). Add a guard that
falls back to the compiled-in default colour in that window.

#### E-14 — Schema-detection cache (closes I-19)

**Motivation.** `Style_MatchLexer()` is O(n × tokens) per file open.
For a batch open of 1000 files, this adds up.

**Change.**
- Maintain a `WCHAR *ext → EDITLEXER*` hash map built once per session
  from the schema extension lists, rebuilt when Customize Schemes
  commits edits.
- `Style_MatchLexer()` first checks the hash, only falling through to
  the existing regex / shebang / sniff pipeline when the hash misses.
- Use `uthash` (`src/uthash/`) to stay consistent with the project's
  existing dependencies.

**Risk.** Medium. Must invalidate on every `_ApplyDialogItemText()`
for the extension field and after every theme switch.

---

### Tier 4 — Nice-to-have / discussion

- **E-15 — Custom-colour load/save symmetry for dark vs light defaults
  (I-17).** Today `s_colorLightDefault[]` / `s_colorDarkDefault[]` are
  compile-time constants; the palette is reset on factory reset. Allow
  persisting user-tweaked defaults per dark/light mode to match
  Windows' adaptive behaviour.

- **E-16 — User-defined highlight classes (I-18).** Huge undertaking —
  would require extending `EDITSTYLE` to be dynamically allocated
  per-lexer, teaching the INI parser about user-added keys, and
  wiring Scintilla indicator slots. Out of scope for a normal
  session; noted here only for completeness.

- **E-17 — Lazy theme scanning (I-20).** Defer the `FindFirstFile`
  sweep of `themes\` to the first time the user opens the *View →
  Themes* submenu. Negligible savings for most users; probably not
  worth it.

- **E-18 — Cancel-revert for extension edits (I-15).** Unverified
  claim; needs a quick read of `Style_CustomizeSchemesDlgProc`'s
  Cancel path. If the Cancel handler only restores `szValue` and not
  `szExtensions`, restore both. Two-liner fix if the issue is real.

## 5. Verification plan (shared across items)

For any change that touches style I/O:

1. **Round-trip test.** Save a reference file list with heavy theming
   (e.g. `Dark.ini`), load it, save again — the second save must be
   byte-identical except for the known `SelectDlgSize*` / timestamp
   differences.
2. **Locale coverage.** For every new `IDS_MUI_*` string, run
   `Build/rc_to_utf8.cmd` and confirm all 26 locale `strings_*.rc`
   files still load (no BOM, CRLF preserved — see project memory
   note).
3. **Theme-switch smoke test.** Switch Standard Config → Dark →
   Obsidian → Standard Config; confirm `Settings.CurrentThemeName`
   is persisted and the menu radio follows.
4. **Customize Schemes smoke test.** Open the dialog, edit several
   styles, click Cancel — verify that the edits are reverted (nothing
   leaks to INI on a subsequent Save).
5. **Missing-theme test.** Rename `themes\Dark.ini` to something else
   between sessions; ensure E-11's warning fires exactly once.
6. **ARM64 smoke.** Theme switching invokes a lot of redraws — verify
   no flicker regression on ARM64 (relevant because of
   `RenderingTechnology=2` default there — see
   `readme/config/Configuration.md`).

## 6. Documentation hooks

Whenever any of the enhancements above lands, update:

- `readme/schema/CustomSchema.md` — user-facing behaviour.
- `readme/config/Configuration.md` — new `Settings2` keys (E-11, and
  anything else that introduces one).
- `CLAUDE.md` and `.github/copilot-instructions.md` — if the change
  introduces a new pattern worth guiding future contributions toward.
- `Build/Notepad3.ini` — commented-out entries for any new INI keys,
  per the project's "document new parameters" convention.

## 7. Out of scope (explicitly)

- Renaming schemas or reordering `g_pLexArray[]`. The array order is
  load-bearing for current-session INI compatibility.
- Switching away from Scintilla / Lexilla lexers.
- Adding a full-blown theme editor GUI separate from Customize
  Schemes.
- Making style keys user-extensible (E-16 above is documented but
  deliberately parked).
