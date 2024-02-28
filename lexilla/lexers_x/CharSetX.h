// encoding: UTF-8
#pragma once

#include <string>

#include "StyleContext.h"
#include "CharacterSet.h"

namespace sci {

    template <typename T>
    constexpr T min(T x, T y) noexcept {
        return (x < y) ? x : y;
    }

    template <typename T>
    constexpr T max(T x, T y) noexcept {
        return (x > y) ? x : y;
    }
}


#if defined(_INC_STRING)
template <typename... Args>
inline bool EqualsAny(const char* s, Args... args) noexcept {
    return ((::strcmp(s, args) == 0) || ...);
}
#endif


// Functions for classifying characters

// *** Methods from "scintilla\lexlib\CharacterSet.h" ***
//- IsASpace(int ch);
//- IsASpaceOrTab(int ch);
//- IsADigit(int ch);
//- IsADigit(int ch, int base);
//- IsASCII(int ch);
//- IsLowerCase(int ch);
//- IsUpperCase(int ch);
//- IsUpperOrLowerCase(int ch);
//- IsAlphaNumeric(int ch);

constexpr int abs_i(const int i) noexcept
{
    return ((i < 0) ? (0 - i) : (0 + i));
}

constexpr bool IsWordCharEx(int ch) noexcept {
    return Lexilla::iswordchar(ch) || ch >= 0x80;
}

constexpr bool IsWhiteSpace(const int ch) noexcept
{
    return Lexilla::isspacechar(ch);
}

constexpr bool IsHexDigit(int ch) noexcept
{
	return Lexilla::IsAHeXDigit(ch);
}

constexpr bool IsALetter(const int ch) noexcept
{
    // 97 to 122 || 65 to 90
	return Lexilla::IsUpperOrLowerCase(ch);
}

constexpr bool IsEOLChar(const int ch) noexcept
{
	return (ch == '\n' || ch == '\r');
}

constexpr bool IsNewline(const int ch) noexcept
{
	// sc.GetRelative(i) returns '\0' if out of range
	return IsEOLChar(ch) || (ch == '\0');
}

constexpr bool IsAGraphic(int ch) noexcept
{
	// excludes C0 control characters and whitespace
	return ch > 32 && ch < 127;
}

constexpr bool IsGraphic(int ch) noexcept
{
	// excludes C0 control characters and whitespace
	return ch > 32 && ch != 127;
}

constexpr bool IsJumpLabelPrevChar(int chPrev) noexcept {
	return chPrev == ';' || chPrev == '{' || chPrev == '}';
}

constexpr bool IsLineEndUTF8(const unsigned char ch0, 
                             const unsigned char ch1,
                             const unsigned char ch2) noexcept
{
    return ((ch0 == '\n') || ((ch0 == '\r') && (ch1 == '\n')) || 
            (((ch0 == 0xe2) && (ch1 == 0x80) && ((ch2 == 0xa8) || (ch2 == 0xa9))) || 
             ((ch1 == 0xc2) && (ch2 == 0x85)))
           );
}

constexpr bool IsIdentifierChar(int ch) noexcept {
    return Lexilla::IsAlphaNumeric(ch) || ch == '_';
}

constexpr bool IsIdentifierCharEx(int ch) noexcept {
    return IsIdentifierChar(ch) || ch >= 0x80;
}

constexpr bool IsIdentifierStart(int ch) noexcept {
    return IsALetter(ch) || ch == '_';
}

constexpr bool IsIdentifierStartEx(int ch) noexcept {
    return IsIdentifierStart(ch) || ch >= 0x80;
}

constexpr int IsNumber(const Lexilla::StyleContext& sc)
{
    return  ((sc.ch >= '0') && (sc.ch <= '9')) ||
            (((sc.ch == '+') || (sc.ch == '-')) && ((sc.chNext >= '0') && (sc.chNext <= '9')));
}

constexpr bool IsNumberStart(int ch, int chNext) noexcept {
    return Lexilla::IsADigit(ch) || (ch == '.' && Lexilla::IsADigit(chNext));
}

constexpr bool IsNumberContinue(int chPrev, int ch, int chNext) noexcept {
    return ((ch == '+' || ch == '-') && (chPrev == 'e' || chPrev == 'E'))
        || (ch == '.' && chNext != '.');
}

constexpr bool IsNumberContinueEx(int chPrev, int ch, int chNext) noexcept {
    return ((ch == '+' || ch == '-') && Lexilla::AnyOf(chPrev, 'e', 'E', 'p', 'P'))
        || (ch == '.' && chNext != '.');
}


constexpr bool IsDecimalNumber(int chPrev, int ch, int chNext) noexcept {
    return IsIdentifierChar(ch) || IsNumberContinue(chPrev, ch, chNext);
}

constexpr bool IsDecimalNumberEx(int chPrev, int ch, int chNext) noexcept {
    return IsIdentifierChar(ch) || IsNumberContinueEx(chPrev, ch, chNext);
}


constexpr int IsNumHex(const Lexilla::StyleContext& sc) noexcept
{
    return (sc.ch == '0') && (sc.chNext == 'x') || (sc.chNext == 'X');
}

constexpr int IsNumBinary(const Lexilla::StyleContext& sc) noexcept
{
    return (sc.ch == '0') && (sc.chNext == 'b') || (sc.chNext == 'B');
}

constexpr int IsNumOctal(const Lexilla::StyleContext& sc)
{
    return (sc.ch == '0') && (sc.chNext == 'o') || (sc.chNext == 'O');
}

// characters can follow jump `label:`, based on Swift's document Labeled Statement at
// https://docs.swift.org/swift-book/ReferenceManual/Statements.html#grammar_labeled-statement
// good coding style should place left aligned label on it's own line.
constexpr bool IsJumpLabelNextChar(int chNext) noexcept {
    // own line, comment, for, foreach, while, do, if, switch, repeat
    // TODO: match each word exactly like HighlightTaskMarker().
    return Lexilla::AnyOf(chNext, '\0', '/', 'f', 'w', 'd', 'i', 's', 'r');
}

inline int IsNumExponent(const Lexilla::StyleContext& sc)
{
    return Lexilla::IsADigit(sc.ch) && ((sc.chNext == 'e') || (sc.chNext == 'E'));
}

inline void TrimIdentifier(const char* input, char* output)
{
    size_t j = 0;
    for (size_t i = 0; input[i] != '\0'; ++i)
    {
        if (!IsWhiteSpace(input[i]))
        {
            output[j++] = input[i];
        }
    }
    output[j] = '\0';
}
