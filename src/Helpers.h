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
#include <math.h>
#include <strsafe.h>
#include <shlwapi.h>
#include <versionhelpers.h>

#include "typedefs.h"

// ============================================================================

extern WCHAR   g_wchIniFile[MAX_PATH];
extern UINT    g_uCurrentDPI;
extern UINT    g_uCurrentPPI;

// ============================================================================

#ifndef _T
#if !defined(ISPP_INVOKED) && (defined(UNICODE) || defined(_UNICODE))
#define _T(text) L##text
#else
#define _T(text) text
#endif
#endif

#define STRGFY(X)     L##(X)
#define MKWSTRG(strg) STRGFY(strg)

#define UNUSED(expr) (void)(expr)
#define SIZEOF(ar) sizeof(ar)
#define COUNTOF(ar) ARRAYSIZE(ar)   //#define COUNTOF(ar) (sizeof(ar)/sizeof(ar[0]))
#define CSTRLEN(s)  (COUNTOF(s)-1)

#define NOOP ((void)0)

// ============================================================================

#if defined(DEBUG) && !defined(NDEBUG)
void DbgLog(const char *fmt, ...);
#else
#define DbgLog(fmt, ...) NOOP
#endif

// ============================================================================

// swap 
static inline void swapi(int* a, int* b) { int t = *a;  *a = *b;  *b = t; }
static inline void swapos(DocPos* a, DocPos* b) { DocPos t = *a;  *a = *b;  *b = t; }

// clamp
static inline int clampi(int x, int lower, int upper) {
  return (x < lower) ? lower : ((x > upper) ? upper : x);
}

static inline unsigned clampu(unsigned x, unsigned lower, unsigned upper) {
  return (x < lower) ? lower : ((x > upper) ? upper : x);
}


// Is the character an octal digit?
static inline bool IsDigit(CHAR ch) { return ((ch >= '0') && (ch <= '9')); }
static inline bool IsDigitW(WCHAR wch) { return ((wch >= L'0') && (wch <= L'9')); }

// Is the character a white space char?
static inline bool IsBlankChar(CHAR ch) { return ((ch == ' ') || (ch == '\t')); }
static inline bool IsBlankCharW(WCHAR wch) { return ((wch == L' ') || (wch == L'\t')); }


static inline int float2int(float f) { return (int)lroundf(f); }
static inline float Round10th(float f) { return (float)float2int(f * 10.0f) / 10; }
static inline bool HasNonZeroFraction(float f) { return ((float2int(f * 10.0f) % 10) != 0); }


// direct heap allocation
static inline LPVOID AllocMem(size_t numBytes, DWORD dwFlags)
{
  return HeapAlloc(GetProcessHeap(), (dwFlags | HEAP_GENERATE_EXCEPTIONS), numBytes);
}

static inline bool FreeMem(LPVOID lpMemory)
{
  return ((lpMemory != NULL) ? HeapFree(GetProcessHeap(), 0, lpMemory) : true);
}

static inline size_t SizeOfMem(LPVOID lpMemory)
{
  return ((lpMemory != NULL) ? HeapSize(GetProcessHeap(), 0, lpMemory) : 0);
}

// ----------------------------------------------------------------------------

#define IniGetString(lpSection,lpName,lpDefault,lpReturnedStr,nSize) GetPrivateProfileString(lpSection,lpName,(lpDefault),(lpReturnedStr),(nSize),g_wchIniFile)
#define IniGetInt(lpSection,lpName,nDefault)                         GetPrivateProfileInt(lpSection,lpName,(nDefault),g_wchIniFile)
#define IniGetBool(lpSection,lpName,nDefault)                        (GetPrivateProfileInt(lpSection,lpName,(int)(nDefault),g_wchIniFile) ? true : false)
#define IniSetString(lpSection,lpName,lpString)                      WritePrivateProfileString(lpSection,lpName,(lpString),g_wchIniFile)
#define IniDeleteSection(lpSection)                                  WritePrivateProfileSection(lpSection,NULL,g_wchIniFile)

static inline bool IniSetInt(LPCWSTR lpSection, LPCWSTR lpName, int i) {
  WCHAR tch[32] = { L'\0' }; StringCchPrintf(tch, COUNTOF(tch), L"%i", i); return IniSetString(lpSection, lpName, tch);
}

#define IniSetBool(lpSection,lpName,nValue)    IniSetInt(lpSection,lpName,((nValue) ? 1 : 0))
#define LoadIniSection(lpSection,lpBuf,cchBuf) GetPrivateProfileSection(lpSection,lpBuf,(cchBuf),g_wchIniFile)
#define SaveIniSection(lpSection,lpBuf)        WritePrivateProfileSection(lpSection,lpBuf,g_wchIniFile)

int IniSectionGetString(LPCWSTR, LPCWSTR, LPCWSTR, LPWSTR, int);
int IniSectionGetInt(LPCWSTR, LPCWSTR, int);
UINT IniSectionGetUInt(LPCWSTR, LPCWSTR, UINT);
DocPos IniSectionGetPos(LPCWSTR, LPCWSTR, DocPos);
static inline bool IniSectionGetBool(LPCWSTR lpCachedIniSection, LPCWSTR lpName, bool bDefault) {
  return (IniSectionGetInt(lpCachedIniSection, lpName, ((bDefault) ? 1 : 0)) ? true : false);
}

bool IniSectionSetString(LPWSTR,LPCWSTR,LPCWSTR);

inline bool IniSectionSetInt(LPWSTR lpCachedIniSection,LPCWSTR lpName, int i) {
  WCHAR tch[32]={L'\0'}; StringCchPrintf(tch,COUNTOF(tch),L"%i",i); return IniSectionSetString(lpCachedIniSection,lpName,tch);
}
inline bool IniSectionSetBool(LPWSTR lpCachedIniSection, LPCWSTR lpName, bool b) {
  return IniSectionSetInt(lpCachedIniSection, lpName, (b ? 1 : 0));
}
inline bool IniSectionSetPos(LPWSTR lpCachedIniSection, LPCWSTR lpName, DocPos pos)
{
  WCHAR tch[64] = { L'\0' }; StringCchPrintf(tch, COUNTOF(tch), L"%td", (long long)pos); return IniSectionSetString(lpCachedIniSection, lpName, tch);
}

// ----------------------------------------------------------------------------

static inline COLORREF GetBackgroundColor(HWND hwnd) { return GetBkColor(GetDC(hwnd)); }

DWORD GetLastErrorToMsgBox(LPWSTR lpszFunction, DWORD dwErrID);

// ----------------------------------------------------------------------------


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

UINT GetCurrentDPI(HWND hwnd);
UINT GetCurrentPPI(HWND hwnd);
HBITMAP ResizeImageForCurrentDPI(HBITMAP hbmp);
#define ScaleIntFontSize(val) MulDiv((val), g_uCurrentDPI, g_uCurrentPPI)
inline int ScaleFontSize(float fSize) { return float2int((fSize * g_uCurrentDPI) / (float)g_uCurrentPPI); }
inline int ScaleFractionalFontSize(float fSize) { return float2int((fSize * 10.0f * g_uCurrentDPI) / (float)g_uCurrentPPI) * 10; }

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

bool IsCmdEnabled(HWND, UINT);


#define DlgBtnChk(b) ((b) ? BST_CHECKED : BST_UNCHECKED)

#define EnableCmd(hmenu,id,b) EnableMenuItem((hmenu),(id),(b)?MF_BYCOMMAND|MF_ENABLED:MF_BYCOMMAND|MF_GRAYED)
#define CheckCmd(hmenu,id,b)  CheckMenuItem((hmenu),(id),(b)?MF_BYCOMMAND|MF_CHECKED:MF_BYCOMMAND|MF_UNCHECKED)

#define EnableCmdPos(hmenu,pos,b) EnableMenuItem((hmenu),(pos),(b)?MF_BYPOSITION|MF_ENABLED:MF_BYPOSITION|MF_GRAYED)
#define CheckCmdPos(hmenu,pos,b)  CheckMenuItem((hmenu),(pos),(b)?MF_BYPOSITION|MF_CHECKED:MF_BYPOSITION|MF_UNCHECKED)


#define DialogEnableWindow(hdlg, id, b) { HWND hctrl = GetDlgItem((hdlg),(id)); if (!(b)) { \
  if (GetFocus() == hctrl) { SendMessage((hdlg), WM_NEXTDLGCTL, 0, false); } }; EnableWindow(hctrl, (b)); }


#define GetLngString(id,pb,cb) LoadLngStringW((id),(pb),(cb))
#define GetLngStringA(id,pb,cb) LoadLngStringA((id),(pb),(cb))
int LoadLngStringW(UINT uID, LPWSTR lpBuffer, int nBufferMax);
int LoadLngStringA(UINT uID, LPSTR lpBuffer, int nBufferMax);
int FormatLngStringW(LPWSTR, int, UINT, ...);
int FormatLngStringA(LPSTR, int, UINT, ...);


bool GetKnownFolderPath(REFKNOWNFOLDERID, LPWSTR, size_t);
void PathRelativeToApp(LPWSTR,LPWSTR,int,bool,bool,bool);
void PathAbsoluteFromApp(LPWSTR,LPWSTR,int,bool);


bool PathIsLnkFile(LPCWSTR);
bool PathGetLnkPath(LPCWSTR,LPWSTR,int);
bool PathIsLnkToDirectory(LPCWSTR,LPWSTR,int);
bool PathCreateDeskLnk(LPCWSTR);
bool PathCreateFavLnk(LPCWSTR,LPCWSTR,LPCWSTR);


bool StrLTrim(LPWSTR,LPCWSTR);
inline bool TrimStringA(LPSTR lpString) {
  if (!lpString || !*lpString) { return false; }
  StrTrimA(lpString, " ");
  return true;
};
inline bool TrimStringW(LPWSTR lpString) {
  if (!lpString || !*lpString) { return false; }
  StrTrimW(lpString, L" ");
  return true;
};
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
#define   MRU_Count(pmru) MRU_Enum((pmru), 0, NULL, 0)

//==== UnSlash Functions ======================================================
void TransformBackslashes(char*,bool,UINT,int*);
void TransformMetaChars(char*,bool,int);

//==== MinimizeToTray Functions - see comments in Helpers.c ===================
bool GetDoAnimateMinimize(VOID);
VOID MinimizeWndToTray(HWND hWnd);
VOID RestoreWndFromTray(HWND hWnd);

//==== StrCut methods ===================

CHAR*  StrCutIA(CHAR*,const CHAR*);
WCHAR* StrCutIW(WCHAR*,const WCHAR*);
#if defined(UNICODE) || defined(_UNICODE)  
#define StrCutI StrCutIW
#else
#define StrCutI _StrCutIA
#endif


//==== StrNextTok methods ===================
CHAR*  StrNextTokA(CHAR*, const CHAR*);
WCHAR* StrNextTokW(WCHAR*, const WCHAR*);
#if defined(UNICODE) || defined(_UNICODE)  
#define StrNextTok StrNextTokW
#else
#define StrNextTok StrNextTokA
#endif

// ----------------------------------------------------------------------------

#define StrEndW(pStart) (pStart + lstrlen(pStart))
#define StrEndA(pStart) (pStart + strlen(pStart))

#if defined(UNICODE) || defined(_UNICODE)  
#define StrEnd(s) StrEndW(s)
#else
#define StrEnd(s) StrEndA(s)
#endif

// ----------------------------------------------------------------------------

bool StrDelChrA(LPSTR pszSource, LPCSTR pCharsToRemove);


//==== StrSafe lstrlen() =======================================================
inline DocPos StringCchLenA(LPCSTR s,size_t m) { size_t len; return (DocPos)(!s ? 0 : (SUCCEEDED(StringCchLengthA(s, m, &len)) ? len : m)); }
inline DocPos StringCchLenW(LPCWSTR s,size_t m) { size_t len; return (DocPos)(!s ? 0 : (SUCCEEDED(StringCchLengthW(s, m, &len)) ? len : m)); }

#if defined(UNICODE) || defined(_UNICODE)  
#define StringCchLen(s,n)  StringCchLenW((s),(n))
#else
#define StringCchLen(s,n)  StringCchLenA((s),(n))
#endif

//==== StrSafe lstrcmp(),lstrcmpi() =============================================
inline int _StringCchCmpNA(PCNZCH s1, DocPos l1,PCNZCH s2, DocPos l2)
{
  return (CompareStringA(LOCALE_INVARIANT,0,s1,(l1 >= 0 ? (int)StringCchLenA(s1,l1) : -1),
                         s2,(l2 >= 0 ? (int)StringCchLenA(s2,l2) : -1)) - CSTR_EQUAL);
}
#define StringCchCompareNA(s1,l1,s2,l2)  _StringCchCmpNA((s1),(l1),(s2),(l2))
#define StringCchCompareXA(s1,s2)        _StringCchCmpNA((s1),-1,(s2),-1)

inline int _StringCchCmpINA(PCNZCH s1, DocPos l1,PCNZCH s2, DocPos l2)
{
  return (CompareStringA(LOCALE_INVARIANT,NORM_IGNORECASE,s1,(l1 >= 0 ? (int)StringCchLenA(s1,l1) : -1),
                         s2,(l2 >= 0 ? (int)StringCchLenA(s2,l2) : -1)) - CSTR_EQUAL);
}
#define StringCchCompareINA(s1,l1,s2,l2)  _StringCchCmpINA((s1),(l1),(s2),(l2))
#define StringCchCompareIXA(s1,s2)        _StringCchCmpINA((s1),-1,(s2),-1)

inline int _StringCchCmpNW(PCNZWCH s1, DocPos l1,PCNZWCH s2, DocPos l2) {
  return (CompareStringW(LOCALE_INVARIANT,0,s1,(l1 >= 0 ? (int)StringCchLenW(s1,l1) : -1),
                         s2,(l2 >= 0 ? (int)StringCchLenW(s2,l2) : -1)) - CSTR_EQUAL);
}
#define StringCchCompareNW(s1,l1,s2,l2)  _StringCchCmpNW((s1),(l1),(s2),(l2))
#define StringCchCompareXW(s1,s2)        _StringCchCmpNW((s1),-1,(s2),-1)

inline int _StringCchCmpINW(PCNZWCH s1, DocPos l1,PCNZWCH s2, DocPos l2) {
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

//==== StrIs(Not)Empty() =============================================

static inline bool StrIsEmptyA(LPCSTR s) { return ((s == NULL) || (*s == '\0')); }
static inline bool StrIsEmptyW(LPCWSTR s) { return ((s == NULL) || (*s == L'\0')); }

#if defined(UNICODE) || defined(_UNICODE)
#define StrIsEmpty(s)     StrIsEmptyW(s)
#define StrIsNotEmpty(s)  (!StrIsEmptyW(s))
#else
#define StrIsEmpty(s)     StrIsEmptyA(s)
#define StrIsNotEmpty(s)  (!StrIsEmptyA(s))
#endif

// ----------------------------------------------------------------------------

// Is the character an octal digit?
#define IsOctalDigit(ch) (((ch) >= '0') && ((ch) <= '7'))

// If the character is an hexa digit, get its value.
inline int GetHexDigit(char ch) {
  if (ch >= '0' && ch <= '9') { return ch - '0'; }
  if (ch >= 'A' && ch <= 'F') { return ch - 'A' + 10; }
  if (ch >= 'a' && ch <= 'f') { return ch - 'a' + 10; }
  return -1;
}

// ----------------------------------------------------------------------------

void UrlUnescapeEx(LPWSTR, LPWSTR, DWORD*);

int ReadStrgsFromCSV(LPCWSTR wchCSVStrg, prefix_t sMatrix[], int const iCount, int const iLen, LPCWSTR sDefault);
int ReadVectorFromString(LPCWSTR wchStrg, int iVector[], int iCount, int iMin, int iMax, int iDefault);

bool Char2FloatW(WCHAR* wnumber, float* fresult);
void Float2String(float fValue, LPWSTR lpszStrg, int cchSize);

// ----------------------------------------------------------------------------

// including <pathcch.h> and linking against pathcch.lib
// api-ms-win-core-path-l1-1-0.dll  library : Minimum supported client is Windows 8 :-/
// so switch back to previous (deprecated) methods:
inline HRESULT PathCchAppend(PWSTR p,size_t l,PCWSTR a)          { UNUSED(l); return (PathAppend(p,a) ? S_OK : E_FAIL); }
inline HRESULT PathCchCanonicalize(PWSTR p,size_t l,PCWSTR a)    { UNUSED(l); return (PathCanonicalize(p,a) ? S_OK : E_FAIL); }
inline HRESULT PathCchRenameExtension(PWSTR p,size_t l,PCWSTR a) { UNUSED(l); return (PathRenameExtension(p,a) ? S_OK : E_FAIL); }
inline HRESULT PathCchRemoveFileSpec(PWSTR p,size_t l)           { UNUSED(l); return (PathRemoveFileSpec(p) ? S_OK : E_FAIL); }


#define _EXTRA_DRAG_N_DROP_HANDLER_ 1

#ifdef _EXTRA_DRAG_N_DROP_HANDLER_

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

#endif


#endif //_NP3_HELPERS_H_

///   End of Helpers.h   \\\
