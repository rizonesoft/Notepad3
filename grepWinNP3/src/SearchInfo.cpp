// grepWin - regex search and replace for Windows

// Copyright (C) 2007-2008, 2012-2014, 2021-2022 - Stefan Kueng

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
#include "SearchInfo.h"

CSearchInfo::CSearchInfo()
    : fileSize(0)
    , matchCount(0)
    , encoding(CTextFile::UnicodeType::AutoType)
    , readError(false)
    , folder(false)
{
    modifiedTime.dwHighDateTime = 0;
    modifiedTime.dwLowDateTime  = 0;
}

CSearchInfo::CSearchInfo(const std::wstring& path)
    : filePath(path)
    , fileSize(0)
    , matchCount(0)
    , encoding(CTextFile::UnicodeType::AutoType)
    , readError(false)
    , folder(false)
{
    modifiedTime.dwHighDateTime = 0;
    modifiedTime.dwLowDateTime  = 0;
}

CSearchInfo::~CSearchInfo()
{
}

bool CSearchInfo::NameCompareAsc(const CSearchInfo& entry1, const CSearchInfo& entry2)
{
    std::wstring name1 = entry1.filePath.substr(entry1.filePath.find_last_of('\\') + 1);
    std::wstring name2 = entry2.filePath.substr(entry2.filePath.find_last_of('\\') + 1);
    return StrCmpLogicalW(name1.c_str(), name2.c_str()) < 0;
}

bool CSearchInfo::SizeCompareAsc(const CSearchInfo& entry1, const CSearchInfo& entry2)
{
    return entry1.fileSize < entry2.fileSize;
}

bool CSearchInfo::MatchesCompareAsc(const CSearchInfo& entry1, const CSearchInfo& entry2)
{
    return entry1.matchCount < entry2.matchCount;
}

bool CSearchInfo::PathCompareAsc(const CSearchInfo& entry1, const CSearchInfo& entry2)
{
    std::wstring name1 = entry1.filePath.substr(entry1.filePath.find_last_of('\\') + 1);
    std::wstring name2 = entry2.filePath.substr(entry2.filePath.find_last_of('\\') + 1);
    std::wstring path1 = entry1.filePath.substr(0, entry1.filePath.size() - name1.size() - 1);
    std::wstring path2 = entry2.filePath.substr(0, entry2.filePath.size() - name2.size() - 1);
    int          cmp   = path1.compare(path2);
    if (cmp != 0)
        return cmp < 0;
    return StrCmpLogicalW(name1.c_str(), name2.c_str()) < 0;
}

bool CSearchInfo::EncodingCompareAsc(const CSearchInfo& entry1, const CSearchInfo& entry2)
{
    return entry1.encoding < entry2.encoding;
}

bool CSearchInfo::ModifiedTimeCompareAsc(const CSearchInfo& entry1, const CSearchInfo& entry2)
{
    return CompareFileTime(&entry1.modifiedTime, &entry2.modifiedTime) < 0;
}

bool CSearchInfo::ExtCompareAsc(const CSearchInfo& entry1, const CSearchInfo& entry2)
{
    auto         dotPos1 = entry1.filePath.find_last_of('.');
    auto         dotPos2 = entry2.filePath.find_last_of('.');
    std::wstring ext1    = dotPos1 != std::wstring::npos ? entry1.filePath.substr(dotPos1 + 1) : L"";
    std::wstring ext2    = dotPos2 != std::wstring::npos ? entry2.filePath.substr(dotPos2 + 1) : L"";
    return StrCmpLogicalW(ext1.c_str(), ext2.c_str()) < 0;
}

bool CSearchInfo::NameCompareDesc(const CSearchInfo& entry1, const CSearchInfo& entry2)
{
    std::wstring name1 = entry1.filePath.substr(entry1.filePath.find_last_of('\\') + 1);
    std::wstring name2 = entry2.filePath.substr(entry2.filePath.find_last_of('\\') + 1);
    return StrCmpLogicalW(name1.c_str(), name2.c_str()) > 0;
}

bool CSearchInfo::SizeCompareDesc(const CSearchInfo& entry1, const CSearchInfo& entry2)
{
    return entry1.fileSize > entry2.fileSize;
}

bool CSearchInfo::MatchesCompareDesc(const CSearchInfo& entry1, const CSearchInfo& entry2)
{
    return entry1.matchCount > entry2.matchCount;
}

bool CSearchInfo::PathCompareDesc(const CSearchInfo& entry1, const CSearchInfo& entry2)
{
    std::wstring name1 = entry1.filePath.substr(entry1.filePath.find_last_of('\\') + 1);
    std::wstring name2 = entry2.filePath.substr(entry2.filePath.find_last_of('\\') + 1);
    std::wstring path1 = entry1.filePath.substr(0, entry1.filePath.size() - name1.size() - 1);
    std::wstring path2 = entry2.filePath.substr(0, entry2.filePath.size() - name2.size() - 1);
    int          cmp   = path1.compare(path2);
    if (cmp != 0)
        return cmp > 0;
    return StrCmpLogicalW(name1.c_str(), name2.c_str()) > 0;
}

bool CSearchInfo::EncodingCompareDesc(const CSearchInfo& entry1, const CSearchInfo& entry2)
{
    return entry1.encoding > entry2.encoding;
}

bool CSearchInfo::ModifiedTimeCompareDesc(const CSearchInfo& entry1, const CSearchInfo& entry2)
{
    return CompareFileTime(&entry1.modifiedTime, &entry2.modifiedTime) > 0;
}

bool CSearchInfo::ExtCompareDesc(const CSearchInfo& entry1, const CSearchInfo& entry2)
{
    auto         dotPos1 = entry1.filePath.find_last_of('.');
    auto         dotPos2 = entry2.filePath.find_last_of('.');
    std::wstring ext1    = dotPos1 != std::wstring::npos ? entry1.filePath.substr(dotPos1 + 1) : L"";
    std::wstring ext2    = dotPos2 != std::wstring::npos ? entry2.filePath.substr(dotPos2 + 1) : L"";
    return StrCmpLogicalW(ext1.c_str(), ext2.c_str()) > 0;
}
