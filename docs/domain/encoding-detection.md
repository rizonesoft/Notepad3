# Encoding Detection

Encoding detection is implemented in `src\uchardet\EncodingDetection.cpp` and is called from `EditLoadFile()` in `src\Edit.c`. It combines explicit declarations, byte signatures, structural checks, and UCHARDET analysis; no single heuristic is authoritative for every input.

## Detection flow

1. UTF-32 BOMs are checked first and immediately select the UTF-32 conversion path. Otherwise the detector checks for UTF-16 BOMs and a UTF-8 signature, examines file variables and forced-encoding settings, and scans the input for strict UTF-8 validity, ASCII-only content, and NUL bytes.
2. When Unicode detection is enabled and encoding is not enforced, BOM-less UTF-32 is checked before BOM-less UTF-16. UTF-16 uses null-byte distribution; UTF-32 uses code-point range checks. These are structural heuristics, not full validation of every Unicode sequence.
3. When ANSI code-page detection is enabled, UCHARDET is normally skipped when a UTF-8/UTF-16 BOM or strong BOM-less UTF-16 evidence is present. Forced detection and developer diagnostics can still request analysis. It is also skipped for pure 7-bit ASCII unless ESC or the HZ `~{` marker suggests a stateful encoding, or developer diagnostics are enabled.
4. Unless an encoding is forced, the resulting encoding prefers a UTF-8 signature or UTF-16 BOM. Otherwise a reliable UCHARDET result can win, except that valid UTF-8 is preferred when detector confidence is below `MIN_CONFIDENCE_ANSI_NOT_UTF8`. Unicode structural evidence is consulted when the detector result is not reliable. Remaining cases use valid UTF-8, configured defaults, weak hints, or ANSI fallback according to settings.

`Settings.AnalyzeReliableConfidenceLevel` determines whether UCHARDET's result is considered reliable. `Encoding_AnalyzeText()` applies `Settings2.UchardetLanguageFilter`, the supplied encoding hint, and `Settings2.LocaleAnsiCodePageAnalysisBonus` when deriving the analysis confidence.

## UTF-8 validation and NUL bytes

The UTF-8 scan records strict validity, pure ASCII, NUL presence, and whether the bytes are valid UTF-8 except for NUL bytes. This lets a file containing otherwise-valid UTF-8 with a stray NUL avoid being decoded as UTF-16. Developer title diagnostics distinguish `[UTF-8 with NUL bytes]` from `[Invalid UTF-8]`.

The distinction matters because an ordinary NUL-rejecting UTF-8 check would send such a file down an incorrect Unicode fallback path, potentially producing a large amount of garbage text and slow loading.

## UTF-16 structural detection

BOM-less UTF-16 is inferred from null bytes concentrated at one byte position within 16-bit units: odd positions suggest little-endian; even positions suggest big-endian. Detection samples up to the first 4096 bytes, ignores a trailing odd byte, and tolerates a small number of nulls at the opposite position. Strong evidence requires ASCII-heavy units with no opposite-position nulls; normal analysis skips UCHARDET in that case. Forced detection and developer diagnostics can still run UCHARDET, so those modes do not guarantee that the structural guess wins.

This is a heuristic: text with few NULs, binary data with a similar distribution, or unusual UTF-16 content can be ambiguous. A BOM remains the explicit signal.

## UTF-32 handling

UTF-32 LE and BE BOMs are recognized before ordinary encoding detection. BOM-less UTF-32 is considered only when Unicode detection is enabled, there is no BOM or UTF-8 signature, NUL bytes are present, and the inspected data has a length divisible by four. The range check examines up to 4096 bytes and rejects all-zero or endian-ambiguous samples.

UTF-32 is not retained as an editor encoding. `EditLoadFile()` converts complete 32-bit units to UTF-8, replacing invalid scalar values with U+FFFD and setting the Unicode-error status. Any trailing bytes that do not form a complete unit are ignored. A BOM-bearing UTF-32 file becomes UTF-8 with signature; BOM-less input becomes UTF-8. The user is warned about the conversion, and saving writes UTF-8.

## UCHARDET sampling

For files up to 400,000 bytes, the detector passes at most the first 200,000 bytes to UCHARDET (the whole file when it is smaller). Larger files are sampled from the head (about 100,000 bytes), middle (about 50,000 bytes), and tail (about 50,000 bytes). Sample boundaries are adjusted toward nearby line breaks, so the actual sample can be slightly larger or smaller.

This improves detection when informative non-ASCII text appears later in a large file; it remains sampling, not a scan of every byte by UCHARDET. Pure ASCII can bypass UCHARDET as described above.

## Implementation constraints

- `IsTextUnicode()` is not used for encoding detection: its result is unreliable for CJK-heavy text with few ASCII characters.
- The detector returns an `ENC_DET_T` value rather than storing the result in a shared result global. The routine still reads application settings and updates current-file-variable state; this alone does not make the overall load path thread-safe.
- Changes to precedence or thresholds can change how ambiguous BOM-less files open. Cover meaningful edge cases and verify behavior at the `EditLoadFile()` boundary.
