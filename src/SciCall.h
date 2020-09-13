// encoding: UTF-8
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
*   ensure that we get the benefit of the compiler's type checking.           *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2020   *
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
//  SciCall()
//
#ifdef SCI_DIRECTFUNCTION_INTERFACE

LRESULT WINAPI Scintilla_DirectFunction(HANDLE, UINT, WPARAM, LPARAM);
#define SciCall(m, w, l) Scintilla_DirectFunction(Globals.hndlScintilla, (m), (w), (l))

#else

#define SciCall(m, w, l) SendMessage(Globals.hwndEdit, m, w, l)

#endif // SCI_DIRECTFUNCTION_INTERFACE

// SciOniguruma RegEx search
ptrdiff_t WINAPI OnigRegExFind(const char* pchPattern, const char* pchText, const bool caseSensitive, const int eolMode);

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

// Initialize
DeclareSciCallR0(GetLayoutCache, GETLAYOUTCACHE, int)
DeclareSciCallV1(SetLayoutCache, SETLAYOUTCACHE, int, cache)
DeclareSciCallR0(GetPositionCache, GETPOSITIONCACHE, int)
DeclareSciCallV1(SetPositionCache, SETPOSITIONCACHE, int, cache)

// Document Pointer Handling
DeclareSciCallR0(GetDocPointer, GETDOCPOINTER, sptr_t)
DeclareSciCallV01(SetDocPointer, SETDOCPOINTER, sptr_t, pdoc)
DeclareSciCallR2(CreateDocument, CREATEDOCUMENT, sptr_t, DocPos, bytes, int, options)
DeclareSciCallR2(CreateLoader, CREATELOADER, sptr_t, DocPos, bytes, int, options)
DeclareSciCallV01(AddRefDocument, ADDREFDOCUMENT, sptr_t, pdoc)
DeclareSciCallV01(ReleaseDocument, RELEASEDOCUMENT, sptr_t, pdoc)
DeclareSciCallR0(GetDocumentOptions, GETDOCUMENTOPTIONS, int)

//  Selection, positions and information
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

DeclareSciCallV1(SetEmptySelection, SETEMPTYSELECTION, DocPos, position)
DeclareSciCallR0(GetCurrentPos, GETCURRENTPOS, DocPos)
DeclareSciCallR0(GetAnchor, GETANCHOR, DocPos)
DeclareSciCallR0(GetSelectionStart, GETSELECTIONSTART, DocPos)
DeclareSciCallR0(GetSelectionEnd, GETSELECTIONEND, DocPos)
DeclareSciCallR1(GetLineSelStartPosition, GETLINESELSTARTPOSITION, DocPos, DocLn, line)
DeclareSciCallR1(GetLineSelEndPosition, GETLINESELENDPOSITION, DocPos, DocLn, line)

DeclareSciCallR2(PositionFromPoint, POSITIONFROMPOINT, DocPos, int, pt_x, int, pt_y)
DeclareSciCallR2(CharPositionFromPoint, CHARPOSITIONFROMPOINT, DocPos, int, pt_x, int, pt_y)
DeclareSciCallR2(CharPositionFromPointClose, CHARPOSITIONFROMPOINTCLOSE, DocPos, int, pt_x, int, pt_y)
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
DeclareSciCallR0(GetSelectionMode, GETSELECTIONMODE, int)
DeclareSciCallV1(SetSelectionMode, SETSELECTIONMODE, int, mode)
DeclareSciCallR0(GetSelections, GETSELECTIONS, DocPosU)
DeclareSciCallV2(SetSelection, SETSELECTION, DocPos, caretPos, DocPos, anchorPos)
DeclareSciCallV2(AddSelection, ADDSELECTION, DocPos, caretPos, DocPos, anchorPos)
DeclareSciCallR0(GetMainSelection, GETMAINSELECTION, DocPosU)
DeclareSciCallV1(SetMainSelection, SETMAINSELECTION, DocPosU, selnum)
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
DeclareSciCallR1(GetSelectionNStartVirtualSpace, GETSELECTIONNSTARTVIRTUALSPACE, DocPos, DocPosU, selnum)
DeclareSciCallR1(GetSelectionNEndVirtualSpace, GETSELECTIONNENDVIRTUALSPACE, DocPos, DocPosU, selnum)

DeclareSciCallV0(SwapMainAnchorCaret, SWAPMAINANCHORCARET)
DeclareSciCallV0(MultipleSelectAddEach, MULTIPLESELECTADDEACH)
DeclareSciCallV0(RotateSelection, ROTATESELECTION)

// Zoom
DeclareSciCallR0(GetZoom, GETZOOM, int)
DeclareSciCallV1(SetZoom, SETZOOM, int, zoom)
DeclareSciCallV0(ZoomIn, ZOOMIN)
DeclareSciCallV0(ZoomOut, ZOOMOUT)

// Keyboard Commands
DeclareSciCallV0(NewLine, NEWLINE)
DeclareSciCallV0(Tab, TAB)
DeclareSciCallV0(BackTab, BACKTAB)
DeclareSciCallV0(VCHome, VCHOME)
DeclareSciCallV0(LineUp, LINEUP)
DeclareSciCallV0(LineDown, LINEDOWN)
DeclareSciCallV0(LineUpExtend, LINEUPEXTEND)
DeclareSciCallV0(LineScrollUp, LINESCROLLUP)
DeclareSciCallV0(LineDownExtend, LINEDOWNEXTEND)
DeclareSciCallV0(LineScrollDown, LINESCROLLDOWN)
DeclareSciCallV0(CharLeft, CHARLEFT)
DeclareSciCallV0(CharLeftExtend, CHARLEFTEXTEND)
DeclareSciCallV0(CharRight, CHARRIGHT)
DeclareSciCallV0(CharRightExtend, CHARRIGHTEXTEND)
DeclareSciCallV0(WordLeft, WORDLEFT)
DeclareSciCallV0(WordRight, WORDRIGHT)
DeclareSciCallV0(DeleteBack, DELETEBACK)
DeclareSciCallV0(DelWordLeft, DELWORDLEFT)
DeclareSciCallV0(DelWordRight, DELWORDRIGHT)
DeclareSciCallV0(DelLineLeft, DELLINELEFT)
DeclareSciCallV0(DelLineRight, DELLINERIGHT)
DeclareSciCallV0(LineDelete, LINEDELETE)
DeclareSciCallV0(LineCut, LINECUT)
DeclareSciCallV1(LinesSplit, LINESSPLIT, int, pix)
DeclareSciCallV0(LinesJoin, LINESJOIN)
DeclareSciCallV0(EditToggleOverType, EDITTOGGLEOVERTYPE)

//  Commands
DeclareSciCallV0(LineDuplicate, LINEDUPLICATE)
DeclareSciCallV0(SelectionDuplicate, SELECTIONDUPLICATE)
DeclareSciCallV0(LineTranspose, LINETRANSPOSE)
DeclareSciCallV0(MoveSelectedLinesUp, MOVESELECTEDLINESUP)
DeclareSciCallV0(MoveSelectedLinesDown, MOVESELECTEDLINESDOWN)
DeclareSciCallR0(GetLexer, GETLEXER, int)
DeclareSciCallR2(FindText, FINDTEXT, DocPos, int, flags, struct Sci_TextToFind*, text)

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
DeclareSciCallR2(GetText, GETTEXT, DocPos, DocPos, length, const char*, text)
DeclareSciCallR01(GetTextRange, GETTEXTRANGE, DocPos, struct Sci_TextRange*, textrange)
DeclareSciCallV0(UpperCase, UPPERCASE)
DeclareSciCallV0(LowerCase, LOWERCASE)

//DeclareSciCallR01(TargetAsUTF8, TARGETASUTF8, DocPos, const char*, text)  // WideCharToMultiByteEx(Encoding_SciCP)
// SCI_ENCODEDFROMUTF8 - no need, internal CP is UTF8 always (fixed const for Notepad3)

DeclareSciCallV2(SetSel, SETSEL, DocPos, anchorPos, DocPos, currentPos)
DeclareSciCallV0(SelectAll, SELECTALL)
DeclareSciCallR01(GetSelText, GETSELTEXT, size_t, const char*, text)
DeclareSciCallV01(ReplaceSel, REPLACESEL, const char*, text)
DeclareSciCallV2(InsertText, INSERTTEXT, DocPos, position, const char*, text)
DeclareSciCallV2(AppendText, APPENDTEXT, DocPos, length, const char*, text)
DeclareSciCallV0(SetSavePoint, SETSAVEPOINT)

DeclareSciCallV1(SetAdditionalSelectionTyping, SETADDITIONALSELECTIONTYPING, bool, flag)
DeclareSciCallR0(GetAdditionalSelectionTyping, GETADDITIONALSELECTIONTYPING, bool)

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
DeclareSciCallV0(DocumentStart, DOCUMENTSTART)
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
DeclareSciCallR0(GetCharacterPointer, GETCHARACTERPOINTER, const char* const)

DeclareSciCallR2(GetLine, GETLINE, DocPos, DocLn, line, const char*, text)
DeclareSciCallR2(GetCurLine, GETCURLINE, DocPos, unsigned int, length, const char*, text)

inline DocPos SciCall_GetLine_Safe(DocLn iLine, char* pTxtBuf) {
  DocPos const iLen = SciCall_GetLine(iLine, pTxtBuf);  if (pTxtBuf) pTxtBuf[iLen] = '\0';  return (iLen + 1);
}


//=============================================================================
//
//  CallTip and AutoComplete
//

DeclareSciCallV1(CallTipSetFore, CALLTIPSETFORE, COLORREF, colour)
DeclareSciCallV1(CallTipSetBack, CALLTIPSETBACK, COLORREF, colour)
DeclareSciCallV1(CallTipSetPosition, CALLTIPSETPOSITION, bool, above)
DeclareSciCallR0(CallTipPosStart, CALLTIPPOSSTART, DocPos)
DeclareSciCallV2(CallTipShow, CALLTIPSHOW, DocPos, position, const char*, text)
DeclareSciCallV2(CallTipSetHlt, CALLTIPSETHLT, int, beg, int, end)
DeclareSciCallR0(CallTipActive, CALLTIPACTIVE, bool)
DeclareSciCallV0(CallTipCancel, CALLTIPCANCEL)
DeclareSciCallV1(SetMouseDWellTime, SETMOUSEDWELLTIME, int, millisec)

DeclareSciCallR0(AutoCActive, AUTOCACTIVE, bool)
DeclareSciCallV0(AutoCComplete, AUTOCCOMPLETE)
DeclareSciCallV0(AutoCCancel, AUTOCCANCEL)
DeclareSciCallV0(ClearRegisteredImages, CLEARREGISTEREDIMAGES)
DeclareSciCallV1(AutoCSetIgnoreCase, AUTOCSETIGNORECASE, bool, flag)
DeclareSciCallV1(AutoCSetCaseInsensitiveBehaviour, AUTOCSETCASEINSENSITIVEBEHAVIOUR, int, options)
DeclareSciCallV1(AutoCSetSeperator, AUTOCSETSEPARATOR, char, seperator)
DeclareSciCallV01(AutoCSetFillups, AUTOCSETFILLUPS, const char*, text)
DeclareSciCallV1(AutoCSetChooseSingle, AUTOCSETCHOOSESINGLE, bool, flag)
DeclareSciCallV1(AutoCSetOrder, AUTOCSETORDER, int, options)
DeclareSciCallV1(AutoCSetMulti, AUTOCSETMULTI, int, options)
DeclareSciCallV2(AutoCShow, AUTOCSHOW, DocPos, len, const char*, list)


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
DeclareSciCallV0(ClearDocumentStyle, CLEARDOCUMENTSTYLE)
DeclareSciCallV0(StyleResetDefault, STYLERESETDEFAULT)
DeclareSciCallV2(StyleSetVisible, STYLESETVISIBLE, int, style, bool, visible)
DeclareSciCallR1(StyleGetFore, STYLEGETFORE, COLORREF, char, style)
DeclareSciCallR1(StyleGetBack, STYLEGETBACK, COLORREF, char, style)
DeclareSciCallR1(GetStyleAt, GETSTYLEAT, char, DocPos, position)
DeclareSciCallV2(SetStyling, SETSTYLING, DocPos, length, int, style)
DeclareSciCallV1(StartStyling, STARTSTYLING, DocPos, position)
DeclareSciCallR0(GetEndStyled, GETENDSTYLED, DocPos)

DeclareSciCallR1(StyleGetHotspot, STYLEGETHOTSPOT, bool, int, iStyle)
DeclareSciCallV2(StyleSetHotspot, STYLESETHOTSPOT, int, iStyle, bool, hotspot)
DeclareSciCallV2(SetHotspotActiveFore, SETHOTSPOTACTIVEFORE, bool, useSetting, int, colour)
DeclareSciCallV2(SetHotspotActiveBack, SETHOTSPOTACTIVEBACK, bool, useSetting, int, colour)
DeclareSciCallV1(SetHotspotActiveUnderline, SETHOTSPOTACTIVEUNDERLINE, bool, underline)
DeclareSciCallV1(SetHotspotSigleLine, SETHOTSPOTSINGLELINE, bool, singleline)

DeclareSciCallV1(SetViewWS, SETVIEWWS, int, wspc)
DeclareSciCallV1(SetViewEOL, SETVIEWEOL, bool, eols)

  //=============================================================================
//
// Indentation Guides and Wraping
//
DeclareSciCallV1(SetWrapMode, SETWRAPMODE, int, mode)
DeclareSciCallV1(SetWrapIndentMode, SETWRAPINDENTMODE, int, mode)
DeclareSciCallV1(SetWrapStartIndent, SETWRAPSTARTINDENT, int, mode)

DeclareSciCallV1(SetEdgeMode, SETEDGEMODE, int, mode)
DeclareSciCallR0(GetEdgeMode, GETEDGEMODE, int)
DeclareSciCallV1(SetEdgeColumn, SETEDGECOLUMN, int, column)
DeclareSciCallR0(GetEdgeColumn, GETEDGECOLUMN, int)
DeclareSciCallV1(SetEdgeColour, SETEDGECOLOUR, int, colour)
DeclareSciCallR0(GetEdgeColour, GETEDGECOLOUR, int)
DeclareSciCallV2(MultiEdgeAddLine, MULTIEDGEADDLINE, int, column, int, colour)
DeclareSciCallV0(MultiEdgeClearAll, MULTIEDGECLEARALL)

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

DeclareSciCallR1(BraceMatch, BRACEMATCH, DocPos, DocPos, pos)
DeclareSciCallR2(BraceMatchNext, BRACEMATCHNEXT, DocPos, DocPos, pos, DocPos, posStart)
DeclareSciCallV2(BraceHighLight, BRACEHIGHLIGHT, DocPos, pos1, DocPos, pos2)
DeclareSciCallV1(BraceBadLight, BRACEBADLIGHT, DocPos, pos)
DeclareSciCallV2(BraceHighLightIndicator, BRACEHIGHLIGHTINDICATOR, bool, use, int, indic)
DeclareSciCallV2(BraceBadLightIndicator, BRACEBADLIGHTINDICATOR, bool, use, int, indic)


//=============================================================================
//
//  Margins
//
DeclareSciCallV2(SetMarginTypeN, SETMARGINTYPEN, uintptr_t, margin, int, type)
DeclareSciCallR1(GetMarginWidthN, GETMARGINWIDTHN, int, uintptr_t, margin)
DeclareSciCallV2(SetMarginWidthN, SETMARGINWIDTHN, uintptr_t, margin, int, pixelWidth)
DeclareSciCallV2(SetMarginMaskN, SETMARGINMASKN, uintptr_t, margin, int, mask)
DeclareSciCallV2(SetMarginSensitiveN, SETMARGINSENSITIVEN, uintptr_t, margin, bool, sensitive)
DeclareSciCallV2(SetMarginBackN, SETMARGINBACKN, uintptr_t, margin, COLORREF, colour)
DeclareSciCallV2(SetMarginCursorN, SETMARGINCURSORN, uintptr_t, margin, int, cursor)
DeclareSciCallV2(SetFoldMarginColour, SETFOLDMARGINCOLOUR, bool, useSetting, COLORREF, colour)
DeclareSciCallV2(SetFoldMarginHiColour, SETFOLDMARGINHICOLOUR, bool, useSetting, COLORREF, colour)
DeclareSciCallV1(MarkerEnableHighlight, MARKERENABLEHIGHLIGHT, bool, flag)
DeclareSciCallR2(MarkerNumberFromLine, MARKERNUMBERFROMLINE, int, DocLn, line, int, which)
DeclareSciCallR2(MarkerHandleFromLine, MARKERHANDLEFROMLINE, int, DocLn, line, int, which)

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
DeclareSciCallV2(MarkerAddSet, MARKERADDSET, DocLn, line, int, markerMask)
DeclareSciCallV2(MarkerDelete, MARKERDELETE, DocLn, line, int, markerNumber)
DeclareSciCallV1(MarkerDeleteAll, MARKERDELETEALL, int, markerNumber)
DeclareSciCallV2(MarkerSetBackSelected, MARKERSETBACKSELECTED, int, markerNumber, int, colour)
DeclareSciCallR2(MarkerNext, MARKERNEXT, DocLn, DocLn, start, int, markerMask)
DeclareSciCallR2(MarkerPrevious, MARKERPREVIOUS, DocLn, DocLn, start, int, markerMask)

  //=============================================================================
//
//  Line State
//
DeclareSciCallV2(SetLineState, SETLINESTATE, DocLn, line, int, state)
DeclareSciCallR1(GetLineState, GETLINESTATE, int, DocLn, line)
DeclareSciCallR0(GetMaxLineState, GETMAXLINESTATE, DocLn)
DeclareSciCallV1(SetIdleStyling, SETIDLESTYLING, int, idlestyle)

//=============================================================================
//
//  Indicators
//
DeclareSciCallV2(IndicSetStyle, INDICSETSTYLE, int, indicID, int, style)
DeclareSciCallR1(IndicGetFore, INDICGETFORE, COLORREF, int, indicID)
DeclareSciCallV2(IndicSetFore, INDICSETFORE, int, indicID, COLORREF, colour)
DeclareSciCallV2(IndicSetUnder, INDICSETUNDER, int, indicID, bool, under)
DeclareSciCallV2(IndicSetHoverStyle, INDICSETHOVERSTYLE, int, indicID, int, style)
DeclareSciCallV2(IndicSetHoverFore, INDICSETHOVERFORE, int, indicID, COLORREF, colour)
DeclareSciCallV2(IndicSetAlpha, INDICSETALPHA, int, indicID, int, alpha)
DeclareSciCallV2(IndicSetOutlineAlpha, INDICSETOUTLINEALPHA, int, indicID, int, alpha)
DeclareSciCallV1(SetIndicatorCurrent, SETINDICATORCURRENT, int, indicID)
DeclareSciCallV2(IndicatorFillRange, INDICATORFILLRANGE, DocPos, position, DocPos, length)
DeclareSciCallV2(IndicatorClearRange, INDICATORCLEARRANGE, DocPos, position, DocPos, length)
DeclareSciCallR2(IndicatorValueAt, INDICATORVALUEAT, int, int, indicID, DocPos, position)
DeclareSciCallR2(IndicatorStart, INDICATORSTART, int, int, indicID, DocPos, position)
DeclareSciCallR2(IndicatorEnd, INDICATOREND, int, int, indicID, DocPos, position)

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
DeclareSciCallR0(GetTechnology, GETTECHNOLOGY, int)
DeclareSciCallV1(SetTechnology, SETTECHNOLOGY, int, technology)
DeclareSciCallR0(GetBidirectional, GETBIDIRECTIONAL, int)
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

#define Sci_IsDocEmpty() (SciCall_GetTextLength() <= 0LL)

#define Sci_IsThinSelection() (SciCall_GetSelectionMode() == SC_SEL_THIN)
#define Sci_IsStreamSelection() (SciCall_GetSelectionMode() == SC_SEL_STREAM)
#define Sci_IsMultiSelection() ((SciCall_GetSelections() > 1) && !SciCall_IsSelectionRectangle())
#define Sci_IsMultiOrRectangleSelection() ((SciCall_GetSelections() > 1) || SciCall_IsSelectionRectangle())

#define Sci_IsSelectionSingleLine() (SciCall_LineFromPosition(SciCall_GetSelectionEnd()) == SciCall_LineFromPosition(SciCall_GetSelectionStart()))
#define Sci_IsSelectionMultiLine() ((SciCall_LineFromPosition(SciCall_GetSelectionEnd()) -  SciCall_LineFromPosition(SciCall_GetSelectionStart())) > 1)

#define Sci_IsPosInSelection(position) ((position >= SciCall_GetSelectionStart()) && (position <= SciCall_GetSelectionEnd()))

#define Sci_IsForwardSelection() (SciCall_GetAnchor() <= SciCall_GetCurrentPos())

// w/o terminating '\0'
#define Sci_GetSelTextLength() (SciCall_GetSelText(NULL) - 1)

#define Sci_HaveUndoRedoHistory() (SciCall_CanUndo() || SciCall_CanRedo())

#define Sci_GetCurrentLineNumber() SciCall_LineFromPosition(SciCall_GetCurrentPos())
#define Sci_GetLastDocLineNumber() (SciCall_GetLineCount() - 1)

#define Sci_GetLineStartPosition(position) SciCall_PositionFromLine(SciCall_LineFromPosition(position))

// length of line w/o line-end chars (full use SciCall_LineLength()
#define Sci_GetNetLineLength(line) (SciCall_GetLineEndPosition(line) - SciCall_PositionFromLine(line))

//~#define Sci_GetDocEndPosition() (SciCall_GetTextLength() - 1)
#define Sci_GetDocEndPosition() SciCall_GetLineEndPosition(SciCall_GetLineCount())

#define Sci_ClampAlpha(alpha) clampi((alpha), SC_ALPHA_TRANSPARENT, /*SC_ALPHA_OPAQUE*/SC_ALPHA_NOALPHA)
  
// max. line length in range (incl. line-breaks)
inline DocPos Sci_GetRangeMaxLineLength(DocLn iBeginLine, DocLn iEndLine) {
  DocPos iMaxLineLen = 0;
  for (DocLn iLine = iBeginLine; iLine <= iEndLine; ++iLine) {
    DocPos const iLnLen = SciCall_LineLength(iLine);
    if (iLnLen > iMaxLineLen) { iMaxLineLen = iLnLen; }
  }
  return iMaxLineLen;
}

// respect VSlop settings 
inline void Sci_GotoPosChooseCaret(const DocPos pos) { SciCall_GotoPos(pos); SciCall_ChooseCaretX(); }
inline void Sci_ScrollChooseCaret() { SciCall_ScrollCaret(); SciCall_ChooseCaretX(); }
inline void Sci_ScrollToLine(const DocLn line) { SciCall_EnsureVisible(line); SciCall_ScrollRange(SciCall_PositionFromLine(line), SciCall_GetLineEndPosition(line)); }
inline void Sci_ScrollToCurrentLine() { Sci_ScrollToLine(Sci_GetCurrentLineNumber()); }


#define Sci_ReplaceTarget(M,L,T) (((M) == SCI_REPLACETARGET) ? SciCall_ReplaceTarget((L),(T)) : SciCall_ReplaceTargetRe((L),(T)))

//  if iRangeEnd == -1 : apply style from iRangeStart to document end
#define Sci_ApplyLexerStyle(B, E) SciCall_Colourise((DocPos)(B), (DocPos)(E));
#define Sci_LexerStyleAll() SciCall_Colourise(0, -1)

#define Sci_DisableMouseDWellNotification()  SciCall_SetMouseDWellTime(SC_TIME_FOREVER)  

// ----------------------------------------------------------------------------

#define Sci_GetEOLLen() ((SciCall_GetEOLMode() == SC_EOL_CRLF) ? 2 : 1)

inline int Sci_GetCurrentEOL_A(LPCH eol) {
  switch (SciCall_GetEOLMode()) {
    case SC_EOL_CRLF:
      if (eol) { eol[0] = '\r'; eol[1] = '\n'; eol[2] = '\0'; }
      return 2;
    case SC_EOL_CR:
      if (eol) { eol[0] = '\r'; eol[1] = '\0'; }
      return 1;
    case SC_EOL_LF:
      if (eol) { eol[0] = '\n'; eol[1] = '\0'; }
      return 1;
    default:
      return 0;
  }
}
// ----------------------------------------------------------------------------

inline int Sci_GetCurrentEOL_W(LPWCH eol) {
  switch (SciCall_GetEOLMode()) {
    case SC_EOL_CRLF:
      if (eol) { eol[0] = L'\r'; eol[1] = L'\n'; eol[2] = L'\0'; }
      return 2;
    case SC_EOL_CR:
      if (eol) { eol[0] = L'\r'; eol[1] = L'\0'; }
      return 1;
    case SC_EOL_LF:
      if (eol) { eol[0] = L'\n'; eol[1] = L'\0'; }
      return 1;
    default:
      return 0;
  }
}
// ----------------------------------------------------------------------------


inline DocPos Sci_GetSelectionStartEx() {
  if (!Sci_IsMultiSelection()) { return SciCall_GetSelectionStart(); }
  DocPosU const nsel = SciCall_GetSelections();
  DocPos selStart = Sci_GetDocEndPosition() + 1;
  for (DocPosU i = 0; i < nsel; ++i) {
    DocPos const iStart = SciCall_GetSelectionNStart(i);
    if (iStart < selStart) { selStart = iStart; }
  }
  return selStart;
}
// ----------------------------------------------------------------------------

inline DocPos Sci_GetSelectionEndEx() {
  if (!Sci_IsMultiSelection()) { return SciCall_GetSelectionEnd(); }
  DocPosU const nsel = SciCall_GetSelections();
  DocPos selEnd = 0;
  for (DocPosU i = 0; i < nsel; ++i) {
    DocPos const iEnd = SciCall_GetSelectionNEnd(i);
    if (iEnd > selEnd) { selEnd = iEnd; }
  }
  return selEnd;
}
// ----------------------------------------------------------------------------


//=============================================================================

#endif //_NP3_SCICALL_H_
