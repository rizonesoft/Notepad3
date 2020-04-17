// sktoolslib - common files for SK tools

// Copyright (C) 2012 - Stefan Kueng

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


// does not implement HKM_SETRULES
class CHotKeyCtrl
{
public:
    CHotKeyCtrl(void);
    virtual ~CHotKeyCtrl(void);

    BOOL ConvertEditToHotKeyCtrl(HWND hwndCtl);
    BOOL ConvertEditToHotKeyCtrl(HWND hwndParent, UINT uiCtlId);

    void SetHotKey(WPARAM hk);
    WPARAM GetHotKey();

private:
    HWND    m_hWnd;
    WNDPROC m_pfnOrigCtlProc;

    static LRESULT CALLBACK _HotKeyProc(HWND hwnd, UINT message,
                                        WPARAM wParam, LPARAM lParam);

    HHOOK kb_hook;
    static LRESULT CALLBACK _KeyboardProc(int code, WPARAM wParam, LPARAM lParam);

    void SetHKText(WORD hk);

    bool controldown;
    bool shiftdown;
    bool menudown;
    bool lwindown;

    TCHAR controltext[MAX_PATH];
    TCHAR shifttext[MAX_PATH];
    TCHAR menutext[MAX_PATH];
    TCHAR lwintext[MAX_PATH];

    WORD hotkey;
};
