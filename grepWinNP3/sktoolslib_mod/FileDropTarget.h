// sktoolslib - common files for SK tools

// Copyright (C) 2012-2013, 2020 - Stefan Kueng

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
#include <memory>
#include <ole2.h>
#include <ShellApi.h>
#include <ShlObj.h>
#include "UnicodeUtils.h"
#include "maxpath.h"

class CIDropTarget : public IDropTarget
{
    DWORD m_cRefCount;
    bool m_bAllowDrop;
    struct IDropTargetHelper *m_pDropTargetHelper;
    std::vector<FORMATETC> m_formatetc;
    FORMATETC* m_pSupportedFrmt;
protected:
    HWND m_hTargetWnd;
public:

    CIDropTarget(HWND m_hTargetWnd);
    virtual ~CIDropTarget();
    void AddSuportedFormat(FORMATETC& ftetc) { m_formatetc.push_back(ftetc); }

    //return values: true - release the medium. false - don't release the medium
    virtual bool OnDrop(FORMATETC* pFmtEtc, STGMEDIUM& medium,DWORD *pdwEffect) = 0;

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef( void) { return ++m_cRefCount; }
    virtual ULONG STDMETHODCALLTYPE Release( void);

    bool QueryDrop(DWORD grfKeyState, LPDWORD pdwEffect);
    virtual HRESULT STDMETHODCALLTYPE DragEnter(
        /* [unique][in] */ IDataObject __RPC_FAR *pDataObj,
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ DWORD __RPC_FAR *pdwEffect);
    virtual HRESULT STDMETHODCALLTYPE DragOver(
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ DWORD __RPC_FAR *pdwEffect);
    virtual HRESULT STDMETHODCALLTYPE DragLeave( void);
    virtual HRESULT STDMETHODCALLTYPE Drop(
        /* [unique][in] */ IDataObject __RPC_FAR *pDataObj,
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ DWORD __RPC_FAR *pdwEffect);
};


class CFileDropTarget : public CIDropTarget
{
public:
    CFileDropTarget(HWND hTargetWnd)
        : CIDropTarget(hTargetWnd)
        , m_hParent(NULL)
        , m_concat(0)
    {}
    CFileDropTarget(HWND hTargetWnd, HWND hParent)
        : CIDropTarget(hTargetWnd)
        , m_hParent(hParent)
        , m_concat(0)
    {
        RegisterDragDrop(hTargetWnd, this);
        // create the supported format:
        FORMATETC ftetc={0};
        ftetc.cfFormat = CF_HDROP;
        ftetc.dwAspect = DVASPECT_CONTENT;
        ftetc.lindex   = -1;
        ftetc.tymed    = TYMED_HGLOBAL;
        AddSuportedFormat(ftetc);
    }
    void SetMultipathConcatenate(wchar_t ch) { m_concat = ch; }
    virtual bool OnDrop(FORMATETC* pFmtEtc, STGMEDIUM& medium, DWORD * /*pdwEffect*/)
    {
        if (m_hParent && (pFmtEtc->cfFormat == CF_HDROP) && (medium.tymed == TYMED_HGLOBAL))
        {
            HDROP hDrop = (HDROP)GlobalLock(medium.hGlobal);
            if (hDrop != NULL)
            {
                SendMessage(m_hParent, WM_DROPFILES, (WPARAM)hDrop, 0);
            }
            GlobalUnlock(medium.hGlobal);
            return true; //let base free the medium
        }
        if (pFmtEtc->cfFormat == CF_TEXT && medium.tymed == TYMED_ISTREAM)
        {
            if (medium.pstm != NULL)
            {
                const int BUF_SIZE = 10000;
                std::unique_ptr<char[]> buff(new char[BUF_SIZE+1]);
                ULONG cbRead=0;
                HRESULT hr = medium.pstm->Read(buff.get(), BUF_SIZE, &cbRead);
                if (SUCCEEDED(hr) && cbRead > 0 && cbRead < BUF_SIZE)
                {
                    buff[cbRead]=0;
                    LRESULT nLen = ::SendMessage(m_hTargetWnd, WM_GETTEXTLENGTH, 0, 0);
                    ::SendMessage(m_hTargetWnd, EM_SETSEL, nLen, -1);
                    std::wstring str = CUnicodeUtils::StdGetUnicode(std::string(buff.get()));
                    ::SendMessage(m_hTargetWnd, EM_REPLACESEL, TRUE, (LPARAM)str.c_str());
                }
                else
                    for(;(hr == S_OK && cbRead >0) && SUCCEEDED(hr) ;)
                    {
                        buff[cbRead]=0;
                        LRESULT nLen = ::SendMessage(m_hTargetWnd, WM_GETTEXTLENGTH, 0, 0);
                        ::SendMessage(m_hTargetWnd, EM_SETSEL, nLen, -1);
                        std::wstring str = CUnicodeUtils::StdGetUnicode(std::string(buff.get()));
                        ::SendMessage(m_hTargetWnd, EM_REPLACESEL, TRUE, (LPARAM)str.c_str());
                        cbRead=0;
                        hr = medium.pstm->Read(buff.get(), BUF_SIZE, &cbRead);
                    }
            }
        }
        if (pFmtEtc->cfFormat == CF_UNICODETEXT && medium.tymed == TYMED_ISTREAM)
        {
            if (medium.pstm != NULL)
            {
                const int BUF_SIZE = 10000;
                std::unique_ptr<char[]> buff(new char[BUF_SIZE+1]);
                ULONG cbRead=0;
                HRESULT hr = medium.pstm->Read(buff.get(), BUF_SIZE, &cbRead);
                if (SUCCEEDED(hr) && cbRead > 0 && cbRead < BUF_SIZE)
                {
                    buff[cbRead]=0;
                    LRESULT nLen = ::SendMessage(m_hTargetWnd, WM_GETTEXTLENGTH, 0, 0);
                    ::SendMessage(m_hTargetWnd, EM_SETSEL, nLen, -1);
                    ::SendMessage(m_hTargetWnd, EM_REPLACESEL, TRUE, (LPARAM)buff.get());
                }
                else
                    for(;(hr == S_OK && cbRead >0) && SUCCEEDED(hr) ;)
                    {
                        buff[cbRead]=0;
                        LRESULT nLen = ::SendMessage(m_hTargetWnd, WM_GETTEXTLENGTH, 0, 0);
                        ::SendMessage(m_hTargetWnd, EM_SETSEL, nLen, -1);
                        ::SendMessage(m_hTargetWnd, EM_REPLACESEL, TRUE, (LPARAM)buff.get());
                        cbRead=0;
                        hr = medium.pstm->Read(buff.get(), BUF_SIZE, &cbRead);
                    }
            }
        }
        if (pFmtEtc->cfFormat == CF_TEXT && medium.tymed == TYMED_HGLOBAL)
        {
            char* pStr = (char*)GlobalLock(medium.hGlobal);
            if (pStr != NULL)
            {
                LRESULT nLen = ::SendMessage(m_hTargetWnd, WM_GETTEXTLENGTH, 0, 0);
                ::SendMessage(m_hTargetWnd, EM_SETSEL, nLen, -1);
                std::wstring str = CUnicodeUtils::StdGetUnicode(std::string(pStr));
                ::SendMessage(m_hTargetWnd, EM_REPLACESEL, TRUE, (LPARAM)str.c_str());
            }
            GlobalUnlock(medium.hGlobal);
        }
        if (pFmtEtc->cfFormat == CF_UNICODETEXT && medium.tymed == TYMED_HGLOBAL)
        {
            WCHAR* pStr = (WCHAR*)GlobalLock(medium.hGlobal);
            if (pStr != NULL)
            {
                LRESULT nLen = ::SendMessage(m_hTargetWnd, WM_GETTEXTLENGTH, 0, 0);
                ::SendMessage(m_hTargetWnd, EM_SETSEL, nLen, -1);
                ::SendMessage(m_hTargetWnd, EM_REPLACESEL, TRUE, (LPARAM)pStr);
            }
            GlobalUnlock(medium.hGlobal);
        }
        if (pFmtEtc->cfFormat == CF_HDROP && medium.tymed == TYMED_HGLOBAL)
        {
            HDROP hDrop = (HDROP)GlobalLock(medium.hGlobal);
            if (hDrop != NULL)
            {
                std::unique_ptr<TCHAR[]> szFileName(new TCHAR[MAX_PATH_NEW]);

                UINT cFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
                std::wstring concatPaths;
                for (UINT i = 0; i < cFiles; ++i)
                {
                    DragQueryFile(hDrop, i, szFileName.get(), MAX_PATH_NEW);
                    if (m_concat)
                    {
                        if (!concatPaths.empty())
                            concatPaths += m_concat;
                        concatPaths += szFileName.get();
                    }
                    else
                        ::SendMessage(m_hTargetWnd, WM_SETTEXT, 0, (LPARAM)szFileName.get());
                }
                if (!concatPaths.empty())
                    ::SendMessage(m_hTargetWnd, WM_SETTEXT, 0, (LPARAM)concatPaths.c_str());

                //DragFinish(hDrop); // base class calls ReleaseStgMedium
            }
            GlobalUnlock(medium.hGlobal);
        }
        return true; //let base free the medium
    }
private:
    HWND    m_hParent;
    wchar_t m_concat;
};
