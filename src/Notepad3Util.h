// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Notepad3Util.h                                                              *
*   Utility functions extracted from Notepad3.c                               *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2026   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#pragma once
#ifndef _NP3_NOTEPAD3UTIL_H_
#define _NP3_NOTEPAD3UTIL_H_

#include "TypeDefs.h"
#include "SciCall.h"

// --- Bitmap / Image Loading ---
#define NUMTOOLBITMAPS (31)
HBITMAP    NP3Util_LoadBitmapFile(const HPATHL hpath);
HIMAGELIST NP3Util_CreateScaledImageListFromBitmap(HWND hWnd, HBITMAP hBmp);
HIMAGELIST NP3Util_XXX_CreateScaledImageListFromBitmap(HWND hWnd, HBITMAP hBmp);

// --- Word-Wrap Configuration ---
void NP3Util_SetWrapStartIndent(void);
void NP3Util_SetWrapIndentMode(void);
void NP3Util_SetWrapVisualFlags(HWND hwndEditCtrl);

// --- Auto-Scroll (middle-click continuous scroll) ---
#define AUTOSCROLL_TIMER_MS             30
#define AUTOSCROLL_DEADZONE             15
#define AUTOSCROLL_DIVISOR              60.0
#define AUTOSCROLL_CLICK_THRESHOLD_MS  200

bool NP3Util_IsAutoScrollMode(void);
void NP3Util_AutoScrollStop(HWND hwndEdit);
void NP3Util_AutoScrollStart(HWND hwndEdit, POINT pt);
void NP3Util_AutoScrollUpdateMouse(POINT pt);
bool NP3Util_IsAutoScrollHeld(void);
ULONGLONG NP3Util_GetAutoScrollStartTick(void);
void NP3Util_SetAutoScrollHeld(bool held);

void CALLBACK NP3Util_AutoScrollTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

#endif // _NP3_NOTEPAD3UTIL_H_
