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
#pragma once
#ifndef _NP3_HELPERS_H_
#define _NP3_HELPERS_H_

#include <VersionHelpers.h>
#define STRSAFE_NO_CB_FUNCTIONS
#undef STRSAFE_NO_DEPRECATE      // don't allow deprecated functions
#include <strsafe.h>
#include <shlwapi.h>

#define STRGFY(X)     L##X
#define MKWSTRG(strg) STRGFY(strg)

#define UNUSED(expr) (void)(expr)
#define SIZEOF(ar) sizeof(ar)
#define COUNTOF(ar) ARRAYSIZE(ar)   //#define COUNTOF(ar) (sizeof(ar)/sizeof(ar[0]))
#define CSTRLEN(s)  (COUNTOF(s)-1)

extern WCHAR szIniFile[MAX_PATH];

#define IniGetString(lpSection,lpName,lpDefault,lpReturnedStr,nSize) \
  GetPrivateProfileString(lpSection,lpName,(lpDefault),(lpReturnedStr),(nSize),szIniFile)
#define IniGetInt(lpSection,lpName,nDefault) \
  GetPrivateProfileInt(lpSection,lpName,(nDefault),szIniFile)
#define IniGetBool(lpSection,lpName,nDefault) \
  (GetPrivateProfileInt(lpSection,lpName,(int)(nDefault),szIniFile) ? TRUE : FALSE)
#define IniSetString(lpSection,lpName,lpString) \
  WritePrivateProfileString(lpSection,lpName,(lpString),szIniFile)
#define IniDeleteSection(lpSection) \
  WritePrivateProfileSection(lpSection,NULL,szIniFile)
__inline BOOL IniSetInt(LPCWSTR lpSection, LPCWSTR lpName, int i)
{
  WCHAR tch[32] = { L'\0' }; StringCchPrintf(tch, COUNTOF(tch), L"%i", i); return IniSetString(lpSection, lpName, tch);
}
#define IniSetBool(lpSection,lpName,nValue) \
  IniSetInt(lpSection,lpName,((nValue) ? 1 : 0))
#define LoadIniSection(lpSection,lpBuf,cchBuf) \
  GetPrivateProfileSection(lpSection,lpBuf,(cchBuf),szIniFile)
#define SaveIniSection(lpSection,lpBuf) \
  WritePrivateProfileSection(lpSection,lpBuf,szIniFile)
int IniSectionGetString(LPCWSTR, LPCWSTR, LPCWSTR, LPWSTR, int);
int IniSectionGetInt(LPCWSTR, LPCWSTR, int);
UINT IniSectionGetUInt(LPCWSTR, LPCWSTR, UINT);
__inline BOOL IniSectionGetBool(LPCWSTR lpCachedIniSection, LPCWSTR lpName, BOOL bDefault) {
  return (IniSectionGetInt(lpCachedIniSection, lpName, ((bDefault) ? 1 : 0)) ? TRUE : FALSE);
}
BOOL IniSectionSetString(LPWSTR,LPCWSTR,LPCWSTR);
__inline BOOL IniSectionSetInt(LPWSTR lpCachedIniSection,LPCWSTR lpName,int i) {
  WCHAR tch[32]={L'\0'}; StringCchPrintf(tch,COUNTOF(tch),L"%i",i); return IniSectionSetString(lpCachedIniSection,lpName,tch);
}
__inline BOOL IniSectionSetBool(LPWSTR lpCachedIniSection, LPCWSTR lpName, BOOL b)
{
  return IniSectionSetInt(lpCachedIniSection, lpName, (b ? 1 : 0));
}


//extern HWND hwndEdit;
#define BeginWaitCursor() SendMessage(hwndEdit,SCI_SETCURSOR,(WPARAM)SC_CURSORWAIT,0)
#define EndWaitCursor() { POINT pt; SendMessage(hwndEdit,SCI_SETCURSOR,(WPARAM)SC_CURSORNORMAL,0); GetCursorPos(&pt); SetCursorPos(pt.x,pt.y); }


//#define Is2k()    (g_uWinVer >= 0x0500)
#define IsXP()     IsWindowsXPOrGreater()        // Indicates if the current OS version matches,or is greater than,the Windows XP version.
#define IsXP1()    IsWindowsXPSP1OrGreater()     // Indicates if the current OS version matches,or is greater than,the Windows XP with Service Pack 1 (SP1)version.
#define IsXP2()    IsWindowsXPSP2OrGreater()     // Indicates if the current OS version matches,or is greater than,the Windows XP with Service Pack 2 (SP2)version.
#define IsXP3()    IsWindowsXPSP3OrGreater()     // Indicates if the current OS version matches,or is greater than,the Windows XP with Service Pack 3 (SP3)version.

#define IsVista()  IsWindowsVistaOrGreater()     // Indicates if the current OS version matches,or is greater than,the Windows Vista version.
#define IsVista1() IsWindowsVistaSP1OrGreater()  // Indicates if the current OS version matches,or is greater than,the Windows Vista with Service Pack 1 (SP1)version.
#define IsVista2() IsWindowsVistaSP2OrGreater()  // Indicates if the current OS version matches,or is greater than,the Windows Vista with Service Pack 2 (SP2)version.

#define IsWin7()   IsWindows7OrGreater()         // Indicates if the current OS version matches,or is greater than,the Windows 7 version.
#define IsWin71()  IsWindows7SP1OrGreater()      // Indicates if the current OS version matches,or is greater than,the Windows 7 with Service Pack 1 (SP1)version.
#define IsWin8()   IsWindows8OrGreater()         // Indicates if the current OS version matches,or is greater than,the Windows 8 version.
#define IsWin81()  IsWindows8Point1OrGreater()   // Indicates if the current OS version matches,or is greater than,the Windows 8.1 version.
                                                 //   For Windows 10,IsWindows8Point1OrGreater returns false unless the application contains a manifest that includes
                                                 //   a compatibility section that contains the GUIDs that designate Windows 8.1 and/or Windows 10.

#define IsWin10()  IsWindows10OrGreater()        // Indicates if the current OS version matches, or is greater than, the Windows 10 version.
                                                 //   For Windows 10,IsWindows10OrGreater returns false unless the application contains a manifest that includes
                                                 //   a compatibility section that contains the GUID that designates Windows 10.

#define IsWinServer() IsWindowsServer()          // Indicates if the current OS is a Windows Server release.
                                                 //   Applications that need to distinguish between server and client versions of Windows should call this function.

#define SCVS_NP3_SPACE_OPT (SCVS_RECTANGULARSELECTION | SCVS_NOWRAPLINESTART)


enum BufferSizes {
  MICRO_BUFFER =   32,
  MINI_BUFFER  =   64,
  SMALL_BUFFER =  128,
  MIDSZ_BUFFER =  256,
  LARGE_BUFFER =  512,
  HUGE_BUFFER  = 1024,
  FILE_ARG_BUF = MAX_PATH+4
};

BOOL PrivateIsAppThemed();
HRESULT PrivateSetCurrentProcessExplicitAppUserModelID(PCWSTR);
BOOL IsElevated();
BOOL IsUserAdmin();
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
//void SnapToDefaultButton(HWND);
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

BOOL IsCmdEnabled(HWND, UINT);

#define EnableCmd(hmenu,id,b) EnableMenuItem(hmenu,id,(b)\
                               ?MF_BYCOMMAND|MF_ENABLED:MF_BYCOMMAND|MF_GRAYED)

#define CheckCmd(hmenu,id,b)  CheckMenuItem(hmenu,id,(b)\
                               ?MF_BYCOMMAND|MF_CHECKED:MF_BYCOMMAND|MF_UNCHECKED)

#define GetString(id,pb,cb) LoadString(g_hInstance,id,pb,cb)

#define StrEnd(pStart) (pStart + lstrlen(pStart))

int FormatString(LPWSTR,int,UINT,...);

BOOL GetKnownFolderPath(REFKNOWNFOLDERID, LPWSTR, size_t);
void PathRelativeToApp(LPWSTR,LPWSTR,int,BOOL,BOOL,BOOL);
void PathAbsoluteFromApp(LPWSTR,LPWSTR,int,BOOL);


BOOL PathIsLnkFile(LPCWSTR);
BOOL PathGetLnkPath(LPCWSTR,LPWSTR,int);
BOOL PathIsLnkToDirectory(LPCWSTR,LPWSTR,int);
BOOL PathCreateDeskLnk(LPCWSTR);
BOOL PathCreateFavLnk(LPCWSTR,LPCWSTR,LPCWSTR);


BOOL StrLTrim(LPWSTR,LPCWSTR);
BOOL TrimString(LPWSTR);
BOOL ExtractFirstArgument(LPCWSTR, LPWSTR, LPWSTR, int);

void PrepareFilterStr(LPWSTR);

void StrTab2Space(LPWSTR);
void PathFixBackslashes(LPWSTR);


void  ExpandEnvironmentStringsEx(LPWSTR,DWORD);
void  PathCanonicalizeEx(LPWSTR,int);
DWORD GetLongPathNameEx(LPWSTR,DWORD);
DWORD NormalizePathEx(LPWSTR,int);
DWORD_PTR SHGetFileInfo2(LPCWSTR,DWORD,SHFILEINFO*,UINT,UINT);


int  FormatNumberStr(LPWSTR);
BOOL SetDlgItemIntEx(HWND,int,UINT);


#define MultiByteToWideCharStrg(c,a,w) MultiByteToWideChar((c),0,(a),-1,(w),COUNTOF(w))
#define WideCharToMultiByteStrg(c,w,a) WideCharToMultiByte((c),0,(w),-1,(a),COUNTOF(a),NULL,NULL)


UINT    GetDlgItemTextA2W(UINT,HWND,int,LPSTR,int);
UINT    SetDlgItemTextA2W(UINT,HWND,int,LPSTR);
LRESULT ComboBox_AddStringA2W(UINT,HWND,LPCSTR);


UINT CodePageFromCharSet(UINT);
UINT CharSetFromCodePage(UINT);


//==== MRU Functions ==========================================================
#define MRU_MAXITEMS 24
#define MRU_NOCASE    1
#define MRU_UTF8      2

typedef struct _mrulist {

  WCHAR  szRegKey[256];
  int    iFlags;
  int    iSize;
  LPWSTR pszItems[MRU_MAXITEMS];
  int    iEncoding[MRU_MAXITEMS];
  int    iCaretPos[MRU_MAXITEMS];

} MRULIST, *PMRULIST, *LPMRULIST;

LPMRULIST MRU_Create(LPCWSTR,int,int);
BOOL      MRU_Destroy(LPMRULIST);
BOOL      MRU_Add(LPMRULIST,LPCWSTR,int,int);
BOOL      MRU_FindFile(LPMRULIST,LPCWSTR,int*);
BOOL      MRU_AddFile(LPMRULIST,LPCWSTR,BOOL,BOOL,int,int);
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

//==== strCut methods ===================

CHAR*  _StrCutIA(CHAR*,const CHAR*);
WCHAR* _StrCutIW(WCHAR*,const WCHAR*);
#if defined(UNICODE) || defined(_UNICODE)  
#define StrCutI _StrCutIW
#else
#define StrCutI _StrCutIA
#endif

//==== StrSafe lstrlen() =======================================================
inline int _StringCchLenNA(LPCSTR s,size_t n) { size_t len; HRESULT hr = StringCchLengthA(s,n,&len); return (SUCCEEDED(hr) ? (int)len : 0); }
#define StringCchLenA(s)  _StringCchLenNA((s),COUNTOF(s))
inline int _StringCchLenNW(LPCWSTR s,size_t n) { size_t len; HRESULT hr = StringCchLengthW(s,n,&len); return (SUCCEEDED(hr) ? (int)len : 0); }
#define StringCchLenW(s)  _StringCchLenNW((s),COUNTOF(s))

#if defined(UNICODE) || defined(_UNICODE)  
#define StringCchLen(s)     _StringCchLenNW((s),COUNTOF(s))
#define StringCchLenN(s,n)  _StringCchLenNW((s),(n))
#else
#define StringCchLen(s)     _StringCchLenNA((s),COUNTOF(s))
#define StringCchLenN(s,n)  _StringCchLenNA((s),(n))
#endif

//==== StrSafe lstrcmp(),lstrcmpi() =============================================
inline int _StringCchCmpNA(PCNZCH s1,int l1,PCNZCH s2,int l2)
{
  return (CompareStringA(LOCALE_INVARIANT,0,s1,(l1 >= 0 ? _StringCchLenNA(s1,l1) : -1),
                         s2,(l2 >= 0 ? _StringCchLenNA(s2,l2) : -1)) - CSTR_EQUAL);
}
#define StringCchCompareA(s1,s2)         _StringCchCmpNA((s1),COUNTOF(s1),(s2),COUNTOF(s2))
#define StringCchCompareNA(s1,l1,s2,l2)  _StringCchCmpNA((s1),(l1),(s2),(l2))
#define StringCchCompareXA(s1,s2)        _StringCchCmpNA((s1),-1,(s2),-1)

inline int _StringCchCmpINA(PCNZCH s1,int l1,PCNZCH s2,int l2)
{
  return (CompareStringA(LOCALE_INVARIANT,NORM_IGNORECASE,s1,(l1 >= 0 ? _StringCchLenNA(s1,l1) : -1),
                         s2,(l2 >= 0 ? _StringCchLenNA(s2,l2) : -1)) - CSTR_EQUAL);
}
#define StringCchCompareIA(s1,s2)         _StringCchCmpINA((s1),COUNTOF(s1),(s2),COUNTOF(s2))
#define StringCchCompareINA(s1,l1,s2,l2)  _StringCchCmpINA((s1),(l1),(s2),(l2))
#define StringCchCompareIXA(s1,s2)        _StringCchCmpINA((s1),-1,(s2),-1)

inline int _StringCchCmpNW(PCNZWCH s1,int l1,PCNZWCH s2,int l2) {
  return (CompareStringW(LOCALE_INVARIANT,0,s1,(l1 >= 0 ? _StringCchLenNW(s1,l1) : -1),
                         s2,(l2 >= 0 ? _StringCchLenNW(s2,l2) : -1)) - CSTR_EQUAL);
}
#define StringCchCompareW(s1,s2)         _StringCchCmpNW((s1),COUNTOF(s1),(s2),COUNTOF(s2))
#define StringCchCompareNW(s1,l1,s2,l2)  _StringCchCmpNW((s1),(l1),(s2),(l2))
#define StringCchCompareXW(s1,s2)        _StringCchCmpNW((s1),-1,(s2),-1)

inline int _StringCchCmpINW(PCNZWCH s1,int l1,PCNZWCH s2,int l2) { 
  return (CompareStringW(LOCALE_INVARIANT,NORM_IGNORECASE,s1,(l1 >= 0 ? _StringCchLenNW(s1,l1) : -1),
                         s2,(l2 >= 0 ? _StringCchLenNW(s2,l2) : -1)) - CSTR_EQUAL);
}
#define StringCchCompareIW(s1,s2)         _StringCchCmpINW((s1),COUNTOF(s1),(s2),COUNTOF(s2))
#define StringCchCompareINW(s1,l1,s2,l2)  _StringCchCmpINW((s1),(l1),(s2),(l2))
#define StringCchCompareIXW(s1,s2)        _StringCchCmpINW((s1),-1,(s2),-1)

#if defined(UNICODE) || defined(_UNICODE)  
#define StringCchCompare(s1,s2)          StringCchCompareW((s1),(s2))
#define StringCchCompareN(s1,l1,s2,l2)   StringCchCompareNW((s1),(l1),(s2),(l2))
#define StringCchCompareX(s1,s2)         StringCchCompareXW((s1),(s2))
#define StringCchCompareI(s1,s2)         StringCchCompareIW((s1),(s2))
#define StringCchCompareIN(s1,l1,s2,l2)  StringCchCompareINW((s1),(l1),(s2),(l2))
#define StringCchCompareIX(s1,s2)        StringCchCompareIXW((s1),(s2))
#else
#define StringCchCompare(s1,s2)          StringCchCompareA((s1),(s2))
#define StringCchCompareN(s1,l1,s2,l2)   StringCchCompareNA((s1),(l1),(s2),(l2))
#define StringCchCompareX(s1,s2)         StringCchCompareXA((s1),(s2))
#define StringCchCompareI(s1,s2)         StringCchCompareIA((s1),(s2))
#define StringCchCompareIN(s1,l1,s2,l2)  StringCchCompareINA((s1),(l1),(s2),(l2))
#define StringCchCompareIX(s1,s2)        StringCchCompareIXA((s1),(s2))
#endif

// =====  File Encoding  =====

extern int g_DOSEncoding;

#define NCP_DEFAULT            1
#define NCP_UTF8               2
#define NCP_UTF8_SIGN          4
#define NCP_UNICODE            8
#define NCP_UNICODE_REVERSE   16
#define NCP_UNICODE_BOM       32
#define NCP_8BIT              64
#define NCP_ANSI             128
#define NCP_OEM              256
#define NCP_MBCS             512
#define NCP_INTERNAL          (NCP_DEFAULT|NCP_UTF8|NCP_UTF8_SIGN|NCP_UNICODE|NCP_UNICODE_REVERSE|NCP_UNICODE_BOM|NCP_ANSI|NCP_OEM|NCP_MBCS)
#define NCP_RECODE           1024

#define CPI_GET               -2
#define CPI_NONE              -1
#define CPI_ANSI_DEFAULT       0
#define CPI_OEM                1
#define CPI_UNICODEBOM         2
#define CPI_UNICODEBEBOM       3
#define CPI_UNICODE            4
#define CPI_UNICODEBE          5
#define CPI_UTF8               6
#define CPI_UTF8SIGN           7
#define CPI_UTF7               8

#define IDS_ENCODINGNAME0  61000
#define IDS_EOLMODENAME0   62000

typedef struct _np2encoding {
  UINT    uFlags;
  UINT    uCodePage;
  char*   pszParseNames;
  int     idsName;
  WCHAR   wchLabel[64];
} NP2ENCODING;

int  Encoding_CountOf();
int  Encoding_Current(int);    // getter/setter
int  Encoding_Source(int);     // getter/setter
int  Encoding_SrcWeak(int);    // getter/setter
BOOL Encoding_HasChanged(int); // query/setter

void Encoding_InitDefaults();
int  Encoding_MapIniSetting(BOOL,int);
int  Encoding_MapUnicode(int);
void Encoding_GetLabel(int);
int  Encoding_MatchW(LPCWSTR);
int  Encoding_MatchA(char*);
BOOL Encoding_IsValid(int);
int  Encoding_GetByCodePage(UINT);
void Encoding_AddToListView(HWND,int,BOOL);
BOOL Encoding_GetFromListView(HWND,int *);
void Encoding_AddToComboboxEx(HWND,int,BOOL);
BOOL Encoding_GetFromComboboxEx(HWND,int *);
BOOL Encoding_IsDefault(int);
BOOL Encoding_IsANSI(int);
BOOL Encoding_IsOEM(int);

UINT Encoding_SciGetCodePage(HWND);
int  Encoding_SciMappedCodePage(int);
void Encoding_SciSetCodePage(HWND,int);


BOOL IsUnicode(const char*,int,LPBOOL,LPBOOL);
BOOL IsUTF8(const char*,int);
BOOL IsUTF7(const char*,int);

#define IsUTF8Signature(p) \
          ((*(p+0) == '\xEF' && *(p+1) == '\xBB' && *(p+2) == '\xBF'))


#define UTF8StringStart(p) \
          (IsUTF8Signature(p)) ? (p+3) : (p)

INT UTF8_mbslen_bytes(LPCSTR utf8_string);
INT UTF8_mbslen(LPCSTR source,INT byte_length);

// --------------------------------------------------------------------------------------------------------------------------------

// including <pathcch.h> and linking against pathcch.lib
// api-ms-win-core-path-l1-1-0.dll  library : Minimum supported client is Windows 8 :-/
// so switch back to previous (deprecated) methods:
inline HRESULT PathCchAppend(PWSTR p,size_t l,PCWSTR a)          { UNUSED(l); return (PathAppend(p,a) ? S_OK : E_FAIL); }
inline HRESULT PathCchCanonicalize(PWSTR p,size_t l,PCWSTR a)    { UNUSED(l); return (PathCanonicalize(p,a) ? S_OK : E_FAIL); }
inline HRESULT PathCchRenameExtension(PWSTR p,size_t l,PCWSTR a) { UNUSED(l); return (PathRenameExtension(p,a) ? S_OK : E_FAIL); }
inline HRESULT PathCchRemoveFileSpec(PWSTR p,size_t l)           { UNUSED(l); return (PathRemoveFileSpec(p) ? S_OK : E_FAIL); }


#endif //_NP3_HELPERS_H_

///   End of Helpers.h   \\\
