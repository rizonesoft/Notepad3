#pragma once

#ifdef __cplusplus
extern "C" {
#endif

  DWORD GetWindowsBuildNumber(LPDWORD major, LPDWORD minor);
  
  void InitListView(HWND hListView);

#ifdef D_NP3_WIN10_DARK_MODE

  void InitDarkMode();

  bool IsDarkModeSupported();
  bool CheckDarkModeEnabled();

  void AllowDarkModeForApp(bool allow);
  bool AllowDarkModeForWindow(HWND hWnd, bool allow);

  bool IsHighContrast();
  void RefreshTitleBarThemeColor(HWND hWnd);
  bool IsColorSchemeChangeMessage(LPARAM lParam);
  bool IsColorSchemeChangeMessageEx(UINT message, LPARAM lParam);

#else
  inline bool IsDarkModeSupported() { return false; }
  inline bool CheckDarkModeEnabled() { return false; }
#endif

#define UseDarkMode() (IsDarkModeSupported() && CheckDarkModeEnabled())

#ifdef __cplusplus
}
#endif
