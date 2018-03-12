/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Notepad3.h                                                                  *
*   Global definitions and declarations                                       *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2016   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#pragma once
#ifndef _NP3_NOTEPAD3_H_
#define _NP3_NOTEPAD3_H_

#include "TypeDefs.h"

//==== Main Window ============================================================
#define WC_NOTEPAD3 L"Notepad3"

#define ONLINE_HELP_WEBSITE L"https://www.rizonesoft.com/documents/notepad3/"

//==== Data Type for WM_COPYDATA ==============================================
#define DATA_NOTEPAD3_PARAMS 0xFB10
typedef struct np3params {

  int   flagFileSpecified;
  int   flagChangeNotify;
  int   flagLexerSpecified;
  int   iInitialLexer;
  int   flagQuietCreate;
  int   flagJumpTo;
  int   iInitialLine;
  int   iInitialColumn;
  int   iSrcEncoding;
  int   flagSetEncoding;
  int   flagSetEOLMode;
  int   flagTitleExcerpt;
  WCHAR wchData;

} np3params, *LPnp3params;


typedef struct _undoSel
{
  int selMode_undo;
  DocPos anchorPos_undo;
  DocPos curPos_undo;
  int anchorVS_undo;
  int curVS_undo;

  int selMode_redo;
  DocPos anchorPos_redo;
  DocPos curPos_redo;
  int anchorVS_redo;
  int curVS_redo;
} 
UndoRedoSelection_t;

#define INIT_UNDOREDOSEL  { SC_SEL_STREAM, (DocPos)-1, (DocPos)-1, 0, 0, SC_SEL_STREAM, (DocPos)-1, (DocPos)-1, 0, 0 }


typedef enum {
  UNDO = TRUE,
  REDO = FALSE
} DoAction;


//==== Toolbar Style ==========================================================
#define WS_TOOLBAR (WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | \
                    TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | TBSTYLE_ALTDRAG | \
                    TBSTYLE_LIST | CCS_NODIVIDER | CCS_NOPARENTALIGN | \
                    CCS_ADJUSTABLE)


//==== ReBar Style ============================================================
#define WS_REBAR (WS_CHILD | WS_CLIPCHILDREN | WS_BORDER | RBS_VARHEIGHT | \
                  RBS_BANDBORDERS | CCS_NODIVIDER | CCS_NOPARENTALIGN)


//==== Ids ====================================================================
#define IDC_STATUSBAR    0xFB00
#define IDC_TOOLBAR      0xFB01
#define IDC_REBAR        0xFB02
#define IDC_EDIT         0xFB03
#define IDC_EDITFRAME    0xFB04
#define IDC_FILENAME     0xFB05
#define IDC_REUSELOCK    0xFB06


//==== Statusbar ==============================================================
#define STATUS_DOCPOS    0
#define STATUS_DOCSIZE   1
#define STATUS_CODEPAGE  2
#define STATUS_EOLMODE   3
#define STATUS_OVRMODE   4
#define STATUS_2ND_DEF   5
#define STATUS_LEXER     6
#define STATUS_HELP    255


//==== Change Notifications ===================================================
#define ID_WATCHTIMER 0xA000
#define WM_CHANGENOTIFY WM_USER+1
//#define WM_CHANGENOTIFYCLEAR WM_USER+2


//==== Callback Message from System Tray ======================================
#define WM_TRAYMESSAGE WM_USER


//==== Paste Board Timer ======================================================
#define ID_PASTEBOARDTIMER 0xA001


//==== Reuse Window Lock Timeout ==============================================
#define REUSEWINDOWLOCKTIMEOUT 1000


//==== Function Declarations ==================================================
BOOL InitApplication(HINSTANCE);
HWND InitInstance(HINSTANCE,LPSTR,int);
BOOL ActivatePrevInst();
BOOL RelaunchMultiInst();
BOOL RelaunchElevated(LPWSTR);
void SnapToDefaultPos(HWND);
void ShowNotifyIcon(HWND,BOOL);
void SetNotifyIconTitle(HWND);
void InstallFileWatching(LPCWSTR);
void CALLBACK WatchTimerProc(HWND,UINT,UINT_PTR,DWORD);
void CALLBACK PasteBoardTimer(HWND,UINT,UINT_PTR,DWORD);


void LoadSettings();
void SaveSettings(BOOL);
void ParseCommandLine();
void LoadFlags();
BOOL CheckIniFile(LPWSTR,LPCWSTR);
BOOL CheckIniFileRedirect(LPWSTR,LPCWSTR);
int  FindIniFile();
int  TestIniFile();
int  CreateIniFile();
int  CreateIniFileEx(LPCWSTR);


void MarkAllOccurrences(int);
void UpdateToolbar();
void UpdateStatusbar();
void UpdateLineNumberWidth();
void UpdateSettingsCmds();
void UpdateVisibleUrlHotspot(int);
void UpdateUI();


void InvalidateSelections();
int  BeginUndoAction();
void EndUndoAction(int);
void RestoreAction(int,DoAction);
int  UndoRedoActionMap(int,UndoRedoSelection_t*);
void OpenHotSpotURL(DocPos, BOOL);


BOOL FileIO(BOOL,LPCWSTR,BOOL,BOOL,int*,int*,BOOL*,BOOL*,BOOL*,BOOL*,BOOL);
BOOL FileLoad(BOOL,BOOL,BOOL,BOOL,BOOL,LPCWSTR);
BOOL FileRevert(LPCWSTR);
BOOL FileSave(BOOL,BOOL,BOOL,BOOL);
BOOL OpenFileDlg(HWND,LPWSTR,int,LPCWSTR);
BOOL SaveFileDlg(HWND,LPWSTR,int,LPCWSTR);


LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT MsgCreate(HWND, WPARAM, LPARAM);
void    MsgEndSession(HWND, UINT);
void    CreateBars(HWND, HINSTANCE);
void    MsgThemeChanged(HWND, WPARAM, LPARAM);
void    MsgSize(HWND, WPARAM, LPARAM);
void    MsgDropFiles(HWND, WPARAM, LPARAM);
LRESULT MsgCopyData(HWND, WPARAM, LPARAM);
LRESULT MsgContextMenu(HWND, UINT, WPARAM, LPARAM);
void    MsgInitMenu(HWND, WPARAM, LPARAM);
void    MsgChangeNotify(HWND, WPARAM, LPARAM);
LRESULT MsgTrayMessage(HWND, WPARAM, LPARAM);
LRESULT MsgSysCommand(HWND, UINT, WPARAM, LPARAM);
LRESULT MsgCommand(HWND, WPARAM, LPARAM);
LRESULT MsgNotify(HWND, WPARAM, LPARAM);

void IgnoreNotifyChangeEvent();
void ObserveNotifyChangeEvent();
BOOL CheckNotifyChangeEvent();

#endif //_NP3_NOTEPAD3_H_
///   End of Notepad3.h   \\\
