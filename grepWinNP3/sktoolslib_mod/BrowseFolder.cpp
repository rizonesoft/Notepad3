// sktoolslib - common files for SK tools

// Copyright (C) 2012, 2014, 2017, 2020-2021 - Stefan Kueng

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
#include <WindowsX.h>
#include <Shobjidl.h>
#include <Shlobj.h>
#include <Shlwapi.h>
#include "BrowseFolder.h"

BOOL         CBrowseFolder::m_bCheck    = FALSE;
BOOL         CBrowseFolder::m_bCheck2   = FALSE;
WNDPROC      CBrowseFolder::m_cbProc    = nullptr;
HWND         CBrowseFolder::m_checkBox  = nullptr;
HWND         CBrowseFolder::m_checkBox2 = nullptr;
HWND         CBrowseFolder::m_listView  = nullptr;
wchar_t      CBrowseFolder::m_checkText[200];
wchar_t      CBrowseFolder::m_checkText2[200];
std::wstring CBrowseFolder::m_sDefaultPath;
bool         CBrowseFolder::m_disableCheckbox2WhenCheckbox1IsChecked = false;

CBrowseFolder::CBrowseFolder()
    : m_style(0)
    , m_root(nullptr)
{
    SecureZeroMemory(m_displayName, sizeof(m_displayName));
    SecureZeroMemory(m_title, sizeof(m_title));
    SecureZeroMemory(m_checkText, sizeof(m_checkText));
}

CBrowseFolder::~CBrowseFolder()
{
}

//show the dialog
CBrowseFolder::RetVal CBrowseFolder::Show(HWND parent, LPWSTR path, size_t pathLen, LPCWSTR szDefaultPath /* = nullptr */)
{
    std::wstring temp;
    temp = path;
    std::wstring sDefault;
    if (szDefaultPath && PathFileExists(szDefaultPath))
        sDefault = szDefaultPath;
    CBrowseFolder::RetVal ret = Show(parent, temp, sDefault);
    wcscpy_s(path, pathLen, temp.c_str());
    return ret;
}
CBrowseFolder::RetVal CBrowseFolder::Show(HWND parent, std::vector<std::wstring>& paths, std::wstring sDefaultPath)
{
    RetVal ret     = RetVal::Ok; //assume OK
    m_sDefaultPath = sDefaultPath;
    if (m_sDefaultPath.empty() && !paths.empty())
    {
        // if the result path already contains a path, use that as the default path
        m_sDefaultPath = paths[0];
    }
    if (!PathFileExists(m_sDefaultPath.c_str()))
        m_sDefaultPath.clear();
    paths.clear();

    // Create a new common open file dialog
    IFileOpenDialog* pfd = nullptr;
    HRESULT          hr  = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr))
    {
        // Set the dialog as a folder picker
        DWORD dwOptions;
        if (SUCCEEDED(hr = pfd->GetOptions(&dwOptions)))
        {
            hr = pfd->SetOptions(dwOptions | FOS_ALLOWMULTISELECT | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
        }

        // Set a title
        if (SUCCEEDED(hr))
        {
            wchar_t* nl = wcschr(m_title, '\n');
            if (nl)
                *nl = 0;
            pfd->SetTitle(m_title);
        }

        // set the default folder
        if (SUCCEEDED(hr) && !m_sDefaultPath.empty())
        {
            IShellItem* psiDefault = nullptr;
            hr                     = SHCreateItemFromParsingName(m_sDefaultPath.c_str(), nullptr, IID_PPV_ARGS(&psiDefault));
            if (SUCCEEDED(hr))
            {
                hr = pfd->SetFolder(psiDefault);
                psiDefault->Release();
            }
        }
    }

    if (wcslen(m_checkText))
    {
        IFileDialogCustomize* pfdCustomize = nullptr;
        hr                                 = pfd->QueryInterface(IID_PPV_ARGS(&pfdCustomize));
        if (SUCCEEDED(hr))
        {
            pfdCustomize->StartVisualGroup(100, L"");
            pfdCustomize->AddCheckButton(101, m_checkText, FALSE);
            if (wcslen(m_checkText2))
            {
                pfdCustomize->AddCheckButton(102, m_checkText2, FALSE);
            }
            pfdCustomize->EndVisualGroup();
            pfdCustomize->Release();
        }
    }

    // Show the open file dialog
    if (SUCCEEDED(hr) && SUCCEEDED(hr = pfd->Show(parent)))
    {
        IShellItemArray* psiResults = nullptr;
        hr                          = pfd->GetResults(&psiResults);
        // Get the selection from the user
        if (SUCCEEDED(hr))
        {
            DWORD resultCount = 0;
            hr                = psiResults->GetCount(&resultCount);
            if (SUCCEEDED(hr))
            {
                for (DWORD i = 0; i < resultCount; ++i)
                {
                    IShellItem* psiResult = nullptr;
                    hr                    = psiResults->GetItemAt(i, &psiResult);
                    if (SUCCEEDED(hr))
                    {
                        PWSTR pszPath = nullptr;
                        hr            = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
                        if (SUCCEEDED(hr))
                        {
                            paths.push_back(pszPath);
                            CoTaskMemFree(pszPath);
                        }
                        psiResult->Release();

                        IFileDialogCustomize* pfdCustomize;
                        hr = pfd->QueryInterface(IID_PPV_ARGS(&pfdCustomize));
                        if (SUCCEEDED(hr))
                        {
                            pfdCustomize->GetCheckButtonState(101, &m_bCheck);
                            pfdCustomize->GetCheckButtonState(102, &m_bCheck2);
                            pfdCustomize->Release();
                        }
                    }
                }
                psiResults->Release();
            }
        }
        else
            ret = RetVal::Cancel;
    }
    else
        ret = RetVal::Cancel;

    pfd->Release();

    return ret;
}

CBrowseFolder::RetVal CBrowseFolder::Show(HWND parent, std::wstring& path, const std::wstring& sDefaultPath /* = std::wstring() */)
{
    RetVal ret     = RetVal::Ok; //assume OK
    m_sDefaultPath = sDefaultPath;
    if (m_sDefaultPath.empty() && !path.empty())
    {
        // if the result path already contains a path, use that as the default path
        m_sDefaultPath = path;
    }

    // Create a new common open file dialog
    IFileOpenDialog* pfd = nullptr;
    HRESULT          hr  = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr))
    {
        // Set the dialog as a folder picker
        DWORD dwOptions;
        if (SUCCEEDED(hr = pfd->GetOptions(&dwOptions)))
        {
            hr = pfd->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
        }

        // Set a title
        if (SUCCEEDED(hr))
        {
            wchar_t* nl = wcschr(m_title, '\n');
            if (nl)
                *nl = 0;
            pfd->SetTitle(m_title);
        }

        // set the default folder
        if (SUCCEEDED(hr))
        {
            using SHCIFPN = HRESULT(WINAPI*)(PCWSTR pszPath, IBindCtx * pbc, REFIID riid, void** ppv);

            HMODULE hLib = LoadLibrary(L"shell32.dll");
            if (hLib)
            {
                SHCIFPN pShcifpn = reinterpret_cast<SHCIFPN>(GetProcAddress(hLib, "SHCreateItemFromParsingName"));
                if (pShcifpn)
                {
                    IShellItem* psiDefault = nullptr;
                    hr                     = pShcifpn(m_sDefaultPath.c_str(), nullptr, IID_PPV_ARGS(&psiDefault));
                    if (SUCCEEDED(hr))
                    {
                        hr = pfd->SetFolder(psiDefault);
                        psiDefault->Release();
                    }
                }
                FreeLibrary(hLib);
            }
        }

        if (wcslen(m_checkText))
        {
            IFileDialogCustomize* pfdCustomize = nullptr;
            hr                                 = pfd->QueryInterface(IID_PPV_ARGS(&pfdCustomize));
            if (SUCCEEDED(hr))
            {
                pfdCustomize->StartVisualGroup(100, L"");
                pfdCustomize->AddCheckButton(101, m_checkText, FALSE);
                if (wcslen(m_checkText2))
                {
                    pfdCustomize->AddCheckButton(102, m_checkText2, FALSE);
                }
                pfdCustomize->EndVisualGroup();
                pfdCustomize->Release();
            }
        }

        // Show the open file dialog
        if (SUCCEEDED(hr) && SUCCEEDED(hr = pfd->Show(parent)))
        {
            // Get the selection from the user
            IShellItem* psiResult = nullptr;
            hr                    = pfd->GetResult(&psiResult);
            if (SUCCEEDED(hr))
            {
                PWSTR pszPath = nullptr;
                hr            = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
                if (SUCCEEDED(hr))
                {
                    path = pszPath;
                    CoTaskMemFree(pszPath);
                }
                psiResult->Release();

                IFileDialogCustomize* pfdCustomize;
                hr = pfd->QueryInterface(IID_PPV_ARGS(&pfdCustomize));
                if (SUCCEEDED(hr))
                {
                    pfdCustomize->GetCheckButtonState(101, &m_bCheck);
                    pfdCustomize->GetCheckButtonState(102, &m_bCheck2);
                    pfdCustomize->Release();
                }
            }
            else
                ret = RetVal::Cancel;
        }
        else
            ret = RetVal::Cancel;

        pfd->Release();
    }
    else
    {
        BROWSEINFO browseInfo;

        browseInfo.hwndOwner      = parent;
        browseInfo.pidlRoot       = m_root;
        browseInfo.pszDisplayName = m_displayName;
        browseInfo.lpszTitle      = m_title;
        browseInfo.ulFlags        = m_style;
        browseInfo.lpfn           = nullptr;
        browseInfo.lParam         = reinterpret_cast<LPARAM>(this);

        if ((m_checkText[0] != '\0') || (m_sDefaultPath.size()))
        {
            browseInfo.lpfn = BrowseCallBackProc;
        }

        LPITEMIDLIST itemIDList = SHBrowseForFolder(&browseInfo);

        //is the dialog canceled?
        if (!itemIDList)
            ret = RetVal::Cancel;

        if (ret != RetVal::Cancel)
        {
            WCHAR p[MAX_PATH] = {0};
            if (!SHGetPathFromIDList(itemIDList, p)) // MAX_PATH ok. Explorer can't handle paths longer than MAX_PATH.
                ret = RetVal::Nopath;

            path = p;

            LPMALLOC shellMalloc;

            hr = SHGetMalloc(&shellMalloc);

            if (SUCCEEDED(hr))
            {
                //free memory
                shellMalloc->Free(itemIDList);
                //release interface
                shellMalloc->Release();
            }
        }
    }

    return ret;
}

void CBrowseFolder::SetInfo(LPCWSTR title)
{
    if (title)
        wcscpy_s(m_title, title);
}

void CBrowseFolder::SetCheckBoxText(LPCWSTR checkText) const
{
    if (checkText)
        wcscpy_s(m_checkText, checkText);
}

void CBrowseFolder::SetCheckBoxText2(LPCWSTR checkText) const
{
    if (checkText)
        wcscpy_s(m_checkText2, checkText);
}

void CBrowseFolder::SetFont(HWND hwnd, LPCWSTR fontName, int fontSize)
{
    LOGFONT lf  = {0};
    HDC     hdc = GetDC(hwnd);

    GetObject(GetWindowFont(hwnd), sizeof(lf), &lf);
    lf.lfWeight = FW_REGULAR;
    lf.lfHeight = static_cast<LONG>(fontSize);
    lstrcpyn(lf.lfFaceName, fontName, _countof(lf.lfFaceName));
    HFONT hf = CreateFontIndirect(&lf);
    SetBkMode(hdc, OPAQUE);
    SendMessage(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(hf), TRUE);
    ReleaseDC(hwnd, hdc);
}

int CBrowseFolder::BrowseCallBackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM /*lpData*/)
{
    RECT listViewRect{}, dialog{};
    //Initialization callback message
    if (uMsg == BFFM_INITIALIZED)
    {
        if (m_checkText[0] != '\0')
        {
            bool bSecondCheckbox = (m_checkText2[0] != '\0');
            //Rectangles for getting the positions
            m_checkBox = CreateWindowEx(0,
                                        WC_BUTTON,
                                        m_checkText,
                                        WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | BS_AUTOCHECKBOX,
                                        0, 100, 100, 50,
                                        hwnd,
                                        nullptr,
                                        nullptr,
                                        nullptr);
            if (m_checkBox == nullptr)
                return 0;

            if (bSecondCheckbox)
            {
                //Rectangles for getting the positions
                m_checkBox2 = CreateWindowEx(0,
                                             WC_BUTTON,
                                             m_checkText2,
                                             WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | BS_AUTOCHECKBOX,
                                             0, 100, 100, 50,
                                             hwnd,
                                             nullptr,
                                             nullptr,
                                             nullptr);
                if (m_checkBox2 == nullptr)
                    return 0;
            }

            m_listView = FindWindowEx(hwnd, nullptr, WC_TREEVIEW, nullptr);
            if (m_listView == nullptr)
                m_listView = FindWindowEx(hwnd, nullptr, L"SHBrowseForFolder ShellNameSpace Control", nullptr);

            if (m_listView == nullptr)
                return 0;

            //Gets the dimensions of the windows
            const int controlHeight = ::GetSystemMetrics(SM_CYMENUCHECK) + 4;
            GetWindowRect(hwnd, &dialog);
            GetWindowRect(m_listView, &listViewRect);
            POINT pt;
            pt.x = listViewRect.left;
            pt.y = listViewRect.top;
            ScreenToClient(hwnd, &pt);
            listViewRect.top  = pt.y;
            listViewRect.left = pt.x;
            pt.x              = listViewRect.right;
            pt.y              = listViewRect.bottom;
            ScreenToClient(hwnd, &pt);
            listViewRect.bottom = pt.y;
            listViewRect.right  = pt.x;
            //Sets the list view controls dimensions
            SetWindowPos(m_listView, nullptr, listViewRect.left,
                         bSecondCheckbox ? listViewRect.top + (2 * controlHeight) : listViewRect.top + controlHeight,
                         (listViewRect.right - listViewRect.left),
                         bSecondCheckbox ? (listViewRect.bottom - listViewRect.top) - (2 * controlHeight) : (listViewRect.bottom - listViewRect.top) - controlHeight,
                         SWP_NOZORDER);
            //Sets the window positions of checkbox and dialog controls
            SetWindowPos(m_checkBox, HWND_BOTTOM, listViewRect.left,
                         listViewRect.top,
                         (listViewRect.right - listViewRect.left),
                         controlHeight,
                         SWP_NOZORDER);
            if (bSecondCheckbox)
            {
                SetWindowPos(m_checkBox2, HWND_BOTTOM, listViewRect.left,
                             listViewRect.top + controlHeight,
                             (listViewRect.right - listViewRect.left),
                             controlHeight,
                             SWP_NOZORDER);
            }
            HWND label = FindWindowEx(hwnd, nullptr, WC_STATIC, nullptr);
            if (label)
            {
                HFONT   hFont = reinterpret_cast<HFONT>(::SendMessage(label, WM_GETFONT, 0, 0));
                LOGFONT lf    = {0};
                GetObject(hFont, sizeof(lf), &lf);
                HFONT hf2 = CreateFontIndirect(&lf);
                ::SendMessage(m_checkBox, WM_SETFONT, reinterpret_cast<WPARAM>(hf2), TRUE);
                if (bSecondCheckbox)
                    ::SendMessage(m_checkBox2, WM_SETFONT, reinterpret_cast<WPARAM>(hf2), TRUE);
            }
            else
            {
                //Sets the fonts of static controls
                SetFont(m_checkBox, L"MS Sans Serif", 12);
                if (bSecondCheckbox)
                    SetFont(m_checkBox2, L"MS Sans Serif", 12);
            }

            // Subclass the checkbox control.
            m_cbProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(m_checkBox, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(CheckBoxSubclassProc)));
            //Sets the checkbox to checked position
            SendMessage(m_checkBox, BM_SETCHECK, static_cast<WPARAM>(m_bCheck), 0);
            if (bSecondCheckbox)
            {
                m_cbProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(m_checkBox2, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(CheckBoxSubclassProc2)));
                SendMessage(m_checkBox2, BM_SETCHECK, static_cast<WPARAM>(m_bCheck), 0);
            }
            // send a resize message to the resized list view control. Otherwise it won't show
            // up properly until the user resizes the window!
            SendMessage(m_listView, WM_SIZE, SIZE_RESTORED, MAKELONG(listViewRect.right - listViewRect.left, bSecondCheckbox ? (listViewRect.bottom - listViewRect.top) - 40 : (listViewRect.bottom - listViewRect.top) - 20));
        }

        // now set the default directory
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, reinterpret_cast<LPARAM>(static_cast<LPCWSTR>(m_sDefaultPath.c_str())));
    }
    if (uMsg == BFFM_SELCHANGED)
    {
        // Set the status window to the currently selected path.
        wchar_t szDir[MAX_PATH]{};
        if (SHGetPathFromIDList(reinterpret_cast<LPITEMIDLIST>(lParam), szDir))
        {
            SendMessage(hwnd, BFFM_SETSTATUSTEXT, 0, reinterpret_cast<LPARAM>(szDir));
        }
    }

    return 0;
}

LRESULT CBrowseFolder::CheckBoxSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_LBUTTONUP)
    {
        m_bCheck = (SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_UNCHECKED);
        if (m_bCheck && m_disableCheckbox2WhenCheckbox1IsChecked)
        {
            ::EnableWindow(m_checkBox2, !m_bCheck);
        }
        else
            ::EnableWindow(m_checkBox2, true);
    }

    return CallWindowProc(m_cbProc, hwnd, uMsg,
                          wParam, lParam);
}

LRESULT CBrowseFolder::CheckBoxSubclassProc2(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_LBUTTONUP)
    {
        m_bCheck2 = (SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_UNCHECKED);
    }

    return CallWindowProc(m_cbProc, hwnd, uMsg,
                          wParam, lParam);
}
