// sktoolslib - common files for SK tools

// Copyright (C) 2012-2013, 2020-2021 - Stefan Kueng

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
#include "EditDoubleClick.h"

#define PROP_OBJECT_PTR MAKEINTATOM(ga.atom)

/*
 * typedefs
 */
class CGlobalAtom
{
public:
    CGlobalAtom()
#ifdef _WIN64
    {
        atom = GlobalAddAtom(L"_EditDoubleClick_Object_Pointer64_"
                             L"\\{FDE85769-946B-45EF-866C-B1EA2F26B171}");
    }
#else
    {
        atom = GlobalAddAtom(L"_EditDoubleClick_Object_Pointer_"
                             L"\\{FDE85769-946B-45EF-866C-B1EA2F26B171}");
    }
#endif
    ~CGlobalAtom()
    {
        DeleteAtom(atom);
    }

    ATOM atom;
};

static CGlobalAtom ga;

/////////////////////////////////////////////////////////////////////////////
// CEditDoubleClick

CEditDoubleClick::CEditDoubleClick()
    : m_ctrlId(0)
    , m_pfnOrigCtlProc(nullptr)
{
}

CEditDoubleClick::~CEditDoubleClick()
{
}

BOOL CEditDoubleClick::Subclass(HWND hwndParent, UINT uiCtlId)
{
    m_ctrlId     = uiCtlId;
    HWND hwndCtl = GetDlgItem(hwndParent, uiCtlId);

    // Subclass the existing control.

    m_pfnOrigCtlProc = reinterpret_cast<WNDPROC>(GetWindowLongPtr(hwndCtl, GWLP_WNDPROC));
    SetProp(hwndCtl, PROP_OBJECT_PTR, static_cast<HANDLE>(this));
    SetWindowLongPtr(hwndCtl, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(EditCtrlWinProc));

    return TRUE;
}

LRESULT CALLBACK CEditDoubleClick::EditCtrlWinProc(HWND hwnd, UINT message,
                                                   WPARAM wParam, LPARAM lParam)
{
    CEditDoubleClick *pCtrl = static_cast<CEditDoubleClick *>(GetProp(hwnd, PROP_OBJECT_PTR));

    switch (message)
    {
        case WM_LBUTTONDBLCLK:
            SendMessage(GetParent(hwnd), WM_EDITDBLCLICK, static_cast<WPARAM>(pCtrl->m_ctrlId), lParam);
            break;
        case WM_DESTROY:
        {
            SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(pCtrl->m_pfnOrigCtlProc));

            RemoveProp(hwnd, PROP_OBJECT_PTR);
        }
        break;
    }

    return CallWindowProc(pCtrl->m_pfnOrigCtlProc, hwnd, message,
                          wParam, lParam);
}
