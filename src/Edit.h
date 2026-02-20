// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Edit.h                                                                      *
*   Text File Editing Helper Stuff                                            *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2026   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#pragma once
#ifndef _NP3_EDIT_H_
#define _NP3_EDIT_H_

#include "Scintilla.h"
#include "SciCall.h"
#include "TypeDefs.h"

void DuplicateEFR(LPEDITFINDREPLACE dst, CLPCEDITFINDREPLACE src);
void ReleaseEFR(LPEDITFINDREPLACE efr);

void EditReplaceSelection(const char* text, bool bForceSel);
void EditInitWordDelimiter(HWND hwnd);
void EditSetNewText(HWND hwnd, const char* lpstrText, DocPosU lenText, bool bClearUndoHistory, bool bReload);
bool EditConvertText(HWND hwnd, cpi_enc_t encSource, cpi_enc_t encDest);
bool EditSetNewEncoding(HWND hwnd, cpi_enc_t iNewEncoding,bool bSupressWarning);
bool EditIsRecodingNeeded(WCHAR* pszText,int cchLen);
size_t EditGetSelectedText(LPWSTR pwchBuffer, size_t wchCount);
char* EditGetClipboardText(HWND hwnd,bool,int* pLineCount,int* pLenLastLn);
void EditGetClipboardW(LPWSTR pwchBuffer, size_t wchLength);
bool EditSetClipboardText(HWND hwnd, const char* pszText, size_t cchText);
bool EditAppendToClipboard(HWND hwnd, const char* pszText, size_t cchText);
bool EditClearClipboard(HWND hwnd);
bool EditSwapClipboard(HWND hwnd,bool);
bool EditCopyRangeAppend(HWND hwnd, DocPos posBegin, DocPos posEnd, bool bAppend);
void EditDetectEOLMode(LPCSTR lpData, size_t cbData, EditFileIOStatus* const status);
void EditIndentationStatistic(HWND hwnd, EditFileIOStatus* const status);
bool EditLoadFile(HWND hwnd, const HPATHL hfile_pth, EditFileIOStatus* const status, FileLoadFlags fLoadFlags, bool bClearUndoHistory);
bool EditSaveFile(HWND hwnd, const HPATHL hfile_pth, EditFileIOStatus* status, FileSaveFlags fSaveFlags, bool bPreserveTimeStamp);

void EditInvertCase(HWND hwnd);
void EditTitleCase(HWND hwnd);
void EditSentenceCase(HWND hwnd);

void EditURLEncode(const bool isPathConvert);
void EditURLDecode(const bool isPathConvert);
void EditReplaceAllChr(const WCHAR chSearch, const WCHAR chReplace);
void EditBase64Code(HWND hwnd, const bool bEncode, cpi_enc_t cpi);
void EditEscapeCChars(HWND hwnd);
void EditUnescapeCChars(HWND hwnd);
void EditChar2Hex(HWND hwnd);
void EditHex2Char(HWND hwnd);
void EditFindMatchingBrace();
void EditSelectToMatchingBrace();
void EditModifyNumber(HWND hwnd, bool bIncrease);
void EditInsertDateTimeStrg(bool bShortFmt, bool bTimestampFmt);
void EditUpdateTimestamps();

void EditTabsToSpaces(int nTabWidth, bool bOnlyIndentingWS);
void EditSpacesToTabs(int nTabWidth, bool bOnlyIndentingWS);

void EditMoveUp(HWND hwnd);
void EditMoveDown(HWND hwnd);
bool EditSetCaretToSelectionStart();
bool EditSetCaretToSelectionEnd();
void EditCutLines(HWND hwnd, const bool bMSBehavSelEmpty);
void EditCopyMultiSelection(HWND hwnd);
void EditModifyLines(const PENCLOSESELDATA pEnclData);
void EditIndentBlock(HWND hwnd,int cmd, bool bFormatIndentation, bool bForceAll);
void EditAlignText(int nMode);
void EditEncloseSelection(LPCWSTR pszOpen, LPCWSTR pszClose);
typedef enum _lncmntmode { LNC_TOGGLE, LNC_ADD, LNC_REMOVE } LnCmtMode_t;
void EditToggleLineCommentsSimple(LPCWSTR pwszComment, bool bInsertAtStart, LnCmtMode_t mode);
void EditToggleLineCommentsExtended(LPCWSTR pwszComment, bool bInsertAtStart);
void EditPadWithSpaces(HWND hwnd, bool bSkipEmpty);
void EditStripFirstCharacter(HWND hwnd);
void EditStripLastCharacter(HWND hwnd, bool bIgnoreSelection, bool bTrailingBlanksOnly);
bool EditHasTrailingBlanks();
void EditCompressBlanks();
void EditRemoveBlankLines(HWND hwnd, bool bMerge, bool bRemoveWhiteSpace);
void EditUniteDuplicateLines(HWND hwnd, bool bRemoveEmptyLines, bool bRemoveLastDup);
void EditFocusMarkedLinesCmd(HWND hwnd, bool bCopy, bool bDelete);
void EditWrapToColumn(HWND hwnd, DocPosU nColumn);
void EditWrapToColumnEx(HWND hwnd, DocPosU nColumn);

void EditSplitLines(HWND hwnd);
void EditJoinLinesEx(bool,bool);
void EditSortLines(HWND hwnd,int iSortFlags);

void EditJumpTo(DocLn iNewLine, DocPos iNewCol);
void EditSetSelectionEx(DocPos iAnchorPos, DocPos iCurrentPos, DocPos vSpcAnchor, DocPos vSpcCurrent);
void EditFixPositions();
void EditEnsureConsistentLineEndings(HWND hwnd);
void EditGetExcerpt(HWND hwnd,LPWSTR lpszExcerpt,DWORD cchExcerpt);

HWND EditFindReplaceDlg(HWND hwnd, LPEDITFINDREPLACE lpefr, bool);
bool EditFindNext(HWND hwnd, const LPEDITFINDREPLACE lpefr, bool bExtendSelection, bool bFocusWnd, bool bFillEmpty);
bool EditFindPrev(HWND hwnd, const LPEDITFINDREPLACE lpefr, bool bExtendSelection, bool bFocusWnd, bool bFillEmpty);
bool EditReplace(HWND hwnd, LPEDITFINDREPLACE lpefr);
DocPosU EditReplaceAllInRange(HWND hwnd, LPEDITFINDREPLACE lpefr, DocPos iStartPos, DocPos iEndPos, DocPos *enlargement);
bool EditReplaceAll(HWND hwnd, LPEDITFINDREPLACE lpefr, bool);
bool EditReplaceAllInSelection(HWND hwnd, LPEDITFINDREPLACE lpefr, bool);
bool EditLinenumDlg(HWND hwnd);
bool EditModifyLinesDlg(HWND hwnd, PENCLOSESELDATA pEnclData);
bool EditEncloseSelectionDlg(HWND hwnd, PENCLOSESELDATA pEnclData);
bool EditInsertTagDlg(HWND hwnd, PENCLOSESELDATA pEnclData, UINT *pRepeat);
bool EditSortDlg(HWND hwnd,int* piSortFlags);
bool EditAlignDlg(HWND hwnd,int* piAlignMode);
bool EditPrint(HWND,LPCWSTR,LPCWSTR);
void EditPrintSetup(HWND hwnd);
void EditPrintInit();
bool EditSetDocumentBuffer(const char* lpstrText, DocPosU lenText, bool reload);
void EditMatchBrace(HWND hwnd);
void EditClearAllOccurrenceMarkers(HWND hwnd);
void EditClearAllBookMarks(HWND hwnd);
void EditToggleView(HWND hwnd);
void EditSelectWordAtPos(const DocPos iPos, const bool bForceWord);
int  EditAddSearchFlags(int flags, bool bRegEx, bool bWordStart, bool bMatchCase, bool bMatchWords, bool bDotMatchAll);
void EditMarkAll(LPCWSTR wchFind, int sFlags, DocPos rangeStart, DocPos rangeEnd, bool bMultiSel);
void EditUpdateVisibleIndicators();
void EditUpdateIndicators(DocPos startPos, DocPos endPos, bool bClearOnly);
void EditReducedWordSeparatorSet(HWND hwnd, bool);
bool EditAutoCompleteWord(HWND hwnd, bool autoInsert);
bool EditCheckNewLineInACFillUps();
void EditGetBookmarkList(HWND hwnd,LPWSTR pszBookMarks,int cchLength);
void EditSetBookmarkList(HWND hwnd,LPCWSTR pszBookMarks);
void EditBookmarkNext(HWND hwnd, const DocLn iLine);
void EditBookmarkPrevious(HWND hwnd, const DocLn iLine);
void EditBookmarkToggle(HWND hwnd, const DocLn ln, const int modifiers);
void EditMarkAllOccurrences(HWND hwnd, bool bForceClear);
void EditFoldMarkedLineRange(HWND hwnd, bool bHideLines);
void EditBookMarkLineRange(HWND hwnd);
void EditDeleteMarkerInSelection();
void EditSelectionMultiSelectAll();
void EditSelectionMultiSelectAllEx(HWND hwnd, CLPCEDITFINDREPLACE edFndRpl);

//
//  Folding Functions
//
typedef enum {
    FOLD = SC_FOLDACTION_CONTRACT,
    EXPAND = SC_FOLDACTION_EXPAND,
    SNIFF = SC_FOLDACTION_TOGGLE

} FOLD_ACTION;

typedef enum {
    UP = -1,
    NONE = 0,
    DOWN = 1

} FOLD_MOVE;

void EditToggleFolds(FOLD_ACTION action,bool);
void EditFoldClick(DocLn ln, int mode);
void EditFoldCmdKey(FOLD_MOVE move, FOLD_ACTION action);


#define NP3_BRACES_TO_MATCH "()[]{}"

inline int GetMarkAllOccSearchFlags()
{
    return EditAddSearchFlags(0, false, false, Settings.MarkOccurrencesMatchCase,
                              Settings.MarkOccurrencesCurrentWord || Settings.MarkOccurrencesMatchWholeWords, false);
}

#endif //_NP3_EDIT_H_

///   End of Edit.h   ///
