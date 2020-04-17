// sktoolslib - common files for SK tools

// Copyright (C) 2012 - Stefan Kueng

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
#include "ShellLink.h"




//////////////// Macros / Locals /////////////////////////////////////

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



//////////////// Implementation //////////////////////////////////////

CShellLinkInfo::CShellLinkInfo()
    : m_pidl(NULL)
    , m_wHotkey(0)
    , m_nIconIndex(0)
    , m_nShowCmd(SW_SHOW)
{
}

CShellLinkInfo::CShellLinkInfo(const CShellLinkInfo& sli)
{
    *this = sli;
}

CShellLinkInfo::~CShellLinkInfo()
{
    // Get the shell's allocator.
    IMalloc* pMalloc;
    HRESULT hRes = SHGetMalloc(&pMalloc);
    if (!SUCCEEDED(hRes))
    {
        return;
    }

    //Free the pidl
    if (m_pidl)
    {
        pMalloc->Free(m_pidl);
        m_pidl = NULL;
    }

    // Release the pointer to IMalloc
    pMalloc->Release();
}

CShellLinkInfo& CShellLinkInfo::operator=(const CShellLinkInfo& sli)
{
    m_sTarget = sli.m_sTarget;
    m_pidl = sli.m_pidl;
    m_sArguments = sli.m_sArguments;
    m_sDescription = sli.m_sDescription;
    m_wHotkey = sli.m_wHotkey;
    m_sIconLocation = sli.m_sIconLocation;
    m_nIconIndex = sli.m_nIconIndex;
    m_nShowCmd = sli.m_nShowCmd;
    m_sWorkingDirectory = sli.m_sWorkingDirectory;

    return *this;
}

CShellLink::CShellLink()
    : m_psl(NULL)
    , m_ppf(NULL)
    , m_bAttemptedInitialise(FALSE)
{
}

CShellLink::~CShellLink()
{
    if (m_ppf)
    {
        m_ppf->Release();
        m_ppf = NULL;
    }
    if (m_psl)
    {
        m_psl->Release();
        m_psl = NULL;
    }
}

BOOL CShellLink::Initialise()
{
    BOOL bSuccess = FALSE;
    if (m_bAttemptedInitialise)
        bSuccess = (m_psl != NULL);
    else
    {
        //Instantiate the COM class
        HRESULT hRes = ::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*) &m_psl);
        if (SUCCEEDED(hRes))
        {
            //Also get a pointer to IPersistFile
            hRes = m_psl->QueryInterface(IID_IPersistFile, (LPVOID*) &m_ppf);
            bSuccess = SUCCEEDED(hRes);
        }

        m_bAttemptedInitialise = TRUE;
    }

    return bSuccess;
}

BOOL CShellLink::Create(const CShellLinkInfo& sli)
{
    if (!Initialise())
        return FALSE;

    m_sli = sli;

    return TRUE;
}

BOOL CShellLink::Save(const std::wstring& sFilename)
{
    if (!Initialise())
        return FALSE;

    BOOL bSuccess = FALSE;
    HRESULT hRes;

    //Convert the path to a UNICODE string
    WCHAR wszPath[MAX_PATH];

    _tcscpy_s(wszPath, _countof(wszPath), sFilename.c_str());

    //Set the various link values
    if (m_sli.m_pidl)
    {
        hRes = m_psl->SetIDList(m_sli.m_pidl);
    }
    else
    {
        hRes = m_psl->SetPath(m_sli.m_sTarget.c_str());
    }

    hRes = m_psl->SetWorkingDirectory(m_sli.m_sWorkingDirectory.c_str());

    hRes = m_psl->SetIconLocation(m_sli.m_sIconLocation.c_str(), m_sli.m_nIconIndex);

    hRes = m_psl->SetDescription(m_sli.m_sDescription.c_str());

    hRes = m_psl->SetArguments(m_sli.m_sArguments.c_str());

    hRes = m_psl->SetHotkey(m_sli.m_wHotkey);

    hRes = m_psl->SetShowCmd(m_sli.m_nShowCmd);

    //Save the link to file
    hRes = m_ppf->Save(wszPath, TRUE);
    if (SUCCEEDED(hRes))
        bSuccess = TRUE;

    return bSuccess;
}

BOOL CShellLink::Load(const std::wstring& sFilename)
{
    if (!Initialise())
        return FALSE;

    BOOL bSuccess = FALSE;

    //Convert the path to a UNICODE string
    WCHAR wszPath[MAX_PATH];

    _tcscpy_s(wszPath, _countof(wszPath), sFilename.c_str());

    //Load the link from file
    HRESULT hRes = m_ppf->Load(wszPath, STGM_READ);
    if (SUCCEEDED(hRes))
    {
        //Get the various link values
        TCHAR szBuf[MAX_PATH];
        WIN32_FIND_DATA fd;
        SecureZeroMemory(&fd, sizeof(fd));

        hRes = m_psl->GetPath(szBuf, _countof(szBuf), &fd, SLGP_UNCPRIORITY);
        if (SUCCEEDED(hRes))
            m_sli.m_sTarget = szBuf;

        hRes = m_psl->GetIDList(&m_sli.m_pidl);

        hRes = m_psl->GetWorkingDirectory(szBuf, _countof(szBuf));
        if (SUCCEEDED(hRes))
            m_sli.m_sWorkingDirectory = szBuf;

        hRes = m_psl->GetIconLocation(szBuf, _countof(szBuf), &m_sli.m_nIconIndex);
        if (SUCCEEDED(hRes))
            m_sli.m_sIconLocation = szBuf;

        hRes = m_psl->GetDescription(szBuf, _countof(szBuf));
        if (SUCCEEDED(hRes))
            m_sli.m_sDescription = szBuf;

        hRes = m_psl->GetArguments(szBuf, _countof(szBuf));
        if (SUCCEEDED(hRes))
            m_sli.m_sArguments = szBuf;

        hRes = m_psl->GetHotkey(&m_sli.m_wHotkey);

        hRes = m_psl->GetShowCmd(&m_sli.m_nShowCmd);

        bSuccess = TRUE;
    }

    return bSuccess;
}

BOOL CShellLink::Resolve(HWND hParentWnd, DWORD dwFlags)
{
    if (!Initialise())
        return FALSE;

    BOOL bSuccess = FALSE;

    //Do the actual link resolve
    HRESULT hRes = m_psl->Resolve(hParentWnd, dwFlags);

    if (SUCCEEDED(hRes))
        bSuccess = TRUE;

    return bSuccess;
}

std::wstring CShellLink::GetPath() const
{
    return m_sli.m_sTarget;
}

std::wstring CShellLink::GetArguments() const
{
    return m_sli.m_sArguments;
}

std::wstring CShellLink::GetDescription() const
{
    return m_sli.m_sDescription;
}

WORD CShellLink::GetHotKey() const
{
    return m_sli.m_wHotkey;
}

std::wstring CShellLink::GetIconLocation() const
{
    return m_sli.m_sIconLocation;
}

int CShellLink::GetIconLocationIndex() const
{
    return m_sli.m_nIconIndex;
}

LPITEMIDLIST CShellLink::GetPathIDList() const
{
    return m_sli.m_pidl;
}

int CShellLink::GetShowCommand() const
{
    return m_sli.m_nShowCmd;
}

std::wstring CShellLink::GetWorkingDirectory() const
{
    return m_sli.m_sWorkingDirectory;
}

void CShellLink::SetPath(const std::wstring& sPath)
{
    m_sli.m_sTarget = sPath;
}

void CShellLink::SetArguments(const std::wstring& sArguments)
{
    m_sli.m_sArguments = sArguments;
}

void CShellLink::SetDescription(const std::wstring& sDescription)
{
    m_sli.m_sDescription = sDescription;
}

void CShellLink::SetHotKey(WORD wHotkey)
{
    m_sli.m_wHotkey = wHotkey;
}

void CShellLink::SetIconLocation(const std::wstring& sIconLocation)
{
    m_sli.m_sIconLocation = sIconLocation;
}

void CShellLink::SetIconLocationIndex(int nIconIndex)
{
    m_sli.m_nIconIndex = nIconIndex;
}

void CShellLink::SetPathIDList(LPITEMIDLIST pidl)
{
    m_sli.m_pidl = pidl;
}

void CShellLink::SetShowCommand(int nShowCmd)
{
    m_sli.m_nShowCmd = nShowCmd;
}

void CShellLink::SetWorkingDirectory(const std::wstring& sWorkingDirectory)
{
    m_sli.m_sWorkingDirectory = sWorkingDirectory;
}


CUrlShellLink::CUrlShellLink()
    : m_pURL(NULL)
{
}

BOOL CUrlShellLink::Create(const CShellLinkInfo& sli)
{
    if (!Initialise())
        return FALSE;

    m_sli = sli;

    return TRUE;
}

CUrlShellLink::~CUrlShellLink()
{
    if (m_psl)
    {
        m_psl->Release();
        m_psl = NULL;
    }
    if (m_ppf)
    {
        m_ppf->Release();
        m_ppf = NULL;
    }
    if (m_pURL)
    {
        m_pURL->Release();
        m_pURL = NULL;
    }
}

BOOL CUrlShellLink::Initialise()
{
    BOOL bSuccess = FALSE;
    if (m_bAttemptedInitialise)
        bSuccess = (m_pURL != NULL);
    else
    {
        //Instantiate the COM class
        HRESULT hRes = ::CoCreateInstance(CLSID_InternetShortcut, NULL, CLSCTX_INPROC_SERVER, IID_IUniformResourceLocator, (LPVOID*) &m_pURL);
        if (SUCCEEDED(hRes))
        {
            //Also get a pointer to IPersistFile
            hRes = m_pURL->QueryInterface(IID_IPersistFile, (LPVOID*) &m_ppf);
            if (SUCCEEDED(hRes))
            {
                //Also get a pointer to IShellLink
                hRes = m_pURL->QueryInterface(IID_IShellLink, (LPVOID*) &m_psl);
                if (SUCCEEDED(hRes))
                    bSuccess = TRUE;
            }
        }

        m_bAttemptedInitialise = TRUE;
    }

    return bSuccess;
}

BOOL CUrlShellLink::Save(const std::wstring& sFilename)
{
    if (!Initialise())
        return FALSE;

    BOOL bSuccess = FALSE;

    //Convert the path to a UNICODE string
    WCHAR wszPath[MAX_PATH];

    _tcscpy_s(wszPath, _countof(wszPath), sFilename.c_str());

    //Set the various arguments
    HRESULT hRes = m_pURL->SetURL(m_sli.m_sTarget.c_str(), 0);

    hRes = m_psl->SetIconLocation(m_sli.m_sIconLocation.c_str(), m_sli.m_nIconIndex);

    hRes = m_psl->SetDescription(m_sli.m_sDescription.c_str());

    hRes = m_psl->SetHotkey(m_sli.m_wHotkey);

    //Save the link to file
    hRes = m_ppf->Save(wszPath, TRUE);
    if (SUCCEEDED(hRes))
        bSuccess = TRUE;

    return bSuccess;
}

BOOL CUrlShellLink::Load(const std::wstring& sFilename)
{
    if (!Initialise())
        return FALSE;

    BOOL bSuccess = FALSE;

    //Convert the path to a UNICODE string
    WCHAR wszPath[MAX_PATH];

    _tcscpy_s(wszPath, _countof(wszPath), sFilename.c_str());

    //Load the link from file
    HRESULT hRes = m_ppf->Load(wszPath, STGM_READ);
    if (SUCCEEDED(hRes))
    {
        //Get the various link values
        LPTSTR lpTemp = NULL;
        hRes = m_pURL->GetURL(&lpTemp);
        if (lpTemp == NULL)
            return FALSE;
        if (SUCCEEDED(hRes))
        {
            m_sli.m_sTarget = lpTemp;

            IMalloc* pMalloc;
            hRes = SHGetMalloc(&pMalloc);
            if (SUCCEEDED(hRes))
            {
                pMalloc->Free(lpTemp);
                pMalloc->Release();
            }
        }

        TCHAR szBuf[MAX_PATH];
        hRes = m_psl->GetWorkingDirectory(szBuf, _countof(szBuf));
        if (SUCCEEDED(hRes))
            m_sli.m_sWorkingDirectory = szBuf;

        hRes = m_psl->GetIconLocation(szBuf, _countof(szBuf), &m_sli.m_nIconIndex);
        if (SUCCEEDED(hRes))
            m_sli.m_sIconLocation = szBuf;

        //WINBUG: URL shortcuts always seem to return a description the same as the name of
        //file in which the shortcut is stored
        hRes = m_psl->GetDescription(szBuf, _countof(szBuf));
        if (SUCCEEDED(hRes))
            m_sli.m_sDescription = szBuf;

        hRes = m_psl->GetHotkey(&m_sli.m_wHotkey);

        hRes = m_psl->GetShowCmd(&m_sli.m_nShowCmd);

        bSuccess = TRUE;
    }

    return bSuccess;
}

BOOL CUrlShellLink::Invoke(HWND hParentWnd, DWORD dwFlags, const std::wstring& sVerb)
{
    BOOL bSuccess = FALSE;

    URLINVOKECOMMANDINFO urlicmi;
    urlicmi.dwcbSize = sizeof(URLINVOKECOMMANDINFO);
    urlicmi.dwFlags = dwFlags;
    urlicmi.hwndParent = NULL;
    if (hParentWnd)
        urlicmi.hwndParent = hParentWnd;
    urlicmi.pcszVerb = sVerb.c_str();

    //Invoke the verb on the URL
    HRESULT hRes = m_pURL->InvokeCommand(&urlicmi);
    if (SUCCEEDED(hRes))
        bSuccess = TRUE;

    return bSuccess;
}

void CUrlShellLink::SetArguments(const std::wstring& /*sArguments*/)
{
    //Arguments are not supported for Internet shortcuts
}

std::wstring CUrlShellLink::GetArguments() const
{
    //Arguments are not supported for Internet shortcuts
    return std::wstring();
}

LPITEMIDLIST CUrlShellLink::GetPathIDList() const
{
    //pidls are not supported for Internet shortcuts
    return NULL;
}

void CUrlShellLink::SetPathIDList(LPITEMIDLIST /*pidl*/)
{
    //pidls are not supported for Internet shortcuts
}
