// sktoolslib - common files for SK tools

// Copyright (C) 2012-2013, 2015-2017 - Stefan Kueng

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
#include "BaseDialog.h"
#include <CommCtrl.h>
#include <WindowsX.h>

static HWND g_hDlgCurrent = nullptr;

INT_PTR CDialog::DoModal(HINSTANCE hInstance, int resID, HWND hWndParent)
{
    m_bPseudoModal = false;
    hResource = hInstance;
    return DialogBoxParam(hInstance, MAKEINTRESOURCE(resID), hWndParent, &CDialog::stDlgFunc, (LPARAM)this);
}

INT_PTR CDialog::DoModal(HINSTANCE hInstance, int resID, HWND hWndParent, UINT idAccel)
{
    m_bPseudoModal = true;
    m_bPseudoEnded = false;
    hResource = hInstance;
    m_hwnd = CreateDialogParam(hInstance, MAKEINTRESOURCE(resID), hWndParent, &CDialog::stDlgFunc, (LPARAM)this);

    // deactivate the parent window
    if (hWndParent)
        ::EnableWindow(hWndParent, FALSE);

    ShowWindow(m_hwnd, SW_SHOW);
    ::BringWindowToTop(m_hwnd);
    ::SetForegroundWindow(m_hwnd);

    // Main message loop:
    MSG msg = {0};
    HACCEL hAccelTable = LoadAccelerators(hResource, MAKEINTRESOURCE(idAccel));
    BOOL bRet = TRUE;
    while (!m_bPseudoEnded && ((bRet = GetMessage(&msg, nullptr, 0, 0)) != 0))
    {
        if (bRet == -1)
        {
            // handle the error and possibly exit
            break;
        }
        else
        {
            if (!PreTranslateMessage(&msg))
            {
                if (!TranslateAccelerator(m_hwnd, hAccelTable, &msg) &&
                    !::IsDialogMessage(m_hwnd, &msg)
                    )
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }
    }
    if (msg.message == WM_QUIT)
        PostQuitMessage((int)msg.wParam);
    // re-enable the parent window
    if (hWndParent)
        ::EnableWindow(hWndParent, TRUE);
    DestroyWindow(m_hwnd);
    if (m_bPseudoModal)
        return m_iPseudoRet;
    return msg.wParam;
}

BOOL CDialog::EndDialog(HWND hDlg, INT_PTR nResult)
{
    if (m_bPseudoModal)
    {
        m_bPseudoEnded = true;
        m_iPseudoRet = nResult;
    }
    return ::EndDialog(hDlg, nResult);
}

HWND CDialog::Create(HINSTANCE hInstance, int resID, HWND hWndParent)
{
    m_bPseudoModal = true;
    hResource = hInstance;
    m_hwnd = CreateDialogParam(hInstance, MAKEINTRESOURCE(resID), hWndParent, &CDialog::stDlgFunc, (LPARAM)this);
    return m_hwnd;
}


void CDialog::ShowModeless( HINSTANCE hInstance, int resID, HWND hWndParent )
{
    if (m_hwnd == nullptr)
    {
        hResource = hInstance;
        m_hwnd = CreateDialogParam(hInstance, MAKEINTRESOURCE(resID), hWndParent, &CDialog::stDlgFunc, (LPARAM)this);
    }
    ShowWindow(m_hwnd, SW_SHOW);
    SetFocus(m_hwnd);
}

void CDialog::InitDialog(HWND hwndDlg, UINT iconID, bool bPosition/* = true*/)
{
    HWND hwndOwner;
    RECT rc, rcDlg, rcOwner;
    WINDOWPLACEMENT placement;
    placement.length = sizeof(WINDOWPLACEMENT);

    hwndOwner = ::GetParent(hwndDlg);
    GetWindowPlacement(hwndOwner, &placement);
    if ((hwndOwner == nullptr) || (placement.showCmd == SW_SHOWMINIMIZED) || (placement.showCmd == SW_SHOWMINNOACTIVE))
        hwndOwner = ::GetDesktopWindow();

    GetWindowRect(hwndOwner, &rcOwner);
    GetWindowRect(hwndDlg, &rcDlg);
    CopyRect(&rc, &rcOwner);

    OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
    OffsetRect(&rc, -rc.left, -rc.top);
    OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);

    if (bPosition)
        SetWindowPos(hwndDlg, HWND_TOP, rcOwner.left + (rc.right / 2), rcOwner.top + (rc.bottom / 2), 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
    HICON hIcon = (HICON)::LoadImage(hResource, MAKEINTRESOURCE(iconID), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE|LR_SHARED);
    ::SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    ::SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    m_Dwm.Initialize();
    m_margins.cxLeftWidth = 0;
    m_margins.cxRightWidth = 0;
    m_margins.cyBottomHeight = 0;
    m_margins.cyTopHeight = 0;
}

void CDialog::AddToolTip(UINT ctrlID, LPCWSTR text)
{
    TOOLINFO tt;
    tt.cbSize = sizeof(TOOLINFO);
    tt.uFlags = TTF_IDISHWND|TTF_SUBCLASS;
    tt.hwnd = GetDlgItem(*this, ctrlID);
    tt.uId = (UINT_PTR)GetDlgItem(*this, ctrlID);
    tt.lpszText = const_cast<LPTSTR>(text);

    SendMessage (m_hToolTips, TTM_ADDTOOL, 0, (LPARAM) &tt);
}

void CDialog::AddToolTip(HWND hWnd, LPCWSTR text)
{
    TOOLINFO tt;
    tt.cbSize = sizeof(TOOLINFO);
    tt.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    tt.hwnd = hWnd;
    tt.uId = (UINT_PTR)hWnd;
    tt.lpszText = const_cast<LPTSTR>(text);

    SendMessage(m_hToolTips, TTM_ADDTOOL, 0, (LPARAM)&tt);
}

INT_PTR CALLBACK CDialog::stDlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static bool bInDlgProc = false;
    if (bInDlgProc)
        return FALSE;

    CDialog* pWnd;
    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            // get the pointer to the window from lpCreateParams
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
            pWnd = (CDialog*)lParam;
            pWnd->m_hwnd = hwndDlg;
            // create the tooltip control
            pWnd->m_hToolTips = CreateWindowEx(0,
                TOOLTIPS_CLASS, nullptr,
                WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
                CW_USEDEFAULT, CW_USEDEFAULT,
                CW_USEDEFAULT, CW_USEDEFAULT,
                hwndDlg,
                nullptr, pWnd->hResource,
                nullptr);

            SetWindowPos(pWnd->m_hToolTips, HWND_TOPMOST,0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            SendMessage(pWnd->m_hToolTips, TTM_SETMAXTIPWIDTH, 0, 600);
            SendMessage(pWnd->m_hToolTips, TTM_ACTIVATE, TRUE, 0);
        }
        break;
    }
    // get the pointer to the window
    pWnd = GetObjectFromWindow(hwndDlg);

    // if we have the pointer, go to the message handler of the window
    if (pWnd)
    {
        LRESULT lRes = pWnd->DlgFunc(hwndDlg, uMsg, wParam, lParam);
        switch (uMsg)
        {
        case WM_DWMCOMPOSITIONCHANGED:
            pWnd->OnCompositionChanged();
            break;
        case WM_ERASEBKGND:
            {
                if (pWnd->m_Dwm.IsDwmCompositionEnabled())
                {
                    bInDlgProc = true;
                    DefDlgProc(hwndDlg, uMsg, wParam, lParam);
                    bInDlgProc = false;
                    HDC hdc = (HDC)wParam;
                    // draw the frame margins in black
                    RECT rc;
                    GetClientRect(hwndDlg, &rc);
                    if (pWnd->m_margins.cxLeftWidth < 0)
                    {
                        SetBkColor(hdc, RGB(0,0,0));
                        ::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, nullptr, 0, nullptr);
                    }
                    else
                    {
                        SetBkColor(hdc, RGB(0,0,0));
                        RECT rect;
                        rect.left = rc.left;
                        rect.top = rc.top;
                        rect.right =  rc.left + pWnd->m_margins.cxLeftWidth;
                        rect.bottom = rc.bottom;
                        ::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, nullptr, 0, nullptr);

                        rect.left = rc.left;
                        rect.top = rc.top;
                        rect.right =  rc.right;
                        rect.bottom = rc.top + pWnd->m_margins.cyTopHeight;
                        ::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, nullptr, 0, nullptr);

                        rect.left = rc.right - pWnd->m_margins.cxRightWidth;
                        rect.top = rc.top;
                        rect.right =  rc.right;
                        rect.bottom = rc.bottom;
                        ::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);

                        rect.left = rc.left;
                        rect.top = rc.bottom - pWnd->m_margins.cyBottomHeight;
                        rect.right =  rc.right;
                        rect.bottom = rc.bottom;
                        ::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
                    }
                    lRes = TRUE;
                }
            }
            break;
        case WM_NCHITTEST:
            {
                if (pWnd->m_Dwm.IsDwmCompositionEnabled())
                {
                    POINTS pts = MAKEPOINTS(lParam);
                    POINT pt;
                    pt.x = pts.x;
                    pt.y = pts.y;
                    RECT rc;
                    GetClientRect(hwndDlg, &rc);
                    MapWindowPoints(hwndDlg, nullptr, (LPPOINT)&rc, 2);

                    if (pWnd->m_margins.cxLeftWidth < 0)
                    {

                        lRes = PtInRect(&rc, pt) ? HTCAPTION : FALSE;
                    }
                    else
                    {
                        RECT m = rc;
                        m.left += pWnd->m_margins.cxLeftWidth;
                        m.top += pWnd->m_margins.cyTopHeight;
                        m.right -= pWnd->m_margins.cxRightWidth;
                        m.bottom -= pWnd->m_margins.cyBottomHeight;
                        lRes = (PtInRect(&rc, pt) && !PtInRect(&m, pt)) ? HTCAPTION : FALSE;
                    }
                }
            }
            break;
        case WM_ACTIVATE:
            if (0 == wParam)             // becoming inactive
                g_hDlgCurrent = nullptr;
            else                         // becoming active
                g_hDlgCurrent = hwndDlg;
            break;
        default:
            break;
        }
        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, lRes);
        return lRes;
    }
    else
        return 0;
}

bool CDialog::PreTranslateMessage(MSG* /*pMsg*/)
{
    return false;
}

bool CDialog::IsCursorOverWindowBorder()
{
    RECT wrc, crc;
    GetWindowRect(*this, &wrc);
    GetClientRect(*this, &crc);
    MapWindowPoints(*this, nullptr, (LPPOINT)&crc, 2);
    DWORD pos = GetMessagePos();
    POINT pt;
    pt.x = GET_X_LPARAM(pos);
    pt.y = GET_Y_LPARAM(pos);
    return (PtInRect(&wrc, pt) && !PtInRect(&crc, pt));
}

/**
 * Wrapper around the CWnd::EnableWindow() method, but
 * makes sure that a control that has the focus is not disabled
 * before the focus is passed on to the next control.
 */
bool CDialog::DialogEnableWindow(UINT nID, bool bEnable)
{
    HWND hwndDlgItem = GetDlgItem(*this, nID);
    if (hwndDlgItem == nullptr)
        return false;
    if (bEnable)
        return !!EnableWindow(hwndDlgItem, bEnable);
    if (GetFocus() == hwndDlgItem)
    {
        SendMessage(*this, WM_NEXTDLGCTL, 0, false);
    }
    return !!EnableWindow(hwndDlgItem, bEnable);
}

void CDialog::OnCompositionChanged()
{
    if (m_Dwm.IsDwmCompositionEnabled())
    {
        m_Dwm.DwmExtendFrameIntoClientArea(*this, &m_margins);
    }
}

void CDialog::ExtendFrameIntoClientArea(UINT leftControl, UINT topControl, UINT rightControl, UINT botomControl)
{
    if (!m_Dwm.IsDwmCompositionEnabled())
        return;
    RECT rc, rc2;
    GetWindowRect(*this, &rc);
    GetClientRect(*this, &rc2);
    MapWindowPoints(*this, nullptr, (LPPOINT)&rc2, 2);

    RECT rccontrol;
    if (leftControl && leftControl != -1)
    {
        HWND hw = GetDlgItem(*this, leftControl);
        if (hw == nullptr)
            return;
        ::GetWindowRect(hw, &rccontrol);
        m_margins.cxLeftWidth = rccontrol.left - rc.left;
        m_margins.cxLeftWidth -= (rc2.left-rc.left);
    }
    else
        m_margins.cxLeftWidth = 0;

    if (topControl && topControl != -1)
    {
        HWND hw = GetDlgItem(*this, topControl);
        if (hw == nullptr)
            return;
        ::GetWindowRect(hw, &rccontrol);
        m_margins.cyTopHeight = rccontrol.top - rc.top;
        m_margins.cyTopHeight -= (rc2.top-rc.top);
    }
    else
        m_margins.cyTopHeight = 0;

    if (rightControl && rightControl != -1)
    {
        HWND hw = GetDlgItem(*this, rightControl);
        if (hw == nullptr)
            return;
        ::GetWindowRect(hw, &rccontrol);
        m_margins.cxRightWidth = rc.right - rccontrol.right;
        m_margins.cxRightWidth -= (rc.right-rc2.right);
    }
    else
        m_margins.cxRightWidth = 0;

    if (botomControl && botomControl != -1)
    {
        HWND hw = GetDlgItem(*this, botomControl);
        if (hw == nullptr)
            return;
        ::GetWindowRect(hw, &rccontrol);
        m_margins.cyBottomHeight = rc.bottom - rccontrol.bottom;
        m_margins.cyBottomHeight -= (rc.bottom-rc2.bottom);
    }
    else
        m_margins.cyBottomHeight = 0;

    if ((m_margins.cxLeftWidth == 0) &&
        (m_margins.cyTopHeight == 0) &&
        (m_margins.cxRightWidth == 0) &&
        (m_margins.cyBottomHeight == 0))
    {
        m_margins.cxLeftWidth = -1;
        m_margins.cyTopHeight = -1;
        m_margins.cxRightWidth = -1;
        m_margins.cyBottomHeight = -1;
    }
    if (m_Dwm.IsDwmCompositionEnabled())
    {
        m_Dwm.DwmExtendFrameIntoClientArea(*this, &m_margins);
    }
}

int CDialog::GetDlgItemTextLength( UINT nId )
{
    HWND hWnd = GetDlgItem(*this, nId);
    return GetWindowTextLength(hWnd);
}

std::unique_ptr<TCHAR[]> CDialog::GetDlgItemText( UINT nId )
{
    int len = GetDlgItemTextLength(nId);
    len++;
    auto buf = std::make_unique<TCHAR[]>(len);
    ::GetDlgItemText(*this, nId, buf.get(), len);
    return buf;
}

void CDialog::RefreshCursor()
{
    POINT pt;
    GetCursorPos(&pt);
    SetCursorPos(pt.x, pt.y);
}

void CDialog::ShowEditBalloon( UINT nId, LPCWSTR title, LPCWSTR text, int icon /*= TTI_ERROR*/ )
{
    EDITBALLOONTIP ebt = {0};
    ebt.cbStruct = sizeof(EDITBALLOONTIP);
    ebt.pszTitle = title;
    ebt.pszText  = text;
    ebt.ttiIcon  = icon;
    if (!::SendMessage(GetDlgItem(*this, nId), EM_SHOWBALLOONTIP, 0, (LPARAM)&ebt))
    {
        UINT uType = MB_ICONERROR;
        switch (icon)
        {
        case TTI_ERROR:
        case TTI_ERROR_LARGE:
            uType = MB_ICONERROR;
            break;
        case TTI_WARNING:
        case TTI_WARNING_LARGE:
            uType = MB_ICONWARNING;
            break;
        case TTI_INFO:
        case TTI_INFO_LARGE:
            uType = MB_ICONINFORMATION;
            break;
        case TTI_NONE:
            uType = 0;
            break;
        }
        ::MessageBox(*this, text, title, uType);
    }
}

void CDialog::SetTransparency( BYTE alpha, COLORREF color /*= 0xFF000000*/ )
{
    if (alpha == 255)
    {
        LONG_PTR exstyle = GetWindowLongPtr(*this, GWL_EXSTYLE);
        exstyle &= ~WS_EX_LAYERED;
        SetWindowLongPtr(*this, GWL_EXSTYLE, exstyle);
    }
    else
    {
        LONG_PTR exstyle = GetWindowLongPtr(*this, GWL_EXSTYLE);
        exstyle |= WS_EX_LAYERED;
        SetWindowLongPtr(*this, GWL_EXSTYLE, exstyle);
    }
    COLORREF col = color;
    DWORD flags = LWA_ALPHA;
    if (col & 0xFF000000)
    {
        col = RGB(255, 255, 255);
        flags = LWA_ALPHA;
    }
    else
    {
        flags = LWA_COLORKEY;
    }
    SetLayeredWindowAttributes(*this, col, alpha, flags);
}

BOOL CDialog::IsDialogMessage( LPMSG lpMsg )
{
    if (g_hDlgCurrent)
    {
        return ::IsDialogMessage(g_hDlgCurrent, lpMsg);
    }
    return FALSE;
}

RECT CDialog::AdjustControlSize(UINT nID)
{
    HWND hwndDlgItem = GetDlgItem(*this, nID);
    // adjust the size of the control to fit its content
    auto sControlText = GetDlgItemText(nID);
    // next step: find the rectangle the control text needs to
    // be displayed

    HDC hDC = GetWindowDC(*this);
    RECT controlrect;
    RECT controlrectorig;
    GetWindowRect(hwndDlgItem, &controlrect);
    ::MapWindowPoints(nullptr, *this, (LPPOINT)&controlrect, 2);
    controlrectorig = controlrect;
    if (hDC)
    {
        HFONT hFont = GetWindowFont(hwndDlgItem);
        HFONT hOldFont = (HFONT)SelectObject(hDC, hFont);
        if (DrawText(hDC, sControlText.get(), -1, &controlrect, DT_WORDBREAK | DT_EDITCONTROL | DT_EXPANDTABS | DT_LEFT | DT_CALCRECT))
        {
            // now we have the rectangle the control really needs
            if ((controlrectorig.right - controlrectorig.left) > (controlrect.right - controlrect.left))
            {
                // we're dealing with radio buttons and check boxes,
                // which means we have to add a little space for the checkbox
                const int checkWidth = GetSystemMetrics(SM_CXMENUCHECK) + 2 * GetSystemMetrics(SM_CXEDGE) + 3;
                controlrectorig.right = controlrectorig.left + (controlrect.right - controlrect.left) + checkWidth;
                MoveWindow(hwndDlgItem, controlrectorig.left, controlrectorig.top, controlrectorig.right - controlrectorig.left, controlrectorig.bottom - controlrectorig.top, TRUE);
            }
        }
        SelectObject(hDC, hOldFont);
        ReleaseDC(*this, hDC);
    }
    return controlrectorig;
}
