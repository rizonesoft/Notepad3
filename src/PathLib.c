// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* PathLib.c                                                                   *
*   Implementation for dynamic wide char Long Path handling                   *
*   Based on DynStrg module                                                   *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2023   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

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
// - GetShortPathNameW
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
// Additional helpers (<stdlib.h> oder <wchar.h>)
// 
// wchar_t *_wfullpath(wchar_t *absPath, const wchar_t *relPath, size_t maxLength);
// 
// errno_t _wmakepath_s(wchar_t *path, size_t sizeInWords, 
//         const wchar_t *drive, const wchar_t *dir, const wchar_t *fname, const wchar_t *ext);
// 
// errno_t _wsplitpath_s(const wchar_t * path, wchar_t * drive, size_t driveNumberOfElements,
//         wchar_t *dir, size_t dirNumberOfElements, wchar_t * fname, size_t nameNumberOfElements,
//         wchar_t * ext, size_t extNumberOfElements);
// 
// ============================================================================

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601 /*_WIN32_WINNT_WIN7*/
#endif
#ifndef WINVER
#define WINVER 0x0601 /*_WIN32_WINNT_WIN7*/
#endif
#ifndef NTDDI_VERSION
#define NTDDI_VERSION 0x06010000 /*NTDDI_WIN7*/
#endif


#define NOMINMAX
#define VC_EXTRALEAN 1
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <assert.h>
#include <wchar.h>
#include <processenv.h>
#include <stdbool.h>
#include <strsafe.h>
#include <fileapi.h>

// get rid of this:
#include <shlobj.h>
#include <shellapi.h>
#include <shlwapi.h>

#include "Dialogs.h"
#include "PathLib.h"


//~ Win8.1+ only:
//~#define PATHCCH_NO_DEPRECATE 1  // <- get rid of this!
//~#include <pathcch.h>
//~#pragma comment(linker, "/defaultlib:Pathcch")


/**************************************************/
/*                                                */
/*              PRIVATE API                       */
/*                                                */
/**************************************************/

#define COUNTOF(ar) ARRAYSIZE(ar)
#define CONSTSTRGLEN(s) (COUNTOF(s) - 1)

// -------------------------------------------------

LPCWSTR const PATHUNC_PREFIX1 = L"\\\\?\\UNC\\";
LPCWSTR const PATHUNC_PREFIX2 = L"\\\\.\\UNC\\";

// TODO: ???
//LPCWSTR const VOLUME_PREFIX = L"\\\\?\\Volume{";

LPCWSTR const PATHLONG_PREFIX = L"\\\\?\\";

LPCWSTR const NETSHARE_PREFIX = L"\\\\";

LPCWSTR const PATHPARENT_PREFIX = L"..\\";

LPCWSTR const PATHDSPL_INFIX = L" ... ";

LPCWSTR const PATH_CSIDL_MYDOCUMENTS = L"%CSIDL:MYDOCUMENTS%";


// TODO: ...
#define limit_len(len) (((len) < PATHLONG_MAX_CCH) ? (len) : (PATHLONG_MAX_CCH - 1))


#define IS_VALID_HANDLE(HNDL) ((HNDL) && ((HNDL) != INVALID_HANDLE_VALUE))

// ----------------------------------------------------------------------------
//
// HPATHL == HSTRINGW  w/o typedef
//
__forceinline HSTRINGW ToHStrgW(HPATHL hpth)
{
    if (!hpth)
        return NULL;
    return (HSTRINGW)hpth;
}

#define PathGet(HPTH) StrgGet((HSTRINGW)HPTH)

// ----------------------------------------------------------------------------


//==== StrSafe extensions =======================================================

__forceinline bool StrIsEmptyW(LPCWSTR s)
{
    return (!s || (*s == L'\0'));
}

//inline size_t StringCchLenA(LPCSTR s, size_t n) {
//  n = (n ? n : STRSAFE_MAX_CCH); size_t len; return (size_t)(!s ? 0 : (SUCCEEDED(StringCchLengthA(s, n, &len)) ? len : n));
//}
__forceinline size_t StringCchLenA(LPCSTR s, const size_t n)
{
    return (s ? strnlen_s(s, (n ? n : STRSAFE_MAX_CCH)) : 0LL);
}

//inline size_t StringCchLenW(LPCWSTR s, size_t n) {
//  n = (n ? n : STRSAFE_MAX_CCH); size_t len; return (size_t)(!s ? 0 : (SUCCEEDED(StringCchLengthW(s, n, &len)) ? len : n));
//}
__forceinline size_t StringCchLenW(LPCWSTR s, const size_t n)
{
    return (s ? wcsnlen_s(s, (n ? n : STRSAFE_MAX_CCH)) : 0LL);
}

#if defined(UNICODE) || defined(_UNICODE)
#define StringCchLen(s, n) StringCchLenW((s), (n))
#else
#define StringCchLen(s, n) StringCchLenA((s), (n))
#endif

// ----------------------------------------------------------------------------

// typesafe min/max
//#define _min_(x,y) (((x) > (y)) ? (y) : (x))
#define _RETCMPMIN_  { return (x > y) ? y : x; }
__forceinline int min_i(const int x, const int y) _RETCMPMIN_
__forceinline size_t min_s(const size_t x, const size_t y) _RETCMPMIN_

//#define _max_(x,y) (((x) < (y)) ? (y) : (x))
#define _RETCMPMAX_  { return (x < y) ? y : x; }
__forceinline int max_i(const int x, const int y) _RETCMPMAX_
__forceinline size_t max_s(const size_t x, const size_t y) _RETCMPMAX_

// ----------------------------------------------------------------------------


__forceinline wchar_t _wcsgetprev(LPCWSTR str, LPCWSTR c)
{
    if (!str)
        return L'\0';
    return (c && (c > str)) ? *(c - 1) : L'\0';
}
__forceinline wchar_t _wcsgetnext(LPCWSTR c)
{
    return (c && *c) ? *(c + 1) : L'\0';
}

static void _PathFixBackslashes(LPCWSTR pstrg)
{
    LPWSTR c = wcschr(pstrg, L'/');
    while (c) {
        if ((_wcsgetprev(pstrg, c) == L':') && (_wcsgetnext(c) == L'/')) {
            c += 2;
        }
        else {
            *c = L'\\';
        }
        c = wcschr(c, L'/'); // next
    }
}
// ----------------------------------------------------------------------------


static bool HasOptInToRemoveMaxPathLimit()
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


static void PrependLongPathPrefix(HPATHL hpth_in_out, bool bForce)
{
    HSTRINGW hstr_io = ToHStrgW(hpth_in_out); // in place
    if (!hstr_io)
        return;

    if (bForce || !HasOptInToRemoveMaxPathLimit()) {
        if (bForce || (StrgGetLength(hstr_io) >= MAX_PATH_EXPLICIT)) {
            if (StrgFind(hstr_io, PATHLONG_PREFIX, 0) != 0) {
                StrgInsert(hstr_io, 0, PATHLONG_PREFIX);
            }
        }
    }
}
// ----------------------------------------------------------------------------

static LPCWSTR _Path_SkipLPPrefix(const HSTRINGW hpth_str)
{
    LPCWSTR start = StrgGet(hpth_str);
    if (wcsstr(start, PATHUNC_PREFIX1) == start) {
        start += wcslen(PATHUNC_PREFIX1) - 1;
    }
    else if (wcsstr(start, PATHLONG_PREFIX) == start) {
        start += wcslen(PATHLONG_PREFIX);
    }
    return start;
}
// ----------------------------------------------------------------------------

static void _UnExpandEnvStrgs(HSTRINGW hstr_in_out)
{
    if (!hstr_in_out) {
        return;
    }
    LPCWSTR env_var_list[] = {
        L"ALLUSERSPROFILE",
        L"APPDATA",
        L"LOCALAPPDATA",
        L"CommonProgramFiles",
        L"TEMP",
        L"USERPROFILE",
        L"COMPUTERNAME",
        L"ProgramFiles",
        L"SystemRoot",
        L"windir",
        L"OneDrive"
    };
    size_t const env_var_cnt = COUNTOF(env_var_list);
    wchar_t      var_strg[64] = { L'\0' };

    HSTRINGW htmp_str = StrgCreate(NULL);

    for (size_t i = 0; i < env_var_cnt; ++i) {

        DWORD const len = GetEnvironmentVariableW(env_var_list[i], NULL, 0);
        if (len > 0) {
            LPWSTR buf = StrgWriteAccessBuf(htmp_str, len);
            if (buf) {
                GetEnvironmentVariableW(env_var_list[i], buf, len);
                size_t const hstr_len = StrgGetLength(hstr_in_out);
                if (hstr_len >= len) {
                    size_t const idx = StrgFind(hstr_in_out, buf, 0);
                    if (idx < len) {
                        StringCchPrintfW(var_strg, COUNTOF(var_strg), L"%%%s%%", env_var_list[i]);
                        StrgReplace(hstr_in_out, buf, var_strg);
                    }
                }
                StrgSanitize(htmp_str);
            }
        }
    }
    StrgDestroy(htmp_str);
}

// ----------------------------------------------------------------------------


static bool _PathCanonicalize(HSTRINGW hstr_in_out)
{
    if (!hstr_in_out) {
        return false;
    }

    LPCWSTR start = _Path_SkipLPPrefix(hstr_in_out);

    LPWSTR const path = StrgWriteAccessBuf(hstr_in_out, 0);
    size_t const   cch = StrgGetAllocLength(hstr_in_out);

    // Replace forward slashes with backslashes
    _PathFixBackslashes(path);

    // Move back to the beginning of the string
    size_t i, j, k;
    i = j = k = (start - path);

    // Skip UNC Path
    while ((path[i] == L'\\') && path[i + 1] == L'\\') {
        i++;
    }
    j = k = i;

    // Parse the entire string
    do {
        // Backslash separator found?
        if (path[i] == L'\\' || path[i] == L'\0') {
            // "." element found?
            if ((i - j) == 1 && wcsncmp(path + j, L".", 1) == 0) {
                // Check whether the pathname is empty?
                if (k == 0) {
                    if (path[i] == L'\0') {
                        path[k++] = L'.';
                    }
                    else if (path[i] == L'\\' && path[i + 1] == L'\0') {
                        path[k++] = L'.';
                        path[k++] = L'\\';
                    }
                }
                else if (k > 1) {
                    // Remove the final slash if necessary
                    if (path[i] == L'\0')
                        k--;
                }
            }
            // ".." element found?
            else if ((i - j) == 2 && wcsncmp(path + j, L"..", 2) == 0) {
                // Check whether the pathname is empty?
                if (k == 0) {
                    path[k++] = L'.';
                    path[k++] = L'.';

                    // Append a slash if necessary
                    if (path[i] == L'\\')
                        path[k++] = L'\\';
                }
                else if (k > 1) {
                    // Search the path for the previous slash
                    for (j = 1; j < k; j++) {
                        if (path[k - j - 1] == L'\\')
                            break;
                    }

                    // Backslash separator found?
                    if (j < k) {
                        if (wcsncmp(path + k - j, L"..", 2) == 0) {
                            path[k++] = L'.';
                            path[k++] = L'.';
                        }
                        else {
                            k = k - j - 1;
                        }

                        // Append a slash if necessary
                        if (k == 0 && path[0] == L'\\')
                            path[k++] = L'\\';
                        else if (path[i] == L'\\')
                            path[k++] = L'\\';
                    }
                    // No slash separator found?
                    else {
                        if (k == 3 && wcsncmp(path, L"..", 2) == 0) {
                            path[k++] = L'.';
                            path[k++] = L'.';

                            // Append a slash if necessary
                            if (path[i] == L'\\')
                                path[k++] = L'\\';
                        }
                        else if (path[i] == L'\0') {
                            k = 0;
                            path[k++] = L'.';
                        }
                        else if (path[i] == L'\\' && path[i + 1] == L'\0') {
                            k = 0;
                            path[k++] = L'.';
                            path[k++] = L'\\';
                        }
                        else {
                            k = 0;
                        }
                    }
                }
            }
            else {
                // Copy directory name
                wmemmove_s(path + k, cch - k, path + j, i - j);

                // Advance write pointer
                k += (i - j);

                // Append a slash if necessary
                if (path[i] == L'\\')
                    path[k++] = L'\\';
            }

            // Move to the next token
            while (path[i] == L'\\')
                i++;
            j = i;
        }

    } while (path[i++] != L'\0');

    // Properly terminate the string with a NULL character
    path[k] = '\0';

    StrgSanitize(hstr_in_out);

    return true;
}
// ----------------------------------------------------------------------------



// Determines whether a path string refers to the root of a volume.
//
//  Path                      PathXCchIsRoot() 
// ----------------------------------------------------------
//  "c:"                          FALSE          ==
//  "c:\"                         TRUE           ==
//  "c:\path1"                    FALSE          ==
//  "c:\path1\"                   FALSE          --
//  "\path1"                      FALSE   (MSDN DOC) -> TRUE
//  "path1"                       FALSE          ==
//  "\\path1\path2"               TRUE           ==
//  "\\path1\path2\"              FALSE          ==
//  "\\path1\path2\path3"         FALSE          ==
//  "\\path1"                     TRUE           ==
//  "\\path1\"                    FALSE          ==
//  "\\"                          TRUE           ==
//  "\\?\UNC\"                    TRUE           ==
//  "\\?\UNC\path1"               TRUE           ==
//  "\\?\UNC\path1\"              FALSE          ==
//  "\\?\UNC\path1\path2"         TRUE           ==
//  "\\?\UNC\path1\path2\"        FALSE          ==
//  "\\?\UNC\path1\path2\path3"   FALSE          ==
//  "\\?\c:"                      FALSE          ==
//  "\\?\c:\"                     TRUE           ==
//  "\\?\c:\path1"                FALSE          ==
//  "\\?\c:\path1\"               FALSE          --
//  "\\?\Volume{guid}\"           TRUE           
//  "\\?\Volume{guid}"            FALSE          
//  "\\?\Volume{guid}\path1"      FALSE          
//  NULL                          FALSE          ==
//  ""                            FALSE          ==
// ----------------------------------------------------------
//

static LPCWSTR _Path_IsValidUNC(const HPATHL hpth, bool* isUNC_out)
{
    if (!hpth) {
        return NULL; // false
    }

    if (Path_GetLength(hpth) == 0) {
        return PathGet(hpth);
    }

    LPCWSTR start = PathGet(hpth);
    LPCWSTR const endz = start + Path_GetLength(hpth); // terminating zero (L'\0')


    bool isUncOrNetShare = false;

    if ((wcsstr(start, PATHUNC_PREFIX1) == start) ||
        (wcsstr(start, PATHUNC_PREFIX2) == start)) {
        start += wcslen(PATHUNC_PREFIX1) + 1;
        isUncOrNetShare = true;
    }

    /// _Path_SkipLPPrefix()
    if (wcsstr(start, PATHLONG_PREFIX) == start) {
        start += wcslen(PATHLONG_PREFIX) + 1;
    }

    if (wcsstr(start, NETSHARE_PREFIX) == start) {
        start += wcslen(NETSHARE_PREFIX) + 1;
        isUncOrNetShare = true;
    }

    // check for valid net-server prefix
    //     \\<server-name>\<share>\...
    if (isUncOrNetShare) {

        // skip <server-name>
        LPCWSTR nextbs = wcschr(start, L'\\');
        isUncOrNetShare = (nextbs && (nextbs > start));
        start = isUncOrNetShare ? (nextbs + 1) : endz;

        // skip <share-name>
        nextbs = wcschr(start, L'\\');
        isUncOrNetShare = (nextbs && (nextbs > start));
        start = isUncOrNetShare ? (nextbs + 1) : endz;
    }

    if (isUNC_out) {
        *isUNC_out = isUncOrNetShare;
    }

    return start;
}
// ----------------------------------------------------------------------------

#if 0
__forceinline LPCWSTR _Path_SkipRoot(const HPATHL hpth)
{
    LPCWSTR path = NULL;
    if (SUCCEEDED(PathCchSkipRoot(Path_Get(hpth), &path))) {
        assert(path != PathGet(hpth));
        return path; // *root == L'\0'  =>  PathCchIsRoot()==TRUE
    }
    return PathGet(hpth);
}
#endif

//
// needs converted forward slashes
//
static LPCWSTR _Path_SkipRoot(const HPATHL hpth)
{
    if (!hpth) {
        assert(hpth);
        return NULL; // false
    }

    if (Path_GetLength(hpth) == 0) {
        return PathGet(hpth);
    }

    bool                 isUncOrNetShare = false;
    LPCWSTR       start = _Path_IsValidUNC(hpth, &isUncOrNetShare);
    LPCWSTR const endz = start + Path_GetLength(hpth); // terminating zero (L'\0')

    if (*start && (*(start + 1) == L':')) { // has drive letter
        start += 2;
        if ((*start == L'\\') && !isUncOrNetShare) {
            return (start + 1);
        }
        return endz; // invalid root
    }

    // anything else ?
    return start;
}
// ----------------------------------------------------------------------------


//=============================================================================
//
//  _Path_IsRelative()
//  TODO: §§§ make LongPath version instead of slicing MAX_PATH
//
static bool _Path_IsRelative(const HPATHL hpth)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hstr)
        return true; // empty is relative

    LPCWSTR const skip = _Path_SkipLPPrefix(hstr);

    bool res = false;
    if (StrgGetLength(hstr) >= MAX_PATH_EXPLICIT) {
        // hack for MAX_PATH_EXPLICIT limit
        wchar_t const wch = StrgGetAt(hstr, MAX_PATH_EXPLICIT);
        StrgSetAt(hstr, MAX_PATH_EXPLICIT, L'\0');
        res = PathIsRelativeW(skip);
        StrgSetAt(hstr, MAX_PATH_EXPLICIT, wch);
    }
    else {
        res = PathIsRelativeW(skip);
    }
    return res;
}
// ----------------------------------------------------------------------------



/**************************************************/
/*                                                */
/*              PUBLIC API                        */
/*                                                */
/**************************************************/

HPATHL PTHAPI Path_Allocate(LPCWSTR path)
{
    return (HPATHL)StrgCreate(path);
}
// ----------------------------------------------------------------------------


void PTHAPI Path_Release(HPATHL hpth)
{
    StrgDestroy(ToHStrgW(hpth));
}
// ----------------------------------------------------------------------------


void PTHAPI Path_Empty(HPATHL hpth_in_out, bool truncate)
{
    StrgEmpty(ToHStrgW(hpth_in_out), truncate);
}
// ----------------------------------------------------------------------------


int PTHAPI Path_Reset(HPATHL hpth_in_out, LPCWSTR path)
{
    return (path ? StrgReset(ToHStrgW(hpth_in_out), path) : 0);
}
// ----------------------------------------------------------------------------


size_t PTHAPI Path_GetLength(HPATHL hpth)
{
    return StrgGetLength(ToHStrgW(hpth));
}
// ----------------------------------------------------------------------------


HPATHL PTHAPI Path_Copy(const HPATHL hpth)
{
    return Path_Allocate(PathGet(hpth));
}
// ----------------------------------------------------------------------------


bool PTHAPI Path_Append(HPATHL hpth_in_out, LPCWSTR more)
{
    HSTRINGW hstr_io = ToHStrgW(hpth_in_out);
    if (!hstr_io)
        return false;

    size_t const hstr_len = StrgGetLength(hstr_io);
    size_t const hmore_len = StringCchLen(more, 0);
    if (!hmore_len) {
        return true;
    }

    LPWSTR const wbuf = StrgWriteAccessBuf(hstr_io, hstr_len + hmore_len + wcslen(PATHLONG_PREFIX) + 2);
    size_t const cch = StrgGetAllocLength(hstr_io);

    // append directory separator
    if (hstr_len > 0) {
        if (((wbuf[hstr_len - 1] != L'/') && (wbuf[hstr_len - 1] != L'\\')) &&
            ((more[0] != L'/') && (more[0] != L'\\'))) {
            wbuf[hstr_len] = L'\\';
            wbuf[hstr_len + 1] = L'\0';
        }
    }
    //else {
    //    wbuf[0] = L'\\';
    //    wbuf[1] = L'\0';
    //}

    StringCchCatW(wbuf, cch, more);
    StrgSanitize(hstr_io);

    return Path_Canonicalize(hpth_in_out);
}
// ----------------------------------------------------------------------------


void PTHAPI Path_Swap(HPATHL hpth1, HPATHL hpth2)
{
    HSTRINGW hstr1 = ToHStrgW(hpth1);
    HSTRINGW hstr2 = ToHStrgW(hpth2);
    assert(hstr1 && hstr2);
    if (!hstr1 || !hstr2)
        return;
    StrgSwap(hstr1, hstr2);
}
// ----------------------------------------------------------------------------


void PTHAPI Path_FreeExtra(HPATHL hpth_in_out, size_t keep_length)
{
    HSTRINGW hstr_io = ToHStrgW(hpth_in_out);
    if (!hstr_io)
        return;
    StrgFreeExtra(hstr_io, keep_length);
}
// ----------------------------------------------------------------------------


//=============================================================================
//
// PathFixBackslashes() - in place conversion
//
bool PTHAPI Path_FixBackslashes(HPATHL hpth_in_out)
{
    HSTRINGW hstr_io = ToHStrgW(hpth_in_out); // inplace hpth_in_out
    if (!hstr_io)
        return false;

    LPWSTR const lpsz = StrgWriteAccessBuf(hstr_io, 0);

    WCHAR* c = lpsz;
    bool   bFixed = false;
    while ((c = StrChr(c, L'/')) != NULL) {
        if (c != lpsz && c[-1] == L':' && c[1] == L'/') {
            c += 2;
        }
        else {
            *c++ = L'\\';
            bFixed = true;
        }
    }
    //~StrgSanitize(hstr_io); : '/' replaced by '\' only
    return bFixed;
}


//=============================================================================


// With untrusted input, this function by itself, cannot be used to convert
// paths into a form that can be compared with other paths for sub-path or identity.
// Callers that need that ability should convert forward to back slashes before
// using this function.

bool PTHAPI Path_Canonicalize(HPATHL hpth_in_out)
{
    if (!hpth_in_out) {
        return false;
    }
    Path_FixBackslashes(hpth_in_out);

    HPATHL hpth_cpy = Path_Allocate(PathGet(hpth_in_out));
    HSTRINGW hstr_cpy = ToHStrgW(hpth_cpy);

    //~ PathXCchCanonicalizeEx() does not convert forward slashes (/) into back slashes (\).
    //~StrgReplaceCh(hstr_cpy, L'/', L'\\');
    //~_PathFixBackslashes(StrgGet(hstr_cpy));
    // but static _PathCanonicalize() does!

    // remove quotes
    StrgTrim(hstr_cpy, L'"');

    // internal buffer access
    //~LPWSTR       wbuf_out = StrgWriteAccessBuf(hstr_cpy, 0);
    //~size_t const cch_out = StrgGetAllocLength(hstr_cpy);
    //~DWORD const dwFlags = PATHCCH_ALLOW_LONG_PATHS;
    //~//  Windows 10, version 1703:
    //~//  PATHCCH_FORCE_ENABLE_LONG_NAME_PROCESS | PATHCCH_ENSURE_IS_EXTENDED_LENGTH_PATH
    //~//  PATHCCH_ENSURE_TRAILING_SLASH
    //~bool const res = SUCCEEDED(PathXCchCanonicalizeEx(wbuf_out, cch_out, PathGet(hpth_in_out), dwFlags));
    //~StrgSanitize(hstr_cpy);

    bool const res = _PathCanonicalize(hstr_cpy);
    StrgSanitize(hstr_cpy);

    // canonicalize prefix
    PrependLongPathPrefix(hpth_cpy, false);

    Path_Swap(hpth_in_out, hpth_cpy);

    Path_Release(hpth_cpy);

    return res;
}
// ----------------------------------------------------------------------------


bool PTHAPI Path_IsEmpty(const HPATHL hpth)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hstr)
        return true;
    return StrgIsEmpty(hstr);
}
// ----------------------------------------------------------------------------


//  TODO: §§§ make LongPath version instead of slicing MAX_PATH
bool PTHAPI Path_IsRoot(const HPATHL hpth)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hstr)
        return false;

    //PrependLongPathPrefix(hpth, false);

    ///WCHAR wchPathBegin[MAX_PATH_EXPLICIT] = { L'\0' };
    ///StringCchCopy(wchPathBegin, COUNTOF(wchPathBegin), StrgGet(hstr));
    ///return PathIsRootW(wchPathBegin);

    bool res = false;
    if (StrgGetLength(hstr) >= MAX_PATH_EXPLICIT) {
        // hack for MAX_PATH_EXPLICIT limit
        wchar_t const wch = StrgGetAt(hstr, MAX_PATH_EXPLICIT);
        StrgSetAt(hstr, MAX_PATH_EXPLICIT, L'\0');
        res = PathIsRootW(StrgGet(hstr));
        StrgSetAt(hstr, MAX_PATH_EXPLICIT, wch);
    }
    else {
        res = PathIsRootW(StrgGet(hstr));
    }

    return res;

}
// ----------------------------------------------------------------------------


bool PTHAPI Path_IsValidUNC(const HPATHL hpth)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hstr)
        return false;

    //PrependLongPathPrefix(hpth, false);

    bool isUncOrNetShare = false;
    _Path_IsValidUNC(hpth, &isUncOrNetShare);

    return isUncOrNetShare;
}
// ----------------------------------------------------------------------------


//=============================================================================
//
//  Path_IsExistingDirectory()
//
bool PTHAPI Path_IsExistingDirectory(const HPATHL hpth)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hstr)
        return false;

    HPATHL hxpth = Path_Allocate(StrgGet(hstr));
    PrependLongPathPrefix(hxpth, false);
    DWORD const dwFileAttrib = GetFileAttributesW(PathGet(hxpth));
    Path_Release(hxpth);

    ///bool const  bAccessOK = (dwFileAttrib != INVALID_FILE_ATTRIBUTES);
    ///if (!bAccessOK) {
    ///    DWORD const dwError = GetLastError();
    ///    switch (dwError) {
    ///    case ERROR_FILE_NOT_FOUND:
    ///        break;
    ///    case ERROR_PATH_NOT_FOUND:
    ///        break;
    ///    case ERROR_ACCESS_DENIED:
    ///        break;
    ///    default:
    ///        break;
    ///    }
    ///}

    return IsExistingDirectory(dwFileAttrib);
}
// ----------------------------------------------------------------------------


int PTHAPI Path_StrgComparePathNormalized(const HPATHL hpth1, const HPATHL hpth2)
{
    if (Path_IsEmpty(hpth1)) { return -1; }
    if (Path_IsEmpty(hpth2)) { return +1; }

    size_t const max_len = min_s(Path_GetLength(hpth1), Path_GetLength(hpth2)) + 1;
    //~return wcsncmp(Path_Get(hpth1), Path_Get(hpth2), max_len);
    return _wcsnicmp(Path_Get(hpth1), Path_Get(hpth2), max_len);
}
// ----------------------------------------------------------------------------


int PTHAPI Path_StrgComparePath(const HPATHL hpth1, const HPATHL hpth2, const HPATHL hpth_wrkdir)
{
    if (Path_IsEmpty(hpth1)) { return -1; }
    if (Path_IsEmpty(hpth2)) { return +1; }

    HPATHL hpth1_tmp = Path_Copy(hpth1);
    HPATHL hpth2_tmp = Path_Copy(hpth2);

    Path_NormalizeEx(hpth1_tmp, hpth_wrkdir, true, false);
    Path_NormalizeEx(hpth2_tmp, hpth_wrkdir, true, false);

    int const cmp = Path_StrgComparePathNormalized(hpth1_tmp, hpth2_tmp);

    Path_Release(hpth1_tmp);
    Path_Release(hpth2_tmp);

    return cmp;
}
// ----------------------------------------------------------------------------


bool PTHAPI Path_RemoveFileSpec(HPATHL hpth_in_out)
{
    HSTRINGW hstr_io = ToHStrgW(hpth_in_out);
    if (!hstr_io)
        return false;
    
    LPWSTR wbuf = StrgWriteAccessBuf(hstr_io, 0); // no need to ReAlloc
    //size_t cch = StrgGetAllocLength(hstr_io);

    LPCWSTR pfile = Path_FindFileName(hpth_in_out);

    if (pfile > wbuf) {
        StrgDelete(hstr_io, (size_t)(pfile - wbuf), StrgGetLength(hstr_io));
    }
    StrgSanitize(hstr_io);

    StrgTrimRight(hstr_io, L'\\');
    StrgTrimRight(hstr_io, L'/');

    return true;
}
// ----------------------------------------------------------------------------


bool PTHAPI Path_StripPath(HPATHL hpth)  // get filename only
{
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hstr)
        return false;

    LPCWSTR wbuf = StrgWriteAccessBuf(hstr, 0);
    LPCWSTR pfile = Path_FindFileName(hpth);

    size_t const idx = (size_t)(pfile - wbuf);
    if (idx != 0) {
        StrgDelete(hstr, 0, idx);
        return true;
    }
    return false;
}
// ----------------------------------------------------------------------------


bool PTHAPI Path_RenameExtension(HPATHL hpth_in_out, LPCWSTR ext)
{
    HSTRINGW hstr_io = ToHStrgW(hpth_in_out);
    if (!hstr_io)
        return false;
    
    size_t const ext_len = (ext ? wcslen(ext) : 0);
    bool const   hasdot = (ext_len && (ext[0] == L'.'));

    size_t const hstr_len = StrgGetLength(hstr_io);

    LPWSTR wbuf = StrgWriteAccessBuf(hstr_io, hstr_len + ext_len + 1);
    size_t cch = StrgGetAllocLength(hstr_io);

    LPWSTR const pdot = (LPWSTR)Path_FindExtension(hpth_in_out);
    if (pdot) {
        *pdot = L'\0';
    }
    if (ext_len) {
        if (hasdot) {
            StringCchCatW(wbuf, cch, ext);
        }
        else {
            StringCchCatW(wbuf, cch, L".");
            StringCchCatW(wbuf, cch, ext);
        }
    }
    StrgSanitize(hstr_io);

    PrependLongPathPrefix(hpth_in_out, false);

    return true;
}
// ----------------------------------------------------------------------------


void PTHAPI Path_ExpandEnvStrings(HPATHL hpth_in_out)
{
    HSTRINGW hstr_io = ToHStrgW(hpth_in_out);
    if (!hstr_io)
        return;
    
    ExpandEnvironmentStrgs(hstr_io, true);
}
// ----------------------------------------------------------------------------


void PTHAPI Path_UnExpandEnvStrings(HPATHL hpth)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hstr)
        return;
    
    _UnExpandEnvStrgs(hstr);
}
// ----------------------------------------------------------------------------


void PTHAPI Path_GetModuleFilePath(HPATHL hpth_out)
{
    static HPATHL mod_path = NULL;

    if (!hpth_out) {
        if (mod_path) {
            Path_Release(mod_path);
            mod_path = NULL;
        }
        else {
            assert(hpth_out != NULL);
        }
        return;
    }

    if (!mod_path) {
        mod_path = Path_Allocate(NULL);
        LPWSTR const buf = Path_WriteAccessBuf(mod_path, PATHLONG_MAX_CCH);
        GetModuleFileNameW(NULL, buf, PATHLONG_MAX_CCH);
        Path_Sanitize(mod_path);
        Path_Canonicalize(mod_path);
        Path_FreeExtra(mod_path, 0);
    }

    Path_Reset(hpth_out, PathGet(mod_path));
}
// ----------------------------------------------------------------------------


//=============================================================================
//
//  Path_GetAppDirectory()
//
void PTHAPI Path_GetAppDirectory(HPATHL hpth_out)
{
    static HPATHL happdir_path = NULL;

    if (!hpth_out) {
        if (happdir_path) {
            Path_Release(happdir_path);
            happdir_path = NULL;
        }
        else {
            assert(hpth_out != NULL);
        }
        return;
    }

    if (!happdir_path) {
        happdir_path = Path_Allocate(NULL);
        Path_GetModuleFilePath(happdir_path);
        Path_RemoveFileSpec(happdir_path);
    }

    Path_Reset(hpth_out, PathGet(happdir_path));
}
// ----------------------------------------------------------------------------


bool PTHAPI Path_IsRelative(const HPATHL hpath)
{
    return _Path_IsRelative(hpath);
}
// ----------------------------------------------------------------------------


bool PTHAPI Path_IsPrefix(const HPATHL hprefix, const HPATHL hpth)
{
    HPATHL hprfx_pth = Path_Allocate(PathGet(hprefix));
    HPATHL hsrch_pth = Path_Allocate(PathGet(hpth));

    Path_Canonicalize(hprfx_pth);
    Path_Canonicalize(hsrch_pth);

    size_t const beg = 0;
    bool const   res = (StrgFind(ToHStrgW(hsrch_pth), PathGet(hprfx_pth), beg) == beg);

    Path_Release(hsrch_pth);
    Path_Release(hprfx_pth);

    return res;
}
// ----------------------------------------------------------------------------


size_t PTHAPI Path_CommonPrefix(const HPATHL hpth1, const HPATHL hpth2, HPATHL hpfx_out)
{
    HSTRINGW hpth1_str = ToHStrgW(hpth1);
    HSTRINGW hpth2_str = ToHStrgW(hpth2);

    size_t ovl_cnt = min_s(StrgGetLength(hpth1_str), StrgGetLength(hpth2_str));

    HSTRINGW hout_str = ToHStrgW(hpfx_out);
    bool const cpy_out = (hout_str != NULL);
    LPWSTR const out_buf = cpy_out ? StrgWriteAccessBuf(hout_str, ovl_cnt + 1) : NULL;

    LPCWSTR p1 = StrgGet(hpth1_str);
    LPCWSTR p2 = StrgGet(hpth2_str);

    size_t cnt = 0;
    while (p1 && p2 && (*p1 == *p2) && (cnt < ovl_cnt)) {
        if (cpy_out) {
            out_buf[cnt] = *p1;
        }
        ++cnt; ++p1; ++p2;
    }
    if (cpy_out) {
        out_buf[cnt] = L'\0';
    }
    StrgSanitize(hout_str);
    return cnt;
}
// ----------------------------------------------------------------------------


// ============================================================================
// 
// Path_FindExtension()
// 
// when this function returns successfully, points to the "." character that
// precedes the extension within pszPath.
// If no extension is found, it points to the string's terminating null character.
//
LPCWSTR PTHAPI Path_FindExtension(const HPATHL hpth)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hstr)
        return NULL;

    LPWSTR wbuf = StrgWriteAccessBuf(hstr, 0);
    //size_t const cch = StrgGetAllocLength(hstr);

    ///PathXCchFindExtension(StrgGet(hstr), StrgGetAllocLength(hstr), &pext);
    LPCWSTR pfile = Path_FindFileName(hpth);
    LPWSTR const pdot = pfile ? wcsrchr(pfile, L'.') : NULL;

    StrgSanitize(hstr);
    return pdot ? pdot : &wbuf[StrgGetLength(hstr)];
}
// ----------------------------------------------------------------------------


LPCWSTR PTHAPI Path_FindFileName(const HPATHL hpth)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hstr)
        return NULL;

    LPCWSTR pstart = _Path_SkipRoot(hpth);
    if (!pstart  || *pstart == L'\0') {
        return pstart;
    }

    ///bool const res = SUCCEEDED(PathXCchRemoveFileSpec(wbuf, cch));
    LPCWSTR const plbs = wcsrchr(pstart, L'\\');
    LPCWSTR const plfs = wcsrchr(pstart, L'/');
    if (plbs || plfs) {
        if (plbs && plfs) {
            if (plbs >= plfs) {
                return (plbs + 1);
            }
            return (plfs + 1);
        }
        if (plbs) {
            return (plbs + 1);
        }
        return (plfs + 1);
    }
    return pstart;
}
// ----------------------------------------------------------------------------


bool PTHAPI Path_QuoteSpaces(HPATHL hpth_in_out, bool bForceQuotes)
{
    HSTRINGW hstr_io = ToHStrgW(hpth_in_out);
    if (!hstr_io)
        return false;

    bool const found = bForceQuotes ? true : (StrgFindCh(hstr_io, L' ', 0) != STRINGW_INVALID_IDX);
    if (found) {
        if (StrgGetAt(hstr_io, 0) != L'"') {
            StrgInsert(hstr_io, 0, L"\"");
            StrgCat(hstr_io, L"\"");
        }
    }
    return found;
}
// ----------------------------------------------------------------------------


void PTHAPI Path_UnQuoteSpaces(HPATHL hpth_in_out)
{
    HSTRINGW hstr_io = ToHStrgW(hpth_in_out);
    if (!hstr_io || StrgIsEmpty(hstr_io))
        return;

    StrgTrimLeft(hstr_io, L'"');
    StrgTrimRight(hstr_io, L'"');
}
// ----------------------------------------------------------------------------


int PTHAPI Path_GetDriveNumber(const HPATHL hpth)
{
    int      res = -1;
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hstr || StrgIsEmpty(hstr))
        return res;

    LPCWSTR const lpszPath = StrgGet(hstr);
    LPCWSTR const colon = wcschr(lpszPath, L':');
    if (colon && ((colon - 1) == lpszPath)) {
        WCHAR buf[2] = { 0 };
        StringCchCopy(buf, COUNTOF(buf), lpszPath);
        if (_wcsupr_s(buf, COUNTOF(buf)) == 0) {
            res = (int)buf[0] - (int)L'A';
            res = (res > 25) ? -1 : res;
        }
    }
    return res;
}
// ----------------------------------------------------------------------------


bool PTHAPI Path_IsUNC(const HPATHL hpth)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hstr || StrgIsEmpty(hstr))
        return false;

    WCHAR maxPath[MAX_PATH];
    StringCchCopy(maxPath, COUNTOF(maxPath), StrgGet(hstr));

    return PathIsUNC(maxPath);
}
// ----------------------------------------------------------------------------


wchar_t PTHAPI Path_GetDriveLetterByNumber(const int number)
{
    wchar_t const DriveLetter[27] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int const num = max_i(0, min_i(number, COUNTOF(DriveLetter) - 2));
    return DriveLetter[num];
}
// ----------------------------------------------------------------------------


DWORD PTHAPI Path_GetFileAttributes(const HPATHL hpth)
{
    return GetFileAttributesW(PathGet(hpth));
}
// ----------------------------------------------------------------------------


bool PTHAPI Path_SetFileAttributes(HPATHL hpth, DWORD dwAttributes)
{
    return SetFileAttributesW(PathGet(hpth), dwAttributes);
}
// ----------------------------------------------------------------------------


bool PTHAPI Path_StripToRoot(HPATHL hpth_in_out)
{
    HSTRINGW hstr_io = ToHStrgW(hpth_in_out);
    if (!hstr_io)
        return false;

    WCHAR maxPath[MAX_PATH];
    StringCchCopy(maxPath, COUNTOF(maxPath), StrgGet(hstr_io));

    if (PathStripToRoot(maxPath)) {
        Path_Reset(hpth_in_out, maxPath);
        return true;
    }
    return false;
}
// ----------------------------------------------------------------------------


bool PTHAPI Path_GetCurrentDirectory(HPATHL hpth_out)
{
    static HPATHL wrk_path = NULL;

    HSTRINGW hstr = ToHStrgW(hpth_out);
    if (!hstr) {
        if (wrk_path) {
            Path_Release(wrk_path);
            wrk_path = NULL;
        }
        else {
            assert(hstr != NULL);
        }
        return true;
    }

    if (!wrk_path) {
        wrk_path = Path_Allocate(NULL);
        HSTRINGW const hwrk_str = ToHStrgW(wrk_path);
        LPWSTR const buf = StrgWriteAccessBuf(hwrk_str, PATHLONG_MAX_CCH);
        GetCurrentDirectoryW(PATHLONG_MAX_CCH, buf);
        StrgSanitize(hwrk_str);
        StrgFreeExtra(hwrk_str, 0);
    }

    Path_Reset(hpth_out, PathGet(wrk_path));
    return (Path_GetLength(wrk_path) != 0);
}
// ----------------------------------------------------------------------------

// use only, if neccessary
size_t PTHAPI Path_ToShortPathName(HPATHL hpth_in_out)
{
    HSTRINGW hstr_io = ToHStrgW(hpth_in_out);
    if (!hstr_io)
        return 0;

    DWORD const _len = GetShortPathNameW(StrgGet(hstr_io), NULL, 0);
    if (!_len) {
#if (defined(_DEBUG) || defined(DEBUG)) && !defined(NDEBUG)
        MsgBoxLastError(L"Path_ToShortPathName()", 0);
#endif // DEBUG
        return 0;
    }
    LPWSTR const buf = StrgWriteAccessBuf(hstr_io, _len);
    
    DWORD const len = GetShortPathNameW(buf, buf, (DWORD)StrgGetAllocLength(hstr_io));
    StrgSanitize(hstr_io);

    return len;
}
// ----------------------------------------------------------------------------


size_t PTHAPI Path_GetLongPathNameEx(HPATHL hpth_in_out)
{
    HSTRINGW hstr_io = ToHStrgW(hpth_in_out);
    if (!hstr_io)
        return 0;

    PrependLongPathPrefix(hpth_in_out, false); // TODO: check or true ?

    DWORD const _len = GetLongPathNameW(StrgGet(hstr_io), NULL, 0);
    if (!_len) {
        //MsgBoxLastError(L"Path_GetLongPathNameEx()", 0);
        return 0;
    }
    LPWSTR const buf = StrgWriteAccessBuf(hstr_io, _len);
    size_t const res = (size_t)GetLongPathNameW(buf, buf, _len);
    StrgSanitize(hstr_io);

    if (res > 2ULL) {
        LPWSTR const pos = wcschr(buf, L':');
        if (pos && (pos > buf)) {
            CharUpperBuffW(pos - 1, 1);
        }
    }
    return res;
}

#if 0
size_t PTHAPI GetLongPathNameEx(LPWSTR lpszPath, const size_t cchBuffer)
{
    HPATHL      hpth = Path_Allocate(lpszPath);
    size_t const res = Path_GetLongPathNameEx(hpth);
    if (res) {
        StringCchCopyW(lpszPath, cchBuffer, PathGet(hpth));
    }
    Path_Release(hpth);
    return res;
}
#endif
// ----------------------------------------------------------------------------


//=============================================================================
//
//  Path_GetDisplayName()
//
void PTHAPI Path_GetDisplayName(LPWSTR lpszDisplayName, const DWORD cchDisplayName, const HPATHL hpth, LPCWSTR repl, bool bStripPath)
{
    if (!lpszDisplayName || (cchDisplayName == 0)) {
        return;
    }
    if (Path_IsEmpty(hpth)) {
        if (!StrIsEmptyW(repl)) {
            StringCchCopyW(lpszDisplayName, cchDisplayName, repl);
        }
        return;
    }

    HPATHL hfnam_pth = Path_Copy(hpth);
    if (bStripPath) {
        Path_StripPath(hfnam_pth);
    }
    size_t const fnam_len = Path_GetLength(hfnam_pth);

    if (fnam_len >= cchDisplayName) {
        // Explorer like display name ???
        HPATHL       hpart_pth = Path_Allocate(PathGet(hfnam_pth));
        HSTRINGW     hpart_str = ToHStrgW(hpart_pth);
        size_t const split_idx = (cchDisplayName >> 1) - wcslen(PATHDSPL_INFIX);
        StrgDelete(hpart_str, split_idx, (fnam_len - cchDisplayName + (wcslen(PATHDSPL_INFIX) << 1)));
        StrgInsert(hpart_str, split_idx, PATHDSPL_INFIX);
        StringCchCopyW(lpszDisplayName, cchDisplayName, StrgGet(hpart_str));
        Path_Release(hpart_pth);
    }
    else {
        StringCchCopyW(lpszDisplayName, cchDisplayName, PathGet(hfnam_pth));
    }
    Path_Release(hfnam_pth);
}
// ----------------------------------------------------------------------------


// ============================================================================
//
//  Path_GetLnkPath()
//
//  Try to get the path to which a lnk-file is linked
//
bool PTHAPI Path_GetLnkPath(const HPATHL hLnkFilePth, HPATHL hResPath_out)
{
    bool bSucceeded = false;

    IShellLink* psl = NULL;
    if (SUCCEEDED(CoCreateInstance(&CLSID_ShellLink, NULL,
            CLSCTX_INPROC_SERVER,
            &IID_IShellLink, (void**)&psl))) {

        IPersistFile* ppf = NULL;
        if (SUCCEEDED(psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, (void**)&ppf))) {

            HPATHL       hres_pth = Path_Allocate(NULL);
            LPWSTR const res_buf = Path_WriteAccessBuf(hres_pth, PATHLONG_MAX_CCH);

            /*MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,pszLnkFile,-1,wsz,MAX_PATH_EXPLICIT);*/

            WIN32_FIND_DATA fd = { 0 };
            if (SUCCEEDED(ppf->lpVtbl->Load(ppf, Path_Get(hLnkFilePth), STGM_READ))) {
                if (NOERROR == psl->lpVtbl->GetPath(psl, res_buf, (int)Path_GetBufCount(hres_pth), &fd, 0)) {
                    Path_Sanitize(hres_pth);
                    if (hResPath_out) {
                        Path_FreeExtra(hres_pth, MAX_PATH_EXPLICIT);
                        Path_Swap(hResPath_out, hres_pth);
                    }
                    // This additional check seems reasonable
                    bSucceeded = hResPath_out ? Path_IsNotEmpty(hResPath_out) : true;
                }
            }
            Path_Release(hres_pth);
            ppf->lpVtbl->Release(ppf);
        }
        psl->lpVtbl->Release(psl);
    }

    if (bSucceeded && hResPath_out) {
        Path_Canonicalize(hResPath_out);
    }

    return bSucceeded;
}


// ============================================================================
//
//  Path_IsLnkFile()
//
//  Determine whether hpth is a Windows Shell Link File by
//  comparing the filename extension with L".lnk"
//
bool PTHAPI Path_IsLnkFile(const HPATHL hpth)
{
    HSTRINGW hstr = ToHStrgW(hpth); // inplace hpth_in_out
    if (!hstr) {
        return false;
    }
    if (_wcsicmp(Path_FindExtension(hpth), L".lnk") != 0) {
        return false;
    }
    return Path_GetLnkPath(hpth, NULL);
}
// ----------------------------------------------------------------------------


// ============================================================================
//
//  Path_IsLnkToDirectory()
//
//  Determine wheter hlnk_pth is a Windows Shell Link File which refers to a directory
//
bool PTHAPI Path_IsLnkToDirectory(const HPATHL hlnk_pth, HPATHL hpth_out)
{
    if (!hlnk_pth) {
        return false;
    }

    bool res = false;
    if (Path_IsLnkFile(hlnk_pth)) {
        HPATHL       hres_pth = Path_Allocate(NULL);
        if (Path_GetLnkPath(hlnk_pth, hres_pth)) {
            res = Path_IsExistingDirectory(hres_pth);
        }
        if (res && hpth_out) {
            Path_Swap(hpth_out, hres_pth);
        }
        Path_Release(hres_pth);
    }
    return res;
}
// ----------------------------------------------------------------------------


// ============================================================================
//
//  Path_CreateFavLnk()
//  Create a Notepad3 favorites link
//
bool PTHAPI Path_CreateFavLnk(LPCWSTR lpszLinkName, const HPATHL hTargetPth, const HPATHL hDirPth)
{
    if (StrIsEmptyW(lpszLinkName)) {
        return true;
    }

    bool bSucceeded = false;

    HPATHL hlnk_pth = Path_Copy(hDirPth);
    Path_Append(hlnk_pth, lpszLinkName);
    Path_RenameExtension(hlnk_pth, L".lnk");

    if (!Path_IsExistingFile(hlnk_pth)) {

        IShellLink* psl = NULL;
        if (SUCCEEDED(CoCreateInstance(&CLSID_ShellLink, NULL,
                CLSCTX_INPROC_SERVER,
                &IID_IShellLink, (void**)&psl)))
        {
            IPersistFile* ppf = NULL;
            if (SUCCEEDED(psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, (void**)&ppf)))
            {
                /*MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,tchLnkFileName,-1,wsz,MAX_PATH_EXPLICIT);*/

                psl->lpVtbl->SetPath(psl, Path_Get(hTargetPth));

                if (SUCCEEDED(ppf->lpVtbl->Save(ppf, Path_Get(hlnk_pth), true))) {
                    bSucceeded = true;
                }
                ppf->lpVtbl->Release(ppf);
            }
            psl->lpVtbl->Release(psl);
        }
    }
    Path_Release(hlnk_pth);
    return bSucceeded;
}
// ----------------------------------------------------------------------------


// ============================================================================
//
//  Path_CreateDeskLnk()
//
//  Modified to create a desktop link to Notepad2
//
bool PTHAPI Path_CreateDeskLnk(const HPATHL hDocumentPath, LPCWSTR pszDescription)
{
    if (Path_IsEmpty(hDocumentPath)) {
        return true;
    }

    HPATHL hdsk_pth = Path_Allocate(NULL);
    if (!Path_GetKnownFolder(&FOLDERID_Desktop, hdsk_pth)) {
        Path_Release(hdsk_pth);
        return false;
    }

    bool bSucceeded = false;

    // Try to construct a valid filename...
    HPATHL       hlnkfile_pth = Path_Allocate(NULL);
    LPWSTR const lnkfile_buf = Path_WriteAccessBuf(hlnkfile_pth, PATHLONG_MAX_CCH);

    BOOL fMustCopy = FALSE;
    if (SHGetNewLinkInfoW(Path_Get(hDocumentPath), Path_Get(hdsk_pth), lnkfile_buf, &fMustCopy, SHGNLI_PREFIXNAME)) {

        IShellLink* psl = NULL;
        if (SUCCEEDED(CoCreateInstance(&CLSID_ShellLink, NULL,
                CLSCTX_INPROC_SERVER,
                &IID_IShellLink, (void**)&psl))) {

            HPATHL hdoc_pth = Path_Copy(hDocumentPath);
            Path_QuoteSpaces(hdoc_pth, true);
            HSTRINGW hargs_str = StrgCreate(L"-n ");
            StrgCat(hargs_str, Path_Get(hdoc_pth));

            HPATHL hmod_pth = Path_Allocate(NULL);
            Path_GetModuleFilePath(hmod_pth);

            IPersistFile* ppf = NULL;
            if (SUCCEEDED(psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, (void**)&ppf))) {
                /*MultiByteToWideCharEx(CP_ACP,MB_PRECOMPOSED,tchLnkFileName,-1,wsz,MAX_PATH_EXPLICIT);*/
                psl->lpVtbl->SetPath(psl, Path_Get(hmod_pth));
                psl->lpVtbl->SetArguments(psl, StrgGet(hargs_str));
                psl->lpVtbl->SetDescription(psl, pszDescription);
                if (SUCCEEDED(ppf->lpVtbl->Save(ppf, Path_Get(hlnkfile_pth), true))) {
                    bSucceeded = true;
                }
                ppf->lpVtbl->Release(ppf);
            }
            psl->lpVtbl->Release(psl);

            Path_Release(hmod_pth);
            StrgDestroy(hargs_str);
            Path_Release(hdoc_pth);
        }
    }

    Path_Release(hlnkfile_pth);
    Path_Release(hdsk_pth);
    return bSucceeded;
}
// ----------------------------------------------------------------------------


//=============================================================================
//
//  Path_BrowseDirectory()
//

static int CALLBACK BFFCallBack(HWND hwnd, UINT umsg, LPARAM lParam, LPARAM lpData)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (umsg) {
    case BFFM_INITIALIZED:
        SetDialogIconNP3(hwnd);
        //~InitWindowCommon(hwnd, true);
        SendMessage(hwnd, BFFM_SETSELECTION, true, lpData);
        break;
    case BFFM_VALIDATEFAILED:
        break;
    default:
        break;
    }
    return 0;
}

bool PTHAPI Path_BrowseDirectory(HWND hwndParent, LPCWSTR lpszTitle, HPATHL hpth_in_out, const HPATHL hbase, bool bNewDialogStyle)
{
    if (!hpth_in_out)
        return false;

    HPATHL hbase_dir = Path_Allocate(NULL);
    if (!hbase || Path_IsEmpty(hbase)) {
        Path_GetCurrentDirectory(hbase_dir);
    }
    else {
        Path_Reset(hbase_dir, Path_Get(hbase));
    }

    HPATHL       hres_pth = Path_Allocate(NULL);
    LPWSTR const res_buf = Path_WriteAccessBuf(hpth_in_out, PATHLONG_MAX_CCH);

    BROWSEINFOW bi = { 0 };
    bi.hwndOwner = hwndParent;
    bi.pidlRoot = NULL;
    bi.pszDisplayName = res_buf;
    bi.lpszTitle = lpszTitle;
    bi.ulFlags = BIF_RETURNONLYFSDIRS | (bNewDialogStyle ? (BIF_NEWDIALOGSTYLE | BIF_USENEWUI) : 0);
    bi.lpfn = &BFFCallBack;
    bi.lParam = (LPARAM)Path_Get(hbase_dir);
    bi.iImage = 0;

    LPITEMIDLIST const pidl = SHBrowseForFolderW(&bi);
    Path_Sanitize(hres_pth);

    bool res = false;
    if (pidl) {
        LPWSTR const pth_buf = Path_WriteAccessBuf(hpth_in_out, PATHLONG_MAX_CCH);
        SHGetPathFromIDListW(pidl, pth_buf);
        Path_Sanitize(hpth_in_out);
        Path_FreeExtra(hpth_in_out, MAX_PATH_EXPLICIT);
        CoTaskMemFree(pidl);
        res = true;
    }

    Path_Release(hres_pth);
    Path_Release(hbase_dir);
    return res;
}
// ----------------------------------------------------------------------------


//=============================================================================
//
//  PathCanonicalizeEx()
//
bool PTHAPI Path_CanonicalizeEx(HPATHL hpth_in_out, const HPATHL hdir_rel_base)
{
    HSTRINGW hstr_io = ToHStrgW(hpth_in_out);
    if (!hstr_io)
        return false;
    
    Path_FixBackslashes(hpth_in_out);

    if (StrgFind(hstr_io, PATH_CSIDL_MYDOCUMENTS, 0) == 0) {

        HPATHL hfld_pth = Path_Allocate(NULL);
        if (!Path_GetKnownFolder(&FOLDERID_Documents, hfld_pth)) {
            if (!Path_GetCurrentDirectory(hfld_pth)) {
                Path_GetAppDirectory(hfld_pth);
            }
        }
        StrgReplace(hstr_io, PATH_CSIDL_MYDOCUMENTS, PathGet(hfld_pth));
        Path_Release(hfld_pth);
    }
    ExpandEnvironmentStrgs(hstr_io, true);

    bool res = false;
    if (!Path_IsEmpty(hdir_rel_base) && _Path_IsRelative(hpth_in_out)) {
        HPATHL hmod_pth = Path_Copy(hdir_rel_base);
        res = Path_Append(hmod_pth, Path_Get(hpth_in_out));
        Path_Swap(hpth_in_out, hmod_pth);
        Path_Release(hmod_pth);
    }
    else {
        res = Path_Canonicalize(hpth_in_out);
    }

    return res;
}
// ----------------------------------------------------------------------------


//=============================================================================
//
//  NormalizePathEx()
//
size_t PTHAPI Path_NormalizeEx(HPATHL hpth_in_out, const HPATHL hpth_wrkdir, bool bRealPath, bool bSearchPathIfRelative)
{
    HSTRINGW hstr_io = ToHStrgW(hpth_in_out); // inplace hpth_in_out
    if (!hstr_io)
        return false;

    ExpandEnvironmentStrgs(hstr_io, true);

    if (_Path_IsRelative(hpth_in_out)) {
        
        HPATHL hsrch_pth = Path_Allocate(PathGet(hpth_in_out));
        Path_CanonicalizeEx(hsrch_pth, hpth_wrkdir);

        if (Path_IsExistingFile(hsrch_pth)) {
            Path_Swap(hpth_in_out, hsrch_pth);
        }
        else if (bSearchPathIfRelative) {
            Path_StripPath(hsrch_pth);
            HSTRINGW       hsrch_str = StrgCreate(NULL);
            LPWSTR const buf = StrgWriteAccessBuf(hsrch_str, PATHLONG_MAX_CCH);
            if (SearchPathW(NULL, PathGet(hsrch_pth), NULL, PATHLONG_MAX_CCH, buf, NULL) != 0) {
                //~StrgSanitize(hsrch_str);
                Path_Reset(hpth_in_out, buf);
                //~PrependLongPathPrefix(hpth_in_out, false);
            }
            else { // not found, use file name only (!)
                Path_Sanitize(hsrch_pth);
                Path_Swap(hpth_in_out, hsrch_pth);
            }
            StrgDestroy(hsrch_str);
        }
        //else {
        //    //~~~Path_CanonicalizeEx(hpth_in_out, hpth_wrkdir);
        //}
        Path_Release(hsrch_pth);
    }

    Path_Canonicalize(hpth_in_out);
    Path_GetLongPathNameEx(hpth_in_out);

    if (Path_IsLnkFile(hpth_in_out)) {
        Path_GetLnkPath(hpth_in_out, hpth_in_out);
    }

    if (bRealPath) {
        // get real path name (based on version developed by zufuliu)
        LPCWSTR const path_io = PathGet(hpth_in_out);
        HANDLE const         hFile = CreateFileW(path_io, // file to open
            GENERIC_READ,                         // open for reading
            FILE_SHARE_READ | FILE_SHARE_WRITE,   // share anyway
            NULL,                                 // default security
            OPEN_EXISTING,                        // existing file only
            FILE_ATTRIBUTE_NORMAL,                // normal file
            NULL);                                // no attr. template

        if (IS_VALID_HANDLE(hFile)) {

            HSTRINGW       hstr = StrgCreate(NULL);
            LPWSTR const buf = StrgWriteAccessBuf(hstr, PATHLONG_MAX_CCH);

            if (GetFinalPathNameByHandleW(hFile, buf, PATHLONG_MAX_CCH, FILE_NAME_OPENED) > 0) {
                StrgSanitize(hstr);
                LPWSTR ptr = buf;
                // remove prefix
                if ((wcslen(buf) < MAX_PATH_EXPLICIT) || HasOptInToRemoveMaxPathLimit()) {
                    if ((wcsstr(ptr, PATHUNC_PREFIX1) == ptr) ||
                        (wcsstr(ptr, PATHUNC_PREFIX2) == ptr)) {
                        ptr += (wcslen(PATHUNC_PREFIX1) - 2);
                        *ptr = L'\\';
                    }
                    else if (wcsstr(ptr, PATHLONG_PREFIX) == ptr) {
                        ptr += wcslen(PATHLONG_PREFIX);
                    }
                }
                Path_Reset(hpth_in_out, ptr);
            }
            CloseHandle(hFile);
            StrgDestroy(hstr);
        }
    }
    return Path_GetLength(hpth_in_out);
}
// ----------------------------------------------------------------------------


//=============================================================================
//
//  Path_RelativePathTo()
//
static bool _Path_RelativePathTo(HPATHL hrecv, const HPATHL hfrom, DWORD attr_from, const HPATHL hto, DWORD attr_to)
{
    if (!hrecv || !hfrom || !hto) {
        return false;
    }
    HSTRINGW hrecv_str = ToHStrgW(hrecv);
    StrgEmpty(hrecv_str, false); // init

    bool from_is_dir = (attr_from & FILE_ATTRIBUTE_DIRECTORY);
    bool to_is_dir = (attr_to & FILE_ATTRIBUTE_DIRECTORY);

    // ensure comparable paths (no relatives(..\)
    HPATHL hfrom_cpy = Path_Allocate(PathGet(hfrom));
    Path_Canonicalize(hfrom_cpy);
    HPATHL hto_cpy = Path_Allocate(PathGet(hto));
    Path_Canonicalize(hto_cpy);

    // dirs should en with backslash
    if (from_is_dir) {
        HSTRINGW hstr_from = ToHStrgW(hfrom_cpy);
        StrgTrimRight(hstr_from, L'\\');
        StrgTrimRight(hstr_from, L'/');
        StrgCat(hstr_from, L"\\");
    }
    if (to_is_dir) {
        HSTRINGW hstr_cpy = ToHStrgW(hto_cpy);
        StrgTrimRight(hstr_cpy, L'\\');
        StrgTrimRight(hstr_cpy, L'/');
        StrgCat(hstr_cpy, L"\\");
    }

    // get first diff
    LPCWSTR hfrom_buf = PathGet(hfrom_cpy);
    LPCWSTR hto_buf = PathGet(hto_cpy);
    size_t const   hfrom_len = StrlenW(hfrom_buf);
    size_t const   hto_len = StrlenW(hto_buf);
    size_t const   max_cmp = min_s(hfrom_len, hto_len);
    size_t         i = 0;
    for (i = 0; i < max_cmp; ++i) {
        if (_wcsnicmp(&hfrom_buf[i], &hto_buf[i], 1) != 0) {
            break;
        }
    }

    // check for root prefix
    LPCWSTR r = _Path_SkipRoot(hfrom_cpy);
    bool const     root_f = (r != hfrom_buf);
    LPCWSTR s = _Path_SkipRoot(hto_cpy);
    bool const     root_t = (s != hto_buf);
    size_t const   lenf = (r - hfrom_buf);
    size_t const   lent = (s - hto_buf);

    bool const same_root = root_f && root_t && (lenf == lent) && (_wcsnicmp(hfrom_buf, hto_buf, lenf) == 0);

    if (same_root) {

        // back to prev sync point
        LPCWSTR p = &hfrom_buf[i];
        while (p > r) {
            if ((*p == L'\\') || (*p == L'/') || (*p == L':')) {
                ++p; break;
            }
            --p;
        }
        size_t prefix = (p - hfrom_buf);

        // count dirs of from-path reverse to sync point
        size_t dir_cnt = 0;
        while (*p != L'\0') {
            if ((*p == L'\\') || (*p == L'/')) {
                ++dir_cnt;
            }
            ++p;
        }

        // prepare buffer for prefix "..\" x dir_cnt
        size_t const alloc_add = wcslen(&hto_buf[prefix]) + 1;
        size_t const len = (wcslen(PATHPARENT_PREFIX) * dir_cnt) + alloc_add;
        LPWSTR const hrecv_buf = StrgWriteAccessBuf(hrecv_str, len);
        for (size_t d = 0; d < dir_cnt; ++d) {
            StringCchCatW(hrecv_buf, len, PATHPARENT_PREFIX);
        }
        if ((hto_buf[prefix] == L'\\') || (hto_buf[prefix] == L'/')) {
            ++prefix;
        }
        // copy rest of to-path (excluding first seperator)
        StringCchCatW(hrecv_buf, len, &hto_buf[prefix]);
    }
    else {
        Path_Swap(hrecv, hto_cpy);
    }
    StrgSanitize(hrecv_str);

    Path_Release(hto_cpy);
    Path_Release(hfrom_cpy);

    return (same_root);
}

bool PTHAPI Path_RelativePathTo(HPATHL hrecv, const HPATHL hfrom, DWORD attr_from, const HPATHL hto, DWORD attr_to)
{
    return _Path_RelativePathTo(hrecv, hfrom, attr_from, hto, attr_to);
}
// ----------------------------------------------------------------------------


// ============================================================================
//
//  PathRelativeToApp()
//
void PTHAPI Path_RelativeToApp(HPATHL hpth_in_out, bool bSrcIsFile, bool bUnexpandEnv, bool bUnexpandMyDocs)
{
    if (!hpth_in_out) {
        return;
    }

    HPATHL const happdir_pth = Path_Allocate(NULL);
    Path_GetAppDirectory(happdir_pth);

    HPATHL const husrdoc_pth = Path_Allocate(NULL);
    if (!Path_GetKnownFolder(&FOLDERID_Documents, husrdoc_pth)) {
        if (!Path_GetCurrentDirectory(husrdoc_pth)) {
            Path_GetAppDirectory(husrdoc_pth);
        }
    }

    HPATHL const hprgs_pth = Path_Allocate(NULL);
#ifdef _WIN64
    if (!Path_GetKnownFolder(&FOLDERID_ProgramFiles, hprgs_pth)) {
        Path_GetAppDirectory(hprgs_pth);
    }
#else
    if (!Path_GetKnownFolder(&FOLDERID_ProgramFilesX86, hprgs_pth)) {
        Path_GetAppDirectory(hprgs_pth);
    }
#endif
    //~HPATHL const hwindows_pth = Path_Allocate(NULL);
    //~Path_GetKnownFolder(&FOLDERID_Windows, hwindows_pth); // deprecated

    bool const bPathIsRelative = _Path_IsRelative(hpth_in_out);
    bool const bAppPathIsUsrDoc = Path_IsPrefix(husrdoc_pth, happdir_pth);
    bool const bPathIsPrefixUsrDoc = Path_IsPrefix(husrdoc_pth, hpth_in_out);
    DWORD      dwAttrTo = (bSrcIsFile) ? FILE_ATTRIBUTE_NORMAL : FILE_ATTRIBUTE_DIRECTORY;
    
    HPATHL htmp_pth = Path_Allocate(NULL);
    if (bUnexpandMyDocs && !bPathIsRelative && !bAppPathIsUsrDoc && bPathIsPrefixUsrDoc
        && _Path_RelativePathTo(htmp_pth, husrdoc_pth, FILE_ATTRIBUTE_DIRECTORY, hpth_in_out, dwAttrTo)) {
        Path_Reset(hpth_in_out, PATH_CSIDL_MYDOCUMENTS);
        Path_Append(hpth_in_out, Path_Get(htmp_pth));
    }
    else if (!bPathIsRelative && !Path_CommonPrefix(happdir_pth, hprgs_pth, NULL)) {
        if (_Path_RelativePathTo(htmp_pth, happdir_pth, FILE_ATTRIBUTE_DIRECTORY, hpth_in_out, dwAttrTo)) {
            Path_Swap(hpth_in_out, htmp_pth);
        }
    }

    Path_Release(htmp_pth);
    Path_Release(hprgs_pth);
    Path_Release(husrdoc_pth);
    Path_Release(happdir_pth);

    if (bUnexpandEnv) {
        Path_UnExpandEnvStrings(hpth_in_out);
    }

}
// ----------------------------------------------------------------------------


// ============================================================================
//
//  GetKnownFolderPath()
//
bool PTHAPI Path_GetKnownFolder(REFKNOWNFOLDERID rfid, HPATHL hpth_out)
{
    HSTRINGW    hstr_out = ToHStrgW(hpth_out);
    PWSTR       pszPath = NULL;
    const DWORD dwFlags = KF_FLAG_NO_ALIAS;
    //(KF_FLAG_DEFAULT_PATH | KF_FLAG_NOT_PARENT_RELATIVE | KF_FLAG_NO_ALIAS);
    HRESULT const hr = SHGetKnownFolderPath(rfid, dwFlags, NULL, &pszPath);
    if (SUCCEEDED(hr) && pszPath) {
        StrgReset(hstr_out, pszPath);
        CoTaskMemFree(pszPath);
        return true;
    }
    return false;
}
// ----------------------------------------------------------------------------


//=============================================================================
//
//  (Path_)ExpandEnvironmentStrings()
//
void PTHAPI ExpandEnvironmentStrgs(HSTRINGW hstr_in_out, bool bStripQ)
{
    if (!hstr_in_out) {
        return;
    }
    if (bStripQ) {
        StrgTrim(hstr_in_out, L'"');
        StrgTrim(hstr_in_out, L'\'');
    }
    size_t const min_len = ExpandEnvironmentStrings(StrgGet(hstr_in_out), NULL, 0);
    LPWSTR       buf_io = StrgWriteAccessBuf(hstr_in_out, min_len);
    DWORD const  cch_io = (DWORD)StrgGetAllocLength(hstr_in_out);

    HSTRINGW const hstr_cpy = StrgCopy(hstr_in_out); // (!) no inplace substitution possible

    if (ExpandEnvironmentStringsW(StrgGet(hstr_cpy), buf_io, cch_io)) {
        StrgSanitize(hstr_in_out);
    }
    StrgDestroy(hstr_cpy);
}
// ----------------------------------------------------------------------------


// ============================================================================
// Old Stuff in INTERMEDIATE DEV state
// ============================================================================


//=============================================================================
//
//  PathIsExistingFile()
//
bool PTHAPI Path_IsExistingFile(const HPATHL hpth)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hstr)
        return false;

    HPATHL hxpth = Path_Allocate(StrgGet(hstr));
    PrependLongPathPrefix(hxpth, false);
    DWORD const dwFileAttrib = GetFileAttributesW(PathGet(hxpth));
    Path_Release(hxpth);

    ///bool const  bAccessOK = (dwFileAttrib != INVALID_FILE_ATTRIBUTES);
    ///if (!bAccessOK) {
    ///    DWORD const dwError = GetLastError();
    ///    switch (dwError) {
    ///    case ERROR_FILE_NOT_FOUND:
    ///        break;
    ///    case ERROR_PATH_NOT_FOUND:
    ///        break;
    ///    case ERROR_ACCESS_DENIED:
    ///        break;
    ///    default:
    ///        break;
    ///    }
    ///}

    return IsExistingFile(dwFileAttrib);
}


bool PTHAPI PathIsExistingFile(LPCWSTR pszPath)
{
    //return (PathFileExists(pszPath) && !PathIsDirectory(pszPath));
    HPATHL const hpth = Path_Allocate(pszPath);
    bool const   res = Path_IsExistingFile(hpth);
    Path_Release(hpth);
    return res;
}


//=============================================================================
//
//  PathAbsoluteFromApp()
//
void PTHAPI Path_AbsoluteFromApp(HPATHL hpth_in_out, bool bExpandEnv)
{
    HSTRINGW hstr_in_out = ToHStrgW(hpth_in_out); // inplace hpth_in_out
    if (!hstr_in_out) {
        return;
    }

    HPATHL   htmp_pth = Path_Allocate(PathGet(hpth_in_out));
    HSTRINGW htmp_str = ToHStrgW(htmp_pth); // inplace hpth_in_out

    if (StrgFind(hstr_in_out, PATH_CSIDL_MYDOCUMENTS, 0) == 0) {

        HPATHL hfld_pth = Path_Allocate(NULL);
        if (!Path_GetKnownFolder(&FOLDERID_Documents, hfld_pth)) {
            if (!Path_GetCurrentDirectory(hfld_pth)) {
                Path_GetAppDirectory(hfld_pth);
            }
        }
        StrgReplace(htmp_str, PATH_CSIDL_MYDOCUMENTS, PathGet(hfld_pth));
        Path_Release(hfld_pth);
    }

    if (bExpandEnv) {
        Path_ExpandEnvStrings(htmp_pth);
    }

    if (_Path_IsRelative(htmp_pth)) {
        Path_GetAppDirectory(hpth_in_out);
        Path_Append(hpth_in_out, Path_Get(htmp_pth)); // does Path_Canonicalize()
    }
    else {
        Path_Swap(hpth_in_out, htmp_pth);
        Path_Canonicalize(hpth_in_out);
    }
    Path_Release(htmp_pth);

    // TODO:
    //if (PathGetDriveNumber(wchResult) != -1) {
    //    CharUpperBuff(wchResult, 1);
    //}
}

void PTHAPI PathAbsoluteFromApp(LPWSTR lpszPath, const size_t cchPath, bool bExpandEnv)
{
    HPATHL hpth_in_out = Path_Allocate(lpszPath);
    Path_AbsoluteFromApp(hpth_in_out, bExpandEnv);
    StringCchCopyW(lpszPath, ((cchPath == 0) ? PATHLONG_MAX_CCH : cchPath), PathGet(hpth_in_out));
    Path_Release(hpth_in_out);
}

// ============================================================================
// ============================================================================

