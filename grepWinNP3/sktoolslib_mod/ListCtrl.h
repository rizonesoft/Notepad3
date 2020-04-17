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

#include <string>


/**
 * Subclasses a list view control.
 */
class CListCtrl
{
public:
    CListCtrl();
    bool SubClassListCtrl(HWND hWnd);

    void SetInfoText(LPCTSTR sText, bool bPermanent = false);
    bool InfoTextShown() {return !m_sInfoText.empty();}


    /// static message handler to put in WNDCLASSEX structure
    static LRESULT CALLBACK stWinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    operator HWND() {return m_hwnd;}
protected:
    HWND        m_hwnd;
    WNDPROC     m_pfnOrigCtlProc;
    std::wstring m_sInfoText;
    bool        m_bInfoTextPermanent;
};
