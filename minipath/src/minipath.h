// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* MiniPath - Notepad3 Explorer Plugin                                         *
*                                                                             *
* minipath.h                                                                  *
*   Global definitions and declarations                                       *
*   Based on code from metapath, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2020   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

//==== Main Window ============================================================
#define WC_MINIPATH L"MiniPath"

#define WS_MINIPATH (WS_OVERLAPPEDWINDOW ^ \
                    (WS_MINIMIZEBOX | WS_MAXIMIZEBOX)) | \
                    (WS_CLIPCHILDREN | WS_POPUP)


//==== Data Type for WM_COPYDATA ==============================================
#define DATA_MINIPATH_PATHARG 0xFB30


//==== ComboBox Control =======================================================
//#define WC_COMBOBOX L"ComboBox"

#define WS_DRIVEBOX (WS_CHILD | \
                     /*WS_VISIBLE |*/ \
                     WS_CLIPSIBLINGS | \
                     WS_VSCROLL | \
                     CBS_DROPDOWNLIST)


//==== Listview Control =======================================================
#define WS_DIRLIST (WS_CHILD | \
                    WS_VISIBLE | \
                    WS_CLIPSIBLINGS | \
                    WS_CLIPCHILDREN | \
                    LVS_REPORT | \
                    LVS_NOCOLUMNHEADER | \
                    LVS_SHAREIMAGELISTS | \
                    LVS_AUTOARRANGE | \
                    LVS_SINGLESEL | \
                    LVS_SHOWSELALWAYS)


//==== Toolbar Style ==========================================================
#define WS_TOOLBAR (WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | \
                    TBSTYLE_TOOLTIPS | TBSTYLE_ALTDRAG | TBSTYLE_LIST | \
                    CCS_NODIVIDER | CCS_NOPARENTALIGN)


//==== ReBar Style ============================================================
#define WS_REBAR (WS_CHILD | /*WS_VISIBLE |*/ WS_CLIPCHILDREN | WS_BORDER | \
                  RBS_VARHEIGHT | RBS_BANDBORDERS /*| RBS_FIXEDORDER */ | \
                  CCS_NODIVIDER |/*CCS_NORESIZE | */CCS_NOPARENTALIGN)


//==== Ids ====================================================================
#define IDC_STATUSBAR 0x00A0
#define IDC_TOOLBAR   0x00A1
#define IDC_REBAR     0x00A2
#define IDC_DRIVEBOX  0xA000
#define IDC_DIRLIST   0xA001


//==== Statusbar ==============================================================
#define ID_FILEINFO   0
#define ID_MENUHELP 255


//==== Timer for Change Notifications =========================================
#define ID_TIMER 0xA000


//==== Callback Message from System Tray ======================================
#define WM_TRAYMESSAGE WM_USER


//==== TypeDefs ======================================
typedef struct _wi
{
  int x;
  int y;
  int cx;
  int cy;
} WININFO;


typedef enum { UTA_UNDEFINED = 0,  UTA_LAUNCH_TARGET = 1, UTA_DEFINE_TARGET = 4 } UseTargetApp;
typedef enum { TAM_ALWAYS_RUN = 0, TAM_SEND_DROP_MSG = 1, TAM_SEND_DDE_MSG = 2 } TargetAppMode;


// ----------------------------------------------------------------------------

#define DL_FILTER_BUFSIZE 256  // should correspond to Dlapi.h def

typedef struct _settings_t
{
  BOOL      bSaveSettings;
  WCHAR     szQuickview[MAX_PATH];
  WCHAR     szQuickviewParams[MAX_PATH];
  WCHAR     g_tchFavoritesDir[MAX_PATH];
  BOOL      bNP3sFavoritesSettings;
  WCHAR     tchOpenWithDir[MAX_PATH];
  WCHAR     tchToolbarButtons[512];
  WCHAR     tchToolbarBitmap[MAX_PATH];
  WCHAR     tchToolbarBitmapHot[MAX_PATH];
  WCHAR     tchToolbarBitmapDisabled[MAX_PATH];
  BOOL      bClearReadOnly;
  BOOL      bRenameOnCollision;
  BOOL      bSingleClick;
  BOOL      bTrackSelect;
  BOOL      bFullRowSelect;
  int       iStartupDir;
  int       iEscFunction;
  BOOL      bFocusEdit;
  BOOL      bAlwaysOnTop;
  BOOL      g_bTransparentMode;
  BOOL      bMinimizeToTray;
  BOOL      fUseRecycleBin;
  BOOL      fNoConfirmDelete;
  BOOL      bShowToolbar;
  BOOL      bShowStatusbar;
  BOOL      bShowDriveBox;
  int       cxGotoDlg;
  int       cxOpenWithDlg;
  int       cyOpenWithDlg;
  int       cxCopyMoveDlg;

  BOOL      bHasQuickview;

  WCHAR     tchFilter[DL_FILTER_BUFSIZE];
  BOOL      bNegFilter;
  BOOL      bDefCrNoFilt;
  BOOL      bDefCrFilter;
  COLORREF  crNoFilt;
  COLORREF  crFilter;
  COLORREF  crCustom[16];

  WININFO   wi;

  WCHAR     szCurDir[MAX_PATH + 40];
  DWORD     dwFillMask;
  int       nSortFlags;
  BOOL      fSortRev;

} SETTINGS_T, * PSETTINGS_T;

extern SETTINGS_T Settings;
extern SETTINGS_T Defaults;


//==== Function Declarations ==================================================
BOOL InitApplication(HINSTANCE);
HWND InitInstance(HINSTANCE,LPWSTR,int);
BOOL ActivatePrevInst();
void ShowNotifyIcon(HWND,BOOL);

BOOL ChangeDirectory(HWND,LPCWSTR,BOOL);
void ParseCommandLine();

BOOL DisplayPath(LPCWSTR,UINT);
BOOL DisplayLnkFile(LPCWSTR);

void LoadTargetParamsOnce(void);
void LaunchTarget(LPCWSTR,BOOL);
void SnapToTarget(HWND);
void SnapToDefaultPos(HWND);

LRESULT CALLBACK HiddenWndProc(HWND,UINT,WPARAM,LPARAM);
LRESULT CALLBACK MainWndProc(HWND,UINT,WPARAM,LPARAM);
LRESULT MsgCreate(HWND,WPARAM,LPARAM);
void    CreateBars(HWND,HINSTANCE);
void    MsgThemeChanged(HWND,WPARAM,LPARAM);
void    MsgSize(HWND,WPARAM,LPARAM);
void    MsgInitMenu(HWND,WPARAM,LPARAM);
LRESULT MsgCommand(HWND,WPARAM,LPARAM);
LRESULT MsgNotify(HWND,WPARAM,LPARAM);



///   End of minipath.h   \\\
