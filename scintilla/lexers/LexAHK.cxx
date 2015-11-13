// Scintilla source code edit control
/** @file LexAHK.cxx
 ** Lexer for AutoHotkey, simplified version
 ** Written by Philippe Lhoste (PhiLho)
 **/
// Copyright 1998-2006 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.
/* notepad2-mod custom code for the AutoHotkey lexer */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>

#include "ILexer.h"
#include "Scintilla.h"
#include "SciLexer.h"

#include "WordList.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "CharacterSet.h"
#include "LexerModule.h"


#ifdef _MSC_VER
#pragma warning(disable: 4100)
#endif

static inline bool IsAWordChar(const int ch) {
	return ch >= 0x80 || isalnum(ch) ||
			ch == '_' || ch == '$' || ch == '[' || ch == ']' ||
			ch == '#' || ch == '@' || ch == '?';
}

// Expression operator
// ( ) + - * ** / // ! ~ ^ & << >> . < > <= >= = == != <> && ||
static inline bool IsExpOperator(const int ch) {
	if (ch >= 0x80 || isalnum(ch))	// Fast exit
		return false;
	return ch == '+' || ch == '-' || ch == '*' || ch == '/' ||
			ch == '(' || ch == ')' || ch == '.' ||
			ch == '=' || ch == '<' || ch == '>' ||
			ch == '&' || ch == '|' || ch == '^' || ch == '~' || ch == '!';
}

static void HighlightKeyword(
	char currentWord[],
	StyleContext &sc,
	WordList *keywordlists[],
	Accessor &styler) {

	WordList &controlFlow = *keywordlists[0];
	WordList &commands = *keywordlists[1];
	WordList &functions = *keywordlists[2];
	WordList &directives = *keywordlists[3];
	WordList &keysButtons = *keywordlists[4];
	WordList &variables = *keywordlists[5];
	WordList &specialParams = *keywordlists[6];
	WordList &userDefined = *keywordlists[7];

	if (controlFlow.InList(currentWord)) {
		sc.ChangeState(SCE_AHK_WORD_CF);
	} else if (commands.InList(currentWord)) {
		sc.ChangeState(SCE_AHK_WORD_CMD);
	} else if (functions.InList(currentWord)) {
		sc.ChangeState(SCE_AHK_WORD_FN);
	}  else if (currentWord[0] == '#' && directives.InList(currentWord + 1)) {
		sc.ChangeState(SCE_AHK_WORD_DIR);
	} else if (keysButtons.InList(currentWord)) {
		sc.ChangeState(SCE_AHK_WORD_KB);
	} else if (variables.InList(currentWord)) {
		sc.ChangeState(SCE_AHK_WORD_VAR);
	} else if (specialParams.InList(currentWord)) {
		sc.ChangeState(SCE_AHK_WORD_SP);
	} else if (userDefined.InList(currentWord)) {
		sc.ChangeState(SCE_AHK_WORD_UD);
	} else {
		sc.ChangeState(SCE_AHK_DEFAULT);
	}
}

static void ColouriseAHKDoc(
	Sci_PositionU startPos,
	Sci_Position length,
	int initStyle,
	WordList *keywordlists[],
	Accessor &styler) {

	WordList &keysButtons = *keywordlists[4];
	WordList &variables = *keywordlists[5];
	char currentWord[256];

	// Do not leak onto next line
	if (initStyle != SCE_AHK_COMMENTBLOCK &&
			initStyle != SCE_AHK_STRING) {
		initStyle = SCE_AHK_DEFAULT;
	}
	int currentState = initStyle;
	int nextState = -1;

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

	StyleContext sc(startPos, length, initStyle, styler);

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
					} else {
						// % followed by a space is expression start
						bInExpression = true;
					}
				} else {
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
				} else if (sc.Match(':', ':')) {
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
					sc.GetCurrentLowered(currentWord, sizeof(currentWord));
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
		} else if (sc.state == SCE_AHK_COMMENTBLOCK) {
			if (bOnlySpaces && sc.Match('*', '/')) {
				// End of comment at start of line (skipping white space)
				sc.Forward();
				sc.ForwardSetState(SCE_C_DEFAULT);
			}
		} else if (sc.state == SCE_AHK_EXPOPERATOR) {
			if (!IsExpOperator(sc.ch)) {
				sc.SetState(SCE_AHK_DEFAULT);
			}
		} else if (sc.state == SCE_AHK_STRING) {
			if (bContinuationSection) {
				if (bOnlySpaces && sc.ch == ')') {
					// End of continuation section
					bContinuationSection = false;
					sc.SetState(SCE_AHK_SYNOPERATOR);
				}
			} else if (bInExprString) {
				if (sc.ch == '\"') {
					if (sc.chNext == '\"') {
						// In expression string, double quotes are doubled to escape them
						sc.Forward();	// Skip it
					} else {
						bInExprString = false;
						sc.ForwardSetState(SCE_AHK_DEFAULT);
					}
				} else if (sc.atLineEnd) {
					sc.ChangeState(SCE_AHK_ERROR);
				}
			} else {
				if (sc.ch == ';' && IsASpace(sc.chPrev)) {
					// Line comments after code must be preceded by a space
					sc.SetState(SCE_AHK_COMMENTLINE);
				}
			}
		} else if (sc.state == SCE_AHK_NUMBER) {
			if (bInHexNumber) {
				if (!IsADigit(sc.ch, 16)) {
					bInHexNumber = false;
					sc.SetState(SCE_AHK_DEFAULT);
				}
			} else if (!(IsADigit(sc.ch) || sc.ch == '.')) {
				sc.SetState(SCE_AHK_DEFAULT);
			}
		} else if (sc.state == SCE_AHK_IDENTIFIER) {
			if (!IsAWordChar(sc.ch)) {
				sc.GetCurrentLowered(currentWord, sizeof(currentWord));
				HighlightKeyword(currentWord, sc, keywordlists, styler);
				if (strcmp(currentWord, "if") == 0) {
					bInExpression = true;
				}
				sc.SetState(SCE_AHK_DEFAULT);
			}
		} else if (sc.state == SCE_AHK_VARREF) {
			if (sc.ch == '%') {
				// End of variable reference
				sc.GetCurrentLowered(currentWord, sizeof(currentWord));
				if (variables.InList(currentWord)) {
					sc.ChangeState(SCE_AHK_VARREFKW);
				}
				sc.SetState(SCE_AHK_SYNOPERATOR);
				nextState = currentState;
				continue;
			} else if (!IsAWordChar(sc.ch)) {
				// Oops! Probably no terminating %
				sc.ChangeState(SCE_AHK_ERROR);
			}
		} else if (sc.state == SCE_AHK_LABEL) {
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
			} else if (bOnlySpaces && sc.Match('/', '*')) {
				// Comment at start of line (skipping white space)
				sc.SetState(SCE_AHK_COMMENTBLOCK);
				sc.Forward();
			} else if (sc.ch == '{' || sc.ch == '}') {
				// Code block or special key {Enter}
				sc.SetState(SCE_AHK_SYNOPERATOR);
			} else if (bOnlySpaces && sc.ch == '(') {
				// Continuation section
				bContinuationSection = true;
				sc.SetState(SCE_AHK_SYNOPERATOR);
				nextState = SCE_AHK_STRING;	// !!! Can be an expression!
			} else if (sc.Match(':', '=') ||
					sc.Match('+', '=') ||
					sc.Match('-', '=') ||
					sc.Match('/', '=') ||
					sc.Match('*', '=')) {
				// Expression assignment
				bInExpression = true;
				sc.SetState(SCE_AHK_SYNOPERATOR);
				sc.Forward();
				nextState = SCE_AHK_DEFAULT;
			} else if (IsExpOperator(sc.ch)) {
				sc.SetState(SCE_AHK_EXPOPERATOR);
			} else if (sc.ch == '\"') {
				bInExprString = true;
				sc.SetState(SCE_AHK_STRING);
			} else if (sc.ch == '0' && (sc.chNext == 'x' || sc.chNext == 'X')) {
				// Hexa, skip forward as we don't accept any other alpha char (beside A-F) inside
				bInHexNumber = true;
				sc.SetState(SCE_AHK_NUMBER);
				sc.Forward(2);
			} else if (isdigit(sc.ch) || (sc.ch == '.' && isdigit(sc.chNext))) {
				sc.SetState(SCE_AHK_NUMBER);
			} else if (IsAWordChar(sc.ch)) {
				sc.SetState(SCE_AHK_IDENTIFIER);
			} else if (sc.ch == ',') {
				sc.SetState(SCE_AHK_SYNOPERATOR);
				nextState = SCE_AHK_DEFAULT;
			} else if (sc.ch == ':') {
				if (bOnlySpaces) {
					// Start of hotstring :*:foo::Stuff or ::btw::Stuff
					bIsHotstring = true;
					sc.SetState(SCE_AHK_SYNOPERATOR);
					if (sc.chNext == ':') {
						sc.Forward();
					}
					nextState = SCE_AHK_LABEL;
				}
			} else if (IsAWordChar(sc.ch)) {
				sc.SetState(SCE_AHK_IDENTIFIER);
			}
		}
		if (!IsASpace(sc.ch)) {
			bOnlySpaces = false;
		}
	}
	// End of file: complete any pending changeState
	if (sc.state == SCE_AHK_IDENTIFIER) {
		sc.GetCurrentLowered(currentWord, sizeof(currentWord));
		HighlightKeyword(currentWord, sc, keywordlists, styler);
	} else if (sc.state == SCE_AHK_STRING && bInExprString) {
		sc.ChangeState(SCE_AHK_ERROR);
	} else if (sc.state == SCE_AHK_VARREF) {
		sc.ChangeState(SCE_AHK_ERROR);
	}
	sc.Complete();
}

static void FoldAHKDoc(Sci_PositionU startPos, Sci_Position length, int initStyle,
                            WordList *[], Accessor &styler) {
	bool foldComment = styler.GetPropertyInt("fold.comment") != 0;
	bool foldCompact = styler.GetPropertyInt("fold.compact", 1) != 0;
	Sci_PositionU endPos = startPos + length;
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
			} else if ((styleNext != SCE_AHK_COMMENTBLOCK) && !atEOL) {
				// Comments don't end at end of line and the next character may be unstyled.
				levelNext--;
			}
		}
		if (style == SCE_AHK_SYNOPERATOR) {
			if (ch == '(' || ch == '{') {
				levelNext++;
			} else if (ch == ')' || ch == '}') {
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

static const char * const ahkWordListDesc[] = {
	"Flow of control",
	"Commands",
	"Functions",
	"Directives",
	"Keys & buttons",
	"Variables",
	"Special Parameters (keywords)",
	"User defined",
	0
};

LexerModule lmAHK(SCLEX_AHK, ColouriseAHKDoc, "ahk", FoldAHKDoc, ahkWordListDesc);
