// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* PathLib.h                                                                   *
*   Definition for dynamic wide char Long Path handling                       *
*   Based on DynStrg module                                                   *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2025   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#pragma once

// comment out PATHCCH_NO_DEPRECATE to show deprecated API
#define PATHCCH_NO_DEPRECATE 1
///#include <pathcch.h>

#include "TypeDefs.h"
#include "DynStrg.h"

#define PTHAPI __stdcall

// PATHCCH_MAX_CCH: (0x7FFF + 1 for NULL terminator)
#define PATHLONG_MAX_CCH 0x8000 

// ----------------------------------------------------------------------------
__forceinline size_t max_sz(const size_t x, const size_t y) { return (x < y) ? y : x; }
// ----------------------------------------------------------------------------

__forceinline LPCWSTR PTHAPI Path_Get(HPATHL hpth)
{
    return StrgGet((HSTRINGW)hpth);
}

__forceinline size_t PTHAPI Path_GetBufCount(HPATHL hpth)
{
    return StrgGetAllocLength((HSTRINGW)hpth);
}

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

// ----------------------------------------------------------------------------

// get wchar buffer with at least MAX_PATH_EXPLICIT size to minimize reallocations
// execept for len = 0, where no reallocation (except buffer is NULL) is done
__forceinline LPWSTR PTHAPI Path_WriteAccessBuf(HPATHL hpth, size_t len)
{
    return StrgWriteAccessBuf((HSTRINGW)hpth, len ? max_sz(len, MAX_PATH_EXPLICIT) : 0);
}

__forceinline void PTHAPI Path_Sanitize(HPATHL hpth)
{
    StrgSanitize((HSTRINGW)hpth);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------



/**************************************************/
/*                                                */
/*          DYNAMIC WIDCHAR LONG PATH             */
/*                                                */
/**************************************************/

HPATHL PTHAPI          Path_Allocate(LPCWSTR path);
void PTHAPI            Path_Release(HPATHL hpth);
void PTHAPI            Path_Empty(HPATHL hpth, bool truncate);
int PTHAPI             Path_Reset(HPATHL hpth, LPCWSTR path);
size_t PTHAPI          Path_GetLength(HPATHL hpth);
HPATHL PTHAPI          Path_Copy(const HPATHL hpth);
bool PTHAPI            Path_Append(HPATHL hpth, LPCWSTR more);
void PTHAPI            Path_Swap(HPATHL hpth1, HPATHL hpth2);
void PTHAPI            Path_FreeExtra(HPATHL hpth_in_out, size_t keep_length);

bool PTHAPI            Path_FixBackslashes(HPATHL hpth_in_out);
bool PTHAPI            Path_Canonicalize(HPATHL hpth_in_out);
bool PTHAPI            Path_IsEmpty(const HPATHL hpth);
inline bool PTHAPI     Path_IsNotEmpty(const HPATHL hpth) { return !Path_IsEmpty(hpth); };
bool PTHAPI            Path_IsRoot(const HPATHL hpth);
bool PTHAPI            Path_IsValidUNC(const HPATHL hpth);
bool PTHAPI            Path_IsExistingDirectory(const HPATHL hpth);

int PTHAPI             Path_StrgComparePath(const HPATHL hpth1, const HPATHL hpth2, const HPATHL hpth_wrkdir, const bool bNormalize);
bool PTHAPI            Path_RemoveFileSpec(HPATHL hpth_in_out);
bool PTHAPI            Path_RenameExtension(HPATHL hpth, LPCWSTR ext);
void PTHAPI            Path_ExpandEnvStrings(HPATHL hpth_in_out);
void PTHAPI            Path_UnExpandEnvStrings(HPATHL hpth);
void PTHAPI            Path_GetModuleFilePath(HPATHL hpth_out);
void PTHAPI            Path_GetAppDirectory(HPATHL hpth_out);
bool PTHAPI            Path_IsRelative(const HPATHL hpath);
bool PTHAPI            Path_IsPrefix(const HPATHL hprefix, const HPATHL hpth);
size_t PTHAPI          Path_CommonPrefix(const HPATHL hpth1, const HPATHL hpth2, HPATHL hpfx_out);
LPCWSTR PTHAPI         Path_FindFileName(const HPATHL hpth);
LPCWSTR PTHAPI         Path_FindExtension(const HPATHL hpth);
bool PTHAPI            Path_QuoteSpaces(HPATHL hpth_in_out, bool bForceQuotes);
void PTHAPI            Path_UnQuoteSpaces(HPATHL hpth_in_out);
int PTHAPI             Path_GetDriveNumber(const HPATHL hpth);
bool PTHAPI            Path_IsUNC(const HPATHL hpth);
wchar_t PTHAPI         Path_GetDriveLetterByNumber(const int number);
DWORD PTHAPI           Path_GetFileAttributes(const HPATHL hpth);
bool PTHAPI            Path_SetFileAttributes(HPATHL hpth, DWORD dwAttributes);
bool PTHAPI            Path_StripToRoot(HPATHL hpth_in_out);
bool PTHAPI            Path_GetCurrentDirectory(HPATHL hpth_out);
size_t PTHAPI          Path_ToShortPathName(HPATHL hpth_in_out);  // use only, if neccessary
size_t PTHAPI          Path_GetLongPathNameEx(HPATHL hpth_in_out);

void PTHAPI            Path_GetDisplayName(LPWSTR lpszDisplayName, const DWORD cchDisplayName, const HPATHL hpth, LPCWSTR repl, bool bStripPath);
bool PTHAPI            Path_GetLnkPath(const HPATHL hLnkFilePth, HPATHL hResPath_out);
bool PTHAPI            Path_IsLnkFile(const HPATHL hpth);
bool PTHAPI            Path_IsLnkToDirectory(const HPATHL hlnk_pth, HPATHL hpth_out);
bool PTHAPI            Path_CreateFavLnk(LPCWSTR lpszLinkName, const HPATHL hTargetPth, const HPATHL hDirPth);
bool PTHAPI            Path_CreateDeskLnk(const HPATHL hDocumentPath, LPCWSTR pszDescription);
bool PTHAPI            Path_BrowseDirectory(HWND hwndParent, LPCWSTR lpszTitle, HPATHL hpth_in_out, const HPATHL hbase, bool bNewDialogStyle);
bool PTHAPI            Path_CanonicalizeEx(HPATHL hpth_in_out, const HPATHL hdir_rel_base);
size_t PTHAPI          Path_NormalizeEx(HPATHL hpth_in_out, const HPATHL hpth_wrkdir, bool bRealPath, bool bSearchPathIfRelative);
bool PTHAPI            Path_RelativePathTo(HPATHL hrecv, const HPATHL hfrom, DWORD attr_from, const HPATHL hto, DWORD attr_to);
void PTHAPI            Path_RelativeToApp(HPATHL hpth_in_out, bool bSrcIsFile, bool bUnexpandEnv, bool bUnexpandMyDocs);
bool PTHAPI            Path_GetKnownFolder(REFKNOWNFOLDERID rfid, HPATHL hpth_out);

void PTHAPI            ExpandEnvironmentStrgs(HSTRINGW hstr, bool bStripQ);


// ============================================================================
// Duplicates for INTERMEDIATE DEV
// ============================================================================

bool PTHAPI Path_IsExistingFile(const HPATHL hpth);
bool PTHAPI PathIsExistingFile(LPCWSTR pszPath);

void PTHAPI Path_AbsoluteFromApp(HPATHL hpth_in_out, bool bExpandEnv);
void PTHAPI PathAbsoluteFromApp(LPWSTR lpszPath, const size_t cchPath, bool bExpandEnv);

// ============================================================================
// ============================================================================

