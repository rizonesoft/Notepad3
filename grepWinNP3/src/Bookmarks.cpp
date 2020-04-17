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
        m_iniPath += _T("\\grepWinNP3");
    }
    CreateDirectory(m_iniPath.c_str(), NULL);
    m_iniPath += _T("\\bookmarks");
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
        m_iniPath += _T("\\grepWinNP3");
    }
    CreateDirectory(m_iniPath.c_str(), NULL);
    m_iniPath += _T("\\bookmarks");

    SaveFile(m_iniPath.c_str(), true);
}

void CBookmarks::AddBookmark(const Bookmark& bm)
{
    std::wstring val = _T("\"");
    val += bm.Search;
    val += _T("\"");
    SetValue(bm.Name.c_str(), _T("searchString"), val.c_str());

    val = _T("\"");
    val += bm.Replace;
    val += _T("\"");
    SetValue(bm.Name.c_str(), _T("replaceString"),      val.c_str());
    SetValue(bm.Name.c_str(), _T("useregex"),           bm.UseRegex ? _T("true") : _T("false"));
    SetValue(bm.Name.c_str(), _T("casesensitive"),      bm.CaseSensitive ? _T("true") : _T("false"));
    SetValue(bm.Name.c_str(), _T("dotmatchesnewline"),  bm.DotMatchesNewline ? _T("true") : _T("false"));
    SetValue(bm.Name.c_str(), _T("backup"),             bm.Backup ? _T("true") : _T("false"));
    SetValue(bm.Name.c_str(), _T("utf8"),               bm.Utf8 ? _T("true") : _T("false"));
    SetValue(bm.Name.c_str(), _T("includesystem"),      bm.IncludeSystem ? _T("true") : _T("false"));
    SetValue(bm.Name.c_str(), _T("includefolder"),      bm.IncludeFolder ? _T("true") : _T("false"));
    SetValue(bm.Name.c_str(), _T("includehidden"),      bm.IncludeHidden ? _T("true") : _T("false"));
    SetValue(bm.Name.c_str(), _T("includebinary"),      bm.IncludeBinary ? _T("true") : _T("false"));
    val = _T("\"");
    val += bm.ExcludeDirs;
    val += _T("\"");
    SetValue(bm.Name.c_str(), _T("excludedirs"),        val.c_str());
    val = _T("\"");
    val += bm.FileMatch;
    val += _T("\"");
    SetValue(bm.Name.c_str(), _T("filematch"),          val.c_str());
    SetValue(bm.Name.c_str(), _T("filematchregex"),     bm.FileMatchRegex ? _T("true") : _T("false"));

}

void CBookmarks::RemoveBookmark(const std::wstring& name)
{
    Delete(name.c_str(), _T("searchString"),        true);
    Delete(name.c_str(), _T("replaceString"),       true);
    Delete(name.c_str(), _T("useregex"),            true);
    Delete(name.c_str(), _T("casesensitive"),       true);
    Delete(name.c_str(), _T("dotmatchesnewline"),   true);
    Delete(name.c_str(), _T("backup"),              true);
    Delete(name.c_str(), _T("utf8"),                true);
    Delete(name.c_str(), _T("includesystem"),       true);
    Delete(name.c_str(), _T("includefolder"),       true);
    Delete(name.c_str(), _T("includehidden"),       true);
    Delete(name.c_str(), _T("includebinary"),       true);
    Delete(name.c_str(), _T("excludedirs"),         true);
    Delete(name.c_str(), _T("filematch"),           true);
    Delete(name.c_str(), _T("filematchregex"),      true);
}

Bookmark CBookmarks::GetBookmark( const std::wstring& name )
{
    Bookmark bk;
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

    return bk;
}

