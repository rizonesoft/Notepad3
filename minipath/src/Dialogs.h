// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* MiniPath - Notepad3 Explorer Plugin                                         *
*                                                                             *
* Dialogs.h                                                                   *
*   Definitions for metapath dialog boxes                                     *
*   Based on code from metapath, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2025   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#include <uxtheme.h>

BOOL GetDirectory(HWND,int,LPWSTR,LPCWSTR,BOOL);
BOOL GetDirectory2(HWND,int,LPWSTR,int);

INT_PTR CALLBACK RunDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR RunDlg(HWND);

INT_PTR CALLBACK GotoDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR GotoDlg(HWND);

INT_PTR CALLBACK AboutDlgProc(HWND,UINT,WPARAM,LPARAM);
INT_PTR OptionsPropSheet(HWND, HINSTANCE);

INT_PTR CALLBACK GetFilterDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL GetFilterDlg(HWND);

INT_PTR CALLBACK RenameFileDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL RenameFileDlg(HWND);

INT_PTR CALLBACK CopyMoveDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CopyMoveDlg(HWND, UINT*);

INT_PTR CALLBACK OpenWithDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL OpenWithDlg(HWND, LPDLITEM);

INT_PTR CALLBACK NewDirDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL NewDirDlg(HWND,LPWSTR);

INT_PTR CALLBACK FindWinDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK FindTargetDlgProc(HWND, UINT, WPARAM, LPARAM);

int  ErrorMessage(int, UINT, ...);
DWORD MsgBoxLastError(LPCWSTR lpszMessage, DWORD dwErrID);

#define SetExplorerTheme(hwnd) SetWindowTheme((hwnd), L"Explorer", NULL)

inline void InitWindowCommon(HWND hwnd, BOOL bSetExplorerTheme)
{
    if (bSetExplorerTheme) {
        SetExplorerTheme(hwnd);
    } else {
        SetWindowTheme(hwnd, L"", L"");
    }
    //SetWindowLayoutRTL(hwnd, Settings.DialogsLayoutRTL);
}


// End of Dialogs.h
