/******************************************************************************
*                                                                             *
*                                                                             *
* MiniPath - Notepad3 Explorer Plugin                                         *
*                                                                             *
* Helpers.h                                                                   *
*   Definitions for general helper functions and macros                       *
*   Based on code from metapath, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2016   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

extern HINSTANCE g_hInstance;
extern UINT16 g_uWinVer;

#define UNUSED(expr) (void)(expr)
#define COUNTOF(ar) ARRAYSIZE(ar)   //#define COUNTOF(ar) (sizeof(ar)/sizeof(ar[0]))

extern WCHAR g_wchIniFile[MAX_PATH];

#define IniGetString(lpSection,lpName,lpDefault,lpReturnedStr,nSize) \
  GetPrivateProfileString(lpSection,lpName,lpDefault,lpReturnedStr,nSize,g_wchIniFile)
#define IniGetInt(lpSection,lpName,nDefault) \
  GetPrivateProfileInt(lpSection,lpName,nDefault,g_wchIniFile)
#define IniSetString(lpSection,lpName,lpString) \
  WritePrivateProfileString(lpSection,lpName,lpString,g_wchIniFile)
#define IniDeleteSection(lpSection) \
  WritePrivateProfileSection(lpSection,NULL,g_wchIniFile)
__inline BOOL IniSetInt(LPCWSTR lpSection,LPCWSTR lpName,int i) {
  WCHAR tch[32]; wsprintf(tch,L"%i",i); 
  return WritePrivateProfileString(lpSection,lpName,tch,g_wchIniFile);
}
#define LoadIniSection(lpSection,lpBuf,cchBuf) \
  GetPrivateProfileSection(lpSection,lpBuf,cchBuf,g_wchIniFile);
#define SaveIniSection(lpSection,lpBuf) \
  WritePrivateProfileSection(lpSection,lpBuf,g_wchIniFile)
int IniSectionGetString(LPCWSTR,LPCWSTR,LPCWSTR,LPWSTR,int);
int IniSectionGetInt(LPCWSTR,LPCWSTR,int);
BOOL IniSectionSetString(LPWSTR,LPCWSTR,LPCWSTR);
__inline BOOL IniSectionSetInt(LPWSTR lpCachedIniSection,LPCWSTR lpName,int i) {
  WCHAR tch[32]; wsprintf(tch,L"%i",i); return IniSectionSetString(lpCachedIniSection,lpName,tch);
}

void BeginWaitCursor();
void EndWaitCursor();

#define Is2k()    (g_uWinVer >= 0x0500)
#define IsXP()    (g_uWinVer >= 0x0501)
#define IsVista() (g_uWinVer >= 0x0600)
#define IsW7()    (g_uWinVer >= 0x0601)

BOOL ExeNameFromWnd(HWND,LPWSTR,int);
//BOOL Is32bitExe(LPCWSTR);
BOOL PrivateIsAppThemed();
//BOOL SetExplorerTheme(HWND);
BOOL SetTheme(HWND,LPCWSTR);
BOOL BitmapMergeAlpha(HBITMAP,COLORREF);
BOOL BitmapAlphaBlend(HBITMAP,COLORREF,BYTE);
BOOL BitmapGrayScale(HBITMAP);

BOOL SetWindowPathTitle(HWND,LPCWSTR);
void CenterDlgInParent(HWND);
void MakeBitmapButton(HWND,int,HINSTANCE,UINT);
void DeleteBitmapButton(HWND,int);
void SetWindowTransparentMode(HWND,BOOL);

#define StatusSetSimple(hwnd,b) SendMessage(hwnd,SB_SIMPLE,(WPARAM)b,0)
BOOL StatusSetText(HWND,UINT,LPCWSTR);

int Toolbar_GetButtons(HWND,int,LPWSTR,int);
int Toolbar_SetButtons(HWND,int,LPCWSTR,void*,int);
void Toolbar_SetButtonImage(HWND,int,int);

LRESULT SendWMSize(HWND);

#define EnableCmd(hmenu,id,b) EnableMenuItem(hmenu,id,(b)\
                               ?MF_BYCOMMAND|MF_ENABLED:MF_BYCOMMAND|MF_GRAYED)

#define CheckCmd(hmenu,id,b)  CheckMenuItem(hmenu,id,(b)\
                               ?MF_BYCOMMAND|MF_CHECKED:MF_BYCOMMAND|MF_UNCHECKED)

#define GetString(id,pb,cb) LoadString(g_hInstance,id,pb,cb)

#define StrEnd(pStart) (pStart + lstrlen(pStart))

int FormatString(LPWSTR,int,UINT,...);

void PathRelativeToApp(LPWSTR,LPWSTR,int,BOOL,BOOL,BOOL);
void PathAbsoluteFromApp(LPWSTR,LPWSTR,int,BOOL);

BOOL PathIsLnkFile(LPCWSTR);
BOOL PathGetLnkPath(LPCWSTR,LPWSTR,int);
BOOL PathIsLnkToDirectory(LPCWSTR,LPWSTR,int);
BOOL PathCreateLnk(LPCWSTR,LPCWSTR);

BOOL TrimString(LPWSTR);
BOOL ExtractFirstArgument(LPCWSTR, LPWSTR, LPWSTR);

LPWSTR QuotateFilenameStr(LPWSTR);
LPWSTR GetFilenameStr(LPWSTR);

void PrepareFilterStr(LPWSTR);
void StrTab2Space(LPWSTR);
void ExpandEnvironmentStringsEx(LPWSTR,DWORD);
void PathCanonicalizeEx(LPWSTR);
DWORD SearchPathEx(LPCWSTR,LPCWSTR,LPCWSTR,DWORD,LPWSTR,LPWSTR*);
int  FormatNumberStr(LPWSTR);

void GetDefaultFavoritesDir(LPWSTR,int);
void GetDefaultOpenWithDir(LPWSTR,int);

HDROP CreateDropHandle(LPCWSTR);

BOOL DirList_IsFileSelected(HWND);

BOOL ExecDDECommand(LPCWSTR,LPCWSTR,LPCWSTR,LPCWSTR);


//==== StrNextTok methods ===================
CHAR*  _StrNextTokA(CHAR*, const CHAR*);
WCHAR* _StrNextTokW(WCHAR*, const WCHAR*);
#if defined(UNICODE) || defined(_UNICODE)  
#define StrNextTok _StrNextTokW
#else
#define StrNextTok _StrNextTokA
#endif

//==== History Functions ======================================================
#define HISTORY_ITEMS 50

typedef struct tagHISTORY
{
  WCHAR *psz[HISTORY_ITEMS]; // Strings
  int  iCurItem;            // Current Item

} HISTORY, *PHISTORY;

BOOL History_Init(PHISTORY);
BOOL History_Uninit(PHISTORY);
BOOL History_Add(PHISTORY,LPCWSTR);
BOOL History_Forward(PHISTORY,LPWSTR,int);
BOOL History_Back(PHISTORY,LPWSTR,int);
BOOL History_CanForward(PHISTORY);
BOOL History_CanBack(PHISTORY);
void History_UpdateToolbar(PHISTORY,HWND,int,int);

//==== MRU Functions ==========================================================
#define MRU_MAXITEMS 24
#define MRU_NOCASE    1
#define MRU_UTF8      2

typedef struct _mrulist {

  WCHAR  szRegKey[256];
  int   iFlags;
  int   iSize;
  LPWSTR pszItems[MRU_MAXITEMS];

} MRULIST, *PMRULIST, *LPMRULIST;

LPMRULIST MRU_Create(LPCWSTR,int,int);
BOOL      MRU_Destroy(LPMRULIST);
BOOL      MRU_Add(LPMRULIST,LPCWSTR);
BOOL      MRU_Delete(LPMRULIST,int);
BOOL      MRU_Empty(LPMRULIST);
int       MRU_Enum(LPMRULIST,int,LPWSTR,int);
BOOL      MRU_Load(LPMRULIST);
BOOL      MRU_Save(LPMRULIST);
void      MRU_LoadToCombobox(HWND,LPCWSTR);
void      MRU_AddOneItem(LPCWSTR,LPCWSTR);

//==== Themed Dialogs =========================================================
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

BOOL GetThemedDialogFont(LPWSTR,WORD*);
DLGTEMPLATE* LoadThemedDialogTemplate(LPCTSTR,HINSTANCE);
#define ThemedDialogBox(hInstance,lpTemplate,hWndParent,lpDialogFunc) \
  ThemedDialogBoxParam(hInstance,lpTemplate,hWndParent,lpDialogFunc,0)
INT_PTR ThemedDialogBoxParam(HINSTANCE,LPCTSTR,HWND,DLGPROC,LPARAM);

//==== MinimizeToTray Functions - see comments in Helpers.c ===================
BOOL GetDoAnimateMinimize(VOID);
VOID MinimizeWndToTray(HWND hWnd);
VOID RestoreWndFromTray(HWND hWnd);



///   End of Helpers.h   \\\
