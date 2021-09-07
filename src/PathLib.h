/****************************************************************/
#pragma once

// comment out PATHCCH_NO_DEPRECATE to show deprcated API
#define PATHCCH_NO_DEPRECATE 1
///#include <pathcch.h>

#include "DynStrg.h"


/**************************************************/
/*             Declared in WINNT.H                */
/*                                                */
/*  Provides bottom line type safety in function  */
/*  calls instead of using void* pointer          */
/**************************************************/
#ifndef DECLARE_HANDLE
#define DECLARE_HANDLE(name) \
    struct name##__ {        \
        int unused;          \
    };                       \
    typedef struct name##__ *name
#endif

#define PTHAPI __stdcall

DECLARE_HANDLE(HPATHL);

/**************************************************/
/*                                                */
/*          DYNAMIC WIDCHAR LONG PATH             */
/*                                                */
/**************************************************/

HPATHL PTHAPI          Path_Allocate(const wchar_t* path);
void PTHAPI            Path_Release(HPATHL hstr);
int PTHAPI             Path_Reset(HPATHL hpth, const wchar_t* path);
size_t PTHAPI          Path_GetLength(HPATHL hpth);
HPATHL PTHAPI          Path_Copy(const HPATHL hpth);
bool PTHAPI            Path_Append(HPATHL hpth, HPATHL hmore);
bool PTHAPI            Path_Canonicalize(HPATHL hpth_in_out);
bool PTHAPI            Path_IsValidUNC(const HPATHL hpth, HSTRINGW server_name_out);
bool PTHAPI            Path_RemoveBackslash(HPATHL hpth_in_out);
bool PTHAPI            Path_RemoveFileSpec(HPATHL hpth);
bool PTHAPI            Path_RenameExtension(HPATHL hpth, const wchar_t* ext);
void PTHAPI            Path_ExpandEnvStrings(HPATHL hpth);
void PTHAPI            Path_GetModuleFilePath(HPATHL hpth_out);
bool PTHAPI            Path_IsPrefix(const HPATHL hprefix, const HPATHL hpth);

// -------------------------------------------------------

// ----------------------------------------------------------------------------
// deprecated (intermediate only)
// ----------------------------------------------------------------------------

const wchar_t* PTHAPI Path_Get(HPATHL hpth);
size_t PTHAPI         Path_GetBufCount(HPATHL hpth);

// get wchar buffer with at least MAX_PATH size
// TODO: get rid of this intermediate state handler
wchar_t* PTHAPI Path_WriteAccessBuf(HPATHL hpth, size_t len);

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

__forceinline bool IsReadOnly(const DWORD dwFileAttr)
{
    return ((dwFileAttr != INVALID_FILE_ATTRIBUTES) && (dwFileAttr & FILE_ATTRIBUTE_READONLY));
}

__forceinline bool IsExistingFile(const DWORD dwFileAttr)
{
    return ((dwFileAttr != INVALID_FILE_ATTRIBUTES) && !(dwFileAttr & FILE_ATTRIBUTE_DIRECTORY));
}

__forceinline bool IsExistingDirectory(const DWORD dwFileAttr)
{
    return ((dwFileAttr != INVALID_FILE_ATTRIBUTES) && (dwFileAttr & FILE_ATTRIBUTE_DIRECTORY));
}


// ============================================================================
// Duplicates for INTERMEDIATE DEV
// ============================================================================

bool PTHAPI Path_IsExistingFile(const HPATHL hpth);
bool PTHAPI PathIsExistingFile(LPCWSTR pszPath);

void PTHAPI ExpandEnvironmentStrgs(HSTRINGW hstr);
void PTHAPI ExpandEnvironmentStringsEx(LPWSTR lpSrc, size_t cchSrc);

size_t PTHAPI Path_GetLongPathNameEx(HPATHL hpth_in_out);
size_t PTHAPI GetLongPathNameEx(LPWSTR lpszPath, const size_t cchBuffer);

bool PTHAPI Path_GetKnownFolder(REFKNOWNFOLDERID rfid, HPATHL hpth_out);
bool PTHAPI PathGetKnownFolder(REFKNOWNFOLDERID, LPWSTR lpOutPath, size_t cchOut);

bool PTHAPI Path_CanonicalizeEx(HPATHL hpth_in_out);
bool PTHAPI PathCanonicalizeEx(LPWSTR lpszPath, size_t cchPath);

size_t PTHAPI Path_NormalizeEx(HPATHL hpth_in_out, const HPATHL hpth_wrkdir, bool bRealPath, bool bSearchPathIfRelative);
size_t PTHAPI NormalizePathEx(LPWSTR lpszPath, size_t cchPath, LPCWSTR lpszWorkDir, bool bRealPath, bool bSearchPathIfRelative);

void PTHAPI Path_GetAppDirectory(HPATHL hpth_out);
void PTHAPI PathGetAppDirectory(LPWSTR lpszDest, size_t cchDest);

void PTHAPI Path_AbsoluteFromApp(HPATHL hpth_in_out, bool bExpandEnv);
void PTHAPI PathAbsoluteFromApp(LPWSTR lpszPath, const size_t cchPath, bool bExpandEnv);

void PTHAPI Path_GetDisplayName(wchar_t* lpszDestPath, const size_t cchDestBuffer, const HPATHL hpth_in, const wchar_t* repl);
void PTHAPI PathGetDisplayName(LPWSTR lpszDestPath, const size_t cchDestBuffer, LPCWSTR lpszSourcePath, const wchar_t* repl);

void PTHAPI Path_RelativeToApp(HPATHL hpth_in_out, bool bSrcIsFile, bool bUnexpandEnv, bool bUnexpandMyDocs);
void PTHAPI PathRelativeToApp(LPWSTR lpszPath, size_t cchPath, bool bSrcIsFile, bool bUnexpandEnv, bool bUnexpandMyDocs);

// ============================================================================
// Some Old MAX_PATH stuff
// ============================================================================

bool PTHAPI   PathIsLnkFile(LPCWSTR pszPath);
bool PTHAPI   PathGetLnkPath(LPCWSTR pszLnkFile, LPWSTR pszResPath, int cchResPath);
bool PTHAPI   PathIsLnkToDirectory(LPCWSTR pszPath, LPWSTR pszResPath, int cchResPath);
bool PTHAPI   PathCreateDeskLnk(LPCWSTR pszDocument, LPCWSTR pszDescription);
bool PTHAPI   PathCreateFavLnk(LPCWSTR pszName, LPCWSTR pszTarget, LPCWSTR pszDir);


// ============================================================================
// ============================================================================

