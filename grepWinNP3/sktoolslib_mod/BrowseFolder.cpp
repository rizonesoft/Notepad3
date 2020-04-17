// sktoolslib - common files for SK tools

// Copyright (C) 2012, 2014, 2017 - Stefan Kueng

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

BOOL CBrowseFolder::m_bCheck = FALSE;
BOOL CBrowseFolder::m_bCheck2 = FALSE;
WNDPROC CBrowseFolder::CBProc = nullptr;
HWND CBrowseFolder::checkbox = nullptr;
HWND CBrowseFolder::checkbox2 = nullptr;
HWND CBrowseFolder::ListView = nullptr;
TCHAR CBrowseFolder::m_CheckText[200];
TCHAR CBrowseFolder::m_CheckText2[200];
std::wstring CBrowseFolder::m_sDefaultPath;
bool CBrowseFolder::m_DisableCheckbox2WhenCheckbox1IsChecked = false;


CBrowseFolder::CBrowseFolder(void)
    : m_style(0)
    , m_root(nullptr)
{
    SecureZeroMemory(m_displayName, sizeof(m_displayName));
    SecureZeroMemory(m_title, sizeof(m_title));
    SecureZeroMemory(m_CheckText, sizeof(m_CheckText));
}

CBrowseFolder::~CBrowseFolder(void)
{
}

//show the dialog
CBrowseFolder::retVal CBrowseFolder::Show(HWND parent, LPTSTR path, size_t pathlen, LPCTSTR szDefaultPath /* = nullptr */)
{
    std::wstring temp;
    temp = path;
    std::wstring sDefault;
    if (szDefaultPath && PathFileExists(szDefaultPath))
        sDefault = szDefaultPath;
    CBrowseFolder::retVal ret = Show(parent, temp, sDefault);
    _tcscpy_s(path, pathlen, temp.c_str());
    return ret;
}
CBrowseFolder::retVal CBrowseFolder::Show(HWND parent, std::wstring& path, const std::wstring& sDefaultPath /* = std::wstring() */)
{
    retVal ret = OK;        //assume OK
    m_sDefaultPath = sDefaultPath;
    if (m_sDefaultPath.empty() && !path.empty())
    {
        // if the result path already contains a path, use that as the default path
        m_sDefaultPath = path;
    }

    HRESULT hr;

    // Create a new common open file dialog
    IFileOpenDialog* pfd = nullptr;
    hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
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
            TCHAR * nl = _tcschr(m_title, '\n');
            if (nl)
                *nl = 0;
            pfd->SetTitle(m_title);
        }

        // set the default folder
        if (SUCCEEDED(hr))
        {
            typedef HRESULT (WINAPI *SHCIFPN)(PCWSTR pszPath, IBindCtx * pbc, REFIID riid, void ** ppv);

            HMODULE hLib = LoadLibrary(L"shell32.dll");
            if (hLib)
            {
                SHCIFPN pSHCIFPN = (SHCIFPN)GetProcAddress(hLib, "SHCreateItemFromParsingName");
                if (pSHCIFPN)
                {
                    IShellItem *psiDefault = 0;
                    hr = pSHCIFPN(m_sDefaultPath.c_str(), nullptr, IID_PPV_ARGS(&psiDefault));
                    if (SUCCEEDED(hr))
                    {
                        hr = pfd->SetFolder(psiDefault);
                        psiDefault->Release();
                    }
                }
                FreeLibrary(hLib);
            }
        }

        if (_tcslen(m_CheckText))
        {
            IFileDialogCustomize* pfdCustomize = 0;
            hr = pfd->QueryInterface(IID_PPV_ARGS(&pfdCustomize));
            if (SUCCEEDED(hr))
            {
                pfdCustomize->StartVisualGroup(100, L"");
                pfdCustomize->AddCheckButton(101, m_CheckText, FALSE);
                if (_tcslen(m_CheckText2))
                {
                    pfdCustomize->AddCheckButton(102, m_CheckText2, FALSE);
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
            hr = pfd->GetResult(&psiResult);
            if (SUCCEEDED(hr))
            {
                PWSTR pszPath = nullptr;
                hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
                if (SUCCEEDED(hr))
                {
                    path = pszPath;
                    CoTaskMemFree(pszPath);
                }
                psiResult->Release();

                IFileDialogCustomize *  pfdCustomize;
                hr = pfd->QueryInterface(IID_PPV_ARGS(&pfdCustomize));
                if (SUCCEEDED(hr))
                {
                    pfdCustomize->GetCheckButtonState(101, &m_bCheck);
                    pfdCustomize->GetCheckButtonState(102, &m_bCheck2);
                    pfdCustomize->Release();
                }
            }
            else
                ret = CANCEL;
        }
        else
            ret = CANCEL;

        pfd->Release();
    }
    else
    {
        LPITEMIDLIST itemIDList;

        BROWSEINFO browseInfo;

        browseInfo.hwndOwner        = parent;
        browseInfo.pidlRoot         = m_root;
        browseInfo.pszDisplayName   = m_displayName;
        browseInfo.lpszTitle        = m_title;
        browseInfo.ulFlags          = m_style;
        browseInfo.lpfn             = nullptr;
        browseInfo.lParam           = (LPARAM)this;

        if ((m_CheckText[0] != '\0') || (m_sDefaultPath.size()))
        {
            browseInfo.lpfn = BrowseCallBackProc;
        }

        itemIDList = SHBrowseForFolder(&browseInfo);

        //is the dialog canceled?
        if (!itemIDList)
            ret = CANCEL;

        if (ret != CANCEL)
        {
            WCHAR p[MAX_PATH] = {0};
            if (!SHGetPathFromIDList(itemIDList, p))     // MAX_PATH ok. Explorer can't handle paths longer than MAX_PATH.
                ret = NOPATH;

            path = p;

            LPMALLOC    shellMalloc;

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

void CBrowseFolder::SetInfo(LPCTSTR title)
{
    if (title)
        _tcscpy_s(m_title, title);
}

void CBrowseFolder::SetCheckBoxText(LPCTSTR checktext)
{
    if (checktext)
        _tcscpy_s(m_CheckText, checktext);
}

void CBrowseFolder::SetCheckBoxText2(LPCTSTR checktext)
{
    if (checktext)
        _tcscpy_s(m_CheckText2, checktext);
}

void CBrowseFolder::SetFont(HWND hwnd,LPTSTR FontName,int FontSize)
{

    HFONT hf;
    LOGFONT lf={0};
    HDC hdc=GetDC(hwnd);

    GetObject(GetWindowFont(hwnd),sizeof(lf),&lf);
    lf.lfWeight = FW_REGULAR;
    lf.lfHeight = (LONG)FontSize;
    lstrcpyn( lf.lfFaceName, FontName, _countof(lf.lfFaceName) );
    hf=CreateFontIndirect(&lf);
    SetBkMode(hdc,OPAQUE);
    SendMessage(hwnd,WM_SETFONT,(WPARAM)hf,TRUE);
    ReleaseDC(hwnd,hdc);

}

int CBrowseFolder::BrowseCallBackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM /*lpData*/)
{
    RECT ListViewRect,Dialog;
    //Initialization callback message
    if (uMsg == BFFM_INITIALIZED)
    {
        if (m_CheckText[0] != '\0')
        {
            bool bSecondCheckbox = (m_CheckText2[0] != '\0');
            //Rectangles for getting the positions
            checkbox = CreateWindowEx(  0,
                _T("BUTTON"),
                m_CheckText,
                WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|BS_AUTOCHECKBOX,
                0,100,100,50,
                hwnd,
                0,
                nullptr,
                nullptr);
            if (checkbox == nullptr)
                return 0;

            if (bSecondCheckbox)
            {
                //Rectangles for getting the positions
                checkbox2 = CreateWindowEx( 0,
                    _T("BUTTON"),
                    m_CheckText2,
                    WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|BS_AUTOCHECKBOX,
                    0,100,100,50,
                    hwnd,
                    0,
                    nullptr,
                    nullptr);
                if (checkbox2 == nullptr)
                    return 0;
            }

            ListView = FindWindowEx(hwnd, nullptr,_T("SysTreeView32"), nullptr);
            if (ListView == nullptr)
                ListView = FindWindowEx(hwnd, nullptr,_T("SHBrowseForFolder ShellNameSpace Control"), nullptr);

            if (ListView == nullptr)
                return 0;

            //Gets the dimensions of the windows
            const int controlHeight = ::GetSystemMetrics(SM_CYMENUCHECK) + 4;
            GetWindowRect(hwnd,&Dialog);
            GetWindowRect(ListView,&ListViewRect);
            POINT pt;
            pt.x = ListViewRect.left;
            pt.y = ListViewRect.top;
            ScreenToClient(hwnd, &pt);
            ListViewRect.top = pt.y;
            ListViewRect.left = pt.x;
            pt.x = ListViewRect.right;
            pt.y = ListViewRect.bottom;
            ScreenToClient(hwnd, &pt);
            ListViewRect.bottom = pt.y;
            ListViewRect.right = pt.x;
            //Sets the list view controls dimensions
            SetWindowPos(ListView,0,ListViewRect.left,
                bSecondCheckbox ? ListViewRect.top+(2*controlHeight) : ListViewRect.top+controlHeight,
                (ListViewRect.right-ListViewRect.left),
                bSecondCheckbox ? (ListViewRect.bottom - ListViewRect.top)-(2*controlHeight) : (ListViewRect.bottom - ListViewRect.top)-controlHeight,
                SWP_NOZORDER);
            //Sets the window positions of checkbox and dialog controls
            SetWindowPos(checkbox,HWND_BOTTOM,ListViewRect.left,
                ListViewRect.top,
                (ListViewRect.right-ListViewRect.left),
                controlHeight,
                SWP_NOZORDER);
            if (bSecondCheckbox)
            {
                SetWindowPos(checkbox2,HWND_BOTTOM,ListViewRect.left,
                    ListViewRect.top+controlHeight,
                    (ListViewRect.right-ListViewRect.left),
                    controlHeight,
                    SWP_NOZORDER);
            }
            HWND label = FindWindowEx(hwnd, nullptr, _T("STATIC"), nullptr);
            if (label)
            {
                HFONT hFont = (HFONT)::SendMessage(label, WM_GETFONT, 0, 0);
                LOGFONT lf = {0};
                GetObject(hFont, sizeof(lf), &lf);
                HFONT hf2 = CreateFontIndirect(&lf);
                ::SendMessage(checkbox, WM_SETFONT, (WPARAM)hf2, TRUE);
                if (bSecondCheckbox)
                    ::SendMessage(checkbox2, WM_SETFONT, (WPARAM)hf2, TRUE);
            }
            else
            {
                //Sets the fonts of static controls
                SetFont(checkbox,_T("MS Sans Serif"),12);
                if (bSecondCheckbox)
                    SetFont(checkbox2,_T("MS Sans Serif"),12);
            }

            // Subclass the checkbox control.
            CBProc = (WNDPROC) SetWindowLongPtr(checkbox,GWLP_WNDPROC, (LONG_PTR) CheckBoxSubclassProc);
            //Sets the checkbox to checked position
            SendMessage(checkbox,BM_SETCHECK,(WPARAM)m_bCheck,0);
            if (bSecondCheckbox)
            {
                CBProc = (WNDPROC) SetWindowLongPtr(checkbox2,GWLP_WNDPROC, (LONG_PTR) CheckBoxSubclassProc2);
                SendMessage(checkbox2,BM_SETCHECK,(WPARAM)m_bCheck,0);
            }
            // send a resize message to the resized list view control. Otherwise it won't show
            // up properly until the user resizes the window!
            SendMessage(ListView, WM_SIZE, SIZE_RESTORED, MAKELONG(ListViewRect.right-ListViewRect.left, bSecondCheckbox ? (ListViewRect.bottom - ListViewRect.top)-40 : (ListViewRect.bottom - ListViewRect.top)-20));
        }

        // now set the default directory
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)(LPCTSTR)m_sDefaultPath.c_str());
    }
    if (uMsg == BFFM_SELCHANGED)
    {
        // Set the status window to the currently selected path.
        TCHAR szDir[MAX_PATH];
        if (SHGetPathFromIDList((LPITEMIDLIST)lParam, szDir))
        {
            SendMessage(hwnd,BFFM_SETSTATUSTEXT, 0, (LPARAM)szDir);
        }
    }

    return 0;
}

LRESULT CBrowseFolder::CheckBoxSubclassProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    if (uMsg == WM_LBUTTONUP)
    {
        m_bCheck = (SendMessage(hwnd,BM_GETCHECK,0,0) == BST_UNCHECKED);
        if (m_bCheck && m_DisableCheckbox2WhenCheckbox1IsChecked)
        {
            ::EnableWindow(checkbox2, !m_bCheck);
        }
        else
            ::EnableWindow(checkbox2, true);
    }

    return CallWindowProc(CBProc, hwnd, uMsg,
        wParam, lParam);
}

LRESULT CBrowseFolder::CheckBoxSubclassProc2(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    if (uMsg == WM_LBUTTONUP)
    {
        m_bCheck2 = (SendMessage(hwnd,BM_GETCHECK,0,0) == BST_UNCHECKED);
    }

    return CallWindowProc(CBProc, hwnd, uMsg,
        wParam, lParam);
}
