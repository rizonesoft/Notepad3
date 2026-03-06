# Windows 10 Minimum Version Upgrade

**Date**: March 2026
**Branch**: `master_Win10`
**Minimum OS**: Windows 10 version 1809 (build 17763)

---

## Summary

Notepad3's minimum supported Windows version has been raised from Windows 7 to Windows 10.
This change enables the use of modern Win32 APIs, removes legacy compatibility code, and simplifies the codebase.

---

## Changes Made

### PR #5565 (RaiKoHoff)

- Updated `_WIN32_WINNT`, `WINVER`, `NTDDI_VERSION` defines to `_WIN32_WINNT_WIN10` / `NTDDI_WIN10_RS5` across all `.vcxproj` files and source headers
- Added `#include <sdkddkver.h>` for symbolic version constants
- Removed `IsWindowsVistaOrGreater()` and `IsWindows7SP1OrGreater()` dead code paths in `Notepad3.c`
- Simplified `GetWinVersionString()` by removing Win7/Win8/Win8.1 branches
- Removed `IsWindowsVistaOrGreater()` guard from `IsProcessElevated()` in `Helpers.c`
- Cleaned manifest `supportedOS` GUIDs in `Notepad3.exe.manifest` and `MiniPath.exe.manifest`
- Removed `#if _WIN32_WINNT < _WIN32_WINNT_WIN8` conditional in `DarkMode.cpp`
- Updated all language `resource.h` files
- Bumped major version in `Version.ps1`

### Follow-up Commits (master_Win10)

- **Manifest cleanup**: Removed Vista/Win7/Win8/Win8.1 `supportedOS` GUIDs from `grepWinNP3/src/compatibility.manifest`
- **Dead code removal**: Removed 3 `IsWindowsXPSP3OrGreater()` checks from toolbar logic in `Notepad3.c`
- **Compile-time conditionals**: Removed ~120 lines of `#if (NTDDI_VERSION < NTDDI_WIN8)` fallback code in URL escape/unescape functions (`Helpers.c`)
- **API modernization**: Upgraded `GetTickCount()` to `GetTickCount64()` across `Notepad3.c` and `Helpers.c` (avoids 49.7-day DWORD rollover)
- **Legacy code cleanup**: Removed disabled `_GetTrueWindowsVersion()` function (used deprecated `GetVersionEx`), removed commented-out `ChangeWindowMessageFilter` and `SetProcessDpiAwarenessContext` calls
- **Build infrastructure**: Updated `Build.ps1` vswhere version range from `[17.0,18.0)` to `[17.0,19.0)` for VS2026 support
- **Version bump**: `$Major` changed from `6` to `7` in `build.yml`

---

## Build Verification

- ✅ VS2026 (v145, MSBuild v18.3.2) — x64 Release: **PASS**
- ✅ VS2022 (v143) — verified compatible before changes

---

## Remaining Evaluation Items

These items are optional modernizations noted for future consideration:

- `RtlAreLongPathsEnabled` in `PathLib.c` — currently dynamically loaded; could link ntdll.lib directly on Win10+
- `PathCch*` APIs — commented-out `#include <pathcch.h>` could be enabled to replace custom path functions
- `GetSystemMetrics()` → `GetSystemMetricsForDpi()` — several call sites could benefit from DPI-aware metrics
- Test file `test/test_files/StyleLexers/styleLexCPP/Config.cpp` still has old Win7 defines (syntax highlighting test data — not compiled)
