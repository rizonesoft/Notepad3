/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Dialogs.h                                                                   *
*   Definitions for Notepad3 dialog boxes                                     *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2016   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#pragma once
#ifndef _NP3_DIALOGS_H_
#define _NP3_DIALOGS_H_

#include "TypeDefs.h"

int  MsgBoxLng(int iType, UINT uIdMsg, ...);
void DisplayCmdLineHelp(HWND hwnd);
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
bool SelectDefEncodingDlg(HWND hwnd,int * pidREncoding);
bool SelectEncodingDlg(HWND hwnd,int * pidREncoding);
bool RecodeDlg(HWND hwnd,int * pidREncoding);
bool SelectDefLineEndingDlg(HWND hwnd,LPARAM piOption);

bool GetMonitorInfoFromRect(const RECT* rc, MONITORINFO* hMonitorInfo);
void WinInfoToScreen(WININFO* pWinInfo);
WININFO GetMyWindowPlacement(HWND hwnd,MONITORINFO * hMonitorInfo);
void FitIntoMonitorWorkArea(RECT* pRect, WININFO* pWinInfo, bool);
WINDOWPLACEMENT WindowPlacementFromInfo(HWND hwnd, const WININFO* pWinInfo);

void DialogNewWindow(HWND hwnd,bool,bool);
void DialogFileBrowse(HWND hwnd);
void DialogAdminExe(HWND hwnd,bool);
INT_PTR InfoBoxLng(int iType, LPCWSTR lpstrSetting, int uidMessage, ...);

bool SetWindowTitle(HWND hwnd, UINT uIDAppName, bool, UINT uIDUntitled, LPCWSTR lpszFile, int iFormat, bool, UINT uIDReadOnly, bool, LPCWSTR lpszExcerpt);
void SetWindowTransparentMode(HWND hwnd, bool);
void CenterDlgInParent(HWND hDlg);
void GetDlgPos(HWND hDlg, LPINT xDlg, LPINT yDlg);
void SetDlgPos(HWND hDlg, int xDlg, int yDlg);
//void SnapToDefaultButton(HWND);
void ResizeDlg_Init(HWND hwnd, int cxFrame, int cyFrame, int nIdGrip);
void ResizeDlg_Destroy(HWND hwnd, int* cxFrame, int* cyFrame);
void ResizeDlg_Size(HWND hwnd, LPARAM lParam, int* cx, int* cy);
void ResizeDlg_GetMinMaxInfo(HWND hwnd, LPARAM lParam);
HDWP DeferCtlPos(HDWP hdwp, HWND hwndDlg, int nCtlId, int dx, int dy, UINT uFlags);
void MakeBitmapButton(HWND hwnd, int nCtlId, HINSTANCE hInstance, WORD uBmpId); 
void MakeColorPickButton(HWND hwnd, int nCtlId, HINSTANCE hInstance, COLORREF crColor);
void DeleteBitmapButton(HWND hwnd, int nCtlId);


#define StatusSetSimple(hwnd,b) SendMessage(hwnd,SB_SIMPLE,(WPARAM)b,0)
void StatusSetText(HWND hwnd, UINT nPart, LPCWSTR lpszText);
bool StatusSetTextID(HWND hwnd, UINT nPart, UINT uID);

int Toolbar_GetButtons(HWND hwnd, int cmdBase, LPWSTR lpszButtons, int cchButtons);
int Toolbar_SetButtons(HWND, int, LPCWSTR, void*, int);

LRESULT SendWMSize(HWND hwnd, RECT* rc);


//==== Themed Dialogs =========================================================
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
#define ThemedDialogBox(hInstance,lpTemplate,hWndParent,lpDialogFunc) \
  ThemedDialogBoxParam(hInstance,lpTemplate,hWndParent,lpDialogFunc,0)
INT_PTR ThemedDialogBoxParam(HINSTANCE hInstance, LPCTSTR lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
HWND    CreateThemedDialogParam(HINSTANCE hInstance, LPCTSTR lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);


#endif //_NP3_DIALOGS_H_
