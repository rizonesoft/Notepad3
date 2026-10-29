# Encoding Diagnostics and Tests

Companion to [Encoding detection](encoding-detection.md): developer title diagnostics and the native regression harness for `src\uchardet\EncodingDetection.cpp`.

## Developer title diagnostics

With `Settings2.DevDebugMode` enabled, the title shows the final selection first, then the validation evidence and UCHARDET candidate:

```text
ENC=UTF-16LE <- UTF-16(NUL-S) | U8=invalid | U16=UTF-16LE/valid | UCD(60.0%)=WINDOWS-1252
```

- `ENC` is the detection result; `<-` identifies the winning path using the abbreviations below. UTF-32 reports conversion to UTF-8.
- `U8` describes the input bytes, not the selected document's validity. `invalid` is expected for most valid UTF-16 input; `valid+NUL` identifies otherwise-valid UTF-8 containing NUL bytes.
- `U16`, when present, identifies the Unicode candidate and validation state. `valid` includes whole-input surrogate validation; `BOM` indicates signature evidence, not a validation claim.
- `UCD(pp%)=candidate` shows the adjusted confidence and candidate; `none` indicates no result. Accepted/rejected/ignored labels are omitted because `<-` already identifies the winning path. `UCD=skipped(reason)` distinguishes disabled analysis, an override, a BOM/strong UTF-16 decision, the ASCII shortcut, or UTF-32 conversion.

| Winning path | Meaning |
|---|---|
| `UCD`, `UCD(FORCE)` | UCHARDET selection, normal or forced auto-detection |
| `BOM` | Unicode byte-order mark/signature |
| `OVR`, `F-TAG`, `NFO/OEM` | Explicit override, file encoding tag, or NFO/OEM preference |
| `UTF-8(valid)` | UTF-8 byte validation |
| `UTF-16(NUL-S)`, `UTF-16(NUL-W)` | Strong/weak NUL-pattern evidence with valid surrogate pairing |
| `UTF-16(WIN)`, `UTF-16(FORCE)` | Validated Windows evidence, or Unicode fallback in forced auto-detection |
| `DEF`, `ASCII-PREF`, `HINT`, `ANSI-FB` | Configured default, ASCII preference, weak hint, or ANSI fallback |
| `BOM(U32LE->U8)`, `BOM(U32BE->U8)` | BOM-bearing UTF-32 converted to UTF-8 |
| `UTF-32LE(range->U8)`, `UTF-32BE(range->U8)` | Range-detected UTF-32 converted to UTF-8 |

`HINT` means `Encoding_SrcWeak(CPI_GET)`, not the OS ANSI code page or the confidence-bonus hint. `FileLoad()` seeds it from the file's MRU encoding; `FileRevert()` normally seeds it from the current document encoding. `_SetResultingEncoding()` can use an external 8-bit weak hint when UCD is not reliable and no higher-priority path won. Thus a correct UCD candidate below the configured threshold can lose to an earlier, incorrect MRU encoding. `OS-CP` is independent of that remembered choice.

`ENC_DET_T` records diagnostic provenance with compact static-literal reason strings and `bUCDRan`; the formatter uses those labels directly. `_SetEncodingTitleInfo()` runs after final selection, including early UTF-32 returns. `EditLoadFile()` appends explicit-override information without replacing the pipeline summary. These fields do not affect selection. The native harness checks both confidence adjustment and title output.

## Regression coverage

Run `test\test_files\encoding\TestEncodingDetection.ps1` with Visual C++ build tools installed. The runner and its `TestEncodingDetection.cpp` harness live alongside the `UTF-16`, `UTF-32`, and `UTF-8` fixture directories. The runner defaults to `UTF-16`; use `-FixtureDirectory` to supply another directory containing the two BOM-less UTF-16 fixtures. Missing fixtures fail the run rather than silently skipping coverage.

It compiles the current detector, confidence adjustment, title formatter, selection helpers, and validators into a temporary native harness. Application state and the low-level UCHARDET call are stubbed; the latter supplies a legacy-encoding result (Windows-1252) that must not override correct UTF-16 evidence. This covers both endiannesses, surrogate validity beyond the sample, sparse-NUL Windows fallback, odd length, BOMs, UTF-8 with NULs, explicit overrides, skip-Unicode mode, diagnostics, and forced auto-detection. The BOM-less UTF-16 fixtures under `test\test_files\encoding\UTF-16` are also checked.

Override tests cover UTF-32 early returns, physical versus requested BOMs, conflicting UTF-16 byte order, file tags, NFO/OEM, and MRU-style weak hints competing with reliable/unreliable UCD results. The harness does not execute `EditLoadFile()` or the real UCD language models; use a full build and editor-load checks for those boundaries.

For cross-process editor-load checks, `WM_GETTEXTW` retrieves UTF-16 text but Windows marshalling stops copying at the first NUL. Validate embedded-NUL documents with pointer-free `SCI_GETLENGTH`/`SCI_GETCHARAT` calls instead; a zero-filled suffix in a marshalled `WM_GETTEXTW` buffer is not evidence that the document lost its suffix.

The BOM-less UTF-16 fixtures have 289 UTF-16 code units, 220 dominant-position NULs, and two opposite-position NULs (valid surrogate units 0xDE00 and 0xDC00), so strong classification must tolerate a few opposite-position NULs. Bundled UCHARDET reports Windows-1252 at 50% for both; the combined preference weight raises that to 60% on a Windows-1252 system, which must still not override the validated UTF-16 evidence. UCHARDET has no BOM-less UTF-16 prober; its UTF-16 recognition in `nsUniversalDetector::HandleData()` is BOM-based.

Do not rely on the Boolean `IsTextUnicode()` return: it is false for the BE fixture even though useful reverse-order flags are set; inspect the flags.

