// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* MiniPath - Notepad3 Explorer Plugin                                         *
*                                                                             *
* Dlapi.h                                                                     *
*   Definitions for Directory Listing APIs                                    *
*   Based on code from metapath, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2023   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

#ifndef _DLAPI_H_
#define _DLAPI_H_

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

BOOL DirList_Init(HWND,LPCWSTR);


//==== DlDestroy() ============================================================

BOOL DirList_Destroy(HWND);


//==== DlStartIconThread() ====================================================

BOOL DirList_StartIconThread(HWND);


//==== DlTerminateIconThread() ================================================

BOOL DirList_TerminateIconThread(HWND);


//==== DlFill() ===============================================================

#define DL_FOLDERS      32
#define DL_NONFOLDERS   64
#define DL_INCLHIDDEN  128
#define DL_ALLOBJECTS  (32|64|128)

int DirList_Fill(HWND,LPCWSTR,DWORD,LPCWSTR,BOOL,BOOL,int,BOOL);


//==== DlIconThread() =========================================================

void WINAPIV DirList_IconThread(LPVOID);


//==== DlGetDispInfo() ========================================================

BOOL DirList_GetDispInfo(HWND,LPARAM,BOOL);


//==== DlDeleteItem() =========================================================

BOOL DirList_DeleteItem(HWND,LPARAM);


//==== DlSort() ===============================================================

#define DS_NAME     0
#define DS_SIZE     1
#define DS_TYPE     2
#define DS_LASTMOD  3

BOOL DirList_Sort(HWND,int,BOOL);


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
    WCHAR szFileName[MAX_PATH];
    WCHAR szDisplayName[MAX_PATH];
    int  ntype;

} DLITEM, *LPDLITEM;

int DirList_GetItem(HWND,int,LPDLITEM);


//==== DlGetItemEx() ==========================================================

int DirList_GetItemEx(HWND,int,LPWIN32_FIND_DATA);


//==== DlPropertyDlg() ========================================================

BOOL DirList_PropertyDlg(HWND,int);


//==== DlDoDragDrop() =========================================================

void DirList_DoDragDrop(HWND,LPARAM);

//==== DlGetLongPathName() ====================================================

BOOL DirList_GetLongPathName(HWND,LPWSTR);

//==== DlSelectItem() =========================================================

BOOL DirList_SelectItem(HWND,LPCWSTR,LPCWSTR);

//==== DlCreateFilter() and DlMatchFilter() ===================================

#define DL_FILTER_BUFSIZE 256

typedef struct tagDL_FILTER { //dlf
    int   nCount;
    WCHAR  tFilterBuf[DL_FILTER_BUFSIZE];
    WCHAR* pFilter[DL_FILTER_BUFSIZE];
    BOOL  bExcludeFilter;
} DL_FILTER, * PDL_FILTER;


void DirList_CreateFilter(PDL_FILTER,LPCWSTR,BOOL);

BOOL DirList_MatchFilter(LPSHELLFOLDER,LPCITEMIDLIST,PDL_FILTER);



//==== DriveBox ===============================================================

BOOL DriveBox_Init(HWND);
int  DriveBox_Fill(HWND);
BOOL DriveBox_GetSelDrive(HWND,LPWSTR,int,BOOL);
BOOL DriveBox_SelectDrive(HWND,LPCWSTR);
BOOL DriveBox_PropertyDlg(HWND);

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
BOOL IL_GetDisplayName(LPSHELLFOLDER,
                       LPCITEMIDLIST,
                       DWORD,LPWSTR,int);




#ifdef __cplusplus
}
#endif //__cplusplus


#endif // _DLAPI_H_


///   End of Dlapi.h   \\\
