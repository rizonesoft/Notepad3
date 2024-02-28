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


using namespace Lexilla;
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
    "Java Classes"
    "Classes",
    "Java Interfaces"
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
    SCE_KOTLIN_DEFAULT,             "SCE_KOTLIN_DEFAULT",               "default",              "Descr",
    SCE_KOTLIN_COMMENTLINE,         "SCE_KOTLIN_COMMENTLINE",           "commentline",          "Descr",
    SCE_KOTLIN_COMMENTLINEDOC,      "SCE_KOTLIN_COMMENTLINEDOC",        "commentlinedoc",       "Descr",
    SCE_KOTLIN_COMMENTBLOCK,        "SCE_KOTLIN_COMMENTBLOCK",          "commentblock",         "Descr",
    SCE_KOTLIN_COMMENTBLOCKDOC,     "SCE_KOTLIN_COMMENTBLOCKDOC",       "commentblockdoc",      "Descr",
    SCE_KOTLIN_COMMENTDOCWORD,      "SCE_KOTLIN_COMMENTDOCWORD",        "commentdocword",       "Descr",
    SCE_KOTLIN_TASKMARKER,          "SCE_KOTLIN_TASKMARKER",            "taskmarker",           "Descr",
    SCE_KOTLIN_NUMBER,              "SCE_KOTLIN_NUMBER",                "number",               "Descr",
    SCE_KOTLIN_OPERATOR,            "SCE_KOTLIN_OPERATOR",              "operator",             "Descr",
    SCE_KOTLIN_OPERATOR2,           "SCE_KOTLIN_OPERATOR2",             "operator2",            "Descr",
    SCE_KOTLIN_CHARACTER,           "SCE_KOTLIN_CHARACTER",             "character",            "Descr",
    SCE_KOTLIN_STRING,              "SCE_KOTLIN_STRING",                "string",               "Descr",
    SCE_KOTLIN_RAWSTRING,           "SCE_KOTLIN_RAWSTRING ",            "rawstring",            "Descr",
    SCE_KOTLIN_ESCAPECHAR,          "SCE_KOTLIN_ESCAPECHAR",            "escapechar",           "Descr",
    SCE_KOTLIN_VARIABLE,            "SCE_KOTLIN_VARIABLE",              "variable",             "Descr",
    SCE_KOTLIN_LABEL,               "SCE_KOTLIN_LABEL",                 "label",                "Descr",
    SCE_KOTLIN_IDENTIFIER,          "SCE_KOTLIN_IDENTIFIER",            "identifier",           "Descr",
    SCE_KOTLIN_ANNOTATION,          "SCE_KOTLIN_ANNOTATION",            "annotation",           "Descr",
    SCE_KOTLIN_BACKTICKS,           "SCE_KOTLIN_BACKTICKS",             "backticks",            "Descr",
    SCE_KOTLIN_WORD,                "SCE_KOTLIN_WORD",                  "word",                 "Descr",
    SCE_KOTLIN_CLASS,               "SCE_KOTLIN_CLASS",                 "class",                "Descr",
    SCE_KOTLIN_INTERFACE,           "SCE_KOTLIN_INTERFACE",             "interface",            "Descr",
    SCE_KOTLIN_ENUM,                "SCE_KOTLIN_ENUM",                  "enum",                 "Descr",
    SCE_KOTLIN_FUNCTION,            "SCE_KOTLIN_FUNCTION",              "function",             "Descr",
    SCE_KOTLIN_FUNCTION_DEFINITION, "SCE_KOTLIN_FUNCTION_DEFINITION",   "functiondef",          "Descr",
};

} // end of namespace

class LexerKotlin : public DefaultLexer
{

    //CharacterSet validKey;
    //CharacterSet validKeyWord;
    //CharacterSet validNumberEnd;
    //CharacterSet chDateTime;

    WordList keywordLists[KEYWORDSET_MAX+1];

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

	virtual ~LexerKotlin() = default;

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

enum {
    KeywordIndex_Keyword = 0,
    KeywordIndex_JavaClass = 1,
    KeywordIndex_Class = 2,
    KeywordIndex_JavaInterface = 3,
    KeywordIndex_Interface = 4,
    KeywordIndex_Enumeration = 5,
    KeywordIndex_Annotation = 6,
    KeywordIndex_Function = 7,
    KeywordIndex_KDoc = 8
};

Sci_Position SCI_METHOD LexerKotlin::WordListSet(int n, const char* wl)
{
    WordList* wordListN = &(keywordLists[n]);

    Sci_Position firstModification = -1LL;
    if (wordListN) {
        if (wordListN->Set(wl)) {
            firstModification = 0;
        }
    }
    return firstModification;
}
// ----------------------------------------------------------------------------

struct EscapeSequence {

	int outerState = SCE_KOTLIN_DEFAULT;
    int digitsLeft = 0;
	bool brace = false;

	// highlight any character as escape sequence.
    bool resetEscapeState(int state, int chNext) noexcept {
        if (IsNewline(chNext)) {
            return false;
        }
        outerState = state;
		brace = false;
		digitsLeft = (chNext == 'x') ? 3 : ((chNext == 'u') ? 5 : 1);
        return true;
    }
    bool atEscapeEnd(int ch) noexcept {
        --digitsLeft;
        return digitsLeft <= 0 || !IsHexDigit(ch);
    }
};

enum {
    KotlinLineStateMaskLineComment = 1, // line comment
    KotlinLineStateMaskImport = 1 << 1, // import
};

enum class KeywordType {
    None = SCE_KOTLIN_DEFAULT,
    Annotation = SCE_KOTLIN_ANNOTATION,
    Class = SCE_KOTLIN_CLASS,
    Interface = SCE_KOTLIN_INTERFACE,
    Enum = SCE_KOTLIN_ENUM,
    Label = SCE_KOTLIN_LABEL,
    Return = 0x40,
};

static_assert(DefaultNestedStateBaseStyle + 1 == SCE_KOTLIN_STRING);
static_assert(DefaultNestedStateBaseStyle + 2 == SCE_KOTLIN_RAWSTRING);

constexpr bool IsSpaceEquiv(int state) noexcept {
    return state <= SCE_KOTLIN_TASKMARKER;
}

constexpr bool IsCommentTagPrev(int chPrev) noexcept {
    return chPrev <= 32 || AnyOf(chPrev, '/', '*', '!');
}



void SCI_METHOD LexerKotlin::Lex(Sci_PositionU startPos, Sci_Position lengthDoc, int initStyle, IDocument* pAccess)
{
    PropSetSimple props;
    Accessor styler(pAccess, &props);

    int lineStateLineType = 0;
    int commentLevel = 0;	// nested block comment level

    KeywordType kwType = KeywordType::None;
    int chBeforeIdentifier = 0;

    std::vector<int> nestedState; // string interpolation "${}"

    int visibleChars = 0;
    int chBefore = 0;
    int visibleCharsBefore = 0;
    int chPrevNonWhite = 0;
    EscapeSequence escSeq;

    StyleContext sc(startPos, lengthDoc, initStyle, styler);
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
    } else if (startPos == 0 && sc.Match('#', '!')) {
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

        case SCE_KOTLIN_VARIABLE:
        case SCE_KOTLIN_LABEL:
        case SCE_KOTLIN_IDENTIFIER:
        case SCE_KOTLIN_ANNOTATION:
            if (!IsIdentifierCharEx(sc.ch)) {
                switch (sc.state) {
                case SCE_KOTLIN_VARIABLE:
                    sc.SetState(escSeq.outerState);
                    continue;

        case SCE_KOTLIN_ANNOTATION:
            if (sc.ch == '.' || sc.ch == ':') {
                sc.SetState(SCE_KOTLIN_OPERATOR);
                sc.ForwardSetState(SCE_KOTLIN_ANNOTATION);
                continue;
            }
                    break;

                case SCE_KOTLIN_IDENTIFIER: {
                    char s[128];
                    sc.GetCurrent(s, sizeof(s));
                    if (keywordLists[KeywordIndex_Keyword].InList(s)) {
                        sc.ChangeState(SCE_KOTLIN_WORD);
                        if (StrEqual(s, "import")) {
                            if (visibleChars == sc.LengthCurrent()) {
                                lineStateLineType = KotlinLineStateMaskImport;
                            }
                        } else if (StrEqualsAny(s, "break", "continue", "return", "this", "super")) {
                            kwType = KeywordType::Label;
                        } else if (StrEqualsAny(s, "class", "typealias")) {
                            if (!(kwType == KeywordType::Annotation || kwType == KeywordType::Enum)) {
                                kwType = KeywordType::Class;
                            }
                        } else if (StrEqual(s, "enum")) {
                            kwType = KeywordType::Enum;
                        } else if (StrEqual(s, "annotation")) {
                            kwType = KeywordType::Annotation;
                        } else if (StrEqual(s, "interface")) {
                            kwType = KeywordType::Interface;
                        } else if (StrEqual(s, "return")) {
                            kwType = KeywordType::Return;
                        }
                        if (kwType > KeywordType::None && kwType < KeywordType::Return) {
                            const int chNext = GetDocNextChar(styler, sc);
                            if (!((kwType == KeywordType::Label) ? (chNext == '@') : IsIdentifierStartEx(chNext))) {
                                kwType = KeywordType::None;
                            }
                        }
                    } else if (sc.ch == '@') {
                        sc.ChangeState(SCE_KOTLIN_LABEL);
                        sc.Forward();
                    } else if (keywordLists[KeywordIndex_JavaClass].InList(s) || keywordLists[KeywordIndex_Class].InList(s)) {
                        sc.ChangeState(SCE_KOTLIN_CLASS);
                    } else if (keywordLists[KeywordIndex_JavaInterface].InList(s) || keywordLists[KeywordIndex_Interface].InList(s)) {
                        sc.ChangeState(SCE_KOTLIN_INTERFACE);
                    } else if (keywordLists[KeywordIndex_Enumeration].InList(s)) {
                        sc.ChangeState(SCE_KOTLIN_ENUM);
                    } else if (sc.ch != '.') {
                        if (kwType > KeywordType::None && kwType < KeywordType::Return) {
                            sc.ChangeState(static_cast<int>(kwType));
                        } else {
                            const int chNext = GetDocNextChar(styler, sc, sc.ch == '?');
                            if (chNext == '(') {
                                // type function()
                                // type[] function()
                                // type<type> function()
                                if (kwType != KeywordType::Return && (IsIdentifierCharEx(chBefore) || chBefore == ']')) {
                                    sc.ChangeState(SCE_KOTLIN_FUNCTION_DEFINITION);
                                } else {
                                    sc.ChangeState(SCE_KOTLIN_FUNCTION);
                                }
                            } else if (sc.Match(':', ':')
                                || (chBeforeIdentifier == '<' && (chNext == '>' || chNext == '<'))) {
                                // type::class
                                // type<type>
                                // type<type?>
                                // type<type<type>>
                                // type<type, type>
                                // class type: type, interface {}
                                sc.ChangeState(SCE_KOTLIN_CLASS);
                            }
                        }
                    }
                    if (sc.state != SCE_KOTLIN_WORD && sc.ch != '.') {
                        kwType = KeywordType::None;
                    }
                } break;
                }
                sc.SetState(SCE_KOTLIN_DEFAULT);
            }
            break;

        case SCE_KOTLIN_COMMENTLINE:
        case SCE_KOTLIN_COMMENTLINEDOC:
            if (sc.atLineStart) {
                sc.SetState(SCE_KOTLIN_DEFAULT);
            } else {
                HighlightTaskMarker(sc, visibleChars, visibleCharsBefore, SCE_KOTLIN_TASKMARKER);
            }
            break;

        case SCE_KOTLIN_COMMENTBLOCK:
        case SCE_KOTLIN_COMMENTBLOCKDOC:
            if (sc.state == SCE_KOTLIN_COMMENTBLOCKDOC && sc.ch == '@' && IsLowerCase(sc.chNext) && IsCommentTagPrev(sc.chPrev)) {
                sc.SetState(SCE_KOTLIN_COMMENTDOCWORD);
            } else if (sc.Match('*', '/')) {
                sc.Forward();
                --commentLevel;
                if (commentLevel == 0) {
                    sc.ForwardSetState(SCE_KOTLIN_DEFAULT);
                }
            } else if (sc.Match('/', '*')) {
                sc.Forward();
                ++commentLevel;
            } else if (HighlightTaskMarker(sc, visibleChars, visibleCharsBefore, SCE_KOTLIN_TASKMARKER)) {
                continue;
            }
            break;

        case SCE_KOTLIN_COMMENTDOCWORD:
            if (!IsLowerCase(sc.ch)) {
                sc.SetState(SCE_KOTLIN_COMMENTBLOCKDOC);
                continue;
            }
            break;

        case SCE_KOTLIN_CHARACTER:
        case SCE_KOTLIN_STRING:
        case SCE_KOTLIN_RAWSTRING:
            if (sc.atLineStart && sc.state != SCE_KOTLIN_RAWSTRING) {
                sc.SetState(SCE_KOTLIN_DEFAULT);
            } else if (sc.ch == '\\' && sc.state != SCE_KOTLIN_RAWSTRING) {
                if (escSeq.resetEscapeState(sc.state, sc.chNext)) {
                    sc.SetState(SCE_KOTLIN_ESCAPECHAR);
                    sc.Forward();
                }
            } else if (sc.ch == '\'' && sc.state == SCE_KOTLIN_CHARACTER) {
                sc.ForwardSetState(SCE_KOTLIN_DEFAULT);
            } else if (sc.state != SCE_KOTLIN_CHARACTER) {
                if (sc.ch == '$') {
                if (sc.chNext == '{') {
                    nestedState.push_back(sc.state);
                    sc.SetState(SCE_KOTLIN_OPERATOR2);
                    sc.Forward();
                    } else if (IsIdentifierStartEx(sc.chNext)) {
                    escSeq.outerState = sc.state;
                    sc.SetState(SCE_KOTLIN_VARIABLE);
                }
                } else if (sc.ch == '\"' && (sc.state == SCE_KOTLIN_STRING || sc.MatchNext('"', '"'))) {
                if (sc.state == SCE_KOTLIN_RAWSTRING) {
                    //sc.Advance(2);
                    sc.Forward(2);
                }
                sc.ForwardSetState(SCE_KOTLIN_DEFAULT);
            }
            }
            break;

        case SCE_KOTLIN_ESCAPECHAR:
            if (escSeq.atEscapeEnd(sc.ch)) {
                sc.SetState(escSeq.outerState);
                continue;
            }
            break;

        case SCE_KOTLIN_BACKTICKS:
            if (sc.atLineStart) {
                sc.SetState(SCE_KOTLIN_DEFAULT);
            } else if (sc.ch == '`') {
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
                } else {
                    commentLevel = 1;
                }
                continue;
            }
            if (sc.ch == '\"') {
                if (sc.MatchNext('"', '"')) {
                    sc.SetState(SCE_KOTLIN_RAWSTRING);
                    //sc.Advance(2);
                    sc.Forward(2);
                } else {
                sc.SetState(SCE_KOTLIN_STRING);
            }
            } else if (sc.ch == '\'') {
                sc.SetState(SCE_KOTLIN_CHARACTER);
            } else if (IsNumberStart(sc.ch, sc.chNext)) {
                sc.SetState(SCE_KOTLIN_NUMBER);
            } else if (sc.ch == '@' && IsIdentifierStartEx(sc.chNext)) {
                sc.SetState((kwType == KeywordType::Label) ? SCE_KOTLIN_LABEL : SCE_KOTLIN_ANNOTATION);
                kwType = KeywordType::None;
            } else if (sc.ch == '`') {
                sc.SetState(SCE_KOTLIN_BACKTICKS);
            } else if (IsIdentifierStartEx(sc.ch)) {
                chBefore = chPrevNonWhite;
                if (chPrevNonWhite != '.') {
                    chBeforeIdentifier = chPrevNonWhite;
                }
                sc.SetState(SCE_KOTLIN_IDENTIFIER);
            } else if (IsAGraphic(sc.ch)) {
                sc.SetState(SCE_KOTLIN_OPERATOR);
                if (!nestedState.empty()) {
                    sc.ChangeState(SCE_KOTLIN_OPERATOR2);
                    if (sc.ch == '{') {
                        nestedState.push_back(SCE_KOTLIN_DEFAULT);
                    } else if (sc.ch == '}') {
                        const int outerState = TakeAndPop(nestedState);
                        sc.ForwardSetState(outerState);
                        continue;
                    }
                }
            }
        }

        if (!isspacechar(sc.ch)) {
            visibleChars++;
            if (!IsSpaceEquiv(sc.state)) {
                chPrevNonWhite = sc.ch;
            }
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
            kwType = KeywordType::None;
        }
        sc.Forward();
    }

    sc.Complete();
}

struct FoldLineState {
    int lineComment;
    int packageImport;
    constexpr explicit FoldLineState(int lineState) noexcept :
        lineComment(lineState & KotlinLineStateMaskLineComment),
        packageImport((lineState >> 1) & 1) {
    }
};

constexpr bool IsStreamCommentStyle(int style) noexcept {
    return style == SCE_KOTLIN_COMMENTBLOCK || style == SCE_KOTLIN_COMMENTBLOCKDOC;
}

void SCI_METHOD LexerKotlin::Fold(Sci_PositionU startPos, Sci_Position lengthDoc, int, IDocument* pAccess)
{
    if (!options.fold) { return; }

    PropSetSimple props;
    Accessor styler(pAccess, &props);

    //const int foldComment = styler.GetPropertyInt("fold.comment", 1);
    //const int foldComment = props.GetInt("fold.comment", 1) != 0;
    //const int foldComment = options.foldComment;

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
    lineStartNext = sci::min(lineStartNext, endPos);

    char chNext = styler[startPos];
    int styleNext = styler.StyleAt(startPos);
    int style = styleNext;
    int visibleChars = 0;

    while (startPos < endPos) {
        const char ch = chNext;
        const int stylePrev = style;
        style = styleNext;
        chNext = styler[++startPos];
        styleNext = styler.StyleAt(startPos);

        switch (style) {
        case SCE_KOTLIN_COMMENTBLOCK:
        case SCE_KOTLIN_COMMENTBLOCKDOC: {
                const int level = (ch == '/' && chNext == '*') ? 1 : ((ch == '*' && chNext == '/') ? -1 : 0);
                if (level != 0) {
                    levelNext += level;
                startPos++;
                chNext = styler[startPos];
                styleNext = styler.StyleAt(startPos);
                }
            } break;

        case SCE_KOTLIN_RAWSTRING:
            if (style != stylePrev) {
                levelNext++;
            }
            if (style != styleNext) {
                levelNext--;
            }
            break;

        case SCE_KOTLIN_OPERATOR:
        case SCE_KOTLIN_OPERATOR2:
            if (ch == '{' || ch == '[' || ch == '(') {
                levelNext++;
            } else if (ch == '}' || ch == ']' || ch == ')') {
                levelNext--;
            }
            break;
        }

        if (visibleChars == 0 && !IsSpaceEquiv(style)) {
            ++visibleChars;
        }
        if (startPos == lineStartNext) {
            const FoldLineState foldNext(styler.GetLineState(lineCurrent + 1));
            levelNext = sci::max(levelNext, SC_FOLDLEVELBASE);
            if (foldCurrent.lineComment) {
                levelNext += foldNext.lineComment - foldPrev.lineComment;
            } else if (foldCurrent.packageImport) {
                levelNext += foldNext.packageImport - foldPrev.packageImport;
            } else if (visibleChars) {
                const Sci_PositionU bracePos = CheckBraceOnNextLine(styler, lineCurrent, SCE_KOTLIN_OPERATOR, SCE_KOTLIN_TASKMARKER);
                if (bracePos) {
                    levelNext++;
                    startPos = bracePos + 1; // skip the brace
                    style = SCE_KOTLIN_OPERATOR;
                    chNext = styler[startPos];
                    styleNext = styler.StyleAt(startPos);
                }
            }

            const int levelUse = levelCurrent;
            int lev = levelUse | levelNext << 16;
            if (levelUse < levelNext) {
                lev |= SC_FOLDLEVELHEADERFLAG;
            }
                styler.SetLevel(lineCurrent, lev);

            lineCurrent++;
            lineStartNext = styler.LineStart(lineCurrent + 1);
            lineStartNext = sci::min(lineStartNext, endPos);
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

