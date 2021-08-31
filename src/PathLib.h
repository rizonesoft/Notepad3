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
bool PTHAPI            Path_Append(HPATHL hpth, HPATHL hmore);
bool PTHAPI            Path_Canonicalize(HPATHL hpth_out, HPATHL hpth_in);
bool PTHAPI            Path_RemoveFileSpec(HPATHL hpth);
bool PTHAPI            Path_RenameExtension(HPATHL hpth, const wchar_t* ext);
void PTHAPI            Path_ExpandEnvStrings(HPATHL hpth);
bool PTHAPI            Path_IsExistingFile(const HPATHL hpth);
void PTHAPI            Path_GetModuleFileName(HPATHL hpth_out);

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

inline bool IsReadOnly(const DWORD dwFileAttr)
{
    return ((dwFileAttr != INVALID_FILE_ATTRIBUTES) && (dwFileAttr & FILE_ATTRIBUTE_READONLY));
}

inline bool IsExistingFile(const DWORD dwFileAttr)
{
    return ((dwFileAttr != INVALID_FILE_ATTRIBUTES) && !(dwFileAttr & FILE_ATTRIBUTE_DIRECTORY));
}

inline bool PathIsExistingFile(LPCWSTR pszPath)
{
    return (PathFileExists(pszPath) && !PathIsDirectory(pszPath));
}


// ============================================================================
// Duplicates for INTERMEDIATE DEV
// ============================================================================

void PTHAPI ExpandEnvironmentStrg(HSTRINGW hstr);
void PTHAPI ExpandEnvironmentStringsEx(LPWSTR lpSrc, size_t cchSrc);

DWORD PTHAPI Path_GetLongPathNameEx(HPATHL hpth_in_out);
DWORD PTHAPI GetLongPathNameEx(LPWSTR lpszPath, DWORD cchBuffer);

bool PTHAPI Path_GetKnownFolder(REFKNOWNFOLDERID rfid, HPATHL hpth_out);
bool PTHAPI PathGetKnownFolder(REFKNOWNFOLDERID, LPWSTR lpOutPath, size_t cchOut);

bool PTHAPI Path_CanonicalizeEx(HPATHL hpth_in_out);
bool PTHAPI PathCanonicalizeEx(LPWSTR lpszPath, size_t cchPath);

size_t PTHAPI Path_NormalizeEx(HPATHL hpth_in_out, const HPATHL hpth_wrkdir, bool bRealPath, bool bSearchPathIfRelative);
size_t PTHAPI NormalizePathEx(LPWSTR lpszPath, DWORD cchPath, LPCWSTR lpszWorkDir, bool bRealPath, bool bSearchPathIfRelative);

void PTHAPI Path_GetAppDirectory(HPATHL hpth_out);
void PTHAPI PathGetAppDirectory(LPWSTR lpszDest, DWORD cchDest);


// ============================================================================
// Some Old MAX_PATH stuff
// ============================================================================

void PTHAPI   PathGetDisplayName(LPWSTR lpszDestPath, DWORD cchDestBuffer, LPCWSTR lpszSourcePath);

void PTHAPI   PathRelativeToApp(LPWSTR lpszSrc, LPWSTR lpszDest, int cchDest, bool, bool, bool);
void PTHAPI   PathAbsoluteFromApp(LPWSTR lpszSrc, LPWSTR lpszDest, int cchDest, bool);

bool PTHAPI   PathIsLnkFile(LPCWSTR pszPath);
bool PTHAPI   PathGetLnkPath(LPCWSTR pszLnkFile, LPWSTR pszResPath, int cchResPath);
bool PTHAPI   PathIsLnkToDirectory(LPCWSTR pszPath, LPWSTR pszResPath, int cchResPath);
bool PTHAPI   PathCreateDeskLnk(LPCWSTR pszDocument, LPCWSTR pszDescription);
bool PTHAPI   PathCreateFavLnk(LPCWSTR pszName, LPCWSTR pszTarget, LPCWSTR pszDir);


// ============================================================================
// ============================================================================

