// Scintilla source code edit control
/** @file LexAccessor.cxx
 ** Interfaces between Scintilla and lexers.
 **/
// Copyright 1998-2010 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.
#include <cstdint>
#include <cassert>
#include <cstring>

#include <string>
#include <string_view>
#include <algorithm>

#include "ILexer.h"
#include "Scintilla.h"

#include "LexAccessor.h"
#include "CharacterSet.h"

using namespace Lexilla;

namespace Lexilla {

 LexAccessor::LexAccessor(Scintilla::IDocument *pAccess_) :
	pAccess(pAccess_), startPos(PTRDIFF_MAX), endPos(0),
	codePage(pAccess->CodePage()),
	encodingType(EncodingType::eightBit),
	lenDoc(pAccess->Length()),
	validLen(0),
	startSeg(0), startPosStyling(0),
	documentVersion(pAccess->Version()) {
	// Prevent warnings by static analyzers about uninitialized buf and styleBuf.
	buf[0] = 0;
	styleBuf[0] = 0;
	switch (codePage) {
	case 65001:
		encodingType = EncodingType::unicode;
		break;
	case 932:
	case 936:
	case 949:
	case 950:
	case 1361:
		encodingType = EncodingType::dbcs;
		break;
	default:
		break;
	}
}

bool LexAccessor::Match(Sci_Position pos, std::string_view sv) {
	for (unsigned int i = 0; i < sv.size(); i++) {
		if (sv[i] != SafeGetCharAt(pos + i)) {
			return false;
		}
	}
	return true;
}

bool LexAccessor::MatchIgnoreCase(Sci_Position pos, const char *s) {
	assert(s);
	for (; *s; s++, pos++) {
		if (*s != MakeLowerCase(SafeGetCharAt(pos))) {
			return false;
		}
	}
	return true;
}

void LexAccessor::GetRange(Sci_PositionU startPos_, Sci_PositionU endPos_, char *s, Sci_PositionU len) const {
	assert(s);
	assert(startPos_ <= endPos_ && len != 0);
	memset(s, '\0', len);
	endPos_ = std::min(endPos_, startPos_ + len - 1);
	endPos_ = std::min(endPos_, static_cast<Sci_PositionU>(lenDoc));
	len = endPos_ - startPos_;
	if (startPos_ >= static_cast<Sci_PositionU>(startPos) && endPos_ <= static_cast<Sci_PositionU>(endPos)) {
		const char * const p = buf + (startPos_ - startPos);
		memcpy(s, p, len);
	} else {
		pAccess->GetCharRange(s, startPos_, len);
	}
}

void LexAccessor::GetRangeLowered(Sci_PositionU startPos_, Sci_PositionU endPos_, char *s, Sci_PositionU len) const {
	assert(s);
	GetRange(startPos_, endPos_, s, len);
	while (*s) {
		if (*s >= 'A' && *s <= 'Z') {
			*s += 'a' - 'A';
		}
		++s;
	}
}

std::string LexAccessor::GetRange(Sci_PositionU startPos_, Sci_PositionU endPos_) const {
	assert(startPos_ <= endPos_);
	endPos_ = std::min(endPos_, static_cast<Sci_PositionU>(lenDoc));
	const Sci_PositionU len = endPos_ - startPos_;
	std::string s(len, '\0');
	GetRange(startPos_, endPos_, s.data(), len + 1);
	return s;
}

std::string LexAccessor::GetRangeLowered(Sci_PositionU startPos_, Sci_PositionU endPos_) const {
	assert(startPos_ <= endPos_);
	endPos_ = std::min(endPos_, static_cast<Sci_PositionU>(lenDoc));
	const Sci_PositionU len = endPos_ - startPos_;
	std::string s(len, '\0');
	GetRangeLowered(startPos_, endPos_, s.data(), len + 1);
	return s;
}

void LexAccessor::SetLevelIfDifferent(Sci_Position line, int level) {
	if (level != pAccess->GetLevel(line)) {
		pAccess->SetLevel(line, level);
	}
}

int FoldLevelFlags(int levelLine, int levelNext, bool white, bool headerPermitted) noexcept {
	int flags = 0;
	if (white) {
		flags |= SC_FOLDLEVELWHITEFLAG;
	}
	if ((levelLine < levelNext) && headerPermitted) {
		flags |= SC_FOLDLEVELHEADERFLAG;
	}
	return flags;
}

}
