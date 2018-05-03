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

#define STRSAFE_NO_CB_FUNCTIONS
#define STRSAFE_NO_DEPRECATE      // don't allow deprecated functions
#include <strsafe.h>
#include <shlwapi.h>
#include <VersionHelpers.h>

#include "typedefs.h"

// ============================================================================

extern WCHAR g_wchIniFile[MAX_PATH];


// ============================================================================

#define STRGFY(X)     L##X
#define MKWSTRG(strg) STRGFY(strg)

#define UNUSED(expr) (void)(expr)
#define SIZEOF(ar) sizeof(ar)
#define COUNTOF(ar) ARRAYSIZE(ar)   //#define COUNTOF(ar) (sizeof(ar)/sizeof(ar[0]))
#define CSTRLEN(s)  (COUNTOF(s)-1)

__forceinline void swapi(int* a, int* b) { int t = *a;  *a = *b;  *b = t; }
__forceinline void swapos(DocPos* a, DocPos* b) { DocPos t = *a;  *a = *b;  *b = t; }

__forceinline bool HasFractionCent(float f) { return ((((int)(f * 100.0)) % 100) != 0); }



// direct heap allocation
__forceinline LPVOID AllocMem(size_t numBytes, DWORD dwFlags)
{
  return HeapAlloc(GetProcessHeap(), (dwFlags | HEAP_GENERATE_EXCEPTIONS), numBytes);
}
__forceinline bool FreeMem(LPVOID lpMemory)
{
  return ((lpMemory != NULL) ? HeapFree(GetProcessHeap(), 0, lpMemory) : true);
}
__forceinline size_t SizeOfMem(LPVOID lpMemory)
{
  return ((lpMemory != NULL) ? HeapSize(GetProcessHeap(), 0, lpMemory) : 0);
}

#define IniGetString(lpSection,lpName,lpDefault,lpReturnedStr,nSize) \
  GetPrivateProfileString(lpSection,lpName,(lpDefault),(lpReturnedStr),(nSize),g_wchIniFile)
#define IniGetInt(lpSection,lpName,nDefault) \
  GetPrivateProfileInt(lpSection,lpName,(nDefault),g_wchIniFile)
#define IniGetBool(lpSection,lpName,nDefault) \
  (GetPrivateProfileInt(lpSection,lpName,(int)(nDefault),g_wchIniFile) ? true : false)
#define IniSetString(lpSection,lpName,lpString) \
  WritePrivateProfileString(lpSection,lpName,(lpString),g_wchIniFile)
#define IniDeleteSection(lpSection) \
  WritePrivateProfileSection(lpSection,NULL,g_wchIniFile)

__inline bool IniSetInt(LPCWSTR lpSection, LPCWSTR lpName, int i) {
  WCHAR tch[32] = { L'\0' }; StringCchPrintf(tch, COUNTOF(tch), L"%i", i); return IniSetString(lpSection, lpName, tch);
}

#define IniSetBool(lpSection,lpName,nValue) \
  IniSetInt(lpSection,lpName,((nValue) ? 1 : 0))
#define LoadIniSection(lpSection,lpBuf,cchBuf) \
  GetPrivateProfileSection(lpSection,lpBuf,(cchBuf),g_wchIniFile)
#define SaveIniSection(lpSection,lpBuf) \
  WritePrivateProfileSection(lpSection,lpBuf,g_wchIniFile)

int IniSectionGetString(LPCWSTR, LPCWSTR, LPCWSTR, LPWSTR, int);
int IniSectionGetInt(LPCWSTR, LPCWSTR, int);
UINT IniSectionGetUInt(LPCWSTR, LPCWSTR, UINT);
DocPos IniSectionGetPos(LPCWSTR, LPCWSTR, DocPos);
__forceinline bool IniSectionGetBool(LPCWSTR lpCachedIniSection, LPCWSTR lpName, bool bDefault) {
  return (IniSectionGetInt(lpCachedIniSection, lpName, ((bDefault) ? 1 : 0)) ? true : false);
}

bool IniSectionSetString(LPWSTR,LPCWSTR,LPCWSTR);

__forceinline bool IniSectionSetInt(LPWSTR lpCachedIniSection,LPCWSTR lpName, int i) {
  WCHAR tch[32]={L'\0'}; StringCchPrintf(tch,COUNTOF(tch),L"%i",i); return IniSectionSetString(lpCachedIniSection,lpName,tch);
}
__forceinline bool IniSectionSetBool(LPWSTR lpCachedIniSection, LPCWSTR lpName, bool b) {
  return IniSectionSetInt(lpCachedIniSection, lpName, (b ? 1 : 0));
}
__forceinline bool IniSectionSetPos(LPWSTR lpCachedIniSection, LPCWSTR lpName, DocPos pos)
{
  WCHAR tch[64] = { L'\0' }; StringCchPrintf(tch, COUNTOF(tch), L"%td", (long long)pos); return IniSectionSetString(lpCachedIniSection, lpName, tch);
}


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

bool SetClipboardTextW(HWND, LPCWSTR);

bool PrivateIsAppThemed();
HRESULT PrivateSetCurrentProcessExplicitAppUserModelID(PCWSTR);
bool IsElevated();
bool IsUserAdmin();
//bool SetExplorerTheme(HWND);


bool BitmapMergeAlpha(HBITMAP,COLORREF);
bool BitmapAlphaBlend(HBITMAP,COLORREF,BYTE);
bool BitmapGrayScale(HBITMAP);
bool VerifyContrast(COLORREF,COLORREF);
bool IsFontAvailable(LPCWSTR);
POINT GetSystemDpi();


bool SetWindowTitle(HWND,UINT,bool,UINT,LPCWSTR,int,bool,UINT,bool,LPCWSTR);
void SetWindowTransparentMode(HWND,bool);


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
void StatusSetText(HWND,UINT,LPCWSTR);
bool StatusSetTextID(HWND,UINT,UINT);
COLORREF GetBackgroundColor(HWND);
LONG StatusCalcPaneWidth(HWND,LPCWSTR);

int Toolbar_GetButtons(HWND,int,LPWSTR,int);
int Toolbar_SetButtons(HWND,int,LPCWSTR,void*,int);

LRESULT SendWMSize(HWND);

bool IsCmdEnabled(HWND, UINT);

#define EnableCmd(hmenu,id,b) EnableMenuItem((hmenu),(id),(b)?MF_BYCOMMAND|MF_ENABLED:MF_BYCOMMAND|MF_GRAYED)
#define CheckCmd(hmenu,id,b)  CheckMenuItem((hmenu),(id),(b)?MF_BYCOMMAND|MF_CHECKED:MF_BYCOMMAND|MF_UNCHECKED)

#define EnableCmdPos(hmenu,pos,b) EnableMenuItem((hmenu),(pos),(b)?MF_BYPOSITION|MF_ENABLED:MF_BYPOSITION|MF_GRAYED)
#define CheckCmdPos(hmenu,pos,b)  CheckMenuItem((hmenu),(pos),(b)?MF_BYPOSITION|MF_CHECKED:MF_BYPOSITION|MF_UNCHECKED)


#define DialogEnableWindow(hdlg, id, b) { HWND hctrl = GetDlgItem((hdlg),(id)); if (!(b)) { \
  if (GetFocus() == hctrl) { SendMessage((hdlg), WM_NEXTDLGCTL, 0, false); } }; EnableWindow(hctrl, (b)); }

#define GetString(id,pb,cb) LoadString(g_hInstance,id,pb,cb)

#define StrEnd(pStart) (pStart + lstrlen(pStart))

int FormatString(LPWSTR,int,UINT,...);

bool GetKnownFolderPath(REFKNOWNFOLDERID, LPWSTR, size_t);
void PathRelativeToApp(LPWSTR,LPWSTR,int,bool,bool,bool);
void PathAbsoluteFromApp(LPWSTR,LPWSTR,int,bool);


bool PathIsLnkFile(LPCWSTR);
bool PathGetLnkPath(LPCWSTR,LPWSTR,int);
bool PathIsLnkToDirectory(LPCWSTR,LPWSTR,int);
bool PathCreateDeskLnk(LPCWSTR);
bool PathCreateFavLnk(LPCWSTR,LPCWSTR,LPCWSTR);


bool StrLTrim(LPWSTR,LPCWSTR);
bool TrimString(LPWSTR);
bool ExtractFirstArgument(LPCWSTR, LPWSTR, LPWSTR, int);

void PrepareFilterStr(LPWSTR);

void StrTab2Space(LPWSTR);
void PathFixBackslashes(LPWSTR);


void  ExpandEnvironmentStringsEx(LPWSTR,DWORD);
void  PathCanonicalizeEx(LPWSTR,int);
DWORD GetLongPathNameEx(LPWSTR,DWORD);
DWORD NormalizePathEx(LPWSTR,int);
DWORD_PTR SHGetFileInfo2(LPCWSTR,DWORD,SHFILEINFO*,UINT,UINT);


int  FormatNumberStr(LPWSTR);
bool SetDlgItemIntEx(HWND,int,UINT);


#define MultiByteToWideCharStrg(c,a,w) MultiByteToWideChar((c),0,(a),-1,(w),COUNTOF(w))
#define WideCharToMultiByteStrg(c,w,a) WideCharToMultiByte((c),0,(w),-1,(a),COUNTOF(a),NULL,NULL)


UINT    GetDlgItemTextW2MB(HWND,int,LPSTR,int);
UINT    SetDlgItemTextMB2W(HWND,int,LPSTR);
LRESULT ComboBox_AddStringMB2W(HWND,LPCSTR);


UINT CodePageFromCharSet(UINT);
UINT CharSetFromCodePage(UINT);


//==== MRU Functions ==========================================================
#define MRU_MAXITEMS    32
#define MRU_ITEMSFILE   32
#define MRU_ITEMSFNDRPL 16
#define MRU_NOCASE    1
#define MRU_UTF8      2
#define MRU_BMRK_SIZE 512

typedef struct _mrulist {

  WCHAR  szRegKey[256];
  int    iFlags;
  int    iSize;
  LPWSTR pszItems[MRU_MAXITEMS];
  int    iEncoding[MRU_MAXITEMS];
  DocPos iCaretPos[MRU_MAXITEMS];
  LPWSTR pszBookMarks[MRU_MAXITEMS];
} 
MRULIST, *PMRULIST, *LPMRULIST;


LPMRULIST MRU_Create(LPCWSTR,int,int);
bool      MRU_Destroy(LPMRULIST);
bool      MRU_Add(LPMRULIST,LPCWSTR,int,DocPos,LPCWSTR);
bool      MRU_FindFile(LPMRULIST,LPCWSTR,int*);
bool      MRU_AddFile(LPMRULIST,LPCWSTR,bool,bool,int,DocPos,LPCWSTR);
bool      MRU_Delete(LPMRULIST,int);
bool      MRU_DeleteFileFromStore(LPMRULIST,LPCWSTR);
bool      MRU_Empty(LPMRULIST);
int       MRU_Enum(LPMRULIST,int,LPWSTR,int);
bool      MRU_Load(LPMRULIST);
bool      MRU_Save(LPMRULIST);
bool      MRU_MergeSave(LPMRULIST,bool,bool,bool);


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

bool GetThemedDialogFont(LPWSTR,WORD*);
DLGTEMPLATE* LoadThemedDialogTemplate(LPCTSTR,HINSTANCE);
#define ThemedDialogBox(hInstance,lpTemplate,hWndParent,lpDialogFunc) \
  ThemedDialogBoxParam(hInstance,lpTemplate,hWndParent,lpDialogFunc,0)
INT_PTR ThemedDialogBoxParam(HINSTANCE,LPCTSTR,HWND,DLGPROC,LPARAM);
HWND    CreateThemedDialogParam(HINSTANCE,LPCTSTR,HWND,DLGPROC,LPARAM);


//==== UnSlash Functions ======================================================
void TransformBackslashes(char*,bool,UINT,int*);
void TransformMetaChars(char*,bool,int);

//==== MinimizeToTray Functions - see comments in Helpers.c ===================
bool GetDoAnimateMinimize(VOID);
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
__forceinline DocPos StringCchLenA(LPCSTR s,size_t m) { size_t len; return (DocPos)(!s ? 0 : (SUCCEEDED(StringCchLengthA(s, m, &len)) ? len : m)); }
__forceinline DocPos StringCchLenW(LPCWSTR s,size_t m) { size_t len; return (DocPos)(!s ? 0 : (SUCCEEDED(StringCchLengthW(s, m, &len)) ? len : m)); }

#if defined(UNICODE) || defined(_UNICODE)  
#define StringCchLen(s,n)  StringCchLenW((s),(n))
#else
#define StringCchLen(s,n)  StringCchLenA((s),(n))
#endif

//==== StrSafe lstrcmp(),lstrcmpi() =============================================
__forceinline int _StringCchCmpNA(PCNZCH s1, DocPos l1,PCNZCH s2, DocPos l2)
{
  return (CompareStringA(LOCALE_INVARIANT,0,s1,(l1 >= 0 ? (int)StringCchLenA(s1,l1) : -1),
                         s2,(l2 >= 0 ? (int)StringCchLenA(s2,l2) : -1)) - CSTR_EQUAL);
}
#define StringCchCompareNA(s1,l1,s2,l2)  _StringCchCmpNA((s1),(l1),(s2),(l2))
#define StringCchCompareXA(s1,s2)        _StringCchCmpNA((s1),-1,(s2),-1)

__forceinline int _StringCchCmpINA(PCNZCH s1, DocPos l1,PCNZCH s2, DocPos l2)
{
  return (CompareStringA(LOCALE_INVARIANT,NORM_IGNORECASE,s1,(l1 >= 0 ? (int)StringCchLenA(s1,l1) : -1),
                         s2,(l2 >= 0 ? (int)StringCchLenA(s2,l2) : -1)) - CSTR_EQUAL);
}
#define StringCchCompareINA(s1,l1,s2,l2)  _StringCchCmpINA((s1),(l1),(s2),(l2))
#define StringCchCompareIXA(s1,s2)        _StringCchCmpINA((s1),-1,(s2),-1)

__forceinline int _StringCchCmpNW(PCNZWCH s1, DocPos l1,PCNZWCH s2, DocPos l2) {
  return (CompareStringW(LOCALE_INVARIANT,0,s1,(l1 >= 0 ? (int)StringCchLenW(s1,l1) : -1),
                         s2,(l2 >= 0 ? (int)StringCchLenW(s2,l2) : -1)) - CSTR_EQUAL);
}
#define StringCchCompareNW(s1,l1,s2,l2)  _StringCchCmpNW((s1),(l1),(s2),(l2))
#define StringCchCompareXW(s1,s2)        _StringCchCmpNW((s1),-1,(s2),-1)

__forceinline int _StringCchCmpINW(PCNZWCH s1, DocPos l1,PCNZWCH s2, DocPos l2) {
  return (CompareStringW(LOCALE_INVARIANT,NORM_IGNORECASE,s1,(l1 >= 0 ? (int)StringCchLenW(s1,l1) : -1),
                         s2,(l2 >= 0 ? (int)StringCchLenW(s2,l2) : -1)) - CSTR_EQUAL);
}
#define StringCchCompareINW(s1,l1,s2,l2)  _StringCchCmpINW((s1),(l1),(s2),(l2))
#define StringCchCompareIXW(s1,s2)        _StringCchCmpINW((s1),-1,(s2),-1)


#if defined(UNICODE) || defined(_UNICODE)  
#define StringCchCompareN(s1,l1,s2,l2)   StringCchCompareNW((s1),(l1),(s2),(l2))
#define StringCchCompareX(s1,s2)         StringCchCompareXW((s1),(s2))
#define StringCchCompareIN(s1,l1,s2,l2)  StringCchCompareINW((s1),(l1),(s2),(l2))
#define StringCchCompareIX(s1,s2)        StringCchCompareIXW((s1),(s2))
#else
#define StringCchCompareN(s1,l1,s2,l2)   StringCchCompareNA((s1),(l1),(s2),(l2))
#define StringCchCompareX(s1,s2)         StringCchCompareXA((s1),(s2))
#define StringCchCompareIN(s1,l1,s2,l2)  StringCchCompareINA((s1),(l1),(s2),(l2))
#define StringCchCompareIX(s1,s2)        StringCchCompareIXA((s1),(s2))
#endif


// Is the character an octal digit?
#define IsOctalDigit(ch) (((ch) >= '0') && ((ch) <= '7'))

// If the character is an hexa digit, get its value.
__forceinline int GetHexDigit(char ch) {
  if (ch >= '0' && ch <= '9') { return ch - '0'; }
  if (ch >= 'A' && ch <= 'F') { return ch - 'A' + 10; }
  if (ch >= 'a' && ch <= 'f') { return ch - 'a' + 10; }
  return -1;
}


void UrlUnescapeEx(LPWSTR, LPWSTR, DWORD*);

int ReadStrgsFromCSV(LPCWSTR wchCSVStrg, prefix_t sMatrix[], int const iCount, int const iLen, LPCWSTR sDefault);
int ReadVectorFromString(LPCWSTR wchStrg, int iVector[], int iCount, int iMin, int iMax, int iDefault);

// --------------------------------------------------------------------------------------------------------------------------------

// including <pathcch.h> and linking against pathcch.lib
// api-ms-win-core-path-l1-1-0.dll  library : Minimum supported client is Windows 8 :-/
// so switch back to previous (deprecated) methods:
__forceinline HRESULT PathCchAppend(PWSTR p,size_t l,PCWSTR a)          { UNUSED(l); return (PathAppend(p,a) ? S_OK : E_FAIL); }
__forceinline HRESULT PathCchCanonicalize(PWSTR p,size_t l,PCWSTR a)    { UNUSED(l); return (PathCanonicalize(p,a) ? S_OK : E_FAIL); }
__forceinline HRESULT PathCchRenameExtension(PWSTR p,size_t l,PCWSTR a) { UNUSED(l); return (PathRenameExtension(p,a) ? S_OK : E_FAIL); }
__forceinline HRESULT PathCchRemoveFileSpec(PWSTR p,size_t l)           { UNUSED(l); return (PathRemoveFileSpec(p) ? S_OK : E_FAIL); }



// special Drag and Drop Handling

typedef struct tDROPDATA
{
  CLIPFORMAT cf;
  POINTL pt;
  DWORD dwKeyState;
  HGLOBAL hData;
} 
DROPDATA, *PDROPDATA;

typedef struct tDROPTARGET *PDROPTARGET;
typedef DWORD(*DNDCALLBACK)(CLIPFORMAT cf, HGLOBAL hData, HWND hWnd, DWORD dwKeyState, POINTL pt, void *pUserData);

void DragAndDropInit(HANDLE hHeap);
PDROPTARGET RegisterDragAndDrop(HWND hWnd, CLIPFORMAT *pFormat, ULONG lFmt, UINT nMsg, DNDCALLBACK, void *pUserData);
PDROPTARGET RevokeDragAndDrop(PDROPTARGET pTarget);


#endif //_NP3_HELPERS_H_

///   End of Helpers.h   \\\
