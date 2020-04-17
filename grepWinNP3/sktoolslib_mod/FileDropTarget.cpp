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
#include "FileDropTarget.h"
#include <ShlGuid.h>
#include <ShObjIdl.h >

CIDropTarget::CIDropTarget(HWND hTargetWnd)
    : m_hTargetWnd(hTargetWnd)
    , m_cRefCount(0)
    , m_bAllowDrop(false)
    , m_pDropTargetHelper(NULL)
    , m_pSupportedFrmt(NULL)
{
    if (FAILED(CoCreateInstance(CLSID_DragDropHelper,NULL,CLSCTX_INPROC_SERVER,
                     IID_IDropTargetHelper,(LPVOID*)&m_pDropTargetHelper)))
        m_pDropTargetHelper = NULL;
}

CIDropTarget::~CIDropTarget()
{
    if (m_pDropTargetHelper != NULL)
    {
        m_pDropTargetHelper->Release();
        m_pDropTargetHelper = NULL;
    }
}

HRESULT STDMETHODCALLTYPE CIDropTarget::QueryInterface( /* [in] */ REFIID riid,
                        /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
{
   *ppvObject = NULL;
   if (IID_IUnknown == riid || IID_IDropTarget == riid)
             *ppvObject=this;

    if (*ppvObject != NULL)
    {
        ((LPUNKNOWN)*ppvObject)->AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CIDropTarget::Release( void)
{
   long nTemp;
   nTemp = --m_cRefCount;
   if (nTemp == 0)
      delete this;
   return nTemp;
}

bool CIDropTarget::QueryDrop(DWORD grfKeyState, LPDWORD pdwEffect)
{
    DWORD dwOKEffects = *pdwEffect;

    if(!m_bAllowDrop)
    {
       *pdwEffect = DROPEFFECT_NONE;
       return false;
    }
    //CTRL+SHIFT  -- DROPEFFECT_LINK
    //CTRL        -- DROPEFFECT_COPY
    //SHIFT       -- DROPEFFECT_MOVE
    //no modifier -- DROPEFFECT_MOVE or whatever is allowed by src
    *pdwEffect = (grfKeyState & MK_CONTROL) ?
                 ( (grfKeyState & MK_SHIFT) ? DROPEFFECT_LINK : DROPEFFECT_COPY ):
                 ( (grfKeyState & MK_SHIFT) ? DROPEFFECT_MOVE : 0 );
    if(*pdwEffect == 0)
    {
       // No modifier keys used by user while dragging.
       if (DROPEFFECT_COPY & dwOKEffects)
          *pdwEffect = DROPEFFECT_COPY;
       else if (DROPEFFECT_MOVE & dwOKEffects)
          *pdwEffect = DROPEFFECT_MOVE;
       else if (DROPEFFECT_LINK & dwOKEffects)
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
       if(!(*pdwEffect & dwOKEffects))
          *pdwEffect = DROPEFFECT_NONE;
    }

    return (DROPEFFECT_NONE == *pdwEffect)?false:true;
}

HRESULT STDMETHODCALLTYPE CIDropTarget::DragEnter(
    /* [unique][in] */ IDataObject __RPC_FAR *pDataObj,
    /* [in] */ DWORD grfKeyState,
    /* [in] */ POINTL pt,
    /* [out][in] */ DWORD __RPC_FAR *pdwEffect)
{
    if (pDataObj == NULL)
        return E_INVALIDARG;

    if (m_pDropTargetHelper)
        m_pDropTargetHelper->DragEnter(m_hTargetWnd, pDataObj, (LPPOINT)&pt, *pdwEffect);

    m_pSupportedFrmt = NULL;
    for (size_t i =0; i<m_formatetc.size(); ++i)
    {
        m_bAllowDrop = (pDataObj->QueryGetData(&m_formatetc[i]) == S_OK)?true:false;
        if (m_bAllowDrop)
        {
            m_pSupportedFrmt = &m_formatetc[i];
            break;
        }
    }

    QueryDrop(grfKeyState, pdwEffect);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CIDropTarget::DragOver(
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ DWORD __RPC_FAR *pdwEffect)
{
    if (m_pDropTargetHelper)
        m_pDropTargetHelper->DragOver((LPPOINT)&pt, *pdwEffect);
    QueryDrop(grfKeyState, pdwEffect);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CIDropTarget::DragLeave( void)
{
    if (m_pDropTargetHelper)
        m_pDropTargetHelper->DragLeave();

    m_bAllowDrop = false;
    m_pSupportedFrmt = NULL;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CIDropTarget::Drop(
    /* [unique][in] */ IDataObject __RPC_FAR *pDataObj,
    /* [in] */ DWORD grfKeyState, /* [in] */ POINTL pt,
    /* [out][in] */ DWORD __RPC_FAR *pdwEffect)
{
    if (pDataObj == NULL)
        return E_INVALIDARG;

    if (m_pDropTargetHelper)
        m_pDropTargetHelper->Drop(pDataObj, (LPPOINT)&pt, *pdwEffect);

    if (QueryDrop(grfKeyState, pdwEffect))
    {
        if (m_bAllowDrop && m_pSupportedFrmt != NULL)
        {
            STGMEDIUM medium;
            if (pDataObj->GetData(m_pSupportedFrmt, &medium) == S_OK)
            {
                if (OnDrop(m_pSupportedFrmt, medium, pdwEffect)) //does derive class wants us to free medium?
                    ReleaseStgMedium(&medium);
            }
        }
    }
    m_bAllowDrop=false;
    *pdwEffect = DROPEFFECT_NONE;
    m_pSupportedFrmt = NULL;
    return S_OK;
}
