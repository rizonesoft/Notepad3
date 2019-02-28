﻿/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* MuiLanguage.c                                                               *
*   General MUI Language support functions                                    *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2019   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

#include "Helpers.h"

#include <muiload.h>

#include "resource.h"
#include "Encoding.h"
#include "MuiLanguage.h"

//=============================================================================

MUILANGUAGE MUI_LanguageDLLs[NUM_OF_MUI_LANGUAGES] =
{
  { IDS_MUI_LANG_EN_US, L"en-US", MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), true, false }, // internal - must be 1st
  // ----------------------------
  { IDS_MUI_LANG_AF_ZA, L"af-ZA", MAKELANGID(LANG_AFRIKAANS, SUBLANG_AFRIKAANS_SOUTH_AFRICA), false, false },
  { IDS_MUI_LANG_BE_BY, L"be-BY", MAKELANGID(LANG_BELARUSIAN, SUBLANG_BELARUSIAN_BELARUS), false, false },
  { IDS_MUI_LANG_DE_DE, L"de-DE", MAKELANGID(LANG_GERMAN, SUBLANG_GERMAN), false, false },
  { IDS_MUI_LANG_EN_GB, L"en-GB", MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_UK), false, false },
  { IDS_MUI_LANG_ES_ES, L"es-ES", MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_MODERN), false, false },
  { IDS_MUI_LANG_FR_FR, L"fr-FR", MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH), false, false },
  { IDS_MUI_LANG_HU_HU, L"hu-HU", MAKELANGID(LANG_HUNGARIAN, SUBLANG_HUNGARIAN_HUNGARY), false, false },
  { IDS_MUI_LANG_IT_IT, L"it-IT", MAKELANGID(LANG_ITALIAN, SUBLANG_ITALIAN), false, false },
  { IDS_MUI_LANG_JP_JP, L"ja-JP", MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN), false, false },
  { IDS_MUI_LANG_KO_KR, L"ko-KR", MAKELANGID(LANG_KOREAN, SUBLANG_KOREAN), false, false },
  { IDS_MUI_LANG_NL_NL, L"nl-NL", MAKELANGID(LANG_DUTCH, SUBLANG_DUTCH), false, false },
  { IDS_MUI_LANG_PT_BR, L"pt-BR", MAKELANGID(LANG_PORTUGUESE, SUBLANG_PORTUGUESE_BRAZILIAN), false, false },
  { IDS_MUI_LANG_RU_RU, L"ru-RU", MAKELANGID(LANG_RUSSIAN, SUBLANG_RUSSIAN_RUSSIA), false, false },
  { IDS_MUI_LANG_ZH_CN, L"zh-CN", MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED), false, false}
};



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
  for (int lng = 1; lng < NUM_OF_MUI_LANGUAGES; ++lng)
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
    WCHAR* last = pLngStr + (Has_UTF16_LE_BOM((char*)pLngStr, clampi((int)strLen, 0, 8)) ? 1 : 0);
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

  if (StringCchLen(pszPrefLocaleName, cchBuffer) > 0)
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
HMODULE LoadLanguageResources(LANGID* pPrefLanguageID)
{
  // 1st check language resources
  Globals.iAvailLngCount = _CheckAvailableLanguageDLLs();

  LANGID languageID = MUI_LanguageDLLs[0].LangId;

  WCHAR wchLngLocalName[LOCALE_NAME_MAX_LENGTH];
  StringCchCopy(wchLngLocalName, COUNTOF(wchLngLocalName), Settings2.PreferredLanguageLocaleName);

  if (_GetUserPreferredLanguage(wchLngLocalName, COUNTOF(wchLngLocalName), &languageID)) {
    // push-back (corrected) name found
    StringCchCopy(Settings2.PreferredLanguageLocaleName, COUNTOF(Settings2.PreferredLanguageLocaleName), wchLngLocalName);
  }

  // set the appropriate fallback list
  int iPrefLngIndex = -1;
  WCHAR tchAvailLngs[2 * (LOCALE_NAME_MAX_LENGTH + 1)] = { L'\0' };
  for (int lng = 0; lng < NUM_OF_MUI_LANGUAGES; ++lng) {
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
    GetLastErrorToMsgBox(L"_LngStrToMultiLngStr()", ERROR_MUI_INVALID_LOCALE_NAME);
  }
  DWORD langCount = 0;
  // using SetProcessPreferredUILanguages is recommended for new applications (esp. multi-threaded applications)
  if (!SetThreadPreferredUILanguages(MUI_LANGUAGE_NAME, tchUserLangMultiStrg, &langCount) || (langCount == 0))
  {
    GetLastErrorToMsgBox(L"SetProcessPreferredUILanguages()", 0);
  }

  // obtains access to the proper resource container 
  // for standard Win32 resource loading this is normally a PE module - use LoadLibraryEx

  HMODULE hLangResourceContainer = NULL;
  Globals.bPrefLngNotAvail = (iPrefLngIndex < 0);

  if ((iPrefLngIndex >= 0) && MUI_LanguageDLLs[iPrefLngIndex].bHasDLL) {
    hLangResourceContainer = (0 == iPrefLngIndex) ? Globals.hInstance :
      LoadMUILibrary(L"lng/np3lng.dll", MUI_LANGUAGE_NAME | MUI_LANGUAGE_EXACT, languageID);
    MUI_LanguageDLLs[iPrefLngIndex].bIsLoaded = true;
  }

  if (!hLangResourceContainer) {
    // fallback to ENGLISH_US
    Globals.bPrefLngNotAvail = (languageID != MUI_LanguageDLLs[0].LangId);
    languageID = MUI_LanguageDLLs[0].LangId;
    hLangResourceContainer = Globals.hInstance;
    MUI_LanguageDLLs[0].bIsLoaded = true;
  }

  SetThreadUILanguage(languageID);

  *pPrefLanguageID = languageID;
  return hLangResourceContainer;
}


//=============================================================================
//
//  FreeLanguageResources
//
//
void FreeLanguageResources(HMODULE hLangResourceContainer)
{
  if (hLangResourceContainer) {
    if (hLangResourceContainer != Globals.hInstance) {
      FreeMUILibrary(hLangResourceContainer);
    }
  }
  for (int i = 0; i < NUM_OF_MUI_LANGUAGES; ++i) {
    MUI_LanguageDLLs[i].bIsLoaded = false;
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

int LoadLngStringW2MB(UINT uID, LPSTR lpBuffer, int nBufferMax)
{
  const int nLen = LoadStringW(Globals.hLngResContainer, uID, s_tmpStringBuffer, COUNTOF(s_tmpStringBuffer));
  if (nLen == 0) { LoadStringW(Globals.hInstance, uID, s_tmpStringBuffer, COUNTOF(s_tmpStringBuffer)); }
  return WideCharToMultiByte(CP_UTF8, 0, s_tmpStringBuffer, -1, lpBuffer, nBufferMax, NULL, NULL);
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
