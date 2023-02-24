// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Config.cpp                                                                  *
*   Methods to read and write configuration                                   *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2023   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#if !defined(WINVER)
#define WINVER 0x601  /*_WIN32_WINNT_WIN7*/
#endif
#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x601  /*_WIN32_WINNT_WIN7*/
#endif
#if !defined(NTDDI_VERSION)
#define NTDDI_VERSION 0x06010000  /*NTDDI_WIN7*/
#endif

#if (defined(_DEBUG) || defined(DEBUG)) && !defined(NDEBUG)
#define DBG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif

#ifndef assert
#include <cassert>
#endif

#define VC_EXTRALEAN 1
#define WIN32_LEAN_AND_MEAN 1
//#define NOMINMAX 1
#include <windows.h>

#include <strsafe.h>
#include <shlobj.h>
#include <shobjidl.h>

// ----------------------------------------------------------------------------

// Scintilla
#include "ILoader.h"

extern "C" {
#include "VersionEx.h"
#include "Helpers.h"
#include "Styles.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Notepad3.h"
#include "MuiLanguage.h"
#include "DynStrg.h"
#include "uthash/utstack.h"
}

#include "DarkMode/DarkMode.h"

extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V1;
extern "C" const WCHAR* const TBBUTTON_DEFAULT_IDS_V2;

extern "C" bool      g_iStatusbarVisible[STATUS_SECTOR_COUNT];
extern "C" int       g_iStatusbarWidthSpec[STATUS_SECTOR_COUNT];
extern "C" int       g_vSBSOrder[STATUS_SECTOR_COUNT];

extern "C" HPATHL    g_tchToolbarBitmap;
extern "C" HPATHL    g_tchToolbarBitmapHot;
extern "C" HPATHL    g_tchToolbarBitmapDisabled;

extern "C"           THEMEFILES Theme_Files[];

// ----------------------------------------------------------------------------

#include "SimpleIni.h"
#include "Config.h"

// ============================================================================

static bool const s_bIsUTF8 = true;
static bool const s_bWriteSIG = true;     // IniFileSetXXX()
static bool const s_bUseMultiKey = false;
static bool const s_bUseMultiLine = true; // find/replace with line breaks
static bool const s_bSetSpaces = false;

// ----------------------------------------------------------------------------

static const WCHAR* const _s_RecentFiles = L"Recent Files";
static const WCHAR* const _s_RecentFind = L"Recent Find";
static const WCHAR* const _s_RecentReplace = L"Recent Replace";

// ----------------------------------------------------------------------------

const WCHAR* const CodeFontPrioList[] = { L"Cascadia Code", L"Cascadia Mono", L"Cousine", L"Fira Code",
    L"Source Code Pro", L"Roboto Mono", L"DejaVu Sans Mono", L"Inconsolata", L"Consolas", L"Lucida Console" };

const WCHAR* const TextFontPrioList[] = { L"Cascadia Mono", L"Cousine", L"Roboto Mono", L"DejaVu Sans Mono",
    L"Inconsolata", L"Consolas", L"Lucida Console" };

WCHAR CodeFontPrioListStrgBuf[LARGE_BUFFER] = { 0 };
WCHAR TextFontPrioListStrgBuf[LARGE_BUFFER] = { 0 };

// ----------------------------------------------------------------------------

static int s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

// ----------------------------------------------------------------------------

constexpr bool SI_Success(const SI_Error rc) noexcept
{
    return ((rc == SI_Error::SI_OK) || (rc == SI_Error::SI_UPDATED) || (rc == SI_Error::SI_INSERTED));
};

// ============================================================================


bool CanAccessPath(const HPATHL hpth, DWORD genericAccessRights)
{
    if (Path_IsEmpty(hpth)) {
        return false;
    }
    DWORD                      length  = 0;
    SECURITY_INFORMATION const secInfo = OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION;

    // check for read-only file attribute
    if (genericAccessRights & GENERIC_WRITE) {
        if (IsReadOnly(Path_GetFileAttributes(hpth))) {
            return false;
        }
    }

    bool bRet = false;
    // check security tokens
    if (!::GetFileSecurityW(Path_Get(hpth), secInfo, NULL, 0, &length) && (ERROR_INSUFFICIENT_BUFFER == GetLastError())) {
        auto const security = static_cast<PSECURITY_DESCRIPTOR>(AllocMem(length, HEAP_ZERO_MEMORY));
        if (security && ::GetFileSecurityW(Path_Get(hpth), secInfo, security, length, &length)) {
            HANDLE hToken = NULL;
            if (::OpenProcessToken(::GetCurrentProcess(), TOKEN_IMPERSONATE | TOKEN_QUERY | TOKEN_DUPLICATE | STANDARD_RIGHTS_READ, &hToken)) {
                HANDLE hImpersonatedToken = NULL;
                if (::DuplicateToken(hToken, SecurityImpersonation, &hImpersonatedToken)) {
                    GENERIC_MAPPING mapping       = {0xFFFFFFFF};
                    PRIVILEGE_SET   privileges    = {0};
                    DWORD           grantedAccess = 0, privilegesLength = sizeof(privileges);
                    BOOL            result = FALSE;

                    mapping.GenericRead    = FILE_GENERIC_READ;
                    mapping.GenericWrite   = FILE_GENERIC_WRITE;
                    mapping.GenericExecute = FILE_GENERIC_EXECUTE;
                    mapping.GenericAll     = FILE_ALL_ACCESS;

                    ::MapGenericMask(&genericAccessRights, &mapping);
                    if (::AccessCheck(security, hImpersonatedToken, genericAccessRights,
                                      &mapping, &privileges, &privilegesLength, &grantedAccess, &result)) {
                        bRet = (result == TRUE);
                    }
                    ::CloseHandle(hImpersonatedToken);
                }
                ::CloseHandle(hToken);
            }
            FreeMem(security);
        }
    }
    return bRet;
}


// ----------------------------------------------------------------------------
// No mechanism for  EXCLUSIVE WRITE / SHARD READ:
// cause we need completely synchronized exclusive access for READ _and_ WRITE
// of complete file to preserve integrity of any transaction
// ----------------------------------------------------------------------------

HANDLE AcquireWriteFileLock(LPCWSTR lpIniFilePath, OVERLAPPED& rOvrLpd)
{
    if (StrIsEmpty(lpIniFilePath)) {
        return INVALID_HANDLE_VALUE;
    }

    bool bLocked = false;

    HANDLE hFile = CreateFile(lpIniFilePath,
                              GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                              OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (IS_VALID_HANDLE(hFile)) {
        bLocked = LockFileEx(hFile, LOCKFILE_EXCLUSIVE_LOCK, 0, MAXDWORD, 0, &rOvrLpd); // wait for exclusive lock
        if (!bLocked) {
            HSTRINGW msg = StrgCreate(NULL);
            StrgFormat(msg, L"AcquireWriteFileLock(%s): NO EXCLUSIVE LOCK ACQUIRED!", lpIniFilePath);
            MsgBoxLastError(StrgGet(msg), 0);
            StrgDestroy(msg);
        }
    } else {
        HSTRINGW msg = StrgCreate(NULL);
        StrgFormat(msg, L"AcquireWriteFileLock(%s): INVALID FILE HANDLE!", lpIniFilePath);
        MsgBoxLastError(StrgGet(msg), 0);
        StrgDestroy(msg);
    }
    return (bLocked ? hFile : INVALID_HANDLE_VALUE);
}

// ----------------------------------------------------------------------------

#define LOCKFILE_SHARED_LOCK (0x00000000)

HANDLE AcquireReadFileLock(LPCWSTR lpIniFilePath, OVERLAPPED& rOvrLpd)
{
    if (StrIsEmpty(lpIniFilePath)) {
        return INVALID_HANDLE_VALUE;
    }

    bool bLocked = false;

    HANDLE hFile = CreateFile(lpIniFilePath,
                              GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (IS_VALID_HANDLE(hFile)) {
        bLocked = LockFileEx(hFile, LOCKFILE_SHARED_LOCK, 0, MAXDWORD, 0, &rOvrLpd);
        if (!bLocked) {
            HSTRINGW msg = StrgCreate(NULL);
            StrgFormat(msg, L"AcquireReadFileLock(%s): NO READER LOCK ACQUIRED!", lpIniFilePath);
            MsgBoxLastError(StrgGet(msg), 0);
            StrgDestroy(msg);
        }
    } else {
        HSTRINGW msg = StrgCreate(NULL);
        StrgFormat(msg, L"AcquireReadFileLock(%s): INVALID FILE HANDLE!", lpIniFilePath);
        MsgBoxLastError(StrgGet(msg), 0);
        StrgDestroy(msg);
    }
    return (bLocked ? hFile : INVALID_HANDLE_VALUE);
}

// ----------------------------------------------------------------------------

bool ReleaseFileLock(HANDLE hFile, OVERLAPPED& rOvrLpd)
{
    bool bUnLocked = true;
    if (IS_VALID_HANDLE(hFile)) {
        FlushFileBuffers(hFile);
        bUnLocked = !UnlockFileEx(hFile, 0, MAXDWORD, 0, &rOvrLpd);
        CloseHandle(hFile);
    }
    return bUnLocked;
}

// ============================================================================

static CSimpleIni s_TMPINI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);

extern "C" bool CopyToTmpCache(LPCSTR lpIniFileResource) {
    if (StrIsEmptyA(lpIniFileResource)) {
        return false;
    }
    // should be UTF-8 or CP-437
    return SI_Success(s_TMPINI.LoadData(lpIniFileResource));
}

extern "C" size_t TmpCacheGetString(LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpDefault,
    LPWSTR lpReturnedString, size_t cchReturnedString) {
    bool bHasMultiple = false;
    StringCchCopy(lpReturnedString, cchReturnedString,
        s_TMPINI.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
    return StringCchLenW(lpReturnedString, cchReturnedString);
}

extern "C" bool TmpCacheSetString(LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString) {
    SI_Error const rc = s_TMPINI.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
    return SI_Success(rc);
}

extern "C" bool ResetTmpCache() {
    s_TMPINI.Reset();
    return true;
}

// ============================================================================

static bool s_bIniFileCacheLoaded = false;
static CSimpleIni s_INI(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);

extern "C" bool ResetIniFileCache()
{
    s_INI.Reset();
    s_bIniFileCacheLoaded = false;
    return true;
}


extern "C" bool LoadIniFileCache(const HPATHL hpthIniFile)
{
    if (Path_IsEmpty(hpthIniFile) || !Path_IsExistingFile(hpthIniFile)) {
        return false;
    }

    s_INI.SetSpaces(s_bSetSpaces);
    s_INI.SetMultiLine(s_bUseMultiLine);

    OVERLAPPED ovrLpd = { 0 };
    HANDLE     hIniFile = AcquireReadFileLock(Path_Get(hpthIniFile), ovrLpd);

    if (!IS_VALID_HANDLE(hIniFile)) {
        return false;
    }

    s_bIniFileCacheLoaded = SI_Success(s_INI.LoadFile(hIniFile));

    ReleaseFileLock(hIniFile, ovrLpd);

    return s_bIniFileCacheLoaded;
}


extern "C" bool IsIniFileCached()
{
    return s_bIniFileCacheLoaded;
}


extern "C" bool SaveIniFileCache(const HPATHL hpthIniFile)
{
    if (!s_bIniFileCacheLoaded || Path_IsEmpty(hpthIniFile)) {
        return false;
    }

    OVERLAPPED ovrLpd = { 0 };
    HANDLE hIniFile = AcquireWriteFileLock(Path_Get(hpthIniFile), ovrLpd);

    if (!IS_VALID_HANDLE(hIniFile)) {
        return false;
    }

    s_INI.SetSpaces(s_bSetSpaces);
    s_INI.SetMultiLine(s_bUseMultiLine);

    bool const res = SI_Success(s_INI.SaveFile(hIniFile, s_bWriteSIG));

    ReleaseFileLock(hIniFile, ovrLpd);

    return res;
}


//=============================================================================
//
//  OpenSettingsFile()
//

typedef struct iniOpen_t {
    CHAR fctname[80];
    struct iniOpen_t* next;
} iniOpen_t;

static iniOpen_t * s_pOpenStackHead = NULL; /* important- initialize to NULL! */
static bool s_bIFCReadOnly = true;

static inline void pushStackHead(LPCSTR fctname)
{
    auto* pOpenBy = (iniOpen_t*)AllocMem(sizeof(iniOpen_t), HEAP_ZERO_MEMORY);
    if (pOpenBy) {
        StringCchCopyA(pOpenBy->fctname, COUNTOF(pOpenBy->fctname), fctname);
        STACK_PUSH(s_pOpenStackHead, pOpenBy);
    }
}

extern "C" bool OpenSettingsFile(LPCSTR fctname)
{
    if (Path_IsNotEmpty(Paths.IniFile)) {

        Globals.bCanSaveIniFile = CreateIniFile(Paths.IniFile, NULL);

        if (!IsIniFileCached()) {
            ResetIniFileCache();
            LoadIniFileCache(Paths.IniFile);
            s_bIFCReadOnly = true; 
        }
    } else {
        Globals.bCanSaveIniFile = false;
    }
    bool const bCached = IsIniFileCached();
    if (bCached) {
        pushStackHead(fctname);
    }
    return bCached;
}


//=============================================================================
//
//  CloseSettingsFile()
//
static inline void popCheckStackHead(LPCSTR fctname)
{
    UNREFERENCED_PARAMETER(fctname); // NDEBUG mode
    iniOpen_t* pOpenBy = NULL;
    STACK_POP(s_pOpenStackHead, pOpenBy);
    assert(StringCchCompareXA(fctname, pOpenBy->fctname) == 0);
    FreeMem(pOpenBy);
}

extern "C" bool CloseSettingsFile(LPCSTR fctname, bool bSaveSettings)
{
    UNREFERENCED_PARAMETER(fctname);

    if (!IsIniFileCached()) {
        assert(STACK_EMPTY(s_pOpenStackHead));
        return false;
    }
    assert(!STACK_EMPTY(s_pOpenStackHead));
    popCheckStackHead(fctname);

    if (Globals.bCanSaveIniFile) {

        if (bSaveSettings) {
            s_bIFCReadOnly = false;
        }

        bool bSaved = false;
        if (STACK_EMPTY(s_pOpenStackHead)) {
            if (!s_bIFCReadOnly) {
                bSaved = SaveIniFileCache(Paths.IniFile);
            }
            ResetIniFileCache();
        }
        return bSaved;
    }
    assert(STACK_EMPTY(s_pOpenStackHead));
    return false;
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
    StringCchCopy(lpReturnedString, cchReturnedString,
                   s_INI.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
    return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" size_t IniSectionGetStringNoQuotes(LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpDefault,
                                              LPWSTR lpReturnedString, size_t cchReturnedString)
{
    bool bHasMultiple = false;
    StringCchCopy(lpReturnedString, cchReturnedString,
                   s_INI.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
    //assert(!bHasMultiple);
    StrTrim(lpReturnedString, L"\"'");
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


extern "C" long IniSectionGetLong(LPCWSTR lpSectionName, LPCWSTR lpKeyName, long lDefault)
{
    bool bHasMultiple = false;
    auto const lValue = s_INI.GetLongValue(lpSectionName, lpKeyName, lDefault, &bHasMultiple);
    //assert(!bHasMultiple);
    return lValue;
}
// ============================================================================


extern "C" long long IniSectionGetLongLong(LPCWSTR lpSectionName, LPCWSTR lpKeyName, long long llDefault)
{
    bool bHasMultiple = false;
    auto const lValue = s_INI.GetLongLongValue(lpSectionName, lpKeyName, llDefault, &bHasMultiple);
    //assert(!bHasMultiple);
    return lValue;
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
    return SI_Success(rc);
}
// ============================================================================


extern "C" bool IniSectionSetInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
    SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
    return SI_Success(rc);
}

extern "C" bool IniSectionSetLong(LPCWSTR lpSectionName, LPCWSTR lpKeyName, long lValue)
{
    SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, lValue, nullptr, false, !s_bUseMultiKey);
    return SI_Success(rc);
}

extern "C" bool IniSectionSetLongLong(LPCWSTR lpSectionName, LPCWSTR lpKeyName, long long llValue)
{
    SI_Error const rc = s_INI.SetLongLongValue(lpSectionName, lpKeyName, llValue, nullptr, false, !s_bUseMultiKey);
    return SI_Success(rc);
}

extern "C" bool IniSectionSetHex(LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
    SI_Error const rc = s_INI.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, true, !s_bUseMultiKey);
    return SI_Success(rc);
}
// ============================================================================


extern "C" bool IniSectionSetDouble(LPCWSTR lpSectionName, LPCWSTR lpKeyName, double dValue)
{
    SI_Error const rc = s_INI.SetDoubleValue(lpSectionName, lpKeyName, dValue, nullptr, !s_bUseMultiKey);
    return SI_Success(rc);
}
// ============================================================================


extern "C" bool IniSectionSetBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
    SI_Error const rc = s_INI.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
    return SI_Success(rc);
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
        return SI_Success(rc);
    }
    return ok;
}
// ============================================================================

extern "C" bool IniClearAllSections(LPCWSTR lpPrefix, bool bRemoveEmpty)
{
    size_t const len = StringCchLen(lpPrefix, 0);

    CSimpleIni::TNamesDepend Sections;
    s_INI.GetAllSections(Sections);
    for (const auto& section : Sections) {
        if (StringCchCompareNI(section.pItem, len, lpPrefix, len) == 0) {
            IniSectionClear(section.pItem, bRemoveEmpty);
        }
    }
    return true;
}
// ============================================================================


// ============================================================================
// ============================================================================


extern "C" size_t IniFileGetString(const HPATHL hpthIniFile, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpDefault,
                                   LPWSTR lpReturnedString, size_t cchReturnedString)
{
    if (Path_IsEmpty(hpthIniFile)) {
        StringCchCopy(lpReturnedString, cchReturnedString, lpDefault);
        return StringCchLenW(lpReturnedString, cchReturnedString);
    }

    CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);

    OVERLAPPED ovrLpd = { 0 };
    HANDLE     hFile = AcquireReadFileLock(Path_Get(hpthIniFile), ovrLpd);
    if (!IS_VALID_HANDLE(hFile)) {
        StringCchCopy(lpReturnedString, cchReturnedString, lpDefault);
        return StringCchLenW(lpReturnedString, cchReturnedString);
    }

    SI_Error const rc = Ini.LoadFile(hFile);
    ReleaseFileLock(hFile, ovrLpd);

    if (SI_Success(rc)) {
        bool bHasMultiple = false;
        StringCchCopy(lpReturnedString, cchReturnedString, Ini.GetValue(lpSectionName, lpKeyName, lpDefault, &bHasMultiple));
        //assert(!bHasMultiple);
    } else {
        StringCchCopy(lpReturnedString, cchReturnedString, lpDefault);
    }
    return StringCchLenW(lpReturnedString, cchReturnedString);
}
// ============================================================================


extern "C" bool IniFileSetString(const HPATHL hpthIniFile, LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
    if (Path_IsEmpty(hpthIniFile)) {
        return false;
    }

    CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
    Ini.SetSpaces(s_bSetSpaces);

    OVERLAPPED ovrLpd = { 0 };
    HANDLE     hFile = AcquireWriteFileLock(Path_Get(hpthIniFile), ovrLpd);
    if (!IS_VALID_HANDLE(hFile)) {
        return false;
    }

    SI_Error rc = Ini.LoadFile(hFile);
    if (SI_Success(rc)) {
        SI_Error const res = Ini.SetValue(lpSectionName, lpKeyName, lpString, nullptr, !s_bUseMultiKey);
        rc = SI_Success(res) ? SI_Error::SI_OK : SI_Error::SI_FAIL;
        if (SI_Success(rc)) {
            rc = Ini.SaveFile(hFile, s_bWriteSIG);
        }
    }
    ReleaseFileLock(hFile, ovrLpd);

    return SI_Success(rc);
}
// ============================================================================


extern "C" int IniFileGetInt(const HPATHL hpthIniFile, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iDefault)
{
    if (Path_IsEmpty(hpthIniFile)) {
        return iDefault;
    }

    CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);

    OVERLAPPED ovrLpd = { 0 };
    HANDLE     hFile = AcquireReadFileLock(Path_Get(hpthIniFile), ovrLpd);
    if (!IS_VALID_HANDLE(hFile)) {
        return iDefault;
    }

    SI_Error rc = Ini.LoadFile(hFile);
    ReleaseFileLock(hFile, ovrLpd);

    if (SI_Success(rc)) {
        bool bHasMultiple = false;
        int const iValue = Ini.GetLongValue(lpSectionName, lpKeyName, (long)iDefault, &bHasMultiple);
        //assert(!bHasMultiple);
        return iValue;
    }
    return iDefault;
}
// ============================================================================


extern "C" bool IniFileSetInt(const HPATHL hpthIniFile, LPCWSTR lpSectionName, LPCWSTR lpKeyName, int iValue)
{
    if (Path_IsEmpty(hpthIniFile)) {
        return false;
    }

    CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
    Ini.SetSpaces(s_bSetSpaces);

    OVERLAPPED ovrLpd = { 0 };
    HANDLE     hFile = AcquireWriteFileLock(Path_Get(hpthIniFile), ovrLpd);
    if (!IS_VALID_HANDLE(hFile)) {
        return false;
    }

    SI_Error rc = Ini.LoadFile(hFile);
    if (SI_Success(rc)) {
        Ini.SetLongValue(lpSectionName, lpKeyName, (long)iValue, nullptr, false, !s_bUseMultiKey);
        rc = Ini.SaveFile(hFile, s_bWriteSIG);
    }
    ReleaseFileLock(hFile, ovrLpd);

    return SI_Success(rc);
}
// ============================================================================


extern "C" bool IniFileGetBool(const HPATHL hpthIniFile, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bDefault)
{
    if (Path_IsEmpty(hpthIniFile)) {
        return bDefault;
    }

    CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);

    OVERLAPPED ovrLpd = { 0 };
    HANDLE     hFile = AcquireReadFileLock(Path_Get(hpthIniFile), ovrLpd);
    if (!IS_VALID_HANDLE(hFile)) {
        return bDefault;
    }

    SI_Error rc = Ini.LoadFile(hFile);
    ReleaseFileLock(hFile, ovrLpd);

    if (SI_Success(rc)) {
        bool bHasMultiple = false;
        bool const bValue = Ini.GetBoolValue(lpSectionName, lpKeyName, bDefault, &bHasMultiple);
        //assert(!bHasMultiple);
        return bValue;
    }
    return bDefault;
}
// ============================================================================


extern "C" bool IniFileSetBool(const HPATHL hpthIniFile, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bValue)
{
    if (Path_IsEmpty(hpthIniFile)) {
        return false;
    }

    CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
    Ini.SetSpaces(s_bSetSpaces);

    OVERLAPPED ovrLpd = { 0 };
    HANDLE     hFile = AcquireWriteFileLock(Path_Get(hpthIniFile), ovrLpd);
    if (!IS_VALID_HANDLE(hFile)) {
        return false;
    }

    SI_Error rc = Ini.LoadFile(hFile);
    if (SI_Success(rc)) {
        Ini.SetBoolValue(lpSectionName, lpKeyName, bValue, nullptr, !s_bUseMultiKey);
        rc = Ini.SaveFile(hFile, s_bWriteSIG);
    }
    ReleaseFileLock(hFile, ovrLpd);

    return SI_Success(rc);
}
// ============================================================================


extern "C" bool IniFileDelete(const HPATHL hpthIniFile, LPCWSTR lpSectionName, LPCWSTR lpKeyName, bool bRemoveEmpty)
{
    if (Path_IsEmpty(hpthIniFile)) {
        return false;
    }

    CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);
    Ini.SetSpaces(s_bSetSpaces);

    OVERLAPPED ovrLpd = { 0 };
    HANDLE     hFile = AcquireWriteFileLock(Path_Get(hpthIniFile), ovrLpd);
    if (!IS_VALID_HANDLE(hFile)) {
        return false;
    }

    SI_Error rc = Ini.LoadFile(hFile);
    if (SI_Success(rc)) {
        Ini.Delete(lpSectionName, lpKeyName, bRemoveEmpty);
        rc = Ini.SaveFile(hFile, s_bWriteSIG);
    }
    ReleaseFileLock(hFile, ovrLpd);

    return SI_Success(rc);
}
// ============================================================================


extern "C" bool IniFileIterateSection(const HPATHL hpthIniFile, LPCWSTR lpSectionName, IterSectionFunc_t callBack)
{
    if (Path_IsEmpty(hpthIniFile)) {
        return false;
    }

    CSimpleIni Ini(s_bIsUTF8, s_bUseMultiKey, s_bUseMultiLine);

    OVERLAPPED ovrLpd = { 0 };
    HANDLE     hFile = AcquireReadFileLock(Path_Get(hpthIniFile), ovrLpd);
    if (!IS_VALID_HANDLE(hFile)) {
        return false;
    }

    SI_Error rc = Ini.LoadFile(hFile);
    ReleaseFileLock(hFile, ovrLpd);

    if (SI_Success(rc)) {
        bool bHasMultiple = false;

        // get all keys in a section
        CSimpleIniW::TNamesDepend keyList;
        Ini.GetAllKeys(lpSectionName, keyList);
        keyList.sort(CSimpleIniW::Entry::LoadOrder());

        for (const auto& key : keyList) {
            callBack(key.pItem, Ini.GetValue(lpSectionName, key.pItem, L"", &bHasMultiple));
        }
    }
    return SI_Success(rc);
}
// ============================================================================



//=============================================================================
//
//  AddFilePathToRecentDocs()
//
extern "C" void AddFilePathToRecentDocs(const HPATHL hpthFile)
{
    if (Path_IsEmpty(hpthFile)) {
        return;
    }
    HPATHL const hpth = Path_Copy(hpthFile);
    Path_CanonicalizeEx(hpth, NULL);

    if (Flags.ShellUseSystemMRU) {
#if TRUE
        SHAddToRecentDocs(SHARD_PATHW, Path_Get(hpth));
#else
        (void)CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_SPEED_OVER_MEMORY | COINIT_DISABLE_OLE1DDE);

        IShellItem* pShellItem = NULL;
        HRESULT const hr = SHCreateItemFromParsingName(szFilePath, NULL, IID_PPV_ARGS(&pShellItem));

        if (SUCCEEDED(hr)) {
            SHARDAPPIDINFO info;
            info.psi = pShellItem;
            info.pszAppID = Settings2.AppUserModelID;  // our AppID - see above
            SHAddToRecentDocs(SHARD_APPIDINFO, &info);
            pShellItem->Release();
        }
        CoUninitialize();
#endif
    }
    Path_Release(hpth);
}


#if 0
//=============================================================================
//
//  ClearDestinationsOnRecentDocs()
//
extern "C" void ClearDestinationsOnRecentDocs()
{
    (void)CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_SPEED_OVER_MEMORY | COINIT_DISABLE_OLE1DDE);

    IApplicationDestinations* pDestinations = NULL;
    HRESULT hr = CoCreateInstance(CLSID_ApplicationDestinations, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDestinations));

    if (SUCCEEDED(hr)) {
        hr = pDestinations->SetAppID(Settings2.AppUserModelID);
        if (SUCCEEDED(hr)) {
            pDestinations->RemoveAllDestinations();
        }
        pDestinations->Release();
    }
    CoUninitialize();
}
#endif


//=============================================================================
//
//  _CheckAndSetIniFile()
//
static bool _CheckAndSetIniFile(HPATHL hpth_in_out)
{
    HPATHL hPathEx = Path_Copy(hpth_in_out);
    Path_ExpandEnvStrings(hPathEx);

    bool result = false;
    if (Path_IsRelative(hPathEx)) {
        //Path_RelativeToApp(hPathEx, true, false, true); // already expanded
        Path_RelativeToApp(hPathEx, false, false, true);
        result = Path_IsExistingFile(hPathEx);
    }
    else if (Path_IsExistingFile(hPathEx)) {
        result = true;
    }

    // ---  Alternate Search Paths  ---

    const wchar_t* wchMore = Path_IsNotEmpty(hpth_in_out) ? Path_FindFileName(hpth_in_out) : SAPPNAME L".ini";

    if (!result) {
        // sub directory (.\np3\)
        HPATHL hmodpth = Path_Allocate(NULL);
        Path_GetAppDirectory(hmodpth);
        Path_Append(hmodpth, L"./np3/");
        Path_Append(hmodpth, wchMore);
        result = Path_IsExistingFile(hmodpth);
        if (result) {
            Path_Swap(hPathEx, hmodpth);
            result = true;
        }
        Path_Release(hmodpth);
    }

    if (!result) {
        // Application Data (%APPDATA%)
        HPATHL happdata = Path_Allocate(NULL);
        if (Path_GetKnownFolder(FOLDERID_RoamingAppData, happdata)) {
            Path_Append(happdata, wchMore);
            result = Path_IsExistingFile(happdata);
            if (result) {
                Path_Swap(hPathEx, happdata);
                result = true;
            }
        }
        Path_Release(happdata);
    }

    if (!result) {
        // Home (%HOMEPATH%) user's profile dir
        HPATHL hprofile = Path_Allocate(NULL);
        if (Path_GetKnownFolder(FOLDERID_Profile, hprofile)) {
            Path_Append(hprofile, wchMore);
            result = Path_IsExistingFile(hprofile);
            if (result) {
                Path_Swap(hPathEx, hprofile);
                result = true;
            }
        }
        Path_Release(hprofile);
    }

#if 0
    if (!result) {
        // in general search path
        Path_Reset(hPathEx, Path_IsNotEmpty(hpth_in_out) ? Path_FindFileName(hpth_in_out) : SAPPNAME L".ini");
        Path_ExpandEnvStrings(hPathEx);
        HPATHL hsearchpth = Path_Allocate(NULL);
        LPWSTR const buf = Path_WriteAccessBuf(hsearchpth, PATHLONG_MAX_CCH);
        if (SearchPathW(NULL, Path_Get(hPathEx), L".ini", PATHLONG_MAX_CCH, buf, NULL)) {
            Path_Sanitize(hsearchpth);
            Path_FreeExtra(hsearchpth, MAX_PATH_EXPLICIT);
            Path_Swap(hPathEx, hsearchpth);
            result = true;
        }
        Path_Release(hsearchpth);
    }
#endif
    
    if (result) {
        Path_Swap(hpth_in_out, hPathEx);
    }

    Path_Release(hPathEx);
    return result;
}
// ============================================================================


static bool _HandleIniFileRedirect(LPCWSTR lpszSecName, LPCWSTR lpszKeyName, HPATHL hpth_in_out)
{
    bool result = false;
    if (Path_IsExistingFile(hpth_in_out)) {
        HPATHL hredirect = Path_Allocate(NULL);
        LPWSTR const buf = Path_WriteAccessBuf(hredirect, PATHLONG_MAX_CCH);
        if (IniFileGetString(hpth_in_out, lpszSecName, lpszKeyName, L"", buf, PATHLONG_MAX_CCH)) {
            Path_Sanitize(hredirect);
            Path_FreeExtra(hredirect, 0);
            if (_CheckAndSetIniFile(hredirect)) {
                Path_Swap(hpth_in_out, hredirect);
            }
            else {
                Path_CanonicalizeEx(hpth_in_out, Paths.ModuleDirectory);
            }
            result = true;
        }
        Path_Release(hredirect);
    }
    return result;
}
// ============================================================================


extern "C" bool FindIniFile()
{
    bool bFound = false;

    HPATHL hdir_pth = Path_Allocate(NULL);
    Path_GetAppDirectory(hdir_pth);
    SetEnvironmentVariableW(NOTEPAD3_MODULE_DIR_ENV_VAR, Path_Get(hdir_pth));
    Path_Release(hdir_pth);

    if (Path_IsNotEmpty(Paths.IniFile)) {
        if (wcscmp(Path_Get(Paths.IniFile), L"*?") == 0) {
            return bFound;
        }
        Path_CanonicalizeEx(Paths.IniFile, Paths.ModuleDirectory);
        bFound = _CheckAndSetIniFile(Paths.IniFile);
    } else {
        // Notepad3.ini
        Path_GetModuleFilePath(Paths.IniFile);
        Path_RenameExtension(Paths.IniFile, L".ini");
        bFound = _CheckAndSetIniFile(Paths.IniFile);

        if (!bFound) {
            Path_GetAppDirectory(Paths.IniFile);
            Path_Append(Paths.IniFile, _W(SAPPNAME) L".ini");
            bFound = _CheckAndSetIniFile(Paths.IniFile);
        }

        if (bFound) {
            // allow two redirections: administrator -> user -> custom
            // 1st:
            if (_HandleIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", Paths.IniFile)) {
                // 2nd:
                _HandleIniFileRedirect(_W(SAPPNAME), _W(SAPPNAME) L".ini", Paths.IniFile);  
                bFound = _CheckAndSetIniFile(Paths.IniFile);
            }

        } else { // force default name

            Path_GetModuleFilePath(Paths.IniFile);
            Path_RenameExtension(Paths.IniFile, L".ini");
        }
    }

    Path_NormalizeEx(Paths.IniFile, Paths.ModuleDirectory, true, false);

    return bFound;
}
//=============================================================================


extern "C" bool TestIniFile()
{
    if (wcscmp(Path_Get(Paths.IniFile), L"*?") == 0) {
        Path_Empty(Paths.IniFileDefault, false);
        Path_Empty(Paths.IniFile, false);
        return false;
    }

    Path_NormalizeEx(Paths.IniFile, Paths.ModuleDirectory, true, false);

    if (!Path_IsExistingFile(Paths.IniFile)) {
        Path_Reset(Paths.IniFileDefault, Path_Get(Paths.IniFile));
        Path_Empty(Paths.IniFile, false);
        return false;
    }

    return true;
}
//=============================================================================


extern "C" bool CreateIniFile(const HPATHL hini_pth, DWORD* pdwFileSize_out)
{
    if (Path_IsNotEmpty(hini_pth)) {

        HPATHL hdir_path = Path_Copy(hini_pth);
        Path_RemoveFileSpec(hdir_path);
        if (Path_IsNotEmpty(hdir_path)) {
            CreateDirectoryW(Path_Get(hdir_path), NULL);
        }
        Path_Release(hdir_path);

        DWORD dwFileSize = 0UL;

        if (!Path_IsExistingFile(hini_pth)) {
            HANDLE hFile = CreateFileW(Path_Get(hini_pth),
                                       GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                                       CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

            if (IS_VALID_HANDLE(hFile)) {
                CloseHandle(hFile); // done
            } else {
                WCHAR fileName[MAX_PATH_EXPLICIT>>1] = { L'\0' };
                Path_GetDisplayName(fileName, COUNTOF(fileName), hini_pth, NULL, true);
                HSTRINGW msg = StrgCreate(NULL);
                StrgFormat(msg, L"CreateIniFile(%s): FAILED TO CREATE INITIAL INI FILE!", fileName);
                MsgBoxLastError(StrgGet(msg), 0);
                StrgDestroy(msg);
            }
        } else {
            HANDLE hFile = CreateFileW(Path_Get(hini_pth),
                                       GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

            if (IS_VALID_HANDLE(hFile)) {
                DWORD dwFSHigh = 0UL;
                dwFileSize = GetFileSize(hFile, &dwFSHigh);
                CloseHandle(hFile);
            } else {
                WCHAR fileName[MAX_PATH_EXPLICIT>>1] = { L'\0' };
                Path_GetDisplayName(fileName, COUNTOF(fileName), hini_pth, NULL, true);
                HSTRINGW msg = StrgCreate(NULL);
                StrgFormat(msg, L"CreateIniFile(%s): FAILED TO READ FILESIZE!", fileName);
                MsgBoxLastError(StrgGet(msg), 0);
                StrgDestroy(msg);
                dwFileSize = INVALID_FILE_SIZE;
            }
        }
        if (pdwFileSize_out) {
            *pdwFileSize_out = dwFileSize;
        }

        return CanAccessPath(Paths.IniFile, GENERIC_WRITE);
    }
    return false;
}
//=============================================================================


//=============================================================================
//
//  LoadSettings()
//
void LoadSettings()
{
    WCHAR tchKeyName[MIDSZ_BUFFER] = { L'\0' };

    CFG_VERSION const _ver = Path_IsEmpty(Paths.IniFile) ? CFG_VER_CURRENT : CFG_VER_NONE;

    auto* const pPathBuffer = (wchar_t*)AllocMem(PATHLONG_MAX_CCH * sizeof(wchar_t), HEAP_ZERO_MEMORY);

    bool bDirtyFlag = false; // do we have to save the file on done

    OpenSettingsFile(__func__);

    // --------------------------------------------------------------------------
    const WCHAR* const IniSecSettings = Constants.Settings_Section;
    const WCHAR* const IniSecSettings2 = Constants.Settings2_Section;
    // --------------------------------------------------------------------------

    // prerequisites
    Globals.iCfgVersionRead = IniSectionGetInt(IniSecSettings, L"SettingsVersion", _ver);

    Defaults.SaveSettings = Path_IsNotEmpty(Paths.IniFile);
    Settings.SaveSettings = Defaults.SaveSettings && IniSectionGetBool(IniSecSettings, L"SaveSettings", Defaults.SaveSettings);

    // ---  first set "hard coded" .ini-Settings  ---

    Flags.bDevDebugMode = IniSectionGetBool(IniSecSettings2, L"DevDebugMode", DefaultFlags.bDevDebugMode);
    Flags.bStickyWindowPosition = IniSectionGetBool(IniSecSettings2, L"StickyWindowPosition", DefaultFlags.bStickyWindowPosition);

    if (Globals.CmdLnFlag_ReuseWindow == 0) {
        Flags.bReuseWindow = IniSectionGetBool(IniSecSettings2, L"ReuseWindow", DefaultFlags.bReuseWindow);
    }
    else {
        Flags.bReuseWindow = (Globals.CmdLnFlag_ReuseWindow == 2);
    }

    if (Globals.CmdLnFlag_SingleFileInstance == 0) {
        Flags.bSingleFileInstance = IniSectionGetBool(IniSecSettings2, L"SingleFileInstance", DefaultFlags.bSingleFileInstance);
    }
    else {
        Flags.bSingleFileInstance = (Globals.CmdLnFlag_SingleFileInstance == 2);
    }

    if (Globals.CmdLnFlag_MultiFileArg == 0) {
        Flags.MultiFileArg = IniSectionGetBool(IniSecSettings2, L"MultiFileArg", DefaultFlags.MultiFileArg);
    }
    else {
        Flags.MultiFileArg = (Globals.CmdLnFlag_MultiFileArg == 2);
    }

    if (Globals.CmdLnFlag_ShellUseSystemMRU == 0) {
        Flags.ShellUseSystemMRU = IniSectionGetBool(IniSecSettings2, L"ShellUseSystemMRU", DefaultFlags.ShellUseSystemMRU);
    }
    else {
        Flags.ShellUseSystemMRU = (Globals.CmdLnFlag_ShellUseSystemMRU == 2);
    }

    Flags.RelativeFileMRU = IniSectionGetBool(IniSecSettings2, L"RelativeFileMRU", DefaultFlags.RelativeFileMRU);
    Flags.PortableMyDocs = IniSectionGetBool(IniSecSettings2, L"PortableMyDocs", DefaultFlags.PortableMyDocs);
    Flags.NoFadeHidden = IniSectionGetBool(IniSecSettings2, L"NoFadeHidden", DefaultFlags.NoFadeHidden);

    Flags.ToolbarLook = IniSectionGetInt(IniSecSettings2, L"ToolbarLook", DefaultFlags.ToolbarLook);
    Flags.ToolbarLook = clampi(Flags.ToolbarLook, 0, 2);

    Flags.SimpleIndentGuides = IniSectionGetBool(IniSecSettings2, L"SimpleIndentGuides", DefaultFlags.SimpleIndentGuides);
    Flags.NoHTMLGuess = IniSectionGetBool(IniSecSettings2, L"NoHTMLGuess", DefaultFlags.NoHTMLGuess);
    Flags.NoCGIGuess = IniSectionGetBool(IniSecSettings2, L"NoCGIGuess", DefaultFlags.NoCGIGuess);
    Flags.NoFileVariables = IniSectionGetInt(IniSecSettings2, L"NoFileVariables", DefaultFlags.NoFileVariables);

    Flags.PrintFileAndLeave = Globals.CmdLnFlag_PrintFileAndLeave;

    // --------------------------------------------------------------------------

#if defined(HAVE_DYN_LOAD_LIBS_MUI_LNGS)

    Default_PreferredLanguageLocaleName[0] = L'\0';
    GetUserPreferredLanguage(Default_PreferredLanguageLocaleName, LOCALE_NAME_MAX_LENGTH);

    IniSectionGetStringNoQuotes(IniSecSettings2, L"PreferredLanguageLocaleName", Default_PreferredLanguageLocaleName,
        Settings2.PreferredLanguageLocaleName, COUNTOF(Settings2.PreferredLanguageLocaleName));
#endif

    // --------------------------------------------------------------------------

    IniSectionGetStringNoQuotes(IniSecSettings2, L"DefaultExtension", L"txt",
        Settings2.DefaultExtension, COUNTOF(Settings2.DefaultExtension));
    StrTrim(Settings2.DefaultExtension, L" \t.");

    IniSectionGetStringNoQuotes(IniSecSettings2, L"DefaultDirectory", L"", pPathBuffer, PATHLONG_MAX_CCH);
    Path_Reset(Settings2.DefaultDirectory, pPathBuffer);
    Path_ExpandEnvStrings(Settings2.DefaultDirectory);

    IniSectionGetStringNoQuotes(IniSecSettings2, L"FileDlgFilters", L"", pPathBuffer, XHUGE_BUFFER);
    StrgReset(Settings2.FileDlgFilters, pPathBuffer);

    Settings2.FileCheckInverval = clampul(IniSectionGetInt(IniSecSettings2, L"FileCheckInverval", 0), 0, 86400000 << 2); // max: 48h
    // handle deprecated old "AutoReloadTimeout"
    int const          autoReload = IniSectionGetInt(IniSecSettings2, L"AutoReloadTimeout", -1); // deprecated
    unsigned int const fci = max_u(250, (autoReload > 0) ? max_u(autoReload, Settings2.FileCheckInverval) : Settings2.FileCheckInverval);
    if ((Settings2.FileCheckInverval > 0) && (fci != Settings2.FileCheckInverval)) {
        Settings2.FileCheckInverval = fci;
        IniSectionSetInt(IniSecSettings2, L"FileCheckInverval", Settings2.FileCheckInverval);
        bDirtyFlag = true;
    }
    FileWatching.FileCheckInverval = Settings2.FileCheckInverval;

    IniSectionGetString(IniSecSettings2, L"FileChangedIndicator", L"[@]", Settings2.FileChangedIndicator, COUNTOF(Settings2.FileChangedIndicator));

    IniSectionGetString(IniSecSettings2, L"FileDeletedIndicator", L"[X]", Settings2.FileDeletedIndicator, COUNTOF(Settings2.FileDeletedIndicator));

    Settings2.UndoTransactionTimeout = clampul(IniSectionGetInt(IniSecSettings2, L"UndoTransactionTimeout", 0UL), 0UL, 86400000UL);

    // Settings2 SciDirectWriteTech deprecated
    Defaults.RenderingTechnology = IniSectionGetInt(IniSecSettings2, L"SciDirectWriteTech", -111);
    if (Defaults.RenderingTechnology != -111) {
        if (Settings.SaveSettings) {
            // cleanup
            IniSectionDelete(IniSecSettings2, L"SciDirectWriteTech", false); // old deprecated
            bDirtyFlag = true;
        }
        Defaults.RenderingTechnology = clampi(Defaults.RenderingTechnology, SC_TECHNOLOGY_DEFAULT, SC_TECHNOLOGY_DIRECTWRITEDC);
    }
    else {
        Defaults.RenderingTechnology = SC_TECHNOLOGY_DIRECTWRITE; // new default DirectWrite (D2D)
    }

    // Settings2 EnableBidirectionalSupport deprecated
    Defaults.Bidirectional = IniSectionGetInt(IniSecSettings2, L"EnableBidirectionalSupport", -111);
    if ((Defaults.Bidirectional != -111) && Settings.SaveSettings) {
        // cleanup
        IniSectionDelete(IniSecSettings2, L"EnableBidirectionalSupport", false);
        bDirtyFlag = true;
    }
    Defaults.Bidirectional = (clampi(Defaults.Bidirectional, SC_BIDIRECTIONAL_DISABLED, SC_BIDIRECTIONAL_R2L) > 0) ? SC_BIDIRECTIONAL_R2L : SC_BIDIRECTIONAL_DISABLED;

    Settings2.IMEInteraction = clampi(IniSectionGetInt(IniSecSettings2, L"IMEInteraction", -1), -1, SC_IME_INLINE);
    // Korean IME use inline mode by default
    if (Settings2.IMEInteraction == -1) { // auto detection once
        // ScintillaWin::KoreanIME()
        int const codePage = Scintilla_InputCodePage();
        Settings2.IMEInteraction = ((codePage == 949 || codePage == 1361) ? SC_IME_INLINE : SC_IME_WINDOWED);
    }

    Settings2.LaunchInstanceWndPosOffset = clampi(IniSectionGetInt(IniSecSettings2, L"LaunchInstanceWndPosOffset", 28), -10000, 10000);
    Settings2.LaunchInstanceFullVisible = IniSectionGetBool(IniSecSettings2, L"LaunchInstanceFullVisible", true);

    Settings2.SciFontQuality = clampi(IniSectionGetInt(IniSecSettings2, L"SciFontQuality", SC_EFF_QUALITY_LCD_OPTIMIZED), SC_EFF_QUALITY_DEFAULT, SC_EFF_QUALITY_LCD_OPTIMIZED);

    Settings2.UpdateDelayMarkAllOccurrences = clampi(IniSectionGetInt(IniSecSettings2, L"UpdateDelayMarkAllOccurrences", USER_TIMER_MINIMUM << 2), (USER_TIMER_MINIMUM << 1), 10000);

    Settings2.DenyVirtualSpaceAccess = IniSectionGetBool(IniSecSettings2, L"DenyVirtualSpaceAccess", false);

    Settings2.UseOldStyleBraceMatching = IniSectionGetBool(IniSecSettings2, L"UseOldStyleBraceMatching", false);

    Settings2.CurrentLineHorizontalSlop = clampi(IniSectionGetInt(IniSecSettings2, L"CurrentLineHorizontalSlop", 40), 0, 240);

    Settings2.CurrentLineVerticalSlop = clampi(IniSectionGetInt(IniSecSettings2, L"CurrentLineVerticalSlop", 5), 0, 100);

    Settings2.NoCopyLineOnEmptySelection = IniSectionGetBool(IniSecSettings2, L"NoCopyLineOnEmptySelection", false);

    Settings2.NoCutLineOnEmptySelection = IniSectionGetBool(IniSecSettings2, L"NoCutLineOnEmptySelection", false);

    Settings2.AnalyzeReliableConfidenceLevel = (float)clampi(IniSectionGetInt(IniSecSettings2, L"AnalyzeReliableConfidenceLevel", 90), 0, 100) / 100.0f;

    int const iAnsiCPBonusSet = clampi(IniSectionGetInt(IniSecSettings2, L"LocaleAnsiCodePageAnalysisBonus", 33), 0, 100);
    Settings2.LocaleAnsiCodePageAnalysisBonus = (float)iAnsiCPBonusSet / 100.0f;

    Settings2.FileLoadWarningMB = clampi(IniSectionGetInt(IniSecSettings2, L"FileLoadWarningMB", 4), 0, 2048);

    Settings2.OpacityLevel = clampi(IniSectionGetInt(IniSecSettings2, L"OpacityLevel", 75), 10, 100);

    Settings2.DarkModeHiglightContrast = (float)clampi(IniSectionGetInt(IniSecSettings2, L"DarkModeHiglightContrast", 75), 0, 1000) / 100.0f;

    Settings2.FindReplaceOpacityLevel = clampi(IniSectionGetInt(IniSecSettings2, L"FindReplaceOpacityLevel", 50), 10, 100);

    IniSectionGetStringNoQuotes(IniSecSettings2, L"filebrowser.exe", L"", pPathBuffer, PATHLONG_MAX_CCH);
    Path_Reset(Settings2.FileBrowserPath, pPathBuffer);
    Path_ExpandEnvStrings(Settings2.FileBrowserPath);

    IniSectionGetStringNoQuotes(IniSecSettings2, L"grepWin.exe", L"", pPathBuffer, PATHLONG_MAX_CCH);
    Path_Reset(Settings2.GrepWinPath, pPathBuffer);
    Path_ExpandEnvStrings(Settings2.GrepWinPath);

    IniSectionGetStringNoQuotes(IniSecSettings2, L"AdministrationTool.exe", L"", pPathBuffer, PATHLONG_MAX_CCH);
    Path_Reset(Settings2.AdministrationTool, pPathBuffer);
    Path_ExpandEnvStrings(Settings2.AdministrationTool);

    if (StrIsEmpty(Settings2.AppUserModelID)) { // set via CmdLine ?
        IniSectionGetString(IniSecSettings2, L"ShellAppUserModelID", _W("Rizonesoft." SAPPNAME), Settings2.AppUserModelID, COUNTOF(Settings2.AppUserModelID));
    }
    IniSectionGetString(IniSecSettings2, L"ExtendedWhiteSpaceChars", L"",
        Settings2.ExtendedWhiteSpaceChars, COUNTOF(Settings2.ExtendedWhiteSpaceChars));

    IniSectionGetString(IniSecSettings2, L"AutoCompleteWordCharSet", L"", Settings2.AutoCompleteWordCharSet, COUNTOF(Settings2.AutoCompleteWordCharSet));

    IniSectionGetString(IniSecSettings2, L"AutoCompleteFillUpChars", L"", Settings2.AutoCompleteFillUpChars, COUNTOF(Settings2.AutoCompleteFillUpChars));

    IniSectionGetString(IniSecSettings2, L"LineCommentPostfixStrg", L"", Settings2.LineCommentPostfixStrg, COUNTOF(Settings2.LineCommentPostfixStrg));
    StrTrim(Settings2.LineCommentPostfixStrg, L"\"'");

    IniSectionGetStringNoQuotes(IniSecSettings2, L"DateTimeFormat", L"", Settings2.DateTimeFormat, COUNTOF(Settings2.DateTimeFormat));

    IniSectionGetStringNoQuotes(IniSecSettings2, L"DateTimeLongFormat", L"", Settings2.DateTimeLongFormat, COUNTOF(Settings2.DateTimeLongFormat));

    IniSectionGetStringNoQuotes(IniSecSettings2, L"TimeStampRegEx", L"\\$Date:[^\\$]+\\$", Settings2.TimeStampRegEx, COUNTOF(Settings2.TimeStampRegEx));

    IniSectionGetStringNoQuotes(IniSecSettings2, L"TimeStampFormat", L"$Date: %s $", Settings2.TimeStampFormat, COUNTOF(Settings2.TimeStampFormat));

    IniSectionGetStringNoQuotes(IniSecSettings2, L"WebTemplate1", L"https://google.com/search?q=%s", pPathBuffer, PATHLONG_MAX_CCH);
    StrgReset(Settings2.WebTemplate1, pPathBuffer);
    //~GetMenuStringW(Globals.hCtxMenu, CMD_WEBACTION1, WebTmpl1MenuName, COUNTOF(WebTmpl1MenuName), MF_BYCOMMAND))
    IniSectionGetStringNoQuotes(IniSecSettings2, L"WebTmpl1MenuName", L"", Settings2.WebTmpl1MenuName, COUNTOF(Settings2.WebTmpl1MenuName));

    IniSectionGetStringNoQuotes(IniSecSettings2, L"WebTemplate2", L"https://en.wikipedia.org/w/index.php?search=%s", pPathBuffer, PATHLONG_MAX_CCH);
    StrgReset(Settings2.WebTemplate2, pPathBuffer);
    //~GetMenuStringW(Globals.hMainMenu, CMD_WEBACTION2, WebTmpl2MenuName, COUNTOF(WebTmpl2MenuName), MF_BYCOMMAND))
    IniSectionGetStringNoQuotes(IniSecSettings2, L"WebTmpl2MenuName", L"", Settings2.WebTmpl2MenuName, COUNTOF(Settings2.WebTmpl2MenuName));

    Settings2.LexerSQLNumberSignAsComment = IniSectionGetBool(IniSecSettings2, L"LexerSQLNumberSignAsComment", true);

    Settings2.ExitOnESCSkipLevel = clampi(IniSectionGetInt(IniSecSettings2, L"ExitOnESCSkipLevel", Default_ExitOnESCSkipLevel), 0, 2);

    Settings2.ZoomTooltipTimeout = clampi(IniSectionGetInt(IniSecSettings2, L"ZoomTooltipTimeout", 3200), 0, 10000);

    Settings2.WrapAroundTooltipTimeout = clampi(IniSectionGetInt(IniSecSettings2, L"WrapAroundTooltipTimeout", 2000), 0, 10000);

    Settings2.LargeIconScalePrecent = clampi(IniSectionGetInt(IniSecSettings2, L"LargeIconScalePrecent", 150), 100, 1000);

    IniSectionGetStringNoQuotes(IniSecSettings2, L"HyperlinkShellExURLWithApp", L"", pPathBuffer, PATHLONG_MAX_CCH);
    StrgReset(Settings2.HyperlinkShellExURLWithApp, pPathBuffer);
    IniSectionGetStringNoQuotes(IniSecSettings2, L"HyperlinkShellExURLCmdLnArgs", URLPLACEHLDR, pPathBuffer, PATHLONG_MAX_CCH);
    StrgReset(Settings2.HyperlinkShellExURLCmdLnArgs, pPathBuffer);

    const static WCHAR* const allowedVerbs[] = { L"edit", L"explore", L"find", L"open", L"print", L"properties", L"runas" };
    Settings2.HyperlinkFileProtocolVerb[0] = L'\0';
    IniSectionGetStringNoQuotes(IniSecSettings2, L"HyperlinkFileProtocolVerb", L"", tchKeyName, COUNTOF(tchKeyName));
    for (auto allowedVerb : allowedVerbs) {
        if (StrStr(tchKeyName, allowedVerb)) {
            StringCchCopy(Settings2.HyperlinkFileProtocolVerb, COUNTOF(Settings2.HyperlinkFileProtocolVerb), tchKeyName);
            break;
        }
    }

    for (int i = 0; i < COUNTOF(Settings2.CodeFontPrefPrioList); ++i) {
        if (i < COUNTOF(CodeFontPrioList))
            Settings2.CodeFontPrefPrioList[i] = CodeFontPrioList[i];
        else
            Settings2.CodeFontPrefPrioList[i] = nullptr;
    }
    if (IniSectionGetStringNoQuotes(IniSecSettings2, L"CodeFontPrefPrioList", L"", CodeFontPrioListStrgBuf, COUNTOF(CodeFontPrioListStrgBuf))) {
        // split string and assign pointer array
        WCHAR* p = &CodeFontPrioListStrgBuf[0];
        int    i = 0;
        while (p && *p && (i < COUNTOF(Settings2.CodeFontPrefPrioList))) {
            Settings2.CodeFontPrefPrioList[i] = p;
            while (p && *p) {
                if (*p == L',') { *p = L'\0'; ++p; break; }
                ++p;
            }
            ++i;
        }
    }

    for (int i = 0; i < COUNTOF(Settings2.TextFontPrefPrioList); ++i) {
        if (i < COUNTOF(TextFontPrioList))
            Settings2.TextFontPrefPrioList[i] = TextFontPrioList[i];
        else
            Settings2.TextFontPrefPrioList[i] = nullptr;
    }
    if (IniSectionGetStringNoQuotes(IniSecSettings2, L"TextFontPrefPrioList", L"", TextFontPrioListStrgBuf, COUNTOF(TextFontPrioListStrgBuf))) {
        // split string and assign pointer array
        WCHAR* p = &TextFontPrioListStrgBuf[0];
        int    i = 0;
        while (p && *p && (i < COUNTOF(Settings2.TextFontPrefPrioList))) {
            Settings2.TextFontPrefPrioList[i] = p;
            while (p && *p) {
                if (*p == L',') { *p = L'\0'; ++p; break; }
                ++p;
            }
            ++i;
        }
    }

#ifdef D_NP3_WIN10_DARK_MODE

    unsigned int iValue = 0;
    WCHAR color[32] = { L'\0' };

    StringCchPrintf(color, COUNTOF(color), L"%#08x", rgbDarkBkgColorRef);
    IniSectionGetStringNoQuotes(IniSecSettings2, L"DarkModeBkgColor", color, tchKeyName, COUNTOF(tchKeyName));
    if (swscanf_s(tchKeyName, L"%x", &iValue) == 1) {
        Settings2.DarkModeBkgColor = RGB((iValue & 0xFF0000) >> 16, (iValue & 0xFF00) >> 8, iValue & 0xFF);
    } else {
        Settings2.DarkModeBkgColor = rgbDarkBkgColorRef;
    }

    if (Globals.hbrDarkModeBkgBrush) {
        DeleteObject(Globals.hbrDarkModeBkgBrush);
    }
    if (Globals.hbrDarkModeBtnFcBrush) {
        DeleteObject(Globals.hbrDarkModeBtnFcBrush);
    }
    Globals.hbrDarkModeBkgBrush = CreateSolidBrush(Settings2.DarkModeBkgColor);

    StringCchPrintf(color, COUNTOF(color), L"%#08x", rgbDarkBtnFcColorRef);
    IniSectionGetStringNoQuotes(IniSecSettings2, L"DarkModeBtnFaceColor", color, tchKeyName, COUNTOF(tchKeyName));
    if (swscanf_s(tchKeyName, L"%x", &iValue) == 1) {
        Settings2.DarkModeBtnFaceColor = RGB((iValue & 0xFF0000) >> 16, (iValue & 0xFF00) >> 8, iValue & 0xFF);
    } else {
        Settings2.DarkModeBtnFaceColor = rgbDarkBtnFcColorRef;
    }
    Globals.hbrDarkModeBtnFcBrush = CreateSolidBrush(Settings2.DarkModeBtnFaceColor);

    StringCchPrintf(color, COUNTOF(color), L"%#08x", rgbDarkTxtColorRef);
    IniSectionGetStringNoQuotes(IniSecSettings2, L"DarkModeTxtColor", color, tchKeyName, COUNTOF(tchKeyName));
    if (swscanf_s(tchKeyName, L"%x", &iValue) == 1) {
        Settings2.DarkModeTxtColor = RGB((iValue & 0xFF0000) >> 16, (iValue & 0xFF00) >> 8, iValue & 0xFF);
    } else {
        Settings2.DarkModeTxtColor = rgbDarkTxtColorRef;
    }

    Globals.hbrDarkModeBkgHotBrush = CreateSolidBrush(Settings2.DarkModeBtnFaceColor);
    Globals.hbrDarkModeBkgSelBrush = CreateSolidBrush(Settings2.DarkModeBtnFaceColor);

#endif

    // --------------------------------------------------------------------------
    // Settings: IniSecSettings
    // --------------------------------------------------------------------------

#define GET_BOOL_VALUE_FROM_INISECTION(VARNAME, DEFAULT) \
  Defaults.VARNAME = DEFAULT;                            \
  Settings.VARNAME = IniSectionGetBool(IniSecSettings, _W(_STRG(VARNAME)), Defaults.VARNAME)

#define GET_INT_VALUE_FROM_INISECTION(VARNAME, DEFAULT, MIN, MAX) \
  Defaults.VARNAME = DEFAULT;                                     \
  Settings.VARNAME = clampi(IniSectionGetInt(IniSecSettings, _W(_STRG(VARNAME)), Defaults.VARNAME), MIN, MAX)

#define GET_CAST_INT_VALUE_FROM_INISECTION(CAST, VARNAME, DEFAULT, MIN, MAX) \
  Defaults.VARNAME = static_cast<CAST>(DEFAULT);                             \
  Settings.VARNAME = static_cast<CAST>(clampi(IniSectionGetInt(IniSecSettings, _W(_STRG(VARNAME)), Defaults.VARNAME), MIN, MAX))

#define GET_ENC_VALUE_FROM_INISECTION(VARNAME, DEFAULT, MIN, MAX) \
  Defaults.VARNAME = (cpi_enc_t)DEFAULT;                          \
  Settings.VARNAME = (cpi_enc_t)clampi(IniSectionGetInt(IniSecSettings, _W(_STRG(VARNAME)), (int)Defaults.VARNAME), (int)MIN, (int)MAX)

    GET_BOOL_VALUE_FROM_INISECTION(SaveRecentFiles, true);
    GET_BOOL_VALUE_FROM_INISECTION(PreserveCaretPos, false);
    GET_BOOL_VALUE_FROM_INISECTION(SaveFindReplace, false);
    GET_BOOL_VALUE_FROM_INISECTION(AutoLoadMRUFile, false);
    GET_BOOL_VALUE_FROM_INISECTION(DocReadOnlyMode, false);

    Defaults.EFR_Data.bFindClose = false;
    Settings.EFR_Data.bFindClose = IniSectionGetBool(IniSecSettings, L"CloseFind", Defaults.EFR_Data.bFindClose);
    Defaults.EFR_Data.bReplaceClose = false;
    Settings.EFR_Data.bReplaceClose = IniSectionGetBool(IniSecSettings, L"CloseReplace", Defaults.EFR_Data.bReplaceClose);
    Defaults.EFR_Data.bNoFindWrap = false;
    Settings.EFR_Data.bNoFindWrap = IniSectionGetBool(IniSecSettings, L"NoFindWrap", Defaults.EFR_Data.bNoFindWrap);
    Defaults.EFR_Data.bTransformBS = false;
    Settings.EFR_Data.bTransformBS = IniSectionGetBool(IniSecSettings, L"FindTransformBS", Defaults.EFR_Data.bTransformBS);
    Defaults.EFR_Data.bRegExprSearch = false;
    Settings.EFR_Data.bRegExprSearch = IniSectionGetBool(IniSecSettings, L"RegExprSearch", Defaults.EFR_Data.bRegExprSearch);
    Defaults.EFR_Data.bWildcardSearch = false;
    Settings.EFR_Data.bWildcardSearch = IniSectionGetBool(IniSecSettings, L"WildcardSearch", Defaults.EFR_Data.bWildcardSearch);
    Defaults.EFR_Data.bMarkOccurences = true;
    Settings.EFR_Data.bMarkOccurences = IniSectionGetBool(IniSecSettings, L"FindMarkAllOccurrences", Defaults.EFR_Data.bMarkOccurences);
    Defaults.EFR_Data.bHideNonMatchedLines = false;
    Settings.EFR_Data.bHideNonMatchedLines = IniSectionGetBool(IniSecSettings, L"HideNonMatchedLines", Defaults.EFR_Data.bHideNonMatchedLines);
    Defaults.EFR_Data.fuFlags = 0;
    Settings.EFR_Data.fuFlags = (UINT)IniSectionGetInt(IniSecSettings, L"efrData_fuFlags", (int)Defaults.EFR_Data.fuFlags);

    if (Path_GetKnownFolder(FOLDERID_Desktop, Defaults.OpenWithDir)) {
        LPWSTR const wchOpenWithDir = Path_WriteAccessBuf(Settings.OpenWithDir, PATHLONG_MAX_CCH);
        if (IniSectionGetStringNoQuotes(IniSecSettings, L"OpenWithDir", Path_Get(Defaults.OpenWithDir), wchOpenWithDir, PATHLONG_MAX_CCH)) {
            Path_Sanitize(Settings.OpenWithDir);
            Path_FreeExtra(Settings.OpenWithDir, 0);
            Path_AbsoluteFromApp(Settings.OpenWithDir, true);
        }
    }
    else {
        Path_Reset(Settings.OpenWithDir, Path_Get(Paths.ModuleDirectory));
    }

    if (Path_GetKnownFolder(FOLDERID_Favorites, Defaults.FavoritesDir)) {
        LPWSTR const wchFavoritesDir = Path_WriteAccessBuf(Settings.FavoritesDir, PATHLONG_MAX_CCH);
        if (IniSectionGetStringNoQuotes(IniSecSettings, L"Favorites", Path_Get(Defaults.FavoritesDir), wchFavoritesDir, PATHLONG_MAX_CCH)) {
            Path_Sanitize(Settings.FavoritesDir);
            Path_FreeExtra(Settings.FavoritesDir, 0);
            Path_AbsoluteFromApp(Settings.FavoritesDir, true);
        }
    }
    else {
        Path_Reset(Settings.FavoritesDir, Path_Get(Paths.WorkingDirectory));
    }

    GET_INT_VALUE_FROM_INISECTION(PathNameFormat, 1, 0, 2);
    GET_INT_VALUE_FROM_INISECTION(WordWrapMode, 0, 0, 1);
    GET_INT_VALUE_FROM_INISECTION(WordWrapIndent, 0, 0, 6);

    GET_BOOL_VALUE_FROM_INISECTION(WordWrap, true);
    Globals.fvCurFile.bWordWrap = Settings.WordWrap;
    GET_BOOL_VALUE_FROM_INISECTION(TabsAsSpaces, false);
    Globals.fvCurFile.bTabsAsSpaces = Settings.TabsAsSpaces;
    GET_BOOL_VALUE_FROM_INISECTION(TabIndents, true);
    Globals.fvCurFile.bTabIndents = Settings.TabIndents;
    GET_INT_VALUE_FROM_INISECTION(TabWidth, 4, 1, 1024);
    Globals.fvCurFile.iTabWidth = Settings.TabWidth;
    GET_INT_VALUE_FROM_INISECTION(IndentWidth, 4, 0, 1024);
    Globals.fvCurFile.iIndentWidth = Settings.IndentWidth;

    GET_BOOL_VALUE_FROM_INISECTION(MarkLongLines, (Globals.iCfgVersionRead < CFG_VER_0002));
    Defaults.MarkLongLines = false; // new default
    GET_INT_VALUE_FROM_INISECTION(LongLineMode, EDGE_LINE, EDGE_LINE, EDGE_MULTILINE);
    GET_INT_VALUE_FROM_INISECTION(LongLinesLimit, 80, 0, LONG_LINES_MARKER_LIMIT);
    Globals.iWrapCol = Settings.LongLinesLimit;

    _itow_s(Settings.LongLinesLimit, Defaults.MultiEdgeLines, COUNTOF(Defaults.MultiEdgeLines), 10);
    IniSectionGetStringNoQuotes(IniSecSettings, L"MultiEdgeLines", Defaults.MultiEdgeLines, Settings.MultiEdgeLines, COUNTOF(Settings.MultiEdgeLines));
    size_t const n = NormalizeColumnVector(NULL, Settings.MultiEdgeLines, COUNTOF(Settings.MultiEdgeLines));
    StringCchCopy(Globals.fvCurFile.wchMultiEdgeLines, COUNTOF(Globals.fvCurFile.wchMultiEdgeLines), Settings.MultiEdgeLines);
    if (n > 1) {
        Settings.LongLineMode = EDGE_MULTILINE;
    }

    Defaults.WordWrapSymbols = 2;
    int const iWS = IniSectionGetInt(IniSecSettings, L"WordWrapSymbols", Defaults.WordWrapSymbols);
    Settings.WordWrapSymbols = clampi(iWS % 10, 0, 2) + clampi((iWS % 100 - iWS % 10) / 10, 0, 2) * 10;

    GET_BOOL_VALUE_FROM_INISECTION(ShowWordWrapSymbols, true);
    GET_BOOL_VALUE_FROM_INISECTION(MatchBraces, true);
    GET_BOOL_VALUE_FROM_INISECTION(AutoCloseTags, false);
    GET_BOOL_VALUE_FROM_INISECTION(AutoCloseQuotes, false);
    GET_BOOL_VALUE_FROM_INISECTION(AutoCloseBrackets, false);
    GET_INT_VALUE_FROM_INISECTION(HighlightCurrentLine, 1, 0, 2);
    GET_INT_VALUE_FROM_INISECTION(ChangeHistoryMode, SC_CHANGE_HISTORY_ENABLED | SC_CHANGE_HISTORY_MARKERS, 0, 7);
    GET_BOOL_VALUE_FROM_INISECTION(HyperlinkHotspot, true);
    GET_BOOL_VALUE_FROM_INISECTION(ShowHypLnkToolTip, false);
    GET_INT_VALUE_FROM_INISECTION(ColorDefHotspot, 2, 0, 3);
    GET_BOOL_VALUE_FROM_INISECTION(ScrollPastEOF, false);
    GET_BOOL_VALUE_FROM_INISECTION(HighlightUnicodePoints, true);

    GET_BOOL_VALUE_FROM_INISECTION(AutoIndent, true);
    GET_BOOL_VALUE_FROM_INISECTION(AutoCompleteWords, false);
    GET_BOOL_VALUE_FROM_INISECTION(AutoCLexerKeyWords, false);
    GET_BOOL_VALUE_FROM_INISECTION(AccelWordNavigation, false);
    GET_BOOL_VALUE_FROM_INISECTION(ShowIndentGuides, false);
    GET_BOOL_VALUE_FROM_INISECTION(BackspaceUnindents, false);
    GET_BOOL_VALUE_FROM_INISECTION(WarnInconsistentIndents, false);
    GET_BOOL_VALUE_FROM_INISECTION(AutoDetectIndentSettings, false);

    GET_BOOL_VALUE_FROM_INISECTION(ShowBookmarkMargin, IniSectionGetBool(IniSecSettings, L"ShowSelectionMargin", true));
    GET_BOOL_VALUE_FROM_INISECTION(ShowLineNumbers, true);
    GET_BOOL_VALUE_FROM_INISECTION(ShowCodeFolding, true);
    FocusedView.ShowCodeFolding = Settings.ShowCodeFolding;

    GET_BOOL_VALUE_FROM_INISECTION(MarkOccurrences, true);
    GET_BOOL_VALUE_FROM_INISECTION(MarkOccurrencesBookmark, false);
    GET_BOOL_VALUE_FROM_INISECTION(MarkOccurrencesMatchVisible, false);
    GET_BOOL_VALUE_FROM_INISECTION(MarkOccurrencesMatchCase, false);
    GET_BOOL_VALUE_FROM_INISECTION(MarkOccurrencesMatchWholeWords, true);

    Defaults.MarkOccurrencesCurrentWord = !Defaults.MarkOccurrencesMatchWholeWords;
    Settings.MarkOccurrencesCurrentWord = IniSectionGetBool(IniSecSettings, L"MarkOccurrencesCurrentWord", Defaults.MarkOccurrencesCurrentWord);
    Settings.MarkOccurrencesCurrentWord = Settings.MarkOccurrencesCurrentWord && !Settings.MarkOccurrencesMatchWholeWords;

    GET_BOOL_VALUE_FROM_INISECTION(ViewWhiteSpace, false);
    GET_BOOL_VALUE_FROM_INISECTION(ViewEOLs, false);

    auto const iDefaultEncoding = (cpi_enc_t)Encoding_MapIniSetting(false, (int)CPI_PREFERRED_ENCODING);
    GET_ENC_VALUE_FROM_INISECTION(DefaultEncoding, iDefaultEncoding, CPI_NONE, INT_MAX);
    Settings.DefaultEncoding = ((Settings.DefaultEncoding == CPI_NONE) ? CPI_PREFERRED_ENCODING : (cpi_enc_t)Encoding_MapIniSetting(true, (int)Settings.DefaultEncoding));
    Globals.fvCurFile.iEncoding = Settings.DefaultEncoding;

    GET_BOOL_VALUE_FROM_INISECTION(UseDefaultForFileEncoding, false);
    GET_BOOL_VALUE_FROM_INISECTION(LoadASCIIasUTF8, true);
    GET_BOOL_VALUE_FROM_INISECTION(UseReliableCEDonly, true);
    GET_BOOL_VALUE_FROM_INISECTION(LoadNFOasOEM, true);
    GET_BOOL_VALUE_FROM_INISECTION(NoEncodingTags, true);
    GET_BOOL_VALUE_FROM_INISECTION(SkipUnicodeDetection, false);
    GET_BOOL_VALUE_FROM_INISECTION(SkipANSICodePageDetection, false);
    GET_INT_VALUE_FROM_INISECTION(DefaultEOLMode, SC_EOL_CRLF, SC_EOL_CRLF, SC_EOL_LF);
    GET_BOOL_VALUE_FROM_INISECTION(WarnInconsistEOLs, true);
    GET_BOOL_VALUE_FROM_INISECTION(FixLineEndings, false);
    GET_BOOL_VALUE_FROM_INISECTION(FixTrailingBlanks, false);
    GET_INT_VALUE_FROM_INISECTION(PrintHeader, 1, 0, 3);
    GET_INT_VALUE_FROM_INISECTION(PrintFooter, 0, 0, 1);
    GET_INT_VALUE_FROM_INISECTION(PrintColorMode, 3, 0, 4);

    //int const zoomScale = 100;
    int const baseZoom = 100;
    int const prtFontSize = 10;
    int const zoomScale = MulDiv(baseZoom, prtFontSize, f2int(GLOBAL_INITIAL_FONTSIZE));
    Defaults.PrintZoom = (Globals.iCfgVersionRead < CFG_VER_0001) ? (zoomScale / 10) : zoomScale;
    int iPrintZoom = clampi(IniSectionGetInt(IniSecSettings, L"PrintZoom", Defaults.PrintZoom), 0, SC_MAX_ZOOM_LEVEL);
    if (Globals.iCfgVersionRead < CFG_VER_0001) {
        iPrintZoom = 100 + (iPrintZoom - 10) * 10;
    }
    Settings.PrintZoom = clampi(iPrintZoom, SC_MIN_ZOOM_LEVEL, SC_MAX_ZOOM_LEVEL);

    GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_IMEASURE, tchKeyName, COUNTOF(tchKeyName));
    LONG const _margin = (tchKeyName[0] == L'0') ? 2000L : 1000L; // Metric system. L'1' is US System
    Defaults.PrintMargin.left = _margin;
    Settings.PrintMargin.left = clampi(IniSectionGetInt(IniSecSettings, L"PrintMarginLeft", Defaults.PrintMargin.left), 0, 40000);
    Defaults.PrintMargin.top = _margin;
    Settings.PrintMargin.top = clampi(IniSectionGetInt(IniSecSettings, L"PrintMarginTop", Defaults.PrintMargin.top), 0, 40000);
    Defaults.PrintMargin.right = _margin;
    Settings.PrintMargin.right = clampi(IniSectionGetInt(IniSecSettings, L"PrintMarginRight", Defaults.PrintMargin.right), 0, 40000);
    Defaults.PrintMargin.bottom = _margin;
    Settings.PrintMargin.bottom = clampi(IniSectionGetInt(IniSecSettings, L"PrintMarginBottom", Defaults.PrintMargin.bottom), 0, 40000);

    if (Globals.iCfgVersionRead < CFG_VER_0005) {
        int const fwm_mode = IniSectionGetInt(IniSecSettings, L"FileWatchingMode", -1);
        if (fwm_mode > (int)FWM_DONT_CARE) {
            IniSectionSetInt(IniSecSettings, L"FileWatchingMode", fwm_mode + 1);
        }
    }
    GET_CAST_INT_VALUE_FROM_INISECTION(FILE_WATCHING_MODE, FileWatchingMode, FWM_MSGBOX, FWM_DONT_CARE, FWM_EXCLUSIVELOCK);

    GET_INT_VALUE_FROM_INISECTION(AutoSaveInterval, 60000, 2000, 86400000); // 2s - 24h
    GET_CAST_INT_VALUE_FROM_INISECTION(AutoSaveBackupOptions, AutoSaveOptions, ASB_Default, ASB_None, INT_MAX);

    GET_BOOL_VALUE_FROM_INISECTION(SaveBeforeRunningTools, false);
    GET_BOOL_VALUE_FROM_INISECTION(EvalTinyExprOnSelection, true);
    GET_BOOL_VALUE_FROM_INISECTION(ResetFileWatching, true);
    GET_INT_VALUE_FROM_INISECTION(EscFunction, 0, 0, 2);
    GET_BOOL_VALUE_FROM_INISECTION(AlwaysOnTop, false);
    if (Globals.CmdLnFlag_AlwaysOnTop) { Settings.AlwaysOnTop = (Globals.CmdLnFlag_AlwaysOnTop == 2); }
    GET_BOOL_VALUE_FROM_INISECTION(MinimizeToTray, false);
    GET_BOOL_VALUE_FROM_INISECTION(TransparentMode, false);
    GET_BOOL_VALUE_FROM_INISECTION(FindReplaceTransparentMode, true);
    GET_INT_VALUE_FROM_INISECTION(RenderingTechnology, Defaults.RenderingTechnology, SC_TECHNOLOGY_DEFAULT, SC_TECHNOLOGY_DIRECTWRITEDC); // default set before
    Defaults.RenderingTechnology = SC_TECHNOLOGY_DIRECTWRITE;                                                                             // DirectWrite (D2D) - reset, if set by deprecated SciDirectWriteTech
    GET_INT_VALUE_FROM_INISECTION(Bidirectional, Defaults.Bidirectional, SC_BIDIRECTIONAL_DISABLED, SC_BIDIRECTIONAL_R2L);                // set before
    Defaults.Bidirectional = SC_BIDIRECTIONAL_DISABLED;                                                                                   // reset
    GET_BOOL_VALUE_FROM_INISECTION(MuteMessageBeep, false);
    GET_BOOL_VALUE_FROM_INISECTION(SplitUndoTypingSeqOnLnBreak, true);
    GET_BOOL_VALUE_FROM_INISECTION(EditLayoutRTL, false);
    GET_BOOL_VALUE_FROM_INISECTION(DialogsLayoutRTL, false);
    GET_BOOL_VALUE_FROM_INISECTION(PreferredLocale4DateFmt, false);
    GET_BOOL_VALUE_FROM_INISECTION(ReplaceByClipboardTag, true);

#ifdef D_NP3_WIN10_DARK_MODE
    Defaults.WinThemeDarkMode = ShouldAppsUseDarkModeEx();
    Settings.WinThemeDarkMode = IniSectionGetBool(IniSecSettings, L"WinThemeDarkMode", Defaults.WinThemeDarkMode) && IsDarkModeSupported();
#endif

    ///~Settings2.IMEInteraction = clampi(IniSectionGetInt(IniSecSettings, L"IMEInteraction", Settings2.IMEInteraction), SC_IME_WINDOWED, SC_IME_INLINE);

    // see TBBUTTON  s_tbbMainWnd[] for initial/reset set of buttons
    StringCchCopy(Defaults.ToolbarButtons, COUNTOF(Defaults.ToolbarButtons), (Globals.iCfgVersionRead < CFG_VER_0002) ? TBBUTTON_DEFAULT_IDS_V1 : TBBUTTON_DEFAULT_IDS_V2);
    IniSectionGetStringNoQuotes(IniSecSettings, L"ToolbarButtons", Defaults.ToolbarButtons, Settings.ToolbarButtons, COUNTOF(Settings.ToolbarButtons));

    GET_BOOL_VALUE_FROM_INISECTION(ShowMenubar, true);
    GET_BOOL_VALUE_FROM_INISECTION(ShowToolbar, true);
    GET_BOOL_VALUE_FROM_INISECTION(ShowStatusbar, true);

    GET_INT_VALUE_FROM_INISECTION(EncodingDlgSizeX, 340, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(EncodingDlgSizeY, 292, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(RecodeDlgSizeX, 340, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(RecodeDlgSizeY, 292, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(FileMRUDlgSizeX, 487, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(FileMRUDlgSizeY, 377, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(OpenWithDlgSizeX, 305, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(OpenWithDlgSizeY, 281, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(FavoritesDlgSizeX, 305, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(FavoritesDlgSizeY, 281, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(AddToFavDlgSizeX, 317, INT_MIN, INT_MAX);

    GET_INT_VALUE_FROM_INISECTION(FindReplaceDlgPosX, CW_USEDEFAULT, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(FindReplaceDlgPosY, CW_USEDEFAULT, INT_MIN, INT_MAX);

    GET_INT_VALUE_FROM_INISECTION(CustomSchemesDlgPosX, CW_USEDEFAULT, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(CustomSchemesDlgPosY, CW_USEDEFAULT, INT_MIN, INT_MAX);

    GET_INT_VALUE_FROM_INISECTION(FocusViewMarkerMode, FVMM_FOLD, FVMM_MARGIN, (FVMM_LN_BACKGR | FVMM_FOLD));
    Settings.FocusViewMarkerMode = (Settings.FocusViewMarkerMode == (FVMM_MARGIN | FVMM_LN_BACKGR) ? FVMM_FOLD : Settings.FocusViewMarkerMode);


    // --------------------------------------------------------------------------
    const WCHAR *const StatusBar_Section = L"Statusbar Settings";
    // --------------------------------------------------------------------------

    IniSectionGetStringNoQuotes(StatusBar_Section, L"VisibleSections", STATUSBAR_DEFAULT_IDS, tchKeyName, COUNTOF(tchKeyName));
    ReadVectorFromString(tchKeyName, s_iStatusbarSections, STATUS_SECTOR_COUNT, 0, (STATUS_SECTOR_COUNT - 1), -1, false);

    // cppcheck-suppress useStlAlgorithm
    for (bool &sbv : g_iStatusbarVisible) {
        sbv = false;
    }
    int cnt = 0;
    for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
        g_vSBSOrder[i] = -1;
        int const id = s_iStatusbarSections[i];
        if (id >= 0) {
            g_vSBSOrder[cnt++] = id;
            g_iStatusbarVisible[id] = true;
        }
    }

    IniSectionGetStringNoQuotes(StatusBar_Section, L"SectionWidthSpecs", STATUSBAR_SECTION_WIDTH_SPECS, tchKeyName, COUNTOF(tchKeyName));
    ReadVectorFromString(tchKeyName, g_iStatusbarWidthSpec, STATUS_SECTOR_COUNT, -4096, 4096, 0, false);

    Globals.bZeroBasedColumnIndex = IniSectionGetBool(StatusBar_Section, L"ZeroBasedColumnIndex", false);
    Globals.bZeroBasedCharacterCount = IniSectionGetBool(StatusBar_Section, L"ZeroBasedCharacterCount", false);

    // --------------------------------------------------------------------------
    const WCHAR *const ToolbarImg_Section = L"Toolbar Images";
    // --------------------------------------------------------------------------

    IniSectionGetStringNoQuotes(ToolbarImg_Section, L"BitmapDefault", L"", pPathBuffer, PATHLONG_MAX_CCH);
    Path_Reset(g_tchToolbarBitmap, pPathBuffer);
    IniSectionGetStringNoQuotes(ToolbarImg_Section, L"BitmapHot", L"", pPathBuffer, PATHLONG_MAX_CCH);
    Path_Reset(g_tchToolbarBitmapHot, pPathBuffer);
    IniSectionGetStringNoQuotes(ToolbarImg_Section, L"BitmapDisabled", L"", pPathBuffer, PATHLONG_MAX_CCH);
    Path_Reset(g_tchToolbarBitmapDisabled, pPathBuffer);

    // --------------------------------------------------------------------------
    const WCHAR *const IniSecWindow = Constants.Window_Section;
    // --------------------------------------------------------------------------

    int const ResX = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int const ResY = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    WCHAR tchHighDpiToolBar[64] = { L'\0' };
    StringCchPrintf(tchHighDpiToolBar, COUNTOF(tchHighDpiToolBar), L"%ix%i HighDpiToolBar", ResX, ResY);

    Defaults.ToolBarTheme = -1;
    Settings.ToolBarTheme = IniSectionGetInt(IniSecWindow, tchHighDpiToolBar, Defaults.ToolBarTheme);
    Settings.ToolBarTheme = clampi(Settings.ToolBarTheme, -1, Path_IsNotEmpty(g_tchToolbarBitmap) ? 2 : 1);

    StringCchPrintf(tchHighDpiToolBar, COUNTOF(tchHighDpiToolBar), L"%ix%i DpiScaleToolBar", ResX, ResY);
    Defaults.DpiScaleToolBar = false;
    Settings.DpiScaleToolBar = IniSectionGetBool(IniSecWindow, tchHighDpiToolBar, Defaults.DpiScaleToolBar);

    // --------------------------------------------------------------
    // startup window  (ignore window position if /p was specified)
    // --------------------------------------------------------------

    StringCchPrintf(tchKeyName, COUNTOF(tchKeyName), L"%ix%i " DEF_WIN_POSITION_STRG, ResX, ResY);
    if (!IniSectionGetString(IniSecWindow, tchKeyName, L"",
                             Settings2.DefaultWindowPosition, COUNTOF(Settings2.DefaultWindowPosition))) {
        IniSectionGetString(IniSecSettings2, DEF_WIN_POSITION_STRG, L"",
            Settings2.DefaultWindowPosition, COUNTOF(Settings2.DefaultWindowPosition));
    }

    if (!Globals.CmdLnFlag_PosParam /*|| g_bStickyWinPos*/) {

        WININFO winInfo = INIT_WININFO;
        WCHAR tchPosX[64], tchPosY[64], tchSizeX[64], tchSizeY[64], tchMaximized[64], tchZoom[64], tchDPI[64];
        StringCchPrintf(tchPosX, COUNTOF(tchPosX), L"%ix%i PosX", ResX, ResY);
        StringCchPrintf(tchPosY, COUNTOF(tchPosY), L"%ix%i PosY", ResX, ResY);
        StringCchPrintf(tchSizeX, COUNTOF(tchSizeX), L"%ix%i SizeX", ResX, ResY);
        StringCchPrintf(tchSizeY, COUNTOF(tchSizeY), L"%ix%i SizeY", ResX, ResY);
        StringCchPrintf(tchMaximized, COUNTOF(tchMaximized), L"%ix%i Maximized", ResX, ResY);
        StringCchPrintf(tchZoom, COUNTOF(tchZoom), L"%ix%i Zoom", ResX, ResY);
        StringCchPrintf(tchDPI, COUNTOF(tchDPI), L"%ix%i DPI", ResX, ResY);

        winInfo.x = IniSectionGetInt(IniSecWindow, tchPosX, CW_USEDEFAULT);
        winInfo.y = IniSectionGetInt(IniSecWindow, tchPosY, CW_USEDEFAULT);
        winInfo.cx = IniSectionGetInt(IniSecWindow, tchSizeX, CW_USEDEFAULT);
        winInfo.cy = IniSectionGetInt(IniSecWindow, tchSizeY, CW_USEDEFAULT);
        winInfo.max = IniSectionGetBool(IniSecWindow, tchMaximized, false);
        winInfo.zoom = IniSectionGetInt(IniSecWindow, tchZoom, (Globals.iCfgVersionRead < CFG_VER_0001) ? 0 : 100);
        if (Globals.iCfgVersionRead < CFG_VER_0001) {
            winInfo.zoom = (winInfo.zoom + 10) * 10;
        }
        winInfo.zoom = clampi(winInfo.zoom, SC_MIN_ZOOM_LEVEL, SC_MAX_ZOOM_LEVEL);
        winInfo.dpi = IniSectionGetInt(IniSecWindow, tchDPI, USER_DEFAULT_SCREEN_DPI);

        int const offset = Settings2.LaunchInstanceWndPosOffset;
        int const instCnt = CountRunningInstances();
        winInfo.x += (instCnt * offset);
        winInfo.y += (instCnt * offset);

        if ((winInfo.x == CW_USEDEFAULT) || (winInfo.y == CW_USEDEFAULT) ||
                (winInfo.cx == CW_USEDEFAULT) || (winInfo.cy == CW_USEDEFAULT)) {
            Globals.CmdLnFlag_WindowPos = 2; // std. default position (CmdLn: /pd)
        } else {
            g_IniWinInfo = winInfo;
            Globals.CmdLnFlag_WindowPos = 0; // init to g_IniWinInfo
        }
    }

    // ------------------------------------------------------------------------

    // ---  override by resolution specific settings  ---
    StringCchPrintf(tchKeyName, COUNTOF(tchKeyName), L"%ix%i RenderingTechnology", ResX, ResY);
    Settings.RenderingTechnology = clampi(IniSectionGetInt(IniSecWindow, tchKeyName, Settings.RenderingTechnology), 0, 3);

    StringCchPrintf(tchKeyName, COUNTOF(tchKeyName), L"%ix%i SciFontQuality", ResX, ResY);
    Settings2.SciFontQuality = clampi(IniSectionGetInt(IniSecWindow, tchKeyName, Settings2.SciFontQuality), SC_EFF_QUALITY_DEFAULT, SC_EFF_QUALITY_LCD_OPTIMIZED);

    // ------------------------------------------------------------------------

    // define scintilla internal codepage
    //int const iSciDefaultCodePage = SC_CP_UTF8; // default UTF8

    // set flag for encoding default
    Encoding_SetDefaultFlag(Settings.DefaultEncoding);

    // define default charset
    Globals.iDefaultCharSet = SC_CHARSET_DEFAULT;

    // File MRU
    Globals.pFileMRU = MRU_Create(_s_RecentFiles, MRU_NOCASE, MRU_ITEMSFILE);
    MRU_Load(Globals.pFileMRU, true);

    Globals.pMRUfind = MRU_Create(_s_RecentFind, (/*IsWindowsNT()*/ true) ? MRU_UTF8 : 0, MRU_ITEMSFNDRPL);
    MRU_Load(Globals.pMRUfind, false);
    if (IsFindPatternEmpty()) {
        SetFindPattern(Globals.pMRUfind->pszItems[0]);
    }

    Globals.pMRUreplace = MRU_Create(_s_RecentReplace, (/*IsWindowsNT()*/ true) ? MRU_UTF8 : 0, MRU_ITEMSFNDRPL);
    MRU_Load(Globals.pMRUreplace, false);

    // --------------------------------------------------------------------------
    const WCHAR *const IniSecStyles = Constants.Styles_Section;
    // --------------------------------------------------------------------------
    IniSectionGetString(IniSecStyles, L"ThemeFileName", L"", Settings.CurrentThemeName, COUNTOF(Settings.CurrentThemeName));
    
    Style_Prerequisites();

    CloseSettingsFile(__func__, bDirtyFlag);

    FreeMem(pPathBuffer);
}
//=============================================================================



//=============================================================================
//
//  _SaveSettings()
//

#define SAVE_VALUE_IF_NOT_EQ_DEFAULT(TYPE, VARNAME)                               \
  if (Settings.VARNAME != Defaults.VARNAME) {                                     \
    IniSectionSet##TYPE(IniSecSettings, _W(_STRG(VARNAME)), Settings.VARNAME);    \
  }                                                                               \
  else {                                                                          \
    IniSectionDelete(IniSecSettings, _W(_STRG(VARNAME)), false);                  \
  }

#define SAVE_VALUE2_IF_NOT_EQ_DEFAULT2(TYPE, VARNAME)                             \
  if (Settings2.VARNAME != Defaults2.VARNAME) {                                   \
    IniSectionSet##TYPE(IniSecSettings2, _W(_STRG(VARNAME)), Settings.VARNAME);   \
  }                                                                               \
  else {                                                                          \
    IniSectionDelete(IniSecSettings2, _W(_STRG(VARNAME)), false);                 \
  }

static bool _SaveSettings(bool bForceSaveSettings)
{
    if (!IsIniFileCached()) {
        return false;
    }

    // --------------------------------------------------------------------------
    const WCHAR* const IniSecSettings = Constants.Settings_Section;
    // --------------------------------------------------------------------------

    // ---  remove deprecated  ---
    IniSectionDelete(IniSecSettings, L"MarkOccurrencesMaxCount", false);


    if (!(Settings.SaveSettings || bForceSaveSettings)) {
        if (Settings.SaveSettings != Defaults.SaveSettings) {
            IniSectionSetBool(IniSecSettings, L"SaveSettings", Settings.SaveSettings);
        } else {
            IniSectionDelete(IniSecSettings, L"SaveSettings", false);
        }
        return true;
    }

    IniSectionSetInt(IniSecSettings, L"SettingsVersion", CFG_VER_CURRENT);  // new settings

    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, SaveSettings);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, SaveRecentFiles);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, PreserveCaretPos);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, SaveFindReplace);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, AutoLoadMRUFile);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, DocReadOnlyMode);

    if (Settings.EFR_Data.bFindClose != Defaults.EFR_Data.bFindClose) {
        IniSectionSetBool(IniSecSettings, L"CloseFind", Settings.EFR_Data.bFindClose);
    } else {
        IniSectionDelete(IniSecSettings, L"CloseFind", false);
    }
    if (Settings.EFR_Data.bReplaceClose != Defaults.EFR_Data.bReplaceClose) {
        IniSectionSetBool(IniSecSettings, L"CloseReplace", Settings.EFR_Data.bReplaceClose);
    } else {
        IniSectionDelete(IniSecSettings, L"CloseReplace", false);
    }
    if (Settings.EFR_Data.bNoFindWrap != Defaults.EFR_Data.bNoFindWrap) {
        IniSectionSetBool(IniSecSettings, L"NoFindWrap", Settings.EFR_Data.bNoFindWrap);
    } else {
        IniSectionDelete(IniSecSettings, L"NoFindWrap", false);
    }
    if (Settings.EFR_Data.bTransformBS != Defaults.EFR_Data.bTransformBS) {
        IniSectionSetBool(IniSecSettings, L"FindTransformBS", Settings.EFR_Data.bTransformBS);
    } else {
        IniSectionDelete(IniSecSettings, L"FindTransformBS", false);
    }
    if (Settings.EFR_Data.bRegExprSearch != Defaults.EFR_Data.bRegExprSearch) {
        IniSectionSetBool(IniSecSettings, L"RegExprSearch", Settings.EFR_Data.bRegExprSearch);
    } else {
        IniSectionDelete(IniSecSettings, L"RegExprSearch", false);
    }
    if (Settings.EFR_Data.bWildcardSearch != Defaults.EFR_Data.bWildcardSearch) {
        IniSectionSetBool(IniSecSettings, L"WildcardSearch", Settings.EFR_Data.bWildcardSearch);
    } else {
        IniSectionDelete(IniSecSettings, L"WildcardSearch", false);
    }
    if (Settings.EFR_Data.bMarkOccurences != Defaults.EFR_Data.bMarkOccurences) {
        IniSectionSetBool(IniSecSettings, L"FindMarkAllOccurrences", Settings.EFR_Data.bMarkOccurences);
    } else {
        IniSectionDelete(IniSecSettings, L"FindMarkAllOccurrences", false);
    }
    if (Settings.EFR_Data.bHideNonMatchedLines != Defaults.EFR_Data.bHideNonMatchedLines) {
        IniSectionSetBool(IniSecSettings, L"HideNonMatchedLines", Settings.EFR_Data.bHideNonMatchedLines);
    } else {
        IniSectionDelete(IniSecSettings, L"HideNonMatchedLines", false);
    }
    if (Settings.EFR_Data.fuFlags != Defaults.EFR_Data.fuFlags) {
        IniSectionSetInt(IniSecSettings, L"efrData_fuFlags", Settings.EFR_Data.fuFlags);
    } else {
        IniSectionDelete(IniSecSettings, L"efrData_fuFlags", false);
    }

    HPATHL hpth = Path_Allocate(NULL);
    if (StringCchCompareXI(Path_Get(Settings.OpenWithDir), Path_Get(Defaults.OpenWithDir)) != 0) {
        Path_Reset(hpth, Path_Get(Settings.OpenWithDir));
        Path_RelativeToApp(hpth, false, true, Flags.PortableMyDocs);
        IniSectionSetString(IniSecSettings, L"OpenWithDir", Path_Get(hpth));
    } else {
        IniSectionDelete(IniSecSettings, L"OpenWithDir", false);
    }
    if (StringCchCompareXI(Path_Get(Settings.FavoritesDir), Path_Get(Defaults.FavoritesDir)) != 0) {
        Path_Reset(hpth, Path_Get(Settings.FavoritesDir));
        Path_RelativeToApp(hpth, false, true, Flags.PortableMyDocs);
        IniSectionSetString(IniSecSettings, L"Favorites", Path_Get(hpth));
    } else {
        IniSectionDelete(IniSecSettings, L"Favorites", false);
    }
    Path_Release(hpth);

    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, PathNameFormat);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, WordWrap);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, TabsAsSpaces);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, TabIndents);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, TabWidth);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, IndentWidth);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, BackspaceUnindents);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, WordWrapMode);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, WordWrapIndent);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, WordWrapSymbols);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ShowWordWrapSymbols);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MatchBraces);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, AutoCloseTags);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, AutoCloseQuotes);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, AutoCloseBrackets);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, HighlightCurrentLine);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, ChangeHistoryMode);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, HyperlinkHotspot);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, ColorDefHotspot);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ScrollPastEOF);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ShowHypLnkToolTip);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, HighlightUnicodePoints);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, AutoIndent);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, AutoCompleteWords);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, AutoCLexerKeyWords);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, AccelWordNavigation);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ShowIndentGuides);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, WarnInconsistentIndents);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, AutoDetectIndentSettings);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MarkLongLines);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int,  LongLineMode);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int,  LongLinesLimit);
    if (StringCchCompareX(Settings.MultiEdgeLines, Defaults.MultiEdgeLines) != 0) {
        if (StrIsNotEmpty(Settings.MultiEdgeLines)) {
            IniSectionSetString(IniSecSettings, L"MultiEdgeLines", Settings.MultiEdgeLines);
        } else {
            IniSectionDelete(IniSecSettings, L"MultiEdgeLines", false);
        }
    } else {
        IniSectionDelete(IniSecSettings, L"MultiEdgeLines", false);
    }
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ShowBookmarkMargin);
    IniSectionDelete(IniSecSettings, L"ShowSelectionMargin", false); // old

    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ShowLineNumbers);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ShowCodeFolding);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MarkOccurrences);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MarkOccurrencesBookmark);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MarkOccurrencesMatchVisible);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MarkOccurrencesMatchCase);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MarkOccurrencesMatchWholeWords);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MarkOccurrencesCurrentWord);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ViewWhiteSpace);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ViewEOLs);

    // encoding: internal<->external mapping
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
        IniSectionSetInt(IniSecSettings, L"PrintMarginLeft", Settings.PrintMargin.left);
    } else {
        IniSectionDelete(IniSecSettings, L"PrintMarginLeft", false);
    }
    if (Settings.PrintMargin.top != Defaults.PrintMargin.top) {
        IniSectionSetInt(IniSecSettings, L"PrintMarginTop", Settings.PrintMargin.top);
    } else {
        IniSectionDelete(IniSecSettings, L"PrintMarginTop", false);
    }
    if (Settings.PrintMargin.right != Defaults.PrintMargin.right) {
        IniSectionSetInt(IniSecSettings, L"PrintMarginRight", Settings.PrintMargin.right);
    } else {
        IniSectionDelete(IniSecSettings, L"PrintMarginRight", false);
    }
    if (Settings.PrintMargin.bottom != Defaults.PrintMargin.bottom) {
        IniSectionSetInt(IniSecSettings, L"PrintMarginBottom", Settings.PrintMargin.bottom);
    } else {
        IniSectionDelete(IniSecSettings, L"PrintMarginBottom", false);
    }

    if (bForceSaveSettings) { Settings.FileWatchingMode = FileWatching.FileWatchingMode; }
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, FileWatchingMode);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ResetFileWatching);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, AutoSaveInterval);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, AutoSaveOptions);

    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, SaveBeforeRunningTools);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, EvalTinyExprOnSelection);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, EscFunction);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, AlwaysOnTop);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MinimizeToTray);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, TransparentMode);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, FindReplaceTransparentMode);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, RenderingTechnology);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, Bidirectional);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MuteMessageBeep);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, SplitUndoTypingSeqOnLnBreak);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, EditLayoutRTL);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, DialogsLayoutRTL);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, PreferredLocale4DateFmt);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ReplaceByClipboardTag);

#ifdef D_NP3_WIN10_DARK_MODE
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, WinThemeDarkMode);
#endif

    ///~IniSectionSetInt(IniSecSettings, L"IMEInteraction", Settings2.IMEInteraction);

    Toolbar_GetButtons(Globals.hwndToolbar, IDT_FILE_NEW, Settings.ToolbarButtons, COUNTOF(Settings.ToolbarButtons));
    if (StringCchCompareX(Settings.ToolbarButtons, Defaults.ToolbarButtons) != 0) {
        if (StrIsNotEmpty(Settings.ToolbarButtons)) {
            IniSectionSetString(IniSecSettings, L"ToolbarButtons", Settings.ToolbarButtons);
        } else {
            IniSectionDelete(IniSecSettings, L"ToolbarButtons", false);
        }
    } else {
        IniSectionDelete(IniSecSettings, L"ToolbarButtons", false);
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

    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, FindReplaceDlgPosX);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, FindReplaceDlgPosY);

    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, CustomSchemesDlgPosX);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, CustomSchemesDlgPosY);

    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, FocusViewMarkerMode);

    // --------------------------------------------------------------------------
    const WCHAR* const IniSecSettings2 = Constants.Settings2_Section;
    // --------------------------------------------------------------------------

    // ---  remove deprecated  ---
    IniSectionDelete(IniSecSettings2, L"MarkOccurrencesMaxCount", false);
    IniSectionDelete(IniSecSettings2, L"AutoReloadTimeout", false);


    // --------------------------------------------------------------------------
    const WCHAR* const IniSecWindow = Constants.Window_Section;
    // --------------------------------------------------------------------------

    int const ResX = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int const ResY = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    WCHAR tchHighDpiToolBar[64];
    StringCchPrintf(tchHighDpiToolBar, COUNTOF(tchHighDpiToolBar), L"%ix%i HighDpiToolBar", ResX, ResY);
    if (Settings.ToolBarTheme != Defaults.ToolBarTheme) {
        IniSectionSetInt(IniSecWindow, tchHighDpiToolBar, Settings.ToolBarTheme);
    } else {
        IniSectionDelete(IniSecWindow, tchHighDpiToolBar, false);
    }

    StringCchPrintf(tchHighDpiToolBar, COUNTOF(tchHighDpiToolBar), L"%ix%i DpiScaleToolBar", ResX, ResY);
    if (Settings.DpiScaleToolBar != Defaults.DpiScaleToolBar) {
        IniSectionSetBool(IniSecWindow, tchHighDpiToolBar, Settings.DpiScaleToolBar);
    } else {
        IniSectionDelete(IniSecWindow, tchHighDpiToolBar, false);
    }

    if (!Flags.bStickyWindowPosition) {
        SaveWindowPositionSettings(false);
    }


    // --------------------------------------------------------------------------
    const WCHAR* const IniSecStyles = Constants.Styles_Section;
    // --------------------------------------------------------------------------

    if (Globals.uCurrentThemeIndex == 0) {
        IniSectionDelete(IniSecStyles, L"ThemeFileName", false);
        Style_ToIniSection(false); // Scintilla Styles
        Style_FileExtToIniSection(false);
    } else {
        IniSectionSetString(IniSecStyles, L"ThemeFileName", Settings.CurrentThemeName);
    }

    return true;
}


//=============================================================================
//
//  SaveWindowPositionSettings()
//
bool SaveWindowPositionSettings(bool bClearSettings)
{
    if (!IsIniFileCached()) {
        return false;
    }

    // set current window position as ne initial window
    WININFO const winInfo = GetMyWindowPlacement(Globals.hwndMain, NULL, 0);

    int const ResX = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int const ResY = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    WCHAR tchPosX[64], tchPosY[64], tchSizeX[64], tchSizeY[64], tchMaximized[64], tchZoom[64], tchDPI[64];
    StringCchPrintf(tchPosX, COUNTOF(tchPosX), L"%ix%i PosX", ResX, ResY);
    StringCchPrintf(tchPosY, COUNTOF(tchPosY), L"%ix%i PosY", ResX, ResY);
    StringCchPrintf(tchSizeX, COUNTOF(tchSizeX), L"%ix%i SizeX", ResX, ResY);
    StringCchPrintf(tchSizeY, COUNTOF(tchSizeY), L"%ix%i SizeY", ResX, ResY);
    StringCchPrintf(tchMaximized, COUNTOF(tchMaximized), L"%ix%i Maximized", ResX, ResY);
    StringCchPrintf(tchDPI, COUNTOF(tchDPI), L"%ix%i DPI", ResX, ResY);
    StringCchPrintf(tchZoom, COUNTOF(tchZoom), L"%ix%i Zoom", ResX, ResY);

    if (bClearSettings) {
        IniSectionDelete(Constants.Window_Section, tchPosX, false);
        IniSectionDelete(Constants.Window_Section, tchPosY, false);
        IniSectionDelete(Constants.Window_Section, tchSizeX, false);
        IniSectionDelete(Constants.Window_Section, tchSizeY, false);
        IniSectionDelete(Constants.Window_Section, tchMaximized, false);
        IniSectionDelete(Constants.Window_Section, tchZoom, false);
        IniSectionDelete(Constants.Window_Section, tchDPI, false);
    } else {
        // overwrite last saved window position
        IniSectionSetInt(Constants.Window_Section, tchPosX, winInfo.x);
        IniSectionSetInt(Constants.Window_Section, tchPosY, winInfo.y);
        IniSectionSetInt(Constants.Window_Section, tchSizeX, winInfo.cx);
        IniSectionSetInt(Constants.Window_Section, tchSizeY, winInfo.cy);
        IniSectionSetBool(Constants.Window_Section, tchMaximized, winInfo.max);
        IniSectionSetBool(Constants.Window_Section, tchMaximized, winInfo.max);
        IniSectionSetInt(Constants.Window_Section, tchZoom, winInfo.zoom);
        IniSectionSetInt(Constants.Window_Section, tchDPI, winInfo.dpi);
        // set current window position as new initial window
        g_IniWinInfo = winInfo;
    }

    return true;
}


//=============================================================================
//
//  SaveAllSettings()
//
bool SaveAllSettings(bool bForceSaveSettings)
{
    if (Flags.bDoRelaunchElevated) {
        return true;
    } // already saved before relaunch
    if (Flags.bSettingsFileSoftLocked) {
        return false;
    }

    WCHAR tchMsg[80];
    GetLngString(IDS_MUI_SAVINGSETTINGS, tchMsg, COUNTOF(tchMsg));

    bool ok = false;

    BeginWaitCursor(true, tchMsg);

    ok = OpenSettingsFile(__func__);

    if (ok) {

        _SaveSettings(bForceSaveSettings);

        if (Globals.bCanSaveIniFile) {
            if (!Settings.SaveRecentFiles) {
                // Cleanup unwanted MRUs
                MRU_Empty(Globals.pFileMRU, false, true);
                MRU_Save(Globals.pFileMRU);
            } else {
                //int const cnt = MRU_Count(Globals.pFileMRU);
                MRU_MergeSave(Globals.pFileMRU, true, Flags.RelativeFileMRU, Flags.PortableMyDocs);
            }

            if (!Settings.SaveFindReplace) {
                // Cleanup unwanted MRUs
                MRU_Empty(Globals.pMRUfind, false, true);
                MRU_Save(Globals.pMRUfind);
                MRU_Empty(Globals.pMRUreplace, false, true);
                MRU_Save(Globals.pMRUreplace);
            } else {
                MRU_MergeSave(Globals.pMRUfind, false, false, false);
                MRU_MergeSave(Globals.pMRUreplace, false, false, false);
            }
        }
    }

    if (Globals.bIniFileFromScratch) {
        Style_CanonicalSectionToIniCache();
    }
    if (Globals.uCurrentThemeIndex == 0) {
        Style_ToIniSection(false);
    }
    Style_FileExtToIniSection(false);

    ok = (ok ? CloseSettingsFile(__func__, true) : true);

    // maybe separate INI files for Style-Themes
    if (Globals.uCurrentThemeIndex > 0) {
        Style_SaveSettings(bForceSaveSettings);
    }

    EndWaitCursor();
    return ok;
}



//=============================================================================
//
//  CmdSaveSettingsNow()
//
void CmdSaveSettingsNow()
{
    bool bCreateFailure = false;
    if (Path_IsEmpty(Paths.IniFile)) {
        if (Path_IsNotEmpty(Paths.IniFileDefault)) {
            Path_Reset(Paths.IniFile, Path_Get(Paths.IniFileDefault));
            DWORD dwFileSize        = 0UL;
            Globals.bCanSaveIniFile = CreateIniFile(Paths.IniFile, &dwFileSize);
            if (Globals.bCanSaveIniFile) {
                Globals.bIniFileFromScratch = (dwFileSize == 0UL);
                Path_Empty(Paths.IniFileDefault, false);
            } else {
                Path_Empty(Paths.IniFile, false);
                Globals.bCanSaveIniFile = false;
                bCreateFailure          = true;
            }
        } else {
            return;
        }
    }
    if (bCreateFailure) {
        InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_CREATEINI_FAIL);
        return;
    }
    DWORD dwFileAttributes = 0;
    if (!Globals.bCanSaveIniFile) {
        dwFileAttributes = Path_GetFileAttributes(Paths.IniFile);
        if (dwFileAttributes == INVALID_FILE_ATTRIBUTES) {
            InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_CREATEINI_FAIL);
            return;
        }
        if (dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
            if (IsYesOkay(InfoBoxLng(MB_YESNO | MB_ICONWARNING, NULL, IDS_MUI_INIFILE_READONLY))) {
                Path_SetFileAttributes(Paths.IniFile, FILE_ATTRIBUTE_NORMAL); // override read-only attrib
                Globals.bCanSaveIniFile = CanAccessPath(Paths.IniFile, GENERIC_WRITE);
            }
        } else {
            dwFileAttributes = 0; // no need to change the file attributes
        }
    }
    if (Globals.bCanSaveIniFile && SaveAllSettings(true)) {
        InfoBoxLng(MB_ICONINFORMATION, L"MsgSaveSettingsInfo", IDS_MUI_SAVEDSETTINGS);
        if ((dwFileAttributes != 0) && (dwFileAttributes != INVALID_FILE_ATTRIBUTES)) {
            Path_SetFileAttributes(Paths.IniFile, dwFileAttributes); // reset
        }
        Globals.bCanSaveIniFile = CanAccessPath(Paths.IniFile, GENERIC_WRITE);
    } else {
        Globals.dwLastError = GetLastError();
        InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_WRITEINI_FAIL);
        return;
    }
}


//=============================================================================
//=============================================================================


//=============================================================================
//
//  MRU functions
//
LPMRULIST MRU_Create(LPCWSTR pszRegKey, int iFlags, int iSize)
{
    auto pmru = (LPMRULIST)AllocMem(sizeof(MRULIST), HEAP_ZERO_MEMORY);
    if (pmru) {
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
            if (pmru->pszItems[i]) {
                LocalFree(pmru->pszItems[i]);    // StrDup()
                pmru->pszItems[i] = NULL;
            }
            if (pmru->pszBookMarks[i]) {
                LocalFree(pmru->pszBookMarks[i]);    // StrDup()
                pmru->pszBookMarks[i] = NULL;
            }
        }
        FreeMem(pmru);
        return true;
    }
    return false;
}


static int _MRU_Compare(LPMRULIST pmru, LPCWSTR psz1, LPCWSTR psz2)
{
    if (pmru) {
        if (pmru->iFlags & MRU_NOCASE) {
            return(StringCchCompareXI(psz1, psz2));
        }
        return(StringCchCompareX(psz1, psz2));
    }
    return 0;
}


bool MRU_Add(LPMRULIST pmru, LPCWSTR pszNew, cpi_enc_t iEnc, DocPos iPos, DocPos iSelAnc, LPCWSTR pszBookMarks)
{
    if (pmru) {
        int i = 0;
        for (; i < pmru->iSize; ++i) {
            if (_MRU_Compare(pmru, pmru->pszItems[i], pszNew) == 0) {
                LocalFree(pmru->pszItems[i]); // StrDup()
                pmru->pszItems[i] = NULL;
                break;
            }
        }
        i = min_i(i, pmru->iSize - 1);
        for (; i > 0; i--) {
            pmru->pszItems[i] = pmru->pszItems[i - 1];
            pmru->iEncoding[i] = pmru->iEncoding[i - 1];
            pmru->iCaretPos[i] = pmru->iCaretPos[i - 1];
            pmru->iSelAnchPos[i] = pmru->iSelAnchPos[i - 1];
        }
        pmru->pszItems[0] = StrDup(pszNew); // LocalAlloc()

        pmru->iEncoding[0] = iEnc;
        pmru->iCaretPos[0] = (Settings.PreserveCaretPos ? iPos : -1);
        pmru->iSelAnchPos[0] = (Settings.PreserveCaretPos ? iSelAnc : -1);
        pmru->pszBookMarks[0] = (pszBookMarks ? StrDup(pszBookMarks) : NULL);  // LocalAlloc()
        return true;
    }
    return false;
}


bool MRU_FindPath(LPMRULIST pmru, const HPATHL hpth, int* iIndex)
{
    *iIndex = 0;
    bool res = false;
    if (pmru) {

        HPATHL hcpy = Path_Copy(hpth);
        Path_AbsoluteFromApp(hcpy, true);

        HPATHL hcmp = Path_Allocate(NULL);
        int    i = 0;
        for (i = 0; i < pmru->iSize; ++i) {
            if (pmru->pszItems[i] == NULL) {
                break;
            }
            Path_Reset(hcmp, pmru->pszItems[i]);
            Path_AbsoluteFromApp(hcmp, true);
            if (StringCchCompareXI(Path_Get(hcmp), Path_Get(hpth)) == 0) {
                res = true;
                break;
            }
        }
        *iIndex = i;

        Path_Release(hcmp);
        Path_Release(hcpy);
    }
    return res;
}


bool MRU_AddPath(LPMRULIST pmru, const HPATHL hpth, bool bRelativePath, bool bUnexpandMyDocs,
                 cpi_enc_t iEnc, DocPos iPos, DocPos iSelAnc, LPCWSTR pszBookMarks)
{
    if (pmru) {
        int i = 0;
        bool const bAlreadyInList = MRU_FindPath(pmru, hpth, &i);
        if (bAlreadyInList) {
            LocalFree(pmru->pszItems[i]);  // StrDup()
            pmru->pszItems[i] = NULL;
        } else {
            i = (i < pmru->iSize) ? i : (pmru->iSize - 1);
        }
        for (; i > 0; i--) {
            pmru->pszItems[i] = pmru->pszItems[i - 1];
            pmru->iEncoding[i] = pmru->iEncoding[i - 1];
            pmru->iCaretPos[i] = pmru->iCaretPos[i - 1];
            pmru->iSelAnchPos[i] = pmru->iSelAnchPos[i - 1];
            pmru->pszBookMarks[i] = pmru->pszBookMarks[i - 1];
        }

        HPATHL hpth_cpy = Path_Copy(hpth);

        if (bRelativePath) {
            Path_RelativeToApp(hpth_cpy, true, true, bUnexpandMyDocs);
        }
        pmru->pszItems[0] = StrDupW(Path_Get(hpth_cpy)); // LocalAlloc()
        pmru->iEncoding[0] = iEnc;
        pmru->iCaretPos[0] = (Settings.PreserveCaretPos ? iPos : -1);
        pmru->iSelAnchPos[0] = (Settings.PreserveCaretPos ? iSelAnc : -1);
        pmru->pszBookMarks[0] = (pszBookMarks ? StrDupW(pszBookMarks) : NULL);  // LocalAlloc()

        Path_Release(hpth_cpy);

        return bAlreadyInList;
    }
    return false;
}


#pragma warning(push)
#pragma warning(disable : 6385 6386)

static void _MRU_DeleteItemInIniFile(LPCWSTR section, LPMRULIST pmru, const int iIndex)
{
    if (OpenSettingsFile(__func__)) {

        WCHAR wchName[32] = { L'\0' };

        if (pmru->pszItems[iIndex]) {

            StringCchPrintf(wchName, COUNTOF(wchName), L"%.2i", iIndex + 1);
            IniSectionDelete(section, wchName, false);

            if (pmru->iEncoding[iIndex] > 0) {
                StringCchPrintf(wchName, COUNTOF(wchName), L"ENC%.2i", iIndex + 1);
                IniSectionDelete(section, wchName, false);
            }
            if (pmru->iCaretPos[iIndex] >= 0) {
                StringCchPrintf(wchName, COUNTOF(wchName), L"POS%.2i", iIndex + 1);
                IniSectionDelete(section, wchName, false);
            }
            if (pmru->iSelAnchPos[iIndex] >= 0) {
                StringCchPrintf(wchName, COUNTOF(wchName), L"ANC%.2i", iIndex + 1);
                IniSectionDelete(section, wchName, false);
            }
            if (StrIsNotEmpty(pmru->pszBookMarks[iIndex])) {
                StringCchPrintf(wchName, COUNTOF(wchName), L"BMRK%.2i", iIndex + 1);
                IniSectionDelete(section, wchName, false);
            }
        }
        CloseSettingsFile(__func__, true);
    }
}


bool MRU_Delete(LPMRULIST pmru, int iIndex)
{
    if (pmru && iIndex < MRU_MAXITEMS) {
        if (iIndex >= 0 && iIndex < pmru->iSize) {
            
            _MRU_DeleteItemInIniFile(pmru->szRegKey, pmru, iIndex);

            if (pmru->pszItems[iIndex]) {
                LocalFree(pmru->pszItems[iIndex]);  // StrDup()
                pmru->pszItems[iIndex] = NULL;
            }
            if (pmru->pszBookMarks[iIndex]) {
                LocalFree(pmru->pszBookMarks[iIndex]); // StrDup()
                pmru->pszBookMarks[iIndex] = NULL;
            }
            bool bZeroMoved = false;
            for (int i = iIndex; (i < pmru->iSize - 1) && !bZeroMoved; ++i) {
                pmru->pszItems[i] = pmru->pszItems[i + 1];
                pmru->iEncoding[i] = pmru->iEncoding[i + 1];
                pmru->iCaretPos[i] = pmru->iCaretPos[i + 1];
                pmru->iSelAnchPos[i] = pmru->iSelAnchPos[i + 1];
                pmru->pszBookMarks[i] = pmru->pszBookMarks[i + 1];

                bZeroMoved = (NULL == pmru->pszItems[i + 1]);

                pmru->pszItems[i + 1] = NULL;
                pmru->iEncoding[i + 1] = 0;
                pmru->iCaretPos[i + 1] = -1;
                pmru->iSelAnchPos[i + 1] = -1;
                pmru->pszBookMarks[i + 1] = NULL;
            }
            return true;
        }
    }
    return false;
}

#pragma warning(pop)


bool MRU_Empty(LPMRULIST pmru, bool bExceptLeast, bool bDelete)
{
    if (pmru) {
        if (OpenSettingsFile(__func__)) {

            int const beg = bExceptLeast ? 1 : 0;
            for (int i = beg; i < pmru->iSize; ++i) {
                if (pmru->pszItems[i]) {
                    if (bDelete) {
                        _MRU_DeleteItemInIniFile(pmru->szRegKey, pmru, i);
                    }
                    LocalFree(pmru->pszItems[i]); // StrDup()
                    pmru->pszItems[i] = NULL;
                    pmru->iEncoding[i] = 0;
                    pmru->iCaretPos[i] = -1;
                    pmru->iSelAnchPos[i] = -1;
                    if (pmru->pszBookMarks[i]) {
                        LocalFree(pmru->pszBookMarks[i]); // StrDup()
                        pmru->pszBookMarks[i] = NULL;
                    }
                }
            }
            CloseSettingsFile(__func__, bDelete);
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
            while (i < pmru->iSize && pmru->pszItems[i]) {
                ++i;
            }
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


bool MRU_Load(LPMRULIST pmru, bool bFileProps)
{
    if (pmru) {
        if (OpenSettingsFile(__func__)) {

            MRU_Empty(pmru, false, false);

            const WCHAR* const RegKey_Section = pmru->szRegKey;
            int n = 0; 
            for (int i = 0; i < pmru->iSize; ++i) {
                WCHAR tchName[32] = { L'\0' };
                StringCchPrintf(tchName, COUNTOF(tchName), L"%.2i", i + 1);
                WCHAR tchItem[2048] = { L'\0' };
                if (IniSectionGetString(RegKey_Section, tchName, L"", tchItem, COUNTOF(tchItem))) {
                    size_t const len = StringCchLen(tchItem, 0);
                    if ((len > 1) && (tchItem[0] == L'"') && (tchItem[len - 1] == L'"')) {
                        MoveMemory(tchItem, (tchItem + 1), len * sizeof(WCHAR));
                        tchItem[len - 2] = L'\0'; // clear dangling '"'
                    }
                    pmru->pszItems[n] = StrDup(tchItem);

                    StringCchPrintf(tchName, COUNTOF(tchName), L"ENC%.2i", i + 1);
                    auto const iCP = (cpi_enc_t)IniSectionGetInt(RegKey_Section, tchName, 0);
                    pmru->iEncoding[n] = bFileProps ? (cpi_enc_t)Encoding_MapIniSetting(true, iCP) : 0;

                    StringCchPrintf(tchName, COUNTOF(tchName), L"POS%.2i", i + 1);
                    pmru->iCaretPos[n] = bFileProps ? ((Settings.PreserveCaretPos) ? IniSectionGetInt(RegKey_Section, tchName, 0) : -1) : -1;

                    StringCchPrintf(tchName, COUNTOF(tchName), L"ANC%.2i", i + 1);
                    pmru->iSelAnchPos[n] = bFileProps ? ((Settings.PreserveCaretPos) ? IniSectionGetInt(RegKey_Section, tchName, 0) : -1) : -1;

                    StringCchPrintf(tchName, COUNTOF(tchName), L"BMRK%.2i", i + 1);

                    WCHAR wchBookMarks[MRU_BMRK_SIZE] = { L'\0' };
                    IniSectionGetString(RegKey_Section, tchName, L"", wchBookMarks, COUNTOF(wchBookMarks));
                    pmru->pszBookMarks[n] = bFileProps ? StrDup(wchBookMarks) : nullptr;

                    ++n;
                }
            }
            CloseSettingsFile(__func__, false); // read only
        }
        return true;
    }
    return false;
}


void MRU_Save(LPMRULIST pmru)
{
    if (pmru) {
        if (OpenSettingsFile(__func__)) {

            WCHAR tchName[32] = { L'\0' };
            WCHAR tchItem[2048] = { L'\0' };

            const WCHAR* const RegKey_Section = pmru->szRegKey;
            IniSectionClear(pmru->szRegKey, false);

            for (int i = 0; i < pmru->iSize; ++i) {
                if (pmru->pszItems[i]) {
                    StringCchPrintf(tchName, COUNTOF(tchName), L"%.2i", i + 1);
                    StringCchPrintf(tchItem, COUNTOF(tchItem), L"\"%s\"", pmru->pszItems[i]);
                    IniSectionSetString(RegKey_Section, tchName, tchItem);

                    if (pmru->iEncoding[i] > 0) {
                        StringCchPrintf(tchName, COUNTOF(tchName), L"ENC%.2i", i + 1);
                        auto const iCP = (int)Encoding_MapIniSetting(false, (int)pmru->iEncoding[i]);
                        IniSectionSetInt(RegKey_Section, tchName, iCP);
                    }
                    if (pmru->iCaretPos[i] >= 0) {
                        StringCchPrintf(tchName, COUNTOF(tchName), L"POS%.2i", i + 1);
                        IniSectionSetPos(RegKey_Section, tchName, pmru->iCaretPos[i]);
                    }
                    if (pmru->iSelAnchPos[i] >= 0) {
                        StringCchPrintf(tchName, COUNTOF(tchName), L"ANC%.2i", i + 1);
                        IniSectionSetPos(RegKey_Section, tchName, pmru->iSelAnchPos[i]);
                    }
                    if (StrIsNotEmpty(pmru->pszBookMarks[i])) {
                        StringCchPrintf(tchName, COUNTOF(tchName), L"BMRK%.2i", i + 1);
                        IniSectionSetString(RegKey_Section, tchName, pmru->pszBookMarks[i]);
                    }
                }
            }
            CloseSettingsFile(__func__, true);
        }
    }
}


bool MRU_MergeSave(LPMRULIST pmru, bool bAddFiles, bool bRelativePath, bool bUnexpandMyDocs)
{
    if (pmru) {
        if (OpenSettingsFile(__func__)) {

            LPMRULIST pmruBase = MRU_Create(pmru->szRegKey, pmru->iFlags, pmru->iSize);
            MRU_Load(pmruBase, bAddFiles);

            if (bAddFiles) {
                HPATHL hpth = Path_Allocate(NULL);
                for (int i = pmru->iSize - 1; i >= 0; i--) {
                    if (pmru->pszItems[i]) {
                        Path_Reset(hpth, pmru->pszItems[i]);
                        Path_AbsoluteFromApp(hpth, true);
                        MRU_AddPath(pmruBase, hpth, bRelativePath, bUnexpandMyDocs,
                                    pmru->iEncoding[i], pmru->iCaretPos[i], pmru->iSelAnchPos[i], pmru->pszBookMarks[i]);
                    }
                }
                Path_Release(hpth);
            }
            else {
                for (int i = pmru->iSize - 1; i >= 0; i--) {
                    if (pmru->pszItems[i])
                        MRU_Add(pmruBase, pmru->pszItems[i],
                                pmru->iEncoding[i], pmru->iCaretPos[i], pmru->iSelAnchPos[i], pmru->pszBookMarks[i]);
                }
            }

            MRU_Save(pmruBase);
            pmruBase = NULL;

            CloseSettingsFile(__func__, true);

            return true;
        }
    }
    return false;
}


// ////////////////////////////////////////////////////////////////////////////
// Some C++ Extentions for Notepad3
// ////////////////////////////////////////////////////////////////////////////

//=============================================================================
//
//  EditSetDocumentBuffer() - Set Document Buffer for Scintilla Edit Component
//

#if TRUE
static bool CreateNewDocument(const char* lpstrText, DocPosU lenText, int docOptions, bool reload)
{
#define RELEASE_RETURN(ret)  { pDocLoad->Release(); return(ret); }

    if (!lpstrText || (lenText == 0)) {
        SciCall_SetDocPointer(0);
    } else {
        sptr_t const pNewDocumentPtr = SciCall_CreateDocument(lenText, docOptions);
        if (pNewDocumentPtr) {
            SciCall_SetDocPointer(pNewDocumentPtr);
            SciCall_ReleaseDocument(pNewDocumentPtr);
        }
        else {
            SciCall_SetDocPointer(0);
        }
        SciCall_TargetWholeDocument();
        if (reload) {
            SciCall_ReplaceTargetMinimal(lenText, lpstrText);
        } else {
            SciCall_ReplaceTarget(lenText, lpstrText);
        }
    }
    return true;
}
#else
static bool CreateNewDocument(const char* lpstrText, DocPosU lenText, int docOptions)
{
    UNREFERENCED_PARAMETER(docOptions);
    if (!lpstrText || (lenText == 0)) {
        SciCall_ClearAll();
    } else {
        SciCall_TargetWholeDocument();
        if (reload) {
            SciCall_ReplaceTargetMinimal(lenText, lpstrText);
        } else {
            SciCall_ReplaceTarget(lenText, lpstrText);
        }
    }
    return true;
}
#endif


extern "C" bool EditSetDocumentBuffer(const char* lpstrText, DocPosU lenText, bool reload)
{
    bool const bLargerThan2GB = (lenText >= ((DocPosU)INT32_MAX));
    bool const bLargeFileLoaded = (lenText >= ((DocPosU)Settings2.FileLoadWarningMB << 20));
    int const docOptions = bLargeFileLoaded ? (bLargerThan2GB ? SC_DOCUMENTOPTION_TEXT_LARGE : SC_DOCUMENTOPTION_STYLES_NONE)
                                                              : SC_DOCUMENTOPTION_DEFAULT;

    if (SciCall_GetDocumentOptions() != docOptions) {
        // we have to create a new document with changed options
        return CreateNewDocument(lpstrText, lenText, docOptions, reload);
    }
    if (!lpstrText || (lenText == 0)) {
        SciCall_ClearAll();
    } else {
        SciCall_TargetWholeDocument();
        if (reload) {
            SciCall_ReplaceTargetMinimal(lenText, lpstrText);
        } else {
            SciCall_ReplaceTarget(lenText, lpstrText);
        }
    }
    return true;
}

