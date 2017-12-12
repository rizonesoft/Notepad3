/**
 * @file  OniguRegExEngine.cxx
 * @brief 
 * @autor Rainer Kottenhoff (RaiKoHoff)
 *
 * Install:
 */

#ifdef SCI_OWNREGEX

#include <stdlib.h>
#include <string>
#include <vector>

#define VC_EXTRALEAN 1
#include <windows.h>

#pragma warning( push )
#pragma warning( disable : 4996 )   // Scintilla's "unsafe" use of std::copy() (SplitVector.h)
 //                                  // or use -D_SCL_SECURE_NO_WARNINGS preprocessor define

#include "Platform.h"
#include "Scintilla.h"
#include "ILexer.h"
#include "SplitVector.h"
#include "Partitioning.h"
#include "CellBuffer.h"
#include "CaseFolder.h"
#include "RunStyles.h"
#include "Decoration.h"
#include "CharClassify.h"
#include "Document.h"
// ---------------------------------------------------------------
#include "oniguruma/oniguruma.h"   // Oniguruma - Regular Expression Engine (v6.7)
// ---------------------------------------------------------------

using namespace Scintilla;

#define SciPos(pos)    static_cast<Sci::Position>(pos)
#define SciLn(line)    static_cast<Sci::Line>(line)
#define SciPosExt(pos) static_cast<Sci_Position>(pos)

#define Cast2long(n)   static_cast<long>(n)

// ---------------------------------------------------------------

const int MAX_GROUP_COUNT = 10;

const OnigEncoding g_pEncodingType = ONIG_ENCODING_ASCII; // ONIG_ENCODING_SJIS
static OnigEncoding use_encs[] = { g_pEncodingType };

const bool gExtended = false;   // ignore spaces and use '#' as line-comment)

// ---------------------------------------------------------------

class OniguRegExEngine : public RegexSearchBase
{
public:

  explicit OniguRegExEngine(CharClassify* charClassTable)
    : m_RegExprStrg()
    , m_CmplOptions(ONIG_OPTION_DEFAULT)
    , m_pRegExpr(nullptr)
    , m_ErrorInfo()
    , m_MatchPos(ONIG_MISMATCH)
    , m_MatchLen(0)
    , m_SubstBuffer()
  {
    onig_initialize(use_encs, sizeof(use_encs) / sizeof(use_encs[0]));
    m_pRegion = onig_region_new();
  }

  virtual ~OniguRegExEngine()
  {
    if (m_pRegion)
      onig_region_free(m_pRegion, 1);

    onig_end();
  }

  virtual long FindText(Document* doc, Sci::Position minPos, Sci::Position maxPos, const char* pattern,
                        bool caseSensitive, bool word, bool wordStart, int flags, Sci::Position* length) override;

  virtual const char* SubstituteByPosition(Document* doc, const char* text, Sci::Position* length) override;


private:

  std::string& translateRegExpr(std::string& regExprStr, bool wholeWord, bool wordStart, int eolMode);
  std::string& convertReplExpr(std::string& replStr);

private:

  std::string m_RegExprStrg;

  OnigOptionType  m_CmplOptions;
  regex_t*        m_pRegExpr;
  OnigRegion*     m_pRegion;
  char            m_ErrorInfo[ONIG_MAX_ERROR_MESSAGE_LEN];

  Sci::Position   m_MatchPos;
  Sci::Position   m_MatchLen;

  std::string m_SubstBuffer;
};
// ============================================================================


RegexSearchBase *Scintilla::CreateRegexSearch(CharClassify *charClassTable)
{
  return new OniguRegExEngine(charClassTable);
}

// ============================================================================


/**
 * Find text in document, supporting both forward and backward
 * searches (just pass minPos > maxPos to do a backward search)
 * Has not been tested with backwards DBCS searches yet.
 */
long OniguRegExEngine::FindText(Document* doc, Sci::Position minPos, Sci::Position maxPos, const char *pattern,
                                bool caseSensitive, bool word, bool wordStart, int searchFlags, Sci::Position *length)
{

                                 // Range endpoints should not be inside DBCS characters, but just in case, move them.
  minPos = doc->MovePositionOutsideChar(minPos, 1, false);
  maxPos = doc->MovePositionOutsideChar(maxPos, 1, false);
  const bool findprevious = (minPos > maxPos);


  OnigOptionType cmplOptions = ONIG_OPTION_DEFAULT;
  cmplOptions |= (ONIG_OPTION_MULTILINE | ONIG_OPTION_CAPTURE_GROUP);  // the .(dot) does not match line-breaks
  cmplOptions |= (gExtended) ? ONIG_OPTION_EXTEND : ONIG_OPTION_NONE;
  cmplOptions |= (caseSensitive) ? ONIG_OPTION_NONE : ONIG_OPTION_IGNORECASE;

  std::string sRegExprStrg = translateRegExpr(std::string(pattern), word, wordStart, doc->eolMode);

  bool bReCompile = (m_CmplOptions != cmplOptions) || (m_RegExprStrg.compare(sRegExprStrg) != 0);

  if (bReCompile) {
    m_RegExprStrg.clear();
    m_RegExprStrg = sRegExprStrg;
    m_CmplOptions = cmplOptions;
    m_ErrorInfo[0] = '\0';
    try {
      OnigErrorInfo einfo;
      int result = onig_new(&m_pRegExpr, (UChar*)m_RegExprStrg.c_str(), (UChar*)(m_RegExprStrg.c_str() + m_RegExprStrg.length()),
                            m_CmplOptions, g_pEncodingType, ONIG_SYNTAX_DEFAULT, &einfo);
      if (result != 0) {
        onig_error_code_to_str((UChar*)m_ErrorInfo, result, &einfo);
        return Cast2long(-2);
      }
    }
    catch (...) {
      return Cast2long(-2);  // -1 is normally used for not found, -2 is used here for invalid regex
    }
  }


  UChar* docBegPtr = (UChar*)doc->RangePointer(0, SciPos(doc->Length()));
  UChar* docSEndPtr = (UChar*)doc->RangePointer(SciPos(doc->Length()),0);
  Sci::Position rangeLength = abs(maxPos - minPos);
  UChar* rangeBegPtr = (UChar*)doc->RangePointer((findprevious) ? maxPos : minPos, rangeLength);
  UChar* rangeEndPtr = (UChar*)doc->RangePointer((findprevious) ? minPos : maxPos, rangeLength);

  m_MatchPos = SciPos(ONIG_MISMATCH); // not found
  m_MatchLen = SciPos(0);

  int result = onig_search(m_pRegExpr, docBegPtr, docSEndPtr, rangeBegPtr, rangeEndPtr, m_pRegion, ONIG_OPTION_NONE);

  if (result < ONIG_MISMATCH) {
    onig_error_code_to_str((UChar*)m_ErrorInfo, result);
    return Cast2long(-2);
  }

  if (findprevious)  // search previous 
  {
    // search for last occurrence in range
    //SPEEDUP: onig_scan() ???
    while ((result >= 0) && (rangeBegPtr <= rangeEndPtr))
    {
      m_MatchPos = SciPos(m_pRegion->beg[0]);
      m_MatchLen = SciPos(m_pRegion->end[0] - m_pRegion->beg[0]);
      
      rangeBegPtr = docBegPtr + (m_MatchPos + m_MatchLen);

      result = onig_search(m_pRegExpr, docBegPtr, docSEndPtr, rangeBegPtr, rangeEndPtr, m_pRegion, ONIG_OPTION_NONE);
    }
  }
  else {
    if ((result >= 0) && (rangeBegPtr <= rangeEndPtr)) {
      m_MatchPos = SciPos(m_pRegion->beg[0]);
      m_MatchLen = SciPos(m_pRegion->end[0] - m_pRegion->beg[0]);
    }
  }

  //NOTE: potential 64-bit-size issue at interface here:
  *length = m_MatchLen;
  return static_cast<long>(m_MatchPos);
}
// ============================================================================


const char* OniguRegExEngine::SubstituteByPosition(Document* doc, const char* text, Sci::Position* length)
{
  //NOTE: potential 64-bit-size issue at interface here:
  *length = SciPos(0);
  return m_SubstBuffer.c_str();
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
/

/**
* Is the character an octal digit?
*/
static bool IsOctalDigit(char ch) {
  return ch >= '0' && ch <= '7';
}
// ----------------------------------------------------------------------------

/**
* If the character is an hexa digit, get its value.
*/
static int GetHexDigit(char ch) {
  if (ch >= '0' && ch <= '9') {
    return ch - '0';
  }
  if (ch >= 'A' && ch <= 'F') {
    return ch - 'A' + 10;
  }
  if (ch >= 'a' && ch <= 'f') {
    return ch - 'a' + 10;
  }
  return -1;
}
// ----------------------------------------------------------------------------


static void replaceAll(std::string& source,const std::string& from,const std::string& to)
{
  std::string newString;
  newString.reserve(source.length() * 2);  // avoids a few memory allocations

  std::string::size_type lastPos = 0;
  std::string::size_type findPos;

  while (std::string::npos != (findPos = source.find(from,lastPos))) {
    newString.append(source,lastPos,findPos - lastPos);
    newString += to;
    lastPos = findPos + from.length();
  }
  // Care for the rest after last occurrence
  newString += source.substr(lastPos);

  source.swap(newString);
}
// ----------------------------------------------------------------------------



std::string& OniguRegExEngine::translateRegExpr(std::string& regExprStr, bool wholeWord, bool wordStart, int eolMode)
{
  std::string	tmpStr;

  if (wholeWord || wordStart) {      // push '\b' at the begin of regexpr
    tmpStr.push_back('\\');
    tmpStr.push_back('b');
    tmpStr.append(regExprStr);
    if (wholeWord) {               // push '\b' at the end of regexpr
      tmpStr.push_back('\\');
      tmpStr.push_back('b');
    }
    replaceAll(tmpStr, ".", R"(\w)");
  }
  else {
    tmpStr.append(regExprStr);
  }

  switch (eolMode) {
  case SC_EOL_LF:
    // we are fine here
    break;

  case SC_EOL_CR:
    //TODO: don't know what to do here ...
    break;

  case SC_EOL_CRLF:
    {
      //replaceAll(tmpStr, "$", R"(\r$)");
      //replaceAll(tmpStr, R"(\\r$)", R"(\$)");
    }
    break;
  }

  std::swap(regExprStr, tmpStr);
  return regExprStr;
}
// ----------------------------------------------------------------------------



std::string& OniguRegExEngine::convertReplExpr(std::string& replStr)
{
  std::string	tmpStr;
  for (size_t i = 0; i < replStr.length(); ++i) {
    char ch = replStr[i];
    if (ch == '\\') {
      ch = replStr[++i]; // next char
      if (ch >= '1' && ch <= '9') {
        // former behavior convenience: 
        // change "\\<n>" to deelx's group reference ($<n>)
        tmpStr.push_back('$');
      }
      switch (ch) {
        // check for escape seq:
      case 'a':
        tmpStr.push_back('\a');
        break;
      case 'b':
        tmpStr.push_back('\b');
        break;
      case 'f':
        tmpStr.push_back('\f');
        break;
      case 'n':
        tmpStr.push_back('\n');
        break;
      case 'r':
        tmpStr.push_back('\r');
        break;
      case 't':
        tmpStr.push_back('\t');
        break;
      case 'v':
        tmpStr.push_back('\v');
        break;
      case '\\':
        tmpStr.push_back('\\'); // preserve escd "\"
        tmpStr.push_back('\\'); 
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
            val[0] = (WCHAR)hex;
            hex = GetHexDigit(replStr[i]);
            if (hex >= 0) {
              ++i;
              val[0] *= 16;
              val[0] += (WCHAR)hex;
              if (!bShort) {
                hex = GetHexDigit(replStr[i]);
                if (hex >= 0) {
                  ++i;
                  val[0] *= 16;
                  val[0] += (WCHAR)hex;
                  hex = GetHexDigit(replStr[i]);
                  if (hex >= 0) {
                    ++i;
                    val[0] *= 16;
                    val[0] += (WCHAR)hex;
                  }
                }
              }
            }
            if (val[0]) {
              val[1] = 0;
              WideCharToMultiByte(CP_UTF8, 0, val, -1, buf, ARRAYSIZE(val), nullptr, nullptr);
              tmpStr.push_back(*pch++);
              while (*pch)
                tmpStr.push_back(*pch++);
            }
            else
              tmpStr.push_back(ch); // unknown ctrl seq
          }
          else
            tmpStr.push_back(ch); // unknown ctrl seq
        }
        break;

      default:
        tmpStr.push_back(ch); // unknown ctrl seq
        break;
      }
    }
    else {
      tmpStr.push_back(ch);
    }
  } //for

  std::swap(replStr,tmpStr);
  return replStr;
}
// ============================================================================

#pragma warning( pop )

#endif //SCI_OWNREGEX
