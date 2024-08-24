// encoding: UTF-8
// Scintilla source code edit control
/** @file LexTOML.cxx
** Lexer for TOML (Tom's Obvious Markup Language -> Tom's Obvious, Minimal Language
** Written by RaiKoHoff
** TOML Spec: https://github.com/toml-lang/toml
**/

#include <string>
#include <assert.h>
#include <map>
//
#include "ILexer.h"
#include "Scintilla.h"
#include "StringCopy.h"
#include "PropSetSimple.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "LexerModule.h"
#include "DefaultLexer.h"
#include "OptionSet.h"
#include "WordList.h"
//
#include "CharSetX.h"
#include "SciXLexer.h"


using namespace Lexilla;
using namespace Scintilla;

namespace
{
// Use an unnamed namespace to protect the functions and classes from name conflicts

enum DataType
{
    String,
    Integer,
    Float,
    Boolean,
    Datetime,
    Array,
    Table,
    Unknown
};

int GetDataTypeStyle(const int numType)
{
    if (numType == DataType::Unknown)
    {
        return SCE_TOML_PARSINGERROR;
    }
    return SCE_TOML_VALUE;
}

struct OptionsTOML
{
    bool fold;
    bool foldCompact;
    OptionsTOML()
        : fold(true)
        , foldCompact(true)
    { }
};

static const char* const tomlWordListsDesc[] =
{
    "Keyword",
    nullptr
};

struct OptionSetTOML : public OptionSet<OptionsTOML>
{
    OptionSetTOML()
    {

        DefineProperty("fold", &OptionsTOML::fold);
        DefineProperty("fold.compact", &OptionsTOML::foldCompact);

        DefineWordListSets(tomlWordListsDesc);
    }
};

LexicalClass lexicalClasses[] =
{
    // Lexer TOML SCLEX_TOML SCE_TOML_:
    0,  "SCE_TOML_DEFAULT",        "default",              "Default",
    1,  "SCE_TOML_KEYWORD",        "keyword",              "Keyword",
    2,  "SCE_TOML_COMMENT",        "comment",              "Comment",
    3,  "SCE_TOML_SECTION",        "section",              "Section",
    4,  "SCE_TOML_KEY",            "key",                  "Key",
    5,  "SCE_TOML_ASSIGNMENT",     "assignment",           "Assignment",
    6,  "SCE_TOML_VALUE",          "value",                "Value",
    7,  "SCE_TOML_NUMBER",         "number",               "Number",
    8,  "SCE_TOML_STR_BASIC",      "string_basic",         "Basic String",
    9,  "SCE_TOML_STR_LITERAL",    "string_literal",       "Literal String",
    10, "SCE_TOML_PARSINGERROR",   "parsing_error",        "Parsing Error",
};

} // end of namespace

class LexerTOML : public DefaultLexer
{

    CharacterSet validKey;
    CharacterSet validKeyWord;
    CharacterSet validNumberEnd;
    CharacterSet chDateTime;

    WordList keywords;

    OptionsTOML options;
    OptionSetTOML osTOML;

public:
    LexerTOML()
        : DefaultLexer("TOML", SCLEX_TOML, lexicalClasses, ELEMENTS(lexicalClasses))
        , validKey(CharacterSet::setAlphaNum, R"(-_.)", 0x80, false)
        , validKeyWord(CharacterSet::setAlphaNum, "_+-", 0x80, false)
        , validNumberEnd(CharacterSet::setNone, " \t\n\v\f\r#,)}]", 0x80, false)
        , chDateTime(CharacterSet::setNone, "-+.:TZ", 0x80, false)
    { }

    virtual ~LexerTOML() { }

    void SCI_METHOD Release() override
    {
        delete this;
    }

    int SCI_METHOD Version() const override
    {
        return lvRelease5;
    }

    const char* SCI_METHOD PropertyNames() override
    {
        return osTOML.PropertyNames();
    }

    int SCI_METHOD PropertyType(const char* name) override
    {
        return osTOML.PropertyType(name);
    }

    const char* SCI_METHOD PropertyGet(const char* name) override
    {
        return osTOML.PropertyGet(name);
    }

    const char* SCI_METHOD DescribeProperty(const char* name) override
    {
        return osTOML.DescribeProperty(name);
    }


    const char* SCI_METHOD DescribeWordListSets() override
    {
        return osTOML.DescribeWordListSets();
    }

    void* SCI_METHOD PrivateCall(int, void*) override
    {
        return nullptr;
    }

    int SCI_METHOD LineEndTypesSupported() override
    {
        //return SC_LINE_END_TYPE_UNICODE;
        return SC_LINE_END_TYPE_DEFAULT;
    }

    int SCI_METHOD PrimaryStyleFromStyle(int style) override
    {
        return style;
    }

    static ILexer5* LexerFactoryTOML()
    {
        return new LexerTOML();
    }

    // --------------------------------------------------------------------------

    Sci_Position SCI_METHOD PropertySet(const char* key, const char* val) override;
    Sci_Position SCI_METHOD WordListSet(int n, const char* wl) override;
    void SCI_METHOD Lex(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument* pAccess) override;
    void SCI_METHOD Fold(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument* pAccess) override;

};


Sci_Position SCI_METHOD LexerTOML::PropertySet(const char* key, const char* val)
{
    if (osTOML.PropertySet(&options, key, val))
    {
        return 0;
    }
    return -1;
}


Sci_Position SCI_METHOD LexerTOML::WordListSet(int n, const char* wl)
{
    WordList* wordListN = nullptr;

    switch (n)
    {
    case 0:
        wordListN = &keywords;
        break;
    }

    Sci_Position firstModification = -1;
    if (wordListN) {
        if (wordListN->Set(wl)) {
            firstModification = 0;
        }
    }
    return firstModification;
}
// ----------------------------------------------------------------------------

constexpr bool IsCommentChar(const int ch) noexcept
{
    //return (ch == '#') || (ch == ':');
    return (ch == '#');
}
// ----------------------------------------------------------------------------

constexpr bool IsAssignChar(const int ch) noexcept
{
    //return (ch == '=') || (ch == ':');
    return (ch == '=');
}
// ----------------------------------------------------------------------------

constexpr bool IsAIdentifierChar(const int ch) noexcept
{
    return (IsAlphaNumeric(ch) || ch == '_' || ch == '.');
}
// ----------------------------------------------------------------------------

constexpr bool IsAKeywordChar(const int ch) noexcept
{
    return (IsAIdentifierChar(ch) || ch == '+' || ch == '-');
}
// ----------------------------------------------------------------------------

inline void SetStateParsingError(StyleContext& sc)
{
    sc.SetState(SCE_TOML_PARSINGERROR);
}
// ----------------------------------------------------------------------------

inline void ForwardSetStateParsingError(StyleContext& sc)
{
    sc.ForwardSetState(SCE_TOML_PARSINGERROR);
}
// ----------------------------------------------------------------------------



static bool IsDateTimeStr(StyleContext& sc, const CharacterSet& validCh, const CharacterSet& valEnd)
{
    auto const posCurrent = static_cast<Sci_Position>(sc.currentPos);
    auto const posEnd = static_cast<Sci_Position>(sc.lineStartNext);

    bool bDateTimeFlag = false;

    Sci_Position i = 0;
    while ((++i + posCurrent) < posEnd)
    {
        int const ch = sc.GetRelative(i);

        if (!Lexilla::IsADigit(ch) && !validCh.Contains(ch) && (ch != '.'))
        {
            if (valEnd.Contains(ch))
            {
                return bDateTimeFlag;
            }
            else
            {
                return false;
            }
        }
        if (validCh.Contains(ch))
        {
            bDateTimeFlag = true;
        }
    }
    return bDateTimeFlag;
}
// ----------------------------------------------------------------------------


static bool IsLookAheadLineEmpty(StyleContext& sc)
{
    auto const posRelEnd = static_cast<Sci_Position>(sc.lineStartNext) -
                           static_cast<Sci_Position>(sc.currentPos);

    bool bLHLineEmpty = true;

    Sci_Position i = 0;
    while (++i < posRelEnd)
    {
        int const ch = sc.GetRelative(i);
        if (!Lexilla::IsASpace(ch))
        {
            if (IsCommentChar(ch))
            {
                break; // ignore rest of line
            }
            bLHLineEmpty = false;
            break;
        }
    }
    return bLHLineEmpty;
}
// ----------------------------------------------------------------------------

static bool IsLookAheadInList(StyleContext& sc, const CharacterSet& validCh, const WordList& keywords)
{
    auto const posCurrent = static_cast<Sci_Position>(sc.currentPos);
    auto const posEnd = static_cast<Sci_Position>(sc.lineStartNext);

    static char identifier[1024] = { '\0' };

    int j = 0;
    Sci_Position i = -1;
    while (((++i + posCurrent) < posEnd) && (j < 1023))
    {
        int const ch = sc.GetRelative(i);

        if (IsASpaceOrTab(ch))
        {
            if (j == 0)
            {
                continue;
            }
        }
        if (validCh.Contains(ch))
        {
            identifier[j++] = static_cast<char>(ch);
            continue;
        }
        identifier[j] = '\0';
        break;
    }

    if (identifier[0] != '\0')
    {
        TrimIdentifier(identifier, identifier);
        if (keywords.InList(identifier))
        {
            return true;
        }
    }
    return false;
}
// ----------------------------------------------------------------------------

//@@@ replace by state
constexpr bool _isQuoted(const bool q1, const bool q2) noexcept
{
    return (q1 || q2);
}

// ----------------------------------------------------------------------------

void SCI_METHOD LexerTOML::Lex(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument* pAccess)
{
    PropSetSimple props;
    Accessor styler(pAccess, &props);
    StyleContext sc(startPos, length, initStyle, styler);

    bool inSQuotedKey = false;
    bool inDQuotedKey = false;

    bool inMultiLnArrayDef = false;

    bool bMultiLineStrg = false;
    int  iSqrBracketLevel = 0;

    bool inHex = false;
    bool inBin = false;
    bool inOct = false;

    bool bPossibleKeyword = true;
    bool bInInlBracket = false;

    for (; sc.More(); sc.Forward())
    {
        if (sc.atLineStart)
        {
            inSQuotedKey = inDQuotedKey = false;
            bPossibleKeyword = true;
            bInInlBracket = false;
            bMultiLineStrg = false;
            iSqrBracketLevel = 0;

            if (sc.currentLine > 0)
            {
                int const iLineState = styler.GetLineState(sc.currentLine - 1);
                // 0-7:  MultiLineStr
                // 8-15: SqrBraLevel
                bMultiLineStrg =  ((iLineState >> 0) & 0xFF) == 0 ? false : true;
                iSqrBracketLevel = (iLineState >> 8) & 0xFF;

                inMultiLnArrayDef = (iSqrBracketLevel > 0);
            }

            if (inMultiLnArrayDef)
            {
                switch (sc.state)
                {
                case SCE_TOML_COMMENT:
                    sc.SetState(SCE_TOML_VALUE); // continue value
                    break;
                default:
                    break;
                }
            }
            else   // NOT in inMultiLnArrayDef
            {

                switch (sc.state)
                {
                case SCE_TOML_KEY:
                case SCE_TOML_ASSIGNMENT:
                    SetStateParsingError(sc);
                    break;

                case SCE_TOML_STR_BASIC:
                case SCE_TOML_STR_LITERAL:
                    if (!bMultiLineStrg)
                    {
                        SetStateParsingError(sc);
                    }
                    break;

                case SCE_TOML_PARSINGERROR:  // reset
                default:
                    sc.SetState(SCE_TOML_DEFAULT);
                    break;
                }
            }
        }

        // ------------------------------------------------------------------------

        if (sc.atLineEnd)
        {
            iSqrBracketLevel = (iSqrBracketLevel > 0) ? iSqrBracketLevel : 0;
            int const iLineState = ((iSqrBracketLevel & 0xFF) << 8) | ((bMultiLineStrg ? 1 : 0) & 0xFF);
            styler.SetLineState(sc.currentLine, iLineState);

            if (bInInlBracket)
            {
                SetStateParsingError(sc);
            }
        }

        // ------------------------------------------------------------------------

        if (IsNewline(sc.ch))
        {
            continue; // eat
        }

        // ------------------------------------------------------------------------

        // -------------------------
        // state dependent
        // -------------------------
        switch (sc.state)
        {
        case SCE_TOML_DEFAULT:
        {
            if (IsASpaceOrTab(sc.ch))
            {
                // eat
            }
            else if (IsCommentChar(sc.ch))
            {
                sc.SetState(SCE_TOML_COMMENT);
            }
            else if (sc.ch == '[')
            {
                if (iSqrBracketLevel == 0)
                {
                    sc.SetState(SCE_TOML_SECTION);
                    ++iSqrBracketLevel;
                }
                else
                {
                    SetStateParsingError(sc);
                }
            }
            else if (sc.ch == ']')
            {
                --iSqrBracketLevel;
                // eat array of tables
            }
            else if (validKey.Contains(sc.ch))
            {
                sc.SetState(SCE_TOML_KEY);
            }
            else   // not valid - maybe quoted
            {
                if (sc.ch == '"')
                {
                    inDQuotedKey = true;
                    sc.SetState(SCE_TOML_KEY);
                }
                else if (sc.ch == '\'')
                {
                    inSQuotedKey = true;
                    sc.SetState(SCE_TOML_KEY);
                }
                else
                {
                    SetStateParsingError(sc);
                }
            }
        }
        break;


        case SCE_TOML_COMMENT:
        {
            // eat - rest of line is comment
        }
        break;


        case SCE_TOML_SECTION:
        {
            if (sc.ch == '"')
            {
                if (!inSQuotedKey)
                {
                    inDQuotedKey = !inDQuotedKey;
                }
            }
            else if (sc.ch == '\'')
            {
                if (!inDQuotedKey)
                {
                    inSQuotedKey = !inSQuotedKey;
                }
            }
            else if (!(inDQuotedKey || inSQuotedKey))
            {
                if (sc.ch == '[')
                {
                    ++iSqrBracketLevel;
                    sc.SetState(SCE_TOML_VALUE);
                    // Array of Tables - eat
                }
                else if (sc.ch == ']')
                {
                    --iSqrBracketLevel;
                    sc.SetState(SCE_TOML_SECTION);
                }
                else if (IsCommentChar(sc.ch))
                {
                    if (iSqrBracketLevel != 0)
                    {
                        SetStateParsingError(sc);
                    }
                    else
                    {
                        sc.SetState(SCE_TOML_COMMENT);
                    }
                }
                else if (IsASpaceOrTab(sc.ch))
                {
                    // eat
                }
                else
                {
                    if (!validKey.Contains(sc.ch))
                    {
                        SetStateParsingError(sc);
                    }
                }
            }
        }
        break;

        case SCE_TOML_KEY:
        {
            if (sc.atLineEnd)
            {
                SetStateParsingError(sc);
                break;
            }
            else if ((sc.ch == '"') && inDQuotedKey)
            {
                sc.ForwardSetState(SCE_TOML_ASSIGNMENT); // end of key
                inDQuotedKey = false;
                break;
            }
            else if ((sc.ch == '\'') && inSQuotedKey)
            {
                sc.ForwardSetState(SCE_TOML_ASSIGNMENT); // end of key
                inSQuotedKey = false;
                break;
            }
            else if (IsASpaceOrTab(sc.ch))
            {
                if (!(inSQuotedKey || inDQuotedKey))
                {
                    sc.SetState(SCE_TOML_ASSIGNMENT); // end of key
                }
                break; // else eat
            }
            else if (IsAssignChar(sc.ch))
            {
                if ((inSQuotedKey || inDQuotedKey))
                {
                    break;  // eat
                }
                sc.SetState(SCE_TOML_ASSIGNMENT); // end of key
                // === fall through ===  case SCE_TOML_ASSIGNMENT:
            }
            else if (validKey.Contains(sc.ch))
            {
                break;  // eat
            }
            else
            {
                if (!(inSQuotedKey || inDQuotedKey))
                {
                    SetStateParsingError(sc);
                }
                break; // no fall through
            }
        }
        // === fall through ===

        case SCE_TOML_ASSIGNMENT:
        {
            if (sc.atLineEnd || (inSQuotedKey || inDQuotedKey))
            {
                SetStateParsingError(sc);
                break;
            }
            else if (IsAssignChar(sc.ch))
            {
                if (IsLookAheadLineEmpty(sc))
                {
                    ForwardSetStateParsingError(sc);
                    break;
                }
                else
                {
                    sc.ForwardSetState(SCE_TOML_VALUE);
                    // === fall through ===  case SCE_TOML_VALUE:
                }
            }
            else if (IsASpace(sc.ch))
            {
                break; // OK
            }
            else
            {
                SetStateParsingError(sc);
                break;
            }
        }
        // === fall through ===

        case SCE_TOML_VALUE:
        {
            if (IsCommentChar(sc.ch))
            {
                sc.SetState(SCE_TOML_COMMENT);
                break;
            }
            if (bPossibleKeyword && IsLookAheadInList(sc, validKeyWord, keywords))
            {
                sc.SetState(SCE_TOML_KEYWORD);
                break;
            }
            else
            {
                bPossibleKeyword = false;
            }
            if (sc.ch == '[')
            {
                ++iSqrBracketLevel;
            }
            else if (sc.ch == ']')
            {
                --iSqrBracketLevel;
                if (iSqrBracketLevel < 0)
                {
                    SetStateParsingError(sc);
                }
            }
            else if (sc.ch == '}')
            {
                //auto p = static_cast<Sci_Position>(sc.currentPos);
                if (!_isQuoted(inSQuotedKey, inDQuotedKey))
                {
                    if (bInInlBracket)
                    {
                        bInInlBracket = false;
                    }
                    else
                    {
                        SetStateParsingError(sc);
                    }
                }
            }
            else if (sc.ch == '{')
            {
                //auto p = static_cast<Sci_Position>(sc.currentPos);
                if (!_isQuoted(inSQuotedKey, inDQuotedKey))
                {
                    if (bInInlBracket)
                    {
                        SetStateParsingError(sc);
                    }
                    else
                    {
                        bInInlBracket = true;
                        sc.SetState(SCE_TOML_VALUE);
                    }
                }
            }
            else if (IsNumber(sc))
            {
                if (IsDateTimeStr(sc, chDateTime, validNumberEnd))
                {
                    sc.SetState(SCE_TOML_DATETIME);
                }
                else
                {
                    sc.SetState(SCE_TOML_NUMBER);
                    if ((sc.ch == '+') || (sc.ch == '-'))
                    {
                        sc.Forward();
                    }
                    inHex = IsNumHex(sc);
                    inBin = IsNumBinary(sc);
                    inOct = IsNumOctal(sc);
                    if (inHex || inBin || inOct)
                    {
                        sc.Forward(2);
                    }
                    if (IsNumExponent(sc))
                    {
                        sc.Forward(2);
                        if ((sc.ch == '+') || (sc.ch == '-'))
                        {
                            sc.Forward();
                        }
                    }
                }
            }
            else if ((sc.ch == '"') && (sc.chPrev != '\\'))
            {
                sc.SetState(SCE_TOML_STR_BASIC);
                if (sc.Match(R"(""")"))
                {
                    bMultiLineStrg = true;
                    sc.Forward(2);
                }
            }
            else if ((sc.ch == '\'') && (sc.chPrev != '\\'))
            {
                sc.SetState(SCE_TOML_STR_LITERAL);
                if (sc.Match(R"(''')"))
                {
                    bMultiLineStrg = true;
                    sc.Forward(2);
                }
            }
        }
        break;


    case SCE_TOML_KEYWORD:
        {
            if (!validKeyWord.Contains(sc.ch))
            {
                if (IsCommentChar(sc.ch))
                {
                    sc.SetState(SCE_TOML_COMMENT);
                    break;
                }
                sc.SetState(SCE_TOML_VALUE);
            }
        }
        break;


    case SCE_TOML_NUMBER:
        {
            if (sc.ch == '_')
            {
                // eat // TODO: only once
            }
            else if (inHex || inBin || inOct)
            {
                if (validNumberEnd.Contains(sc.ch))
                {
                    sc.SetState(SCE_TOML_VALUE);
                    inHex = false;
                    inBin = false;
                    inOct = false;
                    if (IsCommentChar(sc.ch))
                    {
                        sc.SetState(SCE_TOML_COMMENT);
                        break;
                    }
                    else if (sc.ch == ']')
                    {
                        --iSqrBracketLevel;
                        if (iSqrBracketLevel < 0)
                        {
                            SetStateParsingError(sc);
                        }
                    }
                }
                else
                {
                    if ((inHex && !IsADigit(sc.ch, 16)) ||
                            (inBin && !IsADigit(sc.ch, 2)) ||
                            (inOct && !IsADigit(sc.ch, 8)))
                    {
                        SetStateParsingError(sc);
                    }
                }
            }
            else if (IsNumExponent(sc))
            {
                sc.Forward();
                if ((sc.chNext == '+') || (sc.chNext == '-'))
                {
                    sc.Forward();
                }
            }
            else if (sc.ch == '.')
            {
                // eat // TODO: only once
            }
            else if (IsADigit(sc.ch))
            {
                // eat
            }
            else if (chDateTime.Contains(sc.ch))
            {
                sc.SetState(SCE_TOML_DATETIME);
            }
            else
            {
                if (validNumberEnd.Contains(sc.ch))
                {
                    sc.SetState(SCE_TOML_VALUE);
                    inHex = false;
                    inBin = false;
                    inOct = false;
                    if (IsCommentChar(sc.ch))
                    {
                        sc.SetState(SCE_TOML_COMMENT);
                        break;
                    }
                    else if (sc.ch == ']')
                    {
                        --iSqrBracketLevel;
                        if (iSqrBracketLevel < 0)
                        {
                            SetStateParsingError(sc);
                        }
                    }
                }
                else
                {
                    SetStateParsingError(sc);
                }
            }
        }
        break;


    case SCE_TOML_DATETIME:
        {
            if (!IsADigit(sc.ch) && !chDateTime.Contains(sc.ch) && (sc.ch != '.'))
            {
                if (validNumberEnd.Contains(sc.ch))
                {
                    sc.SetState(SCE_TOML_VALUE);
                    if (IsCommentChar(sc.ch))
                    {
                        sc.SetState(SCE_TOML_COMMENT);
                        break;
                    }
                    else if (sc.ch == ']')
                    {
                        --iSqrBracketLevel;
                        if (iSqrBracketLevel < 0)
                        {
                            SetStateParsingError(sc);
                        }
                    }
                }
                else
                {
                    SetStateParsingError(sc);
                }
            }
        }
        break;


    case SCE_TOML_STR_BASIC:
        {
            if (sc.ch == '"')
            {
                if (sc.chPrev != '\\')
                {
                    if (!bMultiLineStrg)
                    {
                        sc.SetState(SCE_TOML_VALUE);
                    }
                    else   // inMultiLnString
                    {
                        if (sc.Match(R"(""")"))
                        {
                            bMultiLineStrg = false;
                            sc.Forward(2);
                            sc.SetState(SCE_TOML_VALUE);
                        }
                    }
                }
                }
            }
            break;

        case SCE_TOML_STR_LITERAL:
            {
                if (sc.ch == '\'')
                {
                    if (!bMultiLineStrg)
                    {
                        sc.SetState(SCE_TOML_VALUE);
                    }
                    else   // inMultiLnString
                    {
                        if (sc.Match(R"(''')"))
                        {
                            bMultiLineStrg = false;
                            sc.Forward(2);
                            sc.SetState(SCE_TOML_VALUE);
                        }
                    }
                }
            }
            break;


        case SCE_TOML_PARSINGERROR:
            {
                // keep parsing error until new line
            }
            break;


        default:
            {
                //~SetStateParsingError(sc); // unknown
            }
            break;
        }

    }
    sc.Complete();
}
// ----------------------------------------------------------------------------



void SCI_METHOD LexerTOML::Fold(Sci_PositionU startPos, Sci_Position length, int, IDocument* pAccess)
{
    if (!options.fold) { return; }

    PropSetSimple props;
    Accessor styler(pAccess, &props);

    //const Sci_Position docLines = styler.GetLine(styler.Length());
    //const Sci_Position maxPos = startPos + length;
    //const Sci_Position maxLines = styler.GetLine(maxPos == styler.Length() ? maxPos : maxPos - 1);

    const Sci_PositionU endPos = startPos + length;
    int visibleChars = 0;
    Sci_Position lineCurrent = styler.GetLine(startPos);

    char chNext = styler[startPos];
    int styleNext = styler.StyleAt(startPos);
    bool headerPoint = false;
    int lev;

    for (Sci_PositionU i = startPos; i < endPos; i++)
    {
        const char ch = chNext;
        chNext = styler[i + 1];

        const int style = styleNext;
        styleNext = styler.StyleAt(i + 1);
        const bool atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');

        if (style == SCE_TOML_SECTION)
        {
            headerPoint = true;
        }

        if (atEOL)
        {
            lev = SC_FOLDLEVELBASE;

            if (lineCurrent > 0)
            {
                const int levelPrevious = styler.LevelAt(lineCurrent - 1);

                if (levelPrevious & SC_FOLDLEVELHEADERFLAG)
                {
                    lev = SC_FOLDLEVELBASE + 1;
                }
                else
                {
                    lev = levelPrevious & SC_FOLDLEVELNUMBERMASK;
                }
            }

            if (headerPoint)
            {
                lev = SC_FOLDLEVELBASE;
            }
            if (visibleChars == 0 && options.foldCompact)
                lev |= SC_FOLDLEVELWHITEFLAG;

            if (headerPoint)
            {
                lev |= SC_FOLDLEVELHEADERFLAG;
            }
            if (lev != styler.LevelAt(lineCurrent))
            {
                styler.SetLevel(lineCurrent, lev);
            }

            lineCurrent++;
            visibleChars = 0;
            headerPoint = false;
        }
        if (!isspacechar(ch))
            visibleChars++;
    }

    if (lineCurrent > 0)
    {
        const int levelPrevious = styler.LevelAt(lineCurrent - 1);
        if (levelPrevious & SC_FOLDLEVELHEADERFLAG)
        {
            lev = SC_FOLDLEVELBASE + 1;
        }
        else
        {
            lev = levelPrevious & SC_FOLDLEVELNUMBERMASK;
        }
    }
    else
    {
        lev = SC_FOLDLEVELBASE;
    }
    int flagsNext = styler.LevelAt(lineCurrent);
    styler.SetLevel(lineCurrent, lev | (flagsNext & ~SC_FOLDLEVELNUMBERMASK));

}
// ----------------------------------------------------------------------------

LexerModule lmTOML(SCLEX_TOML, LexerTOML::LexerFactoryTOML, "TOML", tomlWordListsDesc);

// ----------------------------------------------------------------------------

