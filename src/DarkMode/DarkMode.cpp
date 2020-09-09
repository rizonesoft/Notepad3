
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#ifdef D_NP3_WIN10_DARK_MODE
#include <CommCtrl.h>
#include <Uxtheme.h>
#include <WindowsX.h>
#include <Dwmapi.h>
#include <Vssym32.h>
#include <climits>
#endif

#include <cstdint>

#include "TypeDefs.h"

#include "DarkMode.h"
#include "IatHook.hpp"
#include "ListViewUtil.hpp"


// ============================================================================

using fnRtlGetNtVersionNumbers = void(WINAPI *)(LPDWORD major, LPDWORD minor, LPDWORD build);

extern "C" DWORD GetWindowsBuildNumber(LPDWORD major, LPDWORD minor)
{
  static DWORD _dwWindowsBuildNumber = 0;
  static DWORD _major = 0, _minor = 0;
  if (!_dwWindowsBuildNumber) {
    HMODULE const hNTDLL = GetModuleHandle(L"ntdll.dll");
    if (hNTDLL) {
      fnRtlGetNtVersionNumbers RtlGetNtVersionNumbers = reinterpret_cast<fnRtlGetNtVersionNumbers>(GetProcAddress(hNTDLL, "RtlGetNtVersionNumbers"));
      if (RtlGetNtVersionNumbers) {
        RtlGetNtVersionNumbers(&_major, &_minor, &_dwWindowsBuildNumber);
        _dwWindowsBuildNumber &= ~0xF0000000;
      }
    }
  }
  if (major) { *major = _major; }
  if (minor) { *minor = _minor; }
  return _dwWindowsBuildNumber;
}


#ifdef D_NP3_WIN10_DARK_MODE

#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Uxtheme.lib")
#pragma comment(lib, "Dwmapi.lib")

enum IMMERSIVE_HC_CACHE_MODE
{
  IHCM_USE_CACHED_VALUE,
  IHCM_REFRESH
};
typedef enum IMMERSIVE_HC_CACHE_MODE IMMERSIVE_HC_CACHE_MODE;

// Insider 18334
enum PreferredAppMode
{
  Default,
  AllowDark,
  ForceDark,
  ForceLight,
  Max
};
typedef enum PreferredAppMode PreferredAppMode;


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


struct WINDOWCOMPOSITIONATTRIBDATA {
  WINDOWCOMPOSITIONATTRIB Attrib;
  PVOID pvData;
  SIZE_T cbData;
};

using fnSetWindowCompositionAttribute = BOOL(WINAPI *)(HWND hWnd, WINDOWCOMPOSITIONATTRIBDATA *);
// 1809 17763
using fnShouldAppsUseDarkMode = bool(WINAPI *)();                                            // ordinal 132
using fnAllowDarkModeForWindow = bool(WINAPI *)(HWND hWnd, bool allow);                      // ordinal 133
using fnAllowDarkModeForApp = bool(WINAPI *)(bool allow);                                    // ordinal 135, in 1809
using fnFlushMenuThemes = void(WINAPI *)();                                                  // ordinal 136
using fnRefreshImmersiveColorPolicyState = void(WINAPI *)();                                 // ordinal 104
using fnIsDarkModeAllowedForWindow = bool(WINAPI *)(HWND hWnd);                              // ordinal 137
using fnGetIsImmersiveColorUsingHighContrast = bool(WINAPI *)(IMMERSIVE_HC_CACHE_MODE mode); // ordinal 106
using fnOpenNcThemeData = HTHEME(WINAPI *)(HWND hWnd, LPCWSTR pszClassList);                 // ordinal 49
// 1903 18362
using fnShouldSystemUseDarkMode = bool(WINAPI *)();                                 // ordinal 138
using fnSetPreferredAppMode = PreferredAppMode(WINAPI *)(PreferredAppMode appMode); // ordinal 135, in 1903
using fnIsDarkModeAllowedForApp = bool(WINAPI *)();                                 // ordinal 139

fnSetWindowCompositionAttribute _SetWindowCompositionAttribute = nullptr;
fnShouldAppsUseDarkMode _ShouldAppsUseDarkMode = nullptr;
fnAllowDarkModeForWindow _AllowDarkModeForWindow = nullptr;
fnAllowDarkModeForApp _AllowDarkModeForApp = nullptr;
fnFlushMenuThemes _FlushMenuThemes = nullptr;
fnRefreshImmersiveColorPolicyState _RefreshImmersiveColorPolicyState = nullptr;
fnIsDarkModeAllowedForWindow _IsDarkModeAllowedForWindow = nullptr;
fnIsDarkModeAllowedForApp _IsDarkModeAllowedForApp = nullptr;
fnGetIsImmersiveColorUsingHighContrast _GetIsImmersiveColorUsingHighContrast = nullptr;
fnOpenNcThemeData _OpenNcThemeData = nullptr;
// 1903 18362
fnShouldSystemUseDarkMode _ShouldSystemUseDarkMode = nullptr;
fnSetPreferredAppMode _SetPreferredAppMode = nullptr;

// ============================================================================


static bool _bDarkModeSupported = false;

extern "C" bool IsDarkModeSupported() {
  return _bDarkModeSupported;
}
// ============================================================================


static bool _UserSetDarkMode = false;

extern "C" bool CheckDarkModeEnabled() {
  return _UserSetDarkMode && _bDarkModeSupported && !IsHighContrast();
}
// ============================================================================


extern "C" bool ShouldAppsUseDarkMode() {
  if (_ShouldAppsUseDarkMode) {
    return _ShouldAppsUseDarkMode() && !IsHighContrast();
  }
  return false;
}
// ============================================================================


extern "C" bool AllowDarkModeForWindow(HWND hWnd, bool allow)
{
  return _bDarkModeSupported ? _AllowDarkModeForWindow(hWnd, allow) : false;
}
// ============================================================================


extern "C" bool IsHighContrast()
{
  HIGHCONTRASTW highContrast = { sizeof(highContrast) };

  return SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(highContrast), &highContrast, FALSE) ? 
                               (highContrast.dwFlags & HCF_HIGHCONTRASTON) : false;
}
// ============================================================================


extern "C" void RefreshTitleBarThemeColor(HWND hWnd)
{
  BOOL dark = FALSE;
  if (_IsDarkModeAllowedForWindow(hWnd) &&
      _ShouldAppsUseDarkMode() &&
      !IsHighContrast()) {
    dark = TRUE;
  }
  DWORD const buildNum = GetWindowsBuildNumber(nullptr, nullptr);
  if (buildNum < 18362) {
    SetPropW(hWnd, L"UseImmersiveDarkModeColors", reinterpret_cast<HANDLE>(static_cast<INT_PTR>(dark)));
  }
  else if (_SetWindowCompositionAttribute) {
    WINDOWCOMPOSITIONATTRIBDATA data = { WCA_USEDARKMODECOLORS, &dark, sizeof(dark) };
    _SetWindowCompositionAttribute(hWnd, &data);
  }
}
// ============================================================================


extern "C" bool IsColorSchemeChangeMessage(LPARAM lParam)
{
  bool is = false;
  if (lParam && CompareStringOrdinal(reinterpret_cast<LPCWCH>(lParam), -1, L"ImmersiveColorSet", -1, TRUE) == CSTR_EQUAL)
  {
    _RefreshImmersiveColorPolicyState();
    is = true;
  }
  _GetIsImmersiveColorUsingHighContrast(IHCM_REFRESH);
  return is;
}
// ============================================================================


extern "C" bool IsColorSchemeChangeMessageEx(UINT message, LPARAM lParam)
{
  return (message == WM_SETTINGCHANGE) ? IsColorSchemeChangeMessage(lParam) : false;
}
// ============================================================================


extern "C" void AllowDarkModeForApp(bool allow)
{
  if (_AllowDarkModeForApp) {
    _AllowDarkModeForApp(allow);
  }
  else if (_SetPreferredAppMode) {
    _SetPreferredAppMode(allow ? AllowDark : Default);
  }
}
// ============================================================================


static void _FixDarkScrollBar(bool bDarkMode)
{
  HMODULE hComctl = LoadLibraryExW(L"comctl32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
  if (hComctl) {
    auto addr = FindDelayLoadThunkInModule(hComctl, "uxtheme.dll", 49); // OpenNcThemeData
    if (addr) {
      DWORD oldProtect;
      if (VirtualProtect(addr, sizeof(IMAGE_THUNK_DATA), PAGE_READWRITE, &oldProtect)) {
        auto MyOpenThemeData = [](HWND hWnd, LPCWSTR classList) -> HTHEME {
          if (wcscmp(classList, L"ScrollBar") == 0) {
            hWnd = nullptr;
            classList = L"Explorer::ScrollBar";
          }
          return _OpenNcThemeData(hWnd, classList);
        };
        if (bDarkMode)
          addr->u1.Function = reinterpret_cast<ULONG_PTR>(static_cast<fnOpenNcThemeData>(MyOpenThemeData));
        else
          addr->u1.Function = reinterpret_cast<ULONG_PTR>(_OpenNcThemeData);
        VirtualProtect(addr, sizeof(IMAGE_THUNK_DATA), oldProtect, &oldProtect);
      }
    }
  }
}
// ============================================================================

constexpr bool CheckBuildNumber(DWORD buildNumber) {
  return (buildNumber == 17763 || // 1809
          buildNumber == 18362 || // 1903
          buildNumber == 18363 || // 1909
          buildNumber == 19041);  // 2004
}

extern "C" void SetDarkMode(bool bEnableDarkMode)
{
  DWORD major, minor;
  DWORD const buildNumber = GetWindowsBuildNumber(&major, &minor);
  if (buildNumber) {
    // undocumented function addresses are only valid for this WinVer build numbers
    if (major == 10 && minor == 0 && CheckBuildNumber(buildNumber))
    {
      HMODULE const hUxtheme = LoadLibraryExW(L"uxtheme.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
      if (hUxtheme)
      {
        if (!_OpenNcThemeData) {
          _OpenNcThemeData = reinterpret_cast<fnOpenNcThemeData>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(49)));
        }
        if (!_RefreshImmersiveColorPolicyState) {
          _RefreshImmersiveColorPolicyState = reinterpret_cast<fnRefreshImmersiveColorPolicyState>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(104)));
        }
        if (!_GetIsImmersiveColorUsingHighContrast) {
          _GetIsImmersiveColorUsingHighContrast = reinterpret_cast<fnGetIsImmersiveColorUsingHighContrast>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(106)));
        }
        if (!_ShouldAppsUseDarkMode) {
          _ShouldAppsUseDarkMode = reinterpret_cast<fnShouldAppsUseDarkMode>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(132)));
        }
        if (!_AllowDarkModeForWindow) {
          _AllowDarkModeForWindow = reinterpret_cast<fnAllowDarkModeForWindow>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(133)));
        }

        auto const ord135 = GetProcAddress(hUxtheme, MAKEINTRESOURCEA(135));
        if (buildNumber < 18334) {
          if (!_AllowDarkModeForApp) {
            _AllowDarkModeForApp = reinterpret_cast<fnAllowDarkModeForApp>(ord135);
          }
        } 
        else {
          if (!_SetPreferredAppMode) {
            _SetPreferredAppMode = reinterpret_cast<fnSetPreferredAppMode>(ord135);
          }
        } 

        if (!_FlushMenuThemes) {
          _FlushMenuThemes = reinterpret_cast<fnFlushMenuThemes>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(136)));
        }
        if (!_IsDarkModeAllowedForWindow) {
          _IsDarkModeAllowedForWindow = reinterpret_cast<fnIsDarkModeAllowedForWindow>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(137)));
        }
        if (!_IsDarkModeAllowedForApp) {
          _IsDarkModeAllowedForApp = reinterpret_cast<fnIsDarkModeAllowedForApp>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(139)));
        }
        if (!_SetWindowCompositionAttribute) {
          HMODULE const hModuleUSR32DLL = GetModuleHandleW(L"user32.dll");
          if (hModuleUSR32DLL) {
            _SetWindowCompositionAttribute = reinterpret_cast<fnSetWindowCompositionAttribute>(GetProcAddress(hModuleUSR32DLL, "SetWindowCompositionAttribute"));
          }
        }

        _UserSetDarkMode = bEnableDarkMode;

        if (_OpenNcThemeData &&
          _RefreshImmersiveColorPolicyState &&
          _ShouldAppsUseDarkMode &&
          _AllowDarkModeForWindow &&
          (_AllowDarkModeForApp || _SetPreferredAppMode) &&
          _FlushMenuThemes &&
          _IsDarkModeAllowedForWindow)
        {
          AllowDarkModeForApp(_UserSetDarkMode);
          _RefreshImmersiveColorPolicyState();
          _bDarkModeSupported = _ShouldAppsUseDarkMode() && !IsHighContrast(); // (!) after _RefreshImmersiveColorPolicyState()
          _FlushMenuThemes();
          _FixDarkScrollBar(_UserSetDarkMode);
        }
      }
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
extern "C" LRESULT OwnerDrawTextItem(HWND hwnd, WPARAM wParam, LPARAM lParam) {
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
extern "C" LRESULT OwnerDrawTextItem(HWND hwnd, WPARAM wParam, LPARAM lParam) {
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

