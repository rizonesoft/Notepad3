// sktoolslib - common files for SK tools

// Copyright (C) 2018 - Stefan Kueng

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
        , pfnGetDpiForWindow(nullptr)
        , pfnGetDpiForSystem(nullptr)
        , pfnGetSystemMetricsForDpi(nullptr)
        , pfnSystemParametersInfoForDpi(nullptr)
    {}
    ~CDPIAware() {}
public:
    static CDPIAware&   Instance()
    {
        static CDPIAware instance;
        return instance;
    }

    // Get screen DPI.
    int GetDPI() { _Init(); return m_dpi; }

    // Convert between raw pixels and relative pixels.
    int Scale(int x) { _Init(); return MulDiv(x, m_dpi, 96); }
    float ScaleFactor() { _Init(); return m_dpi / 96.0f; }
    int Unscale(int x) { _Init(); return MulDiv(x, 96, m_dpi); }

    // Determine the screen dimensions in relative pixels.
    int ScaledScreenWidth() { return _ScaledSystemMetric(SM_CXSCREEN); }
    int ScaledScreenHeight() { return _ScaledSystemMetric(SM_CYSCREEN); }

    // Scale rectangle from raw pixels to relative pixels.
    void ScaleRect(__inout RECT *pRect)
    {
        pRect->left = Scale(pRect->left);
        pRect->right = Scale(pRect->right);
        pRect->top = Scale(pRect->top);
        pRect->bottom = Scale(pRect->bottom);
    }

    // Scale Point from raw pixels to relative pixels.
    void ScalePoint(__inout POINT *pPoint)
    {
        pPoint->x = Scale(pPoint->x);
        pPoint->y = Scale(pPoint->y);
    }

    // Scale Size from raw pixels to relative pixels.
    void ScaleSize(__inout SIZE *pSize)
    {
        pSize->cx = Scale(pSize->cx);
        pSize->cy = Scale(pSize->cy);
    }

    // Determine if screen resolution meets minimum requirements in relative pixels.
    bool IsResolutionAtLeast(int cxMin, int cyMin)
    {
        return (ScaledScreenWidth() >= cxMin) && (ScaledScreenHeight() >= cyMin);
    }

    // Convert a point size (1/72 of an inch) to raw pixels.
    int PointsToPixels(int pt) { _Init(); return MulDiv(pt, m_dpi, 72); }

    // returns the system metrics. For Windows 10, it returns the metrics dpi scaled.
    UINT GetSystemMetrics(int nIndex)
    {
        return _ScaledSystemMetric(nIndex);
    }

    // returns the system parameters info. If possible adjusted for dpi.
    UINT SystemParametersInfo(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni)
    {
        _Init();
        UINT ret = 0;
        if (pfnSystemParametersInfoForDpi)
            ret = pfnSystemParametersInfoForDpi(uiAction, uiParam, pvParam, fWinIni, m_dpi);
        if (ret == 0)
            ret = ::SystemParametersInfo(uiAction, uiParam, pvParam, fWinIni);
        return ret;
    }

    // Invalidate any cached metrics.
    void Invalidate() { m_fInitialized = false; }

private:

    // This function initializes the CDPIAware Class
    void _Init()
    {
        if (!m_fInitialized)
        {
            auto hUser = ::GetModuleHandle(L"user32.dll");
            if (hUser)
            {
                pfnGetDpiForWindow = (GetDpiForWindowFN *)GetProcAddress(hUser, "GetDpiForWindow");
                pfnGetDpiForSystem = (GetDpiForSystemFN *)GetProcAddress(hUser, "GetDpiForSystem");
                pfnGetSystemMetricsForDpi = (GetSystemMetricsForDpiFN *)GetProcAddress(hUser, "GetSystemMetricsForDpi");
                pfnSystemParametersInfoForDpi = (SystemParametersInfoForDpiFN *)GetProcAddress(hUser, "SystemParametersInfoForDpi");
            }

            if (pfnGetDpiForSystem)
            {
                m_dpi = pfnGetDpiForSystem();
            }
            else
            {
                HDC hdc = GetDC(nullptr);
                if (hdc)
                {
                    // Initialize the DPI member variable
                    // This will correspond to the DPI setting
                    // With all Windows OS's to date the X and Y DPI will be identical
                    m_dpi = GetDeviceCaps(hdc, LOGPIXELSX);
                    ReleaseDC(nullptr, hdc);
                }
            }
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
        if (pfnGetSystemMetricsForDpi)
            return pfnGetSystemMetricsForDpi(nIndex, m_dpi);
        return MulDiv(::GetSystemMetrics(nIndex), 96, m_dpi);
    }

private:
    typedef UINT STDAPICALLTYPE GetDpiForWindowFN(HWND hWnd);
    typedef UINT STDAPICALLTYPE GetDpiForSystemFN();
    typedef UINT STDAPICALLTYPE GetSystemMetricsForDpiFN(int nIndex, UINT dpi);
    typedef UINT STDAPICALLTYPE SystemParametersInfoForDpiFN(UINT uiAction,UINT uiParam, PVOID pvParam, UINT fWinIni, UINT dpi);

    GetDpiForWindowFN * pfnGetDpiForWindow;
    GetDpiForSystemFN * pfnGetDpiForSystem;
    GetSystemMetricsForDpiFN * pfnGetSystemMetricsForDpi;
    SystemParametersInfoForDpiFN * pfnSystemParametersInfoForDpi;

    // Member variable indicating whether the class has been initialized
    bool m_fInitialized;

    int m_dpi;
};
