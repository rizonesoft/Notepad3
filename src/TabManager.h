// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* TabManager.h                                                                *
*   Multi-Tab Document Manager                                                *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2026   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#pragma once
#ifndef _NP3_TABMANAGER_H_
#define _NP3_TABMANAGER_H_

#include "TypeDefs.h"
#include "SciCall.h"

// Forward declarations
typedef struct TabDoc TabDoc;

// Maximum number of tabs
#define MAX_TABS  64

// Tab document structure - each tab represents one Scintilla document
struct TabDoc {
    sptr_t      pDocPointer;       // Scintilla document pointer (SCI_CREATEDOCUMENT)
    HPATHL      hFilePath;         // File path (NULL for untitled)
    bool        bModified;         // Modified flag
    bool        bReadOnly;         // Read-only flag
    WCHAR       wchTabText[256];   // Tab display text
    FILEVARS    fvFileVars;        // File variables for this tab
    cpi_enc_t   iEncoding;         // Encoding for this tab
    int         iEOLMode;          // EOL mode for this tab
};

// Globals accessed from outside
extern HWND      g_hwndTabControl;
extern TabDoc*   g_pActiveTab;
extern int       g_nTabCount;

//=============================================================================
// Initialization / Cleanup
//=============================================================================

void TabManager_Init(HWND hwndMain, HINSTANCE hInstance);
void TabManager_Cleanup(HWND hwndMain);
void TabManager_RegisterInitialTab(HWND hwndMain);

//=============================================================================
// Tab Operations
//=============================================================================

TabDoc* TabManager_AddNewTab(HWND hwndMain);
bool TabManager_CloseTab(int nIndex);
void TabManager_CloseAllOtherTabs(int nKeepIndex);
void TabManager_CloseAllTabs(HWND hwndMain);
void TabManager_SwitchToTab(HWND hwndMain, int nIndex);

//=============================================================================
// Tab Info / Queries
//=============================================================================

int TabManager_GetActiveTabIndex(void);
TabDoc* TabManager_GetTab(int nIndex);

//=============================================================================
// UI Helpers
//=============================================================================

void TabManager_UpdateTabText(int nIndex);
void TabManager_UpdateActiveTabText(void);
LRESULT TabManager_HandleNotify(HWND hwndMain, WPARAM wParam, LPARAM lParam);

//=============================================================================
// Constants
//=============================================================================

#define IDC_TAB          (0xFB09)   // Tab control ID

#endif //_NP3_TABMANAGER_H_
///   End of TabManager.h   ///
