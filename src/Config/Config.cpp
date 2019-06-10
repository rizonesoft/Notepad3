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

#include <strsafe.h>

#include "SimpleIni.h"
#include "Config.h"


#define _PRIV_PROFILE_STRING_HANDLE_


static bool const s_bIsUTF8 = true;
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = false;
static bool const s_bSetSpaces = false;

static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);


// ============================================================================


extern "C" bool LoadIniFile(LPCWSTR lpIniFilePath)
{
  s_INI.Reset();
  SI_Error const rc = s_INI.LoadFile(lpIniFilePath);
  return (rc == SI_OK);
}

extern "C" bool SaveIniFile()
{
  s_INI.SetSpaces(s_bSetSpaces);
  SI_Error const rc = s_INI.SaveFile(Globals.IniFile, true);
  s_INI.Reset(); // done
  return (rc == SI_OK);
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
  assert(!bHasMultiple);
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" int IniSectionGetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iDefault)
{
  bool bHasMultiple = false;
  int const iValue = (int)s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
  assert(!bHasMultiple);
  return iValue;
}
// ============================================================================


extern "C" double IniSectionGetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dDefault)
{
  bool bHasMultiple = false;
  double const dValue = s_INI.GetDoubleValue(lpSectionName, lpKeyName, dDefault, &bHasMultiple);
  assert(!bHasMultiple);
  return dValue;
}
// ============================================================================


extern "C" bool IniSectionGetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  bool bHasMultiple = false;
  bool const bValue = s_INI.GetBoolValue(lpSectionName, lpKeyName, bDefault, &bHasMultiple);
  assert(!bHasMultiple);
  return bValue;
}
// ============================================================================


extern "C" bool IniSectionSetString(LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  if (lpString == NULL) {
    return s_INI.DeleteValue(lpSectionName, lpKeyName, NULL, false);
  }
  SI_Error const rc = s_INI.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
  return (rc == SI_OK);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  return (rc == SI_OK);
}
extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
  return (rc == SI_OK);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
  SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
  return (rc == SI_OK);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  return (rc == SI_OK);
}
// ============================================================================


extern "C" bool IniSectionClear(LPCWSTR lpSectionName, bool bRemoveEmpty)
{
  return s_INI.Delete(lpSectionName, nullptr, bRemoveEmpty);
}
// ============================================================================


extern "C" bool IniClearAllSections(LPCWSTR lpPrefix, bool bRemoveEmpty)
{
  size_t const len = StringCchLen(lpPrefix, 0);

  CSimpleIni::TNamesDepend Sections;
  s_INI.GetAllSections(Sections);
  bool ok = true;
  for (const auto& section : Sections)
  {
    if (StringCchCompareNI(section.pItem, len, lpPrefix, len) == 0)
    {
      ok = ok && s_INI.Delete(section.pItem, nullptr, bRemoveEmpty);
    }
  }
  return ok;
}
// ============================================================================


// ============================================================================
// ============================================================================


extern "C" size_t IniFileGetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpDefault,
                                   LPWSTR lpReturnedString, size_t cchReturnedString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (rc == SI_OK) {
    bool bHasMultiple = false;
    StringCchCopyW(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    assert(!bHasMultiple);
  }
  return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (rc == SI_OK) {
    if (lpString == NULL) {
      rc = s_INI.DeleteValue(lpSectionName, lpKeyName, NULL, false) ? SI_OK : SI_FAIL;
    }
    else {
      rc = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    }
  }
  if (rc == SI_OK) {
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return (rc == SI_OK);
}
// ============================================================================


extern "C" int IniFileGetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (rc == SI_OK) {
    bool bHasMultiple = false;
    int const iValue = s_INI.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
    assert(!bHasMultiple);
    return iValue;
  }
  return iDefault;
}
// ============================================================================


extern "C" bool IniFileSetInt(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (rc == SI_OK) {
    rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
  }
  if (rc == SI_OK) {
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return (rc == SI_OK);
}
// ============================================================================


extern "C" bool IniFileGetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error const rc = Ini.LoadFile(lpFilePath);
  if (rc == SI_OK) {
    bool bHasMultiple = false;
    bool const bValue = s_INI.GetBoolValue(lpSectionName, lpKeyName, bDefault, &bHasMultiple);
    assert(!bHasMultiple);
    return bValue;
  }
  return bDefault;
}
// ============================================================================


extern "C" bool IniFileSetBool(LPCWSTR lpFilePath, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
  CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
  SI_Error rc = Ini.LoadFile(lpFilePath);
  if (rc == SI_OK) {
    rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
  }
  if (rc == SI_OK) {
    Ini.SetSpaces(s_bSetSpaces);
    rc = Ini.SaveFile(Globals.IniFile, true);
  }
  return (rc == SI_OK);
}
// ============================================================================



//=============================================================================
