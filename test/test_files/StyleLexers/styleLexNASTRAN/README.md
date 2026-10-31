# NASTRAN visual fixtures

Select **NASTRAN** in Notepad3, or open a `.bdf`/`.nas` file. Enable code folding
and test both the default light scheme and a dark theme.

- `formats.bdf`: 11 fold headers: executive, case-control, two SUBCASEs, bulk,
  fixed RBE2, large GRID, two free SET1s, large free GRID, and multiline INCLUDE.
  Compact exponents, E/D exponents, empty fields, continuation labels in columns
  73-80, and named `+`/`*` continuations must be styled correctly. The dollar sign
  inside the multiline quoted INCLUDE remains string data, not a comment.
- `bulk-only.nas`: 4 fold headers: free RBE3, fixed RBE2, fixed SET1 with a blank
  continuation field, and BEGIN SUPER. This file begins directly with bulk cards.
  Tabs must be interpreted at 8-column stops without being replaced. The comma
  in column 10 still selects free format.
- `incomplete.bdf`: 1 fold header: SET1 with its comment and continuation. The
  orphan continuation creates no header. CUSTOM still gets card styling; an
  ordinary unclosed string does not leak onto the following GRID. The unfinished
  INCLUDE at EOF has no following line to fold. There is no final newline.

Collapse/expand individual cards, SUBCASEs, and sections; verify that the next
card/section stays visible. Add, remove, or change the first continuation marker,
including across intervening comments; compare with reopening the file. Repeat
with LF, CRLF, and CR endings. Check that all original field bytes remain intact:
do not auto-align or reformat these fixtures, since fixed column widths matter.

Column-guide visual check (`columns.bdf`, 3 fold headers):

- View > NASTRAN Column Guides defaults to 8-character fields. Check all small
  fields against the guides, then select 16-character fields for GRID*.
- The line after column 72 is blue; the line after column 80 is orange. Check
  both light/dark backgrounds, zoom levels, and horizontal scrolling.
- Ctrl+Alt+L hides/shows the chosen width. Off removes all guides in NASTRAN;
  free-field records do not need a fixed grid. Keep a monospace font, display
  tabs at width 8, and turn off visual word wrap for physical card columns.
- Switch to APDL and back: ordinary long-line preferences must return outside
  NASTRAN, and the NASTRAN preset must return inside it. Check save/restart.
- Guide changes must never edit card text or mark the document modified.
