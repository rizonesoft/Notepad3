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

#include "stdafx.h"
#include "ListCtrl.h"

/*
* typedefs
*/
class CGlobalAtom
{
public:
    CGlobalAtom(void)
    { atom = GlobalAddAtom(TEXT("_ListControl_Object_Pointer_")
    TEXT("\\{8C1D10DA-FC2E-47bf-8281-DC0EF685B568}")); }
    ~CGlobalAtom(void)
    { DeleteAtom(atom); }

    ATOM atom;
};

/*
* Local variables
*/
static CGlobalAtom ga;

#define PROP_OBJECT_PTR         MAKEINTATOM(ga.atom)
#define PROP_ORIGINAL_PROC      MAKEINTATOM(ga.atom)

CListCtrl::CListCtrl()
    : m_pfnOrigCtlProc(NULL)
    , m_bInfoTextPermanent(false)
    , m_hwnd(NULL)
{
}

bool CListCtrl::SubClassListCtrl(HWND hWnd)
{
    m_hwnd = hWnd;
    m_pfnOrigCtlProc = (WNDPROC) GetWindowLongPtr(hWnd, GWLP_WNDPROC);
    SetProp(hWnd, PROP_OBJECT_PTR, (HANDLE)this);
    return (SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR) (WNDPROC) stWinMsgHandler) != 0);
}

void CListCtrl::SetInfoText(LPCTSTR sText, bool bPermanent /* = false */)
{
    m_bInfoTextPermanent = bPermanent;
    if (sText)
    {
        m_sInfoText = sText;
    }
    else
        m_sInfoText = _T("");
}

LRESULT CALLBACK CListCtrl::stWinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CListCtrl *pListCtrl = (CListCtrl *)GetProp(hwnd, PROP_OBJECT_PTR);

    switch (uMsg)
    {
    case WM_PAINT:
        {
            // first call the default draw
            CallWindowProc(pListCtrl->m_pfnOrigCtlProc, hwnd, uMsg, wParam, lParam);
            if (!pListCtrl->m_sInfoText.empty())
            {
                COLORREF clrText = ::GetSysColor(COLOR_WINDOWTEXT);
                COLORREF clrTextBk;
                if (IsWindowEnabled(*pListCtrl))
                    clrTextBk = ::GetSysColor(COLOR_WINDOW);
                else
                    clrTextBk = ::GetSysColor(COLOR_3DFACE);

                RECT rc;
                GetClientRect(*pListCtrl, &rc);
                HWND hWndHeader = ListView_GetHeader(*pListCtrl);
                if (hWndHeader != NULL)
                {
                    RECT rcH;
                    Header_GetItemRect(hWndHeader, 0, &rcH);
                    rc.top += rcH.bottom;
                }
                HDC hDC = GetDC(*pListCtrl);
                {
                    SetTextColor(hDC, clrText);
                    SetBkColor(hDC, clrTextBk);
                    ::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
                    rc.top += 10;
                    HGDIOBJ holdfont = SelectObject(hDC, GetStockObject(DEFAULT_GUI_FONT));
                    DrawTextEx(hDC, (LPWSTR)pListCtrl->m_sInfoText.c_str(), (int)pListCtrl->m_sInfoText.size(), &rc, DT_CENTER | DT_VCENTER |
                        DT_WORDBREAK | DT_NOPREFIX | DT_NOCLIP, NULL);
                    SelectObject(hDC, holdfont);
                }
                ReleaseDC(*pListCtrl, hDC);
            }
            return 0;
        }
        break;
    case WM_LBUTTONDBLCLK:
        {
            if ((!pListCtrl->m_bInfoTextPermanent) && (!pListCtrl->m_sInfoText.empty()))
            {
                pListCtrl->m_sInfoText.clear();
                ::InvalidateRect(*pListCtrl, NULL, FALSE);
#ifdef COMMITMONITOR_LISTCTRLDBLCLICK
                SendMessage(GetParent(*pListCtrl), COMMITMONITOR_LISTCTRLDBLCLICK, 0, 0);
#endif
                return 0;
            }
        }
        break;
    default:
        break;
    }

    return CallWindowProc(pListCtrl->m_pfnOrigCtlProc, hwnd, uMsg,
        wParam, lParam);
}
