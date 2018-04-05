/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Dlapi.h                                                                     *
*   Definitions for Directory Listing APIs                                    *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2016   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#pragma once

#ifndef _NP3_DLAPI_H_
#define _NP3_DLAPI_H_

#ifdef __cplusplus
extern "C" { // C-Declarations
#endif //__cplusplus



//==== DirList ================================================================

//==== LV_ITEMDATA Structure ==================================================
typedef struct tagLV_ITEMDATA // lvid
{
  LPITEMIDLIST  pidl; // Item Id
  LPSHELLFOLDER lpsf; // Parent IShellFolder Interface

} LV_ITEMDATA, *LPLV_ITEMDATA;


//==== DlInit() ===============================================================

bool DirList_Init(HWND,LPCWSTR);


//==== DlDestroy() ============================================================

bool DirList_Destroy(HWND);


//==== DlStartIconThread() ====================================================

bool DirList_StartIconThread(HWND);


//==== DlTerminateIconThread() ================================================

bool DirList_TerminateIconThread(HWND);


//==== DlFill() ===============================================================

#define DL_FOLDERS      32
#define DL_NONFOLDERS   64
#define DL_INCLHIDDEN  128
#define DL_ALLOBJECTS  (32|64|128)

int DirList_Fill(HWND,LPCWSTR,DWORD,LPCWSTR,bool,bool,int,bool);


//==== DlIconThread() =========================================================

DWORD WINAPI DirList_IconThread(LPVOID);


//==== DlGetDispInfo() ========================================================

bool DirList_GetDispInfo(HWND,LPARAM,bool);


//==== DlDeleteItem() =========================================================

bool DirList_DeleteItem(HWND,LPARAM);


//==== DlSort() ===============================================================

#define DS_NAME     0
#define DS_SIZE     1
#define DS_TYPE     2
#define DS_LASTMOD  3

bool DirList_Sort(HWND,int,bool);


//==== DlGetItem() ============================================================

#define DLE_NONE 0
#define DLE_DIR  1
#define DLE_FILE 2

#define DLI_FILENAME 1
#define DLI_DISPNAME 2
#define DLI_TYPE     4
#define DLI_ALL (1|2|4)

typedef struct tagDLITEM // dli
{

  UINT mask;
  WCHAR szFileName[MAX_PATH];
  WCHAR szDisplayName[MAX_PATH];
  int  ntype;

} DLITEM, *LPDLITEM;

int DirList_GetItem(HWND,int,LPDLITEM);


//==== DlGetItemEx() ==========================================================

int DirList_GetItemEx(HWND,int,LPWIN32_FIND_DATA);


//==== DlPropertyDlg() ========================================================

bool DirList_PropertyDlg(HWND,int);


//==== DlGetLongPathName() ====================================================

bool DirList_GetLongPathName(HWND,LPWSTR,int);

//==== DlSelectItem() =========================================================

bool DirList_SelectItem(HWND,LPCWSTR,LPCWSTR);

//==== DlCreateFilter() and DlMatchFilter() ===================================

#define DL_FILTER_BUFSIZE 128

typedef struct tagDL_FILTER { //dlf
  int   nCount;
  WCHAR  tFilterBuf[DL_FILTER_BUFSIZE];
  WCHAR  *pFilter  [DL_FILTER_BUFSIZE];
  bool  bExcludeFilter;
} DL_FILTER, *PDL_FILTER;

void DirList_CreateFilter(PDL_FILTER,LPCWSTR,bool);

bool DirList_MatchFilter(LPSHELLFOLDER,LPCITEMIDLIST,PDL_FILTER);



//==== DriveBox ===============================================================

bool DriveBox_Init(HWND);
int  DriveBox_Fill(HWND);
bool DriveBox_GetSelDrive(HWND,LPWSTR,int,bool);
bool DriveBox_SelectDrive(HWND,LPCWSTR);
bool DriveBox_PropertyDlg(HWND);

LRESULT DriveBox_DeleteItem(HWND,LPARAM);
LRESULT DriveBox_GetDispInfo(HWND,LPARAM);



//==== ItemID =================================================================

//==== IL_Next() ==============================================================
#define _IL_Next(pidl) ((LPITEMIDLIST)(((LPBYTE)(pidl)) + pidl->mkid.cb))

//==== IL_Create() ============================================================
LPITEMIDLIST IL_Create(LPCITEMIDLIST,UINT,
                       LPCITEMIDLIST,UINT);

//==== IL_GetSize() ===========================================================
UINT IL_GetSize(LPCITEMIDLIST);

//==== IL_GetDisplayName() ====================================================
bool IL_GetDisplayName(LPSHELLFOLDER,
                       LPCITEMIDLIST,
                       DWORD,LPWSTR,int);




#ifdef __cplusplus
}
#endif //__cplusplus


#endif //_NP3_DLAPI_H_


///   End of Dlapi.h   \\\
