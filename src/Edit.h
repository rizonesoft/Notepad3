/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Edit.h                                                                      *
*   Text File Editing Helper Stuff                                            *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2016   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#pragma once
#ifndef _NP3_EDIT_H_
#define _NP3_EDIT_H_

#include "Scintilla.h"
#include "TypeDefs.h"

// extern "C" declarations of Scintilla functions
int Scintilla_RegisterClasses(void*);
int Scintilla_ReleaseResources();

void  EditInitializeSciCtrl(HWND);
void  EditInitWordDelimiter(HWND);
void  EditSetNewText(HWND,char*,DWORD);
bool  EditConvertText(HWND,int,int,bool);
bool  EditSetNewEncoding(HWND,int,bool,bool);
bool  EditIsRecodingNeeded(WCHAR*,int);
char* EditGetClipboardText(HWND,bool,int*,int*);
bool  EditSetClipboardText(HWND, const char*, const size_t);
bool  EditClearClipboard(HWND);
bool  EditSwapClipboard(HWND,bool);
bool  EditCopyAppend(HWND,bool);
int   EditDetectEOLMode(HWND,char*);
bool  EditLoadFile(HWND,LPWSTR,bool,bool,int*,int*,bool*,bool*,bool*);
bool  EditSaveFile(HWND,LPCWSTR,int,bool*,bool);

void  EditInvertCase(HWND);
void  EditTitleCase(HWND);
void  EditSentenceCase(HWND);

void  EditURLEncode(HWND);
void  EditURLDecode(HWND);
void  EditEscapeCChars(HWND);
void  EditUnescapeCChars(HWND);
void  EditChar2Hex(HWND);
void  EditHex2Char(HWND);
void  EditFindMatchingBrace(HWND);
void  EditSelectToMatchingBrace(HWND);
void  EditModifyNumber(HWND,bool);

void  EditTabsToSpaces(HWND,int,bool);
void  EditSpacesToTabs(HWND,int,bool);

void  EditMoveUp(HWND);
void  EditMoveDown(HWND);
void  EditJumpToSelectionEnd(HWND);
void  EditJumpToSelectionStart(HWND);
void  EditModifyLines(HWND,LPCWSTR,LPCWSTR);
void  EditIndentBlock(HWND,int,bool);
void  EditAlignText(HWND,int);
void  EditEncloseSelection(HWND,LPCWSTR,LPCWSTR);
void  EditToggleLineComments(HWND,LPCWSTR,bool);
void  EditPadWithSpaces(HWND,bool,bool);
void  EditStripFirstCharacter(HWND);
void  EditStripLastCharacter(HWND,bool,bool);
void  EditCompressBlanks(HWND);
void  EditRemoveBlankLines(HWND,bool,bool);
void  EditRemoveDuplicateLines(HWND,bool);
void  EditWrapToColumn(HWND,DocPos);
void  EditSplitLines(HWND hwnd);
void  EditJoinLinesEx(HWND,bool,bool);
void  EditSortLines(HWND,int);

void  EditJumpTo(HWND, DocLn, DocPos);
void  EditScrollTo(HWND, DocLn, int);
void  EditSetSelectionEx(HWND, DocPos, DocPos, DocPos, DocPos);
void  EditFixPositions(HWND);
void  EditEnsureSelectionVisible(HWND);
void  EditGetExcerpt(HWND,LPWSTR,DWORD);

HWND  EditFindReplaceDlg(HWND,LPCEDITFINDREPLACE,bool);
bool  EditFindNext(HWND,LPCEDITFINDREPLACE,bool,bool);
bool  EditFindPrev(HWND,LPCEDITFINDREPLACE,bool,bool);
bool  EditReplace(HWND,LPCEDITFINDREPLACE);
int   EditReplaceAllInRange(HWND,LPCEDITFINDREPLACE,DocPos,DocPos,DocPos*);
bool  EditReplaceAll(HWND,LPCEDITFINDREPLACE,bool);
bool  EditReplaceAllInSelection(HWND,LPCEDITFINDREPLACE,bool);
bool  EditLinenumDlg(HWND);
bool  EditModifyLinesDlg(HWND,LPWSTR,LPWSTR);
bool  EditEncloseSelectionDlg(HWND,LPWSTR,LPWSTR);
bool  EditInsertTagDlg(HWND,LPWSTR,LPWSTR);
bool  EditSortDlg(HWND,int*);
bool  EditAlignDlg(HWND,int*);
bool  EditPrint(HWND,LPCWSTR,LPCWSTR);
void  EditPrintSetup(HWND);
void  EditPrintInit();
void  EditMatchBrace(HWND);
void  EditClearAllOccurrenceMarkers(HWND);
bool  EditToggleView(HWND hwnd, bool bToggleView);
void  EditMarkAll(HWND, char*, int, DocPos, DocPos, bool, bool);
void  EditUpdateUrlHotspots(HWND, DocPos, DocPos, bool);
void  EditSetAccelWordNav(HWND,bool);
bool  EditCompleteWord(HWND,bool);
void  EditGetBookmarkList(HWND,LPWSTR,int);
void  EditSetBookmarkList(HWND,LPCWSTR);
void  EditApplyLexerStyle(HWND, DocPos, DocPos);
void  EditFinalizeStyling(HWND, DocPos);

void  EditMarkAllOccurrences(HWND hwnd, bool bForceClear);
void  EditUpdateVisibleUrlHotspot(bool);
void  EditHideNotMarkedLineRange(HWND, DocPos, DocPos, bool);

//void SciInitThemes(HWND);
//LRESULT CALLBACK SciThemedWndProc(HWND,UINT,WPARAM,LPARAM);

#define FV_TABWIDTH        1
#define FV_INDENTWIDTH     2
#define FV_TABSASSPACES    4
#define FV_TABINDENTS      8
#define FV_WORDWRAP       16
#define FV_LONGLINESLIMIT 32
#define FV_ENCODING       64
#define FV_MODE          128

typedef struct _filevars {

  int mask;
  int iTabWidth;
  int iIndentWidth;
  bool bTabsAsSpaces;
  bool bTabIndents;
  bool fWordWrap;
  int iLongLinesLimit;
  char tchEncoding[32];
  int  iEncoding;
  char tchMode[32];

} FILEVARS, *LPFILEVARS;

bool FileVars_Init(char*,DWORD,LPFILEVARS);
bool FileVars_Apply(HWND,LPFILEVARS);
bool FileVars_ParseInt(char*,char*,int*);
bool FileVars_ParseStr(char*,char*,char*,int);
bool FileVars_IsUTF8(LPFILEVARS);
bool FileVars_IsNonUTF8(LPFILEVARS);
bool FileVars_IsValidEncoding(LPFILEVARS);
int  FileVars_GetEncoding(LPFILEVARS);


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

void EditToggleFolds(FOLD_ACTION,bool);
void EditFoldClick(DocLn, int);
void EditFoldAltArrow(FOLD_MOVE, FOLD_ACTION);

void EditShowZoomCallTip(HWND hwnd);
void EditShowZeroLengthCallTip(HWND hwnd, DocPos iPosition);

#define NP3_BRACES_TO_MATCH "()[]{}"

#endif //_NP3_EDIT_H_

///   End of Edit.h   \\\
