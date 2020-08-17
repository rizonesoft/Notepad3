// grepWin - regex search and replace for Windows

// Copyright (C) 2007-2008, 2012-2013, 2019-2020 - Stefan Kueng

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
#include "NameDlg.h"
#include "Theme.h"
#include <string>

#pragma warning(push)
#pragma warning(disable : 4996) // warning STL4010: Various members of std::allocator are deprecated in C++17
#include <boost/regex.hpp>
#pragma warning(pop)

CNameDlg::CNameDlg(HWND hParent)
    : m_hParent(hParent)
    , m_themeCallbackId(0)
    , m_bIncludePath(false)
{
}

CNameDlg::~CNameDlg(void)
{
}

LRESULT CNameDlg::DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
            CTheme::Instance().SetThemeForDialog(*this, CTheme::Instance().IsDarkTheme());
            CTheme::Instance().SetFontForDialog(*this, CTheme::Instance().GetDlgFontFaceName(), CTheme::Instance().GetDlgFontSize());

            InitDialog(hwndDlg, IDI_GREPWIN);
            CLanguage::Instance().TranslateWindow(*this);
            // initialize the controls

            SetDlgItemText(*this, IDC_NAME, m_name.c_str());
            SetFocus(GetDlgItem(hwndDlg, IDC_NAME));

            m_resizer.Init(hwndDlg);
            m_resizer.UseSizeGrip(!CTheme::Instance().IsDarkTheme());
        }
            return FALSE;
        case WM_COMMAND:
            return DoCommand(LOWORD(wParam), HIWORD(wParam));
        case WM_SIZE:
        {
            m_resizer.DoResize(LOWORD(lParam), HIWORD(lParam));
        }
        break;
        case WM_GETMINMAXINFO:
        {
            MINMAXINFO* mmi       = (MINMAXINFO*)lParam;
            mmi->ptMinTrackSize.x = m_resizer.GetDlgRect()->right;
            mmi->ptMinTrackSize.y = m_resizer.GetDlgRect()->bottom;
            return 0;
        }
        break;
        case WM_CLOSE:
            CTheme::Instance().RemoveRegisteredCallback(m_themeCallbackId);
            break;
        default:
            return FALSE;
    }
    return FALSE;
}

LRESULT CNameDlg::DoCommand(int id, int /*msg*/)
{
    switch (id)
    {
        case IDOK:
        {
            auto buf       = GetDlgItemText(IDC_NAME);
            m_name         = buf.get();
            m_bIncludePath = IsDlgButtonChecked(*this, IDC_INCLUDEPATH);
        }
            // fall through
        case IDCANCEL:
            EndDialog(*this, id);
            break;
    }
    return 1;
}
