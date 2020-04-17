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

#include "stdafx.h"
#include "EditDoubleClick.h"


#define PROP_OBJECT_PTR         MAKEINTATOM(ga.atom)

/*
 * typedefs
 */
class CGlobalAtom
{
public:
    CGlobalAtom(void)
#ifdef _WIN64
    { atom = GlobalAddAtom(TEXT("_EditDoubleClick_Object_Pointer64_")
             TEXT("\\{FDE85769-946B-45EF-866C-B1EA2F26B171}")); }
#else
    { atom = GlobalAddAtom(TEXT("_EditDoubleClick_Object_Pointer_")
             TEXT("\\{FDE85769-946B-45EF-866C-B1EA2F26B171}")); }
#endif
    ~CGlobalAtom(void)
    { DeleteAtom(atom); }

    ATOM atom;
};

static CGlobalAtom ga;


/////////////////////////////////////////////////////////////////////////////
// CEditDoubleClick

CEditDoubleClick::CEditDoubleClick(void)
    : m_pfnOrigCtlProc(NULL)
    , m_ctrlId(0)
{
}

CEditDoubleClick::~CEditDoubleClick(void)
{
}

BOOL CEditDoubleClick::Subclass(HWND hwndParent, UINT uiCtlId)
{
    m_ctrlId = uiCtlId;
    HWND hwndCtl = GetDlgItem(hwndParent, uiCtlId);

    // Subclass the existing control.

    m_pfnOrigCtlProc = (WNDPROC) GetWindowLongPtr(hwndCtl, GWLP_WNDPROC);
    SetProp(hwndCtl, PROP_OBJECT_PTR, (HANDLE) this);
    SetWindowLongPtr(hwndCtl, GWLP_WNDPROC, (LONG_PTR) EditCtrlWinProc);

    return TRUE;
}

LRESULT CALLBACK CEditDoubleClick::EditCtrlWinProc(HWND hwnd, UINT message,
                                           WPARAM wParam, LPARAM lParam)
{
    CEditDoubleClick *pCtrl = (CEditDoubleClick *)GetProp(hwnd, PROP_OBJECT_PTR);

    switch (message)
    {
    case WM_LBUTTONDBLCLK:
        SendMessage(GetParent(hwnd), WM_EDITDBLCLICK, (WPARAM)pCtrl->m_ctrlId, lParam);
        break;
    case WM_DESTROY:
        {
            SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR) pCtrl->m_pfnOrigCtlProc);

            RemoveProp(hwnd, PROP_OBJECT_PTR);
        }
        break;
    }

    return CallWindowProc(pCtrl->m_pfnOrigCtlProc, hwnd, message,
                          wParam, lParam);
}
