// sktoolslib - common files for SK tools

// Copyright (C) 2018, 2020-2021 - Stefan Kueng

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

class CDPIAware
{
private:
    CDPIAware()
        : m_fInitialized(false)
        , m_dpi(96)
    {
    }
    ~CDPIAware() {}

public:
    static CDPIAware &Instance()
    {
        static CDPIAware instance;
        return instance;
    }

    // Get screen DPI.
    int GetDPI(HWND hWnd)
    {
        _Init();
        if (hWnd)
        {
            return ::GetDpiForWindow(hWnd);
        }
        return m_dpi;
    }
    // Convert between raw pixels and relative pixels.
    int   Scale(HWND hWnd, int x) { return MulDiv(x, GetDPI(hWnd), 96); }
    float ScaleFactor(HWND hWnd) { return GetDPI(hWnd) / 96.0f; }
    float ScaleFactorSystemToWindow(HWND hWnd) { return static_cast<float>(GetDPI(hWnd)) / static_cast<float>(m_dpi); }
    int   Unscale(HWND hWnd, int x) { return MulDiv(x, 96, GetDPI(hWnd)); }

    // Determine the screen dimensions in relative pixels.
    int ScaledScreenWidth() { return _ScaledSystemMetric(SM_CXSCREEN); }
    int ScaledScreenHeight() { return _ScaledSystemMetric(SM_CYSCREEN); }

    // Scale rectangle from raw pixels to relative pixels.
    void ScaleRect(HWND hWnd, __inout RECT *pRect)
    {
        pRect->left   = Scale(hWnd, pRect->left);
        pRect->right  = Scale(hWnd, pRect->right);
        pRect->top    = Scale(hWnd, pRect->top);
        pRect->bottom = Scale(hWnd, pRect->bottom);
    }

    // Scale Point from raw pixels to relative pixels.
    void ScalePoint(HWND hWnd, __inout POINT *pPoint)
    {
        pPoint->x = Scale(hWnd, pPoint->x);
        pPoint->y = Scale(hWnd, pPoint->y);
    }

    // Scale Size from raw pixels to relative pixels.
    void ScaleSize(HWND hWnd, __inout SIZE *pSize)
    {
        pSize->cx = Scale(hWnd, pSize->cx);
        pSize->cy = Scale(hWnd, pSize->cy);
    }

    // Determine if screen resolution meets minimum requirements in relative pixels.
    bool IsResolutionAtLeast(int cxMin, int cyMin)
    {
        return (ScaledScreenWidth() >= cxMin) && (ScaledScreenHeight() >= cyMin);
    }

    // Convert a point size (1/72 of an inch) to raw pixels.
    int PointsToPixels(HWND hWnd, int pt) { return MulDiv(pt, GetDPI(hWnd), 72); }

    // returns the system metrics. For Windows 10, it returns the metrics dpi scaled.
    UINT GetSystemMetrics(int nIndex)
    {
        return _ScaledSystemMetric(nIndex);
    }

    // returns the system parameters info. If possible adjusted for dpi.
    // SystemParametersInfoForDpi only supports SPI_GETNONCLIENTMETRICS,
    // SPI_GETICONMETRICS, and SPI_GETICONTITLELOGFONT. For all others,
    // fall back to regular SystemParametersInfo.
    UINT SystemParametersInfo(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni)
    {
        _Init();
        if (uiAction == SPI_GETNONCLIENTMETRICS ||
            uiAction == SPI_GETICONMETRICS ||
            uiAction == SPI_GETICONTITLELOGFONT)
        {
            return ::SystemParametersInfoForDpi(uiAction, uiParam, pvParam, fWinIni, m_dpi);
        }
        return ::SystemParametersInfoW(uiAction, uiParam, pvParam, fWinIni);
    }

    // Invalidate any cached metrics.
    void Invalidate() { m_fInitialized = false; }

private:
    // This function initializes the CDPIAware Class
    void _Init()
    {
        if (!m_fInitialized)
        {
            m_dpi          = ::GetDpiForSystem();
            m_fInitialized = true;
        }
    }

    // This returns a 96-DPI scaled-down equivalent value for nIndex
    // For example, the value 120 at 120 DPI setting gets scaled down to 96
    // X and Y versions are provided, though to date all Windows OS releases
    // have equal X and Y scale values
    int _ScaledSystemMetric(int nIndex)
    {
        _Init();
        return ::GetSystemMetricsForDpi(nIndex, m_dpi);
    }

private:
    // Member variable indicating whether the class has been initialized
    bool m_fInitialized;

    int m_dpi;
};
