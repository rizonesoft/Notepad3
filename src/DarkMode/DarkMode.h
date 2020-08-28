#pragma once

#ifdef __cplusplus
extern "C" {
#endif

  void InitDarkMode();
  bool IsDarkModeSupported();
  bool CheckDarkModeEnabled();
  DWORD GetWindowsBuildNumber();

  void AllowDarkModeForApp(bool allow);
  bool AllowDarkModeForWindow(HWND hWnd, bool allow);

  bool IsHighContrast();
  void RefreshTitleBarThemeColor(HWND hWnd);
  bool IsColorSchemeChangeMessage(LPARAM lParam);
  bool IsColorSchemeChangeMessageEx(UINT message, LPARAM lParam);

#ifdef __cplusplus
}
#endif
