// sktoolslib - common files for SK tools

// Copyright (C) 2012, 2014, 2017 - Stefan Kueng

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
#include "AeroControls.h"


enum ControlType
{
    Static,
    Button,
    Progressbar
};

#ifndef RECTWIDTH
#define RECTWIDTH(rc)   ((rc).right-(rc).left)
#endif

#ifndef RECTHEIGHT
#define RECTHEIGHT(rc)  ((rc).bottom-(rc).top)
#endif

using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "comctl32.lib")

AeroControlBase::AeroControlBase()
{
    GdiplusStartupInput gdiplusStartupInput;

    m_dwm.Initialize();
    m_theme.Initialize();
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
}

AeroControlBase::~AeroControlBase()
{
    for (std::map<HWND, UINT_PTR>::const_iterator it = subclassedControls.begin(); it != subclassedControls.end(); ++it)
    {
        RemoveWindowSubclass(it->first, SubclassProc, it->second);
    }
    GdiplusShutdown(gdiplusToken);
}

bool AeroControlBase::SubclassControl(HWND hControl)
{
    bool bRet = false;
    if (!m_dwm.IsDwmCompositionEnabled())
        return bRet;
    TCHAR szWndClassName[MAX_PATH];
    if (GetClassName(hControl, szWndClassName, _countof(szWndClassName)))
    {
        if (!_tcscmp(szWndClassName, _T("Static")))
        {
            bRet = !!SetWindowSubclass(hControl, SubclassProc, Static, (DWORD_PTR)this);
            subclassedControls[hControl] = Static;
        }
        if (!_tcscmp(szWndClassName, _T("Button")))
        {
            bRet = !!SetWindowSubclass(hControl, SubclassProc, Button, (DWORD_PTR)this);
            subclassedControls[hControl] = Button;
        }
        if(!_tcscmp(szWndClassName, _T("msctls_progress32")))
        {
            bRet = !!SetWindowSubclass(hControl, SubclassProc, Progressbar, (DWORD_PTR)this);
            subclassedControls[hControl] = Progressbar;
        }
    }
    return bRet;
}

LRESULT AeroControlBase::SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uidSubclass, DWORD_PTR dwRefData)
{
    AeroControlBase * pThis = (AeroControlBase*)dwRefData;
    if (pThis)
    {
        if (pThis->m_dwm.IsDwmCompositionEnabled())
        {
            switch (uidSubclass)
            {
            case Static:
                return pThis->StaticWindowProc(hWnd, uMsg, wParam, lParam);
                break;
            case Button:
                return pThis->ButtonWindowProc(hWnd, uMsg, wParam, lParam);
                break;
            case Progressbar:
                return pThis->ProgressbarWindowProc(hWnd, uMsg, wParam, lParam);
                break;
            }
        }
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

LRESULT AeroControlBase::StaticWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_SETTEXT:
    case WM_ENABLE:
    case WM_STYLECHANGED:
        {
            LRESULT res = DefSubclassProc(hWnd, uMsg, wParam, lParam);
            InvalidateRgn(hWnd, nullptr, FALSE);
            return res;
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            if (hdc)
            {
                HDC hdcPaint = nullptr;
                RECT rcClient;
                GetClientRect(hWnd, &rcClient);

                LONG_PTR dwStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
                LONG_PTR dwStyleEx = GetWindowLongPtr(hWnd, GWL_EXSTYLE);

                HTHEME hTheme = m_theme.OpenThemeData(nullptr, L"ControlPanelStyle");


                if (hTheme)
                {

                    HPAINTBUFFER hBufferedPaint = m_theme.BeginBufferedPaint(hdc, &rcClient, BPBF_TOPDOWNDIB, nullptr, &hdcPaint);
                    if (hdcPaint)
                    {
                        PatBlt(hdcPaint, 0, 0, RECTWIDTH(rcClient), RECTHEIGHT(rcClient), BLACKNESS);
                        m_theme.BufferedPaintSetAlpha(hBufferedPaint, &ps.rcPaint, 0x00);
                        LONG_PTR dwStaticStyle = dwStyle&0x1F;


                        if (dwStaticStyle == SS_ICON || dwStaticStyle == SS_BITMAP)
                        {
                            bool bIcon = dwStaticStyle == SS_ICON;
                            HANDLE hBmpIco = (HANDLE)SendMessage(hWnd, STM_GETIMAGE, bIcon ? IMAGE_ICON:IMAGE_BITMAP, NULL);
                            if (hBmpIco)
                            {
                                std::unique_ptr<Bitmap> pBmp( bIcon ? new Bitmap((HICON)hBmpIco) : new Bitmap((HBITMAP)hBmpIco, nullptr) );
                                std::unique_ptr<Graphics> myGraphics( new Graphics(hdcPaint) );
                                std::unique_ptr<CachedBitmap> pcbmp( new CachedBitmap(pBmp.get(), myGraphics.get()) );
                                myGraphics->DrawCachedBitmap(pcbmp.get(), 0,0);
                            }
                        }
                        else if (SS_BLACKRECT == dwStaticStyle || SS_GRAYRECT == dwStaticStyle || SS_WHITERECT == dwStaticStyle)
                        {
                            ARGB argb = 0L;
                            switch (dwStaticStyle)
                            {
                            case SS_BLACKRECT:
                                argb = 0xFF000000;
                                break;
                            case SS_GRAYRECT:
                                argb = 0xFF808080;
                                break;
                            case SS_WHITERECT:
                                argb = 0xFFFFFFFF;
                                break;
                            default:
                                break;
                            }
                            Color clr(argb);

                            FillRect(&rcClient, hdcPaint, clr);
                        }
                        else if (SS_BLACKFRAME == dwStaticStyle || SS_GRAYFRAME == dwStaticStyle || SS_WHITEFRAME == dwStaticStyle)
                        {
                            ARGB argb = 0L;
                            switch (dwStaticStyle)
                            {
                            case SS_BLACKFRAME:
                                argb = 0xFF000000;
                                break;
                            case SS_GRAYFRAME:
                                argb = 0xFF808080;
                                break;
                            case SS_WHITEFRAME:
                                argb = 0xFFFFFFFF;
                                break;
                            default:
                                break;
                            }
                            Color clr(argb);

                            DrawRect(&rcClient, hdcPaint, DashStyleSolid, clr, 1.0);
                        }
                        else
                        {
                            DTTOPTS DttOpts = {sizeof(DTTOPTS)};
                            DttOpts.dwFlags = DTT_COMPOSITED | DTT_GLOWSIZE;
                            DttOpts.crText   = RGB(255, 255, 255);
                            DttOpts.iGlowSize = 12; // Default value

                            m_theme.DetermineGlowSize(&DttOpts.iGlowSize);

                            HFONT hFontOld = (HFONT)SendMessage(hWnd, WM_GETFONT, 0L, 0L);
                            if (hFontOld)
                                hFontOld = (HFONT) SelectObject(hdcPaint, hFontOld);
                            int iLen = GetWindowTextLength(hWnd);

                            if (iLen)
                            {
                                iLen+=5; // 1 for terminating zero, 4 for DT_MODIFYSTRING
                                LPWSTR szText = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*iLen);
                                if (szText)
                                {
                                    iLen = GetWindowTextW(hWnd, szText, iLen);
                                    if (iLen)
                                    {
                                        DWORD dwFlags = DT_WORDBREAK;

                                        switch (dwStaticStyle)
                                        {
                                        case SS_CENTER:
                                            dwFlags |= DT_CENTER;
                                            break;
                                        case SS_RIGHT:
                                            dwFlags |= DT_RIGHT;
                                            break;
                                        case SS_LEFTNOWORDWRAP:
                                            dwFlags &= ~DT_WORDBREAK;
                                            break;

                                        }

                                        if (dwStyle & SS_CENTERIMAGE)
                                        {
                                            dwFlags |= DT_VCENTER;
                                            dwFlags &= ~DT_WORDBREAK;
                                        }


                                        if (dwStyle & SS_ENDELLIPSIS)
                                            dwFlags |= DT_END_ELLIPSIS|DT_MODIFYSTRING;
                                        else if (dwStyle & SS_PATHELLIPSIS)
                                            dwFlags |= DT_PATH_ELLIPSIS|DT_MODIFYSTRING;
                                        else if (dwStyle & SS_WORDELLIPSIS)
                                            dwFlags |= DT_WORD_ELLIPSIS|DT_MODIFYSTRING;


                                        if (dwStyleEx&WS_EX_RIGHT)
                                            dwFlags |= DT_RIGHT;

                                        if (dwStyle & SS_NOPREFIX)
                                            dwFlags |= DT_NOPREFIX;

                                        m_theme.DrawThemeTextEx(hTheme, hdcPaint, 0, 0,
                                            szText, -1, dwFlags, &rcClient, &DttOpts);

                                        if (dwStyle&SS_SUNKEN || dwStyle&WS_BORDER)
                                            DrawRect(&rcClient, hdcPaint, DashStyleSolid, Color(0xFF, 0,0,0), 1.0);
                                    }

                                    LocalFree(szText);
                                }
                            }

                            if (hFontOld)
                            {
                                SelectObject(hdcPaint, hFontOld);
                                hFontOld    = nullptr;
                            }
                        }

                        m_theme.EndBufferedPaint(hBufferedPaint, TRUE);
                    }

                    m_theme.CloseThemeData(hTheme);
                }
            }

            EndPaint(hWnd, &ps);
            return 1;
        }
        break;
    case WM_NCDESTROY:
    case WM_DESTROY:
        RemoveWindowSubclass(hWnd, SubclassProc, Static);
        subclassedControls.erase(hWnd);
        break;
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

LRESULT AeroControlBase::ButtonWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_SETTEXT:
    case WM_ENABLE:
    case WM_STYLECHANGED:
        {
            LRESULT res = DefSubclassProc(hWnd, uMsg, wParam, lParam);
            InvalidateRgn(hWnd, nullptr, FALSE);
            return res;
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            if (hdc)
            {
                LONG_PTR dwStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
                LONG_PTR dwButtonStyle = LOWORD(dwStyle);
                LONG_PTR dwButtonType = dwButtonStyle&0xF;
                RECT rcClient;
                GetClientRect(hWnd, &rcClient);

                if((dwButtonType&BS_GROUPBOX) == BS_GROUPBOX)
                {
                    ///
                    /// it must be a group box
                    ///
                    HTHEME hTheme = m_theme.OpenThemeData(hWnd, L"Button");
                    if (hTheme)
                    {
                        HDC hdcPaint = nullptr;
                        BP_PAINTPARAMS params = { sizeof(BP_PAINTPARAMS) };
                        params.dwFlags        = BPPF_ERASE;

                        RECT rcExclusion = rcClient;
                        params.prcExclude = &rcExclusion;

                        ///
                        /// We have to calculate the exclusion rect and therefore
                        /// calculate the font height. We select the control's font
                        /// into the DC and fake a drawing operation:
                        ///
                        HFONT hFontOld = (HFONT)SendMessage(hWnd, WM_GETFONT, 0L, 0L);
                        if (hFontOld)
                            hFontOld = (HFONT) SelectObject(hdc, hFontOld);

                        RECT rcDraw = rcClient;
                        DWORD dwFlags = DT_SINGLELINE;

                        ///
                        /// we use uppercase A to determine the height of text, so we
                        /// can draw the upper line of the groupbox:
                        ///
                        DrawTextW(hdc, L"A", -1,  &rcDraw, dwFlags|DT_CALCRECT);

                        if (hFontOld)
                        {
                            SelectObject(hdc, hFontOld);
                            hFontOld    = nullptr;
                        }



                        InflateRect(&rcExclusion, -1, -1*RECTHEIGHT(rcDraw));

                        HPAINTBUFFER hBufferedPaint = m_theme.BeginBufferedPaint(hdc, &rcClient, BPBF_TOPDOWNDIB,
                            &params, &hdcPaint);
                        if (hdcPaint)
                        {
                            ///
                            /// now we again retrieve the font, but this time we select it into
                            /// the buffered DC:
                            ///
                            hFontOld = (HFONT)SendMessage(hWnd, WM_GETFONT, 0L, 0L);
                            if (hFontOld)
                                hFontOld = (HFONT) SelectObject(hdcPaint, hFontOld);


                            PatBlt(hdcPaint, 0, 0, RECTWIDTH(rcClient), RECTHEIGHT(rcClient), BLACKNESS);

                            m_theme.BufferedPaintSetAlpha(hBufferedPaint, &ps.rcPaint, 0x00);

                            int iPartId = BP_GROUPBOX;
                            int iState = GetStateFromBtnState(dwStyle, FALSE, FALSE, 0L, iPartId, FALSE);

                            DTTOPTS DttOpts = {sizeof(DTTOPTS)};
                            DttOpts.dwFlags = DTT_COMPOSITED | DTT_GLOWSIZE;
                            DttOpts.crText   = RGB(255, 255, 255);
                            DttOpts.iGlowSize = 12; // Default value

                            m_theme.DetermineGlowSize(&DttOpts.iGlowSize);

                            COLORREF cr = RGB(0x00, 0x00, 0x00);
                            GetEditBorderColor(hWnd, &cr);

                            ///
                            /// add the alpha value:
                            ///
                            cr |= 0xff000000;

                            std::unique_ptr<Pen> myPen( new Pen(Color(cr), 1) );
                            std::unique_ptr<Graphics> myGraphics( new Graphics(hdcPaint) );
                            int iY = RECTHEIGHT(rcDraw)/2;
                            Rect rr = Rect(rcClient.left, rcClient.top+iY,
                                RECTWIDTH(rcClient), RECTHEIGHT(rcClient)-iY-1);
                            GraphicsPath path;
                            GetRoundRectPath(&path, rr, 10);
                            myGraphics->DrawPath(myPen.get(), &path);
                            //myGraphics->DrawRectangle(myPen, rcClient.left, rcClient.top + iY,
                            //  RECTWIDTH(rcClient)-1, RECTHEIGHT(rcClient) - iY-1);
                            myGraphics.reset();
                            myPen.reset();

                            int iLen = GetWindowTextLength(hWnd);

                            if (iLen)
                            {
                                iLen+=5; // 1 for terminating zero, 4 for DT_MODIFYSTRING
                                LPWSTR szText = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*iLen);
                                if (szText)
                                {
                                    iLen = GetWindowTextW(hWnd, szText, iLen);
                                    if (iLen)
                                    {
                                        int iX = RECTWIDTH(rcDraw);
                                        rcDraw = rcClient;
                                        rcDraw.left += iX;
                                        DrawTextW(hdcPaint, szText, -1,  &rcDraw, dwFlags|DT_CALCRECT);
                                        PatBlt(hdcPaint, rcDraw.left, rcDraw.top , RECTWIDTH(rcDraw) + 3, RECTHEIGHT(rcDraw), BLACKNESS);
                                        rcDraw.left++;
                                        rcDraw.right++;
                                        m_theme.DrawThemeTextEx(hTheme, hdcPaint, iPartId, iState, szText, -1,
                                            dwFlags, &rcDraw, &DttOpts);

                                    }

                                    LocalFree(szText);
                                }
                            }

                            if (hFontOld)
                            {
                                SelectObject(hdcPaint, hFontOld);
                                hFontOld    = nullptr;
                            }

                            m_theme.EndBufferedPaint(hBufferedPaint, TRUE);

                        }

                        m_theme.CloseThemeData(hTheme);
                    }
                }

                else if (dwButtonType == BS_CHECKBOX || dwButtonType == BS_AUTOCHECKBOX ||
                    dwButtonType == BS_3STATE || dwButtonType == BS_AUTO3STATE || dwButtonType == BS_RADIOBUTTON || dwButtonType == BS_AUTORADIOBUTTON)
                {
                    HTHEME hTheme = m_theme.OpenThemeData(hWnd, L"Button");
                    if (hTheme)
                    {
                        HDC hdcPaint = nullptr;
                        BP_PAINTPARAMS params = { sizeof(BP_PAINTPARAMS) };
                        params.dwFlags        = BPPF_ERASE;
                        HPAINTBUFFER hBufferedPaint = m_theme.BeginBufferedPaint(hdc, &rcClient, BPBF_TOPDOWNDIB, &params, &hdcPaint);
                        if (hdcPaint)
                        {
                            PatBlt(hdcPaint, 0, 0, RECTWIDTH(rcClient), RECTHEIGHT(rcClient), BLACKNESS);

                            m_theme.BufferedPaintSetAlpha(hBufferedPaint, &ps.rcPaint, 0x00);
                            int iState = CBS_UNCHECKEDNORMAL;

                            LRESULT dwCheckState = SendMessage(hWnd, BM_GETCHECK, 0, 0L);
                            POINT pt;
                            RECT rc;
                            GetWindowRect(hWnd, &rc);
                            GetCursorPos(&pt);
                            BOOL bHot = PtInRect(&rc, pt);
                            BOOL bFocus = GetFocus() == hWnd;
                            int iPartId = BP_CHECKBOX;



                            if (dwButtonType == BS_RADIOBUTTON || dwButtonType == BS_AUTORADIOBUTTON)
                                iPartId = BP_RADIOBUTTON;

                            iState = GetStateFromBtnState(dwStyle, bHot, bFocus, dwCheckState, iPartId, FALSE);

                            int bmWidth = int(ceil(13.0 * GetDeviceCaps(hdcPaint, LOGPIXELSX) / USER_DEFAULT_SCREEN_DPI));

                            UINT uiHalfWidth = (RECTWIDTH(rcClient) - bmWidth)/2;

                            ///
                            /// we have to use the whole client area, otherwise we get only partially
                            /// drawn areas:
                            ///
                            RECT rcPaint = rcClient;

                            if (dwButtonStyle & BS_LEFTTEXT)
                            {
                                rcPaint.left += uiHalfWidth;
                                rcPaint.right += uiHalfWidth;
                            }
                            else
                            {
                                rcPaint.left -= uiHalfWidth;
                                rcPaint.right -= uiHalfWidth;
                            }


                            ///
                            /// we assume that bm.bmWidth is both the horizontal and the vertical
                            /// dimension of the control bitmap and that it is square. bm.bmHeight
                            /// seems to be the height of a striped bitmap because it is an absurdly
                            /// high dimension value
                            ///
                            if((dwButtonStyle&BS_VCENTER) == BS_VCENTER) /// BS_VCENTER is BS_TOP|BS_BOTTOM
                            {
                                int h = RECTHEIGHT(rcPaint);
                                rcPaint.top = (h - bmWidth) / 2;
                                rcPaint.bottom = rcPaint.top + bmWidth;
                            }
                            else if (dwButtonStyle&BS_TOP)
                            {
                                rcPaint.bottom = rcPaint.top + bmWidth;
                            }
                            else if (dwButtonStyle&BS_BOTTOM)
                            {
                                rcPaint.top =  rcPaint.bottom - bmWidth;
                            }
                            else // default: center the checkbox/radiobutton vertically
                            {
                                int h = RECTHEIGHT(rcPaint);
                                rcPaint.top = (h - bmWidth) / 2;
                                rcPaint.bottom = rcPaint.top + bmWidth;
                            }


                            m_theme.DrawThemeBackground(hTheme, hdcPaint, iPartId, iState, &rcPaint, nullptr);
                            rcPaint = rcClient;


                            m_theme.GetThemeBackgroundContentRect(hTheme, hdcPaint, iPartId, iState, &rcPaint, &rc);

                            if(dwButtonStyle & BS_LEFTTEXT)
                                rc.right -= bmWidth + 2 * GetSystemMetrics(SM_CXEDGE);
                            else
                                rc.left += bmWidth + 2 * GetSystemMetrics(SM_CXEDGE);

                            DTTOPTS DttOpts = {sizeof(DTTOPTS)};
                            DttOpts.dwFlags = DTT_COMPOSITED | DTT_GLOWSIZE;
                            DttOpts.crText   = RGB(255, 255, 255);
                            DttOpts.iGlowSize = 12; // Default value

                            m_theme.DetermineGlowSize(&DttOpts.iGlowSize);


                            HFONT hFontOld = (HFONT)SendMessage(hWnd, WM_GETFONT, 0L, 0L);
                            if (hFontOld)
                                hFontOld = (HFONT) SelectObject(hdcPaint, hFontOld);
                            int iLen = GetWindowTextLength(hWnd);

                            if (iLen)
                            {
                                iLen+=5; // 1 for terminating zero, 4 for DT_MODIFYSTRING
                                LPWSTR szText = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*iLen);
                                if (szText)
                                {
                                    iLen = GetWindowTextW(hWnd, szText, iLen);
                                    if (iLen)
                                    {
                                        DWORD dwFlags = DT_SINGLELINE /*|DT_VCENTER*/;
                                        if (dwButtonStyle&BS_MULTILINE)
                                        {
                                            dwFlags|=DT_WORDBREAK;
                                            dwFlags&= ~(DT_SINGLELINE |DT_VCENTER);
                                        }

                                        if((dwButtonStyle&BS_CENTER) == BS_CENTER) /// BS_CENTER is BS_LEFT|BS_RIGHT
                                            dwFlags|=DT_CENTER;
                                        else if (dwButtonStyle&BS_LEFT)
                                            dwFlags|=DT_LEFT;
                                        else if (dwButtonStyle&BS_RIGHT)
                                            dwFlags|=DT_RIGHT;


                                        if((dwButtonStyle&BS_VCENTER) == BS_VCENTER) /// BS_VCENTER is BS_TOP|BS_BOTTOM
                                            dwFlags|=DT_VCENTER;
                                        else if (dwButtonStyle&BS_TOP)
                                            dwFlags|=DT_TOP;
                                        else if (dwButtonStyle&BS_BOTTOM)
                                            dwFlags|=DT_BOTTOM;
                                        else
                                            dwFlags|=DT_VCENTER;

                                        m_theme.DrawThemeTextEx(hTheme, hdcPaint, iPartId,
                                            iState, szText, -1, dwFlags, &rc, &DttOpts);

                                        ///
                                        /// if our control has the focus, we also have to draw the focus rectangle:
                                        ///
                                        if (bFocus)
                                        {
                                            ///
                                            /// now calculate the text size:
                                            ///
                                            RECT rcDraw = rc;

                                            ///
                                            /// we use GDI's good old DrawText, because it returns much more
                                            /// accurate data than DrawThemeTextEx, which takes the glow
                                            /// into account which we don't want:
                                            ///
                                            DrawTextW(hdcPaint, szText, -1,  &rcDraw, dwFlags|DT_CALCRECT);
                                            if (dwFlags&DT_SINGLELINE)
                                            {
                                                dwFlags &= ~DT_VCENTER;
                                                RECT rcDrawTop;
                                                DrawTextW(hdcPaint, szText, -1,  &rcDrawTop, dwFlags|DT_CALCRECT);
                                                rcDraw.top = rcDraw.bottom - RECTHEIGHT(rcDrawTop);
                                            }

                                            if (dwFlags & DT_RIGHT)
                                            {
                                                int iWidth = RECTWIDTH(rcDraw);
                                                rcDraw.right = rc.right;
                                                rcDraw.left = rcDraw.right - iWidth;
                                            }

                                            RECT rcFocus;
                                            IntersectRect(&rcFocus, &rc, &rcDraw);

                                            DrawFocusRect(&rcFocus, hdcPaint);
                                        }
                                    }

                                    LocalFree(szText);
                                }
                            }


                            if (hFontOld)
                            {
                                SelectObject(hdcPaint, hFontOld);
                                hFontOld    = nullptr;
                            }

                            m_theme.EndBufferedPaint(hBufferedPaint, TRUE);
                        }
                        m_theme.CloseThemeData(hTheme);
                    }

                }
                else if (BS_PUSHBUTTON == dwButtonType || BS_DEFPUSHBUTTON == dwButtonType)
                {
                    ///
                    /// it is a push button
                    ///
                    HTHEME hTheme = m_theme.OpenThemeData(hWnd, L"Button");
                    if (hTheme)
                    {
                        HDC hdcPaint = nullptr;
                        BP_PAINTPARAMS params = { sizeof(BP_PAINTPARAMS) };
                        params.dwFlags        = BPPF_ERASE;
                        HPAINTBUFFER hBufferedPaint = m_theme.BeginBufferedPaint(hdc, &rcClient, BPBF_TOPDOWNDIB, &params, &hdcPaint);
                        if (hdcPaint)
                        {
                            PatBlt(hdcPaint, 0, 0, RECTWIDTH(rcClient), RECTHEIGHT(rcClient), BLACKNESS);

                            m_theme.BufferedPaintSetAlpha(hBufferedPaint, &ps.rcPaint, 0x00);
                            int iState = CBS_UNCHECKEDNORMAL;

                            LRESULT dwCheckState = SendMessage(hWnd, BM_GETCHECK, 0, 0L);
                            POINT pt;
                            RECT rc;
                            GetWindowRect(hWnd, &rc);
                            GetCursorPos(&pt);
                            BOOL bHot = PtInRect(&rc, pt);
                            BOOL bFocus = GetFocus() == hWnd;
                            int iPartId = BP_PUSHBUTTON;

                            if (dwButtonStyle == BS_RADIOBUTTON || dwButtonStyle == BS_AUTORADIOBUTTON)
                                iPartId = BP_RADIOBUTTON;

                            iState = GetStateFromBtnState(dwStyle, bHot, bFocus, dwCheckState, iPartId, GetCapture() == hWnd);

                            ///
                            /// we have to use the whole client area, otherwise we get only partially
                            /// drawn areas:
                            ///
                            RECT rcPaint = rcClient;
                            m_theme.DrawThemeBackground(hTheme, hdcPaint, iPartId, iState, &rcPaint, nullptr);

                            m_theme.GetThemeBackgroundContentRect(hTheme, hdcPaint, iPartId, iState, &rcPaint, &rc);

                            DTTOPTS DttOpts   = {sizeof(DTTOPTS)};
                            DttOpts.dwFlags   = DTT_COMPOSITED;
                            DttOpts.crText    = RGB(255, 255, 255);
                            DttOpts.dwFlags  |= DTT_GLOWSIZE;
                            DttOpts.iGlowSize = 12; // Default value

                            m_theme.DetermineGlowSize(&DttOpts.iGlowSize);

                            HFONT hFontOld = (HFONT)SendMessage(hWnd, WM_GETFONT, 0L, 0L);
                            if (hFontOld)
                                hFontOld = (HFONT) SelectObject(hdcPaint, hFontOld);
                            int iLen = GetWindowTextLength(hWnd);

                            if (iLen)
                            {
                                iLen+=5; // 1 for terminating zero, 4 for DT_MODIFYSTRING
                                LPWSTR szText = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*iLen);
                                if (szText)
                                {
                                    iLen = GetWindowTextW(hWnd, szText, iLen);
                                    if (iLen)
                                    {
                                        DWORD dwFlags = DT_SINGLELINE | DT_CENTER | DT_VCENTER;
                                        m_theme.DrawThemeTextEx(hTheme, hdcPaint,
                                            iPartId, iState, szText, -1, dwFlags, &rc, &DttOpts);

                                        ///
                                        /// if our control has the focus, we also have to draw the focus rectangle:
                                        ///
                                        if (bFocus)
                                        {
                                            RECT rcDraw = rcClient;
                                            InflateRect(&rcDraw, -3, -3);
                                            DrawFocusRect(&rcDraw, hdcPaint);
                                        }

                                    }

                                    LocalFree(szText);
                                }
                            }

                            if (hFontOld)
                            {
                                SelectObject(hdcPaint, hFontOld);
                                hFontOld    = nullptr;
                            }

                            m_theme.EndBufferedPaint(hBufferedPaint, TRUE);
                        }
                        m_theme.CloseThemeData(hTheme);
                    }
                }
                else
                    //PaintControl(hWnd, hdc, &ps.rcPaint, (m_dwFlags & WD_DRAW_BORDER) != 0);
                    PaintControl(hWnd, hdc, &ps.rcPaint, false);
            }

            EndPaint(hWnd, &ps);
            return 0;
        }
        break;
    case WM_DESTROY:
    case WM_NCDESTROY:
        RemoveWindowSubclass(hWnd, SubclassProc, Button);
        subclassedControls.erase(hWnd);
        break;
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

LRESULT AeroControlBase::ProgressbarWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_ENABLE:
    case WM_STYLECHANGED:
        {
            LRESULT res = DefSubclassProc(hWnd, uMsg, wParam, lParam);
            InvalidateRgn(hWnd, nullptr, FALSE);
            return res;
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            RECT rc;
            GetWindowRect(hWnd, &rc);
            MapWindowPoints(nullptr, hWnd, (LPPOINT) &rc, 2);

            if (hdc)
            {
                PaintControl(hWnd, hdc, &rc, false);

                BP_PAINTPARAMS params = { sizeof(BP_PAINTPARAMS) };
                params.dwFlags        = 0L;
                HDC hdcPaint = nullptr;
                HPAINTBUFFER hBufferedPaint = m_theme.BeginBufferedPaint(hdc, &rc, BPBF_TOPDOWNDIB, &params, &hdcPaint);
                if (hdcPaint)
                {
                    COLORREF cr = RGB(0x00, 0x00, 0x00);
                    SetPixel(hdcPaint, 0, 0, cr);
                    SetPixel(hdcPaint, 0, RECTHEIGHT(rc) - 1, cr);
                    SetPixel(hdcPaint, RECTWIDTH(rc) - 1, 0, cr);
                    SetPixel(hdcPaint, RECTWIDTH(rc) - 1, RECTHEIGHT(rc) - 1, cr);

                    m_theme.EndBufferedPaint(hBufferedPaint, TRUE);
                }
            }

            EndPaint(hWnd, &ps);
            return 1;
        }
        break;
    case WM_NCDESTROY:
    case WM_DESTROY:
        RemoveWindowSubclass(hWnd, SubclassProc, Static);
        subclassedControls.erase(hWnd);
        break;
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

void AeroControlBase::FillRect(LPRECT prc, HDC hdcPaint, Color clr) const
{
    auto pBrush = std::make_unique<SolidBrush>(clr);

    if (pBrush)
    {
        auto myGraphics = std::make_unique<Graphics>(hdcPaint);
        if (myGraphics)
        {
            myGraphics->FillRectangle(pBrush.get(), prc->left, prc->top,
                prc->right - 1 - prc->left, prc->bottom - 1 - prc->top);
        }
    }
}

int AeroControlBase::GetStateFromBtnState(LONG_PTR dwStyle, BOOL bHot, BOOL bFocus, LRESULT dwCheckState, int iPartId, BOOL bHasMouseCapture) const
{
    int iState = 0;
    switch (iPartId)
    {
    case BP_PUSHBUTTON:
        iState = PBS_NORMAL;
        if (dwStyle&WS_DISABLED)
            iState = PBS_DISABLED;
        else
        {
            if (dwStyle&BS_DEFPUSHBUTTON)
                iState = PBS_DEFAULTED;

            if (bHasMouseCapture && bHot)
                iState = PBS_PRESSED;
            else if (bHasMouseCapture || bHot)
                iState = PBS_HOT;
        }
        break;
    case BP_GROUPBOX:
        iState = (dwStyle & WS_DISABLED)?GBS_DISABLED:GBS_NORMAL;
        break;

    case BP_RADIOBUTTON:
        iState = RBS_UNCHECKEDNORMAL;
        switch(dwCheckState)
        {
        case BST_CHECKED:
            if (dwStyle&WS_DISABLED)
                iState = RBS_CHECKEDDISABLED;
            else if (bFocus)
                iState = RBS_CHECKEDPRESSED;
            else if (bHot)
                iState = RBS_CHECKEDHOT;
            else
                iState = RBS_CHECKEDNORMAL;
            break;
        case BST_UNCHECKED:
            if (dwStyle&WS_DISABLED)
                iState = RBS_UNCHECKEDDISABLED;
            else if (bFocus)
                iState = RBS_UNCHECKEDPRESSED;
            else if (bHot)
                iState = RBS_UNCHECKEDHOT;
            else
                iState = RBS_UNCHECKEDNORMAL;
            break;
        }
        break;

    case BP_CHECKBOX:
        switch(dwCheckState)
        {
        case BST_CHECKED:
            if (dwStyle&WS_DISABLED)
                iState = CBS_CHECKEDDISABLED;
            else if (bFocus)
                iState = CBS_CHECKEDPRESSED;
            else if (bHot)
                iState = CBS_CHECKEDHOT;
            else
                iState = CBS_CHECKEDNORMAL;
            break;
        case BST_INDETERMINATE:
            if (dwStyle&WS_DISABLED)
                iState = CBS_MIXEDDISABLED;
            else if (bFocus)
                iState = CBS_MIXEDPRESSED;
            else if (bHot)
                iState = CBS_MIXEDHOT;
            else
                iState = CBS_MIXEDNORMAL;
            break;
        case BST_UNCHECKED:
            if (dwStyle&WS_DISABLED)
                iState = CBS_UNCHECKEDDISABLED;
            else if (bFocus)
                iState = CBS_UNCHECKEDPRESSED;
            else if (bHot)
                iState = CBS_UNCHECKEDHOT;
            else
                iState = CBS_UNCHECKEDNORMAL;
            break;
        }
        break;
    default:
        break;
    }

    return iState;
}

void AeroControlBase::DrawRect(LPRECT prc, HDC hdcPaint, DashStyle dashStyle, Color clr, REAL width) const
{
    auto myPen = std::make_unique<Pen>(clr, width);
    myPen->SetDashStyle(dashStyle);
    auto myGraphics = std::make_unique<Graphics>(hdcPaint);

    myGraphics->DrawRectangle(myPen.get(), prc->left, prc->top,
        prc->right - 1 - prc->left, prc->bottom - 1 - prc->top);
}

void AeroControlBase::DrawFocusRect(LPRECT prcFocus, HDC hdcPaint)
{
    DrawRect(prcFocus, hdcPaint, DashStyleDot, Color(0xFF, 0,0,0), 1.0);
};

void AeroControlBase::PaintControl(HWND hWnd, HDC hdc, RECT* prc, bool bDrawBorder)
{
    HDC hdcPaint = nullptr;

    if (bDrawBorder)
        InflateRect(prc, 1, 1);
    HPAINTBUFFER hBufferedPaint = m_theme.BeginBufferedPaint(hdc, prc, BPBF_TOPDOWNDIB, nullptr, &hdcPaint);
    if (hdcPaint)
    {
        RECT rc;
        GetWindowRect(hWnd, &rc);

        PatBlt(hdcPaint, 0, 0, RECTWIDTH(rc), RECTHEIGHT(rc), BLACKNESS);
        m_theme.BufferedPaintSetAlpha(hBufferedPaint, &rc, 0x00);

        ///
        /// first blit white so list ctrls don't look ugly:
        ///
        PatBlt(hdcPaint, 0, 0, RECTWIDTH(rc), RECTHEIGHT(rc), WHITENESS);

        if (bDrawBorder)
            InflateRect(prc, -1, -1);
        // Tell the control to paint itself in our memory buffer
        SendMessage(hWnd, WM_PRINTCLIENT, (WPARAM) hdcPaint, PRF_CLIENT|PRF_ERASEBKGND |PRF_NONCLIENT|PRF_CHECKVISIBLE);

        if (bDrawBorder)
        {
            InflateRect(prc, 1, 1);
            FrameRect(hdcPaint, prc, (HBRUSH)GetStockObject(BLACK_BRUSH));
        }

        // Make every pixel opaque
        m_theme.BufferedPaintMakeOpaque_(hBufferedPaint, prc);
        m_theme.EndBufferedPaint(hBufferedPaint, TRUE);
    }
}

BOOL AeroControlBase::GetEditBorderColor(HWND hWnd, COLORREF *pClr)
{
    HTHEME hTheme = m_theme.OpenThemeData(hWnd, L"Edit");
    if (hTheme)
    {
        m_theme.GetThemeColor(hTheme, EP_BACKGROUNDWITHBORDER, EBWBS_NORMAL, TMT_BORDERCOLOR, pClr);
        m_theme.CloseThemeData(hTheme);
        return TRUE;
    }

    return FALSE;
}

void AeroControlBase::DrawEditBorder(HWND hWnd)
{
    LONG_PTR dwStyleEx = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
    if(!(dwStyleEx&WS_EX_CLIENTEDGE))
        return;

    COLORREF cr = RGB(0x00, 0x00, 0x00);
    GetEditBorderColor(hWnd, &cr);

    Color clr;
    clr.SetFromCOLORREF(cr);
    DrawSolidWndRectOnParent(hWnd, clr );
}

void AeroControlBase::DrawSolidWndRectOnParent(HWND hWnd, Color clr)
{
    RECT rcWnd;
    GetWindowRect(hWnd, &rcWnd);
    HWND hParent = GetParent(hWnd);
    if (hParent)
    {
        ScreenToClient(hParent, &rcWnd);

        HDC hdc = GetDC(hParent);
        if (hdc)
        {
            DrawRect(&rcWnd, hdc, DashStyleSolid, clr, 1.0);
            ReleaseDC(hWnd, hdc);
        }
    }
}

void AeroControlBase::ScreenToClient(HWND hWnd, LPRECT lprc)
{
    POINT pt;
    pt.x = lprc->left;
    pt.y = lprc->top;
    ::ScreenToClient(hWnd, &pt);
    lprc->left = pt.x;
    lprc->top = pt.y;

    pt.x = lprc->right;
    pt.y = lprc->bottom;
    ::ScreenToClient(hWnd, &pt);
    lprc->right = pt.x;
    lprc->bottom = pt.y;
}

void AeroControlBase::GetRoundRectPath(GraphicsPath *pPath, const Rect& r, int dia) const
{
    // diameter can't exceed width or height
    if(dia > r.Width)   dia = r.Width;
    if(dia > r.Height)  dia = r.Height;

    // define a corner
    Rect Corner(r.X, r.Y, dia, dia);

    // begin path
    pPath->Reset();
    pPath->StartFigure();

    // top left
    pPath->AddArc(Corner, 180, 90);

    // top right
    Corner.X += (r.Width - dia - 1);
    pPath->AddArc(Corner, 270, 90);

    // bottom right
    Corner.Y += (r.Height - dia - 1);
    pPath->AddArc(Corner,   0, 90);

    // bottom left
    Corner.X -= (r.Width - dia - 1);
    pPath->AddArc(Corner,  90, 90);

    // end path
    pPath->CloseFigure();
}

