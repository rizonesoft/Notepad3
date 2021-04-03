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
#include "stdafx.h"
#include "Windows10Colors.h"

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "ntdll.lib")

using namespace Microsoft::WRL;
namespace WindowsUI = ABI::Windows::UI;

/// Wrapper class for WinRT string reference
class HStringRef
{
    HSTRING        m_hStr;
    HSTRING_HEADER m_strHeader;

public:
    HStringRef()
        : m_hStr(nullptr)
        , m_strHeader()
    {
    }

    // String ref doesn't need dtor

    template <size_t N>
    HRESULT Set(const wchar_t (&str)[N])
    {
        return Win10Colors::WindowsCreateStringReference(str, N - 1, &m_strHeader, &m_hStr);
    }

    operator HSTRING() const { return m_hStr; }
};

/// Call RoActivateInstance and query an interface
template <typename If>
static HRESULT ActivateInstance(HSTRING classId, ComPtr<If>& instance)
{
    ComPtr<IInspectable> inspectable;
    auto                 hr = Win10Colors::RoActivateInstance(classId, &inspectable);
    if (FAILED(hr))
        return hr;
    return inspectable.As(&instance);
}

Win10Colors Win10Colors::instance;

Win10Colors::Win10Colors()
{
    if (!m_modulesLoaded)
    {
        m_modulesLoaded = true;
        winrt           = LoadLibraryW(L"api-ms-win-core-winrt-l1-1-0.dll");
        if (winrt)
        {
            pRoActivateInstance = reinterpret_cast<PfnRoActivateInstance>(
                GetProcAddress(winrt, "RoActivateInstance"));
        }
        m_winrtString = LoadLibraryW(L"api-ms-win-core-winrt-string-l1-1-0.dll");
        if (m_winrtString)
        {
            pWindowsCreateStringReference = reinterpret_cast<PfnWindowsCreateStringReference>(
                GetProcAddress(m_winrtString, "WindowsCreateStringReference"));
        }
    }
}

Win10Colors::~Win10Colors()
{
    if (winrt)
        FreeLibrary(winrt);
    if (m_winrtString)
        FreeLibrary(m_winrtString);
}

HRESULT Win10Colors::GetAccentColor(AccentColor& color)
{
    HStringRef classId;
    auto       hr = classId.Set(L"Windows.UI.ViewManagement.UISettings");
    if (FAILED(hr))
        return hr;
    Microsoft::WRL::ComPtr<WindowsUI::ViewManagement::IUISettings> settings;
    hr = ActivateInstance(classId, settings);
    if (FAILED(hr))
        return hr;

    ComPtr<WindowsUI::ViewManagement::IUISettings3> settings3;
    hr = settings.As(&settings3);
    if (!settings3)
        return E_FAIL;

    WindowsUI::Color uiColor;
    hr = settings3->GetColorValue(WindowsUI::ViewManagement::UIColorType_Foreground, &uiColor);
    if (FAILED(hr))
        return hr;
    color.foreground = ToRGBA(uiColor);
    hr               = settings3->GetColorValue(WindowsUI::ViewManagement::UIColorType_Background, &uiColor);
    if (FAILED(hr))
        return hr;
    color.background = ToRGBA(uiColor);
    hr               = settings3->GetColorValue(WindowsUI::ViewManagement::UIColorType_AccentDark3, &uiColor);
    if (FAILED(hr))
        return hr;
    color.darkest = ToRGBA(uiColor);
    hr            = settings3->GetColorValue(WindowsUI::ViewManagement::UIColorType_AccentDark2, &uiColor);
    if (FAILED(hr))
        return hr;
    color.darker = ToRGBA(uiColor);
    hr           = settings3->GetColorValue(WindowsUI::ViewManagement::UIColorType_AccentDark1, &uiColor);
    if (FAILED(hr))
        return hr;
    color.dark = ToRGBA(uiColor);
    hr         = settings3->GetColorValue(WindowsUI::ViewManagement::UIColorType_Accent, &uiColor);
    if (FAILED(hr))
        return hr;
    color.accent = ToRGBA(uiColor);
    hr           = settings3->GetColorValue(WindowsUI::ViewManagement::UIColorType_AccentLight1, &uiColor);
    if (FAILED(hr))
        return hr;
    color.light = ToRGBA(uiColor);
    hr          = settings3->GetColorValue(WindowsUI::ViewManagement::UIColorType_AccentLight2, &uiColor);
    if (FAILED(hr))
        return hr;
    color.lighter = ToRGBA(uiColor);
    hr            = settings3->GetColorValue(WindowsUI::ViewManagement::UIColorType_AccentLight3, &uiColor);
    if (FAILED(hr))
        return hr;
    color.lightest = ToRGBA(uiColor);

    return S_OK;
}
