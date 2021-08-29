
// ============================================================================
// 
// https://docs.microsoft.com/en-us/windows/win32/fileio/naming-a-file
// 
// https://docs.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation
//
// In the Windows API (with some exceptions discussed in the following paragraphs),
// the maximum length for a path is MAX_PATH, which is defined as 260 characters.
// A local path is structured in the following order: drive letter, colon, backslash,
// name components separated by backslashes, and a terminating null character.
// For example, the maximum path on drive D is "D:\some 256-character path string<NUL>"
// where "<NUL>" represents the invisible terminating null character for the current
// system codepage.
// (The characters < > are used here for visual clarity and cannot be part of a valid path string.)
//
// The Windows API has many functions that also have Unicode versions to permit an
// extended-length path for a maximum total path length of 32767 characters.
// This type of path is composed of components separated by backslashes,
// each up to the value returned in the lpMaximumComponentLength parameter of the
// GetVolumeInformation function (this value is commonly 255 characters).
// To specify an extended-length path, use the "\\?\" prefix. For example,
// "\\?\D:\very long path".
//
// The "\\?\" prefix can also be used with paths constructed according to the universal
// naming convention (UNC). To specify such a path using UNC, use the "\\?\UNC\" prefix.
// For example, "\\?\UNC\server\share", where "server" is the name of the computer and
// "share" is the name of the shared folder.
// These prefixes are not used as part of the path itself.
// They indicate that the path should be passed to the system with minimal modification,
// which means that you cannot use forward slashes to represent path separators,
// or a period to represent the current directory, or double dots to represent the
// parent directory. Because you cannot use the "\\?\" prefix with a relative path,
// relative paths are always limited to a total of MAX_PATH characters.
//
// There is no need to perform any Unicode normalization on path and file name strings
// for use by the Windows file I/O API functions because the file system treats path and
// file names as an opaque sequence of WCHARs. Any normalization that your application
// requires should be performed with this in mind, external of any calls to related
// Windows file I/O API functions.
//
// When using an API to create a directory, the specified path cannot be so long that you
// cannot append an 8.3 file name (that is, the directory name cannot exceed MAX_PATH minus 12).
//
// The shell and the file system have different requirements.
// It is possible to create a path with the Windows API that the shell user interface
// is not able to interpret properly.
//
// ============================================================================

// ============================================================================
// TODO: if (IsWindows10OrGreater() && OptInRemovedMaxPathLimit()) {}
// https://docs.microsoft.com/de-de/windows/win32/api/fileapi/nf-fileapi-getfileattributesa
// 
// These are the directory management functions that no longer have MAX_PATH restrictions
// if you opt - in to long path behavior : 
// - CreateDirectoryW
// - CreateDirectoryExW
// - GetCurrentDirectoryW
// - RemoveDirectoryW
// - SetCurrentDirectoryW
//
// These are the file management functions that no longer have MAX_PATH restrictions
// if you opt - in to long path behavior :
// - CopyFileW
// - CopyFile2
// - CopyFileExW
// - CreateFileW
// - CreateFile2
// - CreateHardLinkW
// - CreateSymbolicLinkW
// - DeleteFileW
// - FindFirstFileW
// - FindFirstFileExW
// - FindNextFileW
// - GetFileAttributesW
// - GetFileAttributesExW
// - SetFileAttributesW
// - GetFullPathNameW
// - GetLongPathNameW
// - MoveFileW
// - MoveFileExW
// - MoveFileWithProgressW
// - ReplaceFileW
// - SearchPathW
// - FindFirstFileNameW
// - FindNextFileNameW
// - FindFirstStreamW
// - FindNextStreamW
// - GetCompressedFileSizeW
// - GetFinalPathNameByHandleW
//
// ============================================================================

#if !defined(WINVER)
#define WINVER 0x602 /*_WIN32_WINNT_WIN8*/
#endif
#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x602 /*_WIN32_WINNT_WIN8*/
#endif
#if !defined(NTDDI_VERSION)
#define NTDDI_VERSION 0x06020000 /*NTDDI_WIN7*/
#endif

#define VC_EXTRALEAN 1
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

// get rid of this:
#include <shlwapi.h>

#include <fileapi.h>
#include <pathcch.h>
#include <processenv.h>
#include <stdbool.h>

#define NP3_PATH_LIB_IMPLEMENTATION 1

#include "DynStrg.h"
#include "PathLib.h"


/**************************************************/
/*                                                */
/*              PRIVATE API                       */
/*                                                */
/**************************************************/

const wchar_t* const LONG_PATH_PREFIX = L"\\\\?\\";

static bool OptInRemoveMaxPathLimit()
{
    static int s_MaxPathLimitRemoved = -1;

    switch (s_MaxPathLimitRemoved) {

    case -1: {
        // Function pointer to driver function
        BOOLEAN(WINAPI * pRtlAreLongPathsEnabled)(void) = NULL;
        s_MaxPathLimitRemoved = 0; // at least called once
        HINSTANCE const hNTdllDll = LoadLibrary(L"ntdll.dll");
        if (hNTdllDll) {
            // get the function pointer to RtlAreLongPathsEnabled
            pRtlAreLongPathsEnabled = (BOOLEAN(WINAPI*)(void))GetProcAddress(hNTdllDll, "RtlAreLongPathsEnabled");
            if (pRtlAreLongPathsEnabled != NULL) {
                s_MaxPathLimitRemoved = pRtlAreLongPathsEnabled() ? 1 : 0;
            }
            FreeLibrary(hNTdllDll);
        }
        return (s_MaxPathLimitRemoved == 1);
    }

    case 1:
        return true;

    default:
        break;
    }
    return false;
}
// ----------------------------------------------------------------------------


__forceinline HSTRINGW ToHStrgW(HPATHL hpth)
{
    if (!hpth)
        return NULL;
    return (HSTRINGW)hpth;
}
// ----------------------------------------------------------------------------


/**************************************************/
/*                                                */
/*              PUBLIC API                        */
/*                                                */
/**************************************************/

HPATHL PTHAPI Path_Allocate(const wchar_t* path)
{
    HSTRINGW hstr = StrgCreate();
    if (path) {
        StrgSet(hstr, path);
    }
    return (HPATHL)hstr;
}
// ----------------------------------------------------------------------------


void PTHAPI Path_Release(HPATHL hpth)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    StrgDestroy(hstr);
}
// ----------------------------------------------------------------------------


int PTHAPI Path_Reset(HPATHL hpth, const wchar_t* path)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    return StrgSet(hstr, path);
}
// ----------------------------------------------------------------------------


size_t PTHAPI Path_GetLength(HPATHL hpth)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    return StrgGetLength(hstr);
}
// ----------------------------------------------------------------------------


bool PTHAPI Path_RemoveFileSpec(HPATHL hpth)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hpth)
        return false;
    
    LPWSTR wbuf = StrgAccessMaxPathBuf(hstr, MAX_PATH);
    size_t cch = StrgGetAllocLength(hstr);

    bool const bOK = SUCCEEDED(PathCchRemoveFileSpec(wbuf, cch));
    StrgSanitize(hstr);

    return bOK;
}
// ----------------------------------------------------------------------------


void PTHAPI Path_ExpandEnvStrings(HPATHL hpth)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hpth)
        return;

    LPCWSTR const path_buf = StrgGet(hstr);

    HSTRINGW hExPath = StrgCreate();

    size_t const min_len = max(ExpandEnvironmentStrings(path_buf, NULL, 0), MAX_PATH);
    LPWSTR expth_buf = StrgAccessMaxPathBuf(hExPath, min_len);

    DWORD const nSize = (DWORD)StrgGetAllocLength(hExPath);
    ExpandEnvironmentStrings(path_buf, expth_buf, nSize);
    StrgSanitize(hExPath);
    
    StrgSet(hstr, expth_buf);

    StrgDestroy(hExPath);
}
// ----------------------------------------------------------------------------


bool PTHAPI Path_IsExistingFile(HPATHL hpth)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hpth)
        return false;

    HSTRINGW hxpth = StrgCopy(hstr);
    if (!OptInRemoveMaxPathLimit()) {
        StrgInsert(hxpth, 0, L"\\\\?\\");
    }
    LPCWSTR expth_buf = StrgAccessMaxPathBuf(hxpth, 1);

    DWORD const dwFileAttrib = GetFileAttributesW(expth_buf);

    bool const  bAccessOK = (dwFileAttrib != INVALID_FILE_ATTRIBUTES);

    //if (!bAccessOK) {
    //    DWORD const dwError = GetLastError();
    //    switch (dwError) {
    //    case ERROR_FILE_NOT_FOUND:
    //        break;
    //    case ERROR_PATH_NOT_FOUND:
    //        break;
    //    case ERROR_ACCESS_DENIED:
    //        break;
    //    default:
    //        break;
    //    }
    //}

    bool const bIsDirectory = (dwFileAttrib & FILE_ATTRIBUTE_DIRECTORY);

    return (bAccessOK && !bIsDirectory);
}
// ----------------------------------------------------------------------------


// ============================================================================
// ============================================================================


const wchar_t* PTHAPI Path_Get(HPATHL hpth)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    return StrgGet(hstr);
}
// ----------------------------------------------------------------------------

size_t PTHAPI Path_GetBufCount(HPATHL hpth)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    return StrgGetAllocLength(hstr);
}
// ----------------------------------------------------------------------------


// get wchar buffer with at least MAX_PATH size
// TODO: get rid of this intermediate state handler
wchar_t *PTHAPI Path_AccessBuf(HPATHL hpth, size_t len)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    return StrgAccessMaxPathBuf(hstr, max(len, MAX_PATH));
}
// ----------------------------------------------------------------------------

