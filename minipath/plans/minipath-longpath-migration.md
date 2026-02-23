# MiniPath: Migration from MAX_PATH to HPATHL Long Path Support

## Context

MiniPath is Notepad3's companion file browser (Ctrl+M). It currently uses fixed `WCHAR[MAX_PATH]` (260 char) buffers for all path operations, while Notepad3 core has been fully migrated to dynamic `HPATHL` handles (up to 32,768 chars via PathLib/DynStrg). MiniPath's `longPathAware` manifest entry was removed because the code cannot handle long paths.

The goal is to migrate MiniPath to use `HPATHL`/`HSTRINGW` consistently, matching Notepad3's architecture, then re-enable `longPathAware` in the manifest.

## Key Libraries

- **PathLib** (`src/PathLib.h`, `src/PathLib.c`): Provides `HPATHL` opaque handle for dynamic long paths. Built on DynStrg. Max capacity: `PATHLONG_MAX_CCH` = 0x8000 (32,768 chars).
- **DynStrg** (`src/DynStrg.h`, `src/DynStrg.c`): Provides `HSTRINGW` opaque handle for dynamic wide strings.
- Both are already compiled into the MiniPath project (shared `src/` modules).
- `HPATHL` is a type-safe cast of `HSTRINGW` — PathLib wraps DynStrg with path-specific operations.

## HPATHL API Quick Reference

```c
// Lifecycle
HPATHL Path_Allocate(LPCWSTR path);    // Create (NULL for empty)
void   Path_Release(HPATHL hpth);      // Free
HPATHL Path_Copy(const HPATHL hpth);   // Duplicate

// Read access
LPCWSTR Path_Get(HPATHL hpth);                    // Get LPCWSTR pointer
size_t  Path_GetLength(HPATHL hpth);               // String length
size_t  Path_GetBufCount(HPATHL hpth);             // Allocated buffer size
bool    Path_IsEmpty(HPATHL hpth);
bool    Path_IsNotEmpty(HPATHL hpth);

// Write access
void   Path_Reset(HPATHL hpth, LPCWSTR path);     // Replace contents
void   Path_Empty(HPATHL hpth, bool truncate);     // Clear (keep handle)
LPWSTR Path_WriteAccessBuf(HPATHL hpth, size_t len); // Get writable buffer (min MAX_PATH_EXPLICIT)
void   Path_Sanitize(HPATHL hpth);                // Recalc length after external buffer write
void   Path_Swap(HPATHL h1, HPATHL h2);

// Path operations
bool    Path_Append(HPATHL hpth, LPCWSTR more);
bool    Path_RemoveFileSpec(HPATHL hpth);
void    Path_StripPath(HPATHL hpth);
bool    Path_RenameExtension(HPATHL hpth, LPCWSTR ext);
LPCWSTR Path_FindFileName(HPATHL hpth);
LPCWSTR Path_FindExtension(HPATHL hpth);
bool    Path_Canonicalize(HPATHL hpth);
bool    Path_CanonicalizeEx(HPATHL hpth, HPATHL hbase);
void    Path_ExpandEnvStrings(HPATHL hpth);
bool    Path_IsRoot(HPATHL hpth);
bool    Path_IsRelative(HPATHL hpth);
bool    Path_IsUNC(HPATHL hpth);
bool    Path_IsExistingFile(HPATHL hpth);
bool    Path_IsExistingDirectory(HPATHL hpth);
bool    Path_IsLnkFile(HPATHL hpth);
DWORD   Path_GetFileAttributes(HPATHL hpth);
void    Path_QuoteSpaces(HPATHL hpth, bool force);
void    Path_UnQuoteSpaces(HPATHL hpth);
size_t  Path_GetLongPathNameEx(HPATHL hpth);
size_t  Path_ToShortPathName(HPATHL hpth);
void    Path_GetModuleFilePath(HPATHL hpth);
void    Path_GetAppDirectory(HPATHL hpth);
bool    Path_GetCurrentDirectory(HPATHL hpth);
void    Path_FreeExtra(HPATHL hpth, size_t keep);
bool    Path_BrowseDirectory(HWND hwnd, LPCWSTR title, HPATHL hpth_io, HPATHL hbase, bool newStyle);
void    Path_RelativeToApp(HPATHL hpth, bool isFile, bool unexpandEnv, bool unexpandDocs);
int     Path_StrgComparePath(HPATHL h1, HPATHL h2, HPATHL wrkdir, bool normalize);
void    Path_NormalizeEx(HPATHL hpth, HPATHL wrkdir, bool real, bool search);
```

---

## PART 1: WHAT NEEDS TO CHANGE

### 1.1 Global Variables (minipath/src/minipath.c:88-109)

| Current declaration | Line | Purpose | Migration |
|---|---|---|---|
| `WCHAR g_wchIniFile[MAX_PATH]` | 88 | MiniPath INI file path | `HPATHL g_hIniFile = NULL;` |
| `WCHAR g_wchIniFile2[MAX_PATH]` | 89 | Fallback/redirect INI path | `HPATHL g_hIniFile2 = NULL;` |
| `WCHAR g_wchNP3IniFile[MAX_PATH]` | 90 | Notepad3 INI file path | `HPATHL g_hNP3IniFile = NULL;` |
| `WCHAR szTargetApplication[MAX_PATH]` | 107 | Target app executable path | `HPATHL g_hTargetApplication = NULL;` |
| `WCHAR szTargetApplicationParams[MAX_PATH]` | 108 | Target app parameters | `HSTRINGW g_hTargetAppParams = NULL;` |
| `WCHAR szTargetApplicationWndClass[MAX_PATH]` | 109 | Target app window class | `HSTRINGW g_hTargetAppWndClass = NULL;` |

All `extern` declarations must be updated in: Config.cpp:30-32, Dialogs.c:607, Helpers.c:47, and Dialogs.c:2426-2428.

### 1.2 SETTINGS_T Struct (minipath/src/minipath.h:104-154)

Current fields to replace:

| Current field | Line | Purpose | Migration |
|---|---|---|---|
| `WCHAR szCurDir[MAX_PATH + 40]` | 143 | Current browsed directory | `HPATHL hCurDir;` |
| `WCHAR szQuickview[MAX_PATH]` | 144 | Quickview app path | `HPATHL hQuickview;` |
| `WCHAR szQuickviewParams[MAX_PATH]` | 145 | Quickview parameters | `HSTRINGW hQuickviewParams;` |
| `WCHAR tchFavoritesDir[MAX_PATH]` | 146 | Favorites directory | `HPATHL hFavoritesDir;` |
| `WCHAR tchOpenWithDir[MAX_PATH]` | 147 | Open-with directory | `HPATHL hOpenWithDir;` |
| `WCHAR tchToolbarBitmap[MAX_PATH]` | 149 | Toolbar bitmap path | `HPATHL hToolbarBitmap;` |
| `WCHAR tchToolbarBitmapHot[MAX_PATH]` | 150 | Toolbar hot bitmap | `HPATHL hToolbarBitmapHot;` |
| `WCHAR tchToolbarBitmapDisabled[MAX_PATH]` | 151 | Toolbar disabled bitmap | `HPATHL hToolbarBitmapDisabled;` |

Non-path fields to keep as-is: `tchToolbarButtons[512]`, `tchFilter[DL_FILTER_BUFSIZE]`, all `int`/`bool`/`RECT` fields.

Note: There is both a `Settings` and a `Defaults` instance of this struct. Both need HPATHL allocation/release.

### 1.3 DLITEM Struct (minipath/src/Dlapi.h:102-109)

```c
typedef struct tagDLITEM {
    UINT mask;
    WCHAR szFileName[MAX_PATH];      // Line 105 — full file path
    WCHAR szDisplayName[MAX_PATH];   // Line 106 — display name
    int  ntype;
} DLITEM, *LPDLITEM;
```

Populated by `DirList_GetItem()` (Dlapi.c:714-721) via `IL_GetDisplayName()` with `SHGDN_FORPARSING` and `SHGDN_INFOLDER`.

Migration options:
- **Option A (recommended):** Replace with HPATHL members. Callers must allocate/release.
- **Option B (simpler):** Increase buffer to `PATHLONG_MAX_CCH` (wastes 64KB per DLITEM on stack).

### 1.4 DLDATA Internal Struct (minipath/src/Dlapi.c:38)

```c
typedef struct tagDLDATA {
    // ...
    WCHAR szPath[MAX_PATH];  // Current directory path
} DLDATA;
```

Replace with `HPATHL hPath;` — allocate in `DirList_Init()`, release in `DirList_Destroy()`.

---

## PART 2: INITIALIZATION AND CLEANUP PATTERNS

### How Notepad3 Does It (model to follow)

**Allocation (src/Notepad3.c:715-750):**
```c
// In WinMain / initialization:
Paths.CurrentFile = Path_Allocate(NULL);
Paths.ModuleDirectory = Path_Allocate(NULL);
Paths.WorkingDirectory = Path_Allocate(NULL);
Paths.IniFile = Path_Allocate(NULL);
Paths.IniFileDefault = Path_Allocate(NULL);

Settings.OpenWithDir = Path_Allocate(NULL);
Defaults.OpenWithDir = Path_Allocate(NULL);
Settings.FavoritesDir = Path_Allocate(NULL);
Defaults.FavoritesDir = Path_Allocate(NULL);
```

**Deallocation (src/Notepad3.c:839-868):**
```c
// In WM_DESTROY / cleanup (reverse order):
Path_Release(Settings.FavoritesDir);
Path_Release(Defaults.FavoritesDir);
Path_Release(Settings.OpenWithDir);
Path_Release(Defaults.OpenWithDir);
Path_Release(Paths.IniFileDefault);
Path_Release(Paths.IniFile);
// ... etc
```

### MiniPath Initialization Order

```
WinMain()
 +-- [INIT HPATHL globals here]          <-- NEW: allocate g_hIniFile etc.
 +-- [INIT Settings/Defaults HPATHL]     <-- NEW: allocate Settings.hCurDir etc.
 +-- ParseCommandLine()                   Uses lpPathArg (GlobalAlloc) -> migrate to HPATHL
 +-- FindIniFile()          Config.cpp    Fills g_wchIniFile -> use Path_Reset(g_hIniFile, ...)
 +-- TestIniFile()          Config.cpp    Validates INI path
 +-- LoadFlags()            Config.cpp    Loads INI cache
 +-- LoadSettings()         Config.cpp    Fills Settings struct fields
 +-- ChangeDirectory()                    Sets Settings.szCurDir -> Path_Reset(Settings.hCurDir, ...)
 +-- DirList_Init/Fill()                  Uses Settings.szCurDir -> Path_Get(Settings.hCurDir)
 :
 +-- WM_DESTROY
      +-- SaveSettings()    Config.cpp
      +-- [RELEASE all HPATHL globals]    <-- NEW: Path_Release(g_hIniFile) etc.
      +-- [RELEASE Settings/Defaults]     <-- NEW: Path_Release(Settings.hCurDir) etc.
```

---

## PART 3: MAX_PATH OCCURRENCES BY FILE (127 total)

### minipath.c — 48 occurrences

**Local buffers in functions (replace with HPATHL or PATHLONG_MAX_CCH):**

| Line(s) | Variable | Context | Replacement |
|---|---|---|---|
| 594 | `WCHAR tch[MAX_PATH]` | WM_COMMAND path manipulation | HPATHL local |
| 638-639 | `wchMenuEntry[MAX_PATH]`, `wchTargetAppName[MAX_PATH]` | Menu generation | HSTRINGW |
| 870 | `szBuf[MAX_PATH+40]` | WM_DROPFILES handler | HPATHL + Path_WriteAccessBuf |
| 1252 | `szTmp[MAX_PATH]` | File operations | HPATHL |
| 2007, 2010, 2023 | `szNewFile[MAX_PATH]`, `szPath[MAX_PATH]`, `ofn.nMaxFile=MAX_PATH` | Save-As dialog | HPATHL + PATHLONG_MAX_CCH |
| 2069, 2083 | `tchNewDir[MAX_PATH]`, `tchLinkDestination[MAX_PATH]` | Directory/link creation | HPATHL |
| 2102-2103 | `szNewFile[MAX_PATH]`, `tch[MAX_PATH]` | File dialog | HPATHL |
| 2258-2259 | `szModuleName[MAX_PATH]`, `szParameters[MAX_PATH+64]` | New window launch | HPATHL + HSTRINGW |
| 2612-2613, 2670, 2685, 2729, 2770 | Various `szFullPath`, `szDir`, `tch` | File operations | HPATHL |
| 3108-3109 | `szTest[MAX_PATH]`, `szWinDir[MAX_PATH]` | INI file search | HPATHL |
| 3292 | `GlobalAlloc(GPTR, sizeof(WCHAR)*(MAX_PATH+2))` | Command-line path | HPATHL |
| 3313-3314, 3387-3388 | `szPath[MAX_PATH]`, `szTmp[MAX_PATH]` | DisplayPath() | HPATHL |
| 3554 | `tchTmp[MAX_PATH]` | Error messages | HSTRINGW |
| 3733-3736, 3800-3801, 3836 | `szFile`, `szParam`, `szTmp` | Shell execute / WM_COPYDATA | HPATHL + HSTRINGW |
| 3767, 3825 | `GetShortPathName(..., MAX_PATH)` | Short path conversion | Path_ToShortPathName(HPATHL) |

### Config.cpp — 15 occurrences

| Line(s) | Variable | Context | Replacement |
|---|---|---|---|
| 30-32 | `extern g_wchIniFile[MAX_PATH]` etc. | Extern declarations | Update to HPATHL |
| 118, 124 | `msg[MAX_PATH + 128]` | Error messages | HSTRINGW or larger buffer |
| 659 | `tchDir[MAX_PATH]` | CreateIniFileEx() | HPATHL |
| 721-722 | `tchFileExpanded[MAX_PATH]`, `tchBuild[MAX_PATH]` | CheckIniFile() | HPATHL |
| 768, 775 | `tch[MAX_PATH]`, `tchFileExpanded[MAX_PATH]` | CheckIniFileRedirect() | HPATHL |
| 795-796 | `tchTest[MAX_PATH]`, `tchModule[MAX_PATH]` | FindIniFile() | HPATHL |
| 870, 884 | `wchModule[MAX_PATH]` | TestIniFile() | HPATHL |
| 1152 | `wchTmp[MAX_PATH]` | LoadSettings() | HSTRINGW |

### Dialogs.c — 53 occurrences

| Line(s) | Variable | Context | Replacement |
|---|---|---|---|
| 72, 79 | `szBase[MAX_PATH]`, `GetCurrentDirectory(MAX_PATH, ...)` | Browse directory | Path_GetCurrentDirectory(HPATHL) |
| 174, 369 | `EM_LIMITTEXT, MAX_PATH-1` | Edit control limits | Increase to 512+ |
| 193-195 | `szArgs[MAX_PATH]`, `szArg2[MAX_PATH]`, `szFile[MAX_PATH*2]` | Run dialog | HPATHL + HSTRINGW |
| 235, 237, 249-252 | Various dialog text buffers | Command-line dialog | HSTRINGW |
| 469, 471 | `tch[MAX_PATH]` | GoTo dialog | HPATHL |
| 607 | `extern g_wchIniFile[MAX_PATH]` | Extern declaration | Update to HPATHL |
| 1060, 1075, 1079 | `tch[MAX_PATH]`, `EM_LIMITTEXT` | Preferences dialog | HSTRINGW, increase limits |
| 1097-1099 | `tchBuf[MAX_PATH]`, `szFile[MAX_PATH]`, `szParams[MAX_PATH]` | Browse button | HPATHL + HSTRINGW |
| 1137, 1160-1164, 1174 | Various settings read/write buffers | Settings dialog saves | HPATHL |
| 1466-1467 | `szSource[MAX_PATH]`, `szDestination[MAX_PATH]` | Rename dialog | HPATHL |
| 1494 | `EM_LIMITTEXT, MAX_PATH-1` | Rename edit limit | Increase |
| 1549-1551 | `szFullDestination[MAX_PATH]`, `tchSource[MAX_PATH+4]`, `tchDestination[MAX_PATH+4]` | Copy/move dialog (SHFileOperation) | HPATHL |
| 1665 | `CB_LIMITTEXT, MAX_PATH-1` | Destination combo limit | Increase |
| 1763, 1825-1826 | `tch[MAX_PATH]`, `tchSource/tchDestination[MAX_PATH+4]` | Delete dialog | HPATHL |
| 1861-1862 | `wszDir[MAX_PATH]`, `GetCurrentDirectory(...)` | Recycle bin | Path_GetCurrentDirectory() |
| 1907 | `tch[MAX_PATH]` | Delete progress | HSTRINGW |
| 2121, 2128 | `szDestination[MAX_PATH+4]`, `szSource[MAX_PATH+4]` | Open-With dialog | HPATHL |
| 2156 | `szParam[MAX_PATH]` | Shell parameter | HSTRINGW |
| 2207 | `EM_LIMITTEXT, MAX_PATH-1` | New dir edit limit | Increase |
| 2388 | `tch[MAX_PATH]` | Status message | HSTRINGW |
| 2426-2428 | extern declarations | Target app declarations | Update to HPATHL/HSTRINGW |
| 2446 | `wch[MAX_PATH]` | Target app dialog | HSTRINGW |
| 2487 | `EM_LIMITTEXT, MAX_PATH-1` | Target path limit | Increase |
| 2538-2540 | `tchBuf[MAX_PATH]`, `szFile[MAX_PATH]`, `szParams[MAX_PATH]` | Target app browse | HPATHL + HSTRINGW |
| 2652 | `tch[MAX_PATH]` | Message buffer | HSTRINGW |

### Dlapi.c — 12 occurrences

| Line(s) | Variable | Context | Replacement |
|---|---|---|---|
| 38 | `szPath[MAX_PATH]` in DLDATA struct | Current directory | HPATHL member |
| 203 | `wszDir[MAX_PATH]` | DirList_Fill() | HPATHL |
| 255 | `lvi.cchTextMax = MAX_PATH` | ListView display name | Update with DLITEM |
| 715, 721 | `IL_GetDisplayName(..., MAX_PATH)` | DirList_GetItem() | Depends on DLITEM migration |
| 937 | `tch[MAX_PATH]` | DirList_GetLongPathName() | HPATHL |
| 960, 971, 988 | `szShortPath[MAX_PATH]`, `GetShortPathName(..., MAX_PATH)` | Short path ops | Path_ToShortPathName() |
| 977 | `StringCchCopyN(..., MAX_PATH)` | SHFILEINFO display | Keep (shell limitation) |
| 1155 | `cbei.cchTextMax = MAX_PATH` | ComboBoxEx | Keep (UI control) |

### Helpers.c — 23 occurrences

| Line(s) | Variable | Context | Replacement |
|---|---|---|---|
| 47 | `extern g_wchIniFile[MAX_PATH]` | Extern declaration | Update to HPATHL |
| 347, 359 | `szTitle[MAX_PATH+120]`, `tchPath[MAX_PATH]` | SetWindowPathTitle() | HSTRINGW + HPATHL |
| 589-593 | `wchAppPath`, `wchWinDir`, `wchUserFiles`, `wchPath`, `wchResult` [all MAX_PATH] | PathRelativeToApp() | All HPATHL (use Path_RelativeToApp pattern) |
| 626, 628 | `lstrcpyn(..., MAX_PATH)` | Copy with fallback | StringCchCopy or HPATHL |
| 640-641 | `wchPath[MAX_PATH]`, `wchResult[MAX_PATH]` | PathAbsoluteFromApp() | HPATHL |
| 668, 670 | `lstrcpyn(..., MAX_PATH)` | Copy with fallback | StringCchCopy or HPATHL |
| 747 | `WORD wsz[MAX_PATH]` | Link file path (NOTE: should be WCHAR, not WORD) | Fix type + HPATHL |
| 789, 831 | `tchResPath[MAX_PATH]`, `tchLnkFileName[MAX_PATH]` | GetLinkPath() | HPATHL |
| 848 | `WORD wsz[MAX_PATH]` | Link file path (same type bug) | Fix type + HPATHL |
| 1034 | `szDst[MAX_PATH]` | CopyShortPath() with PathCanonicalize | HPATHL + Path_Canonicalize() |

---

## PART 4: DEPRECATED SHLWAPI FUNCTIONS TO REPLACE

| Deprecated function | Occurrences | PathLib replacement |
|---|---|---|
| `PathRemoveFileSpec(szPath)` | 11 calls | `Path_RemoveFileSpec(hpth)` |
| `PathAppend(szPath, more)` | 8 calls | `Path_Append(hpth, more)` |
| `PathFindFileName(szPath)` | 10 calls | `Path_FindFileName(hpth)` |
| `PathIsRelative(szPath)` | 7 calls | `Path_IsRelative(hpth)` (via `Path_IsRelative`) |
| `PathIsRoot(szPath)` | 6 calls | `Path_IsRoot(hpth)` |
| `PathRenameExtension(szPath, ext)` | 5 calls | `Path_RenameExtension(hpth, ext)` |
| `PathCanonicalize(szDst, szSrc)` | 1 call | `Path_Canonicalize(hpth)` or `Path_CanonicalizeEx(hpth, hbase)` |
| `SHGetFolderPath(hwnd, csidl, ...)` | 5 calls | `Path_GetKnownFolder(&FOLDERID_xxx, hpth)` |
| `GetShortPathName(src, dst, MAX_PATH)` | 7 calls | `Path_ToShortPathName(hpth)` |
| `ExpandEnvironmentStrings(src, dst, MAX_PATH)` | 6 calls | `Path_ExpandEnvStrings(hpth)` |
| `GetCurrentDirectory(MAX_PATH, buf)` | 4 calls | `Path_GetCurrentDirectory(hpth)` |
| `GetModuleFileName(NULL, buf, MAX_PATH)` | 5 calls | `Path_GetModuleFilePath(hpth)` |
| `SHGetPathFromIDList(pidl, szPath)` | 5 calls | Keep but use PATHLONG_MAX_CCH buffer |

---

## PART 5: INI FILE HANDLING PATTERNS

### Notepad3 Pattern (model to follow)

**Loading a path setting from INI (src/Config/Config.cpp:1386-1398):**
```c
WCHAR pPathBuffer[PATHLONG_MAX_CCH] = { L'\0' };
IniSectionGetStringNoQuotes(section, L"DefaultDirectory", L"", pPathBuffer, PATHLONG_MAX_CCH);
if (StrIsNotEmpty(pPathBuffer)) {
    Path_Reset(Settings2.DefaultDirectory, pPathBuffer);
    Path_ExpandEnvStrings(Settings2.DefaultDirectory);
}
```

**Saving a path setting to INI (src/Config/Config.cpp:2071-2085):**
```c
HPATHL hpth = Path_Allocate(NULL);
if (StringCchCompareXI(Path_Get(Settings.OpenWithDir), Path_Get(Defaults.OpenWithDir)) != 0) {
    Path_Reset(hpth, Path_Get(Settings.OpenWithDir));
    Path_RelativeToApp(hpth, false, true, Flags.PortableMyDocs);
    IniSectionSetString(section, L"OpenWithDir", Path_Get(hpth));
} else {
    IniSectionDelete(section, L"OpenWithDir", false);
}
Path_Release(hpth);
```

**FindIniFile flow (src/Config/Config.cpp:1045-1088):**
```c
// 1. Try module path with .ini extension
Path_GetModuleFilePath(Paths.IniFile);
Path_RenameExtension(Paths.IniFile, L".ini");
bFound = _CheckAndSetIniFile(Paths.IniFile);

// 2. Try app directory + name
if (!bFound) {
    Path_GetAppDirectory(Paths.IniFile);
    Path_Append(Paths.IniFile, L"AppName.ini");
    bFound = _CheckAndSetIniFile(Paths.IniFile);
}

// 3. Handle redirects
// 4. Normalize result
Path_NormalizeEx(Paths.IniFile, Paths.ModuleDirectory, true, false);
```

**IniFileGetString/SetString (src/Config/Config.cpp:608-666):**
Both accept `const HPATHL hpthIniFile` as first parameter. Internally call `Path_Get(hpthIniFile)` to get the LPCWSTR for file operations. MiniPath's versions currently take `LPCWSTR lpFilePath` — update signature to take `HPATHL`.

---

## PART 6: FILE DIALOG PATTERN

### Notepad3 Open/Save Dialog (src/Dialogs.c:7037-7074)

```c
bool OpenFileDlg(HWND hwnd, HPATHL hfile_pth_io, const HPATHL hinidir_pth)
{
    OPENFILENAME ofn = { sizeof(OPENFILENAME) };
    ofn.lpstrFile = Path_WriteAccessBuf(hfile_pth_io, PATHLONG_MAX_CCH);
    ofn.nMaxFile  = (DWORD)Path_GetBufCount(hfile_pth_io);
    ofn.lpstrInitialDir = Path_IsNotEmpty(hpth_dir) ? Path_Get(hpth_dir) : NULL;
    // ... other setup ...

    bool const res = GetOpenFileNameW(&ofn);
    Path_Sanitize(hfile_pth_io);              // Recalc length after dialog writes to buffer
    Path_FreeExtra(hfile_pth_io, MAX_PATH_EXPLICIT);  // Trim excess allocation
    return res;
}
```

Key pattern: `Path_WriteAccessBuf()` -> dialog writes to buffer -> `Path_Sanitize()` -> `Path_FreeExtra()`.

### Notepad3 DragQueryFile (src/Notepad3.c:3607-3621)

```c
HPATHL hdrop_pth = Path_Allocate(NULL);
wchar_t* const drop_buf = Path_WriteAccessBuf(hdrop_pth, STRINGW_MAX_URL_LENGTH);
DragQueryFileW(hDrop, i, drop_buf, (UINT)Path_GetBufCount(hdrop_pth));
Path_Sanitize(hdrop_pth);
// ... use hdrop_pth ...
Path_Release(hdrop_pth);
```

---

## PART 7: MIGRATION PHASES

### Phase 1: Infrastructure (low risk)

1. Verify `#include "PathLib.h"` and `#include "DynStrg.h"` are available in MiniPath build
2. Add HPATHL global declarations alongside existing WCHAR globals (dual mode)
3. Add allocation in WinMain init, release in WM_DESTROY cleanup
4. Add `#include` directives to all MiniPath source files that need them

### Phase 2: Global Variables (medium risk)

1. Replace `g_wchIniFile[MAX_PATH]` with `HPATHL g_hIniFile`
2. Replace `g_wchIniFile2[MAX_PATH]` with `HPATHL g_hIniFile2`
3. Replace `g_wchNP3IniFile[MAX_PATH]` with `HPATHL g_hNP3IniFile`
4. Replace `szTargetApplication[MAX_PATH]` with `HPATHL g_hTargetApplication`
5. Replace `szTargetApplicationParams[MAX_PATH]` with `HSTRINGW g_hTargetAppParams`
6. Replace `szTargetApplicationWndClass[MAX_PATH]` with `HSTRINGW g_hTargetAppWndClass`
7. Update all extern declarations (Config.cpp:30-32, Dialogs.c:607, Helpers.c:47, Dialogs.c:2426-2428)
8. Update Config.cpp: FindIniFile(), TestIniFile(), IniFileGetString/SetString signatures

### Phase 3: SETTINGS_T Struct (high risk — most dependencies)

1. Replace WCHAR fields with HPATHL/HSTRINGW in minipath.h
2. Add allocation in initialization for both `Settings` and `Defaults` instances
3. Update LoadSettings() (Config.cpp) to use Path_Reset + Path_ExpandEnvStrings
4. Update SaveSettings() (Config.cpp) to use Path_Get + Path_RelativeToApp
5. Update every `Settings.szCurDir` access (dozens of locations) to `Path_Get(Settings.hCurDir)` for reads and `Path_Reset(Settings.hCurDir, ...)` for writes
6. Same for all other replaced fields

### Phase 4: DLITEM Struct (high risk — API change)

1. Replace `WCHAR szFileName[MAX_PATH]` with `HPATHL hFileName` (or increase to PATHLONG_MAX_CCH)
2. Update `DirList_GetItem()` to populate HPATHL
3. Update all callers in Dialogs.c and minipath.c
4. Update `IL_GetDisplayName()` calls

### Phase 5: Local Buffers (medium risk — many but mechanical)

For each function, replace local `WCHAR xxx[MAX_PATH]` with either:
- `HPATHL hxxx = Path_Allocate(NULL);` ... `Path_Release(hxxx);` — for path operations
- `HSTRINGW hxxx = StrgCreate(NULL);` ... `StrgDestroy(hxxx);` — for general strings
- `WCHAR xxx[PATHLONG_MAX_CCH];` — for simple Win32 API output buffers (acceptable for stack)

### Phase 6: Deprecated API Replacement (low risk per call)

Replace each deprecated shlwapi function with its PathLib equivalent per the table in Part 4.

### Phase 7: Re-enable Manifest and Test

1. Add back `<ws2:longPathAware>true</ws2:longPathAware>` to `minipath/res/MiniPath.exe.manifest`
2. Test with paths > 260 characters
3. Test INI file in long-path directory
4. Test file operations (copy/move/rename/delete) with long paths
5. Test drag-drop with long paths
6. Test backward compatibility with existing INI files

---

## PART 8: TYPE BUG TO FIX

In `minipath/src/Helpers.c` lines 747 and 848:
```c
WORD wsz[MAX_PATH];  // BUG: should be WCHAR, not WORD
```
This is a type error — `WORD` is `unsigned short` (same size as `wchar_t` on Windows, so it works, but it's semantically wrong). Fix when migrating.

---

## FILES TO MODIFY

| File | Changes | Effort |
|---|---|---|
| `minipath/src/minipath.h` | SETTINGS_T struct migration | Medium |
| `minipath/src/minipath.c` | 48 MAX_PATH replacements, global vars, init/cleanup | Large |
| `minipath/src/Config.cpp` | 15 MAX_PATH replacements, INI function signatures, load/save | Large |
| `minipath/src/Dialogs.c` | 53 MAX_PATH replacements, dialog buffers, edit limits | Large |
| `minipath/src/Dlapi.h` | DLITEM struct migration | Small |
| `minipath/src/Dlapi.c` | 12 MAX_PATH replacements, DLDATA struct | Medium |
| `minipath/src/Helpers.c` | 23 MAX_PATH replacements, type bug fix | Medium |
| `minipath/res/MiniPath.exe.manifest` | Re-add longPathAware (Phase 7 only) | Trivial |

Total: ~161 MAX_PATH occurrences across 7 source files.
