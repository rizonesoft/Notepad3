// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* MuiLanguage.c                                                               *
*   General MUI Language support functions                                    *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2020   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

#include "Helpers.h"

#include <muiload.h>
#include <locale.h>

#include "resource.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Config/Config.h"
#include "MuiLanguage.h"

//=============================================================================

extern prefix_t  g_mxSBPrefix[STATUS_SECTOR_COUNT];
extern prefix_t  g_mxSBPostfix[STATUS_SECTOR_COUNT];

//=============================================================================

MUILANGUAGE MUI_LanguageDLLs[] =
{
  { IDS_MUI_LANG_EN_US, L"en-US", L"English (United States)\t\t\t[%s]",  MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), true, false }, // internal - must be 1st
  // ----------------------------
  { IDS_MUI_LANG_AF_ZA, L"af-ZA", L"Afrikaans (Suid-Afrika)\t\t\t[%s]",  MAKELANGID(LANG_AFRIKAANS, SUBLANG_AFRIKAANS_SOUTH_AFRICA), false, false },
  { IDS_MUI_LANG_BE_BY, L"be-BY", L"Беларуская (Беларусь)\t\t\t[%s]",    MAKELANGID(LANG_BELARUSIAN, SUBLANG_BELARUSIAN_BELARUS), false, false },
  { IDS_MUI_LANG_DE_DE, L"de-DE", L"Deutsch (Deutschland)\t\t\t[%s]",    MAKELANGID(LANG_GERMAN, SUBLANG_GERMAN), false, false },
  { IDS_MUI_LANG_EN_GB, L"en-GB", L"English (United Kingdom)\t\t\t[%s]", MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_UK), false, false },
  { IDS_MUI_LANG_ES_ES, L"es-ES", L"Español (España)\t\t\t[%s]",         MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_MODERN), false, false },
  { IDS_MUI_LANG_FR_FR, L"fr-FR", L"Français (France)\t\t\t[%s]",        MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH), false, false },
  { IDS_MUI_LANG_HU_HU, L"hu-HU", L"Magyar (Magyarország)\t\t\t[%s]",    MAKELANGID(LANG_HUNGARIAN, SUBLANG_HUNGARIAN_HUNGARY), false, false },
  { IDS_MUI_LANG_IT_IT, L"it-IT", L"Italiano (Italia)\t\t\t[%s]",        MAKELANGID(LANG_ITALIAN, SUBLANG_ITALIAN), false, false },
  { IDS_MUI_LANG_JP_JP, L"ja-JP", L"日本語 （日本）\t\t\t[%s]",           MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN), false, false },
  { IDS_MUI_LANG_KO_KR, L"ko-KR", L"한국어 (대한민국)\t\t\t[%s]",         MAKELANGID(LANG_KOREAN, SUBLANG_KOREAN), false, false },
  { IDS_MUI_LANG_NL_NL, L"nl-NL", L"Nederlands (Nederland)\t\t\t[%s]",   MAKELANGID(LANG_DUTCH, SUBLANG_DUTCH), false, false },
  { IDS_MUI_LANG_PL_PL, L"pl-PL", L"Polski (Polska)\t\t\t[%s]",          MAKELANGID(LANG_POLISH, SUBLANG_POLISH_POLAND), false, false },
  { IDS_MUI_LANG_PT_BR, L"pt-BR", L"Português (Brasil)\t\t\t[%s]",       MAKELANGID(LANG_PORTUGUESE, SUBLANG_PORTUGUESE_BRAZILIAN), false, false },
  { IDS_MUI_LANG_RU_RU, L"ru-RU", L"Русский (Pоссия)\t\t\t[%s]",         MAKELANGID(LANG_RUSSIAN, SUBLANG_RUSSIAN_RUSSIA), false, false },
  { IDS_MUI_LANG_SK_SK, L"sk-SK", L"Slovenčina (Slovensko)\t\t\t[%s]",   MAKELANGID(LANG_SLOVAK, SUBLANG_SLOVAK_SLOVAKIA), false, false },
  { IDS_MUI_LANG_SV_SE, L"sv-SE", L"Svenska (Sverige)\t\t\t[%s]",        MAKELANGID(LANG_SWEDISH, SUBLANG_SWEDISH), false, false },
  { IDS_MUI_LANG_TR_TR, L"tr-TR", L"Türkçe (Türkiye)\t\t\t[%s]",         MAKELANGID(LANG_TURKISH, SUBLANG_TURKISH_TURKEY), false, false },
  { IDS_MUI_LANG_ZH_CN, L"zh-CN", L"简体中文 （中国）\t\t\t[%s]",         MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED), false, false}
};

//NUM_OF_MUI_LANGUAGES
int MuiLanguages_CountOf() { return COUNTOF(MUI_LanguageDLLs); };

//=============================================================================
//
//  CheckAvailableLanguages
//
//
static int _CheckAvailableLanguageDLLs()
{
  WCHAR wchRelPath[MAX_PATH];
  WCHAR wchAbsPath[MAX_PATH];

  int count = 1;
  for (int lng = 1; lng < MuiLanguages_CountOf(); ++lng)
  {
    if (IsValidLocaleName(MUI_LanguageDLLs[lng].szLocaleName))
    {
      //WCHAR wchLngLocalName[LOCALE_NAME_MAX_LENGTH];
      //if (ResolveLocaleName(MUI_LanguageDLLs[i].szLocaleName, wchLngLocalName, LOCALE_NAME_MAX_LENGTH)) {
      //  StringCchCopy(MUI_LanguageDLLs[i].szLocaleName, COUNTOF(MUI_LanguageDLLs[i].szLocaleName), wchLngLocalName); // put back resolved name
      //}
      
      // get LANGID
      GetLocaleInfoEx(MUI_LanguageDLLs[lng].szLocaleName, LOCALE_ILANGUAGE | LOCALE_RETURN_NUMBER, 
        (LPWSTR)&(MUI_LanguageDLLs[lng].LangId), sizeof(LANGID));

      // check for DLL
      StringCchPrintf(wchRelPath, COUNTOF(wchRelPath), L"lng/%s/np3lng.dll.mui", MUI_LanguageDLLs[lng].szLocaleName);
      PathAbsoluteFromApp(wchRelPath, wchAbsPath, COUNTOF(wchAbsPath), false);
      bool const bAvail = PathFileExists(wchAbsPath);
      MUI_LanguageDLLs[lng].bHasDLL = bAvail;
      count += bAvail ? 1 : 0;
    }
  }
  return count;
}


//=============================================================================
//
//  _LngStrToMultiLngStr
//
//
static bool  _LngStrToMultiLngStr(WCHAR* pLngStr, WCHAR* pLngMultiStr, size_t lngMultiStrSize)
{
  bool rtnVal = true;

  size_t strLen = StringCchLenW(pLngStr, 0);

  if ((strLen > 0) && pLngMultiStr && (lngMultiStrSize > 0)) {
    WCHAR* lngMultiStrPtr = pLngMultiStr;
    WCHAR* last = pLngStr + (Has_UTF16_LE_BOM((char*)pLngStr, (strLen * sizeof(WCHAR))) ? 1 : 0);
    while (last && rtnVal) {
      // make sure you validate the user input
      WCHAR* next = StrNextTok(last, L",; :");
      if (next) { *next = L'\0'; }
      strLen = StringCchLenW(last, LOCALE_NAME_MAX_LENGTH);
      if (strLen && IsValidLocaleName(last)) {
        lngMultiStrPtr[0] = L'\0';
        rtnVal &= SUCCEEDED(StringCchCatW(lngMultiStrPtr, (lngMultiStrSize - (lngMultiStrPtr - pLngMultiStr)), last));
        lngMultiStrPtr += strLen + 1;
      }
      last = (next ? next + 1 : next);
    }
    if (rtnVal && (lngMultiStrSize - (lngMultiStrPtr - pLngMultiStr))) // make sure there is a double null term for the multi-string
    {
      lngMultiStrPtr[0] = L'\0';
    }
    else // fail and guard anyone whom might use the multi-string
    {
      lngMultiStrPtr[0] = L'\0';
      lngMultiStrPtr[1] = L'\0';
    }
  }
  return rtnVal;
}



//=============================================================================
//
//  _GetUserPreferredLanguage
//
//
static bool _GetUserPreferredLanguage(LPWSTR pszPrefLocaleName, int cchBuffer, LANGID* pLangID)
{
  int res = 0;
  LANGID lngID = *pLangID;
  WCHAR wchLngLocalName[LOCALE_NAME_MAX_LENGTH+1];

  if (StrIsNotEmpty(pszPrefLocaleName))
  {
    res = ResolveLocaleName(pszPrefLocaleName, wchLngLocalName, COUNTOF(wchLngLocalName));
    if (res > 0) {
      // get LANGID
      DWORD value;
      res = GetLocaleInfoEx(wchLngLocalName, LOCALE_ILANGUAGE | LOCALE_RETURN_NUMBER, (LPWSTR)&value, sizeof(value) / sizeof(WCHAR));
      if (res > 0) {
        lngID = (LANGID)value;
      }
    }
  }

  if (res == 0) // No preferred language defined or retrievable, try to get User UI Language
  {
    //~GetUserDefaultLocaleName(pszPrefLocaleName, cchBuffer);
    ULONG numLngs = 0;
    DWORD cchLngsBuffer = 0;
    BOOL hr = GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &numLngs, NULL, &cchLngsBuffer);
    if (hr) {
      WCHAR* pwszLngsBuffer = AllocMem((cchLngsBuffer + 2) * sizeof(WCHAR), HEAP_ZERO_MEMORY);
      if (pwszLngsBuffer) {
        hr = GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &numLngs, pwszLngsBuffer, &cchLngsBuffer);
        if (hr && (numLngs > 0)) {
          // get the first 
          StringCchCopy(wchLngLocalName, COUNTOF(wchLngLocalName), pwszLngsBuffer);
          lngID = LANGIDFROMLCID(LocaleNameToLCID(wchLngLocalName, 0));
          res = 1;
        }
        FreeMem(pwszLngsBuffer);
      }
    }
    if (res == 0) { // last try
      lngID = GetUserDefaultUILanguage();
      LCID const lcid = MAKELCID(lngID, SORT_DEFAULT);
      res = LCIDToLocaleName(lcid, wchLngLocalName, COUNTOF(wchLngLocalName), 0);
    }
  }
  if (res != 0) {
    *pLangID = lngID;
    StringCchCopy(pszPrefLocaleName, cchBuffer, wchLngLocalName);
    return true;
  }
  return false;
}



//=============================================================================
//
//  LoadLanguageResources
//
//
LANGID LoadLanguageResources()
{
  // 1st check language resources
  Globals.iAvailLngCount = _CheckAvailableLanguageDLLs();

  LANGID languageID = MUI_LanguageDLLs[0].LangId;

  WCHAR wchLngLocalName[LOCALE_NAME_MAX_LENGTH];
  StringCchCopy(wchLngLocalName, COUNTOF(wchLngLocalName), Settings2.PreferredLanguageLocaleName);

  if (_GetUserPreferredLanguage(wchLngLocalName, COUNTOF(wchLngLocalName), &languageID)) {
    // push-back (corrected) name found
    StringCchCopy(Settings2.PreferredLanguageLocaleName, COUNTOF(Settings2.PreferredLanguageLocaleName), wchLngLocalName);
    //_wsetlocale(LC_COLLATE, Settings2.PreferredLanguageLocaleName);
  }

  // set the appropriate fallback list
  int iPrefLngIndex = -1;
  WCHAR tchAvailLngs[2 * (LOCALE_NAME_MAX_LENGTH + 1)] = { L'\0' };
  for (int lng = 0; lng < MuiLanguages_CountOf(); ++lng) {
    if (StringCchCompareXIW(MUI_LanguageDLLs[lng].szLocaleName, Settings2.PreferredLanguageLocaleName) == 0) {
      if (MUI_LanguageDLLs[lng].bHasDLL && (lng > 0)) {
        StringCchCatW(tchAvailLngs, COUNTOF(tchAvailLngs), MUI_LanguageDLLs[lng].szLocaleName);
        StringCchCatW(tchAvailLngs, COUNTOF(tchAvailLngs), L" ");
      }
      iPrefLngIndex = lng;
      break;
    }
  }
  StringCchCatW(tchAvailLngs, COUNTOF(tchAvailLngs), MUI_LanguageDLLs[0].szLocaleName); // en-US

  // NOTES:
  // an application developer that makes the assumption the fallback list provided by the
  // system / OS is entirely sufficient may or may not be making a good assumption based  mostly on:
  // A. your choice of languages installed with your application
  // B. the languages on the OS at application install time
  // C. the OS users propensity to install/uninstall language packs
  // D. the OS users propensity to change language settings

  WCHAR tchUserLangMultiStrg[LARGE_BUFFER] = { L'\0' };
  if (!_LngStrToMultiLngStr(tchAvailLngs, tchUserLangMultiStrg, COUNTOF(tchUserLangMultiStrg)))
  {
    MsgBoxLastError(L"Trying to load Language resource!", ERROR_MUI_INVALID_LOCALE_NAME);
  }
  DWORD langCount = 0;
  // using SetProcessPreferredUILanguages is recommended for new applications (esp. multi-threaded applications)
  if (!SetThreadPreferredUILanguages(MUI_LANGUAGE_NAME, tchUserLangMultiStrg, &langCount) || (langCount == 0))
  {
    MsgBoxLastError(L"Trying to set preferred Language!", ERROR_RESOURCE_LANG_NOT_FOUND);
  }

  // obtains access to the proper resource container 
  // for standard Win32 resource loading this is normally a PE module - use LoadLibraryEx

  HINSTANCE _hLangResourceContainer = NULL;
  Globals.bPrefLngNotAvail = (iPrefLngIndex < 0);

  if ((iPrefLngIndex >= 0) && MUI_LanguageDLLs[iPrefLngIndex].bHasDLL) {
    _hLangResourceContainer = (iPrefLngIndex == 0) ? Globals.hInstance :
      LoadMUILibrary(L"lng/np3lng.dll", MUI_LANGUAGE_NAME | MUI_LANGUAGE_EXACT, languageID);
    if (_hLangResourceContainer) {
      MUI_LanguageDLLs[0].bIsActive = false;
      MUI_LanguageDLLs[iPrefLngIndex].bIsActive = true;
    }
  }
  if (!_hLangResourceContainer) {
    // fallback to ENGLISH_US
    //MsgBoxLastError(L"LoadMUILibrary", 0);
    Globals.bPrefLngNotAvail = (languageID != MUI_LanguageDLLs[0].LangId);
    languageID = MUI_LanguageDLLs[0].LangId;
    _hLangResourceContainer = Globals.hInstance;
    MUI_LanguageDLLs[0].bIsActive = true;
  }

  Globals.hLngResContainer = _hLangResourceContainer;
  SetThreadUILanguage(languageID);

  // ===  update language dependent items  ===

  for (cpi_enc_t enc = 0; enc < Encoding_CountOf(); ++enc)
  {
    Encoding_SetLabel(enc);
  }

  // ------------------------------------------------------------
  const WCHAR* const StatusBar_Section = L"Statusbar Settings";
  // ------------------------------------------------------------

  WCHAR tchStatusBar[MIDSZ_BUFFER] = { L'\0' };
  WCHAR tchDefaultStrg[MIDSZ_BUFFER] = { L'\0' };

  GetLngString(IDS_MUI_STATUSBAR_PREFIXES, tchDefaultStrg, COUNTOF(tchDefaultStrg));
  IniFileGetString(Globals.IniFile, StatusBar_Section, L"SectionPrefixes", tchDefaultStrg, tchStatusBar, COUNTOF(tchStatusBar));
  ReadStrgsFromCSV(tchStatusBar, g_mxSBPrefix, STATUS_SECTOR_COUNT, MICRO_BUFFER, L"_PRFX_");

  GetLngString(IDS_MUI_STATUSBAR_POSTFIXES, tchDefaultStrg, COUNTOF(tchDefaultStrg));
  IniSectionGetString(StatusBar_Section, L"SectionPostfixes", tchDefaultStrg, tchStatusBar, COUNTOF(tchStatusBar));
  ReadStrgsFromCSV(tchStatusBar, g_mxSBPostfix, STATUS_SECTOR_COUNT, MICRO_BUFFER, L"_POFX_");
   
  return languageID;
}


//=============================================================================
//
//  FreeLanguageResources
//
//
void FreeLanguageResources()
{
  if (Globals.hLngResContainer != Globals.hInstance) {
    HINSTANCE const _hLngResContainer = Globals.hLngResContainer;
    Globals.hLngResContainer = Globals.hInstance;
    MUI_LanguageDLLs[0].bIsActive = true;
    FreeMUILibrary(_hLngResContainer);
  }
  for (int i = 1; i < MuiLanguages_CountOf(); ++i) {
    MUI_LanguageDLLs[i].bIsActive = false;
  }
}


//=============================================================================
//
//  LoadLngStringW()
//
int LoadLngStringW(UINT uID, LPWSTR lpBuffer, int nBufferMax)
{
  const int nLen = LoadStringW(Globals.hLngResContainer, uID, lpBuffer, nBufferMax);
  return (nLen != 0) ? nLen : LoadStringW(Globals.hInstance, uID, lpBuffer, nBufferMax);
}

//=============================================================================
//
//  LoadLngStringW2MB()
//
static WCHAR s_tmpStringBuffer[512];

ptrdiff_t LoadLngStringW2MB(UINT uID, LPSTR lpBuffer, int nBufferMax)
{
  const int nLen = LoadStringW(Globals.hLngResContainer, uID, s_tmpStringBuffer, COUNTOF(s_tmpStringBuffer));
  if (nLen == 0) { LoadStringW(Globals.hInstance, uID, s_tmpStringBuffer, COUNTOF(s_tmpStringBuffer)); }
  return WideCharToMultiByteEx(CP_UTF8, 0, s_tmpStringBuffer, -1, lpBuffer, nBufferMax, NULL, NULL);
}

//=============================================================================
//
//  LoadLngStringA()
//
int LoadLngStringA(UINT uID, LPSTR lpBuffer, int nBufferMax)
{
  const int nLen = LoadStringA(Globals.hLngResContainer, uID, lpBuffer, nBufferMax);
  return (nLen != 0) ? nLen : LoadStringA(Globals.hInstance, uID, lpBuffer, nBufferMax);
}


//=============================================================================
//
//  FormatLngStringW()
//
int FormatLngStringW(LPWSTR lpOutput, int nOutput, UINT uIdFormat, ...)
{
  WCHAR* pBuffer = AllocMem(sizeof(WCHAR) * nOutput, HEAP_ZERO_MEMORY);
  if (pBuffer) {
    if (LoadLngStringW(uIdFormat, pBuffer, nOutput)) {
      StringCchVPrintfW(lpOutput, nOutput, pBuffer, (LPVOID)((PUINT_PTR)& uIdFormat + 1));
    }
    FreeMem(pBuffer);
    return (int)StringCchLenW(lpOutput, nOutput);
  }
  return 0;
}

//=============================================================================
//
//  FormatLngStringA()
//
int FormatLngStringA(LPSTR lpOutput, int nOutput, UINT uIdFormat, ...)
{
  CHAR* pBuffer = AllocMem(sizeof(CHAR) * nOutput, HEAP_ZERO_MEMORY);
  if (pBuffer) {
    if (LoadLngStringA(uIdFormat, pBuffer, nOutput)) {
      StringCchVPrintfA(lpOutput, nOutput, pBuffer, (LPVOID)((PUINT_PTR)& uIdFormat + 1));
    }
    FreeMem(pBuffer);
    return (int)StringCchLenA(lpOutput, nOutput);
  }
  return 0;
}




//=============================================================================

///   End of MuiLanguage.c   ///
