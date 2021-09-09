
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

#define NOMINMAX
#define VC_EXTRALEAN 1
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <assert.h>
#include <processenv.h>
#include <stdbool.h>
#include <strsafe.h>
#include <fileapi.h>

#define PATHCCH_NO_DEPRECATE 1  // <- get rid of this!
#include <pathcch.h>

// get rid of this:
#include <shlobj.h>
#include <shellapi.h>
#include <shlwapi.h>


#define NP3_PATH_LIB_IMPLEMENTATION 1
#include "PathLib.h"


#pragma comment(linker, "/defaultlib:Pathcch")


/**************************************************/
/*                                                */
/*              PRIVATE API                       */
/*                                                */
/**************************************************/

#define COUNTOF(ar) ARRAYSIZE(ar)
#define CONSTSTRGLEN(s) (COUNTOF(s) - 1)

const wchar_t* const PATHLONG_PREFIX = L"\\\\?\\";
#define PATHLONG_PREFIX_LEN (COUNTOF(PATHLONG_PREFIX) - 1)

const wchar_t* const PATHUNC_PREFIX = L"UNC\\";
#define PATHUNC_PREFIX_LEN (COUNTOF(PATHUNC_PREFIX) - 1)

const wchar_t* const PATHPARENT_PREFIX = L"..\\";
#define PATHPARENT_PREFIX_LEN (COUNTOF(PATHPARENT_PREFIX) - 1)

const wchar_t* const PATHDSPL_INFIX = L" ... ";
#define PATHDSPL_INFIX_LEN (COUNTOF(PATHDSPL_INFIX) - 1)

const wchar_t* const PATH_CSIDL_MYDOCUMENTS = L"%CSIDL:MYDOCUMENTS%";
#define PATH_CSIDL_MYDOCUMENTS_LEN (COUNTOF(PATH_CSIDL_MYDOCUMENTS) - 1)


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


__forceinline wchar_t _wcsgetprev(const wchar_t* str, const wchar_t* c)
{
    if (!str)
        return L'\0';
    return (c && (c > str)) ? *(c - 1) : L'\0';
}
__forceinline wchar_t _wcsgetnext(const wchar_t* c)
{
    return (c && *c) ? *(c + 1) : L'\0';
}

static void _PathFixBackslashes(const wchar_t* pstrg)
{
    wchar_t* c = wcschr(pstrg, L'/');
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
    HSTRINGW hstr_io = ToHStrgW(hpth_in_out); // inplace
    if (!hstr_io)
        return;

    if (bForce || !HasOptInToRemoveMaxPathLimit()) {
        if (bForce || (StrgGetLength(hstr_io) >= MAX_PATH)) {
            if (StrgFind(hstr_io, PATHLONG_PREFIX, 0) != 0) {
                StrgInsert(hstr_io, 0, PATHLONG_PREFIX);
            }
        }
    }
}

// ----------------------------------------------------------------------------

static void _UnExpandEnvStrgs(HSTRINGW hstr_in_out)
{
    if (!hstr_in_out) {
        return;
    }
    const wchar_t* env_var_list[] = {
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
            wchar_t* buf = StrgWriteAccessBuf(htmp_str, len);
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
            }
        }
    }
    StrgDestroy(htmp_str);
}

// ----------------------------------------------------------------------------

// Determines whether a path string refers to the root of a volume.
//
//  Path                      PathCchIsRoot() 
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
__forceinline static bool _IsRootPath(const HPATHL hpth)
{
    return PathCchIsRoot(PathGet(hpth));
}

// ----------------------------------------------------------------------------



/**************************************************/
/*                                                */
/*              PUBLIC API                        */
/*                                                */
/**************************************************/

HPATHL PTHAPI Path_Allocate(const wchar_t* path)
{
    return (HPATHL)StrgCreate(path);
}
// ----------------------------------------------------------------------------


void PTHAPI Path_Release(HPATHL hpth)
{
    StrgDestroy(ToHStrgW(hpth));
}
// ----------------------------------------------------------------------------


void PTHAPI Path_Empty(HPATHL hpth, bool truncate)
{
    StrgEmpty(ToHStrgW(hpth), truncate);
}
// ----------------------------------------------------------------------------


int PTHAPI Path_Reset(HPATHL hpth_in_out, const wchar_t* path)
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


bool PTHAPI Path_Append(HPATHL hpth, HPATHL hmore)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hstr)
        return false;

    size_t const hstr_len = StrgGetLength(hstr);
    size_t const hmore_len = Path_GetLength(hmore);
    if (!hmore_len) {
        return true;
    }
    LPCWSTR wmore = PathGet(hmore);

    PrependLongPathPrefix(hpth, false);

    LPWSTR       wbuf = StrgWriteAccessBuf(hstr, hstr_len + hmore_len + PATHLONG_PREFIX_LEN + 8);
    size_t const cch = StrgGetAllocLength(hstr);

    DWORD const dwFlags = PATHCCH_ALLOW_LONG_PATHS;
    //  Windows 10, version 1703:
    //  PATHCCH_FORCE_ENABLE_LONG_NAME_PROCESS | PATHCCH_ENSURE_IS_EXTENDED_LENGTH_PATH
    bool const  res = SUCCEEDED(PathCchAppendEx(wbuf, cch, wmore, dwFlags));
    StrgSanitize(hstr);

    return res;
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


// With untrusted input, this function by itself, cannot be used to convert
// paths into a form that can be compared with other paths for sub-path or identity.
// Callers that need that ability should convert forward to back slashes before
// using this function.

bool PTHAPI Path_Canonicalize(HPATHL hpth_in_out)
{
    if (!hpth_in_out) {
        return false;
    }

    HPATHL hpth_cpy = Path_Allocate(PathGet(hpth_in_out));
    HSTRINGW hstr_cpy = ToHStrgW(hpth_cpy);

    // PathCchCanonicalizeEx() does not convert forward slashes (/) into back slashes (\).
    //~StrgReplaceCh(hstr_cpy, L'/', L'\\');
    _PathFixBackslashes(StrgGet(hstr_cpy));

    // canonicalize prefix
    PrependLongPathPrefix(hpth_cpy, false);

    // internal buffer access
    LPWSTR       wbuf_out = StrgWriteAccessBuf(hstr_cpy, 0);
    size_t const cch_out = StrgGetAllocLength(hstr_cpy);

    DWORD const dwFlags = PATHCCH_ALLOW_LONG_PATHS;
    //  Windows 10, version 1703:
    //  PATHCCH_FORCE_ENABLE_LONG_NAME_PROCESS | PATHCCH_ENSURE_IS_EXTENDED_LENGTH_PATH
    //  PATHCCH_ENSURE_TRAILING_SLASH
    bool const res = SUCCEEDED(PathCchCanonicalizeEx(wbuf_out, cch_out, PathGet(hpth_in_out), dwFlags));
    StrgSanitize(hstr_cpy);

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


bool PTHAPI Path_IsValidUNC(const HPATHL hpth, HSTRINGW server_name_out)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hstr)
        return false;

    //PrependLongPathPrefix(hpth, false);

    wchar_t const buffer[1024] = { L'\0' };
    const wchar_t* server_name = (server_name_out ? buffer : NULL);

    bool const res = PathIsUNCEx(PathGet(hpth), &server_name);

    if (server_name) {
        StrgReset(server_name_out, server_name);
    }
    return res;
}
// ----------------------------------------------------------------------------


bool PTHAPI Path_IsRoot(const HPATHL hpth)
{
    return _IsRootPath(hpth);
}
// ----------------------------------------------------------------------------


int PTHAPI Path_StrgComparePath(const HPATHL hpth1, const HPATHL hpth2)
{
    HSTRINGW hstr1 = ToHStrgW(hpth1);
    if (!hstr1 || !StrgGet(hstr1))
        return -1;
    HSTRINGW hstr2 = ToHStrgW(hpth2);
    if (!hstr2 || !StrgGet(hstr2))
        return 1;

    size_t const max_len = min_s(StrgGetLength(hstr1), StrgGetLength(hstr2)) + 1;

    //~return wcsncmp(StrgGet(hstr1), StrgGet(hstr2), max_len);
    return _wcsnicmp(StrgGet(hstr1), StrgGet(hstr2), max_len);
}
// ----------------------------------------------------------------------------


bool PTHAPI Path_RemoveBackslash(HPATHL hpth_in_out)
{
    HSTRINGW hstr = ToHStrgW(hpth_in_out);
    if (!hstr)
        return false;

    LPWSTR wbuf = StrgWriteAccessBuf(hstr, 0); // no need to ReAlloc
    size_t cch = StrgGetAllocLength(hstr);

    bool const res = SUCCEEDED(PathCchRemoveBackslashEx(wbuf, cch, NULL, NULL));
    StrgSanitize(hstr);

    return res;
}
// ----------------------------------------------------------------------------


bool PTHAPI Path_RemoveFileSpec(HPATHL hpth_in_out)
{
    HSTRINGW hstr = ToHStrgW(hpth_in_out);
    if (!hstr)
        return false;
    
    PrependLongPathPrefix(hpth_in_out, false);

    LPWSTR wbuf = StrgWriteAccessBuf(hstr, 0); // no need to ReAlloc
    size_t cch = StrgGetAllocLength(hstr);

    bool const res = SUCCEEDED(PathCchRemoveFileSpec(wbuf, cch));
    StrgSanitize(hstr);

    return res;
}
// ----------------------------------------------------------------------------


bool PTHAPI Path_StripPath(HPATHL hpth)  // get filename only
{
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hstr)
        return false;

    size_t idx = 0;
    Path_FindFileName(hpth, &idx);
    if (idx != 0) {
        StrgDelete(hstr, 0, idx);
        return true;
    }
    return false;
}
// ----------------------------------------------------------------------------


bool PTHAPI Path_RenameExtension(HPATHL hpth, const wchar_t* ext)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hstr)
        return false;
    
    PrependLongPathPrefix(hpth, false);

    size_t const hstr_len = StrgGetLength(hstr);
    size_t const ext_len = (ext ? wcslen(ext) : 0);

    LPWSTR wbuf = StrgWriteAccessBuf(hstr, hstr_len + ext_len);
    size_t cch = StrgGetAllocLength(hstr);

    bool const bOK = SUCCEEDED(PathCchRenameExtension(wbuf, cch, (ext ? ext : L"")));
    StrgSanitize(hstr);

    return bOK;
}
// ----------------------------------------------------------------------------


void PTHAPI Path_ExpandEnvStrings(HPATHL hpth)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hstr)
        return;
    
    ExpandEnvironmentStrgs(hstr);
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
    HSTRINGW hmod_str = ToHStrgW(hpth_out);
    wchar_t* const buf = StrgWriteAccessBuf(hmod_str, PATHLONG_MAX_CCH);
    GetModuleFileNameW(NULL, buf, PATHLONG_MAX_CCH);
    StrgSanitize(hmod_str);
    StrgFreeExtra(hmod_str);
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
    wchar_t* const out_buf = cpy_out ? StrgWriteAccessBuf(hout_str, ovl_cnt + 1) : NULL;

    const wchar_t* p1 = StrgGet(hpth1_str);
    const wchar_t* p2 = StrgGet(hpth2_str);

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
const wchar_t* PTHAPI Path_FindExtension(const HPATHL hpth)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hstr)
        return NULL;

    const wchar_t* pext = NULL;
    //bool const     res = SUCCEEDED(...)
    PathCchFindExtension(StrgGet(hstr), StrgGetAllocLength(hstr), &pext);
    //~return (res ? pext : NULL);
    return pext;
}
// ----------------------------------------------------------------------------


const wchar_t* PTHAPI Path_FindFileName(const HPATHL hpth, size_t* idx_out)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hstr)
        return NULL;

    const wchar_t* const buf = StrgGet(hstr);
    const wchar_t*       pfs = buf;
    const wchar_t*       pbs = buf;

    size_t idxb = StrgReverseFind(hstr, L'\\');
    if (idxb != STRINGW_INVALID_IDX) {
        pbs = &buf[idxb + 1];
    }
    else {
        idxb = 0;
    }

    size_t idxf = StrgReverseFind(hstr, L'/');
    if (idxf != STRINGW_INVALID_IDX) {
        pfs = &buf[idxf + 1];
    }
    else {
        idxf = 0;
    }

    if (idx_out) {
        *idx_out = (idxb >= idxf) ? idxb : idxf; 
    }
    return (pbs >= pfs) ? pbs : pfs;
}
// ----------------------------------------------------------------------------


bool PTHAPI Path_QuoteSpaces(HPATHL hpth_in_out)
{
    HSTRINGW hstr_io = ToHStrgW(hpth_in_out);
    if (!hstr_io)
        return false;

    bool const found = (StrgFindCh(hstr_io, L' ', 0) != STRINGW_INVALID_IDX);
    if (found) {
        if (StrgGetAt(hstr_io, 0) != L'"') {
            StrgInsert(hstr_io, 0, L"\"");
            StrgCat(hstr_io, L"\"");
        }
    }
    return found;
}
// ----------------------------------------------------------------------------


int PTHAPI Path_GetDriveNumber(const HPATHL hpth)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hstr)
        return -1;

    HSTRINGW hcpy = StrgCopy(hstr);
    wchar_t* const buf = StrgWriteAccessBuf(hcpy, 0);
    PathCchStripToRoot(buf, StrgGetAllocLength(hcpy));
    PathCchStripPrefix(buf, StrgGetAllocLength(hcpy));

    int res = max_i(-1, ((int)_wcsupr_s(buf, 1) - (int)L'A'));
    res = (res > 25) ? -1 : res;

    StrgDestroy(hcpy);
    return res;
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




// ============================================================================
// Old Stuff in INTERMEDIATE DEV state
// ============================================================================


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
//  ExpandEnvironmentStringsEx()
//
void PTHAPI ExpandEnvironmentStrgs(HSTRINGW hstr_in_out)
{
    if (!hstr_in_out) {
        return;
    }

    HSTRINGW const hstr_cpy = StrgCopy(hstr_in_out); // no inplace substitution possible

    size_t const min_len = ExpandEnvironmentStringsW(StrgGet(hstr_in_out), NULL, 0);
    LPWSTR       buf_io = StrgWriteAccessBuf(hstr_in_out, min_len);
    DWORD const  cch_io = (DWORD)StrgGetAllocLength(hstr_in_out);

    if (ExpandEnvironmentStringsW(StrgGet(hstr_cpy), buf_io, cch_io)) {
        StrgSanitize(hstr_in_out);
    }
    StrgDestroy(hstr_cpy);
}

void PTHAPI ExpandEnvironmentStringsEx(LPWSTR lpSrc, size_t cchSrc)
{
    HSTRINGW hstr = StrgCreate(lpSrc);
    ExpandEnvironmentStrgs(hstr);
    const wchar_t* buf = StrgGet(hstr);
    if (buf) {
        StringCchCopyW(lpSrc, cchSrc, buf);
    }
    StrgDestroy(hstr);
}


//=============================================================================
//
//  _Path_IsRelative()
//  TODO: make LongPath version instead of slicing MAX_PATH
//
static bool _Path_IsRelative(const HPATHL hpth)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hstr)
        return true; // empty is relative

    if (PathCchIsRoot(StrgGet(hstr))) {
        return false; // cant be relative
    }

    HSTRINGW cpy = StrgCopy(hstr);

    bool res = false;
    if (StrgGetLength(cpy) >= MAX_PATH) {
        // hack for MAX_PATH limit
        wchar_t* const buf = (wchar_t* const)StrgGet(cpy);
        buf[MAX_PATH] = L'\0';
        size_t const idx = StrgReverseFind(cpy, L'\\');
        if (idx != STRINGW_INVALID_IDX) {
            buf[idx] = L'\0';
            res = PathIsRelativeW(StrgGet(cpy));
        }
    }
    else {
        res = PathIsRelativeW(StrgGet(cpy));
    }

    StrgDestroy(cpy);
    return res;
}


//=============================================================================
//
//  _Path_RelativePathTo()
//  TODO: make LongPath version instead of slicing MAX_PATH
//
static bool _Path_RelativePathTo(HPATHL hrecv, const HPATHL hfrom, DWORD attr_from, const HPATHL hto, DWORD attr_to)
{
    if (!hrecv || !hfrom || !hto) {
        return false;
    }
    HSTRINGW hrecv_str = ToHStrgW(hrecv);
    StrgEmpty(hrecv_str, false); // init

    //  TODO: consider attr_from/attr_to  ???
    //bool from_is_dir = (attr_from & FILE_ATTRIBUTE_DIRECTORY);
    //bool to_is_dir = (attr_to & FILE_ATTRIBUTE_DIRECTORY);
    UNREFERENCED_PARAMETER(attr_from);
    UNREFERENCED_PARAMETER(attr_to);

#if 0
    // DEBUG
    {
        Path_Reset(hfrom, L"\\FolderA\\FolderB\\FolderC\\abab");
        //from_is_dir = true;
        Path_Reset(hto, L"\\FolderA\\FolderD\\FolderE\\blah");
        //to_is_dir = false;
    }
#endif

    HPATHL hfrom_cpy = Path_Allocate(PathGet(hfrom));
    HPATHL hto_cpy = Path_Allocate(PathGet(hto));

    // ensure comparable paths (no relatives(..\)
    //~Path_CanonicalizeEx(hfrom_cpy);
    //~Path_CanonicalizeEx(hto_cpy);
    Path_Canonicalize(hfrom_cpy);
    Path_Canonicalize(hto_cpy);

    // get first diff
    const wchar_t* hfrom_buf = PathGet(hfrom_cpy);
    const wchar_t* hto_buf = PathGet(hto_cpy);
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
    const wchar_t* r = hfrom_buf;
    bool const     root_f = SUCCEEDED(PathCchSkipRoot(hfrom_buf, &r));
    const wchar_t* s = hto_buf;
    bool const     root_t = SUCCEEDED(PathCchSkipRoot(hto_buf, &s));
    size_t const   lenf = (r - hfrom_buf);
    size_t const   lent = (s - hto_buf);

    bool const     same_root = root_f && root_t && (lenf == lent) && (_wcsnicmp(hfrom_buf, hto_buf, lenf) == 0);

    if (same_root) {

        // back to prev sync point
        const wchar_t* p = &hfrom_buf[i];
        while (p > r) {
            if ((*p == L'\\') || (*p == L':')) {
                break;
            }
            --p;
        }
        size_t prefix = (p - hfrom_buf);

        // count dirs of from-path reverse to sync point
        size_t dir_cnt = 0;
        while (*p != L'\0') {
            if ((*p == L'\\')) {
                // ignore trailing backslash
                if (*(p + 1)) {
                    ++dir_cnt;
                }
            }
            ++p;
        }

        // prepare buffer for prefix "..\" x dir_cnt

        size_t const   alloc_add = wcslen(&hto_buf[prefix]) + 1;
        size_t const   len = (PATHPARENT_PREFIX_LEN * dir_cnt) + alloc_add;
        wchar_t* const out_buf = StrgWriteAccessBuf(hrecv_str, len);
        for (size_t d = 0; d < dir_cnt; ++d) {
            StringCchCatW(out_buf, len, PATHPARENT_PREFIX);
        }
        //~PathCchRemoveBackslashEx(out_buf, len, NULL, NULL);
        if (hto_buf[prefix] == L'\\') {
            ++prefix;
        }
        // copy rest of to-path (excluding first seperator)
        StringCchCatW(out_buf, len, &hto_buf[prefix]);
    }
    else {
        Path_Swap(hrecv, hto_cpy);
    }

    Path_Release(hto_cpy);
    Path_Release(hfrom_cpy);

    return (same_root);
}


//=============================================================================
//
//  GetLongPathNameEx()
//
size_t PTHAPI Path_GetLongPathNameEx(HPATHL hpth_in_out)
{
    HSTRINGW hstr_io = ToHStrgW(hpth_in_out);
    if (!hstr_io)
        return 0UL;

    PrependLongPathPrefix(hpth_in_out, false); // TODO: check or true ?

    DWORD const    len = (size_t)GetLongPathNameW(StrgGet(hstr_io), NULL, 0);
    wchar_t* const buf = StrgWriteAccessBuf(hstr_io, (size_t)len);

    size_t const res = (size_t)GetLongPathNameW(buf, buf, len);
    StrgSanitize(hstr_io);

    if (res > 2ULL) {
        wchar_t* const pos = wcschr(buf, L':');
        if (pos && (pos > buf)) {
            CharUpperBuffW(pos - 1, 1);
        }
    }
    return res;
}

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


//=============================================================================
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

bool PTHAPI PathGetKnownFolder(REFKNOWNFOLDERID rfid, LPWSTR lpOutPath, size_t cchOut)
{
    HPATHL     hpth = Path_Allocate(NULL);
    bool const res = Path_GetKnownFolder(rfid, hpth);
    if (res) {
        StringCchCopyW(lpOutPath, cchOut, PathGet(hpth));
    }
    Path_Release(hpth);
    return res;
}


//=============================================================================
//
//  PathCanonicalizeEx()
//
bool PTHAPI Path_CanonicalizeEx(HPATHL hpth_in_out)
{
    HSTRINGW hstr_io = ToHStrgW(hpth_in_out);
    if (!hstr_io)
        return false;

   ExpandEnvironmentStrgs(hstr_io); // inplace hpth_in_out

   bool res = false;
   if (_Path_IsRelative(hpth_in_out))
   {
       HPATHL hmod_pth = Path_Allocate(NULL);
       Path_GetModuleFilePath(hmod_pth);
       Path_RemoveFileSpec(hmod_pth);
       Path_Append(hmod_pth, hpth_in_out);
       res = Path_Canonicalize(hmod_pth);
       Path_Swap(hpth_in_out, hmod_pth);
       Path_Release(hmod_pth);
   }
   else {
       res = Path_Canonicalize(hpth_in_out);
   }

   Path_RemoveBackslash(hpth_in_out);

   return res;
}

bool PTHAPI PathCanonicalizeEx(LPWSTR lpszPath, size_t cchPath)
{
    HPATHL   hpth = Path_Allocate(lpszPath);
    bool const res = Path_CanonicalizeEx(hpth);
    if (res) {
        StringCchCopyW(lpszPath, cchPath, PathGet(hpth));
    }
    Path_Release(hpth);
    return res;
}


//=============================================================================
//
//  NormalizePathEx()
//
size_t PTHAPI Path_NormalizeEx(HPATHL hpth_in_out, const HPATHL hpth_wrkdir, bool bRealPath, bool bSearchPathIfRelative)
{
    HSTRINGW hstr_io = ToHStrgW(hpth_in_out); // inplace hpth_in_out
    if (!hstr_io)
        return false;

    ExpandEnvironmentStrgs(hstr_io);

    // strg beg/end: clear spaces and quote s
    StrgTrim(hstr_io, L'"');
    StrgTrim(hstr_io, L'\'');

    if (_Path_IsRelative(hpth_in_out)) {
        HPATHL hsrch_pth = Path_Allocate(PathGet(hpth_wrkdir));
        Path_Append(hsrch_pth, hpth_in_out);
        if (bSearchPathIfRelative) {
            if (!Path_IsExistingFile(hsrch_pth)) {
                Path_StripPath(hsrch_pth);
                HSTRINGW       hsrch_str = StrgCreate(NULL);
                wchar_t* const buf = StrgWriteAccessBuf(hsrch_str, PATHLONG_MAX_CCH);
                if (SearchPathW(NULL, PathGet(hsrch_pth), NULL, PATHLONG_MAX_CCH, buf, NULL) != 0) {
                    Path_Reset(hpth_in_out, buf);
                    //PrependLongPathPrefix(hpth_in_out, false);
                }
                else {
                    Path_Swap(hpth_in_out, hsrch_pth);
                }
                StrgDestroy(hsrch_str);
            }
        }
        Path_Release(hsrch_pth);
    }

    Path_CanonicalizeEx(hpth_in_out);
    Path_GetLongPathNameEx(hpth_in_out);

    // TODO: ...
    //if (Path_IsLnkFile(hpth_in_out)) {
    //    Path_GetLnkPath(hpth_in_out);
    //}

    if (bRealPath) {
        // get real path name (by zufuliu)
        const wchar_t* const path_io = PathGet(hpth_in_out);
        HANDLE const   hFile = CreateFileW(path_io, // file to open
            GENERIC_READ,                                       // open for reading
            FILE_SHARE_READ | FILE_SHARE_WRITE,                 // share anyway
            NULL,                                               // default security
            OPEN_EXISTING,                                      // existing file only
            FILE_ATTRIBUTE_NORMAL,                              // normal file
            NULL);                                              // no attr. template

        if (IS_VALID_HANDLE(hFile)) {

            HSTRINGW       hstr = StrgCreate(NULL);
            wchar_t* const buf = StrgWriteAccessBuf(hstr, PATHLONG_MAX_CCH);

            if (GetFinalPathNameByHandleW(hFile, buf, PATHLONG_MAX_CCH, FILE_NAME_OPENED) > 0) {
                StrgSanitize(hstr);
                // remove prefix
                if (wcslen(buf) < MAX_PATH) {
                    if (wcsncmp(buf, PATHLONG_PREFIX, PATHLONG_PREFIX_LEN) == 0) {
                        WCHAR* ptr = buf + 4;
                        if (wcsncmp(ptr, PATHUNC_PREFIX, PATHUNC_PREFIX_LEN) == 0) {
                            ptr += 2;
                            *ptr = L'\\';
                        }
                        Path_Reset(hpth_in_out, ptr);
                    }
                }
            }
            CloseHandle(hFile);
            StrgDestroy(hstr);
        }
    }
    return Path_GetLength(hpth_in_out);
}

size_t PTHAPI NormalizePathEx(LPWSTR lpszPath, size_t cchPath, LPCWSTR lpszWorkDir, bool bRealPath, bool bSearchPathIfRelative)
{
    HPATHL hpth = Path_Allocate(lpszPath);
    HPATHL const hwrk = Path_Allocate(lpszWorkDir);

    size_t len = Path_NormalizeEx(hpth, hwrk, bRealPath, bSearchPathIfRelative);
    if (len) {
        StringCchCopyW(lpszPath, cchPath, PathGet(hpth));
    }
    Path_Release(hpth);
    Path_Release(hwrk);
    return len;
}


//=============================================================================
//
//  PathGetModuleDirectory()
//
void PTHAPI Path_GetAppDirectory(HPATHL hpth_out)
{
    if (!hpth_out) {
        return;
    }
    Path_GetModuleFilePath(hpth_out);
    Path_RemoveFileSpec(hpth_out);
    Path_Canonicalize(hpth_out);
}

void PTHAPI PathGetAppDirectory(LPWSTR lpszDest, size_t cchDest)
{
    HPATHL hpth = Path_Allocate(NULL);
    Path_GetAppDirectory(hpth);
    const wchar_t* buf = PathGet(hpth);
    if (buf) {
        StringCchCopyW(lpszDest, cchDest, buf);
    }
    Path_Release(hpth);
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
        Path_GetKnownFolder(&FOLDERID_Documents, hfld_pth);
        StrgReplace(htmp_str, PATH_CSIDL_MYDOCUMENTS, PathGet(hfld_pth));
        Path_Release(hfld_pth);
    }

    if (bExpandEnv) {
        Path_ExpandEnvStrings(htmp_pth);
    }

    if (_Path_IsRelative(htmp_pth)) {
        Path_GetAppDirectory(hpth_in_out);
        Path_Append(hpth_in_out, htmp_pth);
    }
    else {
        Path_Swap(hpth_in_out, htmp_pth);
    }
    Path_Release(htmp_pth);

    Path_CanonicalizeEx(hpth_in_out);

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


//=============================================================================
//
//  PathRelativeToApp()
//
void PTHAPI Path_RelativeToApp(HPATHL hpth_in_out, bool bSrcIsFile, bool bUnexpandEnv, bool bUnexpandMyDocs)
{
    if (!hpth_in_out) {
        return;
    }

    HPATHL happdir_pth = Path_Allocate(NULL);
    Path_GetAppDirectory(happdir_pth);

    HPATHL husrdoc_pth = Path_Allocate(NULL);
    Path_GetKnownFolder(&FOLDERID_Documents, husrdoc_pth);

    HPATHL hprgs_pth = Path_Allocate(NULL);
#ifdef _WIN64
    Path_GetKnownFolder(&FOLDERID_ProgramFiles, hprgs_pth);
#else
    Path_GetKnownFolder(&FOLDERID_ProgramFilesX86, hprgs_pth);
#endif
    //~HPATHL hwindows_pth = Path_Allocate(NULL);
    //~Path_GetKnownFolder(&FOLDERID_Windows, hwindows_pth); // deprecated

    HPATHL htmp_pth = Path_Allocate(NULL);

    DWORD dwAttrTo = (bSrcIsFile) ? FILE_ATTRIBUTE_NORMAL : FILE_ATTRIBUTE_DIRECTORY;
    if (bUnexpandMyDocs &&
        !_Path_IsRelative(hpth_in_out) &&
        !Path_IsPrefix(husrdoc_pth, happdir_pth) &&
        Path_IsPrefix(husrdoc_pth, hpth_in_out) &&
        _Path_RelativePathTo(htmp_pth, husrdoc_pth, FILE_ATTRIBUTE_DIRECTORY, hpth_in_out, dwAttrTo))
    {
        Path_Reset(hpth_in_out, PATH_CSIDL_MYDOCUMENTS);
        Path_Append(hpth_in_out, htmp_pth);
    }
    else if (!_Path_IsRelative(hpth_in_out) && !Path_CommonPrefix(happdir_pth, hprgs_pth, NULL))
    {
        if (_Path_RelativePathTo(htmp_pth, happdir_pth, FILE_ATTRIBUTE_DIRECTORY, hpth_in_out, dwAttrTo)) {
            Path_Swap(hpth_in_out, htmp_pth);
        }
    }

    if (bUnexpandEnv) {
        Path_UnExpandEnvStrings(hpth_in_out);
    }

    Path_Release(htmp_pth);
    Path_Release(hprgs_pth);
    Path_Release(husrdoc_pth);
    Path_Release(happdir_pth);
}

void PTHAPI PathRelativeToApp(LPWSTR lpszPath, const size_t cchPath, bool bSrcIsFile, bool bUnexpandEnv, bool bUnexpandMyDocs)
{
    HPATHL hpth_in_out = Path_Allocate(lpszPath);
    Path_RelativeToApp(hpth_in_out, bSrcIsFile, bUnexpandEnv, bUnexpandMyDocs);
    StringCchCopyW(lpszPath, ((cchPath == 0) ? PATHLONG_MAX_CCH : cchPath), PathGet(hpth_in_out));
    Path_Release(hpth_in_out);
}


//=============================================================================
//
//  PathGetDisplayName()
//
void PTHAPI Path_GetDisplayName(wchar_t* lpszDestPath, const size_t cchDestBuffer, const HPATHL hpth, const wchar_t* repl)
{
    if (!lpszDestPath || (cchDestBuffer == 0)) {
        return;
    }
    if (Path_GetLength(hpth) == 0) {
        StringCchCopyW(lpszDestPath, cchDestBuffer, repl ? repl : L"");
        return;
    }

    HPATHL hfnam_pth = Path_Allocate(PathGet(hpth));
    Path_StripPath(hfnam_pth);
    size_t const fnam_len = Path_GetLength(hfnam_pth);

    if (fnam_len >= cchDestBuffer) {
        // TODO: Exlorer like display name ???
        HPATHL   hpart_pth = Path_Allocate(PathGet(hfnam_pth));
        HSTRINGW hpart_str = ToHStrgW(hpart_pth);
        size_t const split_idx = (cchDestBuffer >> 1) - PATHDSPL_INFIX_LEN;
        StrgDelete(hpart_str, split_idx, (fnam_len - cchDestBuffer + (PATHDSPL_INFIX_LEN << 1)));
        StrgInsert(hpart_str, split_idx, PATHDSPL_INFIX);
        StringCchCopyW(lpszDestPath, cchDestBuffer, StrgGet(hpart_str));
        Path_Release(hpart_pth);
    }
    else {
        StringCchCopyW(lpszDestPath, cchDestBuffer, PathGet(hfnam_pth));
    }
    Path_Release(hfnam_pth);
}


void PTHAPI PathGetDisplayName(LPWSTR lpszDestPath, const size_t cchDestBuffer, LPCWSTR lpszSourcePath, const wchar_t* repl)
{
    HPATHL hpth = Path_Allocate(lpszSourcePath);
    Path_GetDisplayName(lpszDestPath, cchDestBuffer, hpth, repl);
    Path_Release(hpth);
}

#if 0
@@@
//=============================================================================
//
//  _SHGetFileInfoEx()
//
//  Return a default name when the file has been removed, and always append
//  a filename extension
//
static DWORD_PTR _SHGetFileInfoEx(LPCWSTR pszPath, DWORD dwFileAttributes, SHFILEINFO* psfi, UINT cbFileInfo, UINT uFlags)
{
    if (PathIsExistingFile(pszPath)) {
        DWORD_PTR dw = SHGetFileInfo(pszPath, dwFileAttributes, psfi, cbFileInfo, uFlags);
        if (StringCchLenW(psfi->szDisplayName, COUNTOF(psfi->szDisplayName)) < StringCchLen(PathFindFileName(pszPath), MAX_PATH)) {
            StringCchCat(psfi->szDisplayName, COUNTOF(psfi->szDisplayName), PathFindExtension(pszPath));
        }
        return (dw);
    }

    DWORD_PTR dw = SHGetFileInfo(pszPath, FILE_ATTRIBUTE_NORMAL, psfi, cbFileInfo, uFlags | SHGFI_USEFILEATTRIBUTES);
    if (StringCchLenW(psfi->szDisplayName, COUNTOF(psfi->szDisplayName)) < StringCchLen(PathFindFileName(pszPath), MAX_PATH)) {
        StringCchCat(psfi->szDisplayName, COUNTOF(psfi->szDisplayName), PathFindExtension(pszPath));
    }
    return (dw);
}

//=============================================================================
//
//  PathResolveDisplayName()
//
void PTHAPI PathGetDisplayName(LPWSTR lpszDestPath, DWORD cchDestBuffer, LPCWSTR lpszSourcePath)
{
    SHFILEINFO shfi;
    UINT const shfi_size = (UINT)sizeof(SHFILEINFO);
    ZeroMemory(&shfi, shfi_size);
    if (_SHGetFileInfoEx(lpszSourcePath, FILE_ATTRIBUTE_NORMAL, &shfi, shfi_size, SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES)) {
        StringCchCopy(lpszDestPath, cchDestBuffer, shfi.szDisplayName);
    }
    else {
        StringCchCopy(lpszDestPath, cchDestBuffer, PathFindFileName(lpszSourcePath));
    }
}
#endif



// ============================================================================
// Some Old MAX_PATH stuff
// TODO: refactor to DynStrg parameter
// ============================================================================



///////////////////////////////////////////////////////////////////////////////
//
//
//  Name: PathIsLnkFile()
//
//  Purpose: Determine whether pszPath is a Windows Shell Link File by
//           comparing the filename extension with L".lnk"
//
//  Manipulates:
//
bool PTHAPI PathIsLnkFile(LPCWSTR pszPath)
{
    WCHAR tchResPath[MAX_PATH] = { L'\0' };

    if (!pszPath || !*pszPath) {
        return false;
    }

    if (_wcsicmp(PathFindExtension(pszPath), L".lnk") != 0) {
        return false;
    }
    return PathGetLnkPath(pszPath, tchResPath, COUNTOF(tchResPath));
}


bool PTHAPI Path_IsLnkFile(const HPATHL hpth)
{
    HSTRINGW hstr = ToHStrgW(hpth); // inplace hpth_in_out
    if (!hstr) {
        return false;
    }

    WCHAR wchPath[MAX_PATH] = { L'\0' };
    StringCchCopy(wchPath, COUNTOF(wchPath), StrgGet(hstr));
    bool const res = PathIsLnkFile(wchPath);
    return res;
}


///////////////////////////////////////////////////////////////////////////////
//
//
//  Name: PathGetLnkPath()
//
//  Purpose: Try to get the path to which a lnk-file is linked
//
//
//  Manipulates: pszResPath
//
bool PTHAPI PathGetLnkPath(LPCWSTR pszLnkFile, LPWSTR pszResPath, int cchResPath)
{
    IShellLink*     psl = NULL;
    WIN32_FIND_DATA fd = { 0 };
    bool            bSucceeded = false;

    if (SUCCEEDED(CoCreateInstance(&CLSID_ShellLink, NULL,
            CLSCTX_INPROC_SERVER,
            &IID_IShellLink, (void**)&psl))) {
        IPersistFile* ppf = NULL;

        if (SUCCEEDED(psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, (void**)&ppf))) {
            WORD wsz[MAX_PATH] = { L'\0' };

            /*MultiByteToWideCharEx(CP_ACP,MB_PRECOMPOSED,pszLnkFile,-1,wsz,MAX_PATH);*/
            StringCchCopy(wsz, COUNTOF(wsz), pszLnkFile);

            if (SUCCEEDED(ppf->lpVtbl->Load(ppf, wsz, STGM_READ))) {
                if (NOERROR == psl->lpVtbl->GetPath(psl, pszResPath, cchResPath, &fd, 0)) {
                    bSucceeded = true;
                }
            }
            ppf->lpVtbl->Release(ppf);
        }
        psl->lpVtbl->Release(psl);
    }

    // This additional check seems reasonable
    if (StrIsEmptyW(pszResPath)) {
        bSucceeded = false;
    }

    if (bSucceeded) {
        PathCanonicalizeEx(pszResPath, cchResPath);
    }

    return (bSucceeded);
}


bool PTHAPI Path_GetLnkPath(const HPATHL hlnk_pth, HPATHL hpth_out)
{
    HSTRINGW hstr_lnk = ToHStrgW(hlnk_pth); // inplace hpth_in_out
    if (!hstr_lnk || !hpth_out) {
        return false;
    }

    WCHAR wchLnkPath[MAX_PATH] = { L'\0' };
    StringCchCopy(wchLnkPath, COUNTOF(wchLnkPath), StrgGet(hstr_lnk));

    WCHAR wchOutPath[MAX_PATH] = { L'\0' };
    bool const res = PathGetLnkPath(wchLnkPath, wchOutPath, COUNTOF(wchOutPath));

    Path_Reset(hpth_out, wchOutPath);

    return res;
}


///////////////////////////////////////////////////////////////////////////////
//
//
//  Name: PathIsLnkToDirectory()
//
//  Purpose: Determine wheter pszPath is a Windows Shell Link File which
//           refers to a directory
//
//  Manipulates: pszResPath
//
bool PTHAPI PathIsLnkToDirectory(LPCWSTR pszPath, LPWSTR pszResPath, int cchResPath)
{
    if (PathIsLnkFile(pszPath)) {
        WCHAR tchResPath[MAX_PATH] = { L'\0' };
        if (PathGetLnkPath(pszPath, tchResPath, sizeof(WCHAR) * COUNTOF(tchResPath))) {
            if (PathIsDirectory(tchResPath)) {
                StringCchCopyN(pszResPath, cchResPath, tchResPath, COUNTOF(tchResPath));
                return (true);
            }
        }
    }
    return false;
}


bool PTHAPI Path_IsLnkToDirectory(const HPATHL hlnk_pth, HPATHL hpth_out)
{
    HSTRINGW hstr_lnk = ToHStrgW(hlnk_pth); // inplace hpth_in_out
    if (!hstr_lnk || !hpth_out) {
        return false;
    }

    WCHAR wchLnkPath[MAX_PATH] = { L'\0' };
    StringCchCopy(wchLnkPath, COUNTOF(wchLnkPath), StrgGet(hstr_lnk));

    WCHAR      wchOutPath[MAX_PATH] = { L'\0' };
    bool const res = PathIsLnkToDirectory(wchLnkPath, wchOutPath, COUNTOF(wchOutPath));

    Path_Reset(hpth_out, wchOutPath);

    return res;
}


///////////////////////////////////////////////////////////////////////////////
//
//
//  Name: PathCreateDeskLnk()
//
//  Purpose: Modified to create a desktop link to Notepad2
//
//  Manipulates:
//
bool PTHAPI PathCreateDeskLnk(LPCWSTR pszDocument, LPCWSTR pszDescription)
{
    WCHAR tchExeFile[MAX_PATH] = { L'\0' };
    WCHAR tchDocTemp[MAX_PATH] = { L'\0' };
    WCHAR tchArguments[MAX_PATH + 16] = { L'\0' };
    WCHAR tchLinkDir[MAX_PATH] = { L'\0' };

    WCHAR tchLnkFileName[MAX_PATH] = { L'\0' };

    IShellLink* psl;
    bool        bSucceeded = false;
    BOOL        fMustCopy;

    if (StrIsEmptyW(pszDocument)) {
        return true;
    }

    // init strings
    GetModuleFileName(NULL, tchExeFile, COUNTOF(tchExeFile));
    PathCanonicalizeEx(tchExeFile, COUNTOF(tchExeFile));

    StringCchCopy(tchDocTemp, COUNTOF(tchDocTemp), pszDocument);
    PathQuoteSpaces(tchDocTemp);

    StringCchCopy(tchArguments, COUNTOF(tchArguments), L"-n ");
    StringCchCat(tchArguments, COUNTOF(tchArguments), tchDocTemp);

    PathGetKnownFolder(&FOLDERID_Desktop, tchLinkDir, COUNTOF(tchLinkDir));

    // Try to construct a valid filename...
    if (!SHGetNewLinkInfo(pszDocument, tchLinkDir, tchLnkFileName, &fMustCopy, SHGNLI_PREFIXNAME)) {
        return false;
    }

    if (SUCCEEDED(CoCreateInstance(&CLSID_ShellLink, NULL,
            CLSCTX_INPROC_SERVER,
            &IID_IShellLink, (void**)&psl))) {
        IPersistFile* ppf;

        if (SUCCEEDED(psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, (void**)&ppf))) {
            WORD wsz[MAX_PATH] = { L'\0' };

            /*MultiByteToWideCharEx(CP_ACP,MB_PRECOMPOSED,tchLnkFileName,-1,wsz,MAX_PATH);*/
            StringCchCopy(wsz, COUNTOF(wsz), tchLnkFileName);

            psl->lpVtbl->SetPath(psl, tchExeFile);
            psl->lpVtbl->SetArguments(psl, tchArguments);
            psl->lpVtbl->SetDescription(psl, pszDescription);

            if (SUCCEEDED(ppf->lpVtbl->Save(ppf, wsz, true))) {
                bSucceeded = true;
            }

            ppf->lpVtbl->Release(ppf);
        }
        psl->lpVtbl->Release(psl);
    }

    return (bSucceeded);
}


///////////////////////////////////////////////////////////////////////////////
//
//
//  Name: PathCreateFavLnk()
//
//  Purpose: Modified to create a Notepad2 favorites link
//
//  Manipulates:
//
bool PTHAPI PathCreateFavLnk(LPCWSTR pszName, LPCWSTR pszTarget, LPCWSTR pszDir)
{

    WCHAR tchLnkFileName[MAX_PATH] = { L'\0' };

    IShellLink* psl;
    bool        bSucceeded = false;

    if (StrIsEmptyW(pszName)) {
        return true;
    }

    StringCchCopy(tchLnkFileName, COUNTOF(tchLnkFileName), pszDir);
    PathAppend(tchLnkFileName, pszName);
    StringCchCat(tchLnkFileName, COUNTOF(tchLnkFileName), L".lnk");

    if (PathIsExistingFile(tchLnkFileName)) {
        return false;
    }

    if (SUCCEEDED(CoCreateInstance(&CLSID_ShellLink, NULL,
            CLSCTX_INPROC_SERVER,
            &IID_IShellLink, (void**)&psl))) {
        IPersistFile* ppf;

        if (SUCCEEDED(psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, (void**)&ppf))) {
            WORD wsz[MAX_PATH] = { L'\0' };

            /*MultiByteToWideCharEx(CP_ACP,MB_PRECOMPOSED,tchLnkFileName,-1,wsz,MAX_PATH);*/
            StringCchCopy(wsz, COUNTOF(wsz), tchLnkFileName);

            psl->lpVtbl->SetPath(psl, pszTarget);

            if (SUCCEEDED(ppf->lpVtbl->Save(ppf, wsz, true))) {
                bSucceeded = true;
            }

            ppf->lpVtbl->Release(ppf);
        }
        psl->lpVtbl->Release(psl);
    }

    return (bSucceeded);
}







// ============================================================================
// deprecated (intermediate only)
// ============================================================================

const wchar_t* PTHAPI Path_Get(HPATHL hpth)
{
    return StrgGet((HSTRINGW)hpth);
}
// ----------------------------------------------------------------------------

size_t PTHAPI Path_GetBufCount(HPATHL hpth)
{
    return StrgGetAllocLength((HSTRINGW)hpth);
}
// ----------------------------------------------------------------------------

// get wchar buffer with at least MAX_PATH size
// TODO: get rid of this intermediate state handler
wchar_t* PTHAPI Path_WriteAccessBuf(HPATHL hpth, size_t len)
{
    return StrgWriteAccessBuf((HSTRINGW)hpth, max(len, MAX_PATH));
}


// ============================================================================
// ============================================================================

