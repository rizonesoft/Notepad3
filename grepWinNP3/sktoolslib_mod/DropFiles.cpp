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
#include "DropFiles.h"
#include "UnicodeUtils.h"
#include "maxpath.h"


#ifndef __IDataObjectAsyncCapability_FWD_DEFINED__
#define IID_IDataObjectAsyncCapability IID_IAsyncOperation
#endif

CLIPFORMAT CF_FILECONTENTS = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILECONTENTS);
CLIPFORMAT CF_FILEDESCRIPTOR = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
CLIPFORMAT CF_PREFERREDDROPEFFECT = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);

FileDataObject::FileDataObject(const std::vector<std::wstring>& paths)
    : m_bInOperation(FALSE)
    , m_bIsAsync(TRUE)
    , m_cRefCount(0)
    , m_allPaths(paths)
{
}

FileDataObject::~FileDataObject()
{
    for (size_t i = 0; i < m_vecStgMedium.size(); ++i)
    {
        ReleaseStgMedium(m_vecStgMedium[i]);
        delete m_vecStgMedium[i];
    }

    for (size_t j = 0; j < m_vecFormatEtc.size(); ++j)
        delete m_vecFormatEtc[j];
}

//////////////////////////////////////////////////////////////////////////
// IUnknown
//////////////////////////////////////////////////////////////////////////

STDMETHODIMP FileDataObject::QueryInterface(REFIID riid, void** ppvObject)
{
    *ppvObject = NULL;
    if (IID_IUnknown == riid || IID_IDataObject == riid)
        *ppvObject=this;
    if (riid == IID_IDataObjectAsyncCapability)
        *ppvObject = (IDataObjectAsyncCapability*)this;

    if (NULL != *ppvObject)
    {
        ((LPUNKNOWN)*ppvObject)->AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) FileDataObject::AddRef(void)
{
    return ++m_cRefCount;
}

STDMETHODIMP_(ULONG) FileDataObject::Release(void)
{
    --m_cRefCount;
    if (m_cRefCount == 0)
    {
        delete this;
        return 0;
    }
    return m_cRefCount;
}

//////////////////////////////////////////////////////////////////////////
// IDataObject
//////////////////////////////////////////////////////////////////////////
STDMETHODIMP FileDataObject::GetData(FORMATETC* pformatetcIn, STGMEDIUM* pmedium)
{
    if (pformatetcIn == NULL || pmedium == NULL)
        return E_INVALIDARG;
    pmedium->hGlobal = NULL;

    if ((pformatetcIn->tymed & TYMED_HGLOBAL) && (pformatetcIn->dwAspect == DVASPECT_CONTENT) && (pformatetcIn->cfFormat == CF_HDROP))
    {
        UINT uBuffSize = 0;
        for (auto it = m_allPaths.begin(); it != m_allPaths.end(); ++it)
        {
            uBuffSize += (UINT)it->size();
            uBuffSize += 1;
        }
        uBuffSize = sizeof(DROPFILES) + sizeof(TCHAR) * ( uBuffSize + 1 );

        HGLOBAL    hgDrop;
        DROPFILES* pDrop;

        // Allocate memory from the heap for the DROPFILES struct.
        hgDrop = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT, uBuffSize);
        if (NULL == hgDrop)
            return E_OUTOFMEMORY;
        pDrop = (DROPFILES*)GlobalLock(hgDrop);

        if (NULL == pDrop)
        {
            GlobalFree(hgDrop);
            return E_OUTOFMEMORY;
        }
        // Fill in the DROPFILES struct.
        pDrop->pFiles = sizeof(DROPFILES);
#ifdef _UNICODE
        // If we're compiling for Unicode, set the Unicode flag in the struct to
        // indicate it contains Unicode strings.
        pDrop->fWide = TRUE;
#endif
        TCHAR* pszBuff;
        // Copy all the filenames into memory after
        // the end of the DROPFILES struct.
        pszBuff = (TCHAR*) (LPBYTE(pDrop) + sizeof(DROPFILES));
        for (auto it = m_allPaths.begin(); it != m_allPaths.end(); ++it)
        {
            lstrcpy(pszBuff, it->c_str());
            pszBuff += it->size();
            *pszBuff = 0;
            pszBuff++;
        }
        *pszBuff = 0;
        pszBuff = (TCHAR*) (LPBYTE(pDrop) + sizeof(DROPFILES));
        GlobalUnlock(hgDrop);
        pmedium->hGlobal = hgDrop;
        pmedium->tymed = TYMED_HGLOBAL;
        pmedium->pUnkForRelease = NULL;
        return S_OK;
    }
    // handling CF_PREFERREDDROPEFFECT is necessary to tell the shell that it should *not* ask for the
    // CF_FILEDESCRIPTOR until the drop actually occurs. If we don't handle CF_PREFERREDDROPEFFECT, the shell
    // will ask for the file descriptor for every object (file/folder) the mouse pointer hovers over and is
    // a potential drop target.
    else if ((pformatetcIn->tymed & TYMED_HGLOBAL) && (pformatetcIn->cfFormat == CF_PREFERREDDROPEFFECT))
    {
        HGLOBAL data = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE | GMEM_ZEROINIT, sizeof(DWORD));
        DWORD* effect = (DWORD*) GlobalLock(data);
        (*effect) = DROPEFFECT_COPY;
        GlobalUnlock(data);
        pmedium->hGlobal = data;
        pmedium->tymed = TYMED_HGLOBAL;
        return S_OK;
    }

    for (size_t i=0; i<m_vecFormatEtc.size(); ++i)
    {
        if ((pformatetcIn->tymed == m_vecFormatEtc[i]->tymed) &&
            (pformatetcIn->dwAspect == m_vecFormatEtc[i]->dwAspect) &&
            (pformatetcIn->cfFormat == m_vecFormatEtc[i]->cfFormat))
        {
            CopyMedium(pmedium, m_vecStgMedium[i], m_vecFormatEtc[i]);
            return S_OK;
        }
    }

    return DV_E_FORMATETC;
}

STDMETHODIMP FileDataObject::GetDataHere(FORMATETC* /*pformatetc*/, STGMEDIUM* /*pmedium*/)
{
    return E_NOTIMPL;
}

STDMETHODIMP FileDataObject::QueryGetData(FORMATETC* pformatetc)
{
    if (pformatetc == NULL)
        return E_INVALIDARG;

    if (!(DVASPECT_CONTENT & pformatetc->dwAspect))
        return DV_E_DVASPECT;

    if ((pformatetc->tymed & TYMED_HGLOBAL) &&
        (pformatetc->dwAspect == DVASPECT_CONTENT) &&
        (pformatetc->cfFormat == CF_PREFERREDDROPEFFECT))
    {
        return S_OK;
    }
    if ((pformatetc->tymed & TYMED_HGLOBAL) &&
        (pformatetc->dwAspect == DVASPECT_CONTENT) &&
        (pformatetc->cfFormat == CF_HDROP))
    {
        return S_OK;
    }

    for (size_t i=0; i<m_vecFormatEtc.size(); ++i)
    {
        if ((pformatetc->tymed == m_vecFormatEtc[i]->tymed) &&
            (pformatetc->dwAspect == m_vecFormatEtc[i]->dwAspect) &&
            (pformatetc->cfFormat == m_vecFormatEtc[i]->cfFormat))
            return S_OK;
    }

    return DV_E_TYMED;
}

STDMETHODIMP FileDataObject::GetCanonicalFormatEtc(FORMATETC* /*pformatectIn*/, FORMATETC* pformatetcOut)
{
    if (pformatetcOut == NULL)
        return E_INVALIDARG;
    return DATA_S_SAMEFORMATETC;
}

STDMETHODIMP FileDataObject::SetData(FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease)
{
    if ((pformatetc == NULL) || (pmedium == NULL))
        return E_INVALIDARG;

    FORMATETC* fetc = new (std::nothrow) FORMATETC;
    STGMEDIUM* pStgMed = new (std::nothrow) STGMEDIUM;

    if ((fetc == NULL) || (pStgMed == NULL))
    {
        delete fetc;
        delete pStgMed;
        return E_OUTOFMEMORY;
    }
    SecureZeroMemory(fetc,sizeof(FORMATETC));
    SecureZeroMemory(pStgMed,sizeof(STGMEDIUM));

    // do we already store this format?
    for (size_t i=0; i<m_vecFormatEtc.size(); ++i)
    {
        if ((pformatetc->tymed == m_vecFormatEtc[i]->tymed) &&
            (pformatetc->dwAspect == m_vecFormatEtc[i]->dwAspect) &&
            (pformatetc->cfFormat == m_vecFormatEtc[i]->cfFormat))
        {
            // yes, this format is already in our object:
            // we have to replace the existing format. To do that, we
            // remove the format we already have
            delete m_vecFormatEtc[i];
            m_vecFormatEtc.erase(m_vecFormatEtc.begin() + i);
            ReleaseStgMedium(m_vecStgMedium[i]);
            delete m_vecStgMedium[i];
            m_vecStgMedium.erase(m_vecStgMedium.begin() + i);
            break;
        }
    }

    *fetc = *pformatetc;
    m_vecFormatEtc.push_back(fetc);

    if (fRelease)
        *pStgMed = *pmedium;
    else
    {
        CopyMedium(pStgMed, pmedium, pformatetc);
    }
    m_vecStgMedium.push_back(pStgMed);

    return S_OK;
}

STDMETHODIMP FileDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc)
{
    if (ppenumFormatEtc == NULL)
        return E_POINTER;

    *ppenumFormatEtc = NULL;
    switch (dwDirection)
    {
    case DATADIR_GET:
        *ppenumFormatEtc= new CSVNEnumFormatEtc(m_vecFormatEtc);
        if (*ppenumFormatEtc == NULL)
            return E_OUTOFMEMORY;
        (*ppenumFormatEtc)->AddRef();
        break;
    default:
        return E_NOTIMPL;
    }
    return S_OK;
}

STDMETHODIMP FileDataObject::DAdvise(FORMATETC* /*pformatetc*/, DWORD /*advf*/, IAdviseSink* /*pAdvSink*/, DWORD* /*pdwConnection*/)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP FileDataObject::DUnadvise(DWORD /*dwConnection*/)
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE FileDataObject::EnumDAdvise(IEnumSTATDATA** /*ppenumAdvise*/)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

void FileDataObject::CopyMedium(STGMEDIUM* pMedDest, STGMEDIUM* pMedSrc, FORMATETC* pFmtSrc)
{
    switch(pMedSrc->tymed)
    {
    case TYMED_HGLOBAL:
        pMedDest->hGlobal = (HGLOBAL)OleDuplicateData(pMedSrc->hGlobal,pFmtSrc->cfFormat, NULL);
        break;
    case TYMED_GDI:
        pMedDest->hBitmap = (HBITMAP)OleDuplicateData(pMedSrc->hBitmap,pFmtSrc->cfFormat, NULL);
        break;
    case TYMED_MFPICT:
        pMedDest->hMetaFilePict = (HMETAFILEPICT)OleDuplicateData(pMedSrc->hMetaFilePict,pFmtSrc->cfFormat, NULL);
        break;
    case TYMED_ENHMF:
        pMedDest->hEnhMetaFile = (HENHMETAFILE)OleDuplicateData(pMedSrc->hEnhMetaFile,pFmtSrc->cfFormat, NULL);
        break;
    case TYMED_FILE:
        pMedSrc->lpszFileName = (LPOLESTR)OleDuplicateData(pMedSrc->lpszFileName,pFmtSrc->cfFormat, NULL);
        break;
    case TYMED_ISTREAM:
        pMedDest->pstm = pMedSrc->pstm;
        pMedSrc->pstm->AddRef();
        break;
    case TYMED_ISTORAGE:
        pMedDest->pstg = pMedSrc->pstg;
        pMedSrc->pstg->AddRef();
        break;
    case TYMED_NULL:
    default:
        break;
    }
    pMedDest->tymed = pMedSrc->tymed;
    pMedDest->pUnkForRelease = NULL;
    if (pMedSrc->pUnkForRelease != NULL)
    {
        pMedDest->pUnkForRelease = pMedSrc->pUnkForRelease;
        pMedSrc->pUnkForRelease->AddRef();
    }
}


//////////////////////////////////////////////////////////////////////////
// IDataObjectAsyncCapability
//////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE FileDataObject::SetAsyncMode(BOOL fDoOpAsync)
{
    m_bIsAsync = fDoOpAsync;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE FileDataObject::GetAsyncMode(BOOL* pfIsOpAsync)
{
    if (!pfIsOpAsync)
        return E_FAIL;

    *pfIsOpAsync = m_bIsAsync;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE FileDataObject::StartOperation(IBindCtx* /*pbcReserved*/)
{
    m_bInOperation = TRUE;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE FileDataObject::InOperation(BOOL* pfInAsyncOp)
{
    if (!pfInAsyncOp)
        return E_FAIL;

    *pfInAsyncOp = m_bInOperation;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE FileDataObject::EndOperation(HRESULT /*hResult*/, IBindCtx* /*pbcReserved*/, DWORD /*dwEffects*/)
{
    m_bInOperation = FALSE;
    Release();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE FileDataObject::SetDropDescription(DROPIMAGETYPE image, LPCTSTR format, LPCTSTR insert)
{
    if (format == NULL || insert == NULL)
        return E_INVALIDARG;

    FORMATETC fetc = {0};
    fetc.cfFormat = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DROPDESCRIPTION);
    fetc.dwAspect = DVASPECT_CONTENT;
    fetc.lindex = -1;
    fetc.tymed = TYMED_HGLOBAL;

    STGMEDIUM medium = {0};
    medium.hGlobal = GlobalAlloc(GHND, sizeof(DROPDESCRIPTION));
    if (medium.hGlobal == 0)
        return E_OUTOFMEMORY;

    DROPDESCRIPTION* pDropDescription = (DROPDESCRIPTION*)GlobalLock(medium.hGlobal);
    lstrcpynW(pDropDescription->szInsert, insert, _countof(pDropDescription->szInsert));
    lstrcpynW(pDropDescription->szMessage, format, _countof(pDropDescription->szMessage));
    pDropDescription->type = image;
    GlobalUnlock(medium.hGlobal);
    return SetData(&fetc, &medium, TRUE);
}

void CSVNEnumFormatEtc::Init()
{
    m_formats[1].cfFormat = CF_PREFERREDDROPEFFECT;
    m_formats[1].dwAspect = DVASPECT_CONTENT;
    m_formats[1].lindex = -1;
    m_formats[1].ptd = NULL;
    m_formats[1].tymed = TYMED_HGLOBAL;

    m_formats[0].cfFormat = CF_HDROP;
    m_formats[0].dwAspect = DVASPECT_CONTENT;
    m_formats[0].lindex = -1;
    m_formats[0].ptd = NULL;
    m_formats[0].tymed = TYMED_HGLOBAL;

}

CSVNEnumFormatEtc::CSVNEnumFormatEtc(const std::vector<FORMATETC>& vec) : m_cRefCount(0)
    , m_iCur(0)
{
    for (size_t i = 0; i < vec.size(); ++i)
        m_vecFormatEtc.push_back(vec[i]);
    Init();
}

CSVNEnumFormatEtc::CSVNEnumFormatEtc(const std::vector<FORMATETC*>& vec) : m_cRefCount(0)
    , m_iCur(0)
{
    for (size_t i = 0; i < vec.size(); ++i)
        m_vecFormatEtc.push_back(*vec[i]);
    Init();
}

STDMETHODIMP  CSVNEnumFormatEtc::QueryInterface(REFIID refiid, void** ppv)
{
    *ppv = NULL;
    if (IID_IUnknown == refiid || IID_IEnumFORMATETC == refiid)
        *ppv=this;

    if (*ppv != NULL)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CSVNEnumFormatEtc::AddRef(void)
{
    return ++m_cRefCount;
}

STDMETHODIMP_(ULONG) CSVNEnumFormatEtc::Release(void)
{
    --m_cRefCount;
    if (m_cRefCount == 0)
    {
        delete this;
        return 0;
    }
    return m_cRefCount;
}

STDMETHODIMP CSVNEnumFormatEtc::Next(ULONG celt, LPFORMATETC lpFormatEtc, ULONG* pceltFetched)
{
    if (celt <= 0)
        return E_INVALIDARG;
    if (pceltFetched == NULL && celt != 1) // pceltFetched can be NULL only for 1 item request
        return E_POINTER;
    if (lpFormatEtc == NULL)
        return E_POINTER;

    if (pceltFetched != NULL)
        *pceltFetched = 0;

    if (m_iCur >= DRAG_NUMFORMATS)
        return S_FALSE;

    ULONG cReturn = celt;

    while (m_iCur < (DRAG_NUMFORMATS + m_vecFormatEtc.size()) && cReturn > 0)
    {
        if (m_iCur < DRAG_NUMFORMATS)
            *lpFormatEtc++ = m_formats[m_iCur++];
        else
            *lpFormatEtc++ = m_vecFormatEtc[m_iCur++ - DRAG_NUMFORMATS];
        --cReturn;
    }

    if (pceltFetched != NULL)
        *pceltFetched = celt - cReturn;

    return (cReturn == 0) ? S_OK : S_FALSE;
}

STDMETHODIMP CSVNEnumFormatEtc::Skip(ULONG celt)
{
    if ((m_iCur + int(celt)) >= (DRAG_NUMFORMATS + m_vecFormatEtc.size()))
        return S_FALSE;
    m_iCur += celt;
    return S_OK;
}

STDMETHODIMP CSVNEnumFormatEtc::Reset(void)
{
    m_iCur = 0;
    return S_OK;
}

STDMETHODIMP CSVNEnumFormatEtc::Clone(IEnumFORMATETC** ppCloneEnumFormatEtc)
{
    if (ppCloneEnumFormatEtc == NULL)
        return E_POINTER;

    try
    {
        CSVNEnumFormatEtc *newEnum = new CSVNEnumFormatEtc(m_vecFormatEtc);

        newEnum->AddRef();
        newEnum->m_iCur = m_iCur;
        *ppCloneEnumFormatEtc = newEnum;
    }
    catch (const std::bad_alloc&)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}





CDropFiles::CDropFiles()
{
}

CDropFiles::~CDropFiles()
{
}

void CDropFiles::AddFile(const std::wstring& sFile)
{
    m_arFiles.push_back(sFile);
}

size_t CDropFiles::GetCount()
{
    return m_arFiles.size();
}

void CDropFiles::CreateStructure(HWND hWnd)
{
    CIDropSource* pdsrc = new (std::nothrow) CIDropSource;
    if (pdsrc == NULL)
        return;
    pdsrc->AddRef();

    FileDataObject* pdobj = new FileDataObject(m_arFiles);
    if (pdobj == NULL)
    {
        delete pdsrc;
        return;
    }
    pdobj->AddRef();

    CDragSourceHelper dragsrchelper;
    POINT pt;
    GetCursorPos(&pt);
    dragsrchelper.InitializeFromWindow(hWnd, pt, pdobj);

    pdsrc->m_pIDataObj = pdobj;
    pdsrc->m_pIDataObj->AddRef();

    DWORD dwEffect;
    ::DoDragDrop(pdobj, pdsrc, DROPEFFECT_MOVE|DROPEFFECT_COPY, &dwEffect);
    pdsrc->Release();
    pdobj->Release();
}

//////////////////////////////////////////////////////////////////////
// CIDropSource Class
//////////////////////////////////////////////////////////////////////

STDMETHODIMP CIDropSource::QueryInterface(/* [in] */ REFIID riid,
    /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
{
    if (!ppvObject)
    {
        return E_POINTER;
    }

    if (riid == IID_IUnknown)
    {
        *ppvObject = (IUnknown*) dynamic_cast<IDropSource*>(this);
    }
    else if (riid == IID_IDropSource)
    {
        *ppvObject = dynamic_cast<IDropSource*>(this);
    }
    else if ((riid == IID_IDropSourceNotify) && (pDragSourceNotify != NULL))
    {
        return pDragSourceNotify->QueryInterface(riid, ppvObject);
    }
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) CIDropSource::AddRef( void)
{
    return ++m_cRefCount;
}

STDMETHODIMP_(ULONG) CIDropSource::Release( void)
{
    long nTemp;
    nTemp = --m_cRefCount;

    if (nTemp == 0)
        delete this;
    return nTemp;
}

STDMETHODIMP CIDropSource::QueryContinueDrag(
    /* [in] */ BOOL fEscapePressed,
    /* [in] */ DWORD grfKeyState)
{
    if (fEscapePressed)
        return DRAGDROP_S_CANCEL;
    if(!(grfKeyState & (MK_LBUTTON|MK_RBUTTON)))
    {
        m_bDropped = true;
        return DRAGDROP_S_DROP;
    }

    return S_OK;
}

STDMETHODIMP CIDropSource::GiveFeedback(
    /* [in] */ DWORD /*dwEffect*/)
{
    if (m_pIDataObj)
    {
        FORMATETC fetc = {0};
        fetc.cfFormat = (CLIPFORMAT)RegisterClipboardFormat(_T("DragWindow"));
        fetc.dwAspect = DVASPECT_CONTENT;
        fetc.lindex = -1;
        fetc.tymed = TYMED_HGLOBAL;
        if (m_pIDataObj->QueryGetData(&fetc) == S_OK)
        {
            STGMEDIUM medium;
            if (m_pIDataObj->GetData(&fetc, &medium) == S_OK)
            {
                HWND hWndDragWindow = *((HWND*) GlobalLock(medium.hGlobal));
                GlobalUnlock(medium.hGlobal);
#define WM_INVALIDATEDRAGIMAGE (WM_USER + 3)
                SendMessage(hWndDragWindow, WM_INVALIDATEDRAGIMAGE, NULL, NULL);
                ReleaseStgMedium(&medium);
            }
        }
    }
    return DRAGDROP_S_USEDEFAULTCURSORS;
}
