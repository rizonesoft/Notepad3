// encoding: UTF-8

#include <cassert>
#include <string>

#include "Scintilla.h"

#include "ILexer.h"
#include "LexAccessor.h"
//#include "Accessor.h"

#include "CharSetX.h"
#include "StringUtils.h"
#include "LexerUtils.h"

//using namespace Lexilla;

namespace Lexilla {


    void BacktrackToStart(const LexAccessor& styler, int stateMask, Sci_PositionU& startPos, Sci_Position& lengthDoc, int& initStyle) noexcept {
        const Sci_Position currentLine = styler.GetLine(startPos);
        if (currentLine != 0) {
            Sci_Position line = currentLine - 1;
            int lineState = styler.GetLineState(line);
            while ((lineState & stateMask) != 0 && line != 0) {
                --line;
                lineState = styler.GetLineState(line);
            }
            if ((lineState & stateMask) == 0) {
                ++line;
            }
            if (line != currentLine) {
                const Sci_Position endPos = startPos + lengthDoc;
                startPos = (line == 0) ? 0 : styler.LineStart(line);
                lengthDoc = endPos - startPos;
                initStyle = (startPos == 0) ? 0 : styler.StyleAt(startPos - 1);
            }
        }
    }

    void LookbackNonWhite(LexAccessor& styler, Sci_PositionU startPos, int maxSpaceStyle, int& chPrevNonWhite, int& stylePrevNonWhite) {
        Sci_PositionU back = startPos - 1;
        while (back) {
            const int style = styler.StyleAt(back);
            if (style > maxSpaceStyle) {
                chPrevNonWhite = static_cast<unsigned char>(styler.SafeGetCharAt(back, '\0'));
                stylePrevNonWhite = style;
                break;
            }
            --back;
        }
    }

    Sci_PositionU CheckBraceOnNextLine(LexAccessor& styler, Sci_Position line, int operatorStyle, int maxSpaceStyle, int ignoreStyle) noexcept {
        // check brace on next line
        Sci_Position startPos = styler.LineStart(line + 1);
        Sci_Position bracePos = startPos;
        char ch;
        while (IsASpaceOrTab(ch = styler[bracePos])) {
            ++bracePos;
        }
        if (ch != '{') {
            return 0;
        }

        int style = styler.StyleAt(bracePos);
        if (style != operatorStyle) {
            return 0;
        }

        // check current line
        Sci_Position endPos = startPos - 1;
        startPos = styler.LineStart(line);

        // ignore current line, e.g. current line is preprocessor.
        if (ignoreStyle) {
            while (startPos < endPos) {
                style = styler.StyleAt(startPos);
                if (style > maxSpaceStyle) {
                    break;
                }
                ++startPos;
            }
            if (style == ignoreStyle) {
                return 0;
            }
        }

        while (endPos >= startPos) {
            style = styler.StyleAt(endPos);
            if (style > maxSpaceStyle) {
                break;
            }
            --endPos;
        }
        if (endPos < startPos) {
            // current line is empty or comment
            return 0;
        }
        if (style == operatorStyle) {
            ch = styler[endPos];
            /*
            function(param)
                { body }

            if (expr)
                { body }
            else
                { body }

            switch (expr)
                { body }

            class name<T>
                { body }

            var name =
                { body }
            var name = new type[]
                { body }

            case constant:
                { body }

            ActionScript:
                function name(param:*):*
                    { body }
            C++:
                [lambda-capture]
                    { body }
            C#:
                => { lambda }
            Java:
                -> { lambda }
            Objective-C:
                ^{ block }
            Rust:
                fn name() -> optional?
                    { body }
            Scala:
                class name[T]
                    { body }
            */
            if (!AnyOf(ch, ')', '>', '=', ':', ']', '^', '?', '*')) {
                return 0;
            }
        }

        /*
            class name
                { body }

            try
                { body }
            catch (exception)
                { body }
        */
        return bracePos;
    }

    constexpr bool IsTaskMarkerPrev(int chPrev) noexcept {
        return chPrev <= 32 || AnyOf(chPrev, '/', '*', '!', '#');
    }

    constexpr bool IsTaskMarkerStart(int visibleChars, int visibleCharsBefore, int chPrev, int ch, int chNext) noexcept {
        return (visibleChars == 0 || (visibleChars <= visibleCharsBefore + 3 && IsTaskMarkerPrev(chPrev)))
            && IsUpperCase(ch) && IsUpperCase(chNext);
    }

    bool HighlightTaskMarker(StyleContext& sc, int& visibleChars, int visibleCharsBefore, int markerStyle) {

        if (IsTaskMarkerStart(visibleChars, visibleCharsBefore, sc.chPrev, sc.ch, sc.chNext)) {
            Sci_PositionU pos = sc.currentPos + 2;
            int ch;
            while (IsUpperCase(ch = sc.GetRelativeCharacter(pos))) {
                ++pos;
            }

            bool marker = false;
            const int len = static_cast<int>(pos - sc.currentPos);
            if (ch == ':' || ch == '(') {
                // highlight first uppercase word after comment characters as task marker.
                marker = true;
            }
            else if (ch <= 32 && len >= 3 && len < 16 && AnyOf(sc.ch, 'T', 'F', 'N', 'X')) {
                char s[8];
                sc.GetCurrent(s, sizeof(s));
                marker = StrEqualsAny(s, "BUG", "FIXME", "HACK", "NOTE", "TBD", "TODO", "XXX")
                    || StrStartsWith(s, "NOLINT"); // clang-tidy: NOLINT, NOLINTNEXTLINE
            }

            visibleChars += len;
            const int state = sc.state;
            sc.SetState(markerStyle);
            sc.Forward(len);
            if (marker) {
                sc.SetState(state);
            }
            else {
                sc.ChangeState(state);
            }
            return true;
        }
        return false;
    }

}

