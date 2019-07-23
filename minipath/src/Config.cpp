/******************************************************************************
*                                                                             *
*                                                                             *
* MiniPath                                                                    *
*                                                                             *
* Config.cpp                                                                  *
*   Methods to read and write configuration                                   *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2019   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

#include <strsafe.h>
#include <shlobj.h>
#include <Shlwapi.h>

// ----------------------------------------------------------------------------

extern "C" {
#include "Dlapi.h"
#include "minipath.h"
#include "resource.h"
}

extern "C" HWND      hwndMain;

extern "C" WCHAR     g_wchIniFile[MAX_PATH];
extern "C" WCHAR     g_wchIniFile2[MAX_PATH];
extern "C" WCHAR     g_wchNP3IniFile[MAX_PATH];

extern "C" BOOL      bSaveSettings;
extern "C" WCHAR     szQuickview[MAX_PATH];
extern "C" WCHAR     szQuickviewParams[MAX_PATH];
extern "C" WCHAR     g_tchFavoritesDir[MAX_PATH];
extern "C" BOOL      bNP3sFavoritesSettings;
extern "C" WCHAR     tchOpenWithDir[MAX_PATH];
extern "C" WCHAR     tchToolbarButtons[512];
extern "C" WCHAR     tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     tchToolbarBitmapDisabled[MAX_PATH];
extern "C" BOOL      bClearReadOnly;
extern "C" BOOL      bRenameOnCollision;
extern "C" BOOL      bSingleClick;
extern "C" BOOL      bTrackSelect;
extern "C" BOOL      bFullRowSelect;
extern "C" int       iStartupDir;
extern "C" int       iEscFunction;
extern "C" BOOL      bFocusEdit;
extern "C" BOOL      bAlwaysOnTop;
extern "C" BOOL      g_bTransparentMode;
extern "C" BOOL      bMinimizeToTray;
extern "C" BOOL      fUseRecycleBin;
extern "C" BOOL      fNoConfirmDelete;
extern "C" BOOL      bShowToolbar;
extern "C" BOOL      bShowStatusbar;
extern "C" BOOL      bShowDriveBox;
extern "C" int       cxGotoDlg;
extern "C" int       cxOpenWithDlg;
extern "C" int       cyOpenWithDlg;
extern "C" int       cxCopyMoveDlg;

extern "C" BOOL      bHasQuickview;

extern "C" WCHAR     tchFilter[DL_FILTER_BUFSIZE];
extern "C" BOOL      bNegFilter;
extern "C" BOOL      bDefCrNoFilt;
extern "C" BOOL      bDefCrFilter;
extern "C" COLORREF  crNoFilt;
extern "C" COLORREF  crFilter;
extern "C" COLORREF  crCustom[16];

extern "C" LPWSTR    lpPathArg;
extern "C" LPWSTR    lpFilterArg;

extern "C" WININFO   wi;

extern "C" WCHAR     szCurDir[MAX_PATH + 40];
extern "C" DWORD     dwFillMask;
extern "C" int       nSortFlags;
extern "C" BOOL      fSortRev;

extern "C" WCHAR     g_tchPrefLngLocName[LOCALE_NAME_MAX_LENGTH + 1];
extern "C" LANGID    g_iPrefLANGID;


//=============================================================================
//
// Flags
//
extern "C" int flagNoReuseWindow;
extern "C" int flagStartAsTrayIcon;
extern "C" int flagPortableMyDocs;
extern "C" int flagGotoFavorites;
extern "C" int flagNoFadeHidden;
extern "C" int flagToolbarLook;
extern "C" int flagPosParam;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static BOOL const s_bIsUTF8 = TRUE;
static BOOL const s_bUseMultiKey = FALSE;
static BOOL const s_bUseMultiLine = FALSE;
static BOOL const s_bSetSpaces = FALSE;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((BOOL)((RC) >= SI_OK))

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" BOOL LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" BOOL SaveIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.SetSpaces(s_bSetSpaces);
  SI_Error const rc = s_INI.SaveFile(lpIniFilePath, true);
  if (SI_SUCCESS(rc)) {
    s_INI.Reset(); // done
  }
  return SI_SUCCESS(rc);
}

extern "C" void ReleaseIniFile()
{
  s_INI.Reset();
}


//=============================================================================
//
//  Manipulation of (cached) INI file 
//
//=============================================================================


extern "C" size_t IniSectionGetString(LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpDefault,
                                      LPWSTR lpReturnedString, size_t cchReturnedString)
{
  bool bHasMultiple = false;
  StringCchCopyW(lpReturnedString, cchReturnedString,
    s_INI.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
  //assert(!bHasMultiple);
  return (size_t)lstrlen(lpReturnedString);
}
// ============================================================================


extern "C" int IniSectionGetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iDefault)
{
  bool bHasMultiple = false;
  auto const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
  //assert(!bHasMultiple);
  return iValue;
}
// ============================================================================


extern "C" double IniSectionGetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dDefault)
{
  bool bHasMultiple = false;
  double const dValue = s_INI.GetDoubleValue(lpSectionName, lpKeyName, dDefault, &bHasMultiple);
  //assert(!bHasMultiple);
  return dValue;
}
// ============================================================================


extern "C" BOOL IniSectionGetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, BOOL bDefault)
{
  bool bHasMultiple = false;
  BOOL const bValue = (BOOL)s_INI.GetBoolValue(lpSectionName, lpKeyName, (bool)bDefault, &bHasMultiple);
  //assert(!bHasMultiple);
  return bValue;
}
// ============================================================================


extern "C" BOOL IniSectionSetString(LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  SI_Error const rc = s_INI.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" BOOL IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" BOOL IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" BOOL IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" BOOL IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, BOOL bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" BOOL IniSectionDelete(LPCWSTR lpSectionName, LPCWSTR lpKeyName, BOOL bRemoveEmpty)
{
  return s_INI.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
}
// ============================================================================


extern "C" BOOL IniSectionClear(LPCWSTR lpSectionName, BOOL bRemoveEmpty)
{

  BOOL const ok = s_INI.Delete(lpSectionName, nullptr, bRemoveEmpty);
  if (!bRemoveEmpty) {
    SI_Error const rc = s_INI.SetValue(lpSectionName, nullptr, nullptr);
    return SI_SUCCESS(rc);
  }
  return ok;
}
// ============================================================================


extern "C" BOOL IniClearAllSections(LPCWSTR lpPrefix, BOOL bRemoveEmpty)
{
  auto const len = (size_t)lstrlen(lpPrefix);

  CSimpleIni::TNamesDepend Sections;
  s_INI.GetAllSections(Sections);
  for (const auto& section : Sections)
  {
    if (lstrcmpi(section.pItem, lpPrefix) == 0)
    {
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
    }
  }
  return TRUE;
}
// ============================================================================


// ============================================================================
// ============================================================================


extern "C" size_t IniFileGetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpDefault,
                                   LPWSTR lpReturnedString, size_t cchReturnedString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return (size_t)lstrlen(lpReturnedString);
}
// ============================================================================


extern "C" BOOL IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(lpFilePath, true);
    }
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" int IniFileGetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    int const iValue = Ini.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
    //assert(!bHasMultiple);
    return iValue;
  }
  return iDefault;
}
// ============================================================================


extern "C" BOOL IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(lpFilePath, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" BOOL IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, BOOL bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    BOOL const bValue = Ini.GetBoolValue(lpSectionName, lpKeyName, bDefault, &bHasMultiple);
    //assert(!bHasMultiple);
    return bValue;
  }
  return bDefault;
}
// ============================================================================


extern "C" BOOL IniFileSetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, BOOL bValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(lpFilePath, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" BOOL IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, BOOL bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(lpFilePath, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================



//=============================================================================
//
//  CreateIniFile()
//
//
int CreateIniFile()
{
  return(CreateIniFileEx(g_wchIniFile));
}

int CreateIniFileEx(LPCWSTR lpszIniFile) {

  if (lpszIniFile && *lpszIniFile) 
  {
    WCHAR* pwchTail = StrRChrW(lpszIniFile, nullptr, L'\\');
    if (pwchTail) {
      *pwchTail = 0;
      SHCreateDirectoryEx(nullptr, lpszIniFile, nullptr);
      *pwchTail = L'\\';
    }

    HANDLE hFile = CreateFile(lpszIniFile,
      GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
      nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
      if (GetFileSize(hFile, nullptr) == 0) {
        DWORD dw;
        WriteFile(hFile, (LPCVOID)L"\xFEFF[minipath]\r\n", 26, &dw, nullptr);
      }
      CloseHandle(hFile);
      return(1);
    }
    else {
      return(0);
    }
  }
  else {
    return(0);
  }
}

//=============================================================================
//
//  FindIniFile()
//
//
int CheckIniFile(LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tchFileExpanded[MAX_PATH];
  WCHAR tchBuild[MAX_PATH];
  ExpandEnvironmentStrings(lpszFile, tchFileExpanded, COUNTOF(tchFileExpanded));

  if (PathIsRelative(tchFileExpanded)) {
    // program directory
    lstrcpy(tchBuild, lpszModule);
    lstrcpy(PathFindFileName(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      lstrcpy(lpszFile, tchBuild);
      return 1;
    }
    // Sub directory (.\np3\) 
    lstrcpy(tchBuild, lpszModule);
    PathRemoveFileSpec(tchBuild);
    lstrcat(tchBuild, L"\\np3\\");
    lstrcat(tchBuild, tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      lstrcpy(lpszFile, tchBuild);
      return 1;
    }
    // Application Data (%APPDATA%)
    if (S_OK == SHGetFolderPath(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, tchBuild)) {
      PathAppend(tchBuild, tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        lstrcpy(lpszFile, tchBuild);
        return 1;
      }
    }
    // Home (%HOMEPATH%)
    if (S_OK == SHGetFolderPath(nullptr, CSIDL_PROFILE, nullptr, SHGFP_TYPE_CURRENT, tchBuild)) {
      PathAppend(tchBuild, tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        lstrcpy(lpszFile, tchBuild);
        return 1;
      }
    }
  }
  else if (PathFileExists(tchFileExpanded)) {
    lstrcpy(lpszFile, tchFileExpanded);
    return 1;
  }

  return 0;
}

int CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH];

  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (CheckIniFile(tch, lpszModule)) {
      lstrcpy(lpszFile, tch);
      return(1);
    }
    else {
      WCHAR tchFileExpanded[MAX_PATH];
      ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
      if (PathIsRelative(tchFileExpanded)) {
        lstrcpy(lpszFile, lpszModule);
        lstrcpy(PathFindFileName(lpszFile), tchFileExpanded);
        return(1);
      }
      else {
        lstrcpy(lpszFile, tchFileExpanded);
        return(1);
      }
    }
  }
  return(0);
}

int FindIniFile() {

  int bFound = 0;
  WCHAR tchTest[MAX_PATH];
  WCHAR tchModule[MAX_PATH];
  GetModuleFileName(nullptr, tchModule, COUNTOF(tchModule));

  if (StrIsNotEmpty(g_wchIniFile)) {
    if (lstrcmpi(g_wchIniFile, L"*?") == 0)
      return(0);
    else {
      if (!CheckIniFile(g_wchIniFile, tchModule)) {
        ExpandEnvironmentStringsEx(g_wchIniFile, COUNTOF(g_wchIniFile));
        if (PathIsRelative(g_wchIniFile)) {
          lstrcpy(tchTest, tchModule);
          PathRemoveFileSpec(tchTest);
          PathAppend(tchTest, g_wchIniFile);
          lstrcpy(g_wchIniFile, tchTest);
        }
      }
    }
    return(1);
  }

  lstrcpy(tchTest, PathFindFileName(tchModule));
  PathRenameExtension(tchTest, L".ini");
  bFound = CheckIniFile(tchTest, tchModule);

  if (!bFound) {
    lstrcpy(tchTest, L"minipath.ini");
    bFound = CheckIniFile(tchTest, tchModule);
  }

  if (bFound) {
    // allow two redirections: administrator -> user -> custom
    if (CheckIniFileRedirect(L"minipath", L"minipath.ini", tchTest, tchModule))
      CheckIniFileRedirect(L"minipath", L"minipath.ini", tchTest, tchModule);
    lstrcpy(g_wchIniFile, tchTest);
  }
  else {
    lstrcpy(g_wchIniFile, tchModule);
    PathRenameExtension(g_wchIniFile, L".ini");
  }

  // --- check for Notepad3.ini to synchronize some settings ---
  PathRemoveFileSpec(tchModule);
  lstrcat(tchModule, L"\\Notepad3.exe");
  lstrcpy(tchTest, PathFindFileName(tchModule));
  PathRenameExtension(tchTest, L".ini");
  bFound = CheckIniFile(tchTest, tchModule);
  if (!bFound) {
    lstrcpy(tchTest, L"notepad3.ini");
    bFound = CheckIniFile(tchTest, tchModule);
  }
  if (bFound) {
    // allow two redirections: administrator -> user -> custom
    if (CheckIniFileRedirect(L"notepad3", L"notepad3.ini", tchTest, tchModule)) {
      CheckIniFileRedirect(L"notepad3", L"notepad3.ini", tchTest, tchModule);
    }
    lstrcpy(g_wchNP3IniFile, tchTest);
  }
  else {
    lstrcpy(g_wchNP3IniFile, tchModule);
    PathRenameExtension(g_wchNP3IniFile, L".ini");
  }
  return (bFound ? 1 : 0);
}


int TestIniFile() {

  if (lstrcmpi(g_wchIniFile, L"*?") == 0) {
    lstrcpy(g_wchIniFile2, L"");
    lstrcpy(g_wchIniFile, L"");
    return(0);
  }

  if (PathIsDirectory(g_wchIniFile) || *CharPrev(g_wchIniFile, StrEnd(g_wchIniFile)) == L'\\') {
    WCHAR wchModule[MAX_PATH];
    GetModuleFileName(nullptr, wchModule, COUNTOF(wchModule));
    PathAppend(g_wchIniFile, PathFindFileName(wchModule));
    PathRenameExtension(g_wchIniFile, L".ini");
    if (!PathFileExists(g_wchIniFile)) {
      lstrcpy(PathFindFileName(g_wchIniFile), L"minipath.ini");
      if (!PathFileExists(g_wchIniFile)) {
        lstrcpy(PathFindFileName(g_wchIniFile), PathFindFileName(wchModule));
        PathRenameExtension(g_wchIniFile, L".ini");
      }
    }
  }
  // --- test for Notepad3.ini ---
  if (PathIsDirectory(g_wchNP3IniFile) || *CharPrev(g_wchNP3IniFile, StrEnd(g_wchNP3IniFile)) == L'\\') {
    WCHAR wchModule[MAX_PATH];
    GetModuleFileName(nullptr, wchModule, COUNTOF(wchModule));
    PathRemoveFileSpec(wchModule);
    lstrcat(wchModule, L"\\Notepad3.exe");
    PathAppend(g_wchNP3IniFile, PathFindFileName(wchModule));
    PathRenameExtension(g_wchNP3IniFile, L".ini");
    if (!PathFileExists(g_wchNP3IniFile)) {
      lstrcpy(PathFindFileName(g_wchNP3IniFile), L"notepad3.ini");
      if (!PathFileExists(g_wchNP3IniFile)) {
        lstrcpy(PathFindFileName(g_wchNP3IniFile), PathFindFileName(wchModule));
        PathRenameExtension(g_wchNP3IniFile, L".ini");
      }
    }
  }
  if (!PathFileExists(g_wchNP3IniFile) || PathIsDirectory(g_wchNP3IniFile)) {
    lstrcpy(g_wchNP3IniFile, L"");
  }

  if (!PathFileExists(g_wchIniFile) || PathIsDirectory(g_wchIniFile)) {
    lstrcpy(g_wchIniFile2, g_wchIniFile);
    lstrcpy(g_wchIniFile, L"");
    return(0);
  }
  else
    return(1);
}


//=============================================================================
//
//  LoadFlags()
//
//
void LoadFlags()
{
  LoadIniFile(g_wchIniFile);

  const WCHAR* const Settings_Section2 = L"Settings2";

  if (!IniSectionGetString(Settings_Section2, L"PreferredLanguageLocaleName", L"",
    g_tchPrefLngLocName, COUNTOF(g_tchPrefLngLocName)))
  {
    // try to fetch Locale Name from Notepad3.ini
    IniFileGetString(g_wchNP3IniFile, L"Settings2", L"PreferredLanguageLocaleName", L"",
      g_tchPrefLngLocName, COUNTOF(g_tchPrefLngLocName));
  }

  if (!flagNoReuseWindow) {

    if (!IniSectionGetInt(Settings_Section2, L"ReuseWindow", 1))
      flagNoReuseWindow = 1;
  }

  if (IniSectionGetInt(Settings_Section2, L"PortableMyDocs", 1))
    flagPortableMyDocs = 1;

  if (IniSectionGetInt(Settings_Section2, L"NoFadeHidden", 0))
    flagNoFadeHidden = 1;

  flagToolbarLook = IniSectionGetInt(Settings_Section2, L"ToolbarLook", 0);
  flagToolbarLook = max(min(flagToolbarLook, 2), 0);

  ReleaseIniFile();
}



//=============================================================================
//
//  LoadSettings()
//
//
void LoadSettings()
{
  LoadIniFile(g_wchIniFile);

  const WCHAR* const Settings_Section = L"Settings";

  bSaveSettings = IniSectionGetInt(Settings_Section, L"SaveSettings", 1);
  if (bSaveSettings) bSaveSettings = 1;

  bSingleClick = IniSectionGetInt(Settings_Section, L"SingleClick", 1);
  if (bSingleClick) bSingleClick = 1;

  bTrackSelect = IniSectionGetInt(Settings_Section, L"TrackSelect", 1);
  if (bTrackSelect) bTrackSelect = 1;

  bFullRowSelect = IniSectionGetInt(Settings_Section, L"FullRowSelect", 0);
  if (bFullRowSelect) bFullRowSelect = 1;

  fUseRecycleBin = IniSectionGetInt(Settings_Section, L"UseRecycleBin", 0);
  if (fUseRecycleBin) fUseRecycleBin = 1;

  fNoConfirmDelete = IniSectionGetInt(Settings_Section, L"NoConfirmDelete", 0);
  if (fNoConfirmDelete) fNoConfirmDelete = 1;

  bClearReadOnly = IniSectionGetInt(Settings_Section, L"ClearReadOnly", 1);
  if (bClearReadOnly) bClearReadOnly = 1;

  bRenameOnCollision = IniSectionGetInt(Settings_Section, L"RenameOnCollision", 0);
  if (bRenameOnCollision) bRenameOnCollision = 1;

  bFocusEdit = IniSectionGetInt(Settings_Section, L"FocusEdit", 1);
  if (bFocusEdit) bFocusEdit = 1;

  bAlwaysOnTop = IniSectionGetInt(Settings_Section, L"AlwaysOnTop", 0);
  if (bAlwaysOnTop) bAlwaysOnTop = 1;

  bMinimizeToTray = IniSectionGetInt(Settings_Section, L"MinimizeToTray", 0);
  if (bMinimizeToTray) bMinimizeToTray = 1;

  g_bTransparentMode = IniSectionGetInt(Settings_Section, L"TransparentMode", 0);
  if (g_bTransparentMode) g_bTransparentMode = 1;

  iEscFunction = IniSectionGetInt(Settings_Section, L"EscFunction", 2);
  iEscFunction = max(min(iEscFunction, 2), 0);

  iStartupDir = IniSectionGetInt(Settings_Section, L"StartupDirectory", 2);
  iStartupDir = max(min(iStartupDir, 2), 0);

  if (!IniSectionGetString(Settings_Section, L"Favorites", L"",
    g_tchFavoritesDir, COUNTOF(g_tchFavoritesDir))) {
    // try to fetch Favorites dir from Notepad3.ini
    if (StrIsNotEmpty(g_wchNP3IniFile)) {
      bNP3sFavoritesSettings = TRUE;
      IniFileGetString(g_wchNP3IniFile, L"Settings", L"Favorites", L"", g_tchFavoritesDir, COUNTOF(g_tchFavoritesDir));
    }
  }
  if (StrIsEmpty(g_tchFavoritesDir))
    SHGetFolderPath(nullptr, CSIDL_PERSONAL, nullptr, SHGFP_TYPE_CURRENT, g_tchFavoritesDir);
  else
    PathAbsoluteFromApp(g_tchFavoritesDir, nullptr, COUNTOF(g_tchFavoritesDir), TRUE);

  if (!IniSectionGetString(Settings_Section, L"Quikview.exe", L"",
    szQuickview, COUNTOF(szQuickview))) {
    GetSystemDirectory(szQuickview, COUNTOF(szQuickview));
    PathAddBackslash(szQuickview);
    lstrcat(szQuickview, L"Viewers\\Quikview.exe");
  }
  else
    PathAbsoluteFromApp(szQuickview, nullptr, COUNTOF(szQuickview), TRUE);

  bHasQuickview = PathFileExists(szQuickview);

  IniSectionGetString(Settings_Section, L"QuikviewParams", L"", szQuickviewParams, COUNTOF(szQuickviewParams));

  if (!IniSectionGetString(Settings_Section, L"OpenWithDir", L"", tchOpenWithDir, COUNTOF(tchOpenWithDir))) {
    // try to fetch Open With dir from Notepad3.ini
    IniFileGetString(g_wchNP3IniFile, L"Settings", L"OpenWithDir", L"", tchOpenWithDir, COUNTOF(tchOpenWithDir));
  }
  if (StrIsEmpty(tchOpenWithDir))
    SHGetSpecialFolderPath(nullptr, tchOpenWithDir, CSIDL_DESKTOPDIRECTORY, TRUE);
  else
    PathAbsoluteFromApp(tchOpenWithDir, nullptr, COUNTOF(tchOpenWithDir), TRUE);

  dwFillMask = IniSectionGetInt(Settings_Section, L"FillMask", DL_ALLOBJECTS);
  if (dwFillMask & ~DL_ALLOBJECTS) dwFillMask = DL_ALLOBJECTS;

  nSortFlags = IniSectionGetInt(Settings_Section, L"SortOptions", DS_TYPE);
  nSortFlags = min(3, max(nSortFlags, 0));

  fSortRev = IniSectionGetInt(Settings_Section, L"SortReverse", 0);
  if (fSortRev) fSortRev = 1;

  if (!lpFilterArg) {
    if (!IniSectionGetString(Settings_Section, L"FileFilter", L"",
      tchFilter, COUNTOF(tchFilter)))
      lstrcpy(tchFilter, L"*.*");

    bNegFilter = IniSectionGetInt(Settings_Section, L"NegativeFilter", 0);
    if (bNegFilter) bNegFilter = 1;
  }

  else { // ignore filter if /m was specified
    if (*lpFilterArg == L'-') {
      bNegFilter = TRUE;
      lstrcpyn(tchFilter, lpFilterArg + 1, COUNTOF(tchFilter));
    }
    else {
      bNegFilter = FALSE;
      lstrcpyn(tchFilter, lpFilterArg, COUNTOF(tchFilter));
    }
  }

  bDefCrNoFilt = IniSectionGetInt(Settings_Section, L"DefColorNoFilter", 1);
  if (bDefCrNoFilt) bDefCrNoFilt = 1;
  bDefCrFilter = IniSectionGetInt(Settings_Section, L"DefColorFilter", 1);
  if (bDefCrFilter) bDefCrFilter = 1;

  crNoFilt = IniSectionGetInt(Settings_Section, L"ColorNoFilter", GetSysColor(COLOR_WINDOWTEXT));
  crFilter = IniSectionGetInt(Settings_Section, L"ColorFilter", GetSysColor(COLOR_HIGHLIGHT));

  if (IniSectionGetString(Settings_Section, L"ToolbarButtons", L"", tchToolbarButtons, COUNTOF(tchToolbarButtons)) == 0) {
    lstrcpy(tchToolbarButtons, L"1 2 3 4 5 0 8");
  }
  bShowToolbar = IniSectionGetInt(Settings_Section, L"ShowToolbar", 1);
  if (bShowToolbar) bShowToolbar = 1;

  bShowStatusbar = IniSectionGetInt(Settings_Section, L"ShowStatusbar", 1);
  if (bShowStatusbar) bShowStatusbar = 1;

  bShowDriveBox = IniSectionGetInt(Settings_Section, L"ShowDriveBox", 1);
  if (bShowDriveBox) bShowDriveBox = 1;

  cxGotoDlg = IniSectionGetInt(Settings_Section, L"GotoDlgSizeX", 0);
  cxGotoDlg = max(cxGotoDlg, 0);

  cxOpenWithDlg = IniSectionGetInt(Settings_Section, L"OpenWithDlgSizeX", 0);
  cxOpenWithDlg = max(cxOpenWithDlg, 0);

  cyOpenWithDlg = IniSectionGetInt(Settings_Section, L"OpenWithDlgSizeY", 0);
  cyOpenWithDlg = max(cyOpenWithDlg, 0);

  cxCopyMoveDlg = IniSectionGetInt(Settings_Section, L"CopyMoveDlgSizeX", 0);
  cxCopyMoveDlg = max(cxCopyMoveDlg, 0);

  int ResX = GetSystemMetrics(SM_CXSCREEN);
  int ResY = GetSystemMetrics(SM_CYSCREEN);

  
  const WCHAR* const ToolbarImages_Section = L"Toolbar Images";

  IniSectionGetString(ToolbarImages_Section, L"BitmapDefault", L"",
    tchToolbarBitmap, COUNTOF(tchToolbarBitmap));
  IniSectionGetString(ToolbarImages_Section, L"BitmapHot", L"",
    tchToolbarBitmapHot, COUNTOF(tchToolbarBitmap));
  IniSectionGetString(ToolbarImages_Section, L"BitmapDisabled", L"",
    tchToolbarBitmapDisabled, COUNTOF(tchToolbarBitmap));

  if (!flagPosParam) { // ignore window position if /p was specified

    WCHAR tchPosX[32], tchPosY[32], tchSizeX[32], tchSizeY[32];

    wsprintf(tchPosX, L"%ix%i PosX", ResX, ResY);
    wsprintf(tchPosY, L"%ix%i PosY", ResX, ResY);
    wsprintf(tchSizeX, L"%ix%i SizeX", ResX, ResY);
    wsprintf(tchSizeY, L"%ix%i SizeY", ResX, ResY);

    const WCHAR* const Window_Section = L"Window";

    wi.x = IniSectionGetInt(Window_Section, tchPosX, CW_USEDEFAULT);
    wi.y = IniSectionGetInt(Window_Section, tchPosY, CW_USEDEFAULT);
    wi.cx = IniSectionGetInt(Window_Section, tchSizeX, CW_USEDEFAULT);
    wi.cy = IniSectionGetInt(Window_Section, tchSizeY, CW_USEDEFAULT);
  }


  // Initialize custom colors for ChooseColor()
  crCustom[0] = RGB(0, 0, 128);                     crCustom[8] = RGB(255, 255, 226);
  crCustom[1] = GetSysColor(COLOR_WINDOWTEXT);    crCustom[9] = GetSysColor(COLOR_WINDOW);
  crCustom[2] = GetSysColor(COLOR_INFOTEXT);      crCustom[10] = GetSysColor(COLOR_INFOBK);
  crCustom[3] = GetSysColor(COLOR_HIGHLIGHTTEXT); crCustom[11] = GetSysColor(COLOR_HIGHLIGHT);
  crCustom[4] = GetSysColor(COLOR_ACTIVECAPTION); crCustom[12] = GetSysColor(COLOR_DESKTOP);
  crCustom[5] = GetSysColor(COLOR_3DFACE);        crCustom[13] = GetSysColor(COLOR_3DFACE);
  crCustom[6] = GetSysColor(COLOR_3DFACE);        crCustom[14] = GetSysColor(COLOR_3DFACE);
  crCustom[7] = GetSysColor(COLOR_3DFACE);        crCustom[15] = GetSysColor(COLOR_3DFACE);

  ReleaseIniFile();
}

//=============================================================================
//
//  SaveSettings()
//
//
void SaveSettings(BOOL bSaveSettingsNow)
{
  WCHAR wchTmp[MAX_PATH];

  if (StrIsEmpty(g_wchIniFile)) { return; }

  CreateIniFile();

  if (!bSaveSettings && !bSaveSettingsNow) {
    if (iStartupDir == 1) {
      IniFileSetString(g_wchIniFile, L"Settings", L"MRUDirectory", szCurDir);
    }
    IniFileSetBool(g_wchIniFile, L"Settings", L"SaveSettings", bSaveSettings);
    return;
  }

  LoadIniFile(g_wchIniFile);

  const WCHAR* const Settings_Section = L"Settings";

  IniSectionSetInt(Settings_Section, L"SaveSettings", bSaveSettings);
  IniSectionSetInt(Settings_Section, L"SingleClick", bSingleClick);
  IniSectionSetInt(Settings_Section, L"TrackSelect", bTrackSelect);
  IniSectionSetInt(Settings_Section, L"FullRowSelect", bFullRowSelect);
  IniSectionSetInt(Settings_Section, L"UseRecycleBin", fUseRecycleBin);
  IniSectionSetInt(Settings_Section, L"NoConfirmDelete", fNoConfirmDelete);
  IniSectionSetInt(Settings_Section, L"ClearReadOnly", bClearReadOnly);
  IniSectionSetInt(Settings_Section, L"RenameOnCollision", bRenameOnCollision);
  IniSectionSetInt(Settings_Section, L"FocusEdit", bFocusEdit);
  IniSectionSetInt(Settings_Section, L"AlwaysOnTop", bAlwaysOnTop);
  IniSectionSetInt(Settings_Section, L"MinimizeToTray", bMinimizeToTray);
  IniSectionSetInt(Settings_Section, L"TransparentMode", g_bTransparentMode);
  IniSectionSetInt(Settings_Section, L"EscFunction", iEscFunction);
  IniSectionSetInt(Settings_Section, L"StartupDirectory", iStartupDir);
  if (iStartupDir == 1)
    IniSectionSetString(Settings_Section, L"MRUDirectory", szCurDir);
  if (!bNP3sFavoritesSettings) {
    PathRelativeToApp(g_tchFavoritesDir, wchTmp, COUNTOF(wchTmp), FALSE, TRUE, flagPortableMyDocs);
    IniSectionSetString(Settings_Section, L"Favorites", wchTmp);
  }
  PathRelativeToApp(szQuickview, wchTmp, COUNTOF(wchTmp), FALSE, TRUE, flagPortableMyDocs);
  IniSectionSetString(Settings_Section, L"Quikview.exe", wchTmp);
  IniSectionSetString(Settings_Section, L"QuikviewParams", szQuickviewParams);
  PathRelativeToApp(tchOpenWithDir, wchTmp, COUNTOF(wchTmp), FALSE, TRUE, flagPortableMyDocs);
  IniSectionSetString(Settings_Section, L"OpenWithDir", wchTmp);
  IniSectionSetInt(Settings_Section, L"FillMask", dwFillMask);
  IniSectionSetInt(Settings_Section, L"SortOptions", nSortFlags);
  IniSectionSetInt(Settings_Section, L"SortReverse", fSortRev);
  IniSectionSetString(Settings_Section, L"FileFilter", tchFilter);
  IniSectionSetInt(Settings_Section, L"NegativeFilter", bNegFilter);
  IniSectionSetInt(Settings_Section, L"DefColorNoFilter", bDefCrNoFilt);
  IniSectionSetInt(Settings_Section, L"DefColorFilter", bDefCrFilter);
  IniSectionSetInt(Settings_Section, L"ColorNoFilter", crNoFilt);
  IniSectionSetInt(Settings_Section, L"ColorFilter", crFilter);
  IniSectionSetString(Settings_Section, L"ToolbarButtons", tchToolbarButtons);
  IniSectionSetInt(Settings_Section, L"ShowToolbar", bShowToolbar);
  IniSectionSetInt(Settings_Section, L"ShowStatusbar", bShowStatusbar);
  IniSectionSetInt(Settings_Section, L"ShowDriveBox", bShowDriveBox);
  IniSectionSetInt(Settings_Section, L"GotoDlgSizeX", cxGotoDlg);
  IniSectionSetInt(Settings_Section, L"OpenWithDlgSizeX", cxOpenWithDlg);
  IniSectionSetInt(Settings_Section, L"OpenWithDlgSizeY", cyOpenWithDlg);
  IniSectionSetInt(Settings_Section, L"CopyMoveDlgSizeX", cxCopyMoveDlg);

  /*
    SaveSettingsNow(): query Window Dimensions
  */

  if (bSaveSettingsNow)
  {
    WINDOWPLACEMENT wndpl;

    // GetWindowPlacement
    wndpl.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(hwndMain, &wndpl);

    wi.x = wndpl.rcNormalPosition.left;
    wi.y = wndpl.rcNormalPosition.top;
    wi.cx = wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left;
    wi.cy = wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top;
  }

  WCHAR tchPosX[32], tchPosY[32], tchSizeX[32], tchSizeY[32], tchMaximized[32];
  int ResX = GetSystemMetrics(SM_CXSCREEN);
  int ResY = GetSystemMetrics(SM_CYSCREEN);
  wsprintf(tchPosX, L"%ix%i PosX", ResX, ResY);
  wsprintf(tchPosY, L"%ix%i PosY", ResX, ResY);
  wsprintf(tchSizeX, L"%ix%i SizeX", ResX, ResY);
  wsprintf(tchSizeY, L"%ix%i SizeY", ResX, ResY);
  wsprintf(tchMaximized, L"%ix%i Maximized", ResX, ResY);

  const WCHAR* const Windows_Section = L"Window";

  IniSectionSetInt(Windows_Section, tchPosX, wi.x);
  IniSectionSetInt(Windows_Section, tchPosY, wi.y);
  IniSectionSetInt(Windows_Section, tchSizeX, wi.cx);
  IniSectionSetInt(Windows_Section, tchSizeY, wi.cy);

  SaveIniFile(g_wchIniFile);
}

//=============================================================================
