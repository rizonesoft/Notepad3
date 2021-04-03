// grepWin - regex search and replace for Windows

// Copyright (C) 2007-2008, 2011-2015, 2021 - Stefan Kueng

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
#include <vector>
#include <shobjidl.h>
#include <shlobj.h>

// ReSharper disable once CppInconsistentNaming
class CIShellFolderHook;
class CSearchInfo;
struct LineData;

class CShellContextMenu
{
public:
    void SetObjects(const std::vector<CSearchInfo> &strVector, const std::vector<LineData> &lineVector);
    UINT ShowContextMenu(HWND hWnd, POINT pt);
    CShellContextMenu();
    virtual ~CShellContextMenu();

private:
    size_t                   m_nItems;
    BOOL                     bDelete;
    HMENU                    m_menu;
    IShellFolder *           m_psfFolder;
    LPITEMIDLIST *           m_pidlArray;
    int                      m_pidlArrayItems;
    std::vector<CSearchInfo> m_strVector;
    std::vector<LineData>    m_lineVector;

    static void             InvokeCommand(LPCONTEXTMENU pContextMenu, UINT idCommand);
    BOOL                    GetContextMenu(HWND hWnd, void **ppContextMenu, int &iMenuType);
    static LRESULT CALLBACK HookWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static void             FreePIDLArray(LPITEMIDLIST *pidlArray, int nItems);

    static HRESULT CALLBACK dfmCallback(IShellFolder *psf, HWND hwnd, IDataObject *pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam);

    CIShellFolderHook *m_pFolderHook;

    friend class CIShellFolderHook;
};

// ReSharper disable once CppInconsistentNaming
class CIShellFolderHook : public IShellFolder
{
public:
    CIShellFolderHook(LPSHELLFOLDER sf, CShellContextMenu *pShellContextMenu)
    {
        sf->AddRef();
        m_iSf               = sf;
        m_pShellContextMenu = pShellContextMenu;
    }

    virtual ~CIShellFolderHook() { m_iSf->Release(); }

    // IUnknown methods --------
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, __RPC__deref_out void **ppvObject) override { return m_iSf->QueryInterface(riid, ppvObject); }
    ULONG STDMETHODCALLTYPE   AddRef() override { return m_iSf->AddRef(); }
    ULONG STDMETHODCALLTYPE   Release() override { return m_iSf->Release(); }

    // IShellFolder methods ----
    HRESULT STDMETHODCALLTYPE GetUIObjectOf(HWND hwndOwner, UINT cidl, LPCITEMIDLIST *apidl, REFIID riid, UINT *rgfReserved, void **ppv) override;

    HRESULT STDMETHODCALLTYPE CompareIDs(LPARAM lParam, __RPC__in PCUIDLIST_RELATIVE pidl1, __RPC__in PCUIDLIST_RELATIVE pidl2) override { return m_iSf->CompareIDs(lParam, pidl1, pidl2); }
    HRESULT STDMETHODCALLTYPE GetDisplayNameOf(__RPC__in_opt PCUITEMID_CHILD pidl, SHGDNF uFlags, __RPC__out STRRET *pName) override { return m_iSf->GetDisplayNameOf(pidl, uFlags, pName); }
    HRESULT STDMETHODCALLTYPE CreateViewObject(__RPC__in_opt HWND hwndOwner, __RPC__in REFIID riid, __RPC__deref_out_opt void **ppv) override { return m_iSf->CreateViewObject(hwndOwner, riid, ppv); }
    HRESULT STDMETHODCALLTYPE EnumObjects(__RPC__in_opt HWND hwndOwner, SHCONTF grfFlags, __RPC__deref_out_opt IEnumIDList **ppenumIDList) override { return m_iSf->EnumObjects(hwndOwner, grfFlags, ppenumIDList); }
    HRESULT STDMETHODCALLTYPE BindToObject(__RPC__in PCUIDLIST_RELATIVE pidl, __RPC__in_opt IBindCtx *pbc, __RPC__in REFIID riid, __RPC__deref_out_opt void **ppv) override { return m_iSf->BindToObject(pidl, pbc, riid, ppv); }
    HRESULT STDMETHODCALLTYPE ParseDisplayName(__RPC__in_opt HWND hwnd, __RPC__in_opt IBindCtx *pbc, __RPC__in_string LPWSTR pszDisplayName, __reserved ULONG *pchEaten, __RPC__deref_out_opt PIDLIST_RELATIVE *ppidl, __RPC__inout_opt ULONG *pdwAttributes) override { return m_iSf->ParseDisplayName(hwnd, pbc, pszDisplayName, pchEaten, ppidl, pdwAttributes); }
    HRESULT STDMETHODCALLTYPE GetAttributesOf(UINT cidl, __RPC__in_ecount_full_opt(cidl) PCUITEMID_CHILD_ARRAY apidl, __RPC__inout SFGAOF *rgfInOut) override { return m_iSf->GetAttributesOf(cidl, apidl, rgfInOut); }
    HRESULT STDMETHODCALLTYPE BindToStorage(__RPC__in PCUIDLIST_RELATIVE pidl, __RPC__in_opt IBindCtx *pbc, __RPC__in REFIID riid, __RPC__deref_out_opt void **ppv) override { return m_iSf->BindToStorage(pidl, pbc, riid, ppv); }
    HRESULT STDMETHODCALLTYPE SetNameOf(__in_opt HWND hwnd, __in PCUITEMID_CHILD pidl, __in LPCWSTR pszName, __in SHGDNF uFlags, __deref_opt_out PITEMID_CHILD *ppidlOut) override { return m_iSf->SetNameOf(hwnd, pidl, pszName, uFlags, ppidlOut); }

protected:
    LPSHELLFOLDER      m_iSf;
    CShellContextMenu *m_pShellContextMenu;
};
