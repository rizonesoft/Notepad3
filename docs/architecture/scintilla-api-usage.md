# Scintilla API Usage Notes

Verified constraints to check before replacing home-grown `src\Edit.c` code with a Scintilla message. Wrapper rules are in [Codebase and conventions](codebase-and-conventions.md).

## Reading document text

- Prefer one `SciCall_GetRangePointer(start, len)` per line or range over a `SciCall_GetCharAt` loop (one message per byte). The pointer is valid only until the next document modification; reacquire it after any edit.
- Byte-wise scanning for ASCII characters is safe in UTF-8 (`SciCall_GetCodePage() == SC_CP_UTF8`), because multi-byte sequences consist of bytes >= 0x80. For other code pages (DBCS trail bytes can equal `[`, `]`, `{`, `}`), step with `SciCall_PositionBefore/After`.

## Pitfalls of specific messages

| Message | Behaviour to respect |
|---|---|
| `SCI_POSITIONRELATIVE` | Clamped to `[0, Length]` in `scintilla\src\Editor.cxx`. An offset past the document end returns **0**, not the end. Compare the result with the start position before using it as a limit (see `EditGetExcerpt`). |
| `SCI_SETLINEINDENTATION` | Does nothing when the requested indentation width equals the current one (`Document::SetLineIndentation`, `scintilla\src\Document.cxx`). It cannot convert tabs to spaces (or back) at the same width. |
| `SCI_REPLACETARGETMINIMAL` | `Sci_ReplaceTargetTestChgHist` (`src\SciCall.h`) uses it only when change history is enabled; otherwise a full `SCI_REPLACETARGET`. |
| `SCI_MARKERNEXT` / `SCI_MARKERPREVIOUS` | Do not see change-history markers (21-24); `_MarkerNext` / `_MarkerPrevious` in `src\Edit.c` scan linearly for them. |

## Wrapped but unused

`SciCall_IsRangeWord` exists in `src\SciCall.h` but has no caller. See [open questions](../open-questions.md) before using it to replace delimiter scans.
