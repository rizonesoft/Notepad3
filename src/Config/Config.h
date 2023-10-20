// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Config.h                                                                    *
*   Methods to read and write configuration                                   *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2023   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#pragma once
#ifndef _NP3_CONFIG_H_
#define _NP3_CONFIG_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "TypeDefs.h"
#include "PathLib.h"
#include "Helpers.h"

//==== Ini-File Handling =============================================

bool FindIniFile();
bool TestIniFile();
bool CanAccessPath(const HPATHL hpth, DWORD genericAccessRights);
bool CreateIniFile(const HPATHL hini_pth, DWORD* pdwFileSize_out);
void LoadSettings();
bool SaveWindowPositionSettings(bool bClearSettings);
bool SaveAllSettings(bool bForceSaveSettings);
void CmdSaveSettingsNow();

bool OpenSettingsFile(LPCSTR fctname);
bool CloseSettingsFile(LPCSTR fctname, bool bSaveSettings);

// ----------------------------------------------------------------------------

bool CopyToTmpCache(LPCSTR lpIniFileResource);
size_t TmpCacheGetString(LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpDefault,
    LPWSTR lpReturnedString, const size_t cchReturnedString);
bool TmpCacheSetString(LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString);
bool ResetTmpCache();

// ----------------------------------------------------------------------------

bool LoadIniFileCache(const HPATHL hpthIniFile);
bool IsIniFileCached();
bool SaveIniFileCache(const HPATHL hpthIniFile);
bool ResetIniFileCache();

size_t IniSectionGetString(LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpDefault,
                           LPWSTR lpReturnedString, const size_t cchReturnedString);
size_t IniSectionGetStringNoQuotes(LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpDefault,
                                   LPWSTR lpReturnedString, const size_t cchReturnedString);
int IniSectionGetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iDefault);
long IniSectionGetLong(LPCWSTR lpSectionName, LPCWSTR lpKeyName, long lDefault);
long long IniSectionGetLongLong(LPCWSTR lpSectionName, LPCWSTR lpKeyName, long long llDefault);
double IniSectionGetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dDefault);
bool IniSectionGetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault);

inline DocPos IniSectionGetPos(LPCWSTR lpSectionName, LPCWSTR lpKeyName, DocPos posDefault)
{
    return (DocPos)IniSectionGetLongLong(lpSectionName, lpKeyName, posDefault);
}

// ----------------------------------------------------------------------------

bool IniSectionSetString(LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString);
bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue);
bool IniSectionSetLong(LPCWSTR lpSectionName, LPCWSTR lpKeyName, long lValue);
bool IniSectionSetLongLong(LPCWSTR lpSectionName, LPCWSTR lpKeyName, long long llValue);
bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue);
bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue);
bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue);

inline bool IniSectionSetPos(LPCWSTR lpSectionName, LPCWSTR lpKeyName, DocPos posValue)
{
    return IniSectionSetLongLong(lpSectionName, lpKeyName, posValue);
}

// ----------------------------------------------------------------------------

//  IniSectionDeleteValue():
//
//  lpSectionName   Section to delete key from, or if
//                  a_pKey is NULL, the section to remove.
//  lpKeyName       Key to remove from the section.Set to
//                  NULL to remove the entire section.
//  bRemoveEmpty    If the section is empty after this key has
//                  been deleted, should the empty section be removed ?
//
bool IniSectionDelete(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty);
bool IniSectionClear(LPCWSTR lpSectionName, bool bRemoveEmpty);
bool IniClearAllSections(LPCWSTR lpPrefix, bool bRemoveEmpty);

// ----------------------------------------------------------------------------

// ==========================================
// open file , get/set value, save(set) file
// ==========================================

size_t IniFileGetString(const HPATHL hpthIniFile, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpDefault,
                        LPWSTR lpReturnedString, size_t cchReturnedString);
bool   IniFileSetString(const HPATHL hpthIniFile, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString);

long IniFileGetLong(const HPATHL hpthIniFile, LPCWSTR lpSectionName, LPCWSTR lpKeyName, long lDefault);
bool IniFileSetLong(const HPATHL hpthIniFile, LPCWSTR lpSectionName, LPCWSTR lpKeyName, long lValue);

bool IniFileGetBool(const HPATHL hpthIniFile, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault);
bool IniFileSetBool(const HPATHL hpthIniFile, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue);

//  IniFileDeleteValue():
//
//  lpSectionName   Section to delete key from, or if
//                  a_pKey is NULL, the section to remove.
//  lpKeyName       Key to remove from the section.Set to
//                  NULL to remove the entire section.
//  bRemoveEmpty    If the section is empty after this key has
//                  been deleted, should the empty section be removed ?
//
bool IniFileDelete(const HPATHL hpthIniFile, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty);

//  IniFileIterateSection():
//
typedef void (CALLBACK* IterSectionFunc_t)(LPCWSTR key, LPCWSTR value);
bool IniFileIterateSection(const HPATHL hpthIniFile, LPCWSTR lpSectionName, IterSectionFunc_t callBack);

//==== MRU Functions ==========================================================

void AddFilePathToRecentDocs(const HPATHL hpthFile);
//void ClearDestinationsOnRecentDocs();

LPMRULIST MRU_Create(LPCWSTR pszRegKey, int iFlags, int iSize);
bool      MRU_Destroy(LPMRULIST pmru);
bool      MRU_Add(LPMRULIST pmru, LPCWSTR pszNew, cpi_enc_t iEnc, DocPos iPos, DocPos iSelAnc, LPCWSTR pszBookMarks);
bool      MRU_FindPath(LPMRULIST pmru, const HPATHL hpth, int* iIndex);
bool      MRU_AddPath(LPMRULIST pmru, const HPATHL hpth, bool bRelativePath, bool bUnexpandMyDocs, cpi_enc_t iEnc, DocPos iPos, DocPos iSelAnc, LPCWSTR pszBookMarks);
bool      MRU_Delete(LPMRULIST pmru, int iIndex);
bool      MRU_Empty(LPMRULIST pmru, bool bExceptLeast, bool bDelete);
int       MRU_Enum(LPMRULIST pmru, int iIndex, LPWSTR pszItem, int cchItem);
bool      MRU_Load(LPMRULIST pmru, bool bFileProps);
void      MRU_Save(LPMRULIST pmru);
bool      MRU_MergeSave(LPMRULIST pmru, bool bAddFiles, bool bRelativePath, bool bUnexpandMyDocs);
#define   MRU_Count(pmru) MRU_Enum((pmru), 0, NULL, 0)

// ----------------------------------------------------------------------------

extern const WCHAR* const g_CodeFontPrioList[10];
extern const WCHAR* const g_TextFontPrioList[7];

#ifdef __cplusplus
}
#endif


#endif //_NP3_CONFIG_H_
