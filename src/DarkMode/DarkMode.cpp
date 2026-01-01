/******************************************************************************
* encoding: UTF-8                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* DarkMode.cpp                                                                *
*   Win10 DarkMode support                                                    *
*   Based on code from win32-darkmode by Richard Yu                           *
*            https://github.com/ysc3839/win32-darkmode/tree/delayload         *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2026   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

#if (defined(_DEBUG) || defined(DEBUG)) && !defined(NDEBUG)
#define DBG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <VersionHelpers.h>

#ifdef D_NP3_WIN10_DARK_MODE
#include <CommCtrl.h>
#include <Uxtheme.h>
#include <WindowsX.h>
#include <Vssym32.h>
#include <climits>
#endif
#include <cstdint>
#include <delayimp.h>

#define USE_DWMAPI 0
#include "DarkMode.h"

#ifdef D_NP3_WIN10_DARK_MODE
#include "IatHook.hpp"
#endif

#include "ListViewUtil.hpp"

#if USE_DWMAPI
#include <Dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")
#endif // USE_DWMAPI

// ----------------------------------------------------------------------------

#ifdef D_NP3_WIN10_DARK_MODE

#ifndef LOAD_LIBRARY_SEARCH_SYSTEM32
#define LOAD_LIBRARY_SEARCH_SYSTEM32 0x00000800
#endif

#if _WIN32_WINNT < _WIN32_WINNT_WIN8
DWORD const kSystemLibraryLoadFlags = (IsWindows8Point1OrGreater() ||
                                       GetProcAddress(GetModuleHandle(L"kernel32.dll"), "SetDefaultDllDirectories"))
                                      ? LOAD_LIBRARY_SEARCH_SYSTEM32
                                      : 0;
#else
#define kSystemLibraryLoadFlags LOAD_LIBRARY_SEARCH_SYSTEM32
#endif

// ============================================================================

// https://docs.microsoft.com/en-us/windows-insider/flight-hub/
// https://docs.microsoft.com/en-US/windows-insider/active-dev-branch

// ----------------------------------------------------------------------------

static bool CheckLoadLibrary(DWORD major, DWORD minor, DWORD buildNumber)
{
    // ignore "minor" build number
    UNREFERENCED_PARAMETER(minor);

    if (major < 10 || buildNumber < 17763) // Win10 v1809 (released)
        return false;        // NOT WORKING

    // keep reentrant for (!)
    static bool bUxThemeDllLoaded = false;

    if (!bUxThemeDllLoaded) {
        __try {
            HRESULT const res = __HrLoadAllImportsForDll("UxTheme.dll"); // Case sensitive
            bUxThemeDllLoaded = (res == NO_ERROR);
        }
        __except (GetExceptionCode() == VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND) ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
            bUxThemeDllLoaded = false;
        }
    }
    return bUxThemeDllLoaded;
}

// ============================================================================


extern "C" {
    // --- NtDll.dll ---
    NTSYSAPI VOID NTAPI RtlGetNtVersionNumbers(_Out_ LPDWORD major, _Out_ LPDWORD minor, _Out_ LPDWORD build);
}

extern "C" DWORD GetWindowsBuildNumber(LPDWORD major, LPDWORD minor)
{
    static DWORD _dwWindowsBuildNumber = 0;
    static DWORD _major = 0, _minor = 0;
    if (!_dwWindowsBuildNumber) {
        RtlGetNtVersionNumbers(&_major, &_minor, &_dwWindowsBuildNumber);
        _dwWindowsBuildNumber &= ~0xF0000000;
    }
    if (major) {
        *major = _major;
    }
    if (minor) {
        *minor = _minor;
    }
    return _dwWindowsBuildNumber;
}
// ============================================================================


enum class IMMERSIVE_HC_CACHE_MODE {
    IHCM_USE_CACHED_VALUE,
    IHCM_REFRESH
};

// Insider 18334
enum class PreferredAppMode {
    Default,
    AllowDark,
    ForceDark,
    ForceLight,
    Max
};


enum WINDOWCOMPOSITIONATTRIB {
    WCA_UNDEFINED = 0,
    WCA_NCRENDERING_ENABLED = 1,
    WCA_NCRENDERING_POLICY = 2,
    WCA_TRANSITIONS_FORCEDISABLED = 3,
    WCA_ALLOW_NCPAINT = 4,
    WCA_CAPTION_BUTTON_BOUNDS = 5,
    WCA_NONCLIENT_RTL_LAYOUT = 6,
    WCA_FORCE_ICONIC_REPRESENTATION = 7,
    WCA_EXTENDED_FRAME_BOUNDS = 8,
    WCA_HAS_ICONIC_BITMAP = 9,
    WCA_THEME_ATTRIBUTES = 10,
    WCA_NCRENDERING_EXILED = 11,
    WCA_NCADORNMENTINFO = 12,
    WCA_EXCLUDED_FROM_LIVEPREVIEW = 13,
    WCA_VIDEO_OVERLAY_ACTIVE = 14,
    WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
    WCA_DISALLOW_PEEK = 16,
    WCA_CLOAK = 17,
    WCA_CLOAKED = 18,
    WCA_ACCENT_POLICY = 19,
    WCA_FREEZE_REPRESENTATION = 20,
    WCA_EVER_UNCLOAKED = 21,
    WCA_VISUAL_OWNER = 22,
    WCA_HOLOGRAPHIC = 23,
    WCA_EXCLUDED_FROM_DDA = 24,
    WCA_PASSIVEUPDATEMODE = 25,
    WCA_USEDARKMODECOLORS = 26,
    WCA_LAST = 27
};
// ============================================================================


struct WINDOWCOMPOSITIONATTRIBDATA {
    WINDOWCOMPOSITIONATTRIB Attrib;
    PVOID pvData;
    SIZE_T cbData;
};
// ============================================================================



extern "C" {

// --- User32.dll
    WINUSERAPI BOOL WINAPI SetWindowCompositionAttribute(_In_ HWND hWnd, _In_ WINDOWCOMPOSITIONATTRIBDATA *);

// --- UxTheme.dll ---
// 1809 17763
    THEMEAPI_(bool) ShouldAppsUseDarkMode(); // ordinal 132
    THEMEAPI_(bool) AllowDarkModeForWindow(HWND hWnd, bool allow); // ordinal 133
    THEMEAPI_(bool) AllowDarkModeForApp(bool allow); // ordinal 135, in 1809
    THEMEAPI_(VOID) FlushMenuThemes(); // ordinal 136
    THEMEAPI_(VOID) RefreshImmersiveColorPolicyState(); // ordinal 104
    THEMEAPI_(bool) IsDarkModeAllowedForWindow(HWND hWnd); // ordinal 137
    THEMEAPI_(bool) GetIsImmersiveColorUsingHighContrast(IMMERSIVE_HC_CACHE_MODE mode); // ordinal 106
    THEMEAPI_(HTHEME) OpenNcThemeData(HWND hWnd, LPCWSTR pszClassList); // ordinal 49

// 1903 18362
    THEMEAPI_(bool) ShouldSystemUseDarkMode(); // ordinal 138
//THEMEAPI_(PreferredAppMode) SetPreferredAppMode(PreferredAppMode appMode); // ordinal 135, in 1903
    THEMEAPI_(bool) IsDarkModeAllowedForApp(); // ordinal 139

}

using fnSetPreferredAppMode = PreferredAppMode(WINAPI *)(PreferredAppMode appMode); // ordinal 135, in 1903

// ============================================================================


extern "C" bool IsHighContrast()
{
    HIGHCONTRASTW highContrast = { sizeof(HIGHCONTRASTW) };

    return SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(HIGHCONTRASTW), &highContrast, FALSE) ? (highContrast.dwFlags & HCF_HIGHCONTRASTON) : false;
}
// ============================================================================


static bool s_bDarkModeSupported = false;

extern "C" bool IsDarkModeSupported()
{
    return s_bDarkModeSupported;
}
// ============================================================================


static bool s_UserSetDarkMode = false;

extern "C" bool CheckDarkModeEnabled()
{
    return s_UserSetDarkMode && s_bDarkModeSupported && !IsHighContrast();
}
// ============================================================================


extern "C" bool ShouldAppsUseDarkModeEx()
{
    return s_bDarkModeSupported ? (ShouldAppsUseDarkMode() && !IsHighContrast()) : false;
}
// ============================================================================


extern "C" bool AllowDarkModeForWindowEx(HWND hWnd, bool allow)
{
    return s_bDarkModeSupported ? AllowDarkModeForWindow(hWnd, allow) : false;
}
// ============================================================================


extern "C" void RefreshTitleBarThemeColor(HWND hWnd)
{
    if (!s_bDarkModeSupported) {
        return;
    }

    BOOL dark = FALSE;
    if (IsDarkModeAllowedForWindow(hWnd) &&
            ShouldAppsUseDarkMode() &&
            !IsHighContrast()) {
        dark = TRUE;
    }
#if USE_DWMAPI
    if (SUCCEEDED(DwmSetWindowAttribute(hWnd, 20, &dark, sizeof(dark)))) {
        return;
    }
    DwmSetWindowAttribute(hWnd, 19, &dark, sizeof(dark));
#else  // USE_DWMAPI
    DWORD const buildNum = GetWindowsBuildNumber(nullptr, nullptr);
    if (buildNum < 18362) {
        SetPropW(hWnd, L"UseImmersiveDarkModeColors", reinterpret_cast<HANDLE>(static_cast<INT_PTR>(dark)));
    } else {
        WINDOWCOMPOSITIONATTRIBDATA data = { WCA_USEDARKMODECOLORS, &dark, sizeof(dark) };
        SetWindowCompositionAttribute(hWnd, &data);
    }
#endif // USE_DWMAPI
}
// ============================================================================


extern "C" bool IsColorSchemeChangeMessage(LPARAM lParam)
{
    bool is = false;
    if (s_bDarkModeSupported) {
        if (lParam && CompareStringOrdinal(reinterpret_cast<LPCWCH>(lParam), -1, L"ImmersiveColorSet", -1, TRUE) == CSTR_EQUAL) {
            RefreshImmersiveColorPolicyState();
            is = true;
        }
        GetIsImmersiveColorUsingHighContrast(IMMERSIVE_HC_CACHE_MODE::IHCM_REFRESH);
    }
    return is;
}
// ============================================================================


extern "C" bool IsColorSchemeChangeMessageMsg(UINT message, LPARAM lParam)
{
    return (message == WM_SETTINGCHANGE) ? IsColorSchemeChangeMessage(lParam) : false;
}
// ============================================================================


extern "C" void AllowDarkModeForAppEx(bool allow)
{
    if (s_bDarkModeSupported) {
        DWORD const buildNum = GetWindowsBuildNumber(nullptr, nullptr);
        if (buildNum < 18362) {
            AllowDarkModeForApp(allow);
        } else {
            reinterpret_cast<fnSetPreferredAppMode>(AllowDarkModeForApp)(allow ? PreferredAppMode::AllowDark : PreferredAppMode::Default);
        }
    }
}
// ============================================================================


static void FixDarkScrollBar(bool bDarkMode)
{
    HMODULE hComctl = LoadLibraryExW(L"comctl32.dll", nullptr, kSystemLibraryLoadFlags);
    if (hComctl) {
        auto addr = FindDelayLoadThunkInModule(hComctl, "uxtheme.dll", 49); // OpenNcThemeData
        if (addr) {
            DWORD oldProtect;
            if (VirtualProtect(addr, sizeof(IMAGE_THUNK_DATA), PAGE_READWRITE, &oldProtect)) {
                auto MyOpenThemeData = [](HWND hWnd, LPCWSTR classList) -> HTHEME {
                    if (wcscmp(classList, L"ScrollBar") == 0)
                    {
                        hWnd = nullptr;
                        classList = L"Explorer::ScrollBar";
                    }
                    //return _FnOpenNcThemeData(hWnd, classList);
                    return OpenNcThemeData(hWnd, classList);
                };
                if (bDarkMode) {
                    addr->u1.Function = reinterpret_cast<ULONG_PTR>(static_cast<decltype(OpenNcThemeData)*>(MyOpenThemeData));
                } else {
                    addr->u1.Function = reinterpret_cast<ULONG_PTR>(OpenNcThemeData);
                }
                VirtualProtect(addr, sizeof(IMAGE_THUNK_DATA), oldProtect, &oldProtect);
            }
        }
        FreeLibrary(hComctl);
    }
}
// ============================================================================


extern "C" void SetDarkMode(bool bEnableDarkMode)
{
    s_UserSetDarkMode = bEnableDarkMode;

    DWORD major, minor;
    DWORD const buildNumber = GetWindowsBuildNumber(&major, &minor);
    if (buildNumber) {
        // undocumented function addresses are only valid for this WinVer build numbers
        if (CheckLoadLibrary(major, minor, buildNumber)) {

            AllowDarkModeForApp(s_UserSetDarkMode);

            RefreshImmersiveColorPolicyState();

            s_bDarkModeSupported = ShouldAppsUseDarkMode() && !IsHighContrast(); // (!) after RefreshImmersiveColorPolicyState()

            FlushMenuThemes();

            FixDarkScrollBar(s_UserSetDarkMode);
        }
    }
}
// ============================================================================

#else // NO DarkMode support

extern "C" void SetDarkMode(bool bEnableDarkMode)
{
    (void)(bEnableDarkMode);
}

#endif

extern "C" void ReleaseDarkMode() { }

// ============================================================================


#if FALSE
//=============================================================================
//
//  OwnerDrawItem() - Handles WM_DRAWITEM
//  https://docs.microsoft.com/en-us/windows/win32/controls/status-bars#owner-drawn-status-bars
//
extern "C" LRESULT OwnerDrawTextItem(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    (void)(hwnd);
    (void)(wParam); // sender control

    const DRAWITEMSTRUCT *const pDIS = (const DRAWITEMSTRUCT *const)lParam;

    //UINT const ctlId = pDIS->CtlID;
    //int const partId = (int)pDIS->itemID;
    //int const stateId = (int)pDIS->itemState;
    //LPCWSTR const data = (LPCWSTR)(pDIS->itemData);

    //~PAINTSTRUCT ps;
    //~BeginPaint(hWndItem, &ps); ~ not needed on WM_DRAWITEM

    HDC const hdc = pDIS->hDC;

#ifdef D_NP3_WIN10_DARK_MODE
    //HTHEME const hTheme = OpenThemeData(hWndSB, L"BUTTON");
    //if (hTheme) {
    SetBkColor(hdc, UseDarkMode() ? Settings2.DarkModeBkgColor : GetSysColor(COLOR_BTNFACE));
    //DrawEdge(hdc, &rc, EDGE_RAISED, BF_RECT);
    //DrawThemeEdge(hTheme, hdc, partId, stateId, &rc, EDGE_RAISED, BF_RECT, NULL);
    SetTextColor(hdc, UseDarkMode() ? Settings2.DarkModeTxtColor : GetSysColor(COLOR_BTNTEXT));
    //  CloseThemeData(hTheme);
    //}
#else
    SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
    SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));
#endif

    WCHAR text[256] = { L'\0' };
    HWND const hWndItem = pDIS->hwndItem;
    int const len = (int)SendMessage(hWndItem, WM_GETTEXT, ARRAYSIZE(text), (LPARAM)text);

    RECT rc = pDIS->rcItem;
    ExtTextOut(hdc, rc.left + 2, rc.top + 2, ETO_OPAQUE | ETO_NUMERICSLOCAL, &rc, text, len, NULL);

    //~EndPaint(hWndItem, &ps);
    return TRUE;
}
// ============================================================================
#endif

#if FALSE
//=============================================================================
//
//  OwnerDrawItem() - Handles WM_DRAWITEM
//  https://docs.microsoft.com/en-us/windows/win32/controls/status-bars#owner-drawn-status-bars
//
extern "C" LRESULT OwnerDrawTextItem(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    (void)(hwnd);
    (void)(wParam); // sender control

    const DRAWITEMSTRUCT *const pDIS = (const DRAWITEMSTRUCT *const)lParam;

    //UINT const ctlId = pDIS->CtlID;
    //LPCWSTR const data = (LPCWSTR)(pDIS->itemData);

    //~PAINTSTRUCT ps;
    //~BeginPaint(hWndItem, &ps); ~ not needed on WM_DRAWITEM

    HWND const hwndButton = pDIS->hwndItem;
    HTHEME const hTheme = OpenThemeData(hwndButton, L"Button");
    if (hTheme) {
        RECT rc;
        GetWindowRect(hwndButton, &rc);

        HDC const hdc = pDIS->hDC;
        int const partId = BS_AUTORADIOBUTTON; // (int)pDIS->partID;
        int const stateId = BST_UNCHECKED;     // (int)pDIS->itemState;
        HRESULT hr = DrawThemeBackground(hTheme, hdc, partId, stateId, &rc, 0);
        RECT rcContent = { 0 };
        if (SUCCEEDED(hr)) {
            hr = GetThemeBackgroundContentRect(hTheme, hdc, partId, stateId, &rc, &rcContent);
        }
        if (SUCCEEDED(hr)) {
            WCHAR szButtonText[255];
            int const len = GetWindowText(hwndButton, szButtonText, ARRAYSIZE(szButtonText));
            hr = DrawThemeText(hTheme, hdc, partId, stateId, szButtonText, len, DT_LEFT | DT_VCENTER | DT_SINGLELINE, 0, &rcContent);
        }
        CloseThemeData(hTheme);
        //~EndPaint(hWndItem, &ps);
        return SUCCEEDED(hr);
    }
    //~EndPaint(hWndItem, &ps);
    return FALSE;
}
// ============================================================================
#endif

