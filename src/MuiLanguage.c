// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* MuiLanguage.c                                                               *
*   General MUI Language support functions                                    *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2023   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

#include "Helpers.h"

#include <assert.h>
#include <locale.h>
#include <commctrl.h>
#include <muiload.h>

#include "PathLib.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Config/Config.h"

#include "MuiLanguage.h"

//=============================================================================


//=============================================================================
//
//  GetMUILanguageIndexByLocaleName
//  deprecated LANGID (!!!)
//
LANGID GetLangIdByLocaleName(LPCWSTR pLocaleName) {

    if (StrIsNotEmpty(pLocaleName)) {
        WCHAR wchLngLocalName[LOCALE_NAME_MAX_LENGTH + 1];
        int res = ResolveLocaleName(pLocaleName, wchLngLocalName, COUNTOF(wchLngLocalName));
        if (res > 0) {
            // get LANGID
            DWORD value = 0;
            res = GetLocaleInfoEx(wchLngLocalName, LOCALE_ILANGUAGE | LOCALE_RETURN_NUMBER, (LPWSTR)&value, sizeof(value) / sizeof(WCHAR));
            if (res > 0) {
                return (LANGID)value;
            }
        }
    }
    return 1033; // (!) en-US, not MUI_BASE_LNG_ID
}
//=============================================================================
//=============================================================================


grepWinLng_t grepWinLangResName[] = {
    { L"en-US",  L".\\lng\\gwLng\\English (United States) [en-US].lang" },
    { L"af-ZA",  L".\\lng\\gwLng\\Afrikaans (Suid-Afrika) [af-ZA].lang" },
    { L"be-BY",  L".\\lng\\gwLng\\Беларуская (Беларусь) [be-BY].lang" },
    { L"de-DE",  L".\\lng\\gwLng\\Deutsch (Deutschland) [de-DE].lang" },
    { L"el-GR",  L".\\lng\\gwLng\\Ελληνικά (Ελλάδα) [el-GR].lang" },
    { L"en-GB",  L".\\lng\\gwLng\\English (United Kingdom) [en-GB].lang" },
    { L"es-ES",  L".\\lng\\gwLng\\Español (España) [es-ES].lang" },
    { L"es-MX",  L".\\lng\\gwLng\\Español Mexicano (Mexico) [es-MX].lang" },
    { L"fr-FR",  L".\\lng\\gwLng\\Français (France) [fr-FR].lang" },
    { L"hi-IN",  L".\\lng\\gwLng\\हिन्दी (भारत) [hi-IN].lang" },
    { L"hu-HU",  L".\\lng\\gwLng\\Magyar (Magyarország) [hu-HU].lang" },
    { L"id-ID",  L".\\lng\\gwLng\\Bahasa Indonesia (Indonesia) [id-ID].lang" },
    { L"it-IT",  L".\\lng\\gwLng\\Italiano (Italia) [it-IT].lang" },
    { L"ja-JP",  L".\\lng\\gwLng\\日本語 (日本) [ja-JP].lang" },
    { L"ko-KR",  L".\\lng\\gwLng\\한국어 (대한민국) [ko-KR].lang" },
    { L"nl-NL",  L".\\lng\\gwLng\\Nederlands (Nederland) [nl-NL].lang" },
    { L"pl-PL",  L".\\lng\\gwLng\\Polski (Polska) [pl-PL].lang" },
    { L"pt-BR",  L".\\lng\\gwLng\\Português Brasileiro (Brasil) [pt-BR].lang" },
    { L"pt-PT",  L".\\lng\\gwLng\\Português (Portugal) [pt-PT].lang" },
    { L"ru-RU",  L".\\lng\\gwLng\\Русский (Pоссия) [ru-RU].lang" },
    { L"sk-SK",  L".\\lng\\gwLng\\Slovenčina (Slovensko) [sk-SK].lang" },
    { L"sv-SE",  L".\\lng\\gwLng\\Svenska (Sverige) [sv-SE].lang" },
    { L"tr-TR",  L".\\lng\\gwLng\\Türkçe (Türkiye) [tr-TR].lang" },
    { L"vi-VN",  L".\\lng\\gwLng\\Tiếng Việt (Việt Nam) [vi-VN].lang" },
    { L"zh-CN",  L".\\lng\\gwLng\\简体中文 (中国大陆) [zh-CN].lang" },
    { L"zh-TW",  L".\\lng\\gwLng\\正體中文 (中國台灣) [zh-TW].lang" }
};

unsigned grepWinLang_CountOf() {
    return COUNTOF(grepWinLangResName);
};


//=============================================================================
//
//  _LngStrToMultiLngStr
//
//
static bool _LngStrToMultiLngStr(LPWSTR const pLngStr, LPWSTR pLngMultiStr, size_t cchLngMultiStrCnt) {

    bool rtnVal = true;

    size_t strLen = StringCchLen(pLngStr, 0);

    if ((strLen > 0) && pLngMultiStr && (cchLngMultiStrCnt > 0)) {

        WCHAR *lngMultiStrPtr = pLngMultiStr;
        WCHAR *last = pLngStr + (Has_UTF16_BOM((char *)pLngStr, (strLen * sizeof(WCHAR))) ? 1 : 0);
        while (last && rtnVal) {
            // make sure you validate the user input
            WCHAR *next = StrNextTok(last, L",; :");
            if (next) {
                *next = L'\0';
            }
            strLen = StringCchLen(last, LOCALE_NAME_MAX_LENGTH);
            if (strLen && IsValidLocaleName(last)) {
                lngMultiStrPtr[0] = L'\0';
                rtnVal &= SUCCEEDED(StringCchCatW(lngMultiStrPtr, (cchLngMultiStrCnt - (lngMultiStrPtr - pLngMultiStr)), last));
                lngMultiStrPtr += strLen + 1;
            }
            last = (next ? next + 1 : next);
        }
        // make sure there is a double null term for the multi-string
        if (rtnVal && (cchLngMultiStrCnt - (lngMultiStrPtr - pLngMultiStr))) {
            lngMultiStrPtr[0] = L'\0';
        } else { // fail and guard anyone whom might use the multi-string
            lngMultiStrPtr[0] = L'\0';
            lngMultiStrPtr[1] = L'\0';
        }
    }
    return rtnVal;
}


//=============================================================================
//
//  SetMuiLocaleAll
//
static void SetMuiLocaleAll(LPCWSTR pszLocaleStr) {
    if (pszLocaleStr) {
        const WCHAR *const pszLocaleCur = _wsetlocale(LC_ALL, pszLocaleStr);
        if (pszLocaleCur && (StringCchCompareXI(pszLocaleStr, pszLocaleCur) != 0)) {
            //const _locale_t pCurLocale = _get_current_locale();
            _wsetlocale(LC_ALL, L""); // system standard
#if (defined(_DEBUG) || defined(DEBUG)) && !defined(NDEBUG)
            WCHAR msg[128];
            StringCchPrintf(msg, COUNTOF(msg), L"Can't set desired locale '%s', using '%s' instead!",
                pszLocaleStr, pszLocaleCur ? pszLocaleCur : L"<default>");
            MsgBoxLastError(msg, ERROR_MUI_INVALID_LOCALE_NAME);
#endif
        }
    }
}


//=============================================================================
//
//  SetMuiLanguage
//
void SetMuiLanguage(const unsigned muiLngIndex) {

    const WCHAR *pLocaleName = MUI_BASE_LNG_ID;
    if (muiLngIndex < MuiLanguages_CountOf()) {
        pLocaleName = MUI_LanguageDLLs[muiLngIndex].LocaleName;
    }

    if (!IsSameLocale(pLocaleName, Globals.CurrentLngLocaleName)) {

        // == MUI_LanguageDLLs[langIdx].LocaleName
        StringCchCopy(Globals.CurrentLngLocaleName, COUNTOF(Globals.CurrentLngLocaleName), pLocaleName);

        SetMuiLocaleAll(pLocaleName);

        WCHAR wchLanguagesList[(LOCALE_NAME_MAX_LENGTH + 1) * 2 + 4] = { L'\0' };
        WCHAR wchLanguagesBuffer[(LOCALE_NAME_MAX_LENGTH + 1) * 2 + 4] = { L'\0' };
        StringCchPrintf(wchLanguagesList, COUNTOF(wchLanguagesList), L"%s;%s", pLocaleName, MUI_BASE_LNG_ID);
        _LngStrToMultiLngStr(wchLanguagesList, wchLanguagesBuffer, COUNTOF(wchLanguagesBuffer));
        ULONG cnt = 2;
        SetProcessPreferredUILanguages(MUI_LANGUAGE_NAME, wchLanguagesBuffer, &cnt);

        // deprecated:
        LANGID const langID = GetLangIdByLocaleName(pLocaleName);
        SetThreadUILanguage(langID);
        InitMUILanguage(langID); // MUI Language for common controls

        const WCHAR *const SettingName = L"PreferredLanguageLocaleName";

        if (!IsSameLocale(Settings2.PreferredLanguageLocaleName, pLocaleName)) {

            StringCchCopyW(Settings2.PreferredLanguageLocaleName, COUNTOF(Settings2.PreferredLanguageLocaleName), pLocaleName);

            if (Globals.bCanSaveIniFile) {
                if (StringCchCompareXIW(Settings2.PreferredLanguageLocaleName, Default_PreferredLanguageLocaleName) != 0) {
                    IniFileSetString(Paths.IniFile, Constants.Settings2_Section, SettingName, Settings2.PreferredLanguageLocaleName);
                } else {
                    IniFileDelete(Paths.IniFile, Constants.Settings2_Section, SettingName, false);
                }
            }
        }
    }
}


//=============================================================================
//=============================================================================


extern prefix_t  g_mxSBPrefix[STATUS_SECTOR_COUNT];
extern prefix_t  g_mxSBPostfix[STATUS_SECTOR_COUNT];


MUILANGUAGE MUI_LanguageDLLs[] = {
    { IDS_MUI_LANG_EN_US,  L"en-US",   L"English (United States)\t\t\t[%s]",       true, false },
    // ----------------------------
    { IDS_MUI_LANG_AF_ZA,  L"af-ZA",   L"Afrikaans (Suid-Afrika)\t\t\t[%s]",       false, false },
    { IDS_MUI_LANG_BE_BY,  L"be-BY",   L"Беларуская (Беларусь)\t\t\t[%s]",         false, false },
    { IDS_MUI_LANG_DE_DE,  L"de-DE",   L"Deutsch (Deutschland)\t\t\t[%s]",         false, false },
    { IDS_MUI_LANG_EL_GR,  L"el-GR",   L"Ελληνικά (Ελλάδα)\t\t\t[%s]",             false, false },
    { IDS_MUI_LANG_EN_GB,  L"en-GB",   L"English (United Kingdom)\t\t\t[%s]",      false, false },
    { IDS_MUI_LANG_ES_ES,  L"es-ES",   L"Español (España)\t\t\t[%s]",              false, false },
    { IDS_MUI_LANG_ES_MX,  L"es-MX",   L"Español Mexicano (Mexico)\t\t\t[%s]",      false, false },
    { IDS_MUI_LANG_FR_FR,  L"fr-FR",   L"Français (France)\t\t\t[%s]",             false, false },
    { IDS_MUI_LANG_HI_IN,  L"hi-IN",   L"हिन्दी (भारत)\t\t\t[%s]",                    false, false },
    { IDS_MUI_LANG_HU_HU,  L"hu-HU",   L"Magyar (Magyarország)\t\t\t[%s]",         false, false },
    { IDS_MUI_LANG_ID_ID,  L"id-ID",   L"Bahasa Indonesia (Indonesia)\t\t\t[%s]",  false, false },
    { IDS_MUI_LANG_IT_IT,  L"it-IT",   L"Italiano (Italia)\t\t\t[%s]",             false, false },
    { IDS_MUI_LANG_JP_JP,  L"ja-JP",   L"日本語 (日本)\t\t\t[%s]",                  false, false },
    { IDS_MUI_LANG_KO_KR,  L"ko-KR",   L"한국어 (대한민국)\t\t\t[%s]",              false, false },
    { IDS_MUI_LANG_NL_NL,  L"nl-NL",   L"Nederlands (Nederland)\t\t\t[%s]",        false, false },
    { IDS_MUI_LANG_PL_PL,  L"pl-PL",   L"Polski (Polska)\t\t\t[%s]",               false, false },
    { IDS_MUI_LANG_PT_BR,  L"pt-BR",   L"Português Brasileiro (Brasil)\t\t\t[%s]", false, false },
    { IDS_MUI_LANG_PT_PT,  L"pt-PT",   L"Português (Portugal)\t\t\t[%s]",          false, false },
    { IDS_MUI_LANG_RU_RU,  L"ru-RU",   L"Русский (Pоссия)\t\t\t[%s]",              false, false },
    { IDS_MUI_LANG_SK_SK,  L"sk-SK",   L"Slovenčina (Slovensko)\t\t\t[%s]",        false, false },
    { IDS_MUI_LANG_SV_SE,  L"sv-SE",   L"Svenska (Sverige)\t\t\t[%s]",             false, false },
    { IDS_MUI_LANG_TR_TR,  L"tr-TR",   L"Türkçe (Türkiye)\t\t\t[%s]",              false, false },
    { IDS_MUI_LANG_VI_VN,  L"vi-VN",   L"Tiếng Việt (Việt Nam)\t\t\t[%s]",         false, false },
    { IDS_MUI_LANG_ZH_CN,  L"zh-CN",   L"简体中文 (中国大陆)\t\t\t[%s]",                false, false },
    { IDS_MUI_LANG_ZH_TW,  L"zh-TW",   L"正體中文 (中國台灣)\t\t\t[%s]",                false, false }
};

//NUM_OF_MUI_LANGUAGES
unsigned MuiLanguages_CountOf()
{
    return (unsigned)COUNTOF(MUI_LanguageDLLs);
};



//=============================================================================
//
//  GetMUILanguageIndexByLocaleName
//
unsigned GetMUILanguageIndexByLocaleName(LPCWSTR pLocaleName) {

    for (unsigned lng = 0; lng < MuiLanguages_CountOf(); ++lng) {
        if (StringCchCompareXI(pLocaleName, MUI_LanguageDLLs[lng].LocaleName) == 0) {
            return lng;
        }
    }
    return 0;
}


#if defined(HAVE_DYN_LOAD_LIBS_MUI_LNGS)

//=============================================================================
//
//  CheckAvailableLanguages
//
static unsigned _CheckAvailableLanguageDLLs()
{
    unsigned count = 1; // internal instance always available

    HPATHL hpth = Path_Allocate(NULL);

    for (unsigned lng = 1; lng < MuiLanguages_CountOf(); ++lng) {

        if (IsValidLocaleName(MUI_LanguageDLLs[lng].LocaleName)) {

#if (defined(_DEBUG) || defined(DEBUG)) && !defined(NDEBUG)
            WCHAR wchLngLocalName[LOCALE_NAME_MAX_LENGTH + 1];
            if (ResolveLocaleName(MUI_LanguageDLLs[lng].LocaleName, wchLngLocalName, COUNTOF(wchLngLocalName))) {
                //~StringCchCopy(MUI_LanguageDLLs[lng].LocaleName, COUNTOF(MUI_LanguageDLLs[lng].LocaleName), wchLngLocalName); // put back resolved name
                assert(IsSameLocale(MUI_LanguageDLLs[lng].LocaleName, wchLngLocalName) && "Problem with Locale Name of Language!");
            }
#endif
            // check for DLL
            WCHAR wchRelPath[SMALL_BUFFER] = { L'\0' };
            StringCchPrintf(wchRelPath, COUNTOF(wchRelPath), L"lng/%s/np3lng.dll.mui", MUI_LanguageDLLs[lng].LocaleName);
            Path_Reset(hpth, wchRelPath);
            Path_AbsoluteFromApp(hpth, false);
            bool const bAvail = Path_IsExistingFile(hpth);
            MUI_LanguageDLLs[lng].bHasDLL = bAvail;
            count += bAvail ? 1 : 0;
        }
    }
    Path_Release(hpth);
    return count;
}


//=============================================================================
//
//  GetUserPreferredLanguage
//  ~~~ GetUserDefaultLocaleName(pszPrefLocaleName_out, int cchBuffer);
//
bool GetUserPreferredLanguage(LPWSTR pszPrefLocaleName_out, int cchBuffer)
{
    ULONG numLngs = 0;
    DWORD cchLngsBuffer = 0;
    WCHAR wchLngLocalName[LOCALE_NAME_MAX_LENGTH + 1] = { L'\0' };
    if (GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &numLngs, NULL, &cchLngsBuffer)) {
        WCHAR * const pwszLngsBuffer = AllocMem((cchLngsBuffer + 2) * sizeof(WCHAR), HEAP_ZERO_MEMORY);
        if (pwszLngsBuffer) {
            if (GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &numLngs, pwszLngsBuffer, &cchLngsBuffer) && (numLngs > 0)) {
                // get the first (list ordered by preference)
                StringCchCopy(wchLngLocalName, COUNTOF(wchLngLocalName), pwszLngsBuffer);
            } else {
                numLngs = 0;
            }
            FreeMem(pwszLngsBuffer);
        } else {
            numLngs = 0;
        }
    }
    if (!numLngs) { // last try
        numLngs = GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SNAME, (LPWSTR)wchLngLocalName, COUNTOF(wchLngLocalName)) > 1 ? 1 : 0;
    }
    if (numLngs) {
        StringCchCopy(pszPrefLocaleName_out, cchBuffer, wchLngLocalName);
    }
    return numLngs;
}


//=============================================================================
//
//  LoadLanguageResources
//  return MUI_LanguageDLLs index
//
unsigned LoadLanguageResources(LPCWSTR pLocaleName) {

    FreeLanguageResources(); // reset
    Globals.hLngResContainer = NULL; // (!) unlink from instance resources

    unsigned const iInternalLngIndex = max_u(0, GetMUILanguageIndexByLocaleName(MUI_BASE_LNG_ID));

    // 1st check language resources
    Globals.uAvailLngCount = _CheckAvailableLanguageDLLs();

    // set the appropriate fallback list
    unsigned iLngIndex = MuiLanguages_CountOf();
    WCHAR tchAvailLngs[2 * (LOCALE_NAME_MAX_LENGTH + 1)] = { L'\0' };
    for (unsigned lng = 0; lng < MuiLanguages_CountOf(); ++lng) {
        if (StringCchCompareXIW(MUI_LanguageDLLs[lng].LocaleName, pLocaleName) == 0) {
            if (MUI_LanguageDLLs[lng].bHasDLL && (lng > 0)) {
                StringCchCatW(tchAvailLngs, COUNTOF(tchAvailLngs), MUI_LanguageDLLs[lng].LocaleName);
                StringCchCatW(tchAvailLngs, COUNTOF(tchAvailLngs), L";");
            }
            iLngIndex = lng;
            break;
        }
    }
    StringCchCatW(tchAvailLngs, COUNTOF(tchAvailLngs), MUI_LanguageDLLs[iInternalLngIndex].LocaleName); // en-US fallback

    // NOTES:
    // an application developer that makes the assumption the fallback list provided by the
    // system / OS is entirely sufficient may or may not be making a good assumption based  mostly on:
    // A. your choice of languages installed with your application
    // B. the languages on the OS at application install time
    // C. the OS users propensity to install/uninstall language packs
    // D. the OS users propensity to change language settings

    WCHAR tchUserLangMultiStrg[LARGE_BUFFER] = { L'\0' };
    if (!_LngStrToMultiLngStr(tchAvailLngs, tchUserLangMultiStrg, COUNTOF(tchUserLangMultiStrg))) {
        MsgBoxLastError(L"Trying to load available Language resources!", ERROR_MUI_INVALID_LOCALE_NAME);
    }
    ULONG langCount = 0;
    // using SetProcessPreferredUILanguages is recommended for new applications (esp. multi-threaded applications)
    SetProcessPreferredUILanguages(0, L"\0\0", &langCount); // clear
    if (!SetProcessPreferredUILanguages(MUI_LANGUAGE_NAME, tchUserLangMultiStrg, &langCount) || (langCount == 0)) {
#if (defined(_DEBUG) || defined(DEBUG)) && !defined(NDEBUG)
        MsgBoxLastError(L"Trying to set preferred Language!", ERROR_RESOURCE_LANG_NOT_FOUND);
#endif
    }

    // obtains access to the proper resource container
    // for standard Win32 resource loading this is normally a PE module - use LoadLibraryEx

    if (iLngIndex == iInternalLngIndex) {

        Globals.hLngResContainer = Globals.hInstance;
        MUI_LanguageDLLs[iInternalLngIndex].bIsActive = true;

    } else if ((iLngIndex >= 0) && MUI_LanguageDLLs[iLngIndex].bHasDLL) {

        LANGID const langID = GetLangIdByLocaleName(MUI_LanguageDLLs[iLngIndex].LocaleName);
        Globals.hLngResContainer = LoadMUILibrary(L"lng/np3lng.dll", MUI_LANGUAGE_NAME | MUI_LANGUAGE_EXACT, langID);

        if (Globals.hLngResContainer) {
            MUI_LanguageDLLs[iLngIndex].bIsActive = true;
            MUI_LanguageDLLs[iInternalLngIndex].bIsActive = false;
        } else {
            //MsgBoxLastError(L"LoadMUILibrary", 0);
            iLngIndex = MuiLanguages_CountOf(); // not found
        }
    }

    if (!Globals.hLngResContainer || (iLngIndex >= MuiLanguages_CountOf())) {
        // fallback to MUI_BASE_LNG_ID
        Globals.hLngResContainer = Globals.hInstance;
        MUI_LanguageDLLs[iInternalLngIndex].bIsActive = true;
        iLngIndex = iInternalLngIndex;

        const WCHAR *const suprMsg = L"MsgPrefLanguageNotAvailable";
        InfoBoxLng(MB_ICONWARNING, suprMsg, IDS_WARN_PREF_LNG_NOT_AVAIL, pLocaleName);
        int const noMsg = IniFileGetLong(Paths.IniFile, Constants.SectionSuppressedMessages, suprMsg, 0);
        if (noMsg && Globals.bCanSaveIniFile) {
            IniFileSetString(Paths.IniFile, Constants.Settings2_Section, L"PreferredLanguageLocaleName", MUI_LanguageDLLs[iInternalLngIndex].LocaleName);
        }
    }

    // ===  update language dependent items  ===

    for (cpi_enc_t enc = 0; enc < Encoding_CountOf(); ++enc) {
        Encoding_SetLabel(enc);
    }

    // ------------------------------------------------------------
    const WCHAR* const StatusBar_Section = L"Statusbar Settings";
    // ------------------------------------------------------------

    WCHAR tchStatusBar[MIDSZ_BUFFER] = { L'\0' };
    WCHAR tchDefaultStrg[MIDSZ_BUFFER] = { L'\0' };

    GetLngString(IDS_MUI_STATUSBAR_PREFIXES, tchDefaultStrg, COUNTOF(tchDefaultStrg));
    IniFileGetString(Paths.IniFile, StatusBar_Section, L"SectionPrefixes", tchDefaultStrg, tchStatusBar, COUNTOF(tchStatusBar));
    ReadStrgsFromCSV(tchStatusBar, g_mxSBPrefix, STATUS_SECTOR_COUNT, MICRO_BUFFER, L"_PRFX_");

    GetLngString(IDS_MUI_STATUSBAR_POSTFIXES, tchDefaultStrg, COUNTOF(tchDefaultStrg));
    IniFileGetString(Paths.IniFile, StatusBar_Section, L"SectionPostfixes", tchDefaultStrg, tchStatusBar, COUNTOF(tchStatusBar));
    ReadStrgsFromCSV(tchStatusBar, g_mxSBPostfix, STATUS_SECTOR_COUNT, MICRO_BUFFER, L"_POFX_");

    return iLngIndex;
}


//=============================================================================
//
//  FreeLanguageResources
//
//
void FreeLanguageResources() {

    CloseNonModalDialogs();
    unsigned const iInternalLngIndex = GetMUILanguageIndexByLocaleName(MUI_BASE_LNG_ID);
    if (Globals.hLngResContainer != Globals.hInstance) {
        FreeMUILibrary(Globals.hLngResContainer);
        Globals.hLngResContainer = Globals.hInstance;
    }
    for (unsigned i = 0; i < MuiLanguages_CountOf(); ++i) {
        MUI_LanguageDLLs[i].bIsActive = (iInternalLngIndex == i);
    }
}


//=============================================================================
//
//  InsertLanguageMenu
//
//

static HMENU s_hmenuLanguage = NULL;

bool InsertLanguageMenu(HMENU hMenuBar) {

    // check, if we need a language switching menu
    if (Globals.uAvailLngCount < 2) {
        Settings.PreferredLocale4DateFmt = false;
        return false;
    }

    if (s_hmenuLanguage) {
        DestroyMenu(s_hmenuLanguage);
    }
    s_hmenuLanguage = CreatePopupMenu();

    WCHAR wchMenuItemFmt[128] = { L'\0' };
    WCHAR wchMenuItemStrg[196] = { L'\0' };
    for (unsigned lng = 0; lng < MuiLanguages_CountOf(); ++lng) {
        if (MUI_LanguageDLLs[lng].bHasDLL) {
            StringCchCopy(wchMenuItemFmt, COUNTOF(wchMenuItemFmt), MUI_LanguageDLLs[lng].MenuItem);
            StringCchPrintfW(wchMenuItemStrg, COUNTOF(wchMenuItemStrg), wchMenuItemFmt, MUI_LanguageDLLs[lng].LocaleName);
            AppendMenu(s_hmenuLanguage, MF_ENABLED | MF_STRING, MUI_LanguageDLLs[lng].rid, wchMenuItemStrg);
        }
    }

    // --- insert ---
    int const pos = GetMenuItemCount(hMenuBar) - 1;
    if (pos >= 0) {
        GetLngString(IDS_MUI_MENU_LANGUAGE, wchMenuItemStrg, COUNTOF(wchMenuItemStrg));
        //return InsertMenu(hMenuBar, pos, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT_PTR)s_hmenuLanguage, wchMenuItemStrg);
        bool const res = InsertMenu(hMenuBar, IDM_SET_TABSASSPACES, MF_BYCOMMAND | MF_POPUP | MF_STRING, (UINT_PTR)s_hmenuLanguage, wchMenuItemStrg);
        GetLngString(IDS_USE_LOCALE_DATEFMT, wchMenuItemStrg, COUNTOF(wchMenuItemStrg));
        InsertMenu(hMenuBar, IDM_SET_TABSASSPACES, MF_BYCOMMAND | MF_STRING, (UINT_PTR)IDS_USE_LOCALE_DATEFMT, wchMenuItemStrg);
        InsertMenu(hMenuBar, IDM_SET_TABSASSPACES, MF_BYCOMMAND | MF_SEPARATOR, (UINT_PTR)NULL, NULL);
        return res;
    }
    return false;
}

//=============================================================================
//
//  DynamicLanguageMenuCmd() - Handles IDS_MUI_LANG_XX_YY messages
//
void DynamicLanguageMenuCmd(int cmd) {

    // consecutive resource IDs
    unsigned const iLngIdx = (unsigned)((cmd >= IDS_MUI_LANG_EN_US) ? 
                             (cmd - IDS_MUI_LANG_EN_US) : MuiLanguages_CountOf()); 
    if (iLngIdx >= MuiLanguages_CountOf()) {
        return;
    }
    if (!MUI_LanguageDLLs[iLngIdx].bIsActive) {

        DestroyMenu(Globals.hMainMenu);

        // desired language
        LPCWSTR desiredLocaleName = MUI_LanguageDLLs[iLngIdx].LocaleName;
        SetMuiLanguage(LoadLanguageResources(desiredLocaleName));

        Globals.hMainMenu = LoadMenu(Globals.hLngResContainer, MAKEINTRESOURCE(IDR_MUI_MAINMENU));
        if (!Globals.hMainMenu) {
            MsgBoxLastError(L"LoadMenu()", 0);
            CloseApplication();
            return;
        }

        InsertLanguageMenu(Globals.hMainMenu);
        SetMenu(Globals.hwndMain, (Settings.ShowMenubar ? Globals.hMainMenu : NULL));
        DrawMenuBar(Globals.hwndMain);
    }
}


#endif  // HAVE_DYN_LOAD_LIBS_MUI_LNGS


//=============================================================================
//
//  LoadLngStringW()
//
int LoadLngStringW(UINT uID, LPWSTR lpBuffer, int nBufferMax)
{
    const int nLen = LoadStringW(Globals.hLngResContainer, uID, lpBuffer, nBufferMax);
    return (nLen ? nLen : LoadStringW(Globals.hInstance, uID, lpBuffer, nBufferMax));
}

//=============================================================================
//
//  LoadLngStringW2MB()
//
int LoadLngStringW2MB(UINT uID, LPSTR lpBuffer, int nBufferMax)
{
    int len = 0;
    WCHAR * const pBuffer = (WCHAR *)AllocMem(sizeof(WCHAR) * nBufferMax, HEAP_ZERO_MEMORY);
    if (pBuffer) {
        const int nLen = LoadStringW(Globals.hLngResContainer, uID, pBuffer, nBufferMax);
        if (nLen == 0) {
            LoadStringW(Globals.hInstance, uID, pBuffer, nBufferMax);
        }
        len = WideCharToMultiByte(Encoding_SciCP, 0, pBuffer, -1, lpBuffer, nBufferMax, NULL, NULL);
        FreeMem(pBuffer);
    }
    return len;
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
    WCHAR* const pBuffer = AllocMem(sizeof(WCHAR) * nOutput, HEAP_ZERO_MEMORY);
    if (pBuffer) {
        if (LoadLngStringW(uIdFormat, pBuffer, nOutput)) {
            StringCchVPrintfW(lpOutput, nOutput, pBuffer, (LPVOID)((PUINT_PTR)& uIdFormat + 1));
        }
        FreeMem(pBuffer);
        return (int)StringCchLen(lpOutput, nOutput);
    }
    return 0;
}

//=============================================================================
//
//  FormatLngStringA()
//
int FormatLngStringA(LPSTR lpOutput, int nOutput, UINT uIdFormat, ...)
{
    CHAR* const pBuffer = AllocMem(sizeof(CHAR) * nOutput, HEAP_ZERO_MEMORY);
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
