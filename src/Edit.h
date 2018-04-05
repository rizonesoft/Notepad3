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

#include "TypeDefs.h"

// extern "C" declarations of Scintilla functions
int Scintilla_RegisterClasses(void*);
int Scintilla_ReleaseResources();

typedef struct _editfindreplace
{
  char szFind[FNDRPL_BUFFER];
  char szReplace[FNDRPL_BUFFER];
  UINT fuFlags;
  bool bTransformBS;
  bool bFindClose;
  bool bReplaceClose;
  bool bNoFindWrap;
  bool bWildcardSearch;
  bool bMarkOccurences;
  bool bHideNonMatchedLines;
  bool bDotMatchAll;
  HWND hwnd;

} EDITFINDREPLACE, *LPEDITFINDREPLACE, *LPCEDITFINDREPLACE;

#define EFR_INIT_DATA  { "", "", /* "",  "", */ 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL }


#define IDMSG_SWITCHTOFIND    300
#define IDMSG_SWITCHTOREPLACE 301

#define MARKER_NP3_BOOKMARK      1
#define MARKER_NP3_OCCUR_LINE    2

#define INDIC_NP3_MARK_OCCURANCE 1
#define INDIC_NP3_MATCH_BRACE    2
#define INDIC_NP3_BAD_BRACE      3

void  EditInitWordDelimiter(HWND);
void  EditSetNewText(HWND,char*,DWORD);
bool  EditConvertText(HWND,int,int,bool);
bool  EditSetNewEncoding(HWND,int,bool,bool);
bool  EditIsRecodingNeeded(WCHAR*,int);
char* EditGetClipboardText(HWND,bool,int*,int*);
bool  EditSetClipboardText(HWND, const char*);
bool  EditClearClipboard(HWND);
void  EditPaste2RectSel(HWND,char*);
bool  EditPasteClipboard(HWND,bool,bool);
bool  EditCopyAppend(HWND,bool);
int   EditDetectEOLMode(HWND,char*,DWORD);
bool  EditLoadFile(HWND,LPCWSTR,bool,bool,int*,int*,bool*,bool*,bool*);
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
void  EditCompressSpaces(HWND);
void  EditRemoveBlankLines(HWND,bool,bool);
void  EditRemoveDuplicateLines(HWND,bool);
void  EditWrapToColumn(HWND,DocPos);
void  EditJoinLinesEx(HWND,bool,bool);
void  EditSortLines(HWND,int);

void  EditJumpTo(HWND, DocLn, DocPos);
void  EditScrollTo(HWND, DocLn, int);
void  EditSelectEx(HWND, DocPos, DocPos, int, int);
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
void  EditClearAllMarks(HWND, DocPos, DocPos);
void  EditMarkAll(HWND, char*, int, DocPos, DocPos, bool, bool, bool);
void  EditUpdateUrlHotspots(HWND, DocPos, DocPos, bool);
void  EditSetAccelWordNav(HWND,bool);
void  EditCompleteWord(HWND,bool);
void  EditGetBookmarkList(HWND,LPWSTR,int);
void  EditSetBookmarkList(HWND,LPCWSTR);
void  EditApplyLexerStyle(HWND, DocPos, DocPos);
void  EditFinalizeStyling(HWND, DocPos);

void  EditMarkAllOccurrences();
void  EditUpdateVisibleUrlHotspot(bool);
void  EditHideNotMarkedLineRange(HWND, DocPos, DocPos, bool);

void  EditEnterTargetTransaction();
void  EditLeaveTargetTransaction();
bool  EditIsInTargetTransaction();

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
