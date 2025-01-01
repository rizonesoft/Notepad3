// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* MuiLanguage.h                                                               *
*   Definitions for MUI Language support                                      *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2025   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

// TODO: Get rid of these deprecated LCID Stuff (see "winnt.h"):
//
// ** DEPRECATED ** DEPRECATED ** DEPRECATED ** DEPRECATED ** DEPRECATED **
//
//  DEPRECATED: The LCID/LANGID/SORTID concept is deprecated, please use
//  Locale Names instead, eg: en-US instead of an LCID like 0x0409.
//  See the documentation for GetLocaleInfoEx.
//
//  A locale ID is a 32 bit value which is the combination of a
//  language ID, a sort ID, and a reserved area.  The bits are
//  allocated as follows:
//
//       +-------------+---------+-------------------------+
//       |   Reserved  | Sort ID |      Language ID        |
//       +-------------+---------+-------------------------+
//        31         20 19     16 15                      0   bit
//
//  WARNING: This pattern isn't always followed (es-ES_tradnl vs es-ES for example)
//
//  WARNING: Some locales do not have assigned LCIDs.  Please use
//           Locale Names, such as "tlh-Piqd".
//
//  It is recommended that applications test for locale names rather than
//  attempting to rely on LCID or LANGID behavior.
//
//  DEPRECATED: Locale ID creation/extraction macros:
//
//    MAKELCID            - construct the locale id from a language id and a sort id.
//    MAKESORTLCID        - construct the locale id from a language id, sort id, and sort version.
//    LANGIDFROMLCID      - extract the language id from a locale id.
//    SORTIDFROMLCID      - extract the sort id from a locale id.
//    SORTVERSIONFROMLCID - extract the sort version from a locale id.
//
//  Note that the LANG, SUBLANG construction is not always consistent.
//  The named locale APIs (eg GetLocaleInfoEx) are recommended.
//
//  DEPRECATED: LCIDs do not exist for all locales.
//
// ** DEPRECATED ** DEPRECATED ** DEPRECATED ** DEPRECATED ** DEPRECATED **
//

#pragma once
#ifndef _NP3_MUI_LANGUAGE_H_
#define _NP3_MUI_LANGUAGE_H_

#include "resource.h"

// ============================================================================
// deprecated LCID/LANGID (!)  try to eliminate in future
// ----------------------------------------------------------------------------
LANGID GetLangIdByLocaleName(LPCWSTR pLocaleName);

inline int LangIDToLocaleName(const LANGID lngID, LPWSTR lpName_out, size_t cchName) {
    LCID const lcid = MAKELCID(lngID, SORT_DEFAULT);
    return LCIDToLocaleName(lcid, lpName_out, (int)cchName, 0);
}
// ============================================================================


typedef struct _muilanguage {
    UINT    rid;
    const WCHAR* const LocaleName;
    const WCHAR* const MenuItem;
    bool    bHasDLL;
    bool    bIsActive;

} MUILANGUAGE, *PMUILANGUAGE;

extern MUILANGUAGE MUI_LanguageDLLs[];
unsigned MuiLanguages_CountOf();

void SetMuiLanguage(const unsigned muiLngIndex);
unsigned GetMUILanguageIndexByLocaleName(LPCWSTR pLocaleName);

inline UINT GetMUILngResourceID(const unsigned idx) {
    return (idx < MuiLanguages_CountOf()) ? MUI_LanguageDLLs[idx].rid : 0;
}

inline const WCHAR* GetMUILocaleNameByIndex(const unsigned idx) {
    return (idx < MuiLanguages_CountOf()) ? MUI_LanguageDLLs[idx].LocaleName : NULL;
}

inline bool IsMUILanguageActive(const unsigned idx) {
    return (idx < MuiLanguages_CountOf()) ? MUI_LanguageDLLs[idx].bIsActive : false;
}

inline bool ExistMUILanguageDLL(const unsigned idx) {
    return (idx < MuiLanguages_CountOf()) ? MUI_LanguageDLLs[idx].bHasDLL : false;
}

inline bool IsSameLocale(const WCHAR *ln1, const WCHAR *ln2) {
    return (((ln1 && ln2) ? _wcsicmp(ln1, ln2) : -1) == 0);
}

// ============================================================================

#if defined(HAVE_DYN_LOAD_LIBS_MUI_LNGS)

bool GetUserPreferredLanguage(LPWSTR pszPrefLocaleName_out, int cchBuffer);
unsigned LoadLanguageResources(LPCWSTR localeName);
void FreeLanguageResources();
bool InsertLanguageMenu(HMENU hMenuBar);
void DynamicLanguageMenuCmd(int cmd);

#endif // HAVE_DYN_LOAD_LIBS_MUI_LNGS

typedef struct _gwlang_ini {
    const WCHAR* const localename;
    const WCHAR* const filename;
} grepWinLng_t;

extern grepWinLng_t grepWinLangResName[];
unsigned grepWinLang_CountOf();

int LoadLngStringW(UINT uID, LPWSTR lpBuffer, int nBufferMax);
int LoadLngStringA(UINT uID, LPSTR lpBuffer, int nBufferMax);
int FormatLngStringW(LPWSTR lpOutput, int nOutput, UINT uIdFormat, ...);
int FormatLngStringA(LPSTR lpOutput, int nOutput, UINT uIdFormat, ...);
int LoadLngStringW2MB(UINT uID, LPSTR lpBuffer, int nBufferMax);

#define GetLngString(id,pb,cb) LoadLngStringW((id),(pb),(cb))
#define GetLngStringA(id,pb,cb) LoadLngStringA((id),(pb),(cb))
#define GetLngStringW2MB(id,pb,cb) LoadLngStringW2MB((id),(pb),(cb))


#endif //_NP3_MUI_LANGUAGE_H_

///   End of MuiLanguage.h   ///
