# Encoding Detection

Notepad3 automatically detects the character encoding of every file it opens. This detection is powered by Mozilla's [uchardet](https://www.freedesktop.org/wiki/Software/uchardet/) library (Universal Charset Detection), layered with additional heuristics for Unicode formats, BOM signatures, and file variable tags.

| | |
|---|---|
| **Detection engine** | [uchardet](https://www.freedesktop.org/wiki/Software/uchardet/) 0.0.8 (Mozilla Universal Charset Detection) |
| **Supported encodings** | 85+ encodings across 40+ languages |
| **Menu** | File → Encoding → Default… |
| **Re-open with encoding** | File → Encoding → Re-open file with Encoding… |
| **Status bar** | Displays detected encoding name |

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
| **90–100%** | Very strong match — the detection is almost certainly correct |
| **70–89%** | Good match — likely correct for longer documents |
| **50–69%** | Moderate match — plausible but uncertain |
| **Below 50%** | Weak match — the detector is essentially guessing |

Notepad3 uses the **`AnalyzeReliableConfidenceLevel`** setting (default: 50%) as the threshold. If uchardet's confidence is below this threshold, the result is considered *unreliable* and Notepad3 falls back to other heuristics (valid UTF-8 structure, default encoding, etc.).

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

Without substantial surrounding context, the detector cannot reliably distinguish between these encodings. Notepad3 addresses this with the **`LocaleAnsiCodePageAnalysisBonus`** setting, which gives a confidence boost to the system's ANSI code page — on the theory that files on a Western European system are more likely to be Windows-1252 than Windows-1251.

---

## Notepad3's Detection Pipeline

When Notepad3 opens a file, it runs a multi-stage detection pipeline. Each stage either produces a definitive answer or passes to the next:

```
File opened
  │
  ▼
1. UTF-32 BOM check
  │  Found? → UTF-32 (unsupported, opened as binary)
  ▼
2. Forced encoding checks
  │  ├── User forced encoding (File → Encoding → Re-open…)?  → Use it
  │  ├── .nfo/.diz file + "Open NFO as DOS" enabled?  → DOS-437 (OEM)
  │  └── Emacs/Vim file variable tag (e.g. -*- coding: utf-8 -*-)?  → Use it
  ▼
3. BOM / Signature detection
  │  ├── UTF-8 BOM (EF BB BF)?  → UTF-8-BOM
  │  ├── UTF-16 LE BOM (FF FE)?  → UTF-16 LE
  │  └── UTF-16 BE BOM (FE FF)?  → UTF-16 BE
  ▼
4. UTF-8 structural validation
  │  All bytes form valid UTF-8 multi-byte sequences?
  │  (Rejects null bytes — files with 0x00 are not valid UTF-8)
  ▼
5. Unicode heuristic (IsTextUnicode API + null-byte distribution)
  │  Detects BOM-less UTF-16 by analyzing null-byte patterns
  ▼
6. uchardet statistical analysis  ← main detection engine
  │  Returns: encoding name + confidence score (0.0–1.0)
  │  Confidence biased by encoding hint and locale bonus
  ▼
7. Decision logic
  │  ├── Reliable result (confidence ≥ threshold)?  → Use analyzed encoding
  │  │     Exception: if data is valid UTF-8 AND confidence < 99.5%  → Prefer UTF-8
  │  ├── Valid UTF-8 byte structure?  → UTF-8
  │  ├── Pure ASCII (all bytes 0x01–0x7F)?
  │  │     └── "Load ASCII as UTF-8" enabled?  → UTF-8, else → ANSI default
  │  └── Nothing matched?  → ANSI default (system code page)
  ▼
Result: encoding used to decode and display the file
```

### Key Design Decision: UTF-8 Preference

Notepad3 **strongly prefers UTF-8** over detected ANSI encodings. If the file's bytes are structurally valid UTF-8 (all multi-byte sequences follow proper UTF-8 bit patterns), Notepad3 will choose UTF-8 unless uchardet reports a **very high** confidence (≥ 99.5%) for a non-UTF-8 encoding.

This preference is deliberate: in modern computing, the vast majority of new text files are UTF-8, and many legacy ANSI files happen to also be valid UTF-8 (because they contain only ASCII characters, or their high-byte sequences coincidentally form valid UTF-8). Choosing UTF-8 in ambiguous cases produces the correct result far more often than choosing a legacy encoding. For a comprehensive argument on why UTF-8 should be the default encoding everywhere, see the [UTF-8 Everywhere Manifesto](https://utf8everywhere.org/).

---

## Configuration

### Encoding Dialog (File → Encoding → Default…)

The Encoding dialog configures Notepad3's detection behavior:

| Control | Description |
|---------|-------------|
| **Default Encoding** | Encoding used for new files and as a fallback. Default: UTF-8 |
| **Use as fallback on detection failure** | When checked, the default encoding is used instead of UTF-8 when detection produces no result. INI: `UseDefaultForFileEncoding` |
| **Open 7-bit ASCII files in UTF-8 mode** | Pure ASCII files (all bytes ≤ 0x7F) are treated as UTF-8 rather than ANSI. INI: `LoadASCIIasUTF8` |
| **Open 8-bit .nfo/diz files in DOS-437 mode** | Forces `.nfo` and `.diz` files to DOS code page 437 (for ANSI art). INI: `LoadNFOasOEM` |
| **Parse encoding file tags** | Reads encoding from Emacs/Vim modelines (e.g., `coding: utf-8`). INI: `NoEncodingTags` (inverted) |
| **Perform ANSI Code Page analysis** | Enables uchardet detection for non-Unicode encodings. INI: `SkipANSICodePageDetection` (inverted) |
| **Confidence level** (spin control) | Minimum confidence (0–100%) for the uchardet result to be accepted. INI: `AnalyzeReliableConfidenceLevel` (default: 50) |
| **Enable UNICODE detection** | Enables the Windows `IsTextUnicode` heuristic for BOM-less UTF-16. INI: `SkipUnicodeDetection` (inverted) |

### INI Settings Reference

#### `[Settings]` — Encoding Detection

| Key | Default | Range | Description |
|-----|---------|-------|-------------|
| `DefaultEncoding` | UTF-8 | Any supported encoding | Encoding for new files and fallback |
| `UseDefaultForFileEncoding` | `false` | bool | Use default encoding as fallback on detection failure |
| `LoadASCIIasUTF8` | `true`* | bool | Treat pure-ASCII files as UTF-8 |
| `SkipANSICodePageDetection` | `false`* | bool | Skip uchardet analysis (disables ANSI detection) |
| `AnalyzeReliableConfidenceLevel` | `50` | 0–100 | Minimum confidence % for uchardet result to be trusted |

\* Default depends on `DefaultEncoding`: if UTF-8, then `LoadASCIIasUTF8=true` and `SkipANSICodePageDetection=false`. If a non-UTF-8 default, these are inverted.

#### `[Settings2]` — Advanced Encoding Tuning

| Key | Default | Range | Description |
|-----|---------|-------|-------------|
| `LocaleAnsiCodePageAnalysisBonus` | `33` | 0–100 | Confidence bonus (%) for the system's ANSI code page. Set to 0 to disable locale bias. |
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

3. **Keep the confidence threshold at 50%** — The default strikes a good balance. Raising it too high may cause uchardet results to be ignored even when correct; lowering it may accept incorrect guesses.

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

Notepad3 displays the detected encoding in the status bar. In **Developer Debug Mode** (`DevDebugMode=1` in `[Settings2]`), the title bar shows extended detection diagnostics:

```
UCD='encoding-name' Conf=XX.X% Thresh=YY%
```

| Field | Meaning |
|-------|---------|
| `UCD='…'` | Encoding name returned by uchardet |
| `Conf=XX.X%` | uchardet's confidence score |
| `Thresh=YY%` | The `AnalyzeReliableConfidenceLevel` threshold |

This is useful for diagnosing detection issues — you can see exactly what uchardet detected and how confident it was.

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
