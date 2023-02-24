// grepWin - regex search and replace for Windows

// Copyright (C) 2007-2008, 2010, 2012-2013, 2021-2023 - Stefan Kueng

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
#include <vector>
#include "TextFile.h"

class CSearchInfo
{
public:
    CSearchInfo();
    CSearchInfo(const std::wstring& path);
    ~CSearchInfo();

    static bool               NameCompareAsc(const CSearchInfo& entry1, const CSearchInfo& entry2);
    static bool               SizeCompareAsc(const CSearchInfo& entry1, const CSearchInfo& entry2);
    static bool               MatchesCompareAsc(const CSearchInfo& entry1, const CSearchInfo& entry2);
    static bool               PathCompareAsc(const CSearchInfo& entry1, const CSearchInfo& entry2);
    static bool               EncodingCompareAsc(const CSearchInfo& entry1, const CSearchInfo& entry2);
    static bool               ModifiedTimeCompareAsc(const CSearchInfo& entry1, const CSearchInfo& entry2);
    static bool               ExtCompareAsc(const CSearchInfo& entry1, const CSearchInfo& entry2);

    static bool               NameCompareDesc(const CSearchInfo& entry1, const CSearchInfo& entry2);
    static bool               SizeCompareDesc(const CSearchInfo& entry1, const CSearchInfo& entry2);
    static bool               MatchesCompareDesc(const CSearchInfo& entry1, const CSearchInfo& entry2);
    static bool               PathCompareDesc(const CSearchInfo& entry1, const CSearchInfo& entry2);
    static bool               EncodingCompareDesc(const CSearchInfo& entry1, const CSearchInfo& entry2);
    static bool               ModifiedTimeCompareDesc(const CSearchInfo& entry1, const CSearchInfo& entry2);
    static bool               ExtCompareDesc(const CSearchInfo& entry1, const CSearchInfo& entry2);

    bool                      operator<(const CSearchInfo& other) const;

    std::wstring              filePath;
    __int64                   fileSize;
    std::vector<DWORD>        matchLinesNumbers;
    std::vector<std::wstring> matchLines;
    __int64                   matchCount;
    CTextFile::UnicodeType    encoding;
    FILETIME                  modifiedTime;
    bool                      readError;
    bool                      folder;
};
