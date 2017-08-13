/******************************************************************************
*                                                                             *
*                                                                             *
* MiniPath - Notepad3 Explorer Plugin                                         *
*                                                                             *
* Dropsource.cpp                                                              *
*   OLE drop source functionality                                             *
*   Based on code from metapath, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2016   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#define _WIN32_WINNT 0x501
#include <windows.h>
#include "helpers.h"
#include "dropsource.h"


/******************************************************************************
*
* IUnknown Implementation
*
******************************************************************************/
STDMETHODIMP CDropSource::QueryInterface(REFIID iid, void FAR* FAR* ppv)
{
  if (iid == IID_IUnknown || iid == IID_IDropSource)
  {
    *ppv = this;
    ++m_refs;
    return NOERROR;
  }
  *ppv = NULL;
  return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG) CDropSource::AddRef()
{
  return ++m_refs;
}


STDMETHODIMP_(ULONG) CDropSource::Release()
{
  if(--m_refs == 0)
  {
    delete this;
    return 0;
  }
  return m_refs;
}


/******************************************************************************
*
* CDropSource Constructor
*
******************************************************************************/
CDropSource::CDropSource()
{
  m_refs = 1;
}


/******************************************************************************
*
* IDropSource Implementation
*
******************************************************************************/
STDMETHODIMP CDropSource::QueryContinueDrag(BOOL fEscapePressed,
                                            DWORD grfKeyState)
{
  if (fEscapePressed)
    return DRAGDROP_S_CANCEL;

  else if (!(grfKeyState & MK_LBUTTON) && !(grfKeyState & MK_RBUTTON))
    return DRAGDROP_S_DROP;

  else
    return NOERROR;
}


STDMETHODIMP CDropSource::GiveFeedback(DWORD dwEffect)
{
  UNUSED(dwEffect);
  return DRAGDROP_S_USEDEFAULTCURSORS;
}


extern "C" {
LPDROPSOURCE CreateDropSource()
{
  return ((LPDROPSOURCE) new CDropSource);
}
}


// End of Dropsource.cpp
