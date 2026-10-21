# Regular Expressions

## PCRE2 integration

`scintilla\pcre2\scintilla\PCRE2RegExEngine.cxx`, compiled with `SCI_OWNREGEX`, replaces Scintilla's built-in regex engine. Scintilla search and replacement use `FindText()` and `SubstituteByPosition()`.

- `translateRegExpr()` maps Notepad3 regex syntax to PCRE2 syntax. It translates `\<` and `\>` word-boundary markers to lookarounds and `\uHHHH` to `\x{HHHH}`.
- `convertReplExpr()` normalizes `\1`-`\9` capture references to `$1`-`$9` and processes `\n`, `\t`, `\xHH`, and `\uHHHH`.
- Supported replacement backreferences include `$0`-`$99`, `\0`-`\9`, `${name}`, and `${+name}`.
- The standalone `RegExFind()` entry point is used by `EditURLDecode()` in `src\Edit.c`.

## URL hotspots

`HYPLNK_REGEX_FULL` in `src\Edit.c` recognizes `https?://`, `ftp://`, `file:///`, `file://`, `mailto:`, `www.`, and `ftp.` forms. The pattern permits selected punctuation inside parenthesized URL segments and requires a non-punctuation final character to avoid consuming sentence punctuation.
