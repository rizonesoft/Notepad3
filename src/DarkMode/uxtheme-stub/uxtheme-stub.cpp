// ----------------------------------------------------------------------------
// Windows 10 1903, aka 18362, broke the API.
// The function at ordinal 135 is AllowDarkModeForApp before,
// and SetPreferredAppMode after 1903. It also broke ShouldAppsUseDarkMode,
// which always return TRUE(actually, random runtime not zero) after 1903.
//
// Before 1903, BOOL __stdcall AllowDarkModeForApp(BOOL) only accepts TRUE or FALSE.
// TRUE means dark mode is allowed and vice versa.
// After 1903, PreferredMode __stdcall SetPreferredAppMode(PreferredMode) accepts
// 4 valid value : Default, AllowDark, ForceDark, ForceLight.
//
extern "C" {
	// 1809 17763
	void __stdcall ShouldAppsUseDarkMode() {} // ordinal 132
	void __stdcall AllowDarkModeForWindow(int, int) {} // ordinal 133
	void __stdcall AllowDarkModeForApp(int) {} // ordinal 135, in 1809
	void __stdcall FlushMenuThemes() {} // ordinal 136
	void __stdcall RefreshImmersiveColorPolicyState() {} // ordinal 104
	void __stdcall IsDarkModeAllowedForWindow(int) {} // ordinal 137
	void __stdcall GetIsImmersiveColorUsingHighContrast(int) {} // ordinal 106
	void __stdcall OpenNcThemeData(int, int) {} // ordinal 49

	// 1903 18362
	void __stdcall ShouldSystemUseDarkMode() {} // ordinal 138
	//void __stdcall SetPreferredAppMode(int) {} // ordinal 135, in 1903
	void __stdcall IsDarkModeAllowedForApp() {} // ordinal 139
}
