// sktoolslib - common files for SK tools

// Copyright (C) 2012, 2014, 2017-2020 - Stefan Kueng

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

static wchar_t inline WideCharSwap(wchar_t nValue)
{
    return (((nValue >> 8)) | (nValue << 8));
}

static UINT64 inline WordSwapBytes(UINT64 nValue)
{
    return ((nValue & 0xff00ff00ff00ff) << 8) | ((nValue >> 8) & 0xff00ff00ff00ff); // swap BYTESs in WORDs
}

CTextFile::CTextFile(void)
    : pFileBuf(nullptr)
    , filelen(0)
    , hasBOM(false)
    , encoding(AUTOTYPE)
    , m_NullByteCount(2)
{
}

CTextFile::~CTextFile(void)
{
    pFileBuf = nullptr;
}

bool CTextFile::Save(LPCTSTR path)
{
    if (pFileBuf == nullptr)
        return false;
    HANDLE hFile = CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ,
                              NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return false;
    DWORD byteswritten;
    if (!WriteFile(hFile, pFileBuf.get(), filelen, &byteswritten, NULL))
    {
        CloseHandle(hFile);
        return false;
    }
    CloseHandle(hFile);
    return true;
}

bool CTextFile::Load(LPCTSTR path, UnicodeType &type, bool bUTF8, volatile LONG* bCancelled)
{
    encoding = AUTOTYPE;
    type     = AUTOTYPE;
    LARGE_INTEGER lint;
    pFileBuf            = nullptr;
    auto   pathbuf      = std::make_unique<wchar_t[]>(MAX_PATH_NEW);
    HANDLE hFile        = INVALID_HANDLE_VALUE;
    int    retrycounter = 0;

    if ((_tcslen(path) > 2) && (path[0] == '\\') && (path[1] == '\\'))
    {
        // UNC path
        _tcscpy_s(pathbuf.get(), MAX_PATH_NEW, L"\\\\?\\UNC");
        _tcscat_s(pathbuf.get(), MAX_PATH_NEW, &path[1]);
    }
    else
    {
        // 'normal' path
        _tcscpy_s(pathbuf.get(), MAX_PATH_NEW, L"\\\\?\\");
        _tcscat_s(pathbuf.get(), MAX_PATH_NEW, path);
    }

    do
    {
        if (retrycounter)
            Sleep(20);
        hFile = CreateFile(pathbuf.get(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                           NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        retrycounter++;
    } while (hFile == INVALID_HANDLE_VALUE && retrycounter < 5);

    if (hFile == INVALID_HANDLE_VALUE)
        return false;
    std::wstring wpath(path);
    size_t       pos = wpath.find_last_of('\\');
    filename         = wpath.substr(pos + 1);
    if (!GetFileSizeEx(hFile, &lint))
    {
        CloseHandle(hFile);
        return false;
    }

    MEMORYSTATUSEX memex = {sizeof(MEMORYSTATUSEX)};
    GlobalMemoryStatusEx(&memex);

    DWORD bytesread   = 0;
    DWORD bytestoread = min(lint.LowPart, DWORD(memex.ullAvailPhys / 4UL));
    if (lint.HighPart)
        bytestoread = 500000; // read 50kb if the file is too big: we only
                              // need to scan for the file type then.

    // if there isn't enough RAM available, only load a small part of the file
    // to do the encoding check. Then only load the full file in case
    // the encoding is UNICODE_LE since that's the only encoding we have
    // to convert first to do a proper search with.
    if (bytestoread < lint.LowPart)
    {
        try
        {
            auto tempfilebuf = std::make_unique<BYTE[]>(bytestoread + 1);
            if (!ReadFile(hFile, tempfilebuf.get(), bytestoread, &bytesread, NULL))
            {
                CloseHandle(hFile);
                return false;
            }
            encoding = CheckUnicodeType(tempfilebuf.get(), bytesread);
            type     = encoding;
            if (lint.HighPart)
            {
                CloseHandle(hFile);
                return false;
            }

            switch (encoding)
            {
                case BINARY:
                case UTF8:
                case ANSI:
                    CloseHandle(hFile);
                    return false;
                    break;
                default:
                    pFileBuf = std::make_unique<BYTE[]>(lint.LowPart);
                    if (pFileBuf)
                    {
                        for (unsigned long bc = 0; bc < bytesread; ++bc)
                        {
                            pFileBuf[bc] = tempfilebuf[bc];
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
    if ((pFileBuf == nullptr) || (!ReadFile(hFile, pFileBuf.get(), lint.LowPart, &bytesread, NULL)))
    {
        pFileBuf = nullptr;
        CloseHandle(hFile);
        return false;
    }
    CloseHandle(hFile);
    filelen = lint.LowPart;

    // we have the file read into memory, now we have to find out what
    // kind of text file we have here.
    if (encoding == AUTOTYPE)
    {
        encoding = CheckUnicodeType(pFileBuf.get(), bytesread);
        if ((bUTF8) && (encoding != BINARY))
            encoding = UTF8;
    }

    if (encoding == UNICODE_LE)
    {
        try
        {
            if ((bytesread > 1) && (*(unsigned char *)pFileBuf.get() == 0xFF))
            {
                // remove the BOM
                textcontent.assign(((wchar_t *)pFileBuf.get() + 1), (bytesread / sizeof(wchar_t)) - 1);
                hasBOM = true;
            }
            else
                textcontent.assign((wchar_t *)pFileBuf.get(), bytesread / sizeof(wchar_t));
        }
        catch (const std::exception &)
        {
            return false;
        }
    }
    else if (encoding == UNICODE_BE)
    {
        // make in place WORD BYTEs swap
        UINT64* p_qw = (UINT64*)pFileBuf.get();
        int nQwords = bytesread / 8;
        for (int nQword = 0; nQword < nQwords; nQword++)
            p_qw[nQword] = WordSwapBytes(p_qw[nQword]);

        wchar_t* p_w = (wchar_t*)p_qw;
        int nWords = bytesread / 2;
        for (int nWord = nQwords * 4; nWord < nWords; nWord++)
            p_w[nWord] = WideCharSwap(p_w[nWord]);

        try
        {
            if ((bytesread > 1) && (*(unsigned char*)pFileBuf.get() == 0xFF))
            {
                // remove the BOM
                textcontent.assign(((wchar_t*)pFileBuf.get() + 1), (bytesread / sizeof(wchar_t)) - 1);
                hasBOM = true;
            }
            else
                textcontent.assign((wchar_t*)pFileBuf.get(), bytesread / sizeof(wchar_t));
        }
        catch (const std::exception&)
        {
            return false;
        }
    }
    else if ((encoding == UTF8) || ((encoding == BINARY) && (bUTF8)))
    {
        try
        {
            int  ret      = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)pFileBuf.get(), bytesread, NULL, 0);
            auto pWideBuf = std::make_unique<wchar_t[]>(ret + 1);
            int  ret2     = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)pFileBuf.get(), bytesread, pWideBuf.get(), ret + 1);
            if (ret2 == ret)
            {
                if ((ret > 1) && (*pWideBuf.get() == 0xFEFF))
                {
                    // remove the BOM
                    textcontent.assign(pWideBuf.get() + 1, ret - 1);
                    hasBOM = true;
                }
                else
                    textcontent.assign(pWideBuf.get(), ret);
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
            int  ret      = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR)pFileBuf.get(), bytesread, NULL, 0);
            auto pWideBuf = std::make_unique<wchar_t[]>(ret + 1);
            int  ret2     = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR)pFileBuf.get(), bytesread, pWideBuf.get(), ret + 1);
            if (ret2 == ret)
                textcontent.assign(pWideBuf.get(), ret);
        }
        catch (const std::exception &)
        {
            return false;
        }
    }
    type = encoding;
    if (type == BINARY)
        return true;
    return CalculateLines(bCancelled);
}

void CTextFile::SetFileContent(const std::wstring &content)
{
    pFileBuf = nullptr;
    filelen  = 0;

    try
    {
        if (encoding == UNICODE_LE)
        {
            if (hasBOM)
            {
                pFileBuf = std::make_unique<BYTE[]>((content.size() + 2) * sizeof(wchar_t));
                if (pFileBuf)
                {
                    memcpy(pFileBuf.get(), "\xFF\xFE", sizeof(wchar_t));
                    memcpy(pFileBuf.get() + 2, content.c_str(), content.size() * sizeof(wchar_t));
                    filelen = ((int)content.size() + 1) * sizeof(wchar_t);
                }
            }
            else
            {
                pFileBuf = std::make_unique<BYTE[]>(content.size() * sizeof(wchar_t));
                if (pFileBuf)
                {
                    memcpy(pFileBuf.get(), content.c_str(), content.size() * sizeof(wchar_t));
                    filelen = (int)content.size() * sizeof(wchar_t);
                }
            }
        }
        else if (encoding == UNICODE_BE)
        {
            if (hasBOM)
            {
                pFileBuf = std::make_unique<BYTE[]>((content.size() + 2) * sizeof(wchar_t));
                if (pFileBuf)
                {
                    memcpy(pFileBuf.get(), "\xFF\xFE", sizeof(wchar_t));
                    memcpy(pFileBuf.get() + 2, content.c_str(), content.size() * sizeof(wchar_t));
                    filelen = ((int)content.size() + 1) * sizeof(wchar_t);
                }
            }
            else
            {
                pFileBuf = std::make_unique<BYTE[]>(content.size() * sizeof(wchar_t));
                if (pFileBuf)
                {
                    memcpy(pFileBuf.get(), content.c_str(), content.size() * sizeof(wchar_t));
                    filelen = (int)content.size() * sizeof(wchar_t);
                }
            }
            // make in place WORD BYTEs swap
            UINT64* p_qw = (UINT64*)pFileBuf.get();
            int nQwords = filelen / 8;
            for (int nQword = 0; nQword < nQwords; nQword++)
                p_qw[nQword] = WordSwapBytes(p_qw[nQword]);

            wchar_t* p_w = (wchar_t*)p_qw;
            int nWords = filelen / 2;
            for (int nWord = nQwords * 4; nWord < nWords; nWord++)
                p_w[nWord] = WideCharSwap(p_w[nWord]);
        }
        else if (encoding == UTF8)
        {
            if (hasBOM)
            {
                int ret  = WideCharToMultiByte(CP_UTF8, 0, content.c_str(), -1, NULL, 0, NULL, NULL);
                pFileBuf = std::make_unique<BYTE[]>(ret + 3);
                if (pFileBuf)
                {
                    memcpy(pFileBuf.get(), "\xEF\xBB\xBF", 3);
                    int ret2 = WideCharToMultiByte(CP_UTF8, 0, content.c_str(), -1, (LPSTR)pFileBuf.get() + 3, ret, NULL, NULL);
                    filelen  = ret2 + 2;
                    if (ret2 != ret)
                    {
                        pFileBuf = nullptr;
                        filelen  = 0;
                    }
                }
            }
            else
            {
                int ret  = WideCharToMultiByte(CP_UTF8, 0, content.c_str(), -1, NULL, 0, NULL, NULL);
                pFileBuf = std::make_unique<BYTE[]>(ret);
                if (pFileBuf)
                {
                    int ret2 = WideCharToMultiByte(CP_UTF8, 0, content.c_str(), -1, (LPSTR)pFileBuf.get(), ret, NULL, NULL);
                    filelen  = ret2 - 1;
                    if (ret2 != ret)
                    {
                        pFileBuf = nullptr;
                        filelen  = 0;
                    }
                }
            }
        }
        else if ((encoding == ANSI) || (encoding == BINARY))
        {
            int ret  = WideCharToMultiByte(CP_ACP, 0, content.c_str(), (int)content.size() + 1, NULL, 0, NULL, NULL);
            pFileBuf = std::make_unique<BYTE[]>(ret);
            if (pFileBuf)
            {
                int ret2 = WideCharToMultiByte(CP_ACP, 0, content.c_str(), (int)content.size() + 1, (LPSTR)pFileBuf.get(), ret, NULL, NULL);
                filelen  = ret2 - 1;
                if (ret2 != ret)
                {
                    pFileBuf = nullptr;
                    filelen  = 0;
                }
            }
        }
    }
    catch (const std::exception &)
    {
    }
    if (pFileBuf)
        textcontent = content;
    else
        textcontent = L"";
}

bool CTextFile::ContentsModified(std::unique_ptr<BYTE[]> pBuf, DWORD newLen)
{
    pFileBuf = std::move(pBuf);
    filelen  = newLen;
    return true;
}

CTextFile::UnicodeType CTextFile::CheckUnicodeType(BYTE *pBuffer, int cb)
{
    if (cb < 2)
        return ANSI;
    UINT16 *pVal16 = (UINT16 *)pBuffer;
    UINT8 * pVal8  = (UINT8 *)(pVal16 + 1);
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
    if (nDblNull > m_NullByteCount) // configured value: allow double null chars to account for 'broken' text files
        return BINARY;
    pVal16 = (UINT16 *)pBuffer;
    pVal8  = (UINT8 *)(pVal16 + 1);
    if (*pVal16 == 0xFEFF)
        return UNICODE_LE;
    if (*pVal16 == 0xFFFE)
        return UNICODE_BE;
    if ((nNull > 3) && ((cb % 2) == 0)) // arbitrary value: allow three null chars to account for 'broken' ANSI/UTF8 text files, otherwise consider the file UTF16-LE
        return UNICODE_LE;
    if (cb < 3)
        return ANSI;
    if (*pVal16 == 0xBBEF)
    {
        if (*pVal8 == 0xBF)
            return UTF8;
    }
    // check for illegal UTF8 chars
    pVal8 = (UINT8 *)pBuffer;
    for (int i = 0; i < cb; ++i)
    {
        if ((*pVal8 == 0xC0) || (*pVal8 == 0xC1) || (*pVal8 >= 0xF5))
            return ANSI;
        pVal8++;
    }
    pVal8      = (UINT8 *)pBuffer;
    bool bUTF8 = false;
    for (int i = 0; i < (cb - 4); ++i)
    {
        if ((*pVal8 & 0xE0) == 0xC0)
        {
            pVal8++;
            i++;
            if ((*pVal8 & 0xC0) != 0x80)
                return ANSI;
            bUTF8 = true;
        }
        if ((*pVal8 & 0xF0) == 0xE0)
        {
            pVal8++;
            i++;
            if ((*pVal8 & 0xC0) != 0x80)
                return ANSI;
            pVal8++;
            i++;
            if ((*pVal8 & 0xC0) != 0x80)
                return ANSI;
            bUTF8 = true;
        }
        if ((*pVal8 & 0xF8) == 0xF0)
        {
            pVal8++;
            i++;
            if ((*pVal8 & 0xC0) != 0x80)
                return ANSI;
            pVal8++;
            i++;
            if ((*pVal8 & 0xC0) != 0x80)
                return ANSI;
            pVal8++;
            i++;
            if ((*pVal8 & 0xC0) != 0x80)
                return ANSI;
            bUTF8 = true;
        }
        pVal8++;
    }
    if (bUTF8)
        return UTF8;
    return ANSI;
}

bool CTextFile::CalculateLines(volatile LONG * bCancelled)
{
    // fill an array with starting positions for every line in the loaded file
    if (pFileBuf == NULL)
        return false;
    if (textcontent.empty())
        return true;
    linepositions.clear();
    linepositions.reserve(textcontent.size() / 10);
    size_t pos = 0;
    for (auto it = textcontent.begin(); it != textcontent.end() && ((bCancelled == nullptr) || !InterlockedExchangeAdd(bCancelled, 0)); ++it)
    {
        if (*it == '\r')
        {
            ++it;
            ++pos;
            if (it != textcontent.end())
            {
                if (*it == '\n')
                {
                    // crlf lineending
                    linepositions.push_back(pos);
                }
                else
                {
                    // cr lineending
                    linepositions.push_back(pos - 1);
                }
            }
            else
                break;
        }
        else if (*it == '\n')
        {
            // lf lineending
            linepositions.push_back(pos);
        }
        ++pos;
    }
    linepositions.push_back(pos);
    return true;
}

long CTextFile::LineFromPosition(long pos) const
{
    auto lb     = std::lower_bound(linepositions.begin(), linepositions.end(), static_cast<size_t>(pos));
    auto lbLine = lb - linepositions.begin();
    return long(lbLine + 1);
}

std::wstring CTextFile::GetLineString(long lineNumber) const
{
    if ((lineNumber - 2) >= (long)linepositions.size())
        return std::wstring();
    if (lineNumber <= 0)
        return std::wstring();

    long startpos = 0;
    if (lineNumber > 1)
        startpos = (long)linepositions[lineNumber - 2] + 1;
    std::wstring endchars(L"\n\0", 2);
    size_t       endpos = textcontent.find_first_of(endchars, startpos);
    std::wstring line;
    if (endpos != std::wstring::npos)
        line = std::wstring(textcontent.begin() + startpos, textcontent.begin() + endpos);
    else
        line = std::wstring(textcontent.begin() + startpos, textcontent.end());

    return line;
}

std::wstring CTextFile::GetFileNameWithoutExtension()
{
    return CPathUtils::GetFileNameWithoutExtension(filename);
}

std::wstring CTextFile::GetFileNameExtension()
{
    return CPathUtils::GetFileExtension(filename);
}
