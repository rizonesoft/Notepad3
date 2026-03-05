# Windows 10 Minimum Version Upgrade – Actionable Checklist

> **Goal**: Raise the minimum supported OS from Windows 7 (`0x0601`) to Windows 10 (`0x0A00`).
> Verified: VS2026 (v145) and VS2022 (v143) both build clean before changes.
>
> **Strategy**: Implement each section → build with VS2026 → commit & push → move on.

---

## 1. Version Defines – vcxproj Files ✅ (PR #5565)

All preprocessor definitions in every `<PreprocessorDefinitions>` block must change:

| Old                        | New                        |
| -------------------------- | -------------------------- |
| `_WIN32_WINNT=0x601`       | `_WIN32_WINNT=0x0A00`      |
| `WINVER=0x601`             | `WINVER=0x0A00`            |
| `NTDDI_VERSION=0x06010000` | `NTDDI_VERSION=0x0A000000` |

### Notepad3 Core

- [x] `src/Notepad3.vcxproj` – 9 config blocks _(PR #5565)_

### Scintilla

- [x] `scintilla/Scintilla.vcxproj` – 12 config blocks _(PR #5565)_
- [x] `scintilla/ScintillaDLL.vcxproj` – 4 config blocks _(PR #5565)_

### Lexilla

- [x] `lexilla/Lexilla.vcxproj` – 9 config blocks _(PR #5565)_

### Other

- [x] `other_sln/Notepad3DLL.vcxproj` – 4 config blocks _(PR #5565)_

> 🔨 **GATE**: ~~Build with VS2026 → commit → push~~ ✅ Done via PR #5565

---

## 2. Version Defines – Source File Headers ✅ (PR #5565)

PR #5565 updated all headers using `_WIN32_WINNT_WIN10` / `NTDDI_WIN10_RS5` symbolic constants + `#include <sdkddkver.h>`.

### Primary headers

- [x] `src/TypeDefs.h` – updated defines _(PR #5565)_
- [x] `src/TypeDefs.h` – `#if 0` block handled _(PR #5565)_
- [x] `src/resource.h` – updated defines _(PR #5565)_

### Individual source files

- [x] `src/EncodingDetection.cpp` _(PR #5565)_
- [x] `src/Print.cpp` _(PR #5565)_
- [x] `src/PathLib.c` _(PR #5565)_
- [x] `src/Config/Config.cpp` _(PR #5565)_
- [x] `src/crypto/crypto.c` _(PR #5565)_
- [x] `src/crypto/rijndael-api-fst.c` _(PR #5565)_
- [x] `src/ChooseFont/ChooseFont.h` _(PR #5565)_

### Backlog / Test files

- [x] `src/_backlog/AccelKeys.c` _(PR #5565)_
- [ ] `test/test_files/StyleLexers/styleLexCPP/Config.cpp` lines 41–48

> 🔨 **GATE**: ~~Build with VS2026 → commit → push~~ ✅ Done via PR #5565

---

## 3. Application Manifests – Clean Up Legacy OS GUIDs (Partial ✅)

### GUIDs to remove (Vista, Win7, Win8, Win8.1):

| GUID                                     | OS     |
| ---------------------------------------- | ------ |
| `{e2011457-1546-43c5-a5fe-008deee3d3f0}` | Vista  |
| `{35138b9a-5d96-4fbd-8e2d-a2440225f93a}` | Win7   |
| `{4a2f28e3-53b9-4441-ba9c-d69d4a4a6e38}` | Win8   |
| `{1f676c76-80e1-4239-95bb-83d0f6d0da78}` | Win8.1 |

### Files to update:

- [x] `res/Notepad3.exe.manifest` – cleaned _(PR #5565)_
- [x] `minipath/res/MiniPath.exe.manifest` – cleaned _(PR #5565)_
- [ ] `grepWinNP3/src/compatibility.manifest` – remove Vista/Win7/Win8/Win8.1 entries

> 🔨 **GATE**: Build with VS2026 → `git commit -m "W10: manifest cleanup"` → `git push`

---

## 4. Runtime Version Checks – Dead Code Removal (Partial ✅)

### `src/Notepad3.c`

- [ ] Line 688 – `IsWindowsXPSP3OrGreater() ? 1 : 2` → always `1` (remove ternary)
- [x] Lines 1010–1014 – `IsWindows7SP1OrGreater()` guard → updated _(PR #5565)_
- [x] Lines 2829–2839 – `if (!IsWindowsVistaOrGreater())` block removed _(PR #5565)_
- [ ] Line 3311 – Remove `!IsWindowsXPSP3OrGreater() &&` from condition (always false)
- [ ] Line 3314 – Remove `if (bProcessed && !IsWindowsXPSP3OrGreater())` block (dead code)
- [x] Line 12112 – `!IsWindowsVistaOrGreater() ||` removed _(PR #5565)_

### `src/Helpers.c`

- [x] Lines 355–361 – `IsProcessElevated()`: `IsWindowsVistaOrGreater()` check removed _(PR #5565)_
- [x] Lines 225–251 – `GetWinVersionString()`: Win7/Win8/Win8.1 branches removed _(PR #5565)_

> 🔨 **GATE**: Build with VS2026 → `git commit -m "W10: remove dead version checks"` → `git push`

---

## 5. Compile-Time Conditionals – Simplified by NTDDI_WIN10 (Partial ✅)

### `src/Helpers.c` – URL Escape Functions

- [ ] Lines 1986–1999 – Remove `#if (NTDDI_VERSION < NTDDI_WIN8)` legacy URL char tables (dead code)
- [ ] Lines 2003–2049 – `UrlEscapeEx()`: Remove `#else` branch, keep only the `NTDDI_WIN8+` path
- [ ] Lines 2057–2100 – `UrlUnescapeEx()`: Remove `#else` branch, keep only the `NTDDI_WIN8+` path

### `src/DarkMode/DarkMode.cpp`

- [x] Lines 58–69 – `#if _WIN32_WINNT < _WIN32_WINNT_WIN8` block removed _(PR #5565)_
- [x] Line 64 – `GetProcAddress` for `SetDefaultDllDirectories` removed _(PR #5565)_

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

- [ ] `src/PathLib.c` lines 284–313 – `HasOptInToRemoveMaxPathLimit()`: Replace `LoadLibrary("ntdll.dll")` + `GetProcAddress` with direct ntdll linkage

### `PathCch*` APIs – Can Now Be Used (Win8.1+)

- [ ] `src/PathLib.c` lines 154–157 – Evaluate uncommenting `#include <pathcch.h>` and `#pragma comment(linker, "/defaultlib:Pathcch")`
- [ ] `src/PathLib.c` – Evaluate replacing `_PathCanonicalize()` with `PathCchCanonicalize()` / `PathCchCanonicalizeEx()`
- [ ] `src/PathLib.c` – Evaluate replacing `_Path_IsRelative()` MAX_PATH hack

### `GetSystemMetrics()` → `GetSystemMetricsForDpi()` (Win10 1607+)

- [ ] `src/Notepad3.c` lines 1106–1110 – Icon size queries during init
- [ ] `src/Notepad3.c` line 3706 – `SM_CYFRAME` query
- [ ] `src/Notepad3.c` line 12181 – `SM_CXSMICON`/`SM_CYSMICON` in `RelaunchElevated()`
- [ ] `src/Config/Config.cpp` lines 1862–1863, 2263–2264, 2315–2316 – Virtual screen size queries

> 🔨 **GATE**: Build with VS2026 → `git commit -m "W10: API modernizations"` → `git push`

---

## 7. Commented-Out / Disabled Legacy Code Cleanup

- [ ] `src/Helpers.c` lines 184–222 – `_GetTrueWindowsVersion()` inside `#if 0`: Remove entire block (uses deprecated `GetVersionEx`)
- [ ] `src/Notepad3.c` lines 1251–1253 – Remove commented-out `ChangeWindowMessageFilter` calls
- [ ] `src/Notepad3.c` lines 1016–1017 – Evaluate enabling `SetProcessDpiAwarenessContext` (manifest already declares PerMonitorV2 — remove commented code)
- [ ] `src/Notepad3.c` line 1091 – Evaluate `SetThreadDpiAwarenessContext` call
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
