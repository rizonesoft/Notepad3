# ANSYS APDL visual fixture

Open [highlighting.apdl](highlighting.apdl) in Notepad3. Check automatic selection
for `.apdl`, and copies with `.mac` and `.cdb` extensions; the scheme can also be
selected manually. Existing `.inp` (ABAQUS) and `.ans` (ANSI Art) associations are
intentionally retained. No ANSYS installation is needed.

- `/PREP7` and `FINISH`: Processor; `NSEL`: Command; `/TITLE`: Slash Command;
  `*DO`: Star Command; `ALL`: Argument; `SQRT` and `ABS`: Function. The mixed-case
  examples should have the same styles. These are the upstream lexer's six
  keyword sets, in that order.
- Check `!`/`!!` comments, numbers including `2.1E11`, both quote styles,
  operators, and ordinary parameter names. An exclamation mark inside a quoted
  string stays a string. Toggle line comments to insert/remove `!`.
- With folding enabled, `*DO`, `*IF`, and `*DOWHILE` open folds; `*ENDDO` and
  `*ENDIF` close them. Test the built-in dark mode and the Dark, Obsidian, and
  Sombra themes for readable styles and inherited fonts.

## Upstream behavior and provenance

`lexilla/lexers/LexAPDL.cxx` is copied unchanged from Lexilla 5.5.5,
tag `rel-5-5-5`, commit `e67537693bfb1ae4e6f643dcab725a38ffeac3e3`, matching
`lexilla/version.txt`:

https://github.com/ScintillaOrg/lexilla/blob/e67537693bfb1ae4e6f643dcab725a38ffeac3e3/lexers/LexAPDL.cxx

This integration retains upstream tokenization and folding. In particular, it
does not add CDB data-block folding or special `/COM` tail parsing; lexer
improvements belong upstream in Lexilla. The fixture describes manual checks,
not completed Windows GUI test results.
