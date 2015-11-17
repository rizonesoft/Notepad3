/******************************************************************************
*                                                                             *
*                                                                             *
* MiniPath - Notepad3 Explorer Plugin                                         *
*                                                                             *
* Dropsource.h                                                                *
*   OLE drop source functionality                                             *
*   Based on code from metapath, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2016   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

class FAR CDropSource : public IDropSource
{

public:
  CDropSource();

  /* IUnknown methods */
  STDMETHOD(QueryInterface)(REFIID riid,void FAR* FAR* ppvObj);
  STDMETHOD_(ULONG,AddRef)();
  STDMETHOD_(ULONG,Release)();

  /* IDropSource methods */
  STDMETHOD(QueryContinueDrag)(BOOL fEscapePressed,DWORD grfKeyState);
  STDMETHOD(GiveFeedback)(DWORD dwEffect);

private:
  ULONG m_refs;

};


// End of Dropsource.h
