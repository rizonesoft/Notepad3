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

#pragma once

#include "AeroGlass.h"
#include "ResString.h"
#include <string>
#include <memory>

/**
 * A base window class.
 * Provides separate window message handlers for every window object based on
 * this class.
 */
class CDialog
{
public:
    CDialog()
        : hResource(nullptr)
        , m_hwnd(nullptr)
        , m_bPseudoModal(false)
        , m_bPseudoEnded(false)
        , m_iPseudoRet(0)
        , m_hToolTips(nullptr)
    {
        m_margins = {};
    }
    INT_PTR         DoModal(HINSTANCE hInstance, int resID, HWND hWndParent);
    INT_PTR         DoModal(HINSTANCE hInstance, int resID, HWND hWndParent, UINT idAccel);
    void            ShowModeless(HINSTANCE hInstance, int resID, HWND hWndParent);
    static BOOL     IsDialogMessage(LPMSG lpMsg);
    HWND            Create(HINSTANCE hInstance, int resID, HWND hWndParent);
    BOOL            EndDialog(HWND hDlg, INT_PTR nResult);
    void            AddToolTip(UINT ctrlID, LPCWSTR text);
    void            AddToolTip(HWND hWnd, LPCWSTR text);
    bool            IsCursorOverWindowBorder();
    void            RefreshCursor();
    void            ShowEditBalloon(UINT nId, LPCWSTR title, LPCWSTR text, int icon = TTI_ERROR);
    /**
     * Sets the transparency of the window.
     * \remark note that this also sets the WS_EX_LAYERED style!
     */
    void            SetTransparency(BYTE alpha, COLORREF color = 0xFF000000);

    /**
     * Wrapper around the CWnd::EnableWindow() method, but
     * makes sure that a control that has the focus is not disabled
     * before the focus is passed on to the next control.
     */
    bool            DialogEnableWindow(UINT nID, bool bEnable);
    void            OnCompositionChanged();
    void            ExtendFrameIntoClientArea(UINT leftControl, UINT topControl, UINT rightControl, UINT botomControl);
    int             GetDlgItemTextLength(UINT nId);
    std::unique_ptr<TCHAR[]> GetDlgItemText(UINT nId);

    virtual LRESULT CALLBACK DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
    virtual bool    PreTranslateMessage(MSG* pMsg);

    operator HWND() { return m_hwnd; }
    operator HWND() const { return m_hwnd; }

    HWND GetToolTipHWND() const { return m_hToolTips; }
protected:
    HINSTANCE       hResource;
    HWND            m_hwnd;
    CDwmApiImpl     m_Dwm;
    MARGINS         m_margins;

    void            InitDialog(HWND hwndDlg, UINT iconID, bool bPosition = true);
    /**
    * Adjusts the size of a checkbox or radio button control.
    * Since we always make the size of those bigger than 'necessary'
    * for making sure that translated strings can fit in those too,
    * this method can reduce the size of those controls again to only
    * fit the text.
    */
    RECT AdjustControlSize(UINT nID);

    // the real message handler
    static INT_PTR CALLBACK stDlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // returns a pointer the dialog (stored as the WindowLong)
    inline static CDialog * GetObjectFromWindow(HWND hWnd)
    {
        return (CDialog *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    }
private:
    bool        m_bPseudoModal;
    bool        m_bPseudoEnded;
    INT_PTR     m_iPseudoRet;
    HWND        m_hToolTips;
};
