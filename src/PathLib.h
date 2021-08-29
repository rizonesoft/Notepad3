/****************************************************************/
#pragma once

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

HPATHL PTHAPI               Path_Allocate(const wchar_t* path);
void PTHAPI                 Path_Release(HPATHL hstr);
int PTHAPI                  Path_Reset(HPATHL hpth, const wchar_t* path);
size_t PTHAPI               Path_GetLength(HPATHL hpth);
bool PTHAPI                 Path_RemoveFileSpec(HPATHL hpth);
void PTHAPI                 Path_ExpandEnvStrings(HPATHL hpth);
bool PTHAPI                 Path_IsExistingFile(HPATHL hpth);

// -------------------------------------------------------

const wchar_t* PTHAPI       Path_Get(HPATHL hpth);
size_t PTHAPI               Path_GetBufCount(HPATHL hpth);

// get wchar buffer with at least MAX_PATH size
// TODO: get rid of this intermediate state handler
wchar_t* PTHAPI Path_AccessBuf(HPATHL hpth, size_t len);

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

inline bool PathIsExistingFile(LPCWSTR pszPath)
{
    return (PathFileExists(pszPath) && !PathIsDirectory(pszPath));
}

inline bool IsReadOnly(const DWORD dwFileAttr)
{
    return ((dwFileAttr != INVALID_FILE_ATTRIBUTES) && (dwFileAttr & FILE_ATTRIBUTE_READONLY));
}

#ifndef NP3_PATH_LIB_IMPLEMENTATION

// including <pathcch.h> and linking against pathcch.lib
// api-ms-win-core-path-l1-1-0.dll  library : Minimum supported client is Windows 8 :-/
// so switch back to previous (deprecated) methods:

inline HRESULT PathCchAppend(PWSTR p, size_t l, PCWSTR a)
{
    UNREFERENCED_PARAMETER(l);
    return (PathAppend(p, a) ? S_OK : S_FALSE);
}
inline HRESULT PathCchCanonicalize(PWSTR p, size_t l, PCWSTR a)
{
    UNREFERENCED_PARAMETER(l);
    return (PathCanonicalize(p, a) ? S_OK : S_FALSE);
}
inline HRESULT PathCchRenameExtension(PWSTR p, size_t l, PCWSTR a)
{
    UNREFERENCED_PARAMETER(l);
    return (PathRenameExtension(p, a) ? S_OK : S_FALSE);
}
inline HRESULT PathCchRemoveFileSpec(PWSTR p, size_t l)
{
    UNREFERENCED_PARAMETER(l);
    return (PathRemoveFileSpec(p) ? S_OK : S_FALSE);
}

#endif

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
