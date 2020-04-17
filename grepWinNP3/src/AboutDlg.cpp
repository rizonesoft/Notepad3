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
#include <string>

CAboutDlg::CAboutDlg(HWND hParent)
    : m_hParent(hParent)
    , m_themeCallbackId(0)
{
}

CAboutDlg::~CAboutDlg(void)
{
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
                });
            CTheme::Instance().SetThemeForDialog(*this, CTheme::Instance().IsDarkTheme());
            InitDialog(hwndDlg, IDI_GREPWIN);
            CLanguage::Instance().TranslateWindow(*this);
            TCHAR buf[MAX_PATH] = {0};
            _stprintf_s(buf, _countof(buf), _T("grepWinNP3 version %ld.%ld.%ld.%ld"), GREPWIN_VERMAJOR, GREPWIN_VERMINOR, GREPWIN_VERMICRO, GREPWIN_VERBUILD);
            SetDlgItemText(*this, IDC_VERSIONINFO, buf);
            SetDlgItemText(*this, IDC_DATE, _T(GREPWIN_VERDATE));
            m_link.ConvertStaticToHyperlink(hwndDlg, IDC_WEBLINK, _T("http://tools.stefankueng.com"));
        }
        return TRUE;
    case WM_COMMAND:
        return DoCommand(LOWORD(wParam), HIWORD(wParam));
        case WM_CLOSE:
            CTheme::Instance().RemoveRegisteredCallback(m_themeCallbackId);
        break;
        default:
            return FALSE;
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
