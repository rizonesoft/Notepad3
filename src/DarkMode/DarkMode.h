#pragma once

#ifdef __cplusplus
extern "C" {
#endif

  extern COLORREF g_rgbDarkBkgColor;
  extern COLORREF g_rgbDarkTextColor;
  extern HBRUSH   g_hbrWndDarkBkgBrush;

  void InitDarkMode(bool bEnableDarkMode);
  void InitListView(HWND hListView);
  void InitTreeView(HWND hTreeView);
  void ReleaseDarkMode();

  //LRESULT OwnerDrawTextItem(HWND hwnd, WPARAM wParam, LPARAM lParam);
  DWORD GetWindowsBuildNumber(LPDWORD major, LPDWORD minor);

#ifdef D_NP3_WIN10_DARK_MODE

  bool IsDarkModeSupported();
  bool CheckDarkModeEnabled();

  void AllowDarkModeForApp(bool allow);
  bool AllowDarkModeForWindow(HWND hWnd, bool allow);

  bool IsHighContrast();
  void RefreshTitleBarThemeColor(HWND hWnd);
  bool IsColorSchemeChangeMessage(LPARAM lParam);
  bool IsColorSchemeChangeMessageEx(UINT message, LPARAM lParam);

  inline INT_PTR SetDarkModeCtlColors(const HDC hdc) {
    SetBkColor(hdc, g_rgbDarkBkgColor);
    SetTextColor(hdc, g_rgbDarkTextColor);
    return (INT_PTR)g_hbrWndDarkBkgBrush;
  }

#else
  inline bool IsDarkModeSupported() { return false; }
  inline bool CheckDarkModeEnabled() { return false; }
#endif

#define UseDarkMode() (IsDarkModeSupported() && CheckDarkModeEnabled())

#ifdef __cplusplus
}
#endif
