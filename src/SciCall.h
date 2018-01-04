/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* SciCall.h                                                                   *
*   Inline wrappers for Scintilla API calls, arranged in the order and        *
*	grouping in which they appear in the Scintilla documentation.             *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*   The use of these inline wrapper functions with declared types will        *
*	ensure that we get the benefit of the compiler's type checking.           *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2016   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#pragma once
#ifndef _NP3_SCICALL_H_
#define _NP3_SCICALL_H_


//=============================================================================
//
//  g_hScintilla
//
//
extern HANDLE g_hScintilla;

__forceinline void InitScintillaHandle(HWND hwnd) {
  g_hScintilla = (HANDLE)SendMessage(hwnd, SCI_GETDIRECTPOINTER, 0, 0);
}


//=============================================================================
//
//  SciCall()
//
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
//
#define DeclareSciCallR0(fn, msg, ret)                             \
__forceinline ret SciCall_##fn() {                                 \
  return((ret)SciCall(SCI_##msg, 0, 0));                           \
}
#define DeclareSciCallR1(fn, msg, ret, type1, var1)                \
__forceinline ret SciCall_##fn(type1 var1) {                       \
  return((ret)SciCall(SCI_##msg, (WPARAM)(var1), 0));              \
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
#define DeclareSciCallV2(fn, msg, type1, var1, type2, var2)        \
__forceinline LRESULT SciCall_##fn(type1 var1, type2 var2) {       \
  return(SciCall(SCI_##msg, (WPARAM)(var1), (LPARAM)(var2)));      \
}


//=============================================================================
//
//  Selection, positions and information
//
//
DeclareSciCallR0(GetCurrentPos, GETCURRENTPOS, int);
DeclareSciCallR0(GetAnchor, GETANCHOR, int);
DeclareSciCallR0(IsSelectionEmpty, GETSELECTIONEMPTY, BOOL);
DeclareSciCallR0(GetSelectionMode, GETSELECTIONMODE, int);
DeclareSciCallR0(GetSelectionStart, GETSELECTIONSTART, int);
DeclareSciCallR0(GetSelectionEnd, GETSELECTIONEND, int);
DeclareSciCallR1(GetLineSelStartPosition, GETLINESELSTARTPOSITION, int, Sci_Position, line);
DeclareSciCallR1(GetLineSelEndPosition, GETLINESELENDPOSITION, int, Sci_Position, line);

DeclareSciCallV2(SetSel, SETSEL, int, anchorPos, int, currentPos);
DeclareSciCallV2(SetTargetRange, SETTARGETRANGE, int, start, int, end);

DeclareSciCallV1(GotoPos, GOTOPOS, int, position);
DeclareSciCallV1(GotoLine, GOTOLINE, int, line);
DeclareSciCallR1(PositionBefore, POSITIONBEFORE, int, Sci_Position, position);
DeclareSciCallR1(PositionAfter, POSITIONAFTER, int, Sci_Position, position);

DeclareSciCallR0(GetLineCount, GETLINECOUNT, int);
DeclareSciCallR0(GetTextLength, GETTEXTLENGTH, int);
DeclareSciCallR1(LineFromPosition, LINEFROMPOSITION, int, Sci_Position, position);
DeclareSciCallR1(PositionFromLine, POSITIONFROMLINE, int, Sci_Position, line);
DeclareSciCallR1(GetLineEndPosition, GETLINEENDPOSITION, int, Sci_Position, line);
DeclareSciCallR0(GetEndStyled, GETENDSTYLED, int);
DeclareSciCallR1(GetColumn, GETCOLUMN, int, Sci_Position, position);
DeclareSciCallR0(LinesOnScreen, LINESONSCREEN, int);
DeclareSciCallR0(GetFirstVisibleLine, GETFIRSTVISIBLELINE, int);
DeclareSciCallR1(DocLineFromVisible, DOCLINEFROMVISIBLE, int, Sci_Position, line);

DeclareSciCallR1(GetCharAt, GETCHARAT, char, Sci_Position, position);

//=============================================================================
//
//  Scrolling and automatic scrolling
//
//
DeclareSciCallV0(ScrollCaret, SCROLLCARET);
DeclareSciCallV2(SetXCaretPolicy, SETXCARETPOLICY, int, caretPolicy, int, caretSlop);
DeclareSciCallV2(SetYCaretPolicy, SETYCARETPOLICY, int, caretPolicy, int, caretSlop);


//=============================================================================
//
//  Style definition
//
//
DeclareSciCallR1(StyleGetFore, STYLEGETFORE, COLORREF, int, styleNumber);
DeclareSciCallR1(StyleGetBack, STYLEGETBACK, COLORREF, int, styleNumber);
DeclareSciCallV1(StartStyling, STARTSTYLING, Sci_Position, position);
DeclareSciCallV2(SetStyling, SETSTYLING, Sci_PositionCR, length, int, style);

//=============================================================================
//
//  Margins
//
//
DeclareSciCallV2(SetMarginType, SETMARGINTYPEN, int, margin, int, type);
DeclareSciCallV2(SetMarginWidth, SETMARGINWIDTHN, int, margin, int, pixelWidth);
DeclareSciCallV2(SetMarginMask, SETMARGINMASKN, int, margin, int, mask);
DeclareSciCallV2(SetMarginSensitive, SETMARGINSENSITIVEN, int, margin, BOOL, sensitive);
DeclareSciCallV2(SetFoldMarginColour, SETFOLDMARGINCOLOUR, BOOL, useSetting, COLORREF, colour);
DeclareSciCallV2(SetFoldMarginHiColour, SETFOLDMARGINHICOLOUR, BOOL, useSetting, COLORREF, colour);


//=============================================================================
//
//  Markers
//
//
DeclareSciCallV2(MarkerDefine, MARKERDEFINE, int, markerNumber, int, markerSymbols);
DeclareSciCallV2(MarkerSetFore, MARKERSETFORE, int, markerNumber, COLORREF, colour);
DeclareSciCallV2(MarkerSetBack, MARKERSETBACK, int, markerNumber, COLORREF, colour);


//=============================================================================
//
//  Folding
//
//
DeclareSciCallR1(GetLineVisible, GETLINEVISIBLE, BOOL, int, line);
DeclareSciCallR1(GetFoldLevel, GETFOLDLEVEL, int, int, line);
DeclareSciCallV1(SetFoldFlags, SETFOLDFLAGS, int, flags);
DeclareSciCallR1(GetFoldParent, GETFOLDPARENT, int, int, line);
DeclareSciCallR1(GetFoldExpanded, GETFOLDEXPANDED, int, int, line);
DeclareSciCallV1(ToggleFold, TOGGLEFOLD, int, line);
DeclareSciCallV1(EnsureVisible, ENSUREVISIBLE, int, line);


//=============================================================================
//
//  Lexer
//
//
DeclareSciCallV2(SetProperty, SETPROPERTY, const char *, key, const char *, value);


//=============================================================================
//
//  SetTechnology
//
//
DeclareSciCallV1(SetBufferedDraw, SETBUFFEREDDRAW, BOOL, value);
DeclareSciCallV1(SetTechnology, SETTECHNOLOGY, int, technology);


#endif //_NP3_SCICALL_H_
