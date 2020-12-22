// encoding: UTF-8
#pragma once

#include "CharacterSet.h"
#include "StyleContext.h"


template <typename T, typename... Args>
constexpr bool AnyOf(T t, Args... args) noexcept {
    return ((t == args) || ...);
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
    return Scintilla::iswordchar(ch) || ch >= 0x80;
}

constexpr bool IsASpaceX(const int ch) noexcept
{
    return ((ch == ' ') || ((ch >= 0x09) && (ch <= 0x0d)));
}

constexpr bool IsADigitX(int ch, int base) noexcept
{
    if (base <= 10)
    {
        return (ch >= '0') && (ch < ('0' + base));
    }
    return ((ch >= '0') && (ch <= '9'))
           || ((ch >= 'A') && (ch < ('A' + base - 10)))
           || ((ch >= 'a') && (ch < ('a' + base - 10)));
}

constexpr bool IsAHexDigit(int ch) noexcept
{
    return ((ch >= '0') && (ch <= '9'))
           || ((ch >= 'A') && (ch <= 'F'))
           || ((ch >= 'a') && (ch <= 'f'));
}

constexpr bool IsABlankOrTab(const int ch) noexcept
{
    return ((ch == ' ') || (ch == '\t'));
}

constexpr bool IsALetter(const int ch) noexcept
{
    // 97 to 122 || 65 to 90
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

constexpr bool IsLineBreak(const int ch) noexcept
{
    return ((ch == '\n') || (ch == '\r') || (ch == '\0'));
}

constexpr bool IsIdentifierChar(int ch) noexcept {
    return Scintilla::IsAlphaNumeric(ch) || ch == '_';
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

constexpr int IsNumber(const Scintilla::StyleContext& sc)
{
    return  ((sc.ch >= '0') && (sc.ch <= '9')) ||
            (((sc.ch == '+') || (sc.ch == '-')) && ((sc.chNext >= '0') && (sc.chNext <= '9')));
}

constexpr bool IsNumberStart(int ch, int chNext) noexcept {
    return Scintilla::IsADigit(ch) || (ch == '.' && Scintilla::IsADigit(chNext));
}

constexpr bool IsNumberContinue(int chPrev, int ch, int chNext) noexcept {
    return ((ch == '+' || ch == '-') && (chPrev == 'e' || chPrev == 'E'))
        || (ch == '.' && chNext != '.');
}

constexpr bool IsNumberContinueEx(int chPrev, int ch, int chNext) noexcept {
    return ((ch == '+' || ch == '-') && (chPrev == 'e' || chPrev == 'E' || chPrev == 'p' || chPrev == 'P'))
        || (ch == '.' && chNext != '.');
}


constexpr bool IsDecimalNumber(int chPrev, int ch, int chNext) noexcept {
    return IsIdentifierChar(ch) || IsNumberContinue(chPrev, ch, chNext);
}

constexpr bool IsDecimalNumberEx(int chPrev, int ch, int chNext) noexcept {
    return IsIdentifierChar(ch) || IsNumberContinueEx(chPrev, ch, chNext);
}


constexpr int IsNumHex(const Scintilla::StyleContext& sc) noexcept
{
    return (sc.ch == '0') && (sc.chNext == 'x') || (sc.chNext == 'X');
}

constexpr int IsNumBinary(const Scintilla::StyleContext& sc) noexcept
{
    return (sc.ch == '0') && (sc.chNext == 'b') || (sc.chNext == 'B');
}

constexpr int IsNumOctal(const Scintilla::StyleContext& sc)
{
    return (sc.ch == '0') && (sc.chNext == 'o') || (sc.chNext == 'O');
}

inline int IsNumExponent(const Scintilla::StyleContext& sc)
{
    return Scintilla::IsADigit(sc.ch) && ((sc.chNext == 'e') || (sc.chNext == 'E'));
}

inline void TrimIdentifier(const char* input, char* output)
{
    size_t j = 0;
    for (size_t i = 0; input[i] != '\0'; ++i)
    {
        if (!IsASpaceX(input[i]))
        {
            output[j++] = input[i];
        }
    }
    output[j] = '\0';
}
