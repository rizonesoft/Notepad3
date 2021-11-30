// sktoolslib - common files for SK tools

// Copyright (C) 2020-2021 - Stefan Kueng

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
#include <shellapi.h>

#pragma comment(lib, "Shell32.lib")

CInfoRtfDialog::CInfoRtfDialog()
    : m_hParent(nullptr)
    , m_hwndRichEdit(nullptr)
    , m_rtfId(0)
    , m_iconId(0)
{
    m_richEditLib = LoadLibrary(TEXT("Msftedit.dll"));
}

CInfoRtfDialog::~CInfoRtfDialog()
{
}

INT_PTR CInfoRtfDialog::DoModal(HINSTANCE hInstance, HWND hParent, const std::string& dlgTitle, UINT rtfId, const std::wstring& resType, UINT iconId, int width, int height)
{
    return DoModal(hInstance, hParent, dlgTitle, rtfId, resType, iconId, 10, 10, width, height);
}

INT_PTR CInfoRtfDialog::DoModal(HINSTANCE hInstance, HWND hParent, const std::string& dlgTitle, UINT rtfId, const std::wstring& resType, UINT iconId, int x, int y, int width, int height)
{
    m_hParent    = hParent;
    m_rtfId      = rtfId;
    m_rtfResType = resType;
    m_iconId     = iconId;

    auto hGbl = GlobalAlloc(GMEM_ZEROINIT, 1024);
    OnOutOfScope(GlobalFree(hGbl));
    if (!hGbl)
        return -1;

    auto lpDt = static_cast<LPDLGTEMPLATE>(GlobalLock(hGbl));

    // Define a dialog box.

    lpDt->style = WS_POPUP | WS_BORDER | WS_SYSMENU | DS_MODALFRAME | WS_CAPTION | WS_SIZEBOX;
    lpDt->cdit  = 0; // Number of controls
    lpDt->x     = static_cast<SHORT>(x);
    lpDt->y     = static_cast<SHORT>(y);
    lpDt->cx    = static_cast<short>(width);
    lpDt->cy    = static_cast<short>(height);

    auto lpw = reinterpret_cast<LPWORD>(lpDt + 1);
    *lpw++   = 0; // No menu
    *lpw++   = 0; // Predefined dialog box class (by default)

    auto lpWsz = reinterpret_cast<LPWSTR>(lpw);
    auto nchar = 1 + MultiByteToWideChar(CP_UTF8, 0, dlgTitle.c_str(), -1, lpWsz, 50);
    lpw += nchar;
    GlobalUnlock(hGbl);
    return __super::DoModal(hInstance, lpDt, hParent);
}

void CInfoRtfDialog::ShowModeless(HINSTANCE hInstance, HWND hParent, const std::string& dlgTitle, UINT rtfId, const std::wstring& resType, UINT iconId, int width, int height)
{
    m_hParent    = hParent;
    m_rtfId      = rtfId;
    m_rtfResType = resType;
    m_iconId     = iconId;

    auto hGbl = GlobalAlloc(GMEM_ZEROINIT, 1024);
    OnOutOfScope(GlobalFree(hGbl));
    if (!hGbl)
        return;

    auto lpDt = static_cast<LPDLGTEMPLATE>(GlobalLock(hGbl));

    // Define a dialog box.

    lpDt->style = WS_POPUP | WS_BORDER | WS_SYSMENU | DS_MODALFRAME | WS_CAPTION | WS_SIZEBOX;
    lpDt->cdit  = 0; // Number of controls
    lpDt->x     = 10;
    lpDt->y     = 10;
    lpDt->cx    = static_cast<short>(width);
    lpDt->cy    = static_cast<short>(height);

    auto lpw = reinterpret_cast<LPWORD>(lpDt + 1);
    *lpw++   = 0; // No menu
    *lpw++   = 0; // Predefined dialog box class (by default)

    auto lpWsz = reinterpret_cast<LPWSTR>(lpw);
    auto nchar = 1 + MultiByteToWideChar(CP_UTF8, 0, dlgTitle.c_str(), -1, lpWsz, 50);
    lpw += nchar;
    GlobalUnlock(hGbl);
    __super::ShowModeless(hInstance, lpDt, hParent);
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
                    auto lpResLock = static_cast<const char*>(LockResource(hResourceLoaded));
                    auto resLen    = SizeofResource(hResource, hRes);
                    if (resLen)
                    {
                        SETTEXTEX stt = {0};
                        stt.codepage  = CP_UTF8;
                        stt.flags     = ST_DEFAULT | ST_NEWCHARS;
                        SendMessage(m_hwndRichEdit, EM_SETTEXTEX, reinterpret_cast<WPARAM>(&stt), reinterpret_cast<LPARAM>(lpResLock));
                        SetFocus(m_hwndRichEdit);
                        SendMessage(m_hwndRichEdit, EM_SETSEL, static_cast<WPARAM>(-1), static_cast<LPARAM>(0));
                        SendMessage(m_hwndRichEdit, EM_SETREADONLY, static_cast<WPARAM>(1), reinterpret_cast<LPARAM>(nullptr));
                        SendMessage(m_hwndRichEdit, EM_SETEVENTMASK, NULL, ENM_LINK | ENM_SCROLL);
                    }
                }
            }

            return static_cast<INT_PTR>(TRUE);
        }
        case WM_SIZE:
        {
            UINT width  = LOWORD(lParam);
            UINT height = HIWORD(lParam);
            MoveWindow(m_hwndRichEdit, 0, 0, width, height, TRUE);
        }
        break;
        case WM_DESTROY:
            CloseWindow(m_hwndRichEdit);
            DestroyWindow(m_hwndRichEdit);
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                case IDCANCEL:
                    EndDialog(*this, LOWORD(wParam));
                    return static_cast<INT_PTR>(TRUE);
                default:
                    break;
            }
            break;
        case WM_NOTIFY:
        {
            auto pHdr = reinterpret_cast<LPNMHDR>(lParam);
            if (pHdr)
            {
                if (pHdr->hwndFrom == m_hwndRichEdit)
                {
                    switch (pHdr->code)
                    {
                        case EN_LINK:
                        {
                            auto pEnLink = reinterpret_cast<ENLINK*>(lParam);
                            if ((pEnLink->msg != WM_LBUTTONUP) && (pEnLink->msg != WM_SETCURSOR))
                                break;

                            auto      buffer = std::make_unique<wchar_t[]>(pEnLink->chrg.cpMax - pEnLink->chrg.cpMin + 1);
                            TEXTRANGE range{};
                            range.chrg      = pEnLink->chrg;
                            range.lpstrText = buffer.get();
                            SendMessage(m_hwndRichEdit, EM_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&range));
                            auto url = std::wstring(buffer.get(), pEnLink->chrg.cpMax - pEnLink->chrg.cpMin);
                            if (!url.empty())
                            {
                                if (pEnLink->msg == WM_SETCURSOR)
                                    SetCursor(LoadCursor(nullptr, IDC_HAND));
                                else
                                    ShellExecute(hwndDlg, L"open", url.c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
                            }
                        }
                        break;
                    }
                }
            }
        }
        break;
    }
    return static_cast<INT_PTR>(FALSE);
}