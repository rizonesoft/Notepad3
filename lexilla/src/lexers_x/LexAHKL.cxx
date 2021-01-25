// encoding: UTF-8
// Scintilla source code edit control
/** @file LexAHKL.cxx
 ** Lexer AutoHotkey L
 ** Created by Isaias "RaptorX" Baez (graptorx@gmail.com)
 **/
// Copyright ©2013 Isaias "RaptorX" Baez <graptorx@gmail.com> - [GPLv3]
// The License.txt file describes the conditions under which this software may be distributed.
// https://github.com/RaptorX/LexAHKL/
//

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


using namespace Scintilla;

// Options used for LexerRust
struct OptionsAHKL
{
    bool fold;
    bool foldComment;
    bool foldCompact;
    OptionsAHKL() 
        : fold(false)
        , foldComment(true)
        , foldCompact(true)
    { }
};

static const char* const ahklWordLists[] =
{
    "Directives",
    "Commands",
    "Command Parameters",
    "Control Flow",
    "Built-in Functions",
    "Built-in Variables",
    "Keyboard & Mouse Keys",
    "User Defined 1",
    "User Defined 2",
    nullptr
};


struct OptionSetAHKL : public OptionSet<OptionsAHKL>
{
    OptionSetAHKL()
    {
        DefineProperty("fold", &OptionsAHKL::fold);
        DefineProperty("fold.comment", &OptionsAHKL::foldComment);
        DefineProperty("fold.compact", &OptionsAHKL::foldCompact);

        DefineWordListSets(ahklWordLists);
    }
};

class LexerAHKL : public DefaultLexer
{

    OptionsAHKL options;
    OptionSetAHKL osAHKL;

    CharacterSet valLabel;
    CharacterSet valHotkeyMod;
    CharacterSet valIdentifier;
    CharacterSet valHotstringOpt;
    CharacterSet valDocComment;

    WordList directives;
    WordList commands;
    WordList parameters;
    WordList flow;
    WordList functions;
    WordList variables;
    WordList keys;
    WordList user1;
    WordList user2;

    CharacterSet ExpOperator;
    CharacterSet SynOperator;
    CharacterSet EscSequence;

public:
    LexerAHKL() : DefaultLexer("AHKL", SCLEX_AHKL, nullptr, 0),
        //valLabel(CharacterSet::setAlphaNum, "@#$_~!^&*()+[]\';./\\<>?|{}-=\""),
        valLabel(CharacterSet::setAlphaNum, R"(@#$_~!^&*()+[]';./\<>?|{}-=")"),
        valHotkeyMod(CharacterSet::setDigits, "#!^+&<>*~$"),
        valIdentifier(CharacterSet::setAlphaNum, "@#$_"),
        valHotstringOpt(CharacterSet::setDigits, "*?BbCcEeIiKkOoPpRrSsZz"),
        valDocComment(CharacterSet::setNone, "'`\""),
        ExpOperator(CharacterSet::setNone, "+-*/!~&|^<>.:"),
        SynOperator(CharacterSet::setNone, "+-*/!~&|^<>.:()[]?,{}"),
        EscSequence(CharacterSet::setNone, ",%`;nrbtvaf")
    {
    }

    virtual ~LexerAHKL() = default;

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
        return osAHKL.PropertyNames();
    }
    int SCI_METHOD PropertyType(const char* name) override
    {
        return osAHKL.PropertyType(name);
    }
    const char* SCI_METHOD PropertyGet(const char* name) override
    {
        return osAHKL.PropertyGet(name);
    }
    const char* SCI_METHOD DescribeProperty(const char* name) override
    {
        return osAHKL.DescribeProperty(name);
    }
    const char* SCI_METHOD DescribeWordListSets() override
    {
        return osAHKL.DescribeWordListSets();
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

    static ILexer5 *LexerFactoryAHKL()
    {
        return new LexerAHKL();
    }
};

Sci_Position SCI_METHOD LexerAHKL::PropertySet(const char* key, const char* val)
{
    if (osAHKL.PropertySet(&options, key, val))
    {
        return 0;
    }
    return -1;
}


Sci_Position SCI_METHOD LexerAHKL::WordListSet(int n, const char *wl)
{
    WordList *wordListN = nullptr;
    switch (n)
    {

    case 0:
        wordListN = &directives;
        break;

    case 1:
        wordListN = &commands;
        break;

    case 2:
        wordListN = &parameters;
        break;

    case 3:
        wordListN = &flow;
        break;

    case 4:
        wordListN = &functions;
        break;

    case 5:
        wordListN = &variables;
        break;

    case 6:
        wordListN = &keys;
        break;

    case 7:
        wordListN = &user1;
        break;

    case 8:
        wordListN = &user2;
        break;

    }

    int firstModification = -1;
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


void SCI_METHOD LexerAHKL::Lex(Sci_PositionU startPos, Sci_Position lengthDoc, int initStyle, IDocument *pAccess)
{
    PropSetSimple props;
    props.SetMultiple(osAHKL.PropertyNames());

    Accessor styler(pAccess, &props);
    StyleContext sc(startPos, lengthDoc, initStyle, styler);

    // non-lexical states
    int expLevel = 0;
    int mainState = SCE_AHKL_NEUTRAL;								// Used on some special cases like objects where SCE_AHKL_NEUTRAL is set but
    // we basically come from SCE_AHKL_OBJECT and we need to be aware of it
    bool OnlySpaces = false;
    bool validLabel = false;
    bool validFunction = false;

    bool inKey = false;
    bool inString = false;
    bool inCommand = false;
    bool inHotstring = false;
    bool inExpString = false;
    bool inDocComment = false;
    bool inExpression = false;

    bool inStringBlk = (sc.state == SCE_AHKL_STRINGOPTS || sc.state == SCE_AHKL_STRINGBLOCK || sc.state == SCE_AHKL_STRINGCOMMENT);
    bool inCommentBlk = (sc.state == SCE_AHKL_COMMENTDOC || sc.state == SCE_AHKL_COMMENTBLOCK);

    for (; sc.More(); sc.Forward())
    {
        // AutoHotkey usually resets lexical state in a per line base except in Comment and String Blocks
        if (sc.atLineStart)
        {
            expLevel = 0;
            mainState = SCE_AHKL_NEUTRAL;

            OnlySpaces = true, validLabel = true, validFunction = true, inKey = false, inString = false;
            inDocComment = false, inCommand = false, inHotstring = false, inExpression = false, inExpString = false;

            if (!inStringBlk && !inCommentBlk)
                sc.SetState(SCE_AHKL_NEUTRAL);
        }

        // Comments are allowed almost everywhere
        if (!inStringBlk && !inCommentBlk && (OnlySpaces || isspacechar(sc.chPrev)) && sc.ch == ';')
            sc.SetState(SCE_AHKL_COMMENTLINE);

        // Exit Current State
        switch (sc.state)
        {

        case SCE_AHKL_IDENTIFIER:
            {
                if (sc.atLineEnd || !valIdentifier.Contains(sc.ch))  			// Check for match after typing whole words or punctuation signs
                {

                    char identifier[256];
                    sc.GetCurrentLowered(identifier, sizeof(identifier));

                    if (!sc.Match('('))
                        validFunction = false;

                    if (directives.InList(identifier))
                    {
                        sc.ChangeState(SCE_AHKL_DIRECTIVE);
                    }
                    else if (!inExpression && !inCommand && !inKey && sc.ch != '(' && commands.InList(identifier))
                    {
                        inCommand = true;
                        sc.ChangeState(SCE_AHKL_COMMAND);
                    }
                    else if (inCommand && parameters.InList(identifier))
                    {
                        sc.ChangeState(SCE_AHKL_PARAM);
                    }
                    else if (!inKey && flow.InList(identifier))  			// avoid conflicts with key identifiers (e.g. pause)
                    {
                        sc.ChangeState(SCE_AHKL_CONTROLFLOW);
                    }
                    else if (sc.ch == '(' && functions.InList(identifier))
                    {
                        inCommand = true;
                        sc.ChangeState(SCE_AHKL_BUILTINFUNCTION);
                    }
                    else if (variables.InList(identifier))
                    {
                        sc.ChangeState(SCE_AHKL_BUILTINVAR);
                    }
                    else if (inKey && keys.InList(identifier))
                    {
                        sc.ChangeState(SCE_AHKL_KEY);
                    }
                    else if (user1.InList(identifier))
                    {
                        sc.ChangeState(SCE_AHKL_USERDEFINED1);
                    }
                    else if (user2.InList(identifier))
                    {
                        sc.ChangeState(SCE_AHKL_USERDEFINED2);
                    }
                    else if (sc.ch == '{')
                    {
                        inKey = true;
                        inCommand = false;
                    }
                    else if (sc.ch == '}')
                    {
                        if (inKey) {
                            inKey = false;
                            inCommand = true;
                        }
                    }
                    else if (inExpression && !(sc.ch == '(' || sc.ch == '[' || sc.ch == '.'))  	// Dont lex as a variable if it is a function or an array
                    {
                        sc.ChangeState(SCE_AHKL_VAR);
                    }
                    else if (!inExpression && !ExpOperator.Contains(sc.chPrev) && sc.ch == '=')
                    {
                        inString = true;
                        sc.ForwardSetState(SCE_AHKL_STRING);
                        continue;
                    }
                    else if (!inCommand && ExpOperator.Contains(sc.ch) && sc.chNext == '=')
                    {
                        sc.SetState(SCE_AHKL_ERROR);
                    }
                    else if (validFunction && sc.ch == '(')
                    {
                        sc.ChangeState(SCE_AHKL_USERFUNCTION);
                        sc.SetState(SCE_AHKL_NEUTRAL);
                    }
                    else if (valLabel.Contains(sc.ch) && !(sc.ch == '(' || sc.ch == '[' || sc.ch == '.'))
                    {
                        continue;
                    }
                    else if (!inCommand && (sc.ch == '[' || sc.ch == '.'))
                    {
                        if (sc.ch == '.' && valIdentifier.Contains(sc.chNext))
                        {
                            mainState = SCE_AHKL_OBJECT;
                            sc.ChangeState(SCE_AHKL_OBJECT);
                            sc.SetState(SCE_AHKL_NEUTRAL);
                        }
                        else if (sc.ch == '.' && !valIdentifier.Contains(sc.chNext))
                        {
                            sc.ChangeState(SCE_AHKL_ERROR);
                            sc.SetState(SCE_AHKL_ERROR);
                            break;
                        }
                        else if (sc.ch == '[')
                        {
                            sc.ChangeState(SCE_AHKL_OBJECT);
                            sc.SetState(SCE_AHKL_NEUTRAL);
                        }

                    }
                    else if (mainState == SCE_AHKL_OBJECT)  			// Special object case
                    {
                        mainState = SCE_AHKL_NEUTRAL;

                        if (sc.ch == '(')
                        {
                            sc.ChangeState(SCE_AHKL_USERFUNCTION);
                            sc.SetState(SCE_AHKL_NEUTRAL);
                        }
                        else
                        {
                            sc.ChangeState(SCE_AHKL_VAR);
                            sc.SetState(SCE_AHKL_NEUTRAL);
                        }
                    }
                    else if (!validLabel && sc.ch == ':')
                    {
                        sc.ChangeState(SCE_AHKL_IDENTIFIER);
                    }
                    else if (sc.ch == ':' && sc.chNext != ':' && sc.chNext != '/' && sc.chNext != '\\')
                    {
                        sc.ChangeState(SCE_AHKL_LABEL);
                        if ((sc.chNext != '\r') && (sc.chNext != '\n'))
                            sc.ForwardSetState(SCE_AHKL_ERROR);
                        break;
                    }
                    else if (!inHotstring && sc.ch == ':' && sc.chNext == ':')
                    {
                        sc.ChangeState(SCE_AHKL_HOTKEY);
                        sc.Forward(2);
                        sc.SetState(SCE_AHKL_NEUTRAL);
                        break;
                    }
                    validLabel = false;
                    sc.SetState(SCE_AHKL_NEUTRAL);

                }
                else if ((sc.chPrev == 'x' || sc.chPrev == 'y'|| sc.chPrev == 'w'|| sc.chPrev == 'h')
                         && inCommand && isdigit(sc.ch & 0xFF) )  				// Special number cases when entering sizes
                {
                    sc.SetState(SCE_AHKL_DECNUMBER);
                }
                break;
            }

        case SCE_AHKL_COMMENTDOC:
            {
                if (OnlySpaces && sc.Match('*','/'))
                {
                    inCommentBlk = false;
                    sc.Forward(2);
                    sc.SetState(SCE_AHKL_NEUTRAL);
                }
                else if ((OnlySpaces || isspacechar(sc.chPrev)) &&
                         ((sc.ch == '@' && isalnum(sc.chNext & 0xFF)) || valDocComment.Contains(sc.ch)))
                {
                    if (valDocComment.Contains(sc.ch))
                        inDocComment = true;

                    sc.SetState(SCE_AHKL_COMMENTKEYWORD);
                }
                break;
            }

        case SCE_AHKL_COMMENTKEYWORD:
            {
                if (sc.atLineStart || (!inDocComment && sc.ch == ':')
                        || (inDocComment && valDocComment.Contains(sc.ch)))
                    sc.ForwardSetState(SCE_AHKL_COMMENTDOC);
                break;
            }

        case SCE_AHKL_COMMENTBLOCK:
            {
                if (OnlySpaces && sc.Match('*','/'))
                {
                    inCommentBlk = false;
                    sc.Forward(2);
                    sc.SetState(SCE_AHKL_NEUTRAL);
                }
                break;
            }

        case SCE_AHKL_HEXNUMBER:
            {
                if (isspacechar(sc.ch) || SynOperator.Contains(sc.ch))
                    sc.SetState(SCE_AHKL_NEUTRAL);
                else if (!isxdigit(sc.ch & 0xFF))
                    sc.ChangeState(SCE_AHKL_IDENTIFIER);
                break;
            }

        case SCE_AHKL_DECNUMBER:
            {
                if (!isdigit(sc.ch & 0xFF))
                {
                    if (sc.ch == 'x' || sc.ch == 'X')
                        sc.ChangeState(SCE_AHKL_HEXNUMBER);
                    else if (isalpha(sc.ch))
                        sc.ChangeState(SCE_AHKL_IDENTIFIER);
                    else
                        sc.SetState(SCE_AHKL_NEUTRAL);
                }
                break;
            }

        case SCE_AHKL_STRING:
            {
                if (inExpression && sc.atLineEnd)
                {
                    sc.ChangeState(SCE_AHKL_ERROR);
                }
                else if (inExpression && sc.ch == '"')
                {
                    if (sc.chNext == '"')  						// In expression string, double quotes are doubled to escape them so skip it
                    {
                        sc.Forward();
                    }
                    else
                    {
                        inExpString = false;
                        sc.ForwardSetState(SCE_AHKL_NEUTRAL);
                    }
                }
                else if (!inExpression && sc.ch == '%' && valIdentifier.Contains(sc.chNext))
                {
                    sc.SetState(SCE_AHKL_NEUTRAL);
                    sc.ForwardSetState(SCE_AHKL_VAR);
                }
                break;
            }

        case SCE_AHKL_STRINGOPTS:
            {
                if (sc.atLineStart)
                    sc.SetState(SCE_AHKL_STRINGBLOCK);
                break;
            }

        case SCE_AHKL_STRINGBLOCK:
            {
                if (OnlySpaces && sc.ch == ')')
                {
                    inStringBlk = false;
                    sc.SetState(SCE_AHKL_NEUTRAL);

                    if (sc.chNext != ',')
                        sc.ForwardSetState(SCE_AHKL_ERROR);
                    else
                        inCommand = true;
                }
                // if ((OnlySpaces || IsASpace(sc.chPrev)) && sc.Match(';')) {
                // sc.SetState(SCE_AHKL_STRINGCOMMENT);
                // }
                break;
            }

        case SCE_AHKL_ESCAPESEQ:
            {
                sc.ForwardSetState(SCE_AHKL_NEUTRAL);
                break;
            }

        case SCE_AHKL_VAR:
            {
                if (!valIdentifier.Contains(sc.ch) && sc.ch != '%')
                {
                    sc.ChangeState(SCE_AHKL_ERROR);
                }
                else if (sc.ch == '%')
                {
                    sc.SetState(SCE_AHKL_NEUTRAL);
                    sc.ForwardSetState(SCE_AHKL_NEUTRAL);
                    if (inString )
                        sc.ForwardSetState(SCE_AHKL_STRING);
                }
                break;
            }

        case SCE_AHKL_VARREF:
            {
                if (!valIdentifier.Contains(sc.ch) && sc.ch != '%')
                {
                    sc.ChangeState(SCE_AHKL_ERROR);
                }
                else if (sc.ch == '%')
                {
                    sc.SetState(SCE_AHKL_NEUTRAL);
                    continue;
                }
                break;
            }

        case SCE_AHKL_HOTSTRINGOPT:
            {
                if (sc.ch == ':')
                {
                    sc.ForwardSetState(SCE_AHKL_HOTSTRING);
                }
                else if (!valHotstringOpt.Contains(sc.ch))
                {
                    sc.SetState(SCE_AHKL_ERROR);
                }
                break;
            }

        case SCE_AHKL_HOTSTRING:
            {
                if (sc.Match(':',':'))
                {
                    sc.SetState(SCE_AHKL_HOTSTRINGOPT);
                    sc.Forward(2);
                    sc.SetState(SCE_AHKL_STRING);
                }
                break;
            }

        case SCE_AHKL_ERROR:
            {
                if (inExpression && inExpString && sc.ch == '"')
                {

                    sc.ChangeState(SCE_AHKL_STRING);

                }
                else if (sc.ch == '%')
                {

                    sc.SetState(SCE_AHKL_NEUTRAL);

                }
                else if (inHotstring && valHotstringOpt.Contains(sc.ch))
                {

                    sc.SetState(SCE_AHKL_HOTSTRINGOPT);

                }
                else if (inHotstring && sc.ch == ':')
                {
                    sc.SetState(SCE_AHKL_HOTSTRINGOPT);
                    sc.ForwardSetState(SCE_AHKL_HOTSTRING);
                }
                break;
            }

        }

        // Enter New State
        if (sc.state == SCE_AHKL_NEUTRAL)
        {
            // Handle Expressions
            if ((OnlySpaces && sc.ch == '.')  || sc.Match(" % ") || (sc.ch == '(')
                    || ((valIdentifier.Contains(sc.chPrev) || isspacechar(sc.chPrev)) && sc.ch == '?')
                    || ((valIdentifier.Contains(sc.chPrev) || isspacechar(sc.chPrev)) && ExpOperator.Contains(sc.ch) && sc.chNext == '=')
                    ||  (valIdentifier.Contains(sc.chPrev) && sc.ch == '['))
            {
                expLevel += 1;
                inExpression = true;

                if (sc.Match(" % "))
                {
                    inCommand = false;
                }
            }
            else if (sc.ch == ']' || sc.ch == ')')
            {
                expLevel -= 1, inCommand = false;

                if (expLevel == 0)
                {
                    inExpression = false;
                }
            }

            // Handle Command continuation section
            if ((OnlySpaces && sc.ch == ',') || (OnlySpaces && sc.Match(')', ',')))
                inCommand = true;

            if ((!sc.atLineEnd && valIdentifier.Contains(sc.ch))
                    ||  (!inExpression && valHotkeyMod.Contains(sc.ch) && sc.chNext != '='))
            {
                if (valIdentifier.Contains(sc.ch))
                    validFunction = true;

                if (IsADigit(sc.ch))
                    sc.SetState(SCE_AHKL_DECNUMBER);
                else if (inCommand && sc.ch == '+')
                    continue;
                else
                    sc.SetState(SCE_AHKL_IDENTIFIER);

            }
            else if (OnlySpaces && sc.Match('/', '*'))
            {
                inCommentBlk = true;
                sc.ChangeState(SCE_AHKL_COMMENTBLOCK);

                if (sc.Match("/**") && !sc.Match("/***"))
                    sc.ChangeState(SCE_AHKL_COMMENTDOC);
            }
            else if (sc.ch == ']')
            {
                mainState = SCE_AHKL_NEUTRAL;						// Reset object state
            }
            else if (sc.ch == ')')
            {
                // inCommand = false;
            }
            else if (sc.ch == '{')
            {
                inKey = true;
                inCommand = false;
            }
            else if (sc.ch == '}')
            {
                if (inKey) {
                    inKey = false;
                    inCommand = true;
                }
            }
            else if (sc.ch == '`' && EscSequence.Contains(sc.chNext))
            {
                sc.SetState(SCE_AHKL_ESCAPESEQ);
            }
            else if (inExpression && sc.ch == '"')
            {
                inExpString = true;
                inString = false;
                sc.SetState(SCE_AHKL_STRING);
            }
            else if (!inExpression && !ExpOperator.Contains(sc.chPrev) && sc.ch == '=')
            {
                inString = true;
                sc.ForwardSetState(SCE_AHKL_STRING);
            }
            else if (OnlySpaces && sc.ch == '(')
            {
                inStringBlk = true;
                sc.ForwardSetState(SCE_AHKL_STRINGOPTS);
            }
            else if (!inExpression && sc.ch == '%' && valIdentifier.Contains(sc.chNext))
            {
                sc.ForwardSetState(SCE_AHKL_VAR);
            }
            else if (inExpression && sc.ch == '%' && valIdentifier.Contains(sc.chNext))
            {
                sc.ForwardSetState(SCE_AHKL_VARREF);
            }
            else if (OnlySpaces && sc.ch == ':')
            {
                inHotstring = true;
                sc.SetState(SCE_AHKL_HOTSTRINGOPT);
            }
        }
        if (!IsASpace((sc.ch)))
        {
            OnlySpaces = false;
        }
    }
    sc.Complete();
}


void SCI_METHOD LexerAHKL::Fold(Sci_PositionU startPos, Sci_Position lengthDoc, int initStyle, IDocument *pAccess)
{
    if (!options.fold) { return; }

    PropSetSimple props;
    props.SetMultiple(osAHKL.PropertyNames());

    Accessor styler(pAccess, &props);

    bool const foldComment = options.foldComment; //props.GetInt("fold.comment") != 0;
    bool const foldCompact = options.foldCompact; //props.GetInt("fold.compact", 1) != 0;

    Sci_PositionU endPos = startPos + lengthDoc;
    bool bOnlySpaces = true;
    int lineCurrent = styler.GetLine(startPos);
    int levelCurrent = SC_FOLDLEVELBASE;
    if (lineCurrent > 0)
    {
        levelCurrent = styler.LevelAt(lineCurrent - 1) & SC_FOLDLEVELNUMBERMASK;
    }
    int levelNext = levelCurrent;
    char chNext = styler[startPos];
    int styleNext = styler.StyleAt(startPos);
    int style = initStyle;
    for (Sci_PositionU i = startPos; i < endPos; i++)
    {
        char ch = chNext;
        chNext = styler.SafeGetCharAt(i + 1);
        int stylePrev = style;
        style = styleNext;
        styleNext = styler.StyleAt(i + 1);
        bool atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');
        if (foldComment && style == SCE_AHKL_COMMENTBLOCK)
        {
            if (stylePrev != SCE_AHKL_COMMENTBLOCK)
            {
                levelNext++;
            }
            else if ((styleNext != SCE_AHKL_COMMENTBLOCK) && !atEOL)
            {
                // Comments don't end at end of line and the next character may be unstyled.
                levelNext--;
            }
        }
        if (ch == '(' || ch == '{')
        {
            levelNext++;
        }
        else if (ch == ')' || ch == '}')
        {
            levelNext--;
        }
        if (atEOL)
        {
            int level = levelCurrent;
            if (bOnlySpaces && foldCompact)
            {
                // Empty line
                level |= SC_FOLDLEVELWHITEFLAG;
            }
            if (!bOnlySpaces && levelNext > levelCurrent)
            {
                level |= SC_FOLDLEVELHEADERFLAG;
            }
            if (level != styler.LevelAt(lineCurrent))
            {
                styler.SetLevel(lineCurrent, level);
            }
            lineCurrent++;
            levelCurrent = levelNext;
            bOnlySpaces = true;
        }
        if (!isspacechar(ch))
        {
            bOnlySpaces = false;
        }
    }

}

LexerModule lmAHKL(SCLEX_AHKL, LexerAHKL::LexerFactoryAHKL, "AHKL", ahklWordLists);
