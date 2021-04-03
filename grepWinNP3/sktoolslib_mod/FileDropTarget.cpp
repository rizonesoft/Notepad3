// sktoolslib - common files for SK tools

// Copyright (C) 2012, 2020-2021 - Stefan Kueng

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
#include "FileDropTarget.h"
#include <ShlGuid.h>

CIDropTarget::CIDropTarget(HWND hTargetWnd)
    : m_cRefCount(0)
    , m_bAllowDrop(false)
    , m_pDropTargetHelper(nullptr)
    , m_pSupportedFrmt(nullptr)
    , m_hTargetWnd(hTargetWnd)
{
    if (FAILED(CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER,
                                IID_IDropTargetHelper, reinterpret_cast<LPVOID *>(&m_pDropTargetHelper))))
        m_pDropTargetHelper = nullptr;
}

CIDropTarget::~CIDropTarget()
{
    if (m_pDropTargetHelper != nullptr)
    {
        m_pDropTargetHelper->Release();
        m_pDropTargetHelper = nullptr;
    }
}

HRESULT STDMETHODCALLTYPE CIDropTarget::QueryInterface(REFIID riid,
                                                       void __RPC_FAR *__RPC_FAR *ppvObject)
{
    *ppvObject = nullptr;
    if (IID_IUnknown == riid || IID_IDropTarget == riid)
        *ppvObject = this;

    if (*ppvObject != nullptr)
    {
        static_cast<LPUNKNOWN>(*ppvObject)->AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CIDropTarget::Release()
{
    long nTemp = --m_cRefCount;
    if (nTemp == 0)
        delete this;
    return nTemp;
}

bool CIDropTarget::QueryDrop(DWORD grfKeyState, LPDWORD pdwEffect) const
{
    DWORD dwOkEffects = *pdwEffect;

    if (!m_bAllowDrop)
    {
        *pdwEffect = DROPEFFECT_NONE;
        return false;
    }
    //CTRL+SHIFT  -- DROPEFFECT_LINK
    //CTRL        -- DROPEFFECT_COPY
    //SHIFT       -- DROPEFFECT_MOVE
    //no modifier -- DROPEFFECT_MOVE or whatever is allowed by src
    *pdwEffect = (grfKeyState & MK_CONTROL) ? ((grfKeyState & MK_SHIFT) ? DROPEFFECT_LINK : DROPEFFECT_COPY) : ((grfKeyState & MK_SHIFT) ? DROPEFFECT_MOVE : 0);
    if (*pdwEffect == 0)
    {
        // No modifier keys used by user while dragging.
        if (DROPEFFECT_COPY & dwOkEffects)
            *pdwEffect = DROPEFFECT_COPY;
        else if (DROPEFFECT_MOVE & dwOkEffects)
            *pdwEffect = DROPEFFECT_MOVE;
        else if (DROPEFFECT_LINK & dwOkEffects)
            *pdwEffect = DROPEFFECT_LINK;
        else
        {
            *pdwEffect = DROPEFFECT_NONE;
        }
    }
    else
    {
        // Check if the drag source application allows the drop effect desired by user.
        // The drag source specifies this in DoDragDrop
        if (!(*pdwEffect & dwOkEffects))
            *pdwEffect = DROPEFFECT_NONE;
    }

    return (DROPEFFECT_NONE == *pdwEffect) ? false : true;
}

HRESULT STDMETHODCALLTYPE CIDropTarget::DragEnter(
    IDataObject __RPC_FAR *pDataObj,
    DWORD                  grfKeyState,
    POINTL                 pt,
    DWORD __RPC_FAR *pdwEffect)
{
    if (pDataObj == nullptr)
        return E_INVALIDARG;

    if (m_pDropTargetHelper)
        m_pDropTargetHelper->DragEnter(m_hTargetWnd, pDataObj, reinterpret_cast<LPPOINT>(&pt), *pdwEffect);

    m_pSupportedFrmt = nullptr;
    for (size_t i = 0; i < m_formatEtc.size(); ++i)
    {
        m_bAllowDrop = (pDataObj->QueryGetData(&m_formatEtc[i]) == S_OK) ? true : false;
        if (m_bAllowDrop)
        {
            m_pSupportedFrmt = &m_formatEtc[i];
            break;
        }
    }

    QueryDrop(grfKeyState, pdwEffect);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CIDropTarget::DragOver(
    DWORD  grfKeyState,
    POINTL pt,
    DWORD __RPC_FAR *pdwEffect)
{
    if (m_pDropTargetHelper)
        m_pDropTargetHelper->DragOver(reinterpret_cast<LPPOINT>(&pt), *pdwEffect);
    QueryDrop(grfKeyState, pdwEffect);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CIDropTarget::DragLeave()
{
    if (m_pDropTargetHelper)
        m_pDropTargetHelper->DragLeave();

    m_bAllowDrop     = false;
    m_pSupportedFrmt = nullptr;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CIDropTarget::Drop(
    IDataObject __RPC_FAR *pDataObj,
    DWORD grfKeyState, POINTL pt,
    DWORD __RPC_FAR *pdwEffect)
{
    if (pDataObj == nullptr)
        return E_INVALIDARG;

    if (m_pDropTargetHelper)
        m_pDropTargetHelper->Drop(pDataObj, reinterpret_cast<LPPOINT>(&pt), *pdwEffect);

    if (QueryDrop(grfKeyState, pdwEffect))
    {
        if (m_bAllowDrop && m_pSupportedFrmt != nullptr)
        {
            STGMEDIUM medium;
            if (pDataObj->GetData(m_pSupportedFrmt, &medium) == S_OK)
            {
                if (OnDrop(m_pSupportedFrmt, medium, pdwEffect)) //does derive class wants us to free medium?
                    ReleaseStgMedium(&medium);
            }
        }
    }
    m_bAllowDrop     = false;
    *pdwEffect       = DROPEFFECT_NONE;
    m_pSupportedFrmt = nullptr;
    return S_OK;
}
