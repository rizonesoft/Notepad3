// encoding: UTF-8
#pragma once
#ifndef _CHARSETX_H_
#define _CHARSETX_H_

#include "StyleContext.h"
#include "CharacterSet.h"

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

constexpr bool IsASpaceX(const int ch) noexcept {
  return ((ch == ' ') || ((ch >= 0x09) && (ch <= 0x0d)));
}

constexpr bool IsABlankOrTabX(const int ch) noexcept {
  return ((ch == ' ') || (ch == '\t'));
}

constexpr bool IsADigitX(const int ch) noexcept {
  return ((ch >= '0') && (ch <= '9'));
}

constexpr bool IsALetter(const int ch) noexcept {
  // 97 to 122 || 65 to 90
  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

constexpr bool IsLineBreak(const int ch) noexcept {
  return ((ch == '\n') || (ch == '\r') || (ch == '\0'));
}

inline int IsNumber(const Scintilla::StyleContext& sc) {
  return  Scintilla::IsADigit(sc.ch) || 
          (((sc.ch == '+') || (sc.ch == '-')) && Scintilla::IsADigit(sc.chNext));
}

constexpr int IsNumHex(const Scintilla::StyleContext& sc) noexcept {
  return (sc.ch == '0') && (sc.chNext == 'x') || (sc.chNext == 'X');
}

constexpr int IsNumBinary(const Scintilla::StyleContext& sc) noexcept {
  return (sc.ch == '0') && (sc.chNext == 'b') || (sc.chNext == 'B');
}

inline int IsNumOctal(const Scintilla::StyleContext& sc) {
  return (sc.ch == '0') && (sc.chNext == 'o') || (sc.chNext == 'O');
}

inline int IsNumExponent(const Scintilla::StyleContext& sc) {
  return Scintilla::IsADigit(sc.ch) && ((sc.chNext == 'e') || (sc.chNext == 'E'));
}

inline void TrimIdentifier(const char* input, char* output)
{
  size_t j = 0;
  for (size_t i = 0; input[i] != '\0'; ++i) {
    if (!IsASpaceX(input[i])) {
      output[j++] = input[i];
    }
  }
  output[j] = '\0';
}


#endif //_CHARSETX_H_
