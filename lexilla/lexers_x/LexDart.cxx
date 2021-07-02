// encoding: UTF-8
// Scintilla source code edit control
/** @file LexDart.cxx
** Lexer for Dart Language Source Code
** Original Code by Zufuliu, Adapted by RaiKoHoff
** Dart Spec:  https://dart.dev/guides/language/spec
**/

#include <cassert>
#include <string>
#include <map>
#include <vector>

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
#include "StringUtils.h"
#include "LexerUtils.h"


using namespace Lexilla;
using namespace Scintilla;


namespace
{
// Use an unnamed namespace to protect the functions and classes from name conflicts

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
    "Keywords2",
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
    SCE_DART_DEFAULT,             "SCE_DART_DEFAULT",                 "default",              "Descr",
    SCE_DART_COMMENTLINE,         "SCE_DART_COMMENTLINE",             "commentline",          "Descr",
    SCE_DART_COMMENTLINEDOC,      "SCE_DART_COMMENTLINEDOC",          "commentlinedoc",       "Descr",
    SCE_DART_COMMENTBLOCK,        "SCE_DART_COMMENTBLOCK",            "commentblock",         "Descr",
    SCE_DART_COMMENTBLOCKDOC,     "SCE_DART_COMMENTBLOCKDOC",         "commentblockdoc",      "Descr",
    SCE_DART_TASKMARKER,          "SCE_DART_TASKMARKER",              "taskmarker",           "Descr",
    SCE_DART_NUMBER,              "SCE_DART_NUMBER",                  "number",               "Descr",
    SCE_DART_OPERATOR,            "SCE_DART_OPERATOR",                "operator",             "Descr",
    SCE_DART_OPERATOR2,           "SCE_DART_OPERATOR2",               "operator2",            "Descr",
    SCE_DART_IDENTIFIER,          "SCE_DART_IDENTIFIER",              "identifier",           "Descr",
    SCE_DART_ESCAPECHAR,          "SCE_DART_ESCAPECHAR ",             "string_sq",            "Descr",
    SCE_DART_STRING_SQ,           "SCE_DART_STRING_SQ",               "string_dq",            "Descr",
    SCE_DART_STRING_DQ,           "SCE_DART_STRING_DQ",               "tristrg_sq",           "Descr",
    SCE_DART_TRIPLE_STRING_SQ,    "SCE_DART_TRIPLE_STRING_SQ",        "tristrg_dq",           "Descr",
    SCE_DART_TRIPLE_STRING_DQ,    "SCE_DART_TRIPLE_STRING_DQ",        "escchar",              "Descr",
    SCE_DART_RAWSTRING_SQ,        "SCE_DART_RAWSTRING_SQ",            "rawstrg_sq",           "Descr",
    SCE_DART_RAWSTRING_DQ,        "SCE_DART_RAWSTRING_DQ",            "rawstrg_dq",           "Descr",
    SCE_DART_TRIPLE_RAWSTRING_SQ, "SCE_DART_TRIPLE_RAWSTRING_SQ",     "trirawstrg_sq",        "Descr",
    SCE_DART_TRIPLE_RAWSTRING_DQ, "SCE_DART_TRIPLE_RAWSTRING_DQ",     "trirawstrg_dq",        "Descr",
    SCE_DART_TRIPLE_STRINGSTART,  "SCE_DART_TRIPLE_STRINGSTART",      "string_beg",           "Descr",
    SCE_DART_TRIPLE_STRINGEND,    "SCE_DART_TRIPLE_STRINGEND",        "string_end",           "Descr",
    SCE_DART_SYMBOL_OPERATOR,     "SCE_DART_SYMBOL_OPERATOR",         "sym_operator",         "Descr",
    SCE_DART_SYMBOL_IDENTIFIER,   "SCE_DART_SYMBOL_IDENTIFIER",       "sym_identifier",       "Descr",
    SCE_DART_VARIABLE,            "SCE_DART_VARIABLE",                "variable",             "Descr",
    SCE_DART_METADATA,            "SCE_DART_METADATA",                "metadata",             "Descr",
    SCE_DART_LABEL,               "SCE_DART_LABEL",                   "label",                "Descr",
    SCE_DART_FUNCTION,            "SCE_DART_FUNCTION",                "function",             "Descr",
    SCE_DART_WORD,                "SCE_DART_WORD",                    "keword",               "Descr",
    SCE_DART_WORD2,               "SCE_DART_WORD2",                   "keyword2",             "Descr",
    SCE_DART_CLASS,               "SCE_DART_CLASS",                   "class",                "Descr",
    SCE_DART_ENUM,                "SCE_DART_ENUM",                    "enum",                 "Descr"
};



} // end of namespace

class LexerDart : public DefaultLexer
{

    //CharacterSet validKey;
    //CharacterSet validKeyWord;
    //CharacterSet validNumberEnd;
    //CharacterSet chDateTime;

    WordList wl_keywords;
    WordList wl_keywords2;
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
        wordListN = &wl_keywords2;
        break;
    case 2:
        wordListN = &wl_types;
        break;
    case 3:
        wordListN = &wl_classes;
        break;
    case 4:
        wordListN = &wl_enums;
        break;
    case 5:
        wordListN = &wl_metadata;
        break;
    case 6:
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
        --digitsLeft;
        return digitsLeft <= 0 || !IsAHexDigit(ch);
    }
};


enum {
    DartLineStateMaskLineComment = 1,	// line comment
    DartLineStateMaskImport = (1 << 1),	// import
};

static_assert(DefaultNestedStateBaseStyle + 1 == SCE_DART_STRING_SQ);
static_assert(DefaultNestedStateBaseStyle + 2 == SCE_DART_STRING_DQ);
static_assert(DefaultNestedStateBaseStyle + 3 == SCE_DART_TRIPLE_STRING_SQ);
static_assert(DefaultNestedStateBaseStyle + 4 == SCE_DART_TRIPLE_STRING_DQ);

constexpr bool IsDeclarableOperator(int ch) noexcept {
    // https://github.com/dart-lang/sdk/blob/master/sdk/lib/core/symbol.dart
    return AnyOf(ch, '+', '-', '*', '/', '%', '~', '&', '|',
        '^', '<', '>', '=', '[', ']');
}

constexpr bool IsSpaceEquiv(int state) noexcept {
    return state <= SCE_DART_TASKMARKER;
}



void SCI_METHOD LexerDart::Lex(Sci_PositionU startPos, Sci_Position lengthDoc, int initStyle, IDocument* pAccess)
{
    PropSetSimple props;
    Accessor styler(pAccess, &props);
    StyleContext sc(startPos, lengthDoc, initStyle, styler);

    int lineStateLineType = 0;
    int commentLevel = 0;	// nested block comment level

    int kwType = SCE_DART_DEFAULT;
    int chBeforeIdentifier = 0;

    std::vector<int> nestedState; // string interpolation "${}"

    int visibleChars = 0;
    int visibleCharsBefore = 0;
    EscapeSequence escSeq;

    if (sc.currentLine > 0) {
        int lineState = styler.GetLineState(sc.currentLine - 1);
        /*
        2: lineStateLineType
        6: commentLevel
        3: nestedState count
        3*4: nestedState
        */
        commentLevel = (lineState >> 2) & 0x3f;
        lineState >>= 8;
        if (lineState) {
            UnpackLineState(lineState, nestedState);
        }
    }
    if (startPos == 0 && sc.Match('#', '!')) {
        // Shell Shebang at beginning of file
        sc.SetState(SCE_DART_COMMENTLINE);
        sc.Forward();
        lineStateLineType = DartLineStateMaskLineComment;
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
                    if (StrEqualsAny(s, "import", "part")) {
                        if (visibleChars == sc.LengthCurrent()) {
                            lineStateLineType = DartLineStateMaskImport;
                        }
                    }
                    else if (StrEqualsAny(s, "class", "extends", "implements", "new", "throw", "as", "is")) {
                        kwType = SCE_DART_CLASS;
                    }
                    else if (StrEqual(s, "enum")) {
                        kwType = SCE_DART_ENUM;
                    }
                    else if (StrEqualsAny(s, "break", "continue")) {
                        kwType = SCE_DART_LABEL;
                    }
                    if (kwType != SCE_DART_DEFAULT) {
                        const int chNext = GetLineNextChar(styler, sc);
                        if (!IsIdentifierStartEx(chNext)) {
                            kwType = SCE_DART_DEFAULT;
                        }
                    }
                }
                else if (wl_keywords2.InList(s)) {
                    sc.ChangeState(SCE_DART_WORD2);
                }
                else if (wl_classes.InList(s)) {
                    sc.ChangeState(SCE_DART_CLASS);
                }
                else if (wl_enums.InList(s)) {
                    sc.ChangeState(SCE_DART_ENUM);
                }
                else if (sc.ch == ':') {
                    if (visibleChars == sc.LengthCurrent()) {
                        const int chNext = GetLineNextChar(styler, sc, true);
                        if (IsJumpLabelNextChar(chNext)) {
                            sc.ChangeState(SCE_DART_LABEL);
                        }
                    }
                }
                else if (sc.ch != '.') {
                    if (kwType != SCE_DART_DEFAULT) {
                        sc.ChangeState(kwType);
                    }
                    else {
                        const int chNext = GetDocNextChar(styler, sc, (sc.ch == '?'));
                        if (chNext == '(') {
                            sc.ChangeState(SCE_DART_FUNCTION);
                        }
                        else if ((chBeforeIdentifier == '<' && (chNext == '>' || chNext == '<'))
                            || IsIdentifierStartEx(chNext)) {
                            // type<type>
                            // type<type?>
                            // type<type<type>>
                            // type identifier
                            // type? identifier
                            sc.ChangeState(SCE_DART_CLASS);
                        }
                    }
                }
                if (sc.state != SCE_DART_WORD && sc.ch != '.') {
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
            if (!IsDeclarableOperator(sc.ch)) {
                sc.SetState(SCE_DART_DEFAULT);
            }
            break;

        case SCE_DART_COMMENTLINE:
        case SCE_DART_COMMENTLINEDOC:
            if (sc.atLineStart) {
                sc.SetState(SCE_DART_DEFAULT);
            }
            else {
                HighlightTaskMarker(sc, visibleChars, visibleCharsBefore, SCE_DART_TASKMARKER);
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
                sc.Forward();
                ++commentLevel;
            }
            else if (HighlightTaskMarker(sc, visibleChars, visibleCharsBefore, SCE_DART_TASKMARKER)) {
                continue;
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
            if ((sc.state == SCE_DART_STRING_SQ || sc.state == SCE_DART_STRING_DQ) && sc.atLineStart) {
                sc.SetState(SCE_DART_DEFAULT);
            }
            else if (sc.ch == '\\') {
                if (escSeq.resetEscapeState(sc.state, sc.chNext)) {
                    sc.SetState(SCE_DART_ESCAPECHAR);
                    sc.Forward();
                }
            }
            else if (sc.ch == '$') {
                if (sc.chNext == '{') {
                    nestedState.push_back(sc.state);
                    sc.SetState(SCE_DART_OPERATOR2);
                    sc.Forward();
                }
                else if (IsIdentifierStartEx(sc.chNext)) {
                    escSeq.outerState = sc.state;
                    sc.SetState(SCE_DART_VARIABLE);
                }
            }
            else if ((sc.ch == '\'' && (sc.state == SCE_DART_STRING_SQ || (sc.state == SCE_DART_TRIPLE_STRING_SQ && sc.MatchNext('\'', '\''))))
                || (sc.ch == '"' && (sc.state == SCE_DART_STRING_DQ || (sc.state == SCE_DART_TRIPLE_STRING_DQ && sc.MatchNext('"', '"'))))) {
                if (sc.state == SCE_DART_TRIPLE_STRING_SQ || sc.state == SCE_DART_TRIPLE_STRING_DQ) {
                    sc.SetState(SCE_DART_TRIPLE_STRINGEND);
                    sc.Forward(2);
                }
                sc.ForwardSetState(SCE_DART_DEFAULT);
            }
            break;

        case SCE_DART_ESCAPECHAR:
            if (escSeq.atEscapeEnd(sc.ch)) {
                sc.SetState(escSeq.outerState);
                continue;
            }
            break;

        case SCE_DART_VARIABLE:
            if (!IsIdentifierCharEx(sc.ch)) {
                sc.SetState(escSeq.outerState);
                continue;
            }
            break;
        }

        if (sc.state == SCE_DART_DEFAULT) {
            if (sc.ch == '/' && (sc.chNext == '/' || sc.chNext == '*')) {
                visibleCharsBefore = visibleChars;
                const int chNext = sc.chNext;
                sc.SetState((chNext == '/') ? SCE_DART_COMMENTLINE : SCE_DART_COMMENTBLOCK);
                sc.Forward(2);
                if (sc.ch == chNext && sc.chNext != chNext) {
                    sc.ChangeState((chNext == '/') ? SCE_DART_COMMENTLINEDOC : SCE_DART_COMMENTBLOCKDOC);
                }
                if (chNext == '/') {
                    if (visibleChars == 0) {
                        lineStateLineType = DartLineStateMaskLineComment;
                    }
                }
                else {
                    commentLevel = 1;
                }
                continue;
            }
            if (sc.ch == 'r' && (sc.chNext == '\'' || sc.chNext == '"')) {
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
            if (sc.ch == '"') {
                if (sc.MatchNext('"', '"')) {
                    sc.SetState(SCE_DART_TRIPLE_STRINGSTART);
                    sc.Forward(2);
                    sc.ForwardSetState(SCE_DART_TRIPLE_STRING_DQ);
                    continue;
                }
                sc.SetState(SCE_DART_STRING_DQ);
            }
            else if (sc.ch == '\'') {
                if (sc.MatchNext('\'', '\'')) {
                    sc.SetState(SCE_DART_TRIPLE_STRINGSTART);
                    sc.Forward(2);
                    sc.ForwardSetState(SCE_DART_TRIPLE_STRING_SQ);
                    continue;
                }
                sc.SetState(SCE_DART_STRING_SQ);
            }
            else if (IsNumberStart(sc.ch, sc.chNext)) {
                sc.SetState(SCE_DART_NUMBER);
            }
            else if ((sc.ch == '@' || sc.ch == '$') && IsIdentifierStartEx(sc.chNext)) {
                escSeq.outerState = SCE_DART_DEFAULT;
                sc.SetState((sc.ch == '@') ? SCE_DART_METADATA : SCE_DART_VARIABLE);
            }
            else if (sc.ch == '#') {
                if (IsIdentifierStartEx(sc.chNext)) {
                    sc.SetState(SCE_DART_SYMBOL_IDENTIFIER);
                }
                else if (IsDeclarableOperator(sc.chNext)) {
                    sc.SetState(SCE_DART_SYMBOL_OPERATOR);
                }
            }
            else if (IsIdentifierStartEx(sc.ch)) {
                if (sc.chPrev != '.') {
                    chBeforeIdentifier = sc.chPrev;
                }
                sc.SetState(SCE_DART_IDENTIFIER);
            }
            else if (isoperator(sc.ch)) {
                const bool interpolating = !nestedState.empty();
                sc.SetState(interpolating ? SCE_DART_OPERATOR2 : SCE_DART_OPERATOR);
                if (interpolating) {
                    if (sc.ch == '{') {
                        nestedState.push_back(SCE_DART_DEFAULT);
                    }
                    else if (sc.ch == '}') {
                        const int outerState = TakeAndPop(nestedState);
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
            int lineState = (commentLevel << 2) | lineStateLineType;
            if (!nestedState.empty()) {
                lineState |= PackLineState(nestedState) << 8;
            }
            styler.SetLineState(sc.currentLine, lineState);
            lineStateLineType = 0;
            visibleChars = 0;
            visibleCharsBefore = 0;
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

void SCI_METHOD LexerDart::Fold(Sci_PositionU startPos, Sci_Position lengthDoc, int, IDocument* pAccess)
{
    if (!options.fold) { return; }

    PropSetSimple props;
    Accessor styler(pAccess, &props);

    //const int foldComment = styler.GetPropertyInt("fold.comment", 1);
    //const int foldComment = props.GetInt("fold.comment", 1) != 0;
    const int foldComment = options.foldComment;

    const Sci_PositionU endPos = startPos + lengthDoc;
    Sci_Position lineCurrent = styler.GetLine(startPos);
    FoldLineState foldPrev(0);
    int levelCurrent = SC_FOLDLEVELBASE;
    if (lineCurrent > 0) {
        levelCurrent = styler.LevelAt(lineCurrent - 1) >> 16;
        foldPrev = FoldLineState(styler.GetLineState(lineCurrent - 1));
        const Sci_PositionU bracePos = CheckBraceOnNextLine(styler, lineCurrent - 1, SCE_DART_OPERATOR, SCE_DART_TASKMARKER);
        if (bracePos) {
            startPos = bracePos + 1; // skip the brace
        }
    }

    int levelNext = levelCurrent;
    FoldLineState foldCurrent(styler.GetLineState(lineCurrent));
    Sci_PositionU lineStartNext = styler.LineStart(lineCurrent + 1);
    Sci_PositionU lineEndPos = sci::min(lineStartNext, endPos) - 1;

    char chNext = styler[startPos];
    int styleNext = styler.StyleAt(startPos);
    int style = styleNext;
    int visibleChars = 0;

    for (Sci_PositionU i = startPos; i < endPos; i++) {
        const char ch = chNext;
        chNext = styler.SafeGetCharAt(i + 1);
        const int stylePrev = style;
        style = styleNext;
        styleNext = styler.StyleAt(i + 1);

        switch (style) {
        case SCE_DART_COMMENTBLOCKDOC:
        case SCE_DART_COMMENTBLOCK: {
                const int level = (ch == '/' && chNext == '*') ? 1 : ((ch == '*' && chNext == '/') ? -1 : 0);
                if (level != 0) {
                    levelNext += level;
                    i++;
                    chNext = styler.SafeGetCharAt(i + 1);
                    styleNext = styler.StyleAt(i + 1);
                }
            } break;

        case SCE_DART_TRIPLE_RAWSTRING_SQ:
        case SCE_DART_TRIPLE_RAWSTRING_DQ:
            if (style != stylePrev) {
                levelNext++;
            }
            else if (style != styleNext) {
                levelNext--;
            }
            break;

        case SCE_DART_TRIPLE_STRINGSTART:
            if (style != stylePrev) {
                levelNext++;
            }
            break;

        case SCE_DART_TRIPLE_STRINGEND:
            if (style != styleNext) {
                levelNext--;
            }
            break;

        case SCE_DART_OPERATOR:
            if (ch == '{' || ch == '[' || ch == '(') {
                levelNext++;
            }
            else if (ch == '}' || ch == ']' || ch == ')') {
                levelNext--;
            }
            break;
        }

        if (visibleChars == 0 && !IsSpaceEquiv(style)) {
            ++visibleChars;
        }
        if (i == lineEndPos) {
            const FoldLineState foldNext(styler.GetLineState(lineCurrent + 1));
            if (foldCurrent.lineComment) {
                levelNext += foldNext.lineComment - foldPrev.lineComment;
            }
            else if (foldCurrent.packageImport) {
                levelNext += foldNext.packageImport - foldPrev.packageImport;
            }
            else if (visibleChars) {
                const Sci_PositionU bracePos = CheckBraceOnNextLine(styler, lineCurrent, SCE_DART_OPERATOR, SCE_DART_TASKMARKER);
                if (bracePos) {
                    levelNext++;
                    i = bracePos; // skip the brace
                    chNext = '\0';
                }
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
            lineEndPos = sci::min(lineStartNext, endPos) - 1;
            levelCurrent = levelNext;
            foldPrev = foldCurrent;
            foldCurrent = foldNext;
            visibleChars = 0;
        }
    }
}
// ----------------------------------------------------------------------------

LexerModule lmDart(SCLEX_DART, LexerDart::LexerFactoryDart, "Dart", dartWordLists);

// ----------------------------------------------------------------------------

