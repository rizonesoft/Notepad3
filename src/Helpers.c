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
*                                                  (c) Rizonesoft 2008-2016   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

#if !defined(WINVER)
#define WINVER 0x601  /*_WIN32_WINNT_WIN7*/
#endif
#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x601  /*_WIN32_WINNT_WIN7*/
#endif
#if !defined(NTDDI_VERSION)
#define NTDDI_VERSION 0x06010000  /*NTDDI_WIN7*/
#endif
#define VC_EXTRALEAN 1

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <stdio.h>
#include <string.h>
//#include <pathcch.h>
#include "scintilla.h"
#include "resource.h"
#include "helpers.h"


extern HINSTANCE g_hInstance;
extern BOOL bSkipUnicodeDetection;
extern BOOL bPreserveCaretPos;


//=============================================================================
//
//  Cut of substrings defined by pattern
//

CHAR* _StrCutIA(CHAR* s,const CHAR* pattern)
{
  CHAR* p = NULL;
  do {
    p = StrStrIA(s,pattern);
    if (p) {
      CHAR* q = p + strlen(pattern);
      while (*p != '\0') { *p++ = *q++; }
    }
  } while (p);
  return s;
}

WCHAR* _StrCutIW(WCHAR* s,const WCHAR* pattern)
{
  WCHAR* p = NULL;
  do {
    p = StrStrIW(s,pattern);
    if (p) {
      WCHAR* q = p + lstrlen(pattern);
      while (*p != L'\0') { *p++ = *q++; }
    }
  } while (p);
  return s;
}


//=============================================================================
//
//  Manipulation of (cached) ini file sections
//
int IniSectionGetString(
      LPCWSTR lpCachedIniSection,
      LPCWSTR lpName,
      LPCWSTR lpDefault,
      LPWSTR lpReturnedString,
      int cchReturnedString)
{
  WCHAR *p = (WCHAR *)lpCachedIniSection;
  WCHAR tch[256] = { L'\0' };
  int  ich;

  if (p) {
    StringCchCopy(tch,COUNTOF(tch),lpName);
    StringCchCat(tch,COUNTOF(tch),L"=");
    ich = StringCchLen(tch);

    while (*p) {
      if (StrCmpNI(p,tch,ich) == 0) {
        StringCchCopyN(lpReturnedString,cchReturnedString,p + ich,cchReturnedString);
        return(StringCchLenN(lpReturnedString,cchReturnedString));
      }
      else
        p = StrEnd(p) + 1;
    }
  }
  StringCchCopyN(lpReturnedString,cchReturnedString,lpDefault,cchReturnedString);
  return(StringCchLenN(lpReturnedString,cchReturnedString));
}


int IniSectionGetInt(
      LPCWSTR lpCachedIniSection,
      LPCWSTR lpName,
      int iDefault)
{
  WCHAR *p = (WCHAR *)lpCachedIniSection;
  WCHAR tch[256] = { L'\0' };
  int  ich;
  int  i;

  if (p) {
    StringCchCopy(tch,COUNTOF(tch),lpName);
    StringCchCat(tch,COUNTOF(tch),L"=");
    ich = StringCchLen(tch);

    while (*p) {
      if (StrCmpNI(p,tch,ich) == 0) {
        if (swscanf_s(p + ich,L"%i",&i) == 1)
          return(i);
        else
          return(iDefault);
      }
      else
        p = StrEnd(p) + 1;
    }
  }
  return(iDefault);
}


UINT IniSectionGetUInt(
    LPCWSTR lpCachedIniSection,
    LPCWSTR lpName,
    UINT uDefault) {
    WCHAR *p = (WCHAR *)lpCachedIniSection;
    WCHAR tch[256] = { L'\0' };
    int  ich;
    UINT u;

    if (p) {
      StringCchCopy(tch,COUNTOF(tch),lpName);
      StringCchCat(tch,COUNTOF(tch),L"=");
      ich = StringCchLen(tch);

        while (*p) {
            if (StrCmpNI(p, tch, ich) == 0) {
                if (swscanf_s(p + ich, L"%u", &u) == 1)
                    return(u);
                else
                    return(uDefault);
            }
            else
                p = StrEnd(p) + 1;
        }
    }
    return(uDefault);
}


BOOL IniSectionSetString(LPWSTR lpCachedIniSection,LPCWSTR lpName,LPCWSTR lpString)
{
  WCHAR tch[32+512*3+32];
  WCHAR* p = lpCachedIniSection;

  if (p) {
    while (*p) {
      p = StrEnd(p) + 1;
    }
    StringCchPrintf(tch,COUNTOF(tch),L"%s=%s",lpName,lpString);
    StringCchCopy(p,512*3+64,tch);
    p = StrEnd(p) + 1;
    *p = 0;
    return(TRUE);
  }
  return(FALSE);
}


//=============================================================================
//
//  PrivateIsAppThemed()
//
extern HMODULE hModUxTheme;
BOOL PrivateIsAppThemed()
{
  FARPROC pfnIsAppThemed;
  BOOL bIsAppThemed = FALSE;

  if (hModUxTheme) {
    pfnIsAppThemed = GetProcAddress(hModUxTheme,"IsAppThemed");

    if (pfnIsAppThemed)
      bIsAppThemed = (BOOL)pfnIsAppThemed();
  }
  return bIsAppThemed;
}


//=============================================================================
//
//  PrivateSetCurrentProcessExplicitAppUserModelID()
//
HRESULT PrivateSetCurrentProcessExplicitAppUserModelID(PCWSTR AppID)
{
  FARPROC pfnSetCurrentProcessExplicitAppUserModelID;

  if (lstrlen(AppID) == 0)
    return(S_OK);

  if (StringCchCompareIX(AppID,L"(default)") == 0)
    return(S_OK);

  pfnSetCurrentProcessExplicitAppUserModelID =
    GetProcAddress(GetModuleHandleA("shell32.dll"),"SetCurrentProcessExplicitAppUserModelID");

  if (pfnSetCurrentProcessExplicitAppUserModelID)
    return((HRESULT)pfnSetCurrentProcessExplicitAppUserModelID(AppID));

  else
    return(S_OK);
}


//=============================================================================
//
//  IsElevated()
//
BOOL IsElevated() {

  BOOL bIsElevated = FALSE;
  HANDLE hToken = NULL;

  if (!IsVista())
    return(FALSE);

  if (OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&hToken)) {

    TOKEN_ELEVATION te;
    DWORD expectedRetVal = sizeof(TOKEN_ELEVATION);
    DWORD dwReturnLength = 0;

    if (GetTokenInformation(hToken,TokenElevation,&te,expectedRetVal,&dwReturnLength)) {
        if (dwReturnLength == expectedRetVal)
          bIsElevated = (BOOL)te.TokenIsElevated;
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
// Routine Description: This routine returns TRUE if the caller's
// process is a member of the Administrators local group. Caller is NOT
// expected to be impersonating anyone and is expected to be able to
// open its own process and process token.
// Arguments: None.
// Return Value:
// TRUE - Caller has Administrators local group.
// FALSE - Caller does not have Administrators local group. --
//
BOOL IsUserAdmin()
{
  PSID AdminGroup;
  SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
  BOOL bIsAdmin = AllocateAndInitializeSid(&NtAuthority,2,
    SECURITY_BUILTIN_DOMAIN_RID,DOMAIN_ALIAS_RID_ADMINS,0,0,0,0,0,0,&AdminGroup);
  if (bIsAdmin) {
    if (!CheckTokenMembership(NULL,AdminGroup,&bIsAdmin))
      bIsAdmin = FALSE;
    FreeSid(AdminGroup);
  }
  return(bIsAdmin);
}



//=============================================================================
//
//  SetExplorerTheme()
//
//BOOL SetExplorerTheme(HWND hwnd)
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
//  return FALSE;
//}


//=============================================================================
//
//  BitmapMergeAlpha()
//  Merge alpha channel into color channel
//
BOOL BitmapMergeAlpha(HBITMAP hbmp,COLORREF crDest)
{
  BITMAP bmp;
  if (GetObject(hbmp,sizeof(BITMAP),&bmp)) {

    if (bmp.bmBitsPixel == 32) {

      int x,y;
      RGBQUAD *prgba = bmp.bmBits;

      for (y = 0; y < bmp.bmHeight; y++) {
        for (x = 0; x < bmp.bmWidth; x++) {
          BYTE alpha = prgba[x].rgbReserved;
          prgba[x].rgbRed = ((prgba[x].rgbRed * alpha) + (GetRValue(crDest) * (255-alpha))) >> 8;
          prgba[x].rgbGreen = ((prgba[x].rgbGreen * alpha) + (GetGValue(crDest) * (255-alpha))) >> 8;
          prgba[x].rgbBlue = ((prgba[x].rgbBlue * alpha) + (GetBValue(crDest) * (255-alpha))) >> 8;
          prgba[x].rgbReserved = 0xFF;
        }
        prgba = (RGBQUAD*)((LPBYTE)prgba + bmp.bmWidthBytes);
      }
      return TRUE;
    }
  }
  return FALSE;
}


//=============================================================================
//
//  BitmapAlphaBlend()
//  Perform alpha blending to color channel only
//
BOOL BitmapAlphaBlend(HBITMAP hbmp,COLORREF crDest,BYTE alpha)
{
  BITMAP bmp;
  if (GetObject(hbmp,sizeof(BITMAP),&bmp)) {

    if (bmp.bmBitsPixel == 32) {

      int x,y;
      RGBQUAD *prgba = bmp.bmBits;

      for (y = 0; y < bmp.bmHeight; y++) {
        for (x = 0; x < bmp.bmWidth; x++) {
          prgba[x].rgbRed = ((prgba[x].rgbRed * alpha) + (GetRValue(crDest) * (255-alpha))) >> 8;
          prgba[x].rgbGreen = ((prgba[x].rgbGreen * alpha) + (GetGValue(crDest) * (255-alpha))) >> 8;
          prgba[x].rgbBlue = ((prgba[x].rgbBlue * alpha) + (GetBValue(crDest) * (255-alpha))) >> 8;
        }
        prgba = (RGBQUAD*)((LPBYTE)prgba + bmp.bmWidthBytes);
      }
      return TRUE;
    }
  }
  return FALSE;
}


//=============================================================================
//
//  BitmapGrayScale()
//  Gray scale color channel only
//
BOOL BitmapGrayScale(HBITMAP hbmp)
{
  BITMAP bmp;
  if (GetObject(hbmp,sizeof(BITMAP),&bmp)) {

    if (bmp.bmBitsPixel == 32) {

      int x,y;
      RGBQUAD *prgba = bmp.bmBits;

      for (y = 0; y < bmp.bmHeight; y++) {
        for (x = 0; x < bmp.bmWidth; x++) {
          prgba[x].rgbRed = prgba[x].rgbGreen = prgba[x].rgbBlue =
          (((BYTE)((prgba[x].rgbRed * 38 + prgba[x].rgbGreen * 75 + prgba[x].rgbBlue * 15) >> 7) * 0x80) + (0xD0 * (255-0x80))) >> 8;
        }
        prgba = (RGBQUAD*)((LPBYTE)prgba + bmp.bmWidthBytes);
      }
      return TRUE;
    }
  }
  return FALSE;
}


//=============================================================================
//
//  VerifyContrast()
//  Check if two colors can be distinguished
//
BOOL VerifyContrast(COLORREF cr1,COLORREF cr2)
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
  *((PBOOL)lParam) = TRUE;
  UNUSED(plf);
  UNUSED(ptm);
  UNUSED(FontType);
  return(FALSE);
}

BOOL IsFontAvailable(LPCWSTR lpszFontName)
{
  BOOL fFound = FALSE;

  HDC hDC = GetDC(NULL);
  EnumFonts(hDC,lpszFontName,EnumFontsProc,(LPARAM)&fFound);
  ReleaseDC(NULL,hDC);

  return(fFound);
}


//=============================================================================
//
//  SetWindowTitle()
//
BOOL bFreezeAppTitle = FALSE;

BOOL SetWindowTitle(HWND hwnd,UINT uIDAppName,BOOL bIsElevated,UINT uIDUntitled,
                    LPCWSTR lpszFile,int iFormat,BOOL bModified,
                    UINT uIDReadOnly,BOOL bReadOnly,LPCWSTR lpszExcerpt)
{

  WCHAR szUntitled[MIDSZ_BUFFER] = { L'\0' };
  WCHAR szExcrptQuot[MIDSZ_BUFFER] = { L'\0' };
  WCHAR szExcrptFmt[32] = { L'\0' };
  WCHAR szAppName[MIDSZ_BUFFER] = { L'\0' };
  WCHAR szElevatedAppName[MIDSZ_BUFFER] = { L'\0' };
  WCHAR szReadOnly[32] = { L'\0' };
  WCHAR szTitle[LARGE_BUFFER] = { L'\0' };
  static WCHAR szCachedFile[MAX_PATH] = { L'\0' };
  static WCHAR szCachedDisplayName[MAX_PATH] = { L'\0' };
  static const WCHAR *pszSep = L" - ";
  static const WCHAR *pszMod = L"* ";

  if (bFreezeAppTitle)
    return FALSE;

  if (!GetString(uIDAppName,szAppName,COUNTOF(szAppName)) ||
      !GetString(uIDUntitled,szUntitled,COUNTOF(szUntitled)))
    return FALSE;

  if (bIsElevated) {
    FormatString(szElevatedAppName,COUNTOF(szElevatedAppName),IDS_APPTITLE_ELEVATED,szAppName);
    StringCchCopyN(szAppName,COUNTOF(szAppName),szElevatedAppName,COUNTOF(szElevatedAppName));
  }

  if (bModified)
    StringCchCopy(szTitle,COUNTOF(szTitle),pszMod);
  else
    StringCchCopy(szTitle,COUNTOF(szTitle),L"");

  if (lstrlen(lpszExcerpt)) {
    GetString(IDS_TITLEEXCERPT,szExcrptFmt,COUNTOF(szExcrptFmt));
    StringCchPrintf(szExcrptQuot,COUNTOF(szExcrptQuot),szExcrptFmt,lpszExcerpt);
    StringCchCat(szTitle,COUNTOF(szTitle),szExcrptQuot);
  }

  else if (StringCchLenN(lpszFile,MAX_PATH))
  {
    if (iFormat < 2 && !PathIsRoot(lpszFile))
    {
      if (StringCchCompareN(szCachedFile,COUNTOF(szCachedFile),lpszFile,MAX_PATH) != 0) {
        SHFILEINFO shfi;
        StringCchCopy(szCachedFile,COUNTOF(szCachedFile),lpszFile);
        if (SHGetFileInfo2(lpszFile,0,&shfi,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME))
          StringCchCopy(szCachedDisplayName,COUNTOF(szCachedDisplayName),shfi.szDisplayName);
        else
          StringCchCopy(szCachedDisplayName,COUNTOF(szCachedDisplayName),PathFindFileName(lpszFile));
      }
      StringCchCat(szTitle,COUNTOF(szTitle),szCachedDisplayName);
      if (iFormat == 1) {
        WCHAR tchPath[MAX_PATH] = { L'\0' };
        StringCchCopyN(tchPath,COUNTOF(tchPath),lpszFile,StringCchLenN(lpszFile,MAX_PATH));
        PathRemoveFileSpec(tchPath);
        StringCchCat(szTitle,COUNTOF(szTitle),L" [");
        StringCchCat(szTitle,COUNTOF(szTitle),tchPath);
        StringCchCat(szTitle,COUNTOF(szTitle),L"]");
      }
    }
    else
      StringCchCat(szTitle,COUNTOF(szTitle),lpszFile);
  }
  else {
    StringCchCopy(szCachedFile,COUNTOF(szCachedFile),L"");
    StringCchCopy(szCachedDisplayName,COUNTOF(szCachedDisplayName),L"");
    StringCchCat(szTitle,COUNTOF(szTitle),szUntitled);
  }

  if (bReadOnly && GetString(uIDReadOnly,szReadOnly,COUNTOF(szReadOnly)))
  {
    StringCchCat(szTitle,COUNTOF(szTitle),L" ");
    StringCchCat(szTitle,COUNTOF(szTitle),szReadOnly);
  }

  StringCchCat(szTitle,COUNTOF(szTitle),pszSep);
  StringCchCat(szTitle,COUNTOF(szTitle),szAppName);

  return SetWindowText(hwnd,szTitle);

}


//=============================================================================
//
//  SetWindowTransparentMode()
//
void SetWindowTransparentMode(HWND hwnd,BOOL bTransparentMode)
{
  if (bTransparentMode) {
    FARPROC fp = GetProcAddress(GetModuleHandle(L"User32"), "SetLayeredWindowAttributes");
    if (fp) {
      SetWindowLongPtr(hwnd,GWL_EXSTYLE,GetWindowLongPtr(hwnd,GWL_EXSTYLE) | WS_EX_LAYERED);

      // get opacity level from registry
      int iAlphaPercent = IniGetInt(L"Settings2",L"OpacityLevel",75);
      if (iAlphaPercent < 0 || iAlphaPercent > 100)
        iAlphaPercent = 75;
      BYTE bAlpha = (BYTE)(iAlphaPercent * 255 / 100);

      fp(hwnd,0,bAlpha,LWA_ALPHA);
    }
  }

  else
    SetWindowLongPtr(hwnd,GWL_EXSTYLE,
      GetWindowLongPtr(hwnd,GWL_EXSTYLE) & ~WS_EX_LAYERED);
}


//=============================================================================
//
//  CenterDlgInParent()
//
void CenterDlgInParent(HWND hDlg)
{

  RECT rcDlg;
  HWND hParent;
  RECT rcParent;
  MONITORINFO mi;
  HMONITOR hMonitor;

  int xMin, yMin, xMax, yMax, x, y;

  GetWindowRect(hDlg,&rcDlg);

  hParent = GetParent(hDlg);
  GetWindowRect(hParent,&rcParent);

  hMonitor = MonitorFromRect(&rcParent,MONITOR_DEFAULTTONEAREST);
  mi.cbSize = sizeof(mi);
  GetMonitorInfo(hMonitor,&mi);

  xMin = mi.rcWork.left;
  yMin = mi.rcWork.top;

  xMax = (mi.rcWork.right) - (rcDlg.right - rcDlg.left);
  yMax = (mi.rcWork.bottom) - (rcDlg.bottom - rcDlg.top);

  if ((rcParent.right - rcParent.left) - (rcDlg.right - rcDlg.left) > 20)
    x = rcParent.left + (((rcParent.right - rcParent.left) - (rcDlg.right - rcDlg.left)) / 2);
  else
    x = rcParent.left + 70;

  if ((rcParent.bottom - rcParent.top) - (rcDlg.bottom - rcDlg.top) > 20)
    y = rcParent.top  + (((rcParent.bottom - rcParent.top) - (rcDlg.bottom - rcDlg.top)) / 2);
  else
    y = rcParent.top + 60;

  SetWindowPos(hDlg,NULL,max(xMin,min(xMax,x)),max(yMin,min(yMax,y)),0,0,SWP_NOZORDER|SWP_NOSIZE);

  //SnapToDefaultButton(hDlg);
}


//=============================================================================
//
//  GetDlgPos()
//
void GetDlgPos(HWND hDlg,LPINT xDlg,LPINT yDlg)
{

  RECT rcDlg;
  HWND hParent;
  RECT rcParent;

  GetWindowRect(hDlg,&rcDlg);

  hParent = GetParent(hDlg);
  GetWindowRect(hParent,&rcParent);

  // return positions relative to parent window
  *xDlg = rcDlg.left - rcParent.left;
  *yDlg = rcDlg.top - rcParent.top;

}


//=============================================================================
//
//  SetDlgPos()
//
void SetDlgPos(HWND hDlg,int xDlg,int yDlg)
{

  RECT rcDlg;
  HWND hParent;
  RECT rcParent;
  MONITORINFO mi;
  HMONITOR hMonitor;

  int xMin, yMin, xMax, yMax, x, y;

  GetWindowRect(hDlg,&rcDlg);

  hParent = GetParent(hDlg);
  GetWindowRect(hParent,&rcParent);

  hMonitor = MonitorFromRect(&rcParent,MONITOR_DEFAULTTONEAREST);
  mi.cbSize = sizeof(mi);
  GetMonitorInfo(hMonitor,&mi);

  xMin = mi.rcWork.left;
  yMin = mi.rcWork.top;

  xMax = (mi.rcWork.right) - (rcDlg.right - rcDlg.left);
  yMax = (mi.rcWork.bottom) - (rcDlg.bottom - rcDlg.top);

  // desired positions relative to parent window
  x = rcParent.left + xDlg;
  y = rcParent.top + yDlg;

  SetWindowPos(hDlg,NULL,max(xMin,min(xMax,x)),max(yMin,min(yMax,y)),0,0,SWP_NOZORDER|SWP_NOSIZE);

}

/*

 ... only if we are working with nonstandard dialog boxes ...

//=============================================================================
//
//  SnapToDefaultButton()
//
// Why doesn't the "Automatically move pointer to the default button in a dialog box"
// work for nonstandard dialog boxes, and how do I add it to my own nonstandard dialog boxes?
// https://blogs.msdn.microsoft.com/oldnewthing/20130826-00/?p=3413/
//
void SnapToDefaultButton(HWND hwndBox)
{
  BOOL bSnapToDefButton = FALSE;
  if (SystemParametersInfo(SPI_GETSNAPTODEFBUTTON, 0, &bSnapToDefButton, 0) && bSnapToDefButton) {
    // get child window at the top of the Z order.
    // for all our MessageBoxs it's the OK or YES button or NULL.
    HWND btn = GetWindow(hwndBox, GW_CHILD);
    if (btn != NULL) {
      WCHAR className[32] = L"";
      GetClassName(btn, className, COUNTOF(className));
      if (lstrcmpi(className, L"Button") == 0) {
        RECT rect;
        int x, y;
        GetWindowRect(btn, &rect);
        x = rect.left + (rect.right - rect.left) / 2;
        y = rect.top + (rect.bottom - rect.top) / 2;
        SetCursorPos(x, y);
      }
    }
  }
}
*/


//=============================================================================
//
//  Resize Dialog Helpers()
//
typedef struct _resizedlg {
  int cxClient;
  int cyClient;
  int cxFrame;
  int cyFrame;
  int mmiPtMinX;
  int mmiPtMinY;
} RESIZEDLG, *PRESIZEDLG;

void ResizeDlg_Init(HWND hwnd,int cxFrame,int cyFrame,int nIdGrip)
{
  RECT rc;
  WCHAR wch[64] = { L'\0' };
  int cGrip;
  RESIZEDLG *pm = LocalAlloc(LPTR,sizeof(RESIZEDLG));

  GetClientRect(hwnd,&rc);
  pm->cxClient = rc.right - rc.left;
  pm->cyClient = rc.bottom - rc.top;

  pm->cxFrame = cxFrame;
  pm->cyFrame = cyFrame;

  AdjustWindowRectEx(&rc,GetWindowLong(hwnd,GWL_STYLE)|WS_THICKFRAME,FALSE,0);
  pm->mmiPtMinX = rc.right-rc.left;
  pm->mmiPtMinY = rc.bottom-rc.top;

  if (pm->cxFrame < (rc.right-rc.left))
    pm->cxFrame = rc.right-rc.left;
  if (pm->cyFrame < (rc.bottom-rc.top))
    pm->cyFrame = rc.bottom-rc.top;

  SetProp(hwnd,L"ResizeDlg",(HANDLE)pm);

  SetWindowPos(hwnd,NULL,rc.left,rc.top,pm->cxFrame,pm->cyFrame,SWP_NOZORDER);

  SetWindowLongPtr(hwnd,GWL_STYLE,GetWindowLongPtr(hwnd,GWL_STYLE)|WS_THICKFRAME);
  SetWindowPos(hwnd,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
  GetMenuString(GetSystemMenu(GetParent(hwnd),FALSE),SC_SIZE,wch,COUNTOF(wch),MF_BYCOMMAND);
  InsertMenu(GetSystemMenu(hwnd,FALSE),SC_CLOSE,MF_BYCOMMAND|MF_STRING|MF_ENABLED,SC_SIZE,wch);
  InsertMenu(GetSystemMenu(hwnd,FALSE),SC_CLOSE,MF_BYCOMMAND|MF_SEPARATOR,0,NULL);

  SetWindowLongPtr(GetDlgItem(hwnd,nIdGrip),GWL_STYLE,
    GetWindowLongPtr(GetDlgItem(hwnd,nIdGrip),GWL_STYLE)|SBS_SIZEGRIP|WS_CLIPSIBLINGS);
  cGrip = GetSystemMetrics(SM_CXHTHUMB);
  SetWindowPos(GetDlgItem(hwnd,nIdGrip),NULL,pm->cxClient-cGrip,pm->cyClient-cGrip,cGrip,cGrip,SWP_NOZORDER);
}

void ResizeDlg_Destroy(HWND hwnd,int *cxFrame,int *cyFrame)
{
  RECT rc;
  PRESIZEDLG pm = GetProp(hwnd,L"ResizeDlg");

  GetWindowRect(hwnd,&rc);
  *cxFrame = rc.right-rc.left;
  *cyFrame = rc.bottom-rc.top;

  RemoveProp(hwnd,L"ResizeDlg");
  LocalFree(pm);
}

void ResizeDlg_Size(HWND hwnd,LPARAM lParam,int *cx,int *cy)
{
  PRESIZEDLG pm = GetProp(hwnd,L"ResizeDlg");

  *cx = LOWORD(lParam) - pm->cxClient;
  *cy = HIWORD(lParam) - pm->cyClient;
  pm->cxClient = LOWORD(lParam);
  pm->cyClient = HIWORD(lParam);
}

void ResizeDlg_GetMinMaxInfo(HWND hwnd,LPARAM lParam)
{
  PRESIZEDLG pm = GetProp(hwnd,L"ResizeDlg");

  LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
  lpmmi->ptMinTrackSize.x = pm->mmiPtMinX;
  lpmmi->ptMinTrackSize.y = pm->mmiPtMinY;
}

HDWP DeferCtlPos(HDWP hdwp,HWND hwndDlg,int nCtlId,int dx,int dy,UINT uFlags)
{
  RECT rc;
  HWND hwndCtl = GetDlgItem(hwndDlg,nCtlId);
  GetWindowRect(hwndCtl,&rc);
  MapWindowPoints(NULL,hwndDlg,(LPPOINT)&rc,2);
  if (uFlags & SWP_NOSIZE)
    return(DeferWindowPos(hdwp,hwndCtl,NULL,rc.left+dx,rc.top+dy,0,0,SWP_NOZORDER|SWP_NOSIZE));
  else
    return(DeferWindowPos(hdwp,hwndCtl,NULL,0,0,rc.right-rc.left+dx,rc.bottom-rc.top+dy,SWP_NOZORDER|SWP_NOMOVE));
}


//=============================================================================
//
//  MakeBitmapButton()
//
void MakeBitmapButton(HWND hwnd,int nCtlId,HINSTANCE hInstance,UINT uBmpId)
{
  HWND hwndCtl = GetDlgItem(hwnd,nCtlId);
  BITMAP bmp;
  BUTTON_IMAGELIST bi;
  HBITMAP hBmp = LoadImage(hInstance,MAKEINTRESOURCE(uBmpId),IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION);
  GetObject(hBmp,sizeof(BITMAP),&bmp);
  bi.himl = ImageList_Create(bmp.bmWidth,bmp.bmHeight,ILC_COLOR32|ILC_MASK,1,0);
  ImageList_AddMasked(bi.himl,hBmp,CLR_DEFAULT);
  DeleteObject(hBmp);
  SetRect(&bi.margin,0,0,0,0);
  bi.uAlign = BUTTON_IMAGELIST_ALIGN_CENTER;
  SendMessage(hwndCtl,BCM_SETIMAGELIST,0,(LPARAM)&bi);
}


//=============================================================================
//
//  MakeColorPickButton()
//
void MakeColorPickButton(HWND hwnd,int nCtlId,HINSTANCE hInstance,COLORREF crColor)
{
  HWND hwndCtl = GetDlgItem(hwnd,nCtlId);
  BUTTON_IMAGELIST bi;
  HIMAGELIST himlOld = NULL;
  HBITMAP hBmp;
  COLORMAP colormap[2];

  if (SendMessage(hwndCtl,BCM_GETIMAGELIST,0,(LPARAM)&bi))
    himlOld = bi.himl;

  if (IsWindowEnabled(hwndCtl) && crColor != -1) {
    colormap[0].from = RGB(0x00,0x00,0x00);
    colormap[0].to   = GetSysColor(COLOR_3DSHADOW);
  }
  else {
    colormap[0].from = RGB(0x00,0x00,0x00);
    colormap[0].to   = RGB(0xFF,0xFF,0xFF);
  }

  if (IsWindowEnabled(hwndCtl) && crColor != -1) {
    if (crColor == RGB(0xFF,0xFF,0xFF))
      crColor = RGB(0xFF,0xFF,0xFE);

    colormap[1].from = RGB(0xFF,0xFF,0xFF);
    colormap[1].to   = crColor;
  }
  else {
    colormap[1].from = RGB(0xFF,0xFF,0xFF);
    colormap[1].to   = RGB(0xFF,0xFF,0xFF);
  }

  hBmp = CreateMappedBitmap(hInstance,IDB_PICK,0,colormap,2);

  bi.himl = ImageList_Create(10,10,ILC_COLORDDB|ILC_MASK,1,0);
  ImageList_AddMasked(bi.himl,hBmp,RGB(0xFF,0xFF,0xFF));
  DeleteObject(hBmp);

  SetRect(&bi.margin,0,0,4,0);
  bi.uAlign = BUTTON_IMAGELIST_ALIGN_RIGHT;

  SendMessage(hwndCtl,BCM_SETIMAGELIST,0,(LPARAM)&bi);
  InvalidateRect(hwndCtl,NULL,TRUE);

  if (himlOld)
    ImageList_Destroy(himlOld);
}


//=============================================================================
//
//  DeleteBitmapButton()
//
void DeleteBitmapButton(HWND hwnd,int nCtlId)
{
  HWND hwndCtl = GetDlgItem(hwnd,nCtlId);
  BUTTON_IMAGELIST bi;
  if (SendMessage(hwndCtl,BCM_GETIMAGELIST,0,(LPARAM)&bi))
    ImageList_Destroy(bi.himl);
}


//=============================================================================
//
//  StatusSetText()
//
BOOL StatusSetText(HWND hwnd,UINT nPart,LPCWSTR lpszText)
{

  UINT uFlags = (nPart == 255) ? nPart|SBT_NOBORDERS : nPart;
  return (BOOL)SendMessage(hwnd,SB_SETTEXT,uFlags,(LPARAM)lpszText);

}


//=============================================================================
//
//  SendWMSize()
//
LRESULT SendWMSize(HWND hwnd)
{
  RECT rc; GetClientRect(hwnd,&rc);
  return(SendMessage(hwnd,WM_SIZE,SIZE_RESTORED,
         MAKELPARAM(rc.right,rc.bottom)));
}


//=============================================================================
//
//  StatusSetTextID()
//
BOOL StatusSetTextID(HWND hwnd,UINT nPart,UINT uID)
{

  WCHAR szText[256] = { L'\0' };
  UINT uFlags = (nPart == 255) ? nPart|SBT_NOBORDERS : nPart;

  if (!uID)
  {
    SendMessage(hwnd,SB_SETTEXT,uFlags,0);
    return TRUE;
  }

  if (!GetString(uID,szText,256))
    return FALSE;

  return (BOOL)SendMessage(hwnd,SB_SETTEXT,uFlags,(LPARAM)szText);

}


//=============================================================================
//
//  StatusCalcPaneWidth()
//
int StatusCalcPaneWidth(HWND hwnd,LPCWSTR lpsz)
{
  SIZE  size;
  HDC   hdc   = GetDC(hwnd);
  HFONT hfont = (HFONT)SendMessage(hwnd,WM_GETFONT,0,0);
  HFONT hfold = SelectObject(hdc,hfont);
  int   mmode = SetMapMode(hdc,MM_TEXT);

  GetTextExtentPoint32(hdc,lpsz,lstrlen(lpsz),&size);

  SetMapMode(hdc,mmode);
  SelectObject(hdc,hfold);
  ReleaseDC(hwnd,hdc);

  return(size.cx + 9);
}


//=============================================================================
//
//  Toolbar_Get/SetButtons()
//
int Toolbar_GetButtons(HWND hwnd,int cmdBase,LPWSTR lpszButtons,int cchButtons)
{
  WCHAR tchButtons[512] = { L'\0' };
  WCHAR tchItem[32] = { L'\0' };
  int i,c;
  TBBUTTON tbb;

  StringCchCopy(tchButtons,COUNTOF(tchButtons),L"");
  c = min(50,(int)SendMessage(hwnd,TB_BUTTONCOUNT,0,0));

  for (i = 0; i < c; i++) {
    SendMessage(hwnd,TB_GETBUTTON,(WPARAM)i,(LPARAM)&tbb);
    StringCchPrintf(tchItem,COUNTOF(tchItem),L"%i ",
      (tbb.idCommand==0)?0:tbb.idCommand-cmdBase+1);
    StringCchCat(tchButtons,COUNTOF(tchButtons),tchItem);
  }
  TrimString(tchButtons);
  StringCchCopyN(lpszButtons,cchButtons,tchButtons,COUNTOF(tchButtons));
  return(c);
}

int Toolbar_SetButtons(HWND hwnd,int cmdBase,LPCWSTR lpszButtons,LPCTBBUTTON ptbb,int ctbb)
{
  WCHAR tchButtons[LARGE_BUFFER];
  int i,c;
  int iCmd;

  ZeroMemory(tchButtons,COUNTOF(tchButtons)*sizeof(tchButtons[0]));
  StringCchCopyN(tchButtons,COUNTOF(tchButtons),lpszButtons,COUNTOF(tchButtons)-2);
  TrimString(tchButtons);
  WCHAR *p = StrStr(tchButtons, L"  ");
  while (p) {
    MoveMemory((WCHAR*)p, (WCHAR*)p + 1, (lstrlen(p) + 1) * sizeof(WCHAR));
    p = StrStr(tchButtons, L"  ");  // next
  }
  c = (int)SendMessage(hwnd,TB_BUTTONCOUNT,0,0);
  for (i = 0; i < c; i++)
    SendMessage(hwnd,TB_DELETEBUTTON,0,0);

  for (i = 0; i < COUNTOF(tchButtons); i++)
    if (tchButtons[i] == L' ') tchButtons[i] = 0;

  p = tchButtons;
  while (*p) {
    if (swscanf_s(p,L"%i",&iCmd) == 1) {
      iCmd = (iCmd==0)?0:iCmd+cmdBase-1;
      for (i = 0; i < ctbb; i++) {
        if (ptbb[i].idCommand == iCmd) {
          SendMessage(hwnd,TB_ADDBUTTONS,(WPARAM)1,(LPARAM)&ptbb[i]);
          break;
        }
      }
    }
    p = StrEnd(p)+1;
  }
  return((int)SendMessage(hwnd,TB_BUTTONCOUNT,0,0));
}


//=============================================================================
//
//  IsCmdEnabled()
//
BOOL IsCmdEnabled(HWND hwnd,UINT uId)
{

  HMENU hmenu;
  UINT ustate;

  hmenu = GetMenu(hwnd);

  SendMessage(hwnd,WM_INITMENU,(WPARAM)hmenu,0);

  ustate = GetMenuState(hmenu,uId,MF_BYCOMMAND);

  if (ustate == 0xFFFFFFFF)
    return TRUE;

  else
    return (!(ustate & (MF_GRAYED|MF_DISABLED)));

}


//=============================================================================
//
//  FormatString()
//
int FormatString(LPWSTR lpOutput,int nOutput,UINT uIdFormat,...)
{

  WCHAR *p = LocalAlloc(LPTR,sizeof(WCHAR)*nOutput);

  if (GetString(uIdFormat,p,nOutput))
    StringCchVPrintf(lpOutput,nOutput,p,(LPVOID)((PUINT_PTR)&uIdFormat + 1));

  LocalFree(p);

  return StringCchLenN(lpOutput,nOutput);

}


//=============================================================================
//
//  GetKnownFolderPath()
//
BOOL GetKnownFolderPath(REFKNOWNFOLDERID rfid, LPWSTR lpOutPath, size_t cchCount)
{
  //const DWORD dwFlags = (KF_FLAG_DEFAULT_PATH | KF_FLAG_NOT_PARENT_RELATIVE | KF_FLAG_NO_ALIAS);
  const DWORD dwFlags = KF_FLAG_NO_ALIAS;

  PWSTR pszPath = NULL;
  HRESULT hr = SHGetKnownFolderPath(rfid, dwFlags, NULL, &pszPath);
  if (SUCCEEDED(hr) && pszPath) {
    StringCchCopy(lpOutPath, cchCount, pszPath);
    CoTaskMemFree(pszPath);
    return TRUE;
  }
  return FALSE;
}


//=============================================================================
//
//  PathRelativeToApp()
//
void PathRelativeToApp(
  LPWSTR lpszSrc,LPWSTR lpszDest,int cchDest,BOOL bSrcIsFile,
  BOOL bUnexpandEnv,BOOL bUnexpandMyDocs) {

  WCHAR wchAppPath[MAX_PATH] = { L'\0' };
  WCHAR wchWinDir[MAX_PATH] = { L'\0' };
  WCHAR wchUserFiles[MAX_PATH] = { L'\0' };
  WCHAR wchPath[MAX_PATH] = { L'\0' };
  WCHAR wchResult[MAX_PATH] = { L'\0' };
  DWORD dwAttrTo = (bSrcIsFile) ? 0 : FILE_ATTRIBUTE_DIRECTORY;

  GetModuleFileName(NULL,wchAppPath,COUNTOF(wchAppPath));
  PathCanonicalizeEx(wchAppPath,MAX_PATH);
  PathCchRemoveFileSpec(wchAppPath,COUNTOF(wchAppPath));
  GetWindowsDirectory(wchWinDir,COUNTOF(wchWinDir));
  //SHGetFolderPath(NULL,CSIDL_PERSONAL,NULL,SHGFP_TYPE_CURRENT,wchUserFiles);
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
void PathAbsoluteFromApp(LPWSTR lpszSrc,LPWSTR lpszDest,int cchDest,BOOL bExpandEnv) {

  WCHAR wchPath[MAX_PATH] = { L'\0'};
  WCHAR wchResult[MAX_PATH] = { L'\0'};

  if (lpszSrc == NULL) {
    ZeroMemory(lpszDest, (cchDest == 0) ? MAX_PATH : cchDest);
    return;
  }

  if (StrCmpNI(lpszSrc,L"%CSIDL:MYDOCUMENTS%",CSTRLEN("%CSIDL:MYDOCUMENTS%")) == 0) {
    //SHGetFolderPath(NULL,CSIDL_PERSONAL,NULL,SHGFP_TYPE_CURRENT,wchPath);
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
    PathCanonicalizeEx(wchResult,MAX_PATH);
    PathRemoveFileSpec(wchResult);
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
BOOL PathIsLnkFile(LPCWSTR pszPath)
{
  WCHAR tchResPath[MAX_PATH] = { L'\0' };

  if (!pszPath || !*pszPath)
    return FALSE;

  if (StringCchCompareIX(PathFindExtension(pszPath),L".lnk"))
    return FALSE;
  else
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
BOOL PathGetLnkPath(LPCWSTR pszLnkFile,LPWSTR pszResPath,int cchResPath)
{

  IShellLink       *psl;
  WIN32_FIND_DATA  fd;
  BOOL             bSucceeded = FALSE;

  if (SUCCEEDED(CoCreateInstance(&CLSID_ShellLink,NULL,
                                 CLSCTX_INPROC_SERVER,
                                 &IID_IShellLink,&psl)))
  {
    IPersistFile *ppf;

    if (SUCCEEDED(psl->lpVtbl->QueryInterface(psl,&IID_IPersistFile,&ppf)))
    {
      WORD wsz[MAX_PATH] = { L'\0' };

      /*MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,pszLnkFile,-1,wsz,MAX_PATH);*/
      StringCchCopy(wsz,COUNTOF(wsz),pszLnkFile);

      if (SUCCEEDED(ppf->lpVtbl->Load(ppf,wsz,STGM_READ)))
      {
        if (NOERROR == psl->lpVtbl->GetPath(psl,pszResPath,cchResPath,&fd,0))
          bSucceeded = TRUE;
      }
      ppf->lpVtbl->Release(ppf);
    }
    psl->lpVtbl->Release(psl);
  }

  // This additional check seems reasonable
  if (!StringCchLenN(pszResPath,cchResPath))
    bSucceeded = FALSE;

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
BOOL PathIsLnkToDirectory(LPCWSTR pszPath,LPWSTR pszResPath,int cchResPath)
{

  WCHAR tchResPath[MAX_PATH] = { L'\0' };

  if (PathIsLnkFile(pszPath)) 
  {
    if (PathGetLnkPath(pszPath,tchResPath,sizeof(WCHAR)*COUNTOF(tchResPath))) 
    {
      if (PathIsDirectory(tchResPath)) 
      {
        StringCchCopyN(pszResPath,cchResPath,tchResPath,COUNTOF(tchResPath));
        return (TRUE);
      }
      else
        return FALSE;
      }
    else
      return FALSE;
    }
  else
    return FALSE;

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
BOOL PathCreateDeskLnk(LPCWSTR pszDocument)
{

  WCHAR tchExeFile[MAX_PATH] = { L'\0' };
  WCHAR tchDocTemp[MAX_PATH] = { L'\0' };
  WCHAR tchArguments[MAX_PATH+16] = { L'\0' };
  WCHAR tchLinkDir[MAX_PATH] = { L'\0' };
  WCHAR tchDescription[64] = { L'\0' };

  WCHAR tchLnkFileName[MAX_PATH] = { L'\0' };

  IShellLink *psl;
  BOOL bSucceeded = FALSE;
  BOOL fMustCopy;

  if (!pszDocument || StringCchLenN(pszDocument,MAX_PATH) == 0)
    return TRUE;

  // init strings
  GetModuleFileName(NULL,tchExeFile,COUNTOF(tchExeFile));

  StringCchCopy(tchDocTemp,COUNTOF(tchDocTemp),pszDocument);
  PathQuoteSpaces(tchDocTemp);

  StringCchCopy(tchArguments,COUNTOF(tchArguments),L"-n ");
  StringCchCat(tchArguments,COUNTOF(tchArguments),tchDocTemp);

  //SHGetSpecialFolderPath(NULL,tchLinkDir,CSIDL_DESKTOPDIRECTORY,TRUE);
  GetKnownFolderPath(&FOLDERID_Desktop, tchLinkDir, COUNTOF(tchLinkDir));

  GetString(IDS_LINKDESCRIPTION,tchDescription,COUNTOF(tchDescription));

  // Try to construct a valid filename...
  if (!SHGetNewLinkInfo(pszDocument,tchLinkDir,tchLnkFileName,&fMustCopy,SHGNLI_PREFIXNAME))
    return(FALSE);

  if (SUCCEEDED(CoCreateInstance(&CLSID_ShellLink,NULL,
                                 CLSCTX_INPROC_SERVER,
                                 &IID_IShellLink,&psl)))
  {
    IPersistFile *ppf;

    if (SUCCEEDED(psl->lpVtbl->QueryInterface(psl,&IID_IPersistFile,&ppf)))
    {
      WORD wsz[MAX_PATH] = { L'\0' };

      /*MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,tchLnkFileName,-1,wsz,MAX_PATH);*/
      StringCchCopy(wsz,COUNTOF(wsz),tchLnkFileName);

      psl->lpVtbl->SetPath(psl,tchExeFile);
      psl->lpVtbl->SetArguments(psl,tchArguments);
      psl->lpVtbl->SetDescription(psl,tchDescription);

      if (SUCCEEDED(ppf->lpVtbl->Save(ppf,wsz,TRUE)))
        bSucceeded = TRUE;

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
BOOL PathCreateFavLnk(LPCWSTR pszName,LPCWSTR pszTarget,LPCWSTR pszDir)
{

  WCHAR tchLnkFileName[MAX_PATH] = { L'\0' };

  IShellLink *psl;
  BOOL bSucceeded = FALSE;

  if (!pszName || StringCchLenN(pszName,MAX_PATH) == 0)
    return TRUE;

  StringCchCopy(tchLnkFileName,COUNTOF(tchLnkFileName),pszDir);
  PathCchAppend(tchLnkFileName,COUNTOF(tchLnkFileName),pszName);
  StringCchCat(tchLnkFileName,COUNTOF(tchLnkFileName),L".lnk");

  if (PathFileExists(tchLnkFileName))
    return FALSE;

  if (SUCCEEDED(CoCreateInstance(&CLSID_ShellLink,NULL,
                                 CLSCTX_INPROC_SERVER,
                                 &IID_IShellLink,&psl)))
  {
    IPersistFile *ppf;

    if (SUCCEEDED(psl->lpVtbl->QueryInterface(psl,&IID_IPersistFile,&ppf)))
    {
      WORD wsz[MAX_PATH] = { L'\0' };

      /*MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,tchLnkFileName,-1,wsz,MAX_PATH);*/
      StringCchCopy(wsz,COUNTOF(wsz),tchLnkFileName);

      psl->lpVtbl->SetPath(psl,pszTarget);

      if (SUCCEEDED(ppf->lpVtbl->Save(ppf,wsz,TRUE)))
        bSucceeded = TRUE;

      ppf->lpVtbl->Release(ppf);
    }
    psl->lpVtbl->Release(psl);
  }

  return(bSucceeded);

}


//=============================================================================
//
//  StrLTrim()
//
BOOL StrLTrim(LPWSTR pszSource,LPCWSTR pszTrimChars)
{
  LPWSTR psz;

  if (!pszSource || !*pszSource)
    return FALSE;

  psz = pszSource;
  while (StrChrI(pszTrimChars,*psz))
    psz++;

  MoveMemory(pszSource,psz,sizeof(WCHAR)*(lstrlen(psz) + 1));

  return TRUE;
}


//=============================================================================
//
//  TrimString()
//
BOOL TrimString(LPWSTR lpString)
{

  LPWSTR psz;

  if (!lpString || !*lpString)
    return FALSE;

  // Trim left
  psz = lpString;

  while (*psz == L' ')
    psz = CharNext(psz);

  MoveMemory(lpString,psz,sizeof(WCHAR)*(lstrlen(psz) + 1));

  // Trim right
  psz = StrEnd(lpString);

  while (*(psz = CharPrev(lpString,psz)) == L' ')
    *psz = L'\0';

  return TRUE;

}


//=============================================================================
//
//  ExtractFirstArgument()
//
BOOL ExtractFirstArgument(LPCWSTR lpArgs,LPWSTR lpArg1,LPWSTR lpArg2,int len)
{

  LPWSTR psz;
  BOOL bQuoted = FALSE;

  StringCchCopy(lpArg1,len,lpArgs);

  if (lpArg2)
    *lpArg2 = L'\0';

  if (!TrimString(lpArg1))
    return FALSE;

  if (*lpArg1 == L'\"')
  {
    *lpArg1 = L' ';
    TrimString(lpArg1);
    bQuoted = TRUE;
  }

  if (bQuoted)
    psz = StrChr(lpArg1,L'\"');
  else
    psz = StrChr(lpArg1,L' ');

  if (psz)
  {
    *psz = L'\0';
    if (lpArg2)
      StringCchCopy(lpArg2,len,psz + 1);
  }

  TrimString(lpArg1);

  if (lpArg2)
    TrimString(lpArg2);

  return TRUE;

}


//=============================================================================
//
//  PrepareFilterStr()
//
void PrepareFilterStr(LPWSTR lpFilter)
{
  LPWSTR psz = StrEnd(lpFilter);
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
void ExpandEnvironmentStringsEx(LPWSTR lpSrc,DWORD dwSrc)
{
  WCHAR szBuf[LARGE_BUFFER];

  if (ExpandEnvironmentStrings(lpSrc,szBuf,COUNTOF(szBuf)))
    StringCchCopyN(lpSrc,dwSrc,szBuf,COUNTOF(szBuf));
}


//=============================================================================
//
//  PathCanonicalizeEx()
//
//
void PathCanonicalizeEx(LPWSTR lpszPath,int len)
{
  WCHAR szDst[FILE_ARG_BUF] = { L'\0' };
  if (PathCchCanonicalize(szDst,len,lpszPath) == S_OK)
    StringCchCopy(lpszPath,len,szDst);
}


//=============================================================================
//
//  GetLongPathNameEx()
//
//
DWORD GetLongPathNameEx(LPWSTR lpszPath,DWORD cchBuffer)
{
  DWORD dwRet = GetLongPathName(lpszPath,lpszPath,cchBuffer);
  if (dwRet) {
    if (PathGetDriveNumber(lpszPath) != -1)
      CharUpperBuff(lpszPath,1);
    return(dwRet);
  }
  return(0);
}


//=============================================================================
//
//  NormalizePathEx()
//
//
DWORD NormalizePathEx(LPWSTR lpszPath,int len)
{
  PathCanonicalizeEx(lpszPath,len);
  return GetLongPathNameEx(lpszPath,(DWORD)len);
}



//=============================================================================
//
//  SHGetFileInfo2()
//
//  Return a default name when the file has been removed, and always append
//  a filename extension
//
DWORD_PTR SHGetFileInfo2(LPCWSTR pszPath,DWORD dwFileAttributes,
                         SHFILEINFO *psfi,UINT cbFileInfo,UINT uFlags)
{

  if (PathFileExists(pszPath)) {

    DWORD_PTR dw = SHGetFileInfo(pszPath,dwFileAttributes,psfi,cbFileInfo,uFlags);
    if (StringCchLen(psfi->szDisplayName) < StringCchLenN(PathFindFileName(pszPath),MAX_PATH))
      StringCchCat(psfi->szDisplayName,COUNTOF(psfi->szDisplayName),PathFindExtension(pszPath));
    return(dw);
  }

  else {
    DWORD_PTR dw = SHGetFileInfo(pszPath,FILE_ATTRIBUTE_NORMAL,psfi,cbFileInfo,uFlags|SHGFI_USEFILEATTRIBUTES);
    if (StringCchLen(psfi->szDisplayName) < StringCchLenN(PathFindFileName(pszPath),MAX_PATH))
      StringCchCat(psfi->szDisplayName,COUNTOF(psfi->szDisplayName),PathFindExtension(pszPath));
    return(dw);
  }

}


//=============================================================================
//
//  FormatNumberStr()
//
int FormatNumberStr(LPWSTR lpNumberStr)
{
  WCHAR *c;
  WCHAR szSep[8];
  int  i = 0;

  if (!lstrlen(lpNumberStr))
    return(0);

  if (!GetLocaleInfo(LOCALE_USER_DEFAULT,
                     LOCALE_STHOUSAND,
                     szSep,
                     COUNTOF(szSep)))
    szSep[0] = L'\'';

  c = StrEnd(lpNumberStr);

  while ((c = CharPrev(lpNumberStr,c)) != lpNumberStr)
  {
    if (++i == 3)
    {
      i = 0;
      MoveMemory(c+1,c,sizeof(WCHAR)*(lstrlen(c)+1));
      *c = szSep[0];
    }
  }

  return(lstrlen(lpNumberStr));
}


//=============================================================================
//
//  SetDlgItemIntEx()
//
BOOL SetDlgItemIntEx(HWND hwnd,int nIdItem,UINT uValue)
{
  WCHAR szBuf[64] = { L'\0' };

  StringCchPrintf(szBuf,COUNTOF(szBuf),L"%u",uValue);
  FormatNumberStr(szBuf);

  return(SetDlgItemText(hwnd,nIdItem,szBuf));
}


//=============================================================================
//
//  A2W: Convert Dialog Item Text form Unicode to UTF-8 and vice versa
//
UINT GetDlgItemTextA2W(UINT uCP,HWND hDlg,int nIDDlgItem,LPSTR lpString,int nMaxCount)
{
  WCHAR wsz[1024] = L"";
  UINT uRet = GetDlgItemTextW(hDlg,nIDDlgItem,wsz,COUNTOF(wsz));
  ZeroMemory(lpString,nMaxCount);
  WideCharToMultiByte(uCP,0,wsz,-1,lpString,nMaxCount - 2,NULL,NULL);
  return uRet;
}

UINT SetDlgItemTextA2W(UINT uCP,HWND hDlg,int nIDDlgItem,LPSTR lpString)
{
  WCHAR wsz[1024] = L"";
  MultiByteToWideCharStrg(uCP,lpString,wsz);
  return SetDlgItemTextW(hDlg,nIDDlgItem,wsz);
}

LRESULT ComboBox_AddStringA2W(UINT uCP,HWND hwnd,LPCSTR lpString)
{
  WCHAR wsz[1024] = L"";
  MultiByteToWideCharStrg(uCP,lpString,wsz);
  return SendMessageW(hwnd,CB_ADDSTRING,0,(LPARAM)wsz);
}


//=============================================================================
//
//  CodePageFromCharSet()
//
UINT CodePageFromCharSet(UINT uCharSet)
{
  CHARSETINFO ci;
  if (TranslateCharsetInfo((DWORD*)(UINT_PTR)uCharSet,&ci,TCI_SRCCHARSET))
    return(ci.ciACP);
  else
    return(GetACP());
}


//=============================================================================
//
//  CharSetFromCodePage()
//
UINT CharSetFromCodePage(UINT uCodePage) {
  CHARSETINFO ci;
  if (TranslateCharsetInfo((DWORD*)(UINT_PTR)uCodePage,&ci,TCI_SRCCODEPAGE))
    return(ci.ciCharset);  // corresponds to SCI: SC_CHARSET_XXX
  else
    return(ANSI_CHARSET);
}


//=============================================================================
//
//  MRU functions
//
LPMRULIST MRU_Create(LPCWSTR pszRegKey,int iFlags,int iSize) {

  LPMRULIST pmru = LocalAlloc(LPTR,sizeof(MRULIST));
  ZeroMemory(pmru,sizeof(MRULIST));
  StringCchCopyN(pmru->szRegKey,COUNTOF(pmru->szRegKey),pszRegKey,COUNTOF(pmru->szRegKey));
  pmru->iFlags = iFlags;
  pmru->iSize = min(iSize,MRU_MAXITEMS);
  return(pmru);
}

BOOL MRU_Destroy(LPMRULIST pmru) 
{
  int i;
  for (i = 0; i < pmru->iSize; i++) {
    if (pmru->pszItems[i])
      LocalFree(pmru->pszItems[i]);
    }
  ZeroMemory(pmru,sizeof(MRULIST));
  LocalFree(pmru);
  return TRUE;
}

int MRU_Compare(LPMRULIST pmru,LPCWSTR psz1,LPCWSTR psz2) 
{
  if (pmru->iFlags & MRU_NOCASE)
    return(StringCchCompareIX(psz1,psz2));
  else
    return(StringCchCompareX(psz1,psz2));
}

BOOL MRU_Add(LPMRULIST pmru,LPCWSTR pszNew, int iEnc, int iPos) 
{
  int i;
  for (i = 0; i < pmru->iSize; i++) {
    if (MRU_Compare(pmru,pmru->pszItems[i],pszNew) == 0) {
      LocalFree(pmru->pszItems[i]);
      break;
    }
  }
  i = min(i,pmru->iSize-1);
  for (; i > 0; i--) {
    pmru->pszItems[i] = pmru->pszItems[i - 1];
    pmru->iEncoding[i] = pmru->iEncoding[i - 1];
    pmru->iCaretPos[i] = pmru->iCaretPos[i - 1];
  }
  pmru->pszItems[0] = StrDup(pszNew);

  pmru->iEncoding[0] = iEnc;
  pmru->iCaretPos[0] = (bPreserveCaretPos) ? iPos : 0;

  return TRUE;
}

BOOL MRU_FindFile(LPMRULIST pmru,LPCWSTR pszFile,int* iIndex) {
  WCHAR wchItem[MAX_PATH] = { L'\0' };
  int i;
  for (i = 0; i < pmru->iSize; i++) {
    if (pmru->pszItems[i] == NULL) {
      *iIndex = i;
      return FALSE;
    }
    else if (StringCchCompareIX(pmru->pszItems[i],pszFile) == 0) {
      *iIndex = i;
      return TRUE;
    }
    else {
      PathAbsoluteFromApp(pmru->pszItems[i],wchItem,COUNTOF(wchItem),TRUE);
      if (StringCchCompareIN(wchItem,COUNTOF(wchItem),pszFile,-1) == 0) {
        *iIndex = i;
        return TRUE;
      }
    }
  }
  *iIndex = i;
  return FALSE;
}

BOOL MRU_AddFile(LPMRULIST pmru,LPCWSTR pszFile,BOOL bRelativePath,BOOL bUnexpandMyDocs,int iEnc,int iPos) {

  int i;
  if (MRU_FindFile(pmru,pszFile,&i)) {
    LocalFree(pmru->pszItems[i]);
  }
  else {
    i = (i < pmru->iSize) ? i : (pmru->iSize - 1);
  }
  for (; i > 0; i--) {
    pmru->pszItems[i] = pmru->pszItems[i - 1];
    pmru->iEncoding[i] = pmru->iEncoding[i - 1];
    pmru->iCaretPos[i] = pmru->iCaretPos[i - 1];
  }
  if (bRelativePath) {
    WCHAR wchFile[MAX_PATH] = { L'\0' };
    PathRelativeToApp((LPWSTR)pszFile,wchFile,COUNTOF(wchFile),TRUE,TRUE,bUnexpandMyDocs);
    pmru->pszItems[0] = StrDup(wchFile);
  }
  else {
    pmru->pszItems[0] = StrDup(pszFile);
  }
  pmru->iEncoding[0] = iEnc;
  pmru->iCaretPos[0] = (bPreserveCaretPos) ? iPos : 0;

  return TRUE;
}

BOOL MRU_Delete(LPMRULIST pmru,int iIndex) {

  int i;
  if (iIndex < 0 || iIndex > pmru->iSize - 1) {
    return FALSE;
  }
  if (pmru->pszItems[iIndex]) {
    LocalFree(pmru->pszItems[iIndex]);
  }
  for (i = iIndex; i < pmru->iSize-1; i++) {
    pmru->pszItems[i] = pmru->pszItems[i + 1];
    pmru->iEncoding[i] = pmru->iEncoding[i + 1];
    pmru->iCaretPos[i] = pmru->iCaretPos[i + 1];

    pmru->pszItems[i+1] = NULL;
    pmru->iEncoding[i+1] = 0;
    pmru->iCaretPos[i+1] = 0;
  }
  return TRUE;
}

BOOL MRU_DeleteFileFromStore(LPMRULIST pmru,LPCWSTR pszFile) {

  int i = 0;
  LPMRULIST pmruStore;
  WCHAR wchItem[MAX_PATH] = { L'\0' };

  pmruStore = MRU_Create(pmru->szRegKey,pmru->iFlags,pmru->iSize);
  MRU_Load(pmruStore);

  while (MRU_Enum(pmruStore,i,wchItem,COUNTOF(wchItem)) != -1) 
  {
    PathAbsoluteFromApp(wchItem,wchItem,COUNTOF(wchItem),TRUE);
    if (StringCchCompareIN(wchItem,COUNTOF(wchItem),pszFile,-1) == 0)
      MRU_Delete(pmruStore,i);
    else
      i++;
  }

  MRU_Save(pmruStore);
  MRU_Destroy(pmruStore);
  return TRUE;
}

BOOL MRU_Empty(LPMRULIST pmru) {

  int i;
  for (i = 0; i < pmru->iSize; i++) {
    if (pmru->pszItems[i]) {
      LocalFree(pmru->pszItems[i]);
      pmru->pszItems[i] = NULL;
      pmru->iEncoding[i] = 0;
      pmru->iCaretPos[i] = 0;
    }
  }
  return TRUE;
}

int MRU_Enum(LPMRULIST pmru,int iIndex,LPWSTR pszItem,int cchItem) {

  if (pszItem == NULL || cchItem == 0) {
    int i = 0;
    while (i < pmru->iSize && pmru->pszItems[i])
      i++;
    return(i);
  }
  else {
    if (iIndex < 0 || iIndex > pmru->iSize-1 || !pmru->pszItems[iIndex])
      return(-1);
    else {
      StringCchCopyN(pszItem,cchItem,pmru->pszItems[iIndex],cchItem);
      return(StringCchLenN(pszItem,cchItem));
    }
  }
}

BOOL MRU_Load(LPMRULIST pmru) {

  WCHAR tchName[32] = { L'\0' };
  WCHAR tchItem[1024] = { L'\0' };
  WCHAR *pIniSection = LocalAlloc(LPTR,sizeof(WCHAR)*32*1024);

  MRU_Empty(pmru);
  LoadIniSection(pmru->szRegKey,pIniSection,(int)LocalSize(pIniSection)/sizeof(WCHAR));

  int n = 0;
  for (int i = 0; i < pmru->iSize; i++) {
    StringCchPrintf(tchName,COUNTOF(tchName),L"%.2i",i+1);
    if (IniSectionGetString(pIniSection,tchName,L"",tchItem,COUNTOF(tchItem))) {
      /*if (pmru->iFlags & MRU_UTF8) {
        WCHAR wchItem[1024];
        int cbw = MultiByteToWideCharStrg(CP_UTF7,tchItem,wchItem);
        WideCharToMultiByte(CP_UTF8,0,wchItem,cbw,tchItem,COUNTOF(tchItem),NULL,NULL);
        pmru->pszItems[n] = StrDup(tchItem);
      }
      else*/
        pmru->pszItems[n] = StrDup(tchItem);

        StringCchPrintf(tchName,COUNTOF(tchName),L"ENC%.2i",i + 1);
        int iCP = IniSectionGetInt(pIniSection,tchName,0);
        pmru->iEncoding[n] = Encoding_MapIniSetting(TRUE,iCP);
        StringCchPrintf(tchName,COUNTOF(tchName),L"POS%.2i",i + 1);
        pmru->iCaretPos[n] = (bPreserveCaretPos) ? IniSectionGetInt(pIniSection,tchName,0) : 0;
        ++n;
    }
  }
  LocalFree(pIniSection);
  return TRUE;
}

BOOL MRU_Save(LPMRULIST pmru) {

  int i;
  WCHAR tchName[32] = { L'\0' };
  WCHAR *pIniSection = LocalAlloc(LPTR,sizeof(WCHAR)*32*1024);

  //IniDeleteSection(pmru->szRegKey);

  for (i = 0; i < pmru->iSize; i++) {
    if (pmru->pszItems[i]) {
      StringCchPrintf(tchName,COUNTOF(tchName),L"%.2i",i + 1);
      /*if (pmru->iFlags & MRU_UTF8) {
        WCHAR  tchItem[1024];
        WCHAR wchItem[1024];
        int cbw = MultiByteToWideCharStrg(CP_UTF8,pmru->pszItems[i],wchItem);
        WideCharToMultiByte(CP_UTF7,0,wchItem,cbw,tchItem,COUNTOF(tchItem),NULL,NULL);
        IniSectionSetString(pIniSection,tchName,tchItem);
      }
      else*/
        IniSectionSetString(pIniSection,tchName,pmru->pszItems[i]);

        if (pmru->iEncoding[i] > 0) {
          StringCchPrintf(tchName,COUNTOF(tchName),L"ENC%.2i",i + 1);
          int iCP = Encoding_MapIniSetting(FALSE,pmru->iEncoding[i]);
          IniSectionSetInt(pIniSection,tchName,iCP);
        }
        if (pmru->iCaretPos[i] > 0) {
          StringCchPrintf(tchName,COUNTOF(tchName),L"POS%.2i",i + 1);
          IniSectionSetInt(pIniSection,tchName,pmru->iCaretPos[i]);
        }
    }
  }
  SaveIniSection(pmru->szRegKey,pIniSection);
  LocalFree(pIniSection);
  return TRUE;
}


BOOL MRU_MergeSave(LPMRULIST pmru,BOOL bAddFiles,BOOL bRelativePath,BOOL bUnexpandMyDocs) {

  int i;
  LPMRULIST pmruBase;

  pmruBase = MRU_Create(pmru->szRegKey,pmru->iFlags,pmru->iSize);
  MRU_Load(pmruBase);

  if (bAddFiles) {
    for (i = pmru->iSize-1; i >= 0; i--) {
      if (pmru->pszItems[i]) {
        WCHAR wchItem[MAX_PATH] = { L'\0' };
        PathAbsoluteFromApp(pmru->pszItems[i],wchItem,COUNTOF(wchItem),TRUE);
        MRU_AddFile(pmruBase,wchItem,bRelativePath,bUnexpandMyDocs,pmru->iEncoding[i],pmru->iCaretPos[i]);
      }
    }
  }

  else {
    for (i = pmru->iSize-1; i >= 0; i--) {
      if (pmru->pszItems[i])
        MRU_Add(pmruBase,pmru->pszItems[i],pmru->iEncoding[i],pmru->iCaretPos[i]);
    }
  }

  MRU_Save(pmruBase);
  MRU_Destroy(pmruBase);
  return TRUE;
}

/*

  Themed Dialogs
  Modify dialog templates to use current theme font
  Based on code of MFC helper class CDialogTemplate

*/

BOOL GetThemedDialogFont(LPWSTR lpFaceName,WORD* wSize)
{
  HDC hDC;
  int iLogPixelsY;
  HTHEME hTheme;
  LOGFONT lf;
  BOOL bSucceed = FALSE;

  hDC = GetDC(NULL);
  iLogPixelsY = GetDeviceCaps(hDC,LOGPIXELSY);
  ReleaseDC(NULL,hDC);

  HMODULE hLocalModUxTheme = GetModuleHandle(L"uxtheme.dll");
  if (hLocalModUxTheme) {
    if ((BOOL)(GetProcAddress(hLocalModUxTheme,"IsAppThemed"))()) {
      hTheme = (HTHEME)(INT_PTR)(GetProcAddress(hLocalModUxTheme,"OpenThemeData"))(NULL,L"WINDOWSTYLE;WINDOW");
      if (hTheme) {
        if (S_OK == (HRESULT)(GetProcAddress(hLocalModUxTheme,"GetThemeSysFont"))(hTheme,/*TMT_MSGBOXFONT*/805,&lf)) {
          if (lf.lfHeight < 0)
            lf.lfHeight = -lf.lfHeight;
          *wSize = (WORD)MulDiv(lf.lfHeight,72,iLogPixelsY);
          if (*wSize == 0)
            *wSize = 8;
          StringCchCopyN(lpFaceName,LF_FACESIZE,lf.lfFaceName,LF_FACESIZE);
          bSucceed = TRUE;
        }
        (GetProcAddress(hLocalModUxTheme,"CloseThemeData"))(hTheme);
      }
    }
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
    StringCchCopyN(lpFaceName,LF_FACESIZE,ncm.lfMessageFont.lfFaceName,LF_FACESIZE);
  }*/

  return(bSucceed);
}

__inline BOOL DialogTemplate_IsDialogEx(const DLGTEMPLATE* pTemplate) {

  return ((DLGTEMPLATEEX*)pTemplate)->signature == 0xFFFF;
}

__inline BOOL DialogTemplate_HasFont(const DLGTEMPLATE* pTemplate) {

  return (DS_SETFONT &
    (DialogTemplate_IsDialogEx(pTemplate) ? ((DLGTEMPLATEEX*)pTemplate)->style : pTemplate->style));
}

__inline int DialogTemplate_FontAttrSize(BOOL bDialogEx) {

  return (int)sizeof(WORD) * (bDialogEx ? 3 : 1);
}

__inline BYTE* DialogTemplate_GetFontSizeField(const DLGTEMPLATE* pTemplate) {

  BOOL bDialogEx = DialogTemplate_IsDialogEx(pTemplate);
  WORD* pw;

  if (bDialogEx)
    pw = (WORD*)((DLGTEMPLATEEX*)pTemplate + 1);
  else
    pw = (WORD*)(pTemplate + 1);

  if (*pw == (WORD)-1)
    pw += 2;
  else
    while(*pw++);

  if (*pw == (WORD)-1)
    pw += 2;
  else
    while(*pw++);

  while (*pw++);

  return (BYTE*)pw;
}

DLGTEMPLATE* LoadThemedDialogTemplate(LPCTSTR lpDialogTemplateID,HINSTANCE hInstance) {

  HRSRC hRsrc;
  HGLOBAL hRsrcMem;
  DLGTEMPLATE *pRsrcMem;
  DLGTEMPLATE *pTemplate;
  UINT dwTemplateSize = 0;
  WCHAR wchFaceName[LF_FACESIZE];
  WORD wFontSize;
  BOOL bDialogEx;
  BOOL bHasFont;
  int cbFontAttr;
  int cbNew;
  int cbOld;
  BYTE* pbNew;
  BYTE* pb;
  BYTE* pOldControls;
  BYTE* pNewControls;
  WORD nCtrl;

  hRsrc = FindResource(hInstance,lpDialogTemplateID,RT_DIALOG);
  if (hRsrc == NULL)
    return(NULL);

  hRsrcMem = LoadResource(hInstance,hRsrc);
  pRsrcMem = (DLGTEMPLATE*)LockResource(hRsrcMem);
  dwTemplateSize = (UINT)SizeofResource(hInstance,hRsrc);

  if ((dwTemplateSize == 0) ||
      (pTemplate = LocalAlloc(LPTR,dwTemplateSize+LF_FACESIZE*2)) == NULL) {
    UnlockResource(hRsrcMem);
    FreeResource(hRsrcMem);
    return(NULL);
  }

  CopyMemory((BYTE*)pTemplate,pRsrcMem,(size_t)dwTemplateSize);
  UnlockResource(hRsrcMem);
  FreeResource(hRsrcMem);

  if (!GetThemedDialogFont(wchFaceName,&wFontSize))
    return(pTemplate);

  bDialogEx  = DialogTemplate_IsDialogEx(pTemplate);
  bHasFont   = DialogTemplate_HasFont(pTemplate);
  cbFontAttr = DialogTemplate_FontAttrSize(bDialogEx);

  if (bDialogEx)
    ((DLGTEMPLATEEX*)pTemplate)->style |= DS_SHELLFONT;
  else
    pTemplate->style |= DS_SHELLFONT;

  cbNew = cbFontAttr + ((StringCchLen(wchFaceName) + 1) * sizeof(WCHAR));
  pbNew = (BYTE*)wchFaceName;

  pb = DialogTemplate_GetFontSizeField(pTemplate);
  cbOld = (int)(bHasFont ? cbFontAttr + 2 * (lstrlen((WCHAR*)(pb + cbFontAttr)) + 1) : 0);

  pOldControls = (BYTE*)(((DWORD_PTR)pb + cbOld + 3) & ~(DWORD_PTR)3);
  pNewControls = (BYTE*)(((DWORD_PTR)pb + cbNew + 3) & ~(DWORD_PTR)3);

  nCtrl = bDialogEx ?
    (WORD)((DLGTEMPLATEEX*)pTemplate)->cDlgItems :
    (WORD)pTemplate->cdit;

  if (cbNew != cbOld && nCtrl > 0)
    MoveMemory(pNewControls,pOldControls,(size_t)(dwTemplateSize - (pOldControls - (BYTE*)pTemplate)));

  *(WORD*)pb = wFontSize;
  MoveMemory(pb + cbFontAttr,pbNew,(size_t)(cbNew - cbFontAttr));

  return(pTemplate);
}

INT_PTR ThemedDialogBoxParam(
  HINSTANCE hInstance,
  LPCTSTR lpTemplate,
  HWND hWndParent,
  DLGPROC lpDialogFunc,
  LPARAM dwInitParam) {

  INT_PTR ret;
  DLGTEMPLATE *pDlgTemplate;

  pDlgTemplate = LoadThemedDialogTemplate(lpTemplate,hInstance);
  ret = DialogBoxIndirectParam(hInstance,pDlgTemplate,hWndParent,lpDialogFunc,dwInitParam);
  if (pDlgTemplate)
    LocalFree(pDlgTemplate);

  return(ret);
}

HWND CreateThemedDialogParam(
  HINSTANCE hInstance,
  LPCTSTR lpTemplate,
  HWND hWndParent,
  DLGPROC lpDialogFunc,
  LPARAM dwInitParam) {

  HWND hwnd;
  DLGTEMPLATE *pDlgTemplate;

  pDlgTemplate = LoadThemedDialogTemplate(lpTemplate,hInstance);
  hwnd = CreateDialogIndirectParam(hInstance,pDlgTemplate,hWndParent,lpDialogFunc,dwInitParam);
  if (pDlgTemplate)
    LocalFree(pDlgTemplate);

  return(hwnd);
}


/******************************************************************************
*
*  UnSlash functions
*  Mostly taken from SciTE, (c) Neil Hodgson, http://www.scintilla.org
*
/

/**
 * Is the character an octal digit?
 */
static BOOL IsOctalDigit(char ch) {
  return ch >= '0' && ch <= '7';
}

/**
 * If the character is an hexa digit, get its value.
 */
static int GetHexDigit(char ch) {
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

/**
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
        BOOL bShort = (*s == 'x');
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
            WideCharToMultiByteStrg(cpEdit,val,ch);
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
 * This is used to get control characters into the regular expresion engine.
 */
unsigned int UnSlashLowOctal(char *s) {
  char *sStart = s;
  char *o = s;
  while (*s) {
    if ((s[0] == '\\') && (s[1] == '0') && IsOctalDigit(s[2]) && IsOctalDigit(s[3])) {
      *o = (char)(8 * (s[2] - '0') + (s[3] - '0'));
      s += 3;
    } else {
      *o = *s;
    }
    o++;
    if (*s)
      s++;
  }
  *o = '\0';
  return (unsigned int)(o - sStart);
}

void TransformBackslashes(char* pszInput,BOOL bRegEx,UINT cpEdit)
{
  if (bRegEx)
    UnSlashLowOctal(pszInput);
  else
    UnSlash(pszInput,cpEdit);
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

  // OK. Haven't found a thing. Provide a default rect based on the current work
  // area
  SystemParametersInfo(SPI_GETWORKAREA,0,lpTrayRect,0);
  lpTrayRect->left=lpTrayRect->right-DEFAULT_RECT_WIDTH;
  lpTrayRect->top=lpTrayRect->bottom-DEFAULT_RECT_HEIGHT;
}

// Check to see if the animation has been disabled
/*static */BOOL GetDoAnimateMinimize(VOID)
{
  ANIMATIONINFO ai;

  ai.cbSize=sizeof(ai);
  SystemParametersInfo(SPI_GETANIMATION,sizeof(ai),&ai,0);

  return ai.iMinAnimate?TRUE:FALSE;
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
//  Encoding Helper Functions
//

int g_DOSEncoding;

// Supported Encodings
WCHAR wchANSI[16] = { L'\0' };
WCHAR wchOEM[16] = { L'\0' };

NP2ENCODING mEncoding[] = {
  { NCP_ANSI | NCP_RECODE,                               CP_ACP, "ansi,system,ascii,",                              61000, L"" },
  { NCP_OEM | NCP_RECODE,                                CP_OEMCP, "oem,oem,",                                      61001, L"" },
  { NCP_UNICODE | NCP_UNICODE_BOM,                       CP_UTF8, "",                                               61002, L"" },
  { NCP_UNICODE | NCP_UNICODE_REVERSE | NCP_UNICODE_BOM, CP_UTF8, "",                                               61003, L"" },
  { NCP_UNICODE | NCP_RECODE,                            CP_UTF8, "utf-16,utf16,unicode,",                          61004, L"" },
  { NCP_UNICODE | NCP_UNICODE_REVERSE | NCP_RECODE,      CP_UTF8, "utf-16be,utf16be,unicodebe,",                    61005, L"" },
  { NCP_UTF8 | NCP_RECODE,                               CP_UTF8, "utf-8,utf8,",                                    61006, L"" },
  { NCP_UTF8 | NCP_UTF8_SIGN,                            CP_UTF8, "utf-8,utf8,",                                    61007, L"" },
  { NCP_8BIT | NCP_RECODE,                               CP_UTF7, "utf-7,utf7,",                                    61008, L"" },
  { NCP_8BIT | NCP_RECODE, 720,   "DOS-720,dos720,",                                                                61009, L"" },
  { NCP_8BIT | NCP_RECODE, 28596, "iso-8859-6,iso88596,arabic,csisolatinarabic,ecma114,isoir127,",                  61010, L"" },
  { NCP_8BIT | NCP_RECODE, 10004, "x-mac-arabic,xmacarabic,",                                                       61011, L"" },
  { NCP_8BIT | NCP_RECODE, 1256,  "windows-1256,windows1256,cp1256",                                                61012, L"" },
  { NCP_8BIT | NCP_RECODE, 775,   "ibm775,ibm775,cp500,",                                                           61013, L"" },
  { NCP_8BIT | NCP_RECODE, 28594, "iso-8859-4,iso88594,csisolatin4,isoir110,l4,latin4,",                            61014, L"" },
  { NCP_8BIT | NCP_RECODE, 1257,  "windows-1257,windows1257,",                                                      61015, L"" },
  { NCP_8BIT | NCP_RECODE, 852,   "ibm852,ibm852,cp852,",                                                           61016, L"" },
  { NCP_8BIT | NCP_RECODE, 28592, "iso-8859-2,iso88592,csisolatin2,isoir101,latin2,l2,",                            61017, L"" },
  { NCP_8BIT | NCP_RECODE, 10029, "x-mac-ce,xmacce,",                                                               61018, L"" },
  { NCP_8BIT | NCP_RECODE, 1250,  "windows-1250,windows1250,xcp1250,",                                              61019, L"" },
  { NCP_8BIT | NCP_RECODE, 936,   "gb2312,gb2312,chinese,cngb,csgb2312,csgb231280,gb231280,gbk,",                   61020, L"" },
  { NCP_8BIT | NCP_RECODE, 10008, "x-mac-chinesesimp,xmacchinesesimp,",                                             61021, L"" },
  { NCP_8BIT | NCP_RECODE, 950,   "big5,big5,cnbig5,csbig5,xxbig5,",                                                61022, L"" },
  { NCP_8BIT | NCP_RECODE, 10002, "x-mac-chinesetrad,xmacchinesetrad,",                                             61023, L"" },
  { NCP_8BIT | NCP_RECODE, 10082, "x-mac-croatian,xmaccroatian,",                                                   61024, L"" },
  { NCP_8BIT | NCP_RECODE, 866,   "cp866,cp866,ibm866,",                                                            61025, L"" },
  { NCP_8BIT | NCP_RECODE, 28595, "iso-8859-5,iso88595,csisolatin5,csisolatincyrillic,cyrillic,isoir144,",          61026, L"" },
  { NCP_8BIT | NCP_RECODE, 20866, "koi8-r,koi8r,cskoi8r,koi,koi8,",                                                 61027, L"" },
  { NCP_8BIT | NCP_RECODE, 21866, "koi8-u,koi8u,koi8ru,",                                                           61028, L"" },
  { NCP_8BIT | NCP_RECODE, 10007, "x-mac-cyrillic,xmaccyrillic,",                                                   61029, L"" },
  { NCP_8BIT | NCP_RECODE, 1251,  "windows-1251,windows1251,xcp1251,",                                              61030, L"" },
  { NCP_8BIT | NCP_RECODE, 28603, "iso-8859-13,iso885913,",                                                         61031, L"" },
  { NCP_8BIT | NCP_RECODE, 863,   "ibm863,ibm863,",                                                                 61032, L"" },
  { NCP_8BIT | NCP_RECODE, 737,   "ibm737,ibm737,",                                                                 61033, L"" },
  { NCP_8BIT | NCP_RECODE, 28597, "iso-8859-7,iso88597,csisolatingreek,ecma118,elot928,greek,greek8,isoir126,",     61034, L"" },
  { NCP_8BIT | NCP_RECODE, 10006, "x-mac-greek,xmacgreek,",                                                         61035, L"" },
  { NCP_8BIT | NCP_RECODE, 1253,  "windows-1253,windows1253,",                                                      61036, L"" },
  { NCP_8BIT | NCP_RECODE, 869,   "ibm869,ibm869,",                                                                 61037, L"" },
  { NCP_8BIT | NCP_RECODE, 862,   "DOS-862,dos862,",                                                                61038, L"" },
  { NCP_8BIT | NCP_RECODE, 38598, "iso-8859-8-i,iso88598i,logical,",                                                61039, L"" },
  { NCP_8BIT | NCP_RECODE, 28598, "iso-8859-8,iso88598,csisolatinhebrew,hebrew,isoir138,visual,",                   61040, L"" },
  { NCP_8BIT | NCP_RECODE, 10005, "x-mac-hebrew,xmachebrew,",                                                       61041, L"" },
  { NCP_8BIT | NCP_RECODE, 1255,  "windows-1255,windows1255,",                                                      61042, L"" },
  { NCP_8BIT | NCP_RECODE, 861,   "ibm861,ibm861,",                                                                 61043, L"" },
  { NCP_8BIT | NCP_RECODE, 10079, "x-mac-icelandic,xmacicelandic,",                                                 61044, L"" },
  { NCP_8BIT | NCP_RECODE, 10001, "x-mac-japanese,xmacjapanese,",                                                   61045, L"" },
  { NCP_8BIT | NCP_RECODE, 932,   "shift_jis,shiftjis,shiftjs,csshiftjis,cswindows31j,mskanji,xmscp932,xsjis,",     61046, L"" },
  { NCP_8BIT | NCP_RECODE, 10003, "x-mac-korean,xmackorean,",                                                       61047, L"" },
  { NCP_8BIT | NCP_RECODE, 949,   "windows-949,windows949,ksc56011987,csksc5601,euckr,isoir149,korean,ksc56011989", 61048, L"" },
  { NCP_8BIT | NCP_RECODE, 28593, "iso-8859-3,iso88593,latin3,isoir109,l3,",                                        61049, L"" },
  { NCP_8BIT | NCP_RECODE, 28605, "iso-8859-15,iso885915,latin9,l9,",                                               61050, L"" },
  { NCP_8BIT | NCP_RECODE, 865,   "ibm865,ibm865,",                                                                 61051, L"" },
  { NCP_8BIT | NCP_RECODE, 437,   "ibm437,ibm437,437,cp437,cspc8,codepage437,",                                     61052, L"" },
  { NCP_8BIT | NCP_RECODE, 858,   "ibm858,ibm858,ibm00858,",                                                        61053, L"" },
  { NCP_8BIT | NCP_RECODE, 860,   "ibm860,ibm860,",                                                                 61054, L"" },
  { NCP_8BIT | NCP_RECODE, 10010, "x-mac-romanian,xmacromanian,",                                                   61055, L"" },
  { NCP_8BIT | NCP_RECODE, 10021, "x-mac-thai,xmacthai,",                                                           61056, L"" },
  { NCP_8BIT | NCP_RECODE, 874,   "windows-874,windows874,dos874,iso885911,tis620,",                                61057, L"" },
  { NCP_8BIT | NCP_RECODE, 857,   "ibm857,ibm857,",                                                                 61058, L"" },
  { NCP_8BIT | NCP_RECODE, 28599, "iso-8859-9,iso88599,latin5,isoir148,l5,",                                        61059, L"" },
  { NCP_8BIT | NCP_RECODE, 10081, "x-mac-turkish,xmacturkish,",                                                     61060, L"" },
  { NCP_8BIT | NCP_RECODE, 1254,  "windows-1254,windows1254,",                                                      61061, L"" },
  { NCP_8BIT | NCP_RECODE, 10017, "x-mac-ukrainian,xmacukrainian,",                                                 61062, L"" },
  { NCP_8BIT | NCP_RECODE, 1258,  "windows-1258,windows-258,",                                                      61063, L"" },
  { NCP_8BIT | NCP_RECODE, 850,   "ibm850,ibm850,",                                                                 61064, L"" },
  { NCP_8BIT | NCP_RECODE, 28591, "iso-8859-1,iso88591,cp819,latin1,ibm819,isoir100,latin1,l1,",                    61065, L"" },
  { NCP_8BIT | NCP_RECODE, 10000, "macintosh,macintosh,",                                                           61066, L"" },
  { NCP_8BIT | NCP_RECODE, 1252,  "windows-1252,windows1252,cp367,cp819,ibm367,us,xansi,",                          61067, L"" },
  { NCP_8BIT | NCP_RECODE, 37,    "ebcdic-cp-us,ebcdiccpus,ebcdiccpca,ebcdiccpwt,ebcdiccpnl,ibm037,cp037,",         61068, L"" },
  { NCP_8BIT | NCP_RECODE, 500,   "x-ebcdic-international,xebcdicinternational,",                                   61069, L"" },
  { NCP_8BIT | NCP_RECODE, 875,   "x-EBCDIC-GreekModern,xebcdicgreekmodern,",                                       61070, L"" },
  { NCP_8BIT | NCP_RECODE, 1026,  "CP1026,cp1026,csibm1026,ibm1026,",                                               61071, L"" },
  //{ NCP_8BIT|NCP_RECODE, 870,   "CP870,cp870,ebcdiccproece,ebcdiccpyu,csibm870,ibm870,",                          00000, L"" }, // IBM EBCDIC (Multilingual Latin-2)
  //{ NCP_8BIT|NCP_RECODE, 1047,  "IBM01047,ibm01047,",                                                             00000, L"" }, // IBM EBCDIC (Open System Latin-1)
  //{ NCP_8BIT|NCP_RECODE, 1140,  "x-ebcdic-cp-us-euro,xebcdiccpuseuro,",                                           00000, L"" }, // IBM EBCDIC (US-Canada-Euro)
  //{ NCP_8BIT|NCP_RECODE, 1141,  "x-ebcdic-germany-euro,xebcdicgermanyeuro,",                                      00000, L"" }, // IBM EBCDIC (Germany-Euro)
  //{ NCP_8BIT|NCP_RECODE, 1142,  "x-ebcdic-denmarknorway-euro,xebcdicdenmarknorwayeuro,",                          00000, L"" }, // IBM EBCDIC (Denmark-Norway-Euro)
  //{ NCP_8BIT|NCP_RECODE, 1143,  "x-ebcdic-finlandsweden-euro,xebcdicfinlandswedeneuro,",                          00000, L"" }, // IBM EBCDIC (Finland-Sweden-Euro)
  //{ NCP_8BIT|NCP_RECODE, 1144,  "x-ebcdic-italy-euro,xebcdicitalyeuro,",                                          00000, L"" }, // IBM EBCDIC (Italy-Euro)
  //{ NCP_8BIT|NCP_RECODE, 1145,  "x-ebcdic-spain-euro,xebcdicspaineuro,",                                          00000, L"" }, // IBM EBCDIC (Spain-Latin America-Euro)
  //{ NCP_8BIT|NCP_RECODE, 1146,  "x-ebcdic-uk-euro,xebcdicukeuro,",                                                00000, L"" }, // IBM EBCDIC (UK-Euro)
  //{ NCP_8BIT|NCP_RECODE, 1147,  "x-ebcdic-france-euro,xebcdicfranceeuro,",                                        00000, L"" }, // IBM EBCDIC (France-Euro)
  //{ NCP_8BIT|NCP_RECODE, 1148,  "x-ebcdic-international-euro,xebcdicinternationaleuro,",                          00000, L"" }, // IBM EBCDIC (International-Euro)
  //{ NCP_8BIT|NCP_RECODE, 1149,  "x-ebcdic-icelandic-euro,xebcdicicelandiceuro,",                                  00000, L"" }, // IBM EBCDIC (Icelandic-Euro)
  //{ NCP_8BIT|NCP_RECODE, 1361,  "johab,johab,",                                                                   00000, L"" }, // Korean (Johab)
  //{ NCP_8BIT|NCP_RECODE, 20273, "x-EBCDIC-Germany,xebcdicgermany,",                                               00000, L"" }, // IBM EBCDIC (Germany)
  //{ NCP_8BIT|NCP_RECODE, 20277, "x-EBCDIC-DenmarkNorway,xebcdicdenmarknorway,ebcdiccpdk,ebcdiccpno,",             00000, L"" }, // IBM EBCDIC (Denmark-Norway)
  //{ NCP_8BIT|NCP_RECODE, 20278, "x-EBCDIC-FinlandSweden,xebcdicfinlandsweden,ebcdicpfi,ebcdiccpse,",              00000, L"" }, // IBM EBCDIC (Finland-Sweden)
  //{ NCP_8BIT|NCP_RECODE, 20280, "x-EBCDIC-Italy,xebcdicitaly,",                                                   00000, L"" }, // IBM EBCDIC (Italy)
  //{ NCP_8BIT|NCP_RECODE, 20284, "x-EBCDIC-Spain,xebcdicspain,ebcdiccpes,",                                        00000, L"" }, // IBM EBCDIC (Spain-Latin America)
  //{ NCP_8BIT|NCP_RECODE, 20285, "x-EBCDIC-UK,xebcdicuk,ebcdiccpgb,",                                              00000, L"" }, // IBM EBCDIC (UK)
  //{ NCP_8BIT|NCP_RECODE, 20290, "x-EBCDIC-JapaneseKatakana,xebcdicjapanesekatakana,",                             00000, L"" }, // IBM EBCDIC (Japanese Katakana)
  //{ NCP_8BIT|NCP_RECODE, 20297, "x-EBCDIC-France,xebcdicfrance,ebcdiccpfr,",                                      00000, L"" }, // IBM EBCDIC (France)
  //{ NCP_8BIT|NCP_RECODE, 20420, "x-EBCDIC-Arabic,xebcdicarabic,ebcdiccpar1,",                                     00000, L"" }, // IBM EBCDIC (Arabic)
  //{ NCP_8BIT|NCP_RECODE, 20423, "x-EBCDIC-Greek,xebcdicgreek,ebcdiccpgr,",                                        00000, L"" }, // IBM EBCDIC (Greek)
  //{ NCP_8BIT|NCP_RECODE, 20424, "x-EBCDIC-Hebrew,xebcdichebrew,ebcdiccphe,",                                      00000, L"" }, // IBM EBCDIC (Hebrew)
  //{ NCP_8BIT|NCP_RECODE, 20833, "x-EBCDIC-KoreanExtended,xebcdickoreanextended,",                                 00000, L"" }, // IBM EBCDIC (Korean Extended)
  //{ NCP_8BIT|NCP_RECODE, 20838, "x-EBCDIC-Thai,xebcdicthai,ibmthai,csibmthai,",                                   00000, L"" }, // IBM EBCDIC (Thai)
  //{ NCP_8BIT|NCP_RECODE, 20871, "x-EBCDIC-Icelandic,xebcdicicelandic,ebcdiccpis,",                                00000, L"" }, // IBM EBCDIC (Icelandic)
  //{ NCP_8BIT|NCP_RECODE, 20880, "x-EBCDIC-CyrillicRussian,xebcdiccyrillicrussian,ebcdiccyrillic,",                00000, L"" }, // IBM EBCDIC (Cyrillic Russian)
  //{ NCP_8BIT|NCP_RECODE, 20905, "x-EBCDIC-Turkish,xebcdicturkish,ebcdiccptr,",                                    00000, L"" }, // IBM EBCDIC (Turkish)
  //{ NCP_8BIT|NCP_RECODE, 20924, "IBM00924,ibm00924,ebcdiclatin9euro,",                                            00000, L"" }, // IBM EBCDIC (Open System-Euro Latin-1)
  //{ NCP_8BIT|NCP_RECODE, 21025, "x-EBCDIC-CyrillicSerbianBulgarian,xebcdiccyrillicserbianbulgarian,",             00000, L"" }, // IBM EBCDIC (Cyrillic Serbian-Bulgarian)
  //{ NCP_8BIT|NCP_RECODE, 50930, "x-EBCDIC-JapaneseAndKana,xebcdicjapaneseandkana,",                               00000, L"" }, // IBM EBCDIC (Japanese and Japanese Katakana)
  //{ NCP_8BIT|NCP_RECODE, 50931, "x-EBCDIC-JapaneseAndUSCanada,xebcdicjapaneseanduscanada,",                       00000, L"" }, // IBM EBCDIC (Japanese and US-Canada)
  //{ NCP_8BIT|NCP_RECODE, 50933, "x-EBCDIC-KoreanAndKoreanExtended,xebcdickoreanandkoreanextended,",               00000, L"" }, // IBM EBCDIC (Korean and Korean Extended)
  //{ NCP_8BIT|NCP_RECODE, 50935, "x-EBCDIC-SimplifiedChinese,xebcdicsimplifiedchinese,",                           00000, L"" }, // IBM EBCDIC (Chinese Simplified)
  //{ NCP_8BIT|NCP_RECODE, 50937, "x-EBCDIC-TraditionalChinese,xebcdictraditionalchinese,",                         00000, L"" }, // IBM EBCDIC (Chinese Traditional)
  //{ NCP_8BIT|NCP_RECODE, 50939, "x-EBCDIC-JapaneseAndJapaneseLatin,xebcdicjapaneseandjapaneselatin,",             00000, L"" }, // IBM EBCDIC (Japanese and Japanese-Latin)
  //{ NCP_8BIT|NCP_RECODE, 20105, "x-IA5,xia5,",                                                                    00000, L"" }, // Western European (IA5)
  //{ NCP_8BIT|NCP_RECODE, 20106, "x-IA5-German,xia5german,",                                                       00000, L"" }, // German (IA5)
  //{ NCP_8BIT|NCP_RECODE, 20107, "x-IA5-Swedish,xia5swedish,",                                                     00000, L"" }, // Swedish (IA5)
  //{ NCP_8BIT|NCP_RECODE, 20108, "x-IA5-Norwegian,xia5norwegian,",                                                 00000, L"" }, // Norwegian (IA5)
  //{ NCP_8BIT|NCP_RECODE, 20936, "x-cp20936,xcp20936,",                                                            00000, L"" }, // Chinese Simplified (GB2312)
  //{ NCP_8BIT|NCP_RECODE, 20932, "euc-jp,,",                                                                       00000, L"" }, // Japanese (JIS X 0208-1990 & 0212-1990)
  //{ NCP_8BIT|NCP_RECODE, 50220, "iso-2022-jp,iso2022jp,",                                                         00000, L"" }, // Japanese (JIS)
  //{ NCP_8BIT|NCP_RECODE, 50221, "csISO2022JP,csiso2022jp,",                                                       00000, L"" }, // Japanese (JIS-Allow 1 byte Kana)
  //{ NCP_8BIT|NCP_RECODE, 50222, "_iso-2022-jp$SIO,iso2022jpSIO,",                                                 00000, L"" }, // Japanese (JIS-Allow 1 byte Kana - SO/SI)
  //{ NCP_8BIT|NCP_RECODE, 50225, "iso-2022-kr,iso2022kr,csiso2022kr,",                                             00000, L"" }, // Korean (ISO-2022-KR)
  //{ NCP_8BIT|NCP_RECODE, 50227, "x-cp50227,xcp50227,",                                                            00000, L"" }, // Chinese Simplified (ISO-2022)
  //{ NCP_8BIT|NCP_RECODE, 50229, "iso-2022-cn,iso2022cn,",                                                         00000, L"" }, // Chinese Traditional (ISO-2022)
  //{ NCP_8BIT|NCP_RECODE, 20000, "x-Chinese-CNS,xchinesecns,",                                                     00000, L"" }, // Chinese Traditional (CNS)
  //{ NCP_8BIT|NCP_RECODE, 20002, "x-Chinese-Eten,xchineseeten,",                                                   00000, L"" }, // Chinese Traditional (Eten)
  //{ NCP_8BIT|NCP_RECODE, 51932, "euc-jp,eucjp,xeuc,xeucjp,",                                                      00000, L"" }, // Japanese (EUC)
  //{ NCP_8BIT|NCP_RECODE, 51936, "euc-cn,euccn,xeuccn,",                                                           00000, L"" }, // Chinese Simplified (EUC)
  //{ NCP_8BIT|NCP_RECODE, 51949, "euc-kr,euckr,cseuckr,",                                                          00000, L"" }, // Korean (EUC)
  //{ NCP_8BIT|NCP_RECODE, 52936, "hz-gb-2312,hzgb2312,hz,",                                                        00000, L"" }, // Chinese Simplified (HZ-GB2312)
  { NCP_8BIT | NCP_RECODE, 54936, "gb18030,gb18030,",                                                               61072, L"" }  // Chinese Simplified (GB18030)
                                                                                                                                  //{ NCP_8BIT|NCP_RECODE, 57002, "x-iscii-de,xisciide,",                                                           00000, L"" }, // ISCII Devanagari
                                                                                                                                  //{ NCP_8BIT|NCP_RECODE, 57003, "x-iscii-be,xisciibe,",                                                           00000, L"" }, // ISCII Bengali
                                                                                                                                  //{ NCP_8BIT|NCP_RECODE, 57004, "x-iscii-ta,xisciita,",                                                           00000, L"" }, // ISCII Tamil
                                                                                                                                  //{ NCP_8BIT|NCP_RECODE, 57005, "x-iscii-te,xisciite,",                                                           00000, L"" }, // ISCII Telugu
                                                                                                                                  //{ NCP_8BIT|NCP_RECODE, 57006, "x-iscii-as,xisciias,",                                                           00000, L"" }, // ISCII Assamese
                                                                                                                                  //{ NCP_8BIT|NCP_RECODE, 57007, "x-iscii-or,xisciior,",                                                           00000, L"" }, // ISCII Oriya
                                                                                                                                  //{ NCP_8BIT|NCP_RECODE, 57008, "x-iscii-ka,xisciika,",                                                           00000, L"" }, // ISCII Kannada
                                                                                                                                  //{ NCP_8BIT|NCP_RECODE, 57009, "x-iscii-ma,xisciima,",                                                           00000, L"" }, // ISCII Malayalam
                                                                                                                                  //{ NCP_8BIT|NCP_RECODE, 57010, "x-iscii-gu,xisciigu,",                                                           00000, L"" }, // ISCII Gujarathi
                                                                                                                                  //{ NCP_8BIT|NCP_RECODE, 57011, "x-iscii-pa,xisciipa,",                                                           00000, L"" }, // ISCII Panjabi
};


int Encoding_CountOf() {
  return COUNTOF(mEncoding);
}

int Encoding_Current(int iEncoding) {
  static int CurrentEncoding = CPI_NONE;

  if (iEncoding >= 0) {
    if (Encoding_IsValid(iEncoding))
      CurrentEncoding = iEncoding;
    else
      CurrentEncoding = CPI_UTF8;
  }
  return CurrentEncoding;
}


int Encoding_Source(int iSrcEncoding) {
  static int SourceEncoding = CPI_NONE;

  if (iSrcEncoding >= 0) {
    if (Encoding_IsValid(iSrcEncoding))
      SourceEncoding = iSrcEncoding;
    else
      SourceEncoding = CPI_UTF8;
  }
  else if (iSrcEncoding == CPI_NONE) {
    SourceEncoding = CPI_NONE;
  }
  return SourceEncoding;
}


int  Encoding_SrcWeak(int iSrcWeakEnc) {
  static int SourceWeakEncoding = CPI_NONE;

  if (iSrcWeakEnc >= 0) {
    if (Encoding_IsValid(iSrcWeakEnc))
      SourceWeakEncoding = iSrcWeakEnc;
    else
      SourceWeakEncoding = CPI_UTF8;
  }
  else if (iSrcWeakEnc == CPI_NONE) {
    SourceWeakEncoding = CPI_NONE;
  }
  return SourceWeakEncoding;
}


BOOL Encoding_HasChanged(int iOriginalEncoding) {
  static int OriginalEncoding = CPI_NONE;

  if (iOriginalEncoding >= CPI_NONE) {
    OriginalEncoding = iOriginalEncoding;
  }
  return (BOOL)(OriginalEncoding != Encoding_Current(CPI_GET));
}


void Encoding_InitDefaults() {
  const UINT uCodePageMBCS[20] = {
    42, // (Symbol)
    50220,50221,50222,50225,50227,50229, // (Chinese, Japanese, Korean) 
    54936, // (GB18030)
    57002,57003,57004,57005,57006,57007,57008,57009,57010,57011, // (ISCII)
    65000, // (UTF-7)
    65001  // (UTF-8)
  };

  mEncoding[CPI_ANSI_DEFAULT].uCodePage = GetACP(); // set ANSI system CP
  StringCchPrintf(wchANSI,COUNTOF(wchANSI),L" (CP-%u)",mEncoding[CPI_ANSI_DEFAULT].uCodePage);

  for (int i = CPI_UTF7 + 1; i < COUNTOF(mEncoding); ++i) {
    if (Encoding_IsValid(i) && (mEncoding[i].uCodePage == mEncoding[CPI_ANSI_DEFAULT].uCodePage)) {
      mEncoding[i].uFlags |= NCP_ANSI;
      if (mEncoding[i].uFlags & NCP_8BIT)
        mEncoding[CPI_ANSI_DEFAULT].uFlags |= NCP_8BIT;
      break;
    }
  }

  mEncoding[CPI_OEM].uCodePage = GetOEMCP();
  StringCchPrintf(wchOEM,COUNTOF(wchOEM),L" (CP-%u)",mEncoding[CPI_OEM].uCodePage);

  for (int i = CPI_UTF7 + 1; i < COUNTOF(mEncoding); ++i) {
    if (Encoding_IsValid(i) && (mEncoding[i].uCodePage == mEncoding[CPI_OEM].uCodePage)) {
      mEncoding[i].uFlags |= NCP_OEM;
      if (mEncoding[i].uFlags & NCP_8BIT)
        mEncoding[CPI_OEM].uFlags |= NCP_8BIT;
      break;
    }
  }

  // multi byte character sets
  for (int i = 0; i < COUNTOF(mEncoding); ++i) {
    for (int k = 0; k < COUNTOF(uCodePageMBCS); k++) {
      if (mEncoding[i].uCodePage == uCodePageMBCS[k]) {
        mEncoding[i].uFlags |= NCP_MBCS;
      }
    }
  }

  g_DOSEncoding = CPI_OEM;
  // Try to set the DOS encoding to DOS-437 if the default OEMCP is not DOS-437
  if (mEncoding[g_DOSEncoding].uCodePage != 437) {
    for (int i = CPI_UTF7 + 1; i < COUNTOF(mEncoding); ++i) {
      if (Encoding_IsValid(i) && (mEncoding[i].uCodePage == 437)) {
        g_DOSEncoding = i;
        break;
      }
    }
  }
}


int Encoding_MapIniSetting(BOOL bLoad,int iSetting) {
  if (bLoad) {
    switch (iSetting) {
    case -1: return CPI_NONE;
    case  0: return CPI_ANSI_DEFAULT;
    case  1: return CPI_UNICODEBOM;
    case  2: return CPI_UNICODEBEBOM;
    case  3: return CPI_UTF8;
    case  4: return CPI_UTF8SIGN;
    case  5: return CPI_OEM;
    case  6: return CPI_UNICODE;
    case  7: return CPI_UNICODEBE;
    case  8: return CPI_UTF7;
    default: {
        for (int i = CPI_UTF7 + 1; i < COUNTOF(mEncoding); i++) {
          if ((mEncoding[i].uCodePage == (UINT)iSetting) && Encoding_IsValid(i))
            return(i);
        }
        return CPI_ANSI_DEFAULT;
      }
    }
  }
  else {
    switch (iSetting) {
    case CPI_NONE:         return -1;
    case CPI_ANSI_DEFAULT: return  0;
    case CPI_UNICODEBOM:   return  1;
    case CPI_UNICODEBEBOM: return  2;
    case CPI_UTF8:         return  3;
    case CPI_UTF8SIGN:     return  4;
    case CPI_OEM:          return  5;
    case CPI_UNICODE:      return  6;
    case CPI_UNICODEBE:    return  7;
    case CPI_UTF7:         return  8;
    default: {
        if (Encoding_IsValid(iSetting))
          return(mEncoding[iSetting].uCodePage);
        else
          return CPI_ANSI_DEFAULT;
      }
    }
  }
}

int Encoding_MapUnicode(int iUni) {

  if (iUni == CPI_UNICODEBOM)
    return CPI_UNICODE;
  else if (iUni == CPI_UNICODEBEBOM)
    return CPI_UNICODEBE;
  else if (iUni == CPI_UTF8SIGN)
    return CPI_UTF8;
  else
    return iUni;
}

void Encoding_GetLabel(int iEncoding) {
  if (mEncoding[iEncoding].wchLabel[0] == L'\0') {
    WCHAR wch1[128] = { L'\0' };
    WCHAR wch2[128] = { L'\0' };
    GetString(mEncoding[iEncoding].idsName,wch1,COUNTOF(wch1));
    WCHAR *pwsz = StrChr(wch1,L';');
    if (pwsz) {
      pwsz = StrChr(CharNext(pwsz),L';');
      if (pwsz) {
        pwsz = CharNext(pwsz);
      }
    }
    if (!pwsz)
      pwsz = wch1;

    StringCchCopyN(wch2,COUNTOF(wch2),pwsz,COUNTOF(wch1));

    if (Encoding_IsANSI(iEncoding))
      StringCchCatN(wch2,COUNTOF(wch2),wchANSI,COUNTOF(wchANSI));
    else if (Encoding_IsOEM(iEncoding))
      StringCchCatN(wch2,COUNTOF(wch2),wchOEM,COUNTOF(wchOEM));

    StringCchCopyN(mEncoding[iEncoding].wchLabel,COUNTOF(mEncoding[iEncoding].wchLabel),
      wch2,COUNTOF(mEncoding[iEncoding].wchLabel));
  }
}


int Encoding_MatchW(LPCWSTR pwszTest) {
  char tchTest[256] = { '\0' };
  WideCharToMultiByteStrg(CP_ACP,pwszTest,tchTest);
  return(Encoding_MatchA(tchTest));
}


int Encoding_MatchA(char *pchTest) {
  char  chTest[256] = { '\0' };
  char *pchSrc = pchTest;
  char *pchDst = chTest;
  *pchDst++ = ',';
  while (*pchSrc) {
    if (IsCharAlphaNumericA(*pchSrc))
      *pchDst++ = *CharLowerA(pchSrc);
    pchSrc++;
  }
  *pchDst++ = ',';
  *pchDst = 0;
  for (int i = 0; i < COUNTOF(mEncoding); i++) {
    if (StrStrIA(mEncoding[i].pszParseNames,chTest)) {
      CPINFO cpi;
      if ((mEncoding[i].uFlags & NCP_INTERNAL) ||
        IsValidCodePage(mEncoding[i].uCodePage) &&
        GetCPInfo(mEncoding[i].uCodePage,&cpi))
        return(i);
      else
        return(-1);
    }
  }
  return(-1);
}


int Encoding_GetByCodePage(UINT cp) {
  for (int i = 0; i < COUNTOF(mEncoding); i++) {
    if (cp == mEncoding[i].uCodePage) {
      return i;
    }
  }
  return CPI_ANSI_DEFAULT;
}


BOOL Encoding_IsValid(int iTestEncoding) {
  CPINFO cpi;
  if ((iTestEncoding >= 0) && (iTestEncoding < COUNTOF(mEncoding))) {
    if ((mEncoding[iTestEncoding].uFlags & NCP_INTERNAL) ||
      IsValidCodePage(mEncoding[iTestEncoding].uCodePage) &&
      GetCPInfo(mEncoding[iTestEncoding].uCodePage,&cpi)) {
      return(TRUE);
    }
  }
  return(FALSE);
}


typedef struct _ee {
  int    id;
  WCHAR  wch[256];
} ENCODINGENTRY,*PENCODINGENTRY;

int CmpEncoding(const void *s1,const void *s2) {
  return StrCmp(((PENCODINGENTRY)s1)->wch,((PENCODINGENTRY)s2)->wch);
}

void Encoding_AddToListView(HWND hwnd,int idSel,BOOL bRecodeOnly) {
  int i;
  int iSelItem = -1;
  LVITEM lvi;
  WCHAR wchBuf[256] = { L'\0' };

  PENCODINGENTRY pEE = LocalAlloc(LPTR,COUNTOF(mEncoding) * sizeof(ENCODINGENTRY));
  for (i = 0; i < COUNTOF(mEncoding); i++) {
    pEE[i].id = i;
    GetString(mEncoding[i].idsName,pEE[i].wch,COUNTOF(pEE[i].wch));
  }
  qsort(pEE,COUNTOF(mEncoding),sizeof(ENCODINGENTRY),CmpEncoding);

  ZeroMemory(&lvi,sizeof(LVITEM));
  lvi.mask = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE;
  lvi.pszText = wchBuf;

  for (i = 0; i < COUNTOF(mEncoding); i++) {

    int id = pEE[i].id;
    if (!bRecodeOnly || (mEncoding[id].uFlags & NCP_RECODE)) {

      lvi.iItem = ListView_GetItemCount(hwnd);

      WCHAR *pwsz = StrChr(pEE[i].wch,L';');
      if (pwsz) {
        StringCchCopyN(wchBuf,COUNTOF(wchBuf),CharNext(pwsz),COUNTOF(wchBuf));
        pwsz = StrChr(wchBuf,L';');
        if (pwsz)
          *pwsz = 0;
      }
      else
        StringCchCopyN(wchBuf,COUNTOF(wchBuf),pEE[i].wch,COUNTOF(wchBuf));

      if (Encoding_IsANSI(id))
        StringCchCatN(wchBuf,COUNTOF(wchBuf),wchANSI,COUNTOF(wchANSI));
      else if (Encoding_IsOEM(id))
        StringCchCatN(wchBuf,COUNTOF(wchBuf),wchOEM,COUNTOF(wchOEM));

      if (Encoding_IsValid(id))
        lvi.iImage = 0;
      else
        lvi.iImage = 1;

      lvi.lParam = (LPARAM)id;
      ListView_InsertItem(hwnd,&lvi);

      if (idSel == id)
        iSelItem = lvi.iItem;
    }
  }

  LocalFree(pEE);

  if (iSelItem != -1) {
    ListView_SetItemState(hwnd,iSelItem,LVIS_SELECTED | LVIS_FOCUSED,LVIS_SELECTED | LVIS_FOCUSED);
    ListView_EnsureVisible(hwnd,iSelItem,FALSE);
  }
  else {
    ListView_SetItemState(hwnd,0,LVIS_FOCUSED,LVIS_FOCUSED);
    ListView_EnsureVisible(hwnd,0,FALSE);
  }
}


BOOL Encoding_GetFromListView(HWND hwnd,int *pidEncoding) {
  LVITEM lvi;

  lvi.iItem = ListView_GetNextItem(hwnd,-1,LVNI_ALL | LVNI_SELECTED);
  lvi.iSubItem = 0;
  lvi.mask = LVIF_PARAM;

  if (ListView_GetItem(hwnd,&lvi)) {
    if (Encoding_IsValid((int)lvi.lParam))
      *pidEncoding = (int)lvi.lParam;
    else
      *pidEncoding = -1;

    return (TRUE);
  }
  return(FALSE);
}


void Encoding_AddToComboboxEx(HWND hwnd,int idSel,BOOL bRecodeOnly) {
  int i;
  int iSelItem = -1;
  COMBOBOXEXITEM cbei;
  WCHAR wchBuf[256] = { L'\0' };

  PENCODINGENTRY pEE = LocalAlloc(LPTR,COUNTOF(mEncoding) * sizeof(ENCODINGENTRY));
  for (i = 0; i < COUNTOF(mEncoding); i++) {
    pEE[i].id = i;
    GetString(mEncoding[i].idsName,pEE[i].wch,COUNTOF(pEE[i].wch));
  }
  qsort(pEE,COUNTOF(mEncoding),sizeof(ENCODINGENTRY),CmpEncoding);

  ZeroMemory(&cbei,sizeof(COMBOBOXEXITEM));
  cbei.mask = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_LPARAM;
  cbei.pszText = wchBuf;
  cbei.cchTextMax = COUNTOF(wchBuf);
  cbei.iImage = 0;
  cbei.iSelectedImage = 0;

  for (i = 0; i < COUNTOF(mEncoding); i++) {

    int id = pEE[i].id;
    if (!bRecodeOnly || (mEncoding[id].uFlags & NCP_RECODE)) {

      CPINFO cpi;

      cbei.iItem = SendMessage(hwnd,CB_GETCOUNT,0,0);

      WCHAR *pwsz = StrChr(pEE[i].wch,L';');
      if (pwsz) {
        StringCchCopyN(wchBuf,COUNTOF(wchBuf),CharNext(pwsz),COUNTOF(wchBuf));
        pwsz = StrChr(wchBuf,L';');
        if (pwsz)
          *pwsz = 0;
      }
      else
        StringCchCopyN(wchBuf,COUNTOF(wchBuf),pEE[i].wch,COUNTOF(wchBuf));

      if (Encoding_IsANSI(id))
        StringCchCatN(wchBuf,COUNTOF(wchBuf),wchANSI,COUNTOF(wchANSI));
      else if (id == CPI_OEM)
        StringCchCatN(wchBuf,COUNTOF(wchBuf),wchOEM,COUNTOF(wchOEM));

      if ((mEncoding[id].uFlags & NCP_INTERNAL) ||
        (IsValidCodePage(mEncoding[id].uCodePage) &&
          GetCPInfo(mEncoding[id].uCodePage,&cpi)))
        cbei.iImage = 0;
      else
        cbei.iImage = 1;

      cbei.lParam = (LPARAM)id;
      SendMessage(hwnd,CBEM_INSERTITEM,0,(LPARAM)&cbei);

      if (idSel == id)
        iSelItem = (int)cbei.iItem;
    }
  }

  LocalFree(pEE);

  if (iSelItem != -1)
    SendMessage(hwnd,CB_SETCURSEL,(WPARAM)iSelItem,0);
}


BOOL Encoding_GetFromComboboxEx(HWND hwnd,int *pidEncoding) {
  COMBOBOXEXITEM cbei;

  cbei.iItem = SendMessage(hwnd,CB_GETCURSEL,0,0);
  cbei.mask = CBEIF_LPARAM;

  if (SendMessage(hwnd,CBEM_GETITEM,0,(LPARAM)&cbei)) {
    if (Encoding_IsValid((int)cbei.lParam))
      *pidEncoding = (int)cbei.lParam;
    else
      *pidEncoding = -1;

    return (TRUE);
  }
  return(FALSE);
}


BOOL Encoding_IsDefault(int iEncoding) {
  return (mEncoding[iEncoding].uFlags & NCP_DEFAULT);
}

BOOL Encoding_IsANSI(int iEncoding) {
  return (mEncoding[iEncoding].uFlags & NCP_ANSI);
}

BOOL Encoding_IsOEM(int iEncoding) {
  return (mEncoding[iEncoding].uFlags & NCP_OEM);
}

UINT Encoding_SciGetCodePage(HWND hwnd) {
  // remove internal support for Chinese, Japan, Korean DBCS  use UTF-8 instead
  /*
  int cp = (UINT)SendMessage(hwnd,SCI_GETCODEPAGE,0,0);
  if (cp == 932 || cp == 936 || cp == 949 || cp == 950) {
  return cp;
  }
  return (cp == 0) ? CP_ACP : CP_UTF8;
  */
  UNUSED(hwnd);
  return CP_UTF8;
}


int Encoding_SciMappedCodePage(int iEncoding) {
  // remove internal support for Chinese, Japan, Korean DBCS  use UTF-8 instead
  /*
  if (Encoding_IsValid(iEncoding)) {
  // check for Chinese, Japan, Korean DBCS code pages and switch accordingly
  int cp = (int)mEncoding[iEncoding].uCodePage;
  if (cp == 932 || cp == 936 || cp == 949 || cp == 950) {
  return cp;
  }
  }
  */
  UNUSED(iEncoding);
  return SC_CP_UTF8;
}


void Encoding_SciSetCodePage(HWND hwnd,int iEncoding) {
  int cp = Encoding_SciMappedCodePage(iEncoding);
  SendMessage(hwnd,SCI_SETCODEPAGE,(WPARAM)cp,0);
  // charsets can be changed via styles schema
  /*
  int charset = SC_CHARSET_ANSI;
  switch (cp) {
  case 932:
  charset = SC_CHARSET_SHIFTJIS;
  break;
  case 936:
  charset = SC_CHARSET_GB2312;
  break;
  case 949:
  charset = SC_CHARSET_HANGUL;
  break;
  case 950:
  charset = SC_CHARSET_CHINESEBIG5;
  break;
  default:
  charset = iDefaultCharSet;
  break;
  }
  SendMessage(hwnd,SCI_STYLESETCHARACTERSET,(WPARAM)STYLE_DEFAULT,(LPARAM)charset);
  */
}


BOOL IsUnicode(const char* pBuffer,int cb,LPBOOL lpbBOM,LPBOOL lpbReverse) {
  int i = 0xFFFF;

  BOOL bIsTextUnicode;

  BOOL bHasBOM;
  BOOL bHasRBOM;

  if (!pBuffer || cb < 2)
    return FALSE;

  if (!bSkipUnicodeDetection)
    bIsTextUnicode = IsTextUnicode(pBuffer,cb,&i);
  else
    bIsTextUnicode = FALSE;

  bHasBOM = (*((UNALIGNED PWCHAR)pBuffer) == 0xFEFF);
  bHasRBOM = (*((UNALIGNED PWCHAR)pBuffer) == 0xFFFE);

  if (i == 0xFFFF) // i doesn't seem to have been modified ...
    i = 0;

  if (bIsTextUnicode || bHasBOM || bHasRBOM ||
    ((i & (IS_TEXT_UNICODE_UNICODE_MASK | IS_TEXT_UNICODE_REVERSE_MASK)) &&
      !((i & IS_TEXT_UNICODE_UNICODE_MASK) && (i & IS_TEXT_UNICODE_REVERSE_MASK)) &&
      !(i & IS_TEXT_UNICODE_ODD_LENGTH) &&
      !(i & IS_TEXT_UNICODE_ILLEGAL_CHARS && !(i & IS_TEXT_UNICODE_REVERSE_SIGNATURE)) &&
      !((i & IS_TEXT_UNICODE_REVERSE_MASK) == IS_TEXT_UNICODE_REVERSE_STATISTICS))) {

    if (lpbBOM)
      *lpbBOM = (bHasBOM || bHasRBOM ||
      (i & (IS_TEXT_UNICODE_SIGNATURE | IS_TEXT_UNICODE_REVERSE_SIGNATURE)))
      ? TRUE : FALSE;

    if (lpbReverse)
      *lpbReverse = (bHasRBOM || (i & IS_TEXT_UNICODE_REVERSE_MASK)) ? TRUE : FALSE;

    return TRUE;
  }

  else

    return FALSE;
}


BOOL IsUTF8(const char* pTest,int nLength) {
  static int byte_class_table[256] = {
    /*       00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  */
    /* 00 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 10 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 20 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 30 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 40 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 50 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 60 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 70 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 80 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 90 */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    /* A0 */ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    /* B0 */ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    /* C0 */ 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    /* D0 */ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    /* E0 */ 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 7, 7,
    /* F0 */ 9,10,10,10,11, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4
    /*       00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  */ };

  /* state table */
  typedef enum {
    kSTART = 0,kA,kB,kC,kD,kE,kF,kG,kERROR,kNumOfStates
  } utf8_state;

  static utf8_state state_table[] = {
    /*                            kSTART, kA,     kB,     kC,     kD,     kE,     kF,     kG,     kERROR */
    /* 0x00-0x7F: 0            */ kSTART, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0x80-0x8F: 1            */ kERROR, kSTART, kA,     kERROR, kA,     kB,     kERROR, kB,     kERROR,
    /* 0x90-0x9f: 2            */ kERROR, kSTART, kA,     kERROR, kA,     kB,     kB,     kERROR, kERROR,
    /* 0xa0-0xbf: 3            */ kERROR, kSTART, kA,     kA,     kERROR, kB,     kB,     kERROR, kERROR,
    /* 0xc0-0xc1, 0xf5-0xff: 4 */ kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0xc2-0xdf: 5            */ kA,     kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0xe0: 6                 */ kC,     kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0xe1-0xec, 0xee-0xef: 7 */ kB,     kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0xed: 8                 */ kD,     kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0xf0: 9                 */ kF,     kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0xf1-0xf3: 10           */ kE,     kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0xf4: 11                */ kG,     kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR };

#define BYTE_CLASS(b) (byte_class_table[(unsigned char)b])
#define NEXT_STATE(b,cur) (state_table[(BYTE_CLASS(b) * kNumOfStates) + (cur)])

  utf8_state current = kSTART;
  int i;

  const char* pt = pTest;
  int len = nLength;

  for (i = 0; i < len; i++,pt++) {

    current = NEXT_STATE(*pt,current);
    if (kERROR == current)
      break;
  }

  return (current == kSTART) ? TRUE : FALSE;
}


BOOL IsUTF7(const char* pTest,int nLength) {
  int i;
  const char *pt = pTest;

  for (i = 0; i < nLength; i++) {
    if (*pt & 0x80 || !*pt)
      return FALSE;
    pt++;
  }

  return TRUE;
}


/* byte length of UTF-8 sequence based on value of first byte.
for UTF-16 (21-bit space), max. code length is 4, so we only need to look
at 4 upper bits.
*/
static const INT utf8_lengths[16] =
{
  1,1,1,1,1,1,1,1,        /* 0000 to 0111 : 1 byte (plain ASCII) */
  0,0,0,0,                /* 1000 to 1011 : not valid */
  2,2,                    /* 1100, 1101 : 2 bytes */
  3,                      /* 1110 : 3 bytes */
  4                       /* 1111 :4 bytes */
};

/*++
Function :
UTF8_mbslen_bytes [INTERNAL]

Calculates the byte size of a NULL-terminated UTF-8 string.

Parameters :
char *utf8_string : string to examine

Return value :
size (in bytes) of a NULL-terminated UTF-8 string.
-1 if invalid NULL-terminated UTF-8 string
--*/
INT UTF8_mbslen_bytes(LPCSTR utf8_string) 
{
  INT length = 0;
  INT code_size;
  BYTE byte;

  while (*utf8_string) {
    byte = (BYTE)*utf8_string;

    if ((byte <= 0xF7) && (0 != (code_size = utf8_lengths[byte >> 4]))) {
      length += code_size;
      utf8_string += code_size;
    }
    else {
      /* we got an invalid byte value but need to count it,
      it will be later ignored during the string conversion */
      //WARN("invalid first byte value 0x%02X in UTF-8 sequence!\n",byte);
      length++;
      utf8_string++;
    }
  }
  length++; /* include NULL terminator */
  return length;
}

/*++
Function :
UTF8_mbslen [INTERNAL]

Calculates the character size of a NULL-terminated UTF-8 string.

Parameters :
char *utf8_string : string to examine
int byte_length : byte size of string

Return value :
size (in characters) of a UTF-8 string.
-1 if invalid UTF-8 string
--*/
INT UTF8_mbslen(LPCSTR source,INT byte_length) 
{
  INT wchar_length = 0;
  INT code_size;
  BYTE byte;

  while (byte_length > 0) {
    byte = (BYTE)*source;

    /* UTF-16 can't encode 5-byte and 6-byte sequences, so maximum value
    for first byte is 11110111. Use lookup table to determine sequence
    length based on upper 4 bits of first byte */
    if ((byte <= 0xF7) && (0 != (code_size = utf8_lengths[byte >> 4]))) {
      /* 1 sequence == 1 character */
      wchar_length++;

      if (code_size == 4)
        wchar_length++;

      source += code_size;        /* increment pointer */
      byte_length -= code_size;   /* decrement counter*/
    }
    else {
      /*
      unlike UTF8_mbslen_bytes, we ignore the invalid characters.
      we only report the number of valid characters we have encountered
      to match the Windows behavior.
      */
      //WARN("invalid byte 0x%02X in UTF-8 sequence, skipping it!\n",
      //     byte);
      source++;
      byte_length--;
    }
  }
  return wchar_length;
}


///   End of Helpers.c   \\\
