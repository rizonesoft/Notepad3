# Windows 10 Minimum Version Upgrade – Actionable Checklist

> **Goal**: Raise the minimum supported OS from Windows 7 (`0x0601`) to Windows 10 (`0x0A00`).
> Verified: VS2026 (v145) and VS2022 (v143) both build clean before changes.
>
> **Strategy**: Implement each section → build with VS2026 → commit & push → move on.

---

## 1. Version Defines – vcxproj Files

All preprocessor definitions in every `<PreprocessorDefinitions>` block must change:

| Old                        | New                        |
| -------------------------- | -------------------------- |
| `_WIN32_WINNT=0x601`       | `_WIN32_WINNT=0x0A00`      |
| `WINVER=0x601`             | `WINVER=0x0A00`            |
| `NTDDI_VERSION=0x06010000` | `NTDDI_VERSION=0x0A000000` |

### Notepad3 Core

- [ ] `src/Notepad3.vcxproj` – 9 config blocks (lines 249, 324, 398, 469, 555, 641, 726, 811, 896)

### Scintilla

- [ ] `scintilla/Scintilla.vcxproj` – 12 config blocks
- [ ] `scintilla/ScintillaDLL.vcxproj` – 4 config blocks

### Lexilla

- [ ] `lexilla/Lexilla.vcxproj` – 9 config blocks (lines 325, 361, 404, 445, 479, 516, 558, 600, 643)

### Other

- [ ] `other_sln/Notepad3DLL.vcxproj` – 4 config blocks (lines 112, 176, 236, 304)

> 🔨 **GATE**: Build with VS2026 → `git commit -m "W10: vcxproj version defines"` → `git push`

---

## 2. Version Defines – Source File Headers

Each file has `#ifndef`/`#define` guards for `_WIN32_WINNT`, `WINVER`, `NTDDI_VERSION`. Update `0x0601` → `0x0A00` and `0x06010000` → `0x0A000000`. Fix comments from `/*_WIN32_WINNT_WIN7*/` → `/*_WIN32_WINNT_WIN10*/` (and `/*NTDDI_WIN7*/` → `/*NTDDI_WIN10*/`).

### Primary headers (affect everything downstream)

- [ ] `src/TypeDefs.h` lines 16–24 – update defines and comments
- [ ] `src/TypeDefs.h` lines 26–33 – **remove** the `#if 0` block (was the future Win10 override, now redundant)
- [ ] `src/resource.h` lines 5–9 – update defines and comments

### Individual source files

- [ ] `src/EncodingDetection.cpp` lines 16–23
- [ ] `src/Print.cpp` lines 27–34
- [ ] `src/PathLib.c` lines 122–129
- [ ] `src/Config/Config.cpp` lines 15–22
- [ ] `src/crypto/crypto.c` lines 15–22
- [ ] `src/crypto/rijndael-api-fst.c` lines 44–51
- [ ] `src/ChooseFont/ChooseFont.h` lines 19–20 (also update comment "Minimum platform is Windows 7" → "Windows 10")

### Backlog / Test files (lower priority)

- [ ] `src/_backlog/AccelKeys.c` lines 21–28
- [ ] `test/test_files/StyleLexers/styleLexCPP/Config.cpp` lines 41–48

> 🔨 **GATE**: Build with VS2026 → `git commit -m "W10: source file header defines"` → `git push`

---

## 3. Application Manifests – Clean Up Legacy OS GUIDs

The manifest `supportedOS` entries for OSes older than Win10 are no longer needed.

### GUIDs to remove (Vista, Win7, Win8, Win8.1):

| GUID                                     | OS     |
| ---------------------------------------- | ------ |
| `{e2011457-1546-43c5-a5fe-008deee3d3f0}` | Vista  |
| `{35138b9a-5d96-4fbd-8e2d-a2440225f93a}` | Win7   |
| `{4a2f28e3-53b9-4441-ba9c-d69d4a4a6e38}` | Win8   |
| `{1f676c76-80e1-4239-95bb-83d0f6d0da78}` | Win8.1 |

### Keep only:

| GUID                                     | OS            |
| ---------------------------------------- | ------------- |
| `{8e0f7a12-bfb3-4fe8-b9a5-48fd50a15a9a}` | Win10 / Win11 |

### Files to update:

- [ ] `res/Notepad3.exe.manifest` – remove lines 28–31, keep line 32
- [ ] `minipath/res/MiniPath.exe.manifest` – remove Vista/Win7/Win8/Win8.1 entries
- [ ] `grepWinNP3/src/compatibility.manifest` – remove Vista/Win7/Win8/Win8.1 entries

> 🔨 **GATE**: Build with VS2026 → `git commit -m "W10: manifest cleanup"` → `git push`

---

## 4. Runtime Version Checks – Dead Code Removal

With Win10 as minimum, checks for Vista/Win7/Win8 are always true and can be simplified.

### `src/Notepad3.c`

- [ ] Line 688 – `IsWindowsXPSP3OrGreater() ? 1 : 2` → always `1` (remove ternary)
- [ ] Lines 1010–1014 – Change `IsWindows7SP1OrGreater()` guard to `IsWindows10OrGreater()`
- [ ] Lines 2829–2839 – Remove `if (!IsWindowsVistaOrGreater())` block entirely (dead code)
- [ ] Line 3311 – Remove `!IsWindowsXPSP3OrGreater() &&` from condition (always false)
- [ ] Line 3314 – Remove `if (bProcessed && !IsWindowsXPSP3OrGreater())` block (dead code)
- [ ] Line 12112 – Remove `!IsWindowsVistaOrGreater() ||` from condition (always false)

### `src/Helpers.c`

- [ ] Lines 355–361 – `IsProcessElevated()`: Remove `if (!IsWindowsVistaOrGreater()) return false;` (dead code, Vista+ is guaranteed)
- [ ] Lines 225–251 – `GetWinVersionString()`: Remove Win7/Win8/Win8.1 branches (lines 234–244), keep only Win10+ path

> 🔨 **GATE**: Build with VS2026 → `git commit -m "W10: remove dead version checks"` → `git push`

---

## 5. Compile-Time Conditionals – Simplified by NTDDI_WIN10

With `NTDDI_VERSION=0x0A000000`, conditions gated on `NTDDI_WIN8` are always true.

### `src/Helpers.c` – URL Escape Functions

- [ ] Lines 1986–1999 – Remove `#if (NTDDI_VERSION < NTDDI_WIN8)` legacy URL char tables (dead code)
- [ ] Lines 2003–2049 – `UrlEscapeEx()`: Remove `#else` branch, keep only the `NTDDI_WIN8+` path (`UrlEscape` with `URL_ESCAPE_URI_COMPONENT`)
- [ ] Lines 2057–2100 – `UrlUnescapeEx()`: Remove `#else` branch, keep only the `NTDDI_WIN8+` path (`UrlUnescape` with `URL_UNESCAPE_AS_UTF8`)

### `src/DarkMode/DarkMode.cpp`

- [ ] Lines 58–69 – Remove `#if _WIN32_WINNT < _WIN32_WINNT_WIN8` block, keep only `#define kSystemLibraryLoadFlags LOAD_LIBRARY_SEARCH_SYSTEM32`
- [ ] Line 64 – Remove `GetProcAddress(GetModuleHandle("kernel32.dll"), "SetDefaultDllDirectories")` runtime check (`SetDefaultDllDirectories` is guaranteed on Win8+ / Win10)

> 🔨 **GATE**: Build with VS2026 → `git commit -m "W10: simplify compile-time conditionals"` → `git push`

---

## 6. API Modernizations – Win10 Guaranteed APIs

### `GetTickCount()` → `GetTickCount64()` (avoids 49.7-day rollover)

- [ ] `src/Notepad3.c` line 179 – `s_dwAutoScrollStartTick` type: `DWORD` → `ULONGLONG`
- [ ] `src/Notepad3.c` line 2482 – `GetTickCount()` → `GetTickCount64()`
- [ ] `src/Notepad3.c` line 2491 – `GetTickCount()` → `GetTickCount64()`
- [ ] `src/Helpers.c` line 615 – `GetTickCount()` → `GetTickCount64()` in `BackgroundWorker_Cancel()`
- [ ] `src/Helpers.c` line 617 – `GetTickCount()` → `GetTickCount64()` comparison

### `RtlAreLongPathsEnabled` – Remove Dynamic Loading

- [ ] `src/PathLib.c` lines 284–313 – `HasOptInToRemoveMaxPathLimit()`: Replace `LoadLibrary("ntdll.dll")` + `GetProcAddress` with direct ntdll linkage. `RtlAreLongPathsEnabled` is always present on Win10 1607+ (consider: just declare `extern` and link ntdll.lib, or keep dynamic but note it's guaranteed)

### `PathCch*` APIs – Can Now Be Used (Win8.1+)

- [ ] `src/PathLib.c` lines 154–157 – Evaluate uncommenting `#include <pathcch.h>` and `#pragma comment(linker, "/defaultlib:Pathcch")` to use modern PathCch APIs directly instead of custom implementations
- [ ] `src/PathLib.c` – Evaluate replacing `_PathCanonicalize()` custom impl with `PathCchCanonicalize()` / `PathCchCanonicalizeEx()`
- [ ] `src/PathLib.c` – Evaluate replacing `_Path_IsRelative()` MAX_PATH hack with `PathIsRelativeW()` or `PathCchIsRelative()` if long path mode is active

### `GetSystemMetrics()` → `GetSystemMetricsForDpi()` (Win10 1607+)

These calls don't use DPI-aware variants and may return incorrect sizes on multi-DPI setups:

- [ ] `src/Notepad3.c` lines 1106–1110 – Icon size queries during init (use `Scintilla_GetSystemMetricsForDpi` or `GetSystemMetricsForDpi` directly)
- [ ] `src/Notepad3.c` line 3706 – `SM_CYFRAME` query
- [ ] `src/Notepad3.c` line 12181 – `SM_CXSMICON`/`SM_CYSMICON` in `RelaunchElevated()`
- [ ] `src/Config/Config.cpp` lines 1862–1863, 2263–2264, 2315–2316 – Virtual screen size queries (evaluate whether DPI-aware version is needed for these specific metrics)

> 🔨 **GATE**: Build with VS2026 → `git commit -m "W10: API modernizations"` → `git push`

---

## 7. Commented-Out / Disabled Legacy Code Cleanup

- [ ] `src/Helpers.c` lines 184–222 – `_GetTrueWindowsVersion()` inside `#if 0`: Remove entire block (uses deprecated `GetVersionEx`)
- [ ] `src/Notepad3.c` lines 1251–1253 – Remove commented-out `ChangeWindowMessageFilter` calls (superseded by `ChangeWindowMessageFilterEx` since Win7)
- [ ] `src/Notepad3.c` lines 1016–1017 – Evaluate enabling `SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)` (currently commented out; manifest already declares `PerMonitorV2` so this may be redundant — confirm and remove or enable)
- [ ] `src/Notepad3.c` line 1091 – Evaluate `SetThreadDpiAwarenessContext` call (same consideration as above)
- [ ] `src/PathLib.c` line 67 – Update TODO comment for `IsWindows10OrGreater()` check (now always true)

> 🔨 **GATE**: Build with VS2026 → `git commit -m "W10: legacy code cleanup"` → `git push`

---

## 8. Build Infrastructure

### Build.ps1 – VS2026 Support

- [ ] `Build/scripts/Build.ps1` line 26 – Extend vswhere version range from `[17.0,18.0)` to `[17.0,19.0)` to find both VS2022 and VS2026

> 🔨 **GATE**: Build with VS2026 via updated script → `git commit -m "W10: build infra VS2026 support"` → `git push`

---

## 9. Verification

- [ ] Build x64 Release with VS2026 (`Build_x64.cmd`)
- [ ] Build x64 Release with VS2022 (`Build_x64.cmd` – verify backward compat)
- [ ] Build Win32 Release (`Build_Win32.cmd`)
- [ ] Grep for any remaining `0x0601` or `0x06010000` references
- [ ] Grep for any remaining `_WIN32_WINNT_WIN7` or `NTDDI_WIN7` comments
- [ ] Smoke-test launch on Windows 10
- [ ] Smoke-test launch on Windows 11
- [ ] Verify Dark Mode still works (Win10 1809+)
- [ ] Verify URL copy/escape in hyperlink features
- [ ] Verify toolbar appearance (XP ternary removed)
- [ ] Verify elevated relaunch still works

---

## 10. Documentation

- [ ] Create `WinUpgrade.md` in repo root documenting all changes made
- [ ] Update any README/docs referencing "Windows 7" as minimum OS requirement
