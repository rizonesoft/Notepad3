// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* MiniPath                                                                    *
*                                                                             *
* Config.h                                                                    *
*   Methods to read and write configuration                                   *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2020   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#pragma once
#ifndef _MINIPATH_CONFIG_H_
#define _MINIPATH_CONFIG_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "Helpers.h"

  //==== Ini-File Handling =============================================

  void InitDefaultSettings();
  void LoadSettings();
  void SaveSettings(BOOL);
  void LoadFlags();
  int  CheckIniFile(LPWSTR, LPCWSTR);
  int  CheckIniFileRedirect(LPWSTR, LPWSTR, LPWSTR, LPCWSTR);
  int  FindIniFile();
  int  TestIniFile();
  int  CreateIniFile();

  // ----------------------------------------------------------------------------

  BOOL LoadIniFileCache(LPCWSTR lpIniFilePath);
  BOOL IsIniFileCached();
  BOOL SaveIniFileCache(LPCWSTR lpIniFilePath);
  BOOL ResetIniFileCache();

  size_t IniSectionGetString(LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpDefault,
    LPWSTR lpReturnedString, size_t cchReturnedString);
  int IniSectionGetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iDefault);
  double IniSectionGetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dDefault);
  BOOL IniSectionGetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, BOOL bDefault);

  // ----------------------------------------------------------------------------

  BOOL IniSectionSetString(LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString);
  BOOL IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue);
  BOOL IniSectionSetLong(LPCWSTR lpSectionName, LPCWSTR lpKeyName, long lValue);
  BOOL IniSectionSetLongLong(LPCWSTR lpSectionName, LPCWSTR lpKeyName, long long llValue);
  BOOL IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue);
  BOOL IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue);
  BOOL IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpName, BOOL bValue);

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
  BOOL IniSectionDelete(LPCWSTR lpSectionName, LPCWSTR lpKeyName, BOOL bRemoveEmpty);
  BOOL IniSectionClear(LPCWSTR lpSectionName, BOOL bRemoveEmpty);
  BOOL IniClearAllSections(LPCWSTR lpPrefix, BOOL bRemoveEmpty);

  // ----------------------------------------------------------------------------

  // ==========================================
  // open file , get/set value, save(set) file
  // ==========================================

  size_t IniFileGetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpDefault,
    LPWSTR lpReturnedString, size_t cchReturnedString);
  BOOL  IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString);

  int  IniFileGetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iDefault);
  BOOL IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue);

  BOOL IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, BOOL bDefault);
  BOOL IniFileSetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, BOOL bValue);

  //  IniFileDeleteValue():
  //
  //  lpSectionName   Section to delete key from, or if
  //                  a_pKey is NULL, the section to remove.
  //  lpKeyName       Key to remove from the section.Set to
  //                  NULL to remove the entire section.
  //  bRemoveEmpty    If the section is empty after this key has
  //                  been deleted, should the empty section be removed ?
  //
  BOOL IniFileDelete(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, BOOL bRemoveEmpty);


  //  IniFileIterateSection():
  //
  typedef void (CALLBACK* IterSectionFunc_t)(LPCWSTR key, LPCWSTR value);
  BOOL IniFileIterateSection(LPCWSTR lpFilePath, LPCWSTR lpSectionName, IterSectionFunc_t callBack);


  // ----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //_MINIPATH_CONFIG_H_
