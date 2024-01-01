// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Dlapi.h                                                                     *
*   Definitions for Directory Listing APIs                                    *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2024   *
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
typedef struct tagLV_ITEMDATA { // lvid
    LPITEMIDLIST  pidl; // Item Id
    LPSHELLFOLDER lpsf; // Parent IShellFolder Interface

} LV_ITEMDATA, *LPLV_ITEMDATA;


//==== DlInit() ===============================================================

bool DirList_Init(HWND hwnd, LPCWSTR pszHeader, const HPATHL hFilePath);


//==== DlDestroy() ============================================================

bool DirList_Destroy(HWND hwnd);


//==== DlStartIconThread() ====================================================

void DirList_StartIconThread(HWND hwnd);


//==== DlFill() ===============================================================

#define DL_FOLDERS      32
#define DL_NONFOLDERS   64
#define DL_INCLHIDDEN  128
#define DL_ALLOBJECTS  (32|64|128)

int DirList_Fill(HWND hwnd,LPCWSTR lpszDir,DWORD grfFlags,LPCWSTR lpszFileSpec,bool,bool,int iSortFlags,bool);


//==== DlIconThread() =========================================================

unsigned int WINAPI DirList_IconThread(LPVOID lpParam);


//==== DlGetDispInfo() ========================================================

bool DirList_GetDispInfo(HWND hwnd,LPARAM lParam,bool);


//==== DlDeleteItem() =========================================================

bool DirList_DeleteItem(HWND hwnd,LPARAM lParam);


//==== DlSort() ===============================================================

#define DS_NAME     0
#define DS_SIZE     1
#define DS_TYPE     2
#define DS_LASTMOD  3

bool DirList_Sort(HWND hwnd,int lFlags,bool);


//==== DlGetItem() ============================================================

#define DLE_NONE 0
#define DLE_DIR  1
#define DLE_FILE 2

#define DLI_FILENAME 1
#define DLI_DISPNAME 2
#define DLI_TYPE     4
#define DLI_ALL (1|2|4)

typedef struct tagDLITEM { // dli

    UINT mask;
    wchar_t* pthFileName;
    wchar_t* strDisplayName;
    int  ntype;

} DLITEM, *LPDLITEM;

int DirList_GetItem(HWND hwnd,int iItem,LPDLITEM lpdli);


//==== DlGetItemEx() ==========================================================

int DirList_GetItemEx(HWND hwnd,int iItem,LPWIN32_FIND_DATA pfd);


//==== DlPropertyDlg() ========================================================

bool DirList_PropertyDlg(HWND hwnd,int iItem);


//==== DlGetLongPathName() ====================================================

//bool DirList_GetLongPathName(HWND hwnd,LPWSTR lpszLongPath,int length);

//==== DlSelectItem() =========================================================

bool DirList_SelectItem(HWND hwnd,LPCWSTR lpszDisplayName,LPCWSTR lpszFullPath);

//==== DlCreateFilter() and DlMatchFilter() ===================================

#define DL_FILTER_BUFSIZE 128

typedef struct tagDL_FILTER { //dlf
    int   nCount;
    WCHAR  tFilterBuf[DL_FILTER_BUFSIZE];
    WCHAR  *pFilter  [DL_FILTER_BUFSIZE];
    bool  bExcludeFilter;
} DL_FILTER, *PDL_FILTER;

void DirList_CreateFilter(PDL_FILTER pdlf,LPCWSTR lpszFileSpec,bool);

bool DirList_MatchFilter(LPSHELLFOLDER lpsf,LPCITEMIDLIST pidl,PDL_FILTER pdlf);



//==== DriveBox ===============================================================

//bool DriveBox_Init(HWND hwnd);
//int  DriveBox_Fill(HWND hwnd);
//bool DriveBox_GetSelDrive(HWND hwnd,LPWSTR lpszDrive,int nDrive,bool);
//bool DriveBox_SelectDrive(HWND hwnd,LPCWSTR lpszPath);
//bool DriveBox_PropertyDlg(HWND hwnd);

//LRESULT DriveBox_DeleteItem(HWND hwnd,LPARAM lParam);
//LRESULT DriveBox_GetDispInfo(HWND hwnd,LPARAM lParam);



//==== ItemID =================================================================

//==== IL_Next() ==============================================================
#define _IL_Next(pidl) ((LPITEMIDLIST)(((LPBYTE)(pidl)) + pidl->mkid.cb))

//==== IL_Create() ============================================================
LPITEMIDLIST IL_Create(LPCITEMIDLIST pidl1,UINT cb1,
                       LPCITEMIDLIST pidl2,UINT cb2);

//==== IL_GetSize() ===========================================================
UINT IL_GetSize(LPCITEMIDLIST pidl);

//==== IL_GetDisplayName() ====================================================
bool IL_GetDisplayName(LPSHELLFOLDER lpsf,
                       LPCITEMIDLIST pidl,
                       DWORD dwFlags,LPWSTR lpszDisplayName,int nDisplayName);




#ifdef __cplusplus
}
#endif //__cplusplus


#endif //_NP3_DLAPI_H_

///   End of Dlapi.h   ///
