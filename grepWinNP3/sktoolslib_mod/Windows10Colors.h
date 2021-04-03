// sktoolslib - common files for SK tools

// Copyright (C) 2018, 2021 - Stefan Kueng

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
    HRESULT WindowsCreateStringReferenceImpl(PCWSTR sourceString, UINT32 length, HSTRING_HEADER* hstringHeader, HSTRING* string) const
    {
        if (!pWindowsCreateStringReference)
            return E_NOTIMPL;
        return pWindowsCreateStringReference(sourceString, length, hstringHeader, string);
    }

    /// Wrap RoActivateInstance
    HRESULT RoActivateInstanceImpl(HSTRING activatableClassId, IInspectable** inst) const
    {
        if (!pRoActivateInstance)
            return E_NOTIMPL;
        return pRoActivateInstance(activatableClassId, inst);
    }

    static RGBA MakeRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        return RGB(r, g, b) | (a << 24);
    }

    static RGBA ToRGBA(ABI::Windows::UI::Color color)
    {
        return MakeRGBA(color.R, color.G, color.B, color.A);
    }

private:
    static Win10Colors instance;
    bool               m_modulesLoaded = false;
    HMODULE            winrt           = nullptr;
    HMODULE            m_winrtString   = nullptr;

    typedef HRESULT(STDAPICALLTYPE* PfnWindowsCreateStringReference)(
        PCWSTR sourceString, UINT32 length, HSTRING_HEADER* hStringHeader, HSTRING* string);
    PfnWindowsCreateStringReference pWindowsCreateStringReference = nullptr;
    typedef HRESULT(WINAPI* PfnRoActivateInstance)(HSTRING activatableClassId, IInspectable** instance);
    PfnRoActivateInstance pRoActivateInstance = nullptr;
};
