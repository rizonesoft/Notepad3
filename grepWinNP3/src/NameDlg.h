// grepWin - regex search and replace for Windows

// Copyright (C) 2007-2008, 2012-2013, 2019-2021 - Stefan Kueng

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
#include <string>

/**
 * name dialog.
 */
class CNameDlg : public CDialog
{
public:
    CNameDlg(HWND hParent);
    ~CNameDlg() override;

    std::wstring GetName() const { return m_name; }
    void         SetName(const std::wstring& n) { m_name = n; }
    bool         IncludePath() const { return m_bIncludePath; }

protected:
    LRESULT CALLBACK DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    LRESULT          DoCommand(int id, int msg);

private:
    HWND         m_hParent;
    std::wstring m_name;
    bool         m_bIncludePath;
    int          m_themeCallbackId;
    CDlgResizer  m_resizer;
};
