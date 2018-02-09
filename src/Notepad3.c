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
#include "helpers.h"
#include "notepad3.h"
#include "SciCall.h"


/******************************************************************************
*
* Local and global Variables for Notepad3.c
*
*/
HWND      g_hwndMain = NULL;
HWND      g_hwndEdit = NULL;
HWND      g_hwndStatus = NULL;
HWND      g_hwndToolbar = NULL;
HWND      hwndReBar = NULL;
HWND      hwndEditFrame = NULL;
HWND      hwndNextCBChain = NULL;
HWND      hDlgFindReplace = NULL;

#define INISECTIONBUFCNT 32
#define NUMTOOLBITMAPS  25
#define NUMINITIALTOOLS 30
#define MARGIN_FOLD_INDEX 2

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
                            { 0,0,0,TBSTYLE_SEP,0,0 },
                            { 21,IDT_FILE_OPENFAV,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
                            { 22,IDT_FILE_ADDTOFAV,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
                            { 0,0,0,TBSTYLE_SEP,0,0 },
                            { 12,IDT_VIEW_ZOOMIN,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
                            { 13,IDT_VIEW_ZOOMOUT,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0 },
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

#define TBBUTTON_DEFAULT_IDS  L"1 2 4 3 0 5 6 0 7 8 9 0 10 11 0 12 0 24 0 22 23 0 13 14 0 15 0 25 0 17"


WCHAR      g_wchIniFile[MAX_PATH] = { L'\0' };
WCHAR      g_wchIniFile2[MAX_PATH] = { L'\0' };
WCHAR      szBufferFile[MAX_PATH] = { L'\0' };
BOOL       bSaveSettings;
BOOL       bEnableSaveSettings;
BOOL       bSaveRecentFiles;
BOOL       bPreserveCaretPos;
BOOL       bSaveFindReplace;
WCHAR      tchLastSaveCopyDir[MAX_PATH] = { L'\0' };
WCHAR      tchOpenWithDir[MAX_PATH] = { L'\0' };
WCHAR      tchFavoritesDir[MAX_PATH] = { L'\0' };
WCHAR      tchDefaultDir[MAX_PATH] = { L'\0' };
WCHAR      tchDefaultExtension[64] = { L'\0' };
WCHAR      tchFileDlgFilters[5*1024] = { L'\0' };
WCHAR      tchToolbarButtons[512] = { L'\0' };
WCHAR      tchToolbarBitmap[MAX_PATH] = { L'\0' };
WCHAR      tchToolbarBitmapHot[MAX_PATH] = { L'\0' };
WCHAR      tchToolbarBitmapDisabled[MAX_PATH] = { L'\0' };

int       iPathNameFormat;
BOOL      bWordWrap;
BOOL      bWordWrapG;
int       iWordWrapMode;
int       iWordWrapIndent;
int       iWordWrapSymbols;
BOOL      bShowWordWrapSymbols;
BOOL      bMatchBraces;
BOOL      bAutoIndent;
BOOL      bAutoCloseTags;
BOOL      bShowIndentGuides;
BOOL      bHiliteCurrentLine;
BOOL      bHyperlinkHotspot;
BOOL      g_bTabsAsSpaces;
BOOL      bTabsAsSpacesG;
BOOL      g_bTabIndents;
BOOL      bTabIndentsG;
BOOL      bBackspaceUnindents;
int       g_iTabWidth;
int       iTabWidthG;
int       g_iIndentWidth;
int       iIndentWidthG;
BOOL      bMarkLongLines;
int       iLongLinesLimit;
int       iLongLinesLimitG;
int       iLongLineMode;
int       iWrapCol = 0;
BOOL      bShowSelectionMargin;
BOOL      bShowLineNumbers;
int       iMarkOccurrences;
int       iMarkOccurrencesCount;
int       iMarkOccurrencesMaxCount;
BOOL      bMarkOccurrencesMatchVisible;
BOOL      bMarkOccurrencesMatchCase;
BOOL      bMarkOccurrencesMatchWords;
BOOL      bMarkOccurrencesCurrentWord;
BOOL      bUseOldStyleBraceMatching;
BOOL      bAutoCompleteWords;
BOOL      bAccelWordNavigation;
BOOL      bDenyVirtualSpaceAccess;
BOOL      bShowCodeFolding;
BOOL      bViewWhiteSpace;
BOOL      bViewEOLs;
BOOL      bSkipUnicodeDetection;
BOOL      bLoadASCIIasUTF8;
BOOL      bLoadNFOasOEM;
BOOL      bNoEncodingTags;
BOOL      bFixLineEndings;
BOOL      bAutoStripBlanks;
int       iPrintHeader;
int       iPrintFooter;
int       iPrintColor;
int       iPrintZoom;
RECT      pagesetupMargin;
BOOL      bSaveBeforeRunningTools;
int       iFileWatchingMode;
BOOL      bResetFileWatching;
DWORD     dwFileCheckInverval;
DWORD     dwAutoReloadTimeout;
int       iEscFunction;
BOOL      bAlwaysOnTop;
BOOL      bMinimizeToTray;
BOOL      bTransparentMode;
BOOL      bTransparentModeAvailable;
BOOL      bShowToolbar;
BOOL      bShowStatusbar;
int       iSciDirectWriteTech;
int       iSciFontQuality;
int       iHighDpiToolBar;
int       iUpdateDelayHyperlinkStyling;
int       iUpdateDelayMarkAllCoccurrences;
int       iCurrentLineVerticalSlop = 5;

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

WININFO g_WinInfo = { CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0 };

BOOL    bStickyWinPos;

BOOL    bIsAppThemed;
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

LPWSTR    lpFileList[32] = { NULL };
int       cFileList = 0;
int       cchiFileList = 0;
LPWSTR    lpFileArg = NULL;
LPWSTR    lpSchemeArg = NULL;
LPWSTR    lpMatchArg = NULL;
LPWSTR    lpEncodingArg = NULL;
LPMRULIST pFileMRU;
LPMRULIST mruFind;
LPMRULIST mruReplace;

DWORD     dwLastIOError;

int       iDefaultEncoding;
int       iDefaultCharSet;

int       iEOLMode;
int       iDefaultEOLMode;

int       iInitialLine;
int       iInitialColumn;

int       iInitialLexer;

BOOL      bLastCopyFromMe = FALSE;
DWORD     dwLastCopyTime;

UINT      uidsAppTitle = IDS_APPTITLE;
WCHAR     szTitleExcerpt[MIDSZ_BUFFER] = { L'\0' };
int       fKeepTitleExcerpt = 0;

HANDLE    hChangeHandle = NULL;
BOOL      bRunningWatch = FALSE;
BOOL      dwChangeNotifyTime = 0;
WIN32_FIND_DATA fdCurFile;

UINT      msgTaskbarCreated = 0;

HMODULE   hModUxTheme = NULL;

EDITFINDREPLACE g_efrData = { "", "", "", "", 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL };
UINT cpLastFind = 0;
BOOL bReplaceInitialized = FALSE;

extern NP2ENCODING g_Encodings[];

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

BOOL      flagIsElevated = FALSE;
WCHAR     wchWndClass[16] = WC_NOTEPAD3;


HINSTANCE g_hInstance = NULL;
HANDLE    g_hScintilla = NULL;
WCHAR     g_wchAppUserModelID[32] = { L'\0' };
WCHAR     g_wchWorkingDirectory[MAX_PATH+2] = { L'\0' };
WCHAR     g_wchCurFile[FILE_ARG_BUF] = { L'\0' };
FILEVARS  fvCurFile;
BOOL      bReadOnly = FALSE;


// undo / redo  selections
static UT_icd UndoRedoSelection_icd = { sizeof(UndoRedoSelection_t), NULL, NULL, NULL };
static UT_array* UndoRedoSelectionUTArray = NULL;


static CLIPFORMAT cfDrpF = CF_HDROP;
static POINTL ptDummy = { 0, 0 };
static PDROPTARGET pDropTarget = NULL;
static DWORD DropFilesProc(CLIPFORMAT cf, HGLOBAL hData, HWND hWnd, DWORD dwKeyState, POINTL pt, void *pUserData);

// Timer bitfield
static volatile LONG g_lInterlockBits = 0;
#define TIMER_BIT_MARK_OCC 1L
#define TIMER_BIT_UPDATE_HYPER 2L
#define LOCK_NOTIFY_CHANGE 4L
#define TEST_AND_SET(B)  InterlockedBitTestAndSet(&g_lInterlockBits, B)
#define TEST_AND_RESET(B)  InterlockedBitTestAndReset(&g_lInterlockBits, B)


//=============================================================================
//
//  IgnoreNotifyChangeEvent(), ObserveNotifyChangeEvent(), CheckNotifyChangeEvent()
//
void IgnoreNotifyChangeEvent() {
  (void)TEST_AND_SET(LOCK_NOTIFY_CHANGE);
}

void ObserveNotifyChangeEvent() {
  (void)TEST_AND_RESET(LOCK_NOTIFY_CHANGE);
}

BOOL CheckNotifyChangeEvent() {
  if (TEST_AND_RESET(LOCK_NOTIFY_CHANGE)) {
    (void)TEST_AND_SET(LOCK_NOTIFY_CHANGE);
    return FALSE;
  }
  return TRUE;
}

// SCN_UPDATEUI notification
#define SC_UPDATE_NP3_INTERNAL_NOTIFY (SC_UPDATE_H_SCROLL << 1)


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
static BOOL IsDocumentModified = FALSE;

void __fastcall SetDocumentModified(BOOL bModified)
{
  if (IsDocumentModified != bModified) {
    IsDocumentModified = bModified;
    UpdateToolbar();
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

  hModUxTheme = LoadLibrary(L"uxtheme.dll");

  Scintilla_RegisterClasses(hInstance);

  // Load Settings
  LoadSettings();

  if (!InitApplication(hInstance))
    return FALSE;
  
  hwnd = InitInstance(hInstance, lpCmdLine, nCmdShow);
  if (!hwnd)
    return FALSE;
  
  // init DragnDrop handler
  DragAndDropInit(NULL);

  if (IsVista()) {
    // Current platforms perform window buffering so it is almost always better for this option to be turned off.
    // There are some older platforms and unusual modes where buffering may still be useful - so keep it ON
    //~SciCall_SetBufferedDraw(TRUE);  // default is TRUE 

    if (iSciDirectWriteTech >= 0) {
      SciCall_SetTechnology(DirectWriteTechnology[iSciDirectWriteTech]);
    }
  }

  hAccMain = LoadAccelerators(hInstance,MAKEINTRESOURCE(IDR_MAINWND));
  hAccFindReplace = LoadAccelerators(hInstance,MAKEINTRESOURCE(IDR_ACCFINDREPLACE));
  
  UpdateLineNumberWidth();
  ObserveNotifyChangeEvent();

  while (GetMessage(&msg,NULL,0,0))
  {
    if (IsWindow(hDlgFindReplace) && ((msg.hwnd == hDlgFindReplace) || IsChild(hDlgFindReplace, msg.hwnd))) 
    {
      int iTr = TranslateAccelerator(hDlgFindReplace, hAccFindReplace, &msg);
      if (iTr || IsDialogMessage(hDlgFindReplace, &msg))
        continue;
    }
    if (!TranslateAccelerator(hwnd,hAccMain,&msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
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
BOOL InitApplication(HINSTANCE hInstance)
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
//  InitInstance()
//
//
HWND InitInstance(HINSTANCE hInstance,LPSTR pszCmdLine,int nCmdShow)
{
  RECT rc;
  rc.left = g_WinInfo.x;  rc.top = g_WinInfo.y;  rc.right = g_WinInfo.x + g_WinInfo.cx;  rc.bottom = g_WinInfo.y + g_WinInfo.cy;
  RECT rc2;
  MONITORINFO mi;

  HMONITOR hMonitor = MonitorFromRect(&rc,MONITOR_DEFAULTTONEAREST);
  mi.cbSize = sizeof(mi);
  GetMonitorInfo(hMonitor,&mi);

  if (flagDefaultPos == 1) {
    g_WinInfo.x = g_WinInfo.y = g_WinInfo.cx = g_WinInfo.cy = CW_USEDEFAULT;
    g_WinInfo.max = 0;
  }
  else if (flagDefaultPos >= 4) {
    SystemParametersInfo(SPI_GETWORKAREA,0,&rc,0);
    if (flagDefaultPos & 8)
      g_WinInfo.x = (rc.right - rc.left) / 2;
    else
      g_WinInfo.x = rc.left;
    g_WinInfo.cx = rc.right - rc.left;
    if (flagDefaultPos & (4|8))
      g_WinInfo.cx /= 2;
    if (flagDefaultPos & 32)
      g_WinInfo.y = (rc.bottom - rc.top) / 2;
    else
      g_WinInfo.y = rc.top;
    g_WinInfo.cy = rc.bottom - rc.top;
    if (flagDefaultPos & (16|32))
      g_WinInfo.cy /= 2;
    if (flagDefaultPos & 64) {
      g_WinInfo.x = rc.left;
      g_WinInfo.y = rc.top;
      g_WinInfo.cx = rc.right - rc.left;
      g_WinInfo.cy = rc.bottom - rc.top;
    }
    if (flagDefaultPos & 128) {
      g_WinInfo.x += (flagDefaultPos & 8) ? 4 : 8;
      g_WinInfo.cx -= (flagDefaultPos & (4|8)) ? 12 : 16;
      g_WinInfo.y += (flagDefaultPos & 32) ? 4 : 8;
      g_WinInfo.cy -= (flagDefaultPos & (16|32)) ? 12 : 16;
    }
  }

  else if (flagDefaultPos == 2 || flagDefaultPos == 3 ||
      g_WinInfo.x == CW_USEDEFAULT || g_WinInfo.y == CW_USEDEFAULT ||
      g_WinInfo.cx == CW_USEDEFAULT || g_WinInfo.cy == CW_USEDEFAULT) {

    // default window position
    SystemParametersInfo(SPI_GETWORKAREA,0,&rc,0);
    g_WinInfo.y = rc.top + 16;
    g_WinInfo.cy = rc.bottom - rc.top - 32;
    g_WinInfo.cx = min(rc.right - rc.left - 32,g_WinInfo.cy);
    g_WinInfo.x = (flagDefaultPos == 3) ? rc.left + 16 : rc.right - g_WinInfo.cx - 16;
  }

  else {

    // fit window into working area of current monitor
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
    SetRect(&rc,g_WinInfo.x,g_WinInfo.y,g_WinInfo.x+g_WinInfo.cx,g_WinInfo.y+g_WinInfo.cy);
    if (!IntersectRect(&rc2,&rc,&mi.rcWork)) {
      g_WinInfo.y = mi.rcWork.top + 16;
      g_WinInfo.cy = mi.rcWork.bottom - mi.rcWork.top - 32;
      g_WinInfo.cx = min(mi.rcWork.right - mi.rcWork.left - 32,g_WinInfo.cy);
      g_WinInfo.x = mi.rcWork.right - g_WinInfo.cx - 16;
    }
  }

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
    SetWindowTransparentMode(g_hwndMain,TRUE);

  // Current file information -- moved in front of ShowWindow()
  FileLoad(TRUE,TRUE,FALSE,FALSE,L"");

  if (!flagStartAsTrayIcon) {
    ShowWindow(g_hwndMain,nCmdShow);
    UpdateWindow(g_hwndMain);
  }
  else {
    ShowWindow(g_hwndMain,SW_HIDE);    // trick ShowWindow()
    ShowNotifyIcon(g_hwndMain,TRUE);
  }

  // Source Encoding
  if (lpEncodingArg)
    Encoding_Source(Encoding_MatchW(lpEncodingArg));

  // Pathname parameter
  if (flagBufferFile || (lpFileArg /*&& !flagNewFromClipboard*/))
  {
    BOOL bOpened = FALSE;

    // Open from Directory
    if (!flagBufferFile && PathIsDirectory(lpFileArg)) {
      WCHAR tchFile[MAX_PATH] = { L'\0' };
      if (OpenFileDlg(g_hwndMain,tchFile,COUNTOF(tchFile),lpFileArg))
        bOpened = FileLoad(FALSE,FALSE,FALSE,FALSE,tchFile);
    }
    else {
      LPCWSTR lpFileToOpen = flagBufferFile ? szBufferFile : lpFileArg;
      bOpened = FileLoad(FALSE,FALSE,FALSE,FALSE,lpFileToOpen);
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

          SetDocumentModified(TRUE);
          UpdateLineNumberWidth();

          // check for temp file and delete
          if (flagIsElevated && PathFileExists(szBufferFile)) {
            DeleteFile(szBufferFile);
          }
        }
        if (flagJumpTo) { // Jump to position
          EditJumpTo(g_hwndEdit,iInitialLine,iInitialColumn);
        }
      }
    }
    GlobalFree(lpFileArg); lpFileArg = NULL;

    if (bOpened) {
      if (flagChangeNotify == 1) {
        iFileWatchingMode = 0;
        bResetFileWatching = TRUE;
        InstallFileWatching(g_wchCurFile);
      }
      else if (flagChangeNotify == 2) {
        iFileWatchingMode = 2;
        bResetFileWatching = TRUE;
        InstallFileWatching(g_wchCurFile);
      }
    }
  }
  else {
    if (Encoding_Source(CPI_GET) != CPI_NONE) {
      Encoding_Current(Encoding_Source(CPI_GET));
      Encoding_HasChanged(Encoding_Source(CPI_GET));
      Encoding_SciSetCodePage(g_hwndEdit,Encoding_Current(CPI_GET));
    }
  }

  // reset
  Encoding_Source(CPI_NONE);
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
      BOOL bAutoIndent2 = bAutoIndent;
      bAutoIndent = 0;
      EditJumpTo(g_hwndEdit, -1, 0);
      SendMessage(g_hwndEdit, SCI_BEGINUNDOACTION, 0, 0);
      if (SendMessage(g_hwndEdit, SCI_GETLENGTH, 0, 0) > 0) {
        SendMessage(g_hwndEdit, SCI_NEWLINE, 0, 0);
      }
      SendMessage(g_hwndEdit, SCI_PASTE, 0, 0);
      SendMessage(g_hwndEdit, SCI_NEWLINE, 0, 0);
      SendMessage(g_hwndEdit, SCI_ENDUNDOACTION, 0, 0);
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

      UINT cp = Encoding_SciGetCodePage(g_hwndEdit);
      WideCharToMultiByteStrg(cp,lpMatchArg,g_efrData.szFind);
      WideCharToMultiByteStrg(CP_UTF8,lpMatchArg,g_efrData.szFindUTF8);
      cpLastFind = cp;

      if (flagMatchText & 4)
        g_efrData.fuFlags |= (SCFIND_REGEXP | SCFIND_POSIX);
      else if (flagMatchText & 8)
        g_efrData.bTransformBS = TRUE;

      if (flagMatchText & 2) {
        if (!flagJumpTo) { SendMessage(g_hwndEdit, SCI_DOCUMENTEND, 0, 0); }
        EditFindPrev(g_hwndEdit,&g_efrData,FALSE);
        EditEnsureSelectionVisible(g_hwndEdit);
      }
      else {
        if (!flagJumpTo) { SendMessage(g_hwndEdit, SCI_DOCUMENTSTART, 0, 0); }
        EditFindNext(g_hwndEdit,&g_efrData,FALSE);
        EditEnsureSelectionVisible(g_hwndEdit);
      }
    }
    GlobalFree(lpMatchArg);
  }

  // Check for Paste Board option -- after loading files
  if (flagPasteBoard) {
    bLastCopyFromMe = TRUE;
    hwndNextCBChain = SetClipboardViewer(g_hwndMain);
    uidsAppTitle = IDS_APPTITLE_PASTEBOARD;
    bLastCopyFromMe = FALSE;

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

  iMarkOccurrencesCount = 0;
  UpdateToolbar();
  UpdateStatusbar();
  UpdateLineNumberWidth();

  // print file immediately and quit
  if (flagPrintFileAndLeave)
  {
    SHFILEINFO shfi;
    WCHAR *pszTitle;
    WCHAR tchUntitled[32] = { L'\0' };
    WCHAR tchPageFmt[32] = { L'\0' };

    if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
      SHGetFileInfo2(g_wchCurFile, 0, &shfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME);
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
  static BOOL bAltKeyIsDown = FALSE;

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
      return DefWindowProc(hwnd,umsg,wParam,lParam);

    case WM_SYSKEYDOWN:
      if (GetAsyncKeyState(VK_MENU) & SHRT_MIN)  // ALT-KEY DOWN
      {
        if (!bAltKeyIsDown) {
          bAltKeyIsDown = TRUE;
          if (!bDenyVirtualSpaceAccess) {
            SendMessage(g_hwndEdit, SCI_SETVIRTUALSPACEOPTIONS, (WPARAM)(SCVS_RECTANGULARSELECTION | SCVS_NOWRAPLINESTART | SCVS_USERACCESSIBLE), 0);
          }
        }
      }
      return DefWindowProc(hwnd, umsg, wParam, lParam);

    case WM_SYSKEYUP:
      if (!(GetAsyncKeyState(VK_MENU) & SHRT_MIN))  // NOT ALT-KEY DOWN
      {
        if (bAltKeyIsDown) {
          bAltKeyIsDown = FALSE;
          SendMessage(g_hwndEdit, SCI_SETVIRTUALSPACEOPTIONS, (WPARAM)SCVS_RECTANGULARSELECTION, 0);
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
      if (FileSave(FALSE,TRUE,FALSE,FALSE))
        DestroyWindow(hwnd);
      break;

    case WM_QUERYENDSESSION:
      if (FileSave(FALSE,TRUE,FALSE,FALSE))
        return TRUE;
      else
        return FALSE;

    // Reinitialize theme-dependent values and resize windows
    case WM_THEMECHANGED:
      MsgThemeChanged(hwnd,wParam,lParam);
      break;

    // update Scintilla colors
    case WM_SYSCOLORCHANGE:
      UpdateLineNumberWidth();
      EditClearAllMarks(g_hwndEdit, 0, -1);
      MarkAllOccurrences(0);
      UpdateVisibleUrlHotspot(0);
      return DefWindowProc(hwnd,umsg,wParam,lParam);

    case WM_TIMER:
      if (LOWORD(wParam) == IDT_TIMER_MAIN_MRKALL) {
        if (TEST_AND_RESET(TIMER_BIT_MARK_OCC)) {
          PostMessage(hwnd, WM_COMMAND, MAKELONG(IDC_MAIN_MARKALL_OCC, 1), 0);
          KillTimer(hwnd, IDT_TIMER_MAIN_MRKALL);
        }
        return TRUE;
      }
      else if (LOWORD(wParam) == IDT_TIMER_UPDATE_HOTSPOT) {
        if (TEST_AND_RESET(TIMER_BIT_UPDATE_HYPER)) {
          PostMessage(hwnd, WM_COMMAND, MAKELONG(IDC_CALL_UPDATE_HOTSPOT, 1), 0);
          KillTimer(hwnd, IDT_TIMER_UPDATE_HOTSPOT);
        }
        return TRUE;
      }
      break;

    case WM_SIZE:
      MsgSize(hwnd,wParam,lParam);
      break;

    case WM_SETFOCUS:
      SetFocus(g_hwndEdit);
      //UpdateToolbar();
      //UpdateStatusbar();
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
    //  bPendingChangeNotify = FALSE;
    //  break;

    case WM_DRAWCLIPBOARD:
      if (!bLastCopyFromMe)
        dwLastCopyTime = GetTickCount();
      else
        bLastCopyFromMe = FALSE;

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
          ShowNotifyIcon(hwnd,TRUE);
        SetNotifyIconTitle(hwnd);
      }
      return DefWindowProc(hwnd, umsg, wParam, lParam);
  }
  return 0; // swallow message
}



//
//  SetWordWrapping() - WordWrapSettings
//
void __fastcall SetWordWrapping()
{
  // Word wrap
  if (bWordWrap)
    SendMessage(g_hwndEdit, SCI_SETWRAPMODE, (iWordWrapMode == 0) ? SC_WRAP_WHITESPACE : SC_WRAP_CHAR, 0);
  else
    SendMessage(g_hwndEdit, SCI_SETWRAPMODE, SC_WRAP_NONE, 0);

  if (iWordWrapIndent == 5)
    SendMessage(g_hwndEdit, SCI_SETWRAPINDENTMODE, SC_WRAPINDENT_SAME, 0);
  else if (iWordWrapIndent == 6)
    SendMessage(g_hwndEdit, SCI_SETWRAPINDENTMODE, SC_WRAPINDENT_INDENT, 0);
  else {
    int i = 0;
    switch (iWordWrapIndent) {
    case 1: i = 1; break;
    case 2: i = 2; break;
    case 3: i = (g_iIndentWidth) ? 1 * g_iIndentWidth : 1 * g_iTabWidth; break;
    case 4: i = (g_iIndentWidth) ? 2 * g_iIndentWidth : 2 * g_iTabWidth; break;
    }
    SendMessage(g_hwndEdit, SCI_SETWRAPSTARTINDENT, i, 0);
    SendMessage(g_hwndEdit, SCI_SETWRAPINDENTMODE, SC_WRAPINDENT_FIXED, 0);
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
    SendMessage(g_hwndEdit, SCI_SETWRAPVISUALFLAGSLOCATION, wrapVisualFlagsLocation, 0);
    SendMessage(g_hwndEdit, SCI_SETWRAPVISUALFLAGS, wrapVisualFlags, 0);
  }
  else {
    SendMessage(g_hwndEdit, SCI_SETWRAPVISUALFLAGS, 0, 0);
  }
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
  g_hwndEdit = EditCreate(hwnd);
  InitScintillaHandle(g_hwndEdit);

  // Properties
  SendMessage(g_hwndEdit, SCI_SETYCARETPOLICY, (WPARAM)(CARET_SLOP | CARET_EVEN | CARET_STRICT), iCurrentLineVerticalSlop);
  SendMessage(g_hwndEdit, SCI_SETVIRTUALSPACEOPTIONS, (WPARAM)(bDenyVirtualSpaceAccess ? SCVS_NONE : SCVS_RECTANGULARSELECTION), 0);

  // Tabs
  SendMessage(g_hwndEdit,SCI_SETUSETABS,!g_bTabsAsSpaces,0);
  SendMessage(g_hwndEdit,SCI_SETTABINDENTS,g_bTabIndents,0);
  SendMessage(g_hwndEdit,SCI_SETBACKSPACEUNINDENTS,bBackspaceUnindents,0);
  SendMessage(g_hwndEdit,SCI_SETTABWIDTH,g_iTabWidth,0);
  SendMessage(g_hwndEdit,SCI_SETINDENT,g_iIndentWidth,0);

  // Indent Guides
  Style_SetIndentGuides(g_hwndEdit,bShowIndentGuides);

  // Word Wrap
  SetWordWrapping();

  // Long Lines
  if (bMarkLongLines)
    SendMessage(g_hwndEdit,SCI_SETEDGEMODE,(iLongLineMode == EDGE_LINE)?EDGE_LINE:EDGE_BACKGROUND,0);
  else
    SendMessage(g_hwndEdit,SCI_SETEDGEMODE,EDGE_NONE,0);

  SendMessage(g_hwndEdit,SCI_SETEDGECOLUMN,iLongLinesLimit,0);

  // Margins
  Style_SetCurrentMargin(g_hwndEdit, bShowSelectionMargin);

  // Code folding
  SciCall_SetMarginType(MARGIN_FOLD_INDEX, SC_MARGIN_SYMBOL);
  SciCall_SetMarginMask(MARGIN_FOLD_INDEX, SC_MASK_FOLDERS);
  SciCall_SetMarginWidth(MARGIN_FOLD_INDEX, (bShowCodeFolding) ? 11 : 0);
  SciCall_SetMarginSensitive(MARGIN_FOLD_INDEX, TRUE);
  SciCall_MarkerDefine(SC_MARKNUM_FOLDEROPEN, SC_MARK_BOXMINUS);
  SciCall_MarkerDefine(SC_MARKNUM_FOLDER, SC_MARK_BOXPLUS);
  SciCall_MarkerDefine(SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);
  SciCall_MarkerDefine(SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNER);
  SciCall_MarkerDefine(SC_MARKNUM_FOLDEREND, SC_MARK_BOXPLUSCONNECTED);
  SciCall_MarkerDefine(SC_MARKNUM_FOLDEROPENMID, SC_MARK_BOXMINUSCONNECTED);
  SciCall_MarkerDefine(SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNER);
  SciCall_SetFoldFlags(16);

  UpdateLineNumberWidth();

  // Nonprinting characters
  SendMessage(g_hwndEdit,SCI_SETVIEWWS,(bViewWhiteSpace)?SCWS_VISIBLEALWAYS:SCWS_INVISIBLE,0);
  SendMessage(g_hwndEdit,SCI_SETVIEWEOL,bViewEOLs,0);

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

    bIsAppThemed = TRUE;

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
    bIsAppThemed = FALSE;

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

  SetDlgItemInt(hwnd,IDC_REUSELOCK,GetTickCount(),FALSE);

  // Menu
  //SetMenuDefaultItem(GetSubMenu(GetMenu(hwnd),0),0);

  // Drag & Drop
  DragAcceptFiles(hwnd,TRUE);
  pDropTarget = RegisterDragAndDrop(hwnd, &cfDrpF, 1, WM_NULL, DropFilesProc, (void*)g_hwndEdit);

  // File MRU
  pFileMRU = MRU_Create(L"Recent Files",MRU_NOCASE,32);
  MRU_Load(pFileMRU);

  mruFind = MRU_Create(L"Recent Find",(/*IsWindowsNT()*/1) ? MRU_UTF8 : 0,16);
  MRU_Load(mruFind);

  mruReplace = MRU_Create(L"Recent Replace",(/*IsWindowsNT()*/1) ? MRU_UTF8 : 0,16);
  MRU_Load(mruReplace);

  if (g_hwndEdit == NULL || hwndEditFrame == NULL ||
      g_hwndStatus == NULL || g_hwndToolbar == NULL || hwndReBar == NULL)
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
  BOOL bExternalBitmap = FALSE;

  DWORD dwToolbarStyle = WS_TOOLBAR;
  DWORD dwReBarStyle = WS_REBAR;

  BOOL bIsPrivAppThemed = PrivateIsAppThemed();

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
  if (StringCchLenW(tchToolbarBitmap,COUNTOF(tchToolbarBitmap)))
  {
    if (!SearchPath(NULL,tchToolbarBitmap,NULL,COUNTOF(szTmp),szTmp,NULL))
      StringCchCopy(szTmp,COUNTOF(szTmp),tchToolbarBitmap);
    hbmp = LoadImage(NULL,szTmp,IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION|LR_LOADFROMFILE);
  }

  if (hbmp)
    bExternalBitmap = TRUE;
  else {
    LPWSTR toolBarIntRes = (iHighDpiToolBar > 0) ? MAKEINTRESOURCE(IDR_MAINWND2) : MAKEINTRESOURCE(IDR_MAINWND);
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
  if (StringCchLenW(tchToolbarBitmapHot,COUNTOF(tchToolbarBitmapHot)))
  {
    if (!SearchPath(NULL,tchToolbarBitmapHot,NULL,COUNTOF(szTmp),szTmp,NULL))
      StringCchCopy(szTmp,COUNTOF(szTmp),tchToolbarBitmapHot);

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
  if (StringCchLenW(tchToolbarBitmapDisabled,COUNTOF(tchToolbarBitmapDisabled)))
  {
    if (!SearchPath(NULL,tchToolbarBitmapDisabled,NULL,COUNTOF(szTmp),szTmp,NULL))
      StringCchCopy(szTmp,COUNTOF(szTmp),tchToolbarBitmapDisabled);

    hbmp = LoadImage(NULL, szTmp, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
    if (hbmp)
    {
      GetObject(hbmp,sizeof(BITMAP),&bmp);
      himl = ImageList_Create(bmp.bmWidth/NUMTOOLBITMAPS,bmp.bmHeight,ILC_COLOR32|ILC_MASK,0,0);
      ImageList_AddMasked(himl,hbmp,CLR_DEFAULT);
      DeleteObject(hbmp);
      SendMessage(g_hwndToolbar,TB_SETDISABLEDIMAGELIST,0,(LPARAM)himl);
      bExternalBitmap = TRUE;
    }
  }

  if (!bExternalBitmap) {
    BOOL fProcessed = FALSE;
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
  n = 1;
  for (i = 0; i < COUNTOF(tbbMainWnd); i++) {

    if (tbbMainWnd[i].fsStyle == TBSTYLE_SEP)
      continue;

    StringCchPrintf(tchIndex,COUNTOF(tchIndex),L"%02i",n++);

    if (IniSectionGetString(pIniSection,tchIndex,L"",tchDesc,COUNTOF(tchDesc))) {
      tbbMainWnd[i].iString = SendMessage(g_hwndToolbar,TB_ADDSTRING,0,(LPARAM)tchDesc);
      tbbMainWnd[i].fsStyle |= BTNS_AUTOSIZE | BTNS_SHOWTEXT;
    }

    else
      tbbMainWnd[i].fsStyle &= ~(BTNS_AUTOSIZE | BTNS_SHOWTEXT);
  }
  LocalFree(pIniSection);

  SendMessage(g_hwndToolbar,TB_SETEXTENDEDSTYLE,0,
    SendMessage(g_hwndToolbar,TB_GETEXTENDEDSTYLE,0,0) | TBSTYLE_EX_MIXEDBUTTONS);

  SendMessage(g_hwndToolbar,TB_ADDBUTTONS,NUMINITIALTOOLS,(LPARAM)tbbMainWnd);

  if (Toolbar_SetButtons(g_hwndToolbar, IDT_FILE_NEW, tchToolbarButtons, tbbMainWnd, COUNTOF(tbbMainWnd)) == 0) {
    SendMessage(g_hwndToolbar, TB_ADDBUTTONS, NUMINITIALTOOLS, (LPARAM)tbbMainWnd);
  }
  SendMessage(g_hwndToolbar,TB_GETITEMRECT,0,(LPARAM)&rc);
  //SendMessage(g_hwndToolbar,TB_SETINDENT,2,0);

  DWORD dwStatusbarStyle = WS_CHILD | WS_CLIPSIBLINGS;

  if (bShowStatusbar)
    dwStatusbarStyle |= WS_VISIBLE;

  g_hwndStatus = CreateStatusWindow(dwStatusbarStyle,NULL,hwnd,IDC_STATUSBAR);

  // Create ReBar and add Toolbar
  hwndReBar = CreateWindowEx(WS_EX_TOOLWINDOW,REBARCLASSNAME,NULL,dwReBarStyle,
                             0,0,0,0,hwnd,(HMENU)IDC_REBAR,hInstance,NULL);

  rbi.cbSize = sizeof(REBARINFO);
  rbi.fMask  = 0;
  rbi.himl   = (HIMAGELIST)NULL;
  SendMessage(hwndReBar,RB_SETBARINFO,0,(LPARAM)&rbi);

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
  SendMessage(hwndReBar,RB_INSERTBAND,(WPARAM)-1,(LPARAM)&rbBand);

  SetWindowPos(hwndReBar,NULL,0,0,0,0,SWP_NOZORDER);
  GetWindowRect(hwndReBar,&rc);
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
  static BOOL bShutdownOK = FALSE;

  if (!bShutdownOK) {

    // Terminate file watching
    InstallFileWatching(NULL);

    // GetWindowPlacement
    g_WinInfo = GetMyWindowPlacement(hwnd, NULL);

    DragAcceptFiles(hwnd, FALSE);
    RevokeDragAndDrop(pDropTarget);

    // Terminate clipboard watching
    if (flagPasteBoard) {
      KillTimer(hwnd, ID_PASTEBOARDTIMER);
      ChangeClipboardChain(hwnd, hwndNextCBChain);
    }

    // Destroy find / replace dialog
    if (IsWindow(hDlgFindReplace))
      DestroyWindow(hDlgFindReplace);

    // call SaveSettings() when g_hwndToolbar is still valid
    SaveSettings(FALSE);

    if (StringCchLenW(g_wchIniFile,COUNTOF(g_wchIniFile)) != 0) {

      // Cleanup unwanted MRU's
      if (!bSaveRecentFiles) {
        MRU_Empty(pFileMRU);
        MRU_Save(pFileMRU);
      }
      else
        MRU_MergeSave(pFileMRU, TRUE, flagRelativeFileMRU, flagPortableMyDocs);

      MRU_Destroy(pFileMRU);

      if (!bSaveFindReplace) {
        MRU_Empty(mruFind);
        MRU_Empty(mruReplace);
        MRU_Save(mruFind);
        MRU_Save(mruReplace);
      }
      else {
        MRU_MergeSave(mruFind, FALSE, FALSE, FALSE);
        MRU_MergeSave(mruReplace, FALSE, FALSE, FALSE);
      }
      MRU_Destroy(mruFind);
      MRU_Destroy(mruReplace);
    }

    // Remove tray icon if necessary
    ShowNotifyIcon(hwnd, FALSE);

    bShutdownOK = TRUE;
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
  RECT rc, rc2;
  HINSTANCE hInstance = (HINSTANCE)(INT_PTR)GetWindowLongPtr(hwnd,GWLP_HINSTANCE);

  // reinitialize edit frame

  if (PrivateIsAppThemed()) {
    bIsAppThemed = TRUE;

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
    bIsAppThemed = FALSE;

    SetWindowLongPtr(g_hwndEdit,GWL_EXSTYLE,WS_EX_CLIENTEDGE|GetWindowLongPtr(g_hwndEdit,GWL_EXSTYLE));
    SetWindowPos(g_hwndEdit,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);

    cxEditFrame = 0;
    cyEditFrame = 0;
  }

  // recreate toolbar and statusbar
  Toolbar_GetButtons(g_hwndToolbar,IDT_FILE_NEW,tchToolbarButtons,COUNTOF(tchToolbarButtons));

  DestroyWindow(g_hwndToolbar);
  DestroyWindow(hwndReBar);
  DestroyWindow(g_hwndStatus);
  CreateBars(hwnd,hInstance);

  GetClientRect(hwnd,&rc);
  SendMessage(hwnd,WM_SIZE,SIZE_RESTORED,MAKELONG(rc.right,rc.bottom));

  UpdateToolbar();
  UpdateStatusbar();
  UpdateLineNumberWidth();
  EditClearAllMarks(g_hwndEdit, 0, -1);
  MarkAllOccurrences(0);
  EditUpdateUrlHotspots(g_hwndEdit, 0, SciCall_GetTextLength(), bHyperlinkHotspot);
  EditFinalizeStyling(g_hwndEdit, -1);

  UNUSED(lParam);
  UNUSED(wParam);
  UNUSED(hwnd);
}


//=============================================================================
//
//  MsgSize() - Handles WM_SIZE
//
//
void MsgSize(HWND hwnd,WPARAM wParam,LPARAM lParam)
{

  RECT rc;
  int x,y,cx,cy;
  HDWP hdwp;

  if (wParam == SIZE_MINIMIZED)
    return;

  x = 0;
  y = 0;

  cx = LOWORD(lParam);
  cy = HIWORD(lParam);

  if (bShowToolbar)
  {
/*  SendMessage(g_hwndToolbar,WM_SIZE,0,0);
    GetWindowRect(g_hwndToolbar,&rc);
    y = (rc.bottom - rc.top);
    cy -= (rc.bottom - rc.top);*/

    //SendMessage(g_hwndToolbar,TB_GETITEMRECT,0,(LPARAM)&rc);
    SetWindowPos(hwndReBar,NULL,0,0,LOWORD(lParam),cyReBar,SWP_NOZORDER);
    // the ReBar automatically sets the correct height
    // calling SetWindowPos() with the height of one toolbar button
    // causes the control not to temporarily use the whole client area
    // and prevents flickering

    //GetWindowRect(hwndReBar,&rc);
    y = cyReBar + cyReBarFrame;    // define
    cy -= cyReBar + cyReBarFrame;  // border
  }

  if (bShowStatusbar)
  {
    SendMessage(g_hwndStatus,WM_SIZE,0,0);
    GetWindowRect(g_hwndStatus,&rc);
    cy -= (rc.bottom - rc.top);
  }

  hdwp = BeginDeferWindowPos(2);

  DeferWindowPos(hdwp,hwndEditFrame,NULL,x,y,cx,cy,
                 SWP_NOZORDER | SWP_NOACTIVATE);

  DeferWindowPos(hdwp,g_hwndEdit,NULL,x+cxEditFrame,y+cyEditFrame,
                 cx-2*cxEditFrame,cy-2*cyEditFrame,
                 SWP_NOZORDER | SWP_NOACTIVATE);

  EndDeferWindowPos(hdwp);

  // Statusbar width
  int aWidth[7];
  aWidth[STATUS_DOCPOS]   = max(100,min(cx/3, StatusCalcPaneWidth(g_hwndStatus,
    L" Ln 9'999'999 : 9'999'999    Col 9'999'999:999 / 999    Sel 9'999'999 (999 Bytes)    SelLn 9'999'999    Occ 9'999'999 ")));
  aWidth[STATUS_DOCSIZE]  = aWidth[STATUS_DOCPOS] + StatusCalcPaneWidth(g_hwndStatus,L" 9999 Bytes [UTF-8] ");
  aWidth[STATUS_CODEPAGE] = aWidth[STATUS_DOCSIZE] + StatusCalcPaneWidth(g_hwndStatus,L" Unicode (UTF-8) Signature ");
  aWidth[STATUS_EOLMODE]  = aWidth[STATUS_CODEPAGE] + StatusCalcPaneWidth(g_hwndStatus,L" CR+LF ");
  aWidth[STATUS_OVRMODE]  = aWidth[STATUS_EOLMODE] + StatusCalcPaneWidth(g_hwndStatus,L" OVR ");
  aWidth[STATUS_2ND_DEF]  = aWidth[STATUS_OVRMODE] + StatusCalcPaneWidth(g_hwndStatus, L" 2ND ");
  aWidth[STATUS_LEXER] = -1;


  SendMessage(g_hwndStatus,SB_SETPARTS,COUNTOF(aWidth),(LPARAM)aWidth);

  UpdateToolbar();
  UpdateStatusbar();
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
  //bPendingChangeNotify = FALSE;

  if (IsIconic(hwnd))
    ShowWindow(hwnd, SW_RESTORE);

  //SetForegroundWindow(hwnd);

  DragQueryFile(hDrop, 0, szBuf, COUNTOF(szBuf));

  if (PathIsDirectory(szBuf)) {
    WCHAR tchFile[MAX_PATH] = { L'\0' };
    if (OpenFileDlg(g_hwndMain, tchFile, COUNTOF(tchFile), szBuf))
      FileLoad(FALSE, FALSE, FALSE, FALSE, tchFile);
  }
  else if (PathFileExists(szBuf))
    FileLoad(FALSE, FALSE, FALSE, FALSE, szBuf);
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
        FileLoad(FALSE, FALSE, FALSE, FALSE, tchFile);
    }
    else
      FileLoad(FALSE, FALSE, FALSE, FALSE, szBuf);

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
  //bPendingChangeNotify = FALSE;

  SetDlgItemInt(hwnd, IDC_REUSELOCK, GetTickCount(), FALSE);

  if (pcds->dwData == DATA_NOTEPAD3_PARAMS) {
    LPnp3params params = LocalAlloc(LPTR, pcds->cbData);
    CopyMemory(params, pcds->lpData, pcds->cbData);

    if (params->flagLexerSpecified)
      flagLexerSpecified = 1;

    if (params->flagQuietCreate)
      flagQuietCreate = 1;

    if (params->flagFileSpecified) {

      BOOL bOpened = FALSE;
      Encoding_Source(params->iSrcEncoding);

      if (PathIsDirectory(&params->wchData)) {
        WCHAR tchFile[MAX_PATH] = { L'\0' };
        if (OpenFileDlg(g_hwndMain, tchFile, COUNTOF(tchFile), &params->wchData))
          bOpened = FileLoad(FALSE, FALSE, FALSE, FALSE, tchFile);
      }

      else
        bOpened = FileLoad(FALSE, FALSE, FALSE, FALSE, &params->wchData);

      if (bOpened) {

        if (params->flagChangeNotify == 1) {
          iFileWatchingMode = 0;
          bResetFileWatching = TRUE;
          InstallFileWatching(g_wchCurFile);
        }
        else if (params->flagChangeNotify == 2) {
          iFileWatchingMode = 2;
          bResetFileWatching = TRUE;
          InstallFileWatching(g_wchCurFile);
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
          SendMessage(
            g_hwndMain,
            WM_COMMAND,
            MAKELONG(IDM_LINEENDINGS_CRLF + flagSetEOLMode - 1, 1),
            0);
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
      Encoding_Source(CPI_NONE);
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
    UpdateStatusbar();
    UpdateLineNumberWidth();

  }

  UNUSED(wParam);
  return TRUE;
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
  //SetMenuDefaultItem(GetSubMenu(hmenu,1),0,FALSE);

  pt.x = (int)(short)LOWORD(lParam);
  pt.y = (int)(short)HIWORD(lParam);

  switch (nID) {
  case IDC_EDIT:
    {
      if (SendMessage(g_hwndEdit, SCI_GETSELECTIONEMPTY, 0, 0) && (pt.x != -1) && (pt.y != -1)) {
        int iNewPos;
        POINT ptc;
        ptc.x = pt.x;  ptc.y = pt.y;
        ScreenToClient(g_hwndEdit, &ptc);
        iNewPos = (int)SendMessage(g_hwndEdit, SCI_POSITIONFROMPOINT, (WPARAM)ptc.x, (LPARAM)ptc.y);
        EditSelectEx(g_hwndEdit, iNewPos, iNewPos);
      }

      if (pt.x == -1 && pt.y == -1) {
        int iCurrentPos = (int)SendMessage(g_hwndEdit, SCI_GETCURRENTPOS, 0, 0);
        pt.x = (int)SendMessage(g_hwndEdit, SCI_POINTXFROMPOSITION, 0, (LPARAM)iCurrentPos);
        pt.y = (int)SendMessage(g_hwndEdit, SCI_POINTYFROMPOSITION, 0, (LPARAM)iCurrentPos);
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
  if (iFileWatchingMode == 1 || IsDocumentModified || Encoding_HasChanged(CPI_GET))
    SetForegroundWindow(hwnd);

  if (PathFileExists(g_wchCurFile)) {
    if ((iFileWatchingMode == 2 && !IsDocumentModified && !Encoding_HasChanged(CPI_GET)) ||
      MsgBox(MBYESNO,IDS_FILECHANGENOTIFY) == IDYES) {

      FileRevert(g_wchCurFile);
    }
  }
  else {
    if (MsgBox(MBYESNO,IDS_FILECHANGENOTIFY2) == IDYES)
      FileSave(TRUE,FALSE,FALSE,FALSE);
  }

  if (!bRunningWatch)
    InstallFileWatching(g_wchCurFile);

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
      SetMenuDefaultItem(hMenuPopup, IDM_TRAY_RESTORE, FALSE);
      iCmd = TrackPopupMenu(hMenuPopup,
                            TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
                            pt.x, pt.y, 0, hwnd, NULL);

      PostMessage(hwnd, WM_NULL, 0, 0);

      DestroyMenu(hMenu);

      if (iCmd == IDM_TRAY_RESTORE) {
        ShowNotifyIcon(hwnd, FALSE);
        RestoreWndFromTray(hwnd);
        ShowOwnedPopups(hwnd, TRUE);
      }

      else if (iCmd == IDM_TRAY_EXIT) {
        //ShowNotifyIcon(hwnd,FALSE);
        SendMessage(hwnd, WM_CLOSE, 0, 0);
      }
    }
    return TRUE;

  case WM_LBUTTONUP:
    ShowNotifyIcon(hwnd, FALSE);
    RestoreWndFromTray(hwnd);
    ShowOwnedPopups(hwnd, TRUE);
    return TRUE;
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

  int i;
  DocPos p;
  BOOL b;

  HMENU hmenu = (HMENU)wParam;

  i = StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile));
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
  CheckCmd(hmenu,IDM_FILE_READONLY,bReadOnly);

  //EnableCmd(hmenu,IDM_ENCODING_UNICODEREV,!bReadOnly);
  //EnableCmd(hmenu,IDM_ENCODING_UNICODE,!bReadOnly);
  //EnableCmd(hmenu,IDM_ENCODING_UTF8SIGN,!bReadOnly);
  //EnableCmd(hmenu,IDM_ENCODING_UTF8,!bReadOnly);
  //EnableCmd(hmenu,IDM_ENCODING_ANSI,!bReadOnly);
  //EnableCmd(hmenu,IDM_LINEENDINGS_CRLF,!bReadOnly);
  //EnableCmd(hmenu,IDM_LINEENDINGS_LF,!bReadOnly);
  //EnableCmd(hmenu,IDM_LINEENDINGS_CR,!bReadOnly);

  EnableCmd(hmenu,IDM_ENCODING_RECODE,i);

  if (g_Encodings[Encoding_Current(CPI_GET)].uFlags & NCP_UNICODE_REVERSE)
    i = IDM_ENCODING_UNICODEREV;
  else if (g_Encodings[Encoding_Current(CPI_GET)].uFlags & NCP_UNICODE)
    i = IDM_ENCODING_UNICODE;
  else if (g_Encodings[Encoding_Current(CPI_GET)].uFlags & NCP_UTF8_SIGN)
    i = IDM_ENCODING_UTF8SIGN;
  else if (g_Encodings[Encoding_Current(CPI_GET)].uFlags & NCP_UTF8)
    i = IDM_ENCODING_UTF8;
  else if (g_Encodings[Encoding_Current(CPI_GET)].uFlags & NCP_ANSI)
    i = IDM_ENCODING_ANSI;
  else
    i = -1;
  CheckMenuRadioItem(hmenu,IDM_ENCODING_ANSI,IDM_ENCODING_UTF8SIGN,i,MF_BYCOMMAND);

  if (iEOLMode == SC_EOL_CRLF)
    i = IDM_LINEENDINGS_CRLF;
  else if (iEOLMode == SC_EOL_LF)
    i = IDM_LINEENDINGS_LF;
  else
    i = IDM_LINEENDINGS_CR;
  CheckMenuRadioItem(hmenu,IDM_LINEENDINGS_CRLF,IDM_LINEENDINGS_CR,i,MF_BYCOMMAND);

  EnableCmd(hmenu,IDM_FILE_RECENT,(MRU_Enum(pFileMRU,0,NULL,0) > 0));

  EnableCmd(hmenu,IDM_EDIT_UNDO,SendMessage(g_hwndEdit,SCI_CANUNDO,0,0) /*&& !bReadOnly*/);
  EnableCmd(hmenu,IDM_EDIT_REDO,SendMessage(g_hwndEdit,SCI_CANREDO,0,0) /*&& !bReadOnly*/);


  i = !SciCall_IsSelectionEmpty();
  p = SciCall_GetTextLength();
  b = (BOOL)SendMessage(g_hwndEdit, SCI_CANPASTE, 0, 0);
  
  EnableCmd(hmenu,IDM_EDIT_CUT,p /*&& !bReadOnly*/);       // allow Ctrl-X w/o selection
  EnableCmd(hmenu,IDM_EDIT_COPY,p /*&& !bReadOnly*/);      // allow Ctrl-C w/o selection

  EnableCmd(hmenu,IDM_EDIT_COPYALL,p /*&& !bReadOnly*/);
  EnableCmd(hmenu,IDM_EDIT_COPYADD,i /*&& !bReadOnly*/);

  EnableCmd(hmenu,IDM_EDIT_PASTE,b /*&& !bReadOnly*/);
  EnableCmd(hmenu,IDM_EDIT_SWAP,i || b /*&& !bReadOnly*/);
  EnableCmd(hmenu,IDM_EDIT_CLEAR,i /*&& !bReadOnly*/);

  EnableCmd(hmenu, IDM_EDIT_SELECTALL, p /*&& !bReadOnly*/);
  

  OpenClipboard(hwnd);
  EnableCmd(hmenu,IDM_EDIT_CLEARCLIPBOARD,CountClipboardFormats());
  CloseClipboard();

  //EnableCmd(hmenu,IDM_EDIT_MOVELINEUP,!bReadOnly);
  //EnableCmd(hmenu,IDM_EDIT_MOVELINEDOWN,!bReadOnly);
  //EnableCmd(hmenu,IDM_EDIT_DUPLICATELINE,!bReadOnly);
  //EnableCmd(hmenu,IDM_EDIT_CUTLINE,!bReadOnly);
  //EnableCmd(hmenu,IDM_EDIT_COPYLINE,!bReadOnly);
  //EnableCmd(hmenu,IDM_EDIT_DELETELINE,!bReadOnly);

  //EnableCmd(hmenu,IDM_EDIT_INDENT,i /*&& !bReadOnly*/);
  //EnableCmd(hmenu,IDM_EDIT_UNINDENT,i /*&& !bReadOnly*/);

  //EnableCmd(hmenu,IDM_EDIT_PADWITHSPACES,!bReadOnly);
  //EnableCmd(hmenu,IDM_EDIT_STRIP1STCHAR,!bReadOnly);
  //EnableCmd(hmenu,IDM_EDIT_STRIPLASTCHAR,!bReadOnly);
  //EnableCmd(hmenu,IDM_EDIT_TRIMLINES,!bReadOnly);
  //EnableCmd(hmenu,IDM_EDIT_MERGEBLANKLINES,!bReadOnly);
  //EnableCmd(hmenu,IDM_EDIT_REMOVEBLANKLINES,!bReadOnly);

  EnableCmd(hmenu, IDM_EDIT_SORTLINES,
    (SciCall_LineFromPosition(SciCall_GetSelectionEnd()) - 
      SciCall_LineFromPosition(SciCall_GetSelectionStart())) >= 1);

  //EnableCmd(hmenu,IDM_EDIT_COLUMNWRAP,i /*&& IsWindowsNT()*/);
  EnableCmd(hmenu,IDM_EDIT_SPLITLINES,i /*&& !bReadOnly*/);
  EnableCmd(hmenu,IDM_EDIT_JOINLINES,i /*&& !bReadOnly*/);
  EnableCmd(hmenu, IDM_EDIT_JOINLN_NOSP,i /*&& !bReadOnly*/);
  EnableCmd(hmenu,IDM_EDIT_JOINLINES_PARA,i /*&& !bReadOnly*/);

  EnableCmd(hmenu,IDM_EDIT_CONVERTUPPERCASE,i /*&& !bReadOnly*/);
  EnableCmd(hmenu,IDM_EDIT_CONVERTLOWERCASE,i /*&& !bReadOnly*/);
  EnableCmd(hmenu,IDM_EDIT_INVERTCASE,i /*&& !bReadOnly*/ /*&& IsWindowsNT()*/);
  EnableCmd(hmenu,IDM_EDIT_TITLECASE,i /*&& !bReadOnly*/ /*&& IsWindowsNT()*/);
  EnableCmd(hmenu,IDM_EDIT_SENTENCECASE,i /*&& !bReadOnly*/ /*&& IsWindowsNT()*/);

  EnableCmd(hmenu,IDM_EDIT_CONVERTTABS,i /*&& !bReadOnly*/);
  EnableCmd(hmenu,IDM_EDIT_CONVERTSPACES,i /*&& !bReadOnly*/);
  EnableCmd(hmenu,IDM_EDIT_CONVERTTABS2,i /*&& !bReadOnly*/);
  EnableCmd(hmenu,IDM_EDIT_CONVERTSPACES2,i /*&& !bReadOnly*/);

  EnableCmd(hmenu,IDM_EDIT_URLENCODE,i /*&& !bReadOnly*/);
  EnableCmd(hmenu,IDM_EDIT_URLDECODE,i /*&& !bReadOnly*/);

  EnableCmd(hmenu,IDM_EDIT_ESCAPECCHARS,i /*&& !bReadOnly*/);
  EnableCmd(hmenu,IDM_EDIT_UNESCAPECCHARS,i /*&& !bReadOnly*/);

  EnableCmd(hmenu,IDM_EDIT_CHAR2HEX,TRUE /*&& !bReadOnly*/);  // Char2Hex allowed for char after curr pos
  EnableCmd(hmenu,IDM_EDIT_HEX2CHAR,i /*&& !bReadOnly*/);

  //EnableCmd(hmenu,IDM_EDIT_INCREASENUM,i /*&& !bReadOnly*/);
  //EnableCmd(hmenu,IDM_EDIT_DECREASENUM,i /*&& !bReadOnly*/);

  EnableCmd(hmenu,IDM_VIEW_SHOWEXCERPT,i);

  i = (int)SendMessage(g_hwndEdit,SCI_GETLEXER,0,0);
  EnableCmd(hmenu,IDM_EDIT_LINECOMMENT,
    !(i == SCLEX_NULL || i == SCLEX_CSS || i == SCLEX_DIFF || i == SCLEX_MARKDOWN || i == SCLEX_JSON));
  EnableCmd(hmenu,IDM_EDIT_STREAMCOMMENT,
    !(i == SCLEX_NULL || i == SCLEX_VBSCRIPT || i == SCLEX_MAKEFILE || i == SCLEX_VB || i == SCLEX_ASM ||
      i == SCLEX_SQL || i == SCLEX_PERL || i == SCLEX_PYTHON || i == SCLEX_PROPERTIES ||i == SCLEX_CONF ||
      i == SCLEX_POWERSHELL || i == SCLEX_BATCH || i == SCLEX_DIFF || i == SCLEX_BASH || i == SCLEX_TCL ||
      i == SCLEX_AU3 || i == SCLEX_LATEX || i == SCLEX_AHK || i == SCLEX_RUBY || i == SCLEX_CMAKE || i == SCLEX_MARKDOWN ||
      i == SCLEX_YAML || i == SCLEX_REGISTRY || i == SCLEX_NIMROD));

  EnableCmd(hmenu,IDM_EDIT_INSERT_ENCODING,*g_Encodings[Encoding_Current(CPI_GET)].pszParseNames);

  //EnableCmd(hmenu,IDM_EDIT_INSERT_SHORTDATE,!bReadOnly);
  //EnableCmd(hmenu,IDM_EDIT_INSERT_LONGDATE,!bReadOnly);
  //EnableCmd(hmenu,IDM_EDIT_INSERT_FILENAME,!bReadOnly);
  //EnableCmd(hmenu,IDM_EDIT_INSERT_PATHNAME,!bReadOnly);

  i = (int)SendMessage(g_hwndEdit,SCI_GETLENGTH,0,0);
  EnableCmd(hmenu,IDM_EDIT_FIND,i);
  EnableCmd(hmenu,IDM_EDIT_SAVEFIND,i);
  EnableCmd(hmenu,IDM_EDIT_FINDNEXT,i);
  EnableCmd(hmenu,IDM_EDIT_FINDPREV,i && strlen(g_efrData.szFind));
  EnableCmd(hmenu,IDM_EDIT_REPLACE,i /*&& !bReadOnly*/);
  EnableCmd(hmenu,IDM_EDIT_REPLACENEXT,i);
  EnableCmd(hmenu,IDM_EDIT_SELTONEXT,i && strlen(g_efrData.szFind));
  EnableCmd(hmenu,IDM_EDIT_SELTOPREV,i && strlen(g_efrData.szFind));
  EnableCmd(hmenu,IDM_EDIT_FINDMATCHINGBRACE,i);
  EnableCmd(hmenu,IDM_EDIT_SELTOMATCHINGBRACE,i);

  EnableCmd(hmenu,BME_EDIT_BOOKMARKPREV,i);
  EnableCmd(hmenu,BME_EDIT_BOOKMARKNEXT,i);
  EnableCmd(hmenu,BME_EDIT_BOOKMARKTOGGLE,i);
  EnableCmd(hmenu,BME_EDIT_BOOKMARKCLEAR,i);

  EnableCmd(hmenu, IDM_EDIT_DELETELINELEFT, i);
  EnableCmd(hmenu, IDM_EDIT_DELETELINERIGHT, i);
  EnableCmd(hmenu, CMD_CTRLBACK, i);
  EnableCmd(hmenu, CMD_CTRLDEL, i);
  EnableCmd(hmenu, CMD_TIMESTAMPS, i);

  EnableCmd(hmenu,IDM_VIEW_TOGGLEFOLDS,i && bShowCodeFolding);
  CheckCmd(hmenu,IDM_VIEW_FOLDING,bShowCodeFolding);

  CheckCmd(hmenu,IDM_VIEW_USE2NDDEFAULT,Style_GetUse2ndDefault());

  CheckCmd(hmenu,IDM_VIEW_WORDWRAP,bWordWrap);
  CheckCmd(hmenu,IDM_VIEW_LONGLINEMARKER,bMarkLongLines);
  CheckCmd(hmenu,IDM_VIEW_TABSASSPACES,g_bTabsAsSpaces);
  CheckCmd(hmenu,IDM_VIEW_SHOWINDENTGUIDES,bShowIndentGuides);
  CheckCmd(hmenu,IDM_VIEW_AUTOINDENTTEXT,bAutoIndent);
  CheckCmd(hmenu,IDM_VIEW_LINENUMBERS,bShowLineNumbers);
  CheckCmd(hmenu,IDM_VIEW_MARGIN,bShowSelectionMargin);

  EnableCmd(hmenu,IDM_EDIT_COMPLETEWORD,i);
  CheckCmd(hmenu,IDM_VIEW_AUTOCOMPLETEWORDS,bAutoCompleteWords);
  CheckCmd(hmenu,IDM_VIEW_ACCELWORDNAV,bAccelWordNavigation);

  CheckCmd(hmenu, IDM_VIEW_MARKOCCUR_ONOFF, iMarkOccurrences != 0);
  CheckCmd(hmenu, IDM_VIEW_MARKOCCUR_VISIBLE, bMarkOccurrencesMatchVisible);
  CheckCmd(hmenu, IDM_VIEW_MARKOCCUR_CASE, bMarkOccurrencesMatchCase);

  if (bMarkOccurrencesMatchWords)
    i = IDM_VIEW_MARKOCCUR_WORD;
  else if (bMarkOccurrencesCurrentWord)
    i = IDM_VIEW_MARKOCCUR_CURRENT;
  else
    i = IDM_VIEW_MARKOCCUR_WNONE;

  CheckMenuRadioItem(hmenu, IDM_VIEW_MARKOCCUR_WNONE, IDM_VIEW_MARKOCCUR_CURRENT, i, MF_BYCOMMAND);
  CheckCmdPos(GetSubMenu(GetSubMenu(GetMenu(g_hwndMain), 2), 17), 5, (i != IDM_VIEW_MARKOCCUR_WNONE));

  i = (int)(iMarkOccurrences != 0);
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
  CheckCmd(hmenu,IDM_VIEW_AUTOCLOSETAGS,bAutoCloseTags /*&& (i == SCLEX_HTML || i == SCLEX_XML)*/);
  CheckCmd(hmenu, IDM_VIEW_HILITECURRENTLINE, bHiliteCurrentLine);
  CheckCmd(hmenu, IDM_VIEW_HYPERLINKHOTSPOTS, bHyperlinkHotspot);

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

  CheckCmd(hmenu,IDM_VIEW_CHANGENOTIFY,iFileWatchingMode);

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

  i = StringCchLenW(g_wchIniFile,COUNTOF(g_wchIniFile));
  CheckCmd(hmenu,IDM_VIEW_SAVESETTINGS,bSaveSettings && i);

  EnableCmd(hmenu,IDM_VIEW_REUSEWINDOW,i);
  EnableCmd(hmenu,IDM_VIEW_STICKYWINPOS,i);
  EnableCmd(hmenu,IDM_VIEW_SINGLEFILEINSTANCE,i);
  EnableCmd(hmenu,IDM_VIEW_NOSAVERECENT,i);
  EnableCmd(hmenu,IDM_VIEW_NOPRESERVECARET,i);
  EnableCmd(hmenu,IDM_VIEW_NOSAVEFINDREPL,i);
  EnableCmd(hmenu,IDM_VIEW_SAVESETTINGS,bEnableSaveSettings && i);

  i = (StringCchLenW(g_wchIniFile,COUNTOF(g_wchIniFile)) > 0 || StringCchLenW(g_wchIniFile2,COUNTOF(g_wchIniFile2)) > 0);
  EnableCmd(hmenu,IDM_VIEW_SAVESETTINGSNOW,bEnableSaveSettings && i);

  BOOL bIsHLink = FALSE;
  if ((BOOL)SendMessage(g_hwndEdit, SCI_STYLEGETHOTSPOT, Style_GetHotspotStyleID(), 0)) 
  {
    bIsHLink = (int)SendMessage(g_hwndEdit, SCI_GETSTYLEAT, SendMessage(g_hwndEdit, SCI_GETCURRENTPOS, 0, 0), 0) == Style_GetHotspotStyleID();
  }
  EnableCmd(hmenu, CMD_OPEN_HYPERLINK, bIsHLink);

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
    ShowOwnedPopups(hwnd, FALSE);
    if (bMinimizeToTray) {
      MinimizeWndToTray(hwnd);
      ShowNotifyIcon(hwnd, TRUE);
      SetNotifyIconTitle(hwnd);
      return(0);
    }
    else
      return DefWindowProc(hwnd, umsg, wParam, lParam);

  case SC_RESTORE:
    {
      LRESULT lrv = DefWindowProc(hwnd, umsg, wParam, lParam);
      ShowOwnedPopups(hwnd, TRUE);
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
    case IDC_MAIN_MARKALL_OCC:
      EditMarkAllOccurrences();
      break;

    case IDC_CALL_UPDATE_HOTSPOT:
      EditUpdateVisibleUrlHotspot(bHyperlinkHotspot);
      break;

    case IDM_FILE_NEW:
      FileLoad(FALSE,TRUE,FALSE,FALSE,L"");
      break;


    case IDM_FILE_OPEN:
      FileLoad(FALSE,FALSE,FALSE,FALSE,L"");
      break;


    case IDM_FILE_REVERT:
      if ((IsDocumentModified || Encoding_HasChanged(CPI_GET)) && MsgBox(MBOKCANCEL,IDS_ASK_REVERT) != IDOK) {
        return(0);
      }
      FileRevert(g_wchCurFile);
      break;


    case IDM_FILE_SAVE:
      FileSave(TRUE,FALSE,FALSE,FALSE);
      break;


    case IDM_FILE_SAVEAS:
      FileSave(TRUE,FALSE,TRUE,FALSE);
      break;


    case IDM_FILE_SAVECOPY:
      FileSave(TRUE,FALSE,TRUE,TRUE);
      break;


    case IDM_FILE_READONLY:
      //bReadOnly = (bReadOnly) ? FALSE : TRUE;
      //SendMessage(g_hwndEdit,SCI_SETREADONLY,bReadOnly,0);
      //UpdateToolbar();
      //UpdateStatusbar();

      if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile)))
      {
        DWORD dwFileAttributes = GetFileAttributes(g_wchCurFile);
        if (dwFileAttributes != INVALID_FILE_ATTRIBUTES) {
          if (bReadOnly)
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
          bReadOnly = (dwFileAttributes & FILE_ATTRIBUTE_READONLY);

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

        if (bSaveBeforeRunningTools && !FileSave(FALSE,TRUE,FALSE,FALSE))
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

        if (bSaveBeforeRunningTools && !FileSave(FALSE,TRUE,FALSE,FALSE))
          break;

        StringCchCopy(tchCmdLine,COUNTOF(tchCmdLine),g_wchCurFile);
        PathQuoteSpaces(tchCmdLine);

        RunDlg(hwnd,tchCmdLine);
      }
      break;


    case IDM_FILE_OPENWITH:
      if (bSaveBeforeRunningTools && !FileSave(FALSE,TRUE,FALSE,FALSE))
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
          SHGetFileInfo2(g_wchCurFile,0,&shfi,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME);
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
      if (FileSave(FALSE,TRUE,FALSE,FALSE)) {

        WCHAR tchSelItem[MAX_PATH] = { L'\0' };

        if (FavoritesDlg(hwnd,tchSelItem))
        {
          if (PathIsLnkToDirectory(tchSelItem,NULL,0))
            PathGetLnkPath(tchSelItem,tchSelItem,COUNTOF(tchSelItem));

          if (PathIsDirectory(tchSelItem))
          {
            WCHAR tchFile[MAX_PATH] = { L'\0' };

            if (OpenFileDlg(g_hwndMain,tchFile,COUNTOF(tchFile),tchSelItem))
              FileLoad(TRUE,FALSE,FALSE,FALSE,tchFile);
          }
          else
            FileLoad(TRUE,FALSE,FALSE,FALSE,tchSelItem);
          }
        }
      break;


    case IDM_FILE_ADDTOFAV:
      if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
        SHFILEINFO shfi;
        SHGetFileInfo2(g_wchCurFile,0,&shfi,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME);
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
        sei.lpFile = tchFavoritesDir;
        sei.lpParameters = NULL;
        sei.lpDirectory = NULL;
        sei.nShow = SW_SHOWNORMAL;
        // Run favorites directory
        ShellExecuteEx(&sei);
      }
      break;


    case IDM_FILE_RECENT:
      if (MRU_Enum(pFileMRU,0,NULL,0) > 0) {
        if (FileSave(FALSE,TRUE,FALSE,FALSE)) {
          WCHAR tchFile[MAX_PATH] = { L'\0' };
          if (FileMRUDlg(hwnd,tchFile))
            FileLoad(TRUE,FALSE,FALSE,FALSE,tchFile);
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

          UpdateToolbar();
          UpdateStatusbar();
        }
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
            Encoding_Source(iNewEncoding);
            FileLoad(TRUE,FALSE,TRUE,FALSE,tchCurFile2);
          }
        }
      }
      break;


    case IDM_ENCODING_SETDEFAULT:
      SelectDefEncodingDlg(hwnd,&iDefaultEncoding);
      break;


    case IDM_LINEENDINGS_CRLF:
    case IDM_LINEENDINGS_LF:
    case IDM_LINEENDINGS_CR:
      {
        int iNewEOLMode = iLineEndings[LOWORD(wParam)-IDM_LINEENDINGS_CRLF];
        iEOLMode = iNewEOLMode;
        SendMessage(g_hwndEdit,SCI_SETEOLMODE,iEOLMode,0);
        SendMessage(g_hwndEdit,SCI_CONVERTEOLS,iEOLMode,0);
        EditFixPositions(g_hwndEdit);
        UpdateToolbar();
        UpdateStatusbar();
      }
      break;


    case IDM_LINEENDINGS_SETDEFAULT:
      SelectDefLineEndingDlg(hwnd,&iDefaultEOLMode);
      break;


    case IDM_EDIT_UNDO:
      IgnoreNotifyChangeEvent();
      SendMessage(g_hwndEdit, SCI_UNDO, 0, 0);
      ObserveNotifyChangeEvent();
      break;


    case IDM_EDIT_REDO:
      IgnoreNotifyChangeEvent();
      SendMessage(g_hwndEdit, SCI_REDO, 0, 0);
      ObserveNotifyChangeEvent();
      break;


    case IDM_EDIT_CUT:
      {
        if (flagPasteBoard)
          bLastCopyFromMe = TRUE;

        int token = BeginUndoAction();
        if (!SciCall_IsSelectionEmpty())
        {
          SendMessage(g_hwndEdit, SCI_CUT, 0, 0);
        }
        else { // VisualStudio behavior
          SendMessage(g_hwndEdit, SCI_COPYALLOWLINE, 0, 0);
          SendMessage(g_hwndEdit, SCI_LINEDELETE, 0, 0);   
        }
        EndUndoAction(token);
        UpdateToolbar();
      }
      break;


    case IDM_EDIT_COPY:
      {
        if (flagPasteBoard)
          bLastCopyFromMe = TRUE;
        int token = BeginUndoAction();
        SendMessage(g_hwndEdit, SCI_COPYALLOWLINE, 0, 0);
        EndUndoAction(token);
        UpdateToolbar();
      }
      break;


    case IDM_EDIT_COPYALL:
      {
        if (flagPasteBoard)
          bLastCopyFromMe = TRUE;
        int token = BeginUndoAction();
        SendMessage(g_hwndEdit,SCI_COPYRANGE,0,(LPARAM)SciCall_GetTextLength());
        EndUndoAction(token);
        UpdateToolbar();
      }
      break;


    case IDM_EDIT_COPYADD:
      {
        if (flagPasteBoard)
          bLastCopyFromMe = TRUE;
        int token = BeginUndoAction();
        EditCopyAppend(g_hwndEdit);
        EndUndoAction(token);
        UpdateToolbar();
      }
      break;

    case IDM_EDIT_PASTE:
      {
        if (flagPasteBoard)
          bLastCopyFromMe = TRUE;
        int token = BeginUndoAction();
        EditPasteClipboard(g_hwndEdit, FALSE);
        EndUndoAction(token);
        UpdateToolbar();
        UpdateStatusbar();
      }
      break;

    case IDM_EDIT_SWAP:
      {
        if (flagPasteBoard)
          bLastCopyFromMe = TRUE;
        int token = BeginUndoAction();
        EditPasteClipboard(g_hwndEdit, TRUE);
        EndUndoAction(token);
        UpdateToolbar();
        UpdateStatusbar();
      }
      break;

    case IDM_EDIT_CLEARCLIPBOARD:
      SciClearClipboard();
      UpdateToolbar();
      UpdateStatusbar();
      break;


    case IDM_EDIT_SELECTALL:
        SendMessage(g_hwndEdit,SCI_SELECTALL,0,0);
      break;


    case IDM_EDIT_SELECTWORD:
      {
        int iPos = (int)SendMessage(g_hwndEdit,SCI_GETCURRENTPOS,0,0);

        if (SendMessage(g_hwndEdit, SCI_GETSELECTIONEMPTY, 0, 0)) {

          int iWordStart = (int)SendMessage(g_hwndEdit,SCI_WORDSTARTPOSITION,iPos,TRUE);
          int iWordEnd   = (int)SendMessage(g_hwndEdit,SCI_WORDENDPOSITION,iPos,TRUE);

          if (iWordStart == iWordEnd) // we are in whitespace salad...
          {
            iWordStart = (int)SendMessage(g_hwndEdit,SCI_WORDENDPOSITION,iPos,FALSE);
            iWordEnd   = (int)SendMessage(g_hwndEdit,SCI_WORDENDPOSITION,iWordStart,TRUE);
            if (iWordStart != iWordEnd) {
              //if (SCLEX_HTML == SendMessage(g_hwndEdit,SCI_GETLEXER,0,0) &&
              //    SCE_HPHP_VARIABLE == SendMessage(g_hwndEdit,SCI_GETSTYLEAT,(WPARAM)iWordStart,0) &&
              //    '$' == (char)SendMessage(g_hwndEdit,SCI_GETCHARAT,(WPARAM)iWordStart-1,0))
              //  iWordStart--;
              SendMessage(g_hwndEdit,SCI_SETSEL,iWordStart,iWordEnd);
            }
          }
          else {
            //if (SCLEX_HTML == SendMessage(g_hwndEdit,SCI_GETLEXER,0,0) &&
            //    SCE_HPHP_VARIABLE == SendMessage(g_hwndEdit,SCI_GETSTYLEAT,(WPARAM)iWordStart,0) &&
            //    '$' == (char)SendMessage(g_hwndEdit,SCI_GETCHARAT,(WPARAM)iWordStart-1,0))
            //  iWordStart--;
            SendMessage(g_hwndEdit,SCI_SETSEL,iWordStart,iWordEnd);
          }

          if (SendMessage(g_hwndEdit, SCI_GETSELECTIONEMPTY, 0, 0)) {
            int iLine = (int)SendMessage(g_hwndEdit,SCI_LINEFROMPOSITION,iPos,0);
            int iLineStart = (int)SendMessage(g_hwndEdit,SCI_GETLINEINDENTPOSITION,iLine,0);
            int iLineEnd   = (int)SendMessage(g_hwndEdit,SCI_GETLINEENDPOSITION,iLine,0);
            SendMessage(g_hwndEdit,SCI_SETSEL,iLineStart,iLineEnd);
          }
        }
        else {
          int iLine = (int)SendMessage(g_hwndEdit,SCI_LINEFROMPOSITION,iPos,0);
          int iLineStart = (int)SendMessage(g_hwndEdit,SCI_GETLINEINDENTPOSITION,iLine,0);
          int iLineEnd   = (int)SendMessage(g_hwndEdit,SCI_GETLINEENDPOSITION,iLine,0);
          SendMessage(g_hwndEdit,SCI_SETSEL,iLineStart,iLineEnd);
        }
      }
      break;


    case IDM_EDIT_SELECTLINE:
      {
        int iSelStart  = (int)SendMessage(g_hwndEdit,SCI_GETSELECTIONSTART,0,0);
        int iSelEnd    = (int)SendMessage(g_hwndEdit,SCI_GETSELECTIONEND,0,0);
        int iLineStart = (int)SendMessage(g_hwndEdit,SCI_LINEFROMPOSITION,iSelStart,0);
        int iLineEnd   = (int)SendMessage(g_hwndEdit,SCI_LINEFROMPOSITION,iSelEnd,0);
        iSelStart = (int)SendMessage(g_hwndEdit,SCI_POSITIONFROMLINE,iLineStart,0);
        iSelEnd   = (int)SendMessage(g_hwndEdit,SCI_POSITIONFROMLINE,iLineEnd+1,0);
        SendMessage(g_hwndEdit,SCI_SETSEL,iSelStart,iSelEnd);
        SendMessage(g_hwndEdit,SCI_CHOOSECARETX,0,0);
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
          bLastCopyFromMe = TRUE;
        int token = BeginUndoAction();
        SendMessage(g_hwndEdit,SCI_LINECUT,0,0);
        UpdateToolbar();
        EndUndoAction(token);
      }
      break;


    case IDM_EDIT_COPYLINE:
      if (flagPasteBoard)
        bLastCopyFromMe = TRUE;
      SendMessage(g_hwndEdit,SCI_LINECOPY,0,0);
      UpdateToolbar();
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
        EditIndentBlock(g_hwndEdit, SCI_TAB, TRUE);
        EndUndoAction(token);
      }
      break;

    case IDM_EDIT_UNINDENT:
      {
        int token = BeginUndoAction();
        EditIndentBlock(g_hwndEdit, SCI_BACKTAB, TRUE);
        EndUndoAction(token);
      }
      break;

    case CMD_TAB:
      {
        int token = BeginUndoAction();
        EditIndentBlock(g_hwndEdit, SCI_TAB, FALSE);
        EndUndoAction(token);
      }
      break;

    case CMD_BACKTAB:
      {
        int token = BeginUndoAction();
        EditIndentBlock(g_hwndEdit, SCI_BACKTAB, FALSE);
        EndUndoAction(token);
      }
      break;

    case CMD_CTRLTAB:
      {
        int token = BeginUndoAction();
        SendMessage(g_hwndEdit, SCI_SETUSETABS, TRUE, 0);
        SendMessage(g_hwndEdit, SCI_SETTABINDENTS, FALSE, 0);
        EditIndentBlock(g_hwndEdit, SCI_TAB, FALSE);
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
        EditPadWithSpaces(g_hwndEdit,FALSE,FALSE);
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
        EditStripLastCharacter(g_hwndEdit, FALSE, FALSE);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_TRIMLINES:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditStripLastCharacter(g_hwndEdit, FALSE, TRUE);
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
        EditRemoveBlankLines(g_hwndEdit,TRUE);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_REMOVEBLANKLINES:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditRemoveBlankLines(g_hwndEdit,FALSE);
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
          iWrapCol = iLongLinesLimit;
        }
        if (ColumnWrapDlg(hwnd,IDD_COLUMNWRAP,&iWrapCol))
        {
          iWrapCol = max(min(iWrapCol,512),1);
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
        EditEnterTargetTransaction();
        SciCall_TargetFromSelection();
        SendMessage(g_hwndEdit,SCI_LINESSPLIT,0,0);
        EditLeaveTargetTransaction();
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_JOINLINES:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditJoinLinesEx(g_hwndEdit, FALSE, TRUE);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;

    case IDM_EDIT_JOINLN_NOSP:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditJoinLinesEx(g_hwndEdit, FALSE, FALSE);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;

    case IDM_EDIT_JOINLINES_PARA:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditJoinLinesEx(g_hwndEdit, TRUE, TRUE);
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
        EditTabsToSpaces(g_hwndEdit, g_iTabWidth, FALSE);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_CONVERTSPACES:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditSpacesToTabs(g_hwndEdit, g_iTabWidth, FALSE);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_CONVERTTABS2:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditTabsToSpaces(g_hwndEdit, g_iTabWidth, TRUE);
        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_CONVERTSPACES2:
      {
        BeginWaitCursor(NULL);
        int token = BeginUndoAction();
        EditSpacesToTabs(g_hwndEdit, g_iTabWidth, TRUE);
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
        if (*g_Encodings[Encoding_Current(CPI_GET)].pszParseNames) {
          char msz[32] = { '\0' };
          //int iSelStart;
          StringCchCopyNA(msz,COUNTOF(msz),g_Encodings[Encoding_Current(CPI_GET)].pszParseNames,COUNTOF(msz));
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

        UINT uCP = Encoding_SciGetCodePage(g_hwndEdit);
        WideCharToMultiByteStrg(uCP,tchDateTime,mszBuf);
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
            SHGetFileInfo2(g_wchCurFile,0,&shfi,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME);
            pszInsert = shfi.szDisplayName;
          }
          else
            pszInsert = g_wchCurFile;
        }

        else {
          GetString(IDS_UNTITLED,tchUntitled,COUNTOF(tchUntitled));
          pszInsert = tchUntitled;
        }

        UINT uCP = Encoding_SciGetCodePage(g_hwndEdit);
        WideCharToMultiByteStrg(uCP,pszInsert,mszBuf);
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
            UINT uCP = Encoding_SciGetCodePage(g_hwndEdit);
            char mszGuid[40 * 4]; // UTF-8 max of 4 bytes per char
            if (WideCharToMultiByteStrg(uCP,pwszGuid,mszGuid)) {
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
          EditToggleLineComments(g_hwndEdit, L"//", FALSE);
          break;
        case SCLEX_VBSCRIPT:
        case SCLEX_VB:
          EditToggleLineComments(g_hwndEdit, L"'", FALSE);
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
          EditToggleLineComments(g_hwndEdit, L"#", TRUE);
          break;
        case SCLEX_ASM:
        case SCLEX_PROPERTIES:
        case SCLEX_AU3:
        case SCLEX_AHK:
        case SCLEX_NSIS: // # could also be used instead
        case SCLEX_INNOSETUP:
          EditToggleLineComments(g_hwndEdit, L";", TRUE);
          break;
        case SCLEX_REGISTRY:
          EditToggleLineComments(g_hwndEdit, L";;", TRUE);
          break;
        case SCLEX_SQL:
        case SCLEX_LUA:
        case SCLEX_VHDL:
          EditToggleLineComments(g_hwndEdit, L"--", TRUE);
          break;
        case SCLEX_BATCH:
          EditToggleLineComments(g_hwndEdit, L"rem ", TRUE);
          break;
        case SCLEX_LATEX:
        case SCLEX_MATLAB:
          EditToggleLineComments(g_hwndEdit, L"%", TRUE);
          break;
        }

        EndUndoAction(token);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_STREAMCOMMENT:
      {
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


    case IDM_EDIT_FIND:
      if (!IsWindow(hDlgFindReplace))
        hDlgFindReplace = EditFindReplaceDlg(g_hwndEdit,&g_efrData,FALSE);
      else {
        if (GetDlgItem(hDlgFindReplace,IDC_REPLACE)) {
          SendMessage(hDlgFindReplace,WM_COMMAND,MAKELONG(IDMSG_SWITCHTOFIND,1),0);
          DestroyWindow(hDlgFindReplace);
          hDlgFindReplace = EditFindReplaceDlg(g_hwndEdit,&g_efrData,FALSE);
        }
        else {
          SetForegroundWindow(hDlgFindReplace);
          PostMessage(hDlgFindReplace,WM_NEXTDLGCTL,(WPARAM)(GetDlgItem(hDlgFindReplace,IDC_FINDTEXT)),1);
        }
      }
      break;


    case IDM_EDIT_REPLACE:
      if (!IsWindow(hDlgFindReplace))
        hDlgFindReplace = EditFindReplaceDlg(g_hwndEdit,&g_efrData,TRUE);
      else {
        if (!GetDlgItem(hDlgFindReplace,IDC_REPLACE)) {
          SendMessage(hDlgFindReplace,WM_COMMAND,MAKELONG(IDMSG_SWITCHTOREPLACE,1),0);
          DestroyWindow(hDlgFindReplace);
          hDlgFindReplace = EditFindReplaceDlg(g_hwndEdit,&g_efrData,TRUE);
        }
        else {
          SetForegroundWindow(hDlgFindReplace);
          PostMessage(hDlgFindReplace,WM_NEXTDLGCTL,(WPARAM)(GetDlgItem(hDlgFindReplace,IDC_FINDTEXT)),1);
        }
      }
      break;


    // Main Bookmark Functions
    case BME_EDIT_BOOKMARKNEXT:
    {
        int iPos = (int)SendMessage( g_hwndEdit , SCI_GETCURRENTPOS , 0 , 0);
        int iLine = (int)SendMessage( g_hwndEdit , SCI_LINEFROMPOSITION , iPos , 0 );

        int bitmask = (1 << MARKER_NP3_BOOKMARK);
        int iNextLine = (int)SendMessage( g_hwndEdit , SCI_MARKERNEXT , iLine+1 , bitmask );
        if( iNextLine == -1 )
        {
            iNextLine = (int)SendMessage( g_hwndEdit , SCI_MARKERNEXT , 0 , bitmask );
        }

        if( iNextLine != -1 )
        {
            SciCall_EnsureVisible(iNextLine);
            SendMessage( g_hwndEdit , SCI_GOTOLINE , iNextLine , 0 );
            SciCall_ScrollCaret();
        }
        break;
    }

    case BME_EDIT_BOOKMARKPREV:
    {
        int iPos = (int)SendMessage( g_hwndEdit , SCI_GETCURRENTPOS , 0 , 0);
        int iLine = (int)SendMessage( g_hwndEdit , SCI_LINEFROMPOSITION , iPos , 0 );

        int bitmask = 1;
        int iNextLine = (int)SendMessage( g_hwndEdit , SCI_MARKERPREVIOUS , iLine-1 , bitmask );
        if( iNextLine == -1 )
        {
            iNextLine = (int)SendMessage( g_hwndEdit , SCI_MARKERPREVIOUS , SciCall_GetLineCount(), bitmask );
        }

        if( iNextLine != -1 )
        {
            SciCall_EnsureVisible(iNextLine);
            SendMessage( g_hwndEdit , SCI_GOTOLINE , iNextLine , 0 );
            SciCall_ScrollCaret();
        }

        break;
    }

    case BME_EDIT_BOOKMARKTOGGLE:
      {
        int iPos = (int)SendMessage(g_hwndEdit, SCI_GETCURRENTPOS, 0, 0);
        int iLine = (int)SendMessage(g_hwndEdit, SCI_LINEFROMPOSITION, iPos, 0);

        int bitmask = (int)SendMessage(g_hwndEdit, SCI_MARKERGET, iLine, MARKER_NP3_BOOKMARK);
        if (bitmask & (1 << MARKER_NP3_BOOKMARK)) {
          // unset
          SendMessage(g_hwndEdit, SCI_MARKERDELETE, iLine, MARKER_NP3_BOOKMARK);
        }
        else {
          Style_SetCurrentMargin(g_hwndEdit, bShowSelectionMargin);
          // set
          SendMessage(g_hwndEdit, SCI_MARKERADD, iLine, MARKER_NP3_BOOKMARK);
          UpdateLineNumberWidth();
        }
        break;
      }

    case BME_EDIT_BOOKMARKCLEAR:
      SendMessage(g_hwndEdit,SCI_MARKERDELETEALL, (WPARAM)MARKER_NP3_BOOKMARK, 0);
    break;

    case IDM_EDIT_FINDNEXT:
    case IDM_EDIT_FINDPREV:
    case IDM_EDIT_REPLACENEXT:
    case IDM_EDIT_SELTONEXT:
    case IDM_EDIT_SELTOPREV:

      if (SendMessage(g_hwndEdit,SCI_GETLENGTH,0,0) == 0)
        break;

      if (!strlen(g_efrData.szFind)) {
        if (LOWORD(wParam) != IDM_EDIT_REPLACENEXT)
          SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_FIND,1),0);
        else
          SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_REPLACE,1),0);
      }

      else {

        UINT cp = Encoding_SciGetCodePage(g_hwndEdit);
        if (cpLastFind != cp) {
          if (cp != CP_UTF8) {

            WCHAR wch[FNDRPL_BUFFER];

            MultiByteToWideCharStrg(CP_UTF8,g_efrData.szFindUTF8,wch);
            WideCharToMultiByteStrg(cp,wch,g_efrData.szFind);

            MultiByteToWideCharStrg(CP_UTF8,g_efrData.szReplaceUTF8,wch);
            WideCharToMultiByteStrg(cp,wch,g_efrData.szReplace);
          }
          else {
            StringCchCopyA(g_efrData.szFind,COUNTOF(g_efrData.szFind),g_efrData.szFindUTF8);
            StringCchCopyA(g_efrData.szReplace,COUNTOF(g_efrData.szReplace),g_efrData.szReplaceUTF8);
          }
        }
        cpLastFind = cp;
        switch (LOWORD(wParam)) {

          case IDM_EDIT_FINDNEXT:
            EditFindNext(g_hwndEdit,&g_efrData,FALSE);
            break;

          case IDM_EDIT_FINDPREV:
            EditFindPrev(g_hwndEdit,&g_efrData,FALSE);
            break;

          case IDM_EDIT_REPLACENEXT:
            if (bReplaceInitialized)
              EditReplace(g_hwndEdit,&g_efrData);
            else
              SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_REPLACE,1),0);
            break;

          case IDM_EDIT_SELTONEXT:
            EditFindNext(g_hwndEdit,&g_efrData,TRUE);
            break;

          case IDM_EDIT_SELTOPREV:
            EditFindPrev(g_hwndEdit,&g_efrData,TRUE);
            break;
        }
      }
      break;

    case IDM_EDIT_COMPLETEWORD:
        EditCompleteWord(g_hwndEdit, TRUE);
        break;


    case IDM_EDIT_GOTOLINE:
      EditLinenumDlg(g_hwndEdit);
      break;


    case IDM_VIEW_SCHEME:
      Style_SelectLexerDlg(g_hwndEdit);
      UpdateToolbar();
      UpdateStatusbar();
      UpdateLineNumberWidth();
      break;


    case IDM_VIEW_USE2NDDEFAULT:
      Style_ToggleUse2ndDefault(g_hwndEdit);
      UpdateToolbar();
      UpdateStatusbar();
      UpdateLineNumberWidth();
      break;


    case IDM_VIEW_SCHEMECONFIG:
      Style_ConfigDlg(g_hwndEdit);
      UpdateToolbar();
      UpdateStatusbar();
      UpdateLineNumberWidth();
      break;


    case IDM_VIEW_FONT:
      Style_SetDefaultFont(g_hwndEdit, TRUE);
      UpdateToolbar();
      UpdateStatusbar();
      UpdateLineNumberWidth();
      break;

    case IDM_VIEW_CURRENTSCHEME:
      Style_SetDefaultFont(g_hwndEdit, FALSE);
      UpdateToolbar();
      UpdateStatusbar();
      UpdateLineNumberWidth();
      break;

    case IDM_VIEW_WORDWRAP:
      bWordWrap = (bWordWrap) ? FALSE : TRUE;
      if (!bWordWrap)
        SendMessage(g_hwndEdit,SCI_SETWRAPMODE,SC_WRAP_NONE,0);
      else
        SendMessage(g_hwndEdit,SCI_SETWRAPMODE,(iWordWrapMode == 0) ? SC_WRAP_WHITESPACE : SC_WRAP_CHAR,0);
      bWordWrapG = bWordWrap;
      //EditApplyLexerStyle(g_hwndEdit, 0, -1);
      UpdateToolbar();
      break;


    case IDM_VIEW_WORDWRAPSETTINGS:
      if (WordWrapSettingsDlg(hwnd,IDD_WORDWRAP,&iWordWrapIndent)) {
        SetWordWrapping();
      }
      break;


    case IDM_VIEW_WORDWRAPSYMBOLS:
      bShowWordWrapSymbols = (bShowWordWrapSymbols) ? FALSE : TRUE;
      SetWordWrapping();
      break;


    case IDM_VIEW_LONGLINEMARKER:
      bMarkLongLines = (bMarkLongLines) ? FALSE: TRUE;
      if (bMarkLongLines) {
        SendMessage(g_hwndEdit,SCI_SETEDGEMODE,(iLongLineMode == EDGE_LINE)?EDGE_LINE:EDGE_BACKGROUND,0);
        Style_SetLongLineColors(g_hwndEdit);
      }
      else
        SendMessage(g_hwndEdit,SCI_SETEDGEMODE,EDGE_NONE,0);

      UpdateToolbar();
      UpdateStatusbar();
      break;


    case IDM_VIEW_LONGLINESETTINGS:
      if (LongLineSettingsDlg(hwnd,IDD_LONGLINES,&iLongLinesLimit)) {
        bMarkLongLines = TRUE;
        SendMessage(g_hwndEdit,SCI_SETEDGEMODE,(iLongLineMode == EDGE_LINE)?EDGE_LINE:EDGE_BACKGROUND,0);
        Style_SetLongLineColors(g_hwndEdit);
        iLongLinesLimit = max(min(iLongLinesLimit,4096),0);
        SendMessage(g_hwndEdit,SCI_SETEDGECOLUMN,iLongLinesLimit,0);
        iLongLinesLimitG = iLongLinesLimit;
        UpdateToolbar();
        UpdateStatusbar();
      }
      break;


    case IDM_VIEW_TABSASSPACES:
      g_bTabsAsSpaces = (g_bTabsAsSpaces) ? FALSE : TRUE;
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
      bShowIndentGuides = (bShowIndentGuides) ? FALSE : TRUE;
      Style_SetIndentGuides(g_hwndEdit,bShowIndentGuides);
      break;


    case IDM_VIEW_AUTOINDENTTEXT:
      bAutoIndent = (bAutoIndent) ? FALSE : TRUE;
      break;


    case IDM_VIEW_LINENUMBERS:
      bShowLineNumbers = (bShowLineNumbers) ? FALSE : TRUE;
      UpdateLineNumberWidth();
      break;


    case IDM_VIEW_MARGIN:
      bShowSelectionMargin = (bShowSelectionMargin) ? FALSE : TRUE;
      Style_SetCurrentMargin(g_hwndEdit, bShowSelectionMargin);
      UpdateLineNumberWidth();
      break;

    case IDM_VIEW_AUTOCOMPLETEWORDS:
      bAutoCompleteWords = (bAutoCompleteWords) ? FALSE : TRUE;  // toggle
      if (!bAutoCompleteWords)
        SendMessage(g_hwndEdit, SCI_AUTOCCANCEL, 0, 0);  // close the auto completion list
      break;

    case IDM_VIEW_ACCELWORDNAV:
      bAccelWordNavigation = (bAccelWordNavigation) ? FALSE : TRUE;  // toggle  
      EditSetAccelWordNav(g_hwndEdit,bAccelWordNavigation);
      EditClearAllMarks(g_hwndEdit, 0, -1);
      MarkAllOccurrences(iUpdateDelayMarkAllCoccurrences);
      break;

    case IDM_VIEW_MARKOCCUR_ONOFF:
      iMarkOccurrences = (iMarkOccurrences == 0) ? max(1, IniGetInt(L"Settings", L"MarkOccurrences", 1)) : 0;
      EditClearAllMarks(g_hwndEdit, 0, -1);
      if (iMarkOccurrences != 0) {
        MarkAllOccurrences(0);
      }
      break;

    case IDM_VIEW_MARKOCCUR_VISIBLE:
      bMarkOccurrencesMatchVisible = (bMarkOccurrencesMatchVisible) ? FALSE : TRUE;
      EditClearAllMarks(g_hwndEdit, 0, -1);
      MarkAllOccurrences(0);
      break;

    case IDM_VIEW_MARKOCCUR_CASE:
      bMarkOccurrencesMatchCase = (bMarkOccurrencesMatchCase) ? FALSE : TRUE;
      EditClearAllMarks(g_hwndEdit, 0, -1);
      MarkAllOccurrences(iUpdateDelayMarkAllCoccurrences);
      break;

    case IDM_VIEW_MARKOCCUR_WNONE:
      bMarkOccurrencesMatchWords = FALSE;
      bMarkOccurrencesCurrentWord = FALSE;
      EditClearAllMarks(g_hwndEdit, 0, -1);
      MarkAllOccurrences(iUpdateDelayMarkAllCoccurrences);
      break;

    case IDM_VIEW_MARKOCCUR_WORD:
      bMarkOccurrencesMatchWords = TRUE;
      bMarkOccurrencesCurrentWord = FALSE;
      EditClearAllMarks(g_hwndEdit, 0, -1);
      MarkAllOccurrences(iUpdateDelayMarkAllCoccurrences);
      break;

    case IDM_VIEW_MARKOCCUR_CURRENT:
      bMarkOccurrencesMatchWords = FALSE;
      bMarkOccurrencesCurrentWord = TRUE;
      EditClearAllMarks(g_hwndEdit, 0, -1);
      MarkAllOccurrences(iUpdateDelayMarkAllCoccurrences);
      break;

    case IDM_VIEW_FOLDING:
      bShowCodeFolding = (bShowCodeFolding) ? FALSE : TRUE;
      SciCall_SetMarginWidth(MARGIN_FOLD_INDEX, (bShowCodeFolding) ? 11 : 0);
      if (!bShowCodeFolding)
        EditFoldToggleAll(EXPAND);
      UpdateToolbar();
      break;


    case IDM_VIEW_TOGGLEFOLDS:
      EditFoldToggleAll(SNIFF);
      break;


    case IDM_VIEW_SHOWWHITESPACE:
      bViewWhiteSpace = (bViewWhiteSpace) ? FALSE : TRUE;
      SendMessage(g_hwndEdit,SCI_SETVIEWWS,(bViewWhiteSpace)?SCWS_VISIBLEALWAYS:SCWS_INVISIBLE,0);
      break;


    case IDM_VIEW_SHOWEOLS:
      bViewEOLs = (bViewEOLs) ? FALSE : TRUE;
      SendMessage(g_hwndEdit,SCI_SETVIEWEOL,bViewEOLs,0);
      break;


    case IDM_VIEW_MATCHBRACES:
      bMatchBraces = (bMatchBraces) ? FALSE : TRUE;
      if (bMatchBraces)
        EditMatchBrace(g_hwndEdit);
      else
        SendMessage(g_hwndEdit,SCI_BRACEHIGHLIGHT,(WPARAM)-1,(LPARAM)-1);
      break;


    case IDM_VIEW_AUTOCLOSETAGS:
      bAutoCloseTags = (bAutoCloseTags) ? FALSE : TRUE;
      break;


    case IDM_VIEW_HILITECURRENTLINE:
      bHiliteCurrentLine = (bHiliteCurrentLine) ? FALSE : TRUE;
      Style_SetCurrentLineBackground(g_hwndEdit, bHiliteCurrentLine);
      break;

    case IDM_VIEW_HYPERLINKHOTSPOTS:
      bHyperlinkHotspot = (bHyperlinkHotspot) ? FALSE : TRUE;
      Style_SetUrlHotSpot(g_hwndEdit, bHyperlinkHotspot);
      if (bHyperlinkHotspot) {
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


    case IDM_VIEW_TOOLBAR:
      if (bShowToolbar) {
        bShowToolbar = 0;
        ShowWindow(hwndReBar,SW_HIDE);
      }
      else {
        bShowToolbar = 1;
        UpdateToolbar();
        ShowWindow(hwndReBar,SW_SHOW);
      }
      SendWMSize(hwnd);
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
        UpdateStatusbar();
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
      bMinimizeToTray =(bMinimizeToTray) ? FALSE : TRUE;
      break;


    case IDM_VIEW_TRANSPARENT:
      bTransparentMode =(bTransparentMode) ? FALSE : TRUE;
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
      bSaveRecentFiles = (bSaveRecentFiles) ? FALSE : TRUE;
      break;

    case IDM_VIEW_NOPRESERVECARET:
      bPreserveCaretPos = (bPreserveCaretPos) ? FALSE : TRUE;
      break;

    case IDM_VIEW_NOSAVEFINDREPL:
      bSaveFindReplace = (bSaveFindReplace) ? FALSE : TRUE;
      break;


    case IDM_VIEW_SAVEBEFORERUNNINGTOOLS:
      bSaveBeforeRunningTools = (bSaveBeforeRunningTools) ? FALSE : TRUE;
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
        bSaveSettings = (bSaveSettings) ? FALSE : TRUE;
      break;


    case IDM_VIEW_SAVESETTINGSNOW:
      if (IsCmdEnabled(hwnd, IDM_VIEW_SAVESETTINGSNOW)) {

        BOOL bCreateFailure = FALSE;

        if (StringCchLenW(g_wchIniFile,COUNTOF(g_wchIniFile)) == 0) {

          if (StringCchLenW(g_wchIniFile2,COUNTOF(g_wchIniFile2)) > 0) {
            if (CreateIniFileEx(g_wchIniFile2)) {
              StringCchCopy(g_wchIniFile,COUNTOF(g_wchIniFile),g_wchIniFile2);
              StringCchCopy(g_wchIniFile2,COUNTOF(g_wchIniFile2),L"");
            }
            else
              bCreateFailure = TRUE;
          }

          else
            break;
        }

        if (!bCreateFailure) {

          if (WritePrivateProfileString(L"Settings",L"WriteTest",L"ok",g_wchIniFile)) {

            BeginWaitCursorID(IDS_SAVINGSETTINGS);
            SaveSettings(TRUE);
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
      ThemedDialogBox(g_hInstance,MAKEINTRESOURCE(IDD_ABOUT),
        hwnd,AboutDlgProc);
      break;

    case IDM_SETPASS:
      if (GetFileKey(g_hwndEdit)) {
        SetDocumentModified(TRUE);
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
      if (FileSave(TRUE,FALSE,FALSE,FALSE))
        SendMessage(hwnd,WM_CLOSE,0,0);
      break;


    case CMD_CTRLENTER:
      {
        int token = BeginUndoAction();
        int iPos = (int)SendMessage(g_hwndEdit,SCI_GETCURRENTPOS,0,0);
        int iLine = (int)SendMessage(g_hwndEdit,SCI_LINEFROMPOSITION,(WPARAM)iPos,0);
        if (iLine <= 0) {
          SendMessage(g_hwndEdit,SCI_GOTOLINE,0,0);
          SendMessage(g_hwndEdit,SCI_NEWLINE,0,0);
          SendMessage(g_hwndEdit,SCI_GOTOLINE,0,0);
        }
        else {
          SendMessage(g_hwndEdit,SCI_GOTOPOS,
                      SendMessage(g_hwndEdit,SCI_GETLINEENDPOSITION,(WPARAM)(iLine - 1),0),0);
          SendMessage(g_hwndEdit,SCI_NEWLINE,0,0);
        }
        EndUndoAction(token);
      }
      break;


    // Newline with toggled auto indent setting
    case CMD_SHIFTCTRLENTER:
      bAutoIndent = (bAutoIndent) ? 0 : 1;
      SendMessage(g_hwndEdit,SCI_NEWLINE,0,0);
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
        int iPos        = (int)SendMessage(g_hwndEdit,SCI_GETCURRENTPOS,0,0);
        int iAnchor     = (int)SendMessage(g_hwndEdit,SCI_GETANCHOR,0,0);
        int iLine       = (int)SendMessage(g_hwndEdit,SCI_LINEFROMPOSITION,(WPARAM)iPos,0);
        int iStartPos   = (int)SendMessage(g_hwndEdit,SCI_POSITIONFROMLINE,(WPARAM)iLine,0);
        int iIndentPos  = (int)SendMessage(g_hwndEdit,SCI_GETLINEINDENTPOSITION,(WPARAM)iLine,0);

        if (iPos != iAnchor) {
          int token = BeginUndoAction();
          SendMessage(g_hwndEdit,SCI_SETSEL,(WPARAM)iPos,(LPARAM)iPos);
          EndUndoAction(token);
        }
        else {
          if (iPos == iStartPos)
            SendMessage(g_hwndEdit,SCI_DELETEBACK,0,0);
          else if (iPos <= iIndentPos)
            SendMessage(g_hwndEdit,SCI_DELLINELEFT,0,0);
          else
            SendMessage(g_hwndEdit,SCI_DELWORDLEFT,0,0);
        }
      }
      break;


    case CMD_CTRLDEL:
      {
        int iPos        = (int)SendMessage(g_hwndEdit,SCI_GETCURRENTPOS,0,0);
        int iAnchor     = (int)SendMessage(g_hwndEdit,SCI_GETANCHOR,0,0);
        int iLine       = (int)SendMessage(g_hwndEdit,SCI_LINEFROMPOSITION,(WPARAM)iPos,0);
        int iStartPos   = (int)SendMessage(g_hwndEdit,SCI_POSITIONFROMLINE,(WPARAM)iLine,0);
        int iEndPos     = (int)SendMessage(g_hwndEdit,SCI_GETLINEENDPOSITION,(WPARAM)iLine,0);

        if (iPos != iAnchor) {
          int token = BeginUndoAction();
          SendMessage(g_hwndEdit, SCI_SETSEL, (WPARAM)iPos, (LPARAM)iPos);
          EndUndoAction(token);
        }
        else {
          if (iStartPos != iEndPos)
            SendMessage(g_hwndEdit,SCI_DELWORDRIGHT,0,0);
          else // iStartPos == iEndPos
            SendMessage(g_hwndEdit,SCI_LINEDELETE,0,0);
        }
      }
      break;


    case CMD_RECODEDEFAULT:
      {
        WCHAR tchCurFile2[MAX_PATH] = { L'\0' };
        if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
          Encoding_Source(Encoding_MapUnicode(iDefaultEncoding));
          StringCchCopy(tchCurFile2,COUNTOF(tchCurFile2),g_wchCurFile);
          FileLoad(FALSE,FALSE,TRUE,TRUE,tchCurFile2);
        }
      }
      break;


    case CMD_RECODEANSI:
      {
        WCHAR tchCurFile2[MAX_PATH] = { L'\0' };
        if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
          Encoding_Source(CPI_ANSI_DEFAULT);
          StringCchCopy(tchCurFile2,COUNTOF(tchCurFile2),g_wchCurFile);
          FileLoad(FALSE,FALSE,TRUE,TRUE,tchCurFile2);
        }
      }
      break;


    case CMD_RECODEOEM:
      {
        WCHAR tchCurFile2[MAX_PATH] = { L'\0' };
        if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
          Encoding_Source(CPI_OEM);
          StringCchCopy(tchCurFile2,COUNTOF(tchCurFile2),g_wchCurFile);
          FileLoad(FALSE,FALSE,TRUE,TRUE,tchCurFile2);
        }
      }
      break;


    case CMD_RELOADASCIIASUTF8:
      {
        WCHAR tchCurFile2[MAX_PATH] = { L'\0' };
        BOOL _bLoadASCIIasUTF8 = bLoadASCIIasUTF8;
        if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
          bLoadASCIIasUTF8 = 1;
          StringCchCopy(tchCurFile2,COUNTOF(tchCurFile2),g_wchCurFile);
          FileLoad(FALSE,FALSE,TRUE,FALSE,tchCurFile2);
          bLoadASCIIasUTF8 = _bLoadASCIIasUTF8;
        }
      }
      break;


    case CMD_RELOADNOFILEVARS:
      {
        WCHAR tchCurFile2[MAX_PATH] = { L'\0' };
        if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
          int _fNoFileVariables = flagNoFileVariables;
          BOOL _bNoEncodingTags = bNoEncodingTags;
          flagNoFileVariables = 1;
          bNoEncodingTags = 1;
          StringCchCopy(tchCurFile2,COUNTOF(tchCurFile2),g_wchCurFile);
          FileLoad(FALSE,FALSE,TRUE,FALSE,tchCurFile2);
          flagNoFileVariables = _fNoFileVariables;
          bNoEncodingTags = _bNoEncodingTags;
        }
      }
      break;


    case CMD_LEXDEFAULT:
      Style_SetDefaultLexer(g_hwndEdit);
      UpdateToolbar();
      UpdateStatusbar();
      UpdateLineNumberWidth();
      break;


    case CMD_LEXHTML:
      Style_SetHTMLLexer(g_hwndEdit);
      UpdateToolbar();
      UpdateStatusbar();
      UpdateLineNumberWidth();
      break;


    case CMD_LEXXML:
      Style_SetXMLLexer(g_hwndEdit);
      UpdateToolbar();
      UpdateStatusbar();
      UpdateLineNumberWidth();
      break;


    case CMD_TIMESTAMPS:
      {
        WCHAR wchFind[256] = { L'\0' };
        WCHAR wchTemplate[256] = { L'\0' };
        WCHAR wchReplace[256] = { L'\0' };

        SYSTEMTIME st;
        struct tm sst;

        UINT cp;
        EDITFINDREPLACE efrTS = { "", "", "", "", SCFIND_REGEXP, 0, 0, 0, 0, 0, 0, 0, 0, NULL };
        efrTS.hwnd = g_hwndEdit;

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

        cp = Encoding_SciGetCodePage(g_hwndEdit);
        WideCharToMultiByteStrg(cp,wchFind,efrTS.szFind);
        WideCharToMultiByteStrg(cp,wchReplace,efrTS.szReplace);

        if (!SendMessage(g_hwndEdit, SCI_GETSELECTIONEMPTY, 0, 0))
          EditReplaceAllInSelection(g_hwndEdit,&efrTS,TRUE);
        else
          EditReplaceAll(g_hwndEdit,&efrTS,TRUE);
      }
      break;


    case IDM_HELP_UPDATECHECK:
      DialogUpdateCheck(hwnd);
      break;


    case CMD_WEBACTION1:
    case CMD_WEBACTION2:
      {
        WCHAR  szCmdTemplate[256] = { L'\0' };

        LPWSTR lpszTemplateName = (LOWORD(wParam) == CMD_WEBACTION1) ? L"WebTemplate1" : L"WebTemplate2";

        BOOL bCmdEnabled = IniGetString(L"Settings2",lpszTemplateName,L"",szCmdTemplate,COUNTOF(szCmdTemplate));

        if (bCmdEnabled) {

          DWORD cchSelection = (int)SendMessage(g_hwndEdit,SCI_GETSELECTIONEND,0,0) -
                               (int)SendMessage(g_hwndEdit,SCI_GETSELECTIONSTART,0,0);

          char  mszSelection[512] = { '\0' };
          if ((cchSelection > 0) && (cchSelection <= 500) && (SendMessage(g_hwndEdit,SCI_GETSELTEXT,0,0) < COUNTOF(mszSelection)))
          {
            SendMessage(g_hwndEdit,SCI_GETSELTEXT,0,(LPARAM)mszSelection);
            mszSelection[cchSelection] = '\0'; // zero terminate

            // Check lpszSelection and truncate bad WCHARs
            char* lpsz = StrChrA(mszSelection,13);
            if (lpsz) *lpsz = '\0';

            lpsz = StrChrA(mszSelection,10);
            if (lpsz) *lpsz = '\0';

            lpsz = StrChrA(mszSelection,9);
            if (lpsz) *lpsz = '\0';

            if (StringCchLenA(mszSelection,COUNTOF(mszSelection))) {

              WCHAR wszSelection[512] = { L'\0' };
              UINT uCP = Encoding_SciGetCodePage(g_hwndEdit);
              MultiByteToWideCharStrg(uCP,mszSelection,wszSelection);

              int cmdsz = (512 + COUNTOF(szCmdTemplate) + MAX_PATH + 32);
              LPWSTR lpszCommand = GlobalAlloc(GPTR,sizeof(WCHAR)*cmdsz);
              StringCchPrintf(lpszCommand,cmdsz,szCmdTemplate,wszSelection);
              ExpandEnvironmentStringsEx(lpszCommand,(DWORD)GlobalSize(lpszCommand)/sizeof(WCHAR));

              LPWSTR lpszArgs = GlobalAlloc(GPTR,GlobalSize(lpszCommand));
              ExtractFirstArgument(lpszCommand,lpszCommand,lpszArgs,cmdsz);

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
              sei.lpParameters = lpszArgs;
              sei.lpDirectory = wchDirectory;
              sei.nShow = SW_SHOWNORMAL;
              ShellExecuteEx(&sei);

              GlobalFree(lpszCommand);
              GlobalFree(lpszArgs);
            }
          }
        }
      }
      break;


    case CMD_FINDNEXTSEL:
    case CMD_FINDPREVSEL:
    case IDM_EDIT_SAVEFIND:
      {
        int cchSelection = (int)SendMessage(g_hwndEdit,SCI_GETSELECTIONEND,0,0) -
                             (int)SendMessage(g_hwndEdit,SCI_GETSELECTIONSTART,0,0);

        if (cchSelection == 0)
        {
          SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_SELECTWORD,1),0);
          cchSelection = (int)SendMessage(g_hwndEdit,SCI_GETSELECTIONEND,0,0) -
                           (int)SendMessage(g_hwndEdit,SCI_GETSELECTIONSTART,0,0);
        }

        if (cchSelection > 0 && cchSelection < FNDRPL_BUFFER && SendMessage(g_hwndEdit,SCI_GETSELTEXT,0,0) < 512)
        {
          char  mszSelection[FNDRPL_BUFFER];
          char  *lpsz;

          SendMessage(g_hwndEdit,SCI_GETSELTEXT,0,(LPARAM)mszSelection);
          mszSelection[cchSelection] = 0; // zero terminate

          // Check lpszSelection and truncate newlines
          lpsz = StrChrA(mszSelection,'\n');
          if (lpsz) *lpsz = '\0';

          lpsz = StrChrA(mszSelection,'\r');
          if (lpsz) *lpsz = '\0';

          cpLastFind = Encoding_SciGetCodePage(g_hwndEdit);
          StringCchCopyA(g_efrData.szFind,COUNTOF(g_efrData.szFind),mszSelection);

          if (cpLastFind != CP_UTF8)
          {
            WCHAR wszBuf[FNDRPL_BUFFER];

            MultiByteToWideCharStrg(cpLastFind,mszSelection,wszBuf);
            WideCharToMultiByteStrg(CP_UTF8,wszBuf,g_efrData.szFindUTF8);
          }
          else
            StringCchCopyA(g_efrData.szFindUTF8,COUNTOF(g_efrData.szFindUTF8),mszSelection);

          g_efrData.fuFlags &= (~(SCFIND_REGEXP|SCFIND_POSIX));
          g_efrData.bTransformBS = FALSE;

          switch (LOWORD(wParam)) {

            case IDM_EDIT_SAVEFIND:
              break;

            case CMD_FINDNEXTSEL:
              EditFindNext(g_hwndEdit,&g_efrData,FALSE);
              break;

            case CMD_FINDPREVSEL:
              EditFindPrev(g_hwndEdit,&g_efrData,FALSE);
              break;
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
          iLongLinesLimit++;
        else
          iLongLinesLimit--;
        iLongLinesLimit = max(min(iLongLinesLimit,4096),0);
        SendMessage(g_hwndEdit,SCI_SETEDGECOLUMN,iLongLinesLimit,0);
        UpdateToolbar();
        UpdateStatusbar();
        iLongLinesLimitG = iLongLinesLimit;
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
      EditModifyNumber(g_hwndEdit,TRUE);
      break;


    case CMD_DECREASENUM:
      EditModifyNumber(g_hwndEdit,FALSE);
      break;


    case CMD_TOGGLETITLE:
      EditGetExcerpt(g_hwndEdit,szTitleExcerpt,COUNTOF(szTitleExcerpt));
      UpdateToolbar();
      break;


    case CMD_JUMP2SELSTART:
      if (!SciCall_IsSelectionRectangle()) {
        int iAnchorPos = (int)SendMessage(g_hwndEdit,SCI_GETANCHOR,0,0);
        int iCursorPos = (int)SendMessage(g_hwndEdit,SCI_GETCURRENTPOS,0,0);
        if (iCursorPos > iAnchorPos) {
          SendMessage(g_hwndEdit,SCI_SETSEL,iCursorPos,iAnchorPos);
          SendMessage(g_hwndEdit,SCI_CHOOSECARETX,0,0);
        }
      }
      break;


    case CMD_JUMP2SELEND:
      if (!SciCall_IsSelectionRectangle()) {
        int iAnchorPos = (int)SendMessage(g_hwndEdit,SCI_GETANCHOR,0,0);
        int iCursorPos = (int)SendMessage(g_hwndEdit,SCI_GETCURRENTPOS,0,0);
        if (iCursorPos < iAnchorPos) {
          SendMessage(g_hwndEdit,SCI_SETSEL,iCursorPos,iAnchorPos);
          SendMessage(g_hwndEdit,SCI_CHOOSECARETX,0,0);
        }
      }
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

        if (OpenClipboard(hwnd)) {
          HANDLE hData;
          WCHAR *pData;
          EmptyClipboard();
          int len = lstrlen(pszCopy);
          hData = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,sizeof(WCHAR) * (len+1));
          pData = GlobalLock(hData);
          StringCchCopyN(pData,(len+1),pszCopy,len);
          GlobalUnlock(hData);
          SetClipboardData(CF_UNICODETEXT,hData);
          CloseClipboard();
        }
      }
      break;


    case CMD_COPYWINPOS: {

        WCHAR wszWinPos[MIDSZ_BUFFER];

        WININFO wi = GetMyWindowPlacement(g_hwndMain,NULL);
        StringCchPrintf(wszWinPos,COUNTOF(wszWinPos),L"/pos %i,%i,%i,%i,%i",wi.x,wi.y,wi.cx,wi.cy,wi.max);

        if (OpenClipboard(hwnd)) {
          HANDLE hData;
          WCHAR *pData;
          EmptyClipboard();
          int len = StringCchLenW(wszWinPos,COUNTOF(wszWinPos));
          hData = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,sizeof(WCHAR) * (len+1));
          pData = GlobalLock(hData);
          StringCchCopyN(pData,(len+1),wszWinPos,len);
          GlobalUnlock(hData);
          SetClipboardData(CF_UNICODETEXT,hData);
          CloseClipboard();
        }
        UpdateToolbar();
      }
      break;


    case CMD_DEFAULTWINPOS:
      SnapToDefaultPos(hwnd);
      break;


    case CMD_OPENINIFILE:
      if (StringCchLenW(g_wchIniFile,COUNTOF(g_wchIniFile))) {
        CreateIniFile();
        FileLoad(FALSE,FALSE,FALSE,FALSE,g_wchIniFile);
      }
      break;


    case CMD_OPEN_HYPERLINK:
        OpenHotSpotURL((int)SendMessage(g_hwndEdit, SCI_GETCURRENTPOS, 0, 0), FALSE);
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
        //SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_COPYLINE,1),0);
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
void OpenHotSpotURL(DocPos position, BOOL bForceBrowser)
{
  int iStyle = (int)SendMessage(g_hwndEdit, SCI_GETSTYLEAT, position, 0);

  if (Style_GetHotspotStyleID() != iStyle)
    return; 

  if (!(BOOL)SendMessage(g_hwndEdit, SCI_STYLEGETHOTSPOT, Style_GetHotspotStyleID(), 0))
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
    MultiByteToWideCharStrg(Encoding_SciGetCodePage(g_hwndEdit), chURL, wchURL);

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
          FileLoad(FALSE, FALSE, FALSE, FALSE, tchFile);
      }
      else
        FileLoad(FALSE, FALSE, FALSE, FALSE, szFileName);

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
        SetDocumentModified(TRUE);
        return TRUE;
      }
      else if (pnmh->code == SCN_SAVEPOINTREACHED) {
        SetDocumentModified(FALSE);
        return TRUE;
      }
      else if (pnmh->code == SCN_SAVEPOINTLEFT) {
        SetDocumentModified(TRUE);
        return TRUE;
      }
    }
    return FALSE;
  }

  switch(pnmh->idFrom)
  {
    case IDC_EDIT:

      switch (pnmh->code)
      {
        case SCN_HOTSPOTCLICK:
        {
          if (scn->modifiers & SCMOD_CTRL) {
            // open in browser
            OpenHotSpotURL((int)scn->position, TRUE);
          }
          if (scn->modifiers & SCMOD_ALT) {
            // open in application, if applicable (file://)
            OpenHotSpotURL((int)scn->position, FALSE);
          }
        }
        break;


        //case SCN_STYLENEEDED:  // this event needs SCI_SETLEXER(SCLEX_CONTAINER)
        //  {
        //    int lineNumber = SciCall_LineFromPosition(SciCall_GetEndStyled());
        //    EditUpdateUrlHotspots(g_hwndEdit, SciCall_PositionFromLine(lineNumber), (int)scn->position, bHyperlinkHotspot);
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

            if (iMarkOccurrences) {
              // clear marks only, if caret/selection changed
              if (scn->updated & SC_UPDATE_SELECTION) {
                EditClearAllMarks(g_hwndEdit, 0, -1);
                MarkAllOccurrences(iUpdateDelayMarkAllCoccurrences);
              }
              else {
                MarkAllOccurrences(iUpdateDelayMarkAllCoccurrences);
              }
            }

            if (bHyperlinkHotspot) {
              UpdateVisibleUrlHotspot(iUpdateDelayHyperlinkStyling);
            }
            UpdateToolbar();
            UpdateStatusbar();
          }
          else if (scn->updated & SC_UPDATE_V_SCROLL)
          {
            if (iMarkOccurrences) {
              MarkAllOccurrences(iUpdateDelayMarkAllCoccurrences);
            }
            if (bHyperlinkHotspot) {
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
              EditUpdateUrlHotspots(g_hwndEdit, (int)scn->position, (int)(scn->position + scn->length), bHyperlinkHotspot);
            }

            if (iMarkOccurrences) {
              EditClearAllMarks(g_hwndEdit, 0, -1);
              MarkAllOccurrences(iUpdateDelayMarkAllCoccurrences);
            }

            if (scn->linesAdded != 0) {
              UpdateLineNumberWidth();
            }

            SetDocumentModified(TRUE);

            UpdateToolbar();
            UpdateStatusbar();
          }
          break;


        case SCN_CHARADDED:
          {
            char chLineBuffer[FNDRPL_BUFFER] = { '\0' };

            // Auto indent
            if (bAutoIndent && (scn->ch == '\x0D' || scn->ch == '\x0A'))
            {
              // in CRLF mode handle LF only...
              if ((SC_EOL_CRLF == iEOLMode && scn->ch != '\x0A') || SC_EOL_CRLF != iEOLMode)
              {
                DocPos iCurPos = SciCall_GetCurrentPos();
                DocLn iCurLine = SciCall_LineFromPosition(iCurPos);

                // Move bookmark along with line if inserting lines (pressing return at beginning of line) because Scintilla does not do this for us
                if (iCurLine > 0)
                {
                  DocPos iPrevLineLength = SciCall_GetLineEndPosition(iCurLine - 1) - SciCall_PositionFromLine(iCurLine - 1);
                  if (iPrevLineLength == 0)
                  {
                    int bitmask = (int)SendMessage(g_hwndEdit, SCI_MARKERGET, iCurLine - 1, 0);
                    if (bitmask & (1 << MARKER_NP3_BOOKMARK))
                    {
                      SendMessage(g_hwndEdit, SCI_MARKERDELETE, iCurLine - 1, MARKER_NP3_BOOKMARK);
                      SendMessage(g_hwndEdit, SCI_MARKERADD, iCurLine, MARKER_NP3_BOOKMARK);
                    }
                  }
                }

                if (iCurLine > 0/* && iLineLength <= 2*/)
                {
                  const DocPos iPrevLineLength = SciCall_LineLength(iCurLine - 1);
                  char* pLineBuf = NULL;
                  if (iPrevLineLength < FNDRPL_BUFFER) {
                    pLineBuf = chLineBuffer;
                  }
                  else {
                    pLineBuf = GlobalAlloc(GPTR, iPrevLineLength + 1);
                  }
                  if (pLineBuf)
                  {
                    SendMessage(g_hwndEdit, SCI_GETLINE, iCurLine - 1, (LPARAM)pLineBuf);
                    *(pLineBuf + iPrevLineLength) = '\0';
                    for (char* pPos = pLineBuf; *pPos; pPos++) {
                      if (*pPos != ' ' && *pPos != '\t')
                        *pPos = '\0';
                    }
                    if (*pLineBuf) {
                      SendMessage(g_hwndEdit, SCI_BEGINUNDOACTION, 0, 0);
                      SendMessage(g_hwndEdit, SCI_ADDTEXT, lstrlenA(pLineBuf), (LPARAM)pLineBuf);
                      SendMessage(g_hwndEdit, SCI_ENDUNDOACTION, 0, 0);
                    }
                    if (iPrevLineLength >= FNDRPL_BUFFER) { GlobalFree(pLineBuf); }
                  }
                }
              }
            }
            // Auto close tags
            else if (bAutoCloseTags && scn->ch == '>')
            {
              //int iLexer = (int)SendMessage(g_hwndEdit,SCI_GETLEXER,0,0);
              //if (iLexer == SCLEX_HTML || iLexer == SCLEX_XML)
              {
                char tchBuf[512] = { '\0' };
                int  iCurPos = (int)SendMessage(g_hwndEdit, SCI_GETCURRENTPOS, 0, 0);
                int  iHelper = iCurPos - (COUNTOF(tchBuf) - 1);
                int  iStartPos = max(0, iHelper);
                int  iSize = iCurPos - iStartPos;

                if (iSize >= 3) {

                  struct Sci_TextRange tr;
                  tr.chrg.cpMin = iStartPos;  tr.chrg.cpMax = iCurPos;  tr.lpstrText = tchBuf;

                  SendMessage(g_hwndEdit, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);

                  if (tchBuf[iSize - 2] != '/') {

                    const char* pBegin = &tchBuf[0];
                    const char* pCur = &tchBuf[iSize - 2];

                    while (pCur > pBegin && *pCur != '<' && *pCur != '>')
                      --pCur;

                    int  cchIns = 2;
                    char tchIns[516] = "</";
                    if (*pCur == '<') {
                      pCur++;
                      while (StrChrA(":_-.", *pCur) || IsCharAlphaNumericA(*pCur)) {
                        tchIns[cchIns++] = *pCur;
                        pCur++;
                      }
                    }

                    tchIns[cchIns++] = '>';
                    tchIns[cchIns] = 0;

                    if (cchIns > 3 &&
                      StringCchCompareINA(tchIns, COUNTOF(tchIns), "</base>", -1) &&
                      StringCchCompareINA(tchIns, COUNTOF(tchIns), "</bgsound>", -1) &&
                      StringCchCompareINA(tchIns, COUNTOF(tchIns), "</br>", -1) &&
                      StringCchCompareINA(tchIns, COUNTOF(tchIns), "</embed>", -1) &&
                      StringCchCompareINA(tchIns, COUNTOF(tchIns), "</hr>", -1) &&
                      StringCchCompareINA(tchIns, COUNTOF(tchIns), "</img>", -1) &&
                      StringCchCompareINA(tchIns, COUNTOF(tchIns), "</input>", -1) &&
                      StringCchCompareINA(tchIns, COUNTOF(tchIns), "</link>", -1) &&
                      StringCchCompareINA(tchIns, COUNTOF(tchIns), "</meta>", -1))
                    {
                      int token = BeginUndoAction();
                      SendMessage(g_hwndEdit, SCI_REPLACESEL, 0, (LPARAM)tchIns);
                      SendMessage(g_hwndEdit, SCI_SETSEL, iCurPos, iCurPos);
                      EndUndoAction(token);
                    }
                  }
                }
              }
            }
            else if (bAutoCompleteWords && !SendMessage(g_hwndEdit, SCI_AUTOCACTIVE, 0, 0)) {
              EditCompleteWord(g_hwndEdit, FALSE);
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
          if (scn->margin == MARGIN_FOLD_INDEX) {
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
          SendMessage(g_hwndEdit, SCI_SETSCROLLWIDTH, DEFAULT_SCROLL_WIDTH, 0);
          SetDocumentModified(FALSE);
          break;


        case SCN_SAVEPOINTLEFT:
          SetDocumentModified(TRUE);
          break;


        case SCN_ZOOM:
          UpdateLineNumberWidth();
          break;


        default:
          return FALSE;
      }
      return TRUE;


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
              return TRUE;
            }
          }
          return FALSE;

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
          return FALSE;
      }
      return TRUE;


    case IDC_STATUSBAR:

      switch(pnmh->code)
      {

        case NM_CLICK:
          {
            LPNMMOUSE pnmm = (LPNMMOUSE)lParam;

            switch (pnmm->dwItemSpec)
            {
              case STATUS_EOLMODE:
                SendMessage(g_hwndEdit,SCI_CONVERTEOLS,SendMessage(g_hwndEdit,SCI_GETEOLMODE,0,0),0);
                EditFixPositions(g_hwndEdit);
                return TRUE;

              default:
                return FALSE;
            }
          }

        case NM_DBLCLK:
          {
            int i;
            LPNMMOUSE pnmm = (LPNMMOUSE)lParam;

            switch (pnmm->dwItemSpec)
            {
              case STATUS_CODEPAGE:
                SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_ENCODING_SELECT,1),0);
                return TRUE;

              case STATUS_EOLMODE:
                if (iEOLMode == SC_EOL_CRLF)
                  i = IDM_LINEENDINGS_CRLF;
                else if (iEOLMode == SC_EOL_LF)
                  i = IDM_LINEENDINGS_LF;
                else
                  i = IDM_LINEENDINGS_CR;
                i++;
                if (i > IDM_LINEENDINGS_CR)
                  i = IDM_LINEENDINGS_CRLF;
                SendMessage(hwnd,WM_COMMAND,MAKELONG(i,1),0);
                return TRUE;

              case STATUS_OVRMODE:
                SendMessage(g_hwndEdit,SCI_EDITTOGGLEOVERTYPE,0,0);
                return TRUE;

              case STATUS_2ND_DEF:
                SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_VIEW_USE2NDDEFAULT, 1), 0);
                return TRUE;

              case STATUS_LEXER:
                SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_VIEW_SCHEME, 1), 0);
                return TRUE;

              default:
                return FALSE;
            }
          }
          break;

      }
      return TRUE;


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

  return FALSE;
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

  LoadIniSection(L"Settings",pIniSection,cchIniSection);

  bEnableSaveSettings = TRUE;
  bSaveSettings = IniSectionGetBool(pIniSection,L"SaveSettings",TRUE);

  bSaveRecentFiles = IniSectionGetBool(pIniSection,L"SaveRecentFiles",FALSE);
  
  bPreserveCaretPos = IniSectionGetBool(pIniSection, L"PreserveCaretPos",FALSE);

  bSaveFindReplace = IniSectionGetBool(pIniSection,L"SaveFindReplace",FALSE);

  g_efrData.bFindClose = IniSectionGetBool(pIniSection,L"CloseFind", FALSE);

  g_efrData.bReplaceClose = IniSectionGetBool(pIniSection,L"CloseReplace", FALSE);

  g_efrData.bNoFindWrap = IniSectionGetBool(pIniSection,L"NoFindWrap", FALSE);

  g_efrData.bTransformBS = IniSectionGetBool(pIniSection,L"FindTransformBS", FALSE);

  g_efrData.bWildcardSearch = IniSectionGetBool(pIniSection,L"WildcardSearch",FALSE);

  g_efrData.bMarkOccurences = IniSectionGetBool(pIniSection, L"FindMarkAllOccurrences", FALSE);

  g_efrData.bDotMatchAll = IniSectionGetBool(pIniSection, L"RegexDotMatchesAll", FALSE);
  
  g_efrData.fuFlags = IniSectionGetUInt(pIniSection, L"efrData_fuFlags", 0);

  if (!IniSectionGetString(pIniSection, L"OpenWithDir", L"", tchOpenWithDir, COUNTOF(tchOpenWithDir))) {
    //SHGetSpecialFolderPath(NULL, tchOpenWithDir, CSIDL_DESKTOPDIRECTORY, TRUE);
    GetKnownFolderPath(&FOLDERID_Desktop, tchOpenWithDir, COUNTOF(tchOpenWithDir));
  }
  else {
    PathAbsoluteFromApp(tchOpenWithDir, NULL, COUNTOF(tchOpenWithDir), TRUE);
  }
  if (!IniSectionGetString(pIniSection, L"Favorites", L"", tchFavoritesDir, COUNTOF(tchFavoritesDir))) {
    //SHGetFolderPath(NULL,CSIDL_PERSONAL,NULL,SHGFP_TYPE_CURRENT,tchFavoritesDir);
    GetKnownFolderPath(&FOLDERID_Favorites, tchFavoritesDir, COUNTOF(tchFavoritesDir));
  }
  else {
    PathAbsoluteFromApp(tchFavoritesDir, NULL, COUNTOF(tchFavoritesDir), TRUE);
  }

  iPathNameFormat = IniSectionGetInt(pIniSection,L"PathNameFormat",0);
  iPathNameFormat = max(min(iPathNameFormat,2),0);

  bWordWrap = IniSectionGetBool(pIniSection,L"WordWrap",FALSE);
  bWordWrapG = bWordWrap;

  iWordWrapMode = IniSectionGetInt(pIniSection,L"WordWrapMode",0);
  iWordWrapMode = max(min(iWordWrapMode,1),0);

  iWordWrapIndent = IniSectionGetInt(pIniSection,L"WordWrapIndent",0);
  iWordWrapIndent = max(min(iWordWrapIndent,6),0);

  iWordWrapSymbols = IniSectionGetInt(pIniSection,L"WordWrapSymbols",22);
  iWordWrapSymbols = max(min(iWordWrapSymbols%10,2),0)+max(min((iWordWrapSymbols%100-iWordWrapSymbols%10)/10,2),0)*10;

  bShowWordWrapSymbols = IniSectionGetBool(pIniSection,L"ShowWordWrapSymbols",0);

  bMatchBraces = IniSectionGetBool(pIniSection,L"MatchBraces",TRUE);

  bAutoCloseTags = IniSectionGetBool(pIniSection,L"AutoCloseTags",FALSE);

  bHiliteCurrentLine = IniSectionGetBool(pIniSection,L"HighlightCurrentLine",FALSE);

  bHyperlinkHotspot = IniSectionGetBool(pIniSection, L"HyperlinkHotspot", FALSE);

  bAutoIndent = IniSectionGetBool(pIniSection,L"AutoIndent",TRUE);

  bAutoCompleteWords = IniSectionGetBool(pIniSection,L"AutoCompleteWords",FALSE);

  bAccelWordNavigation = IniSectionGetBool(pIniSection, L"AccelWordNavigation", FALSE);

  bShowIndentGuides = IniSectionGetBool(pIniSection,L"ShowIndentGuides",FALSE);

  g_bTabsAsSpaces = IniSectionGetBool(pIniSection,L"TabsAsSpaces",TRUE);
  bTabsAsSpacesG = g_bTabsAsSpaces;

  g_bTabIndents = IniSectionGetBool(pIniSection,L"TabIndents",TRUE);
  bTabIndentsG = g_bTabIndents;

  bBackspaceUnindents = IniSectionGetBool(pIniSection,L"BackspaceUnindents",FALSE);

  g_iTabWidth = IniSectionGetInt(pIniSection,L"TabWidth",2);
  g_iTabWidth = max(min(g_iTabWidth,256),1);
  iTabWidthG = g_iTabWidth;

  g_iIndentWidth = IniSectionGetInt(pIniSection,L"IndentWidth",0);
  g_iIndentWidth = max(min(g_iIndentWidth,256),0);
  iIndentWidthG = g_iIndentWidth;

  bMarkLongLines = IniSectionGetBool(pIniSection,L"MarkLongLines",FALSE);

  iLongLinesLimit = IniSectionGetInt(pIniSection,L"LongLinesLimit",72);
  iLongLinesLimit = max(min(iLongLinesLimit,4096),0);
  iLongLinesLimitG = iLongLinesLimit;

  iLongLineMode = IniSectionGetInt(pIniSection,L"LongLineMode",EDGE_LINE);
  iLongLineMode = max(min(iLongLineMode,EDGE_BACKGROUND),EDGE_LINE);

  bShowSelectionMargin = IniSectionGetBool(pIniSection,L"ShowSelectionMargin",FALSE);

  bShowLineNumbers = IniSectionGetBool(pIniSection,L"ShowLineNumbers", TRUE);

  bShowCodeFolding = IniSectionGetBool(pIniSection,L"ShowCodeFolding", TRUE);

  iMarkOccurrences = IniSectionGetInt(pIniSection,L"MarkOccurrences",1);
  iMarkOccurrences = max(min(iMarkOccurrences, 3), 0);
  bMarkOccurrencesMatchVisible = IniSectionGetBool(pIniSection, L"MarkOccurrencesMatchVisible", FALSE);
  bMarkOccurrencesMatchCase = IniSectionGetBool(pIniSection,L"MarkOccurrencesMatchCase",FALSE);
  bMarkOccurrencesMatchWords = IniSectionGetBool(pIniSection,L"MarkOccurrencesMatchWholeWords",TRUE);
  bMarkOccurrencesCurrentWord = IniSectionGetBool(pIniSection, L"MarkOccurrencesCurrentWord", !bMarkOccurrencesMatchWords);
  bMarkOccurrencesCurrentWord = bMarkOccurrencesCurrentWord && !bMarkOccurrencesMatchWords;

  bViewWhiteSpace = IniSectionGetBool(pIniSection,L"ViewWhiteSpace", FALSE);

  bViewEOLs = IniSectionGetBool(pIniSection,L"ViewEOLs", FALSE);

  iDefaultEncoding = IniSectionGetInt(pIniSection,L"DefaultEncoding", CPI_NONE);
  // if DefaultEncoding is not defined set to system's current code-page 
  iDefaultEncoding = (iDefaultEncoding == CPI_NONE) ?
    Encoding_MapIniSetting(TRUE,(int)GetACP()) : Encoding_MapIniSetting(TRUE,iDefaultEncoding);

  bSkipUnicodeDetection = IniSectionGetBool(pIniSection, L"SkipUnicodeDetection", FALSE);

  bLoadASCIIasUTF8 = IniSectionGetBool(pIniSection, L"LoadASCIIasUTF8", FALSE);

  bLoadNFOasOEM = IniSectionGetBool(pIniSection,L"LoadNFOasOEM",TRUE);

  bNoEncodingTags = IniSectionGetBool(pIniSection,L"NoEncodingTags", FALSE);

  iDefaultEOLMode = IniSectionGetInt(pIniSection,L"DefaultEOLMode",0);
  iDefaultEOLMode = max(min(iDefaultEOLMode,2),0);

  bFixLineEndings = IniSectionGetBool(pIniSection,L"FixLineEndings",TRUE);

  bAutoStripBlanks = IniSectionGetBool(pIniSection,L"FixTrailingBlanks",FALSE);

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

  bSaveBeforeRunningTools = IniSectionGetBool(pIniSection,L"SaveBeforeRunningTools",FALSE);

  iFileWatchingMode = IniSectionGetInt(pIniSection,L"FileWatchingMode",0);
  iFileWatchingMode = max(min(iFileWatchingMode,2),0);

  bResetFileWatching = IniSectionGetBool(pIniSection,L"ResetFileWatching",TRUE);

  iEscFunction = IniSectionGetInt(pIniSection,L"EscFunction",0);
  iEscFunction = max(min(iEscFunction,2),0);

  bAlwaysOnTop = IniSectionGetBool(pIniSection,L"AlwaysOnTop",FALSE);

  bMinimizeToTray = IniSectionGetBool(pIniSection,L"MinimizeToTray",FALSE);

  bTransparentMode = IniSectionGetBool(pIniSection,L"TransparentMode",FALSE);

  // Check if SetLayeredWindowAttributes() is available
  bTransparentModeAvailable = (GetProcAddress(GetModuleHandle(L"User32"),"SetLayeredWindowAttributes") != NULL);
  bTransparentModeAvailable = (bTransparentModeAvailable) ? TRUE : FALSE;

  // see TBBUTTON  tbbMainWnd[] for initial/reset set of buttons
  IniSectionGetString(pIniSection,L"ToolbarButtons", L"", tchToolbarButtons, COUNTOF(tchToolbarButtons));

  bShowToolbar = IniSectionGetBool(pIniSection,L"ShowToolbar",TRUE);

  bShowStatusbar = IniSectionGetBool(pIniSection,L"ShowStatusbar",TRUE);

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


  LoadIniSection(L"Settings2",pIniSection,cchIniSection);

  bStickyWinPos = IniSectionGetInt(pIniSection,L"StickyWindowPosition",0);
  if (bStickyWinPos) bStickyWinPos = 1;

  IniSectionGetString(pIniSection,L"DefaultExtension",L"txt",
    tchDefaultExtension,COUNTOF(tchDefaultExtension));
  StrTrim(tchDefaultExtension,L" \t.\"");

  IniSectionGetString(pIniSection,L"DefaultDirectory",L"",
    tchDefaultDir,COUNTOF(tchDefaultDir));

  ZeroMemory(tchFileDlgFilters,sizeof(WCHAR)*COUNTOF(tchFileDlgFilters));
  IniSectionGetString(pIniSection,L"FileDlgFilters",L"",
    tchFileDlgFilters,COUNTOF(tchFileDlgFilters)-2);

  dwFileCheckInverval = IniSectionGetInt(pIniSection,L"FileCheckInverval",2000);
  dwAutoReloadTimeout = IniSectionGetInt(pIniSection,L"AutoReloadTimeout",2000);

  iSciDirectWriteTech = IniSectionGetInt(pIniSection,L"SciDirectWriteTech", DirectWriteTechnology[0]);
  iSciDirectWriteTech = max(min(iSciDirectWriteTech,3),-1);

  iSciFontQuality = IniSectionGetInt(pIniSection,L"SciFontQuality", FontQuality[3]);
  iSciFontQuality = max(min(iSciFontQuality, 3), 0);

  iMarkOccurrencesMaxCount = IniSectionGetInt(pIniSection,L"MarkOccurrencesMaxCount",2000);
  iMarkOccurrencesMaxCount = (iMarkOccurrencesMaxCount <= 0) ? INT_MAX : iMarkOccurrencesMaxCount;

  iUpdateDelayHyperlinkStyling = IniSectionGetInt(pIniSection, L"UpdateDelayHyperlinkStyling", 100);
  iUpdateDelayHyperlinkStyling = max(min(iUpdateDelayHyperlinkStyling, 10000), 0);

  iUpdateDelayMarkAllCoccurrences = IniSectionGetInt(pIniSection, L"UpdateDelayMarkAllCoccurrences", 50);
  iUpdateDelayMarkAllCoccurrences = max(min(iUpdateDelayMarkAllCoccurrences, 10000), 0);

  bDenyVirtualSpaceAccess = IniSectionGetBool(pIniSection, L"DenyVirtualSpaceAccess", FALSE);
  bUseOldStyleBraceMatching = IniSectionGetBool(pIniSection, L"UseOldStyleBraceMatching", FALSE);
  
  iCurrentLineVerticalSlop = IniSectionGetInt(pIniSection, L"CurrentLineVerticalSlop", 5);
  iCurrentLineVerticalSlop = max(min(iCurrentLineVerticalSlop, 80), 0);

  LoadIniSection(L"Toolbar Images",pIniSection,cchIniSection);

  IniSectionGetString(pIniSection,L"BitmapDefault",L"",
    tchToolbarBitmap,COUNTOF(tchToolbarBitmap));
  IniSectionGetString(pIniSection,L"BitmapHot",L"",
    tchToolbarBitmapHot,COUNTOF(tchToolbarBitmap));
  IniSectionGetString(pIniSection,L"BitmapDisabled",L"",
    tchToolbarBitmapDisabled,COUNTOF(tchToolbarBitmap));

  int ResX = GetSystemMetrics(SM_CXSCREEN);
  int ResY = GetSystemMetrics(SM_CYSCREEN);

  LoadIniSection(L"Window", pIniSection, cchIniSection);

  WCHAR tchHighDpiToolBar[32] = { L'\0' };
  StringCchPrintf(tchHighDpiToolBar,COUNTOF(tchHighDpiToolBar),L"%ix%i HighDpiToolBar", ResX, ResY);
  iHighDpiToolBar = IniSectionGetInt(pIniSection, tchHighDpiToolBar, -1);
  iHighDpiToolBar = max(min(iHighDpiToolBar, 1), -1);
  if (iHighDpiToolBar < 0) { // undefined: determine high DPI (higher than Full-HD)
    if ((ResX > 1920) && (ResY > 1080))
      iHighDpiToolBar = 1;
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
  if (iDefaultEncoding == CPI_ANSI_DEFAULT)
  {
    // check for Chinese, Japan, Korean DBCS code pages and switch accordingly
    int acp = (int)GetACP();
    if (acp == 932 || acp == 936 || acp == 949 || acp == 950) {
      iSciDefaultCodePage = acp;
    }
    iDefaultEncoding = Encoding_GetByCodePage(iSciDefaultCodePage);
  }
  */

  // set flag for encoding default
  g_Encodings[iDefaultEncoding].uFlags |= NCP_DEFAULT;

  // define default charset
  iDefaultCharSet = (int)CharSetFromCodePage((UINT)iSciDefaultCodePage);

  // Scintilla Styles
  Style_Load();

}


//=============================================================================
//
//  SaveSettings()
//
//
void SaveSettings(BOOL bSaveSettingsNow) {
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
  IniSectionSetBool(pIniSection, L"RegexDotMatchesAll", g_efrData.bDotMatchAll);
  IniSectionSetInt(pIniSection, L"efrData_fuFlags", g_efrData.fuFlags);
  PathRelativeToApp(tchOpenWithDir, wchTmp, COUNTOF(wchTmp), FALSE, TRUE, flagPortableMyDocs);
  IniSectionSetString(pIniSection, L"OpenWithDir", wchTmp);
  PathRelativeToApp(tchFavoritesDir, wchTmp, COUNTOF(wchTmp), FALSE, TRUE, flagPortableMyDocs);
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
  IniSectionSetBool(pIniSection, L"HyperlinkHotspot", bHyperlinkHotspot);
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
  IniSectionSetInt(pIniSection, L"LongLinesLimit", iLongLinesLimitG);
  IniSectionSetInt(pIniSection, L"LongLineMode", iLongLineMode);
  IniSectionSetBool(pIniSection, L"ShowSelectionMargin", bShowSelectionMargin);
  IniSectionSetBool(pIniSection, L"ShowLineNumbers", bShowLineNumbers);
  IniSectionSetBool(pIniSection, L"ShowCodeFolding", bShowCodeFolding);
  IniSectionSetInt(pIniSection, L"MarkOccurrences", iMarkOccurrences);
  IniSectionSetBool(pIniSection, L"MarkOccurrencesMatchVisible", bMarkOccurrencesMatchVisible);
  IniSectionSetBool(pIniSection, L"MarkOccurrencesMatchCase", bMarkOccurrencesMatchCase);
  IniSectionSetBool(pIniSection, L"MarkOccurrencesMatchWholeWords", bMarkOccurrencesMatchWords);
  IniSectionSetBool(pIniSection, L"MarkOccurrencesCurrentWord", bMarkOccurrencesCurrentWord);
  IniSectionSetBool(pIniSection, L"ViewWhiteSpace", bViewWhiteSpace);
  IniSectionSetBool(pIniSection, L"ViewEOLs", bViewEOLs);
  IniSectionSetInt(pIniSection, L"DefaultEncoding", Encoding_MapIniSetting(FALSE, iDefaultEncoding));
  IniSectionSetBool(pIniSection, L"SkipUnicodeDetection", bSkipUnicodeDetection);
  IniSectionSetInt(pIniSection, L"LoadASCIIasUTF8", bLoadASCIIasUTF8);
  IniSectionSetBool(pIniSection, L"LoadNFOasOEM", bLoadNFOasOEM);
  IniSectionSetBool(pIniSection, L"NoEncodingTags", bNoEncodingTags);
  IniSectionSetInt(pIniSection, L"DefaultEOLMode", iDefaultEOLMode);
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
  IniSectionSetInt(pIniSection, L"FileWatchingMode", iFileWatchingMode);
  IniSectionSetBool(pIniSection, L"ResetFileWatching", bResetFileWatching);
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

    Toolbar_GetButtons(g_hwndToolbar, IDT_FILE_NEW, tchToolbarButtons, COUNTOF(tchToolbarButtons));
  if (StringCchCompareX(tchToolbarButtons, TBBUTTON_DEFAULT_IDS) == 0) { tchToolbarButtons[0] = L'\0'; }
  IniSectionSetString(pIniSection, L"ToolbarButtons", tchToolbarButtons);

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
  BOOL bContinue = TRUE;
  BOOL bIsFileArg = FALSE;
  BOOL bIsNotepadReplacement = FALSE;

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
      bIsFileArg = TRUE;
    }

    else if (!bIsFileArg && (StringCchCompareN(lp1,len,L"-",-1) == 0)) {
      flagMultiFileArg = 1;
      bIsFileArg = TRUE;
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
        StringCchCopyN(szBufferFile,COUNTOF(szBufferFile),
          lp1 + CSTRLEN(L"tmpfbuf="),len - CSTRLEN(L"tmpfbuf="));
        TrimString(szBufferFile);
        PathUnquoteSpaces(szBufferFile);
        NormalizePathEx(szBufferFile,COUNTOF(szBufferFile));
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
            BOOL bFindUp  = FALSE;
            BOOL bRegex   = FALSE;
            BOOL bTransBS = FALSE;

            if (StrChr(lp1,L'-'))
              bFindUp = TRUE;
            if (StrChr(lp1,L'R'))
              bRegex = TRUE;
            if (StrChr(lp1,L'B'))
              bTransBS = TRUE;

            if (ExtractFirstArgument(lp2,lp1,lp2,len)) {
              if (lpMatchArg)
                GlobalFree(lpMatchArg);
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
          bIsNotepadReplacement = TRUE;
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

      if (lpFileArg)
        GlobalFree(lpFileArg);
      
      lpFileArg = GlobalAlloc(GPTR,sizeof(WCHAR)*FILE_ARG_BUF); // changed for ActivatePrevInst() needs
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

      bContinue = FALSE;
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
    if (IniSectionGetInt(pIniSection,L"ShellUseSystemMRU",0))
      flagUseSystemMRU = 2;
  }

  LocalFree(pIniSection);
}


//=============================================================================
//
//  FindIniFile()
//
//
BOOL CheckIniFile(LPWSTR lpszFile,LPCWSTR lpszModule)
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
      return TRUE;
    }
    // %appdata%
    //if (S_OK == SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, tchBuild)) {
    if (GetKnownFolderPath(&FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild,COUNTOF(tchBuild),tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile,MAX_PATH,tchBuild);
        return TRUE;
      }
    }
    // general
    if (SearchPath(NULL,tchFileExpanded,NULL,COUNTOF(tchBuild),tchBuild,NULL)) {
      StringCchCopy(lpszFile,MAX_PATH,tchBuild);
      return TRUE;
    }
  }

  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile,MAX_PATH,tchFileExpanded);
    return TRUE;
  }

  return FALSE;
}

BOOL CheckIniFileRedirect(LPWSTR lpszFile,LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(L"Notepad3",L"Notepad3.ini",L"",tch,COUNTOF(tch),lpszFile)) {
    if (CheckIniFile(tch,lpszModule)) {
      StringCchCopy(lpszFile,MAX_PATH,tch);
      return TRUE;
    }
    else {
      WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
      ExpandEnvironmentStrings(tch,tchFileExpanded,COUNTOF(tchFileExpanded));
      if (PathIsRelative(tchFileExpanded)) {
        StringCchCopy(lpszFile,MAX_PATH,lpszModule);
        StringCchCopy(PathFindFileName(lpszFile),MAX_PATH,tchFileExpanded);
        return TRUE;
      }
      else {
        StringCchCopy(lpszFile,MAX_PATH,tchFileExpanded);
        return TRUE;
      }
    }
  }
  return FALSE;
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
    BOOL bFound = CheckIniFile(tchTest,tchModule);

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
  if (delay < USER_TIMER_MINIMUM) {
    EditMarkAllOccurrences();
    return;
  }
  TEST_AND_SET(TIMER_BIT_MARK_OCC);
  SetTimer(g_hwndMain, IDT_TIMER_MAIN_MRKALL, delay, NULL);
}

//=============================================================================
//
//  UpdateVisibleUrlHotspot()
// 
void UpdateVisibleUrlHotspot(int delay)
{
  if (delay < USER_TIMER_MINIMUM) {
    EditUpdateVisibleUrlHotspot(bHyperlinkHotspot);
    return;
  }
  TEST_AND_SET(TIMER_BIT_UPDATE_HYPER);
  SetTimer(g_hwndMain, IDT_TIMER_UPDATE_HOTSPOT, delay, NULL);
}


//=============================================================================
//
//  UpdateToolbar()
//
#define EnableTool(id,b) SendMessage(g_hwndToolbar,TB_ENABLEBUTTON,id, \
                           MAKELONG(((b) ? 1 : 0), 0))

#define CheckTool(id,b)  SendMessage(g_hwndToolbar,TB_CHECKBUTTON,id, \
                           MAKELONG(b,0))

void UpdateToolbar()
{
  SetWindowTitle(g_hwndMain, uidsAppTitle, flagIsElevated, IDS_UNTITLED, g_wchCurFile,
                 iPathNameFormat, IsDocumentModified || Encoding_HasChanged(CPI_GET),
                 IDS_READONLY, bReadOnly, szTitleExcerpt);

  if (!bShowToolbar) { return; }

  EnableTool(IDT_FILE_ADDTOFAV,StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile)));

  EnableTool(IDT_EDIT_UNDO,SendMessage(g_hwndEdit,SCI_CANUNDO,0,0) /*&& !bReadOnly*/);
  EnableTool(IDT_EDIT_REDO,SendMessage(g_hwndEdit,SCI_CANREDO,0,0) /*&& !bReadOnly*/);
  EnableTool(IDT_EDIT_PASTE,SendMessage(g_hwndEdit,SCI_CANPASTE,0,0) /*&& !bReadOnly*/);

  BOOL b1 = SciCall_IsSelectionEmpty();
  BOOL b2 = (BOOL)(SciCall_GetTextLength() > 0);

  EnableTool(IDT_EDIT_FIND, b2);
  //EnableTool(IDT_EDIT_FINDNEXT,b2);
  //EnableTool(IDT_EDIT_FINDPREV,b2 && strlen(g_efrData.szFind));
  EnableTool(IDT_EDIT_REPLACE, b2 /*&& !bReadOnly*/);

  EnableTool(IDT_EDIT_CUT, !b1 /*&& !bReadOnly*/);
  EnableTool(IDT_EDIT_COPY, !b1 /*&& !bReadOnly*/);
  EnableTool(IDT_EDIT_CLEAR, !b1 /*&& !bReadOnly*/);

  EnableTool(IDT_VIEW_TOGGLEFOLDS, b2 && bShowCodeFolding);
  EnableTool(IDT_FILE_LAUNCH, b2);

  EnableTool(IDT_FILE_SAVE, (IsDocumentModified || Encoding_HasChanged(CPI_GET)) /*&& !bReadOnly*/);

  CheckTool(IDT_VIEW_WORDWRAP,bWordWrap);

}


//=============================================================================
//
//  UpdateStatusbar()
//
//
void UpdateStatusbar() 
{
  static WCHAR tchLn[32] = { L'\0' };
  static WCHAR tchLines[32] = { L'\0' };
  static WCHAR tchCol[32] = { L'\0' };
  static WCHAR tchCols[32] = { L'\0' };
  static WCHAR tchSel[32] = { L'\0' };
  static WCHAR tchSelB[32] = { L'\0' };
  static WCHAR tchOcc[32] = { L'\0' };
  static WCHAR tchDocPos[256] = { L'\0' };

  static WCHAR tchBytes[64] = { L'\0' };
  static WCHAR tchDocSize[64] = { L'\0' };
  static WCHAR tchEncoding[64] = { L'\0' };

  static WCHAR tchEOLMode[32] = { L'\0' };
  static WCHAR tchOvrMode[32] = { L'\0' };
  static WCHAR tch2ndDef[32] = { L'\0' };
  static WCHAR tchLexerName[128] = { L'\0' };
  static WCHAR tchLinesSelected[32] = { L'\0' };
  
  static WCHAR tchTmp[32] = { L'\0' };

  if (!bShowStatusbar) { return; }

  const DocPos iPos = SciCall_GetCurrentPos();
  const DocPos iTextLength = SciCall_GetTextLength();
  const int iEncoding = Encoding_Current(CPI_GET);

  StringCchPrintf(tchLn, COUNTOF(tchLn), L"%i", SciCall_LineFromPosition(iPos) + 1);
  FormatNumberStr(tchLn);

  StringCchPrintf(tchLines, COUNTOF(tchLines), L"%i", SciCall_GetLineCount());
  FormatNumberStr(tchLines);

  DocPos iCol = SciCall_GetColumn(iPos) + 1;
  iCol += (DocPos)SendMessage(g_hwndEdit, SCI_GETSELECTIONNCARETVIRTUALSPACE, 0, 0);
  StringCchPrintf(tchCol, COUNTOF(tchCol), L"%i", iCol);
  FormatNumberStr(tchCol);

  if (bMarkLongLines) {
    StringCchPrintf(tchCols, COUNTOF(tchCols), L"%i", iLongLinesLimit);
    FormatNumberStr(tchCols);
  }

  // Print number of selected chars in statusbar
  const BOOL bIsSelEmpty = SciCall_IsSelectionEmpty();
  const DocPos iSelStart = (bIsSelEmpty ? 0 : SciCall_GetSelectionStart());
  const DocPos iSelEnd = (bIsSelEmpty ? 0 : SciCall_GetSelectionEnd());

  if (!bIsSelEmpty && !SciCall_IsSelectionRectangle())
  {
    const int iSel = (int)SendMessage(g_hwndEdit, SCI_COUNTCHARACTERS, iSelStart, iSelEnd);
    StringCchPrintf(tchSel, COUNTOF(tchSel), L"%i", iSel);
    FormatNumberStr(tchSel);
    StrFormatByteSize((iSelEnd - iSelStart), tchSelB, COUNTOF(tchSelB));
  }
  else {
    tchSel[0] = L'-'; tchSel[1] = L'-'; tchSel[2] = L'\0';
    tchSelB[0] = L'0'; tchSelB[1] = L'\0';
  }

  // Print number of occurrence marks found
  if ((iMarkOccurrencesCount > 0) && !bMarkOccurrencesMatchVisible) 
  {
    if ((iMarkOccurrencesMaxCount < 0) || (iMarkOccurrencesCount < iMarkOccurrencesMaxCount)) 
    {
      StringCchPrintf(tchOcc, COUNTOF(tchOcc), L"%i", iMarkOccurrencesCount);
      FormatNumberStr(tchOcc);
    }
    else {
      StringCchPrintf(tchTmp, COUNTOF(tchTmp), L"%i", iMarkOccurrencesCount);
      FormatNumberStr(tchTmp);
      StringCchPrintf(tchOcc, COUNTOF(tchOcc), L">= %s", tchTmp);
    }
  }
  else {
    StringCchCopy(tchOcc, COUNTOF(tchOcc), L"--");
  }

  // Print number of selected lines in statusbar
  if (bIsSelEmpty) {
    tchLinesSelected[0] = L'-';
    tchLinesSelected[1] = L'-';
    tchLinesSelected[2] = L'\0';
  }
  else {
    const DocLn iLineStart = SciCall_LineFromPosition(iSelStart);
    const DocLn iLineEnd = SciCall_LineFromPosition(iSelEnd);
    const DocPos iStartOfLinePos = SciCall_PositionFromLine(iLineEnd);
    DocLn iLinesSelected = (iLineEnd - iLineStart);
    if ((iSelStart != iSelEnd) && (iStartOfLinePos != iSelEnd)) { iLinesSelected += 1; }
    StringCchPrintf(tchLinesSelected, COUNTOF(tchLinesSelected), L"%i", iLinesSelected);
    FormatNumberStr(tchLinesSelected);
  }

  if (!bMarkLongLines) {
    FormatString(tchDocPos, COUNTOF(tchDocPos), IDS_DOCPOS, tchLn, tchLines, tchCol, tchSel, tchSelB, tchLinesSelected, tchOcc);
  }
  else {
    FormatString(tchDocPos, COUNTOF(tchDocPos), IDS_DOCPOS2, tchLn, tchLines, tchCol, tchCols, tchSel, tchSelB, tchLinesSelected, tchOcc);
  }

  // get number of bytes in current encoding
  StrFormatByteSize(iTextLength, tchBytes, COUNTOF(tchBytes));
  FormatString(tchDocSize, COUNTOF(tchDocSize), IDS_DOCSIZE, tchBytes);

  Encoding_SetLabel(iEncoding);
  StringCchPrintf(tchEncoding, COUNTOF(tchEncoding), L" %s ", g_Encodings[iEncoding].wchLabel);

  if (iEOLMode == SC_EOL_CR) 
  {
    StringCchCopy(tchEOLMode, COUNTOF(tchEOLMode), L" CR ");
  }
  else if (iEOLMode == SC_EOL_LF) 
  {
    StringCchCopy(tchEOLMode, COUNTOF(tchEOLMode), L" LF ");
  }
  else {
    StringCchCopy(tchEOLMode, COUNTOF(tchEOLMode), L" CR+LF ");
  }
  if (SendMessage(g_hwndEdit, SCI_GETOVERTYPE, 0, 0)) 
  {
    StringCchCopy(tchOvrMode, COUNTOF(tchOvrMode), L" OVR ");
  }
  else {
    StringCchCopy(tchOvrMode, COUNTOF(tchOvrMode), L" INS ");
  }
  if (Style_GetUse2ndDefault())
  {
    StringCchCopy(tch2ndDef, COUNTOF(tch2ndDef), L" 2ND ");
  }
  else {
    StringCchCopy(tch2ndDef, COUNTOF(tch2ndDef), L" STD ");
  }
  Style_GetCurrentLexerName(tchLexerName, COUNTOF(tchLexerName));

  StatusSetText(g_hwndStatus, STATUS_DOCPOS, tchDocPos);
  StatusSetText(g_hwndStatus, STATUS_DOCSIZE, tchDocSize);
  StatusSetText(g_hwndStatus, STATUS_CODEPAGE, tchEncoding);
  StatusSetText(g_hwndStatus, STATUS_EOLMODE, tchEOLMode);
  StatusSetText(g_hwndStatus, STATUS_OVRMODE, tchOvrMode);
  StatusSetText(g_hwndStatus, STATUS_2ND_DEF, tch2ndDef);
  StatusSetText(g_hwndStatus, STATUS_LEXER, tchLexerName);

  //InvalidateRect(g_hwndStatus,NULL,TRUE);
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
    StringCchPrintfA(chLines, COUNTOF(chLines), "_%i_", SciCall_GetLineCount());

    int iLineMarginWidthNow = (int)SendMessage(g_hwndEdit, SCI_GETMARGINWIDTHN, MARGIN_NP3_LINENUM, 0);
    int iLineMarginWidthFit = (int)SendMessage(g_hwndEdit, SCI_TEXTWIDTH, STYLE_LINENUMBER, (LPARAM)chLines);

    if (iLineMarginWidthNow != iLineMarginWidthFit) {
      SendMessage(g_hwndEdit, SCI_SETMARGINWIDTHN, MARGIN_NP3_LINENUM, iLineMarginWidthFit);
    }
  }
  else {
    SendMessage(g_hwndEdit, SCI_SETMARGINWIDTHN, MARGIN_NP3_LINENUM, 0);
  }
}


//=============================================================================
//
//  UpdateSettingsCmds()
//
//
void UpdateSettingsCmds()
{
    HMENU hmenu = GetSystemMenu(g_hwndMain, FALSE);
    BOOL hasIniFile = (StringCchLenW(g_wchIniFile,COUNTOF(g_wchIniFile)) > 0 || StringCchLenW(g_wchIniFile2,COUNTOF(g_wchIniFile2)) > 0);
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
//  InvalidateSelections()
//
//
void InvalidateSelections()
{
  // Invalidate invalid selections
  // #pragma message("TODO: Remove check for invalid selections once fixed in Scintilla")
  if (SendMessage(g_hwndEdit, SCI_GETSELECTIONS, 0, 0) > 1 && !SciCall_IsSelectionRectangle()) {
    int iCurPos = (int)SendMessage(g_hwndEdit, SCI_GETCURRENTPOS, 0, 0);
    SendMessage(g_hwndEdit, WM_CANCELMODE, 0, 0);
    SendMessage(g_hwndEdit, SCI_CLEARSELECTIONS, 0, 0);
    SendMessage(g_hwndEdit, SCI_SETSEL, iCurPos, iCurPos);
  }
}


//=============================================================================
//
//  BeginUndoAction()
//
//
int BeginUndoAction()
{
  int token = -1;
  UndoRedoSelection_t sel = INIT_UNDOREDOSEL;
  sel.selMode_undo = (int)SendMessage(g_hwndEdit,SCI_GETSELECTIONMODE,0,0);

  switch (sel.selMode_undo)
  {
  case SC_SEL_RECTANGLE:
  case SC_SEL_THIN:
    sel.anchorPos_undo = (int)SendMessage(g_hwndEdit, SCI_GETRECTANGULARSELECTIONANCHOR, 0, 0);
    sel.curPos_undo = (int)SendMessage(g_hwndEdit, SCI_GETRECTANGULARSELECTIONCARET, 0, 0);
    if (!bDenyVirtualSpaceAccess) {
      sel.anchorVS_undo = (int)SendMessage(g_hwndEdit, SCI_GETRECTANGULARSELECTIONANCHORVIRTUALSPACE, 0, 0);
      sel.curVS_undo = (int)SendMessage(g_hwndEdit, SCI_GETRECTANGULARSELECTIONCARETVIRTUALSPACE, 0, 0);
    }
    break;

  case SC_SEL_LINES:
  case SC_SEL_STREAM:
  default:
    sel.anchorPos_undo = (int)SendMessage(g_hwndEdit, SCI_GETANCHOR, 0, 0);
    sel.curPos_undo = (int)SendMessage(g_hwndEdit, SCI_GETCURRENTPOS, 0, 0);
    break;
  }
  token = UndoRedoActionMap(-1, &sel);
  if (token >= 0) {
    SendMessage(g_hwndEdit, SCI_BEGINUNDOACTION, 0, 0);
    SendMessage(g_hwndEdit, SCI_ADDUNDOACTION, (WPARAM)token, 0);
  }
  return token;
}



//=============================================================================
//
//  EndUndoAction()
//
//
void EndUndoAction(int token)
{
  if (token >= 0) {
    UndoRedoSelection_t sel = INIT_UNDOREDOSEL;
    if (UndoRedoActionMap(token, &sel) >= 0) {

      sel.selMode_redo = (int)SendMessage(g_hwndEdit, SCI_GETSELECTIONMODE, 0, 0);

      switch (sel.selMode_redo)
      {
      case SC_SEL_RECTANGLE:
      case SC_SEL_THIN:
        sel.anchorPos_redo = (int)SendMessage(g_hwndEdit, SCI_GETRECTANGULARSELECTIONANCHOR, 0, 0);
        sel.curPos_redo = (int)SendMessage(g_hwndEdit, SCI_GETRECTANGULARSELECTIONCARET, 0, 0);
        if (!bDenyVirtualSpaceAccess) {
          sel.anchorVS_redo = (int)SendMessage(g_hwndEdit, SCI_GETRECTANGULARSELECTIONANCHORVIRTUALSPACE, 0, 0);
        }
        break;

      case SC_SEL_LINES:
      case SC_SEL_STREAM:
      default:
        sel.anchorPos_redo = (int)SendMessage(g_hwndEdit, SCI_GETANCHOR, 0, 0);
        sel.curPos_redo = (int)SendMessage(g_hwndEdit, SCI_GETCURRENTPOS, 0, 0);
        break;
      }
    }
    UndoRedoActionMap(token,&sel); // set with redo action filled
    SendMessage(g_hwndEdit, SCI_ENDUNDOACTION, 0, 0);
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

    ISSUE_MESSAGE(g_hwndEdit, SCI_CANCEL, 0, 0); // prepare - not needed ?

    const DocPos _anchorPos = (doAct == UNDO ? sel.anchorPos_undo : sel.anchorPos_redo);
    const DocPos _curPos = (doAct == UNDO ? sel.curPos_undo : sel.curPos_redo);

    // Ensure that the first and last lines of a selection are always unfolded
    // This needs to be done _before_ the SCI_SETSEL message
    const DocLn anchorPosLine = SciCall_LineFromPosition(_anchorPos);
    const DocLn currPosLine = SciCall_LineFromPosition(_curPos);
    ISSUE_MESSAGE(g_hwndEdit, SCI_ENSUREVISIBLE, anchorPosLine, 0);
    if (anchorPosLine != currPosLine) { ISSUE_MESSAGE(g_hwndEdit, SCI_ENSUREVISIBLE, currPosLine, 0); }


    const int selectionMode = (doAct == UNDO ? sel.selMode_undo : sel.selMode_redo);
    ISSUE_MESSAGE(g_hwndEdit, SCI_SETSELECTIONMODE, (WPARAM)selectionMode, 0);

    // independant from selection mode
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
        int anchorVS = (doAct == UNDO ? sel.anchorVS_undo : sel.anchorVS_redo);
        int currVS = (doAct == UNDO ? sel.curVS_undo : sel.curVS_redo);
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

    //ISSUE_MESSAGE(g_hwndEdit, SCI_CANCEL, 0, 0);

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
  if (UndoRedoSelectionUTArray == NULL) { return -1; }

  static unsigned int iTokenCnt = 0;

  // indexing is unsigned
  unsigned int utoken = (token >= 0) ? (unsigned int)token : 0U;

  if (selection == NULL) {
    // reset / clear
    SendMessage(g_hwndEdit, SCI_EMPTYUNDOBUFFER, 0, 0);
    utarray_clear(UndoRedoSelectionUTArray);
    utarray_init(UndoRedoSelectionUTArray, &UndoRedoSelection_icd);
    iTokenCnt = 0U;
    return -1;
  }

  if (!SciCall_GetUndoCollection()) { return -1; }

  // get or set map item request ?
  if ((token >= 0) && (utoken < iTokenCnt)) 
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
    utarray_insert(UndoRedoSelectionUTArray, (void*)selection, iTokenCnt);
    token = (int)iTokenCnt;
    iTokenCnt = (iTokenCnt < INT_MAX) ? (iTokenCnt + 1) : 0U;  // round robin next
  }
  return token;
}


//=============================================================================
//
//  FileIO()
//
//
BOOL FileIO(BOOL fLoad,LPCWSTR pszFileName,BOOL bNoEncDetect,int *ienc,int *ieol,
            BOOL *pbUnicodeErr,BOOL *pbFileTooBig, BOOL* pbUnknownExt,
            BOOL *pbCancelDataLoss,BOOL bSaveCopy)
{
  WCHAR tch[MAX_PATH+40];
  BOOL fSuccess;
  DWORD dwFileAttributes;

  FormatString(tch,COUNTOF(tch),(fLoad) ? IDS_LOADFILE : IDS_SAVEFILE,PathFindFileName(pszFileName));

  BeginWaitCursor(tch);

  if (fLoad) {
    fSuccess = EditLoadFile(g_hwndEdit,pszFileName,bNoEncDetect,ienc,ieol,pbUnicodeErr,pbFileTooBig,pbUnknownExt);
  }
  else {
    int idx;
    if (MRU_FindFile(pFileMRU,pszFileName,&idx)) {
      pFileMRU->iEncoding[idx] = *ienc;
      pFileMRU->iCaretPos[idx] = (bPreserveCaretPos) ? (int)SendMessage(g_hwndEdit,SCI_GETCURRENTPOS,0,0) : 0;
      WCHAR wchBookMarks[MRU_BMRK_SIZE] = { L'\0' };
      EditGetBookmarkList(g_hwndEdit, wchBookMarks, COUNTOF(wchBookMarks));
      if (pFileMRU->pszBookMarks[idx])
        LocalFree(pFileMRU->pszBookMarks[idx]);
      pFileMRU->pszBookMarks[idx] = StrDup(wchBookMarks);
    }
    fSuccess = EditSaveFile(g_hwndEdit,pszFileName,*ienc,pbCancelDataLoss,bSaveCopy);
  }

  dwFileAttributes = GetFileAttributes(pszFileName);
  bReadOnly = (dwFileAttributes != INVALID_FILE_ATTRIBUTES && dwFileAttributes & FILE_ATTRIBUTE_READONLY);

  EndWaitCursor();

  return(fSuccess);
}


//=============================================================================
//
//  FileLoad()
//
//
BOOL FileLoad(BOOL bDontSave,BOOL bNew,BOOL bReload,BOOL bNoEncDetect,LPCWSTR lpszFile)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  WCHAR szFileName[MAX_PATH] = { L'\0' };
  BOOL bUnicodeErr = FALSE;
  BOOL bFileTooBig = FALSE;
  BOOL bUnknownExt = FALSE;
  BOOL fSuccess;
  int fileEncoding = CPI_ANSI_DEFAULT;

  if (!bDontSave)
  {
    if (!FileSave(FALSE,TRUE,FALSE,FALSE))
      return FALSE;
  }

  if (!bReload) { ResetEncryption(); }

  if (bNew) {
    StringCchCopy(g_wchCurFile,COUNTOF(g_wchCurFile),L"");
    SetDlgItemText(g_hwndMain,IDC_FILENAME,g_wchCurFile);
    SetDlgItemInt(g_hwndMain,IDC_REUSELOCK,GetTickCount(),FALSE);
    if (!fKeepTitleExcerpt)
      StringCchCopy(szTitleExcerpt,COUNTOF(szTitleExcerpt),L"");
    FileVars_Init(NULL,0,&fvCurFile);
    EditSetNewText(g_hwndEdit,"",0);
    Style_SetLexer(g_hwndEdit,NULL);

    iEOLMode = iLineEndings[iDefaultEOLMode];
    SendMessage(g_hwndEdit,SCI_SETEOLMODE,iLineEndings[iDefaultEOLMode],0);
    Encoding_Current(iDefaultEncoding);
    Encoding_HasChanged(iDefaultEncoding);
    Encoding_SciSetCodePage(g_hwndEdit,iDefaultEncoding);
    EditSetNewText(g_hwndEdit,"",0);

    bReadOnly = FALSE;
    SetDocumentModified(FALSE);
    UpdateToolbar();
    UpdateStatusbar();
    UpdateLineNumberWidth();

    // Terminate file watching
    if (bResetFileWatching)
      iFileWatchingMode = 0;
    InstallFileWatching(NULL);
    bEnableSaveSettings = TRUE;
    UpdateSettingsCmds();
    return TRUE;
  }

  if (!lpszFile || lstrlen(lpszFile) == 0) {
    if (!OpenFileDlg(g_hwndMain,tch,COUNTOF(tch),NULL))
      return FALSE;
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
        iEOLMode = iLineEndings[iDefaultEOLMode];
        SendMessage(g_hwndEdit,SCI_SETEOLMODE,iLineEndings[iDefaultEOLMode],0);
        if (Encoding_Source(CPI_GET) != CPI_NONE) {
          fileEncoding = Encoding_Source(CPI_GET);
          Encoding_Current(fileEncoding);
          Encoding_HasChanged(fileEncoding);
        }
        else {
          Encoding_Current(iDefaultEncoding);
          Encoding_HasChanged(iDefaultEncoding);
        }
        Encoding_SciSetCodePage(g_hwndEdit,Encoding_Current(CPI_GET));
        bReadOnly = FALSE;
        EditSetNewText(g_hwndEdit,"",0);
      }
      if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE)) {
        CloseHandle(hFile);
      }
    }
    else
      return FALSE;
  }
  else {
    int idx;
    if (!bReload && MRU_FindFile(pFileMRU,szFileName,&idx)) {
      fileEncoding = pFileMRU->iEncoding[idx];
      if (fileEncoding > 0)
        Encoding_Source(Encoding_MapUnicode(fileEncoding));
    }
    else
      fileEncoding = Encoding_Current(CPI_GET);

    fSuccess = FileIO(TRUE,szFileName,bNoEncDetect,&fileEncoding,&iEOLMode,&bUnicodeErr,&bFileTooBig,&bUnknownExt,NULL,FALSE);
    if (fSuccess)
      Encoding_Current(fileEncoding); // load may change encoding
  }
  if (fSuccess) {
    StringCchCopy(g_wchCurFile,COUNTOF(g_wchCurFile),szFileName);
    SetDlgItemText(g_hwndMain,IDC_FILENAME,g_wchCurFile);
    SetDlgItemInt(g_hwndMain,IDC_REUSELOCK,GetTickCount(),FALSE);

    if (!fKeepTitleExcerpt)
      StringCchCopy(szTitleExcerpt,COUNTOF(szTitleExcerpt),L"");

    if (!flagLexerSpecified) // flag will be cleared
      Style_SetLexerFromFile(g_hwndEdit,g_wchCurFile);

    SendMessage(g_hwndEdit,SCI_SETEOLMODE,iEOLMode,0);
    fileEncoding = Encoding_Current(CPI_GET);
    Encoding_HasChanged(fileEncoding);
    int idx, iCaretPos = 0;
    LPCWSTR pszBookMarks = L"";
    if (!bReload && MRU_FindFile(pFileMRU,szFileName,&idx)) {
      iCaretPos = pFileMRU->iCaretPos[idx];
      pszBookMarks = pFileMRU->pszBookMarks[idx];
    }
    MRU_AddFile(pFileMRU,szFileName,flagRelativeFileMRU,flagPortableMyDocs,fileEncoding,iCaretPos,pszBookMarks);
    
    EditSetBookmarkList(g_hwndEdit, pszBookMarks);

    if (flagUseSystemMRU == 2)
      SHAddToRecentDocs(SHARD_PATHW,szFileName);

    // Install watching of the current file
    if (!bReload && bResetFileWatching)
      iFileWatchingMode = 0;
    InstallFileWatching(g_wchCurFile);

    // the .LOG feature ...
    if (SciCall_GetTextLength() >= 4) {
      char tchLog[5] = { '\0' };
      SendMessage(g_hwndEdit,SCI_GETTEXT,5,(LPARAM)tchLog);
      if (StringCchCompareXA(tchLog,".LOG") == 0) {
        EditJumpTo(g_hwndEdit,-1,0);
        SendMessage(g_hwndEdit,SCI_BEGINUNDOACTION,0,0);
        SendMessage(g_hwndEdit,SCI_NEWLINE,0,0);
        SendMessage(g_hwndMain,WM_COMMAND,MAKELONG(IDM_EDIT_INSERT_SHORTDATE,1),0);
        EditJumpTo(g_hwndEdit,-1,0);
        SendMessage(g_hwndEdit,SCI_NEWLINE,0,0);
        SendMessage(g_hwndEdit,SCI_ENDUNDOACTION,0,0);
        SendMessage(g_hwndEdit, SCI_DOCUMENTEND, 0, 0);
        EditEnsureSelectionVisible(g_hwndEdit);
      }
      // set historic caret pos
      else if (iCaretPos > 0) 
      {
        SendMessage(g_hwndEdit, SCI_GOTOPOS, (WPARAM)iCaretPos, 0);
        // adjust view
        int iCurPos = (int)SendMessage(g_hwndEdit, SCI_GETCURRENTPOS, 0, 0);
        int iLine   = (int)SendMessage(g_hwndEdit, SCI_LINEFROMPOSITION, (WPARAM)iCurPos, 0);
        int iCol    = (int)SendMessage(g_hwndEdit, SCI_GETCOLUMN, (WPARAM)iCurPos, 0);
        EditJumpTo(g_hwndEdit, iLine+1, iCol+1);
      }
    }

    //bReadOnly = FALSE;
    SetDocumentModified(FALSE);
    UpdateToolbar();
    UpdateStatusbar();
    UpdateLineNumberWidth();
    UpdateVisibleUrlHotspot(0);

    // consistent settings file handling (if loaded in editor)
    bEnableSaveSettings = (StringCchCompareINW(g_wchCurFile, COUNTOF(g_wchCurFile), g_wchIniFile, COUNTOF(g_wchIniFile)) == 0) ? FALSE : TRUE;
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
BOOL FileRevert(LPCWSTR szFileName) 
{
  if (wcslen(szFileName)) {

    int iCurPos = (int)SendMessage(g_hwndEdit,SCI_GETCURRENTPOS,0,0);
    int iAnchorPos = (int)SendMessage(g_hwndEdit,SCI_GETANCHOR,0,0);
    int iCurrLine = (int)SendMessage(g_hwndEdit, SCI_LINEFROMPOSITION, (WPARAM)iCurPos, 0);
    int iVisTopLine = (int)SendMessage(g_hwndEdit,SCI_GETFIRSTVISIBLELINE,0,0);
    int iDocTopLine = (int)SendMessage(g_hwndEdit,SCI_DOCLINEFROMVISIBLE,(WPARAM)iVisTopLine,0);
    int iXOffset = (int)SendMessage(g_hwndEdit,SCI_GETXOFFSET,0,0);
    //BOOL bIsTail = (iCurPos == iAnchorPos) && (iCurPos == SendMessage(g_hwndEdit, SCI_GETLENGTH, 0, 0));
    BOOL bIsTail = (iCurPos == iAnchorPos) && (iCurrLine >= (SciCall_GetLineCount() - 1));

    Encoding_SrcWeak(Encoding_Current(CPI_GET));

    WCHAR tchFileName2[MAX_PATH] = { L'\0' };
    StringCchCopy(tchFileName2,COUNTOF(tchFileName2),szFileName);

    if (FileLoad(TRUE,FALSE,TRUE,FALSE,tchFileName2))
    {
      if (bIsTail && iFileWatchingMode == 2) {
        SendMessage(g_hwndEdit, SCI_DOCUMENTEND, 0, 0);
        EditEnsureSelectionVisible(g_hwndEdit);
      }
      else if (SendMessage(g_hwndEdit,SCI_GETLENGTH,0,0) >= 4) {
        char tch[5] = { '\0' };
        SendMessage(g_hwndEdit,SCI_GETTEXT,5,(LPARAM)tch);
        if (StringCchCompareXA(tch,".LOG") != 0) {
          int iNewTopLine;
          SendMessage(g_hwndEdit,SCI_SETSEL,iAnchorPos,iCurPos);
          SendMessage(g_hwndEdit,SCI_ENSUREVISIBLE,(WPARAM)iDocTopLine,0);
          iNewTopLine = (int)SendMessage(g_hwndEdit,SCI_GETFIRSTVISIBLELINE,0,0);
          SendMessage(g_hwndEdit,SCI_LINESCROLL,0,(LPARAM)iVisTopLine - iNewTopLine);
          SendMessage(g_hwndEdit,SCI_SETXOFFSET,(WPARAM)iXOffset,0);
        }
      }
      return TRUE;
    }
  }
  return FALSE;
}


//=============================================================================
//
//  FileSave()
//
//
BOOL FileSave(BOOL bSaveAlways,BOOL bAsk,BOOL bSaveAs,BOOL bSaveCopy)
{
  WCHAR tchFile[MAX_PATH] = { L'\0' };
  WCHAR tchBase[MAX_PATH] = { L'\0' };
  BOOL fSuccess = FALSE;
  BOOL bCancelDataLoss = FALSE;

  BOOL bIsEmptyNewFile = FALSE;
  if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile)) == 0) {
    int cchText = (int)SendMessage(g_hwndEdit,SCI_GETLENGTH,0,0);
    if (cchText == 0)
      bIsEmptyNewFile = TRUE;
    else if (cchText < 1023) {
      char tchText[2048];
      SendMessage(g_hwndEdit,SCI_GETTEXT,(WPARAM)2047,(LPARAM)tchText);
      StrTrimA(tchText," \t\n\r");
      if (lstrlenA(tchText) == 0)
        bIsEmptyNewFile = TRUE;
    }
  }

  if (!bSaveAlways && (!IsDocumentModified && !Encoding_HasChanged(CPI_GET) || bIsEmptyNewFile) && !bSaveAs) {
    int idx;
    if (MRU_FindFile(pFileMRU,g_wchCurFile,&idx)) {
      pFileMRU->iEncoding[idx] = Encoding_Current(CPI_GET);
      pFileMRU->iCaretPos[idx] = (bPreserveCaretPos) ? (int)SendMessage(g_hwndEdit,SCI_GETCURRENTPOS,0,0) : 0;
      WCHAR wchBookMarks[MRU_BMRK_SIZE] = { L'\0' };
      EditGetBookmarkList(g_hwndEdit, wchBookMarks, COUNTOF(wchBookMarks));
      if (pFileMRU->pszBookMarks[idx])
        LocalFree(pFileMRU->pszBookMarks[idx]);
      pFileMRU->pszBookMarks[idx] = StrDup(wchBookMarks);
    }
    return TRUE;
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
        return FALSE;
      case IDNO:
        return TRUE;
    }
  }

  // Read only...
  if (!bSaveAs && !bSaveCopy && StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile)))
  {
    DWORD dwFileAttributes = GetFileAttributes(g_wchCurFile);
    if (dwFileAttributes != INVALID_FILE_ATTRIBUTES)
      bReadOnly = (dwFileAttributes & FILE_ATTRIBUTE_READONLY);
    if (bReadOnly) {
      UpdateToolbar();
      if (MsgBox(MBYESNOWARN,IDS_READONLY_SAVE,g_wchCurFile) == IDYES)
        bSaveAs = TRUE;
      else
        return FALSE;
    }
  }

  // Save As...
  if (bSaveAs || bSaveCopy || StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile)) == 0)
  {
    WCHAR tchInitialDir[MAX_PATH] = { L'\0' };
    if (bSaveCopy && StringCchLenW(tchLastSaveCopyDir,COUNTOF(tchLastSaveCopyDir))) {
      StringCchCopy(tchInitialDir,COUNTOF(tchInitialDir),tchLastSaveCopyDir);
      StringCchCopy(tchFile,COUNTOF(tchFile),tchLastSaveCopyDir);
      PathCchAppend(tchFile,COUNTOF(tchFile),PathFindFileName(g_wchCurFile));
    }
    else
      StringCchCopy(tchFile,COUNTOF(tchFile),g_wchCurFile);

    if (SaveFileDlg(g_hwndMain,tchFile,COUNTOF(tchFile),tchInitialDir))
    {
      int fileEncoding = Encoding_Current(CPI_GET);
      fSuccess = FileIO(FALSE, tchFile, FALSE, &fileEncoding, &iEOLMode, NULL, NULL, NULL, &bCancelDataLoss, bSaveCopy);
      //~if (fSuccess) Encoding_Current(fileEncoding); // save should not change encoding
      if (fSuccess)
      {
        if (!bSaveCopy)
        {
          StringCchCopy(g_wchCurFile,COUNTOF(g_wchCurFile),tchFile);
          SetDlgItemText(g_hwndMain,IDC_FILENAME,g_wchCurFile);
          SetDlgItemInt(g_hwndMain,IDC_REUSELOCK,GetTickCount(),FALSE);
          if (!fKeepTitleExcerpt)
            StringCchCopy(szTitleExcerpt,COUNTOF(szTitleExcerpt),L"");
          Style_SetLexerFromFile(g_hwndEdit,g_wchCurFile);
          UpdateToolbar();
          UpdateStatusbar();
          UpdateLineNumberWidth();
        }
        else {
          StringCchCopy(tchLastSaveCopyDir,COUNTOF(tchLastSaveCopyDir),tchFile);
          PathRemoveFileSpec(tchLastSaveCopyDir);
        }
      }
    }
    else
      return FALSE;
  }
  else {
    int fileEncoding = Encoding_Current(CPI_GET);
    fSuccess = FileIO(FALSE,g_wchCurFile,FALSE,&fileEncoding,&iEOLMode,NULL,NULL,NULL,&bCancelDataLoss,FALSE);
    //~if (fSuccess) Encoding_Current(fileEncoding); // save should not change encoding
  }

  if (fSuccess)
  {
    if (!bSaveCopy)
    {
      int iCurrEnc = Encoding_Current(CPI_GET);
      Encoding_HasChanged(iCurrEnc);
      int iCaretPos = (int)SendMessage(g_hwndEdit, SCI_GETCURRENTPOS, 0, 0);
      WCHAR wchBookMarks[MRU_BMRK_SIZE] = { L'\0' };
      EditGetBookmarkList(g_hwndEdit, wchBookMarks, COUNTOF(wchBookMarks));
      MRU_AddFile(pFileMRU,g_wchCurFile,flagRelativeFileMRU,flagPortableMyDocs,iCurrEnc,iCaretPos,wchBookMarks);
      if (flagUseSystemMRU == 2)
        SHAddToRecentDocs(SHARD_PATHW,g_wchCurFile);

      SetDocumentModified(FALSE);
      // Install watching of the current file
      if (bSaveAs && bResetFileWatching)
        iFileWatchingMode = 0;
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
          if (FileIO(FALSE,szTempFileName,FALSE,&fileEncoding,&iEOLMode,NULL,NULL,NULL,&bCancelDataLoss,TRUE)) {
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
              SetDocumentModified(FALSE);
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
BOOL OpenFileDlg(HWND hwnd,LPWSTR lpstrFile,int cchFile,LPCWSTR lpstrInitialDir)
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
    else if (StringCchLenW(tchDefaultDir,COUNTOF(tchDefaultDir))) {
      ExpandEnvironmentStrings(tchDefaultDir,tchInitialDir,COUNTOF(tchInitialDir));
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
  ofn.lpstrDefExt = (StringCchLenW(tchDefaultExtension,COUNTOF(tchDefaultExtension))) ? tchDefaultExtension : NULL;

  if (GetOpenFileName(&ofn)) {
    StringCchCopyN(lpstrFile,cchFile,szFile,COUNTOF(szFile));
    return TRUE;
  }

  else
    return FALSE;
}


//=============================================================================
//
//  SaveFileDlg()
//
//
BOOL SaveFileDlg(HWND hwnd,LPWSTR lpstrFile,int cchFile,LPCWSTR lpstrInitialDir)
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
  else if (StringCchLenW(tchDefaultDir,COUNTOF(tchDefaultDir))) {
    ExpandEnvironmentStrings(tchDefaultDir,tchInitialDir,COUNTOF(tchInitialDir));
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
  ofn.lpstrDefExt = (StringCchLenW(tchDefaultExtension,COUNTOF(tchDefaultExtension))) ? tchDefaultExtension : NULL;

  if (GetSaveFileName(&ofn)) {
    StringCchCopyN(lpstrFile,cchFile,szNewFile,COUNTOF(szNewFile));
    return TRUE;
  }

  else
    return FALSE;
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
  return(bContinue);
}

BOOL CALLBACK EnumWndProc2(HWND hwnd,LPARAM lParam)
{
  BOOL bContinue = TRUE;
  WCHAR szClassName[64] = { L'\0' };

  if (GetClassName(hwnd,szClassName,COUNTOF(szClassName)))

    if (StringCchCompareINW(szClassName,COUNTOF(szClassName),wchWndClass,COUNTOF(wchWndClass)) == 0) {

      DWORD dwReuseLock = GetDlgItemInt(hwnd,IDC_REUSELOCK,NULL,FALSE);
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
  return(bContinue);
}

BOOL ActivatePrevInst()
{
  HWND hwnd = NULL;
  COPYDATASTRUCT cds;

  if ((flagNoReuseWindow && !flagSingleFileInstance) || flagStartAsTrayIcon || flagNewFromClipboard || flagPasteBoard)
    return(FALSE);

  if (flagSingleFileInstance && lpFileArg) {

    // Search working directory from second instance, first!
    // lpFileArg is at least MAX_PATH+4 WCHARS
    WCHAR tchTmp[FILE_ARG_BUF] = { L'\0' };

    ExpandEnvironmentStringsEx(lpFileArg,(DWORD)GlobalSize(lpFileArg)/sizeof(WCHAR));

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
        LPnp3params params;
        DWORD cb = sizeof(np3params);

        // Make sure the previous window won't pop up a change notification message
        //SendMessage(hwnd,WM_CHANGENOTIFYCLEAR,0,0);

        if (IsIconic(hwnd))
          ShowWindowAsync(hwnd,SW_RESTORE);

        if (!IsWindowVisible(hwnd)) {
          SendMessage(hwnd,WM_TRAYMESSAGE,0,WM_LBUTTONDBLCLK);
          SendMessage(hwnd,WM_TRAYMESSAGE,0,WM_LBUTTONUP);
        }

        SetForegroundWindow(hwnd);

        if (lpSchemeArg)
          cb += (lstrlen(lpSchemeArg) + 1) * sizeof(WCHAR);

        params = GlobalAlloc(GPTR,cb);
        params->flagFileSpecified = FALSE;
        params->flagChangeNotify = 0;
        params->flagQuietCreate = FALSE;
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
        cds.cbData = (DWORD)GlobalSize(params);
        cds.lpData = params;

        SendMessage(hwnd,WM_COPYDATA,(WPARAM)NULL,(LPARAM)&cds);
        GlobalFree(params);

        return(TRUE);
      }

      else // IsWindowEnabled()
      {
        // Ask...
        if (IDYES == MsgBox(MBYESNO,IDS_ERR_PREVWINDISABLED))
          return(FALSE);
        else
          return(TRUE);
      }
    }
  }

  if (flagNoReuseWindow)
    return(FALSE);

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
        LPnp3params params;
        DWORD cb = sizeof(np3params);
        int cchTitleExcerpt;

        ExpandEnvironmentStringsEx(lpFileArg,(DWORD)GlobalSize(lpFileArg)/sizeof(WCHAR));

        if (PathIsRelative(lpFileArg)) {
          StringCchCopyN(tchTmp,COUNTOF(tchTmp),g_wchWorkingDirectory,COUNTOF(g_wchWorkingDirectory));
          PathCchAppend(tchTmp,COUNTOF(tchTmp),lpFileArg);
          if (PathFileExists(tchTmp))
            StringCchCopy(lpFileArg,FILE_ARG_BUF,tchTmp);
          else {
            if (SearchPath(NULL,lpFileArg,NULL,COUNTOF(tchTmp),tchTmp,NULL))
              StringCchCopy(lpFileArg,FILE_ARG_BUF,tchTmp);
          }
        }

        else if (SearchPath(NULL,lpFileArg,NULL,COUNTOF(tchTmp),tchTmp,NULL))
          StringCchCopy(lpFileArg,FILE_ARG_BUF,tchTmp);

        cb += (lstrlen(lpFileArg) + 1) * sizeof(WCHAR);

        if (lpSchemeArg)
          cb += (lstrlen(lpSchemeArg) + 1) * sizeof(WCHAR);

        cchTitleExcerpt = StringCchLenW(szTitleExcerpt,COUNTOF(szTitleExcerpt));
        if (cchTitleExcerpt)
          cb += (cchTitleExcerpt + 1) * sizeof(WCHAR);

        params = GlobalAlloc(GPTR,cb);
        params->flagFileSpecified = TRUE;
        StringCchCopy(&params->wchData,lstrlen(lpFileArg)+1,lpFileArg);
        params->flagChangeNotify = flagChangeNotify;
        params->flagQuietCreate = flagQuietCreate;
        params->flagLexerSpecified = flagLexerSpecified;
        if (flagLexerSpecified && lpSchemeArg) {
          StringCchCopy(StrEnd(&params->wchData)+1,lstrlen(lpSchemeArg)+1,lpSchemeArg);
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

        if (cchTitleExcerpt) {
          StringCchCopy(StrEnd(&params->wchData)+1,cchTitleExcerpt+1,szTitleExcerpt);
          params->flagTitleExcerpt = 1;
        }
        else
          params->flagTitleExcerpt = 0;

        cds.dwData = DATA_NOTEPAD3_PARAMS;
        cds.cbData = (DWORD)GlobalSize(params);
        cds.lpData = params;

        SendMessage(hwnd,WM_COPYDATA,(WPARAM)NULL,(LPARAM)&cds);
        GlobalFree(params);    params = NULL;
        GlobalFree(lpFileArg); lpFileArg = NULL;
      }
      return(TRUE);
    }
    else // IsWindowEnabled()
    {
      // Ask...
      if (IDYES == MsgBox(MBYESNO,IDS_ERR_PREVWINDISABLED))
        return(FALSE);
      else
        return(TRUE);
    }
  }
  else
    return(FALSE);
}


//=============================================================================
//
//  RelaunchMultiInst()
//
//
BOOL RelaunchMultiInst() {

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

      CreateProcess(NULL,lpCmdLineNew,NULL,NULL,FALSE,0,NULL,g_wchWorkingDirectory,&si,&pi);
    }
    LocalFree(lpCmdLineNew);
    LocalFree(lp1);
    LocalFree(lp2);
    GlobalFree(lpFileArg); lpFileArg = NULL;

    return TRUE;
  }

  else {
    int i;
    for (i = 0; i < cFileList; i++)
      LocalFree(lpFileList[i]);
    return FALSE;
  }
}


//=============================================================================
//
//  RelaunchElevated()
//
//
BOOL RelaunchElevated(LPWSTR lpArgs) {

  BOOL result = FALSE;

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
  WINDOWPLACEMENT wndpl;
  HMONITOR hMonitor;
  MONITORINFO mi;
  int x,y,cx,cy;
  RECT rcOld;

  GetWindowRect(hwnd,&rcOld);

  hMonitor = MonitorFromRect(&rcOld,MONITOR_DEFAULTTONEAREST);
  mi.cbSize = sizeof(mi);
  GetMonitorInfo(hMonitor,&mi);

  y = mi.rcWork.top + 16;
  cy = mi.rcWork.bottom - mi.rcWork.top - 32;
  cx = min(mi.rcWork.right - mi.rcWork.left - 32,cy);
  x = mi.rcWork.right - cx - 16;

  wndpl.length = sizeof(WINDOWPLACEMENT);
  wndpl.flags = WPF_ASYNCWINDOWPLACEMENT;
  wndpl.showCmd = SW_RESTORE;

  wndpl.rcNormalPosition.left = x;
  wndpl.rcNormalPosition.top = y;
  wndpl.rcNormalPosition.right = x + cx;
  wndpl.rcNormalPosition.bottom = y + cy;

  if (EqualRect(&rcOld,&wndpl.rcNormalPosition)) {
    x = mi.rcWork.left + 16;
    wndpl.rcNormalPosition.left = x;
    wndpl.rcNormalPosition.right = x + cx;
  }

  if (GetDoAnimateMinimize()) {
    DrawAnimatedRects(hwnd,IDANI_CAPTION,&rcOld,&wndpl.rcNormalPosition);
    OffsetRect(&wndpl.rcNormalPosition,mi.rcMonitor.left - mi.rcWork.left,mi.rcMonitor.top - mi.rcWork.top);
  }

  SetWindowPlacement(hwnd,&wndpl);
}


//=============================================================================
//
//  ShowNotifyIcon()
//
//
void ShowNotifyIcon(HWND hwnd,BOOL bAdd)
{

  static HICON hIcon;
  NOTIFYICONDATA nid;

  if (!hIcon)
    hIcon = LoadImage(g_hInstance,MAKEINTRESOURCE(IDR_MAINWND),
                      IMAGE_ICON,16,16,LR_DEFAULTCOLOR);

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
    SHGetFileInfo2(g_wchCurFile,0,&shfi,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME);
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
  if (!iFileWatchingMode || !lpszFile || StringCchLen(lpszFile,MAX_PATH) == 0)
  {
    if (bRunningWatch)
    {
      if (hChangeHandle) {
        FindCloseChangeNotification(hChangeHandle);
        hChangeHandle = NULL;
      }
      KillTimer(NULL,ID_WATCHTIMER);
      bRunningWatch = FALSE;
      dwChangeNotifyTime = 0;
    }
  }
  else  // Install
  {
    // Terminate previous watching
    if (bRunningWatch) {
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

    hChangeHandle = FindFirstChangeNotification(tchDirectory,FALSE,
      FILE_NOTIFY_CHANGE_FILE_NAME  | \
      FILE_NOTIFY_CHANGE_DIR_NAME   | \
      FILE_NOTIFY_CHANGE_ATTRIBUTES | \
      FILE_NOTIFY_CHANGE_SIZE | \
      FILE_NOTIFY_CHANGE_LAST_WRITE);

    bRunningWatch = TRUE;
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
  if (bRunningWatch)
  {
    if (dwChangeNotifyTime > 0 && GetTickCount() - dwChangeNotifyTime > dwAutoReloadTimeout)
    {
      if (hChangeHandle) {
        FindCloseChangeNotification(hChangeHandle);
        hChangeHandle = NULL;
      }
      KillTimer(NULL,ID_WATCHTIMER);
      bRunningWatch = FALSE;
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
        if (iFileWatchingMode == 2) {
          bRunningWatch = TRUE; /* ! */
          dwChangeNotifyTime = GetTickCount();
        }
        else {
          KillTimer(NULL,ID_WATCHTIMER);
          bRunningWatch = FALSE;
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

      BOOL bAutoIndent2 = bAutoIndent;
      bAutoIndent = 0;
      EditJumpTo(g_hwndEdit,-1,0);
      SendMessage(g_hwndEdit,SCI_BEGINUNDOACTION,0,0);
      if (SendMessage(g_hwndEdit, SCI_GETLENGTH, 0, 0) > 0) {
        SendMessage(g_hwndEdit, SCI_NEWLINE, 0, 0);
      }
      SendMessage(g_hwndEdit,SCI_PASTE,0,0);
      SendMessage(g_hwndEdit,SCI_NEWLINE,0,0);
      SendMessage(g_hwndEdit,SCI_ENDUNDOACTION,0,0);
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
