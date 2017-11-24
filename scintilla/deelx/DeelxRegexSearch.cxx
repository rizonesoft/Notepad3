/**
 * @file  DeelxRegexSearch.cxx
 * @brief integrate DeelX regex searching for Scintilla library
 *              (Scintilla Lib is copyright 1998-2016 by Neil Hodgson <neilh@scintilla.org>)
 *
 *        uses DEELX - Regular Expression Engine (v1.3) (deelx.h) - http://www.regexlab.com/deelx/
 *               download: http://www.regexlab.com/download/deelx/deelx.zip  (v1.2)
 *               or      : https://github.com/AndreasMartin72/mksqlite/blob/master/deelx/deelx.h  (v1.3)
 *               (Copyright Announcement: Free to use/redistribute. Provenance must be declared when redistributed)
 *               API documentation see accompanying "deelx_en.chm" HTML Help.
 *
 * @autor Rainer Kottenhoff (RaPeHoff)
 *
 * Install:
 *   - place files (deelx64.h, DeelxRegexSearch.cxx, deelx_en.chm)
 *       in a directory (deelx) within the scintilla project (.../scintilla/deelx/)
 *   - add source files to scintilla project (Scintilla.vcxproj in VS)
 *   - define compiler (preprocessor) macro for scintilla project named "SCI_OWNREGEX"
 *       -> this will switch from scintilla's buildin regex engine to deelx's regex engine
 *   - recompile and link scintilla library
 *   - build application
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
//#include "deelx.h"   // DEELX - Regular Expression Engine (v1.2)
#include "deelx64.h"   // DEELX - Regular Expression Engine (v1.3)
// ---------------------------------------------------------------

using namespace Scintilla;

#define SciPos(pos)    static_cast<Sci::Position>(pos)
#define SciLn(line)    static_cast<Sci::Line>(line)
#define SciPosExt(pos) static_cast<Sci_Position>(pos)

#define DeelXPos(pos)  static_cast<deelx::index_t>(pos)
#define Cast2long(n)   static_cast<long>(n)

// ---------------------------------------------------------------

const int MAX_GROUP_COUNT = 10;

// ---------------------------------------------------------------

class DeelxRegexSearch : public RegexSearchBase
{
public:

  explicit DeelxRegexSearch(CharClassify* charClassTable)
    : m_RegExprStrg()
    , m_CompileFlags(-1)
    , m_RegExpr()
    , m_Match()
    , m_MatchPos(-1)
    , m_MatchLength(0)
    , m_SubstBuffer()
  {}

  virtual ~DeelxRegexSearch()
  {
    ReleaseSubstitutionBuffer();
    m_RegExprStrg.clear();
  }

  virtual long FindText(Document* doc, Sci::Position minPos, Sci::Position maxPos, const char* pattern,
                        bool caseSensitive, bool word, bool wordStart, int flags, Sci::Position* length) override;

  virtual const char* SubstituteByPosition(Document* doc, const char* text, Sci::Position* length) override;


private:

  __inline void ReleaseSubstitutionBuffer()
  {
    //if (m_SubstitutionBuffer) {
    //  m_RegExpr.ReleaseString(m_SubstitutionBuffer);
    //  m_SubstitutionBuffer = nullptr;
    //}
  }

  std::string& translateRegExpr(std::string& regExprStr, bool wholeWord, bool wordStart);
  std::string& convertReplExpr(std::string& replStr);

private:

  std::string m_RegExprStrg;
  int m_CompileFlags;
  deelx::CRegexpT<char> m_RegExpr;
  deelx::MatchResult m_Match;
  Sci::Position m_MatchPos;
  Sci::Position m_MatchLength;
  std::string m_SubstBuffer;
};
// ============================================================================


RegexSearchBase *Scintilla::CreateRegexSearch(CharClassify *charClassTable)
{
  return new DeelxRegexSearch(charClassTable);
}

// ============================================================================


/**
 * Find text in document, supporting both forward and backward
 * searches (just pass minPos > maxPos to do a backward search)
 * Has not been tested with backwards DBCS searches yet.
 */
long DeelxRegexSearch::FindText(Document* doc, Sci::Position minPos, Sci::Position maxPos, const char *pattern,
                                bool caseSensitive, bool word, bool wordStart, int searchFlags, Sci::Position *length)
{
  const bool right2left = false; // always left-to-right match mode
  const bool extended = false;   // ignore spaces and use '#' as line-comment)

  // Range endpoints should not be inside DBCS characters, but just in case, move them.
  minPos = doc->MovePositionOutsideChar(minPos,1,false);
  maxPos = doc->MovePositionOutsideChar(maxPos,1,false);
  const bool findprevious = (minPos > maxPos);

  int compileFlags = deelx::NO_FLAG;
  compileFlags |= (deelx::MULTILINE | deelx::GLOBAL); // the .(dot) does not match line-breaks
  //compileFlags |= (deelx::SINGLELINE | deelx::MULTILINE | deelx::GLOBAL);  // the .(dot) also matches line-breaks

  compileFlags |= (extended) ? deelx::EXTENDED : deelx::NO_FLAG;
  compileFlags |= (caseSensitive) ? deelx::NO_FLAG : deelx::IGNORECASE;
  compileFlags |= (right2left) ? deelx::RIGHTTOLEFT : deelx::NO_FLAG;

  std::string sRegExprStrg = translateRegExpr(std::string(pattern),word,wordStart);

  bool bReCompile = (m_CompileFlags != compileFlags) || (m_RegExprStrg.compare(sRegExprStrg) != 0);

  if (bReCompile) {
    m_RegExprStrg = sRegExprStrg;
    m_CompileFlags = compileFlags;
    try {
      m_RegExpr.Compile(m_RegExprStrg.c_str(), m_CompileFlags);
    }
    catch (...) {
      return -2;  // -1 is normally used for not found, -2 is used here for invalid regex
      // DeelX is very fault tolerant and assumes what the user may want ... :-/
      // so -2 may not occur!
    }
  }

  Sci::Position rangeBegin = (findprevious) ? maxPos : minPos;
  Sci::Position rangeEnd   = (findprevious) ? minPos : maxPos;
  
  Sci::Line     linesTotal  = doc->LinesTotal();
  Sci::Position fileLastPos = SciPos(doc->Length());

  Sci::Line lineOfBegPos = SciLn(doc->LineFromPosition(SciPosExt(rangeBegin)));
  Sci::Line lineOfEndPos = SciLn(doc->LineFromPosition(SciPosExt(rangeEnd)));

  Sci::Position lineStartOfBegPos = SciPos(doc->LineStart(SciPosExt(lineOfBegPos)));
  Sci::Position lineEndOfEndPos   = SciPos(doc->LineEnd(SciPosExt(lineOfEndPos)));

  // --- adapt range start/end according to search pattern ---

  size_t begMetaPos = m_RegExprStrg.find_first_of('^');
  bool bFoundBegMeta = (begMetaPos != std::string::npos) && 
                       ((begMetaPos == 0) || (m_RegExprStrg.find_first_of('\\') != (begMetaPos - 1)));
  if (bFoundBegMeta) {
    if (lineStartOfBegPos != rangeBegin) {
      rangeBegin = SciPos((lineOfBegPos < linesTotal) ? 
                          doc->LineStart(SciPosExt(lineOfBegPos + 1)) : 
                          doc->LineEnd(SciPosExt(linesTotal)));
      rangeEnd   = (rangeBegin <= rangeEnd) ? rangeEnd : rangeBegin;
    }
  }

  size_t endMetaPos = m_RegExprStrg.find_last_of('$');
  bool bFoundEndMeta = (endMetaPos != std::string::npos) && 
                       ((endMetaPos == 0) || (m_RegExprStrg.find_last_of('\\') != (endMetaPos - 1)));
  if (bFoundEndMeta) {
    if (lineEndOfEndPos != rangeEnd) {
      rangeEnd   = SciPos((0 < lineOfEndPos) ? doc->LineEnd(SciPosExt(lineOfEndPos - 1)) : 0);
      rangeBegin = (rangeBegin <= rangeEnd) ? rangeBegin : rangeEnd;
    }
  }

  // ---  start search  ---

  m_MatchPos    = SciPos(-1); // not found
  m_MatchLength = SciPos(0);
  const deelx::index_t searchStop = DeelXPos(rangeEnd);

  if (findprevious)  // search previous 
  {
    deelx::CContext* pContext = m_RegExpr.PrepareMatch(doc->RangePointer(0, fileLastPos), 0);
    m_Match = m_RegExpr.Match(pContext);
    // search for last occurrence in range
    while (m_Match.IsMatched() && (m_Match.GetStart() < searchStop))
    {
      m_MatchPos = SciPos(m_Match.GetStart());
      m_MatchLength = SciPos(m_Match.GetEnd() - m_Match.GetStart());
      m_Match = m_RegExpr.Match(pContext); //next
    }
    m_RegExpr.ReleaseContext(pContext);
  }
  else {
    m_Match = m_RegExpr.Match(doc->RangePointer(0, fileLastPos), 
                              DeelXPos(fileLastPos), DeelXPos(rangeBegin));
    if (m_Match.IsMatched() && (m_Match.GetStart() < searchStop)) {
      m_MatchPos = SciPos(m_Match.GetStart());
      m_MatchLength = SciPos(m_Match.GetEnd() - m_Match.GetStart());
    }
  }

  //NOTE: potential 64-bit-size issue at interface here:
  *length = SciPos(m_MatchLength);
  return static_cast<long>(m_MatchPos);
}
// ============================================================================




#if 0

#define _MAX(a,b) ((a)>(b)?(a):(b))
#define _MIN(a,b) ((a)<(b)?(a):(b))


const char* DeelxRegexSearch::SubstituteByPosition(Document* doc, const char* text, Sci::Position* length)
{
  if (!m_Match.IsMatched() || (m_MatchPos < 0)) {
    *length = SciPos(0);
    return nullptr;
  }

  Sci::Position fileLastPos = SciPos(doc->Length());

  std::string rawReplStrg = convertReplExpr(std::string(text,*length));
  deelx::index_t rawReplLength = DeelXPos(rawReplStrg.length());

  // document slice with look-around context (+/- MatchLength ???)
  Sci::Position looka = m_MatchLength;   // what should be the size of the lookahead ???

  Sci::Position begin = _MAX(0, (m_MatchPos - looka));
  Sci::Position end   = _MIN((m_MatchPos + (2 * looka)), fileLastPos);
  Sci::Position len   = (end - begin);
  const char* pDocumentSlice = doc->RangePointer(begin, len);

  deelx::index_t relMatchPos = DeelXPos(m_MatchPos - begin);
  deelx::index_t relSliceLen = DeelXPos(len);

  ReleaseSubstitutionBuffer();
  deelx::index_t resLength_out = rawReplLength; // init
  m_SubstitutionBuffer = m_RegExpr.Replace(pDocumentSlice, relSliceLen, rawReplStrg.c_str(), rawReplLength, resLength_out, relMatchPos, 1, &m_Match, nullptr);

  //NOTE: potential 64-bit-size issue at interface here:
  deelx::index_t realReplLength = SciPos(resLength_out - relSliceLen + m_MatchLength);
  *length = SciPos(realReplLength);

  // cut out replacement sub-string only !!!
  m_SubstitutionBuffer[relMatchPos + realReplLength + 1] = '\0';
  return &(m_SubstitutionBuffer[relMatchPos]);
}
// ============================================================================
#endif



const char* DeelxRegexSearch::SubstituteByPosition(Document* doc, const char* text, Sci::Position* length)
{
  if (m_Match.IsMatched() == 0) {
    *length = SciPos(-1);
    return nullptr;
  }

  std::string rawReplStrg = convertReplExpr(std::string(text, *length));

  m_SubstBuffer.clear();

  for (int j = 0; j < rawReplStrg.length(); j++) {
    if ((rawReplStrg[j] == '$') || (rawReplStrg[j] == '\\'))
    {
      if ((m_Match.IsMatched() != 0) && (rawReplStrg[j + 1] >= '0') && (rawReplStrg[j + 1] <= '9'))
      {
        unsigned int grpNum = rawReplStrg[j + 1] - '0';

        if (grpNum <= m_Match.MaxGroupNumber()) 
        {
          deelx::index_t gStart = m_Match.GetGroupStart(grpNum);
          deelx::index_t len = m_Match.GetGroupEnd(grpNum) - gStart;

          m_SubstBuffer.append(doc->RangePointer(SciPos(gStart), SciPos(len)), len);
        }
        ++j;
      }
      else if (rawReplStrg[j] == '\\') {
        m_SubstBuffer.push_back('\\');
        ++j;
      }
      else {
        m_SubstBuffer.push_back(rawReplStrg[j]);
      }
    }
    else {
      m_SubstBuffer.push_back(rawReplStrg[j]);
    }
  }

  //NOTE: potential 64-bit-size issue at interface here:
  *length = SciPos(m_SubstBuffer.length());
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



std::string& DeelxRegexSearch::translateRegExpr(std::string& regExprStr,bool wholeWord,bool wordStart)
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
    replaceAll(tmpStr,".",R"(\w)");
  }
  else {
    tmpStr.append(regExprStr);
  }
  std::swap(regExprStr,tmpStr);
  return regExprStr;
}
// ----------------------------------------------------------------------------



std::string& DeelxRegexSearch::convertReplExpr(std::string& replStr)
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
              WideCharToMultiByte(CP_UTF8, 0, val, -1, buf, ARRAYSIZE(val), NULL, NULL);
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
