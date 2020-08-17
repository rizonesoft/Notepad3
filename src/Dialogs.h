// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Dialogs.h                                                                   *
*   Definitions for Notepad3 dialog boxes                                     *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2020   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#pragma once
#ifndef _NP3_DIALOGS_H_
#define _NP3_DIALOGS_H_

#include <math.h>
#include <uxtheme.h>
#include "TypeDefs.h"
#include "Scintilla.h"

// ----------------------------------------------------------------------------

#define DIALOG_FONT_SIZE_INCR 0  // will increase default dialog font size

#define SetExplorerTheme(hwnd) SetWindowTheme((hwnd), L"Explorer", NULL)

// ----------------------------------------------------------------------------

INT_PTR DisplayCmdLineHelp(HWND hwnd);
bool GetDirectory(HWND hwndParent,int uiTitle,LPWSTR pszFolder,LPCWSTR pszBase,bool);
INT_PTR CALLBACK AboutDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam);
INT_PTR RunDlg(HWND hwnd,LPCWSTR lpstrDefault);
bool OpenWithDlg(HWND hwnd,LPCWSTR lpstrFile);
bool FavoritesDlg(HWND hwnd,LPWSTR lpstrFile);
bool AddToFavDlg(HWND hwnd,LPCWSTR lpszName,LPCWSTR lpszTarget);
bool FileMRUDlg(HWND hwnd,LPWSTR lpstrFile);
bool ChangeNotifyDlg(HWND hwnd);
bool ColumnWrapDlg(HWND hwnd,UINT uidDlg,UINT * iNumber);
bool WordWrapSettingsDlg(HWND hwnd,UINT uidDlg,int * iNumber);
bool LongLineSettingsDlg(HWND hwnd,UINT uidDlg, LPWSTR pColList);
bool TabSettingsDlg(HWND hwnd,UINT uidDlg,int * iNumber);
bool SelectDefEncodingDlg(HWND hwnd, cpi_enc_t* pidREncoding);
bool SelectEncodingDlg(HWND hwnd, cpi_enc_t* pidREncoding);
bool RecodeDlg(HWND hwnd, cpi_enc_t* pidREncoding);
bool SelectDefLineEndingDlg(HWND hwnd,LPARAM piOption);
bool WarnLineEndingDlg(HWND hwnd, EditFileIOStatus* fioStatus);
bool WarnIndentationDlg(HWND hwnd, EditFileIOStatus* fioStatus);

bool GetMonitorInfoFromRect(const RECT* rc, MONITORINFO* hMonitorInfo);
void WinInfoToScreen(WININFO* pWinInfo);
WININFO GetMyWindowPlacement(HWND hwnd,MONITORINFO * hMonitorInfo);
void FitIntoMonitorGeometry(RECT* pRect, WININFO* pWinInfo, SCREEN_MODE mode);
WINDOWPLACEMENT WindowPlacementFromInfo(HWND hwnd, const WININFO* pWinInfo, SCREEN_MODE mode);

void DialogNewWindow(HWND hwnd, bool bSaveOnRunTools, LPCWSTR lpcwFilePath);
void DialogFileBrowse(HWND hwnd);
void DialogGrepWin(HWND hwnd, LPCWSTR searchPattern);
void DialogAdminExe(HWND hwnd,bool);

int  MessageBoxLng(UINT uType, UINT uidMsg, ...);
INT_PTR InfoBoxLng(UINT uType, LPCWSTR lpstrSetting, UINT uidMsg, ...);
DWORD MsgBoxLastError(LPCWSTR lpszMessage, DWORD dwErrID);
DWORD DbgMsgBoxLastError(LPCWSTR lpszMessage, DWORD dwErrID);

bool SetWindowTitle(HWND hwnd, UINT uIDAppName, bool bIsElevated, UINT uIDUntitled,
                    LPCWSTR lpszFile, int iFormat, bool bModified,
                    UINT uIDReadOnly, bool bReadOnly, LPCWSTR lpszExcerpt);
void SetAdditionalTitleInfo(LPCWSTR lpszAddTitleInfo);
void AppendAdditionalTitleInfo(LPCWSTR lpszAddTitleInfo);
void SetWindowTransparentMode(HWND hwnd, bool bTransparentMode, int iOpacityLevel);
void SetWindowLayoutRTL(HWND hwnd, bool bRTL);
void SetWindowReadingRTL(HWND hwnd, bool bRTL);
POINT GetCenterOfDlgInParent(const RECT* rcDlg, const RECT* rcParent);
HWND GetParentOrDesktop(HWND hDlg);
void CenterDlgInParent(HWND hDlg, HWND hDlgParent);
void GetDlgPos(HWND hDlg, LPINT xDlg, LPINT yDlg);
void SetDlgPos(HWND hDlg, int xDlg, int yDlg);
//void SnapToDefaultButton(HWND);

inline void InitWindowCommon(HWND hwnd, bool bSetExplorerTheme) {
  if (bSetExplorerTheme) { SetExplorerTheme(hwnd); }
  if (Settings.DialogsLayoutRTL) { SetWindowLayoutRTL(hwnd, true); }
}

// resize dialog directions
typedef enum { RSZ_NONE = -1, RSZ_BOTH = 0, RSZ_ONLY_X = 1, RSZ_ONLY_Y = 2 } RSZ_DLG_DIR;

void ResizeDlg_InitEx(HWND hwnd, int cxFrame, int cyFrame, int nIdGrip, RSZ_DLG_DIR iDirection);

inline void ResizeDlg_Init0(HWND hwnd, int nIdGrip) {
  ResizeDlg_InitEx(hwnd, 0, 0, nIdGrip, RSZ_NONE);
}
inline void ResizeDlg_Init(HWND hwnd, int cxFrame, int cyFrame, int nIdGrip) {
  ResizeDlg_InitEx(hwnd, cxFrame, cyFrame, nIdGrip, RSZ_BOTH);
}
inline void ResizeDlg_InitX(HWND hwnd, int cxFrame, int nIdGrip) {
  ResizeDlg_InitEx(hwnd, cxFrame, 0, nIdGrip, RSZ_ONLY_X);
}
inline void ResizeDlg_InitY(HWND hwnd, int cyFrame, int nIdGrip) {
  ResizeDlg_InitEx(hwnd, 0, cyFrame, nIdGrip, RSZ_ONLY_Y);
}
void ResizeDlg_Destroy(HWND hwnd, int* cxFrame, int* cyFrame);
void ResizeDlg_Size(HWND hwnd, LPARAM lParam, int* cx, int* cy);
void ResizeDlg_GetMinMaxInfo(HWND hwnd, LPARAM lParam);

#define MAX_RESIZEDLG_ATTR_COUNT	2
void ResizeDlg_SetAttr(HWND hwnd, int index, int value);
int ResizeDlg_GetAttr(HWND hwnd, int index);

void ResizeDlg_InitY2Ex(HWND hwnd, int cxFrame, int cyFrame, int nIdGrip, RSZ_DLG_DIR iDirection, int nCtlId1, int nCtlId2);
inline void ResizeDlg_InitY2(HWND hwnd, int cxFrame, int cyFrame, int nIdGrip, int nCtlId1, int nCtlId2) {
  ResizeDlg_InitY2Ex(hwnd, cxFrame, cyFrame, nIdGrip, RSZ_BOTH, nCtlId1, nCtlId2);
}
int ResizeDlg_CalcDeltaY2(HWND hwnd, int dy, int cy, int nCtlId1, int nCtlId2);
void ResizeDlgCtl(HWND hwndDlg, int nCtlId, int dx, int dy);
HDWP DeferCtlPos(HDWP hdwp, HWND hwndDlg, int nCtlId, int dx, int dy, UINT uFlags);


void SetBitmapControl(HWND hwnd, int nCtrlId, HBITMAP hBmp);
void SetBitmapControlResample(HWND hwnd, int nCtrlId, HBITMAP hBmp, int width, int height);
void MakeBitmapButton(HWND hwnd, int nCtrlId, WORD uBmpId, int width, int height);
void MakeColorPickButton(HWND hwnd, int nCtrlId, HINSTANCE hInstance, COLORREF crColor);
void DeleteBitmapButton(HWND hwnd, int nCtrlId);


#define StatusSetSimple(hwnd,b) SendMessage(hwnd,SB_SIMPLE,(WPARAM)b,0)
void StatusSetText(HWND hwnd, UINT nPart, LPCWSTR lpszText);
bool StatusSetTextID(HWND hwnd, UINT nPart, UINT uID);

int Toolbar_GetButtons(HANDLE hwnd, int cmdBase, LPWSTR lpszButtons, int cchButtons);
int Toolbar_SetButtons(HANDLE, int, LPCWSTR, void*, int);

// ----------------------------------------------------------------------------

inline int GetDlgCtrlWidth(HWND hwndDlg, int nCtrlId)
{
  RECT rc; GetWindowRect(GetDlgItem(hwndDlg, nCtrlId), &rc);
  return (rc.right - rc.left);
}

inline int GetDlgCtrlHeight(HWND hwndDlg, int nCtrlId)
{
  RECT rc; GetWindowRect(GetDlgItem(hwndDlg, nCtrlId), &rc);
  return (rc.bottom - rc.top);
}

DPI_T GetCurrentPPI(HWND hwnd);

inline int ScaleIntByDPI(int val, unsigned dpi) { return MulDiv(val, dpi, USER_DEFAULT_SCREEN_DPI); }
inline int ScaleIntToDPI_X(HWND hwnd, int val) { DPI_T const dpi = Scintilla_GetWindowDPI(hwnd);  return ScaleIntByDPI(val, dpi.x); }
inline int ScaleIntToDPI_Y(HWND hwnd, int val) { DPI_T const dpi = Scintilla_GetWindowDPI(hwnd);  return ScaleIntByDPI(val, dpi.y); }

inline int ScaleFloatByDPI(float fVal, unsigned dpi) { return (int)lroundf((fVal * dpi) / (float)USER_DEFAULT_SCREEN_DPI); }
inline int ScaleFloatToDPI_X(HWND hwnd, float fVal) { DPI_T const dpi = Scintilla_GetWindowDPI(hwnd);  return ScaleFloatByDPI(fVal, dpi.x); }
inline int ScaleFloatToDPI_Y(HWND hwnd, float fVal) { DPI_T const dpi = Scintilla_GetWindowDPI(hwnd);  return ScaleFloatByDPI(fVal, dpi.y); }

inline unsigned LargeIconDPI() { return (unsigned)MulDiv(USER_DEFAULT_SCREEN_DPI, Settings2.LargeIconScalePrecent, 100); };

// ----------------------------------------------------------------------------

HBITMAP ConvertIconToBitmap(const HICON hIcon, const int cx, const int cy);
HBITMAP ResampleIconToBitmap(HWND hwnd, const HICON hIcon, const int cx, const int cy);
void SetUACIcon(HWND hwnd, const HMENU hMenu, const UINT nItem);
void UpdateWindowLayoutForDPI(HWND hwnd, const RECT* pRC, const DPI_T* pDPI);
//#define HandleDpiChangedMessage(hW,wP,lP) { DPI_T dpi; dpi.x = LOWORD(wP); dpi.y = HIWORD(wP); \
//                                            UpdateWindowLayoutForDPI(hW, (RECT*)lP, &dpi); }

#  define BMP_RESAMPLE_FILTER STOCK_FILTER_LANCZOS8
//#define BMP_RESAMPLE_FILTER   STOCK_FILTER_QUADRATICBSPLINE
HBITMAP ResampleImageBitmap(HWND hwnd, HBITMAP hbmp, int width, int height);
LRESULT SendWMSize(HWND hwnd, RECT* rc);
//HFONT   CreateAndSetFontDlgItemDPI(HWND hdlg, const int idDlgItem, int fontSize, bool bold);

// ----------------------------------------------------------------------------

inline void SimpleBeep() { if (!Settings.MuteMessageBeep) { MessageBeep(0xFFFFFFFF); } }
inline void AttentionBeep(UINT uType) { if (!Settings.MuteMessageBeep) { MessageBeep(uType & MB_ICONMASK); } }

#define DialogEnableControl(hdlg, id, b) { HWND hctrl = GetDlgItem((hdlg),(id)); if (!(b)) { \
  if (GetFocus() == hctrl) { SendMessage((hdlg), WM_NEXTDLGCTL, 0, false); } }; EnableWindow(hctrl, (b)); }

#define DialogHideControl(hdlg, id, b) { HWND hctrl = GetDlgItem((hdlg),(id)); if (!(b)) { \
  if (GetFocus() == hctrl) { SendMessage((hdlg), WM_NEXTDLGCTL, 0, false); } }; ShowWindow(hctrl, (b)?SW_HIDE:SW_SHOW); }

inline bool IsDialogItemEnabled(HWND hdlg, int id) { return IsWindowEnabled(GetDlgItem(hdlg, id)); }

inline void SetDialogIconNP3(HWND hwnd) {
  if (Globals.hDlgIconSmall) {
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIconSmall);
  }
  if (Globals.hDlgIconBig) {
    SendMessage((hwnd), WM_SETICON, ICON_BIG, (LPARAM)Globals.hDlgIconBig);
  }
}

// --- Themed Dialogs ---------------------------------------------------------

#ifndef DLGTEMPLATEEX
#pragma pack(push, 1)
typedef struct {
  WORD      dlgVer;
  WORD      signature;
  DWORD     helpID;
  DWORD     exStyle;
  DWORD     style;
  WORD      cDlgItems;
  short     x;
  short     y;
  short     cx;
  short     cy;
} DLGTEMPLATEEX;
#pragma pack(pop)
#endif

bool GetLocaleDefaultUIFont(LANGID lang, LPWSTR lpFaceName, WORD* wSize);
bool GetThemedDialogFont(LPWSTR lpFaceName, WORD* wSize);
DLGTEMPLATE* LoadThemedDialogTemplate(LPCTSTR lpDialogTemplateID, HINSTANCE hInstance);
#define ThemedDialogBox(hInstance,lpTemplate,hWndParent,lpDialogFunc) ThemedDialogBoxParam(hInstance,lpTemplate,hWndParent,lpDialogFunc,0)
INT_PTR ThemedDialogBoxParam(HINSTANCE hInstance, LPCTSTR lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
HWND    CreateThemedDialogParam(HINSTANCE hInstance, LPCTSTR lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);

// ----------------------------------------------------------------------------

#endif //_NP3_DIALOGS_H_
