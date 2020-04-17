// sktoolslib - common files for SK tools

// Copyright (C) 2012-2013, 2016, 2020 - Stefan Kueng

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
#include "DlgResizer.h"
#include <cassert>
#include <type_traits>

#ifndef ComboBox_GetEditSel
#include <windowsx.h>
#endif

CDlgResizer::CDlgResizer(void)
    : m_hDlg(nullptr)
    , m_wndGrip(nullptr)
    , m_useSizeGrip(true)
{
    m_controls.clear();
    m_dlgRect = {};
    m_dlgRectScreen = {};
    m_sizeGrip = {};
}

CDlgResizer::~CDlgResizer(void)
{
    m_controls.clear();
}


void CDlgResizer::Init(HWND hWndDlg)
{
    m_hDlg = hWndDlg;
    GetClientRect(hWndDlg, &m_dlgRect);
    GetWindowRect(hWndDlg, &m_dlgRectScreen);
    OffsetRect(&m_dlgRectScreen, -m_dlgRectScreen.left, -m_dlgRectScreen.top);

    m_sizeGrip.cx = GetSystemMetrics(SM_CXVSCROLL);
    m_sizeGrip.cy = GetSystemMetrics(SM_CYHSCROLL);

    RECT rect = { 0 , 0, m_sizeGrip.cx, m_sizeGrip.cy };

    m_wndGrip = ::CreateWindowEx(0, _T("SCROLLBAR"),
        (LPCTSTR)nullptr,
        WS_CHILD | WS_CLIPSIBLINGS | SBS_SIZEGRIP,
        rect.left, rect.top,
        rect.right-rect.left,
        rect.bottom-rect.top,
        m_hDlg,
        (HMENU)0,
        nullptr,
        nullptr);

    if (m_wndGrip)
    {
        // set a triangular window region
        HRGN rgnGrip, rgn;
        rgn = ::CreateRectRgn(0,0,1,1);
        rgnGrip = ::CreateRectRgnIndirect(&rect);

        for (int y=0; y<m_sizeGrip.cy; y++)
        {
            ::SetRectRgn(rgn, 0, y, m_sizeGrip.cx-y, y+1);
            ::CombineRgn(rgnGrip, rgnGrip, rgn, RGN_DIFF);
        }
        ::SetWindowRgn(m_wndGrip, rgnGrip, FALSE);

        // update pos
        UpdateGripPos();
        ShowSizeGrip();
    }
}

void CDlgResizer::AdjustMinMaxSize()
{
    GetWindowRect(m_hDlg, &m_dlgRectScreen);
    OffsetRect(&m_dlgRectScreen, -m_dlgRectScreen.left, -m_dlgRectScreen.top);
}

void CDlgResizer::AddControl(HWND hWndDlg, UINT ctrlId, UINT resizeType)
{
    ResizeCtrls ctrlInfo;

    ctrlInfo.hWnd = GetDlgItem(hWndDlg, ctrlId);
    if (!ctrlInfo.hWnd)
    {
        assert(false);
        return;
    }
    ctrlInfo.resizeType = resizeType;

    GetWindowRect(ctrlInfo.hWnd, &ctrlInfo.origSize);
    OffsetRect(&ctrlInfo.origSize, -ctrlInfo.origSize.left, -ctrlInfo.origSize.top);
    MapWindowPoints(ctrlInfo.hWnd, hWndDlg, (LPPOINT)&ctrlInfo.origSize, 2);

    m_controls.push_back(ctrlInfo);
}

void CDlgResizer::DoResize(int width, int height)
{
    UpdateGripPos();
    if (m_controls.empty())
        return;

    InvalidateRect(m_hDlg, nullptr, true);
    HDWP hdwp = BeginDeferWindowPos((int)m_controls.size());

    TCHAR className[257]; // WNDCLASS docs say 256 is the longest class name possible.
    std::vector<std::pair<size_t, DWORD>> savedSelections;
    for (size_t i=0; i<m_controls.size(); ++i)
    {
        const auto& ctrlInfo = m_controls[i];
        // Work around a bug in the standard combo box control that causes it to
        // incorrectly change the selection status after resizing. Without this
        // fix sometimes the combo box will show selected text after a WM_SIZE
        // resize type event even if there was no text selected before the size event.
        // The workaround is to save the current selection state before the resize and
        // to restore that state after the resize.
        int status = GetClassName(ctrlInfo.hWnd, className, (int)std::size(className));
        bool isComboBox = status > 0 &&_tcsicmp(className, _T("COMBOBOX")) == 0;
        if (isComboBox)
        {
            DWORD sel = ComboBox_GetEditSel(ctrlInfo.hWnd);
            savedSelections.push_back({ i, sel });
        }
        RECT newpos = ctrlInfo.origSize;
        switch (ctrlInfo.resizeType)
        {
        case RESIZER_TOPLEFT:
            break;  // do nothing - the original position is fine
        case RESIZER_TOPRIGHT:
            newpos.left += (width - m_dlgRect.right);
            newpos.right += (width - m_dlgRect.right);
            break;
        case RESIZER_TOPLEFTRIGHT:
            newpos.right += (width - m_dlgRect.right);
            break;
        case RESIZER_TOPLEFTBOTTOMRIGHT:
            newpos.right += (width - m_dlgRect.right);
            newpos.bottom += (height - m_dlgRect.bottom);
            break;
        case RESIZER_BOTTOMLEFT:
            newpos.top += (height - m_dlgRect.bottom);
            newpos.bottom += (height - m_dlgRect.bottom);
            break;
        case RESIZER_BOTTOMRIGHT:
            newpos.top += (height - m_dlgRect.bottom);
            newpos.bottom += (height - m_dlgRect.bottom);
            newpos.left += (width - m_dlgRect.right);
            newpos.right += (width - m_dlgRect.right);
            break;
        case RESIZER_BOTTOMLEFTRIGHT:
            newpos.top += (height - m_dlgRect.bottom);
            newpos.bottom += (height - m_dlgRect.bottom);
            newpos.right += (width - m_dlgRect.right);
            break;
        }
        hdwp = DeferWindowPos(hdwp, ctrlInfo.hWnd, nullptr, newpos.left, newpos.top,
            newpos.right-newpos.left, newpos.bottom-newpos.top,
            SWP_NOZORDER | SWP_NOACTIVATE);
    }
    EndDeferWindowPos(hdwp);
    for (const auto& selInfo : savedSelections)
    {
        size_t index = selInfo.first;
        DWORD sel = selInfo.second;
        int startSel = LOWORD(sel);
        int endSel = HIWORD(sel);
        ComboBox_SetEditSel(m_controls[index].hWnd, startSel, endSel);
    }
    UpdateGripPos();
}

void CDlgResizer::ShowSizeGrip(bool bShow)
{
    ::ShowWindow(m_wndGrip, (bShow && m_useSizeGrip) ? SW_SHOW : SW_HIDE);
}

void CDlgResizer::UpdateGripPos()
{
    RECT rect;
    ::GetClientRect(m_hDlg, &rect);

    rect.left = rect.right - m_sizeGrip.cx;
    rect.top = rect.bottom - m_sizeGrip.cy;

    // must stay below other children
    ::SetWindowPos(m_wndGrip,HWND_BOTTOM, rect.left, rect.top, 0, 0,
        SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREPOSITION);

    // maximized windows cannot be resized

    if (::IsZoomed(m_hDlg))
    {
        ::EnableWindow(m_wndGrip, FALSE);
        ShowSizeGrip(false);
    }
    else
    {
        ::EnableWindow(m_wndGrip, TRUE);
        ShowSizeGrip(true);
    }
}
