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

#include <Shlobj.h>
#include <string>
#include <vector>

#include <Shldisp.h>

#ifndef __IDataObjectAsyncCapability_FWD_DEFINED__
#define IDataObjectAsyncCapability IAsyncOperation
#endif

#define DRAG_NUMFORMATS 2

/**
 * Use this class to create the DROPFILES structure which is needed to
 * support drag and drop of file names to other applications.
 */
class CDropFiles
{
public:
    CDropFiles();
    ~CDropFiles();

    /**
     * Add a file with an absolute file name. This file will later be
     * included the DROPFILES structure.
     */
    void AddFile(const std::wstring& sFile);

    /**
     * Returns the number of files which have been added
     */
    size_t GetCount();

    /**
     * Call this method when dragging begins. It will fill
     * the DROPFILES structure with the files previously
     * added with AddFile(...)
     */
    void CreateStructure(HWND hWnd);

protected:
    std::vector<std::wstring> m_arFiles;
};


class CDragSourceNotify : IDropSourceNotify
{
private:
    LONG refCount;

public:

    CDragSourceNotify(void)
    {
        refCount = 0;
    }

    ~CDragSourceNotify(void)
    {
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject)
    {
        if(!ppvObject) {
            return E_POINTER;
        }

        if (riid == IID_IUnknown) {
            *ppvObject = (IUnknown*) dynamic_cast<IDropSourceNotify*>(this);
        }
        else if (riid == IID_IDropSourceNotify) {
            *ppvObject = dynamic_cast<IDropSourceNotify*>(this);
        }
        else {
            *ppvObject = NULL;
            return E_NOINTERFACE;
        }

        AddRef();
        return S_OK;
    }

    ULONG STDMETHODCALLTYPE AddRef(void)
    {
        return InterlockedIncrement(&refCount);
    }

    ULONG STDMETHODCALLTYPE Release(void)
    {
        ULONG ret = InterlockedDecrement(&refCount);
        if(!ret) {
            delete this;
        }
        return ret;
    }

    HRESULT STDMETHODCALLTYPE DragEnterTarget(HWND /*hWndTarget*/)
    {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE DragLeaveTarget(void)
    {
        return S_OK;
    }

};


class CIDropSource : public IDropSource
{
    long m_cRefCount;
public:
    bool m_bDropped;
    IDataObject * m_pIDataObj;
    CDragSourceNotify* pDragSourceNotify;


    CIDropSource()
        : m_cRefCount(0)
        , m_bDropped(false)
        , m_pIDataObj(NULL)
    {
        pDragSourceNotify = new CDragSourceNotify();
        pDragSourceNotify->AddRef();
    }
    ~CIDropSource()
    {
        if (m_pIDataObj)
        {
            m_pIDataObj->Release();
            m_pIDataObj = NULL;
        }
        if (pDragSourceNotify)
        {
            pDragSourceNotify->Release();
            pDragSourceNotify = NULL;
        }
    }
    //IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef( void);
    virtual ULONG STDMETHODCALLTYPE Release( void);
    //IDropSource
    virtual HRESULT STDMETHODCALLTYPE QueryContinueDrag(
        /* [in] */ BOOL fEscapePressed,
        /* [in] */ DWORD grfKeyState);

    virtual HRESULT STDMETHODCALLTYPE GiveFeedback(
        /* [in] */ DWORD dwEffect);
};



extern  CLIPFORMAT  CF_FILECONTENTS;
extern  CLIPFORMAT  CF_FILEDESCRIPTOR;
extern  CLIPFORMAT  CF_PREFERREDDROPEFFECT;

/**
 * Represents a path as an IDataObject.
 * This can be used for drag and drop operations to other applications like
 * the shell itself.
 */
class FileDataObject : public IDataObject, public IDataObjectAsyncCapability
{
public:
    /**
     * Constructs the FileDataObject.
     * \param paths    a list of paths.
     */
    FileDataObject(const std::vector<std::wstring>& paths);
    ~FileDataObject();

    //IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef(void);
    virtual ULONG STDMETHODCALLTYPE Release(void);

    //IDataObject
    virtual HRESULT STDMETHODCALLTYPE GetData(FORMATETC* pformatetcIn, STGMEDIUM* pmedium);
    virtual HRESULT STDMETHODCALLTYPE GetDataHere(FORMATETC* pformatetc, STGMEDIUM* pmedium);
    virtual HRESULT STDMETHODCALLTYPE QueryGetData(FORMATETC* pformatetc);
    virtual HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc(FORMATETC* pformatectIn, FORMATETC* pformatetcOut);
    virtual HRESULT STDMETHODCALLTYPE SetData(FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease);
    virtual HRESULT STDMETHODCALLTYPE EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc);
    virtual HRESULT STDMETHODCALLTYPE DAdvise(FORMATETC* pformatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection);
    virtual HRESULT STDMETHODCALLTYPE DUnadvise(DWORD dwConnection);
    virtual HRESULT STDMETHODCALLTYPE EnumDAdvise(IEnumSTATDATA** ppenumAdvise);

    //IDataObjectAsyncCapability
    virtual HRESULT STDMETHODCALLTYPE SetAsyncMode(BOOL fDoOpAsync);
    virtual HRESULT STDMETHODCALLTYPE GetAsyncMode(BOOL* pfIsOpAsync);
    virtual HRESULT STDMETHODCALLTYPE StartOperation(IBindCtx* pbcReserved);
    virtual HRESULT STDMETHODCALLTYPE InOperation(BOOL* pfInAsyncOp);
    virtual HRESULT STDMETHODCALLTYPE EndOperation(HRESULT hResult, IBindCtx* pbcReserved, DWORD dwEffects);

    virtual HRESULT STDMETHODCALLTYPE SetDropDescription(DROPIMAGETYPE image, LPCTSTR format, LPCTSTR insert);

private:
    void CopyMedium(STGMEDIUM* pMedDest, STGMEDIUM* pMedSrc, FORMATETC* pFmtSrc);

    std::vector<std::wstring>   m_allPaths;
    long                        m_cRefCount;
    BOOL                        m_bInOperation;
    BOOL                        m_bIsAsync;
    std::vector<FORMATETC*>     m_vecFormatEtc;
    std::vector<STGMEDIUM*>     m_vecStgMedium;
};


/**
* Helper class for the FileDataObject class: implements the enumerator
* for the supported clipboard formats of the FileDataObject class.
*/
class CSVNEnumFormatEtc : public IEnumFORMATETC
{
public:
    CSVNEnumFormatEtc(const std::vector<FORMATETC*>& vec);
    CSVNEnumFormatEtc(const std::vector<FORMATETC>& vec);
    //IUnknown members
    STDMETHOD(QueryInterface)(REFIID, void**);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

    //IEnumFORMATETC members
    STDMETHOD(Next)(ULONG, LPFORMATETC, ULONG*);
    STDMETHOD(Skip)(ULONG);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(IEnumFORMATETC**);
private:
    void                        Init();

    std::vector<FORMATETC>      m_vecFormatEtc;
    FORMATETC                   m_formats[DRAG_NUMFORMATS];
    ULONG                       m_cRefCount;
    size_t                      m_iCur;
};

class CDragSourceHelper
{
    IDragSourceHelper2* pDragSourceHelper2;
    IDragSourceHelper* pDragSourceHelper;

public:
    CDragSourceHelper()
    {
        pDragSourceHelper = NULL;
        pDragSourceHelper2 = NULL;
        if (FAILED(CoCreateInstance(CLSID_DragDropHelper,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IDragSourceHelper2,
            (void**)&pDragSourceHelper2)))
        {
            pDragSourceHelper2 = NULL;
            if (FAILED(CoCreateInstance(CLSID_DragDropHelper,
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_IDragSourceHelper,
                (void**)&pDragSourceHelper)))
                pDragSourceHelper = NULL;
        }
    }
    virtual ~CDragSourceHelper()
    {
        if (pDragSourceHelper2 != NULL)
        {
            pDragSourceHelper2->Release();
            pDragSourceHelper2=NULL;
        }
        if (pDragSourceHelper != NULL)
        {
            pDragSourceHelper->Release();
            pDragSourceHelper=NULL;
        }
    }

    // IDragSourceHelper
    HRESULT InitializeFromBitmap(HBITMAP hBitmap,
        POINT& pt,  // cursor position in client coords of the window
        RECT& rc,   // selected item's bounding rect
        IDataObject* pDataObject,
        BOOL allowDropDescription=TRUE,
        COLORREF crColorKey=GetSysColor(COLOR_WINDOW)// color of the window used for transparent effect.
        )
    {
        if((pDragSourceHelper == NULL) && (pDragSourceHelper2 == NULL))
            return E_FAIL;

        if ((allowDropDescription) && (pDragSourceHelper2))
            pDragSourceHelper2->SetFlags(DSH_ALLOWDROPDESCRIPTIONTEXT);

        SHDRAGIMAGE di;
        BITMAP      bm;
        GetObject(hBitmap, sizeof(bm), &bm);
        di.sizeDragImage.cx = bm.bmWidth;
        di.sizeDragImage.cy = bm.bmHeight;
        di.hbmpDragImage = hBitmap;
        di.crColorKey = crColorKey;
        di.ptOffset.x = pt.x - rc.left;
        di.ptOffset.y = pt.y - rc.top;
        if (pDragSourceHelper2)
            return pDragSourceHelper2->InitializeFromBitmap(&di, pDataObject);
        return pDragSourceHelper->InitializeFromBitmap(&di, pDataObject);
    }
    HRESULT InitializeFromWindow(HWND hwnd, POINT& pt, IDataObject* pDataObject, BOOL allowDropDescription=TRUE)
    {
        if((pDragSourceHelper == NULL) && (pDragSourceHelper2 == NULL))
            return E_FAIL;
        if ((allowDropDescription) && (pDragSourceHelper2))
            pDragSourceHelper2->SetFlags(DSH_ALLOWDROPDESCRIPTIONTEXT);
        if (pDragSourceHelper2)
            return pDragSourceHelper2->InitializeFromWindow(hwnd, &pt, pDataObject);
        return pDragSourceHelper->InitializeFromWindow(hwnd, &pt, pDataObject);
    }
};
