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

#pragma once

#include <vector>
#include <memory>
#include <ole2.h>
#include <ShellApi.h>
#include <ShlObj.h>
#include "UnicodeUtils.h"
#include "maxpath.h"

// ReSharper disable once CppInconsistentNaming
class CIDropTarget : public IDropTarget
{
    DWORD                     m_cRefCount;
    bool                      m_bAllowDrop;
    struct IDropTargetHelper *m_pDropTargetHelper;
    std::vector<FORMATETC>    m_formatEtc;
    FORMATETC *               m_pSupportedFrmt;

protected:
    HWND m_hTargetWnd;

public:
    CIDropTarget(HWND hTargetWnd);
    virtual ~CIDropTarget();
    void AddSuportedFormat(FORMATETC &ftetc) { m_formatEtc.push_back(ftetc); }

    //return values: true - release the medium. false - don't release the medium
    virtual bool OnDrop(FORMATETC *pFmtEtc, STGMEDIUM &medium, DWORD *pdwEffect) = 0;

    HRESULT STDMETHODCALLTYPE QueryInterface(
        REFIID riid,
        void __RPC_FAR *__RPC_FAR *ppvObject) override;
    ULONG STDMETHODCALLTYPE AddRef() override { return ++m_cRefCount; }
    ULONG STDMETHODCALLTYPE Release() override;

    bool                      QueryDrop(DWORD grfKeyState, LPDWORD pdwEffect) const;
    HRESULT STDMETHODCALLTYPE DragEnter(
        IDataObject __RPC_FAR *pDataObj,
        DWORD                  grfKeyState,
        POINTL                 pt,
        DWORD __RPC_FAR *pdwEffect) override;
    HRESULT STDMETHODCALLTYPE DragOver(
        DWORD  grfKeyState,
        POINTL pt,
        DWORD __RPC_FAR *pdwEffect) override;
    HRESULT STDMETHODCALLTYPE DragLeave() override;
    HRESULT STDMETHODCALLTYPE Drop(
        IDataObject __RPC_FAR *pDataObj,
        DWORD                  grfKeyState,
        POINTL                 pt,
        DWORD __RPC_FAR *pdwEffect) override;
};

class CFileDropTarget : public CIDropTarget
{
public:
    CFileDropTarget(HWND hTargetWnd)
        : CIDropTarget(hTargetWnd)
        , m_hParent(nullptr)
        , m_concat(0)
    {
    }
    CFileDropTarget(HWND hTargetWnd, HWND hParent)
        : CIDropTarget(hTargetWnd)
        , m_hParent(hParent)
        , m_concat(0)
    {
        RegisterDragDrop(hTargetWnd, this);
        // create the supported format:
        FORMATETC ftEtc = {0};
        ftEtc.cfFormat  = CF_HDROP;
        ftEtc.dwAspect  = DVASPECT_CONTENT;
        ftEtc.lindex    = -1;
        ftEtc.tymed     = TYMED_HGLOBAL;
        AddSuportedFormat(ftEtc);
    }
    void SetMultipathConcatenate(wchar_t ch) { m_concat = ch; }

    bool OnDrop(FORMATETC *pFmtEtc, STGMEDIUM &medium, DWORD * /*pdwEffect*/) override
    {
        if (m_hParent && (pFmtEtc->cfFormat == CF_HDROP) && (medium.tymed == TYMED_HGLOBAL))
        {
            HDROP hDrop = static_cast<HDROP>(GlobalLock(medium.hGlobal));
            if (hDrop != nullptr)
            {
                SendMessage(m_hParent, WM_DROPFILES, reinterpret_cast<WPARAM>(hDrop), 0);
            }
            GlobalUnlock(medium.hGlobal);
            return true; //let base free the medium
        }
        if (pFmtEtc->cfFormat == CF_TEXT && medium.tymed == TYMED_ISTREAM)
        {
            if (medium.pstm != nullptr)
            {
                const int BUF_SIZE = 10000;
                auto      buff     = std::make_unique<char[]>(BUF_SIZE + 1);
                ULONG     cbRead   = 0;
                HRESULT   hr       = medium.pstm->Read(buff.get(), BUF_SIZE, &cbRead);
                if (SUCCEEDED(hr) && cbRead > 0 && cbRead < BUF_SIZE)
                {
                    buff[cbRead] = 0;
                    LRESULT nLen = ::SendMessage(m_hTargetWnd, WM_GETTEXTLENGTH, 0, 0);
                    ::SendMessage(m_hTargetWnd, EM_SETSEL, nLen, -1);
                    std::wstring str = CUnicodeUtils::StdGetUnicode(std::string(buff.get()));
                    ::SendMessage(m_hTargetWnd, EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(str.c_str()));
                }
                else
                    for (; (hr == S_OK && cbRead > 0) && SUCCEEDED(hr);)
                    {
                        buff[cbRead] = 0;
                        LRESULT nLen = ::SendMessage(m_hTargetWnd, WM_GETTEXTLENGTH, 0, 0);
                        ::SendMessage(m_hTargetWnd, EM_SETSEL, nLen, -1);
                        std::wstring str = CUnicodeUtils::StdGetUnicode(std::string(buff.get()));
                        ::SendMessage(m_hTargetWnd, EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(str.c_str()));
                        cbRead = 0;
                        hr     = medium.pstm->Read(buff.get(), BUF_SIZE, &cbRead);
                    }
            }
        }
        if (pFmtEtc->cfFormat == CF_UNICODETEXT && medium.tymed == TYMED_ISTREAM)
        {
            if (medium.pstm != nullptr)
            {
                const int BUF_SIZE = 10000;
                auto      buff     = std::make_unique<char[]>(BUF_SIZE + 1);
                ULONG     cbRead   = 0;
                HRESULT   hr       = medium.pstm->Read(buff.get(), BUF_SIZE, &cbRead);
                if (SUCCEEDED(hr) && cbRead > 0 && cbRead < BUF_SIZE)
                {
                    buff[cbRead] = 0;
                    LRESULT nLen = ::SendMessage(m_hTargetWnd, WM_GETTEXTLENGTH, 0, 0);
                    ::SendMessage(m_hTargetWnd, EM_SETSEL, nLen, -1);
                    ::SendMessage(m_hTargetWnd, EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(buff.get()));
                }
                else
                    for (; (hr == S_OK && cbRead > 0) && SUCCEEDED(hr);)
                    {
                        buff[cbRead] = 0;
                        LRESULT nLen = ::SendMessage(m_hTargetWnd, WM_GETTEXTLENGTH, 0, 0);
                        ::SendMessage(m_hTargetWnd, EM_SETSEL, nLen, -1);
                        ::SendMessage(m_hTargetWnd, EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(buff.get()));
                        cbRead = 0;
                        hr     = medium.pstm->Read(buff.get(), BUF_SIZE, &cbRead);
                    }
            }
        }
        if (pFmtEtc->cfFormat == CF_TEXT && medium.tymed == TYMED_HGLOBAL)
        {
            char *pStr = static_cast<char *>(GlobalLock(medium.hGlobal));
            if (pStr != nullptr)
            {
                LRESULT nLen = ::SendMessage(m_hTargetWnd, WM_GETTEXTLENGTH, 0, 0);
                ::SendMessage(m_hTargetWnd, EM_SETSEL, nLen, -1);
                std::wstring str = CUnicodeUtils::StdGetUnicode(std::string(pStr));
                ::SendMessage(m_hTargetWnd, EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(str.c_str()));
            }
            GlobalUnlock(medium.hGlobal);
        }
        if (pFmtEtc->cfFormat == CF_UNICODETEXT && medium.tymed == TYMED_HGLOBAL)
        {
            WCHAR *pStr = static_cast<WCHAR *>(GlobalLock(medium.hGlobal));
            if (pStr != nullptr)
            {
                LRESULT nLen = ::SendMessage(m_hTargetWnd, WM_GETTEXTLENGTH, 0, 0);
                ::SendMessage(m_hTargetWnd, EM_SETSEL, nLen, -1);
                ::SendMessage(m_hTargetWnd, EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(pStr));
            }
            GlobalUnlock(medium.hGlobal);
        }
        if (pFmtEtc->cfFormat == CF_HDROP && medium.tymed == TYMED_HGLOBAL)
        {
            HDROP hDrop = static_cast<HDROP>(GlobalLock(medium.hGlobal));
            if (hDrop != nullptr)
            {
                auto szFileName = std::make_unique<wchar_t[]>(MAX_PATH_NEW);

                UINT         cFiles = DragQueryFile(hDrop, 0xFFFFFFFF, nullptr, 0);
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
                        ::SendMessage(m_hTargetWnd, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(szFileName.get()));
                }
                if (!concatPaths.empty())
                    ::SendMessage(m_hTargetWnd, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(concatPaths.c_str()));

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
