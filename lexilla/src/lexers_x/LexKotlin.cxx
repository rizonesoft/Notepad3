// encoding: UTF-8
// Scintilla source code edit control
/** @file LexKotlin.cxx
** Lexer for Kotlin Language Source Code
** Original Code by Zufuliu, Adapted by RaiKoHoff
** Kotlin Ref: https://kotlinlang.org/docs/reference/
**/

#include <string>
#include <assert.h>
#include <map>
#include <vector>
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
#include "LexerUtils.h"


using namespace Scintilla;

namespace
{
// Use an unnamed namespace to protect the functions and classes from name conflicts


struct OptionsKotlin
{
    bool fold;
    bool foldComment;
    //bool foldCompact;
    OptionsKotlin()
        : fold(false)
        , foldComment(true)
        //, foldCompact(true)
    { }
};

static const char* const kotlinWordLists[] =
{
    "Keywords",
    "Classes",
    "Interfaces",
    "Enums",
    "Annotations",
    "Functions",
    "KDocs",
    nullptr
};

struct OptionSetKotlin : public OptionSet<OptionsKotlin>
{
    OptionSetKotlin()
    {
        DefineProperty("fold", &OptionsKotlin::fold);
        DefineProperty("fold.comment", &OptionsKotlin::foldComment);
        //DefineProperty("fold.compact", &OptionsDart::foldCompact);

        DefineWordListSets(kotlinWordLists);
    }
};

LexicalClass lexicalClasses[] =
{ 
    // Lexer Dart  SCLEX_DART  SCE_DART_:
    0,  "SCE_KOTLIN_DEFAULT",            "default",              "Descr",
    1,  "SCE_KOTLIN_COMMENTLINE",        "commentline",          "Descr",
    2,  "SCE_KOTLIN_COMMENTLINEDOC",     "commentlinedoc",       "Descr",
    3,  "SCE_KOTLIN_COMMENTBLOCK",       "commentblock",         "Descr",
    4,  "SCE_KOTLIN_COMMENTBLOCKDOC",    "commentblockdoc",      "Descr",
    5,  "SCE_KOTLIN_COMMENTDOCWORD",     "commentdocword",       "Descr",
    6,  "SCE_KOTLIN_STRING",             "srting",               "Descr",
    7,  "SCE_KOTLIN_CHARACTER",          "character",            "Descr",
    8,  "SCE_KOTLIN_ESCAPECHAR",         "escapechar",           "Descr",
    9,  "SCE_KOTLIN_RAWSTRING ",         "rawstring",            "Descr",
    10, "SCE_KOTLIN_RAWSTRINGSTART",     "rawstring_start",      "Descr",
    11, "SCE_KOTLIN_RAWSTRINGEND",       "rawstring_end",        "Descr",
    12, "SCE_KOTLIN_BACKTICKS",          "backticks",            "Descr",
    13, "SCE_KOTLIN_NUMBER",             "number",               "Descr",
    14, "SCE_KOTLIN_OPERATOR",           "operator",             "Descr",
    15, "SCE_KOTLIN_OPERATOR2",          "operator2",            "Descr",
    16, "SCE_KOTLIN_VARIABLE",           "variable",             "Descr",
    17, "SCE_KOTLIN_ANNOTATION",         "annotation",           "Descr",
    18, "SCE_KOTLIN_LABEL",              "label",                "Descr",
    19, "SCE_KOTLIN_IDENTIFIER",         "identifier",           "Descr",
    20, "SCE_KOTLIN_WORD",               "word",                 "Descr",
    21, "SCE_KOTLIN_CLASS",              "class",                "Descr",
    22, "SCE_KOTLIN_INTERFACE",          "interface",            "Descr",
    23, "SCE_KOTLIN_ENUM",               "enum",                 "Descr",
    24, "SCE_KOTLIN_FUNCTION",           "function",             "Descr"
};

} // end of namespace

class LexerKotlin : public DefaultLexer
{

    //CharacterSet validKey;
    //CharacterSet validKeyWord;
    //CharacterSet validNumberEnd;
    //CharacterSet chDateTime;

    WordList wl_keywords;
    WordList wl_classes;
    WordList wl_interfaces;
    WordList wl_enums;
    WordList wl_annotations;
    WordList wl_functions;
    WordList wl_kdocs;

    OptionsKotlin options;
    OptionSetKotlin osKotlin;

public:
    LexerKotlin()
        : DefaultLexer("Kotlin", SCLEX_KOTLIN, lexicalClasses, ELEMENTS(lexicalClasses))
        //, validKey(CharacterSet::setAlphaNum, R"(-_.)", 0x80, false)
        //, validKeyWord(CharacterSet::setAlphaNum, "_+-", 0x80, false)
        //, validNumberEnd(CharacterSet::setNone, " \t\n\v\f\r#,)}]", 0x80, false)
        //, chDateTime(CharacterSet::setNone, "-+.:TZ", 0x80, false)
    { }

    virtual ~LexerKotlin() { }

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
        return osKotlin.PropertyNames();
    }

    int SCI_METHOD PropertyType(const char* name) override
    {
        return osKotlin.PropertyType(name);
    }

    const char* SCI_METHOD PropertyGet(const char* name) override
    {
        return osKotlin.PropertyGet(name);
    }

    const char* SCI_METHOD DescribeProperty(const char* name) override
    {
        return osKotlin.DescribeProperty(name);
    }

    const char* SCI_METHOD DescribeWordListSets() override
    {
        return osKotlin.DescribeWordListSets();
    }

    void* SCI_METHOD PrivateCall(int, void*) override
    {
        return nullptr;
    }

    int SCI_METHOD LineEndTypesSupported() override
    {
        return SC_LINE_END_TYPE_DEFAULT;
    }

    int SCI_METHOD PrimaryStyleFromStyle(int style) override
    {
        return style;
    }

    static ILexer5* LexerFactoryKotlin()
    {
        return new LexerKotlin();
    }

    // --------------------------------------------------------------------------

    Sci_Position SCI_METHOD PropertySet(const char* key, const char* val) override;
    Sci_Position SCI_METHOD WordListSet(int n, const char* wl) override;
    void SCI_METHOD Lex(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument* pAccess) override;
    void SCI_METHOD Fold(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument* pAccess) override;

};


Sci_Position SCI_METHOD LexerKotlin::PropertySet(const char* key, const char* val)
{
    if (osKotlin.PropertySet(&options, key, val))
    {
        return 0;
    }
    return -1;
}


Sci_Position SCI_METHOD LexerKotlin::WordListSet(int n, const char* wl)
{
    WordList* wordListN = nullptr;

    switch (n)
    {
    case 0:
        wordListN = &wl_keywords;
        break;
    case 1:
        wordListN = &wl_classes;
        break;
    case 2:
        wordListN = &wl_interfaces;
        break;
    case 3:
        wordListN = &wl_enums;
        break;
    case 4:
        wordListN = &wl_annotations;
        break;
    case 5:
        wordListN = &wl_functions;
        break;
    case 6:
        wordListN = &wl_kdocs;
        break;
    }

    Sci_Position firstModification = -1LL;

    if (wordListN)
    {
        WordList wlNew;
        wlNew.Set(wl);
        if (*wordListN != wlNew)
        {
            wordListN->Set(wl);
            firstModification = 0;
        }
    }
    return firstModification;
}
// ----------------------------------------------------------------------------

enum
{
    MaxKotlinNestedStateCount = 4,
    KotlinLineStateMaskLineComment = (1 << 14), // line comment
    KotlinLineStateMaskImport = (1 << 15), // import
};

struct EscapeSequence {

    int outerState = SCE_KOTLIN_DEFAULT;
    int digitsLeft = 0;

    // highlight any character as escape sequence, no highlight for hex in '\u{hex}'.
    bool resetEscapeState(int state, int chNext) noexcept {
        if (IsLineBreak(chNext)) { return false; }
        outerState = state;
        digitsLeft = (chNext == 'u') ? 5 : 1;
        return true;
    }

    bool atEscapeEnd(int ch) noexcept {
        if (digitsLeft <= 0) {
            return true;
        }
        return --digitsLeft <= 0 || !IsAHexDigit(ch);
    }
};


constexpr int PackState(int state) noexcept {
    switch (state)
    {
    case SCE_KOTLIN_STRING:
        return 1;
    case SCE_KOTLIN_RAWSTRING:
        return 2;
    default:
        return 0;
    }
}

constexpr int UnpackState(int state) noexcept {
    switch (state)
    {
    case 1:
        return SCE_KOTLIN_STRING;
    case 2:
        return SCE_KOTLIN_RAWSTRING;
    default:
        return SCE_KOTLIN_DEFAULT;
    }
}

inline static int PackNestedState(const std::vector<int>& nestedState) noexcept {
    return PackLineState<2, MaxKotlinNestedStateCount, PackState>(nestedState) << 16;
}

inline static void UnpackNestedState(int lineState, int count, std::vector<int>& nestedState) {
    UnpackLineState<2, MaxKotlinNestedStateCount, UnpackState>(lineState, count, nestedState);
}


void SCI_METHOD LexerKotlin::Lex(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument* pAccess)
{
    PropSetSimple props;
    props.SetMultiple(osKotlin.PropertyNames());

    Accessor styler(pAccess, &props);
    StyleContext sc(startPos, length, initStyle, styler);

    int lineStateLineComment = 0;
    int lineStateImport = 0;
    int commentLevel = 0;	// nested block comment level

    int kwType = SCE_KOTLIN_DEFAULT;
    int chBeforeIdentifier = 0;

    int curlyBrace = 0; // "${}"
    int variableOuter = SCE_KOTLIN_DEFAULT;	// variable inside string
    std::vector<int> nestedState;

    int visibleChars = 0;
    EscapeSequence escSeq = { SCE_KOTLIN_DEFAULT, 0 };

    if (sc.currentLine > 0)
    {
        const int lineState = styler.GetLineState(sc.currentLine - 1);
        /*
        8: curlyBrace
        6: commentLevel
        1: lineStateLineComment
        1: lineStateImport
        8: nestedState
        */
        curlyBrace = lineState & 0xff;
        commentLevel = (lineState >> 8) & 0x3f;
        if (curlyBrace)
        {
            UnpackNestedState((lineState >> 16) & 0xff, curlyBrace, nestedState);
        }
    }
    if (startPos == 0 && sc.Match('#', '!'))
    {
        // Shell Shebang at beginning of file
        sc.SetState(SCE_KOTLIN_COMMENTLINE);
        lineStateLineComment = KotlinLineStateMaskLineComment;
    }

    while (sc.More())
    {
        switch (sc.state)
        {
        case SCE_KOTLIN_OPERATOR:
        case SCE_KOTLIN_OPERATOR2:
            sc.SetState(SCE_KOTLIN_DEFAULT);
            break;

        case SCE_KOTLIN_NUMBER:
            if (!IsDecimalNumber(sc.chPrev, sc.ch, sc.chNext))
            {
                sc.SetState(SCE_KOTLIN_DEFAULT);
            }
            break;

        case SCE_KOTLIN_IDENTIFIER:
            if (!IsIdentifierCharEx(sc.ch))
            {
                char s[128];
                sc.GetCurrent(s, sizeof(s));
                if (wl_keywords.InList(s))
                {
                    sc.ChangeState(SCE_KOTLIN_WORD);
                    if (strcmp(s, "import") == 0)
                    {
                        if (visibleChars == sc.LengthCurrent())
                        {
                            lineStateImport = KotlinLineStateMaskImport;
                        }
                    }
                    else if (strcmp(s, "break") == 0 || strcmp(s, "continue") == 0 || strcmp(s, "return") == 0
                        || strcmp(s, "this") == 0 || strcmp(s, "super") == 0)
                    {
                        kwType = SCE_KOTLIN_LABEL;
                    }
                    else if (((kwType != SCE_KOTLIN_ANNOTATION && kwType != SCE_KOTLIN_ENUM)
                        && (chBeforeIdentifier != ':' && strcmp(s, "class") == 0))
                        || strcmp(s, "typealias") == 0)
                    {
                        kwType = SCE_KOTLIN_CLASS;
                    }
                    else if (strcmp(s, "enum") == 0)
                    {
                        kwType = SCE_KOTLIN_ENUM;
                    }
                    else if (strcmp(s, "annotation") == 0)
                    {
                        kwType = SCE_KOTLIN_ANNOTATION;
                    }
                    else if (strcmp(s, "interface") == 0)
                    {
                        kwType = SCE_KOTLIN_INTERFACE;
                    }
                    if (kwType != SCE_KOTLIN_DEFAULT)
                    {
                        //?const int chNext = sc.GetNextNSChar();
                        while (IsASpace(sc.ch)) { sc.Forward(); };
                        if (!((kwType == SCE_KOTLIN_LABEL && sc.ch == '@') || (kwType != SCE_KOTLIN_LABEL && IsIdentifierStart(sc.ch))))
                        {
                            kwType = SCE_KOTLIN_DEFAULT;
                        }
                    }
                }
                else if (sc.ch == '@')
                {
                    sc.ChangeState(SCE_KOTLIN_LABEL);
                    sc.Forward();
                }
                else if (wl_classes.InList(s))
                {
                    sc.ChangeState(SCE_KOTLIN_CLASS);
                }
                else if (wl_interfaces.InList(s))
                {
                    sc.ChangeState(SCE_KOTLIN_INTERFACE);
                }
                else if (wl_enums.InList(s))
                {
                    sc.ChangeState(SCE_KOTLIN_ENUM);
                }
                else
                {
                    if (kwType != SCE_KOTLIN_DEFAULT && kwType != SCE_KOTLIN_LABEL)
                    {
                        sc.ChangeState(kwType);
                    }
                    else
                    {
                        while (IsASpace(sc.ch)) { sc.Forward(); };
                        if (sc.ch == '(') {
                            sc.ChangeState(SCE_KOTLIN_FUNCTION);
                        }
                    }
                }
                if (sc.state != SCE_KOTLIN_WORD)
                {
                    kwType = SCE_KOTLIN_DEFAULT;
                }
                sc.SetState(SCE_KOTLIN_DEFAULT);
            }
            break;

        case SCE_KOTLIN_LABEL:
            if (!IsIdentifierCharEx(sc.ch))
            {
                sc.SetState(SCE_KOTLIN_DEFAULT);
            }
            break;
        case SCE_KOTLIN_ANNOTATION:
            if (!IsWordCharEx(sc.ch))
            {
                // highlight whole package.Name as annotation
                sc.SetState(SCE_KOTLIN_DEFAULT);
            }
            break;

        case SCE_KOTLIN_COMMENTLINE:
        case SCE_KOTLIN_COMMENTLINEDOC:
            if (sc.atLineStart)
            {
                sc.SetState(SCE_KOTLIN_DEFAULT);
            }
            break;
        case SCE_KOTLIN_COMMENTBLOCK:
            if (sc.Match('*', '/'))
            {
                sc.Forward();
                --commentLevel;
                if (commentLevel == 0)
                {
                    sc.ForwardSetState(SCE_KOTLIN_DEFAULT);
                }
            }
            else if (sc.Match('/', '*'))
            {
                sc.Forward(2);
                ++commentLevel;
            }
            break;
        case SCE_KOTLIN_COMMENTBLOCKDOC:
            if (sc.ch == '@' && IsLowerCase(sc.chNext) && isspacechar(sc.chPrev))
            {
                sc.SetState(SCE_KOTLIN_COMMENTDOCWORD);
            }
            else if (sc.Match('*', '/'))
            {
                sc.Forward();
                --commentLevel;
                if (commentLevel == 0)
                {
                    sc.ForwardSetState(SCE_KOTLIN_DEFAULT);
                }
            }
            else if (sc.Match('/', '*'))
            {
                sc.Forward(2);
                ++commentLevel;
            }
            break;
        case SCE_KOTLIN_COMMENTDOCWORD:
            if (!IsLowerCase(sc.ch))
            {
                sc.SetState(SCE_KOTLIN_COMMENTBLOCKDOC);
                continue;
            }
            break;

        case SCE_KOTLIN_STRING:
        case SCE_KOTLIN_RAWSTRING:
            if (sc.state == SCE_KOTLIN_STRING && sc.atLineStart)
            {
                sc.SetState(SCE_KOTLIN_DEFAULT);
            }
            else if (sc.state == SCE_KOTLIN_STRING && sc.ch == '\\')
            {
                if (escSeq.resetEscapeState(sc.state, sc.chNext))
                {
                    sc.SetState(SCE_KOTLIN_ESCAPECHAR);
                    sc.Forward();
                }
            }
            else if (sc.ch == '$' && IsIdentifierStartEx(sc.chNext))
            {
                variableOuter = sc.state;
                sc.SetState(SCE_KOTLIN_VARIABLE);
            }
            else if (sc.Match('$', '{'))
            {
                ++curlyBrace;
                nestedState.push_back(sc.state);
                sc.SetState(SCE_KOTLIN_OPERATOR2);
                sc.Forward();
            }
            else if (curlyBrace && sc.ch == '}')
            {
                const int outerState = nestedState.empty() ? SCE_KOTLIN_DEFAULT : nestedState.back();
                if (!nestedState.empty())
                {
                    nestedState.pop_back();
                }
                --curlyBrace;
                sc.SetState(SCE_KOTLIN_OPERATOR2);
                sc.ForwardSetState(outerState);
                continue;
            }
            else if (sc.ch == '\"' && (sc.state == SCE_KOTLIN_STRING || sc.Match(R"(""")")))
            {
                if (sc.state == SCE_KOTLIN_RAWSTRING)
                {
                    sc.Forward(2);
                    sc.SetState(SCE_KOTLIN_RAWSTRINGEND);
                }
                sc.ForwardSetState(SCE_KOTLIN_DEFAULT);
            }
            break;

        case SCE_KOTLIN_CHARACTER:
            if (sc.atLineStart)
            {
                sc.SetState(SCE_KOTLIN_DEFAULT);
            }
            else if (sc.ch == '\\')
            {
                if (escSeq.resetEscapeState(sc.state, sc.chNext))
                {
                    sc.SetState(SCE_KOTLIN_ESCAPECHAR);
                    sc.Forward();
                }
            }
            else if (sc.ch == '\'')
            {
                sc.ForwardSetState(SCE_KOTLIN_DEFAULT);
            }
            break;
        case SCE_KOTLIN_ESCAPECHAR:
            if (escSeq.atEscapeEnd(sc.ch))
            {
                const int outerState = escSeq.outerState;
                if (outerState == SCE_KOTLIN_STRING)
                {
                    if (sc.ch == '\\' && escSeq.resetEscapeState(outerState, sc.chNext))
                    {
                        sc.Forward();
                    }
                    else
                    {
                        sc.SetState(outerState);
                        if (sc.ch == '\"')
                        {
                            sc.ForwardSetState(SCE_KOTLIN_DEFAULT);
                        }
                    }
                }
                else
                {
                    sc.SetState(outerState);
                    if (sc.ch == '\'')
                    {
                        sc.ForwardSetState(SCE_KOTLIN_DEFAULT);
                    }
                }
            }
            break;
        case SCE_KOTLIN_VARIABLE:
            if (!IsIdentifierCharEx(sc.ch))
            {
                sc.SetState(variableOuter);
                continue;
            }
            break;

        case SCE_KOTLIN_BACKTICKS:
            if (sc.atLineStart)
            {
                sc.SetState(SCE_KOTLIN_DEFAULT);
            }
            else if (sc.ch == '`')
            {
                sc.ForwardSetState(SCE_KOTLIN_DEFAULT);
            }
            break;
        }

        if (sc.state == SCE_KOTLIN_DEFAULT)
        {
            if (sc.Match('/', '/'))
            {
                const int chNext = sc.GetRelative(2);
                sc.SetState((chNext == '!' || chNext == '/') ? SCE_KOTLIN_COMMENTLINEDOC : SCE_KOTLIN_COMMENTLINE);
                if (visibleChars == 0)
                {
                    lineStateLineComment = KotlinLineStateMaskLineComment;
                }
            }
            else if (sc.Match('/', '*'))
            {
                const int chNext = sc.GetRelative(2);
                sc.SetState((chNext == '*' || chNext == '!') ? SCE_KOTLIN_COMMENTBLOCKDOC : SCE_KOTLIN_COMMENTBLOCK);
                sc.Forward();
                commentLevel = 1;
            }
            else if (sc.Match(R"(""")"))
            {
                sc.SetState(SCE_KOTLIN_RAWSTRINGSTART);
                sc.ForwardSetState(SCE_KOTLIN_RAWSTRING);
                sc.Forward();
            }
            else if (sc.ch == '\"')
            {
                sc.SetState(SCE_KOTLIN_STRING);
            }
            else if (sc.ch == '\'')
            {
                sc.SetState(SCE_KOTLIN_CHARACTER);
            }
            else if (IsADigit(sc.ch))
            {
                sc.SetState(SCE_KOTLIN_NUMBER);
            }
            else if (sc.ch == '@' && IsIdentifierStartEx(sc.chNext))
            {
                sc.SetState((kwType == SCE_KOTLIN_LABEL) ? SCE_KOTLIN_LABEL : SCE_KOTLIN_ANNOTATION);
                kwType = SCE_KOTLIN_DEFAULT;
            }
            else if (sc.ch == '`')
            {
                sc.SetState(SCE_KOTLIN_BACKTICKS);
            }
            else if (IsIdentifierStartEx(sc.ch))
            {
                chBeforeIdentifier = sc.chPrev;
                sc.SetState(SCE_KOTLIN_IDENTIFIER);
            }
            else if (isoperator(sc.ch))
            {
                sc.SetState(curlyBrace ? SCE_KOTLIN_OPERATOR2 : SCE_KOTLIN_OPERATOR);
                if (curlyBrace)
                {
                    if (sc.ch == '{')
                    {
                        ++curlyBrace;
                        nestedState.push_back(SCE_KOTLIN_DEFAULT);
                    }
                    else if (sc.ch == '}')
                    {
                        --curlyBrace;
                        const int outerState = nestedState.empty() ? SCE_KOTLIN_DEFAULT : nestedState.back();
                        if (!nestedState.empty())
                        {
                            nestedState.pop_back();
                        }
                        sc.ForwardSetState(outerState);
                        continue;
                    }
                }
            }
        }

        if (!isspacechar(sc.ch))
        {
            visibleChars++;
        }
        if (sc.atLineEnd)
        {
            int lineState = curlyBrace | (commentLevel << 8) | lineStateLineComment | lineStateImport;
            if (curlyBrace)
            {
                lineState |= PackNestedState(nestedState);
            }
            styler.SetLineState(sc.currentLine, lineState);
            lineStateLineComment = 0;
            lineStateImport = 0;
            visibleChars = 0;
            kwType = SCE_KOTLIN_DEFAULT;
        }
        sc.Forward();
    }

    sc.Complete();
}
// ----------------------------------------------------------------------------




// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

struct FoldLineState {
    int lineComment;
    int packageImport;
    constexpr explicit FoldLineState(int lineState) noexcept :
        lineComment(lineState& KotlinLineStateMaskLineComment),
        packageImport(lineState& KotlinLineStateMaskImport)
    { }
};

constexpr bool IsStreamCommentStyle(int style) noexcept {
    return style == SCE_KOTLIN_COMMENTBLOCK || style == SCE_KOTLIN_COMMENTBLOCKDOC;
}

void SCI_METHOD LexerKotlin::Fold(Sci_PositionU startPos, Sci_Position length, int, IDocument* pAccess)
{
    if (!options.fold) { return; }

    PropSetSimple props;
    props.SetMultiple(osKotlin.PropertyNames());

    Accessor styler(pAccess, &props);

    //const int foldComment = styler.GetPropertyInt("fold.comment", 1);
    //const int foldComment = props.GetInt("fold.comment", 1) != 0;
    const int foldComment = options.foldComment;

    const Sci_PositionU endPos = startPos + length;
    Sci_Position lineCurrent = styler.GetLine(startPos);
    FoldLineState foldPrev(0);
    int levelCurrent = SC_FOLDLEVELBASE;
    if (lineCurrent > 0)
    {
        levelCurrent = styler.LevelAt(lineCurrent - 1) >> 16;
        foldPrev = FoldLineState(styler.GetLineState(lineCurrent - 1));
    }

    int levelNext = levelCurrent;
    FoldLineState foldCurrent(styler.GetLineState(lineCurrent));
    Sci_PositionU lineStartNext = styler.LineStart(lineCurrent + 1);
    Sci_PositionU lineEndPos = ((lineStartNext < endPos) ? lineStartNext : endPos) - 1;

    char chNext = styler[startPos];
    int styleNext = styler.StyleAt(startPos);

    for (Sci_PositionU i = startPos; i < endPos; i++)
    {
        const char ch = chNext;
        chNext = styler.SafeGetCharAt(i + 1);
        const int style = styleNext;
        styleNext = styler.StyleAt(i + 1);

        if (IsStreamCommentStyle(style))
        {
            if (foldComment)
            {
                const int level = (ch == '/' && chNext == '*') ? 1 : ((ch == '*' && chNext == '/') ? -1 : 0);
                if (level != 0)
                {
                    levelNext += level;
                    i++;
                    chNext = styler.SafeGetCharAt(i + 1);
                    styleNext = styler.StyleAt(i + 1);
                }
            }
        }
        else if (style == SCE_KOTLIN_RAWSTRINGSTART)
        {
            levelNext++;
        }
        else if (style == SCE_KOTLIN_RAWSTRINGEND)
        {
            levelNext--;
        }
        else if (style == SCE_KOTLIN_OPERATOR)
        {
            if (ch == '{' || ch == '[' || ch == '(')
            {
                levelNext++;
            }
            else if (ch == '}' || ch == ']' || ch == ')')
            {
                levelNext--;
            }
        }

        if (i == lineEndPos)
        {
            const FoldLineState foldNext(styler.GetLineState(lineCurrent + 1));
            if (foldComment && foldCurrent.lineComment)
            {
                if (!foldPrev.lineComment && foldNext.lineComment)
                {
                    levelNext++;
                }
                else if (foldPrev.lineComment && !foldNext.lineComment)
                {
                    levelNext--;
                }
            }
            else if (foldCurrent.packageImport)
            {
                if (!foldPrev.packageImport && foldNext.packageImport)
                {
                    levelNext++;
                }
                else if (foldPrev.packageImport && !foldNext.packageImport)
                {
                    levelNext--;
                }
            }

            const int levelUse = levelCurrent;
            int lev = levelUse | levelNext << 16;
            if (levelUse < levelNext)
            {
                lev |= SC_FOLDLEVELHEADERFLAG;
            }
            if (lev != styler.LevelAt(lineCurrent))
            {
                styler.SetLevel(lineCurrent, lev);
            }

            lineCurrent++;
            lineStartNext = styler.LineStart(lineCurrent + 1);
            lineEndPos = ((lineStartNext < endPos) ? lineStartNext : endPos) - 1;
            levelCurrent = levelNext;
            foldPrev = foldCurrent;
            foldCurrent = foldNext;
        }
    }
}
// ----------------------------------------------------------------------------

LexerModule lmKotlin(SCLEX_KOTLIN, LexerKotlin::LexerFactoryKotlin, "Kotlin", kotlinWordLists);

// ----------------------------------------------------------------------------

