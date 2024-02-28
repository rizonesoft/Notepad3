#pragma once

#include <vector>

#include "CharSetX.h"

namespace Lexilla {


// TODO: change packed line state to NestedStateStack (convert lexer to class).

    template<int valueBit, int maxStateCount, int countBit, int baseStyle>
    int PackLineState(const std::vector<int>& states) noexcept {
        constexpr size_t countMask = (1 << countBit) - 1;
        size_t index = states.size();
        int count = static_cast<int>(sci::min(index, countMask));
        int lineState = count;
        lineState <<= countBit;
        count = sci::min(count, maxStateCount);
        while (count != 0) {
            --count;
            --index;
            int state = states[index];
            if (state) {
                state -= baseStyle;
            }
            lineState = (lineState << valueBit) | state;
        }
        return lineState;
    }

    template<int valueBit, int maxStateCount, int countBit, int baseStyle>
    void UnpackLineState(int lineState, std::vector<int>& states) {
        constexpr int valueMask = (1 << valueBit) - 1;
        constexpr int countMask = (1 << countBit) - 1;
        int count = lineState & countMask;
        lineState >>= countBit;
        count = sci::min(count, maxStateCount);
        while (count != 0) {
            int state = lineState & valueMask;
            if (state) {
                state += baseStyle;
            }
            states.push_back(state);
            lineState >>= valueBit;
            --count;
        }
    }

    enum {
        DefaultNestedStateValueBit = 3,
        DefaultMaxNestedStateCount = 4,
        DefaultNestedStateCountBit = 3,
        DefaultNestedStateBaseStyle = 10,
    };


    inline int PackLineState(const std::vector<int>& states) noexcept {
        return PackLineState<DefaultNestedStateValueBit, DefaultMaxNestedStateCount, DefaultNestedStateCountBit, DefaultNestedStateBaseStyle>(states);
    }

    inline void UnpackLineState(int lineState, std::vector<int>& states) {
        UnpackLineState<DefaultNestedStateValueBit, DefaultMaxNestedStateCount, DefaultNestedStateCountBit, DefaultNestedStateBaseStyle>(lineState, states);
    }

    inline int TryTakeAndPop(std::vector<int>& states, int value = 0) {
        if (!states.empty()) {
            value = states.back();
            states.pop_back();
        }
        return value;
    }

    inline int TakeAndPop(std::vector<int>& states) {
        const int value = states.back();
        states.pop_back();
        return value;
    }

    inline int TryPopAndPeek(std::vector<int>& states, int value = 0) {
        if (!states.empty()) {
            states.pop_back();
            if (!states.empty()) {
                value = states.back();
            }
        }
        return value;
    }


    inline int GetDocNextChar(LexAccessor& styler, const StyleContext& sc, bool ignoreCurrent = false) noexcept {
        if (!ignoreCurrent && !IsWhiteSpace(sc.ch)) {
            return sc.ch;
        }
        if (!IsWhiteSpace(sc.chNext)) {
            return sc.chNext;
        }
        Sci_Position pos = sc.currentPos + 2;
        do {
            const char ch = styler.SafeGetCharAt(pos, '\0');
            if (!IsWhiteSpace(ch)) {
                return ch;
            }
            ++pos;
        } while (true);
    }


    inline int GetLineNextChar(LexAccessor& styler, const StyleContext& sc, bool ignoreCurrent = false) noexcept {
        if (!ignoreCurrent && !IsWhiteSpace(sc.ch)) {
            return sc.ch;
        }
        if (sc.currentPos + 1 == static_cast<Sci_PositionU>(sc.lineStartNext)) {
            return '\0';
        }
        if (!IsWhiteSpace(sc.chNext)) {
            return sc.chNext;
        }
        Sci_Position pos = sc.currentPos + 2;
        while (pos < sc.lineStartNext) {
            const char ch = styler.SafeGetCharAt(pos, '\0');
            if (!IsWhiteSpace(ch)) {
                return ch;
            }
            ++pos;
        }
        return '\0';
    }


    void BacktrackToStart(const LexAccessor& styler, int stateMask, Sci_PositionU& startPos, Sci_Position& lengthDoc, int& initStyle) noexcept;
    void LookbackNonWhite(LexAccessor& styler, Sci_PositionU startPos, int maxSpaceStyle, int& chPrevNonWhite, int& stylePrevNonWhite);
    Sci_PositionU CheckBraceOnNextLine(LexAccessor& styler, Sci_Position line, int operatorStyle, int maxSpaceStyle, int ignoreStyle = 0) noexcept;

    bool HighlightTaskMarker(StyleContext& sc, int& visibleChars, int visibleCharsBefore, int markerStyle);

#if 0

    // nested state stack on each line
    using NestedStateStack = std::map<Sci_Line, std::vector<int>>;

    inline void GetNestedState(const NestedStateStack& stateStack, Sci_Line line, std::vector<int>& states) {
        const auto it = stateStack.find(line);
        if (it != stateStack.end()) {
            states = it->second;
        }
    }

    inline void SaveNestedState(NestedStateStack& stateStack, Sci_Line line, const std::vector<int>& states) {
        if (states.empty()) {
            auto it = stateStack.find(line);
            if (it != stateStack.end()) {
                stateStack.erase(it);
            }
        }
        else {
            stateStack[line] = states;
        }
    }

#endif

}
