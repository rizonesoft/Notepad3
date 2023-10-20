// sktoolslib - common files for SK tools

// Copyright (C) 2013, 2017-2018, 2020-2022 - Stefan Kueng

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
#include "Language.h"
#include "StringUtils.h"
#include "UnicodeUtils.h"

#include <Commctrl.h>
#include <Shlwapi.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cctype>
#include <memory>
#include <functional>
#include <Uxtheme.h>

#include "DPIAware.h"

#define MAX_STRING_LENGTH (64 * 1024)

bool CLanguage::LoadFile(const std::wstring& path)
{
    static std::wstring lastLangPath;

    // revert to original language
    if (_wcsicmp(lastLangPath.c_str(), path.c_str()))
    {
        std::map<std::wstring, std::wstring> langMap2;
        for (auto it = langmap.cbegin(); it != langmap.cend(); ++it)
        {
            langMap2[it->second] = it->first;
        }
        langmap = langMap2;
    }

    if (!PathFileExists(path.c_str()))
        return false;

    lastLangPath = path;

    // since stream classes still expect the filepath in char and not wchar_t
    // we need to convert the filepath to multibyte first

    std::ifstream file;
    try
    {
    }
    catch (std::ios_base::failure&)
    {
        return false;
    }
    file.open(path);
    if (!file.good())
    {
        return false;
    }
    auto                      line = std::make_unique<char[]>(2 * MAX_STRING_LENGTH);
    std::vector<std::wstring> entry;
    do
    {
        file.getline(line.get(), 2 * MAX_STRING_LENGTH);
        if (line.get()[0] == 0)
        {
            // empty line means end of entry!
            std::wstring msgId;
            std::wstring msgStr;
            int          type = 0;
            for (auto I = entry.begin(); I != entry.end(); ++I)
            {
                if (wcsncmp(I->c_str(), L"# ", 2) == 0)
                {
                    // user comment
                    type = 0;
                }
                if (wcsncmp(I->c_str(), L"#.", 2) == 0)
                {
                    // automatic comments
                    type = 0;
                }
                if (wcsncmp(I->c_str(), L"#,", 2) == 0)
                {
                    // flag
                    type = 0;
                }
                if (wcsncmp(I->c_str(), L"msgid", 5) == 0)
                {
                    // message id
                    msgId          = I->c_str();
                    msgId          = std::wstring(msgId.substr(7, msgId.size() - 8));

                    std::wstring s = msgId;
                    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](wint_t c) { return !iswspace(c); }));
                    type = 1;
                }
                if (wcsncmp(I->c_str(), L"msgstr", 6) == 0)
                {
                    // message string
                    msgStr = I->c_str();
                    msgStr = msgStr.substr(8, msgStr.length() - 9);
                    type   = 2;
                }
                if (wcsncmp(I->c_str(), L"\"", 1) == 0)
                {
                    if (type == 1)
                    {
                        std::wstring temp = I->c_str();
                        temp              = temp.substr(1, temp.length() - 2);
                        msgId += temp;
                    }
                    if (type == 2)
                    {
                        std::wstring temp = I->c_str();
                        temp              = temp.substr(1, temp.length() - 2);
                        msgStr += temp;
                    }
                }
            }
            entry.clear();
            SearchReplace(msgId, L"\\\"", L"\"");
            SearchReplace(msgId, L"\\n", L"\n");
            SearchReplace(msgId, L"\\r", L"\r");
            SearchReplace(msgId, L"\\\\", L"\\");
            SearchReplace(msgStr, L"\\\"", L"\"");
            SearchReplace(msgStr, L"\\n", L"\n");
            SearchReplace(msgStr, L"\\r", L"\r");
            SearchReplace(msgStr, L"\\\\", L"\\");
            if (!msgId.empty() && !msgStr.empty())
                langmap[msgId] = msgStr;
            msgId.clear();
            msgStr.clear();
        }
        else
        {
            //~entry.push_back(CUnicodeUtils::StdGetUnicode(line.get()));
            entry.push_back(UTF8ToWide(line.get()));
        }
    } while (file.gcount() > 0);
    file.close();

    return true;
}

std::wstring CLanguage::GetTranslatedString(const std::wstring& s)
{
    return GetTranslatedString(s, &langmap);
}

std::wstring CLanguage::GetTranslatedString(const std::wstring& s, std::map<std::wstring, std::wstring>* pLangMap)
{
    auto foundIt = pLangMap->find(s);
    if ((foundIt != pLangMap->end()) && (!foundIt->second.empty()))
        return foundIt->second;
    return s;
}

void CLanguage::TranslateWindow(HWND hWnd)
{
    // iterate over all windows and replace their
    // texts with the translation
    TranslateWindowProc(hWnd, reinterpret_cast<LPARAM>(&langmap));
    EnumChildWindows(hWnd, TranslateWindowProc, reinterpret_cast<LPARAM>(&langmap));
    EnumThreadWindows(GetCurrentThreadId(), TranslateWindowProc, reinterpret_cast<LPARAM>(&langmap));
    HMENU hSysMenu = GetSystemMenu(hWnd, FALSE);
    if (hSysMenu)
    {
        TranslateMenu(hSysMenu);
    }
}

void CLanguage::TranslateMenu(HMENU hMenu)
{
    auto count = GetMenuItemCount(hMenu);
    for (int i = 0; i < count; ++i)
    {
        MENUITEMINFO mii = {0};
        mii.cbSize       = sizeof(MENUITEMINFO);
        mii.fMask        = MIIM_STRING | MIIM_SUBMENU;
        mii.dwTypeData   = nullptr;
        if (GetMenuItemInfo(hMenu, i, MF_BYPOSITION, &mii))
        {
            if (mii.hSubMenu)
                TranslateMenu(mii.hSubMenu);
            if (mii.cch)
            {
                ++mii.cch;
                auto textBuf   = std::make_unique<wchar_t[]>(mii.cch + 1);
                mii.dwTypeData = textBuf.get();
                if (GetMenuItemInfo(hMenu, i, MF_BYPOSITION, &mii))
                {
                    auto translated = GetTranslatedString(textBuf.get());
                    mii.fMask       = MIIM_STRING;
                    mii.dwTypeData  = const_cast<wchar_t*>(translated.c_str());
                    SetMenuItemInfo(hMenu, i, MF_BYPOSITION, &mii);
                }
            }
        }
    }
}

BOOL CALLBACK CLanguage::TranslateWindowProc(HWND hwnd, LPARAM lParam)
{
    std::map<std::wstring, std::wstring>* pLangMap = reinterpret_cast<std::map<std::wstring, std::wstring>*>(lParam);
    int                                   length   = GetWindowTextLength(hwnd);
    auto                                  text     = std::make_unique<wchar_t[]>(length + 1);
    std::wstring                          translatedString;
    if (GetWindowText(hwnd, text.get(), length + 1))
    {
        translatedString = GetTranslatedString(text.get(), pLangMap);
        if (translatedString != text.get())
            SetWindowText(hwnd, translatedString.c_str());
    }

    wchar_t className[1024] = {0};
    if (GetClassName(hwnd, className, _countof(className)))
    {
        if ((wcscmp(className, WC_COMBOBOX) == 0) ||
            (wcscmp(className, WC_COMBOBOXEX) == 0))
        {
            // translate the items in the combobox
            int nSel   = static_cast<int>(SendMessage(hwnd, CB_GETCURSEL, 0, 0));
            int nCount = static_cast<int>(SendMessage(hwnd, CB_GETCOUNT, 0, 0));
            for (int i = 0; i < nCount; ++i)
            {
                length   = static_cast<int>(SendMessage(hwnd, CB_GETLBTEXTLEN, i, 0));
                auto buf = std::make_unique<wchar_t[]>(length + 1);
                SendMessage(hwnd, CB_GETLBTEXT, i, reinterpret_cast<LPARAM>(buf.get()));
                std::wstring sTranslated = GetTranslatedString(buf.get(), pLangMap);
                SendMessage(hwnd, CB_INSERTSTRING, i, reinterpret_cast<LPARAM>(sTranslated.c_str()));
                SendMessage(hwnd, CB_DELETESTRING, i + 1, 0);
            }
            SendMessage(hwnd, CB_SETCURSEL, nSel, 0);
        }
        else if (wcscmp(className, WC_BUTTON) == 0)
        {
            LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
            if (((style & BS_GROUPBOX) == 0) &&
                ((style & BS_CHECKBOX) || (style & BS_AUTORADIOBUTTON) || (style & BS_RADIOBUTTON)))
            {
                // adjust the width of checkbox and radio buttons
                HDC  hDC = GetWindowDC(hwnd);
                RECT controlRect;
                RECT controlRectOrig;
                GetWindowRect(hwnd, &controlRect);
                ::MapWindowPoints(nullptr, GetParent(hwnd), reinterpret_cast<LPPOINT>(&controlRect), 2);
                controlRectOrig = controlRect;
                if (hDC)
                {
                    HFONT   hFont    = GetWindowFont(hwnd);
                    HGDIOBJ hOldFont = ::SelectObject(hDC, hFont);
                    if (DrawText(hDC, translatedString.c_str(), -1, &controlRect, DT_WORDBREAK | DT_EXPANDTABS | DT_LEFT | DT_CALCRECT))
                    {
                        // we're dealing with radio buttons and check boxes,
                        // which means we have to add a little space for the checkbox
                        const int checkWidth = GetSystemMetrics(SM_CXMENUCHECK) + 2 * GetSystemMetrics(SM_CXEDGE) + CDPIAware::Instance().Scale(hwnd, 3);
                        controlRect.right += checkWidth;
                        // now we have the rectangle the control really needs
                        if ((controlRectOrig.right - controlRectOrig.left) > (controlRect.right - controlRect.left))
                        {
                            controlRectOrig.right = controlRectOrig.left + (controlRect.right - controlRect.left);
                            MoveWindow(hwnd, controlRectOrig.left, controlRectOrig.top, controlRectOrig.right - controlRectOrig.left, controlRectOrig.bottom - controlRectOrig.top, TRUE);
                        }
                    }
                    SelectObject(hDC, hOldFont);
                    ReleaseDC(hwnd, hDC);
                }
            }
        }
        else if (wcscmp(className, WC_HEADER) == 0)
        {
            // translate column headers in list and other controls
            int  nCount = Header_GetItemCount(hwnd);
            auto buf    = std::make_unique<wchar_t[]>(270);
            for (int i = 0; i < nCount; ++i)
            {
                HDITEM hdi     = {0};
                hdi.mask       = HDI_TEXT;
                hdi.pszText    = buf.get();
                hdi.cchTextMax = 270;
                Header_GetItem(hwnd, i, &hdi);
                std::wstring sTranslated = GetTranslatedString(buf.get(), pLangMap);
                hdi.pszText              = const_cast<LPWSTR>(sTranslated.c_str());
                Header_SetItem(hwnd, i, &hdi);
            }
        }
        else if (wcscmp(className, WC_EDIT) == 0)
        {
            // translate hint texts in edit controls
            constexpr int bufCount = 4096;
            auto          buf      = std::make_unique<wchar_t[]>(bufCount);
            SecureZeroMemory(buf.get(), bufCount * sizeof(wchar_t));
            Edit_GetCueBannerText(hwnd, buf.get(), bufCount);
            auto sTranslated = GetTranslatedString(buf.get(), pLangMap);
            Edit_SetCueBannerText(hwnd, buf.get());
        }
        else if (wcscmp(className, TOOLTIPS_CLASS) == 0)
        {
            constexpr int bufCount  = 4096;
            auto          buf       = std::make_unique<wchar_t[]>(bufCount);
            auto          toolCount = static_cast<int>(SendMessage(hwnd, TTM_GETTOOLCOUNT, 0, 0));
            for (int i = 0; i < toolCount; ++i)
            {
                SecureZeroMemory(buf.get(), bufCount * sizeof(wchar_t));
                TOOLINFO tt = {0};
                tt.cbSize   = sizeof(TOOLINFO);
                tt.lpszText = buf.get();
                SendMessage(hwnd, TTM_ENUMTOOLS, i, reinterpret_cast<LPARAM>(&tt));

                auto sTranslated = GetTranslatedString(buf.get(), pLangMap);
                tt.lpszText      = sTranslated.data();
                if (tt.lpszText[0])
                    SendMessage(hwnd, TTM_SETTOOLINFO, 0, reinterpret_cast<LPARAM>(&tt));
            }
        }
    }

    return TRUE;
}

CLanguage& CLanguage::Instance()
{
    static CLanguage instance;
    return instance;
}
