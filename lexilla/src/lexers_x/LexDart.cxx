// encoding: UTF-8
// Scintilla source code edit control
/** @file LexDart.cxx
** Lexer for Dart Language Source Code
** Original Code by Zufuliu, Adapted by RaiKoHoff
** Dart Spec:  https://dart.dev/guides/language/spec
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

enum {
    MaxDartNestedStateCount = 4,
    DartLineStateMaskLineComment = 1,	// line comment
    DartLineStateMaskImport = (1 << 1),	// import
};

struct OptionsDart
{
    bool fold;
    bool foldComment;
    //bool foldCompact;
    OptionsDart()
        : fold(false)
        , foldComment(true)
        //, foldCompact(true)
    { }
};

static const char* const dartWordLists[] =
{
    "Keywords",
    "Types",
    "Classes",
    "Enums",
    "MetaData",
    "Functions",
    nullptr
};

struct OptionSetDart : public OptionSet<OptionsDart>
{
    OptionSetDart()
    {
        DefineProperty("fold", &OptionsDart::fold);
        DefineProperty("fold.comment", &OptionsDart::foldComment);
        //DefineProperty("fold.compact", &OptionsDart::foldCompact);

        DefineWordListSets(dartWordLists);
    }
};

LexicalClass lexicalClasses[] =
{
    // Lexer Dart  SCLEX_DART  SCE_DART_:
    0,  "SCE_DART_DEFAULT",                 "default",              "Descr",
    1,  "SCE_DART_COMMENTLINE",             "commentline",          "Descr",
    2,  "SCE_DART_COMMENTLINEDOC",          "commentlinedoc",       "Descr",
    3,  "SCE_DART_COMMENTBLOCK",            "commentblock",         "Descr",
    4,  "SCE_DART_COMMENTBLOCKDOC",         "commentblockdoc",      "Descr",
    5,  "SCE_DART_NUMBER",                  "number",               "Descr",
    6,  "SCE_DART_OPERATOR",                "operator",             "Descr",
    7,  "SCE_DART_OPERATOR2",               "operator2",            "Descr",
    8,  "SCE_DART_IDENTIFIER",              "identifier",           "Descr",
    9,  "SCE_DART_STRING_SQ ",              "string_sq",            "Descr",
    10, "SCE_DART_STRING_DQ",               "string_dq",            "Descr",
    11, "SCE_DART_TRIPLE_STRING_SQ",        "tristrg_sq",           "Descr",
    12, "SCE_DART_TRIPLE_STRING_DQ",        "tristrg_dq",           "Descr",
    13, "SCE_DART_ESCAPECHAR",              "escchar",              "Descr",
    14, "SCE_DART_RAWSTRING_SQ",            "rawstrg_sq",           "Descr",
    15, "SCE_DART_RAWSTRING_DQ",            "rawstrg_dq",           "Descr",
    16, "SCE_DART_TRIPLE_RAWSTRING_SQ",     "trirawstrg_sq",        "Descr",
    17, "SCE_DART_TRIPLE_RAWSTRING_DQ",     "trirawstrg_dq",        "Descr",
    18, "SCE_DART_TRIPLE_STRING_SQSTART",   "default",              "Descr",
    19, "SCE_DART_TRIPLE_STRING_DQSTART",   "default",              "Descr",
    20, "SCE_DART_TRIPLE_STRING_SQEND",     "default",              "Descr",
    21, "SCE_DART_TRIPLE_STRING_DQEND",     "default",              "Descr",
    22, "SCE_DART_SYMBOL_OPERATOR",         "symbol_op",            "Descr",
    23, "SCE_DART_SYMBOL_IDENTIFIER",       "symbol_id",            "Descr",
    24, "SCE_DART_VARIABLE",                "var",                  "Descr",
    25, "SCE_DART_METADATA",                "metadata",             "Descr",
    26, "SCE_DART_LABEL",                   "label",                "Descr",
    27, "SCE_DART_FUNCTION",                "function",             "Descr",
    28, "SCE_DART_WORD",                    "word",                 "Descr",
    29, "SCE_DART_WORD2",                   "word2",                "Descr",
    30, "SCE_DART_CLASS",                   "class",                "Descr",
    31, "SCE_DART_ENUM",                    "enum",                 "Descr"
};

} // end of namespace

class LexerDart : public DefaultLexer
{

    //CharacterSet validKey;
    //CharacterSet validKeyWord;
    //CharacterSet validNumberEnd;
    //CharacterSet chDateTime;

    WordList wl_keywords;
    WordList wl_types;
    WordList wl_classes;
    WordList wl_enums;
    WordList wl_metadata;
    WordList wl_functions;

    OptionsDart options;
    OptionSetDart osDart;

public:
    LexerDart()
        : DefaultLexer("Dart", SCLEX_DART, lexicalClasses, ELEMENTS(lexicalClasses))
        //, validKey(CharacterSet::setAlphaNum, R"(-_.)", 0x80, false)
        //, validKeyWord(CharacterSet::setAlphaNum, "_+-", 0x80, false)
        //, validNumberEnd(CharacterSet::setNone, " \t\n\v\f\r#,)}]", 0x80, false)
        //, chDateTime(CharacterSet::setNone, "-+.:TZ", 0x80, false)
    { }

    virtual ~LexerDart() { }

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
        return osDart.PropertyNames();
    }

    int SCI_METHOD PropertyType(const char* name) override
    {
        return osDart.PropertyType(name);
    }

    const char* SCI_METHOD PropertyGet(const char* name) override
    {
        return osDart.PropertyGet(name);
    }

    const char* SCI_METHOD DescribeProperty(const char* name) override
    {
        return osDart.DescribeProperty(name);
    }

    const char* SCI_METHOD DescribeWordListSets() override
    {
        return osDart.DescribeWordListSets();
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

    static ILexer5* LexerFactoryDart()
    {
        return new LexerDart();
    }

    // --------------------------------------------------------------------------

    Sci_Position SCI_METHOD PropertySet(const char* key, const char* val) override;
    Sci_Position SCI_METHOD WordListSet(int n, const char* wl) override;
    void SCI_METHOD Lex(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument* pAccess) override;
    void SCI_METHOD Fold(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument* pAccess) override;

};


Sci_Position SCI_METHOD LexerDart::PropertySet(const char* key, const char* val)
{
    if (osDart.PropertySet(&options, key, val))
    {
        return 0;
    }
    return -1;
}


Sci_Position SCI_METHOD LexerDart::WordListSet(int n, const char* wl)
{
    WordList* wordListN = nullptr;

    switch (n)
    {
    case 0:
        wordListN = &wl_keywords;
        break;
    case 1:
        wordListN = &wl_types;
        break;
    case 2:
        wordListN = &wl_classes;
        break;
    case 3:
        wordListN = &wl_enums;
        break;
    case 4:
        wordListN = &wl_metadata;
        break;
    case 5:
        wordListN = &wl_functions;
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

struct EscapeSequence {

    int outerState = SCE_DART_DEFAULT;
    int digitsLeft = 0;

    // highlight any character as escape sequence, no highlight for hex in '\u{hex}'.
    bool resetEscapeState(int state, int chNext) noexcept {
        outerState = state;
        digitsLeft = (chNext == 'x') ? 3 : ((chNext == 'u') ? 5 : 1);
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
    switch (state) {
    case SCE_DART_STRING_SQ:
        return 1;
    case SCE_DART_STRING_DQ:
        return 2;
    case SCE_DART_TRIPLE_STRING_SQ:
        return 3;
    case SCE_DART_TRIPLE_STRING_DQ:
        return 4;
    default:
        return 0;
    }
}

constexpr int UnpackState(int state) noexcept {
    switch (state) {
    case 1:
        return SCE_DART_STRING_SQ;
    case 2:
        return SCE_DART_STRING_DQ;
    case 3:
        return SCE_DART_TRIPLE_STRING_SQ;
    case 4:
        return SCE_DART_TRIPLE_STRING_DQ;
    default:
        return SCE_DART_DEFAULT;
    }
}

inline static int PackNestedState(const std::vector<int>& nestedState) noexcept {
    return PackLineState<3, MaxDartNestedStateCount, PackState>(nestedState) << 16;
}

inline static void UnpackNestedState(int lineState, int count, std::vector<int>& nestedState) {
    UnpackLineState<3, MaxDartNestedStateCount, UnpackState>(lineState, count, nestedState);
}


void SCI_METHOD LexerDart::Lex(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument* pAccess)
{
    PropSetSimple props;
    props.SetMultiple(osDart.PropertyNames());

    Accessor styler(pAccess, &props);
    StyleContext sc(startPos, length, initStyle, styler);

    int lineStateLineComment = 0;
    int lineStateImport = 0;
    int commentLevel = 0;	// nested block comment level

    int kwType = SCE_DART_DEFAULT;
    int chBeforeIdentifier = 0;

    int curlyBrace = 0; // "${}"
    int variableOuter = SCE_DART_DEFAULT;	// variable inside string
    std::vector<int> nestedState;

    int visibleChars = 0;
    EscapeSequence escSeq = { SCE_DART_DEFAULT, 0 };

    if (sc.currentLine > 0) {
        const int lineState = styler.GetLineState(sc.currentLine - 1);
        /*
        1: lineStateLineComment
        1: lineStateImport
        6: commentLevel
        8: curlyBrace
        3*4: nestedState
        */
        commentLevel = (lineState >> 2) & 0x3f;
        curlyBrace = (lineState >> 8) & 0xff;
        if (curlyBrace) {
            UnpackNestedState((lineState >> 16) & 0xff, curlyBrace, nestedState);
        }
    }
    if (startPos == 0 && sc.Match('#', '!')) {
        // Shell Shebang at beginning of file
        sc.SetState(SCE_DART_COMMENTLINE);
        lineStateLineComment = DartLineStateMaskLineComment;
    }

    while (sc.More()) {
        switch (sc.state) {
        case SCE_DART_OPERATOR:
        case SCE_DART_OPERATOR2:
            sc.SetState(SCE_DART_DEFAULT);
            break;

        case SCE_DART_NUMBER:
            if (!IsDecimalNumber(sc.chPrev, sc.ch, sc.chNext)) {
                sc.SetState(SCE_DART_DEFAULT);
            }
            break;

        case SCE_DART_IDENTIFIER:
            if (!IsIdentifierCharEx(sc.ch)) {
                char s[128];
                sc.GetCurrent(s, sizeof(s));
                
                if (wl_keywords.InList(s)) {
                    sc.ChangeState(SCE_DART_WORD);
                    if (EqualsAny(s, "import", "part")) {
                        if (visibleChars == sc.LengthCurrent()) {
                            lineStateImport = DartLineStateMaskImport;
                        }
                    }
                    else if (EqualsAny(s, "as", "class", "extends", "implements", "is", "new", "throw")) {
                        kwType = SCE_DART_CLASS;
                    }
                    else if (strcmp(s, "enum") == 0) {
                        kwType = SCE_DART_ENUM;
                    }
                }
                else if (wl_types.InList(s)) {
                    sc.ChangeState(SCE_DART_WORD2);
                }
                else if (wl_classes.InList(s)) {
                    sc.ChangeState(SCE_DART_CLASS);
                }
                else if (wl_enums.InList(s)) {
                    sc.ChangeState(SCE_DART_ENUM);
                }
                else if (kwType != SCE_DART_DEFAULT) {
                    sc.ChangeState(kwType);
                }
                else {
                    //?const int chNext = GetNextNSChar();
                    while (IsASpace(sc.ch)){ sc.Forward(); };
                    if (sc.ch == '(') {
                        sc.ChangeState(SCE_DART_FUNCTION);
                    }
                    else if ((chBeforeIdentifier == '<' && sc.ch == '>')
                        || IsIdentifierStartEx(sc.ch)) {
                        // type<type>
                        // type identifier
                        sc.ChangeState(SCE_DART_CLASS);
                    }
                }
                if (sc.state != SCE_DART_WORD) {
                    kwType = SCE_DART_DEFAULT;
                }
                sc.SetState(SCE_DART_DEFAULT);
            }
            break;

        case SCE_DART_METADATA:
        case SCE_DART_SYMBOL_IDENTIFIER:
            if (sc.ch == '.') {
                const int state = sc.state;
                sc.SetState(SCE_DART_OPERATOR);
                sc.ForwardSetState(state);
                continue;
            }
            if (!IsIdentifierCharEx(sc.ch)) {
                sc.SetState(SCE_DART_DEFAULT);
            }
            break;

        case SCE_DART_SYMBOL_OPERATOR:
            if (!isoperator(sc.ch)) {
                sc.SetState(SCE_DART_DEFAULT);
            }
            break;

        case SCE_DART_COMMENTLINE:
        case SCE_DART_COMMENTLINEDOC:
            if (sc.atLineStart) {
                sc.SetState(SCE_DART_DEFAULT);
            }
            break;

        case SCE_DART_COMMENTBLOCK:
        case SCE_DART_COMMENTBLOCKDOC:
            if (sc.Match('*', '/')) {
                sc.Forward();
                --commentLevel;
                if (commentLevel == 0) {
                    sc.ForwardSetState(SCE_DART_DEFAULT);
                }
            }
            else if (sc.Match('/', '*')) {
                sc.Forward(2);
                ++commentLevel;
            }
            break;

        case SCE_DART_RAWSTRING_SQ:
        case SCE_DART_RAWSTRING_DQ:
            if (sc.atLineStart) {
                sc.SetState(SCE_DART_DEFAULT);
            }
            else if ((sc.state == SCE_DART_RAWSTRING_SQ && sc.ch == '\'')
                || (sc.state == SCE_DART_RAWSTRING_DQ && sc.ch == '"')) {
                sc.ForwardSetState(SCE_DART_DEFAULT);
            }
            break;

        case SCE_DART_TRIPLE_RAWSTRING_SQ:
        case SCE_DART_TRIPLE_RAWSTRING_DQ:
            if ((sc.state == SCE_DART_TRIPLE_RAWSTRING_SQ && sc.Match('\'', '\'', '\''))
                || (sc.state == SCE_DART_TRIPLE_RAWSTRING_DQ && sc.Match('"', '"', '"'))) {
                sc.Forward(2);
                sc.ForwardSetState(SCE_DART_DEFAULT);
            }
            break;

        case SCE_DART_STRING_SQ:
        case SCE_DART_STRING_DQ:
        case SCE_DART_TRIPLE_STRING_SQ:
        case SCE_DART_TRIPLE_STRING_DQ:
            if (curlyBrace == 0 && (sc.state == SCE_DART_STRING_SQ || sc.state == SCE_DART_STRING_DQ) && sc.atLineStart) {
                sc.SetState(SCE_DART_DEFAULT);
            }
            else if (sc.ch == '\\') {
                if (escSeq.resetEscapeState(sc.state, sc.chNext)) {
                    sc.SetState(SCE_DART_ESCAPECHAR);
                    sc.Forward();
                }
            }
            else if (sc.ch == '$' && IsIdentifierStartEx(sc.chNext)) {
                variableOuter = sc.state;
                sc.SetState(SCE_DART_VARIABLE);
            }
            else if (sc.Match('$', '{')) {
                ++curlyBrace;
                nestedState.push_back(sc.state);
                sc.SetState(SCE_DART_OPERATOR2);
                sc.Forward();
            }
            else if (curlyBrace && sc.ch == '}') {
                const int outerState = TryPopBack(nestedState);
                --curlyBrace;
                sc.SetState(SCE_DART_OPERATOR2);
                sc.ForwardSetState(outerState);
                continue;
            }
            else if (sc.ch == '\'' && (sc.state == SCE_DART_STRING_SQ
                || (sc.state == SCE_DART_TRIPLE_STRING_SQ && sc.Match('\'', '\'', '\'')))) {
                if (sc.state == SCE_DART_TRIPLE_STRING_SQ) {
                    sc.SetState(SCE_DART_TRIPLE_STRING_SQEND);
                    sc.Forward(2);
                }
                sc.ForwardSetState(SCE_DART_DEFAULT);
            }
            else if (sc.ch == '"' && (sc.state == SCE_DART_STRING_DQ
                || (sc.state == SCE_DART_TRIPLE_STRING_DQ && sc.Match('"', '"', '"')))) {
                if (sc.state == SCE_DART_TRIPLE_STRING_DQ) {
                    sc.SetState(SCE_DART_TRIPLE_STRING_DQEND);
                    sc.Forward(2);
                }
                sc.ForwardSetState(SCE_DART_DEFAULT);
            }
            break;
        case SCE_DART_ESCAPECHAR:
            if (escSeq.atEscapeEnd(sc.ch)) {
                const int outerState = escSeq.outerState;
                if (sc.ch == '\\' && escSeq.resetEscapeState(outerState, sc.chNext)) {
                    sc.Forward();
                }
                else {
                    sc.SetState(outerState);
                    continue;
                }
            }
            break;
        case SCE_DART_VARIABLE:
            if (!IsIdentifierCharEx(sc.ch)) {
                sc.SetState(variableOuter);
                continue;
            }
            break;
        }

        if (sc.state == SCE_DART_DEFAULT) {
            if (sc.Match('/', '/')) {
                const int chNext = sc.GetRelative(2);
                sc.SetState((chNext == '/') ? SCE_DART_COMMENTLINEDOC : SCE_DART_COMMENTLINE);
                if (visibleChars == 0) {
                    lineStateLineComment = DartLineStateMaskLineComment;
                }
            }
            else if (sc.Match('/', '*')) {
                const int chNext = sc.GetRelative(2);
                sc.SetState((chNext == '*') ? SCE_DART_COMMENTBLOCKDOC : SCE_DART_COMMENTBLOCK);
                sc.Forward();
                commentLevel = 1;
            }
            else if (sc.ch == 'r' && (sc.chNext == '\'' || sc.chNext == '"')) {
                sc.SetState((sc.chNext == '\'') ? SCE_DART_RAWSTRING_SQ : SCE_DART_RAWSTRING_DQ);
                sc.Forward(2);
                if (sc.chPrev == '\'' && sc.Match('\'', '\'')) {
                    sc.ChangeState(SCE_DART_TRIPLE_RAWSTRING_SQ);
                    sc.Forward(2);
                }
                else if (sc.chPrev == '"' && sc.Match('"', '"')) {
                    sc.ChangeState(SCE_DART_TRIPLE_RAWSTRING_DQ);
                    sc.Forward(2);
                }
                continue;
            }
            else if (sc.Match('"', '"', '"')) {
                sc.ChangeState(SCE_DART_TRIPLE_STRING_DQSTART);
                sc.Forward(2);
                sc.ForwardSetState(SCE_DART_TRIPLE_STRING_DQ);
                continue;
            }
            else if (sc.ch == '"') {
                sc.SetState(SCE_DART_STRING_DQ);
            }
            else if (sc.Match('\'', '\'', '\'')) {
                sc.ChangeState(SCE_DART_TRIPLE_STRING_SQSTART);
                sc.Forward(2);
                sc.ForwardSetState(SCE_DART_TRIPLE_STRING_SQ);
                continue;
            }
            else if (sc.ch == '\'') {
                sc.SetState(SCE_DART_STRING_SQ);
            }
            else if (IsNumberStart(sc.ch, sc.chNext)) {
                sc.SetState(SCE_DART_NUMBER);
            }
            else if ((sc.ch == '@' || sc.ch == '$') && IsIdentifierStartEx(sc.chNext)) {
                variableOuter = SCE_DART_DEFAULT;
                sc.SetState((sc.ch == '@') ? SCE_DART_METADATA : SCE_DART_VARIABLE);
            }
            else if (sc.ch == '#') {
                if (IsIdentifierStartEx(sc.chNext)) {
                    sc.SetState(SCE_DART_SYMBOL_IDENTIFIER);
                }
                else if (isoperator(sc.ch)) {
                    sc.SetState(SCE_DART_SYMBOL_OPERATOR);
                }
            }
            else if (IsIdentifierStartEx(sc.ch)) {
                chBeforeIdentifier = sc.chPrev;
                sc.SetState(SCE_DART_IDENTIFIER);
            }
            else if (isoperator(sc.ch)) {
                sc.SetState(curlyBrace ? SCE_DART_OPERATOR2 : SCE_DART_OPERATOR);
                if (curlyBrace) {
                    if (sc.ch == '{') {
                        ++curlyBrace;
                        nestedState.push_back(SCE_DART_DEFAULT);
                    }
                    else if (sc.ch == '}') {
                        --curlyBrace;
                        const int outerState = TryPopBack(nestedState);
                        sc.ForwardSetState(outerState);
                        continue;
                    }
                }
            }
        }

        if (!isspacechar(sc.ch)) {
            visibleChars++;
        }
        if (sc.atLineEnd) {
            int lineState = (curlyBrace << 8) | (commentLevel << 2) | lineStateLineComment | lineStateImport;
            if (curlyBrace) {
                lineState |= PackNestedState(nestedState);
            }
            styler.SetLineState(sc.currentLine, lineState);
            lineStateLineComment = 0;
            lineStateImport = 0;
            visibleChars = 0;
            kwType = SCE_DART_DEFAULT;
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
        lineComment(lineState& DartLineStateMaskLineComment),
        packageImport((lineState >> 1) & 1) {
    }
};

constexpr bool IsStreamCommentStyle(int style) noexcept {
    return style == SCE_DART_COMMENTBLOCK || style == SCE_DART_COMMENTBLOCKDOC;
}

void SCI_METHOD LexerDart::Fold(Sci_PositionU startPos, Sci_Position length, int, IDocument* pAccess)
{
    if (!options.fold) { return; }

    PropSetSimple props;
    props.SetMultiple(osDart.PropertyNames());

    Accessor styler(pAccess, &props);

    //const int foldComment = styler.GetPropertyInt("fold.comment", 1);
    //const int foldComment = props.GetInt("fold.comment", 1) != 0;
    const int foldComment = options.foldComment;

    const Sci_PositionU endPos = startPos + length;
    Sci_Position lineCurrent = styler.GetLine(startPos);
    FoldLineState foldPrev(0);
    int levelCurrent = SC_FOLDLEVELBASE;
    if (lineCurrent > 0) {
        levelCurrent = styler.LevelAt(lineCurrent - 1) >> 16;
        foldPrev = FoldLineState(styler.GetLineState(lineCurrent - 1));
    }

    int levelNext = levelCurrent;
    FoldLineState foldCurrent(styler.GetLineState(lineCurrent));
    Sci_PositionU lineStartNext = styler.LineStart(lineCurrent + 1);
    Sci_PositionU lineEndPos = ((lineStartNext < endPos) ? lineStartNext : endPos) - 1;

    char chNext = styler[startPos];
    int styleNext = styler.StyleAt(startPos);
    int style = styleNext; //initStyle;

    for (Sci_PositionU i = startPos; i < endPos; i++) {
        const char ch = chNext;
        chNext = styler.SafeGetCharAt(i + 1);
        const int stylePrev = style;
        style = styleNext;
        styleNext = styler.StyleAt(i + 1);

        if (IsStreamCommentStyle(style)) {
            if (foldComment) {
                const int level = (ch == '/' && chNext == '*') ? 1 : ((ch == '*' && chNext == '/') ? -1 : 0);
                if (level != 0) {
                    levelNext += level;
                    i++;
                    chNext = styler.SafeGetCharAt(i + 1);
                    styleNext = styler.StyleAt(i + 1);
                }
            }
        }
        else if (style == SCE_DART_TRIPLE_RAWSTRING_SQ || style == SCE_DART_TRIPLE_RAWSTRING_DQ) {
            if (style != stylePrev) {
                levelNext++;
            }
            else if (style != styleNext) {
                levelNext--;
            }
        }
        else if (style == SCE_DART_TRIPLE_STRING_SQSTART || style == SCE_DART_TRIPLE_STRING_DQSTART) {
            if (style != stylePrev) {
                levelNext++;
            }
        }
        else if (style == SCE_DART_TRIPLE_STRING_SQEND || style == SCE_DART_TRIPLE_STRING_DQEND) {
            if (style != styleNext) {
                levelNext--;
            }
        }
        else if (style == SCE_DART_OPERATOR) {
            if (ch == '{' || ch == '[' || ch == '(') {
                levelNext++;
            }
            else if (ch == '}' || ch == ']' || ch == ')') {
                levelNext--;
            }
        }

        if (i == lineEndPos) {
            const FoldLineState foldNext(styler.GetLineState(lineCurrent + 1));
            if (foldComment & foldCurrent.lineComment) {
                levelNext += foldNext.lineComment - foldPrev.lineComment;
            }
            else if (foldCurrent.packageImport) {
                levelNext += foldNext.packageImport - foldPrev.packageImport;
            }

            const int levelUse = levelCurrent;
            int lev = levelUse | levelNext << 16;
            if (levelUse < levelNext) {
                lev |= SC_FOLDLEVELHEADERFLAG;
            }
            if (lev != styler.LevelAt(lineCurrent)) {
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

LexerModule lmDart(SCLEX_DART, LexerDart::LexerFactoryDart, "Dart", dartWordLists);

// ----------------------------------------------------------------------------

