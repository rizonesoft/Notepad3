
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

#define PATHLONG_MAX_CCH   PATHCCH_MAX_CCH
// TODO: ...
#define limit_len(len) (((len) < PATHLONG_MAX_CCH) ? (len) : (PATHLONG_MAX_CCH - 1))


#define IS_VALID_HANDLE(HNDL) ((HNDL) && ((HNDL) != INVALID_HANDLE_VALUE))

//==== StrSafe extensions =======================================================

__forceinline bool StrIsEmptyW(LPCWSTR s)
{
    return (!s || (*s == L'\0'));
}

//inline size_t StringCchLenA(LPCSTR s, size_t n) {
//  n = (n ? n : STRSAFE_MAX_CCH); size_t len; return (size_t)(!s ? 0 : (SUCCEEDED(StringCchLengthA(s, n, &len)) ? len : n));
//}
inline size_t StringCchLenA(LPCSTR s, size_t n)
{
    n = (n ? n : STRSAFE_MAX_CCH);
    return (s ? strnlen_s(s, n) : 0LL);
}

//inline size_t StringCchLenW(LPCWSTR s, size_t n) {
//  n = (n ? n : STRSAFE_MAX_CCH); size_t len; return (size_t)(!s ? 0 : (SUCCEEDED(StringCchLengthW(s, n, &len)) ? len : n));
//}
inline size_t StringCchLenW(LPCWSTR s, size_t n)
{
    n = (n ? n : STRSAFE_MAX_CCH);
    return (s ? wcsnlen_s(s, n) : 0LL);
}

#if defined(UNICODE) || defined(_UNICODE)
#define StringCchLen(s, n) StringCchLenW((s), (n))
#else
#define StringCchLen(s, n) StringCchLenA((s), (n))
#endif

// ----------------------------------------------------------------------------


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
    LPCWSTR wmore = Path_Get(hmore);

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


// This function does not convert forward slashes (/) into back slashes (\).
// With untrusted input, this function by itself, cannot be used to convert
// paths into a form that can be compared with other paths for sub-path or identity.
// Callers that need that ability should convert forward to back slashes before
// using this function.
//
bool PTHAPI Path_Canonicalize(HPATHL hpth_out, HPATHL hpth_in)
{
    HSTRINGW hstr_out = ToHStrgW(hpth_out);
    if (!hstr_out)
        return false;

    LPCWSTR wbuf_in = Path_Get(hpth_in);

    LPWSTR wbuf_out = StrgWriteAccessBuf(hstr_out, Path_GetLength(hpth_in) + PATHLONG_PREFIX_LEN + 8);
    size_t const cch_out = StrgGetAllocLength(hstr_out);

    DWORD const dwFlags = PATHCCH_ALLOW_LONG_PATHS;
    //  Windows 10, version 1703:
    //  PATHCCH_FORCE_ENABLE_LONG_NAME_PROCESS | PATHCCH_ENSURE_IS_EXTENDED_LENGTH_PATH
    //  PATHCCH_ENSURE_TRAILING_SLASH
    bool const res = SUCCEEDED(PathCchCanonicalizeEx(wbuf_out, cch_out, wbuf_in, dwFlags));
    StrgSanitize(hstr_out);

    return res;
}
// ----------------------------------------------------------------------------



bool PTHAPI Path_RemoveFileSpec(HPATHL hpth)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hstr)
        return false;
    
    size_t const hstr_len = StrgGetLength(hstr);

    LPWSTR wbuf = StrgWriteAccessBuf(hstr, hstr_len);
    size_t cch = StrgGetAllocLength(hstr);

    bool const bOK = SUCCEEDED(PathCchRemoveFileSpec(wbuf, cch));
    StrgSanitize(hstr);

    return bOK;
}
// ----------------------------------------------------------------------------


bool PTHAPI Path_RenameExtension(HPATHL hpth, const wchar_t* ext)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hstr)
        return false;
    
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


bool PTHAPI Path_IsExistingFile(HPATHL hpth)
{
    HSTRINGW hstr = ToHStrgW(hpth);
    if (!hstr)
        return false;

    HSTRINGW hxpth = StrgCopy(hstr);
    if (!OptInRemoveMaxPathLimit()) {
        if (StrgFind(hxpth, PATHLONG_PREFIX, 0) != 0) {
            StrgInsert(hxpth, 0, PATHLONG_PREFIX);
        }
    }
    LPCWSTR expth_buf = StrgWriteAccessBuf(hxpth, 1);

    DWORD const dwFileAttrib = GetFileAttributesW(expth_buf);

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




// ============================================================================
// Old Stuff in INTERMEDIATE DEV state
// ============================================================================


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
//  GetLongPathNameEx()
//

DWORD PTHAPI Path_GetLongPathNameEx(HPATHL hpth_in_out)
{
    HSTRINGW hstr_io = ToHStrgW(hpth_in_out);
    if (!hstr_io)
        return 0UL;

    DWORD const    len = (size_t)GetLongPathNameW(StrgGet(hstr_io), NULL, 0);
    wchar_t* const buf = StrgWriteAccessBuf(hstr_io, (size_t)len);

    DWORD const dwRet = GetLongPathNameW(buf, buf, len);
    StrgSanitize(hstr_io);

    if (dwRet > 2UL) {
        wchar_t* const pos = wcschr(buf, L':');
        if (pos && (pos > buf)) {
            CharUpperBuffW(pos - 1, 1);
        }
    }
    return dwRet;
}


DWORD PTHAPI GetLongPathNameEx(LPWSTR lpszPath, DWORD cchBuffer)
{
    HPATHL      hpth = Path_Allocate(lpszPath);
    DWORD const dwRet = Path_GetLongPathNameEx(hpth);
    if (dwRet) {
        StringCchCopyW(lpszPath, cchBuffer, Path_Get(hpth));
    }
    Path_Release(hpth);
    return dwRet;
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
        StringCchCopyW(lpOutPath, cchOut, Path_Get(hpth));
    }
    Path_Release(hpth);
    return res;
}


//=============================================================================
//
//  PathCanonicalizeEx()
//

inline static bool _PathIsRelative(HPATHL hpth)
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
        size_t const pos = StrgReverseFind(cpy, L'\\');
        if (pos != STRINGW_INVALID_IDX) {
            buf[pos] = L'\0';
            res = PathIsRelativeW(StrgGet(cpy));
        }
    }
    else {
        res = PathIsRelativeW(StrgGet(cpy));
    }

    StrgDestroy(cpy);
    return res;
}


bool PTHAPI Path_CanonicalizeEx(HPATHL hpth_in_out)
{
    HSTRINGW hstr_io = ToHStrgW(hpth_in_out);
    if (!hstr_io)
        return false;

   ExpandEnvironmentStrg(hstr_io); // inplace hpth_in_out

   bool res = false;
   if (_PathIsRelative(hpth_in_out)) {

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
   return res;
}

bool PTHAPI PathCanonicalizeEx(LPWSTR lpszPath, size_t cchPath)
{
    HPATHL   hpth = Path_Allocate(lpszPath);
    bool const res = Path_CanonicalizeEx(hpth);
    if (res) {
        StringCchCopyW(lpszPath, cchPath, Path_Get(hpth));
    }
    Path_Release(hpth);
    return res;
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

void PTHAPI PathGetAppDirectory(LPWSTR lpszDest, DWORD cchDest)
{
    HPATHL hpth = Path_Allocate(NULL);
    Path_GetAppDirectory(hpth);
    const wchar_t* buf = Path_Get(hpth);
    if (buf) {
        StringCchCopyW(lpszDest, cchDest, buf);
    }
    Path_Release(hpth);
}




// ============================================================================
// Some Old MAX_PATH stuff
// TODO: refactor to DynStrg parameter
// ============================================================================


//=============================================================================
//
//  PathRelativeToApp()
//
void PTHAPI PathRelativeToApp(
    LPWSTR lpszSrc, LPWSTR lpszDest, int cchDest, bool bSrcIsFile,
    bool bUnexpandEnv, bool bUnexpandMyDocs)
{

    WCHAR wchAppDir[MAX_PATH] = { L'\0' };
    WCHAR wchWinDir[MAX_PATH] = { L'\0' };
    WCHAR wchUserFiles[MAX_PATH] = { L'\0' };
    WCHAR wchPath[MAX_PATH] = { L'\0' };
    WCHAR wchResult[MAX_PATH] = { L'\0' };
    DWORD dwAttrTo = (bSrcIsFile) ? FILE_ATTRIBUTE_NORMAL : FILE_ATTRIBUTE_DIRECTORY;

    PathGetAppDirectory(wchAppDir, COUNTOF(wchAppDir));

    (void)GetWindowsDirectory(wchWinDir, COUNTOF(wchWinDir));
    PathGetKnownFolder(&FOLDERID_Documents, wchUserFiles, COUNTOF(wchUserFiles));

    if (bUnexpandMyDocs &&
        !PathIsRelative(lpszSrc) &&
        !PathIsPrefix(wchUserFiles, wchAppDir) &&
        PathIsPrefix(wchUserFiles, lpszSrc) &&
        PathRelativePathTo(wchPath, wchUserFiles, FILE_ATTRIBUTE_DIRECTORY, lpszSrc, dwAttrTo)) {
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
//  PathAbsoluteFromApp()
//
void PTHAPI PathAbsoluteFromApp(LPWSTR lpszSrc, LPWSTR lpszDest, int cchDest, bool bExpandEnv)
{

    WCHAR wchPath[MAX_PATH] = { L'\0' };
    WCHAR wchResult[MAX_PATH] = { L'\0' };

    if (lpszSrc == NULL) {
        ZeroMemory(lpszDest, (cchDest == 0) ? MAX_PATH : cchDest);
        return;
    }

    if (StrCmpNI(lpszSrc, L"%CSIDL:MYDOCUMENTS%", CONSTSTRGLEN("%CSIDL:MYDOCUMENTS%")) == 0) {
        PathGetKnownFolder(&FOLDERID_Documents, wchPath, COUNTOF(wchPath));
        PathAppend(wchPath, lpszSrc + CONSTSTRGLEN("%CSIDL:MYDOCUMENTS%"));
    }
    else {
        StringCchCopyN(wchPath, COUNTOF(wchPath), lpszSrc, COUNTOF(wchPath));
    }

    if (bExpandEnv) {
        ExpandEnvironmentStringsEx(wchPath, COUNTOF(wchPath));
    }
    if (PathIsRelative(wchPath)) {
        PathGetAppDirectory(wchResult, COUNTOF(wchResult));
        PathAppend(wchResult, wchPath);
    }
    else {
        StringCchCopyN(wchResult, COUNTOF(wchResult), wchPath, COUNTOF(wchPath));
    }
    PathCanonicalizeEx(wchResult, MAX_PATH);
    if (PathGetDriveNumber(wchResult) != -1) {
        CharUpperBuff(wchResult, 1);
    }
    if (lpszDest == NULL || lpszSrc == lpszDest) {
        StringCchCopyN(lpszSrc, ((cchDest == 0) ? MAX_PATH : cchDest), wchResult, COUNTOF(wchResult));
    }
    else {
        StringCchCopyN(lpszDest, ((cchDest == 0) ? MAX_PATH : cchDest), wchResult, COUNTOF(wchResult));
    }
}



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


//=============================================================================
//
//  _SHGetFileInfoEx()
//
//  Return a default name when the file has been removed, and always append
//  a filename extension
//
static DWORD_PTR _SHGetFileInfoEx(LPCWSTR pszPath, DWORD dwFileAttributes,
    SHFILEINFO* psfi, UINT cbFileInfo, UINT uFlags)
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


//=============================================================================
//
//  NormalizePathEx()
//
DWORD PTHAPI NormalizePathEx(LPWSTR lpszPath, DWORD cchBuffer, LPCWSTR lpszWorkDir, bool bRealPath, bool bSearchPathIfRelative)
{
    WCHAR tmpFilePath[MAX_PATH] = { L'\0' };
    StringCchCopyN(tmpFilePath, COUNTOF(tmpFilePath), lpszPath, cchBuffer);
    ExpandEnvironmentStringsEx(tmpFilePath, COUNTOF(tmpFilePath));

    PathUnquoteSpaces(tmpFilePath);

    if (PathIsRelative(tmpFilePath)) {
        StringCchCopy(lpszPath, cchBuffer, lpszWorkDir);
        PathAppend(lpszPath, tmpFilePath);
        if (bSearchPathIfRelative) {
            if (!PathIsExistingFile(lpszPath)) {
                PathStripPath(tmpFilePath);
                if (SearchPath(NULL, tmpFilePath, NULL, cchBuffer, lpszPath, NULL) == 0) {
                    StringCchCopy(lpszPath, cchBuffer, tmpFilePath);
                }
            }
        }
    }
    else {
        StringCchCopy(lpszPath, cchBuffer, tmpFilePath);
    }

    PathCanonicalizeEx(lpszPath, cchBuffer);
    GetLongPathNameEx(lpszPath, cchBuffer);

    if (PathIsLnkFile(lpszPath)) {
        PathGetLnkPath(lpszPath, lpszPath, cchBuffer);
    }

    if (bRealPath) {
        // get real path name (by zufuliu)
        HANDLE hFile = CreateFile(lpszPath,     // file to open
            GENERIC_READ,                       // open for reading
            FILE_SHARE_READ | FILE_SHARE_WRITE, // share anyway
            NULL,                               // default security
            OPEN_EXISTING,                      // existing file only
            FILE_ATTRIBUTE_NORMAL,              // normal file
            NULL);                              // no attr. template

        if (IS_VALID_HANDLE(hFile)) {
            if (GetFinalPathNameByHandleW(hFile, tmpFilePath,
                    COUNTOF(tmpFilePath), FILE_NAME_OPENED) > 0) {
                if (StrCmpN(tmpFilePath, L"\\\\?\\", 4) == 0) {
                    WCHAR* p = tmpFilePath + 4;
                    if (StrCmpN(p, L"UNC\\", 4) == 0) {
                        p += 2;
                        *p = L'\\';
                    }
                    StringCchCopy(lpszPath, cchBuffer, p);
                }
            }
            CloseHandle(hFile);
        }
    }

    return (DWORD)StringCchLen(lpszPath, cchBuffer);
}




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
    return StrgWriteAccessBuf(hstr, max(len, MAX_PATH));
}
// ----------------------------------------------------------------------------

