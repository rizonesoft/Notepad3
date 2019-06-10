/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Config.h                                                                    *
*   Methods to read and write configuration                                   *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2019   *
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
#include "Helpers.h"

//==== Ini-File Handling =============================================

bool FindIniFile();
int  TestIniFile();
bool  CreateIniFile();
bool CreateIniFileEx(LPCWSTR lpszIniFile);
void LoadSettings();
void LoadFlags();
bool SaveSettings(bool);

// ----------------------------------------------------------------------------

bool LoadIniFile(LPCWSTR lpIniFilePath);
bool SaveIniFile(LPCWSTR lpIniFilePath);
void ReleaseIniFile();

size_t IniSectionGetString(LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpDefault, 
                           LPWSTR lpReturnedString, size_t cchReturnedString);
int IniSectionGetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iDefault);
double IniSectionGetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dDefault);
bool IniSectionGetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault);

inline DocPos IniSectionGetPos(LPCWSTR lpSectionName, LPCWSTR lpKeyName, DocPos posDefault) {
  return (DocPos)IniSectionGetInt(lpSectionName, lpKeyName, (MBWC_DocPos_Cast)posDefault);
}

// ----------------------------------------------------------------------------

bool IniSectionSetString(LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString);
bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue);
bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue);
bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue);
bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpName, bool bValue);

inline bool IniSectionSetPos(LPCWSTR lpSectionName, LPCWSTR lpKeyName, DocPos posValue) {
  return IniSectionSetInt(lpSectionName, lpKeyName, (MBWC_DocPos_Cast)posValue);
}

// ----------------------------------------------------------------------------

//  IniSectionDeleteValue():
//
//  lpSectionName   Section to delete key from, or if
//                  a_pKey is NULL, the section to remove.
//  lpKeyName       Key to remove from the section.Set to
//                  NULL to remove the entire section.
//  lpValue         Value of key to remove from the section.
//                  Set to NULL to remove all keys.
//  bRemoveEmpty    If the section is empty after this key has
//                  been deleted, should the empty section be removed ?
//
bool IniSectionDeleteValue(LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpValue, bool bRemoveEmpty);
bool IniSectionClear(LPCWSTR lpSectionName, bool bRemoveEmpty);
bool IniClearAllSections(LPCWSTR lpPrefix, bool bRemoveEmpty);

// ----------------------------------------------------------------------------

// ==========================================
// open file , get/set value, save(set) file 
// ==========================================

size_t IniFileGetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpDefault,
                        LPWSTR lpReturnedString, size_t cchReturnedString);
bool  IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString);

int  IniFileGetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iDefault);
bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue);

bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault);
bool IniFileSetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue);

//  IniFileDeleteValue():
//
//  lpSectionName   Section to delete key from, or if
//                  a_pKey is NULL, the section to remove.
//  lpKeyName       Key to remove from the section.Set to
//                  NULL to remove the entire section.
//  lpValue         Value of key to remove from the section.
//                  Set to NULL to remove all keys.
//  bRemoveEmpty    If the section is empty after this key has
//                  been deleted, should the empty section be removed ?
//
bool IniFileDeleteValue(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpValue, bool bRemoveEmpty);

// ----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif //_NP3_CONFIG_H_
