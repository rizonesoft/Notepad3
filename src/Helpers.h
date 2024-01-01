// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Helpers.h                                                                   *
*   Definitions for general helper functions and macros                       *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2024   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#pragma once
#ifndef _NP3_HELPERS_H_
#define _NP3_HELPERS_H_

#include "TypeDefs.h"

#include <heapapi.h>
#include <process.h>
#include <math.h>
#include <shlwapi.h>
#include <VersionHelpers.h>

#include "Scintilla.h"

// ============================================================================
// ---  Disable/Enable some CodeAnalysis Warnings  ---

#pragma warning ( disable: 26451 28159)
//#pragma warning ( enable : 6001 )

// ============================================================================

// ============================================================================

#ifndef _W
#define __CC(p,s) p ## s
#define _W(s)  __CC(L,s)
#endif

#ifndef _STRG
#define _STRINGIFY(s) #s
#define _STRG(s)  _STRINGIFY(s)
#endif

#define SIZEOF(ar) sizeof(ar)
//#define ARRAYSIZE(A) (assert(!(sizeof(A) % sizeof(*(A)))), (sizeof(A) / sizeof((A)[0])))
#define COUNTOF(ar) ARRAYSIZE(ar)
#define CONSTSTRGLEN(s)  (COUNTOF(s)-1)

#define NOOP ((void)0)

// ============================================================================

// direct heap allocation
#if (defined(_DEBUG) || defined(DEBUG)) && !defined(NDEBUG)
#define DEFAULT_ALLOC_FLAGS (HEAP_GENERATE_EXCEPTIONS | HEAP_TAIL_CHECKING_ENABLED | HEAP_FREE_CHECKING_ENABLED | HEAP_CREATE_HARDENED)
#else
#define DEFAULT_ALLOC_FLAGS (HEAP_CREATE_HARDENED)
#endif

static inline LPVOID AllocMem(size_t numBytes, DWORD dwFlags)
{
    return HeapAlloc(Globals.hndlProcessHeap, (dwFlags | DEFAULT_ALLOC_FLAGS), numBytes);
}

static inline LPVOID ReAllocMem(LPVOID lpMem, size_t numBytes, DWORD dwFlags)
{
    if (lpMem) {
        return HeapReAlloc(Globals.hndlProcessHeap, (dwFlags | DEFAULT_ALLOC_FLAGS), lpMem, numBytes);
    }
    return HeapAlloc(Globals.hndlProcessHeap, (dwFlags | DEFAULT_ALLOC_FLAGS), numBytes);
}

static inline LPVOID ReAllocGrowMem(LPVOID lpMem, size_t numBytes, DWORD dwFlags)
{
    if (lpMem) {
        size_t const memSize = HeapSize(Globals.hndlProcessHeap, 0, lpMem);
        if (memSize >= numBytes) {
            if (dwFlags & HEAP_ZERO_MEMORY) {
                SecureZeroMemory(lpMem, memSize);
            }
            return lpMem;
        }
        return HeapReAlloc(Globals.hndlProcessHeap, (dwFlags | DEFAULT_ALLOC_FLAGS), lpMem, numBytes);
    }
    return HeapAlloc(Globals.hndlProcessHeap, (dwFlags | DEFAULT_ALLOC_FLAGS), numBytes);
}

static inline bool FreeMem(LPVOID lpMem)
{
    return (lpMem ? HeapFree(Globals.hndlProcessHeap, 0, lpMem) : true);
}

static inline size_t SizeOfMem(LPCVOID lpMem)
{
    return (lpMem ? HeapSize(Globals.hndlProcessHeap, 0, lpMem) : 0);
}

// ============================================================================

#if (defined(_DEBUG) || defined(DEBUG)) && !defined(NDEBUG)
void DbgLog(const wchar_t *fmt, ...);
#else
#define DbgLog(fmt, ...) NOOP
#endif

// ============================================================================

// min/max
#define _min_(x,y) (((x) > (y)) ? (y) : (x))
#define _RETCMPMIN_  { return (x > y) ? y : x; }
__forceinline int min_i(const int x, const int y) _RETCMPMIN_
__forceinline unsigned int min_u(const unsigned int x, const unsigned int y) _RETCMPMIN_
__forceinline long min_l(const long x, const long y) _RETCMPMIN_
__forceinline long long min_ll(const long long x, const long long y) _RETCMPMIN_
__forceinline long min_dw(const DWORD x, const DWORD y) _RETCMPMIN_
__forceinline size_t min_s(const size_t x, const size_t y) _RETCMPMIN_
__forceinline DocPos min_p(const DocPos x, const DocPos y) _RETCMPMIN_
__forceinline DocLn min_ln(const DocLn x, const DocLn y) _RETCMPMIN_
__forceinline DocPosCR min_cr(const DocPosCR x, const DocPosCR y) _RETCMPMIN_
__forceinline float min_f(float x, float y) _RETCMPMIN_

#define _max_(x,y) (((x) < (y)) ? (y) : (x))
#define _RETCMPMAX_  { return (x < y) ? y : x; }
__forceinline int max_i(int x, int y) _RETCMPMAX_
__forceinline unsigned int max_u(unsigned int x, unsigned int y) _RETCMPMAX_
__forceinline long max_l(const long x, const long y) _RETCMPMAX_
__forceinline long long max_ll(const long long x, const long long y) _RETCMPMAX_
__forceinline long max_dw(const DWORD x, const DWORD y) _RETCMPMAX_
__forceinline size_t max_s(const size_t x, const size_t y) _RETCMPMAX_
__forceinline DocPos max_p(const DocPos x, const DocPos y) _RETCMPMAX_
__forceinline DocLn max_ln(const DocLn x, const DocLn y) _RETCMPMAX_
__forceinline DocPosCR max_cr(const DocPosCR x, const DocPosCR y) _RETCMPMAX_
__forceinline float max_f(float x, float y) _RETCMPMAX_

__forceinline DocPos abs_p(const DocPos x) {
    return (x >= 0LL) ? x : (0LL - x);
}

// swap
__forceinline void swapi(int *a, int *b) {
    int t = *a;
    *a = *b;
    *b = t;
}
__forceinline void swapos(DocPos *a, DocPos *b) {
    DocPos t = *a;
    *a = *b;
    *b = t;
}

// clamp
__forceinline int clampi(int x, int lower, int upper) {
    return (x < lower) ? lower : ((x > upper) ? upper : x);
}

__forceinline unsigned clampu(unsigned x, unsigned lower, unsigned upper) {
    return (x < lower) ? lower : ((x > upper) ? upper : x);
}

__forceinline unsigned clampul(unsigned long x, unsigned long lower, unsigned long upper) {
    return (x < lower) ? lower : ((x > upper) ? upper : x);
}

__forceinline long long clampll(long long x, long long lower, long long upper) {
    return (x < lower) ? lower : ((x > upper) ? upper : x);
}

__forceinline DocPos clampp(DocPos x, DocPos lower, DocPos upper) {
    return (x < lower) ? lower : ((x > upper) ? upper : x);
}

__forceinline DocPosU clamppu(DocPosU x, DocPosU lower, DocPosU upper) {
    return (x < lower) ? lower : ((x > upper) ? upper : x);
}

__forceinline DocLn d2ln(const double d)
{
    return (DocLn)llround(d);
}

__forceinline int f2int(const float f)
{
    return (int)lroundf(f);
}

__forceinline float Round10th(const float f) {
    return (float)f2int(f * 10.0f) / 10;
}

__forceinline bool HasNonZeroFraction(const float f) {
    return ((f2int(f * 10.0f) % 10) != 0);
}

__forceinline bool IsKeyDown(int key) {
    return (((GetKeyState(key) >> 8) & 0xff) != 0);
}

__forceinline bool IsAsyncKeyDown(int key) {
    return (((GetAsyncKeyState(key) >> 8) & 0xff) != 0);
}

// ----------------------------------------------------------------------------

static inline DWORD GetNumberOfProcessors()
{
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
}

// ----------------------------------------------------------------------------

__forceinline bool Str2Int(LPCWSTR str, int* value)
{
    LPWSTR end;
    *value = (int)wcstol(str, &end, 10);
    return (str != end);
}

__forceinline bool Str2Float(LPCWSTR str, float* value)
{
    LPWSTR end;
    *value = (float)wcstod(str, &end);
    return (str != end);
}

bool StrToFloatEx(WCHAR *wnumber, float *fresult);
void FloatToStr(float fValue, LPWSTR lpszStrg, int cchSize);

// ----------------------------------------------------------------------------

#define RGB_SUB(X, Y) (((X) > (Y)) ? ((X) - (Y)) : ((Y) - (X)))

__forceinline COLORREF CalcContrastColor(COLORREF rgb, int alpha) {

    bool const mask = RGB_SUB(MulDiv(rgb >> 0, alpha, SC_ALPHA_OPAQUE) & SC_ALPHA_OPAQUE, 0x80) <= 0x20 &&
                      RGB_SUB(MulDiv(rgb >> 8, alpha, SC_ALPHA_OPAQUE) & SC_ALPHA_OPAQUE, 0x80) <= 0x20 &&
                      RGB_SUB(MulDiv(rgb >> 16, alpha, SC_ALPHA_OPAQUE) & SC_ALPHA_OPAQUE, 0x80) <= 0x20;

    return mask ? ((0x7F7F7F + rgb)) & 0xFFFFFF : (rgb ^ 0xFFFFFF);
}


__forceinline COLORREF AdjustColor(const COLORREF rgb, const int amount) {

    return RGB(clampu(GetRValue(rgb) + amount, 0, 0xFF),
               clampu(GetGValue(rgb) + amount, 0, 0xFF),
               clampu(GetBValue(rgb) + amount, 0, 0xFF));
}


__forceinline COLORREF ContrastColor(const COLORREF rgb, const float factor)
{
    return RGB(clampu(f2int(factor * (GetRValue(rgb) - 128) + 128.0f), 0, 0xFF),
               clampu(f2int(factor * (GetGValue(rgb) - 128) + 128.0f), 0, 0xFF),
               clampu(f2int(factor * (GetBValue(rgb) - 128) + 128.0f), 0, 0xFF));
}


__forceinline void ColorToHtmlCode(COLORREF rgb, LPWSTR strg, size_t count) {
    StringCchPrintf(strg, count, L"#%02X%02X%02X", (int)GetRValue(rgb), (int)GetGValue(rgb), (int)GetBValue(rgb));
}

// ----------------------------------------------------------------------------

#define SendWMCommandEx(hwnd, id, hi)  SendMessage((hwnd), WM_COMMAND, MAKEWPARAM((id), (hi)), 0)
#define SendWMCommand(hwnd, id)        SendWMCommandEx((hwnd), (id), 1)
#define PostWMCommandEx(hwnd, id, hi)  PostMessage((hwnd), WM_COMMAND, MAKEWPARAM((id), (hi)), 0)
#define PostWMCommand(hwnd, id)        PostWMCommandEx((hwnd), (id), 1)

#define SetWindowStyle(hwnd, style)			  SetWindowLong((hwnd), GWL_STYLE, (style))
#define SetWindowExStyle(hwnd, style)     SetWindowLong((hwnd), GWL_EXSTYLE, (style))

//==== StrIs(Not)Empty() =============================================

__forceinline bool StrIsEmptyA(LPCSTR s) {
    return (!s || (*s == '\0'));
}
__forceinline bool StrIsEmptyW(LPCWSTR s) {
    return (!s || (*s == L'\0'));
}

#if defined(UNICODE) || defined(_UNICODE)
#define StrIsEmpty(s)     StrIsEmptyW(s)
#define StrIsNotEmpty(s)  (!StrIsEmptyW(s))
#else
#define StrIsEmpty(s)     StrIsEmptyA(s)
#define StrIsNotEmpty(s)  (!StrIsEmptyA(s))
#endif

// ----------------------------------------------------------------------------

//inline COLORREF GetBackgroundColor(HWND hwnd) { return GetBkColor(GetDC(hwnd)); }


static inline int SetModeBkColor(const HDC hdc, const bool bDarkMode)
{
#ifdef D_NP3_WIN10_DARK_MODE
    return SetBkColor(hdc, bDarkMode ? Settings2.DarkModeBkgColor : GetSysColor(COLOR_WINDOW));
#else
    UNREFERENCED_PARAMETER(bDarkMode);
    return SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
#endif
}

static inline int SetModeBtnFaceColor(const HDC hdc, const bool bDarkMode)
{
#ifdef D_NP3_WIN10_DARK_MODE
    return SetBkColor(hdc, bDarkMode ? Settings2.DarkModeBtnFaceColor : GetSysColor(COLOR_BTNFACE));
#else
    UNREFERENCED_PARAMETER(bDarkMode);
    return SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
#endif
}

static inline COLORREF GetModeBkColor(const bool bDarkMode)
{
#ifdef D_NP3_WIN10_DARK_MODE
    return bDarkMode ? Settings2.DarkModeBkgColor : (COLORREF)GetSysColor(COLOR_WINDOW);
#else
    UNREFERENCED_PARAMETER(bDarkMode);
    return (COLORREF)GetSysColor(COLOR_WINDOW);
#endif
}

static inline COLORREF GetModeBtnfaceColor(const bool bDarkMode)
{
#ifdef D_NP3_WIN10_DARK_MODE
    return bDarkMode ? Settings2.DarkModeBtnFaceColor : (COLORREF)GetSysColor(COLOR_BTNFACE);
#else
    UNREFERENCED_PARAMETER(bDarkMode);
    return (COLORREF)GetSysColor(COLOR_BTNFACE);
#endif
}


static inline int SetModeTextColor(const HDC hdc, const bool bDarkMode)
{
#ifdef D_NP3_WIN10_DARK_MODE
    //return SetTextColor(hdc, bDarkMode ? Settings2.DarkModeTxtColor : GetSysColor(COLOR_WINDOWTEXT));
    return SetTextColor(hdc, bDarkMode ? ContrastColor(Settings2.DarkModeTxtColor, ((float)Settings.DarkModeHiglightContrast / 100.0f)) : GetSysColor(COLOR_WINDOWTEXT));
#else
    UNREFERENCED_PARAMETER(bDarkMode);
    return SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
#endif
}

static inline COLORREF GetModeTextColor(const bool bDarkMode)
{
#ifdef D_NP3_WIN10_DARK_MODE
    //return bDarkMode ? Settings2.DarkModeTxtColor : (COLORREF)GetSysColor(COLOR_WINDOWTEXT);
    return bDarkMode ? ContrastColor(Settings2.DarkModeTxtColor, ((float)Settings.DarkModeHiglightContrast / 100.0f)) : (COLORREF)GetSysColor(COLOR_WINDOWTEXT);
#else
    UNREFERENCED_PARAMETER(bDarkMode);
    return (COLORREF)GetSysColor(COLOR_WINDOWTEXT);
#endif
}


#ifdef D_NP3_WIN10_DARK_MODE

static inline INT_PTR SetDarkModeCtlColors(const HDC hdc, const bool bDarkMode)
{
    if (bDarkMode) {
        SetBkColor(hdc, Settings2.DarkModeBkgColor);
        SetTextColor(hdc, Settings2.DarkModeTxtColor);
    }
    return (INT_PTR)(bDarkMode ? Globals.hbrDarkModeBkgBrush : FALSE);
}

#endif

// ----------------------------------------------------------------------------

void GetWinVersionString(LPWSTR szVersionStr, size_t cchVersionStr);

// ----------------------------------------------------------------------------

#define GET_X_LPARAM(lp)    ((int)(short)LOWORD(lp))  // windowsx.h
#define GET_Y_LPARAM(lp)    ((int)(short)HIWORD(lp))  // windowsx.h

// ----------------------------------------------------------------------------

bool SetClipboardText(HWND hwnd, LPCWSTR pszTextW, size_t cchTextW);

// ----------------------------------------------------------------------------

static inline void GetCurrentMonitorResolution(HWND hwnd, int* pCXScreen, int* pCYScreen)
{
    HMONITOR const hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = { sizeof(MONITORINFO) };
    GetMonitorInfo(hMonitor, &mi);
    *pCXScreen = (mi.rcMonitor.right - mi.rcMonitor.left);
    *pCYScreen = (mi.rcMonitor.bottom - mi.rcMonitor.top);
}

// FullHD? =>   0:'==',   -1:'<',   +1:'>'
static inline int IsFullHD(HWND hwnd, int resX, int resY)
{
    int cxScreen, cyScreen;
    GetCurrentMonitorResolution(hwnd, &cxScreen, &cyScreen);
    if (resX <= 0) {
        resX = cxScreen;
    }
    if (resY <= 0) {
        resY = cyScreen;
    }
    return ((resX == 1920) && (resY == 1080)) ? 0 : (((resX < 1920) || (resY < 1080)) ? -1 : +1);
}

// ----------------------------------------------------------------------------

HRESULT PrivateSetCurrentProcessExplicitAppUserModelID(PCWSTR AppID);

bool IsProcessElevated();
//bool IsUserAdmin();
bool IsUserInAdminGroup();
bool IsRunAsAdmin();

void        BackgroundWorker_Init(BackgroundWorker* worker, HWND hwnd, const HPATHL hFilePath);
void        BackgroundWorker_Start(BackgroundWorker* worker, _beginthreadex_proc_type routine, LPVOID property);
void        BackgroundWorker_Cancel(BackgroundWorker* worker);
void        BackgroundWorker_Destroy(BackgroundWorker* worker);

static inline bool BackgroundWorker_Continue(BackgroundWorker* worker) { 
    return (worker) ? (WaitForSingleObject(worker->eventCancel, 0) != WAIT_OBJECT_0) : false;
}
static inline void BackgroundWorker_End(BackgroundWorker* worker, unsigned int retcode) { if (worker) { _endthreadex(retcode); }}


bool BitmapMergeAlpha(HBITMAP hbmp,COLORREF crDest);
bool BitmapAlphaBlend(HBITMAP hbmp,COLORREF crDest,BYTE alpha);
bool BitmapGrayScale(HBITMAP hbmp);

bool VerifyContrast(COLORREF cr1,COLORREF cr2);
bool IsFontAvailable(LPCWSTR lpszFontName);

void GetSystemCaptionFontA(LPSTR fontFaceName_out, bool bForceRefresh);
void GetSystemMenuFontA(LPSTR fontFaceName_out, bool bForceRefresh);
void GetSystemMessageFontA(LPSTR fontFaceName_out, bool bForceRefresh);
void GetSystemStatusFontA(LPSTR fontFaceName_out, bool bForceRefresh);

bool IsCmdEnabled(HWND hwnd, UINT uId);


#define SetBtn(b) ((b) ? BST_CHECKED : BST_UNCHECKED)

__forceinline bool IsButtonChecked(HWND hwnd, int iButtonID)
{
    return (IsDlgButtonChecked(hwnd, iButtonID) == BST_CHECKED);
}
__forceinline bool IsButtonIntermediate(HWND hwnd, int iButtonID)
{
    return (IsDlgButtonChecked(hwnd, iButtonID) == BST_INDETERMINATE);
}
__forceinline bool IsButtonUnchecked(HWND hwnd, int iButtonID)
{
    return (IsDlgButtonChecked(hwnd, iButtonID) == BST_UNCHECKED);
}

#define EnableItem(hwnd, id, b) EnableWindow(GetDlgItem((hwnd), (id)), (b))

#define EnableCmd(hmenu,id,b) EnableMenuItem((hmenu),(id),(b)?MF_BYCOMMAND|MF_ENABLED:MF_BYCOMMAND|MF_GRAYED)
#define CheckCmd(hmenu,id,b)  CheckMenuItem((hmenu),(id),(b)?MF_BYCOMMAND|MF_CHECKED:MF_BYCOMMAND|MF_UNCHECKED)

#define EnableTool(htbar,id,b) SendMessage((htbar),TB_ENABLEBUTTON,(id), MAKELONG(((b) ? 1 : 0), 0))
#define CheckTool(htbar,id,b)  SendMessage((htbar),TB_CHECKBUTTON,(id), MAKELONG((b),0))

#define EnableCmdPos(hmenu,pos,b) EnableMenuItem((hmenu),(pos),(b)?MF_BYPOSITION|MF_ENABLED:MF_BYPOSITION|MF_GRAYED)
#define CheckCmdPos(hmenu,pos,b)  CheckMenuItem((hmenu),(pos),(b)?MF_BYPOSITION|MF_CHECKED:MF_BYPOSITION|MF_UNCHECKED)


bool ReadFileXL(HANDLE hFile, char* const lpBuffer, const size_t nNumberOfBytesToRead, size_t* const lpNumberOfBytesRead);
bool WriteFileXL(HANDLE hFile, const char* const lpBuffer, const size_t nNumberOfBytesToWrite, size_t* const lpNumberOfBytesWritten);

bool  SplitFilePathLineNum(LPWSTR lpszPath, int *lineNum);

bool StrLTrimI(LPWSTR pszSource,LPCWSTR pszTrimChars);
bool StrRTrimI(LPWSTR pszSource,LPCWSTR pszTrimChars);

static inline bool TrimSpcA(LPSTR lpString)
{
    if (!lpString || !*lpString) {
        return false;
    }
    return (bool)StrTrimA(lpString, " \t\v");
};

static inline bool TrimSpcW(LPWSTR lpString)
{
    if (!lpString || !*lpString) {
        return false;
    }
    return (bool)StrTrim(lpString, L" \t\v");
};

#if (defined(UNICODE) || defined(_UNICODE))
#define TrimSpc TrimSpcW
#else
#define TrimSpc TrimSpcA
#endif

// return memory ownership: use FreeMem() to delete returned string
LPWSTR StrReplaceAll(LPCWSTR str, LPCWSTR from, LPCWSTR to);

bool ExtractFirstArgument(LPCWSTR lpArgs, LPWSTR lpArg1, LPWSTR lpArg2, int len);

void PrepareFilterStr(LPWSTR lpFilter);

void StrTab2Space(LPWSTR lpsz);
void PathFixBackslashes(LPWSTR lpsz);


size_t FormatNumberStr(LPWSTR lpNumberStr, size_t cch, int fixedWidth);
bool SetDlgItemIntEx(HWND hwnd,int nIdItem,UINT uValue);

///////////////////////////////////////////////////////////////////////
///  UINT  GDI CHARSET values  ==  Scintilla's  SC_CHARSET_XXX values
///////////////////////////////////////////////////////////////////////
#define GdiCharsetToSci(charset) ((int)(charset))
UINT CodePageFromCharSet(const UINT uCharSet);
//~UINT CharSetFromCodePage(const UINT uCodePage);


//==== UnSlash Functions ======================================================

size_t UnSlashA(LPSTR pchInOut, UINT cpEdit);
size_t UnSlashW(LPWSTR pchInOut, UINT cpEdit);

size_t UnSlashCharW(LPWSTR pchInOut, WCHAR wch);

size_t SlashCtrlW(LPWSTR pchOutput, size_t cchOutLen, LPCWSTR pchInput);
size_t UnSlashCtrlW(LPWSTR pchInOut);

void TransformBackslashesA(LPSTR pszInput, bool bRegEx, UINT cpEdit, int *iReplaceMsg);
void TransformBackslashesW(LPWSTR pszInput, bool bRegEx, UINT cpEdit, int *iReplaceMsg);
//void TransformMetaChars(char *pszInput, size_t cch, bool bRegEx, int iEOLMode);


//==== Large Text Conversion ==================================================

#undef WC2MB_EX
#undef MB2WC_EX

#ifdef WC2MB_EX
ptrdiff_t WideCharToMultiByteEx(
    UINT CodePage, DWORD dwFlags, LPCWCH lpWideCharStr, ptrdiff_t cchWideChar,
    LPSTR lpMultiByteStr, ptrdiff_t cbMultiByte, LPCCH lpDefaultChar, LPBOOL lpUsedDefaultChar);
#else

__forceinline ptrdiff_t WideCharToMultiByteEx(
    UINT CodePage, DWORD dwFlags, LPCWCH lpWideCharStr, ptrdiff_t cchWideChar,
    LPSTR lpMultiByteStr, ptrdiff_t cbMultiByte, LPCCH lpDefaultChar, LPBOOL lpUsedDefaultChar)
{
    return (ptrdiff_t)WideCharToMultiByte(CodePage, dwFlags, lpWideCharStr, (int)cchWideChar,
                                          lpMultiByteStr, (int)cbMultiByte, lpDefaultChar, lpUsedDefaultChar);
}

#endif


#ifdef MB2WC_EX
ptrdiff_t MultiByteToWideCharEx(
    UINT CodePage, DWORD dwFlags, LPCCH lpMultiByteStr, ptrdiff_t cbMultiByte,
    LPWSTR lpWideCharStr, ptrdiff_t cchWideChar);
#else

__forceinline ptrdiff_t MultiByteToWideCharEx(
    UINT CodePage, DWORD dwFlags, LPCCH lpMultiByteStr, ptrdiff_t cbMultiByte,
    LPWSTR lpWideCharStr, ptrdiff_t cchWideChar)
{
    return (ptrdiff_t)MultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr, (int)cbMultiByte,
                                          lpWideCharStr, (int)cchWideChar);
}

#endif

// ============================================================================

__forceinline int wcscmp_s(const wchar_t* s1, const wchar_t* s2)
{
    return (s1 && s2) ? wcscmp(s1, s2) : ((s1 ? 1 : (s2 ? -1 : 0)));
}

__forceinline int wcscoll_s(const wchar_t* s1, const wchar_t* s2)
{
    return (s1 && s2) ? wcscoll(s1, s2) : ((s1 ? 1 : (s2 ? -1 : 0)));
}

__forceinline int wcsicmp_s(const wchar_t* s1, const wchar_t* s2)
{
    return (s1 && s2) ? _wcsicmp(s1, s2) : ((s1 ? 1 : (s2 ? -1 : 0)));
}

__forceinline int wcsicoll_s(const wchar_t* s1, const wchar_t* s2)
{
    return (s1 && s2) ? _wcsicoll(s1, s2) : ((s1 ? 1 : (s2 ? -1 : 0)));
}

// ============================================================================

static inline void SwabEx(char* src, char* dest, size_t n)
{
    static int const max = (INT_MAX - (INT_MAX % 2));

    if (n <= (size_t)max) {
        _swab(src, dest, (int)n);
    } else {
        size_t size = n - (n % 2LL);
        while (size > (size_t)max) {
            _swab(src, dest, max);
            size -= max;
            src += max;
            dest += max;
        }
        if (size > 0) {
            _swab(src, dest, (int)size);
        }
    }
}

// console helper from Print.cpp
//void RedirectIOToConsole();

//==== StrCut methods ===================

WCHAR* StrCutIW(WCHAR* s, const WCHAR* pattern);
CHAR*  StrCutIA(CHAR* s, const CHAR* pattern);
#if defined(UNICODE) || defined(_UNICODE)
#define StrCutI StrCutIW
#else
#define StrCutI _StrCutIA
#endif


//==== StrNextTok methods ===================
WCHAR* StrNextTokW(WCHAR* strg, const WCHAR* tokens);
CHAR*  StrNextTokA(CHAR* strg, const CHAR* tokens);
#if defined(UNICODE) || defined(_UNICODE)
#define StrNextTok StrNextTokW
#else
#define StrNextTok StrNextTokA
#endif

// ----------------------------------------------------------------------------

bool StrDelChrW(LPWSTR pszSource, LPCWSTR pCharsToRemove);
bool StrDelChrA(LPSTR pszSource, LPCSTR pCharsToRemove);
#if defined(UNICODE) || defined(_UNICODE)
#define StrDelChr(s, r) StrDelChrW((s), (r))
#else
#define StrDelChr(s, r) StrDelChrA((s), (r))
#endif


//==== StrSafe lstrlen() =======================================================

// inline size_t StringCchLenW(LPCWSTR s, size_t n) {
//   n = (n ? n : STRSAFE_MAX_CCH); size_t len; return (size_t)(!s ? 0 : (SUCCEEDED(StringCchLengthW(s, n, &len)) ? len : n));
// }
static inline size_t StringCchLenW(LPCWSTR s, size_t n)
{
    n = (n ? n : STRSAFE_MAX_CCH);
    return (s ? wcsnlen_s(s, n) : 0LL);
}
// inline size_t StringCchLenA(LPCSTR s, size_t n) {
//   n = (n ? n : STRSAFE_MAX_CCH); size_t len; return (size_t)(!s ? 0 : (SUCCEEDED(StringCchLengthA(s, n, &len)) ? len : n));
// }
static inline size_t StringCchLenA(LPCSTR s, size_t n)
{
    n = (n ? n : STRSAFE_MAX_CCH);
    return (s ? strnlen_s(s, n) : 0LL);
}
#if defined(UNICODE) || defined(_UNICODE)
#define StringCchLen(s, n) StringCchLenW((s), (n))
#else
#define StringCchLen(s, n) StringCchLenA((s), (n))
#endif

// ----------------------------------------------------------------------------

static inline WCHAR* StrEndW(const WCHAR* pStart, size_t siz)
{
    // cppcheck-suppress cert-EXP05-C   // Attempt to cast away const - Intended(!)
    return (WCHAR*)(pStart + StringCchLenW(pStart, siz));
}
static inline char* StrEndA(const char* pStart, size_t siz)
{
    // cppcheck-suppress cert-EXP05-C   // Attempt to cast away const - Intended(!)
    return (char*)(pStart + StringCchLenA(pStart, siz));
}
#if defined(UNICODE) || defined(_UNICODE)
#define StrEnd(s, n) StrEndW((s), (n))
#else
#define StrEnd(s, n) StrEndA((s), (n))
#endif

// ----------------------------------------------------------------------------

static inline void StrReplChrW(WCHAR* pStrg, const WCHAR chSearch, const WCHAR chReplace)
{
    while (pStrg && *pStrg) {
        if (*pStrg == chSearch) {
            *pStrg = chReplace;
        }
        ++pStrg;
    }
}
static inline void StrReplChrA(CHAR* pStrg, const CHAR chSearch, const CHAR chReplace)
{
    while (pStrg && *pStrg) {
        if (*pStrg == chSearch) {
            *pStrg = chReplace;
        }
        ++pStrg;
    }
}
#if defined(UNICODE) || defined(_UNICODE)
#define StrReplChr(str, cs, cr) StrReplChrW((str), (cs), (cr))
#else
#define StrReplChr(str, cs, cr) StrReplChrA((str), (cs), (cr))
#endif

// ----------------------------------------------------------------------------

//==== StrSafe lstrcmp(),lstrcmpi() =============================================

__forceinline bool IsSameCharSequence(const char* pSrc, const char* pCmp, CONST DocPos len) {
    DocPos i = 0;
    for (i = 0; (i < len) && (pSrc[i] == pCmp[i]); ++i) {}
    return (i == len);
}

// NOTE: !!! differences in AutoCompleteList depending compare functions (CRT (lstrcmp(),lstrcmpi()) vs. Shlwapi)) !!!
#if defined(UNICODE) || defined(_UNICODE)
#define StringCchCompareX(s1, s2)        StrCmpW((s1), (s2))
#define StringCchCompareXI(s1, s2)       StrCmpIW((s1), (s2))
#else
#define StringCchCompareX(s1, s2)        StrCmpA((s1), (s2))
#define StringCchCompareXI(s1, s2)       StrCmpIA((s1), (s2))
#endif


#define StringCchStartsWithW(s1, s2)     (StrCmpNW((s1), (s2),  (int)StringCchLenW((s2),0)) == 0)
#define StringCchStartsWithIW(s1, s2)    (StrCmpNIW((s1), (s2), (int)StringCchLenW((s2),0)) == 0)
#define StringCchStartsWithA(s1, s2)     (StrCmpNA((s1), (s2),  (int)StringCchLenA((s2),0)) == 0)
#define StringCchStartsWithIA(s1, s2)    (StrCmpNIA((s1), (s2), (int)StringCchLenA((s2),0)) == 0)

#if defined(UNICODE) || defined(_UNICODE)
#define StringCchStartsWith(s1, s2)      StringCchStartsWithW((s1), (s2))
#define StringCchStartsWithI(s1,s2)      StringCchStartsWithIW((s1),(s2))
#else
#define StringCchStartsWith(s1,s2)       StringCchStartsWithA((s1),(s2))
#define StringCchStartsWithI(s1,s2)      StringCchStartsWithIA((s1),(s2))
#endif


#ifdef __cplusplus
#undef NULL
#define NULL nullptr
#endif

// ----------------------------------------------------------------------------

// Is the character an octal digit?
#define IsOctalDigitA(ch) (((ch) >= '0') && ((ch) <= '7'))
#define IsOctalDigitW(wch) (((wch) >= L'0') && ((wch) <= L'7'))

// Is the character an octal digit?
__forceinline bool IsDigitA(const char ch)
{
    return ((ch >= '0') && (ch <= '9'));
}
__forceinline bool IsDigitW(const WCHAR wch)
{
    return ((wch >= L'0') && (wch <= L'9'));
}

// Is the character a white space char?
__forceinline bool IsBlankCharA(const char ch)
{
    return ((ch == ' ') || (ch == '\t'));
}
__forceinline bool IsBlankCharW(const WCHAR wch)
{
    return ((wch == L' ') || (wch == L'\t'));
}

// no encoding for safe chars
__forceinline bool IsAlphaNumericA(const char ch)
{
    return ((ch >= '0') && (ch <= '9')) ||
           ((ch >= 'a') && (ch <= 'z')) ||
           ((ch >= 'A') && (ch <= 'Z'));
}

__forceinline bool IsAlphaNumericW(const WCHAR ch)
{
    return
        ((ch >= L'0') && (ch <= L'9')) ||
        ((ch >= L'a') && (ch <= L'z')) ||
        ((ch >= L'A') && (ch <= L'Z'));
}

// If the character is an hexadecimal digit, get its value.
static inline int GetHexDigitA(const char ch)
{
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    }
    if (ch >= 'a' && ch <= 'f') {
        return ch - 'a' + 10;
    }
    return -1;
}

static inline int GetHexDigitW(const WCHAR ch)
{
    if (ch >= L'0' && ch <= L'9') {
        return ch - L'0';
    }
    if (ch >= L'A' && ch <= L'F') {
        return ch - L'A' + 10;
    }
    if (ch >= L'a' && ch <= L'f') {
        return ch - L'a' + 10;
    }
    return -1;
}
// ----------------------------------------------------------------------------

void UrlEscapeEx(LPCWSTR lpURL, LPWSTR lpEscaped, DWORD* pcchEscaped, bool bEscReserved);
void UrlUnescapeEx(LPWSTR lpURL, LPWSTR lpUnescaped, DWORD* pcchUnescaped);

int    ReadStrgsFromCSV(LPCWSTR wchCSVStrg, prefix_t sMatrix[], int iCount, int iLen, LPCWSTR sDefault);
size_t ReadVectorFromString(LPCWSTR wchStrg, int iVector[], size_t iCount, int iMin, int iMax, int iDefault, bool ordered);
size_t NormalizeColumnVector(LPSTR chStrg_in, LPWSTR wchStrg_out, size_t iCount);

#define MAX_ESCAPE_HEX_DIGIT 4
int Hex2Char(char* ch, int cnt);

size_t SimpleHash(LPCWSTR string);

    void CloseNonModalDialogs();
void CloseApplication();

// ----------------------------------------------------------------------------

static inline int PointSizeToFontHeight(const float fPtHeight, const HDC hdc) {
    return -MulDiv(f2int(fPtHeight * 100.0f), GetDeviceCaps(hdc, LOGPIXELSY), 72 * SC_FONT_SIZE_MULTIPLIER);
}

// ----------------------------------------------------------------------------


static inline LONG64 GetTicks_ms() {
    LARGE_INTEGER freq;
    if (!QueryPerformanceFrequency(&freq)) {
        return (LONG64)GetTickCount64();
    }
    LARGE_INTEGER ticks;
    if (!QueryPerformanceCounter(&ticks)) {
        return (LONG64)GetTickCount64();
    }
    return (ticks.QuadPart * 1000LL) / freq.QuadPart;
}

// ----------------------------------------------------------------------------

#endif //_NP3_HELPERS_H_

///   End of Helpers.h   ///
