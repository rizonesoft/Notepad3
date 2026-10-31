# NASTRAN BDF/NAS

## Implementation

- `lexilla/lexers_x/LexNASTRAN.cxx` implements `SCLEX_NASTRAN` (206). Custom
  lexer/style IDs are declared in `lexilla/lexers_x/SciXLexer.h` and `SciX.iface`.
- `src/StyleLexers/styleLexNASTRAN.c` defines the `NASTRAN` scheme, common-card
  completion dictionary, and control/field keywords. Matching is ASCII
  case-insensitive; unknown and vendor-specific cards still get card styling.
- Default extensions are `bdf; nas`. Preserve other schemes' indices by appending
  the new scheme to `g_pLexArray`. The theme section is `[NASTRAN]`.
- `$` is the line-comment delimiter; no stream-comment delimiter is provided.
  Quoted dollar signs in INCLUDE paths and string fields are literal.

## Field handling

- Small fixed format: an 8-column card-name field, eight 8-column data fields,
  and an optional continuation label in columns 73-80.
- Large fixed format: an 8-column card-name field ending in `*`, followed by four
  16-column data fields and the optional continuation label. Large continuation
  lines begin with `*` or `*label`. Small and large physical records may mix.
- Free format: a comma in the first ten expanded columns selects comma-separated
  fields, retaining empty fields. Commas inside quoted strings are not separators.
  Small/large free records use eight/four payload fields respectively.
- Tabs are mapped to 8-column stops for styling, without modifying the document
  or relying on the user's displayed tab width. Numeric fields include explicit
  E/D exponents and compact NASTRAN exponents (`1.25-3`, `-.21+5`, `1.+6`).
- Lexing and folding must never insert, delete, align, truncate, or convert field
  contents. This feature provides display/recognition/folding, not text formatting.

## Folding and incremental state

- Executive control starts at SOL/ID/ASSIGN/NASTRAN. CEND starts the case-control
  fold; BEGIN BULK and BEGIN SUPER start bulk-data folds; ENDDATA closes a deck.
- SUBCASE entries inside case control create sibling folds inside that section.
- A bulk card followed by `+`/`+label`, `*`/`*label`, or a blank first field folds
  together with its continuation records. Intervening comments/blank lines stay
  inside a continuing card. Orphan continuations do not invent fold headers.
- Quoted multiline INCLUDE paths fold as one entry, including literal `$` inside
  a still-open quote. An unmatched ordinary string does not carry onto another
  card. This lexer is not a solver or a continuation-label validator.
- Immutable state IDs are stored on Scintilla lines. An incremental edit revisits
  the preceding meaningful line to recompute its continuation-dependent header,
  including across blank/comment lines, then propagates until state stabilizes.
  Include the requested final empty line when converging at EOF.

## Verification

Visual fixtures and expected fold counts are in
`test/test_files/StyleLexers/styleLexNASTRAN/README.md`.
Open them in the NASTRAN scheme, test both themes and line endings, and verify
fold-all/expand-all plus edits to continuation markers. The same Windows package
contains APDL and NASTRAN fixtures in separate example directories.

Format references: [pyNastran BDF card formatting](https://pynastran-git.readthedocs.io/en/latest/manual/bdf_developer.html#bdf-card-formatting)
and its [field reader](https://github.com/SteveDoyle2/pyNastran/blob/main/pyNastran/bdf/bdf_interface/utils.py).
