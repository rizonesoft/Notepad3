// encoding: UTF-8
// Scintilla source code edit control
/** @file LexAHK.cxx
 ** Lexer for AutoHotkey, simplified version
 ** Written by Philippe Lhoste (PhiLho)
 **/
 // Copyright 1998-2006 by Neil Hodgson <neilh@scintilla.org>
 // The License.txt file describes the conditions under which this software may be distributed.
 /* notepad2-mod custom code for the AutoHotkey lexer */


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


// Options used for LexerRust
struct OptionsAHK
{
    bool fold;
    bool foldComment;
    bool foldCompact;
    OptionsAHK() 
        : fold(false)
        , foldComment(true)
        , foldCompact(true)
    { }
};

static const char* const ahkWordLists[] =
{
    "Flow of Control",
    "Commands",
    "Functions",
    "Directives",
    "Keys & Buttons",
    "Variables",
    "Special Parameters (keywords)",
    "User Defined",
    nullptr
};



struct OptionSetAHK : public OptionSet<OptionsAHK>
{
    OptionSetAHK()
    {
        DefineProperty("fold", &OptionsAHK::fold);
        DefineProperty("fold.comment", &OptionsAHK::foldComment);
        DefineProperty("fold.compact", &OptionsAHK::foldCompact);

        DefineWordListSets(ahkWordLists);
    }
};

class LexerAHK : public DefaultLexer
{

    OptionsAHK options;
    OptionSetAHK osAHK;

    WordList controlFlow;
    WordList commands;
    WordList functions;
    WordList directives;
    WordList keysButtons;
    WordList variables;
    WordList specialParams;
    WordList userDefined;

    CharacterSet WordChar;
    CharacterSet ExpOperator;

public:
    LexerAHK() : DefaultLexer("AHK", SCLEX_AHK, nullptr, 0),
        //valLabel(CharacterSet::setAlphaNum, "@#$_~!^&*()+[]\';./\\<>?|{}-=\""),
        WordChar(CharacterSet::setAlphaNum, "@#$_[]?"),
        ExpOperator(CharacterSet::setNone, "+-*/!~&|^=<>.()")
    {
    }

    virtual ~LexerAHK() = default;

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
        return osAHK.PropertyNames();
    }
    int SCI_METHOD PropertyType(const char* name) override
    {
        return osAHK.PropertyType(name);
    }
    const char* SCI_METHOD PropertyGet(const char* name) override
    {
        return osAHK.PropertyGet(name);
    }
    const char* SCI_METHOD DescribeProperty(const char* name) override
    {
        return osAHK.DescribeProperty(name);
    }
    const char* SCI_METHOD DescribeWordListSets() override
    {
        return osAHK.DescribeWordListSets();
    }
    void * SCI_METHOD PrivateCall(int, void *) override
    {
        return nullptr;
    }

    // --------------------------------------------------------------------------
    Sci_Position SCI_METHOD PropertySet(const char* key, const char* val) override;
    Sci_Position SCI_METHOD WordListSet(int n, const char *wl) override;
    void SCI_METHOD Lex(Sci_PositionU startPos, Sci_Position lengthDoc, int initStyle, IDocument *pAccess) override;
    void SCI_METHOD Fold(Sci_PositionU startPos, Sci_Position lengthDoc, int initStyle, IDocument *pAccess) override;

    static ILexer5 *LexerFactoryAHK()
    {
        return new LexerAHK();
    }

private:
    void HighlightKeyword(char currentWord[], StyleContext& sc);

};

Sci_Position SCI_METHOD LexerAHK::PropertySet(const char* key, const char* val)
{
    if (osAHK.PropertySet(&options, key, val))
    {
        return 0;
    }
    return -1;
}


Sci_Position SCI_METHOD LexerAHK::WordListSet(int n, const char *wl)
{
    WordList *wordListN = nullptr;
    switch (n)
    {

    case 0:
        wordListN = &controlFlow;
        break;

    case 1:
        wordListN = &commands;
        break;

    case 2:
        wordListN = &functions;
        break;

    case 3:
        wordListN = &directives;
        break;

    case 4:
        wordListN = &keysButtons;
        break;

    case 5:
        wordListN = &variables;
        break;

    case 6:
        wordListN = &specialParams;
        break;

    case 7:
        wordListN = &userDefined;
        break;

    }

    int firstModification = -1;
    if (wordListN) {
        if (wordListN->Set(wl)) {
            firstModification = 0;
        }
    }
    return firstModification;
}


void LexerAHK::HighlightKeyword(char currentWord[], StyleContext& sc) {

    if (controlFlow.InList(currentWord)) {
        sc.ChangeState(SCE_AHK_WORD_CF);
    }
    else if (commands.InList(currentWord)) {
        sc.ChangeState(SCE_AHK_WORD_CMD);
    }
    else if (functions.InList(currentWord)) {
        sc.ChangeState(SCE_AHK_WORD_FN);
    }
    else if (currentWord[0] == '#' && directives.InList(currentWord + 1)) {
        sc.ChangeState(SCE_AHK_WORD_DIR);
    }
    else if (keysButtons.InList(currentWord)) {
        sc.ChangeState(SCE_AHK_WORD_KB);
    }
    else if (variables.InList(currentWord)) {
        sc.ChangeState(SCE_AHK_WORD_VAR);
    }
    else if (specialParams.InList(currentWord)) {
        sc.ChangeState(SCE_AHK_WORD_SP);
    }
    else if (userDefined.InList(currentWord)) {
        sc.ChangeState(SCE_AHK_WORD_UD);
    }
    else {
        sc.ChangeState(SCE_AHK_DEFAULT);
    }
}



void SCI_METHOD LexerAHK::Lex(Sci_PositionU startPos, Sci_Position lengthDoc, int initStyle, IDocument *pAccess)
{
    PropSetSimple props;
    Accessor styler(pAccess, &props);
    StyleContext sc(startPos, lengthDoc, initStyle, styler);

    // Do not leak onto next line
    if (initStyle != SCE_AHK_COMMENTBLOCK &&
        initStyle != SCE_AHK_STRING) {
        initStyle = SCE_AHK_DEFAULT;
    }
    int currentState = initStyle;
    int nextState = -1;
    char currentWord[256];

    /* The AutoHotkey syntax is heavily context-dependent.
    For example, for each command, the lexer knows if parameter #n
    is a string, a variable, a number, an expression, etc.
    I won't go this far, but I will try to handle most regular cases.
    */
    // True if in a continuation section
    bool bContinuationSection = (initStyle == SCE_AHK_STRING);
    // Indicate if the lexer has seen only spaces since the start of the line
    bool bOnlySpaces = (!bContinuationSection);
    // Indicate if since the start of the line, lexer met only legal label chars
    bool bIsLabel = false;
    // Distinguish hotkeys from hotstring
    bool bIsHotkey = false;
    bool bIsHotstring = false;
    // In an expression
    bool bInExpression = false;
    // A quoted string in an expression (share state with continuation section string)
    bool bInExprString = false;
    // To accept A-F chars in a number
    bool bInHexNumber = false;


    for (; sc.More(); sc.Forward()) {
        if (nextState >= 0) {
            // I need to reset a state before checking new char
            sc.SetState(nextState);
            nextState = -1;
        }
        if (sc.state == SCE_AHK_SYNOPERATOR) {
            // Only one char (if two detected, we move Forward() anyway)
            sc.SetState(SCE_AHK_DEFAULT);
        }
        if (sc.atLineEnd && (bIsHotkey || bIsHotstring)) {
            // I make the hotkeys and hotstrings more visible
            // by changing the line end to LABEL style (if style uses eolfilled)
            bIsHotkey = bIsHotstring = false;
            sc.SetState(SCE_AHK_LABEL);
        }
        if (sc.atLineStart) {
            if (sc.state != SCE_AHK_COMMENTBLOCK &&
                !bContinuationSection) {
                // Prevent some styles from leaking back to previous line
                sc.SetState(SCE_AHK_DEFAULT);
            }
            bOnlySpaces = true;
            bIsLabel = false;
            bInExpression = false;	// I don't manage multiline expressions yet!
            bInHexNumber = false;
        }

        // Manage cases occuring in (almost) all states (not in comments)
        if (sc.state != SCE_AHK_COMMENTLINE &&
            sc.state != SCE_AHK_COMMENTBLOCK &&
            !IsASpace(sc.ch)) {
            if (sc.ch == '`') {
                // Backtick, escape sequence
                currentState = sc.state;
                sc.SetState(SCE_AHK_ESCAPE);
                sc.Forward();
                nextState = currentState;
                continue;
            }
            if (sc.ch == '%' && !bIsHotstring && !bInExprString &&
                sc.state != SCE_AHK_VARREF &&
                sc.state != SCE_AHK_VARREFKW &&
                sc.state != SCE_AHK_ERROR) {
                if (IsASpace(sc.chNext)) {
                    if (sc.state == SCE_AHK_STRING) {
                        // Illegal unquoted character!
                        sc.SetState(SCE_AHK_ERROR);
                    }
                    else {
                        // % followed by a space is expression start
                        bInExpression = true;
                    }
                }
                else {
                    // Variable reference
                    currentState = sc.state;
                    sc.SetState(SCE_AHK_SYNOPERATOR);
                    nextState = SCE_AHK_VARREF;
                    continue;
                }
            }
            if (sc.state != SCE_AHK_STRING && !bInExpression) {
                // Management of labels, hotkeys, hotstrings and remapping

                // Check if the starting string is a label candidate
                if (bOnlySpaces &&
                    sc.ch != ',' && sc.ch != ';' && sc.ch != ':' &&
                    sc.ch != '%' && sc.ch != '`') {
                    // A label cannot start with one of the above chars
                    bIsLabel = true;
                }

                // The current state can be IDENTIFIER or DEFAULT,
                // depending if the label starts with a word char or not
                if (bIsLabel && sc.ch == ':' &&
                    (IsASpace(sc.chNext) || sc.atLineEnd)) {
                    // ?l/a|b\e^l!:
                    // Only ; comment should be allowed after
                    sc.ChangeState(SCE_AHK_LABEL);
                    sc.SetState(SCE_AHK_SYNOPERATOR);
                    nextState = SCE_AHK_DEFAULT;
                    continue;
                }
                else if (sc.Match(':', ':')) {
                    if (bOnlySpaces) {
                        // Hotstring ::aa::Foo
                        bIsHotstring = true;
                        sc.SetState(SCE_AHK_SYNOPERATOR);
                        sc.Forward();
                        nextState = SCE_AHK_LABEL;
                        continue;
                    }
                    // Hotkey F2:: or remapping a::b
                    bIsHotkey = true;
                    // Check if it is a known key
                    sc.GetCurrent(currentWord, sizeof(currentWord));
                    if (keysButtons.InList(currentWord)) {
                        sc.ChangeState(SCE_AHK_WORD_KB);
                    }
                    sc.SetState(SCE_AHK_SYNOPERATOR);
                    sc.Forward();
                    if (bIsHotstring) {
                        nextState = SCE_AHK_STRING;
                    }
                    continue;
                }
            }
        }
        // Check if the current string is still a label candidate
        // Labels are much more permissive than regular identifiers...
        if (bIsLabel &&
            (sc.ch == ',' || sc.ch == '%' || sc.ch == '`' || IsASpace(sc.ch))) {
            // Illegal character in a label
            bIsLabel = false;
        }

        // Determine if the current state should terminate.
        if (sc.state == SCE_AHK_COMMENTLINE) {
            if (sc.atLineEnd) {
                sc.SetState(SCE_AHK_DEFAULT);
            }
        }
        else if (sc.state == SCE_AHK_COMMENTBLOCK) {
            if (bOnlySpaces && sc.Match('*', '/')) {
                // End of comment at start of line (skipping white space)
                sc.Forward();
                sc.ForwardSetState(SCE_AHK_DEFAULT);
            }
        }
        else if (sc.state == SCE_AHK_EXPOPERATOR) {
            if (!ExpOperator.Contains(sc.ch)) {
                sc.SetState(SCE_AHK_DEFAULT);
            }
        }
        else if (sc.state == SCE_AHK_STRING) {
            if (bContinuationSection) {
                if (bOnlySpaces && sc.ch == ')') {
                    // End of continuation section
                    bContinuationSection = false;
                    sc.SetState(SCE_AHK_SYNOPERATOR);
                }
            }
            else if (bInExprString) {
                if (sc.ch == '\"') {
                    if (sc.chNext == '\"') {
                        // In expression string, double quotes are doubled to escape them
                        sc.Forward();	// Skip it
                    }
                    else {
                        bInExprString = false;
                        sc.ForwardSetState(SCE_AHK_DEFAULT);
                    }
                }
                else if (sc.atLineEnd) {
                    sc.ChangeState(SCE_AHK_ERROR);
                }
            }
            else {
                if (sc.ch == ';' && IsASpace(sc.chPrev)) {
                    // Line comments after code must be preceded by a space
                    sc.SetState(SCE_AHK_COMMENTLINE);
                }
            }
        }
        else if (sc.state == SCE_AHK_NUMBER) {
            if (bInHexNumber) {
                if (!IsADigit(sc.ch, 16)) {
                    bInHexNumber = false;
                    sc.SetState(SCE_AHK_DEFAULT);
                }
            }
            else if (!(IsADigit(sc.ch) || sc.ch == '.')) {
                sc.SetState(SCE_AHK_DEFAULT);
            }
        }
        else if (sc.state == SCE_AHK_IDENTIFIER) {
            if (!WordChar.Contains(sc.ch)) {
                sc.GetCurrent(currentWord, sizeof(currentWord));
                HighlightKeyword(currentWord, sc);
                if (strcmp(currentWord, "if") == 0) {
                    bInExpression = true;
                }
                sc.SetState(SCE_AHK_DEFAULT);
            }
        }
        else if (sc.state == SCE_AHK_VARREF) {
            if (sc.ch == '%') {
                // End of variable reference
                sc.GetCurrent(currentWord, sizeof(currentWord));
                if (variables.InList(currentWord)) {
                    sc.ChangeState(SCE_AHK_VARREFKW);
                }
                sc.SetState(SCE_AHK_SYNOPERATOR);
                nextState = currentState;
                continue;
            }
            else if (!WordChar.Contains(sc.ch)) {
                // Oops! Probably no terminating %
                sc.ChangeState(SCE_AHK_ERROR);
            }
        }
        else if (sc.state == SCE_AHK_LABEL) {
            // Hotstring -- modifier or trigger string :*:aa::Foo or ::aa::Foo
            if (sc.ch == ':') {
                sc.SetState(SCE_AHK_SYNOPERATOR);
                if (sc.chNext == ':') {
                    sc.Forward();
                }
                nextState = SCE_AHK_LABEL;
                continue;
            }
        }

        // Determine if a new state should be entered
        if (sc.state == SCE_AHK_DEFAULT) {
            if (sc.ch == ';' &&
                (bOnlySpaces || IsASpace(sc.chPrev))) {
                // Line comments are alone on the line or are preceded by a space
                sc.SetState(SCE_AHK_COMMENTLINE);
            }
            else if (bOnlySpaces && sc.Match('/', '*')) {
                // Comment at start of line (skipping white space)
                sc.SetState(SCE_AHK_COMMENTBLOCK);
                sc.Forward();
            }
            else if (sc.ch == '{' || sc.ch == '}') {
                // Code block or special key {Enter}
                sc.SetState(SCE_AHK_SYNOPERATOR);
            }
            else if (bOnlySpaces && sc.ch == '(') {
                // Continuation section
                bContinuationSection = true;
                sc.SetState(SCE_AHK_SYNOPERATOR);
                nextState = SCE_AHK_STRING;	// !!! Can be an expression!
            }
            else if (sc.Match(':', '=') ||
                sc.Match('+', '=') ||
                sc.Match('-', '=') ||
                sc.Match('/', '=') ||
                sc.Match('*', '=')) {
                // Expression assignment
                bInExpression = true;
                sc.SetState(SCE_AHK_SYNOPERATOR);
                sc.Forward();
                nextState = SCE_AHK_DEFAULT;
            }
            else if (ExpOperator.Contains(sc.ch)) {
                sc.SetState(SCE_AHK_EXPOPERATOR);
            }
            else if (sc.ch == '\"') {
                bInExprString = true;
                sc.SetState(SCE_AHK_STRING);
            }
            else if (sc.ch == '0' && (sc.chNext == 'x' || sc.chNext == 'X')) {
                // Hexa, skip forward as we don't accept any other alpha char (beside A-F) inside
                bInHexNumber = true;
                sc.SetState(SCE_AHK_NUMBER);
                sc.Forward(2);
            }
            else if (IsADigit(sc.ch) || (sc.ch == '.' && IsADigit(sc.chNext))) {
                sc.SetState(SCE_AHK_NUMBER);
            }
            else if (WordChar.Contains(sc.ch)) {
                sc.SetState(SCE_AHK_IDENTIFIER);
            }
            else if (sc.ch == ',') {
                sc.SetState(SCE_AHK_SYNOPERATOR);
                nextState = SCE_AHK_DEFAULT;
            }
            else if (sc.ch == ':') {
                if (bOnlySpaces) {
                    // Start of hotstring :*:foo::Stuff or ::btw::Stuff
                    bIsHotstring = true;
                    sc.SetState(SCE_AHK_SYNOPERATOR);
                    if (sc.chNext == ':') {
                        sc.Forward();
                    }
                    nextState = SCE_AHK_LABEL;
                }
            }
            else if (WordChar.Contains(sc.ch)) {
                sc.SetState(SCE_AHK_IDENTIFIER);
            }
        }
        if (!IsASpace(sc.ch)) {
            bOnlySpaces = false;
        }
    }
    // End of file: complete any pending changeState
    if (sc.state == SCE_AHK_IDENTIFIER) {
        sc.GetCurrent(currentWord, sizeof(currentWord));
        HighlightKeyword(currentWord, sc);
    }
    else if (sc.state == SCE_AHK_STRING && bInExprString) {
        sc.ChangeState(SCE_AHK_ERROR);
    }
    else if (sc.state == SCE_AHK_VARREF) {
        sc.ChangeState(SCE_AHK_ERROR);
    }
    sc.Complete();

}


void SCI_METHOD LexerAHK::Fold(Sci_PositionU startPos, Sci_Position lengthDoc, int initStyle, IDocument *pAccess)
{
    if (!options.fold) { return; }

    PropSetSimple props;
    Accessor styler(pAccess, &props);

    bool const foldComment = options.foldComment; //props.GetInt("fold.comment") != 0;
    bool const foldCompact = options.foldCompact; //props.GetInt("fold.compact", 1) != 0;

    Sci_PositionU endPos = startPos + lengthDoc;

    bool bOnlySpaces = true;
    int lineCurrent = styler.GetLine(startPos);
    int levelCurrent = SC_FOLDLEVELBASE;
    if (lineCurrent > 0) {
        levelCurrent = styler.LevelAt(lineCurrent - 1) & SC_FOLDLEVELNUMBERMASK;
    }
    int levelNext = levelCurrent;
    char chNext = styler[startPos];
    int styleNext = styler.StyleAt(startPos);
    int style = initStyle;
    for (Sci_PositionU i = startPos; i < endPos; i++) {
        char ch = chNext;
        chNext = styler.SafeGetCharAt(i + 1);
        int stylePrev = style;
        style = styleNext;
        styleNext = styler.StyleAt(i + 1);
        bool atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');
        if (foldComment && style == SCE_AHK_COMMENTBLOCK) {
            if (stylePrev != SCE_AHK_COMMENTBLOCK) {
                levelNext++;
            }
            else if ((styleNext != SCE_AHK_COMMENTBLOCK) && !atEOL) {
                // Comments don't end at end of line and the next character may be unstyled.
                levelNext--;
            }
        }
        if (style == SCE_AHK_SYNOPERATOR) {
            if (ch == '(' || ch == '{') {
                levelNext++;
            }
            else if (ch == ')' || ch == '}') {
                levelNext--;
            }
        }
        if (atEOL) {
            int level = levelCurrent;
            if (bOnlySpaces && foldCompact) {
                // Empty line
                level |= SC_FOLDLEVELWHITEFLAG;
            }
            if (!bOnlySpaces && levelNext > levelCurrent) {
                level |= SC_FOLDLEVELHEADERFLAG;
            }
            if (level != styler.LevelAt(lineCurrent)) {
                styler.SetLevel(lineCurrent, level);
            }
            lineCurrent++;
            levelCurrent = levelNext;
            bOnlySpaces = true;
        }
        if (!isspacechar(ch)) {
            bOnlySpaces = false;
        }
    }

}

extern const LexerModule lmAHK(SCLEX_AHK, LexerAHK::LexerFactoryAHK, "ahk", ahkWordLists);

