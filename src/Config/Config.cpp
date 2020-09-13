// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Config.cpp                                                                  *
*   Methods to read and write configuration                                   *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2020   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#if !defined(WINVER)
#define WINVER 0x601  /*_WIN32_WINNT_WIN7*/
#endif
#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x601  /*_WIN32_WINNT_WIN7*/
#endif
#if !defined(NTDDI_VERSION)
#define NTDDI_VERSION 0x06010000  /*NTDDI_WIN7*/
#endif
#define VC_EXTRALEAN 1
#define WIN32_LEAN_AND_MEAN 1
#define NOMINMAX 1
#include <windows.h>

#include <strsafe.h>
#include <shlobj.h>
#include <shobjidl.h>

// ----------------------------------------------------------------------------

// Scintilla
#include "ILoader.h"

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "MuiLanguage.h"
#include "resource.h"
}

extern "C" WININFO   g_IniWinInfo;
extern "C" WININFO   g_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C"           THEMEFILES Theme_Files[];

// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bWriteSIG = true;     // IniFileSetXXX()
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = true; // find/replace with line breaks
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static WCHAR* const _s_RecentFiles = L"Recent Files";
static WCHAR* const _s_RecentFind = L"Recent Find";
static WCHAR* const _s_RecentReplace = L"Recent Replace";

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

constexpr bool SI_Success(const SI_Error rc) noexcept { 
  return ((rc == SI_Error::SI_OK) || (rc == SI_Error::SI_UPDATED) || (rc == SI_Error::SI_INSERTED)); 
};

// ============================================================================


bool CanAccessPath(LPCWSTR lpIniFilePath, DWORD genericAccessRights)
{
  bool bRet = false;
  if (StrIsEmpty(lpIniFilePath)) {
    return false;
  }
  DWORD                      length  = 0;
  SECURITY_INFORMATION const secInfo = OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION;

  // check for read-only file attribute
  if (genericAccessRights & GENERIC_WRITE) 
  {
    if (IsReadOnly(GetFileAttributes(lpIniFilePath))) {
      return false;
    }
  }

  // check security tokens
  if (!::GetFileSecurity(lpIniFilePath, secInfo, NULL, 0, &length) && (ERROR_INSUFFICIENT_BUFFER == GetLastError())) {
    PSECURITY_DESCRIPTOR security = static_cast<PSECURITY_DESCRIPTOR>(AllocMem(length, HEAP_ZERO_MEMORY));
    if (security && ::GetFileSecurity(lpIniFilePath, secInfo, security, length, &length)) {
      HANDLE hToken = NULL;
      if (::OpenProcessToken(::GetCurrentProcess(), TOKEN_IMPERSONATE | TOKEN_QUERY | TOKEN_DUPLICATE | STANDARD_RIGHTS_READ, &hToken)) {
        HANDLE hImpersonatedToken = NULL;
        if (::DuplicateToken(hToken, SecurityImpersonation, &hImpersonatedToken)) {
          GENERIC_MAPPING mapping       = {0xFFFFFFFF};
          PRIVILEGE_SET   privileges    = {0};
          DWORD           grantedAccess = 0, privilegesLength = sizeof(privileges);
          BOOL            result = FALSE;

          mapping.GenericRead    = FILE_GENERIC_READ;
          mapping.GenericWrite   = FILE_GENERIC_WRITE;
          mapping.GenericExecute = FILE_GENERIC_EXECUTE;
          mapping.GenericAll     = FILE_ALL_ACCESS;

          ::MapGenericMask(&genericAccessRights, &mapping);
          if (::AccessCheck(security, hImpersonatedToken, genericAccessRights,
                            &mapping, &privileges, &privilegesLength, &grantedAccess, &result)) {
            bRet = (result == TRUE);
          }
          ::CloseHandle(hImpersonatedToken);
        }
        ::CloseHandle(hToken);
      }
      FreeMem(security);
    }
  }
  return bRet;
}


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
      wchar_t msg[MAX_PATH + 128] = { 0 };
      StringCchPrintf(msg, ARRAYSIZE(msg),
        L"AcquireWriteFileLock(%s): NO EXCLUSIVE LOCK ACQUIRED!", lpIniFilePath);
      MsgBoxLastError(msg, 0);
    }
  }
  else {
    wchar_t msg[MAX_PATH + 128] = { 0 };
    StringCchPrintf(msg, ARRAYSIZE(msg),
      L"AcquireWriteFileLock(%s): INVALID FILE HANDLE!", lpIniFilePath);
    MsgBoxLastError(msg, 0);
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

bool ReleaseFileLock(HANDLE hFile, OVERLAPPED& rOvrLpd)
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

static bool s_bIniFileCacheLoaded = false;
static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);

extern "C" bool ResetIniFileCache() {
  s_INI.Reset();
  s_bIniFileCacheLoaded = false;
  return true;
}


extern "C" bool LoadIniFileCache(LPCWSTR lpIniFilePath)
{
  if (StrIsEmpty(lpIniFilePath) || !PathIsExistingFile(lpIniFilePath)) { return false; }

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


extern "C" bool IsIniFileCached() { return s_bIniFileCacheLoaded; }


extern "C" bool SaveIniFileCache(LPCWSTR lpIniFilePath)
{
  if (!s_bIniFileCacheLoaded || StrIsEmpty(lpIniFilePath)) {
    return false;
  }

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
//  OpenSettingsFile()
//
extern "C" bool OpenSettingsFile(bool* keepCached)
{
  if (StrIsNotEmpty(Globals.IniFile)) 
  {
    Globals.bCanSaveIniFile = CreateIniFile(Globals.IniFile, NULL);

    if (!IsIniFileCached()) {
      LoadIniFileCache(Globals.IniFile);
      if (keepCached != NULL) {
        *keepCached = false;
      }
    }
    else if (keepCached != NULL) {
      *keepCached = true;
    }
  }
  else {
    Globals.bCanSaveIniFile = false;
  }
  return IsIniFileCached();
}


//=============================================================================
//
//  CloseSettingsFile()
//
extern "C" bool CloseSettingsFile(bool bSaveChanges, bool keepCached)
{
  if (Globals.bCanSaveIniFile) {
    if (!IsIniFileCached()) {
      return false;
    }
    bool const bSaved = bSaveChanges ? SaveIniFileCache(Globals.IniFile) : false;
    if (!keepCached) {
      ResetIniFileCache();
    }
    return bSaved;
  }
  if (!keepCached) {
    ResetIniFileCache();
  }
  return false;
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
  return StringCchLenW(lpReturnedString, cchReturnedString);
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


extern "C" bool IniSectionGetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  bool bHasMultiple = false;
  bool const bValue = s_INI.GetBoolValue(lpSectionName, lpKeyName, bDefault, &bHasMultiple);
  //assert(!bHasMultiple);
  return bValue;
}
// ============================================================================


extern "C" bool IniSectionSetString(LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  SI_Error const rc = s_INI.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
  return SI_Success(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_Success(rc);
}

extern "C" bool IniSectionSetLong(LPCWSTR lpSectionName, LPCWSTR lpKeyName, long lValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, lValue, nullptr, false, !s_bUseMultiKey);
  return SI_Success(rc);
}

extern "C" bool IniSectionSetLongLong(LPCWSTR lpSectionName, LPCWSTR lpKeyName, long long llValue)
{
  SI_Error const rc = s_INI.SetLongLongValue(lpSectionName, lpKeyName, llValue, nullptr, false, !s_bUseMultiKey);
  return SI_Success(rc);
}

extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_Success(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_Success(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_Success(rc);
}
// ============================================================================


extern "C" bool IniSectionDelete(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  return s_INI.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
}
// ============================================================================


extern "C" bool IniSectionClear(LPCWSTR lpSectionName, bool bRemoveEmpty)
{
  bool const ok = s_INI.Delete(lpSectionName, nullptr, bRemoveEmpty);
  if (!bRemoveEmpty) {
    SI_Error const rc = s_INI.SetValue(lpSectionName, nullptr, nullptr);
    return SI_Success(rc);
  }
  return ok;
}
// ============================================================================

extern "C" bool IniClearAllSections(LPCWSTR lpPrefix, bool bRemoveEmpty)
{
  size_t const len = StringCchLen(lpPrefix, 0);

  CSimpleIni::TNamesDepend Sections;
  s_INI.GetAllSections(Sections);
  for (const auto& section : Sections)
  {
    if (StringCchCompareNI(section.pItem, len, lpPrefix, len) == 0)
    {
      IniSectionClear(section.pItem, bRemoveEmpty);
    }
  }
  return true;
}
// ============================================================================


// ============================================================================
// ============================================================================


extern "C" size_t IniFileGetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpDefault,
  LPWSTR lpReturnedString, size_t cchReturnedString)
{
  if (StrIsEmpty(lpFilePath)) {
    StringCchCopyW(lpReturnedString, cchReturnedString, lpDefault);
    return StringCchLenW(lpReturnedString, cchReturnedString);
  }

  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);

  OVERLAPPED ovrLpd = { 0 };
  HANDLE hFile = AcquireReadFileLock(lpFilePath, ovrLpd);
  if (hFile == INVALID_HANDLE_VALUE) {
    StringCchCopyW(lpReturnedString, cchReturnedString, lpDefault);
    return StringCchLenW(lpReturnedString, cchReturnedString);
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
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
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


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
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


extern "C" bool IniFileSetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
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


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
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


extern "C" bool IniFileIterateSection(LPCWSTR lpFilePath, LPCWSTR lpSectionName, IterSectionFunc_t callBack)
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
//  AddFilePathToRecentDocs()
//
extern "C" void AddFilePathToRecentDocs(LPCWSTR szFilePath)
{
  if (StrIsEmpty(szFilePath)) { return; }

  if (Flags.ShellUseSystemMRU) 
  {
#if TRUE
    SHAddToRecentDocs(SHARD_PATHW, szFilePath);
#else
    (void)CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_SPEED_OVER_MEMORY | COINIT_DISABLE_OLE1DDE);

    IShellItem* pShellItem = NULL;
    HRESULT const hr = SHCreateItemFromParsingName(szFilePath, NULL, IID_PPV_ARGS(&pShellItem));

    if (SUCCEEDED(hr))
    {
      SHARDAPPIDINFO info;
      info.psi = pShellItem;
      info.pszAppID = Settings2.AppUserModelID;  // our AppID - see above
      SHAddToRecentDocs(SHARD_APPIDINFO, &info);
      pShellItem->Release();
    }
    CoUninitialize();
#endif
  }
}


#if 0
//=============================================================================
//
//  ClearDestinationsOnRecentDocs()
//
extern "C" void ClearDestinationsOnRecentDocs()
{
  (void)CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_SPEED_OVER_MEMORY | COINIT_DISABLE_OLE1DDE);

  IApplicationDestinations* pDestinations = NULL;
  HRESULT hr = CoCreateInstance(CLSID_ApplicationDestinations, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDestinations));
    
  if (SUCCEEDED(hr))
  {
    hr = pDestinations->SetAppID(Settings2.AppUserModelID);
    if (SUCCEEDED(hr)) {
      pDestinations->RemoveAllDestinations();
    }
    pDestinations->Release();
  }
  CoUninitialize();
}
#endif


//=============================================================================
//
//  _CheckIniFile()
//
static bool _CheckIniFile(LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
  ExpandEnvironmentStrings(lpszFile, tchFileExpanded, COUNTOF(tchFileExpanded));

  if (PathIsRelative(tchFileExpanded))
  {
    WCHAR tchBuild[MAX_PATH] = { L'\0' };
    // program directory
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    StringCchCopy(PathFindFileName(tchBuild), COUNTOF(tchBuild), tchFileExpanded);
    if (PathIsExistingFile(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathIsExistingFile(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathIsExistingFile(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathIsExistingFile(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    //~// in general search path
    //~if (SearchPath(NULL,tchFileExpanded,L".ini",COUNTOF(tchBuild),tchBuild,NULL)) {
    //~  StringCchCopy(lpszFile,MAX_PATH,tchBuild);
    //~  return true;
    //~}
  }
  else if (PathIsExistingFile(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool _HandleIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR wchPath[MAX_PATH] = { L'\0' };
  if (PathIsExistingFile(lpszFile) && IniFileGetString(lpszFile, lpszAppName, lpszKeyName, L"", wchPath, COUNTOF(wchPath)))
  {
    if (!_CheckIniFile(wchPath, lpszModule)) {
      PathCanonicalizeEx(wchPath, COUNTOF(wchPath));
    }
    StringCchCopy(lpszFile, MAX_PATH, wchPath);
    return true;  // try to use redirection path
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;

  WCHAR tchModule[MAX_PATH] = { L'\0' };
  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));
  PathCanonicalizeEx(tchModule, COUNTOF(tchModule));

  // set env path to module dir
  WCHAR wchIniFilePath[MAX_PATH] = { L'\0' };
  StringCchCopy(wchIniFilePath, COUNTOF(wchIniFilePath), tchModule);
  PathCchRemoveFileSpec(wchIniFilePath, COUNTOF(wchIniFilePath));

  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, wchIniFilePath);

  if (StrIsNotEmpty(Globals.IniFile))
  {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
   
    PathCanonicalizeEx(Globals.IniFile, COUNTOF(Globals.IniFile));
    bFound = _CheckIniFile(wchIniFilePath, tchModule);
  }
  else 
  {
    StringCchCopy(wchIniFilePath, COUNTOF(wchIniFilePath), PathFindFileName(tchModule));
    PathCchRenameExtension(wchIniFilePath, COUNTOF(wchIniFilePath), L".ini");
    bFound = _CheckIniFile(wchIniFilePath, tchModule);

    if (!bFound) {
      StringCchCopy(wchIniFilePath, COUNTOF(wchIniFilePath), _W(SAPPNAME) L".ini");
      bFound = _CheckIniFile(wchIniFilePath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_HandleIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", wchIniFilePath, tchModule)) // 1st
      {
        _HandleIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", wchIniFilePath, tchModule);  // 2nd
        bFound = _CheckIniFile(wchIniFilePath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), wchIniFilePath);
    }
    else // force default name
    {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================


extern "C" bool TestIniFile()
{
  LPWSTR const pszIniFilePath = Globals.IniFile;
  size_t const pathBufCount = COUNTOF(Globals.IniFile);

  if (StringCchCompareXI(pszIniFilePath, L"*?") == 0) {
    StringCchCopy(Globals.IniFileDefault, COUNTOF(Globals.IniFileDefault), L"");
    StringCchCopy(pszIniFilePath, pathBufCount, L"");
    return false;
  }

  if (PathIsDirectory(pszIniFilePath) || *CharPrev(pszIniFilePath, StrEnd(pszIniFilePath, pathBufCount)) == L'\\') {
    WCHAR wchModule[MAX_PATH] = { L'\0' };
    GetModuleFileName(NULL, wchModule, COUNTOF(wchModule));
    PathCchAppend(pszIniFilePath, pathBufCount, PathFindFileName(wchModule));
    PathCchRenameExtension(pszIniFilePath, pathBufCount, L".ini");
    if (!PathIsExistingFile(pszIniFilePath)) {
      StringCchCopy(PathFindFileName(pszIniFilePath), pathBufCount, _W(SAPPNAME) L".ini");
      if (!PathIsExistingFile(pszIniFilePath)) {
        StringCchCopy(PathFindFileName(pszIniFilePath), pathBufCount, PathFindFileName(wchModule));
        PathCchRenameExtension(pszIniFilePath, pathBufCount, L".ini");
      }
    }
  }

  NormalizePathEx(pszIniFilePath, pathBufCount, true, false);

  if (!PathFileExists(pszIniFilePath) || PathIsDirectory(pszIniFilePath)) {
    StringCchCopy(Globals.IniFileDefault, COUNTOF(Globals.IniFileDefault), pszIniFilePath);
    StringCchCopy(pszIniFilePath, pathBufCount, L"");
    return false;
  }

  return true;
}
//=============================================================================


extern "C" bool CreateIniFile(LPCWSTR pszIniFilePath, DWORD* pdwFileSize_out)
{
  if (StrIsNotEmpty(pszIniFilePath))
  {
    WCHAR* pwchTail = StrRChrW(pszIniFilePath, NULL, L'\\');

    if (pwchTail) {
      *pwchTail = L'\0';
      SHCreateDirectoryEx(NULL, pszIniFilePath, NULL);
      *pwchTail = L'\\';
    }
    
    DWORD dwFileSize = 0UL;

    if (!PathIsExistingFile(pszIniFilePath))
    {
      HANDLE hFile = CreateFile(pszIniFilePath,
        GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

      if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile); // done
      }
      else {
        wchar_t msg[MAX_PATH + 128] = { 0 };
        StringCchPrintf(msg, ARRAYSIZE(msg),
          L"CreateIniFile(%s): FAILD TO CREATE INITIAL INI FILE!", pszIniFilePath);
        MsgBoxLastError(msg, 0);
      }
    }
    else 
    {
      HANDLE hFile = CreateFile(pszIniFilePath,
        GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

      if (hFile != INVALID_HANDLE_VALUE) {
        DWORD dwFSHigh = 0UL;
        dwFileSize = GetFileSize(hFile, &dwFSHigh);
        CloseHandle(hFile);
      }
      else {
        wchar_t msg[MAX_PATH + 128] = { 0 };
        StringCchPrintf(msg, ARRAYSIZE(msg),
          L"CreateIniFile(%s): FAILED TO READ FILESIZE!", pszIniFilePath);
        MsgBoxLastError(msg, 0);
        dwFileSize = INVALID_FILE_SIZE;
      }
    }
    if (pdwFileSize_out) { *pdwFileSize_out = dwFileSize; }

    return CanAccessPath(Globals.IniFile, GENERIC_WRITE);
  }
  return false;
}
//=============================================================================


//=============================================================================
//
//  LoadSettings()
//
//
void LoadSettings()
{
  CFG_VERSION const _ver = StrIsEmpty(Globals.IniFile) ? CFG_VER_CURRENT : CFG_VER_NONE;

  bool bDirtyFlag = false;  // do we have to save the file on done

  __try {

    bool dummy = false;
    OpenSettingsFile(&dummy);

    // --------------------------------------------------------------------------
    const WCHAR* const IniSecSettings = Constants.Settings_Section;
    const WCHAR* const IniSecSettings2 = Constants.Settings2_Section;
    // --------------------------------------------------------------------------

    // prerequisites
    Globals.iCfgVersionRead = IniSectionGetInt(IniSecSettings, L"SettingsVersion", _ver);

    Defaults.SaveSettings = StrIsNotEmpty(Globals.IniFile);
    Settings.SaveSettings = Defaults.SaveSettings && IniSectionGetBool(IniSecSettings, L"SaveSettings", Defaults.SaveSettings);

    // ---  first set "hard coded" .ini-Settings  ---

    Flags.bDevDebugMode = IniSectionGetBool(IniSecSettings2, L"DevDebugMode", DefaultFlags.bDevDebugMode);
    Flags.bStickyWindowPosition = IniSectionGetBool(IniSecSettings2, L"StickyWindowPosition", DefaultFlags.bStickyWindowPosition);

    if (Globals.CmdLnFlag_ReuseWindow == 0) {
      Flags.bReuseWindow = IniSectionGetBool(IniSecSettings2, L"ReuseWindow", DefaultFlags.bReuseWindow);
    }
    else {
      Flags.bReuseWindow = (Globals.CmdLnFlag_ReuseWindow == 2);
    }

    if (Globals.CmdLnFlag_SingleFileInstance == 0) {
      Flags.bSingleFileInstance = IniSectionGetBool(IniSecSettings2, L"SingleFileInstance", DefaultFlags.bSingleFileInstance);
    }
    else {
      Flags.bSingleFileInstance = (Globals.CmdLnFlag_SingleFileInstance == 2);
    }

    if (Globals.CmdLnFlag_MultiFileArg == 0) {
      Flags.MultiFileArg = IniSectionGetBool(IniSecSettings2, L"MultiFileArg", DefaultFlags.MultiFileArg);
    }
    else {
      Flags.MultiFileArg = (Globals.CmdLnFlag_MultiFileArg == 2);
    }

    if (Globals.CmdLnFlag_ShellUseSystemMRU == 0) {
      Flags.ShellUseSystemMRU = IniSectionGetBool(IniSecSettings2, L"ShellUseSystemMRU", DefaultFlags.ShellUseSystemMRU);
    }
    else {
      Flags.ShellUseSystemMRU = (Globals.CmdLnFlag_ShellUseSystemMRU == 2);
    }

    Flags.RelativeFileMRU = IniSectionGetBool(IniSecSettings2, L"RelativeFileMRU", DefaultFlags.RelativeFileMRU);
    Flags.PortableMyDocs = IniSectionGetBool(IniSecSettings2, L"PortableMyDocs", DefaultFlags.PortableMyDocs);
    Flags.NoFadeHidden = IniSectionGetBool(IniSecSettings2, L"NoFadeHidden", DefaultFlags.NoFadeHidden);

    Flags.ToolbarLook = IniSectionGetInt(IniSecSettings2, L"ToolbarLook", DefaultFlags.ToolbarLook);
    Flags.ToolbarLook = clampi(Flags.ToolbarLook, 0, 2);

    Flags.SimpleIndentGuides = IniSectionGetBool(IniSecSettings2, L"SimpleIndentGuides", DefaultFlags.SimpleIndentGuides);
    Flags.NoHTMLGuess = IniSectionGetBool(IniSecSettings2, L"NoHTMLGuess", DefaultFlags.NoHTMLGuess);
    Flags.NoCGIGuess = IniSectionGetBool(IniSecSettings2, L"NoCGIGuess", DefaultFlags.NoCGIGuess);
    Flags.NoFileVariables = IniSectionGetInt(IniSecSettings2, L"NoFileVariables", DefaultFlags.NoFileVariables);

    Flags.PrintFileAndLeave = Globals.CmdLnFlag_PrintFileAndLeave;

    // --------------------------------------------------------------------------

    LANGID lngID = 0;
    Defaults2.PreferredLanguageLocaleName[0] = L'\0';
    GetUserPreferredLanguage(Defaults2.PreferredLanguageLocaleName, COUNTOF(Defaults2.PreferredLanguageLocaleName), &lngID);

    IniSectionGetString(IniSecSettings2, L"PreferredLanguageLocaleName", Defaults2.PreferredLanguageLocaleName,
      Settings2.PreferredLanguageLocaleName, COUNTOF(Settings2.PreferredLanguageLocaleName));

    // --------------------------------------------------------------------------

    StringCchCopyW(Defaults2.DefaultExtension, COUNTOF(Defaults2.DefaultExtension), L"txt");
    IniSectionGetString(IniSecSettings2, L"DefaultExtension", Defaults2.DefaultExtension,
      Settings2.DefaultExtension, COUNTOF(Settings2.DefaultExtension));
    StrTrim(Settings2.DefaultExtension, L" \t.\"");

    Defaults2.DefaultDirectory[0] = L'\0';
    IniSectionGetString(IniSecSettings2, L"DefaultDirectory", Defaults2.DefaultDirectory,
      Settings2.DefaultDirectory, COUNTOF(Settings2.DefaultDirectory));

    Defaults2.FileDlgFilters[0] = L'\0';
    IniSectionGetString(IniSecSettings2, L"FileDlgFilters", Defaults2.FileDlgFilters,
      Settings2.FileDlgFilters, COUNTOF(Settings2.FileDlgFilters) - 2);

    Defaults2.FileCheckInverval = 2000UL;
    Settings2.FileCheckInverval = clampul(IniSectionGetInt(IniSecSettings2, L"FileCheckInverval",
      Defaults2.FileCheckInverval), 250UL, 300000UL);
    FileWatching.FileCheckInverval = Settings2.FileCheckInverval;

    Defaults2.AutoReloadTimeout = 2000UL;
    Settings2.AutoReloadTimeout = clampul(IniSectionGetInt(IniSecSettings2, L"AutoReloadTimeout",
      Defaults2.AutoReloadTimeout), 250UL, 300000UL);
    FileWatching.AutoReloadTimeout = Settings2.AutoReloadTimeout;

    Defaults2.UndoTransactionTimeout = 0UL;
    Settings2.UndoTransactionTimeout = clampul(IniSectionGetInt(IniSecSettings2, L"UndoTransactionTimeout",
      Defaults2.UndoTransactionTimeout), 0UL, 86400000UL);

    // deprecated

    Defaults.RenderingTechnology = IniSectionGetInt(IniSecSettings2, L"SciDirectWriteTech", -111);
    if (Defaults.RenderingTechnology != -111) {
      if (Settings.SaveSettings) {
        // cleanup
        IniSectionDelete(IniSecSettings2, L"SciDirectWriteTech", false); // old deprecated
        bDirtyFlag = true;
      }
      Defaults.RenderingTechnology = clampi(Defaults.RenderingTechnology, SC_TECHNOLOGY_DEFAULT, SC_TECHNOLOGY_DIRECTWRITEDC);
    }
    else {
      Defaults.RenderingTechnology = SC_TECHNOLOGY_DIRECTWRITE; // new default DirectWrite (D2D)
    }

    // Settings2 deprecated
    Defaults.Bidirectional = IniSectionGetInt(IniSecSettings2, L"EnableBidirectionalSupport", -111);
    if ((Defaults.Bidirectional != -111) && Settings.SaveSettings) {
      // cleanup
      IniSectionDelete(IniSecSettings2, L"EnableBidirectionalSupport", false);
      bDirtyFlag = true;
    }
    Defaults.Bidirectional = (clampi(Defaults.Bidirectional, SC_BIDIRECTIONAL_DISABLED, SC_BIDIRECTIONAL_R2L) > 0) ? SC_BIDIRECTIONAL_R2L : SC_BIDIRECTIONAL_DISABLED;

    Defaults2.IMEInteraction = -1;
    Settings2.IMEInteraction = clampi(IniSectionGetInt(IniSecSettings2, L"IMEInteraction", Defaults2.IMEInteraction), -1, SC_IME_INLINE);
    // Korean IME use inline mode by default
    if (Settings2.IMEInteraction == -1) { // auto detection once
      // ScintillaWin::KoreanIME()
      int const codePage = Scintilla_InputCodePage();
      Settings2.IMEInteraction = ((codePage == 949 || codePage == 1361) ? SC_IME_INLINE : SC_IME_WINDOWED);
    }

    Defaults2.SciFontQuality = SC_EFF_QUALITY_LCD_OPTIMIZED;
    Settings2.SciFontQuality = clampi(IniSectionGetInt(IniSecSettings2, L"SciFontQuality", Defaults2.SciFontQuality), SC_EFF_QUALITY_DEFAULT, SC_EFF_QUALITY_LCD_OPTIMIZED);

    Defaults2.UpdateDelayMarkAllOccurrences = 50;
    Settings2.UpdateDelayMarkAllOccurrences = clampi(IniSectionGetInt(IniSecSettings2, L"UpdateDelayMarkAllOccurrences",
      Defaults2.UpdateDelayMarkAllOccurrences), USER_TIMER_MINIMUM, 10000);
    Defaults2.DenyVirtualSpaceAccess = false;
    Settings2.DenyVirtualSpaceAccess = IniSectionGetBool(IniSecSettings2, L"DenyVirtualSpaceAccess", Defaults2.DenyVirtualSpaceAccess);

    Defaults2.UseOldStyleBraceMatching = false;
    Settings2.UseOldStyleBraceMatching = IniSectionGetBool(IniSecSettings2, L"UseOldStyleBraceMatching", Defaults2.UseOldStyleBraceMatching);

    Defaults2.CurrentLineHorizontalSlop = 40;
    Settings2.CurrentLineHorizontalSlop = clampi(IniSectionGetInt(IniSecSettings2, L"CurrentLineHorizontalSlop", Defaults2.CurrentLineHorizontalSlop), 0, 240);

    Defaults2.CurrentLineVerticalSlop = 5;
    Settings2.CurrentLineVerticalSlop = clampi(IniSectionGetInt(IniSecSettings2, L"CurrentLineVerticalSlop", Defaults2.CurrentLineVerticalSlop), 0, 25);

    Defaults2.NoCopyLineOnEmptySelection = false;
    Settings2.NoCopyLineOnEmptySelection = IniSectionGetBool(IniSecSettings2, L"NoCopyLineOnEmptySelection", Defaults2.NoCopyLineOnEmptySelection);

    Defaults2.NoCutLineOnEmptySelection = false;
    Settings2.NoCutLineOnEmptySelection = IniSectionGetBool(IniSecSettings2, L"NoCutLineOnEmptySelection", Defaults2.NoCutLineOnEmptySelection);


    int const iARCLdef = 92;
    Defaults2.AnalyzeReliableConfidenceLevel = (float)iARCLdef / 100.0f;
    int const iARCLset = clampi(IniSectionGetInt(IniSecSettings2, L"AnalyzeReliableConfidenceLevel", iARCLdef), 0, 100);
    Settings2.AnalyzeReliableConfidenceLevel = (float)iARCLset / 100.0f;

    int const iAnsiCPBonusDef = 33;
    Defaults2.LocaleAnsiCodePageAnalysisBonus = (float)iAnsiCPBonusDef / 100.0f;
    int const iAnsiCPBonusSet = clampi(IniSectionGetInt(IniSecSettings2, L"LocaleAnsiCodePageAnalysisBonus", iAnsiCPBonusDef), 0, 100);
    Settings2.LocaleAnsiCodePageAnalysisBonus = (float)iAnsiCPBonusSet / 100.0f;

    /* ~~~
    int const iRCEDCMdef = 85;
    Defaults2.ReliableCEDConfidenceMapping = (float)iRCEDCMdef / 100.0f;
    int const iRCEDCMset = clampi(IniSectionGetInt(Settings2_Section, L"ReliableCEDConfidenceMapping", iRCEDCMdef), 0, 100);
    Settings2.ReliableCEDConfidenceMapping = (float)iRCEDCMset / 100.0f;

    int const iURCEDCMdef = 20;
    Defaults2.UnReliableCEDConfidenceMapping = (float)iURCEDCMdef / 100.0f;
    int const iURCEDCMset = clampi(IniSectionGetInt(Settings2_Section, L"UnReliableCEDConfidenceMapping", iURCEDCMdef), 0, iRCEDCMset);
    Settings2.UnReliableCEDConfidenceMapping = (float)iURCEDCMset / 100.0f;
    ~~~ */

    Defaults2.AdministrationTool[0] = L'\0';
    IniSectionGetString(IniSecSettings2, L"AdministrationTool.exe", Defaults2.AdministrationTool,
      Settings2.AdministrationTool, COUNTOF(Settings2.AdministrationTool));

    Defaults2.FileLoadWarningMB = 64;
    Settings2.FileLoadWarningMB = clampi(IniSectionGetInt(IniSecSettings2, L"FileLoadWarningMB", Defaults2.FileLoadWarningMB), 0, 2048);

    Defaults2.OpacityLevel = 75;
    Settings2.OpacityLevel = clampi(IniSectionGetInt(IniSecSettings2, L"OpacityLevel", Defaults2.OpacityLevel), 10, 100);

    Defaults2.FindReplaceOpacityLevel = 50;
    Settings2.FindReplaceOpacityLevel = clampi(IniSectionGetInt(IniSecSettings2, L"FindReplaceOpacityLevel", Defaults2.FindReplaceOpacityLevel), 10, 100);

    Defaults2.FileBrowserPath[0] = L'\0';
    IniSectionGetString(IniSecSettings2, L"filebrowser.exe", Defaults2.FileBrowserPath, Settings2.FileBrowserPath, COUNTOF(Settings2.FileBrowserPath));

    Defaults2.GrepWinPath[0] = L'\0';
    IniSectionGetString(IniSecSettings2, L"grepWin.exe", Defaults2.GrepWinPath, Settings2.GrepWinPath, COUNTOF(Settings2.GrepWinPath));

    StringCchCopyW(Defaults2.AppUserModelID, COUNTOF(Defaults2.AppUserModelID), _W("Rizonesoft." SAPPNAME));
    if (StrIsEmpty(Settings2.AppUserModelID)) { // set via CmdLine ?
      IniSectionGetString(IniSecSettings2, L"ShellAppUserModelID", Defaults2.AppUserModelID, Settings2.AppUserModelID, COUNTOF(Settings2.AppUserModelID));
    }
    Defaults2.ExtendedWhiteSpaceChars[0] = L'\0';
    IniSectionGetString(IniSecSettings2, L"ExtendedWhiteSpaceChars", Defaults2.ExtendedWhiteSpaceChars,
      Settings2.ExtendedWhiteSpaceChars, COUNTOF(Settings2.ExtendedWhiteSpaceChars));

    Defaults2.AutoCompleteWordCharSet[0] = L'\0';
    IniSectionGetString(IniSecSettings2, L"AutoCompleteWordCharSet", Defaults2.AutoCompleteWordCharSet,
      Settings2.AutoCompleteWordCharSet, COUNTOF(Settings2.AutoCompleteWordCharSet));

    Defaults2.AutoCompleteFillUpChars[0] = L'\0';
    IniSectionGetString(IniSecSettings2, L"AutoCompleteFillUpChars", Defaults2.AutoCompleteFillUpChars,
      Settings2.AutoCompleteFillUpChars, COUNTOF(Settings2.AutoCompleteFillUpChars));

    Defaults2.LineCommentPostfixStrg[0] = L'\0';
    IniSectionGetString(IniSecSettings2, L"LineCommentPostfixStrg", Defaults2.LineCommentPostfixStrg,
      Settings2.LineCommentPostfixStrg, COUNTOF(Settings2.LineCommentPostfixStrg));
    StrTrim(Settings2.LineCommentPostfixStrg, L"\"'");

    Defaults2.DateTimeFormat[0] = L'\0'; // empty to get <locale date-time format>
    IniSectionGetString(IniSecSettings2, L"DateTimeFormat", Defaults2.DateTimeFormat, Settings2.DateTimeFormat, COUNTOF(Settings2.DateTimeFormat));
    StrTrim(Settings2.DateTimeFormat, L"\"'");

    Defaults2.DateTimeLongFormat[0] = L'\0'; // empty to get <locale date-time format>
    IniSectionGetString(IniSecSettings2, L"DateTimeLongFormat", Defaults2.DateTimeLongFormat, Settings2.DateTimeLongFormat, COUNTOF(Settings2.DateTimeLongFormat));
    StrTrim(Settings2.DateTimeLongFormat, L"\"'");

    StringCchCopyW(Defaults2.TimeStampRegEx, COUNTOF(Defaults2.TimeStampRegEx), L"\\$Date:[^\\$]+\\$");
    IniSectionGetString(IniSecSettings2, L"TimeStampRegEx", Defaults2.TimeStampRegEx, Settings2.TimeStampRegEx, COUNTOF(Settings2.TimeStampRegEx));
    StrTrim(Settings2.TimeStampRegEx, L"\"'");

    StringCchCopyW(Defaults2.TimeStampFormat, COUNTOF(Defaults2.TimeStampFormat), L"$Date: %s $");
    IniSectionGetString(IniSecSettings2, L"TimeStampFormat", Defaults2.TimeStampFormat, Settings2.TimeStampFormat, COUNTOF(Settings2.TimeStampFormat));
    StrTrim(Settings2.TimeStampFormat, L"\"'");

    StringCchCopyW(Defaults2.WebTemplate1, COUNTOF(Defaults2.WebTemplate1), L"https://google.com/search?q=%s");
    IniSectionGetString(IniSecSettings2, L"WebTemplate1", Defaults2.WebTemplate1, Settings2.WebTemplate1, COUNTOF(Settings2.WebTemplate1));

    StringCchCopyW(Defaults2.WebTemplate2, COUNTOF(Defaults2.WebTemplate2), L"https://en.wikipedia.org/w/index.php?search=%s");
    IniSectionGetString(IniSecSettings2, L"WebTemplate2", Defaults2.WebTemplate2, Settings2.WebTemplate2, COUNTOF(Settings2.WebTemplate2));

    Defaults2.LexerSQLNumberSignAsComment = true;
    Settings2.LexerSQLNumberSignAsComment = IniSectionGetBool(IniSecSettings2, L"LexerSQLNumberSignAsComment", Defaults2.LexerSQLNumberSignAsComment);

    Defaults2.ExitOnESCSkipLevel = 2;
    Settings2.ExitOnESCSkipLevel = clampi(IniSectionGetInt(IniSecSettings2, L"ExitOnESCSkipLevel", Defaults2.ExitOnESCSkipLevel), 0, 2);

    Defaults2.ZoomTooltipTimeout = 3200;
    Settings2.ZoomTooltipTimeout = clampi(IniSectionGetInt(IniSecSettings2, L"ZoomTooltipTimeout", Defaults2.ZoomTooltipTimeout), 0, 10000);

    Defaults2.LargeIconScalePrecent = 150;
    Settings2.LargeIconScalePrecent = clampi(IniSectionGetInt(IniSecSettings2, L"LargeIconScalePrecent", Defaults2.LargeIconScalePrecent), 100, 1000);

    // --------------------------------------------------------------------------
    // Settings: IniSecSettings
    // --------------------------------------------------------------------------

#define GET_BOOL_VALUE_FROM_INISECTION(VARNAME,DEFAULT) \
  Defaults.VARNAME = DEFAULT;                           \
  Settings.VARNAME = IniSectionGetBool(IniSecSettings,  _W(_STRG(VARNAME)), Defaults.VARNAME)

#define GET_INT_VALUE_FROM_INISECTION(VARNAME,DEFAULT,MIN,MAX) \
  Defaults.VARNAME = DEFAULT;                                  \
  Settings.VARNAME = clampi(IniSectionGetInt(IniSecSettings,  _W(_STRG(VARNAME)), Defaults.VARNAME),MIN,MAX)

#define GET_CAST_INT_VALUE_FROM_INISECTION(CAST,VARNAME,DEFAULT,MIN,MAX) \
  Defaults.VARNAME = static_cast<CAST>(DEFAULT);                         \
  Settings.VARNAME = static_cast<CAST>(clampi(IniSectionGetInt(IniSecSettings,  _W(_STRG(VARNAME)), Defaults.VARNAME),MIN,MAX))

#define GET_ENC_VALUE_FROM_INISECTION(VARNAME,DEFAULT,MIN,MAX) \
  Defaults.VARNAME = (cpi_enc_t)DEFAULT;                       \
  Settings.VARNAME = (cpi_enc_t)clampi(IniSectionGetInt(IniSecSettings,  _W(_STRG(VARNAME)), (int)Defaults.VARNAME),(int)MIN,(int)MAX)

    GET_BOOL_VALUE_FROM_INISECTION(SaveRecentFiles, true);
    GET_BOOL_VALUE_FROM_INISECTION(PreserveCaretPos, false);
    GET_BOOL_VALUE_FROM_INISECTION(SaveFindReplace, false);

    Defaults.EFR_Data.bFindClose = false;
    Settings.EFR_Data.bFindClose = IniSectionGetBool(IniSecSettings, L"CloseFind", Defaults.EFR_Data.bFindClose);
    Defaults.EFR_Data.bReplaceClose = false;
    Settings.EFR_Data.bReplaceClose = IniSectionGetBool(IniSecSettings, L"CloseReplace", Defaults.EFR_Data.bReplaceClose);
    Defaults.EFR_Data.bNoFindWrap = false;
    Settings.EFR_Data.bNoFindWrap = IniSectionGetBool(IniSecSettings, L"NoFindWrap", Defaults.EFR_Data.bNoFindWrap);
    Defaults.EFR_Data.bTransformBS = false;
    Settings.EFR_Data.bTransformBS = IniSectionGetBool(IniSecSettings, L"FindTransformBS", Defaults.EFR_Data.bTransformBS);
    Defaults.EFR_Data.bWildcardSearch = false;
    Settings.EFR_Data.bWildcardSearch = IniSectionGetBool(IniSecSettings, L"WildcardSearch", Defaults.EFR_Data.bWildcardSearch);
    Defaults.EFR_Data.bOverlappingFind = false;
    Settings.EFR_Data.bOverlappingFind = IniSectionGetBool(IniSecSettings, L"OverlappingFind", Defaults.EFR_Data.bOverlappingFind);
    Defaults.EFR_Data.bMarkOccurences = true;
    Settings.EFR_Data.bMarkOccurences = IniSectionGetBool(IniSecSettings, L"FindMarkAllOccurrences", Defaults.EFR_Data.bMarkOccurences);
    Defaults.EFR_Data.bHideNonMatchedLines = false;
    Settings.EFR_Data.bHideNonMatchedLines = IniSectionGetBool(IniSecSettings, L"HideNonMatchedLines", Defaults.EFR_Data.bHideNonMatchedLines);
    Defaults.EFR_Data.fuFlags = 0;
    Settings.EFR_Data.fuFlags = (UINT)IniSectionGetInt(IniSecSettings, L"efrData_fuFlags", (int)Defaults.EFR_Data.fuFlags);

    GetKnownFolderPath(FOLDERID_Desktop, Defaults.OpenWithDir, COUNTOF(Defaults.OpenWithDir));
    if (IniSectionGetString(IniSecSettings, L"OpenWithDir", Defaults.OpenWithDir, Settings.OpenWithDir, COUNTOF(Settings.OpenWithDir))) {
      PathAbsoluteFromApp(Settings.OpenWithDir, NULL, COUNTOF(Settings.OpenWithDir), true);
    }

    GetKnownFolderPath(FOLDERID_Favorites, Defaults.FavoritesDir, COUNTOF(Defaults.FavoritesDir));
    if (IniSectionGetString(IniSecSettings, L"Favorites", Defaults.FavoritesDir, Settings.FavoritesDir, COUNTOF(Settings.FavoritesDir))) {
      PathAbsoluteFromApp(Settings.FavoritesDir, NULL, COUNTOF(Settings.FavoritesDir), true);
    }

    GET_INT_VALUE_FROM_INISECTION(PathNameFormat, 1, 0, 2);
    GET_INT_VALUE_FROM_INISECTION(WordWrapMode, 0, 0, 1);
    GET_INT_VALUE_FROM_INISECTION(WordWrapIndent, 0, 0, 6);

    GET_BOOL_VALUE_FROM_INISECTION(WordWrap, true); Globals.fvCurFile.bWordWrap = Settings.WordWrap;
    GET_BOOL_VALUE_FROM_INISECTION(TabsAsSpaces, false);  Globals.fvCurFile.bTabsAsSpaces = Settings.TabsAsSpaces;
    GET_BOOL_VALUE_FROM_INISECTION(TabIndents, true);  Globals.fvCurFile.bTabIndents = Settings.TabIndents;
    GET_INT_VALUE_FROM_INISECTION(TabWidth, 4, 1, 1024);  Globals.fvCurFile.iTabWidth = Settings.TabWidth;
    GET_INT_VALUE_FROM_INISECTION(IndentWidth, 4, 0, 1024);  Globals.fvCurFile.iIndentWidth = Settings.IndentWidth;
    
    GET_BOOL_VALUE_FROM_INISECTION(MarkLongLines, (Globals.iCfgVersionRead < CFG_VER_0002)); Defaults.MarkLongLines = false; // new default
    GET_INT_VALUE_FROM_INISECTION(LongLineMode, EDGE_LINE, EDGE_LINE, EDGE_MULTILINE);
    GET_INT_VALUE_FROM_INISECTION(LongLinesLimit, 80, 0, LONG_LINES_MARKER_LIMIT);
    Globals.iWrapCol = Settings.LongLinesLimit;

    _itow_s(Settings.LongLinesLimit, Defaults.MultiEdgeLines, COUNTOF(Defaults.MultiEdgeLines), 10);
    IniSectionGetString(IniSecSettings, L"MultiEdgeLines", Defaults.MultiEdgeLines, Settings.MultiEdgeLines, COUNTOF(Settings.MultiEdgeLines));
    size_t const n = NormalizeColumnVector(NULL, Settings.MultiEdgeLines, COUNTOF(Settings.MultiEdgeLines));
    StringCchCopy(Globals.fvCurFile.wchMultiEdgeLines, COUNTOF(Globals.fvCurFile.wchMultiEdgeLines), Settings.MultiEdgeLines);
    if (n > 1) { Settings.LongLineMode = EDGE_MULTILINE; }

    Defaults.WordWrapSymbols = 2;
    int const iWS = IniSectionGetInt(IniSecSettings, L"WordWrapSymbols", Defaults.WordWrapSymbols);
    Settings.WordWrapSymbols = clampi(iWS % 10, 0, 2) + clampi((iWS % 100 - iWS % 10) / 10, 0, 2) * 10;

    GET_BOOL_VALUE_FROM_INISECTION(ShowWordWrapSymbols, true);
    GET_BOOL_VALUE_FROM_INISECTION(MatchBraces, true);
    GET_BOOL_VALUE_FROM_INISECTION(AutoCloseTags, false);
    GET_INT_VALUE_FROM_INISECTION(HighlightCurrentLine, 1, 0, 2);
    GET_BOOL_VALUE_FROM_INISECTION(HyperlinkHotspot, true);
    GET_INT_VALUE_FROM_INISECTION(ColorDefHotspot, 2, 0, 3);
    GET_BOOL_VALUE_FROM_INISECTION(ScrollPastEOF, false);
    GET_BOOL_VALUE_FROM_INISECTION(ShowHypLnkToolTip, true);
    GET_BOOL_VALUE_FROM_INISECTION(HighlightUnicodePoints, true);

    GET_BOOL_VALUE_FROM_INISECTION(AutoIndent, true);
    GET_BOOL_VALUE_FROM_INISECTION(AutoCompleteWords, false);
    GET_BOOL_VALUE_FROM_INISECTION(AutoCLexerKeyWords, false);
    GET_BOOL_VALUE_FROM_INISECTION(AccelWordNavigation, false);
    GET_BOOL_VALUE_FROM_INISECTION(EditLineCommentBlock, false);
    GET_BOOL_VALUE_FROM_INISECTION(ShowIndentGuides, false);
    GET_BOOL_VALUE_FROM_INISECTION(BackspaceUnindents, false);
    GET_BOOL_VALUE_FROM_INISECTION(WarnInconsistentIndents, false);
    GET_BOOL_VALUE_FROM_INISECTION(AutoDetectIndentSettings, false);

    GET_BOOL_VALUE_FROM_INISECTION(ShowBookmarkMargin, IniSectionGetBool(IniSecSettings, L"ShowSelectionMargin", true));
    GET_BOOL_VALUE_FROM_INISECTION(ShowLineNumbers, true);
    GET_BOOL_VALUE_FROM_INISECTION(ShowCodeFolding, true); FocusedView.ShowCodeFolding = Settings.ShowCodeFolding;

    GET_BOOL_VALUE_FROM_INISECTION(MarkOccurrences, true);
    GET_BOOL_VALUE_FROM_INISECTION(MarkOccurrencesBookmark, false);
    GET_BOOL_VALUE_FROM_INISECTION(MarkOccurrencesMatchVisible, false);
    GET_BOOL_VALUE_FROM_INISECTION(MarkOccurrencesMatchCase, false);
    GET_BOOL_VALUE_FROM_INISECTION(MarkOccurrencesMatchWholeWords, true);

    Defaults.MarkOccurrencesCurrentWord = !Defaults.MarkOccurrencesMatchWholeWords;
    Settings.MarkOccurrencesCurrentWord = IniSectionGetBool(IniSecSettings, L"MarkOccurrencesCurrentWord", Defaults.MarkOccurrencesCurrentWord);
    Settings.MarkOccurrencesCurrentWord = Settings.MarkOccurrencesCurrentWord && !Settings.MarkOccurrencesMatchWholeWords;

    GET_BOOL_VALUE_FROM_INISECTION(ViewWhiteSpace, false);
    GET_BOOL_VALUE_FROM_INISECTION(ViewEOLs, false);

    auto const iDefaultEncoding = (cpi_enc_t)Encoding_MapIniSetting(false, (int)CPI_PREFERRED_ENCODING);
    GET_ENC_VALUE_FROM_INISECTION(DefaultEncoding, iDefaultEncoding, CPI_NONE, INT_MAX);
    Settings.DefaultEncoding = ((Settings.DefaultEncoding == CPI_NONE) ? CPI_PREFERRED_ENCODING : 
                                                                         (cpi_enc_t)Encoding_MapIniSetting(true, (int)Settings.DefaultEncoding));
    Globals.fvCurFile.iEncoding = Settings.DefaultEncoding;

    GET_BOOL_VALUE_FROM_INISECTION(UseDefaultForFileEncoding, false);
    GET_BOOL_VALUE_FROM_INISECTION(LoadASCIIasUTF8, true);
    GET_BOOL_VALUE_FROM_INISECTION(UseReliableCEDonly, true);
    GET_BOOL_VALUE_FROM_INISECTION(LoadNFOasOEM, true);
    GET_BOOL_VALUE_FROM_INISECTION(NoEncodingTags, true);
    GET_BOOL_VALUE_FROM_INISECTION(SkipUnicodeDetection, false);
    GET_BOOL_VALUE_FROM_INISECTION(SkipANSICodePageDetection, false);
    GET_INT_VALUE_FROM_INISECTION(DefaultEOLMode, SC_EOL_CRLF, SC_EOL_CRLF, SC_EOL_LF);
    GET_BOOL_VALUE_FROM_INISECTION(WarnInconsistEOLs, true);
    GET_BOOL_VALUE_FROM_INISECTION(FixLineEndings, false);
    GET_BOOL_VALUE_FROM_INISECTION(FixTrailingBlanks, false);
    GET_INT_VALUE_FROM_INISECTION(PrintHeader, 1, 0, 3);
    GET_INT_VALUE_FROM_INISECTION(PrintFooter, 0, 0, 1);
    GET_INT_VALUE_FROM_INISECTION(PrintColorMode, 3, 0, 4);

    //int const zoomScale = 100;
    int const baseZoom = 100;
    int const prtFontSize = 10;
    int const zoomScale = MulDiv(baseZoom, prtFontSize, Globals.InitialFontSize);
    Defaults.PrintZoom = (Globals.iCfgVersionRead < CFG_VER_0001) ? (zoomScale / 10) : zoomScale;
    int iPrintZoom = clampi(IniSectionGetInt(IniSecSettings, L"PrintZoom", Defaults.PrintZoom), 0, SC_MAX_ZOOM_LEVEL);
    if (Globals.iCfgVersionRead < CFG_VER_0001) { iPrintZoom = 100 + (iPrintZoom - 10) * 10; }
    Settings.PrintZoom = clampi(iPrintZoom, SC_MIN_ZOOM_LEVEL, SC_MAX_ZOOM_LEVEL);

    WCHAR localeInfo[3];
    GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_IMEASURE, localeInfo, 3);
    LONG const _margin = (localeInfo[0] == L'0') ? 2000L : 1000L; // Metric system. L'1' is US System
    Defaults.PrintMargin.left = _margin;
    Settings.PrintMargin.left = clampi(IniSectionGetInt(IniSecSettings, L"PrintMarginLeft", Defaults.PrintMargin.left), 0, 40000);
    Defaults.PrintMargin.top = _margin;
    Settings.PrintMargin.top = clampi(IniSectionGetInt(IniSecSettings, L"PrintMarginTop", Defaults.PrintMargin.top), 0, 40000);
    Defaults.PrintMargin.right = _margin;
    Settings.PrintMargin.right = clampi(IniSectionGetInt(IniSecSettings, L"PrintMarginRight", Defaults.PrintMargin.right), 0, 40000);
    Defaults.PrintMargin.bottom = _margin;
    Settings.PrintMargin.bottom = clampi(IniSectionGetInt(IniSecSettings, L"PrintMarginBottom", Defaults.PrintMargin.bottom), 0, 40000);

    GET_BOOL_VALUE_FROM_INISECTION(SaveBeforeRunningTools, false);
    GET_BOOL_VALUE_FROM_INISECTION(EvalTinyExprOnSelection, true);
    GET_CAST_INT_VALUE_FROM_INISECTION(FILE_WATCHING_MODE, FileWatchingMode, FWM_DONT_CARE, FWM_DONT_CARE, FWM_AUTORELOAD);
    FileWatching.FileWatchingMode = Settings.FileWatchingMode;
    GET_BOOL_VALUE_FROM_INISECTION(ResetFileWatching, true);   FileWatching.ResetFileWatching = Settings.ResetFileWatching;
    GET_INT_VALUE_FROM_INISECTION(EscFunction, 0, 0, 2);
    GET_BOOL_VALUE_FROM_INISECTION(AlwaysOnTop, false);
    GET_BOOL_VALUE_FROM_INISECTION(MinimizeToTray, false);
    GET_BOOL_VALUE_FROM_INISECTION(TransparentMode, false);
    GET_BOOL_VALUE_FROM_INISECTION(FindReplaceTransparentMode, true);
    GET_INT_VALUE_FROM_INISECTION(RenderingTechnology, Defaults.RenderingTechnology, SC_TECHNOLOGY_DEFAULT, SC_TECHNOLOGY_DIRECTWRITEDC); // default set before
    Defaults.RenderingTechnology = SC_TECHNOLOGY_DIRECTWRITE;  // DirectWrite (D2D) - reset, if set by deprecated SciDirectWriteTech
    GET_INT_VALUE_FROM_INISECTION(Bidirectional, Defaults.Bidirectional, SC_BIDIRECTIONAL_DISABLED, SC_BIDIRECTIONAL_R2L);                // set before
    Defaults.Bidirectional = SC_BIDIRECTIONAL_DISABLED; // reset
    GET_BOOL_VALUE_FROM_INISECTION(MuteMessageBeep, false);
    GET_BOOL_VALUE_FROM_INISECTION(SplitUndoTypingSeqOnLnBreak, true);
    GET_BOOL_VALUE_FROM_INISECTION(EditLayoutRTL, false);
    GET_BOOL_VALUE_FROM_INISECTION(DialogsLayoutRTL, false);

    ///~Settings2.IMEInteraction = clampi(IniSectionGetInt(IniSecSettings, L"IMEInteraction", Settings2.IMEInteraction), SC_IME_WINDOWED, SC_IME_INLINE);

    // see TBBUTTON  s_tbbMainWnd[] for initial/reset set of buttons
    StringCchCopyW(Defaults.ToolbarButtons, COUNTOF(Defaults.ToolbarButtons), (Globals.iCfgVersionRead < CFG_VER_0002) ? TBBUTTON_DEFAULT_IDS_V1 : TBBUTTON_DEFAULT_IDS_V2);
    IniSectionGetString(IniSecSettings, L"ToolbarButtons", Defaults.ToolbarButtons, Settings.ToolbarButtons, COUNTOF(Settings.ToolbarButtons));

    GET_BOOL_VALUE_FROM_INISECTION(ShowMenubar, true);
    GET_BOOL_VALUE_FROM_INISECTION(ShowToolbar, true);
    GET_BOOL_VALUE_FROM_INISECTION(ShowStatusbar, true);

    GET_INT_VALUE_FROM_INISECTION(EncodingDlgSizeX, 340, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(EncodingDlgSizeY, 292, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(RecodeDlgSizeX, 340, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(RecodeDlgSizeY, 292, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(FileMRUDlgSizeX, 487, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(FileMRUDlgSizeY, 339, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(OpenWithDlgSizeX, 305, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(OpenWithDlgSizeY, 281, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(FavoritesDlgSizeX, 305, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(FavoritesDlgSizeY, 281, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(AddToFavDlgSizeX, 317, INT_MIN, INT_MAX);

    GET_INT_VALUE_FROM_INISECTION(FindReplaceDlgPosX, CW_USEDEFAULT, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(FindReplaceDlgPosY, CW_USEDEFAULT, INT_MIN, INT_MAX);

    GET_INT_VALUE_FROM_INISECTION(CustomSchemesDlgPosX, CW_USEDEFAULT, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(CustomSchemesDlgPosY, CW_USEDEFAULT, INT_MIN, INT_MAX);

    GET_INT_VALUE_FROM_INISECTION(FocusViewMarkerMode, FVMM_FOLD, FVMM_MARGIN, (FVMM_LN_BACKGR | FVMM_FOLD));
    Settings.FocusViewMarkerMode = (Settings.FocusViewMarkerMode == (FVMM_MARGIN | FVMM_LN_BACKGR) ? FVMM_FOLD : Settings.FocusViewMarkerMode);

    // --------------------------------------------------------------------------
    const WCHAR* const StatusBar_Section = L"Statusbar Settings";
    // --------------------------------------------------------------------------

    WCHAR tchStatusBar[MIDSZ_BUFFER] = { L'\0' };
    IniSectionGetString(StatusBar_Section, L"VisibleSections", STATUSBAR_DEFAULT_IDS, tchStatusBar, COUNTOF(tchStatusBar));
    ReadVectorFromString(tchStatusBar, s_iStatusbarSections, STATUS_SECTOR_COUNT, 0, (STATUS_SECTOR_COUNT - 1), -1, false);

    // cppcheck-suppress useStlAlgorithm
    for (bool& sbv : s_iStatusbarVisible) { sbv = false; }
    int cnt = 0;
    for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
      s_vSBSOrder[i] = -1;
      int const id = s_iStatusbarSections[i];
      if (id >= 0) {
        s_vSBSOrder[cnt++] = id;
        s_iStatusbarVisible[id] = true;
      }
    }

    IniSectionGetString(StatusBar_Section, L"SectionWidthSpecs", STATUSBAR_SECTION_WIDTH_SPECS, tchStatusBar, COUNTOF(tchStatusBar));
    ReadVectorFromString(tchStatusBar, s_iStatusbarWidthSpec, STATUS_SECTOR_COUNT, -4096, 4096, 0, false);

    Globals.bZeroBasedColumnIndex = IniSectionGetBool(StatusBar_Section, L"ZeroBasedColumnIndex", false);
    Globals.bZeroBasedCharacterCount = IniSectionGetBool(StatusBar_Section, L"ZeroBasedCharacterCount", false);


    // --------------------------------------------------------------------------
    const WCHAR* const ToolbarImg_Section = L"Toolbar Images";
    // --------------------------------------------------------------------------

    IniSectionGetString(ToolbarImg_Section, L"BitmapDefault", L"",
      s_tchToolbarBitmap, COUNTOF(s_tchToolbarBitmap));
    IniSectionGetString(ToolbarImg_Section, L"BitmapHot", L"",
      s_tchToolbarBitmapHot, COUNTOF(s_tchToolbarBitmap));
    IniSectionGetString(ToolbarImg_Section, L"BitmapDisabled", L"",
      s_tchToolbarBitmapDisabled, COUNTOF(s_tchToolbarBitmap));


    // --------------------------------------------------------------------------
    const WCHAR* const IniSecWindow = Constants.Window_Section;
    // --------------------------------------------------------------------------

    int const ResX = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int const ResY = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    WCHAR tchHighDpiToolBar[64] = { L'\0' };
    StringCchPrintf(tchHighDpiToolBar, COUNTOF(tchHighDpiToolBar), L"%ix%i HighDpiToolBar", ResX, ResY);

    Defaults.ToolBarTheme = -1;
    Settings.ToolBarTheme = IniSectionGetInt(IniSecWindow, tchHighDpiToolBar, Defaults.ToolBarTheme);
    Settings.ToolBarTheme = clampi(Settings.ToolBarTheme, -1, StrIsEmpty(s_tchToolbarBitmap) ? 1 : 2);

    StringCchPrintf(tchHighDpiToolBar, COUNTOF(tchHighDpiToolBar), L"%ix%i DpiScaleToolBar", ResX, ResY);
    Defaults.DpiScaleToolBar = false;
    Settings.DpiScaleToolBar = IniSectionGetBool(IniSecWindow, tchHighDpiToolBar, Defaults.DpiScaleToolBar);

    // --------------------------------------------------------------
    // startup window  (ignore window position if /p was specified)
    // --------------------------------------------------------------

    Defaults2.DefaultWindowPosition[0] = L'\0';
    IniSectionGetString(IniSecSettings2, L"DefaultWindowPosition", Defaults2.DefaultWindowPosition,
      Settings2.DefaultWindowPosition, COUNTOF(Settings2.DefaultWindowPosition));

    bool const bExplicitDefaultWinPos = StrIsNotEmpty(Settings2.DefaultWindowPosition);

    // 1st set default window position 

    g_DefWinInfo = GetFactoryDefaultWndPos(2); // std. default position

    if (bExplicitDefaultWinPos) {
      int bMaxi = 0;
      int const itok = swscanf_s(Settings2.DefaultWindowPosition, L"%i,%i,%i,%i,%i",
        &g_DefWinInfo.x, &g_DefWinInfo.y, &g_DefWinInfo.cx, &g_DefWinInfo.cy, &bMaxi);
      if (itok == 4 || itok == 5) { // scan successful
        if (itok == 4) {
          g_DefWinInfo.max = false;
        }
        else {
          g_DefWinInfo.max = bMaxi ? true : false;
        }
      }
      else {
        g_DefWinInfo = GetFactoryDefaultWndPos(2);
        // overwrite bad defined default position
        StringCchPrintf(Settings2.DefaultWindowPosition, COUNTOF(Settings2.DefaultWindowPosition),
          L"%i,%i,%i,%i,%i", g_DefWinInfo.x, g_DefWinInfo.y, g_DefWinInfo.cx, g_DefWinInfo.cy, g_DefWinInfo.max);
        IniSectionSetString(IniSecSettings2, L"DefaultWindowPosition", Settings2.DefaultWindowPosition);
        bDirtyFlag = true;
      }
    }

    // 2nd set initial window position

    if (!Globals.CmdLnFlag_PosParam /*|| g_bStickyWinPos*/) {

      WININFO winInfo = g_IniWinInfo;
      WCHAR tchPosX[64], tchPosY[64], tchSizeX[64], tchSizeY[64], tchMaximized[64], tchZoom[64];
      StringCchPrintf(tchPosX, COUNTOF(tchPosX), L"%ix%i PosX", ResX, ResY);
      StringCchPrintf(tchPosY, COUNTOF(tchPosY), L"%ix%i PosY", ResX, ResY);
      StringCchPrintf(tchSizeX, COUNTOF(tchSizeX), L"%ix%i SizeX", ResX, ResY);
      StringCchPrintf(tchSizeY, COUNTOF(tchSizeY), L"%ix%i SizeY", ResX, ResY);
      StringCchPrintf(tchMaximized, COUNTOF(tchMaximized), L"%ix%i Maximized", ResX, ResY);
      StringCchPrintf(tchZoom, COUNTOF(tchZoom), L"%ix%i Zoom", ResX, ResY);

      winInfo.x = IniSectionGetInt(IniSecWindow, tchPosX, g_IniWinInfo.x);
      winInfo.y = IniSectionGetInt(IniSecWindow, tchPosY, g_IniWinInfo.y);
      winInfo.cx = IniSectionGetInt(IniSecWindow, tchSizeX, g_IniWinInfo.cx);
      winInfo.cy = IniSectionGetInt(IniSecWindow, tchSizeY, g_IniWinInfo.cy);
      winInfo.max = IniSectionGetBool(IniSecWindow, tchMaximized, false);
      winInfo.zoom = IniSectionGetInt(IniSecWindow, tchZoom, (Globals.iCfgVersionRead < CFG_VER_0001) ? 0 : 100);
      if (Globals.iCfgVersionRead < CFG_VER_0001) { winInfo.zoom = (winInfo.zoom + 10) * 10; }
      winInfo.zoom = clampi(winInfo.zoom, SC_MIN_ZOOM_LEVEL, SC_MAX_ZOOM_LEVEL);

      if ((winInfo.x == CW_USEDEFAULT) || (winInfo.y == CW_USEDEFAULT) ||
        (winInfo.cx == CW_USEDEFAULT) || (winInfo.cy == CW_USEDEFAULT))
      {
        g_IniWinInfo = g_DefWinInfo;
        Globals.CmdLnFlag_WindowPos = 2; // std. default position (CmdLn: /pd)
      }
      else {
        g_IniWinInfo = winInfo;
        Globals.CmdLnFlag_WindowPos = 0; // init to g_IniWinInfo
      }
    }

    // ------------------------------------------------------------------------

    // ---  override by resolution specific settings  ---
    WCHAR tchSciDirectWriteTech[64];
    StringCchPrintf(tchSciDirectWriteTech, COUNTOF(tchSciDirectWriteTech), L"%ix%i RenderingTechnology", ResX, ResY);
    Settings.RenderingTechnology = clampi(IniSectionGetInt(IniSecWindow, tchSciDirectWriteTech, Settings.RenderingTechnology), 0, 3);

    WCHAR tchSciFontQuality[64];
    StringCchPrintf(tchSciFontQuality, COUNTOF(tchSciFontQuality), L"%ix%i SciFontQuality", ResX, ResY);
    Settings2.SciFontQuality = clampi(IniSectionGetInt(IniSecWindow, tchSciFontQuality, Settings2.SciFontQuality), SC_EFF_QUALITY_DEFAULT, SC_EFF_QUALITY_LCD_OPTIMIZED);

    IniSectionGetString(Constants.Styles_Section, Constants.StylingThemeName, L"", Globals.SelectedThemeName, COUNTOF(Globals.SelectedThemeName));

    // define scintilla internal codepage
    int const iSciDefaultCodePage = SC_CP_UTF8; // default UTF8

    // set flag for encoding default
    Encoding_SetDefaultFlag(Settings.DefaultEncoding);

    // define default charset
    Globals.iDefaultCharSet = SC_CHARSET_DEFAULT;

    // File MRU
    Globals.pFileMRU = MRU_Create(_s_RecentFiles, MRU_NOCASE, MRU_ITEMSFILE);
    MRU_Load(Globals.pFileMRU, true);

    Globals.pMRUfind = MRU_Create(_s_RecentFind, (/*IsWindowsNT()*/true) ? MRU_UTF8 : 0, MRU_ITEMSFNDRPL);
    MRU_Load(Globals.pMRUfind, false);
    if (IsFindPatternEmpty()) {
      SetFindPattern(Globals.pMRUfind->pszItems[0]);
    }

    Globals.pMRUreplace = MRU_Create(_s_RecentReplace, (/*IsWindowsNT()*/true) ? MRU_UTF8 : 0, MRU_ITEMSFNDRPL);
    MRU_Load(Globals.pMRUreplace, false);

  }
  __finally {
    CloseSettingsFile(bDirtyFlag, true);
  }

  // Scintilla Styles
  Style_Load();

  ResetIniFileCache();
}
//=============================================================================



//=============================================================================
//
//  _SaveSettings()
//

#define SAVE_VALUE_IF_NOT_EQ_DEFAULT(TYPE, VARNAME)                               \
  if (Settings.VARNAME != Defaults.VARNAME) {                                     \
    IniSectionSet##TYPE(IniSecSettings, _W(_STRG(VARNAME)), Settings.VARNAME);    \
  }                                                                               \
  else {                                                                          \
    IniSectionDelete(IniSecSettings, _W(_STRG(VARNAME)), false);                  \
  }

#define SAVE_VALUE2_IF_NOT_EQ_DEFAULT2(TYPE, VARNAME)                             \
  if (Settings2.VARNAME != Defaults2.VARNAME) {                                   \
    IniSectionSet##TYPE(IniSecSettings2, _W(_STRG(VARNAME)), Settings.VARNAME);   \
  }                                                                               \
  else {                                                                          \
    IniSectionDelete(IniSecSettings2, _W(_STRG(VARNAME)), false);                 \
  }

static bool _SaveSettings(bool bForceSaveSettings)
{
  if (!IsIniFileCached()) { return false; }

  // --------------------------------------------------------------------------
  const WCHAR* const IniSecSettings = Constants.Settings_Section;
  // --------------------------------------------------------------------------

  if (!(Settings.SaveSettings || bForceSaveSettings))
  {
    if (Settings.SaveSettings != Defaults.SaveSettings) {
      IniSectionSetBool(IniSecSettings, L"SaveSettings", Settings.SaveSettings);
    }
    else {
      IniSectionDelete(IniSecSettings, L"SaveSettings", false);
    }
    return true;
  }

  IniSectionSetInt(IniSecSettings, L"SettingsVersion", CFG_VER_CURRENT);  // new settings

  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, SaveSettings);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, SaveRecentFiles);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, PreserveCaretPos);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, SaveFindReplace);

  if (Settings.EFR_Data.bFindClose != Defaults.EFR_Data.bFindClose) {
    IniSectionSetBool(IniSecSettings, L"CloseFind", Settings.EFR_Data.bFindClose);
  }
  else {
    IniSectionDelete(IniSecSettings, L"CloseFind", false);
  }
  if (Settings.EFR_Data.bReplaceClose != Defaults.EFR_Data.bReplaceClose) {
    IniSectionSetBool(IniSecSettings, L"CloseReplace", Settings.EFR_Data.bReplaceClose);
  }
  else {
    IniSectionDelete(IniSecSettings, L"CloseReplace", false);
  }
  if (Settings.EFR_Data.bNoFindWrap != Defaults.EFR_Data.bNoFindWrap) {
    IniSectionSetBool(IniSecSettings, L"NoFindWrap", Settings.EFR_Data.bNoFindWrap);
  }
  else {
    IniSectionDelete(IniSecSettings, L"NoFindWrap", false);
  }
  if (Settings.EFR_Data.bTransformBS != Defaults.EFR_Data.bTransformBS) {
    IniSectionSetBool(IniSecSettings, L"FindTransformBS", Settings.EFR_Data.bTransformBS);
  }
  else {
    IniSectionDelete(IniSecSettings, L"FindTransformBS", false);
  }
  if (Settings.EFR_Data.bWildcardSearch != Defaults.EFR_Data.bWildcardSearch) {
    IniSectionSetBool(IniSecSettings, L"WildcardSearch", Settings.EFR_Data.bWildcardSearch);
  }
  else {
    IniSectionDelete(IniSecSettings, L"WildcardSearch", false);
  }
  if (Settings.EFR_Data.bOverlappingFind != Defaults.EFR_Data.bOverlappingFind) {
    IniSectionSetBool(IniSecSettings, L"OverlappingFind", Settings.EFR_Data.bOverlappingFind);
  }
  else {
    IniSectionDelete(IniSecSettings, L"OverlappingFind", false);
  }
  if (Settings.EFR_Data.bMarkOccurences != Defaults.EFR_Data.bMarkOccurences) {
    IniSectionSetBool(IniSecSettings, L"FindMarkAllOccurrences", Settings.EFR_Data.bMarkOccurences);
  }
  else {
    IniSectionDelete(IniSecSettings, L"FindMarkAllOccurrences", false);
  }
  if (Settings.EFR_Data.bHideNonMatchedLines != Defaults.EFR_Data.bHideNonMatchedLines) {
    IniSectionSetBool(IniSecSettings, L"HideNonMatchedLines", Settings.EFR_Data.bHideNonMatchedLines);
  }
  else {
    IniSectionDelete(IniSecSettings, L"HideNonMatchedLines", false);
  }
  if (Settings.EFR_Data.fuFlags != Defaults.EFR_Data.fuFlags) {
    IniSectionSetInt(IniSecSettings, L"efrData_fuFlags", Settings.EFR_Data.fuFlags);
  }
  else {
    IniSectionDelete(IniSecSettings, L"efrData_fuFlags", false);
  }

  WCHAR wchTmp[MAX_PATH] = { L'\0' };
  if (StringCchCompareXIW(Settings.OpenWithDir, Defaults.OpenWithDir) != 0) {
    PathRelativeToApp(Settings.OpenWithDir, wchTmp, COUNTOF(wchTmp), false, true, Flags.PortableMyDocs);
    IniSectionSetString(IniSecSettings, L"OpenWithDir", wchTmp);
  }
  else {
    IniSectionDelete(IniSecSettings, L"OpenWithDir", false);
  }
  if (StringCchCompareXIW(Settings.FavoritesDir, Defaults.FavoritesDir) != 0) {
    PathRelativeToApp(Settings.FavoritesDir, wchTmp, COUNTOF(wchTmp), false, true, Flags.PortableMyDocs);
    IniSectionSetString(IniSecSettings, L"Favorites", wchTmp);
  }
  else {
    IniSectionDelete(IniSecSettings, L"Favorites", false);
  }

  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, PathNameFormat);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, WordWrap);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, TabsAsSpaces);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, TabIndents);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, TabWidth);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, IndentWidth);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, BackspaceUnindents);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, WordWrapMode);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, WordWrapIndent);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, WordWrapSymbols);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ShowWordWrapSymbols);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MatchBraces);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, AutoCloseTags);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, HighlightCurrentLine);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, HyperlinkHotspot);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, ColorDefHotspot);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ScrollPastEOF);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ShowHypLnkToolTip);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, HighlightUnicodePoints);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, AutoIndent);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, AutoCompleteWords);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, AutoCLexerKeyWords);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, AccelWordNavigation);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, EditLineCommentBlock);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ShowIndentGuides);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, WarnInconsistentIndents);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, AutoDetectIndentSettings);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MarkLongLines);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int,  LongLineMode);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int,  LongLinesLimit);
  if (StringCchCompareX(Settings.MultiEdgeLines, Defaults.MultiEdgeLines) != 0) {
    IniSectionSetString(IniSecSettings, L"MultiEdgeLines", Settings.MultiEdgeLines);
  }
  else {
    IniSectionDelete(IniSecSettings, L"MultiEdgeLines", false);
  }
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ShowBookmarkMargin);
  IniSectionDelete(IniSecSettings, L"ShowSelectionMargin", false); // old

  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ShowLineNumbers);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ShowCodeFolding);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MarkOccurrences);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MarkOccurrencesBookmark);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MarkOccurrencesMatchVisible);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MarkOccurrencesMatchCase);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MarkOccurrencesMatchWholeWords);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MarkOccurrencesCurrentWord);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ViewWhiteSpace);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ViewEOLs);

  // encoding: internal<->external mapping
  Settings.DefaultEncoding = (cpi_enc_t)Encoding_MapIniSetting(false, (int)Settings.DefaultEncoding);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, DefaultEncoding);
  Settings.DefaultEncoding = (cpi_enc_t)Encoding_MapIniSetting(true, (int)Settings.DefaultEncoding);

  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, UseDefaultForFileEncoding);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, LoadASCIIasUTF8);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, UseReliableCEDonly);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, LoadNFOasOEM);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, NoEncodingTags);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, SkipUnicodeDetection);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, SkipANSICodePageDetection);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, DefaultEOLMode);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, WarnInconsistEOLs);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, FixLineEndings);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, FixTrailingBlanks);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, PrintHeader);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, PrintFooter);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, PrintColorMode);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, PrintZoom);

  if (Settings.PrintMargin.left != Defaults.PrintMargin.left) {
    IniSectionSetInt(IniSecSettings, L"PrintMarginLeft", Settings.PrintMargin.left);
  }
  else {
    IniSectionDelete(IniSecSettings, L"PrintMarginLeft", false);
  }
  if (Settings.PrintMargin.top != Defaults.PrintMargin.top) {
    IniSectionSetInt(IniSecSettings, L"PrintMarginTop", Settings.PrintMargin.top);
  }
  else {
    IniSectionDelete(IniSecSettings, L"PrintMarginTop", false);
  }
  if (Settings.PrintMargin.right != Defaults.PrintMargin.right) {
    IniSectionSetInt(IniSecSettings, L"PrintMarginRight", Settings.PrintMargin.right);
  }
  else {
    IniSectionDelete(IniSecSettings, L"PrintMarginRight", false);
  }
  if (Settings.PrintMargin.bottom != Defaults.PrintMargin.bottom) {
    IniSectionSetInt(IniSecSettings, L"PrintMarginBottom", Settings.PrintMargin.bottom);
  }
  else {
    IniSectionDelete(IniSecSettings, L"PrintMarginBottom", false);
  }
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, SaveBeforeRunningTools);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, EvalTinyExprOnSelection);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, FileWatchingMode);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ResetFileWatching);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, EscFunction);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, AlwaysOnTop);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MinimizeToTray);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, TransparentMode);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, FindReplaceTransparentMode);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, RenderingTechnology);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, Bidirectional);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MuteMessageBeep);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, SplitUndoTypingSeqOnLnBreak);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, EditLayoutRTL);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, DialogsLayoutRTL);

  ///~IniSectionSetInt(IniSecSettings, L"IMEInteraction", Settings2.IMEInteraction);

  Toolbar_GetButtons(Globals.hwndToolbar, IDT_FILE_NEW, Settings.ToolbarButtons, COUNTOF(Settings.ToolbarButtons));
  if (StringCchCompareX(Settings.ToolbarButtons, Defaults.ToolbarButtons) != 0) {
    IniSectionSetString(IniSecSettings, L"ToolbarButtons", Settings.ToolbarButtons);
  }
  else {
    IniSectionDelete(IniSecSettings, L"ToolbarButtons", false);
  }
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ShowMenubar);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ShowToolbar);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ShowStatusbar);

  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, EncodingDlgSizeX);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, EncodingDlgSizeY);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, RecodeDlgSizeX);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, RecodeDlgSizeY);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, FileMRUDlgSizeX);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, FileMRUDlgSizeY);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, OpenWithDlgSizeX);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, OpenWithDlgSizeY);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, FavoritesDlgSizeX);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, FavoritesDlgSizeY);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, AddToFavDlgSizeX);

  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, FindReplaceDlgPosX);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, FindReplaceDlgPosY);

  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, CustomSchemesDlgPosX);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, CustomSchemesDlgPosY);

  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, FocusViewMarkerMode);

  // --------------------------------------------------------------------------
  const WCHAR* const IniSecSettings2 = Constants.Settings2_Section;
  // --------------------------------------------------------------------------

  // ---  remove deprecated  ---
  IniSectionDelete(IniSecSettings2, L"MarkOccurrencesMaxCount", false);

  // --------------------------------------------------------------------------
  const WCHAR* const IniSecWindow = Constants.Window_Section;
  // --------------------------------------------------------------------------

  int const ResX = GetSystemMetrics(SM_CXVIRTUALSCREEN);
  int const ResY = GetSystemMetrics(SM_CYVIRTUALSCREEN);

  WCHAR tchHighDpiToolBar[64];
  StringCchPrintf(tchHighDpiToolBar, COUNTOF(tchHighDpiToolBar), L"%ix%i HighDpiToolBar", ResX, ResY);
  if (Settings.ToolBarTheme != Defaults.ToolBarTheme) {
    IniSectionSetInt(IniSecWindow, tchHighDpiToolBar, Settings.ToolBarTheme);
  }
  else {
    IniSectionDelete(IniSecWindow, tchHighDpiToolBar, false);
  }

  StringCchPrintf(tchHighDpiToolBar, COUNTOF(tchHighDpiToolBar), L"%ix%i DpiScaleToolBar", ResX, ResY);
  if (Settings.DpiScaleToolBar != Defaults.DpiScaleToolBar) {
    IniSectionSetBool(IniSecWindow, tchHighDpiToolBar, Settings.DpiScaleToolBar);
  }
  else {
    IniSectionDelete(IniSecWindow, tchHighDpiToolBar, false);
  }

  if (!Flags.bStickyWindowPosition) {
    SaveWindowPositionSettings(false);
  }

  
  // --------------------------------------------------------------------------
  const WCHAR* const IniSecStyles = Constants.Styles_Section;
  // --------------------------------------------------------------------------

  switch (Globals.idxSelectedTheme) {
    case 1: 
      Style_ToIniSection(Globals.bIniFileFromScratch, true); // Scintilla Styles
      //~break;     
    case 0: // fall trough
      IniSectionDelete(IniSecStyles, Constants.StylingThemeName, false);
      break;
    default:
      IniSectionSetString(IniSecStyles, Constants.StylingThemeName, Theme_Files[Globals.idxSelectedTheme].szName);
      break;
  }

  return true;
}


//=============================================================================
//
//  SaveWindowPositionSettings()   
//
bool SaveWindowPositionSettings(bool bClearSettings)
{
  if (!IsIniFileCached()) { return false; }

  // set current window position as ne initial window
  WININFO const winInfo = GetMyWindowPlacement(Globals.hwndMain, NULL);

  int const ResX = GetSystemMetrics(SM_CXVIRTUALSCREEN);
  int const ResY = GetSystemMetrics(SM_CYVIRTUALSCREEN);
  WCHAR tchPosX[64], tchPosY[64], tchSizeX[64], tchSizeY[64], tchMaximized[64], tchZoom[64];
  StringCchPrintf(tchPosX, COUNTOF(tchPosX), L"%ix%i PosX", ResX, ResY);
  StringCchPrintf(tchPosY, COUNTOF(tchPosY), L"%ix%i PosY", ResX, ResY);
  StringCchPrintf(tchSizeX, COUNTOF(tchSizeX), L"%ix%i SizeX", ResX, ResY);
  StringCchPrintf(tchSizeY, COUNTOF(tchSizeY), L"%ix%i SizeY", ResX, ResY);
  StringCchPrintf(tchMaximized, COUNTOF(tchMaximized), L"%ix%i Maximized", ResX, ResY);
  StringCchPrintf(tchZoom, COUNTOF(tchMaximized), L"%ix%i Zoom", ResX, ResY);

  if (bClearSettings) {
    IniSectionDelete(Constants.Window_Section, tchPosX, false);
    IniSectionDelete(Constants.Window_Section, tchPosY, false);
    IniSectionDelete(Constants.Window_Section, tchSizeX, false);
    IniSectionDelete(Constants.Window_Section, tchSizeY, false);
    IniSectionDelete(Constants.Window_Section, tchMaximized, false);
    IniSectionDelete(Constants.Window_Section, tchZoom, false);
  }
  else {
    // overwrite last saved window position
    IniSectionSetInt(Constants.Window_Section, tchPosX, winInfo.x);
    IniSectionSetInt(Constants.Window_Section, tchPosY, winInfo.y);
    IniSectionSetInt(Constants.Window_Section, tchSizeX, winInfo.cx);
    IniSectionSetInt(Constants.Window_Section, tchSizeY, winInfo.cy);
    IniSectionSetBool(Constants.Window_Section, tchMaximized, winInfo.max);
    IniSectionSetInt(Constants.Window_Section, tchZoom, winInfo.zoom);
    // set current window position as new initial window
    g_IniWinInfo = winInfo;
  }
  
  return true;
}


//=============================================================================
//
//  SaveAllSettings()
//
bool SaveAllSettings(bool bForceSaveSettings)
{
  if (Flags.bDoRelaunchElevated) { return true; } // already saved before relaunch
  if (Flags.bSettingsFileSoftLocked) { return false; }
  
  WCHAR tchMsg[80];
  GetLngString(IDS_MUI_SAVINGSETTINGS, tchMsg, COUNTOF(tchMsg));

  bool ok = false;

  BeginWaitCursor(true,tchMsg);

__try {

    bool dummy = false;
    ok = OpenSettingsFile(&dummy);

    if (ok) {

      _SaveSettings(bForceSaveSettings);

      if (Globals.bCanSaveIniFile)
      {
        if (!Settings.SaveRecentFiles) {
          // Cleanup unwanted MRUs
          MRU_Empty(Globals.pFileMRU, false);
          MRU_Save(Globals.pFileMRU);
        }
        else {
          //int const cnt = MRU_Count(Globals.pFileMRU);
          MRU_MergeSave(Globals.pFileMRU, true, Flags.RelativeFileMRU, Flags.PortableMyDocs);
        }

        if (!Settings.SaveFindReplace) {
          // Cleanup unwanted MRUs
          MRU_Empty(Globals.pMRUfind, false);
          MRU_Save(Globals.pMRUfind);
          MRU_Empty(Globals.pMRUreplace, false);
          MRU_Save(Globals.pMRUreplace);
        }
        else {
          MRU_MergeSave(Globals.pMRUfind, false, false, false);
          MRU_MergeSave(Globals.pMRUreplace, false, false, false);
        }
      }
    }

    if (Globals.idxSelectedTheme == 1) {
      Style_SaveSettings(bForceSaveSettings);
    }

  }
  __finally {
    ok = CloseSettingsFile(true, false);
  }

  // separate INI files for Style-Themes
  if (Globals.idxSelectedTheme >= 2)
  {
    Style_SaveSettings(bForceSaveSettings);
  }

  EndWaitCursor();
  return ok;
}



//=============================================================================
//
//  CmdSaveSettingsNow()
//
void CmdSaveSettingsNow()
{
  bool bCreateFailure = false;
  if (StrIsEmpty(Globals.IniFile)) {
    if (StrIsNotEmpty(Globals.IniFileDefault)) {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), Globals.IniFileDefault);
      DWORD dwFileSize        = 0UL;
      Globals.bCanSaveIniFile = CreateIniFile(Globals.IniFile, &dwFileSize);
      if (Globals.bCanSaveIniFile) {
        Globals.bIniFileFromScratch = (dwFileSize == 0UL);
        StringCchCopy(Globals.IniFileDefault, COUNTOF(Globals.IniFileDefault), L"");
      }
      else {
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), L"");
        Globals.bCanSaveIniFile = false;
        bCreateFailure          = true;
      }
    }
    else {
      return;
    }
  }
  if (bCreateFailure) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_CREATEINI_FAIL);
    return;
  }
  DWORD dwFileAttributes = 0;
  if (!Globals.bCanSaveIniFile) {
    dwFileAttributes = GetFileAttributes(Globals.IniFile);
    if (dwFileAttributes == INVALID_FILE_ATTRIBUTES) {
      InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_CREATEINI_FAIL);
      return;
    }
    if (dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
      INT_PTR const answer = InfoBoxLng(MB_YESNO | MB_ICONWARNING, NULL, IDS_MUI_INIFILE_READONLY);
      if ((IDOK == answer) || (IDYES == answer)) {
        SetFileAttributes(Globals.IniFile, FILE_ATTRIBUTE_NORMAL); // override read-only attrib
        Globals.bCanSaveIniFile = CanAccessPath(Globals.IniFile, GENERIC_WRITE);
      }
    }
    else {
      dwFileAttributes = 0; // no need to change the file attributes
    }
  }
  if (Globals.bCanSaveIniFile && SaveAllSettings(true)) {
    InfoBoxLng(MB_ICONINFORMATION, L"MsgSaveSettingsInfo", IDS_MUI_SAVEDSETTINGS);
    if ((dwFileAttributes != 0) && (dwFileAttributes != INVALID_FILE_ATTRIBUTES)) {
      SetFileAttributes(Globals.IniFile, dwFileAttributes); // reset
    }
    Globals.bCanSaveIniFile = CanAccessPath(Globals.IniFile, GENERIC_WRITE);
  }
  else {
    Globals.dwLastError = GetLastError();
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_WRITEINI_FAIL);
    return;
  }
}
 

//=============================================================================
//=============================================================================


//=============================================================================
//
//  MRU functions
//
LPMRULIST MRU_Create(LPCWSTR pszRegKey, int iFlags, int iSize)
{
  LPMRULIST pmru = (LPMRULIST)AllocMem(sizeof(MRULIST), HEAP_ZERO_MEMORY);
  if (pmru) {
    ZeroMemory(pmru, sizeof(MRULIST));
    pmru->szRegKey = pszRegKey;
    pmru->iFlags = iFlags;
    pmru->iSize = min_i(iSize, MRU_MAXITEMS);
  }
  return(pmru);
}


bool MRU_Destroy(LPMRULIST pmru)
{
  if (pmru) {
    for (int i = 0; i < pmru->iSize; i++) {
      if (pmru->pszItems[i])
        LocalFree(pmru->pszItems[i]);  // StrDup()
      if (pmru->pszBookMarks[i])
        LocalFree(pmru->pszBookMarks[i]);  // StrDup()
    }
    ZeroMemory(pmru, sizeof(MRULIST));
    FreeMem(pmru);
    return true;
  }
  return false;
}


int MRU_Compare(LPMRULIST pmru, LPCWSTR psz1, LPCWSTR psz2)
{
  if (pmru) {
    if (pmru->iFlags & MRU_NOCASE) {
      return(StringCchCompareXI(psz1, psz2));
    }
    return(StringCchCompareX(psz1, psz2));
  }
  return 0;
}


bool MRU_Add(LPMRULIST pmru, LPCWSTR pszNew, cpi_enc_t iEnc, DocPos iPos, DocPos iSelAnc, LPCWSTR pszBookMarks)
{
  if (pmru) {
    int i = 0;
    for (; i < pmru->iSize; ++i) {
      if (MRU_Compare(pmru, pmru->pszItems[i], pszNew) == 0) {
        LocalFree(pmru->pszItems[i]); // StrDup()
        break;
      }
    }
    i = min_i(i, pmru->iSize - 1);
    for (; i > 0; i--) {
      pmru->pszItems[i] = pmru->pszItems[i - 1];
      pmru->iEncoding[i] = pmru->iEncoding[i - 1];
      pmru->iCaretPos[i] = pmru->iCaretPos[i - 1];
      pmru->iSelAnchPos[i] = pmru->iSelAnchPos[i - 1];
    }
    pmru->pszItems[0] = StrDup(pszNew); // LocalAlloc()

    pmru->iEncoding[0] = iEnc;
    pmru->iCaretPos[0] = (Settings.PreserveCaretPos ? iPos : -1);
    pmru->iSelAnchPos[0] = (Settings.PreserveCaretPos ? iSelAnc : -1);
    pmru->pszBookMarks[0] = (pszBookMarks ? StrDup(pszBookMarks) : NULL);  // LocalAlloc()
    return true;
  }
  return false;
}


bool MRU_FindFile(LPMRULIST pmru, LPCWSTR pszFile, int* iIndex) 
{
  *iIndex = 0;
  if (pmru) {
    WCHAR wchItem[MAX_PATH] = { L'\0' };
    int i = 0;
    for (i = 0; i < pmru->iSize; ++i) {
      if (pmru->pszItems[i] == NULL) {
        *iIndex = i;
        return false;
      }
      if (StringCchCompareXI(pmru->pszItems[i], pszFile) == 0) {
        *iIndex = i;
        return true;
      }
      PathAbsoluteFromApp(pmru->pszItems[i], wchItem, COUNTOF(wchItem), true);
      if (StringCchCompareXI(wchItem, pszFile) == 0) {
        *iIndex = i;
        return true;
      }
    }
    *iIndex = i;
  }
  return false;
}


bool MRU_AddFile(LPMRULIST pmru, LPWSTR pszFile, bool bRelativePath, bool bUnexpandMyDocs,
  cpi_enc_t iEnc, DocPos iPos, DocPos iSelAnc, LPCWSTR pszBookMarks)
{
  if (pmru) {
    int i = 0;
    bool const bAlreadyInList = MRU_FindFile(pmru, pszFile, &i);
    if (bAlreadyInList) {
      LocalFree(pmru->pszItems[i]);  // StrDup()
    }
    else {
      i = (i < pmru->iSize) ? i : (pmru->iSize - 1);
    }
    for (; i > 0; i--) {
      pmru->pszItems[i] = pmru->pszItems[i - 1];
      pmru->iEncoding[i] = pmru->iEncoding[i - 1];
      pmru->iCaretPos[i] = pmru->iCaretPos[i - 1];
      pmru->iSelAnchPos[i] = pmru->iSelAnchPos[i - 1];
      pmru->pszBookMarks[i] = pmru->pszBookMarks[i - 1];
    }
    if (bRelativePath) {
      WCHAR wchFile[MAX_PATH] = { L'\0' };
      PathRelativeToApp(pszFile, wchFile, COUNTOF(wchFile), true, true, bUnexpandMyDocs);
      pmru->pszItems[0] = StrDup(wchFile);  // LocalAlloc()
    }
    else {
      pmru->pszItems[0] = StrDup(pszFile);  // LocalAlloc()
    }

    pmru->iEncoding[0] = iEnc;
    pmru->iCaretPos[0] = (Settings.PreserveCaretPos ? iPos : -1);
    pmru->iSelAnchPos[0] = (Settings.PreserveCaretPos ? iSelAnc : -1);
    pmru->pszBookMarks[0] = (pszBookMarks ? StrDup(pszBookMarks) : NULL);  // LocalAlloc()

    if (!bAlreadyInList) {
      AddFilePathToRecentDocs(pszFile);
    }
    return bAlreadyInList;
  }
  return false;
}


bool MRU_Delete(LPMRULIST pmru, int iIndex) 
{
  if (pmru) {
    if (iIndex >= 0 || iIndex < pmru->iSize)
    {
      if (pmru->pszItems[iIndex]) {
        LocalFree(pmru->pszItems[iIndex]);  // StrDup()
      }
      if (pmru->pszBookMarks[iIndex]) {
        LocalFree(pmru->pszBookMarks[iIndex]);  // StrDup()
      }
      bool bZeroMoved = false;
      for (int i = iIndex; (i < pmru->iSize - 1) && !bZeroMoved; ++i)
      {
        pmru->pszItems[i] = pmru->pszItems[i + 1];
        pmru->iEncoding[i] = pmru->iEncoding[i + 1];
        pmru->iCaretPos[i] = pmru->iCaretPos[i + 1];
        pmru->iSelAnchPos[i] = pmru->iSelAnchPos[i + 1];
        pmru->pszBookMarks[i] = pmru->pszBookMarks[i + 1];

        bZeroMoved = (NULL == pmru->pszItems[i + 1]);

        pmru->pszItems[i + 1] = NULL;
        pmru->iEncoding[i + 1] = 0;
        pmru->iCaretPos[i + 1] = -1;
        pmru->iSelAnchPos[i + 1] = -1;
        pmru->pszBookMarks[i + 1] = NULL;
      }
      return true;
    }
  }
  return false;
}


bool MRU_Empty(LPMRULIST pmru, bool bExceptLeast)
{
  if (pmru) {
    int const beg = bExceptLeast ? 1 : 0;
    for (int i = beg; i < pmru->iSize; ++i) {
      if (pmru->pszItems[i]) {
        LocalFree(pmru->pszItems[i]);  // StrDup()
        pmru->pszItems[i] = NULL;
        pmru->iEncoding[i] = 0;
        pmru->iCaretPos[i] = -1;
        pmru->iSelAnchPos[i] = -1;
        if (pmru->pszBookMarks[i]) {
          LocalFree(pmru->pszBookMarks[i]);  // StrDup()
        }
        pmru->pszBookMarks[i] = NULL;
      }
    }
    return true;
  }
  return false;
}


int MRU_Enum(LPMRULIST pmru, int iIndex, LPWSTR pszItem, int cchItem)
{
  if (pmru) {
    if (pszItem == NULL || cchItem == 0) {
      int i = 0;
      while (i < pmru->iSize && pmru->pszItems[i]) { ++i; }
      return(i);
    }
    if (iIndex < 0 || iIndex > pmru->iSize - 1 || !pmru->pszItems[iIndex]) {
      return(-1);
    }
    StringCchCopyN(pszItem, cchItem, pmru->pszItems[iIndex], cchItem);
    return((int)StringCchLen(pszItem, cchItem));
  }
  return 0;
}


bool MRU_Load(LPMRULIST pmru, bool bFileProps)
{
  if (pmru) 
  {
    bool bOpendByMe = false;
    OpenSettingsFile(&bOpendByMe);

    int n = 0;
    if (IsIniFileCached()) {

      MRU_Empty(pmru, false);

      const WCHAR* const RegKey_Section = pmru->szRegKey;

      for (int i = 0; i < pmru->iSize; ++i)
      {
        WCHAR tchName[32] = { L'\0' };
        StringCchPrintf(tchName, COUNTOF(tchName), L"%.2i", i + 1);
        WCHAR tchItem[2048] = { L'\0' };
        if (IniSectionGetString(RegKey_Section, tchName, L"", tchItem, COUNTOF(tchItem)))
        {
          size_t const len = StringCchLen(tchItem, 0);
          if ((len > 1) && (tchItem[0] == L'"') && (tchItem[len - 1] == L'"')) {
            MoveMemory(tchItem, (tchItem + 1), len * sizeof(WCHAR));
            tchItem[len - 2] = L'\0'; // clear dangling '"'
          }
          pmru->pszItems[n] = StrDup(tchItem);

          StringCchPrintf(tchName, COUNTOF(tchName), L"ENC%.2i", i + 1);
          int const iCP = (cpi_enc_t)IniSectionGetInt(RegKey_Section, tchName, 0);
          pmru->iEncoding[n] = bFileProps ? (cpi_enc_t)Encoding_MapIniSetting(true, iCP) : 0;

          StringCchPrintf(tchName, COUNTOF(tchName), L"POS%.2i", i + 1);
          pmru->iCaretPos[n] = bFileProps ? ((Settings.PreserveCaretPos) ? IniSectionGetInt(RegKey_Section, tchName, 0) : -1) : -1;

          StringCchPrintf(tchName, COUNTOF(tchName), L"ANC%.2i", i + 1);
          pmru->iSelAnchPos[n] = bFileProps ? ((Settings.PreserveCaretPos) ? IniSectionGetInt(RegKey_Section, tchName, 0) : -1) : -1;

          StringCchPrintf(tchName, COUNTOF(tchName), L"BMRK%.2i", i + 1);

          WCHAR wchBookMarks[MRU_BMRK_SIZE] = { L'\0' };
          IniSectionGetString(RegKey_Section, tchName, L"", wchBookMarks, COUNTOF(wchBookMarks));
          pmru->pszBookMarks[n] = bFileProps ? StrDup(wchBookMarks) : nullptr;

          ++n;
        }
      }
      CloseSettingsFile(false, bOpendByMe);
    }
    return true;
  }
  return false;
}


void MRU_Save(LPMRULIST pmru)
{
  if (pmru) {
    bool bOpendByMe = false;
    OpenSettingsFile(&bOpendByMe);

    if (IsIniFileCached()) {
      WCHAR tchName[32] = { L'\0' };
      WCHAR tchItem[2048] = { L'\0' };

      const WCHAR* const RegKey_Section = pmru->szRegKey;
      IniSectionClear(pmru->szRegKey, false);

      for (int i = 0; i < pmru->iSize; ++i) {
        if (pmru->pszItems[i]) {
          StringCchPrintf(tchName, COUNTOF(tchName), L"%.2i", i + 1);
          StringCchPrintf(tchItem, COUNTOF(tchItem), L"\"%s\"", pmru->pszItems[i]);
          IniSectionSetString(RegKey_Section, tchName, tchItem);

          if (pmru->iEncoding[i] > 0) {
            StringCchPrintf(tchName, COUNTOF(tchName), L"ENC%.2i", i + 1);
            int const iCP = (int)Encoding_MapIniSetting(false, (int)pmru->iEncoding[i]);
            IniSectionSetInt(RegKey_Section, tchName, iCP);
          }
          if (pmru->iCaretPos[i] >= 0) {
            StringCchPrintf(tchName, COUNTOF(tchName), L"POS%.2i", i + 1);
            IniSectionSetPos(RegKey_Section, tchName, pmru->iCaretPos[i]);
          }
          if (pmru->iSelAnchPos[i] >= 0) {
            StringCchPrintf(tchName, COUNTOF(tchName), L"ANC%.2i", i + 1);
            IniSectionSetPos(RegKey_Section, tchName, pmru->iSelAnchPos[i]);
          }
          if (StrIsNotEmpty(pmru->pszBookMarks[i])) {
            StringCchPrintf(tchName, COUNTOF(tchName), L"BMRK%.2i", i + 1);
            IniSectionSetString(RegKey_Section, tchName, pmru->pszBookMarks[i]);
          }
        }
      }
      CloseSettingsFile(true, bOpendByMe);
    }
  }
}


bool MRU_MergeSave(LPMRULIST pmru, bool bAddFiles, bool bRelativePath, bool bUnexpandMyDocs)
{
  if (pmru) {

    bool bOpendByMe = false;
    OpenSettingsFile(&bOpendByMe);

    if (IsIniFileCached()) {

      LPMRULIST pmruBase = MRU_Create(pmru->szRegKey, pmru->iFlags, pmru->iSize);
      MRU_Load(pmruBase, bAddFiles);

      if (bAddFiles) {
        for (int i = pmru->iSize - 1; i >= 0; i--) {
          if (pmru->pszItems[i]) {
            WCHAR wchItem[MAX_PATH] = { L'\0' };
            PathAbsoluteFromApp(pmru->pszItems[i], wchItem, COUNTOF(wchItem), true);
            MRU_AddFile(pmruBase, wchItem, bRelativePath, bUnexpandMyDocs,
              pmru->iEncoding[i], pmru->iCaretPos[i], pmru->iSelAnchPos[i], pmru->pszBookMarks[i]);
          }
        }
      }
      else {
        for (int i = pmru->iSize - 1; i >= 0; i--) {
          if (pmru->pszItems[i])
            MRU_Add(pmruBase, pmru->pszItems[i],
              pmru->iEncoding[i], pmru->iCaretPos[i], pmru->iSelAnchPos[i], pmru->pszBookMarks[i]);
        }
      }

      MRU_Save(pmruBase);
      MRU_Destroy(pmruBase);
      pmruBase = NULL;

      CloseSettingsFile(true, bOpendByMe);

      return true;
    }
  }
  return false;
}

// ////////////////////////////////////////////////////////////////////////////
// Some C++ Extentions for Notepad3
// ////////////////////////////////////////////////////////////////////////////

//=============================================================================
//
//  EditSetDocumentBuffer() - Set Document Buffer for Scintilla Edit Component 
//

#if TRUE
static bool CreateNewDocument(const char* lpstrText, DocPosU lenText, int docOptions)
{
  #define RELEASE_RETURN(ret)  { pDocLoad->Release(); return(ret); }

  if (!lpstrText || (lenText == 0)) {
    SciCall_SetDocPointer(0);
  }
  else {
#if TRUE
    ILoader* const pDocLoad = reinterpret_cast<ILoader*>(SciCall_CreateLoader(static_cast<Sci_Position>(lenText) + 1, docOptions));

    if (SC_STATUS_OK != pDocLoad->AddData(lpstrText, lenText)) {
      RELEASE_RETURN(false);
    }
    sptr_t const pNewDocumentPtr = (sptr_t)pDocLoad->ConvertToDocument(); // == SciCall_CreateDocument(lenText, docOptions);
    if (!pNewDocumentPtr) {
      RELEASE_RETURN(false);
    }
    SciCall_SetDocPointer(pNewDocumentPtr);
    SciCall_ReleaseDocument(pNewDocumentPtr);
#else
    sptr_t const pNewDocumentPtr = SciCall_CreateDocument(lenText, docOptions);
    if (pNewDocumentPtr) {
      SciCall_SetDocPointer(pNewDocumentPtr);
      SciCall_ReleaseDocument(pNewDocumentPtr);
    }
    else {
      SciCall_SetDocPointer(0);
    }
    SciCall_TargetWholeDocument();
    SciCall_ReplaceTarget(lenText, lpstrText);
#endif
  }
  return true;
}
#else
static bool CreateNewDocument(const char* lpstrText, DocPosU lenText, int docOptions)
{
  UNUSED(docOptions);
  if (!lpstrText || (lenText == 0)) {
    SciCall_ClearAll();
  }
  else {
    SciCall_TargetWholeDocument();
    SciCall_ReplaceTarget(lenText, lpstrText);
  }
  return true;
}
#endif


extern "C" bool EditSetDocumentBuffer(const char* lpstrText, DocPosU lenText)
{
  bool const bLargerThan2GB = (lenText >= ((DocPosU)INT32_MAX));
  bool const bLargeFileLoaded = (lenText >= ((DocPosU)Settings2.FileLoadWarningMB << 20));
  int const docOptions = bLargeFileLoaded ? (bLargerThan2GB ? SC_DOCUMENTOPTION_TEXT_LARGE : SC_DOCUMENTOPTION_STYLES_NONE) 
                                          : SC_DOCUMENTOPTION_DEFAULT;

  if (SciCall_GetDocumentOptions() != docOptions)
  {
    // we have to create a new document with changed options
    return CreateNewDocument(lpstrText, lenText, docOptions);
  }
  else {
    if (!lpstrText || (lenText == 0)) {
      SciCall_ClearAll();
    }
    else {
      SciCall_TargetWholeDocument();
      SciCall_ReplaceTarget(lenText, lpstrText);
    }
  }
  return true;
}

