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
