// sktoolslib - common files for SK tools

// Copyright (C) 2013 - Stefan Kueng

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
#include "CircularLog.h"
#include "codecvt.h"
#include "StringUtils.h"
#include <fstream>
#include <vector>
#include <time.h>

CCircularLog::CCircularLog()
    : m_maxlines(10000)
{

}

CCircularLog::~CCircularLog()
{
    Save();
}

CCircularLog& CCircularLog::Instance()
{
    static CCircularLog instance;
    return instance;
}

bool CCircularLog::Init( const std::wstring& path, int maxlines )
{
    m_path = path;
    m_maxlines = maxlines;

    try
    {
        std::wifstream File;
        File.imbue(std::locale(std::locale(), new utf8_conversion()));
        File.open(m_path);
        if (!File.good())
        {
            CreateDirectory(path.substr(0, path.find_last_of('\\')).c_str(), NULL);
            HANDLE hFile = CreateFile(path.c_str(), GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile != INVALID_HANDLE_VALUE)
                CloseHandle(hFile);
            return false;
        }
        std::wstring line;
        while (std::getline(File, line))
        {
            m_lines.push_back(line);
        };
        File.close();
    }
    catch (std::exception&)
    {
        return false;
    }
    return true;
}

bool CCircularLog::AddLine( const std::wstring& line )
{
    wchar_t tmpbuf1[128] = {0};
    _wstrtime_s(tmpbuf1, 128);
    wchar_t tmpbuf2[128] = {0};
    _wstrdate_s(tmpbuf2, 128);

    m_lines.push_back(CStringUtils::Format(L"%s %s : %s", tmpbuf2, tmpbuf1, line.c_str()));
    while ((int)m_lines.size() > m_maxlines)
        m_lines.pop_front();
    return true;
}

bool CCircularLog::Save()
{
    if (m_path.empty())
        return false;
    try
    {
        std::wofstream File;
        File.imbue(std::locale(std::locale(), new utf8_conversion()));
        File.open(m_path);
        if (!File.good())
        {
            return false;
        }
        for (const auto& line : m_lines)
        {
            File << line << std::endl;
        }
        File.close();
    }
    catch (std::exception&)
    {
        return false;
    }
    return true;
}

void CCircularLog::operator()( LPCWSTR pszFormat, ... )
{
    va_list marker;

    // Initialize variable arguments
    va_start(marker, pszFormat);

    // Get formatted string length adding one for the NULL
    size_t len = _vscwprintf(pszFormat, marker) + 1;

    // Create a char vector to hold the formatted string.
    std::vector<wchar_t> buffer(len, L'\0');
    _vsnwprintf_s(&buffer[0], buffer.size(), len, pszFormat, marker);

    // Reset variable arguments
    va_end(marker);

    AddLine(&buffer[0]);
}
