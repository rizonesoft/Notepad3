#pragma once

namespace Scintilla {

// TODO: change packed line state to NestedStateStack (convert lexer to class).

template<int bitCount, int maxStateCount, int PackState(int state) noexcept>
inline int PackLineState(const std::vector<int>& states) noexcept {
	int lineState = 0;
	int count = 0;
	size_t index = states.size();
	while (count < maxStateCount && index > 0) {
		++count;
		--index;
		lineState = (lineState << bitCount) | PackState(states[index]);
	}
	return lineState;
}

template<int bitCount, int maxStateCount, int UnpackState(int state) noexcept>
inline void UnpackLineState(int lineState, int count, std::vector<int>& states) {
	constexpr int mask = (1 << bitCount) - 1;
	count = (count > maxStateCount) ? maxStateCount : count;
	while (count > 0) {
		states.push_back(UnpackState(lineState & mask));
		lineState >>= bitCount;
		--count;
	}
}

inline int TryPopBack(std::vector<int>& states, int value = 0) {
	if (!states.empty()) {
		value = states.back();
		states.pop_back();
	}
	return value;
}

#if 0

// nested state stack on each line
using NestedStateStack = std::map<Sci_Position, std::vector<int>>;

inline void GetNestedState(const NestedStateStack& stateStack, Sci_Position line, std::vector<int>& states) {
	const auto it = stateStack.find(line);
	if (it != stateStack.end()) {
		states = it->second;
	}
}

inline void SaveNestedState(NestedStateStack& stateStack, Sci_Position line, const std::vector<int>& states) {
	if (states.empty()) {
		auto it = stateStack.find(line);
		if (it != stateStack.end()) {
			stateStack.erase(it);
		}
	} else {
		stateStack[line] = states;
	}
}

#endif

}
