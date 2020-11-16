// sktoolslib - common files for SK tools

// Copyright (C) 2020 - Stefan Kueng

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
#include "SmartHandle.h"
#include <string>

/**
* bookmarks dialog.
*/
class CInfoRtfDialog : public CDialog
{
public:
    CInfoRtfDialog();
    ~CInfoRtfDialog(void);

    INT_PTR DoModal(HINSTANCE hInstance, HWND hParent, const std::string& dlgTitle, UINT rtfId, const std::wstring& resType, UINT iconId, int width, int height);
    void    ShowModeless(HINSTANCE hInstance, HWND hParent, const std::string& dlgTitle, UINT rtfId, const std::wstring& resType, UINT iconId, int width, int height);

protected:
    LRESULT CALLBACK DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HWND         m_hParent;
    HWND         m_hwndRichEdit;
    UINT         m_rtfId;
    std::wstring m_rtfResType;
    UINT         m_iconId;
    CAutoLibrary m_richEditLib;
};
