// grepWin - regex search and replace for Windows

// Copyright (C) 2007-2013, 2018, 2020 - Stefan Kueng

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
#include "resource.h"
#include "AboutDlg.h"
#include "version.h"
#include "Theme.h"
#include "DPIAware.h"
#include <string>

extern HICON g_hDlgIcon128;

CAboutDlg::CAboutDlg(HWND hParent)
    : m_hParent(hParent)
    , m_themeCallbackId(0)
{
}

CAboutDlg::~CAboutDlg(void)
{
}


static LRESULT DrawGrepWinIcon(HWND hWnd)
{
    if (g_hDlgIcon128)
    {
        PAINTSTRUCT ps;
        HDC const  hDC  = GetDC(hWnd);
        if (hDC)
        {
            BeginPaint(hWnd, &ps);
            int const    dpiSized  = CDPIAware::Instance().Scale(hWnd, 64);
            int const    dpiLeft   = CDPIAware::Instance().Scale(hWnd, 12);
            int const    dpiTop    = CDPIAware::Instance().Scale(hWnd, 12);
            HBRUSH const hBrush    = (HBRUSH)GetSysColorBrush(COLOR_3DFACE);
            DrawIconEx(hDC, dpiLeft, dpiTop, g_hDlgIcon128, dpiSized, dpiSized, 0, hBrush, DI_NORMAL);
            ReleaseDC(hWnd, hDC);
            EndPaint(hWnd, &ps);
        }
    }
    return FALSE;
}


LRESULT CAboutDlg::DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            m_themeCallbackId = CTheme::Instance().RegisterThemeChangeCallback(
                [this]() {
                    CTheme::Instance().SetThemeForDialog(*this, CTheme::Instance().IsDarkTheme());
                    CTheme::Instance().SetFontForDialog(*this, CTheme::Instance().GetDlgFontFaceName(), CTheme::Instance().GetDlgFontSize());
                });
            m_link.ConvertStaticToHyperlink(hwndDlg, IDC_WEBLINK, L"http://tools.stefankueng.com");
            CTheme::Instance().SetThemeForDialog(*this, CTheme::Instance().IsDarkTheme());
            CTheme::Instance().SetFontForDialog(*this, CTheme::Instance().GetDlgFontFaceName(), CTheme::Instance().GetDlgFontSize());
            InitDialog(hwndDlg, IDI_GREPWIN);
            CLanguage::Instance().TranslateWindow(*this);
            TCHAR buf[MAX_PATH] = {0};
#if defined(_WIN64)
            _stprintf_s(buf, _countof(buf), L"grepWinNP3 (x64) version %ld.%ld.%ld.%ld", GREPWIN_VERMAJOR, GREPWIN_VERMINOR, GREPWIN_VERMICRO, GREPWIN_VERBUILD);
#else
            _stprintf_s(buf, _countof(buf), L"grepWinNP3 (x86) version %ld.%ld.%ld.%ld", GREPWIN_VERMAJOR, GREPWIN_VERMINOR, GREPWIN_VERMICRO, GREPWIN_VERBUILD);
#endif
            SetDlgItemText(*this, IDC_VERSIONINFO, buf);
            SetDlgItemText(*this, IDC_DATE, TEXT(GREPWIN_VERDATE));
        }
        return TRUE;

    case WM_COMMAND:
        return DoCommand(LOWORD(wParam), HIWORD(wParam));
        case WM_CLOSE:
            CTheme::Instance().RemoveRegisteredCallback(m_themeCallbackId);
        break;
        default:
            return FALSE;

    case WM_PAINT:
        return DrawGrepWinIcon(*this);

    }
    return FALSE;
}

LRESULT CAboutDlg::DoCommand(int id, int /*msg*/)
{
    switch (id)
    {
    case IDOK:
        // fall through
    case IDCANCEL:
        EndDialog(*this, id);
        break;
    }
    return 1;
}
