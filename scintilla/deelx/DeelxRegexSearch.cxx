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
#include "deelx64.h"   // DEELX - Regular Expression Engine (v1.3)
// ---------------------------------------------------------------

#ifdef SCI_NAMESPACE
using namespace Scintilla;
#endif

class DeelxRegexSearch : public RegexSearchBase
{
public:

    explicit DeelxRegexSearch(CharClassify* charClassTable)
        : m_RegExpr()
        , m_Match()
        , m_MatchPos(-1)
        , m_MatchLength(0)
        , m_pContext(nullptr)
        , m_SubstitutionBuffer(nullptr)
    {}

    virtual ~DeelxRegexSearch()
    {
        ReleaseSubstitutionBuffer();
        ReleaseContext();
    }

    virtual long FindText(Document* doc, int minPos, int maxPos, const char* pattern,
                          bool caseSensitive, bool word, bool wordStart, int flags, int* length) override;

    virtual const char* SubstituteByPosition(Document* doc, const char* text, int* length) override;


private:

    inline void ReleaseContext()
    {
        if (m_pContext != nullptr) {
            m_RegExpr.ReleaseContext(m_pContext);
            m_pContext = nullptr;
        }
    }

    inline void ReleaseSubstitutionBuffer()
    {
        if (m_SubstitutionBuffer) {
            m_RegExpr.ReleaseString(m_SubstitutionBuffer);
            m_SubstitutionBuffer = nullptr;
        }
    }

private:
    deelx::CRegexpT<char> m_RegExpr;
    deelx::MatchResult m_Match;
    deelx::index_t m_MatchPos;
    deelx::index_t m_MatchLength;
    deelx::CContext* m_pContext;
    char* m_SubstitutionBuffer;
};
// ============================================================================


#ifdef SCI_NAMESPACE
RegexSearchBase *Scintilla::CreateRegexSearch(CharClassify *charClassTable)
{
    return new DeelxRegexSearch(charClassTable);
}
#else
RegexSearchBase *CreateRegexSearch(CharClassify *charClassTable)
{
    return new DeelxRegexSearch(charClassTable);
}
#endif

// ============================================================================

/**
 * forward declaration of utility functions
 */
std::string& translateRegExpr(std::string& regExprStr, bool wholeWord, bool wordStart);
std::string& convertReplExpr(std::string& replStr);


// ============================================================================


/**
 * Find text in document, supporting both forward and backward
 * searches (just pass minPos > maxPos to do a backward search)
 * Has not been tested with backwards DBCS searches yet.
 */
long DeelxRegexSearch::FindText(Document* doc, int minPos, int maxPos, const char *pattern,
                                bool caseSensitive, bool word, bool wordStart, int searchFlags, int *length)
{
    int startPos, endPos;
    bool left2right;

    if (minPos <= maxPos) {
        left2right = true;
        startPos = minPos;
        endPos = maxPos;
    }
    else { // backward search
        left2right = false;
        startPos = maxPos;
        endPos = minPos;
    }

    // Range endpoints should not be inside DBCS characters, but just in case, move them.
    startPos = doc->MovePositionOutsideChar(startPos, 1, false);
    endPos = doc->MovePositionOutsideChar(endPos, 1, false);

    int compileFlags(deelx::MULTILINE | deelx::GLOBAL | deelx::EXTENDED); // the .(dot) does not match line-breaks
    //int compileFlags(deelx::SINGLELINE | deelx::MULTILINE | deelx::GLOBAL | deelx::EXTENDED);  // the .(dot) also matches line-breaks
    compileFlags |= (caseSensitive) ? deelx::NO_FLAG : deelx::IGNORECASE;
    compileFlags |= (left2right) ? deelx::NO_FLAG : deelx::RIGHTTOLEFT;

    std::string sRegExprStrg = translateRegExpr(std::string(pattern, *length), word, wordStart);

    try {
        m_RegExpr.Compile(sRegExprStrg.c_str(), compileFlags);
    }
    catch (...) {
        return -2;  // -1 is normally used for not found, -2 is used here for invalid regex
    }

    int rangeLen = endPos - startPos;
    int searchStartPos = left2right ? 0 : rangeLen;
    ReleaseContext();
    m_pContext = m_RegExpr.PrepareMatch(doc->RangePointer(startPos, rangeLen), searchStartPos);

    m_Match = m_RegExpr.Match(m_pContext);

    m_MatchPos = -1; // not found
    m_MatchLength = 0;
    if (m_Match.IsMatched()) {
        m_MatchPos = startPos + m_Match.GetStart();
        m_MatchLength = (m_Match.GetEnd() - m_Match.GetStart());
    }

    //NOTE: potential 64-bit-size issue at interface here:
    *length = static_cast<int>(m_MatchLength);
    return static_cast<long>(m_MatchPos);
}
// ============================================================================


const char* DeelxRegexSearch::SubstituteByPosition(Document* doc, const char* text, int* length)
{
    if (!m_Match.IsMatched() || (m_MatchPos < 0)) {
        *length = 0;
        return nullptr;
    }
    std::string sReplStrg = convertReplExpr(std::string(text, *length));

    //NOTE: potential 64-bit-size issue at interface here:
    const char* pString = doc->RangePointer(static_cast<int>(m_MatchPos), static_cast<int>(m_MatchLength));

    deelx::index_t resLength;
    ReleaseSubstitutionBuffer();
    m_SubstitutionBuffer = m_RegExpr.Replace(pString, m_MatchLength, sReplStrg.c_str(),
                                             static_cast<deelx::index_t>(sReplStrg.length()), resLength);

    //NOTE: potential 64-bit-size issue at interface here:
    *length = static_cast<int>(resLength);

    return m_SubstitutionBuffer;
}
// ============================================================================




// ============================================================================
//   Some Helpers
// ============================================================================


void replaceAll(std::string& source, const std::string& from, const std::string& to)
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



std::string& translateRegExpr(std::string& regExprStr, bool wholeWord, bool wordStart)
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
        replaceAll(tmpStr, ".", "\\w");
    }
    else {
        tmpStr.append(regExprStr);
    }
    std::swap(regExprStr, tmpStr);
    return regExprStr;
}
// ----------------------------------------------------------------------------



std::string& convertReplExpr(std::string& replStr)
{
    std::string	tmpStr;
    for (size_t i = 0; i < replStr.length(); ++i) {
        char ch = replStr[i];
        if (ch == '\\') {
            ch = replStr[++i]; // next char
            if (ch == '\\') {
                // skip 2nd backslash ("\\")
                if (i < replStr.length()) { ch = replStr[++i]; }
                else { break; }
            }
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
                tmpStr.push_back('\\');
                break;
            default:
                // unknown ctrl seq
                tmpStr.push_back(ch);
                break;
            }
        }
        else {
            tmpStr.push_back(ch);
        }
    } //for

    std::swap(replStr, tmpStr);
    return replStr;
}
// ============================================================================

#pragma warning( pop )

#endif //SCI_OWNREGEX
