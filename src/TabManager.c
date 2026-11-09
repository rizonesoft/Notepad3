// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* TabManager.c                                                                *
*   Multi-Tab Document Manager Implementation                                 *
*   Uses Scintilla's native multi-document API:                              *
*     SCI_CREATEDOCUMENT / SCI_SETDOCPOINTER / SCI_RELEASEDOCUMENT           *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2026   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

#include "TypeDefs.h"
#include "Notepad3.h"
#include "TabManager.h"
#include "SciCall.h"
#include "Helpers.h"
#include "PathLib.h"
#include "Encoding.h"
#include "Dialogs.h"

#include <commctrl.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <stdio.h>

#ifdef D_NP3_WIN10_DARK_MODE
#include "DarkMode/DarkMode.h"
#endif

//=============================================================================
// Globals
//=============================================================================

HWND      g_hwndTabControl = NULL;
TabDoc*   g_pActiveTab = NULL;
int       g_nTabCount = 0;
TabDoc    g_Tabs[MAX_TABS];

//=============================================================================
// Internal Helpers
//=============================================================================

static void _TraceStep(const char* pszStep)
{
    FILE* pFile = NULL;
    if (_wfopen_s(&pFile, L"C:\\Users\\Administrator\\Documents\\Trae-CN\\temp_20261109_01\\tab_trace.log", L"a") == 0 && pFile) {
        fprintf(pFile, "%s\n", pszStep);
        fflush(pFile);
        fclose(pFile);
    }
}

static void _BuildTabText(TabDoc* pTab, int nUntitledIndex)
{
    if (!pTab) return;

    if (Path_IsNotEmpty(pTab->hFilePath)) {
        const WCHAR* pName = Path_FindFileName(pTab->hFilePath);
        StringCchPrintf(pTab->wchTabText, COUNTOF(pTab->wchTabText),
                        L"%s%s", pTab->bModified ? L"* " : L"", pName ? pName : L"Untitled");
    } else {
        StringCchPrintf(pTab->wchTabText, COUNTOF(pTab->wchTabText),
                        L"%sUntitled %i", pTab->bModified ? L"* " : L"", nUntitledIndex);
    }
}

static void _RefreshTabControlItem(int nIndex)
{
    if (!g_hwndTabControl || nIndex < 0 || nIndex >= g_nTabCount) return;

    TCITEM tci = { 0 };
    tci.mask = TCIF_TEXT;
    tci.pszText = g_Tabs[nIndex].wchTabText;
    tci.cchTextMax = (int)COUNTOF(g_Tabs[nIndex].wchTabText);
    TabCtrl_SetItem(g_hwndTabControl, nIndex, &tci);
}

// Re-run the main window layout so that the tab bar is shown/hidden correctly
static void _RelayoutMainWindow(HWND hwndMain)
{
    if (!hwndMain) return;

    RECT rc;
    if (!GetClientRect(hwndMain, &rc)) return;

    SendMessage(hwndMain, WM_SIZE, (WPARAM)SIZE_RESTORED,
                MAKELONG((WORD)(rc.right - rc.left), (WORD)(rc.bottom - rc.top)));
}

//=============================================================================
// Initialization / Cleanup
//=============================================================================

void TabManager_Init(HWND hwndMain, HINSTANCE hInstance)
{
    g_hwndTabControl = CreateWindowEx(
        0,
        WC_TABCONTROL,
        NULL,
        (WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TCS_SINGLELINE | TCS_RIGHTJUSTIFY | TCS_TABS) & ~WS_VISIBLE,
        0, 0, 0, 0,
        hwndMain,
        (HMENU)(INT_PTR)IDC_TAB,
        hInstance,
        NULL);

    if (!g_hwndTabControl) {
        return;
    }

    InitWindowCommon(g_hwndTabControl, true);

#ifdef D_NP3_WIN10_DARK_MODE
    if (IsDarkModeSupported()) {
        AllowDarkModeForWindowEx(g_hwndTabControl, CheckDarkModeEnabled());
    }
#endif

    for (int i = 0; i < MAX_TABS; i++) {
        g_Tabs[i].pDocPointer = 0;
        g_Tabs[i].hFilePath = NULL;
        g_Tabs[i].bModified = false;
        g_Tabs[i].bReadOnly = false;
        g_Tabs[i].wchTabText[0] = L'\0';
        memset(&g_Tabs[i].fvFileVars, 0, sizeof(FILEVARS));
        g_Tabs[i].iEncoding = Settings.DefaultEncoding;
        g_Tabs[i].iEOLMode = Settings.DefaultEOLMode;
    }

    g_pActiveTab = NULL;
    g_nTabCount = 0;
}

void TabManager_Cleanup(HWND hwndMain)
{
    UNREFERENCED_PARAMETER(hwndMain);

    for (int i = 0; i < g_nTabCount; i++) {
        if (g_Tabs[i].pDocPointer && Globals.hwndEdit) {
            if (g_pActiveTab != &g_Tabs[i]) {
                SendMessage(Globals.hwndEdit, SCI_SETDOCPOINTER, 0, (LPARAM)g_Tabs[i].pDocPointer);
            }
            SciCall_ReleaseDocument(g_Tabs[i].pDocPointer);
        }
        if (g_Tabs[i].hFilePath) {
            Path_Release(g_Tabs[i].hFilePath);
        }
    }

    g_nTabCount = 0;
    g_pActiveTab = NULL;

    if (g_hwndTabControl) {
        DestroyWindow(g_hwndTabControl);
        g_hwndTabControl = NULL;
    }
}

void TabManager_RegisterInitialTab(HWND hwndMain)
{
    UNREFERENCED_PARAMETER(hwndMain);

    if (!Globals.hwndEdit || !g_hwndTabControl) return;

    sptr_t pDoc = (sptr_t)SendMessage(Globals.hwndEdit, SCI_GETDOCPOINTER, 0, 0);
    if (!pDoc) return;

    int nIndex = g_nTabCount;
    TabDoc* pTab = &g_Tabs[nIndex];

    pTab->pDocPointer = pDoc;
    pTab->bModified = false;
    pTab->bReadOnly = false;
    pTab->fvFileVars = Globals.fvCurFile;
    pTab->iEncoding = Encoding_Current(-1);
    pTab->iEOLMode = Globals.fvCurFile.chMode[0] ? Globals.fvCurFile.chMode[0] : Settings.DefaultEOLMode;

    if (Path_IsNotEmpty(Paths.CurrentFile)) {
        pTab->hFilePath = Path_Copy(Paths.CurrentFile);
    } else {
        pTab->hFilePath = NULL;
    }

    _BuildTabText(pTab, nIndex + 1);

    TCITEM tci = { 0 };
    tci.mask = TCIF_TEXT | TCIF_PARAM;
    tci.pszText = pTab->wchTabText;
    tci.cchTextMax = (int)COUNTOF(pTab->wchTabText);
    tci.lParam = (LPARAM)pTab;
    TabCtrl_InsertItem(g_hwndTabControl, nIndex, &tci);

    g_nTabCount++;
    g_pActiveTab = pTab;

    ShowWindow(g_hwndTabControl, SW_SHOW);
    _RelayoutMainWindow(hwndMain);
}

//=============================================================================
// Tab Operations
//=============================================================================

TabDoc* TabManager_AddNewTab(HWND hwndMain)
{
    if (!Globals.hwndEdit || !g_hwndTabControl || g_nTabCount >= MAX_TABS) {
        return NULL;
    }

    sptr_t pNewDoc = SciCall_CreateDocument(0, SC_DOCUMENTOPTION_DEFAULT);
    if (!pNewDoc) return NULL;

    int nIndex = g_nTabCount;
    TabDoc* pTab = &g_Tabs[nIndex];

    pTab->pDocPointer = pNewDoc;
    pTab->bModified = false;
    pTab->bReadOnly = false;
    pTab->fvFileVars = Globals.fvCurFile;
    pTab->hFilePath = NULL;
    pTab->iEncoding = Settings.DefaultEncoding;
    pTab->iEOLMode = Settings.DefaultEOLMode;

    _BuildTabText(pTab, nIndex + 1);

    TCITEM tci = { 0 };
    tci.mask = TCIF_TEXT | TCIF_PARAM;
    tci.pszText = pTab->wchTabText;
    tci.cchTextMax = (int)COUNTOF(pTab->wchTabText);
    tci.lParam = (LPARAM)pTab;
    TabCtrl_InsertItem(g_hwndTabControl, nIndex, &tci);

    g_nTabCount++;

    TabManager_SwitchToTab(hwndMain, nIndex);

    ShowWindow(g_hwndTabControl, SW_SHOW);
    _RelayoutMainWindow(hwndMain);

    return pTab;
}

bool TabManager_CloseTab(int nIndex)
{
    if (!g_hwndTabControl || nIndex < 0 || nIndex >= g_nTabCount) {
        return false;
    }

    TabDoc* pTab = &g_Tabs[nIndex];

    if (pTab->pDocPointer && Globals.hwndEdit) {
        SciCall_ReleaseDocument(pTab->pDocPointer);
    }

    if (pTab->hFilePath) {
        Path_Release(pTab->hFilePath);
    }

    TabCtrl_DeleteItem(g_hwndTabControl, nIndex);

    for (int i = nIndex; i < g_nTabCount - 1; i++) {
        g_Tabs[i] = g_Tabs[i + 1];
        TCITEM tci = { 0 };
        tci.mask = TCIF_PARAM;
        tci.lParam = (LPARAM)&g_Tabs[i];
        TabCtrl_SetItem(g_hwndTabControl, i, &tci);
    }

    TabDoc empty = { 0 };
    g_Tabs[g_nTabCount - 1] = empty;

    g_nTabCount--;

    if (g_nTabCount == 0) {
        // Detach from the released document: Scintilla creates a fresh one
        if (Globals.hwndEdit) {
            SendMessage(Globals.hwndEdit, SCI_SETDOCPOINTER, 0, 0);
        }
        ShowWindow(g_hwndTabControl, SW_HIDE);
        _RelayoutMainWindow(Globals.hwndMain);
        return true;
    } else {
        int nSwitchIndex = nIndex;
        if (nSwitchIndex >= g_nTabCount) {
            nSwitchIndex = g_nTabCount - 1;
        }
        TabManager_SwitchToTab(Globals.hwndMain, nSwitchIndex);
    }

    return true;
}

void TabManager_CloseAllOtherTabs(int nKeepIndex)
{
    if (nKeepIndex < 0 || nKeepIndex >= g_nTabCount) return;

    for (int i = g_nTabCount - 1; i >= 0; i--) {
        if (i != nKeepIndex) {
            TabManager_CloseTab(i);
        }
    }
}

void TabManager_CloseAllTabs(HWND hwndMain)
{
    while (g_nTabCount > 0) {
        TabManager_CloseTab(g_nTabCount - 1);
    }

    TabManager_Init(hwndMain, GetModuleHandle(NULL));
    TabManager_RegisterInitialTab(hwndMain);
}

void TabManager_SwitchToTab(HWND hwndMain, int nIndex)
{
    if (!Globals.hwndEdit || !g_hwndTabControl) return;
    if (nIndex < 0 || nIndex >= g_nTabCount) return;

    TabDoc* pNewTab = &g_Tabs[nIndex];

    _TraceStep("S1:switch-entry");
    SendMessage(Globals.hwndEdit, SCI_SETDOCPOINTER, 0, (LPARAM)pNewTab->pDocPointer);
    _TraceStep("S2:doc-set");

    g_pActiveTab = pNewTab;

    if (pNewTab->hFilePath) {
        Path_Reset(Paths.CurrentFile, Path_Get(pNewTab->hFilePath));
    } else {
        Path_Empty(Paths.CurrentFile, false);
    }
    _TraceStep("S3:path");

    Encoding_Current(pNewTab->iEncoding);
    Globals.fvCurFile = pNewTab->fvFileVars;
    _TraceStep("S4:encoding");

    TabCtrl_SetCurSel(g_hwndTabControl, nIndex);
    _TraceStep("S5:cursel");

    if (hwndMain) {
        UpdateTitlebar(hwndMain);
        _TraceStep("S6:title");
        UpdateStatusbar(true);
        _TraceStep("S7:status");
        UpdateSaveSettingsCmds();
        _TraceStep("S8:savecmds");
    }
    _TraceStep("S9:switch-done");
}

//=============================================================================
// Tab Info / Queries
//=============================================================================

int TabManager_GetActiveTabIndex(void)
{
    if (!g_hwndTabControl) return -1;
    return TabCtrl_GetCurSel(g_hwndTabControl);
}

TabDoc* TabManager_GetTab(int nIndex)
{
    if (nIndex < 0 || nIndex >= g_nTabCount) return NULL;
    return &g_Tabs[nIndex];
}

//=============================================================================
// UI Helpers
//=============================================================================

void TabManager_UpdateTabText(int nIndex)
{
    TabDoc* pTab = TabManager_GetTab(nIndex);
    if (!pTab) return;

    int nUntitledCount = 0;
    for (int i = 0; i <= nIndex; i++) {
        if (Path_IsEmpty(g_Tabs[i].hFilePath)) {
            nUntitledCount++;
        }
    }
    _BuildTabText(pTab, nUntitledCount);
    _RefreshTabControlItem(nIndex);
}

void TabManager_UpdateActiveTabText(void)
{
    int nActive = TabManager_GetActiveTabIndex();
    if (nActive >= 0) {
        TabManager_UpdateTabText(nActive);
    }
}

LRESULT TabManager_HandleNotify(HWND hwndMain, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);

    LPNMHDR pnmh = (LPNMHDR)lParam;
    if (!pnmh) return 0;

    if ((UINT_PTR)(INT_PTR)pnmh->idFrom != (UINT_PTR)(INT_PTR)IDC_TAB) {
        return 0;
    }

    switch (pnmh->code) {
    case TCN_SELCHANGING:
        return FALSE;

    case TCN_SELCHANGE: {
        _TraceStep("N1:selchange");
        int nIndex = TabCtrl_GetCurSel(g_hwndTabControl);
        if (nIndex >= 0 && nIndex < g_nTabCount) {
            if (g_pActiveTab) {
                g_pActiveTab->fvFileVars = Globals.fvCurFile;
                g_pActiveTab->iEncoding = Encoding_Current(-1);
            }
            TabManager_SwitchToTab(hwndMain, nIndex);
        }
        _TraceStep("N2:selchange-done");
        return 0;
    }
    }

    return 0;
}
