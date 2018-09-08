// Scintilla source code edit control
/** @file CharacterSet.cxx
 ** Simple case functions for ASCII.
 ** Lexer infrastructure.
 **/
// Copyright 1998-2010 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <cstdlib>
#include <cassert>
#include <cstring>

#include "CharacterSet.h"

using namespace Scintilla;

namespace Scintilla {

CharacterSet::CharacterSet(setBase base, const char *initialSet, int size_, bool valueAfter_) {
	size = size_;
	valueAfter = valueAfter_;
	bset = new bool[size]();
	AddString(initialSet);
	if (base & setLower)
		AddString("abcdefghijklmnopqrstuvwxyz");
	if (base & setUpper)
		AddString("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	if (base & setDigits)
		AddString("0123456789");
}

#if 0
int CompareCaseInsensitive(const char *a, const char *b) noexcept {
	while (*a && *b) {
		if (*a != *b) {
			const char upperA = MakeUpperCase(*a);
			const char upperB = MakeUpperCase(*b);
			if (upperA != upperB)
				return upperA - upperB;
		}
		a++;
		b++;
	}
	// Either *a or *b is nul
	return *a - *b;
}

int CompareNCaseInsensitive(const char *a, const char *b, size_t len) noexcept {
	while (*a && *b && len) {
		if (*a != *b) {
			const char upperA = MakeUpperCase(*a);
			const char upperB = MakeUpperCase(*b);
			if (upperA != upperB)
				return upperA - upperB;
		}
		a++;
		b++;
		len--;
	}
	if (len == 0)
		return 0;
	else
		// Either *a or *b is nul
		return *a - *b;
}
#endif
}
