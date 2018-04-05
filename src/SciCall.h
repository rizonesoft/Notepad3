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


/******************************************************************************
* 
* On Windows, the message - passing scheme used to communicate between the 
* container and Scintilla is mediated by the operating system SendMessage 
* function and can lead to bad performance when calling intensively.
* To avoid this overhead, Scintilla provides messages that allow you to call
* the Scintilla message function directly.
* The code to do this in C / C++ is of the form :
* 
*   #include "Scintilla.h"
*   SciFnDirect pSciMsg = (SciFnDirect)SendMessage(hSciWnd, SCI_GETDIRECTFUNCTION, 0, 0);
*   sptr_t pSciWndData = (sptr_t)SendMessage(hSciWnd, SCI_GETDIRECTPOINTER, 0, 0);
* 
*   // now a wrapper to call Scintilla directly
*   sptr_t CallScintilla(unsigned int iMessage, uptr_t wParam, sptr_t lParam) {
*     return pSciMsg(pSciWndData, iMessage, wParam, lParam);
*   }
*
* SciFnDirect, sptr_t and uptr_t are declared in Scintilla.h.hSciWnd 
* is the window handle returned when you created the Scintilla window.
* 
* While faster, this direct calling will cause problems if performed from a 
* different thread to the native thread of the Scintilla window in which case 
* SendMessage(hSciWnd, SCI_*, wParam, lParam) should be used 
* to synchronize with the window's thread.
* 
*******************************************************************************/

#define SCI_DIRECTFUNCTION_INTERFACE 1  // disable for asynchronous operation


#pragma once
#ifndef _NP3_SCICALL_H_
#define _NP3_SCICALL_H_

#include "TypeDefs.h"

extern HANDLE g_hScintilla;
extern HANDLE g_hwndEdit;

//=============================================================================
//
//  Sci_SendMessage()  short version
//
#define Sci_SendMsgV0(CMD)  SendMessage(g_hwndEdit, SCI_##CMD, (WPARAM)0, (LPARAM)0)
#define Sci_SendMsgV1(CMD,WP)  SendMessage(g_hwndEdit, SCI_##CMD, (WPARAM)(WP), (LPARAM)0)
#define Sci_SendMsgV2(CMD,WP,LP)  SendMessage(g_hwndEdit, SCI_##CMD, (WPARAM)(WP), (LPARAM)(LP))

#define Sci_PostMsgV0(CMD)  PostMessage(g_hwndEdit, SCI_##CMD, (WPARAM)0, (LPARAM)0)
#define Sci_PostMsgV1(CMD,WP)  PostMessage(g_hwndEdit, SCI_##CMD, (WPARAM)(WP), (LPARAM)0)
#define Sci_PostMsgV2(CMD,WP,LP)  PostMessage(g_hwndEdit, SCI_##CMD, (WPARAM)(WP), (LPARAM)(LP))

//=============================================================================
//
//  SciCall()
//
#ifdef SCI_DIRECTFUNCTION_INTERFACE

LRESULT WINAPI Scintilla_DirectFunction(HANDLE, UINT, WPARAM, LPARAM);
#define SciCall(m, w, l) Scintilla_DirectFunction(g_hScintilla, m, w, l)

#else

#define SciCall(m, w, l) SendMessage(g_hwndEdit, m, w, l)

#endif // SCI_DIRECTFUNCTION_INTERFACE


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
DeclareSciCallR0(IsDocModified, GETMODIFY, bool)
DeclareSciCallR0(IsSelectionEmpty, GETSELECTIONEMPTY, bool)
DeclareSciCallR0(IsSelectionRectangle, SELECTIONISRECTANGLE, bool)

DeclareSciCallR0(CanPaste, CANPASTE, bool)

DeclareSciCallR0(GetCurrentPos, GETCURRENTPOS, DocPos)
DeclareSciCallR0(GetAnchor, GETANCHOR, DocPos)
DeclareSciCallR0(GetSelectionMode, GETSELECTIONMODE, int)
DeclareSciCallR0(GetSelectionStart, GETSELECTIONSTART, DocPos)
DeclareSciCallR0(GetSelectionEnd, GETSELECTIONEND, DocPos)
DeclareSciCallR1(GetLineSelStartPosition, GETLINESELSTARTPOSITION, DocPos, DocLn, line)
DeclareSciCallR1(GetLineSelEndPosition, GETLINESELENDPOSITION, DocPos, DocLn, line)

// Rectangular selection with virtual space
DeclareSciCallR0(GetRectangularSelectionCaret, GETRECTANGULARSELECTIONCARET, DocPos)
DeclareSciCallV1(SetRectangularSelectionCaret, SETRECTANGULARSELECTIONCARET, DocPos, position)
DeclareSciCallR0(GetRectangularSelectionAnchor, GETRECTANGULARSELECTIONANCHOR, DocPos)
DeclareSciCallV1(SetRectangularSelectionAnchor, SETRECTANGULARSELECTIONANCHOR, DocPos, position)
DeclareSciCallR0(GetRectangularSelectionCaretVirtualSpace, GETRECTANGULARSELECTIONCARETVIRTUALSPACE, DocPos)
DeclareSciCallV1(SetRectangularSelectionCaretVirtualSpace, SETRECTANGULARSELECTIONCARETVIRTUALSPACE, DocPos, position)
DeclareSciCallR0(GetRectangularSelectionAnchorVirtualSpace, GETRECTANGULARSELECTIONANCHORVIRTUALSPACE, DocPos)
DeclareSciCallV1(SetRectangularSelectionAnchorVirtualSpace, SETRECTANGULARSELECTIONANCHORVIRTUALSPACE, DocPos, position)

// Multiselections (Lines of Rectangular selection)
DeclareSciCallV0(ClearSelections, CLEARSELECTIONS)
DeclareSciCallV0(SwapMainAnchorCaret, SWAPMAINANCHORCARET)
DeclareSciCallR0(GetSelections, GETSELECTIONS, DocPosU)
DeclareSciCallR1(GetSelectionNCaret, GETSELECTIONNCARET, DocPos, DocPosU, selnum)
DeclareSciCallR1(GetSelectionNAnchor, GETSELECTIONNANCHOR, DocPos, DocPosU, selnum)
DeclareSciCallR1(GetSelectionNCaretVirtualSpace, GETSELECTIONNCARETVIRTUALSPACE, DocPos, DocPosU, selnum)
DeclareSciCallR1(GetSelectionNAnchorVirtualSpace, GETSELECTIONNANCHORVIRTUALSPACE, DocPos, DocPosU, selnum)
DeclareSciCallV2(SetSelectionNCaret, SETSELECTIONNCARET, DocPosU, selnum, DocPos, caretPos)
DeclareSciCallV2(SetSelectionNAnchor, SETSELECTIONNANCHOR, DocPosU, selnum, DocPos, anchorPos)
DeclareSciCallV2(SetSelectionNCaretVirtualSpace, SETSELECTIONNCARETVIRTUALSPACE, DocPosU, selnum, DocPos, position)
DeclareSciCallV2(SetSelectionNAnchorVirtualSpace, SETSELECTIONNANCHORVIRTUALSPACE, DocPosU, selnum, DocPos, position)


// Operations
DeclareSciCallV0(Cut, CUT)
DeclareSciCallV0(Copy, COPY)
DeclareSciCallV0(Paste, PASTE)
DeclareSciCallV0(Clear, CLEAR)
DeclareSciCallV0(Cancel, CANCEL)
DeclareSciCallV0(CopyAllowLine, COPYALLOWLINE)
DeclareSciCallV0(LineDelete, LINEDELETE)
DeclareSciCallV2(CopyText, COPYTEXT, DocPos, length, const char*, text)
DeclareSciCallV2(GetText, GETTEXT, DocPos, length, const char*, text)

DeclareSciCallV2(SetSel, SETSEL, DocPos, anchorPos, DocPos, currentPos)
DeclareSciCallV0(SelectAll, SELECTALL)
DeclareSciCallR01(GetSelText, GETSELTEXT, DocPos, const char*, text)
DeclareSciCallV01(ReplaceSel, REPLACESEL, const char*, text)
DeclareSciCallR2(GetLine, GETLINE, DocPos, DocLn, line, const char*, text)
DeclareSciCallV2(InsertText, INSERTTEXT, DocPos, position, const char*, text)


DeclareSciCallR0(GetTargetStart, GETTARGETSTART, DocPos)
DeclareSciCallR0(GetTargetEnd, GETTARGETEND, DocPos)
DeclareSciCallV0(TargetFromSelection, TARGETFROMSELECTION)
DeclareSciCallV2(SetTargetRange, SETTARGETRANGE, DocPos, start, DocPos, end)
DeclareSciCallR2(ReplaceTarget, REPLACETARGET, DocPos, DocPos, length, const char*, text)
DeclareSciCallV2(AddText, ADDTEXT, DocPos, length, const char*, text)


DeclareSciCallV1(SetAnchor, SETANCHOR, DocPos, position)
DeclareSciCallV1(SetCurrentPos, SETCURRENTPOS, DocPos, position)
DeclareSciCallV1(SetMultiPaste, SETMULTIPASTE, int, option)

DeclareSciCallV1(GotoPos, GOTOPOS, DocPos, position)
DeclareSciCallV1(GotoLine, GOTOLINE, DocLn, line)
DeclareSciCallR1(PositionBefore, POSITIONBEFORE, DocPos, DocPos, position)
DeclareSciCallR1(PositionAfter, POSITIONAFTER, DocPos, DocPos, position)
DeclareSciCallR1(GetCharAt, GETCHARAT, char, DocPos, position)
DeclareSciCallR0(GetEOLMode, GETEOLMODE, int)

DeclareSciCallR0(GetLineCount, GETLINECOUNT, DocLn)
DeclareSciCallR0(GetTextLength, GETTEXTLENGTH, DocPos)
DeclareSciCallR1(LineLength, LINELENGTH, DocPos, DocLn, line)
DeclareSciCallR1(LineFromPosition, LINEFROMPOSITION, DocLn, DocPos, position)
DeclareSciCallR1(PositionFromLine, POSITIONFROMLINE, DocPos, DocLn, line)
DeclareSciCallR1(GetLineEndPosition, GETLINEENDPOSITION, DocPos, DocLn, line)
DeclareSciCallR1(GetColumn, GETCOLUMN, DocPos, DocPos, position)
DeclareSciCallR2(FindColumn, FINDCOLUMN, DocPos, DocLn, line, DocPos, column)
DeclareSciCallR1(GetLineIndentPosition, GETLINEINDENTPOSITION, DocPos, DocLn, line)

DeclareSciCallR2(GetRangePointer, GETRANGEPOINTER, char* const, DocPos, start, DocPos, length)
DeclareSciCallR0(GetCharacterPointer, GETCHARACTERPOINTER, const char*)

DeclareSciCallV1(SetVirtualSpaceOptions, SETVIRTUALSPACEOPTIONS, int, options)


//=============================================================================
//
//  Commands
//
DeclareSciCallV0(NewLine, NEWLINE)


//=============================================================================
//
//  Scrolling and automatic scrolling
//
DeclareSciCallV2(SetVisiblePolicy, SETVISIBLEPOLICY, int, flags, DocLn, lines)
DeclareSciCallV0(ChooseCaretX, CHOOSECARETX)
DeclareSciCallV0(ScrollCaret, SCROLLCARET)
DeclareSciCallV2(LineScroll, LINESCROLL, DocPos, columns, DocLn, lines)
DeclareSciCallV2(ScrollRange, SCROLLRANGE, DocPos, secondaryPos, DocPos, primaryPos)
DeclareSciCallV1(SetScrollWidth, SETSCROLLWIDTH, int, width)
DeclareSciCallV1(SetEndAtLastLine, SETENDATLASTLINE, bool, flag)
DeclareSciCallR0(GetXoffset, GETXOFFSET, int)
DeclareSciCallV1(SetXoffset, SETXOFFSET, int, offset)

DeclareSciCallR0(LinesOnScreen, LINESONSCREEN, DocLn)
DeclareSciCallR0(GetFirstVisibleLine, GETFIRSTVISIBLELINE, DocLn)
DeclareSciCallV1(SetFirstVisibleLine, SETFIRSTVISIBLELINE, DocLn, line)
DeclareSciCallR1(DocLineFromVisible, DOCLINEFROMVISIBLE, DocLn, DocLn, line)


//=============================================================================
//
//  Style definition
//
DeclareSciCallR1(StyleGetFore, STYLEGETFORE, COLORREF, int, styleNumber)
DeclareSciCallR1(StyleGetBack, STYLEGETBACK, COLORREF, int, styleNumber)
DeclareSciCallV2(SetStyling, SETSTYLING, DocPosCR, length, int, style)
DeclareSciCallV1(StartStyling, STARTSTYLING, DocPos, position)
DeclareSciCallR0(GetEndStyled, GETENDSTYLED, int)

//=============================================================================
//
//  Margins
//
DeclareSciCallV2(SetMarginType, SETMARGINTYPEN, int, margin, int, type)
DeclareSciCallV2(SetMarginWidth, SETMARGINWIDTHN, int, margin, int, pixelWidth)
DeclareSciCallV2(SetMarginMask, SETMARGINMASKN, int, margin, int, mask)
DeclareSciCallV2(SetMarginSensitive, SETMARGINSENSITIVEN, int, margin, bool, sensitive)
DeclareSciCallV2(SetMarginBackN, SETMARGINBACKN, int, margin, COLORREF, colour)
DeclareSciCallV2(SetFoldMarginColour, SETFOLDMARGINCOLOUR, bool, useSetting, COLORREF, colour)
DeclareSciCallV2(SetFoldMarginHiColour, SETFOLDMARGINHICOLOUR, bool, useSetting, COLORREF, colour)


//=============================================================================
//
//  Markers
//
DeclareSciCallR1(MarkerGet, MARKERGET, int, DocLn, line)
DeclareSciCallV2(MarkerDefine, MARKERDEFINE, int, markerNumber, int, markerSymbols)
DeclareSciCallV2(MarkerSetFore, MARKERSETFORE, int, markerNumber, COLORREF, colour)
DeclareSciCallV2(MarkerSetBack, MARKERSETBACK, int, markerNumber, COLORREF, colour)
DeclareSciCallR2(MarkerAdd, MARKERADD, int, DocLn, line, int, markerNumber)
DeclareSciCallV2(MarkerDelete, MARKERDELETE, DocLn, line, int, markerNumber)
DeclareSciCallV1(MarkerDeleteAll, MARKERDELETEALL, int, markerNumber)


//=============================================================================
//
//  Indicators
//
DeclareSciCallR2(IndicatorValueAt, INDICATORVALUEAT, int, int, indicatorID, DocPos, position)
DeclareSciCallV2(IndicatorFillRange, INDICATORFILLRANGE, DocPos, position, DocPos, length)


//=============================================================================
//
//  Folding
//
//
DeclareSciCallR1(GetLineVisible, GETLINEVISIBLE, bool, DocLn, line)
DeclareSciCallV2(SetFoldLevel, SETFOLDLEVEL, DocLn, line, int, flags)
DeclareSciCallR1(GetFoldLevel, GETFOLDLEVEL, int, DocLn, line)
DeclareSciCallV1(SetFoldFlags, SETFOLDFLAGS, int, flags)
DeclareSciCallV1(FoldDisplayTextSetStyle, FOLDDISPLAYTEXTSETSTYLE, int, flags)
DeclareSciCallR1(GetFoldParent, GETFOLDPARENT, int, DocLn, line)
DeclareSciCallR1(GetFoldExpanded, GETFOLDEXPANDED, int, DocLn, line)
DeclareSciCallV1(ToggleFold, TOGGLEFOLD, DocLn, line)
DeclareSciCallV1(FoldAll, FOLDALL, int, flags)
DeclareSciCallV1(EnsureVisible, ENSUREVISIBLE, DocLn, line)
DeclareSciCallV1(EnsureVisibleEnforcePolicy, ENSUREVISIBLEENFORCEPOLICY, DocLn, line)


//=============================================================================
//
//  Lexer
//
DeclareSciCallV2(SetProperty, SETPROPERTY, const char *, key, const char *, value)


//=============================================================================
//
//  Cursor
//
DeclareSciCallV1(SetCursor, SETCURSOR, int, flags)


//=============================================================================
//
//  Undo/Redo Stack
//
DeclareSciCallR0(GetUndoCollection, GETUNDOCOLLECTION, bool)


//=============================================================================
//
//  SetTechnology
//
DeclareSciCallV1(SetBufferedDraw, SETBUFFEREDDRAW, bool, value)
DeclareSciCallV1(SetTechnology, SETTECHNOLOGY, int, technology)


//=============================================================================
//
//  Utilities
//
#define Sci_IsStreamSelected() (SciCall_GetSelectionMode() == SC_SEL_STREAM)
#define Sci_IsFullLineSelected() (SciCall_GetSelectionMode() == SC_SEL_LINES)
#define Sci_IsThinRectangleSelected() (SciCall_GetSelectionMode() == SC_SEL_THIN)
#define Sci_IsSingleLineSelection() \
(SciCall_LineFromPosition(SciCall_GetCurrentPos()) == SciCall_LineFromPosition(SciCall_GetAnchor()))

#define Sci_GetEOLLen() ((SciCall_GetEOLMode() == SC_EOL_CRLF) ? 2 : 1)

#define Sci_GetCurrentLine() SciCall_LineFromPosition(SciCall_GetCurrentPos())

// length of line w/o line-end chars (full use SciCall_LineLength()
#define Sci_GetNetLineLength(line)  (SciCall_GetLineEndPosition(line) - SciCall_PositionFromLine(line))


//=============================================================================

#endif //_NP3_SCICALL_H_
