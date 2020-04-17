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

#pragma once

#include <string>


class CCallback : public IBindStatusCallback, public IAuthenticate, public IHttpSecurity
{
public:
    CCallback();
    ~CCallback();

    void SetAuthData(const std::wstring& username, const std::wstring& password) {m_username = username; m_password = password;}

    // IHttpSecurity method
    STDMETHOD(OnSecurityProblem)(DWORD /*dwProblem*/)
    { return S_OK; }

    STDMETHOD(GetWindow)(
        /* [in] */ REFGUID /*rguidReason*/,
        /* [out] */ HWND * /*phwnd*/)
    { return E_NOTIMPL; }

    // IAuthenticate method
    STDMETHOD(Authenticate)(
        /* [out] */ HWND * phwnd,
        /* [out] */ LPWSTR * pszUsername,
        /* [out] */ LPWSTR * pszPassword);

    // IBindStatusCallback methods.  Note that the only method called by IE
    // is OnProgress(), so the others just return E_NOTIMPL.

    STDMETHOD(OnStartBinding)(
        /* [in] */ DWORD /*dwReserved*/,
        /* [in] */ IBinding __RPC_FAR * /*pib*/)
    { return E_NOTIMPL; }

    STDMETHOD(GetPriority)(
        /* [out] */ LONG __RPC_FAR * /*pnPriority*/)
    { return E_NOTIMPL; }

    STDMETHOD(OnLowResource)(
        /* [in] */ DWORD /*reserved*/)
    { return E_NOTIMPL; }

    STDMETHOD(OnProgress)(
        /* [in] */ ULONG /*ulProgress*/,
        /* [in] */ ULONG /*ulProgressMax*/,
        /* [in] */ ULONG /*ulStatusCode*/,
        /* [in] */ LPCWSTR /*wszStatusText*/)
    { return S_OK; }

    STDMETHOD(OnStopBinding)(
        /* [in] */ HRESULT /*hresult*/,
        /* [unique][in] */ LPCWSTR /*szError*/)
    { return E_NOTIMPL; }

    STDMETHOD(GetBindInfo)(
        /* [out] */ DWORD __RPC_FAR * /*grfBINDF*/,
        /* [unique][out][in] */ BINDINFO __RPC_FAR * /*pbindinfo*/)
    { return E_NOTIMPL; }

    STDMETHOD(OnDataAvailable)(
        /* [in] */ DWORD /*grfBSCF*/,
        /* [in] */ DWORD /*dwSize*/,
        /* [in] */ FORMATETC __RPC_FAR * /*pformatetc*/,
        /* [in] */ STGMEDIUM __RPC_FAR * /*pstgmed*/)
    { return E_NOTIMPL; }

    STDMETHOD(OnObjectAvailable)(
        /* [in] */ REFIID /*riid*/,
        /* [iid_is][in] */ IUnknown __RPC_FAR * /*punk*/)
    { return E_NOTIMPL; }

    // IUnknown methods.  Note that IE never calls any of these methods, since
    // the caller owns the IBindStatusCallback interface, so the methods all
    // return zero/E_NOTIMPL.

    STDMETHOD_(ULONG,AddRef)()
    { return 0; }

    STDMETHOD_(ULONG,Release)()
    { return 0; }

    STDMETHOD(QueryInterface)(
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR * ppvObject)
    {
        if (riid == IID_IAuthenticate)
        {
            *ppvObject = (void*) (IAuthenticate*)this;
            return S_OK;
        }
        else if (riid == IID_IUnknown)
        {
            *ppvObject = (void*) (IUnknown*)(IBindStatusCallback*)this;
            return S_OK;
        }
        else if (riid == IID_IBindStatusCallback)
        {
            *ppvObject = (void*) (IBindStatusCallback*)this;
            return S_OK;
        }
        else if (riid == IID_IHttpSecurity)
        {
            *ppvObject = (void*) (IHttpSecurity*)this;
            return S_OK;
        }
        else if (riid == IID_IWindowForBindingUI)
        {
            *ppvObject = (void*) (IWindowForBindingUI*)this;
            return S_OK;
        }
        return E_NOINTERFACE;
    }

private:
    ULONG           m_cRef;
    std::wstring    m_username;
    std::wstring    m_password;
};
