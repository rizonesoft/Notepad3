/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Helpers.h                                                                   *
*   Definitions for general helper functions and macros                       *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2016   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/


#define UNUSED(expr) (void)(expr)
#define COUNTOF(ar) ARRAYSIZE(ar)   //#define COUNTOF(ar) (sizeof(ar)/sizeof(ar[0]))
#define CSTRLEN(s)  (COUNTOF(s)-1)


extern WCHAR szIniFile[MAX_PATH];
#define IniGetString(lpSection,lpName,lpDefault,lpReturnedStr,nSize) \
  GetPrivateProfileString(lpSection,lpName,lpDefault,lpReturnedStr,nSize,szIniFile)
#define IniGetInt(lpSection,lpName,nDefault) \
  GetPrivateProfileInt(lpSection,lpName,nDefault,szIniFile)
#define IniSetString(lpSection,lpName,lpString) \
  WritePrivateProfileString(lpSection,lpName,lpString,szIniFile)
#define IniDeleteSection(lpSection) \
  WritePrivateProfileSection(lpSection,NULL,szIniFile)
__inline BOOL IniSetInt(LPCWSTR lpSection,LPCWSTR lpName,int i) {
  WCHAR tch[32]; wsprintf(tch,L"%i",i); return IniSetString(lpSection,lpName,tch);
}
#define LoadIniSection(lpSection,lpBuf,cchBuf) \
  GetPrivateProfileSection(lpSection,lpBuf,cchBuf,szIniFile)
#define SaveIniSection(lpSection,lpBuf) \
  WritePrivateProfileSection(lpSection,lpBuf,szIniFile)
int IniSectionGetString(LPCWSTR,LPCWSTR,LPCWSTR,LPWSTR,int);
int IniSectionGetInt(LPCWSTR,LPCWSTR,int);
UINT IniSectionGetUInt(LPCWSTR,LPCWSTR,UINT);
BOOL IniSectionSetString(LPWSTR,LPCWSTR,LPCWSTR);
__inline BOOL IniSectionSetInt(LPWSTR lpCachedIniSection,LPCWSTR lpName,int i) {
  WCHAR tch[32]; wsprintf(tch,L"%i",i); return IniSectionSetString(lpCachedIniSection,lpName,tch);
}


//extern HWND hwndEdit;
#define BeginWaitCursor() SendMessage(hwndEdit,SCI_SETCURSOR,(WPARAM)SC_CURSORWAIT,0)
#define EndWaitCursor() { POINT pt; SendMessage(hwndEdit,SCI_SETCURSOR,(WPARAM)SC_CURSORNORMAL,0); GetCursorPos(&pt); SetCursorPos(pt.x,pt.y); }


#define Is2k()    (g_uWinVer >= 0x0500)
#define IsXP()    (g_uWinVer >= 0x0501)
#define IsVista() (g_uWinVer >= 0x0600)
#define IsW7()    (g_uWinVer >= 0x0601)


BOOL PrivateIsAppThemed();
HRESULT PrivateSetCurrentProcessExplicitAppUserModelID(PCWSTR);
BOOL IsElevated();
//BOOL SetExplorerTheme(HWND);


BOOL BitmapMergeAlpha(HBITMAP,COLORREF);
BOOL BitmapAlphaBlend(HBITMAP,COLORREF,BYTE);
BOOL BitmapGrayScale(HBITMAP);
BOOL VerifyContrast(COLORREF,COLORREF);
BOOL IsFontAvailable(LPCWSTR);


BOOL SetWindowTitle(HWND,UINT,BOOL,UINT,LPCWSTR,int,BOOL,UINT,BOOL,LPCWSTR);
void SetWindowTransparentMode(HWND,BOOL);


void CenterDlgInParent(HWND);
void GetDlgPos(HWND,LPINT,LPINT);
void SetDlgPos(HWND,int,int);
void ResizeDlg_Init(HWND,int,int,int);
void ResizeDlg_Destroy(HWND,int*,int*);
void ResizeDlg_Size(HWND,LPARAM,int*,int*);
void ResizeDlg_GetMinMaxInfo(HWND,LPARAM);
HDWP DeferCtlPos(HDWP,HWND,int,int,int,UINT);
void MakeBitmapButton(HWND,int,HINSTANCE,UINT);
void MakeColorPickButton(HWND,int,HINSTANCE,COLORREF);
void DeleteBitmapButton(HWND,int);


#define StatusSetSimple(hwnd,b) SendMessage(hwnd,SB_SIMPLE,(WPARAM)b,0)
BOOL StatusSetText(HWND,UINT,LPCWSTR);
BOOL StatusSetTextID(HWND,UINT,UINT);
int  StatusCalcPaneWidth(HWND,LPCWSTR);

int Toolbar_GetButtons(HWND,int,LPWSTR,int);
int Toolbar_SetButtons(HWND,int,LPCWSTR,void*,int);

LRESULT SendWMSize(HWND);

#define EnableCmd(hmenu,id,b) EnableMenuItem(hmenu,id,(b)\
                               ?MF_BYCOMMAND|MF_ENABLED:MF_BYCOMMAND|MF_GRAYED)

#define CheckCmd(hmenu,id,b)  CheckMenuItem(hmenu,id,(b)\
                               ?MF_BYCOMMAND|MF_CHECKED:MF_BYCOMMAND|MF_UNCHECKED)

BOOL IsCmdEnabled(HWND, UINT);


#define GetString(id,pb,cb) LoadString(g_hInstance,id,pb,cb)

#define StrEnd(pStart) (pStart + lstrlen(pStart))

int FormatString(LPWSTR,int,UINT,...);


void PathRelativeToApp(LPWSTR,LPWSTR,int,BOOL,BOOL,BOOL);
void PathAbsoluteFromApp(LPWSTR,LPWSTR,int,BOOL);


BOOL PathIsLnkFile(LPCWSTR);
BOOL PathGetLnkPath(LPCWSTR,LPWSTR,int);
BOOL PathIsLnkToDirectory(LPCWSTR,LPWSTR,int);
BOOL PathCreateDeskLnk(LPCWSTR);
BOOL PathCreateFavLnk(LPCWSTR,LPCWSTR,LPCWSTR);


BOOL StrLTrim(LPWSTR,LPCWSTR);
BOOL TrimString(LPWSTR);
BOOL ExtractFirstArgument(LPCWSTR, LPWSTR, LPWSTR);

void PrepareFilterStr(LPWSTR);

void StrTab2Space(LPWSTR);
void PathFixBackslashes(LPWSTR);


void  ExpandEnvironmentStringsEx(LPWSTR,DWORD);
void  PathCanonicalizeEx(LPWSTR);
DWORD GetLongPathNameEx(LPWSTR,DWORD);
DWORD_PTR SHGetFileInfo2(LPCWSTR,DWORD,SHFILEINFO*,UINT,UINT);


int  FormatNumberStr(LPWSTR);
BOOL SetDlgItemIntEx(HWND,int,UINT);


#define MBCSToWChar(c,a,w,i) MultiByteToWideChar(c,0,a,-1,w,i)
#define WCharToMBCS(c,w,a,i) WideCharToMultiByte(c,0,w,-1,a,i,NULL,NULL)

UINT    GetDlgItemTextA2W(UINT,HWND,int,LPSTR,int);
UINT    SetDlgItemTextA2W(UINT,HWND,int,LPSTR);
LRESULT ComboBox_AddStringA2W(UINT,HWND,LPCSTR);


UINT CodePageFromCharSet(UINT);


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
BOOL      MRU_AddFile(LPMRULIST,LPCWSTR,BOOL,BOOL);
BOOL      MRU_Delete(LPMRULIST,int);
BOOL      MRU_DeleteFileFromStore(LPMRULIST,LPCWSTR);
BOOL      MRU_Empty(LPMRULIST);
int       MRU_Enum(LPMRULIST,int,LPWSTR,int);
BOOL      MRU_Load(LPMRULIST);
BOOL      MRU_Save(LPMRULIST);
BOOL      MRU_MergeSave(LPMRULIST,BOOL,BOOL,BOOL);


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
HWND    CreateThemedDialogParam(HINSTANCE,LPCTSTR,HWND,DLGPROC,LPARAM);


//==== UnSlash Functions ======================================================
void TransformBackslashes(char*,BOOL,UINT);


//==== MinimizeToTray Functions - see comments in Helpers.c ===================
BOOL GetDoAnimateMinimize(VOID);
VOID MinimizeWndToTray(HWND hWnd);
VOID RestoreWndFromTray(HWND hWnd);



///   End of Helpers.h   \\\
