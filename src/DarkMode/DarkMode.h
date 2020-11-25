#pragma once

#include <Windows.h>

#pragma comment(lib, "NtDll.lib")
#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Uxtheme.lib")

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

void SetDarkMode(bool bEnableDarkMode);
inline void InitDarkMode()
{
    SetDarkMode(true);
};
void ReleaseDarkMode();

void InitListView(HWND hListView);
void InitTreeView(HWND hTreeView);

//LRESULT OwnerDrawTextItem(HWND hwnd, WPARAM wParam, LPARAM lParam);
DWORD GetWindowsBuildNumber(LPDWORD major, LPDWORD minor);

#ifdef D_NP3_WIN10_DARK_MODE

bool IsDarkModeSupported();
bool CheckDarkModeEnabled();
bool ShouldAppsUseDarkModeEx();

void AllowDarkModeForAppEx(bool allow);
bool AllowDarkModeForWindowEx(HWND hWnd, bool allow);

bool IsHighContrast();
void RefreshTitleBarThemeColor(HWND hWnd);
bool IsColorSchemeChangeMessage(LPARAM lParam);
bool IsColorSchemeChangeMessageMsg(UINT message, LPARAM lParam);

#define  CASE_WM_CTLCOLOR_SET       \
           case WM_CTLCOLORDLG:     \
           case WM_CTLCOLORBTN:     \
           case WM_CTLCOLOREDIT:    \
           case WM_CTLCOLORLISTBOX: \
           case WM_CTLCOLORSTATIC

#else
inline bool IsDarkModeSupported()
{
    return false;
}
inline bool CheckDarkModeEnabled()
{
    return false;
}
#endif

#define UseDarkMode() (IsDarkModeSupported() && CheckDarkModeEnabled())
#define GetModeThemeIndex() (UseDarkMode() ? Globals.idxDarkModeTheme : Globals.idxLightModeTheme)

#ifdef __cplusplus
}
#endif
