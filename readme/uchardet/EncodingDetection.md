# Encoding Detection

Notepad3 automatically detects the character encoding of every file it opens. This detection is powered by Mozilla's [uchardet](https://www.freedesktop.org/wiki/Software/uchardet/) library (Universal Charset Detection), layered with additional heuristics for Unicode formats, BOM signatures, and file variable tags.

| | |
|---|---|
| **Detection engine** | [uchardet](https://www.freedesktop.org/wiki/Software/uchardet/) 0.0.8 (Mozilla Universal Charset Detection) |
| **Supported encodings** | 85+ encodings across 40+ languages |
| **Menu** | File → Encoding → Default… |
| **Re-open with encoding** | File → Encoding → Re-open file with Encoding… |
| **Status bar** | Displays the document encoding; `+NUL` marks UTF-8 containing NUL bytes |

---

## The Fundamental Problem

A text file is just a sequence of bytes. **There is no reliable way to determine — with certainty — what encoding was used to create an arbitrary byte sequence.** This is a fundamental limitation, not a bug in any specific software.

Consider the byte sequence `C4 42`. Depending on the encoding, this means:

| Encoding | Interpretation |
|----------|---------------|
| Windows-1252 (Western European) | ÄB |
| Mac Roman | ƒB |
| GB18030 (Chinese Simplified) | 腂 |

All three interpretations are equally valid at the byte level. The bytes themselves carry no metadata about which encoding produced them. This is fundamentally different from Unicode formats like UTF-8, which have recognizable byte patterns, or files with a BOM (Byte Order Mark), which include an explicit encoding signature.

> **The golden rule:** The only way to be *certain* about a file's encoding is to **know** it — through a BOM, a file header, a metadata tag (like Emacs file variables), or an explicit user choice. Everything else is an educated guess.

For a deeper exploration of encoding fundamentals, see David C. Zentgraf's excellent article *[What Every Programmer Absolutely, Positively Needs to Know About Encodings and Character Sets to Work with Text](https://kunststube.net/encoding/)*.

### Why Unicode Simplifies Things

Unicode (UTF-8, UTF-16, UTF-32) was created to end the "encoding tower of Babel." UTF-8 in particular has become the de-facto standard for text interchange because:

- It is **backward-compatible with ASCII** — any pure ASCII file is also valid UTF-8.
- It has **distinctive byte patterns** — multi-byte UTF-8 sequences use specific bit prefixes (`110xxxxx 10xxxxxx` for 2-byte, `1110xxxx 10xxxxxx 10xxxxxx` for 3-byte, etc.) that make structural validation possible.
- It can represent **every Unicode character** — over 1.1 million code points covering virtually all writing systems.

Notepad3 defaults to UTF-8 for new files and strongly prefers UTF-8 when the byte sequence is structurally valid UTF-8 — because this is almost always the right choice in modern environments.

---

## How uchardet Works

uchardet (Universal Charset Detection) is a character encoding detector originally developed by Mozilla for the Firefox browser. It uses **statistical language models** — essentially a form of machine learning — to guess which encoding best explains a given byte sequence.

### The Detection Approach

uchardet does **not** simply check whether bytes are "valid" in an encoding. Instead, it uses a multi-stage process:

1. **Byte-sequence analysis** — The raw byte distribution is examined. Different encodings produce different statistical fingerprints. For example, Shift-JIS files have characteristic lead-byte/trail-byte patterns, while Windows-1251 (Cyrillic) has a different distribution in the 0x80–0xFF range.

2. **Character frequency models** — For each candidate encoding, uchardet decodes the bytes into characters and compares the character frequency distribution against pre-trained language models. For example, in a Russian text encoded as Windows-1251, the decoded Cyrillic characters should follow known letter-frequency patterns (with "о", "е", "а" being common and "щ", "э", "ъ" being rare).

3. **Sequence analysis** — Beyond individual character frequencies, uchardet examines **character bigrams** (two-character sequences). Some character pairs are common in a given language while others almost never occur. This provides a much stronger signal than single-character frequencies alone.

4. **Confidence scoring** — Each candidate encoding-language pair receives a confidence score from 0.0 to 1.0, representing how well the statistical model matches the input data. The encoding with the highest confidence wins.

### The Confidence Score

The confidence score (0–100% in Notepad3's UI) reflects how strongly the statistical model supports the detected encoding:

| Confidence range | Interpretation |
|-----------------|----------------|
| **90–100%** | Strong model match — not a guarantee of the correct encoding |
| **70–89%** | Good model match — still dependent on the input and competing evidence |
| **50–69%** | Moderate match — plausible but uncertain |
| **Below 50%** | Weak match — the detector is essentially guessing |

These scores are model-specific indicators, **not calibrated probabilities**. Notepad3 uses **`AnalyzeReliableConfidenceLevel`** (default: **60%**) as an inclusive threshold on the *adjusted* confidence. Below it, other heuristics and fallbacks are used. A reliable result still cannot override a BOM, an explicit encoding choice, or strong validated UTF-16 evidence.

For a valid candidate matching either the supplied encoding hint or the system ANSI encoding, Notepad3 applies **one non-stacking preference bonus**:

```text
adjusted = raw + (1 - raw) × weight
```

The weight is controlled by `LocaleAnsiCodePageAnalysisBonus` (default: **20%**). Matching both preferences applies the bonus only once. For example, raw confidence of 50% becomes 60%, meeting the default threshold; raw 80% becomes 84%. The bonus is a fraction of the remaining uncertainty, not a fixed addition of 20 percentage points. Set it to zero to disable both hint and locale boosts. Explicitly configured values are retained when defaults change.

---

## Why Detection Can Fail

### Short Documents

Encoding detection is a **statistical** process. The more text available, the more character frequencies and bigram patterns the detector can analyze, and the more accurate the result. With very short documents — especially 2–3 characters — there is simply **not enough data** to build a meaningful statistical profile.

Consider a file containing just two bytes: `B0 A1`. This sequence is valid in:

| Encoding | Character |
|----------|-----------|
| GB2312 (Chinese Simplified) | 啊 |
| EUC-KR (Korean) | 가 |
| Big5 (Chinese Traditional) | 陰 |
| Windows-1252 + ISO-8859-1 | °¡ |

With only two bytes, the detector has no character bigram statistics to work with, no frequency distribution to compare against language models, and no structural clues beyond raw byte validity. The result is essentially random among the candidate encodings.

**Rule of thumb:** Encoding detection becomes reasonably reliable only with **several hundred bytes or more** of text. Files shorter than ~100 bytes may produce unreliable results, and files under ~20 bytes are practically indeterminate.

### CJK Encoding Ambiguity

Chinese, Japanese, and Korean (CJK) encodings are particularly prone to misdetection because:

1. **Overlapping byte ranges** — GB2312/GBK (Chinese Simplified), Big5 (Chinese Traditional), Shift-JIS (Japanese), and EUC-KR (Korean) all use similar double-byte structures in the 0x81–0xFE lead-byte range. A valid byte sequence in one CJK encoding is often also valid in another.

2. **Shared character sets** — CJK languages share thousands of Han (漢字/汉字) characters. A sequence of common Kanji characters could be equally valid Chinese or Japanese text. Even with character frequency analysis, distinguishing between Chinese and Japanese requires substantial amounts of text.

3. **UTF-8 overlap** — Short CJK text in legacy encodings (2–6 bytes) can sometimes coincidentally form valid UTF-8 sequences, leading the detector to prefer UTF-8 over the correct legacy encoding.

4. **Multiple encodings per language** — Chinese alone has GB2312, GBK (CP936), GB18030, Big5, Big5-HKSCS, HZ-GB-2312, and ISO-2022-CN. Japanese has Shift-JIS (CP932), EUC-JP, and ISO-2022-JP. The detector must distinguish not just the language but the specific encoding variant.

#### Example: GB18030 vs. EUC-JP

A common misdetection is GB18030 (Chinese Simplified) being identified as EUC-JP (Japanese) or vice versa. Both encodings use overlapping byte ranges, and certain Chinese character sequences happen to be valid (though nonsensical) EUC-JP. uchardet's language-frequency models can resolve this with enough text, but for short documents the overlap in valid byte ranges dominates.

Notepad3 provides the **`UchardetLanguageFilter`** setting (see [CJK Language Filter](#cjk-language-filter-settings2) below) to let users restrict which CJK probers are active, eliminating impossible candidates based on the user's knowledge of their files' languages.

### ANSI / Single-Byte Ambiguity

The single-byte (8-bit) encodings used for European languages (Windows-1250 through Windows-1258, ISO-8859-x, etc.) all share the ASCII range (0x00–0x7F) and only differ in the upper half (0x80–0xFF). A file containing mostly ASCII with a few accented characters provides very little statistical signal for the detector.

For example, the byte `0xE9` is:
- **é** in Windows-1252 (Western European), ISO-8859-1, ISO-8859-15
- **щ** in Windows-1251 (Cyrillic)
- **ę** in Windows-1250 (Central European)
- **ω** in ISO-8859-7 (Greek)

Without substantial surrounding context, the detector cannot reliably distinguish between these encodings. **`LocaleAnsiCodePageAnalysisBonus`** supplies a modest preference for a result matching the system ANSI code page or the encoding hint. These preferences are not independent evidence and do not stack.

---

## Notepad3's Detection Pipeline

When Notepad3 opens a file, it combines signatures, structural evidence, explicit choices, and statistical analysis. The ordinary load path is:

```
File opened
  │
  ▼
1. Forced encoding checks
  │  ├── .nfo/.diz file + "Open NFO as DOS" enabled?  → Configured OEM encoding
  │  ├── User forced encoding (File → Encoding → Re-open…)?  → Use it
  │  └── Emacs/Vim file variable tag (e.g. -*- coding: utf-8 -*-)?  → Use it
  ▼
2. UTF-32 BOM check (only without an enforced encoding)
  │  Found? → Convert UTF-32 LE/BE to UTF-8; warn that saving writes UTF-8
  ▼
3. BOM / Signature detection
  │  ├── UTF-8 BOM (EF BB BF)?  → UTF-8-BOM
  │  ├── UTF-16 LE BOM (FF FE)?  → UTF-16 LE
  │  └── UTF-16 BE BOM (FE FF)?  → UTF-16 BE
  ▼
4. UTF-8 structural validation
  │  One scan records UTF-8 validity, pure ASCII, and NUL presence
  │  NUL is valid U+0000; valid UTF-8 with NULs is recorded separately
  ▼
5. BOM-less Unicode checks (when enabled and no encoding is enforced)
  │  ├── UTF-32: sampled code-point range and byte-order checks
  │  └── UTF-16: NUL distribution + guarded IsTextUnicode fallback
  │        Validate surrogate pairing across the proposed UTF-16 input
  ▼
6. uchardet statistical analysis  ← main detection engine
  │  Returns: encoding name + confidence score (0.0–1.0)
  │  Normally skipped for a BOM or strong UTF-16 evidence
  │  Pure ASCII also bypasses it unless stateful encoding markers are present
  │  Apply one hint/locale preference bonus, not two
  ▼
7. Decision logic
  │  ├── Explicit choice / enabled file tag?  → Preserve enforced encoding
  │  ├── UTF-8 signature / UTF-16 BOM?  → Use signed encoding
  │  ├── Strong validated UTF-16 evidence?  → Use UTF-16 LE/BE
  │  ├── Reliable UCD result (adjusted confidence ≥ threshold)?  → Use it
  │  │     Exception: if data is valid UTF-8 AND confidence < 99.5%  → Prefer UTF-8
  │  ├── Weaker Unicode evidence?  → Use Unicode candidate
  │  ├── Configured fallback / valid UTF-8 (including NULs) / weak hint
  │  ├── Pure ASCII (all bytes 0x01–0x7F)?
  │  │     └── "Load ASCII as UTF-8" enabled?  → UTF-8, else → ANSI default
  │  └── Nothing matched?  → ANSI default (system code page)
  ▼
Result: encoding used to decode and display the file
```

Developer diagnostics and forced auto-detection can run uchardet despite a BOM or strong UTF-16 evidence, but do not override those decisions or an explicit encoding choice. Ordinary ASCII bypass excludes text containing ESC or HZ `~{` markers, so stateful 7-bit encodings can still be analyzed.

Selecting a BOM-bearing encoding does not imply that the input actually has a BOM. Only a physical matching signature is removed; requesting UTF-8-BOM or UTF-16 with BOM must not discard the first characters of an unsigned file. Explicit encoding choices also take precedence over UTF-32 import detection.

### BOM-less UTF-16

The bundled uchardet recognizes UTF-16 BOMs but has **no BOM-less UTF-16 prober**. Notepad3 supplies its own detection:

- Sample up to the first 4096 bytes for NUL concentration at even versus odd byte positions. Small numbers of opposite-position NULs are tolerated, including zero low bytes in valid surrogate pairs.
- Validate surrogate pairing throughout the candidate input. Strong NUL-pattern evidence takes precedence over statistical legacy-encoding guesses.
- If necessary, inspect Windows `IsTextUnicode()` flags and validate the proposed byte order. Its Boolean return alone is insufficient for some big-endian files. The fallback is guarded to avoid reclassifying otherwise-valid UTF-8 with stray NULs.

The distribution path ignores a trailing incomplete byte; incomplete surrogate pairs are rejected. These remain heuristics: some BOM-less text and binary data are inherently ambiguous.

### UTF-32 Conversion

UTF-32 LE/BE is recognized by BOM or, when Unicode detection is enabled, a BOM-less range/byte-order heuristic sampling up to 4096 bytes. BOM-less input must have a byte count divisible by four; all-zero and endian-ambiguous samples are rejected.

UTF-32 is **converted to UTF-8 on load**, not retained as a save encoding. A UTF-32 BOM becomes a UTF-8 BOM; BOM-less input becomes UTF-8 without BOM. A warning explains that saving writes UTF-8. Invalid scalar values become U+FFFD; incomplete trailing units are ignored. Both cases set the Unicode-error status. Converted NULs retain the UTF-8 `+NUL`/potentially-binary status.

### Analysis Sampling and File Tags

For files up to 400,000 bytes, uchardet receives at most the first 200,000 bytes. Larger files use head, middle, and tail samples of about 100,000, 50,000, and 50,000 bytes, with boundaries adjusted toward nearby line breaks. This improves detection when characteristic text occurs late, but is not a full-file statistical analysis.

File-variable scanning replaces NUL bytes with spaces in its temporary scan buffer so a stray NUL does not hide later tags. BOM recognition uses the real file head, including when tags are scanned near the end. It does not modify the document bytes.

### Key Design Decision: UTF-8 Preference

Notepad3 **strongly prefers UTF-8** over statistical ANSI guesses. In the reliable-UCD branch, valid UTF-8 without NULs wins when adjusted confidence is below **99.5%**. BOMs, enforced encodings, and strong validated UTF-16 evidence have higher priority. When analysis is unreliable, valid UTF-8 (including UTF-8 with NULs) can supply the structural fallback, subject to configured fallback preferences.

UTF-8 bytes are never decoded as UTF-16 merely because UTF-8 validation failed or Unicode detection was skipped; that load path uses ANSI conversion instead. UTF-8 with NULs is marked `+NUL` in the status bar and treated as potentially binary for indentation and inconsistent-EOL checks.

This preference is deliberate: in modern computing, the vast majority of new text files are UTF-8, and many legacy ANSI files happen to also be valid UTF-8 (because they contain only ASCII characters, or their high-byte sequences coincidentally form valid UTF-8). Choosing UTF-8 in ambiguous cases produces the correct result far more often than choosing a legacy encoding. For a comprehensive argument on why UTF-8 should be the default encoding everywhere, see the [UTF-8 Everywhere Manifesto](https://utf8everywhere.org/).

---

## Configuration

### Encoding Dialog (File → Encoding → Default…)

The Encoding dialog configures Notepad3's detection behavior:

| Control | Description |
|---------|-------------|
| **Default Encoding** | Encoding used for new files and as a fallback. Default: UTF-8 |
| **Use as fallback on detection failure** | When checked, the default encoding is used instead of UTF-8 when detection produces no result. INI: `UseDefaultForFileEncoding` |
| **Open 7-bit ASCII files in UTF-8 mode** | Pure ASCII files (bytes 0x01–0x7F, no NULs) are treated as UTF-8 rather than ANSI. INI: `LoadASCIIasUTF8` |
| **Open 8-bit .nfo/diz files in DOS-437 mode** | Forces `.nfo` and `.diz` files to DOS code page 437 (for ANSI art). INI: `LoadNFOasOEM` |
| **Parse encoding file tags** | Reads encoding from Emacs/Vim modelines (e.g., `coding: utf-8`). INI: `NoEncodingTags` (inverted) |
| **Perform ANSI Code Page analysis** | Enables uchardet detection for non-Unicode encodings. INI: `SkipANSICodePageDetection` (inverted) |
| **Confidence level** (spin control) | Minimum adjusted confidence (0–100%) for a reliable uchardet result. INI: `AnalyzeReliableConfidenceLevel` (default: 60) |
| **Enable UNICODE detection** | Enables BOM-less UTF-16/UTF-32 checks, including the guarded Windows fallback. INI: `SkipUnicodeDetection` (inverted) |

### INI Settings Reference

#### `[Settings]` — Encoding Detection

| Key | Default | Range | Description |
|-----|---------|-------|-------------|
| `DefaultEncoding` | UTF-8 | Any supported encoding | Encoding for new files and fallback |
| `UseDefaultForFileEncoding` | `false` | bool | Use default encoding as fallback on detection failure |
| `LoadASCIIasUTF8` | `true`* | bool | Treat pure-ASCII files as UTF-8 |
| `SkipANSICodePageDetection` | `false`* | bool | Skip uchardet analysis (disables ANSI detection) |
| `AnalyzeReliableConfidenceLevel` | `60` | 0–100 | Inclusive minimum adjusted confidence % for a reliable uchardet result |
| `SkipUnicodeDetection` | `false` | bool | Skip BOM-less Unicode structural detection |

\* Default depends on `DefaultEncoding`: if UTF-8, then `LoadASCIIasUTF8=true` and `SkipANSICodePageDetection=false`. If a non-UTF-8 default, these are inverted.

#### `[Settings2]` — Advanced Encoding Tuning

| Key | Default | Range | Description |
|-----|---------|-------|-------------|
| `LocaleAnsiCodePageAnalysisBonus` | `20` | 0–100 | One combined hint/locale preference weight on remaining uncertainty. Matching both does not stack; 0 disables the boost. |
| `UchardetLanguageFilter` | `31` | 0–31 | Bitmask controlling which CJK charset probers are active (see below) |

### CJK Language Filter (`[Settings2]`)

The `UchardetLanguageFilter` setting is a bitmask that controls which CJK encoding probers are active in uchardet. This is the primary tool for resolving CJK misdetection issues.

| Bit | Value | Probers enabled |
|-----|-------|----------------|
| 0 | 1 | Chinese Simplified (GB18030, GB2312, HZ-GB-2312) |
| 1 | 2 | Chinese Traditional (Big5, EUC-TW) |
| 2 | 4 | Japanese (Shift-JIS, EUC-JP, ISO-2022-JP) |
| 3 | 8 | Korean (EUC-KR, ISO-2022-KR) |
| 4 | 16 | Non-CJK (all single-byte encodings) |

#### Common Configurations

| Value | Active probers | Use case |
|-------|---------------|----------|
| **31** | All (default) | General-purpose — all languages enabled |
| **27** | All except Japanese | Fix GB18030 files misdetected as EUC-JP |
| **17** | Chinese Simplified + Non-CJK | System primarily handles Simplified Chinese files |
| **3** | Chinese only (Simplified + Traditional) | System works exclusively with Chinese text |
| **19** | Chinese + Non-CJK | Chinese files in a Western environment |
| **16** | Non-CJK only | Disable all CJK probers (European-language system) |

> **Tip:** If you consistently work with files in a specific CJK language and experience misdetection, restrict the filter to only the languages you actually use. Eliminating impossible candidates dramatically improves accuracy.

---

## Practical Recommendations

### For Best Results

1. **Use UTF-8 with BOM for important files** — The UTF-8 BOM (`EF BB BF`) eliminates all ambiguity. Notepad3 supports saving as "UTF-8-BOM" explicitly.

2. **Use Emacs/Vim file variables** — Adding a line like `# -*- coding: utf-8 -*-` or `// vim: set fileencoding=utf-8 :` to the top of a file provides a reliable encoding declaration that overrides statistical detection.

3. **Start with the 60% confidence threshold and 20% preference weight** — These are conservative tuning defaults, not guarantees. Raising the threshold increases fallback use; lowering it accepts weaker guesses. Existing explicit INI values are not automatically replaced by new defaults.

4. **Use the CJK language filter** — If you know which CJK languages your files use, set `UchardetLanguageFilter` accordingly. This is the single most effective setting for resolving CJK misdetection.

5. **For short files, specify encoding explicitly** — If you routinely work with very short files in a non-UTF-8 encoding, use **File → Encoding → Re-open file with Encoding…** to manually select the correct encoding, or add a file variable tag.

### When Detection Goes Wrong

If a file opens with garbled text (mojibake):

1. **Don't save** — Saving the garbled text would permanently corrupt the file content.
2. **Re-open with the correct encoding** — Use **File → Encoding → Re-open file with Encoding…** (`Ctrl+Shift+A`) and select the encoding you believe is correct.
3. **If you don't know the encoding** — Try the most likely candidates for your region:
   - Western European: Windows-1252, ISO-8859-1, ISO-8859-15
   - Central European: Windows-1250, ISO-8859-2
   - Cyrillic: Windows-1251, KOI8-R, ISO-8859-5
   - Chinese Simplified: GB18030, GBK (CP936), GB2312
   - Chinese Traditional: Big5
   - Japanese: Shift-JIS (CP932), EUC-JP
   - Korean: EUC-KR (CP949)
4. **Save as UTF-8** — Once the text displays correctly, save the file as UTF-8 (or UTF-8-BOM) to avoid the same problem next time.

---

## Understanding Encoding in the Status Bar

Notepad3 displays the document encoding in the status bar. `+NUL` identifies UTF-8 containing NUL bytes; UTF-32 input is shown as its converted UTF-8 encoding.

In **Developer Debug Mode** (`DevDebugMode=1` in `[Settings2]`), the title bar shows the final detection decision and competing evidence:

```text
ENC=UTF-16LE <- UTF-16(NUL-S) | U8=invalid | U16=UTF-16LE/valid | UCD(60.0%)=WINDOWS-1252
```

| Field | Meaning |
|-------|---------|
| `ENC=… <- …` | Final detection encoding and winning source |
| `U8=ASCII/valid/valid+NUL/invalid` | UTF-8 validity of the input bytes, not validity of the selected encoding |
| `U16=…/valid` | UTF-16 candidate with validated surrogate pairing; `/BOM` indicates signature evidence rather than validation |
| `UCD(pp%)=…` | uchardet candidate and adjusted confidence; `none` means no result |
| `UCD=skipped(reason)` | Analysis was skipped, with its reason |

The winning-source abbreviations are:

| Source | Meaning |
|--------|---------|
| `UCD`, `UCD(FORCE)` | uchardet, normal or forced auto-detection |
| `BOM` | Unicode signature |
| `OVR`, `F-TAG`, `NFO/OEM` | Explicit override, file encoding tag, NFO/OEM preference |
| `UTF-8(valid)` | UTF-8 structural validation |
| `UTF-16(NUL-S)`, `UTF-16(NUL-W)` | Strong/weak NUL-pattern evidence with surrogate validation |
| `UTF-16(WIN)`, `UTF-16(FORCE)` | Validated Windows evidence, or Unicode fallback in forced auto-detection |
| `DEF`, `ASCII-PREF`, `HINT`, `ANSI-FB` | Configured default, ASCII preference, weak encoding hint, ANSI fallback |
| `BOM(U32LE->U8)`, `BOM(U32BE->U8)` | UTF-32 BOM detected; converted to UTF-8 |
| `UTF-32LE(range->U8)`, `UTF-32BE(range->U8)` | BOM-less UTF-32 range detection; converted to UTF-8 |

In the example, the UTF-16 structural result won even though uchardet proposed Windows-1252. `U8=invalid` is expected for this valid UTF-16 input. Accepted/rejected/ignored labels are unnecessary because `<-` identifies the winning source. UTF-32 BOM detection reports `U8=not checked`; BOM-less UTF-32 reports the UTF-8 scan that preceded range detection. Percentages follow the application's numeric locale.

Explicit-override information is appended without replacing the pipeline summary. Diagnostics are formatted after final selection; running extra analysis for diagnostics does not override a BOM or strong validated UTF-16 evidence.

---

## Supported Encodings

Notepad3 supports 85+ encodings organized by region:

| Category | Encodings |
|----------|-----------|
| **Unicode** | UTF-8, UTF-8-BOM, UTF-16 LE, UTF-16 BE (with/without BOM), UTF-7 |
| **Western European** | Windows-1252, ISO-8859-1, ISO-8859-15, Mac Roman, DOS-437, DOS-850, DOS-858, DOS-860, DOS-865 |
| **Central European** | Windows-1250, ISO-8859-2, ISO-8859-16, Mac Central Europe, DOS-852 |
| **Cyrillic** | Windows-1251, ISO-8859-5, KOI8-R, KOI8-U, Mac Cyrillic, DOS-855, DOS-866 |
| **Greek** | Windows-1253, ISO-8859-7, Mac Greek, DOS-737, DOS-869 |
| **Turkish** | Windows-1254, ISO-8859-9, Mac Turkish, DOS-857 |
| **Hebrew** | Windows-1255, ISO-8859-8 (visual), ISO-8859-8-I (logical), Mac Hebrew, DOS-862 |
| **Arabic** | Windows-1256, ISO-8859-6, Mac Arabic, DOS-720 |
| **Baltic** | Windows-1257, ISO-8859-4, ISO-8859-13, DOS-775 |
| **Vietnamese** | Windows-1258 (VISCII) |
| **Thai** | Windows-874, TIS-620, Mac Thai |
| **Chinese Simplified** | GBK (CP936), GB2312, GB18030, HZ-GB-2312, Mac Chinese Simplified |
| **Chinese Traditional** | Big5, Big5-HKSCS, EUC-TW, ISO-2022-CN, Mac Chinese Traditional |
| **Japanese** | Shift-JIS (CP932), EUC-JP, ISO-2022-JP, Mac Japanese |
| **Korean** | EUC-KR (CP949, UHC), ISO-2022-KR, Johab, Mac Korean |
| **Nordic** | ISO-8859-10 |
| **Latin-3** | ISO-8859-3 (Esperanto, Maltese, etc.) |
| **Icelandic** | Mac Icelandic, DOS-861 |
| **French Canadian** | DOS-863 |
| **EBCDIC** | IBM EBCDIC US/International/Greek/Latin-5 |

UTF-32 LE/BE is supported as an **import conversion to UTF-8**, not a retained editor/save encoding.

---

## Encoding Detection vs. Encoding Conversion

It is important to understand the difference:

- **Detection** (what uchardet does): Guessing *which* encoding was used to create a byte sequence. This is inherently uncertain for non-Unicode, non-BOM files.
- **Conversion** (what happens when you change encoding): Transforming the bytes from one encoding to another while preserving the *characters*. This is a deterministic operation — once you know the source encoding, conversion is lossless (for characters present in both encodings).

When you choose **File → Encoding → Re-open file with Encoding…**, Notepad3 re-reads the file's bytes and decodes them using the selected encoding. When you choose **File → Encoding → Set Encoding…**, Notepad3 marks the in-memory text to be saved in the new encoding — the display doesn't change, but the bytes written to disk will be different.

---

## Why Perfect Detection Is Impossible

The fundamental mathematical reality is:

1. **Encodings are not self-describing** — Legacy single-byte and CJK multi-byte encodings contain no metadata. The same byte sequence `C0 C1 C2` could be 3 characters in one encoding or 1.5 characters in another.

2. **Many-to-many mapping** — Any given byte sequence is valid in multiple encodings, and any given character can be encoded in multiple ways. For N candidate encodings, a short byte sequence might be valid in all N.

3. **Statistical analysis requires statistical data** — Language models need enough text to observe meaningful patterns. With 2–3 characters, there are no n-gram frequencies to analyze — the sample is too small for any statistical method.

4. **No universal character frequency model** — Even with long text, a file containing unusual or domain-specific content (code, identifiers, binary-adjacent data) may not match any natural-language frequency model.

This is why Notepad3 provides multiple override mechanisms: forced encoding, file variable tags, manual re-open with encoding, and configurable detection parameters. When automatic detection reaches its limits, human knowledge must fill the gap.

---

## Implementation and Regression Coverage

The pipeline is implemented in `src\uchardet\EncodingDetection.cpp`, UTF-8 validation in `src\Encoding.c`, and decoding/conversion in `EditLoadFile()` in `src\Edit.c`. Detection returns a per-call result rather than a shared result global; application settings and current-file-variable state still belong to the surrounding load path, so this is not a promise of concurrent-load thread safety.

Run `test\test_files\encoding\TestEncodingDetection.ps1` with Visual C++ build tools installed. It compiles current detector functions into a temporary native harness; application state and the low-level uchardet call are stubbed. Coverage includes UTF-16 byte order and surrogate validity, Windows fallback, UTF-8 with NULs, overrides, diagnostic formatting, non-stacking preference weighting, and the inclusive confidence threshold. The runner also reads the BOM-less fixtures in the adjacent `UTF-16` directory; `UTF-8` and `UTF-32` fixture directories are alongside it.

See the [encoding implementation reference](../../docs/domain/encoding-detection.md) for exact heuristic constraints and test boundaries.
