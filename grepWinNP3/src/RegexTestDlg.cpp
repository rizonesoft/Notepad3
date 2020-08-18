// grepWin - regex search and replace for Windows

// Copyright (C) 2007-2008, 2011-2013, 2019-2020 - Stefan Kueng

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
#include "RegexTestDlg.h"
#include "RegexReplaceFormatter.h"
#include "Theme.h"
#include "DarkModeHelper.h"
#include <string>
#include <Richedit.h>
#pragma warning(push)
#pragma warning(disable : 4996) // warning STL4010: Various members of std::allocator are deprecated in C++17
#include <boost/regex.hpp>
#pragma warning(pop)

CRegexTestDlg::CRegexTestDlg(HWND hParent)
    : m_hParent(hParent)
    , bDotMatchesNewline(false)
    , bCaseSensitive(false)
    , m_themeCallbackId(0)
{
}

CRegexTestDlg::~CRegexTestDlg(void)
{
}

LRESULT CRegexTestDlg::DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                    DarkModeHelper::Instance().AllowDarkModeForWindow(GetToolTipHWND(), CTheme::Instance().IsDarkTheme());
                });
            CTheme::Instance().SetThemeForDialog(*this, CTheme::Instance().IsDarkTheme());
            CTheme::Instance().SetFontForDialog(*this, CTheme::Instance().GetDlgFontFaceName(), CTheme::Instance().GetDlgFontSize());
            DarkModeHelper::Instance().AllowDarkModeForWindow(GetToolTipHWND(), CTheme::Instance().IsDarkTheme());

            InitDialog(hwndDlg, IDI_GREPWIN);
            CLanguage::Instance().TranslateWindow(*this);
            // initialize the controls
            SetDlgItemText(hwndDlg, IDC_SEARCHTEXT, m_searchText.c_str());
            SetDlgItemText(hwndDlg, IDC_REPLACETEXT, m_replaceText.c_str());

            SetFocus(GetDlgItem(hwndDlg, IDC_SEARCHTEXT));

            m_resizer.Init(hwndDlg);
            m_resizer.UseSizeGrip(!CTheme::Instance().IsDarkTheme());
            m_resizer.AddControl(hwndDlg, IDC_TEXTCONTENT, RESIZER_TOPLEFTRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_SEARCHTEXT, RESIZER_TOPLEFTRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_REPLACETEXT, RESIZER_TOPLEFTRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_REGEXMATCH, RESIZER_TOPLEFTRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_REGEXREPLACED, RESIZER_TOPLEFTBOTTOMRIGHT);
            m_resizer.AddControl(hwndDlg, IDOK, RESIZER_BOTTOMRIGHT);
            m_resizer.AddControl(hwndDlg, IDCANCEL, RESIZER_BOTTOMRIGHT);

            SendMessage(GetDlgItem(*this, IDC_TEXTCONTENT), EM_SETEVENTMASK, 0, ENM_CHANGE);
            SendMessage(GetDlgItem(*this, IDC_TEXTCONTENT), EM_EXLIMITTEXT, 0, 200 * 1024);
            SendMessage(GetDlgItem(*this, IDC_REGEXREPLACED), EM_EXLIMITTEXT, 0, 200 * 1024);
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
        case WM_TIMER:
        {
            if (wParam == ID_REGEXTIMER)
            {
                KillTimer(*this, ID_REGEXTIMER);
                DoRegex();
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

LRESULT CRegexTestDlg::DoCommand(int id, int msg)
{
    switch (id)
    {
        case IDOK:
        {
            auto buf      = GetDlgItemText(IDC_SEARCHTEXT);
            m_searchText  = buf.get();
            buf           = GetDlgItemText(IDC_REPLACETEXT);
            m_replaceText = buf.get();
        }
            // fall through
        case IDCANCEL:
            EndDialog(*this, id);
            break;
        case IDC_TEXTCONTENT:
        {
            if (msg == EN_CHANGE)
            {
                auto buf      = GetDlgItemText(IDC_TEXTCONTENT);
                m_textContent = std::wstring(buf.get());

                SetTimer(*this, ID_REGEXTIMER, 300, NULL);
            }
        }
        break;
        case IDC_SEARCHTEXT:
        {
            if (msg == EN_CHANGE)
            {
                auto buf     = GetDlgItemText(IDC_SEARCHTEXT);
                m_searchText = buf.get();

                SetTimer(*this, ID_REGEXTIMER, 300, NULL);
            }
        }
        break;
        case IDC_REPLACETEXT:
        {
            if (msg == EN_CHANGE)
            {
                auto buf      = GetDlgItemText(IDC_REPLACETEXT);
                m_replaceText = buf.get();

                SetTimer(*this, ID_REGEXTIMER, 300, NULL);
            }
        }
        break;
    }
    return 1;
}

void CRegexTestDlg::SetStrings(const std::wstring& search, const std::wstring& replace)
{
    m_replaceText = replace;
    m_searchText  = search;
}

void CRegexTestDlg::DoRegex()
{
    if (m_textContent.empty())
    {
        SetDlgItemText(*this, IDC_REGEXMATCH, TranslatedString(hResource, IDS_NOTESTTEXTAVAILABLE).c_str());
        SetDlgItemText(*this, IDC_REGEXREPLACED, TranslatedString(hResource, IDS_NOTESTTEXTAVAILABLE).c_str());
    }
    else if (m_searchText.empty())
    {
        SetDlgItemText(*this, IDC_REGEXMATCH, TranslatedString(hResource, IDS_SEARCHSTRINGEMPTY).c_str());
        SetDlgItemText(*this, IDC_REGEXREPLACED, TranslatedString(hResource, IDS_SEARCHSTRINGEMPTY).c_str());
    }
    else if (m_replaceText.empty())
    {
        SetDlgItemText(*this, IDC_REGEXREPLACED, TranslatedString(hResource, IDS_NOREPLACETEXT).c_str());
    }

    if (!m_textContent.empty())
    {
        std::wstring searchresult;
        std::wstring replaceresult;
        if (!m_searchText.empty())
        {
            std::wstring::const_iterator start, end;
            start = m_textContent.begin();
            end   = m_textContent.end();
            boost::match_results<std::wstring::const_iterator> what;
            try
            {
                int ft = boost::regex::normal;
                if (!bCaseSensitive)
                    ft |= boost::regbase::icase;
                boost::wregex                                      expression = boost::wregex(m_searchText, ft);
                boost::match_results<std::wstring::const_iterator> whatc;
                boost::match_flag_type                             flags = boost::match_default;
                if (!bDotMatchesNewline)
                    flags |= boost::match_not_dot_newline;

                boost::match_flag_type rflags = boost::match_default | boost::format_all;
                if (!bDotMatchesNewline)
                    rflags |= boost::match_not_dot_newline;

                RegexReplaceFormatter replaceFmt(m_replaceText);
                replaceFmt.SetReplacePair(L"${filepath}", L"c:\\grepwintest\\file.txt");
                replaceFmt.SetReplacePair(L"${filename}", L"file");
                replaceFmt.SetReplacePair(L"${fileext}", L"txt");

                replaceresult = regex_replace(m_textContent, expression, replaceFmt, rflags);

                while (boost::regex_search(start, end, whatc, expression, flags))
                {
                    if (!searchresult.empty())
                        searchresult = searchresult + L"\r\n----------------------------\r\n";
                    std::wstring c(whatc[0].first, whatc[0].second);
                    searchresult = searchresult + c;
                    // update search position:
                    if (start == whatc[0].second)
                    {
                        if (start == end)
                            break;
                        ++start;
                    }
                    else
                        start = whatc[0].second;
                    // update flags:
                    flags |= boost::match_prev_avail;
                    flags |= boost::match_not_bob;
                }
            }
            catch (const std::exception&)
            {
            }
            if (searchresult.empty())
                SetDlgItemText(*this, IDC_REGEXMATCH, TranslatedString(hResource, IDS_NOMATCH).c_str());
            else
                SetDlgItemText(*this, IDC_REGEXMATCH, searchresult.c_str());
        }
        if (!searchresult.empty())
            SetDlgItemText(*this, IDC_REGEXMATCH, searchresult.c_str());
        if (!replaceresult.empty())
            SetDlgItemText(*this, IDC_REGEXREPLACED, replaceresult.c_str());
    }
}
