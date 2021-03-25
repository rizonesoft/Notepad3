// encoding: UTF-8
// Scintilla source code edit control
/** @file LexKotlin.cxx
** Lexer for Kotlin Language Source Code
** Original Code by Zufuliu, Adapted by RaiKoHoff
** Kotlin Ref: https://kotlinlang.org/docs/reference/
**/

#include <cassert>
#include <string>
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
#include "StringUtils.h"
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
    SCE_KOTLIN_DEFAULT,         "SCE_KOTLIN_DEFAULT",            "default",              "Descr",
    SCE_KOTLIN_COMMENTLINE,     "SCE_KOTLIN_COMMENTLINE",        "commentline",          "Descr",
    SCE_KOTLIN_COMMENTLINEDOC,  "SCE_KOTLIN_COMMENTLINEDOC",     "commentlinedoc",       "Descr",
    SCE_KOTLIN_COMMENTBLOCK,    "SCE_KOTLIN_COMMENTBLOCK",       "commentblock",         "Descr",
    SCE_KOTLIN_COMMENTBLOCKDOC, "SCE_KOTLIN_COMMENTBLOCKDOC",    "commentblockdoc",      "Descr",
    SCE_KOTLIN_COMMENTDOCWORD,  "SCE_KOTLIN_COMMENTDOCWORD",     "commentdocword",       "Descr",
    SCE_KOTLIN_TASKMARKER,      "SCE_KOTLIN_TASKMARKER",         "taskmarker",           "Descr",
    SCE_KOTLIN_NUMBER,          "SCE_KOTLIN_NUMBER",             "number",               "Descr",
    SCE_KOTLIN_OPERATOR,        "SCE_KOTLIN_OPERATOR",           "operator",             "Descr",
    SCE_KOTLIN_OPERATOR2,       "SCE_KOTLIN_OPERATOR2",          "operator2",            "Descr",
    SCE_KOTLIN_CHARACTER,       "SCE_KOTLIN_CHARACTER",          "character",            "Descr",
    SCE_KOTLIN_STRING,          "SCE_KOTLIN_STRING",             "string",               "Descr",
    SCE_KOTLIN_RAWSTRING,       "SCE_KOTLIN_RAWSTRING ",         "rawstring",            "Descr",
    SCE_KOTLIN_ESCAPECHAR,      "SCE_KOTLIN_ESCAPECHAR",         "escapechar",           "Descr",
    SCE_KOTLIN_RAWSTRINGSTART,  "SCE_KOTLIN_RAWSTRINGSTART",     "rawstring_start",      "Descr",
    SCE_KOTLIN_RAWSTRINGEND,    "SCE_KOTLIN_RAWSTRINGEND",       "rawstring_end",        "Descr",
    SCE_KOTLIN_BACKTICKS,       "SCE_KOTLIN_BACKTICKS",          "backticks",            "Descr",
    SCE_KOTLIN_VARIABLE,        "SCE_KOTLIN_VARIABLE",           "variable",             "Descr",
    SCE_KOTLIN_ANNOTATION,      "SCE_KOTLIN_ANNOTATION",         "annotation",           "Descr",
    SCE_KOTLIN_LABEL,           "SCE_KOTLIN_LABEL",              "label",                "Descr",
    SCE_KOTLIN_IDENTIFIER,      "SCE_KOTLIN_IDENTIFIER",         "identifier",           "Descr",
    SCE_KOTLIN_WORD,            "SCE_KOTLIN_WORD",               "word",                 "Descr",
    SCE_KOTLIN_CLASS,           "SCE_KOTLIN_CLASS",              "class",                "Descr",
    SCE_KOTLIN_INTERFACE,       "SCE_KOTLIN_INTERFACE",          "interface",            "Descr",
    SCE_KOTLIN_ENUM,            "SCE_KOTLIN_ENUM",               "enum",                 "Descr",
    SCE_KOTLIN_FUNCTION,        "SCE_KOTLIN_FUNCTION",           "function",             "Descr",
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

enum {
    KotlinLineStateMaskLineComment = 1, // line comment
    KotlinLineStateMaskImport = 1 << 1, // import
};

static_assert(DefaultNestedStateBaseStyle + 1 == SCE_KOTLIN_STRING);
static_assert(DefaultNestedStateBaseStyle + 2 == SCE_KOTLIN_RAWSTRING);

struct EscapeSequence {
    int outerState = SCE_KOTLIN_DEFAULT;
    int digitsLeft = 0;

    // highlight any character as escape sequence.
    bool resetEscapeState(int state, int chNext) noexcept {
        if (IsLineBreak(chNext)) {
            return false;
        }
        outerState = state;
        digitsLeft = (chNext == 'u') ? 5 : 1;
        return true;
    }
    bool atEscapeEnd(int ch) noexcept {
        --digitsLeft;
        return digitsLeft <= 0 || !IsAHexDigit(ch);
    }
};


constexpr bool IsSpaceEquiv(int state) noexcept {
    return state <= SCE_KOTLIN_TASKMARKER;
}

constexpr bool IsCommentTagPrev(int chPrev) noexcept {
    return chPrev <= 32 || AnyOf(chPrev, '/', '*', '!');
}



void SCI_METHOD LexerKotlin::Lex(Sci_PositionU startPos, Sci_Position lengthDoc, int initStyle, IDocument* pAccess)
{
    PropSetSimple props;
    props.SetMultiple(osKotlin.PropertyNames());

    Accessor styler(pAccess, &props);
    StyleContext sc(startPos, lengthDoc, initStyle, styler);

    int lineStateLineType = 0;
    int commentLevel = 0;	// nested block comment level

    int kwType = SCE_KOTLIN_DEFAULT;
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
        sc.SetState(SCE_KOTLIN_COMMENTLINE);
        sc.Forward();
        lineStateLineType = KotlinLineStateMaskLineComment;
    }

    while (sc.More()) {
        switch (sc.state) {
        case SCE_KOTLIN_OPERATOR:
        case SCE_KOTLIN_OPERATOR2:
            sc.SetState(SCE_KOTLIN_DEFAULT);
            break;

        case SCE_KOTLIN_NUMBER:
            if (!IsDecimalNumber(sc.chPrev, sc.ch, sc.chNext)) {
                sc.SetState(SCE_KOTLIN_DEFAULT);
            }
            break;

        case SCE_KOTLIN_IDENTIFIER:
            if (!IsIdentifierCharEx(sc.ch)) {
                char s[128];
                sc.GetCurrent(s, sizeof(s));
                if (wl_keywords.InList(s)) {
                    sc.ChangeState(SCE_KOTLIN_WORD);
                    if (StrEqual(s, "import")) {
                        if (visibleChars == sc.LengthCurrent()) {
                            lineStateLineType = KotlinLineStateMaskImport;
                        }
                    }
                    else if (StrEqualsAny(s, "break", "continue", "return", "this", "super")) {
                        kwType = SCE_KOTLIN_LABEL;
                    }
                    else if (StrEqualsAny(s, "class", "typealias")) {
                        if (!(kwType == SCE_KOTLIN_ANNOTATION || kwType == SCE_KOTLIN_ENUM)) {
                            kwType = SCE_KOTLIN_CLASS;
                        }
                    }
                    else if (StrEqual(s, "enum")) {
                        kwType = SCE_KOTLIN_ENUM;
                    }
                    else if (StrEqual(s, "annotation")) {
                        kwType = SCE_KOTLIN_ANNOTATION;
                    }
                    else if (StrEqual(s, "interface")) {
                        kwType = SCE_KOTLIN_INTERFACE;
                    }
                    if (kwType != SCE_KOTLIN_DEFAULT) {
                        const int chNext = GetDocNextChar(styler, sc);
                        if (!((kwType == SCE_KOTLIN_LABEL && sc.ch == '@') || (kwType != SCE_KOTLIN_LABEL && IsIdentifierStartEx(sc.ch)))) {
                            kwType = SCE_KOTLIN_DEFAULT;
                        }
                    }
                }
                else if (sc.ch == '@') {
                    sc.ChangeState(SCE_KOTLIN_LABEL);
                    sc.Forward();
                }
                else if (wl_classes.InList(s)) {
                    sc.ChangeState(SCE_KOTLIN_CLASS);
                }
                else if (wl_interfaces.InList(s)) {
                    sc.ChangeState(SCE_KOTLIN_INTERFACE);
                }
                else if (wl_enums.InList(s)) {
                    sc.ChangeState(SCE_KOTLIN_ENUM);
                }
                else if (sc.ch != '.') {
                    if (kwType != SCE_KOTLIN_DEFAULT && kwType != SCE_KOTLIN_LABEL) {
                        sc.ChangeState(kwType);
                    }
                    else {
                        const int chNext = GetDocNextChar(styler, sc, (sc.ch == '?'));
                        if (sc.ch == '(') {
                            sc.ChangeState(SCE_KOTLIN_FUNCTION);
                        }
                        else if (sc.Match(':', ':')
                            || (chBeforeIdentifier == '<' && (sc.ch == '>' || sc.ch == '<'))) {
                            // type::class
                            // type<type>
                            // type<type?>
                            // type<type<type>>
                            sc.ChangeState(SCE_KOTLIN_CLASS);
                        }
                    }
                }
                if (sc.state != SCE_KOTLIN_WORD && sc.ch != '.') {
                    kwType = SCE_KOTLIN_DEFAULT;
                }
                sc.SetState(SCE_KOTLIN_DEFAULT);
            }
            break;

        case SCE_KOTLIN_LABEL:
            if (!IsIdentifierCharEx(sc.ch)) {
                sc.SetState(SCE_KOTLIN_DEFAULT);
            }
            break;

        case SCE_KOTLIN_ANNOTATION:
            if (sc.ch == '.' || sc.ch == ':') {
                sc.SetState(SCE_KOTLIN_OPERATOR);
                sc.ForwardSetState(SCE_KOTLIN_ANNOTATION);
                continue;
            }
            if (!IsIdentifierCharEx(sc.ch)) {
                sc.SetState(SCE_KOTLIN_DEFAULT);
            }
            break;

        case SCE_KOTLIN_COMMENTLINE:
        case SCE_KOTLIN_COMMENTLINEDOC:
            if (sc.atLineStart) {
                sc.SetState(SCE_KOTLIN_DEFAULT);
            }
            else {
                HighlightTaskMarker(sc, visibleChars, visibleCharsBefore, SCE_KOTLIN_TASKMARKER);
            }
            break;

        case SCE_KOTLIN_COMMENTBLOCK:
        case SCE_KOTLIN_COMMENTBLOCKDOC:
            if (sc.state == SCE_KOTLIN_COMMENTBLOCKDOC && sc.ch == '@' && IsLowerCase(sc.chNext) && IsCommentTagPrev(sc.chPrev)) {
                sc.SetState(SCE_KOTLIN_COMMENTDOCWORD);
            }
            else if (sc.Match('*', '/')) {
                sc.Forward();
                --commentLevel;
                if (commentLevel == 0) {
                    sc.ForwardSetState(SCE_KOTLIN_DEFAULT);
                }
            }
            else if (sc.Match('/', '*')) {
                sc.Forward();
                ++commentLevel;
            }
            else if (HighlightTaskMarker(sc, visibleChars, visibleCharsBefore, SCE_KOTLIN_TASKMARKER)) {
                continue;
            }
            break;

        case SCE_KOTLIN_COMMENTDOCWORD:
            if (!IsLowerCase(sc.ch)) {
                sc.SetState(SCE_KOTLIN_COMMENTBLOCKDOC);
                continue;
            }
            break;

        case SCE_KOTLIN_STRING:
        case SCE_KOTLIN_RAWSTRING:
            if (sc.state == SCE_KOTLIN_STRING && sc.atLineStart) {
                sc.SetState(SCE_KOTLIN_DEFAULT);
            }
            else if (sc.state == SCE_KOTLIN_STRING && sc.ch == '\\') {
                if (escSeq.resetEscapeState(sc.state, sc.chNext)) {
                    sc.SetState(SCE_KOTLIN_ESCAPECHAR);
                    sc.Forward();
                }
            }
            else if (sc.ch == '$') {
                if (sc.chNext == '{') {
                    nestedState.push_back(sc.state);
                    sc.SetState(SCE_KOTLIN_OPERATOR2);
                    sc.Forward();
                }
                else if (IsIdentifierStartEx(sc.chNext)) {
                    escSeq.outerState = sc.state;
                    sc.SetState(SCE_KOTLIN_VARIABLE);
                }
            }
            else if (sc.ch == '\"' && (sc.state == SCE_KOTLIN_STRING || sc.MatchNext('"', '"'))) {
                if (sc.state == SCE_KOTLIN_RAWSTRING) {
                    sc.SetState(SCE_KOTLIN_RAWSTRINGEND);
                    sc.Forward(2);
                }
                sc.ForwardSetState(SCE_KOTLIN_DEFAULT);
            }
            break;

        case SCE_KOTLIN_CHARACTER:
            if (sc.atLineStart) {
                sc.SetState(SCE_KOTLIN_DEFAULT);
            }
            else if (sc.ch == '\\') {
                if (escSeq.resetEscapeState(sc.state, sc.chNext)) {
                    sc.SetState(SCE_KOTLIN_ESCAPECHAR);
                    sc.Forward();
                }
            }
            else if (sc.ch == '\'') {
                sc.ForwardSetState(SCE_KOTLIN_DEFAULT);
            }
            break;

        case SCE_KOTLIN_ESCAPECHAR:
            if (escSeq.atEscapeEnd(sc.ch)) {
                sc.SetState(escSeq.outerState);
                continue;
            }
            break;

        case SCE_KOTLIN_VARIABLE:
            if (!IsIdentifierCharEx(sc.ch)) {
                sc.SetState(escSeq.outerState);
                continue;
            }
            break;

        case SCE_KOTLIN_BACKTICKS:
            if (sc.atLineStart) {
                sc.SetState(SCE_KOTLIN_DEFAULT);
            }
            else if (sc.ch == '`') {
                sc.ForwardSetState(SCE_KOTLIN_DEFAULT);
            }
            break;
        }

        if (sc.state == SCE_KOTLIN_DEFAULT) {
            if (sc.ch == '/' && (sc.chNext == '/' || sc.chNext == '*')) {
                visibleCharsBefore = visibleChars;
                const int chNext = sc.chNext;
                sc.SetState((chNext == '/') ? SCE_KOTLIN_COMMENTLINE : SCE_KOTLIN_COMMENTBLOCK);
                sc.Forward(2);
                if (sc.ch == '!' || (sc.ch == chNext && sc.chNext != chNext)) {
                    sc.ChangeState((chNext == '/') ? SCE_KOTLIN_COMMENTLINEDOC : SCE_KOTLIN_COMMENTBLOCKDOC);
                }
                if (chNext == '/') {
                    if (visibleChars == 0) {
                        lineStateLineType = KotlinLineStateMaskLineComment;
                    }
                }
                else {
                    commentLevel = 1;
                }
                continue;
            }
            if (sc.ch == '\"') {
                if (sc.MatchNext('"', '"')) {
                    sc.SetState(SCE_KOTLIN_RAWSTRINGSTART);
                    sc.Forward(2);
                    sc.ForwardSetState(SCE_KOTLIN_RAWSTRING);
                    continue;
                }
                sc.SetState(SCE_KOTLIN_STRING);
            }
            else if (sc.ch == '\'') {
                sc.SetState(SCE_KOTLIN_CHARACTER);
            }
            else if (IsNumberStart(sc.ch, sc.chNext)) {
                sc.SetState(SCE_KOTLIN_NUMBER);
            }
            else if (sc.ch == '@' && IsIdentifierStartEx(sc.chNext)) {
                sc.SetState((kwType == SCE_KOTLIN_LABEL) ? SCE_KOTLIN_LABEL : SCE_KOTLIN_ANNOTATION);
                kwType = SCE_KOTLIN_DEFAULT;
            }
            else if (sc.ch == '`') {
                sc.SetState(SCE_KOTLIN_BACKTICKS);
            }
            else if (IsIdentifierStartEx(sc.ch)) {
                if (sc.chPrev != '.') {
                    chBeforeIdentifier = sc.chPrev;
                }
                sc.SetState(SCE_KOTLIN_IDENTIFIER);
            }
            else if (isoperator(sc.ch)) {
                const bool interpolating = !nestedState.empty();
                sc.SetState(interpolating ? SCE_KOTLIN_OPERATOR2 : SCE_KOTLIN_OPERATOR);
                if (interpolating) {
                    if (sc.ch == '{') {
                        nestedState.push_back(SCE_KOTLIN_DEFAULT);
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

void SCI_METHOD LexerKotlin::Fold(Sci_PositionU startPos, Sci_Position lengthDoc, int, IDocument* pAccess)
{
    if (!options.fold) { return; }

    PropSetSimple props;
    props.SetMultiple(osKotlin.PropertyNames());

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
        const Sci_PositionU bracePos = CheckBraceOnNextLine(styler, lineCurrent - 1, SCE_KOTLIN_OPERATOR, SCE_KOTLIN_TASKMARKER);
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
        case SCE_KOTLIN_COMMENTBLOCK:
        case SCE_KOTLIN_COMMENTBLOCKDOC: {
                const int level = (ch == '/' && chNext == '*') ? 1 : ((ch == '*' && chNext == '/') ? -1 : 0);
                if (level != 0) {
                    levelNext += level;
                    i++;
                    chNext = styler.SafeGetCharAt(i + 1);
                    styleNext = styler.StyleAt(i + 1);
                }
            } break;

        case SCE_KOTLIN_RAWSTRINGSTART:
            if (style != stylePrev) {
                levelNext++;
            }
            break;

        case SCE_KOTLIN_RAWSTRINGEND:
            if (style != styleNext) {
                levelNext--;
            }
            break;

        case SCE_KOTLIN_OPERATOR:
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
                const Sci_PositionU bracePos = CheckBraceOnNextLine(styler, lineCurrent, SCE_KOTLIN_OPERATOR, SCE_KOTLIN_TASKMARKER);
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

LexerModule lmKotlin(SCLEX_KOTLIN, LexerKotlin::LexerFactoryKotlin, "Kotlin", kotlinWordLists);

// ----------------------------------------------------------------------------

