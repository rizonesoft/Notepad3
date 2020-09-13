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
*                                                  (c) Rizonesoft 2008-2020   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#pragma once
#ifndef _NP3_HELPERS_H_
#define _NP3_HELPERS_H_

#include "TypeDefs.h"

#include <math.h>
#include <shlwapi.h>
#include <heapapi.h>
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

#define UNUSED(expr) (void)(expr)
#define SIZEOF(ar) sizeof(ar)
//#define ARRAYSIZE(A) (assert(!(sizeof(A) % sizeof(*(A)))), (sizeof(A) / sizeof((A)[0])))
#define COUNTOF(ar) ARRAYSIZE(ar)
#define CSTRLEN(s)  (COUNTOF(s)-1)

#define NOOP ((void)0)

// ============================================================================

// direct heap allocation
#if (defined(_DEBUG) || defined(DEBUG)) && !defined(NDEBUG)
  #define DEFAULT_ALLOC_FLAGS (HEAP_GENERATE_EXCEPTIONS)
#else
  #define DEFAULT_ALLOC_FLAGS (0)
#endif

inline LPVOID AllocMem(size_t numBytes, DWORD dwFlags)
{
  return HeapAlloc(Globals.hndlProcessHeap, (dwFlags | DEFAULT_ALLOC_FLAGS), numBytes);
}

inline bool FreeMem(LPVOID lpMemory)
{
  return (lpMemory ? HeapFree(Globals.hndlProcessHeap, 0, lpMemory) : true);
}

inline size_t SizeOfMem(LPCVOID lpMemory)
{
  return (lpMemory ? HeapSize(Globals.hndlProcessHeap, 0, lpMemory) : 0);
}

// ============================================================================

#if (defined(_DEBUG) || defined(DEBUG)) && !defined(NDEBUG)
void DbgLog(const char *fmt, ...);
#else
#define DbgLog(fmt, ...) NOOP
#endif

// ============================================================================

// min/max
#define _min_(x,y) (((x) > (y)) ? (y) : (x))
#define _RETCMPMIN_  { return (x > y) ? y : x; }
inline int min_i(const int x, const int y) _RETCMPMIN_
inline unsigned int min_u(const unsigned int x, const unsigned int y) _RETCMPMIN_
inline long min_l(const long x, const long y) _RETCMPMIN_
inline long min_dw(const DWORD x, const DWORD y) _RETCMPMIN_
inline size_t min_s(const size_t x, const size_t y) _RETCMPMIN_
inline DocPos min_p(const DocPos x, const DocPos y) _RETCMPMIN_
inline DocLn min_ln(const DocLn x, const DocLn y) _RETCMPMIN_
inline DocPosCR min_cr(const DocPosCR x, const DocPosCR y) _RETCMPMIN_
inline float min_f(float x, float y) _RETCMPMIN_

#define _max_(x,y) (((x) < (y)) ? (y) : (x))
#define _RETCMPMAX_  { return (x < y) ? y : x; }
inline int max_i(int x, int y) _RETCMPMAX_
inline unsigned int max_u(unsigned int x, unsigned int y) _RETCMPMAX_
inline long max_l(const long x, const long y) _RETCMPMAX_
inline long max_dw(const DWORD x, const DWORD y) _RETCMPMAX_
inline size_t max_s(const size_t x, const size_t y) _RETCMPMAX_
inline DocPos max_p(const DocPos x, const DocPos y) _RETCMPMAX_
inline DocLn max_ln(const DocLn x, const DocLn y) _RETCMPMAX_
inline DocPosCR max_cr(const DocPosCR x, const DocPosCR y) _RETCMPMAX_
inline float max_f(float x, float y) _RETCMPMAX_

inline DocPos abs_p(const DocPos x) { return (x >= 0LL) ? x : (0LL - x); }

// swap 
inline void swapi(int* a, int* b) { int t = *a;  *a = *b;  *b = t; }
inline void swapos(DocPos* a, DocPos* b) { DocPos t = *a;  *a = *b;  *b = t; }

// clamp
inline int clampi(int x, int lower, int upper) {
  return (x < lower) ? lower : ((x > upper) ? upper : x);
}

inline unsigned clampu(unsigned x, unsigned lower, unsigned upper) {
  return (x < lower) ? lower : ((x > upper) ? upper : x);
}

inline unsigned clampul(unsigned long x, unsigned long lower, unsigned long upper) {
  return (x < lower) ? lower : ((x > upper) ? upper : x);
}

inline DocPos clampp(DocPos x, DocPos lower, DocPos upper) {
  return (x < lower) ? lower : ((x > upper) ? upper : x);
}

inline DocPosU clamppu(DocPosU x, DocPosU lower, DocPosU upper) {
  return (x < lower) ? lower : ((x > upper) ? upper : x);
}

// Is the character an octal digit?
inline bool IsDigitA(const CHAR ch) { return ((ch >= '0') && (ch <= '9')); }
inline bool IsDigitW(const WCHAR wch) { return ((wch >= L'0') && (wch <= L'9')); }

// Is the character a white space char?
inline bool IsBlankChar(const CHAR ch) { return ((ch == ' ') || (ch == '\t')); }
inline bool IsBlankCharW(const WCHAR wch) { return ((wch == L' ') || (wch == L'\t')); }

inline int float2int(const float f) { return (int)lroundf(f); }
inline float Round10th(const float f) { return (float)float2int(f * 10.0f) / 10; }
inline bool HasNonZeroFraction(const float f) { return ((float2int(f * 10.0f) % 10) != 0); }

inline bool IsKeyDown(int key) { return (((GetKeyState(key) >> 8) & 0xff) != 0); }
inline bool IsAsyncKeyDown(int key) { return (((GetAsyncKeyState(key) >> 8) & 0xff) != 0); }

// ----------------------------------------------------------------------------

#define SendWMCommandEx(hwnd, id, extra)  SendMessage((hwnd), WM_COMMAND, MAKEWPARAM((id), (extra)), 0)
#define SendWMCommand(hwnd, id)           SendWMCommandEx((hwnd), (id), 1)
#define PostWMCommand(hwnd, id)           PostMessage((hwnd), WM_COMMAND, MAKEWPARAM((id), 1), 0)

#define SetWindowStyle(hwnd, style)			  SetWindowLong((hwnd), GWL_STYLE, (style))
#define SetWindowExStyle(hwnd, style)     SetWindowLong((hwnd), GWL_EXSTYLE, (style))

//==== StrIs(Not)Empty() =============================================

inline bool StrIsEmptyA(LPCSTR s) { return (!s || (*s == '\0')); }
inline bool StrIsEmptyW(LPCWSTR s) { return (!s || (*s == L'\0')); }

#if defined(UNICODE) || defined(_UNICODE)
#define StrIsEmpty(s)     StrIsEmptyW(s)
#define StrIsNotEmpty(s)  (!StrIsEmptyW(s))
#else
#define StrIsEmpty(s)     StrIsEmptyA(s)
#define StrIsNotEmpty(s)  (!StrIsEmptyA(s))
#endif

// ----------------------------------------------------------------------------

inline COLORREF GetBackgroundColor(HWND hwnd) { return GetBkColor(GetDC(hwnd)); }

// ----------------------------------------------------------------------------

void GetWinVersionString(LPWSTR szVersionStr, size_t cchVersionStr);

// ----------------------------------------------------------------------------

#define RGB_GET_R(color)    ((BYTE)((0xFF)&(color)))
#define RGB_GET_G(color)    ((BYTE)(((0xFF<<8)&(color))>>8))
#define RGB_GET_B(color)    ((BYTE)(((0xFF<<16)&(color))>>16))

#define GET_X_LPARAM(lp)    ((int)(short)LOWORD(lp))  // windowsx.h
#define GET_Y_LPARAM(lp)    ((int)(short)HIWORD(lp))  // windowsx.h

// ----------------------------------------------------------------------------

bool SetClipboardTextW(HWND hwnd, LPCWSTR pszTextW, size_t cchTextW);

// ----------------------------------------------------------------------------

inline void GetCurrentMonitorResolution(HWND hwnd, int* pCXScreen, int* pCYScreen)
{
  HMONITOR const hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
  MONITORINFO mi;
  ZeroMemory(&mi, sizeof(MONITORINFO));
  mi.cbSize = sizeof(mi);
  GetMonitorInfo(hMonitor, &mi);
  *pCXScreen = (mi.rcMonitor.right - mi.rcMonitor.left);
  *pCYScreen = (mi.rcMonitor.bottom - mi.rcMonitor.top);
}

// FullHD? =>   0:'==',   -1:'<',   +1:'>'
inline int IsFullHD(HWND hwnd, int resX, int resY) 
{
  int cxScreen, cyScreen;
  GetCurrentMonitorResolution(hwnd, &cxScreen, &cyScreen);
  if (resX <= 0) { resX = cxScreen; }
  if (resY <= 0) { resY = cyScreen; }
  return ((resX == 1920) && (resY == 1080)) ? 0 : (((resX < 1920) || (resY < 1080)) ? -1 : +1);
}

// ----------------------------------------------------------------------------

HRESULT PrivateSetCurrentProcessExplicitAppUserModelID(PCWSTR AppID);
bool IsProcessElevated();
//bool IsUserAdmin();
bool IsUserInAdminGroup();
bool IsRunAsAdmin();

bool BitmapMergeAlpha(HBITMAP hbmp,COLORREF crDest);
bool BitmapAlphaBlend(HBITMAP hbmp,COLORREF crDest,BYTE alpha);
bool BitmapGrayScale(HBITMAP hbmp);

bool VerifyContrast(COLORREF cr1,COLORREF cr2);
bool IsFontAvailable(LPCWSTR lpszFontName);

bool IsCmdEnabled(HWND hwnd, UINT uId);


#define SetBtn(b) ((b) ? BST_CHECKED : BST_UNCHECKED)

inline bool IsButtonChecked(HWND hwnd, int iButtonID) { return (IsDlgButtonChecked(hwnd, iButtonID) == BST_CHECKED); }
inline bool IsButtonIntermediate(HWND hwnd, int iButtonID) { return (IsDlgButtonChecked(hwnd, iButtonID) == BST_INDETERMINATE); }
inline bool IsButtonUnchecked(HWND hwnd, int iButtonID) { return (IsDlgButtonChecked(hwnd, iButtonID) == BST_UNCHECKED); }


#define EnableCmd(hmenu,id,b) EnableMenuItem((hmenu),(id),(b)?MF_BYCOMMAND|MF_ENABLED:MF_BYCOMMAND|MF_GRAYED)
#define CheckCmd(hmenu,id,b)  CheckMenuItem((hmenu),(id),(b)?MF_BYCOMMAND|MF_CHECKED:MF_BYCOMMAND|MF_UNCHECKED)

#define EnableTool(htbar,id,b) SendMessage((htbar),TB_ENABLEBUTTON,(id), MAKELONG(((b) ? 1 : 0), 0))
#define CheckTool(htbar,id,b)  SendMessage((htbar),TB_CHECKBUTTON,(id), MAKELONG((b),0))

#define EnableCmdPos(hmenu,pos,b) EnableMenuItem((hmenu),(pos),(b)?MF_BYPOSITION|MF_ENABLED:MF_BYPOSITION|MF_GRAYED)
#define CheckCmdPos(hmenu,pos,b)  CheckMenuItem((hmenu),(pos),(b)?MF_BYPOSITION|MF_CHECKED:MF_BYPOSITION|MF_UNCHECKED)


bool ReadFileXL(HANDLE hFile, char* const lpBuffer, const size_t nNumberOfBytesToRead, size_t* const lpNumberOfBytesRead);
bool WriteFileXL(HANDLE hFile, const char* const lpBuffer, const size_t nNumberOfBytesToWrite, size_t* const lpNumberOfBytesWritten);
void PathGetAppDirectory(LPWSTR lpszDest, DWORD cchDest);
bool GetKnownFolderPath(REFKNOWNFOLDERID, LPWSTR lpOutPath, size_t cchCount);
void PathRelativeToApp(LPWSTR lpszSrc,LPWSTR lpszDest,int cchDest,bool,bool,bool);
void PathAbsoluteFromApp(LPWSTR lpszSrc,LPWSTR lpszDest,int cchDest,bool);

bool PathIsLnkFile(LPCWSTR pszPath);
bool PathGetLnkPath(LPCWSTR pszLnkFile,LPWSTR pszResPath,int cchResPath);
bool PathIsLnkToDirectory(LPCWSTR pszPath,LPWSTR pszResPath,int cchResPath);
bool PathCreateDeskLnk(LPCWSTR pszDocument);
bool PathCreateFavLnk(LPCWSTR pszName,LPCWSTR pszTarget,LPCWSTR pszDir);

void  ExpandEnvironmentStringsEx(LPWSTR lpSrc, DWORD dwSrc);
bool  PathCanonicalizeEx(LPWSTR lpszPath, DWORD cchPath);
DWORD GetLongPathNameEx(LPWSTR lpszPath, DWORD cchBuffer);
void  PathGetDisplayName(LPWSTR lpszDestPath, DWORD cchDestBuffer, LPCWSTR lpszSourcePath);
DWORD NormalizePathEx(LPWSTR lpszPath, DWORD cchBuffer, bool bRealPath, bool bSearchPathIfRelative);


bool StrLTrimI(LPWSTR pszSource,LPCWSTR pszTrimChars);
bool StrRTrimI(LPWSTR pszSource,LPCWSTR pszTrimChars);

inline bool TrimSpcA(LPSTR lpString) {
  if (!lpString || !*lpString) { return false; }
  return (bool)StrTrimA(lpString, " \t\v");
};

inline bool TrimSpcW(LPWSTR lpString) {
  if (!lpString || !*lpString) { return false; }
  return (bool)StrTrim(lpString, L" \t\v");
};

#if (defined(UNICODE) || defined(_UNICODE))
#define TrimSpc TrimSpcW
#else
#define TrimSpc TrimSpcA
#endif

bool ExtractFirstArgument(LPCWSTR lpArgs, LPWSTR lpArg1, LPWSTR lpArg2, int len);

void PrepareFilterStr(LPWSTR lpFilter);

void StrTab2Space(LPWSTR lpsz);
void PathFixBackslashes(LPWSTR lpsz);


size_t FormatNumberStr(LPWSTR lpNumberStr, size_t cch, int fixedWidth);
bool SetDlgItemIntEx(HWND hwnd,int nIdItem,UINT uValue);

UINT GetDlgItemTextW2MB(HWND hDlg, int nIDDlgItem, LPSTR lpString, int nMaxCount);
UINT SetDlgItemTextMB2W(HWND hDlg, int nIDDlgItem, LPCSTR lpString);
LRESULT ComboBox_AddStringMB2W(HWND hwnd, LPCSTR lpString);


///////////////////////////////////////////////////////////////////////
///  UINT  GDI CHARSET values  ==  Scintilla's  SC_CHARSET_XXX values
///////////////////////////////////////////////////////////////////////
#define GdiCharsetToSci(charset) ((int)(charset))
UINT CodePageFromCharSet(const UINT uCharSet);
//~UINT CharSetFromCodePage(const UINT uCodePage);


//==== UnSlash Functions ======================================================

size_t UnSlashA(LPSTR pchInOut, UINT cpEdit);
size_t UnSlashChar(LPWSTR pchInOut, WCHAR wch);

size_t SlashCtrlW(LPWSTR pchOutput, size_t cchOutLen, LPCWSTR pchInput);
size_t UnSlashCtrlW(LPWSTR pchInOut);

void TransformBackslashes(char *pszInput, bool bRegEx, UINT cpEdit, int *iReplaceMsg);
void TransformMetaChars(char *pszInput, bool bRegEx, int iEOLMode);


//==== Large Text Conversion ==================================================

#undef WC2MB_EX
#undef MB2WC_EX

#ifdef WC2MB_EX
ptrdiff_t WideCharToMultiByteEx(
  UINT CodePage, DWORD dwFlags, LPCWCH lpWideCharStr, ptrdiff_t cchWideChar,
  LPSTR lpMultiByteStr, ptrdiff_t cbMultiByte, LPCCH lpDefaultChar, LPBOOL lpUsedDefaultChar);
#else

__inline ptrdiff_t WideCharToMultiByteEx(
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

__inline ptrdiff_t MultiByteToWideCharEx(
  UINT CodePage, DWORD dwFlags, LPCCH lpMultiByteStr, ptrdiff_t cbMultiByte,
  LPWSTR lpWideCharStr, ptrdiff_t cchWideChar)
{
  return (ptrdiff_t)MultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr, (int)cbMultiByte,
                                        lpWideCharStr, (int)cchWideChar);
}

#endif 

// ============================================================================

inline int32_t bitmask32_n(unsigned short n)
{
  return ((n >= 32) ? 0 - ((int32_t)1) : (((int32_t)1) << n) - 1);
}

// ============================================================================

inline int wcscmp_s(const wchar_t* s1, const wchar_t* s2)
{
  return (s1 && s2) ? wcscmp(s1, s2) : ((s1 ? 1 : (s2 ? -1 : 0)));
}

inline int wcscoll_s(const wchar_t* s1, const wchar_t* s2)
{
  return (s1 && s2) ? wcscoll(s1, s2) : ((s1 ? 1 : (s2 ? -1 : 0)));
}

inline int wcsicmp_s(const wchar_t* s1, const wchar_t* s2)
{
  return (s1 && s2) ? _wcsicmp(s1, s2) : ((s1 ? 1 : (s2 ? -1 : 0)));
}

inline int wcsicoll_s(const wchar_t* s1, const wchar_t* s2)
{
  return (s1 && s2) ? _wcsicoll(s1, s2) : ((s1 ? 1 : (s2 ? -1 : 0)));
}

// ============================================================================

inline void SwabEx(char* src, char* dest, size_t n)
{
  static int const max = (INT_MAX - (INT_MAX % 2));

  if (n <= (size_t)max) {
    _swab(src, dest, (int)n);
  }
  else {
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


//==== MinimizeToTray Functions - see comments in Helpers.c ===================
bool GetDoAnimateMinimize(VOID);
VOID MinimizeWndToTray(HWND hWnd);
VOID RestoreWndFromTray(HWND hWnd);

// console helper from Print.cpp
//void RedirectIOToConsole();

//==== StrCut methods ===================

CHAR*  StrCutIA(CHAR* s,const CHAR* pattern);
WCHAR* StrCutIW(WCHAR* s,const WCHAR* pattern);
#if defined(UNICODE) || defined(_UNICODE)  
#define StrCutI StrCutIW
#else
#define StrCutI _StrCutIA
#endif


//==== StrNextTok methods ===================
CHAR*  StrNextTokA(CHAR* strg, const CHAR* tokens);
WCHAR* StrNextTokW(WCHAR* strg, const WCHAR* tokens);
#if defined(UNICODE) || defined(_UNICODE)  
#define StrNextTok StrNextTokW
#else
#define StrNextTok StrNextTokA
#endif

// ----------------------------------------------------------------------------

bool StrDelChrA(LPSTR pszSource, LPCSTR pCharsToRemove);

//==== StrSafe lstrlen() =======================================================
//inline size_t StringCchLenA(LPCSTR s, size_t n) { 
//  n = (n ? n : STRSAFE_MAX_CCH); size_t len; return (size_t)(!s ? 0 : (SUCCEEDED(StringCchLengthA(s, n, &len)) ? len : n));
//}
//inline size_t StringCchLenW(LPCWSTR s, size_t n) { 
//  n = (n ? n : STRSAFE_MAX_CCH); size_t len; return (size_t)(!s ? 0 : (SUCCEEDED(StringCchLengthW(s, n, &len)) ? len : n));
//}

inline size_t StringCchLenA(LPCSTR s, size_t n) {
  n = (n ? n : STRSAFE_MAX_CCH); return (s ? strnlen_s(s, n) : 0LL);
}
inline size_t StringCchLenW(LPCWSTR s, size_t n) {
  n = (n ? n : STRSAFE_MAX_CCH); return (s ? wcsnlen_s(s, n) : 0LL);
}

#if defined(UNICODE) || defined(_UNICODE)  
#define StringCchLen(s,n)  StringCchLenW((s),(n))
#else
#define StringCchLen(s,n)  StringCchLenA((s),(n))
#endif

// ----------------------------------------------------------------------------

inline char* StrEndA(const char* pStart, size_t siz) {
  // cppcheck-suppress cert-EXP05-C   // Attempt to cast away const - Intended(!)
  return (char*)(pStart + StringCchLenA(pStart, siz));
}

inline WCHAR* StrEndW(const WCHAR* pStart, size_t siz) {
  // cppcheck-suppress cert-EXP05-C   // Attempt to cast away const - Intended(!)
  return (WCHAR*)(pStart + StringCchLenW(pStart, siz));
}

#if defined(UNICODE) || defined(_UNICODE)  
#define StrEnd(s,n) StrEndW((s),(n))
#else
#define StrEnd(s,n) StrEndA((s),(n))
#endif

//==== StrSafe lstrcmp(),lstrcmpi() =============================================

// NOTE: !!! differences in AutoCompleteList depending compare functions (CRT vs. Shlwapi)) !!!

#define StringCchCompareNA(s1,l1,s2,l2)   StrCmpNA((s1),(s2),min_i((int)(l1),(int)(l2)))
//#define StringCchCompareNA(s1,l1,s2,l2)   strncmp((s1),(s2),min_s((l1),(l2)))
#define StringCchCompareXA(s1,s2)         StrCmpA((s1),(s2))
//#define StringCchCompareXA(s1,s2)         strcmp((s1),(s2))

#define StringCchCompareNIA(s1,l1,s2,l2)  StrCmpNIA((s1),(s2),min_i((int)(l1),(int)(l2)))
//#define StringCchCompareNIA(s1,l1,s2,l2)  _strnicmp((s1),(s2),min_s((l1),(l2)))
#define StringCchCompareXIA(s1,s2)        StrCmpIA((s1),(s2))
//#define StringCchCompareXIA(s1,s2)        _stricmp((s1),(s2))


#define StringCchCompareNW(s1,l1,s2,l2)   StrCmpNW((s1),(s2),min_i((int)(l1),(int)(l2)))
#define StringCchCompareXW(s1,s2)         StrCmpW((s1),(s2))

#define StringCchCompareNIW(s1,l1,s2,l2)  StrCmpNIW((s1),(s2),min_i((int)(l1),(int)(l2)))
#define StringCchCompareXIW(s1,s2)        StrCmpIW((s1),(s2))

#if defined(UNICODE) || defined(_UNICODE)  
#define StringCchCompareN(s1,l1,s2,l2)   StringCchCompareNW((s1),(l1),(s2),(l2))
#define StringCchCompareX(s1,s2)         StringCchCompareXW((s1),(s2))
#define StringCchCompareNI(s1,l1,s2,l2)  StringCchCompareNIW((s1),(l1),(s2),(l2))
#define StringCchCompareXI(s1,s2)        StringCchCompareXIW((s1),(s2))
#else
#define StringCchCompareN(s1,l1,s2,l2)   StringCchCompareNA((s1),(l1),(s2),(l2))
#define StringCchCompareX(s1,s2)         StringCchCompareXA((s1),(s2))
#define StringCchCompareNI(s1,l1,s2,l2)  StringCchCompareNIA((s1),(l1),(s2),(l2))
#define StringCchCompareXI(s1,s2)        StringCchCompareXIA((s1),(s2))
#endif

#ifdef __cplusplus
#undef NULL
#define NULL nullptr
#endif

// ----------------------------------------------------------------------------

// Is the character an octal digit?
#define IsOctalDigit(ch) (((ch) >= '0') && ((ch) <= '7'))

// no encoding for safe chars
__inline bool IsAlphaNumeric(WCHAR ch) {
  return
    ((ch >= L'0') && (ch <= L'9')) ||
    ((ch >= L'a') && (ch <= L'z')) ||
    ((ch >= L'A') && (ch <= L'Z'));
}

// If the character is an hexadecimal digit, get its value.
__inline int GetHexDigitA(char ch) {
  if (ch >= '0' && ch <= '9') { return ch - '0'; }
  if (ch >= 'A' && ch <= 'F') { return ch - 'A' + 10; }
  if (ch >= 'a' && ch <= 'f') { return ch - 'a' + 10; }
  return -1;
}

__inline int GetHexDigitW(WCHAR ch) {
  if (ch >= L'0' && ch <= L'9') { return ch - L'0'; }
  if (ch >= L'A' && ch <= L'F') { return ch - L'A' + 10; }
  if (ch >= L'a' && ch <= L'f') { return ch - L'a' + 10; }
  return -1;
}
// ----------------------------------------------------------------------------

void UrlEscapeEx(LPCWSTR lpURL, LPWSTR lpEscaped, DWORD* pcchEscaped, bool bEscReserved);
void UrlUnescapeEx(LPWSTR lpURL, LPWSTR lpUnescaped, DWORD* pcchUnescaped);

int ReadStrgsFromCSV(LPCWSTR wchCSVStrg, prefix_t sMatrix[], int iCount, int iLen, LPCWSTR sDefault);
size_t ReadVectorFromString(LPCWSTR wchStrg, int iVector[], size_t iCount, int iMin, int iMax, int iDefault, bool ordered);
size_t NormalizeColumnVector(LPSTR chStrg_in, LPWSTR wchStrg_out, size_t iCount);

inline bool Char2IntW(LPCWSTR str, int* value) {
  LPWSTR end;
  *value = (int)wcstol(str, &end, 10);
  return (str != end);
}
bool Char2FloatW(WCHAR* wnumber, float* fresult);
void Float2String(float fValue, LPWSTR lpszStrg, int cchSize);

#define MAX_ESCAPE_HEX_DIGIT 4
int Hex2Char(char* ch, int cnt);

// ----------------------------------------------------------------------------

inline bool PathIsExistingFile(LPCWSTR pszPath) { return (PathFileExists(pszPath) && !PathIsDirectory(pszPath)); }

// including <pathcch.h> and linking against pathcch.lib
// api-ms-win-core-path-l1-1-0.dll  library : Minimum supported client is Windows 8 :-/
// so switch back to previous (deprecated) methods:
inline HRESULT PathCchAppend(PWSTR p,size_t l,PCWSTR a)          { UNUSED(l); return (PathAppend(p,a) ? S_OK : E_FAIL); }
inline HRESULT PathCchCanonicalize(PWSTR p,size_t l,PCWSTR a)    { UNUSED(l); return (PathCanonicalize(p,a) ? S_OK : E_FAIL); }
inline HRESULT PathCchRenameExtension(PWSTR p,size_t l,PCWSTR a) { UNUSED(l); return (PathRenameExtension(p,a) ? S_OK : E_FAIL); }
inline HRESULT PathCchRemoveFileSpec(PWSTR p,size_t l)           { UNUSED(l); return (PathRemoveFileSpec(p) ? S_OK : E_FAIL); }

inline bool IsReadOnly(const DWORD dwFileAttr) {
  return ((dwFileAttr != INVALID_FILE_ATTRIBUTES) && (dwFileAttr & FILE_ATTRIBUTE_READONLY));
}

inline int PointSizeToFontHeight(const float fPtHeight, const HDC hdc) {
  return -MulDiv(float2int(fPtHeight * 100.0f), GetDeviceCaps(hdc, LOGPIXELSY), 7200);
}

// ----------------------------------------------------------------------------

#endif //_NP3_HELPERS_H_

///   End of Helpers.h   ///
