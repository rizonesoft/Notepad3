// This file is part of Notepad2.
// See License.txt for details about distribution and modification.
#pragma once

#include <string>

namespace Lexilla {

template <size_t N>
constexpr size_t CStrLen([[maybe_unused]] const char (&s)[N]) noexcept {
	return N - 1;
}

constexpr size_t StrLen(const char *s) noexcept {
	return __builtin_strlen(s);
}

constexpr bool IsEmpty(const char *s) noexcept {
	return *s == '\0';
}


inline bool strequ(const char *s, const char *t) noexcept {
	return strcmp(s, t) == 0;
}

template <size_t N>
inline bool StrHasPrefix(const char *s, const char (&prefix)[N]) noexcept {
    return strncmp(s, prefix, N - 1) == 0;
}

template <size_t N>
inline bool StrHasSuffix(const char *s, size_t length, const char (&suffix)[N]) noexcept {
	return length >= N - 1 && strcmp(s + (length + 1 - N), suffix) == 0;
}


#if defined(__clang__) || defined(__GNUC__) || !defined(_MSC_BUILD)// || (_MSC_VER >= 1920)
template <size_t N>
constexpr bool StrEqual(const char *s, const char (&t)[N]) noexcept {
	return __builtin_memcmp(s, t, N) == 0;
}

template <size_t N>
constexpr bool StrStartsWith(const char *s, const char (&prefix)[N]) noexcept {
	return __builtin_memcmp(s, prefix, N - 1) == 0;
}

#else
// Visual C++ 2017 failed to optimize out string literal in memcmp().
namespace Private {

constexpr uint8_t asU1(const char ch) noexcept {
	return ch;
}

constexpr uint16_t asU2(const char *s) noexcept {
	return *(const uint16_t *)s;
}

constexpr uint32_t asU4(const char *s) noexcept {
	return *(const uint32_t *)s;
}

constexpr uint64_t asU8(const char *s) noexcept {
	return *(const uint64_t *)s;
}

#if 0
constexpr uint32_t asU3_4M1(const char *s) noexcept {
	return asU4(s) & 0x00ffffff;
}

constexpr uint32_t asU3_2P1(const char *s) noexcept {
	return asU2(s) | (asU1(s[2]) << 16);
}
#else
// based on clang memcmp() output.
constexpr uint32_t asU3_4M1(const char *s) noexcept {
	return asU4(s - 1);
}

constexpr uint32_t asU3_2P1(const char *s) noexcept {
	return asU4(s - 1);
}
#endif

template <size_t M, size_t N>
constexpr bool StringEqual(const char *s, const char (&t)[N]) noexcept {
	if constexpr (M == 2) {
		return asU2(s) == asU2(t);
	}
	if constexpr (M == 3) {
		return asU2(s) == asU2(t) && s[2] == t[2];
	}
	if constexpr (M == 4) {
		return asU4(s) == asU4(t);
	}
	if constexpr (M == 5) {
		return asU4(s) == asU4(t) && s[4] == t[4];
	}
	if constexpr (M == 6) {
		return asU4(s) == asU4(t) && asU2(s + 4) == asU2(t + 4);
	}
	if constexpr (M == 7) {
		return asU4(s) == asU4(t) && asU3_4M1(s + 4) == asU3_2P1(t + 4);
	}
#if defined(_WIN64)
	if constexpr (M == 8) {
		return asU8(s) == asU8(t);
	}
	if constexpr (M == 9) {
		return asU8(s) == asU8(t) && s[8] == t[8];
	}
	if constexpr (M == 10) {
		return asU8(s) == asU8(t) && asU2(s + 8) == asU2(t + 8);
	}
	if constexpr (M == 11) {
		return asU8(s) == asU8(t) && asU3_4M1(s + 8) == asU3_2P1(t + 8);
	}
	if constexpr (M == 12) {
		return asU8(s) == asU8(t) && asU4(s + 8) == asU4(t + 8);
	}
	if constexpr (M == 13) {
		return asU8(s) == asU8(t) && asU4(s + 8) == asU4(t + 8) && s[12] == t[12];
	}
	if constexpr (M == 14) {
		return asU8(s) == asU8(t) && asU4(s + 8) == asU4(t + 8) && asU2(s + 12) == asU2(t + 12);
	}
	if constexpr (M == 15) {
		return asU8(s) == asU8(t) && asU4(s + 8) == asU4(t + 8) && asU3_4M1(s + 12) == asU3_2P1(t + 12);
	}
	if constexpr (M == 16) {
		return asU8(s) == asU8(t) && asU8(s + 8) == asU8(t + 8);
	}
#else
	if constexpr (M == 8) {
		return asU4(s) == asU4(t) && asU4(s + 4) == asU4(t + 4);
	}
	if constexpr (M == 9) {
		return asU4(s) == asU4(t) && asU4(s + 4) == asU4(t + 4) && s[8] == t[8];
	}
	if constexpr (M == 10) {
		return asU4(s) == asU4(t) && asU4(s + 4) == asU4(t + 4) && asU2(s + 8) == asU2(t + 8);
	}
	if constexpr (M == 11) {
		return asU4(s) == asU4(t) && asU4(s + 4) == asU4(t + 4) && asU3_4M1(s + 8) == asU3_2P1(t + 8);
	}
	if constexpr (M == 12) {
		return asU4(s) == asU4(t) && asU4(s + 4) == asU4(t + 4) && asU4(s + 8) == asU4(t + 8);
	}
	if constexpr (M == 13) {
		return asU4(s) == asU4(t) && asU4(s + 4) == asU4(t + 4) && asU4(s + 8) == asU4(t + 8) && s[12] == t[12];
	}
	if constexpr (M == 14) {
		return asU4(s) == asU4(t) && asU4(s + 4) == asU4(t + 4) && asU4(s + 8) == asU4(t + 8) && asU2(s + 12) == asU2(t + 12);
	}
	if constexpr (M == 15) {
		return asU4(s) == asU4(t) && asU4(s + 4) == asU4(t + 4) && asU4(s + 8) == asU4(t + 8) && asU3_4M1(s + 12) == asU3_2P1(t + 12);
	}
	if constexpr (M == 16) {
		return asU4(s) == asU4(t) && asU4(s + 4) == asU4(t + 4) && asU4(s + 8) == asU4(t + 8) && asU4(s + 12) == asU4(t + 12);
	}
#endif
}

}

template <size_t N>
constexpr bool StrEqual(const char *s, const char (&t)[N]) noexcept {
	return Private::StringEqual<N>(s, t);
}

template <size_t N>
constexpr bool StrStartsWith(const char *s, const char (&prefix)[N]) noexcept {
	return Private::StringEqual<N - 1>(s, prefix);
}
#endif

template <size_t N>
constexpr bool StrEndsWith(const char *s, size_t length, const char (&suffix)[N]) noexcept {
	return length >= N - 1 && StrEqual(s + (length + 1 - N), suffix);
}


template <size_t N>
constexpr bool StrEqualEx(const char *s, const char (&t)[N]) noexcept {
	return __builtin_memcmp(s, t, N) == 0;
}

template <size_t N>
constexpr bool StrStartsWithEx(const char *s, const char (&prefix)[N]) noexcept {
	return __builtin_memcmp(s, prefix, N - 1) == 0;
}

template <size_t N>
constexpr bool StrEndsWithEx(const char *s, size_t length, const char (&suffix)[N]) noexcept {
	return length >= N - 1 && __builtin_memcmp(s + (length + 1 - N), suffix, N) == 0;
}


template <size_t N1>
constexpr bool StrEqualsAny(const char *s, const char (&t1)[N1]) noexcept {
	return StrEqual(s, t1);
}

template <size_t N1, size_t N2>
constexpr bool StrEqualsAny(const char *s, const char (&t1)[N1], const char (&t2)[N2]) noexcept {
	return StrEqualsAny(s, t1) || StrEqualsAny(s, t2);
}

template <size_t N1, size_t N2, size_t N3>
constexpr bool StrEqualsAny(const char *s, const char (&t1)[N1], const char (&t2)[N2], const char (&t3)[N3]) noexcept {
	return StrEqualsAny(s, t1) || StrEqualsAny(s, t2, t3);
}

template <size_t N1, size_t N2, size_t N3, size_t N4>
constexpr bool StrEqualsAny(const char *s, const char (&t1)[N1], const char (&t2)[N2], const char (&t3)[N3],
	const char (&t4)[N4]) noexcept {
	return StrEqualsAny(s, t1) || StrEqualsAny(s, t2, t3, t4);
}

template <size_t N1, size_t N2, size_t N3, size_t N4, size_t N5>
constexpr bool StrEqualsAny(const char *s, const char (&t1)[N1], const char (&t2)[N2], const char (&t3)[N3],
	const char (&t4)[N4], const char (&t5)[N5]) noexcept {
	return StrEqualsAny(s, t1) || StrEqualsAny(s, t2, t3, t4, t5);
}

template <size_t N1, size_t N2, size_t N3, size_t N4, size_t N5, size_t N6>
constexpr bool StrEqualsAny(const char *s, const char (&t1)[N1], const char (&t2)[N2], const char (&t3)[N3],
	const char (&t4)[N4], const char (&t5)[N5], const char (&t6)[N6]) noexcept {
	return StrEqualsAny(s, t1) || StrEqualsAny(s, t2, t3, t4, t5, t6);
}

template <size_t N1, size_t N2, size_t N3, size_t N4, size_t N5, size_t N6, size_t N7>
constexpr bool StrEqualsAny(const char *s, const char (&t1)[N1], const char (&t2)[N2], const char (&t3)[N3],
	const char (&t4)[N4], const char (&t5)[N5], const char (&t6)[N6], const char (&t7)[N7]) noexcept {
	return StrEqualsAny(s, t1) || StrEqualsAny(s, t2, t3, t4, t5, t6, t7);
}

template <size_t N1, size_t N2, size_t N3, size_t N4, size_t N5, size_t N6, size_t N7, size_t N8>
constexpr bool StrEqualsAny(const char *s, const char (&t1)[N1], const char (&t2)[N2], const char (&t3)[N3],
	const char (&t4)[N4], const char (&t5)[N5], const char (&t6)[N6], const char (&t7)[N7], const char (&t8)[N8]) noexcept {
	return StrEqualsAny(s, t1) || StrEqualsAny(s, t2, t3, t4, t5, t6, t7, t8);
}

template <size_t N1, size_t N2, size_t N3, size_t N4, size_t N5, size_t N6, size_t N7, size_t N8, size_t N9>
constexpr bool StrEqualsAny(const char *s, const char (&t1)[N1], const char (&t2)[N2], const char (&t3)[N3],
	const char (&t4)[N4], const char (&t5)[N5], const char (&t6)[N6], const char (&t7)[N7], const char (&t8)[N8],
	const char (&t9)[N9]) noexcept {
	return StrEqualsAny(s, t1) || StrEqualsAny(s, t2, t3, t4, t5, t6, t7, t8, t9);
}

template <size_t N1, size_t N2, size_t N3, size_t N4, size_t N5, size_t N6, size_t N7, size_t N8, size_t N9, size_t N10>
constexpr bool StrEqualsAny(const char *s, const char (&t1)[N1], const char (&t2)[N2], const char (&t3)[N3],
	const char (&t4)[N4], const char (&t5)[N5], const char (&t6)[N6], const char (&t7)[N7], const char (&t8)[N8],
	const char (&t9)[N9], const char (&t10)[N10]) noexcept {
	return StrEqualsAny(s, t1) || StrEqualsAny(s, t2, t3, t4, t5, t6, t7, t8, t9, t10);
}

}
