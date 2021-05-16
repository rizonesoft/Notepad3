// sktoolslib - common files for SK tools

// Copyright (C) 2012-2021 - Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//

#pragma once

#include <string>
#include <algorithm>
#include <functional>
#include <memory>

/**
 * @ingroup Utils
 * Performs a wild card compare of two strings.
 * @param wild the wild card string
 * @param string the string to compare the wild card to
 * @return TRUE if the wild card matches the string, 0 otherwise
 * @par example
 * @code
 * if (strwildcmp("bl?hblah.*", "bliblah.jpeg"))
 *  printf("success\n");
 * else
 *  printf("not found\n");
 * if (strwildcmp("bl?hblah.*", "blabblah.jpeg"))
 *  printf("success\n");
 * else
 *  printf("not found\n");
 * @endcode
 * The output of the above code would be:
 * @code
 * success
 * not found
 * @endcode
 */
int strwildcmp(const char* wild, const char* string);
int wcswildcmp(const wchar_t* wild, const wchar_t* string);
int wcswildicmp(const wchar_t* wild, const wchar_t* string);

bool WriteAsciiStringToClipboard(const wchar_t* sClipdata, HWND hOwningWnd);
void SearchReplace(std::wstring& str, const std::wstring& toreplace, const std::wstring& replacewith);
void SearchReplace(std::string& str, const std::string& toreplace, const std::string& replacewith);

void SearchRemoveAll(std::string& str, const std::string& toremove);
void SearchRemoveAll(std::wstring& str, const std::wstring& toremove);

// append = true as the default: a default value should never lose data!
template <typename Container>
void stringtok(Container& container, const std::wstring& in, bool trim,
               const wchar_t* const delimiters = L"|", bool append = true)
{
    const std::wstring::size_type len = in.length();
    std::wstring::size_type       i   = 0;
    if (!append)
        container.clear();

    while (i < len)
    {
        if (trim)
        {
            // eat leading whitespace
            i = in.find_first_not_of(delimiters, i);
            if (i == std::wstring::npos)
                return; // nothing left but white space
        }

        // find the end of the token
        std::wstring::size_type j = in.find_first_of(delimiters, i);

        // push token
        if (j == std::wstring::npos)
        {
            if constexpr (std::is_same_v<typename Container::value_type, std::wstring>)
            {
                container.push_back(in.substr(i));
            }
            else
            {
                container.push_back(static_cast<typename Container::value_type>(_wtoi64(in.substr(i).c_str())));
            }
            return;
        }
        else
        {
            if constexpr (std::is_same_v<typename Container::value_type, std::wstring>)
            {
                container.push_back(in.substr(i, j - i));
            }
            else
            {
                container.push_back(static_cast<typename Container::value_type>(_wtoi64(in.substr(i, j - i).c_str())));
            }
        }

        // set up for next loop
        i = j + 1;
    }
}

template <typename Container>
void stringtokset(Container& container, const std::wstring& in, bool trim,
                  const wchar_t* const delimiters = L"|", bool append = false)
{
    const std::wstring::size_type len = in.length();
    std::wstring::size_type       i   = 0;
    if (!append)
        container.clear();

    while (i < len)
    {
        if (trim)
        {
            // eat leading whitespace
            i = in.find_first_not_of(delimiters, i);
            if (i == std::wstring::npos)
                return; // nothing left but white space
        }

        // find the end of the token
        std::wstring::size_type j = in.find_first_of(delimiters, i);

        // push token
        if (j == std::wstring::npos)
        {
            if constexpr (std::is_same_v<typename Container::value_type, std::wstring>)
            {
                container.insert(in.substr(i));
            }
            else
            {
                container.insert(static_cast<typename Container::value_type>(_wtoi64(in.substr(i).c_str())));
            }
            return;
        }
        else
        {
            if constexpr (std::is_same_v<typename Container::value_type, std::wstring>)
            {
                container.insert(in.substr(i, j - i));
            }
            else
            {
                container.insert(static_cast<typename Container::value_type>(_wtoi64(in.substr(i, j - i).c_str())));
            }
        }

        // set up for next loop
        i = j + 1;
    }
}

// append = true as the default: a default value should never lose data!
template <typename Container>
void stringtok(Container& container, const std::string& in, bool trim,
               const char* const delimiters = "|", bool append = true)
{
    const std::string::size_type len = in.length();
    std::string::size_type       i   = 0;
    if (!append)
        container.clear();

    while (i < len)
    {
        if (trim)
        {
            // eat leading whitespace
            i = in.find_first_not_of(delimiters, i);
            if (i == std::string::npos)
                return; // nothing left but white space
        }

        // find the end of the token
        std::string::size_type j = in.find_first_of(delimiters, i);

        // push token
        if (j == std::string::npos)
        {
            if constexpr (std::is_same_v<typename Container::value_type, std::string>)
            {
                container.push_back(in.substr(i));
            }
            else
            {
                container.push_back(static_cast<typename Container::value_type>(_atoi64(in.substr(i).c_str())));
            }
            return;
        }
        else
        {
            if constexpr (std::is_same_v<typename Container::value_type, std::string>)
            {
                container.push_back(in.substr(i, j - i));
            }
            else
            {
                container.push_back(static_cast<typename Container::value_type>(_atoi64(in.substr(i, j - i).c_str())));
            }
        }

        // set up for next loop
        i = j + 1;
    }
}

template <typename Container>
void stringtokset(Container& container, const std::string& in, bool trim,
                  const char* const delimiters = "|", bool append = false)
{
    const std::string::size_type len = in.length();
    std::string::size_type       i   = 0;
    if (!append)
        container.clear();

    while (i < len)
    {
        if (trim)
        {
            // eat leading whitespace
            i = in.find_first_not_of(delimiters, i);
            if (i == std::string::npos)
                return; // nothing left but white space
        }

        // find the end of the token
        std::string::size_type j = in.find_first_of(delimiters, i);

        // push token
        if (j == std::string::npos)
        {
            if constexpr (std::is_same_v<typename Container::value_type, std::string>)
            {
                container.insert(in.substr(i));
            }
            else
            {
                container.insert(static_cast<typename Container::value_type>(_atoi64(in.substr(i).c_str())));
            }
            return;
        }
        else
        {
            if constexpr (std::is_same_v<typename Container::value_type, std::string>)
            {
                container.insert(in.substr(i, j - i));
            }
            else
            {
                container.insert(static_cast<typename Container::value_type>(_atoi64(in.substr(i, j - i).c_str())));
            }
        }

        // set up for next loop
        i = j + 1;
    }
}

// ReSharper disable once CppInconsistentNaming
template <typename T>
std::wstring to_bit_wstring(T number, bool trimSignificantClearBits)
{
    // Unsigned version of type given.
    using UT = typename std::make_unsigned<T>::type;
    UT                                           one  = 1;
    UT                                           zero = 0;
    UT                                           uNumber;
    uNumber            = UT(number);
    const int    nBits = std::numeric_limits<UT>::digits;
    std::wstring bs;
    bool         seenSetBit = false;
    for (int bn = nBits - 1; bn >= 0; --bn)
    {
        UT   mask  = one << bn;
        bool isSet = (uNumber & mask) != zero;
        if (trimSignificantClearBits && !seenSetBit && !isSet)
            continue;
        bs += isSet ? '1' : '0';
        if (isSet)
            seenSetBit = true;
    }
    return bs;
}

// ReSharper disable once CppInconsistentNaming
template <typename T>
std::string to_bit_string(T number, bool trimSignificantClearBits)
{
    // Unsigned version of type given.
    using UT = typename std::make_unsigned<T>::type;
    UT                                           one  = 1;
    UT                                           zero = 0;
    UT                                           uNumber;
    uNumber           = UT(number);
    const int   nBits = std::numeric_limits<UT>::digits;
    std::string bs;
    bool        seenSetBit = false;
    for (int bn = nBits - 1; bn >= 0; --bn)
    {
        UT   mask  = one << bn;
        bool isSet = (uNumber & mask) != zero;
        if (trimSignificantClearBits && !seenSetBit && !isSet)
            continue;
        bs += isSet ? '1' : '0';
        if (isSet)
            seenSetBit = true;
    }
    return bs;
}

/// helper struct for case-insensitive containers.
/// use it as the second/third argument when creating a container, e.g.:
/// std::map< std::string, std::vector<std::string>, ci_less > myMap;
/// std::vector<std::string, ci_less> myVector;
// ReSharper disable once CppInconsistentNaming
struct ci_less
{
    // case-independent (ci) compare_less binary function
    struct NocaseCompare
    {
        bool operator()(const unsigned char& c1, const unsigned char& c2) const
        {
            return tolower(c1) < tolower(c2);
        }
    };
    bool operator()(const std::string& s1, const std::string& s2) const
    {
        return std::lexicographical_compare(s1.begin(), s1.end(), // source range
                                            s2.begin(), s2.end(), // dest range
                                            NocaseCompare());     // comparison
    }
};

// ReSharper disable once CppInconsistentNaming
struct ci_lessW
{
    // case-independent (ci) compare_less binary function
    struct NocaseCompare
    {
        bool operator()(const wchar_t& c1, const wchar_t& c2) const
        {
            return towlower(c1) < towlower(c2);
        }
    };
    bool operator()(const std::wstring& s1, const std::wstring& s2) const
    {
        return std::lexicographical_compare(s1.begin(), s1.end(), // source range
                                            s2.begin(), s2.end(), // dest range
                                            NocaseCompare());     // comparison
    }
};

class CStringUtils
{
public:
    // trim from both ends
    static inline std::string& trim(std::string& s)
    {
        return ltrim(rtrim(s));
    }
    static inline std::string& trim(std::string& s, const std::string& trimchars)
    {
        return ltrim(rtrim(s, trimchars), trimchars);
    }
    static inline std::string& trim(std::string& s, wint_t trimchar)
    {
        return ltrim(rtrim(s, trimchar), trimchar);
    }

    // trim from start
    static inline std::string& ltrim(std::string& s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](wint_t c) { return !iswspace(c); }));
        return s;
    }
    static inline std::string& ltrim(std::string& s, const std::string& trimchars)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [&trimchars](wint_t c) { return trimchars.find(static_cast<char>(c)) == std::string::npos; }));
        return s;
    }
    static inline std::string& ltrim(std::string& s, wint_t trimchar)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [&trimchar](wint_t c) { return c != trimchar; }));
        return s;
    }

    // trim from end
    static inline std::string& rtrim(std::string& s)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](wint_t c) { return !iswspace(c); }).base(), s.end());
        return s;
    }
    static inline std::string& rtrim(std::string& s, const std::string& trimchars)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(), [&trimchars](wint_t c) { return trimchars.find(static_cast<char>(c)) == std::string::npos; }).base(), s.end());
        return s;
    }
    static inline std::string& rtrim(std::string& s, wint_t trimchar)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(), [&trimchar](wint_t c) { return c != trimchar; }).base(), s.end());
        return s;
    }

    // trim from both ends
    static inline std::wstring& trim(std::wstring& s)
    {
        return ltrim(rtrim(s));
    }
    static inline std::wstring& trim(std::wstring& s, const std::wstring& trimchars)
    {
        return ltrim(rtrim(s, trimchars), trimchars);
    }
    static inline std::wstring& trim(std::wstring& s, wint_t trimchar)
    {
        return ltrim(rtrim(s, trimchar), trimchar);
    }

    // trim from start
    static inline std::wstring& ltrim(std::wstring& s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](wint_t c) { return !iswspace(c); }));
        return s;
    }
    static inline std::wstring& ltrim(std::wstring& s, const std::wstring& trimchars)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [&trimchars](wint_t c) { return trimchars.find(c) == std::wstring::npos; }));
        return s;
    }
    static inline std::wstring& ltrim(std::wstring& s, wint_t trimchar)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [&trimchar](wint_t c) { return c != trimchar; }));
        return s;
    }

    // trim from end
    static inline std::wstring& rtrim(std::wstring& s)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](wint_t c) { return !iswspace(c); }).base(), s.end());
        return s;
    }
    static inline std::wstring& rtrim(std::wstring& s, const std::wstring& trimchars)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(), [&trimchars](wint_t c) { return trimchars.find(c) == std::wstring::npos; }).base(), s.end());
        return s;
    }
    static inline std::wstring& rtrim(std::wstring& s, wint_t trimchar)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(), [&trimchar](wint_t c) { return c != trimchar; }).base(), s.end());
        return s;
    }

    static std::wstring ExpandEnvironmentStrings(const std::wstring& s)
    {
        DWORD len = ::ExpandEnvironmentStrings(s.c_str(), nullptr, 0);
        if (len == 0)
            return s;

        auto buf = std::make_unique<wchar_t[]>(len + 1ULL);
        if (::ExpandEnvironmentStrings(s.c_str(), buf.get(), len) == 0)
            return s;

        return buf.get();
    }

    static std::string ToHexString(BYTE* pSrc, int nSrcLen);

    static bool FromHexString(const std::string& src, BYTE* pDest);

    static std::wstring ToHexWString(BYTE* pSrc, int nSrcLen);

    static std::unique_ptr<char[]>    Decrypt(const char* text);
    static std::unique_ptr<wchar_t[]> Decrypt(const wchar_t* text);
    static std::string                Encrypt(const char* text);
    static std::wstring               Encrypt(const wchar_t* text);

    static std::wstring Format(const wchar_t* frmt, ...);
    static std::string  Format(const char* frmt, ...);

    [[deprecated("use case insensitive string comparison instead, or the ci_less container helper")]] static inline void emplace_to_lower(std::wstring& s)
    {
        std::transform(s.begin(), s.end(), s.begin(), ::towlower);
    }

    [[deprecated("use case insensitive string comparison instead, or the ci_less container helper")]] static inline void emplace_to_lower(std::string& s)
    {
        std::transform(s.begin(), s.end(), s.begin(), [](char c) { return static_cast<char>(::tolower(c)); });
    }

    /// converts a string to lowercase
    /// note: please use only where absolutely necessary!
    /// better use stricmp functions if possible since for non ANSI strings there just are too many exceptions
    /// and special cases to handle.
    static inline std::wstring to_lower(const std::wstring& s)
    {
        auto len    = LCMapStringEx(LOCALE_NAME_INVARIANT, LCMAP_LOWERCASE, s.c_str(), -1, nullptr, 0, nullptr, nullptr, 0);
        auto outBuf = std::make_unique<wchar_t[]>(len + 1LL);
        LCMapStringEx(LOCALE_NAME_INVARIANT, LCMAP_LOWERCASE, s.c_str(), -1, outBuf.get(), len, nullptr, nullptr, 0);
        return outBuf.get();
    }

    [[deprecated("use case insensitive string comparison instead, or the ci_less container helper")]] static inline std::string to_lower(const std::string& s)
    {
        std::string ls(s);
        std::transform(ls.begin(), ls.end(), ls.begin(), [](char c) { return static_cast<char>(::tolower(c)); });
        return ls;
    }

    //static inline std::wstring::const_iterator find_caseinsensitive(const std::wstring& haystack, const std::wstring& needle)
    //{
    //    std::wstring::const_iterator it = haystack.end();
    //    for (auto& i = haystack.begin(); i != haystack.end(); ++i)
    //    {
    //        if (_wcsnicmp(&(*i), needle.c_str(), needle.size()) == 0)
    //        {
    //            it = i;
    //            break;
    //        }
    //    }
    //    return it;
    //}
    static size_t find_caseinsensitive(const std::wstring& haystack, const std::wstring& needle)
    {
        auto ret = std::wstring::npos;
        for (size_t i = 0; i < haystack.size(); ++i)
        {
            if (_wcsnicmp(&haystack[i], needle.c_str(), needle.size()) == 0)
            {
                ret = i;
                break;
            }
        }
        return ret;
    }

    template <typename T, typename T2>
    static void TrimLeading(T& s, const T2& vals)
    {
        auto it = s.begin();
        while (it != s.end())
        {
            auto whereAt = std::find(vals.begin(), vals.end(), *it);
            if (whereAt == vals.end())
                break;
            ++it;
            if (it == s.end())
                break;
        }
        s.erase(s.begin(), it);
    }

    template <typename T, typename T2>
    static void TrimTrailing(T& s, const T2& vals)
    {
        while (!s.empty())
        {
            auto whereAt = std::find(vals.begin(), vals.end(), s.back());
            if (whereAt == vals.end())
                break;
            s.pop_back();
        }
    }

    // Trim container T of values in T2.
    // T1 can at least be a string, wstring, vector,
    // T2 can be simiar but initializer_list is the typical type used.
    template <typename T, typename T2>
    static void TrimLeadingAndTrailing(T& s, const T2& vals)
    {
        TrimLeading(s, vals);
        TrimTrailing(s, vals);
    }
};
