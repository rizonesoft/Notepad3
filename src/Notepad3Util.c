// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Notepad3Util.c                                                              *
*   Utility functions extracted from Notepad3.c                               *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2026   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

#include "Helpers.h"

#include <commctrl.h>
#include <shlobj.h>
#include <shlwapi.h>

#include "PathLib.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "MuiLanguage.h"
#include "Notepad3.h"
#include "Notepad3Util.h"


// ============================================================================
// --- Bitmap / Image Loading ---
// ============================================================================

//=============================================================================
//
//  NP3Util_LoadBitmapFile()
//
HBITMAP NP3Util_LoadBitmapFile(const HPATHL hpath)
{
    HBITMAP hbmp = NULL;

    if (Path_IsExistingFile(hpath)) {

        hbmp = (HBITMAP)LoadImage(NULL, Path_Get(hpath), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);

        bool bDimOK = false;
        int  height = 16;
        if (hbmp) {
            BITMAP bmp = { 0 };
            GetObject(hbmp, sizeof(BITMAP), &bmp);
            height = bmp.bmHeight;
            bDimOK = (bmp.bmWidth >= (height * NUMTOOLBITMAPS));
        }
        if (!bDimOK) {
            InfoBoxLng(MB_ICONWARNING, Constants.SuppressKey.NotSuitableToolbarDim, IDS_MUI_ERR_BITMAP, Path_Get(hpath),
                (height * NUMTOOLBITMAPS), height, NUMTOOLBITMAPS);
        }
    }
    else {
        WCHAR displayName[80];
        Path_GetDisplayName(displayName, 80, hpath, L"<unknown>", false);
        InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_ERR_LOADFILE, displayName);
    }

    return hbmp;
}


//=============================================================================
//
//  NP3Util_XXX_CreateScaledImageListFromBitmap()
//
HIMAGELIST NP3Util_XXX_CreateScaledImageListFromBitmap(HWND hWnd, HBITMAP hBmp)
{
    BITMAP bmp = { 0 };
    GetObject(hBmp, sizeof(BITMAP), &bmp);

    int const mod = bmp.bmWidth % NUMTOOLBITMAPS;
    int const cx = (bmp.bmWidth - mod) / NUMTOOLBITMAPS;
    int const cy = bmp.bmHeight;

    HIMAGELIST himl = ImageList_Create(cx, cy, ILC_COLOR32 | ILC_MASK, NUMTOOLBITMAPS, NUMTOOLBITMAPS);
    ImageList_AddMasked(himl, hBmp, CLR_DEFAULT);

    UINT const dpi = Scintilla_GetWindowDPI(hWnd);
    if (!Settings.DpiScaleToolBar || (dpi == USER_DEFAULT_SCREEN_DPI)) {
        return himl; // default DPI, we are done
    }

    // Scale button icons/images
    int const scx = ScaleIntToDPI(hWnd, cx);
    int const scy = ScaleIntToDPI(hWnd, cy);

    HIMAGELIST hsciml = ImageList_Create(scx, scy, ILC_COLOR32 | ILC_MASK | ILC_HIGHQUALITYSCALE, NUMTOOLBITMAPS, NUMTOOLBITMAPS);

    for (int i = 0; i < NUMTOOLBITMAPS; ++i) {
        HICON const hicon = ImageList_GetIcon(himl, i, ILD_TRANSPARENT | ILD_PRESERVEALPHA | ILD_SCALE);
        ImageList_AddIcon(hsciml, hicon);
        DestroyIcon(hicon);
    }

    ImageList_Destroy(himl);

    return hsciml;
}


//=============================================================================
//
//  NP3Util_CreateScaledImageListFromBitmap()
//
HIMAGELIST NP3Util_CreateScaledImageListFromBitmap(HWND hWnd, HBITMAP hBmp)
{
    BITMAP bmp = { 0 };
    GetObject(hBmp, sizeof(BITMAP), &bmp);

    int const numOfToolBitmaps = (int)(bmp.bmWidth / bmp.bmHeight);

    int const mod = bmp.bmWidth % numOfToolBitmaps;
    int const cx = (bmp.bmWidth - mod) / numOfToolBitmaps;
    int const cy = bmp.bmHeight;

    HIMAGELIST himl = ImageList_Create(cx, cy, ILC_COLOR32 | ILC_MASK, numOfToolBitmaps, numOfToolBitmaps);
    ImageList_AddMasked(himl, hBmp, CLR_DEFAULT);

    UINT const dpi = Scintilla_GetWindowDPI(hWnd);
    if (!Settings.DpiScaleToolBar || (dpi == USER_DEFAULT_SCREEN_DPI)) {
        return himl; // default DPI, we are done
    }

    // Scale button icons/images
    int const scx = ScaleIntToDPI(hWnd, cx);
    int const scy = ScaleIntToDPI(hWnd, cy);

    HIMAGELIST hsciml = ImageList_Create(scx, scy, ILC_COLOR32 | ILC_MASK | ILC_HIGHQUALITYSCALE, numOfToolBitmaps, numOfToolBitmaps);

    for (int i = 0; i < numOfToolBitmaps; ++i) {
        HICON const hicon = ImageList_GetIcon(himl, i, ILD_TRANSPARENT | ILD_PRESERVEALPHA | ILD_SCALE);
        ImageList_AddIcon(hsciml, hicon);
        DestroyIcon(hicon);
    }

    ImageList_Destroy(himl);

    return hsciml;
}


// ============================================================================
// --- Word-Wrap Configuration ---
// ============================================================================

//=============================================================================
//
//  NP3Util_SetWrapStartIndent()
//
void NP3Util_SetWrapStartIndent(void)
{
    int i = 0;
    switch (Settings.WordWrapIndent) {
    case 1:
        i = 1;
        break;
    case 2:
        i = 2;
        break;
    case 3:
        i = (Globals.fvCurFile.iIndentWidth) ? 1 * Globals.fvCurFile.iIndentWidth : 1 * Globals.fvCurFile.iTabWidth;
        break;
    case 4:
        i = (Globals.fvCurFile.iIndentWidth) ? 2 * Globals.fvCurFile.iIndentWidth : 2 * Globals.fvCurFile.iTabWidth;
        break;
    default:
        break;
    }
    SciCall_SetWrapStartIndent(i);
}


//=============================================================================
//
//  NP3Util_SetWrapIndentMode()
//
void NP3Util_SetWrapIndentMode(void)
{
    BeginWaitCursorUID(Flags.bHugeFileLoadState, IDS_MUI_SB_WRAP_LINES);

    Sci_SetWrapModeEx(GET_WRAP_MODE());

    if (Settings.WordWrapIndent == 5) {
        SciCall_SetWrapIndentMode(SC_WRAPINDENT_SAME);
    } else if (Settings.WordWrapIndent == 6) {
        SciCall_SetWrapIndentMode(SC_WRAPINDENT_INDENT);
    } else if (Settings.WordWrapIndent == 7) {
        SciCall_SetWrapIndentMode(SC_WRAPINDENT_DEEPINDENT);
    } else {
        NP3Util_SetWrapStartIndent();
        SciCall_SetWrapIndentMode(SC_WRAPINDENT_FIXED);
    }

    EndWaitCursor();
}


//=============================================================================
//
//  NP3Util_SetWrapVisualFlags()
//
void NP3Util_SetWrapVisualFlags(HWND hwndEditCtrl)
{
    UNREFERENCED_PARAMETER(hwndEditCtrl);

    if (Settings.ShowWordWrapSymbols) {
        int wrapVisualFlags = 0;
        int wrapVisualFlagsLocation = 0;
        if (Settings.WordWrapSymbols == 0) {
            Settings.WordWrapSymbols = 22;
        }
        switch (Settings.WordWrapSymbols % 10) {
        case 1:
            wrapVisualFlags |= SC_WRAPVISUALFLAG_END;
            wrapVisualFlagsLocation |= SC_WRAPVISUALFLAGLOC_END_BY_TEXT;
            break;
        case 2:
            wrapVisualFlags |= SC_WRAPVISUALFLAG_END;
            break;
        }
        switch (((Settings.WordWrapSymbols % 100) - (Settings.WordWrapSymbols % 10)) / 10) {
        case 1:
            wrapVisualFlags |= SC_WRAPVISUALFLAG_START;
            wrapVisualFlagsLocation |= SC_WRAPVISUALFLAGLOC_START_BY_TEXT;
            break;
        case 2:
            wrapVisualFlags |= SC_WRAPVISUALFLAG_START;
            break;
        }
        SciCall_SetWrapVisualFlags(wrapVisualFlags);
        SciCall_SetWrapVisualFlagsLocation(wrapVisualFlagsLocation);
    } else {
        SciCall_SetWrapVisualFlags(0);
    }
}


// ============================================================================
// --- Auto-Scroll (middle-click continuous scroll, Firefox-style) ---
// ============================================================================

static bool      s_bAutoScrollMode = false;
static bool      s_bAutoScrollHeld = false;
static ULONGLONG s_dwAutoScrollStartTick = 0;
static POINT     s_ptAutoScrollOrigin = { 0, 0 };
static POINT     s_ptAutoScrollMouse = { 0, 0 };
static double    s_dAutoScrollAccumY = 0.0;

bool NP3Util_IsAutoScrollMode(void)
{
    return s_bAutoScrollMode;
}

bool NP3Util_IsAutoScrollHeld(void)
{
    return s_bAutoScrollHeld;
}

ULONGLONG NP3Util_GetAutoScrollStartTick(void)
{
    return s_dwAutoScrollStartTick;
}

void NP3Util_SetAutoScrollHeld(bool held)
{
    s_bAutoScrollHeld = held;
}

void NP3Util_AutoScrollUpdateMouse(POINT pt)
{
    s_ptAutoScrollMouse = pt;
}


//=============================================================================
//
//  NP3Util_AutoScrollStop()
//
void NP3Util_AutoScrollStop(HWND hwndEdit)
{
    if (s_bAutoScrollMode) {
        KillTimer(hwndEdit, ID_AUTOSCROLLTIMER);
        ReleaseCapture();
        SciCall_SetCursor(SC_CURSORNORMAL);
        s_bAutoScrollMode = false;
        s_bAutoScrollHeld = false;
        s_dAutoScrollAccumY = 0.0;
    }
}


//=============================================================================
//
//  NP3Util_AutoScrollTimerProc()
//
void CALLBACK NP3Util_AutoScrollTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(idEvent);
    UNREFERENCED_PARAMETER(dwTime);

    if (!s_bAutoScrollMode) {
        KillTimer(hwnd, ID_AUTOSCROLLTIMER);
        return;
    }

    int const deltaY = s_ptAutoScrollMouse.y - s_ptAutoScrollOrigin.y;

    if (abs(deltaY) <= AUTOSCROLL_DEADZONE) {
        s_dAutoScrollAccumY = 0.0;
        return;
    }

    // Speed: proportional to distance beyond dead zone
    double const speed = (double)(deltaY - (deltaY > 0 ? AUTOSCROLL_DEADZONE : -AUTOSCROLL_DEADZONE)) / AUTOSCROLL_DIVISOR;
    s_dAutoScrollAccumY += speed;

    DocLn const linesToScroll = (DocLn)s_dAutoScrollAccumY;
    if (linesToScroll != 0) {
        SciCall_LineScroll(0, linesToScroll);
        s_dAutoScrollAccumY -= (double)linesToScroll;
    }
}


//=============================================================================
//
//  NP3Util_AutoScrollStart()
//
void NP3Util_AutoScrollStart(HWND hwndEdit, POINT pt)
{
    s_bAutoScrollMode = true;
    s_bAutoScrollHeld = false;
    s_dwAutoScrollStartTick = GetTickCount64();
    s_ptAutoScrollOrigin = pt;
    s_ptAutoScrollMouse = pt;
    s_dAutoScrollAccumY = 0.0;
    SetCapture(hwndEdit);
    SetCursor(LoadCursor(NULL, IDC_SIZEALL));
    SetTimer(hwndEdit, ID_AUTOSCROLLTIMER, AUTOSCROLL_TIMER_MS, NP3Util_AutoScrollTimerProc);
}
