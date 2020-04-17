// grepWin - regex search and replace for Windows

// Copyright (C) 2007-2008, 2010, 2012-2013 - Stefan Kueng

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
    CSearchInfo(void);
    CSearchInfo(const std::wstring& path);
    ~CSearchInfo(void);

    std::wstring                filepath;
    __int64                     filesize;
    std::vector<DWORD>          matchlinesnumbers;
    std::vector<std::wstring>   matchlines;
    __int64                     matchcount;
    CTextFile::UnicodeType      encoding;
    FILETIME                    modifiedtime;
    bool                        readerror;
    bool                        folder;
};