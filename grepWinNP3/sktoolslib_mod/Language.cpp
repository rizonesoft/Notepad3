// sktoolslib - common files for SK tools

// Copyright (C) 2013, 2017-2018 - Stefan Kueng

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
#include "codecvt.h"
#include "StringUtils.h"

#include <Commctrl.h>
#include <Shlwapi.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cctype>
#include <memory>
#include <functional>

#define MAX_STRING_LENGTH   (64*1024)

inline bool PathIsExistingFile(LPCWSTR pszPath) { return (PathFileExists(pszPath) && !PathIsDirectory(pszPath)); }

bool CLanguage::LoadFile( const std::wstring& path )
{
    static std::wstring lastLangPath;

    // revert to original language
    if (_wcsicmp(lastLangPath.c_str(), path.c_str()))
    {
        std::map<std::wstring, std::wstring> langmap2;
        for (const auto& item : langmap)
        {
            langmap2[item.second] = item.first;
        }
        langmap = langmap2;
    }

    if (!PathIsExistingFile(path.c_str()))
        return false;

    lastLangPath = path;

#ifdef _MSC_VER
    const wchar_t* const filepath = path.c_str();
#else
    // since stream classes still expect the filepath in char and not wchar_t
    // we need to convert the filepath to multibyte first
    char filepath[MAX_PATH+1];
    SecureZeroMemory(filepath, sizeof(filepath));
    WideCharToMultiByte(CP_ACP, 0, path.c_str(), -1, filepath, _countof(filepath)-1, nullptr, nullptr);
#endif

    std::wifstream File;
    File.imbue(std::locale(std::locale(), new utf8_conversion()));
    try
    {
    }
    catch (std::ios_base::failure &)
    {
        return false;
    }
    File.open(filepath);
    if (!File.good())
    {
        return false;
    }
    auto line = std::make_unique<TCHAR[]>(2*MAX_STRING_LENGTH);
    std::vector<std::wstring> entry;
    do
    {
        File.getline(line.get(), 2*MAX_STRING_LENGTH);
        if (line.get()[0]==0)
        {
            //empty line means end of entry!
            std::wstring msgid;
            std::wstring msgstr;
            int type = 0;
            for (const auto& I : entry)
            {
                if (wcsncmp(I.c_str(), L"# ", 2)==0)
                {
                    //user comment
                    type = 0;
                }
                if (wcsncmp(I.c_str(), L"#.", 2)==0)
                {
                    //automatic comments
                    type = 0;
                }
                if (wcsncmp(I.c_str(), L"#,", 2)==0)
                {
                    //flag
                    type = 0;
                }
                if (wcsncmp(I.c_str(), L"msgid", 5)==0)
                {
                    //message id
                    msgid = I.c_str();
                    msgid = std::wstring(msgid.substr(7, msgid.size() - 8));

                    std::wstring s = msgid;
                    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](wint_t c) {return !iswspace(c); }));
                    type = 1;
                }
                if (wcsncmp(I.c_str(), L"msgstr", 6)==0)
                {
                    //message string
                    msgstr = I.c_str();
                    msgstr = msgstr.substr(8, msgstr.length() - 9);
                    type = 2;
                }
                if (wcsncmp(I.c_str(), L"\"", 1)==0)
                {
                    if (type == 1)
                    {
                        std::wstring temp = I.c_str();
                        temp = temp.substr(1, temp.length()-2);
                        msgid += temp;
                    }
                    if (type == 2)
                    {
                        std::wstring temp = I.c_str();
                        temp = temp.substr(1, temp.length()-2);
                        msgstr += temp;
                    }
                }
            }
            entry.clear();
            SearchReplace(msgid, L"\\\"", L"\"");
            SearchReplace(msgid, L"\\n", L"\n");
            SearchReplace(msgid, L"\\r", L"\r");
            SearchReplace(msgid, L"\\\\", L"\\");
            SearchReplace(msgstr, L"\\\"", L"\"");
            SearchReplace(msgstr, L"\\n", L"\n");
            SearchReplace(msgstr, L"\\r", L"\r");
            SearchReplace(msgstr, L"\\\\", L"\\");
            if (!msgid.empty() && !msgstr.empty())
                langmap[msgid] = msgstr;
            msgid.clear();
            msgstr.clear();
        }
        else
        {
            entry.push_back(line.get());
        }
    } while (File.gcount() > 0);
    File.close();

    return true;
}

std::wstring CLanguage::GetTranslatedString( const std::wstring& s )
{
    return GetTranslatedString(s, &langmap);
}

std::wstring CLanguage::GetTranslatedString( const std::wstring& s, std::map<std::wstring, std::wstring>* pLangMap )
{
    auto foundIt = pLangMap->find(s);
    if ((foundIt != pLangMap->end()) && (!foundIt->second.empty()))
        return foundIt->second;
    return s;
}

void CLanguage::TranslateWindow( HWND hWnd )
{
    // iterate over all windows and replace their
    // texts with the translation
    TranslateWindowProc(hWnd, (LPARAM)&langmap);
    EnumChildWindows(hWnd, TranslateWindowProc, (LPARAM)&langmap);
}

void CLanguage::TranslateMenu(HMENU hMenu)
{
    auto count = GetMenuItemCount(hMenu);
    for (int i = 0; i < count; ++i)
    {
        MENUITEMINFO mii = { 0 };
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_STRING | MIIM_SUBMENU;
        mii.dwTypeData = 0;
        if (GetMenuItemInfo(hMenu, i, MF_BYPOSITION, &mii))
        {
            if (mii.hSubMenu)
                TranslateMenu(mii.hSubMenu);
            if (mii.cch)
            {
                ++mii.cch;
                auto textbuf = std::make_unique<wchar_t[]>(mii.cch + 1);
                mii.dwTypeData = textbuf.get();
                if (GetMenuItemInfo(hMenu, i, MF_BYPOSITION, &mii))
                {
                    auto translated = GetTranslatedString(textbuf.get());
                    mii.fMask = MIIM_STRING;
                    mii.dwTypeData = const_cast<wchar_t*>(translated.c_str());
                    SetMenuItemInfo(hMenu, i, MF_BYPOSITION, &mii);
                }
            }
        }
    }
}

BOOL CALLBACK CLanguage::TranslateWindowProc( HWND hwnd, LPARAM lParam )
{
    std::map<std::wstring,std::wstring> * pLangMap = (std::map<std::wstring,std::wstring> *)lParam;
    int length = GetWindowTextLength(hwnd);
    auto text = std::make_unique<wchar_t[]>(length+1);
    std::wstring translatedString;
    if (GetWindowText(hwnd, text.get(), length+1))
    {
        translatedString = GetTranslatedString(text.get(), pLangMap);
        SetWindowText(hwnd, translatedString.c_str());
    }

    wchar_t classname[1024] = {0};
    if (GetClassName(hwnd, classname, _countof(classname)))
    {
        if (wcscmp(classname, L"ComboBox")==0)
        {
            // translate the items in the combobox
            int nSel = (int)SendMessage(hwnd, CB_GETCURSEL, 0, 0);
            int nCount = (int)SendMessage(hwnd, CB_GETCOUNT, 0, 0);
            for (int i = 0; i < nCount; ++i)
            {
                length = (int)SendMessage(hwnd, CB_GETLBTEXTLEN, i, 0);
                auto buf = std::make_unique<wchar_t[]>(length+1);
                SendMessage(hwnd, CB_GETLBTEXT, i, (LPARAM)buf.get());
                std::wstring sTranslated = GetTranslatedString(buf.get(), pLangMap);
                SendMessage(hwnd, CB_INSERTSTRING, i, (LPARAM)sTranslated.c_str());
                SendMessage(hwnd, CB_DELETESTRING, i+1, 0);
            }
            SendMessage(hwnd, CB_SETCURSEL, nSel, 0);
        }
        else if (wcscmp(classname, L"Button")==0)
        {
            LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
            if (((style & BS_GROUPBOX)==0) &&
                ((style & BS_CHECKBOX) || (style & BS_AUTORADIOBUTTON) || (style & BS_RADIOBUTTON)))
            {
                // adjust the width of checkbox and radio buttons
                HDC hDC = GetWindowDC(hwnd);
                RECT controlrect;
                RECT controlrectorig;
                GetWindowRect(hwnd, &controlrect);
                ::MapWindowPoints(nullptr, GetParent(hwnd), (LPPOINT)&controlrect, 2);
                controlrectorig = controlrect;
                if (hDC)
                {
                    HFONT hFont = GetWindowFont(hwnd);
                    HGDIOBJ hOldFont = ::SelectObject(hDC, hFont);
                    if (DrawText(hDC, translatedString.c_str(), -1, &controlrect, DT_WORDBREAK | DT_EDITCONTROL | DT_EXPANDTABS | DT_LEFT | DT_CALCRECT))
                    {
                        // now we have the rectangle the control really needs
                        if ((controlrectorig.right - controlrectorig.left) > (controlrect.right - controlrect.left))
                        {
                            // we're dealing with radio buttons and check boxes,
                            // which means we have to add a little space for the checkbox
                            // the value of 3 pixels added here is necessary in case certain visual styles have
                            // been disabled. Without this, the width is calculated too short.
                            const int checkWidth = GetSystemMetrics(SM_CXMENUCHECK) + 2*GetSystemMetrics(SM_CXEDGE) + 3;
                            controlrectorig.right = controlrectorig.left + (controlrect.right - controlrect.left) + checkWidth;
                            MoveWindow(hwnd, controlrectorig.left, controlrectorig.top, controlrectorig.right-controlrectorig.left, controlrectorig.bottom-controlrectorig.top, TRUE);
                        }
                    }
                    SelectObject(hDC, hOldFont);
                    ReleaseDC(hwnd, hDC);
                }
            }
        }
        else if (wcscmp(classname, L"SysHeader32")==0)
        {
            // translate column headers in list and other controls
            int nCount = Header_GetItemCount(hwnd);
            auto buf = std::make_unique<wchar_t[]>(270);
            for (int i = 0; i < nCount; ++i)
            {
                HDITEM hdi = {0};
                hdi.mask = HDI_TEXT;
                hdi.pszText = buf.get();
                hdi.cchTextMax = 270;
                Header_GetItem(hwnd, i, &hdi);
                std::wstring sTranslated = GetTranslatedString(buf.get(), pLangMap);
                hdi.pszText = const_cast<LPWSTR>(sTranslated.c_str());
                Header_SetItem(hwnd, i, &hdi);
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
