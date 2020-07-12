// grepWin - regex search and replace for Windows

// Copyright (C) 2007-2009, 2012-2013, 2017, 2020 - Stefan Kueng

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
#include "Bookmarks.h"
#include "maxpath.h"
#include "CmdLineParser.h"
#include <shlobj.h>
#include <memory>

CBookmarks::CBookmarks(void)
{
    SetUnicode(true);
    SetMultiLine(true);
    SetSpaces(false);
}

CBookmarks::~CBookmarks(void)
{
}

void CBookmarks::Load()
{
    std::unique_ptr<TCHAR[]> path(new TCHAR[MAX_PATH_NEW]);
    GetModuleFileName(NULL, path.get(), MAX_PATH_NEW);
    if (bPortable)
    {
        m_iniPath = path.get();
        m_iniPath = m_iniPath.substr(0, m_iniPath.rfind('\\'));
    }
    else
    {
        SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, path.get());
        m_iniPath = path.get();
        m_iniPath += L"\\grepWinNP3";
    }
    CreateDirectory(m_iniPath.c_str(), NULL);
    m_iniPath += L"\\bookmarks";
    SetUnicode();
    SetMultiLine();
    SetSpaces(false);
    LoadFile(m_iniPath.c_str());
}

void CBookmarks::Save()
{
    std::unique_ptr<TCHAR[]> path(new TCHAR[MAX_PATH_NEW]);
    GetModuleFileName(NULL, path.get(), MAX_PATH_NEW);
    if (bPortable)
    {
        m_iniPath = path.get();
        m_iniPath = m_iniPath.substr(0, m_iniPath.rfind('\\'));
    }
    else
    {
        SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, path.get());
        m_iniPath = path.get();
        m_iniPath += L"\\grepWinNP3";
    }
    CreateDirectory(m_iniPath.c_str(), NULL);
    m_iniPath += L"\\bookmarks";

    SaveFile(m_iniPath.c_str(), true);
}

void CBookmarks::AddBookmark(const Bookmark& bm)
{
    std::wstring val = L"\"";
    val += bm.Search;
    val += L"\"";
    SetValue(bm.Name.c_str(), L"searchString", val.c_str());

    val = L"\"";
    val += bm.Replace;
    val += L"\"";
    SetValue(bm.Name.c_str(), L"replaceString",      val.c_str());
    SetValue(bm.Name.c_str(), L"useregex",           bm.UseRegex ? L"true" : L"false");
    SetValue(bm.Name.c_str(), L"casesensitive",      bm.CaseSensitive ? L"true" : L"false");
    SetValue(bm.Name.c_str(), L"dotmatchesnewline",  bm.DotMatchesNewline ? L"true" : L"false");
    SetValue(bm.Name.c_str(), L"backup",             bm.Backup ? L"true" : L"false");
    SetValue(bm.Name.c_str(), L"utf8",               bm.Utf8 ? L"true" : L"false");
    SetValue(bm.Name.c_str(), L"includesystem",      bm.IncludeSystem ? L"true" : L"false");
    SetValue(bm.Name.c_str(), L"includefolder",      bm.IncludeFolder ? L"true" : L"false");
    SetValue(bm.Name.c_str(), L"includehidden",      bm.IncludeHidden ? L"true" : L"false");
    SetValue(bm.Name.c_str(), L"includebinary",      bm.IncludeBinary ? L"true" : L"false");
    val = L"\"";
    val += bm.ExcludeDirs;
    val += L"\"";
    SetValue(bm.Name.c_str(), L"excludedirs",        val.c_str());
    val = L"\"";
    val += bm.FileMatch;
    val += L"\"";
    SetValue(bm.Name.c_str(), L"filematch",          val.c_str());
    SetValue(bm.Name.c_str(), L"filematchregex",     bm.FileMatchRegex ? L"true" : L"false");
    SetValue(bm.Name.c_str(), L"searchpath",         bm.Path.c_str());
}

void CBookmarks::RemoveBookmark(const std::wstring& name)
{
    Delete(name.c_str(), L"searchString",        true);
    Delete(name.c_str(), L"replaceString",       true);
    Delete(name.c_str(), L"useregex",            true);
    Delete(name.c_str(), L"casesensitive",       true);
    Delete(name.c_str(), L"dotmatchesnewline",   true);
    Delete(name.c_str(), L"backup",              true);
    Delete(name.c_str(), L"utf8",                true);
    Delete(name.c_str(), L"includesystem",       true);
    Delete(name.c_str(), L"includefolder",       true);
    Delete(name.c_str(), L"includehidden",       true);
    Delete(name.c_str(), L"includebinary",       true);
    Delete(name.c_str(), L"excludedirs",         true);
    Delete(name.c_str(), L"filematch",           true);
    Delete(name.c_str(), L"filematchregex",      true);
    Delete(name.c_str(), L"searchpath",          true);
}

Bookmark CBookmarks::GetBookmark( const std::wstring& name )
{
    Bookmark bk;
    if (GetSectionSize(name.c_str()) >= 0)
    {
        bk.Name = name;
        bk.Search               = GetValue(name.c_str(), L"searchString",       L"");
        bk.Replace              = GetValue(name.c_str(), L"replaceString",      L"");
        bk.UseRegex             = wcscmp(GetValue(name.c_str(), L"useregex",           L"false"), L"true") == 0;
        bk.CaseSensitive        = wcscmp(GetValue(name.c_str(), L"casesensitive",      L"false"), L"true") == 0;
        bk.DotMatchesNewline    = wcscmp(GetValue(name.c_str(), L"dotmatchesnewline",  L"false"), L"true") == 0;
        bk.Backup               = wcscmp(GetValue(name.c_str(), L"backup",             L"false"), L"true") == 0;
        bk.Utf8                 = wcscmp(GetValue(name.c_str(), L"utf8",               L"false"), L"true") == 0;
        bk.IncludeSystem        = wcscmp(GetValue(name.c_str(), L"includesystem",      L"false"), L"true") == 0;
        bk.IncludeFolder        = wcscmp(GetValue(name.c_str(), L"includefolder",      L"false"), L"true") == 0;
        bk.IncludeHidden        = wcscmp(GetValue(name.c_str(), L"includehidden",      L"false"), L"true") == 0;
        bk.IncludeBinary        = wcscmp(GetValue(name.c_str(), L"includebinary",      L"false"), L"true") == 0;
        bk.ExcludeDirs          = GetValue(name.c_str(), L"excludedirs",        L"");
        bk.FileMatch            = GetValue(name.c_str(), L"filematch",          L"");
        bk.FileMatchRegex       = wcscmp(GetValue(name.c_str(), L"filematchregex",     L"false"), L"true") == 0;
        bk.Path                 = GetValue(name.c_str(), L"searchpath", L"");
    }

    return bk;
}

