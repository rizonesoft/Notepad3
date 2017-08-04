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

#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x501
#endif
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <stdio.h>
#include <string.h>
#include "helpers.h"
#include "resource.h"
#include "Edit.h"


//=============================================================================
extern iInternalCodePage;


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
  WCHAR tch[256];
  int  ich;

  if (p) {
    lstrcpy(tch,lpName);
    lstrcat(tch,L"=");
    ich = lstrlen(tch);

    while (*p) {
      if (StrCmpNI(p,tch,ich) == 0) {
        lstrcpyn(lpReturnedString,p + ich,cchReturnedString);
        return(lstrlen(lpReturnedString));
      }
      else
        p = StrEnd(p) + 1;
    }
  }
  lstrcpyn(lpReturnedString,lpDefault,cchReturnedString);
  return(lstrlen(lpReturnedString));
}


int IniSectionGetInt(
      LPCWSTR lpCachedIniSection,
      LPCWSTR lpName,
      int iDefault)
{
  WCHAR *p = (WCHAR *)lpCachedIniSection;
  WCHAR tch[256];
  int  ich;
  int  i;

  if (p) {
    lstrcpy(tch,lpName);
    lstrcat(tch,L"=");
    ich = lstrlen(tch);

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
    WCHAR tch[256];
    int  ich;
    UINT u;

    if (p) {
        lstrcpy(tch, lpName);
        lstrcat(tch, L"=");
        ich = lstrlen(tch);

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
  WCHAR *p = lpCachedIniSection;

  if (p) {
    while (*p) {
      p = StrEnd(p) + 1;
    }
    wsprintf(tch,L"%s=%s",lpName,lpString);
    lstrcpy(p,tch);
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

  if (lstrcmpi(AppID,L"(default)") == 0)
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

    struct {
      DWORD TokenIsElevated;
    } /*TOKEN_ELEVATION*/te;
    DWORD dwReturnLength = 0;

    if (GetTokenInformation(hToken,/*TokenElevation*/20,&te,sizeof(te),&dwReturnLength)) {
        if (dwReturnLength == sizeof(te))
          bIsElevated = te.TokenIsElevated;
    }
    CloseHandle(hToken);
  }
  return bIsElevated;
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
//    if (hModUxTheme) {
//      pfnSetWindowTheme = GetProcAddress(hModUxTheme,"SetWindowTheme");
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

  WCHAR szUntitled[128];
  WCHAR szExcrptQuot[256];
  WCHAR szExcrptFmt[32];
  WCHAR szAppName[128];
  WCHAR szElevatedAppName[128];
  WCHAR szReadOnly[32];
  WCHAR szTitle[512];
  static WCHAR szCachedFile[MAX_PATH];
  static WCHAR szCachedDisplayName[MAX_PATH];
  static const WCHAR *pszSep = L" - ";
  static const WCHAR *pszMod = L"* ";

  if (bFreezeAppTitle)
    return FALSE;

  if (!GetString(uIDAppName,szAppName,COUNTOF(szAppName)) ||
      !GetString(uIDUntitled,szUntitled,COUNTOF(szUntitled)))
    return FALSE;

  if (bIsElevated) {
    FormatString(szElevatedAppName,COUNTOF(szElevatedAppName),IDS_APPTITLE_ELEVATED,szAppName);
    StrCpyN(szAppName,szElevatedAppName,COUNTOF(szAppName));
  }

  if (bModified)
    lstrcpy(szTitle,pszMod);
  else
    lstrcpy(szTitle,L"");

  if (lstrlen(lpszExcerpt)) {
    GetString(IDS_TITLEEXCERPT,szExcrptFmt,COUNTOF(szExcrptFmt));
    wsprintf(szExcrptQuot,szExcrptFmt,lpszExcerpt);
    StrCat(szTitle,szExcrptQuot);
  }

  else if (lstrlen(lpszFile))
  {
    if (iFormat < 2 && !PathIsRoot(lpszFile))
    {
      if (lstrcmp(szCachedFile,lpszFile) != 0) {
        SHFILEINFO shfi;
        lstrcpy(szCachedFile,lpszFile);
        if (SHGetFileInfo2(lpszFile,0,&shfi,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME))
          lstrcpy(szCachedDisplayName,shfi.szDisplayName);
        else
          lstrcpy(szCachedDisplayName,PathFindFileName(lpszFile));
      }
      lstrcat(szTitle,szCachedDisplayName);
      if (iFormat == 1) {
        WCHAR tchPath[MAX_PATH];
        StrCpyN(tchPath,lpszFile,COUNTOF(tchPath));
        PathRemoveFileSpec(tchPath);
        StrCat(szTitle,L" [");
        StrCat(szTitle,tchPath);
        StrCat(szTitle,L"]");
      }
    }
    else
      lstrcat(szTitle,lpszFile);
  }

  else {
    lstrcpy(szCachedFile,L"");
    lstrcpy(szCachedDisplayName,L"");
    lstrcat(szTitle,szUntitled);
  }

  if (bReadOnly && GetString(uIDReadOnly,szReadOnly,COUNTOF(szReadOnly)))
  {
    lstrcat(szTitle,L" ");
    lstrcat(szTitle,szReadOnly);
  }

  lstrcat(szTitle,pszSep);
  lstrcat(szTitle,szAppName);

  return SetWindowText(hwnd,szTitle);

}


//=============================================================================
//
//  SetWindowTransparentMode()
//
void SetWindowTransparentMode(HWND hwnd,BOOL bTransparentMode)
{
  FARPROC fp;
  int  iAlphaPercent;
  BYTE bAlpha;

  if (bTransparentMode) {
    if (fp = GetProcAddress(GetModuleHandle(L"User32"),"SetLayeredWindowAttributes")) {
      SetWindowLongPtr(hwnd,GWL_EXSTYLE,
        GetWindowLongPtr(hwnd,GWL_EXSTYLE) | WS_EX_LAYERED);

      // get opacity level from registry
      iAlphaPercent = IniGetInt(L"Settings2",L"OpacityLevel",75);
      if (iAlphaPercent < 0 || iAlphaPercent > 100)
        iAlphaPercent = 75;
      bAlpha = iAlphaPercent * 255 / 100;

      fp(hwnd,0,bAlpha,LWA_ALPHA);
    }
  }

  else
    SetWindowLongPtr(hwnd,GWL_EXSTYLE,
      GetWindowLongPtr(hwnd,GWL_EXSTYLE) & ~WS_EX_LAYERED);
}


//=============================================================================
//
//  SetInternalCodePage()
//
void SetInternalCodePage(HWND hwnd, int encoding)   {
  iInternalCodePage = (encoding == CPI_ANSI) ? 0 : SC_CP_UTF8;
  SendMessage(hwnd, SCI_SETCODEPAGE, iInternalCodePage, 0);
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
  WCHAR wch[64];
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

  WCHAR szText[256];
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
  WCHAR tchButtons[512];
  WCHAR tchItem[32];
  int i,c;
  TBBUTTON tbb;

  lstrcpy(tchButtons,L"");
  c = min(50,(int)SendMessage(hwnd,TB_BUTTONCOUNT,0,0));

  for (i = 0; i < c; i++) {
    SendMessage(hwnd,TB_GETBUTTON,(WPARAM)i,(LPARAM)&tbb);
    wsprintf(tchItem,L"%i ",
      (tbb.idCommand==0)?0:tbb.idCommand-cmdBase+1);
    lstrcat(tchButtons,tchItem);
  }
  TrimString(tchButtons);
  lstrcpyn(lpszButtons,tchButtons,cchButtons);
  return(c);
}

int Toolbar_SetButtons(HWND hwnd,int cmdBase,LPCWSTR lpszButtons,LPCTBBUTTON ptbb,int ctbb)
{
  WCHAR tchButtons[512];
  WCHAR *p;
  int i,c;
  int iCmd;

  ZeroMemory(tchButtons,COUNTOF(tchButtons)*sizeof(tchButtons[0]));
  lstrcpyn(tchButtons,lpszButtons,COUNTOF(tchButtons)-2);
  TrimString(tchButtons);
  while (p = StrStr(tchButtons,L"  "))
    MoveMemory((WCHAR*)p,(WCHAR*)p+1,(lstrlen(p) + 1) * sizeof(WCHAR));

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
    wvsprintf(lpOutput,p,(LPVOID)((PUINT_PTR)&uIdFormat+1));

  LocalFree(p);

  return lstrlen(lpOutput);

}


//=============================================================================
//
//  PathRelativeToApp()
//
void PathRelativeToApp(
  LPWSTR lpszSrc,LPWSTR lpszDest,int cchDest,BOOL bSrcIsFile,
  BOOL bUnexpandEnv,BOOL bUnexpandMyDocs) {

  WCHAR wchAppPath[MAX_PATH];
  WCHAR wchWinDir[MAX_PATH];
  WCHAR wchUserFiles[MAX_PATH];
  WCHAR wchPath[MAX_PATH];
  WCHAR wchResult[MAX_PATH];
  DWORD dwAttrTo = (bSrcIsFile) ? 0 : FILE_ATTRIBUTE_DIRECTORY;

  GetModuleFileName(NULL,wchAppPath,COUNTOF(wchAppPath));
  PathRemoveFileSpec(wchAppPath);
  GetWindowsDirectory(wchWinDir,COUNTOF(wchWinDir));
  SHGetFolderPath(NULL,CSIDL_PERSONAL,NULL,SHGFP_TYPE_CURRENT,wchUserFiles);

  if (bUnexpandMyDocs &&
      !PathIsRelative(lpszSrc) &&
      !PathIsPrefix(wchUserFiles,wchAppPath) &&
       PathIsPrefix(wchUserFiles,lpszSrc) &&
       PathRelativePathTo(wchPath,wchUserFiles,FILE_ATTRIBUTE_DIRECTORY,lpszSrc,dwAttrTo)) {
    lstrcpy(wchUserFiles,L"%CSIDL:MYDOCUMENTS%");
    PathAppend(wchUserFiles,wchPath);
    lstrcpy(wchPath,wchUserFiles);
  }
  else if (PathIsRelative(lpszSrc) || PathCommonPrefix(wchAppPath,wchWinDir,NULL))
    lstrcpyn(wchPath,lpszSrc,COUNTOF(wchPath));
  else {
    if (!PathRelativePathTo(wchPath,wchAppPath,FILE_ATTRIBUTE_DIRECTORY,lpszSrc,dwAttrTo))
      lstrcpyn(wchPath,lpszSrc,COUNTOF(wchPath));
  }

  if (bUnexpandEnv) {
    if (!PathUnExpandEnvStrings(wchPath,wchResult,COUNTOF(wchResult)))
      lstrcpyn(wchResult,wchPath,COUNTOF(wchResult));
  }
  else
    lstrcpyn(wchResult,wchPath,COUNTOF(wchResult));

  if (lpszDest == NULL || lpszSrc == lpszDest)
    lstrcpyn(lpszSrc,wchResult,(cchDest == 0) ? MAX_PATH : cchDest);
  else
    lstrcpyn(lpszDest,wchResult,(cchDest == 0) ? MAX_PATH : cchDest);
}


//=============================================================================
//
//  PathAbsoluteFromApp()
//
void PathAbsoluteFromApp(LPWSTR lpszSrc,LPWSTR lpszDest,int cchDest,BOOL bExpandEnv) {

  WCHAR wchPath[MAX_PATH];
  WCHAR wchResult[MAX_PATH];

  if (lpszSrc == NULL) {
    ZeroMemory(lpszDest, (cchDest == 0) ? MAX_PATH : cchDest);
    return;
  }

  if (StrCmpNI(lpszSrc,L"%CSIDL:MYDOCUMENTS%",CSTRLEN("%CSIDL:MYDOCUMENTS%")) == 0) {
    SHGetFolderPath(NULL,CSIDL_PERSONAL,NULL,SHGFP_TYPE_CURRENT,wchPath);
    PathAppend(wchPath,lpszSrc+CSTRLEN("%CSIDL:MYDOCUMENTS%"));
  }
  else
    lstrcpyn(wchPath,lpszSrc,COUNTOF(wchPath));

  if (bExpandEnv)
    ExpandEnvironmentStringsEx(wchPath,COUNTOF(wchPath));

  if (PathIsRelative(wchPath)) {
    GetModuleFileName(NULL,wchResult,COUNTOF(wchResult));
    PathRemoveFileSpec(wchResult);
    PathAppend(wchResult,wchPath);
  }
  else
    lstrcpyn(wchResult,wchPath,COUNTOF(wchResult));

  PathCanonicalizeEx(wchResult);
  if (PathGetDriveNumber(wchResult) != -1)
    CharUpperBuff(wchResult,1);

  if (lpszDest == NULL || lpszSrc == lpszDest)
    lstrcpyn(lpszSrc,wchResult,(cchDest == 0) ? MAX_PATH : cchDest);
  else
    lstrcpyn(lpszDest,wchResult,(cchDest == 0) ? MAX_PATH : cchDest);
}


///////////////////////////////////////////////////////////////////////////////
//
//
//  Name: PathIsLnkFile()
//
//  Purpose: Determine wheter pszPath is a Windows Shell Link File by
//           comparing the filename extension with L".lnk"
//
//  Manipulates:
//
BOOL PathIsLnkFile(LPCWSTR pszPath)
{

  //WCHAR *pszExt;

  WCHAR tchResPath[256];

  if (!pszPath || !*pszPath)
    return FALSE;

/*pszExt = StrRChr(pszPath,NULL,L'.');

  if (!pszExt)
    return FALSE;

  if (!lstrcmpi(pszExt,L".lnk"))
    return TRUE;

  else
    return FALSE;*/

  //if (!lstrcmpi(PathFindExtension(pszPath),L".lnk"))
  //  return TRUE;

  //else
  //  return FALSE;

  if (lstrcmpi(PathFindExtension(pszPath),L".lnk"))
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
      WORD wsz[MAX_PATH];

      /*MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,
                          pszLnkFile,-1,wsz,MAX_PATH);*/
      lstrcpy(wsz,pszLnkFile);

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
  if (!lstrlen(pszResPath))
    bSucceeded = FALSE;

  if (bSucceeded) {
    ExpandEnvironmentStringsEx(pszResPath,cchResPath);
    PathCanonicalizeEx(pszResPath);
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

  WCHAR tchResPath[MAX_PATH];

  if (PathIsLnkFile(pszPath)) {

    if (PathGetLnkPath(pszPath,tchResPath,sizeof(WCHAR)*COUNTOF(tchResPath))) {

      if (PathIsDirectory(tchResPath)) {

        lstrcpyn(pszResPath,tchResPath,cchResPath);
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

  WCHAR tchExeFile[MAX_PATH];
  WCHAR tchDocTemp[MAX_PATH];
  WCHAR tchArguments[MAX_PATH+16];
  WCHAR tchLinkDir[MAX_PATH];
  WCHAR tchDescription[64];

  WCHAR tchLnkFileName[MAX_PATH];

  IShellLink *psl;
  BOOL bSucceeded = FALSE;
  BOOL fMustCopy;

  if (!pszDocument || lstrlen(pszDocument) == 0)
    return TRUE;

  // init strings
  GetModuleFileName(NULL,tchExeFile,COUNTOF(tchExeFile));

  lstrcpy(tchDocTemp,pszDocument);
  PathQuoteSpaces(tchDocTemp);

  lstrcpy(tchArguments,L"-n ");
  lstrcat(tchArguments,tchDocTemp);

  SHGetSpecialFolderPath(NULL,tchLinkDir,CSIDL_DESKTOPDIRECTORY,TRUE);

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
      WORD wsz[MAX_PATH];

      /*MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,
                          tchLnkFileName,-1,wsz,MAX_PATH);*/
      lstrcpy(wsz,tchLnkFileName);

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

  WCHAR tchLnkFileName[MAX_PATH];

  IShellLink *psl;
  BOOL bSucceeded = FALSE;

  if (!pszName || lstrlen(pszName) == 0)
    return TRUE;

  lstrcpy(tchLnkFileName,pszDir);
  PathAppend(tchLnkFileName,pszName);
  lstrcat(tchLnkFileName,L".lnk");

  if (PathFileExists(tchLnkFileName))
    return FALSE;

  if (SUCCEEDED(CoCreateInstance(&CLSID_ShellLink,NULL,
                                 CLSCTX_INPROC_SERVER,
                                 &IID_IShellLink,&psl)))
  {
    IPersistFile *ppf;

    if (SUCCEEDED(psl->lpVtbl->QueryInterface(psl,&IID_IPersistFile,&ppf)))
    {
      WORD wsz[MAX_PATH];

      /*MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,
                          tchLnkFileName,-1,wsz,MAX_PATH);*/
      lstrcpy(wsz,tchLnkFileName);

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
BOOL ExtractFirstArgument(LPCWSTR lpArgs,LPWSTR lpArg1,LPWSTR lpArg2)
{

  LPWSTR psz;
  BOOL bQuoted = FALSE;

  lstrcpy(lpArg1,lpArgs);
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
    psz = StrChr(lpArg1,L' ');;

  if (psz)
  {
    *psz = L'\0';
    if (lpArg2)
      lstrcpy(lpArg2,psz + 1);
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
  WCHAR *c = lpsz;
  while (c = StrChr(lpsz,L'\t'))
    *c = L' ';
}


//=============================================================================
//
//  PathFixBackslashes() - in place conversion
//
void PathFixBackslashes(LPWSTR lpsz)
{
  WCHAR *c = lpsz;
  while (c = StrChr(c,L'/')) {
    if (*CharPrev(lpsz,c) == L':' && *CharNext(c) == L'/')
      c += 2;
    else
      *c = L'\\';
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
  WCHAR szBuf[312];

  if (ExpandEnvironmentStrings(lpSrc,szBuf,COUNTOF(szBuf)))
    lstrcpyn(lpSrc,szBuf,dwSrc);
}


//=============================================================================
//
//  PathCanonicalizeEx()
//
//
void PathCanonicalizeEx(LPWSTR lpSrc)
{
  WCHAR szDst[MAX_PATH];

  if (PathCanonicalize(szDst,lpSrc))
    lstrcpy(lpSrc,szDst);
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
    if (lstrlen(psfi->szDisplayName) < lstrlen(PathFindFileName(pszPath)))
      StrCatBuff(psfi->szDisplayName,PathFindExtension(pszPath),COUNTOF(psfi->szDisplayName));
    return(dw);
  }

  else {
    DWORD_PTR dw = SHGetFileInfo(pszPath,FILE_ATTRIBUTE_NORMAL,psfi,cbFileInfo,uFlags|SHGFI_USEFILEATTRIBUTES);
    if (lstrlen(psfi->szDisplayName) < lstrlen(PathFindFileName(pszPath)))
      StrCatBuff(psfi->szDisplayName,PathFindExtension(pszPath),COUNTOF(psfi->szDisplayName));
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
  WCHAR szBuf[64];

  wsprintf(szBuf,L"%u",uValue);
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
  WCharToMBCS(uCP,wsz,lpString,nMaxCount-2);
  return uRet;
}

UINT SetDlgItemTextA2W(UINT uCP,HWND hDlg,int nIDDlgItem,LPSTR lpString)
{
  WCHAR wsz[1024] = L"";
  MBCSToWChar(uCP,lpString,wsz,COUNTOF(wsz));
  return SetDlgItemTextW(hDlg,nIDDlgItem,wsz);
}

LRESULT ComboBox_AddStringA2W(UINT uCP,HWND hwnd,LPCSTR lpString)
{
  WCHAR wsz[1024] = L"";
  MBCSToWChar(uCP,lpString,wsz,COUNTOF(wsz));
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
//  MRU functions
//
LPMRULIST MRU_Create(LPCWSTR pszRegKey,int iFlags,int iSize) {

  LPMRULIST pmru = LocalAlloc(LPTR,sizeof(MRULIST));
  ZeroMemory(pmru,sizeof(MRULIST));
  lstrcpyn(pmru->szRegKey,pszRegKey,COUNTOF(pmru->szRegKey));
  pmru->iFlags = iFlags;
  pmru->iSize = min(iSize,MRU_MAXITEMS);
  return(pmru);
}

BOOL MRU_Destroy(LPMRULIST pmru) {

  int i;
  for (i = 0; i < pmru->iSize; i++) {
    if (pmru->pszItems[i])
      LocalFree(pmru->pszItems[i]);
    }
  ZeroMemory(pmru,sizeof(MRULIST));
  LocalFree(pmru);
  return(1);
}

int MRU_Compare(LPMRULIST pmru,LPCWSTR psz1,LPCWSTR psz2) {

  if (pmru->iFlags & MRU_NOCASE)
    return(lstrcmpi(psz1,psz2));
  else
    return(lstrcmp(psz1,psz2));
}

BOOL MRU_Add(LPMRULIST pmru,LPCWSTR pszNew) {

  int i;
  for (i = 0; i < pmru->iSize; i++) {
    if (MRU_Compare(pmru,pmru->pszItems[i],pszNew) == 0) {
      LocalFree(pmru->pszItems[i]);
      break;
    }
  }
  i = min(i,pmru->iSize-1);
  for (; i > 0; i--)
    pmru->pszItems[i] = pmru->pszItems[i-1];
  pmru->pszItems[0] = StrDup(pszNew);
  return(1);
}

BOOL MRU_AddFile(LPMRULIST pmru,LPCWSTR pszFile,BOOL bRelativePath,BOOL bUnexpandMyDocs) {

  int i;
  for (i = 0; i < pmru->iSize; i++) {
    if (lstrcmpi(pmru->pszItems[i],pszFile) == 0) {
      LocalFree(pmru->pszItems[i]);
      break;
    }
    else if (pmru->pszItems[i] == NULL) {
      break;
    }
    else {
      WCHAR wchItem[MAX_PATH];
      PathAbsoluteFromApp(pmru->pszItems[i],wchItem,COUNTOF(wchItem),TRUE);
      if (lstrcmpi(wchItem,pszFile) == 0) {
        LocalFree(pmru->pszItems[i]);
        break;
      }
    }
  }
  i = min(i,pmru->iSize-1);
  for (; i > 0; i--)
    pmru->pszItems[i] = pmru->pszItems[i-1];

  if (bRelativePath) {
    WCHAR wchFile[MAX_PATH];
    PathRelativeToApp((LPWSTR)pszFile,wchFile,COUNTOF(wchFile),TRUE,TRUE,bUnexpandMyDocs);
    pmru->pszItems[0] = StrDup(wchFile);
  }
  else
    pmru->pszItems[0] = StrDup(pszFile);

/* notepad2-mod custom code start */
  // Needed to make W7 jump lists work when NP2 is not explicitly associated
  if (IsW7()) SHAddToRecentDocs(SHARD_PATHW, pszFile);
/* notepad2-mod custom code end */

  return(1);
}

BOOL MRU_Delete(LPMRULIST pmru,int iIndex) {

  int i;
  if (iIndex < 0 || iIndex > pmru->iSize-1)
    return(0);
  if (pmru->pszItems[iIndex])
    LocalFree(pmru->pszItems[iIndex]);
  for (i = iIndex; i < pmru->iSize-1; i++) {
    pmru->pszItems[i] = pmru->pszItems[i+1];
    pmru->pszItems[i+1] = NULL;
  }
  return(1);
}

BOOL MRU_DeleteFileFromStore(LPMRULIST pmru,LPCWSTR pszFile) {

  int i = 0;
  LPMRULIST pmruStore;
  WCHAR wchItem[256];

  pmruStore = MRU_Create(pmru->szRegKey,pmru->iFlags,pmru->iSize);
  MRU_Load(pmruStore);

  while (MRU_Enum(pmruStore,i,wchItem,COUNTOF(wchItem)) != -1) {
    PathAbsoluteFromApp(wchItem,wchItem,COUNTOF(wchItem),TRUE);
    if (lstrcmpi(wchItem,pszFile) == 0)
      MRU_Delete(pmruStore,i);
    else
      i++;
  }

  MRU_Save(pmruStore);
  MRU_Destroy(pmruStore);
  return(1);
}

BOOL MRU_Empty(LPMRULIST pmru) {

  int i;
  for (i = 0; i < pmru->iSize; i++) {
    if (pmru->pszItems[i]) {
      LocalFree(pmru->pszItems[i]);
      pmru->pszItems[i] = NULL;
    }
  }
  return(1);
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
      lstrcpyn(pszItem,pmru->pszItems[iIndex],cchItem);
      return(lstrlen(pszItem));
    }
  }
}

BOOL MRU_Load(LPMRULIST pmru) {

  int i,n = 0;
  WCHAR tchName[32];
  WCHAR tchItem[1024];
  WCHAR *pIniSection = LocalAlloc(LPTR,sizeof(WCHAR)*32*1024);

  MRU_Empty(pmru);
  LoadIniSection(pmru->szRegKey,pIniSection,(int)LocalSize(pIniSection)/sizeof(WCHAR));

  for (i = 0; i < pmru->iSize; i++) {
    wsprintf(tchName,L"%.2i",i+1);
    if (IniSectionGetString(pIniSection,tchName,L"",tchItem,COUNTOF(tchItem))) {
      /*if (pmru->iFlags & MRU_UTF8) {
        WCHAR wchItem[1024];
        int cbw = MultiByteToWideChar(CP_UTF7,0,tchItem,-1,wchItem,COUNTOF(wchItem));
        WideCharToMultiByte(CP_UTF8,0,wchItem,cbw,tchItem,COUNTOF(tchItem),NULL,NULL);
        pmru->pszItems[n++] = StrDup(tchItem);
      }
      else*/
        pmru->pszItems[n++] = StrDup(tchItem);
    }
  }
  LocalFree(pIniSection);
  return(1);
}

BOOL MRU_Save(LPMRULIST pmru) {

  int i;
  WCHAR tchName[32];
  WCHAR *pIniSection = LocalAlloc(LPTR,sizeof(WCHAR)*32*1024);

  //IniDeleteSection(pmru->szRegKey);

  for (i = 0; i < pmru->iSize; i++) {
    if (pmru->pszItems[i]) {
      wsprintf(tchName,L"%.2i",i+1);
      /*if (pmru->iFlags & MRU_UTF8) {
        WCHAR  tchItem[1024];
        WCHAR wchItem[1024];
        int cbw = MultiByteToWideChar(CP_UTF8,0,pmru->pszItems[i],-1,wchItem,COUNTOF(wchItem));
        WideCharToMultiByte(CP_UTF7,0,wchItem,cbw,tchItem,COUNTOF(tchItem),NULL,NULL);
        IniSectionSetString(pIniSection,tchName,tchItem);
      }
      else*/
        IniSectionSetString(pIniSection,tchName,pmru->pszItems[i]);
    }
  }
  SaveIniSection(pmru->szRegKey,pIniSection);
  LocalFree(pIniSection);
  return(1);
}


BOOL MRU_MergeSave(LPMRULIST pmru,BOOL bAddFiles,BOOL bRelativePath,BOOL bUnexpandMyDocs) {

  int i;
  LPMRULIST pmruBase;

  pmruBase = MRU_Create(pmru->szRegKey,pmru->iFlags,pmru->iSize);
  MRU_Load(pmruBase);

  if (bAddFiles) {
    for (i = pmru->iSize-1; i >= 0; i--) {
      if (pmru->pszItems[i]) {
        WCHAR wchItem[MAX_PATH];
        PathAbsoluteFromApp(pmru->pszItems[i],wchItem,COUNTOF(wchItem),TRUE);
        MRU_AddFile(pmruBase,wchItem,bRelativePath,bUnexpandMyDocs);
      }
    }
  }

  else {
    for (i = pmru->iSize-1; i >= 0; i--) {
      if (pmru->pszItems[i])
        MRU_Add(pmruBase,pmru->pszItems[i]);
    }
  }

  MRU_Save(pmruBase);
  MRU_Destroy(pmruBase);
  return(1);
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
  HMODULE hModUxTheme;
  HTHEME hTheme;
  LOGFONT lf;
  BOOL bSucceed = FALSE;

  hDC = GetDC(NULL);
  iLogPixelsY = GetDeviceCaps(hDC,LOGPIXELSY);
  ReleaseDC(NULL,hDC);

  if (hModUxTheme = GetModuleHandle(L"uxtheme.dll")) {
    if ((BOOL)(GetProcAddress(hModUxTheme,"IsAppThemed"))()) {
      hTheme = (HTHEME)(INT_PTR)(GetProcAddress(hModUxTheme,"OpenThemeData"))(NULL,L"WINDOWSTYLE;WINDOW");
      if (hTheme) {
        if (S_OK == (HRESULT)(GetProcAddress(hModUxTheme,"GetThemeSysFont"))(hTheme,/*TMT_MSGBOXFONT*/805,&lf)) {
          if (lf.lfHeight < 0)
            lf.lfHeight = -lf.lfHeight;
          *wSize = (WORD)MulDiv(lf.lfHeight,72,iLogPixelsY);
          if (*wSize == 0)
            *wSize = 8;
          StrCpyN(lpFaceName,lf.lfFaceName,LF_FACESIZE);
          bSucceed = TRUE;
        }
        (GetProcAddress(hModUxTheme,"CloseThemeData"))(hTheme);
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
    StrCpyN(lpFaceName,ncm.lfMessageFont.lfFaceName,LF_FACESIZE);
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

  cbNew = cbFontAttr + ((lstrlen(wchFaceName) + 1) * sizeof(WCHAR));
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
          val[0] = hex;
          hex = GetHexDigit(*(s+1));
          if (hex >= 0) {
            s++;
            val[0] *= 16;
            val[0] += hex;
            if (!bShort) {
              hex = GetHexDigit(*(s+1));
              if (hex >= 0) {
                s++;
                val[0] *= 16;
                val[0] += hex;
                hex = GetHexDigit(*(s+1));
                if (hex >= 0) {
                  s++;
                  val[0] *= 16;
                  val[0] += hex;
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



///   End of Helpers.c   \\\
