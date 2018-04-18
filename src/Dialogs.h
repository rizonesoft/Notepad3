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

int  MsgBox(int,UINT,...);
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


WININFO GetMyWindowPlacement(HWND,MONITORINFO *);

void DialogNewWindow(HWND,bool,bool);
void DialogFileBrowse(HWND);
void DialogUpdateCheck(HWND,bool);
INT_PTR InfoBox(int,LPCWSTR,int,...);


#endif //_NP3_DIALOGS_H_
