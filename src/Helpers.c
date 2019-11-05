// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Helpers.c                                                                   *
*   General helper functions                                                  *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*	Parts taken from SciTE, (c) Neil Hodgson                                  *
*	MinimizeToTray, (c) 2000 Matthew Ellis                                    *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2019   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

#include "Helpers.h"

#include <shlobj.h>
#include <shellapi.h>

//#include <pathcch.h>

#include "resource.h"
#include "Edit.h"
#include "Encoding.h"
#include "MuiLanguage.h"
#include "Notepad3.h"
#include "Config/Config.h"

#include "Scintilla.h"

//=============================================================================


#if (defined(_DEBUG) || defined(DEBUG)) && !defined(NDEBUG)
void DbgLog(const char *fmt, ...) {
  char buf[1024] = "";
  va_list va;
  va_start(va, fmt);
  wvsprintfA(buf, fmt, va);
  va_end(va);
  OutputDebugStringA(buf);
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
      while (*p != '\0') { *p++ = *q++; }
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
      WCHAR* q = p + StringCchLen(pattern,0);
      while (*p != L'\0') { *p++ = *q++; }
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
  if (!pszSource || !pCharsToRemove)
    return false;

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

static OSVERSIONINFOEX s_OSversion = { 0 };

static void _GetTrueWindowsVersion()
{
  // clear
  ZeroMemory(&s_OSversion, sizeof(OSVERSIONINFOEX));
  s_OSversion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

  // Function pointer to driver function
  void (WINAPI *pRtlGetVersion)(PRTL_OSVERSIONINFOW lpVersionInformation) = NULL;

  // load the System-DLL
  HINSTANCE hNTdllDll = LoadLibrary(L"ntdll.dll");

  if (hNTdllDll != NULL)
  {
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

static DWORD _Win10BuildToReleaseId(DWORD build)
{
  if (build >= 18362) {
    return 1903;
  }
  else if (build >= 17763) {
    return 1809;
  }
  else if (build >= 17134) {
    return 1803;
  }
  else if (build >= 16299) {
    return 1709;
  }
  else if (build >= 15063) {
    return 1809;
  }
  else if (build >= 14393) {
    return 1607;
  }
  else {
    return 1507;
  }
}
// ----------------------------------------------------------------------------

void GetWinVersionString(LPWSTR szVersionStr, size_t cchVersionStr)
{
  StringCchCopy(szVersionStr, cchVersionStr, L"OS Version: Windows ");
  
  if (IsWin10()) {
    StringCchCat(szVersionStr, cchVersionStr, IsWinServer() ? L"Server 2016 " : L"10 ");
  }
  else if (IsWin81()) {
    StringCchCat(szVersionStr, cchVersionStr, IsWinServer() ? L"Server 2012 R2 " : L"8.1");
  }
  else if (IsWin8()) {
    StringCchCat(szVersionStr, cchVersionStr, IsWinServer() ? L"Server 2012 " : L"8");
  }
  else if (IsWin71()) {
    StringCchCat(szVersionStr, cchVersionStr, IsWinServer() ? L"Server 2008 R2 " : L"7 (SP1)");
  }
  else if (IsWin7()) {
    StringCchCat(szVersionStr, cchVersionStr, IsWinServer() ? L"Server 2008 " : L"7");
  }
  else {
    StringCchCat(szVersionStr, cchVersionStr, IsWinServer() ? L"Unkown Server " : L"?");
  }
  
  if (IsWin10()) {
    WCHAR win10ver[80] = { L'\0' };
    if (s_OSversion.dwOSVersionInfoSize == 0) { _GetTrueWindowsVersion(); }
    DWORD const build = s_OSversion.dwBuildNumber;
    StringCchPrintf(win10ver, COUNTOF(win10ver), L" Version %i (Build %i)", 
      _Win10BuildToReleaseId(build) , build);
    StringCchCat(szVersionStr, cchVersionStr, win10ver);
  }
}


//=============================================================================
//
//  SetClipboardWchTextW()
//
bool SetClipboardTextW(HWND hwnd, LPCWSTR pszTextW, size_t cchTextW)
{
  if (!OpenClipboard(hwnd)) { return false; }
  HANDLE hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (cchTextW + 1) * sizeof(WCHAR));
  if (hData) {
    WCHAR* pszNew = GlobalLock(hData);
    if (pszNew) {
      StringCchCopy(pszNew, cchTextW + 1, pszTextW);
      GlobalUnlock(hData);
      EmptyClipboard();
      SetClipboardData(CF_UNICODETEXT, hData);
    }
    CloseClipboard();
    // cppcheck-suppress memleak // ClipBoard is owner now
    return true; 
  }
  CloseClipboard();
  return false;
}


//=============================================================================
//
//  GetCurrentDPI()
//
DPI_T GetCurrentDPI(HWND hwnd) {

  DPI_T curDPI = { 0, 0 };

  if (IsWin10()) {
    HMODULE const hModule = GetModuleHandle(L"user32.dll");
    if (hModule) {
      FARPROC const pfnGetDpiForWindow = GetProcAddress(hModule, "GetDpiForWindow");
      if (pfnGetDpiForWindow) {
        curDPI.x = curDPI.y = (UINT)pfnGetDpiForWindow(hwnd);
      }
    }
  }

  if ((curDPI.x == 0) && IsWin81()) {
    HMODULE hShcore = LoadLibrary(L"shcore.dll");
    if (hShcore) {
      FARPROC const pfnGetDpiForMonitor = GetProcAddress(hShcore, "GetDpiForMonitor");
      if (pfnGetDpiForMonitor) {
        HMONITOR const hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
        UINT dpiX = 0, dpiY = 0;
        if (pfnGetDpiForMonitor(hMonitor, 0 /* MDT_EFFECTIVE_DPI */, &dpiX, &dpiY) == S_OK) {
          curDPI.x = dpiX;
          curDPI.y = dpiY;
        }
      }
      FreeLibrary(hShcore);
    }
  }

  if (curDPI.x == 0) {
    HDC hDC = GetDC(hwnd);
    curDPI.x = GetDeviceCaps(hDC, LOGPIXELSX);
    curDPI.y = GetDeviceCaps(hDC, LOGPIXELSY);
    ReleaseDC(hwnd, hDC);
  }

  curDPI.x = max_u(curDPI.x, USER_DEFAULT_SCREEN_DPI);
  curDPI.y = max_u(curDPI.y, USER_DEFAULT_SCREEN_DPI);
  return curDPI;
}


//=============================================================================
//
//  GetCurrentPPI()
//  (font size) points per inch
//
DPI_T GetCurrentPPI(HWND hwnd) {
  HDC const hDC = GetDC(hwnd);
  DPI_T ppi;
  ppi.x = max_u(GetDeviceCaps(hDC, LOGPIXELSX), USER_DEFAULT_SCREEN_DPI);
  ppi.y = max_u(GetDeviceCaps(hDC, LOGPIXELSY), USER_DEFAULT_SCREEN_DPI);
  ReleaseDC(hwnd, hDC);
  return ppi;
}

/*
if (!bSucceed) {
  NONCLIENTMETRICS ncm;
  ncm.cbSize = sizeof(NONCLIENTMETRICS);
  SystemParametersInfo(SPI_GETNONCLIENTMETRICS,sizeof(NONCLIENTMETRICS),&ncm,0);
  if (ncm.lfMessageFont.lfHeight < 0)
  ncm.lfMessageFont.lfHeight = -ncm.lfMessageFont.lfHeight;
  *wSize = (WORD)MulDiv(ncm.lfMessageFont.lfHeight,72,iLogPixelsY);
  if (*wSize == 0)
    *wSize = 8;
}*/


//=============================================================================
//
//  GetSystemMetricsEx()
//  get system metrix for current DPI 
// https://docs.microsoft.com/de-de/windows/desktop/api/winuser/nf-winuser-getsystemmetricsfordpi
//
int GetSystemMetricsEx(int nValue) {

  return ScaleIntToCurrentDPI(GetSystemMetrics(nValue));
}


//=============================================================================
//
//  UpdateWindowLayoutForDPI()
//
void UpdateWindowLayoutForDPI(HWND hWnd, int x_96dpi, int y_96dpi, int w_96dpi, int h_96dpi)
{
  // only update yet
  SetWindowPos(hWnd, hWnd, x_96dpi, y_96dpi, w_96dpi, h_96dpi,
    SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOREPOSITION );

  // TODO: ...
#if 0
  DPI_T const wndDPI = GetCurrentDPI(hWnd);

  int dpiScaledX = MulDiv(x_96dpi, wndDPI.x, 96);
  int dpiScaledY = MulDiv(y_96dpi, wndDPI.y, 96);
  int dpiScaledWidth = MulDiv(w_96dpi, wndDPI.y, 96);
  int dpiScaledHeight = MulDiv(h_96dpi, wndDPI.y, 96);

  SetWindowPos(hWnd, hWnd, dpiScaledX, dpiScaledY, dpiScaledWidth, dpiScaledY, SWP_NOZORDER | SWP_NOACTIVATE);
#endif

}


//=============================================================================
//
//  ResizeImageForCurrentDPI()
//
HBITMAP ResizeImageForCurrentDPI(HBITMAP hbmp) 
{
  if (hbmp) {
    BITMAP bmp;
    if (GetObject(hbmp, sizeof(BITMAP), &bmp)) {
      UINT const uDPIUnit = (UINT)(USER_DEFAULT_SCREEN_DPI / 2U);
      UINT uDPIScaleFactor = max_u(1U, (UINT)MulDiv(bmp.bmHeight, 8, 64));
      UINT const uDPIBase = (uDPIScaleFactor - 1U) * uDPIUnit;
      if (Globals.CurrentDPI.x > (uDPIBase + uDPIUnit)) {
        int width = MulDiv(bmp.bmWidth, (Globals.CurrentDPI.x - uDPIBase), uDPIUnit);
        int height = MulDiv(bmp.bmHeight, (Globals.CurrentDPI.y - uDPIBase), uDPIUnit);
        HBITMAP hCopy = CopyImage(hbmp, IMAGE_BITMAP, width, height, LR_CREATEDIBSECTION | LR_COPYRETURNORG | LR_COPYDELETEORG);
        if (hCopy) {
          hbmp = hCopy;
        }
      }
    }
  }
  return hbmp;
}


//=============================================================================
//
//  PrivateSetCurrentProcessExplicitAppUserModelID()
//
HRESULT PrivateSetCurrentProcessExplicitAppUserModelID(PCWSTR AppID)
{
  if (StrIsEmpty(AppID)) { return(S_OK); }

  if (StringCchCompareXI(AppID, L"(default)") == 0) { return(S_OK); }

  return SetCurrentProcessExplicitAppUserModelID(AppID);
}


//=============================================================================
//
//  IsElevated()
//
bool IsElevated() {

  bool bIsElevated = false;
  HANDLE hToken = NULL;

  if (!IsVista())
    return false;

  if (OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&hToken)) {

    TOKEN_ELEVATION te;
    DWORD expectedRetVal = sizeof(TOKEN_ELEVATION);
    DWORD dwReturnLength = 0;

    if (GetTokenInformation(hToken,TokenElevation,&te,expectedRetVal,&dwReturnLength)) {
        if (dwReturnLength == expectedRetVal)
          bIsElevated = (bool)te.TokenIsElevated;
    }
    if (hToken)
      CloseHandle(hToken);
  }
  return bIsElevated;
}


//=============================================================================
//
//  IsUserAdmin()
//
// Routine Description: This routine returns true if the caller's
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
  PSID AdminGroup;
  SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
  BOOL bIsAdmin = AllocateAndInitializeSid(&NtAuthority,2,
    SECURITY_BUILTIN_DOMAIN_RID,DOMAIN_ALIAS_RID_ADMINS,0,0,0,0,0,0,&AdminGroup);
  if (bIsAdmin) {
    if (!CheckTokenMembership(NULL,AdminGroup,&bIsAdmin))
      bIsAdmin = false;
    FreeSid(AdminGroup);
  }
  return(bIsAdmin);
}



//=============================================================================
//
//  SetExplorerTheme()
//
//bool SetExplorerTheme(HWND hwnd)
//{
//  FARPROC pfnSetWindowTheme;
//
//  if (IsVista()) {
//    if (hLocalModUxTheme) {
//      pfnSetWindowTheme = GetProcAddress(hLocalModUxTheme,"SetWindowTheme");
//
//      if (pfnSetWindowTheme)
//        return (S_OK == pfnSetWindowTheme(hwnd,L"Explorer",NULL));
//    }
//  }
//  return false;
//}


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

  return(
    ((abs((3*r1 + 5*g1 + 1*b1) - (3*r2 + 6*g2 + 1*b2))) >= 400) ||
    ((abs(r1-r2) + abs(b1-b2) + abs(g1-g2)) >= 400));
}


//=============================================================================
//
//  IsFontAvailable()
//  Test if a certain font is installed on the system
//
int CALLBACK EnumFontsProc(CONST LOGFONT *plf,CONST TEXTMETRIC *ptm,DWORD FontType,LPARAM lParam)
{
  *((PBOOL)lParam) = true;
  UNUSED(plf);
  UNUSED(ptm);
  UNUSED(FontType);
  return 0;
}

bool IsFontAvailable(LPCWSTR lpszFontName)
{
  BOOL fFound = FALSE;

  HDC hDC = GetDC(NULL);
  EnumFonts(hDC,lpszFontName,EnumFontsProc,(LPARAM)&fFound);
  ReleaseDC(NULL,hDC);

  return (bool)(fFound);
}


//=============================================================================
//
//  IsCmdEnabled()
//
bool IsCmdEnabled(HWND hwnd,UINT uId)
{

  HMENU hmenu;
  UINT ustate;

  hmenu = GetMenu(hwnd);

  SendMessage(hwnd,WM_INITMENU,(WPARAM)hmenu,0);

  ustate = GetMenuState(hmenu,uId,MF_BYCOMMAND);

  if (ustate == 0xFFFFFFFF) {
    return true;
  }
  return (!(ustate & (MF_GRAYED|MF_DISABLED)));
}

//=============================================================================
//
//  GetKnownFolderPath()
//
bool GetKnownFolderPath(REFKNOWNFOLDERID rfid, LPWSTR lpOutPath, size_t cchCount)
{
  //const DWORD dwFlags = (KF_FLAG_DEFAULT_PATH | KF_FLAG_NOT_PARENT_RELATIVE | KF_FLAG_NO_ALIAS);
  const DWORD dwFlags = KF_FLAG_NO_ALIAS;

  PWSTR pszPath = NULL;
  HRESULT hr = SHGetKnownFolderPath(rfid, dwFlags, NULL, &pszPath);
  if (SUCCEEDED(hr) && pszPath) {
    StringCchCopy(lpOutPath, cchCount, pszPath);
    CoTaskMemFree(pszPath);
    return true;
  }
  return false;
}

//=============================================================================
//
//  PathRelativeToApp()
//
void PathRelativeToApp(
  LPWSTR lpszSrc,LPWSTR lpszDest,int cchDest,bool bSrcIsFile,
  bool bUnexpandEnv,bool bUnexpandMyDocs) {

  WCHAR wchAppPath[MAX_PATH] = { L'\0' };
  WCHAR wchWinDir[MAX_PATH] = { L'\0' };
  WCHAR wchUserFiles[MAX_PATH] = { L'\0' };
  WCHAR wchPath[MAX_PATH] = { L'\0' };
  WCHAR wchResult[MAX_PATH] = { L'\0' };
  DWORD dwAttrTo = (bSrcIsFile) ? 0 : FILE_ATTRIBUTE_DIRECTORY;

  GetModuleFileName(NULL,wchAppPath,COUNTOF(wchAppPath));
  PathCanonicalizeEx(wchAppPath,MAX_PATH);
  PathCchRemoveFileSpec(wchAppPath,COUNTOF(wchAppPath));
  (void)GetWindowsDirectory(wchWinDir,COUNTOF(wchWinDir));
  GetKnownFolderPath(&FOLDERID_Documents, wchUserFiles, COUNTOF(wchUserFiles));

  if (bUnexpandMyDocs &&
      !PathIsRelative(lpszSrc) &&
      !PathIsPrefix(wchUserFiles,wchAppPath) &&
       PathIsPrefix(wchUserFiles,lpszSrc) &&
       PathRelativePathTo(wchPath,wchUserFiles,FILE_ATTRIBUTE_DIRECTORY,lpszSrc,dwAttrTo)) {
    StringCchCopy(wchUserFiles,COUNTOF(wchUserFiles),L"%CSIDL:MYDOCUMENTS%");
    PathCchAppend(wchUserFiles,COUNTOF(wchUserFiles),wchPath);
    StringCchCopy(wchPath,COUNTOF(wchPath),wchUserFiles);
  }
  else if (PathIsRelative(lpszSrc) || PathCommonPrefix(wchAppPath,wchWinDir,NULL))
    StringCchCopyN(wchPath,COUNTOF(wchPath),lpszSrc,COUNTOF(wchPath));
  else {
    if (!PathRelativePathTo(wchPath,wchAppPath,FILE_ATTRIBUTE_DIRECTORY,lpszSrc,dwAttrTo))
      StringCchCopyN(wchPath,COUNTOF(wchPath),lpszSrc,COUNTOF(wchPath));
  }

  if (bUnexpandEnv) {
    if (!PathUnExpandEnvStrings(wchPath,wchResult,COUNTOF(wchResult)))
      StringCchCopyN(wchResult,COUNTOF(wchResult),wchPath,COUNTOF(wchResult));
  }
  else
    StringCchCopyN(wchResult,COUNTOF(wchResult),wchPath,COUNTOF(wchResult));

  int cchLen = (cchDest == 0) ? MAX_PATH : cchDest;
  if (lpszDest == NULL || lpszSrc == lpszDest)
    StringCchCopyN(lpszSrc,cchLen,wchResult,cchLen);
  else
    StringCchCopyN(lpszDest,cchLen,wchResult,cchLen);
}


//=============================================================================
//
//  PathAbsoluteFromApp()
//
void PathAbsoluteFromApp(LPWSTR lpszSrc,LPWSTR lpszDest,int cchDest,bool bExpandEnv) {

  WCHAR wchPath[MAX_PATH] = { L'\0'};
  WCHAR wchResult[MAX_PATH] = { L'\0'};

  if (lpszSrc == NULL) {
    ZeroMemory(lpszDest, (cchDest == 0) ? MAX_PATH : cchDest);
    return;
  }

  if (StrCmpNI(lpszSrc,L"%CSIDL:MYDOCUMENTS%",CSTRLEN("%CSIDL:MYDOCUMENTS%")) == 0) {
    GetKnownFolderPath(&FOLDERID_Documents, wchPath, COUNTOF(wchPath));
    PathCchAppend(wchPath,COUNTOF(wchPath),lpszSrc+CSTRLEN("%CSIDL:MYDOCUMENTS%"));
  }
  else {
    if (lpszSrc) {
      StringCchCopyN(wchPath,COUNTOF(wchPath),lpszSrc,COUNTOF(wchPath));
    }
  }

  if (bExpandEnv)
    ExpandEnvironmentStringsEx(wchPath,COUNTOF(wchPath));

  if (PathIsRelative(wchPath)) {
    GetModuleFileName(NULL,wchResult,COUNTOF(wchResult));
    PathCanonicalizeEx(wchResult, COUNTOF(wchResult));
    PathCchRemoveFileSpec(wchResult, COUNTOF(wchResult));
    PathCchAppend(wchResult,COUNTOF(wchResult),wchPath);
  }
  else
    StringCchCopyN(wchResult,COUNTOF(wchResult),wchPath,COUNTOF(wchPath));

  PathCanonicalizeEx(wchResult,MAX_PATH);
  if (PathGetDriveNumber(wchResult) != -1)
    CharUpperBuff(wchResult,1);

  if (lpszDest == NULL || lpszSrc == lpszDest)
    StringCchCopyN(lpszSrc,((cchDest == 0) ? MAX_PATH : cchDest),wchResult,COUNTOF(wchResult));
  else
    StringCchCopyN(lpszDest,((cchDest == 0) ? MAX_PATH : cchDest),wchResult,COUNTOF(wchResult));
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
bool PathIsLnkFile(LPCWSTR pszPath)
{
  WCHAR tchResPath[MAX_PATH] = { L'\0' };

  if (!pszPath || !*pszPath)
    return false;

  if (StringCchCompareXI(PathFindExtension(pszPath), L".lnk")) {
    return false;
  }
  return PathGetLnkPath(pszPath,tchResPath,COUNTOF(tchResPath));
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
bool PathGetLnkPath(LPCWSTR pszLnkFile,LPWSTR pszResPath,int cchResPath)
{

  IShellLink       *psl;
  WIN32_FIND_DATA  fd;
  bool             bSucceeded = false;

  if (SUCCEEDED(CoCreateInstance(&CLSID_ShellLink,NULL,
                                 CLSCTX_INPROC_SERVER,
                                 &IID_IShellLink,(void**)&psl)))
  {
    IPersistFile *ppf;

    if (SUCCEEDED(psl->lpVtbl->QueryInterface(psl,&IID_IPersistFile,(void**)&ppf)))
    {
      WORD wsz[MAX_PATH] = { L'\0' };

      /*MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,pszLnkFile,-1,wsz,MAX_PATH);*/
      StringCchCopy(wsz,COUNTOF(wsz),pszLnkFile);

      if (SUCCEEDED(ppf->lpVtbl->Load(ppf,wsz,STGM_READ)))
      {
        if (NOERROR == psl->lpVtbl->GetPath(psl,pszResPath,cchResPath,&fd,0))
          bSucceeded = true;
      }
      ppf->lpVtbl->Release(ppf);
    }
    psl->lpVtbl->Release(psl);
  }

  // This additional check seems reasonable
  if (StrIsEmpty(pszResPath)) {
    bSucceeded = false;
  }

  if (bSucceeded) {
    ExpandEnvironmentStringsEx(pszResPath,cchResPath);
    PathCanonicalizeEx(pszResPath,cchResPath);
  }

  return(bSucceeded);

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
bool PathIsLnkToDirectory(LPCWSTR pszPath,LPWSTR pszResPath,int cchResPath)
{
  if (PathIsLnkFile(pszPath)) {
    WCHAR tchResPath[MAX_PATH] = { L'\0' };
    if (PathGetLnkPath(pszPath, tchResPath, sizeof(WCHAR)*COUNTOF(tchResPath))) {
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
bool PathCreateDeskLnk(LPCWSTR pszDocument)
{

  WCHAR tchExeFile[MAX_PATH] = { L'\0' };
  WCHAR tchDocTemp[MAX_PATH] = { L'\0' };
  WCHAR tchArguments[MAX_PATH+16] = { L'\0' };
  WCHAR tchLinkDir[MAX_PATH] = { L'\0' };
  WCHAR tchDescription[64] = { L'\0' };

  WCHAR tchLnkFileName[MAX_PATH] = { L'\0' };

  IShellLink *psl;
  bool bSucceeded = false;
  BOOL fMustCopy;

  if (StrIsEmpty(pszDocument)) { return true; }

  // init strings
  GetModuleFileName(NULL,tchExeFile,COUNTOF(tchExeFile));

  StringCchCopy(tchDocTemp,COUNTOF(tchDocTemp),pszDocument);
  PathQuoteSpaces(tchDocTemp);

  StringCchCopy(tchArguments,COUNTOF(tchArguments),L"-n ");
  StringCchCat(tchArguments,COUNTOF(tchArguments),tchDocTemp);

  GetKnownFolderPath(&FOLDERID_Desktop, tchLinkDir, COUNTOF(tchLinkDir));

  GetLngString(IDS_MUI_LINKDESCRIPTION,tchDescription,COUNTOF(tchDescription));

  // Try to construct a valid filename...
  if (!SHGetNewLinkInfo(pszDocument,tchLinkDir,tchLnkFileName,&fMustCopy,SHGNLI_PREFIXNAME))
    return false;

  if (SUCCEEDED(CoCreateInstance(&CLSID_ShellLink,NULL,
                                 CLSCTX_INPROC_SERVER,
                                 &IID_IShellLink,(void**)&psl)))
  {
    IPersistFile *ppf;

    if (SUCCEEDED(psl->lpVtbl->QueryInterface(psl,&IID_IPersistFile,(void**)&ppf)))
    {
      WORD wsz[MAX_PATH] = { L'\0' };

      /*MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,tchLnkFileName,-1,wsz,MAX_PATH);*/
      StringCchCopy(wsz,COUNTOF(wsz),tchLnkFileName);

      psl->lpVtbl->SetPath(psl,tchExeFile);
      psl->lpVtbl->SetArguments(psl,tchArguments);
      psl->lpVtbl->SetDescription(psl,tchDescription);

      if (SUCCEEDED(ppf->lpVtbl->Save(ppf,wsz,true)))
        bSucceeded = true;

      ppf->lpVtbl->Release(ppf);
    }
    psl->lpVtbl->Release(psl);
  }

  return(bSucceeded);

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
bool PathCreateFavLnk(LPCWSTR pszName,LPCWSTR pszTarget,LPCWSTR pszDir)
{

  WCHAR tchLnkFileName[MAX_PATH] = { L'\0' };

  IShellLink *psl;
  bool bSucceeded = false;

  if (StrIsEmpty(pszName)) { return true; }

  StringCchCopy(tchLnkFileName,COUNTOF(tchLnkFileName),pszDir);
  PathCchAppend(tchLnkFileName,COUNTOF(tchLnkFileName),pszName);
  StringCchCat(tchLnkFileName,COUNTOF(tchLnkFileName),L".lnk");

  if (PathFileExists(tchLnkFileName))
    return false;

  if (SUCCEEDED(CoCreateInstance(&CLSID_ShellLink,NULL,
                                 CLSCTX_INPROC_SERVER,
                                 &IID_IShellLink,(void**)&psl)))
  {
    IPersistFile *ppf;

    if (SUCCEEDED(psl->lpVtbl->QueryInterface(psl,&IID_IPersistFile,(void**)&ppf)))
    {
      WORD wsz[MAX_PATH] = { L'\0' };

      /*MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,tchLnkFileName,-1,wsz,MAX_PATH);*/
      StringCchCopy(wsz,COUNTOF(wsz),tchLnkFileName);

      psl->lpVtbl->SetPath(psl,pszTarget);

      if (SUCCEEDED(ppf->lpVtbl->Save(ppf,wsz,true)))
        bSucceeded = true;

      ppf->lpVtbl->Release(ppf);
    }
    psl->lpVtbl->Release(psl);
  }

  return(bSucceeded);

}


//=============================================================================
//
//  StrLTrimI()
//
bool StrLTrimI(LPWSTR pszSource,LPCWSTR pszTrimChars)
{
  if (!pszSource || !*pszSource) { return false; }

  LPWSTR psz = pszSource;
  while (StrChrI(pszTrimChars, *psz)) { ++psz; }

  MoveMemory(pszSource, psz, sizeof(WCHAR)*(StringCchLenW(psz,0) + 1));

  return (psz != pszSource);
}

//=============================================================================
//
//  StrRTrimI()
//
bool StrRTrimI(LPWSTR pszSource, LPCWSTR pszTrimChars)
{
  if (!pszSource || !*pszSource) { return false; }
  size_t const length = StringCchLenW(pszSource, 0);
  
  size_t len = length;
  while ((len > 0) && StrChrI(pszTrimChars, pszSource[--len])) { pszSource[len] = L'\0'; }

  return (length != len);
}


#if 0

//=============================================================================
//
//  TrimStringA()
//
bool TrimStringA(LPSTR lpString)
{
  if (!lpString || !*lpString) { return false; }

  // Trim left
  LPSTR psz = lpString;
  while (*psz == ' ') { psz = CharNextA(psz); }

  MoveMemory(lpString, psz, sizeof(CHAR)*(StringCchLenA(psz,0) + 1));

  // Trim right
  psz = StrEndA(lpString,0);
  while (*(psz = CharPrevA(lpString, psz)) == ' ') { *psz = '\0'; }

  return true;
}



//=============================================================================
//
//  TrimStringW()
//
bool TrimStringW(LPWSTR lpString)
{
  if (!lpString || !*lpString) { return false; }

  // Trim left
  LPWSTR psz = lpString;
  while (*psz == L' ') { psz = CharNextW(psz); }

  MoveMemory(lpString,psz,sizeof(WCHAR)*(StringCchLenA(psz,0) + 1));

  // Trim right
  psz = StrEndW(lpString,0);
  while (*(psz = CharPrevW(lpString, psz)) == L' ') { *psz = L'\0'; }

  return true;
}

#endif


//=============================================================================
//
//  ExtractFirstArgument()
//
bool ExtractFirstArgument(LPCWSTR lpArgs, LPWSTR lpArg1, LPWSTR lpArg2, int len)
{
  StringCchCopy(lpArg1, len, lpArgs);

  if (StrIsEmpty(lpArg1)) { return false; }
  if (lpArg2) { *lpArg2 = L'\0'; }

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
  }
  else {
    psz = StrChr(lpArg1, L' ');
  }
  if (psz) {
    *psz = L'\0';
    if (lpArg2) {
      StringCchCopy(lpArg2, len, psz + 1);
    }
  }
  TrimSpcW(lpArg1);

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
  while (psz != lpFilter)
  {
    if (*(psz = CharPrev(lpFilter,psz)) == L'|')
      *psz = L'\0';
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
    if ((*CharPrev(lpsz,c) == L':') && (*CharNext(c) == L'/'))
      c += 2;
    else
      *c = L'\\';

    c = StrChr(c, L'/');  // next
  }
}


//=============================================================================
//
//  ExpandEnvironmentStringsEx()
//
//  Adjusted for Windows 95
//
void ExpandEnvironmentStringsEx(LPWSTR lpSrc, DWORD dwSrc)
{
  WCHAR szBuf[HUGE_BUFFER];
  if (ExpandEnvironmentStrings(lpSrc, szBuf, COUNTOF(szBuf))) {
    StringCchCopyN(lpSrc, dwSrc, szBuf, COUNTOF(szBuf));
  }
}


//=============================================================================
//
//  PathCanonicalizeEx()
//
//
void PathCanonicalizeEx(LPWSTR lpszPath, DWORD cchBuffer)
{
  WCHAR szDst[MAX_PATH] = { L'\0' };
  if (PathCchCanonicalize(szDst, MAX_PATH, lpszPath) == S_OK) {
    StringCchCopy(lpszPath, cchBuffer, szDst);
  }
}


//=============================================================================
//
//  GetLongPathNameEx()
//
//
DWORD GetLongPathNameEx(LPWSTR lpszPath, DWORD cchBuffer)
{
  DWORD const dwRet = GetLongPathName(lpszPath, lpszPath, cchBuffer);
  if (dwRet) {
    if (PathGetDriveNumber(lpszPath) != -1) {
      CharUpperBuff(lpszPath, 1);
    }
  }
  return dwRet;
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
  if (PathFileExists(pszPath))
  {
    DWORD_PTR dw = SHGetFileInfo(pszPath, dwFileAttributes, psfi, cbFileInfo, uFlags);
    if (StringCchLenW(psfi->szDisplayName, COUNTOF(psfi->szDisplayName)) < StringCchLen(PathFindFileName(pszPath), MAX_PATH))
      StringCchCat(psfi->szDisplayName, COUNTOF(psfi->szDisplayName), PathFindExtension(pszPath));
    return(dw);
  }

  DWORD_PTR dw = SHGetFileInfo(pszPath, FILE_ATTRIBUTE_NORMAL, psfi, cbFileInfo, uFlags | SHGFI_USEFILEATTRIBUTES);
  if (StringCchLenW(psfi->szDisplayName, COUNTOF(psfi->szDisplayName)) < StringCchLen(PathFindFileName(pszPath), MAX_PATH)) {
    StringCchCat(psfi->szDisplayName, COUNTOF(psfi->szDisplayName), PathFindExtension(pszPath));
  }
  return(dw);
}


//=============================================================================
//
//  PathResolveDisplayName()
//
void PathGetDisplayName(LPWSTR lpszDestPath, DWORD cchDestBuffer, LPCWSTR lpszSourcePath)
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
DWORD NormalizePathEx(LPWSTR lpszPath, DWORD cchBuffer, bool bRealPath, bool bSearchPathIfRelative)
{
  WCHAR tmpFilePath[MAX_PATH] = { L'\0' };

  StringCchCopyN(tmpFilePath, COUNTOF(tmpFilePath), lpszPath, cchBuffer);
  ExpandEnvironmentStringsEx(tmpFilePath, COUNTOF(tmpFilePath));
  
  PathUnquoteSpaces(tmpFilePath);

  if (PathIsRelative(tmpFilePath)) 
  {
    StringCchCopyN(lpszPath, cchBuffer, Globals.WorkingDirectory, COUNTOF(Globals.WorkingDirectory));
    PathCchAppend(lpszPath, cchBuffer, tmpFilePath);
    if (bSearchPathIfRelative) {
      if (!PathFileExists(lpszPath)) {
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
    HANDLE hFile = CreateFile(lpszPath, // file to open
      GENERIC_READ,                     // open for reading
      FILE_SHARE_READ,                  // share for reading
      NULL,                             // default security
      OPEN_EXISTING,                    // existing file only
      FILE_ATTRIBUTE_NORMAL,            // normal file
      NULL);                            // no attr. template

    if (hFile != INVALID_HANDLE_VALUE) {
      if (GetFinalPathNameByHandleW(hFile, tmpFilePath,
        COUNTOF(tmpFilePath), FILE_NAME_OPENED) > 0)
      {
        if (StrCmpN(tmpFilePath, L"\\\\?\\", 4) == 0) {
          WCHAR* p = tmpFilePath + 4;
          if (StrCmpN(p, L"UNC\\", 4) == 0) {
            p += 2;
            *p = L'\\';
          }
          StringCchCopy(lpszPath, cchBuffer, p);
        }
      }
    }
    CloseHandle(hFile);
  }

  return (DWORD)StringCchLen(lpszPath, cchBuffer);
}


//=============================================================================
//
//  FormatNumberStr()
//
size_t FormatNumberStr(LPWSTR lpNumberStr, size_t cch, int fixedWidth)
{
  static WCHAR szSep[5] = { L'\0' };
  static WCHAR szGrp[11] = { L'\0' };
  static int iPlace[4] = {-1,-1,-1,-1};

  if (StrIsEmpty(lpNumberStr)) { return 0; }

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
        if (iPlace[i] == 0) { --i; } else if (iPlace[i] < 0) { break; }
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
//  A2W: Convert Dialog Item Text form Unicode to UTF-8 and vice versa
//
UINT GetDlgItemTextW2MB(HWND hDlg, int nIDDlgItem, LPSTR lpString, int nMaxCount)
{
  WCHAR wsz[FNDRPL_BUFFER] = L"";
  UINT uRet = GetDlgItemTextW(hDlg, nIDDlgItem, wsz, COUNTOF(wsz));
  ZeroMemory(lpString,nMaxCount);
  WideCharToMultiByte(Encoding_SciCP, 0, wsz, -1, lpString, nMaxCount - 1, NULL, NULL);
  return uRet;
}

UINT SetDlgItemTextMB2W(HWND hDlg, int nIDDlgItem, LPSTR lpString)
{
  WCHAR wsz[FNDRPL_BUFFER] = L"";
  MultiByteToWideChar(Encoding_SciCP, 0, lpString, -1, wsz, FNDRPL_BUFFER);
  return SetDlgItemTextW(hDlg, nIDDlgItem, wsz);
}

LRESULT ComboBox_AddStringMB2W(HWND hwnd, LPCSTR lpString)
{
  WCHAR wsz[FNDRPL_BUFFER] = L"";
  MultiByteToWideChar(Encoding_SciCP, 0, lpString, -1, wsz, FNDRPL_BUFFER);
  return SendMessageW(hwnd, CB_ADDSTRING, 0, (LPARAM)wsz);
}


//=============================================================================
//
//  CodePageFromCharSet()
//
UINT CodePageFromCharSet(const UINT uCharSet)
{
  if (ANSI_CHARSET == uCharSet) {
    if (Globals.uConsoleCodePage != 0) {
      return Globals.uConsoleCodePage;
    }
    CPINFOEX cpinfo; ZeroMemory(&cpinfo, sizeof(CPINFOEX));
    if (GetCPInfoEx(CP_THREAD_ACP, 0, &cpinfo)) {
      return cpinfo.CodePage;
    }
  }
  else {
    CHARSETINFO ci; ZeroMemory(&ci, sizeof(CHARSETINFO));
    if (TranslateCharsetInfo((DWORD*)(UINT_PTR)uCharSet, &ci, TCI_SRCCHARSET)) {
      return(ci.ciACP);
    }
  }
  return GetACP(); // fallback: systems locale ANSI CP
}


//=============================================================================
//
//  CharSetFromCodePage()
//
UINT CharSetFromCodePage(const UINT uCodePage) {
  CHARSETINFO ci;
  if (TranslateCharsetInfo((DWORD*)(UINT_PTR)uCodePage, &ci, TCI_SRCCODEPAGE)) {
    return(ci.ciCharset);  // corresponds to SCI: SC_CHARSET_XXX
  }
  return(ANSI_CHARSET);
}


/** ******************************************************************************
 *
 *  UnSlash functions
 *  Mostly taken from SciTE, (c) Neil Hodgson, http://www.scintilla.org
 *
 * Convert C style \a, \b, \f, \n, \r, \t, \v, \xhh and \uhhhh into their indicated characters.
 */
unsigned int UnSlash(char *s,UINT cpEdit) {
  char *sStart = s;
  char *o = s;

  while (*s) {
    if (*s == '\\') {
      s++;
      if (*s == 'a')
        *o = '\a';
      else if (*s == 'b')
        *o = '\b';
      else if (*s == 'f')
        *o = '\f';
      else if (*s == 'n')
        *o = '\n';
      else if (*s == 'r')
        *o = '\r';
      else if (*s == 't')
        *o = '\t';
      else if (*s == 'v')
        *o = '\v';
      else if (*s == 'x' || *s == 'u') {
        bool bShort = (*s == 'x');
        char ch[8];
        char *pch = ch;
        WCHAR val[2] = L"";
        int hex;
        val[0] = 0;
        hex = GetHexDigit(*(s+1));
        if (hex >= 0) {
          s++;
          val[0] = (WCHAR)hex;
          hex = GetHexDigit(*(s+1));
          if (hex >= 0) {
            s++;
            val[0] *= 16;
            val[0] += (WCHAR)hex;
            if (!bShort) {
              hex = GetHexDigit(*(s+1));
              if (hex >= 0) {
                s++;
                val[0] *= 16;
                val[0] += (WCHAR)hex;
                hex = GetHexDigit(*(s+1));
                if (hex >= 0) {
                  s++;
                  val[0] *= 16;
                  val[0] += (WCHAR)hex;
                }
              }
            }
          }
          if (val[0]) {
            val[1] = 0;
            WideCharToMultiByte(cpEdit,0,val,-1,ch,COUNTOF(ch),NULL,NULL);
            *o = *pch++;
            while (*pch)
              *++o = *pch++;
          }
          else
            o--;
        }
        else
          o--;
      }
      else
        *o = *s;
    }
    else
      *o = *s;
    o++;
    if (*s) {
      s++;
    }
  }
  *o = '\0';
  return (unsigned int)(o - sStart);
}

/**
 * Convert C style \0oo into their indicated characters.
 * This is used to get control characters into the regular expresion engine
 * w/o interfering with group referencing ('\0').
 */
unsigned int UnSlashLowOctal(char* s) {
  char* sStart = s;
  char* o = s;
  while (*s) {
    if ((s[0] == '\\') && (s[1] == '\\')) { // esc seq
      *o = *s; ++o; ++s; *o = *s;
    }
    else if ((s[0] == '\\') && (s[1] == '0') && IsOctalDigit(s[2]) && IsOctalDigit(s[3])) {
      *o = (char)(8 * (s[2] - '0') + (s[3] - '0'));
      s += 3;
    } else {
      *o = *s;
    }
    ++o;
    if (*s)
      ++s;
  }
  *o = '\0';
  return (unsigned int)(o - sStart);
}


/**
 *  check, if we have regex sub-group referencing 
 */
int CheckRegExReplTarget(char* pszInput)
{
  while (*pszInput) {
    if (*pszInput == '$') {
      ++pszInput;
      if (((*pszInput >= '0') && (*pszInput <= '9')) || (*pszInput == '+') || (*pszInput == '{')) {
        return SCI_REPLACETARGETRE;
      }
    }
    else if (*pszInput == '\\') {
      ++pszInput;
      if ((*pszInput >= '0') && (*pszInput <= '9')) {
        return SCI_REPLACETARGETRE;
      }
    }
    ++pszInput;
  }
  return SCI_REPLACETARGET;
}


void TransformBackslashes(char* pszInput, bool bRegEx, UINT cpEdit, int* iReplaceMsg)
{
  if (bRegEx && iReplaceMsg) {
    UnSlashLowOctal(pszInput);
    *iReplaceMsg = CheckRegExReplTarget(pszInput);
  }
  else if (iReplaceMsg) {
    *iReplaceMsg = SCI_REPLACETARGET;  // uses SCI std replacement
  }

  // regex handles backslashes itself
  // except: replacement is not delegated to regex engine
  if (!bRegEx || (iReplaceMsg && (SCI_REPLACETARGET == *iReplaceMsg))) {
    UnSlash(pszInput, cpEdit);
  }
}


void TransformMetaChars(char* pszInput, bool bRegEx, int iEOLMode)
{
  if (!bRegEx)  return;

  char buffer[FNDRPL_BUFFER + 1] = { '\0' };
  char* s = pszInput;
  char* o = buffer;
  while (*s) {
    if ((s[0] != '\\') && (s[1] == '$')) {
      *o = *s;  ++o;  ++s;
      switch (iEOLMode) {
      case SC_EOL_LF:
        *o = '\n';
        break;
      case SC_EOL_CR:
        *o = '\r';
        break;
      case SC_EOL_CRLF:
      default:
        *o = '\r'; ++o; *o = '\n';
        break;
      }
      ++s; // skip $
    }
    else {
      *o = *s;
    }
    ++o;
    if (*s)  ++s;
  }
  *o = '\0';
  StringCchCopyA(pszInput, FNDRPL_BUFFER, buffer);
}



/*

  MinimizeToTray - Copyright 2000 Matthew Ellis <m.t.ellis@bigfoot.com>

  Changes made by flo:
   - Commented out: #include "stdafx.h"
   - Moved variable declaration: APPBARDATA appBarData;

*/

// MinimizeToTray
//
// A couple of routines to show how to make it produce a custom caption
// animation to make it look like we are minimizing to and maximizing
// from the system tray
//
// These routines are public domain, but it would be nice if you dropped
// me a line if you use them!
//
// 1.0 29.06.2000 Initial version
// 1.1 01.07.2000 The window retains it's place in the Z-order of windows
//     when minimized/hidden. This means that when restored/shown, it doen't
//     always appear as the foreground window unless we call SetForegroundWindow
//
// Copyright 2000 Matthew Ellis <m.t.ellis@bigfoot.com>
/*#include "stdafx.h"*/

// Odd. VC++6 winuser.h has IDANI_CAPTION defined (as well as IDANI_OPEN and
// IDANI_CLOSE), but the Platform SDK only has IDANI_OPEN...

// I don't know what IDANI_OPEN or IDANI_CLOSE do. Trying them in this code
// produces nothing. Perhaps they were intended for window opening and closing
// like the MAC provides...
#ifndef IDANI_OPEN
#define IDANI_OPEN 1
#endif
#ifndef IDANI_CLOSE
#define IDANI_CLOSE 2
#endif
#ifndef IDANI_CAPTION
#define IDANI_CAPTION 3
#endif

#define DEFAULT_RECT_WIDTH 150
#define DEFAULT_RECT_HEIGHT 30

// Returns the rect of where we think the system tray is. This will work for
// all current versions of the shell. If explorer isn't running, we try our
// best to work with a 3rd party shell. If we still can't find anything, we
// return a rect in the lower right hand corner of the screen
static VOID GetTrayWndRect(LPRECT lpTrayRect)
{
  APPBARDATA appBarData;
  // First, we'll use a quick hack method. We know that the taskbar is a window
  // of class Shell_TrayWnd, and the status tray is a child of this of class
  // TrayNotifyWnd. This provides us a window rect to minimize to. Note, however,
  // that this is not guaranteed to work on future versions of the shell. If we
  // use this method, make sure we have a backup!
  HWND hShellTrayWnd=FindWindowEx(NULL,NULL,TEXT("Shell_TrayWnd"),NULL);
  if(hShellTrayWnd)
  {
    HWND hTrayNotifyWnd=FindWindowEx(hShellTrayWnd,NULL,TEXT("TrayNotifyWnd"),NULL);
    if(hTrayNotifyWnd)
    {
      GetWindowRect(hTrayNotifyWnd,lpTrayRect);
      return;
    }
  }

  // OK, we failed to get the rect from the quick hack. Either explorer isn't
  // running or it's a new version of the shell with the window class names
  // changed (how dare Microsoft change these undocumented class names!) So, we
  // try to find out what side of the screen the taskbar is connected to. We
  // know that the system tray is either on the right or the bottom of the
  // taskbar, so we can make a good guess at where to minimize to
  /*APPBARDATA appBarData;*/
  appBarData.cbSize=sizeof(appBarData);
  if(SHAppBarMessage(ABM_GETTASKBARPOS,&appBarData))
  {
    // We know the edge the taskbar is connected to, so guess the rect of the
    // system tray. Use various fudge factor to make it look good
    switch(appBarData.uEdge)
    {
      case ABE_LEFT:
      case ABE_RIGHT:
  // We want to minimize to the bottom of the taskbar
  lpTrayRect->top=appBarData.rc.bottom-100;
  lpTrayRect->bottom=appBarData.rc.bottom-16;
  lpTrayRect->left=appBarData.rc.left;
  lpTrayRect->right=appBarData.rc.right;
  break;

      case ABE_TOP:
      case ABE_BOTTOM:
  // We want to minimize to the right of the taskbar
  lpTrayRect->top=appBarData.rc.top;
  lpTrayRect->bottom=appBarData.rc.bottom;
  lpTrayRect->left=appBarData.rc.right-100;
  lpTrayRect->right=appBarData.rc.right-16;
  break;
    }

    return;
  }

  // Blimey, we really aren't in luck. It's possible that a third party shell
  // is running instead of explorer. This shell might provide support for the
  // system tray, by providing a Shell_TrayWnd window (which receives the
  // messages for the icons) So, look for a Shell_TrayWnd window and work out
  // the rect from that. Remember that explorer's taskbar is the Shell_TrayWnd,
  // and stretches either the width or the height of the screen. We can't rely
  // on the 3rd party shell's Shell_TrayWnd doing the same, in fact, we can't
  // rely on it being any size. The best we can do is just blindly use the
  // window rect, perhaps limiting the width and height to, say 150 square.
  // Note that if the 3rd party shell supports the same configuraion as
  // explorer (the icons hosted in NotifyTrayWnd, which is a child window of
  // Shell_TrayWnd), we would already have caught it above
  hShellTrayWnd=FindWindowEx(NULL,NULL,TEXT("Shell_TrayWnd"),NULL);
  if(hShellTrayWnd)
  {
    GetWindowRect(hShellTrayWnd,lpTrayRect);
    if(lpTrayRect->right-lpTrayRect->left>DEFAULT_RECT_WIDTH)
      lpTrayRect->left=lpTrayRect->right-DEFAULT_RECT_WIDTH;
    if(lpTrayRect->bottom-lpTrayRect->top>DEFAULT_RECT_HEIGHT)
      lpTrayRect->top=lpTrayRect->bottom-DEFAULT_RECT_HEIGHT;

    return;
  }

  // OK. Haven't found a thing. Provide a default rect based on the current work area
  SystemParametersInfo(SPI_GETWORKAREA,0,lpTrayRect,0);
  lpTrayRect->left=lpTrayRect->right-DEFAULT_RECT_WIDTH;
  lpTrayRect->top=lpTrayRect->bottom-DEFAULT_RECT_HEIGHT;
}

// Check to see if the animation has been disabled
/*static */bool GetDoAnimateMinimize(VOID)
{
  ANIMATIONINFO ai;

  ai.cbSize=sizeof(ai);
  SystemParametersInfo(SPI_GETANIMATION,sizeof(ai),&ai,0);

  return ai.iMinAnimate?true:false;
}

VOID MinimizeWndToTray(HWND hWnd)
{
  if(GetDoAnimateMinimize())
  {
    RECT rcFrom,rcTo;

    // Get the rect of the window. It is safe to use the rect of the whole
    // window - DrawAnimatedRects will only draw the caption
    GetWindowRect(hWnd,&rcFrom);
    GetTrayWndRect(&rcTo);

    // Get the system to draw our animation for us
    DrawAnimatedRects(hWnd,IDANI_CAPTION,&rcFrom,&rcTo);
  }

  // Add the tray icon. If we add it before the call to DrawAnimatedRects,
  // the taskbar gets erased, but doesn't get redrawn until DAR finishes.
  // This looks untidy, so call the functions in this order

  // Hide the window
  ShowWindow(hWnd,SW_HIDE);
}

VOID RestoreWndFromTray(HWND hWnd)
{
  if(GetDoAnimateMinimize())
  {
    // Get the rect of the tray and the window. Note that the window rect
    // is still valid even though the window is hidden
    RECT rcFrom,rcTo;
    GetTrayWndRect(&rcFrom);
    GetWindowRect(hWnd,&rcTo);

    // Get the system to draw our animation for us
    DrawAnimatedRects(hWnd,IDANI_CAPTION,&rcFrom,&rcTo);
  }

  // Show the window, and make sure we're the foreground window
  ShowWindow(hWnd,SW_SHOW);
  SetActiveWindow(hWnd);
  SetForegroundWindow(hWnd);

  // Remove the tray icon. As described above, remove the icon after the
  // call to DrawAnimatedRects, or the taskbar will not refresh itself
  // properly until DAR finished
}



//=============================================================================
//
//  UrlUnescapeEx()
//
void UrlUnescapeEx(LPWSTR lpURL, LPWSTR lpUnescaped, DWORD* pcchUnescaped)
{
#if defined(URL_UNESCAPE_AS_UTF8)
  UrlUnescape(lpURL, lpUnescaped, pcchUnescaped, URL_UNESCAPE_AS_UTF8);
#else
  char* outBuffer = AllocMem(*pcchUnescaped + 1, HEAP_ZERO_MEMORY);
  if (outBuffer == NULL) {
    return;
  }
  int const outLen = (int)*pcchUnescaped;

  size_t posIn = 0;
  WCHAR buf[5] = { L'\0' };
  size_t lastEsc = StringCchLenW(lpURL,0) - 2;
  unsigned int code;

  int posOut = 0;
  while ((posIn < lastEsc) && (posOut < outLen))
  {
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
          if (lpURL[posIn] == L';') ++posIn;
          bOk = true;
        }
      }
    }
    // TODO(rkotten): HTML Hex encoded (&#x...)
    if (!bOk) {
      posOut += WideCharToMultiByte(Encoding_SciCP, 0, &(lpURL[posIn++]), 1, 
                                    &(outBuffer[posOut]), (MBWC_DocPos_Cast)(outLen - posOut), NULL, NULL);
    }
  }

  // copy rest
  while ((lpURL[posIn] != L'\0') && (posOut < outLen))
  {
    posOut += WideCharToMultiByte(Encoding_SciCP, 0, &(lpURL[posIn++]), 1, 
                                  &(outBuffer[posOut]), (MBWC_DocPos_Cast)(outLen - posOut), NULL, NULL);
  }
  outBuffer[posOut] = '\0';

  int const iOut = MultiByteToWideChar(Encoding_SciCP, 0, outBuffer, -1, lpUnescaped, (MBWC_DocPos_Cast)*pcchUnescaped);
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
    if (sDefault && *sDefault)
      StringCchCopyW(sMatrix[i], (size_t)iLen, sDefault);
    else
      sMatrix[i][0] = L'\0';
  }
  // insert values
  int n = 0;
  WCHAR* p = wchTmpBuff;
  while (p && *p) {
    WCHAR* q = StrStrW(p, L",");
    if (q > p) { *q = L'\0'; }
    if (n < iCount) {
      if (*p != L',') { 
        StringCchCopyW(sMatrix[n], (size_t)iLen, p); 
      }
      else {
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
int ReadVectorFromString(LPCWSTR wchStrg, int iVector[], int iCount, int iMin, int iMax, int iDefault)
{
  static WCHAR wchTmpBuff[SMALL_BUFFER];

  StringCchCopyW(wchTmpBuff, COUNTOF(wchTmpBuff), wchStrg);
  TrimSpcW(wchTmpBuff);
  // ensure single spaces only
  WCHAR *p = StrStr(wchTmpBuff, L"  ");
  while (p) {
    MoveMemory((WCHAR*)p, (WCHAR*)p + 1, (StringCchLenW(p,0) + 1) * sizeof(WCHAR));
    p = StrStr(wchTmpBuff, L"  ");  // next
  }
  // separate values
  int const len = (int)StringCchLenW(wchTmpBuff, COUNTOF(wchTmpBuff));
  for (int i = 0; i < len; ++i) {
    if (wchTmpBuff[i] == L' ') { wchTmpBuff[i] = L'\0'; }
  }
  wchTmpBuff[len + 1] = L'\0'; // double zero at the end

  // fill default
  for (int i = 0; i < iCount; ++i) { iVector[i] = iDefault; }
  // insert values
  int n = 0;
  p = wchTmpBuff;
  while (*p) {
    int iValue;
    if (swscanf_s(p, L"%i", &iValue) == 1) {
      if (n < iCount) {
        iVector[n++] = clampi(iValue, iMin, iMax);
      }
    }
    p = StrEnd(p,0) + 1;
  }
  return n;
}


//=============================================================================
//
//  Char2FloatW()
//  Locale indpendant simple character to tloat conversion
//
bool Char2FloatW(WCHAR* wnumber, float* fresult)
{
  if (!wnumber || !fresult) { return false; }

  int i = 0;
  for (; IsBlankCharW(wnumber[i]); ++i); // skip spaces

  // determine sign
  int const sign = (wnumber[i] == L'-') ? -1 : 1;
  if (wnumber[i] == L'-' || wnumber[i] == L'+') { ++i; }

  // must be digit now
  if (!IsDigitW(wnumber[i])) { return false; }

  // digits before decimal
  float val = 0.0f;
  while (IsDigitW(wnumber[i])) {
    val = (val * 10) + (wnumber[i] - L'0');
    ++i;
  }

  // skip decimal point (or comma) if present
  if ((wnumber[i] == L'.') || (wnumber[i] == L',')) { ++i; }

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
    if (Char2FloatW(&(wnumber[i]), &fexp)) {
      exponent = powf(10, fexp);
    }
  }

  *fresult = ((sign*val*exponent) / (place));
  return true;
}


//=============================================================================
//
//  Float2String()
//  
//
void Float2String(float fValue, LPWSTR lpszStrg, int cchSize)
{
  if (!lpszStrg) { return; };
  fValue = Round10th(fValue);
  if (HasNonZeroFraction(fValue))
    StringCchPrintf(lpszStrg, cchSize, L"%.3G", fValue);
  else
    StringCchPrintf(lpszStrg, cchSize, L"%i", float2int(fValue));
}


///////////////////////////////////////////////////////////////////////////////
//
//   Drag N Drop helpers
//
///////////////////////////////////////////////////////////////////////////////

static HANDLE g_hHeap = NULL;

typedef struct tIDROPTARGET {
  IDropTarget idt;
  LONG lRefCount;
  ULONG lNumFormats;
  CLIPFORMAT *pFormat;
  HWND hWnd;
  bool bAllowDrop;
  DWORD dwKeyState;
  IDataObject *pDataObject;
  UINT nMsg;
  void *pUserData;
  DNDCALLBACK pDropProc;
} 
IDROPTARGET, *PIDROPTARGET;


typedef struct IDRPTRG_VTBL
{
  BEGIN_INTERFACE
    HRESULT(STDMETHODCALLTYPE *QueryInterface)(PIDROPTARGET pThis, REFIID riid, void  **ppvObject);
    ULONG(STDMETHODCALLTYPE   *AddRef)(PIDROPTARGET pThis);
    ULONG(STDMETHODCALLTYPE   *Release)(PIDROPTARGET pThis);
    HRESULT(STDMETHODCALLTYPE *DragEnter)(PIDROPTARGET pThis, IDataObject *pDataObject, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect);
    HRESULT(STDMETHODCALLTYPE *DragOver)(PIDROPTARGET pThis, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect);
    HRESULT(STDMETHODCALLTYPE *DragLeave)(PIDROPTARGET pThis);
    HRESULT(STDMETHODCALLTYPE *Drop)(PIDROPTARGET pThis, IDataObject *pDataObject, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect);
  END_INTERFACE
} 
IDRPTRG_VTBL, *PIDRPTRG_VTBL;


//=============================================================================
//
//  DragAndDropInit()
//
void DragAndDropInit(HANDLE hHeap)
{
  if (g_hHeap == NULL && hHeap == NULL)
    g_hHeap = GetProcessHeap();
  else if (g_hHeap == NULL)
    g_hHeap = hHeap;

  //OleInitialize(NULL); // just in case
}


//=============================================================================
//
//  GetDnDHeap()
//
static HANDLE GetDnDHeap()
{
  if (g_hHeap == NULL) {
    g_hHeap = GetProcessHeap();
  }
  return g_hHeap;
}


//=============================================================================
//
//  IDRPTRG_AddRef()
//
static ULONG STDMETHODCALLTYPE IDRPTRG_AddRef(PIDROPTARGET pThis)
{
  return InterlockedIncrement(&pThis->lRefCount);
}


//=============================================================================
//
//  IDRPTRG_QueryDataObject()
//
static bool IDRPTRG_QueryDataObject(PIDROPTARGET pDropTarget, IDataObject *pDataObject)
{
  ULONG lFmt;
  FORMATETC fmtetc = { CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

  for (lFmt = 0; lFmt < pDropTarget->lNumFormats; lFmt++)
  {
    fmtetc.cfFormat = pDropTarget->pFormat[lFmt];
    if (pDataObject->lpVtbl->QueryGetData(pDataObject, &fmtetc) == S_OK)
      return true;
  }
  return false;
}


//=============================================================================
//
//  IDRPTRG_QueryInterface()
//
static HRESULT STDMETHODCALLTYPE IDRPTRG_QueryInterface(PIDROPTARGET pThis, REFIID riid,
  LPVOID *ppvObject)
{
  *ppvObject = NULL;

  if (IsEqualGUID(riid, &IID_IUnknown))
  {
    IDRPTRG_AddRef(pThis);
    *ppvObject = pThis;
    return S_OK;
  }
  if (IsEqualGUID(riid, &IID_IDropTarget))
  {
    IDRPTRG_AddRef(pThis);
    *ppvObject = pThis;
    return S_OK;
  }
  return E_NOINTERFACE;
}


//=============================================================================
//
//  IDRPTRG_Release()
//
static ULONG STDMETHODCALLTYPE IDRPTRG_Release(PIDROPTARGET pThis)
{
  ULONG nCount;

  if ((nCount = InterlockedDecrement(&pThis->lRefCount)) == 0)
  {
    HeapFree(GetDnDHeap(), 0, pThis);
    return 0;
  }
  return nCount;
}



//=============================================================================
//
//  IDRPTRG_DropEffect()
//
static DWORD IDRPTRG_DropEffect(DWORD dwKeyState, POINTL pt, DWORD dwAllowed)
{
  DWORD dwEffect = 0;

  if (dwKeyState & MK_CONTROL)
    dwEffect = dwAllowed & DROPEFFECT_COPY;
  else if (dwKeyState & MK_SHIFT)
    dwEffect = dwAllowed & DROPEFFECT_MOVE;

  if (dwEffect == 0)
  {
    if (dwAllowed & DROPEFFECT_COPY)
      dwEffect = DROPEFFECT_COPY;
    if (dwAllowed & DROPEFFECT_MOVE)
      dwEffect = DROPEFFECT_MOVE;
  }
  UNUSED(pt);
  return dwEffect;
}


//=============================================================================
//
//  IDRPTRG_DragEnter()
//
static HRESULT STDMETHODCALLTYPE IDRPTRG_DragEnter(PIDROPTARGET pThis, IDataObject *pDataObject,
  DWORD dwKeyState, POINTL pt, DWORD *pdwEffect)
{
  pThis->bAllowDrop = IDRPTRG_QueryDataObject(pThis, pDataObject);
  if (pThis->bAllowDrop)
  {
    *pdwEffect = IDRPTRG_DropEffect(dwKeyState, pt, *pdwEffect);
    SetFocus(pThis->hWnd);
  }
  else
    *pdwEffect = DROPEFFECT_NONE;

  return S_OK;
}


//=============================================================================
//
//  IDRPTRG_DragOver()
//
static HRESULT STDMETHODCALLTYPE IDRPTRG_DragOver(PIDROPTARGET pThis, DWORD dwKeyState, POINTL pt,
  DWORD *pdwEffect)
{
  if (pThis->bAllowDrop)
  {
    pThis->dwKeyState = dwKeyState;

    *pdwEffect = IDRPTRG_DropEffect(dwKeyState, pt, *pdwEffect);
  }
  else
    *pdwEffect = DROPEFFECT_NONE;

  return S_OK;
}


//=============================================================================
//
//  IDRPTRG_DragLeave()
//
static HRESULT STDMETHODCALLTYPE IDRPTRG_DragLeave(PIDROPTARGET pThis)
{
  UNUSED(pThis);
  return S_OK;
}


//=============================================================================
//
//  IDRPTRG_Drop()
//
static HRESULT STDMETHODCALLTYPE IDRPTRG_Drop(PIDROPTARGET pThis, IDataObject *pDataObject,
  DWORD dwKeyState, POINTL pt, DWORD *pdwEffect)
{
  FORMATETC fmtetc = { CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
  STGMEDIUM medium;
  DROPDATA DropData;

  UNUSED(dwKeyState);
  UNUSED(pt);

  if (pThis->bAllowDrop)
  {
    ULONG lFmt;
    for (lFmt = 0; lFmt < pThis->lNumFormats; lFmt++)
    {
      fmtetc.cfFormat = pThis->pFormat[lFmt];
      if (pDataObject->lpVtbl->QueryGetData(pDataObject, &fmtetc) == S_OK)
        break;
    }
    if (lFmt < pThis->lNumFormats)
    {
      pDataObject->lpVtbl->GetData(pDataObject, &fmtetc, &medium);
      *pdwEffect = DROPEFFECT_NONE;
      if (pThis->pDropProc != NULL) {
        *pdwEffect = (*pThis->pDropProc)(pThis->pFormat[lFmt], medium.hGlobal, pThis->hWnd, pThis->dwKeyState, pt, pThis->pUserData);
      }
      else if (pThis->nMsg != WM_NULL)
      {
        DropData.cf = pThis->pFormat[lFmt];
        DropData.dwKeyState = pThis->dwKeyState;
        DropData.hData = medium.hGlobal;
        DropData.pt = pt;

        *pdwEffect = (DWORD)SendMessage(pThis->hWnd, pThis->nMsg, (WPARAM)&DropData, (LPARAM)pThis->pUserData);
      }
      if (*pdwEffect != DROPEFFECT_NONE)
        ReleaseStgMedium(&medium);
    }
  }
  else
    *pdwEffect = DROPEFFECT_NONE;

  return S_OK;
}


//=============================================================================
//
//  CreateDropTarget()
//
IDropTarget* CreateDropTarget(const CLIPFORMAT *pFormat, ULONG lFmt, HWND hWnd, UINT nMsg,
  DWORD(*pDropProc)(CLIPFORMAT cf, HGLOBAL hData, HWND hWnd, DWORD dwKeyState, POINTL pt, void *pUserData),
  void *pUserData)
{
  PIDROPTARGET pRet;
  static IDRPTRG_VTBL idt_vtbl = {
    IDRPTRG_QueryInterface,
    IDRPTRG_AddRef,
    IDRPTRG_Release,
    IDRPTRG_DragEnter,
    IDRPTRG_DragOver,
    IDRPTRG_DragLeave,
    IDRPTRG_Drop };

  if ((pRet = HeapAlloc(GetDnDHeap(), 0, sizeof(IDROPTARGET) + lFmt * sizeof(CLIPFORMAT))) == NULL)
    return NULL;

  pRet->pFormat = (CLIPFORMAT *)(((char *)pRet) + sizeof(IDROPTARGET));

  pRet->idt.lpVtbl = (IDropTargetVtbl*)&idt_vtbl;
  pRet->lRefCount = 1;
  pRet->hWnd = hWnd;
  pRet->nMsg = nMsg;
  pRet->bAllowDrop = false;
  pRet->dwKeyState = 0;
  pRet->lNumFormats = lFmt;
  pRet->pDropProc = pDropProc;
  pRet->pUserData = pUserData;

  for (lFmt = 0; lFmt < pRet->lNumFormats; lFmt++) {
    pRet->pFormat[lFmt] = pFormat[lFmt];
  }
  return (IDropTarget *)pRet;
}



//=============================================================================
//
//  RegisterDragAndDrop()
//
PDROPTARGET RegisterDragAndDrop(HWND hWnd, CLIPFORMAT *pFormat, ULONG lFmt, UINT nMsg, DNDCALLBACK pDropProc, void *pUserData)
{
  IDropTarget *pTarget;

  if ((pTarget = CreateDropTarget(pFormat, lFmt, hWnd, nMsg, pDropProc, pUserData)) == NULL)
    return NULL;

  if (RegisterDragDrop(hWnd, pTarget) != S_OK)
  {
    HeapFree(GetDnDHeap(), 0, pTarget);
    return NULL;
  }

  return (PDROPTARGET)pTarget;
}


//=============================================================================
//
//  RevokeDragAndDrop()
//
PDROPTARGET RevokeDragAndDrop(PDROPTARGET pTarget)
{
  if (pTarget == NULL)
    return NULL;

  if (((PIDROPTARGET)pTarget)->hWnd != NULL)
  {
    if (GetWindowLongPtr(((PIDROPTARGET)pTarget)->hWnd, GWLP_WNDPROC) != 0)
      RevokeDragDrop(((PIDROPTARGET)pTarget)->hWnd);
  }

  ((IDropTarget *)pTarget)->lpVtbl->Release((IDropTarget *)pTarget);

  return NULL;
}

///   End of Helpers.c   ///
