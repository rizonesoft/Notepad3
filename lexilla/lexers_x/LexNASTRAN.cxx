// Scintilla source code edit control
/** @file LexNASTRAN.cxx
 ** NASTRAN executive/case control and small, large and free-field bulk data.
 ** The License.txt file describes the conditions under which this software may be distributed.
 **/

#include <algorithm>
#include <cassert>
#include <map>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include "ILexer.h"
#include "Scintilla.h"
#include "PropSetSimple.h"
#include "WordList.h"
#include "LexAccessor.h"
#include "LexerModule.h"
#include "LexerBase.h"
#include "SciXLexer.h"

using namespace Scintilla;
using namespace Lexilla;

namespace {

constexpr bool Space(char ch) noexcept { return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n'; }
constexpr bool Digit(char ch) noexcept { return ch >= '0' && ch <= '9'; }
constexpr bool Letter(char ch) noexcept { return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_'; }

std::string_view Trim(std::string_view text) noexcept {
	while (!text.empty() && Space(text.front())) text.remove_prefix(1);
	while (!text.empty() && Space(text.back())) text.remove_suffix(1);
	return text;
}

std::string Lower(std::string_view text) {
	std::string result(text);
	for (char &ch : result) if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
	return result;
}

// NASTRAN also accepts compact exponents: 1.25-3, -.21+5, 1.+6.
bool Number(std::string_view text) noexcept {
	size_t pos = 0;
	if (pos < text.size() && (text[pos] == '+' || text[pos] == '-')) ++pos;
	bool digits = false;
	while (pos < text.size() && Digit(text[pos])) { ++pos; digits = true; }
	if (pos < text.size() && text[pos] == '.') {
		++pos;
		while (pos < text.size() && Digit(text[pos])) { ++pos; digits = true; }
	}
	if (!digits) return false;
	if (pos == text.size()) return true;
	if (text[pos] == 'e' || text[pos] == 'E' || text[pos] == 'd' || text[pos] == 'D') ++pos;
	else if (text[pos] != '+' && text[pos] != '-') return false;
	if (pos < text.size() && (text[pos] == '+' || text[pos] == '-')) ++pos;
	const size_t exponent = pos;
	while (pos < text.size() && Digit(text[pos])) ++pos;
	return pos > exponent && pos == text.size();
}

size_t ColumnAfter(size_t column, char ch) noexcept {
	return ch == '\t' ? (column / 8 + 1) * 8 : column + 1;
}

struct CardLine {
	size_t headEnd;
	bool free;
	bool continuation;
	bool large;
};

CardLine Inspect(std::string_view text) {
	size_t headEnd = 0;
	size_t column = 0;
	while (headEnd < text.size() && column < 8 && text[headEnd] != ',') {
		column = ColumnAfter(column, text[headEnd]);
		++headEnd;
	}
	// Free-field detection uses the first ten expanded columns. A comma in a
	// later fixed-width character field must not change the line's layout.
	size_t delimiter = headEnd;
	size_t delimiterColumn = column;
	while (delimiter < text.size() && delimiterColumn < 10 && Space(text[delimiter]))
		delimiterColumn = ColumnAfter(delimiterColumn, text[delimiter++]);
	const bool free = delimiter < text.size() && delimiterColumn < 10 && text[delimiter] == ',';
	if (free) headEnd = delimiter;
	const auto head = Trim(text.substr(0, headEnd));
	const bool continuation = head.empty() || head.front() == '+' || head.front() == '*';
	const bool large = !head.empty() && (head.front() == '*' || head.back() == '*');
	return {headEnd, free, continuation, large};
}

std::string FirstWord(std::string_view text) {
	text = Trim(text);
	size_t end = 0;
	while (end < text.size() && (Letter(text[end]) || Digit(text[end]))) ++end;
	return Lower(text.substr(0, end));
}

// Dollar signs and commas inside quoted INCLUDE paths are literal. Backslashes
// in Windows paths are not C-style escapes. A quoted INCLUDE can span lines.
size_t CommentStart(std::string_view text, char &quote) noexcept {
	for (size_t i = 0; i < text.size(); ++i) {
		if (quote) {
			if (text[i] == quote) {
				if (i + 1 < text.size() && text[i + 1] == quote) ++i;
				else quote = 0;
			}
		} else if (text[i] == '\'' || text[i] == '"') quote = text[i];
		else if (text[i] == '$') return i;
	}
	return text.size();
}

enum class Phase { Unknown, Executive, Case, Bulk };
struct LineState {
	Phase phase = Phase::Unknown;
	bool section = false;
	bool subcase = false;
	bool card = false;
	char includeQuote = 0;
	int level = SC_FOLDLEVELBASE;
	int nextLevel = SC_FOLDLEVELBASE;
	bool header = false;
	bool operator<(const LineState &other) const noexcept {
		return std::tie(phase, section, subcase, card, includeQuote, level, nextLevel, header) <
			std::tie(other.phase, other.section, other.subcase, other.card, other.includeQuote,
				other.level, other.nextLevel, other.header);
	}
};

const char *const nastranWordLists[] = {"Bulk data cards", "Control keywords and field labels", nullptr};

class LexerNASTRAN final : public LexerBase {
	// Immutable IDs are attached to Scintilla lines and move with document edits.
	std::vector<LineState> states{LineState{}};
	std::map<LineState, int> stateIDs{{LineState{}, 0}};

	LineState StateAt(LexAccessor &styler, Sci_Position line) const {
		const int id = line >= 0 ? styler.GetLineState(line) : 0;
		return id >= 0 && static_cast<size_t>(id) < states.size() ? states[id] : LineState{};
	}
	int Remember(const LineState &state) {
		const auto found = stateIDs.find(state);
		if (found != stateIDs.end()) return found->second;
		const int id = static_cast<int>(states.size());
		states.push_back(state);
		stateIDs.emplace(state, id);
		return id;
	}
	static int ParentDepth(const LineState &state) noexcept {
		return SC_FOLDLEVELBASE + state.section + state.subcase;
	}
	static int Depth(const LineState &state) noexcept { return ParentDepth(state) + state.card; }

	static bool NextContinuation(LexAccessor &styler, Sci_Position line, bool quoted) {
		const Sci_Position last = styler.GetLine(styler.Length());
		for (++line; line <= last; ++line) {
			const std::string text = styler.GetRange(styler.LineStart(line), styler.LineEnd(line));
			const auto trimmed = Trim(text);
			if (trimmed.empty()) continue;
			if (quoted) return true;
			if (trimmed.front() == '$') continue;
			return Inspect(text).continuation;
		}
		return false;
	}

	static void Paint(std::vector<char> &styles, size_t begin, size_t end, int style) {
		std::fill(styles.begin() + begin, styles.begin() + end, static_cast<char>(style));
	}

	void Tokens(std::string_view text, size_t offset, std::vector<char> &styles, char incomingQuote = 0) {
		char quote = incomingQuote;
		for (size_t i = 0; i < text.size();) {
			const size_t begin = i;
			if (Space(text[i])) { ++i; continue; }
			if (quote || text[i] == '\'' || text[i] == '"') {
				if (!quote) quote = text[i++];
				while (i < text.size()) {
					if (text[i++] == quote) {
						if (i < text.size() && text[i] == quote) ++i;
						else { quote = 0; break; }
					}
				}
				Paint(styles, offset + begin, offset + i, SCE_NASTRAN_STRING);
			} else if (Digit(text[i]) || text[i] == '.' || text[i] == '+' || text[i] == '-') {
				++i;
				while (i < text.size() && (Digit(text[i]) || text[i] == '.' || text[i] == '+' || text[i] == '-' ||
					text[i] == 'e' || text[i] == 'E' || text[i] == 'd' || text[i] == 'D')) ++i;
				Paint(styles, offset + begin, offset + i,
					Number(text.substr(begin, i - begin)) ? SCE_NASTRAN_NUMBER : SCE_NASTRAN_OPERATOR);
			} else if (Letter(text[i])) {
				while (i < text.size() && (Letter(text[i]) || Digit(text[i]))) ++i;
				const std::string word = Lower(text.substr(begin, i - begin));
				Paint(styles, offset + begin, offset + i,
					keyWordLists[1]->InList(word) ? SCE_NASTRAN_KEYWORD : SCE_NASTRAN_IDENTIFIER);
			} else {
				Paint(styles, offset + i, offset + i + 1, SCE_NASTRAN_OPERATOR);
				++i;
			}
		}
	}

	void Bulk(std::string_view text, std::vector<char> &styles, const CardLine &card) {
		Paint(styles, 0, card.headEnd, card.continuation ? SCE_NASTRAN_CONTINUATION : SCE_NASTRAN_CARD);
		if (card.free) {
			size_t start = card.headEnd + 1;
			Paint(styles, card.headEnd, start, SCE_NASTRAN_OPERATOR);
			size_t field = 1;
			char quote = 0;
			for (size_t i = start; i <= text.size(); ++i) {
				if (i < text.size() && quote) {
					if (text[i] == quote) {
						if (i + 1 < text.size() && text[i + 1] == quote) ++i;
						else quote = 0;
					}
					continue;
				}
				if (i < text.size() && (text[i] == '\'' || text[i] == '"')) { quote = text[i]; continue; }
				if (i < text.size() && text[i] != ',') continue;
				const auto value = text.substr(start, i - start);
				if (field == (card.large ? 5U : 9U)) Paint(styles, start, i, SCE_NASTRAN_CONTINUATION);
				else Tokens(value, start, styles);
				if (i < text.size()) Paint(styles, i, i + 1, SCE_NASTRAN_OPERATOR);
				start = i + 1;
				++field;
			}
		} else {
			// Map physical columns without expanding/replacing tabs in the document.
			// Small fields: 8 + 8*8; large fields: 8 + 4*16; labels: columns 73-80.
			size_t column = 8;
			size_t start = card.headEnd;
			while (start < text.size() && column < 80) {
				const size_t boundary = column >= 72 ? 80 :
					std::min(size_t{72}, size_t{8} + ((column - 8) / (card.large ? 16 : 8) + 1) * (card.large ? 16 : 8));
				size_t end = start;
				while (end < text.size() && column < boundary) column = ColumnAfter(column, text[end++]);
				if (boundary == 80) Paint(styles, start, end, SCE_NASTRAN_CONTINUATION);
				else Tokens(text.substr(start, end - start), start, styles);
				start = end;
			}
		}
	}

	void Line(LexAccessor &styler, Sci_Position line, std::string_view text, std::vector<char> &styles, LineState &state) {
		state.level = Depth(state);
		state.header = false;
		if (Trim(text).empty()) return;
		const char incomingQuote = state.includeQuote;
		char quote = incomingQuote;
		const size_t comment = CommentStart(text, quote);
		if (comment < text.size()) Paint(styles, comment, styles.size(), SCE_NASTRAN_COMMENT);
		const auto code = text.substr(0, comment);
		const auto trimmed = Trim(code);
		if (trimmed.empty()) return;
		const std::string word = incomingQuote ? "" : FirstWord(trimmed);
		const bool begin = word == "begin" && (FirstWord(Trim(trimmed.substr(5))) == "bulk" || FirstWord(Trim(trimmed.substr(5))) == "super");
		const bool executive = (word == "sol" || word == "id" || word == "assign" || word == "nastran") &&
			(state.phase == Phase::Unknown || state.phase == Phase::Executive);
		if (word == "cend" || begin || executive) {
			const bool newSection = !executive || !state.section;
			state.phase = begin ? Phase::Bulk : word == "cend" ? Phase::Case : Phase::Executive;
			state.subcase = state.card = false;
			state.includeQuote = 0;
			state.section = true;
			state.level = newSection ? SC_FOLDLEVELBASE : ParentDepth(state);
			state.header = newSection;
			Paint(styles, 0, comment, SCE_NASTRAN_SECTION);
			return;
		}
		if (word == "enddata") {
			state.card = state.subcase = false;
			state.level = ParentDepth(state);
			state.phase = Phase::Unknown;
			state.section = false;
			state.includeQuote = 0;
			Paint(styles, 0, comment, SCE_NASTRAN_SECTION);
			return;
		}
		if (word == "subcase" && state.phase == Phase::Case) {
			state.card = false;
			state.subcase = false;
			state.level = ParentDepth(state);
			state.subcase = true;
			state.header = true;
			Paint(styles, 0, comment, SCE_NASTRAN_SECTION);
			return;
		}
		const bool include = word == "include" || incomingQuote;
		if (include) {
			const bool continuation = incomingQuote && state.card;
			state.level = ParentDepth(state) + continuation;
			state.includeQuote = quote;
			state.card = quote && NextContinuation(styler, line, true);
			state.header = !incomingQuote && state.card;
			Tokens(code, 0, styles, incomingQuote);
			if (!incomingQuote) {
				const size_t start = text.find_first_not_of(" \t");
				Paint(styles, start, start + 7, SCE_NASTRAN_KEYWORD);
			}
		} else if (state.phase == Phase::Unknown || state.phase == Phase::Bulk) {
			const CardLine card = Inspect(code);
			const bool continuation = card.continuation && state.card;
			state.level = ParentDepth(state) + continuation;
			state.card = (!card.continuation || continuation) && NextContinuation(styler, line, false);
			state.header = !card.continuation && state.card;
			Bulk(code, styles, card);
		} else {
			state.card = false;
			state.level = ParentDepth(state);
			Tokens(code, 0, styles);
		}
	}

	void FoldLine(LexAccessor &styler, Sci_Position line, const LineState &state, bool blank) const {
		int level = SC_FOLDLEVELBASE;
		if (props.GetInt("fold")) {
			level = state.level | (state.nextLevel << 16);
			if (state.header) level |= SC_FOLDLEVELHEADERFLAG;
			if (blank && props.GetInt("fold.compact")) level |= SC_FOLDLEVELWHITEFLAG;
		}
		styler.SetLevelIfDifferent(line, level);
	}

public:
	const char *SCI_METHOD GetName() override { return "nastran"; }
	int SCI_METHOD GetIdentifier() override { return SCLEX_NASTRAN; }
	const char *SCI_METHOD PropertyNames() override { return "fold\nfold.compact"; }
	const char *SCI_METHOD DescribeWordListSets() override { return "Bulk data cards\nControl keywords and field labels"; }

	void SCI_METHOD Lex(Sci_PositionU startPos, Sci_Position length, int, IDocument *document) override {
		LexAccessor styler(document);
		const Sci_Position requestedEnd = std::min(styler.Length(), static_cast<Sci_Position>(startPos) + length);
		const Sci_Position requestedLast = styler.GetLine(requestedEnd);
		Sci_Position line = styler.GetLine(startPos);
		// Fold headers depend on the following meaningful line. Revisit that
		// predecessor when an edit creates/removes a continuation, even across
		// comments/blank lines; do not rescan the entire deck on each keystroke.
		if (line > 0) --line;
		while (line > 0) {
			const std::string text = styler.GetRange(styler.LineStart(line), styler.LineEnd(line));
			const auto trimmed = Trim(text);
			if (!trimmed.empty() && trimmed.front() != '$') break;
			--line;
		}
		LineState state = StateAt(styler, line - 1);
		const Sci_Position last = styler.GetLine(styler.Length());
		for (; line <= last; ++line) {
			const Sci_Position begin = styler.LineStart(line);
			const Sci_Position end = line < last ? styler.LineStart(line + 1) : styler.Length();
			const std::string text = styler.GetRange(begin, styler.LineEnd(line));
			std::vector<char> styles(static_cast<size_t>(end - begin), SCE_NASTRAN_DEFAULT);
			Line(styler, line, text, styles, state);
			state.nextLevel = Depth(state);
			styler.StartAt(begin);
			styler.StartSegment(begin);
			for (size_t i = 0; i < styles.size();) {
				size_t next = i + 1;
				while (next < styles.size() && styles[next] == styles[i]) ++next;
				styler.ColourTo(begin + next - 1, styles[i]);
				i = next;
			}
			styler.Flush();
			FoldLine(styler, line, state, Trim(text).empty());
			const int id = Remember(state);
			const int previous = styler.GetLineState(line);
			styler.SetLineState(line, id);
			if (line >= requestedLast && id == previous) break;
		}
	}

	void SCI_METHOD Fold(Sci_PositionU startPos, Sci_Position length, int, IDocument *document) override {
		LexAccessor styler(document);
		const Sci_Position last = styler.GetLine(std::min(styler.Length(), static_cast<Sci_Position>(startPos) + length));
		for (Sci_Position line = styler.GetLine(startPos); line <= last; ++line)
			FoldLine(styler, line, StateAt(styler, line), Trim(styler.GetRange(styler.LineStart(line), styler.LineEnd(line))).empty());
	}
};

ILexer5 *LexerFactoryNASTRAN() { return new LexerNASTRAN; }

}

extern const LexerModule lmNASTRAN(SCLEX_NASTRAN, LexerFactoryNASTRAN, "nastran", nastranWordLists);
