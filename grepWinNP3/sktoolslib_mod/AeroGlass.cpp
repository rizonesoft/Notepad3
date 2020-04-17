// sktoolslib - common files for SK tools

// Copyright (C) 2012, 2017 - Stefan Kueng

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
#include "AeroGlass.h"

#include <VersionHelpers.h>
// SDKs prior to Win10 don't have the IsWindows10OrGreater API in the versionhelpers header, so
// we define it here just in case:
#ifndef _WIN32_WINNT_WIN10
#define _WIN32_WINNT_WIN10 0x0A00
#define _WIN32_WINNT_WINTHRESHOLD 0x0A00
#define  IsWindows10OrGreater() (IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WIN10), LOBYTE(_WIN32_WINNT_WIN10), 0))
#endif


typedef HRESULT (__stdcall *DWM_EXTEND_FRAME_INTO_CLIENT_AREA)(HWND ,const MARGINS* );
typedef HRESULT (__stdcall *DWM_IS_COMPOSITION_ENABLED)(BOOL *pfEnabled);
typedef HRESULT (__stdcall *DWM_ENABLE_COMPOSITION)(UINT uCompositionAction);

CDwmApiImpl::CDwmApiImpl(void)
    : m_hDwmApiLib(nullptr)
{
}

BOOL CDwmApiImpl::Initialize(void)
{
    if (m_hDwmApiLib)
    {
        SetLastError(ERROR_ALREADY_INITIALIZED);
        return FALSE;
    }

    m_hDwmApiLib = LoadLibraryW(L"dwmapi.dll");
    return IsInitialized();
}

BOOL CDwmApiImpl::IsInitialized(void)
{
    return (nullptr != m_hDwmApiLib);
}

CDwmApiImpl::~CDwmApiImpl(void)
{
    if (IsInitialized())
    {
        FreeLibrary(m_hDwmApiLib);
        m_hDwmApiLib = nullptr;
    }
}

HRESULT CDwmApiImpl::DwmExtendFrameIntoClientArea(HWND hWnd,const MARGINS* pMarInset)
{
    if(!IsInitialized())
    {
        return OLE_E_BLANK;
    }
    if (!IsDwmCompositionEnabled())
        return OLE_E_BLANK;
    DWM_EXTEND_FRAME_INTO_CLIENT_AREA pfnDwmExtendFrameIntoClientArea = (DWM_EXTEND_FRAME_INTO_CLIENT_AREA)GetProcAddress(m_hDwmApiLib, "DwmExtendFrameIntoClientArea");
    if(!pfnDwmExtendFrameIntoClientArea)
        return HRESULT_FROM_WIN32(GetLastError());

    return pfnDwmExtendFrameIntoClientArea(hWnd, pMarInset);
}

BOOL CDwmApiImpl::IsDwmCompositionEnabled(void)
{
    if(!IsInitialized())
    {
        SetLastError((DWORD)OLE_E_BLANK);
        return FALSE;
    }
    // disable aero dialogs in high contrast mode and on Windows 10:
    // in high contrast mode, while DWM is still active, the aero effect is not
    // in Win 10, the dialog title bar is not rendered transparent, so the dialogs would
    // look really ugly and not symmetric.
    HIGHCONTRAST hc = { sizeof(HIGHCONTRAST) };
    SystemParametersInfo(SPI_GETHIGHCONTRAST, sizeof(HIGHCONTRAST), &hc, FALSE);
    DWM_IS_COMPOSITION_ENABLED pfnDwmIsCompositionEnabled = (DWM_IS_COMPOSITION_ENABLED)GetProcAddress(m_hDwmApiLib, "DwmIsCompositionEnabled");
    if(!pfnDwmIsCompositionEnabled)
        return FALSE;
    BOOL bEnabled = FALSE;
    HRESULT hRes = pfnDwmIsCompositionEnabled(&bEnabled);
    return SUCCEEDED(hRes) && bEnabled && ((hc.dwFlags & HCF_HIGHCONTRASTON) == 0) && !IsWindows10OrGreater();
}

HRESULT CDwmApiImpl::DwmEnableComposition(UINT uCompositionAction)
{
    if(!IsInitialized())
    {
        SetLastError((DWORD)OLE_E_BLANK);
        return FALSE;
    }
    DWM_ENABLE_COMPOSITION pfnDwmEnableComposition = (DWM_ENABLE_COMPOSITION)GetProcAddress(m_hDwmApiLib, "DwmEnableComposition");
    if(!pfnDwmEnableComposition)
        return HRESULT_FROM_WIN32(GetLastError());

    return pfnDwmEnableComposition(uCompositionAction);
}

CUxThemeAeroImpl::CUxThemeAeroImpl(void)
    : m_hUxThemeLib(nullptr)
{
}

BOOL CUxThemeAeroImpl::Initialize(void)
{
    if (m_hUxThemeLib)
    {
        SetLastError(ERROR_ALREADY_INITIALIZED);
        return FALSE;
    }

    m_hUxThemeLib = LoadLibraryW(L"uxtheme.dll");
    return IsInitialized();
}

typedef HRESULT (__stdcall *BUFFERED_PAINT_INIT)(VOID);
typedef HTHEME (__stdcall *OPEN_THEME_DATA)(HWND hwnd, LPCWSTR pszClassList);
typedef HRESULT (__stdcall *CLOSE_THEME_DATA)(HTHEME hTheme);
typedef HPAINTBUFFER (__stdcall *BEGIN_BUFFERED_PAINT)(HDC hdcTarget, const RECT* prcTarget, BP_BUFFERFORMAT dwFormat, BP_PAINTPARAMS *pPaintParams,  HDC *phdc);
typedef HRESULT (__stdcall *END_BUFFERED_PAINT)(HPAINTBUFFER hBufferedPaint, BOOL fUpdateTarget);
typedef HRESULT (__stdcall *DRAW_THEME_TEXT_EX)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int cchText, DWORD dwTextFlags, LPRECT pRect,  const DTTOPTS *pOptions);
typedef HRESULT (__stdcall *GET_THEME_INT)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, int *piVal);
typedef HRESULT (__stdcall *GET_THEME_SYS_FONT)(HTHEME hTheme, int iFontId, LOGFONTW *plf);
typedef HRESULT (__stdcall *BUFFERED_PAINT_SET_ALPHA)(HPAINTBUFFER hBufferedPaint, const RECT *prc, BYTE alpha);
typedef HRESULT (__stdcall *DRAW_THEME_BACKGROUND)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect);
typedef HRESULT (__stdcall *GET_THEME_BKG_CONTENT_RECT)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pBoundingRect, LPRECT pContentRect);
typedef HRESULT (__stdcall *GET_THEME_BKG_CONTENT_EXTENT)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pContentRect, LPRECT pExtentRect);
typedef HRESULT (__stdcall *GET_THEME_BITMAP)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, ULONG dwFlags, HBITMAP *phBitmap);
typedef HRESULT (__stdcall *DRAW_THEME_PARENT_BACKGROUND)(HWND hwnd, HDC hdc, const RECT *prc);
typedef BOOL (__stdcall *IS_THEME_BACKGROUND_PARTIALLY_TRANSPARENT)(HTHEME hTheme,int iPartId, int iStateId);
typedef HRESULT (__stdcall *DRAW_THEME_TEXT)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, LPCRECT pRect);
typedef HRESULT (__stdcall *GET_THEME_COLOR)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, COLORREF *pColor);
typedef HRESULT (__stdcall *GET_THEME_PART_SIZE)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT prc, THEMESIZE eSize,SIZE *psz);
typedef HRESULT (__stdcall *GET_THEME_POSITION)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, POINT *pPoint);
typedef HRESULT (__stdcall *GET_THEME_MARGINS)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, LPRECT prc, MARGINS *pMargins);
typedef HRESULT (__stdcall *GET_THEME_METRIC)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, int *piVal);
typedef HRESULT (__stdcall *GET_THEME_RECT)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, LPRECT pRect);

BOOL CUxThemeAeroImpl::IsInitialized(void)
{
    return (nullptr != m_hUxThemeLib);
}

CUxThemeAeroImpl::~CUxThemeAeroImpl(void)
{
    if (IsInitialized())
    {
        FreeLibrary(m_hUxThemeLib);
        m_hUxThemeLib = nullptr;
    }
}

HRESULT CUxThemeAeroImpl::BufferedPaintInit(void)
{
    if(!IsInitialized())
    {
        return OLE_E_BLANK;
    }
    BUFFERED_PAINT_INIT pfnBufferedPaintInit = (BUFFERED_PAINT_INIT)GetProcAddress(m_hUxThemeLib, "BufferedPaintInit");
    if(!pfnBufferedPaintInit)
        return HRESULT_FROM_WIN32(GetLastError());

    return pfnBufferedPaintInit();
}

HRESULT CUxThemeAeroImpl::BufferedPaintUnInit(void)
{
    if(!IsInitialized())
    {
        return OLE_E_BLANK;
    }
    BUFFERED_PAINT_INIT pfnBufferedPaintUnInit = (BUFFERED_PAINT_INIT)GetProcAddress(m_hUxThemeLib, "BufferedPaintUnInit");
    if(!pfnBufferedPaintUnInit)
        return HRESULT_FROM_WIN32(GetLastError());

    return pfnBufferedPaintUnInit();
}

HTHEME CUxThemeAeroImpl::OpenThemeData(HWND hwnd, LPCWSTR pszClassList)
{
    if(!IsInitialized())
    {
        SetLastError((DWORD)OLE_E_BLANK);
        return nullptr;
    }
    OPEN_THEME_DATA pfnOpenThemeData = (OPEN_THEME_DATA)GetProcAddress(m_hUxThemeLib, "OpenThemeData");
    if(!pfnOpenThemeData)
        return nullptr;

    return pfnOpenThemeData(hwnd, pszClassList);
}

HRESULT CUxThemeAeroImpl::CloseThemeData(HTHEME hTheme)
{
    if(!IsInitialized())
    {
        return OLE_E_BLANK;
    }
    CLOSE_THEME_DATA pfnCloseThemeData = (CLOSE_THEME_DATA)GetProcAddress(m_hUxThemeLib, "CloseThemeData");
    if(!pfnCloseThemeData)
        return HRESULT_FROM_WIN32(GetLastError());

    return pfnCloseThemeData(hTheme);
}

HANDLE CUxThemeAeroImpl::BeginBufferedPaint(HDC hdcTarget, const RECT* prcTarget, BP_BUFFERFORMAT dwFormat, BP_PAINTPARAMS *pPaintParams, HDC *phdc)
{
    if(!IsInitialized())
    {
        SetLastError((DWORD)OLE_E_BLANK);
        return nullptr;
    }
    BEGIN_BUFFERED_PAINT pfnBeginBufferedPaint = (BEGIN_BUFFERED_PAINT)GetProcAddress(m_hUxThemeLib, "BeginBufferedPaint");
    if(!pfnBeginBufferedPaint)
        return nullptr;

    return pfnBeginBufferedPaint(hdcTarget, prcTarget, dwFormat, pPaintParams, phdc);
}

HRESULT CUxThemeAeroImpl::EndBufferedPaint(HANDLE hBufferedPaint, BOOL fUpdateTarget)
{
    if(!IsInitialized())
    {
        return OLE_E_BLANK;
    }
    END_BUFFERED_PAINT pfnEndBufferedPaint = (END_BUFFERED_PAINT)GetProcAddress(m_hUxThemeLib, "EndBufferedPaint");
    if(!pfnEndBufferedPaint)
        return HRESULT_FROM_WIN32(GetLastError());

    return pfnEndBufferedPaint(hBufferedPaint, fUpdateTarget);
}

HRESULT CUxThemeAeroImpl::DrawThemeTextEx(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int cchText, DWORD dwTextFlags, LPRECT pRect, const DTTOPTS *pOptions)
{
    if(!IsInitialized())
    {
        return OLE_E_BLANK;
    }
    DRAW_THEME_TEXT_EX pfnDrawThemeTextEx = (DRAW_THEME_TEXT_EX)GetProcAddress(m_hUxThemeLib, "DrawThemeTextEx");
    if(!pfnDrawThemeTextEx)
        return HRESULT_FROM_WIN32(GetLastError());

    return pfnDrawThemeTextEx(hTheme, hdc, iPartId, iStateId, pszText, cchText, dwTextFlags, pRect, pOptions);
}

HRESULT CUxThemeAeroImpl::GetThemeInt(HTHEME hTheme, int iPartId, int iStateId, int iPropId, int *piVal)
{
    if(!IsInitialized())
    {
        return OLE_E_BLANK;
    }
    GET_THEME_INT pfnGetThemeInt = (GET_THEME_INT)GetProcAddress(m_hUxThemeLib, "GetThemeInt");
    if(!pfnGetThemeInt)
        return HRESULT_FROM_WIN32(GetLastError());

    return pfnGetThemeInt(hTheme, iPartId, iStateId, iPropId, piVal);
}

HRESULT CUxThemeAeroImpl::GetThemeSysFont(HTHEME hTheme, int iFontId, LOGFONTW *plf)
{
    if(!IsInitialized())
    {
        return OLE_E_BLANK;
    }
    GET_THEME_SYS_FONT pfnGetThemeSysFont = (GET_THEME_SYS_FONT)GetProcAddress(m_hUxThemeLib, "GetThemeSysFont");
    if(!pfnGetThemeSysFont)
        return HRESULT_FROM_WIN32(GetLastError());

    return pfnGetThemeSysFont(hTheme, iFontId, plf);
}

HRESULT CUxThemeAeroImpl::BufferedPaintSetAlpha(HPAINTBUFFER hBufferedPaint, const RECT *prc, BYTE alpha)
{
    if(!IsInitialized())
    {
        return OLE_E_BLANK;
    }

    BUFFERED_PAINT_SET_ALPHA pfnBufferedPaintSetAlpha = (BUFFERED_PAINT_SET_ALPHA)GetProcAddress(m_hUxThemeLib, "BufferedPaintSetAlpha");
    if(!pfnBufferedPaintSetAlpha)
        return HRESULT_FROM_WIN32(GetLastError());

    return pfnBufferedPaintSetAlpha(hBufferedPaint, prc, alpha);
}

HRESULT CUxThemeAeroImpl::BufferedPaintMakeOpaque_(HPAINTBUFFER hBufferedPaint, const RECT *prc)
{
    return BufferedPaintSetAlpha(hBufferedPaint, prc, 255);
}

HRESULT CUxThemeAeroImpl::DrawThemeBackground(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect)
{
    if(!IsInitialized())
    {
        return OLE_E_BLANK;
    }

    DRAW_THEME_BACKGROUND pfnDrawThemeBackground = (DRAW_THEME_BACKGROUND)GetProcAddress(m_hUxThemeLib, "DrawThemeBackground");
    if(!pfnDrawThemeBackground)
        return HRESULT_FROM_WIN32(GetLastError());

    return pfnDrawThemeBackground(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
}

HRESULT CUxThemeAeroImpl::GetThemeBackgroundContentRect(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pBoundingRect, LPRECT pContentRect)
{
    if(!IsInitialized())
    {
        return OLE_E_BLANK;
    }

    GET_THEME_BKG_CONTENT_RECT pfnGetThemeBackgroundContentRect = (GET_THEME_BKG_CONTENT_RECT)GetProcAddress(m_hUxThemeLib, "GetThemeBackgroundContentRect");
    if(!pfnGetThemeBackgroundContentRect)
        return HRESULT_FROM_WIN32(GetLastError());

    return pfnGetThemeBackgroundContentRect(hTheme, hdc, iPartId, iStateId, pBoundingRect, pContentRect);
}

HRESULT CUxThemeAeroImpl::GetThemeBackgroundExtent(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pContentRect, LPRECT pExtentRect)
{
    if(!IsInitialized())
    {
        return OLE_E_BLANK;
    }

    GET_THEME_BKG_CONTENT_EXTENT pfnGetThemeBackgroundExtent = (GET_THEME_BKG_CONTENT_EXTENT)GetProcAddress(m_hUxThemeLib, "GetThemeBackgroundExtent");
    if(!pfnGetThemeBackgroundExtent)
        return HRESULT_FROM_WIN32(GetLastError());

    return pfnGetThemeBackgroundExtent(hTheme, hdc, iPartId, iStateId, pContentRect, pExtentRect);
}

HRESULT CUxThemeAeroImpl::GetThemeBitmap(HTHEME hTheme, int iPartId, int iStateId, int iPropId, ULONG dwFlags, HBITMAP *phBitmap)
{
    if(!IsInitialized())
    {
        return OLE_E_BLANK;
    }

    GET_THEME_BITMAP pfnGetThemeBitmap = (GET_THEME_BITMAP)GetProcAddress(m_hUxThemeLib, "GetThemeBitmap");
    if(!pfnGetThemeBitmap)
        return HRESULT_FROM_WIN32(GetLastError());

    return pfnGetThemeBitmap(hTheme, iPartId, iStateId, iPropId, dwFlags, phBitmap);
}

BOOL CUxThemeAeroImpl::DetermineGlowSize(int *piSize, LPCWSTR pszClassIdList /*= nullptr*/)
{
    if(!piSize)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if(!pszClassIdList)
        pszClassIdList = L"CompositedWindow::Window";

    HTHEME hThemeWindow = OpenThemeData(nullptr, pszClassIdList);
    if (hThemeWindow != nullptr)
    {
        GetThemeInt(hThemeWindow, 0, 0, TMT_TEXTGLOWSIZE, piSize);
        CloseThemeData(hThemeWindow);
        return TRUE;
    }

    SetLastError(ERROR_FILE_NOT_FOUND);
    return FALSE;
}

HRESULT CUxThemeAeroImpl::DrawThemeParentBackground(HWND hwnd, HDC hdc, const RECT *prc)
{
    if(!IsInitialized())
    {
        return OLE_E_BLANK;
    }

    DRAW_THEME_PARENT_BACKGROUND pfnDrawThemeParentBackground = (DRAW_THEME_PARENT_BACKGROUND)GetProcAddress(m_hUxThemeLib, "DrawThemeParentBackground");
    if(!pfnDrawThemeParentBackground)
        return HRESULT_FROM_WIN32(GetLastError());

    return pfnDrawThemeParentBackground(hwnd, hdc, prc);
}

BOOL CUxThemeAeroImpl::IsThemeBackgroundPartiallyTransparent(HTHEME hTheme,int iPartId, int iStateId)
{
    if(!IsInitialized())
    {
        return FALSE;
    }

    IS_THEME_BACKGROUND_PARTIALLY_TRANSPARENT pfnIsThemeBackgroundPartiallyTransparent = (IS_THEME_BACKGROUND_PARTIALLY_TRANSPARENT)GetProcAddress(m_hUxThemeLib, "IsThemeBackgroundPartiallyTransparent");
    if(!pfnIsThemeBackgroundPartiallyTransparent)
        return FALSE;

    return pfnIsThemeBackgroundPartiallyTransparent(hTheme,iPartId, iStateId);
}

HRESULT CUxThemeAeroImpl::DrawThemeText(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, LPCRECT pRect)
{
    if(!IsInitialized())
    {
        return OLE_E_BLANK;
    }

    DRAW_THEME_TEXT pfnDrawThemeText = (DRAW_THEME_TEXT)GetProcAddress(m_hUxThemeLib, "DrawThemeText");
    if(!pfnDrawThemeText)
        return HRESULT_FROM_WIN32(GetLastError());

    return pfnDrawThemeText(hTheme, hdc, iPartId, iStateId, pszText, iCharCount, dwTextFlags, dwTextFlags2, pRect);
}

HRESULT CUxThemeAeroImpl::GetThemeColor(HTHEME hTheme, int iPartId, int iStateId, int iPropId, COLORREF *pColor)
{
    if(!IsInitialized())
    {
        return OLE_E_BLANK;
    }

    GET_THEME_COLOR pfnGetThemeColor = (GET_THEME_COLOR)GetProcAddress(m_hUxThemeLib, "GetThemeColor");
    if(!pfnGetThemeColor)
        return HRESULT_FROM_WIN32(GetLastError());

    return pfnGetThemeColor(hTheme, iPartId, iStateId, iPropId, pColor);
}

HRESULT CUxThemeAeroImpl::GetThemePartSize(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT prc, THEMESIZE eSize,SIZE *psz)
{
    if(!IsInitialized())
    {
        return OLE_E_BLANK;
    }

    GET_THEME_PART_SIZE pfnGetThemePartSize = (GET_THEME_PART_SIZE)GetProcAddress(m_hUxThemeLib, "GetThemePartSize");
    if(!pfnGetThemePartSize)
        return HRESULT_FROM_WIN32(GetLastError());

    return pfnGetThemePartSize(hTheme, hdc, iPartId, iStateId, prc, eSize, psz);
}

HRESULT CUxThemeAeroImpl::GetThemePosition(HTHEME hTheme, int iPartId, int iStateId, int iPropId, POINT *pPoint)
{
    if(!IsInitialized())
    {
        return OLE_E_BLANK;
    }

    GET_THEME_POSITION pfnGetThemePosition = (GET_THEME_POSITION)GetProcAddress(m_hUxThemeLib, "GetThemePosition");
    if(!pfnGetThemePosition)
        return HRESULT_FROM_WIN32(GetLastError());

    return pfnGetThemePosition(hTheme, iPartId, iStateId, iPropId, pPoint);
}

HRESULT CUxThemeAeroImpl::GetThemeMargins(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, LPRECT prc, MARGINS *pMargins)
{
    if(!IsInitialized())
    {
        return OLE_E_BLANK;
    }

    GET_THEME_MARGINS pfnGetThemeMargins = (GET_THEME_MARGINS)GetProcAddress(m_hUxThemeLib, "GetThemeMargins");
    if(!pfnGetThemeMargins)
        return HRESULT_FROM_WIN32(GetLastError());

    return pfnGetThemeMargins(hTheme, hdc, iPartId, iStateId, iPropId, prc, pMargins);
}

HRESULT CUxThemeAeroImpl::GetThemeMetric(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, int *piVal)
{
    if(!IsInitialized())
    {
        return OLE_E_BLANK;
    }

    GET_THEME_METRIC pfnGetThemeMetric = (GET_THEME_METRIC)GetProcAddress(m_hUxThemeLib, "GetThemeMetric");
    if(!pfnGetThemeMetric)
        return HRESULT_FROM_WIN32(GetLastError());

    return pfnGetThemeMetric(hTheme, hdc, iPartId, iStateId, iPropId, piVal);
}

HRESULT CUxThemeAeroImpl::GetThemeRect(HTHEME hTheme, int iPartId, int iStateId, int iPropId, LPRECT pRect)
{
    if(!IsInitialized())
    {
        return OLE_E_BLANK;
    }

    GET_THEME_RECT pfnGetThemeRect = (GET_THEME_RECT)GetProcAddress(m_hUxThemeLib, "GetThemeRect");
    if(!pfnGetThemeRect)
        return HRESULT_FROM_WIN32(GetLastError());

    return pfnGetThemeRect(hTheme, iPartId, iStateId, iPropId, pRect);
}
