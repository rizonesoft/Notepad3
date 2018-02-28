/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* SciCall.h                                                                   *
*   Inline wrappers for Scintilla API calls, arranged in the order and        *
*	  grouping in which they appear in the Scintilla documentation.             *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*   The use of these inline wrapper functions with declared types will        *
*   ensure that we get the benefit of the compiler's type checking.           *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2018   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#pragma once
#ifndef _NP3_SCICALL_H_
#define _NP3_SCICALL_H_

#include "TypeDefs.h"

//=============================================================================
//
//  g_hScintilla
//
extern HANDLE g_hScintilla;

__forceinline void InitScintillaHandle(HWND hwnd) {
  g_hScintilla = (HANDLE)SendMessage(hwnd, SCI_GETDIRECTPOINTER, 0, 0);
}

//=============================================================================
//
//  SciCall()
//
LRESULT WINAPI Scintilla_DirectFunction(HANDLE, UINT, WPARAM, LPARAM);
#define SciCall(m, w, l) Scintilla_DirectFunction(g_hScintilla, m, w, l)

//=============================================================================
//
//  DeclareSciCall[RV][0-2] Macros
//
//  R: With an explicit return type
//  V: No return type defined ("void"); defaults to SendMessage's LRESULT
//  0-2: Number of parameters to define
//
#define DeclareSciCallR0(fn, msg, ret)                             \
__forceinline ret SciCall_##fn() {                                 \
  return((ret)SciCall(SCI_##msg, 0, 0));                           \
}
#define DeclareSciCallR1(fn, msg, ret, type1, var1)                \
__forceinline ret SciCall_##fn(type1 var1) {                       \
  return((ret)SciCall(SCI_##msg, (WPARAM)(var1), 0));              \
}
#define DeclareSciCallR01(fn, msg, ret, type2, var2)               \
__forceinline ret SciCall_##fn(type2 var2) {                       \
  return((ret)SciCall(SCI_##msg, 0, (LPARAM)(var2)));              \
}
#define DeclareSciCallR2(fn, msg, ret, type1, var1, type2, var2)   \
__forceinline ret SciCall_##fn(type1 var1, type2 var2) {           \
  return((ret)SciCall(SCI_##msg, (WPARAM)(var1), (LPARAM)(var2))); \
}

#define DeclareSciCallV0(fn, msg)                                  \
__forceinline LRESULT SciCall_##fn() {                             \
  return(SciCall(SCI_##msg, 0, 0));                                \
}
#define DeclareSciCallV1(fn, msg, type1, var1)                     \
__forceinline LRESULT SciCall_##fn(type1 var1) {                   \
  return(SciCall(SCI_##msg, (WPARAM)(var1), 0));                   \
}
#define DeclareSciCallV01(fn, msg, type2, var2)                    \
__forceinline LRESULT SciCall_##fn(type2 var2) {                   \
  return(SciCall(SCI_##msg, 0, (LPARAM)(var2)));                   \
}
#define DeclareSciCallV2(fn, msg, type1, var1, type2, var2)        \
__forceinline LRESULT SciCall_##fn(type1 var1, type2 var2) {       \
  return(SciCall(SCI_##msg, (WPARAM)(var1), (LPARAM)(var2)));      \
}


//=============================================================================
//
//  Selection, positions and information
//
DeclareSciCallR0(IsDocModified, GETMODIFY, bool);
DeclareSciCallR0(IsSelectionEmpty, GETSELECTIONEMPTY, bool);
DeclareSciCallR0(IsSelectionRectangle, SELECTIONISRECTANGLE, bool);

DeclareSciCallR0(CanPaste, CANPASTE, bool);

DeclareSciCallR0(GetCurrentPos, GETCURRENTPOS, DocPos);
DeclareSciCallR0(GetAnchor, GETANCHOR, DocPos);
DeclareSciCallR0(GetSelectionMode, GETSELECTIONMODE, int);
DeclareSciCallR0(GetSelectionStart, GETSELECTIONSTART, DocPos);
DeclareSciCallR0(GetSelectionEnd, GETSELECTIONEND, DocPos);
DeclareSciCallR1(GetLineSelStartPosition, GETLINESELSTARTPOSITION, DocPos, DocLn, line);
DeclareSciCallR1(GetLineSelEndPosition, GETLINESELENDPOSITION, DocPos, DocLn, line);


DeclareSciCallV0(Cut, CUT);
DeclareSciCallV0(Copy, COPY);
DeclareSciCallV0(Paste, PASTE);
DeclareSciCallV0(Clear, CLEAR);
DeclareSciCallV0(CopyAllowLine, COPYALLOWLINE);
DeclareSciCallV0(LineDelete, LINEDELETE);
DeclareSciCallV2(CopyText, COPYTEXT, DocPos, length, LPCCH, text);
DeclareSciCallV2(GetTextFromBegin, GETTEXT, DocPos, length, LPCCH, text);

DeclareSciCallV2(SetSel, SETSEL, DocPos, anchorPos, DocPos, currentPos);
DeclareSciCallV0(SelectAll, SELECTALL);
DeclareSciCallR01(GetSelText, GETSELTEXT, DocPos, LPCCH, text);
DeclareSciCallV01(ReplaceSel, REPLACESEL, LPCCH, text);

DeclareSciCallR0(GetTargetStart, GETTARGETSTART, DocPos);
DeclareSciCallR0(GetTargetEnd, GETTARGETEND, DocPos);
DeclareSciCallV0(TargetFromSelection, TARGETFROMSELECTION);
DeclareSciCallV2(SetTargetRange, SETTARGETRANGE, DocPos, start, DocPos, end);
DeclareSciCallV2(ReplaceTarget, REPLACETARGET, DocPos, length, LPCCH, text);

DeclareSciCallV1(SetAnchor, SETANCHOR, DocPos, position);
DeclareSciCallV1(SetCurrentPos, SETCURRENTPOS, DocPos, position);
DeclareSciCallV1(SetMultiPaste, SETMULTIPASTE, int, option);

DeclareSciCallV1(GotoPos, GOTOPOS, DocPos, position);
DeclareSciCallV1(GotoLine, GOTOLINE, DocLn, line);
DeclareSciCallR1(PositionBefore, POSITIONBEFORE, DocPos, DocPos, position);
DeclareSciCallR1(PositionAfter, POSITIONAFTER, DocPos, DocPos, position);
DeclareSciCallR1(GetCharAt, GETCHARAT, char, DocPos, position);
DeclareSciCallR0(GetEOLMode, GETEOLMODE, int);

DeclareSciCallR0(GetLineCount, GETLINECOUNT, DocLn);
DeclareSciCallR0(GetTextLength, GETTEXTLENGTH, DocPos);
DeclareSciCallR1(LineLength, LINELENGTH, DocPos, DocLn, line);
DeclareSciCallR1(LineFromPosition, LINEFROMPOSITION, DocLn, DocPos, position);
DeclareSciCallR1(PositionFromLine, POSITIONFROMLINE, DocPos, DocLn, line);
DeclareSciCallR1(GetLineEndPosition, GETLINEENDPOSITION, DocPos, DocLn, line);
DeclareSciCallR1(GetColumn, GETCOLUMN, DocPos, DocPos, position);
DeclareSciCallR0(LinesOnScreen, LINESONSCREEN, DocLn);
DeclareSciCallR0(GetFirstVisibleLine, GETFIRSTVISIBLELINE, DocLn);
DeclareSciCallR1(DocLineFromVisible, DOCLINEFROMVISIBLE, DocLn, DocLn, line);

DeclareSciCallR2(GetRangePointer, GETRANGEPOINTER, LPCCH, DocPos, start, DocPos, length);
DeclareSciCallR0(GetCharacterPointer, GETCHARACTERPOINTER, LPCCH);

DeclareSciCallV1(SetVirtualSpaceOptions, SETVIRTUALSPACEOPTIONS, int, options);

//=============================================================================
//
//  Scrolling and automatic scrolling
//
DeclareSciCallV0(ChooseCaretX, CHOOSECARETX);
DeclareSciCallV0(ScrollCaret, SCROLLCARET);
DeclareSciCallV2(SetXCaretPolicy, SETXCARETPOLICY, int, caretPolicy, int, caretSlop);
DeclareSciCallV2(SetYCaretPolicy, SETYCARETPOLICY, int, caretPolicy, int, caretSlop);
DeclareSciCallV2(ScrollRange, SCROLLRANGE, DocPos, secondaryPos, DocPos, primaryPos);


//=============================================================================
//
//  Style definition
//
DeclareSciCallR1(StyleGetFore, STYLEGETFORE, COLORREF, int, styleNumber);
DeclareSciCallR1(StyleGetBack, STYLEGETBACK, COLORREF, int, styleNumber);
DeclareSciCallV2(SetStyling, SETSTYLING, DocPosCR, length, int, style);
DeclareSciCallV1(StartStyling, STARTSTYLING, DocPos, position);
DeclareSciCallR0(GetEndStyled, GETENDSTYLED, int);

//=============================================================================
//
//  Margins
//
DeclareSciCallV2(SetMarginType, SETMARGINTYPEN, int, margin, int, type);
DeclareSciCallV2(SetMarginWidth, SETMARGINWIDTHN, int, margin, int, pixelWidth);
DeclareSciCallV2(SetMarginMask, SETMARGINMASKN, int, margin, int, mask);
DeclareSciCallV2(SetMarginSensitive, SETMARGINSENSITIVEN, int, margin, bool, sensitive);
DeclareSciCallV2(SetMarginBackN, SETMARGINBACKN, int, margin, COLORREF, colour);
DeclareSciCallV2(SetFoldMarginColour, SETFOLDMARGINCOLOUR, bool, useSetting, COLORREF, colour);
DeclareSciCallV2(SetFoldMarginHiColour, SETFOLDMARGINHICOLOUR, bool, useSetting, COLORREF, colour);


//=============================================================================
//
//  Markers
//
DeclareSciCallV2(MarkerDefine, MARKERDEFINE, int, markerNumber, int, markerSymbols);
DeclareSciCallV2(MarkerSetFore, MARKERSETFORE, int, markerNumber, COLORREF, colour);
DeclareSciCallV2(MarkerSetBack, MARKERSETBACK, int, markerNumber, COLORREF, colour);


//=============================================================================
//
//  Indicators
//
DeclareSciCallR2(IndicatorValueAt, INDICATORVALUEAT, int, int, indicatorID, DocPos, position);
DeclareSciCallV2(IndicatorFillRange, INDICATORFILLRANGE, DocPos, position, DocPos, length);


//=============================================================================
//
//  Folding
//
//
DeclareSciCallR1(GetLineVisible, GETLINEVISIBLE, bool, DocLn, line);
DeclareSciCallR1(GetFoldLevel, GETFOLDLEVEL, int, DocLn, line);
DeclareSciCallV1(SetFoldFlags, SETFOLDFLAGS, int, flags);
DeclareSciCallR1(GetFoldParent, GETFOLDPARENT, int, DocLn, line);
DeclareSciCallR1(GetFoldExpanded, GETFOLDEXPANDED, int, DocLn, line);
DeclareSciCallV1(ToggleFold, TOGGLEFOLD, DocLn, line);
DeclareSciCallV1(EnsureVisible, ENSUREVISIBLE, DocLn, line);


//=============================================================================
//
//  Lexer
//
DeclareSciCallV2(SetProperty, SETPROPERTY, const char *, key, const char *, value);


//=============================================================================
//
//  Cursor
//
DeclareSciCallV1(SetCursor, SETCURSOR, int, flags);


//=============================================================================
//
//  Undo/Redo Stack
//
DeclareSciCallR0(GetUndoCollection, GETUNDOCOLLECTION, bool);


//=============================================================================
//
//  SetTechnology
//
DeclareSciCallV1(SetBufferedDraw, SETBUFFEREDDRAW, bool, value);
DeclareSciCallV1(SetTechnology, SETTECHNOLOGY, int, technology);


//=============================================================================
//
//  Utilities
//
#define IsStreamSelected() (SciCall_GetSelectionMode() == SC_SEL_STREAM)
#define IsFullLineSelected() (SciCall_GetSelectionMode() == SC_SEL_LINES)
#define IsThinRectangleSelected() (SciCall_GetSelectionMode() == SC_SEL_THIN)
#define IsSingleLineSelection() \
(SciCall_LineFromPosition(SciCall_GetCurrentPos()) == SciCall_LineFromPosition(SciCall_GetAnchor()))

#define GetEOLLen() ((SciCall_GetEOLMode() == SC_EOL_CRLF) ? 2 : 1)




//=============================================================================

#endif //_NP3_SCICALL_H_
