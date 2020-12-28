/******************************************************************************
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
*******************************************************************************/

#include "Helpers.h"

#include <commctrl.h>
#include <uxtheme.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <commdlg.h>
#include <stdio.h>
#include <string.h>
//#include <pathcch.h>
#include <time.h>


/******************************************************************************
*
* Local and global Variables for Notepad3.c
*
*/
CONSTANTS_T Constants;

FLAGS_T     Flags;
FLAGS_T     DefaultFlags;

GLOBALS_T   Globals;
SETTINGS_T  Settings;
SETTINGS_T  Defaults;
SETTINGS2_T Settings2;
SETTINGS2_T Defaults2;

FOCUSEDVIEW_T FocusedView;
FILEWATCHING_T FileWatching;

WININFO   s_WinInfo = INIT_WININFO;
WININFO   s_DefWinInfo = INIT_WININFO;

COLORREF  g_colorCustom[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

prefix_t  s_mxSBPrefix[STATUS_SECTOR_COUNT];
prefix_t  s_mxSBPostfix[STATUS_SECTOR_COUNT];

bool      s_iStatusbarVisible[STATUS_SECTOR_COUNT] = SBS_INIT_ZERO;
int       s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT] = SBS_INIT_ZERO;
int       s_vSBSOrder[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

WCHAR     s_tchToolbarBitmap[MAX_PATH] = { L'\0' };
WCHAR     s_tchToolbarBitmapHot[MAX_PATH] = { L'\0' };
WCHAR     s_tchToolbarBitmapDisabled[MAX_PATH] = { L'\0' };

bool      s_bEnableSaveSettings = true;
int       s_iToolBarTheme = -1;
bool      s_flagPosParam = false;
int       s_flagWindowPos = 0;

int       s_flagReuseWindow = 0;
int       s_flagSingleFileInstance = 0;
int       s_flagMultiFileArg = 0;
int       s_flagShellUseSystemMRU = 0;
int       s_flagPrintFileAndLeave = 0;


// ------------------------------------

static WCHAR     s_wchWndClass[16] = _W(SAPPNAME);

static HWND      s_hwndEditFrame = NULL;
static HWND      s_hwndNextCBChain = NULL;
static HWND      s_hwndReBar = NULL;

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

static WCHAR* const _s_RecentFiles = L"Recent Files";
static WCHAR* const _s_RecentFind = L"Recent Find";
static WCHAR* const _s_RecentReplace = L"Recent Replace";

static WCHAR     s_tchLastSaveCopyDir[MAX_PATH] = { L'\0' };

static bool      s_bRunningWatch = false;
static bool      s_bFileReadOnly = false;
static bool      s_bIsElevated = false;

static int       s_iSortOptions = 0;
static int       s_iAlignMode = 0;
static bool      s_bIsAppThemed = true;
static UINT      s_msgTaskbarCreated = 0;
static bool      s_dwChangeNotifyTime = 0;
static HANDLE    s_hChangeHandle = NULL;
static WCHAR     s_wchTitleExcerpt[MIDSZ_BUFFER] = { L'\0' };
static UINT      s_uidsAppTitle = IDS_MUI_APPTITLE;
static DWORD     s_dwLastCopyTime = 0;
static bool      s_bLastCopyFromMe = false;
static bool      s_bIndicMultiEdit = false;

static int       s_iInitialLine;
static int       s_iInitialColumn;
static int       s_iInitialLexer;

static int       s_cyReBar;
static int       s_cyReBarFrame;
static int       s_cxEditFrame;
static int       s_cyEditFrame;

// for tiny expression calculation
static double    s_dExpression = 0.0;
static int       s_iExprError = -1;

static WIN32_FIND_DATA s_fdCurFile;

static HMODULE s_hRichEdit = INVALID_HANDLE_VALUE;

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
static const int NUMTOOLBITMAPS = 29;

// ----------------------------------------------------------------------------

const WCHAR* const TBBUTTON_DEFAULT_IDS_V1 = L"1 2 4 3 28 0 5 6 0 7 8 9 0 10 11 0 12 0 24 26 0 22 23 0 13 14 0 27 0 15 0 25 0 17";
const WCHAR* const TBBUTTON_DEFAULT_IDS_V2 = L"1 2 4 3 28 0 5 6 0 7 8 9 0 10 11 0 12 0 24 26 0 22 23 0 13 14 0 15 0 25 0 29 0 17";

//=============================================================================

// some Mappings internal idx -> Scintilla values

static int const s_DirectWriteTechnology[4] = {
    SC_TECHNOLOGY_DEFAULT
  , SC_TECHNOLOGY_DIRECTWRITE
  , SC_TECHNOLOGY_DIRECTWRITERETAIN
  , SC_TECHNOLOGY_DIRECTWRITEDC
};

static int const s_SciBidirectional[3] = {
  SC_BIDIRECTIONAL_DISABLED
  , SC_BIDIRECTIONAL_L2R
  , SC_BIDIRECTIONAL_R2L
};


int const g_FontQuality[4] = {
    SC_EFF_QUALITY_DEFAULT
  , SC_EFF_QUALITY_NON_ANTIALIASED
  , SC_EFF_QUALITY_ANTIALIASED
  , SC_EFF_QUALITY_LCD_OPTIMIZED
};

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
static int   _UndoRedoActionMap(int token, UndoRedoSelection_t** selection);

// ----------------------------------------------------------------------------

static void  _DelayClearZoomCallTip(int delay);

#ifdef _EXTRA_DRAG_N_DROP_HANDLER_
static CLIPFORMAT cfDrpF = CF_HDROP;
static POINTL ptDummy = { 0, 0 };
static PDROPTARGET pDropTarget = NULL;
static DWORD DropFilesProc(CLIPFORMAT cf, HGLOBAL hData, HWND hWnd, DWORD dwKeyState, POINTL pt, void *pUserData);
#endif


//#define NP3_VIRTUAL_SPACE_ACCESS_OPTIONS  (SCVS_RECTANGULARSELECTION | SCVS_NOWRAPLINESTART | SCVS_USERACCESSIBLE)
#define NP3_VIRTUAL_SPACE_ACCESS_OPTIONS  (SCVS_RECTANGULARSELECTION)

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
    UpdateVisibleHotspotIndicators();
    UpdateAllBars(false);
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

  if ((pMQC1->hwnd == pMQC2->hwnd)
       && (pMQC1->cmd == pMQC2->cmd)
       && (pMQC1->wparam == pMQC2->wparam)
       && (pMQC1->lparam == pMQC2->lparam))
  {
    return 0;
  }
  return 1;
}
// ----------------------------------------------------------------------------

#define _MQ_ms(T) ((T) / USER_TIMER_MINIMUM)

static void  _MQ_AppendCmd(CmdMessageQueue_t* const pMsgQCmd, int cycles)
{
  CmdMessageQueue_t* pmqc = NULL;
  DL_SEARCH(MessageQueue, pmqc, pMsgQCmd, msgcmp);

  if (!pmqc) { // NOT found
    pmqc = AllocMem(sizeof(CmdMessageQueue_t), HEAP_ZERO_MEMORY);
    pmqc->hwnd = pMsgQCmd->hwnd;
    pmqc->cmd = pMsgQCmd->cmd;
    pmqc->wparam = pMsgQCmd->wparam;
    pmqc->lparam = pMsgQCmd->lparam;
    pmqc->delay = cycles;
    DL_APPEND(MessageQueue, pmqc);
  }

  if (cycles < 2) {
    pmqc->delay = -1; // execute now (do not use PostMessage() here)
    SendMessage(pMsgQCmd->hwnd, pMsgQCmd->cmd, pMsgQCmd->wparam, pMsgQCmd->lparam);
  }
  else {
    pmqc->delay = (pmqc->delay + cycles) / 2; // increase delay
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


