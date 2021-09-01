
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

const wchar_t* const PATHDSPL_INFIX = L" ... ";
#define PATHDSPL_INFIX_LEN (COUNTOF(PATHDSPL_INFIX) - 1)

const wchar_t* const PATH_CSIDL_MYDOCUMENTS = L"%CSIDL:MYDOCUMENTS%";
#define PATH_CSIDL_MYDOCUMENTS_LEN (COUNTOF(PATH_CSIDL_MYDOCUMENTS) - 1)


#define PATHLONG_MAX_CCH   PATHCCH_MAX_CCH
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
#define _min_(x,y) (((x) > (y)) ? (y) : (x))
#define _RETCMPMIN_  { return (x > y) ? y : x; }
__forceinline size_t min_s(const size_t x, const size_t y) _RETCMPMIN_

#define _max_(x,y) (((x) < (y)) ? (y) : (x))
#define _RETCMPMAX_  { return (x < y) ? y : x; }
__forceinline size_t max_s(const size_t x, const size_t y) _RETCMPMAX_

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


inline static void PrependLongPathPrefix(HPATHL hpth_in_out, bool bForce)
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


bool PTHAPI Path_Canonicalize(HPATHL hpth_out, const HPATHL hpth_in)
{
    HSTRINGW hstr_out = ToHStrgW(hpth_out);
    if (!hstr_out)
        return false;

    DWORD const dwFlags = PATHCCH_ALLOW_LONG_PATHS;
    //  Windows 10, version 1703:
    //  PATHCCH_FORCE_ENABLE_LONG_NAME_PROCESS | PATHCCH_ENSURE_IS_EXTENDED_LENGTH_PATH
    //  PATHCCH_ENSURE_TRAILING_SLASH
    PrependLongPathPrefix(hpth_out, false);

    LPWSTR       wbuf_out = StrgWriteAccessBuf(hstr_out, Path_GetLength(hpth_in) + PATHLONG_PREFIX_LEN + 8);
    size_t const cch_out = StrgGetAllocLength(hstr_out);

    // PathCchCanonicalizeEx() does not convert forward slashes (/) into back slashes (\).
    // With untrusted input, this function by itself, cannot be used to convert
    // paths into a form that can be compared with other paths for sub-path or identity.
    // Callers that need that ability should convert forward to back slashes before
    // using this function.
    HSTRINGW hstr_in = ToHStrgW(hpth_in);
    StrgReplaceCh(hstr_in, L'/', L'\\');
    PathCchRemoveBackslashEx(wbuf_out, cch_out, NULL, NULL);
    bool const res = SUCCEEDED(PathCchCanonicalizeEx(wbuf_out, cch_out, StrgGet(hstr_in), dwFlags));
    StrgSanitize(hstr_out);

    return res;
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
        StrgSet(server_name_out, server_name);
    }
    return res;
}
// ----------------------------------------------------------------------------


bool PTHAPI Path_RemoveFileSpec(HPATHL hpth)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hstr)
        return false;
    
    PrependLongPathPrefix(hpth, false);

    size_t const hstr_len = StrgGetLength(hstr);

    LPWSTR wbuf = StrgWriteAccessBuf(hstr, hstr_len);
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

    size_t const idx = StrgReverseFind(hstr, L'\\');
    if (idx != STRINGW_INVALID_IDX) {
        StrgDelete(hstr, 0, idx + 1);
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

    LPWSTR wbuf = StrgWriteAccessBuf(hstr, hstr_len + 64);
    size_t cch = StrgGetAllocLength(hstr);

    bool const bOK = SUCCEEDED(PathCchRenameExtension(wbuf, cch, ext));
    StrgSanitize(hstr);

    return bOK;
}
// ----------------------------------------------------------------------------


void PTHAPI Path_ExpandEnvStrings(HPATHL hpth)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hstr)
        return;
    
    ExpandEnvironmentStrg(hstr);
}
// ----------------------------------------------------------------------------


void PTHAPI Path_GetModuleFileName(HPATHL hpth_out)
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
    HPATHL hprfx_pth = Path_Allocate(NULL);
    HPATHL hsrch_pth = Path_Allocate(NULL);

    // TODO: need normalize here ???
    Path_Canonicalize(hprfx_pth, hprefix);
    Path_Canonicalize(hsrch_pth, hpth);

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
    ///bool const bIsDirectory = (dwFileAttrib & FILE_ATTRIBUTE_DIRECTORY);

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
void PTHAPI ExpandEnvironmentStrg(HSTRINGW hstr_in_out)
{
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
    HSTRINGW hstr = StrgCreate();
    StrgSet(hstr, lpSrc);
    ExpandEnvironmentStrg(hstr);
    const wchar_t* buf = StrgGet(hstr);
    if (buf) {
        StringCchCopyW(lpSrc, cchSrc, buf);
    }
    StrgDestroy(hstr);
}


//=============================================================================
//
//  _Path_IsRelative()
//
static bool _Path_IsRelative(const HPATHL hpth)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hstr)
        return true;

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
        StrgSet(hstr_out, pszPath);
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

   ExpandEnvironmentStrg(hstr_io); // inplace hpth_in_out

   bool res = false;
   if (_Path_IsRelative(hpth_in_out)) {

       HPATHL hmod_pth = Path_Allocate(NULL);
       Path_GetModuleFileName(hmod_pth);
       Path_RemoveFileSpec(hmod_pth);
       Path_Append(hmod_pth, hpth_in_out);
       res = Path_Canonicalize(hpth_in_out, hmod_pth);

       Path_Release(hmod_pth);
   }
   else {
       HPATHL const pth_cpy = Path_Allocate(StrgGet(hstr_io));
       res = Path_Canonicalize(hpth_in_out, pth_cpy);
       Path_Release(pth_cpy);
   }
   PrependLongPathPrefix(hpth_in_out, false);
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

    ExpandEnvironmentStrg(hstr_io);

    // strg beg/end: clear spaces and quote s
    StrgTrim(hstr_io, L'"');
    StrgTrim(hstr_io, L'\'');

    if (_Path_IsRelative(hpth_in_out)) {
        HPATHL hsrch_pth = Path_Allocate(PathGet(hpth_wrkdir));
        Path_Append(hsrch_pth, hpth_in_out);
        if (bSearchPathIfRelative) {
            if (!Path_IsExistingFile(hsrch_pth)) {
                Path_StripPath(hsrch_pth);
                HSTRINGW hsrch_str = StrgCreate();
                wchar_t* const buf = StrgWriteAccessBuf(hsrch_str, PATHLONG_MAX_CCH);
                if (SearchPathW(NULL, PathGet(hsrch_pth), NULL, PATHLONG_MAX_CCH, buf, NULL) != 0) {
                    Path_Reset(hpth_in_out, buf);
                    //PrependLongPathPrefix(hpth_in_out, false);
                }
                else {
                    Path_Reset(hpth_in_out, PathGet(hsrch_pth));
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

            HSTRINGW       hstr = StrgCreate();
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
    HPATHL hmod_pth = Path_Allocate(NULL);
    Path_GetModuleFileName(hmod_pth);
    Path_RemoveFileSpec(hmod_pth);
    Path_Canonicalize(hpth_out, hmod_pth);
    Path_Release(hmod_pth);
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
        Path_Reset(hpth_in_out, PathGet(htmp_pth));
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
// Need LongPath Impl of:
// - PathRelativePathTo()
// - PathUnExpandEnvStrings()
//
#if 0

static bool Path_RelativePathTo(HPATHL hrecv, const HPATHL hfrom, DWORD attr_from, const HPATHL hto, DWORD attr_to)
{


}

// TODO: fill dummy here
static bool Path_UnExpandEnvStrings(const HPATHL hpth_in, HPATHL hpth_out)
{
    if (!hpth_in || !hpth_out) {
        return false;
    }
    Path_Reset(hpth_out, PathGet(hpth_in)); // dummy: no expand, just copy
    return true;
}


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
    HPATHL hres_pth = Path_Allocate(NULL);

    DWORD dwAttrTo = (bSrcIsFile) ? FILE_ATTRIBUTE_NORMAL : FILE_ATTRIBUTE_DIRECTORY;
    if (bUnexpandMyDocs &&
        !_Path_IsRelative(hpth_in_out) &&
        !Path_IsPrefix(husrdoc_pth, happdir_pth) &&
        Path_IsPrefix(husrdoc_pth, hpth_in_out) &&
        Path_RelativePathTo(htmp_pth, husrdoc_pth, FILE_ATTRIBUTE_DIRECTORY, hpth_in_out, dwAttrTo))
    {
        Path_Reset(hres_pth, PATH_CSIDL_MYDOCUMENTS);
        Path_Append(hres_pth, htmp_pth);
    }
    else if (_Path_IsRelative(hpth_in_out) || Path_CommonPrefix(happdir_pth, hprgs_pth, NULL)) {
        Path_Reset(hres_pth, PathGet(hpth_in_out));
    }
    else {
        if (!Path_RelativePathTo(hres_pth, happdir_pth, FILE_ATTRIBUTE_DIRECTORY, hpth_in_out, dwAttrTo)) {
            Path_Reset(hres_pth, PathGet(hpth_in_out));
        }
    }
    // -> result in 'hres_pth'

    if (!bUnexpandEnv || !Path_UnExpandEnvStrings(hres_pth, hpth_in_out)) {
        Path_Reset(hpth_in_out, PathGet(hres_pth));
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

#endif



void PTHAPI PathRelativeToApp(LPWSTR lpszSrc, LPWSTR lpszDest, int cchDest, bool bSrcIsFile, bool bUnexpandEnv, bool bUnexpandMyDocs)
{
    WCHAR wchAppDir[MAX_PATH] = { L'\0' };
    WCHAR wchWinDir[MAX_PATH] = { L'\0' };
    WCHAR wchUserFiles[MAX_PATH] = { L'\0' };
    WCHAR wchPath[MAX_PATH] = { L'\0' };
    WCHAR wchResult[MAX_PATH] = { L'\0' };
    DWORD dwAttrTo = (bSrcIsFile) ? FILE_ATTRIBUTE_NORMAL : FILE_ATTRIBUTE_DIRECTORY;

    PathGetAppDirectory(wchAppDir, COUNTOF(wchAppDir));

    PathGetKnownFolder(&FOLDERID_Windows, wchWinDir, COUNTOF(wchWinDir));  // deprecated ???
    PathGetKnownFolder(&FOLDERID_Documents, wchUserFiles, COUNTOF(wchUserFiles));

    if (bUnexpandMyDocs &&
        !PathIsRelative(lpszSrc) &&
        !PathIsPrefix(wchUserFiles, wchAppDir) &&
        PathIsPrefix(wchUserFiles, lpszSrc) &&
        PathRelativePathToW(wchPath, wchUserFiles, FILE_ATTRIBUTE_DIRECTORY, lpszSrc, dwAttrTo))
    {
        StringCchCopy(wchUserFiles, COUNTOF(wchUserFiles), L"%CSIDL:MYDOCUMENTS%");
        PathAppend(wchUserFiles, wchPath);
        StringCchCopy(wchPath, COUNTOF(wchPath), wchUserFiles);
    }
    else if (PathIsRelative(lpszSrc) || PathCommonPrefix(wchAppDir, wchWinDir, NULL)) {
        StringCchCopyN(wchPath, COUNTOF(wchPath), lpszSrc, COUNTOF(wchPath));
    }
    else {
        if (!PathRelativePathTo(wchPath, wchAppDir, FILE_ATTRIBUTE_DIRECTORY, lpszSrc, dwAttrTo)) {
            StringCchCopyN(wchPath, COUNTOF(wchPath), lpszSrc, COUNTOF(wchPath));
        }
    }

    if (bUnexpandEnv) {
        if (!PathUnExpandEnvStrings(wchPath, wchResult, COUNTOF(wchResult))) {
            StringCchCopyN(wchResult, COUNTOF(wchResult), wchPath, COUNTOF(wchResult));
        }
    }
    else {
        StringCchCopyN(wchResult, COUNTOF(wchResult), wchPath, COUNTOF(wchResult));
    }
    int cchLen = (cchDest == 0) ? MAX_PATH : cchDest;
    if (lpszDest == NULL || lpszSrc == lpszDest) {
        StringCchCopyN(lpszSrc, cchLen, wchResult, cchLen);
    }
    else {
        StringCchCopyN(lpszDest, cchLen, wchResult, cchLen);
    }
}



//=============================================================================
//
//  PathGetDisplayName()
//
void PTHAPI Path_GetDisplayName(wchar_t* lpszDestPath, const size_t cchDestBuffer, const HPATHL hpth_in, const wchar_t* repl)
{
    if (!lpszDestPath || (cchDestBuffer == 0)) {
        return;
    }
    if (Path_GetLength(hpth_in) == 0) {
        StringCchCopyW(lpszDestPath, cchDestBuffer, repl);
        return;
    }

    size_t idx = StrgReverseFind(ToHStrgW(hpth_in), L'\\');
    idx = ((idx == STRINGW_INVALID_IDX) ? 0 : idx);

    HPATHL hfnam_pth = Path_Allocate(PathGet(hpth_in));
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

