// Scintilla source code edit control
/** @file UniqueString.h
 ** Define UniqueString, a unique_ptr based string type for storage in containers
 ** and an allocator for UniqueString.
 **/
// Copyright 2017 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef UNIQUESTRING_H
#define UNIQUESTRING_H

#ifdef SCI_NAMESPACE
namespace Scintilla {
#endif

using UniqueString = std::unique_ptr<const char[]>;

/// Equivalent to strdup but produces a std::unique_ptr<const char[]> allocation to go
/// into collections.
inline UniqueString UniqueStringCopy(const char *text) {
	if (!text) {
		return UniqueString();
	}
	const size_t len = strlen(text);
#if (_MSC_VER >= 1900)
	char *sNew = new char[len + 1];
	std::copy(text, text + len + 1, sNew);
	return UniqueString(sNew);
#else
	// this works for VS2013 (vc120) 
	auto buf = std::make_unique<char[]>(len + 1);
	std::copy(text, text + len + 1, buf.get());
	std::unique_ptr<const char[]> ret;
	ret.reset(const_cast<const char*>(buf.release()));
	return std::move(ret);
#endif
}

#ifdef SCI_NAMESPACE
}
#endif

#endif
