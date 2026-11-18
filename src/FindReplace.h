// encoding: UTF-8
/******************************************************************************
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* FindReplace.h                                                               *
*   Find/Replace and occurrence matching                                      *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2026   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*******************************************************************************/
#pragma once
#ifndef _NP3_FINDREPLACE_H_
#define _NP3_FINDREPLACE_H_

#include "TypeDefs.h"

typedef struct {
    RECT windowRect;
    bool bFocusedView;
    bool bReplace;
} FindReplaceDlgRestore;

void FindReplaceInit(void);
void FindReplaceRelease(void);
bool IsFindPatternEmpty(void);
LPCWSTR GetFindPattern(void);
void SetFindPattern(LPCWSTR wchFindPattern);
size_t LengthOfFindPattern(void);
void ReleaseEFR(LPEDITFINDREPLACE efr);

LPEDITFINDREPLACE FindReplacePrepare(void);
LPEDITFINDREPLACE FindReplaceUseSelection(void);
void FindReplaceShowDialog(bool bReplace);
bool FindReplaceCaptureDialog(FindReplaceDlgRestore* pRestore);
void FindReplaceRestoreDialog(const FindReplaceDlgRestore* pRestore);

bool EditFindNext(HWND hwnd, const LPEDITFINDREPLACE lpefr, bool bExtendSelection, bool bFocusWnd, bool bFillEmpty);
bool EditFindPrev(HWND hwnd, const LPEDITFINDREPLACE lpefr, bool bExtendSelection, bool bFocusWnd, bool bFillEmpty);
bool EditReplace(HWND hwnd, LPEDITFINDREPLACE lpefr);
bool EditReplaceAll(HWND hwnd, LPEDITFINDREPLACE lpefr, bool bShowInfo);
bool EditReplaceAllInSelection(HWND hwnd, LPEDITFINDREPLACE lpefr, bool bShowInfo);

// Background matching does not update the Find/Replace status.
DocPos EditFindInTarget(LPCWSTR wchFind, int sFlags, DocPos* begin, DocPos* end, bool bForceNext);

void EditMarkAllOccurrences(HWND hwnd, bool bForceClear);
void EditClearAllOccurrenceMarkers(HWND hwnd);
void EditSelectionMultiSelectAll(void);
void EditSelectionMultiSelectAllEx(HWND hwnd, CLPCEDITFINDREPLACE edFndRpl);

#endif //_NP3_FINDREPLACE_H_
