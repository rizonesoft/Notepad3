// sktoolslib - common files for SK tools

// Copyright (C) 2012-2013, 2020-2021 - Stefan Kueng

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
#include "AutoComplete.h"

#include <shlguid.h>
#include <shobjidl.h>

CAutoComplete::CAutoComplete(CSimpleIni* pIni)
    : CRegHistory(pIni)
    , m_pcacs(nullptr)
    , m_pac(nullptr)
    , m_pdrop(nullptr)
{
}

CAutoComplete::~CAutoComplete()
{
    if (m_pac)
        m_pac->Release();
    if (m_pcacs)
        m_pcacs->Release();
    if (m_pdrop)
        m_pdrop->Release();
}

bool CAutoComplete::Init(HWND hEdit)
{
    if (m_pac)
        m_pac->Release();
    if (m_pdrop)
        m_pdrop->Release();
    if (CoCreateInstance(CLSID_AutoComplete,
                         nullptr,
                         CLSCTX_INPROC_SERVER,
                         IID_IAutoComplete2,
                         reinterpret_cast<LPVOID*>(&m_pac)) == S_OK)
    {
        IUnknown* punkSource;

        if (m_pcacs)
            delete m_pcacs;
        m_pcacs = new CAutoCompleteEnum(m_arEntries);

        if (m_pcacs->QueryInterface(IID_IUnknown,
                                    reinterpret_cast<void**>(&punkSource)) == S_OK)
        {
            if (m_pac->Init(hEdit, punkSource, nullptr, nullptr) == S_OK)
            {
                m_pac->Enable(TRUE);
                m_pac->SetOptions(ACO_UPDOWNKEYDROPSLIST | ACO_AUTOSUGGEST);
                if (m_pac->QueryInterface(IID_IAutoCompleteDropDown, reinterpret_cast<void**>(&m_pdrop)) != S_OK)
                {
                    m_pdrop = nullptr;
                }
                return true;
            }
        }
        else
        {
            delete m_pcacs;
            m_pcacs = nullptr;
        }
    }

    return false;
}

bool CAutoComplete::Enable(bool bEnable) const
{
    if (m_pac)
    {
        if (m_pac->Enable(bEnable) == S_OK)
            return true;
    }
    return false;
}

bool CAutoComplete::AddEntry(LPCWSTR szText)
{
    bool bRet = CRegHistory::AddEntry(szText);
    if (m_pcacs)
        m_pcacs->Init(m_arEntries);
    if (m_pdrop)
        m_pdrop->ResetEnumerator();
    return bRet;
}

bool CAutoComplete::RemoveSelected()
{
    if (m_pdrop == nullptr)
        return false;
    if (m_pcacs == nullptr)
        return false;

    DWORD  flags;
    LPWSTR string = nullptr;
    if (m_pdrop->GetDropDownStatus(&flags, &string) == S_OK)
    {
        if (flags & ACDD_VISIBLE)
        {
            RemoveEntry(string);
            // need to save the history now, otherwise adding a new
            // entry with AddEntry() will reload the history from the
            // registry, and our removed item would get restored
            Save();
            if (string)
                CoTaskMemFree(string);
            m_pcacs->Init(m_arEntries);
            m_pdrop->ResetEnumerator();
        }
        else
        {
            if (string)
                CoTaskMemFree(string);
            return false;
        }
    }
    return true;
}

void CAutoComplete::SetOptions(DWORD dwFlags) const
{
    if (m_pac)
    {
        m_pac->SetOptions(dwFlags);
        if (m_pcacs)
            m_pcacs->Init(m_arEntries);
        if (m_pdrop)
            m_pdrop->ResetEnumerator();
    }
}

DWORD CAutoComplete::GetOptions() const
{
    DWORD dwFlags = 0;
    if (m_pac)
    {
        m_pac->GetOptions(&dwFlags);
    }
    return dwFlags;
}

CAutoCompleteEnum::CAutoCompleteEnum(const std::vector<std::wstring>& vec)
    : m_cRefCount(0)
    , m_iCur(0)
{
    Init(vec);
}

CAutoCompleteEnum::CAutoCompleteEnum(const std::vector<std::wstring*>& vec)
    : m_cRefCount(0)
    , m_iCur(0)
{
    Init(vec);
}

void CAutoCompleteEnum::Init(const std::vector<std::wstring>& vec)
{
    m_vecStrings.clear();
    for (size_t i = 0; i < vec.size(); ++i)
        m_vecStrings.push_back(vec[i]);
}
void CAutoCompleteEnum::Init(const std::vector<std::wstring*>& vec)
{
    m_vecStrings.clear();
    for (size_t i = 0; i < vec.size(); ++i)
        m_vecStrings.push_back(*vec[i]);
}

STDMETHODIMP CAutoCompleteEnum::QueryInterface(REFIID refiid, void** ppv)
{
    *ppv = nullptr;
    if (IID_IUnknown == refiid || IID_IEnumString == refiid)
        *ppv = this;

    if (*ppv != nullptr)
    {
        static_cast<LPUNKNOWN>(*ppv)->AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG)
CAutoCompleteEnum::AddRef()
{
    return ++m_cRefCount;
}

STDMETHODIMP_(ULONG)
CAutoCompleteEnum::Release()
{
    --m_cRefCount;
    if (m_cRefCount == 0)
    {
        delete this;
        return 0;
    }
    return m_cRefCount;
}

STDMETHODIMP CAutoCompleteEnum::Next(ULONG celt, LPOLESTR* rgelt, ULONG* pceltFetched)
{
    HRESULT hr = S_FALSE;

    if (!celt)
        celt = 1;

    if (pceltFetched)
        *pceltFetched = 0;

    ULONG i = 0;
    for (; i < celt; i++)
    {
        if (m_iCur == static_cast<ULONG>(m_vecStrings.size()))
            break;

        rgelt[i] = static_cast<LPWSTR>(::CoTaskMemAlloc(static_cast<ULONG>(sizeof(WCHAR)) * (m_vecStrings[m_iCur].size() + 1)));
        wcscpy_s(rgelt[i], m_vecStrings[m_iCur].size() + 1, m_vecStrings[m_iCur].c_str());

        if (pceltFetched)
            (*pceltFetched)++;

        m_iCur++;
    }

    if (i == celt)
        hr = S_OK;

    return hr;
}

STDMETHODIMP CAutoCompleteEnum::Skip(ULONG celt)
{
    if ((m_iCur + static_cast<int>(celt)) >= m_vecStrings.size())
        return S_FALSE;
    m_iCur += celt;
    return S_OK;
}

STDMETHODIMP CAutoCompleteEnum::Reset()
{
    m_iCur = 0;
    return S_OK;
}

STDMETHODIMP CAutoCompleteEnum::Clone(IEnumString** ppenum)
{
    if (ppenum == nullptr)
        return E_POINTER;

    try
    {
        CAutoCompleteEnum* newEnum = new CAutoCompleteEnum(m_vecStrings);

        newEnum->AddRef();
        newEnum->m_iCur = m_iCur;
        *ppenum         = newEnum;
    }
    catch (const std::bad_alloc&)
    {
        return E_OUTOFMEMORY;
    }
    return S_OK;
}
