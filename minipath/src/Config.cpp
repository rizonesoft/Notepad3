// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* MiniPath                                                                    *
*                                                                             *
* Config.cpp                                                                  *
*   Methods to read and write configuration                                   *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2020   *
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
#include "Dialogs.h"
#include "minipath.h"
#include "resource.h"
}

extern "C" HWND      hwndMain;
extern "C" WCHAR     g_wchIniFile[MAX_PATH];
extern "C" WCHAR     g_wchIniFile2[MAX_PATH];
extern "C" WCHAR     g_wchNP3IniFile[MAX_PATH];

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

#include "..\..\src\Config\SimpleIni.h"
#include "Config.h"

// ============================================================================

static BOOL const s_bIsUTF8 = TRUE;
static BOOL const s_bWriteSIG = TRUE;     // BOM
static BOOL const s_bUseMultiKey = FALSE;
static BOOL const s_bUseMultiLine = FALSE;
static BOOL const s_bSetSpaces = FALSE;

// ----------------------------------------------------------------------------

constexpr bool SI_Success(const SI_Error rc) noexcept { 
  return ((rc == SI_Error::SI_OK) || (rc == SI_Error::SI_UPDATED) || (rc == SI_Error::SI_INSERTED)); 
};

// ============================================================================

// ----------------------------------------------------------------------------
// No mechanism for  EXCLUSIVE WRITE / SHARD READ:
// cause we need completely synchronized exclusive access for READ _and_ WRITE
// of complete file to preserve integrity of any transaction
// ----------------------------------------------------------------------------

HANDLE AcquireWriteFileLock(LPCWSTR lpIniFilePath, OVERLAPPED& rOvrLpd)
{
  if (StrIsEmpty(lpIniFilePath)) { return INVALID_HANDLE_VALUE; }
  
  bool bLocked = false;

  HANDLE hFile = CreateFile(lpIniFilePath, 
    GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
    OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

  if (hFile != INVALID_HANDLE_VALUE)
  {
    bLocked = LockFileEx(hFile, LOCKFILE_EXCLUSIVE_LOCK, 0, MAXDWORD, 0, &rOvrLpd); // wait for exclusive lock
    if (!bLocked) {
      MsgBoxLastError(L"AcquireWriteFileLock(): NO EXCLUSIVE LOCK ACQUIRED!", 0);
    }
  }
  else {
    MsgBoxLastError(L"AcquireWriteFileLock(): INVALID FILE HANDLE!", 0);
  }
  return (bLocked ? hFile : INVALID_HANDLE_VALUE);
}

// ----------------------------------------------------------------------------

#define LOCKFILE_SHARED_LOCK (0x00000000)

HANDLE AcquireReadFileLock(LPCWSTR lpIniFilePath, OVERLAPPED& rOvrLpd)
{
  if (StrIsEmpty(lpIniFilePath)) { return INVALID_HANDLE_VALUE; }

  bool bLocked = false;

  HANDLE hFile = CreateFile(lpIniFilePath,
    GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

  if (hFile != INVALID_HANDLE_VALUE) 
  {
    bLocked = LockFileEx(hFile, LOCKFILE_SHARED_LOCK, 0, MAXDWORD, 0, &rOvrLpd);
    if (!bLocked) {
      wchar_t msg[MAX_PATH + 128] = { 0 };
      StringCchPrintf(msg, ARRAYSIZE(msg),
        L"AcquireReadFileLock(%s): NO READER LOCK ACQUIRED!", lpIniFilePath);
      MsgBoxLastError(msg, 0);
    }
  }
  else {
    wchar_t msg[MAX_PATH + 128] = { 0 };
    StringCchPrintf(msg, ARRAYSIZE(msg),
      L"AcquireReadFileLock(%s): INVALID FILE HANDLE!", lpIniFilePath);
    MsgBoxLastError(msg, 0);
  }
  return (bLocked ? hFile : INVALID_HANDLE_VALUE);
}

// ----------------------------------------------------------------------------

BOOL ReleaseFileLock(HANDLE hFile, OVERLAPPED& rOvrLpd)
{
  bool bUnLocked = true;
  if (hFile != INVALID_HANDLE_VALUE) {
    FlushFileBuffers(hFile);
    bUnLocked = !UnlockFileEx(hFile, 0, MAXDWORD, 0, &rOvrLpd);
    CloseHandle(hFile);
  }
  return bUnLocked;
}

// ============================================================================

static BOOL s_bIniFileCacheLoaded = FALSE;
static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);

extern "C" BOOL ResetIniFileCache() {
  s_INI.Reset();
  s_bIniFileCacheLoaded = false;
  return true;
}


extern "C" BOOL LoadIniFileCache(LPCWSTR lpIniFilePath)
{
  if (StrIsEmpty(lpIniFilePath) || !PathIsExistingFile(lpIniFilePath)) { return FALSE; }

  ResetIniFileCache();
  
  s_INI.SetSpaces(s_bSetSpaces);
  s_INI.SetMultiLine(s_bUseMultiLine);

  OVERLAPPED ovrLpd = { 0 };
  HANDLE hIniFile = AcquireReadFileLock(lpIniFilePath, ovrLpd);

  if (hIniFile == INVALID_HANDLE_VALUE) {
    return false;
  }

  s_bIniFileCacheLoaded = SI_Success(s_INI.LoadFile(hIniFile));

  ReleaseFileLock(hIniFile, ovrLpd);

  return s_bIniFileCacheLoaded;
}


extern "C" BOOL IsIniFileCached() { return s_bIniFileCacheLoaded; }


extern "C" BOOL SaveIniFileCache(LPCWSTR lpIniFilePath)
{
  if (!s_bIniFileCacheLoaded) { return false; }

  OVERLAPPED ovrLpd = { 0 };
  HANDLE hIniFile = AcquireWriteFileLock(lpIniFilePath, ovrLpd);

  if (hIniFile == INVALID_HANDLE_VALUE) {
    return false;
}

  s_INI.SetSpaces(s_bSetSpaces);
  s_INI.SetMultiLine(s_bUseMultiLine);

  bool const res = SI_Success(s_INI.SaveFile(hIniFile, s_bWriteSIG));

  ReleaseFileLock(hIniFile, ovrLpd);

  return res;
}


//=============================================================================
//
//  Manipulation of (cached) ini file 
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


extern "C" long IniSectionGetLong(LPCWSTR lpSectionName, LPCWSTR lpKeyName, long lDefault)
{
  bool bHasMultiple = false;
  auto const lValue = s_INI.GetLongValue(lpSectionName, lpKeyName, lDefault, &bHasMultiple);
  //assert(!bHasMultiple);
  return lValue;
}
// ============================================================================


extern "C" long long IniSectionGetLongLong(LPCWSTR lpSectionName, LPCWSTR lpKeyName, long long llDefault)
{
  bool bHasMultiple = false;
  auto const lValue = s_INI.GetLongLongValue(lpSectionName, lpKeyName, llDefault, &bHasMultiple);
  //assert(!bHasMultiple);
  return lValue;
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
  return SI_Success(rc);
}
// ============================================================================


extern "C" BOOL IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_Success(rc);
}

extern "C" BOOL IniSectionSetLong(LPCWSTR lpSectionName, LPCWSTR lpKeyName, long lValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, lValue, nullptr, false, !s_bUseMultiKey);
  return SI_Success(rc);
}

extern "C" BOOL IniSectionSetLongLong(LPCWSTR lpSectionName, LPCWSTR lpKeyName, long long llValue)
{
  SI_Error const rc = s_INI.SetLongLongValue(lpSectionName, lpKeyName, llValue, nullptr, false, !s_bUseMultiKey);
  return SI_Success(rc);
}

extern "C" BOOL IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_Success(rc);
}
// ============================================================================


extern "C" BOOL IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_Success(rc);
}
// ============================================================================


extern "C" BOOL IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, BOOL bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_Success(rc);
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
    return SI_Success(rc);
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
      IniSectionClear(section.pItem, bRemoveEmpty);
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
  if (StrIsEmpty(lpFilePath)) {
    StringCchCopyW(lpReturnedString, cchReturnedString, lpDefault);
    return (size_t)lstrlen(lpReturnedString);
  }

  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);

  OVERLAPPED ovrLpd = { 0 };
  HANDLE hFile = AcquireReadFileLock(lpFilePath, ovrLpd);
  if (hFile == INVALID_HANDLE_VALUE) {
    StringCchCopyW(lpReturnedString, cchReturnedString, lpDefault);
    return (size_t)lstrlen(lpReturnedString);
  }

  SI_Error const rc = Ini.LoadFile(hFile);
  ReleaseFileLock(hFile, ovrLpd);

  if (SI_Success(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  else {
    StringCchCopyW(lpReturnedString, cchReturnedString, lpDefault);
  }
  return (size_t)lstrlen(lpReturnedString);
}
// ============================================================================


extern "C" BOOL IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  if (StrIsEmpty(lpFilePath)) {
    return false;
  }

  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  Ini.SetSpaces(s_bSetSpaces);

  OVERLAPPED ovrLpd = { 0 };
  HANDLE hFile = AcquireWriteFileLock(lpFilePath, ovrLpd);
  if (hFile == INVALID_HANDLE_VALUE) {
    return false;
  }

  SI_Error rc = Ini.LoadFile(hFile);
  if (SI_Success(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_Success(res) ? SI_Error::SI_OK : SI_Error::SI_FAIL;
    if (SI_Success(rc)) {
      rc = Ini.SaveFile(hFile, s_bWriteSIG);
    }
  }
  ReleaseFileLock(hFile, ovrLpd);

  return SI_Success(rc);
}
// ============================================================================


extern "C" int IniFileGetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iDefault)
{
  if (StrIsEmpty(lpFilePath)) {
    return iDefault;
  }

  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);

  OVERLAPPED ovrLpd = { 0 };
  HANDLE hFile = AcquireReadFileLock(lpFilePath, ovrLpd);
  if (hFile == INVALID_HANDLE_VALUE) {
    return iDefault;
  }

  SI_Error rc = Ini.LoadFile(hFile);
  ReleaseFileLock(hFile, ovrLpd);

  if (SI_Success(rc))
  {
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
  if (StrIsEmpty(lpFilePath)) {
    return false;
  }

  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  Ini.SetSpaces(s_bSetSpaces);

  OVERLAPPED ovrLpd = { 0 };
  HANDLE hFile = AcquireWriteFileLock(lpFilePath, ovrLpd);
  if (hFile == INVALID_HANDLE_VALUE) {
    return false;
  }

  SI_Error rc = Ini.LoadFile(hFile);
  if (SI_Success(rc))
  {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    rc = Ini.SaveFile(hFile, s_bWriteSIG);
  }
  ReleaseFileLock(hFile, ovrLpd);

  return SI_Success(rc);
}
// ============================================================================


extern "C" BOOL IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, BOOL bDefault)
{
  if (StrIsEmpty(lpFilePath)) {
    return bDefault;
  }

  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);

  OVERLAPPED ovrLpd = { 0 };
  HANDLE hFile = AcquireReadFileLock(lpFilePath, ovrLpd);
  if (hFile == INVALID_HANDLE_VALUE) {
    return bDefault;
  }

  SI_Error rc = Ini.LoadFile(hFile);
  ReleaseFileLock(hFile, ovrLpd);
  
  if (SI_Success(rc))
  {
    bool bHasMultiple = false;
    bool const bValue = Ini.GetBoolValue(lpSectionName, lpKeyName, bDefault, &bHasMultiple);
    //assert(!bHasMultiple);
    return bValue;
  }
  return bDefault;
}
// ============================================================================


extern "C" BOOL IniFileSetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, BOOL bValue)
{
  if (StrIsEmpty(lpFilePath)) {
    return false;
  }

  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  Ini.SetSpaces(s_bSetSpaces);

  OVERLAPPED ovrLpd = { 0 };
  HANDLE hFile = AcquireWriteFileLock(lpFilePath, ovrLpd);
  if (hFile == INVALID_HANDLE_VALUE) {
    return false;
  }

  SI_Error rc = Ini.LoadFile(hFile);
  if (SI_Success(rc))
  {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    rc = Ini.SaveFile(hFile, s_bWriteSIG);
  }
  ReleaseFileLock(hFile, ovrLpd);

  return SI_Success(rc);
}
// ============================================================================


extern "C" BOOL IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, BOOL bRemoveEmpty)
{
  if (StrIsEmpty(lpFilePath)) {
    return false;
  }

  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  Ini.SetSpaces(s_bSetSpaces);

  OVERLAPPED ovrLpd = { 0 };
  HANDLE hFile = AcquireWriteFileLock(lpFilePath, ovrLpd);
  if (hFile == INVALID_HANDLE_VALUE) {
    return false;
  }

  SI_Error rc = Ini.LoadFile(hFile);
  if (SI_Success(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    rc = Ini.SaveFile(hFile, s_bWriteSIG);
  }
  ReleaseFileLock(hFile, ovrLpd);

  return SI_Success(rc);
}
// ============================================================================


extern "C" BOOL IniFileIterateSection(LPCWSTR lpFilePath, LPCWSTR lpSectionName, IterSectionFunc_t callBack)
{
  if (StrIsEmpty(lpFilePath)) {
    return false;
  }

  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);

  OVERLAPPED ovrLpd = { 0 };
  HANDLE hFile = AcquireReadFileLock(lpFilePath, ovrLpd);
  if (hFile == INVALID_HANDLE_VALUE) {
    return false;
  }

  SI_Error rc = Ini.LoadFile(hFile);
  ReleaseFileLock(hFile, ovrLpd);

  if (SI_Success(rc))
  {
    bool bHasMultiple = false;

    // get all keys in a section
    CSimpleIniW::TNamesDepend keyList;
    Ini.GetAllKeys(lpSectionName, keyList);
    keyList.sort(CSimpleIniW::Entry::LoadOrder());

    for (const auto& key : keyList)
    {
      callBack(key.pItem, Ini.GetValue(lpSectionName, key.pItem, L"", &bHasMultiple));
    }
  }
  return SI_Success(rc);
}
// ============================================================================



//=============================================================================
//
//  InitDefaultSettings()
//
//
void InitDefaultSettings()
{
  Defaults.szQuickview[0] = L'\0';
  Defaults.szQuickviewParams[0] = L'\0';
  Defaults.g_tchFavoritesDir[0] = L'\0';
  Defaults.tchOpenWithDir[0] = L'\0';
  Defaults.tchToolbarButtons[0] = L'\0';
  Defaults.tchToolbarBitmap[0] = L'\0';
  Defaults.tchToolbarBitmapHot[0] = L'\0';
  Defaults.tchToolbarBitmapDisabled[0] = L'\0';
  Defaults.tchFilter[0] = L'\0';
  Defaults.szCurDir[0] = L'\0';

  // Initialize custom colors for ChooseColor()
  Defaults.crCustom[0] = RGB(0, 0, 128);                   Defaults.crCustom[8] = RGB(255, 255, 226);
  Defaults.crCustom[1] = GetSysColor(COLOR_WINDOWTEXT);    Defaults.crCustom[9] = GetSysColor(COLOR_WINDOW);
  Defaults.crCustom[2] = GetSysColor(COLOR_INFOTEXT);      Defaults.crCustom[10] = GetSysColor(COLOR_INFOBK);
  Defaults.crCustom[3] = GetSysColor(COLOR_HIGHLIGHTTEXT); Defaults.crCustom[11] = GetSysColor(COLOR_HIGHLIGHT);
  Defaults.crCustom[4] = GetSysColor(COLOR_ACTIVECAPTION); Defaults.crCustom[12] = GetSysColor(COLOR_DESKTOP);
  Defaults.crCustom[5] = GetSysColor(COLOR_3DFACE);        Defaults.crCustom[13] = GetSysColor(COLOR_3DFACE);
  Defaults.crCustom[6] = GetSysColor(COLOR_3DFACE);        Defaults.crCustom[14] = GetSysColor(COLOR_3DFACE);
  Defaults.crCustom[7] = GetSysColor(COLOR_3DFACE);        Defaults.crCustom[15] = GetSysColor(COLOR_3DFACE);

  Defaults.wi.x = CW_USEDEFAULT;
  Defaults.wi.y = CW_USEDEFAULT;
  Defaults.wi.cx = CW_USEDEFAULT;
  Defaults.wi.cy = CW_USEDEFAULT;
}



//=============================================================================
//
//  CreateIniFile()
//
//
int CreateIniFile()
{
  int result = 0;
  if (g_wchIniFile[0] != L'\0')
  {
    WCHAR* pwchTail = StrRChrW(g_wchIniFile, NULL, L'\\');

    if (pwchTail) {
      *pwchTail = 0;
      SHCreateDirectoryEx(NULL, g_wchIniFile, NULL);
      *pwchTail = L'\\';
    }

    DWORD dwFileSize = 0UL;

    if (!PathIsExistingFile(g_wchIniFile))
    {
      HANDLE hFile = CreateFile(g_wchIniFile,
        GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
      if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
      }
    }
    else {
      HANDLE hFile = CreateFile(g_wchIniFile,
        GENERIC_READ, FILE_SHARE_READ,
        nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
      if (hFile == INVALID_HANDLE_VALUE) {
        return result;
      }
      DWORD dwFSHigh = 0UL;
      dwFileSize = GetFileSize(hFile, &dwFSHigh);
      CloseHandle(hFile);
    }

    if ((dwFileSize == 0) && (dwFileSize != INVALID_FILE_SIZE)) {
      result = IniFileSetString(g_wchIniFile, L"minipath", NULL, NULL);
    }
    else {
      result = true;
    }

    return result;
  }
  return result;
}
//=============================================================================



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
    if (PathIsExistingFile(tchBuild)) {
      lstrcpy(lpszFile, tchBuild);
      return 1;
    }
    // Sub directory (.\np3\) 
    lstrcpy(tchBuild, lpszModule);
    PathRemoveFileSpec(tchBuild);
    lstrcat(tchBuild, L"\\np3\\");
    lstrcat(tchBuild, tchFileExpanded);
    if (PathIsExistingFile(tchBuild)) {
      lstrcpy(lpszFile, tchBuild);
      return 1;
    }
    // Application Data (%APPDATA%)
    if (S_OK == SHGetFolderPath(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, tchBuild)) {
      PathAppend(tchBuild, tchFileExpanded);
      if (PathIsExistingFile(tchBuild)) {
        lstrcpy(lpszFile, tchBuild);
        return 1;
      }
    }
    // Home (%HOMEPATH%)
    if (S_OK == SHGetFolderPath(nullptr, CSIDL_PROFILE, nullptr, SHGFP_TYPE_CURRENT, tchBuild)) {
      PathAppend(tchBuild, tchFileExpanded);
      if (PathIsExistingFile(tchBuild)) {
        lstrcpy(lpszFile, tchBuild);
        return 1;
      }
    }
  }
  else if (PathIsExistingFile(tchFileExpanded)) {
    lstrcpy(lpszFile, tchFileExpanded);
    return 1;
  }

  return 0;
}

int CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH];

  if (IniFileGetString(lpszFile, lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch)))
  {
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
    if (!PathIsExistingFile(g_wchIniFile)) {
      lstrcpy(PathFindFileName(g_wchIniFile), L"minipath.ini");
      if (!PathIsExistingFile(g_wchIniFile)) {
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
    if (!PathIsExistingFile(g_wchNP3IniFile)) {
      lstrcpy(PathFindFileName(g_wchNP3IniFile), L"notepad3.ini");
      if (!PathIsExistingFile(g_wchNP3IniFile)) {
        lstrcpy(PathFindFileName(g_wchNP3IniFile), PathFindFileName(wchModule));
        PathRenameExtension(g_wchNP3IniFile, L".ini");
      }
    }
  }
  if (!PathIsExistingFile(g_wchNP3IniFile) || PathIsDirectory(g_wchNP3IniFile)) {
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
  __try {

    LoadIniFileCache(g_wchIniFile);

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

  }
  __finally {
    ResetIniFileCache();
  }
}



//=============================================================================
//
//  LoadSettings()
//
//
extern "C" LPWSTR lpFilterArg;

#define GET_BOOL_VALUE_FROM_INISECTION(VARNAME,KEYNAME,DEFAULT) \
  Defaults.VARNAME = DEFAULT;                           \
  Settings.VARNAME = IniSectionGetBool(Settings_Section, KEYNAME, Defaults.VARNAME)

#define GET_INT_VALUE_FROM_INISECTION(VARNAME,KEYNAME,DEFAULT,MIN,MAX) \
  Defaults.VARNAME = DEFAULT;                                  \
  Settings.VARNAME = clampi(IniSectionGetInt(Settings_Section, KEYNAME, Defaults.VARNAME),MIN,MAX)

// -----------------------------------------------------------

void LoadSettings()
{
  __try {

  LoadIniFileCache(g_wchIniFile);

  const WCHAR* const Settings_Section = L"Settings";

  GET_BOOL_VALUE_FROM_INISECTION(bSaveSettings, L"SaveSettings", TRUE);
  GET_BOOL_VALUE_FROM_INISECTION(bSingleClick, L"SingleClick", TRUE);
  GET_BOOL_VALUE_FROM_INISECTION(bTrackSelect, L"TrackSelect", TRUE);
  GET_BOOL_VALUE_FROM_INISECTION(bFullRowSelect, L"FullRowSelect", FALSE);
  GET_BOOL_VALUE_FROM_INISECTION(fUseRecycleBin, L"UseRecycleBin", FALSE);
  GET_BOOL_VALUE_FROM_INISECTION(fNoConfirmDelete, L"NoConfirmDelete", FALSE);
  GET_BOOL_VALUE_FROM_INISECTION(bClearReadOnly, L"ClearReadOnly", TRUE);
  GET_BOOL_VALUE_FROM_INISECTION(bRenameOnCollision, L"RenameOnCollision", FALSE);
  GET_BOOL_VALUE_FROM_INISECTION(bFocusEdit, L"FocusEdit", TRUE);
  GET_BOOL_VALUE_FROM_INISECTION(bAlwaysOnTop, L"AlwaysOnTop", FALSE);
  GET_BOOL_VALUE_FROM_INISECTION(bMinimizeToTray, L"MinimizeToTray", FALSE);
  GET_BOOL_VALUE_FROM_INISECTION(g_bTransparentMode, L"TransparentMode", FALSE);
  GET_INT_VALUE_FROM_INISECTION(iEscFunction, L"EscFunction", 2, 0, 2);
  GET_INT_VALUE_FROM_INISECTION(iStartupDir, L"StartupDirectory", 2, 0, 2);

  Defaults.g_tchFavoritesDir[0] = L'\0';
  if (!IniSectionGetString(Settings_Section, L"Favorites", Defaults.g_tchFavoritesDir,
    Settings.g_tchFavoritesDir, COUNTOF(Settings.g_tchFavoritesDir))) {
    // try to fetch Favorites dir from Notepad3.ini
    if (StrIsNotEmpty(g_wchNP3IniFile)) {
      Settings.bNP3sFavoritesSettings = TRUE;
      IniFileGetString(g_wchNP3IniFile, L"Settings", L"Favorites", L"", Settings.g_tchFavoritesDir, COUNTOF(Settings.g_tchFavoritesDir));
    }
  }
  if (StrIsEmpty(Settings.g_tchFavoritesDir))
    SHGetFolderPath(nullptr, CSIDL_PERSONAL, nullptr, SHGFP_TYPE_CURRENT, Settings.g_tchFavoritesDir);
  else
    PathAbsoluteFromApp(Settings.g_tchFavoritesDir, nullptr, COUNTOF(Settings.g_tchFavoritesDir), TRUE);


  Defaults.szQuickview[0] = L'\0';
  if (!IniSectionGetString(Settings_Section, L"Quikview.exe", Defaults.szQuickview,
    Settings.szQuickview, COUNTOF(Settings.szQuickview))) {
    GetSystemDirectory(Settings.szQuickview, COUNTOF(Settings.szQuickview));
    PathAddBackslash(Settings.szQuickview);
    lstrcat(Settings.szQuickview, L"Viewers\\Quikview.exe");
  }
  else
    PathAbsoluteFromApp(Settings.szQuickview, nullptr, COUNTOF(Settings.szQuickview), TRUE);

  Settings.bHasQuickview = PathIsExistingFile(Settings.szQuickview);

  Defaults.szQuickviewParams[0] = L'\0';
  IniSectionGetString(Settings_Section, L"QuikviewParams", Defaults.szQuickviewParams,
    Settings.szQuickviewParams, COUNTOF(Settings.szQuickviewParams));


  lstrcpy(Defaults.tchOpenWithDir, L"%USERPROFILE%\\Desktop");
  if (IniSectionGetString(Settings_Section, L"OpenWithDir", L"",
    Settings.tchOpenWithDir, COUNTOF(Settings.tchOpenWithDir)) == 0) {
    // try to fetch Open With dir from Notepad3.ini
    IniFileGetString(g_wchNP3IniFile, L"Settings", L"OpenWithDir", L"", Settings.tchOpenWithDir, COUNTOF(Settings.tchOpenWithDir));
  }
  if (StrIsEmpty(Settings.tchOpenWithDir))
    SHGetSpecialFolderPath(nullptr, Settings.tchOpenWithDir, CSIDL_DESKTOPDIRECTORY, TRUE);
  else
    PathAbsoluteFromApp(Settings.tchOpenWithDir, nullptr, COUNTOF(Settings.tchOpenWithDir), TRUE);

  GET_INT_VALUE_FROM_INISECTION(dwFillMask, L"FillMask", DL_ALLOBJECTS, DL_FOLDERS, DL_ALLOBJECTS);
  GET_INT_VALUE_FROM_INISECTION(nSortFlags, L"SortOptions", DS_TYPE, 0, 3);
  GET_BOOL_VALUE_FROM_INISECTION(fSortRev, L"SortReverse", FALSE);

  lstrcpy(Defaults.tchFilter, L"*.*");
  if (!lpFilterArg) {
    IniSectionGetString(Settings_Section, L"FileFilter", Defaults.tchFilter, Settings.tchFilter, COUNTOF(Settings.tchFilter));
  }
  else { // ignore filter if /m was specified
    if (*(lpFilterArg) == L'-') {
      Settings.bNegFilter = TRUE;
      (void)lstrcpyn(Settings.tchFilter, lpFilterArg + 1, COUNTOF(Settings.tchFilter));
    }
    else {
      Settings.bNegFilter = FALSE;
      (void)lstrcpyn(Settings.tchFilter, lpFilterArg, COUNTOF(Settings.tchFilter));
    }
  }
  GET_BOOL_VALUE_FROM_INISECTION(bNegFilter, L"NegativeFilter", FALSE);
  GET_BOOL_VALUE_FROM_INISECTION(bDefCrNoFilt, L"DefColorNoFilter", TRUE);
  GET_BOOL_VALUE_FROM_INISECTION(bDefCrFilter, L"DefColorFilter", TRUE);
  GET_INT_VALUE_FROM_INISECTION(crNoFilt, L"ColorNoFilter", GetSysColor(COLOR_WINDOWTEXT), 0, INT_MAX);
  GET_INT_VALUE_FROM_INISECTION(crFilter, L"ColorFilter", GetSysColor(COLOR_HIGHLIGHT), 0, INT_MAX);

  lstrcpy(Defaults.tchToolbarButtons, L"1 2 3 4 5 0 8");
  if (IniSectionGetString(Settings_Section, L"ToolbarButtons", Defaults.tchToolbarButtons, Settings.tchToolbarButtons, COUNTOF(Settings.tchToolbarButtons)) == 0) {
    lstrcpy(Settings.tchToolbarButtons, Defaults.tchToolbarButtons);
  }

  GET_BOOL_VALUE_FROM_INISECTION(bShowToolbar, L"ShowToolbar", TRUE);
  GET_BOOL_VALUE_FROM_INISECTION(bShowStatusbar, L"ShowStatusbar", TRUE);
  GET_BOOL_VALUE_FROM_INISECTION(bShowDriveBox, L"ShowDriveBox", TRUE);
  GET_INT_VALUE_FROM_INISECTION(cxGotoDlg, L"GotoDlgSizeX", 0, 0, INT_MAX);
  GET_INT_VALUE_FROM_INISECTION(cxOpenWithDlg, L"OpenWithDlgSizeX", 0, 0, INT_MAX);
  GET_INT_VALUE_FROM_INISECTION(cyOpenWithDlg, L"OpenWithDlgSizeY", 0, 0, INT_MAX);
  GET_INT_VALUE_FROM_INISECTION(cxCopyMoveDlg, L"CopyMoveDlgSizeX", 0, 0, INT_MAX);

  // --------------------------------------------------------------------------

  int ResX = GetSystemMetrics(SM_CXSCREEN);
  int ResY = GetSystemMetrics(SM_CYSCREEN);
  
  const WCHAR* const ToolbarImages_Section = L"Toolbar Images";

  IniSectionGetString(ToolbarImages_Section, L"BitmapDefault", L"", Settings.tchToolbarBitmap, COUNTOF(Settings.tchToolbarBitmap));
  IniSectionGetString(ToolbarImages_Section, L"BitmapHot", L"", Settings.tchToolbarBitmapHot, COUNTOF(Settings.tchToolbarBitmap));
  IniSectionGetString(ToolbarImages_Section, L"BitmapDisabled", L"", Settings.tchToolbarBitmapDisabled, COUNTOF(Settings.tchToolbarBitmap));

  if (!flagPosParam) { // ignore window position if /p was specified

    WCHAR tchPosX[32], tchPosY[32], tchSizeX[32], tchSizeY[32];

    wsprintf(tchPosX, L"%ix%i PosX", ResX, ResY);
    wsprintf(tchPosY, L"%ix%i PosY", ResX, ResY);
    wsprintf(tchSizeX, L"%ix%i SizeX", ResX, ResY);
    wsprintf(tchSizeY, L"%ix%i SizeY", ResX, ResY);

    const WCHAR* const Window_Section = L"Window";

    Settings.wi.x = IniSectionGetInt(Window_Section, tchPosX, Defaults.wi.x);
    Settings.wi.y = IniSectionGetInt(Window_Section, tchPosY, Defaults.wi.y);
    Settings.wi.cx = IniSectionGetInt(Window_Section, tchSizeX, Defaults.wi.cx);
    Settings.wi.cy = IniSectionGetInt(Window_Section, tchSizeY, Defaults.wi.cy);
  }

  // Initialize custom colors for ChooseColor()
  for (int i = 0; i < COUNTOF(Settings.crCustom); ++i) {
    Settings.crCustom[i] = Defaults.crCustom[i];
  }
  
  }
  __finally {
    ResetIniFileCache();
  }
}


//=============================================================================
//
//  SaveSettings()
//
//

#define SAVE_VALUE_IF_NOT_EQ_DEFAULT(TYPE,KEYNAME,VARNAME)             \
  if (Settings.VARNAME != Defaults.VARNAME) {                          \
    IniSectionSet##TYPE(Settings_Section, KEYNAME, Settings.VARNAME);  \
  }                                                                    \
  else {                                                               \
    IniSectionDelete(Settings_Section, KEYNAME, false);                \
  }

#define SAVE_STRING_IF_NOT_EQ_DEFAULT(KEYNAME,VARNAME)                 \
  if (lstrcmp(Settings.VARNAME, Defaults.VARNAME) != 0) {              \
    IniSectionSetString(Settings_Section, KEYNAME, Settings.VARNAME);  \
  }                                                                    \
  else {                                                               \
    IniSectionDelete(Settings_Section, KEYNAME, false);                \
  }

// ----------------------------------------------------------------------------

void SaveSettings(BOOL bSaveSettingsNow)
{
  WCHAR wchTmp[MAX_PATH];

  if (StrIsEmpty(g_wchIniFile)) { return; }

  CreateIniFile();

  if (!Settings.bSaveSettings && !bSaveSettingsNow) {
    if (Settings.iStartupDir == 1) {
      IniFileSetString(g_wchIniFile, L"Settings", L"MRUDirectory", Settings.szCurDir);
    }
    IniFileSetBool(g_wchIniFile, L"Settings", L"SaveSettings", Settings.bSaveSettings);
    return;
  }

  __try {

    LoadIniFileCache(g_wchIniFile);

    const WCHAR* const Settings_Section = L"Settings";

    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, L"SaveSettings", bSaveSettings);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, L"SingleClick", bSingleClick);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, L"TrackSelect", bTrackSelect);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, L"FullRowSelect", bFullRowSelect);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, L"UseRecycleBin", fUseRecycleBin);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, L"NoConfirmDelete", fNoConfirmDelete);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, L"ClearReadOnly", bClearReadOnly);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, L"RenameOnCollision", bRenameOnCollision);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, L"FocusEdit", bFocusEdit);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, L"AlwaysOnTop", bAlwaysOnTop);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, L"MinimizeToTray", bMinimizeToTray);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, L"TransparentMode", g_bTransparentMode);

    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, L"EscFunction", iEscFunction);

    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, L"StartupDirectory", iStartupDir);
    if (Settings.iStartupDir == 1) {
      IniSectionSetString(Settings_Section, L"MRUDirectory", Settings.szCurDir);
    }
    if (!Settings.bNP3sFavoritesSettings) {
      PathRelativeToApp(Settings.g_tchFavoritesDir, wchTmp, COUNTOF(wchTmp), FALSE, TRUE, flagPortableMyDocs);
      IniSectionSetString(Settings_Section, L"Favorites", wchTmp);
    }

    PathRelativeToApp(Settings.szQuickview, wchTmp, COUNTOF(wchTmp), FALSE, TRUE, flagPortableMyDocs);
    IniSectionSetString(Settings_Section, L"Quikview.exe", wchTmp);
    SAVE_STRING_IF_NOT_EQ_DEFAULT(L"Quikview.exe", szQuickviewParams);

    PathRelativeToApp(Settings.tchOpenWithDir, wchTmp, COUNTOF(wchTmp), FALSE, TRUE, flagPortableMyDocs);
    if (lstrcmp(wchTmp, Defaults.tchOpenWithDir) != 0) {
      IniSectionSetString(Settings_Section, L"OpenWithDir", wchTmp);
    }

    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, L"FillMask", dwFillMask);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, L"SortOptions", nSortFlags);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, L"SortReverse", fSortRev);
    SAVE_STRING_IF_NOT_EQ_DEFAULT(L"FileFilter", tchFilter);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, L"NegativeFilter", bNegFilter);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, L"DefColorNoFilter", bDefCrNoFilt);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, L"DefColorFilter", bDefCrFilter);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, L"ColorNoFilter", crNoFilt);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, L"ColorFilter", crFilter);
    SAVE_STRING_IF_NOT_EQ_DEFAULT(L"ToolbarButtons", tchToolbarButtons);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, L"ShowToolbar", bShowToolbar);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, L"ShowStatusbar", bShowStatusbar);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, L"ShowDriveBox", bShowDriveBox);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, L"GotoDlgSizeX", cxGotoDlg);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, L"OpenWithDlgSizeX", cxOpenWithDlg);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, L"OpenWithDlgSizeY", cyOpenWithDlg);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, L"CopyMoveDlgSizeX", cxCopyMoveDlg);

    // cleanup
    IniSectionDelete(Settings_Section, L"WriteTest", FALSE);

    /*
      SaveSettingsNow(): query Window Dimensions
    */

    if (bSaveSettingsNow)
    {
      WINDOWPLACEMENT wndpl;
      ZeroMemory(&wndpl, sizeof(WINDOWPLACEMENT));
      // GetWindowPlacement
      wndpl.length = sizeof(WINDOWPLACEMENT);
      GetWindowPlacement(hwndMain, &wndpl);

      Settings.wi.x = wndpl.rcNormalPosition.left;
      Settings.wi.y = wndpl.rcNormalPosition.top;
      Settings.wi.cx = wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left;
      Settings.wi.cy = wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top;
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

    IniSectionSetInt(Windows_Section, tchPosX, Settings.wi.x);
    IniSectionSetInt(Windows_Section, tchPosY, Settings.wi.y);
    IniSectionSetInt(Windows_Section, tchSizeX, Settings.wi.cx);
    IniSectionSetInt(Windows_Section, tchSizeY, Settings.wi.cy);

  }
  __finally {
    SaveIniFileCache(g_wchIniFile);
  }
}

//=============================================================================
