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
*                                                  (c) Rizonesoft 2008-2019   *
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

#include "Scintilla.h"
#include "TypeDefs.h"

//=============================================================================
//
//  Sci_SendMessage()  short version
//
#define Sci_SendMsgV0(CMD)  SendMessage(Globals.hwndEdit, SCI_##CMD, (WPARAM)0, (LPARAM)0)
#define Sci_SendMsgV1(CMD,WP)  SendMessage(Globals.hwndEdit, SCI_##CMD, (WPARAM)(WP), (LPARAM)0)
#define Sci_SendMsgV2(CMD,WP,LP)  SendMessage(Globals.hwndEdit, SCI_##CMD, (WPARAM)(WP), (LPARAM)(LP))

#define Sci_PostMsgV0(CMD)  PostMessage(Globals.hwndEdit, SCI_##CMD, (WPARAM)0, (LPARAM)0)
#define Sci_PostMsgV1(CMD,WP)  PostMessage(Globals.hwndEdit, SCI_##CMD, (WPARAM)(WP), (LPARAM)0)
#define Sci_PostMsgV2(CMD,WP,LP)  PostMessage(Globals.hwndEdit, SCI_##CMD, (WPARAM)(WP), (LPARAM)(LP))

//=============================================================================
//
//  SciCall()
//
#ifdef SCI_DIRECTFUNCTION_INTERFACE

LRESULT WINAPI Scintilla_DirectFunction(HANDLE, UINT, WPARAM, LPARAM);
#define SciCall(m, w, l) Scintilla_DirectFunction(Globals.hndlScintilla, (m), (w), (l))

#else

#define SciCall(m, w, l) SendMessage(Globals.hwndEdit, m, w, l)

#endif // SCI_DIRECTFUNCTION_INTERFACE

// SciOniguruma RegEx search
ptrdiff_t WINAPI OnigRegExFind(const char* pchPattern, const char* pchText, const bool caseSensitive);

//=============================================================================

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

DeclareSciCallR0(GetReadOnly, GETREADONLY, bool)
DeclareSciCallV1(SetReadOnly, SETREADONLY, bool, flag)
DeclareSciCallV0(Undo, UNDO)
DeclareSciCallV0(Redo, REDO)
DeclareSciCallR0(CanUndo, CANUNDO, bool)
DeclareSciCallR0(CanRedo, CANREDO, bool)
DeclareSciCallR0(GetModify, GETMODIFY, bool)
DeclareSciCallR0(CanPaste, CANPASTE, bool)
DeclareSciCallV0(GrabFocus, GRABFOCUS)
DeclareSciCallV1(SetFocus, SETFOCUS, bool, flag)
DeclareSciCallR0(GetFocus, GETFOCUS, bool)

DeclareSciCallV0(LinesSplit, LINESSPLIT)

DeclareSciCallV1(SetEmptySelection, SETEMPTYSELECTION, DocPos, position)
DeclareSciCallR0(GetCurrentPos, GETCURRENTPOS, DocPos)
DeclareSciCallR0(GetAnchor, GETANCHOR, DocPos)
DeclareSciCallR0(GetSelectionMode, GETSELECTIONMODE, int)
DeclareSciCallR0(GetSelectionStart, GETSELECTIONSTART, DocPos)
DeclareSciCallR0(GetSelectionEnd, GETSELECTIONEND, DocPos)
DeclareSciCallR1(GetLineSelStartPosition, GETLINESELSTARTPOSITION, DocPos, DocLn, line)
DeclareSciCallR1(GetLineSelEndPosition, GETLINESELENDPOSITION, DocPos, DocLn, line)

DeclareSciCallR2(PositionFromPoint, POSITIONFROMPOINT, DocPos, int, pt_x, int, pt_y)
DeclareSciCallR2(CharPositionFromPoint, CHARPOSITIONFROMPOINT, DocPos, int, pt_x, int, pt_y)
DeclareSciCallR01(PointXFromPosition, POINTXFROMPOSITION, int, DocPos, position)
DeclareSciCallR01(PointYFromPosition, POINTYFROMPOSITION, int, DocPos, position)

DeclareSciCallR2(WordStartPosition, WORDSTARTPOSITION, DocPos, DocPos, pos, bool, onlyWordChars)
DeclareSciCallR2(WordEndPosition, WORDENDPOSITION, DocPos, DocPos, pos, bool, onlyWordChars)

// Rectangular selection with virtual space
DeclareSciCallR0(GetRectangularSelectionCaret, GETRECTANGULARSELECTIONCARET, DocPos)
DeclareSciCallV1(SetRectangularSelectionCaret, SETRECTANGULARSELECTIONCARET, DocPos, position)
DeclareSciCallR0(GetRectangularSelectionAnchor, GETRECTANGULARSELECTIONANCHOR, DocPos)
DeclareSciCallV1(SetRectangularSelectionAnchor, SETRECTANGULARSELECTIONANCHOR, DocPos, position)
DeclareSciCallR0(GetRectangularSelectionCaretVirtualSpace, GETRECTANGULARSELECTIONCARETVIRTUALSPACE, DocPos)
DeclareSciCallV1(SetRectangularSelectionCaretVirtualSpace, SETRECTANGULARSELECTIONCARETVIRTUALSPACE, DocPos, position)
DeclareSciCallR0(GetRectangularSelectionAnchorVirtualSpace, GETRECTANGULARSELECTIONANCHORVIRTUALSPACE, DocPos)
DeclareSciCallV1(SetRectangularSelectionAnchorVirtualSpace, SETRECTANGULARSELECTIONANCHORVIRTUALSPACE, DocPos, position)
DeclareSciCallV1(SetVirtualSpaceOptions, SETVIRTUALSPACEOPTIONS, int, options)

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
DeclareSciCallR1(GetSelectionNStart, GETSELECTIONNSTART, DocPos, DocPosU, selnum)
DeclareSciCallR1(GetSelectionNEnd, GETSELECTIONNEND, DocPos, DocPosU, selnum)


// Zoom
DeclareSciCallR0(GetZoom, GETZOOM, int)
DeclareSciCallV1(SetZoom, SETZOOM, int, zoom)
DeclareSciCallV0(ZoomIn, ZOOMIN)
DeclareSciCallV0(ZoomOut, ZOOMOUT)

// Operations
DeclareSciCallV0(Cut, CUT)
DeclareSciCallV0(Copy, COPY)
DeclareSciCallV0(Paste, PASTE)
DeclareSciCallV0(Clear, CLEAR)
DeclareSciCallV0(ClearAll, CLEARALL)
DeclareSciCallV2(CopyRange, COPYRANGE, DocPos, start, DocPos, end)
DeclareSciCallV0(Cancel, CANCEL)
DeclareSciCallV0(CopyAllowLine, COPYALLOWLINE)
DeclareSciCallV2(CopyText, COPYTEXT, DocPos, length, const char*, text)
DeclareSciCallV2(GetText, GETTEXT, DocPos, length, const char*, text)
DeclareSciCallR01(GetTextRange, GETTEXTRANGE, DocPos, struct Sci_TextRange*, textrange)

//DeclareSciCallR01(TargetAsUTF8, TARGETASUTF8, DocPos, const char*, text)  // ::WideCharToMultiByte(Encoding_SciCP)
// SCI_ENCODEDFROMUTF8 - no need, internal CP is UTF8 always (fixed const for Notepad3)

DeclareSciCallV2(SetSel, SETSEL, DocPos, anchorPos, DocPos, currentPos)
DeclareSciCallV0(SelectAll, SELECTALL)
DeclareSciCallR01(GetSelText, GETSELTEXT, size_t, const char*, text)
DeclareSciCallV01(ReplaceSel, REPLACESEL, const char*, text)
DeclareSciCallV2(InsertText, INSERTTEXT, DocPos, position, const char*, text)
DeclareSciCallV2(AppendText, APPENDTEXT, DocPos, length, const char*, text)
DeclareSciCallV0(SetSavePoint, SETSAVEPOINT)

DeclareSciCallR0(GetTargetStart, GETTARGETSTART, DocPos)
DeclareSciCallR0(GetTargetEnd, GETTARGETEND, DocPos)
//DeclareSciCallR01(GetTargetText, GETTARGETTEXT, sptr_t, const unsigned char*, text)
DeclareSciCallV0(TargetFromSelection, TARGETFROMSELECTION)
DeclareSciCallV0(TargetWholeDocument, TARGETWHOLEDOCUMENT)
DeclareSciCallV2(SetTargetRange, SETTARGETRANGE, DocPos, start, DocPos, end)
DeclareSciCallR2(ReplaceTarget, REPLACETARGET, DocPos, DocPos, length, const char*, text)
DeclareSciCallR2(ReplaceTargetRe, REPLACETARGETRE, DocPos, DocPos, length, const char*, text)
DeclareSciCallV2(AddText, ADDTEXT, DocPos, length, const char*, text)
DeclareSciCallV1(SetSearchFlags, SETSEARCHFLAGS, int, flags)
DeclareSciCallR2(SearchInTarget, SEARCHINTARGET, DocPos, DocPos, length, const char*, text)
DeclareSciCallV2(DeleteRange, DELETERANGE, DocPos, start, DocPos, length)

DeclareSciCallV1(SetAnchor, SETANCHOR, DocPos, position)
DeclareSciCallV1(SetCurrentPos, SETCURRENTPOS, DocPos, position)
DeclareSciCallV1(SetMultiPaste, SETMULTIPASTE, int, option)

DeclareSciCallV1(GotoPos, GOTOPOS, DocPos, position)
DeclareSciCallV1(GotoLine, GOTOLINE, DocLn, line)
DeclareSciCallV0(DocumentEnd, DOCUMENTEND)
DeclareSciCallR1(PositionBefore, POSITIONBEFORE, DocPos, DocPos, position)
DeclareSciCallR1(PositionAfter, POSITIONAFTER, DocPos, DocPos, position)
DeclareSciCallR1(GetCharAt, GETCHARAT, char, DocPos, position)
DeclareSciCallR0(GetEOLMode, GETEOLMODE, int)
DeclareSciCallV1(SetEOLMode, SETEOLMODE, int, eolmode)
DeclareSciCallV1(ConvertEOLs, CONVERTEOLS, int, eolmode)


DeclareSciCallV0(SetCharsDefault, SETCHARSDEFAULT)
DeclareSciCallV01(SetWordChars, SETWORDCHARS, const char*, chrs)
DeclareSciCallV01(SetWhitespaceChars, SETWHITESPACECHARS, const char*, chrs)
DeclareSciCallV01(SetPunctuationChars, SETPUNCTUATIONCHARS, const char*, chrs)

DeclareSciCallR0(GetLineCount, GETLINECOUNT, DocLn)
DeclareSciCallR0(GetTextLength, GETTEXTLENGTH, DocPos)
DeclareSciCallR1(LineLength, LINELENGTH, DocPos, DocLn, line)
DeclareSciCallR1(LineFromPosition, LINEFROMPOSITION, DocLn, DocPos, position)
DeclareSciCallR1(PositionFromLine, POSITIONFROMLINE, DocPos, DocLn, line)
DeclareSciCallR1(GetLineEndPosition, GETLINEENDPOSITION, DocPos, DocLn, line)
DeclareSciCallR1(GetColumn, GETCOLUMN, DocPos, DocPos, position)
DeclareSciCallR2(FindColumn, FINDCOLUMN, DocPos, DocLn, line, DocPos, column)
DeclareSciCallR2(CountCharacters, COUNTCHARACTERS, DocPos, DocPos, startpos, DocPos, endpos)
DeclareSciCallR2(PositionRelative, POSITIONRELATIVE, DocPos, DocPos, startpos, DocPos, relative)

DeclareSciCallR2(GetRangePointer, GETRANGEPOINTER, char* const, DocPos, start, DocPos, length)
DeclareSciCallR0(GetCharacterPointer, GETCHARACTERPOINTER, char* const)

DeclareSciCallR2(GetLine, GETLINE, DocPos, DocLn, line, const char*, text)
DeclareSciCallR2(GetCurLine, GETCURLINE, DocPos, unsigned int, length, const char*, text)

inline DocPos SciCall_GetLine_Safe(DocLn iLine, char* pTxtBuf) {
  DocPos const iLen = SciCall_GetLine(iLine, pTxtBuf);  if (pTxtBuf) pTxtBuf[iLen] = '\0';  return iLen;
}


//=============================================================================
//
//  CallTip and AutoComplete
//

DeclareSciCallV1(CallTipSetFore, CALLTIPSETFORE, COLORREF, colour)
DeclareSciCallV1(CallTipSetBack, CALLTIPSETBACK, COLORREF, colour)
DeclareSciCallV1(CallTipSetPosition, CALLTIPSETPOSITION, bool, above)
DeclareSciCallV2(CallTipShow, CALLTIPSHOW, DocPos, position, const char*, text)
DeclareSciCallV2(CallTipSetHlt, CALLTIPSETHLT, int, beg, int, end)
DeclareSciCallR0(CallTipActive, CALLTIPACTIVE, bool)
DeclareSciCallV0(CallTipCancel, CALLTIPCANCEL)
DeclareSciCallV1(SetMouseDWellTime, SETMOUSEDWELLTIME, int, millisec)

DeclareSciCallR0(AutoCActive, AUTOCACTIVE, bool)
DeclareSciCallV0(AutoCComplete, AUTOCCOMPLETE)
DeclareSciCallV0(AutoCCancel, AUTOCCANCEL)
DeclareSciCallV1(AutoCSetIgnoreCase, AUTOCSETIGNORECASE, bool, flag)
DeclareSciCallV1(AutoCSetSeperator, AUTOCSETSEPARATOR, char, seperator)
DeclareSciCallV01(AutoCSetFillups, AUTOCSETFILLUPS, const char*, text)
DeclareSciCallV1(AutoCSetChooseSingle, AUTOCSETCHOOSESINGLE, bool, flag)
DeclareSciCallV1(AutoCSetOrder, AUTOCSETORDER, int, options)
DeclareSciCallV2(AutoCShow, AUTOCSHOW, DocPos, len, const char*, list)


//=============================================================================
//
//  Commands
//
DeclareSciCallV0(NewLine, NEWLINE)
DeclareSciCallV0(LineDuplicate, LINEDUPLICATE)
DeclareSciCallV0(SelectionDuplicate, SELECTIONDUPLICATE)
DeclareSciCallV0(LineTranspose, LINETRANSPOSE)
DeclareSciCallV0(MoveSelectedLinesUp, MOVESELECTEDLINESUP)
DeclareSciCallV0(MoveSelectedLinesDown, MOVESELECTEDLINESDOWN)
DeclareSciCallV0(LineDelete, LINEDELETE)
DeclareSciCallV0(DelLineLeft, DELLINELEFT)
DeclareSciCallV0(DelLineRight, DELLINERIGHT)
DeclareSciCallR0(GetLexer, GETLEXER, int)
DeclareSciCallR2(FindText, FINDTEXT, DocPos, int, flags, struct Sci_TextToFind*, text)


//=============================================================================
//
//  Scrolling and automatic scrolling
//
DeclareSciCallV0(ScrollToStart, SCROLLTOSTART)
DeclareSciCallV0(ScrollToEnd, SCROLLTOEND)
DeclareSciCallV0(ScrollCaret, SCROLLCARET)
DeclareSciCallV0(ChooseCaretX, CHOOSECARETX)
DeclareSciCallV2(LineScroll, LINESCROLL, DocPos, columns, DocLn, lines)
DeclareSciCallV2(ScrollRange, SCROLLRANGE, DocPos, secondaryPos, DocPos, primaryPos)
DeclareSciCallV1(SetScrollWidth, SETSCROLLWIDTH, int, width)
DeclareSciCallV1(SetEndAtLastLine, SETENDATLASTLINE, bool, flag)
DeclareSciCallR0(GetXOffset, GETXOFFSET, int)
DeclareSciCallV1(SetXOffset, SETXOFFSET, int, offset)
DeclareSciCallV2(SetVisiblePolicy, SETVISIBLEPOLICY, int, flags, DocLn, lines)
DeclareSciCallV0(MoveCaretInsideView, MOVECARETINSIDEVIEW)

DeclareSciCallR0(LinesOnScreen, LINESONSCREEN, DocLn)
DeclareSciCallR0(GetFirstVisibleLine, GETFIRSTVISIBLELINE, DocLn)
DeclareSciCallV1(SetFirstVisibleLine, SETFIRSTVISIBLELINE, DocLn, line)
DeclareSciCallR1(VisibleFromDocLine, VISIBLEFROMDOCLINE, DocLn, DocLn, line)
DeclareSciCallR1(DocLineFromVisible, DOCLINEFROMVISIBLE, DocLn, DocLn, line)

//=============================================================================
//
//  Style definition
//
DeclareSciCallR1(StyleGetFore, STYLEGETFORE, COLORREF, char, style)
DeclareSciCallR1(StyleGetBack, STYLEGETBACK, COLORREF, char, style)
DeclareSciCallR1(GetStyleAt, GETSTYLEAT, char, DocPos, position)
DeclareSciCallV2(SetStyling, SETSTYLING, DocPosCR, length, int, style)
DeclareSciCallV1(StartStyling, STARTSTYLING, DocPos, position)
DeclareSciCallR0(GetEndStyled, GETENDSTYLED, DocPos)
DeclareSciCallR1(StyleGetHotspot, STYLEGETHOTSPOT, bool, int, iStyle)

//=============================================================================
//
// Indentation Guides and Wraping
//
DeclareSciCallV1(SetWrapMode, SETWRAPMODE, int, mode)
DeclareSciCallV1(SetWrapIndentMode, SETWRAPINDENTMODE, int, mode)
DeclareSciCallV1(SetWrapStartIndent, SETWRAPSTARTINDENT, int, mode)

DeclareSciCallV1(SetEdgeMode, SETEDGEMODE, int, mode)
DeclareSciCallV1(SetEdgeColumn, SETEDGECOLUMN, int, column)

DeclareSciCallV1(SetTabWidth, SETTABWIDTH, int, width)
DeclareSciCallR0(GetTabWidth, GETTABWIDTH, int)

DeclareSciCallV1(SetIndent, SETINDENT, int, width)
DeclareSciCallR0(GetIndent, GETINDENT, int)
DeclareSciCallV1(SetUseTabs, SETUSETABS, bool, use)
DeclareSciCallR0(GetUseTabs, GETUSETABS, bool)
DeclareSciCallV1(SetTabIndents, SETTABINDENTS, bool, indents)
DeclareSciCallR0(GetTabIndents, GETTABINDENTS, bool)
DeclareSciCallV1(SetBackSpaceUnIndents, SETBACKSPACEUNINDENTS, bool, unindents)
DeclareSciCallR0(GetBackSpaceUnIndents, GETBACKSPACEUNINDENTS, bool)

DeclareSciCallV2(SetLineIndentation, SETLINEINDENTATION, DocLn, line, DocPos, pos)
DeclareSciCallR1(GetLineIndentation, GETLINEINDENTATION, int, DocLn, line)
DeclareSciCallR1(GetLineIndentPosition, GETLINEINDENTPOSITION, DocPos, DocLn, line)

DeclareSciCallV1(SetIndentationGuides, SETINDENTATIONGUIDES, int, iview)
DeclareSciCallV1(SetHighLightGuide, SETHIGHLIGHTGUIDE, int, column)

DeclareSciCallR1(BraceMatch, BRACEMATCH, DocPos, DocPos, position)
DeclareSciCallV2(BraceHighLight, BRACEHIGHLIGHT, DocPos, pos1, DocPos, pos2)
DeclareSciCallV1(BraceBadLight, BRACEBADLIGHT, DocPos, pos)
DeclareSciCallV2(BraceHighLightIndicator, BRACEHIGHLIGHTINDICATOR, bool, use, int, indic)
DeclareSciCallV2(BraceBadLightIndicator, BRACEBADLIGHTINDICATOR, bool, use, int, indic)


//=============================================================================
//
//  Margins
//
DeclareSciCallV2(SetMarginTypeN, SETMARGINTYPEN, int, margin, int, type)
DeclareSciCallR1(GetMarginWidthN, GETMARGINWIDTHN, int, int, margin)
DeclareSciCallV2(SetMarginWidthN, SETMARGINWIDTHN, int, margin, int, pixelWidth)
DeclareSciCallV2(SetMarginMaskN, SETMARGINMASKN, int, margin, int, mask)
DeclareSciCallV2(SetMarginSensitiveN, SETMARGINSENSITIVEN, int, margin, bool, sensitive)
DeclareSciCallV2(SetMarginBackN, SETMARGINBACKN, int, margin, COLORREF, colour)
DeclareSciCallV2(SetFoldMarginColour, SETFOLDMARGINCOLOUR, bool, useSetting, COLORREF, colour)
DeclareSciCallV2(SetFoldMarginHiColour, SETFOLDMARGINHICOLOUR, bool, useSetting, COLORREF, colour)
DeclareSciCallV1(MarkerEnableHighlight, MARKERENABLEHIGHLIGHT, bool, flag)
DeclareSciCallR2(TextWidth, TEXTWIDTH, int, int, styleNumber, const char*, text)

//=============================================================================
//
//  Markers
//
DeclareSciCallR1(MarkerGet, MARKERGET, int, DocLn, line)
DeclareSciCallV2(MarkerDefine, MARKERDEFINE, int, markerNumber, int, markerSymbols)
DeclareSciCallV2(MarkerSetFore, MARKERSETFORE, int, markerNumber, COLORREF, colour)
DeclareSciCallV2(MarkerSetBack, MARKERSETBACK, int, markerNumber, COLORREF, colour)
DeclareSciCallV2(MarkerSetAlpha, MARKERSETALPHA, int, markerNumber, int, alpha)
DeclareSciCallR2(MarkerAdd, MARKERADD, int, DocLn, line, int, markerNumber)
DeclareSciCallV2(MarkerDelete, MARKERDELETE, DocLn, line, int, markerNumber)
DeclareSciCallV1(MarkerDeleteAll, MARKERDELETEALL, int, markerNumber)
DeclareSciCallV2(MarkerSetBackSelected, MARKERSETBACKSELECTED, int, markerNumber, int, colour)

//=============================================================================
//
//  Line State
//
DeclareSciCallV2(SetLineState, SETLINESTATE, DocLn, line, int, state)
DeclareSciCallR1(GetLineState, GETLINESTATE, int, DocLn, line)
DeclareSciCallR0(GetMaxLineState, GETMAXLINESTATE, DocLn)

//=============================================================================
//
//  Indicators
//
DeclareSciCallV2(IndicSetStyle, INDICSETSTYLE, int, indicatorID, int, style)
DeclareSciCallV2(IndicSetFore, INDICSETFORE, int, indicatorID, COLORREF, colour)
DeclareSciCallV2(IndicSetHoverStyle, INDICSETHOVERSTYLE, int, indicatorID, int, style)
DeclareSciCallV2(IndicSetHoverFore, INDICSETHOVERFORE, int, indicatorID, COLORREF, colour)
DeclareSciCallV2(IndicSetAlpha, INDICSETALPHA, int, indicatorID, int, alpha)
DeclareSciCallV2(IndicSetOutlineAlpha, INDICSETOUTLINEALPHA, int, indicatorID, int, alpha)
DeclareSciCallV1(SetIndicatorCurrent, SETINDICATORCURRENT, int, indicatorID)
DeclareSciCallV2(IndicatorFillRange, INDICATORFILLRANGE, DocPos, position, DocPos, length)
DeclareSciCallV2(IndicatorClearRange, INDICATORCLEARRANGE, DocPos, position, DocPos, length)
DeclareSciCallR2(IndicatorValueAt, INDICATORVALUEAT, int, int, indicatorID, DocPos, position)
DeclareSciCallR2(IndicatorStart, INDICATORSTART, int, int, indicatorID, DocPos, position)
DeclareSciCallR2(IndicatorEnd, INDICATOREND, int, int, indicatorID, DocPos, position)

//=============================================================================
//
//  Folding
//
//
DeclareSciCallR1(GetLineVisible, GETLINEVISIBLE, bool, DocLn, line)
DeclareSciCallV2(SetFoldLevel, SETFOLDLEVEL, DocLn, line, int, level)
DeclareSciCallR1(GetFoldLevel, GETFOLDLEVEL, int, DocLn, line)
DeclareSciCallV1(SetFoldFlags, SETFOLDFLAGS, int, flags)
DeclareSciCallV1(FoldDisplayTextSetStyle, FOLDDISPLAYTEXTSETSTYLE, int, flags)
DeclareSciCallR1(GetFoldParent, GETFOLDPARENT, DocLn, DocLn, line)
DeclareSciCallR2(GetLastChild, GETLASTCHILD, DocLn, DocLn, line, int, level)
DeclareSciCallR1(GetFoldExpanded, GETFOLDEXPANDED, bool, DocLn, line)
DeclareSciCallV1(ToggleFold, TOGGLEFOLD, DocLn, line)
DeclareSciCallV1(FoldAll, FOLDALL, int, flags)
DeclareSciCallV1(EnsureVisible, ENSUREVISIBLE, DocLn, line)
DeclareSciCallV1(EnsureVisibleEnforcePolicy, ENSUREVISIBLEENFORCEPOLICY, DocLn, line)


//=============================================================================
//
//  Lexer
//
DeclareSciCallV2(SetProperty, SETPROPERTY, const char*, key, const char*, value)
DeclareSciCallV2(SetKeywords, SETKEYWORDS, int, keywordset, const char*, keywords)
DeclareSciCallV2(Colourise, COLOURISE, DocPos, startPos, DocPos, endPos)


//=============================================================================
//
//  Cursor
//
DeclareSciCallV1(SetCursor, SETCURSOR, int, flags)


//=============================================================================
//
//  Undo/Redo Stack
//
DeclareSciCallV0(EmptyUndoBuffer, EMPTYUNDOBUFFER)
DeclareSciCallV0(BeginUndoAction, BEGINUNDOACTION)
DeclareSciCallV2(AddUndoAction, ADDUNDOACTION, int, token, int, flags)
DeclareSciCallV0(EndUndoAction, ENDUNDOACTION)
DeclareSciCallR0(GetUndoCollection, GETUNDOCOLLECTION, bool)
DeclareSciCallV1(SetUndoCollection, SETUNDOCOLLECTION, bool, bCollectUndo)


//=============================================================================
//
//  SetTechnology
//
DeclareSciCallV1(SetBufferedDraw, SETBUFFEREDDRAW, bool, value)
DeclareSciCallV1(SetTechnology, SETTECHNOLOGY, int, technology)
DeclareSciCallV1(SetBidirectional, SETBIDIRECTIONAL, int, direction)
DeclareSciCallV1(SetCharacterCategoryOptimization, SETCHARACTERCATEGORYOPTIMIZATION, int, count)

//=============================================================================
//
//  IME
//
DeclareSciCallV1(SetIMEInteraction, SETIMEINTERACTION, int, interact)
DeclareSciCallR0(IsIMEOpen, ISIMEOPEN, bool)
DeclareSciCallR0(IsIMEModeCJK, ISIMEMODECJK, bool)


//=============================================================================
//
//  Utilities
//
DeclareSciCallR0(IsSelectionEmpty, GETSELECTIONEMPTY, bool)
DeclareSciCallR0(IsSelectionRectangle, SELECTIONISRECTANGLE, bool)
#define Sci_IsMultiSelection() (SciCall_GetSelections() > 1)
#define Sci_IsMultiOrRectangleSelection() (SciCall_IsSelectionRectangle() || Sci_IsMultiSelection())

#define Sci_IsSingleLineSelection() (SciCall_LineFromPosition(SciCall_GetSelectionEnd()) == SciCall_LineFromPosition(SciCall_GetSelectionStart()))
#define Sci_IsMultiLineSelection() ((SciCall_LineFromPosition(SciCall_GetSelectionEnd()) -  SciCall_LineFromPosition(SciCall_GetSelectionStart())) > 1)

#define Sci_IsForwardSelection() (SciCall_GetAnchor() <= SciCall_GetCurrentPos())

#define Sci_HaveUndoRedoHistory() (SciCall_CanUndo() || SciCall_CanRedo())

#define Sci_GetEOLLen() ((SciCall_GetEOLMode() == SC_EOL_CRLF) ? 2 : 1)

#define Sci_GetCurrentLineNumber() SciCall_LineFromPosition(SciCall_GetCurrentPos())
#define Sci_GetLastDocLineNumber() (SciCall_GetLineCount() - 1)

#define Sci_GetLineStartPosition(position) SciCall_PositionFromLine(SciCall_LineFromPosition(position))

// length of line w/o line-end chars (full use SciCall_LineLength()
#define Sci_GetNetLineLength(line) (SciCall_GetLineEndPosition(line) - SciCall_PositionFromLine(line))

//~#define Sci_GetDocEndPosition() (SciCall_GetTextLength() - 1)
#define Sci_GetDocEndPosition() SciCall_GetLineEndPosition(SciCall_GetLineCount() - 1)

// max. line length in range (incl. line-breaks)
inline DocPos Sci_GetRangeMaxLineLength(DocLn iBeginLine, DocLn iEndLine) {
  DocPos iMaxLineLen = 0;
  for (DocLn iLine = iBeginLine; iLine <= iEndLine; ++iLine) {
    DocPos const iLnLen = SciCall_LineLength(iLine);
    if (iLnLen > iMaxLineLen) { iMaxLineLen = iLnLen; }
  }
  return iMaxLineLen;
}

#define Sci_ReplaceTarget(M,L,T) (((M) == SCI_REPLACETARGET) ? SciCall_ReplaceTarget((L),(T)) : SciCall_ReplaceTargetRe((L),(T)))

//  if iRangeEnd == -1 : apply style from iRangeStart to document end
#define Sci_ApplyLexerStyle(B, E) SciCall_Colourise((B), (E));

#define Sci_DisableMouseDWellNotification()  SciCall_SetMouseDWellTime(SC_TIME_FOREVER)  

//=============================================================================

#endif //_NP3_SCICALL_H_
