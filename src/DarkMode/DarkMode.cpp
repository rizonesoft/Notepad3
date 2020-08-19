#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <CommCtrl.h>
#include <Uxtheme.h>
#include <WindowsX.h>
#include <Dwmapi.h>
#include <Vssym32.h>

#include <climits>
#include <cstdint>

#include "DarkMode.h"
#include "IatHook.hpp"

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

typedef void (WINAPI* fnRtlGetNtVersionNumbers)(LPDWORD major, LPDWORD minor, LPDWORD build);
// 1809 17763
typedef bool   (WINAPI* fnShouldAppsUseDarkMode)(); // ordinal 132
typedef bool   (WINAPI* fnAllowDarkModeForWindow)(HWND hWnd, bool allow); // ordinal 133
typedef bool   (WINAPI* fnAllowDarkModeForApp)(bool allow); // ordinal 135, removed since 18334
typedef void   (WINAPI* fnFlushMenuThemes)(); // ordinal 136
typedef void   (WINAPI* fnRefreshImmersiveColorPolicyState)(); // ordinal 104
typedef bool   (WINAPI* fnIsDarkModeAllowedForWindow)(HWND hWnd); // ordinal 137
typedef bool   (WINAPI* fnGetIsImmersiveColorUsingHighContrast)(IMMERSIVE_HC_CACHE_MODE mode); // ordinal 106
typedef HTHEME(WINAPI* fnOpenNcThemeData)(HWND hWnd, LPCWSTR pszClassList); // ordinal 49
// Insider 18290
typedef bool   (WINAPI* fnShouldSystemUseDarkMode)(); // ordinal 138
// Insider 18334
typedef PreferredAppMode(WINAPI* fnSetPreferredAppMode)(PreferredAppMode appMode); // ordinal 135, since 18334
typedef bool   (WINAPI* fnIsDarkModeAllowedForApp)(); // ordinal 139


fnShouldAppsUseDarkMode _ShouldAppsUseDarkMode = nullptr;
fnAllowDarkModeForWindow _AllowDarkModeForWindow = nullptr;
fnAllowDarkModeForApp _AllowDarkModeForApp = nullptr;
fnFlushMenuThemes _FlushMenuThemes = nullptr;
fnRefreshImmersiveColorPolicyState _RefreshImmersiveColorPolicyState = nullptr;
fnIsDarkModeAllowedForWindow _IsDarkModeAllowedForWindow = nullptr;
fnGetIsImmersiveColorUsingHighContrast _GetIsImmersiveColorUsingHighContrast = nullptr;
fnOpenNcThemeData _OpenNcThemeData = nullptr;
// Insider 18290
fnShouldSystemUseDarkMode _ShouldSystemUseDarkMode = nullptr;
// Insider 18334
fnSetPreferredAppMode _SetPreferredAppMode = nullptr;

static bool s_darkModeSupported = false;
static bool s_darkModeEnabled = false;
static DWORD s_buildNumber = 0;

// ============================================================================

extern "C" bool IsDarkModeSupported()
{
  return s_darkModeSupported;
}
// ============================================================================


extern "C" bool CheckDarkModeEnabled()
{
  s_darkModeEnabled = _ShouldAppsUseDarkMode() && !IsHighContrast();
  return s_darkModeEnabled;
}
// ============================================================================


extern "C" bool AllowDarkModeForWindow(HWND hWnd, bool allow)
{
  return s_darkModeSupported ? _AllowDarkModeForWindow(hWnd, allow) : false;
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
    !IsHighContrast())
  {
    dark = TRUE;
  }
  DwmSetWindowAttribute(hWnd, 19, &dark, sizeof(dark));
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


static void _FixDarkScrollBar()
{
  auto const addr = FindDelayLoadThunkInModule(GetModuleHandleW(L"comctl32.dll"), "uxtheme.dll", 49); // OpenNcThemeData
  if (addr)
  {
    DWORD oldProtect;
    if (VirtualProtect(addr, sizeof(IMAGE_THUNK_DATA), PAGE_READWRITE, &oldProtect))
    {
      auto MyOpenThemeData = [](HWND hWnd, LPCWSTR classList) -> HTHEME {
        if (wcscmp(classList, L"ScrollBar") == 0)
        {
          hWnd = nullptr;
          classList = L"Explorer::ScrollBar";
        }
        return _OpenNcThemeData(hWnd, classList);
      };

      addr->u1.Function = reinterpret_cast<ULONG_PTR>(static_cast<fnOpenNcThemeData>(MyOpenThemeData));
      VirtualProtect(addr, sizeof(IMAGE_THUNK_DATA), oldProtect, &oldProtect);
    }
  }
}
// ============================================================================


extern "C" void InitDarkMode()
{
  fnRtlGetNtVersionNumbers RtlGetNtVersionNumbers = reinterpret_cast<fnRtlGetNtVersionNumbers>(GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetNtVersionNumbers"));
  if (RtlGetNtVersionNumbers)
  {
    DWORD major, minor;
    RtlGetNtVersionNumbers(&major, &minor, &s_buildNumber);
    s_buildNumber &= ~0xF0000000;
    if (major == 10 && minor == 0 && 17763 <= s_buildNumber && s_buildNumber <= 18362) // Windows 10 1809 10.0.17763 - 1903 10.0.18362
    {
      HMODULE hUxtheme = LoadLibraryExW(L"uxtheme.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
      if (hUxtheme)
      {
        _OpenNcThemeData = reinterpret_cast<fnOpenNcThemeData>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(49)));
        _RefreshImmersiveColorPolicyState = reinterpret_cast<fnRefreshImmersiveColorPolicyState>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(104)));
        _GetIsImmersiveColorUsingHighContrast = reinterpret_cast<fnGetIsImmersiveColorUsingHighContrast>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(106)));
        _ShouldAppsUseDarkMode = reinterpret_cast<fnShouldAppsUseDarkMode>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(132)));
        _AllowDarkModeForWindow = reinterpret_cast<fnAllowDarkModeForWindow>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(133)));

        auto ord135 = GetProcAddress(hUxtheme, MAKEINTRESOURCEA(135));
        if (s_buildNumber < 18334)
          _AllowDarkModeForApp = reinterpret_cast<fnAllowDarkModeForApp>(ord135);
        else
          _SetPreferredAppMode = reinterpret_cast<fnSetPreferredAppMode>(ord135);

        //_FlushMenuThemes = reinterpret_cast<fnFlushMenuThemes>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(136)));
        _IsDarkModeAllowedForWindow = reinterpret_cast<fnIsDarkModeAllowedForWindow>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(137)));

        if (_OpenNcThemeData &&
          _RefreshImmersiveColorPolicyState &&
          _ShouldAppsUseDarkMode &&
          _AllowDarkModeForWindow &&
          (_AllowDarkModeForApp || _SetPreferredAppMode) &&
          //_FlushMenuThemes &&
          _IsDarkModeAllowedForWindow)
        {
          s_darkModeSupported = true;

          AllowDarkModeForApp(true);

          _RefreshImmersiveColorPolicyState();

          s_darkModeEnabled = _ShouldAppsUseDarkMode() && !IsHighContrast();

          _FixDarkScrollBar();
        }
      }
    }
  }
}
// ============================================================================

