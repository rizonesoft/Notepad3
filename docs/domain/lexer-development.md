# Lexer Development

Each language has an `EDITLEXER` definition in `src\StyleLexers\styleLexXXX.c`. Register new lexers in the `Styles.c` lexer array and add localization string IDs.

## Implementation touchpoints

- **Scintilla properties:** if a lexer exposes properties (for example, `lexer.foo.allow.X`), add its `case SCLEX_FOO:` to the property dispatch in `EditLexer.c` and call `SciCall_SetProperty(...)`. Otherwise constructor defaults silently remain in effect.
- **Shared lexer IDs:** `SCLEX_CPP` is used for C, C#, Java, JS, and RC; `SCLEX_D` for D and Go; `SCLEX_PYTHON` for Python and Awk. Changes to a shared lexer or its property dispatch can affect every language wired to that ID.
- **Comment toggling:** lexers with comments need `SCLEX_FOO` cases in both `Lexer_GetStreamCommentStrgs` and `Lexer_GetLineCommentStrg` in `EditLexer.c`.
- **Theme sections:** INI sections use the `pszName` field (the fourth `EDITLEXER` field), not the lexer name string. Add sections for new lexers to applicable theme INIs: `Build\Notepad3.ini`, `Build\Themes\*.ini`, `res\StdDarkModeScheme.ini`, and locale variants `Build\Notepad3_<locale>.ini`. Renaming `pszName` can orphan existing user style customizations.
- **New style rows:** add a matching style entry to the applicable theme overrides. Only `res\StdDarkModeScheme.ini` and `Build\Themes\{Dark,Obsidian,Sombra}.ini` have per-style overrides; the main and locale INIs often have empty lexer sections where inline defaults apply. For foreground/background pairs, target WCAG contrast of at least 4.5:1. `lexilla\wscite\*.properties` can provide color references.
- **Style localization:** each new `EDITSTYLE` label needs an `IDS_LEX_STR_*` definition in `language\common_res.h` and an English placeholder in all 26 `language\np3_*\lexer_*.rc` files. Check ID gaps when choosing an ID. For bulk insertion, anchor on an existing identifier (not translated text), run once, and verify one occurrence per file.

## Lexilla sources

Homebrew lexer sources under `lexilla\lexers_x\` include AutoHotkey (`LexAHK` and `LexAHK2`), CSV, JSON5, Kotlin, and Verilog. Their `SCE_*_*` enums are in `lexilla\lexers_x\SciXLexer.h`, not `lexilla\include\SciLexer.h`. `LexHTML.cxx` is under `lexilla\lexers\` in this repository; `SCLEX_HTML` and `SCLEX_XML` use that source.

## Fixtures and verification

`test\test_files\StyleLexers\styleLex<NAME>\` contains lexer fixtures. Any tokenization change (style, state-machine branch, keyword list, escape handling, fold rule, etc.) must extend or add a fixture. Open fixtures in Notepad3 for visual verification; automated lexer assertions are not in scope. Folder names follow the EDITLEXER source file; the historical AHK folder is `styleLexAHKL\`.

Style-label resources are part of lexer work; general locale resource invariants and UI strings are documented in [Localization](localization.md).

## ANSYS APDL

- `lexilla/lexers_x/LexAPDL.cxx` uses the upstream `SCLEX_APDL` and `SCE_APDL_*`
  constants already declared in `SciLexer.h`; do not allocate another custom ID.
  Register `lmAPDL` in the curated `lexilla/src/Lexilla.cxx` catalogue and keep
  `lexilla/Lexilla.vcxproj` plus the main application's project in sync.
- `src/StyleLexers/styleLexAPDL.c` defines processors, commands, arguments, and
  functions as lowercase keyword lists. Matching is ASCII case-insensitive.
  Only command positions (line start or after `$`) open/close blocks. Strings,
  `!` comments, `/COM` tails, and VREAD/VWRITE/MWRITE format records are excluded.
- Fold pairs are DO/DOWHILE-ENDDO, IF-THEN/ELSEIF/ELSE-ENDIF, and CREATE-END.
  IF forms with STOP/EXIT/label actions do not open a fold. Closing commands must
  match the innermost block type; unmatched closes cannot underflow the level.
- CDB folding follows the [Programmer's Reference 2026 R1, section 3.2](https://ansyshelp.ansys.com/public/Views/Secured/corp/v261/en/ans_prog/Hlp_P_INT3_3.html).
  Use explicit grammars, not a generic command-name suffix rule:

  | Block | Boundary |
  |---|---|
  | NBLOCK | `N,UNBL,LOC,-1` (also legacy `N,R5.3,LOC,-1`), `N,-1`, or a `-1` entity field |
  | EBLOCK, ETBLOCK, DBLOCK, FBLOCK, NSMBLOCK | `-1` in the first entity field |
  | BFBLOCK, BFEBLOCK, SFEBLOCK | Corresponding `BF`/`BFE`/`SFE,end,LOC,-1` record |
  | CMBLOCK | Declared count of encoded entries using the integer field width |
  | RLBLOCK | NUMSETS sets; per-set value count determines continuation records |
  | SECBLOCK (beam/shell) | Nodes + cells from the first record, or N layer records |
  | SECBLOCK (reinforcement) | Member records plus negative-ID Format2 continuations; resume at the following command |
  | *PREAD | `END PREAD` |
  | /BLOCK | Matching type's START/END, or ELST/ELEN |

- Entity-field terminators must not match negative load values, coordinates, or
  substrings such as `-1.0`. Load command terminators must match the complete
  record, including the corresponding command and LOC field. A command following
  a missing data terminator resumes normal command parsing.
- CMBLOCK counts negative range endpoints as single encoded entries. EFACE/EFAC
  components may contain `(1a240)` component-name records followed by another
  descriptor and NUMBF bitmap entries. Those names are data even when they look
  like APDL commands. Keep both phases in one fold.
- RLBLOCK has two descriptors. The first record of each set contains its ID and
  value count (normally 2i8), then up to six reals; continuation records normally
  contain seven. A negative real value is not a terminator. SECBLOCK reinforcing
  Format2 records start with a negative element ID; do not mistake them for an
  ETBLOCK/EBLOCK sentinel.
- /BLOCK supports FILE, NODE, ELEM, MAT, CONT, LOAD, INIS, and ELST/ELEN groups.
  Its body is APDL code, so nested data blocks remain independently foldable.
- Fold closing records stay inside the fold; ELSE/ELSEIF start sibling folds.
  Immutable parse states are interned and referenced by Scintilla line-state IDs,
  which move with lines on edits. Restyling starts at a complete line, restores the
  previous state, and continues until the requested range and changed states end.
  Do not replace this with a whole-document pass for each edit of a large CDB.
- The theme section is `[ANSYS APDL]`. Default extensions are `apdl; mac; cdb`;
  preserve ABAQUS's `inp` and ANSI Art's `ans`. Append to `g_pLexArray` to avoid
  shifting existing saved default-scheme indices and command-line lexer indices.
- Visual checks and expected fold headers are in
  `test/test_files/StyleLexers/styleLexAPDL/README.md`. No ANSYS installation or
  license is needed to edit, highlight, or fold APDL source.
