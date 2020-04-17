// sktoolslib - common files for SK tools

// Copyright (C) 2012-2017, 2019 - Stefan Kueng

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
#include "StringUtils.h"
#include "OnOutOfScope.h"

#include <Wincrypt.h>

#pragma comment(lib, "Crypt32.lib")


int strwildcmp(const char *wild, const char *str)
{
    const char *cp = nullptr;
    const char *mp = nullptr;
    while ((*str) && (*wild != '*'))
    {
        if ((*wild != *str) && (*wild != '?'))
        {
            return 0;
        }
        wild++;
        str++;
    }
    while (*str)
    {
        if (*wild == '*')
        {
            if (!*++wild)
            {
                return 1;
            }
            mp = wild;
            cp = str +1;
        }
        else if ((*wild == *str) || (*wild == '?'))
        {
            wild++;
            str++;
        }
        else
        {
            wild = mp;
            str = cp++;
        }
    }

    while (*wild == '*')
    {
        wild++;
    }
    return !*wild;
}

int wcswildcmp(const wchar_t *wild, const wchar_t *str)
{
    const wchar_t *cp = nullptr;
    const wchar_t *mp = nullptr;
    while ((*str) && (*wild != L'*'))
    {
        if ((*wild != *str) && (*wild != L'?'))
        {
            return 0;
        }
        wild++;
        str++;
    }
    while (*str)
    {
        if (*wild == L'*')
        {
            if (!*++wild)
            {
                return 1;
            }
            mp = wild;
            cp = str +1;
        }
        else if ((*wild == *str) || (*wild == L'?'))
        {
            wild++;
            str++;
        }
        else
        {
            wild = mp;
            str = cp++;
        }
    }

    while (*wild == L'*')
    {
        wild++;
    }
    return !*wild;
}

int wcswildicmp(const wchar_t *wild, const wchar_t *str)
{
    const wchar_t *cp = nullptr;
    const wchar_t *mp = nullptr;
    while ((*str) && (*wild != L'*'))
    {
        if ((*wild != L'?') && (::towlower(*wild) != ::towlower(*str)))
        {
            return 0;
        }
        wild++;
        str++;
    }
    while (*str)
    {
        if (*wild == L'*')
        {
            if (!*++wild)
            {
                return 1;
            }
            mp = wild;
            cp = str + 1;
        }
        else if ((*wild == L'?') || (::towlower(*wild) == ::towlower(*str)))
        {
            wild++;
            str++;
        }
        else
        {
            wild = mp;
            str = cp++;
        }
    }

    while (*wild == L'*')
    {
        wild++;
    }
    return !*wild;
}

static constexpr BYTE HexLookup[513] = {
    "000102030405060708090a0b0c0d0e0f"
    "101112131415161718191a1b1c1d1e1f"
    "202122232425262728292a2b2c2d2e2f"
    "303132333435363738393a3b3c3d3e3f"
    "404142434445464748494a4b4c4d4e4f"
    "505152535455565758595a5b5c5d5e5f"
    "606162636465666768696a6b6c6d6e6f"
    "707172737475767778797a7b7c7d7e7f"
    "808182838485868788898a8b8c8d8e8f"
    "909192939495969798999a9b9c9d9e9f"
    "a0a1a2a3a4a5a6a7a8a9aaabacadaeaf"
    "b0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
    "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf"
    "d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
    "e0e1e2e3e4e5e6e7e8e9eaebecedeeef"
    "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff"
};
static constexpr BYTE DecLookup[] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // gap before first hex digit
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,1,2,3,4,5,6,7,8,9,       // 0123456789
    0,0,0,0,0,0,0,             // :;<=>?@ (gap)
    10,11,12,13,14,15,         // ABCDEF
    0,0,0,0,0,0,0,0,0,0,0,0,0, // GHIJKLMNOPQRS (gap)
    0,0,0,0,0,0,0,0,0,0,0,0,0, // TUVWXYZ[/]^_` (gap)
    10,11,12,13,14,15          // abcdef
};

std::string CStringUtils::ToHexString( BYTE* pSrc, int nSrcLen )
{
    WORD * pwHex=  (WORD*)HexLookup;
    auto dest = std::make_unique<char[]>((nSrcLen*2)+1);
    WORD * pwDest = (WORD*)dest.get();
    for (int j=0; j<nSrcLen; j++ )
    {
        *pwDest= pwHex[*pSrc];
        pwDest++; pSrc++;
    }
    *((BYTE*)pwDest)= 0;  // terminate the string
    return std::string(dest.get());
}

bool CStringUtils::FromHexString( const std::string& src, BYTE* pDest )
{
    if (src.size() % 2)
        return false;
    for (auto it = src.cbegin(); it != src.cend(); ++it)
    {
        if ((*it < '0') || (*it > 'f'))
            return false;
        int d = DecLookup[*it] << 4;
        ++it;
        d |= DecLookup[*it];
        *pDest++ = (BYTE)d;
    }
    return true;
}

std::wstring CStringUtils::ToHexWString( BYTE* pSrc, int nSrcLen )
{
    std::string s = ToHexString(pSrc, nSrcLen);
    return std::wstring(s.begin(), s.end());
}

std::unique_ptr<char[]> CStringUtils::Decrypt(const char * text)
{
    DWORD dwLen = 0;
    if (CryptStringToBinaryA(text, (DWORD)strlen(text), CRYPT_STRING_HEX, nullptr, &dwLen, nullptr, nullptr) == FALSE)
        return nullptr;

    auto strIn = std::make_unique<BYTE[]>(dwLen + 1);
    if (CryptStringToBinaryA(text, (DWORD)strlen(text), CRYPT_STRING_HEX, strIn.get(), &dwLen, nullptr, nullptr) == FALSE)
        return nullptr;

    DATA_BLOB blobin;
    blobin.cbData = dwLen;
    blobin.pbData = strIn.get();
    LPWSTR descr = nullptr;
    DATA_BLOB blobout = { 0 };
    if (CryptUnprotectData(&blobin, &descr, nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &blobout) == FALSE)
        return nullptr;
    SecureZeroMemory(blobin.pbData, blobin.cbData);

    auto result = std::make_unique<char[]>(blobout.cbData + 1);
    strncpy_s(result.get(), blobout.cbData + 1, (const char*)blobout.pbData, blobout.cbData);
    SecureZeroMemory(blobout.pbData, blobout.cbData);
    LocalFree(blobout.pbData);
    LocalFree(descr);
    return result;
}

std::unique_ptr<wchar_t[]> CStringUtils::Decrypt(const wchar_t * text)
{
    DWORD dwLen = 0;
    if (CryptStringToBinaryW(text, (DWORD)wcslen(text), CRYPT_STRING_HEX, nullptr, &dwLen, nullptr, nullptr) == FALSE)
        return nullptr;

    auto strIn = std::make_unique<BYTE[]>(dwLen + 1);
    if (CryptStringToBinaryW(text, (DWORD)wcslen(text), CRYPT_STRING_HEX, strIn.get(), &dwLen, nullptr, nullptr) == FALSE)
        return nullptr;

    DATA_BLOB blobin;
    blobin.cbData = dwLen;
    blobin.pbData = strIn.get();
    LPWSTR descr = nullptr;
    DATA_BLOB blobout = { 0 };
    if (CryptUnprotectData(&blobin, &descr, nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &blobout) == FALSE)
        return nullptr;
    SecureZeroMemory(blobin.pbData, blobin.cbData);

    auto result = std::make_unique<wchar_t[]>((blobout.cbData) / sizeof(wchar_t) + 1);
    wcsncpy_s(result.get(), (blobout.cbData) / sizeof(wchar_t) + 1, (const wchar_t*)blobout.pbData, blobout.cbData / sizeof(wchar_t));
    SecureZeroMemory(blobout.pbData, blobout.cbData);
    LocalFree(blobout.pbData);
    LocalFree(descr);
    return result;
}

std::string CStringUtils::Encrypt(const char * text)
{
    DATA_BLOB blobin = { 0 };
    DATA_BLOB blobout = { 0 };
    std::string result;

    blobin.cbData = (DWORD)strlen(text);
    blobin.pbData = (BYTE*)(LPCSTR)text;
    if (CryptProtectData(&blobin, L"TSVNAuth", nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &blobout) == FALSE)
        return result;
    DWORD dwLen = 0;
    if (CryptBinaryToStringA(blobout.pbData, blobout.cbData, CRYPT_STRING_HEX | CRYPT_STRING_NOCRLF, nullptr, &dwLen) == FALSE)
        return result;
    auto strOut = std::make_unique<char[]>(dwLen + 1);
    if (CryptBinaryToStringA(blobout.pbData, blobout.cbData, CRYPT_STRING_HEX | CRYPT_STRING_NOCRLF, strOut.get(), &dwLen) == FALSE)
        return result;
    LocalFree(blobout.pbData);

    result = strOut.get();

    return result;
}

std::wstring CStringUtils::Encrypt(const wchar_t * text)
{
    DATA_BLOB blobin = { 0 };
    DATA_BLOB blobout = { 0 };
    std::wstring result;

    blobin.cbData = (DWORD)wcslen(text)*sizeof(wchar_t);
    blobin.pbData = (BYTE*)(LPCWSTR)text;
    if (CryptProtectData(&blobin, L"TSVNAuth", nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &blobout) == FALSE)
        return result;
    DWORD dwLen = 0;
    if (CryptBinaryToStringW(blobout.pbData, blobout.cbData, CRYPT_STRING_HEX | CRYPT_STRING_NOCRLF, nullptr, &dwLen) == FALSE)
        return result;
    auto strOut = std::make_unique<wchar_t[]>(dwLen + 1);
    if (CryptBinaryToStringW(blobout.pbData, blobout.cbData, CRYPT_STRING_HEX | CRYPT_STRING_NOCRLF, strOut.get(), &dwLen) == FALSE)
        return result;
    LocalFree(blobout.pbData);

    result = strOut.get();

    return result;
}

std::wstring CStringUtils::Format( const wchar_t* frmt, ... )
{
    std::wstring buffer;
    if (frmt != nullptr)
    {
        va_list marker;

        va_start(marker, frmt);

        // Get formatted string length adding one for the NUL
        auto len = _vscwprintf(frmt, marker);
        if (len > 0)
        {
            buffer.resize(len + 1);
            _vsnwprintf_s(&buffer[0], buffer.size(), len, frmt, marker);
            buffer.resize(len);
        }

        va_end(marker);
    }
    return buffer;
}

std::string CStringUtils::Format( const char* frmt, ... )
{
    std::string buffer;
    if (frmt != nullptr)
    {
        va_list marker;

        va_start(marker, frmt);

        // Get formatted string length adding one for the NUL
        auto len = _vscprintf(frmt, marker);
        if (len > 0)
        {
            buffer.resize(len + 1);
            _vsnprintf_s(&buffer[0], buffer.size(), len, frmt, marker);
            buffer.resize(len);
        }

        va_end(marker);
    }
    return buffer;
}

bool WriteAsciiStringToClipboard(const wchar_t * sClipdata, HWND hOwningWnd)
{
    if (OpenClipboard(hOwningWnd))
    {
        OnOutOfScope(
            CloseClipboard();
        );
        EmptyClipboard();
        size_t sLen = _tcslen(sClipdata);
        HGLOBAL hClipboardData = GlobalAlloc(GMEM_DDESHARE, (sLen+1)*sizeof(wchar_t));
        if (hClipboardData)
        {
            wchar_t* pchData = (wchar_t*)GlobalLock(hClipboardData);
            if (pchData)
            {
                _tcscpy_s(pchData, sLen+1, sClipdata);
                if (GlobalUnlock(hClipboardData))
                {
                    if (SetClipboardData(CF_UNICODETEXT, hClipboardData) == nullptr)
                        return true;
                }
            }
        }
    }
    return false;
}

void SearchReplace(std::wstring& str, const std::wstring& toreplace, const std::wstring& replacewith)
{
    std::wstring result;
    std::wstring::size_type pos = 0;
    for (;;)
    {
        std::wstring::size_type next = str.find(toreplace, pos);
        result.append(str, pos, next-pos);
        if (next != std::wstring::npos)
        {
            result.append(replacewith);
            pos = next + toreplace.size();
        }
        else
        {
            break;  // exit loop
        }
    }
    str = std::move(result);
}


void SearchReplace( std::string& str, const std::string& toreplace, const std::string& replacewith )
{
    std::string result;
    std::string::size_type pos = 0;
    for (;;)
    {
        std::string::size_type next = str.find(toreplace, pos);
        result.append(str, pos, next-pos);
        if (next != std::string::npos)
        {
            result.append(replacewith);
            pos = next + toreplace.size();
        }
        else
        {
            break;  // exit loop
        }
    }
    str = std::move(result);
}

void SearchRemoveAll(std::string& str, const std::string& toremove)
{
    std::string::size_type pos = 0;
    for (;;)
    {
        auto nextpos = str.find(toremove, pos);
        if (nextpos == std::string::npos)
            break;
        str.erase(nextpos, toremove.length());
        pos = nextpos;
    }
}

void SearchRemoveAll(std::wstring& str, const std::wstring& toremove)
{
    std::wstring::size_type pos = 0;
    for (;;)
    {
        auto nextpos = str.find(toremove, pos);
        if (nextpos == std::wstring::npos)
            break;
        str.erase(nextpos, toremove.length());
        pos = nextpos;
    }
}
