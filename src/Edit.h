﻿/******************************************************************************
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

#include "TypeDefs.h"

// extern "C" declarations of Scintilla functions
BOOL Scintilla_RegisterClasses(void*);
BOOL Scintilla_ReleaseResources();

#define FNDRPL_BUFFER 512
typedef struct _editfindreplace
{
  char szFind[FNDRPL_BUFFER];
  char szReplace[FNDRPL_BUFFER];
  char szFindUTF8[3 * FNDRPL_BUFFER];
  char szReplaceUTF8[3 * FNDRPL_BUFFER];
  UINT fuFlags;
  BOOL bTransformBS;
  BOOL bObsolete /* was bFindUp */;
  BOOL bFindClose;
  BOOL bReplaceClose;
  BOOL bNoFindWrap;
  BOOL bWildcardSearch;
  BOOL bMarkOccurences;
  BOOL bDotMatchAll;
  HWND hwnd;

} EDITFINDREPLACE, *LPEDITFINDREPLACE, *LPCEDITFINDREPLACE;

#define EFR_INIT_DATA  { "", "", "", "", 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL }


#define IDMSG_SWITCHTOFIND    300
#define IDMSG_SWITCHTOREPLACE 301

#define MARKER_NP3_BOOKMARK      0

#define INDIC_NP3_MARK_OCCURANCE 1
#define INDIC_NP3_MATCH_BRACE    2
#define INDIC_NP3_BAD_BRACE      3

// [pixel] auto calculate by SCI_SETSCROLLWIDTHTRACKING
//#define DEFAULT_SCROLL_WIDTH (8*80)
#define DEFAULT_SCROLL_WIDTH ((8*g_WinInfo.cx)/10)

HWND  EditCreate(HWND);
void  EditInitWordDelimiter(HWND);
void  EditSetNewText(HWND,char*,DWORD);
BOOL  EditConvertText(HWND,int,int,BOOL);
BOOL  EditSetNewEncoding(HWND,int,BOOL,BOOL);
BOOL  EditIsRecodingNeeded(WCHAR*,int);
char* EditGetClipboardText(HWND,BOOL,int*,int*);
BOOL  EditSetClipboardText(HWND, const char*);
BOOL  EditClearClipboard(HWND);
void  EditPaste2RectSel(HWND,char*);
BOOL  EditPasteClipboard(HWND,BOOL,BOOL);
BOOL  EditCopyAppend(HWND,BOOL);
int   EditDetectEOLMode(HWND,char*,DWORD);
BOOL  EditLoadFile(HWND,LPCWSTR,BOOL,BOOL,int*,int*,BOOL*,BOOL*,BOOL*);
BOOL  EditSaveFile(HWND,LPCWSTR,int,BOOL*,BOOL);

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
void  EditModifyNumber(HWND,BOOL);

void  EditTabsToSpaces(HWND,int,BOOL);
void  EditSpacesToTabs(HWND,int,BOOL);

void  EditMoveUp(HWND);
void  EditMoveDown(HWND);
void  EditModifyLines(HWND,LPCWSTR,LPCWSTR);
void  EditIndentBlock(HWND,int,BOOL);
void  EditAlignText(HWND,int);
void  EditEncloseSelection(HWND,LPCWSTR,LPCWSTR);
void  EditToggleLineComments(HWND,LPCWSTR,BOOL);
void  EditPadWithSpaces(HWND,BOOL,BOOL);
void  EditStripFirstCharacter(HWND);
void  EditStripLastCharacter(HWND,BOOL,BOOL);
void  EditCompressSpaces(HWND);
void  EditRemoveBlankLines(HWND,BOOL);
void  EditWrapToColumn(HWND,int);
void  EditJoinLinesEx(HWND,BOOL,BOOL);
void  EditSortLines(HWND,int);

void  EditJumpTo(HWND, DocLn, DocPos);
void  EditSelectEx(HWND, DocPos, DocPos);
void  EditFixPositions(HWND);
void  EditEnsureSelectionVisible(HWND);
void  EditGetExcerpt(HWND,LPWSTR,DWORD);

HWND  EditFindReplaceDlg(HWND,LPCEDITFINDREPLACE,BOOL);
BOOL  EditFindNext(HWND,LPCEDITFINDREPLACE,BOOL,BOOL);
BOOL  EditFindPrev(HWND,LPCEDITFINDREPLACE,BOOL,BOOL);
BOOL  EditReplace(HWND,LPCEDITFINDREPLACE);
int   EditReplaceAllInRange(HWND,LPCEDITFINDREPLACE,BOOL, DocPos, DocPos);
BOOL  EditReplaceAll(HWND,LPCEDITFINDREPLACE,BOOL);
BOOL  EditReplaceAllInSelection(HWND,LPCEDITFINDREPLACE,BOOL);
BOOL  EditLinenumDlg(HWND);
BOOL  EditModifyLinesDlg(HWND,LPWSTR,LPWSTR);
BOOL  EditEncloseSelectionDlg(HWND,LPWSTR,LPWSTR);
BOOL  EditInsertTagDlg(HWND,LPWSTR,LPWSTR);
BOOL  EditSortDlg(HWND,int*);
BOOL  EditAlignDlg(HWND,int*);
BOOL  EditPrint(HWND,LPCWSTR,LPCWSTR);
void  EditPrintSetup(HWND);
void  EditPrintInit();
void  EditMatchBrace(HWND);
void  EditClearAllMarks(HWND, DocPos, DocPos);
void  EditMarkAll(HWND, char*, int, DocPos, DocPos, BOOL, BOOL);
void  EditUpdateUrlHotspots(HWND, DocPos, DocPos, BOOL);
void  EditSetAccelWordNav(HWND,BOOL);
void  EditCompleteWord(HWND,BOOL);
void  EditGetBookmarkList(HWND,LPWSTR,int);
void  EditSetBookmarkList(HWND,LPCWSTR);
void  EditApplyLexerStyle(HWND, DocPos, DocPos);
void  EditFinalizeStyling(HWND, DocPos);

void  EditMarkAllOccurrences();
void  EditUpdateVisibleUrlHotspot(BOOL);

void  EditEnterTargetTransaction();
void  EditLeaveTargetTransaction();
BOOL  EditIsInTargetTransaction();

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
  BOOL bTabsAsSpaces;
  BOOL bTabIndents;
  BOOL fWordWrap;
  int iLongLinesLimit;
  char tchEncoding[32];
  int  iEncoding;
  char tchMode[32];

} FILEVARS, *LPFILEVARS;

BOOL FileVars_Init(char*,DWORD,LPFILEVARS);
BOOL FileVars_Apply(HWND,LPFILEVARS);
BOOL FileVars_ParseInt(char*,char*,int*);
BOOL FileVars_ParseStr(char*,char*,char*,int);
BOOL FileVars_IsUTF8(LPFILEVARS);
BOOL FileVars_IsNonUTF8(LPFILEVARS);
BOOL FileVars_IsValidEncoding(LPFILEVARS);
int  FileVars_GetEncoding(LPFILEVARS);


//
//  Folding Functions
//
typedef enum {
  EXPAND = 1,
  SNIFF = 0,
  FOLD = -1
} FOLD_ACTION;

typedef enum {
  UP = -1,
  NONE = 0,
  DOWN = 1
} FOLD_MOVE;

void EditFoldToggleAll(FOLD_ACTION);
void EditFoldClick(DocLn, int);
void EditFoldAltArrow(FOLD_MOVE, FOLD_ACTION);


#endif //_NP3_EDIT_H_

///   End of Edit.h   \\\
