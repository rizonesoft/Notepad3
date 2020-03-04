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
#include "TypeDefs.h"

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
bool LongLineSettingsDlg(HWND hwnd,UINT uidDlg,int * iNumber);
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
void DialogAdminExe(HWND hwnd,bool);

int  MessageBoxLng(HWND hwnd, UINT uType, UINT uIdMsg, ...);
INT_PTR InfoBoxLng(UINT uType, LPCWSTR lpstrSetting, UINT uidMessage, ...);
DWORD MsgBoxLastError(LPCWSTR lpszMessage, DWORD dwErrID);

bool SetWindowTitle(HWND hwnd, UINT uIDAppName, bool, UINT uIDUntitled, LPCWSTR lpszFile, 
                    int iFormat, bool bIsElevated, UINT uIDReadOnly, bool, LPCWSTR lpszExcerpt);
void SetAdditionalTitleInfo(LPCWSTR lpszAddTitleInfo);
void AppendAdditionalTitleInfo(LPCWSTR lpszAddTitleInfo);
void SetWindowTransparentMode(HWND hwnd, bool bTransparentMode, int iOpacityLevel);
POINT GetCenterOfDlgInParent(const RECT* rcDlg, const RECT* rcParent);
void CenterDlgInParent(HWND hDlg, HWND hDlgParent);
void GetDlgPos(HWND hDlg, LPINT xDlg, LPINT yDlg);
void SetDlgPos(HWND hDlg, int xDlg, int yDlg);
//void SnapToDefaultButton(HWND);

#define ResizeDlgDirection_Both		0
#define ResizeDlgDirection_OnlyX	1
#define ResizeDlgDirection_OnlyY	2
void ResizeDlg_InitEx(HWND hwnd, int cxFrame, int cyFrame, int nIdGrip, int iDirection);
inline void ResizeDlg_Init(HWND hwnd, int cxFrame, int cyFrame, int nIdGrip) {
  ResizeDlg_InitEx(hwnd, cxFrame, cyFrame, nIdGrip, ResizeDlgDirection_Both);
}
inline void ResizeDlg_InitX(HWND hwnd, int cxFrame, int nIdGrip) {
  ResizeDlg_InitEx(hwnd, cxFrame, 0, nIdGrip, ResizeDlgDirection_OnlyX);
}
inline void ResizeDlg_InitY(HWND hwnd, int cyFrame, int nIdGrip) {
  ResizeDlg_InitEx(hwnd, 0, cyFrame, nIdGrip, ResizeDlgDirection_OnlyY);
}
void ResizeDlg_Destroy(HWND hwnd, int* cxFrame, int* cyFrame);
void ResizeDlg_Size(HWND hwnd, LPARAM lParam, int* cx, int* cy);
void ResizeDlg_GetMinMaxInfo(HWND hwnd, LPARAM lParam);

#define MAX_RESIZEDLG_ATTR_COUNT	2
void ResizeDlg_SetAttr(HWND hwnd, int index, int value);
int ResizeDlg_GetAttr(HWND hwnd, int index);

void ResizeDlg_InitY2Ex(HWND hwnd, int cxFrame, int cyFrame, int nIdGrip, int iDirection, int nCtlId1, int nCtlId2);
inline void ResizeDlg_InitY2(HWND hwnd, int cxFrame, int cyFrame, int nIdGrip, int nCtlId1, int nCtlId2) {
  ResizeDlg_InitY2Ex(hwnd, cxFrame, cyFrame, nIdGrip, ResizeDlgDirection_Both, nCtlId1, nCtlId2);
}
int ResizeDlg_CalcDeltaY2(HWND hwnd, int dy, int cy, int nCtlId1, int nCtlId2);
void ResizeDlgCtl(HWND hwndDlg, int nCtlId, int dx, int dy);
HDWP DeferCtlPos(HDWP hdwp, HWND hwndDlg, int nCtlId, int dx, int dy, UINT uFlags);


void MakeBitmapButton(HWND hwnd, int nCtlId, HINSTANCE hInstance, WORD uBmpId); 
void MakeColorPickButton(HWND hwnd, int nCtlId, HINSTANCE hInstance, COLORREF crColor);
void DeleteBitmapButton(HWND hwnd, int nCtlId);


#define StatusSetSimple(hwnd,b) SendMessage(hwnd,SB_SIMPLE,(WPARAM)b,0)
void StatusSetText(HWND hwnd, UINT nPart, LPCWSTR lpszText);
bool StatusSetTextID(HWND hwnd, UINT nPart, UINT uID);

int Toolbar_GetButtons(HANDLE hwnd, int cmdBase, LPWSTR lpszButtons, int cchButtons);
int Toolbar_SetButtons(HANDLE, int, LPCWSTR, void*, int);

// ----------------------------------------------------------------------------

DPI_T GetCurrentDPI(HWND hwnd);
DPI_T GetCurrentPPI(HWND hwnd);

int GetSystemMetricsEx(int nValue);

// ----------------------------------------------------------------------------

inline int ScaleIntToCurrentDPI(int val) { return MulDiv((val), Globals.MainWndDPI.y, USER_DEFAULT_SCREEN_DPI); }
inline int ScaleToCurrentDPI(float fVal) { return (int)lroundf((fVal * Globals.MainWndDPI.y) / (float)USER_DEFAULT_SCREEN_DPI); }
inline int ScaleIntFontSize(int val) { return MulDiv((val), Globals.MainWndDPI.y, Globals.MainWndPPI.y); }
inline int ScaleFontSize(float fSize) { return (int)lroundf((fSize * Globals.MainWndDPI.y) / (float)Globals.MainWndPPI.y); }
inline int ScaleFractionalFontSize(float fSize) { return (int)lroundf((fSize * 10.0f * Globals.MainWndDPI.y) / (float)Globals.MainWndPPI.y) * 10; }

HBITMAP ConvertIconToBitmap(const HICON hIcon, const int cx, const int cy);
void SetUACIcon(const HMENU hMenu, const UINT nItem);
void UpdateWindowLayoutForDPI(HWND hWnd, int x_96dpi, int y_96dpi, int w_96dpi, int h_96dpi);
HBITMAP ResizeImageForCurrentDPI(HBITMAP hbmp);
LRESULT SendWMSize(HWND hwnd, RECT* rc);

// ----------------------------------------------------------------------------

inline void SimpleBeep() { if (!Settings.MuteMessageBeep) { MessageBeep(0xFFFFFFFF); } }
inline void AttentionBeep(UINT uType) { if (!Settings.MuteMessageBeep) { MessageBeep(uType & MB_ICONMASK); } }

#define DialogEnableControl(hdlg, id, b) { HWND hctrl = GetDlgItem((hdlg),(id)); if (!(b)) { \
  if (GetFocus() == hctrl) { SendMessage((hdlg), WM_NEXTDLGCTL, 0, false); } }; EnableWindow(hctrl, (b)); }

#define DialogHideControl(hdlg, id, b) { HWND hctrl = GetDlgItem((hdlg),(id)); if (!(b)) { \
  if (GetFocus() == hctrl) { SendMessage((hdlg), WM_NEXTDLGCTL, 0, false); } }; ShowWindow(hctrl, (b)?SW_HIDE:SW_SHOW); }

 
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

bool GetThemedDialogFont(LPWSTR lpFaceName, WORD* wSize);
DLGTEMPLATE* LoadThemedDialogTemplate(LPCTSTR lpDialogTemplateID, HINSTANCE hInstance);
#define ThemedDialogBox(hInstance,lpTemplate,hWndParent,lpDialogFunc) ThemedDialogBoxParam(hInstance,lpTemplate,hWndParent,lpDialogFunc,0)
INT_PTR ThemedDialogBoxParam(HINSTANCE hInstance, LPCTSTR lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
HWND    CreateThemedDialogParam(HINSTANCE hInstance, LPCTSTR lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);

// ----------------------------------------------------------------------------

#endif //_NP3_DIALOGS_H_
