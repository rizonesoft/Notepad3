// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* MiniPath - Notepad3 Explorer Plugin                                         *
*                                                                             *
* Dropsource.h                                                                *
*   OLE drop source functionality                                             *
*   Based on code from metapath, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2025   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

#ifndef METAPATH_DROPSOURCE_H_
#define METAPATH_DROPSOURCE_H_

class CDropSource : public IDropSource
{
public:
    CDropSource() noexcept;
    virtual ~CDropSource() = default;

    /* IUnknown methods */
    STDMETHODIMP QueryInterface(REFIID riid, PVOID *ppv) noexcept override;
    STDMETHODIMP_(ULONG)AddRef() noexcept override;
    STDMETHODIMP_(ULONG)Release() noexcept override;

    /* IDropSource methods */
    STDMETHODIMP QueryContinueDrag(BOOL fEsc, DWORD grfKeyState) noexcept override;
    STDMETHODIMP GiveFeedback(DWORD) noexcept override;

private:
    ULONG m_refs;
};

#endif // METAPATH_DROPSOURCE_H_


// End of Dropsource.h
