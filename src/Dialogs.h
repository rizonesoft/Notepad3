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

int  MsgBoxLng(int, UINT, ...);
void DisplayCmdLineHelp(HWND);
bool GetDirectory(HWND,int,LPWSTR,LPCWSTR,bool);
INT_PTR CALLBACK AboutDlgProc(HWND,UINT,WPARAM,LPARAM);
void RunDlg(HWND,LPCWSTR);
bool OpenWithDlg(HWND,LPCWSTR);
bool FavoritesDlg(HWND,LPWSTR);
bool AddToFavDlg(HWND,LPCWSTR,LPCWSTR);
bool FileMRUDlg(HWND,LPWSTR);
bool ChangeNotifyDlg(HWND);
bool ColumnWrapDlg(HWND,UINT,UINT *);
bool WordWrapSettingsDlg(HWND,UINT,int *);
bool LongLineSettingsDlg(HWND,UINT,int *);
bool TabSettingsDlg(HWND,UINT,int *);
bool SelectDefEncodingDlg(HWND,int *);
bool SelectEncodingDlg(HWND,int *);
bool RecodeDlg(HWND,int *);
bool SelectDefLineEndingDlg(HWND,int *);

void WinInfoToScreen(WININFO*);
WININFO GetMyWindowPlacement(HWND,MONITORINFO *);
void FitIntoMonitorWorkArea(RECT*, WININFO*, bool);
WINDOWPLACEMENT WindowPlacementFromInfo(HWND, const WININFO* const);

void DialogNewWindow(HWND,bool,bool);
void DialogFileBrowse(HWND);
void DialogAdminExe(HWND,bool);
INT_PTR InfoBoxLng(int, LPCWSTR, int, ...);

bool SetWindowTitle(HWND, UINT, bool, UINT, LPCWSTR, int, bool, UINT, bool, LPCWSTR);
void SetWindowTransparentMode(HWND, bool);
void CenterDlgInParent(HWND);
void GetDlgPos(HWND, LPINT, LPINT);
void SetDlgPos(HWND, int, int);
//void SnapToDefaultButton(HWND);
void ResizeDlg_Init(HWND, int, int, int);
void ResizeDlg_Destroy(HWND, int*, int*);
void ResizeDlg_Size(HWND, LPARAM, int*, int*);
void ResizeDlg_GetMinMaxInfo(HWND, LPARAM);
HDWP DeferCtlPos(HDWP, HWND, int, int, int, UINT);
void MakeBitmapButton(HWND, int, HINSTANCE, UINT);
void MakeColorPickButton(HWND, int, HINSTANCE, COLORREF);
void DeleteBitmapButton(HWND, int);


#define StatusSetSimple(hwnd,b) SendMessage(hwnd,SB_SIMPLE,(WPARAM)b,0)
void StatusSetText(HWND, UINT, LPCWSTR);
bool StatusSetTextID(HWND, UINT, UINT);

int Toolbar_GetButtons(HWND, int, LPWSTR, int);
int Toolbar_SetButtons(HWND, int, LPCWSTR, void*, int);

LRESULT SendWMSize(HWND, RECT*);


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

bool GetThemedDialogFont(LPWSTR, WORD*);
DLGTEMPLATE* LoadThemedDialogTemplate(LPCTSTR, HINSTANCE);
#define ThemedDialogBox(hInstance,lpTemplate,hWndParent,lpDialogFunc) \
  ThemedDialogBoxParam(hInstance,lpTemplate,hWndParent,lpDialogFunc,0)
INT_PTR ThemedDialogBoxParam(HINSTANCE, LPCTSTR, HWND, DLGPROC, LPARAM);
HWND    CreateThemedDialogParam(HINSTANCE, LPCTSTR, HWND, DLGPROC, LPARAM);


#endif //_NP3_DIALOGS_H_
