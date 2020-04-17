// sktoolslib - common files for SK tools

// Copyright (C) 2012-2013 - Stefan Kueng

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

#include <windows.h>

#define WM_EDITDBLCLICK (WM_USER+1)

class CEditDoubleClick
{
public:
    CEditDoubleClick(void);
    virtual ~CEditDoubleClick(void);

    BOOL Subclass(HWND hwndParent, UINT uiCtlId);

private:
    UINT     m_ctrlId;
    WNDPROC  m_pfnOrigCtlProc;

    static LRESULT CALLBACK _HyperlinkParentProc(HWND hwnd, UINT message,
                                                 WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK EditCtrlWinProc(HWND hwnd, UINT message,
                                           WPARAM wParam, LPARAM lParam);
};
