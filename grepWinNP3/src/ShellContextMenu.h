// grepWin - regex search and replace for Windows

// Copyright (C) 2007-2008, 2011-2015 - Stefan Kueng

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
#include <shobjidl.h>
#include <shlobj.h>


class CIShellFolderHook;
class CSearchInfo;
struct LineData;

class CShellContextMenu
{
public:
    void SetObjects(const std::vector<CSearchInfo>& strVector, const std::vector<LineData>& lineVector);
    UINT ShowContextMenu(HWND hWnd, POINT pt);
    CShellContextMenu();
    virtual ~CShellContextMenu();

private:
    size_t m_nItems;
    BOOL bDelete;
    HMENU m_Menu;
    IShellFolder * m_psfFolder;
    LPITEMIDLIST * m_pidlArray;
    int            m_pidlArrayItems;
    std::vector<CSearchInfo> m_strVector;
    std::vector<LineData> m_lineVector;

    void InvokeCommand(LPCONTEXTMENU pContextMenu, UINT idCommand);
    BOOL GetContextMenu(HWND hWnd, void ** ppContextMenu, int & iMenuType);
    static LRESULT CALLBACK HookWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    void FreePIDLArray(LPITEMIDLIST * pidlArray, int nItems);

    static HRESULT CALLBACK dfmCallback(IShellFolder *psf, HWND hwnd, IDataObject *pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam);

    CIShellFolderHook * m_pFolderhook;

    friend class CIShellFolderHook;
};

class CIShellFolderHook : public IShellFolder
{
public:
    CIShellFolderHook(LPSHELLFOLDER sf, CShellContextMenu * pShellContextMenu)
    {
        sf->AddRef();
        m_iSF = sf;
        m_pShellContextMenu = pShellContextMenu;
    }

    ~CIShellFolderHook() { m_iSF->Release(); }

    // IUnknown methods --------
    virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, __RPC__deref_out void **ppvObject ) {return m_iSF->QueryInterface(riid, ppvObject);}
    virtual ULONG STDMETHODCALLTYPE AddRef( void ) {return m_iSF->AddRef();}
    virtual ULONG STDMETHODCALLTYPE Release( void ) {return m_iSF->Release();}


    // IShellFolder methods ----
    virtual HRESULT STDMETHODCALLTYPE GetUIObjectOf(HWND hwndOwner, UINT cidl, LPCITEMIDLIST *apidl, REFIID riid, UINT *rgfReserved, void **ppv);

    virtual HRESULT STDMETHODCALLTYPE CompareIDs( LPARAM lParam, __RPC__in PCUIDLIST_RELATIVE pidl1, __RPC__in PCUIDLIST_RELATIVE pidl2 ) {return m_iSF->CompareIDs(lParam, pidl1, pidl2);}
    virtual HRESULT STDMETHODCALLTYPE GetDisplayNameOf( __RPC__in_opt PCUITEMID_CHILD pidl, SHGDNF uFlags, __RPC__out STRRET *pName ) {return m_iSF->GetDisplayNameOf(pidl, uFlags, pName);}
    virtual HRESULT STDMETHODCALLTYPE CreateViewObject( __RPC__in_opt HWND hwndOwner, __RPC__in REFIID riid, __RPC__deref_out_opt void **ppv ) {return m_iSF->CreateViewObject(hwndOwner, riid, ppv);}
    virtual HRESULT STDMETHODCALLTYPE EnumObjects( __RPC__in_opt HWND hwndOwner, SHCONTF grfFlags, __RPC__deref_out_opt IEnumIDList **ppenumIDList ) {return m_iSF->EnumObjects(hwndOwner, grfFlags, ppenumIDList);}
    virtual HRESULT STDMETHODCALLTYPE BindToObject( __RPC__in PCUIDLIST_RELATIVE pidl, __RPC__in_opt IBindCtx *pbc, __RPC__in REFIID riid, __RPC__deref_out_opt void **ppv ) {return m_iSF->BindToObject(pidl, pbc, riid, ppv);}
    virtual HRESULT STDMETHODCALLTYPE ParseDisplayName( __RPC__in_opt HWND hwnd, __RPC__in_opt IBindCtx *pbc, __RPC__in_string LPWSTR pszDisplayName, __reserved ULONG *pchEaten, __RPC__deref_out_opt PIDLIST_RELATIVE *ppidl, __RPC__inout_opt ULONG *pdwAttributes ) {return m_iSF->ParseDisplayName(hwnd, pbc, pszDisplayName, pchEaten, ppidl, pdwAttributes);}
    virtual HRESULT STDMETHODCALLTYPE GetAttributesOf( UINT cidl, __RPC__in_ecount_full_opt(cidl ) PCUITEMID_CHILD_ARRAY apidl, __RPC__inout SFGAOF *rgfInOut) {return m_iSF->GetAttributesOf(cidl, apidl, rgfInOut);}
    virtual HRESULT STDMETHODCALLTYPE BindToStorage( __RPC__in PCUIDLIST_RELATIVE pidl, __RPC__in_opt IBindCtx *pbc, __RPC__in REFIID riid, __RPC__deref_out_opt void **ppv ) {return m_iSF->BindToStorage(pidl, pbc, riid, ppv);}
    virtual HRESULT STDMETHODCALLTYPE SetNameOf( __in_opt HWND hwnd, __in PCUITEMID_CHILD pidl, __in LPCWSTR pszName, __in SHGDNF uFlags, __deref_opt_out PITEMID_CHILD *ppidlOut ) {return m_iSF->SetNameOf(hwnd, pidl, pszName, uFlags, ppidlOut);}



protected:
    LPSHELLFOLDER m_iSF;
    CShellContextMenu * m_pShellContextMenu;
};
