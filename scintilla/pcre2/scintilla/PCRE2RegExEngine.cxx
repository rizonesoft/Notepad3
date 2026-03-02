// encoding: UTF-8
/**
 * @file  PCRE2RegExEngine.cxx
 * @brief integrate PCRE2 regex engine for Scintilla library
 *        (Scintilla Lib is copyright 1998-2017 by Neil Hodgson <neilh@scintilla.org>)
 *
 *        uses PCRE2 - Perl Compatible Regular Expressions (v10.47)
 *        https://github.com/PCRE2Project/pcre2
 *
 *        Replaces Oniguruma (archived April 2025)
 *
 * @autor Rainer Kottenhoff (RaiKoHoff)
 *
 */

#ifdef SCI_OWNREGEX

#include <cstdlib>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
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

// ---------------------------------------------------------------
#ifndef PCRE2_CODE_UNIT_WIDTH
#define PCRE2_CODE_UNIT_WIDTH 8
#endif
#ifndef PCRE2_STATIC
#define PCRE2_STATIC
#endif
#include "../src/pcre2.h"
// ---------------------------------------------------------------

using namespace Scintilla;
using namespace Scintilla::Internal;

#define SciPos(pos)     static_cast<Sci::Position>(pos)
#define SciLn(line)     static_cast<Sci::Line>(line)
#define SciPosExt(pos)  static_cast<Sci_Position>(pos)

// ============================================================================
// ***   PCRE2 configuration   ***
// ============================================================================

enum class EOLmode : int { UDEF = -1, CRLF = SC_EOL_CRLF, CR = SC_EOL_CR, LF = SC_EOL_LF };

// ============================================================================
// ============================================================================

class PCRE2RegExEngine : public RegexSearchBase
{
public:

  explicit PCRE2RegExEngine(CharClassify* /*charClassTable*/)
    : m_CompileOptions(PCRE2_UTF | PCRE2_UCP | PCRE2_MULTILINE)
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
    // Set match limits to prevent catastrophic backtracking
    pcre2_set_match_limit(m_MatchContext, 10000000);
    pcre2_set_depth_limit(m_MatchContext, 10000000);
  }

  ~PCRE2RegExEngine() override
  {
    clear();
    if (m_MatchContext) {
      pcre2_match_context_free(m_MatchContext);
      m_MatchContext = nullptr;
    }
  }

  Sci::Position FindText(Document* doc, Sci::Position minPos, Sci::Position maxPos, const char* pattern,
      bool caseSensitive, bool word, bool wordStart, Scintilla::FindOption searchFlags, Sci::Position *length) override;

  const char* SubstituteByPosition(Document* doc, const char* text, Sci::Position* length) override;

private:

  void clear();

  std::string translateRegExpr(const std::string & regExprStr, bool wholeWord, bool wordStart, EndOfLine eolMode);

  std::string convertReplExpr(const std::string & replStr);

private:

  std::string m_RegExprStrg;

  uint32_t            m_CompileOptions;
  pcre2_code*         m_CompiledPattern;
  pcre2_match_data*   m_MatchData;
  pcre2_match_context* m_MatchContext;
  EOLmode             m_EOLmode;

  Sci::Position       m_RangeBeg;
  Sci::Position       m_RangeEnd;

  char                m_ErrorInfo[256];

  Sci::Position       m_MatchPos;
  Sci::Position       m_MatchLen;

public:
  std::string m_SubstBuffer;

};

// ============================================================================

RegexSearchBase *Scintilla::Internal::CreateRegexSearch(CharClassify *charClassTable)
{
  return new PCRE2RegExEngine(charClassTable);
}

// ============================================================================



// ============================================================================
//   Some Helpers
// ============================================================================


/******************************************************************************
*
*  UnSlash functions
*  Mostly taken from SciTE, (c) Neil Hodgson, http://www.scintilla.org
*
* Is the character an octal digit?
*/
constexpr bool IsOctalDigit(char ch)
{
  return ((ch >= '0') && (ch <= '7'));
}
// ----------------------------------------------------------------------------

/**
* If the character is an hex digit, get its value.
*/
constexpr int GetHexDigit(char ch)
{
  if ((ch >= '0') && (ch <= '9')) {
    return (ch - '0');
  }
  if ((ch >= 'A') && (ch <= 'F')) {
    return (ch - 'A' + 10);
  }
  if ((ch >= 'a') && (ch <= 'f')) {
    return (ch - 'a' + 10);
  }
  return -1;
}
// ----------------------------------------------------------------------------


static void replaceAll(std::string& source, const std::string& from, const std::string& to)
{
  std::string newString;
  newString.reserve(source.length() * 2);  // avoids a few memory allocations

  std::string::size_type lastPos = 0;
  std::string::size_type findPos;

  while (std::string::npos != (findPos = source.find(from, lastPos))) {
    newString.append(source, lastPos, findPos - lastPos);
    newString += to;
    lastPos = findPos + from.length();
  }
  // Care for the rest after last occurrence
  newString += source.substr(lastPos);

  source.swap(newString);
}
// ----------------------------------------------------------------------------



// ============================================================================
//   private methods
// ============================================================================


void PCRE2RegExEngine::clear()
{
  m_RegExprStrg.clear();
  if (m_MatchData) {
    pcre2_match_data_free(m_MatchData);
    m_MatchData = nullptr;
  }
  if (m_CompiledPattern) {
    pcre2_code_free(m_CompiledPattern);
    m_CompiledPattern = nullptr;
  }
  m_RangeBeg = -1;
  m_RangeEnd = -1;
  m_MatchPos = -1;
  m_MatchLen = 0;
}
// ============================================================================


/**
 * Find text in document, supporting both forward and backward
 * searches (just pass minPos > maxPos to do a backward search)
 * Has not been tested with backwards DBCS searches yet.
 */
Sci::Position PCRE2RegExEngine::FindText(Document* doc, Sci::Position minPos, Sci::Position maxPos, const char *pattern,
                                         bool caseSensitive, bool word, bool wordStart, Scintilla::FindOption searchFlags, Sci::Position *length)
{
  if (!(pattern && (strlen(pattern) > 0))) {
    *length = 0;
    return SciPos(-1);
  }

  bool const findForward = (minPos <= maxPos);
  int const increment = findForward ? 1 : -1;
  // Range endpoints should not be inside DBCS characters, but just in case, move them.
  minPos = doc->MovePositionOutsideChar(minPos, increment, true);
  maxPos = doc->MovePositionOutsideChar(maxPos, -increment, true);

  Sci::Position const rangeBeg = (findForward) ? minPos : maxPos;
  Sci::Position const rangeEnd = (findForward) ? maxPos : minPos;
  //Sci::Position const rangeLen = (rangeEnd - rangeBeg);

  EOLmode const eolMode = static_cast<EOLmode>(doc->eolMode);

  // --- Build compile options ---
  // PCRE2_MULTILINE: ^/$ match at line boundaries (Oniguruma's default behavior)
  uint32_t compileOptions = PCRE2_UTF | PCRE2_UCP | PCRE2_MULTILINE;

  if (!caseSensitive) {
    compileOptions |= PCRE2_CASELESS;
  }
  if (FlagSet(searchFlags, FindOption::DotMatchAll)) {
    compileOptions |= PCRE2_DOTALL;  // Note: Oniguruma called this MULTILINE
  }

  std::string const sRegExprStrg = translateRegExpr(pattern, word, wordStart, doc->eolMode);

  bool const bReCompile = (m_CompiledPattern == nullptr) || (m_CompileOptions != compileOptions)
                          || (m_RegExprStrg.compare(sRegExprStrg) != 0) || (m_EOLmode != eolMode);

  if (bReCompile) {
    clear();
    m_RegExprStrg = sRegExprStrg;
    m_CompileOptions = compileOptions;
    m_RangeBeg = rangeBeg;
    m_RangeEnd = rangeEnd;
    m_EOLmode = eolMode;
    m_ErrorInfo[0] = '\0';

    try {
      int errorcode = 0;
      PCRE2_SIZE erroroffset = 0;
      m_CompiledPattern = pcre2_compile(
        reinterpret_cast<PCRE2_SPTR>(m_RegExprStrg.c_str()),
        m_RegExprStrg.length(),
        m_CompileOptions,
        &errorcode,
        &erroroffset,
        nullptr  // default compile context
      );

      if (!m_CompiledPattern) {
        pcre2_get_error_message(errorcode, reinterpret_cast<PCRE2_UCHAR*>(m_ErrorInfo), sizeof(m_ErrorInfo));
        OutputDebugStringA("PCRE2 compile error: ");
        OutputDebugStringA(m_ErrorInfo);
        OutputDebugStringA("\n");
        clear();
        return SciPos(-2);   // -1 is normally used for not found, -2 is used here for invalid regex
      }

      m_MatchData = pcre2_match_data_create_from_pattern(m_CompiledPattern, nullptr);

      // JIT compile for performance (silently ignored if JIT not available)
      pcre2_jit_compile(m_CompiledPattern, PCRE2_JIT_COMPLETE);
    }
    catch (...) {
      clear();
      return SciPos(-2);
    }
  } else {
    // check if already searched for (same range = same result)
    if ((m_RangeBeg == rangeBeg) && (m_RangeEnd == rangeEnd)) {
      *length = m_MatchLen;
      return m_MatchPos;
    }
  }

  // ---  search document range for pattern match   ---
  // !!! Performance issue: Scintilla: moving Gap needs memcopy - high costs for find/replace in large document
  auto const docBegPtr = reinterpret_cast<PCRE2_SPTR>(doc->BufferPointer());
  auto const docLen = static_cast<PCRE2_SIZE>(doc->Length());

  m_RangeBeg = rangeBeg;
  m_RangeEnd = rangeEnd;
  m_MatchPos = SciPos(-1);  // not found
  m_MatchLen = SciPos(0);

  try {
    // Match options
    // Note: Oniguruma's ONIG_OPTION_NOT_BEGIN_STRING/NOT_END_STRING controlled
    // \A/\z anchors. PCRE2 has no equivalent, but since the subject IS the full
    // document buffer, \A and \z naturally match at doc start/end — no flags needed.
    // Do NOT use PCRE2_NOTBOL/PCRE2_NOTEOL — those control ^/$ which is handled
    // correctly by PCRE2_MULTILINE.
    uint32_t matchOptions = PCRE2_NO_UTF_CHECK;

    if (findForward) {
      // --- Forward search ---
      int rc = pcre2_match(
        m_CompiledPattern,
        docBegPtr,
        docLen,
        static_cast<PCRE2_SIZE>(rangeBeg),   // start offset
        matchOptions,
        m_MatchData,
        m_MatchContext
      );

      if (rc > 0) {
        PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(m_MatchData);
        Sci::Position pos = SciPos(ovector[0]);
        Sci::Position len = SciPos(ovector[1]) - pos;
        if ((pos >= rangeBeg) && (pos <= rangeEnd)) {
          m_MatchPos = pos;
          m_MatchLen = len;
        }
      }
      else if (rc < 0 && rc != PCRE2_ERROR_NOMATCH) {
        pcre2_get_error_message(rc, reinterpret_cast<PCRE2_UCHAR*>(m_ErrorInfo), sizeof(m_ErrorInfo));
        OutputDebugStringA("PCRE2 match error: ");
        OutputDebugStringA(m_ErrorInfo);
        OutputDebugStringA("\n");
      }
    }
    else {
      // --- Backward search (reverse chunking) ---
      // PCRE2 has no native backward search. Instead of scanning the entire
      // range forward (O(n)), we search in reverse chunks starting near rangeEnd.
      // Each chunk searches forward within a window and keeps the last match.
      // Average case is O(chunk_size) instead of O(range_size).

      Sci::Position lastMatchPos = SciPos(-1);
      Sci::Position lastMatchLen = SciPos(0);

      pcre2_match_data* iterMatchData = pcre2_match_data_create_from_pattern(m_CompiledPattern, nullptr);

      Sci::Position const chunkSize = 4096;  // search window size
      Sci::Position chunkStart = (rangeEnd > chunkSize) ? (rangeEnd - chunkSize) : rangeBeg;
      if (chunkStart < rangeBeg) chunkStart = rangeBeg;

      // Align chunkStart to a valid UTF-8 character boundary
      chunkStart = doc->MovePositionOutsideChar(chunkStart, -1, true);

      bool found = false;
      while (!found) {
        // Search forward within [chunkStart, rangeEnd] and keep the last match
        PCRE2_SIZE searchStart = static_cast<PCRE2_SIZE>(chunkStart);

        while (searchStart <= static_cast<PCRE2_SIZE>(rangeEnd)) {
          int rc = pcre2_match(
            m_CompiledPattern,
            docBegPtr,
            docLen,
            searchStart,
            matchOptions,
            iterMatchData,
            m_MatchContext
          );

          if (rc <= 0) break;  // no more matches

          PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(iterMatchData);
          Sci::Position pos = SciPos(ovector[0]);
          Sci::Position len = SciPos(ovector[1]) - pos;

          if (pos > rangeEnd) break;   // past search range

          lastMatchPos = pos;
          lastMatchLen = len;
          found = true;

          // Advance past this match (at least 1 byte, handle UTF-8)
          if (ovector[1] > ovector[0]) {
            searchStart = ovector[1];
          } else {
            searchStart = static_cast<PCRE2_SIZE>(
              doc->MovePositionOutsideChar(SciPos(searchStart + 1), 1, true)
            );
          }
        }

        // If we already searched from the very beginning, we're done
        if (chunkStart <= rangeBeg) break;

        // Step back by another chunk
        if (!found) {
          Sci::Position newStart = (chunkStart > chunkSize) ? (chunkStart - chunkSize) : rangeBeg;
          if (newStart < rangeBeg) newStart = rangeBeg;
          newStart = doc->MovePositionOutsideChar(newStart, -1, true);
          if (newStart < rangeBeg) newStart = rangeBeg;
          chunkStart = newStart;
        }
      }

      // Re-run the match at the final position to populate m_MatchData for SubstituteByPosition
      if (lastMatchPos >= 0) {
        pcre2_match(
          m_CompiledPattern,
          docBegPtr,
          docLen,
          static_cast<PCRE2_SIZE>(lastMatchPos),
          matchOptions | PCRE2_ANCHORED,
          m_MatchData,
          m_MatchContext
        );
      }

      pcre2_match_data_free(iterMatchData);

      m_MatchPos = lastMatchPos;
      m_MatchLen = lastMatchLen;
    }
  }
  catch (...) {
    return SciPos(-3);  // -1 is normally used for not found, -3 is used here for exception
  }

  //NOTE: potential 64-bit-size issue at interface here:
  *length = m_MatchLen;
  return m_MatchPos;
}
// ============================================================================



// ============================================================================


const char* PCRE2RegExEngine::SubstituteByPosition(Document* doc, const char* text, Sci::Position* length)
{
  if (m_MatchPos < 0) {
    *length = SciPos(-1);
    return nullptr;
  }
  std::string sText(text, *length);
  std::string const & rawReplStrg = convertReplExpr(sText);

  m_SubstBuffer.clear();

  PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(m_MatchData);
  uint32_t ovecCount = pcre2_get_ovector_count(m_MatchData);

  for (size_t j = 0; j < rawReplStrg.length(); ++j)
  {
    bool bReplaced = false;
    if ((rawReplStrg[j] == '$') || (rawReplStrg[j] == '\\'))
    {
      if ((rawReplStrg[j + 1] >= '0') && (rawReplStrg[j + 1] <= '9'))
      {
        // group # limit = 99 / TODO: allow for arbitrary number of groups/regions

        bool const digit2nd = ((rawReplStrg[j + 2] >= '0') && (rawReplStrg[j + 2] <= '9')) && (ovecCount > 10);
        int const grpNum = digit2nd ? (rawReplStrg[j + 1] - '0') * 10 + (rawReplStrg[j + 2] - '0') : (rawReplStrg[j + 1] - '0');
        if (static_cast<uint32_t>(grpNum) < ovecCount)
        {
          if (ovector[2 * grpNum] != PCRE2_UNSET) {  // check for unset group (unsigned compare)
            auto const rBeg = SciPos(ovector[2 * grpNum]);
            auto const len = SciPos(ovector[2 * grpNum + 1]) - rBeg;
            m_SubstBuffer.append(doc->RangePointer(rBeg, len), static_cast<size_t>(len));
          }
        }
        bReplaced = true;
        j += digit2nd ? 2 : 1;
      }
      else if (rawReplStrg[j] == '$')
      {
        size_t k = ((rawReplStrg[j + 1] == '+') && (rawReplStrg[j + 2] == '{')) ? (j + 3) : ((rawReplStrg[j + 1] == '{') ? (j + 2) : 0);
        if (k > 0) {
          // named group replacement
          size_t nameStart = k;
          while (rawReplStrg[k] && IsCharAlphaNumericA(rawReplStrg[k])) { ++k; }
          if (rawReplStrg[k] == '}')
          {
            // PCRE2 needs null-terminated name — extract to stack buffer
            char nameBuf[128];
            size_t nameLen = k - nameStart;
            if (nameLen >= sizeof(nameBuf)) nameLen = sizeof(nameBuf) - 1;
            memcpy(nameBuf, &rawReplStrg[nameStart], nameLen);
            nameBuf[nameLen] = '\0';

            int const grpNum = pcre2_substring_number_from_name(
              m_CompiledPattern, reinterpret_cast<PCRE2_SPTR>(nameBuf));

            if ((grpNum >= 0) && (static_cast<uint32_t>(grpNum) < ovecCount))
            {
              if (ovector[2 * grpNum] != PCRE2_UNSET) {  // check for unset group (unsigned compare)
                auto const rBeg = SciPos(ovector[2 * grpNum]);
                auto const len = SciPos(ovector[2 * grpNum + 1]) - rBeg;
                m_SubstBuffer.append(doc->RangePointer(rBeg, len), static_cast<size_t>(len));
              }
            }
            bReplaced = true;
            j = k;
          }
        }
      }
      else if ((rawReplStrg[j + 1] == '$') || (rawReplStrg[j + 1] == '\\')) {
        ++j; //  '\$' -> '$' or '\\' -> '\'
      }
    }
    if (!bReplaced) { m_SubstBuffer.push_back(rawReplStrg[j]); }
  }

  //NOTE: potential 64-bit-size issue at interface here:
  *length = SciPos(m_SubstBuffer.length());
  return m_SubstBuffer.c_str();
}
// ============================================================================



// ============================================================================
//
// private methods

std::string PCRE2RegExEngine::translateRegExpr(const std::string & regExprStr, bool wholeWord, bool wordStart,
                                               EndOfLine eolMode)
{
  UNREFERENCED_PARAMETER(eolMode);

  std::string	transRegExpr;

  if (wholeWord || wordStart) {      // push '\b' at the begin of regexpr
    transRegExpr.push_back('\\');
    transRegExpr.push_back('b');
    transRegExpr.append(regExprStr);
    if (wholeWord) {               // push '\b' at the end of regexpr
      transRegExpr.push_back('\\');
      transRegExpr.push_back('b');
    }
    replaceAll(transRegExpr, ".", R"(\w)");
  }
  else {
    transRegExpr.append(regExprStr);
  }

  // Single-pass translation of PCRE2-incompatible escape sequences:
  // - \< \> word boundaries → lookaround equivalents
  // - \uHHHH → \x{HHHH} Unicode escapes
  // PCRE2 \h and \H are native (horizontal whitespace) — no translation needed.
  {
    std::string result;
    result.reserve(transRegExpr.length() + 32);
    for (size_t i = 0; i < transRegExpr.length(); ++i) {
      if (transRegExpr[i] == '\\' && (i + 1) < transRegExpr.length()) {
        if (transRegExpr[i + 1] == '\\') {
          // escaped backslash — copy both and skip
          result.push_back('\\');
          result.push_back('\\');
          ++i;
        }
        else if (transRegExpr[i + 1] == '<') {
          result.append(R"((?<!\w)(?=\w))");  // word begin
          ++i;
        }
        else if (transRegExpr[i + 1] == '>') {
          result.append(R"((?<=\w)(?!\w))");  // word end
          ++i;
        }
        else if (transRegExpr[i + 1] == 'u' &&
                 (i + 5) < transRegExpr.length() &&
                 GetHexDigit(transRegExpr[i + 2]) >= 0 &&
                 GetHexDigit(transRegExpr[i + 3]) >= 0 &&
                 GetHexDigit(transRegExpr[i + 4]) >= 0 &&
                 GetHexDigit(transRegExpr[i + 5]) >= 0)
        {
          result.append("\\x{");
          result.push_back(transRegExpr[i + 2]);
          result.push_back(transRegExpr[i + 3]);
          result.push_back(transRegExpr[i + 4]);
          result.push_back(transRegExpr[i + 5]);
          result.push_back('}');
          i += 5;
        }
        else {
          result.push_back(transRegExpr[i]);
        }
      }
      else {
        result.push_back(transRegExpr[i]);
      }
    }
    transRegExpr = result;
  }

  return transRegExpr;
}
// ----------------------------------------------------------------------------


std::string PCRE2RegExEngine::convertReplExpr(const std::string& replStr)
{
  std::string	convReplExpr;

  for (size_t i = 0; i < replStr.length(); ++i) {
    char ch = replStr[i];
    if (ch == '\\') {
      if (i + 1 >= replStr.length()) {
        convReplExpr.push_back('\\');
        break;
      }
      ch = replStr[++i]; // next char
      if (ch >= '1' && ch <= '9') {
        // former behavior convenience:
        // change "\\<n>" to deelx's group reference ($<n>)
        convReplExpr.push_back('$');
        convReplExpr.push_back(ch);
        continue;
      }
      switch (ch) {
        // check for escape seq:
      case 'a':
        convReplExpr.push_back('\a');
        break;
      case 'b':
        convReplExpr.push_back('\x1B');
        break;
      case 'f':
        convReplExpr.push_back('\f');
        break;
      case 'n':
        convReplExpr.push_back('\n');
        break;
      case 'r':
        convReplExpr.push_back('\r');
        break;
      case 't':
        convReplExpr.push_back('\t');
        break;
      case 'v':
        convReplExpr.push_back('\v');
        break;
      case '\\':
        convReplExpr.push_back('\\'); // preserve escd "\"
        convReplExpr.push_back('\\');
        break;
      case 'x':
      case 'u':
        {
          bool bShort = (ch == 'x');
          char buf[8] = { '\0' };
          char *pch = buf;
          WCHAR val[2] = L"";
          int hex;
          val[0] = 0;
          ++i;
          hex = GetHexDigit(replStr[i]);
          if (hex >= 0) {
            ++i;
            val[0] = static_cast<WCHAR>(hex);
            hex = GetHexDigit(replStr[i]);
            if (hex >= 0) {
              ++i;
              val[0] *= 16;
              val[0] += static_cast<WCHAR>(hex);
              if (!bShort) {
                hex = GetHexDigit(replStr[i]);
                if (hex >= 0) {
                  ++i;
                  val[0] *= 16;
                  val[0] += static_cast<WCHAR>(hex);
                  hex = GetHexDigit(replStr[i]);
                  if (hex >= 0) {
                    ++i;
                    val[0] *= 16;
                    val[0] += static_cast<WCHAR>(hex);
                  }
                }
              }
            }
            if (val[0]) {
              val[1] = 0;
              WideCharToMultiByte(CP_UTF8, 0, val, -1, buf, ARRAYSIZE(buf), nullptr, nullptr);
              convReplExpr.push_back(*pch++);
              while (*pch)
                convReplExpr.push_back(*pch++);
            }
            else
              convReplExpr.push_back(ch); // unknown hex seq
          }
          else
            convReplExpr.push_back(ch); // unknown hex seq
        }
        break;

      default: // unknown ctrl seq
          convReplExpr.push_back('\\'); // revert
          convReplExpr.push_back(ch);
        break;
      }
    }
    else {
      convReplExpr.push_back(ch);
    }
  } //for

  return convReplExpr;
}
// ============================================================================




// ============================================================================
// ============================================================================


class SimplePCRE2Engine
{
public:

  SimplePCRE2Engine()
    : m_CompileOptions(0)
    , m_CompiledPattern(nullptr)
    , m_MatchData(nullptr)
    , m_ErrorInfo()
  {
  }

  ~SimplePCRE2Engine() noexcept
  {
    if (m_MatchData) {
      pcre2_match_data_free(m_MatchData);
    }
    if (m_CompiledPattern) {
      pcre2_code_free(m_CompiledPattern);
    }
  }

  // non-copyable
  SimplePCRE2Engine(const SimplePCRE2Engine&) = delete;
  SimplePCRE2Engine& operator=(const SimplePCRE2Engine&) = delete;

  ptrdiff_t Find(const char *pattern, const char *document, const bool caseSensitive, int *matchLen_out = nullptr);

private:

  std::string         m_CachedPattern;
  uint32_t            m_CompileOptions;
  pcre2_code*         m_CompiledPattern;
  pcre2_match_data*   m_MatchData;

  char                m_ErrorInfo[256];

};
// ============================================================================


// Translate \< \> word boundaries to PCRE2 lookaround equivalents
static std::string TranslateSimplePattern(const char* pattern, size_t patternLen)
{
  std::string result;
  result.reserve(patternLen + 32);
  for (size_t i = 0; i < patternLen; ++i) {
    if (pattern[i] == '\\' && (i + 1) < patternLen) {
      if (pattern[i + 1] == '\\') {
        result.push_back('\\');
        result.push_back('\\');
        ++i;
      }
      else if (pattern[i + 1] == '<') {
        result.append(R"((?<!\w)(?=\w))");
        ++i;
      }
      else if (pattern[i + 1] == '>') {
        result.append(R"((?<=\w)(?!\w))");
        ++i;
      }
      else {
        result.push_back(pattern[i]);
      }
    }
    else {
      result.push_back(pattern[i]);
    }
  }
  return result;
}


ptrdiff_t SimplePCRE2Engine::Find(const char* pattern, const char* document, const bool caseSensitive, int* matchLen_out /*=nullptr*/)
{
  if (!pattern || !*pattern) {
    return ptrdiff_t(-1);
  }
  if (!document || !*document) {
    return ptrdiff_t(-1);
  }

  auto const patternLen = strlen(pattern);
  auto const stringLen = strlen(document);

  // Build compile options
  uint32_t compileOptions = PCRE2_UTF | PCRE2_UCP | PCRE2_MULTILINE;
  if (!caseSensitive) {
    compileOptions |= PCRE2_CASELESS;
  }

  try {
    // Recompile only if pattern or options changed
    std::string translatedPattern = TranslateSimplePattern(pattern, patternLen);

    bool const bReCompile = (m_CompiledPattern == nullptr)
                            || (m_CompileOptions != compileOptions)
                            || (m_CachedPattern != translatedPattern);

    if (bReCompile) {
      if (m_MatchData) {
        pcre2_match_data_free(m_MatchData);
        m_MatchData = nullptr;
      }
      if (m_CompiledPattern) {
        pcre2_code_free(m_CompiledPattern);
        m_CompiledPattern = nullptr;
      }

      m_CachedPattern = translatedPattern;
      m_CompileOptions = compileOptions;
      m_ErrorInfo[0] = '\0';

      int errorcode = 0;
      PCRE2_SIZE erroroffset = 0;
      m_CompiledPattern = pcre2_compile(
        reinterpret_cast<PCRE2_SPTR>(m_CachedPattern.c_str()),
        m_CachedPattern.length(),
        m_CompileOptions,
        &errorcode,
        &erroroffset,
        nullptr
      );

      if (!m_CompiledPattern) {
        pcre2_get_error_message(errorcode, reinterpret_cast<PCRE2_UCHAR*>(m_ErrorInfo), sizeof(m_ErrorInfo));
        OutputDebugStringA("PCRE2 compile error (Simple): ");
        OutputDebugStringA(m_ErrorInfo);
        OutputDebugStringA("\n");
        m_CachedPattern.clear();
        return ptrdiff_t(-111);
      }

      m_MatchData = pcre2_match_data_create_from_pattern(m_CompiledPattern, nullptr);

      // JIT compile for performance
      pcre2_jit_compile(m_CompiledPattern, PCRE2_JIT_COMPLETE);
    }

    // Execute match
    int rc = pcre2_match(
      m_CompiledPattern,
      reinterpret_cast<PCRE2_SPTR>(document),
      stringLen,
      0,              // start at beginning
      PCRE2_NO_UTF_CHECK,
      m_MatchData,
      nullptr         // default match context
    );

    ptrdiff_t matchPos = ptrdiff_t(-1);
    ptrdiff_t matchLen = ptrdiff_t(0);

    if (rc > 0) // found
    {
      PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(m_MatchData);
      matchPos = static_cast<ptrdiff_t>(ovector[0]);
      matchLen = static_cast<ptrdiff_t>(ovector[1]) - matchPos;
    }
    else if (rc < 0 && rc != PCRE2_ERROR_NOMATCH)
    {
      pcre2_get_error_message(rc, reinterpret_cast<PCRE2_UCHAR*>(m_ErrorInfo), sizeof(m_ErrorInfo));
      OutputDebugStringA("PCRE2 match error (Simple): ");
      OutputDebugStringA(m_ErrorInfo);
      OutputDebugStringA("\n");
      matchPos = ptrdiff_t(-3);
    }

    if (matchLen_out) {
      *matchLen_out = static_cast<int>(matchLen);
    }
    return matchPos;
  }
  catch (...) {
    return ptrdiff_t(-666);
  }
}
// ============================================================================

extern "C"
#ifdef SCINTILLA_DLL
__declspec(dllexport)
#endif
ptrdiff_t WINAPI RegExFind(const char *pchPattern, const char *pchText, const bool caseSensitive, const int eolMode, int *matchLen_out) {

  UNREFERENCED_PARAMETER(eolMode);

  // Static cached engine: pattern is compiled once, reused across calls.
  // Only recompiles when pattern or options change.
  static SimplePCRE2Engine s_Engine;

  return s_Engine.Find(pchPattern, pchText, caseSensitive, matchLen_out);
}
// ============================================================================

#endif //SCI_OWNREGEX
