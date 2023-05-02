// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Helpers.c                                                                   *
*   General helper functions                                                  *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*   Parts taken from SciTE, (c) Neil Hodgson                                  *
*   MinimizeToTray, (c) 2000 Matthew Ellis                                    *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2023   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

#include <shlobj.h>
#include <shellapi.h>
#include <ctype.h>
#include <wchar.h>

#include "Helpers.h"

#include "PathLib.h"
#include "Edit.h"
#include "Encoding.h"
#include "MuiLanguage.h"
#include "Notepad3.h"
#include "Dialogs.h"
#include "Config/Config.h"
#include "DarkMode/DarkMode.h"
#include "Version.h"

#pragma warning(push)
#pragma warning(disable : 4201) // union/struct w/o name
#include "tinyexpr/tinyexpr.h"
//#include "tinyexprcpp/tinyexpr_cif.h"
#pragma warning(pop)

#include "Scintilla.h"


//=============================================================================


#if (defined(_DEBUG) || defined(DEBUG)) && !defined(NDEBUG)
void DbgLog(const wchar_t *fmt, ...)
{
    static wchar_t buf[2048] = { 0 };
    va_list va;
    va_start(va, fmt);
    wvnsprintfW(buf, COUNTOF(buf), fmt, va);
    va_end(va);
    OutputDebugStringW(buf);
}
#endif

//=============================================================================
//
//  Cut of substrings defined by pattern
//
CHAR* StrCutIA(CHAR* s,const CHAR* pattern)
{
    CHAR* p = NULL;
    do {
        p = StrStrIA(s,pattern);
        if (p) {
            CHAR* q = p + StringCchLenA(pattern,0);
            while (*p != '\0') {
                *p++ = *q++;
            }
        }
    } while (p);
    return s;
}

WCHAR* StrCutIW(WCHAR* s,const WCHAR* pattern)
{
    WCHAR* p = NULL;
    do {
        p = StrStrIW(s,pattern);
        if (p) {
            WCHAR* q = p + StringCchLenW(pattern,0);
            while (*p != L'\0') {
                *p++ = *q++;
            }
        }
    } while (p);
    return s;
}
//=============================================================================



//=============================================================================
//
//  StrDelChrA()
//
bool StrDelChrA(LPSTR pszSource, LPCSTR pCharsToRemove)
{
    if (!pszSource || !pCharsToRemove) {
        return false;
    }

    LPSTR pch = pszSource;
    while (*pch) {
        LPSTR prem = pch;
        while (StrChrA(pCharsToRemove, *prem)) {
            ++prem;
        }
        if (prem > pch) {
            MoveMemory(pch, prem, sizeof(CHAR)*(StringCchLenA(prem,0) + 1));
        }
        ++pch;
    }
    return true;
}


//=============================================================================
//
//  StrDelChrW()
//
bool StrDelChrW(LPWSTR pszSource, LPCWSTR pCharsToRemove)
{
    if (!pszSource || !pCharsToRemove) {
        return false;
    }
    LPWSTR pch = pszSource;
    while (*pch) {
        LPWSTR prem = pch;
        while (StrChrW(pCharsToRemove, *prem)) {
            ++prem;
        }
        if (prem > pch) {
            MoveMemory(pch, prem, sizeof(WCHAR)*(StringCchLenW(prem,0) + 1));
        }
        ++pch;
    }
    return true;
}


//=============================================================================
//
//  Find next token in string
//
CHAR* StrNextTokA(CHAR* strg, const CHAR* tokens)
{
    CHAR* n = NULL;
    const CHAR* t = tokens;
    while (t && *t) {
        CHAR* const f = StrChrA(strg, *t);
        if (!n || (f && (f < n))) {
            n = f;
        }
        ++t;
    }
    return n;
}

WCHAR* StrNextTokW(WCHAR* strg, const WCHAR* tokens)
{
    WCHAR* n = NULL;
    const WCHAR* t = tokens;
    while (t && *t) {
        WCHAR* const f = StrChrW(strg, *t);
        if (!n || (f && (f < n))) {
            n = f;
        }
        ++t;
    }
    return n;
}

//=============================================================================
//
//  GetWinVersionString()
//

#if 0
static OSVERSIONINFOEX s_OSversion = { 0 };

static void _GetTrueWindowsVersion()
{
    if (s_OSversion.dwOSVersionInfoSize != 0) {
        return;
    }

    // clear
    ZeroMemory(&s_OSversion, sizeof(OSVERSIONINFOEX));
    s_OSversion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    // Function pointer to driver function
    void (WINAPI *pRtlGetVersion)(PRTL_OSVERSIONINFOW lpVersionInformation) = NULL;

    // load the System-DLL
    HINSTANCE const hNTdllDll = LoadLibrary(L"ntdll.dll");
    if (hNTdllDll) {
        // get the function pointer to RtlGetVersion
        pRtlGetVersion = (void (WINAPI*)(PRTL_OSVERSIONINFOW)) GetProcAddress(hNTdllDll, "RtlGetVersion");

        if (pRtlGetVersion != NULL) {
            pRtlGetVersion((PRTL_OSVERSIONINFOW)& s_OSversion);
        }
        FreeLibrary(hNTdllDll);
    } // if (hNTdllDll != NULL)

#pragma warning ( push )
#pragma warning ( disable: 4996 )
    // if function failed, use fallback to old version
    if (pRtlGetVersion == NULL) {
        GetVersionEx((OSVERSIONINFO*)& s_OSversion);
    }
#pragma warning ( pop )

}
// ----------------------------------------------------------------------------
#endif


void GetWinVersionString(LPWSTR szVersionStr, size_t cchVersionStr)
{
    StringCchCopy(szVersionStr, cchVersionStr, L"OS Version: Windows ");

    DWORD const build = GetWindowsBuildNumber(NULL, NULL);

    if (IsWindows10OrGreater()) {
        StringCchCat(szVersionStr, cchVersionStr, IsWindowsServer() ? ((build >= 17134) ? L"Server 2019 " : L"Server 2016 ") :
                                                                      ((build >= 22000) ? L"11 " : L"10 "));          
    } else if (IsWindows8Point1OrGreater()) {
        StringCchCat(szVersionStr, cchVersionStr, IsWindowsServer() ? L"Server 2012 R2 " : L"8.1");
    } else if (IsWindows8OrGreater()) {
        StringCchCat(szVersionStr, cchVersionStr, IsWindowsServer() ? L"Server 2012 " : L"8");
    } else if (IsWindows7SP1OrGreater()) {
        StringCchCat(szVersionStr, cchVersionStr, IsWindowsServer() ? L"Server 2008 R2 " : L"7 (SP1)");
    } else if (IsWindows7OrGreater()) {
        StringCchCat(szVersionStr, cchVersionStr, IsWindowsServer() ? L"Server 2008 " : L"7");
    } else {
        StringCchCat(szVersionStr, cchVersionStr, IsWindowsServer() ? L"Unkown Server " : L"?");
    }

    if (IsWindows10OrGreater()) {
        WCHAR win10ver[80] = { L'\0' };
        StringCchPrintf(win10ver, COUNTOF(win10ver), L" Version %s (Build %lu)", _Win10BuildToReleaseId(build), GetWindowsBuildNumber(NULL, NULL));
        StringCchCat(szVersionStr, cchVersionStr, win10ver);
    }
}


//=============================================================================
//
//  SetClipboardTextW()
//
bool SetClipboardText(HWND hwnd, LPCWSTR pszTextW, size_t cchTextW)
{
    bool ok = false;
    HANDLE const hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (cchTextW + 1) * sizeof(WCHAR));
    if (hData) {
        WCHAR* const pszNew = GlobalLock(hData);
        if (pszNew) {
            StringCchCopy(pszNew, cchTextW + 1, pszTextW);
            GlobalUnlock(hData);
            if (OpenClipboard(hwnd)) {
                EmptyClipboard();
                ok = (SetClipboardData(CF_UNICODETEXT, hData) != NULL); // move ownership
                CloseClipboard();
            }
        }
        if (!ok) {
            GlobalFree(hData);
        }
    }
    return ok;
}


//=============================================================================
//
//  PrivateSetCurrentProcessExplicitAppUserModelID()
//
HRESULT PrivateSetCurrentProcessExplicitAppUserModelID(PCWSTR AppID)
{
    if (StrIsEmpty(AppID)) {
        return(S_OK);
    }

    if (StringCchCompareXI(AppID, L"(default)") == 0) {
        return(S_OK);
    }

    return SetCurrentProcessExplicitAppUserModelID(AppID);
}


//=============================================================================
//
//  IsProcessElevated()
//
//   PURPOSE: The function gets the elevation information of the current
//   process. It dictates whether the process is elevated or not. Token
//   elevation is only available on Windows Vista and newer operating
//   systems, thus IsProcessElevated throws a C++ exception if it is called
//   on systems prior to Windows Vista. It is not appropriate to use this
//   function to determine whether a process is run as administartor.
//
//   RETURN VALUE: Returns TRUE if the process is elevated. Returns FALSE if
//   it is not.
//
//   NOTE: TOKEN_INFORMATION_CLASS provides TokenElevationType to check the
//   elevation type (TokenElevationTypeDefault / TokenElevationTypeLimited /
//   TokenElevationTypeFull) of the process. It is different from
//   TokenElevation in that, when UAC is turned off, elevation type always
//   returns TokenElevationTypeDefault even though the process is elevated
//   (Integrity Level == High). In other words, it is not safe to say if the
//   process is elevated based on elevation type. Instead, we should use
//   TokenElevation.
//
bool IsProcessElevated()
{

    // When the process is run on operating systems prior to Windows
    // Vista, GetTokenInformation returns FALSE with the
    // ERROR_INVALID_PARAMETER error code because TokenElevation is
    // not supported on those operating systems.
    if (!IsWindowsVistaOrGreater()) {
        return false;
    }

    bool bIsElevated = false;
    HANDLE hToken = NULL;
    Globals.dwLastError = ERROR_SUCCESS;
    const WCHAR* pLastErrMsg = L"";

    do {
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
            TOKEN_ELEVATION elevationToken;
            DWORD expectedRetVal = sizeof(TOKEN_ELEVATION);
            DWORD dwReturnLength = 0;
            if (GetTokenInformation(hToken, TokenElevation, &elevationToken, expectedRetVal, &dwReturnLength)) {
                if (dwReturnLength == expectedRetVal) {
                    bIsElevated = elevationToken.TokenIsElevated;
                }
            }
        } else {
            Globals.dwLastError = GetLastError();
            pLastErrMsg = L"IsProcessElevated()";
            break;
        }
    } while (false); // Centralized cleanup for all allocated resources.

    if (hToken) {
        CloseHandle(hToken);
        hToken = NULL;
    }

    if (Globals.dwLastError != ERROR_SUCCESS) {
        MsgBoxLastError(pLastErrMsg, Globals.dwLastError);
    }

    return bIsElevated;
}

#if 0
//=============================================================================
//
//  IsUserAdmin()
//
// Routine Description: This routine returns TRUE if the caller's
// process is a member of the Administrators local group. Caller is NOT
// expected to be impersonating anyone and is expected to be able to
// open its own process and process token.
// Arguments: None.
// Return Value:
// true - Caller has Administrators local group.
// false - Caller does not have Administrators local group. --
//
bool IsUserAdmin()
{
    PSID pAdminGroup;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    BOOL bIsAdmin = AllocateAndInitializeSid(&NtAuthority, 2,
                    SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdminGroup);
    if (bIsAdmin) {
        if (!CheckTokenMembership(NULL, pAdminGroup, &bIsAdmin)) {
            bIsAdmin = false;
        }
        FreeSid(pAdminGroup);
    }
    return bIsAdmin;
}
#endif

//=============================================================================
//
//   IsUserInAdminGroup()
//
//   PURPOSE: The function checks whether the primary access token of the
//   process belongs to user account that is a member of the local
//   Administrators group, even if it currently is not elevated.
//
//   RETURN VALUE: Returns TRUE if the primary access token of the process
//   belongs to user account that is a member of the local Administrators
//   group. Returns FALSE if the token does not.
//
//
//
bool IsUserInAdminGroup()
{
    BOOL fInAdminGroup = FALSE;
    HANDLE hToken = NULL;
    HANDLE hTokenToCheck = NULL;

    Globals.dwLastError = ERROR_SUCCESS;
    const WCHAR* pLastErrMsg = L"";

    do {
        // Open the primary access token of the process for query and duplicate.
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_DUPLICATE, &hToken)) {
            Globals.dwLastError = GetLastError();
            pLastErrMsg = L"OpenProcessToken()";
            break;
        }

        DWORD cbSize = 0;

        // Determine token type: limited, elevated, or default.
        TOKEN_ELEVATION_TYPE elevType;
        if (!GetTokenInformation(hToken, TokenElevationType, &elevType, sizeof(elevType), &cbSize)) {
            Globals.dwLastError = GetLastError();
            pLastErrMsg = L"GetTokenInformation()";
            break;
        }

        // If limited, get the linked elevated token for further check.
        if (TokenElevationTypeLimited == elevType) {
            if (!GetTokenInformation(hToken, TokenLinkedToken, &hTokenToCheck, sizeof(hTokenToCheck), &cbSize)) {
                Globals.dwLastError = GetLastError();
                pLastErrMsg = L"GetTokenInformation()";
                break;
            }
        }

        // CheckTokenMembership requires an impersonation token. If we just got a
        // linked token, it already is an impersonation token.  If we did not get
        // a linked token, duplicate the original into an impersonation token for
        // CheckTokenMembership.
        if (!hTokenToCheck) {
            if (!DuplicateToken(hToken, SecurityIdentification, &hTokenToCheck)) {
                Globals.dwLastError = GetLastError();
                pLastErrMsg = L"DuplicateToken()";
                break;
            }
        }

        // Create the SID corresponding to the Administrators group.
        BYTE adminSID[SECURITY_MAX_SID_SIZE];
        cbSize = sizeof(adminSID);
        if (!CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, &adminSID, &cbSize)) {
            Globals.dwLastError = GetLastError();
            pLastErrMsg = L"CreateWellKnownSid()";
            break;
        }

        // Check if the token to be checked contains admin SID.
        // http://msdn.microsoft.com/en-us/library/aa379596(VS.85).aspx:
        // To determine whether a SID is enabled in a token, that is, whether it
        // has the SE_GROUP_ENABLED attribute, call CheckTokenMembership.
        if (!CheckTokenMembership(hTokenToCheck, &adminSID, &fInAdminGroup)) {
            Globals.dwLastError = GetLastError();
            pLastErrMsg = L"CheckTokenMembership()";
            break;
        }

    } while (false); // Centralized cleanup for all allocated resources.

    if (hToken) {
        CloseHandle(hToken);
        hToken = NULL;
    }
    if (hTokenToCheck) {
        CloseHandle(hTokenToCheck);
        hTokenToCheck = NULL;
    }

    if (Globals.dwLastError != ERROR_SUCCESS) {
        MsgBoxLastError(pLastErrMsg, Globals.dwLastError);
    }

    return fInAdminGroup;
}


//=============================================================================
//
//   IsRunAsAdmin()
//
//   PURPOSE: The function checks whether the current process is run as
//   administrator. In other words, it dictates whether the primary access
//   token of the process belongs to user account that is a member of the
//   local Administrators group and it is elevated.
//
//   RETURN VALUE: Returns TRUE if the primary access token of the process
//   belongs to user account that is a member of the local Administrators
//   group and it is elevated. Returns FALSE if the token does not.
//
bool IsRunAsAdmin()
{
    BOOL fIsRunAsAdmin = FALSE;
    PSID pAdministratorsGroup = NULL;

    Globals.dwLastError = ERROR_SUCCESS;

    do {
        //const WCHAR* pLastErrMsg = L"";

        // Allocate and initialize a SID of the administrators group.
        SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
        if (!AllocateAndInitializeSid(
                    &NtAuthority,
                    2,
                    SECURITY_BUILTIN_DOMAIN_RID,
                    DOMAIN_ALIAS_RID_ADMINS,
                    0, 0, 0, 0, 0, 0,
                    &pAdministratorsGroup)) {
            Globals.dwLastError = GetLastError();
            //pLastErrMsg = L"AllocateAndInitializeSid()";
            break;
        }

        // Determine whether the SID of administrators group is enabled in
        // the primary access token of the process.
        if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin)) {
            Globals.dwLastError = GetLastError();
            //pLastErrMsg = L"CheckTokenMembership()";
            break;
        }

    } while (false); // Centralized cleanup for all allocated resources.

    // Centralized cleanup for all allocated resources.
    if (pAdministratorsGroup) {
        FreeSid(pAdministratorsGroup);
        pAdministratorsGroup = NULL;
    }

    return (bool)fIsRunAsAdmin;
}

//=============================================================================


void BackgroundWorker_Init(BackgroundWorker* worker, HWND hwnd, const HPATHL hFilePath)
{
    if (worker) {
        worker->hwnd = hwnd;
        // manual (not automatic) reset & initial state: not signaled (TRUE, FALSE)
        worker->eventCancel = CreateEvent(NULL, TRUE, FALSE, NULL);
        worker->workerThread = INVALID_HANDLE_VALUE;
        worker->hFilePath = Path_Allocate(Path_Get(hFilePath));
    }
}

void BackgroundWorker_Start(BackgroundWorker* worker, _beginthreadex_proc_type routine, LPVOID property)
{
    if (worker) {
        ResetEvent(worker->eventCancel); // init should be 'not signaled'
        //~worker->workerThread = CreateThread(NULL, 0, routine, property, 0, NULL);  // MD(d) dll
        uintptr_t const thread = _beginthreadex(NULL, 0, routine, property, 0, NULL); // MT(d) static
        InterlockedExchangePointer(&(worker->workerThread), (thread != 0LL) ? (HANDLE)thread : INVALID_HANDLE_VALUE);
    }
}

void BackgroundWorker_Cancel(BackgroundWorker* worker) {
    if (worker) {
        SetEvent(worker->eventCancel); // signal
        HANDLE const workerThread = InterlockedExchangePointer(&(worker->workerThread), INVALID_HANDLE_VALUE);
        if (IS_VALID_HANDLE(workerThread)) {
            // Optimize: MsgDispatch only in case of hwnd ?
            // DWORD const wait = SignalObjectAndWait(worker->eventCancel, workerThread, 100 /*INFINITE*/, FALSE);
            while (WaitForSingleObject(workerThread, 0) != WAIT_OBJECT_0) {
                MSG msg;
                if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
            CloseHandle(workerThread);
        }
    }
}

// inline void BackgroundWorker_End(BackgroundWorker* worker, unsigned int retcode);

void BackgroundWorker_Destroy(BackgroundWorker* worker)
{
    if (worker) {
        BackgroundWorker_Cancel(worker);
        CloseHandle(InterlockedExchangePointer(&(worker->eventCancel), INVALID_HANDLE_VALUE));
        Path_Release(worker->hFilePath);
        worker->hFilePath = NULL;
    }
}

// inline bool BackgroundWorker_Continue(BackgroundWorker* worker);

//=============================================================================
//
//  BitmapMergeAlpha()
//  Merge alpha channel into color channel
//
bool BitmapMergeAlpha(HBITMAP hbmp,COLORREF crDest)
{
    BITMAP bmp;
    if (GetObject(hbmp,sizeof(BITMAP),&bmp)) {
        if (bmp.bmBitsPixel == 32) {
            RGBQUAD *prgba = bmp.bmBits;
            if (prgba) {
                for (int y = 0; y < bmp.bmHeight; y++) {
                    for (int x = 0; x < bmp.bmWidth; x++) {
                        BYTE alpha = prgba[x].rgbReserved;
                        prgba[x].rgbRed = ((prgba[x].rgbRed * alpha) + (GetRValue(crDest) * (255 - alpha))) >> 8;
                        prgba[x].rgbGreen = ((prgba[x].rgbGreen * alpha) + (GetGValue(crDest) * (255 - alpha))) >> 8;
                        prgba[x].rgbBlue = ((prgba[x].rgbBlue * alpha) + (GetBValue(crDest) * (255 - alpha))) >> 8;
                        prgba[x].rgbReserved = 0xFF;
                    }
                    prgba = (RGBQUAD*)((LPBYTE)prgba + bmp.bmWidthBytes);
                }
                return true;
            }
        }
    }
    return false;
}


//=============================================================================
//
//  BitmapAlphaBlend()
//  Perform alpha blending to color channel only
//
bool BitmapAlphaBlend(HBITMAP hbmp,COLORREF crDest,BYTE alpha)
{
    BITMAP bmp;
    if (GetObject(hbmp,sizeof(BITMAP),&bmp)) {
        if (bmp.bmBitsPixel == 32) {
            RGBQUAD *prgba = bmp.bmBits;
            if (prgba) {
                for (int y = 0; y < bmp.bmHeight; y++) {
                    for (int x = 0; x < bmp.bmWidth; x++) {
                        prgba[x].rgbRed = ((prgba[x].rgbRed * alpha) + (GetRValue(crDest) * (255 - alpha))) >> 8;
                        prgba[x].rgbGreen = ((prgba[x].rgbGreen * alpha) + (GetGValue(crDest) * (255 - alpha))) >> 8;
                        prgba[x].rgbBlue = ((prgba[x].rgbBlue * alpha) + (GetBValue(crDest) * (255 - alpha))) >> 8;
                    }
                    prgba = (RGBQUAD*)((LPBYTE)prgba + bmp.bmWidthBytes);
                }
                return true;
            }
        }
    }
    return false;
}


//=============================================================================
//
//  BitmapGrayScale()
//  Gray scale color channel only
//
bool BitmapGrayScale(HBITMAP hbmp)
{
    BITMAP bmp;
    if (GetObject(hbmp,sizeof(BITMAP),&bmp)) {
        if (bmp.bmBitsPixel == 32) {
            RGBQUAD *prgba = bmp.bmBits;
            if (prgba) {
                for (int y = 0; y < bmp.bmHeight; y++) {
                    for (int x = 0; x < bmp.bmWidth; x++) {
                        prgba[x].rgbRed = prgba[x].rgbGreen = prgba[x].rgbBlue =
                                (((BYTE)((prgba[x].rgbRed * 38 + prgba[x].rgbGreen * 75 + prgba[x].rgbBlue * 15) >> 7) * 0x80) + (0xD0 * (255 - 0x80))) >> 8;
                    }
                    prgba = (RGBQUAD*)((LPBYTE)prgba + bmp.bmWidthBytes);
                }
                return true;
            }
        }
    }
    return false;
}


//=============================================================================
//
//  VerifyContrast()
//  Check if two colors can be distinguished
//
bool VerifyContrast(COLORREF cr1,COLORREF cr2)
{
    BYTE r1 = GetRValue(cr1);
    BYTE g1 = GetGValue(cr1);
    BYTE b1 = GetBValue(cr1);
    BYTE r2 = GetRValue(cr2);
    BYTE g2 = GetGValue(cr2);
    BYTE b2 = GetBValue(cr2);
    return (((abs((3*r1 + 5*g1 + 1*b1) - (3*r2 + 6*g2 + 1*b2))) >= 400) ||
            ((abs(r1-r2) + abs(b1-b2) + abs(g1-g2)) >= 400));
}


//=============================================================================
//
//  IsFontAvailable()
//  Test if a certain font is installed on the system
//
static int CALLBACK EnumFontFamExProcFound(CONST LOGFONT *plf, CONST TEXTMETRIC *ptm, DWORD FontType, LPARAM lParam) {

    UNREFERENCED_PARAMETER(plf);
    UNREFERENCED_PARAMETER(ptm);
    UNREFERENCED_PARAMETER(FontType);
    *((bool*)lParam) = true;
    return 0; // stop further enumerating
}

bool IsFontAvailable(LPCWSTR lpszFontName) {

    BOOL fFound = FALSE;
    if (!lpszFontName)
        return fFound;

    LOGFONT lf = { 0 };
    StringCchCopy(lf.lfFaceName, LF_FACESIZE, lpszFontName);
    lf.lfCharSet = DEFAULT_CHARSET;

    HDC hDC = GetDC(NULL);
    EnumFontFamiliesEx(hDC, &lf, EnumFontFamExProcFound, (LPARAM)&fFound, 0);
    ReleaseDC(NULL, hDC);

    return fFound;
}



//=============================================================================
//
//  IsCmdEnabled()
//
bool IsCmdEnabled(HWND hwnd, UINT uId)
{
    HMENU const hmenu = GetMenu(hwnd);
    SendMessage(hwnd, WM_INITMENU,(WPARAM)hmenu, 0);
    UINT const ustate = GetMenuState(hmenu, uId, MF_BYCOMMAND);
    return ((ustate == 0xFFFFFFFF) ? true : (!(ustate & (MF_GRAYED | MF_DISABLED))));
}


//=============================================================================
//
//  ReadFileXL()
//
bool ReadFileXL(HANDLE hFile, char* const lpBuffer, const size_t nNumberOfBytesToRead, size_t* const lpNumberOfBytesRead)
{
    DWORD dwRead = 0;
    size_t bytesRead = 0ULL;
    size_t bytesLeft = nNumberOfBytesToRead;
    bool bReadOk = false;
    do {
        DWORD const chunk_size = (bytesLeft < (size_t)DWORD_MAX) ? (DWORD)bytesLeft : DWORD_MAX - 1UL;
        bReadOk = ReadFile(hFile, &lpBuffer[bytesRead], chunk_size, &dwRead, NULL);
        bytesRead += (size_t)dwRead;
        bytesLeft -= (size_t)dwRead;
    } while (bReadOk && ((dwRead != 0) && (bytesLeft > 0)));

    *lpNumberOfBytesRead = bytesRead;
    return (bytesRead == nNumberOfBytesToRead);
}

//=============================================================================
//
//  WriteFileXL()
//
bool WriteFileXL(HANDLE hFile, const char* const lpBuffer, const size_t nNumberOfBytesToWrite, size_t* const lpNumberOfBytesWritten)
{
    DWORD dwWritten = 0;
    size_t bytesWritten = 0ULL;
    size_t bytesLeft = nNumberOfBytesToWrite;
    bool bWriteOk = false;
    do {
        DWORD const chunk_size = (bytesLeft < (size_t)DWORD_MAX) ? (DWORD)bytesLeft : DWORD_MAX - 1UL;

        bWriteOk = WriteFile(hFile, &lpBuffer[bytesWritten], chunk_size, &dwWritten, NULL);
        bytesWritten += (size_t)dwWritten;
        bytesLeft -= (size_t)dwWritten;
    } while (bWriteOk && ((dwWritten != 0) && (bytesLeft > 0)));

    FlushFileBuffers(hFile);

    *lpNumberOfBytesWritten = bytesWritten;
    return (bytesWritten == nNumberOfBytesToWrite);
}


//=============================================================================
//
//  StrLTrimI()
//
bool StrLTrimI(LPWSTR pszSource,LPCWSTR pszTrimChars)
{
    if (!pszSource || !*pszSource) {
        return false;
    }

    LPWSTR psz = pszSource;
    while (StrChrI(pszTrimChars, *psz)) {
        ++psz;
    }

    MoveMemory(pszSource, psz, sizeof(WCHAR)*(StringCchLen(psz,0) + 1));

    return (psz != pszSource);
}


//=============================================================================
//
//  StrRTrimI()
//
bool StrRTrimI(LPWSTR pszSource, LPCWSTR pszTrimChars)
{
    if (!pszSource || !*pszSource) {
        return false;
    }
    size_t const length = StringCchLen(pszSource, 0);

    size_t len = length;
    while ((len > 0) && StrChrI(pszTrimChars, pszSource[--len])) {
        pszSource[len] = L'\0';
    }

    return (length != len);
}


#if 0

//=============================================================================
//
//  TrimStringA()
//
bool TrimStringA(LPSTR lpString)
{
    if (!lpString || !*lpString) {
        return false;
    }

    // Trim left
    LPSTR psz = lpString;
    while (*psz == ' ') {
        psz = CharNextA(psz);
    }

    MoveMemory(lpString, psz, sizeof(CHAR)*(StringCchLenA(psz,0) + 1));

    // Trim right
    psz = StrEndA(lpString,0);
    while (*(psz = CharPrevA(lpString, psz)) == ' ') {
        *psz = '\0';
    }

    return true;
}



//=============================================================================
//
//  TrimStringW()
//
bool TrimStringW(LPWSTR lpString)
{
    if (!lpString || !*lpString) {
        return false;
    }

    // Trim left
    LPWSTR psz = lpString;
    while (*psz == L' ') {
        psz = CharNextW(psz);
    }

    MoveMemory(lpString,psz,sizeof(WCHAR)*(StringCchLenA(psz,0) + 1));

    // Trim right
    psz = StrEndW(lpString,0);
    while (*(psz = CharPrevW(lpString, psz)) == L' ') {
        *psz = L'\0';
    }

    return true;
}

#endif



//=============================================================================
//
//  ExtractFirstArgument()
//  Adapted from https://creativeandcritical.net/str-replace-c#repl_wcs
//
LPWSTR StrReplaceAll(LPCWSTR str, LPCWSTR from, LPCWSTR to)
{
    if (!str || !to || StrIsEmpty(from)) {
        return NULL;
    }
    /* Adjust each of the below values to suit your needs. */

    /* Increment positions cache size initially by this number. */
    size_t cache_sz_inc = 64;
    /* Thereafter, each time capacity needs to be increased,
	 * multiply the increment by this factor. */
    const size_t cache_sz_inc_factor = 2;
    /* But never increment capacity by more than this number. */
    const size_t cache_sz_inc_max = 1048576;

    LPWSTR ret = NULL;

    LPWSTR pret;
    LPCWSTR pstr2;
    LPCWSTR pstr = str;
    size_t i, count = 0;
    ptrdiff_t *pos_cache_tmp = NULL;
    size_t cache_sz = 0;
    size_t cpylen, orglen, retlen, tolen = 0, fromlen = wcslen(from);

    ptrdiff_t* pos_cache = AllocMem(cache_sz_inc, HEAP_ZERO_MEMORY); // init for ReAllocGrowMem()

    /* Find all matches and cache their positions. */
    while ((pstr2 = wcsstr(pstr, from)) != NULL) {
        count++;

        /* Increase the cache size when necessary. */
        if (cache_sz < count) {
            cache_sz += cache_sz_inc;
            pos_cache_tmp = ReAllocGrowMem(pos_cache, cache_sz, HEAP_ZERO_MEMORY);
            if (pos_cache_tmp == NULL) {
                goto end_repl_wcs;
            } else
                pos_cache = pos_cache_tmp;
            cache_sz_inc *= cache_sz_inc_factor;
            if (cache_sz_inc > cache_sz_inc_max) {
                cache_sz_inc = cache_sz_inc_max;
            }
        }

        pos_cache[count - 1] = pstr2 - str;
        pstr = pstr2 + fromlen;
    }

    orglen = pstr - str + wcslen(pstr);

    /* Allocate memory for the post-replacement string. */
    if (count > 0) {
        tolen = wcslen(to);
        retlen = orglen + (tolen - fromlen) * count;
    } else
        retlen = orglen;
    const size_t retsize = (retlen + 1) * sizeof(WCHAR);
    ret = AllocMem(retsize, HEAP_ZERO_MEMORY);
    if (ret == NULL) {
        goto end_repl_wcs;
    }

    if (count == 0) {
        /* If no matches, then just duplicate the string. */
        wcscpy_s(ret, retsize, str);
    } else {
        /* Otherwise, duplicate the string whilst performing
		 * the replacements using the position cache. */
        pret = ret;
        wmemcpy(pret, str, pos_cache[0]);
        pret += pos_cache[0];
        for (i = 0; i < count; i++) {
            wmemcpy(pret, to, tolen);
            pret += tolen;
            pstr = str + pos_cache[i] + fromlen;
            cpylen = (i == count - 1 ? orglen : pos_cache[i + 1]) - pos_cache[i] - fromlen;
            wmemcpy(pret, pstr, cpylen);
            pret += cpylen;
        }
        ret[retlen] = L'\0';
    }

end_repl_wcs:
    /* Free the cache and return the post-replacement string,
	 * which will be NULL in the event of an error. */
    FreeMem(pos_cache);
    return ret;
}


//=============================================================================
//
//  ExtractFirstArgument()
//
bool ExtractFirstArgument(LPCWSTR lpArgs, LPWSTR lpArg1, LPWSTR lpArg2, int len)
{
    StringCchCopy(lpArg1, len, lpArgs);

    if (StrIsEmpty(lpArg1)) {
        return false;
    }
    if (lpArg2) {
        *lpArg2 = L'\0';
    }

    TrimSpcW(lpArg1);

    bool bQuoted = false;
    if (*lpArg1 == L'\"') {
        *lpArg1 = L' ';
        TrimSpcW(lpArg1);
        bQuoted = true;
    }

    LPWSTR psz;
    if (bQuoted) {
        psz = StrChr(lpArg1, L'\"');
        // skip esc'd quotes
        while (psz && (psz[-1] == L'\\')) {
            psz = StrChr(psz + 1, L'\"');
        }
    } else {
        psz = StrChr(lpArg1, L' ');
    }
    if (psz) {
        *psz = L'\0';
        if (lpArg2) {
            StringCchCopy(lpArg2, len, psz + 1);
        }
    }
    TrimSpcW(lpArg1);
    UnSlashCharW(lpArg1, L'"');

    if (lpArg2) {
        TrimSpcW(lpArg2);
    }
    return true;
}


//=============================================================================
//
//  PrepareFilterStr()
//
void PrepareFilterStr(LPWSTR lpFilter)
{
    LPWSTR psz = StrEnd(lpFilter,0);
    while (psz != lpFilter) {
        if (*(psz = CharPrev(lpFilter, psz)) == L'|') {
            *psz = L'\0';
        }
    }
}


//=============================================================================
//
//  StrTab2Space() - in place conversion
//
void StrTab2Space(LPWSTR lpsz)
{
    WCHAR *c = StrChr(lpsz, L'\t');
    while (c) {
        *c = L' ';
        c = StrChr(lpsz, L'\t');  // next
    }
}


//=============================================================================
//
//  PathFixBackslashes() - in place conversion
//
void PathFixBackslashes(LPWSTR lpsz)
{
    WCHAR *c = StrChr(lpsz, L'/');
    while (c) {
        if ((*CharPrev(lpsz,c) == L':') && (*CharNext(c) == L'/')) {
            c += 2;
        } else {
            *c = L'\\';
        }
        c = StrChr(c, L'/');  // next
    }
}


//=============================================================================
//
//  SplitFilePathLineNum()
//
bool SplitFilePathLineNum(LPWSTR lpszPath, int* lineNum)
{

    LPWSTR const lpszSplit = StrRChr(lpszPath, NULL, L':');

    bool res = false;
    if (lpszSplit) {
        char       chLnNumber[128];
        char const defchar = (char)0x24;
        WideCharToMultiByte(CP_ACP, (WC_COMPOSITECHECK | WC_DISCARDNS), &lpszSplit[1], -1, chLnNumber, COUNTOF(chLnNumber), &defchar, NULL);
        te_int_t iExprError = true;
        int const ln = (int)te_interp(chLnNumber, &iExprError);
        if (!iExprError) {
            res = true;
            lpszSplit[0] = L'\0'; // split
            if (lineNum) {
                *lineNum = ln;
            }
        }
    }
    return res;
}


//=============================================================================
//
//  FormatNumberStr()
//
size_t FormatNumberStr(LPWSTR lpNumberStr, size_t cch, int fixedWidth)
{
    static WCHAR szSep[SMALL_BUFFER] = { L'\0' };
    static WCHAR szGrp[SMALL_BUFFER] = { L'\0' };
    static int iPlace[4] = {-1,-1,-1,-1};

    if (StrIsEmpty(lpNumberStr)) {
        return 0;
    }

    StrTrim(lpNumberStr, L" \t");

    if (szSep[0] == L'\0') {
        if (!GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_STHOUSAND, szSep, COUNTOF(szSep))) {
            szSep[0] = L'\'';
        }
    }

    if (szGrp[0] == L'\0') {
        if (!GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SGROUPING, szGrp, COUNTOF(szGrp))) {
            szGrp[0] = L'0';
        }
        if (szGrp[0] == L'\0') {
            szGrp[0] = L'0';
        }
        swscanf_s(szGrp, L"%i;%i;%i;%i", &iPlace[0], &iPlace[1], &iPlace[2], &iPlace[3]);
    }
    if (iPlace[0] <= 0) {
        return StringCchLen(lpNumberStr,0);
    }

    if ((int)StringCchLen(lpNumberStr,0) > iPlace[0]) {

        WCHAR* ch = StrEnd(lpNumberStr,0);

        int  iCnt = 0;
        int  i = 0;
        while ((ch = CharPrev(lpNumberStr, ch)) != lpNumberStr) {
            if (((++iCnt) % iPlace[i]) == 0) {
                MoveMemory(ch + 1, ch, sizeof(WCHAR)*(StringCchLen(ch,0) + 1));
                *ch = szSep[0];
                i = (i < 3) ? (i + 1) : 3;
                if (iPlace[i] == 0) {
                    --i;
                } else if (iPlace[i] < 0) {
                    break;
                }
                iCnt = 0;
            }
        }
    }

    if (fixedWidth > 0) {
        static WCHAR szCrop[256] = { L'\0' };
        StringCchPrintf(szCrop, COUNTOF(szCrop), L"%*s", fixedWidth, lpNumberStr);
        StringCchCopy(lpNumberStr, cch, szCrop);
    }

    return StringCchLen(lpNumberStr,0);
}


//=============================================================================
//
//  SetDlgItemIntEx()
//
bool SetDlgItemIntEx(HWND hwnd,int nIdItem,UINT uValue)
{
    WCHAR szBuf[64] = { L'\0' };
    StringCchPrintf(szBuf,COUNTOF(szBuf),L"%u",uValue);
    FormatNumberStr(szBuf, COUNTOF(szBuf), 0);
    return(SetDlgItemText(hwnd,nIdItem,szBuf));
}


//=============================================================================
//
//  CodePageFromCharSet()
//
UINT CodePageFromCharSet(const UINT uCharSet)
{
    if (ANSI_CHARSET == uCharSet) {
        CPINFOEX cpinfo = { 0 };
        if (GetCPInfoEx(CP_THREAD_ACP, 0, &cpinfo)) {
            return cpinfo.CodePage;
        }
    } else {
        CHARSETINFO ci;
        ZeroMemory(&ci, sizeof(CHARSETINFO));
        if (TranslateCharsetInfo((DWORD*)(UINT_PTR)uCharSet, &ci, TCI_SRCCHARSET)) {
            return(ci.ciACP);
        }
    }
    return GetACP(); // fallback: systems locale ANSI CP
}

#if 0
//=============================================================================
//
//  CharSetFromCodePage()
//
UINT CharSetFromCodePage(const UINT uCodePage)
{
    CHARSETINFO ci;
    if (TranslateCharsetInfo((DWORD*)(UINT_PTR)uCodePage, &ci, TCI_SRCCODEPAGE)) {
        return(ci.ciCharset);  // corresponds to SCI: SC_CHARSET_XXX
    }
    return(ANSI_CHARSET);
}
#endif

/**
 * Convert C style \0oo into their indicated characters.
 * This is used to get control characters into the regular expresion engine
 * w/o interfering with group referencing ('\0').
 */
ptrdiff_t UnSlashLowOctalA(LPSTR s)
{
    LPSTR sStart = s;
    LPSTR o = s;
    while (*s) {
        if ((s[0] == '\\') && (s[1] == '\\')) { // esc seq
            *o = *s;
            ++o;
            ++s;
            *o = *s;
        } else if ((s[0] == '\\') && (s[1] == '0') && IsOctalDigitA(s[2]) && IsOctalDigitA(s[3])) {
            *o = (CHAR)(8 * (s[2] - '0') + (s[3] - '0'));
            s += 3;
        } else {
            *o = *s;
        }
        ++o;
        if (*s) {
            ++s;
        }
    }
    *o = '\0';
    return (ptrdiff_t)(o - sStart);
}


ptrdiff_t UnSlashLowOctalW(LPWSTR s)
{
    LPWSTR sStart = s;
    LPWSTR o = s;
    while (*s) {
        if ((s[0] == L'\\') && (s[1] == L'\\')) { // esc seq
            *o = *s;
            ++o;
            ++s;
            *o = *s;
        } else if ((s[0] == L'\\') && (s[1] == L'0') && IsOctalDigitW(s[2]) && IsOctalDigitW(s[3])) {
            *o = (WCHAR)(8 * (s[2] - L'0') + (s[3] - L'0'));
            s += 3;
        } else {
            *o = *s;
        }
        ++o;
        if (*s) {
            ++s;
        }
    }
    *o = L'\0';
    return (ptrdiff_t)(o - sStart);
}


/** ******************************************************************************
 *
 *  UnSlash functions
 *  Mostly taken from SciTE, (c) Neil Hodgson, http://www.scintilla.org
 *
 * Convert C style \a, \b, \f, \n, \r, \t, \v, \xhh, \uhhhh and \\  into their indicated characters.
 */
size_t UnSlashA(LPSTR pchInOut, UINT cpEdit)
{
    LPSTR s = pchInOut;
    LPSTR o = pchInOut;
    LPCSTR const sStart = pchInOut;

    while (*s) {
        if (*s == '\\') {
            ++s;
            if (*s == 'a') {
                *o = '\a';
            } else if (*s == 'b') {
                *o = '\b';
            } else if (*s == 'e') {
                *o = '\x1B';
            } else if (*s == 'f') {
                *o = '\f';
            } else if (*s == 'n') {
                *o = '\n';
            } else if (*s == 'r') {
                *o = '\r';
            } else if (*s == 't') {
                *o = '\t';
            } else if (*s == 'v') {
                *o = '\v';
            } else if (*s == '"') {
                *o = '"';
            } else if (*s == '\\') {
                *o = '\\';
            } else if (*s == 'x' || *s == 'u') {
                bool bShort = (*s == 'x');
                char ch[8];
                char* pch = ch;
                WCHAR val[2] = L"";
                int hex;
                val[0] = 0;
                hex = GetHexDigitA(*(s + 1));
                if (hex >= 0) {
                    ++s;
                    val[0] = (WCHAR)hex;
                    hex = GetHexDigitA(*(s + 1));
                    if (hex >= 0) {
                        ++s;
                        val[0] *= 16;
                        val[0] += (WCHAR)hex;
                        if (!bShort) {
                            hex = GetHexDigitA(*(s + 1));
                            if (hex >= 0) {
                                ++s;
                                val[0] *= 16;
                                val[0] += (WCHAR)hex;
                                hex = GetHexDigitA(*(s + 1));
                                if (hex >= 0) {
                                    ++s;
                                    val[0] *= 16;
                                    val[0] += (WCHAR)hex;
                                }
                            }
                        }
                    }
                    if (val[0]) {
                        val[1] = 0;
                        WideCharToMultiByte(cpEdit, 0, val, -1, ch, (int)COUNTOF(ch), NULL, NULL);
                        *o = *pch++;
                        while (*pch) {
                            *++o = *pch++;
                        }
                    } else {
                        --o;
                    }
                } else {
                    --o;
                }
            } else {
                //~*o = '\\';  *++o = *s;   // revert
                *o = *s;   // swallow single '\'
            }
        } else {
            *o = *s;
        }

        ++o;
        if (*s) {
            ++s;
        }
    }
    *o = '\0';
    return (size_t)((ptrdiff_t)(o - sStart));
}


size_t UnSlashW(LPWSTR pchInOut, UINT cpEdit)
{
    LPWSTR s = pchInOut;
    LPWSTR o = pchInOut;
    LPCWSTR const sStart = pchInOut;

    while (*s) {
        if (*s == L'\\') {
            ++s;
            if (*s == L'a') {
                *o = L'\a';
            } else if (*s == L'b') {
                *o = L'\b';
            } else if (*s == L'e') {
                *o = L'\x1B';
            } else if (*s == L'f') {
                *o = L'\f';
            } else if (*s == L'n') {
                *o = L'\n';
            } else if (*s == L'r') {
                *o = L'\r';
            } else if (*s == L't') {
                *o = L'\t';
            } else if (*s == L'v') {
                *o = L'\v';
            } else if (*s == L'"') {
                *o = L'"';
            } else if (*s == L'\\') {
                *o = L'\\';
            } else if (*s == L'x' || *s == L'u') {
                bool bShort = (*s == L'x');
                char ch[8];
                char* pch = ch;
                WCHAR val[2] = { L'\0' };
                int hex;
                val[0] = L'\0';
                hex = GetHexDigitW(*(s + 1));
                if (hex >= 0) {
                    ++s;
                    val[0] = (WCHAR)hex;
                    hex = GetHexDigitW(*(s + 1));
                    if (hex >= 0) {
                        ++s;
                        val[0] *= 16;
                        val[0] += (WCHAR)hex;
                        if (!bShort) {
                            hex = GetHexDigitW(*(s + 1));
                            if (hex >= 0) {
                                ++s;
                                val[0] *= 16;
                                val[0] += (WCHAR)hex;
                                hex = GetHexDigitW(*(s + 1));
                                if (hex >= 0) {
                                    ++s;
                                    val[0] *= 16;
                                    val[0] += (WCHAR)hex;
                                }
                            }
                        }
                    }
                    if (val[0]) {
                        val[1] = L'\0';
                        WideCharToMultiByte(cpEdit, 0, val, -1, ch, (int)COUNTOF(ch), NULL, NULL);
                        *o = *pch++;
                        while (*pch) {
                            *++o = (WCHAR)*pch++;
                        }
                    } else {
                        --o;
                    }
                } else {
                    --o;
                }
            } else {
                //~*o = L'\\';  *++o = *s;   // revert
                *o = *s;   // swallow single L'\'
            }
        } else {
            *o = *s;
        }

        ++o;
        if (*s) {
            ++s;
        }
    }
    *o = L'\0';
    return (size_t)((ptrdiff_t)(o - sStart));
}


//=============================================================================

size_t SlashCtrlW(LPWSTR pchOutput, size_t cchOutLen, LPCWSTR pchInput)
{
    if (!pchOutput || cchOutLen < 2 || !pchInput) {
        return 0;
    }

    size_t i = 0;
    size_t k = 0;
    size_t const maxcnt = cchOutLen - 2;
    while ((pchInput[k] != L'\0') && (i < maxcnt)) {
        switch (pchInput[k]) {
        //~case L'\\':
        //~    pchOutput[i++] = L'\\';
        //~    pchOutput[i++] = L'\\';
        //~    break;
        case L'\n':
            pchOutput[i++] = L'\\';
            pchOutput[i++] = L'n';
            break;
        case L'\r':
            pchOutput[i++] = L'\\';
            pchOutput[i++] = L'r';
            break;
        case L'\t':
            pchOutput[i++] = L'\\';
            pchOutput[i++] = L't';
            break;
        case L'\f':
            pchOutput[i++] = L'\\';
            pchOutput[i++] = L'f';
            break;
        case L'\v':
            pchOutput[i++] = L'\\';
            pchOutput[i++] = L'v';
            break;
        case L'\a':
            pchOutput[i++] = L'\\';
            pchOutput[i++] = L'a';
            break;
        case L'\b':
            pchOutput[i++] = L'\\';
            pchOutput[i++] = L'b';
            break;
        case L'\x1B':
            pchOutput[i++] = L'\\';
            pchOutput[i++] = L'e';
            break;
        default:
            pchOutput[i++] = pchInput[k];
            break;
        }
        ++k;
    }
    pchOutput[i] = pchInput[k];
    // ensure string end
    if (pchInput[k] != L'\0') {
        pchOutput[++i] = L'\0';
    }
    return i;
}


size_t UnSlashCtrlW(LPWSTR pchInOut)
{
    LPWSTR s = pchInOut;
    LPWSTR o = pchInOut;
    LPCWSTR const sStart = pchInOut;

    while (*s) {
        if (*s == L'\\') {
            ++s;
            //~if (*s == L'\\') {
            //~    *o = L'\\';
            //~} else 
            if (*s == L'n') {
                *o = L'\n';
            } else if (*s == L'r') {
                *o = L'\r';
            } else if (*s == L't') {
                *o = L'\t';
            } else if (*s == L'f') {
                *o = L'\f';
            } else if (*s == L'v') {
                *o = L'\v';
            } else if (*s == L'a') {
                *o = L'\a';
            } else if (*s == L'b') {
                *o = L'\b';
            } else if (*s == L'e') {
                *o = L'\x1B';
            } else {
                *o = *s;    // swallow single L'\'
            }
        } else {
            *o = *s;
        }
        ++o;
        if (*s) {
            ++s;
        }
    }
    *o = L'\0';
    return (size_t)((ptrdiff_t)(o - sStart));
}
//=============================================================================


size_t UnSlashCharW(LPWSTR pchInOut, WCHAR wch)
{
    LPCWSTR const sStart = pchInOut;

    LPWSTR s = pchInOut;
    LPWSTR o = pchInOut;
    while (*s) {
        if (*s == L'\\') {
            ++s;
            if (*s == wch) {
                *o++ = wch;
            } else {
                *o++ = L'\\'; // restore
                *o++ = *s;
            }
        } else {
            *o++ = *s;
        }
        if (*s) {
            ++s;
        }
    }
    *o = L'\0';
    return (size_t)((ptrdiff_t)(o - sStart));
}


/**
 *  check, if we have regex sub-group referencing
 */
static int _CheckRegExReplTargetA(LPSTR pszInput)
{
    while (*pszInput) {
        if (*pszInput == '$') {
            ++pszInput;
            if (((*pszInput >= '0') && (*pszInput <= '9')) || (*pszInput == '+') || (*pszInput == '{')) {
                return SCI_REPLACETARGETRE;
            }
        } else if (*pszInput == '\\') {
            ++pszInput;
            if ((*pszInput >= '0') && (*pszInput <= '9')) {
                return SCI_REPLACETARGETRE;
            }
        } else {
            ++pszInput;
        }
    }
    return SciCall_GetChangeHistory() ? SCI_REPLACETARGETMINIMAL : SCI_REPLACETARGET;
}

static int _CheckRegExReplTargetW(LPWSTR pszInput)
{
    while (*pszInput) {
        if (*pszInput == L'$') {
            ++pszInput;
            if (((*pszInput >= L'0') && (*pszInput <= L'9')) || (*pszInput == L'+') || (*pszInput == L'{')) {
                return SCI_REPLACETARGETRE;
            }
        } else if (*pszInput == L'\\') {
            ++pszInput;
            if ((*pszInput >= L'0') && (*pszInput <= L'9')) {
                return SCI_REPLACETARGETRE;
            }
        } else {
            ++pszInput;
        }
    }
    return SciCall_GetChangeHistory() ? SCI_REPLACETARGETMINIMAL : SCI_REPLACETARGET;
}


void TransformBackslashesA(LPSTR pszInput, bool bRegEx, UINT cpEdit, int* iReplaceMsg)
{
    if (iReplaceMsg) {
        if (bRegEx) {
            UnSlashLowOctalA(pszInput);
            *iReplaceMsg = _CheckRegExReplTargetA(pszInput);
        } else {
            *iReplaceMsg = SciCall_GetChangeHistory() ? SCI_REPLACETARGETMINIMAL : SCI_REPLACETARGET;
        }
    }
    // regex handles backslashes itself
    bool const bStdReplace = (iReplaceMsg && (SCI_REPLACETARGETRE != *iReplaceMsg));
    if (!bRegEx || bStdReplace) {
        UnSlashA(pszInput, cpEdit);
    }
}

void TransformBackslashesW(LPWSTR pszInput, bool bRegEx, UINT cpEdit, int* iReplaceMsg)
{
    if (iReplaceMsg) {
        if (bRegEx) {
            UnSlashLowOctalW(pszInput);
            *iReplaceMsg = _CheckRegExReplTargetW(pszInput);
        } else {
            *iReplaceMsg = SciCall_GetChangeHistory() ? SCI_REPLACETARGETMINIMAL : SCI_REPLACETARGET;
        }
    }
    // regex handles backslashes itself
    bool const bStdReplace = (iReplaceMsg && (SCI_REPLACETARGETRE != *iReplaceMsg));
    if (!bRegEx || bStdReplace) {
        UnSlashW(pszInput, cpEdit);
    }
}


#if 0
void TransformMetaChars(char* pszInput, size_t cch, bool bRegEx, int iEOLMode)
{
    if (!bRegEx) {
        return;
    }
    char* const buffer = AllocMem((cch << 1) * sizeof(char), HEAP_ZERO_MEMORY);
    if (buffer) {
        char* s = pszInput;
        char* o = buffer;
        while (*s) {
            if ((s[0] != '\\') && (s[1] == '$')) {
                *o++ = *s++;
                switch (iEOLMode) {
                case SC_EOL_LF:
                    *o++ = '\n';
                    break;
                case SC_EOL_CR:
                    *o++ = '\r';
                    break;
                case SC_EOL_CRLF:
                default:
                    *o++ = '\r';
                    *o++ = '\n';
                    break;
                }
                ++s; // skip $
            }
            else {
                *o++ = *s;
            }
            if (*s) {
                ++s;
            }
        }
        *o = '\0';
        StringCchCopyA(pszInput, cch, buffer);
        FreeMem(buffer);
    }
}
#endif

//=============================================================================
//
//  Hex2Char() - zero('\0') terminated strings
//  by Zufuliu
//
int Hex2Char(char* ch, int cnt)
{
    int cch = 0;
    WCHAR* const wch = AllocMem(cnt * sizeof(WCHAR), HEAP_ZERO_MEMORY);
    if (wch) {
        int ci  = 0;
        char* p = ch;
        while (*p) {
            if (*p == '\\') {
                p++;
                if ((*p == 'x' || *p == 'u') ||
                        (*p == 'X' || *p == 'U')) {
                    p++;
                    ci      = 0;
                    int ucc = 0;
                    while (*p && (ucc++ < MAX_ESCAPE_HEX_DIGIT)) {
                        if (*p >= '0' && *p <= '9') {
                            ci = ci * 16 + (*p++ - '0');
                        } else if (*p >= 'a' && *p <= 'f') {
                            ci = ci * 16 + (*p++ - 'a') + 10;
                        } else if (*p >= 'A' && *p <= 'F') {
                            ci = ci * 16 + (*p++ - 'A') + 10;
                        } else {
                            break;
                        }
                    }
                } else {
                    ci = *p++;
                }
            } else {
                ci = *p++;
            }
            wch[cch++] = (WCHAR)ci;
            if (ci == 0) {
                break;
            }
        }
        wch[cch] = L'\0';

        cch = WideCharToMultiByte(Encoding_SciCP, 0, wch, -1, ch, cnt, NULL, NULL) - 1; // '\0'

        FreeMem(wch);
    }
    return cch;
}


#ifdef WC2MB_EX

TODO:  how to avoid Code-Point truncation/splitting

//=============================================================================
//
//  WideCharToMultiByteEx()
//
ptrdiff_t WideCharToMultiByteEx(
    UINT CodePage, DWORD dwFlags, LPCWCH lpWideCharStr, ptrdiff_t cchWideChar,
    LPSTR lpMultiByteStr, ptrdiff_t cbMultiByte, LPCCH lpDefaultChar, LPBOOL lpUsedDefaultChar)
{
    LPCWCH inPtr = lpWideCharStr;
    ptrdiff_t inBufCnt = cchWideChar;
    LPSTR outPtr = (cbMultiByte == 0LL) ? NULL : lpMultiByteStr;
    ptrdiff_t outBufSiz = cbMultiByte;
    ptrdiff_t bytesConv = 0LL;

    static ptrdiff_t const maxBufSize = (INT_MAX - 1);

    BOOL bIsDefCharUse = FALSE;

    while ((inBufCnt > 0LL) || (inBufCnt == -1LL)) {
        int const cnt = (inBufCnt > maxBufSize) ? (int)maxBufSize : ((inBufCnt > 0LL) ? (int)inBufCnt : -1);
        int const siz = (outBufSiz > (ptrdiff_t)INT_MAX) ? INT_MAX : (int)outBufSiz;

        int const bytes = WideCharToMultiByte(CodePage, dwFlags, inPtr, cnt, outPtr, siz, lpDefaultChar, lpUsedDefaultChar);
        if (bytes <= 0) {
            break;
        }

        if (lpUsedDefaultChar && *lpUsedDefaultChar) {
            bIsDefCharUse = TRUE;
        }

        int const usedWChr = (inBufCnt > maxBufSize) ? (outPtr ? MultiByteToWideChar(CodePage, dwFlags, outPtr, bytes, NULL, 0) : 0) : (int)inBufCnt;

        bytesConv += (ptrdiff_t)bytes;
        if (outPtr) {
            outPtr += (ptrdiff_t)bytes;
            outBufSiz -= (ptrdiff_t)bytes;
        }
        if (inBufCnt > 0LL) {
            inBufCnt -= (ptrdiff_t)usedWChr;
        }
        inPtr += (ptrdiff_t)usedWChr;
    }

    if (lpUsedDefaultChar) {
        *lpUsedDefaultChar = bIsDefCharUse;
    }
    return bytesConv;
}
#endif

#ifdef MB2WC_EX

TODO:  how to avoid Code-Point truncation/splitting

//=============================================================================
//
//  MultiByteToWideCharEx()
//
ptrdiff_t MultiByteToWideCharEx(
    UINT CodePage, DWORD dwFlags, LPCCH lpMultiByteStr, ptrdiff_t cbMultiByte,
    LPWSTR lpWideCharStr, ptrdiff_t cchWideChar)
{
    static CHAR const strgEnd = '\0'; // stop reading

    LPCCH inPtr = lpMultiByteStr ? lpMultiByteStr : &strgEnd;
    ptrdiff_t inBufSiz = (cbMultiByte >= 0LL) ? cbMultiByte : -1LL;

    LPWSTR outPtr = (cchWideChar == 0LL) ? NULL : lpWideCharStr;
    ptrdiff_t outBufCnt = cchWideChar;
    ptrdiff_t wcharConv = 0LL;

    static ptrdiff_t const maxBufSize = (INT_MAX - 1);

    while ((inBufSiz > 0LL) || (inPtr && (*inPtr != '\0'))) {
        int const siz = (inBufSiz > maxBufSize) ? (int)maxBufSize : ((inBufSiz > 0LL) ? (int)inBufSiz : -1);
        int const cnt = (outBufCnt > (ptrdiff_t)INT_MAX) ? INT_MAX : (int)outBufCnt;

        int const wchars = MultiByteToWideChar(CodePage, dwFlags, inPtr, siz, outPtr, cnt);
        if (wchars <= 0) {
            break;
        }

        int const usedMBC = (inBufSiz > maxBufSize) ? (outPtr ? WideCharToMultiByte(CodePage, dwFlags, outPtr, wchars, NULL, 0, NULL, NULL) : 0) : (int)inBufSiz;

        wcharConv += (ptrdiff_t)wchars;
        if (outPtr) {
            outPtr += (ptrdiff_t)wchars;
            outBufCnt -= (ptrdiff_t)wchars;
        }
        if (inBufSiz > 0LL) {
            inBufSiz -= (ptrdiff_t)usedMBC;
            if (inBufSiz <= 0LL) {
                inPtr = &strgEnd;
            }
        } else {
            inPtr += (ptrdiff_t)usedMBC;
        }
    }
    return wcharConv;
}
#endif


//=============================================================================
//
//  UrlEscapeEx()
//

#if (NTDDI_VERSION < NTDDI_WIN8)

// Convert a byte into Hexadecimal Unicode character
__inline int toHEX(BYTE val, WCHAR* pOutChr)
{
    StringCchPrintfW(pOutChr, 4, L"%%%0.2X", val);
    return 3; // num of wchars ('%FF')
}

LPCTSTR const lpszUnreservedChars = L"-_.~"; // or IsAlphaNumeric()
LPCTSTR const lpszReservedChars = L"!#$%&'()*+,/:;=?@[]";
LPCTSTR const lpszUnsafeChars = L" \"\\<>{|}^`";

#endif

// ----------------------------------------------------------------------------

void UrlEscapeEx(LPCWSTR lpURL, LPWSTR lpEscaped, DWORD* pcchEscaped, bool bEscReserved)
{
#if (NTDDI_VERSION >= NTDDI_WIN8)
    UNREFERENCED_PARAMETER(bEscReserved);
    UrlEscape(lpURL, lpEscaped, pcchEscaped, (URL_ESCAPE_SEGMENT_ONLY | URL_ESCAPE_URI_COMPONENT));
#else
    //UrlEscape(lpURL, lpEscaped, pcchEscaped, (URL_ESCAPE_SEGMENT_ONLY | URL_ESCAPE_PERCENT | URL_ESCAPE_AS_UTF8));

    DWORD posIn = 0;
    DWORD posOut = 0;

    while (lpURL[posIn] && (posOut < *pcchEscaped)) {
        if (IsAlphaNumericW(lpURL[posIn]) || StrChrW(lpszUnreservedChars, lpURL[posIn])) {
            lpEscaped[posOut++] = lpURL[posIn++];
        } else if (StrChrW(lpszReservedChars, lpURL[posIn])) {
            if (posOut < (*pcchEscaped - 3)) {
                if (bEscReserved) {
                    posOut += toHEX(toascii(lpURL[posIn++]), &lpEscaped[posOut]);
                } else {
                    lpEscaped[posOut++] = lpURL[posIn++];
                }
            }
        } else if (StrChrW(lpszUnsafeChars, lpURL[posIn])) {
            if (posOut < (*pcchEscaped - 3)) {
                posOut += toHEX(toascii(lpURL[posIn++]), &lpEscaped[posOut]);
            }
        }
        // Encode unprintable characters 0x00-0x1F, and 0x7F
        else if ((lpURL[posIn] <= 0x1F) || (lpURL[posIn] == 0x7F)) {
            if (posOut < (*pcchEscaped - 3)) {
                posOut += toHEX((BYTE)lpURL[posIn++], &lpEscaped[posOut]);
            }
        }
        // Now encode all other unsafe characters
        else {
            CHAR mb[4] = { '\0', '\0', '\0', '\0' };
            int const n = WideCharToMultiByte(Encoding_SciCP, 0, &lpURL[posIn++], 1, mb, 4, 0, 0);
            if (posOut < (*pcchEscaped - (n*3))) {
                for (int i = 0; i < n; ++i) {
                    posOut += toHEX((BYTE)mb[i], &lpEscaped[posOut]);
                }
            }
        }
    }
    lpEscaped[posOut] = L'\0';
    *pcchEscaped = posOut;
#endif
}


//=============================================================================
//
//  UrlUnescapeEx()
//
void UrlUnescapeEx(LPWSTR lpURL, LPWSTR lpUnescaped, DWORD* pcchUnescaped)
{
#if (NTDDI_VERSION >= NTDDI_WIN8)
    UrlUnescape(lpURL, lpUnescaped, pcchUnescaped, URL_UNESCAPE_AS_UTF8);
#else
    char * const outBuffer = AllocMem((size_t)*pcchUnescaped + 1, HEAP_ZERO_MEMORY);
    if (!outBuffer) {
        return;
    }
    DWORD const outLen = *pcchUnescaped;

    DWORD posIn = 0;
    WCHAR buf[5] = { L'\0' };
    DWORD lastEsc = (DWORD)StringCchLen(lpURL,0) - 2;
    unsigned int code;

    DWORD posOut = 0;
    while ((posIn < lastEsc) && (posOut < outLen)) {
        bool bOk = false;
        // URL encoded
        if (lpURL[posIn] == L'%') {
            buf[0] = lpURL[posIn + 1];
            buf[1] = lpURL[posIn + 2];
            buf[2] = L'\0';
            if (swscanf_s(buf, L"%x", &code) == 1) {
                outBuffer[posOut++] = (char)code;
                posIn += 3;
                bOk = true;
            }
        }
        // HTML encoded
        else if ((lpURL[posIn] == L'&') && (lpURL[posIn + 1] == L'#')) {
            int n = 0;
            while (IsDigitW(lpURL[posIn + 2 + n]) && (n < 4)) {
                buf[n] = lpURL[posIn + 2 + n];
                ++n;
            }
            buf[n] = L'\0';
            if (swscanf_s(buf, L"%ui", &code) == 1) {
                if (code <= 0xFF) {
                    outBuffer[posOut++] = (char)code;
                    posIn += (2 + n);
                    if (lpURL[posIn] == L';') {
                        ++posIn;
                    }
                    bOk = true;
                }
            }
        }

        if (!bOk) {
            posOut += WideCharToMultiByte(Encoding_SciCP, 0, &lpURL[posIn++], 1,
                                          &outBuffer[posOut], (int)(outLen - posOut), NULL, NULL);
        }
    }

    // copy rest
    while ((lpURL[posIn] != L'\0') && (posOut < outLen)) {
        posOut += WideCharToMultiByte(Encoding_SciCP, 0, &lpURL[posIn++], 1,
                                      &outBuffer[posOut], (int)(outLen - posOut), NULL, NULL);
    }
    outBuffer[posOut] = '\0';

    DWORD const iOut = MultiByteToWideChar(Encoding_SciCP, 0, outBuffer, -1, lpUnescaped, (int)*pcchUnescaped);
    FreeMem(outBuffer);

    *pcchUnescaped = ((iOut > 0) ? (iOut - 1) : 0);
#endif
}


//=============================================================================
//
//  ReadStrgsFromCSV()
//
//
int ReadStrgsFromCSV(LPCWSTR wchCSVStrg, prefix_t sMatrix[], int iCount, int iLen, LPCWSTR sDefault)
{
    static WCHAR wchTmpBuff[MIDSZ_BUFFER];

    StringCchCopyW(wchTmpBuff, COUNTOF(wchTmpBuff), wchCSVStrg);
    TrimSpcW(wchTmpBuff);
    // fill default
    for (int i = 0; i < iCount; ++i) {
        if (sDefault && *sDefault) {
            StringCchCopyW(sMatrix[i], (size_t)iLen, sDefault);
        } else {
            sMatrix[i][0] = L'\0';
        }
    }
    // insert values
    int n = 0;
    WCHAR* p = wchTmpBuff;
    while (p && *p) {
        WCHAR* q = StrStrW(p, L",");
        if (q > p) {
            *q = L'\0';
        }
        if (n < iCount) {
            if (*p != L',') {
                StringCchCopyW(sMatrix[n], (size_t)iLen, p);
            } else {
                sMatrix[n][0] = L'\0';
            }
        }
        p = (q > p) ? (q + 1) : (p + 1);
        ++n;
    }
    return n;
}


//=============================================================================
//
//  ReadVectorFromString()
//
//
static int _cmpifunc(const void* a, const void* b)
{
    return (*(int*)a - *(int*)b);
}

size_t ReadVectorFromString(LPCWSTR wchStrg, int iVector[], size_t iCount, int iMin, int iMax, int iDefault, bool ordered)
{
    static WCHAR wchTmpBuff[SMALL_BUFFER];

    StringCchCopyW(wchTmpBuff, COUNTOF(wchTmpBuff), wchStrg);
    TrimSpcW(wchTmpBuff);

    // replace ',' and ';' by space
    WCHAR* s = wchTmpBuff;
    while (s) {
        s = StrChr(wchTmpBuff, L',');  // next
        if (s && *s) {
            *s++ = L' ';
        }
    }
    s = wchTmpBuff;
    while (s) {
        s = StrChr(wchTmpBuff, L';');  // next
        if (s && *s) {
            *s++ = L' ';
        }
    }
    // ensure single spaces only
    const WCHAR *p = StrStr(wchTmpBuff, L"  ");
    while (p) {
        MoveMemory((WCHAR*)p, (WCHAR*)p + 1, (StringCchLen(p,0) + 1) * sizeof(WCHAR));
        p = StrStr(wchTmpBuff, L"  ");  // next
    }
    // separate values
    int const len = (int)StringCchLen(wchTmpBuff, COUNTOF(wchTmpBuff));
    for (int i = 0; i < len; ++i) {
        if (wchTmpBuff[i] == L' ') {
            wchTmpBuff[i] = L'\0';
        }
    }
    wchTmpBuff[len + 1] = L'\0'; // double zero at the end

    // fill default
    for (size_t i = 0; i < iCount; ++i) {
        iVector[i] = iDefault;
    }
    // insert values
    size_t n = 0;
    p = wchTmpBuff;
    while (*p) {
        int iValue;
        if (n < iCount) {
            //if (swscanf_s(p, L"%i", &iValue) == 1) {
            if (StrToIntEx(p, STIF_DEFAULT, &iValue)) {
                iVector[n++] = clampi(iValue, iMin, iMax);
            }
        }
        p = StrEnd(p,0) + 1;
    }

    if (ordered) {
        qsort(iVector, n, sizeof(int), _cmpifunc);
    }

    return n;
}

size_t NormalizeColumnVector(LPSTR chStrg_in, LPWSTR wchStrg_out, size_t iCount)
{
    if (chStrg_in) {
        MultiByteToWideChar(CP_UTF8, 0, chStrg_in, -1, wchStrg_out, (int)iCount);
    }
    StrTrim(wchStrg_out, L"\"'");

    int* const intVector = (int*)AllocMem(iCount * sizeof(int), HEAP_ZERO_MEMORY);
    if (intVector) {
        size_t const cnt = ReadVectorFromString(wchStrg_out, intVector, iCount, 0, LONG_LINES_MARKER_LIMIT, 0, true);
        WCHAR col[32];
        wchStrg_out[0] = L'\0';
        for (size_t i = 0; i < cnt; ++i) {
            StringCchPrintf(col, COUNTOF(col), ((i == 0) ? L"%i" : L" %i"), intVector[i]);
            StringCchCat(wchStrg_out, iCount, col);
        }
        FreeMem(intVector);
        return cnt;
    }
    return 0;
}

//=============================================================================
//
//  StrToFloat()
//  Locale indpendant simple character to float conversion
//
bool StrToFloatEx(WCHAR* wnumber, float* fresult)
{
    if (!wnumber || !fresult) {
        return false;
    }

    int i = 0;
    for (; IsBlankCharW(wnumber[i]); ++i); // skip spaces

    // determine sign
    int const sign = (wnumber[i] == L'-') ? -1 : 1;
    if (wnumber[i] == L'-' || wnumber[i] == L'+') {
        ++i;
    }

    // must be digit now
    if (!IsDigitW(wnumber[i])) {
        return false;
    }

    // digits before decimal
    float val = 0.0f;
    while (IsDigitW(wnumber[i])) {
        val = (val * 10) + (wnumber[i] - L'0');
        ++i;
    }

    // skip decimal point (or comma) if present
    if ((wnumber[i] == L'.') || (wnumber[i] == L',')) {
        ++i;
    }

    //digits after decimal
    int place = 1;
    while (IsDigitW(wnumber[i])) {
        val = val * 10 + (wnumber[i] - L'0');
        place *= 10;
        ++i;
    }

    // the extended part for scientific notations
    float exponent = 1.0f;
    if (wnumber[i] == L'e' || wnumber[i] == L'E') {
        ++i;
        float fexp = 0.0f;
        if (StrToFloatEx(&(wnumber[i]), &fexp)) {
            exponent = powf(10, fexp);
        }
    }

    *fresult = ((sign*val*exponent) / (place));
    return true;
}


//=============================================================================
//
//  FloatToStr()
//
//
void FloatToStr(float fValue, LPWSTR lpszStrg, int cchSize)
{
    if (!lpszStrg) {
        return;
    };
    fValue = Round10th(fValue);
    if (HasNonZeroFraction(fValue)) {
        StringCchPrintf(lpszStrg, cchSize, L"%.3G", fValue);
    } else {
        StringCchPrintf(lpszStrg, cchSize, L"%i", f2int(fValue));
    }
}


//=============================================================================
//
//  CloseNonModalDialogs()
//
void CloseNonModalDialogs()
{
    if (IsWindow(Globals.hwndDlgFindReplace)) {
        SendMessage(Globals.hwndDlgFindReplace, WM_CLOSE, 0, 0);
    }
    if (IsWindow(Globals.hwndDlgCustomizeSchemes)) {
        SendMessage(Globals.hwndDlgCustomizeSchemes, WM_CLOSE, 0, 0);
    }
}

//=============================================================================
//
//  CloseApplication()
//
void CloseApplication()
{
    CloseNonModalDialogs();
    PostMessage(Globals.hwndMain, WM_CLOSE, 0, 0);
}


///////////////////////////////////////////////////////////////////////////////

///   End of Helpers.c   ///
