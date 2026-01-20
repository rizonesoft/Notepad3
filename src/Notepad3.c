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
*                                                  (c) Rizonesoft 2008-2026   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

#include "Helpers.h"

#if (defined(_DEBUG) || defined(DEBUG)) && !defined(NDEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <commctrl.h>
#include <uxtheme.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <commdlg.h>
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <vsstyle.h>
//#include <ShellScalingApi.h>

#include "PathLib.h"
#include "Edit.h"
#include "Styles.h"
#include "Dialogs.h"
#include "crypto/crypto.h"
#include "uthash/utarray.h"
#include "uthash/utlist.h"
#include "tinyexpr/tinyexpr.h"
//#include "tinyexprcpp/tinyexpr_cif.h"
#include "Encoding.h"
#include "VersionEx.h"
#include "MuiLanguage.h"
#include "Notepad3.h"
#include "Config/Config.h"
#include "DarkMode/DarkMode.h"
#include "StyleLexers/EditLexer.h"

#if (defined(_DEBUG) || defined(DEBUG)) && !defined(NDEBUG)
#if defined(WIN32) && !defined(_WIN64)
        #pragma comment(linker, "/defaultlib:clang_rt.asan-i386.lib")
    #endif // _WIN64
#endif // DEBUG


// ============================================================================
//
//   Local and global Variables for Notepad3.c
//
// ============================================================================

LPCWSTR WordBookMarks[MARKER_NP3_BOOKMARK] = {
    /*0*/ L"back:#0000", // OCC MARKER
    /*1*/ L"back:#FF0000",
    /*2*/ L"back:#00FF00",
    /*3*/ L"back:#0000FF",
    /*4*/ L"back:#FF8000",
    /*5*/ L"back:#0080FF",
    /*6*/ L"back:#FF00FF",
    /*7*/ L"back:#408040",
    /*8*/ L"back:#C080C0"
};


#define RELAUNCH_ELEVATED_BUF_ARG L"tmpfbuf="

CONSTANTS_T const Constants = {
      2                                    // StdDefaultLexerID
    , L"minipath.exe"                      // FileBrowserMiniPath
    , L"grepWinNP3.exe"                    // FileSearchGrepWin
    , L"Settings"                          // Inifile Section "Settings"
    , L"Settings2"                         // Inifile Section "Settings2"
    , L"Window"                            // Inifile Section "Window"
    , L"Styles"                            // Inifile Section "Styles"
    , L"Suppressed Messages"               // Inifile Section "SuppressedMessages"
};


FLAGS_T     Flags = { 0 };
FLAGS_T DefaultFlags = { 0 };

GLOBALS_T   Globals = { 0 };
SETTINGS_T  Settings = { 0 };
SETTINGS_T  Defaults = { 0 };
SETTINGS2_T Settings2 = { 0 };

PATHS_T Paths = { 0 };

FOCUSEDVIEW_T  FocusedView = { 0 };
FILEWATCHING_T FileWatching = { 0 };

// set by InitScintillaHandle()
HWND      g_hwndEditWindow = NULL; 
HANDLE    g_hndlScintilla  = NULL;

// window positioning
WININFO   g_IniWinInfo = INIT_WININFO;
WININFO   g_DefWinInfo = INIT_WININFO;

COLORREF  g_colorCustom[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

prefix_t g_mxSBPrefix[STATUS_SECTOR_COUNT] = { L'\0' };
prefix_t g_mxSBPostfix[STATUS_SECTOR_COUNT] = { L'\0' };

int       g_flagMatchText = 0;
bool      g_iStatusbarVisible[STATUS_SECTOR_COUNT] = SBS_INIT_ZERO;
int       g_iStatusbarWidthSpec[STATUS_SECTOR_COUNT] = SBS_INIT_ZERO;
int       g_vSBSOrder[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

HPATHL    g_tchToolbarBitmap = NULL;
HPATHL    g_tchToolbarBitmapHot = NULL;
HPATHL    g_tchToolbarBitmapDisabled = NULL;

WCHAR Default_PreferredLanguageLocaleName[LOCALE_NAME_MAX_LENGTH + 1] = { L'\0' };

// ------------------------------------

HPATHL s_hpthRelaunchElevatedFile = NULL;

static bool      s_bInitAppDone = false;
static bool      s_bIsProcessElevated = false;
static bool      s_bIsUserInAdminGroup = false;
static bool      s_bIsRunAsAdmin = false;
static bool      s_flagSaveOnRelaunch = false;
static bool      s_IsThisAnElevatedRelaunch = false;

static WCHAR     s_wchWndClass[64] = { L'\0' };

static HWND      s_hwndEditFrame = NULL;
static HWND      s_hwndNextCBChain = NULL;

static int       s_WinCurrentWidth = 0;

#define FILE_LIST_SIZE 32
static LPWSTR    s_lpFileList[FILE_LIST_SIZE] = { NULL };
static int       s_cFileList = 0;
static int       s_cchiFileList = 0;

static int       s_iSortOptions = 0;
static int       s_iAlignMode = 0;
static bool      s_bIsAppThemed = true;
static UINT      s_msgTaskbarCreated = 0;
static WCHAR     s_wchTitleExcerpt[MIDSZ_BUFFER] = { L'\0' };
static LONG64    s_iLastCopyTime = 0;
static bool      s_bLastCopyFromMe = false;
static bool      s_bInMultiEditMode = false;
static bool      s_bCallTipEscDisabled = false;

static int       s_iInitialLine = 0;
static int       s_iInitialColumn = 0;
static int       s_iInitialLexer = 0;

static int       s_cyReBar = 0;
static int       s_cyReBarFrame = 0;
static int       s_cxEditFrame = 0;
static int       s_cyEditFrame = 0;
static bool      s_bUndoRedoScroll = false;

// for tiny expression calculation
static double   s_dExpression = 0.0;
static te_int_t s_iExprError  = -1;

static char*    s_SelectionBuffer = NULL;

//~static CONST WCHAR *const s_ToolbarWndClassName = L"NP3_TOOLBAR_CLASS";

static int const INISECTIONBUFCNT = 32; // .ini file load buffer in KB

// ----------------------------------------------------------------------------

const char* const _assert_msg = "Broken UndoRedo-Transaction!";

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// (!) ENSURE  IDT_FILE_NEW -> IDT_VIEW_NEW_WINDOW corresponds to order of Toolbar.bmp
#define NUMTOOLBITMAPS (31)
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static TBBUTTON  s_tbbMainWnd[] = {
    { 0, IDT_FILE_NEW, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 31, IDT_VIEW_NEW_WINDOW, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 0, 0, 0, BTNS_SEP, { 0 }, 0, 0 },
    { 1, IDT_FILE_OPEN, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 3, IDT_FILE_SAVE, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 2, IDT_FILE_BROWSE, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 27, IDT_FILE_RECENT, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 0, 0, 0, BTNS_SEP, { 0 }, 0, 0 },
    { 4, IDT_EDIT_UNDO, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 5, IDT_EDIT_REDO, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 0, 0, 0, BTNS_SEP, { 0 }, 0, 0 },
    { 6, IDT_EDIT_CUT, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 7, IDT_EDIT_COPY, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 8, IDT_EDIT_PASTE, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 0, 0, 0, BTNS_SEP, { 0 }, 0, 0 },
    { 9, IDT_EDIT_FIND, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 10, IDT_EDIT_REPLACE, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 0, 0, 0, BTNS_SEP, { 0 }, 0, 0 },
    { 29, IDT_GREP_WIN_TOOL, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 0, 0, 0, BTNS_SEP, { 0 }, 0, 0 },
    { 11, IDT_VIEW_WORDWRAP, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 0, 0, 0, BTNS_SEP, { 0 }, 0, 0 },
    { 23, IDT_VIEW_TOGGLEFOLDS, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 25, IDT_VIEW_TOGGLE_VIEW, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 0, 0, 0, BTNS_SEP, { 0 }, 0, 0 },
    { 21, IDT_FILE_OPENFAV, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 22, IDT_FILE_ADDTOFAV, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 0, 0, 0, BTNS_SEP, { 0 }, 0, 0 },
    { 12, IDT_VIEW_ZOOMIN, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 30, IDT_VIEW_RESETZOOM, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 13, IDT_VIEW_ZOOMOUT, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 0, 0, 0, BTNS_SEP, { 0 }, 0, 0 },
    { 14, IDT_VIEW_SCHEME, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 0, 0, 0, BTNS_SEP, { 0 }, 0, 0 },
    { 24, IDT_FILE_LAUNCH, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 0, 0, 0, BTNS_SEP, { 0 }, 0, 0 },
    { 28, IDT_VIEW_PIN_ON_TOP, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 0, 0, 0, BTNS_SEP, { 0 }, 0, 0 },
    { 16, IDT_FILE_EXIT, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 0, 0, 0, BTNS_SEP, { 0 }, 0, 0 },
    { 15, IDT_VIEW_SCHEMECONFIG, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 17, IDT_FILE_SAVEAS, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 18, IDT_FILE_SAVECOPY, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 19, IDT_EDIT_CLEAR, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 20, IDT_FILE_PRINT, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 },
    { 26, IDT_VIEW_CHASING_DOCTAIL, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0, 0 }
};
// don't show buttons beyond this TBBUTTON[] index:
#define TBBUTTON_LAST_DEFAULT (39)

WCHAR              TBBUTTON_DEFAULT_IDS[256] = { L'\0' };  // filled in _InitGlobals()
const WCHAR* const TBBUTTON_DEFAULT_IDS_OLD = L"1 32 2 4 3 28 0 5 6 0 7 8 9 0 10 11 0 30 0 12 0 24 26 0 22 23 0 13 14 0 27 0 15 0 25 0 17";

// ----------------------------------------------------------------------------

const char chr_currency[6] = { '$', 0x80, 0xA2, 0xA3, 0xA5, '\0' }; // "$€¢£¥" CP-1252

//==============================================================================
//
//  Save Needed Flag
//
static bool s_NeedSavingForced = false; // dirty-flag

static inline bool IsSaveNeeded()
{
    return SciCall_GetModify() || s_NeedSavingForced;
}

static inline void SetSaveNeeded(const bool bSetFlagForced)
{
    AutoSaveStart(!s_NeedSavingForced);
    if (bSetFlagForced) {
        s_NeedSavingForced = true;
    }
    if (IsWindow(Globals.hwndDlgFindReplace)) {
        PostWMCommand(Globals.hwndDlgFindReplace, IDC_DOC_MODIFIED);
    }
    UpdateToolbar();
}

static inline void SetSavePoint()
{
    SciCall_SetSavePoint();
    UpdateToolbar();
}

void SetSaveDone()
{
    s_NeedSavingForced = false;
    SetSavePoint();
}

//==============================================================================

// current find pattern
static HSTRINGW s_hstrCurrentFindPattern = NULL;


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

    if (selection != NULL)
    {
        selection->selMode_undo = SC_SEL_STREAM;
        selection->selMode_redo = SC_SEL_STREAM;

        if (selection->anchorPos_undo != NULL) {
            utarray_free(selection->anchorPos_undo);
            selection->anchorPos_undo = NULL;
        }
        if (selection->curPos_undo != NULL) {
            utarray_free(selection->curPos_undo);
            selection->curPos_undo = NULL;
        }
        if (selection->anchorVS_undo != NULL) {
            utarray_free(selection->anchorVS_undo);
            selection->anchorVS_undo = NULL;
        }
        if (selection->curVS_undo != NULL) {
            utarray_free(selection->curVS_undo);
            selection->curVS_undo = NULL;
        }

        if (selection->anchorPos_redo != NULL) {
            utarray_free(selection->anchorPos_redo);
            selection->anchorPos_redo = NULL;
        }
        if (selection->curPos_redo != NULL) {
            utarray_free(selection->curPos_redo);
            selection->curPos_redo = NULL;
        }
        if (selection->anchorVS_redo != NULL) {
            utarray_free(selection->anchorVS_redo);
            selection->anchorVS_redo = NULL;
        }
        if (selection->curVS_redo != NULL) {
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
static void  _SaveRedoSelection(const LONG token, const bool bAddAction);
static LONG  _SaveUndoSelection();
static LONG  _UndoRedoActionMap(const LONG token, const UndoRedoSelection_t** selection);
// => UndoTransActionBegin();
// => EndUndoTransAction();

// ----------------------------------------------------------------------------

static inline void _SplitUndoTransaction()
{
    if (SciCall_GetUndoSequence() > 0) {
        SciCall_EndUndoAction();
        SciCall_BeginUndoAction();
    }
    else {
        SciCall_BeginUndoAction();
        SciCall_EndUndoAction();
    }
}

// ----------------------------------------------------------------------------

static void _DelayClearCallTip(const LONG64 delay);
static void _DelaySplitUndoTransaction(const LONG64 delay);
static void _RestoreActionSelection(const LONG token, DoAction doAct);

// ----------------------------------------------------------------------------

#if 0
static HANDLE s_hEvent = INVALID_HANDLE_VALUE;
// SetEvent(s_hEvent);
// ResetEvent(s_hEvent);
static __forceinline bool IsEventSignaled() {
    return (WaitForSingleObject(s_hEvent, 0) == WAIT_OBJECT_0);
}
#endif

// ----------------------------------------------------------------------------


__forceinline bool NotifyDocChanged()
{
    return ((SciCall_GetModEventMask() & EVM_Default) == EVM_Default);
}

int DisableDocChangeNotification()
{
    int const currentMask = SciCall_GetModEventMask();
    SciCall_SetModEventMask(EVM_None);
    return currentMask;
}

void EnableDocChangeNotification(const int evm)
{
    SciCall_SetModEventMask(evm);
    if (NotifyDocChanged()) {
        EditUpdateVisibleIndicators();
        UpdateStatusbar(false);
    }
}


//=============================================================================
//
//  Delay Message Queue Handling  (TODO: MultiThreading)
//

static CmdMessageQueue_t* MessageQueue = NULL;

// ----------------------------------------------------------------------------

static int msgcmp(void* mqc1, void* mqc2)
{
    const CmdMessageQueue_t* const pMQC1 = (CmdMessageQueue_t*)mqc1;
    const CmdMessageQueue_t* const pMQC2 = (CmdMessageQueue_t*)mqc2;

    if ((pMQC1->cmd == pMQC2->cmd)
        && (pMQC1->hwnd == pMQC2->hwnd)
        && (pMQC1->wparam == pMQC2->wparam) // command
        && (pMQC1->lparam == pMQC2->lparam) // true/false
    ) {
        return 0; // equal
    }
    return 1;
}

//~static int sortcmp(void *mqc1, void *mqc2) {
//~
//~    const CmdMessageQueue_t *const pMQC1 = (CmdMessageQueue_t *)mqc1;
//~    const CmdMessageQueue_t *const pMQC2 = (CmdMessageQueue_t *)mqc2;
//~
//~    return (pMQC1->delay - pMQC2->delay);
//~}

// ----------------------------------------------------------------------------

#define _MQ_TIMER_CYCLE (USER_TIMER_MINIMUM << 1) // 20ms cycle
#define _MQ_IMMEDIATE (USER_TIMER_MINIMUM >> 1)
#define _MQ_FAST (_MQ_TIMER_CYCLE << 1)
#define _MQ_STD (_MQ_TIMER_CYCLE << 2)
#define _MQ_LAZY (_MQ_TIMER_CYCLE << 3)

#define _MQ_ms2cycl(T) (((T) + USER_TIMER_MINIMUM) / _MQ_TIMER_CYCLE)

static void _MQ_AppendCmd(CmdMessageQueue_t* const pMsgQCmd, LONG64 cycles)
{
    if (!pMsgQCmd) { return; }

    cycles = clampll(cycles, 0, _MQ_ms2cycl(60000));

    CmdMessageQueue_t* pmqc = NULL;
    DL_SEARCH(MessageQueue, pmqc, pMsgQCmd, msgcmp);

    if (!pmqc) { // NOT found, create one
        pmqc = AllocMem(sizeof(CmdMessageQueue_t), HEAP_ZERO_MEMORY);
        if (pmqc) {
            *pmqc = *pMsgQCmd;
            pmqc->delay = cycles;
            DL_APPEND(MessageQueue, pmqc);
        }
    } else {
        if ((pmqc->delay > 0) && (cycles > 0)) {
            pmqc->delay = (pmqc->delay + cycles) >> 1; // median delay
        } else {
            pmqc->delay = cycles;
        }
    }
    if (0 == cycles) {
        PostMessage(pMsgQCmd->hwnd, pMsgQCmd->cmd, pMsgQCmd->wparam, pMsgQCmd->lparam);
    }
    //~DL_SORT(MessageQueue, sortcmp); // next scheduled first
}
// ----------------------------------------------------------------------------

/* not used yet
static void _MQ_DropAll()
{
    CmdMessageQueue_t *pmqc = NULL;
    DL_FOREACH(MessageQueue, pmqc)
    {
        pmqc->delay = -1;
    }
}
*/
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
//
// called by Timer(IDT_TIMER_MRKALL)
//
// TIMERPROC
static void CALLBACK MQ_ExecuteNext(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    UNREFERENCED_PARAMETER(hwnd);    // must be main window handle
    UNREFERENCED_PARAMETER(uMsg);    // must be WM_TIMER
    UNREFERENCED_PARAMETER(idEvent); // must be IDT_TIMER_MRKALL
    UNREFERENCED_PARAMETER(dwTime);  // This is the value returned by the GetTickCount() function

    CmdMessageQueue_t* pmqc;
    DL_FOREACH(MessageQueue, pmqc) {
        if (pmqc->delay >= 0) {
            --(pmqc->delay);  // count down
        }
        if (pmqc->delay == 0) {
            PostMessage(pmqc->hwnd, pmqc->cmd, pmqc->wparam, pmqc->lparam);
        }
    }
}


//=============================================================================
//
// IsFileReadOnly
//
static bool IsFileReadOnly()
{
    return (Path_IsNotEmpty(Paths.CurrentFile) ? IsReadOnly(Path_GetFileAttributes(Paths.CurrentFile)) : false);
}



//=============================================================================
//
// HasCurrentFileChanged
//

static FCOBSRVDATA_T s_FileChgObsvrData = INIT_FCOBSRV_T;

// ----------------------------------------------------------------------------

static inline bool IsFileChangedFlagSet() {
    return (WaitForSingleObject(s_FileChgObsvrData.hEventFileChanged, 0) != WAIT_TIMEOUT);
}

static inline bool IsFileDeletedFlagSet() {
    return (WaitForSingleObject(s_FileChgObsvrData.hEventFileDeleted, 0) != WAIT_TIMEOUT);
}

static inline bool RaiseFlagIfCurrentFileChanged() {

    if (Path_IsEmpty(Paths.CurrentFile)) {
        return false;
    }
    WIN32_FIND_DATA fdUpdated = { 0 };
    if (!GetFileAttributesExW(Path_Get(Paths.CurrentFile), GetFileExInfoStandard, &fdUpdated)) {
        // The current file has been removed
        if (IsFileDeletedFlagSet()) {
            return false;
        }
        SetEvent(s_FileChgObsvrData.hEventFileChanged);
        SetEvent(s_FileChgObsvrData.hEventFileDeleted);
        return true;
    }
    if (IsFileDeletedFlagSet()) {
        // The current file has been restored
        ResetEvent(s_FileChgObsvrData.hEventFileDeleted);
    }
    
    bool const changed = (s_FileChgObsvrData.fdCurFile.nFileSizeLow != fdUpdated.nFileSizeLow) || (s_FileChgObsvrData.fdCurFile.nFileSizeHigh != fdUpdated.nFileSizeHigh)
                         //~|| (CompareFileTime(&(s_FileChgObsvrData.fdCurFile.ftLastWriteTime), &fdUpdated.ftLastWriteTime) != 0)
                         || (s_FileChgObsvrData.fdCurFile.ftLastWriteTime.dwLowDateTime != fdUpdated.ftLastWriteTime.dwLowDateTime)
                         || (s_FileChgObsvrData.fdCurFile.ftLastWriteTime.dwHighDateTime != fdUpdated.ftLastWriteTime.dwHighDateTime);
    if (changed) {
        SetEvent(s_FileChgObsvrData.hEventFileChanged);
    }
    return changed;
}
// ----------------------------------------------------------------------------

static inline void ResetFileObservationData(const bool bResetEvt) {
    if (bResetEvt) {
        ResetEvent(s_FileChgObsvrData.hEventFileChanged);
        ResetEvent(s_FileChgObsvrData.hEventFileDeleted);
    }
    if (Path_IsNotEmpty(Paths.CurrentFile)) {
        if (!GetFileAttributesEx(Path_Get(Paths.CurrentFile), GetFileExInfoStandard, &(s_FileChgObsvrData.fdCurFile)))
        {
            ZeroMemory(&(s_FileChgObsvrData.fdCurFile), sizeof(WIN32_FIND_DATA));
        }
    }

}
// ----------------------------------------------------------------------------

#define TE_ZERO (1.0E-8)
#define TE_FMTA  "%.8G"
#define TE_FMTW  L"%.8G"

void TinyExprToStringA(LPSTR pszDest, size_t cchDest, const double dExprEval)
{
    double       intpart = 0.0;
    double const fracpart = modf(dExprEval, &intpart);
    if ((fabs(fracpart) < TE_ZERO) && (fabs(intpart) < 1.0E+21)) {
        StringCchPrintfA(pszDest, cchDest, "%.21G", intpart); // integer full number display
    }
    else {
        StringCchPrintfA(pszDest, cchDest, TE_FMTA, dExprEval);
    }
}
// ----------------------------------------------------------------------------

void TinyExprToString(LPWSTR pszDest, size_t cchDest, const double dExprEval)
{
    double       intpart = 0.0;
    double const fracpart = modf(dExprEval, &intpart);
    if ((fabs(fracpart) < TE_ZERO) && (fabs(intpart) < 1.0E+21)) {
        StringCchPrintf(pszDest, cchDest, L"%.21G", intpart); // integer full number display
    }
    else {
        StringCchPrintf(pszDest, cchDest, TE_FMTW, dExprEval);
    }
}
// ----------------------------------------------------------------------------


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

static HPATHL                s_pthArgFilePath = NULL;
static HPATHL                s_pthCheckFilePath = NULL;

static cpi_enc_t             s_flagSetEncoding = CPI_NONE;
static int                   s_flagSetEOLMode = 0;
static bool                  s_flagStartAsTrayIcon = false;
static bool                  s_flagKeepTitleExcerpt = false;
static bool                  s_flagNewFromClipboard = false;
static bool                  s_flagPasteBoard = false;
static bool                  s_flagJumpTo = false;
static FILE_WATCHING_MODE    s_flagChangeNotify = FWM_NO_INIT;
static bool                  s_flagQuietCreate = false;
static bool                  s_flagLexerSpecified = false;
static bool                  s_flagDisplayHelp = false;
static int                   s_iCaretPolicyV = CARET_EVEN;


//==============================================================================

static volatile HANDLE    s_hEventAppIsClosing = INVALID_HANDLE_VALUE;
static __forceinline bool IsAppClosing() {
    return (WaitForSingleObject(s_hEventAppIsClosing, 0) == WAIT_OBJECT_0);
}


//==============================================================================

// static forward declarations
static void  _UpdateStatusbarDelayed(bool bForceRedraw);
static void  _UpdateToolbarDelayed();
static void  _UpdateTitlebarDelayed(const HWND hwnd);

//==============================================================================


static void _InitGlobals()
{
    Globals.hLngResContainer = NULL;

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
    Globals.uAvailLngCount = 1;
    Globals.iWrapCol = 80;

    Globals.CmdLnFlag_PosParam = false;
    Globals.CmdLnFlag_AlwaysOnTop = 0;
    Globals.CmdLnFlag_WindowPos = 0;
    Globals.CmdLnFlag_ReuseWindow = 0;
    Globals.CmdLnFlag_SingleFileInstance = 0;
    Globals.CmdLnFlag_MultiFileArg = 0;
    Globals.CmdLnFlag_ShellUseSystemMRU = 0;
    Globals.CmdLnFlag_PrintFileAndLeave = 0;

    Globals.iWhiteSpaceSize = 2;
    Globals.iCaretOutLineFrameSize = 0;

    Globals.DOSEncoding = CPI_NONE;
    Globals.bZeroBasedColumnIndex = false;
    Globals.bZeroBasedCharacterCount = false;
    Globals.iReplacedOccurrences = 0;
    Globals.iSelectionMarkNumber = 0;
    Globals.iMarkOccurrencesCount = 0;
    Globals.bUseLimitedAutoCCharSet = false;
    Globals.bIsCJKInputCodePage = false;
    Globals.bIniFileFromScratch = false;
    Globals.bFindReplCopySelOrClip = true;
    Globals.bReplaceInitialized = false;
    Globals.FindReplaceMatchFoundState = FND_NOP;
    Globals.bDocHasInconsistentEOLs = false;
    Globals.pStdDarkModeIniStyles = NULL;
    Globals.bMinimizedToTray = false;
    Globals.uCurrentThemeIndex = 0;

    Flags.bHugeFileLoadState = DefaultFlags.bHugeFileLoadState = false;
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
    Flags.SaveBlankNewFile = DefaultFlags.SaveBlankNewFile = true;
    Flags.ShellUseSystemMRU = DefaultFlags.ShellUseSystemMRU = true;
    Flags.PrintFileAndLeave = DefaultFlags.PrintFileAndLeave = 0;
    Flags.bPreserveFileModTime = DefaultFlags.bPreserveFileModTime = false;
    Flags.bDoRelaunchElevated = DefaultFlags.bDoRelaunchElevated = false;
    Flags.bSearchPathIfRelative = DefaultFlags.bSearchPathIfRelative = false;

    Flags.bSettingsFileSoftLocked = DefaultFlags.bSettingsFileSoftLocked = false;

    Settings.EFR_Data.chFindPattern = StrgCreate(NULL);
    Settings.EFR_Data.chReplaceTemplate = StrgCreate(NULL);
    Defaults.EFR_Data.chFindPattern = StrgCreate(NULL);
    Defaults.EFR_Data.chReplaceTemplate = StrgCreate(NULL);

    Settings.OpenWithDir = Path_Allocate(NULL);
    Defaults.OpenWithDir = Path_Allocate(NULL);
    Settings.FavoritesDir = Path_Allocate(NULL);
    Defaults.FavoritesDir = Path_Allocate(NULL);

    Settings2.DefaultDirectory = Path_Allocate(NULL);
    Settings2.FileBrowserPath = Path_Allocate(NULL);
    Settings2.GrepWinPath = Path_Allocate(NULL);
    Settings2.AdministrationTool = Path_Allocate(NULL);

    Settings2.WebTemplate1 = StrgCreate(NULL);
    Settings2.WebTemplate2 = StrgCreate(NULL);
    Settings2.HyperlinkShellExURLWithApp = StrgCreate(NULL);
    Settings2.HyperlinkShellExURLCmdLnArgs = StrgCreate(NULL);
    Settings2.FileDlgFilters = StrgCreate(NULL);

    FocusedView.HideNonMatchedLines = false;
    FocusedView.CodeFoldingAvailable = false;
    FocusedView.ShowCodeFolding = true;

    FileWatching.FileWatchingMode = FWM_DONT_CARE;
    FileWatching.MonitoringLog = false;

    Paths.CurrentFile = Path_Allocate(NULL);
    Paths.ModuleDirectory = Path_Allocate(NULL);
    Paths.WorkingDirectory = Path_Allocate(NULL);
    Paths.IniFile = Path_Allocate(NULL);
    Paths.IniFileDefault = Path_Allocate(NULL);

    // --- unstructured globals ---
    
    g_tchToolbarBitmap = Path_Allocate(NULL);
    g_tchToolbarBitmapHot = Path_Allocate(NULL);
    g_tchToolbarBitmapDisabled = Path_Allocate(NULL);

    // --- dynamicly created globals ---

    WCHAR tchIndex[16] = { L'\0' };
    StringCchPrintf(tchIndex, COUNTOF(tchIndex), L"%i", s_tbbMainWnd[0].iBitmap + 1);
    StringCchCopy(TBBUTTON_DEFAULT_IDS, COUNTOF(TBBUTTON_DEFAULT_IDS), tchIndex);
    assert(TBBUTTON_LAST_DEFAULT <= COUNTOF(s_tbbMainWnd));
    for (int i = 1; i < TBBUTTON_LAST_DEFAULT; ++i) {
        if (s_tbbMainWnd[i].idCommand) {
            StringCchPrintf(tchIndex, COUNTOF(tchIndex), L" %i", s_tbbMainWnd[i].iBitmap + 1);
            StringCchCat(TBBUTTON_DEFAULT_IDS, COUNTOF(TBBUTTON_DEFAULT_IDS), tchIndex);
        } else {
            StringCchCat(TBBUTTON_DEFAULT_IDS, COUNTOF(TBBUTTON_DEFAULT_IDS), L" 0");
        }
    }

    // --- static locals ---

    s_hpthRelaunchElevatedFile = Path_Allocate(NULL);

    ThemesItems_Init();
    s_hstrCurrentFindPattern = StrgCreate(NULL);

    s_pthArgFilePath = Path_Allocate(NULL);
    s_pthCheckFilePath = Path_Allocate(NULL);
    
    // don't allow empty extensions settings => use default ext
    Style_InitFileExtensions();
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

    if (Globals.pStdDarkModeIniStyles) {
        FreeMem(Globals.pStdDarkModeIniStyles);
        Globals.pStdDarkModeIniStyles = NULL;
    }

    CmdMessageQueue_t* pmqc = NULL;
    CmdMessageQueue_t* dummy;
    DL_FOREACH_SAFE(MessageQueue, pmqc, dummy) {
        DL_DELETE(MessageQueue, pmqc);
        FreeMem(pmqc);
    }

    if (UndoRedoSelectionUTArray != NULL) {
        utarray_free(UndoRedoSelectionUTArray);
        UndoRedoSelectionUTArray = NULL;
    }

    if (IS_VALID_HANDLE(s_FileChgObsvrData.hEventFileChanged)) {
        CloseHandle(s_FileChgObsvrData.hEventFileChanged);
        s_FileChgObsvrData.hEventFileChanged = INVALID_HANDLE_VALUE;
    }
    if (IS_VALID_HANDLE(s_FileChgObsvrData.hEventFileDeleted)) {
        CloseHandle(s_FileChgObsvrData.hEventFileDeleted);
        s_FileChgObsvrData.hEventFileDeleted = INVALID_HANDLE_VALUE;
    }

    BackgroundWorker_Destroy(&(s_FileChgObsvrData.worker));

    // ---------------------------------------------

    if (s_SelectionBuffer) {
        FreeMem(s_SelectionBuffer);
        s_SelectionBuffer = NULL;
    }

    // ---------------------------------------------
    // Save Settings should be done elsewhere before
    // ---------------------------------------------

    if (Globals.hMainMenu) {
        DestroyMenu(Globals.hMainMenu);
    }

    #if defined(HAVE_DYN_LOAD_LIBS_MUI_LNGS)
        FreeLanguageResources();
    #endif

    CleanupDlgResources();

    Scintilla_ReleaseResources();

    if (bIsInitialized) {
        //~UnregisterClass(s_ToolbarWndClassName, Globals.hInstance);
        UnregisterClass(s_wchWndClass, Globals.hInstance);
    }

    ReleaseDarkMode();

    // ---  free allocated memory  ---

    if (s_lpOrigFileArg) {
        FreeMem(s_lpOrigFileArg);
        s_lpOrigFileArg = NULL;
    }

    MRU_Destroy(Globals.pFileMRU);
    MRU_Destroy(Globals.pMRUfind);
    MRU_Destroy(Globals.pMRUreplace);

    StrgDestroy(Settings2.FileDlgFilters);
    StrgDestroy(Settings2.HyperlinkShellExURLCmdLnArgs);
    StrgDestroy(Settings2.HyperlinkShellExURLWithApp);
    StrgDestroy(Settings2.WebTemplate2);
    StrgDestroy(Settings2.WebTemplate1);

    Path_Release(Settings2.AdministrationTool);
    Path_Release(Settings2.GrepWinPath);
    Path_Release(Settings2.FileBrowserPath);
    Path_Release(Settings2.DefaultDirectory);

    Path_Release(Settings.FavoritesDir);
    Path_Release(Defaults.FavoritesDir);
    Path_Release(Settings.OpenWithDir);
    Path_Release(Defaults.OpenWithDir);

    ReleaseEFR(&(Defaults.EFR_Data));
    ReleaseEFR(&(Settings.EFR_Data));

    Path_Release(Paths.CurrentFile);
    Path_Release(Paths.ModuleDirectory);
    Path_Release(Paths.WorkingDirectory);
    Path_Release(Paths.IniFileDefault);
    Path_Release(Paths.IniFile);

    Path_Release(g_tchToolbarBitmapDisabled);
    Path_Release(g_tchToolbarBitmapHot);
    Path_Release(g_tchToolbarBitmap);

    Path_Release(s_pthArgFilePath);
    Path_Release(s_pthCheckFilePath);

    StrgDestroy(s_hstrCurrentFindPattern);
    ThemesItems_Release();

    Path_Release(s_hpthRelaunchElevatedFile);

    // ---------------------------------------------

    if (Globals.hDlgIcon256) {
        DestroyIcon(Globals.hDlgIcon256);
    }
    if (Globals.hDlgIcon128) {
        DestroyIcon(Globals.hDlgIcon128);
    }
    if (Globals.hDlgIconBig) {
        DestroyIcon(Globals.hDlgIconBig);
    }
    if (Globals.hDlgIconSmall) {
        DestroyIcon(Globals.hDlgIconSmall);
    }
    if (Globals.hDlgIconPrefs256) {
        DestroyIcon(Globals.hDlgIconPrefs256);
    }
    if (Globals.hDlgIconPrefs128) {
        DestroyIcon(Globals.hDlgIconPrefs128);
    }
    if (Globals.hDlgIconPrefs64) {
        DestroyIcon(Globals.hDlgIconPrefs64);
    }
    if (Globals.hIconMsgUser) {
        DestroyIcon(Globals.hIconMsgUser);
    }
    if (Globals.hIconMsgInfo) {
        DestroyIcon(Globals.hIconMsgInfo);
    }
    if (Globals.hIconMsgWarn) {
        DestroyIcon(Globals.hIconMsgWarn);
    }
    if (Globals.hIconMsgError) {
        DestroyIcon(Globals.hIconMsgError);
    }
    if (Globals.hIconMsgQuest) {
        DestroyIcon(Globals.hIconMsgQuest);
    }
    if (Globals.hIconMsgShield) {
        DestroyIcon(Globals.hIconMsgShield);
    }
    if (Globals.hIconMsgWinCmd) {
        DestroyIcon(Globals.hIconMsgWinCmd);
    }
    if (Globals.hIconGrepWinNP3) {
        DestroyIcon(Globals.hIconGrepWinNP3);
    }

    // install previous handler
    if (_hOldInvalidParamHandler) {
        _set_invalid_parameter_handler(_hOldInvalidParamHandler);
    }
    
    CoUninitialize();
    OleUninitialize();
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
    UNREFERENCED_PARAMETER(expression);
    UNREFERENCED_PARAMETER(function);
    UNREFERENCED_PARAMETER(file);
    UNREFERENCED_PARAMETER(line);
    UNREFERENCED_PARAMETER(pReserved);
#if (defined(_DEBUG) || defined(DEBUG)) && !defined(NDEBUG)
    WCHAR msg[256];
    StringCchPrintf(msg, COUNTOF(msg),
                    L"Invalid Parameter in function '%s()' - File:'%s' Line:%i !",
                    function, file, line);
    MsgBoxLastError(msg, ERROR_INVALID_PARAMETER);
#endif
}


//=============================================================================
//
//  WinMain()
//
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
    UNREFERENCED_PARAMETER(lpCmdLine);

    _invalid_parameter_handler const hNewInvalidParamHandler = InvalidParameterHandler;
    _hOldInvalidParamHandler= _set_invalid_parameter_handler(hNewInvalidParamHandler);

#if (defined(_DEBUG) || defined(DEBUG)) && !defined(NDEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_CRT_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_ASSERT, 0); // Disable the message box for assertions.
#endif

    _InitGlobals();
    InitDarkMode();

    // Windows Class name
    StringCchCopy(s_wchWndClass, COUNTOF(s_wchWndClass), _W(SAPPNAME));

    // Set global variable Globals.hInstance
    Globals.hInstance = hInstance;
    Globals.hPrevInst = hPrevInstance;
    Globals.hndlProcessHeap = GetProcessHeap();

    // now using AllocMen() methods
    s_SelectionBuffer = (char*)AllocMem(128, HEAP_ZERO_MEMORY);


    Path_GetAppDirectory(Paths.ModuleDirectory);

    if (!Path_GetCurrentDirectory(Paths.WorkingDirectory)) {
        Path_Reset(Paths.WorkingDirectory, Path_Get(Paths.ModuleDirectory));
    }
    // Don't keep working directory locked
    SetCurrentDirectoryW(Path_Get(Paths.ModuleDirectory));

    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);

    // check if running at least on Windows 7 (SP1)
    if (!IsWindows7SP1OrGreater()) {
        MsgBoxLastError(L"Application Initialization", ERROR_OLD_WIN_VERSION);
        return 1; // exit
    }

    //~SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    //~SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    // Check if running with elevated privileges
    s_bIsProcessElevated = IsProcessElevated();
    s_bIsUserInAdminGroup = IsUserInAdminGroup();
    s_bIsRunAsAdmin = IsRunAsAdmin();

    // Adapt window class name
    if (s_bIsProcessElevated) {
        StringCchCat(s_wchWndClass, COUNTOF(s_wchWndClass), L"U");
    }

    // Default Encodings (may already be used for command line parsing)
    Encoding_InitDefaults();

    // Command Line, Ini File and Flags
    ParseCommandLine();

    // Adapt window class name
    if (s_flagPasteBoard) {
        StringCchCat(s_wchWndClass, COUNTOF(s_wchWndClass), L"B");
    }

    // INI File Handling
    FindIniFile();
    TestIniFile();
    DWORD dwFileSize = 0UL;
    Globals.bCanSaveIniFile = CreateIniFile(Paths.IniFile, &dwFileSize);
    Globals.bIniFileFromScratch = (dwFileSize == 0UL);
    if (Globals.bIniFileFromScratch && Globals.bCanSaveIniFile) {
        // Set at least Application Name Section
        IniFileSetString(Paths.IniFile, _W(SAPPNAME), NULL, NULL);
    }
    LoadSettings();

    PrivateSetCurrentProcessExplicitAppUserModelID(Settings2.AppUserModelID);

    (void)CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_SPEED_OVER_MEMORY);
    (void)OleInitialize(NULL);

    INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX) };
    icex.dwICC = ICC_WIN95_CLASSES | ICC_USEREX_CLASSES | ICC_COOL_CLASSES | ICC_NATIVEFNTCTL_CLASS | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    Scintilla_RegisterClasses(hInstance);

#ifdef D_NP3_WIN10_DARK_MODE
    SetDarkMode(IsDarkModeSupported() && IsSettingDarkMode()); // settings
#endif

    HRSRC const hRes = FindResourceEx(hInstance, RT_RCDATA, MAKEINTRESOURCE(IDR_STD_DARKMODE_THEME), 
                                                            MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
    if (hRes) {
        HGLOBAL const hMem = LoadResource(hInstance, hRes);
        if (hMem) {
            const char* const resText = (const char*)LockResource(hMem);
            DWORD const       size = SizeofResource(hInstance, hRes);
            Globals.pStdDarkModeIniStyles = (char*)AllocMem(((size_t)size + 1), 0);
            if (Globals.pStdDarkModeIniStyles) {
                CopyMemory(Globals.pStdDarkModeIniStyles, resText, size);
                Globals.pStdDarkModeIniStyles[size] = '\0'; // zero termination
            }
            FreeResource(hMem);
        }
    }
    Style_ImportTheme(-1); // init (!)
    Style_ImportTheme(Globals.uCurrentThemeIndex);

    //SetProcessDPIAware(); // ->.manifest
    //SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    //~Scintilla_LoadDpiForWindow(); done in Sci::Platform_Initialize();

    // ----------------------------------------------------
    // MultiLingual
    //
#if defined(HAVE_DYN_LOAD_LIBS_MUI_LNGS)
    SetMuiLanguage(LoadLanguageResources(Settings2.PreferredLanguageLocaleName));
#else
    SetMuiLanguage(GetMUILanguageIndexByLocaleName(MUI_BASE_LNG_ID));
#endif

    // ----------------------------------------------------

    // ICON_BIG (32x32)
    int const cxb = GetSystemMetrics(SM_CXICON);
    int const cyb = GetSystemMetrics(SM_CYICON);
    // ICON_SMALL (16x16)
    int const cxs = GetSystemMetrics(SM_CXSMICON);
    int const cys = GetSystemMetrics(SM_CYSMICON);

    UINT const fuLoad = LR_DEFAULTCOLOR | LR_SHARED;

    if (!Globals.hDlgIcon256) {
        if (FAILED(LoadIconWithScaleDown(hInstance, MAKEINTRESOURCE(IDR_MAINWND), 256, 256, &(Globals.hDlgIcon256))))
            Globals.hDlgIcon256 = LoadImage(hInstance, MAKEINTRESOURCE(IDR_MAINWND), IMAGE_ICON, 256, 256, fuLoad);
    }
    if (!Globals.hDlgIcon128) {
        if (FAILED(LoadIconWithScaleDown(hInstance, MAKEINTRESOURCE(IDR_MAINWND), 128, 128, &(Globals.hDlgIcon128))))
            Globals.hDlgIcon128 = LoadImage(hInstance, MAKEINTRESOURCE(IDR_MAINWND), IMAGE_ICON, 128, 128, fuLoad);
    }
    if (!Globals.hDlgIconBig) {
        if (FAILED(LoadIconWithScaleDown(hInstance, MAKEINTRESOURCE(IDR_MAINWND), cxb, cyb, &(Globals.hDlgIconBig))))
            Globals.hDlgIconBig = LoadImage(hInstance, MAKEINTRESOURCE(IDR_MAINWND), IMAGE_ICON, cxb, cyb, fuLoad);
    }
    if (!Globals.hDlgIconSmall) {
        if (FAILED(LoadIconWithScaleDown(hInstance, MAKEINTRESOURCE(IDR_MAINWND), cxs, cys, &(Globals.hDlgIconSmall))))
            Globals.hDlgIconSmall = LoadImage(hInstance, MAKEINTRESOURCE(IDR_MAINWND), IMAGE_ICON, cxs, cys, fuLoad);
    }

    if (!Globals.hDlgIconPrefs256) {
        if (FAILED(LoadIconWithScaleDown(hInstance, MAKEINTRESOURCE(IDI_MUI_STYLES), 256, 256, &(Globals.hDlgIconPrefs256))))
            Globals.hDlgIconPrefs256 = LoadImage(hInstance, MAKEINTRESOURCE(IDI_MUI_STYLES), IMAGE_ICON, 256, 256, fuLoad);
    }
    if (!Globals.hDlgIconPrefs128) {
        if (FAILED(LoadIconWithScaleDown(hInstance, MAKEINTRESOURCE(IDI_MUI_STYLES), 128, 128, &(Globals.hDlgIconPrefs128))))
            Globals.hDlgIconPrefs128 = LoadImage(hInstance, MAKEINTRESOURCE(IDI_MUI_STYLES), IMAGE_ICON, 128, 128, fuLoad);
    }
    if (!Globals.hDlgIconPrefs64) {
        if (FAILED(LoadIconWithScaleDown(hInstance, MAKEINTRESOURCE(IDI_MUI_STYLES), 64, 64, &(Globals.hDlgIconPrefs64))))
            Globals.hDlgIconPrefs64 = LoadImage(hInstance, MAKEINTRESOURCE(IDI_MUI_STYLES), IMAGE_ICON, 64, 64, fuLoad);
    }

    if (!Globals.hIconMsgUser) {
        if (FAILED(LoadIconWithScaleDown(hInstance, MAKEINTRESOURCE(IDR_MAINWND), cxb, cyb, &(Globals.hIconMsgUser))))
            Globals.hIconMsgUser = LoadImage(hInstance, MAKEINTRESOURCE(IDR_MAINWND), IMAGE_ICON, cxb, cyb, fuLoad);
    }
    if (!Globals.hIconMsgInfo) {
        if (FAILED(LoadIconWithScaleDown(NULL, IDI_INFORMATION, cxb, cyb, &(Globals.hIconMsgInfo))))
            Globals.hIconMsgInfo = LoadImage(NULL, IDI_INFORMATION, IMAGE_ICON, cxb, cyb, fuLoad);
    }
    if (!Globals.hIconMsgWarn) {
        if (FAILED(LoadIconWithScaleDown(NULL, IDI_WARNING, cxb, cyb, &(Globals.hIconMsgWarn))))
            Globals.hIconMsgWarn = LoadImage(NULL, IDI_WARNING, IMAGE_ICON, cxb, cyb, fuLoad);
    }
    if (!Globals.hIconMsgError) {
        if (FAILED(LoadIconWithScaleDown(NULL, IDI_ERROR, cxb, cyb, &(Globals.hIconMsgError))))
            Globals.hIconMsgError = LoadImage(NULL, IDI_ERROR, IMAGE_ICON, cxb, cyb, fuLoad);
    }
    if (!Globals.hIconMsgQuest) {
        if (FAILED(LoadIconWithScaleDown(NULL, IDI_QUESTION, cxb, cyb, &(Globals.hIconMsgQuest))))
            Globals.hIconMsgQuest = LoadImage(NULL, IDI_QUESTION, IMAGE_ICON, cxb, cyb, fuLoad);
    }
    if (!Globals.hIconMsgShield) {
        if (FAILED(LoadIconWithScaleDown(NULL, IDI_SHIELD, cxb, cyb, &(Globals.hIconMsgShield))))
            Globals.hIconMsgShield = LoadImage(NULL, IDI_SHIELD, IMAGE_ICON, cxb, cyb, fuLoad);
    }
    if (!Globals.hIconMsgWinCmd) {
        if (FAILED(LoadIconWithScaleDown(hInstance, MAKEINTRESOURCE(IDI_MUI_RUN), cxb, cyb, &(Globals.hIconMsgWinCmd))))
            Globals.hIconMsgWinCmd = LoadImage(hInstance, MAKEINTRESOURCE(IDI_MUI_RUN), IMAGE_ICON, cxb, cyb, fuLoad);
    }
    if (!Globals.hIconGrepWinNP3) {
        if (FAILED(LoadIconWithScaleDown(hInstance, MAKEINTRESOURCE(IDI_MUI_GREPWINNP3), cxs, cys, &(Globals.hIconGrepWinNP3))))
            Globals.hIconGrepWinNP3 = LoadImage(hInstance, MAKEINTRESOURCE(IDI_MUI_GREPWINNP3), IMAGE_ICON, cxs, cys, fuLoad);
    }

    if (s_IsThisAnElevatedRelaunch && !IsRunAsAdmin()) {
        InfoBoxLng(MB_ICONSHIELD, NULL, IDS_MUI_ERR_ELEVATED_RIGHTS);
        s_flagSaveOnRelaunch = false;
    }

    // try autoload most recent file, if activated
    bool const bIsAutoLoadMostRecent = CheckAutoLoadMostRecent();

    // Try to Relaunch with elevated privileges
    if (RelaunchElevated(NULL)) {
        return FALSE;
    }

    // Try to run multiple instances
    if (RelaunchMultiInst()) {
        return FALSE;
    }
    // Try to activate another window
    if (ActivatePrevInst(!bIsAutoLoadMostRecent)) {
        if (!bIsAutoLoadMostRecent) {
            return FALSE;
        }
        // instance with most recent exists,
        // so open empty new instance
        Path_Empty(s_pthArgFilePath, false);
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

    #if defined(HAVE_DYN_LOAD_LIBS_MUI_LNGS)
        InsertLanguageMenu(Globals.hMainMenu);
    #endif
    Style_InsertThemesMenu(Globals.hMainMenu);

    if (!InitApplication(Globals.hInstance)) {
        _CleanUpResources(NULL, false);
        return 1;
    }

    HWND const hwnd = InitInstance(Globals.hInstance, nShowCmd);
    if (!hwnd) {
        _CleanUpResources(hwnd, true);
        return 1;
    }

    // !!!  now, SciCall_ functions are available (initialized library)

    DrawMenuBar(hwnd);

    HACCEL const hAccMain = LoadAccelerators(hInstance,MAKEINTRESOURCE(IDR_MAINWND));
    HACCEL const hAccFindReplace = LoadAccelerators(hInstance,MAKEINTRESOURCE(IDR_ACCFINDREPLACE));
    HACCEL const hAccCoustomizeSchemes = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCCUSTOMSCHEMES));

    SetTimer(hwnd, IDT_TIMER_MRKALL, _MQ_TIMER_CYCLE, MQ_ExecuteNext);

    // clear caches
    ResetTmpCache();
    ResetIniFileCache();

    // drag-n-drop into elevated process even does not work using:
    ///ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
    ///ChangeWindowMessageFilter(WM_COPYDATA, MSGFLT_ADD);
    ///ChangeWindowMessageFilter(0x0049, MSGFLT_ADD);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {

        if (IsWindow(Globals.hwndDlgFindReplace) && ((msg.hwnd == Globals.hwndDlgFindReplace) || IsChild(Globals.hwndDlgFindReplace, msg.hwnd))) {
            const int iTr = TranslateAccelerator(Globals.hwndDlgFindReplace, hAccFindReplace, &msg);
            if (iTr || IsDialogMessage(Globals.hwndDlgFindReplace, &msg)) {
                continue;
            }
        }
        if (IsWindow(Globals.hwndDlgCustomizeSchemes) && ((msg.hwnd == Globals.hwndDlgCustomizeSchemes) || IsChild(Globals.hwndDlgCustomizeSchemes, msg.hwnd))) {
            const int iTr = TranslateAccelerator(Globals.hwndDlgCustomizeSchemes, hAccCoustomizeSchemes, &msg);
            if (iTr || IsDialogMessage(Globals.hwndDlgCustomizeSchemes, &msg)) {
                continue;
            }
        }
        if (!TranslateAccelerator(hwnd, hAccMain, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    _CleanUpResources(hwnd, true);

#ifdef _DEBUG
    _CrtDumpMemoryLeaks();
#endif
    return (int)(msg.wParam);
}

//=============================================================================
//
//  GetFactoryDefaultWndPos()
//
//
WININFO GetFactoryDefaultWndPos(HWND hwnd, const int flagsPos)
{
    hwnd = hwnd ? hwnd : GetDesktopWindow();
    unsigned int const dpi = Scintilla_GetWindowDPI(hwnd);

    RECT rc = { 0 };
    GetWindowRect(hwnd, &rc);
    MONITORINFO mi;
    GetMonitorInfoFromRect(&rc, &mi);
    
    rc = mi.rcWork;
    //~RelAdjustRectForDPI(&rc, USER_DEFAULT_SCREEN_DPI, dpi);

    WININFO winfo = INIT_WININFO;
    winfo.y = rc.top;
    winfo.cy = rc.bottom - rc.top;
    winfo.cx = (rc.right - rc.left) / 2;
    winfo.x = (flagsPos == 3) ? rc.left : winfo.cx;
    winfo.max = 0;
    winfo.zoom = 100;
    winfo.dpi = dpi;

    return winfo;
}
// ----------------------------------------------------------------------------


//=============================================================================
//
//  _GetDefaultWinInfoByStrg()
//
static void _IniFileWriteDefaultWindowPosition(WININFO wi)
{
    WCHAR tchScrnDim[64] = { L'\0' };
    StringCchPrintf(tchScrnDim, COUNTOF(tchScrnDim), L"%ix%i " DEF_WIN_POSITION_STRG,
        GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN));
    StringCchPrintf(Settings2.DefaultWindowPosition, COUNTOF(Settings2.DefaultWindowPosition),
        WINDOWPOS_STRGFORMAT, wi.x, wi.y, wi.cx, wi.cy, wi.dpi, wi.max);
    IniFileSetString(Paths.IniFile, Constants.Window_Section, tchScrnDim, Settings2.DefaultWindowPosition);
    IniFileDelete(Paths.IniFile, Constants.Settings2_Section, DEF_WIN_POSITION_STRG, true);
}

static WININFO _GetDefaultWinInfoByStrg(HWND hwnd, LPCWSTR strDefaultWinPos)
{
    hwnd = hwnd ? hwnd : GetDesktopWindow();
    WININFO const wiDef = GetFactoryDefaultWndPos(hwnd, 2); // std. default position

    if (StrIsEmpty(strDefaultWinPos)) {
        return wiDef;
    }

    WININFO   wi = wiDef;
    int       bMaxi = 0;
    int const itok = swscanf_s(Settings2.DefaultWindowPosition, WINDOWPOS_STRGFORMAT,
        &wi.x, &wi.y, &wi.cx, &wi.cy, &wi.dpi, &bMaxi);
    if (itok == 4 || itok == 5 || itok == 6) { // scan successful
        if (itok == 4) {
            wi.dpi = USER_DEFAULT_SCREEN_DPI;
            wi.max = false;
        }
        else if (itok == 5) { // maybe DPI or Maxi (old)
            if (wi.dpi < (USER_DEFAULT_SCREEN_DPI >> 2)) {
                wi.max = wi.dpi ? true : false;
                wi.dpi = USER_DEFAULT_SCREEN_DPI;
            }
            else {
                wi.max = false;
            }
        }
        else {
            wi.max = bMaxi ? true : false;
        }
    }
    else {
        wi = wiDef;
        // overwrite bad defined default position
        _IniFileWriteDefaultWindowPosition(wi);
    }
    return wi;
}


//=============================================================================
//
//  GetWinInfoByFlag()
//
//
WININFO GetWinInfoByFlag(HWND hwnd, const int flagsPos)
{
    hwnd = hwnd ? hwnd : GetDesktopWindow();
    if ((g_DefWinInfo.x == CW_USEDEFAULT) || (g_DefWinInfo.y == CW_USEDEFAULT)) {
        g_DefWinInfo = _GetDefaultWinInfoByStrg(hwnd, Settings2.DefaultWindowPosition);
    }
    
    WININFO winfo = INIT_WININFO;
    if (flagsPos < 0) {
        winfo = GetMyWindowPlacement(hwnd, NULL, 0, false); // current window position
    } else if (flagsPos == 0) {
        winfo = g_IniWinInfo; // initial window position
    } else if (flagsPos == 1) {
        winfo.x = winfo.y = winfo.cx = winfo.cy = CW_USEDEFAULT;
        winfo.max = false;
        winfo.zoom = 100;
    } else if (flagsPos == 2) {
        winfo = g_DefWinInfo; // NP3 default window position
    } else if (flagsPos == 3) {
        winfo = GetFactoryDefaultWndPos(hwnd, flagsPos);
    } else if ((flagsPos >= 4) && (flagsPos < 256)) {
        RECT rc = { 0 };
        GetWindowRect(hwnd, &rc);
        MONITORINFO mi;
        GetMonitorInfoFromRect(&rc, &mi);
        rc = mi.rcWork;

        int const width = (rc.right - rc.left);
        int const height = (rc.bottom - rc.top);

        if (flagsPos & 8) {
            winfo.x = rc.left + (width >> 1);
        } else {
            winfo.x = rc.left;
        }

        if (flagsPos & (4 | 8)) {
            winfo.cx = (width >> 1);
        } else {
            winfo.cx = width;
        }

        if (flagsPos & 32) {
            winfo.y = rc.top + (height >> 1);
        } else {
            winfo.y = rc.top;
        }

        if (flagsPos & (16 | 32)) {
            winfo.cy = (height >> 1);
        } else {
            winfo.cy = height;
        }

        if (flagsPos & 64) {
            winfo.x = rc.left;
            winfo.y = rc.top;
            winfo.cx = width;
            winfo.cy = height;
        }
        if (flagsPos & 128) {
            winfo = g_DefWinInfo;
            winfo.max = true;
            winfo.zoom = 100;
        }
        winfo.dpi = Scintilla_GetWindowDPI(hwnd);

    } else { // ( > 256) restore window, move upper left corner to Work Area

        RECT rc = { 0 };
        RectFromWinInfo(&winfo, &rc);
        MONITORINFO mi;
        GetMonitorInfoFromRect(&rc, &mi);
        rc = mi.rcWork;
        WININFO wi = winfo;
        wi.cx = wi.cy = 16; // really small
        FitIntoMonitorGeometry(&rc, &wi, SCR_NORMAL, false);
        winfo.x = wi.x;
        winfo.y = wi.y;
    }

    return winfo;
}


//=============================================================================
//
//  _EnumWndProc : find other Notepad3 window 
//
static BOOL CALLBACK _EnumWndProc(HWND hwnd, LPARAM lParam)
{
    BOOL  bContinue = TRUE;
    WCHAR szClassName[128] = { L'\0' };

    if (GetClassName(hwnd, szClassName, COUNTOF(szClassName))) {

        if (StrCmpW(szClassName, s_wchWndClass) == 0) {

            UINT const iReuseLock = GetDlgItemInt(hwnd, IDC_REUSELOCK, NULL, FALSE);
            if ((GetTicks_ms() - iReuseLock) >= REUSEWINDOWLOCKTIMEOUT) {

                *(HWND*)lParam = hwnd;

                if (IsWindowEnabled(hwnd)) {
                    bContinue = FALSE;
                }
            }
        }
    }
    return bContinue;
}


//=============================================================================
//
//  _EnumWndProc2 : find other Notepad3 window w/ same file loaded
//
static BOOL CALLBACK _EnumWndProc2(HWND hwnd, LPARAM lParam)
{
    BOOL  bContinue = TRUE;
    WCHAR szClassName[128] = { L'\0' };

    if (GetClassName(hwnd, szClassName, COUNTOF(szClassName))) {

        if (StrCmpW(szClassName, s_wchWndClass) == 0) {

            WCHAR wchFileName[INTERNET_MAX_URL_LENGTH] = { L'\0' };
            GetDlgItemText(hwnd, IDC_FILENAME, wchFileName, COUNTOF(wchFileName));

            HPATHL hpthFileName = Path_Allocate(wchFileName);
            if (Path_StrgComparePath(hpthFileName, s_pthCheckFilePath, Paths.WorkingDirectory, true) == 0) {
                *(HWND*)lParam = hwnd;
                bContinue = FALSE;
            }
            Path_Release(hpthFileName);
        }
    }
    return bContinue;
}


static bool FindOtherInstance(HWND* phwnd, HPATHL hpthFileName)
{
    Path_Reset(s_pthCheckFilePath, Path_Get(hpthFileName));
    *phwnd = NULL;
    return !EnumWindows(_EnumWndProc2, (LPARAM)phwnd) && (*phwnd != NULL);
}


//=============================================================================
//
//  _StatusCalcTextSize()
//
static SIZE _StatusCalcTextSize(HWND hwnd, LPCWSTR lpsz)
{
    HDC const hdc = GetDC(hwnd);
    HGDIOBJ const hfont = (HGDIOBJ)SendMessage(hwnd, WM_GETFONT, 0, 0);
    HGDIOBJ const hfold = SelectObject(hdc, hfont);
    int const mmode = SetMapMode(hdc, MM_TEXT);

    SIZE size = { 0L, 0L };
    GetTextExtentPoint32(hdc, lpsz, (int)StringCchLen(lpsz, 0), &size);

    SetMapMode(hdc, mmode);
    SelectObject(hdc, hfold);
    ReleaseDC(hwnd, hdc);

    return size;
}


//=============================================================================
//
//  Set/Get FindPattern()
//

bool IsFindPatternEmpty()
{
    return StrgIsEmpty(s_hstrCurrentFindPattern);
}

//=============================================================================
//
//  GetFindPatternMB()
//
LPCWSTR GetFindPattern()
{
    return StrgGet(s_hstrCurrentFindPattern);
}

//=============================================================================
//
//  GetFindPatternMB()
//
void GetFindPatternMB(LPSTR chPattern, int cch)
{
    StrgGetAsUTF8(s_hstrCurrentFindPattern, chPattern, cch);
}

//=============================================================================
//
//  SetFindPattern()
//
void SetFindPattern(LPCWSTR wchFindPattern)
{
    StrgReset(s_hstrCurrentFindPattern, StrIsNotEmpty(wchFindPattern) ? wchFindPattern : L"");
}

//=============================================================================
//
//  SetFindPatternMB()
//
void SetFindPatternMB(LPCSTR chFindPattern)
{
    StrgResetFromUTF8(s_hstrCurrentFindPattern, !StrIsEmptyA(chFindPattern) ? chFindPattern : "");
}

//=============================================================================
//
//  LengthOfFindPattern()
//
size_t LengthOfFindPattern()
{
    return StrgGetLength(s_hstrCurrentFindPattern);
}

//=============================================================================
//
//  LengthOfFindPatternMB()
//
size_t LengthOfFindPatternMB()
{
    return (size_t)StrgGetAsUTF8(s_hstrCurrentFindPattern, NULL, 0);
}

// ----------------------------------------------------------------------------

static EDITFINDREPLACE s_FindReplaceData = INIT_EFR_DATA;

//=============================================================================
//
// SetFindReplaceData()
//
static void SetFindReplaceData()
{
    // init find pattern with current
    if (!IsFindPatternEmpty()) {
        StrgReset(Settings.EFR_Data.chFindPattern, GetFindPattern());
    }
    // copy settings to working data
    DuplicateEFR(&s_FindReplaceData, &(Settings.EFR_Data));

    if (g_flagMatchText) { // cmd line
        if (g_flagMatchText & 4) {
            s_FindReplaceData.fuFlags = (SCFIND_REGEXP | SCFIND_POSIX);
        } else {
            // /m without R flag: force text mode (clear regex flags) - fixes #5060
            s_FindReplaceData.fuFlags &= ~(SCFIND_REGEXP | SCFIND_POSIX);
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
        s_FindReplaceData.bRegExprSearch = (s_FindReplaceData.fuFlags & SCFIND_REGEXP);
        s_FindReplaceData.bWildcardSearch = false;
        s_FindReplaceData.bReplaceClose = false;
    }
}


//=============================================================================
//
// SetCurrentSelAsFindReplaceData()
//
static bool SetCurrentSelAsFindReplaceData(LPEDITFINDREPLACE lpefr)
{
    if (SciCall_IsSelectionEmpty()) {
        EditSelectWordAtPos(SciCall_GetCurrentPos(), true);
    }

    size_t const cchSelection = SciCall_GetSelText(NULL);

    if (0 < cchSelection) {
        char* const szSelection = AllocMem((cchSelection + 1), HEAP_ZERO_MEMORY);
        if (szSelection) {
            SciCall_GetSelText(szSelection);

            SetFindPatternMB(szSelection);
            SetFindReplaceData(); // s_FindReplaceData
            if (lpefr) {
                StrgReset(lpefr->chFindPattern, GetFindPattern());
            }

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
    WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
    wc.style = CS_BYTEALIGNWINDOW | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = (WNDPROC)MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = Globals.hDlgIcon256;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
#ifdef D_NP3_WIN10_DARK_MODE
    wc.hbrBackground = UseDarkMode() ? Globals.hbrDarkModeBkgBrush : (HBRUSH)(COLOR_WINDOW + 1);
#else
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
#endif
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MUI_MAINMENU);
    wc.lpszClassName = s_wchWndClass;

    return RegisterClassEx(&wc);
}


#if 0
//=============================================================================
//
// InitToolbarWndClass()
//
bool InitWndClass(const HINSTANCE hInstance, LPCWSTR lpszWndClassName, LPCWSTR lpszCopyFromWC, bool bUnregisterFirst)
{
    if (bUnregisterFirst) {
        UnregisterClass(lpszWndClassName, hInstance);
    }
    WNDCLASSEX wcx = { sizeof(WNDCLASSEX) };
    GetClassInfoEx(hInstance, lpszCopyFromWC, &wcx); // copy members

    //wcx.lpfnWndProc = (WNDPROC)TBWndProc; ~ don't do that
    wcx.hInstance = hInstance; // done already
    wcx.hCursor = LoadCursor(NULL, IDC_HAND);
    wcx.hbrBackground = UseDarkMode() ? Globals.hbrDarkModeBkgBrush : (HBRUSH)(COLOR_WINDOW + 1);
    wcx.lpszClassName = lpszWndClassName;

    return RegisterClassEx(&wcx);
}
#endif


//=============================================================================
//
//  InitInstance() - DarkMode already initialized !
//
HWND InitInstance(const HINSTANCE hInstance, int nCmdShow)
{
    // manual (not automatic) reset & initial state: not signaled (TRUE, FALSE)
    s_hEventAppIsClosing = CreateEvent(NULL, TRUE, FALSE, NULL);

    // init w/o hwnd
    g_IniWinInfo = GetWinInfoByFlag(NULL, Globals.CmdLnFlag_WindowPos);

    if (Settings2.LaunchInstanceFullVisible) {
        RECT rc = { 0 };
        RectFromWinInfo(&g_IniWinInfo, &rc);
        FitIntoMonitorGeometry(&rc, &g_IniWinInfo, SCR_NORMAL, false);
    }

    // get monitor coordinates from g_IniWinInfo
    WININFO srcninfo = g_IniWinInfo;

    WinInfoToScreenCoord(&srcninfo);

    Globals.hwndMain = NULL;

    // initialy hidden/not visible
    DWORD const dwStyle = ((WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN) & ~WS_VISIBLE); // | (g_IniWinInfo.max ? WS_MAXIMIZE : 0);

    HWND const hwndMain = CreateWindowEx(
        WS_EX_ACCEPTFILES,
        s_wchWndClass,
        _W(SAPPNAME),
        dwStyle,
        srcninfo.x,
        srcninfo.y,
        srcninfo.cx,
        srcninfo.cy,
        NULL,
        NULL,
        hInstance,
        NULL);

    InitWindowCommon(hwndMain, true);
    SetDialogIconNP3(hwndMain);

    //assert("Attach Debugger" && 0);

    // correct infos based on hwnd
    g_DefWinInfo = _GetDefaultWinInfoByStrg(hwndMain, Settings2.DefaultWindowPosition);
    s_WinCurrentWidth = g_IniWinInfo.cx;
    if (g_IniWinInfo.max) {
        nCmdShow = SW_SHOWMAXIMIZED;
    }

    // manual (not automatic) reset & initial state: not signaled (TRUE, FALSE)
    s_FileChgObsvrData.hEventFileChanged = CreateEvent(NULL, TRUE, FALSE, NULL);
    s_FileChgObsvrData.hEventFileDeleted = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (Settings.TransparentMode) {
        SetWindowTransparentMode(hwndMain, true, Settings2.OpacityLevel);
    }

    CreateBars(hwndMain, hInstance);

    SetMenu(hwndMain, (Settings.ShowMenubar ? Globals.hMainMenu : NULL));
    DrawMenuBar(hwndMain);

    Globals.hwndMain = hwndMain; // make main window globaly available

    HPATHL        hfile_pth = Path_Copy(s_pthArgFilePath);
    FileLoadFlags fLoadFlags = FLF_None;
    
    // Source Encoding
    Encoding_Forced(s_flagSetEncoding);

    switch (s_flagChangeNotify) {
    case FWM_DONT_CARE:
    case FWM_INDICATORSILENT:
    case FWM_MSGBOX:
    case FWM_AUTORELOAD:
    case FWM_EXCLUSIVELOCK:
        FileWatching.FileWatchingMode = s_flagChangeNotify;
        break;
    case FWM_NO_INIT:
    default:
        FileWatching.FileWatchingMode = Settings.FileWatchingMode;
        break;
    }

    // Restore saved Monitoring Log setting - fixes #5037
    FileWatching.MonitoringLog = Settings.MonitoringLog;

    // initial set text in front of ShowWindow()
    EditSetNewText(Globals.hwndEdit, "", 0, false, false);

    ShowWindowAsync(s_hwndEditFrame, SW_SHOWDEFAULT);
    ShowWindowAsync(Globals.hwndEdit, SW_SHOWDEFAULT);
    //~SnapToWinInfoPos(hwndMain, g_IniWinInfo, SCR_NORMAL, SW_HIDE); ~ instead set all needed properties  here:
    SetWindowPos(hwndMain, Settings.AlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    UpdateWindow(hwndMain);
    if (!Settings.ShowTitlebar) {
        SetWindowLong(hwndMain, GWL_STYLE, GetWindowLong(hwndMain, GWL_STYLE) & ~WS_CAPTION);
    }

    if (s_flagStartAsTrayIcon || (nCmdShow == SW_MINIMIZE) || (nCmdShow == SW_SHOWMINIMIZED)) {
        if (Settings.MinimizeToTray) {
            MinimizeWndToTray(hwndMain);
        }
        else {
            MinimizeWndToTaskbar(hwndMain);
        }
    }
    else {
        ShowWindow(hwndMain, nCmdShow);
    }

    bool bOpened = false;

    // Pathname parameter
    if (s_IsThisAnElevatedRelaunch || (Path_IsNotEmpty(s_pthArgFilePath) /*&& !g_flagNewFromClipboard*/))
    {
        fLoadFlags |= Settings.SkipUnicodeDetection ? FLF_SkipUnicodeDetect : 0;
        fLoadFlags |= Settings.SkipANSICodePageDetection ? FLF_SkipANSICPDetection : 0;

        // Open from Directory
        Path_CanonicalizeEx(hfile_pth, Paths.WorkingDirectory);

        if (!s_IsThisAnElevatedRelaunch && Path_IsExistingDirectory(hfile_pth)) {
            if (OpenFileDlg(Globals.hwndMain, hfile_pth, hfile_pth)) {
                bOpened = FileLoad(hfile_pth, fLoadFlags, 0, 0);
            }
        } else {
            HPATHL const hpthFileToOpen = s_IsThisAnElevatedRelaunch ? s_hpthRelaunchElevatedFile : hfile_pth;
            bOpened = FileLoad(hpthFileToOpen, fLoadFlags, 0, 0);
            if (bOpened) {
                if (s_IsThisAnElevatedRelaunch) {
                    if (Path_IsNotEmpty(hfile_pth)) {
                        Path_Reset(Paths.CurrentFile, Path_Get(hfile_pth));
                    } else {
                        Path_Empty(Paths.CurrentFile, false);
                    }
                    if (!s_flagLexerSpecified) {
                        Style_SetLexerFromFile(Globals.hwndEdit, Paths.CurrentFile);
                    }

                    // check for temp file and delete
                    if (s_IsThisAnElevatedRelaunch && Path_IsExistingFile(s_hpthRelaunchElevatedFile)) {
                        
                        DeleteFileW(Path_Get(s_hpthRelaunchElevatedFile));
                        
                        // delete possible .tmp guard
                        Path_RenameExtension(s_hpthRelaunchElevatedFile, L".tmp");
                        if (Path_IsExistingFile(s_hpthRelaunchElevatedFile)) {
                            DeleteFileW(Path_Get(s_hpthRelaunchElevatedFile));
                        }
                        SetSaveNeeded(true);
                    }

                    if (Path_IsNotEmpty(Paths.CurrentFile)) {
                        if (s_flagSaveOnRelaunch) {
                            FileSave(FSF_SaveAlways); // issued from elevation instances
                        }
                    }
                }
                else if (s_flagChangeNotify == FWM_AUTORELOAD) {
                    FileWatching.MonitoringLog = false; // will be reset in IDM_VIEW_CHASING_DOCTAIL
                    PostWMCommand(hwndMain, IDM_VIEW_CHASING_DOCTAIL);
                }
            }
        }

        Path_Empty(s_pthArgFilePath, false);

    } else {
        cpi_enc_t const forcedEncoding = Encoding_Forced(CPI_GET);
        if (Encoding_IsValid(forcedEncoding)) {
            Encoding_Current(forcedEncoding);
        }
    }

    if (!bOpened) {
        Path_Reset(hfile_pth, L"");
        fLoadFlags = FLF_DontSave | FLF_New | FLF_SkipUnicodeDetect | FLF_SkipANSICPDetection;
        FileLoad(hfile_pth, fLoadFlags, 0, 0); // init editor frame
    }

    // reset
    Encoding_Forced(CPI_NONE);
    s_flagQuietCreate = false;
    s_flagKeepTitleExcerpt = false;

    // undo / redo selections
    if (UndoRedoSelectionUTArray != NULL) {
        utarray_free(UndoRedoSelectionUTArray);
        UndoRedoSelectionUTArray = NULL;
    }
    utarray_new(UndoRedoSelectionUTArray, &UndoRedoSelection_icd);
    utarray_reserve(UndoRedoSelectionUTArray,256);

    // Check for /c [if no file is specified] -- even if a file is specified
    if (s_flagNewFromClipboard) {
        if (SciCall_CanPaste()) {
            bool bAutoIndent2 = Settings.AutoIndent;
            Settings.AutoIndent = 0;
            EditJumpTo(-1, 0);
            UndoTransActionBegin();
            if (!Sci_IsDocEmpty()) {
                SciCall_NewLine();
            }
            SciCall_Paste();
            SciCall_NewLine();
            EndUndoTransAction();
            Settings.AutoIndent = bAutoIndent2;
            if (s_flagJumpTo) {
                SciCall_SetYCaretPolicy(s_iCaretPolicyV | CARET_JUMPS, Settings2.CurrentLineVerticalSlop);
                EditJumpTo(s_iInitialLine, s_iInitialColumn);
                SciCall_SetYCaretPolicy(s_iCaretPolicyV, Settings2.CurrentLineVerticalSlop);
            } else {
                Sci_ScrollSelectionToView();
            }
        }
    }
    else {
        if (s_flagJumpTo) { // Jump to position
            SciCall_SetYCaretPolicy(s_iCaretPolicyV | CARET_JUMPS, Settings2.CurrentLineVerticalSlop);
            EditJumpTo(s_iInitialLine, s_iInitialColumn);
            SciCall_SetYCaretPolicy(s_iCaretPolicyV, Settings2.CurrentLineVerticalSlop);
        }
        else {
            Sci_ScrollSelectionToView();
        }
    }

    // Encoding
    if (s_flagSetEncoding != CPI_NONE) {
        SendMessage(Globals.hwndMain, WM_COMMAND, (WPARAM)MAKELONG(IDM_ENCODING_SELECT, IDM_ENCODING_SELECT + s_flagSetEncoding), 0);
        s_flagSetEncoding = CPI_NONE;
    }

    // EOL mode
    if (s_flagSetEOLMode != 0) {
        SendWMCommand(Globals.hwndMain, IDM_LINEENDINGS_CRLF + s_flagSetEOLMode - 1);
        s_flagSetEOLMode = 0;
    }

    // Match Text
    if (g_flagMatchText && !IsFindPatternEmpty()) {
        if (!Sci_IsDocEmpty()) {

            SetFindReplaceData(); // s_FindReplaceData

            if (g_flagMatchText & 2) {
                if (!s_flagJumpTo) {
                    Sci_SetCaretScrollDocEnd();
                }
                EditFindPrev(Globals.hwndEdit, &s_FindReplaceData, false, false, false);
            } else {
                if (!s_flagJumpTo) {
                    SciCall_DocumentStart();
                }
                EditFindNext(Globals.hwndEdit, &s_FindReplaceData, false, false, false);
            }
        }
    }

    // Check for Paste Board option -- after loading files
    if (s_flagPasteBoard) {
        s_bLastCopyFromMe = true;
        s_hwndNextCBChain = SetClipboardViewer(Globals.hwndMain);
        s_bLastCopyFromMe = false;

        s_iLastCopyTime = 0;
        SetTimer(Globals.hwndMain, ID_PASTEBOARDTIMER, 100, PasteBoardTimerProc);
    }

    // check if a lexer was specified from the command line
    if (s_flagLexerSpecified) {
        if (s_lpSchemeArg) {
            Style_SetLexerFromName(Globals.hwndEdit, Paths.CurrentFile, s_lpSchemeArg);
            LocalFree(s_lpSchemeArg);  // StrDup()
            s_lpSchemeArg = NULL;
        } else if ((s_iInitialLexer >= 0) && (s_iInitialLexer < Style_NumOfLexers())) {
            Style_SetLexerFromID(Globals.hwndEdit, s_iInitialLexer);
        }
        s_flagLexerSpecified = false;
    }

    // If start as tray icon, set current filename as tooltip
    if (s_flagStartAsTrayIcon) {
        SetNotifyIconTitle(Globals.hwndMain);
    }
    Globals.iSelectionMarkNumber = Globals.iMarkOccurrencesCount = Globals.iReplacedOccurrences = 0;

    ResetMouseDWellTime();

    // print file immediately and quit
    if (Globals.CmdLnFlag_PrintFileAndLeave) {
        WCHAR tchPageFmt[32] = { L'\0' };
        WCHAR szDisplayName[MAX_PATH_EXPLICIT>>1];

        GetLngString(IDS_MUI_UNTITLED, szDisplayName, COUNTOF(szDisplayName));
        Path_GetDisplayName(szDisplayName, COUNTOF(szDisplayName), Paths.CurrentFile, NULL, true);

        GetLngString(IDS_MUI_PRINT_PAGENUM, tchPageFmt, COUNTOF(tchPageFmt));

        if (!EditPrint(Globals.hwndEdit, szDisplayName, tchPageFmt)) {
            InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_PRINT_ERROR, szDisplayName);
        }
    }

    if (Globals.CmdLnFlag_PrintFileAndLeave || IsAppClosing()) {
        CloseApplication();
    }

    Path_Release(hfile_pth);

    s_bInitAppDone = true;

    return Globals.hwndMain;
}


//=============================================================================
//
//  MainWndProc()
//
//  Messages are distributed to the MsgXXX-handlers
//
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
    switch(umsg) {
    case WM_CREATE:
        return MsgCreate(hwnd, wParam, lParam);

    case WM_SETFOCUS:
        SetFocus(Globals.hwndEdit);
        break;

    case WM_CLOSE:
        SetEvent(s_hEventAppIsClosing);
        InstallFileWatching(false);
        if (IsIconic(hwnd)) {
            ShowWindowAsync(hwnd, SW_RESTORE);
        }
        if (FileSave(FSF_Ask)) {
            DestroyWindow(Globals.hwndMain);
        }
        else {
            ResetEvent(s_hEventAppIsClosing);
            InstallFileWatching(true);
        }
        break;

    case WM_QUERYENDSESSION:
        if (Settings.AutoSaveOptions & ASB_Shutdown) {
            AutoSaveDoWork(FSF_EndSession);
        }
        else if (FileSave(FSF_Ask | FSF_EndSession)) {
            return TRUE;
        }
        break;

	case WM_POWERBROADCAST:
        if (wParam == PBT_APMSUSPEND) {
            // we only have 2 seconds to save current file
            if (Settings.AutoSaveOptions & ASB_Suspend) {
                AutoSaveDoWork(FSF_None);
            }
        }
        break;

    case WM_DESTROY:
    case WM_ENDSESSION:
        return MsgEndSession(hwnd, umsg, wParam, lParam);

    case WM_SYSCOLORCHANGE:
        // update Scintilla colors
        SendMessage(Globals.hwndEdit, WM_SYSCOLORCHANGE, wParam, lParam);
        // [[FallThrough]]

    // Reinitialize theme-dependent values and resize windows
    case WM_THEMECHANGED:
        return MsgThemeChanged(hwnd, wParam, lParam);

    case WM_DPICHANGED:
        return MsgDPIChanged(hwnd, wParam, lParam);

    case WM_SIZE:
        return MsgSize(hwnd, wParam, lParam);

#ifdef D_NP3_WIN10_DARK_MODE
    case WM_SETTINGCHANGE: {
        if (IsColorSchemeChangeMessage(lParam)) {
            RefreshTitleBarThemeColor(hwnd);
            SendMessage(Globals.hwndEdit, WM_THEMECHANGED, 0, 0);
        }
        // Forward to Scintilla to refresh mouse scroll settings (issue #5223)
        SendMessage(Globals.hwndEdit, WM_SETTINGCHANGE, wParam, lParam);
    }
    break;
#endif

    case WM_DRAWITEM:
        return MsgDrawItem(hwnd, wParam, lParam);

    case WM_DROPFILES:
        // see SCN_URIDROPPED
        return MsgDropFiles(hwnd, wParam, lParam);

    case WM_COPYDATA:
        return MsgCopyData(hwnd, wParam, lParam);

    case WM_CONTEXTMENU:
    case WM_NCRBUTTONDOWN:
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

    //~case WM_KEYDOWN:
    //~    return MsgKeyDown(hwnd, wParam, lParam);

    case WM_FILECHANGEDNOTIFY:
        return MsgFileChangeNotify(hwnd, wParam, lParam);

    case WM_RESTORE_UNDOREDOACTION:
        _RestoreActionSelection((LONG)lParam, (DoAction)wParam);
        break;

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
        if (!s_bLastCopyFromMe) {
            s_iLastCopyTime = GetTicks_ms();
        } else {
            s_bLastCopyFromMe = false;
        }

        if (s_hwndNextCBChain) {
            SendMessage(s_hwndNextCBChain,WM_DRAWCLIPBOARD,wParam,lParam);
        }
        break;

    case WM_CHANGECBCHAIN:
        if ((HWND)wParam == s_hwndNextCBChain) {
            s_hwndNextCBChain = (HWND)lParam;
        }
        if (s_hwndNextCBChain) {
            SendMessage(s_hwndNextCBChain,WM_CHANGECBCHAIN,lParam,wParam);
        }
        break;

    case WM_SYSCOMMAND:
        return MsgSysCommand(hwnd, umsg, wParam, lParam);

    case WM_MBUTTONDOWN: {
        POINT const  cpt = POINTFromLParam(lParam);
        DocPos const pos = SciCall_CharPositionFromPointClose(cpt.x, cpt.y);
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
        } else if (wParam & MK_RBUTTON) {
            // Hold RIGHT MOUSE BUTTON and SCROLL to cycle through UNDO history
            if (GET_WHEEL_DELTA_WPARAM(wParam) > 0) {
                s_bUndoRedoScroll = true;
                //LimitNotifyEvents(EVM_UndoRedo);
                SciCall_Redo();
                //RestoreNotifyEvents();
            } else if (GET_WHEEL_DELTA_WPARAM(wParam) < 0) {
                s_bUndoRedoScroll = true;
                //LimitNotifyEvents(EVM_UndoRedo);
                SciCall_Undo();
                //RestoreNotifyEvents();
            }
        }
        break;

    case WM_INPUTLANGCHANGE:
        Globals.bIsCJKInputCodePage = IsDBCSCodePage(Scintilla_InputCodePage());
        break;

    case WM_UAHINITMENU:
    case WM_UAHDRAWMENU:
    case WM_UAHDRAWMENUITEM:
    case WM_UAHDESTROYWINDOW:
    case WM_UAHMEASUREMENUITEM:
    case WM_UAHNCPAINTMENUPOPUP: 
        return MsgUahMenuBar(hwnd, umsg, wParam, lParam);

    case WM_NCACTIVATE:
    case WM_NCPAINT: {
        LRESULT const res = DefWindowProc(hwnd, umsg, wParam, lParam);
        if (UseDarkMode()) {
            // handle dark menu bottom line
            MsgUahMenuBar(hwnd, umsg, wParam, lParam);
        }
        return res;
    }

    default:
        if (umsg == s_msgTaskbarCreated) {
            if (!IsWindowVisible(hwnd)) {
                ShowNotifyIcon(hwnd, true);
            }
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
    case 1:
        i = 1;
        break;
    case 2:
        i = 2;
        break;
    case 3:
        i = (Globals.fvCurFile.iIndentWidth) ? 1 * Globals.fvCurFile.iIndentWidth : 1 * Globals.fvCurFile.iTabWidth;
        break;
    case 4:
        i = (Globals.fvCurFile.iIndentWidth) ? 2 * Globals.fvCurFile.iIndentWidth : 2 * Globals.fvCurFile.iTabWidth;
        break;
    default:
        break;
    }
    SciCall_SetWrapStartIndent(i);
}


//=============================================================================
//
//  _SetWrapIndentMode()
//
static void  _SetWrapIndentMode()
{
    BeginWaitCursorUID(Flags.bHugeFileLoadState, IDS_MUI_SB_WRAP_LINES);

    Sci_SetWrapModeEx(GET_WRAP_MODE());

    if (Settings.WordWrapIndent == 5) {
        SciCall_SetWrapIndentMode(SC_WRAPINDENT_SAME);
    } else if (Settings.WordWrapIndent == 6) {
        SciCall_SetWrapIndentMode(SC_WRAPINDENT_INDENT);
    } else if (Settings.WordWrapIndent == 7) {
        SciCall_SetWrapIndentMode(SC_WRAPINDENT_DEEPINDENT);
    } else {
        _SetWrapStartIndent();
        SciCall_SetWrapIndentMode(SC_WRAPINDENT_FIXED);
    }
    
    EndWaitCursor();
}


//=============================================================================
//
//  _SetWrapVisualFlags()
//
static void  _SetWrapVisualFlags(HWND hwndEditCtrl)
{
    UNREFERENCED_PARAMETER(hwndEditCtrl);

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
        SciCall_SetWrapVisualFlags(wrapVisualFlags);
        SciCall_SetWrapVisualFlagsLocation(wrapVisualFlagsLocation);
    } else {
        SciCall_SetWrapVisualFlags(0);
    }
}


//=============================================================================
//
//  InitializeSciEditCtrl()
//
static void  _InitializeSciEditCtrl(HWND hwndEditCtrl)
{
    InitWindowCommon(hwndEditCtrl, true);

    SciCall_SetTechnology(Settings.RenderingTechnology);
    Settings.RenderingTechnology = SciCall_GetTechnology();
    SciCall_SetBidirectional(Settings.Bidirectional);  // experimental
    Settings.Bidirectional = SciCall_GetBidirectional();

    if (SciCall_SupportsFeature(SC_SUPPORTS_THREAD_SAFE_MEASURE_WIDTHS)) {
        SciCall_SetLayoutThreads(max_dw(2, GetNumberOfProcessors())); // MultiThreading Layout (SCI v5.2.0)
    }
    //~SciCall_SetPhasesDraw(SC_PHASES_TWO);   // (= default)
    SciCall_SetPhasesDraw(SC_PHASES_MULTIPLE);
    SciCall_SetLayoutCache(SC_CACHE_PAGE);     //~ SC_CACHE_DOCUMENT ~ beware of memory consumption !

    // Idle Styling (very large text)
    SciCall_SetIdleStyling(SC_IDLESTYLING_NONE); // needed for focused view

    SciCall_SetModEventMask(EVM_Default);
    SciCall_SetCommandEvents(false); // speedup folding

    SciCall_StyleSetCharacterSet(SC_CHARSET_DEFAULT);
    SciCall_SetCodePage(SC_CP_UTF8); // fixed internal UTF-8 (Sci:default)

    SciCall_SetMargins(NUMBER_OF_MARGINS);
    SciCall_SetMarginTypeN(MARGIN_SCI_LINENUM, SC_MARGIN_NUMBER);

    SciCall_SetMarginTypeN(MARGIN_SCI_BOOKMRK, SC_MARGIN_SYMBOL);
    SciCall_SetMarginMaskN(MARGIN_SCI_BOOKMRK, ~SC_MASK_FOLDERS & ~MARGIN_MARK_HISTORY_MASK);

    SciCall_SetMarginTypeN(MARGIN_SCI_CHGHIST, SC_MARGIN_SYMBOL);
    SciCall_SetMarginMaskN(MARGIN_SCI_CHGHIST, MARGIN_MARK_HISTORY_MASK);

    SciCall_SetMarginTypeN(MARGIN_SCI_FOLDING, SC_MARGIN_COLOUR);
    SciCall_SetMarginMaskN(MARGIN_SCI_FOLDING, SC_MASK_FOLDERS);

    //~SciCall_SetMarginLeft(1); ~ set by STYLE_INDENTGUIDE
    //~SciCall_SetMarginRight(1);

    SciCall_SetEOLMode(Settings.DefaultEOLMode);
    SciCall_SetPasteConvertEndings(true);
    SciCall_UsePopUp(SC_POPUP_TEXT);
    SciCall_SetScrollWidthTracking(true);
    // SciCall_SetScrollWidth(2000);

    SciCall_SetMultipleSelection(Settings.MultipleSelection);
    SciCall_SetMultiPaste(SC_MULTIPASTE_EACH); // paste into rectangular selection
    SciCall_SetAdditionalSelectionTyping(true);
    SciCall_SetMouseSelectionRectangularSwitch(true);
    SciCall_SetVirtualSpaceOptions(NP3_VIRTUAL_SPACE_ACCESS_OPTIONS);
    SciCall_AutoCSetMulti(SC_MULTIAUTOC_EACH);

    SciCall_SetAdditionalCaretsBlink(true);
    SciCall_SetAdditionalCaretsVisible(true);

    // assign command keys
    SciCall_AssignCmdKey(SCK_NEXT + (SCMOD_CTRL << 16), SCI_PARADOWN);
    SciCall_AssignCmdKey(SCK_PRIOR + (SCMOD_CTRL << 16), SCI_PARAUP);
    SciCall_AssignCmdKey(SCK_NEXT + ((SCMOD_CTRL | SCMOD_SHIFT) << 16), SCI_PARADOWNEXTEND);
    SciCall_AssignCmdKey(SCK_PRIOR + ((SCMOD_CTRL | SCMOD_SHIFT) << 16), SCI_PARAUPEXTEND);
    SciCall_AssignCmdKey(SCK_HOME + (0 << 16), SCI_VCHOMEWRAP);
    SciCall_AssignCmdKey(SCK_END + (0 << 16), SCI_LINEENDWRAP);
    SciCall_AssignCmdKey(SCK_HOME + (SCMOD_SHIFT << 16), SCI_VCHOMEWRAPEXTEND);
    SciCall_AssignCmdKey(SCK_END + (SCMOD_SHIFT << 16), SCI_LINEENDWRAPEXTEND);

    // set indicator styles (foreground and alpha maybe overridden by style settings)
    SciCall_IndicSetStyle(INDIC_NP3_MATCH_BRACE, INDIC_FULLBOX);
    SciCall_IndicSetFore(INDIC_NP3_MATCH_BRACE, RGB(0x00, 0xFF, 0x00));
    SciCall_IndicSetAlpha(INDIC_NP3_MATCH_BRACE, 120);
    SciCall_IndicSetOutlineAlpha(INDIC_NP3_MATCH_BRACE, 120);

    SciCall_IndicSetStyle(INDIC_NP3_BAD_BRACE, INDIC_FULLBOX);
    SciCall_IndicSetFore(INDIC_NP3_BAD_BRACE, RGB(0xFF, 0x00, 0x00));
    SciCall_IndicSetAlpha(INDIC_NP3_BAD_BRACE, 120);
    SciCall_IndicSetOutlineAlpha(INDIC_NP3_BAD_BRACE, 120);

    if (!Settings2.UseOldStyleBraceMatching) {
        SciCall_BraceHighLightIndicator(true, INDIC_NP3_MATCH_BRACE);
        SciCall_BraceBadLightIndicator(true, INDIC_NP3_BAD_BRACE);
    }

    SciCall_IndicSetStyle(INDIC_NP3_MARK_OCCURANCE, INDIC_ROUNDBOX);
    SciCall_IndicSetFore(INDIC_NP3_MARK_OCCURANCE, RGB(0x00, 0x00, 0xFF));
    SciCall_IndicSetAlpha(INDIC_NP3_MARK_OCCURANCE, 10);
    SciCall_IndicSetOutlineAlpha(INDIC_NP3_MARK_OCCURANCE, 10);

    SciCall_IndicSetStyle(INDIC_NP3_HYPERLINK, INDIC_TEXTFORE);
    SciCall_IndicSetFore(INDIC_NP3_HYPERLINK, RGB(0x00, 0x00, 0xA0));
    SciCall_IndicSetStyle(INDIC_NP3_HYPERLINK_U, INDIC_COMPOSITIONTHIN);
    SciCall_IndicSetFore(INDIC_NP3_HYPERLINK_U, RGB(0x00, 0x00, 0xA0));

    SciCall_IndicSetHoverStyle(INDIC_NP3_HYPERLINK, INDIC_TEXTFORE);
    SciCall_IndicSetHoverFore(INDIC_NP3_HYPERLINK, RGB(0x00, 0x00, 0xFF));
    SciCall_IndicSetHoverStyle(INDIC_NP3_HYPERLINK_U, INDIC_COMPOSITIONTHICK);
    SciCall_IndicSetHoverFore(INDIC_NP3_HYPERLINK_U, RGB(0x00, 0x00, 0xFF));

    SciCall_IndicSetStyle(INDIC_NP3_COLOR_DEF, INDIC_COMPOSITIONTHIN /*INDIC_HIDDEN*/); // MARKER only
    SciCall_IndicSetUnder(INDIC_NP3_COLOR_DEF, true);
    SciCall_IndicSetAlpha(INDIC_NP3_COLOR_DEF, SC_ALPHA_TRANSPARENT); // reset on hover
    SciCall_IndicSetOutlineAlpha(INDIC_NP3_COLOR_DEF, SC_ALPHA_OPAQUE);
    SciCall_IndicSetHoverStyle(INDIC_NP3_COLOR_DEF, INDIC_HIDDEN);         // initially hidden, INDIC_FULLBOX on hover
    SciCall_IndicSetHoverFore(INDIC_NP3_COLOR_DEF, RGB(0x00, 0x00, 0x00)); // recalc on hover

    SciCall_IndicSetStyle(INDIC_NP3_COLOR_DEF_T, INDIC_HIDDEN); // invisible
    SciCall_IndicSetHoverStyle(INDIC_NP3_COLOR_DEF_T, INDIC_HIDDEN); // initially hidden, INDIC_TEXTFORE on hover
    SciCall_IndicSetHoverFore(INDIC_NP3_COLOR_DEF_T, RGB(0x00, 0x00, 0x00));

    SciCall_IndicSetStyle(INDIC_NP3_UNICODE_POINT, INDIC_COMPOSITIONTHIN /*INDIC_HIDDEN*/);
    //SciCall_IndicSetUnder(INDIC_NP3_UNICODE_POINT, false);
    SciCall_IndicSetAlpha(INDIC_NP3_UNICODE_POINT, SC_ALPHA_TRANSPARENT);
    SciCall_IndicSetOutlineAlpha(INDIC_NP3_UNICODE_POINT, SC_ALPHA_OPAQUE);
    SciCall_IndicSetHoverStyle(INDIC_NP3_UNICODE_POINT, INDIC_ROUNDBOX);
    //SciCall_IndicSetHoverFore(INDIC_NP3_UNICODE_POINT, RGB(0xE0, 0xE0, 0xE0);

    SciCall_IndicSetStyle(INDIC_NP3_MULTI_EDIT, INDIC_ROUNDBOX);
    SciCall_IndicSetFore(INDIC_NP3_MULTI_EDIT, RGB(0xFF, 0xA5, 0x00));
    SciCall_IndicSetAlpha(INDIC_NP3_MULTI_EDIT, 60);
    SciCall_IndicSetOutlineAlpha(INDIC_NP3_MULTI_EDIT, 180);

    // No SC_AUTOMATICFOLD_CLICK, performed by
    SciCall_SetAutomaticFold(SC_AUTOMATICFOLD_SHOW | SC_AUTOMATICFOLD_CHANGE);

    // Properties
    SciCall_SetCaretSticky(SC_CARETSTICKY_OFF);
    //SciCall_SetCaretSticky(SC_CARETSTICKY_WHITESPACE);

    ResetMouseDWellTime();

    int const iCaretPolicy = CARET_SLOP | CARET_EVEN | CARET_STRICT;
    SciCall_SetXCaretPolicy(iCaretPolicy, Settings2.CurrentLineHorizontalSlop);
    s_iCaretPolicyV = (Settings2.CurrentLineVerticalSlop > 0) ? iCaretPolicy : CARET_EVEN;
    SciCall_SetXCaretPolicy(iCaretPolicy, Settings2.CurrentLineHorizontalSlop);
    SciCall_SetYCaretPolicy(s_iCaretPolicyV, Settings2.CurrentLineVerticalSlop);
    SciCall_SetVisiblePolicy(s_iCaretPolicyV, Settings2.CurrentLineVerticalSlop);
    SciCall_SetEndAtLastLine(!Settings.ScrollPastEOF);

    // Tabs
    SciCall_SetUseTabs(!Globals.fvCurFile.bTabsAsSpaces);
    SciCall_SetTabIndents(Globals.fvCurFile.bTabIndents);
    SciCall_SetBackSpaceUnIndents(Settings.BackspaceUnindents);
    SciCall_SetTabWidth(Globals.fvCurFile.iTabWidth);
    SciCall_SetIndent(Globals.fvCurFile.iIndentWidth);

    // Indent Guides
    Style_SetIndentGuides(hwndEditCtrl, Settings.ShowIndentGuides);

    // Word Wrap
    _SetWrapIndentMode(hwndEditCtrl);
    _SetWrapVisualFlags(hwndEditCtrl);

    // Long Lines
    if (Settings.MarkLongLines) {
        SciCall_SetEdgeMode((Settings.LongLineMode == EDGE_BACKGROUND) ? EDGE_BACKGROUND : EDGE_LINE);
    } else {
        SciCall_SetEdgeMode(EDGE_NONE);
    }
    SciCall_SetEdgeColumn(Settings.LongLinesLimit);

    // general margin
    SciCall_SetMarginOptions(Settings2.SubWrappedLineSelectOnMarginClick ? SC_MARGINOPTION_SUBLINESELECT : SC_MARGINOPTION_NONE);

    // Nonprinting characters
    SciCall_SetViewWS(Settings.ViewWhiteSpace ? SCWS_VISIBLEALWAYS : SCWS_INVISIBLE);
    SciCall_SetViewEOL(Settings.ViewEOLs);

    // IME Interaction
    SciCall_SetIMEInteraction(Settings2.IMEInteraction);

    // word delimiter handling
    EditInitWordDelimiter(hwndEditCtrl);
    EditReducedWordSeparatorSet(hwndEditCtrl, Settings.AccelWordNavigation);

    SciCall_ClearRegisteredImages();
    SciCall_AutoCSetOptions(SC_AUTOCOMPLETE_FIXED_SIZE);
    SciCall_AutoCSetIgnoreCase(true);
    //~SciCall_AutoCSetCaseInsensitiveBehaviour(SC_CASEINSENSITIVEBEHAVIOUR_IGNORECASE);
    SciCall_AutoCSetOrder(SC_ORDER_PRESORTED); // already sorted ~ SC_ORDER_PERFORMSORT
}


//=============================================================================
//
//  _EvalTinyExpr() - called on '?' or ENTER insert
//
static bool _EvalTinyExpr(bool qmark)
{
    if (!Settings.EvalTinyExprOnSelection) {
        return false;
    }

    DocPos const posSelStart = SciCall_GetSelectionStart();
    DocPos const posBegin = qmark ? SciCall_PositionBefore(posSelStart) : posSelStart;

    DocPos posBefore = SciCall_PositionBefore(posBegin);
    char chBefore = SciCall_GetCharAt(posBefore);

    while (IsBlankCharA(chBefore) && (posBefore > 0)) {
        posBefore = SciCall_PositionBefore(posBefore);
        chBefore = SciCall_GetCharAt(posBefore);
    }
    if (chBefore == '=') { // got "=?" or ENTER : evaluate expression trigger

        int const lineLen = (int)SciCall_LineLength(SciCall_LineFromPosition(posSelStart));
        char * const lineBuf = (char *)AllocMem((lineLen + 1), HEAP_ZERO_MEMORY);
        WCHAR * const lineBufW = (WCHAR *)AllocMem((lineLen + 1) * sizeof(WCHAR), HEAP_ZERO_MEMORY);
        if (lineBuf && lineBufW) {

            if (posSelStart < SciCall_GetCurrentPos()) {
                SciCall_SwapMainAnchorCaret();
            }
            DocPos const iLnCaretPos = SciCall_GetCurLine(lineLen, lineBuf);
            lineBuf[iLnCaretPos - (posSelStart - posBefore)] = '\0'; // exclude "=?"

            char const defchar = (char)te_invalid_chr();
            MultiByteToWideChar(Encoding_SciCP, 0, lineBuf, -1, lineBufW, (lineLen + 1));
            int const len = WideCharToMultiByte(te_cp(), (WC_COMPOSITECHECK | WC_DISCARDNS), lineBufW, -1, lineBuf, (lineLen + 1), &defchar, NULL);
            FreeMem(lineBufW);
            if (!len) {
                return false;
            }

            // canonicalize fetched line
            StrDelChrA(lineBuf, chr_currency);
            const char *p = lineBuf;
            while (IsBlankCharA(*p)) {
                ++p;
            }

            double dExprEval = 0.0;
            te_int_t exprErr = 1;
            while (*p && exprErr) {
                dExprEval = te_interp(p, &exprErr);
                // proceed to next possible expression
                while (*++p && exprErr && !(te_is_num(p) || te_is_op(p))) {}
            }
            FreeMem(lineBuf);

            if (!exprErr) {
                char chExpr[80] = { '\0' };
                TinyExprToStringA(chExpr, COUNTOF(chExpr), dExprEval);
                SciCall_ReplaceSel("");
                SciCall_SetSel(posBegin, posSelStart);
                SciCall_ReplaceSel(chExpr);
                return true;
            }
        }
    }
    return false;
}


//=============================================================================
//
//  _InitEditWndFrame()
//
static void _InitEditWndFrame()
{
    s_cxEditFrame = 0;
    s_cyEditFrame = 0;

    s_bIsAppThemed = IsAppThemed();

    InitWindowCommon(s_hwndEditFrame, true);

    if (s_bIsAppThemed) {

        SetWindowLongPtr(Globals.hwndEdit, GWL_EXSTYLE, GetWindowLongPtr(Globals.hwndEdit, GWL_EXSTYLE) & ~WS_EX_CLIENTEDGE);
        SetWindowPos(Globals.hwndEdit, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

        if (!IsWindowsVistaOrGreater()) {

            SetWindowPos(s_hwndEditFrame, Globals.hwndEdit, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
            ShowWindow(s_hwndEditFrame, SW_HIDE);

            RECT rc, rc2;
            GetClientRect(s_hwndEditFrame, &rc);
            GetWindowRect(s_hwndEditFrame, &rc2);
            s_cxEditFrame = ((rc2.right - rc2.left) - (rc.right - rc.left)) / 2;
            s_cyEditFrame = ((rc2.bottom - rc2.top) - (rc.bottom - rc.top)) / 2;
        }

    } else {

        SetWindowLongPtr(Globals.hwndEdit, GWL_EXSTYLE, WS_EX_CLIENTEDGE | GetWindowLongPtr(Globals.hwndEdit, GWL_EXSTYLE));
        SetWindowPos(Globals.hwndEdit, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
        ShowWindow(Globals.hwndEdit, SW_HIDE);
    }
}


static void _SetEnumWindowsItems(HWND hwnd)
{
    SetDlgItemText(hwnd, IDC_FILENAME, Path_Get(Paths.CurrentFile));
    SetDlgItemInt(hwnd, IDC_REUSELOCK, (UINT)GetTicks_ms(), false);
}


//=============================================================================
//
//  MsgCreate() - Handles WM_CREATE
//
//
LRESULT MsgCreate(HWND hwnd, WPARAM wParam,LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);

#ifdef D_NP3_WIN10_DARK_MODE
    if (IsDarkModeSupported()) {
        AllowDarkModeForWindowEx(hwnd, CheckDarkModeEnabled());
        RefreshTitleBarThemeColor(hwnd);
    }
#endif

    HINSTANCE const hInstance = ((LPCREATESTRUCT)lParam)->hInstance;

    // Setup edit control
    Globals.hwndEdit = CreateWindowEx(
                           WS_EX_CLIENTEDGE,
                           L"Scintilla",
                           NULL,
                           (WS_CHILD | WS_CLIPSIBLINGS) & ~WS_VISIBLE,
                           0, 0, 0, 0,
                           hwnd,
                           (HMENU)IDC_EDIT,
                           hInstance,
                           NULL);

    InitScintillaHandle(Globals.hwndEdit);

    _InitializeSciEditCtrl(Globals.hwndEdit);

    // Create Border Frame
    s_hwndEditFrame = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        WC_LISTVIEW,
        NULL,
        WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0, 0, 8, 8,
        hwnd,
        (HMENU)IDC_EDITFRAME,
        hInstance,
        NULL);

    _InitEditWndFrame();

    // Create Toolbar and Statusbar
    CreateBars(hwnd, hInstance);

    // Window Initialization

    (void)CreateWindow(
        WC_STATIC,
        NULL,
        WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0,0,10,10,
        hwnd,
        (HMENU)IDC_FILENAME,
        hInstance,
        NULL);

    (void)CreateWindow(
        WC_STATIC,
        NULL,
        WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        10,10,10,10,
        hwnd,
        (HMENU)IDC_REUSELOCK,
        hInstance,
        NULL);

    _SetEnumWindowsItems(hwnd);

    // Menu
    //~SetMenuDefaultItem(GetSubMenu(GetMenu(hwnd),0),0);

    // Drag & Drop
    DragAcceptFiles(hwnd,TRUE);

    if (Globals.hwndEdit == NULL || s_hwndEditFrame == NULL || Globals.hwndStatus == NULL || Globals.hwndToolbar == NULL || Globals.hwndRebar == NULL) {
        return -1LL;
    }

    //~ Style_SetDefaultLexer(Globals.hwndEdit); -- done by WM_THEMECHANGED

    ShowWindow(Globals.hwndEdit, SW_HIDE);

    Encoding_Current(Settings.DefaultEncoding);

    SciCall_SetZoom(g_IniWinInfo.zoom ? g_IniWinInfo.zoom : 100);

    return 0LL;
}


//=============================================================================
//
//  SelectExternalToolBar() - Select and Load an external Bitmap as ToolBarImage
//
bool SelectExternalToolBar(HWND hwnd)
{
    UNREFERENCED_PARAMETER(hwnd);

    HPATHL hfile_pth = Path_Allocate(NULL);
    Path_WriteAccessBuf(hfile_pth, CMDLN_LENGTH_LIMIT);

    WCHAR wchFilter[MIDSZ_BUFFER] = { L'\0' };
    GetLngString(IDS_MUI_FILTER_BITMAP, wchFilter, COUNTOF(wchFilter));
    PrepareFilterStr(wchFilter);

    OPENFILENAME ofn = { sizeof(OPENFILENAME) };
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = wchFilter;
    ofn.lpstrFile = Path_WriteAccessBuf(hfile_pth, 0);
    ofn.lpstrDefExt = L"bmp";
    ofn.nMaxFile = (DWORD)Path_GetBufCount(hfile_pth);
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT
                | OFN_PATHMUSTEXIST | OFN_SHAREAWARE | OFN_NODEREFERENCELINKS;

    if (GetOpenFileNameW(&ofn)) {
        Path_Sanitize(hfile_pth);
        Path_CanonicalizeEx(hfile_pth, Paths.ModuleDirectory);
        Path_Reset(g_tchToolbarBitmap, Path_Get(hfile_pth));
        Path_RelativeToApp(g_tchToolbarBitmap, true, true, true);
        if (Globals.bCanSaveIniFile) {
            IniFileSetString(Paths.IniFile, L"Toolbar Images", L"BitmapDefault", Path_Get(g_tchToolbarBitmap));
        }
    }

    bool res = false;

    if (Path_IsNotEmpty(hfile_pth)) {

        WCHAR strFileName[MAX_PATH_EXPLICIT] = { 0 };
        WCHAR strNewFileName[MAX_PATH_EXPLICIT] = { 0 };

        Path_RenameExtension(hfile_pth, NULL); // remove
        StringCchCopy(strFileName, COUNTOF(strFileName), Path_FindFileName(hfile_pth));
        Path_RemoveFileSpec(hfile_pth);

        if (Globals.bCanSaveIniFile) {
            StringCchCopy(strNewFileName, COUNTOF(strNewFileName), strFileName);
            StringCchCat(strNewFileName, COUNTOF(strNewFileName), L"Hot.bmp");
            HPATHL hfile_hot_pth = Path_Copy(hfile_pth);
            Path_Append(hfile_hot_pth, strNewFileName);
            if (Path_IsExistingFile(hfile_hot_pth)) {
                Path_Reset(g_tchToolbarBitmapHot, Path_Get(hfile_hot_pth));
                Path_RelativeToApp(g_tchToolbarBitmapHot, true, true, true);
                IniFileSetString(Paths.IniFile, L"Toolbar Images", L"BitmapHot", Path_Get(g_tchToolbarBitmapHot));
            } else {
                Path_Reset(g_tchToolbarBitmapHot, L"");
                IniFileDelete(Paths.IniFile, L"Toolbar Images", L"BitmapHot", false);
            }
            Path_Release(hfile_hot_pth);
        }

        if (Globals.bCanSaveIniFile) {
            StringCchCopy(strNewFileName, COUNTOF(strNewFileName), strFileName);
            StringCchCat(strNewFileName, COUNTOF(strNewFileName), L"Disabled.bmp");
            HPATHL hfile_dis_pth = Path_Copy(hfile_pth);
            Path_Append(hfile_dis_pth, strNewFileName);
            if (Path_IsExistingFile(hfile_dis_pth)) {
                Path_Reset(g_tchToolbarBitmapDisabled, Path_Get(hfile_dis_pth));
                Path_RelativeToApp(g_tchToolbarBitmapDisabled, true, true, true);
                IniFileSetString(Paths.IniFile, L"Toolbar Images", L"BitmapDisabled", Path_Get(g_tchToolbarBitmapDisabled));
            } else {
                Path_Reset(g_tchToolbarBitmapDisabled, L"");
                IniFileDelete(Paths.IniFile, L"Toolbar Images", L"BitmapDisabled", false);
            }
            Path_Release(hfile_dis_pth);
        }

        Settings.ToolBarTheme = 2;
        res = true;

    } else {
        IniFileDelete(Paths.IniFile, L"Toolbar Images", L"BitmapHot", false);
        IniFileDelete(Paths.IniFile, L"Toolbar Images", L"BitmapDisabled", false);
    }

    Path_Release(hfile_pth);

    return res;
}


//=============================================================================
//
//  LoadBitmapFile()
//
static HBITMAP LoadBitmapFile(const HPATHL hpath)
{
    HBITMAP hbmp = NULL;

    if (Path_IsExistingFile(hpath)) {

        hbmp = (HBITMAP)LoadImage(NULL, Path_Get(hpath), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);

        bool bDimOK = false;
        int  height = 16;
        if (hbmp) {
            BITMAP bmp = { 0 };
            GetObject(hbmp, sizeof(BITMAP), &bmp);
            height = bmp.bmHeight;
            bDimOK = (bmp.bmWidth >= (height * NUMTOOLBITMAPS));
        }
        if (!bDimOK) {
            InfoBoxLng(MB_ICONWARNING, L"NotSuitableToolbarDim", IDS_MUI_ERR_BITMAP, Path_Get(hpath),
                (height * NUMTOOLBITMAPS), height, NUMTOOLBITMAPS);
        }
    }
    else {
        WCHAR displayName[80];
        Path_GetDisplayName(displayName, 80, hpath, L"<unknown>", false);
        InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_ERR_LOADFILE, displayName);
    }

    return hbmp;
}


//=============================================================================
//
//  CreateScaledImageListFromBitmap()
//
static HIMAGELIST XXX_CreateScaledImageListFromBitmap(HWND hWnd, HBITMAP hBmp)
{
    BITMAP bmp = { 0 };
    GetObject(hBmp, sizeof(BITMAP), &bmp);

    int const mod = bmp.bmWidth % NUMTOOLBITMAPS;
    int const cx = (bmp.bmWidth - mod) / NUMTOOLBITMAPS;
    int const cy = bmp.bmHeight;

    HIMAGELIST himl = ImageList_Create(cx, cy, ILC_COLOR32 | ILC_MASK, NUMTOOLBITMAPS, NUMTOOLBITMAPS);
    ImageList_AddMasked(himl, hBmp, CLR_DEFAULT);

    UINT const dpi = Scintilla_GetWindowDPI(hWnd);
    if (!Settings.DpiScaleToolBar || (dpi == USER_DEFAULT_SCREEN_DPI)) {
        return himl; // default DPI, we are done
    }

    // Scale button icons/images
    int const scx = ScaleIntToDPI(hWnd, cx);
    int const scy = ScaleIntToDPI(hWnd, cy);

    HIMAGELIST hsciml = ImageList_Create(scx, scy, ILC_COLOR32 | ILC_MASK | ILC_HIGHQUALITYSCALE, NUMTOOLBITMAPS, NUMTOOLBITMAPS);

    for (int i = 0; i < NUMTOOLBITMAPS; ++i) {
        HICON const hicon = ImageList_GetIcon(himl, i, ILD_TRANSPARENT | ILD_PRESERVEALPHA | ILD_SCALE);
        ImageList_AddIcon(hsciml, hicon);
        DestroyIcon(hicon);
    }

    ImageList_Destroy(himl);

    return hsciml;
}


//=============================================================================
//
//  CreateScaledImageListFromBitmap()
//
static HIMAGELIST CreateScaledImageListFromBitmap(HWND hWnd, HBITMAP hBmp)
{
    BITMAP bmp = { 0 };
    GetObject(hBmp, sizeof(BITMAP), &bmp);

    int const numOfToolBitmaps = (int)(bmp.bmWidth / bmp.bmHeight);

    int const mod = bmp.bmWidth % numOfToolBitmaps;
    int const cx = (bmp.bmWidth - mod) / numOfToolBitmaps;
    int const cy = bmp.bmHeight;

    HIMAGELIST himl = ImageList_Create(cx, cy, ILC_COLOR32 | ILC_MASK, numOfToolBitmaps, numOfToolBitmaps);
    ImageList_AddMasked(himl, hBmp, CLR_DEFAULT);

    UINT const dpi = Scintilla_GetWindowDPI(hWnd);
    if (!Settings.DpiScaleToolBar || (dpi == USER_DEFAULT_SCREEN_DPI)) {
        return himl; // default DPI, we are done
    }

    // Scale button icons/images
    int const scx = ScaleIntToDPI(hWnd, cx);
    int const scy = ScaleIntToDPI(hWnd, cy);

    HIMAGELIST hsciml = ImageList_Create(scx, scy, ILC_COLOR32 | ILC_MASK | ILC_HIGHQUALITYSCALE, numOfToolBitmaps, numOfToolBitmaps);

    for (int i = 0; i < numOfToolBitmaps; ++i) {
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
        UINT const dpi = Scintilla_GetWindowDPI(hwnd);
        Settings.ToolBarTheme = (dpi < LargeIconDPI()) ? 0 : 1;
    }

    if (Globals.hwndToolbar) {
        HIMAGELIST himl = (HIMAGELIST)SendMessage(Globals.hwndToolbar, TB_GETIMAGELIST, 0, 0);
        if (himl) {
            ImageList_Destroy(himl);
        }
        himl = (HIMAGELIST)SendMessage(Globals.hwndToolbar, TB_GETHOTIMAGELIST, 0, 0);
        if (himl) {
            ImageList_Destroy(himl);
        }
        himl = (HIMAGELIST)SendMessage(Globals.hwndToolbar, TB_GETDISABLEDIMAGELIST, 0, 0);
        if (himl) {
            ImageList_Destroy(himl);
        }
        DestroyWindow(Globals.hwndToolbar);
    }

    OpenSettingsFile(__func__);
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

    InitWindowCommon(Globals.hwndToolbar, true); // (!) themed = true : glow effects

#ifdef D_NP3_WIN10_DARK_MODE
    if (IsDarkModeSupported()) {
        AllowDarkModeForWindowEx(Globals.hwndToolbar, CheckDarkModeEnabled());
    }
#endif

    SendMessage(Globals.hwndToolbar,TB_BUTTONSTRUCTSIZE,(WPARAM)sizeof(TBBUTTON),0);

    // -------------------------
    // Add Toolbar Bitmap
    // -------------------------
    HBITMAP hbmp = NULL;
    HBITMAP hbmpCopy = NULL;

    if ((Settings.ToolBarTheme == 2) && Path_IsNotEmpty(g_tchToolbarBitmap)) {
        HPATHL hfile_pth = Path_Copy(g_tchToolbarBitmap);
        Path_AbsoluteFromApp(hfile_pth, true);
        hbmp = LoadBitmapFile(hfile_pth);
        if (!hbmp) {
            Path_Reset(g_tchToolbarBitmap, L"");
            IniSectionDelete(L"Toolbar Images", L"BitmapDefault", false);
            bDirtyFlag = true;
        }
        Path_Release(hfile_pth);
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
    hbmp = NULL;
    if ((Settings.ToolBarTheme == 2) && Path_IsNotEmpty(g_tchToolbarBitmapHot)) {
        HPATHL hfile_pth = Path_Copy(g_tchToolbarBitmapHot);
        Path_AbsoluteFromApp(hfile_pth, true);
        hbmp = Path_IsExistingFile(hfile_pth) ? LoadBitmapFile(hfile_pth) : NULL;
        if (!hbmp) {
            Path_Reset(g_tchToolbarBitmapHot, L"");
            IniSectionDelete(L"Toolbar Images", L"BitmapHot", false);
            bDirtyFlag = true;
        }
        Path_Release(hfile_pth);
    }
    if (!hbmp && (Settings.ToolBarTheme < 2)) {
        LPCWSTR toolBarIntRes = (Settings.ToolBarTheme == 0) ? (LPCWSTR)MAKEINTRESOURCE(IDR_MAINWNDTBHOT) : (LPCWSTR)MAKEINTRESOURCE(IDR_MAINWNDTB2HOT);
        hbmp = LoadImage(hInstance, toolBarIntRes, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    }
    if (hbmp) {
        himl = CreateScaledImageListFromBitmap(hwnd, hbmp);
        DeleteObject(hbmp);
        hbmp = NULL;
        SendMessage(Globals.hwndToolbar, TB_SETHOTIMAGELIST, 0, (LPARAM)himl);
    } else { // clear the old one
        SendMessage(Globals.hwndToolbar, TB_SETHOTIMAGELIST, 0, 0);
    }


    // ------------------------------
    // Add a disabled Toolbar Bitmap
    // ------------------------------
    hbmp = NULL;
    if ((Settings.ToolBarTheme == 2) && Path_IsNotEmpty(g_tchToolbarBitmapDisabled)) {
        HPATHL hfile_pth = Path_Copy(g_tchToolbarBitmapDisabled);
        Path_AbsoluteFromApp(hfile_pth, true);
        hbmp = Path_IsExistingFile(hfile_pth) ? LoadBitmapFile(hfile_pth) : NULL;
        if (!hbmp) {
            Path_Reset(g_tchToolbarBitmapDisabled, L"");
            IniSectionDelete(L"Toolbar Images", L"BitmapDisabled", false);
            bDirtyFlag = true;
        }
    }
    if (!hbmp && (Settings.ToolBarTheme < 2)) {
        LPCWSTR toolBarIntRes = (Settings.ToolBarTheme == 0) ? (LPCWSTR)MAKEINTRESOURCE(IDR_MAINWNDTBDIS) : (LPCWSTR)MAKEINTRESOURCE(IDR_MAINWNDTB2DIS);
        hbmp = LoadImage(hInstance, toolBarIntRes, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    }
    if (hbmp) {
        himl = CreateScaledImageListFromBitmap(hwnd, hbmp);
        DeleteObject(hbmp);
        hbmp = NULL;
        SendMessage(Globals.hwndToolbar, TB_SETDISABLEDIMAGELIST, 0, (LPARAM)himl);

    } else { // create disabled Toolbar, no external bitmap is supplied

        if ((Settings.ToolBarTheme == 2) && !Path_IsNotEmpty(g_tchToolbarBitmapDisabled)) {
            bool bProcessed = false;
            if (Flags.ToolbarLook == 1) {
                bProcessed = BitmapAlphaBlend(hbmpCopy, GetSysColor(COLOR_3DFACE), 0x60);
            } else if (Flags.ToolbarLook == 2 || (!IsWindowsXPSP3OrGreater() && Flags.ToolbarLook == 0)) {
                bProcessed = BitmapGrayScale(hbmpCopy);
            }
            if (bProcessed && !IsWindowsXPSP3OrGreater()) {
                BitmapMergeAlpha(hbmpCopy, GetSysColor(COLOR_3DFACE));
            }
            if (bProcessed) {
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
    WCHAR tchIndex[16] = { L'\0' };
    WCHAR tchDesc[80] = { L'\0' };
    for (int i = 0; i < COUNTOF(s_tbbMainWnd); ++i) {

        if (s_tbbMainWnd[i].fsStyle == BTNS_SEP) {
            continue;
        }

        StringCchPrintf(tchIndex, COUNTOF(tchIndex), L"%02i", s_tbbMainWnd[i].iBitmap + 1);

        if (IniSectionGetString(L"Toolbar Labels", tchIndex, L"", tchDesc, COUNTOF(tchDesc)) > 0) {
            s_tbbMainWnd[i].iString = SendMessage(Globals.hwndToolbar, TB_ADDSTRING, 0, (LPARAM)tchDesc);
            s_tbbMainWnd[i].fsStyle |= BTNS_AUTOSIZE | BTNS_SHOWTEXT;
        } else {
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

    CloseSettingsFile(__func__, bDirtyFlag);

    // ------------------------------
    // Create ReBar and add Toolbar
    // ------------------------------
    if (Globals.hwndRebar) {
        DestroyWindow(Globals.hwndRebar);
    }
    Globals.hwndRebar = CreateWindowEx(WS_EX_TOOLWINDOW, REBARCLASSNAME, NULL, NP3_WS_REBAR,
                                       0,0,0,0,hwnd,(HMENU)IDC_REBAR,hInstance,NULL);

    // Theme = false (!) ~ you cannot change a toolbar's color when a visual style is active
    InitWindowCommon(Globals.hwndRebar, !(IsWindows10OrGreater() && IsDarkModeSupported()));

#ifdef D_NP3_WIN10_DARK_MODE
    if (IsDarkModeSupported()) {
        AllowDarkModeForWindowEx(Globals.hwndRebar, CheckDarkModeEnabled());
    }
#endif

    REBARINFO rbi = { sizeof(REBARINFO) };
    //rbi.fMask  = 0;
    rbi.himl   = (HIMAGELIST)NULL;
    SendMessage(Globals.hwndRebar, RB_SETBARINFO, 0, (LPARAM)&rbi);

    RECT rc = { 0, 0, 0, 0 };
    SendMessage(Globals.hwndToolbar, TB_GETITEMRECT, 0, (LPARAM)&rc);
    //SendMessage(Globals.hwndToolbar,TB_SETINDENT,2,0);

    REBARBANDINFO rbBand = { sizeof(REBARBANDINFO) };
    rbBand.fMask = RBBIM_COLORS /*| RBBIM_TEXT | RBBIM_BACKGROUND */ |
                   RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE /*| RBBIM_SIZE*/;
    //rbBand.fStyle  = /*RBBS_CHILDEDGE |*//* RBBS_BREAK |*/ RBBS_FIXEDSIZE /*| RBBS_GRIPPERALWAYS*/;
    rbBand.fStyle = s_bIsAppThemed ? (RBBS_FIXEDSIZE | RBBS_CHILDEDGE) : RBBS_FIXEDSIZE;
    rbBand.hbmBack = NULL;
    rbBand.lpText  = L"Toolbar";
    rbBand.clrFore = GetModeTextColor(UseDarkMode());
    rbBand.clrBack = IsWindows10OrGreater() ? GetModeBkColor(UseDarkMode()) : GetModeBtnfaceColor(UseDarkMode());
    rbBand.hwndChild  = Globals.hwndToolbar;
    rbBand.cxMinChild = (rc.right - rc.left) * COUNTOF(s_tbbMainWnd);
    rbBand.cyMinChild = (rc.bottom - rc.top) + (2 * rc.top);
    rbBand.cx         = 0;
    SendMessage(Globals.hwndRebar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand);

    SetWindowPos(Globals.hwndRebar, NULL, 0, 0, 0, 0, SWP_NOZORDER);
    GetWindowRect(Globals.hwndRebar, &rc);
    s_cyReBar = (rc.bottom - rc.top);
    s_cyReBarFrame = s_bIsAppThemed ? 0 : 2;  // (!) frame color is same as INITIAL title-bar ???

    ShowWindow(Globals.hwndRebar, Settings.ShowToolbar ? SW_SHOWDEFAULT : SW_HIDE);

    // -------------------
    // Create Statusbar
    // -------------------
    DWORD const dwStatusbarStyle = SBT_NOBORDERS | SBT_OWNERDRAW | WS_CHILD | WS_CLIPSIBLINGS;

    if (Globals.hwndStatus) {
        DestroyWindow(Globals.hwndStatus);
    }

    //~Globals.hwndStatus = CreateStatusWindow(dwStatusbarStyle, NULL, hwnd, IDC_STATUSBAR);
    Globals.hwndStatus = CreateWindowEx(
        WS_EX_COMPOSITED,     // => double-buffering avoids flickering
        STATUSCLASSNAME,      // name of status bar class
        (PCTSTR)NULL,         // no text when first created
        dwStatusbarStyle,     // creates a visible child window
        0, 0, 0, 0,           // ignores size and position
        hwnd,                 // handle to parent window
        (HMENU)IDC_STATUSBAR, // child window identifier
        hInstance,            // handle to application instance
        NULL);                // no window creation data

    InitWindowCommon(Globals.hwndStatus, true); // (!) themed = true : resize grip

    // no simple status bar, to allow owner draw for dark mode
    if (SendMessage(Globals.hwndStatus, SB_ISSIMPLE, 0, 0)) {
        SendMessage(Globals.hwndStatus, SB_SIMPLE, FALSE, 0);
    }

#ifdef D_NP3_WIN10_DARK_MODE
    if (IsDarkModeSupported()) {
        AllowDarkModeForWindowEx(Globals.hwndStatus, CheckDarkModeEnabled());
    }
    //~HDC const hdc = GetDC(Globals.hwndStatus);
    //~SetBkColor(hdc, GetModeBtnfaceColor(UseDarkMode()));
    //~SetTextColor(hdc, GetModeTextColor(UseDarkMode()));
    //~ReleaseDC(Globals.hwndStatus, hdc);
#endif

    ShowWindow(Globals.hwndStatus, Settings.ShowStatusbar ? SW_SHOWDEFAULT : SW_HIDE);
}


//=============================================================================
//
//  MsgEndSession() - Handle WM_ENDSESSION,WM_DESTROY
//
LRESULT MsgEndSession(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    static bool bShutdownOK = false;

    if (!bShutdownOK) {

        // Terminate file watching
        InstallFileWatching(false);

        RestorePrevScreenPos(hwnd);

        // Terminate AutoSave
        AutoSaveStop();

        DragAcceptFiles(hwnd, FALSE);

        // Terminate clipboard watching
        if (s_flagPasteBoard) {
            KillTimer(hwnd, ID_PASTEBOARDTIMER);
            ChangeClipboardChain(hwnd, s_hwndNextCBChain);
        }

        // close Find/Replace and CustomizeSchemes
        CloseNonModalDialogs();

        // call SaveAllSettings() when Globals.hwndToolbar is still valid
        SaveAllSettings(false);

        // Remove tray icon in any case
        ShowNotifyIcon(hwnd, false);

        //if (IS_VALID_HANDLE(s_hEvent)) {
        //    CloseHandle(s_hEvent);
        //}

        bShutdownOK = true;
    }

    assert(!IsIniFileCached());

    if (WM_DESTROY == umsg) {
        if (IS_VALID_HANDLE(s_hEventAppIsClosing)) {
            CloseHandle(s_hEventAppIsClosing);
        }
        PostQuitMessage(0);
    }

    return FALSE;
}


//=============================================================================
//
// MsgDPIChanged() - Handle WM_DPICHANGED
//
LRESULT MsgDPIChanged(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    UINT const dpi = LOWORD(wParam);
    DbgLog(L"MsgDPIChanged() -> %u\n", dpi);

    const RECT* const rc = (RECT*)lParam;

    DocPos const pos = SciCall_GetCurrentPos();

    UpdateWindowLayoutForDPI(hwnd, rc, dpi);

    SendMessage(Globals.hwndEdit, WM_DPICHANGED, wParam, lParam);

    MsgThemeChanged(hwnd, wParam, lParam);

    Sci_GotoPosChooseCaret(pos);

    return TRUE;
}


//=============================================================================
//
//  MsgThemeChanged() - Handle WM_THEMECHANGED
//
LRESULT MsgThemeChanged(HWND hwnd, WPARAM wParam,LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(wParam);

#ifdef D_NP3_WIN10_DARK_MODE
    AllowDarkModeForWindowEx(hwnd, UseDarkMode());
#endif

    if (Globals.hwndMain) {

#ifdef D_NP3_WIN10_DARK_MODE
        RefreshTitleBarThemeColor(hwnd);
#endif

        // reinitialize edit frame
        _InitEditWndFrame();

        // recreate toolbar and statusbar
        CreateBars(hwnd, Globals.hInstance);

        Style_ResetCurrentLexer(Globals.hwndEdit);

        Sci_RedrawScrollbars();

        SetMenu(hwnd, (Settings.ShowMenubar ? Globals.hMainMenu : NULL));
        DrawMenuBar(hwnd);

        if (FocusedView.HideNonMatchedLines) {
            EditToggleView(Globals.hwndEdit);
        }

        MarkAllOccurrences(_MQ_FAST, false);

        SciCall_StartStyling(0);
        Sci_ColouriseAll();

        EditUpdateVisibleIndicators();

        UpdateUI(hwnd);
    }

    UpdateWindowEx(hwnd);

    return FALSE;
}


//=============================================================================
//
//  MsgSize() - Handles WM_SIZE
//
//
LRESULT MsgSize(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    //switch (wParam) {
    //case SIZE_MINIMIZED:
    //    return FALSE;
    //case SIZE_MAXIMIZED:
    //case SIZE_RESTORED:
    //    break;
    //default:
    //    break;
    //}

    int x = 0;
    int y = 0;
    int cx = GET_X_LPARAM(lParam);
    int cy = GET_Y_LPARAM(lParam);

    if (Settings.ShowToolbar) {
        //~SendMessage(Globals.hwndToolbar,WM_SIZE,0,0);
        //~RECT rc;
        //~GetWindowRect(Globals.hwndToolbar,&rc);
        //~y = (rc.bottom - rc.top);
        //~cy -= (rc.bottom - rc.top);

        //~SendMessage(Globals.hwndToolbar,TB_GETITEMRECT,0,(LPARAM)&rc);

        SetWindowPos(Globals.hwndRebar, NULL, 0, 0, LOWORD(lParam), s_cyReBar, SWP_NOZORDER);

        // the ReBar automatically sets the correct height
        // calling SetWindowPos() with the height of one toolbar button
        // causes the control not to temporarily use the whole client area
        // and prevents flickering

        //GetWindowRect(Globals.hwndRebar,&rc);
        y = s_cyReBar + s_cyReBarFrame;    // define
        cy -= s_cyReBar + s_cyReBarFrame;  // border
    }

    if (Settings.ShowStatusbar) {
        SendMessage(Globals.hwndStatus,WM_SIZE,0,0);
        RECT rc;
        GetWindowRect(Globals.hwndStatus, &rc);
        cy -= (rc.bottom - rc.top);
    }


    HDWP const hdwp = BeginDeferWindowPos(2);

    DeferWindowPos(hdwp,s_hwndEditFrame,NULL,x,y,cx,cy, SWP_NOZORDER | SWP_NOACTIVATE);

    DeferWindowPos(hdwp, g_hwndEditWindow, s_hwndEditFrame,
                   x+s_cxEditFrame,y+s_cyEditFrame, cx-2*s_cxEditFrame,cy-2*s_cyEditFrame,
                   SWP_NOZORDER | SWP_NOACTIVATE);

    EndDeferWindowPos(hdwp);

    s_WinCurrentWidth = cx;

    UpdateToolbar_Now(hwnd);
    UpdateMargins(true);
    //~UpdateUI(); //~ recursion
    
    return FALSE;
}


//=============================================================================
//
//  UpdateContentArea()
//
void UpdateContentArea()
{
    Sci_ForceNotifyUpdateUI(Globals.hwndMain, IDC_EDIT);
}


//=============================================================================
//
//  MsgDrawItem() - Handles WM_DRAWITEM  (needs SBT_OWNERDRAW)
//
//  https://docs.microsoft.com/en-us/windows/win32/controls/status-bars#owner-drawn-status-bars
//
LRESULT MsgDrawItem(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(hwnd);

    if (LOWORD(wParam) == IDC_STATUSBAR) { // Statusbar SB_SETTEXT caused parent's WM_DRAWITEM message
        const DRAWITEMSTRUCT* const pDIS = (const DRAWITEMSTRUCT* const)lParam;

        int const partId = (int)pDIS->itemID;
        if (partId == -1) {
            return FALSE;
        }

        HDC const hdc = pDIS->hDC;
        RECT const rc = pDIS->rcItem;

        //UINT const ctlId = pDIS->CtlID; // child window identifier
        //~int const stateId = (int)pDIS->itemState ~ don't use

        //~PAINTSTRUCT ps;
        //~BeginPaint(hWndItem, &ps); ~ not needed on WM_DRAWITEM

        //~SetModeBkColor(hdc, UseDarkMode());
        SetModeBtnFaceColor(hdc, UseDarkMode());
        SetModeTextColor(hdc, UseDarkMode());

#ifdef D_NP3_WIN10_DARK_MODE

        if (UseDarkMode()) {
            // overpaint part frames
            HWND const hWndItem = pDIS->hwndItem;
            int const bdh = GetSystemMetrics(SM_CYFRAME);
            HDC const hdcFrm = GetWindowDC(hWndItem);
            RECT rcf = rc;
            for (int i = 1; i < bdh; ++i) {
                FrameRect(hdcFrm, &rcf, Globals.hbrDarkModeBtnFcBrush);
                rcf.left -= 1;
                rcf.top -= 1;
                rcf.bottom += 1;
                rcf.right += 1;
            }
            FrameRect(hdcFrm, &rcf, GetSysColorBrush(COLOR_3DDKSHADOW));
            ReleaseDC(hWndItem, hdcFrm);
        }

#endif

        LPCWSTR const text = (LPCWSTR)(pDIS->itemData);
        ExtTextOut(hdc, rc.left + 1, rc.top + 1, ETO_OPAQUE | ETO_NUMERICSLOCAL, &rc, text, lstrlen(text), NULL);

        //~EndPaint(hWndItem, &ps);
        return TRUE;
    }
    return FALSE;
}


//=============================================================================
//
//  _OnDropOneFile()
//
static LRESULT _OnDropOneFile(HWND hwnd, HPATHL hFilePath, WININFO* wi)
{
    if (IsIconic(hwnd)) {
        ShowWindow(hwnd, SW_RESTORE);
    }

    FileLoadFlags fLoadFlags = FLF_None;
    fLoadFlags |= Settings.SkipUnicodeDetection ? FLF_SkipUnicodeDetect : 0;
    fLoadFlags |= Settings.SkipANSICodePageDetection ? FLF_SkipANSICPDetection : 0;

    if (Path_IsExistingDirectory(hFilePath)) {
        if (OpenFileDlg(Globals.hwndMain, hFilePath, hFilePath)) {
            FileLoad(hFilePath, fLoadFlags, 0, 0);
        }
    }
    else if (Path_IsExistingFile(hFilePath)) {
        //~ ignore Flags.bReuseWindow
        bool const sameFile = (Path_StrgComparePath(hFilePath, Paths.CurrentFile, Paths.ModuleDirectory, true) == 0);
        if (IsKeyDown(VK_CONTROL) || wi) {
            DialogNewWindow(hwnd, sameFile, hFilePath, wi);
        } else {
            FileLoad(hFilePath, fLoadFlags, 0, 0);
        }
    } else {
        // Windows Bug: wParam (HDROP) pointer is corrupted if dropped from 32-bit App
        InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_DROP_NO_FILE);
    }
    return FALSE;
}


//=============================================================================
//
//  MsgDropFiles() - Handles WM_DROPFILES
//
LRESULT MsgDropFiles(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    HDROP hDrop = NULL;
    if (IsWindows10OrGreater()) {
        hDrop = (HDROP)wParam;
    }
    else // Windows7 Bug drag&drop of files from 32bit app to 64bit app
    {
    #ifdef _WIN64
        HANDLE hProcessHeap = GetProcessHeap();
        if (NULL != hProcessHeap && HeapLock(hProcessHeap)) {
            PROCESS_HEAP_ENTRY heapEntry = { 0 };
            while (HeapWalk(hProcessHeap, &heapEntry) != FALSE) {
                if ((heapEntry.wFlags & PROCESS_HEAP_ENTRY_BUSY) != 0) {
                    HGLOBAL hGlobal = GlobalHandle(heapEntry.lpData);
                    // Assuming wParam is the WM_DROPFILES WPARAM
                    if ((((DWORD_PTR)hGlobal) & 0xFFFFFFFF) == (wParam & 0xFFFFFFFF)) {
                        hDrop = (HDROP)hGlobal; // We got it !!
                        break;
                    }
                }
            }
            HeapUnlock(hProcessHeap);
        }
    #else
        hDrop = (HDROP)wParam;
    #endif
    }

    if (hDrop) {

        bool const vkCtrlDown = IsKeyDown(VK_CONTROL);

        HPATHL         hdrop_pth = Path_Allocate(NULL);
        wchar_t* const drop_buf = Path_WriteAccessBuf(hdrop_pth, STRINGW_MAX_URL_LENGTH);
        UINT const     cnt = DragQueryFileW(hDrop, UINT_MAX, NULL, 0);

        int const offset = Settings2.LaunchInstanceWndPosOffset;
        bool const bFullVisible = Settings2.LaunchInstanceFullVisible;
        for (UINT i = 0; i < cnt; ++i) {
            WININFO wi = GetMyWindowPlacement(hwnd, NULL, (vkCtrlDown ? (offset * (i + 1)) : 0), bFullVisible);
            DragQueryFileW(hDrop, i, drop_buf, (UINT)Path_GetBufCount(hdrop_pth));
            Path_Sanitize(hdrop_pth);
            _OnDropOneFile(hwnd, hdrop_pth, (((0 == i) && !IsKeyDown(VK_CONTROL)) ? NULL : &wi));
        }

        DragFinish(hDrop);
        Path_Release(hdrop_pth);
        UpdateToolbar_Now(hwnd);
    }
    return 0;
}

#if 0
//=============================================================================
//
//  DropFilesProc() - Handles DROPFILES
//
//
static DWORD DropFilesProc(CLIPFORMAT cf, HGLOBAL hData, HWND hWnd, DWORD dwKeyState, POINTL pt, void *pUserData)
{
    DWORD dwEffect = DROPEFFECT_NONE;

    //HWND hEditWnd = (HWND)pUserData;
    UNREFERENCED_PARAMETER(pUserData);

    if (cf == CF_HDROP) {
        WCHAR szBuf[MAX_PATH_EXPLICIT + 40];
        HDROP hDrop = (HDROP)hData;

        if (IsIconic(hWnd)) {
            ShowWindow(hWnd, SW_RESTORE);
        }

        DragQueryFile(hDrop, 0, szBuf, COUNTOF(szBuf));

        FileLoadFlags fLoadFlags = FLF_None;
        fLoadFlags |= Settings.SkipUnicodeDetection ? FLF_SkipUnicodeDetect : 0;
        fLoadFlags |= Settings.SkipANSICodePageDetection ? FLF_SkipANSICPDetection : 0;

        if (PathIsDirectory(szBuf)) {
            WCHAR tchFile[MAX_PATH_EXPLICIT] = { L'\0' };
            if (OpenFileDlg(hWnd, tchFile, COUNTOF(tchFile), szBuf)) {
                FileLoad(tchFile, fLoadFlags, 0, 0);
            }
        } else {
            FileLoad(szBuf, fLoadFlags, 0, 0);
        }

        if (DragQueryFile(hDrop, (UINT)(-1), NULL, 0) > 1) {
            InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_ERR_DROP);
        }
        dwEffect = DROPEFFECT_COPY;
    }

    UNREFERENCED_PARAMETER(dwKeyState);
    UNREFERENCED_PARAMETER(pt);

    return dwEffect;
}
#endif


//=============================================================================
//
//  MsgCopyData() - Handles WM_COPYDATA
//
//
LRESULT MsgCopyData(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);

    PCOPYDATASTRUCT pcds = (PCOPYDATASTRUCT)lParam;

    // Reset Change Notify
    //bPendingChangeNotify = false;

    SetDlgItemInt(hwnd, IDC_REUSELOCK, (UINT)GetTicks_ms(), false);

    if (pcds->dwData == DATA_NOTEPAD3_PARAMS) {
        LPnp3params const params = AllocMem(pcds->cbData, HEAP_ZERO_MEMORY);
        if (params) {

            CopyMemory(params, pcds->lpData, pcds->cbData);

            HPATHL hfile_pth = Path_Allocate(&params->wchData);

            if (params->flagLexerSpecified) {
                s_flagLexerSpecified = true;
            }
            if (params->flagQuietCreate) {
                s_flagQuietCreate = true;
            }
            if (params->flagFileSpecified) {

                bool bOpened = false;
                Encoding_Forced(params->flagSetEncoding);

                FileLoadFlags fLoadFlags = FLF_None;
                fLoadFlags |= Settings.SkipUnicodeDetection ? FLF_SkipUnicodeDetect : 0;
                fLoadFlags |= Settings.SkipANSICodePageDetection ? FLF_SkipANSICPDetection : 0;

                if (Path_IsExistingDirectory(hfile_pth)) {
                    if (OpenFileDlg(Globals.hwndMain, hfile_pth, hfile_pth)) {
                        bOpened = FileLoad(hfile_pth, fLoadFlags, 0, 0);
                    }
                } else {
                    bOpened = FileLoad(hfile_pth, fLoadFlags, 0, 0);
                }
                if (bOpened) {
                    if (params->flagChangeNotify == FWM_MSGBOX) {
                        FileWatching.FileWatchingMode = FWM_MSGBOX;
                        InstallFileWatching(true);
                    }
                    else if (params->flagChangeNotify == FWM_AUTORELOAD) {
                        if (FileWatching.MonitoringLog) {
                            PostWMCommand(Globals.hwndMain, IDM_VIEW_CHASING_DOCTAIL);
                        }
                        else {
                            FileWatching.FileWatchingMode = FWM_AUTORELOAD;
                        }
                        InstallFileWatching(true);
                    }
                    else if (params->flagChangeNotify == FWM_INDICATORSILENT) {
                        InstallFileWatching(true);
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
                            StringCchCopyN(CharNextW(wchExt), 32, StrEnd(Path_Get(hfile_pth), 0) + 1, 31);
                            Style_SetLexerFromName(Globals.hwndEdit, hfile_pth, wchExt);
                        } else if (params->iInitialLexer >= 0 && params->iInitialLexer < Style_NumOfLexers()) {
                            Style_SetLexerFromID(Globals.hwndEdit, params->iInitialLexer);
                        }
                    }

                    if (params->flagTitleExcerpt) {
                        StringCchCopyN(s_wchTitleExcerpt, COUNTOF(s_wchTitleExcerpt), StrEnd(Path_Get(hfile_pth), 0) + 1, COUNTOF(s_wchTitleExcerpt));
                    }
                }
                // reset
                Encoding_Forced(CPI_NONE);
            }

            if (params->flagJumpTo) {
                s_flagJumpTo = true;
                SciCall_SetYCaretPolicy(s_iCaretPolicyV | CARET_JUMPS, Settings2.CurrentLineVerticalSlop);
                EditJumpTo(params->iInitialLine, params->iInitialColumn);
                SciCall_SetYCaretPolicy(s_iCaretPolicyV, Settings2.CurrentLineVerticalSlop);
            }

            if (params->flagMatchText) {
                g_flagMatchText = params->flagMatchText;

                SetFindPattern(StrEndW(&params->wchData, 0) + 1);
                SetFindReplaceData(); // s_FindReplaceData

                if (g_flagMatchText & 2) {
                    if (!s_flagJumpTo) {
                        Sci_SetCaretScrollDocEnd();
                    }
                    EditFindPrev(Globals.hwndEdit, &s_FindReplaceData, false, false, false);
                } else {
                    if (!s_flagJumpTo) {
                        SciCall_DocumentStart();
                    }
                    EditFindNext(Globals.hwndEdit, &s_FindReplaceData, false, false, false);
                }
            }

            s_flagLexerSpecified = false;
            s_flagQuietCreate = false;

            Path_Release(hfile_pth);
            FreeMem(params);
        }

        UpdateToolbar_Now(hwnd);
        UpdateMargins(true);
    }

    return FALSE;
}

//=============================================================================
//
//  MsgContextMenu() - Handles WM_CONTEXTMENU and SCN_MARGINRIGHTCLICK
//
LRESULT MsgContextMenu(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
    bool const bMargin  = (SCN_MARGINRIGHTCLICK == umsg);
    bool const bNCArea  = (WM_NCRBUTTONDOWN == umsg);

    int const nID = bMargin ? IDC_MARGIN : (bNCArea ? IDC_NCAREA : GetDlgCtrlID((HWND)wParam));

    if ((nID != IDC_MARGIN) && 
        (nID != IDC_EDIT) && 
        (nID != IDC_REBAR) && 
        (nID != IDC_TOOLBAR) && 
        (nID != IDC_STATUSBAR) &&
        (nID != IDC_NCAREA)) {
        return DefWindowProc(hwnd, umsg, wParam, lParam);
    }

    // no context menu after undo/redo history scrolling
    if (s_bUndoRedoScroll) {
        s_bUndoRedoScroll = false;
        return FALSE;
    }

    POINT pt = { -1, -1 };
    pt.x = (int)((short)LOWORD(bMargin ? wParam : lParam));
    pt.y = (int)((short)HIWORD(bMargin ? wParam : lParam));
    #define IS_CTX_PT_VALID(P) (((P).x != -1 || (P).y != -1))

    if (nID == IDC_NCAREA) { // only valid for Menu Bar
        if (!IS_CTX_PT_VALID(pt)) {
            GetCursorPos(&pt);
        }
        MENUBARINFO mbi = { sizeof(MENUBARINFO) };
        GetMenuBarInfo(hwnd, OBJID_MENU, 0, &mbi);
        if (pt.y < mbi.rcBar.top || pt.y > mbi.rcBar.bottom) {
            return DefWindowProc(hwnd, umsg, wParam, lParam);
        }
    }

    typedef enum { MNU_NONE = -1, MNU_EDIT = 0, MNU_BAR, MNU_MARGIN, MNU_TRAY, MNU_NCAREA } mnu_t;
    mnu_t imenu = MNU_NONE;

    HMENU const hMenuCtx = LoadMenu(Globals.hLngResContainer, MAKEINTRESOURCE(IDR_MUI_POPUPMENU));
    //SetMenuDefaultItem(GetSubMenu(hmenu,1),0,false);

    switch (nID) {
    case IDC_EDIT: {

        if (!IS_CTX_PT_VALID(pt)) {
            // caused by keyboard near caret pos
            DocPos const iCurrentPos = SciCall_GetCurrentPos();
            pt.x = (LONG)SciCall_PointXFromPosition(iCurrentPos);
            pt.y = (LONG)SciCall_PointYFromPosition(iCurrentPos);
        }
        else {
            ScreenToClient(Globals.hwndEdit, &pt);
        }
        imenu = bMargin ? MNU_MARGIN : MNU_EDIT;

        if (imenu == MNU_EDIT) {
            // modify configured items
            HMENU const hStdCtxMenu = GetSubMenu(hMenuCtx, imenu);
            if (StrIsNotEmpty(Settings2.WebTmpl1MenuName)) {
                ModifyMenu(hStdCtxMenu, CMD_WEBACTION1, MF_BYCOMMAND | MF_STRING, CMD_WEBACTION1, Settings2.WebTmpl1MenuName);
            }
            if (StrIsNotEmpty(Settings2.WebTmpl2MenuName)) {
                ModifyMenu(hStdCtxMenu, CMD_WEBACTION2, MF_BYCOMMAND | MF_STRING, CMD_WEBACTION2, Settings2.WebTmpl2MenuName);
            }
        }

        // back to screen coordinates for menu display
        ClientToScreen(Globals.hwndEdit, &pt);

    } break;

    case IDC_TOOLBAR:
    case IDC_STATUSBAR:
    case IDC_REBAR:
    case IDC_NCAREA: {
        if (!IS_CTX_PT_VALID(pt)) {
            GetCursorPos(&pt);
        }
        imenu = MNU_BAR;
    } break;

    case IDC_MARGIN: {
        if (!IS_CTX_PT_VALID(pt)) {
            GetCursorPos(&pt);
        }

        DocLn const curLn = Sci_GetCurrentLineNumber();
        int const   bitmask = SciCall_MarkerGet(curLn) & ALL_MARKERS_BITMASK();
        EnableCmd(hMenuCtx, IDM_EDIT_CLEAR_MARKER, bitmask);
        EnableCmd(hMenuCtx, IDM_EDIT_CUT_MARKED, bitmask);
        EnableCmd(hMenuCtx, IDM_EDIT_COPY_MARKED, bitmask);
        EnableCmd(hMenuCtx, IDM_EDIT_DELETE_MARKED, bitmask);

        const SCNotification* const scn = (SCNotification*)lParam;
        switch (scn->margin) {
        case MARGIN_SCI_FOLDING:
        case MARGIN_SCI_CHGHIST:
        case MARGIN_SCI_BOOKMRK:
        case MARGIN_SCI_LINENUM:
            imenu = MNU_MARGIN;
            break;
        default:
            break;
        }
    } break;

    default:
        break;
    }

    if (imenu != MNU_NONE) {
        TrackPopupMenuEx(GetSubMenu(hMenuCtx, imenu), TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x + 1, pt.y + 1, hwnd, NULL);
    }
    DestroyMenu(hMenuCtx);

    return (imenu != MNU_NONE) ? !0 : 0;
}

//=============================================================================
//
//  MsgTrayMessage() - Handles WM_TRAYMESSAGE
//
//
LRESULT MsgTrayMessage(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);

    switch (lParam) {
    case WM_RBUTTONUP: {
        HMENU hTrayMenu  = LoadMenu(Globals.hLngResContainer, MAKEINTRESOURCE(IDR_MUI_POPUPMENU));
        HMENU hMenuPopup = GetSubMenu(hTrayMenu, 3);

        SetForegroundWindow(hwnd);

        POINT pt = { -1, -1 };
        GetCursorPos(&pt);
        SetMenuDefaultItem(hMenuPopup, IDM_TRAY_RESTORE, false);
        int iCmd = TrackPopupMenu(hMenuPopup,
                              TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
                              pt.x, pt.y, 0, hwnd, NULL);

        PostMessage(hwnd, WM_NULL, 0, 0);

        DestroyMenu(hTrayMenu);

        if (iCmd == IDM_TRAY_RESTORE) {
            ShowNotifyIcon(hwnd, false);
            RestoreWndFromTray(hwnd);
            ShowOwnedPopups(hwnd, true);
        } else if (iCmd == IDM_TRAY_EXIT) {
            ShowNotifyIcon(hwnd,false);
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
    if ((BOOL)wParam == FALSE) { // is main menu
        HMENU const hCurMenu = GetMenu(hwnd);
        if (!hCurMenu) {
            SetMenu(hwnd, Globals.hMainMenu);
            DrawMenuBar(hwnd);
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
    if ((BOOL)wParam == FALSE) { // is main menu
        HMENU const hCurMenu = GetMenu(hwnd);
        if (hCurMenu && !Settings.ShowMenubar) {
            SetMenu(hwnd, NULL);
            DrawMenuBar(hwnd);
        }
    }
    return (LRESULT)wParam;
}


//=============================================================================
//
//  MsgInitMenu() - Handles WM_INITMENU
//
LRESULT MsgInitMenu(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    HMENU const hmenu = wParam ? (HMENU)wParam : GetMenu(hwnd);
    if (!hmenu) {
        return FALSE;
    }

    //bool const dm = UseDarkMode();
    bool const si = Flags.bSingleFileInstance;
    bool const cf = Path_IsNotEmpty(Paths.CurrentFile);
    bool const ro = SciCall_GetReadOnly();                                 // scintilla mode read-only
    bool const lck = (FileWatching.FileWatchingMode == FWM_EXCLUSIVELOCK); // file write lock
    bool const faro = IsFileReadOnly();                                    // file attrib read-only
    bool const pst = SciCall_CanPaste();
    bool const se = SciCall_IsSelectionEmpty();
    bool const mrs = Sci_IsMultiOrRectangleSelection();
    bool const te = Sci_IsDocEmpty();
    bool const mls = Sci_IsSelectionMultiLine();
    bool const moe = IsMarkOccurrencesEnabled();
    bool const isn = IsSaveNeeded();

    //bool const lfl = Flags.bHugeFileLoadState;

    //~bool const sav = Globals.bCanSaveIniFile; ~ done by UpdateSaveSettingsCmds()

    DocPos const iCurPos = SciCall_GetCurrentPos();
    DocLn const iCurLine = SciCall_LineFromPosition(iCurPos);
    bool const bPosInSel = Sci_IsPosInSelection(iCurPos);

    // ------------------------------------------------------

    EnableCmd(hmenu, IDM_FILE_REVERT, cf);
    EnableCmd(hmenu, CMD_RELOADASCIIASUTF8, cf);
    EnableCmd(hmenu, CMD_RELOADFORCEDETECTION, cf);
    EnableCmd(hmenu, CMD_RECODEANSI, cf);
    EnableCmd(hmenu, CMD_RECODEOEM, cf);
    EnableCmd(hmenu, CMD_RELOADNOENCODETAGS, cf);
    EnableCmd(hmenu, CMD_RECODEDEFAULT, cf);
    EnableCmd(hmenu, CMD_RECODEGB18030, cf);

    SetGrepWinIcon(hwnd, hmenu, IDM_GREP_WIN_SEARCH);

    EnableCmd(hmenu, IDM_FILE_NEWWINDOW2, !(cf && si) && !isn);

    SetWinIcon(hwnd, hmenu, IDM_FILE_LAUNCH);
    EnableCmd(hmenu, IDM_FILE_LAUNCH, cf);

    SetUACIcon(hwnd, hmenu, IDM_FILE_LAUNCH_ELEVATED);
    CheckCmd(hmenu, IDM_FILE_LAUNCH_ELEVATED, s_bIsProcessElevated);
    EnableCmd(hmenu, IDM_FILE_LAUNCH_ELEVATED, !s_bIsProcessElevated);

    SetWinIcon(hwnd, hmenu, IDM_FILE_RUN);

    CheckCmd(hmenu, CMD_IGNORE_FILE_VARS, Flags.NoFileVariables);

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
    EnableCmd(hmenu, IDM_FILE_LOCK_SHARE_READ, cf);
    CheckCmd(hmenu, IDM_FILE_LOCK_SHARE_READ, lck);

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
    } else if (Encoding_IsUNICODE(Encoding_GetCurrent())) {
        i = IDM_ENCODING_UNICODE;
    } else if (Encoding_IsUTF8_SIGN(Encoding_GetCurrent())) {
        i = IDM_ENCODING_UTF8SIGN;
    } else if (Encoding_IsUTF8(Encoding_GetCurrent())) {
        i = IDM_ENCODING_UTF8;
    } else if (Encoding_IsANSI(Encoding_GetCurrent())) {
        i = IDM_ENCODING_ANSI;
    } else {
        i = -1;
    }
    CheckMenuRadioItem(hmenu,IDM_ENCODING_ANSI,IDM_ENCODING_UTF8SIGN,i,MF_BYCOMMAND);

    int const eol_mode = SciCall_GetEOLMode();
    if (eol_mode == SC_EOL_CRLF) {
        i = IDM_LINEENDINGS_CRLF;
    } else if (eol_mode == SC_EOL_CR) {
        i = IDM_LINEENDINGS_CR;
    } else {
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
    EnableCmd(hmenu, IDM_EDIT_COPYLINE, !mrs);
    EnableCmd(hmenu, IDM_EDIT_DELETELINE, !ro);

    EnableCmd(hmenu, IDM_EDIT_UNITEDUPLICATELINES, !ro);
    EnableCmd(hmenu, IDM_EDIT_REMOVEDUPLICATELINES, !ro);
    EnableCmd(hmenu, IDM_EDIT_MERGEBLANKLINES, !ro);
    EnableCmd(hmenu, IDM_EDIT_MERGEEMPTYLINES, !ro);
    EnableCmd(hmenu, IDM_EDIT_REMOVEBLANKLINES, !ro);
    EnableCmd(hmenu, IDM_EDIT_REMOVEEMPTYLINES, !ro);

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
    EnableCmd(hmenu, IDM_EDIT_SORTLINES, !ro);

    //EnableCmd(hmenu,IDM_EDIT_COLUMNWRAP,i /*&& IsWindowsNT()*/);
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
    EnableCmd(hmenu, IDM_EDIT_BASE64ENCODE, !se && !ro);
    EnableCmd(hmenu, IDM_EDIT_BASE64DECODE, !se && !ro);
    EnableCmd(hmenu, IDM_EDIT_B64DECODESEL, !se && !ro);
    EnableCmd(hmenu, IDM_EDIT_PATH2URL, !se && !ro);
    EnableCmd(hmenu, IDM_EDIT_URL2PATH, !se && !ro);
    EnableCmd(hmenu, IDM_EDIT_INVERTBACKSLASH, !se && !ro);
    EnableCmd(hmenu, IDM_EDIT_INVERTSLASH, !se && !ro);

    EnableCmd(hmenu, IDM_EDIT_ESCAPECCHARS, !se && !ro);
    EnableCmd(hmenu, IDM_EDIT_UNESCAPECCHARS, !se && !ro);

    EnableCmd(hmenu, IDM_EDIT_CHAR2HEX, !ro);  // Char2Hex allowed for char after current pos
    EnableCmd(hmenu, IDM_EDIT_HEX2CHAR, !se && !ro);

    EnableCmd(hmenu, IDM_SET_SHOWEXCERPT, !se);

    WCHAR cmnt[8];
    Lexer_GetLineCommentStrg(cmnt, COUNTOF(cmnt));
    EnableCmd(hmenu, IDM_EDIT_LINECOMMENT, StrIsNotEmpty(cmnt) && !ro);
    EnableCmd(hmenu, IDM_EDIT_LINECOMMENT_ADD, StrIsNotEmpty(cmnt) && !ro);
    EnableCmd(hmenu, IDM_EDIT_LINECOMMENT_REMOVE, StrIsNotEmpty(cmnt) && !ro);
    EnableCmd(hmenu, IDM_EDIT_LINECOMMENT_BLOCKEDIT, StrIsNotEmpty(cmnt) && !ro);

    Lexer_GetStreamCommentStrgs(cmnt, cmnt, COUNTOF(cmnt));
    EnableCmd(hmenu, IDM_EDIT_STREAMCOMMENT, StrIsNotEmpty(cmnt) && !ro);

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

    SetGrepWinIcon(hwnd, hmenu, IDM_GREP_WIN_SEARCH2);

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

    CheckCmd(hmenu, IDM_VIEW_USE2NDDEFAULT, Style_GetUse2ndDefault());

    CheckCmd(hmenu, IDM_VIEW_READONLY, ro);
    CheckCmd(hmenu, IDM_VIEW_WORDWRAP, Globals.fvCurFile.bWordWrap);
    CheckCmd(hmenu, IDM_VIEW_LONGLINEMARKER, Settings.MarkLongLines);
    CheckCmd(hmenu, IDM_VIEW_SHOWINDENTGUIDES, Settings.ShowIndentGuides);
    CheckCmd(hmenu, IDM_VIEW_LINENUMBERS, SciCall_GetMarginWidthN(MARGIN_SCI_LINENUM) > 0);
    CheckCmd(hmenu, IDM_VIEW_BOOKMARK_MARGIN, SciCall_GetMarginWidthN(MARGIN_SCI_BOOKMRK) > 0);
    CheckCmd(hmenu, IDM_VIEW_CHGHIST_TOGGLE_MARGIN, SciCall_GetMarginWidthN(MARGIN_SCI_CHGHIST) > 0);
    CheckCmd(hmenu, IDM_VIEW_CHASING_DOCTAIL, FileWatching.MonitoringLog);

    CheckCmd(hmenu, IDM_VIEW_MARKOCCUR_ONOFF, moe);
    CheckCmd(hmenu, IDM_VIEW_MARKOCCUR_BOOKMARKS, Settings.MarkOccurrencesBookmark);
    EnableCmd(hmenu, IDM_VIEW_MARKOCCUR_BOOKMARKS, moe);
    CheckCmd(hmenu, IDM_VIEW_MARKOCCUR_VISIBLE, Settings.MarkOccurrencesMatchVisible);
    CheckCmd(hmenu, IDM_VIEW_MARKOCCUR_CASE, Settings.MarkOccurrencesMatchCase);

    CheckCmd(hmenu, IDM_VIEW_TOGGLE_VIEW, FocusedView.HideNonMatchedLines);
    EnableCmd(hmenu, IDM_VIEW_TOGGLE_VIEW, IsFocusedViewAllowed());

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

    int const _SUB_MNU_2 = 8;  // menu:View -> base for parent of sub-menus (adj. offset accordingly)

    int const chState = SciCall_GetChangeHistory();
    assert(chState == Settings.ChangeHistoryMode);
    i = IDM_VIEW_CHGHIST_NONE;
    i += (chState & SC_CHANGE_HISTORY_MARKERS) ? 1 : 0;
    i += (chState & SC_CHANGE_HISTORY_INDICATORS) ? 2 : 0;
    CheckMenuRadioItem(hmenu, IDM_VIEW_CHGHIST_NONE, IDM_VIEW_CHGHIST_ALL, i, MF_BYCOMMAND);
    CheckCmdPos(GetSubMenu(hmenu, 2), _SUB_MNU_2 + 0, (i != IDM_VIEW_CHGHIST_NONE));

    i = IDM_VIEW_COLORDEFHOTSPOTS + Settings.ColorDefHotspot;
    CheckMenuRadioItem(hmenu, IDM_VIEW_COLORDEFHOTSPOTS, IDM_VIEW_COLOR_BGRA, i, MF_BYCOMMAND);
    CheckCmdPos(GetSubMenu(hmenu, 2), _SUB_MNU_2 + 4, IsColorDefHotspotEnabled());

    CheckCmd(hmenu, IDM_VIEW_UNICODE_POINTS, Settings.HighlightUnicodePoints);
    CheckCmd(hmenu, IDM_VIEW_MATCHBRACES, Settings.MatchBraces);

    i = IDM_VIEW_HILITCURLN_NONE + Settings.HighlightCurrentLine;
    CheckMenuRadioItem(hmenu, IDM_VIEW_HILITCURLN_NONE, IDM_VIEW_HILITCURLN_FRAME, i, MF_BYCOMMAND);
    CheckCmdPos(GetSubMenu(hmenu, 2), _SUB_MNU_2 + 7, (i != IDM_VIEW_HILITCURLN_NONE));

#ifdef D_NP3_WIN10_DARK_MODE
    EnableCmd(hmenu, IDM_VIEW_WIN_DARK_MODE, IsDarkModeSupported());
    CheckCmd(hmenu, IDM_VIEW_WIN_DARK_MODE, IsSettingDarkMode());
#else
    RemoveMenu(hmenu, IDM_VIEW_WIN_DARK_MODE, 0);
#endif

    // --------------------------------------------------------------------------

    int const mnuMain = 2;
    int const mnuSubOcc = _SUB_MNU_2 + 8;
    int const mnuSubSubWord = 6;

    if (Settings.MarkOccurrencesMatchWholeWords) {
        i = IDM_VIEW_MARKOCCUR_WORD;
    } else if (Settings.MarkOccurrencesCurrentWord) {
        i = IDM_VIEW_MARKOCCUR_CURRENT;
    } else {
        i = IDM_VIEW_MARKOCCUR_WNONE;
    }
    CheckMenuRadioItem(hmenu, IDM_VIEW_MARKOCCUR_WNONE, IDM_VIEW_MARKOCCUR_CURRENT, i, MF_BYCOMMAND);
    CheckCmdPos(GetSubMenu(GetSubMenu(hmenu, mnuMain), mnuSubOcc), mnuSubSubWord, (i != IDM_VIEW_MARKOCCUR_WNONE));

    i = IsMarkOccurrencesEnabled();
    EnableCmd(hmenu, IDM_VIEW_MARKOCCUR_VISIBLE, i);
    EnableCmd(hmenu, IDM_VIEW_MARKOCCUR_CASE, i);
    EnableCmd(hmenu, IDM_VIEW_MARKOCCUR_WNONE, i);
    EnableCmd(hmenu, IDM_VIEW_MARKOCCUR_WORD, i);
    EnableCmd(hmenu, IDM_VIEW_MARKOCCUR_CURRENT, i);
    EnableCmdPos(GetSubMenu(GetSubMenu(hmenu, mnuMain), mnuSubOcc), mnuSubSubWord, i);
    CheckCmdPos(GetSubMenu(hmenu, mnuMain), mnuSubOcc, i);

    // --------------------------------------------------------------------------

    bool const fd = (FocusedView.CodeFoldingAvailable && FocusedView.ShowCodeFolding);
    EnableCmd(hmenu, IDM_VIEW_FOLDING, FocusedView.CodeFoldingAvailable && !FocusedView.HideNonMatchedLines);
    CheckCmd(hmenu, IDM_VIEW_FOLDING, fd);
    EnableCmd(hmenu, IDM_VIEW_TOGGLEFOLDS, !te && fd);
    EnableCmd(hmenu, CMD_FOLDJUMPDOWN, !te && fd);
    EnableCmd(hmenu, CMD_FOLDJUMPUP, !te && fd);
    EnableCmd(hmenu, CMD_FOLDCOLLAPSE, !te && fd);
    EnableCmd(hmenu, CMD_FOLDEXPAND, !te && fd);
    bool const bF = (SC_FOLDLEVELBASE < (SciCall_GetFoldLevel(iCurLine) & SC_FOLDLEVELNUMBERMASK));
    bool const bH = (SciCall_GetFoldLevel(iCurLine) & SC_FOLDLEVELHEADERFLAG);
    EnableCmd(hmenu, IDM_VIEW_TOGGLE_CURRENT_FOLD, !te && fd && (bF || bH));
    CheckCmdPos(GetSubMenu(hmenu, 2), _SUB_MNU_2 + 14, fd);


    // --------------------------------------------------------------------------

    CheckCmd(hmenu, IDM_VIEW_SHOWBLANKS, Settings.ViewWhiteSpace);
    CheckCmd(hmenu, IDM_VIEW_SHOWEOLS, Settings.ViewEOLs);
    CheckCmd(hmenu, IDM_VIEW_WORDWRAPSYMBOLS, Settings.ShowWordWrapSymbols);
    CheckCmd(hmenu, IDM_VIEW_TITLEBAR, Settings.ShowTitlebar);
    CheckCmd(hmenu, IDM_VIEW_MENUBAR, Settings.ShowMenubar);
    CheckCmd(hmenu, IDM_VIEW_TOOLBAR, Settings.ShowToolbar);
    EnableCmd(hmenu, IDM_VIEW_CUSTOMIZETB, Settings.ShowToolbar);
    CheckCmd(hmenu, IDM_VIEW_STATUSBAR, Settings.ShowStatusbar);
    CheckCmd(hmenu, IDM_VIEW_DPISCALETB, Settings.DpiScaleToolBar);

    CheckCmd(hmenu, IDM_VIEW_STICKYWINPOS, Flags.bStickyWindowPosition);
    CheckCmd(hmenu, IDM_VIEW_SHOW_HYPLNK_CALLTIP, Settings.ShowHypLnkToolTip);
    CheckCmd(hmenu, IDM_VIEW_SCROLLPASTEOF, Settings.ScrollPastEOF);

    //i = SciCall_GetLexer();
    //EnableCmd(hmenu,IDM_SET_AUTOCLOSETAGS,(i == SCLEX_HTML || i == SCLEX_XML));
    CheckCmd(hmenu, IDM_SET_AUTOCLOSETAGS, Settings.AutoCloseTags /*&& (i == SCLEX_HTML || i == SCLEX_XML)*/);
    CheckCmd(hmenu, IDM_SET_AUTOCLOSEQUOTES, Settings.AutoCloseQuotes);
    CheckCmd(hmenu, IDM_SET_AUTOCLOSEBRACKETS, Settings.AutoCloseBrackets);

    CheckCmd(hmenu, IDM_SET_REUSEWINDOW, Flags.bReuseWindow);
    CheckCmd(hmenu, IDM_SET_SINGLEFILEINSTANCE, Flags.bSingleFileInstance);

    CheckCmd(hmenu, IDM_SET_ALWAYSONTOP, Settings.AlwaysOnTop);
    CheckCmd(hmenu, IDM_SET_MINTOTRAY, Settings.MinimizeToTray);
    CheckCmd(hmenu, IDM_SET_TRANSPARENT, Settings.TransparentMode);

    CheckCmd(hmenu, IDM_SET_TABSASSPACES, Globals.fvCurFile.bTabsAsSpaces);
    CheckCmd(hmenu, IDM_SET_AUTOINDENTTEXT, Settings.AutoIndent);

    EnableCmd(hmenu, IDM_EDIT_COMPLETEWORD, !te && !ro);
    CheckCmd(hmenu, IDM_SET_AUTOCOMPLETEWORDS, Settings.AutoCompleteWords && !ro);
    CheckCmd(hmenu, IDM_SET_AUTOCLEXKEYWORDS, Settings.AutoCLexerKeyWords && !ro);

    CheckCmd(hmenu, IDM_SET_ALTERNATE_WORD_SEPS, Settings.AccelWordNavigation);
    CheckCmd(hmenu, IDM_SET_AUTOSAVE_BACKUP, (Settings.AutoSaveOptions & (ASB_Periodic | ASB_Backup)));

    CheckCmd(hmenu, IDM_SET_MULTIPLE_SELECTION, Settings.MultipleSelection);

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
    } else {
        i = IDM_SET_BIDIRECTIONAL_NONE;
        CheckMenuRadioItem(hmenu, IDM_SET_BIDIRECTIONAL_NONE, IDM_SET_BIDIRECTIONAL_R2L, i, MF_BYCOMMAND);
    }
    EnableCmd(hmenu, IDM_SET_BIDIRECTIONAL_NONE, dwr);
    EnableCmd(hmenu, IDM_SET_BIDIRECTIONAL_L2R, dwr);
    EnableCmd(hmenu, IDM_SET_BIDIRECTIONAL_R2L, dwr);

    CheckCmd(hmenu, IDS_USE_LOCALE_DATEFMT, Settings.PreferredLocale4DateFmt);

    CheckCmd(hmenu, IDM_SET_MUTE_MESSAGEBEEP, Settings.MuteMessageBeep);
    CheckCmd(hmenu, IDM_SET_SAVEBEFORERUNNINGTOOLS, Settings.SaveBeforeRunningTools);
    //~EnableCmd(hmenu, IDM_SET_SAVEBEFORERUNNINGTOOLS, !faro);

    CheckCmd(hmenu, IDM_SET_NOSAVERECENT, Settings.SaveRecentFiles);
    CheckCmd(hmenu, IDM_SET_NOPRESERVECARET, Settings.PreserveCaretPos);
    CheckCmd(hmenu, IDM_SET_NOSAVEFINDREPL, Settings.SaveFindReplace);
    CheckCmd(hmenu, IDM_SET_AUTOLOAD_MRU_FILE, Settings.AutoLoadMRUFile);

    CheckCmd(hmenu, IDM_SET_EVALTINYEXPRONSEL, Settings.EvalTinyExprOnSelection);

    CheckCmd(hmenu, IDM_SET_CHANGENOTIFY, (FileWatching.FileWatchingMode != FWM_DONT_CARE));

    if (StrIsNotEmpty(s_wchTitleExcerpt)) {
        i = IDM_SET_SHOWEXCERPT;
    } else if (Settings.PathNameFormat == 0) {
        i = IDM_SET_SHOWFILENAMEONLY;
    } else if (Settings.PathNameFormat == 1) {
        i = IDM_SET_SHOWFILENAMEFIRST;
    } else {
        i = IDM_SET_SHOWFULLPATH;
    }
    CheckMenuRadioItem(hmenu, IDM_SET_SHOWFILENAMEONLY, IDM_SET_SHOWEXCERPT, i, MF_BYCOMMAND);

    if (Settings.EscFunction == 1) {
        i = IDM_SET_ESCMINIMIZE;
    } else if (Settings.EscFunction == 2) {
        i = IDM_SET_ESCEXIT;
    } else {
        i = IDM_SET_NOESCFUNC;
    }
    CheckMenuRadioItem(hmenu, IDM_SET_NOESCFUNC, IDM_SET_ESCEXIT, i, MF_BYCOMMAND);

    bool const bIsHLink = (SciCall_IndicatorValueAt(INDIC_NP3_HYPERLINK, iCurPos) > 0);
    EnableCmd(hmenu, CMD_OPEN_HYPERLINK, !mrs && bIsHLink);
    EnableCmd(hmenu, CMD_WEBACTION1, !se && !mrs && bPosInSel && !bIsHLink);
    EnableCmd(hmenu, CMD_WEBACTION2, !se && !mrs && bPosInSel && !bIsHLink);

    i = (int)Path_IsNotEmpty(Settings2.AdministrationTool);
    EnableCmd(hmenu, IDM_HELP_ADMINEXE, i);

    #if defined(HAVE_DYN_LOAD_LIBS_MUI_LNGS)
        for (unsigned lng = 0; lng < MuiLanguages_CountOf(); ++lng) {
            //EnableCmd(hmenu, GetMUILngResourceID(lng), ExistMUILanguageDLL(lng));
            CheckCmd(hmenu, GetMUILngResourceID(lng), IsMUILanguageActive(lng));
        }
    #endif

    UpdateSaveSettingsCmds();

    return FALSE;
}


#if 0
//=============================================================================
//
//  MsgKeyDown() - Handles WM_KEYDOWN event
//  ~~~  no event from Scintilla  ~~~
//
//
LRESULT MsgKeyDown(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    if (IsKeyDown(VK_CONTROL)) {
        SciCall_SetCursor(SC_NP3_CURSORHAND);
    }
    return FALSE;
}
#endif


//=============================================================================
//
//  _ApplyChangeHistoryMode() - Handles Change-History Settings
//
static void _ApplyChangeHistoryMode()
{
    int const iChgHist = SciCall_GetChangeHistory();
    if (iChgHist == Settings.ChangeHistoryMode) { return; }
    if ((!iChgHist && Settings.ChangeHistoryMode) || !Settings.ChangeHistoryMode) {
        if (IsYesOkay(InfoBoxLng(MB_YESNO | MB_ICONWARNING, L"AllowClearUndoHistory", IDS_MUI_ASK_CLEAR_UNDO))) {
            UndoRedoReset();
        }
        else {
            Settings.ChangeHistoryMode = iChgHist;
            return;
        }
    }
    else {
        SciCall_SetChangeHistory(Settings.ChangeHistoryMode);
    }
    UpdateToolbar();
    UpdateMargins(true);
}


//=============================================================================
//
//  MsgCommand() - Handles WM_COMMAND
//
LRESULT MsgCommand(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
    unsigned const iLoWParam = (unsigned)LOWORD(wParam);

    #if defined(HAVE_DYN_LOAD_LIBS_MUI_LNGS)
    bool const bIsLngMenuCmd = ((iLoWParam >= IDS_MUI_LANG_EN_US) && (iLoWParam < (IDS_MUI_LANG_EN_US + MuiLanguages_CountOf())));
    if (bIsLngMenuCmd) {
        DynamicLanguageMenuCmd(iLoWParam);
        Style_InsertThemesMenu(Globals.hMainMenu);
        DrawMenuBar(Globals.hwndMain);
        UpdateToolbar();
        return FALSE;
    }
    #endif

    bool const bIsThemesMenuCmd = ((iLoWParam >= IDM_THEMES_FACTORY_RESET) && (iLoWParam < (int)(IDM_THEMES_FACTORY_RESET + ThemeItems_CountOf())));
    if (bIsThemesMenuCmd) {
        if (iLoWParam == IDM_THEMES_FACTORY_RESET) {
            if (!IsYesOkay(InfoBoxLng(MB_OKCANCEL | MB_ICONWARNING, L"MsgResetScheme", IDS_MUI_WARN_STYLE_RESET))) {
                return FALSE;
            }
        }
        Style_DynamicThemesMenuCmd(iLoWParam);
        return FALSE;
    }

    switch(iLoWParam) {

    case SCEN_CHANGE:
        EditUpdateVisibleIndicators();
        MarkAllOccurrences(-1, false);
        break;

    case IDT_TIMER_UPDATE_STATUSBAR:
        _UpdateStatusbarDelayed((bool)lParam);
        break;

    case IDT_TIMER_UPDATE_TOOLBAR:
        _UpdateToolbarDelayed();
        break;

    case IDT_TIMER_UPDATE_TITLEBAR:
        _UpdateTitlebarDelayed((HWND)lParam);
        break;

    case IDT_TIMER_CALLBACK_MRKALL:
        EditMarkAllOccurrences(Globals.hwndEdit, (bool)lParam);
        break;

    case IDT_TIMER_CLEAR_CALLTIP:
        Sci_CallTipCancelEx();
        break;

    case IDT_TIMER_UNDO_TRANSACTION:
        _SplitUndoTransaction();
        break;

    case IDM_FILE_NEW: {
        HPATHL hfile_pth = Path_Allocate(L"");
        FileLoadFlags fLoadFlags = FLF_New;
        fLoadFlags |= Settings.SkipUnicodeDetection ? FLF_SkipUnicodeDetect : 0;
        fLoadFlags |= Settings.SkipANSICodePageDetection ? FLF_SkipANSICPDetection : 0;
        FileLoad(hfile_pth, fLoadFlags, 0, 0);
        Path_Release(hfile_pth);
    } break;


    case IDM_FILE_OPEN: {
        HPATHL hfile_pth = Path_Allocate(L"");
        FileLoadFlags fLoadFlags = FLF_None;
        fLoadFlags |= Settings.SkipUnicodeDetection ? FLF_SkipUnicodeDetect : 0;
        fLoadFlags |= Settings.SkipANSICodePageDetection ? FLF_SkipANSICPDetection : 0;
        FileLoad(hfile_pth, fLoadFlags, 0, 0);
        Path_Release(hfile_pth);
    } break;


    case IDM_FILE_REVERT:
        if (!FileWatching.MonitoringLog) {
            if (IsSaveNeeded()) {
                if (!IsYesOkay(InfoBoxLng(MB_YESNO | MB_ICONQUESTION, NULL, IDS_MUI_ASK_REVERT))) {
                    break;
                }
                //~ don't revert if no save needed
                //~FileRevert(Paths.CurrentFile, false);
            }
            // revert in any case (manually forced)
            FileRevert(Paths.CurrentFile, true);
        }
        break;


    case IDM_FILE_SAVE: {
        FileSave((FileWatching.FileWatchingMode <= FWM_DONT_CARE) ? FSF_SaveAlways : FSF_None);
    } break;


    case IDM_FILE_SAVEAS:
        FileSave(FSF_SaveAlways | FSF_SaveAs);
        break;


    case IDM_FILE_SAVECOPY:
        FileSave(FSF_SaveAlways | FSF_SaveAs | FSF_SaveCopy);
        break;


    case IDM_FILE_PRESERVE_FILEMODTIME: {
            if (!Flags.bPreserveFileModTime) {
                InfoBoxLng(MB_OK, L"PreserveFileModTime", IDS_MUI_INF_PRSVFILEMODTM);
            }
            Flags.bPreserveFileModTime = true;
            FileSave(FSF_SaveAlways);
        }
        break;


    case IDM_FILE_READONLY:
        if (Path_IsNotEmpty(Paths.CurrentFile)) {
            DWORD dwFileAttributes = Path_GetFileAttributes(Paths.CurrentFile);
            if (IsReadOnly(dwFileAttributes)) {
                dwFileAttributes = (dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);
            } else {
                dwFileAttributes |= FILE_ATTRIBUTE_READONLY;
            }
            WCHAR szDisplayName[MAX_PATH_EXPLICIT>>1] = { L'\0' };
            if (!Path_SetFileAttributes(Paths.CurrentFile, dwFileAttributes)) {
                Path_GetDisplayName(szDisplayName, COUNTOF(szDisplayName), Paths.CurrentFile, NULL, false);
                InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_READONLY_MODIFY, szDisplayName);
            }
            if (Flags.bSettingsFileSoftLocked) {
                Globals.bCanSaveIniFile = CanAccessPath(Paths.IniFile, GENERIC_WRITE);
                UpdateSaveSettingsCmds();
            }
            UpdateToolbar();
        }
        break;


    case IDM_FILE_LOCK_SHARE_READ:
        InstallFileWatching(false); // terminate
        if (FileWatching.FileWatchingMode == FWM_EXCLUSIVELOCK) {
            FileWatching.FileWatchingMode = Settings.FileWatchingMode;
        } else {
            FileWatching.FileWatchingMode = FWM_EXCLUSIVELOCK;
        }
        InstallFileWatching(true);
        break;


    case IDM_FILE_BROWSE:
        DialogFileBrowse(hwnd);
        break;


    case IDM_GREP_WIN_SEARCH:
    case IDM_GREP_WIN_SEARCH2: {
        WCHAR wchBuffer[MIDSZ_BUFFER] = { L'\0' };
        EditGetSelectedText(wchBuffer, COUNTOF(wchBuffer));
        DialogGrepWin(hwnd, wchBuffer);
    }
    break;


    case IDM_FILE_NEWWINDOW:
    case IDM_FILE_NEWWINDOW2: {
        SaveAllSettings(false);
        HPATHL hpth = (iLoWParam == IDM_FILE_NEWWINDOW2) ? Paths.CurrentFile : NULL;
        DialogNewWindow(hwnd, (hpth != NULL), hpth, NULL);
    }
    break;


    case IDM_FILE_LAUNCH: {
        if (Path_IsEmpty(Paths.CurrentFile)) {
            break;
        }
        if (Settings.SaveBeforeRunningTools && !FileSave(FSF_Ask)) {
            break;
        }

        HPATHL hdir = Path_Copy(Paths.CurrentFile); 
        Path_RemoveFileSpec(hdir);

        SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
        sei.fMask = SEE_MASK_DEFAULT;
        sei.hwnd = hwnd;
        sei.lpVerb = NULL;
        sei.lpFile = Path_Get(Paths.CurrentFile);
        sei.lpParameters = NULL;
        sei.lpDirectory = Path_Get(hdir);
        sei.nShow = SW_SHOWNORMAL;
        ShellExecuteExW(&sei);

        Path_Release(hdir);
    }
    break;


    case IDM_FILE_EXPLORE_DIR: {
        if (Settings.SaveBeforeRunningTools && !FileSave(FSF_Ask)) {
            break;
        }
        PIDLIST_ABSOLUTE pidl = NULL;
        SHParseDisplayName(Path_IsNotEmpty(Paths.CurrentFile) ? Path_Get(Paths.CurrentFile) : Path_Get(Paths.WorkingDirectory),
                           NULL, &pidl, SFGAO_BROWSABLE | SFGAO_FILESYSTEM, NULL);
        if (pidl) {
            SHOpenFolderAndSelectItems(pidl, 0, NULL, 0);
            ILFree(pidl);
        }
    }
    break;


    case IDM_FILE_LAUNCH_ELEVATED: {
        EditFileIOStatus fioStatus = INIT_FILEIO_STATUS;
        fioStatus.iEncoding        = Encoding_GetCurrent();
        fioStatus.iEOLMode         = SciCall_GetEOLMode();

        if (DoElevatedRelaunch(&fioStatus, false)) {
            CloseApplication();
        } else {
            InfoBoxLng(MB_ICONSHIELD, NULL, IDS_MUI_ERR_ELEVATED_RIGHTS);
        }
    }
    break;


    case IDM_FILE_RUN: {
        if (Settings.SaveBeforeRunningTools && !FileSave(FSF_Ask)) {
            break;
        }
        HPATHL hcpy = Path_Copy(Paths.CurrentFile);
        Path_QuoteSpaces(hcpy, false);
        RunDlg(hwnd, Path_Get(hcpy));
        Path_Release(hcpy);
    }
    break;

    case IDM_FILE_OPENWITH:
        if (Settings.SaveBeforeRunningTools && !FileSave(FSF_Ask)) {
            break;
        }
        OpenWithDlg(hwnd,Path_Get(Paths.CurrentFile));
        break;


    case IDM_FILE_PAGESETUP:
        EditPrintSetup(Globals.hwndEdit);
        break;

    case IDM_FILE_PRINT: {
        WCHAR tchPageFmt[32] = { L'\0' };
        WCHAR szDisplayName[MAX_PATH_EXPLICIT>>1];

        GetLngString(IDS_MUI_UNTITLED, szDisplayName, COUNTOF(szDisplayName));
        Path_GetDisplayName(szDisplayName, COUNTOF(szDisplayName), Paths.CurrentFile, NULL, false);

        GetLngString(IDS_MUI_PRINT_PAGENUM,tchPageFmt,COUNTOF(tchPageFmt));

        if (!EditPrint(Globals.hwndEdit, szDisplayName, tchPageFmt)) {
            InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_PRINT_ERROR, szDisplayName);
        }
    }
    break;


    case IDM_FILE_PROPERTIES: {
        if (Path_IsEmpty(Paths.CurrentFile)) {
            break;
        }

        SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
        sei.fMask = SEE_MASK_INVOKEIDLIST;
        sei.hwnd = hwnd;
        sei.lpVerb = L"properties";
        sei.lpFile = Path_Get(Paths.CurrentFile);
        sei.lpParameters = NULL;
        sei.lpDirectory = NULL;
        sei.nShow = SW_SHOWNORMAL;
        ShellExecuteExW(&sei);
    }
    break;

    case IDM_FILE_CREATELINK: {
        if (Path_IsEmpty(Paths.CurrentFile)) {
            break;
        }
        WCHAR tchDescription[128] = { L'\0' };
        GetLngString(IDS_MUI_LINKDESCRIPTION, tchDescription, COUNTOF(tchDescription));

        if (!Path_CreateDeskLnk(Paths.CurrentFile, tchDescription)) {
            InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_ERR_CREATELINK);
        }
    }
    break;


    case IDM_FILE_OPENFAV:
        if (FileSave(FSF_Ask)) {
            HPATHL hfile_pth = Path_Allocate(NULL);
            if (FavoritesDlg(hwnd, hfile_pth)) {
                if (Path_IsLnkFile(hfile_pth)) {
                    Path_GetLnkPath(hfile_pth, hfile_pth);
                }
                FileLoadFlags fLoadFlags = FLF_DontSave;
                fLoadFlags |= Settings.SkipUnicodeDetection ? FLF_SkipUnicodeDetect : 0;
                fLoadFlags |= Settings.SkipANSICodePageDetection ? FLF_SkipANSICPDetection : 0;
                if (Path_IsExistingDirectory(hfile_pth)) {
                    if (OpenFileDlg(Globals.hwndMain, hfile_pth, hfile_pth)) {
                        FileLoad(hfile_pth, fLoadFlags, 0, 0);
                    }
                }
                else {
                    FileLoad(hfile_pth, fLoadFlags, 0, 0);
                }
            }
            Path_Release(hfile_pth);
        }
        break;


    case IDM_FILE_ADDTOFAV:
        if (Path_IsNotEmpty(Paths.CurrentFile)) {
            AddToFavDlg(hwnd, Paths.CurrentFile);
        }
        break;


    case IDM_FILE_MANAGEFAV: {

        // Run favorites directory
        SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
        sei.fMask = SEE_MASK_DEFAULT;
        sei.hwnd = hwnd;
        sei.lpVerb = NULL;
        sei.lpFile = Path_Get(Settings.FavoritesDir);
        sei.lpParameters = NULL;
        sei.lpDirectory = NULL;
        sei.nShow = SW_SHOWNORMAL;
        ShellExecuteExW(&sei);
    }
    break;


    case IDM_FILE_RECENT:
        if (MRU_Count(Globals.pFileMRU) > 0) {
            if (FileSave(FSF_Ask)) {
                HPATHL hfile_pth = Path_Allocate(NULL);
                if (FileMRUDlg(hwnd, hfile_pth)) {
                    FileLoadFlags fLoadFlags = FLF_DontSave;
                    fLoadFlags |= Settings.SkipUnicodeDetection ? FLF_SkipUnicodeDetect : 0;
                    fLoadFlags |= Settings.SkipANSICodePageDetection ? FLF_SkipANSICPDetection : 0;
                    FileLoad(hfile_pth, fLoadFlags, 0, 0);
                }
                Path_Release(hfile_pth);
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
    case IDM_ENCODING_SELECT: {
        cpi_enc_t iNewEncoding = (HIWORD(wParam) >= IDM_ENCODING_SELECT) ?
                                 (cpi_enc_t)(HIWORD(wParam) - IDM_ENCODING_SELECT) : Encoding_GetCurrent();

        if (iLoWParam == IDM_ENCODING_SELECT) {
            if ((HIWORD(wParam) < IDM_ENCODING_SELECT) && !SelectEncodingDlg(hwnd, &iNewEncoding, false)) {
                break; // no change
            }
        } else {
            switch (iLoWParam) {
            case IDM_ENCODING_UNICODE:
                iNewEncoding = CPI_UNICODEBOM;
                break;
            case IDM_ENCODING_UNICODEREV:
                iNewEncoding = CPI_UNICODEBEBOM;
                break;
            case IDM_ENCODING_UTF8:
                iNewEncoding = CPI_UTF8;
                break;
            case IDM_ENCODING_UTF8SIGN:
                iNewEncoding = CPI_UTF8SIGN;
                break;
            case IDM_ENCODING_ANSI:
                iNewEncoding = CPI_ANSI_DEFAULT;
            }
        }
        BeginWaitCursorUID(Flags.bHugeFileLoadState, IDS_MUI_SB_RECODING_DOC);
        if (EditSetNewEncoding(Globals.hwndEdit, iNewEncoding, (s_flagSetEncoding != CPI_NONE))) {
            SetSaveNeeded(true);
        }
        EndWaitCursor();
    }
    break;


    case IDM_ENCODING_RECODE: {
        if (Path_IsNotEmpty(Paths.CurrentFile)) {
            cpi_enc_t iNewEncoding = Encoding_MapSignature(Encoding_GetCurrent());

            if (IsSaveNeeded()) {
                if (!IsYesOkay(InfoBoxLng(MB_YESNO | MB_ICONQUESTION, NULL, IDS_MUI_ASK_RECODE))) {
                    break;
                }
            }
            if (SelectEncodingDlg(hwnd, &iNewEncoding, true)) {
                Encoding_Forced(iNewEncoding);
                FileLoadFlags const fLoadFlags = FLF_DontSave | FLF_Reload | FLF_SkipUnicodeDetect | FLF_SkipANSICPDetection;
                FileLoad(Paths.CurrentFile, fLoadFlags, SciCall_GetCurrentPos(), SciCall_GetFirstVisibleLine());
            }
        }
    }
    break;


    case IDM_ENCODING_SETDEFAULT:
        SelectDefEncodingDlg(hwnd, &Settings.DefaultEncoding);
        break;


    case IDM_LINEENDINGS_CRLF:
    case IDM_LINEENDINGS_CR:
    case IDM_LINEENDINGS_LF: {
        int const eolMode = (iLoWParam - IDM_LINEENDINGS_CRLF); // SC_EOL_CRLF(0), SC_EOL_CR(1), SC_EOL_LF(2)
        BeginWaitCursorUID(Flags.bHugeFileLoadState, IDS_MUI_SB_CONV_LNBRK);
        SciCall_SetEOLMode(eolMode);
        EditEnsureConsistentLineEndings(Globals.hwndEdit);
        EndWaitCursor();
        UpdateToolbar();
    }
    break;


    case IDM_LINEENDINGS_SETDEFAULT:
        SelectDefLineEndingDlg(hwnd, (LPARAM)&Settings.DefaultEOLMode);
        break;


    case IDM_EDIT_UNDO:
        if (SciCall_CanUndo()) {
            //LimitNotifyEvents(EVM_UndoRedo);
            SciCall_Undo();
            //RestoreNotifyEvents();
            UpdateToolbar();
        }
        break;


    case IDM_EDIT_REDO:
        if (SciCall_CanRedo()) {
            //LimitNotifyEvents(EVM_UndoRedo);
            SciCall_Redo();
            //RestoreNotifyEvents();
            UpdateToolbar();
        }
        break;


    case IDM_EDIT_CUT: {
        if (SciCall_IsSelectionEmpty() && Settings2.NoCutLineOnEmptySelection) {
            break;
        }
        if (s_flagPasteBoard) {
            s_bLastCopyFromMe = true;
        }
        EditDeleteMarkerInSelection();
        if (SciCall_IsSelectionEmpty()) {
            EditCutLines(Globals.hwndEdit, true);
        } else {
            SciCall_Cut();
        }
    } 
    break;


    case IDM_EDIT_CUTLINE: {
        if (s_flagPasteBoard) {
            s_bLastCopyFromMe = true;
        }
        //~ explicit(!): ignore (SciCall_IsSelectionEmpty()) && Settings2.NoCutLineOnEmptySelection) 
        EditDeleteMarkerInSelection();
        EditCutLines(Globals.hwndEdit, false);
    }
    break;


    case IDM_EDIT_COPY: {
        if (s_flagPasteBoard) {
            s_bLastCopyFromMe = true;
        }
        if (SciCall_IsSelectionEmpty()) {
            if (!HandleHotSpotURLClicked(SciCall_GetCurrentPos(), COPY_HYPERLINK) &&
                    !Settings2.NoCopyLineOnEmptySelection) {
                if (Sci_GetNetLineLength(Sci_GetCurrentLineNumber()) > 0) {
                    SciCall_CopyAllowLine(); // (!) VisualStudio behavior
                    // On Windows, an extra "MSDEVLineSelect" marker is added to the clipboard
                    // which is then used in SCI_PASTE to paste the whole line before the current line.
                }
            }
        } else {
            EditCopyMultiSelection(Globals.hwndEdit);
        }
    }
    break;


    case IDM_EDIT_COPYLINE: {
        if (Sci_IsMultiOrRectangleSelection()) {
            InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECTORMULTI);
            break;
        }
        if (s_flagPasteBoard) {
            s_bLastCopyFromMe = true;
        }
        DocPos const iSelLnStart = Sci_GetLineStartPosition(SciCall_GetSelectionStart());
        // copy incl last line-breaks
        DocLn const lnSelLast = SciCall_LineFromPosition(SciCall_GetSelectionEnd());
        DocPos const iSelLnEnd = SciCall_PositionFromLine(lnSelLast) + SciCall_LineLength(lnSelLast);
        SciCall_CopyRange(iSelLnStart, iSelLnEnd);
    }
    break;


    case IDM_EDIT_COPYALL: {
        if (s_flagPasteBoard) {
            s_bLastCopyFromMe = true;
        }
        SciCall_CopyRange(0, Sci_GetDocEndPosition());
    }
    break;


    case IDM_EDIT_COPYADD: {
        if (Sci_IsMultiOrRectangleSelection()) {
            InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECTORMULTI);
            break;
        }
        if (s_flagPasteBoard) {
            s_bLastCopyFromMe = true;
        }
        DocPos const posSelStart = SciCall_IsSelectionEmpty() ? Sci_GetLineStartPosition(SciCall_GetSelectionStart()) : SciCall_GetSelectionStart();
        DocPos const posSelEnd   = SciCall_IsSelectionEmpty() ? Sci_GetLineEndPosition(SciCall_GetSelectionEnd()) : SciCall_GetSelectionEnd();
        EditCopyRangeAppend(Globals.hwndEdit, posSelStart, posSelEnd, true);
    }
    break;


    case IDM_EDIT_PASTE:
        if (SciCall_CanPaste()) {
            if (s_flagPasteBoard) {
                s_bLastCopyFromMe = true;
            }
            UndoTransActionBegin();
            SciCall_Paste();
            EndUndoTransAction();
        }
        break;


    case IDM_EDIT_SWAP:
        if (!SciCall_IsSelectionEmpty() && SciCall_CanPaste()) {
            if (s_flagPasteBoard) {
                s_bLastCopyFromMe = true;
            }
            EditSwapClipboard(Globals.hwndEdit, Settings.SkipUnicodeDetection);
        }
        break;


    case IDM_EDIT_CLEARCLIPBOARD:
        EditClearClipboard(Globals.hwndEdit);
        break;


    case IDM_EDIT_SELECTALL:
        SciCall_SelectAll();
        break;


    case IDM_EDIT_SELECTWORD: {
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


    case IDM_EDIT_SELECTALLMATCHES: {
        if (!Sci_IsMultiOrRectangleSelection()) {
            if (IsWindow(Globals.hwndDlgFindReplace)) {
                SetFindReplaceData(); // s_FindReplaceData
                EditSelectionMultiSelectAllEx(Globals.hwndEdit, &s_FindReplaceData);
            } else {
                if (SciCall_IsSelectionEmpty()) {
                    EditSelectWordAtPos(SciCall_GetCurrentPos(), false);
                }
                EditSelectionMultiSelectAll();
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
        if (SciCall_IsSelectionEmpty()) {
            SciCall_LineDuplicate();
        } else {
            SciCall_SelectionDuplicate();
        }
        break;


    case IDM_EDIT_LINETRANSPOSE:
        SciCall_LineTranspose();
        break;


    case IDM_EDIT_DELETELINE: {
        bool const bInLastLn = Sci_InLastLine();
        UserMarkerDeleteAll(Sci_GetCurrentLineNumber());
        SciCall_LineDelete();
        if (bInLastLn) {
            SciCall_DeleteBack();
            SciCall_Home();
        }
    }
    break;


    case IDM_EDIT_DELETELINELEFT: {
        SciCall_DelLineLeft();
    }
    break;


    case IDM_EDIT_DELETELINERIGHT: {
        SciCall_DelLineRight();
    }
    break;


    case IDM_EDIT_INDENT:
        EditIndentBlock(Globals.hwndEdit, SCI_TAB, true, false);
        //EditIndentBlock(Globals.hwndEdit, SCI_LINEINDENT, true, false);
        break;

    case IDM_EDIT_UNINDENT:
        EditIndentBlock(Globals.hwndEdit, SCI_BACKTAB, true, false);
        //EditIndentBlock(Globals.hwndEdit, SCI_LINEDEDENT, true, false);
        break;

    case CMD_TAB:
        EditIndentBlock(Globals.hwndEdit, SCI_TAB, false, false);
        //EditIndentBlock(Globals.hwndEdit, SCI_LINEINDENT, false, false);
        break;

    case CMD_BACKTAB:
        EditIndentBlock(Globals.hwndEdit, SCI_BACKTAB, false, false);
        //EditIndentBlock(Globals.hwndEdit, SCI_LINEDEDENT, false, false);
        break;

    case CMD_CTRLTAB:
        SciCall_SetUseTabs(true);
        SciCall_SetTabIndents(false);
        EditIndentBlock(Globals.hwndEdit, SCI_TAB, false, false);
        SciCall_SetTabIndents(Globals.fvCurFile.bTabIndents);
        SciCall_SetUseTabs(!Globals.fvCurFile.bTabsAsSpaces);
        break;

    case CMD_CHECK_INDENTATION: {
        EditFileIOStatus status = INIT_FILEIO_STATUS;
        EditIndentationStatistic(Globals.hwndEdit, &status);
        if (ConsistentIndentationCheck(&status)) {
            InfoBoxLng(MB_ICONINFORMATION, NULL, IDS_MUI_INDENT_CONSISTENT);
        }
    }
    break;

    case CMD_DELETEBACK: {
        EditDeleteMarkerInSelection();
        SciCall_DeleteBack();
    }
    break;

    case CMD_VK_INSERT:
        SciCall_EditToggleOverType();
        break;

    case IDM_EDIT_ENCLOSESELECTION: {
        static ENCLOSESELDATA data = { 0 };
        if (EditEncloseSelectionDlg(hwnd, &data)) {
            EditEncloseSelection(data.pwsz1, data.pwsz2);
        }
    }
    break;


    case IDM_EDIT_PADWITHSPACES:
        UndoTransActionBegin();
        EditPadWithSpaces(Globals.hwndEdit, false);
        EndUndoTransAction();
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


    case IDM_EDIT_UNITEDUPLICATELINES:
        EditUniteDuplicateLines(Globals.hwndEdit, false, false);
        break;

    case IDM_EDIT_REMOVEDUPLICATELINES:
        EditUniteDuplicateLines(Globals.hwndEdit, false, true);
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


    case IDM_EDIT_MODIFYLINES: {
        static ENCLOSESELDATA data = { 0 };
        if (EditModifyLinesDlg(hwnd, &data)) {
            EditModifyLines(&data);
        }
    }
    break;


    case IDM_EDIT_ALIGN:
        if (EditAlignDlg(hwnd,&s_iAlignMode)) {
            EditAlignText(s_iAlignMode);
        }
        break;


    case IDM_EDIT_SORTLINES:
        if (EditSortDlg(hwnd,&s_iSortOptions)) {
            EditSortLines(Globals.hwndEdit, s_iSortOptions);
        }
        break;


    case IDM_EDIT_COLUMNWRAP: {
        UINT uWrpCol = Globals.iWrapCol;
        if (ColumnWrapDlg(hwnd, IDD_MUI_COLUMNWRAP, &uWrpCol)) {
            Globals.iWrapCol = clampi((int)uWrpCol, SciCall_GetTabWidth(), LONG_LINES_MARKER_LIMIT);
            EditWrapToColumnEx(Globals.hwndEdit, Globals.iWrapCol);
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
        SciCall_UpperCase();
        break;


    case IDM_EDIT_CONVERTLOWERCASE:
        SciCall_LowerCase();
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


    case IDM_EDIT_INSERT_TAG: {
        static ENCLOSESELDATA data = { 0 };
        UINT repeat = 1;
        if (EditInsertTagDlg(hwnd, &data, &repeat)) {
            while (repeat > 0) {
                EditEncloseSelection(data.pwsz1, data.pwsz2);
                --repeat;
            }
        }
    }
    break;


    case IDM_EDIT_INSERT_ENCODING: {
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
    case IDM_EDIT_INSERT_PATHNAME: {
        if (Path_IsNotEmpty(Paths.CurrentFile)) {
            if (iLoWParam == IDM_EDIT_INSERT_FILENAME) {
                HPATHL hfilename = Path_Allocate(Path_FindFileName(Paths.CurrentFile));
                SetClipboardText(hwnd, Path_Get(hfilename), Path_GetLength(hfilename));
                Path_Release(hfilename);
            }
            else if (iLoWParam == IDM_EDIT_INSERT_DIRNAME) {
                HPATHL hcpy = Path_Copy(Paths.CurrentFile);
                Path_RemoveFileSpec(hcpy);
                SetClipboardText(hwnd, Path_Get(hcpy), Path_GetLength(hcpy));
                Path_Release(hcpy);
            } else {
                SetClipboardText(hwnd, Path_Get(Paths.CurrentFile), Path_GetLength(Paths.CurrentFile));
            }
        } else {
            WCHAR tchUntitled[64];
            GetLngString(IDS_MUI_UNTITLED, tchUntitled, COUNTOF(tchUntitled));
            SetClipboardText(hwnd, tchUntitled, StringCchLen(tchUntitled, 0));
        }
    }
    break;


    case IDM_EDIT_INSERT_GUID: {
        GUID guid;
        if (SUCCEEDED(CoCreateGuid(&guid))) {
            WCHAR wchBuf[128] = { L'\0' };
            if (StringFromGUID2(&guid, wchBuf, COUNTOF(wchBuf))) {
                StrTrim(wchBuf, L"{}");
                SetClipboardText(hwnd, wchBuf, StringCchLen(wchBuf, 0));
                //char chGuidBuffer[128] = { '\0' };
                //if (WideCharToMultiByte(Encoding_SciCP, 0, wchBuf, -1, chGuidBuffer, (int)COUNTOF(chGuidBuffer), NULL, NULL)) {
                //    EditReplaceSelection(chGuidBuffer, false);
                //}
            }
        }
    }
    break;


    case IDM_EDIT_LINECOMMENT:
    case IDM_EDIT_LINECOMMENT_ADD:
    case IDM_EDIT_LINECOMMENT_REMOVE: {
        WCHAR comment[8] = { L'\0' };
        bool const bAtStart = Lexer_GetLineCommentStrg(comment, COUNTOF(comment));
        if (StrIsNotEmpty(comment)) {
            switch (iLoWParam) {
            case IDM_EDIT_LINECOMMENT_ADD:
                EditToggleLineCommentsSimple(comment, bAtStart, LNC_ADD);
                break;
            case IDM_EDIT_LINECOMMENT_REMOVE:
                EditToggleLineCommentsSimple(comment, bAtStart, LNC_REMOVE);
                break;
            default:
                EditToggleLineCommentsSimple(comment, bAtStart, LNC_TOGGLE);
                break;
            }
        }
    }
    break;


    case IDM_EDIT_LINECOMMENT_BLOCKEDIT: {
        WCHAR comment[8] = { L'\0' };
        bool const bAtStart = Lexer_GetLineCommentStrg(comment, COUNTOF(comment));
        if (StrIsNotEmpty(comment)) {
            EditToggleLineCommentsExtended(comment, bAtStart);
        }
    }
    break;


    case IDM_EDIT_STREAMCOMMENT: {
        static ENCLOSESELDATA data = { 0 };
        Lexer_GetStreamCommentStrgs(data.pwsz1, data.pwsz2, ENCLDATA_SIZE);
        if (StrIsNotEmpty(data.pwsz1)) {
            EditEncloseSelection(data.pwsz1, data.pwsz2);
        }
    }
    break;


    case IDM_EDIT_URLENCODE: {
        EditURLEncode(false);
    }
    break;

    case IDM_EDIT_URLDECODE: {
        EditURLDecode(false);
    } 
    break;


    case IDM_EDIT_BASE64ENCODE: {
        EditBase64Code(Globals.hwndEdit, true, Encoding_GetCurrent());
    }
    break;

    case IDM_EDIT_BASE64DECODE: {
        EditBase64Code(Globals.hwndEdit, false, Encoding_GetCurrent());
    }
    break;

    case IDM_EDIT_B64DECODESEL: {
        cpi_enc_t iEncoding = Encoding_GetCurrent();
        if (!SelectEncodingDlg(hwnd, &iEncoding, false)) {
            break; // no selection
        }
        EditBase64Code(Globals.hwndEdit, false, iEncoding);
    }
    break;


    case IDM_EDIT_URL2PATH: {
        EditURLEncode(true);
    }
    break;

    case IDM_EDIT_PATH2URL: {
        EditURLDecode(true);
    }
    break;


    case IDM_EDIT_INVERTBACKSLASH: {
        EditReplaceAllChr(L'\\', L'/');
    } break;

    case IDM_EDIT_INVERTSLASH: {
        EditReplaceAllChr(L'/', L'\\');
    }
    break;


    case IDM_EDIT_ESCAPECCHARS: {
        EditEscapeCChars(Globals.hwndEdit);
    }
    break;


    case IDM_EDIT_UNESCAPECCHARS: {
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


    case IDM_EDIT_FIND: {
        SetFindReplaceData(); // s_FindReplaceData
        if (!IsWindow(Globals.hwndDlgFindReplace)) {
            Globals.bFindReplCopySelOrClip = true;
            /*Globals.hwndDlgFindReplace =*/ EditFindReplaceDlg(Globals.hwndEdit, &s_FindReplaceData, false);
        } else {
            Globals.bFindReplCopySelOrClip = (GetForegroundWindow() != Globals.hwndDlgFindReplace);
            if (GetDlgItem(Globals.hwndDlgFindReplace, IDC_REPLACE)) {
                SendWMCommand(Globals.hwndDlgFindReplace, IDMSG_SWITCHTOFIND);
                DestroyWindow(Globals.hwndDlgFindReplace);
                /*Globals.hwndDlgFindReplace =*/ EditFindReplaceDlg(Globals.hwndEdit, &s_FindReplaceData, false);
            } else {
                SetForegroundWindow(Globals.hwndDlgFindReplace);
                PostMessage(Globals.hwndDlgFindReplace, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(Globals.hwndDlgFindReplace, IDC_FINDTEXT)), 1);
            }
        }
    }
    break;


    case IDM_EDIT_REPLACE: {
        SetFindReplaceData(); // s_FindReplaceData
        if (!IsWindow(Globals.hwndDlgFindReplace)) {
            Globals.bFindReplCopySelOrClip = true;
            /*Globals.hwndDlgFindReplace =*/ EditFindReplaceDlg(Globals.hwndEdit, &s_FindReplaceData, true);
        } else {
            Globals.bFindReplCopySelOrClip = (GetForegroundWindow() != Globals.hwndDlgFindReplace);
            if (!GetDlgItem(Globals.hwndDlgFindReplace, IDC_REPLACE)) {
                SendWMCommand(Globals.hwndDlgFindReplace, IDMSG_SWITCHTOREPLACE);
                DestroyWindow(Globals.hwndDlgFindReplace);
                /*Globals.hwndDlgFindReplace =*/ EditFindReplaceDlg(Globals.hwndEdit, &s_FindReplaceData, true);
            } else {
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

        if (Sci_IsDocEmpty()) {
            break;
        }

        if (Sci_IsMultiSelection()) {
            switch (iLoWParam) {
            case IDM_EDIT_SELTONEXT: {
                SciCall_RotateSelection();
                Sci_ScrollSelectionToView();
            }
            break;

            case IDM_EDIT_SELTOPREV: {
                DocPosU const iMain = SciCall_GetMainSelection();
                if (iMain > 0) {
                    SciCall_SetMainSelection(iMain - 1);
                } else {
                    DocPosU const iNewMain = SciCall_GetSelections() - 1;
                    SciCall_SetMainSelection(iNewMain);
                }
                Sci_ScrollSelectionToView();
            }
            break;

            default:
                break;
            }
            break; // done
        }

        SetFindReplaceData(); // s_FindReplaceData

        if (IsFindPatternEmpty() && StrgIsNotEmpty(s_FindReplaceData.chFindPattern)) {
            if (iLoWParam != IDM_EDIT_REPLACENEXT) {
                SendWMCommand(hwnd, IDM_EDIT_FIND);
            } else {
                SendWMCommand(hwnd, IDM_EDIT_REPLACE);
            }
        } else {

            switch (iLoWParam) {

            case IDM_EDIT_FINDNEXT:
                EditFindNext(Globals.hwndEdit, &s_FindReplaceData, false, false, true);
                break;

            case IDM_EDIT_FINDPREV:
                EditFindPrev(Globals.hwndEdit, &s_FindReplaceData, false, false, true);
                break;

            case IDM_EDIT_REPLACENEXT:
                if (Globals.bReplaceInitialized) {
                    EditReplace(Globals.hwndEdit, &s_FindReplaceData);
                } else {
                    SendWMCommand(hwnd, IDM_EDIT_REPLACE);
                }
                break;

            case IDM_EDIT_SELTONEXT:
                if (IsFindPatternEmpty()) {
                    if (!SetCurrentSelAsFindReplaceData(&s_FindReplaceData)) {
                        break;
                    }
                }
                EditFindNext(Globals.hwndEdit, &s_FindReplaceData, true, false, false);
                break;

            case IDM_EDIT_SELTOPREV:
                if (IsFindPatternEmpty()) {
                    if (!SetCurrentSelAsFindReplaceData(&s_FindReplaceData)) {
                        break;
                    }
                }
                EditFindPrev(Globals.hwndEdit, &s_FindReplaceData, true, false, false);
                break;
            }
        }
        break;


    case CMD_FINDNEXTSEL:
    case CMD_FINDPREVSEL:
    case IDM_EDIT_SAVEFIND: {
        if (SetCurrentSelAsFindReplaceData(&s_FindReplaceData)) {
            MRU_Add(Globals.pMRUfind, GetFindPattern(), 0, -1, -1, NULL);
            s_FindReplaceData.fuFlags &= (~(SCFIND_REGEXP | SCFIND_POSIX));
            s_FindReplaceData.bTransformBS = false;

            switch (iLoWParam) {

            case IDM_EDIT_SAVEFIND:
                break;

            case CMD_FINDNEXTSEL:
                EditFindNext(Globals.hwndEdit, &s_FindReplaceData, false, false, false);
                break;

            case CMD_FINDPREVSEL:
                EditFindPrev(Globals.hwndEdit, &s_FindReplaceData, false, false, false);
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
        } else {
            SetForegroundWindow(Globals.hwndDlgCustomizeSchemes);
        }
        SendWMCommand(Globals.hwndDlgCustomizeSchemes, IDC_SETCURLEXERTV);
        break;


    case IDM_VIEW_FONT:
    case IDM_VIEW_CURRENTSCHEME:
        if (!IsWindow(Globals.hwndDlgCustomizeSchemes)) {
            Style_SetDefaultFont(Globals.hwndEdit, (iLoWParam == IDM_VIEW_FONT));
        }
        UpdateMargins(true);
        break;


    case IDM_VIEW_READONLY:
        SciCall_SetReadOnly(!SciCall_GetReadOnly());
        Settings.DocReadOnlyMode = SciCall_GetReadOnly();
        UpdateToolbar();
        break;


    case IDM_VIEW_WORDWRAP:
        Globals.fvCurFile.bWordWrap = Settings.WordWrap = !Settings.WordWrap;
        _SetWrapIndentMode(Globals.hwndEdit);
        //~Sci_ScrollSelectionToView(); // does not work here bug ?
        SciCall_SetFirstVisibleLine(max_ln(0, Sci_GetCurrentLineNumber() - Settings2.CurrentLineVerticalSlop));
        UpdateToolbar();
        break;


    case IDM_SET_WORDWRAPSETTINGS:
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


    case IDM_VIEW_LONGLINEMARKER: {
        Settings.MarkLongLines = !Settings.MarkLongLines;
        size_t cnt = 0;
        int    edgeColumns[EDGELINE_NUM_LIMIT] = { 0 };
        if (Settings.MarkLongLines) {
            cnt = ReadVectorFromString(Globals.fvCurFile.wchMultiEdgeLines, edgeColumns, COUNTOF(edgeColumns), 0, LONG_LINES_MARKER_LIMIT, 0, true);
        }
        Style_SetMultiEdgeLine(edgeColumns, cnt);
    }
    break;


    case IDM_SET_LONGLINESETTINGS: {
        int iLongLinesLimit = Defaults.LongLinesLimit;

        if (LongLineSettingsDlg(hwnd, IDD_MUI_LONGLINES, Globals.fvCurFile.wchMultiEdgeLines)) {

            int          edgeColumns[EDGELINE_NUM_LIMIT];
            size_t const cnt = ReadVectorFromString(Globals.fvCurFile.wchMultiEdgeLines, edgeColumns, COUNTOF(edgeColumns), 0, LONG_LINES_MARKER_LIMIT, 0, true);

            if (cnt == 0) {
                Settings.MarkLongLines = false;
            } else if (cnt == 1) {
                iLongLinesLimit = edgeColumns[0];
                Settings.MarkLongLines = true;
                //~Settings.LongLineMode = EDGE_LINE|EDGE_BACKGROUND; // set by Dlg
            } else {
                iLongLinesLimit = edgeColumns[cnt - 1];
                Settings.MarkLongLines = true;
                Settings.LongLineMode = EDGE_MULTILINE;
            }
            Globals.iWrapCol = iLongLinesLimit;
            Settings.LongLinesLimit = iLongLinesLimit;

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


    case IDS_USE_LOCALE_DATEFMT:
        Settings.PreferredLocale4DateFmt = !Settings.PreferredLocale4DateFmt;
        break;


    case IDM_SET_TABSASSPACES: {
        Settings.TabsAsSpaces = !Settings.TabsAsSpaces;
        Globals.fvCurFile.bTabsAsSpaces = Settings.TabsAsSpaces;
        SciCall_SetUseTabs(!Globals.fvCurFile.bTabsAsSpaces);
    }
    break;


    case IDM_SET_TABSETTINGS:
        if (TabSettingsDlg(hwnd,IDD_MUI_TABSETTINGS,NULL)) {
            SciCall_SetUseTabs(!Globals.fvCurFile.bTabsAsSpaces);
            SciCall_SetTabIndents(Globals.fvCurFile.bTabIndents);
            SciCall_SetBackSpaceUnIndents(Settings.BackspaceUnindents);
            SciCall_SetTabWidth(Globals.fvCurFile.iTabWidth);
            SciCall_SetIndent(Globals.fvCurFile.iIndentWidth);
            if (SciCall_GetWrapIndentMode() == SC_WRAPINDENT_FIXED) {
                _SetWrapStartIndent(Globals.hwndEdit);
            }
        }
        break;


    case IDM_VIEW_SHOWINDENTGUIDES:
        Settings.ShowIndentGuides = !Settings.ShowIndentGuides;
        Style_SetIndentGuides(Globals.hwndEdit,Settings.ShowIndentGuides);
        break;


    case IDM_SET_AUTOINDENTTEXT:
        Settings.AutoIndent = !Settings.AutoIndent;
        break;


    case IDM_SET_MULTIPLE_SELECTION:
        Settings.MultipleSelection = !Settings.MultipleSelection;
        SciCall_SetMultipleSelection(Settings.MultipleSelection);
        break;


    case IDM_VIEW_LINENUMBERS:
        Settings.ShowLineNumbers = !Settings.ShowLineNumbers;
        UpdateMargins(true);
        break;


    case IDM_VIEW_BOOKMARK_MARGIN:
        Settings.ShowBookmarkMargin = !Settings.ShowBookmarkMargin;
        UpdateMargins(true);
        break;

    case IDM_SET_AUTOCOMPLETEWORDS:
        Settings.AutoCompleteWords = !Settings.AutoCompleteWords;
        SciCall_AutoCCancel();
        break;

    case IDM_SET_AUTOCLEXKEYWORDS:
        Settings.AutoCLexerKeyWords = !Settings.AutoCLexerKeyWords;
        SciCall_AutoCCancel();
        break;

    case IDM_SET_ALTERNATE_WORD_SEPS:
        Settings.AccelWordNavigation = !Settings.AccelWordNavigation;
        EditReducedWordSeparatorSet(Globals.hwndEdit, Settings.AccelWordNavigation);
        MarkAllOccurrences(-1, true);
        break;

    case IDM_VIEW_MARKOCCUR_ONOFF:
        Settings.MarkOccurrences = !Settings.MarkOccurrences;
        if (!Settings.MarkOccurrences && FocusedView.HideNonMatchedLines) {
            EditToggleView(Globals.hwndEdit);
        }
        if (IsMarkOccurrencesEnabled()) {
            MarkAllOccurrences(_MQ_FAST, true);
        } else {
            EditClearAllOccurrenceMarkers(Globals.hwndEdit);
        }
        break;

    case IDM_VIEW_MARKOCCUR_BOOKMARKS:
        Settings.MarkOccurrencesBookmark = !Settings.MarkOccurrencesBookmark;
        SciCall_MarkerDefine(MARKER_NP3_OCCURRENCE, Settings.MarkOccurrencesBookmark ? SC_MARK_ARROWS : SC_MARK_BACKGROUND);
        break;

    case IDM_VIEW_MARKOCCUR_VISIBLE:
        Settings.MarkOccurrencesMatchVisible = !Settings.MarkOccurrencesMatchVisible;
        MarkAllOccurrences(_MQ_FAST, true);
        break;

    case IDM_VIEW_TOGGLE_VIEW:
        if (FocusedView.HideNonMatchedLines) {
            EditToggleView(Globals.hwndEdit);
            MarkAllOccurrences(_MQ_FAST, true);
        } else {
            EditToggleView(Globals.hwndEdit);
        }
        CheckCmd(GetMenu(hwnd), IDM_VIEW_TOGGLE_VIEW, FocusedView.HideNonMatchedLines);
        break;

    case IDM_VIEW_FV_FOLD:
    case IDM_VIEW_FV_BOOKMARK:
    case IDM_VIEW_FV_HIGHLIGHT:
    case IDM_VIEW_FV_BKMRKFOLD:
    case IDM_VIEW_FV_HIGHLGFOLD: {
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
        if (newSetting != Settings.FocusViewMarkerMode) {
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
    }
    break;

    case IDM_VIEW_MARKOCCUR_CASE:
        Settings.MarkOccurrencesMatchCase = !Settings.MarkOccurrencesMatchCase;
        MarkAllOccurrences(-1, true);
        break;

    case IDM_VIEW_MARKOCCUR_WNONE:
        Settings.MarkOccurrencesMatchWholeWords = false;
        Settings.MarkOccurrencesCurrentWord = false;
        MarkAllOccurrences(-1, true);
        break;

    case IDM_VIEW_MARKOCCUR_WORD:
        Settings.MarkOccurrencesMatchWholeWords = true;
        Settings.MarkOccurrencesCurrentWord = false;
        MarkAllOccurrences(-1, true);
        break;

    case IDM_VIEW_MARKOCCUR_CURRENT:
        Settings.MarkOccurrencesMatchWholeWords = false;
        Settings.MarkOccurrencesCurrentWord = true;
        MarkAllOccurrences(-1, true);
        break;

    case IDM_VIEW_FOLDING:
        Settings.ShowCodeFolding = !Settings.ShowCodeFolding;
        FocusedView.ShowCodeFolding = Settings.ShowCodeFolding;
        Style_UpdateFoldingMargin(Globals.hwndEdit, FocusedView.ShowCodeFolding);
        if (!FocusedView.ShowCodeFolding) {
            EditToggleFolds(EXPAND, true);
        }
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
        if (Settings.MatchBraces) {
            EditMatchBrace(Globals.hwndEdit);
        } else {
            SciCall_BraceHighLight(INVALID_POSITION, INVALID_POSITION);
        }
        break;

    case IDM_SET_AUTOCLOSETAGS:
        Settings.AutoCloseTags = !Settings.AutoCloseTags;
        break;

    case IDM_SET_AUTOCLOSEQUOTES:
        Settings.AutoCloseQuotes = !Settings.AutoCloseQuotes;
        break;

    case IDM_SET_AUTOCLOSEBRACKETS:
        Settings.AutoCloseBrackets = !Settings.AutoCloseBrackets;
        break;

    case IDM_VIEW_TOGGLE_HILITCURLN:
    case IDM_VIEW_HILITCURLN_NONE:
    case IDM_VIEW_HILITCURLN_BACK:
    case IDM_VIEW_HILITCURLN_FRAME: {
        int const set = iLoWParam - IDM_VIEW_HILITCURLN_NONE;
        Settings.HighlightCurrentLine = (set >= 0) ? set : ((Settings.HighlightCurrentLine + 1) % 3);
        Style_HighlightCurrentLine(Globals.hwndEdit, Settings.HighlightCurrentLine);
    }
    break;

    case IDM_VIEW_CHGHIST_NONE:
    case IDM_VIEW_CHGHIST_MARGIN:
    case IDM_VIEW_CHGHIST_DOCTXT:
    case IDM_VIEW_CHGHIST_ALL: {
        int const set = iLoWParam - IDM_VIEW_CHGHIST_NONE;
        switch (set) {
        case 0: 
            Settings.ChangeHistoryMode = SC_CHANGE_HISTORY_DISABLED;
            break;
        case 1:
            Settings.ChangeHistoryMode = SC_CHANGE_HISTORY_ENABLED | SC_CHANGE_HISTORY_MARKERS;
            break;
        case 2: 
            Settings.ChangeHistoryMode = SC_CHANGE_HISTORY_ENABLED | SC_CHANGE_HISTORY_INDICATORS;
            break;
        case 3: 
            Settings.ChangeHistoryMode = SC_CHANGE_HISTORY_ENABLED | SC_CHANGE_HISTORY_INDICATORS | SC_CHANGE_HISTORY_MARKERS;
            break;
        default:
            break;
        }
        _ApplyChangeHistoryMode();
        break;
    } 
    break;

    case IDM_VIEW_CHGHIST_TOGGLE_MARGIN:
        if (Settings.ChangeHistoryMode & SC_CHANGE_HISTORY_MARKERS) {
            Settings.ChangeHistoryMode &= ~SC_CHANGE_HISTORY_MARKERS;
            if (!(Settings.ChangeHistoryMode & SC_CHANGE_HISTORY_INDICATORS)) {
                Settings.ChangeHistoryMode = SC_CHANGE_HISTORY_DISABLED;
            }
        }
        else {
            Settings.ChangeHistoryMode |= (SC_CHANGE_HISTORY_ENABLED | SC_CHANGE_HISTORY_MARKERS);
        }
        _ApplyChangeHistoryMode();
        break;

    case IDM_VIEW_CHGHIST_CLEAR_UNDOREDO:
        if (IsYesOkay(InfoBoxLng(MB_YESNO | MB_ICONWARNING, L"AllowClearUndoHistory", IDS_MUI_ASK_CLEAR_UNDO))) {
            UndoRedoReset();
            UpdateToolbar();
            UpdateMargins(true);
        }
        break;

    case IDM_VIEW_HYPERLINKHOTSPOTS:
        Settings.HyperlinkHotspot = !Settings.HyperlinkHotspot;
        EditUpdateVisibleIndicators();
        ResetMouseDWellTime();
        break;

    case IDM_VIEW_SHOW_HYPLNK_CALLTIP:
        Settings.ShowHypLnkToolTip = !Settings.ShowHypLnkToolTip;
        ResetMouseDWellTime();
        break;

    case IDM_VIEW_COLORDEFHOTSPOTS:
    case IDM_VIEW_COLOR_ARGB:
    case IDM_VIEW_COLOR_RGBA:
    case IDM_VIEW_COLOR_BGRA: {
        Settings.ColorDefHotspot = iLoWParam - IDM_VIEW_COLORDEFHOTSPOTS;
        EditUpdateVisibleIndicators();
        ResetMouseDWellTime();
    }
    break;

    case IDM_VIEW_UNICODE_POINTS:
        Settings.HighlightUnicodePoints = !Settings.HighlightUnicodePoints;
        EditUpdateVisibleIndicators();
        ResetMouseDWellTime();
        break;

    case IDM_VIEW_ZOOMIN: {
        SciCall_ZoomIn();
        ShowZoomCallTip();
    }
    break;

    case IDM_VIEW_ZOOMOUT: {
        SciCall_ZoomOut();
        ShowZoomCallTip();
    }
    break;

    case IDM_VIEW_RESETZOOM: {
        SciCall_SetZoom(100);
        ShowZoomCallTip();
    }
    break;

    case IDM_VIEW_CHASING_DOCTAIL: {

        InstallFileWatching(false);

        static DocPos _lastCaretPos = -1;
        if (_lastCaretPos == -1) {
            _lastCaretPos = SciCall_GetCurrentPos();
        }
        static FILE_WATCHING_MODE _saveChgNotify = FWM_NO_INIT;
        if (_saveChgNotify == FWM_NO_INIT) {
            _saveChgNotify = FileWatching.FileWatchingMode;    
        }

        FileWatching.MonitoringLog = !FileWatching.MonitoringLog; // toggle

        if (FileWatching.MonitoringLog) {
            SetForegroundWindow(hwnd);
            _lastCaretPos = SciCall_GetCurrentPos();
            _saveChgNotify = FileWatching.FileWatchingMode;
            FileWatching.FileWatchingMode = FWM_AUTORELOAD;
            SciCall_SetEndAtLastLine(false); // false(!)
            FileRevert(Paths.CurrentFile, true);
            SciCall_SetReadOnly(FileWatching.MonitoringLog);
        }
        else {
            FileWatching.FileWatchingMode = _saveChgNotify;
            SciCall_SetEndAtLastLine(!Settings.ScrollPastEOF);
            SciCall_SetReadOnly(Settings.DocReadOnlyMode);
            SciCall_GotoPos(_lastCaretPos);
        }

        InstallFileWatching(true);

        CheckCmd(GetMenu(Globals.hwndMain), IDM_VIEW_CHASING_DOCTAIL, FileWatching.MonitoringLog);
    }
    break;

    case IDM_VIEW_SCROLLPASTEOF:
        Settings.ScrollPastEOF = !Settings.ScrollPastEOF;
        SciCall_SetEndAtLastLine(!Settings.ScrollPastEOF);
        break;

    case IDM_VIEW_TITLEBAR:
        Settings.ShowTitlebar = !Settings.ShowTitlebar;
        if (Settings.ShowTitlebar) {
            SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) | WS_CAPTION);
            UpdateTitlebar(hwnd);
        }
        else {
            SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~WS_CAPTION);
        }
        UpdateUI(hwnd);      
        break;

    case IDM_VIEW_MENUBAR:
        Settings.ShowMenubar = !Settings.ShowMenubar;
        SetMenu(hwnd, (Settings.ShowMenubar ? Globals.hMainMenu : NULL));
        DrawMenuBar(Globals.hwndMain);
        break;

    case IDM_VIEW_TOOLBAR:
        Settings.ShowToolbar = !Settings.ShowToolbar;
        ShowWindow(Globals.hwndRebar, (Settings.ShowToolbar ? SW_SHOWDEFAULT : SW_HIDE));
        UpdateUI(hwnd);
        break;

    case IDM_VIEW_CUSTOMIZETB:
        SendMessage(Globals.hwndToolbar,TB_CUSTOMIZE,0,0);
        UpdateUI(hwnd);
        break;

    case IDM_VIEW_TOGGLETB:
        Settings.ToolBarTheme = (Settings.ToolBarTheme + 1) % 3;
        CreateBars(hwnd, Globals.hInstance);
        UpdateUI(hwnd);
        break;

    case IDM_VIEW_LOADTHEMETB:
        if (SelectExternalToolBar(hwnd)) {
            CreateBars(hwnd, Globals.hInstance);
            UpdateUI(hwnd);
        }
        break;

    case IDM_VIEW_DPISCALETB:
        Settings.DpiScaleToolBar = !Settings.DpiScaleToolBar;
        SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
        break;

    case IDM_VIEW_STATUSBAR:
        Settings.ShowStatusbar = !Settings.ShowStatusbar;
        ShowWindow(Globals.hwndStatus, (Settings.ShowStatusbar ? SW_SHOWDEFAULT : SW_HIDE));
        UpdateUI(hwnd);
        break;


    case IDM_VIEW_STICKYWINPOS:
        if (IsCmdEnabled(hwnd, IDM_VIEW_STICKYWINPOS)) {
            Flags.bStickyWindowPosition = !Flags.bStickyWindowPosition; // toggle

            if (Flags.bStickyWindowPosition) {
                InfoBoxLng(MB_OK, L"MsgStickyWinPos", IDS_MUI_STICKYWINPOS);
            }

            if (OpenSettingsFile("IDM_VIEW_STICKYWINPOS")) {

                SaveWindowPositionSettings(!Flags.bStickyWindowPosition);

                if (Flags.bStickyWindowPosition != DefaultFlags.bStickyWindowPosition) {
                    if (Globals.bCanSaveIniFile) {
                        IniSectionSetBool(Constants.Settings2_Section, L"StickyWindowPosition", Flags.bStickyWindowPosition);
                    }
                }
                else {
                    IniSectionDelete(Constants.Settings2_Section, L"StickyWindowPosition", false);
                }
                CloseSettingsFile("IDM_VIEW_STICKYWINPOS", true);
            }
        }
        break;


    case IDM_SET_REUSEWINDOW:
        if (IsCmdEnabled(hwnd, IDM_SET_REUSEWINDOW)) {
            Flags.bReuseWindow = !Flags.bReuseWindow; // reverse
            if (Globals.bCanSaveIniFile) {
                if (Flags.bReuseWindow != DefaultFlags.bReuseWindow) {
                    IniFileSetBool(Paths.IniFile, Constants.Settings2_Section, L"ReuseWindow", Flags.bReuseWindow);
                } else {
                    IniFileDelete(Paths.IniFile, Constants.Settings2_Section, L"ReuseWindow", false);
                }
            }
        }
        break;


    case IDM_SET_SINGLEFILEINSTANCE:
        if (IsCmdEnabled(hwnd, IDM_SET_SINGLEFILEINSTANCE)) {
            Flags.bSingleFileInstance = !Flags.bSingleFileInstance; // reverse
            if (Globals.bCanSaveIniFile) {
                if (Flags.bSingleFileInstance != DefaultFlags.bSingleFileInstance) {
                    IniFileSetBool(Paths.IniFile, Constants.Settings2_Section, L"SingleFileInstance", Flags.bSingleFileInstance);
                } else {
                    IniFileDelete(Paths.IniFile, Constants.Settings2_Section, L"SingleFileInstance", false);
                }
            }
        }
        break;


    case IDM_SET_ALWAYSONTOP:
        if (Settings.AlwaysOnTop) {
            SetWindowPos(hwnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
            Settings.AlwaysOnTop = false;
        } else {
            SetWindowPos(hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
            Settings.AlwaysOnTop = true;
        }
        UpdateToolbar();
        break;


    case IDM_SET_MINTOTRAY:
        Settings.MinimizeToTray = !Settings.MinimizeToTray;
        break;


    case IDM_SET_TRANSPARENT:
        Settings.TransparentMode = !Settings.TransparentMode;
        SetWindowTransparentMode(hwnd,Settings.TransparentMode, Settings2.OpacityLevel);
        break;


    case IDM_SET_RENDER_TECH_GDI:
    case IDM_SET_RENDER_TECH_D2D:
    case IDM_SET_RENDER_TECH_D2DRETAIN:
    case IDM_SET_RENDER_TECH_D2DDC: {
        int const prevRT = SciCall_GetTechnology();
        Settings.RenderingTechnology = (iLoWParam - IDM_SET_RENDER_TECH_GDI);
        if (prevRT != Settings.RenderingTechnology) {
            SciCall_SetTechnology(Settings.RenderingTechnology);
            Settings.RenderingTechnology = SciCall_GetTechnology(); // switched ?
        }
        int const prevBD = SciCall_GetBidirectional();
        if (prevBD != Settings.Bidirectional) {
            SciCall_SetBidirectional(Settings.Bidirectional);
            Settings.Bidirectional = SciCall_GetBidirectional(); // switched ?
        }
        if ((prevRT != Settings.RenderingTechnology) || (prevBD != Settings.Bidirectional)) {
            Style_ResetCurrentLexer(Globals.hwndEdit);
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
    case IDM_SET_BIDIRECTIONAL_R2L: {
        SciCall_SetBidirectional(iLoWParam - IDM_SET_BIDIRECTIONAL_NONE);
        Settings.Bidirectional = SciCall_GetBidirectional();
    }
    break;

#ifdef D_NP3_WIN10_DARK_MODE

    case IDM_VIEW_WIN_DARK_MODE: {

        if (!IsYesOkay(InfoBoxLng(MB_OKCANCEL | MB_ICONWARNING, L"MsgResetScheme", IDS_MUI_WARN_STYLE_RESET))) {
           break;
        }

        Settings.WinThemeDarkMode = IsSettingDarkMode() ? WINDSPMOD_LIGHT : WINDSPMOD_DARK; // toggle non auto!
        SetDarkMode(IsSettingDarkMode());

        Style_DynamicThemesMenuCmd(IDM_THEMES_FACTORY_RESET);

        if (IsWindow(Globals.hwndDlgFindReplace)) {
            //~SendMessage(Globals.hwndDlgFindReplace, WM_THEMECHANGED, 0, 0); ~ (!) incomplete update
            bool const isReplDlg = !!GetDlgItem(Globals.hwndDlgFindReplace, IDC_REPLACE);
            PostWMCommand(hwnd, isReplDlg ? IDM_EDIT_FIND : IDM_EDIT_REPLACE); // swap
            PostWMCommand(hwnd, isReplDlg ? IDM_EDIT_REPLACE : IDM_EDIT_FIND); // restore
            PostMessage(hwnd, WM_SETFOCUS, 0, 0);
        }

        if (IsWindow(Globals.hwndDlgCustomizeSchemes)) {
            //~SendMessage(Globals.hwndDlgCustomizeSchemes, WM_CLOSE, 0, 0); ~ no need for restart
            //~PostWMCommand(hwnd, IDM_VIEW_SCHEMECONFIG);
            SendMessage(Globals.hwndDlgCustomizeSchemes, WM_THEMECHANGED, 0, 0);
            //PostMessage(Globals.hwndDlgCustomizeSchemes, WM_NCACTIVATE, FALSE, -1); // (!)
            //PostMessage(Globals.hwndDlgCustomizeSchemes, WM_NCACTIVATE, TRUE, 0);
            PostMessage(hwnd, WM_SETFOCUS, 0, 0);
        }

        PostMessage(hwnd, WM_THEMECHANGED, 0, 0);
    }
    break;

#endif

    case IDM_SET_MUTE_MESSAGEBEEP:
        Settings.MuteMessageBeep = !Settings.MuteMessageBeep;
        break;

    case IDM_VIEW_SPLIT_UNDOTYPSEQ_LNBRK:
        Settings.SplitUndoTypingSeqOnLnBreak = !Settings.SplitUndoTypingSeqOnLnBreak;
        break;

    //case IDM_SET_INLINE_IME:
    //  Settings2.IMEInteraction = (Settings2.IMEInteraction == SC_IME_WINDOWED) ? SC_IME_INLINE : SC_IME_WINDOWED;
    //  SciCall_SetIMEInteraction(Settings2.IMEInteraction);
    //  break;

    case IDM_SET_SHOWFILENAMEONLY:
    case IDM_SET_SHOWFILENAMEFIRST:
    case IDM_SET_SHOWFULLPATH:
        Settings.PathNameFormat = iLoWParam - IDM_SET_SHOWFILENAMEONLY;
        StringCchCopy(s_wchTitleExcerpt,COUNTOF(s_wchTitleExcerpt),L"");
        UpdateTitlebar(hwnd);
        break;


    case IDM_SET_SHOWEXCERPT:
        EditGetExcerpt(Globals.hwndEdit,s_wchTitleExcerpt,COUNTOF(s_wchTitleExcerpt));
        UpdateTitlebar(hwnd);
        break;


    case IDM_SET_NOSAVERECENT:
        Settings.SaveRecentFiles = !Settings.SaveRecentFiles;
        break;


    case IDM_SET_NOPRESERVECARET:
        Settings.PreserveCaretPos = !Settings.PreserveCaretPos;
        break;


    case IDM_SET_NOSAVEFINDREPL:
        Settings.SaveFindReplace = !Settings.SaveFindReplace;
        break;


    case IDM_SET_AUTOLOAD_MRU_FILE:
        Settings.AutoLoadMRUFile = !Settings.AutoLoadMRUFile;
        break;


    case IDM_SET_SAVEBEFORERUNNINGTOOLS:
        Settings.SaveBeforeRunningTools = !Settings.SaveBeforeRunningTools;
        break;

    case IDM_SET_EVALTINYEXPRONSEL:
        Settings.EvalTinyExprOnSelection = !Settings.EvalTinyExprOnSelection;
        UpdateStatusbar(false);
        break;

    case IDM_SET_CHANGENOTIFY:
        InstallFileWatching(false);
        if (ChangeNotifyDlg(hwnd)) {
            InstallFileWatching(true);
        }
        break;

    case IDM_SET_AUTOSAVE_BACKUP: {
        int const periodSav = Settings.AutoSaveInterval;
        if (AutoSaveBackupSettingsDlg(hwnd)) {
            if (Settings.AutoSaveOptions & ASB_Periodic) {
                AutoSaveStart(periodSav != Settings.AutoSaveInterval);
            }
            else {
                AutoSaveStop();
            }
        }
    }
    break;

    case IDM_SET_NOESCFUNC:
    case IDM_SET_ESCMINIMIZE:
    case IDM_SET_ESCEXIT:
        Settings.EscFunction = iLoWParam - IDM_SET_NOESCFUNC;
        break;


    case IDM_SET_SAVESETTINGS:
        if (IsCmdEnabled(hwnd, IDM_SET_SAVESETTINGS)) {
            Settings.SaveSettings = !Settings.SaveSettings;
            if (Globals.bCanSaveIniFile) {
                if (Settings.SaveSettings == Defaults.SaveSettings) {
                    IniFileDelete(Paths.IniFile, Constants.Settings_Section, L"SaveSettings", false);
                } else {
                    IniFileSetBool(Paths.IniFile, Constants.Settings_Section, L"SaveSettings", Settings.SaveSettings);
                }
            }
        }
        break;


    case IDM_SET_SAVESETTINGSNOW:
        if (IsCmdEnabled(hwnd, IDM_SET_SAVESETTINGSNOW)) {
            CmdSaveSettingsNow();
        }
        break;


    case IDM_HELP_ONLINEDOCUMENTATION:
        ShellExecute(0, 0, ONLINE_HELP_WEBSITE, 0, 0, SW_SHOW);
        break;

    case IDM_HELP_ABOUT: {
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
            SetSaveNeeded(true);
        }
        break;

    case IDM_HELP_CMD:
        DisplayCmdLineHelp(hwnd);
        break;


    case CMD_ESCAPE: {

        DocLn const  vis1stLine = SciCall_GetFirstVisibleLine();
        DocPos const iCurPos = SciCall_GetCurrentPos();

        int skipLevel = Settings2.ExitOnESCSkipLevel;

        if (SciCall_AutoCActive()) {
            SciCall_AutoCCancel();
            --skipLevel;
        } else if (SciCall_CallTipActive()) {
            Sci_CallTipCancelEx();
            s_bCallTipEscDisabled = true;
            --skipLevel;
        } else if (s_bInMultiEditMode) {
            //~UndoTransActionBegin();
            SciCall_SetIndicatorCurrent(INDIC_NP3_MULTI_EDIT);
            SciCall_IndicatorClearRange(0, Sci_GetDocEndPosition());
            SciCall_ClearSelections();
            //~EndUndoTransAction();
            SciCall_GotoPos(iCurPos);
            SciCall_SetFirstVisibleLine(vis1stLine);
            s_bInMultiEditMode = false;
            --skipLevel;
        }

        if ((!SciCall_IsSelectionEmpty() || Sci_IsMultiOrRectangleSelection()) && (skipLevel == Settings2.ExitOnESCSkipLevel)) {
            Sci_GotoPosChooseCaret(iCurPos);
            skipLevel -= Default_ExitOnESCSkipLevel;
        }

        if ((skipLevel < 0) || (skipLevel == Settings2.ExitOnESCSkipLevel)) {
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
        FileSave(FSF_EndSession);
    case IDT_FILE_EXIT:
        CloseApplication();
        break;


    case CMD_INSERTNEWLINE: {
        UndoTransActionBegin();
        const DocPos iPos = SciCall_GetCurrentPos();
        const DocLn iLine = SciCall_LineFromPosition(iPos);
        if (iLine <= 0) {
            SciCall_GotoLine(0);
            SciCall_NewLine();
            SciCall_GotoLine(0);
        } else {
            SciCall_GotoPos(SciCall_GetLineEndPosition(iLine - 1));
            SciCall_NewLine();
        }
        EndUndoTransAction();
    }
    break;

    case CMD_ENTER_RETURN: {
        _EvalTinyExpr(false);
        SciCall_NewLine();
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
        //~UndoTransActionBegin();
        EditDeleteMarkerInSelection();
        SciCall_Clear();
        //~EndUndoTransAction();
        break;


    case CMD_ARROW_UP:
        if (Sci_IsMultiSelection()) {
            SciCall_Cancel();
        }
        SciCall_LineUp();
        break;

    case CMD_ARROW_DOWN:
        if (Sci_IsMultiSelection()) {
            SciCall_Cancel();
        }
        SciCall_LineDown();
        break;


    case CMD_SCROLLUP:
        if (Sci_IsMultiSelection()) {
            SciCall_LineUpExtend();
        } else {
            //~SciCall_LineScrollUp();
            SciCall_LineScroll(0, -1LL);
        }
        break;


    case CMD_SCROLLDOWN:
        if (Sci_IsMultiSelection()) {
            SciCall_LineDownExtend();
        } else {
            //~SciCall_LineScrollDown();
            SciCall_LineScroll(0, 1LL);
        }
        break;


    case CMD_CTRLLEFT:
        if (Sci_IsMultiSelection()) {
            SciCall_CharLeftExtend();
        } else {
            SciCall_WordLeft();
        }
        break;


    case CMD_CTRLRIGHT:
        if (Sci_IsMultiSelection()) {
            SciCall_CharRightExtend();
        } else {
            SciCall_WordRight();
        }
        break;


    case CMD_PARAGRAPH_UP:
        if (!SciCall_IsSelectionEmpty() && Sci_IsStreamSelection()) {
            SciCall_ParaUpExtend();
        } else {
            SciCall_ParaUp();
        }
        break;


    case CMD_PARAGRAPH_DOWN:
        if (!SciCall_IsSelectionEmpty() && Sci_IsStreamSelection()) {
            SciCall_ParaDownExtend();
        } else {
            SciCall_ParaDown();
        }
        break;


    case CMD_CTRLBACK: {
        DocPos const iPos = SciCall_GetCurrentPos();
        DocPos const iAnchor = SciCall_GetAnchor();
        DocLn  const iLine = SciCall_LineFromPosition(iPos);
        DocPos const iStartPos = SciCall_PositionFromLine(iLine);
        DocPos const iIndentPos = SciCall_GetLineIndentPosition(iLine);

        if (iPos != iAnchor) {
            UndoTransActionBegin();
            SciCall_GotoPos(iPos);
            EndUndoTransAction();
        } else {
            if (iPos == iStartPos) {
                SciCall_DeleteBack();
            } else if (iPos <= iIndentPos) {
                SciCall_DelLineLeft();
            } else {
                SciCall_DelWordLeft();
            }
        }
    }
    break;


    case CMD_CTRLDEL: {
        const DocPos iPos = SciCall_GetCurrentPos();
        const DocPos iAnchor = SciCall_GetAnchor();
        const DocLn iLine = SciCall_LineFromPosition(iPos);
        const DocPos iStartPos = SciCall_PositionFromLine(iLine);
        const DocPos iEndPos = SciCall_GetLineEndPosition(iLine);

        if (iPos != iAnchor) {
            UndoTransActionBegin();
            SciCall_GotoPos(iPos);
            EndUndoTransAction();
        } else {
            if (iStartPos != iEndPos) {
                SciCall_DelWordRight();
            } else { // iStartPos == iEndPos
                UserMarkerDeleteAll(Sci_GetCurrentLineNumber());
                SciCall_LineDelete();
            }
        }
    }
    break;


    case CMD_RECODEDEFAULT: {
        if (Path_IsNotEmpty(Paths.CurrentFile)) {
            Encoding_Forced(Settings.DefaultEncoding);
            FileLoadFlags const fLoadFlags = FLF_Reload | FLF_SkipUnicodeDetect | FLF_SkipANSICPDetection;
            FileLoad(Paths.CurrentFile, fLoadFlags, SciCall_GetCurrentPos(), SciCall_GetFirstVisibleLine());
        }
    }
    break;


    case CMD_RECODEANSI: {
        if (Path_IsNotEmpty(Paths.CurrentFile)) {
            Encoding_Forced(CPI_ANSI_DEFAULT);
            FileLoadFlags const fLoadFlags = FLF_Reload | FLF_SkipUnicodeDetect | FLF_SkipANSICPDetection;
            FileLoad(Paths.CurrentFile, fLoadFlags, SciCall_GetCurrentPos(), SciCall_GetFirstVisibleLine());
        }
    }
    break;


    case CMD_RECODEOEM: {
        if (Path_IsNotEmpty(Paths.CurrentFile)) {
            Encoding_Forced(CPI_OEM);
            FileLoadFlags const fLoadFlags = FLF_Reload | FLF_SkipUnicodeDetect | FLF_SkipANSICPDetection;
            FileLoad(Paths.CurrentFile, fLoadFlags, SciCall_GetCurrentPos(), SciCall_GetFirstVisibleLine());
        }
    }
    break;


    case CMD_RECODEGB18030: {
        if (Path_IsNotEmpty(Paths.CurrentFile)) {
            Encoding_Forced(Encoding_GetByCodePage(54936)); // GB18030
            FileLoadFlags const fLoadFlags = FLF_Reload | FLF_SkipUnicodeDetect | FLF_SkipANSICPDetection;
            FileLoad(Paths.CurrentFile, fLoadFlags, SciCall_GetCurrentPos(), SciCall_GetFirstVisibleLine());
        }
    }
    break;


    case CMD_RELOADASCIIASUTF8: {
        if (Path_IsNotEmpty(Paths.CurrentFile)) {
            Encoding_Forced(CPI_UTF8);
            FileLoadFlags const fLoadFlags = FLF_Reload | FLF_SkipUnicodeDetect | FLF_SkipANSICPDetection;
            FileLoad(Paths.CurrentFile, fLoadFlags, SciCall_GetCurrentPos(), SciCall_GetFirstVisibleLine());
        }
    }
    break;


    case CMD_RELOADFORCEDETECTION: {
        if (Path_IsNotEmpty(Paths.CurrentFile)) {
            Encoding_Forced(CPI_NONE);
            FileLoadFlags const fLoadFlags = FLF_Reload | FLF_ForceEncDetection;
            FileLoad(Paths.CurrentFile, fLoadFlags, SciCall_GetCurrentPos(), SciCall_GetFirstVisibleLine());
        }
    }
    break;


    case CMD_RELOADNOENCODETAGS: {
        if (Path_IsNotEmpty(Paths.CurrentFile)) {
            bool const _bNoEncodingTags = Settings.NoEncodingTags;
            Settings.NoEncodingTags = true;
            FileLoadFlags fLoadFlags = FLF_Reload;
            fLoadFlags |= Settings.SkipUnicodeDetection ? FLF_SkipUnicodeDetect : 0;
            fLoadFlags |= Settings.SkipANSICodePageDetection ? FLF_SkipANSICPDetection : 0;
            FileLoad(Paths.CurrentFile, fLoadFlags, SciCall_GetCurrentPos(), SciCall_GetFirstVisibleLine());
            Settings.NoEncodingTags = _bNoEncodingTags;
        }
    }
    break;


    case CMD_IGNORE_FILE_VARS:
        Flags.NoFileVariables = !Flags.NoFileVariables;
        if (Globals.bCanSaveIniFile) {
            if (Flags.NoFileVariables != DefaultFlags.NoFileVariables) {
                IniFileSetBool(Paths.IniFile, Constants.Settings2_Section, L"NoFileVariables", Flags.NoFileVariables);
            } else {
                IniFileDelete(Paths.IniFile, Constants.Settings2_Section, L"NoFileVariables", false);
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
    case CMD_WEBACTION2: {

        const wchar_t* const wchWebTemplate = ((iLoWParam == CMD_WEBACTION1) ? StrgGet(Settings2.WebTemplate1) : StrgGet(Settings2.WebTemplate2));

        if (StrIsNotEmpty(wchWebTemplate)) {

            WCHAR        wszSelection[STRINGW_MAX_URL_LENGTH + 1] = { L'\0' };
            size_t const cchSelection = EditGetSelectedText(wszSelection, COUNTOF(wszSelection));

            if (1 < cchSelection) {

                HPATHL hdir = Path_Copy(Paths.CurrentFile);
                Path_RemoveFileSpec(hdir);

                // Check lpszSelection and truncate bad WCHARs
                WCHAR* lpsz = StrChr(wszSelection, L'\r');
                if (lpsz) {
                    *lpsz = L'\0';
                }

                lpsz = StrChr(wszSelection, L'\n');
                if (lpsz) {
                    *lpsz = L'\0';
                }

                lpsz = StrChr(wszSelection, L'\t');
                if (lpsz) {
                    *lpsz = L'\0';
                }

                HSTRINGW hstr_hyplnk = StrgCreate(NULL);
                StrgFormat(hstr_hyplnk, wchWebTemplate, wszSelection);
                ExpandEnvironmentStrgs(hstr_hyplnk, false);

                if (StrgIsNotEmpty(Settings2.HyperlinkShellExURLWithApp))
                {
                    HSTRINGW hstr_params = StrgCopy(Settings2.HyperlinkShellExURLCmdLnArgs);
                    StrgReplace(hstr_params, URLPLACEHLDR, StrgGet(hstr_hyplnk));

                    SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
                    sei.fMask = SEE_MASK_DEFAULT;
                    sei.hwnd = NULL;
                    sei.lpVerb = NULL;
                    sei.lpFile = StrgGet(Settings2.HyperlinkShellExURLWithApp);
                    sei.lpParameters = StrgGet(hstr_params);
                    sei.lpDirectory = Path_Get(hdir);
                    sei.nShow = SW_SHOWNORMAL;
                    ShellExecuteExW(&sei);

                    StrgDestroy(hstr_params);
                }
                else {
                    SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
                    sei.fMask = SEE_MASK_NOZONECHECKS;
                    sei.hwnd = NULL;
                    sei.lpVerb = NULL;
                    sei.lpFile = StrgGet(hstr_hyplnk);
                    sei.lpParameters = NULL;
                    sei.lpDirectory = Path_Get(hdir);
                    sei.nShow = SW_SHOWNORMAL;
                    ShellExecuteExW(&sei);
                }
                StrgDestroy(hstr_hyplnk);
                Path_Release(hdir);
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
            SciCall_SetEdgeColumn(Settings.LongLinesLimit);
            //Globals.fvCurFile.iLongLinesLimit = Settings.LongLinesLimit;
          }
          break;
    ~~~ */
        

    case CMD_STRINGIFY: {
        EditEncloseSelection(L"'", L"'");
    }
    break;


    case CMD_STRINGIFY2: {
        EditEncloseSelection(L"\"", L"\"");
    }
    break;


    case CMD_EMBRACE: {
        EditEncloseSelection(L"(", L")");
    }
    break;


    case CMD_EMBRACE2: {
        EditEncloseSelection(L"[", L"]");
    }
    break;


    case CMD_EMBRACE3: {
        EditEncloseSelection(L"{", L"}");
    }
    break;


    case CMD_EMBRACE4: {
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
        UpdateTitlebar(hwnd);
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
        if (Path_IsNotEmpty(Paths.CurrentFile)) {
            SetClipboardText(hwnd, Path_Get(Paths.CurrentFile), Path_GetLength(Paths.CurrentFile));
        } else {
            WCHAR tchUntitled[64] = { L'\0' };
            GetLngString(IDS_MUI_UNTITLED, tchUntitled, COUNTOF(tchUntitled));
            SetClipboardText(hwnd, tchUntitled, StringCchLen(tchUntitled, 0));
        }
    }
    break;


    case CMD_COPYWINPOS: {
        WININFO wi = GetMyWindowPlacement(Globals.hwndMain, NULL, 0, false);
        WCHAR   wchBuf[128] = { L'\0' };
        StringCchPrintf(wchBuf, COUNTOF(wchBuf), L"/pos " WINDOWPOS_STRGFORMAT, wi.x, wi.y, wi.cx, wi.cy, wi.dpi, (int)wi.max);
        SetClipboardText(hwnd, wchBuf, StringCchLen(wchBuf, 0));
    }
    break;


    case CMD_INITIALWINPOS:
        SnapToWinInfoPos(hwnd, g_IniWinInfo, SCR_NORMAL, SW_SHOWDEFAULT);
        break;

    case CMD_FULLSCRWINPOS: {
        WININFO wi = GetMyWindowPlacement(hwnd, NULL, 0, false);
        SnapToWinInfoPos(hwnd, wi, SCR_FULL_SCREEN, SW_SHOWDEFAULT);
    }
    break;

    case CMD_DEFAULTWINPOS:
        SnapToWinInfoPos(hwnd, g_DefWinInfo, SCR_NORMAL, SW_SHOWDEFAULT);
        break;

    case CMD_SAVEASDEFWINPOS: {
        WININFO const wi = GetMyWindowPlacement(hwnd, NULL, 0, false);
        StringCchPrintf(Settings2.DefaultWindowPosition, COUNTOF(Settings2.DefaultWindowPosition), 
                        WINDOWPOS_STRGFORMAT, wi.x, wi.y, wi.cx, wi.cy, wi.dpi, (int)wi.max);
        if (Globals.bCanSaveIniFile) {
            // overwrite bad defined default position
            _IniFileWriteDefaultWindowPosition(wi);
        }
        g_DefWinInfo = wi; //~GetWinInfoByFlag(-1); // use current win pos as new default
    }
    break;

    case CMD_CLEARSAVEDWINPOS:
        g_DefWinInfo = GetFactoryDefaultWndPos(hwnd, 2);
        StringCchCopy(Settings2.DefaultWindowPosition, COUNTOF(Settings2.DefaultWindowPosition), L"");
        WCHAR tchScrnDim[64] = { L'\0' };
        StringCchPrintf(tchScrnDim, COUNTOF(tchScrnDim), L"%ix%i " DEF_WIN_POSITION_STRG,
            GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN));
        IniFileDelete(Paths.IniFile, Constants.Window_Section, tchScrnDim, true);
        IniFileDelete(Paths.IniFile, Constants.Settings2_Section, DEF_WIN_POSITION_STRG, true);
        break;

    case CMD_OPENINIFILE:
        if (Path_IsNotEmpty(Paths.IniFile)) {
            SaveAllSettings(false);
            FileLoadFlags const fLoadFlags = FLF_SkipANSICPDetection;
            FileLoad(Paths.IniFile, fLoadFlags, 0, 0);
        }
        break;

    case CMD_OPEN_HYPERLINK:
        HandleHotSpotURLClicked(SciCall_GetCurrentPos(), (OPEN_WITH_BROWSER | OPEN_IN_NOTEPAD3));
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
        if (IsCmdEnabled(hwnd,IDM_FILE_NEW)) {
            SendWMCommand(hwnd, IDM_FILE_NEW);
        } else {
            SimpleBeep();
        }
        break;


    case IDT_VIEW_NEW_WINDOW:
        if (IsCmdEnabled(hwnd, IDM_FILE_NEWWINDOW)) {
            SendWMCommand(hwnd, IDM_FILE_NEWWINDOW);
        } else {
            SimpleBeep();
        }
        break;


    case IDT_FILE_OPEN:
        if (IsCmdEnabled(hwnd,IDM_FILE_OPEN)) {
            SendWMCommand(hwnd, IDM_FILE_OPEN);
        } else {
            SimpleBeep();
        }
        break;


    case IDT_FILE_BROWSE:
        if (IsCmdEnabled(hwnd,IDM_FILE_BROWSE)) {
            SendWMCommand(hwnd, IDM_FILE_BROWSE);
        } else {
            SimpleBeep();
        }
        break;


    case IDT_FILE_RECENT:
        if (IsCmdEnabled(hwnd,IDM_FILE_RECENT)) {
            SendWMCommand(hwnd, IDM_FILE_RECENT);
        } else {
            SimpleBeep();
        }
        break;

    case IDT_FILE_SAVE:
        if (IsCmdEnabled(hwnd,IDM_FILE_SAVE)) {
            SendWMCommand(hwnd, IDM_FILE_SAVE);
        } else {
            SimpleBeep();
        }
        break;


    case IDT_EDIT_UNDO:
        if (IsCmdEnabled(hwnd,IDM_EDIT_UNDO)) {
            SendWMCommand(hwnd, IDM_EDIT_UNDO);
        } else {
            SimpleBeep();
        }
        break;


    case IDT_EDIT_REDO:
        if (IsCmdEnabled(hwnd,IDM_EDIT_REDO)) {
            SendWMCommand(hwnd, IDM_EDIT_REDO);
        } else {
            SimpleBeep();
        }
        break;


    case IDT_EDIT_CUT:
        if (IsCmdEnabled(hwnd,IDM_EDIT_CUT)) {
            SendWMCommand(hwnd, IDM_EDIT_CUT);
            //~SendWMCommand(hwnd, IDM_EDIT_CUTLINE);
        } else {
            SimpleBeep();
        }
        break;


    case IDT_EDIT_COPY:
        if (IsCmdEnabled(hwnd,IDM_EDIT_COPY)) {
            SendWMCommand(hwnd, IDM_EDIT_COPY);
        } else {
            SendWMCommand(hwnd, IDM_EDIT_COPYALL);
        }
        break;


    case IDT_EDIT_PASTE:
        if (IsCmdEnabled(hwnd,IDM_EDIT_PASTE)) {
            SendWMCommand(hwnd, IDM_EDIT_PASTE);
        } else {
            SimpleBeep();
        }
        break;


    case IDT_EDIT_FIND:
        if (IsCmdEnabled(hwnd,IDM_EDIT_FIND)) {
            SendWMCommand(hwnd, IDM_EDIT_FIND);
        } else {
            SimpleBeep();
        }
        break;


    case IDT_EDIT_REPLACE:
        if (IsCmdEnabled(hwnd,IDM_EDIT_REPLACE)) {
            SendWMCommand(hwnd, IDM_EDIT_REPLACE);
        } else {
            SimpleBeep();
        }
        break;


    case IDT_GREP_WIN_TOOL:
        if (IsCmdEnabled(hwnd, IDM_GREP_WIN_SEARCH)) {
            SendWMCommand(hwnd, IDM_GREP_WIN_SEARCH);
        } else {
            SimpleBeep();
        }
        break;


    case IDT_VIEW_WORDWRAP:
        if (IsCmdEnabled(hwnd,IDM_VIEW_WORDWRAP)) {
            SendWMCommand(hwnd, IDM_VIEW_WORDWRAP);
        } else {
            SimpleBeep();
        }
        break;


    case IDT_VIEW_ZOOMIN:
        if (IsCmdEnabled(hwnd,IDM_VIEW_ZOOMIN)) {
            SendWMCommand(hwnd, IDM_VIEW_ZOOMIN);
        } else {
            SimpleBeep();
        }
        break;

    case IDT_VIEW_RESETZOOM:
        if (IsCmdEnabled(hwnd, IDM_VIEW_RESETZOOM)) {
            SendWMCommand(hwnd, IDM_VIEW_RESETZOOM);
        } else {
            SimpleBeep();
        }
        break;


    case IDT_VIEW_ZOOMOUT:
        if (IsCmdEnabled(hwnd,IDM_VIEW_ZOOMOUT)) {
            SendWMCommand(hwnd, IDM_VIEW_ZOOMOUT);
        } else {
            SimpleBeep();
        }
        break;


    case IDT_VIEW_CHASING_DOCTAIL:
        if (IsCmdEnabled(hwnd, IDM_VIEW_CHASING_DOCTAIL)) {
            SendWMCommand(hwnd, IDM_VIEW_CHASING_DOCTAIL);
        } else {
            SimpleBeep();
        }
        break;


    case IDT_VIEW_SCHEME:
        if (IsCmdEnabled(hwnd,IDM_VIEW_SCHEME)) {
            SendWMCommand(hwnd, IDM_VIEW_SCHEME);
        } else {
            SimpleBeep();
        }
        break;


    case IDT_VIEW_SCHEMECONFIG:
        if (IsCmdEnabled(hwnd,IDM_VIEW_SCHEMECONFIG)) {
            SendWMCommand(hwnd, IDM_VIEW_SCHEMECONFIG);
        } else {
            SimpleBeep();
        }
        break;


    case IDT_FILE_SAVEAS:
        if (IsCmdEnabled(hwnd,IDM_FILE_SAVEAS)) {
            SendWMCommand(hwnd, IDM_FILE_SAVEAS);
        } else {
            SimpleBeep();
        }
        break;


    case IDT_FILE_SAVECOPY:
        if (IsCmdEnabled(hwnd,IDM_FILE_SAVECOPY)) {
            SendWMCommand(hwnd, IDM_FILE_SAVECOPY);
        } else {
            SimpleBeep();
        }
        break;


    case IDT_EDIT_CLEAR:
        if (IsCmdEnabled(hwnd,IDM_EDIT_CLEAR)) {
            SendWMCommand(hwnd, IDM_EDIT_CLEAR);
        } else {
            SciCall_ClearAll();
        }
        break;


    case IDT_FILE_PRINT:
        if (IsCmdEnabled(hwnd,IDM_FILE_PRINT)) {
            SendWMCommand(hwnd, IDM_FILE_PRINT);
        } else {
            SimpleBeep();
        }
        break;


    case IDT_FILE_OPENFAV:
        if (IsCmdEnabled(hwnd,IDM_FILE_OPENFAV)) {
            SendWMCommand(hwnd, IDM_FILE_OPENFAV);
        } else {
            SimpleBeep();
        }
        break;


    case IDT_FILE_ADDTOFAV:
        if (IsCmdEnabled(hwnd,IDM_FILE_ADDTOFAV)) {
            SendWMCommand(hwnd, IDM_FILE_ADDTOFAV);
        } else {
            SimpleBeep();
        }
        break;


    case IDT_VIEW_TOGGLEFOLDS:
        if (IsCmdEnabled(hwnd,IDM_VIEW_TOGGLEFOLDS)) {
            SendWMCommand(hwnd, IDM_VIEW_TOGGLEFOLDS);
        } else {
            SimpleBeep();
        }
        break;


    case IDT_VIEW_TOGGLE_VIEW:
        if (IsCmdEnabled(hwnd,IDM_VIEW_TOGGLE_VIEW)) {
            SendWMCommand(hwnd, IDM_VIEW_TOGGLE_VIEW);
        } else {
            SimpleBeep();
        }
        break;


    case IDT_VIEW_PIN_ON_TOP:
        if (IsCmdEnabled(hwnd, IDM_SET_ALWAYSONTOP)) {
            SendWMCommand(hwnd, IDM_SET_ALWAYSONTOP);
        } else {
            SimpleBeep();
        }
        break;


    case IDT_FILE_LAUNCH:
        if (IsCmdEnabled(hwnd,IDM_FILE_LAUNCH)) {
            SendWMCommand(hwnd, IDM_FILE_LAUNCH);
        } else {
            SimpleBeep();
        }
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
        }
        else {
            MinimizeWndToTaskbar(hwnd);
        }
        return FALSE; // swallowed

    case SC_RESTORE: {
        if (Globals.bMinimizedToTray) {
            RestoreWndFromTray(hwnd);
        }
        else {
            RestoreWndFromTaskbar(hwnd);
        }
        ShowOwnedPopups(hwnd, true);
        return FALSE; // swallowed
    }

    case SC_MAXIMIZE:
        break;
    default:
        break;
    }
    return DefWindowProc(hwnd, umsg, wParam, lParam);
}


//=============================================================================
//
//  MsgUahMenuBar() - Handles WM_UAH... commands
//  https://github.com/adzm/win32-custom-menubar-aero-theme
//  https://stackoverflow.com/questions/57177310/how-to-paint-over-white-line-between-menu-bar-and-client-area-of-window
//

#if 0
inline static RECT GetNonclientMenuBorderRect(HWND hwnd)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    MapRectClientToWndCoords(hwnd, &rc);
    rc.top = rc.bottom + 1;
    rc.bottom += 2;
    return rc;
}
#endif


LRESULT MsgUahMenuBar(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
    static HTHEME s_darkMenuTheme = NULL;

    switch (umsg) {

    case WM_UAHINITMENU: {
        if (!s_darkMenuTheme) {
            s_darkMenuTheme = OpenThemeData(hwnd, L"Menu");
        }
    } break;

    case WM_UAHDESTROYWINDOW: {
        if (s_darkMenuTheme) {
            CloseThemeData(s_darkMenuTheme);
            s_darkMenuTheme = NULL;
        }
    } break;

    case WM_UAHDRAWMENU: {

        if (!UseDarkMode()) {
            break;
        }

        UAHMENU* const pUDM = (UAHMENU*)lParam;
        RECT           rc = { 0 };

        // get the menubar rect
        MENUBARINFO mbi = { sizeof(mbi) };
        GetMenuBarInfo(hwnd, OBJID_MENU, 0, &mbi);

        RECT rcWindow;
        GetWindowRect(hwnd, &rcWindow);

        // the rcBar is offset by the window rect
        rc = mbi.rcBar;
        OffsetRect(&rc, -rcWindow.left, -rcWindow.top);
        // fill line below bar
        //~rc.bottom += 2;
        FillRect(pUDM->hdc, &rc, Globals.hbrDarkModeBkgBrush);

        return TRUE;
    }


    case WM_NCACTIVATE:
    case WM_NCPAINT: {
        // get rid of annoying menu-bar bottom line
        // (only called if in dark mode)

        MENUBARINFO mbi = { sizeof(mbi) };
        if (!GetMenuBarInfo(hwnd, OBJID_MENU, 0, &mbi)) {
            return FALSE;
        }

        RECT rcClient = { 0 };
        GetClientRect(hwnd, &rcClient);
        MapWindowPoints(hwnd, NULL, (POINT*)&rcClient, 2);

        RECT rcWindow = { 0 };
        GetWindowRect(hwnd, &rcWindow);

        OffsetRect(&rcClient, -rcWindow.left, -rcWindow.top);

        // the rcBar is offset by the window rect
        RECT rcAnnoyingLine = rcClient;
        rcAnnoyingLine.bottom = rcAnnoyingLine.top;
        rcAnnoyingLine.top -= 1;

        HDC const hdc = GetWindowDC(hwnd);
        FillRect(hdc, &rcAnnoyingLine, Globals.hbrDarkModeBtnFcBrush); // Globals.hbrDarkModeBkgBrush
        ReleaseDC(hwnd, hdc);

        return TRUE;
    }


    case WM_UAHDRAWMENUITEM: {

        if (!UseDarkMode()) {
            break;
        }

        UAHDRAWMENUITEM* pUDMI = (UAHDRAWMENUITEM*)lParam;

        HBRUSH* pbrBackground = &Globals.hbrDarkModeBkgBrush;

        // get the menu item string
        wchar_t      menuString[256] = { 0 };
        MENUITEMINFO mii = { sizeof(mii), MIIM_STRING };
        {
            mii.dwTypeData = menuString;
            mii.cch = (sizeof(menuString) / 2) - 1;

            GetMenuItemInfo(pUDMI->um.hmenu, pUDMI->umi.iPosition, TRUE, &mii);
        }

        // get the item state for drawing

        DWORD dwFlags = DT_CENTER | DT_SINGLELINE | DT_VCENTER;

        int iTextStateID = 0;
        int iBackgroundStateID = 0;
        {
            if ((pUDMI->dis.itemState & ODS_INACTIVE) | (pUDMI->dis.itemState & ODS_DEFAULT)) {
                // normal display
                iTextStateID = MPI_NORMAL;
                iBackgroundStateID = MPI_NORMAL;
            }
            if (pUDMI->dis.itemState & ODS_HOTLIGHT) {
                // hot tracking
                iTextStateID = MPI_HOT;
                iBackgroundStateID = MPI_HOT;
                pbrBackground = &Globals.hbrDarkModeBkgHotBrush;
            }
            if (pUDMI->dis.itemState & ODS_SELECTED) {
                // clicked -- MENU_POPUPITEM has no state for this, though MENU_BARITEM does
                iTextStateID = MPI_HOT;
                iBackgroundStateID = MPI_HOT;
                pbrBackground = &Globals.hbrDarkModeBkgSelBrush;
            }
            if ((pUDMI->dis.itemState & ODS_GRAYED) || (pUDMI->dis.itemState & ODS_DISABLED)) {
                // disabled / grey text
                iTextStateID = MPI_DISABLED;
                iBackgroundStateID = MPI_DISABLED;
            }
            if (pUDMI->dis.itemState & ODS_NOACCEL) {
                dwFlags |= DT_HIDEPREFIX;
            }
        }

        DTTOPTS opts = { sizeof(opts), DTT_TEXTCOLOR, iTextStateID != MPI_DISABLED ? Settings2.DarkModeTxtColor : RGB(0x80, 0x80, 0x80) };

        FillRect(pUDMI->um.hdc, &pUDMI->dis.rcItem, *pbrBackground);
        DrawThemeTextEx(s_darkMenuTheme, pUDMI->um.hdc, MENU_BARITEM, MBI_NORMAL, menuString, mii.cch, dwFlags, &pUDMI->dis.rcItem, &opts);

        return TRUE;

    } break;

    case WM_UAHMEASUREMENUITEM: {

        // allow the default window procedure to handle the message
        // since we don't really care about changing the width
        //LRESULT const res = DefWindowProc(hwnd, umsg, wParam, lParam);

        // but we can modify it here to make it 1/3rd wider and higher for example
        //UAHMEASUREMENUITEM* const pMmi = (UAHMEASUREMENUITEM*)lParam;
        //pMmi->mis.itemWidth = (pMmi->mis.itemWidth * 4) / 3;
        //pMmi->mis.itemHeight = (pMmi->mis.itemHeight * 4) / 3;

        //return res;

    } break;

    // don't care
    case WM_UAHNCPAINTMENUPOPUP:
    default:
        break;
    }

    return DefWindowProc(hwnd, umsg, wParam, lParam);
}


//=============================================================================
//
//  HandleDWellStartEnd()
//
static DocPos prevCaretPosition = -1;

void HandleDWellStartEnd(const DocPos position, const UINT uid)
{
    static DocPos prevStartPosition = -1;
    static DocPos prevEndPosition = -1;

    if (position >= 0) {
        if (prevCaretPosition < 0) {
            prevCaretPosition = position;
        }
        if (prevStartPosition < 0) {
            prevStartPosition = position;
        }
        if (prevEndPosition < 0) {
            prevEndPosition = position;
        }
    }

    int indicator_id = INDICATOR_CONTAINER;

    switch (uid) {
    case SCN_DWELLSTART: {

        if (position < 0) {
            Sci_CallTipCancelEx();
            prevCaretPosition = -1;
            return;
        }

        if (Settings.HyperlinkHotspot) {
            if (SciCall_IndicatorValueAt(INDIC_NP3_HYPERLINK, position) > 0) {
                indicator_id = INDIC_NP3_HYPERLINK;
                if (position != prevCaretPosition) {
                    Sci_CallTipCancelEx();
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

        switch (indicator_id) {

        case INDIC_NP3_HYPERLINK:
            if (!Settings.ShowHypLnkToolTip || SciCall_CallTipActive()) {
                return;
            }
            break;

        case INDIC_NP3_UNICODE_POINT:
            if (!Settings.HighlightUnicodePoints || SciCall_CallTipActive()) {
                return;
            }
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

        if ((position < prevStartPosition) || (position > prevEndPosition)) {
            s_bCallTipEscDisabled = false;
        }

        DocPos const firstPos = SciCall_IndicatorStart(indicator_id, position);
        DocPos const lastPos = SciCall_IndicatorEnd(indicator_id, position);
        DocPos const length = (lastPos - firstPos);

        // WebLinks and Color Refs are ASCII only - No need for UTF-8 conversion here

        if (INDIC_NP3_HYPERLINK == indicator_id) {

            if (!s_bCallTipEscDisabled) {

                const char * const pUrlBegin = SciCall_GetRangePointer(firstPos, length);

                char chScheme[32] = { '\0' };
                for (unsigned i = 0; i < COUNTOF(chScheme); ++i) {
                    chScheme[i] = pUrlBegin[i];
                    if (!pUrlBegin[i] || pUrlBegin[i] == ':') {
                        break;
                    }
                }

                CHAR chCallTip[MIDSZ_BUFFER] = { L'\0' };

                size_t cch = 0;
                if (StrStrIA(chScheme, "file:") == chScheme) {

                    WCHAR wchUrl[INTERNET_MAX_URL_LENGTH] = { L'\0' };

                    int const cchUrl = MultiByteToWideChar(Encoding_SciCP, 0, pUrlBegin, (int)length, wchUrl, COUNTOF(wchUrl));
                    wchUrl[cchUrl] = L'\0';
                    StrTrim(wchUrl, L" \r\n\t");

                    SplitFilePathLineNum(wchUrl, NULL); // cut off possible linenum spec

                    HPATHL       hurl_pth = Path_Allocate(NULL);
                    DWORD        cchPath = INTERNET_MAX_URL_LENGTH;
                    LPWSTR const url_buf = Path_WriteAccessBuf(hurl_pth, cchPath);

                    if (FAILED(PathCreateFromUrlW(wchUrl, url_buf, &cchPath, 0))) {
                        Path_Sanitize(hurl_pth);
                        const char *p = &pUrlBegin[CONSTSTRGLEN("file://")];
                        while (p && (*p == '/')) { ++p; }
                        StringCchCopyN(url_buf, Path_GetBufCount(hurl_pth), wchUrl, cchUrl); // no op
                        Path_Sanitize(hurl_pth);
                        cchPath = (DWORD)Path_GetLength(hurl_pth);
                    }
                    Path_Sanitize(hurl_pth);
                    Path_NormalizeEx(hurl_pth, Paths.WorkingDirectory, true, false);
 
                    bool found = true;
                    if (Path_IsExistingFile(hurl_pth)) {
                        GetLngStringW2MB(IDS_MUI_URL_FILE_EXISTS, chCallTip, (int)(COUNTOF(chCallTip) >> 1));
                    } else if (Path_IsExistingDirectory(hurl_pth)) {
                        GetLngStringW2MB(IDS_MUI_URL_DIR_EXISTS, chCallTip, (int)(COUNTOF(chCallTip) >> 1));
                    } else {
                        found = false;
                        GetLngStringW2MB(IDS_MUI_URL_PATH_NOT_FOUND, chCallTip, (int)(COUNTOF(chCallTip) >> 1));
                    }
                    if (found) {
                        cch = StringCchLenA(chCallTip, COUNTOF(chCallTip));
                        GetLngStringW2MB(IDS_MUI_URL_OPEN_FILE, &chCallTip[cch], (int)(COUNTOF(chCallTip) - cch));
                    }
                    Path_Release(hurl_pth);

                } else { // Web URL

                    StringCchCopyNA(chCallTip, COUNTOF(chCallTip) >> 1, pUrlBegin, length);
                    cch = StringCchLenA(chCallTip, COUNTOF(chCallTip) >> 1);
                    GetLngStringW2MB(IDS_MUI_URL_OPEN_BROWSER, &chCallTip[cch], (int)(COUNTOF(chCallTip) - cch));

                }

                if (!StrIsEmptyA(chCallTip)) {
                    // first show, then set highlight range
                    // SciCall_CallTipSetPosition(true);
                    SciCall_CallTipShow(position, chCallTip);
                    SciCall_CallTipSetHlt(0, (int)cch);
                }
            }

        } else if (INDIC_NP3_COLOR_DEF == indicator_id) {

            char chText[MICRO_BUFFER] = { '\0' };
            // Color Refs are ASCII only - No need for UTF-8 conversion here
            StringCchCopyNA(chText, COUNTOF(chText), SciCall_GetRangePointer(firstPos, length), length);
            unsigned int iValue = 0;
            if (sscanf_s(&chText[1], "%x", &iValue) == 1) {
                COLORREF rgb = 0x000000;
                int alpha = SC_ALPHA_OPAQUE;
                if (length >= 8) { // ARGB, RGBA, BGRA
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
                } else { // RGB
                    rgb   = RGB((iValue >> 16) & 0xFF, (iValue >> 8) & 0xFF, iValue & 0xFF);
                    alpha = SC_ALPHA_OPAQUE;
                }
                COLORREF const fgr = CalcContrastColor(rgb, alpha);

                SciCall_SetIndicatorCurrent(INDIC_NP3_COLOR_DEF_T);
                SciCall_IndicatorFillRange(firstPos, length);
                SciCall_IndicSetHoverFore(INDIC_NP3_COLOR_DEF_T, fgr);

                SciCall_IndicSetAlpha(INDIC_NP3_COLOR_DEF, Sci_ClampAlpha(alpha));
                SciCall_IndicSetHoverFore(INDIC_NP3_COLOR_DEF, rgb);

                SciCall_IndicSetHoverStyle(INDIC_NP3_COLOR_DEF, INDIC_FULLBOX);
                SciCall_IndicSetHoverStyle(INDIC_NP3_COLOR_DEF_T, INDIC_TEXTFORE);
            }

        } else if (INDIC_NP3_UNICODE_POINT == indicator_id) {

            if (!s_bCallTipEscDisabled) {
                char chHex2Char[MIDSZ_BUFFER] = {'\0'};
                // No need for UTF-8 conversion here and
                StringCchCopyNA(chHex2Char, COUNTOF(chHex2Char), SciCall_GetRangePointer(firstPos, length), length);
                //StrTrimA(chHex2Char, " \t\n\r");

                Hex2Char(chHex2Char, COUNTOF(chHex2Char));
                if (StrIsEmptyA(chHex2Char)) {
                    break;
                }

                // first show, then set highlight range
                // SciCall_CallTipSetPosition(true);
                SciCall_CallTipShow(position, chHex2Char);
                SciCall_CallTipSetHlt(0, (int)length);
            }
        }

        prevCaretPosition = position;
        prevStartPosition = firstPos;
        prevEndPosition = lastPos;
    }
    break;

    case SCN_DWELLEND: {

        if ((position >= prevStartPosition) && ((position <= prevEndPosition))) {
            return;    // avoid flickering
        }

        Sci_CallTipCancelEx();

        DocPos const curPos = SciCall_GetCurrentPos();
        if ((curPos >= prevStartPosition) && ((curPos <= prevEndPosition))) {
            return;    // no change for if caret in range
        }
        s_bCallTipEscDisabled = false;
        prevCaretPosition = -1;

        // clear SCN_DWELLSTART visual styles
        SciCall_SetIndicatorCurrent(INDIC_NP3_COLOR_DEF_T);
        SciCall_IndicatorClearRange(0, Sci_GetDocEndPosition());

        // hide color of color definition
        SciCall_IndicSetHoverStyle(INDIC_NP3_COLOR_DEF, INDIC_HIDDEN);   // hide box
        SciCall_IndicSetHoverStyle(INDIC_NP3_COLOR_DEF_T, INDIC_HIDDEN); // hide txt

        //~ this will destroy rectangular selection on multi-replace ???
        //~ !!! strange side-effects using following statements !!!
        //~~~SciCall_IndicSetAlpha(INDIC_NP3_COLOR_DEF, SC_ALPHA_TRANSPARENT);
        //~~~SciCall_IndicSetFore(INDIC_NP3_COLOR_DEF, RGB(0,0,0));
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
    if (position < 0) {
        return false;
    }

    //~SciCall_PostMsg(WM_LBUTTONUP, MK_LBUTTON, 0);
    Sci_CallTipCancelEx();

    bool bHandled = false;
    if (SciCall_IndicatorValueAt(INDIC_NP3_HYPERLINK, position)) {

        DocPos const firstPos = SciCall_IndicatorStart(INDIC_NP3_HYPERLINK, position);
        DocPos const lastPos = SciCall_IndicatorEnd(INDIC_NP3_HYPERLINK, position);
        DocPos const length = min_p(lastPos - firstPos, INTERNET_MAX_URL_LENGTH);

        if (length < 4) {
            return false;
        }

        const char * const pszText = SciCall_GetRangePointer(firstPos, length);

        WCHAR szTextW[INTERNET_MAX_URL_LENGTH + 1];
        ptrdiff_t const cchTextW = MultiByteToWideChar(Encoding_SciCP, 0, pszText, (int)length, szTextW, COUNTOF(szTextW));
        szTextW[cchTextW] = L'\0';
        StrTrim(szTextW, L" \r\n\t");

        const WCHAR* const chkPreFix = L"file://";
        size_t const lenPfx = StringCchLen(chkPreFix, 0);

        if (operation & SELECT_HYPERLINK) {

            SciCall_SetSelection(firstPos, lastPos);
            bHandled = true;

        } else if (operation & COPY_HYPERLINK) {

            if (cchTextW > 0) {
                DWORD cchEscapedW = (DWORD)(length * 3 + 1);
                LPWSTR const pszEscapedW = (LPWSTR)AllocMem(cchEscapedW * sizeof(WCHAR), HEAP_ZERO_MEMORY);
                if (pszEscapedW) {
                    //~UrlEscape(szTextW, pszEscapedW, &cchEscapedW, (URL_BROWSER_MODE | URL_ESCAPE_AS_UTF8));
                    UrlEscapeEx(szTextW, pszEscapedW, &cchEscapedW, false);
                    SetClipboardText(Globals.hwndMain, pszEscapedW, cchEscapedW);
                    FreeMem(pszEscapedW);
                    bHandled = true;
                }
            }

        } else {

            WCHAR szUnEscW[INTERNET_MAX_URL_LENGTH + 1];
            DWORD dCch = COUNTOF(szUnEscW);

            int lineNum = -1;
            SplitFilePathLineNum(szTextW, &lineNum);
            lineNum = clampi(lineNum, 0, INT_MAX);

            if (((operation & OPEN_IN_NOTEPAD3) || (operation & OPEN_NEW_NOTEPAD3)) && UrlIsFileUrl(szTextW)) {

                bool const bReuseWindow = Flags.bReuseWindow && !(operation & OPEN_NEW_NOTEPAD3);

                PathCreateFromUrlW(szTextW, szUnEscW, &dCch, 0);
                szUnEscW[INTERNET_MAX_URL_LENGTH] = L'\0'; // limit length
                StrTrim(szUnEscW, L"/");

                HPATHL hfile_pth = Path_Allocate(szUnEscW);
                Path_CanonicalizeEx(hfile_pth, Paths.ModuleDirectory);
                //@@@???Path_CanonicalizeEx(hfile_pth, Paths.WorkingDirectory);

                bool success = false;
                FileLoadFlags fLoadFlags = FLF_None;
                fLoadFlags |= Settings.SkipUnicodeDetection ? FLF_SkipUnicodeDetect : 0;
                fLoadFlags |= Settings.SkipANSICodePageDetection ? FLF_SkipANSICPDetection : 0;

                if (Path_IsExistingFile(hfile_pth)) {
                    if (bReuseWindow) {
                        success = FileLoad(hfile_pth, fLoadFlags, 0, 0);
                    } else {
                        WCHAR wchParams[64];
                        StringCchPrintf(wchParams, COUNTOF(wchParams), L"%s /g %i", Flags.bSingleFileInstance ? L"/ns" : L"/n", lineNum);
                        success = LaunchNewInstance(Globals.hwndMain, wchParams, Path_Get(hfile_pth));
                    }
                }
                else if (Path_IsExistingDirectory(hfile_pth)) {
                    if (bReuseWindow) {
                        if (OpenFileDlg(Globals.hwndMain, hfile_pth, hfile_pth)) {
                            success = FileLoad(hfile_pth, fLoadFlags, 0, 0);
                        }
                    } else {
                        WCHAR wchParams[64];
                        StringCchPrintf(wchParams, COUNTOF(wchParams), L"%s", Flags.bSingleFileInstance ? L"/ns" : L"/n");
                        success = LaunchNewInstance(Globals.hwndMain, wchParams, Path_Get(hfile_pth));
                    }
                }
                if (bReuseWindow && success && (lineNum >= 0)) {
                    lineNum = clampi(lineNum - 1, 0, INT_MAX);
                    //~SciCall_GotoLine((DocLn)lineNum);
                    SciCall_PostMsg(SCI_GOTOLINE, (WPARAM)lineNum, 0);
                }
                bHandled = true;
                Path_Release(hfile_pth);

            } else if (operation & OPEN_WITH_BROWSER) {  // open in web browser or associated application
                
                HPATHL hDirectory = Path_Allocate(NULL);

                if (UrlIsFileUrl(szTextW)) {
                    // ShellExecuteExW() will handle file-system path correctly for "file://" protocol
                    StringCchCopy(szUnEscW, COUNTOF(szUnEscW), chkPreFix);
                    dCch -= (DWORD)lenPfx;
                    PathCreateFromUrl(szTextW, &szUnEscW[lenPfx], &dCch, 0);
                    Path_Reset(hDirectory, szUnEscW);
                    Path_RemoveFileSpec(hDirectory);
                } else {
                    UrlUnescapeEx(szTextW, szUnEscW, &dCch);
                }
                if (Path_IsEmpty(hDirectory) && Path_IsNotEmpty(Paths.CurrentFile)) {
                    Path_Reset(hDirectory, Path_Get(Paths.CurrentFile));
                    Path_RemoveFileSpec(hDirectory);
                }

                if (StrgIsNotEmpty(Settings2.HyperlinkShellExURLWithApp)) {

                    HSTRINGW hstr_params = StrgCopy(Settings2.HyperlinkShellExURLCmdLnArgs);
                    StrgReplace(hstr_params, URLPLACEHLDR, szUnEscW);

                    SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
                    sei.fMask = SEE_MASK_DEFAULT;
                    sei.hwnd = NULL;
                    sei.lpVerb = NULL;
                    sei.lpFile = StrgGet(Settings2.HyperlinkShellExURLWithApp);
                    sei.lpParameters = StrgIsNotEmpty(hstr_params) ? StrgGet(hstr_params) : szUnEscW;
                    sei.lpDirectory = Path_Get(hDirectory);
                    sei.nShow = SW_SHOWNORMAL;
                    bHandled = ShellExecuteExW(&sei);

                    StrgDestroy(hstr_params);

                } else {

                    const WCHAR *const lpVerb = StrIsEmpty(Settings2.HyperlinkFileProtocolVerb) ? NULL : Settings2.HyperlinkFileProtocolVerb;

                    SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
                    sei.fMask = SEE_MASK_NOZONECHECKS;
                    sei.hwnd = NULL;
                    sei.lpVerb = lpVerb;
                    sei.lpFile = szUnEscW;
                    sei.lpParameters = NULL;
                    sei.lpDirectory = Path_Get(hDirectory);
                    sei.nShow = SW_SHOWNORMAL;
                    bHandled = ShellExecuteExW(&sei);
                }

                Path_Release(hDirectory);
            }
        }
    }

    if (!(operation & SELECT_HYPERLINK)) {
        SciCall_SetEmptySelection(position);
    }

    return bHandled;
}


//=============================================================================
//
//  HandleColorDefClicked()
//
void HandleColorDefClicked(HWND hwnd, const DocPos position)
{
    if (SciCall_IndicatorValueAt(INDIC_NP3_COLOR_DEF, position) == 0) {
        return;
    }

    DocPos const firstPos = SciCall_IndicatorStart(INDIC_NP3_COLOR_DEF, position);
    DocPos const lastPos = SciCall_IndicatorEnd(INDIC_NP3_COLOR_DEF, position);
    DocPos const length = (lastPos - firstPos);

    char chText[32] = { '\0' };
    // Color Refs are ASCII only - No need for UTF-8 conversion here
    StringCchCopyNA(chText, COUNTOF(chText), SciCall_GetRangePointer(firstPos, length), length);
    unsigned int iValue = 0;
    if (sscanf_s(&chText[1], "%x", &iValue) == 1) {
        COLORREF rgbCur = 0x000000;
        BYTE     alpha  = 0xFF;
        if (length >= 8) { // ARGB, RGBA, BGRA
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
        } else { // RGB
            rgbCur = RGB((iValue >> 16) & 0xFF, (iValue >> 8) & 0xFF, iValue & 0xFF);
            alpha  = 0xFF;
        }

        CHOOSECOLOR cc = { sizeof(CHOOSECOLOR) };
        cc.hwndOwner = hwnd;
        cc.hInstance = (HWND)Globals.hLngResContainer; // Globals.hInstance;
        cc.rgbResult = rgbCur;
        cc.lpCustColors = &g_colorCustom[0];
        cc.Flags = CC_FULLOPEN | CC_RGBINIT | CC_ANYCOLOR;

        // custom hook
        cc.Flags |= CC_ENABLEHOOK;
        cc.lpfnHook = (LPCCHOOKPROC)ColorDialogHookProc;
        WININFO const wi = GetMyWindowPlacement(Globals.hwndEdit, NULL, 0, false);
        int const offset = f2int(Style_GetCurrentLexerFontSize()) << 1;
        POINT pt = { 0L, 0L };
        pt.x = wi.x + SciCall_PointXFromPosition(SciCall_GetCurrentPos()) + offset;
        pt.y = wi.y + SciCall_PointYFromPosition(SciCall_GetCurrentPos()) + offset;
        cc.lCustData = (LPARAM)&pt;

        // Color.dlg resource template
        cc.Flags |= CC_ENABLETEMPLATE | CC_ENABLETEMPLATEHANDLE;
        cc.lpTemplateName = MAKEINTRESOURCEW(IDD_MUI_SYSCOLOR_DLG);

        if (!ChooseColor(&cc)) {
            return;
        }

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
        } else {
            StringCchPrintfA(wchColor, COUNTOF(wchColor), "#%02X%02X%02X",
                             (int)GetRValue(rgbNew), (int)GetGValue(rgbNew), (int)GetBValue(rgbNew));
        }

        SciCall_SetTargetRange(firstPos, lastPos);
        Sci_ReplaceTargetTestChgHist(length, wchColor);

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
                int const bitmask = SciCall_MarkerGet(iCurLine - 1) & ALL_MARKERS_BITMASK();
                if (bitmask) {
                    UserMarkerDeleteAll(iCurLine - 1);
                    SciCall_MarkerAddSet(iCurLine, bitmask);
                }
            }

            //~if (iLineLength <= 2)
            {
                DocLn const  iPrevLine       = iCurLine - 1;
                DocPos const iPrevLineLength = SciCall_LineLength(iPrevLine);
                char * const pLineBuf        = (char*)AllocMem(iPrevLineLength + 1, HEAP_ZERO_MEMORY);
                if (pLineBuf) {
                    SciCall_GetLine_Safe(iPrevLine, pLineBuf);
                    for (char* pPos = pLineBuf; *pPos; pPos++) {
                        if ((*pPos != ' ') && (*pPos != '\t')) {
                            *pPos = '\0';
                            break;
                        }
                    }
                    if (*pLineBuf) {
                        SciCall_AddText((DocPos)StringCchLenA(pLineBuf, SizeOfMem(pLineBuf)), pLineBuf);
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
static void _HandleAutoCloseTags()
{
    if (Sci_IsMultiSelection()) {
        return;
    }
    /// int const lexerID = SciCall_GetLexer();
    /// if (lexerID == SCLEX_HTML || lexerID == SCLEX_XML)
    DocPos const maxSearchBackward = 8192;
    DocLn const  vis1stLine = SciCall_GetFirstVisibleLine();
    DocPos const iCurPos = SciCall_GetCurrentPos();
    DocPos const iHelper = iCurPos - maxSearchBackward;
    DocPos const iStartPos = max_p(0, iHelper);
    DocPos const iSize = iCurPos - iStartPos;

    if (iSize >= 3) {

        const char* const pBegin = SciCall_GetRangePointer(iStartPos, iSize);

        if (pBegin[iSize - 2] != '/') {
            const char* pCur = &pBegin[iSize - 2];
            while (pCur > pBegin && *pCur != '<' && *pCur != '>') {
                --pCur;
            }

            int  cchIns = 2;
            char replaceBuf[SMALL_BUFFER];
            StringCchCopyA(replaceBuf, COUNTOF(replaceBuf), "</");

            if (*pCur == '<') {
                ++pCur;
                while ((StrChrA(":_-.", *pCur) || IsCharAlphaNumericA(*pCur)) && (cchIns < (COUNTOF(replaceBuf) - 2))) {
                    replaceBuf[cchIns++] = *pCur++;
                }
            }
            replaceBuf[cchIns++] = '>';
            replaceBuf[cchIns] = '\0';

            // except tags w/o closing tags
            const char* const nonClosingTags[9] = { "</base>", "</bgsound>", "</br>", "</embed>", "</hr>", "</img>", "</input>", "</link>", "</meta>" };
            int const         cntCount = COUNTOF(nonClosingTags);

            bool isNonClosingTag = false;
            for (int i = 0; ((i < cntCount) && !isNonClosingTag); ++i) {
                isNonClosingTag = (StrCmpIA(replaceBuf, nonClosingTags[i]) == 0);
            }
            if ((cchIns > 3) && !isNonClosingTag) {
                EditReplaceSelection(replaceBuf, false);
                SciCall_GotoPos(iCurPos);
                SciCall_SetFirstVisibleLine(vis1stLine);
            }
        }
    }
}


//=============================================================================
//
//  _HandleInsertCheck()
//

static inline void _SaveSelectionToBuffer()
{
    if (!s_SelectionBuffer) {
        return;
    }
    if (Sci_IsMultiOrRectangleSelection()) {
        s_SelectionBuffer[0] = '\0';
        s_SelectionBuffer[1] = 'X';
        return;
    }
    size_t len = SciCall_GetSelText(NULL);
    if ((len + 3) > SizeOfMem(s_SelectionBuffer)) {
        s_SelectionBuffer = (char*)ReAllocMem(s_SelectionBuffer, len + 3, HEAP_ZERO_MEMORY);
    }
    Sci_GetSelectionTextN(&s_SelectionBuffer[1], SizeOfMem(s_SelectionBuffer) - 2);
    s_SelectionBuffer[0] = s_SelectionBuffer[1];
}

static inline DocPos _EncloseSelectionBuffer(const char op, const char cl)
{
    if (!s_SelectionBuffer) {
        return 0;
    }
    if (s_SelectionBuffer[0] == '\0' && s_SelectionBuffer[1] == 'X') {
        // IsMultiOrRectangleSelection:
        s_SelectionBuffer[0] = op;
        s_SelectionBuffer[1] = '\0';
        return 1;
    }
    size_t len = s_SelectionBuffer ? strlen(s_SelectionBuffer) : 0;
    len += (len ? 0 : 1); // empty correction
    s_SelectionBuffer[0] = op;
    s_SelectionBuffer[len++] = cl;
    s_SelectionBuffer[len] = '\0';
    return len;
}

static inline void _HandleInsertCheck(const SCNotification* const scn)
{
    if (Sci_IsMultiOrRectangleSelection() || !scn || !(scn->text)) {
        return;
    }
    if (Settings.AutoCloseQuotes) {
        if (scn->length == 1) {
            DocPos len = 0;
            switch (scn->text[0]) {
            case '"':
                if (Sci_GetCurrChar() == '"') {
                    SciCall_ChangeInsertion(0, ""); // clear
                    SciCall_PostMsg(SCI_CHARRIGHT, 0, 0);
                }
                else {
                    len = _EncloseSelectionBuffer('"', '"');
                }
                break;
            case '\'':
                if (Sci_GetCurrChar() == '\'') {
                    SciCall_ChangeInsertion(0, ""); // clear
                    SciCall_PostMsg(SCI_CHARRIGHT, 0, 0);
                }
                else {
                    len = _EncloseSelectionBuffer('\'', '\'');
                }
                break;
            case '`':
                if (Sci_GetCurrChar() == '`') {
                    SciCall_ChangeInsertion(0, ""); // clear
                    SciCall_PostMsg(SCI_CHARRIGHT, 0, 0);
                }
                else {
                    len = _EncloseSelectionBuffer('`', '`');
                }
                break;
            default:
                break;
            }
            if (len) {
                SciCall_ChangeInsertion(len, s_SelectionBuffer);
                if (len == 2) {
                    SciCall_PostMsg(SCI_CHARLEFT, 0, 0);
                }
            }
        }
    }
    if (Settings.AutoCloseBrackets) {
        if (scn->length == 1) {
            DocPos len = 0;
            switch (scn->text[0]) {
            case '[':
                len = _EncloseSelectionBuffer('[', ']');
                break;
            case '{':
                len = _EncloseSelectionBuffer('{', '}');
                break;
            case '(':
                len = _EncloseSelectionBuffer('(', ')');
                break;
            case ')':
                if (Sci_GetCurrChar() == ')') {
                    SciCall_ChangeInsertion(0, ""); // clear
                    SciCall_PostMsg(SCI_CHARRIGHT, 0, 0);
                }
                break;
            case '}':
                if (Sci_GetCurrChar() == '}') {
                    SciCall_ChangeInsertion(0, ""); // clear
                    SciCall_PostMsg(SCI_CHARRIGHT, 0, 0);
                }
                break;
            case ']':
                if (Sci_GetCurrChar() == ']') {
                    SciCall_ChangeInsertion(0, ""); // clear
                    SciCall_PostMsg(SCI_CHARRIGHT, 0, 0);
                }
                break;
            default:
                break;
            }
            if (len) {
                SciCall_ChangeInsertion(len, s_SelectionBuffer);
                if (len == 2) {
                    SciCall_PostMsg(SCI_CHARLEFT, 0, 0);
                }
            }
        }
    }
}

static inline void _HandleDeleteCheck(const SCNotification* const scn)
{
    if (Settings.AutoCloseQuotes) {
        if (scn->length == 1) {
            bool       bDelPair = false;
            char const chrAfter = SciCall_GetCharAt(scn->position);
            switch (scn->text[0]) {
            case '"':
                if (chrAfter == '"')
                    bDelPair = true;
                break;
            case '\'':
                if (chrAfter == '\'')
                    bDelPair = true;
                break;
            case '`':
                if (chrAfter == '`')
                    bDelPair = true;
                break;
            default:
                break;
            }
            if (bDelPair) {
                SciCall_PostMsg(SCI_CHARRIGHT, 0, 0);
                SciCall_PostMsg(SCI_DELETEBACK, 0, 0);
            }
        }
    }

    if (Settings.AutoCloseBrackets) {
        if (scn->length == 1) {
            bool       bDelPair = false;
            char const chrAfter = SciCall_GetCharAt(scn->position);         
            switch (scn->text[0]) {
            case '[':
                if (chrAfter == ']')
                    bDelPair = true;
                break;
            case '{':
                if (chrAfter == '}')
                    bDelPair = true;
            case '(':
                if (chrAfter == ')')
                    bDelPair = true;
                break;
            default:
                break;
            }
            if (bDelPair) {
                SciCall_PostMsg(SCI_CHARRIGHT, 0, 0);
                SciCall_PostMsg(SCI_DELETEBACK, 0, 0);
            }
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

inline static LRESULT _MsgNotifyLean(const SCNotification* const scn, bool* bModified)
{

    const LPNMHDR pnmh = (LPNMHDR)scn;

    static LONG _urtoken = URTok_NoTransaction;

    // --- check only mandatory events (must be fast !!!) ---
    if (pnmh->idFrom != IDC_EDIT) {
        return FALSE;
    }

    switch (pnmh->code) {

    case SCN_MODIFIED: {
        *bModified = false; // init
        int const iModType = scn->modificationType;
        if ((iModType & SC_MULTISTEPUNDOREDO) && !(iModType & SC_LASTSTEPINUNDOREDO)) {
            return TRUE; // wait for last step in multi-step-undo/redo
        }
        bool const bInUndoRedoStep = (iModType & (SC_PERFORMED_UNDO | SC_PERFORMED_REDO));
        if (iModType & SC_MOD_INSERTCHECK) {
            if (!bInUndoRedoStep) {
                _HandleInsertCheck(scn);
            }
        }
        if (iModType & (SC_MOD_BEFOREINSERT | SC_MOD_BEFOREDELETE)) {
            *bModified = false; // not yet
            if (!bInUndoRedoStep) {
                if ((SciCall_GetUndoSequence() <= 1) && (_urtoken < URTok_TokenStart)) {
                    _SaveSelectionToBuffer();
                    bool const bSelEmpty = SciCall_IsSelectionEmpty();
                    bool const bIsMultiRectSel = Sci_IsMultiOrRectangleSelection();
                    if (!bSelEmpty || bIsMultiRectSel) {
                        LONG const tok = _SaveUndoSelection();
                        _urtoken = (tok >= URTok_TokenStart ? tok : _urtoken);
                    }
                    // TODO: @@@ Find reason for why this NOP workaround is needed:
                    if (!bSelEmpty && bIsMultiRectSel) {
                        // need to trigger SCI:InvalidateCaret()
                        bool const bAddSelTyping = SciCall_GetAdditionalSelectionTyping();
                        //~SciCall_SetAdditionalSelectionTyping(!bAddSelTyping); // v5.1.1: no check for change, so:
                        SciCall_SetAdditionalSelectionTyping(bAddSelTyping);
                    }
                }
            }
        }
        else if (iModType & (SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT)) {
            if (!bInUndoRedoStep) {
                if ((SciCall_GetUndoSequence() <= 1) && (_urtoken >= URTok_TokenStart)) {
                    _SaveRedoSelection(_urtoken, SciCall_GetModify());
                    _urtoken = URTok_NoTransaction;
                }
                if (iModType & SC_MOD_DELETETEXT) {
                    _HandleDeleteCheck(scn);
                }
            }
            *bModified = true;
        }
        // check for ADDUNDOACTION step
        if (iModType & SC_MOD_CONTAINER) {
            // we are inside undo/redo transaction, so do delayed PostMessage() instead of SendMessage()
            if (iModType & SC_PERFORMED_UNDO) {
                PostMessage(Globals.hwndMain, WM_RESTORE_UNDOREDOACTION, (WPARAM)UNDO, (LPARAM)scn->token);
            }
            else if (iModType & SC_PERFORMED_REDO) {
                PostMessage(Globals.hwndMain, WM_RESTORE_UNDOREDOACTION, (WPARAM)REDO, (LPARAM)scn->token);
            }
        }
        if (*bModified) {
            LONG64 const timeout = Settings2.UndoTransactionTimeout;
            if (timeout != 0LL) {
                if (!bInUndoRedoStep) {
                    _DelaySplitUndoTransaction(max_ll(_MQ_IMMEDIATE, timeout));
                }
            }
        }
    } break;

    case SCN_SAVEPOINTREACHED: {
        SetSaveDone();
    } break;

    case SCN_SAVEPOINTLEFT: {
        SetSaveNeeded(false);
    } break;

    case SCN_MODIFYATTEMPTRO: {
        if (FocusedView.HideNonMatchedLines) {
            EditToggleView(Globals.hwndEdit);
        }
        else {
            if (!FileWatching.MonitoringLog && !IsYesOkay(InfoBoxLng(MB_YESNO | MB_ICONINFORMATION, L"QuietKeepReadonlyLock", IDS_MUI_DOCUMENT_READONLY))) {
                SendWMCommand(Globals.hwndMain, IDM_VIEW_READONLY);
            }
            else {
                AttentionBeep(MB_YESNO | MB_ICONINFORMATION);
            }
        }
    } break;

    default:
        break;

    } // switch
    return TRUE;
}


//=============================================================================
//
//  _MsgNotifyFromEdit() - Handles WM_NOTIFY (only absolute neccessary events)
//
//  !!! Set correct SCI_SETMODEVENTMASK in _InitializeSciEditCtrl()
//
static LRESULT _MsgNotifyFromEdit(HWND hwnd, const SCNotification* const scn)
{
    const LPNMHDR pnmh = (LPNMHDR)scn;

    static int _s_indic_click_modifiers = SCMOD_NORM;

    bool bModified = false;
    LRESULT resMN = _MsgNotifyLean(scn, &bModified);

    switch (pnmh->code) {
    // not send
    //~ case SCN_KEY:
    
    // unused:
    case SCN_HOTSPOTCLICK:
    case SCN_HOTSPOTDOUBLECLICK:
    case SCN_HOTSPOTRELEASECLICK:
        return FALSE;


    case SCN_MODIFIED: {
        /// bModified = set in _MsgNotifyLean() !
        if (bModified) {
            int const iModType = scn->modificationType;
            if (IsMarkOccurrencesEnabled()) {
                MarkAllOccurrences(-1, true);
            }
            EditUpdateVisibleIndicators();
            if (scn->linesAdded != 0) {
                if (Settings.SplitUndoTypingSeqOnLnBreak && (scn->linesAdded > 0)) {
                    if (!(iModType & (SC_PERFORMED_UNDO | SC_PERFORMED_REDO))) {
                        _SplitUndoTransaction();
                    }
                }
                UpdateMargins(false);
            }
            if (s_bInMultiEditMode && !(iModType & SC_MULTILINEUNDOREDO)) {
                if (!Sci_IsMultiSelection()) {
                    SciCall_SetIndicatorCurrent(INDIC_NP3_MULTI_EDIT);
                    SciCall_IndicatorClearRange(0, Sci_GetDocEndPosition());
                    s_bInMultiEditMode = false;
                }
            }
        }
        return resMN;
    }


    case SCN_AUTOCSELECTION: {
        switch (scn->listCompletionMethod) {
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
                SciCall_PostMsg(SCI_NEWLINE, 0, 0);
            }
            break;

        default:
            SciCall_AutoCCancel(); // rejected
            break;
        }
    }
    break;


    case SCN_STYLENEEDED: { 
        // this event needs SCI_SETLEXER(SCLEX_CONTAINER)
        //~EditUpdateIndicators(SciCall_GetEndStyled(), scn->position, false);
    }
    break;


    case SCN_UPDATEUI: {
        static DocPos selCnt = 0;

        int const iUpd = scn->updated;

        if (iUpd & (SC_UPDATE_SELECTION | SC_UPDATE_CONTENT)) {
            // Brace Match
            if (Settings.MatchBraces) {
                EditMatchBrace(Globals.hwndEdit);
            }
            if (iUpd & SC_UPDATE_SELECTION) {
                // clear marks only, if selection changed
                if (IsMarkOccurrencesEnabled()) {
                    bool const bValidSel = !SciCall_IsSelectionEmpty() && !Sci_IsMultiOrRectangleSelection();
                    if (bValidSel || Settings.MarkOccurrencesCurrentWord) {
                        MarkAllOccurrences(-1, true);
                    } else {
                        if (Globals.iMarkOccurrencesCount) {
                            EditClearAllOccurrenceMarkers(Globals.hwndEdit);
                        }
                    }
                }
                DocPos const selDiff = (SciCall_GetSelectionEnd() - SciCall_GetSelectionStart());
                if (selDiff != selCnt) {
                    selCnt = selDiff;
                    UpdateToolbar();
                }
            }
            if (iUpd & SC_UPDATE_CONTENT) {
                UpdateMargins(false);
                UpdateTitlebar(Globals.hwndMain);
                //~ Style and Marker are out of scope here => using WM_COMMAND -> SCEN_CHANGE  instead!
                //~MarkAllOccurrences(-1, false);
                //~EditUpdateVisibleIndicators(); // will lead to recursion
            }
            UpdateStatusbar(false);

        } else if (iUpd & SC_UPDATE_V_SCROLL) {

            if (IsMarkOccurrencesEnabled() && Settings.MarkOccurrencesMatchVisible) {
                MarkAllOccurrences(-1, false);
            }
            EditUpdateVisibleIndicators();
        }
    }
    break;


    case SCN_DWELLSTART:
    case SCN_DWELLEND: {
        HandleDWellStartEnd(scn->position, pnmh->code);
    }
    break;


    case SCN_DOUBLECLICK: {
        HandleHotSpotURLClicked(scn->position, SELECT_HYPERLINK); // COPY_HYPERLINK
    }
    break;


    case SCN_CALLTIPCLICK: {
        if (prevCaretPosition >= 0) {
            //~HandleHotSpotURLClicked(SciCall_CallTipPosStart(), OPEN_WITH_BROWSER);
            HandleHotSpotURLClicked(prevCaretPosition, OPEN_WITH_BROWSER);
        }
    }
    break;


    case SCN_INDICATORCLICK: {
        _s_indic_click_modifiers = scn->modifiers;
    }
    break;


    case SCN_INDICATORRELEASE: {
        if (SciCall_IndicatorValueAt(INDIC_NP3_HYPERLINK, scn->position) > 0) {
            bool const bIsNoSel = Sci_IsSingleSelection() && SciCall_IsSelectionEmpty();
            if ((_s_indic_click_modifiers & SCMOD_ALT) && bIsNoSel)
            {
                if (_s_indic_click_modifiers & SCMOD_CTRL) {
                    HandleHotSpotURLClicked(scn->position, OPEN_NEW_NOTEPAD3);
                } else {
                    HandleHotSpotURLClicked(scn->position, OPEN_IN_NOTEPAD3);
                }
            } else if (_s_indic_click_modifiers & SCMOD_CTRL) {
                HandleHotSpotURLClicked(scn->position, OPEN_WITH_BROWSER); // if applicable (file://)
            }
        } else if (SciCall_IndicatorValueAt(INDIC_NP3_COLOR_DEF, scn->position) > 0) {
            if (_s_indic_click_modifiers & SCMOD_CTRL) {
                HandleColorDefClicked(Globals.hwndEdit, scn->position);
            }
        }
        _s_indic_click_modifiers = SCMOD_NORM;
    }
    break;


    case SCN_CHARADDED: {
        int const ich = scn->ch;
        Sci_CallTipCancelEx();

        //if (IsMouseVanish()) {
        //    showMouseCursor(false);
        //}

        if (Sci_IsMultiSelection()) {
            SciCall_SetIndicatorCurrent(INDIC_NP3_MULTI_EDIT);
            DocPosU const selCount = SciCall_GetSelections();
            for (DocPosU s = 0; s < selCount; ++s) {
                DocPos const pos = SciCall_GetSelectionNStart(s);
                SciCall_IndicatorFillRange(SciCall_PositionBefore(pos), 1);
            }
            s_bInMultiEditMode = true;
        }

        switch (ich) {
        case '\r':
        case '\n':
            if (Settings.AutoIndent) {
                _HandleAutoIndent(ich);
            }
            break;
        case '>':
            if (Settings.AutoCloseTags) {
                _HandleAutoCloseTags();
            }
            break;
        case '?':
            _EvalTinyExpr(true);
            break;
        default:
            break;
        }

        if ((Settings.AutoCompleteWords || Settings.AutoCLexerKeyWords)) {
            if (!EditAutoCompleteWord(Globals.hwndEdit, false)) {
                return FALSE;
            }
        }
    }
    break;

    case SCN_AUTOCCHARDELETED:
        if ((Settings.AutoCompleteWords || Settings.AutoCLexerKeyWords)) {
            if (!EditAutoCompleteWord(Globals.hwndEdit, false)) {
                return FALSE;
            }
        }
        break;


    case SCN_NEEDSHOWN: {
        DocLn const iFirstLine = SciCall_LineFromPosition(scn->position);
        DocLn const iLastLine = SciCall_LineFromPosition(scn->position + scn->length - 1);
        for (DocLn i = iFirstLine; i <= iLastLine; ++i) {
            if (!SciCall_GetLineVisible(i)) {
                SciCall_EnsureVisible(i);
            }
        }
        EditUpdateVisibleIndicators();
    }
    break;


    case SCN_MARGINCLICK: {
        switch (scn->margin) {
        case MARGIN_SCI_FOLDING:
            EditFoldClick(SciCall_LineFromPosition(scn->position), scn->modifiers);
            break;
        case MARGIN_SCI_BOOKMRK:
            EditBookmarkToggle(Globals.hwndEdit, SciCall_LineFromPosition(scn->position), scn->modifiers);
            break;
        case MARGIN_SCI_CHGHIST:
        case MARGIN_SCI_LINENUM:
        //~SciCall_GotoLine(SciCall_LineFromPosition(scn->position));
        // fallthrough
        default:
            return FALSE; // not swallowed
        }
    }
    break;


    case SCN_MARGINRIGHTCLICK: {
        POINT pt = { -1, -1 };
        MsgContextMenu(hwnd, SCN_MARGINRIGHTCLICK, MAKEWPARAM(pt.x, pt.y), (LPARAM)scn);
    }
    break;


    // ~~~ Not used in Windows ~~~
    // see: CMD_ALTUP / CMD_ALTDOWN
    //case SCN_KEY:
    //  // Also see the corresponding patch in scintilla\src\Editor.cxx
    //  FoldAltArrow(scn->ch, scn->modifiers);
    //  break;


    case SCN_ZOOM:
        SciCall_SetWhiteSpaceSize(MulDiv(Globals.iWhiteSpaceSize, SciCall_GetZoom(), 100));
        SciCall_SetCaretLineFrame(MulDiv(Globals.iCaretOutLineFrameSize, SciCall_GetZoom(), 100));
        UpdateToolbar();
        UpdateMargins(true);
        break;

#if 0
    case SCN_URIDROPPED: {
        HPATHL         hfile_pth = Path_Allocate(NULL);
        wchar_t* const file_buf = Path_WriteAccessBuf(hfile_pth, STRINGW_MAX_URL_LENGTH);
        int const      cnt = MultiByteToWideChar(CP_UTF8, 0, scn->text, -1, file_buf, (int)Path_GetBufCount(hfile_pth));
        Path_Sanitize(hfile_pth);
        LRESULT const  result = (cnt > 0) ? _OnDropOneFile(hwnd, hfile_pth, NULL) : FALSE;
        Path_Release(hfile_pth);
        return result;
    }
#endif

    default:
        return FALSE; // not swallowed
    }
    return TRUE; // swallowed
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
    UNREFERENCED_PARAMETER(wParam);
    LRESULT result = FALSE;

    SET_FCT_GUARD(result)

    const SCNotification* const scn = (SCNotification*)lParam;

    const LPNMHDR pnmh = (LPNMHDR)scn;
    switch (pnmh->idFrom) {

    case IDC_EDIT: {
        bool bModified = false;
        result = NotifyDocChanged() ? _MsgNotifyFromEdit(hwnd, scn) : _MsgNotifyLean(scn, &bModified);
    } break;

    // ------------------------------------------------------------------------

    case IDC_TOOLBAR:

        switch (pnmh->code) {
        case TBN_QUERYDELETE:
        case TBN_QUERYINSERT:
            // (!) must exist and return true
            result = TRUE;
            break;

        case TBN_BEGINADJUST:
            s_tb_reset_already = false;
            break;

        case TBN_GETBUTTONINFO: {
            if (((LPTBNOTIFY)lParam)->iItem < COUNTOF(s_tbbMainWnd)) {
                WCHAR tch[SMALL_BUFFER] = { L'\0' };
                GetLngString(s_tbbMainWnd[((LPTBNOTIFY)lParam)->iItem].idCommand, tch, COUNTOF(tch));
                StringCchCopyN(((LPTBNOTIFY)lParam)->pszText, ((LPTBNOTIFY)lParam)->cchText, tch, ((LPTBNOTIFY)lParam)->cchText);
                CopyMemory(&((LPTBNOTIFY)lParam)->tbButton, &s_tbbMainWnd[((LPTBNOTIFY)lParam)->iItem], sizeof(TBBUTTON));
                result = TRUE;
            }
        } 
        break;

        case TBN_RESET: {
            int const count = (int)SendMessage(Globals.hwndToolbar, TB_BUTTONCOUNT, 0, 0);
            for (int i = 0; i < count; i++) {
                SendMessage(Globals.hwndToolbar, TB_DELETEBUTTON, 0, 0);
            }
            if (s_tb_reset_already) {
                if (Toolbar_SetButtons(Globals.hwndToolbar, IDT_FILE_NEW, Defaults.ToolbarButtons, s_tbbMainWnd, COUNTOF(s_tbbMainWnd)) == 0) {
                    SendMessage(Globals.hwndToolbar, TB_ADDBUTTONS, COUNTOF(s_tbbMainWnd), (LPARAM)s_tbbMainWnd);
                }
            } else {
                if (Toolbar_SetButtons(Globals.hwndToolbar, IDT_FILE_NEW, Settings.ToolbarButtons, s_tbbMainWnd, COUNTOF(s_tbbMainWnd)) == 0) {
                    SendMessage(Globals.hwndToolbar, TB_ADDBUTTONS, COUNTOF(s_tbbMainWnd), (LPARAM)s_tbbMainWnd);
                }
            }
            s_tb_reset_already = !s_tb_reset_already;
        }
        break;

        case TBN_ENDADJUST:
            UpdateToolbar();
            result = TRUE;
            break;

        case NM_CUSTOMDRAW: {
            LPNMTBCUSTOMDRAW const lpNMTBCustomDraw = (LPNMTBCUSTOMDRAW)lParam;
            result = CDRF_DODEFAULT;
            switch (lpNMTBCustomDraw->nmcd.dwDrawStage) {
            case CDDS_PREPAINT:
                result = CDRF_NOTIFYITEMDRAW;
                break;
            case CDDS_ITEMPREPAINT: {
                //~HDC const hdc = lpNMTBCustomDraw->nmcd.hdc;
                //~if (hdc) {
                //~  SetBkColor(hdc, GetModeBtnfaceColor(UseDarkMode()));
                //~  SetTextColor(hdc, GetModeTextColor(UseDarkMode()));
                //~}
                lpNMTBCustomDraw->clrBtnFace = GetModeBtnfaceColor(UseDarkMode());
                lpNMTBCustomDraw->clrText = GetModeTextColor(UseDarkMode());
                result = TBCDRF_USECDCOLORS;
            }
            break;

            default:
                break;
            }
        }

        default:
            break;
        }

        break;

    // ------------------------------------------------------------------------

    case IDC_STATUSBAR:
        result = TRUE;
        switch(pnmh->code) {
        case NM_CLICK: { // single click
            LPNMMOUSE const pnmm = (LPNMMOUSE)lParam;
            
            if (pnmm->dwItemSpec >= COUNTOF(g_vSBSOrder)) {
                break;
            }
            switch (g_vSBSOrder[pnmm->dwItemSpec]) {
            case STATUS_EOLMODE: {
                if (Globals.bDocHasInconsistentEOLs) {

                    int const eol_mode = SciCall_GetEOLMode();

                    int const  eol_cmd  = (eol_mode == SC_EOL_CRLF) ? IDM_LINEENDINGS_CRLF :
                                          ((eol_mode == SC_EOL_CR) ? IDM_LINEENDINGS_CR : IDM_LINEENDINGS_LF);

                    UINT const msgid    = (eol_mode == SC_EOL_CRLF) ? IDS_MUI_EOLMODENAME_CRLF :
                                          ((eol_mode == SC_EOL_CR) ? IDS_MUI_EOLMODENAME_CR : IDS_MUI_EOLMODENAME_LF);

                    WCHAR wch[64] = {L'\0'};
                    GetLngString(msgid, wch, COUNTOF(wch));
                    if (IsYesOkay(InfoBoxLng(MB_YESNO | MB_ICONWARNING, NULL, IDS_MUI_WARN_NORMALIZE_EOLS, wch))) {
                        PostWMCommand(hwnd, eol_cmd);
                    }
                }
            } 
            break;

            default:
                result = FALSE;
                break;
            }
        }
        break;

        case NM_DBLCLK: { // double click
            LPNMMOUSE const pnmm = (LPNMMOUSE)lParam;
            result = TRUE;
            switch (g_vSBSOrder[pnmm->dwItemSpec]) {
            case STATUS_DOCLINE:
            case STATUS_DOCCOLUMN:
                PostWMCommand(hwnd, IDM_EDIT_GOTOLINE);
                break;

            case STATUS_CODEPAGE:
                PostWMCommand(hwnd, IDM_ENCODING_SELECT);
                break;

            case STATUS_EOLMODE: {
                int const eol_mode = (SciCall_GetEOLMode() + 1) % 3;
                // skip unusual CR-only mode; should be explicitly set by menu or dialog only, so:
                int const eol_cmd = (eol_mode == SC_EOL_CRLF) ? IDM_LINEENDINGS_CRLF : IDM_LINEENDINGS_LF;
                                    //~((eol_mode == SC_EOL_CR) ? IDM_LINEENDINGS_CR : IDM_LINEENDINGS_LF);
                PostWMCommand(hwnd, eol_cmd);
            }
            break;

            case STATUS_OVRMODE:
                PostWMCommand(hwnd, CMD_VK_INSERT);
                break;

            case STATUS_2ND_DEF:
                PostWMCommand(hwnd, IDM_VIEW_USE2NDDEFAULT);
                break;

            case STATUS_LEXER:
                PostWMCommand(hwnd, IDM_VIEW_SCHEME);
                break;

            case STATUS_TINYEXPR: {
                char chExpr[80] = { '\0' };
                if (s_iExprError == 0) {
                    TinyExprToStringA(chExpr, COUNTOF(chExpr), s_dExpression);
                } else if (s_iExprError > 0) {
                    StringCchPrintfA(chExpr, COUNTOF(chExpr), "^[" TE_INT_FMT "]", s_iExprError);
                    SciCall_CopyText((DocPos)StringCchLenA(chExpr, COUNTOF(chExpr)), chExpr);
                }
                SciCall_CopyText((DocPos)StringCchLenA(chExpr, COUNTOF(chExpr)), chExpr);
            }
            break;

            default:
                result = FALSE;
                break;
            }
        }
        break;

        case NM_RCLICK: {
            POINT pt = { -1, -1 };
            GetCursorPos(&pt);
            MsgContextMenu(hwnd, 0, (WPARAM)Globals.hwndStatus, MAKELPARAM(pt.x, pt.y));
        } break;

        default:
            break;
        }
        break;

    // ------------------------------------------------------------------------

    default:
        switch(pnmh->code) {
        // ToolTip
        case TTN_NEEDTEXT:
            if (!(((LPTOOLTIPTEXT)lParam)->uFlags & TTF_IDISHWND)) {
                WCHAR tch[SMALL_BUFFER] = { L'\0' };
                GetLngString((UINT)pnmh->idFrom,tch,COUNTOF(tch));
                WCHAR* pttText = ((LPTOOLTIPTEXT)lParam)->szText;
                size_t const ttLen = COUNTOF(((LPTOOLTIPTEXT)lParam)->szText);
                StringCchCopyN(pttText, ttLen, tch,COUNTOF(tch));
                result = TRUE;
            }
            break;

        default:
            break;
        }
        break;
    }

    RESET_FCT_GUARD();

    return result;
}


//=============================================================================
//
//  ParseCommandLine()
//

static void ParseCmdLnOption(LPWSTR lp1, LPWSTR lp2, const size_t len); // forward decl

// -----------------------------------------------------------------------------

void ParseCommandLine()
{
    Path_Empty(s_pthArgFilePath, false);

    LPWSTR lpCmdLine = GetCommandLine();
    if (StrIsEmpty(lpCmdLine)) {
        return;
    }

    // Good old console can also send args separated by Tabs
    StrTab2Space(lpCmdLine);

    size_t const len = StringCchLen(lpCmdLine,0) + 2UL;
    LPWSTR const lp1 = AllocMem(sizeof(WCHAR)*len,HEAP_ZERO_MEMORY);
    LPWSTR const lp2 = AllocMem(sizeof(WCHAR)*len,HEAP_ZERO_MEMORY);
    LPWSTR const lp3 = AllocMem(sizeof(WCHAR)*len,HEAP_ZERO_MEMORY);

    //assert(!"ParseCommandLine() - ATTACH DEBUGGER NOW");

    if (lp1 && lp2 && lp3) {

        // Start with 2nd argument
        ExtractFirstArgument(lpCmdLine, lp1, lp3, (int)len);

        bool bContinue = true;
        bool bIsFileArg = false;
        s_flagSetEncoding = CPI_NONE;

        while (bContinue && ExtractFirstArgument(lp3, lp1, lp2, (int)len)) {
            // options
            size_t const lp3_len = StringCchLen(lp3, len);
            bIsFileArg = bIsFileArg || (lp3[0] == L'"') && (lp3[lp3_len - 1] == L'"');
            if (!bIsFileArg && (lp1[0] == L'+') && (lp1[1] == L'\0')) {
                Globals.CmdLnFlag_MultiFileArg = 2;
                bIsFileArg = true;
                StringCchCopy(lp3, len, lp2); // next arg
            }
            else if (!bIsFileArg && (lp1[0] == L'-') && (lp1[1] == L'\0')) {
                Globals.CmdLnFlag_MultiFileArg = 1;
                bIsFileArg = true;
                StringCchCopy(lp3, len, lp2); // next arg
            }
            else if (!bIsFileArg && ((*lp1 == L'/') && (*lp2 == '\0')) && (StrStrW(&lp1[1], L"/") != NULL)) {
                bIsFileArg = true; // WSL2 filepath (at least 2 slashes needed)
            } 
            else if (!bIsFileArg && ((*lp1 == L'/') || (*lp1 == L'-'))) {
                StrLTrimI(lp1, L"-/"); // LeftTrim
                ParseCmdLnOption(lp1, lp2, len);
                bIsFileArg = false;
            }
            else {
                bIsFileArg = true;
            }
            // pathname

            if (bIsFileArg) {

                LPWSTR const lpFileBuf = AllocMem(sizeof(WCHAR) * len, HEAP_ZERO_MEMORY);
                if (lpFileBuf) {

                    size_t const fileArgLen = StringCchLen(lp3, len);
                    s_cchiFileList = (int)(StringCchLen(lpCmdLine, len - 2) - fileArgLen);

                    if (s_lpOrigFileArg) {
                        FreeMem(s_lpOrigFileArg);
                        s_lpOrigFileArg = NULL;
                    }
                    size_t const alloc_spc = sizeof(WCHAR) * (fileArgLen + 1);
                    s_lpOrigFileArg = AllocMem(alloc_spc, HEAP_ZERO_MEMORY); // changed for ActivatePrevInst() needs
                    StringCchCopy(s_lpOrigFileArg, alloc_spc, lp3);

                    Path_Reset(s_pthArgFilePath, lp3);

                    if (!Path_IsRelative(s_pthArgFilePath) && !Path_IsUNC(s_pthArgFilePath) && (Path_GetDriveNumber(s_pthArgFilePath) == -1))
                    {
                         HPATHL pthAdjustPath = Path_Copy(Paths.WorkingDirectory);
                         Path_StripToRoot(pthAdjustPath);
                         Path_Append(pthAdjustPath, Path_Get(s_pthArgFilePath));
                         Path_Reset(s_pthArgFilePath, Path_Get(pthAdjustPath));
                         Path_Release(pthAdjustPath);
                    }

                    HPATHL pthAddFile = Path_Allocate(NULL);
                    while ((s_cFileList < FILE_LIST_SIZE) && ExtractFirstArgument(lp3, lpFileBuf, lp3, (int)len)) {
                        Path_Reset(pthAddFile, lpFileBuf);
                        Path_NormalizeEx(pthAddFile, Paths.WorkingDirectory, true, true);
                        Path_QuoteSpaces(pthAddFile, false);
                        s_lpFileList[s_cFileList] = StrDupW(Path_Get(pthAddFile)); // LocalAlloc()
                        s_cFileList += 1;
                    }
                    Path_Release(pthAddFile);
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

static void ParseCmdLnOption(LPWSTR lp1, LPWSTR lp2, const size_t len)
{
    static bool bIsNotepadReplacement = false;

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
    else if (StrCmpNI(lp1, L"appid=", CONSTSTRGLEN(L"appid=")) == 0) {
        StringCchCopyN(Settings2.AppUserModelID, COUNTOF(Settings2.AppUserModelID),
            lp1 + CONSTSTRGLEN(L"appid="), len - CONSTSTRGLEN(L"appid="));
        StrTrim(Settings2.AppUserModelID, L" ");
        if (StrIsEmpty(Settings2.AppUserModelID)) {
            StringCchCopy(Settings2.AppUserModelID, COUNTOF(Settings2.AppUserModelID), _W("Rizonesoft.") _W(SAPPNAME));
        }
    }
    else if (StrCmpNI(lp1, L"sysmru=", CONSTSTRGLEN(L"sysmru=")) == 0) {
        WCHAR wch[16];
        StringCchCopyN(wch, COUNTOF(wch), lp1 + CONSTSTRGLEN(L"sysmru="), COUNTOF(wch));
        StrTrim(wch, L" ");
        if (*wch == L'1') {
            Globals.CmdLnFlag_ShellUseSystemMRU = 2;
        }
        else {
            Globals.CmdLnFlag_ShellUseSystemMRU = 1;
        }
    }
    // Relaunch elevated
    else if (StrCmpNI(lp1, RELAUNCH_ELEVATED_BUF_ARG, CONSTSTRGLEN(RELAUNCH_ELEVATED_BUF_ARG)) == 0) {
        Path_Reset(s_hpthRelaunchElevatedFile, lp1 + CONSTSTRGLEN(RELAUNCH_ELEVATED_BUF_ARG));
        //?TrimSpcW(s_hpthRelaunchElevatedFile);
        Path_NormalizeEx(s_hpthRelaunchElevatedFile, Paths.ModuleDirectory, true, Flags.bSearchPathIfRelative);
        s_IsThisAnElevatedRelaunch = true;
    }

    else {

        switch (*CharUpper(lp1)) {

        case L'N':
            Globals.CmdLnFlag_ReuseWindow = 1;
            if (*CharUpper(lp1 + 1) == L'S') {
                Globals.CmdLnFlag_SingleFileInstance = 2;
            }
            else {
                Globals.CmdLnFlag_SingleFileInstance = 1;
            }
            break;

        case L'R':
            if (*CharUpper(lp1 + 1) == L'P') {
                Flags.bPreserveFileModTime = true;
            }
            else {
                Globals.CmdLnFlag_ReuseWindow = 2;
                if (*CharUpper(lp1 + 1) == L'S') {
                    Globals.CmdLnFlag_SingleFileInstance = 2;
                }
                else {
                    Globals.CmdLnFlag_SingleFileInstance = 1;
                }
            }
            break;

        case L'F':
            if (*(lp1 + 1) == L'0' || *CharUpper(lp1 + 1) == L'O') {
                Path_Reset(Paths.IniFile, L"*?");
            }
            else if (ExtractFirstArgument(lp2, lp1, lp2, (int)len)) {
                WCHAR wchPath[INTERNET_MAX_URL_LENGTH];
                StringCchCopyN(wchPath, COUNTOF(wchPath), lp1, len);
                Path_Reset(Paths.IniFile, wchPath);
                Path_NormalizeEx(Paths.IniFile, Paths.ModuleDirectory, true, false);
            }
            break;

        case L'I':
            s_flagStartAsTrayIcon = true;
            break;

        case L'O':
            if (*(lp1 + 1) == L'0' || *(lp1 + 1) == L'-' || *CharUpper(lp1 + 1) == L'O') {
                Globals.CmdLnFlag_AlwaysOnTop = 1;
            }
            else {
                Globals.CmdLnFlag_AlwaysOnTop = 2;
            }
            break;

        case L'P': {
            WCHAR* lp = lp1;
            if (StrCmpNI(lp1, L"POS:", CONSTSTRGLEN(L"POS:")) == 0) {
                lp += CONSTSTRGLEN(L"POS:") - 1;
            }
            else if (StrCmpNI(lp1, L"POS", CONSTSTRGLEN(L"POS")) == 0) {
                lp += CONSTSTRGLEN(L"POS") - 1;
            }
            else if (*(lp1 + 1) == L':') {
                lp += 1;
            }
            else if (bIsNotepadReplacement) {
                if (*(lp1 + 1) == L'T') {
                    ExtractFirstArgument(lp2, lp1, lp2, (int)len);
                }
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
                    case L'R':
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
                        if (Globals.CmdLnFlag_WindowPos == 0) {
                            Globals.CmdLnFlag_WindowPos |= 64;
                        }
                        Globals.CmdLnFlag_WindowPos |= 128;
                        break;
                    }
                    p = CharNext(p);
                }
            }
            else if (ExtractFirstArgument(lp2, lp1, lp2, (int)len)) {
                WININFO   wi = INIT_WININFO;
                int       iMaximize = 0;
                int const itok = swscanf_s(lp1, WINDOWPOS_STRGFORMAT, &wi.x, &wi.y, &wi.cx, &wi.cy, &wi.dpi, &iMaximize);
                if (itok == 4 || itok == 5 || itok == 6) { // scan successful
                    Globals.CmdLnFlag_PosParam = true;
                    Globals.CmdLnFlag_WindowPos = 0;
                    if (itok == 4) {
                        wi.dpi = USER_DEFAULT_SCREEN_DPI;
                        iMaximize = 0;
                    }
                    else if (itok == 5) { // maybe DPI or Maxi (old)
                        if (wi.dpi < (USER_DEFAULT_SCREEN_DPI >> 2)) {
                            iMaximize = wi.dpi;
                            wi.dpi = USER_DEFAULT_SCREEN_DPI;
                        }
                        else {
                            iMaximize = 0;
                        }
                    }
                    wi.max = !!iMaximize;
                    g_IniWinInfo = wi; // set window placement
                }
            }
        } break;

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
                int itok = swscanf_s(lp1, L"%i,%i", &s_iInitialLine, &s_iInitialColumn);
                if (itok == 1 || itok == 2) { // scan successful
                    s_flagJumpTo = true;
                }
            }
            break;

        case L'M': {
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
            if (ExtractFirstArgument(lp2, lp1, lp2, (int)len)) {

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
        } break;

        case L'L':
            if (*(lp1 + 1) == L'0' || *(lp1 + 1) == L'-' || *CharUpper(lp1 + 1) == L'O') {
                s_flagChangeNotify = FWM_MSGBOX;
            }
            else if (*(lp1 + 1) == L'1' || *(lp1 + 1) == L'+' || *CharUpper(lp1 + 1) == L'X') {
                s_flagChangeNotify = FWM_EXCLUSIVELOCK;
            }
            else {
                s_flagChangeNotify = FWM_AUTORELOAD;
            }
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
                if (s_lpSchemeArg) {
                    LocalFree(s_lpSchemeArg); // StrDup()
                    s_lpSchemeArg = NULL;
                }
                s_lpSchemeArg = StrDup(lp1);
                s_flagLexerSpecified = true;
            }
            break;

        case L'D':
            if (s_lpSchemeArg) {
                LocalFree(s_lpSchemeArg); // StrDup()
                s_lpSchemeArg = NULL;
            }
            s_iInitialLexer = 0;
            s_flagLexerSpecified = true;
            break;

        case L'H':
            if (s_lpSchemeArg) {
                LocalFree(s_lpSchemeArg); // StrDup()
                s_lpSchemeArg = NULL;
            }
            s_iInitialLexer = 35;
            s_flagLexerSpecified = true;
            break;

        case L'X':
            if (s_lpSchemeArg) {
                LocalFree(s_lpSchemeArg); // StrDup()
                s_lpSchemeArg = NULL;
            }
            s_iInitialLexer = 36;
            s_flagLexerSpecified = true;
            break;

        case L'U':
            if (*CharUpper(lp1 + 1) == L'C') {
                SetEvent(s_hEventAppIsClosing);
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
                Globals.CmdLnFlag_PrintFileAndLeave = 2; // open printer dialog
            }
            break;

        default:
            break;
        }
    }
}


//=============================================================================
//
//  CheckAutoLoadMostRecent()
//
bool CheckAutoLoadMostRecent()
{
    // Add most recent from file history
    if (Settings.AutoLoadMRUFile && !Globals.CmdLnFlag_SingleFileInstance && Path_IsEmpty(s_pthArgFilePath)) {
        if (MRU_Count(Globals.pFileMRU) > 0) {
            LPWSTR const szFileBuf = Path_WriteAccessBuf(s_pthArgFilePath, PATHLONG_MAX_CCH); // reserve buffer
            int const    cchFileBuf = (int)Path_GetBufCount(s_pthArgFilePath);
            MRU_Enum(Globals.pFileMRU, 0, szFileBuf, cchFileBuf);
            Path_Sanitize(s_pthArgFilePath);
            Path_UnQuoteSpaces(s_pthArgFilePath);
            Path_AbsoluteFromApp(s_pthArgFilePath, true);
            return true;
        }
    }
    return false;
}


//=============================================================================
//
//  _DelayUpdateStatusbar()
//
static void  _DelayUpdateStatusbar(const int delay, const bool bForceRedraw)
{
    CmdMessageQueue_t mqc = MQ_WM_CMD_INIT(Globals.hwndMain, IDT_TIMER_UPDATE_STATUSBAR, bForceRedraw);
    _MQ_AppendCmd(&mqc, _MQ_ms2cycl(delay));
}


//=============================================================================
//
//  _DelayUpdateToolbar()
//
static void _DelayUpdateToolbar(const LONG64 delay)
{
    CmdMessageQueue_t mqc = MQ_WM_CMD_INIT(Globals.hwndMain, IDT_TIMER_UPDATE_TOOLBAR, 0LL);
    _MQ_AppendCmd(&mqc, _MQ_ms2cycl(delay));
}


//=============================================================================
//
//  _DelayUpdateTitlebar()
//
static void _DelayUpdateTitlebar(const LONG64 delay, const HWND hwnd)
{
    CmdMessageQueue_t mqc = MQ_WM_CMD_INIT(Globals.hwndMain, IDT_TIMER_UPDATE_TITLEBAR, (LPARAM)hwnd);
    _MQ_AppendCmd(&mqc, _MQ_ms2cycl(delay));
}


//=============================================================================
//
//  _DelayClearCallTip()
//
static void _DelayClearCallTip(const LONG64 delay)
{
    CmdMessageQueue_t mqc = MQ_WM_CMD_INIT(Globals.hwndMain, IDT_TIMER_CLEAR_CALLTIP, 0LL);
    _MQ_AppendCmd(&mqc, _MQ_ms2cycl(delay));
}


//=============================================================================
//
//  _DelaySplitUndoTransaction()
//
static void _DelaySplitUndoTransaction(const LONG64 delay)
{
    CmdMessageQueue_t mqc = MQ_WM_CMD_INIT(Globals.hwndMain, IDT_TIMER_UNDO_TRANSACTION, 0);
    _MQ_AppendCmd(&mqc, _MQ_ms2cycl(delay));
}


//=============================================================================
//
//  MarkAllOccurrences()
//
void MarkAllOccurrences(const LONG64 delay, const bool bForceClear)
{
    CmdMessageQueue_t mqc = MQ_WM_CMD_INIT(Globals.hwndMain, IDT_TIMER_CALLBACK_MRKALL, bForceClear);
    LONG64 const      timer = (delay < 0) ? Settings2.UpdateDelayMarkAllOccurrences : delay;
    _MQ_AppendCmd(&mqc, _MQ_ms2cycl(timer));
}


//=============================================================================
//
//  UpdateTitlebar()
//
void UpdateTitlebar(const HWND hwnd)
{
    _DelayUpdateTitlebar(_MQ_STD, hwnd);
}


//=============================================================================
//
//  _UpdateTitlebarDelayed()
//

static void _UpdateTitlebarDelayed(const HWND hwnd)
{
    if (hwnd == Globals.hwndMain && Settings.ShowTitlebar) {

        TITLEPROPS_T props = { 0 };
        props.iFormat = Settings.PathNameFormat;
        props.bPasteBoard = s_flagPasteBoard;
        props.bIsElevated = s_bIsProcessElevated;
        props.bModified = IsSaveNeeded();
        props.bFileLocked = (FileWatching.FileWatchingMode == FWM_EXCLUSIVELOCK);
        props.bFileChanged = IsFileChangedFlagSet();
        props.bFileDeleted = IsFileDeletedFlagSet();
        props.bReadOnly = IsFileReadOnly();

        SetWindowTitle(Globals.hwndMain, Paths.CurrentFile, props, s_wchTitleExcerpt, false);
    }
    //if (!IsWindows10OrGreater()) {
    //    PostMessage(hwnd, WM_NCACTIVATE, FALSE, -1); // (!)
    //    PostMessage(hwnd, WM_NCACTIVATE, TRUE, 0);
    //}
}


//=============================================================================
//
//  UpdateToolbar()
//
void UpdateToolbar()
{
    _DelayUpdateToolbar(_MQ_STD);
    _DelayUpdateStatusbar(_MQ_STD, false);
    _DelayUpdateTitlebar(_MQ_STD, Globals.hwndMain);
}

void UpdateToolbar_Now(const HWND hwnd)
{
    _UpdateToolbarDelayed();
    _UpdateStatusbarDelayed(true);
    _UpdateTitlebarDelayed(hwnd);
}


//=============================================================================

static void  _UpdateToolbarDelayed()
{
    if (!Settings.ShowToolbar) {
        return;
    }

    EnableTool(Globals.hwndToolbar, IDT_FILE_ADDTOFAV, Path_IsNotEmpty(Paths.CurrentFile));
    EnableTool(Globals.hwndToolbar, IDT_FILE_SAVE, IsSaveNeeded() || IsFileChangedFlagSet() /*&& !bReadOnly*/);
    EnableTool(Globals.hwndToolbar, IDT_FILE_RECENT, (MRU_Count(Globals.pFileMRU) > 0));

    CheckTool(Globals.hwndToolbar, IDT_VIEW_WORDWRAP, Globals.fvCurFile.bWordWrap);
    CheckTool(Globals.hwndToolbar, IDT_VIEW_CHASING_DOCTAIL, FileWatching.MonitoringLog);
    CheckTool(Globals.hwndToolbar, IDT_VIEW_PIN_ON_TOP, Settings.AlwaysOnTop);

    bool const b1 = SciCall_IsSelectionEmpty();
    bool const b2 = !Sci_IsDocEmpty();
    bool const ro = SciCall_GetReadOnly();
    bool const tv = FocusedView.HideNonMatchedLines;

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

    int const zoom = SciCall_GetZoom();
    CheckTool(Globals.hwndToolbar, IDT_VIEW_ZOOMIN,    (zoom > 100));
    CheckTool(Globals.hwndToolbar, IDT_VIEW_RESETZOOM, (zoom == 100));
    CheckTool(Globals.hwndToolbar, IDT_VIEW_ZOOMOUT,   (zoom < 100));
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
    // main window width changed ?
    static int s_iWinFormerWidth = -1;
    if (s_iWinFormerWidth != s_WinCurrentWidth) {
        *bIsUpdNeeded = true;
        s_iWinFormerWidth = s_WinCurrentWidth;
    }
    if (!(*bIsUpdNeeded)) {
        return;
    }

    // count fixed and dynamic optimized pixels
    int pxCount = 0;
    for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
        if (g_iStatusbarVisible[i]) {
            if (g_iStatusbarWidthSpec[i] == 0) { // dynamic optimized
                SIZE const size = _StatusCalcTextSize(Globals.hwndStatus, tchStatusBar[i]);
                vSectionWidth[i] = (size.cx + 8L);
            } else if (g_iStatusbarWidthSpec[i] < -1) { // fixed pixel count
                vSectionWidth[i] = -(g_iStatusbarWidthSpec[i]);
            }
            //else { /* 0,-1 : relative counts */ }
            // accumulate
            if (vSectionWidth[i] > 0) {
                pxCount += vSectionWidth[i];
            }
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
            SIZE const size = _StatusCalcTextSize(Globals.hwndStatus, tchStatusBar[i]);
            int const iMinWidth = (size.cx + 8L);
            vMinWidth[i] = iMinWidth;
            iTotalMinWidth += iMinWidth;
        }
    }

    if (iTotalMinWidth >= iPropSectTotalWidth) {
        for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
            if (bIsPropSection[i]) {
                vSectionWidth[i] = vMinWidth[i];
            }
        }
    } else { // space left for proportional elements
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
        } else { // handling overlaps
            while (iOverlappingText > 0) {
                const int iNoProgress = iOverlappingText;
                for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
                    if (bIsPropSection[i]) {
                        if (vMinWidth[i] < vPropWidth[i]) {
                            vPropWidth[i] -= 1;
                            --iOverlappingText;
                        } else if (vMinWidth[i] > vPropWidth[i]) {
                            vPropWidth[i] = vMinWidth[i];
                        }
                    }
                    if (iOverlappingText == 0) {
                        break; /* for */
                    }
                }
                if ((iOverlappingText == 0) || (iNoProgress == iOverlappingText)) {
                    break;
                }
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
static double _InterpMultiSelectionTinyExpr(te_int_t* piExprError)
{
#define _tmpBufCnt 128
    char tmpRectSelN[_tmpBufCnt] = { '\0' };

    DocPosU const selCount = SciCall_GetSelections();
    int const calcBufSize = (int)(_tmpBufCnt * selCount + 1);
    char * const calcBuffer = (char*)AllocMem(calcBufSize, HEAP_ZERO_MEMORY);
    WCHAR * const calcBufferW = (WCHAR*)AllocMem(calcBufSize * sizeof(WCHAR), HEAP_ZERO_MEMORY);

    bool bLastCharWasDigit = false;
    for (DocPosU i = 0; i < selCount; ++i) {
        DocPos const posSelStart = SciCall_GetSelectionNStart(i);
        DocPos const posSelEnd = SciCall_GetSelectionNEnd(i);
        size_t const cchToCopy = (size_t)(posSelEnd - posSelStart);
        StringCchCopyNA(tmpRectSelN, _tmpBufCnt, SciCall_GetRangePointer(posSelStart, (DocPos)cchToCopy), cchToCopy);
        StrTrimA(tmpRectSelN, " ");

        char const defchar = (char)0x24;
        MultiByteToWideChar(Encoding_SciCP, 0, calcBuffer, -1, calcBufferW, calcBufSize);
        WideCharToMultiByte(1252, (WC_COMPOSITECHECK | WC_DISCARDNS), calcBufferW, -1, calcBuffer, calcBufSize, &defchar, NULL);
        StrDelChrA(calcBuffer, chr_currency);

        if (!StrIsEmptyA(tmpRectSelN)) {
            if (IsDigitA(tmpRectSelN[0]) && bLastCharWasDigit) {
                StringCchCatA(calcBuffer, SizeOfMem(calcBuffer), "+"); // default: add numbers
            }
            bLastCharWasDigit = IsDigitA(tmpRectSelN[StringCchLenA(tmpRectSelN,COUNTOF(tmpRectSelN)) - 1]);
            StringCchCatA(calcBuffer, SizeOfMem(calcBuffer), tmpRectSelN);
        }
    }
    double const result = te_interp(calcBuffer, piExprError);
    FreeMem(calcBufferW);
    FreeMem(calcBuffer);
    return result;
}


//=============================================================================
//
//  UpdateStatusbar()
//
//
void UpdateStatusbar(const bool bForceRedraw)
{
    _DelayUpdateStatusbar(_MQ_FAST, bForceRedraw);
}

//=============================================================================

const static WCHAR *const FR_StatusW[] = { L"[>--<]", L"[>>--]", L"[>>-+]", L"[+->]>", L"[--<<]", L"[+-<<]", L"<[<-+]" };


static void  _UpdateStatusbarDelayed(bool bForceRedraw)
{
    if (!Settings.ShowStatusbar) {
        return;
    }

    static sectionTxt_t tchStatusBar[STATUS_SECTOR_COUNT] = { L'\0' };

    // ------------------------------------------------------
    // common calculations
    // ------------------------------------------------------
    DocPos const iPos              = SciCall_GetCurrentPos();
    DocLn  const iLnFromPos        = SciCall_LineFromPosition(iPos);
    DocPos const iLineBegin        = SciCall_PositionFromLine(iLnFromPos);
    DocPos const iLineBack         = SciCall_GetLineEndPosition(iLnFromPos);
    DocPos const iSelStart         = SciCall_GetSelectionStart();
    DocPos const iSelEnd           = SciCall_GetSelectionEnd();

    bool const bIsSelectionEmpty = SciCall_IsSelectionEmpty();
    bool const bIsMultiSelection = Sci_IsMultiOrRectangleSelection();
    bool const bIsSelCharCountable = !bIsSelectionEmpty && !bIsMultiSelection;
    bool const bIsWindowFindReplace = IsWindow(Globals.hwndDlgFindReplace);

    bool bIsUpdateNeeded = bForceRedraw;

    static WCHAR tchLn[32] = { L'\0' };
    static WCHAR tchLines[32] = { L'\0' };

    // ------------------------------------------------------

    if (g_iStatusbarVisible[STATUS_DOCLINE] || bIsWindowFindReplace) {
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

        if (bForceRedraw || ((s_iLnFromPos != iLnFromPos) || (s_iLnCnt != iLnCnt))) {
            StringCchPrintf(tchStatusBar[STATUS_DOCLINE], txtWidth, L"%s%s / %s%s",
                            g_mxSBPrefix[STATUS_DOCLINE], tchLn, tchLines, g_mxSBPostfix[STATUS_DOCLINE]);
            s_iLnFromPos = iLnFromPos;
            s_iLnCnt = iLnCnt;
            bIsUpdateNeeded = true;
        }
    }
    // ------------------------------------------------------

    static WCHAR tchCol[32] = { L'\0' };

    if (g_iStatusbarVisible[STATUS_DOCCOLUMN] || bIsWindowFindReplace) {
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

    if (g_iStatusbarVisible[STATUS_DOCCHAR]) {
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

    if (g_iStatusbarVisible[STATUS_UNICODEPT]) {

        static WCHAR     tchChr[32] = { L'\0' };
        static UINT64    s_wChr = L'\0';
        static int const len = sizeof(UINT64) / sizeof(WCHAR);

        DocPos const iPosAfter = SciCall_PositionAfter(iPos);
        int const    chrLen = (int)(iPosAfter - iPos);

        char chChrs[8] = { '\0' };
        struct Sci_TextRangeFull tr = { { iPos, iPosAfter }, chChrs };
        SciCall_GetTextRangeFull(&tr);

        UINT64 wChr = L'\0';
        MultiByteToWideChar(Encoding_SciCP, 0, chChrs, chrLen, (LPWSTR)&wChr, len);

        if (bForceRedraw || (s_wChr != wChr)) {
            if (wChr <= 0x7F)
                StringCchPrintf(tchChr, COUNTOF(tchChr), L"0x%.4X (ASC:%i)", LOWORD(wChr), LOWORD(wChr));
            else if (wChr <= 0xFFFF)
                StringCchPrintf(tchChr, COUNTOF(tchChr), L"0x%.4X (DEC:%i)", LOWORD(wChr), LOWORD(wChr));
            else
                StringCchPrintf(tchChr, COUNTOF(tchChr), L"0x%.4X 0x%.4X", LOWORD(wChr), HIWORD(wChr));
        }
        if (s_wChr != wChr) {
            StringCchPrintf(tchStatusBar[STATUS_UNICODEPT], txtWidth, L"%s%s%s",
                g_mxSBPrefix[STATUS_UNICODEPT], tchChr, g_mxSBPostfix[STATUS_UNICODEPT]);
            s_wChr = wChr;
            bIsUpdateNeeded = true;
        }
    }
    // ------------------------------------------------------

    static WCHAR tchSel[32] = { L'\0' };

    // number of selected chars in statusbar
    if (g_iStatusbarVisible[STATUS_SELECTION] || g_iStatusbarVisible[STATUS_SELCTBYTES] || bIsWindowFindReplace) {
        static bool s_bIsSelCountable = false;
        static bool s_bIsMultiSelection = false;
        static DocPos s_iSelStart = -1;
        static DocPos s_iSelEnd = -1;

        if (bForceRedraw || ((s_bIsSelCountable != bIsSelCharCountable) || (s_iSelStart != iSelStart)
                             || (s_iSelEnd != iSelEnd)) || (s_bIsMultiSelection != bIsMultiSelection)) {
            static WCHAR tchSelB[64] = { L'\0' };
            if (bIsSelCharCountable) {
                DocPos const iSel = Flags.bHugeFileLoadState ? (iSelEnd - iSelStart) : SciCall_CountCharacters(iSelStart, iSelEnd);
                StringCchPrintf(tchSel, COUNTOF(tchSel), DOCPOSFMTW, iSel);
                FormatNumberStr(tchSel, COUNTOF(tchSel), 0);
                StrFormatByteSizeEx((ULONGLONG)(iSelEnd - iSelStart), SFBS_FLAGS_ROUND_TO_NEAREST_DISPLAYED_DIGIT, tchSelB, COUNTOF(tchSelB));
            } else if (bIsMultiSelection) {
                StringCchPrintf(tchSel, COUNTOF(tchSel), L"# " DOCPOSFMTW, SciCall_GetSelections());
                tchSelB[0] = L'0';
                tchSelB[1] = L'\0';
            } else {
                tchSel[0] = L'-';
                tchSel[1] = L'-';
                tchSel[2] = L'\0';
                tchSelB[0] = L'0';
                tchSelB[1] = L'\0';
            }

            if (Flags.bHugeFileLoadState) {
                StringCchPrintf(tchStatusBar[STATUS_SELECTION], txtWidth, L"%s%s%s",
                                g_mxSBPrefix[STATUS_SELCTBYTES], tchSel, g_mxSBPostfix[STATUS_SELCTBYTES]);
            } else {
                StringCchPrintf(tchStatusBar[STATUS_SELECTION], txtWidth, L"%s%s%s",
                                g_mxSBPrefix[STATUS_SELECTION], tchSel, g_mxSBPostfix[STATUS_SELECTION]);
            }
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
    if (g_iStatusbarVisible[STATUS_SELCTLINES]) {
        static bool s_bIsSelectionEmpty = true;
        static DocLn s_iLinesSelected = -1;

        DocLn const iLineStart = SciCall_LineFromPosition(iSelStart);
        DocLn const iLineEnd = SciCall_LineFromPosition(iSelEnd);
        DocPos const iStartOfLinePos = SciCall_PositionFromLine(iLineEnd);

        DocLn const iLinesSelected = ((iSelStart != iSelEnd) && (iStartOfLinePos != iSelEnd)) ? ((iLineEnd - iLineStart) + 1) : (iLineEnd - iLineStart);

        if (bForceRedraw || ((s_bIsSelectionEmpty != bIsSelectionEmpty) || (s_iLinesSelected != iLinesSelected))) {
            static bool s_bIsMultiSelection = false;
            static WCHAR tchLinesSelected[32] = { L'\0' };
            if (bIsSelectionEmpty || bIsMultiSelection) {
                tchLinesSelected[0] = L'-';
                tchLinesSelected[1] = L'-';
                tchLinesSelected[2] = L'\0';
            } else {
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
    if (g_iStatusbarVisible[STATUS_TINYEXPR]) {
        static WCHAR tchExpression[32] = { L'\0' };
        static te_int_t s_iExErr          = -3;
        s_dExpression = 0.0;
        tchExpression[0] = L'-';
        tchExpression[1] = L'-';
        tchExpression[2] = L'\0';

        if (Settings.EvalTinyExprOnSelection) {
            if (bIsSelCharCountable) {
                static char chSeBuf[LARGE_BUFFER] = { '\0' };
                static WCHAR wchSelBuf[LARGE_BUFFER] = { L'\0' };
                DocPos const iSelLen = SciCall_GetSelText(NULL);
                if (iSelLen < COUNTOF(chSeBuf)) { // should be fast !
                    SciCall_GetSelText(chSeBuf);
                    //~StrDelChrA(chExpression, " \r\n\t\v");
                    StrDelChrA(chSeBuf, "\r\n");
                    StrTrimA(chSeBuf, "= ?");

                    char const defchar = (char)0x24;
                    MultiByteToWideChar(Encoding_SciCP, 0, chSeBuf, -1, wchSelBuf, LARGE_BUFFER);
                    WideCharToMultiByte(1252, (WC_COMPOSITECHECK | WC_DISCARDNS), wchSelBuf, -1, chSeBuf, LARGE_BUFFER, &defchar, NULL);
                    StrDelChrA(chSeBuf, chr_currency);

                    s_dExpression = te_interp(chSeBuf, &s_iExprError);
                } else {
                    s_iExprError = -1;
                }
            } else if (Sci_IsMultiOrRectangleSelection() && !bIsSelectionEmpty) {
                s_dExpression = _InterpMultiSelectionTinyExpr(&s_iExprError);
            } else {
                s_iExprError = -2;
            }
        } else {
            s_iExprError = -3;
        }

        if (!s_iExprError) {
            TinyExprToString(tchExpression, COUNTOF(tchExpression), s_dExpression);
        } else if (s_iExprError > 0) {
            StringCchPrintf(tchExpression, COUNTOF(tchExpression), L"^[" _W(TE_INT_FMT) L"]", s_iExprError);
        }

        if (bForceRedraw || (!s_iExprError || (s_iExErr != s_iExprError))) {
            StringCchPrintf(tchStatusBar[STATUS_TINYEXPR], txtWidth, L"%s%s%s ",
                            g_mxSBPrefix[STATUS_TINYEXPR], tchExpression, g_mxSBPostfix[STATUS_TINYEXPR]);

            s_iExErr = s_iExprError;
            bIsUpdateNeeded = true;
        }
    }
    // ------------------------------------------------------

    static WCHAR tchOccOf[50] = { L'\0' };
    static WCHAR tchOcc2[24] = { L'\0' };

    // number of occurrence marks found
    if (g_iStatusbarVisible[STATUS_OCCURRENCE] || bIsWindowFindReplace) {
        static DocPosU s_iSeletionMarkNumber = 0;
        static DocPosU s_iMarkOccurrencesCount = 0;
        static bool s_bMOVisible = false;
        if (bForceRedraw || ((s_bMOVisible != Settings.MarkOccurrencesMatchVisible) ||
                             (s_iSeletionMarkNumber != Globals.iSelectionMarkNumber) ||
                             (s_iMarkOccurrencesCount != Globals.iMarkOccurrencesCount))) {
            if (Globals.iMarkOccurrencesCount) {
                StringCchPrintf(tchOccOf, COUNTOF(tchOccOf), DOCPOSFMTW, Globals.iSelectionMarkNumber);
                FormatNumberStr(tchOccOf, COUNTOF(tchOccOf), 0);
                StringCchPrintf(tchOcc2, COUNTOF(tchOcc2), DOCPOSFMTW, Globals.iMarkOccurrencesCount);
                FormatNumberStr(tchOcc2, COUNTOF(tchOcc2), 0);
                StringCchCat(tchOccOf, COUNTOF(tchOccOf), L"/");
                StringCchCat(tchOccOf, COUNTOF(tchOccOf), tchOcc2);
                
            } else {
                StringCchCopy(tchOccOf, COUNTOF(tchOccOf), L"-/-");
            }
            if (Settings.MarkOccurrencesMatchVisible) {
                StringCchCat(tchOccOf, COUNTOF(tchOccOf), L"(V)");
            }

            StringCchPrintf(tchStatusBar[STATUS_OCCURRENCE], txtWidth, L"%s%s%s",
                            g_mxSBPrefix[STATUS_OCCURRENCE], tchOccOf, g_mxSBPostfix[STATUS_OCCURRENCE]);

            s_bMOVisible = Settings.MarkOccurrencesMatchVisible;
            s_iSeletionMarkNumber = Globals.iSelectionMarkNumber;
            s_iMarkOccurrencesCount = Globals.iMarkOccurrencesCount;
            bIsUpdateNeeded = true;
        }
    }
    // ------------------------------------------------------

    // number of replaced pattern
    if (g_iStatusbarVisible[STATUS_OCCREPLACE] || bIsWindowFindReplace) {
        static DocPosU s_iReplacedOccurrences = 0;
        if (bForceRedraw || (s_iReplacedOccurrences != Globals.iReplacedOccurrences)) {
            static WCHAR tchRepl[64] = { L'\0' };
            if (Globals.iReplacedOccurrences) {
                StringCchPrintf(tchRepl, COUNTOF(tchRepl), DOCPOSFMTW, Globals.iReplacedOccurrences);
                FormatNumberStr(tchRepl, COUNTOF(tchRepl), 0);
            } else {
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
    if (g_iStatusbarVisible[STATUS_DOCSIZE]) {
        static DocPos s_iTextLength = -1;
        DocPos const iTextLength = SciCall_GetTextLength();
        if (bForceRedraw || (s_iTextLength != iTextLength)) {
            static WCHAR tchBytes[32] = { L'\0' };
            StrFormatByteSizeEx((ULONGLONG)iTextLength, SFBS_FLAGS_ROUND_TO_NEAREST_DISPLAYED_DIGIT, tchBytes, COUNTOF(tchBytes));

            StringCchPrintf(tchStatusBar[STATUS_DOCSIZE], txtWidth, L"%s%s%s",
                            g_mxSBPrefix[STATUS_DOCSIZE], tchBytes, g_mxSBPostfix[STATUS_DOCSIZE]);

            s_iTextLength = iTextLength;
            bIsUpdateNeeded = true;
        }
    }
    // ------------------------------------------------------

    if (g_iStatusbarVisible[STATUS_CODEPAGE]) {
        static cpi_enc_t s_iEncoding = CPI_NONE;
        cpi_enc_t const  iEncoding   = Encoding_GetCurrent();
        if (bForceRedraw || (s_iEncoding != iEncoding)) {
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

    if (g_iStatusbarVisible[STATUS_EOLMODE]) {
        static int s_iEOLMode = -1;
        int const eol_mode = SciCall_GetEOLMode();

        if (bForceRedraw || (s_iEOLMode != eol_mode)) {
            static WCHAR tchEOL[16] = { L'\0' };
            if (eol_mode == SC_EOL_LF) {
                StringCchPrintf(tchStatusBar[STATUS_EOLMODE], txtWidth, (Globals.bDocHasInconsistentEOLs ? _LFi_f : _LF_f),
                                g_mxSBPrefix[STATUS_EOLMODE], g_mxSBPostfix[STATUS_EOLMODE]);
            } else if (eol_mode == SC_EOL_CR) {
                StringCchPrintf(tchStatusBar[STATUS_EOLMODE], txtWidth, (Globals.bDocHasInconsistentEOLs ? _CRi_f : _CR_f),
                                g_mxSBPrefix[STATUS_EOLMODE], g_mxSBPostfix[STATUS_EOLMODE]);
            } else {
                StringCchPrintf(tchStatusBar[STATUS_EOLMODE], txtWidth, (Globals.bDocHasInconsistentEOLs ? _CRLFi_f : _CRLF_f),
                                g_mxSBPrefix[STATUS_EOLMODE], g_mxSBPostfix[STATUS_EOLMODE]);
            }
            s_iEOLMode = eol_mode;
            bIsUpdateNeeded = true;
        }
    }
    // ------------------------------------------------------

    if (g_iStatusbarVisible[STATUS_OVRMODE]) {
        static bool s_bIsOVR = -1;
        bool const bIsOVR = SciCall_GetOverType();
        if (bForceRedraw || (s_bIsOVR != bIsOVR)) {
            if (bIsOVR) {
                StringCchPrintf(tchStatusBar[STATUS_OVRMODE], txtWidth, L"%sOVR%s",
                                g_mxSBPrefix[STATUS_OVRMODE], g_mxSBPostfix[STATUS_OVRMODE]);
            } else {
                StringCchPrintf(tchStatusBar[STATUS_OVRMODE], txtWidth, L"%sINS%s",
                                g_mxSBPrefix[STATUS_OVRMODE], g_mxSBPostfix[STATUS_OVRMODE]);
            }
            s_bIsOVR = bIsOVR;
            bIsUpdateNeeded = true;
        }
    }
    // ------------------------------------------------------

    if (g_iStatusbarVisible[STATUS_2ND_DEF]) {
        static bool s_bUse2ndDefault = true;
        bool const bUse2ndDefault = Style_GetUse2ndDefault();
        if (bForceRedraw || (s_bUse2ndDefault != bUse2ndDefault)) {
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

    if (g_iStatusbarVisible[STATUS_LEXER]) {
        static int s_iCurLexer = -1;
        int const iCurLexer = Style_GetCurrentLexerRID();
        if (bForceRedraw || (s_iCurLexer != iCurLexer)) {
            static WCHAR tchLexerName[MINI_BUFFER];
            if (Style_IsCurLexerStandard()) {
                Style_GetLexerDisplayName(NULL, tchLexerName, MINI_BUFFER);    // don't distinguish between STD/2ND
            } else {
                Style_GetLexerDisplayName(Style_GetCurrentLexerPtr(), tchLexerName, MINI_BUFFER);
            }

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
            if ((id >= 0) && (g_vStatusbarSectionWidth[id] >= 0)) {
                totalWidth += g_vStatusbarSectionWidth[id];
                aStatusbarSections[cnt++] = totalWidth;
            }
        }

        if (cnt > 0) {
            aStatusbarSections[cnt - 1] = -1; // expand to right edge
        } else {
            aStatusbarSections[0] = -1;
            Settings.ShowStatusbar = false;
        }


        SendMessage(Globals.hwndStatus, WM_SETREDRAW, FALSE, 0);

        SIZE const size = _StatusCalcTextSize(Globals.hwndStatus, L"X");
        SendMessage(Globals.hwndStatus, SB_SETMINHEIGHT, MAKEWPARAM(size.cy + 2, 0), 0);

        SendMessage(Globals.hwndStatus, SB_SETPARTS, (WPARAM)cnt, (LPARAM)aStatusbarSections);
        cnt = 0;
        for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
            int const id = g_vSBSOrder[i];
            if ((id >= 0) && (g_vStatusbarSectionWidth[id] >= 0)) {
                StatusSetText(Globals.hwndStatus, cnt++, tchStatusBar[id]);
            }
        }

        SendMessage(Globals.hwndStatus, WM_SETREDRAW, TRUE, 0);
        InvalidateRect(Globals.hwndStatus, NULL, TRUE);

    }
    // --------------------------------------------------------------------------

    // update Find/Replace dialog (if any)
    if (bIsWindowFindReplace) {
        static WCHAR tchReplOccs[64] = { L'\0' };
        if (Globals.iReplacedOccurrences) {
            StringCchPrintf(tchReplOccs, COUNTOF(tchReplOccs), DOCPOSFMTW, Globals.iReplacedOccurrences);
            FormatNumberStr(tchReplOccs, COUNTOF(tchReplOccs), 0);
        }
        else {
            StringCchCopy(tchReplOccs, COUNTOF(tchReplOccs), L"--");
        }

        const WCHAR* const SBFMT = L" %s%s / %s     %s%s     %s%s     %s%s     %s%s     (  %s  )              ";

        static WCHAR tchFRStatus[128] = { L'\0' };
        StringCchPrintf(tchFRStatus, COUNTOF(tchFRStatus), SBFMT,
                        g_mxSBPrefix[STATUS_DOCLINE], tchLn, tchLines,
                        g_mxSBPrefix[STATUS_DOCCOLUMN], tchCol,
                        g_mxSBPrefix[STATUS_SELECTION], tchSel,
                        g_mxSBPrefix[STATUS_OCCURRENCE], tchOccOf,
                        g_mxSBPrefix[STATUS_OCCREPLACE], tchReplOccs,
                        FR_StatusW[Globals.FindReplaceMatchFoundState]);

        SetWindowText(GetDlgItem(Globals.hwndDlgFindReplace, IDS_FR_STATUS_TEXT), tchFRStatus);
    }

}


//=============================================================================
//
//  UpdateMargins()
//
//
void UpdateMargins(const bool bForce)
{
    Style_UpdateAllMargins(Globals.hwndEdit, bForce);
}


//=============================================================================
//
//  UpdateSaveSettingsCmds()
//
//
void UpdateSaveSettingsCmds()
{
    bool const bSoftLocked = Flags.bSettingsFileSoftLocked;
    bool const bHaveIniFile = Path_IsNotEmpty(Paths.IniFile);
    bool const bHaveFallbackIniFile = Path_IsNotEmpty(Paths.IniFileDefault);
    bool const bCurrFileLocked = (FileWatching.FileWatchingMode == FWM_EXCLUSIVELOCK);

    bool const bCanSav = Globals.bCanSaveIniFile = !(bSoftLocked && bCurrFileLocked) && CanAccessPath(Paths.IniFile, GENERIC_WRITE);

    HMENU const hmenu = Globals.hMainMenu;

    CheckCmd(hmenu, IDM_SET_SAVESETTINGS, Settings.SaveSettings && !bSoftLocked);

    EnableCmd(hmenu, IDM_SET_SAVESETTINGS, bCanSav && !bSoftLocked);
    EnableCmd(hmenu, IDM_SET_SAVESETTINGSNOW, (bHaveIniFile || bHaveFallbackIniFile) && !bSoftLocked);
    EnableCmd(hmenu, CMD_OPENINIFILE, bHaveIniFile && !bSoftLocked);

    EnableCmd(hmenu, IDM_SET_REUSEWINDOW, bCanSav);
    EnableCmd(hmenu, IDM_SET_SINGLEFILEINSTANCE, bCanSav);
    EnableCmd(hmenu, IDM_VIEW_STICKYWINPOS, bCanSav);
    EnableCmd(hmenu, CMD_SAVEASDEFWINPOS, bCanSav);
    EnableCmd(hmenu, IDM_SET_NOSAVERECENT, bCanSav);
    EnableCmd(hmenu, IDM_SET_NOPRESERVECARET, Settings.SaveRecentFiles && bCanSav);
    EnableCmd(hmenu, IDM_SET_NOSAVEFINDREPL, bCanSav);
    EnableCmd(hmenu, IDM_SET_AUTOLOAD_MRU_FILE, bCanSav);

    DrawMenuBar(Globals.hwndMain);
}


//=============================================================================
//
//  UndoRedoRecordingStart()
//
static void _UndoRedoRecordingStart()
{
    _UndoRedoActionMap(URTok_NoTransaction, NULL); // clear
    SciCall_SetUndoCollection(true);
    SciCall_EmptyUndoBuffer();
    SciCall_SetSavePoint();
    SciCall_SetChangeHistory(Settings.ChangeHistoryMode);
    UpdateMargins(true);
}

//=============================================================================
//
//  UndoRedoRecordingStop()
//
static void _UndoRedoRecordingStop()
{
    _UndoRedoActionMap(URTok_NoTransaction, NULL); // clear
    while (SciCall_GetUndoSequence() > 0)
        SciCall_EndUndoAction();
    SciCall_EmptyUndoBuffer();
    SciCall_SetSavePoint();
    SciCall_SetChangeHistory(SC_CHANGE_HISTORY_DISABLED);
    SciCall_SetUndoCollection(false);
}

//=============================================================================
//
//  UndoRedoReset()
//
void UndoRedoReset()
{
    _UndoRedoRecordingStop();
    _UndoRedoRecordingStart();
}


//=============================================================================
//
//  _SaveUndoSelection()
//
static LONG _SaveUndoSelection()
{
    static DocPosU _s_iSelection = 0LL;           // index

    UndoRedoSelection_t sel = INIT_UNDOREDOSEL; // = InitUndoRedoSelection(&sel);
    CopyUndoRedoSelection(&sel, NULL);          // utarray_new()

    DocPosU const numOfSel = SciCall_GetSelections();

    // each single selection of a multi-selection will call thid method
    // we are only interested in the first call
    if (0LL == _s_iSelection) {
        _s_iSelection = numOfSel;
    }
    if ((numOfSel-1) != --_s_iSelection) {
        return URTok_NoTransaction;
    }

    int const selMode = ((numOfSel > 1) && !SciCall_IsSelectionRectangle()) ? NP3_SEL_MULTI : SciCall_GetSelectionMode();

    sel.selMode_undo = selMode;

    switch (selMode) {
    case NP3_SEL_MULTI: {
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
    case SC_SEL_THIN: {
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
    default: {
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
    LONG const token = _UndoRedoActionMap(URTok_NoTransaction, &pSel);

    DelUndoRedoSelection(&sel); // utarray_free()

    _s_iSelection = 0; // reset

    return token;
}


//=============================================================================
//
//  _SaveRedoSelection()
//
//
static void _SaveRedoSelection(const LONG token, const bool bAddAction)
{
    static DocPosU _s_iSelection = 0; // index

    if (token <= URTok_NoTransaction) {
        return;
    }

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
    if (bAddAction) {

        if ((_UndoRedoActionMap(token, &pSel) >= URTok_TokenStart) && (pSel != NULL)) {

            int const selMode = ((numOfSel > 1) && !SciCall_IsSelectionRectangle()) ? NP3_SEL_MULTI : SciCall_GetSelectionMode();

            pSel->selMode_redo = selMode;

            switch (selMode) {
            case NP3_SEL_MULTI: {
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
            } break;

            case SC_SEL_RECTANGLE:
            case SC_SEL_THIN: {
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
            } break;

            case SC_SEL_LINES:
            case SC_SEL_STREAM:
            default: {
                DocPos const anchorPos = SciCall_GetAnchor();
                utarray_push_back(pSel->anchorPos_redo, &anchorPos);
                DocPos const curPos = SciCall_GetCurrentPos();
                utarray_push_back(pSel->curPos_redo, &curPos);
                //~DocPos const dummy = (DocPos)-1;
                //~utarray_push_back(pSel->anchorVS_redo, &dummy);
                //~utarray_push_back(pSel->curVS_redo, &dummy);
            } break;
            }

            //~SciCall_AddUndoAction((int)token, UNDO_MAY_COALESCE);
            SciCall_AddUndoAction((int)token, UNDO_NONE);

        }
        else {
            _UndoRedoActionMap(token, NULL);  // remove
        }
    }
}


//=============================================================================
//
//  BeginUndoActionSelection()
//
LONG BeginUndoActionSelection()
{
    if (SciCall_GetUndoCollection()) {
        SciCall_BeginUndoAction();
        if (SciCall_GetUndoSequence() == 1) {
            DisableDocChangeNotification();
        }
        return SciCall_IsSelectionEmpty() ? URTok_NoTransaction : _SaveUndoSelection();
    }
    return URTok_NoRecording;
}


//=============================================================================
//
//  EndUndoActionSelection()
//
void EndUndoActionSelection(const LONG token)
{
    if (SciCall_GetUndoCollection()) {
        if (token >= URTok_TokenStart) {
            _SaveRedoSelection(token, SciCall_GetModify());
        }
        SciCall_EndUndoAction();
        if (SciCall_GetUndoSequence() == 0) {
            EnableDocChangeNotification(EVM_Default);
        }
    }
}

//=============================================================================
//
//  _RestoreActionSelection()
//
static void _RestoreActionSelection(const LONG token, DoAction doAct)
{
    if (SciCall_GetUndoSequence() > 0) {
        assert("Wrong Transaction!" && 0);
        return;
    }

    UndoRedoSelection_t* pSel = NULL;

    if ((_UndoRedoActionMap(token, &pSel) >= URTok_TokenStart) && (pSel != NULL)) {

        LimitNotifyEvents();

        DocPos* pPosAnchor = (DocPos*)((UNDO == doAct) ? utarray_front(pSel->anchorPos_undo) : utarray_front(pSel->anchorPos_redo));
        DocPos* pPosCur = (DocPos*)((UNDO == doAct) ? utarray_front(pSel->curPos_undo) : utarray_front(pSel->curPos_redo));
        DocPos* pPosAnchorVS = (DocPos*)((UNDO == doAct) ? utarray_front(pSel->anchorVS_undo) : utarray_front(pSel->anchorVS_redo));
        DocPos* pPosCurVS = (DocPos*)((UNDO == doAct) ? utarray_front(pSel->curVS_undo) : utarray_front(pSel->curVS_redo));

        if (pPosAnchor && pPosCur) {
            // Ensure that the first and last lines of a selection are always unfolded
            // This needs to be done _before_ the SCI_SETSEL message
            DocLn const anchorPosLine = SciCall_LineFromPosition((*pPosAnchor));
            DocLn const currPosLine = SciCall_LineFromPosition((*pPosCur));
            SciCall_EnsureVisible(anchorPosLine);
            if (anchorPosLine != currPosLine) {
                SciCall_EnsureVisible(currPosLine);
            }

            int const selectionMode = (UNDO == doAct) ? pSel->selMode_undo : pSel->selMode_redo;

            SciCall_SetSelectionMode((selectionMode == NP3_SEL_MULTI) ? SC_SEL_STREAM : selectionMode);

            switch (selectionMode) {
            case NP3_SEL_MULTI: {
                unsigned int i = 0;

                DocPosU const selCount = (UNDO == doAct) ? utarray_len(pSel->anchorPos_undo) : utarray_len(pSel->anchorPos_redo);
                DocPosU const selCountVS = (UNDO == doAct) ? utarray_len(pSel->anchorVS_undo) : utarray_len(pSel->anchorVS_redo);

                SciCall_SetSelection(*pPosCur, *pPosAnchor);
                if (pPosAnchorVS && pPosCurVS) {
                    SciCall_SetSelectionNAnchorVirtualSpace(0, *pPosAnchorVS);
                    SciCall_SetSelectionNCaretVirtualSpace(0, *pPosCurVS);
                }
                SciCall_Cancel(); // (!) else shift-key selection behavior is kept

                ++i;
                while (i < selCount) {
                    pPosAnchor = (DocPos*)((UNDO == doAct) ? utarray_eltptr(pSel->anchorPos_undo, i) : utarray_eltptr(pSel->anchorPos_redo, i));
                    pPosCur = (DocPos*)((UNDO == doAct) ? utarray_eltptr(pSel->curPos_undo, i) : utarray_eltptr(pSel->curPos_redo, i));
                    if (pPosAnchor && pPosCur) {
                        SciCall_AddSelection(*pPosCur, *pPosAnchor);
                        if (i < selCountVS) {
                            pPosAnchorVS = (DocPos*)((UNDO == doAct) ? utarray_eltptr(pSel->anchorVS_undo, i) : utarray_eltptr(pSel->anchorVS_redo, i));
                            pPosCurVS = (DocPos*)((UNDO == doAct) ? utarray_eltptr(pSel->curVS_undo, i) : utarray_eltptr(pSel->curVS_redo, i));
                            if (pPosAnchorVS && pPosCurVS) {
                                SciCall_SetSelectionNAnchorVirtualSpace(i, *pPosAnchorVS);
                                SciCall_SetSelectionNCaretVirtualSpace(i, *pPosCurVS);
                            }
                        }
                    }
                    ++i;
                }
                //~SciCall_SetMainSelection(0);
            }
            break;

            case SC_SEL_RECTANGLE:
            case SC_SEL_THIN:
                SciCall_SetRectangularSelectionAnchor(*pPosAnchor);
                SciCall_SetRectangularSelectionCaret(*pPosCur);
                if (pPosAnchorVS && pPosCurVS) {
                    SciCall_SetRectangularSelectionAnchorVirtualSpace(*pPosAnchorVS);
                    SciCall_SetRectangularSelectionCaretVirtualSpace(*pPosCurVS);
                }
                SciCall_Cancel(); // (!) else shift-key selection behavior is kept
                break;

            case SC_SEL_LINES:
            case SC_SEL_STREAM:
            default:
                if (pPosAnchor && pPosCur) {
                    SciCall_SetSelection(*pPosCur, *pPosAnchor);
                }
                SciCall_Cancel(); // (!) else shift-key selection behavior is kept
                break;
            }
        }
        if (pPosAnchor && pPosCur) {
            SciCall_ScrollRange(*pPosAnchor, *pPosCur);
        }
        SciCall_ChooseCaretX();

        RestoreNotifyEvents();
    }
    else {
        assert("Invalid Token to Restore!" && 0);
    }
}


#if 0
//=============================================================================
//
//  _RestoreActionSelection()
//
//
static void _RestoreActionSelection(const LONG token, DoAction doAct)
{
    if (SciCall_GetUndoSequence() > 0) {
        assert("Wrong Transaction!" && 0);
        return;
    }

    UndoRedoSelection_t* pSel = NULL;

    if ((_UndoRedoActionMap(token, &pSel) >= URTok_TokenStart) && (pSel != NULL)) {

        // we are inside undo/redo transaction, so do delayed PostMessage() instead of SendMessage()

        DocPos* pPosAnchor = (DocPos*)((UNDO == doAct) ? utarray_front(pSel->anchorPos_undo) : utarray_front(pSel->anchorPos_redo));
        DocPos* pPosCur = (DocPos*)((UNDO == doAct) ? utarray_front(pSel->curPos_undo) : utarray_front(pSel->curPos_redo));
        DocPos* pPosAnchorVS = (DocPos*)((UNDO == doAct) ? utarray_front(pSel->anchorVS_undo) : utarray_front(pSel->anchorVS_redo));
        DocPos* pPosCurVS = (DocPos*)((UNDO == doAct) ? utarray_front(pSel->curVS_undo) : utarray_front(pSel->curVS_redo));

        if (pPosAnchor && pPosCur) {
            // Ensure that the first and last lines of a selection are always unfolded
            // This needs to be done _before_ the SCI_SETSEL message
            DocLn const anchorPosLine = SciCall_LineFromPosition((*pPosAnchor));
            DocLn const currPosLine = SciCall_LineFromPosition((*pPosCur));
            SciCall_PostMsg(SCI_ENSUREVISIBLE, anchorPosLine, 0);
            if (anchorPosLine != currPosLine) {
                SciCall_PostMsg(SCI_ENSUREVISIBLE, currPosLine, 0);
            }

            int const selectionMode = (UNDO == doAct) ? pSel->selMode_undo : pSel->selMode_redo;

            SciCall_PostMsg(SCI_SETSELECTIONMODE, (WPARAM)((selectionMode == NP3_SEL_MULTI) ? SC_SEL_STREAM : selectionMode), 0);

            switch (selectionMode) {
            case NP3_SEL_MULTI: {
                unsigned int i = 0;

                DocPosU const selCount = (UNDO == doAct) ? utarray_len(pSel->anchorPos_undo) : utarray_len(pSel->anchorPos_redo);
                DocPosU const selCountVS = (UNDO == doAct) ? utarray_len(pSel->anchorVS_undo) : utarray_len(pSel->anchorVS_redo);

                SciCall_PostMsg(SCI_SETSELECTION, (WPARAM)(*pPosCur), (LPARAM)(*pPosAnchor));
                if (pPosAnchorVS && pPosCurVS) {
                    SciCall_PostMsg(SCI_SETSELECTIONNANCHORVIRTUALSPACE, (WPARAM)0, (LPARAM)(*pPosAnchorVS));
                    SciCall_PostMsg(SCI_SETSELECTIONNCARETVIRTUALSPACE, (WPARAM)0, (LPARAM)(*pPosCurVS));
                }
                SciCall_PostMsg(SCI_CANCEL, 0, 0); // (!) else shift-key selection behavior is kept

                ++i;
                while (i < selCount) {
                    pPosAnchor = (DocPos*)((UNDO == doAct) ? utarray_eltptr(pSel->anchorPos_undo, i) : utarray_eltptr(pSel->anchorPos_redo, i));
                    pPosCur = (DocPos*)((UNDO == doAct) ? utarray_eltptr(pSel->curPos_undo, i) : utarray_eltptr(pSel->curPos_redo, i));
                    if (pPosAnchor && pPosCur) {
                        SciCall_PostMsg(SCI_ADDSELECTION, (WPARAM)(*pPosCur), (LPARAM)(*pPosAnchor));
                        if (i < selCountVS) {
                            pPosAnchorVS = (DocPos*)((UNDO == doAct) ? utarray_eltptr(pSel->anchorVS_undo, i) : utarray_eltptr(pSel->anchorVS_redo, i));
                            pPosCurVS = (DocPos*)((UNDO == doAct) ? utarray_eltptr(pSel->curVS_undo, i) : utarray_eltptr(pSel->curVS_redo, i));
                            if (pPosAnchorVS && pPosCurVS) {
                                SciCall_PostMsg(SCI_SETSELECTIONNANCHORVIRTUALSPACE, (WPARAM)i, (LPARAM)(*pPosAnchorVS));
                                SciCall_PostMsg(SCI_SETSELECTIONNCARETVIRTUALSPACE, (WPARAM)i, (LPARAM)(*pPosCurVS));
                            }
                        }
                    }
                    ++i;
                }
                //~SciCall_PostMsg(SCI_SETMAINSELECTION, (WPARAM)0, (LPARAM)0);
            }
            break;

            case SC_SEL_RECTANGLE:
            case SC_SEL_THIN:
                SciCall_PostMsg(SCI_SETRECTANGULARSELECTIONANCHOR, (WPARAM)(*pPosAnchor), 0);
                SciCall_PostMsg(SCI_SETRECTANGULARSELECTIONCARET, (WPARAM)(*pPosCur), 0);
                if (pPosAnchorVS && pPosCurVS) {
                    SciCall_PostMsg(SCI_SETRECTANGULARSELECTIONANCHORVIRTUALSPACE, (WPARAM)(*pPosAnchorVS), 0);
                    SciCall_PostMsg(SCI_SETRECTANGULARSELECTIONCARETVIRTUALSPACE, (WPARAM)(*pPosCurVS), 0);
                }
                SciCall_PostMsg(SCI_CANCEL, 0, 0); // (!) else shift-key selection behavior is kept
                break;

            case SC_SEL_LINES:
            case SC_SEL_STREAM:
            default:
                if (pPosAnchor && pPosCur) {
                    SciCall_PostMsg(SCI_SETSELECTION, (WPARAM)(*pPosCur), (LPARAM)(*pPosAnchor));
                }
                SciCall_PostMsg(SCI_CANCEL, 0, 0); // (!) else shift-key selection behavior is kept
                break;
            }
        }
        if (pPosAnchor && pPosCur) {
            SciCall_PostMsg(SCI_SCROLLRANGE, (WPARAM)(*pPosAnchor), (LPARAM)(*pPosCur));
        }
        SciCall_PostMsg(SCI_CHOOSECARETX, 0, 0);
    }
    else {
        assert("Invalid Token to Restore!" && 0);
    }
}
#endif


//=============================================================================
//
//  _UndoRedoActionMap()
//
//
static LONG _UndoRedoActionMap(const LONG token, const UndoRedoSelection_t** selection)
{
    static ULONG uiTokenCnt = URTok_TokenStart; 

    if (UndoRedoSelectionUTArray == NULL) {
        return URTok_NoRecording;
    };

    if (selection == NULL) {

        if (token <= URTok_NoTransaction) { // reset / clear
            if (SciCall_GetUndoCollection()) {
                while (SciCall_GetUndoSequence() > 0) {
                    SciCall_EndUndoAction();
                }
            }

            utarray_clear(UndoRedoSelectionUTArray);
            //~utarray_free(UndoRedoSelectionUTArray);
            //~utarray_init(UndoRedoSelectionUTArray, &UndoRedoSelection_icd);
            uiTokenCnt = URTok_TokenStart;
        }
        else { // remove token indexed action
            
            utarray_erase(UndoRedoSelectionUTArray, (ULONG)token, 1);
            if (((ULONG)token + 1UL) == uiTokenCnt) {
                --uiTokenCnt;
            }
            else {
                assert("Invalid index of item to remove!" && 0);
            }
        }
        return URTok_NoTransaction;
    }

    // indexing is unsigned
    ULONG utoken = (token >= URTok_TokenStart) ? (ULONG)token : 0UL;

    if (!SciCall_GetUndoCollection()) {
        assert("Inactive Undo Collection!" && 0);
        return URTok_NoRecording;
    }

    // get or set map item request ?
    if ((token >= URTok_TokenStart) && (utoken < uiTokenCnt)) {
        if ((*selection) == NULL) {
            // this is a get request
            (*selection) = (UndoRedoSelection_t*)utarray_eltptr(UndoRedoSelectionUTArray, utoken);
        } else {
            // this is a set request (fill redo pos)
            assert("Invalid set request (fill redo pos)!" && 0); // not used yet
            //~utarray_insert(UndoRedoSelectionUTArray, (void*)(*selection), utoken);
        }
        // don't clear map item here (token used in redo/undo again)
    }
    else if (token <= URTok_NoTransaction) {
        // set map new item request
        LONG const newToken = (LONG)uiTokenCnt;
        utarray_insert(UndoRedoSelectionUTArray, (void*)(*selection), uiTokenCnt);
        uiTokenCnt = (uiTokenCnt < (ULONG)LONG_MAX) ? (uiTokenCnt + 1UL) : 0UL; // round robin next
        return newToken;
    }
    else {
        assert("Invalid Token to Set/Get!" && 0);
    }
    return token;
}


//=============================================================================
//
//  FileIO()
//
//
bool FileIO(bool fLoad, const HPATHL hfile_pth, EditFileIOStatus* status,
    FileLoadFlags fLoadFlags, FileSaveFlags fSaveFlags, bool bSetSavePoint)
{
    bool bSuccess = false;

    if (fLoad) {
        bSuccess = EditLoadFile(Globals.hwndEdit, hfile_pth, status, fLoadFlags, bSetSavePoint);
        SciCall_SetReadOnly(Settings.DocReadOnlyMode || FileWatching.MonitoringLog);
    }
    else {
        int idx;
        if (MRU_FindPath(Globals.pFileMRU, hfile_pth, &idx)) {
            Globals.pFileMRU->iEncoding[idx] = status->iEncoding;
            Globals.pFileMRU->iCaretPos[idx] = (Settings.PreserveCaretPos ? SciCall_GetCurrentPos() : -1);
            Globals.pFileMRU->iSelAnchPos[idx] = (Settings.PreserveCaretPos ? SciCall_GetAnchor() : -1);

            WCHAR wchBookMarks[MRU_BMRK_SIZE] = { L'\0' };
            EditGetBookmarkList(Globals.hwndEdit, wchBookMarks, COUNTOF(wchBookMarks));
            if (Globals.pFileMRU->pszBookMarks[idx]) {
                LocalFree(Globals.pFileMRU->pszBookMarks[idx]);  // StrDup()
                Globals.pFileMRU->pszBookMarks[idx] = NULL;
            }
            Globals.pFileMRU->pszBookMarks[idx] = StrDup(wchBookMarks);
        }
        bSuccess = EditSaveFile(Globals.hwndEdit, hfile_pth, status, fSaveFlags, Flags.bPreserveFileModTime);
    }
    return bSuccess;
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

    if (hasTabOrSpaceIndent || hasMixedIndents /*|| hasIrregularIndentDepth */) {

        if (WarnIndentationDlg(Globals.hwndMain, status)) {

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

            Sci_GotoPosChooseCaret(0);

        } else {
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

static inline bool IsFileVarLogFile()
{
    if (SciCall_GetTextLength() >= 4) {
        char tch[5] = { '\0', '\0', '\0', '\0', '\0' };
        SciCall_GetText(COUNTOF(tch) - 1, tch);
        return (StrCmpA(tch, ".LOG") == 0); 
    }
    return false;
}

static inline void _ResetFileWatchingMode() {
    FileWatching.FileWatchingMode = (s_flagChangeNotify != FWM_NO_INIT) ? s_flagChangeNotify : Settings.FileWatchingMode;
    if (FileWatching.MonitoringLog) {
        FileWatching.FileWatchingMode = FWM_AUTORELOAD;
        PostWMCommand(Globals.hwndMain, IDM_VIEW_CHASING_DOCTAIL);
    }
    ResetFileObservationData(true);
}

bool FileLoad(const HPATHL hfile_pth, const FileLoadFlags fLoadFlags, const DocPos curPos, const DocLn visLn)
{
    bool fSuccess = false;
    bool const bReloadFile = (fLoadFlags & FLF_Reload);

    EditFileIOStatus fioStatus = INIT_FILEIO_STATUS;
    fioStatus.iEOLMode = Settings.DefaultEOLMode;
    fioStatus.iEncoding = Settings.DefaultEncoding;

    if (!(fLoadFlags & FLF_DontSave)) {
        if (!FileSave(FSF_Ask)) {
            return false;
        }
    }

    if (!bReloadFile) {
        ResetEncryption();
    }

    if (fLoadFlags & FLF_New) {

        if (FocusedView.HideNonMatchedLines) {
            EditToggleView(Globals.hwndEdit);
        }

        if (!s_IsThisAnElevatedRelaunch) {
            Flags.bPreserveFileModTime = DefaultFlags.bPreserveFileModTime;
        }

        Path_Empty(Paths.CurrentFile, false);
        _SetEnumWindowsItems(Globals.hwndMain);
        if (!s_flagKeepTitleExcerpt) {
            StringCchCopy(s_wchTitleExcerpt, COUNTOF(s_wchTitleExcerpt), L"");
        }
        FileVars_GetFromData(NULL, 0, &Globals.fvCurFile); // init-reset

        EditSetNewText(Globals.hwndEdit, "", 0, false, false);

        SciCall_SetEOLMode(Settings.DefaultEOLMode);
        Encoding_Current(Settings.DefaultEncoding);

        Style_SetDefaultLexer(Globals.hwndEdit);

        SetSaveDone();

        // Restart file watching
        AutoSaveStop();
        InstallFileWatching(false); // terminate old
        if (Settings.ResetFileWatching) {
            _ResetFileWatchingMode();
        }
        InstallFileWatching(true);

        Flags.bSettingsFileSoftLocked = false;
        UpdateSaveSettingsCmds();

        UndoRedoReset();

        UpdateToolbar();
        UpdateMargins(true);
        if (SciCall_GetZoom() != 100) {
            ShowZoomCallTip();
        }

        return true;
    }

    HPATHL hopen_file = Path_Copy(hfile_pth);

    if (Path_IsEmpty(hopen_file)) {
        if (!OpenFileDlg(Globals.hwndMain, hopen_file, hopen_file)) {
            Path_Release(hopen_file);
            return false;
        }
    }
    else {
        Path_Reset(hopen_file, Path_Get(hfile_pth));
    }

    Path_NormalizeEx(hopen_file, Paths.WorkingDirectory, true, Flags.bSearchPathIfRelative);

    if (!bReloadFile && Path_StrgComparePath(hopen_file, Paths.CurrentFile, Paths.WorkingDirectory, false) == 0) {
        Path_Release(hopen_file);
        return false;
    }
    if (!bReloadFile && Flags.bSingleFileInstance) {

        HWND hwnd = NULL;
        if (FindOtherInstance(&hwnd, hopen_file)) {
            if (!s_bInitAppDone || IsYesOkay(InfoBoxLng(MB_YESNO | MB_ICONQUESTION, L"InfoInstanceExist", IDS_MUI_ASK_INSTANCE_EXISTS))) {
                if (IsIconic(hwnd)) {
                    ShowWindowAsync(hwnd, SW_RESTORE);
                }
                if (!IsWindowVisible(hwnd)) {
                    SendMessage(hwnd, WM_TRAYMESSAGE, 0, WM_LBUTTONDBLCLK);
                    SendMessage(hwnd, WM_TRAYMESSAGE, 0, WM_LBUTTONUP);
                }
                LockSetForegroundWindow(LSFW_UNLOCK);
                SetForegroundWindow(hwnd);
            }
            Path_Release(hopen_file);
            return false;
        }
    }

    // Ask to create a new file...
    if (!bReloadFile && !Path_IsExistingFile(hopen_file)) {
        bool bCreateFile = s_flagQuietCreate;
        if (!bCreateFile) {

            WCHAR szDisplayName[MAX_PATH_EXPLICIT >> 1] = { L'\0' };
            GetLngString(IDS_MUI_UNTITLED, szDisplayName, COUNTOF(szDisplayName));
            Path_GetDisplayName(szDisplayName, COUNTOF(szDisplayName), hopen_file, NULL, false); //~Path_FindFileName(hopen_file)

            if (IsYesOkay(InfoBoxLng(MB_YESNO | MB_ICONQUESTION, NULL, IDS_MUI_ASK_CREATE, szDisplayName))) {
                Path_CanonicalizeEx(hopen_file, Paths.WorkingDirectory);
                bCreateFile = true;
            }
        }
        if (bCreateFile) {
            HANDLE hFile = CreateFileW(Path_Get(hopen_file),
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
            Globals.dwLastError = GetLastError();
            fSuccess = IS_VALID_HANDLE(hFile);
            if (fSuccess) {
                FileVars_GetFromData(NULL, 0, &Globals.fvCurFile); // init/reset
                EditSetNewText(Globals.hwndEdit, "", 0, false, false);
                SciCall_SetEOLMode(Settings.DefaultEOLMode);
                Style_SetDefaultLexer(Globals.hwndEdit);
                if (Encoding_IsValid(Encoding_Forced(CPI_GET))) {
                    fioStatus.iEncoding = Encoding_Forced(CPI_GET);
                    Encoding_Current(fioStatus.iEncoding);
                }
                else {
                    fioStatus.iEncoding = Globals.fvCurFile.iEncoding;
                    Encoding_Current(Globals.fvCurFile.iEncoding);
                }
            }
            if (IS_VALID_HANDLE(hFile)) {
                CloseHandle(hFile);
            }
        }
        else {
            Path_Release(hopen_file);
            return false;
        }
    }
    else {
        int idx;
        if (!bReloadFile && MRU_FindPath(Globals.pFileMRU, hopen_file, &idx)) {
            fioStatus.iEncoding = Globals.pFileMRU->iEncoding[idx];
            if (Encoding_IsValid(fioStatus.iEncoding)) {
                Encoding_SrcWeak(fioStatus.iEncoding);
            }
        }
        else {
            fioStatus.iEncoding = Encoding_GetCurrent();
        }

        fSuccess = FileIO(true, hopen_file, &fioStatus, fLoadFlags, FSF_None, !(bReloadFile || s_IsThisAnElevatedRelaunch));
    }

    bool bUnknownLexer = s_flagLexerSpecified;

    if (fSuccess) {

        // keep change-history on reload (!)
        if (!bReloadFile) {
            UndoRedoReset();
        }

        if (!s_IsThisAnElevatedRelaunch) {
            Flags.bPreserveFileModTime = DefaultFlags.bPreserveFileModTime;
        }

        //~Path_Swap(Paths.CurrentFile, hopen_file); ~ hopen_file needed later
        Path_Reset(Paths.CurrentFile, Path_Get(hopen_file)); // dup

        _SetEnumWindowsItems(Globals.hwndMain);

        if (!s_flagKeepTitleExcerpt) {
            StringCchCopy(s_wchTitleExcerpt, COUNTOF(s_wchTitleExcerpt), L"");
        }

        if (!s_flagLexerSpecified) { // flagLexerSpecified will be cleared
            bUnknownLexer = !Style_SetLexerFromFile(Globals.hwndEdit, Paths.CurrentFile);
        }
        SciCall_SetEOLMode(fioStatus.iEOLMode);
        Encoding_Current(fioStatus.iEncoding); // load may change encoding

        int     idx = 0;
        DocPos  iCaretPos = -1;
        DocPos  iAnchorPos = -1;
        LPCWSTR pszBookMarks = L"";
        if (!bReloadFile && MRU_FindPath(Globals.pFileMRU, Paths.CurrentFile, &idx)) {
            iCaretPos = Globals.pFileMRU->iCaretPos[idx];
            iAnchorPos = Globals.pFileMRU->iSelAnchPos[idx];
            pszBookMarks = Globals.pFileMRU->pszBookMarks[idx];
        }
        if (!(Flags.bDoRelaunchElevated || s_IsThisAnElevatedRelaunch)) {
            MRU_AddPath(Globals.pFileMRU, Paths.CurrentFile, Flags.RelativeFileMRU, Flags.PortableMyDocs, fioStatus.iEncoding, iCaretPos, iAnchorPos, pszBookMarks);
            AddFilePathToRecentDocs(Paths.CurrentFile);
        }

        EditSetBookmarkList(Globals.hwndEdit, pszBookMarks);
        if (IsFindPatternEmpty()) {
            SetFindPattern((Globals.pMRUfind ? Globals.pMRUfind->pszItems[0] : L""));
        }

        // Install watching of the current file
        AutoSaveStop();
        InstallFileWatching(false); // terminate previous
        if (!bReloadFile && Settings.ResetFileWatching) {
            _ResetFileWatchingMode();
        }

        // consistent settings file handling (if loaded in editor)
        Flags.bSettingsFileSoftLocked = (Path_StrgComparePath(Paths.CurrentFile, Paths.IniFile, Paths.WorkingDirectory, true) == 0);

        // the .LOG feature ...
        if (IsFileVarLogFile()) {
            Sci_SetCaretScrollDocEnd();
            UndoTransActionBegin();
            SciCall_NewLine();
            SendWMCommand(Globals.hwndMain, IDM_EDIT_INSERT_SHORTDATE);
            SciCall_DocumentEnd();
            SciCall_NewLine();
            EndUndoTransAction();
        }

        if (!bReloadFile) {
            UpdateSaveSettingsCmds();
        }

        // set historic caret/selection  pos
        if (!FileWatching.MonitoringLog && (s_flagChangeNotify != FWM_AUTORELOAD)) {
            if ((iCaretPos >= 0) && (iAnchorPos >= 0)) {
                Sci_SetStreamSelection(iAnchorPos, iCaretPos, true);
                Sci_ScrollSelectionToView();
            }
            else {
                Sci_GotoPosChooseCaret(0);
            }
        }

        // Show warning: Unicode file loaded as ANSI
        if (fioStatus.bUnicodeErr) {
            InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_ERR_UNICODE);
        }

        // Show inconsistent line endings warning
        Globals.bDocHasInconsistentEOLs = fioStatus.bInconsistentEOLs;

        bool const bCheckFile = !Globals.CmdLnFlag_PrintFileAndLeave && !fioStatus.bEncryptedRaw && !(fioStatus.bUnknownExt && bUnknownLexer) && !bReloadFile;
        //&& (fioStatus.iEncoding == Settings.DefaultEncoding) ???

        bool const bCheckEOL = bCheckFile && Globals.bDocHasInconsistentEOLs && Settings.WarnInconsistEOLs;

        if (bCheckEOL && !Style_MaybeBinaryFile(Globals.hwndEdit, Paths.CurrentFile)) {
            if (WarnLineEndingDlg(Globals.hwndMain, &fioStatus)) {
                UndoTransActionBegin();
                SciCall_ConvertEOLs(fioStatus.iEOLMode);
                EndUndoTransAction();
                Globals.bDocHasInconsistentEOLs = false;
            }
            SciCall_SetEOLMode(fioStatus.iEOLMode);
        }

        // Show inconsistent indentation
        fioStatus.iGlobalIndent = I_MIX_LN; // init

        bool const bCheckIndent = bCheckFile && !Flags.bHugeFileLoadState && Settings.WarnInconsistentIndents;

        if (bCheckIndent && !Style_MaybeBinaryFile(Globals.hwndEdit, Paths.CurrentFile)) {
            EditIndentationStatistic(Globals.hwndEdit, &fioStatus);
            ConsistentIndentationCheck(&fioStatus);
        }

        if (Settings.AutoDetectIndentSettings && !Globals.CmdLnFlag_PrintFileAndLeave) {
            if (!Settings.WarnInconsistentIndents || (fioStatus.iGlobalIndent != I_MIX_LN)) {
                EditIndentationStatistic(Globals.hwndMain, &fioStatus); // new statistic needed
            }
            Globals.fvCurFile.bTabsAsSpaces = (fioStatus.indentCount[I_TAB_LN] < fioStatus.indentCount[I_SPC_LN]) ? true : false;
            SciCall_SetUseTabs(!Globals.fvCurFile.bTabsAsSpaces);
        }

    }
    else if (!(Flags.bHugeFileLoadState || fioStatus.bUnknownExt)) {
        WCHAR displayName[80];
        Path_GetDisplayName(displayName, 80, hopen_file, L"<unknown>", false);
        InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_ERR_LOADFILE, displayName);
        Flags.bHugeFileLoadState = false; // reset
    }


    Path_Release(hopen_file);

    ResetFileObservationData(true);
    InstallFileWatching(fSuccess);

    if (curPos > 0) {
        SciCall_GotoPos(curPos);
    }
    if (visLn > 0) {
        SciCall_SetFirstVisibleLine(visLn);
    }

    UpdateMargins(true);
    if (SciCall_GetZoom() != 100) {
        ShowZoomCallTip();
    }
    UpdateToolbar_Now(Globals.hwndMain);

    return fSuccess;
}



//=============================================================================
//
//  FileRevert()
//
bool FileRevert(const HPATHL hfile_pth, bool bIgnoreCmdLnEnc)
{
    if (Path_IsEmpty(hfile_pth)) {
        return false;
    }

    DocPos const curPos = SciCall_GetCurrentPos();
    DocLn const  firstVisibleLine = SciCall_GetFirstVisibleLine();
    DocLn const  curLineNum = Sci_GetCurrentLineNumber();
    DocPos const curColumnNum = Sci_GetCurrentColumnNumber();

    bool const bIsAtDocEnd = (curLineNum >= (Sci_GetLastDocLineNumber() - Settings2.CurrentLineVerticalSlop));

    Encoding_SrcWeak(CPI_NONE);
    if (bIgnoreCmdLnEnc) {
        Encoding_Forced(CPI_NONE);  // ignore history too
    } else {
        Encoding_SrcWeak(Encoding_GetCurrent());
    }

    //~InstallFileWatching(false); // terminate
    FileLoadFlags fLoadFlags = FLF_DontSave | FLF_Reload;
    fLoadFlags |= Settings.SkipUnicodeDetection ? FLF_SkipUnicodeDetect : 0;
    fLoadFlags |= Settings.SkipANSICodePageDetection ? FLF_SkipANSICPDetection : 0;
    bool const result = FileLoad(hfile_pth, fLoadFlags, curPos, firstVisibleLine);
    //~InstallFileWatching(true);

    if (result) {
        bool bPreserveView = !IsFileVarLogFile();
        if (FileWatching.FileWatchingMode == FWM_AUTORELOAD) {
            if (bIsAtDocEnd || FileWatching.MonitoringLog || (s_flagChangeNotify == FWM_AUTORELOAD)) {
                bPreserveView = false;
            }
        }
        if (bPreserveView) {
            Sci_GotoPosChooseCaret(SciCall_FindColumn(curLineNum, curColumnNum));
        }
        else { // watch document end
            Sci_SetCaretScrollDocEnd();
        }
    }

    SetSaveDone();
    UpdateToolbar();
    UpdateMargins(true);

    return result;
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
    LPWSTR const lpExe = AllocMem(sizeof(WCHAR) * wlen, HEAP_ZERO_MEMORY);
    LPWSTR const lpArgs = AllocMem(sizeof(WCHAR) * wlen, HEAP_ZERO_MEMORY);

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
    if (IsAppClosing()) {
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
    WININFO const wi = GetMyWindowPlacement(Globals.hwndMain, NULL, 0, false);

    HSTRINGW hstr_args = StrgCreate(NULL);
    StrgFormat(hstr_args, L"%s/pos " WINDOWPOS_STRGFORMAT L" /g %i,%i %s",
               wchFlags, wi.x, wi.y, wi.cx, wi.cy, wi.dpi, (int)wi.max, iCurLn, iCurCol, lpArgs);

    WCHAR wchTempFileName[MAX_PATH_EXPLICIT + 1] = { L'\0' };
    WCHAR wchTempPathBuffer[MAX_PATH_EXPLICIT + 1] = { L'\0' };
    // MAX_PATH_EXPLICIT is okay for GetTempPathW() and GetTempFileNameW()
    if (GetTempPathW(MAX_PATH_EXPLICIT, wchTempPathBuffer) && GetTempFileNameW(wchTempPathBuffer, L"NP3", 0, wchTempFileName)) {

        HPATHL htmp_pth = Path_Allocate(wchTempFileName);
        // replace possible unknown extension
        Path_RenameExtension(htmp_pth, Path_IsNotEmpty(Paths.CurrentFile) ? Path_FindExtension(Paths.CurrentFile) : L".txt");

        if (pFioStatus && FileIO(false, htmp_pth, pFioStatus, FLF_None, FSF_SaveCopy, true)) {
            // preserve encoding
            WCHAR wchEncoding[80] = { L'\0' };
            Encoding_GetNameW(Encoding_GetCurrent(), wchEncoding, COUNTOF(wchEncoding));

            StrgFormat(hstr_args, L"%s/%s /pos " WINDOWPOS_STRGFORMAT L" /g %i,%i /%s\"%s\" %s",
                       wchFlags, wchEncoding, wi.x, wi.y, wi.cx, wi.cy, wi.dpi, (int)wi.max, iCurLn, iCurCol, RELAUNCH_ELEVATED_BUF_ARG, Path_Get(htmp_pth), lpArgs);

            if (!StrStrI(StrgGet(hstr_args), Path_FindFileName(Paths.CurrentFile))) {
                if (Path_IsNotEmpty(Paths.CurrentFile)) {
                    StrgFormat(hstr_args, L"%s \"%s\"", StrgGet(hstr_args), Path_Get(Paths.CurrentFile));
                }
            }
        }

        Path_Release(htmp_pth);
        FreeMem(lpExe);
        FreeMem(lpArgs);
    }

    if (RelaunchElevated(StrgGet(hstr_args))) {
        // set no change and quit
        SetSaveDone();
    } else {
        Globals.dwLastError = GetLastError();
        if (PathIsExistingFile(wchTempFileName)) {
            DeleteFileW(wchTempFileName);
        }
        Flags.bDoRelaunchElevated = false;
    }
   
    StrgDestroy(hstr_args);

    return Flags.bDoRelaunchElevated;
}


//=============================================================================

static void _MRU_UpdateSession()
{
    int idx = 0;
    if (MRU_FindPath(Globals.pFileMRU, Paths.CurrentFile, &idx)) {
        Globals.pFileMRU->iEncoding[idx] = Encoding_GetCurrent();
        Globals.pFileMRU->iCaretPos[idx] = (Settings.PreserveCaretPos) ? SciCall_GetCurrentPos() : -1;
        Globals.pFileMRU->iSelAnchPos[idx] = (Settings.PreserveCaretPos) ? (Sci_IsMultiOrRectangleSelection() ? -1 : SciCall_GetAnchor()) : -1;
        WCHAR wchBookMarks[MRU_BMRK_SIZE] = { L'\0' };
        EditGetBookmarkList(Globals.hwndEdit, wchBookMarks, COUNTOF(wchBookMarks));
        if (Globals.pFileMRU->pszBookMarks[idx]) {
            LocalFree(Globals.pFileMRU->pszBookMarks[idx]); // StrDup()
            Globals.pFileMRU->pszBookMarks[idx] = NULL;
        }
        Globals.pFileMRU->pszBookMarks[idx] = StrDup(wchBookMarks);
    }
}

static void _MRU_AddSession()
{
    cpi_enc_t    iCurrEnc = Encoding_GetCurrent();
    const DocPos iCaretPos = SciCall_GetCurrentPos();
    const DocPos iAnchorPos = Sci_IsMultiOrRectangleSelection() ? -1 : SciCall_GetAnchor();
    WCHAR        wchBookMarks[MRU_BMRK_SIZE] = { L'\0' };
    EditGetBookmarkList(Globals.hwndEdit, wchBookMarks, COUNTOF(wchBookMarks));
    MRU_AddPath(Globals.pFileMRU, Paths.CurrentFile, Flags.RelativeFileMRU, Flags.PortableMyDocs, iCurrEnc, iCaretPos, iAnchorPos, wchBookMarks);
}

// ----------------------------------------------------------------------------
//
//  FileSave()
//
//
bool FileSave(FileSaveFlags fSaveFlags)
{
    bool fSuccess = false;

    EditFileIOStatus fioStatus = INIT_FILEIO_STATUS;
    fioStatus.iEncoding        = Encoding_GetCurrent();
    fioStatus.iEOLMode         = SciCall_GetEOLMode();

    bool bIsEmptyNewFile = false;
    if (Flags.SaveBlankNewFile) {
        bIsEmptyNewFile = (Path_IsEmpty(Paths.CurrentFile) && (SciCall_GetTextLength() <= 0LL));
    }
    else {
        if (Path_IsEmpty(Paths.CurrentFile)) {
            DocPos const cchText = SciCall_GetTextLength();
            if (cchText <= 0) {
                bIsEmptyNewFile = true;
            }
            else {
                struct Sci_TextToFindFull ft = { { 0, 0 }, "\\S+", { 0, 0 } };
                ft.chrg.cpMax = Sci_GetDocEndPosition();
                DocPos const iPos = SciCall_FindTextFull(SCFIND_REGEXP, &ft);
                bIsEmptyNewFile = ((iPos < 0) || (iPos >= ft.chrg.cpMax));
            }
        }
    }

    bool const bSaveNeeded = (IsSaveNeeded() || IsFileChangedFlagSet()) && !bIsEmptyNewFile;

    if (!(fSaveFlags & FSF_SaveAs) && !(fSaveFlags & FSF_SaveAlways) && !bSaveNeeded) {
        _MRU_UpdateSession();
        AutoSaveStop();
        ResetFileObservationData(true);
        return true;
    }

    if (fSaveFlags & FSF_Ask) {
        // File or "Untitled" ...
        WCHAR wchFileName[MAX_PATH_EXPLICIT>>1] = { L'\0' };

        GetLngString(IDS_MUI_UNTITLED, wchFileName, COUNTOF(wchFileName));
        Path_GetDisplayName(wchFileName, COUNTOF(wchFileName), Paths.CurrentFile, NULL, false);

        INT_PTR const answer = (Settings.MuteMessageBeep) ?
                               InfoBoxLng(MB_YESNOCANCEL | MB_ICONWARNING, NULL, IDS_MUI_ASK_SAVE, wchFileName) :
                               MessageBoxLng(MB_YESNOCANCEL | MB_ICONWARNING, IDS_MUI_ASK_SAVE, wchFileName);
        switch (answer)
        {
        case IDCANCEL:
            return false;
        case IDNO:
            _MRU_UpdateSession();
            AutoSaveStop();
            return true;
        default:
            // proceed
            break;
        }
    }

    // Read only...
    if (!(fSaveFlags & FSF_SaveAs) && !(fSaveFlags & FSF_SaveCopy) && Path_IsNotEmpty(Paths.CurrentFile)) {
        if (IsFileReadOnly()) {
            UpdateToolbar();
            INT_PTR const answer = (Settings.MuteMessageBeep) ?
                                   InfoBoxLng(MB_YESNO | MB_ICONWARNING, NULL, IDS_MUI_READONLY_SAVE, Path_FindFileName(Paths.CurrentFile)) :
                                   MessageBoxLng(MB_YESNO | MB_ICONWARNING, IDS_MUI_READONLY_SAVE, Path_Get(Paths.CurrentFile));
            if (IsYesOkay(answer)) {
                fSaveFlags |= FSF_SaveAs;
            } else {
                return false;
            }
        }
    }

    // Save As...
    if ((fSaveFlags & FSF_SaveAs) || (fSaveFlags & FSF_SaveCopy) || Path_IsEmpty(Paths.CurrentFile)) {

        static HPATHL _hpthLastSaveCopyDir = NULL; // session remember copyTo dir
        if (!_hpthLastSaveCopyDir) {
            _hpthLastSaveCopyDir = Path_Allocate(L"");
        }

        HPATHL hfile_pth = Path_Allocate(NULL);
        HPATHL hdir_pth = Path_Allocate(NULL);

        if ((fSaveFlags & FSF_SaveCopy) && Path_IsNotEmpty(_hpthLastSaveCopyDir)) {
            Path_Reset(hdir_pth, Path_Get(_hpthLastSaveCopyDir));
        } else {
            Path_Reset(hdir_pth, Path_Get(Paths.CurrentFile));
            Path_RemoveFileSpec(hdir_pth);
        }

        Path_Reset(hfile_pth, Path_FindFileName(Paths.CurrentFile));

        bool const ok = SaveFileDlg(Globals.hwndMain, hfile_pth, Path_IsNotEmpty(hdir_pth) ? hdir_pth : NULL);
        if (ok) {
            if (!(fSaveFlags & FSF_SaveCopy)) {
                if (fSaveFlags & FSF_SaveAs) {
                    SaveAllSettings(false); // session on old file ends, save side-by-side settings
                }
                InstallFileWatching(false); // terminate
                fSaveFlags |= FSF_EndSession;
            }
            fSuccess = FileIO(false, hfile_pth, &fioStatus, FLF_None, fSaveFlags, true);
            
            if (fSuccess) {
                if (!(fSaveFlags & FSF_SaveCopy)) {
                    Path_Swap(Paths.CurrentFile, hfile_pth);
                    _SetEnumWindowsItems(Globals.hwndMain);
                    if (!s_flagKeepTitleExcerpt) {
                        StringCchCopy(s_wchTitleExcerpt, COUNTOF(s_wchTitleExcerpt), L"");
                    }
                    Style_SetLexerFromFile(Globals.hwndEdit, Paths.CurrentFile);
                } else {
                    Path_Swap(_hpthLastSaveCopyDir, hfile_pth);
                    Path_RemoveFileSpec(_hpthLastSaveCopyDir);
                }
            }
        }
        Path_Release(hdir_pth);
        Path_Release(hfile_pth);
        if (!ok) {
            return false;
        }

    } else {
        InstallFileWatching(false); // terminate
        fSuccess = FileIO(false, Paths.CurrentFile, &fioStatus, FLF_None, FSF_EndSession, true);
    }

    if (fSuccess) {

        if (!(fSaveFlags & FSF_SaveCopy) && !Flags.bDoRelaunchElevated) {
            _MRU_AddSession();
            AddFilePathToRecentDocs(Paths.CurrentFile);
            // Install watching of the current file
            if ((fSaveFlags & FSF_SaveAs) && Settings.ResetFileWatching) {
                _ResetFileWatchingMode();
            }
            InstallFileWatching(true);
        }
        else {
            _MRU_UpdateSession();
        }

        // if current file is settings/config file: ask to start
        if (Flags.bSettingsFileSoftLocked && !IsAppClosing()) {
            ///~ LoadSettings(); NOT all settings will be applied ...
            WCHAR tch[256] = { L'\0' };
            if (Settings.SaveSettings) { LoadLngStringW(IDS_MUI_RELOADCFGSEX, tch, COUNTOF(tch)); }
            UINT const typ = Settings.SaveSettings ? (MB_YESNO | MB_ICONWARNING) : (MB_YESNO | MB_ICONINFORMATION);
            LONG const answer = InfoBoxLng(typ, L"ReloadExSavedCfg", IDS_MUI_RELOADSETTINGS, tch);
            if (IsYesOkay(answer)) {
                ///~SaveAllSettings(true); ~ already saved (CurrentFile)
                DialogNewWindow(Globals.hwndMain, true, Paths.CurrentFile, NULL);
                CloseApplication();
            }
        }

    } else if (!fioStatus.bCancelDataLoss) {

        LPCWSTR const currentFileName = Path_FindFileName(Paths.CurrentFile);

        if (!s_bIsProcessElevated && (Globals.dwLastError == ERROR_ACCESS_DENIED)) {
            INT_PTR const answer = (Settings.MuteMessageBeep) ?
                                   InfoBoxLng(MB_YESNO | MB_ICONSHIELD, NULL, IDS_MUI_ERR_ACCESSDENIED, currentFileName, _W(SAPPNAME)) :
                                   MessageBoxLng(MB_YESNO | MB_ICONSHIELD, IDS_MUI_ERR_ACCESSDENIED, Path_Get(Paths.CurrentFile), _W(SAPPNAME));
            if (IsYesOkay(answer)) {
                if (DoElevatedRelaunch(&fioStatus, true)) {
                    CloseApplication();
                } else {
                    ResetEvent(s_hEventAppIsClosing);
                    if (Settings.MuteMessageBeep) {
                        InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_ERR_SAVEFILE, currentFileName);
                    } else {
                        MessageBoxLng(MB_ICONWARNING, IDS_MUI_ERR_SAVEFILE, Path_Get(Paths.CurrentFile));
                    }
                }
            }
        } else {
            if (Settings.MuteMessageBeep) {
                InfoBoxLng(MB_ICONERROR, NULL, IDS_MUI_ERR_SAVEFILE, currentFileName);
            } else {
                MessageBoxLng(MB_ICONERROR, IDS_MUI_ERR_SAVEFILE, Path_Get(Paths.CurrentFile));
            }
        }
    }

    if (fSuccess) {
        AutoSaveStop();
        ResetFileObservationData(true);
    }

    UpdateToolbar();

    return fSuccess;
}


// ============================================================================
//
// CountRunningInstances()
//
//
static BOOL CALLBACK _EnumWndCountProc(HWND hwnd, LPARAM lParam)
{
    WCHAR szClassName[64] = { L'\0' };
    if (GetClassName(hwnd, szClassName, COUNTOF(szClassName))) {
        if (StrCmpW(szClassName, s_wchWndClass) == 0) {
            *(int*)lParam += 1;
        }
    }
    return TRUE;
}

int CountRunningInstances() {

    int count = 0;
    EnumWindows(_EnumWndCountProc, (LPARAM)&count);
    return count;
}


/******************************************************************************
*
* ActivatePrevInst()
*
* Tries to find and activate an already open Notepad3 Window
*
*
******************************************************************************/

bool ActivatePrevInst(const bool bSetForground)
{
    HWND           hwnd = NULL;
    COPYDATASTRUCT cds = { 0 };

    if ((!Flags.bReuseWindow && !Flags.bSingleFileInstance) || s_flagStartAsTrayIcon || s_flagNewFromClipboard || s_flagPasteBoard) {
        return false;
    }

    if (Flags.bSingleFileInstance && Path_IsNotEmpty(s_pthArgFilePath)) {

        Path_NormalizeEx(s_pthArgFilePath, Paths.WorkingDirectory, true, Flags.bSearchPathIfRelative);

        if (FindOtherInstance(&hwnd, s_pthArgFilePath)) {

            // Enabled
            if (IsWindowEnabled(hwnd)) {

                // Make sure the previous window won't pop up a change notification message
                //SendMessage(hwnd,WM_CHANGENOTIFYCLEAR,0,0);

                if (IsIconic(hwnd)) {
                    ShowWindowAsync(hwnd, SW_RESTORE);
                }
                if (!IsWindowVisible(hwnd)) {
                    SendMessage(hwnd,WM_TRAYMESSAGE,0,WM_LBUTTONDBLCLK);
                    SendMessage(hwnd,WM_TRAYMESSAGE,0,WM_LBUTTONUP);
                }
                if (bSetForground) {
                    SetForegroundWindow(hwnd);
                }
                size_t cb = sizeof(np3params);
                if (s_lpSchemeArg) {
                    cb += ((StringCchLen(s_lpSchemeArg, 0) + 1) * sizeof(WCHAR));
                }

                if (!IsFindPatternEmpty()) {
                    cb += ((LengthOfFindPattern() + 1) * sizeof(WCHAR));
                }
                LPnp3params params = AllocMem(cb, HEAP_ZERO_MEMORY);
                params->flagFileSpecified = false;
                params->flagChangeNotify = FWM_DONT_CARE; //(!)
                params->flagQuietCreate = false;
                params->flagLexerSpecified = s_flagLexerSpecified ? 1 : 0;
                if (s_flagLexerSpecified && s_lpSchemeArg) {
                    StringCchCopyW(StrEnd(&params->wchData,0)+1,(StringCchLen(s_lpSchemeArg,0)+1),s_lpSchemeArg);
                    params->iInitialLexer = -1;
                } else {
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
                    StringCchCopyW(StrEnd(&params->wchData, 0) + 1, (LengthOfFindPattern() + 1), GetFindPattern());
                }

                cds.dwData = DATA_NOTEPAD3_PARAMS;
                cds.cbData = (DWORD)SizeOfMem(params);
                cds.lpData = params;

                SendMessage(hwnd,WM_COPYDATA,(WPARAM)NULL,(LPARAM)&cds);
                FreeMem(params);
                params = NULL;

                return true;
            }
            // IsWindowEnabled()
            if (IsYesOkay(InfoBoxLng(MB_YESNO | MB_ICONWARNING, NULL, IDS_MUI_ERR_PREVWINDISABLED))) {
                return false;
            }
            return true;
        }
    }

    if (!Flags.bReuseWindow) {
        return false;
    }


    hwnd = NULL;
    if (!EnumWindows(_EnumWndProc, (LPARAM)&hwnd) && (hwnd != NULL)) {
        // Enabled
        if (hwnd && IsWindowEnabled(hwnd)) {

            // Make sure the previous window won't pop up a change notification message
            //SendMessage(hwnd,WM_CHANGENOTIFYCLEAR,0,0);

            if (IsIconic(hwnd)) {
                ShowWindowAsync(hwnd, SW_RESTORE);
            }
            if (!IsWindowVisible(hwnd)) {
                SendMessage(hwnd,WM_TRAYMESSAGE,0,WM_LBUTTONDBLCLK);
                SendMessage(hwnd,WM_TRAYMESSAGE,0,WM_LBUTTONUP);
            }
            // always set foreground (ignoring: bSetForground here)
            SetForegroundWindow(hwnd);

            if (Path_IsNotEmpty(s_pthArgFilePath))
            {
                size_t cb = sizeof(np3params);
                cb += (Path_GetLength(s_pthArgFilePath) + 1) * sizeof(WCHAR);

                if (s_lpSchemeArg) {
                    cb += (StringCchLen(s_lpSchemeArg, 0) + 1) * sizeof(WCHAR);
                }
                size_t cchTitleExcerpt = StringCchLen(s_wchTitleExcerpt,COUNTOF(s_wchTitleExcerpt));
                if (cchTitleExcerpt) {
                    cb += (cchTitleExcerpt + 1) * sizeof(WCHAR);
                }
                if (!IsFindPatternEmpty()) {
                    cb += ((LengthOfFindPattern() + 1) * sizeof(WCHAR));
                }

                LPnp3params params = AllocMem(cb, HEAP_ZERO_MEMORY);
                params->flagFileSpecified = true;
                StringCchCopyW(&params->wchData, Path_GetLength(s_pthArgFilePath) + 1, Path_Get(s_pthArgFilePath));
                params->flagChangeNotify = s_flagChangeNotify;
                params->flagQuietCreate = s_flagQuietCreate ? 1 : 0;
                params->flagLexerSpecified = s_flagLexerSpecified ? 1 : 0;
                if (s_flagLexerSpecified && s_lpSchemeArg) {
                    StringCchCopyW(StrEnd(&params->wchData,0)+1, StringCchLen(s_lpSchemeArg,0)+1,s_lpSchemeArg);
                    params->iInitialLexer = -1;
                } else {
                    params->iInitialLexer = s_iInitialLexer;
                }
                params->flagJumpTo = s_flagJumpTo ? 1 : 0;
                params->iInitialLine = s_iInitialLine;
                params->iInitialColumn = s_iInitialColumn;

                params->flagSetEncoding = s_flagSetEncoding;
                params->flagSetEOLMode = s_flagSetEOLMode;

                if (cchTitleExcerpt) {
                    StringCchCopyW(StrEnd(&params->wchData,0)+1,cchTitleExcerpt+1,s_wchTitleExcerpt);
                    params->flagTitleExcerpt = 1;
                } else {
                    params->flagTitleExcerpt = 0;
                }

                params->flagMatchText = g_flagMatchText;
                if (!IsFindPatternEmpty()) {
                    StringCchCopyW(StrEnd(&params->wchData, 0) + 1, (LengthOfFindPattern() + 1), GetFindPattern());
                }

                cds.dwData = DATA_NOTEPAD3_PARAMS;
                cds.cbData = (DWORD)SizeOfMem(params);
                cds.lpData = params;

                SendMessage(hwnd,WM_COPYDATA,(WPARAM)NULL,(LPARAM)&cds);
                FreeMem(params);
                params = NULL;
                Path_Empty(s_pthArgFilePath, false);
            }
            return true;
        }
        // IsWindowEnabled()
        return !IsYesOkay(InfoBoxLng(MB_YESNO | MB_ICONWARNING, NULL, IDS_MUI_ERR_PREVWINDISABLED));
    }
    return false;
}


//=============================================================================
//
//  LaunchNewInstance()
//
//
bool LaunchNewInstance(HWND hwnd, LPCWSTR lpszParameter, LPCWSTR lpszFilePath)
{
    HPATHL hexe_pth = Path_Allocate(NULL);
    Path_GetModuleFilePath(hexe_pth);

    HSTRINGW     hstrParams = StrgCreate(lpszParameter);
    LPWSTR const params_buf = StrgWriteAccessBuf(hstrParams, CMDLN_LENGTH_LIMIT);

    HPATHL hdir_pth = Path_Allocate(lpszFilePath);
    if (Path_IsEmpty(hdir_pth)) {
        Path_Reset(hdir_pth, Path_Get(Paths.WorkingDirectory));
    }
    else {
        int const offset = Settings2.LaunchInstanceWndPosOffset;
        int const bFullVisible = Settings2.LaunchInstanceFullVisible;
        int const instCnt = CountRunningInstances();
        WININFO wi = GetMyWindowPlacement(hwnd, NULL, offset * instCnt, bFullVisible);
        WCHAR wchPos[80] = { L'\0' };
        StringCchPrintf(wchPos, COUNTOF(wchPos), L"-pos " WINDOWPOS_STRGFORMAT, wi.x, wi.y, wi.cx, wi.cy, wi.dpi, (int)wi.max);

        StringCchPrintf(params_buf, StrgGetAllocLength(hstrParams),
            L"%s %s \"%s\"", lpszParameter, wchPos, lpszFilePath);
        Path_RemoveFileSpec(hdir_pth);
    }

    SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
    sei.fMask = SEE_MASK_DEFAULT;
    sei.hwnd = hwnd;
    sei.lpVerb = NULL;
    sei.lpFile = Path_Get(hexe_pth);
    sei.lpParameters = StrgGet(hstrParams);
    sei.lpDirectory = Path_Get(hdir_pth);
    sei.nShow = SW_NORMAL;
    bool const res = ShellExecuteExW(&sei);

    Path_Release(hdir_pth);
    StrgDestroy(hstrParams);
    Path_Release(hexe_pth);
    return res;
}


//=============================================================================
//
//  RelaunchMultiInst()
//
//
bool RelaunchMultiInst()
{
    if (Flags.MultiFileArg && (s_cFileList > 1)) {

        LPWSTR lpCmdLine = StrDup(GetCommandLine());

        size_t fl = 0;
        for (int i = 0; i < s_cFileList; i++) {
            fl = max_s(fl, StringCchLen(s_lpFileList[i], 0));
        }
        size_t len = StringCchLen(lpCmdLine,0) + fl + 80ULL;
        LPWSTR lpCmdLineNew = AllocMem(sizeof(WCHAR) * len, HEAP_ZERO_MEMORY);

        StrTab2Space(lpCmdLine);
        StringCchCopy(lpCmdLine + s_cchiFileList, 2, L"");

        WCHAR* pwch = CharPrev(lpCmdLine, StrEnd(lpCmdLine, len));
        int k = 0;
        while (*pwch == L' ' || *pwch == L'-' || *pwch == L'+') {
            *pwch = L' ';
            pwch = CharPrev(lpCmdLine, pwch);
            if (k++ > 1) {
                s_cchiFileList--;
            }
        }

        int const offset = Settings2.LaunchInstanceWndPosOffset;

        for (int i = 0; i < s_cFileList; i++) {

            WININFO wi = g_IniWinInfo;
            wi.x += (i * offset);
            wi.y += (i * offset);
            WCHAR wchPos[80] = { L'\0' };
            if (!Globals.CmdLnFlag_PosParam) {
                StringCchPrintf(wchPos, COUNTOF(wchPos), L" -pos " WINDOWPOS_STRGFORMAT, wi.x, wi.y, wi.cx, wi.cy, wi.dpi, (int)wi.max);
            }
            size_t const pl = StringCchLen(wchPos, 80) + 1;

            StringCchCopy(lpCmdLineNew, len, lpCmdLine);
            StringCchCat(lpCmdLineNew, len, wchPos);
            StringCchCopy(lpCmdLineNew + s_cchiFileList + pl, 8, L" /n - ");
            StringCchCat(lpCmdLineNew, len, s_lpFileList[i]);

            LocalFree(s_lpFileList[i]); // StrDup()
            s_lpFileList[i] = NULL;

            STARTUPINFO si = { sizeof(STARTUPINFO) };
            PROCESS_INFORMATION pi = { 0 };
            CreateProcessW(NULL, lpCmdLineNew, NULL, NULL, false, CREATE_NEW_PROCESS_GROUP, NULL, Path_Get(Paths.WorkingDirectory), &si, &pi);
            //~WaitForSingleObject(pi.hProcess, INFINITE);
            WaitForSingleObject(pi.hProcess, 125); // pause 125ms to start next
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        }

        LocalFree(lpCmdLine); // StrDup()
        lpCmdLine = NULL;
        FreeMem(lpCmdLineNew);
        lpCmdLineNew = NULL;
        Path_Empty(s_pthArgFilePath, true);

        return true;
    }

    for (int i = 0; i < s_cFileList; i++) {
        LocalFree(s_lpFileList[i]); // StrDup()
        s_lpFileList[i] = NULL;
    }

    return false;
}


//=============================================================================
//
//  RelaunchElevated()
//
//
bool RelaunchElevated(LPCWSTR lpNewCmdLnArgs)
{
    if (!IsWindowsVistaOrGreater() || !Flags.bDoRelaunchElevated ||
            s_bIsProcessElevated || s_IsThisAnElevatedRelaunch || s_bIsRunAsAdmin ||
            s_flagDisplayHelp) {
        return false; // reject initial RelaunchElevated() try
    }

    LPCWSTR      lpCmdLine = GetCommandLine();
    size_t const wlen = StringCchLen(lpCmdLine, 0) + 2ULL;

    HPATHL hfile = Path_Allocate(NULL);
    wchar_t* const fbuf = Path_WriteAccessBuf(hfile, PATHLONG_MAX_CCH);

    HSTRINGW hstrOrigArgs = StrgCreate(NULL);
    wchar_t* const arg_buf = StrgWriteAccessBuf(hstrOrigArgs, CMDLN_LENGTH_LIMIT);

    ExtractFirstArgument(lpCmdLine, fbuf, arg_buf, min_i((int)wlen, CMDLN_LENGTH_LIMIT));
    // overrides:
    Path_GetModuleFilePath(hfile);
    if (StrIsNotEmpty(lpNewCmdLnArgs)) {
        StringCchCopy(arg_buf, StrgGetAllocLength(hstrOrigArgs), lpNewCmdLnArgs);
    }
    StrgSanitize(hstrOrigArgs);
    StrgCat(hstrOrigArgs, L" ");
    // remove relaunch elevated, we are doing this here already
    StrCutIW(arg_buf, L"/u ");
    StrCutIW(arg_buf, L"-u ");
    StrgSanitize(hstrOrigArgs);

    if (!(StrStrIW(arg_buf, L"/f ") || StrStrIW(arg_buf, L"-f ") || Path_IsEmpty(Paths.IniFile)))
    {
        HSTRINGW hipa_str = StrgCreate(NULL);
        StrgFormat(hipa_str, L"/f \"%s\" ", Path_Get(Paths.IniFile));
        StrgInsert(hstrOrigArgs, 0, StrgGet(hipa_str));
        StrgDestroy(hipa_str);
    }

    bool res = false;
    if (StrgIsNotEmpty(hstrOrigArgs)) {

        STARTUPINFO si = { sizeof(STARTUPINFO) };
        GetStartupInfo(&si);

        SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
        sei.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOASYNC | SEE_MASK_UNICODE | SEE_MASK_HMONITOR | SEE_MASK_NOZONECHECKS | SEE_MASK_WAITFORINPUTIDLE;
        sei.hwnd = GetForegroundWindow();
        sei.hMonitor = MonitorFromWindow(sei.hwnd, MONITOR_DEFAULTTONEAREST);
        sei.lpVerb = L"runas";
        sei.lpFile = Path_Get(hfile);
        sei.lpParameters = StrgGet(hstrOrigArgs);
        sei.lpDirectory = Path_Get(Paths.WorkingDirectory);
        sei.nShow = si.wShowWindow ? si.wShowWindow : SW_SHOWNORMAL;
        res = ShellExecuteExW(&sei);
    }

    StrgDestroy(hstrOrigArgs);
    Path_Release(hfile);
    return res;
}


//=============================================================================
//
//  ShowNotifyIcon()
//
//
void ShowNotifyIcon(HWND hwnd,bool bAdd)
{
    HICON hIcon = NULL;
    LoadIconWithScaleDown(Globals.hInstance, MAKEINTRESOURCE(IDR_MAINWND),
                          GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), &hIcon);

    NOTIFYICONDATA nid = { sizeof(NOTIFYICONDATA) };
    nid.hWnd = hwnd;
    nid.uID = 0;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYMESSAGE;
    nid.hIcon = hIcon;
    StringCchCopy(nid.szTip,COUNTOF(nid.szTip), _W(SAPPNAME));

    if (bAdd) {
        Shell_NotifyIcon(NIM_ADD,&nid);
    } else {
        Shell_NotifyIcon(NIM_DELETE,&nid);
    }
}


//=============================================================================
//
//  SetNotifyIconTitle()
//
void SetNotifyIconTitle(HWND hwnd)
{
    NOTIFYICONDATA nid = { sizeof(NOTIFYICONDATA) };
    nid.hWnd = hwnd;
    nid.uID = 0;
    nid.uFlags = NIF_TIP;
    nid.szTip[0] = L'\0';

    WCHAR tchTitle[256] = { L'\0' };
    if (StrIsNotEmpty(s_wchTitleExcerpt)) {
        WCHAR tchFormat[128] = { L'\0' };
        GetLngString(IDS_MUI_TITLEEXCERPT,tchFormat,COUNTOF(tchFormat));
        StringCchPrintf(tchTitle,COUNTOF(tchTitle),tchFormat,s_wchTitleExcerpt);
    }
    else if (Path_IsNotEmpty(Paths.CurrentFile)) {
        HPATHL hfilename = Path_Allocate(Path_FindFileName(Paths.CurrentFile));
        StringCchCopy(tchTitle, COUNTOF(tchTitle), Path_Get(hfilename));
        Path_Release(hfilename);
    } else {
        GetLngString(IDS_MUI_UNTITLED, tchTitle, COUNTOF(tchTitle) - 4);
    }
    if (IsSaveNeeded()) {
        StringCchCat(nid.szTip, COUNTOF(nid.szTip), DOCMODDIFYD);
    } 
    if (IsFileChangedFlagSet()) {
        if (IsFileDeletedFlagSet()) {
            StringCchCatN(nid.szTip, COUNTOF(nid.szTip), Settings2.FileDeletedIndicator, COUNTOF(Settings2.FileDeletedIndicator));
        } else {
            StringCchCatN(nid.szTip, COUNTOF(nid.szTip), Settings2.FileChangedIndicator, COUNTOF(Settings2.FileChangedIndicator));
        }
        StringCchCat(nid.szTip, COUNTOF(nid.szTip), L" ");
    }
    StringCchCat(nid.szTip, COUNTOF(nid.szTip), tchTitle);

    Shell_NotifyIcon(NIM_MODIFY,&nid);
}


//=============================================================================
//
//  ResetMouseDWellTime()
//
void ResetMouseDWellTime()
{
    //~if (Settings.ShowHypLnkToolTip || IsColorDefHotspotEnabled() || Settings.HighlightUnicodePoints) {
    //~    SciCall_SetMouseDWellTime(USER_TIMER_MINIMUM << 4);
    //~} else {
    //~    Sci_DisableMouseDWellNotification();
    //~}
    SciCall_SetMouseDWellTime(500); // needed for "Mouse cursor vanish handling (hide while typing)"
}

//=============================================================================
//
//  ShowZoomCallTip()
//
void ShowZoomCallTip()
{
    static char chToolTip[64] = { '\0' };
    int const   delayClr = Settings2.ZoomTooltipTimeout;
    if (delayClr >= (_MQ_TIMER_CYCLE << 3)) {

        StringCchPrintfA(chToolTip, COUNTOF(chToolTip), "Zoom: %i%%", SciCall_GetZoom());

        DocPos const iPos = SciCall_PositionFromLine(SciCall_GetFirstVisibleLine());

        int const iXOff = SciCall_GetXOffset();
        SciCall_SetXOffset(-4); // move away from margin

        SciCall_CallTipSetPosition(true);  // show above
        SciCall_CallTipShow(iPos, chToolTip);
        
        SciCall_SetXOffset(iXOff);
        _DelayClearCallTip(delayClr);
    } else {
        Sci_CallTipCancelEx();
    }
}


//=============================================================================
//
//  ShowWrapAroundCallTip()
//
void ShowWrapAroundCallTip(bool forwardSearch)
{
    static char chToolTip[80<<2] = { '\0' };
    int const   delayClr = Settings2.WrapAroundTooltipTimeout;
    if (delayClr >= (_MQ_TIMER_CYCLE << 3)) {
        WCHAR wchToolTip[80] = { '\0' };
        if (forwardSearch) {
            GetLngString(IDS_MUI_WRAPSEARCH_FWD, wchToolTip, COUNTOF(wchToolTip));
        } else {
            GetLngString(IDS_MUI_WRAPSEARCH_BCK, wchToolTip, COUNTOF(wchToolTip));
        }
        //StringCchCat(wchToolTip, COUNTOF(wchToolTip), FR_StatusW[Globals.FindReplaceMatchFoundState]);
        WideCharToMultiByte(Encoding_SciCP, 0, wchToolTip, -1, chToolTip, (int)COUNTOF(chToolTip), NULL, NULL);
        SciCall_CallTipShow(SciCall_GetCurrentPos(), chToolTip);
        _DelayClearCallTip(delayClr);
    } else {
        Sci_CallTipCancelEx();
    }
}


//=============================================================================
//
//  PasteBoardTimerProc()
//
void CALLBACK PasteBoardTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {

    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(idEvent);
    UNREFERENCED_PARAMETER(dwTime);

    if ((s_iLastCopyTime > 0) && ((GetTicks_ms() - s_iLastCopyTime) > 200)) {

        if (SciCall_CanPaste()) {
            bool bAutoIndent2 = Settings.AutoIndent;
            Settings.AutoIndent = 0;
            EditJumpTo(-1, 0);
            UndoTransActionBegin();
            if (!Sci_IsDocEmpty()) {
                SciCall_NewLine();
            }
            SciCall_Paste();
            SciCall_NewLine();
            EndUndoTransAction();
            Sci_ScrollSelectionToView();
            Settings.AutoIndent = bAutoIndent2;
        }
        s_iLastCopyTime = 0;
    }
}


//=============================================================================
//
//  MsgFileChangeNotify() - Handles WM_FILECHANGEDNOTIFY
//
LRESULT MsgFileChangeNotify(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    if (IsAppClosing()) { return TRUE; }

    SET_FCT_GUARD(TRUE);

    DocPos const iCurPos = SciCall_GetCurrentPos();

    if (Path_IsExistingFile(Paths.CurrentFile)) {

        bool bRevertFile = IsSaveNeeded();

        switch (FileWatching.FileWatchingMode) {
        case FWM_AUTORELOAD:
            bRevertFile = true;
            break;

        case FWM_MSGBOX: {
            /// LONG const answer = MessageBoxExW(Globals.hwndMain, L"File change, Cancel, Retry, Continue", L"NP3", MB_ABORTRETRYIGNORE, GetLangIdByLocaleName(Globals.CurrentLngLocaleName));
            LONG const answer = InfoBoxLng(MB_FILECHANGEDNOTIFY | MB_ICONWARNING, NULL, IDS_MUI_FILECHANGENOTIFY);
            switch (LOWORD(answer)) {
            case IDCANCEL:
            case IDABORT:
                FileWatching.FileWatchingMode = FWM_INDICATORSILENT;
                SetSaveNeeded(true);
                bRevertFile = false;
                ResetFileObservationData(false); // false (!)
                UpdateToolbar();
                break;

            case IDIGNORE:
            case IDCONTINUE:
                UndoRedoReset();
                FileRevert(Paths.CurrentFile, false);
                SciCall_SetReadOnly(FileWatching.MonitoringLog);
                FileWatching.MonitoringLog = false; // will be reset in IDM_VIEW_CHASING_DOCTAIL
                PostWMCommand(Globals.hwndMain, IDM_VIEW_CHASING_DOCTAIL);
                bRevertFile = false; // done already
                break;

            case IDTRYAGAIN:
            case IDCLOSE:
            default:
                bRevertFile = true;
                ResetFileObservationData(true);
                break;
            }
        } break;

        case FWM_DONT_CARE:
        case FWM_EXCLUSIVELOCK:
            ResetFileObservationData(true);
            break;

        case FWM_INDICATORSILENT:
            bRevertFile = false;
            UpdateToolbar();
            break;

        case FWM_NO_INIT:
        default:
            assert("Invalid FileWatching Mode!" && 0);
            break;
        }

        if (bRevertFile) {
            FileRevert(Paths.CurrentFile, false);
            if (FileWatching.MonitoringLog || (s_flagChangeNotify == FWM_AUTORELOAD)) {
                SciCall_SetReadOnly(FileWatching.MonitoringLog);
            }
            else {
                Sci_GotoPosChooseCaret(iCurPos);
            }
        }
    }
    else { // file has been deleted

        InstallFileWatching(false); // terminate

        if (FileWatching.FileWatchingMode == FWM_MSGBOX) {
            if (IsYesOkay(InfoBoxLng(MB_YESNO | MB_ICONWARNING, NULL, IDS_MUI_FILECHANGENOTIFY2))) {
                FileSave(FSF_SaveAlways);
            }
            else {
                SetSaveNeeded(true);
            }
        }
        else {
            // FWM_INDICATORSILENT: nothing todo here
            SetSaveNeeded(true);
        }
    }

    RESET_FCT_GUARD();
    return TRUE;
}


//=============================================================================
//
//  InstallFileWatching()
//
//=============================================================================

static inline void NotifyIfFileHasChanged()
{
    if (IsFileChangedFlagSet() || IsFileDeletedFlagSet() || RaiseFlagIfCurrentFileChanged()) {
        PostMessage(Globals.hwndMain, WM_FILECHANGEDNOTIFY, 0, 0);
    }
    // reset Timeout interval
    InterlockedExchange64(&(s_FileChgObsvrData.iFileChangeNotifyTime), GetTicks_ms());
}
// ----------------------------------------------------------------------------

// FWM_MSGBOX (polling: FileWatching.FileCheckInterval)
// FWM_AUTORELOAD (also FileWatching.MonitoringLog)
static void CALLBACK WatchTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {

    UNREFERENCED_PARAMETER(dwTime);
    UNREFERENCED_PARAMETER(idEvent);
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(hwnd);

    LONG64 const diff = (GetTicks_ms() - InterlockedOr64(&(s_FileChgObsvrData.iFileChangeNotifyTime), 0LL));
    // Directory-Observer is not notified for continuously updated (log-)files
    if (diff > FileWatching.FileCheckInterval) {
        NotifyIfFileHasChanged();
    }
}
// ----------------------------------------------------------------------------


unsigned int WINAPI FileChangeObserver(LPVOID lpParam)
{
    PFCOBSRVDATA_T const pFCOBSVData = (PFCOBSRVDATA_T)(LONG_PTR)lpParam;

    unsigned int retcode = 0;

    if (pFCOBSVData) {

        BackgroundWorker* const worker = &(pFCOBSVData->worker);

        assert(!IS_VALID_HANDLE(pFCOBSVData->hFileChanged) && "ChangeHandle not properly closed!");

        pFCOBSVData->hFileChanged = FindFirstChangeNotificationW(Path_Get(pFCOBSVData->worker.hFilePath), false,
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
            FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
            FILE_NOTIFY_CHANGE_LAST_WRITE);

        while (BackgroundWorker_Continue(worker)) {

            switch (WaitForSingleObject(pFCOBSVData->hFileChanged, 100)) {

            case WAIT_TIMEOUT:
                // okay, check BGWorker cancellation or wait again until done
                break;

            case WAIT_OBJECT_0:
                // check if current file is trigger for directory notification
                if (RaiseFlagIfCurrentFileChanged()) {
                    if (FileWatching.FileCheckInterval <= MIN_FC_POLL_INTERVAL) {
                        NotifyIfFileHasChanged(); // immediate notification
                    }
                }
                FindNextChangeNotification(pFCOBSVData->hFileChanged);
                break;

            case WAIT_ABANDONED:
            case WAIT_FAILED:
            default:
                BackgroundWorker_Cancel(worker);
                retcode = 1;
                break;
            }
        }

        FindCloseChangeNotification(pFCOBSVData->hFileChanged); // stop monitoring
        pFCOBSVData->hFileChanged = INVALID_HANDLE_VALUE;

        BackgroundWorker_End(worker, retcode);
    }
    return retcode;
}
// ----------------------------------------------------------------------------


void InstallFileWatching(const bool bInstall) {

    static HANDLE _hCurrFileHandle = INVALID_HANDLE_VALUE;  // exclusive lock

    // don't install FileWathing on own Settings IniFile
    if (bInstall && Flags.bSettingsFileSoftLocked) {
        return;
    }

    HPATHL hdir_pth = Path_Copy(Paths.CurrentFile);
    Path_RemoveFileSpec(hdir_pth);

    bool const bFileDirExists = Path_IsNotEmpty(Paths.CurrentFile) && Path_IsExistingDirectory(hdir_pth);
    bool const bFileExists = bFileDirExists && Path_IsExistingFile(Paths.CurrentFile);

    bool const bExclusiveLock = (FileWatching.FileWatchingMode == FWM_EXCLUSIVELOCK);
    bool const bWatchFile = (FileWatching.FileWatchingMode != FWM_DONT_CARE) && !bExclusiveLock;

    // always release exclusive file lock in any case
    if (IS_VALID_HANDLE(_hCurrFileHandle)) {
        CloseHandle(_hCurrFileHandle);
        _hCurrFileHandle = INVALID_HANDLE_VALUE;
    }

    // static init
    if (!IS_VALID_HANDLE(s_FileChgObsvrData.worker.eventCancel)) {
        BackgroundWorker_Init(&(s_FileChgObsvrData.worker), NULL, NULL);
    }

    bool const bTerminate = !bInstall || !bWatchFile || !bFileDirExists /*~||!bFileExists~*/;

    // Terminate previous watching
    if (bTerminate) {
        KillTimer(Globals.hwndMain, ID_WATCHTIMER);
        BackgroundWorker_Cancel(&(s_FileChgObsvrData.worker));
        ResetFileObservationData(false); // (!) false
    }

    if (bInstall && bFileExists) {

        if (bWatchFile) {

            if (!IS_VALID_HANDLE(s_FileChgObsvrData.worker.workerThread)) {

                // Save data of current file
                ResetFileObservationData(false); // (!) false

                Path_Reset(s_FileChgObsvrData.worker.hFilePath, Path_Get(hdir_pth)); // directory monitoring

                BackgroundWorker_Start(&(s_FileChgObsvrData.worker), FileChangeObserver, &s_FileChgObsvrData);
            }

            InterlockedExchange64(&(s_FileChgObsvrData.iFileChangeNotifyTime), GetTicks_ms());

            if (Settings2.FileCheckInterval > 0) {
                SetTimer(Globals.hwndMain, ID_WATCHTIMER, (UINT)FileWatching.FileCheckInterval, WatchTimerProc);
            }
            else {
                KillTimer(Globals.hwndMain, ID_WATCHTIMER);
            }

        }
        else if (bExclusiveLock) {

            assert(!IS_VALID_HANDLE(_hCurrFileHandle) && "CurrFileHandle not properly closed!");

            bool const bPrevReadOnlyAttrib = IsFileReadOnly();
            if (bPrevReadOnlyAttrib) {
                PostWMCommand(Globals.hwndMain, IDM_FILE_READONLY); // try to gain access
            }

            if (!IsFileReadOnly()) {
                _hCurrFileHandle = CreateFile(Path_Get(Paths.CurrentFile),
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ, // 0 => NO FILE_SHARE_RW
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);

                Globals.dwLastError = GetLastError();

                if (!IS_VALID_HANDLE(_hCurrFileHandle)) {

                    WCHAR wchDisplayName[MAX_PATH_EXPLICIT>>1];

                    GetLngString(IDS_MUI_UNTITLED, wchDisplayName, COUNTOF(wchDisplayName));
                    Path_GetDisplayName(wchDisplayName, COUNTOF(wchDisplayName), Paths.CurrentFile, NULL, false);

                    InfoBoxLng(MB_ICONERROR, NULL, IDS_MUI_FILELOCK_ERROR, wchDisplayName);

                    // need to chose another mode
                    FILE_WATCHING_MODE const fwm = Settings.FileWatchingMode;
                    FileWatching.FileWatchingMode = (fwm != FWM_EXCLUSIVELOCK) ? fwm : FWM_MSGBOX;
                    InstallFileWatching(true);
                }
            }

            if (bPrevReadOnlyAttrib && !IsFileReadOnly()) {
                PostWMCommand(Globals.hwndMain, IDM_FILE_READONLY); // try to reset
            }
        }
    }
    Path_Release(hdir_pth);
    
    UpdateSaveSettingsCmds(); // (!) reflection
    UpdateToolbar();
}



//=============================================================================
//
//  AutoSaveStart()
//

static bool s_bAutoSaveTimerSet = false;

void        AutoSaveStart(bool bReset)
{
    if ((Settings.AutoSaveOptions & ASB_Periodic) && Settings.AutoSaveInterval >= USER_TIMER_MINIMUM) {
        if (bReset || !s_bAutoSaveTimerSet) {
            s_bAutoSaveTimerSet = true;
            SetTimer(Globals.hwndMain, ID_AUTOSAVETIMER, Settings.AutoSaveInterval, AutoSaveTimerProc);
        }
        return;
    }
    if (s_bAutoSaveTimerSet) {
        s_bAutoSaveTimerSet = false;
        KillTimer(Globals.hwndMain, ID_AUTOSAVETIMER);
    }
}


//=============================================================================
//
//  AutoSaveStop()
//
void AutoSaveStop()
{
    if (s_bAutoSaveTimerSet) {
        s_bAutoSaveTimerSet = false;
        KillTimer(Globals.hwndMain, ID_AUTOSAVETIMER);
    }
}


//=============================================================================
//
//  AutoSaveDoWork()
//
void AutoSaveDoWork(FileSaveFlags fSaveFlags)
{
	if (!IsSaveNeeded()) {
        return;
    }
    FileSave(fSaveFlags | FSF_AutoSave);
}

//=============================================================================
//
//  AutoSaveTimerProc()
//
void CALLBACK AutoSaveTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(idEvent);
    UNREFERENCED_PARAMETER(dwTime);

    AutoSaveDoWork(FSF_None);
}


///  End of Notepad3.c  ///
