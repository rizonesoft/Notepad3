// sktoolslib - common files for SK tools

// Copyright (C) 2013, 2017, 2020-2021 - Stefan Kueng

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

#include "stdafx.h"
#include "EscapeUtils.h"
#include "StringUtils.h"
#include "UnicodeUtils.h"

#if defined(_M_IX86) || defined(_M_X64)
#    include <emmintrin.h>
#endif

static BOOL sse2Supported = ::IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE);
// clang-format off
static constexpr char iri_escape_chars[256] = {
    1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,

    /* 128 */
    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0
};

const char uri_autoescape_chars[256] = {
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 1, 0, 0,

    /* 64 */
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 0, 1,
    0, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 1, 0,

    /* 128 */
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,

    /* 192 */
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
};

// clang-format on

bool CEscapeUtils::ContainsEscapedChars(const char* psz, size_t length)
{
    // most of our strings will be tens of bytes long
    // -> afford some minor overhead to handle the main part very fast

    const char* end = psz + length;
#if defined(_M_IX86) || defined(_M_X64)
    if (sse2Supported)
    {
        __m128i mask = _mm_set_epi8('%', '%', '%', '%', '%', '%', '%', '%', '%', '%', '%', '%', '%', '%', '%', '%');

        for (; psz + sizeof(mask) <= end; psz += sizeof(mask))
        {
            // fetch the next 16 bytes from the source

            __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(psz));

            // check for non-ASCII

            int flags = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk, mask));
            if (flags != 0)
                return true;
        };
    }
#endif

    // return odd bytes at the end of the string

    for (; psz < end; ++psz)
        if (*psz == '%')
            return true;

    return false;
}

char* CEscapeUtils::Unescape(char* psz)
{
    char* pszSource = psz;
    char* pszDest   = psz;

    static constexpr char szHex[] = "0123456789ABCDEF";

    // Unescape special characters. The number of characters
    // in the *pszDest is assumed to be <= the number of characters
    // in pszSource (they are both the same string anyway)

    while (*pszSource != '\0' && *pszDest != '\0')
    {
        if (*pszSource == '%')
        {
            // The next two chars following '%' should be digits
            if (*(pszSource + 1) == '\0' ||
                *(pszSource + 2) == '\0')
            {
                // nothing left to do
                break;
            }

            char        nValue  = '?';
            const char* pszHigh = nullptr;
            pszSource++;

            *pszSource = static_cast<char>(toupper(*pszSource));
            pszHigh    = strchr(szHex, *pszSource);

            if (pszHigh != nullptr)
            {
                pszSource++;
                *pszSource         = static_cast<char>(toupper(*pszSource));
                const char* pszLow = strchr(szHex, *pszSource);

                if (pszLow != nullptr)
                {
                    nValue = static_cast<char>(((pszHigh - szHex) << 4) +
                                               (pszLow - szHex));
                }
            }
            else
            {
                pszSource--;
                nValue = *pszSource;
            }
            *pszDest++ = nValue;
        }
        else
            *pszDest++ = *pszSource;

        pszSource++;
    }

    *pszDest = '\0';
    return pszDest;
}

std::string CEscapeUtils::EscapeString(const std::string& str)
{
    std::string ret2;
    int         c;
    int         i;
    for (i = 0; str[i]; ++i)
    {
        c = static_cast<unsigned char>(str[i]);
        if (iri_escape_chars[c])
        {
            // no escaping needed for that char
            ret2 += static_cast<unsigned char>(str[i]);
        }
        else
        {
            // char needs escaping
            ret2 += CStringUtils::Format("%%%02X", static_cast<unsigned char>(c));
        }
    }
    std::string ret;
    for (i = 0; ret2[i]; ++i)
    {
        c = static_cast<unsigned char>(ret2[i]);
        if (uri_autoescape_chars[c])
        {
            if ((c == '%') && (DoesPercentNeedEscaping(ret2.substr(i).c_str())))
            {
                // this percent sign needs escaping!
                ret += CStringUtils::Format("%%%02X", static_cast<unsigned char>(c));
            }
            else
            {
                // no escaping needed for that char
                ret += static_cast<unsigned char>(ret2[i]);
            }
        }
        else
        {
            // char needs escaping
            ret += CStringUtils::Format("%%%02X", static_cast<unsigned char>(c));
        }
    }
    return ret;
}

std::wstring CEscapeUtils::EscapeString(const std::wstring& str)
{
    return CUnicodeUtils::StdGetUnicode(EscapeString(CUnicodeUtils::StdGetUTF8(str)));
}

std::string CEscapeUtils::StringUnescape(const std::string& str)
{
    auto urlabuf = std::make_unique<char[]>(str.size() + 1);

    strcpy_s(urlabuf.get(), str.size() + 1, str.c_str());
    Unescape(urlabuf.get());

    return urlabuf.get();
}

std::wstring CEscapeUtils::StringUnescape(const std::wstring& str)
{
    auto len = str.size();
    if (len == 0)
        return std::wstring();
    std::string stra = CUnicodeUtils::StdGetUTF8(str);

    stra = StringUnescape(stra);

    return CUnicodeUtils::StdGetUnicode(stra);
}

bool CEscapeUtils::DoesPercentNeedEscaping(LPCSTR str)
{
    if (str[1] == 0)
        return true;
    if (!(((str[1] >= '0') && (str[1] <= '9')) || ((str[1] >= 'A') && (str[1] <= 'F')) || ((str[1] >= 'a') && (str[1] <= 'f'))))
        return true;
    if (!(((str[2] >= '0') && (str[2] <= '9')) || ((str[2] >= 'A') && (str[2] <= 'F')) || ((str[2] >= 'a') && (str[2] <= 'f'))))
        return true;
    return false;
}
