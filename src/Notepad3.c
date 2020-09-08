// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Notepad3.c                                                                  *
*   Main application window functionality                                     *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2020   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

#include "Helpers.h"

#include <crtdbg.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <commdlg.h>
#include <stdio.h>
#include <string.h>
//#include <pathcch.h>
//#include <locale.h>

#include "Edit.h"
#include "Styles.h"
#include "Dialogs.h"
#include "resource.h"
#include "../crypto/crypto.h"
#include "../uthash/utarray.h"
#include "../uthash/utlist.h"
#include "../tinyexpr/tinyexpr.h"
#include "Encoding.h"
#include "VersionEx.h"
#include "MuiLanguage.h"
#include "Notepad3.h"
#include "Config/Config.h"
#include "DarkMode/DarkMode.h"

#include "SciLexer.h"
#include "SciXLexer.h"

// ============================================================================
//
//   Local and global Variables for Notepad3.c
//
// ============================================================================

LPCWSTR WordBookMarks[MARKER_NP3_BOOKMARK] = {
  /*0*/ L"back:#0000", // OCC MARKER
  /*1*/ L"back:#FF0000",
  /*2*/ L"back:#0000FF",
  /*3*/ L"back:#00FF00",
  /*4*/ L"back:#FFFF00",
  /*5*/ L"back:#00E8E8",
  /*6*/ L"back:#FF00FF",
  /*7*/ L"back:#FF8F20",
  /*8*/ L"back:#950095"};

#define RELAUNCH_ELEVATED_BUF_ARG L"tmpfbuf="

CONSTANTS_T const Constants = { 
    2                                    // StdDefaultLexerID
  , L"minipath.exe"                      // FileBrowserMiniPath
  , L"grepWinNP3.exe"                    // FileSearchGrepWin
  , L"ThemeFileName"                     // StylingThemeName
  , L"Settings"                          // Inifile Section "Settings"
  , L"Settings2"                         // Inifile Section "Settings2"
  , L"Window"                            // Inifile Section "Window"
  , L"Styles"                            // Inifile Section "Styles"
  , L"Suppressed Messages"               // Inifile Section "SuppressedMessages"
};

FLAGS_T     Flags;
FLAGS_T     DefaultFlags;

GLOBALS_T   Globals;
SETTINGS_T  Settings;
SETTINGS_T  Defaults;
SETTINGS2_T Settings2;
SETTINGS2_T Defaults2;

FOCUSEDVIEW_T FocusedView;
FILEWATCHING_T FileWatching;

WININFO   g_IniWinInfo = INIT_WININFO;
WININFO   g_DefWinInfo = INIT_WININFO;

COLORREF  g_colorCustom[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

prefix_t  g_mxSBPrefix[STATUS_SECTOR_COUNT];
prefix_t  g_mxSBPostfix[STATUS_SECTOR_COUNT];

bool      g_iStatusbarVisible[STATUS_SECTOR_COUNT] = SBS_INIT_ZERO;
int       g_iStatusbarWidthSpec[STATUS_SECTOR_COUNT] = SBS_INIT_ZERO;
int       g_vSBSOrder[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

WCHAR     g_tchToolbarBitmap[MAX_PATH] = { L'\0' };
WCHAR     g_tchToolbarBitmapHot[MAX_PATH] = { L'\0' };
WCHAR     g_tchToolbarBitmapDisabled[MAX_PATH] = { L'\0' };

int       g_flagMatchText = 0;

// ------------------------------------
static bool      s_bIsProcessElevated = false;
static bool      s_bIsUserInAdminGroup = false;
static bool      s_bIsRunAsAdmin = false;
static bool      s_flagSaveOnRelaunch = false;
static bool      s_IsThisAnElevatedRelaunch = false;

static WCHAR     s_wchWndClass[64] = { L'\0' };

static HWND      s_hwndEditFrame = NULL;
static HWND      s_hwndNextCBChain = NULL;

static WCHAR     s_wchTmpFilePath[MAX_PATH] = { L'\0' };

static WCHAR     s_wchPrefixSelection[256] = { L'\0' };
static WCHAR     s_wchAppendSelection[256] = { L'\0' };
static WCHAR     s_wchPrefixLines[256] = { L'\0' };
static WCHAR     s_wchAppendLines[256] = { L'\0' };

static int       s_WinCurrentWidth = 0;

#define FILE_LIST_SIZE 32
static LPWSTR    s_lpFileList[FILE_LIST_SIZE] = { NULL };
static int       s_cFileList = 0;
static int       s_cchiFileList = 0;

static WCHAR     s_tchLastSaveCopyDir[MAX_PATH] = { L'\0' };

static bool      s_bRunningWatch = false;
static bool      s_bFileReadOnly = false;

static int       s_iSortOptions = 0;
static int       s_iAlignMode = 0;
static bool      s_bIsAppThemed = true;
static UINT      s_msgTaskbarCreated = 0;
static DWORD     s_dwChangeNotifyTime = 0;
static HANDLE    s_hChangeHandle = NULL;
static WCHAR     s_wchTitleExcerpt[MIDSZ_BUFFER] = { L'\0' };
static UINT      s_uidsAppTitle = IDS_MUI_APPTITLE;
static DWORD     s_dwLastCopyTime = 0;
static bool      s_bLastCopyFromMe = false;
static bool      s_bInMultiEditMode = false;
static bool      s_bCallTipEscDisabled = false;

static int       s_iInitialLine;
static int       s_iInitialColumn;
static int       s_iInitialLexer;

static int       s_cyReBar;
static int       s_cyReBarFrame;
static int       s_cxEditFrame;
static int       s_cyEditFrame;
static bool      s_bUndoRedoScroll = false;
static bool      s_bPrevFullScreenFlag = false;

// for tiny expression calculation
static double    s_dExpression = 0.0;
static te_xint_t s_iExprError  = -1;

static WIN32_FIND_DATA s_fdCurFile;

//~static CONST WCHAR *const s_ToolbarWndClassName = L"NP3_TOOLBAR_CLASS";

static int const INISECTIONBUFCNT = 32; // .ini file load buffer in KB

static TBBUTTON  s_tbbMainWnd[] = { 
  { 0,IDT_FILE_NEW,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
  { 1,IDT_FILE_OPEN,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
  { 3,IDT_FILE_SAVE,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
  { 2,IDT_FILE_BROWSE,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
  { 27,IDT_FILE_RECENT,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
  { 0,0,0,BTNS_SEP,{0},0,0 },
  { 4,IDT_EDIT_UNDO,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
  { 5,IDT_EDIT_REDO,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
  { 0,0,0,BTNS_SEP,{0},0,0 },
  { 6,IDT_EDIT_CUT,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
  { 7,IDT_EDIT_COPY,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
  { 8,IDT_EDIT_PASTE,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
  { 0,0,0,BTNS_SEP,{0},0,0 },
  { 9,IDT_EDIT_FIND,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
  { 10,IDT_EDIT_REPLACE,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
  { 0,0,0,BTNS_SEP,{0},0,0 },
  { 29,IDT_GREP_WIN_TOOL,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
  { 0,0,0,BTNS_SEP,{0},0,0 },
  { 11,IDT_VIEW_WORDWRAP,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
  { 0,0,0,BTNS_SEP,{0},0,0 },
  { 23,IDT_VIEW_TOGGLEFOLDS,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
  { 25,IDT_VIEW_TOGGLE_VIEW,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
  { 0,0,0,BTNS_SEP,{0},0,0 },
  { 21,IDT_FILE_OPENFAV,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
  { 22,IDT_FILE_ADDTOFAV,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
  { 0,0,0,BTNS_SEP,{0},0,0 },
  { 12,IDT_VIEW_ZOOMIN,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
  { 13,IDT_VIEW_ZOOMOUT,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
  { 0,0,0,BTNS_SEP,{0},0,0 },
  { 14,IDT_VIEW_SCHEME,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
  { 0,0,0,BTNS_SEP,{0},0,0 },
  { 24,IDT_FILE_LAUNCH,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
  { 0,0,0,BTNS_SEP,{0},0,0 },
  { 28,IDT_VIEW_PIN_ON_TOP,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
  { 0,0,0,BTNS_SEP,{0},0,0 },
  { 16,IDT_FILE_EXIT,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
  { 0,0,0,BTNS_SEP,{0},0,0 },
  { 15,IDT_VIEW_SCHEMECONFIG,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
  { 17,IDT_FILE_SAVEAS,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
  { 18,IDT_FILE_SAVECOPY,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
  { 19,IDT_EDIT_CLEAR,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
  { 20,IDT_FILE_PRINT,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
  { 26,IDT_VIEW_CHASING_DOCTAIL,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
};
static const int NUMTOOLBITMAPS = 30;

// ----------------------------------------------------------------------------

const WCHAR* const TBBUTTON_DEFAULT_IDS_V1 = L"1 2 4 3 28 0 5 6 0 7 8 9 0 10 11 0 30 0 12 0 24 26 0 22 23 0 13 14 0 27 0 15 0 25 0 17";
const WCHAR* const TBBUTTON_DEFAULT_IDS_V2 = L"1 2 4 3 28 0 5 6 0 7 8 9 0 10 11 0 30 0 12 0 24 26 0 22 23 0 13 14 0 15 0 25 0 29 0 17";

//=============================================================================
// static method declarations

// undo / redo  selections

static UT_icd UndoRedoSelElement_icd = { sizeof(DocPos), NULL, NULL, NULL };

static void InitUndoRedoSelection(void* elt)
{
  UndoRedoSelection_t* selection = (UndoRedoSelection_t*)elt;

  if (selection != NULL) {
    selection->selMode_undo = SC_SEL_STREAM;
    selection->anchorPos_undo = NULL;
    selection->curPos_undo = NULL;
    selection->anchorVS_undo = NULL;
    selection->curVS_undo = NULL;

    selection->selMode_redo = SC_SEL_STREAM;
    selection->anchorPos_redo = NULL;
    selection->curPos_redo = NULL;
    selection->anchorVS_redo = NULL;
    selection->curVS_redo = NULL;
  }
}


static void DelUndoRedoSelection(void* elt)
{
  UndoRedoSelection_t* selection = (UndoRedoSelection_t*)elt;

  if (selection != NULL) {
    if (selection->anchorPos_undo != NULL) {
      utarray_clear(selection->anchorPos_undo);
      utarray_free(selection->anchorPos_undo);
      selection->anchorPos_undo = NULL;
    }
    if (selection->curPos_undo != NULL) {
      utarray_clear(selection->curPos_undo);
      utarray_free(selection->curPos_undo);
      selection->curPos_undo = NULL;
    }
    if (selection->anchorVS_undo != NULL) {
      utarray_clear(selection->anchorVS_undo);
      utarray_free(selection->anchorVS_undo);
      selection->anchorVS_undo = NULL;
    }
    if (selection->curVS_undo != NULL) {
      utarray_clear(selection->curVS_undo);
      utarray_free(selection->curVS_undo);
      selection->curVS_undo = NULL;
    }

    if (selection->anchorPos_redo != NULL) {
      utarray_clear(selection->anchorPos_redo);
      utarray_free(selection->anchorPos_redo);
      selection->anchorPos_redo = NULL;
    }
    if (selection->curPos_redo != NULL) {
      utarray_clear(selection->curPos_redo);
      utarray_free(selection->curPos_redo);
      selection->curPos_redo = NULL;
    }
    if (selection->anchorVS_redo != NULL) {
      utarray_clear(selection->anchorVS_redo);
      utarray_free(selection->anchorVS_redo);
      selection->anchorVS_redo = NULL;
    }
    if (selection->curVS_redo != NULL) {
      utarray_clear(selection->curVS_redo);
      utarray_free(selection->curVS_redo);
      selection->curVS_redo = NULL;
    }
  }
}

static void CopyUndoRedoSelection(void* dst, const void* src)
{
  UndoRedoSelection_t* dst_sel = (UndoRedoSelection_t*)dst;
  const UndoRedoSelection_t* src_sel = (UndoRedoSelection_t*)src;

  DocPos* pPos = NULL;
  InitUndoRedoSelection(dst);

  dst_sel->selMode_undo = (src_sel) ? src_sel->selMode_undo : SC_SEL_STREAM;

  utarray_new(dst_sel->anchorPos_undo, &UndoRedoSelElement_icd);
  if (src_sel) {
    while ((pPos = (DocPos*)utarray_next(src_sel->anchorPos_undo, pPos)) != NULL) {
      utarray_push_back(dst_sel->anchorPos_undo, pPos);
    }
  }

  utarray_new(dst_sel->curPos_undo, &UndoRedoSelElement_icd);
  if (src_sel) {
    while ((pPos = (DocPos*)utarray_next(src_sel->curPos_undo, pPos)) != NULL) {
      utarray_push_back(dst_sel->curPos_undo, pPos);
    }
  }
  
  utarray_new(dst_sel->anchorVS_undo, &UndoRedoSelElement_icd);
  if (src_sel) {
    while ((pPos = (DocPos*)utarray_next(src_sel->anchorVS_undo, pPos)) != NULL) {
      utarray_push_back(dst_sel->anchorVS_undo, pPos);
    }
  }

  utarray_new(dst_sel->curVS_undo, &UndoRedoSelElement_icd);
  if (src_sel) {
    while ((pPos = (DocPos*)utarray_next(src_sel->curVS_undo, pPos)) != NULL) {
      utarray_push_back(dst_sel->curVS_undo, pPos);
    }
  }


  dst_sel->selMode_redo = (src_sel) ? src_sel->selMode_redo : SC_SEL_STREAM;

  utarray_new(dst_sel->anchorPos_redo, &UndoRedoSelElement_icd);
  if (src_sel) {
    while ((pPos = (DocPos*)utarray_next(src_sel->anchorPos_redo, pPos)) != NULL) {
      utarray_push_back(dst_sel->anchorPos_redo, pPos);
    }
  }

  utarray_new(dst_sel->curPos_redo, &UndoRedoSelElement_icd);
  if (src_sel) {
    while ((pPos = (DocPos*)utarray_next(src_sel->curPos_redo, pPos)) != NULL) {
      utarray_push_back(dst_sel->curPos_redo, pPos);
    }
  }

  utarray_new(dst_sel->anchorVS_redo, &UndoRedoSelElement_icd);
  if (src_sel) {
    while ((pPos = (DocPos*)utarray_next(src_sel->anchorVS_redo, pPos)) != NULL) {
      utarray_push_back(dst_sel->anchorVS_redo, pPos);
    }
  }

  utarray_new(dst_sel->curVS_redo, &UndoRedoSelElement_icd);
  if (src_sel) {
    while ((pPos = (DocPos*)utarray_next(src_sel->curVS_redo, pPos)) != NULL) {
      utarray_push_back(dst_sel->curVS_redo, pPos);
    }
  }
}

static UT_icd UndoRedoSelection_icd = { sizeof(UndoRedoSelection_t), InitUndoRedoSelection, CopyUndoRedoSelection, DelUndoRedoSelection };
static UT_array* UndoRedoSelectionUTArray = NULL;
static bool  _InUndoRedoTransaction();
static void  _SaveRedoSelection(int token);
static int   _SaveUndoSelection();
static int   _UndoRedoActionMap(int token, const UndoRedoSelection_t** selection);
static void  _SplitUndoTransaction(const int iModType);

// => _BEGIN_UNDO_ACTION_
// => _END_UNDO_ACTION_

// ----------------------------------------------------------------------------

static void  _DelayClearZoomCallTip(int delay);
static void  _DelaySplitUndoTransaction(int delay, int iModType);

//=============================================================================
//
//  IgnoreNotifyChangeEvent(), ObserveNotifyChangeEvent(), CheckNotifyChangeEvent()
//
static volatile LONG iNotifyChangeStackCounter = 0L;

static __forceinline bool CheckNotifyChangeEvent()
{
  return (InterlockedOr(&iNotifyChangeStackCounter, 0L) == 0L);
}

void IgnoreNotifyChangeEvent() 
{
  InterlockedIncrement(&iNotifyChangeStackCounter);
}

void ObserveNotifyChangeEvent() 
{
  if (!CheckNotifyChangeEvent()) {
    InterlockedDecrement(&iNotifyChangeStackCounter);
  }
  if (CheckNotifyChangeEvent()) {
    EditUpdateVisibleIndicators();
    //@@@ §§§ UpdateToolbar();
    UpdateStatusbar(false);
  }
}

// SCN_UPDATEUI notification
#define SC_UPDATE_NP3_INTERNAL_NOTIFY (SC_UPDATE_H_SCROLL << 1)


//=============================================================================
//
//  Delay Message Queue Handling  (TODO: MultiThreading)
//

static CmdMessageQueue_t* MessageQueue = NULL;

// ----------------------------------------------------------------------------

static int msgcmp(void* mqc1, void* mqc2)
{
  CmdMessageQueue_t* const pMQC1 = (CmdMessageQueue_t*)mqc1;
  CmdMessageQueue_t* const pMQC2 = (CmdMessageQueue_t*)mqc2;

  if ((pMQC1->cmd == pMQC2->cmd)
       //&& (pMQC1->hwnd == pMQC2->hwnd)
       && (pMQC1->wparam == pMQC2->wparam) // command
       //&& (pMQC1->lparam == pMQC2->lparam)
  ){
    return FALSE;
  }
  return 1;
}
// ----------------------------------------------------------------------------

#define _MQ_IMMEDIATE (2 * USER_TIMER_MINIMUM - 1)
#define _MQ_FAST (4 * USER_TIMER_MINIMUM)
#define _MQ_ms(T) ((T) / USER_TIMER_MINIMUM)

static void  _MQ_AppendCmd(CmdMessageQueue_t* const pMsgQCmd, int cycles)
{
  if (!pMsgQCmd) { return; }

  CmdMessageQueue_t* pmqc = NULL;
  DL_SEARCH(MessageQueue, pmqc, pMsgQCmd, msgcmp);

  if (!pmqc) { // NOT found
    pmqc = pMsgQCmd;
    pmqc->delay = cycles;
    DL_APPEND(MessageQueue, pmqc);
  }
  else {
    pmqc->delay = (pmqc->delay + cycles) / 2; // increase delay
  }
  if (pmqc->delay < 2) {
    // execute now (do not use PostMessage() here)
    SendMessage(pMsgQCmd->hwnd, pMsgQCmd->cmd, pMsgQCmd->wparam, pMsgQCmd->lparam);
    pmqc->delay = -1;
    pmqc->lparam = 0;
  }
}
// ----------------------------------------------------------------------------

/* UNUSED yet
static void _MQ_RemoveCmd(CmdMessageQueue_t* const pMsgQCmd)
{
  CmdMessageQueue_t* pmqc;

  DL_FOREACH(MessageQueue, pmqc)
  {
    if ((pMsgQCmd->hwnd == pmqc->hwnd)
      && (pMsgQCmd->cmd == pmqc->cmd)
      && (pMsgQCmd->wparam == pmqc->wparam))
    {
      pmqc->delay = -1;
    }
  }
}
*/
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
//
// called by Timer(IDT_TIMER_MRKALL)
//
static void CALLBACK MQ_ExecuteNext(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
  UNUSED(hwnd);    // must be main window handle
  UNUSED(uMsg);    // must be WM_TIMER
  UNUSED(idEvent); // must be IDT_TIMER_MRKALL
  UNUSED(dwTime);  // This is the value returned by the GetTickCount function

  CmdMessageQueue_t* pmqc;

  DL_FOREACH(MessageQueue, pmqc) 
  {
    if (pmqc->delay == 0) {
      SendMessage(pmqc->hwnd, pmqc->cmd, pmqc->wparam, pmqc->lparam);
      pmqc->delay = -1;
      pmqc->lparam = 0;
    }
    else if (pmqc->delay >= 0) {
      pmqc->delay -= 1;  // decrease
    }
  }
}


//=============================================================================
//
// InvalidateStyleRedraw
//
static inline void InvalidateStyleRedraw()
{
  SciCall_SetViewEOL(Settings.ViewEOLs);
}


//=============================================================================
//
// CommandLine Parsing Flags
//
static LPWSTR                s_lpSchemeArg = NULL;
static LPWSTR                s_lpOrigFileArg = NULL;
static WCHAR                 s_lpFileArg[MAX_PATH + 1] = { L'\0' };

static cpi_enc_t             s_flagSetEncoding = CPI_NONE;
static int                   s_flagSetEOLMode = 0;
static bool                  s_flagStartAsTrayIcon = false;
static int                   s_flagAlwaysOnTop = 0;
static bool                  s_flagKeepTitleExcerpt = false;
static bool                  s_flagNewFromClipboard = false;
static bool                  s_flagPasteBoard = false;
static bool                  s_flagJumpTo = false;
static FILE_WATCHING_MODE    s_flagChangeNotify = FWM_DONT_CARE;
static bool                  s_flagQuietCreate = false;
static bool                  s_flagLexerSpecified = false;
static bool                  s_flagAppIsClosing = false;
static bool                  s_flagDisplayHelp = false;

//==============================================================================

// static forward declarations 
static void  _UpdateStatusbarDelayed(bool bForceRedraw);
static void  _UpdateToolbarDelayed();

//==============================================================================
//
//  Save Needed Flag
//
//
static bool s_DocNeedSaving = false; // dirty-flag

bool GetDocModified()
{
  return (SciCall_GetModify() || s_DocNeedSaving);
}

static void SetSaveNeeded()
{
  if (!GetDocModified()) {
    if (IsWindow(Globals.hwndDlgFindReplace)) {
      PostWMCommand(Globals.hwndDlgFindReplace, IDC_DOC_MODIFIED);
    }
  }
  s_DocNeedSaving = true;
  UpdateToolbar();
  UpdateTitleBar();
}

void SetSavePoint()
{
  s_DocNeedSaving = false;
  if (SciCall_GetModify()) { SciCall_SetSavePoint(); }
  UpdateToolbar();
  UpdateTitleBar();
}

//==============================================================================


static void _InitGlobals()
{
  ZeroMemory(&Globals, sizeof(GLOBALS_T));
  ZeroMemory(&Defaults, sizeof(SETTINGS_T));
  ZeroMemory(&Settings, sizeof(SETTINGS_T));
  ZeroMemory(&Defaults2, sizeof(SETTINGS_T));
  ZeroMemory(&Settings2, sizeof(SETTINGS2_T));
  ZeroMemory(&Flags, sizeof(FLAGS_T));

  ZeroMemory(&(Globals.fvCurFile), sizeof(FILEVARS));

  Globals.WindowsBuildNumber = GetWindowsBuildNumber(NULL, NULL);

  Globals.hDlgIcon256   = NULL;
  Globals.hDlgIcon128   = NULL;
  Globals.hDlgIconBig   = NULL;
  Globals.hDlgIconSmall = NULL;
  Globals.hDlgIconPrefs256 = NULL;
  Globals.hDlgIconPrefs128 = NULL;
  Globals.hDlgIconPrefs64  = NULL;

  Globals.hMainMenu = NULL;
  Globals.pFileMRU = NULL;
  Globals.pMRUfind = NULL;
  Globals.pMRUreplace = NULL;
  Globals.iAvailLngCount = 1;
  Globals.iPrefLANGID = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
  Globals.iWrapCol = 80;
  Globals.CallTipType = CT_NONE;

  Globals.CmdLnFlag_PosParam = false;
  Globals.CmdLnFlag_WindowPos = 0;
  Globals.CmdLnFlag_ReuseWindow = 0;
  Globals.CmdLnFlag_SingleFileInstance = 0;
  Globals.CmdLnFlag_MultiFileArg = 0;
  Globals.CmdLnFlag_ShellUseSystemMRU = 0;
  Globals.CmdLnFlag_PrintFileAndLeave = 0;

  Globals.DOSEncoding = CPI_NONE;
  Globals.bZeroBasedColumnIndex = false;
  Globals.bZeroBasedCharacterCount = false;
  Globals.iReplacedOccurrences = 0;
  Globals.iMarkOccurrencesCount = 0;
  Globals.bUseLimitedAutoCCharSet = false;
  Globals.bIsCJKInputCodePage = false;
  Globals.bIniFileFromScratch = false;
  Globals.bFindReplCopySelOrClip = true;
  Globals.bReplaceInitialized = false;
  Globals.FindReplaceMatchFoundState = FND_NOP;
  Globals.bDocHasInconsistentEOLs = false;
  Globals.idxSelectedTheme = 1; // Default(0), Standard(1)
  Globals.InitialFontSize = (IsFullHD(NULL, -1, -1) < 0) ? 10 : 11;

  Flags.bLargeFileLoaded = DefaultFlags.bLargeFileLoaded = false;
  Flags.bDevDebugMode = DefaultFlags.bDevDebugMode = false;
  Flags.bStickyWindowPosition = DefaultFlags.bStickyWindowPosition = false;
  Flags.bReuseWindow = DefaultFlags.bReuseWindow = false;
  Flags.bSingleFileInstance = DefaultFlags.bSingleFileInstance = true;
  Flags.MultiFileArg = DefaultFlags.MultiFileArg = false;
  Flags.RelativeFileMRU = DefaultFlags.RelativeFileMRU = true;
  Flags.PortableMyDocs = DefaultFlags.PortableMyDocs = Flags.RelativeFileMRU;
  Flags.NoFadeHidden = DefaultFlags.NoFadeHidden = false;
  Flags.ToolbarLook = DefaultFlags.ToolbarLook = IsWindowsXPSP3OrGreater() ? 1 : 2;
  Flags.SimpleIndentGuides = DefaultFlags.SimpleIndentGuides = false;
  Flags.NoHTMLGuess =DefaultFlags.NoHTMLGuess = false;
  Flags.NoCGIGuess = DefaultFlags.NoCGIGuess = false;
  Flags.NoFileVariables = DefaultFlags.NoFileVariables = false;
  Flags.ShellUseSystemMRU = DefaultFlags.ShellUseSystemMRU = true;
  Flags.PrintFileAndLeave = DefaultFlags.PrintFileAndLeave = 0;
  Flags.bPreserveFileModTime = DefaultFlags.bPreserveFileModTime = false;
  Flags.bDoRelaunchElevated = DefaultFlags.bDoRelaunchElevated = false;
  Flags.bSearchPathIfRelative = DefaultFlags.bSearchPathIfRelative = false;

  Flags.bSettingsFileSoftLocked = DefaultFlags.bSettingsFileSoftLocked = false;

  FocusedView.HideNonMatchedLines = false;
  FocusedView.CodeFoldingAvailable = false;
  FocusedView.ShowCodeFolding = true;

  FileWatching.flagChangeNotify = FWM_DONT_CARE;
  FileWatching.FileWatchingMode = FWM_DONT_CARE;
  FileWatching.ResetFileWatching = true;
  FileWatching.MonitoringLog = false;
}


//=============================================================================
//
//  _InsertLanguageMenu()
//

//typedef struct _lng_menu_t {
//  LANGID LangID;
//  const WCHAR* MenuItem;
//} LNG_MENU_T;

static HMENU s_hmenuLanguage = NULL;

static bool _InsertLanguageMenu(HMENU hMenuBar)
{
  // check, if we need a language switching menu
  if (Globals.iAvailLngCount < 2) {
    return false;
  }

  if (s_hmenuLanguage) { DestroyMenu(s_hmenuLanguage); }
  s_hmenuLanguage = CreatePopupMenu();

  WCHAR wchMenuItemFmt[128] = { L'\0' };
  WCHAR wchMenuItemStrg[196] = { L'\0' };
  for (int lng = 0; lng < MuiLanguages_CountOf(); ++lng)
  {
    if (MUI_LanguageDLLs[lng].bHasDLL) 
    {
      StringCchCopy(wchMenuItemFmt, COUNTOF(wchMenuItemFmt), MUI_LanguageDLLs[lng].szMenuItem);
      StringCchPrintfW(wchMenuItemStrg, COUNTOF(wchMenuItemStrg), wchMenuItemFmt, MUI_LanguageDLLs[lng].szLocaleName);
      AppendMenu(s_hmenuLanguage, MF_ENABLED | MF_STRING, MUI_LanguageDLLs[lng].rid, wchMenuItemStrg);
    }
  }

  // --- insert ---
  int const pos = GetMenuItemCount(hMenuBar) - 1;
  if (pos >= 0) {
    GetLngString(IDS_MUI_MENU_LANGUAGE, wchMenuItemStrg, COUNTOF(wchMenuItemStrg));
    //return InsertMenu(hMenuBar, pos, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT_PTR)s_hmenuLanguage, wchMenuItemStrg);
    bool const res = InsertMenu(hMenuBar, IDM_VIEW_TABSASSPACES, MF_BYCOMMAND | MF_POPUP | MF_STRING, (UINT_PTR)s_hmenuLanguage, wchMenuItemStrg);
    InsertMenu(hMenuBar, IDM_VIEW_TABSASSPACES, MF_BYCOMMAND | MF_SEPARATOR, (UINT_PTR)NULL, NULL);
    return res;
  }
  return false;
}

//=============================================================================
//
//  _CleanUpResources()
//
static _invalid_parameter_handler _hOldInvalidParamHandler = NULL;

static void _CleanUpResources(const HWND hwnd, bool bIsInitialized)
{
  if (hwnd) {
    KillTimer(hwnd, IDT_TIMER_MRKALL);
  }

  CmdMessageQueue_t* pmqc = NULL;
  CmdMessageQueue_t* dummy;
  DL_FOREACH_SAFE(MessageQueue, pmqc, dummy)
  {
    DL_DELETE(MessageQueue, pmqc);
    //~FreeMem(pmqc); // No AllocMem Anymore
  }

  if (UndoRedoSelectionUTArray != NULL) {
    utarray_clear(UndoRedoSelectionUTArray);
    utarray_free(UndoRedoSelectionUTArray);
    UndoRedoSelectionUTArray = NULL;
  }

  // -------------------------------
  // Save Settings is done elsewhere
  // -------------------------------

  if (Globals.hMainMenu) { 
    DestroyMenu(Globals.hMainMenu); 
  }

  FreeLanguageResources();

  Scintilla_ReleaseResources();

  OleUninitialize();

  if (bIsInitialized) {
    //~UnregisterClass(s_ToolbarWndClassName, Globals.hInstance);
    UnregisterClass(s_wchWndClass, Globals.hInstance);
  }

  ReleaseDarkMode();

  if (s_lpOrigFileArg) {
    FreeMem(s_lpOrigFileArg);
    s_lpOrigFileArg = NULL;
  }

  if (_hOldInvalidParamHandler) {
    _set_invalid_parameter_handler(_hOldInvalidParamHandler);
  }
}



//=============================================================================
//
//  InvalidParameterHandler()
//
void InvalidParameterHandler(const wchar_t* expression,
                             const wchar_t* function,
                             const wchar_t* file,
                             unsigned int   line,
                             uintptr_t      pReserved)
{
  UNUSED(expression);
  UNUSED(pReserved);
  WCHAR msg[256];
  StringCchPrintf(msg, COUNTOF(msg), 
                  L"Invalid Parameter in function '%s()' - File:'%s' Line:%i !",
                  function, file, line);
  DbgMsgBoxLastError(msg, ERROR_INVALID_PARAMETER);
}



//=============================================================================
//
//  WinMain()
//
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
  _invalid_parameter_handler const hNewInvalidParamHandler = InvalidParameterHandler;
  _hOldInvalidParamHandler= _set_invalid_parameter_handler(hNewInvalidParamHandler);
  _CrtSetReportMode(_CRT_ASSERT, 0); // Disable the message box for assertions.

  _InitGlobals();

  // Set global variable Globals.hInstance
  Globals.hInstance = hInstance;
  Globals.hPrevInst = hPrevInstance;
  Globals.hndlProcessHeap = GetProcessHeap();

  WCHAR wchAppDir[MAX_PATH] = { L'\0' };
  PathGetAppDirectory(wchAppDir, COUNTOF(wchAppDir));

  if (!GetCurrentDirectory(COUNTOF(Globals.WorkingDirectory),Globals.WorkingDirectory)) {
    StringCchCopy(Globals.WorkingDirectory,COUNTOF(Globals.WorkingDirectory),wchAppDir);
  }

  // Don't keep working directory locked
  SetCurrentDirectory(wchAppDir);

  SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);

  // check if running at least on Windows 7 (SP1)
  if (!IsWindows7SP1OrGreater()) {
    MsgBoxLastError(L"Application Initialization", ERROR_OLD_WIN_VERSION);
    return 1; // exit
  }

  // Check if running with elevated privileges
  s_bIsProcessElevated = IsProcessElevated();
  s_bIsUserInAdminGroup = IsUserInAdminGroup();
  s_bIsRunAsAdmin = IsRunAsAdmin();

  // Default Encodings (may already be used for command line parsing)
  Encoding_InitDefaults();

  // Command Line, Ini File and Flags
  ParseCommandLine();
  FindIniFile();
  TestIniFile();
  DWORD dwFileSize = 0UL;
  Globals.bCanSaveIniFile = CreateIniFile(Globals.IniFile, &dwFileSize);
  Globals.bIniFileFromScratch = (dwFileSize == 0UL);
  if (Globals.bIniFileFromScratch && Globals.bCanSaveIniFile) {
    // Set at least Application Name Section
    IniFileSetString(Globals.IniFile, _W(SAPPNAME), NULL, NULL);
  }
  LoadSettings();

  InitDarkMode(true); // try

  // set AppUserModelID
  PrivateSetCurrentProcessExplicitAppUserModelID(Settings2.AppUserModelID);

  // Adapt window class name
  StringCchCat(s_wchWndClass, COUNTOF(s_wchWndClass), _W(SAPPNAME));
  if (s_bIsProcessElevated) {
    StringCchCat(s_wchWndClass, COUNTOF(s_wchWndClass), L"U");
  }
  if (s_flagPasteBoard) {
    StringCchCat(s_wchWndClass, COUNTOF(s_wchWndClass), L"B");
  }
  
  (void)OleInitialize(NULL);
  
  INITCOMMONCONTROLSEX icex;
  ZeroMemory(&icex, sizeof(INITCOMMONCONTROLSEX));
  icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
  icex.dwICC = ICC_WIN95_CLASSES | ICC_USEREX_CLASSES | ICC_COOL_CLASSES | ICC_NATIVEFNTCTL_CLASS | ICC_STANDARD_CLASSES;
  InitCommonControlsEx(&icex);

  Scintilla_RegisterClasses(hInstance);

  //SetProcessDPIAware(); -> .manifest
  //SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
  Scintilla_LoadDpiForWindow();

  // ----------------------------------------------------
  // MultiLingual
  //
  SetPreferredLanguage(LoadLanguageResources());

  // ----------------------------------------------------

  // ICON_BIG
  int const cxb = GetSystemMetrics(SM_CXICON) << 2;
  int const cyb = GetSystemMetrics(SM_CYICON) << 2;
  // ICON_SMALL
  int const cxs = GetSystemMetrics(SM_CXSMICON) << 1;
  int const cys = GetSystemMetrics(SM_CYSMICON) << 1;

  //UINT const fuLoad = LR_DEFAULTCOLOR | LR_SHARED;

  if (!Globals.hDlgIcon256) {
    LoadIconWithScaleDown(hInstance, MAKEINTRESOURCE(IDR_MAINWND), 256, 256, &(Globals.hDlgIcon256));
  }
  if (!Globals.hDlgIcon128) {
    LoadIconWithScaleDown(hInstance, MAKEINTRESOURCE(IDR_MAINWND), 128, 128, &(Globals.hDlgIcon128));
  }
  if (!Globals.hDlgIconBig) {
    LoadIconWithScaleDown(hInstance, MAKEINTRESOURCE(IDR_MAINWND), cxb, cyb, &(Globals.hDlgIconBig));
  }
  if (!Globals.hDlgIconSmall) {
    LoadIconWithScaleDown(hInstance, MAKEINTRESOURCE(IDR_MAINWND), cxs, cys, &(Globals.hDlgIconSmall));
  }

  if (!Globals.hDlgIconPrefs256) {
    LoadIconWithScaleDown(hInstance, MAKEINTRESOURCE(IDI_MUI_STYLES), 256, 256, &(Globals.hDlgIconPrefs256));
  }
  if (!Globals.hDlgIconPrefs128) {
    LoadIconWithScaleDown(hInstance, MAKEINTRESOURCE(IDI_MUI_STYLES), 128, 128, &(Globals.hDlgIconPrefs128));
  }
  if (!Globals.hDlgIconPrefs64) {
    LoadIconWithScaleDown(hInstance, MAKEINTRESOURCE(IDI_MUI_STYLES), 64, 64, &(Globals.hDlgIconPrefs64));
  }

  if (!Globals.hIconMsgUser) {
    LoadIconWithScaleDown(hInstance, MAKEINTRESOURCE(IDR_MAINWND), cxb, cyb, &(Globals.hIconMsgUser));
  }
  if (!Globals.hIconMsgInfo) {
    LoadIconWithScaleDown(NULL, IDI_INFORMATION, cxb, cyb, &(Globals.hIconMsgInfo));
  }
  if (!Globals.hIconMsgWarn) {
    LoadIconWithScaleDown(NULL, IDI_WARNING, cxb, cyb, &(Globals.hIconMsgWarn));
  }
  if (!Globals.hIconMsgError) {
    LoadIconWithScaleDown(NULL, IDI_ERROR, cxb, cyb, &(Globals.hIconMsgError));
  }
  if (!Globals.hIconMsgQuest) {
    LoadIconWithScaleDown(NULL, IDI_QUESTION, cxb, cyb, &(Globals.hIconMsgQuest));
  }
  if (!Globals.hIconMsgShield) {
    LoadIconWithScaleDown(NULL, IDI_SHIELD, cxb, cyb, &(Globals.hIconMsgShield));
  }
  //if (!Globals.hIconMsgWinLogo) {
  //  LoadIconWithScaleDown(NULL, IDI_WINLOGO, cxl, cyl, &(Globals.hIconMsgWinLogo));
  //}

  if (s_IsThisAnElevatedRelaunch && !IsRunAsAdmin()) {
    InfoBoxLng(MB_ICONSHIELD, NULL, IDS_MUI_ERR_ELEVATED_RIGHTS);
    s_flagSaveOnRelaunch = false;
  }

  // Try to Relaunch with elevated privileges
  if (RelaunchElevated(NULL)) {
    return FALSE;
  }

  // Try to run multiple instances
  if (RelaunchMultiInst()) {
    return FALSE;
  }
  // Try to activate another window
  if (ActivatePrevInst()) {
    return FALSE;
  }

  // Command Line Help Dialog
  if (s_flagDisplayHelp) {
    DisplayCmdLineHelp(NULL);
    _CleanUpResources(NULL, false);
    return FALSE;
  }

  s_msgTaskbarCreated = RegisterWindowMessage(L"TaskbarCreated");

  Globals.hMainMenu = LoadMenu(Globals.hLngResContainer, MAKEINTRESOURCE(IDR_MUI_MAINMENU));
  if (!Globals.hMainMenu) {
    MsgBoxLastError(L"LoadMenu()", 0);
    _CleanUpResources(NULL, false);
    return 1;
  }
  
  _InsertLanguageMenu(Globals.hMainMenu);
  Style_InsertThemesMenu(Globals.hMainMenu);

  if (!InitApplication(Globals.hInstance)) 
  {
    _CleanUpResources(NULL, false);
    return 1; 
  }

  InitDarkMode(IsDarkModeSupported() && Settings.WinThemeDarkMode); // settings

  HWND const hwnd = InitInstance(Globals.hInstance, lpCmdLine, nShowCmd);
  if (!hwnd) { 
    _CleanUpResources(hwnd, true);
    return 1; 
  }
  DrawMenuBar(hwnd);

  HACCEL const hAccMain = LoadAccelerators(hInstance,MAKEINTRESOURCE(IDR_MAINWND));
  HACCEL const hAccFindReplace = LoadAccelerators(hInstance,MAKEINTRESOURCE(IDR_ACCFINDREPLACE));
  HACCEL const hAccCoustomizeSchemes = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCCUSTOMSCHEMES));
 
  SetTimer(hwnd, IDT_TIMER_MRKALL, USER_TIMER_MINIMUM, (TIMERPROC)MQ_ExecuteNext);
  
  if (Globals.bPrefLngNotAvail) {
    const WCHAR* const suprMsg = L"MsgPrefLanguageNotAvailable";
    InfoBoxLng(MB_ICONWARNING, suprMsg, IDS_WARN_PREF_LNG_NOT_AVAIL, Settings2.PreferredLanguageLocaleName);
    int const noMsg = IniFileGetInt(Globals.IniFile, Constants.SectionSuppressedMessages, suprMsg, 0);
    if (noMsg && Globals.bCanSaveIniFile) {
      IniFileSetString(Globals.IniFile, Constants.Settings2_Section, L"PreferredLanguageLocaleName", MUI_LanguageDLLs[0].szLocaleName);
    }
  }

  MSG msg;
  while (GetMessage(&msg,NULL,0,0))
  {
    if (IsWindow(Globals.hwndDlgFindReplace) && ((msg.hwnd == Globals.hwndDlgFindReplace) || IsChild(Globals.hwndDlgFindReplace, msg.hwnd))) 
    {
      const int iTr = TranslateAccelerator(Globals.hwndDlgFindReplace, hAccFindReplace, &msg);
      if (iTr || IsDialogMessage(Globals.hwndDlgFindReplace, &msg))
        continue;
    }
    if (IsWindow(Globals.hwndDlgCustomizeSchemes) && ((msg.hwnd == Globals.hwndDlgCustomizeSchemes) || IsChild(Globals.hwndDlgCustomizeSchemes, msg.hwnd))) {
      const int iTr = TranslateAccelerator(Globals.hwndDlgCustomizeSchemes, hAccCoustomizeSchemes, &msg);
      if (iTr || IsDialogMessage(Globals.hwndDlgCustomizeSchemes, &msg))
        continue;
    }
    if (!TranslateAccelerator(hwnd, hAccMain, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  _CleanUpResources(hwnd, true);

  return (int)(msg.wParam);
}

//=============================================================================
//
//  GetFactoryDefaultWndPos()
//
//
WININFO GetFactoryDefaultWndPos(const int flagsPos)
{
  RECT rc;
  GetWindowRect(GetDesktopWindow(), &rc);
  MONITORINFO mi;
  GetMonitorInfoFromRect(&rc, &mi);
  WININFO winfo = INIT_WININFO;
  winfo.y = mi.rcMonitor.top;
  winfo.cy = mi.rcWork.bottom - mi.rcWork.top;
  winfo.cx = (mi.rcWork.right - mi.rcWork.left) / 2;
  winfo.x = (flagsPos == 3) ? mi.rcMonitor.left : winfo.cx;
  winfo.max = 0;
  winfo.zoom = 100;
  return winfo;
}
// ----------------------------------------------------------------------------


//=============================================================================
//
//  GetWinInfoByFlag()
//
//
WININFO GetWinInfoByFlag(const int flagsPos)
{
  WININFO winfo = INIT_WININFO;

  if (flagsPos < 0) {
    winfo = GetMyWindowPlacement(Globals.hwndMain, NULL); // current window position
  }
  else if (flagsPos == 0) {
    winfo = g_IniWinInfo; // initial window position
  }
  else if (flagsPos == 1) {
    winfo.x = winfo.y = winfo.cx = winfo.cy = CW_USEDEFAULT;
    winfo.max = false;
    winfo.zoom = 100;
  }
  else if (flagsPos == 2)
  {
    winfo = g_DefWinInfo; // NP3 default window position
  }
  else if (flagsPos == 3)
  {
    winfo = GetFactoryDefaultWndPos(flagsPos);
  }
  else if ((flagsPos >= 4) && (flagsPos < 256))
  {
    RECT rc;
    GetWindowRect(GetDesktopWindow(), &rc);
    MONITORINFO mi;
    GetMonitorInfoFromRect(&rc, &mi);

    int const width = (mi.rcWork.right - mi.rcWork.left);
    int const height = (mi.rcWork.bottom - mi.rcWork.top);

    if (flagsPos & 8)
      winfo.x = mi.rcMonitor.left + (width >> 1);
    else
      winfo.x = mi.rcMonitor.left;

    if (flagsPos & (4 | 8))
      winfo.cx = (width >> 1);
    else 
      winfo.cx = width;

    if (flagsPos & 32)
      winfo.y = mi.rcMonitor.top + (height >> 1);
    else
      winfo.y = mi.rcMonitor.top;

    if (flagsPos & (16 | 32))
      winfo.cy = (height >> 1);
    else
      winfo.cy = height;

    if (flagsPos & 64) {
      winfo.x = mi.rcMonitor.left;
      winfo.y = mi.rcMonitor.top;
      winfo.cx = width;
      winfo.cy = height;
    }
    if (flagsPos & 128) {
      winfo = g_DefWinInfo;
      winfo.max = true;
      winfo.zoom = 100;
    }
  }
  else { // ( > 256) restore window, move upper left corner to Work Area 
    
    MONITORINFO mi;
    RECT const rc = RectFromWinInfo(&winfo);
    GetMonitorInfoFromRect(&rc, &mi);
    WININFO wi = winfo; wi.cx = wi.cy = 16; // really small
    FitIntoMonitorGeometry(&(mi.rcWork), &wi, SCR_NORMAL);
    winfo.x = wi.x;
    winfo.y = wi.y;
  }

  return winfo;
}



//=============================================================================
//
//  Set/Get FindPattern()
// 
static WCHAR sCurrentFindPattern[FNDRPL_BUFFER] = { L'\0' };

bool IsFindPatternEmpty()
{
  return  StrIsEmpty(sCurrentFindPattern);
}

//=============================================================================
//
//  SetFindPattern()
// 
void SetFindPattern(LPCWSTR wchFindPattern)
{
  StringCchCopy(sCurrentFindPattern, COUNTOF(sCurrentFindPattern), (wchFindPattern ? wchFindPattern : L""));
}

//=============================================================================
//
//  SetFindPatternMB()
// 
void SetFindPatternMB(LPCSTR chFindPattern)
{
  MultiByteToWideCharEx(Encoding_SciCP, 0, chFindPattern, -1, sCurrentFindPattern, COUNTOF(sCurrentFindPattern));
}


//=============================================================================
//
//  LengthOfFindPattern()
// 
size_t LengthOfFindPattern()
{
  return StringCchLen(sCurrentFindPattern, 0);
}


//=============================================================================
//
//  GetFindPattern()
// 
LPCWSTR GetFindPattern()
{
  return sCurrentFindPattern;
}


//=============================================================================
//
//  CopyFindPattern()
// 
void CopyFindPattern(LPWSTR wchFindPattern, size_t bufferCount)
{
  StringCchCopy(wchFindPattern, bufferCount, sCurrentFindPattern);
}

//=============================================================================
//
//  CopyFindPatternMB()
// 
void CopyFindPatternMB(LPSTR chFindPattern, size_t bufferCount)
{
  WideCharToMultiByte(Encoding_SciCP, 0, sCurrentFindPattern, -1, chFindPattern, (int)bufferCount, NULL, NULL);
}


static EDITFINDREPLACE s_FindReplaceData = INIT_EFR_DATA;

//=============================================================================
//
// SetFindReplaceData()
//
static void SetFindReplaceData()
{
  s_FindReplaceData = Settings.EFR_Data; // reset

  if (!IsFindPatternEmpty()) {
    CopyFindPatternMB(s_FindReplaceData.szFind, COUNTOF(s_FindReplaceData.szFind));
    CopyFindPatternMB(Settings.EFR_Data.szFind, COUNTOF(Settings.EFR_Data.szFind));
  }

  if (g_flagMatchText) // cmd line
  {
    if (g_flagMatchText & 4) {
      s_FindReplaceData.fuFlags = (SCFIND_REGEXP | SCFIND_POSIX);
    }
    if (g_flagMatchText & 8) {
      s_FindReplaceData.fuFlags |= SCFIND_MATCHCASE;
    }
    if (g_flagMatchText & 16) {
      s_FindReplaceData.fuFlags |= SCFIND_DOT_MATCH_ALL;
    }
    if (g_flagMatchText & 32) {
      s_FindReplaceData.bTransformBS = true;
    }
    s_FindReplaceData.bOverlappingFind = false;
    s_FindReplaceData.bWildcardSearch = false;
    s_FindReplaceData.bReplaceClose = false;
  }
}


//=============================================================================
//
// SetCurrentSelAsFindReplaceData()
//
static bool SetCurrentSelAsFindReplaceData()
{
  if (SciCall_IsSelectionEmpty()) {
    EditSelectWordAtPos(SciCall_GetCurrentPos(), true);
  }

  size_t const cchSelection = SciCall_GetSelText(NULL);

  if (1 < cchSelection) {
    char* szSelection = AllocMem(cchSelection, HEAP_ZERO_MEMORY);
    if (szSelection) {
      SciCall_GetSelText(szSelection);
      SetFindPatternMB(szSelection);
      SetFindReplaceData(); // s_FindReplaceData
      FreeMem(szSelection);
      return true;
    }
  }
  return false;
}

//=============================================================================
//
// InitApplication()
//
//
bool InitApplication(const HINSTANCE hInstance)
{
  WNDCLASSEX wc;
  ZeroMemory(&wc, sizeof(WNDCLASSEX));
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.style = CS_BYTEALIGNWINDOW | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = (WNDPROC)MainWndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hInstance;
  wc.hIcon = Globals.hDlgIcon256;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = UseDarkMode() ? g_hbrWndDarkBkgBrush : (HBRUSH)(COLOR_WINDOW + 1);
  wc.lpszMenuName = MAKEINTRESOURCE(IDR_MUI_MAINMENU);
  wc.lpszClassName = s_wchWndClass;

  return RegisterClassEx(&wc);
}


#if 0
//=============================================================================
//
// InitToolbarWndClass()
//
bool InitWndClass(const HINSTANCE hInstance, LPCWSTR lpszWndClassName, LPCWSTR lpszCopyFromWC, bool bUnregisterFirst) {
  WNDCLASSEX wcx;
  if (bUnregisterFirst) {
    UnregisterClass(lpszWndClassName, hInstance);
  }
  ZeroMemory(&wcx, sizeof(WNDCLASSEX));
  wcx.cbSize = sizeof(WNDCLASSEX);

  GetClassInfoEx(hInstance, lpszCopyFromWC, &wcx); // copy members

  //wcx.lpfnWndProc = (WNDPROC)TBWndProc; ~ don't do that
  wcx.hInstance = hInstance; // done already
  wcx.hCursor = LoadCursor(NULL, IDC_HAND); 
  wcx.hbrBackground = UseDarkMode() ? g_hbrWndDarkBkgBrush : (HBRUSH)(COLOR_WINDOW + 1);
  wcx.lpszClassName = lpszWndClassName;

  return RegisterClassEx(&wcx);
}
#endif


//=============================================================================
//
//  InitInstance()
//
HWND InitInstance(const HINSTANCE hInstance, LPCWSTR pszCmdLine, int nCmdShow)
{
  UNUSED(pszCmdLine);
 
  g_IniWinInfo = GetWinInfoByFlag(Globals.CmdLnFlag_WindowPos);
  s_WinCurrentWidth = g_IniWinInfo.cx;

  // get monitor coordinates from g_IniWinInfo
  WININFO srcninfo = g_IniWinInfo;
  WinInfoToScreen(&srcninfo);

  Globals.hwndMain = CreateWindowEx(
               0,
               s_wchWndClass,
               _W(SAPPNAME),
               WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
               srcninfo.x,
               srcninfo.y,
               srcninfo.cx,
               srcninfo.cy,
               NULL,
               NULL,
               hInstance,
               NULL);

  if (g_IniWinInfo.max) {
    nCmdShow = SW_SHOWMAXIMIZED;
  }

  if ((Settings.AlwaysOnTop || s_flagAlwaysOnTop == 2) && s_flagAlwaysOnTop != 1) {
    SetWindowPos(Globals.hwndMain, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
  }

  SetDialogIconNP3(Globals.hwndMain);

  InitWindowCommon(Globals.hwndMain, true);

  if (Settings.TransparentMode) {
    SetWindowTransparentMode(Globals.hwndMain, true, Settings2.OpacityLevel);
  }
  
  SetMenu(Globals.hwndMain, Globals.hMainMenu);
  SetMenu(Globals.hwndMain, (Settings.ShowMenubar ? Globals.hMainMenu : NULL));
  DrawMenuBar(Globals.hwndMain);

  // Current file information -- moved in front of ShowWindow()
  FileLoad(true,true,false,Settings.SkipUnicodeDetection,Settings.SkipANSICodePageDetection,false,L"");

  if (!s_flagStartAsTrayIcon) {
    ShowWindow(Globals.hwndMain,nCmdShow);
    UpdateWindow(Globals.hwndMain);
  }
  else {
    ShowWindow(Globals.hwndMain,SW_HIDE);    // trick ShowWindow()
    ShowNotifyIcon(Globals.hwndMain,true);
  }

  // Source Encoding
  Encoding_Forced(s_flagSetEncoding);

  // Pathname parameter
  if (s_IsThisAnElevatedRelaunch || (StrIsNotEmpty(s_lpFileArg) /*&& !g_flagNewFromClipboard*/))
  {
    bool bOpened = false;

    // Open from Directory
    if (!s_IsThisAnElevatedRelaunch && PathIsDirectory(s_lpFileArg)) {
      WCHAR tchFile[MAX_PATH] = { L'\0' };
      if (OpenFileDlg(Globals.hwndMain, tchFile, COUNTOF(tchFile), s_lpFileArg))
        bOpened = FileLoad(false, false, false, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, false, tchFile);
    }
    else {
      LPCWSTR lpFileToOpen = s_IsThisAnElevatedRelaunch ? s_wchTmpFilePath : s_lpFileArg;
      bOpened = FileLoad(false, false, false, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, false, lpFileToOpen);
      if (bOpened) {
        if (s_IsThisAnElevatedRelaunch)
        {
          if (StrIsNotEmpty(s_lpFileArg)) {
            StringCchCopy(Globals.CurrentFile, COUNTOF(Globals.CurrentFile), s_lpFileArg);
            InstallFileWatching(Globals.CurrentFile);
          }
          else {
            StringCchCopy(Globals.CurrentFile, COUNTOF(Globals.CurrentFile), L"");
          }
          if (!s_flagLexerSpecified) {
            Style_SetLexerFromFile(Globals.hwndEdit, Globals.CurrentFile);
          }

          // check for temp file and delete
          if (s_IsThisAnElevatedRelaunch && PathIsExistingFile(s_wchTmpFilePath))
          {
            DeleteFile(s_wchTmpFilePath);
            // delete possible .tmp guard
            size_t const len = StringCchLen(s_wchTmpFilePath, MAX_PATH);
            LPWSTR p = PathFindExtension(s_wchTmpFilePath);
            if (p && *p) {
              StringCchCopy(p, (MAX_PATH - len), L".tmp");
            }
            if (PathIsExistingFile(s_wchTmpFilePath)) {
              DeleteFile(s_wchTmpFilePath);
            }
          }

          UndoRedoReset();
          SetSaveNeeded();

          if (StrIsNotEmpty(Globals.CurrentFile)) {
            if (s_flagSaveOnRelaunch) {
              FileSave(true, false, false, false, Flags.bPreserveFileModTime); // issued from elevation instances
            }
          }
        }
        if (s_flagJumpTo) { // Jump to position
          EditJumpTo(s_iInitialLine,s_iInitialColumn);
        }
      }
    }

    s_lpFileArg[0] = L'\0';

    if (bOpened) {
      switch (s_flagChangeNotify) {
        case FWM_MSGBOX:
          FileWatching.FileWatchingMode = FWM_DONT_CARE;
          FileWatching.ResetFileWatching = true;
          InstallFileWatching(Globals.CurrentFile);
          break;
        case FWM_AUTORELOAD:
          if (!FileWatching.MonitoringLog) {
            PostWMCommand(Globals.hwndMain, IDM_VIEW_CHASING_DOCTAIL);
          }
          else {
            FileWatching.FileWatchingMode = FWM_AUTORELOAD;
            FileWatching.ResetFileWatching = true;
            InstallFileWatching(Globals.CurrentFile);
          }
          break;
        case FWM_DONT_CARE:
        default:
          break;
      }
    }
  }
  else {
    cpi_enc_t const forcedEncoding = Encoding_Forced(CPI_GET);
    if (Encoding_IsValid(forcedEncoding)) {
      Encoding_Current(forcedEncoding);
    }
  }

  // reset
  Encoding_Forced(CPI_NONE);
  s_flagQuietCreate = false;
  s_flagKeepTitleExcerpt = false;

  // undo / redo selections
  if (UndoRedoSelectionUTArray != NULL) {
    utarray_clear(UndoRedoSelectionUTArray);
    utarray_free(UndoRedoSelectionUTArray);
    UndoRedoSelectionUTArray = NULL;
  }
  utarray_new(UndoRedoSelectionUTArray, &UndoRedoSelection_icd);
  utarray_reserve(UndoRedoSelectionUTArray,256);

  // Check for /c [if no file is specified] -- even if a file is specified
  /*else */if (s_flagNewFromClipboard) {
    if (SciCall_CanPaste()) {
      bool bAutoIndent2 = Settings.AutoIndent;
      Settings.AutoIndent = 0;
      EditJumpTo(-1, 0);
      _BEGIN_UNDO_ACTION_;
      if (!Sci_IsDocEmpty()) {
        SciCall_NewLine();
      }
      SciCall_Paste();
      SciCall_NewLine();
      _END_UNDO_ACTION_;
      Settings.AutoIndent = bAutoIndent2;
      if (s_flagJumpTo)
        EditJumpTo(s_iInitialLine, s_iInitialColumn);
      else
        EditEnsureSelectionVisible();
    }
  }

  // Encoding
  if (s_flagSetEncoding != CPI_NONE) {
    SendMessage(Globals.hwndMain, WM_COMMAND, MAKELONG(IDM_ENCODING_SELECT, IDM_ENCODING_SELECT + s_flagSetEncoding), 0);
    s_flagSetEncoding = CPI_NONE;
  }

  // EOL mode
  if (s_flagSetEOLMode != 0) {
    SendWMCommand(Globals.hwndMain, IDM_LINEENDINGS_CRLF + s_flagSetEOLMode - 1);
    s_flagSetEOLMode = 0;
  }

  // Match Text
  if (g_flagMatchText && !IsFindPatternEmpty()) 
  {
    if (!Sci_IsDocEmpty()) {

      SetFindReplaceData(); // s_FindReplaceData

      if (g_flagMatchText & 2) {
        if (!s_flagJumpTo) { SciCall_DocumentEnd(); }
        EditFindPrev(Globals.hwndEdit,&s_FindReplaceData,false,false);
      }
      else {
        if (!s_flagJumpTo) { SciCall_DocumentStart(); }
        EditFindNext(Globals.hwndEdit,&s_FindReplaceData,false,false);
      }
    }
  }

  // Check for Paste Board option -- after loading files
  if (s_flagPasteBoard) {
    s_bLastCopyFromMe = true;
    s_hwndNextCBChain = SetClipboardViewer(Globals.hwndMain);
    s_uidsAppTitle = IDS_MUI_APPTITLE_PASTEBOARD;
    s_bLastCopyFromMe = false;

    s_dwLastCopyTime = 0;
    SetTimer(Globals.hwndMain,ID_PASTEBOARDTIMER,100,PasteBoardTimer);
  }

  // check if a lexer was specified from the command line
  if (s_flagLexerSpecified) {
    if (s_lpSchemeArg) {
      Style_SetLexerFromName(Globals.hwndEdit,Globals.CurrentFile,s_lpSchemeArg);
      LocalFree(s_lpSchemeArg);  // StrDup()
    }
    else if ((s_iInitialLexer >= 0) && (s_iInitialLexer < NUMLEXERS)) {
      Style_SetLexerFromID(Globals.hwndEdit, s_iInitialLexer);
    }
    s_flagLexerSpecified = false;
  }

  // If start as tray icon, set current filename as tooltip
  if (s_flagStartAsTrayIcon) {
    SetNotifyIconTitle(Globals.hwndMain);
  }
  Globals.iReplacedOccurrences = 0;
  Globals.iMarkOccurrencesCount = 0;

  UpdateToolbar();
  UpdateStatusbar(true);
  UpdateMarginWidth();
  UpdateMouseDWellTime();

  // print file immediately and quit
  if (Globals.CmdLnFlag_PrintFileAndLeave)
  {
    WCHAR *pszTitle;
    WCHAR tchUntitled[32] = { L'\0' };
    WCHAR tchPageFmt[32] = { L'\0' };
    WCHAR szDisplayName[MAX_PATH];

    if (StrIsNotEmpty(Globals.CurrentFile)) {
      PathGetDisplayName(szDisplayName, COUNTOF(szDisplayName), Globals.CurrentFile);
      pszTitle = szDisplayName;
    }
    else {
      GetLngString(IDS_MUI_UNTITLED, tchUntitled, COUNTOF(tchUntitled));
      pszTitle = tchUntitled;
    }

    GetLngString(IDS_MUI_PRINT_PAGENUM, tchPageFmt, COUNTOF(tchPageFmt));

    if (!EditPrint(Globals.hwndEdit, pszTitle, tchPageFmt)) {
      InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_PRINT_ERROR, pszTitle);
    }
  }

  if (s_flagAppIsClosing || Globals.CmdLnFlag_PrintFileAndLeave) {
    CloseApplication();
  }

  return(Globals.hwndMain);
}


//=============================================================================
//
//  MainWndProc()
//
//  Messages are distributed to the MsgXXX-handlers
//
//
//inline bool KeyboardIsKeyDown(int key) { return (GetKeyState(key) & 0x8000) != 0; }

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  switch(umsg)
  {
    case WM_CREATE:
      return MsgCreate(hwnd, wParam, lParam);

    case WM_SETFOCUS:
      SetFocus(Globals.hwndEdit);
      break;

    case WM_CLOSE:
      s_flagAppIsClosing = true;
      if (FileSave(false, true, false, false, Flags.bPreserveFileModTime)) {
        DestroyWindow(Globals.hwndMain);
      }
      break;

    case WM_QUERYENDSESSION:
      if (FileSave(false, true, false, false, Flags.bPreserveFileModTime)) {
        return TRUE;
      }
      break;

    case WM_DESTROY:
    case WM_ENDSESSION:
      return MsgEndSession(hwnd, umsg, wParam, lParam);

    // Reinitialize theme-dependent values and resize windows
    case WM_THEMECHANGED:
      return MsgThemeChanged(hwnd, wParam, lParam);

    case WM_DPICHANGED:
      return MsgDPIChanged(hwnd, wParam, lParam);

    // update Scintilla colors
    case WM_SYSCOLORCHANGE:
      if (Flags.bLargeFileLoaded) {
        EditUpdateVisibleIndicators();
      }
      else {
        EditUpdateIndicators(0, -1, false);
      }
      MarkAllOccurrences(0, true);
      UpdateToolbar();
      UpdateStatusbar(true);
      UpdateMarginWidth();
      return DefWindowProc(hwnd,umsg,wParam,lParam);

    case WM_SIZE:
      return MsgSize(hwnd, wParam, lParam);

#ifdef D_NP3_WIN10_DARK_MODE
	  case WM_SETTINGCHANGE: {
      if (IsColorSchemeChangeMessage(lParam)) {
        RefreshTitleBarThemeColor(hwnd);
        SendMessage(Globals.hwndEdit, WM_THEMECHANGED, 0, 0);
      }
    }
    break;
#endif

    case WM_DRAWITEM:
      return MsgDrawItem(hwnd, wParam, lParam);

    case WM_DROPFILES:
      // see SCN_URIDROPP
      return MsgDropFiles(hwnd, wParam, lParam);

    case WM_COPYDATA:
      return MsgCopyData(hwnd, wParam, lParam);

    case WM_CONTEXTMENU:
      MsgContextMenu(hwnd, umsg, wParam, lParam);
      break;

    case WM_ENTERMENULOOP:
      return MsgEnterMenuLoop(hwnd, wParam);

    case WM_INITMENU:
      return MsgInitMenu(hwnd, wParam, lParam);

    case WM_EXITMENULOOP:
      return MsgExitMenuLoop(hwnd, wParam);

    case WM_NOTIFY:
      return MsgNotify(hwnd, wParam, lParam);

    case WM_CHANGENOTIFY:
      return MsgChangeNotify(hwnd, wParam, lParam);

    //case WM_PARENTNOTIFY:
    //  if (iLoWParam & WM_DESTROY) {
    //    if (IsWindow(hDlgFindReplace) && (hDlgFindReplace == (HWND)lParam)) {
    //      hDlgFindReplace = NULL;
    //    }
    //  }
    //  break;

    case WM_COMMAND:
      return MsgCommand(hwnd, umsg, wParam, lParam);

    case WM_TRAYMESSAGE:
      return MsgTrayMessage(hwnd, wParam, lParam);

    //// This message is posted before Notepad3 reactivates itself
    //case WM_CHANGENOTIFYCLEAR:
    //  bPendingChangeNotify = false;
    //  break;

    case WM_DRAWCLIPBOARD:
      if (!s_bLastCopyFromMe)
        s_dwLastCopyTime = GetTickCount();
      else
        s_bLastCopyFromMe = false;

      if (s_hwndNextCBChain)
        SendMessage(s_hwndNextCBChain,WM_DRAWCLIPBOARD,wParam,lParam);
      break;

    case WM_CHANGECBCHAIN:
      if ((HWND)wParam == s_hwndNextCBChain)
        s_hwndNextCBChain = (HWND)lParam;
      if (s_hwndNextCBChain)
        SendMessage(s_hwndNextCBChain,WM_CHANGECBCHAIN,lParam,wParam);
      break;

    case WM_SYSCOMMAND:
      return MsgSysCommand(hwnd, umsg, wParam, lParam);

    case WM_MBUTTONDOWN:
      {
        DocPos const pos = SciCall_CharPositionFromPointClose(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        if (pos >= 0) {
          HandleHotSpotURLClicked(pos, OPEN_WITH_BROWSER);
        }
      }
      break;

    //case WM_LBUTTONDBLCLK:
    //  //return DefWindowProc(hwnd, umsg, wParam, lParam);
    //  break;

    case WM_MOUSEWHEEL:
      if (wParam & MK_CONTROL) { 
        ShowZoomCallTip(); 
      }
      else if (wParam & MK_RBUTTON) {
        // Hold RIGHT MOUSE BUTTON and SCROLL to cycle through UNDO history
        if (GET_WHEEL_DELTA_WPARAM(wParam) > 0) {
          s_bUndoRedoScroll = true;
          SciCall_Redo();
        }
        else if (GET_WHEEL_DELTA_WPARAM(wParam) < 0) {
          s_bUndoRedoScroll = true;
          SciCall_Undo();
        }
      }
      break;

    case WM_INPUTLANGCHANGE:
      Globals.bIsCJKInputCodePage = IsDBCSCodePage(Scintilla_InputCodePage());
      break;

    default:
      if (umsg == s_msgTaskbarCreated) {
        if (!IsWindowVisible(hwnd)) { ShowNotifyIcon(hwnd, true); }
        SetNotifyIconTitle(hwnd);
      }
      return DefWindowProc(hwnd, umsg, wParam, lParam);
  }
  return FALSE;
}


//=============================================================================
//
//  _SetWrapStartIndent()
//
static void  _SetWrapStartIndent()
{
  int i = 0;
  switch (Settings.WordWrapIndent) {
  case 1: i = 1; break;
  case 2: i = 2; break;
  case 3: i = (Globals.fvCurFile.iIndentWidth) ? 1 * Globals.fvCurFile.iIndentWidth : 1 * Globals.fvCurFile.iTabWidth; break;
  case 4: i = (Globals.fvCurFile.iIndentWidth) ? 2 * Globals.fvCurFile.iIndentWidth : 2 * Globals.fvCurFile.iTabWidth; break;
  default: break;
  }
  SciCall_SetWrapStartIndent(i);
}


//=============================================================================
//
//  _SetWrapIndentMode()
//
static void  _SetWrapIndentMode()
{
  int const wrap_mode = (!Globals.fvCurFile.bWordWrap ? SC_WRAP_NONE : ((Settings.WordWrapMode == 0) ? SC_WRAP_WHITESPACE : SC_WRAP_CHAR));

  SciCall_SetWrapMode(wrap_mode);

  if (Settings.WordWrapIndent == 5) {
    SciCall_SetWrapIndentMode(SC_WRAPINDENT_SAME);
  }
  else if (Settings.WordWrapIndent == 6) {
    SciCall_SetWrapIndentMode(SC_WRAPINDENT_INDENT);
  }
  else if (Settings.WordWrapIndent == 7) {
    SciCall_SetWrapIndentMode(SC_WRAPINDENT_DEEPINDENT);
  }
  else {
    _SetWrapStartIndent();
    SciCall_SetWrapIndentMode(SC_WRAPINDENT_FIXED);
  }
}


//=============================================================================
//
//  _SetWrapVisualFlags()
//
static void  _SetWrapVisualFlags(HWND hwndEditCtrl)
{
  if (Settings.ShowWordWrapSymbols) {
    int wrapVisualFlags = 0;
    int wrapVisualFlagsLocation = 0;
    if (Settings.WordWrapSymbols == 0) {
      Settings.WordWrapSymbols = 22;
    }
    switch (Settings.WordWrapSymbols % 10) {
    case 1:
      wrapVisualFlags |= SC_WRAPVISUALFLAG_END;
      wrapVisualFlagsLocation |= SC_WRAPVISUALFLAGLOC_END_BY_TEXT;
      break;
    case 2:
      wrapVisualFlags |= SC_WRAPVISUALFLAG_END;
      break;
    }
    switch (((Settings.WordWrapSymbols % 100) - (Settings.WordWrapSymbols % 10)) / 10) {
    case 1:
      wrapVisualFlags |= SC_WRAPVISUALFLAG_START;
      wrapVisualFlagsLocation |= SC_WRAPVISUALFLAGLOC_START_BY_TEXT;
      break;
    case 2:
      wrapVisualFlags |= SC_WRAPVISUALFLAG_START;
      break;
    }
    SendMessage(hwndEditCtrl, SCI_SETWRAPVISUALFLAGSLOCATION, wrapVisualFlagsLocation, 0);
    SendMessage(hwndEditCtrl, SCI_SETWRAPVISUALFLAGS, wrapVisualFlags, 0);
  }
  else {
    SendMessage(hwndEditCtrl, SCI_SETWRAPVISUALFLAGS, 0, 0);
  }
}


//=============================================================================
//
//  InitializeSciEditCtrl()
//
static void  _InitializeSciEditCtrl(HWND hwndEditCtrl)
{
  InitWindowCommon(hwndEditCtrl, true);

  SendMessage(hwndEditCtrl, SCI_SETTECHNOLOGY, (WPARAM)Settings.RenderingTechnology, 0);
  Settings.RenderingTechnology = SciCall_GetTechnology();
  SendMessage(hwndEditCtrl, SCI_SETBIDIRECTIONAL, (WPARAM)Settings.Bidirectional, 0); // experimental
  Settings.Bidirectional = SciCall_GetBidirectional();

  // Current platforms perform window buffering so it is almost always better for this option to be turned off.
  // There are some older platforms and unusual modes where buffering may still be useful 
  SendMessage(hwndEditCtrl, SCI_SETBUFFEREDDRAW, (WPARAM)(Settings.RenderingTechnology == SC_TECHNOLOGY_DEFAULT), 0);
  //~SendMessage(hwndEditCtrl, SCI_SETPHASESDRAW, SC_PHASES_TWO, 0); // (= default)
  SendMessage(hwndEditCtrl, SCI_SETPHASESDRAW, SC_PHASES_MULTIPLE, 0);
  //~SendMessage(hwndEditCtrl, SCI_SETLAYOUTCACHE, SC_CACHE_DOCUMENT, 0); // memory consumption !
  SendMessage(hwndEditCtrl, SCI_SETLAYOUTCACHE, SC_CACHE_PAGE, 0);
  //~SendMessage(hwndEditCtrl, SCI_SETPOSITIONCACHE, 1024, 0); // default = 1024
  SendMessage(hwndEditCtrl, SCI_SETPOSITIONCACHE, 2048, 0); // default = 1024

  // The possible notification types are the same as the modificationType bit flags used by SCN_MODIFIED: 
  // SC_MOD_INSERTTEXT, SC_MOD_DELETETEXT, SC_MOD_CHANGESTYLE, SC_MOD_CHANGEFOLD, SC_PERFORMED_USER, 
  // SC_PERFORMED_UNDO, SC_PERFORMED_REDO, SC_MULTISTEPUNDOREDO, SC_LASTSTEPINUNDOREDO, SC_MOD_CHANGEMARKER, 
  // SC_MOD_BEFOREINSERT, SC_MOD_BEFOREDELETE, SC_MULTILINEUNDOREDO, and SC_MODEVENTMASKALL.
  //  
  ///~ int const evtMask = SC_MODEVENTMASKALL; (!) - don't listen to all events (SC_MOD_CHANGESTYLE) => RECURSON!
  ///~ SendMessage(hwndEditCtrl, SCI_SETMODEVENTMASK, (WPARAM)evtMask, 0);
  ///~ Don't use: SC_PERFORMED_USER | SC_MOD_CHANGESTYLE; 
  /// SC_MOD_CHANGESTYLE and SC_MOD_CHANGEINDICATOR needs SCI_SETCOMMANDEVENTS=true

  int const evtMask1 = SC_MOD_CONTAINER | SC_PERFORMED_UNDO | SC_PERFORMED_REDO | SC_MULTILINEUNDOREDO;
  int const evtMask2 = SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT | SC_MOD_BEFOREINSERT | SC_MOD_BEFOREDELETE;

  SendMessage(hwndEditCtrl, SCI_SETMODEVENTMASK, (WPARAM)(evtMask1 | evtMask2), 0);
  SendMessage(hwndEditCtrl, SCI_SETCOMMANDEVENTS, false, 0); // speedup folding
  SendMessage(hwndEditCtrl, SCI_SETCODEPAGE, (WPARAM)SC_CP_UTF8, 0); // fixed internal UTF-8 (Sci:default)

  SendMessage(hwndEditCtrl, SCI_SETEOLMODE, Settings.DefaultEOLMode, 0);
  SendMessage(hwndEditCtrl, SCI_SETPASTECONVERTENDINGS, true, 0);
  SendMessage(hwndEditCtrl, SCI_USEPOPUP, false, 0);
  SendMessage(hwndEditCtrl, SCI_SETSCROLLWIDTH, 1, 0);
  SendMessage(hwndEditCtrl, SCI_SETSCROLLWIDTHTRACKING, true, 0);
  SendMessage(hwndEditCtrl, SCI_SETENDATLASTLINE, true, 0);

  SendMessage(hwndEditCtrl, SCI_SETMULTIPLESELECTION, true, 0);
  SendMessage(hwndEditCtrl, SCI_SETADDITIONALSELECTIONTYPING, true, 0);
  SendMessage(hwndEditCtrl, SCI_SETMULTIPASTE, SC_MULTIPASTE_EACH, 0);  // paste into rectangular selection
  SendMessage(hwndEditCtrl, SCI_AUTOCSETMULTI, SC_MULTIAUTOC_EACH, 0);
  SendMessage(hwndEditCtrl, SCI_SETMOUSESELECTIONRECTANGULARSWITCH, true, 0);

  SendMessage(hwndEditCtrl, SCI_SETVIRTUALSPACEOPTIONS, NP3_VIRTUAL_SPACE_ACCESS_OPTIONS, 0);

  SendMessage(hwndEditCtrl, SCI_SETADDITIONALCARETSBLINK, true, 0);
  SendMessage(hwndEditCtrl, SCI_SETADDITIONALCARETSVISIBLE, true, 0);

  // Idle Styling (very large text)
  //~~~SendMessage(hwndEditCtrl, SCI_SETIDLESTYLING, SC_IDLESTYLING_AFTERVISIBLE, 0);
  //~~~SendMessage(hwndEditCtrl, SCI_SETIDLESTYLING, SC_IDLESTYLING_ALL, 0);
  SendMessage(hwndEditCtrl, SCI_SETIDLESTYLING, SC_IDLESTYLING_NONE, 0); // needed for focused view

  // assign command keys
  SendMessage(hwndEditCtrl, SCI_ASSIGNCMDKEY, (SCK_NEXT + (SCMOD_CTRL << 16)), SCI_PARADOWN);
  SendMessage(hwndEditCtrl, SCI_ASSIGNCMDKEY, (SCK_PRIOR + (SCMOD_CTRL << 16)), SCI_PARAUP);
  SendMessage(hwndEditCtrl, SCI_ASSIGNCMDKEY, (SCK_NEXT + ((SCMOD_CTRL | SCMOD_SHIFT) << 16)), SCI_PARADOWNEXTEND);
  SendMessage(hwndEditCtrl, SCI_ASSIGNCMDKEY, (SCK_PRIOR + ((SCMOD_CTRL | SCMOD_SHIFT) << 16)), SCI_PARAUPEXTEND);
  SendMessage(hwndEditCtrl, SCI_ASSIGNCMDKEY, (SCK_HOME + (0 << 16)), SCI_VCHOMEWRAP);
  SendMessage(hwndEditCtrl, SCI_ASSIGNCMDKEY, (SCK_END + (0 << 16)), SCI_LINEENDWRAP);
  SendMessage(hwndEditCtrl, SCI_ASSIGNCMDKEY, (SCK_HOME + (SCMOD_SHIFT << 16)), SCI_VCHOMEWRAPEXTEND);
  SendMessage(hwndEditCtrl, SCI_ASSIGNCMDKEY, (SCK_END + (SCMOD_SHIFT << 16)), SCI_LINEENDWRAPEXTEND);

  // set indicator styles (foreground and alpha maybe overridden by style settings)

  SendMessage(hwndEditCtrl, SCI_INDICSETSTYLE, INDIC_NP3_MATCH_BRACE, INDIC_FULLBOX);
  SendMessage(hwndEditCtrl, SCI_INDICSETFORE, INDIC_NP3_MATCH_BRACE, RGB(0x00, 0xFF, 0x00));
  SendMessage(hwndEditCtrl, SCI_INDICSETALPHA, INDIC_NP3_MATCH_BRACE, 120);
  SendMessage(hwndEditCtrl, SCI_INDICSETOUTLINEALPHA, INDIC_NP3_MATCH_BRACE, 120);

  SendMessage(hwndEditCtrl, SCI_INDICSETSTYLE, INDIC_NP3_BAD_BRACE, INDIC_FULLBOX);
  SendMessage(hwndEditCtrl, SCI_INDICSETFORE, INDIC_NP3_BAD_BRACE, RGB(0xFF, 0x00, 0x00));
  SendMessage(hwndEditCtrl, SCI_INDICSETALPHA, INDIC_NP3_BAD_BRACE, 120);
  SendMessage(hwndEditCtrl, SCI_INDICSETOUTLINEALPHA, INDIC_NP3_BAD_BRACE, 120);
  if (!Settings2.UseOldStyleBraceMatching) {
    SendMessage(hwndEditCtrl, SCI_BRACEHIGHLIGHTINDICATOR, true, INDIC_NP3_MATCH_BRACE);
    SendMessage(hwndEditCtrl, SCI_BRACEBADLIGHTINDICATOR, true, INDIC_NP3_BAD_BRACE);
  }

  SendMessage(hwndEditCtrl, SCI_INDICSETSTYLE, INDIC_NP3_MARK_OCCURANCE, INDIC_ROUNDBOX);
  SendMessage(hwndEditCtrl, SCI_INDICSETFORE, INDIC_NP3_MARK_OCCURANCE, RGB(0x00, 0x00, 0xFF));
  SendMessage(hwndEditCtrl, SCI_INDICSETALPHA, INDIC_NP3_MARK_OCCURANCE, 100);
  SendMessage(hwndEditCtrl, SCI_INDICSETOUTLINEALPHA, INDIC_NP3_MARK_OCCURANCE, 100);

  SendMessage(hwndEditCtrl, SCI_INDICSETSTYLE, INDIC_NP3_HYPERLINK, INDIC_TEXTFORE);
  SendMessage(hwndEditCtrl, SCI_INDICSETFORE, INDIC_NP3_HYPERLINK, RGB(0x00, 0x00, 0xA0));
  SendMessage(hwndEditCtrl, SCI_INDICSETSTYLE, INDIC_NP3_HYPERLINK_U, INDIC_COMPOSITIONTHIN);
  SendMessage(hwndEditCtrl, SCI_INDICSETFORE, INDIC_NP3_HYPERLINK_U, RGB(0x00, 0x00, 0xA0));

  SendMessage(hwndEditCtrl, SCI_INDICSETHOVERSTYLE, INDIC_NP3_HYPERLINK, INDIC_TEXTFORE);
  SendMessage(hwndEditCtrl, SCI_INDICSETHOVERFORE, INDIC_NP3_HYPERLINK, RGB(0x00, 0x00, 0xFF));
  SendMessage(hwndEditCtrl, SCI_INDICSETHOVERSTYLE, INDIC_NP3_HYPERLINK_U, INDIC_COMPOSITIONTHICK);
  SendMessage(hwndEditCtrl, SCI_INDICSETHOVERFORE, INDIC_NP3_HYPERLINK_U, RGB(0x00, 0x00, 0xFF));

  SendMessage(hwndEditCtrl, SCI_INDICSETSTYLE, INDIC_NP3_COLOR_DEF, INDIC_COMPOSITIONTHIN /*INDIC_HIDDEN*/); // MARKER only
  SendMessage(hwndEditCtrl, SCI_INDICSETUNDER, INDIC_NP3_COLOR_DEF, true);
  SendMessage(hwndEditCtrl, SCI_INDICSETALPHA, INDIC_NP3_COLOR_DEF, SC_ALPHA_TRANSPARENT); // reset on hover
  SendMessage(hwndEditCtrl, SCI_INDICSETOUTLINEALPHA, INDIC_NP3_COLOR_DEF, SC_ALPHA_OPAQUE);
  SendMessage(hwndEditCtrl, SCI_INDICSETHOVERSTYLE, INDIC_NP3_COLOR_DEF, INDIC_ROUNDBOX); // HOVER
  SendMessage(hwndEditCtrl, SCI_INDICSETHOVERFORE, INDIC_NP3_COLOR_DEF, RGB(0x00, 0x00, 0x00)); // recalc on hover

  SendMessage(hwndEditCtrl, SCI_INDICSETSTYLE, INDIC_NP3_COLOR_DEF_T, INDIC_HIDDEN ); // invisible
  SendMessage(hwndEditCtrl, SCI_INDICSETHOVERSTYLE, INDIC_NP3_COLOR_DEF_T, INDIC_TEXTFORE);       // HOVER
  SendMessage(hwndEditCtrl, SCI_INDICSETHOVERFORE, INDIC_NP3_COLOR_DEF_T, RGB(0x00, 0x00, 0x00)); // recalc on hover
 
  SendMessage(hwndEditCtrl, SCI_INDICSETSTYLE, INDIC_NP3_UNICODE_POINT, INDIC_COMPOSITIONTHIN /*INDIC_HIDDEN*/); // MARKER only
  //SendMessage(hwndEditCtrl, SCI_INDICSETUNDER, INDIC_NP3_UNICODE_POINT, false);
  SendMessage(hwndEditCtrl, SCI_INDICSETALPHA, INDIC_NP3_UNICODE_POINT, SC_ALPHA_TRANSPARENT);
  SendMessage(hwndEditCtrl, SCI_INDICSETOUTLINEALPHA, INDIC_NP3_UNICODE_POINT, SC_ALPHA_NOALPHA);
  SendMessage(hwndEditCtrl, SCI_INDICSETHOVERSTYLE, INDIC_NP3_UNICODE_POINT, INDIC_ROUNDBOX); // HOVER
  //SendMessage(hwndEditCtrl, SCI_INDICSETHOVERFORE, INDIC_NP3_UNICODE_POINT, RGB(0xE0, 0xE0, 0xE0));

  SendMessage(hwndEditCtrl, SCI_INDICSETSTYLE, INDIC_NP3_MULTI_EDIT, INDIC_ROUNDBOX);
  SendMessage(hwndEditCtrl, SCI_INDICSETFORE, INDIC_NP3_MULTI_EDIT, RGB(0xFF, 0xA5, 0x00));
  SendMessage(hwndEditCtrl, SCI_INDICSETALPHA, INDIC_NP3_MULTI_EDIT, 60);
  SendMessage(hwndEditCtrl, SCI_INDICSETOUTLINEALPHA, INDIC_NP3_MULTI_EDIT, 180);

  // No SC_AUTOMATICFOLD_CLICK, performed by 
  SendMessage(hwndEditCtrl, SCI_SETAUTOMATICFOLD, (WPARAM)(SC_AUTOMATICFOLD_SHOW | SC_AUTOMATICFOLD_CHANGE), 0);

  // Properties
  SendMessage(hwndEditCtrl, SCI_SETCARETSTICKY, (WPARAM)SC_CARETSTICKY_OFF, 0);
  //SendMessage(hwndEditCtrl,SCI_SETCARETSTICKY,SC_CARETSTICKY_WHITESPACE,0);

  if (Settings.ShowHypLnkToolTip || IsColorDefHotspotEnabled()) {
    SendMessage(hwndEditCtrl, SCI_SETMOUSEDWELLTIME, (WPARAM)100, 0);
  }
  else {
    SendMessage(hwndEditCtrl, SCI_SETMOUSEDWELLTIME, (WPARAM)SC_TIME_FOREVER, 0); // default
  }


  #define _CARET_SYMETRY CARET_EVEN /// CARET_EVEN or 0
  #define _CARET_ENFORCE CARET_STRICT /// CARET_STRICT or 0

  if (Settings2.CurrentLineHorizontalSlop > 0)
    SendMessage(hwndEditCtrl, SCI_SETXCARETPOLICY, (WPARAM)(CARET_SLOP | _CARET_SYMETRY | _CARET_ENFORCE), Settings2.CurrentLineHorizontalSlop);
  else
    SendMessage(hwndEditCtrl, SCI_SETXCARETPOLICY, (WPARAM)(CARET_SLOP | _CARET_SYMETRY | _CARET_ENFORCE), (LPARAM)0);

  if (Settings2.CurrentLineVerticalSlop > 0) {
    SendMessage(hwndEditCtrl, SCI_SETYCARETPOLICY, (WPARAM)(CARET_SLOP | _CARET_SYMETRY | _CARET_ENFORCE), Settings2.CurrentLineVerticalSlop);
  }
  else {
    SendMessage(hwndEditCtrl, SCI_SETYCARETPOLICY, (WPARAM)(_CARET_SYMETRY), 0);
  }
  SendMessage(hwndEditCtrl, SCI_SETENDATLASTLINE, (WPARAM)((Settings.ScrollPastEOF) ? 0 : 1), 0);

  // Tabs
  SendMessage(hwndEditCtrl, SCI_SETUSETABS, (WPARAM)!Globals.fvCurFile.bTabsAsSpaces, 0);
  SendMessage(hwndEditCtrl, SCI_SETTABINDENTS, (WPARAM)Globals.fvCurFile.bTabIndents, 0);
  SendMessage(hwndEditCtrl, SCI_SETBACKSPACEUNINDENTS, (WPARAM)Settings.BackspaceUnindents, 0);
  SendMessage(hwndEditCtrl, SCI_SETTABWIDTH, (WPARAM)Globals.fvCurFile.iTabWidth, 0);
  SendMessage(hwndEditCtrl, SCI_SETINDENT, (WPARAM)Globals.fvCurFile.iIndentWidth, 0);

  // Indent Guides
  Style_SetIndentGuides(hwndEditCtrl, Settings.ShowIndentGuides);

  // Word Wrap
  _SetWrapIndentMode(hwndEditCtrl);
  _SetWrapVisualFlags(hwndEditCtrl);

  // Long Lines
  if (Settings.MarkLongLines) {
    SendMessage(hwndEditCtrl, SCI_SETEDGEMODE, (WPARAM)((Settings.LongLineMode == EDGE_BACKGROUND) ? EDGE_BACKGROUND : EDGE_LINE), 0);
  }
  else {
    SendMessage(hwndEditCtrl, SCI_SETEDGEMODE, (WPARAM)EDGE_NONE, 0);
  }
  SendMessage(hwndEditCtrl, SCI_SETEDGECOLUMN, (WPARAM)Settings.LongLinesLimit, 0);

  // general margin
  SendMessage(hwndEditCtrl, SCI_SETMARGINOPTIONS, (WPARAM)SC_MARGINOPTION_SUBLINESELECT, 0);

  // Nonprinting characters
  SendMessage(hwndEditCtrl, SCI_SETVIEWWS, (WPARAM)(Settings.ViewWhiteSpace ? SCWS_VISIBLEALWAYS : SCWS_INVISIBLE), 0);
  SendMessage(hwndEditCtrl, SCI_SETVIEWEOL, (WPARAM)Settings.ViewEOLs, 0);

  // IME Interaction
  SendMessage(hwndEditCtrl, SCI_SETIMEINTERACTION, (WPARAM)Settings2.IMEInteraction, 0);

  // word delimiter handling
  EditInitWordDelimiter(hwndEditCtrl);
  EditSetAccelWordNav(hwndEditCtrl, Settings.AccelWordNavigation);

  UpdateMarginWidth();
}


//=============================================================================
//
//  MsgCreate() - Handles WM_CREATE
//
//
LRESULT MsgCreate(HWND hwnd, WPARAM wParam,LPARAM lParam)
{
  UNUSED(wParam);

#ifdef D_NP3_WIN10_DARK_MODE
  if (IsDarkModeSupported()) {
    AllowDarkModeForWindow(hwnd, CheckDarkModeEnabled());
    RefreshTitleBarThemeColor(hwnd);
  }
#endif

  HINSTANCE const hInstance = ((LPCREATESTRUCT)lParam)->hInstance;

  // Setup edit control
  Globals.hwndEdit = CreateWindowEx(
    WS_EX_CLIENTEDGE,
    L"Scintilla",
    NULL,
    WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
    0, 0, 0, 0,
    hwnd,
    (HMENU)IDC_EDIT,
    hInstance,
    NULL);

  Globals.hndlScintilla = (HANDLE)SendMessage(Globals.hwndEdit, SCI_GETDIRECTPOINTER, 0, 0);

  _InitializeSciEditCtrl(Globals.hwndEdit);

  s_hwndEditFrame = CreateWindowEx(
                    WS_EX_CLIENTEDGE,
                    WC_LISTVIEW,
                    NULL,
                    WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
                    0,0,100,100,
                    hwnd,
                    (HMENU)IDC_EDITFRAME,
                    hInstance,
                    NULL);

  if (IsAppThemed()) {

    RECT rc, rc2;

    s_bIsAppThemed = true;

    SetWindowLongPtr(Globals.hwndEdit,GWL_EXSTYLE,GetWindowLongPtr(Globals.hwndEdit,GWL_EXSTYLE) & ~WS_EX_CLIENTEDGE);
    SetWindowPos(Globals.hwndEdit,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);

    if (IsWindowsVistaOrGreater()) {
      s_cxEditFrame = 0;
      s_cyEditFrame = 0;
    }

    else {
      GetClientRect(s_hwndEditFrame,&rc);
      GetWindowRect(s_hwndEditFrame,&rc2);

      s_cxEditFrame = ((rc2.right-rc2.left) - (rc.right-rc.left)) / 2;
      s_cyEditFrame = ((rc2.bottom-rc2.top) - (rc.bottom-rc.top)) / 2;
    }
  }
  else {
    s_bIsAppThemed = false;

    s_cxEditFrame = 0;
    s_cyEditFrame = 0;
  }

  // Create Toolbar and Statusbar
  CreateBars(hwnd, hInstance);

  // Window Initialization

  (void)CreateWindow(
    WC_STATIC,
    NULL,
    WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
    0,0,10,10,
    hwnd,
    (HMENU)IDC_FILENAME,
    hInstance,
    NULL);

  SetDlgItemText(hwnd,IDC_FILENAME,Globals.CurrentFile);

  (void)CreateWindow(
    WC_STATIC,
    NULL,
    WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
    10,10,10,10,
    hwnd,
    (HMENU)IDC_REUSELOCK,
    hInstance,
    NULL);

  SetDlgItemInt(hwnd,IDC_REUSELOCK,GetTickCount(),false);

  // Menu
  //SetMenuDefaultItem(GetSubMenu(GetMenu(hwnd),0),0);

  // Drag & Drop
  DragAcceptFiles(hwnd,true);

  if (Globals.hwndEdit == NULL || s_hwndEditFrame == NULL ||
    Globals.hwndStatus == NULL || Globals.hwndToolbar == NULL || Globals.hwndRebar == NULL) {
    return -1LL;
  }
  Style_SetDefaultLexer(Globals.hwndEdit);

  Encoding_Current(Settings.DefaultEncoding);

  ObserveNotifyChangeEvent();

  if (g_IniWinInfo.zoom) {
    SciCall_SetZoom(g_IniWinInfo.zoom);
  }

  return 0LL;
}


//=============================================================================
//
//  SelectExternalToolBar() - Select and Load an external Bitmal as ToolBarImage
//
bool SelectExternalToolBar(HWND hwnd)
{
  UNUSED(hwnd);

  WCHAR szArgs[MAX_PATH] = { L'\0' };
  WCHAR szArg2[MAX_PATH] = { L'\0' };
  WCHAR szFile[MAX_PATH] = { L'\0' };
  WCHAR szFilter[MAX_PATH] = { L'\0' };
  OPENFILENAME ofn;
  ZeroMemory(&ofn, sizeof(OPENFILENAME));

  GetDlgItemText(hwnd, IDC_COMMANDLINE, szArgs, COUNTOF(szArgs));
  ExpandEnvironmentStringsEx(szArgs, COUNTOF(szArgs));
  ExtractFirstArgument(szArgs, szFile, szArg2, MAX_PATH);

  GetLngString(IDS_MUI_FILTER_BITMAP, szFilter, COUNTOF(szFilter));
  PrepareFilterStr(szFilter);

  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFilter = szFilter;
  ofn.lpstrFile = szFile;
  ofn.nMaxFile = COUNTOF(szFile);
  ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT
    | OFN_PATHMUSTEXIST | OFN_SHAREAWARE | OFN_NODEREFERENCELINKS;

  if (GetOpenFileName(&ofn)) {
    PathQuoteSpaces(szFile);
    if (StrIsNotEmpty(szArg2))
    {
      StringCchCat(szFile, COUNTOF(szFile), L" ");
      StringCchCat(szFile, COUNTOF(szFile), szArg2);
    }
    PathRelativeToApp(szFile, g_tchToolbarBitmap, COUNTOF(g_tchToolbarBitmap), true,true, true);
    if (Globals.bCanSaveIniFile) {
      IniFileSetString(Globals.IniFile, L"Toolbar Images", L"BitmapDefault", g_tchToolbarBitmap);
    }
  }

  if (StrIsNotEmpty(g_tchToolbarBitmap))
  {
    StringCchCopy(szFile, COUNTOF(szFile), g_tchToolbarBitmap);
    PathRemoveExtension(szFile);
    StringCchCat(szFile, COUNTOF(szFile), L"Hot.bmp");
    if (Globals.bCanSaveIniFile) {
      if (PathIsExistingFile(szFile)) {
        PathRelativeToApp(szFile, g_tchToolbarBitmapHot, COUNTOF(g_tchToolbarBitmapHot), true, true, true);
        IniFileSetString(Globals.IniFile, L"Toolbar Images", L"BitmapHot", g_tchToolbarBitmapHot);
      }
      else {
        StringCchCopy(g_tchToolbarBitmapHot, COUNTOF(g_tchToolbarBitmapHot), L"");
        IniFileDelete(Globals.IniFile, L"Toolbar Images", L"BitmapHot", false);
      }
    }

    StringCchCopy(szFile, COUNTOF(szFile), g_tchToolbarBitmap);
    PathRemoveExtension(szFile);
    StringCchCat(szFile, COUNTOF(szFile), L"Disabled.bmp");
    if (Globals.bCanSaveIniFile) {
      if (PathIsExistingFile(szFile)) {
        PathRelativeToApp(szFile, g_tchToolbarBitmapDisabled, COUNTOF(g_tchToolbarBitmapDisabled), true, true, true);
        IniFileSetString(Globals.IniFile, L"Toolbar Images", L"BitmapDisabled", g_tchToolbarBitmapDisabled);
      }
      else {
        StringCchCopy(g_tchToolbarBitmapHot, COUNTOF(g_tchToolbarBitmapHot), L"");
        IniFileDelete(Globals.IniFile, L"Toolbar Images", L"BitmapDisabled", false);
      }
    }
    Settings.ToolBarTheme = 2;
    return true;
  }
  else {
    IniFileDelete(Globals.IniFile, L"Toolbar Images", L"BitmapHot", false);
    IniFileDelete(Globals.IniFile, L"Toolbar Images", L"BitmapDisabled", false);
  }
  return false;
}


//=============================================================================
//
//  LoadBitmapFile()
//
static HBITMAP LoadBitmapFile(LPCWSTR path)
{
  WCHAR szTmp[MAX_PATH];
  if (PathIsRelative(path)) {
    PathGetAppDirectory(szTmp, COUNTOF(szTmp));
    PathAppend(szTmp, path);
    path = szTmp;
  }
  if (!PathIsExistingFile(path)) { return NULL; }

  HBITMAP hbmp = (HBITMAP)LoadImage(NULL, path, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);

  bool bDimOK = false;
  int height = 16;
  if (hbmp) { 
    BITMAP bmp;  GetObject(hbmp, sizeof(BITMAP), &bmp);
    height = bmp.bmHeight;
    bDimOK = (bmp.bmWidth >= (height * NUMTOOLBITMAPS));
  }
  if (!bDimOK) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_ERR_BITMAP, path, (height * NUMTOOLBITMAPS), height, NUMTOOLBITMAPS);
    if (hbmp) { DeleteObject(hbmp); }
    hbmp = NULL;
  }
  return hbmp;
}


//=============================================================================
//
//  CreateScaledImageListFromBitmap()
//
static HIMAGELIST CreateScaledImageListFromBitmap(HWND hWnd, HBITMAP hBmp)
{
  BITMAP bmp;
  GetObject(hBmp, sizeof(BITMAP), &bmp);

  int const mod = bmp.bmWidth % NUMTOOLBITMAPS;
  int const cx = (bmp.bmWidth - mod) / NUMTOOLBITMAPS;
  int const cy = bmp.bmHeight;

  HIMAGELIST himl = ImageList_Create(cx, cy, ILC_COLOR32 | ILC_MASK, NUMTOOLBITMAPS, NUMTOOLBITMAPS);
  ImageList_AddMasked(himl, hBmp, CLR_DEFAULT);

  DPI_T dpi = Scintilla_GetWindowDPI(hWnd);
  if (!Settings.DpiScaleToolBar || 
      ((dpi.x == USER_DEFAULT_SCREEN_DPI) && (dpi.y == USER_DEFAULT_SCREEN_DPI)))
  {
    return himl; // default DPI, we are done
  }
  

  // Scale button icons/images 
  int const scx = ScaleIntToDPI_X(hWnd, cx);
  int const scy = ScaleIntToDPI_Y(hWnd, cy);

  HIMAGELIST hsciml = ImageList_Create(scx, scy, ILC_COLOR32 | ILC_MASK | ILC_HIGHQUALITYSCALE, NUMTOOLBITMAPS, NUMTOOLBITMAPS);

  for (int i = 0; i < NUMTOOLBITMAPS; ++i) 
  {
    HICON const hicon = ImageList_GetIcon(himl, i, ILD_TRANSPARENT | ILD_PRESERVEALPHA | ILD_SCALE);
    ImageList_AddIcon(hsciml, hicon);
    DestroyIcon(hicon);
  }

  ImageList_Destroy(himl);

  return hsciml;
}


//==== Toolbar Style ==========================================================
#define NP3_WS_TOOLBAR (WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |                          \
                        TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | TBSTYLE_ALTDRAG | TBSTYLE_LIST | \
                        CCS_NODIVIDER | CCS_NOPARENTALIGN | CCS_ADJUSTABLE)

//==== ReBar Style ============================================================
#define NP3_WS_REBAR (WS_CHILD | WS_CLIPCHILDREN | WS_BORDER | RBS_VARHEIGHT | \
                      RBS_BANDBORDERS | CCS_NODIVIDER | CCS_NOPARENTALIGN)

//=============================================================================
//
//  CreateBars() - Create Toolbar and Statusbar
//
//
void CreateBars(HWND hwnd, HINSTANCE hInstance)
{
  DWORD dwToolbarStyle = NP3_WS_TOOLBAR /*| TBSTYLE_CUSTOMERASE */ | TBSTYLE_TRANSPARENT;
  DWORD dwToolbarExStyle = TBSTYLE_EX_DOUBLEBUFFER /* | TBSTYLE_EX_HIDECLIPPEDBUTTONS */;

  if (Settings.ToolBarTheme < 0) { // undefined: determine High DPI screen
    DPI_T const dpi       = Scintilla_GetWindowDPI(hwnd);
    Settings.ToolBarTheme = (dpi.y < LargeIconDPI()) ? 0 : 1;
  }

  if (Globals.hwndToolbar) 
  { 
    HIMAGELIST himl = (HIMAGELIST)SendMessage(Globals.hwndToolbar, TB_GETIMAGELIST, 0, 0);
    if (himl) { ImageList_Destroy(himl); }
    himl = (HIMAGELIST)SendMessage(Globals.hwndToolbar, TB_GETHOTIMAGELIST, 0, 0);
    if (himl) { ImageList_Destroy(himl); }
    himl = (HIMAGELIST)SendMessage(Globals.hwndToolbar, TB_GETDISABLEDIMAGELIST, 0, 0);
    if (himl) { ImageList_Destroy(himl); }
    DestroyWindow(Globals.hwndToolbar); 
  }

  bool bOpendByMe = false;
  OpenSettingsFile(&bOpendByMe);
  bool bDirtyFlag = false;

  //InitToolbarWndClass(hInstance);
  Globals.hwndToolbar = CreateWindowEx(dwToolbarExStyle, TOOLBARCLASSNAME, NULL, dwToolbarStyle,
                                       0,0,0,0,hwnd,(HMENU)IDC_TOOLBAR,hInstance,NULL);
  //Globals.hwndToolbar = CreateWindowEx(
  //    dwToolbarExStyle,          // no extended styles
  //    s_ToolbarWndClassName,     // name of status bar class
  //    (PCTSTR)NULL,              // no text when first created
  //    dwToolbarStyle,            // creates a visible child window
  //    0, 0, 0, 0,                // ignores size and position
  //    hwnd,                      // handle to parent window
  //    (HMENU)IDC_TOOLBAR,        // child window identifier
  //    hInstance,                 // handle to application instance
  //    NULL);                     // no window creation data

  InitWindowCommon(Globals.hwndToolbar, true);

#ifdef D_NP3_WIN10_DARK_MODE
  if (IsDarkModeSupported()) {
    AllowDarkModeForWindow(Globals.hwndToolbar, CheckDarkModeEnabled());
  }
#endif

  SendMessage(Globals.hwndToolbar,TB_BUTTONSTRUCTSIZE,(WPARAM)sizeof(TBBUTTON),0);

  // -------------------------
  // Add Toolbar Bitmap
  // -------------------------
  HBITMAP hbmp = NULL;
  HBITMAP hbmpCopy = NULL;

  if ((Settings.ToolBarTheme == 2) && StrIsNotEmpty(g_tchToolbarBitmap))
  {
    hbmp = LoadBitmapFile(g_tchToolbarBitmap);
    if (!hbmp) {
      StringCchCopy(g_tchToolbarBitmap, COUNTOF(g_tchToolbarBitmap), L"");
      IniSectionDelete(L"Toolbar Images", L"BitmapDefault", false);
      bDirtyFlag = true;
    }
  }
  if (!hbmp) {
    Settings.ToolBarTheme = Settings.ToolBarTheme % 2;
    LPCWSTR toolBarIntRes = (Settings.ToolBarTheme == 0) ? (LPCWSTR)MAKEINTRESOURCE(IDR_MAINWNDTB) : (LPCWSTR)MAKEINTRESOURCE(IDR_MAINWNDTB2);
    hbmp = LoadImage(hInstance, toolBarIntRes, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
  }

  // use copy for alphablend a disabled Toolbar (if not provided)
  hbmpCopy = CopyImage(hbmp, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);


  HIMAGELIST himl = CreateScaledImageListFromBitmap(hwnd, hbmp);
  DeleteObject(hbmp);
  hbmp = NULL;

  SendMessage(Globals.hwndToolbar,TB_SETIMAGELIST,0,(LPARAM)himl);


  // --------------------------
  // Add a Hot Toolbar Bitmap
  // --------------------------
  if ((Settings.ToolBarTheme == 2) && StrIsNotEmpty(g_tchToolbarBitmapHot))
  {
    hbmp = LoadBitmapFile(g_tchToolbarBitmapHot);
    if (!hbmp) {
      StringCchCopy(g_tchToolbarBitmapHot, COUNTOF(g_tchToolbarBitmapHot), L"");
      IniSectionDelete(L"Toolbar Images", L"BitmapHot", false);
      bDirtyFlag = true;
    }
  }
  if (!hbmp && (Settings.ToolBarTheme < 2)) {
    LPCWSTR toolBarIntRes = (Settings.ToolBarTheme == 0) ? (LPCWSTR)MAKEINTRESOURCE(IDR_MAINWNDTBHOT) : (LPCWSTR)MAKEINTRESOURCE(IDR_MAINWNDTB2HOT);
    hbmp = LoadImage(hInstance, toolBarIntRes, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
  }

  if (hbmp) 
  {
    himl = CreateScaledImageListFromBitmap(hwnd, hbmp);
    DeleteObject(hbmp);
    hbmp = NULL;

    SendMessage(Globals.hwndToolbar, TB_SETHOTIMAGELIST, 0, (LPARAM)himl);
  }
  else { // clear the old one
    SendMessage(Globals.hwndToolbar, TB_SETHOTIMAGELIST, 0, 0);
  }


  // ------------------------------
  // Add a disabled Toolbar Bitmap
  // ------------------------------
  if ((Settings.ToolBarTheme == 2) && StrIsNotEmpty(g_tchToolbarBitmapDisabled))
  {
    hbmp = LoadBitmapFile(g_tchToolbarBitmapDisabled);
    if (!hbmp) {
      StringCchCopy(g_tchToolbarBitmapDisabled, COUNTOF(g_tchToolbarBitmapDisabled), L"");
      IniSectionDelete(L"Toolbar Images", L"BitmapDisabled", false);
      bDirtyFlag = true;
    }
  }
  if (!hbmp && (Settings.ToolBarTheme < 2)) {
    LPCWSTR toolBarIntRes = (Settings.ToolBarTheme == 0) ? (LPCWSTR)MAKEINTRESOURCE(IDR_MAINWNDTBDIS) : (LPCWSTR)MAKEINTRESOURCE(IDR_MAINWNDTB2DIS);
    hbmp = LoadImage(hInstance, toolBarIntRes, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
  }

  if (hbmp)
  {
    himl = CreateScaledImageListFromBitmap(hwnd, hbmp);
    DeleteObject(hbmp);
    hbmp = NULL;

    SendMessage(Globals.hwndToolbar, TB_SETDISABLEDIMAGELIST, 0, (LPARAM)himl);
  }
  else {  // create disabled Toolbar, no external bitmap is supplied

    if ((Settings.ToolBarTheme == 2) && StrIsEmpty(g_tchToolbarBitmapDisabled))
    {
      bool bProcessed = false;
      if (Flags.ToolbarLook == 1) {
        bProcessed = BitmapAlphaBlend(hbmpCopy, GetSysColor(COLOR_3DFACE), 0x60);
      }
      else if (Flags.ToolbarLook == 2 || (!IsWindowsXPSP3OrGreater() && Flags.ToolbarLook == 0)) {
        bProcessed = BitmapGrayScale(hbmpCopy);
      }
      if (bProcessed && !IsWindowsXPSP3OrGreater()) {
        BitmapMergeAlpha(hbmpCopy, GetSysColor(COLOR_3DFACE));
      }
      if (bProcessed)
      {
        himl = CreateScaledImageListFromBitmap(hwnd, hbmpCopy);

        SendMessage(Globals.hwndToolbar, TB_SETDISABLEDIMAGELIST, 0, (LPARAM)himl);
      }
    }
  }

  if (hbmpCopy) {
    DeleteObject(hbmpCopy);
    hbmpCopy = NULL;
  }

  // Load toolbar labels
  WCHAR tchDesc[256] = { L'\0' };
  WCHAR tchIndex[256] = { L'\0' };
  for (int i = 0; i < COUNTOF(s_tbbMainWnd); ++i) {
    if (s_tbbMainWnd[i].fsStyle == BTNS_SEP) { continue; }

    int n = s_tbbMainWnd[i].iBitmap + 1;
    StringCchPrintf(tchIndex, COUNTOF(tchIndex), L"%02i", n);

    if (IniSectionGetString(L"Toolbar Labels", tchIndex, L"", tchDesc, COUNTOF(tchDesc)) > 0) {
      s_tbbMainWnd[i].iString = SendMessage(Globals.hwndToolbar, TB_ADDSTRING, 0, (LPARAM)tchDesc);
      s_tbbMainWnd[i].fsStyle |= BTNS_AUTOSIZE | BTNS_SHOWTEXT;
    }
    else {
      GetLngString(s_tbbMainWnd[i].idCommand, tchDesc, COUNTOF(tchDesc));
      s_tbbMainWnd[i].iString = SendMessage(Globals.hwndToolbar, TB_ADDSTRING, 0, (LPARAM)tchDesc); // tooltip
      s_tbbMainWnd[i].fsStyle &= ~(BTNS_AUTOSIZE | BTNS_SHOWTEXT);
    }
  }

  //~SendMessage(Globals.hwndToolbar, TB_SETMAXTEXTROWS, 0, 0);
  DWORD const tbxstyle = (DWORD)SendMessage(Globals.hwndToolbar, TB_GETEXTENDEDSTYLE, 0, 0);
  SendMessage(Globals.hwndToolbar,TB_SETEXTENDEDSTYLE,0,(tbxstyle | (TBSTYLE_EX_MIXEDBUTTONS | TBSTYLE_EX_DOUBLEBUFFER)));

  SendMessage(Globals.hwndToolbar, TB_ADDBUTTONS, COUNTOF(s_tbbMainWnd), (LPARAM)s_tbbMainWnd);

  if (Toolbar_SetButtons(Globals.hwndToolbar, IDT_FILE_NEW, Settings.ToolbarButtons, s_tbbMainWnd, COUNTOF(s_tbbMainWnd)) == 0) {
    SendMessage(Globals.hwndToolbar, TB_ADDBUTTONS, COUNTOF(s_tbbMainWnd), (LPARAM)s_tbbMainWnd);
  }

  CloseSettingsFile(bDirtyFlag, bOpendByMe);

  // ------------------------------
  // Create ReBar and add Toolbar
  // ------------------------------
  DWORD const dwReBarStyle = Settings.ShowToolbar ? (NP3_WS_REBAR | WS_VISIBLE) : (NP3_WS_REBAR);

  if (Globals.hwndRebar) {
    DestroyWindow(Globals.hwndRebar);
  }
  Globals.hwndRebar = CreateWindowEx(WS_EX_TOOLWINDOW, REBARCLASSNAME, NULL, dwReBarStyle,
                                     0,0,0,0,hwnd,(HMENU)IDC_REBAR,hInstance,NULL);

  InitWindowCommon(Globals.hwndRebar, false); // false(!) ~ you cannot change a toolbar's color when a visual style is active

#ifdef D_NP3_WIN10_DARK_MODE
  if (IsDarkModeSupported()) {
    AllowDarkModeForWindow(Globals.hwndRebar, CheckDarkModeEnabled());
  }
#endif

  REBARINFO rbi;
  rbi.cbSize = sizeof(REBARINFO);
  rbi.fMask  = 0;
  rbi.himl   = (HIMAGELIST)NULL;
  SendMessage(Globals.hwndRebar, RB_SETBARINFO, 0, (LPARAM)&rbi);

  RECT rc;
  SendMessage(Globals.hwndToolbar, TB_GETITEMRECT, 0, (LPARAM)&rc);
  //SendMessage(Globals.hwndToolbar,TB_SETINDENT,2,0);

  REBARBANDINFO rbBand;  ZeroMemory(&rbBand, sizeof(REBARBANDINFO));
  rbBand.cbSize  = sizeof(REBARBANDINFO);
  rbBand.fMask   = RBBIM_COLORS /*| RBBIM_TEXT | RBBIM_BACKGROUND */ |
                   RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE /*| RBBIM_SIZE*/;
  //rbBand.fStyle  = /*RBBS_CHILDEDGE |*//* RBBS_BREAK |*/ RBBS_FIXEDSIZE /*| RBBS_GRIPPERALWAYS*/;
  rbBand.fStyle = s_bIsAppThemed ? (RBBS_FIXEDSIZE | RBBS_CHILDEDGE) : RBBS_FIXEDSIZE;
  rbBand.hbmBack = NULL;
  rbBand.lpText  = L"Toolbar";
  rbBand.clrFore = UseDarkMode() ? g_rgbDarkTextColor : (COLORREF)GetSysColor(COLOR_WINDOWTEXT);
  rbBand.clrBack = UseDarkMode() ? g_rgbDarkBkgColor : (COLORREF)GetSysColor(COLOR_WINDOW);
  rbBand.hwndChild  = Globals.hwndToolbar;
  rbBand.cxMinChild = (rc.right - rc.left) * COUNTOF(s_tbbMainWnd);
  rbBand.cyMinChild = (rc.bottom - rc.top) + 2 * rc.top;
  rbBand.cx         = 0;
  SendMessage(Globals.hwndRebar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand);

  SetWindowPos(Globals.hwndRebar, NULL, 0, 0, 0, 0, SWP_NOZORDER);
  GetWindowRect(Globals.hwndRebar, &rc);
  s_cyReBar = rc.bottom - rc.top;

  s_cyReBarFrame = s_bIsAppThemed ? 0 : 2;


  // -------------------
  // Create Statusbar 
  // -------------------
  DWORD const dwStatusbarStyle = SBT_NOBORDERS | Settings.ShowStatusbar ? (WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE) : (WS_CHILD | WS_CLIPSIBLINGS);

  if (Globals.hwndStatus) { DestroyWindow(Globals.hwndStatus); }


  Globals.hwndStatus = CreateStatusWindow(dwStatusbarStyle, NULL, hwnd, IDC_STATUSBAR);
  //~Globals.hwndStatus = CreateWindowEx(
  //~    0,                         // no extended styles
  //~    STATUSCLASSNAME,           // name of status bar class
  //~    (PCTSTR)NULL,              // no text when first created
  //~    dwStatusbarStyle,          // creates a visible child window
  //~    0, 0, 0, 0,                // ignores size and position
  //~    hwnd,                      // handle to parent window
  //~    (HMENU)IDC_STATUSBAR,      // child window identifier
  //~    hInstance,                 // handle to application instance
  //~    NULL);                     // no window creation data

  InitWindowCommon(Globals.hwndStatus, true);

#ifdef D_NP3_WIN10_DARK_MODE
  if (IsDarkModeSupported()) {
    AllowDarkModeForWindow(Globals.hwndStatus, CheckDarkModeEnabled());
  }
#endif
}


//=============================================================================
//
//  MsgEndSession() - Handle WM_ENDSESSION,WM_DESTROY
//
//
LRESULT MsgEndSession(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  UNUSED(wParam);
  UNUSED(lParam);

  static bool bShutdownOK = false;

  if (!bShutdownOK) {

    if (s_bPrevFullScreenFlag) {
      SendWMCommand(hwnd, CMD_FULLSCRWINPOS);
    }

    // Terminate file watching
    InstallFileWatching(NULL);

    DragAcceptFiles(hwnd, true);

    // Terminate clipboard watching
    if (s_flagPasteBoard) {
      KillTimer(hwnd, ID_PASTEBOARDTIMER);
      ChangeClipboardChain(hwnd, s_hwndNextCBChain);
    }

    // close Find/Replace and CustomizeSchemes
    CloseNonModalDialogs();
    
    // call SaveAllSettings() when Globals.hwndToolbar is still valid
    SaveAllSettings(false);

    // Remove tray icon if necessary
    ShowNotifyIcon(hwnd, false);

    bShutdownOK = true;
  }

  if (umsg == WM_DESTROY) {
    PostQuitMessage(0);
  }

  return FALSE;
}



//=============================================================================
//
// MsgDPIChanged() - Handle WM_DPICHANGED
//
//
LRESULT MsgDPIChanged(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  //DPI_T dpi;
  //dpi.x = LOWORD(wParam);
  //dpi.y = HIWORD(wParam);
  UNUSED(wParam);
  //const RECT* const rc = (RECT*)lParam;

  DocPos const pos = SciCall_GetCurrentPos();

  UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, NULL);

  SendMessage(Globals.hwndEdit, WM_DPICHANGED, wParam, lParam);

  MsgThemeChanged(hwnd, wParam, lParam);

  Sci_GotoPosChooseCaret(pos);
    
  return TRUE;
}


//=============================================================================
//
//  MsgThemeChanged() - Handle WM_THEMECHANGED
//
//
LRESULT MsgThemeChanged(HWND hwnd, WPARAM wParam ,LPARAM lParam)
{
  UNUSED(lParam);
  UNUSED(wParam);
  
  RECT rc, rc2;

  // reinitialize edit frame
  if (IsAppThemed()) {
    s_bIsAppThemed = true;

    SetWindowLongPtr(Globals.hwndEdit,GWL_EXSTYLE,GetWindowLongPtr(Globals.hwndEdit,GWL_EXSTYLE) & ~WS_EX_CLIENTEDGE);
    SetWindowPos(Globals.hwndEdit,NULL,0,0,0,0,SWP_NOZORDER|SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE);

    if (IsWindowsVistaOrGreater()) {
      s_cxEditFrame = 0;
      s_cyEditFrame = 0;
    }
    else {
      SetWindowPos(s_hwndEditFrame,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
      GetClientRect(s_hwndEditFrame,&rc);
      GetWindowRect(s_hwndEditFrame,&rc2);

      s_cxEditFrame = ((rc2.right-rc2.left) - (rc.right-rc.left)) / 2;
      s_cyEditFrame = ((rc2.bottom-rc2.top) - (rc.bottom-rc.top)) / 2;
    }
  }
  else {
    s_bIsAppThemed = false;

    SetWindowLongPtr(Globals.hwndEdit,GWL_EXSTYLE,WS_EX_CLIENTEDGE|GetWindowLongPtr(Globals.hwndEdit,GWL_EXSTYLE));
    SetWindowPos(Globals.hwndEdit,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);

    s_cxEditFrame = 0;
    s_cyEditFrame = 0;
  }

  // recreate toolbar and statusbar
  CreateBars(hwnd,Globals.hInstance);
  SendWMSize(hwnd, NULL);

  Style_ResetCurrentLexer(Globals.hwndEdit);

  if (FocusedView.HideNonMatchedLines) {
    EditToggleView(Globals.hwndEdit);
  }

  MarkAllOccurrences(0, false);

  if (Flags.bLargeFileLoaded) {
    EditDoVisibleStyling();
  }
  else {
    EditDoStyling(0, -1);
  }

  EditUpdateVisibleIndicators();

#ifdef D_NP3_WIN10_DARK_MODE
  AllowDarkModeForWindow(hwnd, UseDarkMode());
  RefreshTitleBarThemeColor(hwnd);
#endif

  UpdateUI();
  UpdateToolbar();
  UpdateStatusbar(true);
  UpdateMarginWidth();
  UpdateTitleBar();

  return FALSE;
}


//=============================================================================
//
//  MsgSize() - Handles WM_SIZE
//
//
LRESULT MsgSize(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  UNUSED(hwnd);
  UNUSED(lParam);

  if (wParam == SIZE_MINIMIZED) { return FALSE; }

  int x = 0;
  int y = 0;

  int cx = GET_X_LPARAM(lParam);
  int cy = GET_Y_LPARAM(lParam);

  if (Settings.ShowToolbar)
  {
/*  SendMessage(Globals.hwndToolbar,WM_SIZE,0,0);
    RECT rc;
    GetWindowRect(Globals.hwndToolbar,&rc);
    y = (rc.bottom - rc.top);
    cy -= (rc.bottom - rc.top);*/

    //SendMessage(Globals.hwndToolbar,TB_GETITEMRECT,0,(LPARAM)&rc);
    SetWindowPos(Globals.hwndRebar, NULL, 0, 0, LOWORD(lParam), s_cyReBar, SWP_NOZORDER);
    // the ReBar automatically sets the correct height
    // calling SetWindowPos() with the height of one toolbar button
    // causes the control not to temporarily use the whole client area
    // and prevents flickering

    //GetWindowRect(Globals.hwndRebar,&rc);
    y = s_cyReBar + s_cyReBarFrame;    // define
    cy -= s_cyReBar + s_cyReBarFrame;  // border
  }

  if (Settings.ShowStatusbar)
  {
    RECT rc;
    SendMessage(Globals.hwndStatus,WM_SIZE,0,0);
    GetWindowRect(Globals.hwndStatus,&rc);
    cy -= (rc.bottom - rc.top);
  }

  HDWP hdwp = BeginDeferWindowPos(2);

  DeferWindowPos(hdwp,s_hwndEditFrame,NULL,x,y,cx,cy, SWP_NOZORDER | SWP_NOACTIVATE);

  DeferWindowPos(hdwp,Globals.hwndEdit,NULL,x+s_cxEditFrame,y+s_cyEditFrame,
                 cx-2*s_cxEditFrame,cy-2*s_cyEditFrame,
                 SWP_NOZORDER | SWP_NOACTIVATE);

  EndDeferWindowPos(hdwp);

  s_WinCurrentWidth = cx;

  UpdateToolbar();
  UpdateStatusbar(true);
  UpdateMarginWidth();

  return FALSE;
}


//=============================================================================
//
//  MsgDrawItem() - Handles WM_DRAWITEM
//
//  https://docs.microsoft.com/en-us/windows/win32/controls/status-bars#owner-drawn-status-bars
//
LRESULT MsgDrawItem(HWND hwnd, WPARAM wParam, LPARAM lParam) 
{
  UNUSED(hwnd);

  if (LOWORD(wParam) == IDC_STATUSBAR) // Statusbar SB_SETTEXT caused parent's WM_DRAWITEM message
  {
    const DRAWITEMSTRUCT* const pDIS = (const DRAWITEMSTRUCT* const)lParam;

    HDC const hdc = pDIS->hDC;
    RECT const rc = pDIS->rcItem;

    //UINT const ctlId = pDIS->CtlID; // child window identifier
    //~int const partId = (int)pDIS->itemID ~ don't use
    //~int const stateId = (int)pDIS->itemState ~ don't use

    //~PAINTSTRUCT ps;
    //~BeginPaint(hWndItem, &ps); ~ not needed on WM_DRAWITEM

#ifdef D_NP3_WIN10_DARK_MODE

    HWND const hWndItem = pDIS->hwndItem;

    if (UseDarkMode())
    {
      // overpaint part frames
      HDC const hdcFrm = GetWindowDC(hWndItem);
      RECT rcf = rc;
      rcf.left -= 1;
      rcf.top -= 1;
      FrameRect(hdcFrm, &rcf, g_hbrWndDarkBkgBrush);
      rcf.left -= 1;
      rcf.top -= 1;
      rcf.bottom += 1;
      rcf.right += 1;
      FrameRect(hdcFrm, &rcf, g_hbrWndDarkBkgBrush);
      rcf.left -= 1;
      rcf.top -= 1;
      FrameRect(hdcFrm, &rcf, g_hbrWndDarkBkgBrush);
      ReleaseDC(hWndItem, hdcFrm);

      SetBkColor(hdc, g_rgbDarkBkgColor);
      SetTextColor(hdc, g_rgbDarkTextColor);
    } 
    else {
      SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
      SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));
    }

#endif

    LPCWSTR const text = (LPCWSTR)(pDIS->itemData);
    ExtTextOut(hdc, rc.left + 2, rc.top + 2, ETO_OPAQUE | ETO_NUMERICSLOCAL, &rc, text, lstrlen(text), NULL);

    //~EndPaint(hWndItem, &ps);
    return TRUE;
  }
  return FALSE;
}


//=============================================================================
//
//  MsgDropFiles() - Handles WM_DROPFILES
//
//
LRESULT MsgDropFiles(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  UNUSED(lParam);

  WCHAR szDropStrgBuf[MAX_PATH + 40];
  HDROP hDrop = (HDROP)wParam;

  if (IsIconic(hwnd)) {
    ShowWindow(hwnd, SW_RESTORE);
  }

  DragQueryFile(hDrop, 0, szDropStrgBuf, COUNTOF(szDropStrgBuf));

  if (PathIsDirectory(szDropStrgBuf)) {
    WCHAR tchFile[MAX_PATH] = { L'\0' };
    if (OpenFileDlg(Globals.hwndMain, tchFile, COUNTOF(tchFile), szDropStrgBuf)) {
      FileLoad(false, false, false, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, false, tchFile);
    }
  }
  else if (PathIsExistingFile(szDropStrgBuf)) {
    if (Flags.bReuseWindow) {
      FileLoad(false, false, false, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, false, szDropStrgBuf);
    }
    else {
      DialogNewWindow(hwnd, Settings.SaveBeforeRunningTools, szDropStrgBuf);
    }
  }
  else {
    // delegated to SCN_URIDROPPED
    // Windows Bug: wParam (HDROP) pointer is corrupted if dropped from 32-bit App
    //~InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_DROP_NO_FILE);
  }

  if (DragQueryFile(hDrop, (UINT)(-1), NULL, 0) > 1) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_ERR_DROP);
  }

  DragFinish(hDrop);

  return FALSE;
}


//=============================================================================
//
//  DropFilesProc() - Handles DROPFILES
//
//
static DWORD DropFilesProc(CLIPFORMAT cf, HGLOBAL hData, HWND hWnd, DWORD dwKeyState, POINTL pt, void *pUserData)
{
  DWORD dwEffect = DROPEFFECT_NONE;

  //HWND hEditWnd = (HWND)pUserData;
  UNUSED(pUserData);

  if (cf == CF_HDROP)
  {
    WCHAR szBuf[MAX_PATH + 40];
    HDROP hDrop = (HDROP)hData;

    if (IsIconic(hWnd))
      ShowWindow(hWnd, SW_RESTORE);

    DragQueryFile(hDrop, 0, szBuf, COUNTOF(szBuf));

    if (PathIsDirectory(szBuf)) {
      WCHAR tchFile[MAX_PATH] = { L'\0' };
      if (OpenFileDlg(hWnd, tchFile, COUNTOF(tchFile), szBuf))
        FileLoad(false, false, false, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, false, tchFile);
    }
    else
      FileLoad(false, false, false, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, false, szBuf);

    if (DragQueryFile(hDrop, (UINT)(-1), NULL, 0) > 1) {
      InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_ERR_DROP);
    }
    dwEffect = DROPEFFECT_COPY;
  }

  UNUSED(dwKeyState);
  UNUSED(pt);

  return dwEffect;
} 


//=============================================================================
//
//  MsgCopyData() - Handles WM_COPYDATA
//
//
LRESULT MsgCopyData(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  UNUSED(wParam);

  PCOPYDATASTRUCT pcds = (PCOPYDATASTRUCT)lParam;

  // Reset Change Notify
  //bPendingChangeNotify = false;

  SetDlgItemInt(hwnd, IDC_REUSELOCK, GetTickCount(), false);

  if (pcds->dwData == DATA_NOTEPAD3_PARAMS) 
  {
    LPnp3params params = AllocMem(pcds->cbData, HEAP_ZERO_MEMORY);
    if (params) {
      CopyMemory(params, pcds->lpData, pcds->cbData);

      if (params->flagLexerSpecified) {
        s_flagLexerSpecified = true;
      }
      if (params->flagQuietCreate) {
        s_flagQuietCreate = true;
      }
      if (params->flagFileSpecified) {

        bool bOpened = false;
        Encoding_Forced(params->flagSetEncoding);

        if (PathIsDirectory(&params->wchData)) {
          WCHAR tchFile[MAX_PATH] = { L'\0' };
          if (OpenFileDlg(Globals.hwndMain, tchFile, COUNTOF(tchFile), &params->wchData))
            bOpened = FileLoad(false, false, false, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, false, tchFile);
        }
        else {
          bOpened = FileLoad(false, false, false, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, false, &params->wchData);
        }
        if (bOpened) {
          if (params->flagChangeNotify == FWM_MSGBOX) 
          {
            FileWatching.FileWatchingMode = FWM_DONT_CARE;
            FileWatching.ResetFileWatching = true;
            InstallFileWatching(Globals.CurrentFile);
          }
          else if (params->flagChangeNotify == FWM_AUTORELOAD) {
            if (!FileWatching.MonitoringLog) {
              PostWMCommand(Globals.hwndMain, IDM_VIEW_CHASING_DOCTAIL);
            }
            else {
              FileWatching.FileWatchingMode = FWM_AUTORELOAD;
              FileWatching.ResetFileWatching = true;
              InstallFileWatching(Globals.CurrentFile);
            }
          }

          if (params->flagSetEncoding != CPI_NONE) {
            s_flagSetEncoding = params->flagSetEncoding;
            SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_ENCODING_SELECT, IDM_ENCODING_SELECT + s_flagSetEncoding), 0);
            s_flagSetEncoding = CPI_NONE;
          }

          if (0 != params->flagSetEOLMode) {
            s_flagSetEOLMode = params->flagSetEOLMode;
            SendWMCommand(Globals.hwndMain, IDM_LINEENDINGS_CRLF + s_flagSetEOLMode - 1);
            s_flagSetEOLMode = 0;
          }

          if (params->flagLexerSpecified) {
            if (params->iInitialLexer < 0) {
              WCHAR wchExt[32] = { L'\0' };
              StringCchCopy(wchExt, COUNTOF(wchExt), L".");
              StringCchCopyN(CharNext(wchExt), 32, StrEnd(&params->wchData, 0) + 1, 31);
              Style_SetLexerFromName(Globals.hwndEdit, &params->wchData, wchExt);
            }
            else if (params->iInitialLexer >= 0 && params->iInitialLexer < NUMLEXERS)
              Style_SetLexerFromID(Globals.hwndEdit, params->iInitialLexer);
          }

          if (params->flagTitleExcerpt) {
            StringCchCopyN(s_wchTitleExcerpt, COUNTOF(s_wchTitleExcerpt), StrEnd(&params->wchData, 0) + 1, COUNTOF(s_wchTitleExcerpt));
          }
        }
        // reset
        Encoding_Forced(CPI_NONE);
      }

      if (params->flagJumpTo) {
        s_flagJumpTo = true;
        EditJumpTo(params->iInitialLine, params->iInitialColumn);
      }

      if (params->flagMatchText)
      {
        g_flagMatchText = params->flagMatchText;
        SetFindPattern(StrEnd(&params->wchData, 0) + 1);
        SetFindReplaceData(); // s_FindReplaceData

        if (g_flagMatchText & 2) {
          if (!s_flagJumpTo) { SciCall_DocumentEnd(); }
          EditFindPrev(Globals.hwndEdit, &s_FindReplaceData, false, false);
        }
        else {
          if (!s_flagJumpTo) { SciCall_DocumentStart(); }
          EditFindNext(Globals.hwndEdit, &s_FindReplaceData, false, false);
        }
      }

      s_flagLexerSpecified = false;
      s_flagQuietCreate = false;

      FreeMem(params);
    }

    UpdateToolbar();
    UpdateStatusbar(true);
    UpdateMarginWidth();
  }

  return FALSE;
}

//=============================================================================
//
//  MsgContextMenu() - Handles WM_CONTEXTMENU
//
//
LRESULT MsgContextMenu(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  bool const bMargin = (SCN_MARGINRIGHTCLICK == umsg);
  int const nID = bMargin ? IDC_MARGIN : GetDlgCtrlID((HWND)wParam);
  if ((nID != IDC_MARGIN) && (nID != IDC_EDIT) && (nID != IDC_STATUSBAR) && (nID != IDC_REBAR) && (nID != IDC_TOOLBAR)) {
    return DefWindowProc(hwnd, umsg, wParam, lParam);
  }

  // no context menu after undo/redo history scrolling
  if (s_bUndoRedoScroll) {
    s_bUndoRedoScroll = false;
    return FALSE;
  }

  HMENU hMenuCtx = LoadMenu(Globals.hLngResContainer, MAKEINTRESOURCE(IDR_MUI_POPUPMENU));
  //SetMenuDefaultItem(GetSubMenu(hmenu,1),0,false);

  POINT pt;
  pt.x = (int)(short)LOWORD(lParam);
  pt.y = (int)(short)HIWORD(lParam);

  int imenu = 0;
  switch (nID) {
  case IDC_EDIT:
    {
      if (SciCall_IsSelectionEmpty() && (pt.x != -1) && (pt.y != -1)) {
        POINT ptc;
        ptc.x = pt.x;  ptc.y = pt.y;
        ScreenToClient(Globals.hwndEdit, &ptc);
        //~SciCall_GotoPos(SciCall_PositionFromPoint(ptc.x, ptc.y));
      }

      if (pt.x == -1 && pt.y == -1) {
        DocPos const iCurrentPos = SciCall_GetCurrentPos();
        pt.x = (LONG)SciCall_PointXFromPosition(iCurrentPos);
        pt.y = (LONG)SciCall_PointYFromPosition(iCurrentPos);
        ClientToScreen(Globals.hwndEdit, &pt);
      }

      DocLn const curLn = Sci_GetCurrentLineNumber();
      int const bitmask = SciCall_MarkerGet(curLn) & OCCURRENCE_MARKER_BITMASK() & ~(1 << MARKER_NP3_BOOKMARK);
      imenu = (bitmask && ((Settings.FocusViewMarkerMode & FVMM_LN_BACKGR) || !Settings.ShowBookmarkMargin)) ? 2 : 0;
    }
    break;

  case IDC_TOOLBAR:
  case IDC_STATUSBAR:
  case IDC_REBAR:
    {
      if ((pt.x == -1) && (pt.y == -1)) {
        GetCursorPos(&pt);
      }
      imenu = 1;
    }
    break;

  case IDC_MARGIN:
    {
      if ((pt.x == -1) && (pt.y == -1)) {
        GetCursorPos(&pt);
      }

      DocLn const curLn = Sci_GetCurrentLineNumber();
      int const bitmask = SciCall_MarkerGet(curLn) & OCCURRENCE_MARKER_BITMASK();
      EnableCmd(hMenuCtx, IDM_EDIT_CLEAR_MARKER, bitmask);
      EnableCmd(hMenuCtx, IDM_EDIT_CUT_MARKED, bitmask);
      EnableCmd(hMenuCtx, IDM_EDIT_COPY_MARKED, bitmask);
      EnableCmd(hMenuCtx, IDM_EDIT_DELETE_MARKED, bitmask);

      //DocLn const curLn = Sci_GetCurrentLineNumber();
      const SCNotification* const scn = (SCNotification*)wParam;
      switch (scn->margin) {
        case MARGIN_SCI_FOLDING:
          //[[fallthrough]];
        case MARGIN_SCI_LINENUM:
          //[[fallthrough]];
        case MARGIN_SCI_BOOKMRK:
          imenu = 2;
          break;
        default:
          imenu = 0;
          break;
      }
    }
    break;
  }

  TrackPopupMenuEx(GetSubMenu(hMenuCtx, imenu),
                   TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x + 1, pt.y + 1, hwnd, NULL);

  DestroyMenu(hMenuCtx);

  return (bMargin ? !0 : 0);
}

//=============================================================================
//
//  MsgChangeNotify() - Handles WM_CHANGENOTIFY
//
LRESULT MsgChangeNotify(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  UNUSED(wParam);
  UNUSED(lParam);

  DocPos const iCurPos = SciCall_GetCurrentPos();

  if (FileWatching.FileWatchingMode == FWM_MSGBOX || GetDocModified()) {
    SetForegroundWindow(hwnd);
  }

  if (PathIsExistingFile(Globals.CurrentFile))
  {
    bool bRevertFile = (FileWatching.FileWatchingMode == FWM_AUTORELOAD && !GetDocModified());

    if (!bRevertFile) {
      INT_PTR const answer = InfoBoxLng(MB_YESNO | MB_ICONWARNING, NULL, IDS_MUI_FILECHANGENOTIFY);
      bRevertFile = ((IDOK == answer) || (IDYES == answer));
    }

    if (bRevertFile) 
    {
      FileRevert(Globals.CurrentFile, /*Encoding_Changed(CPI_GET)*/false);
      if (FileWatching.MonitoringLog) 
      {
        SciCall_SetReadOnly(FileWatching.MonitoringLog);
        EditEnsureSelectionVisible();
      }
      else {
        Sci_GotoPosChooseCaret(iCurPos);
      }
    }
  }
  else {
    INT_PTR const answer = InfoBoxLng(MB_YESNO | MB_ICONWARNING, NULL, IDS_MUI_FILECHANGENOTIFY2);
    if ((IDOK == answer) || (IDYES == answer)) {
      FileSave(true, false, false, false, Flags.bPreserveFileModTime);
    }
  }

  if (!s_bRunningWatch) {
    InstallFileWatching(Globals.CurrentFile);
  }
  return FALSE;
}


//=============================================================================
//
//  MsgTrayMessage() - Handles WM_TRAYMESSAGE
//
//
LRESULT MsgTrayMessage(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  UNUSED(wParam);

  switch (lParam) 
  {
    case WM_RBUTTONUP:
    {
      HMENU hTrayMenu  = LoadMenu(Globals.hLngResContainer, MAKEINTRESOURCE(IDR_MUI_POPUPMENU));
      HMENU hMenuPopup = GetSubMenu(hTrayMenu, 3);

      POINT pt;
      int iCmd;

      SetForegroundWindow(hwnd);

      GetCursorPos(&pt);
      SetMenuDefaultItem(hMenuPopup, IDM_TRAY_RESTORE, false);
      iCmd = TrackPopupMenu(hMenuPopup,
        TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
        pt.x, pt.y, 0, hwnd, NULL);

      PostMessage(hwnd, WM_NULL, 0, 0);

      DestroyMenu(hTrayMenu);

      if (iCmd == IDM_TRAY_RESTORE) {
        ShowNotifyIcon(hwnd, false);
        RestoreWndFromTray(hwnd);
        ShowOwnedPopups(hwnd, true);
      }
      else if (iCmd == IDM_TRAY_EXIT) {
        //ShowNotifyIcon(hwnd,false);
        CloseApplication();
      }
    }
    break;

    case WM_LBUTTONUP:
      ShowNotifyIcon(hwnd, false);
      RestoreWndFromTray(hwnd);
      ShowOwnedPopups(hwnd, true);
      break;

    default:
      return TRUE;

  }
  return FALSE;
}


//=============================================================================
//
//  MsgEnterMenuLoop() - Handles WM_ENTERMENULOOP
//
//
LRESULT MsgEnterMenuLoop(HWND hwnd, WPARAM wParam)
{
  if ((BOOL)wParam == FALSE) // is main menu
  {
    HMENU const hCurMenu = GetMenu(hwnd);
    if (!hCurMenu)
    {
      SetMenu(hwnd, Globals.hMainMenu);
      DrawMenuBar(hwnd);
      //SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
  }
  return (LRESULT)wParam;
}



//=============================================================================
//
//  MsgExitMenuLoop() - Handles WM_EXITMENULOOP
//
//
LRESULT MsgExitMenuLoop(HWND hwnd, WPARAM wParam)
{
  if ((BOOL)wParam == FALSE) // is main menu
  {
    HMENU const hCurMenu = GetMenu(hwnd);
    if (hCurMenu && !Settings.ShowMenubar)
    {
      SetMenu(hwnd, NULL);
      DrawMenuBar(hwnd);
      //SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
  }
  return (LRESULT)wParam;
}


//=============================================================================
//
//  MsgInitMenu() - Handles WM_INITMENU
//
//
LRESULT MsgInitMenu(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  UNUSED(lParam);

  HMENU const hmenu = wParam ? (HMENU)wParam : GetMenu(hwnd);
  if (!hmenu) { return FALSE; }

  bool const sav = Globals.bCanSaveIniFile;
  bool const ro = SciCall_GetReadOnly(); // scintilla mode read-only
  bool const faro = s_bFileReadOnly;     // file attrib read-only
  DocPos const iCurPos = SciCall_GetCurrentPos();
  DocLn  const iCurLine = SciCall_LineFromPosition(iCurPos);
  bool const bPosInSel = Sci_IsPosInSelection(iCurPos);

  bool const pst = SciCall_CanPaste();
  bool const se = SciCall_IsSelectionEmpty();
  bool const mrs = Sci_IsMultiOrRectangleSelection();
  bool const cf = StrIsNotEmpty(Globals.CurrentFile);
  bool const te = Sci_IsDocEmpty();
  bool const mls = Sci_IsSelectionMultiLine();
  //bool const lfl = Flags.bLargeFileLoaded;

  EnableCmd(hmenu, IDM_FILE_REVERT, cf);
  EnableCmd(hmenu, CMD_RELOADASCIIASUTF8, cf);
  EnableCmd(hmenu, CMD_RELOADFORCEDETECTION, cf);
  EnableCmd(hmenu, CMD_RECODEANSI, cf);
  EnableCmd(hmenu, CMD_RECODEOEM, cf);
  EnableCmd(hmenu, CMD_RELOADNOFILEVARS, cf);
  EnableCmd(hmenu, CMD_RECODEDEFAULT, cf);
  EnableCmd(hmenu, CMD_RECODEGB18030, cf);
  EnableCmd(hmenu, IDM_FILE_LAUNCH, cf);

  SetUACIcon(hwnd, hmenu, IDM_FILE_LAUNCH_ELEVATED);
  CheckCmd(hmenu, IDM_FILE_LAUNCH_ELEVATED, s_bIsProcessElevated);
  EnableCmd(hmenu, IDM_FILE_LAUNCH_ELEVATED, !s_bIsProcessElevated);

  EnableCmd(hmenu, IDM_FILE_LAUNCH, cf);
  EnableCmd(hmenu, IDM_FILE_PROPERTIES, cf);
  EnableCmd(hmenu, IDM_FILE_CREATELINK, cf);
  EnableCmd(hmenu, IDM_FILE_ADDTOFAV, cf);

  EnableCmd(hmenu, IDM_FILE_READONLY, cf);
  EnableCmd(hmenu, IDM_EDIT_INSERT_FILENAME, cf);
  EnableCmd(hmenu, IDM_EDIT_INSERT_DIRNAME, cf);
  EnableCmd(hmenu, IDM_EDIT_INSERT_PATHNAME, cf);
  EnableCmd(hmenu, IDM_ENCODING_RECODE, cf);

  CheckCmd(hmenu, IDM_FILE_READONLY, faro);
  CheckCmd(hmenu, IDM_FILE_PRESERVE_FILEMODTIME, Flags.bPreserveFileModTime);

  EnableCmd(hmenu, IDM_ENCODING_UNICODEREV, !ro);
  EnableCmd(hmenu, IDM_ENCODING_UNICODE, !ro);
  EnableCmd(hmenu, IDM_ENCODING_UTF8SIGN, !ro);
  EnableCmd(hmenu, IDM_ENCODING_UTF8, !ro);
  EnableCmd(hmenu, IDM_ENCODING_ANSI, !ro);
  EnableCmd(hmenu, IDM_LINEENDINGS_CRLF, !ro);
  EnableCmd(hmenu, IDM_LINEENDINGS_LF, !ro);
  EnableCmd(hmenu, IDM_LINEENDINGS_CR, !ro);

  int i;

  if (Encoding_IsUNICODE_REVERSE(Encoding_GetCurrent())) {
    i = IDM_ENCODING_UNICODEREV;
  }
  else if (Encoding_IsUNICODE(Encoding_GetCurrent())) {
    i = IDM_ENCODING_UNICODE;
  }
  else if (Encoding_IsUTF8_SIGN(Encoding_GetCurrent())) {
    i = IDM_ENCODING_UTF8SIGN;
  }
  else if (Encoding_IsUTF8(Encoding_GetCurrent())) {
    i = IDM_ENCODING_UTF8;
  }
  else if (Encoding_IsANSI(Encoding_GetCurrent())) {
    i = IDM_ENCODING_ANSI;
  }
  else {
    i = -1;
  }
  CheckMenuRadioItem(hmenu,IDM_ENCODING_ANSI,IDM_ENCODING_UTF8SIGN,i,MF_BYCOMMAND);

  int const eol_mode = SciCall_GetEOLMode();
  if (eol_mode == SC_EOL_CRLF) {
    i = IDM_LINEENDINGS_CRLF;
  }
  else if (eol_mode == SC_EOL_CR) {
    i = IDM_LINEENDINGS_CR;
  }
  else {
    i = IDM_LINEENDINGS_LF;
  }
  CheckMenuRadioItem(hmenu,IDM_LINEENDINGS_CRLF,IDM_LINEENDINGS_LF,i,MF_BYCOMMAND);

  EnableCmd(hmenu, IDM_FILE_RECENT, (MRU_Count(Globals.pFileMRU) > 0));

  EnableCmd(hmenu, IDM_EDIT_UNDO, SciCall_CanUndo() && !ro);
  EnableCmd(hmenu, IDM_EDIT_REDO, SciCall_CanRedo() && !ro);

  EnableCmd(hmenu, IDM_EDIT_CUT, !te && !ro);       // allow Ctrl-X w/o selection
  EnableCmd(hmenu, IDM_EDIT_COPY, !te);             // allow Ctrl-C w/o selection

  EnableCmd(hmenu, IDM_EDIT_COPYALL, !te);
  EnableCmd(hmenu, IDM_EDIT_COPYADD, !te);

  EnableCmd(hmenu, IDM_EDIT_PASTE, pst && !ro);
  EnableCmd(hmenu, IDM_EDIT_SWAP, (!se || pst) && !ro);
  EnableCmd(hmenu, IDM_EDIT_CLEAR, !se && !ro);

  EnableCmd(hmenu, IDM_EDIT_SELECTALL, !te);
  EnableCmd(hmenu, IDM_EDIT_GOTOLINE, !te);

  OpenClipboard(hwnd);
  EnableCmd(hmenu, IDM_EDIT_CLEARCLIPBOARD, CountClipboardFormats());
  CloseClipboard();

  EnableCmd(hmenu, IDM_EDIT_MOVELINEUP, !ro);
  EnableCmd(hmenu, IDM_EDIT_MOVELINEDOWN, !ro);
  EnableCmd(hmenu, IDM_EDIT_DUPLINEORSELECTION, !ro);
  EnableCmd(hmenu, IDM_EDIT_LINETRANSPOSE, !ro);
  EnableCmd(hmenu, IDM_EDIT_CUTLINE, !ro);
  //EnableCmd(hmenu, IDM_EDIT_COPYLINE, true);
  EnableCmd(hmenu, IDM_EDIT_DELETELINE, !ro);

  EnableCmd(hmenu, IDM_EDIT_MERGEBLANKLINES, !ro);
  EnableCmd(hmenu, IDM_EDIT_MERGEEMPTYLINES, !ro);
  EnableCmd(hmenu, IDM_EDIT_REMOVEBLANKLINES, !ro);
  EnableCmd(hmenu, IDM_EDIT_REMOVEEMPTYLINES, !ro);
  EnableCmd(hmenu, IDM_EDIT_REMOVEDUPLICATELINES, !ro);

  EnableCmd(hmenu, IDM_EDIT_INDENT, !se && !ro);
  EnableCmd(hmenu, IDM_EDIT_UNINDENT, !se && !ro);

  EnableCmd(hmenu, CMD_JUMP2SELSTART, !se || mrs);
  EnableCmd(hmenu, CMD_JUMP2SELEND, !se || mrs);

  EnableCmd(hmenu, IDM_EDIT_PADWITHSPACES, !ro && !se);
  EnableCmd(hmenu, IDM_EDIT_STRIP1STCHAR, !ro && !se);
  EnableCmd(hmenu, IDM_EDIT_STRIPLASTCHAR, !ro && !se);
  EnableCmd(hmenu, IDM_EDIT_TRIMLINES, !ro);
  EnableCmd(hmenu, IDM_EDIT_COMPRESS_BLANKS, !ro);

  EnableCmd(hmenu, IDM_EDIT_MODIFYLINES, !ro);
  EnableCmd(hmenu, IDM_EDIT_ALIGN, mls && !ro);
  EnableCmd(hmenu, IDM_EDIT_SORTLINES, mls && !ro);

  //EnableCmd(hmenu,IDM_EDIT_COLUMNWRAP,i /*&& IsWindowsNT()*/);
  EnableCmd(hmenu, IDM_EDIT_SPLITLINES, !se && !ro);
  EnableCmd(hmenu, IDM_EDIT_JOINLINES, !se && !ro);
  EnableCmd(hmenu, IDM_EDIT_JOINLN_NOSP, !se && !ro);
  EnableCmd(hmenu, IDM_EDIT_JOINLINES_PARA, !se && !ro);

  EnableCmd(hmenu, IDM_EDIT_CONVERTUPPERCASE, !se && !ro);
  EnableCmd(hmenu, IDM_EDIT_CONVERTLOWERCASE, !se && !ro);
  EnableCmd(hmenu, IDM_EDIT_INVERTCASE, !se && !ro /*&& IsWindowsNT()*/);
  EnableCmd(hmenu, IDM_EDIT_TITLECASE, !se && !ro /*&& IsWindowsNT()*/);
  EnableCmd(hmenu, IDM_EDIT_SENTENCECASE, !se && !ro /*&& IsWindowsNT()*/);

  EnableCmd(hmenu, IDM_EDIT_CONVERTTABS, !se && !ro);
  EnableCmd(hmenu, IDM_EDIT_CONVERTSPACES, !se && !ro);
  EnableCmd(hmenu, IDM_EDIT_CONVERTTABS2, !se && !ro);
  EnableCmd(hmenu, IDM_EDIT_CONVERTSPACES2, !se && !ro);

  EnableCmd(hmenu, IDM_EDIT_URLENCODE, !se && !ro);
  EnableCmd(hmenu, IDM_EDIT_URLDECODE, !se && !ro);

  EnableCmd(hmenu, IDM_EDIT_ESCAPECCHARS, !se && !ro);
  EnableCmd(hmenu, IDM_EDIT_UNESCAPECCHARS, !se && !ro);

  EnableCmd(hmenu, IDM_EDIT_CHAR2HEX, !ro);  // Char2Hex allowed for char after current pos
  EnableCmd(hmenu, IDM_EDIT_HEX2CHAR, !se && !ro);

  EnableCmd(hmenu, IDM_VIEW_SHOWEXCERPT, !se);

  i = SciCall_GetLexer();

  EnableCmd(hmenu, IDM_EDIT_LINECOMMENT,
    !(i == SCLEX_NULL || i == SCLEX_CSS || i == SCLEX_DIFF || i == SCLEX_MARKDOWN || i == SCLEX_JSON) && !ro);

  EnableCmd(hmenu, IDM_EDIT_STREAMCOMMENT,
    !(i == SCLEX_NULL || i == SCLEX_VBSCRIPT || i == SCLEX_MAKEFILE || i == SCLEX_VB || i == SCLEX_ASM ||
      i == SCLEX_PERL || i == SCLEX_PYTHON || i == SCLEX_PROPERTIES || i == SCLEX_CONF ||
      i == SCLEX_POWERSHELL || i == SCLEX_BATCH || i == SCLEX_DIFF || i == SCLEX_BASH || i == SCLEX_TCL ||
      i == SCLEX_AU3 || i == SCLEX_LATEX || i == SCLEX_AHKL || i == SCLEX_RUBY || i == SCLEX_CMAKE || i == SCLEX_MARKDOWN ||
      i == SCLEX_YAML || i == SCLEX_REGISTRY || i == SCLEX_NIM || i == SCLEX_TOML) && !ro);

  EnableCmd(hmenu, CMD_INSERTNEWLINE, !ro);
  EnableCmd(hmenu, IDM_EDIT_INSERT_TAG, !ro);
  EnableCmd(hmenu, IDM_EDIT_INSERT_ENCODING, (Encoding_GetParseNames(Encoding_GetCurrent()) != NULL) && !ro);

  EnableCmd(hmenu, IDM_EDIT_INSERT_SHORTDATE, !ro);
  EnableCmd(hmenu, IDM_EDIT_INSERT_LONGDATE, !ro);

  EnableCmd(hmenu, IDM_EDIT_INSERT_GUID, !ro);

  EnableCmd(hmenu, IDM_EDIT_FIND, !te);
  EnableCmd(hmenu, IDM_EDIT_SAVEFIND, !te);
  EnableCmd(hmenu, IDM_EDIT_FINDNEXT, !te);
  EnableCmd(hmenu, IDM_EDIT_FINDPREV, !te);
  EnableCmd(hmenu, IDM_EDIT_REPLACE, !te && !ro);
  EnableCmd(hmenu, IDM_EDIT_REPLACENEXT, !te && !ro);
  EnableCmd(hmenu, IDM_EDIT_SELTONEXT, !te);
  EnableCmd(hmenu, IDM_EDIT_SELTOPREV, !te);
  EnableCmd(hmenu, IDM_EDIT_FINDMATCHINGBRACE, !te);
  EnableCmd(hmenu, IDM_EDIT_SELTOMATCHINGBRACE, !te);

  CheckCmd(hmenu, IDM_VIEW_SPLIT_UNDOTYPSEQ_LNBRK, Settings.SplitUndoTypingSeqOnLnBreak);

  EnableCmd(hmenu, BME_EDIT_BOOKMARKPREV, !te);
  EnableCmd(hmenu, BME_EDIT_BOOKMARKNEXT, !te);
  EnableCmd(hmenu, BME_EDIT_BOOKMARKTOGGLE, !te);
  EnableCmd(hmenu, BME_EDIT_BOOKMARKCLEAR, !te);

  EnableCmd(hmenu, IDM_EDIT_DELETELINELEFT, !te && !ro);
  EnableCmd(hmenu, IDM_EDIT_DELETELINERIGHT, !te && !ro);
  EnableCmd(hmenu, CMD_CTRLBACK, !te && !ro);
  EnableCmd(hmenu, CMD_CTRLDEL, !te && !ro);
  EnableCmd(hmenu, CMD_INSERT_TIMESTAMP, !ro);
  EnableCmd(hmenu, CMD_UPDATE_TIMESTAMPS, !te && !ro);

  EnableCmd(hmenu, IDM_VIEW_FONT, !IsWindow(Globals.hwndDlgCustomizeSchemes));
  EnableCmd(hmenu, IDM_VIEW_CURRENTSCHEME, !IsWindow(Globals.hwndDlgCustomizeSchemes));

  EnableCmd(hmenu, IDM_VIEW_FOLDING, FocusedView.CodeFoldingAvailable && !FocusedView.HideNonMatchedLines);
  bool const fd = (FocusedView.CodeFoldingAvailable && FocusedView.ShowCodeFolding);
  CheckCmd(hmenu, IDM_VIEW_FOLDING, fd);
  EnableCmd(hmenu, IDM_VIEW_TOGGLEFOLDS, !te && fd);
  EnableCmd(hmenu, CMD_FOLDJUMPDOWN, !te && fd);
  EnableCmd(hmenu, CMD_FOLDJUMPUP, !te && fd);
  EnableCmd(hmenu, CMD_FOLDCOLLAPSE, !te && fd);
  EnableCmd(hmenu, CMD_FOLDEXPAND, !te && fd);
  bool const bF = (SC_FOLDLEVELBASE < (SciCall_GetFoldLevel(iCurLine) & SC_FOLDLEVELNUMBERMASK));
  bool const bH = (SciCall_GetFoldLevel(iCurLine) & SC_FOLDLEVELHEADERFLAG);
  EnableCmd(hmenu, IDM_VIEW_TOGGLE_CURRENT_FOLD, !te && fd && (bF || bH));
  CheckCmdPos(GetSubMenu(GetMenu(Globals.hwndMain), 2), 18, fd);

  CheckCmd(hmenu, IDM_VIEW_USE2NDDEFAULT, Style_GetUse2ndDefault());

  CheckCmd(hmenu, IDM_VIEW_WORDWRAP, Globals.fvCurFile.bWordWrap);
  CheckCmd(hmenu, IDM_VIEW_LONGLINEMARKER, Settings.MarkLongLines);
  CheckCmd(hmenu, IDM_VIEW_TABSASSPACES, Globals.fvCurFile.bTabsAsSpaces);
  CheckCmd(hmenu, IDM_VIEW_SHOWINDENTGUIDES, Settings.ShowIndentGuides);
  CheckCmd(hmenu, IDM_VIEW_AUTOINDENTTEXT, Settings.AutoIndent);
  CheckCmd(hmenu, IDM_VIEW_LINENUMBERS, Settings.ShowLineNumbers);
  CheckCmd(hmenu, IDM_VIEW_BOOKMARK_MARGIN, Settings.ShowBookmarkMargin);
  CheckCmd(hmenu, IDM_VIEW_CHASING_DOCTAIL, FileWatching.MonitoringLog);

  EnableCmd(hmenu, IDM_EDIT_COMPLETEWORD, !te && !ro);
  CheckCmd(hmenu, IDM_VIEW_AUTOCOMPLETEWORDS, Settings.AutoCompleteWords && !ro);
  CheckCmd(hmenu, IDM_VIEW_AUTOCLEXKEYWORDS, Settings.AutoCLexerKeyWords && !ro);

  CheckCmd(hmenu, IDM_VIEW_ACCELWORDNAV, Settings.AccelWordNavigation);
  CheckCmd(hmenu, IDM_VIEW_EDIT_LINECOMMENT, Settings.EditLineCommentBlock);

  CheckCmd(hmenu, IDM_VIEW_MARKOCCUR_ONOFF, IsMarkOccurrencesEnabled());
  CheckCmd(hmenu, IDM_VIEW_MARKOCCUR_BOOKMARKS, Settings.MarkOccurrencesBookmark);
  CheckCmd(hmenu, IDM_VIEW_MARKOCCUR_VISIBLE, Settings.MarkOccurrencesMatchVisible);
  CheckCmd(hmenu, IDM_VIEW_MARKOCCUR_CASE, Settings.MarkOccurrencesMatchCase);

  EnableCmd(hmenu, IDM_VIEW_TOGGLE_VIEW, IsFocusedViewAllowed());
  CheckCmd(hmenu, IDM_VIEW_TOGGLE_VIEW, FocusedView.HideNonMatchedLines);

  i = IDM_VIEW_FV_FOLD;
  int const fvm_mode = Settings.FocusViewMarkerMode;
  if (fvm_mode == FVMM_MARGIN) {
    i = IDM_VIEW_FV_BOOKMARK;
  } else if (fvm_mode == FVMM_LN_BACKGR) {
    i = IDM_VIEW_FV_HIGHLIGHT;
  } else if (fvm_mode == (FVMM_MARGIN | FVMM_FOLD)) {
    i = IDM_VIEW_FV_BKMRKFOLD;
  } else if (fvm_mode == (FVMM_LN_BACKGR | FVMM_FOLD)) {
    i = IDM_VIEW_FV_HIGHLGFOLD;
  } else {
    i = IDM_VIEW_FV_FOLD;
  }
  CheckMenuRadioItem(hmenu, IDM_VIEW_FV_FOLD, IDM_VIEW_FV_HIGHLGFOLD, i, MF_BYCOMMAND);

  CheckCmd(hmenu, IDM_VIEW_HYPERLINKHOTSPOTS, Settings.HyperlinkHotspot);
  
  i = IDM_VIEW_COLORDEFHOTSPOTS + Settings.ColorDefHotspot;
  CheckMenuRadioItem(hmenu, IDM_VIEW_COLORDEFHOTSPOTS, IDM_VIEW_COLOR_BGRA, i, MF_BYCOMMAND);
  CheckCmdPos(GetSubMenu(GetMenu(Globals.hwndMain), 2), 9, IsColorDefHotspotEnabled());

  CheckCmd(hmenu, IDM_VIEW_UNICODE_POINTS, Settings.HighlightUnicodePoints);
  CheckCmd(hmenu, IDM_VIEW_MATCHBRACES, Settings.MatchBraces);
  
  i = IDM_VIEW_HILITCURLN_NONE + Settings.HighlightCurrentLine;
  CheckMenuRadioItem(hmenu, IDM_VIEW_HILITCURLN_NONE, IDM_VIEW_HILITCURLN_FRAME, i, MF_BYCOMMAND);
  CheckCmdPos(GetSubMenu(GetMenu(Globals.hwndMain), 2), 12, (i != IDM_VIEW_HILITCURLN_NONE));

  EnableCmd(hmenu, IDM_VIEW_WIN_DARK_MODE, IsDarkModeSupported());
  CheckCmd(hmenu, IDM_VIEW_WIN_DARK_MODE, Settings.WinThemeDarkMode);

  // --------------------------------------------------------------------------

  int const mnuMain = 2;
  int const mnuSubOcc = 13;
  int const mnuSubSubWord = 6;

  if (Settings.MarkOccurrencesMatchWholeWords) {
    i = IDM_VIEW_MARKOCCUR_WORD;
  }
  else if (Settings.MarkOccurrencesCurrentWord) {
    i = IDM_VIEW_MARKOCCUR_CURRENT;
  }
  else {
    i = IDM_VIEW_MARKOCCUR_WNONE;
  }
  CheckMenuRadioItem(hmenu, IDM_VIEW_MARKOCCUR_WNONE, IDM_VIEW_MARKOCCUR_CURRENT, i, MF_BYCOMMAND);
  CheckCmdPos(GetSubMenu(GetSubMenu(GetMenu(Globals.hwndMain), mnuMain), mnuSubOcc), mnuSubSubWord, (i != IDM_VIEW_MARKOCCUR_WNONE));
  
  i = IsMarkOccurrencesEnabled();
  EnableCmd(hmenu, IDM_VIEW_MARKOCCUR_VISIBLE, i);
  EnableCmd(hmenu, IDM_VIEW_MARKOCCUR_CASE, i);
  EnableCmd(hmenu, IDM_VIEW_MARKOCCUR_WNONE, i);
  EnableCmd(hmenu, IDM_VIEW_MARKOCCUR_WORD, i);
  EnableCmd(hmenu, IDM_VIEW_MARKOCCUR_CURRENT, i);
  EnableCmdPos(GetSubMenu(GetSubMenu(GetMenu(Globals.hwndMain), mnuMain), mnuSubOcc), mnuSubSubWord, i);
  CheckCmdPos(GetSubMenu(GetMenu(Globals.hwndMain), mnuMain), mnuSubOcc, i);

  // --------------------------------------------------------------------------

  CheckCmd(hmenu, IDM_VIEW_SHOWBLANKS, Settings.ViewWhiteSpace);
  CheckCmd(hmenu, IDM_VIEW_SHOWEOLS, Settings.ViewEOLs);
  CheckCmd(hmenu, IDM_VIEW_WORDWRAPSYMBOLS, Settings.ShowWordWrapSymbols);
  CheckCmd(hmenu, IDM_VIEW_MENUBAR, Settings.ShowMenubar);
  CheckCmd(hmenu, IDM_VIEW_TOOLBAR, Settings.ShowToolbar);
  EnableCmd(hmenu, IDM_VIEW_CUSTOMIZETB, Settings.ShowToolbar);
  CheckCmd(hmenu, IDM_VIEW_STATUSBAR, Settings.ShowStatusbar);
  CheckCmd(hmenu, IDM_VIEW_DPISCALETB, Settings.DpiScaleToolBar);

  //i = SciCall_GetLexer();
  //EnableCmd(hmenu,IDM_VIEW_AUTOCLOSETAGS,(i == SCLEX_HTML || i == SCLEX_XML));
  CheckCmd(hmenu, IDM_VIEW_AUTOCLOSETAGS, Settings.AutoCloseTags /*&& (i == SCLEX_HTML || i == SCLEX_XML)*/);

  CheckCmd(hmenu, IDM_VIEW_SHOW_HYPLNK_CALLTIP, Settings.ShowHypLnkToolTip);
  CheckCmd(hmenu, IDM_VIEW_SCROLLPASTEOF, Settings.ScrollPastEOF);

  CheckCmd(hmenu, IDM_VIEW_REUSEWINDOW, Flags.bReuseWindow);
  EnableCmd(hmenu, IDM_VIEW_REUSEWINDOW, sav);
  CheckCmd(hmenu, IDM_VIEW_SINGLEFILEINSTANCE, Flags.bSingleFileInstance);
  EnableCmd(hmenu, IDM_VIEW_SINGLEFILEINSTANCE, sav);
  CheckCmd(hmenu, IDM_VIEW_STICKYWINPOS, Flags.bStickyWindowPosition);
  EnableCmd(hmenu, IDM_VIEW_STICKYWINPOS, sav);
  EnableCmd(hmenu, CMD_SAVEASDEFWINPOS, sav);

  CheckCmd(hmenu, IDM_VIEW_ALWAYSONTOP, ((Settings.AlwaysOnTop || s_flagAlwaysOnTop == 2) && s_flagAlwaysOnTop != 1));
  CheckCmd(hmenu, IDM_VIEW_MINTOTRAY, Settings.MinimizeToTray);
  CheckCmd(hmenu, IDM_VIEW_TRANSPARENT, Settings.TransparentMode);

  bool const dwr = (Settings.RenderingTechnology > SC_TECHNOLOGY_DEFAULT);
  //bool const gdi = ((Settings.RenderingTechnology % SC_TECHNOLOGY_DIRECTWRITEDC) == 0);
  
  i = IDM_SET_RENDER_TECH_GDI + Settings.RenderingTechnology;
  CheckMenuRadioItem(hmenu, IDM_SET_RENDER_TECH_GDI, IDM_SET_RENDER_TECH_D2DDC, i, MF_BYCOMMAND);
  
  CheckCmd(hmenu, IDM_SET_RTL_LAYOUT_EDIT, Settings.EditLayoutRTL);
  //~EnableCmd(hmenu, IDM_SET_RTL_LAYOUT_EDIT, gdi);
  CheckCmd(hmenu, IDM_SET_RTL_LAYOUT_DLG, Settings.DialogsLayoutRTL);

  if (dwr) {
    i = IDM_SET_BIDIRECTIONAL_NONE + Settings.Bidirectional;
    CheckMenuRadioItem(hmenu, IDM_SET_BIDIRECTIONAL_NONE, IDM_SET_BIDIRECTIONAL_R2L, i, MF_BYCOMMAND);
  }
  else {
    i = IDM_SET_BIDIRECTIONAL_NONE;
    CheckMenuRadioItem(hmenu, IDM_SET_BIDIRECTIONAL_NONE, IDM_SET_BIDIRECTIONAL_R2L, i, MF_BYCOMMAND);
  }
  EnableCmd(hmenu, IDM_SET_BIDIRECTIONAL_NONE, dwr);
  EnableCmd(hmenu, IDM_SET_BIDIRECTIONAL_L2R, dwr);
  EnableCmd(hmenu, IDM_SET_BIDIRECTIONAL_R2L, dwr);


  CheckCmd(hmenu, IDM_VIEW_MUTE_MESSAGEBEEP, Settings.MuteMessageBeep);
  CheckCmd(hmenu, IDM_VIEW_SAVEBEFORERUNNINGTOOLS, Settings.SaveBeforeRunningTools);
  //~EnableCmd(hmenu, IDM_VIEW_SAVEBEFORERUNNINGTOOLS, !faro);

  CheckCmd(hmenu, IDM_VIEW_NOSAVERECENT, Settings.SaveRecentFiles);
  EnableCmd(hmenu, IDM_VIEW_NOSAVERECENT, sav);
  CheckCmd(hmenu, IDM_VIEW_NOPRESERVECARET, Settings.PreserveCaretPos);
  EnableCmd(hmenu, IDM_VIEW_NOPRESERVECARET, Settings.SaveRecentFiles && sav);
  CheckCmd(hmenu, IDM_VIEW_NOSAVEFINDREPL, Settings.SaveFindReplace);
  EnableCmd(hmenu, IDM_VIEW_NOSAVEFINDREPL, sav);

  CheckCmd(hmenu, IDM_VIEW_EVALTINYEXPRONSEL, Settings.EvalTinyExprOnSelection);
  
  CheckCmd(hmenu, IDM_VIEW_CHANGENOTIFY, Settings.FileWatchingMode);

  if (StrIsNotEmpty(s_wchTitleExcerpt))
    i = IDM_VIEW_SHOWEXCERPT;
  else if (Settings.PathNameFormat == 0)
    i = IDM_VIEW_SHOWFILENAMEONLY;
  else if (Settings.PathNameFormat == 1)
    i = IDM_VIEW_SHOWFILENAMEFIRST;
  else
    i = IDM_VIEW_SHOWFULLPATH;
  CheckMenuRadioItem(hmenu, IDM_VIEW_SHOWFILENAMEONLY, IDM_VIEW_SHOWEXCERPT, i, MF_BYCOMMAND);

  if (Settings.EscFunction == 1)
    i = IDM_VIEW_ESCMINIMIZE;
  else if (Settings.EscFunction == 2)
    i = IDM_VIEW_ESCEXIT;
  else
    i = IDM_VIEW_NOESCFUNC;
  CheckMenuRadioItem(hmenu, IDM_VIEW_NOESCFUNC, IDM_VIEW_ESCEXIT, i, MF_BYCOMMAND);

  bool const bIsHLink = (SciCall_IndicatorValueAt(INDIC_NP3_HYPERLINK, iCurPos) > 0);
  EnableCmd(hmenu, CMD_OPEN_HYPERLINK, !mrs && bIsHLink);
  EnableCmd(hmenu, CMD_WEBACTION1, !se && !mrs && bPosInSel && !bIsHLink);
  EnableCmd(hmenu, CMD_WEBACTION2, !se && !mrs && bPosInSel && !bIsHLink);

  i = (int)StrIsNotEmpty(Settings2.AdministrationTool);
  EnableCmd(hmenu, IDM_HELP_ADMINEXE, i);

  for (int lng = 0; lng < MuiLanguages_CountOf(); ++lng) {
    //EnableCmd(hmenu, MUI_LanguageDLLs[lng].rid, MUI_LanguageDLLs[lng].bHasDLL);
    CheckCmd(hmenu, MUI_LanguageDLLs[lng].rid, MUI_LanguageDLLs[lng].bIsActive);
  }

  UpdateSaveSettingsCmds();
  
  DrawMenuBar(hwnd);

  return FALSE;
}


//=============================================================================
//
//  _DynamicLanguageMenuCmd() - Handles IDS_MUI_LANG_XX_YY messages
//
//
static void _DynamicLanguageMenuCmd(int cmd)
{
  int const iLngIdx = (cmd - IDS_MUI_LANG_EN_US); // consecutive IDs

  if ((iLngIdx < 0) || (iLngIdx >= MuiLanguages_CountOf())) {
    return;
  }
  if (!MUI_LanguageDLLs[iLngIdx].bIsActive)
  {
    CloseNonModalDialogs();

    DestroyMenu(Globals.hMainMenu);
    
    // desired language
    LANGID const desiredLngID = MUI_LanguageDLLs[iLngIdx].LangId;
    SetPreferredLanguage(desiredLngID);

    FreeLanguageResources();
    LoadLanguageResources();

    Globals.hMainMenu = LoadMenu(Globals.hLngResContainer, MAKEINTRESOURCE(IDR_MUI_MAINMENU));
    if (!Globals.hMainMenu) {
      MsgBoxLastError(L"LoadMenu()", 0);
      CloseApplication();
      return;
    }

    _InsertLanguageMenu(Globals.hMainMenu);
    Style_InsertThemesMenu(Globals.hMainMenu);
    SetMenu(Globals.hwndMain, (Settings.ShowMenubar ? Globals.hMainMenu : NULL));
    DrawMenuBar(Globals.hwndMain);

    MsgThemeChanged(Globals.hwndMain, (WPARAM)NULL, (LPARAM)NULL);
  }
  return;
}


//=============================================================================
//
//  MsgCommand() - Handles WM_COMMAND
//

static   WCHAR tchMaxPathBuffer[MAX_PATH] = { L'\0' };

LRESULT MsgCommand(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  int const iLoWParam = LOWORD(wParam);
 
  bool const bIsLngMenuCmd = ((iLoWParam >= IDS_MUI_LANG_EN_US) && (iLoWParam < (IDS_MUI_LANG_EN_US + MuiLanguages_CountOf())));
  if (bIsLngMenuCmd) {
    _DynamicLanguageMenuCmd(iLoWParam);
    UpdateToolbar();
    UpdateStatusbar(true);
    UpdateMarginWidth();
    return FALSE;
  }

  bool const bIsThemesMenuCmd = ((iLoWParam >= IDM_THEMES_DEFAULT) && (iLoWParam < (int)(IDM_THEMES_DEFAULT + ThemeItems_CountOf())));
  if (bIsThemesMenuCmd) {
    Style_DynamicThemesMenuCmd(iLoWParam);
    UpdateToolbar();
    UpdateStatusbar(true);
    UpdateMarginWidth();
    return FALSE;
  }

  switch(iLoWParam)
  {
    case SCEN_CHANGE:
      EditUpdateVisibleIndicators();
      MarkAllOccurrences(Settings2.UpdateDelayMarkAllOccurrences, false);
      break;

    case IDT_TIMER_UPDATE_STATUSBAR:
      _UpdateStatusbarDelayed((bool)lParam);
      break;

    case IDT_TIMER_UPDATE_TOOLBAR:
      _UpdateToolbarDelayed();
      break;

    case IDT_TIMER_MAIN_MRKALL:
      EditMarkAllOccurrences(Globals.hwndEdit, (bool)lParam);
      break;

    case IDT_TIMER_CLEAR_CALLTIP:
      CancelCallTip();
      break;

    case IDT_TIMER_UNDO_TRANSACTION:
      _SplitUndoTransaction((int)lParam);
      break;


    case IDM_FILE_NEW:
      FileLoad(false,true,false,Settings.SkipUnicodeDetection,Settings.SkipANSICodePageDetection, false, L"");
      break;


    case IDM_FILE_OPEN:
      FileLoad(false,false,false,Settings.SkipUnicodeDetection,Settings.SkipANSICodePageDetection, false, L"");
      break;


    case IDM_FILE_REVERT:
      if (GetDocModified()) {
        INT_PTR const answer = InfoBoxLng(MB_YESNO | MB_ICONQUESTION, NULL, IDS_MUI_ASK_REVERT);
        if (!((IDOK == answer) || (IDYES == answer))) {
          break;
        }
        //~ don't revert if no save needed
        //~FileRevert(Globals.CurrentFile, Encoding_Changed(CPI_GET)); 
      }
      // revert in any case (manually forced)
      FileRevert(Globals.CurrentFile, /*Encoding_Changed(CPI_GET)*/true);
      break;


    case IDM_FILE_SAVE:
      FileSave(true, false, false, false, Flags.bPreserveFileModTime);
      break;


    case IDM_FILE_SAVEAS:
      FileSave(true, false, true, false, Flags.bPreserveFileModTime);
      break;


    case IDM_FILE_SAVECOPY:
      FileSave(true, false, true, true, Flags.bPreserveFileModTime);
      break;


    case IDM_FILE_PRESERVE_FILEMODTIME:
      if (!Flags.bPreserveFileModTime) {
        InfoBoxLng(MB_OK, L"PreserveFileModTime", IDS_MUI_INF_PRSVFILEMODTM);
      }
      Flags.bPreserveFileModTime = true;
      FileSave(true, false, false, false, Flags.bPreserveFileModTime);
      break;


    case IDM_FILE_READONLY:
      if (StrIsNotEmpty(Globals.CurrentFile))
      {
        DWORD dwFileAttributes = GetFileAttributes(Globals.CurrentFile);
        if (dwFileAttributes != INVALID_FILE_ATTRIBUTES) {
          if (s_bFileReadOnly) {
            dwFileAttributes = (dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);
          }
          else {
            dwFileAttributes |= FILE_ATTRIBUTE_READONLY;
          }
          if (!SetFileAttributes(Globals.CurrentFile, dwFileAttributes)) {
            InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_READONLY_MODIFY, PathFindFileName(Globals.CurrentFile));
          }
        }
        else {
          InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_READONLY_MODIFY, PathFindFileName(Globals.CurrentFile));
        }

        s_bFileReadOnly = IsReadOnly(GetFileAttributes(Globals.CurrentFile)); // ensure setting

        if (Flags.bSettingsFileSoftLocked) {
          Globals.bCanSaveIniFile = CanAccessPath(Globals.IniFile, GENERIC_WRITE);
          UpdateSaveSettingsCmds();
        }
        UpdateToolbar();
      }
      break;


    case IDM_FILE_BROWSE:
        DialogFileBrowse(hwnd);
      break;


    case IDM_GREP_WIN_SEARCH:
      {
        WCHAR wchBuffer[MIDSZ_BUFFER] = { L'\0' };
        EditGetSelectedText(wchBuffer, COUNTOF(wchBuffer));
        DialogGrepWin(hwnd, wchBuffer);
      }
      break;


    case IDM_FILE_NEWWINDOW:
    case IDM_FILE_NEWWINDOW2:
      SaveAllSettings(false);
      LPCWSTR lpcwFilePath = (iLoWParam != IDM_FILE_NEWWINDOW2) ? Globals.CurrentFile : NULL;
      DialogNewWindow(hwnd, Settings.SaveBeforeRunningTools, lpcwFilePath);
      break;


    case IDM_FILE_LAUNCH:
      {
        if (StrIsEmpty(Globals.CurrentFile))
          break;

        if (Settings.SaveBeforeRunningTools && !FileSave(false,true,false,false,Flags.bPreserveFileModTime))
          break;

        StringCchCopy(tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer),Globals.CurrentFile);
        PathCchRemoveFileSpec(tchMaxPathBuffer, COUNTOF(tchMaxPathBuffer));

        SHELLEXECUTEINFO sei;
        ZeroMemory(&sei,sizeof(SHELLEXECUTEINFO));
        sei.cbSize = sizeof(SHELLEXECUTEINFO);
        sei.fMask = 0;
        sei.hwnd = hwnd;
        sei.lpVerb = NULL;
        sei.lpFile = Globals.CurrentFile;
        sei.lpParameters = NULL;
        sei.lpDirectory = tchMaxPathBuffer;
        sei.nShow = SW_SHOWNORMAL;
        ShellExecuteEx(&sei);
      }
      break;


    case IDM_FILE_EXPLORE_DIR:
    {
      if (Settings.SaveBeforeRunningTools && !FileSave(false, true, false, false, Flags.bPreserveFileModTime)) {
        break;
      }
      PIDLIST_ABSOLUTE pidl = NULL;
      DWORD rfg = 0;
      SHILCreateFromPath(StrIsEmpty(Globals.CurrentFile) ? Globals.WorkingDirectory : Globals.CurrentFile, &pidl, &rfg);
      if (pidl) {
        SHOpenFolderAndSelectItems(pidl, 0, NULL, 0);
        ILFree(pidl);
      }
    }
    break;


    case IDM_FILE_LAUNCH_ELEVATED:
      {
        EditFileIOStatus fioStatus = INIT_FILEIO_STATUS;
        fioStatus.iEncoding        = Encoding_GetCurrent();
        fioStatus.iEOLMode         = SciCall_GetEOLMode();

        if (DoElevatedRelaunch(&fioStatus, false))
        {
          CloseApplication();
        }
        else {
          InfoBoxLng(MB_ICONSHIELD, NULL, IDS_MUI_ERR_ELEVATED_RIGHTS);
        }
      }
      break;


    case IDM_FILE_RUN:
      {
        if (Settings.SaveBeforeRunningTools && !FileSave(false, true, false, false, Flags.bPreserveFileModTime)) {
          break;
        }
        StringCchCopy(tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer),Globals.CurrentFile);
        PathQuoteSpaces(tchMaxPathBuffer);

        RunDlg(hwnd,tchMaxPathBuffer);
      }
      break;

    case IDM_FILE_OPENWITH:
      if (Settings.SaveBeforeRunningTools && !FileSave(false, true, false, false, Flags.bPreserveFileModTime))
        break;
      OpenWithDlg(hwnd,Globals.CurrentFile);
      break;


    case IDM_FILE_PAGESETUP:
      EditPrintSetup(Globals.hwndEdit);
      break;

    case IDM_FILE_PRINT:
      {
        WCHAR *pszTitle;
        WCHAR tchUntitled[32] = { L'\0' };
        WCHAR tchPageFmt[32] = { L'\0' };
        WCHAR szDisplayName[MAX_PATH];

        if (StrIsNotEmpty(Globals.CurrentFile))
        {
          PathGetDisplayName(szDisplayName, COUNTOF(szDisplayName), Globals.CurrentFile);
          pszTitle = szDisplayName;
        }
        else {
          GetLngString(IDS_MUI_UNTITLED, tchUntitled, COUNTOF(tchUntitled));
          pszTitle = tchUntitled;
        }

        GetLngString(IDS_MUI_PRINT_PAGENUM,tchPageFmt,COUNTOF(tchPageFmt));

        if (!EditPrint(Globals.hwndEdit, pszTitle, tchPageFmt)) {
          InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_PRINT_ERROR, pszTitle);
        }
      }
      break;


    case IDM_FILE_PROPERTIES:
      {
        if (StrIsEmpty(Globals.CurrentFile))
          break;

        SHELLEXECUTEINFO sei;
        ZeroMemory(&sei,sizeof(SHELLEXECUTEINFO));
        sei.cbSize = sizeof(SHELLEXECUTEINFO);
        sei.fMask = SEE_MASK_INVOKEIDLIST;
        sei.hwnd = hwnd;
        sei.lpVerb = L"properties";
        sei.lpFile = Globals.CurrentFile;
        sei.nShow = SW_SHOWNORMAL;
        ShellExecuteEx(&sei);
      }
      break;

    case IDM_FILE_CREATELINK:
      {
        if (StrIsEmpty(Globals.CurrentFile)) {
          break;
        }
        if (!PathCreateDeskLnk(Globals.CurrentFile)) {
          InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_ERR_CREATELINK);
        }
      }
      break;


    case IDM_FILE_OPENFAV:
      if (FileSave(false, true, false, false, Flags.bPreserveFileModTime)) {
        if (FavoritesDlg(hwnd,tchMaxPathBuffer))
        {
          if (PathIsLnkToDirectory(tchMaxPathBuffer,NULL,0))
            PathGetLnkPath(tchMaxPathBuffer,tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer));

          if (PathIsDirectory(tchMaxPathBuffer))
          {
            if (OpenFileDlg(Globals.hwndMain, tchMaxPathBuffer, COUNTOF(tchMaxPathBuffer), tchMaxPathBuffer))
              FileLoad(true, false, false, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, false, tchMaxPathBuffer);
          }
          else
            FileLoad(true, false, false, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, false, tchMaxPathBuffer);
          }
        }
      break;


    case IDM_FILE_ADDTOFAV:
      if (StrIsNotEmpty(Globals.CurrentFile)) {
        WCHAR szDisplayName[MAX_PATH];
        PathGetDisplayName(szDisplayName, COUNTOF(szDisplayName), Globals.CurrentFile);
        AddToFavDlg(hwnd, szDisplayName, Globals.CurrentFile);
      }
      break;


    case IDM_FILE_MANAGEFAV:
      {
        SHELLEXECUTEINFO sei;
        ZeroMemory(&sei,sizeof(SHELLEXECUTEINFO));
        sei.cbSize = sizeof(SHELLEXECUTEINFO);
        sei.fMask = 0;
        sei.hwnd = hwnd;
        sei.lpVerb = NULL;
        sei.lpFile = Settings.FavoritesDir;
        sei.lpParameters = NULL;
        sei.lpDirectory = NULL;
        sei.nShow = SW_SHOWNORMAL;
        // Run favorites directory
        ShellExecuteEx(&sei);
      }
      break;


    case IDM_FILE_RECENT:
      if (MRU_Count(Globals.pFileMRU) > 0) {
        if (FileSave(false, true, false, false, Flags.bPreserveFileModTime)) {
          WCHAR tchFile[MAX_PATH] = { L'\0' };
          if (FileMRUDlg(hwnd, tchFile)) {
            FileLoad(true, false, false, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, false, tchFile);
          }
        }
      }
      break;


    case IDM_FILE_EXIT:
      CloseApplication();
      break;


    case IDM_ENCODING_ANSI:
    case IDM_ENCODING_UNICODE:
    case IDM_ENCODING_UNICODEREV:
    case IDM_ENCODING_UTF8:
    case IDM_ENCODING_UTF8SIGN:
    case IDM_ENCODING_SELECT:
      {
        cpi_enc_t iNewEncoding = (HIWORD(wParam) >= IDM_ENCODING_SELECT) ? 
          (cpi_enc_t)(HIWORD(wParam) - IDM_ENCODING_SELECT) : Encoding_GetCurrent();

        if (iLoWParam == IDM_ENCODING_SELECT) {
          if ((HIWORD(wParam) < IDM_ENCODING_SELECT) && !SelectEncodingDlg(hwnd, &iNewEncoding)) {
            break; // no change
          }
        }
        else {
          switch (iLoWParam)
          {
          case IDM_ENCODING_UNICODE:    iNewEncoding = CPI_UNICODEBOM; break;
          case IDM_ENCODING_UNICODEREV: iNewEncoding = CPI_UNICODEBEBOM; break;
          case IDM_ENCODING_UTF8:       iNewEncoding = CPI_UTF8; break;
          case IDM_ENCODING_UTF8SIGN:   iNewEncoding = CPI_UTF8SIGN; break;
          case IDM_ENCODING_ANSI:       iNewEncoding = CPI_ANSI_DEFAULT; break;
          }
        }
        BeginWaitCursor(true,L"Recoding...");
        if (EditSetNewEncoding(Globals.hwndEdit, iNewEncoding, (s_flagSetEncoding != CPI_NONE)))
        {
          SetSaveNeeded();
        }
        EndWaitCursor();
        UpdateToolbar();
        UpdateStatusbar(false);
      }
      break;


    case IDM_ENCODING_RECODE:
      {
        if (StrIsNotEmpty(Globals.CurrentFile))
        {
          cpi_enc_t iNewEncoding = Encoding_MapSignature(Encoding_GetCurrent());

          if (GetDocModified()) {
            INT_PTR const answer = InfoBoxLng(MB_YESNO | MB_ICONQUESTION, NULL, IDS_MUI_ASK_RECODE);
            if (!((IDOK == answer) || (IDYES == answer))) {
              break;
            }
          }

          if (RecodeDlg(hwnd,&iNewEncoding)) 
          {
            StringCchCopy(tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer),Globals.CurrentFile);
            Encoding_Forced(iNewEncoding);
            FileLoad(true, false, true, true, true, false, tchMaxPathBuffer);
          }
        }
      }
      break;


    case IDM_ENCODING_SETDEFAULT:
      SelectDefEncodingDlg(hwnd, &Settings.DefaultEncoding);
      UpdateToolbar();
      UpdateStatusbar(false);
      break;


    case IDM_LINEENDINGS_CRLF:
    case IDM_LINEENDINGS_CR:
    case IDM_LINEENDINGS_LF:
      {
        BeginWaitCursor(true,L"Line Breaks...");
        _IGNORE_NOTIFY_CHANGE_;
        int const _eol_mode = (iLoWParam - IDM_LINEENDINGS_CRLF); // SC_EOL_CRLF(0), SC_EOL_CR(1), SC_EOL_LF(2)
        SciCall_SetEOLMode(_eol_mode);
        EditEnsureConsistentLineEndings(Globals.hwndEdit);
        _OBSERVE_NOTIFY_CHANGE_;
        EndWaitCursor();
        UpdateToolbar();
        UpdateStatusbar(false);
      }
      break;


    case IDM_LINEENDINGS_SETDEFAULT:
        SelectDefLineEndingDlg(hwnd, (LPARAM)&Settings.DefaultEOLMode);
      break;


    case IDM_EDIT_UNDO:
      if (SciCall_CanUndo()) {
        _IGNORE_NOTIFY_CHANGE_;
        SciCall_Undo();
        _OBSERVE_NOTIFY_CHANGE_;
      }
      break;


    case IDM_EDIT_REDO:
      if (SciCall_CanRedo()) {
        _IGNORE_NOTIFY_CHANGE_;
        SciCall_Redo();
        _OBSERVE_NOTIFY_CHANGE_;
      }
      break;


    case IDM_EDIT_CUT:
      {
        if (s_flagPasteBoard) {
          s_bLastCopyFromMe = true;
        }
        if (SciCall_IsSelectionEmpty())
        {
          if (!Settings2.NoCutLineOnEmptySelection) {
            _BEGIN_UNDO_ACTION_;
            SciCall_MarkerDelete(Sci_GetCurrentLineNumber(), -1);
            SciCall_LineCut();
            _END_UNDO_ACTION_;
          }
        }
        else {
          _BEGIN_UNDO_ACTION_;
          SciCall_Cut();
          _END_UNDO_ACTION_;
        }
      }
      break;


    case IDM_EDIT_CUTLINE:
    {
      if (s_flagPasteBoard) {
        s_bLastCopyFromMe = true;
      }
      _BEGIN_UNDO_ACTION_;
        SciCall_MarkerDelete(Sci_GetCurrentLineNumber(), -1);
        SciCall_LineCut();
      _END_UNDO_ACTION_;
    }
    break;


    case IDM_EDIT_COPY:
      {
        if (s_flagPasteBoard) {
          s_bLastCopyFromMe = true;
        }
        if (SciCall_IsSelectionEmpty()) {
          if (!HandleHotSpotURLClicked(SciCall_GetCurrentPos(), COPY_HYPERLINK) && 
              !Settings2.NoCopyLineOnEmptySelection)
          {
            // VisualStudio behavior
            SciCall_CopyAllowLine();
          }
        }
        else {
          SciCall_Copy();
        }
      }
      break;


    case IDM_EDIT_COPYLINE:
      {
        if (s_flagPasteBoard) {
          s_bLastCopyFromMe = true;
        }
        DocPos const iSelLnStart = SciCall_PositionFromLine(SciCall_LineFromPosition(SciCall_GetSelectionStart()));
        DocPos const iLineSelLast = SciCall_LineFromPosition(SciCall_GetSelectionEnd());
        // copy incl last line-breaks
        DocPos const iSelLnEnd = SciCall_PositionFromLine(iLineSelLast) + SciCall_LineLength(iLineSelLast);
        if (s_flagPasteBoard) {
          s_bLastCopyFromMe = true;
        }
        _BEGIN_UNDO_ACTION_;
        SciCall_CopyRange(iSelLnStart, iSelLnEnd);
        _END_UNDO_ACTION_;
    }
      break;


    case IDM_EDIT_COPYALL:
      {
        if (s_flagPasteBoard) {
          s_bLastCopyFromMe = true;
        }
        SciCall_CopyRange(0, Sci_GetDocEndPosition());
      }
      break;


    case IDM_EDIT_COPYADD:
      {
        if (SciCall_IsSelectionEmpty()) {
          break;
        }
        if (Sci_IsMultiOrRectangleSelection()) {
          InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECTORMULTI);
          break;
        }
        DocPos const posSelStart = SciCall_GetSelectionStart();
        DocPos const posSelEnd   = SciCall_GetSelectionEnd();
        if (s_flagPasteBoard) {
          s_bLastCopyFromMe = true;
        }
        EditCopyRangeAppend(Globals.hwndEdit, posSelStart, posSelEnd, true);
      }
      break;


    case IDM_EDIT_PASTE:
      if (SciCall_CanPaste()) {
        if (s_flagPasteBoard) {
          s_bLastCopyFromMe = true;
        }
        _BEGIN_UNDO_ACTION_;
        SciCall_Paste();
        _END_UNDO_ACTION_;
      }
      break;


    case IDM_EDIT_SWAP:
      if (!SciCall_IsSelectionEmpty() && SciCall_CanPaste()) {
        if (s_flagPasteBoard) {
          s_bLastCopyFromMe = true;
        }
        _BEGIN_UNDO_ACTION_;
        EditSwapClipboard(Globals.hwndEdit, Settings.SkipUnicodeDetection);
        _END_UNDO_ACTION_;
      }
      break;


    case IDM_EDIT_CLEARCLIPBOARD:
      EditClearClipboard(Globals.hwndEdit);
      break;


    case IDM_EDIT_SELECTALL:
        SciCall_SelectAll();
      break;


    case IDM_EDIT_SELECTWORD:
    {
      if (SciCall_IsSelectionEmpty()) {

        EditSelectWordAtPos(SciCall_GetCurrentPos(), false);

        if (!SciCall_IsSelectionEmpty()) {
          SciCall_ChooseCaretX();
          break;
        }
      }
      // selection not empty or no word found - select line
      DocPos const iLineStart = SciCall_LineFromPosition(SciCall_GetSelectionStart());
      DocPos const iLineEnd = SciCall_LineFromPosition(SciCall_GetSelectionEnd());
      SciCall_SetSelection(SciCall_GetLineEndPosition(iLineEnd), SciCall_PositionFromLine(iLineStart));
      SciCall_ChooseCaretX();
    }
    break;


    case IDM_EDIT_SELECTALLMATCHES:
    {
      if (!Sci_IsMultiOrRectangleSelection()) {
        if (!IsWindow(Globals.hwndDlgFindReplace)) {
          if (SciCall_IsSelectionEmpty()) {
            EditSelectWordAtPos(SciCall_GetCurrentPos(), false);
          }
          EditSelectionMultiSelectAll();
        }
        else {
          SetFindReplaceData();  // s_FindReplaceData 
          EditSelectionMultiSelectAllEx(s_FindReplaceData);
        }
      }
    }
    break;


    case IDM_EDIT_MOVELINEUP:
      EditMoveUp(Globals.hwndEdit);
      break;


    case IDM_EDIT_MOVELINEDOWN:
      EditMoveDown(Globals.hwndEdit);
      break;


    case IDM_EDIT_DUPLINEORSELECTION:
      _BEGIN_UNDO_ACTION_;
      if (SciCall_IsSelectionEmpty()) { 
        SciCall_LineDuplicate(); 
      } 
      else { 
        SciCall_SelectionDuplicate(); 
      }
      _END_UNDO_ACTION_;
      break;


    case IDM_EDIT_LINETRANSPOSE:
      _BEGIN_UNDO_ACTION_;
      SciCall_LineTranspose();
      _END_UNDO_ACTION_;
      break;


    case IDM_EDIT_DELETELINE:
      {
        _BEGIN_UNDO_ACTION_;
        SciCall_MarkerDelete(Sci_GetCurrentLineNumber(), -1);
        SciCall_LineDelete();
        _END_UNDO_ACTION_;
      }
      break;


    case IDM_EDIT_DELETELINELEFT:
      {
        _BEGIN_UNDO_ACTION_;
        SciCall_DelLineLeft();
        _END_UNDO_ACTION_;
      }
      break;


    case IDM_EDIT_DELETELINERIGHT:
      {
        _BEGIN_UNDO_ACTION_;
        SciCall_DelLineRight();
        _END_UNDO_ACTION_;
      }
      break;


    case IDM_EDIT_INDENT:
      EditIndentBlock(Globals.hwndEdit, SCI_TAB, true, false);
      break;

    case IDM_EDIT_UNINDENT:
      EditIndentBlock(Globals.hwndEdit, SCI_BACKTAB, true, false);
      break;

    case CMD_TAB:
      EditIndentBlock(Globals.hwndEdit, SCI_TAB, false, false);
      break;

    case CMD_BACKTAB:
      EditIndentBlock(Globals.hwndEdit, SCI_BACKTAB, false, false);
      break;

    case CMD_CTRLTAB:
      SciCall_SetUseTabs(true);
      SciCall_SetTabIndents(false);
      EditIndentBlock(Globals.hwndEdit, SCI_TAB, false, false);
      SciCall_SetTabIndents(Globals.fvCurFile.bTabIndents);
      SciCall_SetUseTabs(!Globals.fvCurFile.bTabsAsSpaces);
      break;

    case CMD_CHECK_INDENTATION:
      {  
        EditFileIOStatus status = INIT_FILEIO_STATUS;
        EditIndentationStatistic(Globals.hwndEdit, &status);
        if (ConsistentIndentationCheck(&status)) {
          InfoBoxLng(MB_ICONINFORMATION, NULL, IDS_MUI_INDENT_CONSISTENT);
        }
      }
      break;

    case CMD_DELETEBACK:
      {
        ///~_BEGIN_UNDO_ACTION_;
        EditDeleteMarkerInSelection();
        SciCall_DeleteBack();
        ///~_END_UNDO_ACTION_;
      }
      break;

    case CMD_VK_INSERT:
      SciCall_EditToggleOverType();
      break;

    case IDM_EDIT_ENCLOSESELECTION:
      if (EditEncloseSelectionDlg(hwnd,s_wchPrefixSelection,s_wchAppendSelection)) {
        EditEncloseSelection(s_wchPrefixSelection,s_wchAppendSelection);
      }
      break;


    case IDM_EDIT_PADWITHSPACES:
      EditPadWithSpaces(Globals.hwndEdit, false, false);
      break;


    case IDM_EDIT_STRIP1STCHAR:
      EditStripFirstCharacter(Globals.hwndEdit);
      break;


    case IDM_EDIT_STRIPLASTCHAR:
      EditStripLastCharacter(Globals.hwndEdit, false, false);
      break;


    case IDM_EDIT_TRIMLINES:
      EditStripLastCharacter(Globals.hwndEdit, false, true);
      break;


    case IDM_EDIT_COMPRESS_BLANKS:
      EditCompressBlanks(Globals.hwndEdit);
      break;


    case IDM_EDIT_MERGEBLANKLINES:
      EditRemoveBlankLines(Globals.hwndEdit, true, true);
      break;

    case IDM_EDIT_MERGEEMPTYLINES:
      EditRemoveBlankLines(Globals.hwndEdit, true, false);
      break;


    case IDM_EDIT_REMOVEBLANKLINES:
      EditRemoveBlankLines(Globals.hwndEdit, false, true);
      break;


    case IDM_EDIT_REMOVEEMPTYLINES:
      EditRemoveBlankLines(Globals.hwndEdit, false, false);
      break;


    case IDM_EDIT_REMOVEDUPLICATELINES:
      EditRemoveDuplicateLines(Globals.hwndEdit, false);
      break;


    case IDM_EDIT_CLEAR_MARKER:
      EditBookmarkToggle(Globals.hwndEdit, Sci_GetCurrentLineNumber(), 0);
      break;


    case IDM_EDIT_CUT_MARKED:
      EditFocusMarkedLinesCmd(Globals.hwndEdit, true, true);
      break;


    case IDM_EDIT_COPY_MARKED:
      EditFocusMarkedLinesCmd(Globals.hwndEdit, true, false);
      break;


    case IDM_EDIT_DELETE_MARKED:
      EditFocusMarkedLinesCmd(Globals.hwndEdit, false, true);
      break;


    case IDM_EDIT_MODIFYLINES:
      if (EditModifyLinesDlg(hwnd,s_wchPrefixLines,s_wchAppendLines)) {
        EditModifyLines(s_wchPrefixLines,s_wchAppendLines);
      }
      break;


    case IDM_EDIT_ALIGN:
      if (EditAlignDlg(hwnd,&s_iAlignMode)) {
        EditAlignText(s_iAlignMode);
      }
      break;


    case IDM_EDIT_SORTLINES:
      if (EditSortDlg(hwnd,&s_iSortOptions)) {
        EditSortLines(Globals.hwndEdit,s_iSortOptions);
      }
      break;


    case IDM_EDIT_COLUMNWRAP:
      {
        UINT uWrpCol = Globals.iWrapCol;
        if (ColumnWrapDlg(hwnd, IDD_MUI_COLUMNWRAP, &uWrpCol))
        {
          Globals.iWrapCol = clampi((int)uWrpCol, SciCall_GetTabWidth(), LONG_LINES_MARKER_LIMIT);
          EditWrapToColumn(Globals.iWrapCol);
        }
      }
      break;


    case IDM_EDIT_SPLITLINES:
      EditSplitLines(Globals.hwndEdit);
      break;


    case IDM_EDIT_JOINLINES:
      EditJoinLinesEx(false, true);
      break;

    case IDM_EDIT_JOINLN_NOSP:
      EditJoinLinesEx(false, false);
      break;

    case IDM_EDIT_JOINLINES_PARA:
      EditJoinLinesEx(true, true);
      break;


    case IDM_EDIT_CONVERTUPPERCASE:
      {
        _BEGIN_UNDO_ACTION_;
        SciCall_UpperCase();
        _END_UNDO_ACTION_;
      }
      break;


    case IDM_EDIT_CONVERTLOWERCASE:
      {
        _BEGIN_UNDO_ACTION_;
        SciCall_LowerCase();
        _END_UNDO_ACTION_;
      }
      break;


    case IDM_EDIT_INVERTCASE:
      EditInvertCase(Globals.hwndEdit);
      break;


    case IDM_EDIT_TITLECASE:
      EditTitleCase(Globals.hwndEdit);
      break;


    case IDM_EDIT_SENTENCECASE:
      EditSentenceCase(Globals.hwndEdit);
      break;


    case IDM_EDIT_CONVERTTABS:
      EditTabsToSpaces(Globals.fvCurFile.iTabWidth, false);
      break;


    case IDM_EDIT_CONVERTSPACES:
      EditSpacesToTabs(Globals.fvCurFile.iTabWidth, false);
      break;


    case IDM_EDIT_CONVERTTABS2:
      EditTabsToSpaces(Globals.fvCurFile.iTabWidth, true);
      break;


    case IDM_EDIT_CONVERTSPACES2:
      EditSpacesToTabs(Globals.fvCurFile.iTabWidth, true);
      break;


    case IDM_EDIT_INSERT_TAG:
      {
        WCHAR wszOpen[256] = { L'\0' };
        WCHAR wszClose[256] = { L'\0' };
        UINT repeat = 1;
        if (EditInsertTagDlg(hwnd, wszOpen, wszClose, &repeat)) {
          while (repeat > 0) {
            EditEncloseSelection(wszOpen, wszClose);
            --repeat;
          }
        }
      }
      break;


    case IDM_EDIT_INSERT_ENCODING:
      {
        cpi_enc_t const iEncoding = Encoding_GetCurrent();
        char chEncStrg[128] = { '\0' };
        WideCharToMultiByteEx(Encoding_SciCP, 0, Encoding_GetLabel(iEncoding), -1, chEncStrg, COUNTOF(chEncStrg), NULL, NULL);
        EditReplaceSelection(chEncStrg, false);
      }
      break;


    case IDM_EDIT_INSERT_SHORTDATE:
    case IDM_EDIT_INSERT_LONGDATE:
      EditInsertDateTimeStrg((iLoWParam == IDM_EDIT_INSERT_SHORTDATE), false);
      break;


    case IDM_EDIT_INSERT_FILENAME:
    case IDM_EDIT_INSERT_DIRNAME:
    case IDM_EDIT_INSERT_PATHNAME:
      {
        WCHAR *pszInsert;
        WCHAR tchUntitled[32];
        WCHAR szDisplayName[MAX_PATH];

        if (StrIsNotEmpty(Globals.CurrentFile)) {
          if (iLoWParam == IDM_EDIT_INSERT_FILENAME) 
          {
            PathGetDisplayName(szDisplayName, COUNTOF(szDisplayName), Globals.CurrentFile);
            pszInsert = szDisplayName;
          }
          else if (iLoWParam == IDM_EDIT_INSERT_DIRNAME) 
          {
            StringCchCopy(szDisplayName, COUNTOF(szDisplayName), Globals.CurrentFile);
            PathCchRemoveFileSpec(szDisplayName, COUNTOF(szDisplayName));
            pszInsert = szDisplayName;
          }
          else {
            pszInsert = Globals.CurrentFile;
          }
        }
        else {
          GetLngString(IDS_MUI_UNTITLED, tchUntitled, COUNTOF(tchUntitled));
          pszInsert = tchUntitled;
        }
        SetClipboardTextW(hwnd, pszInsert, StringCchLen(pszInsert, 0));
      }
      break;


    case IDM_EDIT_INSERT_GUID:
      {
        GUID guid;
        if (SUCCEEDED(CoCreateGuid(&guid))) {  
          if (StringFromGUID2(&guid, tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer))) {
            StrTrimW(tchMaxPathBuffer, L"{}");
            //char chMaxPathBuffer[MAX_PATH] = { '\0' };
            //if (WideCharToMultiByteEx(Encoding_SciCP, 0, tchMaxPathBuffer, -1, chMaxPathBuffer, COUNTOF(chMaxPathBuffer), NULL, NULL)) {
            //  EditReplaceSelection(chMaxPathBuffer, false);
            //}
            SetClipboardTextW(hwnd, tchMaxPathBuffer, StringCchLen(tchMaxPathBuffer, 0));
          }
        }
      }
      break;


    case IDM_EDIT_LINECOMMENT:
      {
        switch (SciCall_GetLexer()) {
          case SCLEX_CPP:
          case SCLEX_D:
          case SCLEX_HTML:
          case SCLEX_PASCAL:
          case SCLEX_RUST:
          case SCLEX_XML:
            EditToggleLineComments(Globals.hwndEdit, L"//", false);
            break;
          case SCLEX_VB:
          case SCLEX_VBSCRIPT:
            EditToggleLineComments(Globals.hwndEdit, L"'", false);
            break;
          case SCLEX_AVS:
          case SCLEX_BASH:
          case SCLEX_CMAKE:
          case SCLEX_COFFEESCRIPT:
          case SCLEX_CONF:
          case SCLEX_MAKEFILE:
          case SCLEX_NIM:
          case SCLEX_PERL:
          case SCLEX_POWERSHELL:
          case SCLEX_PYTHON:
          case SCLEX_R:
          case SCLEX_RUBY:
          case SCLEX_TCL:
          case SCLEX_TOML:
          case SCLEX_YAML:
            EditToggleLineComments(Globals.hwndEdit, L"#", true);
            break;
          case SCLEX_AHKL:
          case SCLEX_ASM:
          case SCLEX_AU3:
          case SCLEX_INNOSETUP:
          case SCLEX_NSIS: // # could also be used instead
          case SCLEX_PROPERTIES:
          case SCLEX_REGISTRY:
            EditToggleLineComments(Globals.hwndEdit, L";", true);
            break;
          case SCLEX_LUA:
          case SCLEX_SQL:
          case SCLEX_VHDL:
            EditToggleLineComments(Globals.hwndEdit, L"--", true);
            break;
          case SCLEX_BATCH:
            EditToggleLineComments(Globals.hwndEdit, L"rem ", true);
            break;
          case SCLEX_LATEX:
          case SCLEX_MATLAB:
            EditToggleLineComments(Globals.hwndEdit, L"%", true);
            break;
          // ------------------
          case SCLEX_NULL:
          case SCLEX_CSS:
          case SCLEX_DIFF:
          case SCLEX_JSON:
          case SCLEX_MARKDOWN:
          default:
            // do nothing
            break;
        }
      }
      break;


    case IDM_EDIT_STREAMCOMMENT:
      {
        switch (SciCall_GetLexer()) {
          case SCLEX_D:
            //~EditEncloseSelection(Globals.hwndEdit, L"/+", L"+/");
            //~break;
          case SCLEX_AVS:
          case SCLEX_CPP:
          case SCLEX_CSS:
          case SCLEX_HTML:
          case SCLEX_NSIS:
          case SCLEX_RUST:
          case SCLEX_SQL:
          case SCLEX_VHDL:
          case SCLEX_XML:
            EditEncloseSelection(L"/*", L"*/");
            break;
          case SCLEX_INNOSETUP:
          case SCLEX_PASCAL:
            EditEncloseSelection(L"{", L"}");
            break;
          case SCLEX_LUA:
            EditEncloseSelection(L"--[[", L"]]");
            break;
          case SCLEX_COFFEESCRIPT:
            EditEncloseSelection(L"###", L"###");
            break;
          case SCLEX_MATLAB:
            EditEncloseSelection(L"%{", L"%}");
            break;
          // ------------------
          case SCLEX_NULL:
          case SCLEX_AHKL:
          case SCLEX_ASM:
          case SCLEX_AU3:
          case SCLEX_BASH:
          case SCLEX_BATCH:
          case SCLEX_CMAKE:
          case SCLEX_CONF:
          case SCLEX_DIFF:
          case SCLEX_JSON:
          case SCLEX_LATEX:
          case SCLEX_MAKEFILE:
          case SCLEX_MARKDOWN:
          case SCLEX_NIM:
          case SCLEX_PERL:
          case SCLEX_POWERSHELL:
          case SCLEX_PROPERTIES:
          case SCLEX_PYTHON:
          case SCLEX_R:
          case SCLEX_REGISTRY:
          case SCLEX_RUBY:
          case SCLEX_TCL:
          case SCLEX_TOML:
          case SCLEX_VB:
          case SCLEX_VBSCRIPT:
          case SCLEX_YAML:
          default:
            // do nothing
            break;
        }
      }
      break;


    case IDM_EDIT_URLENCODE:
      {
        EditURLEncode(Globals.hwndEdit);
      }
      break;


    case IDM_EDIT_URLDECODE:
      {
        EditURLDecode(Globals.hwndEdit);
      }
      break;


    case IDM_EDIT_ESCAPECCHARS:
      {
        EditEscapeCChars(Globals.hwndEdit);
      }
      break;


    case IDM_EDIT_UNESCAPECCHARS:
      {
        EditUnescapeCChars(Globals.hwndEdit);
      }
      break;


    case IDM_EDIT_CHAR2HEX:
      EditChar2Hex(Globals.hwndEdit);
      break;


    case IDM_EDIT_HEX2CHAR:
      EditHex2Char(Globals.hwndEdit);
      break;


    case IDM_EDIT_FINDMATCHINGBRACE:
      EditFindMatchingBrace(Globals.hwndEdit);
      break;


    case IDM_EDIT_SELTOMATCHINGBRACE:
      EditSelectToMatchingBrace(Globals.hwndEdit);
      break;


    // Main Bookmark Functions
    case BME_EDIT_BOOKMARKNEXT:
      EditBookmarkNext(Globals.hwndEdit, Sci_GetCurrentLineNumber());
      break;


    case BME_EDIT_BOOKMARKPREV:
      EditBookmarkPrevious(Globals.hwndEdit, Sci_GetCurrentLineNumber());
      break;


    case BME_EDIT_BOOKMARKTOGGLE:
      EditBookmarkToggle(Globals.hwndEdit, Sci_GetCurrentLineNumber(), 0);
      break;


    case BME_EDIT_BOOKMARKCLEAR:
      EditClearAllBookMarks(Globals.hwndEdit);
      break;


    case IDM_EDIT_FIND:
      {
        SetFindReplaceData(); // s_FindReplaceData
        if (!IsWindow(Globals.hwndDlgFindReplace)) {
          Globals.bFindReplCopySelOrClip = true;
          /*Globals.hwndDlgFindReplace =*/ EditFindReplaceDlg(Globals.hwndEdit, &s_FindReplaceData, false);
        }
        else {
          Globals.bFindReplCopySelOrClip = (GetForegroundWindow() != Globals.hwndDlgFindReplace);
          if (GetDlgItem(Globals.hwndDlgFindReplace, IDC_REPLACE)) {
            SendWMCommand(Globals.hwndDlgFindReplace, IDMSG_SWITCHTOFIND);
            DestroyWindow(Globals.hwndDlgFindReplace);
            /*Globals.hwndDlgFindReplace =*/ EditFindReplaceDlg(Globals.hwndEdit, &s_FindReplaceData, false);
          }
          else {
            SetForegroundWindow(Globals.hwndDlgFindReplace);
            PostMessage(Globals.hwndDlgFindReplace, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(Globals.hwndDlgFindReplace, IDC_FINDTEXT)), 1);
          }
        }
      }
      break;


    case IDM_EDIT_REPLACE:
      {
        SetFindReplaceData(); // s_FindReplaceData
        if (!IsWindow(Globals.hwndDlgFindReplace)) {
          Globals.bFindReplCopySelOrClip = true;
          /*Globals.hwndDlgFindReplace =*/ EditFindReplaceDlg(Globals.hwndEdit, &s_FindReplaceData, true);
        }
        else {
          Globals.bFindReplCopySelOrClip = (GetForegroundWindow() != Globals.hwndDlgFindReplace);
          if (!GetDlgItem(Globals.hwndDlgFindReplace, IDC_REPLACE)) {
            SendWMCommand(Globals.hwndDlgFindReplace, IDMSG_SWITCHTOREPLACE);
            DestroyWindow(Globals.hwndDlgFindReplace);
            /*Globals.hwndDlgFindReplace =*/ EditFindReplaceDlg(Globals.hwndEdit, &s_FindReplaceData, true);
          }
          else {
            SetForegroundWindow(Globals.hwndDlgFindReplace);
            PostMessage(Globals.hwndDlgFindReplace, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(Globals.hwndDlgFindReplace, IDC_FINDTEXT)), 1);
          }
        }
      }
      break;


    case IDM_EDIT_FINDNEXT:
    case IDM_EDIT_FINDPREV:
    case IDM_EDIT_REPLACENEXT:
    case IDM_EDIT_SELTONEXT:
    case IDM_EDIT_SELTOPREV:

      if (Sci_IsDocEmpty()) { break; }

      if (Sci_IsMultiSelection()) { 
        switch (iLoWParam) {
          case IDM_EDIT_SELTONEXT:
          {
            SciCall_RotateSelection();
            EditEnsureSelectionVisible();
          }
          break;

          case IDM_EDIT_SELTOPREV:
          {
            DocPosU const iMain = SciCall_GetMainSelection();
            if (iMain > 0) {
              SciCall_SetMainSelection(iMain - 1);
            } else {
              DocPosU const iNewMain = SciCall_GetSelections() - 1;
              SciCall_SetMainSelection(iNewMain);
            }
            EditEnsureSelectionVisible();
          }
          break;

          default: break;
        }
        break; // done
      }

      SetFindReplaceData(); // s_FindReplaceData

      if (IsFindPatternEmpty() && !StrIsEmptyA(s_FindReplaceData.szFind))
      {
        if (iLoWParam != IDM_EDIT_REPLACENEXT) {
          SendWMCommand(hwnd, IDM_EDIT_FIND);
        }
        else {
          SendWMCommand(hwnd, IDM_EDIT_REPLACE);
        }
      }
      else {

        switch (iLoWParam) {

          case IDM_EDIT_FINDNEXT:
            EditFindNext(Globals.hwndEdit,&s_FindReplaceData,false,false);
            break;

          case IDM_EDIT_FINDPREV:
            EditFindPrev(Globals.hwndEdit,&s_FindReplaceData,false,false);
            break;

          case IDM_EDIT_REPLACENEXT:
            if (Globals.bReplaceInitialized) {
              EditReplace(Globals.hwndEdit, &s_FindReplaceData);
            }
            else {
              SendWMCommand(hwnd, IDM_EDIT_REPLACE);
            }
            break;

          case IDM_EDIT_SELTONEXT:
            if (IsFindPatternEmpty()) {
              if (!SetCurrentSelAsFindReplaceData()) {
                break;
              }
            }
            EditFindNext(Globals.hwndEdit,&s_FindReplaceData,true,false);
            break;

          case IDM_EDIT_SELTOPREV:
            if (IsFindPatternEmpty()) {
              if (!SetCurrentSelAsFindReplaceData()) {
                break;
              }
            }
            EditFindPrev(Globals.hwndEdit,&s_FindReplaceData,true,false);
            break;
        }
      }
      break;


    case CMD_FINDNEXTSEL:
    case CMD_FINDPREVSEL:
    case IDM_EDIT_SAVEFIND:
    {
      if (SetCurrentSelAsFindReplaceData()) {

        MRU_Add(Globals.pMRUfind, GetFindPattern(), 0, -1, -1, NULL);

        s_FindReplaceData.fuFlags &= (~(SCFIND_REGEXP | SCFIND_POSIX));
        s_FindReplaceData.bTransformBS = false;

        switch (iLoWParam) {

        case IDM_EDIT_SAVEFIND:
          break;

        case CMD_FINDNEXTSEL:
          EditFindNext(Globals.hwndEdit, &s_FindReplaceData, false, false);
          break;

        case CMD_FINDPREVSEL:
          EditFindPrev(Globals.hwndEdit, &s_FindReplaceData, false, false);
          break;
        }
      }
    }
    break;


    case IDM_EDIT_COMPLETEWORD:
      EditAutoCompleteWord(Globals.hwndEdit, true);
      break;


    case IDM_EDIT_GOTOLINE:
      EditLinenumDlg(Globals.hwndEdit);
      break;


    case IDM_VIEW_SCHEME:
      Style_SelectLexerDlg(Globals.hwndEdit);
      break;


    case IDM_VIEW_USE2NDDEFAULT:
      Style_ToggleUse2ndDefault(Globals.hwndEdit);
      break;


    case IDM_VIEW_SCHEMECONFIG:
      if (!IsWindow(Globals.hwndDlgCustomizeSchemes)) {
        Globals.hwndDlgCustomizeSchemes = Style_CustomizeSchemesDlg(Globals.hwndEdit);
      }
      else {
        SetForegroundWindow(Globals.hwndDlgCustomizeSchemes);
      }
      PostWMCommand(Globals.hwndDlgCustomizeSchemes, IDC_SETCURLEXERTV);
      UpdateStatusbar(true);
      UpdateMarginWidth();
      break;


    case IDM_VIEW_FONT:
      if (!IsWindow(Globals.hwndDlgCustomizeSchemes)) {
        Style_SetDefaultFont(Globals.hwndEdit, true);
      }
      UpdateToolbar();
      UpdateStatusbar(true);
      UpdateMarginWidth();
      break;


    case IDM_VIEW_CURRENTSCHEME:
      if (!IsWindow(Globals.hwndDlgCustomizeSchemes)) {
        Style_SetDefaultFont(Globals.hwndEdit, false);
      }
      UpdateToolbar();
      UpdateStatusbar(true);
      UpdateMarginWidth();
      break;


    case IDM_VIEW_WORDWRAP:
      Settings.WordWrap = !Settings.WordWrap;
      Globals.fvCurFile.bWordWrap = Settings.WordWrap;
      _SetWrapIndentMode(Globals.hwndEdit);
      EditEnsureSelectionVisible();
      UpdateToolbar();
      break;


    case IDM_VIEW_WORDWRAPSETTINGS:
      if (WordWrapSettingsDlg(hwnd,IDD_MUI_WORDWRAP, &Settings.WordWrapIndent)) {
        _SetWrapIndentMode(Globals.hwndEdit);
        _SetWrapVisualFlags(Globals.hwndEdit);
        UpdateToolbar();
      }
      break;


    case IDM_VIEW_WORDWRAPSYMBOLS:
      Settings.ShowWordWrapSymbols = !Settings.ShowWordWrapSymbols;
      _SetWrapVisualFlags(Globals.hwndEdit);
      UpdateToolbar();
      break;


    case IDM_VIEW_LONGLINEMARKER:
      {
        Settings.MarkLongLines = !Settings.MarkLongLines;
        size_t cnt = 0;
        int edgeColumns[SMALL_BUFFER] = { 0 };
        if (Settings.MarkLongLines) {
          cnt = ReadVectorFromString(Globals.fvCurFile.wchMultiEdgeLines, edgeColumns, COUNTOF(edgeColumns), 0, LONG_LINES_MARKER_LIMIT, 0, true);
        }
        Style_SetMultiEdgeLine(edgeColumns, cnt);
      }
      break;


    case IDM_VIEW_LONGLINESETTINGS:
      {
        int _iLongLinesLimit = Defaults.LongLinesLimit;

        if (LongLineSettingsDlg(hwnd, IDD_MUI_LONGLINES, Globals.fvCurFile.wchMultiEdgeLines)) {

          int edgeColumns[SMALL_BUFFER];
          size_t const cnt = ReadVectorFromString(Globals.fvCurFile.wchMultiEdgeLines, edgeColumns, COUNTOF(edgeColumns), 0, LONG_LINES_MARKER_LIMIT, 0, true);

          if (cnt == 0) {
            Settings.MarkLongLines = false;
          }
          else if (cnt == 1) {
            _iLongLinesLimit = edgeColumns[0];
            Settings.MarkLongLines = true;
            //~Settings.LongLineMode = EDGE_LINE|EDGE_BACKGROUND; // set by Dlg
          }
          else {
            _iLongLinesLimit = edgeColumns[cnt - 1];
            Settings.MarkLongLines = true;
            Settings.LongLineMode = EDGE_MULTILINE;
          }
          Globals.iWrapCol = _iLongLinesLimit;
          Settings.LongLinesLimit = _iLongLinesLimit;

          // new multi-edge lines setting
          WCHAR col[32];
          Settings.MultiEdgeLines[0] = L'\0';
          for (size_t i = 0; i < cnt; ++i) {
            StringCchPrintf(col, COUNTOF(col), ((i == 0) ? L"%i" : L" %i"), edgeColumns[i]);
            StringCchCat(Settings.MultiEdgeLines, COUNTOF(Settings.MultiEdgeLines), col);
          }
          // make current too
          StringCchCopy(Globals.fvCurFile.wchMultiEdgeLines, COUNTOF(Globals.fvCurFile.wchMultiEdgeLines), Settings.MultiEdgeLines);

          Style_SetMultiEdgeLine(edgeColumns, cnt);
        }
      }
      break;


    case IDM_VIEW_TABSASSPACES:
      {
        Settings.TabsAsSpaces = !Settings.TabsAsSpaces;
        Globals.fvCurFile.bTabsAsSpaces = Settings.TabsAsSpaces;
        SciCall_SetUseTabs(!Globals.fvCurFile.bTabsAsSpaces);
      }
      break;


    case IDM_VIEW_TABSETTINGS:
      if (TabSettingsDlg(hwnd,IDD_MUI_TABSETTINGS,NULL))
      {
        SciCall_SetUseTabs(!Globals.fvCurFile.bTabsAsSpaces);
        SciCall_SetTabIndents(Globals.fvCurFile.bTabIndents);
        SciCall_SetBackSpaceUnIndents(Settings.BackspaceUnindents);
        SciCall_SetTabWidth(Globals.fvCurFile.iTabWidth);
        SciCall_SetIndent(Globals.fvCurFile.iIndentWidth);
        if (SendMessage(Globals.hwndEdit, SCI_GETWRAPINDENTMODE, 0, 0) == SC_WRAPINDENT_FIXED) {
          _SetWrapStartIndent(Globals.hwndEdit);
        }
      }
      break;


    case IDM_VIEW_SHOWINDENTGUIDES:
      Settings.ShowIndentGuides = !Settings.ShowIndentGuides;
      Style_SetIndentGuides(Globals.hwndEdit,Settings.ShowIndentGuides);
      break;


    case IDM_VIEW_AUTOINDENTTEXT:
      Settings.AutoIndent = !Settings.AutoIndent;
      break;


    case IDM_VIEW_LINENUMBERS:
      Settings.ShowLineNumbers = !Settings.ShowLineNumbers;
      UpdateMarginWidth();
      break;


    case IDM_VIEW_BOOKMARK_MARGIN:
      Settings.ShowBookmarkMargin = !Settings.ShowBookmarkMargin;
      UpdateMarginWidth();
      break;

    case IDM_VIEW_AUTOCOMPLETEWORDS:
      Settings.AutoCompleteWords = !Settings.AutoCompleteWords;
      SciCall_AutoCCancel();
      break;

    case IDM_VIEW_AUTOCLEXKEYWORDS:
      Settings.AutoCLexerKeyWords = !Settings.AutoCLexerKeyWords;
      SciCall_AutoCCancel();
      break;

    case IDM_VIEW_ACCELWORDNAV:
      Settings.AccelWordNavigation = !Settings.AccelWordNavigation;
      EditSetAccelWordNav(Globals.hwndEdit,Settings.AccelWordNavigation);
      MarkAllOccurrences(Settings2.UpdateDelayMarkAllOccurrences, true);
      break;

    case IDM_VIEW_EDIT_LINECOMMENT:
      Settings.EditLineCommentBlock = !Settings.EditLineCommentBlock;
      break;
      
    case IDM_VIEW_MARKOCCUR_ONOFF:
      Settings.MarkOccurrences = !Settings.MarkOccurrences;
      if (!Settings.MarkOccurrences && FocusedView.HideNonMatchedLines) {
        EditToggleView(Globals.hwndEdit);
      }
      EnableCmd(GetMenu(hwnd), IDM_VIEW_TOGGLE_VIEW, IsFocusedViewAllowed());
      if (IsMarkOccurrencesEnabled()) {
        MarkAllOccurrences(0, true);
      }
      else {
        EditClearAllOccurrenceMarkers(Globals.hwndEdit);
        Globals.iMarkOccurrencesCount = 0;
      }
      break;

    case IDM_VIEW_MARKOCCUR_BOOKMARKS:
      Settings.MarkOccurrencesBookmark = !Settings.MarkOccurrencesBookmark;
      SciCall_MarkerDefine(MARKER_NP3_OCCURRENCE, Settings.MarkOccurrencesBookmark ? SC_MARK_ARROWS : SC_MARK_BACKGROUND);
      break;

    case IDM_VIEW_MARKOCCUR_VISIBLE:
      Settings.MarkOccurrencesMatchVisible = !Settings.MarkOccurrencesMatchVisible;
      MarkAllOccurrences(0, true);
      break;

    case IDM_VIEW_TOGGLE_VIEW:
      if (FocusedView.HideNonMatchedLines) {
        EditToggleView(Globals.hwndEdit);
        MarkAllOccurrences(0, true);
      }
      else {
        EditToggleView(Globals.hwndEdit);
      }
      CheckCmd(GetMenu(hwnd), IDM_VIEW_TOGGLE_VIEW, FocusedView.HideNonMatchedLines);
      break;

    case IDM_VIEW_FV_FOLD:
    case IDM_VIEW_FV_BOOKMARK:
    case IDM_VIEW_FV_HIGHLIGHT:
    case IDM_VIEW_FV_BKMRKFOLD:
    case IDM_VIEW_FV_HIGHLGFOLD:
    {
        int newSetting = Settings.FocusViewMarkerMode;
        switch (iLoWParam) {
            case IDM_VIEW_FV_FOLD:
                newSetting = (FVMM_FOLD);
                break;
            case IDM_VIEW_FV_BOOKMARK:
                newSetting = (FVMM_MARGIN);
                break;
            case IDM_VIEW_FV_HIGHLIGHT:
                newSetting = (FVMM_LN_BACKGR);
                break;
            case IDM_VIEW_FV_BKMRKFOLD:
                newSetting = (FVMM_MARGIN | FVMM_FOLD);
                break;
            case IDM_VIEW_FV_HIGHLGFOLD:
                newSetting = (FVMM_LN_BACKGR | FVMM_FOLD);
                break;
        }
        if (newSetting != Settings.FocusViewMarkerMode)
        {
            if (FocusedView.HideNonMatchedLines) {
                if ((newSetting & FVMM_FOLD) != (Settings.FocusViewMarkerMode & FVMM_FOLD)) {
                    EditToggleView(Globals.hwndEdit);
                }
            }
            for (int m = MARKER_NP3_1; m < MARKER_NP3_BOOKMARK; ++m) {
                SciCall_MarkerDefine(m, (newSetting & FVMM_LN_BACKGR) ? SC_MARK_BACKGROUND : SC_MARK_BOOKMARK);
            }
            Settings.FocusViewMarkerMode = newSetting;
        }
    } break;

    case IDM_VIEW_MARKOCCUR_CASE:
      Settings.MarkOccurrencesMatchCase = !Settings.MarkOccurrencesMatchCase;
      MarkAllOccurrences(Settings2.UpdateDelayMarkAllOccurrences, true);
      break;

    case IDM_VIEW_MARKOCCUR_WNONE:
      Settings.MarkOccurrencesMatchWholeWords = false;
      Settings.MarkOccurrencesCurrentWord = false;
      MarkAllOccurrences(Settings2.UpdateDelayMarkAllOccurrences, true);
      break;

    case IDM_VIEW_MARKOCCUR_WORD:
      Settings.MarkOccurrencesMatchWholeWords = true;
      Settings.MarkOccurrencesCurrentWord = false;
      MarkAllOccurrences(Settings2.UpdateDelayMarkAllOccurrences, true);
      break;

    case IDM_VIEW_MARKOCCUR_CURRENT:
      Settings.MarkOccurrencesMatchWholeWords = false;
      Settings.MarkOccurrencesCurrentWord = true;
      MarkAllOccurrences(Settings2.UpdateDelayMarkAllOccurrences, true);
      break;

    case IDM_VIEW_FOLDING:
      Settings.ShowCodeFolding = !Settings.ShowCodeFolding;
      FocusedView.ShowCodeFolding = Settings.ShowCodeFolding;
      Style_SetFolding(Globals.hwndEdit, FocusedView.ShowCodeFolding);
      if (!FocusedView.ShowCodeFolding) { EditToggleFolds(EXPAND, true); }
      break;


    case IDM_VIEW_TOGGLEFOLDS:
      EditToggleFolds(SNIFF, SciCall_IsSelectionEmpty());
      break;
      
    case IDM_VIEW_TOGGLE_CURRENT_FOLD:
      EditToggleFolds(SNIFF, false);
      break;

    case IDM_VIEW_SHOWBLANKS:
      Settings.ViewWhiteSpace = !Settings.ViewWhiteSpace;
      SciCall_SetViewWS(Settings.ViewWhiteSpace ? SCWS_VISIBLEALWAYS : SCWS_INVISIBLE);
      break;

    case IDM_VIEW_SHOWEOLS:
      Settings.ViewEOLs = !Settings.ViewEOLs;
      SciCall_SetViewEOL(Settings.ViewEOLs);
      break;

    case IDM_VIEW_MATCHBRACES:
      Settings.MatchBraces = !Settings.MatchBraces;
      if (Settings.MatchBraces)
        EditMatchBrace(Globals.hwndEdit);
      else
        SciCall_BraceHighLight(INVALID_POSITION, INVALID_POSITION);
      break;

    case IDM_VIEW_AUTOCLOSETAGS:
      Settings.AutoCloseTags = !Settings.AutoCloseTags;
      break;

    case IDM_VIEW_TOGGLE_HILITCURLN:
    case IDM_VIEW_HILITCURLN_NONE:
    case IDM_VIEW_HILITCURLN_BACK:
    case IDM_VIEW_HILITCURLN_FRAME:
      {
        int const set = iLoWParam - IDM_VIEW_HILITCURLN_NONE;
        Settings.HighlightCurrentLine = (set >= 0) ? set : ((Settings.HighlightCurrentLine + 1) % 3);
        Style_HighlightCurrentLine(Globals.hwndEdit, Settings.HighlightCurrentLine);
      }
      break;

    case IDM_VIEW_HYPERLINKHOTSPOTS:
      Settings.HyperlinkHotspot = !Settings.HyperlinkHotspot;
      EditUpdateVisibleIndicators();
      break;

    case IDM_VIEW_SHOW_HYPLNK_CALLTIP:
      Settings.ShowHypLnkToolTip = !Settings.ShowHypLnkToolTip;
      UpdateMouseDWellTime();
      break;

    case IDM_VIEW_COLORDEFHOTSPOTS:
    case IDM_VIEW_COLOR_ARGB:
    case IDM_VIEW_COLOR_RGBA:
    case IDM_VIEW_COLOR_BGRA:
      {
        Settings.ColorDefHotspot = iLoWParam - IDM_VIEW_COLORDEFHOTSPOTS;
        EditUpdateVisibleIndicators();
        UpdateMouseDWellTime();
      }
      break;

    case IDM_VIEW_UNICODE_POINTS:
      Settings.HighlightUnicodePoints = !Settings.HighlightUnicodePoints;
      EditUpdateVisibleIndicators();
      UpdateMouseDWellTime();
      break;

    case IDM_VIEW_ZOOMIN:
      {
        SciCall_ZoomIn();
        ShowZoomCallTip();
      }
      break;

    case IDM_VIEW_ZOOMOUT:
      {
        SciCall_ZoomOut();
        ShowZoomCallTip();
    }
      break;

    case IDM_VIEW_RESETZOOM:
      {
        SciCall_SetZoom(100);
        ShowZoomCallTip();
    }
      break;

    case IDM_VIEW_CHASING_DOCTAIL: 
      {
        FileWatching.MonitoringLog = !FileWatching.MonitoringLog; // toggle
        SciCall_SetReadOnly(FileWatching.MonitoringLog);

        if (FileWatching.MonitoringLog)
        {
          SetForegroundWindow(hwnd);
          SendWMCommand(hwnd, IDM_FILE_REVERT);
          FileWatching.flagChangeNotify = s_flagChangeNotify;
          s_flagChangeNotify = FWM_AUTORELOAD;
          FileWatching.FileWatchingMode = FWM_AUTORELOAD;
          FileWatching.ResetFileWatching = true;
          FileWatching.FileCheckInverval = 250UL;
          FileWatching.AutoReloadTimeout = 250UL;
          UndoRedoRecordingStop();
          SciCall_SetEndAtLastLine(false);
        }
        else {
          s_flagChangeNotify = FileWatching.flagChangeNotify;
          FileWatching.FileWatchingMode = Settings.FileWatchingMode;
          FileWatching.ResetFileWatching = Settings.ResetFileWatching;
          FileWatching.FileCheckInverval = Settings2.FileCheckInverval;
          FileWatching.AutoReloadTimeout = Settings2.AutoReloadTimeout;
          UndoRedoRecordingStart();
          SciCall_SetEndAtLastLine(!Settings.ScrollPastEOF);
        }
        EditEnsureSelectionVisible();

        InstallFileWatching(Globals.CurrentFile); // force

        CheckCmd(GetMenu(Globals.hwndMain), IDM_VIEW_CHASING_DOCTAIL, FileWatching.MonitoringLog);

      }
      break;
    
    case IDM_VIEW_SCROLLPASTEOF:
      Settings.ScrollPastEOF = !Settings.ScrollPastEOF;
      SciCall_SetEndAtLastLine(!Settings.ScrollPastEOF);
      break;

    case IDM_VIEW_MENUBAR:
      Settings.ShowMenubar = !Settings.ShowMenubar;
      SetMenu(hwnd, (Settings.ShowMenubar ? Globals.hMainMenu : NULL));
      DrawMenuBar(Globals.hwndMain);
      break;

    case IDM_VIEW_TOOLBAR:
      Settings.ShowToolbar = !Settings.ShowToolbar;
      ShowWindow(Globals.hwndRebar, (Settings.ShowToolbar ? SW_SHOW : SW_HIDE));
      SendWMSize(hwnd, NULL);
      break;

    case IDM_VIEW_TOGGLETB:
      Settings.ToolBarTheme = (Settings.ToolBarTheme + 1) % 3;
      SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
      break;

    case IDM_VIEW_CUSTOMIZETB:
      SendMessage(Globals.hwndToolbar,TB_CUSTOMIZE,0,0);
      break;

    case IDM_VIEW_LOADTHEMETB:
      if (SelectExternalToolBar(hwnd)) {
        SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
      }
      break;

    case IDM_VIEW_DPISCALETB:
      Settings.DpiScaleToolBar = !Settings.DpiScaleToolBar;
      SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
      break;

    case IDM_VIEW_STATUSBAR:
      Settings.ShowStatusbar = !Settings.ShowStatusbar;
      ShowWindow(Globals.hwndStatus, (Settings.ShowStatusbar ? SW_SHOW : SW_HIDE));
      UpdateStatusbar(Settings.ShowStatusbar);
      SendWMSize(hwnd, NULL);
      break;


    case IDM_VIEW_STICKYWINPOS:
      if (IsCmdEnabled(hwnd, IDM_VIEW_STICKYWINPOS)) {
        Flags.bStickyWindowPosition = !Flags.bStickyWindowPosition; // toggle

        if (Flags.bStickyWindowPosition) { InfoBoxLng(MB_OK, L"MsgStickyWinPos", IDS_MUI_STICKYWINPOS); }

        bool bOpendByMe = false;
        OpenSettingsFile(&bOpendByMe);

        SaveWindowPositionSettings(!Flags.bStickyWindowPosition);

        if (Flags.bStickyWindowPosition != DefaultFlags.bStickyWindowPosition)
        {
          IniSectionSetBool(Constants.Settings2_Section, L"StickyWindowPosition", Flags.bStickyWindowPosition);
        }
        else {
          IniSectionDelete(Constants.Settings2_Section, L"StickyWindowPosition", false);
        }

        CloseSettingsFile(true, bOpendByMe);
      }
      break;


    case IDM_VIEW_REUSEWINDOW:
      if (IsCmdEnabled(hwnd, IDM_VIEW_REUSEWINDOW)) {
        Flags.bReuseWindow = !Flags.bReuseWindow; // reverse
        if (Globals.bCanSaveIniFile) {
          if (Flags.bReuseWindow != DefaultFlags.bReuseWindow) {
            IniFileSetBool(Globals.IniFile, Constants.Settings2_Section, L"ReuseWindow", Flags.bReuseWindow);
          }
          else {
            IniFileDelete(Globals.IniFile, Constants.Settings2_Section, L"ReuseWindow", false);
          }
        }
      }
      break;


    case IDM_VIEW_SINGLEFILEINSTANCE:
      if (IsCmdEnabled(hwnd, IDM_VIEW_SINGLEFILEINSTANCE)) {
        Flags.bSingleFileInstance = !Flags.bSingleFileInstance; // reverse
        if (Globals.bCanSaveIniFile) {
          if (Flags.bSingleFileInstance != DefaultFlags.bSingleFileInstance) {
            IniFileSetInt(Globals.IniFile, Constants.Settings2_Section, L"SingleFileInstance", Flags.bSingleFileInstance);
          }
          else {
            IniFileDelete(Globals.IniFile, Constants.Settings2_Section, L"SingleFileInstance", false);
          }
        }
      }
      break;


    case IDM_VIEW_ALWAYSONTOP:
      if ((Settings.AlwaysOnTop || s_flagAlwaysOnTop == 2) && s_flagAlwaysOnTop != 1) {
        Settings.AlwaysOnTop = false;
        s_flagAlwaysOnTop = 0;
        SetWindowPos(hwnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
      }
      else {
        Settings.AlwaysOnTop = true;
        s_flagAlwaysOnTop = 0;
        SetWindowPos(hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
      }
      CheckCmd(GetMenu(Globals.hwndMain), IDM_VIEW_ALWAYSONTOP, Settings.AlwaysOnTop);
      UpdateToolbar();
      break;


    case IDM_VIEW_MINTOTRAY:
      Settings.MinimizeToTray = !Settings.MinimizeToTray;
      break;


    case IDM_VIEW_TRANSPARENT:
      Settings.TransparentMode = !Settings.TransparentMode;
      SetWindowTransparentMode(hwnd,Settings.TransparentMode, Settings2.OpacityLevel);
      break;


    case IDM_SET_RENDER_TECH_GDI:
    case IDM_SET_RENDER_TECH_D2D:
    case IDM_SET_RENDER_TECH_D2DRETAIN:
    case IDM_SET_RENDER_TECH_D2DDC:
      {
        int const prevRT = Settings.RenderingTechnology;
        Settings.RenderingTechnology = (iLoWParam - IDM_SET_RENDER_TECH_GDI);
        SciCall_SetTechnology(Settings.RenderingTechnology);
        Settings.RenderingTechnology = SciCall_GetTechnology();
        SciCall_SetBufferedDraw(Settings.RenderingTechnology == SC_TECHNOLOGY_DEFAULT);

        int const prevBD = Settings.Bidirectional;
        SciCall_SetBidirectional(Settings.Bidirectional);
        Settings.Bidirectional = SciCall_GetBidirectional();
        
        if ((prevRT != Settings.RenderingTechnology) || (prevBD != Settings.Bidirectional)) {
          UpdateMarginWidth();
        }
      }
      break;

   	case IDM_SET_RTL_LAYOUT_EDIT:
      Settings.EditLayoutRTL = !Settings.EditLayoutRTL;
      SetWindowLayoutRTL(Globals.hwndEdit, Settings.EditLayoutRTL);
      InvalidateStyleRedraw();
      break;

    case IDM_SET_RTL_LAYOUT_DLG:
      Settings.DialogsLayoutRTL = !Settings.DialogsLayoutRTL;
      SetWindowLayoutRTL(Globals.hwndMain, Settings.DialogsLayoutRTL);
      SetWindowLayoutRTL(Globals.hwndToolbar, Settings.DialogsLayoutRTL);
      SetWindowLayoutRTL(Globals.hwndRebar, Settings.DialogsLayoutRTL);
      SetWindowLayoutRTL(Globals.hwndStatus, Settings.DialogsLayoutRTL);
      break;

    case IDM_SET_BIDIRECTIONAL_NONE:
    case IDM_SET_BIDIRECTIONAL_L2R:
    case IDM_SET_BIDIRECTIONAL_R2L:
      {
        SciCall_SetBidirectional(iLoWParam - IDM_SET_BIDIRECTIONAL_NONE);
        Settings.Bidirectional = SciCall_GetBidirectional();
      }
      break;

    case IDM_VIEW_WIN_DARK_MODE:
      Settings.WinThemeDarkMode = !Settings.WinThemeDarkMode;
      InitDarkMode(Settings.WinThemeDarkMode);
      SciCall_SetHScrollbar(false);
      SciCall_SetHScrollbar(true);
      SciCall_SetVScrollbar(false);
      SciCall_SetVScrollbar(true);
      PostMessage(hwnd, WM_THEMECHANGED, 0, 0);
      break;

    case IDM_VIEW_MUTE_MESSAGEBEEP:
      Settings.MuteMessageBeep = !Settings.MuteMessageBeep;
      break;

    case IDM_VIEW_SPLIT_UNDOTYPSEQ_LNBRK:
      Settings.SplitUndoTypingSeqOnLnBreak = !Settings.SplitUndoTypingSeqOnLnBreak;
      break;

    //case IDM_SET_INLINE_IME:
    //  Settings2.IMEInteraction = (Settings2.IMEInteraction == SC_IME_WINDOWED) ? SC_IME_INLINE : SC_IME_WINDOWED;
    //  SciCall_SetIMEInteraction(Settings2.IMEInteraction);
    //  break;

    case IDM_VIEW_SHOWFILENAMEONLY:
    case IDM_VIEW_SHOWFILENAMEFIRST:
    case IDM_VIEW_SHOWFULLPATH:
      Settings.PathNameFormat = iLoWParam - IDM_VIEW_SHOWFILENAMEONLY;
      StringCchCopy(s_wchTitleExcerpt,COUNTOF(s_wchTitleExcerpt),L"");
      UpdateTitleBar();
      break;


    case IDM_VIEW_SHOWEXCERPT:
      EditGetExcerpt(Globals.hwndEdit,s_wchTitleExcerpt,COUNTOF(s_wchTitleExcerpt));
      UpdateTitleBar();
      break;


    case IDM_VIEW_NOSAVERECENT:
      Settings.SaveRecentFiles = !Settings.SaveRecentFiles;
      break;


    case IDM_VIEW_NOPRESERVECARET:
      Settings.PreserveCaretPos = !Settings.PreserveCaretPos;
      break;


    case IDM_VIEW_NOSAVEFINDREPL:
      Settings.SaveFindReplace = !Settings.SaveFindReplace;
      break;


    case IDM_VIEW_SAVEBEFORERUNNINGTOOLS:
      Settings.SaveBeforeRunningTools = !Settings.SaveBeforeRunningTools;
      break;

    case IDM_VIEW_EVALTINYEXPRONSEL:
      Settings.EvalTinyExprOnSelection = !Settings.EvalTinyExprOnSelection;
      UpdateStatusbar(false);
      break;

    case IDM_VIEW_CHANGENOTIFY:
      if (ChangeNotifyDlg(hwnd)) {
        InstallFileWatching(Globals.CurrentFile);
      }
      break;


    case IDM_VIEW_NOESCFUNC:
    case IDM_VIEW_ESCMINIMIZE:
    case IDM_VIEW_ESCEXIT:
      Settings.EscFunction = iLoWParam - IDM_VIEW_NOESCFUNC;
      break;


    case IDM_VIEW_SAVESETTINGS:
      if (IsCmdEnabled(hwnd, IDM_VIEW_SAVESETTINGS)) {
        Settings.SaveSettings = !Settings.SaveSettings;
        if (Globals.bCanSaveIniFile) {
          if (Settings.SaveSettings == Defaults.SaveSettings) {
            IniFileDelete(Globals.IniFile, Constants.Settings_Section, L"SaveSettings", false);
          }
          else {
            IniFileSetBool(Globals.IniFile, Constants.Settings_Section, L"SaveSettings", Settings.SaveSettings);
          }
        }
      }
      break;


    case IDM_VIEW_SAVESETTINGSNOW:
      if (IsCmdEnabled(hwnd, IDM_VIEW_SAVESETTINGSNOW))
      {
        CmdSaveSettingsNow();
      }
      break;


    case IDM_HELP_ONLINEDOCUMENTATION:
      ShellExecute(0, 0, ONLINE_HELP_WEBSITE, 0, 0, SW_SHOW);
      break;

    case IDM_HELP_ABOUT:
      {
        //~HMODULE hRichEdit = LoadLibrary(L"RICHED20.DLL");  // Use RICHEDIT_CONTROL_VER for control in common_res.h
        HMODULE const hRichEdit = LoadLibrary(L"MSFTEDIT.DLL");  // Use "RichEdit50W" for control in common_res.h;
        if (hRichEdit) {
          ThemedDialogBox(Globals.hLngResContainer, MAKEINTRESOURCE(IDD_MUI_ABOUT), hwnd, AboutDlgProc);
          FreeLibrary(hRichEdit);
        }
      }
      break;

    case IDM_SETPASS:
      if (GetFileKey(Globals.hwndEdit)) {
        SetSaveNeeded();
      }
      break;

    case IDM_HELP_CMD:
      DisplayCmdLineHelp(hwnd);
      break;


    case CMD_ESCAPE:
      {
        DocPos const iCurPos = SciCall_GetCurrentPos();
        
        int skipLevel = Settings2.ExitOnESCSkipLevel;

        if (SciCall_AutoCActive()) {
          SciCall_AutoCCancel();
          --skipLevel;
        }
        else if (SciCall_CallTipActive()) {
          CancelCallTip();
          s_bCallTipEscDisabled = true;
          --skipLevel;
        }
        else if (s_bInMultiEditMode) {
          //~_BEGIN_UNDO_ACTION_;
          SciCall_SetIndicatorCurrent(INDIC_NP3_MULTI_EDIT);
          SciCall_IndicatorClearRange(0, Sci_GetDocEndPosition());
          SciCall_ClearSelections();
          //~_END_UNDO_ACTION_;
          SciCall_GotoPos(iCurPos);
          s_bInMultiEditMode = false;
          --skipLevel;
        }

        if ((!SciCall_IsSelectionEmpty() || Sci_IsMultiOrRectangleSelection()) && (skipLevel == Settings2.ExitOnESCSkipLevel)) {
          //~_BEGIN_UNDO_ACTION_;
          Sci_GotoPosChooseCaret(iCurPos);
          //~_END_UNDO_ACTION_;
          skipLevel -= Defaults2.ExitOnESCSkipLevel;
        }

        if ((skipLevel < 0) || (skipLevel == Settings2.ExitOnESCSkipLevel))
        {
          switch (Settings.EscFunction) {
          case 1:
            SendMessage(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
            break;

          case 2:
            CloseApplication();
            break;

          default:
            Sci_GotoPosChooseCaret(iCurPos);
            break;
          }
        }
        SciCall_Cancel();
      }
      break;


    case CMD_SHIFTESC:
      FileSave(false, false, false, false, Flags.bPreserveFileModTime);
    case IDT_FILE_EXIT:
      CloseApplication();
      break;


    case CMD_INSERTNEWLINE:
      {
        _BEGIN_UNDO_ACTION_;
        const DocPos iPos = SciCall_GetCurrentPos();
        const DocLn iLine = SciCall_LineFromPosition(iPos);
        if (iLine <= 0) {
          SciCall_GotoLine(0);
          SciCall_NewLine();
          SciCall_GotoLine(0);
        }
        else {
          SciCall_GotoPos(SciCall_GetLineEndPosition(iLine - 1));
          SciCall_NewLine();
        }
        _END_UNDO_ACTION_;
      }
      break;


    // Newline with toggled auto indent setting
    case CMD_SHIFTCTRLENTER:
      Settings.AutoIndent = !Settings.AutoIndent;
      SciCall_NewLine();
      Settings.AutoIndent = !Settings.AutoIndent;
      break;


    case CMD_CLEAR:
    case IDM_EDIT_CLEAR:
        ///~_BEGIN_UNDO_ACTION_;
        EditDeleteMarkerInSelection();
        SciCall_Clear();
        ///~_END_UNDO_ACTION_;
      break;


    case CMD_ARROW_UP:
      if (Sci_IsMultiSelection())
      {
        SciCall_Cancel();
      }
      SciCall_LineUp();
      break;
    
    case CMD_ARROW_DOWN:
      if (Sci_IsMultiSelection())
      {
        SciCall_Cancel();
      }
      SciCall_LineDown();
      break;
    

    case CMD_SCROLLUP:
      if (Sci_IsMultiSelection())
      {
        SciCall_LineUpExtend();
      }
      else {
        SciCall_LineScrollUp();
      }
      break;


    case CMD_SCROLLDOWN:
      if (Sci_IsMultiSelection())
      {
        SciCall_LineDownExtend();
      }
      else {
        SciCall_LineScrollDown();
      }
      break;


    case CMD_CTRLLEFT:
      if (Sci_IsMultiSelection())
      {
        SciCall_CharLeftExtend();
      }
      else {
        SciCall_WordLeft();
      }
      break;


    case CMD_CTRLRIGHT:
      if (Sci_IsMultiSelection())
      {
        SciCall_CharRightExtend();
      }
      else {
        SciCall_WordRight();
      }
      break;


    case CMD_CTRLBACK:
      {
        DocPos const iPos = SciCall_GetCurrentPos();
        DocPos const iAnchor = SciCall_GetAnchor();
        DocLn  const iLine = SciCall_LineFromPosition(iPos);
        DocPos const iStartPos = SciCall_PositionFromLine(iLine);
        DocPos const iIndentPos = SciCall_GetLineIndentPosition(iLine);

        if (iPos != iAnchor) {
          _BEGIN_UNDO_ACTION_;
          SciCall_SetSel(iPos, iPos);
          _END_UNDO_ACTION_;
        }
        else {
          if (iPos == iStartPos)
            SciCall_DeleteBack();
          else if (iPos <= iIndentPos)
            SciCall_DelLineLeft();
          else
            SciCall_DelWordLeft();
        }
      }
      break;


    case CMD_CTRLDEL:
      {
        const DocPos iPos = SciCall_GetCurrentPos();
        const DocPos iAnchor = SciCall_GetAnchor();
        const DocLn iLine = SciCall_LineFromPosition(iPos);
        const DocPos iStartPos = SciCall_PositionFromLine(iLine);
        const DocPos iEndPos = SciCall_GetLineEndPosition(iLine);

        if (iPos != iAnchor) {
          _BEGIN_UNDO_ACTION_;
          SciCall_SetSel(iPos, iPos);
          _END_UNDO_ACTION_;
        }
        else {
          if (iStartPos != iEndPos) {
            SciCall_DelWordRight();
          } else { // iStartPos == iEndPos
            SciCall_MarkerDelete(Sci_GetCurrentLineNumber(), -1);
            SciCall_LineDelete();
          }
        }
      }
      break;


    case CMD_RECODEDEFAULT:
      {
      if (StrIsNotEmpty(Globals.CurrentFile)) {
        StringCchCopy(tchMaxPathBuffer, COUNTOF(tchMaxPathBuffer), Globals.CurrentFile);
        Encoding_Forced(Settings.DefaultEncoding);
        FileLoad(false, false, true, true, true, false, tchMaxPathBuffer);
      }
      }
      break;


    case CMD_RECODEANSI:
      {
      if (StrIsNotEmpty(Globals.CurrentFile)) {
        StringCchCopy(tchMaxPathBuffer, COUNTOF(tchMaxPathBuffer), Globals.CurrentFile);
        Encoding_Forced(CPI_ANSI_DEFAULT);
        FileLoad(false, false, true, true, true, false, tchMaxPathBuffer);
      }
      }
      break;


    case CMD_RECODEOEM:
      {
      if (StrIsNotEmpty(Globals.CurrentFile)) {
        StringCchCopy(tchMaxPathBuffer, COUNTOF(tchMaxPathBuffer), Globals.CurrentFile);
        Encoding_Forced(CPI_OEM);
        FileLoad(false, false, true, true, true, false, tchMaxPathBuffer);
      }
      }
      break;


    case CMD_RECODEGB18030:
    {
      if (StrIsNotEmpty(Globals.CurrentFile)) {
        StringCchCopy(tchMaxPathBuffer, COUNTOF(tchMaxPathBuffer), Globals.CurrentFile);
        Encoding_Forced(Encoding_GetByCodePage(54936)); // GB18030
        FileLoad(false, false, true, true, true, false, tchMaxPathBuffer);
      }
    }
    break;


    case CMD_RELOADASCIIASUTF8:
      {
      if (StrIsNotEmpty(Globals.CurrentFile))
      {
        StringCchCopy(tchMaxPathBuffer, COUNTOF(tchMaxPathBuffer), Globals.CurrentFile);
        Encoding_Forced(CPI_UTF8);
        FileLoad(false, false, true, true, true, false, tchMaxPathBuffer);
      }
      }
      break;


    case CMD_RELOADFORCEDETECTION:
    {
      if (StrIsNotEmpty(Globals.CurrentFile))
      {
        Encoding_Forced(CPI_NONE);
        StringCchCopy(tchMaxPathBuffer, COUNTOF(tchMaxPathBuffer), Globals.CurrentFile);
        FileLoad(false, false, true, false, false, true, tchMaxPathBuffer);
      }
    }
    break;

    case CMD_RELOADNOFILEVARS:
      {
        if (StrIsNotEmpty(Globals.CurrentFile))
        {
          bool const _fNoFileVariables = Flags.NoFileVariables;
          bool const _bNoEncodingTags = Settings.NoEncodingTags;
          Flags.NoFileVariables = true;
          Settings.NoEncodingTags = true;
          StringCchCopy(tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer),Globals.CurrentFile);
          FileLoad(false,false,true, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, false, tchMaxPathBuffer);
          Flags.NoFileVariables = _fNoFileVariables;
          Settings.NoEncodingTags = _bNoEncodingTags;
        }
      }
      break;


    case CMD_LEXDEFAULT:
      Style_SetDefaultLexer(Globals.hwndEdit);
      break;


    //case CMD_LEXHTML:
    //  Style_SetHTMLLexer(Globals.hwndEdit);
    //  break;


    //case CMD_LEXXML:
    //  Style_SetXMLLexer(Globals.hwndEdit);
    //  break;


    case CMD_INSERT_TIMESTAMP:
      EditInsertDateTimeStrg(true, true);
      break;

    case CMD_UPDATE_TIMESTAMPS:
      EditUpdateTimestamps();
      break;


    case IDM_HELP_ADMINEXE:
      DialogAdminExe(hwnd, true);
      break;

    case IDM_HELP_UPDATEWEBSITE:
      DialogAdminExe(hwnd, false);
      break;

    case CMD_WEBACTION1:
    case CMD_WEBACTION2:
      {
        StringCchCopyW(tchMaxPathBuffer, COUNTOF(tchMaxPathBuffer),
          (iLoWParam == CMD_WEBACTION1) ? Settings2.WebTemplate1 : Settings2.WebTemplate2);

        if (StringCchLenW(tchMaxPathBuffer,0) > 0) {

          WCHAR wszSelection[HUGE_BUFFER] = { L'\0' };
          size_t const cchSelection = EditGetSelectedText(wszSelection, HUGE_BUFFER);

          if (1 < cchSelection)
          {
            // Check lpszSelection and truncate bad WCHARs
            WCHAR* lpsz = StrChr(wszSelection, L'\r');
            if (lpsz) *lpsz = L'\0';

            lpsz = StrChr(wszSelection, L'\n');
            if (lpsz) *lpsz = L'\0';

            lpsz = StrChr(wszSelection, L'\t');
            if (lpsz) *lpsz = L'\0';

            int cmdsz = (512 + COUNTOF(tchMaxPathBuffer) + MAX_PATH + 32);
            LPWSTR lpszCommand = AllocMem(sizeof(WCHAR) * cmdsz, HEAP_ZERO_MEMORY);
            StringCchPrintf(lpszCommand, cmdsz, tchMaxPathBuffer, wszSelection);
            ExpandEnvironmentStringsEx(lpszCommand, cmdsz);

            WCHAR wchDirectory[MAX_PATH] = { L'\0' };
            if (StrIsNotEmpty(Globals.CurrentFile)) {
              StringCchCopy(wchDirectory, COUNTOF(wchDirectory), Globals.CurrentFile);
              PathCchRemoveFileSpec(wchDirectory, COUNTOF(wchDirectory));
            }

            SHELLEXECUTEINFO sei;
            ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
            sei.cbSize = sizeof(SHELLEXECUTEINFO);
            sei.fMask = SEE_MASK_NOZONECHECKS;
            sei.hwnd = NULL;
            sei.lpVerb = NULL;
            sei.lpFile = lpszCommand;
            sei.lpParameters = NULL;
            sei.lpDirectory = wchDirectory;
            sei.nShow = SW_SHOWNORMAL;
            ShellExecuteEx(&sei);

            FreeMem(lpszCommand);
          }
        }
      }
      break;

/* ~~~
    case CMD_INCLINELIMIT:
    case CMD_DECLINELIMIT:
      if (!Settings.MarkLongLines)
        SendWMCommand(hwnd, IDM_VIEW_LONGLINEMARKER);
      else {
        if (iLoWParam == CMD_INCLINELIMIT)
          Settings.LongLinesLimit++;
        else
          Settings.LongLinesLimit--;
        Globals.fvCurFile.iLongLinesLimit = clampi(Settings.LongLinesLimit, 0, LONG_LINES_MARKER_LIMIT);
        SendMessage(Globals.hwndEdit,SCI_SETEDGECOLUMN,Settings.LongLinesLimit,0);
        //Globals.fvCurFile.iLongLinesLimit = Settings.LongLinesLimit;
      }
      break;
~~~ */
      

    case CMD_STRINGIFY:
      {
        EditEncloseSelection(L"'", L"'");
      }
      break;


    case CMD_STRINGIFY2:
      {
        EditEncloseSelection(L"\"", L"\"");
      }
      break;


    case CMD_EMBRACE:
      {
        EditEncloseSelection(L"(", L")");
      }
      break;


    case CMD_EMBRACE2:
      {
        EditEncloseSelection(L"[", L"]");
      }
      break;


    case CMD_EMBRACE3:
      {
        EditEncloseSelection(L"{", L"}");
      }
      break;


    case CMD_EMBRACE4:
      {
        EditEncloseSelection(L"`", L"`");
      }
      break;


    case CMD_INCREASENUM:
      EditModifyNumber(Globals.hwndEdit,true);
      break;


    case CMD_DECREASENUM:
      EditModifyNumber(Globals.hwndEdit,false);
      break;


    case CMD_TOGGLETITLE:
      EditGetExcerpt(Globals.hwndEdit,s_wchTitleExcerpt,COUNTOF(s_wchTitleExcerpt));
      UpdateTitleBar();
      break;


    case CMD_JUMP2SELSTART:
      if (!EditSetCaretToSelectionStart() && Sci_IsMultiOrRectangleSelection()) {
        size_t const n = SciCall_GetSelections();
        DocLn const lineStart = SciCall_LineFromPosition(SciCall_GetSelectionNCaret(0));
        DocPos const beg = SciCall_PositionFromLine(lineStart);
        SciCall_ClearSelections(); // needed to reset mode
        SciCall_SetSelection(beg, beg);
        for (size_t i = 1; i < n; ++i) {
          DocPos const pos = SciCall_PositionFromLine(lineStart + i);
          SciCall_AddSelection(pos, pos);
        }
      }
      Sci_ScrollChooseCaret();
      break;

    case CMD_JUMP2SELEND:
      if (!EditSetCaretToSelectionEnd() && Sci_IsMultiOrRectangleSelection()) {
        size_t const n = SciCall_GetSelections();
        DocLn const lineStart = SciCall_LineFromPosition(SciCall_GetSelectionNCaret(0));
        DocPos const beg = SciCall_GetLineEndPosition(lineStart);
        SciCall_ClearSelections(); // needed to reset mode
        SciCall_SetSelection(beg, beg);
        for (size_t i = 1; i < n; ++i) {
          DocPos const pos = SciCall_GetLineEndPosition(lineStart + i);
          SciCall_AddSelection(pos, pos);
        }
      }
      Sci_ScrollChooseCaret();
      break;


    case CMD_COPYPATHNAME: {

        WCHAR *pszCopy;
        WCHAR tchUntitled[32] = { L'\0' };
        if (StrIsNotEmpty(Globals.CurrentFile))
          pszCopy = Globals.CurrentFile;
        else {
          GetLngString(IDS_MUI_UNTITLED, tchUntitled, COUNTOF(tchUntitled));
          pszCopy = tchUntitled;
        }
        SetClipboardTextW(hwnd, pszCopy, StringCchLen(pszCopy,0));
      }
      break;


    case CMD_COPYWINPOS: {
        WININFO wi = GetMyWindowPlacement(Globals.hwndMain,NULL);
        StringCchPrintf(tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer),L"/pos %i,%i,%i,%i,%i",wi.x,wi.y,wi.cx,wi.cy,wi.max);
        SetClipboardTextW(hwnd, tchMaxPathBuffer, StringCchLen(tchMaxPathBuffer, 0));
      }
      break;


    case CMD_INITIALWINPOS:
      SnapToWinInfoPos(hwnd, g_IniWinInfo, SCR_NORMAL);
      break;

    case CMD_FULLSCRWINPOS:
      {
        WININFO const wi = GetMyWindowPlacement(Globals.hwndMain, NULL);
        SnapToWinInfoPos(hwnd, wi, SCR_FULL_SCREEN);
      }
      break;

    case CMD_DEFAULTWINPOS:
      SnapToWinInfoPos(hwnd, g_DefWinInfo, SCR_NORMAL);
      break;

    case CMD_SAVEASDEFWINPOS:
      {
        WININFO const wi = GetMyWindowPlacement(Globals.hwndMain, NULL);
        WCHAR tchDefWinPos[80];
        StringCchPrintf(tchDefWinPos, COUNTOF(tchDefWinPos), L"%i,%i,%i,%i,%i", wi.x, wi.y, wi.cx, wi.cy, wi.max);
        if (Globals.bCanSaveIniFile) {
          IniFileSetString(Globals.IniFile, Constants.Settings2_Section, L"DefaultWindowPosition", tchDefWinPos);
        }
        g_DefWinInfo = wi; //GetWinInfoByFlag(-1); // use current win pos as new default
      }
      break;

    case CMD_CLEARSAVEDWINPOS:
      g_DefWinInfo = GetFactoryDefaultWndPos(2);
      IniFileDelete(Globals.IniFile, Constants.Settings2_Section, L"DefaultWindowPosition", false);
    break;

    case CMD_OPENINIFILE:
      if (StrIsNotEmpty(Globals.IniFile)) {
        SaveAllSettings(false);
        FileLoad(false, false, false, false, true, false, Globals.IniFile);
      }
      break;

    case CMD_OPEN_HYPERLINK:
        HandleHotSpotURLClicked(SciCall_GetCurrentPos(), (OPEN_WITH_BROWSER | OPEN_WITH_NOTEPAD3));
      break;

    case CMD_FOLDJUMPDOWN:
      EditFoldCmdKey(DOWN, SNIFF);
      break;

    case CMD_FOLDJUMPUP:
      EditFoldCmdKey(UP, SNIFF);
      break;

    case CMD_FOLDCOLLAPSE:
      EditFoldCmdKey(NONE, FOLD);
      break;

    case CMD_FOLDEXPAND:
      EditFoldCmdKey(NONE, EXPAND);
      break;


    case IDT_FILE_NEW:
      if (IsCmdEnabled(hwnd,IDM_FILE_NEW))
        SendWMCommand(hwnd, IDM_FILE_NEW);
      else
        SimpleBeep();
      break;


    case IDT_FILE_OPEN:
      if (IsCmdEnabled(hwnd,IDM_FILE_OPEN))
        SendWMCommand(hwnd, IDM_FILE_OPEN);
      else
        SimpleBeep();
      break;


    case IDT_FILE_BROWSE:
      if (IsCmdEnabled(hwnd,IDM_FILE_BROWSE))
        SendWMCommand(hwnd, IDM_FILE_BROWSE);
      else
        SimpleBeep();
      break;


    case IDT_FILE_RECENT:
      if (IsCmdEnabled(hwnd,IDM_FILE_RECENT))
        SendWMCommand(hwnd, IDM_FILE_RECENT);
      else
        SimpleBeep();
      break;

    case IDT_FILE_SAVE:
      if (IsCmdEnabled(hwnd,IDM_FILE_SAVE))
        SendWMCommand(hwnd, IDM_FILE_SAVE);
      else
        SimpleBeep();
      break;


    case IDT_EDIT_UNDO:
      if (IsCmdEnabled(hwnd,IDM_EDIT_UNDO))
        SendWMCommand(hwnd, IDM_EDIT_UNDO);
      else
        SimpleBeep();
      break;


    case IDT_EDIT_REDO:
      if (IsCmdEnabled(hwnd,IDM_EDIT_REDO))
        SendWMCommand(hwnd, IDM_EDIT_REDO);
      else
        SimpleBeep();
      break;


    case IDT_EDIT_CUT:
      if (IsCmdEnabled(hwnd,IDM_EDIT_CUT))
        SendWMCommand(hwnd, IDM_EDIT_CUT);
      else
        SimpleBeep();
        //SendWMCommand(hwnd, IDM_EDIT_CUTLINE);
      break;


    case IDT_EDIT_COPY:
      if (IsCmdEnabled(hwnd,IDM_EDIT_COPY))
        SendWMCommand(hwnd, IDM_EDIT_COPY);
      else
        SendWMCommand(hwnd, IDM_EDIT_COPYALL);
      break;


    case IDT_EDIT_PASTE:
      if (IsCmdEnabled(hwnd,IDM_EDIT_PASTE))
        SendWMCommand(hwnd, IDM_EDIT_PASTE);
      else
        SimpleBeep();
      break;


    case IDT_EDIT_FIND:
      if (IsCmdEnabled(hwnd,IDM_EDIT_FIND))
        SendWMCommand(hwnd, IDM_EDIT_FIND);
      else
        SimpleBeep();
      break;


    case IDT_EDIT_REPLACE:
      if (IsCmdEnabled(hwnd,IDM_EDIT_REPLACE))
        SendWMCommand(hwnd, IDM_EDIT_REPLACE);
      else
        SimpleBeep();
      break;


    case IDT_GREP_WIN_TOOL:
      if (IsCmdEnabled(hwnd, IDM_GREP_WIN_SEARCH))
        SendWMCommand(hwnd, IDM_GREP_WIN_SEARCH);
      else
        SimpleBeep();
      break;


    case IDT_VIEW_WORDWRAP:
      if (IsCmdEnabled(hwnd,IDM_VIEW_WORDWRAP))
        SendWMCommand(hwnd, IDM_VIEW_WORDWRAP);
      else
        SimpleBeep();
      break;


    case IDT_VIEW_ZOOMIN:
      if (IsCmdEnabled(hwnd,IDM_VIEW_ZOOMIN))
        SendWMCommand(hwnd, IDM_VIEW_ZOOMIN);
      else
        SimpleBeep();
      break;


    case IDT_VIEW_ZOOMOUT:
      if (IsCmdEnabled(hwnd,IDM_VIEW_ZOOMOUT))
        SendWMCommand(hwnd, IDM_VIEW_ZOOMOUT);
      else
        SimpleBeep();
      break;


    case IDT_VIEW_CHASING_DOCTAIL:
      if (IsCmdEnabled(hwnd, IDM_VIEW_CHASING_DOCTAIL))
        SendWMCommand(hwnd, IDM_VIEW_CHASING_DOCTAIL);
      else
        SimpleBeep();
      break;


    case IDT_VIEW_SCHEME:
      if (IsCmdEnabled(hwnd,IDM_VIEW_SCHEME))
        SendWMCommand(hwnd, IDM_VIEW_SCHEME);
      else
        SimpleBeep();
      break;


    case IDT_VIEW_SCHEMECONFIG:
      if (IsCmdEnabled(hwnd,IDM_VIEW_SCHEMECONFIG))
        SendWMCommand(hwnd, IDM_VIEW_SCHEMECONFIG);
      else
        SimpleBeep();
      break;


    case IDT_FILE_SAVEAS:
      if (IsCmdEnabled(hwnd,IDM_FILE_SAVEAS))
        SendWMCommand(hwnd, IDM_FILE_SAVEAS);
      else
        SimpleBeep();
      break;


    case IDT_FILE_SAVECOPY:
      if (IsCmdEnabled(hwnd,IDM_FILE_SAVECOPY))
        SendWMCommand(hwnd, IDM_FILE_SAVECOPY);
      else
        SimpleBeep();
      break;


    case IDT_EDIT_CLEAR:
      if (IsCmdEnabled(hwnd,IDM_EDIT_CLEAR))
        SendWMCommand(hwnd, IDM_EDIT_CLEAR);
      else
        SendMessage(Globals.hwndEdit,SCI_CLEARALL,0,0);
      break;


    case IDT_FILE_PRINT:
      if (IsCmdEnabled(hwnd,IDM_FILE_PRINT))
        SendWMCommand(hwnd, IDM_FILE_PRINT);
      else
        SimpleBeep();
      break;


    case IDT_FILE_OPENFAV:
      if (IsCmdEnabled(hwnd,IDM_FILE_OPENFAV))
        SendWMCommand(hwnd, IDM_FILE_OPENFAV);
      else
        SimpleBeep();
      break;


    case IDT_FILE_ADDTOFAV:
      if (IsCmdEnabled(hwnd,IDM_FILE_ADDTOFAV))
        SendWMCommand(hwnd, IDM_FILE_ADDTOFAV);
      else
        SimpleBeep();
      break;


    case IDT_VIEW_TOGGLEFOLDS:
      if (IsCmdEnabled(hwnd,IDM_VIEW_TOGGLEFOLDS))
        SendWMCommand(hwnd, IDM_VIEW_TOGGLEFOLDS);
      else
        SimpleBeep();
      break;

      
    case IDT_VIEW_TOGGLE_VIEW:
      if (IsCmdEnabled(hwnd,IDM_VIEW_TOGGLE_VIEW))
        SendWMCommand(hwnd, IDM_VIEW_TOGGLE_VIEW);
      else
        SimpleBeep();
      break;


    case IDT_VIEW_PIN_ON_TOP:
      if (IsCmdEnabled(hwnd, IDM_VIEW_ALWAYSONTOP))
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_VIEW_ALWAYSONTOP, 1), 0);
      else
        SimpleBeep();
      break;


    case IDT_FILE_LAUNCH:
      if (IsCmdEnabled(hwnd,IDM_FILE_LAUNCH))
        SendWMCommand(hwnd, IDM_FILE_LAUNCH);
      else
        SimpleBeep();
      break;

    default:
      return DefWindowProc(hwnd, umsg, wParam, lParam);
  }
  return FALSE;
}


//=============================================================================
//
//  MsgSysCommand() - Handles WM_SYSCOMMAND
//
LRESULT MsgSysCommand(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  switch (wParam) {
  case SC_MINIMIZE:
    ShowOwnedPopups(hwnd, false);
    if (Settings.MinimizeToTray) {
      MinimizeWndToTray(hwnd);
      ShowNotifyIcon(hwnd, true);
      SetNotifyIconTitle(hwnd);
      return FALSE; // swallowed
    }
    break;

  case SC_RESTORE:
    {
      LRESULT lrv = DefWindowProc(hwnd, umsg, wParam, lParam);
      ShowOwnedPopups(hwnd, true);
      return(lrv);
    }

  default:
    break;
  }
  return DefWindowProc(hwnd, umsg, wParam, lParam);
}


//=============================================================================
//
//  HandlePosChange()
//
void HandlePosChange()
{
  static DocPos prevPosition = -1;
  DocPos const curPos = SciCall_GetCurrentPos();
  if (curPos == prevPosition) { return; }

  prevPosition = curPos;
}


//=============================================================================
//
//  HandleDWellStartEnd()
//
static DocPos prevCursorPosition = -1;

#define ARGB_TO_COLREF(X) (RGB(((X) >> 16) & SC_ALPHA_OPAQUE, ((X) >>  8) & SC_ALPHA_OPAQUE, (X) & SC_ALPHA_OPAQUE))
#define RGBA_TO_COLREF(X) (RGB(((X) >> 24) & SC_ALPHA_OPAQUE, ((X) >> 16) & SC_ALPHA_OPAQUE, ((X) >> 8) & SC_ALPHA_OPAQUE))
#define BGRA_TO_COLREF(X) (RGB(((X) >>  8) & SC_ALPHA_OPAQUE, ((X) >> 16) & SC_ALPHA_OPAQUE, ((X) >> 24) & SC_ALPHA_OPAQUE))
#define ARGB_GET_ALPHA(A) (((A) >> 24) & SC_ALPHA_OPAQUE)
#define RGBA_GET_ALPHA(A) ((A) & SC_ALPHA_OPAQUE)
#define BGRA_GET_ALPHA(A) RGBA_GET_ALPHA(A)
                               
// ----------------------------------------------------------------------------

void HandleDWellStartEnd(const DocPos position, const UINT uid)
{
  static DocPos prevStartPosition = -1;
  static DocPos prevEndPosition = -1;

  if (position >= 0) {
    if (prevCursorPosition < 0) { prevCursorPosition = position; }
    if (prevStartPosition < 0) { prevStartPosition = position; }
    if (prevEndPosition < 0) { prevEndPosition = position; }
  }

  int indicator_id = INDICATOR_CONTAINER;

  switch (uid)
  {
    case SCN_DWELLSTART:
    {
      if (position < 0) { CancelCallTip(); prevCursorPosition = -1;  return; }
        
      if (Settings.HyperlinkHotspot) {
        if (SciCall_IndicatorValueAt(INDIC_NP3_HYPERLINK, position) > 0) {
          indicator_id = INDIC_NP3_HYPERLINK;
          if (position != prevCursorPosition) {
            CancelCallTip();
          }
        }
      }
      if (IsColorDefHotspotEnabled()) {
        if (SciCall_IndicatorValueAt(INDIC_NP3_COLOR_DEF, position) > 0) {
          indicator_id = INDIC_NP3_COLOR_DEF;
        }
      }
      if (Settings.HighlightUnicodePoints) {
        if (SciCall_IndicatorValueAt(INDIC_NP3_UNICODE_POINT, position) > 0) {
          indicator_id = INDIC_NP3_UNICODE_POINT;
        }
      }
     
      switch (indicator_id) 
      {
        case INDIC_NP3_HYPERLINK:
          if (!Settings.ShowHypLnkToolTip || SciCall_CallTipActive()) { return; }
          break;

        case INDIC_NP3_UNICODE_POINT:
          if (!Settings.HighlightUnicodePoints || SciCall_CallTipActive()) { return; }
          break;

        case INDIC_NP3_COLOR_DEF:
          // ok
          break;
  
        // nothing to do for these indicators
        case INDICATOR_CONTAINER:
        default:
          return;
      }

      // ----------------------------------------------------------------------

      if ((position < prevStartPosition) || (position > prevEndPosition)) { s_bCallTipEscDisabled = false; }

      //SciCall_SetCursor(SC_NP3_CURSORHAND);

      DocPos const firstPos = SciCall_IndicatorStart(indicator_id, position);
      DocPos const lastPos = SciCall_IndicatorEnd(indicator_id, position);
      DocPos const length = (lastPos - firstPos);

      // WebLinks and Color Refs are ASCII only - No need for UTF-8 conversion here

      if (INDIC_NP3_HYPERLINK == indicator_id) 
      {
        if (!s_bCallTipEscDisabled) {
          char chText[MIDSZ_BUFFER] = { '\0' };
          // No need for UTF-8 conversion here and 
          StringCchCopyNA(chText, COUNTOF(chText), SciCall_GetRangePointer(firstPos, length), length);
          StrTrimA(chText, " \t\n\r");
          if (StrIsEmptyA(chText)) { break; }

          WCHAR wchCalltipAdd[SMALL_BUFFER] = { L'\0' };
          if (StrStrIA(chText, "file:") == chText) {
            GetLngString(IDS_MUI_URL_OPEN_FILE, wchCalltipAdd, COUNTOF(wchCalltipAdd));
          }
          else {
            GetLngString(IDS_MUI_URL_OPEN_BROWSER, wchCalltipAdd, COUNTOF(wchCalltipAdd));
          }
          CHAR  chAdd[LARGE_BUFFER] = { L'\0' };
          WideCharToMultiByte(Encoding_SciCP, 0, wchCalltipAdd, -1, chAdd, (int)COUNTOF(chAdd), NULL, NULL);

          char chCallTip[HUGE_BUFFER] = { '\0' };
          StringCchCatA(chCallTip, COUNTOF(chCallTip), chText);
          StringCchCatA(chCallTip, COUNTOF(chCallTip), chAdd);
          //SciCall_CallTipSetPosition(true);
          SciCall_CallTipShow(position, chCallTip);
          SciCall_CallTipSetHlt(0, (int)length);
          Globals.CallTipType = CT_DWELL;
        }
      }
      else if (INDIC_NP3_COLOR_DEF == indicator_id)
      {
        char chText[MICRO_BUFFER] = { '\0' };
        // Color Refs are ASCII only - No need for UTF-8 conversion here
        StringCchCopyNA(chText, COUNTOF(chText), SciCall_GetRangePointer(firstPos, length), length);
        unsigned int iValue = 0;
        if (sscanf_s(&chText[1], "%x", &iValue) == 1) 
        {
          COLORREF rgb = 0x000000;
          int alpha = SC_ALPHA_OPAQUE;
          if (length >= 8) // ARGB, RGBA, BGRA
          {
            switch (Settings.ColorDefHotspot) {
              case 1:
                rgb   = ARGB_TO_COLREF(iValue);
                alpha = ARGB_GET_ALPHA(iValue);
                break;
              case 2:
                rgb   = RGBA_TO_COLREF(iValue);
                alpha = RGBA_GET_ALPHA(iValue);
                break;
              case 3:
                rgb   = BGRA_TO_COLREF(iValue);
                alpha = BGRA_GET_ALPHA(iValue);
                break;
              case 0:
              default:
                break;
            }
          }
          else // RGB
          {
            rgb   = RGB((iValue >> 16) & 0xFF, (iValue >> 8) & 0xFF, iValue & 0xFF);
            alpha = SC_ALPHA_OPAQUE;
          }
          COLORREF const fgr = CalcContrastColor(rgb, alpha);

          SciCall_SetIndicatorCurrent(INDIC_NP3_COLOR_DEF_T);
          SciCall_IndicatorFillRange(firstPos, length);
          SciCall_IndicSetHoverFore(INDIC_NP3_COLOR_DEF_T, fgr);

          SciCall_IndicSetAlpha(INDIC_NP3_COLOR_DEF, Sci_ClampAlpha(alpha));
          SciCall_IndicSetHoverFore(INDIC_NP3_COLOR_DEF, rgb);
        }
      }
      else if (INDIC_NP3_UNICODE_POINT == indicator_id)
      {
        if (!s_bCallTipEscDisabled) {
          char chHex2Char[MIDSZ_BUFFER] = {'\0'};
          // No need for UTF-8 conversion here and
          StringCchCopyNA(chHex2Char, COUNTOF(chHex2Char), SciCall_GetRangePointer(firstPos, length), length);
          //StrTrimA(chHex2Char, " \t\n\r");

          Hex2Char(chHex2Char, COUNTOF(chHex2Char));
          if (StrIsEmptyA(chHex2Char)) { break; }

          //SciCall_CallTipSetPosition(true);
          SciCall_CallTipShow(position, chHex2Char);
          SciCall_CallTipSetHlt(0, (int)length);
          Globals.CallTipType = CT_DWELL;
        }
      }

      prevCursorPosition = position;
      prevStartPosition = firstPos;
      prevEndPosition = lastPos;
    }
    break;

    case SCN_DWELLEND:
    {
      if ((position >= prevStartPosition) && ((position <= prevEndPosition))) { return; } // avoid flickering
      
      CancelCallTip();

      DocPos const curPos = SciCall_GetCurrentPos();
      if ((curPos >= prevStartPosition) && ((curPos <= prevEndPosition))) { return; } // no change for if caret in range
      s_bCallTipEscDisabled = false;
      prevCursorPosition = -1;

      // clear SCN_DWELLSTART visual styles
      SciCall_SetIndicatorCurrent(INDIC_NP3_COLOR_DEF_T);
      SciCall_IndicatorClearRange(0, Sci_GetDocEndPosition());

      SciCall_IndicSetAlpha(INDIC_NP3_COLOR_DEF, SC_ALPHA_TRANSPARENT);
      SciCall_IndicSetFore(INDIC_NP3_COLOR_DEF, 0);

      HandlePosChange();
    }
    break;

    default:
      break;
  }
}

//=============================================================================
//
//  HandleHotSpotURLClicked()
//
//
bool HandleHotSpotURLClicked(const DocPos position, const HYPERLINK_OPS operation)
{
  if (position < 0) { return false; }

  //~PostMessage(Globals.hwndEdit, WM_LBUTTONUP, MK_LBUTTON, 0);
  CancelCallTip();

  bool bHandled = false;
  if (SciCall_IndicatorValueAt(INDIC_NP3_HYPERLINK, position))
  {
    DocPos const firstPos = SciCall_IndicatorStart(INDIC_NP3_HYPERLINK, position);
    DocPos const lastPos = SciCall_IndicatorEnd(INDIC_NP3_HYPERLINK, position);
    DocPos const length = min_p(lastPos - firstPos, INTERNET_MAX_URL_LENGTH);

    if (length < 4) { return false; }

    const char* pszText = (const char*)SciCall_GetRangePointer(firstPos, length);

    WCHAR szTextW[INTERNET_MAX_URL_LENGTH + 1];
    ptrdiff_t const cchTextW = MultiByteToWideChar(Encoding_SciCP, 0, pszText, (int)length, szTextW, COUNTOF(szTextW));
    szTextW[cchTextW] = L'\0';
    StrTrimW(szTextW, L" \r\n\t");

    const WCHAR* chkPreFix = L"file://";

    if (operation & SELECT_HYPERLINK)
    {
      SciCall_SetSelection(firstPos, lastPos);
      bHandled = true;
    }
    else if (operation & COPY_HYPERLINK)
    {
      if (cchTextW > 0) {
        DWORD cchEscapedW = (DWORD)(length * 3);
        LPWSTR pszEscapedW = (LPWSTR)AllocMem(cchEscapedW * sizeof(WCHAR), HEAP_ZERO_MEMORY);
        if (pszEscapedW) {
          //~UrlEscape(szTextW, pszEscapedW, &cchEscapedW, (URL_BROWSER_MODE | URL_ESCAPE_AS_UTF8));
          UrlEscapeEx(szTextW, pszEscapedW, &cchEscapedW, false);
          SetClipboardTextW(Globals.hwndMain, pszEscapedW, cchEscapedW);
          FreeMem(pszEscapedW);
          bHandled = true;
        }
      }
    }
    else if ((operation & OPEN_WITH_NOTEPAD3) && (StrStrI(szTextW, chkPreFix) == szTextW))
    {
      size_t const lenPfx = StringCchLenW(chkPreFix, 0);
      WCHAR* szFileName = &(szTextW[lenPfx]);
      szTextW[lenPfx + MAX_PATH] = L'\0'; // limit length
      StrTrimW(szFileName, L"/");

      PathCanonicalizeEx(szFileName, (DWORD)(COUNTOF(szTextW) - lenPfx));
      if (PathIsDirectory(szFileName))
      {
        WCHAR tchFile[MAX_PATH] = { L'\0' };
        if (OpenFileDlg(Globals.hwndMain, tchFile, COUNTOF(tchFile), szFileName))
        {
          FileLoad(false, false, false, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, false, tchFile);
        }
      }
      else {
        FileLoad(false, false, false, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, false, szFileName);
      }
      bHandled = true;
    }
    else if (operation & OPEN_WITH_BROWSER) // open in web browser
    {
      WCHAR wchDirectory[MAX_PATH] = { L'\0' };
      if (StrIsNotEmpty(Globals.CurrentFile)) {
        StringCchCopy(wchDirectory, COUNTOF(wchDirectory), Globals.CurrentFile);
        PathCchRemoveFileSpec(wchDirectory, COUNTOF(wchDirectory));
      }

      SHELLEXECUTEINFO sei;
      ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
      sei.cbSize = sizeof(SHELLEXECUTEINFO);
      sei.fMask = SEE_MASK_NOZONECHECKS;
      sei.hwnd = NULL;
      sei.lpVerb = NULL;
      sei.lpFile = szTextW;
      sei.lpParameters = NULL;
      sei.lpDirectory = wchDirectory;
      sei.nShow = SW_SHOWNORMAL;
      ShellExecuteEx(&sei);

      bHandled = true;
    }
  }

  if (!(operation & SELECT_HYPERLINK)) { SciCall_SetEmptySelection(position); }

  return bHandled;
}


//=============================================================================
//
//  HandleColorDefClicked()
//
void HandleColorDefClicked(HWND hwnd, const DocPos position)
{
  if (SciCall_IndicatorValueAt(INDIC_NP3_COLOR_DEF, position) == 0) { return; }

  DocPos const firstPos = SciCall_IndicatorStart(INDIC_NP3_COLOR_DEF, position);
  DocPos const lastPos = SciCall_IndicatorEnd(INDIC_NP3_COLOR_DEF, position);
  DocPos const length = (lastPos - firstPos);

  char chText[32] = { '\0' };
  // Color Refs are ASCII only - No need for UTF-8 conversion here
  StringCchCopyNA(chText, COUNTOF(chText), SciCall_GetRangePointer(firstPos, length), length);
  unsigned int iValue = 0;
  if (sscanf_s(&chText[1], "%x", &iValue) == 1)
  {
    COLORREF rgbCur = 0x000000;
    BYTE     alpha  = 0xFF;
    if (length >= 8) // ARGB, RGBA, BGRA
    {
      switch (Settings.ColorDefHotspot) {
        case 1:
          rgbCur = ARGB_TO_COLREF(iValue);
          alpha  = ARGB_GET_ALPHA(iValue);
          break;
        case 2:
          rgbCur = RGBA_TO_COLREF(iValue);
          alpha  = RGBA_GET_ALPHA(iValue);
          break;
        case 3:
          rgbCur = BGRA_TO_COLREF(iValue);
          alpha  = BGRA_GET_ALPHA(iValue);
          break;
        case 0:
        default:
          break;
      }
    }
    else // RGB
    {
      rgbCur = RGB((iValue >> 16) & 0xFF, (iValue >> 8) & 0xFF, iValue & 0xFF);
      alpha  = 0xFF;
    }

    CHOOSECOLOR cc;
    ZeroMemory(&cc, sizeof(CHOOSECOLOR));
    cc.lStructSize = sizeof(CHOOSECOLOR);
    cc.hwndOwner = hwnd;
    cc.rgbResult = rgbCur;
    cc.lpCustColors = g_colorCustom;
    //cc.Flags = CC_FULLOPEN | CC_RGBINIT | CC_SOLIDCOLOR;
    cc.Flags = CC_FULLOPEN | CC_RGBINIT | CC_ANYCOLOR;

    if (!ChooseColor(&cc)) { return; }

    COLORREF const rgbNew = cc.rgbResult;

    CHAR wchColor[32] = { L'\0' };
    if (length >= 8) {
      switch (Settings.ColorDefHotspot) {
        case 1:
          StringCchPrintfA(wchColor, COUNTOF(wchColor), "#%02X%02X%02X%02X", (int)(alpha),
                           (int)GetRValue(rgbNew), (int)GetGValue(rgbNew), (int)GetBValue(rgbNew));
          break;
        case 2:
          StringCchPrintfA(wchColor, COUNTOF(wchColor), "#%02X%02X%02X%02X",
                           (int)GetRValue(rgbNew), (int)GetGValue(rgbNew), (int)GetBValue(rgbNew),
                           (int)(alpha));
          break;
        case 3:
          StringCchPrintfA(wchColor, COUNTOF(wchColor), "#%02X%02X%02X%02X",
                           (int)GetBValue(rgbNew), (int)GetGValue(rgbNew), (int)GetRValue(rgbNew),
                           (int)(alpha));
          break;
        case 0:
        default:
          break;
      }
    }
    else {
      StringCchPrintfA(wchColor, COUNTOF(wchColor), "#%02X%02X%02X",
                       (int)GetRValue(rgbNew), (int)GetGValue(rgbNew), (int)GetBValue(rgbNew));
    }

    DocPos const saveTargetBeg = SciCall_GetTargetStart();
    DocPos const saveTargetEnd = SciCall_GetTargetEnd();

    SciCall_SetTargetRange(firstPos, lastPos);
    SciCall_ReplaceTarget(length, wchColor);

    SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore

    EditUpdateVisibleIndicators();
  }
}


//=============================================================================
//
//  _HandleAutoIndent()
//
static void _HandleAutoIndent(int const charAdded)
{
    // TODO: handle indent after '{' and un-indent on '}' in C/C++ ?
    // in CRLF mode handle LF only...
    int const eol_mode = SciCall_GetEOLMode();
    if (((SC_EOL_CRLF == eol_mode) && (charAdded != '\r')) || (SC_EOL_CRLF != eol_mode)) {
        DocLn const iCurLine = Sci_GetCurrentLineNumber();

        // Move bookmark along with line if inserting lines (pressing return within indent area of line) because Scintilla does not do this for us
        if (iCurLine > 0) {
            //~DocPos const iPrevLineLength = Sci_GetNetLineLength(iCurLine - 1);
            if (SciCall_GetLineEndPosition(iCurLine - 1) == SciCall_GetLineIndentPosition(iCurLine - 1)) {
                int const bitmask = SciCall_MarkerGet(iCurLine - 1) & bitmask32_n(MARKER_NP3_BOOKMARK + 1); // all bookmarks 
                if (bitmask) {
                    SciCall_MarkerDelete((iCurLine - 1), -1);
                    SciCall_MarkerAddSet(iCurLine, bitmask);
                }
            }

            //~if (iLineLength <= 2)
            {
                DocLn const  iPrevLine       = iCurLine - 1;
                DocPos const iPrevLineLength = SciCall_LineLength(iPrevLine);
                char*        pLineBuf        = (char*)AllocMem(iPrevLineLength + 1, HEAP_ZERO_MEMORY);
                if (pLineBuf) {
                    SciCall_GetLine_Safe(iPrevLine, pLineBuf);
                    for (char* pPos = pLineBuf; *pPos; pPos++) {
                        if ((*pPos != ' ') && (*pPos != '\t')) {
                            *pPos = '\0';
                            break;
                        }
                    }
                    if (*pLineBuf) {
                        _BEGIN_UNDO_ACTION_;
                        SciCall_AddText((DocPos)StringCchLenA(pLineBuf, SizeOfMem(pLineBuf)), pLineBuf);
                        _END_UNDO_ACTION_;
                    }
                    FreeMem(pLineBuf);
                }
            }
        }
    }
}


//=============================================================================
//
//  _HandleAutoCloseTags()
//
static void  _HandleAutoCloseTags()
{
  ///int lexerID = SciCall_GetLexer();
  ///if (lexerID == SCLEX_HTML || lexerID == SCLEX_XML)
  DocPos const maxSearchBackward = 4096;
  {
    DocPos const iCurPos = SciCall_GetCurrentPos();
    DocPos const iHelper = iCurPos - maxSearchBackward;
    DocPos const iStartPos = max_p(0, iHelper);
    DocPos const iSize = iCurPos - iStartPos;

    if (iSize >= 3)
    {
      const char* pBegin = SciCall_GetRangePointer(iStartPos, iSize);

      if (pBegin[iSize - 2] != '/') 
      {
        const char* pCur = &pBegin[iSize - 2];
        while (pCur > pBegin && *pCur != '<' && *pCur != '>') { --pCur; }

        int  cchIns = 2;
        char replaceBuf[FNDRPL_BUFFER+2];
        StringCchCopyA(replaceBuf, FNDRPL_BUFFER, "</");
        if (*pCur == '<') {
          ++pCur;
          while ((StrChrA(":_-.", *pCur) || IsCharAlphaNumericA(*pCur)) && (cchIns < (FNDRPL_BUFFER-2))) {
            replaceBuf[cchIns++] = *pCur;
            ++pCur;
          }
        }
        replaceBuf[cchIns++] = '>';
        replaceBuf[cchIns] = '\0';

        // except tags w/o closing tags
        const char* const nonClosingTags[9] = { "</base>", "</bgsound>", "</br>", "</embed>", "</hr>", "</img>", "</input>", "</link>", "</meta>" };
        int const cntCount = COUNTOF(nonClosingTags);

        bool isNonClosingTag = false;
        for (int i = 0; ((i < cntCount) && !isNonClosingTag); ++i) {
          isNonClosingTag = (StringCchCompareXIA(replaceBuf, nonClosingTags[i]) == 0);
        }
        if ((cchIns > 3) && !isNonClosingTag)
        {
          EditReplaceSelection(replaceBuf,false);
          SciCall_SetSel(iCurPos,iCurPos);
        }
      }
    }
  }
}


//=============================================================================
//
//  _HandleTinyExpr() - called on '?' insert
//
static void  _HandleTinyExpr()
{
  DocPos const iCurPos = SciCall_GetCurrentPos();
  DocPos const iPosBefore = SciCall_PositionBefore(iCurPos);
  char const chBefore = SciCall_GetCharAt(iPosBefore - 1);
  if (chBefore == '=') // got "=?" evaluate expression trigger
  {
    DocPos lineLen = SciCall_LineLength(SciCall_LineFromPosition(iCurPos));
    char* lineBuf = (char*)AllocMem(lineLen + 1, HEAP_ZERO_MEMORY);
    if (lineBuf) {
      DocPos const iLnCaretPos = SciCall_GetCurLine((unsigned int)lineLen, lineBuf);
      lineBuf[(iLnCaretPos > 1) ? (iLnCaretPos - 2) : 0] = '\0'; // break before "=?"

      te_xint_t iExprErr  = 1;
      const char* pBegin = lineBuf;
      double dExprEval = 0.0;

      while (*pBegin && iExprErr) {
        dExprEval = te_interp(pBegin++, &iExprErr);
      }
      if (*pBegin && !iExprErr) {
        char chExpr[64] = { '\0' };
        StringCchPrintfA(chExpr, COUNTOF(chExpr), "%.6G", dExprEval);
        SciCall_SetSel(iPosBefore, iCurPos);
        SciCall_ReplaceSel(chExpr);
      }
      FreeMem(lineBuf);
    }
  }
}

#if 0
//=============================================================================
//
//   _IsIMEOpenInNoNativeMode()
//
static bool  _IsIMEOpenInNoNativeMode()
{
  bool result = false;
  HIMC const himc = ImmGetContext(Globals.hwndEdit);
  if (himc) {
    if (ImmGetOpenStatus(himc)) {
      DWORD dwConversion = IME_CMODE_ALPHANUMERIC, dwSentence = 0;
      if (ImmGetConversionStatus(himc, &dwConversion, &dwSentence)) {
        result = (dwConversion != IME_CMODE_ALPHANUMERIC);
      }
    }
    ImmReleaseContext(Globals.hwndEdit, himc);
  }
  return result;
}
#endif


//=============================================================================
//
//  MsgNotifyLean() - Handles WM_NOTIFY (only absolute neccessary events)
//
//  !!! Set correct SCI_SETMODEVENTMASK in _InitializeSciEditCtrl()
//
inline static LRESULT _MsgNotifyLean(const LPNMHDR pnmh, const SCNotification* const scn)
{
  static int _mod_insdel_token = -1;
  // --- check only mandatory events (must be fast !!!) ---
  if (pnmh->idFrom == IDC_EDIT) {
    if (pnmh->code == SCN_MODIFIED) {
      bool bModified = true;
      int const iModType = scn->modificationType;
      if (iModType & (SC_MOD_BEFOREINSERT | SC_MOD_BEFOREDELETE)) {
        if (!(iModType & (SC_PERFORMED_UNDO | SC_PERFORMED_REDO))) {
          if (!_InUndoRedoTransaction() && (_mod_insdel_token < 0) && 
             (!SciCall_IsSelectionEmpty() || Sci_IsMultiOrRectangleSelection())) {
            int const tok = _SaveUndoSelection();
            if (tok >= 0) {
              _mod_insdel_token = tok;
            }
          }
        }
        bModified = false; // not yet
      }
      else if (iModType & (SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT)) {
        if (!(iModType & (SC_PERFORMED_UNDO | SC_PERFORMED_REDO))) {
          if (!_InUndoRedoTransaction() && (_mod_insdel_token >= 0)) {
            _SaveRedoSelection(_mod_insdel_token);
            _mod_insdel_token = -1;
          }
        }
      }
      // check for ADDUNDOACTION step
      if (iModType & SC_MOD_CONTAINER)
      {
        if (iModType & SC_PERFORMED_UNDO) {
          RestoreAction(scn->token, UNDO);
        }
        else if (iModType & SC_PERFORMED_REDO) {
          RestoreAction(scn->token, REDO);
        }
      }
      if (bModified) {
        DWORD const timeout = Settings2.UndoTransactionTimeout;
        if (timeout != 0UL) {
          if (timeout > _MQ_IMMEDIATE) {
            _DelaySplitUndoTransaction(timeout, iModType);
          }
          else {
            _SplitUndoTransaction(iModType);
          }
        }
      }
    }
    else if (pnmh->code == SCN_SAVEPOINTREACHED) {
      SetSavePoint();
    }
    else if (pnmh->code == SCN_SAVEPOINTLEFT) {
      SetSaveNeeded();
    }
    else if (pnmh->code == SCN_MODIFYATTEMPTRO) {
      if (FocusedView.HideNonMatchedLines) { EditToggleView(Globals.hwndEdit); }
    }
  }
  return TRUE;
}


//=============================================================================
//
//  _MsgNotifyFromEdit() - Handles WM_NOTIFY (only absolute neccessary events)
//
//  !!! Set correct SCI_SETMODEVENTMASK in _InitializeSciEditCtrl()
//
static LRESULT _MsgNotifyFromEdit(HWND hwnd, const LPNMHDR pnmh, const SCNotification* const scn)
{
  static int  _s_indic_click_modifiers = SCMOD_NORM;
  static int _mod_insdel_token         = -1;

  switch (pnmh->code)
  {
    // unused:
    case SCN_HOTSPOTCLICK:
    case SCN_HOTSPOTDOUBLECLICK:
    case SCN_HOTSPOTRELEASECLICK:
      return FALSE;

    case SCN_AUTOCSELECTION:
    {
      switch (scn->listCompletionMethod) 
      {
        case SC_AC_TAB:
        case SC_AC_COMMAND:
        case SC_AC_DOUBLECLICK:
          // accepted
          break;

        case SC_AC_FILLUP:
          // see: SciCall_AutoCSetFillups() -> accepted
          break;

        case SC_AC_NEWLINE:
          if (!EditCheckNewLineInACFillUps()) {
            SciCall_AutoCCancel(); // rejected
            PostMessage(Globals.hwndEdit, SCI_NEWLINE, 0, 0);
          }
          break;

        default:
          SciCall_AutoCCancel(); // rejected
          break;
      }
    }
    break;

    case SCN_MODIFIED:
    {
      int const iModType = scn->modificationType;
      bool bModified = true;
      if (iModType & (SC_MOD_BEFOREINSERT | SC_MOD_BEFOREDELETE)) {
        if (!(iModType & (SC_PERFORMED_UNDO | SC_PERFORMED_REDO))) {
          if (!_InUndoRedoTransaction() && (_mod_insdel_token < 0) &&
             (!SciCall_IsSelectionEmpty() || Sci_IsMultiOrRectangleSelection())) {
            _mod_insdel_token = _SaveUndoSelection();
          }
        }
        bModified = false; // not yet
      }
      else if (iModType & (SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT)) {
        if (!(iModType & (SC_PERFORMED_UNDO | SC_PERFORMED_REDO))) {
          if (!_InUndoRedoTransaction() && (_mod_insdel_token >= 0)) {
            _SaveRedoSelection(_mod_insdel_token);
            _mod_insdel_token = -1;
          }
        }
      }
      if (iModType & SC_MOD_CONTAINER) {
        if (iModType & SC_PERFORMED_UNDO) {
          bModified = RestoreAction(scn->token, UNDO);
        }
        else if (iModType & SC_PERFORMED_REDO) {
          bModified = RestoreAction(scn->token, REDO);
        }
      }
      if (bModified) {
        if (IsMarkOccurrencesEnabled()) {
          MarkAllOccurrences(Settings2.UpdateDelayMarkAllOccurrences, true);
        }
        EditUpdateVisibleIndicators();
        if (scn->linesAdded != 0) {
          if (Settings.SplitUndoTypingSeqOnLnBreak && (scn->linesAdded == 1)) {
            _SplitUndoTransaction(iModType);
          }
          UpdateMarginWidth();
        }
        DWORD const timeout = Settings2.UndoTransactionTimeout;
        if (timeout != 0UL) {
          if (timeout > _MQ_IMMEDIATE) {
            _DelaySplitUndoTransaction(timeout, iModType);
          }
          else {
            _SplitUndoTransaction(iModType);
          }
        }
      }

      if (s_bInMultiEditMode && !(iModType & SC_MULTILINEUNDOREDO)) {
        if (!Sci_IsMultiSelection()) {
          SciCall_SetIndicatorCurrent(INDIC_NP3_MULTI_EDIT);
          SciCall_IndicatorClearRange(0, Sci_GetDocEndPosition());
          s_bInMultiEditMode = false;
        }
      }
    }
    break;


    case SCN_STYLENEEDED:  // this event needs SCI_SETLEXER(SCLEX_CONTAINER)
    {
      EditDoStyling(-1, scn->position);
    }
    break;


    case SCN_UPDATEUI:
    {
      int const iUpd = scn->updated;

      if (iUpd & (SC_UPDATE_SELECTION | SC_UPDATE_CONTENT))
      {
        // Brace Match
        if (Settings.MatchBraces) {
          EditMatchBrace(Globals.hwndEdit);
        }
        if (iUpd & SC_UPDATE_SELECTION)
        {
          // clear marks only, if selection changed
          if (IsMarkOccurrencesEnabled()) {
            if (!SciCall_IsSelectionEmpty() || Settings.MarkOccurrencesCurrentWord)
            {
              MarkAllOccurrences(Settings2.UpdateDelayMarkAllOccurrences, true);
            }
            else {
              if (Globals.iMarkOccurrencesCount > 0) {
                EditClearAllOccurrenceMarkers(Globals.hwndEdit);
              }
            }
          }
        }
        //~if (iUpd & SC_UPDATE_CONTENT) {
          //~ ignoring SC_UPDATE_CONTENT cause Style and Marker are out of scope here
          //~ using WM_COMMAND -> SCEN_CHANGE  instead!
          //~~~MarkAllOccurrences(Settings2.UpdateDelayMarkAllCoccurrences, false);
          //~~~EditUpdateVisibleIndicators(); // will lead to recursion
        //~}
        HandlePosChange();
        UpdateToolbar();
        UpdateMarginWidth();
        UpdateStatusbar(false);
      }
      else if (iUpd & SC_UPDATE_V_SCROLL)
      {
        if (IsMarkOccurrencesEnabled() && Settings.MarkOccurrencesMatchVisible) {
          MarkAllOccurrences(Settings2.UpdateDelayMarkAllOccurrences, false);
        }
        EditUpdateVisibleIndicators();
      }
    }
    break;


    case SCN_DWELLSTART:
    case SCN_DWELLEND:
    {
      HandleDWellStartEnd(scn->position, pnmh->code);
    }
    break;


    case SCN_DOUBLECLICK:
    {
      HandleHotSpotURLClicked(scn->position, SELECT_HYPERLINK); // COPY_HYPERLINK
    }
    break;


    case SCN_CALLTIPCLICK:
    {
      if (prevCursorPosition >= 0)
      {
        //~HandleHotSpotURLClicked(SciCall_CallTipPosStart(), OPEN_WITH_BROWSER);
        HandleHotSpotURLClicked(prevCursorPosition, OPEN_WITH_BROWSER);
      }
    }
    break;


    case SCN_INDICATORCLICK:
    {
      _s_indic_click_modifiers = scn->modifiers;
    }
    break;


    case SCN_INDICATORRELEASE:
    {
      if (SciCall_IndicatorValueAt(INDIC_NP3_HYPERLINK, scn->position) > 0)
      {
        if (_s_indic_click_modifiers & SCMOD_CTRL)
        {
          HandleHotSpotURLClicked(scn->position, OPEN_WITH_BROWSER);
        }
        else if (_s_indic_click_modifiers & SCMOD_ALT)
        {
          HandleHotSpotURLClicked(scn->position, OPEN_WITH_NOTEPAD3); // if applicable (file://)
        }
      }
      else if (SciCall_IndicatorValueAt(INDIC_NP3_COLOR_DEF, scn->position) > 0)
      {
        if (_s_indic_click_modifiers & SCMOD_ALT)
        {
          HandleColorDefClicked(Globals.hwndEdit, scn->position);
        }
      }
      _s_indic_click_modifiers = SCMOD_NORM;
    }
    break;


    case SCN_CHARADDED:
    {
      int const ich = scn->ch;

      if (Globals.CallTipType != CT_NONE) { CancelCallTip(); }

      if (Sci_IsMultiSelection())
      {
        SciCall_SetIndicatorCurrent(INDIC_NP3_MULTI_EDIT);
        DocPosU const selCount = SciCall_GetSelections();
        for (DocPosU s = 0; s < selCount; ++s)
        {
          DocPos const pos = SciCall_GetSelectionNStart(s);
          SciCall_IndicatorFillRange(SciCall_PositionBefore(pos), 1);
        }
        s_bInMultiEditMode = true;
      }

      switch (ich) {
        case '\r':
        case '\n':
          if (Settings.AutoIndent) { _HandleAutoIndent(ich); }
          break;
        case '>':
          if (Settings.AutoCloseTags) { _HandleAutoCloseTags(); }
          break;
        case '?':
          _HandleTinyExpr();
          break;
        default:
          break;
      }

      if ((Settings.AutoCompleteWords || Settings.AutoCLexerKeyWords))
      {
        if (!EditAutoCompleteWord(Globals.hwndEdit, false)) { return FALSE; }
      }
    }
    break;


    case SCN_AUTOCCHARDELETED:
      if ((Settings.AutoCompleteWords || Settings.AutoCLexerKeyWords))
      {
        if (!EditAutoCompleteWord(Globals.hwndEdit, false)) { return FALSE; }
      }
      break;


    case SCN_NEEDSHOWN:
    {
      DocLn const iFirstLine = SciCall_LineFromPosition(scn->position);
      DocLn const iLastLine = SciCall_LineFromPosition(scn->position + scn->length - 1);
      for (DocLn i = iFirstLine; i <= iLastLine; ++i) { SciCall_EnsureVisible(i); }
      EditUpdateVisibleIndicators();
    }
    break;


    case SCN_MARGINCLICK:
    switch (scn->margin) 
    {
      case MARGIN_SCI_FOLDING:
        EditFoldClick(SciCall_LineFromPosition(scn->position), scn->modifiers);
        break;
      case MARGIN_SCI_BOOKMRK:
        EditBookmarkToggle(Globals.hwndEdit, SciCall_LineFromPosition(scn->position), scn->modifiers);
        break;
      case MARGIN_SCI_LINENUM:
        //~SciCall_GotoLine(SciCall_LineFromPosition(scn->position));
        break;
      default:
        break;
    }
    break;


    case SCN_MARGINRIGHTCLICK:
      {
        POINT pt = {-1,-1};
        MsgContextMenu(hwnd, SCN_MARGINRIGHTCLICK, (WPARAM)scn, MAKELPARAM(pt.x,pt.y));
      }
      break;


    // ~~~ Not used in Windows ~~~
    // see: CMD_ALTUP / CMD_ALTDOWN
    //case SCN_KEY:
    //  // Also see the corresponding patch in scintilla\src\Editor.cxx
    //  FoldAltArrow(scn->ch, scn->modifiers);
    //  break;


    case SCN_SAVEPOINTREACHED:
      SetSavePoint();
      break;


    case SCN_SAVEPOINTLEFT:
      SetSaveNeeded();
      break;


    case SCN_ZOOM:
      UpdateToolbar();
      UpdateMarginWidth();
      break;


    case SCN_URIDROPPED:
    {
      // see WM_DROPFILES
      WCHAR szBuf[MAX_PATH + 40];
      if (MultiByteToWideCharEx(CP_UTF8, 0, scn->text, -1, szBuf, COUNTOF(szBuf)) > 0)
      {
        if (IsIconic(hwnd)) {
          ShowWindow(hwnd, SW_RESTORE);
        }
        //SetForegroundWindow(hwnd);
        if (PathIsDirectory(szBuf)) {
          WCHAR tchFile[MAX_PATH];
          if (OpenFileDlg(Globals.hwndMain, tchFile, COUNTOF(tchFile), szBuf)) {
            FileLoad(false, false, false, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, false, tchFile);
          }
        }
        else if (PathIsExistingFile(szBuf)) {
          FileLoad(false, false, false, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, false, szBuf);
        }
      }
    }
    break;

    default:
      return FALSE;
  }
  return TRUE;
}



//=============================================================================
//
//  MsgNotify() - Handles WM_NOTIFY
//
//  !!! Set correct SCI_SETMODEVENTMASK in _InitializeSciEditCtrl()
//
static bool s_mod_ctrl_pressed = false;
static bool s_tb_reset_already = false;

LRESULT MsgNotify(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  UNUSED(wParam);

  static bool _guard = false;
  if (_guard) { return TRUE; } else { _guard = true; } // avoid recursion

  #define GUARD_RETURN(res) { _guard = false; return(res); }

  LPNMHDR const pnmh = (LPNMHDR)lParam;
  const SCNotification* const scn = (SCNotification*)lParam;

  if (!CheckNotifyChangeEvent()) 
  {
    LRESULT const res = _MsgNotifyLean(pnmh, scn);
    GUARD_RETURN(res);
  }

  switch(pnmh->idFrom)
  {
    case IDC_EDIT:
      {
        LRESULT const res = _MsgNotifyFromEdit(hwnd, pnmh, scn);
        GUARD_RETURN(res);
      }
      
    // ------------------------------------------------------------------------

    case IDC_TOOLBAR:

      switch (pnmh->code)
      {
        case TBN_QUERYDELETE:
        case TBN_QUERYINSERT:
          // (!) must exist and return true 
          GUARD_RETURN(TRUE);

        case TBN_BEGINADJUST:
          s_tb_reset_already = false;
          GUARD_RETURN(FALSE);

        case TBN_GETBUTTONINFO:
        {
          if (((LPTBNOTIFY)lParam)->iItem < COUNTOF(s_tbbMainWnd))
          {
            WCHAR tch[SMALL_BUFFER] = { L'\0' };
            GetLngString(s_tbbMainWnd[((LPTBNOTIFY)lParam)->iItem].idCommand, tch, COUNTOF(tch));
            StringCchCopyN(((LPTBNOTIFY)lParam)->pszText, ((LPTBNOTIFY)lParam)->cchText, tch, ((LPTBNOTIFY)lParam)->cchText);
            CopyMemory(&((LPTBNOTIFY)lParam)->tbButton, &s_tbbMainWnd[((LPTBNOTIFY)lParam)->iItem], sizeof(TBBUTTON));
            GUARD_RETURN(TRUE);
          }
        }
        GUARD_RETURN(FALSE);

        case TBN_RESET:
        {
          int const count = (int)SendMessage(Globals.hwndToolbar, TB_BUTTONCOUNT, 0, 0);
          for (int i = 0; i < count; i++) {
            SendMessage(Globals.hwndToolbar, TB_DELETEBUTTON, 0, 0);
          }
          if (s_tb_reset_already) {
            if (Toolbar_SetButtons(Globals.hwndToolbar, IDT_FILE_NEW, Defaults.ToolbarButtons, s_tbbMainWnd, COUNTOF(s_tbbMainWnd)) == 0) {
              SendMessage(Globals.hwndToolbar, TB_ADDBUTTONS, COUNTOF(s_tbbMainWnd), (LPARAM)s_tbbMainWnd);
            }
          }
          else {
            if (Toolbar_SetButtons(Globals.hwndToolbar, IDT_FILE_NEW, Settings.ToolbarButtons, s_tbbMainWnd, COUNTOF(s_tbbMainWnd)) == 0) {
              SendMessage(Globals.hwndToolbar, TB_ADDBUTTONS, COUNTOF(s_tbbMainWnd), (LPARAM)s_tbbMainWnd);
            }
          }
          s_tb_reset_already = !s_tb_reset_already;
        }
        GUARD_RETURN(FALSE);

        case TBN_ENDADJUST:
          UpdateToolbar();
          GUARD_RETURN(TRUE);

        default:
          GUARD_RETURN(FALSE);
      }
      break;

    // ------------------------------------------------------------------------

    case IDC_STATUSBAR:

      switch(pnmh->code)
      {
        case NM_CLICK: // single click
          {
            LPNMMOUSE const pnmm = (LPNMMOUSE)lParam;

            switch (g_vSBSOrder[pnmm->dwItemSpec])
            {
              case STATUS_EOLMODE:
                {
                  if (Globals.bDocHasInconsistentEOLs)
                  {
                    int const eol_mode = SciCall_GetEOLMode();
                    
                    int const  eol_cmd  = (eol_mode == SC_EOL_CRLF) ? IDM_LINEENDINGS_CRLF : 
                                           ((eol_mode == SC_EOL_CR) ? IDM_LINEENDINGS_CR : IDM_LINEENDINGS_LF);

                    UINT const msgid    = (eol_mode == SC_EOL_CRLF) ? IDS_MUI_EOLMODENAME_CRLF : 
                                           ((eol_mode == SC_EOL_CR) ? IDS_MUI_EOLMODENAME_CR : IDS_MUI_EOLMODENAME_LF); 
                    
                    WCHAR wch[64] = {L'\0'};
                    GetLngString(msgid, wch, COUNTOF(wch));
                    INT_PTR const answer = InfoBoxLng(MB_YESNO | MB_ICONWARNING, NULL, IDS_MUI_WARN_NORMALIZE_EOLS, wch);

                    if ((IDOK == answer) || (IDYES == answer))
                    {
                      PostWMCommand(hwnd, eol_cmd);
                    }
                  }
                }
                GUARD_RETURN(!0);

              default:
                break;
            }
          }
          break;

        case NM_DBLCLK: // double click
          {
            LPNMMOUSE const pnmm = (LPNMMOUSE)lParam;

            switch (g_vSBSOrder[pnmm->dwItemSpec])
            {
              case STATUS_DOCLINE:
              case STATUS_DOCCOLUMN:
                PostWMCommand(hwnd, IDM_EDIT_GOTOLINE);
                GUARD_RETURN(!0);

              case STATUS_CODEPAGE:
                PostWMCommand(hwnd, IDM_ENCODING_SELECT);
                GUARD_RETURN(!0);

              case STATUS_EOLMODE:
                {
                  int const eol_mode = (SciCall_GetEOLMode() + 1) % 3;
                  int const eol_cmd  = (eol_mode == SC_EOL_CRLF) ? IDM_LINEENDINGS_CRLF : 
                                        ((eol_mode == SC_EOL_CR) ? IDM_LINEENDINGS_CR : IDM_LINEENDINGS_LF); 
                  PostWMCommand(hwnd, eol_cmd);
                }
                GUARD_RETURN(!0);

              case STATUS_OVRMODE:
                PostWMCommand(hwnd, CMD_VK_INSERT);
                GUARD_RETURN(!0);

              case STATUS_2ND_DEF:
                PostWMCommand(hwnd, IDM_VIEW_USE2NDDEFAULT);
                GUARD_RETURN(!0);

              case STATUS_LEXER:
                PostWMCommand(hwnd, IDM_VIEW_SCHEME);
                GUARD_RETURN(!0);

              case STATUS_TINYEXPR:
                {
                  char chBuf[80];
                  if (s_iExprError == 0) {
                    StringCchPrintfA(chBuf, COUNTOF(chBuf), "%.6G", s_dExpression);
                    SciCall_CopyText((DocPos)StringCchLenA(chBuf,80), chBuf);
                  }
                  else if (s_iExprError > 0) {
#ifdef _WIN64
                    StringCchPrintfA(chBuf, COUNTOF(chBuf), "^[%ll]", s_iExprError);
#else
                    StringCchPrintfA(chBuf, COUNTOF(chBuf), "^[%i]", s_iExprError);
#endif
                    SciCall_CopyText((DocPos)StringCchLenA(chBuf,80), chBuf);
                  }
                  else {
                    SciCall_CopyText(0, "");
                  }
                }
                break;

              default:
                break;
            }
          }
          break;

        default:
          break;
      }
      break;

    // ------------------------------------------------------------------------

    default:
      switch(pnmh->code)
      {
        // ToolTip
        case TTN_NEEDTEXT:
          if (!(((LPTOOLTIPTEXT)lParam)->uFlags & TTF_IDISHWND))
          {
            WCHAR tch[SMALL_BUFFER] = { L'\0' };
            GetLngString((UINT)pnmh->idFrom,tch,COUNTOF(tch));
            WCHAR* pttText = ((LPTOOLTIPTEXT)lParam)->szText;
            size_t const ttLen = COUNTOF(((LPTOOLTIPTEXT)lParam)->szText);
            StringCchCopyN(pttText, ttLen, tch,COUNTOF(tch));
          }
          break;
          
        default:
          break;
      }
      break;

  }
  GUARD_RETURN(0);
}


//=============================================================================
//
//  ParseCommandLine()
//
//
void ParseCommandLine()
{
  LPWSTR lpCmdLine = GetCommandLine();
  if (StrIsEmpty(lpCmdLine)) { return; }

  // Good old console can also send args separated by Tabs
  StrTab2Space(lpCmdLine);

  DocPos const len = (DocPos)(StringCchLenW(lpCmdLine,0) + 2UL);
  LPWSTR lp1 = AllocMem(sizeof(WCHAR)*len,HEAP_ZERO_MEMORY);
  LPWSTR lp2 = AllocMem(sizeof(WCHAR)*len,HEAP_ZERO_MEMORY);
  LPWSTR lp3 = AllocMem(sizeof(WCHAR)*len,HEAP_ZERO_MEMORY);

  if (lp1 && lp2 && lp3) 
  {
    bool bIsNotepadReplacement = false;
    
    // Start with 2nd argument
    ExtractFirstArgument(lpCmdLine, lp1, lp3, (int)len);

    bool bContinue = true;
    bool bIsFileArg = false;
    s_flagSetEncoding = CPI_NONE;

    while (bContinue && ExtractFirstArgument(lp3, lp1, lp2, (int)len)) {
      // options
      if (lp1[1] == L'\0') {
        if (!bIsFileArg && (lp1[0] == L'+')) {
          Globals.CmdLnFlag_MultiFileArg = 2;
          bIsFileArg = true;
        }
        else if (!bIsFileArg && (lp1[0] == L'-')) {
          Globals.CmdLnFlag_MultiFileArg = 1;
          bIsFileArg = true;
        }
      }
      else if (!bIsFileArg && ((*lp1 == L'/') || (*lp1 == L'-'))) 
      {
        // LTrim
        StrLTrimI(lp1, L"-/");

        // Encoding
        cpi_enc_t const encoding = Encoding_MatchW(lp1);

        if (StringCchCompareXI(lp1, L"ANSI") == 0 || StringCchCompareXI(lp1, L"A") == 0 || StringCchCompareXI(lp1, L"MBCS") == 0) {
          s_flagSetEncoding = CPI_ANSI_DEFAULT;
        }
        else if (StringCchCompareXI(lp1, L"UNICODE") == 0 || StringCchCompareXI(lp1, L"W") == 0) {
          s_flagSetEncoding = CPI_UNICODEBOM;
        }
        else if (StringCchCompareXI(lp1, L"UNICODEBE") == 0 || StringCchCompareXI(lp1, L"UNICODE-BE") == 0) {
          s_flagSetEncoding = CPI_UNICODEBEBOM;
        }
        else if (StringCchCompareXI(lp1, L"UTF8") == 0 || StringCchCompareXI(lp1, L"UTF-8") == 0) {
          s_flagSetEncoding = CPI_UTF8;
        }
        else if (StringCchCompareXI(lp1, L"UTF8SIG") == 0 || StringCchCompareXI(lp1, L"UTF-8SIG") == 0 ||
          StringCchCompareXI(lp1, L"UTF8SIGNATURE") == 0 || StringCchCompareXI(lp1, L"UTF-8SIGNATURE") == 0 ||
            StringCchCompareXI(lp1, L"UTF8-SIGNATURE") == 0 || StringCchCompareXI(lp1, L"UTF-8-SIGNATURE") == 0) {
          s_flagSetEncoding = CPI_UTF8SIGN;
        }
        // maybe parsed encoding
        else if (Encoding_IsValid(encoding)) {
          s_flagSetEncoding = encoding;
        }
        // EOL Mode
        else if (StringCchCompareXI(lp1, L"CRLF") == 0 || StringCchCompareXI(lp1, L"CR+LF") == 0) {
          s_flagSetEOLMode = IDM_LINEENDINGS_CRLF - IDM_LINEENDINGS_CRLF + 1;
        }
        else if (StringCchCompareXI(lp1, L"CR") == 0) {
          s_flagSetEOLMode = IDM_LINEENDINGS_CR - IDM_LINEENDINGS_CRLF + 1;
        }
        else if (StringCchCompareXI(lp1, L"LF") == 0) {
          s_flagSetEOLMode = IDM_LINEENDINGS_LF - IDM_LINEENDINGS_CRLF + 1;
        }
        // Shell integration
        else if (StrCmpNI(lp1, L"appid=", CSTRLEN(L"appid=")) == 0) {
          StringCchCopyN(Settings2.AppUserModelID, COUNTOF(Settings2.AppUserModelID),
                         lp1 + CSTRLEN(L"appid="), len - CSTRLEN(L"appid="));
          StrTrim(Settings2.AppUserModelID, L" ");
          if (StrIsEmpty(Settings2.AppUserModelID)) {
            StringCchCopy(Settings2.AppUserModelID, COUNTOF(Settings2.AppUserModelID), _W("Rizonesoft." SAPPNAME));
          }
        }
        else if (StrCmpNI(lp1, L"sysmru=", CSTRLEN(L"sysmru=")) == 0) {
          WCHAR wch[16];
          StringCchCopyN(wch, COUNTOF(wch), lp1 + CSTRLEN(L"sysmru="), COUNTOF(wch));
          StrTrim(wch, L" ");
          if (*wch == L'1')
            Globals.CmdLnFlag_ShellUseSystemMRU = 2;
          else
            Globals.CmdLnFlag_ShellUseSystemMRU = 1;
        }
        // Relaunch elevated
        else if (StrCmpNI(lp1, RELAUNCH_ELEVATED_BUF_ARG, CSTRLEN(RELAUNCH_ELEVATED_BUF_ARG)) == 0) {
          StringCchCopyN(s_wchTmpFilePath, COUNTOF(s_wchTmpFilePath),
                         lp1 + CSTRLEN(RELAUNCH_ELEVATED_BUF_ARG), len - CSTRLEN(RELAUNCH_ELEVATED_BUF_ARG));
          TrimSpcW(s_wchTmpFilePath);
          NormalizePathEx(s_wchTmpFilePath, COUNTOF(s_wchTmpFilePath), true, Flags.bSearchPathIfRelative);
          s_IsThisAnElevatedRelaunch = true;
        }

        else {

          switch (*CharUpper(lp1)) {

            case L'N':
              Globals.CmdLnFlag_ReuseWindow = 1;
              if (*CharUpper(lp1 + 1) == L'S')
                Globals.CmdLnFlag_SingleFileInstance = 2;
              else
                Globals.CmdLnFlag_SingleFileInstance = 1;
              break;

            case L'R':
              if (*CharUpper(lp1 + 1) == L'P') {
                Flags.bPreserveFileModTime = true;
              }
              else {
                Globals.CmdLnFlag_ReuseWindow = 2;
                if (*CharUpper(lp1 + 1) == L'S')
                  Globals.CmdLnFlag_SingleFileInstance = 2;
                else
                  Globals.CmdLnFlag_SingleFileInstance = 1;
              }
              break;

            case L'F':
              if (*(lp1 + 1) == L'0' || *CharUpper(lp1 + 1) == L'O')
                StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), L"*?");
              else if (ExtractFirstArgument(lp2, lp1, lp2, (int)len)) {
                StringCchCopyN(Globals.IniFile, COUNTOF(Globals.IniFile), lp1, len);
                TrimSpcW(Globals.IniFile);
                NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile), true, false);
              }
              break;

            case L'I':
              s_flagStartAsTrayIcon = true;
              break;

            case L'O':
              if (*(lp1 + 1) == L'0' || *(lp1 + 1) == L'-' || *CharUpper(lp1 + 1) == L'O')
                s_flagAlwaysOnTop = 1;
              else
                s_flagAlwaysOnTop = 2;
              break;

            case L'P':
            {
              WCHAR* lp = lp1;
              if (StrCmpNI(lp1, L"POS:", CSTRLEN(L"POS:")) == 0)
                lp += CSTRLEN(L"POS:") - 1;
              else if (StrCmpNI(lp1, L"POS", CSTRLEN(L"POS")) == 0)
                lp += CSTRLEN(L"POS") - 1;
              else if (*(lp1 + 1) == L':')
                lp += 1;
              else if (bIsNotepadReplacement) {
                if (*(lp1 + 1) == L'T')
                  ExtractFirstArgument(lp2, lp1, lp2, (int)len);
                break;
              }
              if (*(lp + 1) == L'0' || *CharUpper(lp + 1) == L'O') {
                Globals.CmdLnFlag_PosParam = true;
                Globals.CmdLnFlag_WindowPos = 1;
              }
              else if (*CharUpper(lp + 1) == L'D' || *CharUpper(lp + 1) == L'S') {
                Globals.CmdLnFlag_PosParam = true;
                Globals.CmdLnFlag_WindowPos = (StrChrI((lp + 1), L'L')) ? 3 : 2;
              }
              else if (StrChrI(L"FLTRBM", *(lp + 1))) {
                WCHAR* p = (lp + 1);
                Globals.CmdLnFlag_PosParam = true;
                Globals.CmdLnFlag_WindowPos = 0;
                while (*p) {
                  switch (*CharUpper(p)) {
                    case L'F':
                      Globals.CmdLnFlag_WindowPos &= ~(4 | 8 | 16 | 32);
                      Globals.CmdLnFlag_WindowPos |= 64;
                      break;
                    case L'L':
                      Globals.CmdLnFlag_WindowPos &= ~(8 | 64);
                      Globals.CmdLnFlag_WindowPos |= 4;
                      break;
                    case  L'R':
                      Globals.CmdLnFlag_WindowPos &= ~(4 | 64);
                      Globals.CmdLnFlag_WindowPos |= 8;
                      break;
                    case L'T':
                      Globals.CmdLnFlag_WindowPos &= ~(32 | 64);
                      Globals.CmdLnFlag_WindowPos |= 16;
                      break;
                    case L'B':
                      Globals.CmdLnFlag_WindowPos &= ~(16 | 64);
                      Globals.CmdLnFlag_WindowPos |= 32;
                      break;
                    case L'M':
                      if (Globals.CmdLnFlag_WindowPos == 0)
                        Globals.CmdLnFlag_WindowPos |= 64;
                      Globals.CmdLnFlag_WindowPos |= 128;
                      break;
                  }
                  p = CharNext(p);
                }
              }
              else if (ExtractFirstArgument(lp2, lp1, lp2, (int)len)) {
                WININFO wi = INIT_WININFO;
                int bMaximize = 0;
                int itok = swscanf_s(lp1, L"%i,%i,%i,%i,%i", &wi.x, &wi.y, &wi.cx, &wi.cy, &bMaximize);
                if (itok == 4 || itok == 5) { // scan successful
                  Globals.CmdLnFlag_PosParam = true;
                  Globals.CmdLnFlag_WindowPos = 0;
                  if (bMaximize) wi.max = true;
                  if (itok == 4) wi.max = false;
                  g_IniWinInfo = wi; // set window placement
                }
              }
            }
            break;

            case L'T':
              if (ExtractFirstArgument(lp2, lp1, lp2, (int)len)) {
                StringCchCopyN(s_wchTitleExcerpt, COUNTOF(s_wchTitleExcerpt), lp1, len);
                s_flagKeepTitleExcerpt = true;
              }
              break;

            case L'C':
              s_flagNewFromClipboard = true;
              break;

            case L'B':
              s_flagPasteBoard = true;
              break;

            case L'E':
              if (ExtractFirstArgument(lp2, lp1, lp2, (int)len)) {
                s_flagSetEncoding = Encoding_MatchW(lp1);
              }
              break;

            case L'G':
              if (ExtractFirstArgument(lp2, lp1, lp2, (int)len)) {
                int itok =
                  swscanf_s(lp1, L"%i,%i", &s_iInitialLine, &s_iInitialColumn);
                if (itok == 1 || itok == 2) { // scan successful
                  s_flagJumpTo = true;
                }
              }
              break;

            case L'M':
              {
                bool bFindUp = false;
                bool bMatchCase = false;
                bool bRegex = false;
                bool bDotMatchAll = false;
                bool bTransBS = false;

                if (StrChr(lp1, L'-')) {
                  bFindUp = true;
                }
                if (StrChr(lp1, L'C')) {
                  bMatchCase = true;
                }
                if (StrChr(lp1, L'R')) {
                  bRegex = true;
                  bTransBS = true;
                }
                if (StrChr(lp1, L'A')) {
                  bDotMatchAll = true;
                }
                if (StrChr(lp1, L'B')) {
                  bTransBS = true;
                }
                if (ExtractFirstArgument(lp2, lp1, lp2, (int)len)) 
                {
                  SetFindPattern(lp1);
                  
                  g_flagMatchText = 1;
                  if (bFindUp) {
                    g_flagMatchText |= 2;
                  }
                  if (bRegex) {
                    g_flagMatchText |= 4;
                  }
                  if (bMatchCase) {
                    g_flagMatchText |= 8;
                  }
                  if (bDotMatchAll) {
                    g_flagMatchText |= 16;
                  }
                  if (bTransBS) {
                    g_flagMatchText |= 32;
                  }
                }
              }
              break;

            case L'L':
              if (*(lp1 + 1) == L'0' || *(lp1 + 1) == L'-' || *CharUpper(lp1 + 1) == L'O')
                s_flagChangeNotify = FWM_MSGBOX;
              else
                s_flagChangeNotify = FWM_AUTORELOAD;
              break;

            case L'Q':
              if (*CharUpper(lp1 + 1) == L'S') {
                s_flagSaveOnRelaunch = true;
              }
              else {
                s_flagQuietCreate = true;
              }
              break;

            case L'S':
              if (ExtractFirstArgument(lp2, lp1, lp2, (int)len)) {
                if (s_lpSchemeArg) { LocalFree(s_lpSchemeArg); }  // StrDup()
                s_lpSchemeArg = StrDup(lp1);
                s_flagLexerSpecified = true;
              }
              break;

            case L'D':
              if (s_lpSchemeArg) {
                LocalFree(s_lpSchemeArg);  // StrDup()
                s_lpSchemeArg = NULL;
              }
              s_iInitialLexer = 0;
              s_flagLexerSpecified = true;
              break;

            case L'H':
              if (s_lpSchemeArg) {
                LocalFree(s_lpSchemeArg);  // StrDup()
                s_lpSchemeArg = NULL;
              }
              s_iInitialLexer = 35;
              s_flagLexerSpecified = true;
              break;

            case L'X':
              if (s_lpSchemeArg) {
                LocalFree(s_lpSchemeArg);  // StrDup()
                s_lpSchemeArg = NULL;
              }
              s_iInitialLexer = 36;
              s_flagLexerSpecified = true;
              break;

            case L'U':
              if (*CharUpper(lp1 + 1) == L'C') {
                s_flagAppIsClosing = true;
              }
              else {
                Flags.bDoRelaunchElevated = true;
              }
              break;

            case L'Y':
              Flags.bSearchPathIfRelative = true;
              break;

            case L'Z':
              ExtractFirstArgument(lp2, lp1, lp2, (int)len);
              Globals.CmdLnFlag_MultiFileArg = 1;
              bIsNotepadReplacement = true;
              break;

            case L'?':
              s_flagDisplayHelp = true;
              break;

            case L'V':
              Globals.CmdLnFlag_PrintFileAndLeave = 1;
              if (*CharUpper(lp1 + 1) == L'D') {
                Globals.CmdLnFlag_PrintFileAndLeave = 2;  // open printer dialog
              }
              break;

            default:
              break;

          }
        }
      }
      // pathname
      else {
        LPWSTR lpFileBuf = AllocMem(sizeof(WCHAR)*len, HEAP_ZERO_MEMORY);
        if (lpFileBuf) {
          size_t const fileArgLen = StringCchLenW(lp3, len);
          s_cchiFileList = (int)(StringCchLenW(lpCmdLine, len - 2) - fileArgLen);

          if (s_lpOrigFileArg) {
            FreeMem(s_lpOrigFileArg);
            //s_lpOrigFileArg = NULL;
          }
          s_lpOrigFileArg = AllocMem(sizeof(WCHAR)*(fileArgLen + 1), HEAP_ZERO_MEMORY); // changed for ActivatePrevInst() needs
          StringCchCopy(s_lpOrigFileArg, fileArgLen + 1, lp3);

          StringCchCopy(s_lpFileArg, COUNTOF(s_lpFileArg), lp3);
          PathFixBackslashes(s_lpFileArg);
          if (!PathIsRelative(s_lpFileArg) && !PathIsUNC(s_lpFileArg) &&
              PathGetDriveNumber(s_lpFileArg) == -1 /*&& PathGetDriveNumber(Globals.WorkingDirectory) != -1*/) 
          {
            WCHAR wchPath[MAX_PATH] = { L'\0' };
            StringCchCopy(wchPath, COUNTOF(wchPath), Globals.WorkingDirectory);
            PathStripToRoot(wchPath);
            PathCchAppend(wchPath, COUNTOF(wchPath), s_lpFileArg);
            StringCchCopy(s_lpFileArg, COUNTOF(s_lpFileArg), wchPath);
          }
          StrTrim(s_lpFileArg, L" \"");

          while ((s_cFileList < FILE_LIST_SIZE) && ExtractFirstArgument(lp3, lpFileBuf, lp3, (int)len)) {
            PathQuoteSpaces(lpFileBuf);
            s_lpFileList[s_cFileList++] = StrDup(lpFileBuf); // LocalAlloc()
          }
          bContinue = false;
          FreeMem(lpFileBuf);
        }
      }

      // Continue with next argument
      if (bContinue) {
        StringCchCopy(lp3, len, lp2);
      }
    }

    FreeMem(lp1);
    FreeMem(lp2);
    FreeMem(lp3);
  }
}


//=============================================================================
//
//  _DelayUpdateStatusbar()
//  
//
static void  _DelayUpdateStatusbar(int delay, bool bForceRedraw)
{
  static CmdMessageQueue_t mqc = MQ_WM_CMD_INIT(IDT_TIMER_UPDATE_STATUSBAR, 0);
  mqc.hwnd = Globals.hwndMain;
  if (bForceRedraw) {
    mqc.lparam = (LPARAM)bForceRedraw;
  }
  _MQ_AppendCmd(&mqc, (UINT)(delay <= 0 ? 0 : _MQ_ms(delay)));
}


//=============================================================================
//
//  _DelayUpdateToolbar()
//  
//
static void  _DelayUpdateToolbar(int delay)
{
  static CmdMessageQueue_t mqc = MQ_WM_CMD_INIT(IDT_TIMER_UPDATE_TOOLBAR, 0);
  mqc.hwnd = Globals.hwndMain;
  //mqc.lparam = (LPARAM)2nd_param;
  _MQ_AppendCmd(&mqc, (UINT)(delay <= 0 ? 0 : _MQ_ms(delay)));
}


//=============================================================================
//
//  _DelayClearZoomCallTip()
//  
//
static void  _DelayClearZoomCallTip(int delay)
{
  static CmdMessageQueue_t mqc = MQ_WM_CMD_INIT(IDT_TIMER_CLEAR_CALLTIP, 0);
  mqc.hwnd = Globals.hwndMain;
  //mqc.lparam = (LPARAM)2nd_param;
  _MQ_AppendCmd(&mqc, (UINT)(delay <= 0 ? 0 : _MQ_ms(delay)));
}


//=============================================================================
//
//  _DelaySplitUndoTransaction()
//  
//
static void  _DelaySplitUndoTransaction(int delay, int iModType)
{
  static CmdMessageQueue_t mqc = MQ_WM_CMD_INIT(IDT_TIMER_UNDO_TRANSACTION, 0);
  if (!((iModType & SC_PERFORMED_UNDO) || (iModType & SC_PERFORMED_REDO))) {
    mqc.hwnd = Globals.hwndMain;
    mqc.lparam = (LPARAM)iModType;
    _MQ_AppendCmd(&mqc, (UINT)(delay <= 0 ? 0 : _MQ_ms(delay)));
  }
}


//=============================================================================
//
//  MarkAllOccurrences()
// 
void MarkAllOccurrences(int delay, bool bForceClear)
{
  static CmdMessageQueue_t mqc = MQ_WM_CMD_INIT(IDT_TIMER_MAIN_MRKALL, 0);
  mqc.hwnd = Globals.hwndMain;
  if (bForceClear) {
    mqc.lparam = (LPARAM)bForceClear;
  }
  _MQ_AppendCmd(&mqc, (UINT)(delay <= 0 ? 0 : _MQ_ms(delay)));
}


//=============================================================================
//
//  UpdateToolbar()
//
void UpdateToolbar()
{
  _DelayUpdateToolbar(_MQ_FAST);
}


//=============================================================================

static void  _UpdateToolbarDelayed()
{
  bool const bDocModified = GetDocModified();

  if (!Settings.ShowToolbar) { return; }

  EnableTool(Globals.hwndToolbar, IDT_FILE_ADDTOFAV, StrIsNotEmpty(Globals.CurrentFile));
  EnableTool(Globals.hwndToolbar, IDT_FILE_SAVE, bDocModified /*&& !bReadOnly*/);
  EnableTool(Globals.hwndToolbar, IDT_FILE_RECENT, (MRU_Count(Globals.pFileMRU) > 0));

  CheckTool(Globals.hwndToolbar, IDT_VIEW_WORDWRAP, Globals.fvCurFile.bWordWrap);
  CheckTool(Globals.hwndToolbar, IDT_VIEW_CHASING_DOCTAIL, FileWatching.MonitoringLog);
  CheckTool(Globals.hwndToolbar, IDT_VIEW_PIN_ON_TOP, Settings.AlwaysOnTop);

  bool const b1 = SciCall_IsSelectionEmpty();
  bool const b2 = !Sci_IsDocEmpty();
  bool const ro = SciCall_GetReadOnly();
  bool const tv = FocusedView.HideNonMatchedLines;
  bool const zi = (SciCall_GetZoom() > 100);
  bool const zo = (SciCall_GetZoom() < 100);

  EnableTool(Globals.hwndToolbar, IDT_EDIT_UNDO, SciCall_CanUndo() && !ro);
  EnableTool(Globals.hwndToolbar, IDT_EDIT_REDO, SciCall_CanRedo() && !ro);
  EnableTool(Globals.hwndToolbar, IDT_EDIT_PASTE, SciCall_CanPaste() && !ro);

  EnableTool(Globals.hwndToolbar, IDT_FILE_LAUNCH, b2);

  EnableTool(Globals.hwndToolbar, IDT_EDIT_FIND, b2);
  //EnableTool(Globals.hwndToolbar, ,b2);
  //EnableTool(Globals.hwndToolbar, IDT_EDIT_FINDPREV,b2 && !StrIsEmptyA(s_FindReplaceData.szFind));
  EnableTool(Globals.hwndToolbar, IDT_EDIT_REPLACE, b2 && !ro);

  EnableTool(Globals.hwndToolbar, IDT_EDIT_CUT, !b1 && !ro);
  EnableTool(Globals.hwndToolbar, IDT_EDIT_COPY, !b1 && !ro);
  EnableTool(Globals.hwndToolbar, IDT_EDIT_CLEAR, !b1 && !ro);

  EnableTool(Globals.hwndToolbar, IDT_VIEW_TOGGLEFOLDS, b2 && (FocusedView.CodeFoldingAvailable && FocusedView.ShowCodeFolding));

  EnableTool(Globals.hwndToolbar, IDT_VIEW_TOGGLE_VIEW, b2 && IsFocusedViewAllowed());
  CheckTool(Globals.hwndToolbar, IDT_VIEW_TOGGLE_VIEW, tv);

  CheckTool(Globals.hwndToolbar, IDT_VIEW_ZOOMIN, zi);
  CheckTool(Globals.hwndToolbar, IDT_VIEW_ZOOMOUT, zo);
}



//=============================================================================
//
//  _StatusCalcPaneWidth()
//
static LONG  _StatusCalcPaneWidth(HWND hwnd, LPCWSTR lpsz)
{
  HDC const hdc = GetDC(hwnd);
  HGDIOBJ const hfont = (HGDIOBJ)SendMessage(hwnd, WM_GETFONT, 0, 0);
  HGDIOBJ const hfold = SelectObject(hdc, hfont);
  int const mmode = SetMapMode(hdc, MM_TEXT);

  SIZE size = { 0L, 0L };
  GetTextExtentPoint32(hdc, lpsz, (int)StringCchLenW(lpsz,0), &size);

  SetMapMode(hdc, mmode);
  SelectObject(hdc, hfold);
  ReleaseDC(hwnd, hdc);

  return (size.cx + 8L);
}



//=============================================================================
//
//  _CalculateStatusbarSections
//  vSectionWidth[] must be pre-filled with -1
//
#define txtWidth 80
typedef WCHAR sectionTxt_t[txtWidth];

static void  _CalculateStatusbarSections(int vSectionWidth[], sectionTxt_t tchStatusBar[], bool* bIsUpdNeeded)
{
  static int s_iWinFormerWidth = -1;
  if (s_iWinFormerWidth != s_WinCurrentWidth) {
    *bIsUpdNeeded = true;
    s_iWinFormerWidth = s_WinCurrentWidth;
  }
  if (!(*bIsUpdNeeded)) { return; }

  // count fixed and dynamic optimized pixels
  int pxCount = 0;
  for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
    if (g_iStatusbarVisible[i]) {
      if (g_iStatusbarWidthSpec[i] == 0) { // dynamic optimized
        vSectionWidth[i] = _StatusCalcPaneWidth(Globals.hwndStatus, tchStatusBar[i]);
      }
      else if (g_iStatusbarWidthSpec[i] < -1) { // fixed pixel count
        vSectionWidth[i] = -(g_iStatusbarWidthSpec[i]);
      }
      //else { /* 0,-1 : relative counts */ }
      // accumulate
      if (vSectionWidth[i] > 0) { pxCount += vSectionWidth[i]; }
    }
  }

  int const iPropSectTotalWidth = s_WinCurrentWidth - pxCount - STAUSBAR_RIGHT_MARGIN;

  // init proportional section checker
  bool bIsPropSection[STATUS_SECTOR_COUNT] = SBS_INIT_ZERO;
  for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
    if ((g_iStatusbarVisible[i]) && (vSectionWidth[i] < 0)) {
      assert(g_iStatusbarWidthSpec[i] > 0);
      bIsPropSection[i] = true;
    }
  }

  // get min. required widths
  int vMinWidth[STATUS_SECTOR_COUNT] = SBS_INIT_ZERO;
  int iTotalMinWidth = 0;
  for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
    if (bIsPropSection[i]) {
      int const iMinWidth = _StatusCalcPaneWidth(Globals.hwndStatus, tchStatusBar[i]);
      vMinWidth[i] = iMinWidth;
      iTotalMinWidth += iMinWidth;
    }
  }

  if (iTotalMinWidth >= iPropSectTotalWidth) 
  {
    for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
      if (bIsPropSection[i]) {
        vSectionWidth[i] = vMinWidth[i];
      }
    }
  }
  else // space left for proportional elements
  {
    int vPropWidth[STATUS_SECTOR_COUNT] = SBS_INIT_ZERO;
    int totalCnt = 0;
    for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
      if (bIsPropSection[i]) {
        vPropWidth[i] = iPropSectTotalWidth * g_iStatusbarWidthSpec[i];
        totalCnt += g_iStatusbarWidthSpec[i];
      }
    }
    // normalize
    int const iCeilFloor = (totalCnt + 1) / 2;
    //int iTotalPropWidth = 0;
    for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
      if (bIsPropSection[i]) {
        int const width = (totalCnt > 1) ? ((vPropWidth[i] + iCeilFloor) / totalCnt) : 0;
        vPropWidth[i] = width;
        //iTotalPropWidth += width;
      }
    }
    // check for fitting
    int iOverlappingText = 0;
    int iOvlTxtCount = 0;
    for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
      if (bIsPropSection[i]) {
        if (vMinWidth[i] > vPropWidth[i]) {
          iOverlappingText += (vMinWidth[i] - vPropWidth[i]);
          ++iOvlTxtCount;
        }
      }
    }
    if (iOvlTxtCount == 0) {
      // we are fine
      for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
        if (bIsPropSection[i]) {
          vSectionWidth[i] = vPropWidth[i];
        }
      }
    }
    else // handling overlaps
    {
      while (iOverlappingText > 0) {
        const int iNoProgress = iOverlappingText;
        for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
          if (bIsPropSection[i]) {
            if (vMinWidth[i] < vPropWidth[i]) {
              vPropWidth[i] -= 1;
              --iOverlappingText;
            }
            else if (vMinWidth[i] > vPropWidth[i]) {
              vPropWidth[i] = vMinWidth[i];
            }
          }
          if (iOverlappingText == 0) { break; /* for */}
        }
        if ((iOverlappingText == 0) || (iNoProgress == iOverlappingText)) { break; }
      }
      // fill missing widths
      for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
        if (bIsPropSection[i]) {
          vSectionWidth[i] = vPropWidth[i];
        }
      }
    }
  }
}



//=============================================================================
//
//  _InterpMultiSelectionTinyExpr()
//
//
static double _InterpMultiSelectionTinyExpr(te_xint_t* piExprError)
{
  #define _tmpBufCnt 128
  char tmpRectSelN[_tmpBufCnt] = { '\0' };
  
  DocPosU const selCount = SciCall_GetSelections();
  DocPosU const calcBufSize = _tmpBufCnt * selCount;
  char* calcBuffer = (char*)AllocMem(calcBufSize + 1, HEAP_ZERO_MEMORY);

  bool bLastCharWasDigit = false;
  for (DocPosU i = 0; i < selCount; ++i)
  {
    DocPos const posSelStart = SciCall_GetSelectionNStart(i);
    DocPos const posSelEnd = SciCall_GetSelectionNEnd(i);
    size_t const cchToCopy = (size_t)(posSelEnd - posSelStart);
    StringCchCopyNA(tmpRectSelN, _tmpBufCnt, SciCall_GetRangePointer(posSelStart, (DocPos)cchToCopy), cchToCopy);
    StrTrimA(tmpRectSelN, " ");

    if (!StrIsEmptyA(tmpRectSelN))
    {
      if (IsDigitA(tmpRectSelN[0]) && bLastCharWasDigit) {
        StringCchCatA(calcBuffer, SizeOfMem(calcBuffer), "+"); // default: add numbers
      }
      bLastCharWasDigit = IsDigitA(tmpRectSelN[StringCchLenA(tmpRectSelN,COUNTOF(tmpRectSelN)) - 1]);
      StringCchCatA(calcBuffer, SizeOfMem(calcBuffer), tmpRectSelN);
    }
  }
  return te_interp(calcBuffer, piExprError);
}


//=============================================================================
//
//  UpdateStatusbar()
//
//
void UpdateStatusbar(bool bForceRedraw)
{
  _DelayUpdateStatusbar(_MQ_FAST, bForceRedraw);
}

//=============================================================================

const static WCHAR* FR_Status[] = { L"[>--<]", L"[>>--]", L"[>>-+]", L"[+->]>", L"[--<<]", L"[+-<<]", L"<[<-+]"};

static void  _UpdateStatusbarDelayed(bool bForceRedraw)
{
  if (!Settings.ShowStatusbar) { return; }

  static sectionTxt_t tchStatusBar[STATUS_SECTOR_COUNT];

  // ------------------------------------------------------
  // common calculations 
  // ------------------------------------------------------
  DocPos const iPos              = SciCall_GetCurrentPos();
  DocLn  const iLnFromPos        = SciCall_LineFromPosition(iPos);
  DocPos const iLineBegin        = SciCall_PositionFromLine(iLnFromPos);
  DocPos const iLineBack         = SciCall_GetLineEndPosition(iLnFromPos);
  DocPos const iSelStart         = SciCall_GetSelectionStart();
  DocPos const iSelEnd           = SciCall_GetSelectionEnd();

  bool const   bIsSelectionEmpty = SciCall_IsSelectionEmpty();
  bool const   bIsSelCharCountable = !(bIsSelectionEmpty || Sci_IsMultiOrRectangleSelection());
  bool const   bIsMultiSelection = Sci_IsMultiSelection();
  bool const   bIsWindowFindReplace = IsWindow(Globals.hwndDlgFindReplace);

  bool bIsUpdateNeeded = bForceRedraw;

  static WCHAR tchLn[32] = { L'\0' };
  static WCHAR tchLines[32] = { L'\0' };

  // ------------------------------------------------------

  if (g_iStatusbarVisible[STATUS_DOCLINE] || bIsWindowFindReplace)
  {
    static DocLn s_iLnFromPos = -1;
    static DocLn s_iLnCnt = -1;

    if (bForceRedraw || (s_iLnFromPos != iLnFromPos)) {
      StringCchPrintf(tchLn, COUNTOF(tchLn), DOCPOSFMTW, iLnFromPos + 1);
      FormatNumberStr(tchLn, COUNTOF(tchLn), 0);
    }

    DocLn const  iLnCnt = SciCall_GetLineCount();
    if (bForceRedraw || (s_iLnCnt != iLnCnt)) {
      StringCchPrintf(tchLines, COUNTOF(tchLines), DOCPOSFMTW, iLnCnt);
      FormatNumberStr(tchLines, COUNTOF(tchLines), 0);
    }

    if (bForceRedraw || ((s_iLnFromPos != iLnFromPos) || (s_iLnCnt != iLnCnt)))
    {
      StringCchPrintf(tchStatusBar[STATUS_DOCLINE], txtWidth, L"%s%s / %s%s",
        g_mxSBPrefix[STATUS_DOCLINE], tchLn, tchLines, g_mxSBPostfix[STATUS_DOCLINE]);
      s_iLnFromPos = iLnFromPos;
      s_iLnCnt = iLnCnt;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  static WCHAR tchCol[32] = { L'\0' };

  if (g_iStatusbarVisible[STATUS_DOCCOLUMN] || bIsWindowFindReplace)
  {
    DocPos const colOffset = Globals.bZeroBasedColumnIndex ? 0 : 1;

    static DocPos s_iCol = -1;
    DocPos const iCol = SciCall_GetColumn(iPos) + SciCall_GetSelectionNCaretVirtualSpace(0);
    if (bForceRedraw || (s_iCol != iCol)) {
      StringCchPrintf(tchCol, COUNTOF(tchCol), DOCPOSFMTW, iCol + colOffset);
      FormatNumberStr(tchCol, COUNTOF(tchCol), 0);
    }

    static DocPos s_iCols = -1;
    static WCHAR tchCols[32] = { L'\0' };
    DocPos const iCols = SciCall_GetColumn(iLineBack);
    if (bForceRedraw || (s_iCols != iCols)) {
      StringCchPrintf(tchCols, COUNTOF(tchCols), DOCPOSFMTW, iCols);
      FormatNumberStr(tchCols, COUNTOF(tchCols), 0);
    }

    if (bForceRedraw || ((s_iCol != iCol) || (s_iCols != iCols))) {
      StringCchPrintf(tchStatusBar[STATUS_DOCCOLUMN], txtWidth, L"%s%s / %s%s",
        g_mxSBPrefix[STATUS_DOCCOLUMN], tchCol, tchCols, g_mxSBPostfix[STATUS_DOCCOLUMN]);

      s_iCol = iCol;
      s_iCols = iCols;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  if (g_iStatusbarVisible[STATUS_DOCCHAR])
  {
    static WCHAR tchChr[32] = { L'\0' };
    static WCHAR tchChrs[32] = { L'\0' };
    static DocPos s_iChr = -1;
    DocPos const chrOffset = Globals.bZeroBasedCharacterCount ? 0 : 1;

    DocPos const iChr = SciCall_CountCharacters(iLineBegin, iPos);
    if (bForceRedraw || (s_iChr != iChr)) {
      StringCchPrintf(tchChr, COUNTOF(tchChr), DOCPOSFMTW, iChr + chrOffset);
      FormatNumberStr(tchChr, COUNTOF(tchChr), 0);
    }

    static DocPos s_iChrs = -1;
    DocPos const iChrs = SciCall_CountCharacters(iLineBegin, iLineBack);
    if (bForceRedraw || (s_iChrs != iChrs)) {
      StringCchPrintf(tchChrs, COUNTOF(tchChrs), DOCPOSFMTW, iChrs);
      FormatNumberStr(tchChrs, COUNTOF(tchChrs), 0);
    }

    if ((s_iChr != iChr) || (s_iChrs != iChrs)) {
      StringCchPrintf(tchStatusBar[STATUS_DOCCHAR], txtWidth, L"%s%s / %s%s",
        g_mxSBPrefix[STATUS_DOCCHAR], tchChr, tchChrs, g_mxSBPostfix[STATUS_DOCCHAR]);

      s_iChr = iChr;
      s_iChrs = iChrs;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  static WCHAR tchSel[32] = { L'\0' };

  // number of selected chars in statusbar
  if (g_iStatusbarVisible[STATUS_SELECTION] || g_iStatusbarVisible[STATUS_SELCTBYTES] || bIsWindowFindReplace)
  {
    static bool s_bIsSelCountable = false;
    static bool s_bIsMultiSelection = false;
    static DocPos s_iSelStart = -1;
    static DocPos s_iSelEnd = -1;

    if (bForceRedraw || ((s_bIsSelCountable != bIsSelCharCountable) || (s_iSelStart != iSelStart) 
        || (s_iSelEnd != iSelEnd)) || (s_bIsMultiSelection != bIsMultiSelection))
    {
      static WCHAR tchSelB[64] = { L'\0' };
      if (bIsSelCharCountable)
      {
        const DocPos iSel = (DocPos)SendMessage(Globals.hwndEdit, SCI_COUNTCHARACTERS, iSelStart, iSelEnd);
        StringCchPrintf(tchSel, COUNTOF(tchSel), DOCPOSFMTW, iSel);
        FormatNumberStr(tchSel, COUNTOF(tchSel), 0);
        StrFormatByteSize((iSelEnd - iSelStart), tchSelB, COUNTOF(tchSelB));
      }
      else if (bIsMultiSelection) {
        StringCchPrintf(tchSel, COUNTOF(tchSel), L"# " DOCPOSFMTW, SciCall_GetSelections());
        tchSelB[0] = L'0'; tchSelB[1] = L'\0';
      }
      else {
        tchSel[0] = L'-'; tchSel[1] = L'-'; tchSel[2] = L'\0';
        tchSelB[0] = L'0'; tchSelB[1] = L'\0';
      }
      StringCchPrintf(tchStatusBar[STATUS_SELECTION], txtWidth, L"%s%s%s",
        g_mxSBPrefix[STATUS_SELECTION], tchSel, g_mxSBPostfix[STATUS_SELECTION]);

      StringCchPrintf(tchStatusBar[STATUS_SELCTBYTES], txtWidth, L"%s%s%s",
        g_mxSBPrefix[STATUS_SELCTBYTES], tchSelB, g_mxSBPostfix[STATUS_SELCTBYTES]);

      s_bIsSelCountable = bIsSelCharCountable;
      s_bIsMultiSelection = bIsMultiSelection;
      s_iSelStart = iSelStart;
      s_iSelEnd = iSelEnd;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  // number of selected lines in statusbar
  if (g_iStatusbarVisible[STATUS_SELCTLINES])
  {
    static bool s_bIsSelectionEmpty = true;
    static DocLn s_iLinesSelected = -1;

    DocLn const iLineStart = SciCall_LineFromPosition(iSelStart);
    DocLn const iLineEnd = SciCall_LineFromPosition(iSelEnd);
    DocPos const iStartOfLinePos = SciCall_PositionFromLine(iLineEnd);

    DocLn const iLinesSelected = ((iSelStart != iSelEnd) && (iStartOfLinePos != iSelEnd)) ? ((iLineEnd - iLineStart) + 1) : (iLineEnd - iLineStart);

    if (bForceRedraw || ((s_bIsSelectionEmpty != bIsSelectionEmpty) || (s_iLinesSelected != iLinesSelected)))
    {
      static bool s_bIsMultiSelection = false;
      static WCHAR tchLinesSelected[32] = { L'\0' };
      if (bIsSelectionEmpty || bIsMultiSelection) {
        tchLinesSelected[0] = L'-';
        tchLinesSelected[1] = L'-';
        tchLinesSelected[2] = L'\0';
      }
      else {
        StringCchPrintf(tchLinesSelected, COUNTOF(tchLinesSelected), DOCPOSFMTW, iLinesSelected);
        FormatNumberStr(tchLinesSelected, COUNTOF(tchLinesSelected), 0);
      }
      StringCchPrintf(tchStatusBar[STATUS_SELCTLINES], txtWidth, L"%s%s%s",
        g_mxSBPrefix[STATUS_SELCTLINES], tchLinesSelected, g_mxSBPostfix[STATUS_SELCTLINES]);

      s_bIsSelectionEmpty = bIsSelectionEmpty;
      s_bIsMultiSelection = bIsMultiSelection;
      s_iLinesSelected = iLinesSelected;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  // try calculate expression of selection
  if (g_iStatusbarVisible[STATUS_TINYEXPR])
  {
    static WCHAR tchExpression[32] = { L'\0' };
    static te_xint_t s_iExErr          = -3;
    s_dExpression = 0.0;
    tchExpression[0] = L'-';
    tchExpression[1] = L'-';
    tchExpression[2] = L'\0';

    if (Settings.EvalTinyExprOnSelection)
    {
      if (bIsSelCharCountable) {
        static char  chSelectionBuffer[XHUGE_BUFFER];
        DocPos const iSelSize = SciCall_GetSelText(NULL);
        if (iSelSize < COUNTOF(chSelectionBuffer)) // should be fast !
        {
          SciCall_GetSelText(chSelectionBuffer);
          //~StrDelChrA(chExpression, " \r\n\t\v");
          StrDelChrA(chSelectionBuffer, "\r\n");
          s_dExpression = te_interp(chSelectionBuffer, &s_iExprError);
        }
        else {
          s_iExprError = -1;
        }
      }
      else if (Sci_IsMultiOrRectangleSelection() && !bIsSelectionEmpty) {
        s_dExpression = _InterpMultiSelectionTinyExpr(&s_iExprError);
      }
      else
        s_iExprError = -2;
    }
    else {
      s_iExprError = -3;
    }

    if (!s_iExprError) {
      if (fabs(s_dExpression) > 99999999.9999)
        StringCchPrintf(tchExpression, COUNTOF(tchExpression), L"%.4E", s_dExpression);
      else
        StringCchPrintf(tchExpression, COUNTOF(tchExpression), L"%.6G", s_dExpression);
    }
    else if (s_iExprError > 0) {
#ifdef _WIN64
        StringCchPrintf(tchExpression, COUNTOF(tchExpression), L"^[%ll]", s_iExprError);
#else
        StringCchPrintf(tchExpression, COUNTOF(tchExpression), L"^[%i]", s_iExprError);
#endif
    }

    if (bForceRedraw || (!s_iExprError || (s_iExErr != s_iExprError))) 
    {
      StringCchPrintf(tchStatusBar[STATUS_TINYEXPR], txtWidth, L"%s%s%s ",
        g_mxSBPrefix[STATUS_TINYEXPR], tchExpression, g_mxSBPostfix[STATUS_TINYEXPR]);

      s_iExErr = s_iExprError;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  static WCHAR tchOcc[32] = { L'\0' };

  // number of occurrence marks found
  if (g_iStatusbarVisible[STATUS_OCCURRENCE] || bIsWindowFindReplace)
  {
    static DocPosU s_iMarkOccurrencesCount = 0;
    static bool s_bMOVisible = false;
    if (bForceRedraw || ((s_bMOVisible != Settings.MarkOccurrencesMatchVisible) || (s_iMarkOccurrencesCount != Globals.iMarkOccurrencesCount)))
    {
      if (Globals.iMarkOccurrencesCount > 0)
      {
        StringCchPrintf(tchOcc, COUNTOF(tchOcc), DOCPOSFMTW, Globals.iMarkOccurrencesCount);
        FormatNumberStr(tchOcc, COUNTOF(tchOcc), 0);
      }
      else {
        StringCchCopy(tchOcc, COUNTOF(tchOcc), L"--");
      }
      if (Settings.MarkOccurrencesMatchVisible) {
        StringCchCat(tchOcc, COUNTOF(tchOcc), L"(V)");
      }

      StringCchPrintf(tchStatusBar[STATUS_OCCURRENCE], txtWidth, L"%s%s%s",
        g_mxSBPrefix[STATUS_OCCURRENCE], tchOcc, g_mxSBPostfix[STATUS_OCCURRENCE]);

      s_bMOVisible = Settings.MarkOccurrencesMatchVisible;
      s_iMarkOccurrencesCount = Globals.iMarkOccurrencesCount;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  // number of replaced pattern
  if (g_iStatusbarVisible[STATUS_OCCREPLACE] || bIsWindowFindReplace)
  {
    static int s_iReplacedOccurrences = -1;

    if (bForceRedraw || (s_iReplacedOccurrences != Globals.iReplacedOccurrences))
    {
      static WCHAR tchRepl[32] = { L'\0' };
      if (Globals.iReplacedOccurrences > 0)
      {
        StringCchPrintf(tchRepl, COUNTOF(tchRepl), L"%i", Globals.iReplacedOccurrences);
        FormatNumberStr(tchRepl, COUNTOF(tchRepl), 0);
      }
      else {
        StringCchCopy(tchRepl, COUNTOF(tchRepl), L"--");
      }

      StringCchPrintf(tchStatusBar[STATUS_OCCREPLACE], txtWidth, L"%s%s%s",
        g_mxSBPrefix[STATUS_OCCREPLACE], tchRepl, g_mxSBPostfix[STATUS_OCCREPLACE]);

      s_iReplacedOccurrences = Globals.iReplacedOccurrences;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  // get number of bytes in current encoding
  if (g_iStatusbarVisible[STATUS_DOCSIZE])
  {
    static DocPos s_iTextLength = -1;
    DocPos const iTextLength = SciCall_GetTextLength();
    if (bForceRedraw || (s_iTextLength != iTextLength)) 
    {
      static WCHAR tchBytes[32] = { L'\0' };
      StrFormatByteSize(iTextLength, tchBytes, COUNTOF(tchBytes));

      StringCchPrintf(tchStatusBar[STATUS_DOCSIZE], txtWidth, L"%s%s%s",
        g_mxSBPrefix[STATUS_DOCSIZE], tchBytes, g_mxSBPostfix[STATUS_DOCSIZE]);

      s_iTextLength = iTextLength;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  if (g_iStatusbarVisible[STATUS_CODEPAGE])
  {
    static cpi_enc_t s_iEncoding = CPI_NONE;
    cpi_enc_t const  iEncoding   = Encoding_GetCurrent();
    if (bForceRedraw || (s_iEncoding != iEncoding)) 
    {
      StringCchPrintf(tchStatusBar[STATUS_CODEPAGE], txtWidth, L"%s%s%s",
        g_mxSBPrefix[STATUS_CODEPAGE], Encoding_GetLabel(iEncoding), g_mxSBPostfix[STATUS_CODEPAGE]);

      s_iEncoding = iEncoding;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  const WCHAR* const _LF_f    = L"%sLF%s";
  const WCHAR* const _LFi_f   = L"%sLF*%s";
  const WCHAR* const _CR_f    = L"%sCR%s";
  const WCHAR* const _CRi_f   = L"%sCR*%s";
  const WCHAR* const _CRLF_f  = L"%sCR+LF%s";
  const WCHAR* const _CRLFi_f = L"%sCR+LF*%s";

  if (g_iStatusbarVisible[STATUS_EOLMODE]) 
  {
    static int s_iEOLMode = -1;
    int const eol_mode = SciCall_GetEOLMode();

    if (bForceRedraw || (s_iEOLMode != eol_mode))
    {
      static WCHAR tchEOL[16] = { L'\0' };
      if (eol_mode == SC_EOL_LF) 
      {
        StringCchPrintf(tchStatusBar[STATUS_EOLMODE], txtWidth, (Globals.bDocHasInconsistentEOLs ? _LFi_f : _LF_f),
          g_mxSBPrefix[STATUS_EOLMODE], g_mxSBPostfix[STATUS_EOLMODE]);
      }
      else if (eol_mode == SC_EOL_CR) 
      {
        StringCchPrintf(tchStatusBar[STATUS_EOLMODE], txtWidth, (Globals.bDocHasInconsistentEOLs ? _CRi_f : _CR_f),
          g_mxSBPrefix[STATUS_EOLMODE], g_mxSBPostfix[STATUS_EOLMODE]);
      }
      else {
        StringCchPrintf(tchStatusBar[STATUS_EOLMODE], txtWidth, (Globals.bDocHasInconsistentEOLs ? _CRLFi_f : _CRLF_f),
          g_mxSBPrefix[STATUS_EOLMODE], g_mxSBPostfix[STATUS_EOLMODE]);
      }
      s_iEOLMode = eol_mode;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  if (g_iStatusbarVisible[STATUS_OVRMODE])
  {
    static bool s_bIsOVR = -1;
    bool const bIsOVR = (bool)SendMessage(Globals.hwndEdit, SCI_GETOVERTYPE, 0, 0);
    if (bForceRedraw || (s_bIsOVR != bIsOVR)) 
    {
      if (bIsOVR)
      {
        StringCchPrintf(tchStatusBar[STATUS_OVRMODE], txtWidth, L"%sOVR%s",
          g_mxSBPrefix[STATUS_OVRMODE], g_mxSBPostfix[STATUS_OVRMODE]);
      }
      else {
        StringCchPrintf(tchStatusBar[STATUS_OVRMODE], txtWidth, L"%sINS%s",
          g_mxSBPrefix[STATUS_OVRMODE], g_mxSBPostfix[STATUS_OVRMODE]);
      }
      s_bIsOVR = bIsOVR;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  if (g_iStatusbarVisible[STATUS_2ND_DEF])
  {
    static bool s_bUse2ndDefault = -1;
    bool const bUse2ndDefault = Style_GetUse2ndDefault();
    if (bForceRedraw || (s_bUse2ndDefault != bUse2ndDefault))
    {
      if (bUse2ndDefault)
        StringCchPrintf(tchStatusBar[STATUS_2ND_DEF], txtWidth, L"%s2ND%s",
          g_mxSBPrefix[STATUS_2ND_DEF], g_mxSBPostfix[STATUS_2ND_DEF]);
      else
        StringCchPrintf(tchStatusBar[STATUS_2ND_DEF], txtWidth, L"%sSTD%s",
          g_mxSBPrefix[STATUS_2ND_DEF], g_mxSBPostfix[STATUS_2ND_DEF]);

      s_bUse2ndDefault = bUse2ndDefault;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  if (g_iStatusbarVisible[STATUS_LEXER])
  {
    static int s_iCurLexer = -1;
    int const iCurLexer = Style_GetCurrentLexerRID();
    if (bForceRedraw || (s_iCurLexer != iCurLexer))
    {
      static WCHAR tchLexerName[MINI_BUFFER];
      if (Style_IsCurLexerStandard())
        Style_GetLexerDisplayName(NULL, tchLexerName, MINI_BUFFER); // don't distinguish between STD/2ND
      else
        Style_GetLexerDisplayName(Style_GetCurrentLexerPtr(), tchLexerName, MINI_BUFFER);

      StringCchPrintf(tchStatusBar[STATUS_LEXER], txtWidth, L"%s%s%s",
        g_mxSBPrefix[STATUS_LEXER], tchLexerName, g_mxSBPostfix[STATUS_LEXER]);

      s_iCurLexer = iCurLexer;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  // ====  Statusbar widths  ====

  int g_vStatusbarSectionWidth[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

  _CalculateStatusbarSections(g_vStatusbarSectionWidth, tchStatusBar, &bIsUpdateNeeded);

  if (bIsUpdateNeeded) {
    int aStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_ZERO;
    BYTE cnt = 0;
    int totalWidth = 0;
    for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
      int const id = g_vSBSOrder[i];
      if ((id >= 0) && (g_vStatusbarSectionWidth[id] >= 0))
      {
        totalWidth += g_vStatusbarSectionWidth[id];
        aStatusbarSections[cnt++] = totalWidth;
      }
    }

    if (cnt > 0) { aStatusbarSections[cnt - 1] = -1; }
    else { aStatusbarSections[0] = -1; Settings.ShowStatusbar = false; }

    SendMessage(Globals.hwndStatus, SB_SETPARTS, (WPARAM)cnt, (LPARAM)aStatusbarSections);

    cnt = 0;
    for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
      int const id = g_vSBSOrder[i];
      if ((id >= 0) && (g_vStatusbarSectionWidth[id] >= 0)) {
        StatusSetText(Globals.hwndStatus, cnt++, tchStatusBar[id]);
      }
    }
  }
  // --------------------------------------------------------------------------

  // update Find/Replace dialog (if any)
  if (bIsWindowFindReplace) {
    static WCHAR tchReplOccs[32] = { L'\0' };
    if (Globals.iReplacedOccurrences > 0)
      StringCchPrintf(tchReplOccs, COUNTOF(tchReplOccs), L"%i", Globals.iReplacedOccurrences);
    else
      StringCchCopy(tchReplOccs, COUNTOF(tchReplOccs), L"--");

    const WCHAR* SBFMT = L" %s%s / %s     %s%s     %s%s     %s%s     %s%s     (  %s  )              ";

    static WCHAR tchFRStatus[128] = { L'\0' };
    StringCchPrintf(tchFRStatus, COUNTOF(tchFRStatus), SBFMT,
      g_mxSBPrefix[STATUS_DOCLINE], tchLn, tchLines,
      g_mxSBPrefix[STATUS_DOCCOLUMN], tchCol,
      g_mxSBPrefix[STATUS_SELECTION], tchSel,
      g_mxSBPrefix[STATUS_OCCURRENCE], tchOcc,
      g_mxSBPrefix[STATUS_OCCREPLACE], tchReplOccs,
      FR_Status[Globals.FindReplaceMatchFoundState]);

    SetWindowText(GetDlgItem(Globals.hwndDlgFindReplace, IDS_FR_STATUS_TEXT), tchFRStatus);
  }

}


//=============================================================================
//
//  UpdateMarginWidth()
//
//
void UpdateMarginWidth()
{
  if (Settings.ShowLineNumbers)
  {
    static char chLines[32] = { '\0' };
    StringCchPrintfA(chLines, COUNTOF(chLines), "_%td", (size_t)SciCall_GetLineCount());

    int const iLineMarginWidthNow = SciCall_GetMarginWidthN(MARGIN_SCI_LINENUM);
    int const iLineMarginWidthFit = SciCall_TextWidth(STYLE_LINENUMBER, chLines);

    if (iLineMarginWidthNow != iLineMarginWidthFit) {
      SciCall_SetMarginWidthN(MARGIN_SCI_LINENUM, iLineMarginWidthFit);
    }
  }
  else {
    SciCall_SetMarginWidthN(MARGIN_SCI_LINENUM, 0);
  }

  Style_SetBookmark(Globals.hwndEdit, Settings.ShowBookmarkMargin);
  Style_SetFolding(Globals.hwndEdit, (FocusedView.CodeFoldingAvailable && FocusedView.ShowCodeFolding));
}



//=============================================================================
//
//  UpdateSaveSettingsCmds()
//
//
void UpdateSaveSettingsCmds()
{
  bool const bSoftLocked = Flags.bSettingsFileSoftLocked;
  bool const bHaveIniFile = StrIsNotEmpty(Globals.IniFile);
  bool const bHaveFallbackIniFile = StrIsNotEmpty(Globals.IniFileDefault);
  CheckCmd(Globals.hMainMenu, IDM_VIEW_SAVESETTINGS, Settings.SaveSettings && !bSoftLocked);
  EnableCmd(Globals.hMainMenu, IDM_VIEW_SAVESETTINGS, Globals.bCanSaveIniFile && !bSoftLocked);
  EnableCmd(Globals.hMainMenu, IDM_VIEW_SAVESETTINGSNOW, (bHaveIniFile || bHaveFallbackIniFile) && !bSoftLocked);
  EnableCmd(Globals.hMainMenu, CMD_OPENINIFILE, bHaveIniFile && !bSoftLocked);
}


//=============================================================================
//
//  UpdateUI()
//
void UpdateUI()
{
  struct SCNotification scn;
  scn.nmhdr.hwndFrom = Globals.hwndEdit;
  scn.nmhdr.idFrom = IDC_EDIT;
  scn.nmhdr.code = SCN_UPDATEUI;
  scn.updated = (SC_UPDATE_CONTENT/* | SC_UPDATE_NP3_INTERNAL_NOTIFY */);
  SendMessage(Globals.hwndMain, WM_NOTIFY, IDC_EDIT, (LPARAM)&scn);
  //PostMessage(Globals.hwndMain, WM_NOTIFY, IDC_EDIT, (LPARAM)&scn);
  COND_SHOW_ZOOM_CALLTIP();
}


//=============================================================================
//
//  UpdateTitleBar()
//
void UpdateTitleBar() {
  SetWindowTitle(Globals.hwndMain, s_uidsAppTitle, s_bIsProcessElevated, IDS_MUI_UNTITLED, Globals.CurrentFile,
                 Settings.PathNameFormat, GetDocModified(), IDS_MUI_READONLY, s_bFileReadOnly, s_wchTitleExcerpt);
  PostMessage(Globals.hwndMain, WM_NCACTIVATE, FALSE, -1);
  PostMessage(Globals.hwndMain, WM_NCACTIVATE, TRUE, 0);
}


//=============================================================================

#define UNDOREDO_FREE (-1L)
#define UNDOREDO_BLOCKED (-2L)
static volatile LONG UndoActionToken = UNDOREDO_BLOCKED; // block

//=============================================================================

static bool  _InUndoRedoTransaction() {
  return (InterlockedOr(&UndoActionToken, 0L) != UNDOREDO_FREE);
}

//=============================================================================
//
//  UndoRedoRecordingStart()
//
void UndoRedoRecordingStart()
{
  InterlockedExchange(&UndoActionToken, UNDOREDO_FREE); // clear
  _UndoRedoActionMap(-1, NULL);
  SciCall_SetUndoCollection(true);
}


//=============================================================================
//
//  UndoRedoRecordingStop()
//
void UndoRedoRecordingStop()
{
  int const curToken = InterlockedOr(&UndoActionToken, 0L);
  if (curToken >= 0) { EndUndoAction(curToken); }

  _UndoRedoActionMap(-1, NULL);

  SciCall_SetUndoCollection(false);
  SciCall_EmptyUndoBuffer();
}


//=============================================================================
//
//  UndoRedoReset()
//
void UndoRedoReset()
{
  UndoRedoRecordingStop();
  UndoRedoRecordingStart();
}

//=============================================================================
//
//  _SaveUndoSelection()
//
//
static int _SaveUndoSelection()
{
  static DocPosU _s_iSelection = 0;           // index

  UndoRedoSelection_t sel = INIT_UNDOREDOSEL;
  CopyUndoRedoSelection(&sel, NULL); // init

  DocPosU const numOfSel = SciCall_GetSelections();

  // each single selection of a multi-selection will call thid method
  // we are only interested in the first call
  if (0 == _s_iSelection) {
    _s_iSelection = numOfSel;
  }
  if ((numOfSel-1) != --_s_iSelection) {
    return -1;
  }

  int const selMode = ((numOfSel > 1) && !SciCall_IsSelectionRectangle()) ? NP3_SEL_MULTI : SciCall_GetSelectionMode();

  sel.selMode_undo = selMode;

  switch (selMode)
  {
    case NP3_SEL_MULTI:
    {
      for (DocPosU i = 0; i < numOfSel; ++i) {
        if (sel.anchorPos_undo && sel.curPos_undo) {
          DocPos const anchorPos = SciCall_GetSelectionNAnchor(i);
          utarray_push_back(sel.anchorPos_undo, &anchorPos);
          DocPos const curPos = SciCall_GetSelectionNCaret(i);
          utarray_push_back(sel.curPos_undo, &curPos);
        }
        if (!Settings2.DenyVirtualSpaceAccess && sel.anchorVS_undo && sel.curVS_undo) {
          DocPos const anchorVS = SciCall_GetSelectionNAnchorVirtualSpace(i);
          utarray_push_back(sel.anchorVS_undo, &anchorVS);
          DocPos const curVS = SciCall_GetSelectionNCaretVirtualSpace(i);
          utarray_push_back(sel.curVS_undo, &curVS);
        }
      }
    }
    break;

    case SC_SEL_RECTANGLE:
    case SC_SEL_THIN:
    {
      DocPos const anchorPos = SciCall_GetRectangularSelectionAnchor();
      utarray_push_back(sel.anchorPos_undo, &anchorPos);
      DocPos const curPos = SciCall_GetRectangularSelectionCaret();
      utarray_push_back(sel.curPos_undo, &curPos);
      if (!Settings2.DenyVirtualSpaceAccess) {
        DocPos const anchorVS = SciCall_GetRectangularSelectionAnchorVirtualSpace();
        utarray_push_back(sel.anchorVS_undo, &anchorVS);
        DocPos const curVS = SciCall_GetRectangularSelectionCaretVirtualSpace();
        utarray_push_back(sel.curVS_undo, &curVS);
      }
    }
    break;

    case SC_SEL_LINES:
    case SC_SEL_STREAM:
    default:
    {
      DocPos const anchorPos = SciCall_GetAnchor();
      utarray_push_back(sel.anchorPos_undo, &anchorPos);
      DocPos const curPos = SciCall_GetCurrentPos();
      utarray_push_back(sel.curPos_undo, &curPos);
      DocPos const dummy = (DocPos)-1;
      utarray_push_back(sel.anchorVS_undo, &dummy);
      utarray_push_back(sel.curVS_undo, &dummy);
    }
    break;
  }

  const UndoRedoSelection_t* pSel = &sel;
  int const token = _UndoRedoActionMap(-1, &pSel);

  if (token >= 0) {
    //~SciCall_AddUndoAction(token, UNDO_MAY_COALESCE);
    SciCall_AddUndoAction(token, UNDO_NONE);
  }
  _s_iSelection = 0; // reset

  return token;
}


//=============================================================================
//
//  _SaveRedoSelection()
//
//
static void  _SaveRedoSelection(int token)
{
  static DocPosU _s_iSelection = 0;  // index

  if (token < 0) { return; }

  UndoRedoSelection_t* pSel = NULL;

  DocPosU const numOfSel = SciCall_GetSelections();

  // each single selection of a multi-selection will call this method
  // we are only interested in the last call
  if (0 == _s_iSelection) {
    _s_iSelection = numOfSel;
  }
  if (0 != --_s_iSelection) {
    return;
  }

  if ((_UndoRedoActionMap(token, &pSel) >= 0) && (pSel != NULL))
  {
    int const selMode = ((numOfSel > 1) && !SciCall_IsSelectionRectangle()) ? NP3_SEL_MULTI : SciCall_GetSelectionMode();
    
    pSel->selMode_redo = selMode;

    switch (selMode)
    {
      case NP3_SEL_MULTI:
      {
        for (DocPosU i = 0; i < numOfSel; ++i) {
          DocPos const anchorPos = SciCall_GetSelectionNAnchor(i);
          utarray_push_back(pSel->anchorPos_redo, &anchorPos);
          DocPos const curPos = SciCall_GetSelectionNCaret(i);
          utarray_push_back(pSel->curPos_redo, &curPos);
          if (!Settings2.DenyVirtualSpaceAccess) {
            DocPos const anchorVS = SciCall_GetSelectionNAnchorVirtualSpace(i);
            utarray_push_back(pSel->anchorVS_redo, &anchorVS);
            DocPos const curVS = SciCall_GetSelectionNCaretVirtualSpace(i);
            utarray_push_back(pSel->curVS_redo, &curVS);
          }
        }
      }
      break;

      case SC_SEL_RECTANGLE:
      case SC_SEL_THIN:
      {
        DocPos const anchorPos = SciCall_GetRectangularSelectionAnchor();
        utarray_push_back(pSel->anchorPos_redo, &anchorPos);
        DocPos const curPos = SciCall_GetRectangularSelectionCaret();
        utarray_push_back(pSel->curPos_redo, &curPos);
        if (!Settings2.DenyVirtualSpaceAccess) {
          DocPos const anchorVS = SciCall_GetRectangularSelectionAnchorVirtualSpace();
          utarray_push_back(pSel->anchorVS_redo, &anchorVS);
          DocPos const curVS = SciCall_GetRectangularSelectionCaretVirtualSpace();
          utarray_push_back(pSel->curVS_redo, &curVS);
        }
      }
      break;

      case SC_SEL_LINES:
      case SC_SEL_STREAM:
      default:
      {
        DocPos const anchorPos = SciCall_GetAnchor();
        utarray_push_back(pSel->anchorPos_redo, &anchorPos);
        DocPos const curPos = SciCall_GetCurrentPos();
        utarray_push_back(pSel->curPos_redo, &curPos);
        //~DocPos const dummy = (DocPos)-1;
        //~utarray_push_back(pSel->anchorVS_redo, &dummy);
        //~utarray_push_back(pSel->curVS_redo, &dummy);
      }
      break;
    }
  }
}


//=============================================================================
//
//  BeginUndoAction()
//
//
int BeginUndoAction()
{
  if (_InUndoRedoTransaction()) { return -1; }
  SciCall_BeginUndoAction();
  int const token = _SaveUndoSelection();
  InterlockedExchange(&UndoActionToken, (LONG)token);
  return token;
}



//=============================================================================
//
//  EndUndoAction()
//
//
void EndUndoAction(int token)
{
  if ((token >= 0) && (token == (int)InterlockedOr(&UndoActionToken, 0L)))
  {
    _SaveRedoSelection(token);
    SciCall_EndUndoAction();
    InterlockedExchange(&UndoActionToken, UNDOREDO_FREE);
  }
}


//=============================================================================
//
//  RestoreAction()
//
//
bool RestoreAction(int token, DoAction doAct)
{
  if (_InUndoRedoTransaction()) { return false; }

  UndoRedoSelection_t* pSel = NULL;

  if ((_UndoRedoActionMap(token, &pSel) >= 0) && (pSel != NULL))
  {
    // we are inside undo/redo transaction, so do delayed PostMessage() instead of SendMessage()
    HWND const hwndedit = Globals.hwndEdit;

    DocPos* pPosAnchor = NULL;
    DocPos* pPosCur = NULL;
    DocPos* pPosAnchorVS = NULL;
    DocPos* pPosCurVS = NULL;
    pPosAnchor = (DocPos*)((UNDO == doAct) ? utarray_front(pSel->anchorPos_undo) : utarray_front(pSel->anchorPos_redo));
    pPosCur = (DocPos*)((UNDO == doAct) ? utarray_front(pSel->curPos_undo) : utarray_front(pSel->curPos_redo));
    pPosAnchorVS = (DocPos*)((UNDO == doAct) ? utarray_front(pSel->anchorVS_undo) : utarray_front(pSel->anchorVS_redo));
    pPosCurVS = (DocPos*)((UNDO == doAct) ? utarray_front(pSel->curVS_undo) : utarray_front(pSel->curVS_redo));

    if (pPosAnchor && pPosCur)
    {
      // Ensure that the first and last lines of a selection are always unfolded
      // This needs to be done _before_ the SCI_SETSEL message
      DocLn const anchorPosLine = SciCall_LineFromPosition((*pPosAnchor));
      DocLn const currPosLine = SciCall_LineFromPosition((*pPosCur));
      PostMessage(hwndedit, SCI_ENSUREVISIBLE, anchorPosLine, 0);
      if (anchorPosLine != currPosLine) { PostMessage(hwndedit, SCI_ENSUREVISIBLE, currPosLine, 0); }


      int const selectionMode = (UNDO == doAct) ? pSel->selMode_undo : pSel->selMode_redo;

      PostMessage(hwndedit, SCI_SETSELECTIONMODE, (WPARAM)((selectionMode == NP3_SEL_MULTI) ? SC_SEL_STREAM : selectionMode), 0);

      switch (selectionMode)
      {
        case NP3_SEL_MULTI:
        {
          unsigned int i = 0;

          DocPosU const selCount = (UNDO == doAct) ? utarray_len(pSel->anchorPos_undo) : utarray_len(pSel->anchorPos_redo);
          DocPosU const selCountVS = (UNDO == doAct) ? utarray_len(pSel->anchorVS_undo) : utarray_len(pSel->anchorVS_redo);

          PostMessage(hwndedit, SCI_SETSELECTION, (WPARAM)(*pPosCur), (LPARAM)(*pPosAnchor));
          if (pPosAnchorVS && pPosCurVS) {
            PostMessage(hwndedit, SCI_SETSELECTIONNANCHORVIRTUALSPACE, (WPARAM)0, (LPARAM)(*pPosAnchorVS));
            PostMessage(hwndedit, SCI_SETSELECTIONNCARETVIRTUALSPACE, (WPARAM)0, (LPARAM)(*pPosCurVS));
          }
          PostMessage(hwndedit, SCI_CANCEL, 0, 0); // (!) else shift-key selection behavior is kept

          ++i;
          while (i < selCount)
          {
            pPosAnchor = (DocPos*)((UNDO == doAct) ? utarray_eltptr(pSel->anchorPos_undo, i) : utarray_eltptr(pSel->anchorPos_redo, i));
            pPosCur = (DocPos*)((UNDO == doAct) ? utarray_eltptr(pSel->curPos_undo, i) : utarray_eltptr(pSel->curPos_redo, i));
            if (pPosAnchor && pPosCur) {
              PostMessage(hwndedit, SCI_ADDSELECTION, (WPARAM)(*pPosCur), (LPARAM)(*pPosAnchor));
              if (i < selCountVS) {
                pPosAnchorVS = (DocPos*)((UNDO == doAct) ? utarray_eltptr(pSel->anchorVS_undo, i) : utarray_eltptr(pSel->anchorVS_redo, i));
                pPosCurVS = (DocPos*)((UNDO == doAct) ? utarray_eltptr(pSel->curVS_undo, i) : utarray_eltptr(pSel->curVS_redo, i));
                if (pPosAnchorVS && pPosCurVS) {
                  PostMessage(hwndedit, SCI_SETSELECTIONNANCHORVIRTUALSPACE, (WPARAM)i, (LPARAM)(*pPosAnchorVS));
                  PostMessage(hwndedit, SCI_SETSELECTIONNCARETVIRTUALSPACE, (WPARAM)i, (LPARAM)(*pPosCurVS));
                }
              }
            }
            ++i;
          }
          //~PostMessage(hwndedit, SCI_SETMAINSELECTION, (WPARAM)0, (LPARAM)0);
        }
        break;

        case SC_SEL_RECTANGLE:
        case SC_SEL_THIN:
          PostMessage(Globals.hwndEdit, SCI_SETRECTANGULARSELECTIONANCHOR, (WPARAM)(*pPosAnchor), 0);
          PostMessage(Globals.hwndEdit, SCI_SETRECTANGULARSELECTIONCARET, (WPARAM)(*pPosCur), 0);
          if (pPosAnchorVS && pPosCurVS) {
            PostMessage(hwndedit, SCI_SETRECTANGULARSELECTIONANCHORVIRTUALSPACE, (WPARAM)(*pPosAnchorVS), 0);
            PostMessage(hwndedit, SCI_SETRECTANGULARSELECTIONCARETVIRTUALSPACE, (WPARAM)(*pPosCurVS), 0);
          }
          PostMessage(hwndedit, SCI_CANCEL, 0, 0); // (!) else shift-key selection behavior is kept
          break;

        case SC_SEL_LINES:
        case SC_SEL_STREAM:
        default:
          PostMessage(hwndedit, SCI_SETSELECTION, (WPARAM)(*pPosCur), (LPARAM)(*pPosAnchor));
          PostMessage(hwndedit, SCI_CANCEL, 0, 0); // (!) else shift-key selection behavior is kept
          break;
      }
    }
    PostMessage(hwndedit, SCI_SCROLLCARET, 0, 0);
    PostMessage(hwndedit, SCI_CHOOSECARETX, 0, 0);
  }
  return true;
}


//=============================================================================
//
//  _UndoRedoActionMap()
//
//
static int  _UndoRedoActionMap(int token, const UndoRedoSelection_t** selection)
{
  if (UndoRedoSelectionUTArray == NULL) { return -1; };

  static unsigned int uiTokenCnt = 0U;

  // indexing is unsigned
  unsigned int utoken = (token >= 0) ? (unsigned int)token : 0U;

  if (selection == NULL) {
    // reset / clear
    int const curToken = InterlockedOr(&UndoActionToken, 0L);
    if (curToken >= 0) { EndUndoAction(curToken); }
    utarray_clear(UndoRedoSelectionUTArray);
    utarray_init(UndoRedoSelectionUTArray, &UndoRedoSelection_icd);
    uiTokenCnt = 0U;
    InterlockedExchange(&UndoActionToken, UNDOREDO_FREE);
    return -1;
  }

  if (!SciCall_GetUndoCollection()) { return -1; }

  // get or set map item request ?
  if ((token >= 0) && (utoken < uiTokenCnt)) 
  {
    if ((*selection) == NULL) {
      // this is a get request
      (*selection) = (UndoRedoSelection_t*)utarray_eltptr(UndoRedoSelectionUTArray, utoken);
    }
    else {
      // this is a set request (fill redo pos)
      assert(false); // not used yet
      //utarray_insert(UndoRedoSelectionUTArray, (void*)(*selection), utoken);
    }
    // don't clear map item here (token used in redo/undo again)
  }
  else if (token < 0) {
    // set map new item request
    token = (int)uiTokenCnt;
    utarray_insert(UndoRedoSelectionUTArray, (void*)(*selection), uiTokenCnt);
    uiTokenCnt = (uiTokenCnt < (unsigned int)INT_MAX) ? (uiTokenCnt + 1U) : 0U;  // round robin next
  }
  return token;
}


//=============================================================================
//
//  _SplitUndoTransaction()
//
//
static void _SplitUndoTransaction(const int iModType) 
{
  if (!_InUndoRedoTransaction()) {
    if (!((iModType & SC_PERFORMED_UNDO) || (iModType & SC_PERFORMED_REDO))) {
      SciCall_BeginUndoAction();
      SciCall_EndUndoAction();
    }
  }
}


//=============================================================================
//
//  FileIO()
//
//
bool FileIO(bool fLoad,LPWSTR pszFileName,
            bool bSkipUnicodeDetect,bool bSkipANSICPDetection, bool bForceEncDetection, bool bSetSavePoint,
            EditFileIOStatus* status, bool bSaveCopy, bool bPreserveTimeStamp)
{
  WCHAR tch[MAX_PATH + 40];
  FormatLngStringW(tch, COUNTOF(tch), (fLoad) ? IDS_MUI_LOADFILE : IDS_MUI_SAVEFILE, PathFindFileName(pszFileName));
  bool fSuccess = false;

  BeginWaitCursor(true,tch);

  if (fLoad) {
    fSuccess = EditLoadFile(Globals.hwndEdit,pszFileName,bSkipUnicodeDetect,bSkipANSICPDetection,bForceEncDetection,bSetSavePoint,status);
  }
  else {
    int idx;
    if (MRU_FindFile(Globals.pFileMRU,pszFileName,&idx)) {
      Globals.pFileMRU->iEncoding[idx] = status->iEncoding;
      Globals.pFileMRU->iCaretPos[idx] = (Settings.PreserveCaretPos ? SciCall_GetCurrentPos() : -1);
      Globals.pFileMRU->iSelAnchPos[idx] = (Settings.PreserveCaretPos ? SciCall_GetAnchor() : -1);

      WCHAR wchBookMarks[MRU_BMRK_SIZE] = { L'\0' };
      EditGetBookmarkList(Globals.hwndEdit, wchBookMarks, COUNTOF(wchBookMarks));
      if (Globals.pFileMRU->pszBookMarks[idx]) {
        LocalFree(Globals.pFileMRU->pszBookMarks[idx]);  // StrDup()
      }
      Globals.pFileMRU->pszBookMarks[idx] = StrDup(wchBookMarks);
    }
    fSuccess = EditSaveFile(Globals.hwndEdit, pszFileName, status, bSaveCopy, bPreserveTimeStamp);
  }

  s_bFileReadOnly = IsReadOnly(GetFileAttributes(pszFileName));
  
  EndWaitCursor();

  return(fSuccess);
}


//=============================================================================
//
//  ConsistentIndentationCheck()
//
//
bool ConsistentIndentationCheck(EditFileIOStatus* status)
{
  bool const hasTabOrSpaceIndent = (status->indentCount[I_TAB_LN] > 0) && (status->indentCount[I_SPC_LN] > 0);
  bool const hasMixedIndents = (status->indentCount[I_MIX_LN] > 0);
  //bool const hasIrregularIndentDepth = (status->indentCount[I_TAB_MOD_X] > 0) || (status->indentCount[I_SPC_MOD_X] > 0);

  if (hasTabOrSpaceIndent || hasMixedIndents /*|| hasIrregularIndentDepth */)
  {
    if (WarnIndentationDlg(Globals.hwndMain, status))
    {
      bool const useTabs = SciCall_GetUseTabs();
      SciCall_SetUseTabs(status->iGlobalIndent == I_TAB_LN);
      bool const tabIndents = SciCall_GetTabIndents();
      SciCall_SetTabIndents(true);
      bool const backSpcUnindents = SciCall_GetBackSpaceUnIndents();
      SciCall_SetBackSpaceUnIndents(true);

      EditIndentBlock(Globals.hwndEdit, SCI_TAB, true, true);
      EditIndentBlock(Globals.hwndEdit, SCI_BACKTAB, true, true);

      SciCall_SetUseTabs(useTabs);
      SciCall_SetTabIndents(tabIndents);
      SciCall_SetBackSpaceUnIndents(backSpcUnindents);
    }
    else {
      status->iGlobalIndent = I_MIX_LN;
      return false;
    }
  }
  return true;
}


//=============================================================================
//
//  FileLoad()
//
//
bool FileLoad(bool bDontSave, bool bNew, bool bReload, 
              bool bSkipUnicodeDetect, bool bSkipANSICPDetection, bool bForceEncDetection, LPCWSTR lpszFile)
{
  WCHAR szFilePath[MAX_PATH] = { L'\0' };
  bool fSuccess = false;

  EditFileIOStatus fioStatus = INIT_FILEIO_STATUS;
  fioStatus.iEOLMode = Settings.DefaultEOLMode;
  fioStatus.iEncoding = CPI_ANSI_DEFAULT;

  if (!bDontSave)
  {
    if (!FileSave(false, true, false, false, Flags.bPreserveFileModTime)) {
      return false;
    }
  }

  if (!bReload) { 
    ResetEncryption(); 
  }

  if (bNew) 
  {
    if (FocusedView.HideNonMatchedLines) { EditToggleView(Globals.hwndEdit); }

    if (!s_IsThisAnElevatedRelaunch) {
      Flags.bPreserveFileModTime = DefaultFlags.bPreserveFileModTime;
    }

    StringCchCopy(Globals.CurrentFile,COUNTOF(Globals.CurrentFile),L"");
    SetDlgItemText(Globals.hwndMain,IDC_FILENAME,Globals.CurrentFile);
    SetDlgItemInt(Globals.hwndMain,IDC_REUSELOCK,GetTickCount(),false);
    if (!s_flagKeepTitleExcerpt) { StringCchCopy(s_wchTitleExcerpt, COUNTOF(s_wchTitleExcerpt), L""); }
    FileVars_GetFromData(NULL,0,&Globals.fvCurFile); // init-reset

    EditSetNewText(Globals.hwndEdit, "", 0, true);

    SciCall_SetEOLMode(Settings.DefaultEOLMode);
    Encoding_Current(Settings.DefaultEncoding);
    
    Style_SetDefaultLexer(Globals.hwndEdit);

    s_bFileReadOnly = false;
    SetSavePoint();

    UpdateToolbar();
    UpdateStatusbar(true);
    UpdateMarginWidth();

    // Terminate file watching
    if (FileWatching.ResetFileWatching) {
      if (FileWatching.MonitoringLog) {
        PostWMCommand(Globals.hwndMain, IDM_VIEW_CHASING_DOCTAIL);
      }
      FileWatching.FileWatchingMode = Settings.FileWatchingMode;
    }
    InstallFileWatching(NULL);
    Flags.bSettingsFileSoftLocked = false;
    UpdateSaveSettingsCmds();
    COND_SHOW_ZOOM_CALLTIP();

    return true;
  }

  if (StrIsEmpty(lpszFile)) {
    if (!OpenFileDlg(Globals.hwndMain, szFilePath, COUNTOF(szFilePath), NULL)) {
      return false;
    }
  }
  else {
    StringCchCopy(szFilePath, COUNTOF(szFilePath), lpszFile);
  }
  NormalizePathEx(szFilePath, COUNTOF(szFilePath), true, Flags.bSearchPathIfRelative);

  // change current directory to prevent directory lock on another path
  WCHAR szFolder[MAX_PATH];
  if (SUCCEEDED(StringCchCopy(szFolder,COUNTOF(szFolder), szFilePath))) {
    if (SUCCEEDED(PathCchRemoveFileSpec(szFolder,COUNTOF(szFolder)))) {
      SetCurrentDirectory(szFolder);
    }
  }
 
  // Ask to create a new file...
  if (!bReload && !PathIsExistingFile(szFilePath))
  {
    bool bCreateFile = s_flagQuietCreate;
    if (!bCreateFile) {
      INT_PTR const answer = InfoBoxLng(MB_YESNO | MB_ICONQUESTION, NULL, IDS_MUI_ASK_CREATE, PathFindFileName(szFilePath));
      if ((IDOK == answer) || (IDYES == answer)) {
        bCreateFile = true;
      }
    }
    if (bCreateFile) {
      HANDLE hFile = CreateFile(szFilePath,
                      GENERIC_READ | GENERIC_WRITE,
                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                      NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
      Globals.dwLastError = GetLastError();
      fSuccess = (hFile != INVALID_HANDLE_VALUE);
      if (fSuccess) {
        FileVars_GetFromData(NULL,0,&Globals.fvCurFile); // init/reset
        EditSetNewText(Globals.hwndEdit,"",0, true);
        Style_SetDefaultLexer(Globals.hwndEdit);
        SciCall_SetEOLMode(Settings.DefaultEOLMode);
        if (Encoding_IsValid(Encoding_Forced(CPI_GET))) {
          fioStatus.iEncoding = Encoding_Forced(CPI_GET);
          Encoding_Current(fioStatus.iEncoding);
        }
        else {
          fioStatus.iEncoding = Globals.fvCurFile.iEncoding;
          Encoding_Current(Globals.fvCurFile.iEncoding);
        }
        s_bFileReadOnly = false;
      }
      if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE)) {
        CloseHandle(hFile);
      }
    }
    else
      return false;
  }
  else {
    int idx;
    if (!bReload && MRU_FindFile(Globals.pFileMRU,szFilePath,&idx)) {
      fioStatus.iEncoding = Globals.pFileMRU->iEncoding[idx];
      if (Encoding_IsValid(fioStatus.iEncoding)) {
        Encoding_SrcWeak(fioStatus.iEncoding);
      }
    }
    else {
      fioStatus.iEncoding = Encoding_GetCurrent();
    }
    if (bReload && !FileWatching.MonitoringLog) 
    {
      Sci_GotoPosChooseCaret(0);

      _BEGIN_UNDO_ACTION_;
      fSuccess = FileIO(true, szFilePath, bSkipUnicodeDetect, bSkipANSICPDetection, bForceEncDetection, !bReload , &fioStatus, false, false);
      _END_UNDO_ACTION_;
    }
    else {
      fSuccess = FileIO(true, szFilePath, bSkipUnicodeDetect, bSkipANSICPDetection, bForceEncDetection, !s_IsThisAnElevatedRelaunch, &fioStatus, false, false);
    }
  }

  bool bUnknownLexer = s_flagLexerSpecified;

  if (fSuccess)
  {
    BeginWaitCursor(true, L"Styling...");

    Sci_GotoPosChooseCaret(0);

    if (!s_IsThisAnElevatedRelaunch) {
      Flags.bPreserveFileModTime = DefaultFlags.bPreserveFileModTime;
    }

    StringCchCopy(Globals.CurrentFile,COUNTOF(Globals.CurrentFile),szFilePath);
    SetDlgItemText(Globals.hwndMain,IDC_FILENAME,Globals.CurrentFile);
    SetDlgItemInt(Globals.hwndMain,IDC_REUSELOCK,GetTickCount(),false);

    if (!s_flagKeepTitleExcerpt) {
      StringCchCopy(s_wchTitleExcerpt, COUNTOF(s_wchTitleExcerpt), L"");
    }
    
    if (!s_flagLexerSpecified) { // flagLexerSpecified will be cleared
      bUnknownLexer = !Style_SetLexerFromFile(Globals.hwndEdit, Globals.CurrentFile);
    }
    SciCall_SetEOLMode(fioStatus.iEOLMode);
    Encoding_Current(fioStatus.iEncoding); // load may change encoding

    int idx = 0;
    DocPos iCaretPos = -1;
    DocPos iAnchorPos = -1;
    LPCWSTR pszBookMarks = L"";
    if (!bReload && MRU_FindFile(Globals.pFileMRU,szFilePath,&idx)) {
      iCaretPos = Globals.pFileMRU->iCaretPos[idx];
      iAnchorPos = Globals.pFileMRU->iSelAnchPos[idx];
      pszBookMarks = Globals.pFileMRU->pszBookMarks[idx];
    }
    if (!(Flags.bDoRelaunchElevated || s_IsThisAnElevatedRelaunch))
    {
      MRU_AddFile(Globals.pFileMRU, szFilePath, Flags.RelativeFileMRU, Flags.PortableMyDocs, fioStatus.iEncoding, iCaretPos, iAnchorPos, pszBookMarks);
    }

    EditSetBookmarkList(Globals.hwndEdit, pszBookMarks);
    if (IsFindPatternEmpty()) {
      SetFindPattern((Globals.pMRUfind ? Globals.pMRUfind->pszItems[0] : L""));
    }

    // Install watching of the current file
    if (!bReload && FileWatching.ResetFileWatching) {
      if (FileWatching.MonitoringLog) {
        PostWMCommand(Globals.hwndMain, IDM_VIEW_CHASING_DOCTAIL);
      }
      FileWatching.FileWatchingMode = Settings.FileWatchingMode;
    }
    InstallFileWatching(Globals.CurrentFile);

    // the .LOG feature ...
    if (SciCall_GetTextLength() >= 4) {
      char tchLog[5] = { '\0','\0','\0','\0','\0' };
      SciCall_GetText(COUNTOF(tchLog), tchLog);
      if (StringCchCompareXA(tchLog,".LOG") == 0) {
        SciCall_DocumentEnd();
        _BEGIN_UNDO_ACTION_;
        SciCall_NewLine();
        SendWMCommand(Globals.hwndMain, IDM_EDIT_INSERT_SHORTDATE);
        SciCall_DocumentEnd();
        SciCall_NewLine();
        _END_UNDO_ACTION_;
        SciCall_ScrollToEnd();
      }
    }

    // set historic caret/selection  pos
    if ((iCaretPos >= 0) && (iAnchorPos >= 0) && (SciCall_GetCurrentPos() == 0))
    {
      EditSetSelectionEx(iAnchorPos, iCaretPos, -1, -1);
    }

    SetSavePoint();

    // consistent settings file handling (if loaded in editor)
    Flags.bSettingsFileSoftLocked = (StringCchCompareXIW(Globals.CurrentFile, Globals.IniFile) == 0);
    UpdateSaveSettingsCmds();
    COND_SHOW_ZOOM_CALLTIP();

    // Show warning: Unicode file loaded as ANSI
    if (fioStatus.bUnicodeErr) {
      InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_ERR_UNICODE);
    }

    // Show inconsistent line endings warning
    Globals.bDocHasInconsistentEOLs = fioStatus.bInconsistentEOLs;

    bool const bCheckFile = !Globals.CmdLnFlag_PrintFileAndLeave
      && !fioStatus.bEncryptedRaw
      && !(fioStatus.bUnknownExt && bUnknownLexer)
      && !bReload;
    //&& (fioStatus.iEncoding == CPI_ANSI_DEFAULT) ???

    bool const bCheckEOL = bCheckFile && Globals.bDocHasInconsistentEOLs && Settings.WarnInconsistEOLs;

    if (bCheckEOL && !Style_MaybeBinaryFile(Globals.hwndEdit, szFilePath))
    {
      if (WarnLineEndingDlg(Globals.hwndMain, &fioStatus)) {
        SciCall_ConvertEOLs(fioStatus.iEOLMode);
        Globals.bDocHasInconsistentEOLs = false;
      }
      SciCall_SetEOLMode(fioStatus.iEOLMode);
    }


    // Show inconsistent indentation 
    fioStatus.iGlobalIndent = I_MIX_LN; // init

    bool const bCheckIndent = bCheckFile && !Flags.bLargeFileLoaded && Settings.WarnInconsistentIndents;

    if (bCheckIndent && !Style_MaybeBinaryFile(Globals.hwndEdit, szFilePath))
    {
      EditIndentationStatistic(Globals.hwndEdit, &fioStatus);
      ConsistentIndentationCheck(&fioStatus);
    }

    if (Settings.AutoDetectIndentSettings && !Globals.CmdLnFlag_PrintFileAndLeave)
    {
      if (!Settings.WarnInconsistentIndents || (fioStatus.iGlobalIndent != I_MIX_LN))
      {
        EditIndentationStatistic(Globals.hwndMain, &fioStatus);  // new statistic needed
      }
      Globals.fvCurFile.bTabsAsSpaces = (fioStatus.indentCount[I_TAB_LN] < fioStatus.indentCount[I_SPC_LN]) ? true : false;
      SciCall_SetUseTabs(!Globals.fvCurFile.bTabsAsSpaces);
    }
  
    EndWaitCursor();
  }
  else if (!(Flags.bLargeFileLoaded || fioStatus.bUnknownExt)) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_ERR_LOADFILE, PathFindFileName(szFilePath));
  }

  UpdateToolbar();
  UpdateStatusbar(true);
  UpdateMarginWidth();

  return fSuccess;
}



//=============================================================================
//
//  FileRevert()
//
//
bool FileRevert(LPCWSTR szFileName, bool bIgnoreCmdLnEnc)
{
  if (StrIsEmpty(szFileName)) { return false; }

  bool bPreserveView = true;
  DocLn const curLineNum = Sci_GetCurrentLineNumber();
  bool const bIsAtDocEnd = (curLineNum >= (Sci_GetLastDocLineNumber() - Settings2.CurrentLineVerticalSlop));

  Encoding_SrcWeak(CPI_NONE);
  if (bIgnoreCmdLnEnc) {
    Encoding_Forced(CPI_NONE);  // ignore history too
  }
  else {
    Encoding_SrcWeak(Encoding_GetCurrent());
  }

  WCHAR tchFileName2[MAX_PATH] = { L'\0' };
  StringCchCopyW(tchFileName2, COUNTOF(tchFileName2), szFileName);

  if (!FileLoad(true, false, true, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, false, tchFileName2)) { 
    return false; 
  }

  if (FileWatching.FileWatchingMode == FWM_AUTORELOAD) {
    if (bIsAtDocEnd || FileWatching.MonitoringLog) {
      bPreserveView = false;
      SciCall_DocumentEnd();
      EditEnsureSelectionVisible();
    }
  }

  if (SciCall_GetTextLength() >= 4) {
    char tch[5] = { '\0','\0','\0','\0','\0' };
    SciCall_GetText(COUNTOF(tch), tch);
    if (StringCchCompareXA(tch, ".LOG") == 0) {
      SciCall_ClearSelections();
      bPreserveView = false;
      SciCall_DocumentEnd();
      EditEnsureSelectionVisible();
    }
  }

  SetSavePoint();

  UpdateToolbar();
  UpdateStatusbar(true);
  UpdateMarginWidth();

  if (bPreserveView) {
    EditJumpTo(curLineNum + 1, 0);
  }

  return true;
}


//=============================================================================
//
//  DoElevatedRelaunch()
//
//
bool DoElevatedRelaunch(EditFileIOStatus* pFioStatus, bool bAutoSaveOnRelaunch)
{
  SaveAllSettings(false);

  Flags.bDoRelaunchElevated = true;

  LPWSTR lpCmdLine = GetCommandLine();
  size_t const wlen = StringCchLen(lpCmdLine, 0) + 2;
  LPWSTR lpExe = AllocMem(sizeof(WCHAR) * wlen, HEAP_ZERO_MEMORY);
  LPWSTR lpArgs = AllocMem(sizeof(WCHAR) * wlen, HEAP_ZERO_MEMORY);

  // ~ don't use original argument list (try to reconstruct current state as close as possible
#if 0
  ExtractFirstArgument(lpCmdLine, lpExe, lpArgs, (int)wlen);

  // remove relaunch elevated, we are doing this here already
  lpArgs[StringCchLen(lpArgs, 0)] = L' '; // add a space
  lpArgs = StrCutI(lpArgs, L"/u ");
  lpArgs = StrCutI(lpArgs, L"-u ");

  // remove forced command line encoding from argument list
  WCHAR wchEncoding[80] = { L'\0' };
  wchEncoding[0] = L'/';
  Encoding_GetNameW(Encoding_Forced(CPI_GET), &wchEncoding[1], COUNTOF(wchEncoding)-1);
  if (StrIsNotEmpty(&wchEncoding[1])) {
    lpArgs = StrCutI(lpArgs, wchEncoding);
  }

  // remove file from argument list
  if (s_lpOrigFileArg) {
    lpArgs = StrCutI(lpArgs, s_lpOrigFileArg);
  }
#else
  lpArgs[0] = L'\0';
#endif

  // ----------------------------------------------

  WCHAR wchFlags[32] = { L'\0' };
  if (s_flagAppIsClosing) { 
    StringCchCat(wchFlags, COUNTOF(wchFlags), L"/UC "); 
  }
  if (bAutoSaveOnRelaunch) {
    StringCchCat(wchFlags, COUNTOF(wchFlags), L"/QS ");
  }
  if (Flags.bPreserveFileModTime) {
    StringCchCat(wchFlags, COUNTOF(wchFlags), L"/RP ");
  }

  DocPos const iCurPos = SciCall_GetCurrentPos();
  int const iCurLn = (int)SciCall_LineFromPosition(iCurPos) + 1;
  int const iCurCol = (int)SciCall_GetColumn(iCurPos) + 1;
  WININFO const wi = GetMyWindowPlacement(Globals.hwndMain, NULL);


  WCHAR szArguments[2048] = { L'\0' };
  StringCchPrintf(szArguments, COUNTOF(szArguments),
    L"%s/pos %i,%i,%i,%i,%i /g %i,%i %s", wchFlags, wi.x, wi.y, wi.cx, wi.cy, wi.max, iCurLn, iCurCol, lpArgs);

  WCHAR lpTempPathBuffer[MAX_PATH] = { L'\0' };
  WCHAR szTempFileName[MAX_PATH] = { L'\0' };

  const WCHAR* szCurFile = StrIsNotEmpty(Globals.CurrentFile) ? Globals.CurrentFile : L".\\Untitled.txt";

  WCHAR tchBase[MAX_PATH] = { L'\0' };
  StringCchCopy(tchBase, COUNTOF(tchBase), PathFindFileName(szCurFile));  // eq. PathStripPath(tchBase);


  if (GetTempPath(MAX_PATH, lpTempPathBuffer) && GetTempFileName(lpTempPathBuffer, TEXT("NP3"), 0, szTempFileName))
  {
    size_t const len = StringCchLen(szTempFileName, MAX_PATH); // replace possible unknown extension
      LPWSTR p = PathFindExtension(szTempFileName);
      LPCWSTR q = PathFindExtension(szCurFile);
    if ((p && *p) && (q && *q)) {
      StringCchCopy(p, (MAX_PATH - len), q);
    }

    if (pFioStatus && FileIO(false, szTempFileName, true, true, false, true, pFioStatus, true, false))
    {
      // preserve encoding
      WCHAR wchEncoding[80] = { L'\0' };
      Encoding_GetNameW(Encoding_GetCurrent(), wchEncoding, COUNTOF(wchEncoding));

      StringCchPrintf(szArguments, COUNTOF(szArguments),
        L"%s/%s /pos %i,%i,%i,%i,%i /g %i,%i /%s\"%s\" %s", 
        wchFlags, wchEncoding, wi.x, wi.y, wi.cx, wi.cy, wi.max, iCurLn, iCurCol, RELAUNCH_ELEVATED_BUF_ARG, szTempFileName, lpArgs);

      if (!StrStrI(szArguments, tchBase)) {
        if (StrIsNotEmpty(Globals.CurrentFile)) {
          StringCchPrintf(szArguments, COUNTOF(szArguments), L"%s \"%s\"", szArguments, Globals.CurrentFile);
        }
      }
    }
    FreeMem(lpExe);
    FreeMem(lpArgs);
  }

  if (RelaunchElevated(szArguments)) {
    // set no change and quit
    SetSavePoint();
  }
  else {
    Globals.dwLastError = GetLastError();
    if (PathFileExists(szTempFileName)) {
      DeleteFile(szTempFileName);
    }
    Flags.bDoRelaunchElevated = false;
  }

  return Flags.bDoRelaunchElevated;
}


//=============================================================================
//
//  FileSave()
//
//
bool FileSave(bool bSaveAlways, bool bAsk, bool bSaveAs, bool bSaveCopy, bool bPreserveTimeStamp)
{
  bool fSuccess = false;

  EditFileIOStatus fioStatus = INIT_FILEIO_STATUS;
  fioStatus.iEncoding        = Encoding_GetCurrent();
  fioStatus.iEOLMode         = SciCall_GetEOLMode();

#if 0
  bool bIsEmptyNewFile = false;
  if (StrIsEmpty(Globals.CurrentFile)) {
    DocPos const cchText = SciCall_GetTextLength();
    if (cchText <= 0) {
      bIsEmptyNewFile = true;
    }
    else if (cchText < 2048) {
      char chTextBuf[2048] = { '\0' };
      SciCall_GetText(COUNTOF(chTextBuf), chTextBuf);
      StrTrimA(chTextBuf, " \t\n\r");
      if (StrIsEmptyA(chTextBuf)) {
        bIsEmptyNewFile = true;
      }
    }
  }
#else
  bool const bIsEmptyNewFile = (StrIsEmpty(Globals.CurrentFile) && (SciCall_GetTextLength() <= 0LL));
#endif


  if (!bSaveAlways && (!GetDocModified() || bIsEmptyNewFile) && !bSaveAs) {
    int idx;
    if (MRU_FindFile(Globals.pFileMRU, Globals.CurrentFile, &idx)) {
      Globals.pFileMRU->iEncoding[idx]   = Encoding_GetCurrent();
      Globals.pFileMRU->iCaretPos[idx]   = (Settings.PreserveCaretPos) ? SciCall_GetCurrentPos() : -1;
      Globals.pFileMRU->iSelAnchPos[idx] = (Settings.PreserveCaretPos) ? SciCall_GetAnchor() : -1;
      WCHAR wchBookMarks[MRU_BMRK_SIZE]  = {L'\0'};
      EditGetBookmarkList(Globals.hwndEdit, wchBookMarks, COUNTOF(wchBookMarks));
      if (Globals.pFileMRU->pszBookMarks[idx]) {
        LocalFree(Globals.pFileMRU->pszBookMarks[idx]);  // StrDup()
      }
      Globals.pFileMRU->pszBookMarks[idx] = StrDup(wchBookMarks);
    }
    return true;
  }

  if (bAsk)
  {
    // File or "Untitled" ...
    WCHAR tch[MAX_PATH] = { L'\0' };
    if (StrIsNotEmpty(Globals.CurrentFile)) {
      StringCchCopy(tch, COUNTOF(tch), PathFindFileName(Globals.CurrentFile));  // eq. PathStripPath(tch);
    }
    else {
      GetLngString(IDS_MUI_UNTITLED, tch, COUNTOF(tch));
    }

    INT_PTR const answer = (Settings.MuteMessageBeep) ? 
                           InfoBoxLng(MB_YESNOCANCEL | MB_ICONWARNING, NULL, IDS_MUI_ASK_SAVE, tch) :
                           MessageBoxLng(MB_YESNOCANCEL | MB_ICONWARNING, IDS_MUI_ASK_SAVE, tch);
    switch (answer)
    //switch ()
    {
    case IDCANCEL:
      return false;
    case IDNO:
      return true;
    default:
      // proceed
      break;
    }
  }

  // Read only...
  if (!bSaveAs && !bSaveCopy && StrIsNotEmpty(Globals.CurrentFile))
  {
    s_bFileReadOnly = IsReadOnly(GetFileAttributes(Globals.CurrentFile));
    if (s_bFileReadOnly) {
      INT_PTR const answer = (Settings.MuteMessageBeep) ? 
                             InfoBoxLng(MB_YESNO | MB_ICONWARNING, NULL, IDS_MUI_READONLY_SAVE, PathFindFileName(Globals.CurrentFile)) : 
                             MessageBoxLng(MB_YESNO | MB_ICONWARNING, IDS_MUI_READONLY_SAVE, Globals.CurrentFile);
      if ((IDOK == answer) || (IDYES == answer)) {
        bSaveAs = true;
      }
      else {
        return false;
      }
    }
  }

  // Save As...
  if (bSaveAs || bSaveCopy || StrIsEmpty(Globals.CurrentFile))
  {
    WCHAR tchFile[MAX_PATH] = { L'\0' };
    WCHAR tchInitialDir[MAX_PATH] = { L'\0' };
    if (bSaveCopy && StrIsNotEmpty(s_tchLastSaveCopyDir)) {
      StringCchCopy(tchInitialDir, COUNTOF(tchInitialDir), s_tchLastSaveCopyDir);
      StringCchCopy(tchFile, COUNTOF(tchFile), s_tchLastSaveCopyDir);
      PathCchAppend(tchFile, COUNTOF(tchFile), PathFindFileName(Globals.CurrentFile));
    }
    else {
      StringCchCopy(tchFile, COUNTOF(tchFile), Globals.CurrentFile);
    }

    if (SaveFileDlg(Globals.hwndMain, tchFile, COUNTOF(tchFile), tchInitialDir))
    {
      fSuccess = FileIO(false, tchFile, true, true, false, true, &fioStatus, bSaveCopy, bPreserveTimeStamp);
      if (fSuccess)
      {
        if (!bSaveCopy)
        {
          StringCchCopy(Globals.CurrentFile, COUNTOF(Globals.CurrentFile), tchFile);
          SetDlgItemText(Globals.hwndMain, IDC_FILENAME, Globals.CurrentFile);
          SetDlgItemInt(Globals.hwndMain, IDC_REUSELOCK, GetTickCount(), false);
          if (!s_flagKeepTitleExcerpt) {
            StringCchCopy(s_wchTitleExcerpt, COUNTOF(s_wchTitleExcerpt), L"");
          }
          Style_SetLexerFromFile(Globals.hwndEdit, Globals.CurrentFile);
        }
        else {
          StringCchCopy(s_tchLastSaveCopyDir, COUNTOF(s_tchLastSaveCopyDir), tchFile);
          PathCchRemoveFileSpec(s_tchLastSaveCopyDir, COUNTOF(s_tchLastSaveCopyDir));
        }
      }
    }
    else {
      return false;
    }
  }
  else {
    fSuccess = FileIO(false, Globals.CurrentFile, true, true, false, true, &fioStatus, false, bPreserveTimeStamp);
  }

  if (fSuccess)
  {
    if (!(bSaveCopy || Flags.bDoRelaunchElevated))
    {
      cpi_enc_t iCurrEnc = Encoding_GetCurrent();
      const DocPos iCaretPos = SciCall_GetCurrentPos();
      const DocPos iAnchorPos = Sci_IsMultiOrRectangleSelection() ? -1 : SciCall_GetAnchor();
      WCHAR wchBookMarks[MRU_BMRK_SIZE] = { L'\0' };
      EditGetBookmarkList(Globals.hwndEdit, wchBookMarks, COUNTOF(wchBookMarks));
      MRU_AddFile(Globals.pFileMRU, Globals.CurrentFile, Flags.RelativeFileMRU, Flags.PortableMyDocs, iCurrEnc, iCaretPos, iAnchorPos, wchBookMarks);

      SetSavePoint();

      // Install watching of the current file
      if (bSaveAs && Settings.ResetFileWatching) {
        if (FileWatching.MonitoringLog) {
          PostWMCommand(Globals.hwndMain, IDM_VIEW_CHASING_DOCTAIL);
        }
        FileWatching.FileWatchingMode = Settings.FileWatchingMode;
      }
      InstallFileWatching(Globals.CurrentFile);
    }

    // if current file is settings/config file: ask to start
    if (Flags.bSettingsFileSoftLocked && !s_flagAppIsClosing)
    {
      //~ LoadSettings(); NOT all settings will be applied ...
      INT_PTR answer = 0;
      if (Settings.SaveSettings) {
        WCHAR tch[256] = { L'\0' };
        LoadLngStringW(IDS_MUI_RELOADCFGSEX, tch, COUNTOF(tch));
        answer = InfoBoxLng(MB_YESNO | MB_ICONWARNING, L"ReloadExSavedCfg", IDS_MUI_RELOADSETTINGS, tch);
      }
      else {
        answer = InfoBoxLng(MB_YESNO | MB_ICONINFORMATION, L"ReloadExSavedCfg", IDS_MUI_RELOADSETTINGS, L"");
      }
      if ((IDOK == answer) || (IDYES == answer))
      {
        DialogNewWindow(Globals.hwndMain, false, Globals.CurrentFile);
        CloseApplication();
      }
    }

  }
  else if (!fioStatus.bCancelDataLoss)
  {
    LPCWSTR const currentFileName = PathFindFileName(Globals.CurrentFile);

    if (!s_bIsProcessElevated && (Globals.dwLastError == ERROR_ACCESS_DENIED))
    {
      INT_PTR const answer = (Settings.MuteMessageBeep) ?
                             InfoBoxLng(MB_YESNO | MB_ICONSHIELD, NULL, IDS_MUI_ERR_ACCESSDENIED, currentFileName, _W(SAPPNAME)) : 
                             MessageBoxLng(MB_YESNO | MB_ICONSHIELD, IDS_MUI_ERR_ACCESSDENIED, Globals.CurrentFile, _W(SAPPNAME));
      if ((IDOK == answer) || (IDYES == answer)) 
      {
        if (DoElevatedRelaunch(&fioStatus, true))
        {
          CloseApplication();
        }
        else {
          s_flagAppIsClosing = false;
          if (Settings.MuteMessageBeep) {
            InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_ERR_SAVEFILE, currentFileName);
          }
          else {
            MessageBoxLng(MB_ICONWARNING, IDS_MUI_ERR_SAVEFILE, Globals.CurrentFile);
          }
        }
      }
    }
    else {
      if (Settings.MuteMessageBeep) {
        InfoBoxLng(MB_ICONERROR, NULL, IDS_MUI_ERR_SAVEFILE, currentFileName);
      }
      else {
        MessageBoxLng(MB_ICONERROR, IDS_MUI_ERR_SAVEFILE, Globals.CurrentFile);
      }
    }
  }
  return fSuccess;
}


//=============================================================================
//
//  OpenFileDlg()
//
//
static WCHAR s_szFilter[NUMLEXERS * AVG_NUM_OF_STYLES_PER_LEXER * 100];

bool OpenFileDlg(HWND hwnd,LPWSTR lpstrFile,int cchFile,LPCWSTR lpstrInitialDir)
{
  OPENFILENAME ofn;
  WCHAR szFile[MAX_PATH] = { L'\0' };
  WCHAR tchInitialDir[MAX_PATH] = { L'\0' };

  Style_GetOpenDlgFilterStr(s_szFilter,COUNTOF(s_szFilter));

  if (!lpstrInitialDir) {
    if (StrIsNotEmpty(Globals.CurrentFile)) {
      StringCchCopy(tchInitialDir,COUNTOF(tchInitialDir),Globals.CurrentFile);
      PathCchRemoveFileSpec(tchInitialDir, COUNTOF(tchInitialDir));
    }
    else if (StrIsNotEmpty(Settings2.DefaultDirectory)) {
      ExpandEnvironmentStrings(Settings2.DefaultDirectory,tchInitialDir,COUNTOF(tchInitialDir));
      if (PathIsRelative(tchInitialDir)) {
        WCHAR tchModule[MAX_PATH] = { L'\0' };
        PathGetAppDirectory(tchModule, COUNTOF(tchModule));
        PathCchAppend(tchModule,COUNTOF(tchModule),tchInitialDir);
      }
    }
    else
      StringCchCopy(tchInitialDir,COUNTOF(tchInitialDir),Globals.WorkingDirectory);
  }

  ZeroMemory(&ofn,sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFilter = s_szFilter;
  ofn.lpstrFile = szFile;
  ofn.lpstrInitialDir = (lpstrInitialDir) ? lpstrInitialDir : tchInitialDir;
  ofn.nMaxFile = COUNTOF(szFile);
  ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | /* OFN_NOCHANGEDIR |*/
              OFN_DONTADDTORECENT | OFN_PATHMUSTEXIST |
              OFN_SHAREAWARE /*| OFN_NODEREFERENCELINKS*/;
  ofn.lpstrDefExt = StrIsNotEmpty(Settings2.DefaultExtension) ? Settings2.DefaultExtension : NULL;

  if (GetOpenFileName(&ofn)) {
    StringCchCopyN(lpstrFile,cchFile,szFile,COUNTOF(szFile));
    return true;
  }
  return false;
}


//=============================================================================
//
//  SaveFileDlg()
//
//
bool SaveFileDlg(HWND hwnd,LPWSTR lpstrFile,int cchFile,LPCWSTR lpstrInitialDir)
{
  OPENFILENAME ofn;
  WCHAR szNewFile[MAX_PATH] = { L'\0' };
  WCHAR tchInitialDir[MAX_PATH] = { L'\0' };

  StringCchCopy(szNewFile,COUNTOF(szNewFile),lpstrFile);
  Style_GetOpenDlgFilterStr(s_szFilter,COUNTOF(s_szFilter));

  if (StrIsNotEmpty(lpstrInitialDir)) {
    StringCchCopy(tchInitialDir, COUNTOF(tchInitialDir), lpstrInitialDir);
  }
  else if (StrIsNotEmpty(Globals.CurrentFile)) {
    StringCchCopy(tchInitialDir,COUNTOF(tchInitialDir),Globals.CurrentFile);
    PathCchRemoveFileSpec(tchInitialDir, COUNTOF(tchInitialDir));
  }
  else if (StrIsNotEmpty(Settings2.DefaultDirectory)) {
    ExpandEnvironmentStrings(Settings2.DefaultDirectory,tchInitialDir,COUNTOF(tchInitialDir));
    if (PathIsRelative(tchInitialDir)) {
      WCHAR tchModule[MAX_PATH] = { L'\0' };
      PathGetAppDirectory(tchModule, COUNTOF(tchModule));
      PathCchAppend(tchModule,COUNTOF(tchModule),tchInitialDir);
    }
  }
  else {
    StringCchCopy(tchInitialDir, COUNTOF(tchInitialDir), Globals.WorkingDirectory);
  }
  ZeroMemory(&ofn,sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFilter = s_szFilter;
  ofn.lpstrFile = szNewFile;
  ofn.lpstrInitialDir = tchInitialDir;
  ofn.nMaxFile = MAX_PATH;
  ofn.Flags = OFN_HIDEREADONLY /*| OFN_NOCHANGEDIR*/ |
            /*OFN_NODEREFERENCELINKS |*/ OFN_OVERWRITEPROMPT |
            OFN_DONTADDTORECENT | OFN_PATHMUSTEXIST;
  ofn.lpstrDefExt = StrIsNotEmpty(Settings2.DefaultExtension) ? Settings2.DefaultExtension : NULL;

  if (GetSaveFileName(&ofn)) {
    StringCchCopyN(lpstrFile,cchFile,szNewFile,COUNTOF(szNewFile));
    return true;
  }
  return false;
}


/******************************************************************************
*
* ActivatePrevInst()
*
* Tries to find and activate an already open Notepad3 Window
*
*
******************************************************************************/
BOOL CALLBACK EnumWndProc(HWND hwnd,LPARAM lParam)
{
  BOOL bContinue = TRUE;
  WCHAR szClassName[64] = { L'\0' };

  if (GetClassName(hwnd,szClassName,COUNTOF(szClassName)))

    if (StringCchCompareNIW(szClassName,COUNTOF(szClassName),s_wchWndClass,COUNTOF(s_wchWndClass)) == 0) {

      DWORD const dwReuseLock = GetDlgItemInt(hwnd, IDC_REUSELOCK, NULL, FALSE);
      if (GetTickCount() - dwReuseLock >= REUSEWINDOWLOCKTIMEOUT) {

        *(HWND*)lParam = hwnd;

        if (IsWindowEnabled(hwnd))
          bContinue = FALSE;
      }
    }
  return bContinue;
}

BOOL CALLBACK EnumWndProc2(HWND hwnd,LPARAM lParam)
{
  BOOL bContinue = TRUE;
  WCHAR szClassName[64] = { L'\0' };

  if (GetClassName(hwnd,szClassName,COUNTOF(szClassName)))

    if (StringCchCompareNIW(szClassName,COUNTOF(szClassName),s_wchWndClass,COUNTOF(s_wchWndClass)) == 0) {

      DWORD const dwReuseLock = GetDlgItemInt(hwnd,IDC_REUSELOCK,NULL, FALSE);
      if (GetTickCount() - dwReuseLock >= REUSEWINDOWLOCKTIMEOUT) 
      {
        if (IsWindowEnabled(hwnd)) { bContinue = FALSE; }

        WCHAR tchFileName[MAX_PATH] = { L'\0' };
        GetDlgItemText(hwnd, IDC_FILENAME, tchFileName, COUNTOF(tchFileName));
        
        if (StringCchCompareXI(tchFileName, s_lpFileArg) == 0) {
          *(HWND*)lParam = hwnd;
        }
        else {
          bContinue = TRUE;
        }
      }
    }
  return bContinue;
}


bool ActivatePrevInst()
{
  HWND hwnd = NULL;
  COPYDATASTRUCT cds;

  if ((!Flags.bReuseWindow && !Flags.bSingleFileInstance) || s_flagStartAsTrayIcon || s_flagNewFromClipboard || s_flagPasteBoard) {
    return false;
  }

  if (Flags.bSingleFileInstance && StrIsNotEmpty(s_lpFileArg))
  {
    NormalizePathEx(s_lpFileArg, COUNTOF(s_lpFileArg), true, Flags.bSearchPathIfRelative);

    EnumWindows(EnumWndProc2,(LPARAM)&hwnd);

    if (hwnd != NULL)
    {
      // Enabled
      if (IsWindowEnabled(hwnd))
      {
        // Make sure the previous window won't pop up a change notification message
        //SendMessage(hwnd,WM_CHANGENOTIFYCLEAR,0,0);

        if (IsIconic(hwnd)) {
          ShowWindowAsync(hwnd, SW_RESTORE);
        }
        if (!IsWindowVisible(hwnd)) {
          SendMessage(hwnd,WM_TRAYMESSAGE,0,WM_LBUTTONDBLCLK);
          SendMessage(hwnd,WM_TRAYMESSAGE,0,WM_LBUTTONUP);
        }

        SetForegroundWindow(hwnd);

        size_t cb = sizeof(np3params);
        if (s_lpSchemeArg) {
          cb += ((StringCchLen(s_lpSchemeArg, 0) + 1) * sizeof(WCHAR));
        }

        if (!IsFindPatternEmpty()) {
          cb += ((LengthOfFindPattern() + 1) * sizeof(WCHAR));
        }
        LPnp3params params = AllocMem(cb, HEAP_ZERO_MEMORY);
        params->flagFileSpecified = false;
        params->flagChangeNotify = FWM_DONT_CARE;
        params->flagQuietCreate = false;
        params->flagLexerSpecified = s_flagLexerSpecified ? 1 : 0;
        if (s_flagLexerSpecified && s_lpSchemeArg) {
          StringCchCopy(StrEnd(&params->wchData,0)+1,(StringCchLen(s_lpSchemeArg,0)+1),s_lpSchemeArg);
          params->iInitialLexer = -1;
        }
        else {
          params->iInitialLexer = s_iInitialLexer;
        }
        params->flagJumpTo = s_flagJumpTo ? 1 : 0;
        params->iInitialLine = s_iInitialLine;
        params->iInitialColumn = s_iInitialColumn;

        params->flagSetEncoding = s_flagSetEncoding;
        params->flagSetEOLMode = s_flagSetEOLMode;
        params->flagTitleExcerpt = 0;

        params->flagMatchText = g_flagMatchText;
        if (!IsFindPatternEmpty()) {
          StringCchCopy(StrEnd(&params->wchData, 0) + 1, (LengthOfFindPattern() + 1), GetFindPattern());
        }

        cds.dwData = DATA_NOTEPAD3_PARAMS;
        cds.cbData = (DWORD)SizeOfMem(params);
        cds.lpData = params;

        SendMessage(hwnd,WM_COPYDATA,(WPARAM)NULL,(LPARAM)&cds);
        FreeMem(params);    params = NULL;

        return true;
      }
      // IsWindowEnabled()
      INT_PTR const answer = InfoBoxLng(MB_YESNO | MB_ICONWARNING, NULL, IDS_MUI_ERR_PREVWINDISABLED);
      if ((IDOK == answer) || (IDYES == answer)) {
        return false;
      }
      return true;
    }
  }

  if (!Flags.bReuseWindow) {
    return false;
  }

  hwnd = NULL;
  EnumWindows(EnumWndProc,(LPARAM)&hwnd);

  // Found a window
  if (hwnd != NULL)
  {
    // Enabled
    if (IsWindowEnabled(hwnd))
    {
      // Make sure the previous window won't pop up a change notification message
      //SendMessage(hwnd,WM_CHANGENOTIFYCLEAR,0,0);

      if (IsIconic(hwnd)) {
        ShowWindowAsync(hwnd, SW_RESTORE);
      }
      if (!IsWindowVisible(hwnd)) {
        SendMessage(hwnd,WM_TRAYMESSAGE,0,WM_LBUTTONDBLCLK);
        SendMessage(hwnd,WM_TRAYMESSAGE,0,WM_LBUTTONUP);
      }

      SetForegroundWindow(hwnd);

      if (StrIsNotEmpty(s_lpFileArg))
      {
        size_t cb = sizeof(np3params);
        cb += (StringCchLenW(s_lpFileArg,0) + 1) * sizeof(WCHAR);

        if (s_lpSchemeArg) {
          cb += (StringCchLenW(s_lpSchemeArg, 0) + 1) * sizeof(WCHAR);
        }
        size_t cchTitleExcerpt = StringCchLenW(s_wchTitleExcerpt,COUNTOF(s_wchTitleExcerpt));
        if (cchTitleExcerpt) {
          cb += (cchTitleExcerpt + 1) * sizeof(WCHAR);
        }
        if (!IsFindPatternEmpty()) {
          cb += ((LengthOfFindPattern() + 1) * sizeof(WCHAR));
        }

        LPnp3params params = AllocMem(cb, HEAP_ZERO_MEMORY);
        params->flagFileSpecified = true;
        StringCchCopy(&params->wchData, StringCchLenW(s_lpFileArg,0)+1,s_lpFileArg);
        params->flagChangeNotify = s_flagChangeNotify;
        params->flagQuietCreate = s_flagQuietCreate ? 1 : 0;
        params->flagLexerSpecified = s_flagLexerSpecified ? 1 : 0;
        if (s_flagLexerSpecified && s_lpSchemeArg) {
          StringCchCopy(StrEnd(&params->wchData,0)+1, StringCchLen(s_lpSchemeArg,0)+1,s_lpSchemeArg);
          params->iInitialLexer = -1;
        }
        else {
          params->iInitialLexer = s_iInitialLexer;
        }
        params->flagJumpTo = s_flagJumpTo ? 1 : 0;
        params->iInitialLine = s_iInitialLine;
        params->iInitialColumn = s_iInitialColumn;

        params->flagSetEncoding = s_flagSetEncoding;
        params->flagSetEOLMode = s_flagSetEOLMode;

        if (cchTitleExcerpt) {
          StringCchCopy(StrEnd(&params->wchData,0)+1,cchTitleExcerpt+1,s_wchTitleExcerpt);
          params->flagTitleExcerpt = 1;
        }
        else {
          params->flagTitleExcerpt = 0;
        }

        params->flagMatchText = g_flagMatchText;
        if (!IsFindPatternEmpty()) {
          StringCchCopy(StrEnd(&params->wchData, 0) + 1, (LengthOfFindPattern() + 1), GetFindPattern());
        }

        cds.dwData = DATA_NOTEPAD3_PARAMS;
        cds.cbData = (DWORD)SizeOfMem(params);
        cds.lpData = params;

        SendMessage(hwnd,WM_COPYDATA,(WPARAM)NULL,(LPARAM)&cds);
        FreeMem(params);    params = NULL;
        s_lpFileArg[0] = L'\0';
      }
      return true;
    }
    // IsWindowEnabled()
    INT_PTR const answer = InfoBoxLng(MB_YESNO | MB_ICONWARNING, NULL, IDS_MUI_ERR_PREVWINDISABLED);
    return ((IDOK == answer) || (IDYES == answer)) ? false : true;;
  }
  return false;
}


//=============================================================================
//
//  RelaunchMultiInst()
//
//
bool RelaunchMultiInst() {

  if (Flags.MultiFileArg && (s_cFileList > 1)) 
  {
    LPWSTR lpCmdLineNew = StrDup(GetCommandLine());
    size_t len = StringCchLen(lpCmdLineNew,0) + 1UL;
    LPWSTR lp1 = AllocMem(sizeof(WCHAR)*len, HEAP_ZERO_MEMORY);
    LPWSTR lp2 = AllocMem(sizeof(WCHAR)*len, HEAP_ZERO_MEMORY);

    StrTab2Space(lpCmdLineNew);
    StringCchCopy(lpCmdLineNew + s_cchiFileList,2,L"");

    WCHAR* pwch = CharPrev(lpCmdLineNew,StrEnd(lpCmdLineNew,len));
    int k = 0;
    while (*pwch == L' ' || *pwch == L'-' || *pwch == L'+') {
      *pwch = L' ';
      pwch = CharPrev(lpCmdLineNew,pwch);
      if (k++ > 1)
        s_cchiFileList--;
    }

    for (int i = 0; i < s_cFileList; i++) 
    {
      StringCchCopy(lpCmdLineNew + s_cchiFileList,8,L" /n - ");
      StringCchCat(lpCmdLineNew,len,s_lpFileList[i]);
      LocalFree(s_lpFileList[i]); // StrDup()

      STARTUPINFO si;
      ZeroMemory(&si,sizeof(STARTUPINFO));
      si.cb = sizeof(STARTUPINFO);

      PROCESS_INFORMATION pi;
      ZeroMemory(&pi,sizeof(PROCESS_INFORMATION));

      CreateProcess(NULL, lpCmdLineNew, NULL, NULL, false, CREATE_NEW_PROCESS_GROUP, NULL, Globals.WorkingDirectory, &si, &pi);
      
      // don't wait for it to finish.
      //::WaitForSingleObject(pi.hProcess, INFINITE);
      // free up resources...
      CloseHandle(pi.hThread);
      CloseHandle(pi.hProcess);
    }

    LocalFree(lpCmdLineNew); // StrDup()
    FreeMem(lp1);
    FreeMem(lp2);
    s_lpFileArg[0] = L'\0';

    return true;
  }

  for (int i = 0; i < s_cFileList; i++) {
    LocalFree(s_lpFileList[i]); // StrDup()
  }

  return false;
}


//=============================================================================
//
//  RelaunchElevated()
//
//
bool RelaunchElevated(LPWSTR lpNewCmdLnArgs) 
{
  if (!IsWindowsVistaOrGreater() || !Flags.bDoRelaunchElevated ||
      s_bIsProcessElevated || s_IsThisAnElevatedRelaunch || s_bIsRunAsAdmin ||
      s_flagDisplayHelp) 
  {
    return false; // reject initial RelaunchElevated() try
  }

  STARTUPINFO si;
  si.cb = sizeof(STARTUPINFO);
  GetStartupInfo(&si);

  WCHAR lpExe[MAX_PATH] = { L'\0' };
  WCHAR szOrigArgs[2032] = { L'\0' };

  LPWSTR lpCmdLine = GetCommandLine();
  size_t wlen = StringCchLenW(lpCmdLine, 0) + 2UL;
  ExtractFirstArgument(lpCmdLine, lpExe, szOrigArgs, (int)wlen);
  // override
  GetModuleFileName(NULL, lpExe, COUNTOF(lpExe)); // full path
  PathCanonicalizeEx(lpExe, COUNTOF(lpExe));
  if (lpNewCmdLnArgs) {
    StringCchCopy(szOrigArgs, COUNTOF(szOrigArgs), lpNewCmdLnArgs);
  }
  size_t const len = StringCchLen(szOrigArgs, 0);
  szOrigArgs[len] = L' '; // add a space
  szOrigArgs[len+1] = L'\0'; // ensure termination
  // remove relaunch elevated, we are doing this here already
  StrCutI(szOrigArgs, L"/u ");
  StrCutI(szOrigArgs, L"-u ");

  WCHAR szArguments[2032] = { L'\0' };
  if (StrStrI(szOrigArgs, L"/f ") || StrStrI(szOrigArgs, L"-f ") || StrIsEmpty(Globals.IniFile))
  {
    StringCchCopy(szArguments, COUNTOF(szArguments), szOrigArgs);
  }
  else {
    StringCchPrintf(szArguments, COUNTOF(szArguments), L"/f \"%s\" %s", Globals.IniFile, szOrigArgs);
  }

  if (StrIsNotEmpty(szArguments)) {
    SHELLEXECUTEINFO sei;
    ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
    sei.cbSize = sizeof(SHELLEXECUTEINFO);
    sei.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOASYNC | SEE_MASK_UNICODE | SEE_MASK_HMONITOR | SEE_MASK_NOZONECHECKS | SEE_MASK_WAITFORINPUTIDLE;
    sei.hwnd = GetForegroundWindow();
    sei.hMonitor = MonitorFromWindow(sei.hwnd, MONITOR_DEFAULTTONEAREST);
    sei.lpVerb = L"runas";
    sei.lpFile = lpExe;
    sei.lpParameters = szArguments;
    sei.lpDirectory = Globals.WorkingDirectory;
    sei.nShow = si.wShowWindow ? si.wShowWindow : SW_SHOWNORMAL;
    return ShellExecuteEx(&sei);
  }
  return false;
}


//=============================================================================
//
//  SnapToWinInfoPos()
//  Aligns Notepad3 to the default window position on the current screen
//
void SnapToWinInfoPos(HWND hwnd, const WININFO winInfo, SCREEN_MODE mode)
{
  static bool s_bPrevShowMenubar   = true;
  static bool s_bPrevShowToolbar   = true;
  static bool s_bPrevShowStatusbar = true;
  static WINDOWPLACEMENT s_wndplPrev;
  s_wndplPrev.length = sizeof(WINDOWPLACEMENT);

  UINT const fPrevFlags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED;
  UINT const  fFScrFlags     = SWP_NOOWNERZORDER | SWP_FRAMECHANGED;
  DWORD const dwRmvFScrStyle = WS_OVERLAPPEDWINDOW | WS_BORDER;

  HWND const hWindow = hwnd ? hwnd : GetDesktopWindow();

  DWORD dwStyle = GetWindowLong(hWindow, GWL_STYLE);
  RECT rcCurrent; GetWindowRect(hWindow, &rcCurrent);

  if ((mode == SCR_NORMAL) || s_bPrevFullScreenFlag) 
  { 
    SetWindowLong(hWindow, GWL_STYLE, dwStyle | dwRmvFScrStyle);
    if (s_bPrevFullScreenFlag) {
      SetWindowPlacement(hWindow, &s_wndplPrev); // 1st set correct screen (DPI Aware)
      SetWindowPlacement(hWindow, &s_wndplPrev); // 2nd resize position to correct DPI settings
      Settings.ShowMenubar = s_bPrevShowMenubar;
      Settings.ShowToolbar = s_bPrevShowToolbar;
      Settings.ShowStatusbar = s_bPrevShowStatusbar;
    }
    else {
      WINDOWPLACEMENT wndpl = WindowPlacementFromInfo(hWindow, &winInfo, mode);
      if (GetDoAnimateMinimize()) { DrawAnimatedRects(hWindow, IDANI_CAPTION, &rcCurrent, &wndpl.rcNormalPosition); }
      SetWindowPlacement(hWindow, &wndpl); // 1st set correct screen (DPI Aware)
      SetWindowPlacement(hWindow, &wndpl); // 2nd resize position to correct DPI settings
    }
    SetWindowPos(hWindow, (Settings.AlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST), 0, 0, 0, 0, fPrevFlags);
    s_bPrevFullScreenFlag = false;
  }
  else { // full screen mode
    GetWindowPlacement(hWindow, &s_wndplPrev);
    MONITORINFO mi = { sizeof(mi) };
    GetMonitorInfo(MonitorFromWindow(hWindow, MONITOR_DEFAULTTOPRIMARY), &mi);
    SetWindowLong(hWindow, GWL_STYLE, dwStyle & ~dwRmvFScrStyle);
    WINDOWPLACEMENT wndpl = WindowPlacementFromInfo(hWindow, NULL, mode);
    if (GetDoAnimateMinimize()) { DrawAnimatedRects(hWindow, IDANI_CAPTION, &rcCurrent, &wndpl.rcNormalPosition); }
    SetWindowPlacement(hWindow, &wndpl);
    SetWindowPos(hWindow, HWND_TOPMOST, mi.rcMonitor.left, mi.rcMonitor.top,
      mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, fFScrFlags);
    s_bPrevShowMenubar = Settings.ShowMenubar;
    s_bPrevShowToolbar = Settings.ShowToolbar;
    s_bPrevShowStatusbar = Settings.ShowStatusbar;
    Settings.ShowMenubar = Settings.ShowToolbar = Settings.ShowStatusbar = false;
    s_bPrevFullScreenFlag = true;
  }

  DrawMenuBar(Globals.hwndMain);
  MsgThemeChanged(hWindow, (WPARAM)NULL, (LPARAM)NULL);
}


//=============================================================================
//
//  ShowNotifyIcon()
//
//
void ShowNotifyIcon(HWND hwnd,bool bAdd)
{
  static HICON hIcon = NULL;
  if (!hIcon) {
    LoadIconWithScaleDown(Globals.hInstance, MAKEINTRESOURCE(IDR_MAINWND), 
                          GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), &hIcon);
  }
  NOTIFYICONDATA nid;
  ZeroMemory(&nid,sizeof(NOTIFYICONDATA));
  nid.cbSize = sizeof(NOTIFYICONDATA);
  nid.hWnd = hwnd;
  nid.uID = 0;
  nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
  nid.uCallbackMessage = WM_TRAYMESSAGE;
  nid.hIcon = hIcon;
  StringCchCopy(nid.szTip,COUNTOF(nid.szTip), _W(SAPPNAME));

  if(bAdd)
    Shell_NotifyIcon(NIM_ADD,&nid);
  else
    Shell_NotifyIcon(NIM_DELETE,&nid);

}


//=============================================================================
//
//  SetNotifyIconTitle()
//
//
void SetNotifyIconTitle(HWND hwnd)
{
  NOTIFYICONDATA nid;
  ZeroMemory(&nid,sizeof(NOTIFYICONDATA));
  nid.cbSize = sizeof(NOTIFYICONDATA);
  nid.hWnd = hwnd;
  nid.uID = 0;
  nid.uFlags = NIF_TIP;

  WCHAR tchTitle[256] = { L'\0' };
  if (StrIsNotEmpty(s_wchTitleExcerpt)) {
    WCHAR tchFormat[32] = { L'\0' };
    GetLngString(IDS_MUI_TITLEEXCERPT,tchFormat,COUNTOF(tchFormat));
    StringCchPrintf(tchTitle,COUNTOF(tchTitle),tchFormat,s_wchTitleExcerpt);
  }
  else if (StrIsNotEmpty(Globals.CurrentFile)) {
    WCHAR szDisplayName[MAX_PATH];
    PathGetDisplayName(szDisplayName, COUNTOF(szDisplayName), Globals.CurrentFile);
    PathCompactPathEx(tchTitle,szDisplayName,COUNTOF(tchTitle)-4,0);
  }
  else {
    GetLngString(IDS_MUI_UNTITLED, tchTitle, COUNTOF(tchTitle) - 4);
  }
  if (GetDocModified()) {
    StringCchCopy(nid.szTip, COUNTOF(nid.szTip), L"* ");
  }
  else {
    StringCchCopy(nid.szTip, COUNTOF(nid.szTip), L"");
  }
  StringCchCat(nid.szTip,COUNTOF(nid.szTip),tchTitle);

  Shell_NotifyIcon(NIM_MODIFY,&nid);
}


//=============================================================================
//
//  UpdateMouseDWellTime()
//
void UpdateMouseDWellTime()
{
  if (Settings.ShowHypLnkToolTip || IsColorDefHotspotEnabled() || Settings.HighlightUnicodePoints)
    SciCall_SetMouseDWellTime(100);
  else
    Sci_DisableMouseDWellNotification();
}


//=============================================================================
//
//  ShowZoomCallTip()
//
void ShowZoomCallTip()
{
  int const delayClr = Settings2.ZoomTooltipTimeout;
  if (delayClr >= (10*USER_TIMER_MINIMUM)) {
    int const iZoomLevelPercent = SciCall_GetZoom();

    char chToolTip[32] = { '\0' };
    StringCchPrintfA(chToolTip, COUNTOF(chToolTip), "Zoom: %i%%", iZoomLevelPercent);

    DocPos const iPos = SciCall_PositionFromLine(SciCall_GetFirstVisibleLine());

    int const iXOff = SciCall_GetXOffset();
    SciCall_SetXOffset(0);
    SciCall_CallTipShow(iPos, chToolTip);
    SciCall_SetXOffset(iXOff);
    Globals.CallTipType = CT_ZOOM;
    _DelayClearZoomCallTip(delayClr);
  }
  else {
    CancelCallTip();
  }
}


//=============================================================================
//
//  CancelCallTip()
//
void CancelCallTip()
{
  SciCall_CallTipCancel();
  Globals.CallTipType = CT_NONE;
}


//=============================================================================
//
//  TerminateFileWatching()
//
static void TerminateFileWatching()
{
  if (s_bRunningWatch)
  {
    KillTimer(NULL, ID_WATCHTIMER);
    if (s_hChangeHandle) {
      FindCloseChangeNotification(s_hChangeHandle);
      s_hChangeHandle = NULL;
    }
    s_bRunningWatch = false;
    s_dwChangeNotifyTime = 0UL; // reset
  }
}

//=============================================================================
//
//  InstallFileWatching()
//
void InstallFileWatching(LPCWSTR lpszFile)
{
  // Terminate
  if (StrIsEmpty(lpszFile) || (FileWatching.FileWatchingMode == FWM_DONT_CARE))
  {
    TerminateFileWatching();
  }
  else  // Install
  {
    // Terminate previous watching
    TerminateFileWatching();

    WCHAR tchDirectory[MAX_PATH] = { L'\0' };
    StringCchCopy(tchDirectory,COUNTOF(tchDirectory),lpszFile);
    PathCchRemoveFileSpec(tchDirectory, COUNTOF(tchDirectory));

    // Save data of current file
    HANDLE hFind = FindFirstFile(Globals.CurrentFile, &s_fdCurFile);
    if (hFind != INVALID_HANDLE_VALUE) {
      FindClose(hFind);
    }
    else {
      ZeroMemory(&s_fdCurFile, sizeof(WIN32_FIND_DATA));
    }
    s_hChangeHandle = FindFirstChangeNotification(
      tchDirectory,false,
      FILE_NOTIFY_CHANGE_FILE_NAME  | \
      FILE_NOTIFY_CHANGE_DIR_NAME   | \
      FILE_NOTIFY_CHANGE_ATTRIBUTES | \
      FILE_NOTIFY_CHANGE_SIZE | \
      FILE_NOTIFY_CHANGE_LAST_WRITE);

    // No previous watching installed, so launch the timer first
    if (!s_bRunningWatch) {
      SetTimer(NULL, ID_WATCHTIMER, 
               min_dw(FileWatching.FileCheckInverval, FileWatching.AutoReloadTimeout), 
               WatchTimerProc);
      s_bRunningWatch = true;
    }
    s_dwChangeNotifyTime = GetTickCount(); // init
  }
  UpdateToolbar();
}


static inline bool CurrentFileChanged() 
{
  if (StrIsEmpty(Globals.CurrentFile)) { return false; }

  // Check if the file has been changed
  WIN32_FIND_DATA fdUpdated;
  ZeroMemory(&fdUpdated, sizeof(WIN32_FIND_DATA));

  //~HANDLE const hFind = FindFirstFile(Globals.CurrentFile, &fdUpdated);
  //~if (hFind != INVALID_HANDLE_VALUE)
  //~  FindClose(hFind);
  //~else
  //~  return true;

  if (!GetFileAttributesEx(Globals.CurrentFile, GetFileExInfoStandard, &fdUpdated)) {
    return true;   // The current file has been removed
  }

  bool const changed = (s_fdCurFile.nFileSizeLow != fdUpdated.nFileSizeLow)
    || (s_fdCurFile.nFileSizeHigh != fdUpdated.nFileSizeHigh)
    //|| (CompareFileTime(&s_fdCurFile.ftLastWriteTime, &fdUpdated.ftLastWriteTime) != 0)
    || (s_fdCurFile.ftLastWriteTime.dwLowDateTime != fdUpdated.ftLastWriteTime.dwLowDateTime)
    || (s_fdCurFile.ftLastWriteTime.dwHighDateTime != fdUpdated.ftLastWriteTime.dwHighDateTime);

  return changed;
}

//=============================================================================
//
//  WatchTimerProc()
//
//
void CALLBACK WatchTimerProc(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime)
{
  UNUSED(dwTime);
  UNUSED(idEvent);
  UNUSED(uMsg);
  UNUSED(hwnd);
  
  if (s_bRunningWatch)
  {
    switch (FileWatching.FileWatchingMode) 
    {
      case FWM_AUTORELOAD:
        if (((GetTickCount() - s_dwChangeNotifyTime) > FileWatching.AutoReloadTimeout)
            && //|| // TODO: OR for read only auto reload without save requester
            CurrentFileChanged()) 
        {
          TerminateFileWatching();
          PostMessage(Globals.hwndMain, WM_CHANGENOTIFY, 0, 0);
        }
        break;

      case FWM_MSGBOX:
        {
          if (s_hChangeHandle) 
          {
            while (WaitForSingleObject(s_hChangeHandle, 0) == WAIT_OBJECT_0) {
              // Check if the changes affect the current file
              if (CurrentFileChanged()) {
                // Shutdown current watching and give control to main window
                TerminateFileWatching();
                PostMessage(Globals.hwndMain, WM_CHANGENOTIFY, 0, 0);
                break; // while
              }
              FindNextChangeNotification(s_hChangeHandle);
            }
          }
        }
        break;

      case FWM_DONT_CARE:
      default:
        break;
    }
  }
}


//=============================================================================
//
//  PasteBoardTimer()
//
void CALLBACK PasteBoardTimer(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime)
{
  if ((s_dwLastCopyTime > 0) && ((GetTickCount() - s_dwLastCopyTime) > 200)) {

    if (SciCall_CanPaste()) {
      bool bAutoIndent2 = Settings.AutoIndent;
      Settings.AutoIndent = 0;
      EditJumpTo(-1,0);
      _BEGIN_UNDO_ACTION_;
      if (!Sci_IsDocEmpty()) {
        SciCall_NewLine();
      }
      SciCall_Paste();
      SciCall_NewLine();
      _END_UNDO_ACTION_;
      EditEnsureSelectionVisible();
      Settings.AutoIndent = bAutoIndent2;
    }
    s_dwLastCopyTime = 0;
  }

  UNUSED(dwTime);
  UNUSED(idEvent);
  UNUSED(uMsg);
  UNUSED(hwnd);
}


//=============================================================================
//
//  CloseNonModalDialogs()
//
void CloseNonModalDialogs()
{
  if (IsWindow(Globals.hwndDlgFindReplace)) {
    SendMessage(Globals.hwndDlgFindReplace, WM_CLOSE, 0, 0);
  }
  if (IsWindow(Globals.hwndDlgCustomizeSchemes)) {
    SendMessage(Globals.hwndDlgCustomizeSchemes, WM_CLOSE, 0, 0);
  }
}


//=============================================================================
//
//  CloseApplication()
//
void CloseApplication()
{
  CloseNonModalDialogs();
  PostMessage(Globals.hwndMain, WM_CLOSE, 0, 0);
}


///  End of Notepad3.c  ///
