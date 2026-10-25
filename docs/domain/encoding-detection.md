# Encoding Detection

Encoding detection is implemented in `src\uchardet\EncodingDetection.cpp` and is called from `EditLoadFile()` in `src\Edit.c`. It combines explicit declarations, byte signatures, structural checks, and UCHARDET analysis; no single heuristic is authoritative for every input.

## Detection flow

1. The detector examines file variables and forced-encoding settings before automatic selection. Without an enforced encoding, a UTF-32 BOM immediately selects the UTF-32 conversion path. Otherwise it records actual UTF-16/UTF-8 signatures and scans for strict UTF-8 validity, ASCII-only content, and NUL bytes.
2. When Unicode detection is enabled and encoding is not enforced, BOM-less UTF-32 is checked before BOM-less UTF-16. UTF-16 uses null-byte distribution and a guarded Windows fallback; both validate surrogate pairing throughout the proposed UTF-16 input. UTF-32 uses sampled code-point range checks.
3. When ANSI code-page detection is enabled, UCHARDET is normally skipped when a UTF-8/UTF-16 BOM or strong BOM-less UTF-16 evidence is present. Forced detection and developer diagnostics can still request analysis. It is also skipped for pure 7-bit ASCII unless ESC or the HZ `~{` marker suggests a stateful encoding, or developer diagnostics are enabled.
4. Unless an encoding is explicitly forced, final selection prefers a UTF-8 signature, UTF-16 BOM, or strong validated UTF-16 evidence over UCHARDET. Diagnostics and forced auto-detection may run UCHARDET without overriding these decisions. Otherwise a reliable UCHARDET result can win, except that valid UTF-8 is preferred when detector confidence is below `MIN_CONFIDENCE_ANSI_NOT_UTF8`. Weaker Unicode structural evidence is consulted when the detector result is not reliable. Remaining cases use valid UTF-8, configured defaults, weak hints, or ANSI fallback according to settings.

`Settings.AnalyzeReliableConfidenceLevel` determines whether UCHARDET's adjusted confidence is considered reliable (inclusive threshold, default 60%). Explicit configured values and the existing legacy-setting migration remain respected. `Encoding_AnalyzeText()` applies `Settings2.UchardetLanguageFilter`, the supplied encoding hint, and `Settings2.LocaleAnsiCodePageAnalysisBonus` when deriving the analysis confidence.

For a valid candidate matching either the encoding hint or locale ANSI, apply one preference boost: `adjusted = raw + (1 - raw) * weight`. Matching both does not stack bonuses. The existing `Settings2.LocaleAnsiCodePageAnalysisBonus` INI key controls this combined preference weight (0-100, default 20); explicit existing values are retained. A raw 50% becomes 60% with the default weight, meeting the inclusive default 60% reliability threshold. This is a preference adjustment, not a calibrated probability.

## UTF-8 validation and NUL bytes

The UTF-8 scan records strict validity, pure ASCII, NUL presence, and whether the bytes are valid UTF-8 except for NUL bytes. This lets a file containing otherwise-valid UTF-8 with a stray NUL avoid being decoded as UTF-16. Developer title diagnostics distinguish `U8=valid+NUL` from `U8=invalid`.

The distinction matters because an ordinary NUL-rejecting UTF-8 check would send such a file down an incorrect Unicode fallback path, potentially producing a large amount of garbage text and slow loading.

The structural UTF-8 fallback accepts `bUTF8WithNulls` even when UCHARDET confidence is below the reliability threshold, including when the configured default is UTF-8 and `UseDefaultForFileEncoding` is enabled. Strong validated UTF-16 evidence still takes precedence; NUL presence alone does not decide the encoding.

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

## UTF-16 structural detection

BOM-less UTF-16 is inferred from null bytes concentrated at one byte position within 16-bit units: odd positions suggest little-endian; even positions suggest big-endian. `DetectUTF16ByNullDistribution()` samples up to the first 4096 bytes and tolerates opposite-position NULs up to one thirty-second of the dominant count. Strong evidence requires dominant-position NULs in at least 10% of sampled units, not zero opposite-position NULs.

`IsValidUTF16()` checks surrogate pairing across the entire candidate input without mutating it. As before, the distribution path ignores a trailing incomplete byte; incomplete surrogate pairs are rejected.

When distribution evidence is not strong, `DetectUTF16ByWindows()` uses `IsTextUnicode()` on at most 4096 bytes. It rejects illegal-character flags and ambiguous byte-order evidence, then validates the proposed order across the whole input. This fallback requires even length and NUL-containing data that is not otherwise-valid UTF-8 with NULs. It may confirm a weak distribution candidate but cannot reverse it. Successful fallback evidence is treated as strong in final selection.

These are heuristics: binary data or text without recognizable byte-order evidence can remain ambiguous. A BOM remains the explicit signal.

### Regression coverage

Run `test\test_files\encoding\TestEncodingDetection.ps1` with Visual C++ build tools installed. The runner and its `TestEncodingDetection.cpp` harness live alongside the `UTF-16`, `UTF-32`, and `UTF-8` fixture directories. The runner defaults to `UTF-16`; use `-FixtureDirectory` to supply another directory containing the two BOM-less UTF-16 fixtures. Missing fixtures fail the run rather than silently skipping coverage.

It compiles the current detector, confidence adjustment, title formatter, selection helpers, and validators into a temporary native harness. Application state and the low-level UCHARDET call are stubbed; the latter supplies the legacy-encoding result that previously overrode correct UTF-16 evidence. This covers both endiannesses, surrogate validity beyond the sample, sparse-NUL Windows fallback, odd length, BOMs, UTF-8 with NULs, explicit overrides, skip-Unicode mode, diagnostics, and forced auto-detection. The BOM-less UTF-16 fixtures under `test\test_files\encoding\UTF-16` are also checked.

Override tests cover UTF-32 early returns, physical versus requested BOMs, conflicting UTF-16 byte order, file tags, NFO/OEM, and MRU-style weak hints competing with reliable/unreliable UCD results. The harness does not execute `EditLoadFile()` or the real UCD language models; use a full build and editor-load checks for those boundaries.

For cross-process editor-load checks, `WM_GETTEXTW` retrieves UTF-16 text but Windows marshalling stops copying at the first NUL. Validate embedded-NUL documents with pointer-free `SCI_GETLENGTH`/`SCI_GETCHARAT` calls instead; a zero-filled suffix in a marshalled `WM_GETTEXTW` buffer is not evidence that the document lost its suffix.

The reported fixtures have 289 UTF-16 code units, 220 dominant-position NULs, and two opposite-position NULs from valid surrogate units 0xDE00 and 0xDC00. Requiring zero opposite-position NULs previously disabled strong classification. Bundled UCHARDET reports Windows-1252 at 50% for both; the combined default preference weight raises that to 60% on a Windows-1252 system (previously, stacked hint and ANSI bonuses raised it to 83.25%). This must not override the validated UTF-16 evidence. UCHARDET has no BOM-less UTF-16 prober; its UTF-16 recognition in `nsUniversalDetector::HandleData()` is BOM-based.

The sibling `..\notepad4\src\EditEncoding.cpp` also combines Windows flag inspection with strict UTF-16 validation. Do not rely on the Boolean `IsTextUnicode()` return: it is false for the BE fixture even though useful reverse-order flags are set.

## UTF-32 handling

UTF-32 LE and BE BOMs are recognized before ordinary automatic encoding detection, but never override an enforced encoding. BOM-less UTF-32 also respects enforced encodings, including during forced auto-detection. It is considered only when Unicode detection is enabled, there is no BOM or UTF-8 signature, NUL bytes are present, and the inspected data has a length divisible by four. The range check examines up to 4096 bytes and rejects all-zero or endian-ambiguous samples.

UTF-32 is not retained as an editor encoding. `EditLoadFile()` converts complete 32-bit units to UTF-8, replacing invalid scalar values with U+FFFD and setting the Unicode-error status. Trailing bytes that do not form a complete unit are ignored and also set that status. Allocation or conversion failures fail the load instead of reporting a successful empty document. Converted NUL characters propagate the UTF-8/NUL and potentially-binary status flags. A BOM-bearing UTF-32 file becomes UTF-8 with signature; BOM-less input becomes UTF-8. The user is warned about the conversion, and saving writes UTF-8.

## UCHARDET sampling

For files up to 400,000 bytes, the detector passes at most the first 200,000 bytes to UCHARDET (the whole file when it is smaller). Larger files are sampled from the head (up to 100,000 bytes), middle (up to 50,000 bytes), and tail (up to 50,000 bytes). Sample boundaries are adjusted toward nearby line breaks without exceeding the 200,000-byte budget. If sample allocation fails, analysis uses the bounded prefix instead.

This improves detection when informative non-ASCII text appears later in a large file; it remains sampling, not a scan of every byte by UCHARDET. Pure ASCII can bypass UCHARDET as described above.

Boundaries without nearby line breaks can still split multibyte characters or stateful escape sequences. UTF-32 range evidence likewise validates only a prefix, not the entire file; its range check does not exclude surrogate scalars, which the loader replaces. Neither sample is a whole-file validity guarantee.

## Implementation constraints

- `IsTextUnicode()` supplies fallback evidence, not validation. Preserve flag inspection, strict surrogate validation, UTF-8/NUL guards, and final-selection precedence when changing this path.
- `bHasBOM`/`bIsUTF8Sig` describe input bytes, not a requested save format. Never manufacture a signature flag from a forced BOM-bearing encoding: the loader uses these flags to skip bytes. For forced UTF-16, only a physical BOM matching the selected byte order is consumed.
- The detector returns an `ENC_DET_T` value rather than storing the result in a shared result global. The routine still reads application settings and updates current-file-variable state; this alone does not make the overall load path thread-safe.
- Changes to precedence or thresholds can change how ambiguous BOM-less files open. Cover meaningful edge cases and verify behavior at the `EditLoadFile()` boundary.
