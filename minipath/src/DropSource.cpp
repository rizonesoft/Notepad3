// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* MiniPath - Notepad3 Explorer Plugin                                         *
*                                                                             *
* Dropsource.cpp                                                              *
*   OLE drop source functionality                                             *
*   Based on code from metapath, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2023   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#define _WIN32_WINNT 0x601
#include <windows.h>
//#include <strsafe.h>
#include "DropSource.h"

/******************************************************************************
*
* IUnknown Implementation
*
******************************************************************************/
STDMETHODIMP CDropSource::QueryInterface(REFIID iid, PVOID *ppv) noexcept
{
    if (iid == IID_IUnknown || iid == IID_IDropSource) {
        *ppv = this;
        AddRef();
        return NOERROR;
    }
    *ppv = nullptr;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CDropSource::AddRef() noexcept
{
    return ++m_refs;
}

STDMETHODIMP_(ULONG) CDropSource::Release() noexcept
{
    const ULONG refs = --m_refs;
    if (refs == 0) {
        delete this;
    }
    return refs;
}

/******************************************************************************
*
* CDropSource Constructor
*
******************************************************************************/
CDropSource::CDropSource() noexcept
{
    m_refs = 1;
}

/******************************************************************************
*
* IDropSource Implementation
*
******************************************************************************/
STDMETHODIMP CDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState) noexcept
{
    if (fEscapePressed) {
        return DRAGDROP_S_CANCEL;
    }
    if (!(grfKeyState & MK_LBUTTON) && !(grfKeyState & MK_RBUTTON)) {
        return DRAGDROP_S_DROP;
    }
    return NOERROR;
}

STDMETHODIMP CDropSource::GiveFeedback(DWORD dwEffect) noexcept
{
    (void)(dwEffect);
    return DRAGDROP_S_USEDEFAULTCURSORS;
}

extern "C" {

    LPDROPSOURCE CreateDropSource(void)
    {
        return ((LPDROPSOURCE) new CDropSource);
    }

}

// End of DropSource.cpp
