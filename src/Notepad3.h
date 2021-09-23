// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Notepad3.h                                                                  *
*   Global definitions and declarations                                       *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2021   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#pragma once
#ifndef _NP3_NOTEPAD3_H_
#define _NP3_NOTEPAD3_H_

#include "TypeDefs.h"
#include "SciCall.h"
#include "uthash/utarray.h"

//==== Main Window ============================================================

#define ONLINE_HELP_WEBSITE L"https://www.rizonesoft.com/documents/notepad3/"

//==== Data Type for WM_COPYDATA ==============================================
#define DATA_NOTEPAD3_PARAMS 0xFB10
typedef struct np3params {
    int                 flagFileSpecified;
    FILE_WATCHING_MODE  flagChangeNotify;
    int                 flagLexerSpecified;
    int                 iInitialLexer;
    int                 flagQuietCreate;
    int                 flagJumpTo;
    int                 iInitialLine;
    int                 iInitialColumn;
    cpi_enc_t           flagSetEncoding;
    int                 flagSetEOLMode;
    int                 flagTitleExcerpt;
    int                 flagMatchText;
    WCHAR               wchData;
}
np3params, *LPnp3params;


#pragma pack(push, 1)
typedef struct _undoSel {
    int selMode_undo;
    UT_array* anchorPos_undo;
    UT_array* curPos_undo;
    UT_array* anchorVS_undo;
    UT_array* curVS_undo;

    int selMode_redo;
    UT_array* anchorPos_redo;
    UT_array* curPos_redo;
    UT_array* anchorVS_redo;
    UT_array* curVS_redo;
}
UndoRedoSelection_t;
#pragma pack(pop)

//#define INIT_UNDOREDOSEL  { SC_SEL_STREAM, (DocPos)-1, (DocPos)-1, 0, 0, SC_SEL_STREAM, (DocPos)-1, (DocPos)-1, 0, 0 }
#define INIT_UNDOREDOSEL  { SC_SEL_STREAM, NULL, NULL, NULL, NULL, SC_SEL_STREAM, NULL, NULL, NULL, NULL }

#define NP3_SEL_MULTI  (SC_SEL_RECTANGLE + SC_SEL_LINES + SC_SEL_THIN)

typedef enum {
    UNDO = true,
    REDO = false

} DoAction;


//==== Ids ====================================================================
#define IDC_STATUSBAR    (0xFB00)
#define IDC_TOOLBAR      (0xFB01)
#define IDC_REBAR        (0xFB02)
#define IDC_EDIT         (0xFB03)
#define IDC_EDITFRAME    (0xFB04)
#define IDC_FILENAME     (0xFB05)
#define IDC_REUSELOCK    (0xFB06)
#define IDC_MARGIN       (0xFB07)
#define IDC_NCAREA       (0xFB08)



//==== Notifications ==========================================================
#define WM_TRAYMESSAGE         WM_USER          // Callback Message from System Tray
#define WM_FILECHANGEDNOTIFY  (WM_USER+1)       // Change Notifications
//#define WM_CHANGENOTIFYCLEAR (WM_USER+2)

//==== Timer ==================================================================
#define ID_WATCHTIMER       (0xA000)        // File Watching
#define ID_PASTEBOARDTIMER  (0xA001)        // Paste Board


//==== Reuse Window Lock Timeout ==============================================
#define REUSEWINDOWLOCKTIMEOUT  (1000)


//==== Function Declarations ==================================================
bool InitApplication(const HINSTANCE hInstance);
//~bool InitToolbarWndClass(const HINSTANCE hInstance);
HWND InitInstance(const HINSTANCE hInstance, LPCWSTR pszCmdLine, int nCmdShow);
WININFO GetFactoryDefaultWndPos(const int flagsPos);
WININFO GetWinInfoByFlag(const int flagsPos);
bool ActivatePrevInst();
bool LaunchNewInstance(HWND hwnd, LPCWSTR lpszParameter, LPCWSTR lpszFilePath);
bool RelaunchMultiInst();
bool RelaunchElevated(LPWSTR lpNewCmdLnArgs);
bool DoElevatedRelaunch(EditFileIOStatus* pFioStatus, bool bAutoSaveOnRelaunch);
void SnapToWinInfoPos(HWND hwnd, const WININFO winInfo, SCREEN_MODE mode);
void ShowNotifyIcon(HWND hwnd, bool bAdd);
void SetNotifyIconTitle(HWND hwnd);
void InstallFileWatching(const bool bInstall);
//bool GetDocModified();
void SetSavePoint();
void CALLBACK PasteBoardTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

void ParseCommandLine();
void ShowZoomCallTip();
void ShowWrapAroundCallTip(bool forwardSearch);

void MarkAllOccurrences(const int delay, const bool bForceClear);
void UpdateUI();
void UpdateToolbar();
void UpdateStatusbar(const bool bForceRedraw);
void UpdateMarginWidth(const bool bForce);
void UpdateSaveSettingsCmds();
void ResetMouseDWellTime();
void UpdateTitleBar(const HWND hwnd);

void UndoRedoRecordingStart();
void UndoRedoRecordingStop();
void UndoRedoReset();
int  BeginUndoAction();
void EndUndoAction(int token);
bool RestoreAction(int token, DoAction doAct);

void HandleDWellStartEnd(const DocPos position, const UINT uid);
bool HandleHotSpotURLClicked(const DocPos position, const HYPERLINK_OPS operation);
void HandleColorDefClicked(HWND hwnd, const DocPos position);

bool    IsFindPatternEmpty();
LPCWSTR GetFindPattern();
void    GetFindPatternMB(LPSTR chPattern, int cch);
void    SetFindPattern(LPCWSTR wchFindPattern);
void    SetFindPatternMB(LPCSTR chFindPattern);
size_t  LengthOfFindPattern();
size_t  LengthOfFindPatternMB();

bool ConsistentIndentationCheck(EditFileIOStatus* status);

bool FileIO(bool fLoad, LPCWSTR pszFileName, EditFileIOStatus * status,
            bool bSkipUnicodeDetect, bool bSkipANSICPDetection, bool bForceEncDetection, bool bSetSavePoint,
            bool bSaveCopy, bool bPreserveTimeStamp);
bool FileLoad(LPCWSTR lpszFile, bool bDontSave, bool bNew, bool bReload,
              bool bSkipUnicodeDetect, bool bSkipANSICPDetection, bool bForceEncDetection);
bool FileRevert(LPCWSTR szFileName, bool);
bool FileSave(bool bSaveAlways, bool bAsk, bool bSaveAs, bool bSaveCopy, bool bPreserveTimeStamp);
bool OpenFileDlg(HWND hwnd,LPWSTR lpstrFile,int cchFile,LPCWSTR lpstrInitialDir);
bool SaveFileDlg(HWND hwnd,LPWSTR lpstrFile,int cchFile,LPCWSTR lpstrInitialDir);

void CreateBars(HWND hwnd, HINSTANCE hInstance);

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam);
LRESULT MsgCreate(HWND hwnd, WPARAM wParam, LPARAM lParam);
LRESULT MsgEndSession(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam);
LRESULT MsgThemeChanged(HWND hwnd, WPARAM wParam, LPARAM lParam);
LRESULT MsgDPIChanged(HWND hwnd, WPARAM wParam, LPARAM lParam);
LRESULT MsgSize(HWND hwnd, WPARAM wParam, LPARAM lParam);
LRESULT MsgDrawItem(HWND hwnd, WPARAM wParam, LPARAM lParam);
LRESULT MsgDropFiles(HWND hwnd, WPARAM wParam, LPARAM lParam);
LRESULT MsgCopyData(HWND hwnd, WPARAM wParam, LPARAM lParam);
LRESULT MsgContextMenu(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam);
LRESULT MsgEnterMenuLoop(HWND hwnd, WPARAM wParam);
LRESULT MsgInitMenu(HWND hwnd, WPARAM wParam, LPARAM lParam);
LRESULT MsgExitMenuLoop(HWND hwnd, WPARAM wParam);
LRESULT MsgNotify(HWND hwnd, WPARAM wParam, LPARAM lParam);
LRESULT MsgFileChangeNotify(HWND hwnd, WPARAM wParam, LPARAM lParam);
LRESULT MsgTrayMessage(HWND hwnd, WPARAM wParam, LPARAM lParam);
//~LRESULT MsgKeyDown(HWND hwnd, WPARAM wParam, LPARAM lParam);
LRESULT MsgCommand(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam);

LRESULT MsgSysCommand(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam);

// ----------------------------------------------------------------------------

void IgnoreNotifyDocChangedEvent(const bool bStealthMode);
void ObserveNotifyDocChangedEvent();

// ----------------------------------------------------------------------------

#define DocChangeTransactionBegin()  __try { SciCall_BeginUndoAction(); IgnoreNotifyDocChangedEvent(false);
#define EndDocChangeTransaction()    } __finally { ObserveNotifyDocChangedEvent(); SciCall_EndUndoAction(); }

// ----------------------------------------------------------------------------

#define UndoTransActionBegin()  { int const _token_ = BeginUndoAction(); __try { IgnoreNotifyDocChangedEvent(true);
#define EndUndoTransAction()    } __finally { ObserveNotifyDocChangedEvent(); EndUndoAction(_token_); } }

// ----------------------------------------------------------------------------

#define BeginWaitCursor(cond, text)           \
    __try {                                   \
        IgnoreNotifyDocChangedEvent(true);    \
        if (cond) {                           \
            SciCall_SetCursor(SC_CURSORWAIT); \
            StatusSetText(Globals.hwndStatus, STATUS_HELP, (text)); \
        }

#define BeginWaitCursorUID(cond, uid)         \
    __try {                                   \
        IgnoreNotifyDocChangedEvent(true);    \
        if (cond) {                           \
            SciCall_SetCursor(SC_CURSORWAIT); \
            StatusSetTextID(Globals.hwndStatus, STATUS_HELP, (uid)); \
        }

#define EndWaitCursor()                       \
    }                                         \
    __finally {                               \
        ObserveNotifyDocChangedEvent();       \
        SciCall_SetCursor(SC_CURSORNORMAL);   \
        POINT pt;                             \
        GetCursorPos(&pt);                    \
        SetCursorPos(pt.x, pt.y);             \
        UpdateStatusbar(true);                \
    }

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

#endif //_NP3_NOTEPAD3_H_
///   End of Notepad3.h   ///
