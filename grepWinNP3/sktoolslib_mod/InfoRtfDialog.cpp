// sktoolslib - common files for SK tools

// Copyright (C) 2020 - Stefan Kueng

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
#include "InfoRtfDialog.h"
#include "OnOutOfScope.h"
#include <richedit.h>

CInfoRtfDialog::CInfoRtfDialog()
    : m_hParent(nullptr)
    , m_rtfId(0)
    , m_iconId(0)
    , m_hwndRichEdit(nullptr)
{
    m_richEditLib = LoadLibrary(TEXT("Msftedit.dll"));
}

CInfoRtfDialog::~CInfoRtfDialog(void)
{
}

INT_PTR CInfoRtfDialog::DoModal(HINSTANCE hInstance, HWND hParent, const std::string& dlgTitle, UINT rtfId, const std::wstring& resType, UINT iconId, int width, int height)
{
    m_hParent    = hParent;
    m_rtfId      = rtfId;
    m_rtfResType = resType;
    m_iconId     = iconId;

    auto hgbl = GlobalAlloc(GMEM_ZEROINIT, 1024);
    OnOutOfScope(GlobalFree(hgbl));
    if (!hgbl)
        return -1;

    auto lpdt = (LPDLGTEMPLATE)GlobalLock(hgbl);

    // Define a dialog box.

    lpdt->style = WS_POPUP | WS_BORDER | WS_SYSMENU | DS_MODALFRAME | WS_CAPTION | WS_SIZEBOX;
    lpdt->cdit  = 0; // Number of controls
    lpdt->x     = 10;
    lpdt->y     = 10;
    lpdt->cx    = (short)width;
    lpdt->cy    = (short)height;

    auto lpw = (LPWORD)(lpdt + 1);
    *lpw++   = 0; // No menu
    *lpw++   = 0; // Predefined dialog box class (by default)

    auto lpwsz = (LPWSTR)lpw;
    auto nchar = 1 + MultiByteToWideChar(CP_UTF8, 0, dlgTitle.c_str(), -1, lpwsz, 50);
    lpw += nchar;
    GlobalUnlock(hgbl);
    return __super::DoModal(hInstance, lpdt, hParent);
}

void CInfoRtfDialog::ShowModeless(HINSTANCE hInstance, HWND hParent, const std::string& dlgTitle, UINT rtfId, const std::wstring& resType, UINT iconId, int width, int height)
{
    m_hParent    = hParent;
    m_rtfId      = rtfId;
    m_rtfResType = resType;
    m_iconId     = iconId;

    auto hgbl = GlobalAlloc(GMEM_ZEROINIT, 1024);
    OnOutOfScope(GlobalFree(hgbl));
    if (!hgbl)
        return;

    auto lpdt = (LPDLGTEMPLATE)GlobalLock(hgbl);

    // Define a dialog box.

    lpdt->style = WS_POPUP | WS_BORDER | WS_SYSMENU | DS_MODALFRAME | WS_CAPTION | WS_SIZEBOX;
    lpdt->cdit  = 0; // Number of controls
    lpdt->x     = 10;
    lpdt->y     = 10;
    lpdt->cx    = (short)width;
    lpdt->cy    = (short)height;

    auto lpw = (LPWORD)(lpdt + 1);
    *lpw++   = 0; // No menu
    *lpw++   = 0; // Predefined dialog box class (by default)

    auto lpwsz = (LPWSTR)lpw;
    auto nchar = 1 + MultiByteToWideChar(CP_UTF8, 0, dlgTitle.c_str(), -1, lpwsz, 50);
    lpw += nchar;
    GlobalUnlock(hgbl);
    __super::ShowModeless(hInstance, lpdt, hParent);
}

LRESULT CInfoRtfDialog::DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            InitDialog(hwndDlg, m_iconId);
            RECT rc = {0};
            GetClientRect(*this, &rc);
            m_hwndRichEdit = CreateWindowEx(0, MSFTEDIT_CLASS, L"",
                                            ES_MULTILINE | WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | ES_NOOLEDRAGDROP | ES_SAVESEL | ES_READONLY,
                                            0, 0, rc.right - rc.left, rc.bottom - rc.top,
                                            *this, nullptr, hResource, nullptr);

            auto hRes = FindResource(hResource, MAKEINTRESOURCE(m_rtfId), m_rtfResType.c_str());
            if (hRes)
            {
                auto hResourceLoaded = LoadResource(hResource, hRes);
                if (hResourceLoaded)
                {
                    auto lpResLock = (const char*)LockResource(hResourceLoaded);
                    auto reslen    = SizeofResource(hResource, hRes);
                    if (reslen)
                    {
                        SETTEXTEX stt = {0};
                        stt.codepage  = CP_UTF8;
                        stt.flags     = ST_DEFAULT | ST_NEWCHARS;
                        SendMessage(m_hwndRichEdit, EM_SETTEXTEX, (WPARAM)&stt, (LPARAM)lpResLock);
                        SetFocus(m_hwndRichEdit);
                        SendMessage(m_hwndRichEdit, EM_SETSEL, (WPARAM)-1, (LPARAM)0);
                        SendMessage(m_hwndRichEdit, EM_SETREADONLY, (WPARAM)1, (LPARAM) nullptr);
                    }
                }
            }

            return (INT_PTR)TRUE;
        }
        case WM_SIZE:
        {
            UINT width  = LOWORD(lParam);
            UINT height = HIWORD(lParam);
            MoveWindow(m_hwndRichEdit, 0, 0, width, height, TRUE);
        }
        break;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                case IDCANCEL:
                    CloseWindow(m_hwndRichEdit);
                    DestroyWindow(m_hwndRichEdit);
                    EndDialog(*this, LOWORD(wParam));
                    return (INT_PTR)TRUE;
            }
            break;
    }
    return (INT_PTR)FALSE;
}
