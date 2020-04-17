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

#pragma once

#include <specstrings.h>

#define TMSCHEMA_H // this excludes the deprecated tmschema.h without dependency on _WIN32_WINNT macro
#include <windows.h>
#include <Uxtheme.h>
#include <vssym32.h>


class CDwmApiImpl
{
private:
    HINSTANCE m_hDwmApiLib;
    BOOL IsInitialized(void);

public:
    CDwmApiImpl(void);
    ~CDwmApiImpl(void);
    BOOL Initialize(void);
    HRESULT DwmExtendFrameIntoClientArea(HWND hWnd,const MARGINS* pMarInset);
    BOOL IsDwmCompositionEnabled(void);
    HRESULT DwmEnableComposition(UINT uCompositionAction);
};


class CUxThemeAeroImpl
{
private:
    HINSTANCE m_hUxThemeLib;
    BOOL IsInitialized(void);

public:
    CUxThemeAeroImpl(void);
    BOOL Initialize(void);
    ~CUxThemeAeroImpl(void);
    HRESULT BufferedPaintInit(void);
    HRESULT BufferedPaintUnInit(void);
    HTHEME OpenThemeData(HWND hwnd, LPCWSTR pszClassList);
    BOOL DetermineGlowSize(int *piSize, LPCWSTR pszClassIdList = nullptr);
    HRESULT CloseThemeData(HTHEME hTheme);
    HPAINTBUFFER BeginBufferedPaint(HDC hdcTarget, const RECT* prcTarget, BP_BUFFERFORMAT dwFormat, BP_PAINTPARAMS *pPaintParams, HDC *phdc);
    HRESULT EndBufferedPaint(HPAINTBUFFER hBufferedPaint, BOOL fUpdateTarget);
    HRESULT DrawThemeTextEx(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int cchText, DWORD dwTextFlags, LPRECT pRect, const DTTOPTS *pOptions);
    HRESULT GetThemeInt(HTHEME hTheme, int iPartId, int iStateId, int iPropId, int *piVal);
    HRESULT GetThemeSysFont(HTHEME hTheme, int iFontId, LOGFONTW *plf);
    HRESULT BufferedPaintSetAlpha(HPAINTBUFFER hBufferedPaint, const RECT *prc, BYTE alpha);
    HRESULT BufferedPaintMakeOpaque_(HPAINTBUFFER hBufferedPaint, const RECT *prc);
    HRESULT DrawThemeBackground(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect);
    HRESULT GetThemeBackgroundContentRect(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pBoundingRect, LPRECT pContentRect);
    HRESULT GetThemeBackgroundExtent(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pContentRect, LPRECT pExtentRect);
    HRESULT GetThemeBitmap(HTHEME hTheme, int iPartId, int iStateId, int iPropId, ULONG dwFlags, HBITMAP *phBitmap);
    HRESULT DrawThemeParentBackground(HWND hwnd, HDC hdc, const RECT *prc);
    BOOL IsThemeBackgroundPartiallyTransparent(HTHEME hTheme,int iPartId, int iStateId);
    HRESULT DrawThemeText(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, LPCRECT pRect);
    HRESULT GetThemeColor(HTHEME hTheme, int iPartId, int iStateId, int iPropId, COLORREF *pColor);
    HRESULT GetThemePartSize(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT prc, THEMESIZE eSize,SIZE *psz);
    HRESULT GetThemePosition(HTHEME hTheme, int iPartId, int iStateId, int iPropId, POINT *pPoint);
    HRESULT GetThemeMargins(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, LPRECT prc, MARGINS *pMargins);
    HRESULT GetThemeMetric(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, int *piVal);
    HRESULT GetThemeRect(HTHEME hTheme, int iPartId, int iStateId, int iPropId, LPRECT pRect);
};
