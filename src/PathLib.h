/****************************************************************/
#pragma once

// comment out PATHCCH_NO_DEPRECATE to show deprcated API
#define PATHCCH_NO_DEPRECATE 1
///#include <pathcch.h>


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
bool PTHAPI            Path_IsExistingFile(HPATHL hpth);

// -------------------------------------------------------

const wchar_t* PTHAPI  Path_Get(HPATHL hpth);
size_t PTHAPI          Path_GetBufCount(HPATHL hpth);

// get wchar buffer with at least MAX_PATH size
// TODO: get rid of this intermediate state handler
wchar_t* PTHAPI Path_AccessBuf(HPATHL hpth, size_t len);

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
// Some Old MAX_PATH stuff
// ============================================================================

bool PTHAPI   GetKnownFolderPath(REFKNOWNFOLDERID, LPWSTR lpOutPath, size_t cchCount);
void PTHAPI   PathGetAppDirectory(LPWSTR lpszDest, DWORD cchDest);
void PTHAPI   PathRelativeToApp(LPWSTR lpszSrc, LPWSTR lpszDest, int cchDest, bool, bool, bool);
void PTHAPI   PathAbsoluteFromApp(LPWSTR lpszSrc, LPWSTR lpszDest, int cchDest, bool);

bool PTHAPI   PathIsLnkFile(LPCWSTR pszPath);
bool PTHAPI   PathGetLnkPath(LPCWSTR pszLnkFile, LPWSTR pszResPath, int cchResPath);
bool PTHAPI   PathIsLnkToDirectory(LPCWSTR pszPath, LPWSTR pszResPath, int cchResPath);
bool PTHAPI   PathCreateDeskLnk(LPCWSTR pszDocument, LPCWSTR pszDescription);
bool PTHAPI   PathCreateFavLnk(LPCWSTR pszName, LPCWSTR pszTarget, LPCWSTR pszDir);

void PTHAPI   ExpandEnvironmentStringsEx(LPWSTR lpSrc, size_t cchSrc);
bool PTHAPI   PathCanonicalizeEx(LPWSTR lpszPath, size_t cchPath);
DWORD PTHAPI  GetLongPathNameEx(LPWSTR lpszPath, DWORD cchBuffer);
void PTHAPI   PathGetDisplayName(LPWSTR lpszDestPath, DWORD cchDestBuffer, LPCWSTR lpszSourcePath);
DWORD PTHAPI  NormalizePathEx(LPWSTR lpszPath, DWORD cchBuffer, LPCWSTR lpszWorkDir, bool bRealPath, bool bSearchPathIfRelative);


// ============================================================================
// ============================================================================

