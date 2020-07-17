// grepWin - regex search and replace for Windows

// Copyright (C) 2007-2008, 2012-2013, 2020 - Stefan Kueng

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
#include "SimpleIni.h"
#include <string>
#include <vector>

class Bookmark
{
public:
    Bookmark()
        : UseRegex(false)
        , CaseSensitive(false)
        , DotMatchesNewline(false)
        , Backup(false)
        , Utf8(false)
        , IncludeSystem(false)
        , IncludeFolder(false)
        , IncludeHidden(false)
        , IncludeBinary(false)
        , FileMatchRegex(false)
    {
    }
    ~Bookmark() {};

    std::wstring            Name;
    std::wstring            Search;
    std::wstring            Replace;
    std::wstring            Path;
    bool                    UseRegex;
    bool                    CaseSensitive;
    bool                    DotMatchesNewline;
    bool                    Backup;
    bool                    Utf8;
    bool                    Binary;
    bool                    IncludeSystem;
    bool                    IncludeFolder;
    bool                    IncludeHidden;
    bool                    IncludeBinary;
    std::wstring            ExcludeDirs;
    std::wstring            FileMatch;
    bool                    FileMatchRegex;

};

class CBookmarks : public CSimpleIni
{
public:
    CBookmarks(void);
    ~CBookmarks(void);

    void                Load();
    void                Save();
    void                AddBookmark(const Bookmark& bm);
    void                RemoveBookmark(const std::wstring& name);
    Bookmark            GetBookmark(const std::wstring& name);

protected:
    std::wstring        m_iniPath;
    TNamesDepend        m_sections;
};
