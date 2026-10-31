# APDL visual fixtures

Select **ANSYS APDL** and enable code folding in Notepad3. The default extensions
are `.apdl`, `.mac`, and `.cdb`; `.inp` remains ABAQUS and `.ans` remains ANSI Art.
Check both the default light scheme and a dark theme.

- `control.mac`: six fold headers: macro, outer loop, IF, ELSEIF, inner loop,
  ELSE. Each closing line belongs to its block. Single-line IF actions and a
  loop opened/closed on one line do not create headers. Comment/string contents
  do not create folds. Check `/COM` text with unmatched quotes, quoted `!`/`$`,
  doubled quotes, E/D exponents, expressions, and VWRITE format records.
- `data.cdb`: six fold headers: first NBLOCK, EBLOCK, two CMBLOCKs, loop, nested
  NBLOCK. Collapsing a data block hides its format, records, and terminator;
  the next command stays visible. Negative coordinates and negative component
  range endpoints must not end a block. The second CMBLOCK has adjacent integer
  fields without whitespace and ends by its encoded item count.
- `blocks.cdb`: 24 fold headers (16 data blocks and 8 `/BLOCK` groups), based on
  Programmer's Reference 2026 R1 section 3.2. Covers BFBLOCK/BFEBLOCK/SFEBLOCK
  command-ended records, DBLOCK/FBLOCK/ETBLOCK/NSMBLOCK first-field sentinels,
  NBLOCK's UNBL ending, RLBLOCK with two descriptors and multiple continuation
  lines, all three SECBLOCK forms, *PREAD, and typed /BLOCK groups including
  ELST/ELEN. Also covers EFACE/EFAC CMBLOCK names and bitmap entries in one fold.
  Each `/COM, after ...` line must be at the data block's parent level. Negative
  payloads, table references, a reinforcing element's negative-ID continuation,
  and component names looking like commands must remain inside the block.
  NOTABLOCK and a `bfblock = 1` assignment must not create folds.
- `incomplete.apdl`: three fold headers: IF, NBLOCK, unfinished loop. An unmatched
  or wrong-type closing command does not decrease the level. A missing data-block
  terminator recovers at the next command. The unclosed string does not bleed to
  the next line. This fixture intentionally has no final newline.

For each fixture, collapse/expand individual and nested blocks, then use the
existing fold-all/expand-all commands. Edit an opening/closing command, insert or
remove lines inside a block, and undo/redo. The result should match reopening the
file. Repeat with CRLF, LF, and CR line endings. Row widths in `data.cdb` are
significant: do not reformat the fixed-width records.

Primary boundary reference: [Programmer's Reference 2026 R1, Coded Database File
Commands](https://ansyshelp.ansys.com/public/Views/Secured/corp/v261/en/ans_prog/Hlp_P_INT3_3.html).
Additional examples: ANSYS's open-source PyMAPDL Reader writers
[`write_nblock`/`write_eblock`](https://github.com/ansys/pymapdl-reader/blob/main/ansys/mapdl/reader/cython/archive.c)
and [`write_cmblock`](https://github.com/ansys/pymapdl-reader/blob/main/ansys/mapdl/reader/archive.py).
