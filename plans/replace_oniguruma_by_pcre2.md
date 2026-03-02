# Plan: Replace Oniguruma by PCRE2 in Notepad3/Scintilla

## Background & Motivation

Oniguruma (https://github.com/kkos/oniguruma) was **archived on April 24, 2025** by its
maintainer K. Kosako. The repository is now read-only. No actively maintained community
fork exists. Security vulnerabilities will not be patched.

**PCRE2** (https://github.com/PCRE2Project/pcre2) is the recommended replacement:
actively maintained, C library, UTF-8 native, feature-complete superset of Oniguruma
for Notepad3's use case.

---

## Current Architecture

### Scintilla Regex Engine Interface

**File**: `scintilla/src/Document.h` (lines 107-119)

```cpp
class RegexSearchBase {
public:
    virtual ~RegexSearchBase() = default;
    virtual Sci::Position FindText(Document *doc, Sci::Position minPos, Sci::Position maxPos,
                          const char *s, bool caseSensitive, bool word, bool wordStart,
                          Scintilla::FindOption flags, Sci::Position *length) = 0;
    virtual const char *SubstituteByPosition(Document *doc, const char *text,
                          Sci::Position *length) = 0;
};
extern RegexSearchBase *CreateRegexSearch(CharClassify *charClassTable);
```

### Factory Selection (compile-time)

**File**: `scintilla/src/Document.cxx` (lines 3604-3610)

```cpp
#ifndef SCI_OWNREGEX
RegexSearchBase *Scintilla::Internal::CreateRegexSearch(CharClassify *charClassTable) {
    return new BuiltinRegex(charClassTable);  // fallback: RESearch
}
#endif
```

When `SCI_OWNREGEX` is defined, the factory is provided by the custom engine file instead.

### Current Oniguruma Implementation

**File**: `scintilla/oniguruma/scintilla/OnigurumaRegExEngine.cxx` (904 lines)

This file contains **two classes**:

1. **`OnigurumaRegExEngine`** (line 162-244) — implements `RegexSearchBase` for Scintilla's
   find/replace system. This is the main engine.
2. **`SimpleRegExEngine`** (line 753-805) — lightweight standalone engine used via the
   exported `OnigRegExFind()` function for simple pattern matching outside Scintilla
   (URL detection, lexer file matching).

**Factory** (line 248-251):
```cpp
RegexSearchBase *Scintilla::Internal::CreateRegexSearch(CharClassify *charClassTable) {
    return new OnigurumaRegExEngine(charClassTable);
}
```

**Exported C function** (line 888-901):
```cpp
extern "C" __declspec(dllexport)
ptrdiff_t WINAPI OnigRegExFind(const char *pchPattern, const char *pchText,
                                const bool caseSensitive, const int eolMode, int *matchLen_out);
```

### Callers of OnigRegExFind (outside Scintilla)

**Declaration**: `src/SciCall.h` (line 107-108)
```cpp
ptrdiff_t WINAPI OnigRegExFind(const char *pchPattern, const char *pchText,
                               const bool caseSensitive, const int eolMode, int *matchLen_out);
```

**Call sites**:
- `src/Edit.c:2126` — URL validation via hyperlink regex pattern
- `src/Styles.c:2533` — Lexer file-path pattern matching

Both call sites use forward-only search on small strings (not document buffers).

### Build Configuration

**Two vcxproj files** reference Oniguruma:

#### `scintilla/Scintilla.vcxproj` (static library)
- Preprocessor: `NO_CXX11_REGEX;SCI_OWNREGEX;SCI_EMPTYCATALOGUE;ONIG_STATIC`
- Include dirs: `../oniguruma/src`
- Source files (lines 523-541): 19 Oniguruma `.c` files + `OnigurumaRegExEngine.cxx`
- Header files (lines 587-593): 7 Oniguruma `.h` files

#### `scintilla/ScintillaDLL.vcxproj` (DLL)
- Preprocessor: `NO_CXX11_REGEX;SCI_OWNREGEX;ONIG_EXTERN=extern;SCINTILLA_DLL`
- Include dirs: `../oniguruma/src`
- Source files (lines 227-247): 20 Oniguruma `.c` files + `OnigurumaRegExEngine.cxx`
- Header files (lines 341-347): 7 Oniguruma `.h` files

### Oniguruma Source Files (vendored)

Located in `scintilla/oniguruma/src/`:
```
ascii.c  mktable.c  onig_init.c  regcomp.c  regenc.c  regerror.c
regexec.c  regext.c  reggnu.c  regparse.c  regposix.c  regsyntax.c
regtrav.c  regversion.c  st.c  unicode.c  unicode_egcb_data.c
unicode_fold1_key.c  unicode_fold2_key.c  unicode_fold3_key.c
unicode_fold_data.c  unicode_property_data.c  unicode_property_data_posix.c
unicode_unfold_key.c  unicode_wb_data.c  utf8.c
```

Headers: `config.h  oniggnu.h  oniguruma.h  regenc.h  regint.h  regparse.h  st.h`

---

## Notepad3 Regex Feature Usage (What Must Be Preserved)

### Syntax features exposed to users via find/replace

| Feature | Oniguruma syntax | PCRE2 equivalent | Action needed |
|---------|-----------------|------------------|---------------|
| Basic matching `.` `*` `+` `?` `{n,m}` | Same | Same | None |
| Character classes `[...]` | Same | Same | None |
| POSIX classes `[:alpha:]` | Same | Same | None |
| Alternation `\|` | Same | Same | None |
| Capturing groups `(...)` | Same | Same | None |
| Non-capturing groups `(?:...)` | Same | Same | None |
| Backreferences `\1`..`\99` | Same | Same | None |
| Named groups `(?<name>...)` | Same | Same | None |
| Named groups `(?P<name>...)` | Same | Same | None |
| Lookbehind `(?<=...)` `(?<!...)` | Variable-length | Variable-length (PCRE2 10.23+) | None |
| Lookahead `(?=...)` `(?!...)` | Same | Same | None |
| Possessive quantifiers `*+` `++` | Same | Same | None |
| Non-greedy `*?` `+?` `??` | Same | Same | None |
| Unicode properties `\p{L}` | Same | Same (needs `PCRE2_UCP`) | Set flag |
| `\K` (reset match start) | Same | Same | None |
| `\R` (generic newline) | Same | Same (`PCRE2_BSR_UNICODE`) | Set flag |
| `\X` (grapheme cluster) | Same | Same | None |
| Inline modifiers `(?imsx)` | Same | Same | None |
| Conditional patterns `(?(n)...\|...)` | Same | Same | None |
| `\d` `\w` `\s` `\b` `\B` | Same | Same (Unicode-aware with `PCRE2_UCP`) | Set flag |
| **`\<` `\>`** word boundaries | Oniguruma-specific | **Not supported** | **Translate** |
| **`\h` `\H`** horiz. space | Currently translated to char class | **Native in PCRE2** | **Simplify** |
| **`\uHHHH`** Unicode escape | Oniguruma-specific | `\x{HHHH}` in PCRE2 | **Translate** |

### Replacement string features

The replacement logic is **not** handled by Oniguruma itself — it's custom code in
`SubstituteByPosition()` (lines 458-522). It processes:
- `$1`..`$99` and `\1`..`\9` — numbered group references
- `${name}` and `$+{name}` — named group references (uses `onig_name_to_backref_number()`)
- `$$` and `\\` — literal escapes
- `\n \r \t \a \b \f \v` — escape sequences
- `\xHH` and `\uHHHH` — hex/Unicode escapes (converted to UTF-8 via `WideCharToMultiByte`)

The `convertReplExpr()` method (lines 640-743) converts `\1`..`\9` to `$1`..`$9` format
and processes escape sequences into literal bytes before `SubstituteByPosition()` handles
group substitution.

### Compile options mapping

Current NP3 configuration in `SetSimpleOptions()` (lines 97-158):

| Oniguruma option | PCRE2 equivalent | NP3 default |
|-----------------|------------------|-------------|
| `ONIG_OPTION_IGNORECASE` | `PCRE2_CASELESS` | OFF (case-sensitive) |
| `ONIG_OPTION_MULTILINE` (dot-matches-all) | `PCRE2_DOTALL` | OFF (enabled by `FindOption::DotMatchAll`) |
| `ONIG_OPTION_SINGLELINE` | (none needed) | OFF |
| `ONIG_OPTION_NOT_BEGIN_STRING` | `PCRE2_NOTBOL` | Dynamic (based on range position) |
| `ONIG_OPTION_NOT_END_STRING` | `PCRE2_NOTEOL` | Dynamic (based on range position) |

Custom syntax modifications in constructor (lines 78-88):
- **Removed**: `ONIG_SYN_OP2_ESC_H_XDIGIT` (replaced `\h`/`\H` with custom char classes)
- **Added**: `ONIG_SYN_OP_ESC_LTGT_WORD_BEGIN_END` (enables `\<` `\>`)
- **Added**: `ONIG_SYN_OP2_ESC_U_HEX4` (enables `\uHHHH`)

---

## Implementation Plan

### Step 1: Obtain PCRE2 Sources

Download PCRE2 source release from https://github.com/PCRE2Project/pcre2/releases
(latest stable, currently 10.44+).

Create directory `scintilla/pcre2/` (parallel to existing `scintilla/oniguruma/`).

Minimal files needed for static compilation:
```
pcre2.h              (public API — generated from pcre2.h.in)
pcre2_internal.h
pcre2_intmodedep.h
pcre2_ucp.h
pcre2_chartables.c   (generated or use default)
pcre2_auto_possess.c
pcre2_chkdint.c
pcre2_compile.c
pcre2_config.c
pcre2_context.c
pcre2_dfa_match.c
pcre2_error.c
pcre2_extuni.c
pcre2_find_bracket.c
pcre2_maketables.c
pcre2_match.c
pcre2_match_data.c
pcre2_newline.c
pcre2_ord2utf.c
pcre2_pattern_info.c
pcre2_script_run.c
pcre2_string_utils.c
pcre2_study.c
pcre2_substitute.c
pcre2_substring.c
pcre2_tables.c
pcre2_ucd.c
pcre2_valid_utf.c
pcre2_xclass.c
```

Generate `config.h` and `pcre2.h` for MSVC (or use the CMake config step, or create
manually — PCRE2 provides `pcre2.h.generic` as a starting point).

Key defines for `config.h`:
```c
#define PCRE2_CODE_UNIT_WIDTH 8     // UTF-8 mode
#define HAVE_MEMMOVE 1
#define PCRE2_STATIC 1              // static linking
#define SUPPORT_UNICODE 1           // Unicode properties, scripts, \p{}, \X, etc.
#define SUPPORT_UCP 1               // Unicode character properties
#define SUPPORT_UTF 1               // UTF-8/16/32 support
#define BSR_ANYCRLF 0               // \R matches any Unicode newline by default
#define NEWLINE_DEFAULT 0           // 0 = any newline
#define LINK_SIZE 2                 // internal link size
#define PARENS_NEST_LIMIT 250       // parentheses nesting limit
#define MATCH_LIMIT 10000000        // default match limit
#define MATCH_LIMIT_DEPTH 10000000  // default depth limit (prevents stack overflow)
#define MAX_NAME_SIZE 128           // max named group name length
#define MAX_NAME_COUNT 10000        // max number of named groups
```

### Step 2: Create PCRE2RegExEngine.cxx

New file: `scintilla/pcre2/scintilla/PCRE2RegExEngine.cxx`

Structure mirrors `OnigurumaRegExEngine.cxx` exactly:

```cpp
// encoding: UTF-8
/**
 * @file  PCRE2RegExEngine.cxx
 * @brief integrate PCRE2 regex engine for Scintilla library
 *        Replaces Oniguruma (archived April 2025)
 *
 *        uses PCRE2 - Perl Compatible Regular Expressions (v10.x)
 *        https://github.com/PCRE2Project/pcre2
 */

#ifdef SCI_OWNREGEX

#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>
#include <mbstring.h>

#define VC_EXTRALEAN 1
#define NOMINMAX 1
#include <windows.h>

#include "Geometry.h"
#include "Platform.h"
#include "Scintilla.h"
#include "ScintillaTypes.h"
#include "ILexer.h"
#include "SplitVector.h"
#include "Partitioning.h"
#include "CellBuffer.h"
#include "CaseFolder.h"
#include "RunStyles.h"
#include "Decoration.h"
#include "CharClassify.h"
#include "CharacterCategoryMap.h"
#include "Document.h"

#define PCRE2_CODE_UNIT_WIDTH 8
#define PCRE2_STATIC
#include "pcre2.h"

// ... implementation ...

#endif //SCI_OWNREGEX
```

#### Class: PCRE2RegExEngine

```cpp
class PCRE2RegExEngine : public RegexSearchBase {
public:
    explicit PCRE2RegExEngine(CharClassify* charClassTable);
    ~PCRE2RegExEngine() override;

    Sci::Position FindText(Document* doc, Sci::Position minPos, Sci::Position maxPos,
        const char* pattern, bool caseSensitive, bool word, bool wordStart,
        Scintilla::FindOption searchFlags, Sci::Position *length) override;

    const char* SubstituteByPosition(Document* doc, const char* text,
        Sci::Position* length) override;

private:
    void clear();
    std::string translateRegExpr(const std::string& regExprStr, bool wholeWord,
        bool wordStart, EndOfLine eolMode);
    std::string convertReplExpr(const std::string& replStr);

    std::string     m_RegExprStrg;
    uint32_t        m_CompileOptions;
    pcre2_code*     m_CompiledPattern;
    pcre2_match_data* m_MatchData;
    pcre2_match_context* m_MatchContext;
    EOLmode         m_EOLmode;
    Sci::Position   m_RangeBeg;
    Sci::Position   m_RangeEnd;
    char            m_ErrorInfo[256];
    Sci::Position   m_MatchPos;
    Sci::Position   m_MatchLen;

public:
    std::string m_SubstBuffer;
};
```

#### Member mapping (Oniguruma -> PCRE2)

| Oniguruma member | PCRE2 member | Notes |
|-----------------|--------------|-------|
| `OnigRegex m_RegExpr` | `pcre2_code* m_CompiledPattern` | Compiled pattern handle |
| `OnigRegion m_Region` | `pcre2_match_data* m_MatchData` | Match result storage |
| `OnigSyntaxType m_OnigSyntax` | (not needed) | PCRE2 has fixed syntax |
| `OnigOptionType m_CmplOptions` | `uint32_t m_CompileOptions` | Compile-time flags |
| — | `pcre2_match_context* m_MatchContext` | **New**: needed for offset limit |

#### Constructor

```cpp
PCRE2RegExEngine::PCRE2RegExEngine(CharClassify* /*charClassTable*/)
    : m_CompileOptions(PCRE2_UTF | PCRE2_UCP)  // UTF-8 + Unicode properties always on
    , m_CompiledPattern(nullptr)
    , m_MatchData(nullptr)
    , m_MatchContext(nullptr)
    , m_EOLmode(EOLmode::UDEF)
    , m_RangeBeg(-1)
    , m_RangeEnd(-1)
    , m_ErrorInfo()
    , m_MatchPos(-1)
    , m_MatchLen(0)
{
    m_MatchContext = pcre2_match_context_create(nullptr);
    // Set match limit to prevent catastrophic backtracking
    pcre2_set_match_limit(m_MatchContext, 10000000);
    pcre2_set_depth_limit(m_MatchContext, 10000000);
}
```

#### Destructor

```cpp
PCRE2RegExEngine::~PCRE2RegExEngine() {
    clear();
    if (m_MatchContext) pcre2_match_context_free(m_MatchContext);
}
```

#### clear()

```cpp
void PCRE2RegExEngine::clear() {
    m_RegExprStrg.clear();
    if (m_MatchData) { pcre2_match_data_free(m_MatchData); m_MatchData = nullptr; }
    if (m_CompiledPattern) { pcre2_code_free(m_CompiledPattern); m_CompiledPattern = nullptr; }
    m_RangeBeg = -1;
    m_RangeEnd = -1;
    m_MatchPos = -1;
    m_MatchLen = 0;
}
```

#### FindText() — Key implementation

This is the most complex method. Key differences from Oniguruma:

**1. Compile options assembly:**
```cpp
uint32_t options = PCRE2_UTF | PCRE2_UCP;  // always UTF-8 + Unicode properties
if (!caseSensitive) options |= PCRE2_CASELESS;
if (FlagSet(searchFlags, FindOption::DotMatchAll)) options |= PCRE2_DOTALL;
```

**2. Pattern compilation:**
```cpp
int errorcode;
PCRE2_SIZE erroroffset;
m_CompiledPattern = pcre2_compile(
    (PCRE2_SPTR)sRegExprStrg.c_str(),
    sRegExprStrg.length(),
    options,
    &errorcode,
    &erroroffset,
    nullptr  // default compile context
);
if (!m_CompiledPattern) {
    pcre2_get_error_message(errorcode, (PCRE2_UCHAR*)m_ErrorInfo, sizeof(m_ErrorInfo));
    return SciPos(-2);
}
m_MatchData = pcre2_match_data_create_from_pattern(m_CompiledPattern, nullptr);
// Optional: JIT compile for performance
pcre2_jit_compile(m_CompiledPattern, PCRE2_JIT_COMPLETE);
```

**3. Search execution (FORWARD):**

```cpp
// Get document buffer pointer
auto const docBegPtr = (PCRE2_SPTR)doc->BufferPointer();
auto const docLen = (PCRE2_SIZE)doc->Length();

// Set offset limit to constrain search end
pcre2_set_offset_limit(m_MatchContext, (PCRE2_SIZE)rangeEnd);

// Match options
uint32_t matchOptions = 0;
if (rangeBeg != 0) matchOptions |= PCRE2_NOTBOL;
if (rangeEnd != docLen) matchOptions |= PCRE2_NOTEOL;

int rc = pcre2_match(
    m_CompiledPattern,
    docBegPtr,
    docLen,
    (PCRE2_SIZE)rangeBeg,    // start offset
    matchOptions,
    m_MatchData,
    m_MatchContext
);

if (rc > 0) {
    PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(m_MatchData);
    m_MatchPos = SciPos(ovector[0]);
    m_MatchLen = SciPos(ovector[1]) - m_MatchPos;
}
```

**4. Search execution (BACKWARD) — THE HARD PART:**

PCRE2 has no native backward search. Oniguruma does this by swapping range pointers.
Implementation strategy:

```cpp
// Backward search: iterate forward, keep last match within range
Sci::Position lastMatchPos = -1;
Sci::Position lastMatchLen = 0;
PCRE2_SIZE searchStart = (PCRE2_SIZE)rangeBeg;

while (true) {
    int rc = pcre2_match(
        m_CompiledPattern, docBegPtr, docLen,
        searchStart, matchOptions,
        m_MatchData, m_MatchContext
    );
    if (rc <= 0) break;

    PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(m_MatchData);
    Sci::Position pos = SciPos(ovector[0]);
    Sci::Position len = SciPos(ovector[1]) - pos;

    if (pos > rangeEnd) break;   // past search range
    if (pos <= minPos) {         // within backward search range (minPos > maxPos)
        lastMatchPos = pos;
        lastMatchLen = len;
    }

    // Advance past this match (at least 1 byte, handle UTF-8)
    searchStart = ovector[1];
    if (searchStart == ovector[0]) {
        // zero-length match — advance by one UTF-8 character
        searchStart = doc->MovePositionOutsideChar(searchStart + 1, 1, true);
    }
}
m_MatchPos = lastMatchPos;
m_MatchLen = lastMatchLen;
```

**Performance note**: For backward search in large files, this iterates all forward matches.
Optimization: restrict the search range using `pcre2_set_offset_limit()` to only search
within `[maxPos, minPos]` (the relevant backward range), then find the last match in that window.

**Alternative backward search (line-by-line):**
Search backward line by line from `minPos`, stopping at `maxPos`. This avoids scanning
the entire document but is more complex to implement. Scintilla's built-in `BuiltinRegex`
uses this approach.

#### SubstituteByPosition() — Nearly identical

The replacement logic is custom and doesn't use Oniguruma APIs except for one call:
`onig_name_to_backref_number()` (line 499) to resolve named group references.

**PCRE2 replacement**: Use `pcre2_substring_number_from_name()`:

```cpp
// Where Oniguruma had:
int grpNum = onig_name_to_backref_number(m_RegExpr, name_beg, name_end, &m_Region);

// PCRE2 equivalent:
int grpNum = pcre2_substring_number_from_name(m_CompiledPattern, (PCRE2_SPTR)name_str);
```

**Important**: The name must be null-terminated for PCRE2 (Oniguruma took begin/end pointers).
Extract the name into a temporary buffer first.

For numbered group access, replace `m_Region.beg[n]`/`m_Region.end[n]` with:
```cpp
PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(m_MatchData);
auto rBeg = SciPos(ovector[2 * grpNum]);       // was: m_Region.beg[grpNum]
auto rEnd = SciPos(ovector[2 * grpNum + 1]);   // was: m_Region.end[grpNum]
auto len = rEnd - rBeg;
// Check for unset group: ovector[2*n] == PCRE2_UNSET
```

Number of capture groups: `pcre2_get_ovector_count(m_MatchData)` replaces `m_Region.num_regs`.

The rest of `SubstituteByPosition()` and `convertReplExpr()` can be copied verbatim —
they are pure string processing with no Oniguruma dependencies.

#### translateRegExpr() — Pattern translation

```cpp
std::string PCRE2RegExEngine::translateRegExpr(const std::string& regExprStr,
    bool wholeWord, bool wordStart, EndOfLine eolMode)
{
    std::string transRegExpr;

    // Word boundary wrapping (same as Oniguruma version)
    if (wholeWord || wordStart) {
        transRegExpr.push_back('\\');
        transRegExpr.push_back('b');
        transRegExpr.append(regExprStr);
        if (wholeWord) {
            transRegExpr.push_back('\\');
            transRegExpr.push_back('b');
        }
        replaceAll(transRegExpr, ".", R"(\w)");
    } else {
        transRegExpr.append(regExprStr);
    }

    // PCRE2 does NOT support \< and \> — translate to lookaround equivalents
    replaceAll(transRegExpr, R"(\<)", R"((?<!\w)(?=\w))");   // word begin
    replaceAll(transRegExpr, R"(\>)", R"((?<=\w)(?!\w))");   // word end
    // NOTE: must also handle escaped versions \\< and \\> (literal < >) — do NOT translate those

    // \h and \H are NATIVE in PCRE2 — no translation needed!
    // (Oniguruma version had: replaceAll \h -> [^\S\n\v\f\r\u2028\u2029])
    // REMOVE those translations.

    // \uHHHH -> \x{HHHH} for PCRE2
    // This requires a smarter regex-based replacement (not simple string replace)
    // to transform \uHHHH patterns to \x{HHHH}
    // Implementation: scan for \u followed by exactly 4 hex digits
    translateUnicodeEscapes(transRegExpr);  // helper function

    return transRegExpr;
}
```

**Helper for `\uHHHH` -> `\x{HHHH}` translation:**
```cpp
static void translateUnicodeEscapes(std::string& s) {
    std::string result;
    result.reserve(s.length());
    for (size_t i = 0; i < s.length(); ++i) {
        if (s[i] == '\\' && (i + 5) <= s.length() && s[i+1] == 'u' &&
            isxdigit(s[i+2]) && isxdigit(s[i+3]) && isxdigit(s[i+4]) && isxdigit(s[i+5])) {
            result += "\\x{";
            result += s[i+2]; result += s[i+3]; result += s[i+4]; result += s[i+5];
            result += '}';
            i += 5;
        } else {
            result += s[i];
        }
    }
    s = result;
}
```

**Edge case warning for `\<` / `\>` translation**: The naive `replaceAll` approach will
also incorrectly translate escaped `\\<` (literal `<`). A proper implementation must
skip escaped backslashes. Consider scanning character by character instead of using
`replaceAll`.

#### SimpleRegExEngine / OnigRegExFind export

Replace with a PCRE2-based equivalent. This is simpler — forward-only, no document
buffer concerns:

```cpp
class SimplePCRE2Engine {
public:
    explicit SimplePCRE2Engine(EOLmode eolMode);
    ~SimplePCRE2Engine();
    ptrdiff_t Find(const char* pattern, const char* text,
                   bool caseSensitive, int* matchLen_out = nullptr);
private:
    EOLmode m_EOLmode;
    pcre2_code* m_Code;
    pcre2_match_data* m_MatchData;
};

// Exported function — keep same signature for binary compatibility
extern "C" __declspec(dllexport)
ptrdiff_t WINAPI OnigRegExFind(const char *pchPattern, const char *pchText,
                                const bool caseSensitive, const int eolMode, int *matchLen_out) {
    SimplePCRE2Engine engine(static_cast<EOLmode>(eolMode));
    return engine.Find(pchPattern, pchText, caseSensitive, matchLen_out);
}
```

**IMPORTANT**: Keep the exported function name as `OnigRegExFind` (despite using PCRE2)
to avoid changing `src/SciCall.h`, `src/Edit.c`, and `src/Styles.c`. The function
signature is the same. Consider renaming to `NP3RegExFind` in a follow-up if desired.

### Step 3: Update Build Configuration

#### scintilla/Scintilla.vcxproj

**Preprocessor** — All configurations, change:
```
ONIG_STATIC  -->  PCRE2_CODE_UNIT_WIDTH=8;PCRE2_STATIC;HAVE_CONFIG_H
```
Keep `SCI_OWNREGEX` and `NO_CXX11_REGEX` unchanged.

**Include directories** — All configurations, change:
```
../oniguruma/src  -->  ../pcre2/src
```

**Source files** — Replace Oniguruma `.c` files with PCRE2 `.c` files:
```xml
<!-- Remove all <ClCompile Include="oniguruma\src\*.c" /> entries -->
<!-- Remove <ClCompile Include="oniguruma\scintilla\OnigurumaRegExEngine.cxx" /> -->

<!-- Add PCRE2 source files -->
<ClCompile Include="pcre2\src\pcre2_auto_possess.c" />
<ClCompile Include="pcre2\src\pcre2_chartables.c" />
<ClCompile Include="pcre2\src\pcre2_chkdint.c" />
<ClCompile Include="pcre2\src\pcre2_compile.c" />
<ClCompile Include="pcre2\src\pcre2_config.c" />
<ClCompile Include="pcre2\src\pcre2_context.c" />
<ClCompile Include="pcre2\src\pcre2_dfa_match.c" />
<ClCompile Include="pcre2\src\pcre2_error.c" />
<ClCompile Include="pcre2\src\pcre2_extuni.c" />
<ClCompile Include="pcre2\src\pcre2_find_bracket.c" />
<ClCompile Include="pcre2\src\pcre2_jit_compile.c" />
<ClCompile Include="pcre2\src\pcre2_maketables.c" />
<ClCompile Include="pcre2\src\pcre2_match.c" />
<ClCompile Include="pcre2\src\pcre2_match_data.c" />
<ClCompile Include="pcre2\src\pcre2_newline.c" />
<ClCompile Include="pcre2\src\pcre2_ord2utf.c" />
<ClCompile Include="pcre2\src\pcre2_pattern_info.c" />
<ClCompile Include="pcre2\src\pcre2_script_run.c" />
<ClCompile Include="pcre2\src\pcre2_string_utils.c" />
<ClCompile Include="pcre2\src\pcre2_study.c" />
<ClCompile Include="pcre2\src\pcre2_substitute.c" />
<ClCompile Include="pcre2\src\pcre2_substring.c" />
<ClCompile Include="pcre2\src\pcre2_tables.c" />
<ClCompile Include="pcre2\src\pcre2_ucd.c" />
<ClCompile Include="pcre2\src\pcre2_valid_utf.c" />
<ClCompile Include="pcre2\src\pcre2_xclass.c" />
<ClCompile Include="pcre2\scintilla\PCRE2RegExEngine.cxx" />
```

**Header files** — Replace Oniguruma headers:
```xml
<!-- Remove oniguruma headers -->
<!-- Add -->
<ClInclude Include="pcre2\src\config.h" />
<ClInclude Include="pcre2\src\pcre2.h" />
<ClInclude Include="pcre2\src\pcre2_internal.h" />
<ClInclude Include="pcre2\src\pcre2_intmodedep.h" />
<ClInclude Include="pcre2\src\pcre2_ucp.h" />
```

#### scintilla/ScintillaDLL.vcxproj

Same changes as above, but paths use `../pcre2/` prefix instead of `../oniguruma/`.

**Preprocessor** change: `ONIG_EXTERN=extern` --> `PCRE2_CODE_UNIT_WIDTH=8;PCRE2_STATIC;HAVE_CONFIG_H`

### Step 4: Optional — JIT Support

PCRE2 includes an optional JIT compiler (`pcre2_jit_compile`) that can significantly
speed up repeated matches. Enable it by:

1. Including `pcre2_jit_compile.c` in the build (already listed above)
2. Including the sljit sources that PCRE2 bundles (`sljit/` subdirectory)
3. Calling `pcre2_jit_compile(m_CompiledPattern, PCRE2_JIT_COMPLETE)` after compilation
4. Using `pcre2_jit_match()` instead of `pcre2_match()` for JIT-compiled patterns

If JIT adds too much complexity or build size, it can be omitted — the interpreter
is already faster than Oniguruma for most patterns.

### Step 5: Testing Checklist

1. **Forward search**: Basic patterns, regex patterns, case-sensitive/insensitive
2. **Backward search**: Find Previous with regex enabled
3. **Replace**: `$1`, `${name}`, escape sequences in replacement
4. **Replace All**: Full document replacement
5. **Word boundary**: `\<word\>` patterns still work (via translation)
6. **Unicode**: `\p{L}`, `\p{N}`, Unicode text in documents
7. **Lookbehind**: `(?<=prefix)match` patterns
8. **Possessive**: `a++b` type patterns
9. **\h \H**: Horizontal space matching (now native, should be same behavior)
10. **\uHHHH**: Unicode escapes in patterns (translated to `\x{HHHH}`)
11. **\K**: Match reset patterns
12. **\R**: Newline sequence matching
13. **OnigRegExFind**: URL detection in Edit.c still works
14. **OnigRegExFind**: Lexer file pattern matching in Styles.c still works
15. **Large files**: Performance regression test with multi-MB files
16. **All platforms**: Build x86, x64, x64_AVX2, ARM64

### Step 6: Cleanup

After confirming everything works:
1. Remove `scintilla/oniguruma/` directory entirely
2. Update `CLAUDE.md` vendored dependencies table
3. Update `.github/copilot-instructions.md` if it references Oniguruma
4. Consider renaming `OnigRegExFind` export to `NP3RegExFind` (requires updating
   `src/SciCall.h:107`, `src/Edit.c:2126`, `src/Styles.c:2533`)

---

## API Quick Reference

### PCRE2 Core Functions (8-bit)

```c
// Compilation
pcre2_code *pcre2_compile(PCRE2_SPTR pattern, PCRE2_SIZE length,
    uint32_t options, int *errorcode, PCRE2_SIZE *erroroffset,
    pcre2_compile_context *ccontext);
void pcre2_code_free(pcre2_code *code);

// JIT (optional)
int pcre2_jit_compile(pcre2_code *code, uint32_t options);

// Matching
pcre2_match_data *pcre2_match_data_create_from_pattern(const pcre2_code *code,
    pcre2_general_context *gcontext);
void pcre2_match_data_free(pcre2_match_data *match_data);

int pcre2_match(const pcre2_code *code, PCRE2_SPTR subject, PCRE2_SIZE length,
    PCRE2_SIZE startoffset, uint32_t options,
    pcre2_match_data *match_data, pcre2_match_context *mcontext);

// Results
PCRE2_SIZE *pcre2_get_ovector_pointer(pcre2_match_data *match_data);
uint32_t pcre2_get_ovector_count(pcre2_match_data *match_data);

// Named groups
int pcre2_substring_number_from_name(const pcre2_code *code, PCRE2_SPTR name);

// Context (for offset limit, match limits)
pcre2_match_context *pcre2_match_context_create(pcre2_general_context *gcontext);
void pcre2_match_context_free(pcre2_match_context *mcontext);
int pcre2_set_offset_limit(pcre2_match_context *mcontext, PCRE2_SIZE value);
int pcre2_set_match_limit(pcre2_match_context *mcontext, uint32_t value);
int pcre2_set_depth_limit(pcre2_match_context *mcontext, uint32_t value);

// Error messages
int pcre2_get_error_message(int errorcode, PCRE2_UCHAR *buffer, PCRE2_SIZE bufflen);
```

### Key PCRE2 Compile Options

```c
PCRE2_UTF           // Treat pattern and subject as UTF-8
PCRE2_UCP           // Use Unicode properties for \d, \w, \s, \b
PCRE2_CASELESS      // Case-insensitive matching
PCRE2_DOTALL        // . matches any character including newline
PCRE2_MULTILINE     // ^ and $ match at line boundaries (NOT same as Onig MULTILINE!)
PCRE2_BSR_UNICODE   // \R matches any Unicode newline sequence
```

**IMPORTANT naming confusion**: Oniguruma `ONIG_OPTION_MULTILINE` = "dot matches newline"
= PCRE2 `PCRE2_DOTALL`. PCRE2 `PCRE2_MULTILINE` = "^ $ match line boundaries" which
is a different concept.

---

## Risk Assessment

| Risk | Impact | Mitigation |
|------|--------|------------|
| Backward search performance | Medium | Start with iterate-forward approach; optimize later if needed |
| `\<` `\>` translation edge cases | Low | Thorough testing; character-by-character scanner |
| `\uHHHH` translation edge cases | Low | Simple pattern; well-defined 4-hex-digit format |
| PCRE2 compile/config complexity | Low | Use `.h.generic` files; minimal config |
| Binary size change | Low | PCRE2 is similar size to Oniguruma |
| User-visible regex syntax changes | Low | All features preserved; `\h`/`\H` now native (better) |

## Files to Create/Modify Summary

| Action | File |
|--------|------|
| **CREATE** | `scintilla/pcre2/src/` — PCRE2 source + headers |
| **CREATE** | `scintilla/pcre2/scintilla/PCRE2RegExEngine.cxx` |
| **MODIFY** | `scintilla/Scintilla.vcxproj` — swap source/header/define/include |
| **MODIFY** | `scintilla/ScintillaDLL.vcxproj` — swap source/header/define/include |
| **DELETE** | `scintilla/oniguruma/` — entire directory (after validation) |
| **Optional MODIFY** | `src/SciCall.h:107` — rename `OnigRegExFind` to `NP3RegExFind` |
| **Optional MODIFY** | `src/Edit.c:2126` — update function call name |
| **Optional MODIFY** | `src/Styles.c:2533` — update function call name |
