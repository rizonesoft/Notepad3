// sktoolslib - common files for SK tools

// Copyright (C) 2012, 2017 - Stefan Kueng

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
#include "ProgressDlg.h"

CProgressDlg::CProgressDlg()
    : m_pIDlg(nullptr)
    , m_bValid(false)       //not valid by default
    , m_isVisible(false)
    , m_dwDlgFlags(PROGDLG_NORMAL)
    , m_hWndProgDlg(nullptr)
{
    EnsureValid();
}

CProgressDlg::~CProgressDlg()
{
    if (m_bValid)
    {
        if (m_isVisible)            //still visible, so stop first before destroying
            m_pIDlg->StopProgressDialog();

        m_pIDlg->Release();
        m_hWndProgDlg = nullptr;
    }
}

bool CProgressDlg::EnsureValid()
{
    if (!m_bValid)
    {
        HRESULT hr;

        hr = CoCreateInstance (CLSID_ProgressDialog, nullptr, CLSCTX_INPROC_SERVER,
            IID_IProgressDialog, (void**)&m_pIDlg);

        if (SUCCEEDED(hr))
            m_bValid = true;                //instance successfully created
    }
    return m_bValid;
}

void CProgressDlg::SetTitle(LPCWSTR szTitle)
{
    if (m_bValid)
    {
        m_pIDlg->SetTitle(szTitle);
    }
}

void CProgressDlg::SetLine(DWORD dwLine, LPCWSTR szText, bool bCompactPath /* = false */)
{
    if (m_bValid)
    {
        m_pIDlg->SetLine(dwLine, szText, bCompactPath, nullptr);
    }
}

#ifdef _MFC_VER
void CProgressDlg::SetCancelMsg ( UINT idMessage )
{
    SetCancelMsg(CString(MAKEINTRESOURCE(idMessage)));
}
#endif // _MFC_VER

void CProgressDlg::SetCancelMsg(LPCWSTR szMessage)
{
    if (m_bValid)
    {
        m_pIDlg->SetCancelMsg(szMessage, nullptr);
    }
}

void CProgressDlg::SetAnimation(HINSTANCE hinst, UINT uRsrcID)
{
    if (m_bValid)
    {
        m_pIDlg->SetAnimation(hinst, uRsrcID);
    }
}

#ifdef _MFC_VER
void CProgressDlg::SetAnimation(UINT uRsrcID)
{
    if (m_bValid)
    {
        m_pIDlg->SetAnimation(AfxGetResourceHandle(), uRsrcID);
    }
}
#endif

void CProgressDlg::SetTime(bool bTime /* = true */)
{
    m_dwDlgFlags &= ~(PROGDLG_NOTIME | PROGDLG_AUTOTIME);

    if (bTime)
        m_dwDlgFlags |= PROGDLG_AUTOTIME;
    else
        m_dwDlgFlags |= PROGDLG_NOTIME;
}

void CProgressDlg::SetShowProgressBar(bool bShow /* = true */)
{
    if (bShow)
        m_dwDlgFlags &= ~PROGDLG_NOPROGRESSBAR;
    else
        m_dwDlgFlags |= PROGDLG_NOPROGRESSBAR;
}

#ifdef _MFC_VER
HRESULT CProgressDlg::ShowModal (CWnd* pwndParent)
{
    EnsureValid();
    return ShowModal(pwndParent->GetSafeHwnd());
}

HRESULT CProgressDlg::ShowModeless(CWnd* pwndParent)
{
    EnsureValid();
    return ShowModeless(pwndParent->GetSafeHwnd());
}

void CProgressDlg::FormatPathLine ( DWORD dwLine, UINT idFormatText, ...)
{
    va_list args;
    va_start(args, idFormatText);

    CString sText;
    sText.FormatV(CString(MAKEINTRESOURCE(idFormatText)), args);
    SetLine(dwLine, sText, true);

    va_end(args);
}

void CProgressDlg::FormatNonPathLine(DWORD dwLine, UINT idFormatText, ...)
{
    va_list args;
    va_start(args, idFormatText);

    CString sText;
    sText.FormatV(CString(MAKEINTRESOURCE(idFormatText)), args);
    SetLine(dwLine, sText, false);

    va_end(args);
}
#endif

HRESULT CProgressDlg::ShowModal (HWND hWndParent)
{
    EnsureValid();
    if (m_bValid)
    {
        HRESULT hr;
        hr = m_pIDlg->StartProgressDialog(hWndParent,
                                          nullptr,
                                          m_dwDlgFlags | PROGDLG_MODAL,
                                          nullptr);

        if (SUCCEEDED(hr))
        {
            m_isVisible = true;
        }
        return hr;
    }
    return E_FAIL;
}

HRESULT CProgressDlg::ShowModeless(HWND hWndParent)
{
    EnsureValid();
    HRESULT hr = E_FAIL;
    m_hWndProgDlg = nullptr;
    if (m_bValid)
    {
        hr = m_pIDlg->StartProgressDialog(hWndParent, nullptr, m_dwDlgFlags, nullptr);

        if (SUCCEEDED(hr))
        {
            m_isVisible = true;

            // The progress window can be remarkably slow to display, particularly
            // if its parent is blocked.
            // This process finds the hwnd for the progress window and gives it a kick...
            IOleWindow *pOleWindow;
            HRESULT hr2 = m_pIDlg->QueryInterface(IID_IOleWindow,(LPVOID *)&pOleWindow);
            if (SUCCEEDED(hr2))
            {
                hr2 = pOleWindow->GetWindow(&m_hWndProgDlg);
                if (SUCCEEDED(hr2))
                {
                    ShowWindow(m_hWndProgDlg, SW_NORMAL);
                }
                pOleWindow->Release();
            }
        }
    }
    return hr;
}

void CProgressDlg::SetProgress(DWORD dwProgress, DWORD dwMax)
{
    if (m_bValid)
    {
        m_pIDlg->SetProgress(dwProgress, dwMax);
    }
}

void CProgressDlg::SetProgress64(ULONGLONG u64Progress, ULONGLONG u64ProgressMax)
{
    if (m_bValid)
    {
        m_pIDlg->SetProgress64(u64Progress, u64ProgressMax);
    }
}

bool CProgressDlg::HasUserCancelled()
{
    if (m_bValid)
    {
        return (0 != m_pIDlg->HasUserCancelled());
    }
    return FALSE;
}

void CProgressDlg::Stop()
{
    if ((m_isVisible) && (m_bValid))
    {
        m_pIDlg->StopProgressDialog();
        // Sometimes the progress dialog sticks around after stopping it,
        // until the mouse pointer is moved over it or some other triggers.
        // We hide the window here immediately.
        if (m_hWndProgDlg)
        {
            ShowWindow(m_hWndProgDlg, SW_HIDE);
        }
        m_isVisible = false;
        m_pIDlg->Release();
        m_bValid = false;
        m_hWndProgDlg = nullptr;
    }
}

void CProgressDlg::ResetTimer()
{
    if (m_bValid)
    {
        m_pIDlg->Timer(PDTIMER_RESET, nullptr);
    }
}
