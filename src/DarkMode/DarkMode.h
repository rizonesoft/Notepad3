#pragma once

#ifdef __cplusplus
extern "C" {
#endif

  void SetDarkMode(bool bEnableDarkMode);
  void InitListView(HWND hListView);
  void InitTreeView(HWND hTreeView);
  inline void InitDarkMode() { SetDarkMode(true); };
  void ReleaseDarkMode();

  //LRESULT OwnerDrawTextItem(HWND hwnd, WPARAM wParam, LPARAM lParam);
  DWORD GetWindowsBuildNumber(LPDWORD major, LPDWORD minor);

#ifdef D_NP3_WIN10_DARK_MODE

  bool IsDarkModeSupported();
  bool CheckDarkModeEnabled();
  bool ShouldAppsUseDarkMode();

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
#define GetModeThemeIndex() (UseDarkMode() ? Globals.idxDarkModeTheme : Globals.idxLightModeTheme)

#ifdef __cplusplus
}
#endif
