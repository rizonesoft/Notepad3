/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Notepad3.c                                                                  *
*   Main application window functionality                                     *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2017   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

#if !defined(WINVER)
#define WINVER 0x601  /*_WIN32_WINNT_WIN7*/
#endif
#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x601  /*_WIN32_WINNT_WIN7*/
#endif
#if !defined(NTDDI_VERSION)
#define NTDDI_VERSION 0x06010000  /*NTDDI_WIN7*/
#endif
#define VC_EXTRALEAN 1
#define WIN32_LEAN_AND_MEAN 1

#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <commdlg.h>
#include <stdio.h>
#include <string.h>
//#include <pathcch.h>
#include <time.h>

#include "scintilla.h"
#include "scilexer.h"
#include "edit.h"
#include "styles.h"
#include "dialogs.h"
#include "resource.h"
#include "../crypto/crypto.h"
#include "../uthash/utarray.h"
#include "../uthash/utlist.h"
#include "encoding.h"
#include "helpers.h"
#include "SciCall.h"

#include "notepad3.h"



/******************************************************************************
*
* Local and global Variables for Notepad3.c
*
*/
HWND      g_hwndMain = NULL;
HWND      g_hwndStatus = NULL;
HWND      g_hwndToolbar = NULL;
HWND      g_hwndDlgFindReplace = NULL;
HWND      g_hwndDlgCustomizeSchemes = NULL;
HWND      g_hwndReBar = NULL;
HWND      hwndEditFrame = NULL;
HWND      hwndNextCBChain = NULL;

bool g_bExternalBitmap = false;

#define INISECTIONBUFCNT 32

TBBUTTON  tbbMainWnd[] = {  { 0,IDT_FILE_NEW,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
                            { 1,IDT_FILE_OPEN,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
                            { 3,IDT_FILE_SAVE,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
                            { 2,IDT_FILE_BROWSE,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
                            { 0,0,0,TBSTYLE_SEP,0,0 },
                            { 4,IDT_EDIT_UNDO,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
                            { 5,IDT_EDIT_REDO,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
                            { 0,0,0,TBSTYLE_SEP,0,0 },
                            { 6,IDT_EDIT_CUT,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
                            { 7,IDT_EDIT_COPY,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
                            { 8,IDT_EDIT_PASTE,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
                            { 0,0,0,TBSTYLE_SEP,0,0 },
                            { 9,IDT_EDIT_FIND,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
                            { 10,IDT_EDIT_REPLACE,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
                            { 0,0,0,TBSTYLE_SEP,0,0 },
                            { 11,IDT_VIEW_WORDWRAP,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
                            { 0,0,0,TBSTYLE_SEP,0,0 },
                            { 23,IDT_VIEW_TOGGLEFOLDS,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
                            { 25,IDT_VIEW_TOGGLE_VIEW,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
                            { 0,0,0,TBSTYLE_SEP,0,0 },
                            { 21,IDT_FILE_OPENFAV,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
                            { 22,IDT_FILE_ADDTOFAV,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
                            { 0,0,0,TBSTYLE_SEP,0,0 },
                            { 12,IDT_VIEW_ZOOMIN,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
                            { 13,IDT_VIEW_ZOOMOUT,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
                            { 0,0,0,TBSTYLE_SEP,0,0 },
                            { 26,IDT_VIEW_CHASING_DOCTAIL,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
                            { 0,0,0,TBSTYLE_SEP,0,0 },
                            { 14,IDT_VIEW_SCHEME,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
                            { 0,0,0,TBSTYLE_SEP,0,0 },
                            { 24,IDT_FILE_LAUNCH,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
                            { 0,0,0,TBSTYLE_SEP,0,0 },
                            { 16,IDT_FILE_EXIT,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
                            { 0,0,0,TBSTYLE_SEP,0,0 },
                            { 15,IDT_VIEW_SCHEMECONFIG,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
                            { 0,0,0,TBSTYLE_SEP,0,0 },
                            { 17,IDT_FILE_SAVEAS,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
                            { 18,IDT_FILE_SAVECOPY,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
                            { 19,IDT_EDIT_CLEAR,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
                            { 20,IDT_FILE_PRINT,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 }
};

#define NUMTOOLBITMAPS  27
#define NUMINITIALTOOLS 33
#define TBBUTTON_DEFAULT_IDS  L"1 2 4 3 0 5 6 0 7 8 9 0 10 11 0 12 0 24 26 0 22 23 0 13 14 0 27 0 15 0 25 0 17"


WCHAR         g_wchIniFile[MAX_PATH] = { L'\0' };
WCHAR         g_wchIniFile2[MAX_PATH] = { L'\0' };
static WCHAR  g_szTmpFilePath[MAX_PATH] = { L'\0' };

bool          bSaveSettings;
bool          bEnableSaveSettings;
bool          bSaveRecentFiles;
bool          bPreserveCaretPos;
bool          bSaveFindReplace;
bool          bFindReplCopySelOrClip = true;

WCHAR         g_tchFileDlgFilters[XXXL_BUFFER] = { L'\0' };

WCHAR         g_tchLastSaveCopyDir[MAX_PATH] = { L'\0' };
WCHAR         g_tchOpenWithDir[MAX_PATH] = { L'\0' };
WCHAR         g_tchFavoritesDir[MAX_PATH] = { L'\0' };
WCHAR         g_tchUpdateCheckerExe[MAX_PATH] = { L'\0' };

static WCHAR  g_tchDefaultExtension[64] = { L'\0' };
static WCHAR  g_tchDefaultDir[MAX_PATH] = { L'\0' };
static WCHAR  g_tchToolbarButtons[MIDSZ_BUFFER] = { L'\0' };


static WCHAR  g_tchStatusbarPrefixes[MIDSZ_BUFFER] = { L'\0' };
static prefix_t g_mxStatusBarPrefix[STATUS_SECTOR_COUNT];

static WCHAR  g_tchStatusbarSections[SMALL_BUFFER] = { L'\0' };
static int    g_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;
static bool   g_iStatusbarVisible[STATUS_SECTOR_COUNT] = SBS_INIT_ZERO;
static int    g_vSBSOrder[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

static WCHAR  g_tchStatusbarWidthSpec[SMALL_BUFFER] = { L'\0' };
static int    g_iStatusbarWidthSpec[STATUS_SECTOR_COUNT] = SBS_INIT_ZERO;


static WCHAR  g_tchToolbarBitmap[MAX_PATH] = { L'\0' };
static WCHAR  g_tchToolbarBitmapHot[MAX_PATH] = { L'\0' };
static WCHAR  g_tchToolbarBitmapDisabled[MAX_PATH] = { L'\0' };

int       iPathNameFormat;
bool      g_bWordWrap;
bool      bWordWrapG;
int       iWordWrapMode;
int       iWordWrapIndent;
int       iWordWrapSymbols;
bool      bShowWordWrapSymbols;
bool      bMatchBraces;
bool      bAutoIndent;
bool      bAutoCloseTags;
bool      bShowIndentGuides;
bool      bHiliteCurrentLine;
bool      g_bHyperlinkHotspot;
bool      bScrollPastEOF;
bool      g_bTabsAsSpaces;
bool      bTabsAsSpacesG;
bool      g_bTabIndents;
bool      bTabIndentsG;
bool      bBackspaceUnindents;
int       g_iTabWidth;
int       iTabWidthG;
int       g_iIndentWidth;
int       iIndentWidthG;
bool      bMarkLongLines;
int       g_iLongLinesLimit;
int       iLongLinesLimitG;
int       iLongLineMode;
int       iWrapCol = 0;
bool      g_bShowSelectionMargin;
bool      bShowLineNumbers;
int       iReplacedOccurrences;
int       g_iMarkOccurrences;
int       g_iMarkOccurrencesCount;
int       g_iMarkOccurrencesMaxCount;
bool      g_bMarkOccurrencesMatchVisible;
bool      bMarkOccurrencesMatchCase;
bool      bMarkOccurrencesMatchWords;
bool      bMarkOccurrencesCurrentWord;
bool      bUseOldStyleBraceMatching;
bool      bAutoCompleteWords;
bool      bAccelWordNavigation;
bool      bDenyVirtualSpaceAccess;
bool      g_bCodeFoldingAvailable;
bool      g_bShowCodeFolding;
bool      bViewWhiteSpace;
bool      bViewEOLs;
bool      bUseDefaultForFileEncoding;
bool      bSkipUnicodeDetection;
bool      bSkipANSICodePageDetection;
bool      bLoadASCIIasUTF8;
bool      bLoadNFOasOEM;
bool      bNoEncodingTags;
bool      bFixLineEndings;
bool      bAutoStripBlanks;
int       iPrintHeader;
int       iPrintFooter;
int       iPrintColor;
int       iPrintZoom;
RECT      pagesetupMargin;
bool      bSaveBeforeRunningTools;
int       g_iFileWatchingMode;
bool      g_bResetFileWatching;
DWORD     dwFileCheckInverval;
DWORD     dwAutoReloadTimeout;
int       iEscFunction;
bool      bAlwaysOnTop;
bool      bMinimizeToTray;
bool      bTransparentMode;
bool      bTransparentModeAvailable;
bool      bShowToolbar;
bool      bShowStatusbar;
int       iSciDirectWriteTech;
int       iSciFontQuality;
int       iHighDpiToolBar;
int       iUpdateDelayHyperlinkStyling;
int       iUpdateDelayMarkAllCoccurrences;
int       iCurrentLineHorizontalSlop = 0;
int       iCurrentLineVerticalSlop = 0;
bool      g_bChasingDocTail = false;


const int DirectWriteTechnology[4] = {
    SC_TECHNOLOGY_DEFAULT
  , SC_TECHNOLOGY_DIRECTWRITE
  , SC_TECHNOLOGY_DIRECTWRITERETAIN
  , SC_TECHNOLOGY_DIRECTWRITEDC
};

const int FontQuality[4] = {
    SC_EFF_QUALITY_DEFAULT
  , SC_EFF_QUALITY_NON_ANTIALIASED
  , SC_EFF_QUALITY_ANTIALIASED
  , SC_EFF_QUALITY_LCD_OPTIMIZED
};

static  WININFO g_WinInfo = { CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0 };
static  int     g_WinCurrentWidth = 0;

bool    bStickyWinPos;

bool    bIsAppThemed;
int     cyReBar;
int     cyReBarFrame;
int     cxEditFrame;
int     cyEditFrame;

int     cxEncodingDlg;
int     cyEncodingDlg;
int     cxRecodeDlg;
int     cyRecodeDlg;
int     cxFileMRUDlg;
int     cyFileMRUDlg;
int     cxOpenWithDlg;
int     cyOpenWithDlg;
int     cxFavoritesDlg;
int     cyFavoritesDlg;
int     xFindReplaceDlg;
int     yFindReplaceDlg;
int     xCustomSchemesDlg;
int     yCustomSchemesDlg;


LPWSTR    lpFileList[32] = { NULL };
int       cFileList = 0;
int       cchiFileList = 0;
LPWSTR    lpFileArg = NULL;
LPWSTR    lpSchemeArg = NULL;
LPWSTR    lpMatchArg = NULL;
LPWSTR    lpEncodingArg = NULL;
LPMRULIST g_pFileMRU;
LPMRULIST g_pMRUfind;
LPMRULIST g_pMRUreplace;

DWORD     dwLastIOError;

int       g_iDefaultNewFileEncoding;
int       g_iDefaultCharSet;

int       g_iEOLMode;
int       g_iDefaultEOLMode;

int       iInitialLine;
int       iInitialColumn;

int       iInitialLexer;

bool      bLastCopyFromMe = false;
DWORD     dwLastCopyTime;

UINT      uidsAppTitle = IDS_APPTITLE;
WCHAR     szTitleExcerpt[MIDSZ_BUFFER] = { L'\0' };
int       fKeepTitleExcerpt = 0;

HANDLE    hChangeHandle = NULL;
bool      dwChangeNotifyTime = 0;
WIN32_FIND_DATA fdCurFile;

UINT      msgTaskbarCreated = 0;

HMODULE   hModUxTheme = NULL;
HMODULE   hRichEdit = NULL;

static bool g_bRunningWatch = false;

static EDITFINDREPLACE g_efrData = EFR_INIT_DATA;
bool bReplaceInitialized = false;

int iLineEndings[3] = {
  SC_EOL_CRLF,
  SC_EOL_LF,
  SC_EOL_CR
};

WCHAR wchPrefixSelection[256] = { L'\0' };
WCHAR wchAppendSelection[256] = { L'\0' };

WCHAR wchPrefixLines[256] = { L'\0' };
WCHAR wchAppendLines[256] = { L'\0' };

int   iSortOptions = 0;
int   iAlignMode   = 0;

bool      flagIsElevated = false;
WCHAR     wchWndClass[16] = WC_NOTEPAD3;


HINSTANCE g_hInstance = NULL;
HANDLE    g_hScintilla = NULL;
HANDLE    g_hwndEdit = NULL;

WCHAR     g_wchAppUserModelID[32] = { L'\0' };
WCHAR     g_wchWorkingDirectory[MAX_PATH+2] = { L'\0' };
WCHAR     g_wchCurFile[FILE_ARG_BUF] = { L'\0' };
FILEVARS  fvCurFile;
bool      g_bFileReadOnly = false;


// temporary line buffer for fast line ops 
static char g_pTempLineBufferMain[TEMPLINE_BUFFER];


// undo / redo  selections
static UT_icd UndoRedoSelection_icd = { sizeof(UndoRedoSelection_t), NULL, NULL, NULL };
static UT_array* UndoRedoSelectionUTArray = NULL;

static CLIPFORMAT cfDrpF = CF_HDROP;
static POINTL ptDummy = { 0, 0 };
static PDROPTARGET pDropTarget = NULL;
static DWORD DropFilesProc(CLIPFORMAT cf, HGLOBAL hData, HWND hWnd, DWORD dwKeyState, POINTL pt, void *pUserData);

//=============================================================================
//
//  IgnoreNotifyChangeEvent(), ObserveNotifyChangeEvent(), CheckNotifyChangeEvent()
//
static volatile LONG iNotifyChangeStackCounter = 0L;

bool CheckNotifyChangeEvent()
{
  return (InterlockedExchange(&iNotifyChangeStackCounter, iNotifyChangeStackCounter) == 0L);
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
      UpdateToolbar();
      UpdateStatusbar(false);
      UpdateLineNumberWidth();
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

static void __fastcall _MQ_AppendCmd(CmdMessageQueue_t* const pMsgQCmd, int cycles)
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

static void __fastcall _MQ_RemoveCmd(CmdMessageQueue_t* const pMsgQCmd)
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
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
//
// called by Timer(IDT_TIMER_MRKALL)
//
static void CALLBACK MQ_ExecuteNext(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
  UNUSED(hwnd);    // must be main wnd
  UNUSED(uMsg);    // must be WM_TIMER
  UNUSED(idEvent); // must be IDT_TIMER_MRKALL
  UNUSED(dwTime);  // This is the value returned by the GetTickCount function

  CmdMessageQueue_t* pmqc;

  DL_FOREACH(MessageQueue, pmqc) 
  {
    if (pmqc->delay == 0) {
      pmqc->delay = -1;
      SendMessage(pmqc->hwnd, pmqc->cmd, pmqc->wparam, pmqc->lparam);
    }
    else if (pmqc->delay >= 0) {
      pmqc->delay -= 1;
    }
  }
}


//=============================================================================
//
// Flags
//
int flagNoReuseWindow      = 0;
int flagReuseWindow        = 0;
int flagMultiFileArg       = 0;
int flagSingleFileInstance = 0;
int flagStartAsTrayIcon    = 0;
int flagAlwaysOnTop        = 0;
int flagRelativeFileMRU    = 0;
int flagPortableMyDocs     = 0;
int flagNoFadeHidden       = 0;
int flagToolbarLook        = 0;
int flagSimpleIndentGuides = 0;
int flagNoHTMLGuess        = 0;
int flagNoCGIGuess         = 0;
int flagNoFileVariables    = 0;
int flagPosParam           = 0;
int flagDefaultPos         = 0;
int flagNewFromClipboard   = 0;
int flagPasteBoard         = 0;
int flagSetEncoding        = 0;
int flagSetEOLMode         = 0;
int flagJumpTo             = 0;
int flagMatchText          = 0;
int flagChangeNotify       = 0;
int flagLexerSpecified     = 0;
int flagQuietCreate        = 0;
int flagUseSystemMRU       = 0;
int flagRelaunchElevated   = 0;
int flagDisplayHelp        = 0;
int flagPrintFileAndLeave  = 0;
int flagBufferFile         = 0;


//==============================================================================
//
//  Document Modified Flag
//
//
static bool IsDocumentModified = false;

static void __fastcall _SetDocumentModified(bool bModified)
{
  if (IsDocumentModified != bModified) {
    IsDocumentModified = bModified;
    UpdateToolbar();
  }
  if (bModified) {
    if (IsWindow(g_hwndDlgFindReplace)) {
      SendMessage(g_hwndDlgFindReplace, WM_COMMAND, MAKELONG(IDC_DOC_MODIFIED, 1), 0);
    }
  }
}


//=============================================================================
//
//  WinMain()
//
//

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInst,LPSTR lpCmdLine,int nCmdShow)
{

  MSG msg;
  HWND hwnd;
  HACCEL hAccMain;
  HACCEL hAccFindReplace;
  HACCEL hAccCoustomizeSchemes;
  INITCOMMONCONTROLSEX icex;
  //HMODULE hSciLexer;
  WCHAR wchAppDir[2*MAX_PATH+4] = { L'\0' };

  // Set global variable g_hInstance
  g_hInstance = hInstance;

  GetModuleFileName(NULL,wchAppDir,COUNTOF(wchAppDir));
  PathRemoveFileSpec(wchAppDir);
  PathCanonicalizeEx(wchAppDir,COUNTOF(wchAppDir));

  if (!GetCurrentDirectory(COUNTOF(g_wchWorkingDirectory),g_wchWorkingDirectory)) {
    StringCchCopy(g_wchWorkingDirectory,COUNTOF(g_wchWorkingDirectory),wchAppDir);
  }

  // Don't keep working directory locked
  SetCurrentDirectory(wchAppDir);

  SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);

  // check if running at least on Windows XP
  if (!IsXP()) {
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER|
        FORMAT_MESSAGE_FROM_SYSTEM|
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        ERROR_OLD_WIN_VERSION,
        MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT), // Default language
        (LPWSTR)&lpMsgBuf,
        0,
        NULL);
    MessageBox(NULL,(LPCWSTR)lpMsgBuf,L"Notepad3",MB_OK|MB_ICONEXCLAMATION);
    LocalFree(lpMsgBuf);
    return(0);
  }

  // Check if running with elevated privileges
  flagIsElevated = IsUserAdmin() || IsElevated();

  // Default Encodings (may already be used for command line parsing)
  Encoding_InitDefaults();

  // Command Line, Ini File and Flags
  ParseCommandLine();
  FindIniFile();
  TestIniFile();
  CreateIniFile();
  LoadFlags();

  // set AppUserModelID
  PrivateSetCurrentProcessExplicitAppUserModelID(g_wchAppUserModelID);

  // Command Line Help Dialog
  if (flagDisplayHelp) {
    DisplayCmdLineHelp(NULL);
    return(0);
  }

  // Adapt window class name
  if (flagIsElevated)
    StringCchCat(wchWndClass,COUNTOF(wchWndClass),L"U");
  if (flagPasteBoard)
    StringCchCat(wchWndClass,COUNTOF(wchWndClass),L"B");

  // Relaunch with elevated privileges
  if (RelaunchElevated(NULL))
    return(0);

  // Try to run multiple instances
  if (RelaunchMultiInst())
    return(0);

  // Try to activate another window
  if (ActivatePrevInst())
    return(0);

  // Init OLE and Common Controls
  OleInitialize(NULL);

  icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
  icex.dwICC  = ICC_WIN95_CLASSES|ICC_COOL_CLASSES|ICC_BAR_CLASSES|ICC_USEREX_CLASSES;
  InitCommonControlsEx(&icex);

  msgTaskbarCreated = RegisterWindowMessage(L"TaskbarCreated");

  if (!IsWin8()) {
    hModUxTheme = LoadLibrary(L"uxtheme.dll");
  }
  hRichEdit = LoadLibrary(L"RICHED20.DLL");  // Use "RichEdit20W" for control in .rc
  //hRichEdit = LoadLibrary(L"MSFTEDIT.DLL");  // Use "RichEdit50W" for control in .rc

  Scintilla_RegisterClasses(hInstance);

  // Load Settings
  LoadSettings();

  if (!InitApplication(hInstance))
    return false;
  
  hwnd = InitInstance(hInstance, lpCmdLine, nCmdShow);
  if (!hwnd)
    return false;
  
  // init DragnDrop handler
  DragAndDropInit(NULL);

  if (IsVista()) {
    // Current platforms perform window buffering so it is almost always better for this option to be turned off.
    // There are some older platforms and unusual modes where buffering may still be useful - so keep it ON
    //~SciCall_SetBufferedDraw(true);  // default is true 

    if (iSciDirectWriteTech >= 0) {
      SciCall_SetTechnology(DirectWriteTechnology[iSciDirectWriteTech]);
    }
  }

  hAccMain = LoadAccelerators(hInstance,MAKEINTRESOURCE(IDR_MAINWND));
  hAccFindReplace = LoadAccelerators(hInstance,MAKEINTRESOURCE(IDR_ACCFINDREPLACE));
  hAccCoustomizeSchemes = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCCUSTOMSCHEMES));

  UpdateLineNumberWidth();
  ObserveNotifyChangeEvent();
  
  SetTimer(hwnd, IDT_TIMER_MRKALL, USER_TIMER_MINIMUM, (TIMERPROC)MQ_ExecuteNext);
  
  while (GetMessage(&msg,NULL,0,0))
  {
    if (IsWindow(g_hwndDlgFindReplace) && ((msg.hwnd == g_hwndDlgFindReplace) || IsChild(g_hwndDlgFindReplace, msg.hwnd))) 
    {
      const int iTr = TranslateAccelerator(g_hwndDlgFindReplace, hAccFindReplace, &msg);
      if (iTr || IsDialogMessage(g_hwndDlgFindReplace, &msg))
        continue;
    }
    if (IsWindow(g_hwndDlgCustomizeSchemes) && ((msg.hwnd == g_hwndDlgCustomizeSchemes) || IsChild(g_hwndDlgCustomizeSchemes, msg.hwnd))) {
      const int iTr = TranslateAccelerator(g_hwndDlgCustomizeSchemes, hAccCoustomizeSchemes, &msg);
      if (iTr || IsDialogMessage(g_hwndDlgCustomizeSchemes, &msg))
        continue;
    }
    if (!TranslateAccelerator(hwnd,hAccMain,&msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  KillTimer(hwnd, IDT_TIMER_MRKALL);

  CmdMessageQueue_t* pmqc = NULL;
  CmdMessageQueue_t* dummy;
  DL_FOREACH_SAFE(MessageQueue, pmqc, dummy)
  {
    DL_DELETE(MessageQueue, pmqc);
    FreeMem(pmqc);
  }

  // Save Settings is done elsewhere

  Scintilla_ReleaseResources();
  UnregisterClass(wchWndClass,hInstance);

  if (hModUxTheme)
    FreeLibrary(hModUxTheme);

  OleUninitialize();

  UNUSED(hPrevInst);

  return(int)(msg.wParam);
}


//=============================================================================
//
//  InitApplication()
//
//
bool InitApplication(HINSTANCE hInstance)
{

  WNDCLASS   wc;

  wc.style         = CS_BYTEALIGNWINDOW | CS_DBLCLKS;
  wc.lpfnWndProc   = (WNDPROC)MainWndProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = hInstance;
  wc.hIcon         = LoadIcon(hInstance,MAKEINTRESOURCE(IDR_MAINWND));
  wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
  wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MAINWND);
  wc.lpszClassName = wchWndClass;

  return RegisterClass(&wc);

}



//=============================================================================
//
//  WaitCursorStack
//
//
static volatile LONG iWaitCursorStackCounter = 0L;

//=============================================================================
//
//  CheckWaitCursorStack()
//
//
static bool __fastcall CheckWaitCursorStack()
{
  return (InterlockedExchange(&iWaitCursorStackCounter, iWaitCursorStackCounter) == 0L);
}


//=============================================================================
//
//  BeginWaitCursor()
//
//
void BeginWaitCursor(LPCWSTR text)
{
  if (CheckWaitCursorStack())
  {
    SciCall_SetCursor(SC_CURSORWAIT); // delayed to SCN_DWELLSTART
    StatusSetText(g_hwndStatus, STATUS_HELP, (LPCWSTR)text);
    //StatusSetTextID(g_hwndStatus, STATUS_HELP, uid);
  }
  InterlockedIncrement(&iWaitCursorStackCounter);
}


//=============================================================================
//
//  EndWaitCursor()
//
//
void EndWaitCursor()
{
  if (!CheckWaitCursorStack()) {
    InterlockedDecrement(&iWaitCursorStackCounter);
  }
  if (CheckWaitCursorStack()) 
  {
    POINT pt;
    SciCall_SetCursor(SC_CURSORNORMAL);
    GetCursorPos(&pt); SetCursorPos(pt.x, pt.y);
    StatusSetSimple(g_hwndStatus, false);
    UpdateStatusbar(false);
  }
}


//=============================================================================
//
//  _InitWindowPosition()
//
//
static void __fastcall _InitWindowPosition(HWND hwnd)
{
  RECT rc;
  if (hwnd) {
    GetWindowRect(hwnd, &rc);
  }
  else {
    rc.left = g_WinInfo.x;  
    rc.top = g_WinInfo.y;  
    rc.right = g_WinInfo.x + g_WinInfo.cx;  
    rc.bottom = g_WinInfo.y + g_WinInfo.cy;
  }

  if (flagDefaultPos == 1) 
  {
    g_WinInfo.x = g_WinInfo.y = g_WinInfo.cx = g_WinInfo.cy = CW_USEDEFAULT;
    g_WinInfo.max = 0;
  }
  else if (flagDefaultPos >= 4) 
  {
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
    if (flagDefaultPos & 8)
      g_WinInfo.x = (rc.right - rc.left) / 2;
    else
      g_WinInfo.x = rc.left;
    g_WinInfo.cx = rc.right - rc.left;
    if (flagDefaultPos & (4 | 8))
      g_WinInfo.cx /= 2;
    if (flagDefaultPos & 32)
      g_WinInfo.y = (rc.bottom - rc.top) / 2;
    else
      g_WinInfo.y = rc.top;
    g_WinInfo.cy = rc.bottom - rc.top;
    if (flagDefaultPos & (16 | 32))
      g_WinInfo.cy /= 2;
    if (flagDefaultPos & 64) {
      g_WinInfo.x = rc.left;
      g_WinInfo.y = rc.top;
      g_WinInfo.cx = rc.right - rc.left;
      g_WinInfo.cy = rc.bottom - rc.top;
    }
    if (flagDefaultPos & 128) {
      g_WinInfo.x += (flagDefaultPos & 8) ? 4 : 8;
      g_WinInfo.cx -= (flagDefaultPos & (4 | 8)) ? 12 : 16;
      g_WinInfo.y += (flagDefaultPos & 32) ? 4 : 8;
      g_WinInfo.cy -= (flagDefaultPos & (16 | 32)) ? 12 : 16;
      g_WinInfo.max = 1;
    }
  }
  else if (flagDefaultPos == 2 || flagDefaultPos == 3) // NP3 default window position
  {
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
    g_WinInfo.y = rc.top + 16;
    g_WinInfo.cy = rc.bottom - rc.top - 32;
    g_WinInfo.cx = (rc.right - rc.left)/2; //min(rc.right - rc.left - 32, g_WinInfo.cy);
    g_WinInfo.x = (flagDefaultPos == 3) ? rc.left + 16 : rc.right - g_WinInfo.cx - 16;
  }
  else {  // fit window into working area of current monitor

    MONITORINFO mi;
    mi.cbSize = sizeof(mi);
    HMONITOR hMonitor = MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);
    GetMonitorInfo(hMonitor, &mi);
   
    g_WinInfo.x += (mi.rcWork.left - mi.rcMonitor.left);
    g_WinInfo.y += (mi.rcWork.top - mi.rcMonitor.top);
    if (g_WinInfo.x < mi.rcWork.left)
      g_WinInfo.x = mi.rcWork.left;
    if (g_WinInfo.y < mi.rcWork.top)
      g_WinInfo.y = mi.rcWork.top;
    if (g_WinInfo.x + g_WinInfo.cx > mi.rcWork.right) {
      g_WinInfo.x -= (g_WinInfo.x + g_WinInfo.cx - mi.rcWork.right);
      if (g_WinInfo.x < mi.rcWork.left)
        g_WinInfo.x = mi.rcWork.left;
      if (g_WinInfo.x + g_WinInfo.cx > mi.rcWork.right)
        g_WinInfo.cx = mi.rcWork.right - g_WinInfo.x;
    }
    if (g_WinInfo.y + g_WinInfo.cy > mi.rcWork.bottom) {
      g_WinInfo.y -= (g_WinInfo.y + g_WinInfo.cy - mi.rcWork.bottom);
      if (g_WinInfo.y < mi.rcWork.top)
        g_WinInfo.y = mi.rcWork.top;
      if (g_WinInfo.y + g_WinInfo.cy > mi.rcWork.bottom)
        g_WinInfo.cy = mi.rcWork.bottom - g_WinInfo.y;
    }
    SetRect(&rc, g_WinInfo.x, g_WinInfo.y, g_WinInfo.x + g_WinInfo.cx, g_WinInfo.y + g_WinInfo.cy);

    RECT rc2;
    if (!IntersectRect(&rc2, &rc, &mi.rcWork)) {
      g_WinInfo.y = mi.rcWork.top + 16;
      g_WinInfo.cy = mi.rcWork.bottom - mi.rcWork.top - 32;
      g_WinInfo.cx = min(mi.rcWork.right - mi.rcWork.left - 32, g_WinInfo.cy);
      g_WinInfo.x = mi.rcWork.right - g_WinInfo.cx - 16;
    }
  }
  g_WinCurrentWidth = g_WinInfo.cx;
}


//=============================================================================
//
//  InitInstance()
//
//
HWND InitInstance(HINSTANCE hInstance,LPSTR pszCmdLine,int nCmdShow)
{
  g_hwndMain = NULL;

  _InitWindowPosition(g_hwndMain);

  g_hwndMain = CreateWindowEx(
               0,
               wchWndClass,
               L"Notepad3",
               WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
               g_WinInfo.x,
               g_WinInfo.y,
               g_WinInfo.cx,
               g_WinInfo.cy,
               NULL,
               NULL,
               hInstance,
               NULL);

  if (g_WinInfo.max)
    nCmdShow = SW_SHOWMAXIMIZED;

  if ((bAlwaysOnTop || flagAlwaysOnTop == 2) && flagAlwaysOnTop != 1)
    SetWindowPos(g_hwndMain,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

  if (bTransparentMode)
    SetWindowTransparentMode(g_hwndMain,true);

  // Current file information -- moved in front of ShowWindow()
  FileLoad(true,true,false,bSkipUnicodeDetection,bSkipANSICodePageDetection,L"");

  if (!flagStartAsTrayIcon) {
    ShowWindow(g_hwndMain,nCmdShow);
    UpdateWindow(g_hwndMain);
  }
  else {
    ShowWindow(g_hwndMain,SW_HIDE);    // trick ShowWindow()
    ShowNotifyIcon(g_hwndMain,true);
  }

  // Source Encoding
  if (lpEncodingArg)
    Encoding_SrcCmdLn(Encoding_MatchW(lpEncodingArg));

  // Pathname parameter
  if (flagBufferFile || (lpFileArg /*&& !flagNewFromClipboard*/))
  {
    bool bOpened = false;

    // Open from Directory
    if (!flagBufferFile && PathIsDirectory(lpFileArg)) {
      WCHAR tchFile[MAX_PATH] = { L'\0' };
      if (OpenFileDlg(g_hwndMain, tchFile, COUNTOF(tchFile), lpFileArg))
        bOpened = FileLoad(false, false, false, bSkipUnicodeDetection, bSkipANSICodePageDetection, tchFile);
    }
    else {
      LPCWSTR lpFileToOpen = flagBufferFile ? g_szTmpFilePath : lpFileArg;
      bOpened = FileLoad(false, false, false, bSkipUnicodeDetection, bSkipANSICodePageDetection, lpFileToOpen);
      if (bOpened) {
        if (flagBufferFile) {
          if (lpFileArg) {
            InstallFileWatching(NULL); // Terminate file watching
            StringCchCopy(g_wchCurFile,COUNTOF(g_wchCurFile),lpFileArg);
            InstallFileWatching(g_wchCurFile);
          }
          else
            StringCchCopy(g_wchCurFile,COUNTOF(g_wchCurFile),L"");

          if (!flagLexerSpecified)
            Style_SetLexerFromFile(g_hwndEdit,g_wchCurFile);

          _SetDocumentModified(true);
          UpdateLineNumberWidth();

          // check for temp file and delete
          if (flagIsElevated && PathFileExists(g_szTmpFilePath)) {
            DeleteFile(g_szTmpFilePath);
          }
        }
        if (flagJumpTo) { // Jump to position
          EditJumpTo(g_hwndEdit,iInitialLine,iInitialColumn);
        }
      }
    }
    if (lpFileArg) {
      FreeMem(lpFileArg);
      lpFileArg = NULL;
    }
    if (bOpened) {
      if (flagChangeNotify == 1) {
        g_iFileWatchingMode = 0;
        g_bResetFileWatching = true;
        InstallFileWatching(g_wchCurFile);
      }
      else if (flagChangeNotify == 2) {
        if (!g_bChasingDocTail) { 
          SendMessage(g_hwndMain, WM_COMMAND, MAKELONG(IDM_VIEW_CHASING_DOCTAIL, 1), 0); 
        }
        else {
          g_iFileWatchingMode = 2;
          g_bResetFileWatching = true;
          InstallFileWatching(g_wchCurFile);
        }
      }
    }
  }
  else {
    if (Encoding_SrcCmdLn(CPI_GET) != CPI_NONE) {
      Encoding_Current(Encoding_SrcCmdLn(CPI_GET));
      Encoding_HasChanged(Encoding_SrcCmdLn(CPI_GET));
    }
  }

  // reset
  Encoding_SrcCmdLn(CPI_NONE);
  flagQuietCreate = 0;
  fKeepTitleExcerpt = 0;

  // undo / redo selections
  if (UndoRedoSelectionUTArray != NULL) {
    utarray_clear(UndoRedoSelectionUTArray);
    utarray_free(UndoRedoSelectionUTArray);
    UndoRedoSelectionUTArray = NULL;
  }
  utarray_new(UndoRedoSelectionUTArray, &UndoRedoSelection_icd);
  utarray_reserve(UndoRedoSelectionUTArray,256);

  // Check for /c [if no file is specified] -- even if a file is specified
  /*else */if (flagNewFromClipboard) {
    if (SendMessage(g_hwndEdit, SCI_CANPASTE, 0, 0)) {
      bool bAutoIndent2 = bAutoIndent;
      bAutoIndent = 0;
      EditJumpTo(g_hwndEdit, -1, 0);
      int token = BeginUndoAction();
      if (SendMessage(g_hwndEdit, SCI_GETLENGTH, 0, 0) > 0) {
        SendMessage(g_hwndEdit, SCI_NEWLINE, 0, 0);
      }
      SendMessage(g_hwndEdit, SCI_PASTE, 0, 0);
      SendMessage(g_hwndEdit, SCI_NEWLINE, 0, 0);
      EndUndoAction(token);
      bAutoIndent = bAutoIndent2;
      if (flagJumpTo)
        EditJumpTo(g_hwndEdit, iInitialLine, iInitialColumn);
      else
        EditEnsureSelectionVisible(g_hwndEdit);
    }
  }

  // Encoding
  if (0 != flagSetEncoding) {
    SendMessage(
      g_hwndMain,
      WM_COMMAND,
      MAKELONG(IDM_ENCODING_ANSI + flagSetEncoding -1,1),
      0);
    flagSetEncoding = 0;
  }

  // EOL mode
  if (0 != flagSetEOLMode) {
    SendMessage(
      g_hwndMain,
      WM_COMMAND,
      MAKELONG(IDM_LINEENDINGS_CRLF + flagSetEOLMode -1,1),
      0);
    flagSetEOLMode = 0;
  }

  // Match Text
  if (flagMatchText && lpMatchArg) {
    if (lstrlen(lpMatchArg) && SendMessage(g_hwndEdit,SCI_GETLENGTH,0,0)) {

      WideCharToMultiByteStrg(Encoding_SciCP,lpMatchArg,g_efrData.szFind);

      if (flagMatchText & 4)
        g_efrData.fuFlags |= (SCFIND_REGEXP | SCFIND_POSIX);
      else if (flagMatchText & 8)
        g_efrData.bTransformBS = true;

      if (flagMatchText & 2) {
        if (!flagJumpTo) { SendMessage(g_hwndEdit, SCI_DOCUMENTEND, 0, 0); }
        EditFindPrev(g_hwndEdit,&g_efrData,false,false);
        EditEnsureSelectionVisible(g_hwndEdit);
      }
      else {
        if (!flagJumpTo) { SendMessage(g_hwndEdit, SCI_DOCUMENTSTART, 0, 0); }
        EditFindNext(g_hwndEdit,&g_efrData,false,false);
        EditEnsureSelectionVisible(g_hwndEdit);
      }
    }
    LocalFree(lpMatchArg);
    lpMatchArg = NULL;
  }

  // Check for Paste Board option -- after loading files
  if (flagPasteBoard) {
    bLastCopyFromMe = true;
    hwndNextCBChain = SetClipboardViewer(g_hwndMain);
    uidsAppTitle = IDS_APPTITLE_PASTEBOARD;
    bLastCopyFromMe = false;

    dwLastCopyTime = 0;
    SetTimer(g_hwndMain,ID_PASTEBOARDTIMER,100,PasteBoardTimer);
  }

  // check if a lexer was specified from the command line
  if (flagLexerSpecified) {
    if (lpSchemeArg) {
      Style_SetLexerFromName(g_hwndEdit,g_wchCurFile,lpSchemeArg);
      LocalFree(lpSchemeArg);
    }
    else if (iInitialLexer >=0 && iInitialLexer < NUMLEXERS)
      Style_SetLexerFromID(g_hwndEdit,iInitialLexer);
    flagLexerSpecified = 0;
  }

  // If start as tray icon, set current filename as tooltip
  if (flagStartAsTrayIcon)
    SetNotifyIconTitle(g_hwndMain);

  iReplacedOccurrences = 0;
  g_iMarkOccurrencesCount = (g_iMarkOccurrences > 0) ? 0 : -1;

  UpdateToolbar();
  UpdateStatusbar(false);
  UpdateLineNumberWidth();

  // print file immediately and quit
  if (flagPrintFileAndLeave)
  {
    SHFILEINFO shfi;
    WCHAR *pszTitle;
    WCHAR tchUntitled[32] = { L'\0' };
    WCHAR tchPageFmt[32] = { L'\0' };

    if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
      SHGetFileInfo2(g_wchCurFile, FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES);
      pszTitle = shfi.szDisplayName;
    }
    else {
      GetString(IDS_UNTITLED, tchUntitled, COUNTOF(tchUntitled));
      pszTitle = tchUntitled;
    }

    GetString(IDS_PRINT_PAGENUM, tchPageFmt, COUNTOF(tchPageFmt));

    if (!EditPrint(g_hwndEdit, pszTitle, tchPageFmt))
      MsgBox(MBWARN, IDS_PRINT_ERROR, pszTitle);

    PostMessage(g_hwndMain, WM_CLOSE, 0, 0);
  }

  UNUSED(pszCmdLine);

  return(g_hwndMain);
}


//=============================================================================
//
//  MainWndProc()
//
//  Messages are distributed to the MsgXXX-handlers
//
//
LRESULT CALLBACK MainWndProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  static bool bAltKeyIsDown = false;

  switch(umsg)
  {
    // Quickly handle painting and sizing messages, found in ScintillaWin.cxx
    // Cool idea, don't know if this has any effect... ;-)
    case WM_MOVE:
    case WM_MOUSEACTIVATE:
    case WM_NCHITTEST:
    case WM_NCCALCSIZE:
    case WM_NCPAINT:
    case WM_PAINT:
    case WM_ERASEBKGND:
    case WM_NCMOUSEMOVE:
    case WM_NCLBUTTONDOWN:
    case WM_WINDOWPOSCHANGING:
    case WM_WINDOWPOSCHANGED:
    case WM_TIMER:
      return DefWindowProc(hwnd, umsg, wParam, lParam);

    case WM_SYSKEYDOWN:
      if (GetAsyncKeyState(VK_MENU) & SHRT_MIN)  // ALT-KEY DOWN
      {
        if (!bAltKeyIsDown) {
          bAltKeyIsDown = true;
          if (!bDenyVirtualSpaceAccess) {
            SciCall_SetVirtualSpaceOptions(SCVS_RECTANGULARSELECTION | SCVS_NOWRAPLINESTART | SCVS_USERACCESSIBLE);
          }
        }
      }
      return DefWindowProc(hwnd, umsg, wParam, lParam);

    case WM_SYSKEYUP:
      if (!(GetAsyncKeyState(VK_MENU) & SHRT_MIN))  // NOT ALT-KEY DOWN
      {
        if (bAltKeyIsDown) {
          bAltKeyIsDown = false;
          SciCall_SetVirtualSpaceOptions(bDenyVirtualSpaceAccess ? SCVS_NONE : SCVS_RECTANGULARSELECTION);
        }
      }
      return DefWindowProc(hwnd, umsg, wParam, lParam);


    case WM_CREATE:
      return MsgCreate(hwnd,wParam,lParam);

    case WM_DESTROY:
    case WM_ENDSESSION:
      MsgEndSession(hwnd,umsg);
      break;

    case WM_CLOSE:
      if (FileSave(false,true,false,false))
        DestroyWindow(hwnd);
      break;

    case WM_QUERYENDSESSION:
      if (FileSave(false,true,false,false))
        return true;
      else
        return false;

    // Reinitialize theme-dependent values and resize windows
    case WM_THEMECHANGED:
      MsgThemeChanged(hwnd,wParam,lParam);
      break;

    // update Scintilla colors
    case WM_SYSCOLORCHANGE:
      UpdateLineNumberWidth();
      EditClearAllOccurrenceMarkers(g_hwndEdit, 0, -1);
      MarkAllOccurrences(0);
      UpdateVisibleUrlHotspot(0);
      return DefWindowProc(hwnd,umsg,wParam,lParam);

    case WM_SIZE:
      MsgSize(hwnd,wParam,lParam);
      break;

    case WM_SETFOCUS:
      SetFocus(g_hwndEdit);
      //UpdateToolbar();
      //UpdateStatusbar(false);
      //UpdateLineNumberWidth();
      //if (bPendingChangeNotify)
      //  PostMessage(hwnd,WM_CHANGENOTIFY,0,0);
      break;

    case WM_DROPFILES:
      MsgDropFiles(hwnd, wParam, lParam);
      break;

    case WM_COPYDATA:
      return MsgCopyData(hwnd, wParam, lParam);

    case WM_CONTEXTMENU:
      return MsgContextMenu(hwnd, umsg, wParam, lParam);

    case WM_INITMENU:
      MsgInitMenu(hwnd,wParam,lParam);
      break;

    case WM_NOTIFY:
      return MsgNotify(hwnd,wParam,lParam);

    //case WM_PARENTNOTIFY:
    //  if (LOWORD(wParam) & WM_DESTROY) {
    //    if (IsWindow(hDlgFindReplace) && (hDlgFindReplace == (HWND)lParam)) {
    //      hDlgFindReplace = NULL;
    //    }
    //  }
    //  break;

    case WM_COMMAND:
      return MsgCommand(hwnd,wParam,lParam);

    case WM_SYSCOMMAND:
      return MsgSysCommand(hwnd, umsg, wParam, lParam);

    case WM_CHANGENOTIFY:
      MsgChangeNotify(hwnd, wParam, lParam);
      break;

    //// This message is posted before Notepad3 reactivates itself
    //case WM_CHANGENOTIFYCLEAR:
    //  bPendingChangeNotify = false;
    //  break;

    case WM_DRAWCLIPBOARD:
      if (!bLastCopyFromMe)
        dwLastCopyTime = GetTickCount();
      else
        bLastCopyFromMe = false;

      if (hwndNextCBChain)
        SendMessage(hwndNextCBChain,WM_DRAWCLIPBOARD,wParam,lParam);
      break;

    case WM_CHANGECBCHAIN:
      if ((HWND)wParam == hwndNextCBChain)
        hwndNextCBChain = (HWND)lParam;
      if (hwndNextCBChain)
        SendMessage(hwndNextCBChain,WM_CHANGECBCHAIN,lParam,wParam);
      break;

    case WM_TRAYMESSAGE:
      return MsgTrayMessage(hwnd, wParam, lParam);

    default:
      if (umsg == msgTaskbarCreated) {
        if (!IsWindowVisible(hwnd))
          ShowNotifyIcon(hwnd,true);
        SetNotifyIconTitle(hwnd);
      }
      return DefWindowProc(hwnd, umsg, wParam, lParam);
  }
  return 0; // swallow message
}



//=============================================================================
//
//  SetWordWrapping() - WordWrapSettings
//
static void __fastcall _SetWordWrapping(HWND hwndEditCtrl)
{
  // Word wrap
  if (g_bWordWrap)
    SendMessage(hwndEditCtrl, SCI_SETWRAPMODE, (iWordWrapMode == 0) ? SC_WRAP_WHITESPACE : SC_WRAP_CHAR, 0);
  else
    SendMessage(hwndEditCtrl, SCI_SETWRAPMODE, SC_WRAP_NONE, 0);

  if (iWordWrapIndent == 5)
    SendMessage(hwndEditCtrl, SCI_SETWRAPINDENTMODE, SC_WRAPINDENT_SAME, 0);
  else if (iWordWrapIndent == 6)
    SendMessage(hwndEditCtrl, SCI_SETWRAPINDENTMODE, SC_WRAPINDENT_INDENT, 0);
  else {
    int i = 0;
    switch (iWordWrapIndent) {
    case 1: i = 1; break;
    case 2: i = 2; break;
    case 3: i = (g_iIndentWidth) ? 1 * g_iIndentWidth : 1 * g_iTabWidth; break;
    case 4: i = (g_iIndentWidth) ? 2 * g_iIndentWidth : 2 * g_iTabWidth; break;
    }
    SendMessage(hwndEditCtrl, SCI_SETWRAPSTARTINDENT, i, 0);
    SendMessage(hwndEditCtrl, SCI_SETWRAPINDENTMODE, SC_WRAPINDENT_FIXED, 0);
  }

  if (bShowWordWrapSymbols) {
    int wrapVisualFlags = 0;
    int wrapVisualFlagsLocation = 0;
    if (iWordWrapSymbols == 0)
      iWordWrapSymbols = 22;
    switch (iWordWrapSymbols % 10) {
    case 1: wrapVisualFlags |= SC_WRAPVISUALFLAG_END; wrapVisualFlagsLocation |= SC_WRAPVISUALFLAGLOC_END_BY_TEXT; break;
    case 2: wrapVisualFlags |= SC_WRAPVISUALFLAG_END; break;
    }
    switch (((iWordWrapSymbols % 100) - (iWordWrapSymbols % 10)) / 10) {
    case 1: wrapVisualFlags |= SC_WRAPVISUALFLAG_START; wrapVisualFlagsLocation |= SC_WRAPVISUALFLAGLOC_START_BY_TEXT; break;
    case 2: wrapVisualFlags |= SC_WRAPVISUALFLAG_START; break;
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
static void __fastcall _InitializeSciEditCtrl(HWND hwndEditCtrl)
{
  Encoding_Current(g_iDefaultNewFileEncoding);

  // general setup
  SendMessage(hwndEditCtrl, SCI_SETCODEPAGE, (WPARAM)SC_CP_UTF8, 0); // fixed internal UTF-8 
  SendMessage(hwndEditCtrl, SCI_SETEOLMODE, SC_EOL_CRLF, 0);
  SendMessage(hwndEditCtrl, SCI_SETPASTECONVERTENDINGS, true, 0);
  SendMessage(hwndEditCtrl, SCI_SETMODEVENTMASK,/*SC_MODEVENTMASKALL*/SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT | SC_MOD_CONTAINER, 0);
  SendMessage(hwndEditCtrl, SCI_USEPOPUP, false, 0);
  SendMessage(hwndEditCtrl, SCI_SETSCROLLWIDTH, 1, 0);
  SendMessage(hwndEditCtrl, SCI_SETSCROLLWIDTHTRACKING, true, 0);
  SendMessage(hwndEditCtrl, SCI_SETENDATLASTLINE, true, 0);
  SendMessage(hwndEditCtrl, SCI_SETMOUSESELECTIONRECTANGULARSWITCH, true, 0);
  SendMessage(hwndEditCtrl, SCI_SETMULTIPLESELECTION, false, 0);
  SendMessage(hwndEditCtrl, SCI_SETADDITIONALSELECTIONTYPING, false, 0);
  SendMessage(hwndEditCtrl, SCI_SETADDITIONALCARETSBLINK, true, 0);
  SendMessage(hwndEditCtrl, SCI_SETADDITIONALCARETSVISIBLE, true, 0);
  SendMessage(hwndEditCtrl, SCI_SETVIRTUALSPACEOPTIONS, SCVS_NONE, 0);
  SendMessage(hwndEditCtrl, SCI_SETLAYOUTCACHE, SC_CACHE_PAGE, 0);

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
  SendMessage(hwndEditCtrl, SCI_INDICSETSTYLE, INDIC_NP3_MARK_OCCURANCE, INDIC_ROUNDBOX);
  SendMessage(hwndEditCtrl, SCI_INDICSETFORE, INDIC_NP3_MARK_OCCURANCE, RGB(0x00, 0x00, 0xFF));
  SendMessage(hwndEditCtrl, SCI_INDICSETALPHA, INDIC_NP3_MARK_OCCURANCE, 100);
  SendMessage(hwndEditCtrl, SCI_INDICSETOUTLINEALPHA, INDIC_NP3_MARK_OCCURANCE, 100);

  SendMessage(hwndEditCtrl, SCI_INDICSETSTYLE, INDIC_NP3_MATCH_BRACE, INDIC_FULLBOX);
  SendMessage(hwndEditCtrl, SCI_INDICSETFORE, INDIC_NP3_MATCH_BRACE, RGB(0x00, 0xFF, 0x00));
  SendMessage(hwndEditCtrl, SCI_INDICSETALPHA, INDIC_NP3_MATCH_BRACE, 120);
  SendMessage(hwndEditCtrl, SCI_INDICSETOUTLINEALPHA, INDIC_NP3_MATCH_BRACE, 120);

  SendMessage(hwndEditCtrl, SCI_INDICSETSTYLE, INDIC_NP3_BAD_BRACE, INDIC_FULLBOX);
  SendMessage(hwndEditCtrl, SCI_INDICSETFORE, INDIC_NP3_BAD_BRACE, RGB(0xFF, 0x00, 0x00));
  SendMessage(hwndEditCtrl, SCI_INDICSETALPHA, INDIC_NP3_BAD_BRACE, 120);
  SendMessage(hwndEditCtrl, SCI_INDICSETOUTLINEALPHA, INDIC_NP3_BAD_BRACE, 120);

  // paste into rectangular selection
  SendMessage(hwndEditCtrl, SCI_SETMULTIPASTE, SC_MULTIPASTE_EACH, 0);

  // No SC_AUTOMATICFOLD_CLICK, performed by 
  SendMessage(hwndEditCtrl, SCI_SETAUTOMATICFOLD, (WPARAM)(SC_AUTOMATICFOLD_SHOW | SC_AUTOMATICFOLD_CHANGE), 0);

  // Properties
  SendMessage(hwndEditCtrl, SCI_SETCARETSTICKY, SC_CARETSTICKY_OFF, 0);
  //SendMessage(hwndEditCtrl,SCI_SETCARETSTICKY,SC_CARETSTICKY_WHITESPACE,0);
  
  SendMessage(hwndEditCtrl, SCI_SETMOUSEDWELLTIME, SC_TIME_FOREVER, 0); // default
  //SendMessage(hwndEditCtrl, SCI_SETMOUSEDWELLTIME, (WPARAM)500, 0);
  

  #define _CARET_SYMETRY CARET_EVEN /// CARET_EVEN or 0
  if (iCurrentLineHorizontalSlop > 0)
    SendMessage(hwndEditCtrl, SCI_SETXCARETPOLICY, (WPARAM)(CARET_SLOP | _CARET_SYMETRY | CARET_STRICT), iCurrentLineHorizontalSlop);
  else
    SendMessage(hwndEditCtrl, SCI_SETXCARETPOLICY, (WPARAM)(CARET_SLOP | _CARET_SYMETRY | CARET_STRICT), (LPARAM)0);

  if (iCurrentLineVerticalSlop > 0)
    SendMessage(hwndEditCtrl, SCI_SETYCARETPOLICY, (WPARAM)(CARET_SLOP | _CARET_SYMETRY | CARET_STRICT), iCurrentLineVerticalSlop);
  else
    SendMessage(hwndEditCtrl, SCI_SETYCARETPOLICY, (WPARAM)(_CARET_SYMETRY), 0);

  SendMessage(hwndEditCtrl, SCI_SETVIRTUALSPACEOPTIONS, (WPARAM)(bDenyVirtualSpaceAccess ? SCVS_NONE : SCVS_RECTANGULARSELECTION), 0);
  SendMessage(hwndEditCtrl, SCI_SETENDATLASTLINE, (WPARAM)((bScrollPastEOF) ? 0 : 1), 0);

  // Tabs
  SendMessage(hwndEditCtrl, SCI_SETUSETABS, !g_bTabsAsSpaces, 0);
  SendMessage(hwndEditCtrl, SCI_SETTABINDENTS, g_bTabIndents, 0);
  SendMessage(hwndEditCtrl, SCI_SETBACKSPACEUNINDENTS, bBackspaceUnindents, 0);
  SendMessage(hwndEditCtrl, SCI_SETTABWIDTH, g_iTabWidth, 0);
  SendMessage(hwndEditCtrl, SCI_SETINDENT, g_iIndentWidth, 0);

  // Indent Guides
  Style_SetIndentGuides(hwndEditCtrl, bShowIndentGuides);

  // Word Wrap
  _SetWordWrapping(hwndEditCtrl);

  // Long Lines
  if (bMarkLongLines)
    SendMessage(hwndEditCtrl, SCI_SETEDGEMODE, (iLongLineMode == EDGE_LINE) ? EDGE_LINE : EDGE_BACKGROUND, 0);
  else
    SendMessage(hwndEditCtrl, SCI_SETEDGEMODE, EDGE_NONE, 0);

  SendMessage(hwndEditCtrl, SCI_SETEDGECOLUMN, g_iLongLinesLimit, 0);

  // general margin
  SendMessage(hwndEditCtrl, SCI_SETMARGINOPTIONS, SC_MARGINOPTION_SUBLINESELECT, 0);

  // Nonprinting characters
  SendMessage(hwndEditCtrl, SCI_SETVIEWWS, (bViewWhiteSpace) ? SCWS_VISIBLEALWAYS : SCWS_INVISIBLE, 0);
  SendMessage(hwndEditCtrl, SCI_SETVIEWEOL, bViewEOLs, 0);

  // word delimiter handling
  EditInitWordDelimiter(hwndEditCtrl);
  EditSetAccelWordNav(hwndEditCtrl, bAccelWordNavigation);

  // Init default values for printing
  EditPrintInit();

  //SciInitThemes(hwndEditCtrl);

  UpdateLineNumberWidth();
}


//=============================================================================
//
//  MsgCreate() - Handles WM_CREATE
//
//
LRESULT MsgCreate(HWND hwnd,WPARAM wParam,LPARAM lParam)
{

  HINSTANCE hInstance = ((LPCREATESTRUCT)lParam)->hInstance;

  // Setup edit control
  g_hwndEdit = CreateWindowEx(
    WS_EX_CLIENTEDGE,
    L"Scintilla",
    NULL,
    WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
    0, 0, 0, 0,
    hwnd,
    (HMENU)IDC_EDIT,
    hInstance,
    NULL);

  g_hScintilla = (HANDLE)SendMessage(g_hwndEdit, SCI_GETDIRECTPOINTER, 0, 0);

  _InitializeSciEditCtrl(g_hwndEdit);

  hwndEditFrame = CreateWindowEx(
                    WS_EX_CLIENTEDGE,
                    WC_LISTVIEW,
                    NULL,
                    WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
                    0,0,100,100,
                    hwnd,
                    (HMENU)IDC_EDITFRAME,
                    hInstance,
                    NULL);

  if (PrivateIsAppThemed()) {

    RECT rc, rc2;

    bIsAppThemed = true;

    SetWindowLongPtr(g_hwndEdit,GWL_EXSTYLE,GetWindowLongPtr(g_hwndEdit,GWL_EXSTYLE) & ~WS_EX_CLIENTEDGE);
    SetWindowPos(g_hwndEdit,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);

    if (IsVista()) {
      cxEditFrame = 0;
      cyEditFrame = 0;
    }

    else {
      GetClientRect(hwndEditFrame,&rc);
      GetWindowRect(hwndEditFrame,&rc2);

      cxEditFrame = ((rc2.right-rc2.left) - (rc.right-rc.left)) / 2;
      cyEditFrame = ((rc2.bottom-rc2.top) - (rc.bottom-rc.top)) / 2;
    }
  }
  else {
    bIsAppThemed = false;

    cxEditFrame = 0;
    cyEditFrame = 0;
  }

  // Create Toolbar and Statusbar
  CreateBars(hwnd,hInstance);

  // Window Initialization

  CreateWindow(
    WC_STATIC,
    NULL,
    WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
    0,0,10,10,
    hwnd,
    (HMENU)IDC_FILENAME,
    hInstance,
    NULL);

  SetDlgItemText(hwnd,IDC_FILENAME,g_wchCurFile);

  CreateWindow(
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
  pDropTarget = RegisterDragAndDrop(hwnd, &cfDrpF, 1, WM_NULL, DropFilesProc, (void*)g_hwndEdit);

  // File MRU
  g_pFileMRU = MRU_Create(L"Recent Files", MRU_NOCASE, MRU_ITEMSFILE);
  MRU_Load(g_pFileMRU);

  g_pMRUfind = MRU_Create(L"Recent Find", (/*IsWindowsNT()*/true) ? MRU_UTF8 : 0, MRU_ITEMSFNDRPL);
  MRU_Load(g_pMRUfind);
  SetFindPattern(g_pMRUfind->pszItems[0]);

  g_pMRUreplace = MRU_Create(L"Recent Replace", (/*IsWindowsNT()*/true) ? MRU_UTF8 : 0, MRU_ITEMSFNDRPL);
  MRU_Load(g_pMRUreplace);

  if (g_hwndEdit == NULL || hwndEditFrame == NULL ||
      g_hwndStatus == NULL || g_hwndToolbar == NULL || g_hwndReBar == NULL)
    return(-1);

  UNUSED(wParam);
  return(0);
}


//=============================================================================
//
//  CreateBars() - Create Toolbar and Statusbar
//
//
void CreateBars(HWND hwnd,HINSTANCE hInstance)
{
  RECT rc;

  REBARINFO rbi;
  REBARBANDINFO rbBand;

  BITMAP bmp;
  HBITMAP hbmp, hbmpCopy = NULL;
  HIMAGELIST himl;
  WCHAR szTmp[MAX_PATH] = { L'\0' };

  DWORD dwToolbarStyle = WS_TOOLBAR;
  DWORD dwReBarStyle = WS_REBAR;

  bool bIsPrivAppThemed = PrivateIsAppThemed();

  int i,n;
  WCHAR tchDesc[256] = { L'\0' };
  WCHAR tchIndex[256] = { L'\0' };

  WCHAR *pIniSection = NULL;
  int   cchIniSection = 0;

  if (bShowToolbar)
    dwReBarStyle |= WS_VISIBLE;

  g_hwndToolbar = CreateWindowEx(0,TOOLBARCLASSNAME,NULL,dwToolbarStyle,
                               0,0,0,0,hwnd,(HMENU)IDC_TOOLBAR,hInstance,NULL);

  SendMessage(g_hwndToolbar,TB_BUTTONSTRUCTSIZE,(WPARAM)sizeof(TBBUTTON),0);

  // Add normal Toolbar Bitmap
  hbmp = NULL;
  if (StringCchLenW(g_tchToolbarBitmap,COUNTOF(g_tchToolbarBitmap)))
  {
    if (!SearchPath(NULL,g_tchToolbarBitmap,L".bmp",COUNTOF(szTmp),szTmp,NULL))
      StringCchCopy(szTmp,COUNTOF(szTmp),g_tchToolbarBitmap);
    hbmp = LoadImage(NULL,szTmp,IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION|LR_LOADFROMFILE);
  }

  if (hbmp) {
    g_bExternalBitmap = true;
  }
  else {
    LPWSTR toolBarIntRes = (iHighDpiToolBar > 0) ? MAKEINTRESOURCE(IDR_MAINWNDTB2) : MAKEINTRESOURCE(IDR_MAINWNDTB);
    hbmp = LoadImage(hInstance, toolBarIntRes, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    hbmpCopy = CopyImage(hbmp, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
  }
  GetObject(hbmp,sizeof(BITMAP),&bmp);
  if (!IsXP())
    BitmapMergeAlpha(hbmp,GetSysColor(COLOR_3DFACE));
  himl = ImageList_Create(bmp.bmWidth/NUMTOOLBITMAPS,bmp.bmHeight,ILC_COLOR32|ILC_MASK,0,0);
  ImageList_AddMasked(himl,hbmp,CLR_DEFAULT);
  DeleteObject(hbmp);
  SendMessage(g_hwndToolbar,TB_SETIMAGELIST,0,(LPARAM)himl);

  // Optionally add hot Toolbar Bitmap
  hbmp = NULL;
  if (StringCchLenW(g_tchToolbarBitmapHot,COUNTOF(g_tchToolbarBitmapHot)))
  {
    if (!SearchPath(NULL,g_tchToolbarBitmapHot,L".bmp",COUNTOF(szTmp),szTmp,NULL))
      StringCchCopy(szTmp,COUNTOF(szTmp),g_tchToolbarBitmapHot);

    hbmp = LoadImage(NULL, szTmp, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
    if (hbmp)
    {
      GetObject(hbmp,sizeof(BITMAP),&bmp);
      himl = ImageList_Create(bmp.bmWidth/NUMTOOLBITMAPS,bmp.bmHeight,ILC_COLOR32|ILC_MASK,0,0);
      ImageList_AddMasked(himl,hbmp,CLR_DEFAULT);
      DeleteObject(hbmp);
      SendMessage(g_hwndToolbar,TB_SETHOTIMAGELIST,0,(LPARAM)himl);
    }
  }

  // Optionally add disabled Toolbar Bitmap
  hbmp = NULL;
  if (StringCchLenW(g_tchToolbarBitmapDisabled,COUNTOF(g_tchToolbarBitmapDisabled)))
  {
    if (!SearchPath(NULL,g_tchToolbarBitmapDisabled,L".bmp",COUNTOF(szTmp),szTmp,NULL))
      StringCchCopy(szTmp,COUNTOF(szTmp),g_tchToolbarBitmapDisabled);

    hbmp = LoadImage(NULL, szTmp, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
    if (hbmp)
    {
      GetObject(hbmp,sizeof(BITMAP),&bmp);
      himl = ImageList_Create(bmp.bmWidth/NUMTOOLBITMAPS,bmp.bmHeight,ILC_COLOR32|ILC_MASK,0,0);
      ImageList_AddMasked(himl,hbmp,CLR_DEFAULT);
      DeleteObject(hbmp);
      SendMessage(g_hwndToolbar,TB_SETDISABLEDIMAGELIST,0,(LPARAM)himl);
      g_bExternalBitmap = true;
    }
  }

  if (!g_bExternalBitmap) {
    bool fProcessed = false;
    if (flagToolbarLook == 1)
      fProcessed = BitmapAlphaBlend(hbmpCopy,GetSysColor(COLOR_3DFACE),0x60);
    else if (flagToolbarLook == 2 || (!IsXP() && flagToolbarLook == 0))
      fProcessed = BitmapGrayScale(hbmpCopy);
    if (fProcessed && !IsXP())
      BitmapMergeAlpha(hbmpCopy,GetSysColor(COLOR_3DFACE));
    if (fProcessed) {
      himl = ImageList_Create(bmp.bmWidth/NUMTOOLBITMAPS,bmp.bmHeight,ILC_COLOR32|ILC_MASK,0,0);
      ImageList_AddMasked(himl,hbmpCopy,CLR_DEFAULT);
      SendMessage(g_hwndToolbar,TB_SETDISABLEDIMAGELIST,0,(LPARAM)himl);
    }
  }
  if (hbmpCopy)
    DeleteObject(hbmpCopy);

  // Load toolbar labels
  pIniSection = LocalAlloc(LPTR,sizeof(WCHAR) * 32 * 1024);
  cchIniSection = (int)LocalSize(pIniSection)/sizeof(WCHAR);
  LoadIniSection(L"Toolbar Labels",pIniSection,cchIniSection);
  for (i = 0; i < COUNTOF(tbbMainWnd); i++) {

    if (tbbMainWnd[i].fsStyle == TBSTYLE_SEP)
      continue;

    n = tbbMainWnd[i].iBitmap + 1;
    StringCchPrintf(tchIndex,COUNTOF(tchIndex),L"%02i",n);
    if (IniSectionGetString(pIniSection,tchIndex,L"",tchDesc,COUNTOF(tchDesc))) 
    {
      tbbMainWnd[i].iString = SendMessage(g_hwndToolbar,TB_ADDSTRING,0,(LPARAM)tchDesc);
      tbbMainWnd[i].fsStyle |= BTNS_AUTOSIZE | BTNS_SHOWTEXT;
    }
    else {
      tbbMainWnd[i].fsStyle &= ~(BTNS_AUTOSIZE | BTNS_SHOWTEXT);
    }
  }
  LocalFree(pIniSection);

  SendMessage(g_hwndToolbar,TB_SETEXTENDEDSTYLE,0,
    SendMessage(g_hwndToolbar,TB_GETEXTENDEDSTYLE,0,0) | TBSTYLE_EX_MIXEDBUTTONS);

  SendMessage(g_hwndToolbar,TB_ADDBUTTONS,NUMINITIALTOOLS,(LPARAM)tbbMainWnd);

  if (Toolbar_SetButtons(g_hwndToolbar, IDT_FILE_NEW, g_tchToolbarButtons, tbbMainWnd, COUNTOF(tbbMainWnd)) == 0) {
    SendMessage(g_hwndToolbar, TB_ADDBUTTONS, NUMINITIALTOOLS, (LPARAM)tbbMainWnd);
  }
  SendMessage(g_hwndToolbar,TB_GETITEMRECT,0,(LPARAM)&rc);
  //SendMessage(g_hwndToolbar,TB_SETINDENT,2,0);

  DWORD dwStatusbarStyle = WS_CHILD | WS_CLIPSIBLINGS;

  if (bShowStatusbar)
    dwStatusbarStyle |= WS_VISIBLE;

  g_hwndStatus = CreateStatusWindow(dwStatusbarStyle,NULL,hwnd,IDC_STATUSBAR);

  // Create ReBar and add Toolbar
  g_hwndReBar = CreateWindowEx(WS_EX_TOOLWINDOW,REBARCLASSNAME,NULL,dwReBarStyle,
                             0,0,0,0,hwnd,(HMENU)IDC_REBAR,hInstance,NULL);

  rbi.cbSize = sizeof(REBARINFO);
  rbi.fMask  = 0;
  rbi.himl   = (HIMAGELIST)NULL;
  SendMessage(g_hwndReBar,RB_SETBARINFO,0,(LPARAM)&rbi);

  rbBand.cbSize  = sizeof(REBARBANDINFO);
  rbBand.fMask   = /*RBBIM_COLORS | RBBIM_TEXT | RBBIM_BACKGROUND | */
                   RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE /*| RBBIM_SIZE*/;
  rbBand.fStyle  = /*RBBS_CHILDEDGE |*//* RBBS_BREAK |*/ RBBS_FIXEDSIZE /*| RBBS_GRIPPERALWAYS*/;
  if (bIsPrivAppThemed)
    rbBand.fStyle |= RBBS_CHILDEDGE;
  rbBand.hbmBack = NULL;
  rbBand.lpText     = L"Toolbar";
  rbBand.hwndChild  = g_hwndToolbar;
  rbBand.cxMinChild = (rc.right - rc.left) * COUNTOF(tbbMainWnd);
  rbBand.cyMinChild = (rc.bottom - rc.top) + 2 * rc.top;
  rbBand.cx         = 0;
  SendMessage(g_hwndReBar,RB_INSERTBAND,(WPARAM)-1,(LPARAM)&rbBand);

  SetWindowPos(g_hwndReBar,NULL,0,0,0,0,SWP_NOZORDER);
  GetWindowRect(g_hwndReBar,&rc);
  cyReBar = rc.bottom - rc.top;

  cyReBarFrame = bIsPrivAppThemed ? 0 : 2;
}


//=============================================================================
//
//  MsgEndSession() - Handle WM_ENDSESSION,WM_DESTROY
//
//
void MsgEndSession(HWND hwnd, UINT umsg)
{
  static bool bShutdownOK = false;

  if (!bShutdownOK) {

    // Terminate file watching
    InstallFileWatching(NULL);

    // GetWindowPlacement
    g_WinInfo = GetMyWindowPlacement(hwnd, NULL);

    DragAcceptFiles(hwnd, false);
    RevokeDragAndDrop(pDropTarget);

    // Terminate clipboard watching
    if (flagPasteBoard) {
      KillTimer(hwnd, ID_PASTEBOARDTIMER);
      ChangeClipboardChain(hwnd, hwndNextCBChain);
    }

    // Destroy find / replace dialog
    if (IsWindow(g_hwndDlgFindReplace))
      DestroyWindow(g_hwndDlgFindReplace);

    // Destroy customize schemes
    if (IsWindow(g_hwndDlgCustomizeSchemes))
      DestroyWindow(g_hwndDlgCustomizeSchemes);

    // call SaveSettings() when g_hwndToolbar is still valid
    SaveSettings(false);

    if (StringCchLenW(g_wchIniFile,COUNTOF(g_wchIniFile)) != 0) {

      // Cleanup unwanted MRU's
      if (!bSaveRecentFiles) {
        MRU_Empty(g_pFileMRU);
        MRU_Save(g_pFileMRU);
      }
      else
        MRU_MergeSave(g_pFileMRU, true, flagRelativeFileMRU, flagPortableMyDocs);

      MRU_Destroy(g_pFileMRU);

      if (!bSaveFindReplace) {
        MRU_Empty(g_pMRUfind);
        MRU_Empty(g_pMRUreplace);
        MRU_Save(g_pMRUfind);
        MRU_Save(g_pMRUreplace);
      }
      else {
        MRU_MergeSave(g_pMRUfind, false, false, false);
        MRU_MergeSave(g_pMRUreplace, false, false, false);
      }
      MRU_Destroy(g_pMRUfind);
      MRU_Destroy(g_pMRUreplace);
    }

    // Remove tray icon if necessary
    ShowNotifyIcon(hwnd, false);

    bShutdownOK = true;
  }

  if (umsg == WM_DESTROY)
    PostQuitMessage(0);
}


//=============================================================================
//
//  MsgThemeChanged() - Handle WM_THEMECHANGED
//
//
void MsgThemeChanged(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
  UNUSED(lParam);
  UNUSED(wParam);
  UNUSED(hwnd);
  
  RECT rc, rc2;
  HINSTANCE hInstance = (HINSTANCE)(INT_PTR)GetWindowLongPtr(hwnd,GWLP_HINSTANCE);

  // reinitialize edit frame

  if (PrivateIsAppThemed()) {
    bIsAppThemed = true;

    SetWindowLongPtr(g_hwndEdit,GWL_EXSTYLE,GetWindowLongPtr(g_hwndEdit,GWL_EXSTYLE) & ~WS_EX_CLIENTEDGE);
    SetWindowPos(g_hwndEdit,NULL,0,0,0,0,SWP_NOZORDER|SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE);

    if (IsVista()) {
      cxEditFrame = 0;
      cyEditFrame = 0;
    }

    else {
      SetWindowPos(hwndEditFrame,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
      GetClientRect(hwndEditFrame,&rc);
      GetWindowRect(hwndEditFrame,&rc2);

      cxEditFrame = ((rc2.right-rc2.left) - (rc.right-rc.left)) / 2;
      cyEditFrame = ((rc2.bottom-rc2.top) - (rc.bottom-rc.top)) / 2;
    }
  }

  else {
    bIsAppThemed = false;

    SetWindowLongPtr(g_hwndEdit,GWL_EXSTYLE,WS_EX_CLIENTEDGE|GetWindowLongPtr(g_hwndEdit,GWL_EXSTYLE));
    SetWindowPos(g_hwndEdit,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);

    cxEditFrame = 0;
    cyEditFrame = 0;
  }

  // recreate toolbar and statusbar
  Toolbar_GetButtons(g_hwndToolbar,IDT_FILE_NEW,g_tchToolbarButtons,COUNTOF(g_tchToolbarButtons));

  DestroyWindow(g_hwndToolbar);
  DestroyWindow(g_hwndReBar);
  DestroyWindow(g_hwndStatus);
  CreateBars(hwnd,hInstance);

  SendWMSize(hwnd);

  EditFinalizeStyling(g_hwndEdit, -1);

  if (EditToggleView(g_hwndEdit, false)) {
    EditToggleView(g_hwndEdit, true);
  }
  EditClearAllOccurrenceMarkers(g_hwndEdit, 0, -1);
  MarkAllOccurrences(0);
  EditUpdateUrlHotspots(g_hwndEdit, 0, SciCall_GetTextLength(), g_bHyperlinkHotspot);

  UpdateUI();
  UpdateToolbar();
  UpdateStatusbar(true);
  UpdateLineNumberWidth();
}


//=============================================================================
//
//  MsgSize() - Handles WM_SIZE
//
//
void MsgSize(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
  if (wParam == SIZE_MINIMIZED)
    return;

  int x = 0;
  int y = 0;

  int cx = LOWORD(lParam);
  int cy = HIWORD(lParam);

  if (bShowToolbar)
  {
/*  SendMessage(g_hwndToolbar,WM_SIZE,0,0);
    RECT rc;
    GetWindowRect(g_hwndToolbar,&rc);
    y = (rc.bottom - rc.top);
    cy -= (rc.bottom - rc.top);*/

    //SendMessage(g_hwndToolbar,TB_GETITEMRECT,0,(LPARAM)&rc);
    SetWindowPos(g_hwndReBar,NULL,0,0,LOWORD(lParam),cyReBar,SWP_NOZORDER);
    // the ReBar automatically sets the correct height
    // calling SetWindowPos() with the height of one toolbar button
    // causes the control not to temporarily use the whole client area
    // and prevents flickering

    //GetWindowRect(g_hwndReBar,&rc);
    y = cyReBar + cyReBarFrame;    // define
    cy -= cyReBar + cyReBarFrame;  // border
  }

  if (bShowStatusbar)
  {
    RECT rc;
    SendMessage(g_hwndStatus,WM_SIZE,0,0);
    GetWindowRect(g_hwndStatus,&rc);
    cy -= (rc.bottom - rc.top);
  }

  HDWP hdwp = BeginDeferWindowPos(2);

  DeferWindowPos(hdwp,hwndEditFrame,NULL,x,y,cx,cy,
                 SWP_NOZORDER | SWP_NOACTIVATE);

  DeferWindowPos(hdwp,g_hwndEdit,NULL,x+cxEditFrame,y+cyEditFrame,
                 cx-2*cxEditFrame,cy-2*cyEditFrame,
                 SWP_NOZORDER | SWP_NOACTIVATE);

  EndDeferWindowPos(hdwp);

  g_WinCurrentWidth = cx;

  UpdateToolbar();
  UpdateStatusbar(false);
  UpdateLineNumberWidth();

  UNUSED(hwnd);
  UNUSED(lParam);
}



//=============================================================================
//
//  MsgDropFiles() - Handles WM_DROPFILES
//
//
void MsgDropFiles(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  WCHAR szBuf[MAX_PATH + 40];
  HDROP hDrop = (HDROP)wParam;

  // Reset Change Notify
  //bPendingChangeNotify = false;

  if (IsIconic(hwnd))
    ShowWindow(hwnd, SW_RESTORE);

  //SetForegroundWindow(hwnd);

  DragQueryFile(hDrop, 0, szBuf, COUNTOF(szBuf));

  if (PathIsDirectory(szBuf)) {
    WCHAR tchFile[MAX_PATH] = { L'\0' };
    if (OpenFileDlg(g_hwndMain, tchFile, COUNTOF(tchFile), szBuf))
      FileLoad(false, false, false, bSkipUnicodeDetection, bSkipANSICodePageDetection, tchFile);
  }
  else if (PathFileExists(szBuf))
    FileLoad(false, false, false, bSkipUnicodeDetection, bSkipANSICodePageDetection, szBuf);
  else
    // Windows Bug: wParam (HDROP) pointer is corrupted if dropped from 32-bit App
    MsgBox(MBWARN, IDS_DROP_NO_FILE);

  if (DragQueryFile(hDrop, (UINT)(-1), NULL, 0) > 1)
    MsgBox(MBWARN, IDS_ERR_DROP);

  DragFinish(hDrop);

  UNUSED(lParam);
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
        FileLoad(false, false, false, bSkipUnicodeDetection, bSkipANSICodePageDetection, tchFile);
    }
    else
      FileLoad(false, false, false, bSkipUnicodeDetection, bSkipANSICodePageDetection, szBuf);

    if (DragQueryFile(hDrop, (UINT)(-1), NULL, 0) > 1)
      MsgBox(MBWARN, IDS_ERR_DROP);

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
  PCOPYDATASTRUCT pcds = (PCOPYDATASTRUCT)lParam;

  // Reset Change Notify
  //bPendingChangeNotify = false;

  SetDlgItemInt(hwnd, IDC_REUSELOCK, GetTickCount(), false);

  if (pcds->dwData == DATA_NOTEPAD3_PARAMS) {
    LPnp3params params = LocalAlloc(LPTR, pcds->cbData);
    CopyMemory(params, pcds->lpData, pcds->cbData);

    if (params->flagLexerSpecified)
      flagLexerSpecified = 1;

    if (params->flagQuietCreate)
      flagQuietCreate = 1;

    if (params->flagFileSpecified) {

      bool bOpened = false;
      Encoding_SrcCmdLn(params->iSrcEncoding);

      if (PathIsDirectory(&params->wchData)) {
        WCHAR tchFile[MAX_PATH] = { L'\0' };
        if (OpenFileDlg(g_hwndMain, tchFile, COUNTOF(tchFile), &params->wchData))
          bOpened = FileLoad(false, false, false, bSkipUnicodeDetection, bSkipANSICodePageDetection, tchFile);
      }

      else
        bOpened = FileLoad(false, false, false, bSkipUnicodeDetection, bSkipANSICodePageDetection, &params->wchData);

      if (bOpened) {

        if (params->flagChangeNotify == 1) {
          g_iFileWatchingMode = 0;
          g_bResetFileWatching = true;
          InstallFileWatching(g_wchCurFile);
        }
        else if (params->flagChangeNotify == 2) {
          if (!g_bChasingDocTail) {
            SendMessage(g_hwndMain, WM_COMMAND, MAKELONG(IDM_VIEW_CHASING_DOCTAIL, 1), 0);
          }
          else {
            g_iFileWatchingMode = 2;
            g_bResetFileWatching = true;
            InstallFileWatching(g_wchCurFile);
          }
        }

        if (0 != params->flagSetEncoding) {
          flagSetEncoding = params->flagSetEncoding;
          SendMessage(
            hwnd,
            WM_COMMAND,
            MAKELONG(IDM_ENCODING_ANSI + flagSetEncoding - 1, 1),
            0);
          flagSetEncoding = 0;
        }

        if (0 != params->flagSetEOLMode) {
          flagSetEOLMode = params->flagSetEOLMode;
          SendMessage(g_hwndMain, WM_COMMAND, MAKELONG(IDM_LINEENDINGS_CRLF + flagSetEOLMode - 1, 1), 0);
          flagSetEOLMode = 0;
        }

        if (params->flagLexerSpecified) {
          if (params->iInitialLexer < 0) {
            WCHAR wchExt[32] = L".";
            StringCchCopyN(CharNext(wchExt), 32, StrEnd(&params->wchData) + 1, 31);
            Style_SetLexerFromName(g_hwndEdit, &params->wchData, wchExt);
          }
          else if (params->iInitialLexer >= 0 && params->iInitialLexer < NUMLEXERS)
            Style_SetLexerFromID(g_hwndEdit, params->iInitialLexer);
        }

        if (params->flagTitleExcerpt) {
          StringCchCopyN(szTitleExcerpt, COUNTOF(szTitleExcerpt), StrEnd(&params->wchData) + 1, COUNTOF(szTitleExcerpt));
        }
      }
      // reset
      Encoding_SrcCmdLn(CPI_NONE);
    }

    if (params->flagJumpTo) {
      if (params->iInitialLine == 0)
        params->iInitialLine = 1;
      EditJumpTo(g_hwndEdit, params->iInitialLine, params->iInitialColumn);
    }

    flagLexerSpecified = 0;
    flagQuietCreate = 0;

    LocalFree(params);

    UpdateToolbar();
    UpdateStatusbar(false);
    UpdateLineNumberWidth();

  }

  UNUSED(wParam);
  return true;
}

//=============================================================================
//
//  MsgContextMenu() - Handles WM_CONTEXTMENU
//
//
LRESULT MsgContextMenu(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  HMENU hmenu;
  int imenu = 0;
  POINT pt;
  int nID = GetDlgCtrlID((HWND)wParam);

  if ((nID != IDC_EDIT) && (nID != IDC_STATUSBAR) &&
    (nID != IDC_REBAR) && (nID != IDC_TOOLBAR))
    return DefWindowProc(hwnd, umsg, wParam, lParam);

  hmenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_POPUPMENU));
  //SetMenuDefaultItem(GetSubMenu(hmenu,1),0,false);

  pt.x = (int)(short)LOWORD(lParam);
  pt.y = (int)(short)HIWORD(lParam);

  switch (nID) {
  case IDC_EDIT:
    {
      if (SendMessage(g_hwndEdit, SCI_GETSELECTIONEMPTY, 0, 0) && (pt.x != -1) && (pt.y != -1)) {
        DocPos iNewPos;
        POINT ptc;
        ptc.x = pt.x;  ptc.y = pt.y;
        ScreenToClient(g_hwndEdit, &ptc);
        iNewPos = (DocPos)SendMessage(g_hwndEdit, SCI_POSITIONFROMPOINT, (WPARAM)ptc.x, (LPARAM)ptc.y);
        EditSelectEx(g_hwndEdit, iNewPos, iNewPos, -1, -1);
      }

      if (pt.x == -1 && pt.y == -1) {
        DocPos iCurrentPos = (DocPos)SendMessage(g_hwndEdit, SCI_GETCURRENTPOS, 0, 0);
        pt.x = (LONG)SendMessage(g_hwndEdit, SCI_POINTXFROMPOSITION, 0, (LPARAM)iCurrentPos);
        pt.y = (LONG)SendMessage(g_hwndEdit, SCI_POINTYFROMPOSITION, 0, (LPARAM)iCurrentPos);
        ClientToScreen(g_hwndEdit, &pt);
      }
      imenu = 0;
    }
    break;

  case IDC_TOOLBAR:
  case IDC_STATUSBAR:
  case IDC_REBAR:
    if (pt.x == -1 && pt.y == -1)
      GetCursorPos(&pt);
    imenu = 1;
    break;
  }

  TrackPopupMenuEx(GetSubMenu(hmenu, imenu),
                   TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x + 1, pt.y + 1, hwnd, NULL);

  DestroyMenu(hmenu);
  return 0;
}



//=============================================================================
//
//  MsgChangeNotify() - Handles WM_CHANGENOTIFY
//
//
void MsgChangeNotify(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  if (g_iFileWatchingMode == 1 || IsDocumentModified || Encoding_HasChanged(CPI_GET)) {
    SetForegroundWindow(hwnd);
  }

  if (PathFileExists(g_wchCurFile)) 
  {
    if ((g_iFileWatchingMode == 2 && !IsDocumentModified && !Encoding_HasChanged(CPI_GET)) ||
      MsgBox(MBYESNOWARN,IDS_FILECHANGENOTIFY) == IDYES) 
    {
      FileRevert(g_wchCurFile);
      
      if (g_bChasingDocTail) 
      {
        SciCall_SetReadOnly(g_bChasingDocTail);
        //SetForegroundWindow(hwnd);
        SciCall_ScrollToEnd(); 
      }
    }
  }
  else {
    if (MsgBox(MBYESNOWARN, IDS_FILECHANGENOTIFY2) == IDYES) {
      FileSave(true, false, false, false);
    }
  }

  if (!g_bRunningWatch) {
    InstallFileWatching(g_wchCurFile);
  }

  UNUSED(wParam);
  UNUSED(lParam);
}


//=============================================================================
//
//  MsgTrayMessage() - Handles WM_TRAYMESSAGE
//
//
LRESULT MsgTrayMessage(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  switch (lParam) {
  case WM_RBUTTONUP:
    {

      HMENU hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_POPUPMENU));
      HMENU hMenuPopup = GetSubMenu(hMenu, 2);

      POINT pt;
      int iCmd;

      SetForegroundWindow(hwnd);

      GetCursorPos(&pt);
      SetMenuDefaultItem(hMenuPopup, IDM_TRAY_RESTORE, false);
      iCmd = TrackPopupMenu(hMenuPopup,
                            TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
                            pt.x, pt.y, 0, hwnd, NULL);

      PostMessage(hwnd, WM_NULL, 0, 0);

      DestroyMenu(hMenu);

      if (iCmd == IDM_TRAY_RESTORE) {
        ShowNotifyIcon(hwnd, false);
        RestoreWndFromTray(hwnd);
        ShowOwnedPopups(hwnd, true);
      }

      else if (iCmd == IDM_TRAY_EXIT) {
        //ShowNotifyIcon(hwnd,false);
        SendMessage(hwnd, WM_CLOSE, 0, 0);
      }
    }
    return true;

  case WM_LBUTTONUP:
    ShowNotifyIcon(hwnd, false);
    RestoreWndFromTray(hwnd);
    ShowOwnedPopups(hwnd, true);
    return true;
  }

  UNUSED(wParam);
  return 0;
}



//=============================================================================
//
//  MsgInitMenu() - Handles WM_INITMENU
//
//
void MsgInitMenu(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
  //DocPos p;
  bool b,e,s;

  HMENU hmenu = (HMENU)wParam;

  bool ro = SciCall_GetReadOnly();

  int i = (int)StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile));
  EnableCmd(hmenu,IDM_FILE_REVERT,i);
  EnableCmd(hmenu, CMD_RELOADASCIIASUTF8, i);
  EnableCmd(hmenu, CMD_RECODEANSI, i);
  EnableCmd(hmenu, CMD_RECODEOEM, i);
  EnableCmd(hmenu, CMD_RELOADNOFILEVARS, i);
  EnableCmd(hmenu, CMD_RECODEDEFAULT, i);
  EnableCmd(hmenu, IDM_FILE_LAUNCH, i);

  EnableCmd(hmenu,IDM_FILE_LAUNCH,i);
  EnableCmd(hmenu,IDM_FILE_PROPERTIES,i);
  EnableCmd(hmenu,IDM_FILE_CREATELINK,i);
  EnableCmd(hmenu,IDM_FILE_ADDTOFAV,i);

  EnableCmd(hmenu,IDM_FILE_READONLY,i);
  CheckCmd(hmenu,IDM_FILE_READONLY,g_bFileReadOnly);

  EnableCmd(hmenu,IDM_ENCODING_UNICODEREV,!ro);
  EnableCmd(hmenu,IDM_ENCODING_UNICODE,!ro);
  EnableCmd(hmenu,IDM_ENCODING_UTF8SIGN,!ro);
  EnableCmd(hmenu,IDM_ENCODING_UTF8,!ro);
  EnableCmd(hmenu,IDM_ENCODING_ANSI,!ro);
  EnableCmd(hmenu,IDM_LINEENDINGS_CRLF,!ro);
  EnableCmd(hmenu,IDM_LINEENDINGS_LF,!ro);
  EnableCmd(hmenu,IDM_LINEENDINGS_CR,!ro);

  EnableCmd(hmenu,IDM_ENCODING_RECODE,i);

  if (Encoding_IsUNICODE_REVERSE(Encoding_Current(CPI_GET)))
    i = IDM_ENCODING_UNICODEREV;
  else if (Encoding_IsUNICODE(Encoding_Current(CPI_GET)))
    i = IDM_ENCODING_UNICODE;
  else if (Encoding_IsUTF8_SIGN(Encoding_Current(CPI_GET)))
    i = IDM_ENCODING_UTF8SIGN;
  else if (Encoding_IsUTF8(Encoding_Current(CPI_GET)))
    i = IDM_ENCODING_UTF8;
  else if (Encoding_IsANSI(Encoding_Current(CPI_GET)))
    i = IDM_ENCODING_ANSI;
  else
    i = -1;
  CheckMenuRadioItem(hmenu,IDM_ENCODING_ANSI,IDM_ENCODING_UTF8SIGN,i,MF_BYCOMMAND);

  if (g_iEOLMode == SC_EOL_CRLF)
    i = IDM_LINEENDINGS_CRLF;
  else if (g_iEOLMode == SC_EOL_LF)
    i = IDM_LINEENDINGS_LF;
  else
    i = IDM_LINEENDINGS_CR;
  CheckMenuRadioItem(hmenu,IDM_LINEENDINGS_CRLF,IDM_LINEENDINGS_CR,i,MF_BYCOMMAND);

  EnableCmd(hmenu,IDM_FILE_RECENT,(MRU_Enum(g_pFileMRU,0,NULL,0) > 0));

  EnableCmd(hmenu,IDM_EDIT_UNDO,SciCall_CanUndo() && !ro);
  EnableCmd(hmenu,IDM_EDIT_REDO,SciCall_CanRedo() && !ro);

  s = SciCall_IsSelectionEmpty();
  e = (SciCall_GetTextLength() == 0);
  b = SciCall_CanPaste();
  
  EnableCmd(hmenu,IDM_EDIT_CUT, !e && !ro);       // allow Ctrl-X w/o selection
  EnableCmd(hmenu,IDM_EDIT_COPY, !e);             // allow Ctrl-C w/o selection

  EnableCmd(hmenu,IDM_EDIT_COPYALL, !e);
  EnableCmd(hmenu,IDM_EDIT_COPYADD, !s);

  EnableCmd(hmenu,IDM_EDIT_PASTE, b && !ro);
  EnableCmd(hmenu,IDM_EDIT_SWAP, (!s || b) && !ro);
  EnableCmd(hmenu,IDM_EDIT_CLEAR, !s && !ro);

  EnableCmd(hmenu, IDM_EDIT_SELECTALL, !e);
  
  OpenClipboard(hwnd);
  EnableCmd(hmenu,IDM_EDIT_CLEARCLIPBOARD,CountClipboardFormats());
  CloseClipboard();

  EnableCmd(hmenu,IDM_EDIT_MOVELINEUP,!ro);
  EnableCmd(hmenu,IDM_EDIT_MOVELINEDOWN,!ro);
  EnableCmd(hmenu,IDM_EDIT_DUPLICATELINE,!ro);
  EnableCmd(hmenu,IDM_EDIT_CUTLINE,!ro);
  EnableCmd(hmenu,IDM_EDIT_COPYLINE,true);
  EnableCmd(hmenu,IDM_EDIT_DELETELINE,!ro);

  EnableCmd(hmenu, IDM_EDIT_MERGEBLANKLINES, !ro);
  EnableCmd(hmenu, IDM_EDIT_MERGEEMPTYLINES, !ro);
  EnableCmd(hmenu, IDM_EDIT_REMOVEBLANKLINES, !ro);
  EnableCmd(hmenu, IDM_EDIT_REMOVEEMPTYLINES, !ro);
  EnableCmd(hmenu, IDM_EDIT_REMOVEDUPLICATELINES, !ro);

  EnableCmd(hmenu,IDM_EDIT_INDENT, !s && !ro);
  EnableCmd(hmenu,IDM_EDIT_UNINDENT, !s && !ro);

  EnableCmd(hmenu,IDM_EDIT_PADWITHSPACES,!ro);
  EnableCmd(hmenu,IDM_EDIT_STRIP1STCHAR,!ro);
  EnableCmd(hmenu,IDM_EDIT_STRIPLASTCHAR,!ro);
  EnableCmd(hmenu,IDM_EDIT_TRIMLINES,!ro);
  EnableCmd(hmenu, IDM_EDIT_SELECTIONDUPLICATE, !ro);
  EnableCmd(hmenu, IDM_EDIT_COMPRESSWS, !ro);

  EnableCmd(hmenu, IDM_EDIT_MODIFYLINES, !ro);
  EnableCmd(hmenu, IDM_EDIT_ALIGN, !ro);
  EnableCmd(hmenu, IDM_EDIT_SORTLINES,
    (SciCall_LineFromPosition(SciCall_GetSelectionEnd()) -
      SciCall_LineFromPosition(SciCall_GetSelectionStart())) >= 1);
 
  //EnableCmd(hmenu,IDM_EDIT_COLUMNWRAP,i /*&& IsWindowsNT()*/);
  EnableCmd(hmenu,IDM_EDIT_SPLITLINES,!s && !ro);
  EnableCmd(hmenu,IDM_EDIT_JOINLINES,!s && !ro);
  EnableCmd(hmenu, IDM_EDIT_JOINLN_NOSP,!s && !ro);
  EnableCmd(hmenu,IDM_EDIT_JOINLINES_PARA,!s && !ro);

  EnableCmd(hmenu,IDM_EDIT_CONVERTUPPERCASE,!s && !ro);
  EnableCmd(hmenu,IDM_EDIT_CONVERTLOWERCASE,!s && !ro);
  EnableCmd(hmenu,IDM_EDIT_INVERTCASE,!s && !ro /*&& IsWindowsNT()*/);
  EnableCmd(hmenu,IDM_EDIT_TITLECASE,!s && !ro /*&& IsWindowsNT()*/);
  EnableCmd(hmenu,IDM_EDIT_SENTENCECASE,!s && !ro /*&& IsWindowsNT()*/);

  EnableCmd(hmenu,IDM_EDIT_CONVERTTABS,!s && !ro);
  EnableCmd(hmenu,IDM_EDIT_CONVERTSPACES,!s && !ro);
  EnableCmd(hmenu,IDM_EDIT_CONVERTTABS2,!s && !ro);
  EnableCmd(hmenu,IDM_EDIT_CONVERTSPACES2,!s && !ro);

  EnableCmd(hmenu,IDM_EDIT_URLENCODE,!s && !ro);
  EnableCmd(hmenu,IDM_EDIT_URLDECODE,!s && !ro);

  EnableCmd(hmenu,IDM_EDIT_ESCAPECCHARS,!s && !ro);
  EnableCmd(hmenu,IDM_EDIT_UNESCAPECCHARS,!s && !ro);

  EnableCmd(hmenu,IDM_EDIT_CHAR2HEX, !ro);  // Char2Hex allowed for char after curr pos
  EnableCmd(hmenu,IDM_EDIT_HEX2CHAR, !s && !ro);

  //EnableCmd(hmenu,IDM_EDIT_INCREASENUM,!s && !ro);
  //EnableCmd(hmenu,IDM_EDIT_DECREASENUM,!s && !ro);

  EnableCmd(hmenu,IDM_VIEW_SHOWEXCERPT, !s);

  i = (int)SendMessage(g_hwndEdit,SCI_GETLEXER,0,0);

  EnableCmd(hmenu,IDM_EDIT_LINECOMMENT,
    !(i == SCLEX_NULL || i == SCLEX_CSS || i == SCLEX_DIFF || i == SCLEX_MARKDOWN || i == SCLEX_JSON) && !ro);
  EnableCmd(hmenu,IDM_EDIT_STREAMCOMMENT,
    !(i == SCLEX_NULL || i == SCLEX_VBSCRIPT || i == SCLEX_MAKEFILE || i == SCLEX_VB || i == SCLEX_ASM ||
      i == SCLEX_SQL || i == SCLEX_PERL || i == SCLEX_PYTHON || i == SCLEX_PROPERTIES ||i == SCLEX_CONF ||
      i == SCLEX_POWERSHELL || i == SCLEX_BATCH || i == SCLEX_DIFF || i == SCLEX_BASH || i == SCLEX_TCL ||
      i == SCLEX_AU3 || i == SCLEX_LATEX || i == SCLEX_AHK || i == SCLEX_RUBY || i == SCLEX_CMAKE || i == SCLEX_MARKDOWN ||
      i == SCLEX_YAML || i == SCLEX_REGISTRY || i == SCLEX_NIMROD) && !ro);

  EnableCmd(hmenu, CMD_CTRLENTER, !ro);
  EnableCmd(hmenu, IDM_EDIT_INSERT_TAG, !ro);
  EnableCmd(hmenu, IDM_EDIT_INSERT_ENCODING, (Encoding_GetParseNames(Encoding_Current(CPI_GET)) != NULL) && !ro);

  EnableCmd(hmenu,IDM_EDIT_INSERT_SHORTDATE,!ro);
  EnableCmd(hmenu,IDM_EDIT_INSERT_LONGDATE,!ro);
  EnableCmd(hmenu,IDM_EDIT_INSERT_FILENAME,!ro);
  EnableCmd(hmenu,IDM_EDIT_INSERT_PATHNAME,!ro);

  EnableCmd(hmenu, IDM_EDIT_INSERT_GUID, !ro);

  e = (SciCall_GetTextLength() == 0);

  EnableCmd(hmenu,IDM_EDIT_FIND, !e);
  EnableCmd(hmenu,IDM_EDIT_SAVEFIND, !e);
  EnableCmd(hmenu,IDM_EDIT_FINDNEXT, !e);
  EnableCmd(hmenu,IDM_EDIT_FINDPREV, !e);
  EnableCmd(hmenu,IDM_EDIT_REPLACE, !e && !ro);
  EnableCmd(hmenu,IDM_EDIT_REPLACENEXT, !e && !ro);
  EnableCmd(hmenu,IDM_EDIT_SELTONEXT, !e);
  EnableCmd(hmenu,IDM_EDIT_SELTOPREV, !e);
  EnableCmd(hmenu,IDM_EDIT_FINDMATCHINGBRACE, !e);
  EnableCmd(hmenu,IDM_EDIT_SELTOMATCHINGBRACE, !e);

  EnableCmd(hmenu,BME_EDIT_BOOKMARKPREV, !e);
  EnableCmd(hmenu,BME_EDIT_BOOKMARKNEXT, !e);
  EnableCmd(hmenu,BME_EDIT_BOOKMARKTOGGLE, !e);
  EnableCmd(hmenu,BME_EDIT_BOOKMARKCLEAR, !e);

  EnableCmd(hmenu, IDM_EDIT_DELETELINELEFT, !e && !ro);
  EnableCmd(hmenu, IDM_EDIT_DELETELINERIGHT, !e && !ro);
  EnableCmd(hmenu, CMD_CTRLBACK, !e && !ro);
  EnableCmd(hmenu, CMD_CTRLDEL, !e && !ro);
  EnableCmd(hmenu, CMD_TIMESTAMPS, !e && !ro);

  EnableCmd(hmenu, IDM_VIEW_FONT, !IsWindow(g_hwndDlgCustomizeSchemes));
  EnableCmd(hmenu, IDM_VIEW_CURRENTSCHEME, !IsWindow(g_hwndDlgCustomizeSchemes));

  EnableCmd(hmenu,IDM_VIEW_TOGGLEFOLDS,!e && (g_bCodeFoldingAvailable && g_bShowCodeFolding));
  CheckCmd(hmenu,IDM_VIEW_FOLDING, (g_bCodeFoldingAvailable && g_bShowCodeFolding));
  EnableCmd(hmenu, IDM_VIEW_FOLDING, g_bCodeFoldingAvailable);

  CheckCmd(hmenu,IDM_VIEW_USE2NDDEFAULT,Style_GetUse2ndDefault());

  CheckCmd(hmenu,IDM_VIEW_WORDWRAP,g_bWordWrap);
  CheckCmd(hmenu,IDM_VIEW_LONGLINEMARKER,bMarkLongLines);
  CheckCmd(hmenu,IDM_VIEW_TABSASSPACES,g_bTabsAsSpaces);
  CheckCmd(hmenu,IDM_VIEW_SHOWINDENTGUIDES,bShowIndentGuides);
  CheckCmd(hmenu,IDM_VIEW_AUTOINDENTTEXT,bAutoIndent);
  CheckCmd(hmenu,IDM_VIEW_LINENUMBERS,bShowLineNumbers);
  CheckCmd(hmenu,IDM_VIEW_MARGIN,g_bShowSelectionMargin);
  CheckCmd(hmenu,IDM_VIEW_CHASING_DOCTAIL, g_bChasingDocTail);

  EnableCmd(hmenu,IDM_EDIT_COMPLETEWORD,!e && !ro);
  CheckCmd(hmenu,IDM_VIEW_AUTOCOMPLETEWORDS,bAutoCompleteWords && !ro);
  CheckCmd(hmenu,IDM_VIEW_ACCELWORDNAV,bAccelWordNavigation);

  CheckCmd(hmenu, IDM_VIEW_MARKOCCUR_ONOFF, (g_iMarkOccurrences > 0));
  CheckCmd(hmenu, IDM_VIEW_MARKOCCUR_VISIBLE, g_bMarkOccurrencesMatchVisible);
  CheckCmd(hmenu, IDM_VIEW_MARKOCCUR_CASE, bMarkOccurrencesMatchCase);

  EnableCmd(hmenu, IDM_VIEW_TOGGLE_VIEW, (g_iMarkOccurrences > 0) && !g_bMarkOccurrencesMatchVisible);
  CheckCmd(hmenu, IDM_VIEW_TOGGLE_VIEW, EditToggleView(g_hwndEdit, false));

  if (bMarkOccurrencesMatchWords)
    i = IDM_VIEW_MARKOCCUR_WORD;
  else if (bMarkOccurrencesCurrentWord)
    i = IDM_VIEW_MARKOCCUR_CURRENT;
  else
    i = IDM_VIEW_MARKOCCUR_WNONE;

  CheckMenuRadioItem(hmenu, IDM_VIEW_MARKOCCUR_WNONE, IDM_VIEW_MARKOCCUR_CURRENT, i, MF_BYCOMMAND);
  CheckCmdPos(GetSubMenu(GetSubMenu(GetMenu(g_hwndMain), 2), 17), 5, (i != IDM_VIEW_MARKOCCUR_WNONE));

  i = (int)(g_iMarkOccurrences > 0);
  EnableCmd(hmenu, IDM_VIEW_MARKOCCUR_VISIBLE, i);
  EnableCmd(hmenu, IDM_VIEW_MARKOCCUR_CASE, i);
  EnableCmd(hmenu, IDM_VIEW_MARKOCCUR_WNONE, i);
  EnableCmd(hmenu,IDM_VIEW_MARKOCCUR_WORD, i);
  EnableCmd(hmenu, IDM_VIEW_MARKOCCUR_CURRENT, i);
  EnableCmdPos(GetSubMenu(GetSubMenu(GetMenu(g_hwndMain), 2), 17), 5, i);


  CheckCmd(hmenu,IDM_VIEW_SHOWWHITESPACE,bViewWhiteSpace);
  CheckCmd(hmenu,IDM_VIEW_SHOWEOLS,bViewEOLs);
  CheckCmd(hmenu,IDM_VIEW_WORDWRAPSYMBOLS,bShowWordWrapSymbols);
  CheckCmd(hmenu,IDM_VIEW_MATCHBRACES,bMatchBraces);
  CheckCmd(hmenu,IDM_VIEW_TOOLBAR,bShowToolbar);
  EnableCmd(hmenu,IDM_VIEW_CUSTOMIZETB,bShowToolbar);
  CheckCmd(hmenu,IDM_VIEW_STATUSBAR,bShowStatusbar);

  i = (int)SendMessage(g_hwndEdit,SCI_GETLEXER,0,0);
  //EnableCmd(hmenu,IDM_VIEW_AUTOCLOSETAGS,(i == SCLEX_HTML || i == SCLEX_XML));
  CheckCmd(hmenu, IDM_VIEW_AUTOCLOSETAGS, bAutoCloseTags /*&& (i == SCLEX_HTML || i == SCLEX_XML)*/);
  CheckCmd(hmenu, IDM_VIEW_HILITECURRENTLINE, bHiliteCurrentLine);
  CheckCmd(hmenu, IDM_VIEW_HYPERLINKHOTSPOTS, g_bHyperlinkHotspot);
  CheckCmd(hmenu, IDM_VIEW_SCROLLPASTEOF, bScrollPastEOF);
 

  i = IniGetInt(L"Settings2",L"ReuseWindow",0);
  CheckCmd(hmenu,IDM_VIEW_REUSEWINDOW,i);
  i = IniGetInt(L"Settings2",L"SingleFileInstance",0);
  CheckCmd(hmenu,IDM_VIEW_SINGLEFILEINSTANCE,i);
  bStickyWinPos = IniGetInt(L"Settings2",L"StickyWindowPosition",0);
  CheckCmd(hmenu,IDM_VIEW_STICKYWINPOS,bStickyWinPos);
  CheckCmd(hmenu,IDM_VIEW_ALWAYSONTOP,((bAlwaysOnTop || flagAlwaysOnTop == 2) && flagAlwaysOnTop != 1));
  CheckCmd(hmenu,IDM_VIEW_MINTOTRAY,bMinimizeToTray);
  CheckCmd(hmenu,IDM_VIEW_TRANSPARENT,bTransparentMode && bTransparentModeAvailable);
  EnableCmd(hmenu,IDM_VIEW_TRANSPARENT,bTransparentModeAvailable);

  CheckCmd(hmenu,IDM_VIEW_NOSAVERECENT,bSaveRecentFiles);
  CheckCmd(hmenu,IDM_VIEW_NOPRESERVECARET, bPreserveCaretPos);
  CheckCmd(hmenu,IDM_VIEW_NOSAVEFINDREPL,bSaveFindReplace);
  CheckCmd(hmenu,IDM_VIEW_SAVEBEFORERUNNINGTOOLS,bSaveBeforeRunningTools);

  CheckCmd(hmenu,IDM_VIEW_CHANGENOTIFY,g_iFileWatchingMode);

  if (StringCchLenW(szTitleExcerpt,COUNTOF(szTitleExcerpt)))
    i = IDM_VIEW_SHOWEXCERPT;
  else if (iPathNameFormat == 0)
    i = IDM_VIEW_SHOWFILENAMEONLY;
  else if (iPathNameFormat == 1)
    i = IDM_VIEW_SHOWFILENAMEFIRST;
  else
    i = IDM_VIEW_SHOWFULLPATH;
  CheckMenuRadioItem(hmenu,IDM_VIEW_SHOWFILENAMEONLY,IDM_VIEW_SHOWEXCERPT,i,MF_BYCOMMAND);

  if (iEscFunction == 1)
    i = IDM_VIEW_ESCMINIMIZE;
  else if (iEscFunction == 2)
    i = IDM_VIEW_ESCEXIT;
  else
    i = IDM_VIEW_NOESCFUNC;
  CheckMenuRadioItem(hmenu,IDM_VIEW_NOESCFUNC,IDM_VIEW_ESCEXIT,i,MF_BYCOMMAND);

  i = (int)StringCchLenW(g_wchIniFile,COUNTOF(g_wchIniFile));
  CheckCmd(hmenu,IDM_VIEW_SAVESETTINGS,bSaveSettings && i);

  EnableCmd(hmenu,IDM_VIEW_REUSEWINDOW,i);
  EnableCmd(hmenu,IDM_VIEW_STICKYWINPOS,i);
  EnableCmd(hmenu,IDM_VIEW_SINGLEFILEINSTANCE,i);
  EnableCmd(hmenu,IDM_VIEW_NOSAVERECENT,i);
  EnableCmd(hmenu,IDM_VIEW_NOPRESERVECARET,i);
  EnableCmd(hmenu,IDM_VIEW_NOSAVEFINDREPL,i);
  EnableCmd(hmenu,IDM_VIEW_SAVESETTINGS,bEnableSaveSettings && i);

  CheckCmd(hmenu, IDM_VIEW_TOGGLETB, (iHighDpiToolBar > 0));
  EnableCmd(hmenu, IDM_VIEW_TOGGLETB, !g_bExternalBitmap);

  i = (StringCchLenW(g_wchIniFile,COUNTOF(g_wchIniFile)) > 0 || StringCchLenW(g_wchIniFile2,COUNTOF(g_wchIniFile2)) > 0);
  EnableCmd(hmenu,IDM_VIEW_SAVESETTINGSNOW,bEnableSaveSettings && i);

  bool bIsHLink = false;
  if ((bool)SendMessage(g_hwndEdit, SCI_STYLEGETHOTSPOT, Style_GetHotspotStyleID(), 0)) 
  {
    bIsHLink = (Style_GetHotspotStyleID() == (int)SendMessage(g_hwndEdit, SCI_GETSTYLEAT, SciCall_GetCurrentPos(), 0));
  }
  EnableCmd(hmenu, CMD_OPEN_HYPERLINK, bIsHLink);

  i = StringCchLenW(g_tchUpdateCheckerExe, COUNTOF(g_tchUpdateCheckerExe));
  EnableCmd(hmenu, IDM_HELP_UPDATEINSTALLER, i);

  UNUSED(lParam);
}


//=============================================================================
//
//  MsgSysCommand() - Handles WM_SYSCOMMAND
//
//
LRESULT MsgSysCommand(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  switch (wParam) {
  case SC_MINIMIZE:
    ShowOwnedPopups(hwnd, false);
    if (bMinimizeToTray) {
      MinimizeWndToTray(hwnd);
      ShowNotifyIcon(hwnd, true);
      SetNotifyIconTitle(hwnd);
      return(0);
    }
    else
      return DefWindowProc(hwnd, umsg, wParam, lParam);

  case SC_RESTORE:
    {
      LRESULT lrv = DefWindowProc(hwnd, umsg, wParam, lParam);
      ShowOwnedPopups(hwnd, true);
      return(lrv);
    }
  }
  return DefWindowProc(hwnd, umsg, wParam, lParam);
}


//=============================================================================
//
//  MsgCommand() - Handles WM_COMMAND
//
//
LRESULT MsgCommand(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  switch(LOWORD(wParam))
  {
    case SCEN_CHANGE:
      MarkAllOccurrences(iUpdateDelayMarkAllCoccurrences);
      break;

    case IDT_TIMER_MAIN_MRKALL:
      EditMarkAllOccurrences();
      break;


    case IDT_TIMER_UPDATE_HOTSPOT:
      EditUpdateVisibleUrlHotspot(g_bHyperlinkHotspot);
      break;

    case IDM_FILE_NEW:
      FileLoad(false,true,false,bSkipUnicodeDetection,bSkipANSICodePageDetection,L"");
      break;


    case IDM_FILE_OPEN:
      FileLoad(false,false,false,bSkipUnicodeDetection,bSkipANSICodePageDetection,L"");
      break;


    case IDM_FILE_REVERT:
      if ((IsDocumentModified || Encoding_HasChanged(CPI_GET)) && MsgBox(MBOKCANCEL,IDS_ASK_REVERT) != IDOK) {
        return(0);
      }
      FileRevert(g_wchCurFile);
      break;


    case IDM_FILE_SAVE:
      FileSave(true,false,false,false);
      break;


    case IDM_FILE_SAVEAS:
      FileSave(true,false,true,false);
      break;


    case IDM_FILE_SAVECOPY:
      FileSave(true,false,true,true);
      break;


    case IDM_FILE_READONLY:
      if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile)))
      {
        DWORD dwFileAttributes = GetFileAttributes(g_wchCurFile);
        if (dwFileAttributes != INVALID_FILE_ATTRIBUTES) {
          if (g_bFileReadOnly)
            dwFileAttributes = (dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);
          else
            dwFileAttributes |= FILE_ATTRIBUTE_READONLY;
          if (!SetFileAttributes(g_wchCurFile,dwFileAttributes))
            MsgBox(MBWARN,IDS_READONLY_MODIFY,g_wchCurFile);
        }
        else
          MsgBox(MBWARN,IDS_READONLY_MODIFY,g_wchCurFile);

        dwFileAttributes = GetFileAttributes(g_wchCurFile);
        if (dwFileAttributes != INVALID_FILE_ATTRIBUTES)
          g_bFileReadOnly = (dwFileAttributes & FILE_ATTRIBUTE_READONLY);

        UpdateToolbar();
      }
      break;


    case IDM_FILE_BROWSE:
        DialogFileBrowse(hwnd);
      break;


    case IDM_FILE_NEWWINDOW:
    case IDM_FILE_NEWWINDOW2:
      DialogNewWindow(hwnd, bSaveBeforeRunningTools, (LOWORD(wParam) != IDM_FILE_NEWWINDOW2));
      break;


    case IDM_FILE_LAUNCH:
      {
        WCHAR wchDirectory[MAX_PATH] = { L'\0' };

        if (!StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile)))
          break;

        if (bSaveBeforeRunningTools && !FileSave(false,true,false,false))
          break;

        if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
          StringCchCopy(wchDirectory,COUNTOF(wchDirectory),g_wchCurFile);
          PathRemoveFileSpec(wchDirectory);
        }

        SHELLEXECUTEINFO sei;
        ZeroMemory(&sei,sizeof(SHELLEXECUTEINFO));
        sei.cbSize = sizeof(SHELLEXECUTEINFO);
        sei.fMask = 0;
        sei.hwnd = hwnd;
        sei.lpVerb = NULL;
        sei.lpFile = g_wchCurFile;
        sei.lpParameters = NULL;
        sei.lpDirectory = wchDirectory;
        sei.nShow = SW_SHOWNORMAL;
        ShellExecuteEx(&sei);
      }
      break;


    case IDM_FILE_RUN:
      {
        WCHAR tchCmdLine[MAX_PATH+4];

        if (bSaveBeforeRunningTools && !FileSave(false,true,false,false))
          break;

        StringCchCopy(tchCmdLine,COUNTOF(tchCmdLine),g_wchCurFile);
        PathQuoteSpaces(tchCmdLine);

        RunDlg(hwnd,tchCmdLine);
      }
      break;


    case IDM_FILE_OPENWITH:
      if (bSaveBeforeRunningTools && !FileSave(false,true,false,false))
        break;
      OpenWithDlg(hwnd,g_wchCurFile);
      break;


    case IDM_FILE_PAGESETUP:
      EditPrintSetup(g_hwndEdit);
      break;

    case IDM_FILE_PRINT:
      {
        SHFILEINFO shfi;
        WCHAR *pszTitle;
        WCHAR tchUntitled[32] = { L'\0' };
        WCHAR tchPageFmt[32] = { L'\0' };

        if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
          SHGetFileInfo2(g_wchCurFile,FILE_ATTRIBUTE_NORMAL,&shfi,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES);
          pszTitle = shfi.szDisplayName;
        }
        else {
          GetString(IDS_UNTITLED,tchUntitled,COUNTOF(tchUntitled));
          pszTitle = tchUntitled;
        }

        GetString(IDS_PRINT_PAGENUM,tchPageFmt,COUNTOF(tchPageFmt));

        if (!EditPrint(g_hwndEdit,pszTitle,tchPageFmt))
          MsgBox(MBWARN,IDS_PRINT_ERROR,pszTitle);
      }
      break;


    case IDM_FILE_PROPERTIES:
      {
        if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile)) == 0)
          break;

        SHELLEXECUTEINFO sei;
        ZeroMemory(&sei,sizeof(SHELLEXECUTEINFO));
        sei.cbSize = sizeof(SHELLEXECUTEINFO);
        sei.fMask = SEE_MASK_INVOKEIDLIST;
        sei.hwnd = hwnd;
        sei.lpVerb = L"properties";
        sei.lpFile = g_wchCurFile;
        sei.nShow = SW_SHOWNORMAL;
        ShellExecuteEx(&sei);
      }
      break;

    case IDM_FILE_CREATELINK:
      {
        if (!StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile)))
          break;

        if (!PathCreateDeskLnk(g_wchCurFile))
          MsgBox(MBWARN,IDS_ERR_CREATELINK);
      }
      break;


    case IDM_FILE_OPENFAV:
      if (FileSave(false,true,false,false)) {

        WCHAR tchSelItem[MAX_PATH] = { L'\0' };

        if (FavoritesDlg(hwnd,tchSelItem))
        {
          if (PathIsLnkToDirectory(tchSelItem,NULL,0))
            PathGetLnkPath(tchSelItem,tchSelItem,COUNTOF(tchSelItem));

          if (PathIsDirectory(tchSelItem))
          {
            WCHAR tchFile[MAX_PATH] = { L'\0' };

            if (OpenFileDlg(g_hwndMain,tchFile,COUNTOF(tchFile),tchSelItem))
              FileLoad(true,false,false,bSkipUnicodeDetection,bSkipANSICodePageDetection,tchFile);
          }
          else
            FileLoad(true,false,false,bSkipUnicodeDetection,bSkipANSICodePageDetection,tchSelItem);
          }
        }
      break;


    case IDM_FILE_ADDTOFAV:
      if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
        SHFILEINFO shfi;
        SHGetFileInfo2(g_wchCurFile,FILE_ATTRIBUTE_NORMAL,
          &shfi,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES);
        AddToFavDlg(hwnd,shfi.szDisplayName,g_wchCurFile);
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
        sei.lpFile = g_tchFavoritesDir;
        sei.lpParameters = NULL;
        sei.lpDirectory = NULL;
        sei.nShow = SW_SHOWNORMAL;
        // Run favorites directory
        ShellExecuteEx(&sei);
      }
      break;


    case IDM_FILE_RECENT:
      if (MRU_Enum(g_pFileMRU,0,NULL,0) > 0) {
        if (FileSave(false,true,false,false)) {
          WCHAR tchFile[MAX_PATH] = { L'\0' };
          if (FileMRUDlg(hwnd,tchFile))
            FileLoad(true,false,false,false,true,tchFile);
          }
        }
      break;


    case IDM_FILE_EXIT:
      SendMessage(hwnd,WM_CLOSE,0,0);
      break;


    case IDM_ENCODING_ANSI:
    case IDM_ENCODING_UNICODE:
    case IDM_ENCODING_UNICODEREV:
    case IDM_ENCODING_UTF8:
    case IDM_ENCODING_UTF8SIGN:
    case IDM_ENCODING_SELECT:
      {
        int iNewEncoding = Encoding_Current(CPI_GET);
        if (LOWORD(wParam) == IDM_ENCODING_SELECT && !SelectEncodingDlg(hwnd,&iNewEncoding))
          break;
        else {
          switch (LOWORD(wParam)) {
            case IDM_ENCODING_UNICODE:    iNewEncoding = CPI_UNICODEBOM; break;
            case IDM_ENCODING_UNICODEREV: iNewEncoding = CPI_UNICODEBEBOM; break;
            case IDM_ENCODING_UTF8:       iNewEncoding = CPI_UTF8; break;
            case IDM_ENCODING_UTF8SIGN:   iNewEncoding = CPI_UTF8SIGN; break;
            case IDM_ENCODING_ANSI:       iNewEncoding = CPI_ANSI_DEFAULT; break;
          }
        }

        BeginWaitCursor(NULL);
        _IGNORE_NOTIFY_CHANGE_;
        if (EditSetNewEncoding(g_hwndEdit,
                               iNewEncoding,
                               (flagSetEncoding),
                               StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile)) == 0)) {

          if (SendMessage(g_hwndEdit,SCI_GETLENGTH,0,0) == 0) {
            Encoding_Current(iNewEncoding);
            Encoding_HasChanged(iNewEncoding);
          }
          else {
            if (Encoding_IsANSI(Encoding_Current(CPI_GET)) || Encoding_IsANSI(iNewEncoding))
              Encoding_HasChanged(CPI_NONE);
            Encoding_Current(iNewEncoding);
          }
        }
        _OBSERVE_NOTIFY_CHANGE_;
        EndWaitCursor();
        UpdateStatusbar(false);
      }
      break;


    case IDM_ENCODING_RECODE:
      {
        if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {

          WCHAR tchCurFile2[MAX_PATH] = { L'\0' };

          int iNewEncoding = Encoding_MapUnicode(Encoding_Current(CPI_GET));

          if ((IsDocumentModified || Encoding_HasChanged(CPI_GET)) && MsgBox(MBOKCANCEL,IDS_ASK_RECODE) != IDOK)
            return(0);

          if (RecodeDlg(hwnd,&iNewEncoding)) 
          {
            StringCchCopy(tchCurFile2,COUNTOF(tchCurFile2),g_wchCurFile);
            Encoding_SrcCmdLn(iNewEncoding);
            FileLoad(true,false,true,false,true,tchCurFile2);
          }
        }
      }
      break;


    case IDM_ENCODING_SETDEFAULT:
      SelectDefEncodingDlg(hwnd,&g_iDefaultNewFileEncoding);
      break;


    case IDM_LINEENDINGS_CRLF:
    case IDM_LINEENDINGS_LF:
    case IDM_LINEENDINGS_CR:
      {
        BeginWaitCursor(NULL);
        _IGNORE_NOTIFY_CHANGE_;
        int iNewEOLMode = iLineEndings[LOWORD(wParam)-IDM_LINEENDINGS_CRLF];
        g_iEOLMode = iNewEOLMode;
        SendMessage(g_hwndEdit,SCI_SETEOLMODE,g_iEOLMode,0);
        SendMessage(g_hwndEdit,SCI_CONVERTEOLS,g_iEOLMode,0);
        EditFixPositions(g_hwndEdit);
        _OBSERVE_NOTIFY_CHANGE_;
        EndWaitCursor();
        UpdateStatusbar(false);
      }
      break;


    case IDM_LINEENDINGS_SETDEFAULT:
      SelectDefLineEndingDlg(hwnd,&g_iDefaultEOLMode);
      break;


    case IDM_EDIT_UNDO:
      _IGNORE_NOTIFY_CHANGE_;
      SendMessage(g_hwndEdit, SCI_UNDO, 0, 0);
      _OBSERVE_NOTIFY_CHANGE_;
      break;


    case IDM_EDIT_REDO:
      _IGNORE_NOTIFY_CHANGE_;
      SendMessage(g_hwndEdit, SCI_REDO, 0, 0);
      _OBSERVE_NOTIFY_CHANGE_;
      break;


    case IDM_EDIT_CUT:
      {
        if (flagPasteBoard)
          bLastCopyFromMe = true;

        int token = BeginUndoAction();
        if (!SciCall_IsSelectionEmpty())
        {
          SciCall_Cut();
        }
        else { // VisualStudio behavior
          SciCall_CopyAllowLine();
          SciCall_LineDelete();
        }
        EndUndoAction(token);
        UpdateToolbar();
      }
      break;


    case IDM_EDIT_COPY:
    case IDM_EDIT_COPYLINE:
      if (flagPasteBoard)
        bLastCopyFromMe = true;
      SciCall_CopyAllowLine();
      UpdateToolbar();
      break;


    case IDM_EDIT_COPYALL:
      {
        if (flagPasteBoard)
          bLastCopyFromMe = true;
        SendMessage(g_hwndEdit,SCI_COPYRANGE,0,(LPARAM)SciCall_GetTextLength());
        UpdateToolbar();
      }
      break;


    case IDM_EDIT_COPYADD:
      {
        if (flagPasteBoard)
          bLastCopyFromMe = true;
        EditCopyAppend(g_hwndEdit,true);
        UpdateToolbar();
      }
      break;

    case IDM_EDIT_PASTE:
      {
        if (flagPasteBoard)
          bLastCopyFromMe = true;
        int token = BeginUndoAction();
        EditPasteClipboard(g_hwndEdit, false, bSkipUnicodeDetection);
        EndUndoAction(token);
        // Updates done by EditPasteClipboard():
        //~UpdateToolbar();
        //~UpdateStatusbar(false);
        //~UpdateLineNumberWidth();
      }
      break;

    case IDM_EDIT_SWAP:
      {
        if (flagPasteBoard)
          bLastCopyFromMe = true;
        int token = BeginUndoAction();
        EditPasteClipboard(g_hwndEdit, true, bSkipUnicodeDetection);
        EndUndoAction(token);
        UpdateToolbar();
        UpdateStatusbar(false);
      }
      break;

    case IDM_EDIT_CLEARCLIPBOARD:
      EditClearClipboard(g_hwndEdit);
      UpdateToolbar();
      break;


    case IDM_EDIT_SELECTALL:
        SendMessage(g_hwndEdit,SCI_SELECTALL,0,0);
        UpdateStatusbar(false);
      break;


    case IDM_EDIT_SELECTWORD:
      {
        DocPos iPos = SciCall_GetCurrentPos();

        if (SendMessage(g_hwndEdit, SCI_GETSELECTIONEMPTY, 0, 0)) {

          DocPos iWordStart = (DocPos)SendMessage(g_hwndEdit,SCI_WORDSTARTPOSITION,iPos,true);
          DocPos iWordEnd   = (DocPos)SendMessage(g_hwndEdit,SCI_WORDENDPOSITION,iPos,true);

          if (iWordStart == iWordEnd) // we are in whitespace salad...
          {
            iWordStart = (DocPos)SendMessage(g_hwndEdit,SCI_WORDENDPOSITION,iPos,false);
            iWordEnd   = (DocPos)SendMessage(g_hwndEdit,SCI_WORDENDPOSITION,iWordStart,true);
            if (iWordStart != iWordEnd) {
              SciCall_SetSel(iWordStart, iWordEnd);
            }
          }
          else {
            SciCall_SetSel(iWordStart, iWordEnd);
          }

          if (SciCall_IsSelectionEmpty()) {
            const DocLn iLine = SciCall_LineFromPosition(iPos);
            const DocPos iLineStart = SciCall_GetLineIndentPosition(iLine);
            const DocPos iLineEnd = SciCall_GetLineEndPosition(iLine);
            SciCall_SetSel(iLineStart, iLineEnd);
          }
        }
        else {
          const DocLn iLine = SciCall_LineFromPosition(iPos);
          const DocPos iLineStart = SciCall_GetLineIndentPosition(iLine);
          const DocPos iLineEnd = SciCall_GetLineEndPosition(iLine);
          SciCall_SetSel(iLineStart, iLineEnd);
        }
        UpdateStatusbar(false);
      }
      break;


    case IDM_EDIT_SELECTLINE:
      {
        const DocPos iSelStart = SciCall_GetSelectionStart();
        const DocPos iSelEnd = SciCall_GetSelectionEnd();
        const DocPos iLineStart = SciCall_LineFromPosition(iSelStart);
        const DocPos iLineEnd = SciCall_LineFromPosition(iSelEnd);
        SciCall_SetSel(SciCall_PositionFromLine(iLineStart), SciCall_PositionFromLine(iLineEnd + 1));
        SciCall_ChooseCaretX();
        UpdateStatusbar(false);
      }
      break;


    case IDM_EDIT_MOVELINEUP:
      {
        int token = BeginUndoAction();
        EditMoveUp(g_hwndEdit);
        EndUndoAction(token);
      }
      break;


    case IDM_EDIT_MOVELINEDOWN:
      {
        int token = BeginUndoAction();
        EditMoveDown(g_hwndEdit);
        EndUndoAction(token);
      }
      break;


    case IDM_EDIT_DUPLICATELINE:
      SendMessage(g_hwndEdit,SCI_LINEDUPLICATE,0,0);
      break;


    case IDM_EDIT_CUTLINE:
      {
        if (flagPasteBoard)
          bLastCopyFromMe = true;
        int token = BeginUndoAction();
        SendMessage(g_hwndEdit,SCI_LINECUT,0,0);
        UpdateToolbar();
        EndUndoAction(token);
      }
      break;


    case IDM_EDIT_DELETELINE:
      {
        int token = BeginUndoAction();
        SendMessage(g_hwndEdit, SCI_LINEDELETE, 0, 0);
        EndUndoAction(token);
      }
      break;


    case IDM_EDIT_DELETELINELEFT:
      {
        int token = BeginUndoAction();
        SendMessage(g_hwndEdit, SCI_DELLINELEFT, 0, 0);
        EndUndoAction(token);
      }
      break;


    case IDM_EDIT_DELETELINERIGHT:
      {
        int token = BeginUndoAction();
        SendMessage(g_hwndEdit, SCI_DELLINERIGHT, 0, 0);
        EndUndoAction(token);
      }
      break;


    case IDM_EDIT_INDENT:
      {
        int token = BeginUndoAction();
        EditIndentBlock(g_hwndEdit, SCI_TAB, true);
        EndUndoAction(token);
      }
      break;

    case IDM_EDIT_UNINDENT:
      {
        int token = BeginUndoAction();
        EditIndentBlock(g_hwndEdit, SCI_BACKTAB, true);
        EndUndoAction(token);
      }
      break;

    case CMD_TAB:
      {
        int token = BeginUndoAction();
        EditIndentBlock(g_hwndEdit, SCI_TAB, false);
        EndUndoAction(token);
      }
      break;

    case CMD_BACKTAB:
      {
        int token = BeginUndoAction();
        EditIndentBlock(g_hwndEdit, SCI_BACKTAB, false);
        EndUndoAction(token);
      }
      break;

    case CMD_CTRLTAB:
      {
        int token = BeginUndoAction();
        SendMessage(g_hwndEdit, SCI_SETUSETABS, true, 0);
        SendMessage(g_hwndEdit, SCI_SETTABINDENTS, false, 0);
        EditIndentBlock(g_hwndEdit, SCI_TAB, false);
        SendMessage(g_hwndEdit, SCI_SETTABINDENTS, g_bTabIndents, 0);
        SendMessage(g_hwndEdit, SCI_SETUSETABS, !g_bTabsAsSpaces, 0);
        EndUndoAction(token);
      }
      break;

    case CMD_DELETEBACK:
      {
        int token = BeginUndoAction();
        SendMessage(g_hwndEdit, SCI_DELETEBACK, 0, 0);
        EndUndoAction(token);
      }
      break;

    case CMD_VK_INSERT:
      SendMessage(g_hwndEdit, SCI_EDITTOGGLEOVERTYPE, 0, 0);
      UpdateStatusbar(false);
      break;

    case IDM_EDIT_ENCLOSESELECTION:
      if (EditEncloseSelectionDlg(hwnd,wchPrefixSelection,wchAppendSelection)) {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditEncloseSelection(g_hwndEdit,wchPrefixSelection,wchAppendSelection);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_SELECTIONDUPLICATE:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        SendMessage(g_hwndEdit,SCI_SELECTIONDUPLICATE,0,0);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_PADWITHSPACES:
      {
        BeginWaitCursor(NULL);
        EditPadWithSpaces(g_hwndEdit,false,false);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_STRIP1STCHAR:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditStripFirstCharacter(g_hwndEdit);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_STRIPLASTCHAR:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditStripLastCharacter(g_hwndEdit, false, false);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_TRIMLINES:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditStripLastCharacter(g_hwndEdit, false, true);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_COMPRESSWS:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditCompressSpaces(g_hwndEdit);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_MERGEBLANKLINES:
    {
      BeginWaitCursor(NULL);
      int token = BeginUndoAction();
      EditRemoveBlankLines(g_hwndEdit, true, true);
      EndUndoAction(token);
      EndWaitCursor();
    }
    break;

    case IDM_EDIT_MERGEEMPTYLINES:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditRemoveBlankLines(g_hwndEdit, true, false);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_REMOVEBLANKLINES:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditRemoveBlankLines(g_hwndEdit, false, true);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_REMOVEEMPTYLINES:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditRemoveBlankLines(g_hwndEdit, false, false);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_REMOVEDUPLICATELINES:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditRemoveDuplicateLines(g_hwndEdit, false);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_MODIFYLINES:
      {
        if (EditModifyLinesDlg(hwnd,wchPrefixLines,wchAppendLines)) {
          BeginWaitCursor(NULL);
          int token = BeginUndoAction();
          EditModifyLines(g_hwndEdit,wchPrefixLines,wchAppendLines);
          EndUndoAction(token);
          EndWaitCursor();
        }
      }
      break;


    case IDM_EDIT_ALIGN:
      {
        if (EditAlignDlg(hwnd,&iAlignMode)) {
          BeginWaitCursor(NULL);
          int token = BeginUndoAction();
          EditAlignText(g_hwndEdit,iAlignMode);
          EndUndoAction(token);
          EndWaitCursor();
        }
      }
      break;


    case IDM_EDIT_SORTLINES:
      {
        if (EditSortDlg(hwnd,&iSortOptions)) {
          BeginWaitCursor(NULL);
          int token = BeginUndoAction();
          EditSortLines(g_hwndEdit,iSortOptions);
          EndUndoAction(token);
          EndWaitCursor();
        }
      }
      break;


    case IDM_EDIT_COLUMNWRAP:
      {
        if (iWrapCol == 0) {
          iWrapCol = g_iLongLinesLimit;
        }

        UINT uWrpCol = 0;
        if (ColumnWrapDlg(hwnd,IDD_COLUMNWRAP,&uWrpCol))
        {
          iWrapCol = (DocPos)max(min(uWrpCol,(UINT)g_iLongLinesLimit),1);
          BeginWaitCursor(NULL);
          int token = BeginUndoAction();
          EditWrapToColumn(g_hwndEdit,iWrapCol);
          EndUndoAction(token);
          EndWaitCursor();
        }
      }
      break;


    case IDM_EDIT_SPLITLINES:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditSplitLines(g_hwndEdit);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_JOINLINES:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditJoinLinesEx(g_hwndEdit, false, true);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;

    case IDM_EDIT_JOINLN_NOSP:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditJoinLinesEx(g_hwndEdit, false, false);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;

    case IDM_EDIT_JOINLINES_PARA:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditJoinLinesEx(g_hwndEdit, true, true);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_CONVERTUPPERCASE:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        SendMessage(g_hwndEdit,SCI_UPPERCASE,0,0);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_CONVERTLOWERCASE:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        SendMessage(g_hwndEdit,SCI_LOWERCASE,0,0);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_INVERTCASE:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditInvertCase(g_hwndEdit);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_TITLECASE:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditTitleCase(g_hwndEdit);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_SENTENCECASE:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditSentenceCase(g_hwndEdit);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_CONVERTTABS:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditTabsToSpaces(g_hwndEdit, g_iTabWidth, false);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_CONVERTSPACES:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditSpacesToTabs(g_hwndEdit, g_iTabWidth, false);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_CONVERTTABS2:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditTabsToSpaces(g_hwndEdit, g_iTabWidth, true);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_CONVERTSPACES2:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditSpacesToTabs(g_hwndEdit, g_iTabWidth, true);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_INSERT_TAG:
      {
        WCHAR wszOpen[256] = { L'\0' };
        WCHAR wszClose[256] = { L'\0' };
        if (EditInsertTagDlg(hwnd, wszOpen, wszClose)) {
          int token = BeginUndoAction();
          EditEncloseSelection(g_hwndEdit, wszOpen, wszClose);
          EndUndoAction(token);
        }
      }
      break;


    case IDM_EDIT_INSERT_ENCODING:
      {
        if (*Encoding_GetParseNames(Encoding_Current(CPI_GET))) {
          char msz[32] = { '\0' };
          //int iSelStart;
          StringCchCopyNA(msz,COUNTOF(msz), Encoding_GetParseNames(Encoding_Current(CPI_GET)),COUNTOF(msz));
          char *p = StrChrA(msz, ',');
          if (p)
            *p = 0;
          int token = BeginUndoAction();
          SendMessage(g_hwndEdit,SCI_REPLACESEL,0,(LPARAM)msz);
          EndUndoAction(token);
        }
      }
      break;


    case IDM_EDIT_INSERT_SHORTDATE:
    case IDM_EDIT_INSERT_LONGDATE:
      {
        WCHAR tchDate[128] = { L'\0' };
        WCHAR tchTime[128] = { L'\0' };
        WCHAR tchDateTime[256] = { L'\0' };
        WCHAR tchTemplate[256] = { L'\0' };
        SYSTEMTIME st;
        char  mszBuf[MAX_PATH*3] = { '\0' };
        //int   iSelStart;

        GetLocalTime(&st);

        if (IniGetString(L"Settings2",
              (LOWORD(wParam) == IDM_EDIT_INSERT_SHORTDATE) ? L"DateTimeShort" : L"DateTimeLong",
              L"",tchTemplate,COUNTOF(tchTemplate))) {
          struct tm sst;
          sst.tm_isdst       = -1;
          sst.tm_sec         = (int)st.wSecond;
          sst.tm_min         = (int)st.wMinute;
          sst.tm_hour        = (int)st.wHour;
          sst.tm_mday        = (int)st.wDay;
          sst.tm_mon         = (int)st.wMonth - 1;
          sst.tm_year        = (int)st.wYear - 1900;
          sst.tm_wday        = (int)st.wDayOfWeek;
          mktime(&sst);
          wcsftime(tchDateTime,COUNTOF(tchDateTime),tchTemplate,&sst);
        }
        else {
          GetDateFormat(LOCALE_USER_DEFAULT,(
            LOWORD(wParam) == IDM_EDIT_INSERT_SHORTDATE) ? DATE_SHORTDATE : DATE_LONGDATE,
            &st,NULL,tchDate,COUNTOF(tchDate));
          GetTimeFormat(LOCALE_USER_DEFAULT,TIME_NOSECONDS,&st,NULL,tchTime,COUNTOF(tchTime));

          StringCchPrintf(tchDateTime,COUNTOF(tchDateTime),L"%s %s",tchTime,tchDate);
        }

        WideCharToMultiByteStrg(Encoding_SciCP,tchDateTime,mszBuf);
        int token = BeginUndoAction();
        SendMessage(g_hwndEdit,SCI_REPLACESEL,0,(LPARAM)mszBuf);
        EndUndoAction(token);
      }
      break;


    case IDM_EDIT_INSERT_FILENAME:
    case IDM_EDIT_INSERT_PATHNAME:
      {
        SHFILEINFO shfi;
        WCHAR *pszInsert;
        WCHAR tchUntitled[32];
        char  mszBuf[MAX_PATH*3];
        //int   iSelStart;

        if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
          if (LOWORD(wParam) == IDM_EDIT_INSERT_FILENAME) {
            SHGetFileInfo2(g_wchCurFile,FILE_ATTRIBUTE_NORMAL,&shfi,sizeof(SHFILEINFO),
              SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES);
            pszInsert = shfi.szDisplayName;
          }
          else
            pszInsert = g_wchCurFile;
        }
        else {
          GetString(IDS_UNTITLED,tchUntitled,COUNTOF(tchUntitled));
          pszInsert = tchUntitled;
        }

        WideCharToMultiByteStrg(Encoding_SciCP,pszInsert,mszBuf);
        int token = BeginUndoAction();
        SendMessage(g_hwndEdit,SCI_REPLACESEL,0,(LPARAM)mszBuf);
        EndUndoAction(token);
    }
      break;


    case IDM_EDIT_INSERT_GUID:
      {
        GUID guid;
        if (SUCCEEDED(CoCreateGuid(&guid))) {  
          WCHAR wszGuid[40];
          if (StringFromGUID2(&guid,wszGuid,COUNTOF(wszGuid))) {
            WCHAR* pwszGuid = wszGuid + 1; // trim first brace char
            wszGuid[wcslen(wszGuid) - 1] = L'\0'; // trim last brace char 
            char mszGuid[40 * 4]; // UTF-8 max of 4 bytes per char
            if (WideCharToMultiByteStrg(Encoding_SciCP,pwszGuid,mszGuid)) {
              int token = BeginUndoAction();
              SendMessage(g_hwndEdit,SCI_REPLACESEL,0,(LPARAM)mszGuid);
              EndUndoAction(token);
            }
          }
        }
      }
      break;


    case IDM_EDIT_LINECOMMENT:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();

        switch (SendMessage(g_hwndEdit, SCI_GETLEXER, 0, 0)) {
        default:
        case SCLEX_NULL:
        case SCLEX_CSS:
        case SCLEX_DIFF:
        case SCLEX_MARKDOWN:
        case SCLEX_JSON:
          break;
        case SCLEX_HTML:
        case SCLEX_XML:
        case SCLEX_CPP:
        case SCLEX_PASCAL:
          EditToggleLineComments(g_hwndEdit, L"//", false);
          break;
        case SCLEX_VBSCRIPT:
        case SCLEX_VB:
          EditToggleLineComments(g_hwndEdit, L"'", false);
          break;
        case SCLEX_MAKEFILE:
        case SCLEX_PERL:
        case SCLEX_PYTHON:
        case SCLEX_CONF:
        case SCLEX_BASH:
        case SCLEX_TCL:
        case SCLEX_RUBY:
        case SCLEX_POWERSHELL:
        case SCLEX_CMAKE:
        case SCLEX_AVS:
        case SCLEX_YAML:
        case SCLEX_COFFEESCRIPT:
        case SCLEX_NIMROD:
          EditToggleLineComments(g_hwndEdit, L"#", true);
          break;
        case SCLEX_ASM:
        case SCLEX_PROPERTIES:
        case SCLEX_AU3:
        case SCLEX_AHK:
        case SCLEX_NSIS: // # could also be used instead
        case SCLEX_INNOSETUP:
        case SCLEX_REGISTRY:
          EditToggleLineComments(g_hwndEdit, L";", true);
          break;
        case SCLEX_SQL:
        case SCLEX_LUA:
        case SCLEX_VHDL:
          EditToggleLineComments(g_hwndEdit, L"--", true);
          break;
        case SCLEX_BATCH:
          EditToggleLineComments(g_hwndEdit, L"rem ", true);
          break;
        case SCLEX_LATEX:
        case SCLEX_MATLAB:
          EditToggleLineComments(g_hwndEdit, L"%", true);
          break;
        }

        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_STREAMCOMMENT:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();

        switch (SendMessage(g_hwndEdit, SCI_GETLEXER, 0, 0)) {
        default:
        case SCLEX_NULL:
        case SCLEX_VBSCRIPT:
        case SCLEX_MAKEFILE:
        case SCLEX_VB:
        case SCLEX_ASM:
        case SCLEX_SQL:
        case SCLEX_PERL:
        case SCLEX_PYTHON:
        case SCLEX_PROPERTIES:
        case SCLEX_CONF:
        case SCLEX_POWERSHELL:
        case SCLEX_BATCH:
        case SCLEX_DIFF:
        case SCLEX_BASH:
        case SCLEX_TCL:
        case SCLEX_AU3:
        case SCLEX_LATEX:
        case SCLEX_AHK:
        case SCLEX_RUBY:
        case SCLEX_CMAKE:
        case SCLEX_MARKDOWN:
        case SCLEX_YAML:
        case SCLEX_JSON:
        case SCLEX_REGISTRY:
        case SCLEX_NIMROD:
          break;
        case SCLEX_HTML:
        case SCLEX_XML:
        case SCLEX_CSS:
        case SCLEX_CPP:
        case SCLEX_NSIS:
        case SCLEX_AVS:
        case SCLEX_VHDL:
          EditEncloseSelection(g_hwndEdit, L"/*", L"*/");
          break;
        case SCLEX_PASCAL:
        case SCLEX_INNOSETUP:
          EditEncloseSelection(g_hwndEdit, L"{", L"}");
          break;
        case SCLEX_LUA:
          EditEncloseSelection(g_hwndEdit, L"--[[", L"]]");
          break;
        case SCLEX_COFFEESCRIPT:
          EditEncloseSelection(g_hwndEdit, L"###", L"###");
          break;
        case SCLEX_MATLAB:
          EditEncloseSelection(g_hwndEdit, L"%{", L"%}");
        }
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_URLENCODE:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditURLEncode(g_hwndEdit);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_URLDECODE:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditURLDecode(g_hwndEdit);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_ESCAPECCHARS:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditEscapeCChars(g_hwndEdit);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_UNESCAPECCHARS:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditUnescapeCChars(g_hwndEdit);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_CHAR2HEX:
      {
        int token = BeginUndoAction();
        EditChar2Hex(g_hwndEdit);
        EndUndoAction(token);
      }
      break;


    case IDM_EDIT_HEX2CHAR:
      EditHex2Char(g_hwndEdit);
      break;


    case IDM_EDIT_FINDMATCHINGBRACE:
      EditFindMatchingBrace(g_hwndEdit);
      break;


    case IDM_EDIT_SELTOMATCHINGBRACE:
      {
        int token = BeginUndoAction();
        EditSelectToMatchingBrace(g_hwndEdit);
        EndUndoAction(token);
      }
      break;


    // Main Bookmark Functions
    case BME_EDIT_BOOKMARKNEXT:
    {
        const DocPos iPos = SciCall_GetCurrentPos();
        const DocLn iLine = SciCall_LineFromPosition(iPos);

        int bitmask = (1 << MARKER_NP3_BOOKMARK);
        DocLn iNextLine = (DocLn)SendMessage( g_hwndEdit , SCI_MARKERNEXT , iLine+1 , bitmask );
        if (iNextLine == (DocLn)-1)
        {
            iNextLine = (DocLn)SendMessage( g_hwndEdit , SCI_MARKERNEXT , 0 , bitmask );
        }

        if (iNextLine != (DocLn)-1)
        {
            SciCall_EnsureVisible(iNextLine);
            SciCall_GotoLine(iNextLine);
            SciCall_ScrollCaret();
        }
        break;
    }

    case BME_EDIT_BOOKMARKPREV:
    {
        const DocPos iPos = SciCall_GetCurrentPos();
        const DocLn iLine = SciCall_LineFromPosition(iPos);

        int bitmask = (1 << MARKER_NP3_BOOKMARK);
        DocLn iNextLine = (DocLn)SendMessage( g_hwndEdit , SCI_MARKERPREVIOUS , iLine-1 , bitmask );
        if (iNextLine == (DocLn)-1)
        {
            iNextLine = (DocLn)SendMessage( g_hwndEdit , SCI_MARKERPREVIOUS , SciCall_GetLineCount(), bitmask );
        }

        if (iNextLine != (DocLn)-1)
        {
            SciCall_EnsureVisible(iNextLine);
            SciCall_GotoLine(iNextLine);
            SciCall_ScrollCaret();
        }
        break;
    }

    case BME_EDIT_BOOKMARKTOGGLE:
      {
        const DocPos iPos = SciCall_GetCurrentPos();
        const DocLn iLine = SciCall_LineFromPosition(iPos);

        int bitmask = SciCall_MarkerGet(iLine);

        if (bitmask & (1 << MARKER_NP3_BOOKMARK)) {
          // unset
          SciCall_MarkerDelete(iLine, MARKER_NP3_BOOKMARK);
        }
        else {
          Style_SetBookmark(g_hwndEdit, g_bShowSelectionMargin);
          // set
          SciCall_MarkerAdd(iLine, MARKER_NP3_BOOKMARK);
          UpdateLineNumberWidth();
        }
        break;
      }

    case BME_EDIT_BOOKMARKCLEAR:
      SciCall_MarkerDeleteAll(MARKER_NP3_BOOKMARK);
    break;



    case IDM_EDIT_FIND:
      if (!IsWindow(g_hwndDlgFindReplace)) {
        bFindReplCopySelOrClip = true;
        g_hwndDlgFindReplace = EditFindReplaceDlg(g_hwndEdit, &g_efrData, false);
      }
      else {
        bFindReplCopySelOrClip = (GetForegroundWindow() != g_hwndDlgFindReplace);
        if (GetDlgItem(g_hwndDlgFindReplace, IDC_REPLACE)) {
          SendMessage(g_hwndDlgFindReplace, WM_COMMAND, MAKELONG(IDMSG_SWITCHTOFIND, 1), 0);
          DestroyWindow(g_hwndDlgFindReplace);
          g_hwndDlgFindReplace = EditFindReplaceDlg(g_hwndEdit, &g_efrData, false);
        }
        else {
          SetForegroundWindow(g_hwndDlgFindReplace);
          PostMessage(g_hwndDlgFindReplace, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(g_hwndDlgFindReplace, IDC_FINDTEXT)), 1);
        }
        UpdateStatusbar(false);
      }
      break;


    case IDM_EDIT_REPLACE:
      if (!IsWindow(g_hwndDlgFindReplace)) {
        bFindReplCopySelOrClip = true;
        g_hwndDlgFindReplace = EditFindReplaceDlg(g_hwndEdit, &g_efrData, true);
      }
      else {
        bFindReplCopySelOrClip = (GetForegroundWindow() != g_hwndDlgFindReplace);
        if (!GetDlgItem(g_hwndDlgFindReplace, IDC_REPLACE)) {
          SendMessage(g_hwndDlgFindReplace, WM_COMMAND, MAKELONG(IDMSG_SWITCHTOREPLACE, 1), 0);
          DestroyWindow(g_hwndDlgFindReplace);
          g_hwndDlgFindReplace = EditFindReplaceDlg(g_hwndEdit, &g_efrData, true);
        }
        else {
          SetForegroundWindow(g_hwndDlgFindReplace);
          PostMessage(g_hwndDlgFindReplace, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(g_hwndDlgFindReplace, IDC_FINDTEXT)), 1);
        }
        UpdateStatusbar(false);
      }
      break;


    case IDM_EDIT_FINDNEXT:
    case IDM_EDIT_FINDPREV:
    case IDM_EDIT_REPLACENEXT:
    case IDM_EDIT_SELTONEXT:
    case IDM_EDIT_SELTOPREV:

      if (SciCall_GetTextLength() == 0)
        break;

      if (IsFindPatternEmpty() && !StringCchLenA(g_efrData.szFind, COUNTOF(g_efrData.szFind)))
      {
        if (LOWORD(wParam) != IDM_EDIT_REPLACENEXT)
          SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_FIND,1),0);
        else
          SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_REPLACE,1),0);
      }
      else {

        switch (LOWORD(wParam)) {

          case IDM_EDIT_FINDNEXT:
            if (!SciCall_IsSelectionEmpty()) {
              EditJumpToSelectionEnd(g_hwndEdit);
            }
            EditFindNext(g_hwndEdit,&g_efrData,false,false);
            break;

          case IDM_EDIT_FINDPREV:
            if (!SciCall_IsSelectionEmpty()) {
              EditJumpToSelectionStart(g_hwndEdit);
            }
            EditFindPrev(g_hwndEdit,&g_efrData,false,false);
            break;

          case IDM_EDIT_REPLACENEXT:
            if (bReplaceInitialized)
              EditReplace(g_hwndEdit,&g_efrData);
            else
              SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_REPLACE,1),0);
            break;

          case IDM_EDIT_SELTONEXT:
            if (!SciCall_IsSelectionEmpty()) {
              EditJumpToSelectionEnd(g_hwndEdit);
            }
            EditFindNext(g_hwndEdit,&g_efrData,true,false);
            break;

          case IDM_EDIT_SELTOPREV:
            if (!SciCall_IsSelectionEmpty()) {
              EditJumpToSelectionStart(g_hwndEdit);
            }
            EditFindPrev(g_hwndEdit,&g_efrData,true,false);
            break;
        }
      }
      break;


    case CMD_FINDNEXTSEL:
    case CMD_FINDPREVSEL:
    case IDM_EDIT_SAVEFIND:
    {
      DocPos cchSelection = SciCall_GetSelText(NULL);

      if (1 >= cchSelection)
      {
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_EDIT_SELECTWORD, 1), 0);
        cchSelection = SciCall_GetSelText(NULL);
      }

      if ((1 < cchSelection) && (cchSelection < FNDRPL_BUFFER))
      {
        char  mszSelection[FNDRPL_BUFFER];
        SciCall_GetSelText(mszSelection);

        // Check lpszSelection and truncate newlines
        char *lpsz = StrChrA(mszSelection, '\n');
        if (lpsz) *lpsz = '\0';

        lpsz = StrChrA(mszSelection, '\r');
        if (lpsz) *lpsz = '\0';

        StringCchCopyA(g_efrData.szFind, COUNTOF(g_efrData.szFind), mszSelection);
        g_efrData.fuFlags &= (~(SCFIND_REGEXP | SCFIND_POSIX));
        g_efrData.bTransformBS = false;

        WCHAR wszBuf[FNDRPL_BUFFER];
        MultiByteToWideCharStrg(Encoding_SciCP, mszSelection, wszBuf);
        MRU_Add(g_pMRUfind, wszBuf, 0, 0, NULL);
        SetFindPattern(wszBuf);

        switch (LOWORD(wParam)) {

        case IDM_EDIT_SAVEFIND:
          break;

        case CMD_FINDNEXTSEL:
          if (!SciCall_IsSelectionEmpty()) {
            EditJumpToSelectionEnd(g_hwndEdit);
          }
          EditFindNext(g_hwndEdit, &g_efrData, false, false);
          break;

        case CMD_FINDPREVSEL:
          if (!SciCall_IsSelectionEmpty()) {
            EditJumpToSelectionStart(g_hwndEdit);
          }
          EditFindPrev(g_hwndEdit, &g_efrData, false, false);
          break;
        }
      }
    }
    break;



    case IDM_EDIT_COMPLETEWORD:
      EditCompleteWord(g_hwndEdit, true);
      break;


    case IDM_EDIT_GOTOLINE:
      EditLinenumDlg(g_hwndEdit);
      UpdateStatusbar(false);
      break;


    case IDM_VIEW_SCHEME:
      Style_SelectLexerDlg(g_hwndEdit);
      UpdateToolbar();
      UpdateStatusbar(false);
      UpdateLineNumberWidth();
      break;


    case IDM_VIEW_USE2NDDEFAULT:
      Style_ToggleUse2ndDefault(g_hwndEdit);
      UpdateToolbar();
      UpdateStatusbar(false);
      UpdateLineNumberWidth();
      break;


    case IDM_VIEW_SCHEMECONFIG:
      if (!IsWindow(g_hwndDlgCustomizeSchemes)) {
        g_hwndDlgCustomizeSchemes = Style_CustomizeSchemesDlg(g_hwndEdit);
      }
      else {
        SetForegroundWindow(g_hwndDlgCustomizeSchemes);
      }
      PostMessage(g_hwndDlgCustomizeSchemes, WM_COMMAND, MAKELONG(IDC_SETCURLEXERTV, 1), 0);
      UpdateToolbar();
      UpdateStatusbar(false);
      UpdateLineNumberWidth();
      break;


    case IDM_VIEW_FONT:
      if (!IsWindow(g_hwndDlgCustomizeSchemes))
        Style_SetDefaultFont(g_hwndEdit, true);
      UpdateToolbar();
      UpdateLineNumberWidth();
      break;

    case IDM_VIEW_CURRENTSCHEME:
      if (!IsWindow(g_hwndDlgCustomizeSchemes))
        Style_SetDefaultFont(g_hwndEdit, false);
      UpdateToolbar();
      UpdateLineNumberWidth();
      break;


    case IDM_VIEW_WORDWRAP:
      g_bWordWrap = (g_bWordWrap) ? false : true;
      if (!g_bWordWrap)
        SendMessage(g_hwndEdit,SCI_SETWRAPMODE,SC_WRAP_NONE,0);
      else
        SendMessage(g_hwndEdit,SCI_SETWRAPMODE,(iWordWrapMode == 0) ? SC_WRAP_WHITESPACE : SC_WRAP_CHAR,0);
      bWordWrapG = g_bWordWrap;
      UpdateToolbar();
      break;


    case IDM_VIEW_WORDWRAPSETTINGS:
      if (WordWrapSettingsDlg(hwnd,IDD_WORDWRAP,&iWordWrapIndent)) {
        _SetWordWrapping(g_hwndEdit);
      }
      break;


    case IDM_VIEW_WORDWRAPSYMBOLS:
      bShowWordWrapSymbols = (bShowWordWrapSymbols) ? false : true;
      _SetWordWrapping(g_hwndEdit);
      break;


    case IDM_VIEW_LONGLINEMARKER:
      bMarkLongLines = (bMarkLongLines) ? false: true;
      if (bMarkLongLines) {
        SendMessage(g_hwndEdit,SCI_SETEDGEMODE,(iLongLineMode == EDGE_LINE)?EDGE_LINE:EDGE_BACKGROUND,0);
        Style_SetLongLineColors(g_hwndEdit);
      }
      else
        SendMessage(g_hwndEdit,SCI_SETEDGEMODE,EDGE_NONE,0);

      UpdateToolbar();
      UpdateStatusbar(false);
      break;


    case IDM_VIEW_LONGLINESETTINGS:
      if (LongLineSettingsDlg(hwnd,IDD_LONGLINES,&g_iLongLinesLimit)) {
        bMarkLongLines = true;
        SendMessage(g_hwndEdit,SCI_SETEDGEMODE,(iLongLineMode == EDGE_LINE)?EDGE_LINE:EDGE_BACKGROUND,0);
        Style_SetLongLineColors(g_hwndEdit);
        g_iLongLinesLimit = max(min(g_iLongLinesLimit,4096),0);
        SendMessage(g_hwndEdit,SCI_SETEDGECOLUMN,g_iLongLinesLimit,0);
        iLongLinesLimitG = g_iLongLinesLimit;
        UpdateToolbar();
        UpdateStatusbar(false);
      }
      break;


    case IDM_VIEW_TABSASSPACES:
      g_bTabsAsSpaces = (g_bTabsAsSpaces) ? false : true;
      SendMessage(g_hwndEdit,SCI_SETUSETABS,!g_bTabsAsSpaces,0);
      bTabsAsSpacesG = g_bTabsAsSpaces;
      break;


    case IDM_VIEW_TABSETTINGS:
      if (TabSettingsDlg(hwnd,IDD_TABSETTINGS,NULL))
      {
        SendMessage(g_hwndEdit,SCI_SETUSETABS,!g_bTabsAsSpaces,0);
        SendMessage(g_hwndEdit,SCI_SETTABINDENTS,g_bTabIndents,0);
        SendMessage(g_hwndEdit,SCI_SETBACKSPACEUNINDENTS,bBackspaceUnindents,0);
        g_iTabWidth = max(min(g_iTabWidth,256),1);
        g_iIndentWidth = max(min(g_iIndentWidth,256),0);
        SendMessage(g_hwndEdit,SCI_SETTABWIDTH,g_iTabWidth,0);
        SendMessage(g_hwndEdit,SCI_SETINDENT,g_iIndentWidth,0);
        bTabsAsSpacesG = g_bTabsAsSpaces;
        bTabIndentsG   = g_bTabIndents;
        iTabWidthG     = g_iTabWidth;
        iIndentWidthG  = g_iIndentWidth;
        if (SendMessage(g_hwndEdit,SCI_GETWRAPINDENTMODE,0,0) == SC_WRAPINDENT_FIXED) {
          int i = 0;
          switch (iWordWrapIndent) {
            case 1: i = 1; break;
            case 2: i = 2; break;
            case 3: i = (g_iIndentWidth) ? 1 * g_iIndentWidth : 1 * g_iTabWidth; break;
            case 4: i = (g_iIndentWidth) ? 2 * g_iIndentWidth : 2 * g_iTabWidth; break;
          }
          SendMessage(g_hwndEdit,SCI_SETWRAPSTARTINDENT,i,0);
        }
      }
      break;


    case IDM_VIEW_SHOWINDENTGUIDES:
      bShowIndentGuides = (bShowIndentGuides) ? false : true;
      Style_SetIndentGuides(g_hwndEdit,bShowIndentGuides);
      break;


    case IDM_VIEW_AUTOINDENTTEXT:
      bAutoIndent = (bAutoIndent) ? false : true;
      break;


    case IDM_VIEW_LINENUMBERS:
      bShowLineNumbers = (bShowLineNumbers) ? false : true;
      UpdateLineNumberWidth();
      break;


    case IDM_VIEW_MARGIN:
      g_bShowSelectionMargin = (g_bShowSelectionMargin) ? false : true;
      Style_SetBookmark(g_hwndEdit, g_bShowSelectionMargin);
      UpdateLineNumberWidth();
      break;

    case IDM_VIEW_AUTOCOMPLETEWORDS:
      bAutoCompleteWords = (bAutoCompleteWords) ? false : true;  // toggle
      if (!bAutoCompleteWords)
        SendMessage(g_hwndEdit, SCI_AUTOCCANCEL, 0, 0);  // close the auto completion list
      break;

    case IDM_VIEW_ACCELWORDNAV:
      bAccelWordNavigation = (bAccelWordNavigation) ? false : true;  // toggle  
      EditSetAccelWordNav(g_hwndEdit,bAccelWordNavigation);
      EditClearAllOccurrenceMarkers(g_hwndEdit, 0, -1);
      MarkAllOccurrences(iUpdateDelayMarkAllCoccurrences);
      break;

    case IDM_VIEW_MARKOCCUR_ONOFF:
      g_iMarkOccurrences = (g_iMarkOccurrences == 0) ? max(1, IniGetInt(L"Settings", L"MarkOccurrences", 1)) : 0;
      EditClearAllOccurrenceMarkers(g_hwndEdit, 0, -1);
      MarkAllOccurrences(0);
      EnableCmd(GetMenu(hwnd), IDM_VIEW_TOGGLE_VIEW, (g_iMarkOccurrences > 0) && !g_bMarkOccurrencesMatchVisible);
      break;

    case IDM_VIEW_MARKOCCUR_VISIBLE:
      g_bMarkOccurrencesMatchVisible = (g_bMarkOccurrencesMatchVisible) ? false : true;
      EditClearAllOccurrenceMarkers(g_hwndEdit, 0, -1);
      MarkAllOccurrences(0);
      EnableCmd(GetMenu(hwnd), IDM_VIEW_TOGGLE_VIEW, (g_iMarkOccurrences > 0) && !g_bMarkOccurrencesMatchVisible);
      break;

    case IDM_VIEW_TOGGLE_VIEW:
      if (EditToggleView(g_hwndEdit, false)) {
        EditToggleView(g_hwndEdit, true);
        EditClearAllOccurrenceMarkers(g_hwndEdit, 0, -1);
        MarkAllOccurrences(0);
      }
      else {
        EditToggleView(g_hwndEdit, true);
      }
      CheckCmd(GetMenu(hwnd), IDM_VIEW_TOGGLE_VIEW, EditToggleView(g_hwndEdit, false));
      UpdateToolbar();
      break;

    case IDM_VIEW_MARKOCCUR_CASE:
      bMarkOccurrencesMatchCase = (bMarkOccurrencesMatchCase) ? false : true;
      EditClearAllOccurrenceMarkers(g_hwndEdit, 0, -1);
      MarkAllOccurrences(iUpdateDelayMarkAllCoccurrences);
      break;

    case IDM_VIEW_MARKOCCUR_WNONE:
      bMarkOccurrencesMatchWords = false;
      bMarkOccurrencesCurrentWord = false;
      EditClearAllOccurrenceMarkers(g_hwndEdit, 0, -1);
      MarkAllOccurrences(iUpdateDelayMarkAllCoccurrences);
      break;

    case IDM_VIEW_MARKOCCUR_WORD:
      bMarkOccurrencesMatchWords = true;
      bMarkOccurrencesCurrentWord = false;
      EditClearAllOccurrenceMarkers(g_hwndEdit, 0, -1);
      MarkAllOccurrences(iUpdateDelayMarkAllCoccurrences);
      break;

    case IDM_VIEW_MARKOCCUR_CURRENT:
      bMarkOccurrencesMatchWords = false;
      bMarkOccurrencesCurrentWord = true;
      EditClearAllOccurrenceMarkers(g_hwndEdit, 0, -1);
      MarkAllOccurrences(iUpdateDelayMarkAllCoccurrences);
      break;

    case IDM_VIEW_FOLDING:
      g_bShowCodeFolding = (g_bShowCodeFolding) ? false : true;
      Style_SetFolding(g_hwndEdit, g_bShowCodeFolding);
      if (!g_bShowCodeFolding) { EditToggleFolds(EXPAND, true); }
      UpdateToolbar();
      break;


    case IDM_VIEW_TOGGLEFOLDS:
      EditToggleFolds(SNIFF, false);
      break;


    case IDM_VIEW_SHOWWHITESPACE:
      bViewWhiteSpace = (bViewWhiteSpace) ? false : true;
      SendMessage(g_hwndEdit,SCI_SETVIEWWS,(bViewWhiteSpace)?SCWS_VISIBLEALWAYS:SCWS_INVISIBLE,0);
      break;


    case IDM_VIEW_SHOWEOLS:
      bViewEOLs = (bViewEOLs) ? false : true;
      SendMessage(g_hwndEdit,SCI_SETVIEWEOL,bViewEOLs,0);
      break;


    case IDM_VIEW_MATCHBRACES:
      bMatchBraces = (bMatchBraces) ? false : true;
      if (bMatchBraces)
        EditMatchBrace(g_hwndEdit);
      else
        SendMessage(g_hwndEdit,SCI_BRACEHIGHLIGHT,(WPARAM)-1,(LPARAM)-1);
      break;


    case IDM_VIEW_AUTOCLOSETAGS:
      bAutoCloseTags = (bAutoCloseTags) ? false : true;
      break;

    case IDM_VIEW_HILITECURRENTLINE:
      bHiliteCurrentLine = (bHiliteCurrentLine) ? false : true;
      Style_SetCurrentLineBackground(g_hwndEdit, bHiliteCurrentLine);
      break;

    case IDM_VIEW_HYPERLINKHOTSPOTS:
      g_bHyperlinkHotspot = (g_bHyperlinkHotspot) ? false : true;
      Style_SetUrlHotSpot(g_hwndEdit, g_bHyperlinkHotspot);
      if (g_bHyperlinkHotspot) {
        UpdateVisibleUrlHotspot(0);
      }
      else {
        SciCall_StartStyling(0);
        Style_ResetCurrentLexer(g_hwndEdit);
      }
      break;

    case IDM_VIEW_ZOOMIN:
      SendMessage(g_hwndEdit,SCI_ZOOMIN,0,0);
      UpdateLineNumberWidth();
      break;

    case IDM_VIEW_ZOOMOUT:
      SendMessage(g_hwndEdit,SCI_ZOOMOUT,0,0);
      UpdateLineNumberWidth();
      break;

    case IDM_VIEW_RESETZOOM:
      SendMessage(g_hwndEdit,SCI_SETZOOM,0,0);
      UpdateLineNumberWidth();
      break;

    case IDM_VIEW_CHASING_DOCTAIL: 
      {
        static int flagPrevChangeNotify = 0;
        static int iPrevFileWatchingMode = 0;
        static bool bPrevResetFileWatching = false;

        g_bChasingDocTail = (g_bChasingDocTail) ? false : true;
        SciCall_SetReadOnly(g_bChasingDocTail);

        if (g_bChasingDocTail) 
        {
          SetForegroundWindow(hwnd);
          flagPrevChangeNotify = flagChangeNotify;
          iPrevFileWatchingMode = g_iFileWatchingMode;
          bPrevResetFileWatching = g_bResetFileWatching;
          flagChangeNotify = 2;
          g_iFileWatchingMode = 2;
          g_bResetFileWatching = true;
        }
        else {
          flagChangeNotify = flagPrevChangeNotify;
          g_iFileWatchingMode = iPrevFileWatchingMode;
          g_bResetFileWatching = bPrevResetFileWatching;
        }
        if (!g_bRunningWatch) { InstallFileWatching(g_wchCurFile); }

        CheckCmd(GetMenu(g_hwndMain), IDM_VIEW_CHASING_DOCTAIL, g_bChasingDocTail);
        UpdateToolbar();
      }
      break;
    
    case IDM_VIEW_SCROLLPASTEOF:
      bScrollPastEOF = (bScrollPastEOF) ? false : true;
      SciCall_SetEndAtLastLine(!bScrollPastEOF);
      break;

    case IDM_VIEW_TOOLBAR:
      if (bShowToolbar) {
        bShowToolbar = 0;
        ShowWindow(g_hwndReBar,SW_HIDE);
      }
      else {
        bShowToolbar = 1;
        UpdateToolbar();
        ShowWindow(g_hwndReBar,SW_SHOW);
      }
      SendWMSize(hwnd);
      break;


    case IDM_VIEW_TOGGLETB:
      iHighDpiToolBar = (iHighDpiToolBar <= 0) ? 1 : 0;
      SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
      break;

    case IDM_VIEW_CUSTOMIZETB:
      SendMessage(g_hwndToolbar,TB_CUSTOMIZE,0,0);
      break;


    case IDM_VIEW_STATUSBAR:
      if (bShowStatusbar) {
        bShowStatusbar = 0;
        ShowWindow(g_hwndStatus,SW_HIDE);
      }
      else {
        bShowStatusbar = 1;
        UpdateStatusbar(false);
        ShowWindow(g_hwndStatus,SW_SHOW);
      }
      SendWMSize(hwnd);
      break;


    case IDM_VIEW_STICKYWINPOS:
      bStickyWinPos = IniGetInt(L"Settings2",L"StickyWindowPosition",bStickyWinPos);
      if (!bStickyWinPos) 
      {
        WCHAR tchPosX[32], tchPosY[32], tchSizeX[32], tchSizeY[32], tchMaximized[32];

        int ResX = GetSystemMetrics(SM_CXSCREEN);
        int ResY = GetSystemMetrics(SM_CYSCREEN);

        StringCchPrintf(tchPosX,COUNTOF(tchPosX),L"%ix%i PosX",ResX,ResY);
        StringCchPrintf(tchPosY,COUNTOF(tchPosY),L"%ix%i PosY",ResX,ResY);
        StringCchPrintf(tchSizeX,COUNTOF(tchSizeX),L"%ix%i SizeX",ResX,ResY);
        StringCchPrintf(tchSizeY,COUNTOF(tchSizeY),L"%ix%i SizeY",ResX,ResY);
        StringCchPrintf(tchMaximized,COUNTOF(tchMaximized),L"%ix%i Maximized",ResX,ResY);

        bStickyWinPos = 1;
        IniSetInt(L"Settings2",L"StickyWindowPosition",1);

        // GetWindowPlacement
        WININFO wi = GetMyWindowPlacement(g_hwndMain,NULL);
        IniSetInt(L"Window",tchPosX,wi.x);
        IniSetInt(L"Window",tchPosY,wi.y);
        IniSetInt(L"Window",tchSizeX,wi.cx);
        IniSetInt(L"Window",tchSizeY,wi.cy);
        IniSetInt(L"Window",tchMaximized,wi.max);

        InfoBox(0,L"MsgStickyWinPos",IDS_STICKYWINPOS);
      }
      else {
        bStickyWinPos = 0;
        IniSetInt(L"Settings2",L"StickyWindowPosition",0);
      }
      break;


    case IDM_VIEW_REUSEWINDOW:
      if (IniGetInt(L"Settings2",L"ReuseWindow",0))
        IniSetInt(L"Settings2",L"ReuseWindow",0);
      else
        IniSetInt(L"Settings2",L"ReuseWindow",1);
      break;


    case IDM_VIEW_SINGLEFILEINSTANCE:
      if (IniGetInt(L"Settings2",L"SingleFileInstance",0))
        IniSetInt(L"Settings2",L"SingleFileInstance",0);
      else
        IniSetInt(L"Settings2",L"SingleFileInstance",1);
      break;


    case IDM_VIEW_ALWAYSONTOP:
      if ((bAlwaysOnTop || flagAlwaysOnTop == 2) && flagAlwaysOnTop != 1) {
        bAlwaysOnTop = 0;
        flagAlwaysOnTop = 0;
        SetWindowPos(hwnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
      }
      else {
        bAlwaysOnTop = 1;
        flagAlwaysOnTop = 0;
        SetWindowPos(hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
      }
      break;


    case IDM_VIEW_MINTOTRAY:
      bMinimizeToTray =(bMinimizeToTray) ? false : true;
      break;


    case IDM_VIEW_TRANSPARENT:
      bTransparentMode =(bTransparentMode) ? false : true;
      SetWindowTransparentMode(hwnd,bTransparentMode);
      break;


    case IDM_VIEW_SHOWFILENAMEONLY:
      iPathNameFormat = 0;
      StringCchCopy(szTitleExcerpt,COUNTOF(szTitleExcerpt),L"");
      UpdateToolbar();
      break;


    case IDM_VIEW_SHOWFILENAMEFIRST:
      iPathNameFormat = 1;
      StringCchCopy(szTitleExcerpt,COUNTOF(szTitleExcerpt),L"");
      UpdateToolbar();
      break;


    case IDM_VIEW_SHOWFULLPATH:
      iPathNameFormat = 2;
      StringCchCopy(szTitleExcerpt,COUNTOF(szTitleExcerpt),L"");
      UpdateToolbar();
      break;


    case IDM_VIEW_SHOWEXCERPT:
      EditGetExcerpt(g_hwndEdit,szTitleExcerpt,COUNTOF(szTitleExcerpt));
      UpdateToolbar();
      break;


    case IDM_VIEW_NOSAVERECENT:
      bSaveRecentFiles = (bSaveRecentFiles) ? false : true;
      break;


    case IDM_VIEW_NOPRESERVECARET:
      bPreserveCaretPos = (bPreserveCaretPos) ? false : true;
      break;


    case IDM_VIEW_NOSAVEFINDREPL:
      bSaveFindReplace = (bSaveFindReplace) ? false : true;
      break;


    case IDM_VIEW_SAVEBEFORERUNNINGTOOLS:
      bSaveBeforeRunningTools = (bSaveBeforeRunningTools) ? false : true;
      break;


    case IDM_VIEW_CHANGENOTIFY:
      if (ChangeNotifyDlg(hwnd))
        InstallFileWatching(g_wchCurFile);
      break;


    case IDM_VIEW_NOESCFUNC:
      iEscFunction = 0;
      break;


    case IDM_VIEW_ESCMINIMIZE:
      iEscFunction = 1;
      break;


    case IDM_VIEW_ESCEXIT:
      iEscFunction = 2;
      break;


    case IDM_VIEW_SAVESETTINGS:
      if (IsCmdEnabled(hwnd, IDM_VIEW_SAVESETTINGS))
        bSaveSettings = (bSaveSettings) ? false : true;
      break;


    case IDM_VIEW_SAVESETTINGSNOW:
      if (IsCmdEnabled(hwnd, IDM_VIEW_SAVESETTINGSNOW)) {

        bool bCreateFailure = false;

        if (StringCchLenW(g_wchIniFile,COUNTOF(g_wchIniFile)) == 0) {

          if (StringCchLenW(g_wchIniFile2,COUNTOF(g_wchIniFile2)) > 0) {
            if (CreateIniFileEx(g_wchIniFile2)) {
              StringCchCopy(g_wchIniFile,COUNTOF(g_wchIniFile),g_wchIniFile2);
              StringCchCopy(g_wchIniFile2,COUNTOF(g_wchIniFile2),L"");
            }
            else
              bCreateFailure = true;
          }

          else
            break;
        }

        if (!bCreateFailure) {

          if (WritePrivateProfileString(L"Settings",L"WriteTest",L"ok",g_wchIniFile)) {

            BeginWaitCursor(L"Saving settings..."); // IDS_SAVINGSETTINGS
            SaveSettings(true);
            EndWaitCursor();
            MsgBox(MBINFO,IDS_SAVEDSETTINGS);
          }
          else {
            dwLastIOError = GetLastError();
            MsgBox(MBWARN,IDS_WRITEINI_FAIL);
          }
        }
        else
          MsgBox(MBWARN,IDS_CREATEINI_FAIL);
      }
      break;


    case IDM_HELP_ONLINEDOCUMENTATION:
      ShellExecute(0, 0, ONLINE_HELP_WEBSITE, 0, 0, SW_SHOW);
      break;

    case IDM_HELP_ABOUT:
        ThemedDialogBox(g_hInstance, MAKEINTRESOURCE(IDD_ABOUT), hwnd, AboutDlgProc);
      break;

    case IDM_SETPASS:
      if (GetFileKey(g_hwndEdit)) {
        _SetDocumentModified(true);
      }
      break;

    case IDM_HELP_CMD:
      DisplayCmdLineHelp(hwnd);
      break;

    case CMD_ESCAPE:
      //close the autocomplete box
      SendMessage(g_hwndEdit,SCI_AUTOCCANCEL,0, 0);

      if (iEscFunction == 1)
        SendMessage(hwnd,WM_SYSCOMMAND,SC_MINIMIZE,0);
      else if (iEscFunction == 2)
        SendMessage(hwnd,WM_CLOSE,0,0);
      break;


    case CMD_SHIFTESC:
      if (FileSave(true,false,false,false))
        SendMessage(hwnd,WM_CLOSE,0,0);
      break;


    case CMD_CTRLENTER:
      {
        int token = BeginUndoAction();
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
        EndUndoAction(token);
      }
      break;


    // Newline with toggled auto indent setting
    case CMD_SHIFTCTRLENTER:
      bAutoIndent = (bAutoIndent) ? 0 : 1;
      SciCall_NewLine();
      bAutoIndent = (bAutoIndent) ? 0 : 1;
      break;


    case IDM_EDIT_CLEAR:
    case CMD_DEL:
      {
        int token = BeginUndoAction();
        SciCall_Clear();
        EndUndoAction(token);
      }
      break;


    case CMD_CTRLLEFT:
        SendMessage(g_hwndEdit, SCI_WORDLEFT, 0, 0);
      break;


    case CMD_CTRLRIGHT:
        SendMessage(g_hwndEdit, SCI_WORDRIGHT, 0, 0);
      break;


    case CMD_CTRLBACK:
      {
        const DocPos iPos = SciCall_GetCurrentPos();
        const DocPos iAnchor = SciCall_GetAnchor();
        const DocLn iLine = SciCall_LineFromPosition(iPos);
        const DocPos iStartPos = SciCall_PositionFromLine(iLine);
        const DocPos iIndentPos = SciCall_GetLineIndentPosition(iLine);

        if (iPos != iAnchor) {
          int token = BeginUndoAction();
          SciCall_SetSel(iPos, iPos);
          EndUndoAction(token);
        }
        else {
          if (iPos == iStartPos)
            Sci_SendMsgV0(DELETEBACK);
          else if (iPos <= iIndentPos)
            Sci_SendMsgV0(DELLINELEFT);
          else
            Sci_SendMsgV0(DELWORDLEFT);
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
          int token = BeginUndoAction();
          SciCall_SetSel(iPos, iPos);
          EndUndoAction(token);
        }
        else {
          if (iStartPos != iEndPos)
            Sci_SendMsgV0(DELWORDRIGHT);
          else // iStartPos == iEndPos
            Sci_SendMsgV0(LINEDELETE);
        }
      }
      break;


    case CMD_RECODEDEFAULT:
      {
        WCHAR tchCurFile2[MAX_PATH] = { L'\0' };
        if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
          Encoding_SrcCmdLn(Encoding_MapUnicode(g_iDefaultNewFileEncoding));
          StringCchCopy(tchCurFile2,COUNTOF(tchCurFile2),g_wchCurFile);
          FileLoad(false,false,true,true,true,tchCurFile2);
        }
      }
      break;


    case CMD_RECODEANSI:
      {
        WCHAR tchCurFile2[MAX_PATH] = { L'\0' };
        if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
          Encoding_SrcCmdLn(CPI_ANSI_DEFAULT);
          StringCchCopy(tchCurFile2,COUNTOF(tchCurFile2),g_wchCurFile);
          FileLoad(false,false,true,true,bSkipANSICodePageDetection,tchCurFile2);
        }
      }
      break;


    case CMD_RECODEOEM:
      {
        WCHAR tchCurFile2[MAX_PATH] = { L'\0' };
        if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
          Encoding_SrcCmdLn(CPI_OEM);
          StringCchCopy(tchCurFile2,COUNTOF(tchCurFile2),g_wchCurFile);
          FileLoad(false,false,true,true,true,tchCurFile2);
        }
      }
      break;


    case CMD_RELOADASCIIASUTF8:
      {
        WCHAR tchCurFile2[MAX_PATH] = { L'\0' };
        bool _bLoadASCIIasUTF8 = bLoadASCIIasUTF8;
        if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
          bLoadASCIIasUTF8 = 1;
          StringCchCopy(tchCurFile2,COUNTOF(tchCurFile2),g_wchCurFile);
          FileLoad(false,false,true,false,true,tchCurFile2);
          bLoadASCIIasUTF8 = _bLoadASCIIasUTF8;
        }
      }
      break;


    case CMD_RELOADNOFILEVARS:
      {
        WCHAR tchCurFile2[MAX_PATH] = { L'\0' };
        if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
          int _fNoFileVariables = flagNoFileVariables;
          bool _bNoEncodingTags = bNoEncodingTags;
          flagNoFileVariables = 1;
          bNoEncodingTags = 1;
          StringCchCopy(tchCurFile2,COUNTOF(tchCurFile2),g_wchCurFile);
          FileLoad(false,false,true, bSkipUnicodeDetection, bSkipANSICodePageDetection, tchCurFile2);
          flagNoFileVariables = _fNoFileVariables;
          bNoEncodingTags = _bNoEncodingTags;
        }
      }
      break;


    case CMD_LEXDEFAULT:
      Style_SetDefaultLexer(g_hwndEdit);
      UpdateToolbar();
      UpdateStatusbar(false);
      UpdateLineNumberWidth();
      break;


    case CMD_LEXHTML:
      Style_SetHTMLLexer(g_hwndEdit);
      UpdateToolbar();
      UpdateStatusbar(false);
      UpdateLineNumberWidth();
      break;


    case CMD_LEXXML:
      Style_SetXMLLexer(g_hwndEdit);
      UpdateToolbar();
      UpdateStatusbar(false);
      UpdateLineNumberWidth();
      break;


    case CMD_TIMESTAMPS:
      {
        WCHAR wchFind[256] = { L'\0' };
        WCHAR wchTemplate[256] = { L'\0' };
        WCHAR wchReplace[256] = { L'\0' };

        SYSTEMTIME st;
        struct tm sst;

        EDITFINDREPLACE efrTS = EFR_INIT_DATA;
        efrTS.hwnd = g_hwndEdit;
        efrTS.fuFlags = SCFIND_REGEXP;

        IniGetString(L"Settings2",L"TimeStamp",L"\\$Date:[^\\$]+\\$ | $Date: %Y/%m/%d %H:%M:%S $",wchFind,COUNTOF(wchFind));

        WCHAR *pwchSep = StrChr(wchFind, L'|');
        if (pwchSep) {
          StringCchCopy(wchTemplate,COUNTOF(wchTemplate),pwchSep + 1);
          *pwchSep = 0;
        }

        StrTrim(wchFind,L" ");
        StrTrim(wchTemplate,L" ");

        if (StringCchLenW(wchFind,COUNTOF(wchFind)) == 0 || StringCchLenW(wchTemplate,COUNTOF(wchTemplate)) == 0)
          break;

        GetLocalTime(&st);
        sst.tm_isdst = -1;
        sst.tm_sec   = (int)st.wSecond;
        sst.tm_min   = (int)st.wMinute;
        sst.tm_hour  = (int)st.wHour;
        sst.tm_mday  = (int)st.wDay;
        sst.tm_mon   = (int)st.wMonth - 1;
        sst.tm_year  = (int)st.wYear - 1900;
        sst.tm_wday  = (int)st.wDayOfWeek;
        mktime(&sst);
        wcsftime(wchReplace,COUNTOF(wchReplace),wchTemplate,&sst);

        WideCharToMultiByteStrg(Encoding_SciCP,wchFind,efrTS.szFind);
        WideCharToMultiByteStrg(Encoding_SciCP,wchReplace,efrTS.szReplace);

        if (!SendMessage(g_hwndEdit, SCI_GETSELECTIONEMPTY, 0, 0))
          EditReplaceAllInSelection(g_hwndEdit, &efrTS, true);
        else
          EditReplaceAll(g_hwndEdit,&efrTS,true);
      }
      break;


    case IDM_HELP_UPDATEINSTALLER:
      DialogUpdateCheck(hwnd, true);
      break;

    case IDM_HELP_UPDATEWEBSITE:
      DialogUpdateCheck(hwnd, false);
      break;

    case CMD_WEBACTION1:
    case CMD_WEBACTION2:
      {
        WCHAR  szCmdTemplate[256] = { L'\0' };

        LPWSTR lpszTemplateName = (LOWORD(wParam) == CMD_WEBACTION1) ? L"WebTemplate1" : L"WebTemplate2";

        bool bCmdEnabled = IniGetString(L"Settings2",lpszTemplateName,L"",szCmdTemplate,COUNTOF(szCmdTemplate));

        if (bCmdEnabled) {

          const DocPos cchSelection = SciCall_GetSelText(NULL);

          char  mszSelection[512] = { '\0' };
          if ((1 < cchSelection) && (cchSelection < (DocPos)COUNTOF(mszSelection)))
          {
            SciCall_GetSelText(mszSelection);

            // Check lpszSelection and truncate bad WCHARs
            char* lpsz = StrChrA(mszSelection,13);
            if (lpsz) *lpsz = '\0';

            lpsz = StrChrA(mszSelection,10);
            if (lpsz) *lpsz = '\0';

            lpsz = StrChrA(mszSelection,9);
            if (lpsz) *lpsz = '\0';

            if (StringCchLenA(mszSelection,COUNTOF(mszSelection))) {

              WCHAR wszSelection[512] = { L'\0' };
              MultiByteToWideCharStrg(Encoding_SciCP,mszSelection,wszSelection);

              int cmdsz = (512 + COUNTOF(szCmdTemplate) + MAX_PATH + 32);
              LPWSTR lpszCommand = AllocMem(sizeof(WCHAR)*cmdsz, HEAP_ZERO_MEMORY);
              StringCchPrintf(lpszCommand,cmdsz,szCmdTemplate,wszSelection);
              ExpandEnvironmentStringsEx(lpszCommand, cmdsz);

              WCHAR wchDirectory[MAX_PATH] = { L'\0' };
              if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
                StringCchCopy(wchDirectory,COUNTOF(wchDirectory),g_wchCurFile);
                PathRemoveFileSpec(wchDirectory);
              }

              SHELLEXECUTEINFO sei;
              ZeroMemory(&sei,sizeof(SHELLEXECUTEINFO));
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
      }
      break;


    case CMD_INCLINELIMIT:
    case CMD_DECLINELIMIT:
      if (!bMarkLongLines)
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_VIEW_LONGLINEMARKER,1),0);
      else {
        if (LOWORD(wParam) == CMD_INCLINELIMIT)
          g_iLongLinesLimit++;
        else
          g_iLongLinesLimit--;
        g_iLongLinesLimit = max(min(g_iLongLinesLimit,4096),0);
        SendMessage(g_hwndEdit,SCI_SETEDGECOLUMN,g_iLongLinesLimit,0);
        UpdateToolbar();
        UpdateStatusbar(false);
        iLongLinesLimitG = g_iLongLinesLimit;
      }
      break;

    case CMD_STRINGIFY:
      {
        int token = BeginUndoAction();
        EditEncloseSelection(g_hwndEdit, L"'", L"'");
        EndUndoAction(token);
      }
      break;


    case CMD_STRINGIFY2:
      {
        int token = BeginUndoAction();
        EditEncloseSelection(g_hwndEdit, L"\"", L"\"");
        EndUndoAction(token);
      }
      break;


    case CMD_EMBRACE:
      {
        int token = BeginUndoAction();
        EditEncloseSelection(g_hwndEdit, L"(", L")");
        EndUndoAction(token);
      }
      break;


    case CMD_EMBRACE2:
      {
        int token = BeginUndoAction();
        EditEncloseSelection(g_hwndEdit, L"[", L"]");
        EndUndoAction(token);
      }
      break;


    case CMD_EMBRACE3:
      {
        int token = BeginUndoAction();
        EditEncloseSelection(g_hwndEdit, L"{", L"}");
        EndUndoAction(token);
      }
      break;


    case CMD_EMBRACE4:
      {
        int token = BeginUndoAction();
        EditEncloseSelection(g_hwndEdit, L"`", L"`");
        EndUndoAction(token);
      }
      break;


    case CMD_INCREASENUM:
      EditModifyNumber(g_hwndEdit,true);
      break;


    case CMD_DECREASENUM:
      EditModifyNumber(g_hwndEdit,false);
      break;


    case CMD_TOGGLETITLE:
      EditGetExcerpt(g_hwndEdit,szTitleExcerpt,COUNTOF(szTitleExcerpt));
      UpdateToolbar();
      break;


    case CMD_JUMP2SELSTART:
      EditJumpToSelectionStart(g_hwndEdit);
      SciCall_ChooseCaretX();
      break;

    case CMD_JUMP2SELEND:
      EditJumpToSelectionEnd(g_hwndEdit);
      SciCall_ChooseCaretX();
      break;


    case CMD_COPYPATHNAME: {

        WCHAR *pszCopy;
        WCHAR tchUntitled[32] = { L'\0' };
        if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile)))
          pszCopy = g_wchCurFile;
        else {
          GetString(IDS_UNTITLED,tchUntitled,COUNTOF(tchUntitled));
          pszCopy = tchUntitled;
        }
        SetClipboardTextW(hwnd, pszCopy);
        UpdateToolbar();
      }
      break;


    case CMD_COPYWINPOS: {

        WCHAR wszWinPos[MIDSZ_BUFFER];
        WININFO wi = GetMyWindowPlacement(g_hwndMain,NULL);
        StringCchPrintf(wszWinPos,COUNTOF(wszWinPos),L"/pos %i,%i,%i,%i,%i",wi.x,wi.y,wi.cx,wi.cy,wi.max);
        SetClipboardTextW(hwnd, wszWinPos);
        UpdateToolbar();
      }
      break;


    case CMD_DEFAULTWINPOS:
      SnapToDefaultPos(hwnd);
      break;


    case CMD_OPENINIFILE:
      if (StringCchLenW(g_wchIniFile,COUNTOF(g_wchIniFile))) {
        CreateIniFile();
        FileLoad(false,false,false,false,true,g_wchIniFile);
      }
      break;


    case CMD_OPEN_HYPERLINK:
        OpenHotSpotURL(SciCall_GetCurrentPos(), false);
      break;


    case CMD_ALTDOWN:
      EditFoldAltArrow(DOWN, SNIFF);
      break;

    case CMD_ALTUP:
      EditFoldAltArrow(UP, SNIFF);
      break;

    case CMD_ALTLEFT:
      EditFoldAltArrow(NONE, FOLD);
      break;

    case CMD_ALTRIGHT:
      EditFoldAltArrow(NONE, EXPAND);
      break;


    case IDT_FILE_NEW:
      if (IsCmdEnabled(hwnd,IDM_FILE_NEW))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_NEW,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_FILE_OPEN:
      if (IsCmdEnabled(hwnd,IDM_FILE_OPEN))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_OPEN,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_FILE_BROWSE:
      if (IsCmdEnabled(hwnd,IDM_FILE_BROWSE))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_BROWSE,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_FILE_SAVE:
      if (IsCmdEnabled(hwnd,IDM_FILE_SAVE))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_SAVE,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_EDIT_UNDO:
      if (IsCmdEnabled(hwnd,IDM_EDIT_UNDO))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_UNDO,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_EDIT_REDO:
      if (IsCmdEnabled(hwnd,IDM_EDIT_REDO))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_REDO,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_EDIT_CUT:
      if (IsCmdEnabled(hwnd,IDM_EDIT_CUT))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_CUT,1),0);
      else
        MessageBeep(0);
        //SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_CUTLINE,1),0);
      break;


    case IDT_EDIT_COPY:
      if (IsCmdEnabled(hwnd,IDM_EDIT_COPY))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_COPY,1),0);
      else
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_COPYALL,1),0);     // different to Keyboard-Shortcut
      break;


    case IDT_EDIT_PASTE:
      if (IsCmdEnabled(hwnd,IDM_EDIT_PASTE))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_PASTE,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_EDIT_FIND:
      if (IsCmdEnabled(hwnd,IDM_EDIT_FIND))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_FIND,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_EDIT_REPLACE:
      if (IsCmdEnabled(hwnd,IDM_EDIT_REPLACE))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_REPLACE,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_VIEW_WORDWRAP:
      if (IsCmdEnabled(hwnd,IDM_VIEW_WORDWRAP))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_VIEW_WORDWRAP,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_VIEW_ZOOMIN:
      if (IsCmdEnabled(hwnd,IDM_VIEW_ZOOMIN))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_VIEW_ZOOMIN,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_VIEW_ZOOMOUT:
      if (IsCmdEnabled(hwnd,IDM_VIEW_ZOOMOUT))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_VIEW_ZOOMOUT,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_VIEW_CHASING_DOCTAIL:
      if (IsCmdEnabled(hwnd, IDM_VIEW_CHASING_DOCTAIL))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_VIEW_CHASING_DOCTAIL,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_VIEW_SCHEME:
      if (IsCmdEnabled(hwnd,IDM_VIEW_SCHEME))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_VIEW_SCHEME,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_VIEW_SCHEMECONFIG:
      if (IsCmdEnabled(hwnd,IDM_VIEW_SCHEMECONFIG))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_VIEW_SCHEMECONFIG,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_FILE_EXIT:
      SendMessage(hwnd,WM_CLOSE,0,0);
      break;


    case IDT_FILE_SAVEAS:
      if (IsCmdEnabled(hwnd,IDM_FILE_SAVEAS))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_SAVEAS,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_FILE_SAVECOPY:
      if (IsCmdEnabled(hwnd,IDM_FILE_SAVECOPY))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_SAVECOPY,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_EDIT_CLEAR:
      if (IsCmdEnabled(hwnd,IDM_EDIT_CLEAR))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_CLEAR,1),0);
      else
        SendMessage(g_hwndEdit,SCI_CLEARALL,0,0);
      break;


    case IDT_FILE_PRINT:
      if (IsCmdEnabled(hwnd,IDM_FILE_PRINT))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_PRINT,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_FILE_OPENFAV:
      if (IsCmdEnabled(hwnd,IDM_FILE_OPENFAV))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_OPENFAV,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_FILE_ADDTOFAV:
      if (IsCmdEnabled(hwnd,IDM_FILE_ADDTOFAV))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_ADDTOFAV,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_VIEW_TOGGLEFOLDS:
      if (IsCmdEnabled(hwnd,IDM_VIEW_TOGGLEFOLDS))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_VIEW_TOGGLEFOLDS,1),0);
      else
        MessageBeep(0);
      break;

      
    case IDT_VIEW_TOGGLE_VIEW:
      if (IsCmdEnabled(hwnd, IDM_VIEW_TOGGLE_VIEW))
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_VIEW_TOGGLE_VIEW, 1), 0);
      else
        MessageBeep(0);
      break;


    case IDT_FILE_LAUNCH:
      if (IsCmdEnabled(hwnd,IDM_FILE_LAUNCH))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_LAUNCH,1),0);
      else
        MessageBeep(0);
      break;

  }

  UNUSED(wParam);
  UNUSED(lParam);

  return(0);
}


//=============================================================================
//
//  OpenHotSpotURL()
//
//
void OpenHotSpotURL(DocPos position, bool bForceBrowser)
{
  int iStyle = (int)SendMessage(g_hwndEdit, SCI_GETSTYLEAT, position, 0);

  if (Style_GetHotspotStyleID() != iStyle)
    return; 

  if (!(bool)SendMessage(g_hwndEdit, SCI_STYLEGETHOTSPOT, Style_GetHotspotStyleID(), 0))
    return;

  // get left most position of style
  DocPos pos = position;
  int iNewStyle = iStyle;
  while ((iNewStyle == iStyle) && (--pos > 0)) {
    iNewStyle = (int)SendMessage(g_hwndEdit, SCI_GETSTYLEAT, pos, 0);
  }
  DocPos firstPos = (pos != 0) ? (pos + 1) : 0;

  // get right most position of style
  pos = position;
  iNewStyle = iStyle;
  DocPos posTextLength = SciCall_GetTextLength();
  while ((iNewStyle == iStyle) && (++pos < posTextLength)) {
    iNewStyle = (int)SendMessage(g_hwndEdit, SCI_GETSTYLEAT, pos, 0);
  }
  DocPos lastPos = pos;
  DocPos length = (lastPos - firstPos);

  if ((length > 0) && (length < XHUGE_BUFFER))
  {
    char chURL[XHUGE_BUFFER] = { '\0' };

    StringCchCopyNA(chURL, XHUGE_BUFFER, SciCall_GetRangePointer(firstPos, length), length);
    StrTrimA(chURL, " \t\n\r");
    
    if (!StringCchLenA(chURL, COUNTOF(chURL))) { return; }

    WCHAR wchURL[HUGE_BUFFER] = { L'\0' };
    MultiByteToWideCharStrg(Encoding_SciCP, chURL, wchURL);

    const WCHAR* chkPreFix = L"file://";
    const int len = lstrlen(chkPreFix);

    if (!bForceBrowser && (StrStrIW(wchURL, chkPreFix) == wchURL))
    {
      WCHAR* szFileName = &(wchURL[len]);
      StrTrimW(szFileName, L"/");

      PathCanonicalizeEx(szFileName, COUNTOF(wchURL) - len);

      if (PathIsDirectory(szFileName))
      {
        WCHAR tchFile[MAX_PATH + 1] = { L'\0' };

        if (OpenFileDlg(g_hwndMain, tchFile, COUNTOF(tchFile), szFileName))
          FileLoad(false, false, false, bSkipUnicodeDetection, bSkipANSICodePageDetection, tchFile);
      }
      else
        FileLoad(false, false, false, bSkipUnicodeDetection, bSkipANSICodePageDetection, szFileName);

    }
    else { // open in web browser

      WCHAR wchDirectory[MAX_PATH+1] = { L'\0' };
      if (StringCchLenW(g_wchCurFile, COUNTOF(g_wchCurFile))) {
        StringCchCopy(wchDirectory, COUNTOF(wchDirectory), g_wchCurFile);
        PathRemoveFileSpec(wchDirectory);
      }

      SHELLEXECUTEINFO sei;
      ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
      sei.cbSize = sizeof(SHELLEXECUTEINFO);
      sei.fMask = SEE_MASK_NOZONECHECKS;
      sei.hwnd = NULL;
      sei.lpVerb = NULL;
      sei.lpFile = wchURL;
      sei.lpParameters = NULL;
      sei.lpDirectory = wchDirectory;
      sei.nShow = SW_SHOWNORMAL;
      ShellExecuteEx(&sei);

    }

  }
}



//=============================================================================
//
//  MsgNotify() - Handles WM_NOTIFY
//
//
LRESULT MsgNotify(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
  LPNMHDR pnmh = (LPNMHDR)lParam;
  struct SCNotification* scn = (struct SCNotification*)lParam;

  if (!CheckNotifyChangeEvent()) 
  {
    // --- check only mandatory events (must be fast !!!) ---
    if (pnmh->idFrom == IDC_EDIT) {
      if (pnmh->code == SCN_MODIFIED) {
        // check for ADDUNDOACTION step
        if (scn->modificationType & SC_MOD_CONTAINER)
        {
          if (scn->modificationType & SC_PERFORMED_UNDO) {
            RestoreAction(scn->token, UNDO);
          }
          else if (scn->modificationType & SC_PERFORMED_REDO) {
            RestoreAction(scn->token, REDO);
          }
        }
        _SetDocumentModified(true);
      }
      else if (pnmh->code == SCN_SAVEPOINTREACHED) {
        _SetDocumentModified(false);
      }
      else if (pnmh->code == SCN_SAVEPOINTLEFT) {
        _SetDocumentModified(true);
      }
      else if (pnmh->code == SCN_MODIFYATTEMPTRO) {
        if (EditToggleView(g_hwndEdit, false)) {
          EditToggleView(g_hwndEdit, true);
        }
      }
    }
    return true; // swallowed
  }

  // --- check ALL events ---

  switch(pnmh->idFrom)
  {
    case IDC_EDIT:

      switch (pnmh->code)
      {
        case SCN_HOTSPOTCLICK:
        {
          if (scn->modifiers & SCMOD_CTRL) {
            // open in browser
            OpenHotSpotURL((int)scn->position, true);
          }
          if (scn->modifiers & SCMOD_ALT) {
            // open in application, if applicable (file://)
            OpenHotSpotURL((int)scn->position, false);
          }
        }
        break;


        //case SCN_STYLENEEDED:  // this event needs SCI_SETLEXER(SCLEX_CONTAINER)
        //  {
        //    int lineNumber = SciCall_LineFromPosition(SciCall_GetEndStyled());
        //    EditUpdateUrlHotspots(g_hwndEdit, SciCall_PositionFromLine(lineNumber), (int)scn->position, bHyperlinkHotspot);
        //    EditUpdateHiddenLineRange(hwnd, &g_efrData, 0, SciCall_GetLineCount());
        //  }
        //  break;

        case SCN_UPDATEUI:

          //if (scn->updated & SC_UPDATE_NP3_INTERNAL_NOTIFY) {
          //  // special case
          //}
          //else

          if (scn->updated & (SC_UPDATE_SELECTION | SC_UPDATE_CONTENT))
          {
            //~InvalidateSelections(); // fixed in SCI ?

            // Brace Match
            if (bMatchBraces) {
              EditMatchBrace(g_hwndEdit);
            }

            if (g_iMarkOccurrences > 0) {
              // clear marks only, if selection changed
              if (scn->updated & SC_UPDATE_SELECTION) {
                EditClearAllOccurrenceMarkers(g_hwndEdit, 0, -1);
                if (!SciCall_IsSelectionEmpty()) {
                  MarkAllOccurrences(iUpdateDelayMarkAllCoccurrences);
                }
              }
              else if (scn->updated & SC_UPDATE_CONTENT) {
                // ignoring SC_UPDATE_CONTENT cause Style and Marker are out of scope here
                // using WM_COMMAND -> SCEN_CHANGE  instead!
                //~MarkAllOccurrences(iUpdateDelayMarkAllCoccurrences);
              }
            }

            if (g_bHyperlinkHotspot) {
              UpdateVisibleUrlHotspot(iUpdateDelayHyperlinkStyling);
            }
            UpdateToolbar();
            UpdateStatusbar(false);
          }
          else if (scn->updated & SC_UPDATE_V_SCROLL)
          {
            if ((g_iMarkOccurrences > 0) && g_bMarkOccurrencesMatchVisible) {
              MarkAllOccurrences(iUpdateDelayMarkAllCoccurrences);
            }
            if (g_bHyperlinkHotspot) {
              UpdateVisibleUrlHotspot(iUpdateDelayHyperlinkStyling);
            }
          }
          break;


        case SCN_MODIFIED:
          {
            // check for ADDUNDOACTION step
            if (scn->modificationType & SC_MOD_CONTAINER) {
              if (scn->modificationType & SC_PERFORMED_UNDO) {
                RestoreAction(scn->token, UNDO);
              }
              else if (scn->modificationType & SC_PERFORMED_REDO) {
                RestoreAction(scn->token, REDO);
              }
            }
            else if (scn->modificationType & SC_MOD_CHANGESTYLE) {
              const DocPos iStartPos = (DocPos)scn->position;
              const DocPos iEndPos = (DocPos)(scn->position + scn->length);
              EditUpdateUrlHotspots(g_hwndEdit, iStartPos, iEndPos, g_bHyperlinkHotspot);
            }

            if (g_iMarkOccurrences > 0) {
              EditClearAllOccurrenceMarkers(g_hwndEdit, 0, -1);
              MarkAllOccurrences(iUpdateDelayMarkAllCoccurrences);
            }

            if (scn->linesAdded != 0) {
              UpdateLineNumberWidth();
            }

            _SetDocumentModified(true);

            UpdateToolbar();
            UpdateStatusbar(false);
          }
          break;


        case SCN_CHARADDED:
          {
            // Auto indent
            if (bAutoIndent && (scn->ch == '\x0D' || scn->ch == '\x0A'))
            {
              // in CRLF mode handle LF only...
              if ((SC_EOL_CRLF == g_iEOLMode && scn->ch != '\x0A') || SC_EOL_CRLF != g_iEOLMode)
              {
                const DocPos iCurPos = SciCall_GetCurrentPos();
                const DocLn iCurLine = SciCall_LineFromPosition(iCurPos);

                // Move bookmark along with line if inserting lines (pressing return within indent area of line) because Scintilla does not do this for us
                if (iCurLine > 0)
                {
                  //const DocPos iPrevLineLength = Sci_GetNetLineLength(iCurLine - 1);
                  if (SciCall_GetLineEndPosition(iCurLine - 1) == SciCall_GetLineIndentPosition(iCurLine - 1))
                  {
                    int bitmask = SciCall_MarkerGet(iCurLine - 1);
                    if (bitmask & (1 << MARKER_NP3_BOOKMARK))
                    {
                      SciCall_MarkerDelete(iCurLine - 1, MARKER_NP3_BOOKMARK);
                      SciCall_MarkerAdd(iCurLine, MARKER_NP3_BOOKMARK);
                    }
                  }
                }
                
                if (iCurLine > 0/* && iLineLength <= 2*/)
                {
                  const DocPos iPrevLineLength = SciCall_LineLength(iCurLine - 1);
                  char* pLineBuf = NULL;
                  bool bAllocLnBuf = false;
                  if (iPrevLineLength < TEMPLINE_BUFFER) {
                    pLineBuf = g_pTempLineBufferMain;
                  }
                  else {
                    bAllocLnBuf = true;
                    pLineBuf = AllocMem(iPrevLineLength + 1, HEAP_ZERO_MEMORY);
                  }
                  if (pLineBuf)
                  {
                    SciCall_GetLine(iCurLine - 1, pLineBuf);
                    *(pLineBuf + iPrevLineLength) = '\0';
                    for (char* pPos = pLineBuf; *pPos; pPos++) {
                      if (*pPos != ' ' && *pPos != '\t')
                        *pPos = '\0';
                    }
                    if (*pLineBuf) {
                      int token = BeginUndoAction();
                      SciCall_AddText(lstrlenA(pLineBuf), pLineBuf);
                      EndUndoAction(token);
                    }
                    if (bAllocLnBuf) { FreeMem(pLineBuf); }
                  }
                }
              }
            }
            // Auto close tags
            else if (bAutoCloseTags && scn->ch == '>')
            {
              //int lexerID = (int)SendMessage(g_hwndEdit,SCI_GETLEXER,0,0);
              //if (lexerID == SCLEX_HTML || lexerID == SCLEX_XML)
              {
                const DocPos iCurPos = SciCall_GetCurrentPos();
                const DocPos iHelper = iCurPos - (DocPos)(COUNTOF(g_pTempLineBufferMain) - 1);
                const DocPos iStartPos = max(0, iHelper);
                const DocPos iSize = iCurPos - iStartPos;

                if (iSize >= 3) 
                {
                  const char* pBegin = SciCall_GetRangePointer(iStartPos, iSize);

                  if (pBegin[iSize - 2] != '/') {

                    const char* pCur = &pBegin[iSize - 2];

                    while (pCur > pBegin && *pCur != '<' && *pCur != '>')
                      --pCur;

                    int  cchIns = 2;
                    StringCchCopyA(g_pTempLineBufferMain, FNDRPL_BUFFER, "</");
                    if (*pCur == '<') {
                      pCur++;
                      while (StrChrA(":_-.", *pCur) || IsCharAlphaNumericA(*pCur)) {
                        g_pTempLineBufferMain[cchIns++] = *pCur;
                        pCur++;
                      }
                    }
                    g_pTempLineBufferMain[cchIns++] = '>';
                    g_pTempLineBufferMain[cchIns] = '\0';

                    if (cchIns > 3 &&
                      StringCchCompareINA(g_pTempLineBufferMain, COUNTOF(g_pTempLineBufferMain), "</base>", -1) &&
                      StringCchCompareINA(g_pTempLineBufferMain, COUNTOF(g_pTempLineBufferMain), "</bgsound>", -1) &&
                      StringCchCompareINA(g_pTempLineBufferMain, COUNTOF(g_pTempLineBufferMain), "</br>", -1) &&
                      StringCchCompareINA(g_pTempLineBufferMain, COUNTOF(g_pTempLineBufferMain), "</embed>", -1) &&
                      StringCchCompareINA(g_pTempLineBufferMain, COUNTOF(g_pTempLineBufferMain), "</hr>", -1) &&
                      StringCchCompareINA(g_pTempLineBufferMain, COUNTOF(g_pTempLineBufferMain), "</img>", -1) &&
                      StringCchCompareINA(g_pTempLineBufferMain, COUNTOF(g_pTempLineBufferMain), "</input>", -1) &&
                      StringCchCompareINA(g_pTempLineBufferMain, COUNTOF(g_pTempLineBufferMain), "</link>", -1) &&
                      StringCchCompareINA(g_pTempLineBufferMain, COUNTOF(g_pTempLineBufferMain), "</meta>", -1))
                    {
                      int token = BeginUndoAction();
                      SciCall_ReplaceSel(g_pTempLineBufferMain);
                      SciCall_SetSel(iCurPos, iCurPos);
                      EndUndoAction(token);
                    }
                  }
                }
              }
            }
            else if (bAutoCompleteWords && !SendMessage(g_hwndEdit, SCI_AUTOCACTIVE, 0, 0)) {
              EditCompleteWord(g_hwndEdit, false);
            }
          }
          break;


        case SCN_NEEDSHOWN:
          {
            DocLn iFirstLine = SciCall_LineFromPosition((DocPos)scn->position);
            DocLn iLastLine = SciCall_LineFromPosition((DocPos)(scn->position + scn->length - 1));
            for (DocLn i = iFirstLine; i <= iLastLine; ++i) { SciCall_EnsureVisible(i); }
          }
          break;


        case SCN_MARGINCLICK:
          if (scn->margin == MARGIN_SCI_FOLDING) {
            EditFoldClick(SciCall_LineFromPosition((DocPos)scn->position), scn->modifiers);
          }
          break;


        // ~~~ Not used in Windows ~~~
        // see: CMD_ALTUP / CMD_ALTDOWN
        //case SCN_KEY:
        //  // Also see the corresponding patch in scintilla\src\Editor.cxx
        //  FoldAltArrow(scn->ch, scn->modifiers);
        //  break;


        case SCN_SAVEPOINTREACHED:
          SciCall_SetScrollWidth(1);
          _SetDocumentModified(false);
          break;


        case SCN_SAVEPOINTLEFT:
          _SetDocumentModified(true);
          break;


        case SCN_ZOOM:
          UpdateLineNumberWidth();
          break;


        default:
          return false;
      }
      return true;


    case IDC_TOOLBAR:

      switch(pnmh->code)
      {
        case TBN_ENDADJUST:
          UpdateToolbar();
          break;

        case TBN_QUERYDELETE:
        case TBN_QUERYINSERT:
          break;

        case TBN_GETBUTTONINFO:
          {
            if (((LPTBNOTIFY)lParam)->iItem < COUNTOF(tbbMainWnd))
            {
              WCHAR tch[MIDSZ_BUFFER] = { L'\0' };
              GetString(tbbMainWnd[((LPTBNOTIFY)lParam)->iItem].idCommand,tch,COUNTOF(tch));
              StringCchCopyN(((LPTBNOTIFY)lParam)->pszText,((LPTBNOTIFY)lParam)->cchText,tch,((LPTBNOTIFY)lParam)->cchText);
              CopyMemory(&((LPTBNOTIFY)lParam)->tbButton,&tbbMainWnd[((LPTBNOTIFY)lParam)->iItem],sizeof(TBBUTTON));
              return true;
            }
          }
          return false;

        case TBN_RESET:
          {
            int i; int c = (int)SendMessage(g_hwndToolbar,TB_BUTTONCOUNT,0,0);
            for (i = 0; i < c; i++) {
              SendMessage(g_hwndToolbar, TB_DELETEBUTTON, 0, 0);
            }
            SendMessage(g_hwndToolbar,TB_ADDBUTTONS,NUMINITIALTOOLS,(LPARAM)tbbMainWnd);
            return(0);
          }
          break;

        default:
          return false;
      }
      return true;


    case IDC_STATUSBAR:

      switch(pnmh->code)
      {

        case NM_CLICK:
          {
            LPNMMOUSE pnmm = (LPNMMOUSE)lParam;

            switch (pnmm->dwItemSpec)
            {
              case STATUS_EOLMODE:
                SendMessage(g_hwndEdit,SCI_CONVERTEOLS, SciCall_GetEOLMode(),0);
                EditFixPositions(g_hwndEdit);
                return true;

              default:
                return false;
            }
          }

        case NM_DBLCLK:
          {
            LPNMMOUSE pnmm = (LPNMMOUSE)lParam;

            switch (g_vSBSOrder[pnmm->dwItemSpec])
            {
              case STATUS_DOCLINE:
              case STATUS_DOCCOLUMN:
                PostMessage(hwnd, WM_COMMAND, MAKELONG(IDM_EDIT_GOTOLINE,1),0);
                return true;

              case STATUS_CODEPAGE:
                PostMessage(hwnd,WM_COMMAND,MAKELONG(IDM_ENCODING_SELECT,1),0);
                return true;

              case STATUS_EOLMODE:
                {
                  int i;
                  if (g_iEOLMode == SC_EOL_CRLF)
                    i = IDM_LINEENDINGS_CRLF;
                  else if (g_iEOLMode == SC_EOL_LF)
                    i = IDM_LINEENDINGS_LF;
                  else
                    i = IDM_LINEENDINGS_CR;
                  i++;
                  if (i > IDM_LINEENDINGS_CR) { i = IDM_LINEENDINGS_CRLF; }
                  PostMessage(hwnd, WM_COMMAND, MAKELONG(i, 1), 0);
                }
                return true;

              case STATUS_OVRMODE:
                PostMessage(hwnd, WM_COMMAND, MAKELONG(CMD_VK_INSERT, 1), 0);
                return true;

              case STATUS_2ND_DEF:
                PostMessage(hwnd, WM_COMMAND, MAKELONG(IDM_VIEW_USE2NDDEFAULT, 1), 0);
                return true;

              case STATUS_LEXER:
                PostMessage(hwnd, WM_COMMAND, MAKELONG(IDM_VIEW_SCHEME, 1), 0);
                return true;

              default:
                return false;
            }
          }
          break;

      }
      return true;


    default:

      switch(pnmh->code)
      {
        case TTN_NEEDTEXT:
          {
            if (!(((LPTOOLTIPTEXT)lParam)->uFlags & TTF_IDISHWND))
            {
              WCHAR tch[MIDSZ_BUFFER] = { L'\0' };
              GetString((UINT)pnmh->idFrom,tch,COUNTOF(tch));
              StringCchCopyN(((LPTOOLTIPTEXT)lParam)->szText,COUNTOF(((LPTOOLTIPTEXT)lParam)->szText),tch,COUNTOF(((LPTOOLTIPTEXT)lParam)->szText));
            }
          }
          break;

      }
      break;

  }

  UNUSED(wParam);

  return false;
}



//=============================================================================
//
//  Set/Get FindPattern()
// 
static WCHAR sCurrentFindPattern[FNDRPL_BUFFER] = { L'\0' };

bool IsFindPatternEmpty()
{
  return (StringCchLenW(sCurrentFindPattern, COUNTOF(sCurrentFindPattern)) == 0);
}

//=============================================================================
//
//  SetFindPattern()
// 
void SetFindPattern(LPCWSTR wchFindPattern)
{
  StringCchCopyW(sCurrentFindPattern, COUNTOF(sCurrentFindPattern), (wchFindPattern ? wchFindPattern : L""));
}

//=============================================================================
//
//  SetFindPatternMB()
// 
void SetFindPatternMB(LPCSTR chFindPattern)
{
  MultiByteToWideChar(Encoding_SciCP, 0, chFindPattern, -1, sCurrentFindPattern, COUNTOF(sCurrentFindPattern));
}

//=============================================================================
//
//  GetFindPattern()
// 
void GetFindPattern(LPWSTR wchFindPattern, size_t bufferSize)
{
  StringCchCopyW(wchFindPattern, bufferSize, sCurrentFindPattern);
}

//=============================================================================
//
//  GetFindPatternMB()
// 
void GetFindPatternMB(LPSTR chFindPattern, size_t bufferSize)
{
  WideCharToMultiByte(Encoding_SciCP, 0, sCurrentFindPattern, -1, chFindPattern, (int)bufferSize, NULL, NULL);
}


//=============================================================================
//
//  LoadSettings()
//
//
void LoadSettings()
{
  WCHAR *pIniSection = LocalAlloc(LPTR, sizeof(WCHAR) * INISECTIONBUFCNT * HUGE_BUFFER);
  int   cchIniSection = (int)LocalSize(pIniSection)/sizeof(WCHAR);

  // --------------------------------------------------------------------------
  LoadIniSection(L"Settings", pIniSection, cchIniSection);
  // --------------------------------------------------------------------------

  bEnableSaveSettings = true;
  bSaveSettings = IniSectionGetBool(pIniSection,L"SaveSettings",true);
  bSaveRecentFiles = IniSectionGetBool(pIniSection,L"SaveRecentFiles",false);
  bPreserveCaretPos = IniSectionGetBool(pIniSection, L"PreserveCaretPos",false);
  bSaveFindReplace = IniSectionGetBool(pIniSection,L"SaveFindReplace",false);

  g_efrData.bFindClose = IniSectionGetBool(pIniSection,L"CloseFind", false);
  g_efrData.bReplaceClose = IniSectionGetBool(pIniSection,L"CloseReplace", false);
  g_efrData.bNoFindWrap = IniSectionGetBool(pIniSection,L"NoFindWrap", false);
  g_efrData.bTransformBS = IniSectionGetBool(pIniSection,L"FindTransformBS", false);
  g_efrData.bWildcardSearch = IniSectionGetBool(pIniSection,L"WildcardSearch",false);
  g_efrData.bMarkOccurences = IniSectionGetBool(pIniSection, L"FindMarkAllOccurrences", false);
  g_efrData.bHideNonMatchedLines = IniSectionGetBool(pIniSection, L"HideNonMatchedLines", false);
  g_efrData.bDotMatchAll = IniSectionGetBool(pIniSection, L"RegexDotMatchesAll", false);
  g_efrData.fuFlags = IniSectionGetUInt(pIniSection, L"efrData_fuFlags", 0);

  if (!IniSectionGetString(pIniSection, L"OpenWithDir", L"", g_tchOpenWithDir, COUNTOF(g_tchOpenWithDir))) {
    //SHGetSpecialFolderPath(NULL, g_tchOpenWithDir, CSIDL_DESKTOPDIRECTORY, true);
    GetKnownFolderPath(&FOLDERID_Desktop, g_tchOpenWithDir, COUNTOF(g_tchOpenWithDir));
  }
  else {
    PathAbsoluteFromApp(g_tchOpenWithDir, NULL, COUNTOF(g_tchOpenWithDir), true);
  }
  if (!IniSectionGetString(pIniSection, L"Favorites", L"", g_tchFavoritesDir, COUNTOF(g_tchFavoritesDir))) {
    //SHGetFolderPath(NULL,CSIDL_PERSONAL,NULL,SHGFP_TYPE_CURRENT,g_tchFavoritesDir);
    GetKnownFolderPath(&FOLDERID_Favorites, g_tchFavoritesDir, COUNTOF(g_tchFavoritesDir));
  }
  else {
    PathAbsoluteFromApp(g_tchFavoritesDir, NULL, COUNTOF(g_tchFavoritesDir), true);
  }

  iPathNameFormat = IniSectionGetInt(pIniSection,L"PathNameFormat",0);
  iPathNameFormat = max(min(iPathNameFormat,2),0);

  g_bWordWrap = IniSectionGetBool(pIniSection,L"WordWrap",false);
  bWordWrapG = g_bWordWrap;

  iWordWrapMode = IniSectionGetInt(pIniSection,L"WordWrapMode",0);
  iWordWrapMode = max(min(iWordWrapMode,1),0);

  iWordWrapIndent = IniSectionGetInt(pIniSection,L"WordWrapIndent",0);
  iWordWrapIndent = max(min(iWordWrapIndent,6),0);

  iWordWrapSymbols = IniSectionGetInt(pIniSection,L"WordWrapSymbols",22);
  iWordWrapSymbols = max(min(iWordWrapSymbols%10,2),0)+max(min((iWordWrapSymbols%100-iWordWrapSymbols%10)/10,2),0)*10;

  bShowWordWrapSymbols = IniSectionGetBool(pIniSection,L"ShowWordWrapSymbols",0);

  bMatchBraces = IniSectionGetBool(pIniSection,L"MatchBraces",true);

  bAutoCloseTags = IniSectionGetBool(pIniSection,L"AutoCloseTags",false);

  bHiliteCurrentLine = IniSectionGetBool(pIniSection,L"HighlightCurrentLine",false);

  g_bHyperlinkHotspot = IniSectionGetBool(pIniSection, L"HyperlinkHotspot", false);

  bScrollPastEOF = IniSectionGetBool(pIniSection, L"ScrollPastEOF", false);

  bAutoIndent = IniSectionGetBool(pIniSection,L"AutoIndent",true);

  bAutoCompleteWords = IniSectionGetBool(pIniSection,L"AutoCompleteWords",false);

  bAccelWordNavigation = IniSectionGetBool(pIniSection, L"AccelWordNavigation", false);

  bShowIndentGuides = IniSectionGetBool(pIniSection,L"ShowIndentGuides",false);

  g_bTabsAsSpaces = IniSectionGetBool(pIniSection,L"TabsAsSpaces",true);
  bTabsAsSpacesG = g_bTabsAsSpaces;

  g_bTabIndents = IniSectionGetBool(pIniSection,L"TabIndents",true);
  bTabIndentsG = g_bTabIndents;

  bBackspaceUnindents = IniSectionGetBool(pIniSection,L"BackspaceUnindents",false);

  g_iTabWidth = IniSectionGetInt(pIniSection,L"TabWidth",2);
  g_iTabWidth = max(min(g_iTabWidth,256),1);
  iTabWidthG = g_iTabWidth;

  g_iIndentWidth = IniSectionGetInt(pIniSection,L"IndentWidth",0);
  g_iIndentWidth = max(min(g_iIndentWidth,256),0);
  iIndentWidthG = g_iIndentWidth;

  bMarkLongLines = IniSectionGetBool(pIniSection,L"MarkLongLines",false);

  g_iLongLinesLimit = IniSectionGetInt(pIniSection,L"LongLinesLimit",72);
  g_iLongLinesLimit = max(min(g_iLongLinesLimit,4096),0);
  iLongLinesLimitG = g_iLongLinesLimit;

  iLongLineMode = IniSectionGetInt(pIniSection,L"LongLineMode",EDGE_LINE);
  iLongLineMode = max(min(iLongLineMode,EDGE_BACKGROUND),EDGE_LINE);

  g_bShowSelectionMargin = IniSectionGetBool(pIniSection,L"ShowSelectionMargin",false);

  bShowLineNumbers = IniSectionGetBool(pIniSection,L"ShowLineNumbers", true);

  g_bShowCodeFolding = IniSectionGetBool(pIniSection,L"ShowCodeFolding", true);

  g_iMarkOccurrences = IniSectionGetInt(pIniSection,L"MarkOccurrences",1);
  g_iMarkOccurrences = max(min(g_iMarkOccurrences, 3), 0);
  g_bMarkOccurrencesMatchVisible = IniSectionGetBool(pIniSection, L"MarkOccurrencesMatchVisible", false);
  bMarkOccurrencesMatchCase = IniSectionGetBool(pIniSection,L"MarkOccurrencesMatchCase",false);
  bMarkOccurrencesMatchWords = IniSectionGetBool(pIniSection,L"MarkOccurrencesMatchWholeWords",true);
  bMarkOccurrencesCurrentWord = IniSectionGetBool(pIniSection, L"MarkOccurrencesCurrentWord", !bMarkOccurrencesMatchWords);
  bMarkOccurrencesCurrentWord = bMarkOccurrencesCurrentWord && !bMarkOccurrencesMatchWords;

  bViewWhiteSpace = IniSectionGetBool(pIniSection,L"ViewWhiteSpace", false);

  bViewEOLs = IniSectionGetBool(pIniSection,L"ViewEOLs", false);

  g_iDefaultNewFileEncoding = IniSectionGetInt(pIniSection,L"DefaultEncoding", CPI_NONE);
  // if DefaultEncoding is not defined set to system's current code-page 
  g_iDefaultNewFileEncoding = (g_iDefaultNewFileEncoding == CPI_NONE) ?
    Encoding_MapIniSetting(true,(int)GetACP()) : Encoding_MapIniSetting(true,g_iDefaultNewFileEncoding);

  bUseDefaultForFileEncoding = IniSectionGetBool(pIniSection, L"UseDefaultForFileEncoding", false);

  bSkipUnicodeDetection = IniSectionGetBool(pIniSection, L"SkipUnicodeDetection", false);

  bSkipANSICodePageDetection = IniSectionGetBool(pIniSection, L"SkipANSICodePageDetection", true);

  bLoadASCIIasUTF8 = IniSectionGetBool(pIniSection, L"LoadASCIIasUTF8", false);

  bLoadNFOasOEM = IniSectionGetBool(pIniSection,L"LoadNFOasOEM",true);

  bNoEncodingTags = IniSectionGetBool(pIniSection,L"NoEncodingTags", false);

  g_iDefaultEOLMode = IniSectionGetInt(pIniSection,L"DefaultEOLMode",0);
  g_iDefaultEOLMode = max(min(g_iDefaultEOLMode,2),0);

  bFixLineEndings = IniSectionGetBool(pIniSection,L"FixLineEndings",true);

  bAutoStripBlanks = IniSectionGetBool(pIniSection,L"FixTrailingBlanks",false);

  iPrintHeader = IniSectionGetInt(pIniSection,L"PrintHeader",1);
  iPrintHeader = max(min(iPrintHeader,3),0);

  iPrintFooter = IniSectionGetInt(pIniSection,L"PrintFooter",0);
  iPrintFooter = max(min(iPrintFooter,1),0);

  iPrintColor = IniSectionGetInt(pIniSection,L"PrintColorMode",3);
  iPrintColor = max(min(iPrintColor,4),0);

  iPrintZoom = IniSectionGetInt(pIniSection,L"PrintZoom",10)-10;
  iPrintZoom = max(min(iPrintZoom,20),-10);

  pagesetupMargin.left = IniSectionGetInt(pIniSection,L"PrintMarginLeft",-1);
  pagesetupMargin.left = max(pagesetupMargin.left,-1);

  pagesetupMargin.top = IniSectionGetInt(pIniSection,L"PrintMarginTop",-1);
  pagesetupMargin.top = max(pagesetupMargin.top,-1);

  pagesetupMargin.right = IniSectionGetInt(pIniSection,L"PrintMarginRight",-1);
  pagesetupMargin.right = max(pagesetupMargin.right,-1);

  pagesetupMargin.bottom = IniSectionGetInt(pIniSection,L"PrintMarginBottom",-1);
  pagesetupMargin.bottom = max(pagesetupMargin.bottom,-1);

  bSaveBeforeRunningTools = IniSectionGetBool(pIniSection,L"SaveBeforeRunningTools",false);

  g_iFileWatchingMode = IniSectionGetInt(pIniSection,L"FileWatchingMode",0);
  g_iFileWatchingMode = max(min(g_iFileWatchingMode,2),0);

  g_bResetFileWatching = IniSectionGetBool(pIniSection,L"ResetFileWatching",true);

  iEscFunction = IniSectionGetInt(pIniSection,L"EscFunction",0);
  iEscFunction = max(min(iEscFunction,2),0);

  bAlwaysOnTop = IniSectionGetBool(pIniSection,L"AlwaysOnTop",false);

  bMinimizeToTray = IniSectionGetBool(pIniSection,L"MinimizeToTray",false);

  bTransparentMode = IniSectionGetBool(pIniSection,L"TransparentMode",false);

  // Check if SetLayeredWindowAttributes() is available
  bTransparentModeAvailable = (GetProcAddress(GetModuleHandle(L"User32"),"SetLayeredWindowAttributes") != NULL);
  bTransparentModeAvailable = (bTransparentModeAvailable) ? true : false;

  // see TBBUTTON  tbbMainWnd[] for initial/reset set of buttons
  IniSectionGetString(pIniSection,L"ToolbarButtons", L"", g_tchToolbarButtons, COUNTOF(g_tchToolbarButtons));

  bShowToolbar = IniSectionGetBool(pIniSection,L"ShowToolbar",true);

  bShowStatusbar = IniSectionGetBool(pIniSection,L"ShowStatusbar",true);

  cxEncodingDlg = IniSectionGetInt(pIniSection,L"EncodingDlgSizeX",256);
  cxEncodingDlg = max(cxEncodingDlg,0);

  cyEncodingDlg = IniSectionGetInt(pIniSection,L"EncodingDlgSizeY",262);
  cyEncodingDlg = max(cyEncodingDlg,0);

  cxRecodeDlg = IniSectionGetInt(pIniSection,L"RecodeDlgSizeX",256);
  cxRecodeDlg = max(cxRecodeDlg,0);

  cyRecodeDlg = IniSectionGetInt(pIniSection,L"RecodeDlgSizeY",262);
  cyRecodeDlg = max(cyRecodeDlg,0);

  cxFileMRUDlg = IniSectionGetInt(pIniSection,L"FileMRUDlgSizeX",412);
  cxFileMRUDlg = max(cxFileMRUDlg,0);

  cyFileMRUDlg = IniSectionGetInt(pIniSection,L"FileMRUDlgSizeY",376);
  cyFileMRUDlg = max(cyFileMRUDlg,0);

  cxOpenWithDlg = IniSectionGetInt(pIniSection,L"OpenWithDlgSizeX",384);
  cxOpenWithDlg = max(cxOpenWithDlg,0);

  cyOpenWithDlg = IniSectionGetInt(pIniSection,L"OpenWithDlgSizeY",386);
  cyOpenWithDlg = max(cyOpenWithDlg,0);

  cxFavoritesDlg = IniSectionGetInt(pIniSection,L"FavoritesDlgSizeX",334);
  cxFavoritesDlg = max(cxFavoritesDlg,0);

  cyFavoritesDlg = IniSectionGetInt(pIniSection,L"FavoritesDlgSizeY",316);
  cyFavoritesDlg = max(cyFavoritesDlg,0);

  xFindReplaceDlg = IniSectionGetInt(pIniSection,L"FindReplaceDlgPosX",0);
  yFindReplaceDlg = IniSectionGetInt(pIniSection,L"FindReplaceDlgPosY",0);

  xCustomSchemesDlg = IniSectionGetInt(pIniSection, L"CustomSchemesDlgPosX", 0);
  yCustomSchemesDlg = IniSectionGetInt(pIniSection, L"CustomSchemesDlgPosY", 0);

  // --------------------------------------------------------------------------
  LoadIniSection(L"Settings2",pIniSection,cchIniSection);
  // --------------------------------------------------------------------------

  bStickyWinPos = IniSectionGetInt(pIniSection,L"StickyWindowPosition",0);
  if (bStickyWinPos) bStickyWinPos = 1;

  IniSectionGetString(pIniSection,L"DefaultExtension",L"txt", g_tchDefaultExtension,COUNTOF(g_tchDefaultExtension));
  StrTrim(g_tchDefaultExtension,L" \t.\"");

  IniSectionGetString(pIniSection,L"DefaultDirectory",L"", g_tchDefaultDir,COUNTOF(g_tchDefaultDir));

  ZeroMemory(g_tchFileDlgFilters,sizeof(WCHAR)*COUNTOF(g_tchFileDlgFilters));
  IniSectionGetString(pIniSection,L"FileDlgFilters",L"",
    g_tchFileDlgFilters,COUNTOF(g_tchFileDlgFilters)-2);

  dwFileCheckInverval = IniSectionGetInt(pIniSection,L"FileCheckInverval",2000);
  dwAutoReloadTimeout = IniSectionGetInt(pIniSection,L"AutoReloadTimeout",2000);

  iSciDirectWriteTech = IniSectionGetInt(pIniSection,L"SciDirectWriteTech", DirectWriteTechnology[0]);
  iSciDirectWriteTech = max(min(iSciDirectWriteTech,3),-1);

  iSciFontQuality = IniSectionGetInt(pIniSection,L"SciFontQuality", FontQuality[3]);
  iSciFontQuality = max(min(iSciFontQuality, 3), 0);

  g_iMarkOccurrencesMaxCount = IniSectionGetInt(pIniSection,L"MarkOccurrencesMaxCount",2000);
  g_iMarkOccurrencesMaxCount = (g_iMarkOccurrencesMaxCount <= 0) ? INT_MAX : g_iMarkOccurrencesMaxCount;

  iUpdateDelayHyperlinkStyling = IniSectionGetInt(pIniSection, L"UpdateDelayHyperlinkStyling", 100);
  iUpdateDelayHyperlinkStyling = max(min(iUpdateDelayHyperlinkStyling, 10000), USER_TIMER_MINIMUM);

  iUpdateDelayMarkAllCoccurrences = IniSectionGetInt(pIniSection, L"UpdateDelayMarkAllCoccurrences", 50);
  iUpdateDelayMarkAllCoccurrences = max(min(iUpdateDelayMarkAllCoccurrences, 10000), USER_TIMER_MINIMUM);

  bDenyVirtualSpaceAccess = IniSectionGetBool(pIniSection, L"DenyVirtualSpaceAccess", false);
  bUseOldStyleBraceMatching = IniSectionGetBool(pIniSection, L"UseOldStyleBraceMatching", false);
  
  iCurrentLineHorizontalSlop = IniSectionGetInt(pIniSection, L"CurrentLineHorizontalSlop", 40);
  iCurrentLineHorizontalSlop = max(min(iCurrentLineHorizontalSlop, 2000), 0);

  iCurrentLineVerticalSlop = IniSectionGetInt(pIniSection, L"CurrentLineVerticalSlop", 5);
  iCurrentLineVerticalSlop = max(min(iCurrentLineVerticalSlop, 200), 0);

  IniSectionGetString(pIniSection, L"UpdateChecker.exe", L"", g_tchUpdateCheckerExe, COUNTOF(g_tchUpdateCheckerExe));

  // --------------------------------------------------------------------------
  LoadIniSection(L"Statusbar Settings", pIniSection, cchIniSection);
  // --------------------------------------------------------------------------

  IniSectionGetString(pIniSection, L"SectionPrefixes", STATUSBAR_EXTION_PREFIXES, g_tchStatusbarPrefixes, COUNTOF(g_tchStatusbarPrefixes));
  ReadStrgsFromCSV(g_tchStatusbarPrefixes, g_mxStatusBarPrefix, STATUS_SECTOR_COUNT, MICRO_BUFFER, L"");

  IniSectionGetString(pIniSection, L"VisibleSections", STATUSBAR_DEFAULT_IDS, g_tchStatusbarSections, COUNTOF(g_tchStatusbarSections));
  ReadVectorFromString(g_tchStatusbarSections, g_iStatusbarSections, STATUS_SECTOR_COUNT, 0, (STATUS_SECTOR_COUNT - 1), -1);

  for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
    g_iStatusbarVisible[i] = false;
  }
  int cnt = 0;
  for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
    g_vSBSOrder[i] = -1;
    int const id = g_iStatusbarSections[i];
    if (id >= 0) {
      g_vSBSOrder[cnt++] = id;
      g_iStatusbarVisible[id] = true;
    }
  }

  IniSectionGetString(pIniSection, L"SectionWidthSpecs", STATUSBAR_SECTION_WIDTH_SPECS, g_tchStatusbarWidthSpec, COUNTOF(g_tchStatusbarWidthSpec));
  ReadVectorFromString(g_tchStatusbarWidthSpec, g_iStatusbarWidthSpec, STATUS_SECTOR_COUNT, -4096, 4096, 0);
 

  // --------------------------------------------------------------------------
  LoadIniSection(L"Toolbar Images",pIniSection,cchIniSection);
  // --------------------------------------------------------------------------

  IniSectionGetString(pIniSection,L"BitmapDefault",L"",
    g_tchToolbarBitmap,COUNTOF(g_tchToolbarBitmap));
  IniSectionGetString(pIniSection,L"BitmapHot",L"",
    g_tchToolbarBitmapHot,COUNTOF(g_tchToolbarBitmap));
  IniSectionGetString(pIniSection,L"BitmapDisabled",L"",
    g_tchToolbarBitmapDisabled,COUNTOF(g_tchToolbarBitmap));

  int ResX = GetSystemMetrics(SM_CXSCREEN);
  int ResY = GetSystemMetrics(SM_CYSCREEN);

  // --------------------------------------------------------------------------
  LoadIniSection(L"Window", pIniSection, cchIniSection);
  // --------------------------------------------------------------------------

  WCHAR tchHighDpiToolBar[32] = { L'\0' };
  StringCchPrintf(tchHighDpiToolBar,COUNTOF(tchHighDpiToolBar),L"%ix%i HighDpiToolBar", ResX, ResY);
  iHighDpiToolBar = IniSectionGetInt(pIniSection, tchHighDpiToolBar, -1);
  iHighDpiToolBar = max(min(iHighDpiToolBar, 1), -1);
  if (iHighDpiToolBar < 0) { // undefined: determine high DPI (higher than Full-HD)
    if ((ResX > 1920) && (ResY > 1080))
      iHighDpiToolBar = 1;
    else
      iHighDpiToolBar = 0;
  }

  if (!flagPosParam /*|| bStickyWinPos*/) { // ignore window position if /p was specified

    WCHAR tchPosX[32], tchPosY[32], tchSizeX[32], tchSizeY[32], tchMaximized[32];

    StringCchPrintf(tchPosX,COUNTOF(tchPosX),L"%ix%i PosX",ResX,ResY);
    StringCchPrintf(tchPosY,COUNTOF(tchPosY),L"%ix%i PosY",ResX,ResY);
    StringCchPrintf(tchSizeX,COUNTOF(tchSizeX),L"%ix%i SizeX",ResX,ResY);
    StringCchPrintf(tchSizeY,COUNTOF(tchSizeY),L"%ix%i SizeY",ResX,ResY);
    StringCchPrintf(tchMaximized,COUNTOF(tchMaximized),L"%ix%i Maximized",ResX,ResY);

    g_WinInfo.x = IniSectionGetInt(pIniSection,tchPosX,CW_USEDEFAULT);
    g_WinInfo.y = IniSectionGetInt(pIniSection,tchPosY,CW_USEDEFAULT);
    g_WinInfo.cx = IniSectionGetInt(pIniSection,tchSizeX,CW_USEDEFAULT);
    g_WinInfo.cy = IniSectionGetInt(pIniSection,tchSizeY,CW_USEDEFAULT);
    g_WinInfo.max = IniSectionGetInt(pIniSection,tchMaximized,0);
    if (g_WinInfo.max) g_WinInfo.max = 1;
  }

  // ---  override by resolution specific settings  ---

  WCHAR tchSciDirectWriteTech[64];
  StringCchPrintf(tchSciDirectWriteTech,COUNTOF(tchSciDirectWriteTech),L"%ix%i SciDirectWriteTech",ResX,ResY);
  iSciDirectWriteTech = IniSectionGetInt(pIniSection,tchSciDirectWriteTech,iSciDirectWriteTech);
  iSciDirectWriteTech = max(min(iSciDirectWriteTech,3),-1);

  WCHAR tchSciFontQuality[64];
  StringCchPrintf(tchSciFontQuality,COUNTOF(tchSciFontQuality),L"%ix%i SciFontQuality",ResX,ResY);
  iSciFontQuality = IniSectionGetInt(pIniSection,tchSciFontQuality,iSciFontQuality);
  iSciFontQuality = max(min(iSciFontQuality, SC_EFF_QUALITY_LCD_OPTIMIZED), SC_TECHNOLOGY_DEFAULT);


  LocalFree(pIniSection);


  // define scintilla internal codepage
  const int iSciDefaultCodePage = SC_CP_UTF8; // default UTF8

  // remove internal support for Chinese, Japan, Korean DBCS  use UTF-8 instead
  /*
  if (g_iDefaultNewFileEncoding == CPI_ANSI_DEFAULT)
  {
    // check for Chinese, Japan, Korean DBCS code pages and switch accordingly
    int acp = (int)GetACP();
    if (acp == 932 || acp == 936 || acp == 949 || acp == 950) {
      iSciDefaultCodePage = acp;
    }
    g_iDefaultNewFileEncoding = Encoding_GetByCodePage(iSciDefaultCodePage);
  }
  */

  // set flag for encoding default
  Encoding_SetDefaultFlag(g_iDefaultNewFileEncoding);

  // define default charset
  g_iDefaultCharSet = (int)CharSetFromCodePage((UINT)iSciDefaultCodePage);

  // Scintilla Styles
  Style_Load();

}


//=============================================================================
//
//  SaveSettings()
//
//
void SaveSettings(bool bSaveSettingsNow) {
  WCHAR *pIniSection = NULL;

  WCHAR wchTmp[MAX_PATH] = { L'\0' };

  if (StringCchLenW(g_wchIniFile,COUNTOF(g_wchIniFile)) == 0)
    return;

  if (!bEnableSaveSettings)
    return; 

  CreateIniFile();

  if (!bSaveSettings && !bSaveSettingsNow) {
    IniSetInt(L"Settings", L"SaveSettings", bSaveSettings);
    return;
  }

  pIniSection = LocalAlloc(LPTR, sizeof(WCHAR) * INISECTIONBUFCNT * HUGE_BUFFER);
  //int cchIniSection = (int)LocalSize(pIniSection) / sizeof(WCHAR);

  IniSectionSetBool(pIniSection, L"SaveSettings", bSaveSettings);
  IniSectionSetBool(pIniSection, L"SaveRecentFiles", bSaveRecentFiles);
  IniSectionSetBool(pIniSection, L"PreserveCaretPos", bPreserveCaretPos);
  IniSectionSetBool(pIniSection, L"SaveFindReplace", bSaveFindReplace);
  IniSectionSetBool(pIniSection, L"CloseFind", g_efrData.bFindClose);
  IniSectionSetBool(pIniSection, L"CloseReplace", g_efrData.bReplaceClose);
  IniSectionSetBool(pIniSection, L"NoFindWrap", g_efrData.bNoFindWrap);
  IniSectionSetBool(pIniSection, L"FindTransformBS", g_efrData.bTransformBS);
  IniSectionSetBool(pIniSection, L"WildcardSearch", g_efrData.bWildcardSearch);
  IniSectionSetBool(pIniSection, L"FindMarkAllOccurrences", g_efrData.bMarkOccurences);
  IniSectionSetBool(pIniSection, L"HideNonMatchedLines", g_efrData.bHideNonMatchedLines);
  IniSectionSetBool(pIniSection, L"RegexDotMatchesAll", g_efrData.bDotMatchAll);
  IniSectionSetInt(pIniSection, L"efrData_fuFlags", g_efrData.fuFlags);
  PathRelativeToApp(g_tchOpenWithDir, wchTmp, COUNTOF(wchTmp), false, true, flagPortableMyDocs);
  IniSectionSetString(pIniSection, L"OpenWithDir", wchTmp);
  PathRelativeToApp(g_tchFavoritesDir, wchTmp, COUNTOF(wchTmp), false, true, flagPortableMyDocs);
  IniSectionSetString(pIniSection, L"Favorites", wchTmp);
  IniSectionSetInt(pIniSection, L"PathNameFormat", iPathNameFormat);
  IniSectionSetBool(pIniSection, L"WordWrap", bWordWrapG);
  IniSectionSetInt(pIniSection, L"WordWrapMode", iWordWrapMode);
  IniSectionSetInt(pIniSection, L"WordWrapIndent", iWordWrapIndent);
  IniSectionSetInt(pIniSection, L"WordWrapSymbols", iWordWrapSymbols);
  IniSectionSetBool(pIniSection, L"ShowWordWrapSymbols", bShowWordWrapSymbols);
  IniSectionSetBool(pIniSection, L"MatchBraces", bMatchBraces);
  IniSectionSetBool(pIniSection, L"AutoCloseTags", bAutoCloseTags);
  IniSectionSetBool(pIniSection, L"HighlightCurrentLine", bHiliteCurrentLine);
  IniSectionSetBool(pIniSection, L"HyperlinkHotspot", g_bHyperlinkHotspot);
  IniSectionSetBool(pIniSection, L"ScrollPastEOF", bScrollPastEOF);
  IniSectionSetBool(pIniSection, L"AutoIndent", bAutoIndent);
  IniSectionSetBool(pIniSection, L"AutoCompleteWords", bAutoCompleteWords);
  IniSectionSetBool(pIniSection, L"AccelWordNavigation", bAccelWordNavigation);
  IniSectionSetBool(pIniSection, L"ShowIndentGuides", bShowIndentGuides);
  IniSectionSetBool(pIniSection, L"TabsAsSpaces", bTabsAsSpacesG);
  IniSectionSetBool(pIniSection, L"TabIndents", bTabIndentsG);
  IniSectionSetBool(pIniSection, L"BackspaceUnindents", bBackspaceUnindents);
  IniSectionSetInt(pIniSection, L"TabWidth", iTabWidthG);
  IniSectionSetInt(pIniSection, L"IndentWidth", iIndentWidthG);
  IniSectionSetBool(pIniSection, L"MarkLongLines", bMarkLongLines);
  IniSectionSetPos(pIniSection, L"LongLinesLimit", iLongLinesLimitG);
  IniSectionSetInt(pIniSection, L"LongLineMode", iLongLineMode);
  IniSectionSetBool(pIniSection, L"ShowSelectionMargin", g_bShowSelectionMargin);
  IniSectionSetBool(pIniSection, L"ShowLineNumbers", bShowLineNumbers);
  IniSectionSetBool(pIniSection, L"ShowCodeFolding", g_bShowCodeFolding);
  IniSectionSetInt(pIniSection, L"MarkOccurrences", g_iMarkOccurrences);
  IniSectionSetBool(pIniSection, L"MarkOccurrencesMatchVisible", g_bMarkOccurrencesMatchVisible);
  IniSectionSetBool(pIniSection, L"MarkOccurrencesMatchCase", bMarkOccurrencesMatchCase);
  IniSectionSetBool(pIniSection, L"MarkOccurrencesMatchWholeWords", bMarkOccurrencesMatchWords);
  IniSectionSetBool(pIniSection, L"MarkOccurrencesCurrentWord", bMarkOccurrencesCurrentWord);
  IniSectionSetBool(pIniSection, L"ViewWhiteSpace", bViewWhiteSpace);
  IniSectionSetBool(pIniSection, L"ViewEOLs", bViewEOLs);
  IniSectionSetInt(pIniSection, L"DefaultEncoding", Encoding_MapIniSetting(false, g_iDefaultNewFileEncoding));
  IniSectionSetBool(pIniSection, L"UseDefaultForFileEncoding", bUseDefaultForFileEncoding);
  IniSectionSetBool(pIniSection, L"SkipUnicodeDetection", bSkipUnicodeDetection);
  IniSectionSetBool(pIniSection, L"SkipANSICodePageDetection", bSkipANSICodePageDetection);
  IniSectionSetInt(pIniSection, L"LoadASCIIasUTF8", bLoadASCIIasUTF8);
  IniSectionSetBool(pIniSection, L"LoadNFOasOEM", bLoadNFOasOEM);
  IniSectionSetBool(pIniSection, L"NoEncodingTags", bNoEncodingTags);
  IniSectionSetInt(pIniSection, L"DefaultEOLMode", g_iDefaultEOLMode);
  IniSectionSetBool(pIniSection, L"FixLineEndings", bFixLineEndings);
  IniSectionSetBool(pIniSection, L"FixTrailingBlanks", bAutoStripBlanks);
  IniSectionSetInt(pIniSection, L"PrintHeader", iPrintHeader);
  IniSectionSetInt(pIniSection, L"PrintFooter", iPrintFooter);
  IniSectionSetInt(pIniSection, L"PrintColorMode", iPrintColor);
  IniSectionSetInt(pIniSection, L"PrintZoom", iPrintZoom + 10);
  IniSectionSetInt(pIniSection, L"PrintMarginLeft", pagesetupMargin.left);
  IniSectionSetInt(pIniSection, L"PrintMarginTop", pagesetupMargin.top);
  IniSectionSetInt(pIniSection, L"PrintMarginRight", pagesetupMargin.right);
  IniSectionSetInt(pIniSection, L"PrintMarginBottom", pagesetupMargin.bottom);
  IniSectionSetBool(pIniSection, L"SaveBeforeRunningTools", bSaveBeforeRunningTools);
  IniSectionSetInt(pIniSection, L"FileWatchingMode", g_iFileWatchingMode);
  IniSectionSetBool(pIniSection, L"ResetFileWatching", g_bResetFileWatching);
  IniSectionSetInt(pIniSection, L"EscFunction", iEscFunction);
  IniSectionSetBool(pIniSection, L"AlwaysOnTop", bAlwaysOnTop);
  IniSectionSetBool(pIniSection, L"MinimizeToTray", bMinimizeToTray);
  IniSectionSetBool(pIniSection, L"TransparentMode", bTransparentMode);
  IniSectionSetBool(pIniSection, L"ShowToolbar", bShowToolbar);
  IniSectionSetBool(pIniSection, L"ShowStatusbar", bShowStatusbar);
  IniSectionSetInt(pIniSection, L"EncodingDlgSizeX", cxEncodingDlg);
  IniSectionSetInt(pIniSection, L"EncodingDlgSizeY", cyEncodingDlg);
  IniSectionSetInt(pIniSection, L"RecodeDlgSizeX", cxRecodeDlg);
  IniSectionSetInt(pIniSection, L"RecodeDlgSizeY", cyRecodeDlg);
  IniSectionSetInt(pIniSection, L"FileMRUDlgSizeX", cxFileMRUDlg);
  IniSectionSetInt(pIniSection, L"FileMRUDlgSizeY", cyFileMRUDlg);
  IniSectionSetInt(pIniSection, L"OpenWithDlgSizeX", cxOpenWithDlg);
  IniSectionSetInt(pIniSection, L"OpenWithDlgSizeY", cyOpenWithDlg);
  IniSectionSetInt(pIniSection, L"FavoritesDlgSizeX", cxFavoritesDlg);
  IniSectionSetInt(pIniSection, L"FavoritesDlgSizeY", cyFavoritesDlg);
  IniSectionSetInt(pIniSection, L"FindReplaceDlgPosX", xFindReplaceDlg);
  IniSectionSetInt(pIniSection, L"FindReplaceDlgPosY", yFindReplaceDlg);
  IniSectionSetInt(pIniSection, L"CustomSchemesDlgPosX", xCustomSchemesDlg);
  IniSectionSetInt(pIniSection, L"CustomSchemesDlgPosY", yCustomSchemesDlg);

  Toolbar_GetButtons(g_hwndToolbar, IDT_FILE_NEW, g_tchToolbarButtons, COUNTOF(g_tchToolbarButtons));
  if (StringCchCompareX(g_tchToolbarButtons, TBBUTTON_DEFAULT_IDS) == 0) { g_tchToolbarButtons[0] = L'\0'; }
  IniSectionSetString(pIniSection, L"ToolbarButtons", g_tchToolbarButtons);

  SaveIniSection(L"Settings", pIniSection);
  LocalFree(pIniSection);

  /*
    SaveSettingsNow(): query Window Dimensions
  */

  if (bSaveSettingsNow) {
    // GetWindowPlacement
    g_WinInfo = GetMyWindowPlacement(g_hwndMain,NULL);
  }

  int ResX = GetSystemMetrics(SM_CXSCREEN);
  int ResY = GetSystemMetrics(SM_CYSCREEN);

  WCHAR tchHighDpiToolBar[32];
  StringCchPrintf(tchHighDpiToolBar,COUNTOF(tchHighDpiToolBar),L"%ix%i HighDpiToolBar", ResX, ResY);
  IniSetInt(L"Window", tchHighDpiToolBar, iHighDpiToolBar);

  if (!IniGetInt(L"Settings2",L"StickyWindowPosition",0)) {

    WCHAR tchPosX[32], tchPosY[32], tchSizeX[32], tchSizeY[32], tchMaximized[32];

    StringCchPrintf(tchPosX,COUNTOF(tchPosX),L"%ix%i PosX",ResX,ResY);
    StringCchPrintf(tchPosY,COUNTOF(tchPosY),L"%ix%i PosY",ResX,ResY);
    StringCchPrintf(tchSizeX,COUNTOF(tchSizeX),L"%ix%i SizeX",ResX,ResY);
    StringCchPrintf(tchSizeY,COUNTOF(tchSizeY),L"%ix%i SizeY",ResX,ResY);
    StringCchPrintf(tchMaximized,COUNTOF(tchMaximized),L"%ix%i Maximized",ResX,ResY);

    IniSetInt(L"Window",tchPosX,g_WinInfo.x);
    IniSetInt(L"Window",tchPosY,g_WinInfo.y);
    IniSetInt(L"Window",tchSizeX,g_WinInfo.cx);
    IniSetInt(L"Window",tchSizeY,g_WinInfo.cy);
    IniSetInt(L"Window",tchMaximized,g_WinInfo.max);
  }

  // Scintilla Styles
  Style_Save();

}







//=============================================================================
//
//  ParseCommandLine()
//
//
void ParseCommandLine()
{

  LPWSTR lp1,lp2,lp3;
  bool bContinue = true;
  bool bIsFileArg = false;
  bool bIsNotepadReplacement = false;

  LPWSTR lpCmdLine = GetCommandLine();

  if (lstrlen(lpCmdLine) == 0)
    return;

  // Good old console can also send args separated by Tabs
  StrTab2Space(lpCmdLine);

  int len = lstrlen(lpCmdLine) + 2;
  lp1 = LocalAlloc(LPTR,sizeof(WCHAR)*len);
  lp2 = LocalAlloc(LPTR,sizeof(WCHAR)*len);
  lp3 = LocalAlloc(LPTR,sizeof(WCHAR)*len);

  // Start with 2nd argument
  ExtractFirstArgument(lpCmdLine,lp1,lp3,len);

  while (bContinue && ExtractFirstArgument(lp3,lp1,lp2,len))
  {

    // options
    if (!bIsFileArg && (StringCchCompareN(lp1,len,L"+",-1) == 0)) {
      flagMultiFileArg = 2;
      bIsFileArg = true;
    }

    else if (!bIsFileArg && (StringCchCompareN(lp1,len,L"-",-1) == 0)) {
      flagMultiFileArg = 1;
      bIsFileArg = true;
    }

    else if (!bIsFileArg && ((*lp1 == L'/') || (*lp1 == L'-')))
    {

      // LTrim
      StrLTrim(lp1,L"-/");

      // Encoding
      if (StringCchCompareIX(lp1,L"ANSI") == 0 || StringCchCompareIX(lp1,L"A") == 0 || StringCchCompareIX(lp1,L"MBCS") == 0)
        flagSetEncoding = IDM_ENCODING_ANSI-IDM_ENCODING_ANSI + 1;
      else if (StringCchCompareIX(lp1,L"UNICODE") == 0 || StringCchCompareIX(lp1,L"W") == 0)
        flagSetEncoding = IDM_ENCODING_UNICODE-IDM_ENCODING_ANSI + 1;
      else if (StringCchCompareIX(lp1,L"UNICODEBE") == 0 || StringCchCompareIX(lp1,L"UNICODE-BE") == 0)
        flagSetEncoding = IDM_ENCODING_UNICODEREV-IDM_ENCODING_ANSI + 1;
      else if (StringCchCompareIX(lp1,L"UTF8") == 0 || StringCchCompareIX(lp1,L"UTF-8") == 0)
        flagSetEncoding = IDM_ENCODING_UTF8-IDM_ENCODING_ANSI + 1;
      else if (StringCchCompareIX(lp1,L"UTF8SIG") == 0 || StringCchCompareIX(lp1,L"UTF-8SIG") == 0 ||
               StringCchCompareIX(lp1,L"UTF8SIGNATURE") == 0 || StringCchCompareIX(lp1,L"UTF-8SIGNATURE") == 0 ||
               StringCchCompareIX(lp1,L"UTF8-SIGNATURE") == 0 || StringCchCompareIX(lp1,L"UTF-8-SIGNATURE") == 0)
        flagSetEncoding = IDM_ENCODING_UTF8SIGN-IDM_ENCODING_ANSI + 1;

      // EOL Mode
      else if (StringCchCompareIX(lp1,L"CRLF") == 0 || StringCchCompareIX(lp1,L"CR+LF") == 0)
        flagSetEOLMode = IDM_LINEENDINGS_CRLF-IDM_LINEENDINGS_CRLF + 1;
      else if (StringCchCompareIX(lp1,L"LF") == 0)
        flagSetEOLMode = IDM_LINEENDINGS_LF-IDM_LINEENDINGS_CRLF + 1;
      else if (StringCchCompareIX(lp1,L"CR") == 0)
        flagSetEOLMode = IDM_LINEENDINGS_CR-IDM_LINEENDINGS_CRLF + 1;

      // Shell integration
      else if (StrCmpNI(lp1,L"appid=",CSTRLEN(L"appid=")) == 0) {
        StringCchCopyN(g_wchAppUserModelID,COUNTOF(g_wchAppUserModelID),
                       lp1 + CSTRLEN(L"appid="),len - CSTRLEN(L"appid="));
        StrTrim(g_wchAppUserModelID,L" ");
        if (StringCchLenW(g_wchAppUserModelID,COUNTOF(g_wchAppUserModelID)) == 0)
          StringCchCopy(g_wchAppUserModelID,COUNTOF(g_wchAppUserModelID),L"Notepad3");
      }

      else if (StrCmpNI(lp1,L"sysmru=",CSTRLEN(L"sysmru=")) == 0) {
        WCHAR wch[16];
        StringCchCopyN(wch,COUNTOF(wch),lp1 + CSTRLEN(L"sysmru="),COUNTOF(wch));
        StrTrim(wch,L" ");
        if (*wch == L'1')
          flagUseSystemMRU = 2;
        else
          flagUseSystemMRU = 1;
      }

      // Relaunch elevated
      else if (StrCmpNI(lp1,L"tmpfbuf=",CSTRLEN(L"tmpfbuf=")) == 0) {
        StringCchCopyN(g_szTmpFilePath,COUNTOF(g_szTmpFilePath),
          lp1 + CSTRLEN(L"tmpfbuf="),len - CSTRLEN(L"tmpfbuf="));
        TrimString(g_szTmpFilePath);
        PathUnquoteSpaces(g_szTmpFilePath);
        NormalizePathEx(g_szTmpFilePath,COUNTOF(g_szTmpFilePath));
        flagBufferFile = 1;
      }

      else switch (*CharUpper(lp1))
      {

        case L'N':
          flagReuseWindow = 0;
          flagNoReuseWindow = 1;
          if (*CharUpper(lp1+1) == L'S')
            flagSingleFileInstance = 1;
          else
            flagSingleFileInstance = 0;
          break;

        case L'R':
          flagReuseWindow = 1;
          flagNoReuseWindow = 0;
          if (*CharUpper(lp1+1) == L'S')
            flagSingleFileInstance = 1;
          else
            flagSingleFileInstance = 0;
          break;

        case L'F':
          if (*(lp1+1) == L'0' || *CharUpper(lp1+1) == L'O')
            StringCchCopy(g_wchIniFile,COUNTOF(g_wchIniFile),L"*?");
          else if (ExtractFirstArgument(lp2,lp1,lp2,len)) {
            StringCchCopyN(g_wchIniFile,COUNTOF(g_wchIniFile),lp1,len);
            TrimString(g_wchIniFile);
            PathUnquoteSpaces(g_wchIniFile);
            NormalizePathEx(g_wchIniFile,COUNTOF(g_wchIniFile));
          }
          break;

        case L'I':
          flagStartAsTrayIcon = 1;
          break;

        case L'O':
          if (*(lp1+1) == L'0' || *(lp1+1) == L'-' || *CharUpper(lp1+1) == L'O')
            flagAlwaysOnTop = 1;
          else
            flagAlwaysOnTop = 2;
          break;

        case L'P':
          {
            WCHAR *lp = lp1;
            if (StrCmpNI(lp1,L"POS:",CSTRLEN(L"POS:")) == 0)
              lp += CSTRLEN(L"POS:") -1;
            else if (StrCmpNI(lp1,L"POS",CSTRLEN(L"POS")) == 0)
              lp += CSTRLEN(L"POS") -1;
            else if (*(lp1+1) == L':')
              lp += 1;
            else if (bIsNotepadReplacement) {
              if (*(lp1+1) == L'T')
                ExtractFirstArgument(lp2,lp1,lp2,len);
              break;
            }
            if (*(lp+1) == L'0' || *CharUpper(lp+1) == L'O') {
              flagPosParam = 1;
              flagDefaultPos = 1;
            }
            else if (*CharUpper(lp+1) == L'D' || *CharUpper(lp+1) == L'S') {
              flagPosParam = 1;
              flagDefaultPos = (StrChrI((lp+1),L'L')) ? 3 : 2;
            }
            else if (StrChrI(L"FLTRBM",*(lp+1))) {
              WCHAR *p = (lp+1);
              flagPosParam = 1;
              flagDefaultPos = 0;
              while (*p) {
                switch (*CharUpper(p)) {
                  case L'F':
                    flagDefaultPos &= ~(4|8|16|32);
                    flagDefaultPos |= 64;
                    break;
                  case L'L':
                    flagDefaultPos &= ~(8|64);
                    flagDefaultPos |= 4;
                    break;
                  case  L'R':
                    flagDefaultPos &= ~(4|64);
                    flagDefaultPos |= 8;
                    break;
                  case L'T':
                    flagDefaultPos &= ~(32|64);
                    flagDefaultPos |= 16;
                    break;
                  case L'B':
                    flagDefaultPos &= ~(16|64);
                    flagDefaultPos |= 32;
                    break;
                  case L'M':
                    if (flagDefaultPos == 0)
                      flagDefaultPos |= 64;
                    flagDefaultPos |= 128;
                    break;
                }
                p = CharNext(p);
              }
            }
            else if (ExtractFirstArgument(lp2,lp1,lp2,len)) {
              int itok =
                swscanf_s(lp1,L"%i,%i,%i,%i,%i",&g_WinInfo.x,&g_WinInfo.y,&g_WinInfo.cx,&g_WinInfo.cy,&g_WinInfo.max);
              if (itok == 4 || itok == 5) { // scan successful
                flagPosParam = 1;
                flagDefaultPos = 0;

                if (g_WinInfo.cx < 1) g_WinInfo.cx = CW_USEDEFAULT;
                if (g_WinInfo.cy < 1) g_WinInfo.cy = CW_USEDEFAULT;
                if (g_WinInfo.max) g_WinInfo.max = 1;
                if (itok == 4) g_WinInfo.max = 0;
              }
            }
          }
          break;

        case L'T':
          if (ExtractFirstArgument(lp2,lp1,lp2,len)) {
            StringCchCopyN(szTitleExcerpt,COUNTOF(szTitleExcerpt),lp1,len);
            fKeepTitleExcerpt = 1;
          }
          break;

        case L'C':
          flagNewFromClipboard = 1;
          break;

        case L'B':
          flagPasteBoard = 1;
          break;

        case L'E':
          if (ExtractFirstArgument(lp2,lp1,lp2,len)) {
            if (lpEncodingArg)
              LocalFree(lpEncodingArg);
            lpEncodingArg = StrDup(lp1);
          }
          break;

        case L'G':
          if (ExtractFirstArgument(lp2,lp1,lp2,len)) {
            int itok =
              swscanf_s(lp1,L"%i,%i",&iInitialLine,&iInitialColumn);
            if (itok == 1 || itok == 2) { // scan successful
              flagJumpTo = 1;
            }
          }
          break;

        case L'M':
          {
            bool bFindUp  = false;
            bool bRegex   = false;
            bool bTransBS = false;

            if (StrChr(lp1,L'-'))
              bFindUp = true;
            if (StrChr(lp1,L'R'))
              bRegex = true;
            if (StrChr(lp1,L'B'))
              bTransBS = true;

            if (ExtractFirstArgument(lp2,lp1,lp2,len)) {
              if (lpMatchArg)
                LocalFree(lpMatchArg);
              lpMatchArg = StrDup(lp1);
              flagMatchText = 1;

              if (bFindUp)
                flagMatchText |= 2;

              if (bRegex) {
                flagMatchText &= ~8;
                flagMatchText |= 4;
              }

              if (bTransBS) {
                flagMatchText &= ~4;
                flagMatchText |= 8;
              }
            }
          }
          break;

        case L'L':
          if (*(lp1+1) == L'0' || *(lp1+1) == L'-' || *CharUpper(lp1+1) == L'O')
            flagChangeNotify = 1;
          else
            flagChangeNotify = 2;
          break;

        case L'Q':
          flagQuietCreate = 1;
          break;

        case L'S':
          if (ExtractFirstArgument(lp2,lp1,lp2,len)) {
            if (lpSchemeArg)
              LocalFree(lpSchemeArg);
            lpSchemeArg = StrDup(lp1);
            flagLexerSpecified = 1;
          }
          break;

        case L'D':
          if (lpSchemeArg) {
            LocalFree(lpSchemeArg);
            lpSchemeArg = NULL;
          }
          iInitialLexer = 0;
          flagLexerSpecified = 1;
          break;

        case L'H':
          if (lpSchemeArg) {
            LocalFree(lpSchemeArg);
            lpSchemeArg = NULL;
          }
          iInitialLexer = 35;
          flagLexerSpecified = 1;
          break;

        case L'X':
          if (lpSchemeArg) {
            LocalFree(lpSchemeArg);
            lpSchemeArg = NULL;
          }
          iInitialLexer = 36;
          flagLexerSpecified = 1;
          break;

        case L'U':
          flagRelaunchElevated = 1;
          break;

        case L'Z':
          ExtractFirstArgument(lp2,lp1,lp2,len);
          flagMultiFileArg = 1;
          bIsNotepadReplacement = true;
          break;

        case L'?':
          flagDisplayHelp = 1;
          break;

        case L'V':
          flagPrintFileAndLeave = 1;
          if (*CharUpper(lp1 + 1) == L'D')
            flagPrintFileAndLeave = 2;  // open printer dialog
          break;

        default:
          break;

      }

    }

    // pathname
    else
    {
      LPWSTR lpFileBuf = LocalAlloc(LPTR,sizeof(WCHAR)*len);

      cchiFileList = lstrlen(lpCmdLine) - lstrlen(lp3);

      if (lpFileArg) {
        FreeMem(lpFileArg);
        //lpFileArg = NULL;
      }
      lpFileArg = AllocMem(sizeof(WCHAR)*FILE_ARG_BUF, HEAP_ZERO_MEMORY); // changed for ActivatePrevInst() needs
      StringCchCopy(lpFileArg,FILE_ARG_BUF,lp3);

      PathFixBackslashes(lpFileArg);

      if (!PathIsRelative(lpFileArg) && !PathIsUNC(lpFileArg) &&
            PathGetDriveNumber(lpFileArg) == -1 /*&& PathGetDriveNumber(g_wchWorkingDirectory) != -1*/) {

        WCHAR wchPath[FILE_ARG_BUF] = { L'\0' };
        StringCchCopy(wchPath,COUNTOF(wchPath),g_wchWorkingDirectory);
        PathStripToRoot(wchPath);
        PathCchAppend(wchPath,COUNTOF(wchPath),lpFileArg);
        StringCchCopy(lpFileArg,FILE_ARG_BUF,wchPath);
      }

      StrTrim(lpFileArg,L" \"");

      while (cFileList < 32 && ExtractFirstArgument(lp3,lpFileBuf,lp3,len)) {
        PathQuoteSpaces(lpFileBuf);
        lpFileList[cFileList++] = StrDup(lpFileBuf);
      }

      bContinue = false;
      LocalFree(lpFileBuf);
    }

    // Continue with next argument
    if (bContinue)
      StringCchCopy(lp3,len,lp2);

  }

  LocalFree(lp1);
  LocalFree(lp2);
  LocalFree(lp3);
}


//=============================================================================
//
//  LoadFlags()
//
//
void LoadFlags()
{
  WCHAR *pIniSection = LocalAlloc(LPTR,sizeof(WCHAR)*32*1024);
  int   cchIniSection = (int)LocalSize(pIniSection)/sizeof(WCHAR);

  LoadIniSection(L"Settings2",pIniSection,cchIniSection);

  if (!flagReuseWindow && !flagNoReuseWindow) {

    if (!IniSectionGetInt(pIniSection,L"ReuseWindow",0))
      flagNoReuseWindow = 1;

    if (IniSectionGetInt(pIniSection,L"SingleFileInstance",0))
      flagSingleFileInstance = 1;
  }

  if (flagMultiFileArg == 0) {
    if (IniSectionGetInt(pIniSection,L"MultiFileArg",0))
      flagMultiFileArg = 2;
  }

  if (IniSectionGetInt(pIniSection,L"RelativeFileMRU",1))
    flagRelativeFileMRU = 1;

  if (IniSectionGetInt(pIniSection,L"PortableMyDocs",flagRelativeFileMRU))
    flagPortableMyDocs = 1;

  if (IniSectionGetInt(pIniSection,L"NoFadeHidden",0))
    flagNoFadeHidden = 1;

  flagToolbarLook = IniSectionGetInt(pIniSection,L"ToolbarLook",IsXP() ? 1 : 2);
  flagToolbarLook = max(min(flagToolbarLook,2),0);

  if (IniSectionGetInt(pIniSection,L"SimpleIndentGuides",0))
    flagSimpleIndentGuides = 1;

  if (IniSectionGetInt(pIniSection,L"NoHTMLGuess",0))
    flagNoHTMLGuess = 1;

  if (IniSectionGetInt(pIniSection,L"NoCGIGuess",0))
    flagNoCGIGuess = 1;

  if (IniSectionGetInt(pIniSection,L"NoFileVariables",0))
    flagNoFileVariables = 1;

  if (StringCchLenW(g_wchAppUserModelID,COUNTOF(g_wchAppUserModelID)) == 0) {
    IniSectionGetString(pIniSection,L"ShellAppUserModelID",L"Notepad3",
      g_wchAppUserModelID,COUNTOF(g_wchAppUserModelID));
  }

  if (flagUseSystemMRU == 0) {
    if (IniSectionGetInt(pIniSection, L"ShellUseSystemMRU", 0)) {
      flagUseSystemMRU = 2;
    }
  }

  LocalFree(pIniSection);
}


//=============================================================================
//
//  FindIniFile()
//
//
bool CheckIniFile(LPWSTR lpszFile,LPCWSTR lpszModule)
{
  WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
  WCHAR tchBuild[MAX_PATH] = { L'\0' };
  ExpandEnvironmentStrings(lpszFile,tchFileExpanded,COUNTOF(tchFileExpanded));

  if (PathIsRelative(tchFileExpanded)) {
    // program directory
    StringCchCopy(tchBuild,COUNTOF(tchBuild),lpszModule);
    StringCchCopy(PathFindFileName(tchBuild),COUNTOF(tchBuild),tchFileExpanded);

    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile,MAX_PATH,tchBuild);
      return true;
    }
    // %appdata%
    //if (S_OK == SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, tchBuild)) {
    if (GetKnownFolderPath(&FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild,COUNTOF(tchBuild),tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile,MAX_PATH,tchBuild);
        return true;
      }
    }
    // general
    if (SearchPath(NULL,tchFileExpanded,L".ini",COUNTOF(tchBuild),tchBuild,NULL)) {
      StringCchCopy(lpszFile,MAX_PATH,tchBuild);
      return true;
    }
  }

  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile,MAX_PATH,tchFileExpanded);
    return true;
  }

  return false;
}

bool CheckIniFileRedirect(LPWSTR lpszFile,LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(L"Notepad3",L"Notepad3.ini",L"",tch,COUNTOF(tch),lpszFile)) {
    if (CheckIniFile(tch,lpszModule)) {
      StringCchCopy(lpszFile,MAX_PATH,tch);
      return true;
    }
    else {
      WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
      ExpandEnvironmentStrings(tch,tchFileExpanded,COUNTOF(tchFileExpanded));
      if (PathIsRelative(tchFileExpanded)) {
        StringCchCopy(lpszFile,MAX_PATH,lpszModule);
        StringCchCopy(PathFindFileName(lpszFile),MAX_PATH,tchFileExpanded);
        return true;
      }
      else {
        StringCchCopy(lpszFile,MAX_PATH,tchFileExpanded);
        return true;
      }
    }
  }
  return false;
}

int FindIniFile() {

  WCHAR tchTest[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };
  GetModuleFileName(NULL,tchModule,COUNTOF(tchModule));

  if (StringCchLenW(g_wchIniFile,COUNTOF(g_wchIniFile))) {
    if (StringCchCompareIX(g_wchIniFile,L"*?") == 0)
      return(0);
    else {
      if (!CheckIniFile(g_wchIniFile,tchModule)) {
        ExpandEnvironmentStringsEx(g_wchIniFile,COUNTOF(g_wchIniFile));
        if (PathIsRelative(g_wchIniFile)) {
          StringCchCopy(tchTest,COUNTOF(tchTest),tchModule);
          PathRemoveFileSpec(tchTest);
          PathCchAppend(tchTest,COUNTOF(tchTest),g_wchIniFile);
          StringCchCopy(g_wchIniFile,COUNTOF(g_wchIniFile),tchTest);
        }
      }
    }
  }
  else {
    StringCchCopy(tchTest,COUNTOF(tchTest),PathFindFileName(tchModule));
    PathCchRenameExtension(tchTest,COUNTOF(tchTest),L".ini");
    bool bFound = CheckIniFile(tchTest,tchModule);

    if (!bFound) {
      StringCchCopy(tchTest,COUNTOF(tchTest),L"Notepad3.ini");
      bFound = CheckIniFile(tchTest,tchModule);
    }

    if (bFound) {

      // allow two redirections: administrator -> user -> custom
      if (CheckIniFileRedirect(tchTest,tchModule))
        CheckIniFileRedirect(tchTest,tchModule);

      StringCchCopy(g_wchIniFile,COUNTOF(g_wchIniFile),tchTest);
    }
    else {
      StringCchCopy(g_wchIniFile,COUNTOF(g_wchIniFile),tchModule);
      PathCchRenameExtension(g_wchIniFile,COUNTOF(g_wchIniFile),L".ini");
    }
  }

  NormalizePathEx(g_wchIniFile,COUNTOF(g_wchIniFile));
 
  return(1);
}


int TestIniFile() {

  if (StringCchCompareIX(g_wchIniFile,L"*?") == 0) {
    StringCchCopy(g_wchIniFile2,COUNTOF(g_wchIniFile2),L"");
    StringCchCopy(g_wchIniFile,COUNTOF(g_wchIniFile),L"");
    return(0);
  }

  if (PathIsDirectory(g_wchIniFile) || *CharPrev(g_wchIniFile,StrEnd(g_wchIniFile)) == L'\\') {
    WCHAR wchModule[MAX_PATH] = { L'\0' };
    GetModuleFileName(NULL,wchModule,COUNTOF(wchModule));
    PathCchAppend(g_wchIniFile,COUNTOF(g_wchIniFile),PathFindFileName(wchModule));
    PathCchRenameExtension(g_wchIniFile,COUNTOF(g_wchIniFile),L".ini");
    if (!PathFileExists(g_wchIniFile)) {
      StringCchCopy(PathFindFileName(g_wchIniFile),COUNTOF(g_wchIniFile),L"Notepad3.ini");
      if (!PathFileExists(g_wchIniFile)) {
        StringCchCopy(PathFindFileName(g_wchIniFile),COUNTOF(g_wchIniFile),PathFindFileName(wchModule));
        PathCchRenameExtension(g_wchIniFile,COUNTOF(g_wchIniFile),L".ini");
      }
    }
  }
  
  NormalizePathEx(g_wchIniFile,COUNTOF(g_wchIniFile));

  if (!PathFileExists(g_wchIniFile) || PathIsDirectory(g_wchIniFile)) {
    StringCchCopy(g_wchIniFile2,COUNTOF(g_wchIniFile2),g_wchIniFile);
    StringCchCopy(g_wchIniFile,COUNTOF(g_wchIniFile),L"");
    return(0);
  }
  else
    return(1);
}


int CreateIniFile() {

  return(CreateIniFileEx(g_wchIniFile));
}


int CreateIniFileEx(LPCWSTR lpszIniFile) {

  if (*lpszIniFile) {

    WCHAR *pwchTail = StrRChrW(lpszIniFile, NULL, L'\\');
    if (pwchTail) {
      *pwchTail = 0;
      SHCreateDirectoryEx(NULL,lpszIniFile,NULL);
      *pwchTail = L'\\';
    }

    HANDLE hFile = CreateFile(lpszIniFile,
              GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,
              NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    dwLastIOError = GetLastError();
    if (hFile != INVALID_HANDLE_VALUE) {
      if (GetFileSize(hFile,NULL) == 0) {
        DWORD dw;
        WriteFile(hFile,(LPCVOID)L"\xFEFF[Notepad3]\r\n",26,&dw,NULL);
      }
      CloseHandle(hFile);
      return(1);
    }
    else
      return(0);
  }

  else
    return(0);
}






//=============================================================================
//
//  MarkAllOccurrences()
// 
void MarkAllOccurrences(int delay)
{
  static CmdMessageQueue_t mqc = { NULL, WM_COMMAND, (WPARAM)MAKELONG(IDT_TIMER_MAIN_MRKALL, 1), (LPARAM)0 , 0 };
  mqc.hwnd = g_hwndMain;
  _MQ_AppendCmd(&mqc, (UINT)(delay <= 0 ? 0 : _MQ_ms(delay)));
}


//=============================================================================
//
//  UpdateVisibleUrlHotspot()
// 
void UpdateVisibleUrlHotspot(int delay)
{
  static CmdMessageQueue_t mqc = { NULL, WM_COMMAND, (WPARAM)MAKELONG(IDT_TIMER_UPDATE_HOTSPOT, 1), (LPARAM)0 , 0 };
  mqc.hwnd = g_hwndMain;
  _MQ_AppendCmd(&mqc, (UINT)(delay <= 0 ? 0 : _MQ_ms(delay)));
}


//=============================================================================
//
//  UpdateToolbar()
//
#define EnableTool(id,b) SendMessage(g_hwndToolbar,TB_ENABLEBUTTON,id, MAKELONG(((b) ? 1 : 0), 0))
#define CheckTool(id,b)  SendMessage(g_hwndToolbar,TB_CHECKBUTTON,id, MAKELONG((b),0))


void UpdateToolbar()
{
  SetWindowTitle(g_hwndMain, uidsAppTitle, flagIsElevated, IDS_UNTITLED, g_wchCurFile,
                 iPathNameFormat, IsDocumentModified || Encoding_HasChanged(CPI_GET),
                 IDS_READONLY, g_bFileReadOnly, szTitleExcerpt);

  if (!bShowToolbar) { return; }

  EnableTool(IDT_FILE_ADDTOFAV, StringCchLenW(g_wchCurFile, COUNTOF(g_wchCurFile)));
  EnableTool(IDT_FILE_SAVE, (IsDocumentModified || Encoding_HasChanged(CPI_GET)) /*&& !bReadOnly*/);
  CheckTool(IDT_VIEW_WORDWRAP, g_bWordWrap);
  CheckTool(IDT_VIEW_CHASING_DOCTAIL, g_bChasingDocTail);

  bool b1 = SciCall_IsSelectionEmpty();
  bool b2 = (bool)(SciCall_GetTextLength() > 0);
  bool ro = SciCall_GetReadOnly();
  bool tv = EditToggleView(g_hwndEdit, false);

  EnableTool(IDT_EDIT_UNDO, SciCall_CanUndo() && !ro);
  EnableTool(IDT_EDIT_REDO, SciCall_CanRedo() && !ro);
  EnableTool(IDT_EDIT_PASTE, SciCall_CanPaste() && !ro);

  EnableTool(IDT_FILE_LAUNCH, b2);


  EnableTool(IDT_EDIT_FIND, b2);
  //EnableTool(IDT_EDIT_FINDNEXT,b2);
  //EnableTool(IDT_EDIT_FINDPREV,b2 && strlen(g_efrData.szFind));
  EnableTool(IDT_EDIT_REPLACE, b2 && !ro);

  EnableTool(IDT_EDIT_CUT, !b1 && !ro);
  EnableTool(IDT_EDIT_COPY, !b1 && !ro);
  EnableTool(IDT_EDIT_CLEAR, !b1 && !ro);

  EnableTool(IDT_VIEW_TOGGLEFOLDS, b2 && (g_bCodeFoldingAvailable && g_bShowCodeFolding));

  EnableTool(IDT_VIEW_TOGGLE_VIEW, b2 && ((g_iMarkOccurrences > 0) && !g_bMarkOccurrencesMatchVisible));
  CheckTool(IDT_VIEW_TOGGLE_VIEW, tv);
}




//=============================================================================
//
//  _CalculateStatusbarSections
//  vSectionWidth[] must be pre-filled with -1
//
#define txtWidth 80
typedef WCHAR sectionTxt_t[txtWidth];

static void __fastcall _CalculateStatusbarSections(int vSectionWidth[], sectionTxt_t tchStatusBar[], bool* bIsUpdNeeded)
{
  static int s_iWinFormerWidth = -1;
  if (s_iWinFormerWidth != g_WinCurrentWidth) {
    *bIsUpdNeeded = true;
    s_iWinFormerWidth = g_WinCurrentWidth;
  }
  if (!(*bIsUpdNeeded)) { return; }

  // count fixed and dynamic optimized pixels
  int pxCount = 0;
  for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
    if (g_iStatusbarVisible[i]) {
      if (g_iStatusbarWidthSpec[i] == 0) { // dynamic optimized
        vSectionWidth[i] = StatusCalcPaneWidth(g_hwndStatus, tchStatusBar[i]);
      }
      else if (g_iStatusbarWidthSpec[i] < -1) { // fixed pixel count
        vSectionWidth[i] = -(g_iStatusbarWidthSpec[i]);
      }
      //else { /* 0,-1 : relative counts */ }
      // accumulate
      if (vSectionWidth[i] > 0) { pxCount += vSectionWidth[i]; }
    }
  }

  int const iPropSectTotalWidth = g_WinCurrentWidth - pxCount - STAUSBAR_RIGHT_MARGIN;

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
      int const iMinWidth = StatusCalcPaneWidth(g_hwndStatus, tchStatusBar[i]);
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
    int iTotalPropWidth = 0;
    for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
      if (bIsPropSection[i]) {
        int const width = (totalCnt > 1) ? ((vPropWidth[i] + iCeilFloor) / totalCnt) : 0;
        vPropWidth[i] = width;
        iTotalPropWidth += width;
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
//  UpdateStatusbar()
//
//
const static WCHAR* FR_Status[] = { L"[>--<]", L"[>>--]", L"[>>-+]", L"[+->]>", L"[--<<]", L"[+-<<]", L"<[<-+]"};

FR_STATES g_FindReplaceMatchFoundState = FND_NOP;


void UpdateStatusbar(bool bUpdNeeded)
{
  if (!bShowStatusbar) { return; }

  bool bIsUpdateNeeded = bUpdNeeded;

  static sectionTxt_t tchStatusBar[STATUS_SECTOR_COUNT];
  static WCHAR tchFRStatus[128] = { L'\0' };
  static WCHAR tchTmp[32] = { L'\0' };

  DocPos const iPos = SciCall_GetCurrentPos();
  bool const bIsSelectionEmpty = SciCall_IsSelectionEmpty();

  // ------------------------------------------------------

  static WCHAR tchLn[32] = { L'\0' };
  static DocLn s_iLnFromPos = -1;
  DocLn const iLnFromPos = SciCall_LineFromPosition(iPos) + 1;
  if (s_iLnFromPos != iLnFromPos) {
    StringCchPrintf(tchLn, COUNTOF(tchLn), L"%td", iLnFromPos);
    FormatNumberStr(tchLn);
  }

  static WCHAR tchLines[32] = { L'\0' };
  static DocLn s_iLnCnt = -1;
  DocLn const iLnCnt = SciCall_GetLineCount();
  if (s_iLnCnt != iLnCnt) {
    StringCchPrintf(tchLines, COUNTOF(tchLines), L"%td", iLnCnt);
    FormatNumberStr(tchLines);
  }

  if ((s_iLnFromPos != iLnFromPos) || (s_iLnFromPos != iLnFromPos)) 
  {
    FormatString(tchStatusBar[STATUS_DOCLINE], txtWidth, IDS_STATUS_DOCLINE, g_mxStatusBarPrefix[STATUS_DOCLINE], tchLn, tchLines);
    s_iLnFromPos = iLnFromPos;
    s_iLnCnt = iLnCnt;
    bIsUpdateNeeded = true;
  }

  // ------------------------------------------------------

  static WCHAR tchCol[32] = { L'\0' };

  static DocPos s_iCol = -1;
  DocPos const iCol = SciCall_GetColumn(iPos) + SciCall_GetSelectionNCaretVirtualSpace(0) + 1;
  if (s_iCol != iCol) {
    StringCchPrintf(tchCol, COUNTOF(tchCol), L"%td", iCol);
    FormatNumberStr(tchCol);
  }

  static WCHAR tchCols[32] = { L'\0' };
  static bool s_bmarkLongLines = false;
  static int s_iLongLinesLimit = -1;
  if ((s_bmarkLongLines != bMarkLongLines) || (s_iCol != iCol) || (s_iLongLinesLimit != g_iLongLinesLimit)) {
    if (bMarkLongLines) {
      StringCchPrintf(tchCols, COUNTOF(tchCols), L"%td", g_iLongLinesLimit);
      FormatNumberStr(tchCols);
      FormatString(tchStatusBar[STATUS_DOCCOLUMN], txtWidth, IDS_STATUS_DOCCOLUMN2, g_mxStatusBarPrefix[STATUS_DOCCOLUMN], tchCol, tchCols);
    }
    else {
      tchCols[0] = L'\0';
      FormatString(tchStatusBar[STATUS_DOCCOLUMN], txtWidth, IDS_STATUS_DOCCOLUMN, g_mxStatusBarPrefix[STATUS_DOCCOLUMN], tchCol);
    }
    s_iCol = iCol;
    s_bmarkLongLines = bMarkLongLines;
    s_iLongLinesLimit = g_iLongLinesLimit;
    bIsUpdateNeeded = true;
  }

  // ------------------------------------------------------


  // number of selected chars in statusbar
  static WCHAR tchSel[32] = { L'\0' };
  static WCHAR tchSelB[64] = { L'\0' };

  static bool s_bIsSelCountable = false;
  static DocPos s_iSelStart = -1;
  static DocPos s_iSelEnd = -1;

  bool const bIsSelCountable = !(bIsSelectionEmpty || SciCall_IsSelectionRectangle());
  DocPos const iSelStart = (!bIsSelCountable ? 0 : SciCall_GetSelectionStart());
  DocPos const iSelEnd = (!bIsSelCountable ? 0 : SciCall_GetSelectionEnd());

  if ((s_bIsSelCountable != bIsSelCountable) || (s_iSelStart != iSelStart) || (s_iSelEnd != iSelEnd)) {

    if (bIsSelCountable)
    {
      const DocPos iSel = (DocPos)SendMessage(g_hwndEdit, SCI_COUNTCHARACTERS, iSelStart, iSelEnd);
      StringCchPrintf(tchSel, COUNTOF(tchSel), L"%td", iSel);
      FormatNumberStr(tchSel);
      StrFormatByteSize((iSelEnd - iSelStart), tchSelB, COUNTOF(tchSelB));
    }
    else {
      tchSel[0] = L'-'; tchSel[1] = L'-'; tchSel[2] = L'\0';
      tchSelB[0] = L'0'; tchSelB[1] = L'\0';
    }

    FormatString(tchStatusBar[STATUS_SELECTION], txtWidth, IDS_STATUS_SELECTION, g_mxStatusBarPrefix[STATUS_SELECTION], tchSel);
    FormatString(tchStatusBar[STATUS_SELCTBYTES], txtWidth, IDS_STATUS_SELCTBYTES, g_mxStatusBarPrefix[STATUS_SELCTBYTES], tchSelB);

    s_bIsSelCountable = bIsSelCountable;
    s_iSelStart = iSelStart;
    s_iSelEnd = iSelEnd;
    bIsUpdateNeeded = true;
  }

  // ------------------------------------------------------

  // number of selected lines in statusbar
  static WCHAR tchLinesSelected[32] = { L'\0' };

  static bool s_bIsSelectionEmpty = true;
  static DocLn s_iLinesSelected = -1;

  DocLn const iLineStart = SciCall_LineFromPosition(iSelStart);
  DocLn const iLineEnd = SciCall_LineFromPosition(iSelEnd);
  DocPos const iStartOfLinePos = SciCall_PositionFromLine(iLineEnd);

  DocLn const iLinesSelected = ((iSelStart != iSelEnd) && (iStartOfLinePos != iSelEnd)) ? ((iLineEnd - iLineStart) + 1) : (iLineEnd - iLineStart);

  if ((s_bIsSelectionEmpty != bIsSelectionEmpty) || (s_iLinesSelected != iLinesSelected))
  {
    if (bIsSelectionEmpty) {
      tchLinesSelected[0] = L'-';
      tchLinesSelected[1] = L'-';
      tchLinesSelected[2] = L'\0';
    }
    else {

      StringCchPrintf(tchLinesSelected, COUNTOF(tchLinesSelected), L"%i", iLinesSelected);
      FormatNumberStr(tchLinesSelected);
    }

    FormatString(tchStatusBar[STATUS_SELCTLINES], txtWidth, IDS_STATUS_SELCTLINES, g_mxStatusBarPrefix[STATUS_SELCTLINES], tchLinesSelected);

    s_bIsSelectionEmpty = bIsSelectionEmpty;
    s_iLinesSelected = iLinesSelected;
    bIsUpdateNeeded = true;
  }

  // ------------------------------------------------------

  // number of occurrence marks found
  static WCHAR tchOcc[32] = { L'\0' };

  static int s_iMarkOccurrencesCount = -111;
  static bool s_bMOVisible = false;

  if ((s_bMOVisible != g_bMarkOccurrencesMatchVisible) || (s_iMarkOccurrencesCount != g_iMarkOccurrencesCount))
  {
    if ((g_iMarkOccurrencesCount >= 0) && !g_bMarkOccurrencesMatchVisible)
    {
      if ((g_iMarkOccurrencesMaxCount < 0) || (g_iMarkOccurrencesCount < g_iMarkOccurrencesMaxCount))
      {
        StringCchPrintf(tchOcc, COUNTOF(tchOcc), L"%i", g_iMarkOccurrencesCount);
        FormatNumberStr(tchOcc);
      }
      else {
        StringCchPrintf(tchTmp, COUNTOF(tchTmp), L"%i", g_iMarkOccurrencesCount);
        FormatNumberStr(tchTmp);
        StringCchPrintf(tchOcc, COUNTOF(tchOcc), L">= %s", tchTmp);
      }
    }
    else {
      StringCchCopy(tchOcc, COUNTOF(tchOcc), L"--");
    }

    FormatString(tchStatusBar[STATUS_OCCURRENCE], txtWidth, IDS_STATUS_OCCURRENCE, g_mxStatusBarPrefix[STATUS_OCCURRENCE], tchOcc);

    s_bMOVisible = g_bMarkOccurrencesMatchVisible;
    s_iMarkOccurrencesCount = g_iMarkOccurrencesCount;
    bIsUpdateNeeded = true;
  }

  // ------------------------------------------------------

  // get number of bytes in current encoding
  static WCHAR tchBytes[32] = { L'\0' };
  static DocPos s_iTextLength = -1;
  DocPos const iTextLength = SciCall_GetTextLength();
  if (s_iTextLength != iTextLength) {
    StrFormatByteSize(iTextLength, tchBytes, COUNTOF(tchBytes));
    FormatString(tchStatusBar[STATUS_DOCSIZE], txtWidth, IDS_STATUS_DOCSIZE, g_mxStatusBarPrefix[STATUS_DOCSIZE], tchBytes);
    s_iTextLength = iTextLength;
    bIsUpdateNeeded = true;
  }

  static int s_iEncoding = -1;
  int const iEncoding = Encoding_Current(CPI_GET);
  if (s_iEncoding != iEncoding) {
    Encoding_SetLabel(iEncoding);
    StringCchPrintf(tchStatusBar[STATUS_CODEPAGE], txtWidth, L"%s%s", g_mxStatusBarPrefix[STATUS_CODEPAGE], Encoding_GetLabel(iEncoding));
    s_iEncoding = iEncoding;
    bIsUpdateNeeded = true;
  }

  // ------------------------------------------------------

  static int s_iEOLMode = -1;
  if (s_iEOLMode != g_iEOLMode) {
    if (g_iEOLMode == SC_EOL_CR)
    {
      StringCchPrintf(tchStatusBar[STATUS_EOLMODE], txtWidth, L"%sCR", g_mxStatusBarPrefix[STATUS_EOLMODE]);
    }
    else if (g_iEOLMode == SC_EOL_LF)
    {
      StringCchPrintf(tchStatusBar[STATUS_EOLMODE], txtWidth, L"%sLF", g_mxStatusBarPrefix[STATUS_EOLMODE]);
    }
    else {
      StringCchPrintf(tchStatusBar[STATUS_EOLMODE], txtWidth, L"%sCR+LF", g_mxStatusBarPrefix[STATUS_EOLMODE]);
    }
    s_iEOLMode = g_iEOLMode;
    bIsUpdateNeeded = true;
  }
  // ------------------------------------------------------

  static bool s_bIsOVR = -1;
  bool const bIsOVR = (bool)SendMessage(g_hwndEdit, SCI_GETOVERTYPE, 0, 0);
  if (s_bIsOVR != bIsOVR) {
    if (bIsOVR)
    {
      StringCchPrintf(tchStatusBar[STATUS_OVRMODE], txtWidth, L"%sOVR", g_mxStatusBarPrefix[STATUS_OVRMODE]);
    }
    else {
      StringCchPrintf(tchStatusBar[STATUS_OVRMODE], txtWidth, L"%sINS", g_mxStatusBarPrefix[STATUS_OVRMODE]);
    }
    s_bIsOVR = bIsOVR;
    bIsUpdateNeeded = true;
  }
  // ------------------------------------------------------

  static bool s_bUse2ndDefault = -1;
  bool const bUse2ndDefault = Style_GetUse2ndDefault();
  if (s_bUse2ndDefault != bUse2ndDefault) {
    if (bUse2ndDefault)
    {
      StringCchPrintf(tchStatusBar[STATUS_2ND_DEF], txtWidth, L"%s2ND", g_mxStatusBarPrefix[STATUS_2ND_DEF]);
    }
    else {
      StringCchPrintf(tchStatusBar[STATUS_2ND_DEF], txtWidth, L"%sSTD", g_mxStatusBarPrefix[STATUS_2ND_DEF]);
    }
    s_bUse2ndDefault = bUse2ndDefault;
    bIsUpdateNeeded = true;
  }
  // ------------------------------------------------------

  static WCHAR tchLexerName[MINI_BUFFER];

  static int s_iCurLexer = -1;
  static bool s_bIs2ndDefault = -1;
  int const iCurLexer = Style_GetCurrentLexerRID();
  if ((s_iCurLexer != iCurLexer) || (s_bIs2ndDefault != bUse2ndDefault)) {
    Style_GetCurrentLexerName(tchLexerName, MINI_BUFFER);
    StringCchPrintf(tchStatusBar[STATUS_LEXER], txtWidth, L"%s%s", g_mxStatusBarPrefix[STATUS_LEXER], tchLexerName);
    s_iCurLexer = iCurLexer;
    s_bIs2ndDefault = bUse2ndDefault;
    bIsUpdateNeeded = true;
  }
  // ------------------------------------------------------

  // ====  Statusbar widths  ====

  int g_vStatusbarSectionWidth[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

  _CalculateStatusbarSections(g_vStatusbarSectionWidth, tchStatusBar, &bIsUpdateNeeded);

  if (bIsUpdateNeeded) {
    int aStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_ZERO;
    int cnt = 0;
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
    else { aStatusbarSections[0] = -1; bShowStatusbar = false; }

    SendMessage(g_hwndStatus, SB_SETPARTS, (WPARAM)cnt, (LPARAM)aStatusbarSections);

    cnt = 0;
    for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
      int const id = g_vSBSOrder[i];
      if ((id >= 0) && (g_vStatusbarSectionWidth[id] >= 0)) {
        StatusSetText(g_hwndStatus, cnt++, tchStatusBar[id]);
      }
    }
    //InvalidateRect(g_hwndStatus,NULL,true);
  }
  // --------------------------------------------------------------------------

  // update Find/Replace dialog (if any)
  static WCHAR tchReplOccs[32] = { L'\0' };
  if (g_hwndDlgFindReplace) {
    if (iReplacedOccurrences > 0)
      StringCchPrintf(tchReplOccs, COUNTOF(tchReplOccs), L"%i", iReplacedOccurrences);
    else
      StringCchCopy(tchReplOccs, COUNTOF(tchReplOccs), L"--");

    FormatString(tchFRStatus, COUNTOF(tchFRStatus), IDS_FR_STATUS_FMT,
                 tchLn, tchLines, tchCol, tchSel, tchOcc, tchReplOccs, FR_Status[g_FindReplaceMatchFoundState]);

    SetWindowText(GetDlgItem(g_hwndDlgFindReplace, IDS_FR_STATUS_TEXT), tchFRStatus);
  }

}


//=============================================================================
//
//  UpdateLineNumberWidth()
//
//
void UpdateLineNumberWidth()
{
  if (bShowLineNumbers)
  {
    char chLines[32] = { '\0' };
    StringCchPrintfA(chLines, COUNTOF(chLines), "_%td", (size_t)SciCall_GetLineCount());

    int iLineMarginWidthNow = SciCall_GetMarginWidthN(MARGIN_SCI_LINENUM);
    int iLineMarginWidthFit = SciCall_TextWidth(STYLE_LINENUMBER, chLines);

    if (iLineMarginWidthNow != iLineMarginWidthFit) {
      SciCall_SetMarginWidthN(MARGIN_SCI_LINENUM, iLineMarginWidthFit);
    }
  }
  else {
    SciCall_SetMarginWidthN(MARGIN_SCI_LINENUM, 0);
  }
}


//=============================================================================
//
//  UpdateSettingsCmds()
//
//
void UpdateSettingsCmds()
{
    HMENU hmenu = GetSystemMenu(g_hwndMain, false);
    bool hasIniFile = (StringCchLenW(g_wchIniFile,COUNTOF(g_wchIniFile)) > 0 || StringCchLenW(g_wchIniFile2,COUNTOF(g_wchIniFile2)) > 0);
    CheckCmd(hmenu, IDM_VIEW_SAVESETTINGS, bSaveSettings && bEnableSaveSettings);
    EnableCmd(hmenu, IDM_VIEW_SAVESETTINGS, hasIniFile && bEnableSaveSettings);
    EnableCmd(hmenu, IDM_VIEW_SAVESETTINGSNOW, hasIniFile && bEnableSaveSettings);
}


//=============================================================================
//
//  UpdateUI()
//
void UpdateUI()
{
  struct SCNotification scn;
  scn.nmhdr.hwndFrom = g_hwndEdit;
  scn.nmhdr.idFrom = IDC_EDIT;
  scn.nmhdr.code = SCN_UPDATEUI;
  scn.updated = (SC_UPDATE_CONTENT | SC_UPDATE_NP3_INTERNAL_NOTIFY);
  SendMessage(g_hwndMain, WM_NOTIFY, IDC_EDIT, (LPARAM)&scn);
  //PostMessage(g_hwndMain, WM_NOTIFY, IDC_EDIT, (LPARAM)&scn);
}


//=============================================================================
//
//  BeginUndoAction()
//
//

static volatile LONG UndoActionToken = -1L;

int BeginUndoAction()
{
  if (InterlockedExchange(&UndoActionToken, UndoActionToken) >= 0L) { return -1; } // already active

  UndoRedoSelection_t sel = INIT_UNDOREDOSEL;
  sel.selMode_undo = (int)SendMessage(g_hwndEdit,SCI_GETSELECTIONMODE,0,0);

  switch (sel.selMode_undo)
  {
  case SC_SEL_RECTANGLE:
  case SC_SEL_THIN:
    sel.anchorPos_undo = (DocPos)SendMessage(g_hwndEdit, SCI_GETRECTANGULARSELECTIONANCHOR, 0, 0);
    sel.curPos_undo = (DocPos)SendMessage(g_hwndEdit, SCI_GETRECTANGULARSELECTIONCARET, 0, 0);
    if (!bDenyVirtualSpaceAccess) {
      sel.anchorVS_undo = (DocPos)SendMessage(g_hwndEdit, SCI_GETRECTANGULARSELECTIONANCHORVIRTUALSPACE, 0, 0);
      sel.curVS_undo = (DocPos)SendMessage(g_hwndEdit, SCI_GETRECTANGULARSELECTIONCARETVIRTUALSPACE, 0, 0);
    }
    break;

  case SC_SEL_LINES:
  case SC_SEL_STREAM:
  default:
    sel.anchorPos_undo = (DocPos)SendMessage(g_hwndEdit, SCI_GETANCHOR, 0, 0);
    sel.curPos_undo = (DocPos)SendMessage(g_hwndEdit, SCI_GETCURRENTPOS, 0, 0);
    break;
  }

  int token = UndoRedoActionMap(-1, &sel);
 
  if (token >= 0) {
    SciCall_BeginUndoAction();
    SciCall_AddUndoAction(token, 0);
  }

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
  if ((token >= 0) && (token == (int)InterlockedExchange(&UndoActionToken, UndoActionToken)))
  {
    UndoRedoSelection_t sel = INIT_UNDOREDOSEL;

    if (UndoRedoActionMap(token, &sel) >= 0) 
    {
      sel.selMode_redo = (int)SendMessage(g_hwndEdit, SCI_GETSELECTIONMODE, 0, 0);

      switch (sel.selMode_redo)
      {
      case SC_SEL_RECTANGLE:
      case SC_SEL_THIN:
        sel.anchorPos_redo = (DocPos)SendMessage(g_hwndEdit, SCI_GETRECTANGULARSELECTIONANCHOR, 0, 0);
        sel.curPos_redo = (DocPos)SendMessage(g_hwndEdit, SCI_GETRECTANGULARSELECTIONCARET, 0, 0);
        if (!bDenyVirtualSpaceAccess) {
          sel.anchorVS_redo = (DocPos)SendMessage(g_hwndEdit, SCI_GETRECTANGULARSELECTIONANCHORVIRTUALSPACE, 0, 0);
        }
        break;

      case SC_SEL_LINES:
      case SC_SEL_STREAM:
      default:
        sel.anchorPos_redo = (DocPos)SendMessage(g_hwndEdit, SCI_GETANCHOR, 0, 0);
        sel.curPos_redo = (DocPos)SendMessage(g_hwndEdit, SCI_GETCURRENTPOS, 0, 0);
        break;
      }
    }
    
    UndoRedoActionMap(token,&sel); // set with redo action filled

    SciCall_EndUndoAction();
    
    InterlockedExchange(&UndoActionToken, -1L);
  }
}


//=============================================================================
//
//  RestoreAction()
//
//
void RestoreAction(int token, DoAction doAct)
{
  UndoRedoSelection_t sel = INIT_UNDOREDOSEL;

  if (UndoRedoActionMap(token, &sel) >= 0)
  {
    // we are inside undo/redo transaction, so do delayed PostMessage() instead of SendMessage()
  #define ISSUE_MESSAGE PostMessage

    DocPos const _anchorPos = (doAct == UNDO ? sel.anchorPos_undo : sel.anchorPos_redo);
    DocPos const _curPos = (doAct == UNDO ? sel.curPos_undo : sel.curPos_redo);

    // Ensure that the first and last lines of a selection are always unfolded
    // This needs to be done _before_ the SCI_SETSEL message
    DocLn const anchorPosLine = SciCall_LineFromPosition(_anchorPos);
    DocLn const currPosLine = SciCall_LineFromPosition(_curPos);
    ISSUE_MESSAGE(g_hwndEdit, SCI_ENSUREVISIBLE, anchorPosLine, 0);
    if (anchorPosLine != currPosLine) { ISSUE_MESSAGE(g_hwndEdit, SCI_ENSUREVISIBLE, currPosLine, 0); }


    int const selectionMode = (doAct == UNDO ? sel.selMode_undo : sel.selMode_redo);
    ISSUE_MESSAGE(g_hwndEdit, SCI_SETSELECTIONMODE, (WPARAM)selectionMode, 0);

    // independent from selection mode
    ISSUE_MESSAGE(g_hwndEdit, SCI_SETANCHOR, (WPARAM)_anchorPos, 0);
    ISSUE_MESSAGE(g_hwndEdit, SCI_SETCURRENTPOS, (WPARAM)_curPos, 0);

    switch (selectionMode)
    {
    case SC_SEL_RECTANGLE: 
      ISSUE_MESSAGE(g_hwndEdit, SCI_SETRECTANGULARSELECTIONANCHOR, (WPARAM)_anchorPos, 0);
      ISSUE_MESSAGE(g_hwndEdit, SCI_SETRECTANGULARSELECTIONCARET, (WPARAM)_curPos, 0);
      // fall-through

    case SC_SEL_THIN:
      {
        DocPos const anchorVS = (doAct == UNDO ? sel.anchorVS_undo : sel.anchorVS_redo);
        DocPos const currVS = (doAct == UNDO ? sel.curVS_undo : sel.curVS_redo);
        if ((anchorVS != 0) || (currVS != 0)) {
          ISSUE_MESSAGE(g_hwndEdit, SCI_SETRECTANGULARSELECTIONANCHORVIRTUALSPACE, (WPARAM)anchorVS, 0);
          ISSUE_MESSAGE(g_hwndEdit, SCI_SETRECTANGULARSELECTIONCARETVIRTUALSPACE, (WPARAM)currVS, 0);
        }
      }
      break;

    case SC_SEL_LINES:
    case SC_SEL_STREAM:
    default:
      // nothing to do here
      break;
    }
    ISSUE_MESSAGE(g_hwndEdit, SCI_SCROLLCARET, 0, 0);
    ISSUE_MESSAGE(g_hwndEdit, SCI_CHOOSECARETX, 0, 0);
    ISSUE_MESSAGE(g_hwndEdit, SCI_CANCEL, 0, 0);

  #undef ISSUE_MASSAGE
  }
}


//=============================================================================
//
//  UndoSelectionMap()
//
//
int UndoRedoActionMap(int token, UndoRedoSelection_t* selection)
{
  if (UndoRedoSelectionUTArray == NULL) { return -1L; }

  static unsigned int uiTokenCnt = 0U;

  // indexing is unsigned
  unsigned int utoken = (token >= 0) ? (unsigned int)token : 0U;

  if (selection == NULL) {
    // reset / clear
    int const curToken = InterlockedExchange(&UndoActionToken, UndoActionToken);
    if (curToken >= 0) { EndUndoAction(curToken); }
    SciCall_SetUndoCollection(false);
    SciCall_EmptyUndoBuffer();
    utarray_clear(UndoRedoSelectionUTArray);
    utarray_init(UndoRedoSelectionUTArray, &UndoRedoSelection_icd);
    uiTokenCnt = 0U;
    SciCall_SetUndoCollection(true);
    InterlockedExchange(&UndoActionToken, -1L);
    return -1;
  }

  if (!SciCall_GetUndoCollection()) { return -1; }

  // get or set map item request ?
  if ((token >= 0) && (utoken < uiTokenCnt)) 
  {
    if (selection->anchorPos_undo < 0) {
      // this is a get request
      *selection = *(UndoRedoSelection_t*)utarray_eltptr(UndoRedoSelectionUTArray, utoken);
    }
    else {
      // this is a set request (fill redo pos)
      utarray_insert(UndoRedoSelectionUTArray, (void*)selection, utoken);
    }
    // don't clear map item here (token used in redo/undo again)
  }
  else if (token < 0) {
    // set map new item request
    token = (int)uiTokenCnt;
    utarray_insert(UndoRedoSelectionUTArray, (void*)selection, uiTokenCnt);
    uiTokenCnt = (uiTokenCnt < (unsigned int)INT_MAX) ? (uiTokenCnt + 1U) : 0U;  // round robin next
  }
  return token;
}


//=============================================================================
//
//  FileIO()
//
//
bool FileIO(bool fLoad,LPCWSTR pszFileName,bool bSkipUnicodeDetect,bool bSkipANSICPDetection,
            int *ienc,int *ieol,
            bool *pbUnicodeErr,bool *pbFileTooBig, bool* pbUnknownExt,
            bool *pbCancelDataLoss,bool bSaveCopy)
{
  WCHAR tch[MAX_PATH+40];
  bool fSuccess;
  DWORD dwFileAttributes;

  FormatString(tch,COUNTOF(tch),(fLoad) ? IDS_LOADFILE : IDS_SAVEFILE, PathFindFileName(pszFileName));

  BeginWaitCursor(tch);

  if (fLoad) {
    fSuccess = EditLoadFile(g_hwndEdit,pszFileName,bSkipUnicodeDetect,bSkipANSICPDetection,ienc,ieol,pbUnicodeErr,pbFileTooBig,pbUnknownExt);
  }
  else {
    int idx;
    if (MRU_FindFile(g_pFileMRU,pszFileName,&idx)) {
      g_pFileMRU->iEncoding[idx] = *ienc;
      g_pFileMRU->iCaretPos[idx] = (bPreserveCaretPos ? SciCall_GetCurrentPos() : 0);
      WCHAR wchBookMarks[MRU_BMRK_SIZE] = { L'\0' };
      EditGetBookmarkList(g_hwndEdit, wchBookMarks, COUNTOF(wchBookMarks));
      if (g_pFileMRU->pszBookMarks[idx])
        LocalFree(g_pFileMRU->pszBookMarks[idx]);
      g_pFileMRU->pszBookMarks[idx] = StrDup(wchBookMarks);
    }
    fSuccess = EditSaveFile(g_hwndEdit,pszFileName,*ienc,pbCancelDataLoss,bSaveCopy);
  }

  dwFileAttributes = GetFileAttributes(pszFileName);
  g_bFileReadOnly = (dwFileAttributes != INVALID_FILE_ATTRIBUTES && dwFileAttributes & FILE_ATTRIBUTE_READONLY);

  EndWaitCursor();

  return(fSuccess);
}


//=============================================================================
//
//  FileLoad()
//
//
bool FileLoad(bool bDontSave, bool bNew, bool bReload, bool bSkipUnicodeDetect, bool bSkipANSICPDetection, LPCWSTR lpszFile)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  WCHAR szFileName[MAX_PATH] = { L'\0' };
  bool bUnicodeErr = false;
  bool bFileTooBig = false;
  bool bUnknownExt = false;
  bool fSuccess;
  int fileEncoding = CPI_ANSI_DEFAULT;

  if (bNew || bReload) {
    if (EditToggleView(g_hwndEdit, false)) {
      EditToggleView(g_hwndEdit, true);
    }
  }

  if (!bDontSave)
  {
    if (!FileSave(false,true,false,false))
      return false;
  }

  if (!bReload) { ResetEncryption(); }

  if (bNew) {
    StringCchCopy(g_wchCurFile,COUNTOF(g_wchCurFile),L"");
    SetDlgItemText(g_hwndMain,IDC_FILENAME,g_wchCurFile);
    SetDlgItemInt(g_hwndMain,IDC_REUSELOCK,GetTickCount(),false);
    if (!fKeepTitleExcerpt)
      StringCchCopy(szTitleExcerpt,COUNTOF(szTitleExcerpt),L"");
    FileVars_Init(NULL,0,&fvCurFile);
    EditSetNewText(g_hwndEdit,"",0);
    Style_SetLexer(g_hwndEdit,NULL);

    g_iEOLMode = iLineEndings[g_iDefaultEOLMode];
    SendMessage(g_hwndEdit,SCI_SETEOLMODE,iLineEndings[g_iDefaultEOLMode],0);
    Encoding_Current(g_iDefaultNewFileEncoding);
    Encoding_HasChanged(g_iDefaultNewFileEncoding);
    EditSetNewText(g_hwndEdit,"",0);

    g_bFileReadOnly = false;
    _SetDocumentModified(false);

    UpdateToolbar();
    UpdateStatusbar(false);
    UpdateLineNumberWidth();

    // Terminate file watching
    if (g_bResetFileWatching) {
      if (g_bChasingDocTail) {
        SendMessage(g_hwndMain, WM_COMMAND, MAKELONG(IDM_VIEW_CHASING_DOCTAIL, 1), 0);
      }
      g_iFileWatchingMode = 0;
    }
    InstallFileWatching(NULL);
    bEnableSaveSettings = true;
    UpdateSettingsCmds();
    return true;
  }

  if (!lpszFile || lstrlen(lpszFile) == 0) {
    if (!OpenFileDlg(g_hwndMain,tch,COUNTOF(tch),NULL))
      return false;
  }
  else
    StringCchCopy(tch,COUNTOF(tch),lpszFile);

  ExpandEnvironmentStringsEx(tch,COUNTOF(tch));

  if (PathIsRelative(tch)) {
    StringCchCopyN(szFileName,COUNTOF(szFileName),g_wchWorkingDirectory,COUNTOF(g_wchWorkingDirectory));
    PathCchAppend(szFileName,COUNTOF(szFileName),tch);
    if (!PathFileExists(szFileName)) {
      WCHAR wchFullPath[MAX_PATH] = { L'\0' };
      if (SearchPath(NULL,tch,NULL,COUNTOF(wchFullPath),wchFullPath,NULL)) {
        StringCchCopy(szFileName,COUNTOF(szFileName),wchFullPath);
      }
    }
  }
  else
    StringCchCopy(szFileName,COUNTOF(szFileName),tch);

  NormalizePathEx(szFileName,COUNTOF(szFileName));

  if (PathIsLnkFile(szFileName))
    PathGetLnkPath(szFileName,szFileName,COUNTOF(szFileName));

  // change current directory to prevent directory lock on another path
  WCHAR szFolder[MAX_PATH+2];
  if (SUCCEEDED(StringCchCopy(szFolder,COUNTOF(szFolder),tch))) {
    if (SUCCEEDED(PathCchRemoveFileSpec(szFolder,COUNTOF(szFolder)))) {
      SetCurrentDirectory(szFolder);
    }
  }
 
  // Ask to create a new file...
  if (!bReload && !PathFileExists(szFileName))
  {
    if (flagQuietCreate || MsgBox(MBYESNO,IDS_ASK_CREATE,szFileName) == IDYES) {
      HANDLE hFile = CreateFile(szFileName,
                      GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,
                      NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
      dwLastIOError = GetLastError();
      fSuccess = (hFile != INVALID_HANDLE_VALUE);
      if (fSuccess) {
        FileVars_Init(NULL,0,&fvCurFile);
        EditSetNewText(g_hwndEdit,"",0);
        Style_SetLexer(g_hwndEdit,NULL);
        g_iEOLMode = iLineEndings[g_iDefaultEOLMode];
        SendMessage(g_hwndEdit,SCI_SETEOLMODE,iLineEndings[g_iDefaultEOLMode],0);
        if (Encoding_SrcCmdLn(CPI_GET) != CPI_NONE) {
          fileEncoding = Encoding_SrcCmdLn(CPI_GET);
          Encoding_Current(fileEncoding);
          Encoding_HasChanged(fileEncoding);
        }
        else {
          Encoding_Current(g_iDefaultNewFileEncoding);
          Encoding_HasChanged(g_iDefaultNewFileEncoding);
        }
        g_bFileReadOnly = false;
        EditSetNewText(g_hwndEdit,"",0);
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
    if (!bReload && MRU_FindFile(g_pFileMRU,szFileName,&idx)) {
      fileEncoding = g_pFileMRU->iEncoding[idx];
      if (fileEncoding > 0)
        Encoding_SrcCmdLn(Encoding_MapUnicode(fileEncoding));
    }
    else
      fileEncoding = Encoding_Current(CPI_GET);

    fSuccess = FileIO(true,szFileName,bSkipUnicodeDetect,bSkipANSICPDetection,&fileEncoding,&g_iEOLMode,&bUnicodeErr,&bFileTooBig,&bUnknownExt,NULL,false);
    if (fSuccess)
      Encoding_Current(fileEncoding); // load may change encoding
  }
  if (fSuccess) {
    StringCchCopy(g_wchCurFile,COUNTOF(g_wchCurFile),szFileName);
    SetDlgItemText(g_hwndMain,IDC_FILENAME,g_wchCurFile);
    SetDlgItemInt(g_hwndMain,IDC_REUSELOCK,GetTickCount(),false);

    if (!fKeepTitleExcerpt)
      StringCchCopy(szTitleExcerpt,COUNTOF(szTitleExcerpt),L"");

    if (!flagLexerSpecified) // flag will be cleared
      Style_SetLexerFromFile(g_hwndEdit,g_wchCurFile);

    SendMessage(g_hwndEdit,SCI_SETEOLMODE,g_iEOLMode,0);
    fileEncoding = Encoding_Current(CPI_GET);
    Encoding_HasChanged(fileEncoding);
    int idx = 0;
    DocPos iCaretPos = 0;
    LPCWSTR pszBookMarks = L"";
    if (!bReload && MRU_FindFile(g_pFileMRU,szFileName,&idx)) {
      iCaretPos = g_pFileMRU->iCaretPos[idx];
      pszBookMarks = g_pFileMRU->pszBookMarks[idx];
    }
    MRU_AddFile(g_pFileMRU,szFileName,flagRelativeFileMRU,flagPortableMyDocs,fileEncoding,iCaretPos,pszBookMarks);
   
    EditSetBookmarkList(g_hwndEdit, pszBookMarks);
    SetFindPattern((g_pMRUfind ? g_pMRUfind->pszItems[0] : L""));

    if (flagUseSystemMRU == 2)
      SHAddToRecentDocs(SHARD_PATHW,szFileName);

    // Install watching of the current file
    if (!bReload && g_bResetFileWatching) {
      if (g_bChasingDocTail) {
        SendMessage(g_hwndMain, WM_COMMAND, MAKELONG(IDM_VIEW_CHASING_DOCTAIL, 1), 0);
      }
      g_iFileWatchingMode = 0;
    }
    InstallFileWatching(g_wchCurFile);

    // the .LOG feature ...
    if (SciCall_GetTextLength() >= 4) {
      char tchLog[5] = { '\0' };
      SendMessage(g_hwndEdit,SCI_GETTEXT,5,(LPARAM)tchLog);
      if (StringCchCompareXA(tchLog,".LOG") == 0) {
        EditJumpTo(g_hwndEdit,-1,0);
        int token = BeginUndoAction();
        SendMessage(g_hwndEdit,SCI_NEWLINE,0,0);
        SendMessage(g_hwndMain,WM_COMMAND,MAKELONG(IDM_EDIT_INSERT_SHORTDATE,1),0);
        EditJumpTo(g_hwndEdit,-1,0);
        SendMessage(g_hwndEdit,SCI_NEWLINE,0,0);
        EndUndoAction(token);
        SendMessage(g_hwndEdit, SCI_DOCUMENTEND, 0, 0);
        EditEnsureSelectionVisible(g_hwndEdit);
      }
      // set historic caret pos
      else if (iCaretPos > 0) 
      {
        SendMessage(g_hwndEdit, SCI_GOTOPOS, (WPARAM)iCaretPos, 0);
        // adjust view
        const DocPos iCurPos = SciCall_GetCurrentPos();
        EditJumpTo(g_hwndEdit, SciCall_LineFromPosition(iCurPos) + 1, SciCall_GetColumn(iCurPos) + 1);
      }
    }

    //bReadOnly = false;
    _SetDocumentModified(false);
    UpdateToolbar();
    UpdateStatusbar(false);
    UpdateLineNumberWidth();
    UpdateVisibleUrlHotspot(0);

    // consistent settings file handling (if loaded in editor)
    bEnableSaveSettings = (StringCchCompareINW(g_wchCurFile, COUNTOF(g_wchCurFile), g_wchIniFile, COUNTOF(g_wchIniFile)) == 0) ? false : true;
    UpdateSettingsCmds();

    // Show warning: Unicode file loaded as ANSI
    if (bUnicodeErr)
      MsgBox(MBWARN,IDS_ERR_UNICODE);
  }

  else if (!(bFileTooBig || bUnknownExt))
    MsgBox(MBWARN,IDS_ERR_LOADFILE,szFileName);

  return(fSuccess);
}



//=============================================================================
//
//  FileRevert()
//
//
bool FileRevert(LPCWSTR szFileName) 
{
  if (StringCchLen(szFileName, MAX_PATH) != 0) {

    const DocPos iCurPos = SciCall_IsSelectionRectangle() ? SciCall_GetRectangularSelectionCaret() : SciCall_GetCurrentPos();
    const DocPos iAnchorPos = SciCall_IsSelectionRectangle() ? SciCall_GetRectangularSelectionAnchor() : SciCall_GetAnchor();
    //const int vSpcCaretPos = SciCall_IsSelectionRectangle() ? SciCall_GetRectangularSelectionCaretVirtualSpace() : -1;
    //const int vSpcAnchorPos = SciCall_IsSelectionRectangle() ? SciCall_GetRectangularSelectionAnchorVirtualSpace() : -1;

    const DocLn iCurrLine = SciCall_LineFromPosition(iCurPos);
    const DocPos iCurColumn = SciCall_GetColumn(iCurPos);
    const DocLn iVisTopLine = SciCall_GetFirstVisibleLine();
    const DocLn iDocTopLine = SciCall_DocLineFromVisible(iVisTopLine);
    const int   iXOffset = SciCall_GetXoffset();
    const bool bIsTail = (iCurPos == iAnchorPos) && (iCurrLine >= (SciCall_GetLineCount() - 1));

    Encoding_SrcWeak(Encoding_Current(CPI_GET));

    WCHAR tchFileName2[MAX_PATH] = { L'\0' };
    StringCchCopyW(tchFileName2,COUNTOF(tchFileName2),szFileName);

    if (FileLoad(true,false,true,false,true,tchFileName2))
    {
      if (bIsTail && g_iFileWatchingMode == 2) {
        SendMessage(g_hwndEdit, SCI_DOCUMENTEND, 0, 0);
        EditEnsureSelectionVisible(g_hwndEdit);
      }
      else if (SciCall_GetTextLength() >= 4) {
        char tch[5] = { '\0' };
        
        SciCall_GetText(5, tch);
        if (StringCchCompareXA(tch,".LOG") != 0) {
          SciCall_ClearSelections();
          //~EditSelectEx(g_hwndEdit, iAnchorPos, iCurPos, vSpcAnchorPos, vSpcCaretPos);
          EditJumpTo(g_hwndEdit, iCurrLine+1, iCurColumn+1);
          SciCall_EnsureVisible(iDocTopLine);
          const DocLn iNewTopLine = SciCall_GetFirstVisibleLine();
          SciCall_LineScroll(0,iVisTopLine - iNewTopLine);
          SciCall_SetXoffset(iXOffset);
        }
      }
      return true;
    }
  }
  return false;
}


//=============================================================================
//
//  FileSave()
//
//
bool FileSave(bool bSaveAlways,bool bAsk,bool bSaveAs,bool bSaveCopy)
{
  WCHAR tchFile[MAX_PATH] = { L'\0' };
  WCHAR tchBase[MAX_PATH] = { L'\0' };
  bool fSuccess = false;
  bool bCancelDataLoss = false;

  bool bIsEmptyNewFile = false;
  if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile)) == 0) {
    const DocPos cchText = SciCall_GetTextLength();
    if (cchText == 0)
      bIsEmptyNewFile = true;
    else if (cchText < 1023) {
      char tchText[1024];
      SendMessage(g_hwndEdit,SCI_GETTEXT,(WPARAM)1023,(LPARAM)tchText);
      StrTrimA(tchText," \t\n\r");
      if (lstrlenA(tchText) == 0)
        bIsEmptyNewFile = true;
    }
  }

  if (!bSaveAlways && (!IsDocumentModified && !Encoding_HasChanged(CPI_GET) || bIsEmptyNewFile) && !bSaveAs) {
    int idx;
    if (MRU_FindFile(g_pFileMRU,g_wchCurFile,&idx)) {
      g_pFileMRU->iEncoding[idx] = Encoding_Current(CPI_GET);
      g_pFileMRU->iCaretPos[idx] = (bPreserveCaretPos) ? SciCall_GetCurrentPos() : 0;
      WCHAR wchBookMarks[MRU_BMRK_SIZE] = { L'\0' };
      EditGetBookmarkList(g_hwndEdit, wchBookMarks, COUNTOF(wchBookMarks));
      if (g_pFileMRU->pszBookMarks[idx])
        LocalFree(g_pFileMRU->pszBookMarks[idx]);
      g_pFileMRU->pszBookMarks[idx] = StrDup(wchBookMarks);
    }
    return true;
  }

  if (bAsk)
  {
    // File or "Untitled" ...
    WCHAR tch[MAX_PATH] = { L'\0' };
    if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile)))
      StringCchCopy(tch,COUNTOF(tch),g_wchCurFile);
    else
      GetString(IDS_UNTITLED,tch,COUNTOF(tch));

    switch (MsgBox(MBYESNOCANCEL,IDS_ASK_SAVE,tch)) {
      case IDCANCEL:
        return false;
      case IDNO:
        return true;
    }
  }

  // Read only...
  if (!bSaveAs && !bSaveCopy && StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile)))
  {
    DWORD dwFileAttributes = GetFileAttributes(g_wchCurFile);
    if (dwFileAttributes != INVALID_FILE_ATTRIBUTES)
      g_bFileReadOnly = (dwFileAttributes & FILE_ATTRIBUTE_READONLY);
    if (g_bFileReadOnly) {
      UpdateToolbar();
      if (MsgBox(MBYESNOWARN,IDS_READONLY_SAVE,g_wchCurFile) == IDYES)
        bSaveAs = true;
      else
        return false;
    }
  }

  // Save As...
  if (bSaveAs || bSaveCopy || StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile)) == 0)
  {
    WCHAR tchInitialDir[MAX_PATH] = { L'\0' };
    if (bSaveCopy && StringCchLenW(g_tchLastSaveCopyDir,COUNTOF(g_tchLastSaveCopyDir))) {
      StringCchCopy(tchInitialDir,COUNTOF(tchInitialDir),g_tchLastSaveCopyDir);
      StringCchCopy(tchFile,COUNTOF(tchFile),g_tchLastSaveCopyDir);
      PathCchAppend(tchFile,COUNTOF(tchFile),PathFindFileName(g_wchCurFile));
    }
    else
      StringCchCopy(tchFile,COUNTOF(tchFile),g_wchCurFile);

    if (SaveFileDlg(g_hwndMain,tchFile,COUNTOF(tchFile),tchInitialDir))
    {
      int fileEncoding = Encoding_Current(CPI_GET);
      fSuccess = FileIO(false, tchFile, false, true, &fileEncoding, &g_iEOLMode, NULL, NULL, NULL, &bCancelDataLoss, bSaveCopy);
      //~if (fSuccess) Encoding_Current(fileEncoding); // save should not change encoding
      if (fSuccess)
      {
        if (!bSaveCopy)
        {
          StringCchCopy(g_wchCurFile,COUNTOF(g_wchCurFile),tchFile);
          SetDlgItemText(g_hwndMain,IDC_FILENAME,g_wchCurFile);
          SetDlgItemInt(g_hwndMain,IDC_REUSELOCK,GetTickCount(),false);
          if (!fKeepTitleExcerpt)
            StringCchCopy(szTitleExcerpt,COUNTOF(szTitleExcerpt),L"");
          Style_SetLexerFromFile(g_hwndEdit,g_wchCurFile);
          UpdateToolbar();
          UpdateStatusbar(false);
          UpdateLineNumberWidth();
        }
        else {
          StringCchCopy(g_tchLastSaveCopyDir,COUNTOF(g_tchLastSaveCopyDir),tchFile);
          PathRemoveFileSpec(g_tchLastSaveCopyDir);
        }
      }
    }
    else
      return false;
  }
  else {
    int fileEncoding = Encoding_Current(CPI_GET);
    fSuccess = FileIO(false, g_wchCurFile, false, true, &fileEncoding, &g_iEOLMode, NULL, NULL, NULL, &bCancelDataLoss, false);
    //~if (fSuccess) Encoding_Current(fileEncoding); // save should not change encoding
  }

  if (fSuccess)
  {
    if (!bSaveCopy)
    {
      int iCurrEnc = Encoding_Current(CPI_GET);
      Encoding_HasChanged(iCurrEnc);
      const DocPos iCaretPos = SciCall_GetCurrentPos();
      WCHAR wchBookMarks[MRU_BMRK_SIZE] = { L'\0' };
      EditGetBookmarkList(g_hwndEdit, wchBookMarks, COUNTOF(wchBookMarks));
      MRU_AddFile(g_pFileMRU,g_wchCurFile,flagRelativeFileMRU,flagPortableMyDocs,iCurrEnc,iCaretPos,wchBookMarks);
      if (flagUseSystemMRU == 2)
        SHAddToRecentDocs(SHARD_PATHW,g_wchCurFile);

      _SetDocumentModified(false);
      // Install watching of the current file
      if (bSaveAs && g_bResetFileWatching) {
        if (g_bChasingDocTail) {
          SendMessage(g_hwndMain, WM_COMMAND, MAKELONG(IDM_VIEW_CHASING_DOCTAIL, 1), 0);
        }
        g_iFileWatchingMode = 0;
      }
      InstallFileWatching(g_wchCurFile);
    }
  }
  else if (!bCancelDataLoss)
  {
    if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile)) > 0) {
      StringCchCopy(tchFile,COUNTOF(tchFile),g_wchCurFile);
      StringCchCopy(tchBase,COUNTOF(tchBase),g_wchCurFile);
      PathStripPath(tchBase);
    }
    if (!flagIsElevated && dwLastIOError == ERROR_ACCESS_DENIED) {
      if (IDYES == MsgBox(MBYESNOWARN,IDS_ERR_ACCESSDENIED,tchFile)) {
        WCHAR lpTempPathBuffer[MAX_PATH];
        WCHAR szTempFileName[MAX_PATH];

        if (GetTempPath(MAX_PATH,lpTempPathBuffer) &&
            GetTempFileName(lpTempPathBuffer,TEXT("NP3"),0,szTempFileName)) {
          int fileEncoding = Encoding_Current(CPI_GET);
          if (FileIO(false,szTempFileName,false,true,&fileEncoding,&g_iEOLMode,NULL,NULL,NULL,&bCancelDataLoss,true)) {
            //~Encoding_Current(fileEncoding); // save should not change encoding
            WCHAR szArguments[2048] = { L'\0' };
            LPWSTR lpCmdLine = GetCommandLine();
            int wlen = lstrlen(lpCmdLine) + 2;
            LPWSTR lpExe = LocalAlloc(LPTR,sizeof(WCHAR)*wlen);
            LPWSTR lpArgs = LocalAlloc(LPTR,sizeof(WCHAR)*wlen);
            ExtractFirstArgument(lpCmdLine,lpExe,lpArgs,wlen);
            // remove relaunch elevated, we are doing this here already
            lpArgs = StrCutI(lpArgs,L"/u ");
            lpArgs = StrCutI(lpArgs,L"-u ");
            WININFO wi = GetMyWindowPlacement(g_hwndMain,NULL);
            StringCchPrintf(szArguments,COUNTOF(szArguments),
              L"/pos %i,%i,%i,%i,%i /tmpfbuf=\"%s\" %s",wi.x,wi.y,wi.cx,wi.cy,wi.max,szTempFileName,lpArgs);
            if (StringCchLenW(tchFile,COUNTOF(tchFile))) {
              if (!StrStrI(szArguments,tchBase)) {
                StringCchPrintf(szArguments,COUNTOF(szArguments),L"%s \"%s\"",szArguments,tchFile);
              }
            }
            flagRelaunchElevated = 1;
            if (RelaunchElevated(szArguments)) {
              LocalFree(lpExe);
              LocalFree(lpArgs);
              // set no change and quit
              Encoding_HasChanged(Encoding_Current(CPI_GET));
              _SetDocumentModified(false);
              PostMessage(g_hwndMain,WM_CLOSE,0,0);
            }
            else {
              if (PathFileExists(szTempFileName)) {
                DeleteFile(szTempFileName);
              }
              UpdateToolbar();
              MsgBox(MBWARN,IDS_ERR_SAVEFILE,tchFile);
            }
          }
        }
      }
    }
    else {
      UpdateToolbar();
      MsgBox(MBWARN,IDS_ERR_SAVEFILE,tchFile);
    }
  }
  return(fSuccess);
}


//=============================================================================
//
//  OpenFileDlg()
//
//
bool OpenFileDlg(HWND hwnd,LPWSTR lpstrFile,int cchFile,LPCWSTR lpstrInitialDir)
{
  OPENFILENAME ofn;
  WCHAR szFile[MAX_PATH] = { L'\0' };
  WCHAR szFilter[NUMLEXERS * AVG_NUM_OF_STYLES_PER_LEXER * 100];
  WCHAR tchInitialDir[MAX_PATH] = { L'\0' };

  Style_GetOpenDlgFilterStr(szFilter,COUNTOF(szFilter));

  if (!lpstrInitialDir) {
    if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
      StringCchCopy(tchInitialDir,COUNTOF(tchInitialDir),g_wchCurFile);
      PathRemoveFileSpec(tchInitialDir);
    }
    else if (StringCchLenW(g_tchDefaultDir,COUNTOF(g_tchDefaultDir))) {
      ExpandEnvironmentStrings(g_tchDefaultDir,tchInitialDir,COUNTOF(tchInitialDir));
      if (PathIsRelative(tchInitialDir)) {
        WCHAR tchModule[MAX_PATH] = { L'\0' };
        GetModuleFileName(NULL,tchModule,COUNTOF(tchModule));
        PathRemoveFileSpec(tchModule);
        PathCchAppend(tchModule,COUNTOF(tchModule),tchInitialDir);
        PathCchCanonicalize(tchInitialDir,COUNTOF(tchInitialDir),tchModule);
      }
    }
    else
      StringCchCopy(tchInitialDir,COUNTOF(tchInitialDir),g_wchWorkingDirectory);
  }

  ZeroMemory(&ofn,sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFilter = szFilter;
  ofn.lpstrFile = szFile;
  ofn.lpstrInitialDir = (lpstrInitialDir) ? lpstrInitialDir : tchInitialDir;
  ofn.nMaxFile = COUNTOF(szFile);
  ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | /* OFN_NOCHANGEDIR |*/
              OFN_DONTADDTORECENT | OFN_PATHMUSTEXIST |
              OFN_SHAREAWARE /*| OFN_NODEREFERENCELINKS*/;
  ofn.lpstrDefExt = (StringCchLenW(g_tchDefaultExtension,COUNTOF(g_tchDefaultExtension))) ? g_tchDefaultExtension : NULL;

  if (GetOpenFileName(&ofn)) {
    StringCchCopyN(lpstrFile,cchFile,szFile,COUNTOF(szFile));
    return true;
  }

  else
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
  WCHAR szFilter[NUMLEXERS * AVG_NUM_OF_STYLES_PER_LEXER * 100] = { L'\0' };
  WCHAR tchInitialDir[MAX_PATH] = { L'\0' };

  StringCchCopy(szNewFile,COUNTOF(szNewFile),lpstrFile);
  Style_GetOpenDlgFilterStr(szFilter,COUNTOF(szFilter));

  if (lstrlen(lpstrInitialDir))
    StringCchCopy(tchInitialDir,COUNTOF(tchInitialDir),lpstrInitialDir);
  else if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
    StringCchCopy(tchInitialDir,COUNTOF(tchInitialDir),g_wchCurFile);
    PathRemoveFileSpec(tchInitialDir);
  }
  else if (StringCchLenW(g_tchDefaultDir,COUNTOF(g_tchDefaultDir))) {
    ExpandEnvironmentStrings(g_tchDefaultDir,tchInitialDir,COUNTOF(tchInitialDir));
    if (PathIsRelative(tchInitialDir)) {
      WCHAR tchModule[MAX_PATH] = { L'\0' };
      GetModuleFileName(NULL,tchModule,COUNTOF(tchModule));
      PathRemoveFileSpec(tchModule);
      PathCchAppend(tchModule,COUNTOF(tchModule),tchInitialDir);
      PathCchCanonicalize(tchInitialDir,COUNTOF(tchInitialDir),tchModule);
    }
  }
  else
    StringCchCopy(tchInitialDir,COUNTOF(tchInitialDir),g_wchWorkingDirectory);

  ZeroMemory(&ofn,sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFilter = szFilter;
  ofn.lpstrFile = szNewFile;
  ofn.lpstrInitialDir = tchInitialDir;
  ofn.nMaxFile = MAX_PATH;
  ofn.Flags = OFN_HIDEREADONLY /*| OFN_NOCHANGEDIR*/ |
            /*OFN_NODEREFERENCELINKS |*/ OFN_OVERWRITEPROMPT |
            OFN_DONTADDTORECENT | OFN_PATHMUSTEXIST;
  ofn.lpstrDefExt = (StringCchLenW(g_tchDefaultExtension,COUNTOF(g_tchDefaultExtension))) ? g_tchDefaultExtension : NULL;

  if (GetSaveFileName(&ofn)) {
    StringCchCopyN(lpstrFile,cchFile,szNewFile,COUNTOF(szNewFile));
    return true;
  }

  else
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

    if (StringCchCompareINW(szClassName,COUNTOF(szClassName),wchWndClass,COUNTOF(wchWndClass)) == 0) {

      DWORD dwReuseLock = GetDlgItemInt(hwnd,IDC_REUSELOCK,NULL,FALSE);
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

    if (StringCchCompareINW(szClassName,COUNTOF(szClassName),wchWndClass,COUNTOF(wchWndClass)) == 0) {

      DWORD dwReuseLock = GetDlgItemInt(hwnd,IDC_REUSELOCK,NULL,false);
      if (GetTickCount() - dwReuseLock >= REUSEWINDOWLOCKTIMEOUT) {

        WCHAR tchFileName[MAX_PATH] = { L'\0' };

        if (IsWindowEnabled(hwnd))
          bContinue = FALSE;

        GetDlgItemText(hwnd,IDC_FILENAME,tchFileName,COUNTOF(tchFileName));
        if (StringCchCompareIN(tchFileName,COUNTOF(tchFileName),lpFileArg,-1) == 0)
          *(HWND*)lParam = hwnd;
        else
          bContinue = TRUE;
      }
    }
  return bContinue;
}

bool ActivatePrevInst()
{
  HWND hwnd = NULL;
  COPYDATASTRUCT cds;

  if ((flagNoReuseWindow && !flagSingleFileInstance) || flagStartAsTrayIcon || flagNewFromClipboard || flagPasteBoard)
    return(false);

  if (flagSingleFileInstance && lpFileArg) {

    // Search working directory from second instance, first!
    // lpFileArg is at least MAX_PATH+4 WCHARS
    WCHAR tchTmp[FILE_ARG_BUF] = { L'\0' };

    ExpandEnvironmentStringsEx(lpFileArg,(DWORD)SizeOfMem(lpFileArg)/sizeof(WCHAR));

    if (PathIsRelative(lpFileArg)) {
      StringCchCopyN(tchTmp,COUNTOF(tchTmp),g_wchWorkingDirectory,COUNTOF(g_wchWorkingDirectory));
      PathCchAppend(tchTmp,COUNTOF(tchTmp),lpFileArg);
      if (PathFileExists(tchTmp))
        StringCchCopy(lpFileArg,FILE_ARG_BUF,tchTmp);
      else {
        if (SearchPath(NULL,lpFileArg,NULL,COUNTOF(tchTmp),tchTmp,NULL))
          StringCchCopy(lpFileArg,FILE_ARG_BUF,tchTmp);
        else {
          StringCchCopyN(tchTmp,COUNTOF(tchTmp),g_wchWorkingDirectory,COUNTOF(g_wchWorkingDirectory));
          PathCchAppend(tchTmp,COUNTOF(tchTmp),lpFileArg);
          StringCchCopy(lpFileArg,FILE_ARG_BUF,tchTmp);
        }
      }
    }

    else if (SearchPath(NULL,lpFileArg,NULL,COUNTOF(tchTmp),tchTmp,NULL))
      StringCchCopy(lpFileArg,FILE_ARG_BUF,tchTmp);

    NormalizePathEx(lpFileArg,FILE_ARG_BUF);

    EnumWindows(EnumWndProc2,(LPARAM)&hwnd);

    if (hwnd != NULL)
    {
      // Enabled
      if (IsWindowEnabled(hwnd))
      {
        // Make sure the previous window won't pop up a change notification message
        //SendMessage(hwnd,WM_CHANGENOTIFYCLEAR,0,0);

        if (IsIconic(hwnd))
          ShowWindowAsync(hwnd,SW_RESTORE);

        if (!IsWindowVisible(hwnd)) {
          SendMessage(hwnd,WM_TRAYMESSAGE,0,WM_LBUTTONDBLCLK);
          SendMessage(hwnd,WM_TRAYMESSAGE,0,WM_LBUTTONUP);
        }

        SetForegroundWindow(hwnd);

        DWORD cb = sizeof(np3params);
        if (lpSchemeArg)
          cb += (lstrlen(lpSchemeArg) + 1) * sizeof(WCHAR);

        LPnp3params params = AllocMem(cb, HEAP_ZERO_MEMORY);
        params->flagFileSpecified = false;
        params->flagChangeNotify = 0;
        params->flagQuietCreate = false;
        params->flagLexerSpecified = flagLexerSpecified;
        if (flagLexerSpecified && lpSchemeArg) {
          StringCchCopy(StrEnd(&params->wchData)+1,(lstrlen(lpSchemeArg)+1),lpSchemeArg);
          params->iInitialLexer = -1;
        }
        else
          params->iInitialLexer = iInitialLexer;
        params->flagJumpTo = flagJumpTo;
        params->iInitialLine = iInitialLine;
        params->iInitialColumn = iInitialColumn;

        params->iSrcEncoding = (lpEncodingArg) ? Encoding_MatchW(lpEncodingArg) : CPI_NONE;
        params->flagSetEncoding = flagSetEncoding;
        params->flagSetEOLMode = flagSetEOLMode;
        params->flagTitleExcerpt = 0;

        cds.dwData = DATA_NOTEPAD3_PARAMS;
        cds.cbData = (DWORD)SizeOfMem(params);
        cds.lpData = params;

        SendMessage(hwnd,WM_COPYDATA,(WPARAM)NULL,(LPARAM)&cds);
        FreeMem(params);

        return(true);
      }

      else // IsWindowEnabled()
      {
        // Ask...
        if (IDYES == MsgBox(MBYESNOWARN,IDS_ERR_PREVWINDISABLED))
          return(false);
        else
          return(true);
      }
    }
  }

  if (flagNoReuseWindow)
    return(false);

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

      if (IsIconic(hwnd))
        ShowWindowAsync(hwnd,SW_RESTORE);

      if (!IsWindowVisible(hwnd)) {
        SendMessage(hwnd,WM_TRAYMESSAGE,0,WM_LBUTTONDBLCLK);
        SendMessage(hwnd,WM_TRAYMESSAGE,0,WM_LBUTTONUP);
      }

      SetForegroundWindow(hwnd);

      if (lpFileArg)
      {
        // Search working directory from second instance, first!
        // lpFileArg is at least MAX_PATH+4 WCHAR
        WCHAR tchTmp[FILE_ARG_BUF] = { L'\0' };

        ExpandEnvironmentStringsEx(lpFileArg,(DWORD)SizeOfMem(lpFileArg)/sizeof(WCHAR));

        if (PathIsRelative(lpFileArg)) {
          StringCchCopyN(tchTmp,COUNTOF(tchTmp),g_wchWorkingDirectory,COUNTOF(g_wchWorkingDirectory));
          PathCchAppend(tchTmp,COUNTOF(tchTmp),lpFileArg);
          if (PathFileExists(tchTmp)) {
            StringCchCopy(lpFileArg, FILE_ARG_BUF, tchTmp);
          }
          else {
            if (SearchPath(NULL, lpFileArg, NULL, COUNTOF(tchTmp), tchTmp, NULL)) {
              StringCchCopy(lpFileArg, FILE_ARG_BUF, tchTmp);
            }
          }
        }
        else if (SearchPath(NULL, lpFileArg, NULL, COUNTOF(tchTmp), tchTmp, NULL)) {
          StringCchCopy(lpFileArg, FILE_ARG_BUF, tchTmp);
        }

        DWORD cb = sizeof(np3params);
        cb += (lstrlen(lpFileArg) + 1) * sizeof(WCHAR);

        if (lpSchemeArg)
          cb += (lstrlen(lpSchemeArg) + 1) * sizeof(WCHAR);

        int cchTitleExcerpt = (int)StringCchLenW(szTitleExcerpt,COUNTOF(szTitleExcerpt));
        if (cchTitleExcerpt) {
          cb += (cchTitleExcerpt + 1) * sizeof(WCHAR);
        }
        LPnp3params params = AllocMem(cb, HEAP_ZERO_MEMORY);
        params->flagFileSpecified = true;
        StringCchCopy(&params->wchData,lstrlen(lpFileArg)+1,lpFileArg);
        params->flagChangeNotify = flagChangeNotify;
        params->flagQuietCreate = flagQuietCreate;
        params->flagLexerSpecified = flagLexerSpecified;
        if (flagLexerSpecified && lpSchemeArg) {
          StringCchCopy(StrEnd(&params->wchData)+1,lstrlen(lpSchemeArg)+1,lpSchemeArg);
          params->iInitialLexer = -1;
        }
        else {
          params->iInitialLexer = iInitialLexer;
        }
        params->flagJumpTo = flagJumpTo;
        params->iInitialLine = iInitialLine;
        params->iInitialColumn = iInitialColumn;

        params->iSrcEncoding = (lpEncodingArg) ? Encoding_MatchW(lpEncodingArg) : CPI_NONE;
        params->flagSetEncoding = flagSetEncoding;
        params->flagSetEOLMode = flagSetEOLMode;

        if (cchTitleExcerpt) {
          StringCchCopy(StrEnd(&params->wchData)+1,cchTitleExcerpt+1,szTitleExcerpt);
          params->flagTitleExcerpt = 1;
        }
        else {
          params->flagTitleExcerpt = 0;
        }
        cds.dwData = DATA_NOTEPAD3_PARAMS;
        cds.cbData = (DWORD)SizeOfMem(params);
        cds.lpData = params;

        SendMessage(hwnd,WM_COPYDATA,(WPARAM)NULL,(LPARAM)&cds);
        FreeMem(params);    params = NULL;
        FreeMem(lpFileArg); lpFileArg = NULL;
      }
      return(true);
    }
    else // IsWindowEnabled()
    {
      // Ask...
      return ((IDYES == MsgBox(MBYESNOWARN, IDS_ERR_PREVWINDISABLED)) ? false : true);
    }
  }
  else
    return(false);
}


//=============================================================================
//
//  RelaunchMultiInst()
//
//
bool RelaunchMultiInst() {

  if (flagMultiFileArg == 2 && cFileList > 1) {

    WCHAR *pwch;
    int i = 0;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    LPWSTR lpCmdLineNew = StrDup(GetCommandLine());
    int len = lstrlen(lpCmdLineNew) + 1;
    LPWSTR lp1 = LocalAlloc(LPTR,sizeof(WCHAR)*len);
    LPWSTR lp2 = LocalAlloc(LPTR,sizeof(WCHAR)*len);

    StrTab2Space(lpCmdLineNew);
    StringCchCopy(lpCmdLineNew + cchiFileList,2,L"");

    pwch = CharPrev(lpCmdLineNew,StrEnd(lpCmdLineNew));
    while (*pwch == L' ' || *pwch == L'-' || *pwch == L'+') {
      *pwch = L' ';
      pwch = CharPrev(lpCmdLineNew,pwch);
      if (i++ > 1)
        cchiFileList--;
    }

    for (i = 0; i < cFileList; i++) 
    {
      StringCchCopy(lpCmdLineNew + cchiFileList,8,L" /n - ");
      StringCchCat(lpCmdLineNew,len,lpFileList[i]);
      LocalFree(lpFileList[i]);

      ZeroMemory(&si,sizeof(STARTUPINFO));
      si.cb = sizeof(STARTUPINFO);

      ZeroMemory(&pi,sizeof(PROCESS_INFORMATION));

      CreateProcess(NULL,lpCmdLineNew,NULL,NULL,false,0,NULL,g_wchWorkingDirectory,&si,&pi);
    }

    LocalFree(lpCmdLineNew);
    LocalFree(lp1);
    LocalFree(lp2);
    FreeMem(lpFileArg); lpFileArg = NULL;

    return true;
  }

  else {
    int i;
    for (i = 0; i < cFileList; i++)
      LocalFree(lpFileList[i]);
    return false;
  }
}


//=============================================================================
//
//  RelaunchElevated()
//
//
bool RelaunchElevated(LPWSTR lpArgs) {

  bool result = false;

  if (!IsVista() || flagIsElevated || !flagRelaunchElevated || flagDisplayHelp)
    return result;

  STARTUPINFO si;
  si.cb = sizeof(STARTUPINFO);
  GetStartupInfo(&si);

  LPWSTR lpCmdLine = GetCommandLine();
  int wlen = lstrlen(lpCmdLine) + 2;

  WCHAR lpExe[MAX_PATH + 2] = { L'\0' };
  WCHAR szArgs[2032] = { L'\0' };
  WCHAR szArguments[2032] = { L'\0' };

  ExtractFirstArgument(lpCmdLine,lpExe,szArgs,wlen);

  if (lpArgs) {
    StringCchCopy(szArgs,COUNTOF(szArgs),lpArgs); // override
  }

  if (StrStrI(szArgs,L"/f ") || StrStrI(szArgs,L"-f ")) {
    StringCchCopy(szArguments,COUNTOF(szArguments),szArgs);
  }
  else {
    if (StringCchLenW(g_wchIniFile,COUNTOF(g_wchIniFile)) > 0)
      StringCchPrintf(szArguments,COUNTOF(szArguments),L"/f \"%s\" %s",g_wchIniFile,szArgs);
    else
      StringCchCopy(szArguments,COUNTOF(szArguments),szArgs);
  }

  if (lstrlen(szArguments)) {
    SHELLEXECUTEINFO sei;
    ZeroMemory(&sei,sizeof(SHELLEXECUTEINFO));
    sei.cbSize = sizeof(SHELLEXECUTEINFO);
    sei.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOASYNC | SEE_MASK_NOZONECHECKS;
    sei.hwnd = GetForegroundWindow();
    sei.lpVerb = L"runas";
    sei.lpFile = lpExe;
    sei.lpParameters = szArguments;
    sei.lpDirectory = g_wchWorkingDirectory;
    sei.nShow = si.wShowWindow ? si.wShowWindow : SW_SHOWNORMAL;
    result = ShellExecuteEx(&sei);
  }

  return result;
}


//=============================================================================
//
//  SnapToDefaultPos()
//
//  Aligns Notepad3 to the default window position on the current screen
//
//
void SnapToDefaultPos(HWND hwnd)
{  
  RECT rcOld; GetWindowRect(hwnd, &rcOld);

  RECT rc; SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);

  flagDefaultPos = 2;
  _InitWindowPosition(hwnd);

  WINDOWPLACEMENT wndpl;
  ZeroMemory(&wndpl, sizeof(WINDOWPLACEMENT));
  wndpl.length = sizeof(WINDOWPLACEMENT);
  wndpl.flags = WPF_ASYNCWINDOWPLACEMENT;
  wndpl.showCmd = SW_RESTORE;

  wndpl.rcNormalPosition.left = g_WinInfo.x - rc.left;
  wndpl.rcNormalPosition.top = g_WinInfo.y - rc.top;
  wndpl.rcNormalPosition.right = g_WinInfo.x - rc.left + g_WinInfo.cx;
  wndpl.rcNormalPosition.bottom = g_WinInfo.y - rc.top + g_WinInfo.cy;

  if (GetDoAnimateMinimize()) {
    DrawAnimatedRects(hwnd,IDANI_CAPTION,&rcOld,&wndpl.rcNormalPosition);
    //OffsetRect(&wndpl.rcNormalPosition,mi.rcMonitor.left - mi.rcWork.left,mi.rcMonitor.top - mi.rcWork.top);
  }
  SetWindowPlacement(hwnd,&wndpl);
}


//=============================================================================
//
//  ShowNotifyIcon()
//
//
void ShowNotifyIcon(HWND hwnd,bool bAdd)
{

  static HICON hIcon;
  NOTIFYICONDATA nid;

  if (!hIcon)
    hIcon = LoadImage(g_hInstance,MAKEINTRESOURCE(IDR_MAINWND),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);

  ZeroMemory(&nid,sizeof(NOTIFYICONDATA));
  nid.cbSize = sizeof(NOTIFYICONDATA);
  nid.hWnd = hwnd;
  nid.uID = 0;
  nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
  nid.uCallbackMessage = WM_TRAYMESSAGE;
  nid.hIcon = hIcon;
  StringCchCopy(nid.szTip,COUNTOF(nid.szTip),L"Notepad3");

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
  SHFILEINFO shfi;
  WCHAR tchTitle[256] = { L'\0' };
  WCHAR tchFormat[32] = { L'\0' };

  ZeroMemory(&nid,sizeof(NOTIFYICONDATA));
  nid.cbSize = sizeof(NOTIFYICONDATA);
  nid.hWnd = hwnd;
  nid.uID = 0;
  nid.uFlags = NIF_TIP;

  if (StringCchLenW(szTitleExcerpt,COUNTOF(szTitleExcerpt))) {
    GetString(IDS_TITLEEXCERPT,tchFormat,COUNTOF(tchFormat));
    StringCchPrintf(tchTitle,COUNTOF(tchTitle),tchFormat,szTitleExcerpt);
  }

  else if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
    SHGetFileInfo2(g_wchCurFile,FILE_ATTRIBUTE_NORMAL,
      &shfi,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES);
    PathCompactPathEx(tchTitle,shfi.szDisplayName,COUNTOF(tchTitle)-4,0);
  }
  else
    GetString(IDS_UNTITLED,tchTitle,COUNTOF(tchTitle)-4);

  if (IsDocumentModified || Encoding_HasChanged(CPI_GET))
    StringCchCopy(nid.szTip,COUNTOF(nid.szTip),L"* ");
  else
    StringCchCopy(nid.szTip,COUNTOF(nid.szTip),L"");

  StringCchCat(nid.szTip,COUNTOF(nid.szTip),tchTitle);

  Shell_NotifyIcon(NIM_MODIFY,&nid);
}


//=============================================================================
//
//  InstallFileWatching()
//
//
void InstallFileWatching(LPCWSTR lpszFile)
{

  WCHAR tchDirectory[MAX_PATH] = { L'\0' };
  HANDLE hFind;

  // Terminate
  if (!g_iFileWatchingMode || !lpszFile || StringCchLen(lpszFile,MAX_PATH) == 0)
  {
    if (g_bRunningWatch)
    {
      if (hChangeHandle) {
        FindCloseChangeNotification(hChangeHandle);
        hChangeHandle = NULL;
      }
      KillTimer(NULL,ID_WATCHTIMER);
      g_bRunningWatch = false;
      dwChangeNotifyTime = 0;
    }
  }
  else  // Install
  {
    // Terminate previous watching
    if (g_bRunningWatch) {
      if (hChangeHandle) {
        FindCloseChangeNotification(hChangeHandle);
        hChangeHandle = NULL;
      }
      dwChangeNotifyTime = 0;
    }

    // No previous watching installed, so launch the timer first
    else
      SetTimer(NULL,ID_WATCHTIMER,dwFileCheckInverval,WatchTimerProc);

    StringCchCopy(tchDirectory,COUNTOF(tchDirectory),lpszFile);
    PathRemoveFileSpec(tchDirectory);

    // Save data of current file
    hFind = FindFirstFile(g_wchCurFile,&fdCurFile);
    if (hFind != INVALID_HANDLE_VALUE)
      FindClose(hFind);
    else
      ZeroMemory(&fdCurFile,sizeof(WIN32_FIND_DATA));

    hChangeHandle = FindFirstChangeNotification(tchDirectory,false,
      FILE_NOTIFY_CHANGE_FILE_NAME  | \
      FILE_NOTIFY_CHANGE_DIR_NAME   | \
      FILE_NOTIFY_CHANGE_ATTRIBUTES | \
      FILE_NOTIFY_CHANGE_SIZE | \
      FILE_NOTIFY_CHANGE_LAST_WRITE);

    g_bRunningWatch = true;
    dwChangeNotifyTime = 0;
  }
  UpdateToolbar();
}


//=============================================================================
//
//  WatchTimerProc()
//
//
void CALLBACK WatchTimerProc(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime)
{
  if (g_bRunningWatch)
  {
    if (dwChangeNotifyTime > 0 && GetTickCount() - dwChangeNotifyTime > dwAutoReloadTimeout)
    {
      if (hChangeHandle) {
        FindCloseChangeNotification(hChangeHandle);
        hChangeHandle = NULL;
      }
      KillTimer(NULL,ID_WATCHTIMER);
      g_bRunningWatch = false;
      dwChangeNotifyTime = 0;
      SendMessage(g_hwndMain,WM_CHANGENOTIFY,0,0);
    }

    // Check Change Notification Handle
    else if (WAIT_OBJECT_0 == WaitForSingleObject(hChangeHandle,0))
    {
      // Check if the changes affect the current file
      WIN32_FIND_DATA fdUpdated;
      HANDLE hFind = FindFirstFile(g_wchCurFile,&fdUpdated);
      if (INVALID_HANDLE_VALUE != hFind)
        FindClose(hFind);
      else
        // The current file has been removed
        ZeroMemory(&fdUpdated,sizeof(WIN32_FIND_DATA));

      // Check if the file has been changed
      if (CompareFileTime(&fdCurFile.ftLastWriteTime,&fdUpdated.ftLastWriteTime) != 0 ||
            fdCurFile.nFileSizeLow != fdUpdated.nFileSizeLow ||
            fdCurFile.nFileSizeHigh != fdUpdated.nFileSizeHigh)
      {
        // Shutdown current watching and give control to main window
        if (hChangeHandle) {
          FindCloseChangeNotification(hChangeHandle);
          hChangeHandle = NULL;
        }
        if (g_iFileWatchingMode == 2) {
          g_bRunningWatch = true; /* ! */
          dwChangeNotifyTime = GetTickCount();
        }
        else {
          KillTimer(NULL,ID_WATCHTIMER);
          g_bRunningWatch = false;
          dwChangeNotifyTime = 0;
          SendMessage(g_hwndMain,WM_CHANGENOTIFY,0,0);
        }
      }

      else
        FindNextChangeNotification(hChangeHandle);
    }
  }

  UNUSED(dwTime);
  UNUSED(idEvent);
  UNUSED(uMsg);
  UNUSED(hwnd);
}


//=============================================================================
//
//  PasteBoardTimer()
//
//
void CALLBACK PasteBoardTimer(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime)
{
  if ((dwLastCopyTime > 0) && ((GetTickCount() - dwLastCopyTime) > 200)) {

    if (SendMessage(g_hwndEdit,SCI_CANPASTE,0,0)) {

      bool bAutoIndent2 = bAutoIndent;
      bAutoIndent = 0;
      EditJumpTo(g_hwndEdit,-1,0);
      int token = BeginUndoAction();
      if (SendMessage(g_hwndEdit, SCI_GETLENGTH, 0, 0) > 0) {
        SendMessage(g_hwndEdit, SCI_NEWLINE, 0, 0);
      }
      SendMessage(g_hwndEdit,SCI_PASTE,0,0);
      SendMessage(g_hwndEdit,SCI_NEWLINE,0,0);
      EndUndoAction(token);
      EditEnsureSelectionVisible(g_hwndEdit);
      bAutoIndent = bAutoIndent2;
    }
    dwLastCopyTime = 0;
  }

  UNUSED(dwTime);
  UNUSED(idEvent);
  UNUSED(uMsg);
  UNUSED(hwnd);
}



///  End of Notepad3.c  \\\
