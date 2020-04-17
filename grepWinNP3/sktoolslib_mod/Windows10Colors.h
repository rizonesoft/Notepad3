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
/**
 * Functions to obtain Windows 10 accent colors and colors used to paint window frames.
 */

#include <Windows.h>
#include <vector>
#include <wrl.h>
#include <windows.ui.viewmanagement.h>

class Win10Colors
{
public:
    Win10Colors();
    ~Win10Colors();

    /**
     * RGBA color. Red is in the LSB, Alpha in the MSB.
     * You can use GetRValue() et al to access individual components.
     */
    typedef DWORD RGBA;

    /// Accent color shades
    struct AccentColor
    {
        /// foreground accent color
        RGBA foreground;
        /// background accent color
        RGBA background;
        /// Base accent color
        RGBA accent;
        /// Darkest shade.
        RGBA darkest;
        /// Darker shade.
        RGBA darker;
        /// Dark shade.
        RGBA dark;
        /// Light shade.
        RGBA light;
        /// Lighter shade.
        RGBA lighter;
        /// Lightest shade.
        RGBA lightest;
    };

    static HRESULT GetAccentColor(AccentColor& color);

    /// Dynamically loaded WindowsCreateStringReference, if available
    static HRESULT WindowsCreateStringReference(PCWSTR sourceString, UINT32 length, HSTRING_HEADER* hstringHeader, HSTRING* string)
    {
        return instance.WindowsCreateStringReferenceImpl(sourceString, length, hstringHeader, string);
    }

    /// Dynamically loaded RoActivateInstance, if available
    static HRESULT RoActivateInstance(HSTRING activatableClassId, IInspectable** newInstance)
    {
        return instance.RoActivateInstanceImpl(activatableClassId, newInstance);
    }

protected:
    /// Wrap WindowsCreateStringReference
    inline HRESULT WindowsCreateStringReferenceImpl(PCWSTR sourceString, UINT32 length, HSTRING_HEADER* hstringHeader, HSTRING* string)
    {
        if (!pWindowsCreateStringReference)
            return E_NOTIMPL;
        return pWindowsCreateStringReference(sourceString, length, hstringHeader, string);
    }

    /// Wrap RoActivateInstance
    inline HRESULT RoActivateInstanceImpl(HSTRING activatableClassId, IInspectable** inst)
    {
        if (!pRoActivateInstance)
            return E_NOTIMPL;
        return pRoActivateInstance(activatableClassId, inst);
    }

    static inline RGBA MakeRGBA(uint8_t R, uint8_t G, uint8_t B, uint8_t A)
    {
        return RGB(R, G, B) | (A << 24);
    }

    static inline RGBA ToRGBA(ABI::Windows::UI::Color color)
    {
        return MakeRGBA(color.R, color.G, color.B, color.A);
    }

private:
    static Win10Colors instance;
    bool               modules_loaded = false;
    HMODULE            winrt          = 0;
    HMODULE            winrt_string   = 0;

    typedef HRESULT(STDAPICALLTYPE* pfnWindowsCreateStringReference)(
        PCWSTR sourceString, UINT32 length, HSTRING_HEADER* hstringHeader, HSTRING* string);
    pfnWindowsCreateStringReference pWindowsCreateStringReference = nullptr;
    typedef HRESULT(WINAPI* pfnRoActivateInstance)(HSTRING activatableClassId, IInspectable** instance);
    pfnRoActivateInstance pRoActivateInstance = nullptr;
};
