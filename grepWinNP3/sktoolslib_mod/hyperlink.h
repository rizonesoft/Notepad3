/*
 * Module ID: hyperlink.h
 * Title    : CHyperLink Declaration.
 *
 * Author   : Olivier Langlois <olivier@olivierlanglois.net>
 * Date     : November 15, 2005
 *
 * To read the article describing this class, visit
 * http://www.olivierlanglois.net/hyperlinkdemo.htm
 *
 * Note: Strongly inspired by Neal Stublen code
 *       Minor ideas come from Chris Maunder and Paul DiLascia code
 *
 * Revision :
 *
 * 001        26-Nov-2005 - Olivier Langlois
 *            - Added changes to make CHyperLink compatible with UNICODE
 *            - Use dynamic memory allocation for the URL string
 */

#pragma once
#include <windows.h>
#include <memory>

class CHyperLink
{
public:
    CHyperLink();
    virtual ~CHyperLink();

    BOOL ConvertStaticToHyperlink(HWND hwndCtl, LPCWSTR strURL);
    BOOL ConvertStaticToHyperlink(HWND hwndParent, UINT uiCtlId, LPCWSTR strURL);

    BOOL    setURL(LPCWSTR strURL);
    LPCWSTR getURL() const { return m_strURL.get(); }

protected:
    /*
     * Override if you want to perform some action when the link has the focus
     * or when the cursor is over the link such as displaying the URL somewhere.
     */
    virtual void OnSelect() {}
    virtual void OnDeselect() {}

    std::unique_ptr<wchar_t[]> m_strURL; // hyperlink URL

private:
    static COLORREF m_gCrLinkColor, m_gCrVisitedColor; // Hyperlink colors
    static HCURSOR  m_gHLinkCursor;                    // Cursor for hyperlink
    static HFONT    m_gUnderlineFont;                  // Font for underline display
    static int      m_gCounter;                        // Global resources user counter
    BOOL            m_bOverControl;                    // cursor over control?
    BOOL            m_bVisited;                        // Has it been visited?
    HFONT           m_stdFont;                         // Standard font
    WNDPROC         m_pfnOrigCtlProc;

    void        createUnderlineFont() const;
    static void createLinkCursor();
    void        createGlobalResources() const
    {
        createUnderlineFont();
        createLinkCursor();
    }
    static void destroyGlobalResources()
    {
        /*
         * No need to call DestroyCursor() for cursors acquired through
         * LoadCursor().
         */
        m_gHLinkCursor = nullptr;
        DeleteObject(m_gUnderlineFont);
        m_gUnderlineFont = nullptr;
    }

    void Navigate();

    static void             DrawFocusRect(HWND hwnd);
    static LRESULT CALLBACK _HyperlinkParentProc(HWND hwnd, UINT message,
                                                 WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK _HyperlinkProc(HWND hwnd, UINT message,
                                           WPARAM wParam, LPARAM lParam);
};
