/******************************************************************************
*                                                                             *
* Config.cpp                                                                  *
*   TODO:      HACK                                                           *

addindex 

*******************************************************************************/

#include <strsafe.h>
#include <shlobj.h>

// TODO :   fkdlkldfdl

// ----------------------------------------------------------------------------
namespace {

1
file:///D:/DEV/GitHub/Notepad3/language/np3_af_za/menu_af_za.rc:472
file:///D:/DEV/GitHub/Notepad3/language/np3_be_by/menu_be_by.rc:472
file:///D:/DEV/GitHub/Notepad3/language/np3_de_de/menu_de_de.rc:472
7ꟹ
8
9
𖠀

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================
extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================

extern "C" {
#include "Version.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "resource.h"
}

extern "C" const int g_FontQuality[4];
extern "C" WININFO   s_WinInfo;
extern "C" WININFO   s_DefWinInfo;

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
extern "C" prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];
extern "C" bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       s_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" WCHAR     s_tchToolbarBitmap[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapHot[MAX_PATH];
extern "C" WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH];

extern "C" bool      s_bEnableSaveSettings;
extern "C" int       s_iToolBarTheme;

extern "C" bool      s_flagPosParam;
extern "C" int       s_flagWindowPos;
extern "C" int       s_flagReuseWindow;
extern "C" int       s_flagSingleFileInstance;
extern "C" int       s_flagMultiFileArg;
extern "C" int       s_flagShellUseSystemMRU;
extern "C" int       s_flagPrintFileAndLeave;


// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

#define SI_SUCCESS(RC) ((RC) >= SI_OK)

// ============================================================================

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return SI_SUCCESS(rc);
}

extern "C" bool SaveIniFile(LPCWSTR lpIniFilePath)
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
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
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
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return SI_SUCCESS(rc);
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
    return SI_SUCCESS(rc);
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
       s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) 
  {
    SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    rc = SI_SUCCESS(res) ? SI_OK : SI_FAIL;

    if (SI_SUCCESS(rc)) {
      Ini.SetSpaces(s_bSetSpaces);
      rc = Ini.SaveFile(Globals.IniFile, true);
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


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
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
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc)) {
    Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
  {
    Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return SI_SUCCESS(rc);
}
// ============================================================================


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
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathCchRemoveFileSpec(tchBuild, COUNTOF(tchBuild));
    StringCchCat(tchBuild, COUNTOF(tchBuild), L"\\np3\\");
    StringCchCat(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // Application Data (%APPDATA%)
    if (GetKnownFolderPath(FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile, MAX_PATH, tchBuild);
        return true;
      }
    }
    // Home (%HOMEPATH%) user's profile dir
    if (GetKnownFolderPath(FOLDERID_Profile, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild, COUNTOF(tchBuild), tchFileExpanded);
      if (PathFileExists(tchBuild)) {
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
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile, LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch, lpszModule)) {
      StringCchCopy(lpszFile, MAX_PATH, tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch, tchFileExpanded, COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile, MAX_PATH, lpszModule);
      StringCchCopy(PathFindFileName(lpszFile), MAX_PATH, tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile, MAX_PATH, tchFileExpanded);
    return true;
  }
  return false;
}
// ============================================================================


extern "C" bool FindIniFile()
{
  bool bFound = false;
  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchModule, COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
  SetEnvironmentVariable(NOTEPAD3_MODULE_DIR_ENV_VAR, tchPath);

  if (StrIsNotEmpty(Globals.IniFile)) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return bFound;
    }
    if (!_CheckIniFile(Globals.IniFile, tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile, COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        PathCchAppend(tchPath, COUNTOF(tchPath), Globals.IniFile);
        StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
      }
    }
  }
  else {
    StringCchCopy(tchPath, COUNTOF(tchPath), PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath, COUNTOF(tchPath), L".ini");

    bFound = _CheckIniFile(tchPath, tchModule);

    if (!bFound) {
      StringCchCopy(tchPath, COUNTOF(tchPath), L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath, tchModule);
    }

    if (bFound)
    {
      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule))
      {
        _CheckIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), tchModule);
      PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  return bFound;
}
//=============================================================================
