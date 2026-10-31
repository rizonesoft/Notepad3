// Scintilla source code edit control
/** @file LexAPDL.cxx
 ** ANSYS Mechanical APDL, including CDB data blocks.
 ** The License.txt file describes the conditions under which this software may be distributed.
 **/

#include <algorithm>
#include <cassert>
#include <charconv>
#include <limits>
#include <map>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include "ILexer.h"
#include "Scintilla.h"
#include "SciLexer.h"
#include "PropSetSimple.h"
#include "WordList.h"
#include "LexAccessor.h"
#include "LexerModule.h"
#include "LexerBase.h"

using namespace Scintilla;
using namespace Lexilla;

namespace {

constexpr bool Space(char ch) noexcept { return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n'; }
constexpr bool Digit(char ch) noexcept { return ch >= '0' && ch <= '9'; }
constexpr bool Letter(char ch) noexcept { return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_'; }
constexpr bool WordChar(char ch) noexcept { return Letter(ch) || Digit(ch); }

std::string_view Trim(std::string_view text) noexcept {
	while (!text.empty() && Space(text.front())) text.remove_prefix(1);
	while (!text.empty() && Space(text.back())) text.remove_suffix(1);
	return text;
}

std::string Lower(std::string_view text) {
	std::string result(text);
	for (char &ch : result) {
		if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
	}
	return result;
}

// Commas in expressions and quoted strings are not argument separators.
std::vector<std::string_view> Fields(std::string_view text) {
	std::vector<std::string_view> fields;
	char quote = 0;
	int parentheses = 0;
	size_t start = 0;
	for (size_t i = 0; i < text.size(); ++i) {
		const char ch = text[i];
		if (quote) {
			if (ch == quote) {
				if (i + 1 < text.size() && text[i + 1] == quote) ++i;
				else quote = 0;
			}
		} else if (ch == '\'' || ch == '"') {
			quote = ch;
		} else if (ch == '(') {
			++parentheses;
		} else if (ch == ')') {
			parentheses = std::max(0, parentheses - 1);
		} else if (ch == ',' && !parentheses) {
			fields.push_back(Trim(text.substr(start, i - start)));
			start = i + 1;
		}
	}
	fields.push_back(Trim(text.substr(start)));
	while (!fields.empty() && fields.back().empty()) fields.pop_back();
	return fields;
}

size_t Unsigned(std::string_view text) noexcept {
	text = Trim(text);
	size_t value = 0;
	const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
	return result.ec == std::errc() && result.ptr == text.data() + text.size() ? value : 0;
}

// CDB block grammars: Programmer's Reference, "Coded Database File Commands".
// Keep explicit rules: a *BLOCK suffix alone does not imply a -1 terminator.
enum class Block { Root, Loop, Conditional, Macro, File, Node, Element, ElementList, Material, Contact, Load, InitialState };
enum class Data { None, Nodes, Sentinel, BodyForce, ElementBodyForce, SurfaceLoad, Component, ComponentNames,
	RealConstants, SectionHeader, SectionRows, Reinforcement, Parameter };

struct DataBlock {
	std::string_view command;
	Data kind;
};

constexpr DataBlock dataBlocks[] = {
	{"nblock", Data::Nodes}, {"eblock", Data::Sentinel}, {"etblock", Data::Sentinel},
	{"dblock", Data::Sentinel}, {"fblock", Data::Sentinel}, {"nsmblock", Data::Sentinel},
	{"bfblock", Data::BodyForce}, {"bfeblock", Data::ElementBodyForce}, {"sfeblock", Data::SurfaceLoad},
	{"*pread", Data::Parameter},
};

std::vector<std::string_view> SpaceFields(std::string_view text) {
	std::vector<std::string_view> fields;
	while (!text.empty()) {
		while (!text.empty() && (Space(text.front()) || text.front() == ',')) text.remove_prefix(1);
		if (text.empty()) break;
		size_t end = 0;
		while (end < text.size() && !Space(text[end]) && text[end] != ',') ++end;
		fields.push_back(text.substr(0, end));
		text.remove_prefix(end);
	}
	return fields;
}

size_t FormatWidth(std::string_view format) noexcept {
	const size_t integer = format.find('i');
	if (integer == std::string_view::npos) return 0;
	size_t end = integer + 1;
	while (end < format.size() && Digit(format[end])) ++end;
	return Unsigned(format.substr(integer + 1, end - integer - 1));
}

size_t FormatRepeat(std::string_view format, char descriptor, size_t fallback) noexcept {
	const size_t end = format.find(descriptor);
	if (end == std::string_view::npos) return fallback;
	size_t begin = end;
	while (begin && Digit(format[begin - 1])) --begin;
	return begin < end ? Unsigned(format.substr(begin, end - begin)) : fallback;
}

struct Frame {
	int parent;
	Block kind;
	int depth;
};

struct LineState {
	int stack = 0;
	Data data = Data::None;
	size_t remaining = 0;
	size_t width = 0;
	size_t continuation = 0; // remaining reals in an RL set, or EFACE bitmap entries
	size_t firstValues = 6;
	size_t valuesPerLine = 7;
	int format = 0; // number of format descriptor lines still expected
	bool ioFormat = false;
	int level = SC_FOLDLEVELBASE;
	int nextLevel = SC_FOLDLEVELBASE;

	bool operator<(const LineState &other) const noexcept {
		return std::tie(stack, data, remaining, width, continuation, firstValues, valuesPerLine, format, ioFormat, level, nextLevel) <
			std::tie(other.stack, other.data, other.remaining, other.width, other.continuation, other.firstValues,
				other.valuesPerLine, other.format, other.ioFormat, other.level, other.nextLevel);
	}
};

const char *const apdlWordLists[] = {
	"Processors", "Commands", "Arguments", "Functions", nullptr
};

class LexerAPDL final : public LexerBase {
	// IDocument moves line-state IDs with lines on insertion/deletion. Interned
	// immutable snapshots therefore remain valid without shifting a private cache.
	std::vector<LineState> states{LineState{}};
	std::map<LineState, int> stateIDs{{LineState{}, 0}};
	std::vector<Frame> frames{{0, Block::Root, 0}};
	std::map<std::pair<int, Block>, int> frameIDs;

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

	void Push(LineState &state, Block kind) {
		// Fold levels have only SC_FOLDLEVELNUMBERMASK bits available.
		if (frames[state.stack].depth >= SC_FOLDLEVELNUMBERMASK - SC_FOLDLEVELBASE - 1) return;
		const auto key = std::make_pair(state.stack, kind);
		const auto found = frameIDs.find(key);
		if (found != frameIDs.end()) state.stack = found->second;
		else {
			const int id = static_cast<int>(frames.size());
			frames.push_back({state.stack, kind, frames[state.stack].depth + 1});
			frameIDs.emplace(key, id);
			state.stack = id;
		}
	}

	void Pop(LineState &state, Block kind) const {
		if (frames[state.stack].kind == kind) state.stack = frames[state.stack].parent;
	}

	int Depth(const LineState &state) const noexcept {
		return SC_FOLDLEVELBASE + frames[state.stack].depth + (state.data != Data::None);
	}

	static void EndData(LineState &state) noexcept {
		state.data = Data::None;
		state.remaining = state.width = 0;
		state.continuation = 0;
		state.firstValues = 6;
		state.valuesPerLine = 7;
		state.format = 0;
	}

	static void EndComponentPart(LineState &state) noexcept {
		if (state.continuation) {
			state.data = Data::Component;
			state.remaining = state.continuation;
			state.continuation = state.width = 0;
			state.format = 1;
		} else EndData(state);
	}

	void Command(std::string_view text, std::string_view command, LineState &state) {
		const auto fields = Fields(text);
		if (command == "*do" || command == "*dowhile") Push(state, Block::Loop);
		else if (command == "*enddo") Pop(state, Block::Loop);
		else if (command == "*if" && !fields.empty() && Lower(fields.back()) == "then") Push(state, Block::Conditional);
		else if ((command == "*else" || command == "*elseif") && frames[state.stack].kind == Block::Conditional) {
			state.level = std::min(state.level, Depth(state) - 1);
		} else if (command == "*endif") Pop(state, Block::Conditional);
		else if (command == "*create") Push(state, Block::Macro);
		else if (command == "*end") Pop(state, Block::Macro);
		else if (command == "/block" && fields.size() >= 2) {
			const std::string type = Lower(fields[1]);
			Block kind = Block::Root;
			if (type == "file") kind = Block::File;
			else if (type == "node") kind = Block::Node;
			else if (type == "elem") kind = Block::Element;
			else if (type == "elst" || type == "elen") kind = Block::ElementList;
			else if (type == "mat") kind = Block::Material;
			else if (type == "cont") kind = Block::Contact;
			else if (type == "load") kind = Block::Load;
			else if (type == "inis") kind = Block::InitialState;
			const std::string action = fields.size() >= 3 ? Lower(fields[2]) : "";
			if (kind != Block::Root) {
				if (type == "elen" || action == "end") Pop(state, kind);
				else if (type == "elst" || action == "start") Push(state, kind);
			}
		} else if (command == "cmblock" && fields.size() >= 4) {
			state.remaining = Unsigned(fields[3]);
			if (state.remaining) {
				state.data = Data::Component;
				state.format = 1;
				if ((Lower(fields[2]) == "efac" || Lower(fields[2]) == "eface") && fields.size() > 6)
					state.continuation = Unsigned(fields[6]);
			}
		} else if (command == "rlblock" && fields.size() >= 2) {
			state.remaining = Unsigned(fields[1]);
			if (state.remaining) {
				state.data = Data::RealConstants;
				state.width = 8;
				state.format = 2;
			}
		} else if (command == "secblock") {
			if (fields.size() == 1) state.data = Data::SectionHeader;
			else {
				state.remaining = Unsigned(fields[1]);
				if (state.remaining) state.data = fields.size() >= 4 ? Data::Reinforcement : Data::SectionRows;
			}
		} else {
			for (const auto &block : dataBlocks) {
				if (command == block.command) {
					state.data = block.kind;
					state.format = 1;
					break;
				}
			}
		}
		state.ioFormat = command == "*vread" || command == "*vwrite" || command == "*mwrite";
	}

	void Colour(std::string_view text, size_t offset, std::vector<char> &styles, bool commands) {
		auto paint = [&](size_t begin, size_t end, int style) {
			std::fill(styles.begin() + offset + begin, styles.begin() + offset + end, static_cast<char>(style));
		};
		bool head = commands;
		for (size_t i = 0; i < text.size();) {
			const size_t begin = i;
			const char ch = text[i];
			if (Space(ch)) { ++i; continue; }
			if (ch == '!') { paint(i, text.size(), SCE_APDL_COMMENT); break; }
			if (ch == '\'' || ch == '"') {
				++i;
				while (i < text.size()) {
					if (text[i++] == ch) {
						if (i < text.size() && text[i] == ch) ++i;
						else break;
					}
				}
				paint(begin, i, SCE_APDL_STRING);
			} else if (Letter(ch) || (head && (ch == '/' || ch == '*') && i + 1 < text.size() && Letter(text[i + 1]))) {
				++i;
				while (i < text.size() && WordChar(text[i])) ++i;
				const std::string word = Lower(text.substr(begin, i - begin));
				size_t next = i;
				while (next < text.size() && Space(text[next])) ++next;
				int style = SCE_APDL_WORD;
				if (head && word == "/com") { paint(begin, text.size(), SCE_APDL_COMMENT); break; }
				if (head && (next == text.size() || text[next] != '=')) {
					style = keyWordLists[0]->InList(word) ? SCE_APDL_PROCESSOR :
						ch == '*' ? SCE_APDL_STARCOMMAND : ch == '/' ? SCE_APDL_SLASHCOMMAND : SCE_APDL_COMMAND;
				} else if (keyWordLists[3]->InList(word) && next < text.size() && text[next] == '(') style = SCE_APDL_FUNCTION;
				else if (keyWordLists[2]->InList(word)) style = SCE_APDL_ARGUMENT;
				paint(begin, i, style);
			} else if (Digit(ch) || (ch == '.' && i + 1 < text.size() && Digit(text[i + 1]))) {
				while (i < text.size() && Digit(text[i])) ++i;
				if (i < text.size() && text[i] == '.') {
					++i;
					while (i < text.size() && Digit(text[i])) ++i;
				}
				if (i < text.size() && (text[i] == 'e' || text[i] == 'E' || text[i] == 'd' || text[i] == 'D')) {
					size_t exponent = i + 1;
					if (exponent < text.size() && (text[exponent] == '+' || text[exponent] == '-')) ++exponent;
					if (exponent < text.size() && Digit(text[exponent])) {
						i = exponent + 1;
						while (i < text.size() && Digit(text[i])) ++i;
					}
				}
				paint(begin, i, SCE_APDL_NUMBER);
			} else if (ch == '%' && text.find('%', i + 1) != std::string_view::npos) {
				i = text.find('%', i + 1) + 1;
				paint(begin, i, SCE_APDL_WORD);
			} else {
				paint(i, i + 1, SCE_APDL_OPERATOR);
				++i;
			}
			head = false;
		}
	}

	void Line(std::string_view text, std::vector<char> &styles, LineState &state) {
		const std::string_view trimmed = Trim(text);
		state.level = Depth(state);
		if (trimmed.empty()) return;
		const bool comment = trimmed.size() >= 4 && Lower(trimmed.substr(0, 4)) == "/com" &&
			(trimmed.size() == 4 || trimmed[4] == ',' || Space(trimmed[4]));
		if (trimmed.front() == '!' || comment) {
			if (comment && state.data == Data::Reinforcement && !state.remaining) {
				EndData(state);
				state.level = Depth(state);
			}
			Colour(text, 0, styles, comment);
			return;
		}
		if (state.data != Data::None) {
			const std::string record = Lower(Trim(text.substr(0, text.find('!'))));
			const auto fields = Fields(record);
			auto commandEnd = [&](std::string_view name) {
				if (fields.empty() || fields[0] != name) return false;
				return (fields.size() == 2 && fields[1] == "-1") ||
					(fields.size() == 4 && fields[2] == "loc" && fields[3] == "-1" &&
						(fields[1] == "end" || fields[1] == "unbl" || (name == "n" && fields[1] == "r5.3")));
			};
			const bool endCommand = (state.data == Data::Nodes && commandEnd("n")) ||
				(state.data == Data::BodyForce && commandEnd("bf")) ||
				(state.data == Data::ElementBodyForce && commandEnd("bfe")) ||
				(state.data == Data::SurfaceLoad && commandEnd("sfe"));
			const auto firstField = std::string_view(record).substr(0, record.find_first_of(" \t,"));
			const bool sentinel = (state.data == Data::Nodes || state.data == Data::Sentinel) && firstField == "-1";
			const auto words = state.data == Data::Parameter ? SpaceFields(record) : std::vector<std::string_view>{};
			const bool parameterEnd = words.size() == 2 && words[0] == "end" && words[1] == "pread";
			if (endCommand || sentinel || parameterEnd) {
				Colour(text, 0, styles, endCommand || parameterEnd);
				EndData(state);
				return;
			}
			if (trimmed.front() == '(' && state.format) {
				const std::string format = Lower(trimmed);
				if (state.data == Data::Component) {
					state.width = FormatWidth(format);
					if (format.find('a') != std::string::npos) state.data = Data::ComponentNames;
				} else if (state.data == Data::RealConstants) {
					if (state.format == 2) {
						state.width = FormatWidth(format);
						state.firstValues = FormatRepeat(format, 'g', 6);
					} else state.valuesPerLine = FormatRepeat(format, 'g', 7);
				}
				std::fill(styles.begin(), styles.end(), SCE_APDL_STRING);
				--state.format;
				return;
			}
			// EFACE component references are names, not APDL commands. A second
			// descriptor and NUMBF bitmap entries follow the NUMITEMS references.
			if (state.data == Data::ComponentNames) {
				std::fill(styles.begin(), styles.end(), SCE_APDL_STRING);
				if (state.remaining && --state.remaining == 0) EndComponentPart(state);
				return;
			}
			if (!Letter(trimmed.front()) && trimmed.front() != '*' && trimmed.front() != '/') {
				Colour(text, 0, styles, false);
				if (state.data == Data::Component) {
					// Count encoded integers, not the expanded node ranges. The format
					// width also handles adjacent fields with no separating whitespace.
					size_t count = 0;
					const auto data = text.substr(0, text.find('!'));
					for (size_t pos = 0; pos < data.size();) {
						if (state.width) {
							if (!Trim(data.substr(pos, state.width)).empty()) ++count;
							pos += std::min(state.width, data.size() - pos);
						} else {
							while (pos < data.size() && (Space(data[pos]) || data[pos] == ',')) ++pos;
							if (pos < data.size()) ++count;
							while (pos < data.size() && !Space(data[pos]) && data[pos] != ',') ++pos;
						}
					}
					if (count >= state.remaining) EndComponentPart(state);
					else state.remaining -= count;
				} else if (state.data == Data::RealConstants) {
					if (state.continuation) {
						state.continuation -= std::min(state.continuation, state.valuesPerLine);
					} else {
						// The first record of each set has two integer fields followed
						// by up to six reals; subsequent records contain up to seven.
						size_t values = 0;
						if (state.width && state.width < text.size()) values = Unsigned(text.substr(state.width, state.width));
						else {
							const auto numbers = SpaceFields(text);
							if (numbers.size() >= 2) values = Unsigned(numbers[1]);
						}
						state.continuation = values - std::min(values, state.firstValues);
					}
					if (!state.continuation && state.remaining && --state.remaining == 0) EndData(state);
				} else if (state.data == Data::SectionHeader) {
					const auto numbers = SpaceFields(text);
					if (numbers.size() >= 2) {
						const size_t nodes = Unsigned(numbers[0]);
						const size_t cells = Unsigned(numbers[1]);
						if (nodes <= std::numeric_limits<size_t>::max() - cells) {
							state.remaining = nodes + cells;
							state.data = Data::SectionRows;
							if (!state.remaining) EndData(state);
						}
					}
				} else if (state.data == Data::SectionRows) {
					if (state.remaining && --state.remaining == 0) EndData(state);
				} else if (state.data == Data::Reinforcement) {
					// A negative element-ID record is an optional Format2 continuation,
					// including for the last member; it is never a -1 sentinel. Keep
					// those records in the fold and resume at the following command.
					if (trimmed.front() != '-') {
						if (state.remaining) --state.remaining;
						else { EndData(state); state.level = Depth(state); }
					}
				}
				return;
			}
			// Recover at a command if an incomplete/malformed data block lacks its
			// terminator. Do not swallow all subsequent APDL commands as data.
			EndData(state);
			state.level = Depth(state);
		}
		if (state.ioFormat && (trimmed.front() == '(' || trimmed.front() == '%')) {
			std::fill(styles.begin(), styles.end(), SCE_APDL_STRING);
			state.ioFormat = false;
			return;
		}
		state.ioFormat = false;
		// Each $-separated command has its own command position. Comments consume
		// the rest of the physical line, including any dollar signs.
		size_t start = 0;
		char quote = 0;
		for (size_t i = 0; i <= text.size(); ++i) {
			if (i == start) {
				const auto rest = Trim(text.substr(start));
				if (rest.size() >= 4 && Lower(rest.substr(0, 4)) == "/com" &&
					(rest.size() == 4 || rest[4] == ',' || Space(rest[4]))) {
					Colour(text.substr(start), start, styles, true);
					break;
				}
			}
			const char ch = i < text.size() ? text[i] : '\0';
			if (quote && i < text.size()) {
				if (ch == quote) {
					if (i + 1 < text.size() && text[i + 1] == quote) ++i;
					else quote = 0;
				}
				continue;
			}
			if (ch == '\'' || ch == '"') { quote = ch; continue; }
			if (ch != '$' && ch != '!' && i != text.size()) continue;
			const auto segment = Trim(text.substr(start, i - start));
			Colour(text.substr(start, i - start), start, styles, true);
			size_t wordEnd = !segment.empty() && (segment[0] == '*' || segment[0] == '/') ? 1 : 0;
			while (wordEnd < segment.size() && WordChar(segment[wordEnd])) ++wordEnd;
			const auto tail = Trim(segment.substr(wordEnd));
			if (wordEnd && (tail.empty() || tail.front() == ',')) Command(segment, Lower(segment.substr(0, wordEnd)), state);
			if (ch == '!') {
				std::fill(styles.begin() + i, styles.end(), SCE_APDL_COMMENT);
				break;
			}
			if (ch == '$') styles[i] = SCE_APDL_OPERATOR;
			start = i + 1;
		}
	}

	void FoldLine(LexAccessor &styler, Sci_Position line, const LineState &state, bool blank) const {
		int level = SC_FOLDLEVELBASE;
		if (props.GetInt("fold")) {
			level = state.level | (state.nextLevel << 16);
			if (state.nextLevel > state.level) level |= SC_FOLDLEVELHEADERFLAG;
			if (blank && props.GetInt("fold.compact")) level |= SC_FOLDLEVELWHITEFLAG;
		}
		styler.SetLevelIfDifferent(line, level);
	}

public:
	const char *SCI_METHOD GetName() override { return "apdl"; }
	int SCI_METHOD GetIdentifier() override { return SCLEX_APDL; }
	const char *SCI_METHOD PropertyNames() override { return "fold\nfold.compact"; }
	const char *SCI_METHOD DescribeWordListSets() override { return "Processors\nCommands\nArguments\nFunctions"; }

	void SCI_METHOD Lex(Sci_PositionU startPos, Sci_Position length, int, IDocument *document) override {
		LexAccessor styler(document);
		Sci_Position line = styler.GetLine(startPos);
		LineState state = StateAt(styler, line - 1);
		const Sci_Position requestedEnd = std::min(styler.Length(), static_cast<Sci_Position>(startPos) + length);
		const Sci_Position lastLine = styler.GetLine(styler.Length());
		for (; line <= lastLine; ++line) {
			const Sci_Position begin = styler.LineStart(line);
			const Sci_Position end = line < lastLine ? styler.LineStart(line + 1) : styler.Length();
			const std::string text = styler.GetRange(begin, styler.LineEnd(line));
			std::vector<char> styles(static_cast<size_t>(end - begin), SCE_APDL_DEFAULT);
			Line(text, styles, state);
			state.nextLevel = Depth(state);
			styler.StartAt(begin);
			styler.StartSegment(begin);
			// Styling whole lines ensures restarting inside a string/command is
			// identical to a full pass. Emit runs rather than one call per byte.
			for (size_t i = 0; i < styles.size();) {
				size_t next = i + 1;
				while (next < styles.size() && styles[next] == styles[i]) ++next;
				styler.ColourTo(begin + next - 1, styles[i]);
				i = next;
			}
			styler.Flush();
			FoldLine(styler, line, state, Trim(text).empty());
			const int id = Remember(state);
			const int oldID = styler.GetLineState(line);
			styler.SetLineState(line, id);
			if (end >= requestedEnd && id == oldID) break;
		}
	}

	void SCI_METHOD Fold(Sci_PositionU startPos, Sci_Position length, int, IDocument *document) override {
		LexAccessor styler(document);
		const Sci_Position last = styler.GetLine(std::min(styler.Length(), static_cast<Sci_Position>(startPos) + length));
		for (Sci_Position line = styler.GetLine(startPos); line <= last; ++line) {
			FoldLine(styler, line, StateAt(styler, line),
				Trim(styler.GetRange(styler.LineStart(line), styler.LineEnd(line))).empty());
		}
	}
};

ILexer5 *LexerFactoryAPDL() { return new LexerAPDL; }

}

extern const LexerModule lmAPDL(SCLEX_APDL, LexerFactoryAPDL, "apdl", apdlWordLists);
