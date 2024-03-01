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
*                                                  (c) Rizonesoft 2008-2024   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#pragma once
#ifndef _NP3_DIALOGS_H_
#define _NP3_DIALOGS_H_

#include <math.h>
#include <uxtheme.h>

#include "win/dlgs.h" // Windows Dialog API infos

#include "TypeDefs.h"
#include "Scintilla.h"

// ----------------------------------------------------------------------------

#define DIALOG_FONT_SIZE_INCR 0  // will increase default dialog font size

#define SetExplorerTheme(hwnd) SetWindowTheme((hwnd), L"Explorer", NULL)

#define UpdateWindowEx(hwnd) /* UpdateWindow(hwnd) */ \
  RedrawWindow((hwnd), NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_INTERNALPAINT /* | RDW_UPDATENOW */)

// additional InfoBoxLng() state (MB_CANCELTRYCONTINUE == 0x00000006L)
#define MB_FILECHANGEDNOTIFY 0x0000000EL

// ----------------------------------------------------------------------------

// === MinimizeToTray Functions - see comments in Dialogs.c  ===
bool HasDrawAnimation();
void MinimizeWndToTray(HWND hWnd);
void MinimizeWndToTaskbar(HWND hWnd);
void RestoreWndFromTray(HWND hWnd);
void RestoreWndFromTaskbar(HWND hWnd);


INT_PTR DisplayCmdLineHelp(HWND hwnd);
INT_PTR CALLBACK AboutDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam);
INT_PTR RunDlg(HWND hwnd,LPCWSTR lpstrDefault);
bool OpenWithDlg(HWND hwnd,LPCWSTR lpstrFile);
bool FavoritesDlg(HWND hwnd, HPATHL hpath_in_out);
bool AddToFavDlg(HWND hwnd, HPATHL hTargetPth);
bool FileMRUDlg(HWND hwnd, HPATHL hFilePath_out);
bool ChangeNotifyDlg(HWND hwnd);
bool ColumnWrapDlg(HWND hwnd,UINT uidDlg,UINT * iNumber);
bool WordWrapSettingsDlg(HWND hwnd,UINT uidDlg,int * iNumber);
bool LongLineSettingsDlg(HWND hwnd,UINT uidDlg, LPWSTR pColList);
bool TabSettingsDlg(HWND hwnd,UINT uidDlg,int * iNumber);
bool SelectDefEncodingDlg(HWND hwnd, cpi_enc_t* pidREncoding);
bool SelectEncodingDlg(HWND hwnd, cpi_enc_t* pidREncoding, bool bRecode);
bool SelectDefLineEndingDlg(HWND hwnd,LPARAM piOption);
bool WarnLineEndingDlg(HWND hwnd, EditFileIOStatus* fioStatus);
bool WarnIndentationDlg(HWND hwnd, EditFileIOStatus* fioStatus);
bool AutoSaveBackupSettingsDlg(HWND hwnd);

void            RelAdjustRectForDPI(LPRECT rc, const UINT oldDPI, const UINT newDPI);
void            MapRectClientToWndCoords(HWND hwnd, LPRECT rc);
bool            GetMonitorInfoFromRect(const LPRECT rc, MONITORINFO* hMonitorInfo);
void            WinInfoToScreenCoord(WININFO* pWinInfo);
WININFO         GetMyWindowPlacement(HWND hwnd, MONITORINFO* hMonitorInfo, const int offset, const bool bFullVisible);
bool            GetWindowRectEx(HWND hwnd, LPRECT pRect);
void            FitIntoMonitorGeometry(LPRECT pRect, WININFO* pWinInfo, SCREEN_MODE mode, bool bTopLeft);
WINDOWPLACEMENT WindowPlacementFromInfo(HWND hwnd, const WININFO* pWinInfo, SCREEN_MODE mode, UINT nCmdShow);
void            SnapToWinInfoPos(HWND hwnd, const WININFO winInfo, SCREEN_MODE mode, UINT nCmdShow);
void            RestorePrevScreenPos(HWND hwnd);

void DialogNewWindow(HWND hwnd, bool bSaveOnRunTools, const HPATHL hFilePath, WININFO* wi);
void DialogFileBrowse(HWND hwnd);
void DialogGrepWin(HWND hwnd, LPCWSTR searchPattern);
void DialogAdminExe(HWND hwnd,bool);

int  MessageBoxLng(UINT uType, UINT uidMsg, ...);
DWORD MsgBoxLastError(LPCWSTR lpszMessage, DWORD dwErrID);

LONG InfoBoxLng(UINT uType, LPCWSTR lpstrSetting, UINT uidMsg, ...);
#define INFOBOX_ANSW(_R_) LOWORD(_R_)
#define INFOBOX_MODE(_R_) HIWORD(_R_)

inline bool IsYesOkay(INT_PTR answ) {
    return ((LOWORD(answ) == IDOK) || (LOWORD(answ) == IDYES));
}
inline bool IsRetryContinue(INT_PTR answ) {
    return ((LOWORD(answ) == IDRETRY) || (LOWORD(answ) == IDCONTINUE));
}
inline bool IsNoCancelClose(INT_PTR answ) {
    return ((LOWORD(answ) == IDNO) || (LOWORD(answ) == IDCANCEL) || (LOWORD(answ) == 0));
}

typedef struct TITLEPROPS_T {
    int  iFormat;
    bool bPasteBoard;
    bool bIsElevated;
    bool bModified;
    bool bFileLocked;
    bool bFileChanged;
    bool bFileDeleted;
    bool bReadOnly;
} TITLEPROPS_T;

void SetWindowTitle(HWND hwnd, const HPATHL pthFilePath, TITLEPROPS_T properties, LPCWSTR lpszExcerpt, bool forceRedraw);

void SetAdditionalTitleInfo(LPCWSTR lpszAddTitleInfo);
void AppendAdditionalTitleInfo(LPCWSTR lpszAddTitleInfo);
void SetWindowTransparentMode(HWND hwnd, bool bTransparentMode, int iOpacityLevel);
void SetWindowLayoutRTL(HWND hwnd, bool bRTL);
void SetWindowReadingRTL(HWND hwnd, bool bRTL);

UINT ComboBox_GetTextLengthEx(HWND hDlg, int nIDDlgItem);
UINT ComboBox_GetCurSelEx(HWND hDlg, int nIDDlgItem);
int  ComboBox_GetTextHW(HWND hDlg, int nIDDlgItem, HSTRINGW hstr);
int  ComboBox_GetTextW2MB(HWND hDlg, int nIDDlgItem, LPSTR lpString, size_t cch);
void ComboBox_SetTextW(HWND hDlg, int nIDDlgItem, LPCWSTR wstr);
void ComboBox_SetTextHW(HWND hDlg, int nIDDlgItem, const HSTRINGW hstr);
void ComboBox_SetTextMB2W(HWND hDlg, int nIDDlgItem, LPCSTR lpString);
//void ComboBox_AddStringMB2W(HWND hDlg, int nIDDlgItem, LPCSTR lpString);

POINT GetCenterOfDlgInParent(const RECT* rcDlg, const RECT* rcParent);
HWND GetParentOrDesktop(HWND hDlg);
void CenterDlgInParent(HWND hDlg, HWND hDlgParent);
void GetDlgPos(HWND hDlg, LPINT xDlg, LPINT yDlg);
void SetDlgPos(HWND hDlg, int xDlg, int yDlg);


inline void InitWindowCommon(HWND hwnd, bool bSetExplorerTheme)
{
    if (bSetExplorerTheme) {
        SetExplorerTheme(hwnd);
    } else {
        SetWindowTheme(hwnd, L"", L"");
    }
    SetWindowLayoutRTL(hwnd, Settings.DialogsLayoutRTL);
}


// resize dialog directions
typedef enum { RSZ_NONE = -1, RSZ_BOTH = 0, RSZ_ONLY_X = 1, RSZ_ONLY_Y = 2 } RSZ_DLG_DIR;

void ResizeDlg_InitEx(HWND hwnd, int cxFrame, int cyFrame, int nIdGrip, RSZ_DLG_DIR iDirection);

inline void ResizeDlg_Init0(HWND hwnd, int nIdGrip)
{
    ResizeDlg_InitEx(hwnd, 0, 0, nIdGrip, RSZ_NONE);
}
inline void ResizeDlg_Init(HWND hwnd, int cxFrame, int cyFrame, int nIdGrip)
{
    ResizeDlg_InitEx(hwnd, cxFrame, cyFrame, nIdGrip, RSZ_BOTH);
}
inline void ResizeDlg_InitX(HWND hwnd, int cxFrame, int nIdGrip)
{
    ResizeDlg_InitEx(hwnd, cxFrame, 0, nIdGrip, RSZ_ONLY_X);
}
inline void ResizeDlg_InitY(HWND hwnd, int cyFrame, int nIdGrip)
{
    ResizeDlg_InitEx(hwnd, 0, cyFrame, nIdGrip, RSZ_ONLY_Y);
}
void ResizeDlg_Destroy(HWND hwnd, int* cxFrame, int* cyFrame);
void ResizeDlg_Size(HWND hwnd, LPARAM lParam, int* cx, int* cy);
void ResizeDlg_GetMinMaxInfo(HWND hwnd, LPARAM lParam);

#define MAX_RESIZEDLG_ATTR_COUNT	2
void ResizeDlg_SetAttr(HWND hwnd, int index, int value);
int ResizeDlg_GetAttr(HWND hwnd, int index);

void        ResizeDlg_InitY2Ex(HWND hwnd, int cxFrame, int cyFrame, int nIdGrip, int iDirection, int nCtlId1, int nCtlId2);
inline void ResizeDlg_InitY2(HWND hwnd, int cxFrame, int cyFrame, int nIdGrip, int nCtlId1, int nCtlId2)
{
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

void StatusSetText(HWND hwnd, BYTE nPart, LPCWSTR lpszText);
void StatusSetTextID(HWND hwnd, BYTE nPart, UINT uID);

int Toolbar_GetButtons(HANDLE hwnd, int cmdBase, LPWSTR lpszButtons, int cchButtons);
int Toolbar_SetButtons(HANDLE hwnd, int cmdBase, LPCWSTR lpszButtons, LPCTBBUTTON ptbb, int ctbb);

    // ----------------------------------------------------------------------------

inline int GetDlgCtrlWidth(HWND hwndDlg, int nCtrlId)
{
    RECT rc;
    GetWindowRect(GetDlgItem(hwndDlg, nCtrlId), &rc);
    return (rc.right - rc.left);
}

inline int GetDlgCtrlHeight(HWND hwndDlg, int nCtrlId)
{
    RECT rc;
    GetWindowRect(GetDlgItem(hwndDlg, nCtrlId), &rc);
    return (rc.bottom - rc.top);
}

UINT GetCurrentPPI(HWND hwnd);

inline int ScaleIntByDPI(int val, unsigned dpi)
{
    return MulDiv(val, dpi, USER_DEFAULT_SCREEN_DPI);
}
inline int ScaleIntToDPI(HWND hwnd, int val)
{
    UINT const dpi = Scintilla_GetWindowDPI(hwnd);
    return ScaleIntByDPI(val, dpi);
}

inline int ScaleFloatByDPI(float fVal, unsigned dpi)
{
    return (int)lroundf((fVal * dpi) / (float)USER_DEFAULT_SCREEN_DPI);
}
inline int ScaleFloatToDPI(HWND hwnd, float fVal)
{
    UINT const dpi = Scintilla_GetWindowDPI(hwnd);
    return ScaleFloatByDPI(fVal, dpi);
}

inline unsigned LargeIconDPI()
{
    return (unsigned)MulDiv(USER_DEFAULT_SCREEN_DPI, Settings2.LargeIconScalePrecent, 100);
};

// ----------------------------------------------------------------------------

HBITMAP ConvertIconToBitmap(const HICON hIcon, const int cx, const int cy);
HBITMAP ResampleIconToBitmap(HWND hwnd, HBITMAP hOldBmp, const HICON hIcon, const int cx, const int cy);
void    SetUACIcon(HWND hwnd, const HMENU hMenu, const UINT nItem);
void    SetWinIcon(HWND hwnd, const HMENU hMenu, const UINT nItem);
void    SetGrepWinIcon(HWND hwnd, const HMENU hMenu, const UINT nItem);
void    UpdateWindowLayoutForDPI(HWND hwnd, const RECT *pRC, const UINT dpi);
//#define HandleDpiChangedMessage(hW,wP,lP) { UINT dpi; dpi = LOWORD(wP); /*dpi = HIWORD(wP);*/ \
//                                            UpdateWindowLayoutForDPI(hW, (RECT*)lP, dpi); }

#  define BMP_RESAMPLE_FILTER STOCK_FILTER_LANCZOS8
//#define BMP_RESAMPLE_FILTER   STOCK_FILTER_QUADRATICBSPLINE
HBITMAP ResampleImageBitmap(HWND hwnd, HBITMAP hbmp, int width, int height);
LRESULT SendWMSize(HWND hwnd, RECT* rc);
void    UpdateUI(HWND hwnd);
// HFONT   CreateAndSetFontDlgItemDPI(HWND hdlg, const int idDlgItem, int fontSize, bool bold);

// ----------------------------------------------------------------------------

inline void SimpleBeep()
{
    if (!Settings.MuteMessageBeep) {
        MessageBeep(0xFFFFFFFF);
    }
}
inline void AttentionBeep(UINT uType)
{
    if (!Settings.MuteMessageBeep) {
        MessageBeep(uType & MB_ICONMASK);
    }
}

#define DialogEnableControl(hdlg, id, b) { HWND hctrl = GetDlgItem((hdlg),(id)); if (!(b)) { \
  if (GetFocus() == hctrl) { SendMessage((hdlg), WM_NEXTDLGCTL, 0, false); } }; EnableWindow(hctrl, (b)); }

#define DialogHideControl(hdlg, id, b) { HWND hctrl = GetDlgItem((hdlg),(id)); if (!(b)) { \
  if (GetFocus() == hctrl) { SendMessage((hdlg), WM_NEXTDLGCTL, 0, false); } }; ShowWindow(hctrl, (b)?SW_HIDE:SW_SHOW); }

inline bool IsDialogControlEnabled(HWND hdlg, int id)
{
    return IsWindowEnabled(GetDlgItem(hdlg, id));
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
void    CleanupDlgResources();

//bool GetFolderDlg(HWND hwnd, HPATHL hdir_pth_io, const HPATHL hinidir_pth);
bool OpenFileDlg(HWND hwnd, HPATHL hfile_pth_io, const HPATHL hinidir_pth);
bool SaveFileDlg(HWND hwnd, HPATHL hfile_pth_io, const HPATHL hinidir_pth);


// --- Hook Procedures for Std-System Dialogs ---------------------------------

extern WCHAR FontSelTitle[128];
INT_PTR CALLBACK FontDialogHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam); // LPCFHOOKPROC
INT_PTR CALLBACK ColorDialogHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam); // LPCCHOOKPROC

// ----------------------------------------------------------------------------

#endif //_NP3_DIALOGS_H_
