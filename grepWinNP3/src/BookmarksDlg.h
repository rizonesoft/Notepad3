// grepWin - regex search and replace for Windows

// Copyright (C) 2007-2009, 2012-2013, 2016, 2019-2022 - Stefan Kueng

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
#include "BaseDialog.h"
#include "DlgResizer.h"
#include "Bookmarks.h"
#include <string>

#define WM_BOOKMARK (WM_APP + 20)

/**
 * bookmarks dialog.
 */
class CBookmarksDlg : public CDialog
{
public:
    CBookmarksDlg(HWND hParent);
    ~CBookmarksDlg() override;

    void         InitBookmarks();
    std::wstring GetName() const { return m_name; }
    std::wstring GetSelectedSearchString() const { return m_searchString; }
    std::wstring GetSelectedReplaceString() const { return m_replaceString; }
    std::wstring GetPath() const { return m_path; }
    bool         GetSelectedUseRegex() const { return m_bUseRegex; }
    bool         GetSelectedSearchCase() const { return m_bCaseSensitive; }
    bool         GetSelectedDotMatchNewline() const { return m_bDotMatchesNewline; }
    bool         GetSelectedBackup() const { return m_bBackup; }
    bool         GetSelectedKeepFileDate() const { return m_bKeepFileDate; }
    bool         GetSelectedWholeWords() const { return m_bWholeWords; }
    bool         GetSelectedTreatAsUtf8() const { return m_bUtf8; }
    bool         GetSelectedTreatAsBinary() const { return m_bForceBinary; }
    bool         GetSelectedIncludeSystem() const { return m_bIncludeSystem; }
    bool         GetSelectedIncludeFolder() const { return m_bIncludeFolder; }
    bool         GetSelectedIncludeSymLinks() const { return m_bIncludeSymLinks; }
    bool         GetSelectedIncludeHidden() const { return m_bIncludeHidden; }
    bool         GetSelectedIncludeBinary() const { return m_bIncludeBinary; }
    std::wstring GetSelectedExcludeDirs() const { return m_sExcludeDirs; }
    std::wstring GetSelectedFileMatch() const { return m_sFileMatch; }
    bool         GetSelectedFileMatchRegex() const { return m_bFileMatchRegex; }

protected:
    LRESULT CALLBACK DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    LRESULT          DoCommand(int id, int msg);
    static void      RemoveQuotes(std::wstring& str);
    void             PrepareSelected();

private:
    HWND         m_hParent;
    std::wstring m_name;
    CBookmarks   m_bookmarks;

    std::wstring m_searchString;
    std::wstring m_replaceString;
    std::wstring m_path;
    bool         m_bUseRegex;
    bool         m_bCaseSensitive;
    bool         m_bDotMatchesNewline;
    bool         m_bBackup;
    bool         m_bKeepFileDate;
    bool         m_bWholeWords;
    bool         m_bUtf8;
    bool         m_bForceBinary;
    bool         m_bIncludeSystem;
    bool         m_bIncludeFolder;
    bool         m_bIncludeSymLinks;
    bool         m_bIncludeHidden;
    bool         m_bIncludeBinary;
    std::wstring m_sExcludeDirs;
    std::wstring m_sFileMatch;
    bool         m_bFileMatchRegex;

    int         m_themeCallbackId;
    CDlgResizer m_resizer;
};
