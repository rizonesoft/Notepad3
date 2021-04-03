// sktoolslib - common files for SK tools

// Copyright (C) 2012, 2017, 2020-2021 - Stefan Kueng

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

#define TMSCHEMA_H // this excludes the deprecated tmschema.h without dependency on _WIN32_WINNT macro
#include <windows.h>
#include <Uxtheme.h>

class CDwmApiImpl
{
private:
    HINSTANCE m_hDwmApiLib;
    BOOL      IsInitialized() const;

public:
    CDwmApiImpl();
    ~CDwmApiImpl();
    BOOL    Initialize();
    HRESULT DwmExtendFrameIntoClientArea(HWND hWnd, const MARGINS* pMarInset) const;
    BOOL    IsDwmCompositionEnabled() const;
    HRESULT DwmEnableComposition(UINT uCompositionAction) const;
};

class CUxThemeAeroImpl
{
private:
    HINSTANCE m_hUxThemeLib;
    BOOL      IsInitialized() const;

public:
    CUxThemeAeroImpl();
    BOOL Initialize();
    ~CUxThemeAeroImpl();
    HRESULT      BufferedPaintInit() const;
    HRESULT      BufferedPaintUnInit() const;
    HTHEME       OpenThemeData(HWND hwnd, LPCWSTR pszClassList) const;
    BOOL         DetermineGlowSize(int* piSize, LPCWSTR pszClassIdList = nullptr) const;
    HRESULT      CloseThemeData(HTHEME hTheme) const;
    HPAINTBUFFER BeginBufferedPaint(HDC hdcTarget, const RECT* prcTarget, BP_BUFFERFORMAT dwFormat, BP_PAINTPARAMS* pPaintParams, HDC* phdc) const;
    HRESULT      EndBufferedPaint(HPAINTBUFFER hBufferedPaint, BOOL fUpdateTarget) const;
    HRESULT      DrawThemeTextEx(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int cchText, DWORD dwTextFlags, LPRECT pRect, const DTTOPTS* pOptions) const;
    HRESULT      GetThemeInt(HTHEME hTheme, int iPartId, int iStateId, int iPropId, int* piVal) const;
    HRESULT      GetThemeSysFont(HTHEME hTheme, int iFontId, LOGFONTW* plf) const;
    HRESULT      BufferedPaintSetAlpha(HPAINTBUFFER hBufferedPaint, const RECT* prc, BYTE alpha) const;
    HRESULT      BufferedPaintMakeOpaque_(HPAINTBUFFER hBufferedPaint, const RECT* prc) const;
    HRESULT      DrawThemeBackground(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT* pRect, const RECT* pClipRect) const;
    HRESULT      GetThemeBackgroundContentRect(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pBoundingRect, LPRECT pContentRect) const;
    HRESULT      GetThemeBackgroundExtent(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pContentRect, LPRECT pExtentRect) const;
    HRESULT      GetThemeBitmap(HTHEME hTheme, int iPartId, int iStateId, int iPropId, ULONG dwFlags, HBITMAP* phBitmap) const;
    HRESULT      DrawThemeParentBackground(HWND hwnd, HDC hdc, const RECT* prc) const;
    BOOL         IsThemeBackgroundPartiallyTransparent(HTHEME hTheme, int iPartId, int iStateId) const;
    HRESULT      DrawThemeText(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, LPCRECT pRect) const;
    HRESULT      GetThemeColor(HTHEME hTheme, int iPartId, int iStateId, int iPropId, COLORREF* pColor) const;
    HRESULT      GetThemePartSize(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT prc, THEMESIZE eSize, SIZE* psz) const;
    HRESULT      GetThemePosition(HTHEME hTheme, int iPartId, int iStateId, int iPropId, POINT* pPoint) const;
    HRESULT      GetThemeMargins(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, LPRECT prc, MARGINS* pMargins) const;
    HRESULT      GetThemeMetric(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, int* piVal) const;
    HRESULT      GetThemeRect(HTHEME hTheme, int iPartId, int iStateId, int iPropId, LPRECT pRect) const;
};
