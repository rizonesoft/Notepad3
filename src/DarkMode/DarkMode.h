#pragma once
#ifndef _DARKMODE_H_
#define _DARKMODE_H_

#ifdef __cplusplus
extern "C" {
#endif

  void InitDarkMode();
  bool IsDarkModeSupported();
  bool CheckDarkModeEnabled();
  //bool AllowDarkModeForWindow(HWND hWnd, bool allow);
  void AllowDarkModeForApp(bool allow);
  bool IsHighContrast();
  void RefreshTitleBarThemeColor(HWND hWnd);
  bool IsColorSchemeChangeMessage(LPARAM lParam);
  bool IsColorSchemeChangeMessageEx(UINT message, LPARAM lParam);

#ifdef __cplusplus
}
#endif

#endif //_DARKMODE_H_
