// sktoolslib - common files for SK tools

// Copyright (C) 2013, 2020-2021 - Stefan Kueng

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
#include "UnicodeUtils.h"
#include <fstream>
#include <vector>
#include <time.h>

CCircularLog::CCircularLog()
    : m_maxLines(10000)
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

bool CCircularLog::Init(const std::wstring& path, int maxlines)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    m_path     = path;
    m_maxLines = maxlines;

    try
    {
        std::ifstream file;
        file.open(m_path);
        if (!file.good())
        {
            CreateDirectory(path.substr(0, path.find_last_of('\\')).c_str(), nullptr);
            HANDLE hFile = CreateFile(path.c_str(), GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (hFile != INVALID_HANDLE_VALUE)
                CloseHandle(hFile);
            return false;
        }
        std::string line;
        while (std::getline(file, line))
        {
            m_lines.push_back(CUnicodeUtils::StdGetUnicode(line));
        };
        file.close();
    }
    catch (std::exception&)
    {
        return false;
    }
    return true;
}

bool CCircularLog::AddLine(const std::wstring& line)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    wchar_t                      tmpBuf1[128] = {0};
    _wstrtime_s(tmpBuf1, 128);
    wchar_t tmpBuf2[128] = {0};
    _wstrdate_s(tmpBuf2, 128);

    m_lines.push_back(CStringUtils::Format(L"%s %s : %s", tmpBuf2, tmpBuf1, line.c_str()));
    while (static_cast<int>(m_lines.size()) > m_maxLines)
        m_lines.pop_front();
    return true;
}

bool CCircularLog::Save()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_path.empty())
        return false;
    try
    {
        std::ofstream file;
        file.open(m_path);
        if (!file.good())
        {
            return false;
        }
        for (const auto& line : m_lines)
        {
            file << CUnicodeUtils::StdGetUTF8(line) << std::endl;
        }
        file.close();
    }
    catch (std::exception&)
    {
        return false;
    }
    return true;
}

void CCircularLog::operator()(LPCWSTR pszFormat, ...)
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
