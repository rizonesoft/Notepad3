// sktoolslib - common files for SK tools

// Copyright (C) 2012-2013 - Stefan Kueng

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

#pragma once

#include <string>
#include <vector>
#include <ShlDisp.h>
#include <ShlGuid.h>
#include <ShObjIdl.h >

#include "RegHistory.h"


/**
* Helper class for the CAutoComplete class: implements the string enumerator.
*/
class CAutoCompleteEnum : public IEnumString
{
public:
    CAutoCompleteEnum(const std::vector<std::wstring*>& vec);
    CAutoCompleteEnum(const std::vector<std::wstring>& vec);
    ~CAutoCompleteEnum() {}
    //IUnknown members
    STDMETHOD(QueryInterface)(REFIID, void**);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

    //IEnumString members
    STDMETHOD(Next)(ULONG, LPOLESTR*, ULONG*);
    STDMETHOD(Skip)(ULONG);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(IEnumString**);

    void Init(const std::vector<std::wstring*>& vec);
    void Init(const std::vector<std::wstring>& vec);
private:
    std::vector<std::wstring>   m_vecStrings;
    ULONG                       m_cRefCount;
    size_t                      m_iCur;
};



class CAutoComplete : public CRegHistory
{
public:
    CAutoComplete(CSimpleIni * pIni = NULL);
    ~CAutoComplete(void);

    bool        Init(HWND hEdit);
    bool        Enable(bool bEnable);
    bool        AddEntry(LPCTSTR szText);

    bool        RemoveSelected();
    void        SetOptions(DWORD dwFlags);
    DWORD       GetOptions() const;
private:
    CAutoCompleteEnum *         m_pcacs;
    IAutoComplete2 *            m_pac;
    IAutoCompleteDropDown *     m_pdrop;

};
