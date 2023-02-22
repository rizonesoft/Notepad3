// grepWin - regex search and replace for Windows

// Copyright (C) 2007-2010, 2012-2017, 2019-2022 - Stefan Kueng

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
#include "maxpath.h"
#include "StringUtils.h"
#include "BookmarksDlg.h"
#include "NameDlg.h"
#include "Theme.h"
#include "ResString.h"
#include <string>

#pragma warning(push)
#pragma warning(disable: 4996) // warning STL4010: Various members of std::allocator are deprecated in C++17
#include <boost/regex.hpp>
#pragma warning(pop)

CBookmarksDlg::CBookmarksDlg(HWND hParent)
    : m_hParent(hParent)
    , m_bUseRegex(false)
    , m_bCaseSensitive(false)
    , m_bDotMatchesNewline(false)
    , m_bBackup(false)
    , m_bKeepFileDate(false)
    , m_bWholeWords(false)
    , m_bUtf8(false)
    , m_bForceBinary(false)
    , m_bIncludeSystem(false)
    , m_bIncludeFolder(false)
    , m_bIncludeSymLinks(false)
    , m_bIncludeHidden(false)
    , m_bIncludeBinary(false)
    , m_bFileMatchRegex(false)
    , m_themeCallbackId(0)
{
}

CBookmarksDlg::~CBookmarksDlg()
{
}

LRESULT CBookmarksDlg::DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
            InitBookmarks();

            m_resizer.Init(hwndDlg);
            m_resizer.UseSizeGrip(!CTheme::Instance().IsDarkTheme());
            m_resizer.AddControl(hwndDlg, IDC_INFOLABEL, RESIZER_TOPLEFTRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_BOOKMARKS, RESIZER_TOPLEFTBOTTOMRIGHT);
            m_resizer.AddControl(hwndDlg, IDOK, RESIZER_BOTTOMRIGHT);
            m_resizer.AddControl(hwndDlg, IDCANCEL, RESIZER_BOTTOMRIGHT);

            WINDOWPLACEMENT wpl = { 0 };
            DWORD size = sizeof(wpl);
            if (bPortable)
            {
                std::wstring sPos = g_iniFile.GetValue(L"global", L"windowposBookmarks", L"");
                if (!sPos.empty())
                {
                    auto read  = swscanf_s(sPos.c_str(), L"%d;%d;%d;%d;%d;%d;%d;%d;%d;%d",
                                          &wpl.flags, &wpl.showCmd,
                                          &wpl.ptMinPosition.x, &wpl.ptMinPosition.y,
                                          &wpl.ptMaxPosition.x, &wpl.ptMaxPosition.y,
                                          &wpl.rcNormalPosition.left, &wpl.rcNormalPosition.top,
                                          &wpl.rcNormalPosition.right, &wpl.rcNormalPosition.bottom);
                    wpl.length = size;
                    if (read == 10)
                        SetWindowPlacement(*this, &wpl);
                    else
                        ShowWindow(*this, SW_SHOW);
                }
                else
                    ShowWindow(*this, SW_SHOW);
            }
            else
            {
                if (SHGetValue(HKEY_CURRENT_USER, L"Software\\grepWinNP3", L"windowposBookmarks", REG_NONE, &wpl, &size) == ERROR_SUCCESS)
                    SetWindowPlacement(*this, &wpl);
                else
                    ShowWindow(*this, SW_SHOW);
            }
    }
        return TRUE;
    case WM_COMMAND:
        return DoCommand(LOWORD(wParam), HIWORD(wParam));
    case WM_SIZE:
        {
            m_resizer.DoResize(LOWORD(lParam), HIWORD(lParam));
        }
        break;
    case WM_GETMINMAXINFO:
        {
            MINMAXINFO* mmi       = reinterpret_cast<MINMAXINFO*>(lParam);
            mmi->ptMinTrackSize.x = m_resizer.GetDlgRect()->right;
            mmi->ptMinTrackSize.y = m_resizer.GetDlgRect()->bottom;
            return 0;
        }
        case WM_CONTEXTMENU:
        {
            long x = GET_X_LPARAM(lParam);
            long y = GET_Y_LPARAM(lParam);
            HWND hListControl = GetDlgItem(*this, IDC_BOOKMARKS);
            if (reinterpret_cast<HWND>(wParam) == hListControl)
            {
                int nCount = ListView_GetItemCount(hListControl);
                if (nCount == 0)
                    break;
                int iItem = ListView_GetSelectionMark(hListControl);
                if (iItem < 0)
                    break;

                POINT pt = {x,y};
                if ((x==-1)&&(y==-1))
                {
                    RECT rc;
                    ListView_GetItemRect(hListControl, iItem, &rc, LVIR_LABEL);
                    pt.x = (rc.right-rc.left)/2;
                    pt.y = (rc.bottom-rc.top)/2;
                    ClientToScreen(hListControl, &pt);
                }
                HMENU hMenu = LoadMenu(hResource, MAKEINTRESOURCE(IDC_BKPOPUP));
                HMENU hPopup = GetSubMenu(hMenu, 0);
                CLanguage::Instance().TranslateMenu(hPopup);
                TrackPopupMenu(hPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON, x, y, 0, *this, nullptr);
            }
        }
        break;
    case WM_NOTIFY:
        {
            if (wParam == IDC_BOOKMARKS)
            {
                LPNMITEMACTIVATE lpnmitem = reinterpret_cast<LPNMITEMACTIVATE>(lParam);
                if (lpnmitem->hdr.code == NM_DBLCLK)
                {
                    PrepareSelected();
                    SendMessage(m_hParent, WM_BOOKMARK, 0, 0);
                }
            }
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

LRESULT CBookmarksDlg::DoCommand(int id, int /*msg*/)
{
    switch (id)
    {
    case IDOK:
        {
            PrepareSelected();
        }
            [[fallthrough]];
        case IDCANCEL:
        {
            WINDOWPLACEMENT wpl = {0};
            wpl.length          = sizeof(WINDOWPLACEMENT);
            GetWindowPlacement(*this, &wpl);
            if (bPortable)
            {
                auto sPos = CStringUtils::Format(L"%d;%d;%d;%d;%d;%d;%d;%d;%d;%d",
                                                 wpl.flags, wpl.showCmd,
                                                 wpl.ptMinPosition.x, wpl.ptMinPosition.y,
                                                 wpl.ptMaxPosition.x, wpl.ptMaxPosition.y,
                                                 wpl.rcNormalPosition.left, wpl.rcNormalPosition.top, wpl.rcNormalPosition.right, wpl.rcNormalPosition.bottom);
                g_iniFile.SetValue(L"global", L"windowposBookmarks", sPos.c_str());
            }
            else
            {
                SHSetValue(HKEY_CURRENT_USER, L"Software\\grepWinNP3", L"windowposBookmarks", REG_NONE, &wpl, sizeof(wpl));
            }
            if ((id == IDOK) && (ListView_GetNextItem(GetDlgItem(*this, IDC_BOOKMARKS), -1, LVNI_SELECTED) >= 0))
                SendMessage(m_hParent, WM_BOOKMARK, 0, 0);
            ShowWindow(*this, SW_HIDE);
        }
        break;
    case ID_REMOVEBOOKMARK:
        {
            int iItem = ListView_GetNextItem(GetDlgItem(*this, IDC_BOOKMARKS), -1, LVNI_SELECTED);
            if (iItem >= 0)
            {
                auto   buf    = std::make_unique<wchar_t[]>(MAX_PATH_NEW);
                LVITEM lv     = {0};
                lv.iItem      = iItem;
                lv.mask       = LVIF_TEXT;
                lv.pszText    = buf.get();
                lv.cchTextMax = MAX_PATH_NEW;
                ListView_GetItem(GetDlgItem(*this, IDC_BOOKMARKS), &lv);
                m_bookmarks.RemoveBookmark(buf.get());
                ListView_DeleteItem(GetDlgItem(*this, IDC_BOOKMARKS), iItem);
            }
        }
        break;
    case ID_RENAMEBOOKMARK:
        {
            int iItem = ListView_GetNextItem(GetDlgItem(*this, IDC_BOOKMARKS), -1, LVNI_SELECTED);
            if (iItem >= 0)
            {
                auto   buf    = std::make_unique<wchar_t[]>(MAX_PATH_NEW);
                LVITEM lv     = {0};
                lv.iItem      = iItem;
                lv.mask       = LVIF_TEXT;
                lv.pszText    = buf.get();
                lv.cchTextMax = MAX_PATH_NEW;
                ListView_GetItem(GetDlgItem(*this, IDC_BOOKMARKS), &lv);
                CNameDlg nameDlg(*this);
                nameDlg.SetName(buf.get());
                if (nameDlg.DoModal(hResource, IDD_NAME, *this) == IDOK)
                {
                    if (nameDlg.GetName().compare(buf.get()))
                    {
                        Bookmark bk = m_bookmarks.GetBookmark(buf.get());
                        RemoveQuotes(bk.Search);
                        RemoveQuotes(bk.Replace);
                        bk.Name = nameDlg.GetName();
                        m_bookmarks.AddBookmark(bk);
                        m_bookmarks.RemoveBookmark(buf.get());
                        m_bookmarks.Save();
                        InitBookmarks();
                    }
                }
            }
        }
        break;
        default:
            break;
    }
    return 1;
}

void CBookmarksDlg::InitBookmarks()
{
    HWND hListControl = GetDlgItem(*this, IDC_BOOKMARKS);
    DWORD exStyle = LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT;
    ListView_DeleteAllItems(hListControl);

    int c = Header_GetItemCount(ListView_GetHeader(hListControl))-1;
    while (c>=0)
        ListView_DeleteColumn(hListControl, c--);

    ListView_SetExtendedListViewStyle(hListControl, exStyle);

    std::wstring sName          = TranslatedString(hResource, IDS_NAME);
    std::wstring sSearchString  = TranslatedString(hResource, IDS_SEARCHSTRING);
    std::wstring sReplaceString = TranslatedString(hResource, IDS_REPLACESTRING);

    LVCOLUMN     lvc            = {0};
    lvc.mask                    = LVCF_TEXT;
    lvc.fmt                     = LVCFMT_LEFT;
    lvc.cx                      = -1;
    lvc.pszText                 = const_cast<LPWSTR>(static_cast<LPCWSTR>(sName.c_str()));
    ListView_InsertColumn(hListControl, 0, &lvc);
    lvc.pszText = const_cast<LPWSTR>(static_cast<LPCWSTR>(sSearchString.c_str()));
    ListView_InsertColumn(hListControl, 1, &lvc);
    lvc.pszText = const_cast<LPWSTR>(static_cast<LPCWSTR>(sReplaceString.c_str()));
    ListView_InsertColumn(hListControl, 2, &lvc);

    m_bookmarks.Load();
    CSimpleIni::TNamesDepend sections;
    m_bookmarks.GetAllSections(sections);
    for (const auto & section : sections)
    {
        std::wstring searchString = m_bookmarks.GetValue(section.pItem, L"searchString", L"");
        std::wstring replaceString = m_bookmarks.GetValue(section.pItem, L"replaceString", L"");
        RemoveQuotes(searchString);
        RemoveQuotes(replaceString);

        LVITEM lv = {0};
        lv.mask   = LVIF_TEXT;
        auto pBuf = std::make_unique<wchar_t[]>(wcslen(section.pItem) + 1);
        wcscpy_s(pBuf.get(), wcslen(section.pItem) + 1, section.pItem);
        lv.pszText = pBuf.get();
        lv.iItem   = ListView_GetItemCount(hListControl);
        int ret    = ListView_InsertItem(hListControl, &lv);
        if (ret >= 0)
        {
            lv.iItem = ret;
            lv.iSubItem = 1;
            pBuf        = std::make_unique<wchar_t[]>(searchString.size() + 1);
            lv.pszText  = pBuf.get();
            wcscpy_s(lv.pszText, searchString.size() + 1, searchString.c_str());
            ListView_SetItem(hListControl, &lv);
            lv.iSubItem = 2;
            pBuf        = std::make_unique<wchar_t[]>(replaceString.size() + 1);
            lv.pszText  = pBuf.get();
            wcscpy_s(lv.pszText, replaceString.size() + 1, replaceString.c_str());
            ListView_SetItem(hListControl, &lv);
        }
    }

    ListView_SetColumnWidth(hListControl, 0, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth(hListControl, 1, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth(hListControl, 2, LVSCW_AUTOSIZE_USEHEADER);
}

void CBookmarksDlg::RemoveQuotes(std::wstring& str)
{
    if (!str.empty())
    {
        if (str[0] == '"')
            str = str.substr(1);
        if (!str.empty())
        {
            if (str[str.size()-1] == '"')
                str = str.substr(0, str.size()-1);
        }
    }
}

void CBookmarksDlg::PrepareSelected()
{
    m_bookmarks.Save();
    int iItem = ListView_GetNextItem(GetDlgItem(*this, IDC_BOOKMARKS), -1, LVNI_SELECTED);
    if (iItem >= 0)
    {
        auto   buf    = std::make_unique<wchar_t[]>(MAX_PATH_NEW);
        LVITEM lv     = {0};
        lv.iItem      = iItem;
        lv.mask       = LVIF_TEXT;
        lv.pszText    = buf.get();
        lv.cchTextMax = MAX_PATH_NEW;
        ListView_GetItem(GetDlgItem(*this, IDC_BOOKMARKS), &lv);
        m_searchString = m_bookmarks.GetValue(buf.get(), L"searchString", L"");
        m_path          = m_bookmarks.GetValue(buf.get(), L"searchpath", L"");
        m_replaceString = m_bookmarks.GetValue(buf.get(), L"replaceString", L"");
        m_sExcludeDirs = m_bookmarks.GetValue(buf.get(), L"excludedirs", L"");
        m_sFileMatch = m_bookmarks.GetValue(buf.get(), L"filematch", L"");
        RemoveQuotes(m_searchString);
        RemoveQuotes(m_replaceString);
        RemoveQuotes(m_sExcludeDirs);
        RemoveQuotes(m_sFileMatch);
        m_bUseRegex          = wcscmp(m_bookmarks.GetValue(buf.get(), L"useregex", L"false"), L"true") == 0;
        m_bCaseSensitive     = wcscmp(m_bookmarks.GetValue(buf.get(), L"casesensitive", L"false"), L"true") == 0;
        m_bDotMatchesNewline = wcscmp(m_bookmarks.GetValue(buf.get(), L"dotmatchesnewline", L"false"), L"true") == 0;
        m_bBackup            = wcscmp(m_bookmarks.GetValue(buf.get(), L"backup", L"false"), L"true") == 0;
        m_bKeepFileDate      = wcscmp(m_bookmarks.GetValue(buf.get(), L"keepfiledate", L"false"), L"true") == 0;
        m_bWholeWords        = wcscmp(m_bookmarks.GetValue(buf.get(), L"wholewords", L"false"), L"true") == 0;
        m_bUtf8              = wcscmp(m_bookmarks.GetValue(buf.get(), L"utf8", L"false"), L"true") == 0;
        m_bForceBinary       = wcscmp(m_bookmarks.GetValue(buf.get(), L"binary", L"false"), L"true") == 0;
        m_bIncludeSystem     = wcscmp(m_bookmarks.GetValue(buf.get(), L"includesystem", L"false"), L"true") == 0;
        m_bIncludeFolder     = wcscmp(m_bookmarks.GetValue(buf.get(), L"includefolder", L"false"), L"true") == 0;
        m_bIncludeSymLinks   = wcscmp(m_bookmarks.GetValue(buf.get(), L"includesymlink", L"false"), L"true") == 0;
        m_bIncludeHidden     = wcscmp(m_bookmarks.GetValue(buf.get(), L"includehidden", L"false"), L"true") == 0;
        m_bIncludeBinary     = wcscmp(m_bookmarks.GetValue(buf.get(), L"includebinary", L"false"), L"true") == 0;
        m_bFileMatchRegex    = wcscmp(m_bookmarks.GetValue(buf.get(), L"filematchregex", L"false"), L"true") == 0;
    }
}
