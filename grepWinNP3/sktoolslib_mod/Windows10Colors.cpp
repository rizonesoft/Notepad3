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
#include "stdafx.h"
#include "Windows10Colors.h"

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "ntdll.lib")

using namespace Microsoft::WRL;
namespace WindowsUI = ABI::Windows::UI;

/// Wrapper class for WinRT string reference
class HStringRef
{
    HSTRING        hstr;
    HSTRING_HEADER str_header;

public:
    HStringRef()
        : hstr(nullptr)
    {
    }
    // String ref doesn't need dtor

    template <size_t N>
    HRESULT Set(const wchar_t (&str)[N])
    {
        return Win10Colors::WindowsCreateStringReference(str, N - 1, &str_header, &hstr);
    }

    operator HSTRING() const { return hstr; }
};

/// Call RoActivateInstance and query an interface
template <typename IF>
static HRESULT ActivateInstance(HSTRING classId, ComPtr<IF>& instance)
{
    ComPtr<IInspectable> inspectable;
    auto hr = Win10Colors::RoActivateInstance(classId, &inspectable);
    if (FAILED(hr))
        return hr;
    return inspectable.As(&instance);
}

Win10Colors Win10Colors::instance;

Win10Colors::Win10Colors()
{
    if (!modules_loaded)
    {
        modules_loaded = true;
        winrt          = LoadLibraryW(L"api-ms-win-core-winrt-l1-1-0.dll");
        if (winrt)
        {
            pRoActivateInstance = reinterpret_cast<pfnRoActivateInstance>(
                GetProcAddress(winrt, "RoActivateInstance"));
        }
        winrt_string = LoadLibraryW(L"api-ms-win-core-winrt-string-l1-1-0.dll");
        if (winrt_string)
        {
            pWindowsCreateStringReference = reinterpret_cast<pfnWindowsCreateStringReference>(
                GetProcAddress(winrt_string, "WindowsCreateStringReference"));
        }
    }
}

Win10Colors::~Win10Colors()
{
    if (winrt)
        FreeLibrary(winrt);
    if (winrt_string)
        FreeLibrary(winrt_string);
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

    WindowsUI::Color ui_color;
    hr = settings3->GetColorValue(WindowsUI::ViewManagement::UIColorType_Foreground, &ui_color);
    if (FAILED(hr))
        return hr;
    color.foreground = ToRGBA(ui_color);
    hr = settings3->GetColorValue(WindowsUI::ViewManagement::UIColorType_Background, &ui_color);
    if (FAILED(hr))
        return hr;
    color.background = ToRGBA(ui_color);
    hr = settings3->GetColorValue(WindowsUI::ViewManagement::UIColorType_AccentDark3, &ui_color);
    if (FAILED(hr))
        return hr;
    color.darkest = ToRGBA(ui_color);
    hr            = settings3->GetColorValue(WindowsUI::ViewManagement::UIColorType_AccentDark2, &ui_color);
    if (FAILED(hr))
        return hr;
    color.darker = ToRGBA(ui_color);
    hr           = settings3->GetColorValue(WindowsUI::ViewManagement::UIColorType_AccentDark1, &ui_color);
    if (FAILED(hr))
        return hr;
    color.dark = ToRGBA(ui_color);
    hr         = settings3->GetColorValue(WindowsUI::ViewManagement::UIColorType_Accent, &ui_color);
    if (FAILED(hr))
        return hr;
    color.accent = ToRGBA(ui_color);
    hr           = settings3->GetColorValue(WindowsUI::ViewManagement::UIColorType_AccentLight1, &ui_color);
    if (FAILED(hr))
        return hr;
    color.light = ToRGBA(ui_color);
    hr          = settings3->GetColorValue(WindowsUI::ViewManagement::UIColorType_AccentLight2, &ui_color);
    if (FAILED(hr))
        return hr;
    color.lighter = ToRGBA(ui_color);
    hr            = settings3->GetColorValue(WindowsUI::ViewManagement::UIColorType_AccentLight3, &ui_color);
    if (FAILED(hr))
        return hr;
    color.lightest = ToRGBA(ui_color);

    return S_OK;
}
