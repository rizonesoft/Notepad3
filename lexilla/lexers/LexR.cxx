// Scintilla source code edit control
/** @file LexR.cxx
 ** Lexer for R, S, SPlus Statistics Program (Heavily derived from CPP Lexer).
 **
 **/
// Copyright 1998-2002 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <cassert>

#include <cstring>

#include <string>
#include <string_view>
#include <vector>
#include <map>

#include "ILexer.h"
#include "Scintilla.h"
#include "SciLexer.h"

#include "WordList.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "CharacterSet.h"
#include "CharacterCategory.h"
#include "LexerModule.h"
#include "OptionSet.h"
#include "SubStyles.h"
#include "DefaultLexer.h"

using namespace Scintilla;
using namespace Lexilla;

// Use an unnamed namespace to protect the functions and classes from name conflicts
namespace {

const char * const RWordLists[] = {
	"Language Keywords",
	"Functions from Base-Priority R Packages",
	"Functions from Recommended-Priority R Packages",
	nullptr,
};

const char styleSubable[] = {SCE_R_IDENTIFIER, 0};

const LexicalClass lexicalClasses[] = {
	// Lexer R SCLEX_R SCE_R_:
	0, "SCE_R_DEFAULT", "default", "White space",
	1, "SCE_R_COMMENT", "comment line", "Comment",
	2, "SCE_R_KWORD", "keyword", "Keywords",
	3, "SCE_R_BASEKWORD", "identifier", "Keywords 2",
	4, "SCE_R_OTHERKWORD", "identifier", "Keywords 3",
	5, "SCE_R_NUMBER", "literal numeric", "Number",
	6, "SCE_R_STRING", "literal string", "Double quoted String",
	7, "SCE_R_STRING2", "literal string", "Single quoted String",
	8, "SCE_R_OPERATOR", "operator", "Operator",
	9, "SCE_R_IDENTIFIER", "identifier", "Identifier",
	10, "SCE_R_INFIX", "operator", "Infix operator",
	11, "SCE_R_INFIXEOL", "error operator", "End of line where infix operator is not closed",
	12, "SCE_R_BACKTICKS", "backtick", "Backticks",
	13, "SCE_R_RAWSTRING", "literal string raw", "Double quoted raw string",
	14, "SCE_R_RAWSTRING2", "literal string raw", "Single quoted raw string",
	15, "SCE_R_ESCAPESEQUENCE", "literal string escapesequence", "Escape sequence",
};

// Options used for LexerR
struct OptionsR {
	bool fold = false;
	bool foldCompact = true;
	bool foldAtElse = false;
	bool unicodeIdentifiers = true;
	bool escapeSequence = false;
};

struct OptionSetR : public OptionSet<OptionsR> {
	OptionSetR() {
		DefineProperty("fold", &OptionsR::fold);

		DefineProperty("fold.compact", &OptionsR::foldCompact);

		DefineProperty("fold.at.else", &OptionsR::foldAtElse,
			"This option enables R folding on a \"} else {\" line of an if statement.");

		DefineProperty("lexer.r.unicode.identifiers", &OptionsR::unicodeIdentifiers,
			"Set to 0 to not recognise R Unicode identifiers.");

		DefineProperty("lexer.r.escape.sequence", &OptionsR::escapeSequence,
			"Set to 1 to enable highlighting of escape sequences in strings.");

		DefineWordListSets(RWordLists);
	}
};

class LexerR : public DefaultLexer {
	WordList keywords;
	WordList keywords2;
	WordList keywords3;
	OptionsR options;
	OptionSetR osR;
	SubStyles subStyles{styleSubable};
public:
	explicit LexerR() : DefaultLexer("r", SCLEX_R, lexicalClasses, std::size(lexicalClasses)) {
	}
	~LexerR() override = default;
	void SCI_METHOD Release() override {
		delete this;
	}
	int SCI_METHOD Version() const override {
		return lvRelease5;
	}
	const char *SCI_METHOD PropertyNames() override {
		return osR.PropertyNames();
	}
	int SCI_METHOD PropertyType(const char *name) override {
		return osR.PropertyType(name);
	}
	const char *SCI_METHOD DescribeProperty(const char *name) override {
		return osR.DescribeProperty(name);
	}
	Sci_Position SCI_METHOD PropertySet(const char *key, const char *val) override;
	const char *SCI_METHOD PropertyGet(const char *key) override {
		return osR.PropertyGet(key);
	}
	const char *SCI_METHOD DescribeWordListSets() override {
		return osR.DescribeWordListSets();
	}
	Sci_Position SCI_METHOD WordListSet(int n, const char *wl) override;
	void SCI_METHOD Lex(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument *pAccess) override;
	void SCI_METHOD Fold(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument *pAccess) override;

	void *SCI_METHOD PrivateCall(int, void *) override {
		return nullptr;
	}

	int SCI_METHOD LineEndTypesSupported() override {
		return SC_LINE_END_TYPE_DEFAULT;
	}

	int SCI_METHOD AllocateSubStyles(int styleBase, int numberStyles) override {
		return subStyles.Allocate(styleBase, numberStyles);
	}
	int SCI_METHOD SubStylesStart(int styleBase) override {
		return subStyles.Start(styleBase);
	}
	int SCI_METHOD SubStylesLength(int styleBase) override {
		return subStyles.Length(styleBase);
	}
	int SCI_METHOD StyleFromSubStyle(int subStyle) override {
		const int styleBase = subStyles.BaseStyle(subStyle);
		return styleBase;
	}
	int SCI_METHOD PrimaryStyleFromStyle(int style) override {
		return style;
	}
	void SCI_METHOD FreeSubStyles() override {
		subStyles.Free();
	}
	void SCI_METHOD SetIdentifiers(int style, const char *identifiers) override {
		subStyles.SetIdentifiers(style, identifiers);
	}
	int SCI_METHOD DistanceToSecondaryStyles() override {
		return 0;
	}
	const char *SCI_METHOD GetSubStyleBases() override {
		return styleSubable;
	}

	static ILexer5 *LexerFactoryR() {
		return new LexerR();
	}
};

Sci_Position SCI_METHOD LexerR::PropertySet(const char *key, const char *val) {
	if (osR.PropertySet(&options, key, val)) {
		return 0;
	}
	return -1;
}

Sci_Position SCI_METHOD LexerR::WordListSet(int n, const char *wl) {
	WordList *wordListN = nullptr;
	switch (n) {
	case 0:
		wordListN = &keywords;
		break;
	case 1:
		wordListN = &keywords2;
		break;
	case 2:
		wordListN = &keywords3;
		break;
	default:
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

// https://cran.r-project.org/doc/manuals/r-devel/R-lang.html#Identifiers-1
constexpr bool IsAWordChar(int ch, bool unicodeIdentifiers) noexcept {
	if (IsASCII(ch))
		return (IsAlphaNumeric(ch) || ch == '.' || ch == '_');

	if (!unicodeIdentifiers)
		return false;

	// R uses the XID_Continue set from Unicode data
	return IsXidContinue(ch);
}

constexpr bool IsAWordStart(int ch, bool unicodeIdentifiers) noexcept {
	if (IsASCII(ch))
		return IsUpperOrLowerCase(ch) || ch == '.';

	if (!unicodeIdentifiers)
		return false;

	// R uses the XID_Start set from Unicode data
	return IsXidStart(ch);
}

constexpr bool IsAnOperator(int ch) noexcept {
	// '.' left out as it is used to make up numbers and identifiers
	if (ch == '-' || ch == '+' || ch == '!' || ch == '~' ||
		ch == '?' || ch == ':' || ch == '*' || ch == '/' ||
		ch == '^' || ch == '<' || ch == '>' || ch == '=' ||
		ch == '&' || ch == '|' || ch == '$' || ch == '(' ||
		ch == ')' || ch == '}' || ch == '{' || ch == '[' ||
		ch == ']' || ch == '_' || ch == '@')
		return true;
	return false;
}

constexpr bool IsOctalOrHex(int ch, bool hex) noexcept {
	return IsAnOctalDigit(ch) || (hex && IsAHeXDigit(ch));
}

// https://search.r-project.org/R/refmans/base/html/Quotes.html
struct EscapeSequence {
	int outerState = SCE_R_DEFAULT;
	int digitsLeft = 0;
	bool hex = false;
	bool brace = false;

	// highlight any character as escape sequence, unrecognized escape sequence is syntax error.
	void resetEscapeState(int state, int chNext) noexcept {
		outerState = state;
		digitsLeft = 1;
		hex = true;
		brace = false;
		if (chNext == 'x') {
			digitsLeft = 3;
		} else if (chNext == 'u') {
			digitsLeft = 5;
		} else if (chNext == 'U') {
			digitsLeft = 9;
		} else if (IsAnOctalDigit(chNext)) {
			digitsLeft = 3;
			hex = false;
		}
	}
	bool atEscapeEnd(int ch) noexcept {
		--digitsLeft;
		return digitsLeft <= 0 || !IsOctalOrHex(ch, hex);
	}
};

int CheckRawString(LexAccessor &styler, Sci_Position pos, int &dashCount) {
	dashCount = 0;
	while (true) {
		const char ch = styler.SafeGetCharAt(pos++);
		switch (ch) {
		case '-':
			++dashCount;
			break;
		case '(':
			return ')';
		case '[':
			return ']';
		case '{':
			return '}';
		default:
			dashCount = 0;
			return 0;
		}
	}
}

constexpr bool IsADotDotDot(const char *idnt, size_t len) noexcept {
	if ((len == 3) && (idnt[0] == '.') && (idnt[1] == '.') && (idnt[2] == '.'))
		return true;

	if ((len > 2) && (idnt[0] == '.') && (idnt[1] == '.')) {
		for (size_t i = 2; i < len; ++i)
			if ((idnt[i] < '0') || (idnt[i] > '9'))
				return false;
		return true;
	}

	return false;
}

void SCI_METHOD LexerR::Lex(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument *pAccess) {
	Accessor styler(pAccess, nullptr);

	// state for raw string
	int matchingDelimiter = 0;
	int dashCount = 0;

	EscapeSequence escapeSeq;

	const WordClassifier &classifierIdentifiers = subStyles.Classifier(SCE_R_IDENTIFIER);

	StyleContext sc(startPos, length, initStyle, styler);
	if (sc.currentLine > 0) {
		const int lineState = styler.GetLineState(sc.currentLine - 1);
		matchingDelimiter = lineState & 0xff;
		dashCount = lineState >> 8;
	}

	while (sc.More()) {
		// Determine if the current state should terminate.
		switch (sc.state) {
		case SCE_R_OPERATOR:
			sc.SetState(SCE_R_DEFAULT);
			break;

		case SCE_R_NUMBER:
			// https://cran.r-project.org/doc/manuals/r-release/R-lang.html#Literal-constants
			if (AnyOf(sc.ch, 'e', 'E', 'p', 'P') && (IsADigit(sc.chNext) || sc.chNext == '+' || sc.chNext == '-')) {
				sc.Forward(); // exponent part
			} else if (!(IsAHeXDigit(sc.ch) || (sc.ch == '.' && IsAHeXDigit(sc.chNext)))) {
				if (AnyOf(sc.ch, 'L', 'i')) {
					sc.Forward(); // integer and complex qualifier
				}
				sc.SetState(SCE_R_DEFAULT);
			}
			break;

		case SCE_R_IDENTIFIER:
			if (!IsAWordChar(sc.ch, options.unicodeIdentifiers)) {
				char identifier[100];
				sc.GetCurrent(identifier, sizeof(identifier));

				// if identifier is a dot-dot-dot object (... ..1 ..2 ..3 etc.) or
				// identifier is part of the keywords list, style as SCE_R_KWORD
				if(IsADotDotDot(identifier, strlen(identifier)) || keywords.InList(identifier)) {
					sc.ChangeState(SCE_R_KWORD);
				} else if (keywords2.InList(identifier)) {
					sc.ChangeState(SCE_R_BASEKWORD);
				} else if (keywords3.InList(identifier)) {
					sc.ChangeState(SCE_R_OTHERKWORD);
				} else {
					if (const int subStyle = classifierIdentifiers.ValueFor(identifier); subStyle >= 0) {
						sc.ChangeState(subStyle);
					}
				}
				sc.SetState(SCE_R_DEFAULT);
			}
			break;

		case SCE_R_COMMENT:
			if (sc.MatchLineEnd()) {
				sc.SetState(SCE_R_DEFAULT);
			}
			break;

		case SCE_R_STRING:
		case SCE_R_STRING2:
		case SCE_R_BACKTICKS:
			if (sc.ch == '\\') {
				if (options.escapeSequence) {
					escapeSeq.resetEscapeState(sc.state, sc.chNext);
					sc.SetState(SCE_R_ESCAPESEQUENCE);
					sc.Forward();
					if (sc.chNext == '{' && AnyOf(sc.ch, 'u', 'U')) {
						escapeSeq.brace = true;
						sc.Forward();
					} else if (sc.MatchLineEnd()) {
						// don't highlight line ending as escape sequence:
						// escapeSeq.outerState is lost when editing on next line.
						sc.SetState(escapeSeq.outerState);
					}
				} else {
					sc.Forward(); // Skip all characters after the backslash
				}
			} else if ((sc.state == SCE_R_STRING && sc.ch == '\"')
				|| (sc.state == SCE_R_STRING2 && sc.ch == '\'')
				|| (sc.state == SCE_R_BACKTICKS && sc.ch == '`')) {
				sc.ForwardSetState(SCE_R_DEFAULT);
			}
			break;

		case SCE_R_ESCAPESEQUENCE:
			if (escapeSeq.atEscapeEnd(sc.ch)) {
				if (escapeSeq.brace && sc.ch == '}') {
					sc.Forward();
				}
				sc.SetState(escapeSeq.outerState);
				continue;
			}
			break;

		case SCE_R_RAWSTRING:
		case SCE_R_RAWSTRING2:
			while (sc.ch == matchingDelimiter) {
				sc.Forward();
				int count = dashCount;
				while (count != 0 && sc.ch == '-') {
					--count;
					sc.Forward();
				}
				if (count == 0 && sc.ch == ((sc.state == SCE_R_RAWSTRING) ? '\"' : '\'')) {
					matchingDelimiter = 0;
					dashCount = 0;
					sc.ForwardSetState(SCE_R_DEFAULT);
					break;
				}
			}
			break;

		case SCE_R_INFIX:
			if (sc.ch == '%') {
				sc.ForwardSetState(SCE_R_DEFAULT);
			} else if (sc.atLineEnd) {
				sc.ChangeState(SCE_R_INFIXEOL);
			}
			break;

		case SCE_R_INFIXEOL:
			if (sc.atLineStart) {
				sc.SetState(SCE_R_DEFAULT);
			}
			break;
		}

		// Determine if a new state should be entered.
		if (sc.state == SCE_R_DEFAULT) {
			if (IsADigit(sc.ch) || (sc.ch == '.' && IsADigit(sc.chNext))) {
				sc.SetState(SCE_R_NUMBER);
				if (sc.ch == '0' && AnyOf(sc.chNext, 'x', 'X')) {
					sc.Forward();
				}
			} else if (AnyOf(sc.ch, 'r', 'R') && AnyOf(sc.chNext, '\"', '\'')) {
				const int chNext = sc.chNext;
				matchingDelimiter = CheckRawString(styler, sc.currentPos + 2, dashCount);
				if (matchingDelimiter) {
					sc.SetState((chNext == '\"') ? SCE_R_RAWSTRING : SCE_R_RAWSTRING2);
					sc.Forward(dashCount + 2);
				} else {
					// syntax error
					sc.SetState(SCE_R_IDENTIFIER);
					sc.ForwardSetState((chNext == '\"') ? SCE_R_STRING : SCE_R_STRING2);
				}
			} else if (IsAWordStart(sc.ch, options.unicodeIdentifiers) ) {
				sc.SetState(SCE_R_IDENTIFIER);
			} else if (sc.Match('#')) {
				sc.SetState(SCE_R_COMMENT);
			} else if (sc.ch == '\"') {
				sc.SetState(SCE_R_STRING);
			} else if (sc.ch == '%') {
				sc.SetState(SCE_R_INFIX);
			} else if (sc.ch == '\'') {
				sc.SetState(SCE_R_STRING2);
			} else if (sc.ch == '`') {
				sc.SetState(SCE_R_BACKTICKS);
			} else if (IsAnOperator(sc.ch)) {
				sc.SetState(SCE_R_OPERATOR);
			}
		}

		if (sc.atLineEnd) {
			const int lineState = matchingDelimiter | (dashCount << 8);
			styler.SetLineState(sc.currentLine, lineState);
		}
		sc.Forward();
	}
	sc.Complete();
}

// Store both the current line's fold level and the next lines in the
// level store to make it easy to pick up with each increment
// and to make it possible to fiddle the current level for "} else {".
void SCI_METHOD LexerR::Fold(Sci_PositionU startPos, Sci_Position length, int /*initStyle - unused*/, IDocument *pAccess) {
	if (!options.fold)
		return;

	Accessor styler(pAccess, nullptr);

	const Sci_PositionU endPos = startPos + length;

	int visibleChars = 0;

	Sci_Position lineCurrent = styler.GetLine(startPos);
	int levelCurrent = SC_FOLDLEVELBASE;
	if (lineCurrent > 0)
		levelCurrent = styler.LevelAt(lineCurrent-1) >> 16;
	int levelMinCurrent = levelCurrent;
	int levelNext = levelCurrent;
	char chNext = styler[startPos];
	int styleNext = styler.StyleAt(startPos);

	for (Sci_PositionU i = startPos; i < endPos; i++) {
		const char ch = chNext;
		chNext = styler.SafeGetCharAt(i + 1);
		const int style = styleNext;
		styleNext = styler.StyleAt(i + 1);
		const bool atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');
		if (style == SCE_R_OPERATOR) {
			if (ch == '{') {
				// Measure the minimum before a '{' to allow
				// folding on "} else {"
				if (levelMinCurrent > levelNext) {
					levelMinCurrent = levelNext;
				}
				levelNext++;
			} else if (ch == '}') {
				levelNext--;
			}
		}
		if (atEOL) {
			int levelUse = levelCurrent;
			if (options.foldAtElse) {
				levelUse = levelMinCurrent;
			}
			int lev = levelUse | levelNext << 16;
			if (visibleChars == 0 && options.foldCompact)
				lev |= SC_FOLDLEVELWHITEFLAG;
			if (levelUse < levelNext)
				lev |= SC_FOLDLEVELHEADERFLAG;
			if (lev != styler.LevelAt(lineCurrent)) {
				styler.SetLevel(lineCurrent, lev);
			}
			lineCurrent++;
			levelCurrent = levelNext;
			levelMinCurrent = levelCurrent;
			visibleChars = 0;
		}
		if (!isspacechar(ch))
			visibleChars++;
	}
}

}

extern const LexerModule lmR(SCLEX_R, LexerR::LexerFactoryR, "r", RWordLists);
