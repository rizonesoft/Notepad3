# Open Questions and Coverage Review

Holding area for findings that are **not yet verified enough** for a topic page. Nothing here is authoritative; do not act on an entry without checking the code. Promote verified entries into the relevant page and delete them here; delete entries that turn out to be wrong.

## Open findings

Add one bullet per finding: `- <area/file>: <observation> — <what would verify it> (<YYYY-MM-DD>)`.

- `src\Edit.c` `EditMarkAll` (whole-word check): replacing the delimiter loop with `SCI_ISRANGEWORD` looks stricter, because the range must also sit on word boundaries. A selection inside a longer word (for example `foo` in `foo_bar`) would stop marking whole-word `foo` occurrences elsewhere — read `Document::IsWordAt` and compare with `DelimChars` / `DelimCharsAccel` behaviour before changing (2026-10-29).
- `src\Edit.c` whole-range replacements (`EditCompressBlanks`, `EditTabsToSpaces`, `EditSpacesToTabs`, `EditJoinLinesEx`, `EditWrapToColumnEx`, case functions): using `SCI_REPLACETARGETMINIMAL` unconditionally would keep markers/indicators on the unchanged prefix and suffix; cost and behaviour change not measured. First check in `scintilla\src\Document.cxx` / `CellBuffer.cxx` whether it only trims a common prefix and suffix or does a real diff, since marker preservation in the middle of the range depends on it; then test on a document with bookmarks (2026-10-29).
- `src\Edit.c` `EditMarkAll(bMultiSel)`: one `AddSelection` per match, while `EditSelectionMultiSelectAll` uses `SCI_MULTIPLESELECTADDEACH`; converging needs `iSelectionMarkNumber` / `iMarkOccurrencesCount` derived afterwards (2026-10-29).
- `src\Edit.c` `EditWrapToColumnEx`: only caller is `src\Notepad3.c:6402`; whether `SCI_LINESSPLIT` (used by `EditWrapToColumn`) can replace it depends on its delimiter and tab handling — compare behaviour first (2026-10-29).
- `src\Edit.c` `EditHasTrailingBlanks`: three messages per line; a `SCI_SEARCHINTARGET` regex for `[ \t]$` might be faster — benchmark on a large file before changing (2026-10-29).
- `src\Edit.c` `_FindInTarget`: the pattern conversion is now cached, but `SciCall_SetSearchFlags` and the target save/restore still run on every call; the gain from splitting those into a prepare and an iterate step was not measured (2026-10-30).
- `src\Edit.c` Scintilla-API audit, second pass (2026-10-30): read the find/replace functions, `EditSortLines`, clipboard, `EditConvertText`, URL, Base64 and hex tools and `EditModifyNumber`. No further Scintilla message replaces home-grown code there (there is no Scintilla replace-all, recode, locale sort or wildcard search). Not read: the dialog procedures (~6930-7930 and ~9377-10260).

## Coverage review

Pages that are small relative to the area they cover. They are review candidates, not known to be incomplete. When a session works in one of these areas, compare the page against the code and extend it with verified guidance; remove the row once reviewed.

| Page | Size | Area to compare against |
|---|---|---|
| [Regular expressions](domain/regular-expressions.md) | ~1 KB | PCRE2 integration and search/replace paths |
| [Localization](domain/localization.md) | ~1.4 KB | Locale resources under `language\` and string-adding workflow |
| [Configuration and INI](domain/configuration-and-ini.md) | ~1.9 KB | `Config.cpp` settings load/save and `Settings` / `Settings2` structs |
| [Lexer development](domain/lexer-development.md) | ~2.9 KB | Lexer registration, styles, and themes |
