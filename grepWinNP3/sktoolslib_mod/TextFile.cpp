// sktoolslib - common files for SK tools

// Copyright (C) 2012, 2014, 2017-2023 - Stefan Kueng

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
#include "TextFile.h"
#include "PathUtils.h"
#include "maxpath.h"
#include <memory>

static wchar_t WideCharSwap(wchar_t nValue)
{
    return (((nValue >> 8)) | (nValue << 8));
}

static UINT64 WordSwapBytes(UINT64 nValue)
{
    return ((nValue & 0xff00ff00ff00ff) << 8) | ((nValue >> 8) & 0xff00ff00ff00ff); // swap BYTESs in WORDs
}

CTextFile::CTextFile()
    : pFileBuf(nullptr)
    , fileLen(0)
    , encoding(AutoType)
    , hasBOM(false)
    , nullByteCount(2)
{
}

CTextFile::~CTextFile()
{
    pFileBuf = nullptr;
}

bool CTextFile::Save(LPCWSTR path, bool keepFileDate) const
{
    if (pFileBuf == nullptr)
        return false;
    FILETIME creationTime{};
    FILETIME lastAccessTime{};
    FILETIME lastWriteTime{};
    if (keepFileDate)
    {
        HANDLE hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                  nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            GetFileTime(hFile, &creationTime, &lastAccessTime, &lastWriteTime);
            CloseHandle(hFile);
        }
    }

    HANDLE hFile = CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ,
                              nullptr, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        assert(false);
        return false;
    }

    DWORD byteswritten;
    if (!WriteFile(hFile, pFileBuf.get(), fileLen, &byteswritten, nullptr))
    {
        CloseHandle(hFile);
        return false;
    }
    if (keepFileDate)
        SetFileTime(hFile, &creationTime, &lastAccessTime, &lastWriteTime);
    CloseHandle(hFile);
    return true;
}

bool CTextFile::Load(LPCWSTR path, UnicodeType &type, bool bUTF8, std::atomic_bool& bCancelled)
{
    encoding = AutoType;
    type     = AutoType;
    LARGE_INTEGER lint;
    pFileBuf            = nullptr;
    auto   pathBuf      = std::make_unique<wchar_t[]>(MAX_PATH_NEW);
    HANDLE hFile        = INVALID_HANDLE_VALUE;
    int    retryCounter = 0;

    if ((wcslen(path) > 2) && (path[0] == '\\') && (path[1] == '\\'))
    {
        // UNC path
        wcscpy_s(pathBuf.get(), MAX_PATH_NEW, L"\\\\?\\UNC");
        wcscat_s(pathBuf.get(), MAX_PATH_NEW, &path[1]);
    }
    else
    {
        // 'normal' path
        wcscpy_s(pathBuf.get(), MAX_PATH_NEW, L"\\\\?\\");
        wcscat_s(pathBuf.get(), MAX_PATH_NEW, path);
    }

    do
    {
        if (retryCounter)
            Sleep(20 + retryCounter * 50);
        hFile = CreateFile(pathBuf.get(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                           nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
        retryCounter++;
    } while (hFile == INVALID_HANDLE_VALUE && retryCounter < 5);

    if (hFile == INVALID_HANDLE_VALUE)
        return false;
    std::wstring wPath(path);
    size_t       pos = wPath.find_last_of('\\');
    filename         = wPath.substr(pos + 1);
    if (!GetFileSizeEx(hFile, &lint))
    {
        CloseHandle(hFile);
        return false;
    }

    MEMORYSTATUSEX memEx = {sizeof(MEMORYSTATUSEX)};
    GlobalMemoryStatusEx(&memEx);

    DWORD bytesRead   = 0;
    DWORD bytesToRead = min(lint.LowPart, DWORD(memEx.ullAvailPhys / 4UL));
    if (lint.HighPart)
        bytesToRead = 500000; // read 50kb if the file is too big: we only
                              // need to scan for the file type then.

    // if there isn't enough RAM available, only load a small part of the file
    // to do the encoding check. Then only load the full file in case
    // the encoding is UNICODE_LE since that's the only encoding we have
    // to convert first to do a proper search with.
    if (bytesToRead < lint.LowPart)
    {
        try
        {
            auto tempFileBuf = std::make_unique<BYTE[]>(bytesToRead + 1);
            if (!ReadFile(hFile, tempFileBuf.get(), bytesToRead, &bytesRead, nullptr))
            {
                CloseHandle(hFile);
                return false;
            }
            encoding = CheckUnicodeType(tempFileBuf.get(), bytesRead);
            type     = encoding;
            if (lint.HighPart)
            {
                CloseHandle(hFile);
                return false;
            }

            switch (encoding)
            {
                case Binary:
                case UTF8:
                case Ansi:
                    CloseHandle(hFile);
                    return false;
                default:
                    pFileBuf = std::make_unique<BYTE[]>(lint.LowPart);
                    if (pFileBuf)
                    {
                        for (unsigned long bc = 0; bc < bytesRead; ++bc)
                        {
                            pFileBuf[bc] = tempFileBuf[bc];
                        }
                    }
                    break;
            }
        }
        catch (const std::exception &)
        {
            return false;
        }
    }
    else
    {
        try
        {
            pFileBuf = std::make_unique<BYTE[]>(lint.LowPart);
        }
        catch (const std::exception &)
        {
            return false;
        }
    }
    if ((pFileBuf == nullptr) || (!ReadFile(hFile, pFileBuf.get(), lint.LowPart, &bytesRead, nullptr)))
    {
        pFileBuf = nullptr;
        CloseHandle(hFile);
        return false;
    }
    CloseHandle(hFile);
    fileLen = lint.LowPart;

    // we have the file read into memory, now we have to find out what
    // kind of text file we have here.
    if (encoding == AutoType)
    {
        encoding = CheckUnicodeType(pFileBuf.get(), bytesRead);
        if ((bUTF8) && (encoding != Binary))
            encoding = UTF8;
    }

    if (encoding == Unicode_Le)
    {
        try
        {
            if ((bytesRead > 1) && (*static_cast<unsigned char *>(pFileBuf.get()) == 0xFF))
            {
                // remove the BOM
                textContent.assign((reinterpret_cast<wchar_t *>(pFileBuf.get()) + 1), (bytesRead / sizeof(wchar_t)) - 1);
                hasBOM = true;
            }
            else
                textContent.assign(reinterpret_cast<wchar_t *>(pFileBuf.get()), bytesRead / sizeof(wchar_t));
        }
        catch (const std::exception &)
        {
            return false;
        }
    }
    else if (encoding == Unicode_Be)
    {
        // make in place WORD BYTEs swap
        UINT64 *pQw     = reinterpret_cast<UINT64 *>(pFileBuf.get());
        int     nQwords = bytesRead / 8;
        for (int nQword = 0; nQword < nQwords; nQword++)
            pQw[nQword] = WordSwapBytes(pQw[nQword]);

        wchar_t *pW     = reinterpret_cast<wchar_t *>(pQw);
        int      nWords = bytesRead / 2;
        for (int nWord = nQwords * 4; nWord < nWords; nWord++)
            pW[nWord] = WideCharSwap(pW[nWord]);

        try
        {
            if ((bytesRead > 1) && (*static_cast<unsigned char *>(pFileBuf.get()) == 0xFF))
            {
                // remove the BOM
                textContent.assign((reinterpret_cast<wchar_t *>(pFileBuf.get()) + 1), (bytesRead / sizeof(wchar_t)) - 1);
                hasBOM = true;
            }
            else
                textContent.assign(reinterpret_cast<wchar_t *>(pFileBuf.get()), bytesRead / sizeof(wchar_t));
        }
        catch (const std::exception &)
        {
            return false;
        }
    }
    else if ((encoding == UTF8) || ((encoding == Binary) && (bUTF8)))
    {
        try
        {
            int  ret      = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<LPCSTR>(pFileBuf.get()), bytesRead, nullptr, 0);
            auto pWideBuf = std::make_unique<wchar_t[]>(ret + 1);
            int  ret2     = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<LPCSTR>(pFileBuf.get()), bytesRead, pWideBuf.get(), ret + 1);
            if (ret2 == ret)
            {
                if ((ret > 1) && (*pWideBuf.get() == 0xFEFF))
                {
                    // remove the BOM
                    textContent.assign(pWideBuf.get() + 1, ret - 1);
                    hasBOM = true;
                }
                else
                    textContent.assign(pWideBuf.get(), ret);
            }
        }
        catch (const std::exception &)
        {
            return false;
        }
    }
    else //if (encoding == ANSI)
    {
        try
        {
            int  ret      = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, reinterpret_cast<LPCSTR>(pFileBuf.get()), bytesRead, nullptr, 0);
            auto pWideBuf = std::make_unique<wchar_t[]>(ret + 1);
            int  ret2     = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, reinterpret_cast<LPCSTR>(pFileBuf.get()), bytesRead, pWideBuf.get(), ret + 1);
            if (ret2 == ret)
                textContent.assign(pWideBuf.get(), ret);
        }
        catch (const std::exception &)
        {
            return false;
        }
    }
    type = encoding;
    if (type == Binary)
        return true;
    return CalculateLines(bCancelled);
}

void CTextFile::SetFileContent(const std::wstring &content)
{
    pFileBuf = nullptr;
    fileLen  = 0;

    try
    {
        if (encoding == Unicode_Le)
        {
            if (hasBOM)
            {
                pFileBuf = std::make_unique<BYTE[]>((content.size() + 2) * sizeof(wchar_t));
                if (pFileBuf)
                {
                    memcpy(pFileBuf.get(), "\xFF\xFE", sizeof(wchar_t));
                    memcpy(pFileBuf.get() + 2, content.c_str(), content.size() * sizeof(wchar_t));
                    fileLen = (static_cast<int>(content.size()) + 1) * sizeof(wchar_t);
                }
            }
            else
            {
                pFileBuf = std::make_unique<BYTE[]>(content.size() * sizeof(wchar_t));
                if (pFileBuf)
                {
                    memcpy(pFileBuf.get(), content.c_str(), content.size() * sizeof(wchar_t));
                    fileLen = static_cast<int>(content.size()) * sizeof(wchar_t);
                }
            }
        }
        else if (encoding == Unicode_Be)
        {
            if (hasBOM)
            {
                pFileBuf = std::make_unique<BYTE[]>((content.size() + 2) * sizeof(wchar_t));
                if (pFileBuf)
                {
                    memcpy(pFileBuf.get(), "\xFF\xFE", sizeof(wchar_t));
                    memcpy(pFileBuf.get() + 2, content.c_str(), content.size() * sizeof(wchar_t));
                    fileLen = (static_cast<int>(content.size()) + 1) * sizeof(wchar_t);
                }
            }
            else
            {
                pFileBuf = std::make_unique<BYTE[]>(content.size() * sizeof(wchar_t));
                if (pFileBuf)
                {
                    memcpy(pFileBuf.get(), content.c_str(), content.size() * sizeof(wchar_t));
                    fileLen = static_cast<int>(content.size()) * sizeof(wchar_t);
                }
            }
            // make in place WORD BYTEs swap
            UINT64 *pQw     = reinterpret_cast<UINT64 *>(pFileBuf.get());
            int     nQwords = fileLen / 8;
            for (int nQword = 0; nQword < nQwords; nQword++)
                pQw[nQword] = WordSwapBytes(pQw[nQword]);

            wchar_t *pW     = reinterpret_cast<wchar_t *>(pQw);
            int      nWords = fileLen / 2;
            for (int nWord = nQwords * 4; nWord < nWords; nWord++)
                pW[nWord] = WideCharSwap(pW[nWord]);
        }
        else if (encoding == UTF8)
        {
            if (hasBOM)
            {
                int ret  = WideCharToMultiByte(CP_UTF8, 0, content.c_str(), -1, nullptr, 0, nullptr, nullptr);
                pFileBuf = std::make_unique<BYTE[]>(ret + 3);
                if (pFileBuf)
                {
                    memcpy(pFileBuf.get(), "\xEF\xBB\xBF", 3);
                    int ret2 = WideCharToMultiByte(CP_UTF8, 0, content.c_str(), -1, reinterpret_cast<LPSTR>(pFileBuf.get()) + 3, ret, nullptr, nullptr);
                    fileLen  = ret2 + 2;
                    if (ret2 != ret)
                    {
                        pFileBuf = nullptr;
                        fileLen  = 0;
                    }
                }
            }
            else
            {
                int ret  = WideCharToMultiByte(CP_UTF8, 0, content.c_str(), -1, nullptr, 0, nullptr, nullptr);
                pFileBuf = std::make_unique<BYTE[]>(ret);
                if (pFileBuf)
                {
                    int ret2 = WideCharToMultiByte(CP_UTF8, 0, content.c_str(), -1, reinterpret_cast<LPSTR>(pFileBuf.get()), ret, nullptr, nullptr);
                    fileLen  = ret2 - 1;
                    if (ret2 != ret)
                    {
                        pFileBuf = nullptr;
                        fileLen  = 0;
                    }
                }
            }
        }
        else if ((encoding == Ansi) || (encoding == Binary))
        {
            int ret  = WideCharToMultiByte(CP_ACP, 0, content.c_str(), static_cast<int>(content.size()) + 1, nullptr, 0, nullptr, nullptr);
            pFileBuf = std::make_unique<BYTE[]>(ret);
            if (pFileBuf)
            {
                int ret2 = WideCharToMultiByte(CP_ACP, 0, content.c_str(), static_cast<int>(content.size()) + 1, reinterpret_cast<LPSTR>(pFileBuf.get()), ret, nullptr, nullptr);
                fileLen  = ret2 - 1;
                if (ret2 != ret)
                {
                    pFileBuf = nullptr;
                    fileLen  = 0;
                }
            }
        }
    }
    catch (const std::exception &)
    {
    }
    if (pFileBuf)
        textContent = content;
    else
        textContent = L"";
}

bool CTextFile::ContentsModified(std::unique_ptr<BYTE[]> pBuf, DWORD newLen)
{
    pFileBuf = std::move(pBuf);
    fileLen  = newLen;
    return true;
}

CTextFile::UnicodeType CTextFile::CheckUnicodeType(BYTE *pBuffer, int cb) const
{
    if (cb < 2)
        return Ansi;
    UINT16 *pVal16 = reinterpret_cast<UINT16 *>(pBuffer);
    UINT8 * pVal8  = reinterpret_cast<UINT8 *>(pVal16 + 1);
    // scan the whole buffer for a 0x0000 sequence
    // if found, we assume a binary file
    int nNull    = 0;
    int nDblNull = 0;
    for (int i = 0; i < (cb - 2); i = i + 2)
    {
        if (0x0000 == *pVal16++)
            ++nDblNull;
        if (0x00 == *pVal8++)
            ++nNull;
        if (0x00 == *pVal8++)
            ++nNull;
    }
    if (nDblNull > nullByteCount) // configured value: allow double null chars to account for 'broken' text files
        return Binary;
    pVal16 = reinterpret_cast<UINT16 *>(pBuffer);
    pVal8  = reinterpret_cast<UINT8 *>(pVal16 + 1);
    if (*pVal16 == 0xFEFF)
        return Unicode_Le;
    if (*pVal16 == 0xFFFE)
        return Unicode_Be;
    if ((nNull > 3) && ((cb % 2) == 0)) // arbitrary value: allow three null chars to account for 'broken' ANSI/UTF8 text files, otherwise consider the file UTF16-LE
        return Unicode_Le;
    if (cb < 3)
        return Ansi;
    if (*pVal16 == 0xBBEF)
    {
        if (*pVal8 == 0xBF)
            return UTF8;
    }
    // check for illegal UTF8 chars
    pVal8 = static_cast<UINT8 *>(pBuffer);
    for (int i = 0; i < cb; ++i)
    {
        if ((*pVal8 == 0xC0) || (*pVal8 == 0xC1) || (*pVal8 >= 0xF5))
            return Ansi;
        pVal8++;
    }
    pVal8      = static_cast<UINT8 *>(pBuffer);
    bool bUTF8 = false;
    for (int i = 0; i < (cb - 4); ++i)
    {
        if ((*pVal8 & 0xE0) == 0xC0)
        {
            pVal8++;
            i++;
            if ((*pVal8 & 0xC0) != 0x80)
                return Ansi;
            bUTF8 = true;
        }
        if ((*pVal8 & 0xF0) == 0xE0)
        {
            pVal8++;
            i++;
            if ((*pVal8 & 0xC0) != 0x80)
                return Ansi;
            pVal8++;
            i++;
            if ((*pVal8 & 0xC0) != 0x80)
                return Ansi;
            bUTF8 = true;
        }
        if ((*pVal8 & 0xF8) == 0xF0)
        {
            pVal8++;
            i++;
            if ((*pVal8 & 0xC0) != 0x80)
                return Ansi;
            pVal8++;
            i++;
            if ((*pVal8 & 0xC0) != 0x80)
                return Ansi;
            pVal8++;
            i++;
            if ((*pVal8 & 0xC0) != 0x80)
                return Ansi;
            bUTF8 = true;
        }
        pVal8++;
    }
    if (bUTF8)
        return UTF8;
    return Ansi;
}

bool CTextFile::CalculateLines(std::atomic_bool& bCancelled)
{
    // fill an array with starting positions for every line in the loaded file
    if (pFileBuf == nullptr)
        return false;
    if (textContent.empty())
        return true;
    linePositions.clear();
    linePositions.reserve(textContent.size() / 10);
    size_t pos = 0;
    for (auto it = textContent.begin(); it != textContent.end() && !bCancelled; ++it)
    {
        if (*it == '\r')
        {
            ++it;
            ++pos;
            if (it != textContent.end())
            {
                if (*it == '\n')
                {
                    // crlf lineending
                    linePositions.push_back(pos);
                }
                else
                {
                    // cr lineending
                    linePositions.push_back(pos - 1);
                }
            }
            else
                break;
        }
        else if (*it == '\n')
        {
            // lf lineending
            linePositions.push_back(pos);
        }
        ++pos;
    }
    linePositions.push_back(pos);
    return true;
}

long CTextFile::LineFromPosition(long pos) const
{
    auto lb     = std::lower_bound(linePositions.begin(), linePositions.end(), static_cast<size_t>(pos));
    auto lbLine = lb - linePositions.begin();
    return static_cast<long>(lbLine + 1);
}

std::wstring CTextFile::GetLineString(long lineNumber) const
{
    if ((lineNumber - 2) >= static_cast<long>(linePositions.size()))
        return std::wstring();
    if (lineNumber <= 0)
        return std::wstring();

    long startPos = 0;
    if (lineNumber > 1)
        startPos = static_cast<long>(linePositions[lineNumber - 2]) + 1;
    std::wstring endChars(L"\n\0", 2);
    size_t       endPos = textContent.find_first_of(endChars, startPos);
    std::wstring line;
    if (endPos != std::wstring::npos)
        line = std::wstring(textContent.begin() + startPos, textContent.begin() + endPos);
    else
        line = std::wstring(textContent.begin() + startPos, textContent.end());

    return line;
}

std::wstring CTextFile::GetEncodingString(UnicodeType type)
{
    switch (type)
    {
        case CTextFile::Ansi:
            return L"ANSI";
        case CTextFile::Unicode_Le:
            return L"UTF-16-LE";
        case CTextFile::Unicode_Be:
            return L"UTF-16-BE";
        case CTextFile::UTF8:
            return L"UTF8";
        case CTextFile::Binary:
            return L"BINARY";
        default:
            break;
    }
    return {};
}

std::wstring CTextFile::GetFileNameWithoutExtension() const
{
    return CPathUtils::GetFileNameWithoutExtension(filename);
}

std::wstring CTextFile::GetFileNameExtension() const
{
    return CPathUtils::GetFileExtension(filename);
}
