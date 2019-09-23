/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Config.cpp                                                                  *
*   Methods to read and write configuration                                   *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2019   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

//#include <locale.h>
#include <strsafe.h>
#include <shlobj.h>

// ----------------------------------------------------------------------------

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

extern "C"           THEMEFILES Theme_Files[];

// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static WCHAR* const _s_RecentFiles = L"Recent Files";
static WCHAR* const _s_RecentFind = L"Recent Find";
static WCHAR* const _s_RecentReplace = L"Recent Replace";

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
      rc = Ini.SaveFile(lpFilePath, true);
    }
    Ini.Reset();
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
    rc = Ini.SaveFile(lpFilePath, true);
  }
  Ini.Reset();
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
    rc = Ini.SaveFile(lpFilePath, true);
  }
  Ini.Reset();
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
    rc = Ini.SaveFile(lpFilePath, true);
  }
  Ini.Reset();
  return SI_SUCCESS(rc);
}
// ============================================================================



extern "C" bool IniFileIterateSection(LPCWSTR lpFilePath, LPCWSTR lpSectionName, IterSectionFunc_t callBack)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (SI_SUCCESS(rc))
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


extern "C" int TestIniFile() {

  if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
    StringCchCopy(Globals.IniFileDefault, COUNTOF(Globals.IniFileDefault), L"");
    StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), L"");
    return(0);
  }

  if (PathIsDirectory(Globals.IniFile) || *CharPrev(Globals.IniFile, StrEnd(Globals.IniFile, COUNTOF(Globals.IniFile))) == L'\\') {
    WCHAR wchModule[MAX_PATH] = { L'\0' };
    GetModuleFileName(NULL, wchModule, COUNTOF(wchModule));
    PathCchAppend(Globals.IniFile, COUNTOF(Globals.IniFile), PathFindFileName(wchModule));
    PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
    if (!PathFileExists(Globals.IniFile)) {
      StringCchCopy(PathFindFileName(Globals.IniFile), COUNTOF(Globals.IniFile), L"Notepad3.ini");
      if (!PathFileExists(Globals.IniFile)) {
        StringCchCopy(PathFindFileName(Globals.IniFile), COUNTOF(Globals.IniFile), PathFindFileName(wchModule));
        PathCchRenameExtension(Globals.IniFile, COUNTOF(Globals.IniFile), L".ini");
      }
    }
  }

  NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);

  if (!PathFileExists(Globals.IniFile) || PathIsDirectory(Globals.IniFile)) {
    StringCchCopy(Globals.IniFileDefault, COUNTOF(Globals.IniFileDefault), Globals.IniFile);
    StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), L"");
    return(0);
  }
  return(1);
}
//=============================================================================


extern "C" bool CreateIniFile()
{
  return(CreateIniFileEx(Globals.IniFile));
}
//=============================================================================

extern "C" bool CreateIniFileEx(LPCWSTR lpszIniFile)
{
  if (StrIsNotEmpty(lpszIniFile))
  {
    WCHAR* pwchTail = StrRChrW(lpszIniFile, NULL, L'\\');
    if (pwchTail) {
      *pwchTail = 0;
      SHCreateDirectoryEx(NULL, lpszIniFile, NULL);
      *pwchTail = L'\\';
    }

    HANDLE hFile = CreateFile(lpszIniFile,
      GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
      NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    Globals.dwLastError = GetLastError();
    if (hFile != INVALID_HANDLE_VALUE) {
      if (GetFileSize(hFile, NULL) == 0) {
        DWORD dw;
        //WriteFile(hFile,(LPCVOID)L"\xFE\xFF[Notepad3]\r\n",26,&dw,NULL); // UTF-16LE
        WriteFile(hFile, (LPCVOID)L"\xEF\xBB\xBF[Notepad3]\r\n", 26, &dw, NULL);  // UTF-8 SIG
        Globals.bIniFileFromScratch = true;
      }
      CloseHandle(hFile);
      Style_SetIniFile(lpszIniFile);
      return true;
    }
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
  int const _ver = StrIsEmpty(Globals.IniFile) ? CFG_VER_CURRENT : CFG_VER_NONE;

  LoadIniFile(Globals.IniFile);

  bool bDirtyFlag = false;  // do we have to save the file on done

  // prerequisites
  Globals.iCfgVersionRead = IniSectionGetInt(L"Settings", L"SettingsVersion", _ver);

  Defaults.SaveSettings = StrIsNotEmpty(Globals.IniFile);
  Settings.SaveSettings = IniSectionGetBool(L"Settings", L"SaveSettings", Defaults.SaveSettings);

  // --------------------------------------------------------------------------
  // first set "hard coded" .ini-Settings
  // --------------------------------------------------------------------------
  const WCHAR* const Settings2_Section = L"Settings2";

  Defaults2.PreferredLanguageLocaleName[0] = L'\0';
  IniSectionGetString(Settings2_Section, L"PreferredLanguageLocaleName", Defaults2.PreferredLanguageLocaleName,
    Settings2.PreferredLanguageLocaleName, COUNTOF(Settings2.PreferredLanguageLocaleName));
  //_wsetlocale(LC_COLLATE, Settings2.PreferredLanguageLocaleName);

  StringCchCopyW(Defaults2.DefaultExtension, COUNTOF(Defaults2.DefaultExtension), L"txt");
  IniSectionGetString(Settings2_Section, L"DefaultExtension", Defaults2.DefaultExtension,
    Settings2.DefaultExtension, COUNTOF(Settings2.DefaultExtension));
  StrTrim(Settings2.DefaultExtension, L" \t.\"");

  Defaults2.DefaultDirectory[0] = L'\0';
  IniSectionGetString(Settings2_Section, L"DefaultDirectory", Defaults2.DefaultDirectory,
    Settings2.DefaultDirectory, COUNTOF(Settings2.DefaultDirectory));

  Defaults2.FileDlgFilters[0] = L'\0';
  IniSectionGetString(Settings2_Section, L"FileDlgFilters", Defaults2.FileDlgFilters,
    Settings2.FileDlgFilters, COUNTOF(Settings2.FileDlgFilters) - 2);

  Defaults2.FileCheckInverval = 2000UL;
  Settings2.FileCheckInverval = clampul(IniSectionGetInt(Settings2_Section, L"FileCheckInverval",
    Defaults2.FileCheckInverval), 250UL, 300000UL);
  FileWatching.FileCheckInverval = Settings2.FileCheckInverval;

  Defaults2.AutoReloadTimeout = 2000UL;
  Settings2.AutoReloadTimeout = clampul(IniSectionGetInt(Settings2_Section, L"AutoReloadTimeout",
    Defaults2.AutoReloadTimeout), 250UL, 300000UL);
  FileWatching.AutoReloadTimeout = Settings2.AutoReloadTimeout;

  Defaults2.UndoTransactionTimeout = 0UL;
  Settings2.UndoTransactionTimeout = clampul(IniSectionGetInt(Settings2_Section, L"UndoTransactionTimeout",
    Defaults2.UndoTransactionTimeout), 0UL, 86400000UL);

  // deprecated
  Defaults.RenderingTechnology = IniSectionGetInt(Settings2_Section, L"SciDirectWriteTech", -111);
  if ((Defaults.RenderingTechnology != -111) && Settings.SaveSettings) {
    // cleanup
    IniSectionDelete(Settings2_Section, L"SciDirectWriteTech", false);
    bDirtyFlag = true;
  }
  Defaults.RenderingTechnology = clampi(Defaults.RenderingTechnology, 0, 3);

  // Settings2 deprecated
  Defaults.Bidirectional = IniSectionGetInt(Settings2_Section, L"EnableBidirectionalSupport", -111);
  if ((Defaults.Bidirectional != -111) && Settings.SaveSettings) {
    // cleanup
    IniSectionDelete(Settings2_Section, L"EnableBidirectionalSupport", false);
    bDirtyFlag = true;
  }
  Defaults.Bidirectional = (clampi(Defaults.Bidirectional, SC_BIDIRECTIONAL_DISABLED, SC_BIDIRECTIONAL_R2L) > 0) ? SC_BIDIRECTIONAL_R2L : 0;

  Defaults2.IMEInteraction = -1;
  Settings2.IMEInteraction = clampi(IniSectionGetInt(Settings2_Section, L"IMEInteraction", Defaults2.IMEInteraction), -1, SC_IME_INLINE);
  // Korean IME use inline mode by default
  if (Settings2.IMEInteraction == -1) { // auto detection once
    // ScintillaWin::KoreanIME()
    int const codePage = Scintilla_InputCodePage();
    Settings2.IMEInteraction = ((codePage == 949 || codePage == 1361) ? SC_IME_INLINE : SC_IME_WINDOWED);
  }

  Defaults2.SciFontQuality = g_FontQuality[3];
  Settings2.SciFontQuality = clampi(IniSectionGetInt(Settings2_Section, L"SciFontQuality", Defaults2.SciFontQuality), 0, 3);

  Defaults2.MarkOccurrencesMaxCount = 2000;
  Settings2.MarkOccurrencesMaxCount = IniSectionGetInt(Settings2_Section, L"MarkOccurrencesMaxCount", Defaults2.MarkOccurrencesMaxCount);
  if (Settings2.MarkOccurrencesMaxCount <= 0) { Settings2.MarkOccurrencesMaxCount = INT_MAX; }

  Defaults2.UpdateDelayMarkAllOccurrences = 50;
  Settings2.UpdateDelayMarkAllOccurrences = clampi(IniSectionGetInt(Settings2_Section, L"UpdateDelayMarkAllOccurrences",
    Defaults2.UpdateDelayMarkAllOccurrences), USER_TIMER_MINIMUM, 10000);
  Defaults2.DenyVirtualSpaceAccess = false;
  Settings2.DenyVirtualSpaceAccess = IniSectionGetBool(Settings2_Section, L"DenyVirtualSpaceAccess", Defaults2.DenyVirtualSpaceAccess);

  Defaults2.UseOldStyleBraceMatching = false;
  Settings2.UseOldStyleBraceMatching = IniSectionGetBool(Settings2_Section, L"UseOldStyleBraceMatching", Defaults2.UseOldStyleBraceMatching);

  Defaults2.CurrentLineHorizontalSlop = 40;
  Settings2.CurrentLineHorizontalSlop = clampi(IniSectionGetInt(Settings2_Section, L"CurrentLineHorizontalSlop", Defaults2.CurrentLineHorizontalSlop), 0, 240);

  Defaults2.CurrentLineVerticalSlop = 5;
  Settings2.CurrentLineVerticalSlop = clampi(IniSectionGetInt(Settings2_Section, L"CurrentLineVerticalSlop", Defaults2.CurrentLineVerticalSlop), 0, 25);

  Defaults2.NoCopyLineOnEmptySelection = false;
  Settings2.NoCopyLineOnEmptySelection = IniSectionGetBool(Settings2_Section, L"NoCopyLineOnEmptySelection", Defaults2.NoCopyLineOnEmptySelection);
    

  int const iARCLdef = 50;
  Defaults2.AnalyzeReliableConfidenceLevel = (float)iARCLdef / 100.0f;
  int const iARCLset = clampi(IniSectionGetInt(Settings2_Section, L"AnalyzeReliableConfidenceLevel", iARCLdef), 0, 100);
  Settings2.AnalyzeReliableConfidenceLevel = (float)iARCLset / 100.0f;

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
  IniSectionGetString(Settings2_Section, L"AdministrationTool.exe", Defaults2.AdministrationTool,
    Settings2.AdministrationTool, COUNTOF(Settings2.AdministrationTool));

  Defaults2.DefaultWindowPosition[0] = L'\0';
  IniSectionGetString(Settings2_Section, L"DefaultWindowPosition", Defaults2.DefaultWindowPosition,
    Settings2.DefaultWindowPosition, COUNTOF(Settings2.DefaultWindowPosition));
  bool const bExplicitDefaultWinPos = (StringCchLenW(Settings2.DefaultWindowPosition, 0) != 0);

  Defaults2.FileLoadWarningMB = 256;
  Settings2.FileLoadWarningMB = clampi(IniSectionGetInt(Settings2_Section, L"FileLoadWarningMB", Defaults2.FileLoadWarningMB), 0, 2048);

  Defaults2.OpacityLevel = 75;
  Settings2.OpacityLevel = clampi(IniSectionGetInt(Settings2_Section, L"OpacityLevel", Defaults2.OpacityLevel), 10, 100);

  Defaults2.FindReplaceOpacityLevel = 50;
  Settings2.FindReplaceOpacityLevel = clampi(IniSectionGetInt(Settings2_Section, L"FindReplaceOpacityLevel", Defaults2.FindReplaceOpacityLevel), 10, 100);

  Defaults2.FileBrowserPath[0] = L'\0';
  IniSectionGetString(Settings2_Section, L"filebrowser.exe", Defaults2.FileBrowserPath, Settings2.FileBrowserPath, COUNTOF(Settings2.FileBrowserPath));

  StringCchCopyW(Defaults2.AppUserModelID, COUNTOF(Defaults2.AppUserModelID), _W(SAPPNAME));
  IniSectionGetString(Settings2_Section, L"ShellAppUserModelID", Defaults2.AppUserModelID, Settings2.AppUserModelID, COUNTOF(Settings2.AppUserModelID));

  Defaults2.ExtendedWhiteSpaceChars[0] = L'\0';
  IniSectionGetString(Settings2_Section, L"ExtendedWhiteSpaceChars", Defaults2.ExtendedWhiteSpaceChars,
    Settings2.ExtendedWhiteSpaceChars, COUNTOF(Settings2.ExtendedWhiteSpaceChars));

  Defaults2.AutoCompleteWordCharSet[0] = L'\0';
  IniSectionGetString(Settings2_Section, L"AutoCompleteWordCharSet", Defaults2.AutoCompleteWordCharSet,
    Settings2.AutoCompleteWordCharSet, COUNTOF(Settings2.AutoCompleteWordCharSet));

  Defaults2.AutoCompleteFillUpChars[0] = L'\0';
  IniSectionGetString(Settings2_Section, L"AutoCompleteFillUpChars", Defaults2.AutoCompleteFillUpChars,
    Settings2.AutoCompleteFillUpChars, COUNTOF(Settings2.AutoCompleteFillUpChars));

  StringCchCopyW(Defaults2.TimeStamp, COUNTOF(Defaults2.TimeStamp), L"\\$Date:[^\\$]+\\$ | $Date: %Y/%m/%d %H:%M:%S $");
  IniSectionGetString(Settings2_Section, L"TimeStamp", Defaults2.TimeStamp, Settings2.TimeStamp, COUNTOF(Settings2.TimeStamp));

  Defaults2.DateTimeShort[0] = L'\0';
  IniSectionGetString(Settings2_Section, L"DateTimeShort", Defaults2.DateTimeShort, Settings2.DateTimeShort, COUNTOF(Settings2.DateTimeShort));

  Defaults2.DateTimeLong[0] = L'\0';
  IniSectionGetString(Settings2_Section, L"DateTimeLong", Defaults2.DateTimeLong, Settings2.DateTimeLong, COUNTOF(Settings2.DateTimeLong));

  StringCchCopyW(Defaults2.WebTemplate1, COUNTOF(Defaults2.WebTemplate1), L"https://google.com/search?q=%s");
  IniSectionGetString(Settings2_Section, L"WebTemplate1", Defaults2.WebTemplate1, Settings2.WebTemplate1, COUNTOF(Settings2.WebTemplate1));

  StringCchCopyW(Defaults2.WebTemplate2, COUNTOF(Defaults2.WebTemplate2), L"https://en.wikipedia.org/w/index.php?search=%s");
  IniSectionGetString(Settings2_Section, L"WebTemplate2", Defaults2.WebTemplate2, Settings2.WebTemplate2, COUNTOF(Settings2.WebTemplate2));

  // --------------------------------------------------------------------------
  const WCHAR* const Settings_Section = L"Settings";
  // --------------------------------------------------------------------------

#define GET_BOOL_VALUE_FROM_INISECTION(VARNAME,DEFAULT) \
  Defaults.VARNAME = DEFAULT;                           \
  Settings.VARNAME = IniSectionGetBool(Settings_Section,  _W(_STRG(VARNAME)), Defaults.VARNAME)

#define GET_INT_VALUE_FROM_INISECTION(VARNAME,DEFAULT,MIN,MAX) \
  Defaults.VARNAME = DEFAULT;                                  \
  Settings.VARNAME = clampi(IniSectionGetInt(Settings_Section,  _W(_STRG(VARNAME)), Defaults.VARNAME),MIN,MAX)

#define GET_CAST_INT_VALUE_FROM_INISECTION(CAST,VARNAME,DEFAULT,MIN,MAX) \
  Defaults.VARNAME = static_cast<CAST>(DEFAULT);                         \
  Settings.VARNAME = static_cast<CAST>(clampi(IniSectionGetInt(Settings_Section,  _W(_STRG(VARNAME)), Defaults.VARNAME),MIN,MAX))

#define GET_ENC_VALUE_FROM_INISECTION(VARNAME,DEFAULT,MIN,MAX) \
  Defaults.VARNAME = (cpi_enc_t)DEFAULT;                       \
  Settings.VARNAME = (cpi_enc_t)clampi(IniSectionGetInt(Settings_Section,  _W(_STRG(VARNAME)), (int)Defaults.VARNAME),(int)MIN,(int)MAX)

  GET_BOOL_VALUE_FROM_INISECTION(SaveRecentFiles, true);
  GET_BOOL_VALUE_FROM_INISECTION(PreserveCaretPos, false);
  GET_BOOL_VALUE_FROM_INISECTION(SaveFindReplace, false);

  Defaults.EFR_Data.bFindClose = false;
  Settings.EFR_Data.bFindClose = IniSectionGetBool(Settings_Section, L"CloseFind", Defaults.EFR_Data.bFindClose);
  Defaults.EFR_Data.bReplaceClose = false;
  Settings.EFR_Data.bReplaceClose = IniSectionGetBool(Settings_Section, L"CloseReplace", Defaults.EFR_Data.bReplaceClose);
  Defaults.EFR_Data.bNoFindWrap = false;
  Settings.EFR_Data.bNoFindWrap = IniSectionGetBool(Settings_Section, L"NoFindWrap", Defaults.EFR_Data.bNoFindWrap);
  Defaults.EFR_Data.bTransformBS = false;
  Settings.EFR_Data.bTransformBS = IniSectionGetBool(Settings_Section, L"FindTransformBS", Defaults.EFR_Data.bTransformBS);
  Defaults.EFR_Data.bWildcardSearch = false;
  Settings.EFR_Data.bWildcardSearch = IniSectionGetBool(Settings_Section, L"WildcardSearch", Defaults.EFR_Data.bWildcardSearch);
  Defaults.EFR_Data.bMarkOccurences = true;
  Settings.EFR_Data.bMarkOccurences = IniSectionGetBool(Settings_Section, L"FindMarkAllOccurrences", Defaults.EFR_Data.bMarkOccurences);
  Defaults.EFR_Data.bHideNonMatchedLines = false;
  Settings.EFR_Data.bHideNonMatchedLines = IniSectionGetBool(Settings_Section, L"HideNonMatchedLines", Defaults.EFR_Data.bHideNonMatchedLines);
  Defaults.EFR_Data.bDotMatchAll = false;
  Settings.EFR_Data.bDotMatchAll = IniSectionGetBool(Settings_Section, L"RegexDotMatchesAll", Defaults.EFR_Data.bDotMatchAll);
  Defaults.EFR_Data.fuFlags = 0;
  Settings.EFR_Data.fuFlags = (UINT)IniSectionGetInt(Settings_Section, L"efrData_fuFlags", (int)Defaults.EFR_Data.fuFlags);

  Defaults.OpenWithDir[0] = L'\0';
  if (!IniSectionGetString(Settings_Section, L"OpenWithDir", Defaults.OpenWithDir, Settings.OpenWithDir, COUNTOF(Settings.OpenWithDir))) {
    GetKnownFolderPath(FOLDERID_Desktop, Settings.OpenWithDir, COUNTOF(Settings.OpenWithDir));
  }
  else {
    PathAbsoluteFromApp(Settings.OpenWithDir, NULL, COUNTOF(Settings.OpenWithDir), true);
  }

  Defaults.FavoritesDir[0] = L'\0';
  //StringCchCopyW(Defaults.FavoritesDir, COUNTOF(Defaults.FavoritesDir), L"%USERPROFILE%");
  if (!IniSectionGetString(Settings_Section, L"Favorites", Defaults.FavoritesDir, Settings.FavoritesDir, COUNTOF(Settings.FavoritesDir))) {
    GetKnownFolderPath(FOLDERID_Favorites, Settings.FavoritesDir, COUNTOF(Settings.FavoritesDir));
  }
  else {
    PathAbsoluteFromApp(Settings.FavoritesDir, NULL, COUNTOF(Settings.FavoritesDir), true);
  }

  GET_INT_VALUE_FROM_INISECTION(PathNameFormat, 1, 0, 2);
  GET_INT_VALUE_FROM_INISECTION(WordWrapMode, 0, 0, 1);
  GET_INT_VALUE_FROM_INISECTION(WordWrapIndent, 2, 0, 6);

  GET_BOOL_VALUE_FROM_INISECTION(WordWrap, false);  Globals.fvBackup.bWordWrap = Settings.WordWrap;
  GET_BOOL_VALUE_FROM_INISECTION(TabsAsSpaces, false);  Globals.fvBackup.bTabsAsSpaces = Settings.TabsAsSpaces;
  GET_BOOL_VALUE_FROM_INISECTION(TabIndents, true);  Globals.fvBackup.bTabIndents = Settings.TabIndents;
  GET_INT_VALUE_FROM_INISECTION(TabWidth, 4, 1, 1024);  Globals.fvBackup.iTabWidth = Settings.TabWidth;
  GET_INT_VALUE_FROM_INISECTION(IndentWidth, 4, 0, 1024);  Globals.fvBackup.iIndentWidth = Settings.IndentWidth;
  GET_INT_VALUE_FROM_INISECTION(LongLinesLimit, 80, 0, LONG_LINES_MARKER_LIMIT);  Globals.fvBackup.iLongLinesLimit = Settings.LongLinesLimit;
  Globals.iWrapCol = Settings.LongLinesLimit;

  Defaults.WordWrapSymbols = 22;
  int const iWS = IniSectionGetInt(Settings_Section, L"WordWrapSymbols", Defaults.WordWrapSymbols);
  Settings.WordWrapSymbols = clampi(iWS % 10, 0, 2) + clampi((iWS % 100 - iWS % 10) / 10, 0, 2) * 10;

  GET_BOOL_VALUE_FROM_INISECTION(ShowWordWrapSymbols, true);
  GET_BOOL_VALUE_FROM_INISECTION(MatchBraces, true);
  GET_BOOL_VALUE_FROM_INISECTION(AutoCloseTags, false);
  GET_INT_VALUE_FROM_INISECTION(HighlightCurrentLine, 1, 0, 2);
  GET_BOOL_VALUE_FROM_INISECTION(HyperlinkHotspot, true);
  GET_BOOL_VALUE_FROM_INISECTION(ColorDefHotspot, true);
  GET_BOOL_VALUE_FROM_INISECTION(ScrollPastEOF, false);
  GET_BOOL_VALUE_FROM_INISECTION(ShowHypLnkToolTip, true);

  GET_BOOL_VALUE_FROM_INISECTION(AutoIndent, true);
  GET_BOOL_VALUE_FROM_INISECTION(AutoCompleteWords, false);
  GET_BOOL_VALUE_FROM_INISECTION(AutoCLexerKeyWords, false);
  GET_BOOL_VALUE_FROM_INISECTION(AccelWordNavigation, false);
  GET_BOOL_VALUE_FROM_INISECTION(ShowIndentGuides, false);
  GET_BOOL_VALUE_FROM_INISECTION(BackspaceUnindents, false);
  GET_BOOL_VALUE_FROM_INISECTION(WarnInconsistentIndents, false);
  GET_BOOL_VALUE_FROM_INISECTION(AutoDetectIndentSettings, false);
  GET_BOOL_VALUE_FROM_INISECTION(MarkLongLines, (Globals.iCfgVersionRead < CFG_VER_0002)); Defaults.MarkLongLines = false; // new default
  GET_INT_VALUE_FROM_INISECTION(LongLineMode, EDGE_LINE, EDGE_LINE, EDGE_BACKGROUND);
  GET_BOOL_VALUE_FROM_INISECTION(ShowSelectionMargin, true);
  GET_BOOL_VALUE_FROM_INISECTION(ShowLineNumbers, true);
  GET_BOOL_VALUE_FROM_INISECTION(ShowCodeFolding, true); FocusedView.ShowCodeFolding = Settings.ShowCodeFolding;

  GET_BOOL_VALUE_FROM_INISECTION(MarkOccurrences, true);
  GET_BOOL_VALUE_FROM_INISECTION(MarkOccurrencesMatchVisible, false);
  GET_BOOL_VALUE_FROM_INISECTION(MarkOccurrencesMatchCase, false);
  GET_BOOL_VALUE_FROM_INISECTION(MarkOccurrencesMatchWholeWords, true);

  Defaults.MarkOccurrencesCurrentWord = !Defaults.MarkOccurrencesMatchWholeWords;
  Settings.MarkOccurrencesCurrentWord = IniSectionGetBool(Settings_Section, L"MarkOccurrencesCurrentWord", Defaults.MarkOccurrencesCurrentWord);
  Settings.MarkOccurrencesCurrentWord = Settings.MarkOccurrencesCurrentWord && !Settings.MarkOccurrencesMatchWholeWords;

  GET_BOOL_VALUE_FROM_INISECTION(ViewWhiteSpace, false);
  GET_BOOL_VALUE_FROM_INISECTION(ViewEOLs, false);

  auto const iPrefEncIniSetting = (cpi_enc_t)Encoding_MapIniSetting(false, (int)CPI_UTF8);
  GET_ENC_VALUE_FROM_INISECTION(DefaultEncoding, iPrefEncIniSetting, CPI_NONE, INT_MAX);
  Settings.DefaultEncoding = ((Settings.DefaultEncoding == CPI_NONE) ? CPI_UTF8 : (cpi_enc_t)Encoding_MapIniSetting(true, (int)Settings.DefaultEncoding));
  GET_BOOL_VALUE_FROM_INISECTION(UseDefaultForFileEncoding, false);
  GET_BOOL_VALUE_FROM_INISECTION(LoadASCIIasUTF8, true);
  GET_BOOL_VALUE_FROM_INISECTION(UseReliableCEDonly, true);
  GET_BOOL_VALUE_FROM_INISECTION(LoadNFOasOEM, true);
  GET_BOOL_VALUE_FROM_INISECTION(NoEncodingTags, false);
  GET_BOOL_VALUE_FROM_INISECTION(SkipUnicodeDetection, false);
  GET_BOOL_VALUE_FROM_INISECTION(SkipANSICodePageDetection, false);
  GET_INT_VALUE_FROM_INISECTION(DefaultEOLMode, SC_EOL_CRLF, SC_EOL_CRLF, SC_EOL_LF);
  GET_BOOL_VALUE_FROM_INISECTION(WarnInconsistEOLs, true);
  GET_BOOL_VALUE_FROM_INISECTION(FixLineEndings, false);
  GET_BOOL_VALUE_FROM_INISECTION(FixTrailingBlanks, false);
  GET_INT_VALUE_FROM_INISECTION(PrintHeader, 1, 0, 3);
  GET_INT_VALUE_FROM_INISECTION(PrintFooter, 0, 0, 1);
  GET_INT_VALUE_FROM_INISECTION(PrintColorMode, 3, 0, 4);

  int const zoomScale = float2int(1000.0f / GetBaseFontSize(Globals.hwndMain));
  Defaults.PrintZoom = (Globals.iCfgVersionRead < CFG_VER_0001) ? (zoomScale / 10) : zoomScale;
  int iPrintZoom = clampi(IniSectionGetInt(Settings_Section, L"PrintZoom", Defaults.PrintZoom), 0, SC_MAX_ZOOM_LEVEL);
  if (Globals.iCfgVersionRead < CFG_VER_0001) { iPrintZoom = 100 + (iPrintZoom - 10) * 10; }
  Settings.PrintZoom = clampi(iPrintZoom, SC_MIN_ZOOM_LEVEL, SC_MAX_ZOOM_LEVEL);

  WCHAR localeInfo[3];
  GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_IMEASURE, localeInfo, 3);
  LONG const _margin = (localeInfo[0] == L'0') ? 2000L : 1000L; // Metric system. L'1' is US System
  Defaults.PrintMargin.left = _margin;
  Settings.PrintMargin.left = clampi(IniSectionGetInt(Settings_Section, L"PrintMarginLeft", Defaults.PrintMargin.left), 0, 40000);
  Defaults.PrintMargin.top = _margin;
  Settings.PrintMargin.top = clampi(IniSectionGetInt(Settings_Section, L"PrintMarginTop", Defaults.PrintMargin.top), 0, 40000);
  Defaults.PrintMargin.right = _margin;
  Settings.PrintMargin.right = clampi(IniSectionGetInt(Settings_Section, L"PrintMarginRight", Defaults.PrintMargin.right), 0, 40000);
  Defaults.PrintMargin.bottom = _margin;
  Settings.PrintMargin.bottom = clampi(IniSectionGetInt(Settings_Section, L"PrintMarginBottom", Defaults.PrintMargin.bottom), 0, 40000);

  GET_BOOL_VALUE_FROM_INISECTION(SaveBeforeRunningTools, false);
  GET_CAST_INT_VALUE_FROM_INISECTION(FILE_WATCHING_MODE, FileWatchingMode, FWM_NONE, FWM_NONE, FWM_AUTORELOAD);  FileWatching.FileWatchingMode = Settings.FileWatchingMode;
  GET_BOOL_VALUE_FROM_INISECTION(ResetFileWatching, true);   FileWatching.ResetFileWatching = Settings.ResetFileWatching;
  GET_INT_VALUE_FROM_INISECTION(EscFunction, 0, 0, 2);
  GET_BOOL_VALUE_FROM_INISECTION(AlwaysOnTop, false);
  GET_BOOL_VALUE_FROM_INISECTION(MinimizeToTray, false);
  GET_BOOL_VALUE_FROM_INISECTION(TransparentMode, false);
  GET_BOOL_VALUE_FROM_INISECTION(FindReplaceTransparentMode, true);
  GET_INT_VALUE_FROM_INISECTION(RenderingTechnology, Defaults.RenderingTechnology, 0, 3);  // set before
  GET_INT_VALUE_FROM_INISECTION(Bidirectional, Defaults.Bidirectional, 0, 2);  // set before
  GET_BOOL_VALUE_FROM_INISECTION(MuteMessageBeep, false);
  GET_BOOL_VALUE_FROM_INISECTION(SplitUndoTypingSeqOnLnBreak, false);

  ///~Settings2.IMEInteraction = clampi(IniSectionGetInt(Settings_Section, L"IMEInteraction", Settings2.IMEInteraction), SC_IME_WINDOWED, SC_IME_INLINE);

  // see TBBUTTON  s_tbbMainWnd[] for initial/reset set of buttons
  StringCchCopyW(Defaults.ToolbarButtons, COUNTOF(Defaults.ToolbarButtons), (Globals.iCfgVersionRead < CFG_VER_0002) ? TBBUTTON_DEFAULT_IDS_V1 : TBBUTTON_DEFAULT_IDS_V2);
  IniSectionGetString(Settings_Section, L"ToolbarButtons", Defaults.ToolbarButtons, Settings.ToolbarButtons, COUNTOF(Settings.ToolbarButtons));

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

  GET_INT_VALUE_FROM_INISECTION(FindReplaceDlgSizeX, 494, INT_MIN, INT_MAX);
  GET_INT_VALUE_FROM_INISECTION(FindReplaceDlgPosX, CW_USEDEFAULT, INT_MIN, INT_MAX);
  GET_INT_VALUE_FROM_INISECTION(FindReplaceDlgPosY, CW_USEDEFAULT, INT_MIN, INT_MAX);

  GET_INT_VALUE_FROM_INISECTION(CustomSchemesDlgSizeX, 833, INT_MIN, INT_MAX);
  GET_INT_VALUE_FROM_INISECTION(CustomSchemesDlgSizeY, 515, INT_MIN, INT_MAX);
  GET_INT_VALUE_FROM_INISECTION(CustomSchemesDlgPosX, CW_USEDEFAULT, INT_MIN, INT_MAX);
  GET_INT_VALUE_FROM_INISECTION(CustomSchemesDlgPosY, CW_USEDEFAULT, INT_MIN, INT_MAX);

  // --------------------------------------------------------------------------
  const WCHAR* const StatusBar_Section = L"Statusbar Settings";
  // --------------------------------------------------------------------------

  WCHAR tchStatusBar[MIDSZ_BUFFER] = { L'\0' };

  IniSectionGetString(StatusBar_Section, L"SectionPrefixes", STATUSBAR_SECTION_PREFIXES, tchStatusBar, COUNTOF(tchStatusBar));
  ReadStrgsFromCSV(tchStatusBar, s_mxSBPrefix, STATUS_SECTOR_COUNT, MICRO_BUFFER, L"_PRFX_");

  IniSectionGetString(StatusBar_Section, L"SectionPostfixes", STATUSBAR_SECTION_POSTFIXES, tchStatusBar, COUNTOF(tchStatusBar));
  ReadStrgsFromCSV(tchStatusBar, s_mxSBPostfix, STATUS_SECTOR_COUNT, MICRO_BUFFER, L"_POFX_");

  IniSectionGetString(StatusBar_Section, L"VisibleSections", STATUSBAR_DEFAULT_IDS, tchStatusBar, COUNTOF(tchStatusBar));
  ReadVectorFromString(tchStatusBar, s_iStatusbarSections, STATUS_SECTOR_COUNT, 0, (STATUS_SECTOR_COUNT - 1), -1);

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
  ReadVectorFromString(tchStatusBar, s_iStatusbarWidthSpec, STATUS_SECTOR_COUNT, -4096, 4096, 0);

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
  const WCHAR* const Window_Section = L"Window";
  // --------------------------------------------------------------------------

  int ResX, ResY;
  GetCurrentMonitorResolution(Globals.hwndMain, &ResX, &ResY);

  WCHAR tchHighDpiToolBar[32] = { L'\0' };
  StringCchPrintf(tchHighDpiToolBar, COUNTOF(tchHighDpiToolBar), L"%ix%i HighDpiToolBar", ResX, ResY);
  s_iToolBarTheme = IniSectionGetInt(Window_Section, tchHighDpiToolBar, -1);
  s_iToolBarTheme = clampi(s_iToolBarTheme, -1, StrIsEmpty(s_tchToolbarBitmap) ? 1 : 2);
  if (s_iToolBarTheme < 0) { // undefined: determine high DPI (higher than Full-HD)
    s_iToolBarTheme = IsFullHDOrHigher(Globals.hwndMain, ResX, ResY) ? 1 : 0;
  }

  // --------------------------------------------------------------
  // startup window  (ignore window position if /p was specified)
  // --------------------------------------------------------------

  // 1st set default window position 

  s_DefWinInfo = InitDefaultWndPos(2); // std. default position

  if (bExplicitDefaultWinPos) {
    int bMaxi = 0;
    int const itok = swscanf_s(Settings2.DefaultWindowPosition, L"%i,%i,%i,%i,%i",
      &s_DefWinInfo.x, &s_DefWinInfo.y, &s_DefWinInfo.cx, &s_DefWinInfo.cy, &bMaxi);
    if (itok == 4 || itok == 5) { // scan successful
      if (s_DefWinInfo.cx < 1) s_DefWinInfo.cx = CW_USEDEFAULT;
      if (s_DefWinInfo.cy < 1) s_DefWinInfo.cy = CW_USEDEFAULT;
      if (bMaxi) s_DefWinInfo.max = true;
      if (itok == 4) s_DefWinInfo.max = false;
      InitWindowPosition(&s_DefWinInfo, 0);
    }
    else {
      // overwrite bad defined default position
      StringCchPrintf(Settings2.DefaultWindowPosition, COUNTOF(Settings2.DefaultWindowPosition),
        L"%i,%i,%i,%i,%i", s_DefWinInfo.x, s_DefWinInfo.y, s_DefWinInfo.cx, s_DefWinInfo.cy, s_DefWinInfo.max);
      IniFileSetString(Globals.IniFile, L"Settings2", L"DefaultWindowPosition", Settings2.DefaultWindowPosition);
    }
  }

  // 2nd set initial window position

  if (!s_flagPosParam /*|| g_bStickyWinPos*/) {

    s_WinInfo = s_DefWinInfo;

    WCHAR tchPosX[32], tchPosY[32], tchSizeX[32], tchSizeY[32], tchMaximized[32], tchZoom[32];

    StringCchPrintf(tchPosX, COUNTOF(tchPosX), L"%ix%i PosX", ResX, ResY);
    StringCchPrintf(tchPosY, COUNTOF(tchPosY), L"%ix%i PosY", ResX, ResY);
    StringCchPrintf(tchSizeX, COUNTOF(tchSizeX), L"%ix%i SizeX", ResX, ResY);
    StringCchPrintf(tchSizeY, COUNTOF(tchSizeY), L"%ix%i SizeY", ResX, ResY);
    StringCchPrintf(tchMaximized, COUNTOF(tchMaximized), L"%ix%i Maximized", ResX, ResY);
    StringCchPrintf(tchZoom, COUNTOF(tchZoom), L"%ix%i Zoom", ResX, ResY);

    s_WinInfo.x = IniSectionGetInt(Window_Section, tchPosX, CW_USEDEFAULT);
    s_WinInfo.y = IniSectionGetInt(Window_Section, tchPosY, CW_USEDEFAULT);
    s_WinInfo.cx = IniSectionGetInt(Window_Section, tchSizeX, CW_USEDEFAULT);
    s_WinInfo.cy = IniSectionGetInt(Window_Section, tchSizeY, CW_USEDEFAULT);
    s_WinInfo.max = IniSectionGetBool(Window_Section, tchMaximized, false);
    s_WinInfo.zoom = IniSectionGetInt(Window_Section, tchZoom, (Globals.iCfgVersionRead < CFG_VER_0001) ? 0 : 100);
    if (Globals.iCfgVersionRead < CFG_VER_0001) { s_WinInfo.zoom = (s_WinInfo.zoom + 10) * 10; }
    s_WinInfo.zoom = clampi(s_WinInfo.zoom, SC_MIN_ZOOM_LEVEL, SC_MAX_ZOOM_LEVEL);

    if ((s_WinInfo.x == CW_USEDEFAULT) || (s_WinInfo.y == CW_USEDEFAULT) ||
      (s_WinInfo.cx == CW_USEDEFAULT) || (s_WinInfo.cy == CW_USEDEFAULT))
    {
      s_flagWindowPos = 2; // std. default position (CmdLn: /pd)
    }
    else
      s_flagWindowPos = 0; // init to g_WinInfo
  }

  // ------------------------------------------------------------------------

  // ---  override by resolution specific settings  ---
  WCHAR tchSciDirectWriteTech[64];
  StringCchPrintf(tchSciDirectWriteTech, COUNTOF(tchSciDirectWriteTech), L"%ix%i RenderingTechnology", ResX, ResY);
  Settings.RenderingTechnology = clampi(IniSectionGetInt(Window_Section, tchSciDirectWriteTech, Settings.RenderingTechnology), 0, 3);

  WCHAR tchSciFontQuality[64];
  StringCchPrintf(tchSciFontQuality, COUNTOF(tchSciFontQuality), L"%ix%i SciFontQuality", ResX, ResY);
  Settings2.SciFontQuality = clampi(IniSectionGetInt(Window_Section, tchSciFontQuality, Settings2.SciFontQuality), 0, 3);

  // --------------------------------------------------------------------------
  const WCHAR* const Styles_Section = L"Styles";
  // --------------------------------------------------------------------------
  IniSectionGetString(Styles_Section, Constants.StylingThemeName, L"", Globals.SelectedThemeName, COUNTOF(Globals.SelectedThemeName));


  // define scintilla internal codepage
  const int iSciDefaultCodePage = SC_CP_UTF8; // default UTF8

  // remove internal support for Chinese, Japan, Korean DBCS  use UTF-8 instead
  /*
  if (Settings.DefaultEncoding == CPI_ANSI_DEFAULT)
  {
    // check for Chinese, Japan, Korean DBCS code pages and switch accordingly
    int acp = (int)GetACP();
    if (acp == 932 || acp == 936 || acp == 949 || acp == 950) {
      iSciDefaultCodePage = acp;
    }
    Settings.DefaultEncoding = Encoding_GetByCodePage(iSciDefaultCodePage);
  }
  */

  // set flag for encoding default
  Encoding_SetDefaultFlag(Settings.DefaultEncoding);

  // define default charset
  Globals.iDefaultCharSet = (int)CharSetFromCodePage((UINT)iSciDefaultCodePage);

  // File MRU
  Globals.pFileMRU = MRU_Create(_s_RecentFiles, MRU_NOCASE, MRU_ITEMSFILE);
  MRU_Load(Globals.pFileMRU);

  Globals.pMRUfind = MRU_Create(_s_RecentFind, (/*IsWindowsNT()*/true) ? MRU_UTF8 : 0, MRU_ITEMSFNDRPL);
  MRU_Load(Globals.pMRUfind);
  SetFindPattern(Globals.pMRUfind->pszItems[0]);

  Globals.pMRUreplace = MRU_Create(_s_RecentReplace, (/*IsWindowsNT()*/true) ? MRU_UTF8 : 0, MRU_ITEMSFNDRPL);
  MRU_Load(Globals.pMRUreplace);


  if (bDirtyFlag) {
    SaveIniFile(Globals.IniFile);
  }
  ReleaseIniFile();

  // Scintilla Styles
  Style_Load();
}
//=============================================================================



//=============================================================================
//
//  LoadFlags()
//
//
void LoadFlags()
{
  LoadIniFile(Globals.IniFile);

  const WCHAR* const Settings2_Section = L"Settings2";

  Flags.bDevDebugMode = IniSectionGetBool(Settings2_Section, L"DevDebugMode", DefaultFlags.bDevDebugMode);
  Flags.bStickyWindowPosition = IniSectionGetBool(Settings2_Section, L"StickyWindowPosition", DefaultFlags.bStickyWindowPosition);

  if (s_flagReuseWindow == 0) {
    Flags.bReuseWindow = IniSectionGetBool(Settings2_Section, L"ReuseWindow", DefaultFlags.bReuseWindow);
  }
  else {
    Flags.bReuseWindow = (s_flagReuseWindow == 2);
  }

  if (s_flagSingleFileInstance == 0) {
    Flags.bSingleFileInstance = IniSectionGetBool(Settings2_Section, L"SingleFileInstance", DefaultFlags.bSingleFileInstance);
  }
  else {
    Flags.bSingleFileInstance = (s_flagSingleFileInstance == 2);
  }

  if (s_flagMultiFileArg == 0) {
    Flags.MultiFileArg = IniSectionGetBool(Settings2_Section, L"MultiFileArg", DefaultFlags.MultiFileArg);
  }
  else {
    Flags.MultiFileArg = (s_flagMultiFileArg == 2);
  }

  if (s_flagShellUseSystemMRU == 0) {
    Flags.ShellUseSystemMRU = IniSectionGetBool(Settings2_Section, L"ShellUseSystemMRU", DefaultFlags.ShellUseSystemMRU);
  }
  else {
    Flags.ShellUseSystemMRU = (s_flagShellUseSystemMRU == 2);
  }

  Flags.RelativeFileMRU = IniSectionGetBool(Settings2_Section, L"RelativeFileMRU", DefaultFlags.RelativeFileMRU);
  Flags.PortableMyDocs = IniSectionGetBool(Settings2_Section, L"PortableMyDocs", DefaultFlags.PortableMyDocs);
  Flags.NoFadeHidden = IniSectionGetBool(Settings2_Section, L"NoFadeHidden", DefaultFlags.NoFadeHidden);

  Flags.ToolbarLook = IniSectionGetInt(Settings2_Section, L"ToolbarLook", DefaultFlags.ToolbarLook);
  Flags.ToolbarLook = clampi(Flags.ToolbarLook, 0, 2);

  Flags.SimpleIndentGuides = IniSectionGetBool(Settings2_Section, L"SimpleIndentGuides", DefaultFlags.SimpleIndentGuides);
  Flags.NoHTMLGuess = IniSectionGetBool(Settings2_Section, L"NoHTMLGuess", DefaultFlags.NoHTMLGuess);
  Flags.NoCGIGuess = IniSectionGetBool(Settings2_Section, L"NoCGIGuess", DefaultFlags.NoCGIGuess);
  Flags.NoFileVariables = IniSectionGetInt(Settings2_Section, L"NoFileVariables", DefaultFlags.NoFileVariables);

  Flags.PrintFileAndLeave = s_flagPrintFileAndLeave;
  Flags.bPreserveFileModTime = false;

  ReleaseIniFile();
}



//=============================================================================
//
//  OpenSettingsFile()
//

static bool s_bSettingsIniFileOpend = false;

bool OpenSettingsFile()
{
  if (StrIsNotEmpty(Globals.IniFile)) {
    if (!s_bSettingsIniFileOpend) {
      CreateIniFile();
      s_bSettingsIniFileOpend = LoadIniFile(Globals.IniFile);
    }
  }
  else {
    s_bSettingsIniFileOpend = false;
  }
  return s_bSettingsIniFileOpend;
}


//=============================================================================
//
//  SaveSettings()
//

#define SAVE_VALUE_IF_NOT_EQ_DEFAULT(TYPE, VARNAME)                               \
  if (Settings.VARNAME != Defaults.VARNAME) {                                     \
    IniSectionSet##TYPE(Settings_Section, _W(_STRG(VARNAME)), Settings.VARNAME);  \
  }                                                                               \
  else {                                                                          \
    IniSectionDelete(Settings_Section, _W(_STRG(VARNAME)), false);     \
  }


bool SaveSettings(bool bSaveSettingsNow)
{
  // update window placement 
  s_WinInfo = GetMyWindowPlacement(Globals.hwndMain, NULL);

  if (!s_bSettingsIniFileOpend) { return false; }

  const WCHAR* const Settings_Section = L"Settings";

  if (!Settings.SaveSettings && !bSaveSettingsNow)
  {
    if (Settings.SaveSettings != Defaults.SaveSettings) {
      IniSectionSetBool(Settings_Section, L"SaveSettings", Settings.SaveSettings);
    }
    else {
      IniSectionDelete(Settings_Section, L"SaveSettings", false);
    }
    return true;
  }

  IniSectionSetInt(Settings_Section, L"SettingsVersion", CFG_VER_CURRENT);  // new settings

  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, SaveSettings);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, SaveRecentFiles);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, PreserveCaretPos);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, SaveFindReplace);

  if (Settings.EFR_Data.bFindClose != Defaults.EFR_Data.bFindClose) {
    IniSectionSetBool(Settings_Section, L"CloseFind", Settings.EFR_Data.bFindClose);
  }
  else {
    IniSectionDelete(Settings_Section, L"CloseFind", false);
  }
  if (Settings.EFR_Data.bReplaceClose != Defaults.EFR_Data.bReplaceClose) {
    IniSectionSetBool(Settings_Section, L"CloseReplace", Settings.EFR_Data.bReplaceClose);
  }
  else {
    IniSectionDelete(Settings_Section, L"CloseReplace", false);
  }
  if (Settings.EFR_Data.bNoFindWrap != Defaults.EFR_Data.bNoFindWrap) {
    IniSectionSetBool(Settings_Section, L"NoFindWrap", Settings.EFR_Data.bNoFindWrap);
  }
  else {
    IniSectionDelete(Settings_Section, L"NoFindWrap", false);
  }
  if (Settings.EFR_Data.bTransformBS != Defaults.EFR_Data.bTransformBS) {
    IniSectionSetBool(Settings_Section, L"FindTransformBS", Settings.EFR_Data.bTransformBS);
  }
  else {
    IniSectionDelete(Settings_Section, L"FindTransformBS", false);
  }
  if (Settings.EFR_Data.bWildcardSearch != Defaults.EFR_Data.bWildcardSearch) {
    IniSectionSetBool(Settings_Section, L"WildcardSearch", Settings.EFR_Data.bWildcardSearch);
  }
  else {
    IniSectionDelete(Settings_Section, L"WildcardSearch", false);
  }
  if (Settings.EFR_Data.bMarkOccurences != Defaults.EFR_Data.bMarkOccurences) {
    IniSectionSetBool(Settings_Section, L"FindMarkAllOccurrences", Settings.EFR_Data.bMarkOccurences);
  }
  else {
    IniSectionDelete(Settings_Section, L"FindMarkAllOccurrences", false);
  }
  if (Settings.EFR_Data.bHideNonMatchedLines != Defaults.EFR_Data.bHideNonMatchedLines) {
    IniSectionSetBool(Settings_Section, L"HideNonMatchedLines", Settings.EFR_Data.bHideNonMatchedLines);
  }
  else {
    IniSectionDelete(Settings_Section, L"HideNonMatchedLines", false);
  }
  if (Settings.EFR_Data.bDotMatchAll != Defaults.EFR_Data.bDotMatchAll) {
    IniSectionSetBool(Settings_Section, L"RegexDotMatchesAll", Settings.EFR_Data.bDotMatchAll);
  }
  else {
    IniSectionDelete(Settings_Section, L"RegexDotMatchesAll", false);
  }
  if (Settings.EFR_Data.fuFlags != Defaults.EFR_Data.fuFlags) {
    IniSectionSetInt(Settings_Section, L"efrData_fuFlags", Settings.EFR_Data.fuFlags);
  }
  else {
    IniSectionDelete(Settings_Section, L"efrData_fuFlags", false);
  }

  WCHAR wchTmp[MAX_PATH] = { L'\0' };
  if (StringCchCompareXIW(Settings.OpenWithDir, Defaults.OpenWithDir) != 0) {
    PathRelativeToApp(Settings.OpenWithDir, wchTmp, COUNTOF(wchTmp), false, true, Flags.PortableMyDocs);
    IniSectionSetString(Settings_Section, L"OpenWithDir", wchTmp);
  }
  else {
    IniSectionDelete(Settings_Section, L"OpenWithDir", false);
  }
  if (StringCchCompareXIW(Settings.FavoritesDir, Defaults.FavoritesDir) != 0) {
    PathRelativeToApp(Settings.FavoritesDir, wchTmp, COUNTOF(wchTmp), false, true, Flags.PortableMyDocs);
    IniSectionSetString(Settings_Section, L"Favorites", wchTmp);
  }
  else {
    IniSectionDelete(Settings_Section, L"Favorites", false);
  }

  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, PathNameFormat);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, WordWrap);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, TabsAsSpaces);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, TabIndents);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, TabWidth);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, IndentWidth);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, LongLinesLimit);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, BackspaceUnindents);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, WordWrapMode);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, WordWrapIndent);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, WordWrapSymbols);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ShowWordWrapSymbols);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MatchBraces);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, AutoCloseTags);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, HighlightCurrentLine);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, HyperlinkHotspot);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ColorDefHotspot);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ScrollPastEOF);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ShowHypLnkToolTip);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, AutoIndent);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, AutoCompleteWords);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, AutoCLexerKeyWords);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, AccelWordNavigation);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ShowIndentGuides);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, WarnInconsistentIndents);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, AutoDetectIndentSettings);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MarkLongLines);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, LongLineMode);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ShowSelectionMargin);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ShowLineNumbers);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ShowCodeFolding);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MarkOccurrences);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MarkOccurrencesMatchVisible);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MarkOccurrencesMatchCase);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MarkOccurrencesMatchWholeWords);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MarkOccurrencesCurrentWord);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ViewWhiteSpace);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ViewEOLs);

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
    IniSectionSetInt(Settings_Section, L"PrintMarginLeft", Settings.PrintMargin.left);
  }
  else {
    IniSectionDelete(Settings_Section, L"PrintMarginLeft", false);
  }
  if (Settings.PrintMargin.top != Defaults.PrintMargin.top) {
    IniSectionSetInt(Settings_Section, L"PrintMarginTop", Settings.PrintMargin.top);
  }
  else {
    IniSectionDelete(Settings_Section, L"PrintMarginTop", false);
  }
  if (Settings.PrintMargin.right != Defaults.PrintMargin.right) {
    IniSectionSetInt(Settings_Section, L"PrintMarginRight", Settings.PrintMargin.right);
  }
  else {
    IniSectionDelete(Settings_Section, L"PrintMarginRight", false);
  }
  if (Settings.PrintMargin.bottom != Defaults.PrintMargin.bottom) {
    IniSectionSetInt(Settings_Section, L"PrintMarginBottom", Settings.PrintMargin.bottom);
  }
  else {
    IniSectionDelete(Settings_Section, L"PrintMarginBottom", false);
  }
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, SaveBeforeRunningTools);
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

  ///~IniSectionSetInt(Settings_Section, L"IMEInteraction", Settings2.IMEInteraction);

  Toolbar_GetButtons(Globals.hwndToolbar, IDT_FILE_NEW, Settings.ToolbarButtons, COUNTOF(Settings.ToolbarButtons));
  if (StringCchCompareX(Settings.ToolbarButtons, Defaults.ToolbarButtons) != 0) {
    IniSectionSetString(Settings_Section, L"ToolbarButtons", Settings.ToolbarButtons);
  }
  else {
    IniSectionDelete(Settings_Section, L"ToolbarButtons", false);
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

  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, FindReplaceDlgSizeX);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, FindReplaceDlgPosX);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, FindReplaceDlgPosY);

  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, CustomSchemesDlgSizeX);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, CustomSchemesDlgSizeY);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, CustomSchemesDlgPosX);
  SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, CustomSchemesDlgPosY);

  int ResX, ResY;
  GetCurrentMonitorResolution(Globals.hwndMain, &ResX, &ResY);

  const WCHAR* const Window_Section = L"Window";

  WCHAR tchHighDpiToolBar[32];
  StringCchPrintf(tchHighDpiToolBar, COUNTOF(tchHighDpiToolBar), L"%ix%i HighDpiToolBar", ResX, ResY);
  IniSectionSetInt(Window_Section, tchHighDpiToolBar, s_iToolBarTheme);

  WCHAR tchPosX[32], tchPosY[32], tchSizeX[32], tchSizeY[32], tchMaximized[32], tchZoom[32];
  StringCchPrintf(tchPosX, COUNTOF(tchPosX), L"%ix%i PosX", ResX, ResY);
  StringCchPrintf(tchPosY, COUNTOF(tchPosY), L"%ix%i PosY", ResX, ResY);
  StringCchPrintf(tchSizeX, COUNTOF(tchSizeX), L"%ix%i SizeX", ResX, ResY);
  StringCchPrintf(tchSizeY, COUNTOF(tchSizeY), L"%ix%i SizeY", ResX, ResY);
  StringCchPrintf(tchMaximized, COUNTOF(tchMaximized), L"%ix%i Maximized", ResX, ResY);
  StringCchPrintf(tchZoom, COUNTOF(tchMaximized), L"%ix%i Zoom", ResX, ResY);

  if (Flags.bStickyWindowPosition)
  {
    IniSectionDelete(Window_Section, tchPosX, false);
    IniSectionDelete(Window_Section, tchPosY, false);
    IniSectionDelete(Window_Section, tchSizeX, false);
    IniSectionDelete(Window_Section, tchSizeY, false);
    IniSectionDelete(Window_Section, tchMaximized, false);
    IniSectionDelete(Window_Section, tchZoom, false);
  }
  else {
    IniSectionSetInt(Window_Section, tchPosX, s_WinInfo.x);
    IniSectionSetInt(Window_Section, tchPosY, s_WinInfo.y);
    IniSectionSetInt(Window_Section, tchSizeX, s_WinInfo.cx);
    IniSectionSetInt(Window_Section, tchSizeY, s_WinInfo.cy);
    IniSectionSetBool(Window_Section, tchMaximized, s_WinInfo.max);
    IniSectionSetInt(Window_Section, tchZoom, s_WinInfo.zoom);
  }


  const WCHAR* const Styles_Section = L"Styles";

  switch (Globals.idxSelectedTheme) {
    case 1: 
      Style_ToIniSection(false); // Scintilla Styles
      // falltrough
    case 0:
      IniSectionDelete(Styles_Section, Constants.StylingThemeName, false);
      break;
    default:
      IniSectionSetString(Styles_Section, Constants.StylingThemeName, Theme_Files[Globals.idxSelectedTheme].szName);
      break;
  }

  return true;
}


//=============================================================================
//
//  CloseSettingsFile()
//

bool CloseSettingsFile()
{
  if (!s_bSettingsIniFileOpend || StrIsEmpty(Globals.IniFile)) { return false; }

  bool const ok = SaveIniFile(Globals.IniFile);

  if (ok) {
    Globals.bIniFileFromScratch = false;
  }

  // force filesystem cache to sync
  LoadIniFile(Globals.IniFile);
  ReleaseIniFile();

  s_bSettingsIniFileOpend = false;
  return ok;
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


bool MRU_Add(LPMRULIST pmru, LPCWSTR pszNew, cpi_enc_t iEnc, DocPos iPos, LPCWSTR pszBookMarks)
{
  if (pmru) {
    int i = 0;
    for (; i < pmru->iSize; i++) {
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
    }
    pmru->pszItems[0] = StrDup(pszNew); // LocalAlloc()

    pmru->iEncoding[0] = iEnc;
    pmru->iCaretPos[0] = (Settings.PreserveCaretPos ? iPos : 0);
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
    for (i = 0; i < pmru->iSize; i++) {
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


bool MRU_AddFile(LPMRULIST pmru, LPCWSTR pszFile, bool bRelativePath, bool bUnexpandMyDocs,
  cpi_enc_t iEnc, DocPos iPos, LPCWSTR pszBookMarks) 
{
  if (pmru) {
    int i = 0;
    if (MRU_FindFile(pmru, pszFile, &i)) {
      LocalFree(pmru->pszItems[i]);  // StrDup()
    }
    else {
      i = (i < pmru->iSize) ? i : (pmru->iSize - 1);
    }
    for (; i > 0; i--) {
      pmru->pszItems[i] = pmru->pszItems[i - 1];
      pmru->iEncoding[i] = pmru->iEncoding[i - 1];
      pmru->iCaretPos[i] = pmru->iCaretPos[i - 1];
      pmru->pszBookMarks[i] = pmru->pszBookMarks[i - 1];
    }
    if (bRelativePath) {
      WCHAR wchFile[MAX_PATH] = { L'\0' };
      PathRelativeToApp((LPWSTR)pszFile, wchFile, COUNTOF(wchFile), true, true, bUnexpandMyDocs);
      pmru->pszItems[0] = StrDup(wchFile);  // LocalAlloc()
    }
    else {
      pmru->pszItems[0] = StrDup(pszFile);  // LocalAlloc()
    }
    pmru->iEncoding[0] = iEnc;
    pmru->iCaretPos[0] = (Settings.PreserveCaretPos ? iPos : 0);
    pmru->pszBookMarks[0] = (pszBookMarks ? StrDup(pszBookMarks) : NULL);  // LocalAlloc()
    return true;
  }
  return false;
}


bool MRU_Delete(LPMRULIST pmru, int iIndex) 
{
  if (pmru) {
    int i;
    if (iIndex < 0 || iIndex > pmru->iSize - 1) {
      return false;
    }
    if (pmru->pszItems[iIndex]) {
      LocalFree(pmru->pszItems[iIndex]);  // StrDup()
    }
    if (pmru->pszBookMarks[iIndex]) {
      LocalFree(pmru->pszBookMarks[iIndex]);  // StrDup()
    }
    for (i = iIndex; i < pmru->iSize - 1; i++) {
      pmru->pszItems[i] = pmru->pszItems[i + 1];
      pmru->iEncoding[i] = pmru->iEncoding[i + 1];
      pmru->iCaretPos[i] = pmru->iCaretPos[i + 1];
      pmru->pszBookMarks[i] = pmru->pszBookMarks[i + 1];

      pmru->pszItems[i + 1] = NULL;
      pmru->iEncoding[i + 1] = 0;
      pmru->iCaretPos[i + 1] = 0;
      pmru->pszBookMarks[i + 1] = NULL;
    }
    return true;
  }
  return false;
}


bool MRU_Empty(LPMRULIST pmru)
{
  if (pmru) {
    for (int i = 0; i < pmru->iSize; i++) {
      if (pmru->pszItems[i]) {
        LocalFree(pmru->pszItems[i]);  // StrDup()
        pmru->pszItems[i] = NULL;
        pmru->iEncoding[i] = 0;
        pmru->iCaretPos[i] = 0;
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


bool MRU_Load(LPMRULIST pmru)
{
  if (pmru) 
  {
    MRU_Empty(pmru);

    const WCHAR* const RegKey_Section = pmru->szRegKey;

    int n = 0;
    for (int i = 0; i < pmru->iSize; i++) {
      WCHAR tchName[32] = { L'\0' };
      StringCchPrintf(tchName, COUNTOF(tchName), L"%.2i", i + 1);
      WCHAR tchItem[2048] = { L'\0' };
      if (IniSectionGetString(RegKey_Section, tchName, L"", tchItem, COUNTOF(tchItem))) {
        size_t const len = StringCchLen(tchItem, 0);
        if ((len > 0) && (tchItem[0] == L'"') && (tchItem[len - 1] == L'"')) {
          MoveMemory(tchItem, (tchItem + 1), len * sizeof(WCHAR));
          tchItem[len - 2] = L'\0'; // clear dangling '"'
        }
        pmru->pszItems[n] = StrDup(tchItem);

        StringCchPrintf(tchName, COUNTOF(tchName), L"ENC%.2i", i + 1);
        int const iCP = (cpi_enc_t)IniSectionGetInt(RegKey_Section, tchName, 0);
        pmru->iEncoding[n] = (cpi_enc_t)Encoding_MapIniSetting(true, iCP);

        StringCchPrintf(tchName, COUNTOF(tchName), L"POS%.2i", i + 1);
        pmru->iCaretPos[n] = (Settings.PreserveCaretPos) ? IniSectionGetInt(RegKey_Section, tchName, 0) : 0;

        StringCchPrintf(tchName, COUNTOF(tchName), L"BMRK%.2i", i + 1);

        WCHAR wchBookMarks[MRU_BMRK_SIZE] = { L'\0' };
        IniSectionGetString(RegKey_Section, tchName, L"", wchBookMarks, COUNTOF(wchBookMarks));
        pmru->pszBookMarks[n] = StrDup(wchBookMarks);

        ++n;
      }
    }
    return true;
  }
  return false;
}


bool MRU_Save(LPMRULIST pmru)
{
  if (s_bSettingsIniFileOpend) {
    if (pmru) {
      WCHAR tchName[32] = { L'\0' };
      WCHAR tchItem[2048] = { L'\0' };

      const WCHAR* const RegKey_Section = pmru->szRegKey;
      IniSectionClear(pmru->szRegKey, false);

      for (int i = 0; i < pmru->iSize; i++) {
        if (pmru->pszItems[i]) {
          StringCchPrintf(tchName, COUNTOF(tchName), L"%.2i", i + 1);
          StringCchPrintf(tchItem, COUNTOF(tchItem), L"\"%s\"", pmru->pszItems[i]);
          IniSectionSetString(RegKey_Section, tchName, tchItem);

          if (pmru->iEncoding[i] > 0) {
            StringCchPrintf(tchName, COUNTOF(tchName), L"ENC%.2i", i + 1);
            int const iCP = (int)Encoding_MapIniSetting(false, (int)pmru->iEncoding[i]);
            IniSectionSetInt(RegKey_Section, tchName, iCP);
          }
          if (pmru->iCaretPos[i] > 0) {
            StringCchPrintf(tchName, COUNTOF(tchName), L"POS%.2i", i + 1);
            IniSectionSetPos(RegKey_Section, tchName, pmru->iCaretPos[i]);
          }
          if (pmru->pszBookMarks[i] && (StringCchLenW(pmru->pszBookMarks[i], MRU_BMRK_SIZE) > 0)) {
            StringCchPrintf(tchName, COUNTOF(tchName), L"BMRK%.2i", i + 1);
            IniSectionSetString(RegKey_Section, tchName, pmru->pszBookMarks[i]);
          }
        }
      }
    }
  }
  return s_bSettingsIniFileOpend;
}


bool MRU_MergeSave(LPMRULIST pmru, bool bAddFiles, bool bRelativePath, bool bUnexpandMyDocs)
{
  if (pmru) {
    int i;
    LPMRULIST pmruBase;

    pmruBase = MRU_Create(pmru->szRegKey, pmru->iFlags, pmru->iSize);
    MRU_Load(pmruBase);

    if (bAddFiles) {
      for (i = pmru->iSize - 1; i >= 0; i--) {
        if (pmru->pszItems[i]) {
          WCHAR wchItem[MAX_PATH] = { L'\0' };
          PathAbsoluteFromApp(pmru->pszItems[i], wchItem, COUNTOF(wchItem), true);
          MRU_AddFile(pmruBase, wchItem, bRelativePath, bUnexpandMyDocs,
            pmru->iEncoding[i], pmru->iCaretPos[i], pmru->pszBookMarks[i]);
        }
      }
    }
    else {
      for (i = pmru->iSize - 1; i >= 0; i--) {
        if (pmru->pszItems[i])
          MRU_Add(pmruBase, pmru->pszItems[i],
            pmru->iEncoding[i], pmru->iCaretPos[i], pmru->pszBookMarks[i]);
      }
    }

    MRU_Save(pmruBase);
    MRU_Destroy(pmruBase);
    pmruBase = NULL;
    return true;
  }
  return false;
}

