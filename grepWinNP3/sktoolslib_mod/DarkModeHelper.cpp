// sktoolslib - common files for SK tools

// Copyright (C) 2019-2020 - Stefan Kueng

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
#include "DarkModeHelper.h"
#include "StringUtils.h"
#include "PathUtils.h"
#include <vector>
#include <Shlobj.h>

DarkModeHelper& DarkModeHelper::Instance()
{
    static DarkModeHelper helper;
    return helper;
}

bool DarkModeHelper::CanHaveDarkMode()
{
    return m_bCanHaveDarkMode;
}

void DarkModeHelper::AllowDarkModeForApp(BOOL allow)
{
    if (m_pAllowDarkModeForApp)
        m_pAllowDarkModeForApp(allow ? 1 : 0);
    if (m_pSetPreferredAppMode)
        m_pSetPreferredAppMode(allow ? AllowDark : Default);
}

void DarkModeHelper::AllowDarkModeForWindow(HWND hwnd, BOOL allow)
{
    if (m_pAllowDarkModeForWindow)
        m_pAllowDarkModeForWindow(hwnd, allow);
}

BOOL DarkModeHelper::ShouldAppsUseDarkMode()
{
    if (m_pShouldAppsUseDarkMode)
        return m_pShouldAppsUseDarkMode() & 0x01;
    return FALSE;
}

BOOL DarkModeHelper::IsDarkModeAllowedForWindow(HWND hwnd)
{
    if (m_pIsDarkModeAllowedForWindow)
        return m_pIsDarkModeAllowedForWindow(hwnd);
    return FALSE;
}

BOOL DarkModeHelper::IsDarkModeAllowedForApp()
{
    if (m_pIsDarkModeAllowedForApp)
        return m_pIsDarkModeAllowedForApp();
    return FALSE;
}

BOOL DarkModeHelper::ShouldSystemUseDarkMode()
{
    if (m_pShouldSystemUseDarkMode)
        return m_pShouldSystemUseDarkMode();
    return FALSE;
}

void DarkModeHelper::RefreshImmersiveColorPolicyState()
{
    if (m_pRefreshImmersiveColorPolicyState)
        m_pRefreshImmersiveColorPolicyState();
}

BOOL DarkModeHelper::GetIsImmersiveColorUsingHighContrast(IMMERSIVE_HC_CACHE_MODE mode)
{
    if (m_pGetIsImmersiveColorUsingHighContrast)
        return m_pGetIsImmersiveColorUsingHighContrast(mode);
    return FALSE;
}

void DarkModeHelper::FlushMenuThemes()
{
    if (m_pFlushMenuThemes)
        m_pFlushMenuThemes();
}

BOOL DarkModeHelper::SetWindowCompositionAttribute(HWND hWnd, WINDOWCOMPOSITIONATTRIBDATA* data)
{
    if (m_pSetWindowCompositionAttribute)
        return m_pSetWindowCompositionAttribute(hWnd, data);
    return FALSE;
}

DarkModeHelper::DarkModeHelper()
{
    INITCOMMONCONTROLSEX used = {
        sizeof(INITCOMMONCONTROLSEX),
        ICC_STANDARD_CLASSES | ICC_BAR_CLASSES | ICC_COOL_CLASSES};
    InitCommonControlsEx(&used);

    m_bCanHaveDarkMode = false;
    PWSTR sysPath      = nullptr;
    long  micro        = 0;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_System, 0, nullptr, &sysPath)))
    {
        std::wstring dllPath = sysPath;
        CoTaskMemFree(sysPath);
        dllPath += L"\\uxtheme.dll";
        auto                      version = CPathUtils::GetVersionFromFile(L"uxtheme.dll");
        std::vector<std::wstring> tokens;
        stringtok(tokens, version, false, L".");
        if (tokens.size() == 4)
        {
            auto major = std::stol(tokens[0]);
            auto minor = std::stol(tokens[1]);
            micro      = std::stol(tokens[2]);
            //auto build = std::stol(tokens[3]);

            // the windows 10 update 1809 has the version
            // number as 10.0.17763.1
            if (major > 10)
                m_bCanHaveDarkMode = true;
            else if (major == 10)
            {
                if (minor > 0)
                    m_bCanHaveDarkMode = true;
                else if (micro > 17762)
                    m_bCanHaveDarkMode = true;
            }
            m_bCanHaveDarkMode = true;
        }
    }

    m_hUxthemeLib = LoadLibrary(L"uxtheme.dll");
    if (m_hUxthemeLib && m_bCanHaveDarkMode)
    {
        // Note: these functions are undocumented! Which meas I shouldn't even use them.
        // So, since MS decided to keep this new feature to themselves, I have to use
        // undocumented functions to adjust.
        // Let's just hope they change their minds and document these functions one day...

        // first try with the names, just in case MS decides to properly export these functions
        if (micro < 18362)
            m_pAllowDarkModeForApp = (AllowDarkModeForAppFPN)GetProcAddress(m_hUxthemeLib, "AllowDarkModeForApp");
        else
            m_pSetPreferredAppMode = (SetPreferredAppModeFPN)GetProcAddress(m_hUxthemeLib, "SetPreferredAppMode");
        m_pAllowDarkModeForWindow               = (AllowDarkModeForWindowFPN)GetProcAddress(m_hUxthemeLib, "AllowDarkModeForWindow");
        m_pShouldAppsUseDarkMode                = (ShouldAppsUseDarkModeFPN)GetProcAddress(m_hUxthemeLib, "ShouldAppsUseDarkMode");
        m_pIsDarkModeAllowedForWindow           = (IsDarkModeAllowedForWindowFPN)GetProcAddress(m_hUxthemeLib, "IsDarkModeAllowedForWindow");
        m_pIsDarkModeAllowedForApp              = (IsDarkModeAllowedForAppFPN)GetProcAddress(m_hUxthemeLib, "IsDarkModeAllowedForApp");
        m_pShouldSystemUseDarkMode              = (ShouldSystemUseDarkModeFPN)GetProcAddress(m_hUxthemeLib, "ShouldSystemUseDarkMode");
        m_pRefreshImmersiveColorPolicyState     = (RefreshImmersiveColorPolicyStateFN)GetProcAddress(m_hUxthemeLib, "RefreshImmersiveColorPolicyState");
        m_pGetIsImmersiveColorUsingHighContrast = (GetIsImmersiveColorUsingHighContrastFN)GetProcAddress(m_hUxthemeLib, "GetIsImmersiveColorUsingHighContrast");
        m_pFlushMenuThemes                      = (FlushMenuThemesFN)GetProcAddress(m_hUxthemeLib, "FlushMenuThemes");
        m_pSetWindowCompositionAttribute        = (SetWindowCompositionAttributeFPN)GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetWindowCompositionAttribute");
        if (m_pAllowDarkModeForApp == nullptr && micro < 18362)
            m_pAllowDarkModeForApp = (AllowDarkModeForAppFPN)GetProcAddress(m_hUxthemeLib, MAKEINTRESOURCEA(135));
        if (m_pSetPreferredAppMode == nullptr && micro >= 18362)
            m_pSetPreferredAppMode = (SetPreferredAppModeFPN)GetProcAddress(m_hUxthemeLib, MAKEINTRESOURCEA(135));
        if (m_pAllowDarkModeForWindow == nullptr)
            m_pAllowDarkModeForWindow = (AllowDarkModeForWindowFPN)GetProcAddress(m_hUxthemeLib, MAKEINTRESOURCEA(133));
        if (m_pShouldAppsUseDarkMode == nullptr)
            m_pShouldAppsUseDarkMode = (ShouldAppsUseDarkModeFPN)GetProcAddress(m_hUxthemeLib, MAKEINTRESOURCEA(132));
        if (m_pIsDarkModeAllowedForWindow == nullptr)
            m_pIsDarkModeAllowedForWindow = (IsDarkModeAllowedForWindowFPN)GetProcAddress(m_hUxthemeLib, MAKEINTRESOURCEA(137));
        if (m_pIsDarkModeAllowedForApp == nullptr)
            m_pIsDarkModeAllowedForApp = (IsDarkModeAllowedForAppFPN)GetProcAddress(m_hUxthemeLib, MAKEINTRESOURCEA(139));
        if (m_pShouldSystemUseDarkMode == nullptr)
            m_pShouldSystemUseDarkMode = (ShouldSystemUseDarkModeFPN)GetProcAddress(m_hUxthemeLib, MAKEINTRESOURCEA(138));
        if (m_pRefreshImmersiveColorPolicyState == nullptr)
            m_pRefreshImmersiveColorPolicyState = (RefreshImmersiveColorPolicyStateFN)GetProcAddress(m_hUxthemeLib, MAKEINTRESOURCEA(104));
        if (m_pGetIsImmersiveColorUsingHighContrast == nullptr)
            m_pGetIsImmersiveColorUsingHighContrast = (GetIsImmersiveColorUsingHighContrastFN)GetProcAddress(m_hUxthemeLib, MAKEINTRESOURCEA(106));
        if (m_pFlushMenuThemes == nullptr)
            m_pFlushMenuThemes = (FlushMenuThemesFN)GetProcAddress(m_hUxthemeLib, MAKEINTRESOURCEA(136));
    }
}

DarkModeHelper::~DarkModeHelper()
{
    FreeLibrary(m_hUxthemeLib);
}
