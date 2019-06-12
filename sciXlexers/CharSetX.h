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

constexpr bool IsALetter(const int ch) noexcept {
  // 97 to 122 || 65 to 90
  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

constexpr bool IsLineBreak(const int ch) noexcept {
  return ((ch == '\n') || (ch == '\r'));
}

constexpr int IsNumHex(const Scintilla::StyleContext& sc) noexcept {
  return (sc.chNext == 'x') || (sc.chNext == 'X');
}

constexpr int IsNumBinary(const Scintilla::StyleContext& sc) noexcept {
  return (sc.chNext == 'b') || (sc.chNext == 'B');
}


inline int IsNumOctal(const Scintilla::StyleContext& sc) {
  return Scintilla::IsADigit(sc.chNext) || (sc.chNext == 'o');
}

inline bool IsAIdentifierChar(const int ch) {
  return (Scintilla::IsAlphaNumeric(ch) || ch == '_' || ch == '.');
}



#endif //_CHARSETX_H_
