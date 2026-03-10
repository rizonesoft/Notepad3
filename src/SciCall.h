// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* SciCall.h                                                                   *
*   Inline wrappers for Scintilla API calls, arranged in the order and        *
*   grouping in which they appear in the Scintilla documentation.             *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*   The use of these inline wrapper functions with declared types will        *
*   ensure that we get the benefit of the compiler's type checking.           *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2026   *
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
//  Scintilla Window Handle
//
#if defined(__cplusplus)
extern "C" HWND   g_hwndEditWindow;
extern "C" HANDLE g_hndlScintilla;
#else
extern HWND   g_hwndEditWindow;
extern HANDLE g_hndlScintilla;
#endif


__forceinline void InitScintillaHandle(HWND hwnd) {
    g_hwndEditWindow = hwnd;
    g_hndlScintilla = (HANDLE)SendMessage(hwnd, SCI_GETDIRECTPOINTER, 0, 0);
}


//=============================================================================
//
//  SciCall()
//
#ifdef SCI_DIRECTFUNCTION_INTERFACE

LRESULT WINAPI Scintilla_DirectFunction(HANDLE, UINT, WPARAM, LPARAM);
#define SciCall(m, w, l) Scintilla_DirectFunction(g_hndlScintilla, (m), (w), (l))

LRESULT WINAPI Scintilla_DirectStatusFunction(HANDLE, UINT, WPARAM, LPARAM, LPINT);
#define SciCallEx(m, w, l, i) Scintilla_DirectStatusFunction(g_hndlScintilla, (m), (w), (l), (i))

#else

//~#define SciCall(m, w, l) SendMessage(g_hndlScintilla, (m), (w), (l))
#define SciCall(m, w, l) SendMessage(g_hwndEditWindow, (m), (w), (l))

#endif // SCI_DIRECTFUNCTION_INTERFACE


//=============================================================================

#define SciCall_SendMsg(m, w, l) SendMessage(g_hwndEditWindow, (m), (w), (l))
#define SciCall_PostMsg(m, w, l) PostMessage(g_hwndEditWindow, (m), (w), (l))

//=============================================================================

// PCRE2 RegEx search (exported from PCRE2RegExEngine.cxx)
ptrdiff_t WINAPI RegExFind(const char* pchPattern, const char* pchText,
                           const bool caseSensitive, int *matchLen_out);

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
// Initialize
//
# 
//  SetTechnology
DeclareSciCallR0(GetTechnology, GETTECHNOLOGY, int);
DeclareSciCallV1(SetTechnology, SETTECHNOLOGY, int, technology);
DeclareSciCallR0(GetBidirectional, GETBIDIRECTIONAL, int);
DeclareSciCallV1(SetBidirectional, SETBIDIRECTIONAL, int, direction);
DeclareSciCallR0(GetBufferedDraw, GETBUFFEREDDRAW, bool);
DeclareSciCallV1(SetBufferedDraw, SETBUFFEREDDRAW, bool, value);
DeclareSciCallR0(GetPhasesDraw, GETPHASESDRAW, int);
DeclareSciCallV1(SetPhasesDraw, SETPHASESDRAW, int, phases);
DeclareSciCallR0(GetCharacterCategoryOptimization, GETCHARACTERCATEGORYOPTIMIZATION, int);
DeclareSciCallV1(SetCharacterCategoryOptimization, SETCHARACTERCATEGORYOPTIMIZATION, int, count);
DeclareSciCallR1(SupportsFeature, SUPPORTSFEATURE, bool, int, feature);
// Layout, Position Cache and Layout Threads
DeclareSciCallR0(GetLayoutCache, GETLAYOUTCACHE, int);
DeclareSciCallV1(SetLayoutCache, SETLAYOUTCACHE, int, cache);
DeclareSciCallR0(GetPositionCache, GETPOSITIONCACHE, int);
DeclareSciCallV1(SetPositionCache, SETPOSITIONCACHE, int, cache);
DeclareSciCallR0(GetLayoutThreads, GETLAYOUTTHREADS, int);
DeclareSciCallV1(SetLayoutThreads, SETLAYOUTTHREADS, int, threads);

// Event Masks
DeclareSciCallR0(GetModEventMask, GETMODEVENTMASK, int);
DeclareSciCallV1(SetModEventMask, SETMODEVENTMASK, int, mask);
DeclareSciCallR0(GetCommandEvents, GETCOMMANDEVENTS, bool);
DeclareSciCallV1(SetCommandEvents, SETCOMMANDEVENTS, bool, flag);
// Code Page
DeclareSciCallR0(GetCodePage, GETCODEPAGE, int);
DeclareSciCallV1(SetCodePage, SETCODEPAGE, int, cp);
DeclareSciCallV1(StyleSetCharacterSet, STYLESETCHARACTERSET, int, cs);
// Divers
DeclareSciCallR0(GetMargins, GETMARGINS, int);
DeclareSciCallV1(SetMargins, SETMARGINS, int, nmarg);
DeclareSciCallV1(SetPasteConvertEndings, SETPASTECONVERTENDINGS, bool, flag);
DeclareSciCallV1(UsePopUp, USEPOPUP, int, option);
// Multi Selection
DeclareSciCallR0(GetMultipleSelection, GETMULTIPLESELECTION, bool);
DeclareSciCallV1(SetMultipleSelection, SETMULTIPLESELECTION, bool, flag);
DeclareSciCallR0(GetMultiPaste, GETMULTIPASTE, int);
DeclareSciCallV1(SetMultiPaste, SETMULTIPASTE, int, option);
DeclareSciCallR0(GetAdditionalSelectionTyping, GETADDITIONALSELECTIONTYPING, bool);
DeclareSciCallV1(SetAdditionalSelectionTyping, SETADDITIONALSELECTIONTYPING, bool, flag);
DeclareSciCallR0(GetMouseSelectionRectangularSwitch, GETMOUSESELECTIONRECTANGULARSWITCH, bool);
DeclareSciCallV1(SetMouseSelectionRectangularSwitch, SETMOUSESELECTIONRECTANGULARSWITCH, bool, flag);

// Caret Line
DeclareSciCallR0(GetCaretLineVisible, GETCARETLINEVISIBLE, bool);
DeclareSciCallV1(SetCaretLineVisible, SETCARETLINEVISIBLE, bool, show);
DeclareSciCallR0(GetCaretLineBack, GETCARETLINEBACK, COLORREF);
DeclareSciCallV1(SetCaretLineBack, SETCARETLINEBACK, COLORREF, back);
DeclareSciCallR0(GetCaretLineBackAlpha, GETCARETLINEBACKALPHA, int);
DeclareSciCallV1(SetCaretLineBackAlpha, SETCARETLINEBACKALPHA, int, alpha);
DeclareSciCallR0(GetCaretLineLayer, GETCARETLINELAYER, int);
DeclareSciCallV1(SetCaretLineLayer, SETCARETLINELAYER, int, layer);
DeclareSciCallR0(GetCaretLineFrame, GETCARETLINEFRAME, int);
DeclareSciCallV1(SetCaretLineFrame, SETCARETLINEFRAME, int, frm);
DeclareSciCallR0(GetCaretLineVisibleAlways, GETCARETLINEVISIBLEALWAYS, bool);
DeclareSciCallV1(SetCaretLineVisibleAlways, SETCARETLINEVISIBLEALWAYS, bool, flag);
DeclareSciCallR0(GetCaretLineHighlightSubLine, GETCARETLINEHIGHLIGHTSUBLINE, bool);
DeclareSciCallV1(SetCaretLineHighlightSubLine, SETCARETLINEHIGHLIGHTSUBLINE, bool, subLine);

DeclareSciCallR0(GetAutomaticFold, GETAUTOMATICFOLD, int);
DeclareSciCallV1(SetAutomaticFold, SETAUTOMATICFOLD, int, option);

DeclareSciCallR0(GetCaretSticky, GETCARETSTICKY, int);
DeclareSciCallV1(SetCaretSticky, SETCARETSTICKY, int, option);
DeclareSciCallV0(ToggleCaretSticky, TOGGLECARETSTICKY);
DeclareSciCallV2(SetXCaretPolicy, SETXCARETPOLICY, int, policy, int, slop);
DeclareSciCallV2(SetYCaretPolicy, SETYCARETPOLICY, int, policy, int, slop);

DeclareSciCallV01(GetWordChars, GETWORDCHARS, char*, ptxt);
DeclareSciCallV01(GetWhiteSpaceChars, GETWHITESPACECHARS, char*, ptxt);
DeclareSciCallV01(GetPunctuationChars, GETPUNCTUATIONCHARS, char*, ptxt);

DeclareSciCallV2(SetRepresentation, SETREPRESENTATION, const char*, encChar, const char*, represent);
DeclareSciCallR2(GetRepresentation, GETREPRESENTATION, int, const char*, encodedCharacter, char*, representation);
DeclareSciCallV1(ClearRepresentation, CLEARREPRESENTATION, const char*, encodedCharacter);
DeclareSciCallV0(ClearAllRepresentations, CLEARALLREPRESENTATIONS);
DeclareSciCallV2(SetRepresentationAppearance, SETREPRESENTATIONAPPEARANCE, const char*, encChar, int, appear);
DeclareSciCallR1(GetRepresentationAppearance, GETREPRESENTATIONAPPEARANCE, int, const char*, encodedCharacter);
DeclareSciCallV2(SetRepresentationColour, SETREPRESENTATIONCOLOUR, const char*, encChar, COLORALPHAREF, colour);
DeclareSciCallR1(GetRepresentationColour, GETREPRESENTATIONCOLOUR, COLORALPHAREF, const char*, encodedCharacter);
// Line End Types
DeclareSciCallV1(SetLineEndTypesAllowed, SETLINEENDTYPESALLOWED, int, lineEndBitSet);
DeclareSciCallR0(GetLineEndTypesAllowed, GETLINEENDTYPESALLOWED, int);
DeclareSciCallR0(GetLineEndTypesActive, GETLINEENDTYPESACTIVE, int);
// Tab Draw Mode
DeclareSciCallR0(GetTabDrawMode, GETTABDRAWMODE, int);
DeclareSciCallV1(SetTabDrawMode, SETTABDRAWMODE, int, tabDrawMode);
// Tab Minimum Width
DeclareSciCallV1(SetTabMinimumWidth, SETTABMINIMUMWIDTH, int, pixels);
DeclareSciCallR0(GetTabMinimumWidth, GETTABMINIMUMWIDTH, int);
// Accessibility
DeclareSciCallV1(SetAccessibility, SETACCESSIBILITY, int, accessibility);
DeclareSciCallR0(GetAccessibility, GETACCESSIBILITY, int);
// Identifier
DeclareSciCallV1(SetIdentifier, SETIDENTIFIER, int, identifier);
DeclareSciCallR0(GetIdentifier, GETIDENTIFIER, int);
// Font
DeclareSciCallR0(GetFontQuality, GETFONTQUALITY, int);
DeclareSciCallR01(GetFontLocale, GETFONTLOCALE, int, char*, localeName);
// Direct Function Pointers
DeclareSciCallR0(GetDirectFunction, GETDIRECTFUNCTION, sptr_t);
DeclareSciCallR0(GetDirectStatusFunction, GETDIRECTSTATUSFUNCTION, sptr_t);
// Allocation
DeclareSciCallV1(AllocateLines, ALLOCATELINES, DocLn, lines);
DeclareSciCallV1(Allocate, ALLOCATE, DocPos, bytes);

// Document Pointer Handling
DeclareSciCallR0(GetDocPointer, GETDOCPOINTER, sptr_t);
DeclareSciCallV01(SetDocPointer, SETDOCPOINTER, sptr_t, pdoc);
DeclareSciCallR2(CreateDocument, CREATEDOCUMENT, sptr_t, DocPos, bytes, int, options);
DeclareSciCallR2(CreateLoader, CREATELOADER, sptr_t, DocPos, bytes, int, options);
DeclareSciCallV01(AddRefDocument, ADDREFDOCUMENT, sptr_t, pdoc);
DeclareSciCallV01(ReleaseDocument, RELEASEDOCUMENT, sptr_t, pdoc);
DeclareSciCallR0(GetDocumentOptions, GETDOCUMENTOPTIONS, int);

// Element Colors
DeclareSciCallV2(SetElementColour, SETELEMENTCOLOUR, int, element, COLORALPHAREF, colourElement);
DeclareSciCallR1(GetElementColour, GETELEMENTCOLOUR, COLORALPHAREF, int, element);
DeclareSciCallR1(GetElementBaseColour, GETELEMENTBASECOLOUR, COLORALPHAREF, int, element);
DeclareSciCallV1(ResetElementColour, RESETELEMENTCOLOUR, int, element);
DeclareSciCallR1(GetElementIsSet, GETELEMENTISSET, bool, int, element);
DeclareSciCallR1(GetElementAllowsTranslucent, GETELEMENTALLOWSTRANSLUCENT, bool, int, element);

//  Selection, positions and information
DeclareSciCallV0(Undo, UNDO);
DeclareSciCallV0(Redo, REDO);
DeclareSciCallR0(CanUndo, CANUNDO, bool);
DeclareSciCallR0(CanRedo, CANREDO, bool);
DeclareSciCallR0(GetModify, GETMODIFY, bool);
DeclareSciCallR0(CanPaste, CANPASTE, bool);
DeclareSciCallV0(GrabFocus, GRABFOCUS);
DeclareSciCallV1(SetFocus, SETFOCUS, bool, flag);
DeclareSciCallR0(GetFocus, GETFOCUS, bool);
DeclareSciCallR0(GetPasteConvertEndings, GETPASTECONVERTENDINGS, bool);
DeclareSciCallV1(SetOvertype, SETOVERTYPE, bool, overType);
DeclareSciCallR0(GetOverType, GETOVERTYPE, bool);
DeclareSciCallR0(GetReadOnly, GETREADONLY, bool);
DeclareSciCallV1(SetReadOnly, SETREADONLY, bool, flag);
DeclareSciCallR0(GetSelectionHidden, GETSELECTIONHIDDEN, bool);
DeclareSciCallV1(HideSelection, HIDESELECTION, bool, hide);
DeclareSciCallV1(SetStatus, SETSTATUS, int, status);
DeclareSciCallR0(GetStatus, GETSTATUS, int);
DeclareSciCallV1(SetMouseDownCaptures, SETMOUSEDOWNCAPTURES, bool, captures);
DeclareSciCallR0(GetMouseDownCaptures, GETMOUSEDOWNCAPTURES, bool);
DeclareSciCallV1(SetMouseWheelCaptures, SETMOUSEWHEELCAPTURES, bool, captures);
DeclareSciCallR0(GetMouseWheelCaptures, GETMOUSEWHEELCAPTURES, bool);
DeclareSciCallR0(GetCursor, GETCURSOR, int);
DeclareSciCallV1(SetControlCharSymbol, SETCONTROLCHARSYMBOL, int, symbol);
DeclareSciCallR0(GetControlCharSymbol, GETCONTROLCHARSYMBOL, int);

DeclareSciCallV1(SetSelectionLayer, SETSELECTIONLAYER, int, layer);
DeclareSciCallR0(GetSelectionLayer, GETSELECTIONLAYER, int);

DeclareSciCallV1(SetEmptySelection, SETEMPTYSELECTION, DocPos, position);
DeclareSciCallR0(GetCurrentPos, GETCURRENTPOS, DocPos);
DeclareSciCallR0(GetAnchor, GETANCHOR, DocPos);
DeclareSciCallR0(GetSelectionStart, GETSELECTIONSTART, DocPos);
DeclareSciCallR0(GetSelectionEnd, GETSELECTIONEND, DocPos);
DeclareSciCallR1(GetLineSelStartPosition, GETLINESELSTARTPOSITION, DocPos, DocLn, line);
DeclareSciCallR1(GetLineSelEndPosition, GETLINESELENDPOSITION, DocPos, DocLn, line);

DeclareSciCallR2(PositionFromPoint, POSITIONFROMPOINT, DocPos, int, pt_x, int, pt_y);
DeclareSciCallR2(PositionFromPointClose, POSITIONFROMPOINTCLOSE, DocPos, int, pt_x, int, pt_y);
DeclareSciCallR2(CharPositionFromPoint, CHARPOSITIONFROMPOINT, DocPos, int, pt_x, int, pt_y);
DeclareSciCallR2(CharPositionFromPointClose, CHARPOSITIONFROMPOINTCLOSE, DocPos, int, pt_x, int, pt_y);
DeclareSciCallR01(PointXFromPosition, POINTXFROMPOSITION, int, DocPos, position);
DeclareSciCallR01(PointYFromPosition, POINTYFROMPOSITION, int, DocPos, position);

DeclareSciCallR2(WordStartPosition, WORDSTARTPOSITION, DocPos, DocPos, pos, bool, onlyWordChars);
DeclareSciCallR2(WordEndPosition, WORDENDPOSITION, DocPos, DocPos, pos, bool, onlyWordChars);

// Selection colors
DeclareSciCallV2(SetSelFore, SETSELFORE, bool, useSetting, COLORREF, fore);
DeclareSciCallV2(SetSelBack, SETSELBACK, bool, useSetting, COLORREF, back);
DeclareSciCallR0(GetSelAlpha, GETSELALPHA, int);
DeclareSciCallV1(SetSelAlpha, SETSELALPHA, int, alpha);
DeclareSciCallR0(GetSelEOLFilled, GETSELEOLFILLED, bool);
DeclareSciCallV1(SetAdditionalSelFore, SETADDITIONALSELFORE, COLORREF, fore);
DeclareSciCallV1(SetAdditionalSelBack, SETADDITIONALSELBACK, COLORREF, back);
DeclareSciCallV1(SetAdditionalSelAlpha, SETADDITIONALSELALPHA, int, alpha);
DeclareSciCallR0(GetAdditionalSelAlpha, GETADDITIONALSELALPHA, int);
DeclareSciCallV1(SetAdditionalCaretFore, SETADDITIONALCARETFORE, COLORREF, fore);
DeclareSciCallR0(GetAdditionalCaretFore, GETADDITIONALCARETFORE, COLORREF);
DeclareSciCallR0(GetAdditionalCaretsBlink, GETADDITIONALCARETSBLINK, bool);
DeclareSciCallR0(GetAdditionalCaretsVisible, GETADDITIONALCARETSVISIBLE, bool);
DeclareSciCallV2(SetWhitespaceFore, SETWHITESPACEFORE, bool, useSetting, COLORREF, fore);
DeclareSciCallV2(SetWhitespaceBack, SETWHITESPACEBACK, bool, useSetting, COLORREF, back);
DeclareSciCallV1(SetCaretFore, SETCARETFORE, COLORREF, fore);
DeclareSciCallR0(GetCaretFore, GETCARETFORE, COLORREF);

// Rectangular selection with virtual space
DeclareSciCallR0(GetVirtualSpaceOptions, GETVIRTUALSPACEOPTIONS, int);
DeclareSciCallV1(SetVirtualSpaceOptions, SETVIRTUALSPACEOPTIONS, int, options);
DeclareSciCallV1(SetRectangularSelectionModifier, SETRECTANGULARSELECTIONMODIFIER, int, modifier);
DeclareSciCallR0(GetRectangularSelectionModifier, GETRECTANGULARSELECTIONMODIFIER, int);
DeclareSciCallR0(GetRectangularSelectionCaret, GETRECTANGULARSELECTIONCARET, DocPos);
DeclareSciCallV1(SetRectangularSelectionCaret, SETRECTANGULARSELECTIONCARET, DocPos, position);
DeclareSciCallR0(GetRectangularSelectionAnchor, GETRECTANGULARSELECTIONANCHOR, DocPos);
DeclareSciCallV1(SetRectangularSelectionAnchor, SETRECTANGULARSELECTIONANCHOR, DocPos, position);
DeclareSciCallR0(GetRectangularSelectionCaretVirtualSpace, GETRECTANGULARSELECTIONCARETVIRTUALSPACE, DocPos);
DeclareSciCallV1(SetRectangularSelectionCaretVirtualSpace, SETRECTANGULARSELECTIONCARETVIRTUALSPACE, DocPos, position);
DeclareSciCallR0(GetRectangularSelectionAnchorVirtualSpace, GETRECTANGULARSELECTIONANCHORVIRTUALSPACE, DocPos);
DeclareSciCallV1(SetRectangularSelectionAnchorVirtualSpace, SETRECTANGULARSELECTIONANCHORVIRTUALSPACE, DocPos, position);

// Multiselections (Lines of Rectangular selection);
DeclareSciCallV0(ClearSelections, CLEARSELECTIONS);
DeclareSciCallR0(GetSelectionMode, GETSELECTIONMODE, int);
DeclareSciCallV1(SetSelectionMode, SETSELECTIONMODE, int, mode);
DeclareSciCallV1(ChangeSelectionMode, CHANGESELECTIONMODE, int, mode);
DeclareSciCallR0(GetSelections, GETSELECTIONS, DocPosU);
DeclareSciCallV2(SetSelection, SETSELECTION, DocPos, caretPos, DocPos, anchorPos);
DeclareSciCallV1(SetSelectionStart, SETSELECTIONSTART, DocPos, anchorPos);
DeclareSciCallV1(SetSelectionEnd, SETSELECTIONEND, DocPos, caretPos);
DeclareSciCallV2(AddSelection, ADDSELECTION, DocPos, caretPos, DocPos, anchorPos);
DeclareSciCallR0(GetMainSelection, GETMAINSELECTION, DocPosU);
DeclareSciCallV1(SetMainSelection, SETMAINSELECTION, DocPosU, selnum);
DeclareSciCallR1(GetSelectionNCaret, GETSELECTIONNCARET, DocPos, DocPosU, selnum);
DeclareSciCallR1(GetSelectionNAnchor, GETSELECTIONNANCHOR, DocPos, DocPosU, selnum);
DeclareSciCallR1(GetSelectionNCaretVirtualSpace, GETSELECTIONNCARETVIRTUALSPACE, DocPos, DocPosU, selnum);
DeclareSciCallR1(GetSelectionNAnchorVirtualSpace, GETSELECTIONNANCHORVIRTUALSPACE, DocPos, DocPosU, selnum);
DeclareSciCallV2(SetSelectionNCaret, SETSELECTIONNCARET, DocPosU, selnum, DocPos, caretPos);
DeclareSciCallV2(SetSelectionNAnchor, SETSELECTIONNANCHOR, DocPosU, selnum, DocPos, anchorPos);
DeclareSciCallV2(SetSelectionNCaretVirtualSpace, SETSELECTIONNCARETVIRTUALSPACE, DocPosU, selnum, DocPos, position);
DeclareSciCallV2(SetSelectionNAnchorVirtualSpace, SETSELECTIONNANCHORVIRTUALSPACE, DocPosU, selnum, DocPos, position);
DeclareSciCallR1(GetSelectionNStart, GETSELECTIONNSTART, DocPos, DocPosU, selnum);
DeclareSciCallR1(GetSelectionNEnd, GETSELECTIONNEND, DocPos, DocPosU, selnum);
DeclareSciCallR1(GetSelectionNStartVirtualSpace, GETSELECTIONNSTARTVIRTUALSPACE, DocPos, DocPosU, selnum);
DeclareSciCallV2(SetSelectionNStart, SETSELECTIONNSTART, int, selection, DocPos, anchor);
DeclareSciCallV2(SetSelectionNEnd, SETSELECTIONNEND, int, selection, DocPos, caret);
DeclareSciCallR1(GetSelectionNEndVirtualSpace, GETSELECTIONNENDVIRTUALSPACE, DocPos, DocPosU, selnum);
DeclareSciCallV1(DropSelectionN, DROPSELECTIONN, int, selection);
DeclareSciCallV1(SetMoveExtendsSelection, SETMOVEEXTENDSSELECTION, bool, moveExtendsSelection);
DeclareSciCallR0(GetMoveExtendsSelection, GETMOVEEXTENDSSELECTION, bool);
DeclareSciCallR2(SelectionFromPoint, SELECTIONFROMPOINT, int, int, x, int, y);
// Serialized selection
DeclareSciCallV01(SetSelectionSerialized, SETSELECTIONSERIALIZED, const char*, selectionString);
DeclareSciCallR01(GetSelectionSerialized, GETSELECTIONSERIALIZED, DocPos, char*, selectionString);

DeclareSciCallV0(SwapMainAnchorCaret, SWAPMAINANCHORCARET);
DeclareSciCallV0(MultipleSelectAddNext, MULTIPLESELECTADDNEXT);
DeclareSciCallV0(MultipleSelectAddEach, MULTIPLESELECTADDEACH);
DeclareSciCallV0(RotateSelection, ROTATESELECTION);

// Zoom
DeclareSciCallR0(GetZoom, GETZOOM, int);
DeclareSciCallV1(SetZoom, SETZOOM, int, zoom);
DeclareSciCallV0(ZoomIn, ZOOMIN);
DeclareSciCallV0(ZoomOut, ZOOMOUT);

// Keyboard Commands
// The SCI_HOME* commands move the caret to the start of the line,
// while the SCI_VCHOME* commands move the caret to the first non-blank character of the line
// (ie. just after the indentation) unless it is already there; in this case, it acts as SCI_HOME*.
DeclareSciCallV0(Home, HOME);
DeclareSciCallV0(HomeExtend, HOMEEXTEND);
DeclareSciCallV0(VCHome, VCHOME);
DeclareSciCallV0(VCHomeExtend, VCHOMEEXTEND);
DeclareSciCallV0(VCHomeWrap, VCHOMEWRAP);
DeclareSciCallV0(VCHomeWrapExtend, VCHOMEWRAPEXTEND);
DeclareSciCallV0(VCHomeDisplay, VCHOMEDISPLAY);
DeclareSciCallV0(VCHomeDisplayExtend, VCHOMEDISPLAYEXTEND);
DeclareSciCallV0(LineEnd, LINEEND);
DeclareSciCallV0(LineEndExtend, LINEENDEXTEND);
DeclareSciCallV0(LineEndWrap, LINEENDWRAP);
DeclareSciCallV0(LineEndWrapExtend, LINEENDWRAPEXTEND);
DeclareSciCallV0(LineEndDisplay, LINEENDDISPLAY);
DeclareSciCallV0(LineEndDisplayExtend, LINEENDDISPLAYEXTEND);
DeclareSciCallV0(HomeDisplay, HOMEDISPLAY);
DeclareSciCallV0(HomeDisplayExtend, HOMEDISPLAYEXTEND);
DeclareSciCallV0(HomeWrap, HOMEWRAP);
DeclareSciCallV0(HomeWrapExtend, HOMEWRAPEXTEND);
DeclareSciCallV0(NewLine, NEWLINE);
DeclareSciCallV0(FormFeed, FORMFEED);
DeclareSciCallV0(Tab, TAB);
DeclareSciCallV0(BackTab, BACKTAB);
DeclareSciCallV0(LineIndent, LINEINDENT);
DeclareSciCallV0(LineDedent, LINEDEDENT);
DeclareSciCallV0(LineUp, LINEUP);
DeclareSciCallV0(LineUpExtend, LINEUPEXTEND);
DeclareSciCallV0(LineDown, LINEDOWN);
DeclareSciCallV0(LineDownExtend, LINEDOWNEXTEND);
DeclareSciCallV0(LineScrollUp, LINESCROLLUP);
DeclareSciCallV0(LineScrollDown, LINESCROLLDOWN);
DeclareSciCallV0(CharLeft, CHARLEFT);
DeclareSciCallV0(CharLeftExtend, CHARLEFTEXTEND);
DeclareSciCallV0(CharRight, CHARRIGHT);
DeclareSciCallV0(CharRightExtend, CHARRIGHTEXTEND);
DeclareSciCallV0(WordLeft, WORDLEFT);
DeclareSciCallV0(WordLeftExtend, WORDLEFTEXTEND);
DeclareSciCallV0(WordRight, WORDRIGHT);
DeclareSciCallV0(WordRightExtend, WORDRIGHTEXTEND);
DeclareSciCallV0(WordPartLeft, WORDPARTLEFT);
DeclareSciCallV0(WordPartLeftExtend, WORDPARTLEFTEXTEND);
DeclareSciCallV0(WordPartRight, WORDPARTRIGHT);
DeclareSciCallV0(WordPartRightExtend, WORDPARTRIGHTEXTEND);
DeclareSciCallV0(WordLeftEnd, WORDLEFTEND);
DeclareSciCallV0(WordLeftEndExtend, WORDLEFTENDEXTEND);
DeclareSciCallV0(WordRightEnd, WORDRIGHTEND);
DeclareSciCallV0(WordRightEndExtend, WORDRIGHTENDEXTEND);
DeclareSciCallV0(DocumentStart, DOCUMENTSTART);
DeclareSciCallV0(DocumentStartExtend, DOCUMENTSTARTEXTEND);
DeclareSciCallV0(DocumentEnd, DOCUMENTEND);
DeclareSciCallV0(DocumentEndExtend, DOCUMENTENDEXTEND);
DeclareSciCallV0(PageUp, PAGEUP);
DeclareSciCallV0(PageUpExtend, PAGEUPEXTEND);
DeclareSciCallV0(PageDown, PAGEDOWN);
DeclareSciCallV0(PageDownExtend, PAGEDOWNEXTEND);
DeclareSciCallV0(StutteredPageUp, STUTTEREDPAGEUP);
DeclareSciCallV0(StutteredPageUpExtend, STUTTEREDPAGEUPEXTEND);
DeclareSciCallV0(StutteredPageDown, STUTTEREDPAGEDOWN);
DeclareSciCallV0(StutteredPageDownExtend, STUTTEREDPAGEDOWNEXTEND);
DeclareSciCallV0(ParaUp, PARAUP);
DeclareSciCallV0(ParaUpExtend, PARAUPEXTEND);
DeclareSciCallV0(ParaDown, PARADOWN);
DeclareSciCallV0(ParaDownExtend, PARADOWNEXTEND);
// Rectangular extension
DeclareSciCallV0(LineDownRectExtend, LINEDOWNRECTEXTEND);
DeclareSciCallV0(LineUpRectExtend, LINEUPRECTEXTEND);
DeclareSciCallV0(CharLeftRectExtend, CHARLEFTRECTEXTEND);
DeclareSciCallV0(CharRightRectExtend, CHARRIGHTRECTEXTEND);
DeclareSciCallV0(HomeRectExtend, HOMERECTEXTEND);
DeclareSciCallV0(VCHomeRectExtend, VCHOMERECTEXTEND);
DeclareSciCallV0(LineEndRectExtend, LINEENDRECTEXTEND);
DeclareSciCallV0(PageUpRectExtend, PAGEUPRECTEXTEND);
DeclareSciCallV0(PageDownRectExtend, PAGEDOWNRECTEXTEND);
// Delete and line operations
DeclareSciCallV0(DeleteBack, DELETEBACK);
DeclareSciCallV0(DeleteBackNotLine, DELETEBACKNOTLINE);
DeclareSciCallV0(DelWordLeft, DELWORDLEFT);
DeclareSciCallV0(DelWordRight, DELWORDRIGHT);
DeclareSciCallV0(DelWordRightEnd, DELWORDRIGHTEND);
DeclareSciCallV0(DelLineLeft, DELLINELEFT);
DeclareSciCallV0(DelLineRight, DELLINERIGHT);
DeclareSciCallV0(LineDelete, LINEDELETE);
DeclareSciCallV0(LineCut, LINECUT);
DeclareSciCallV0(LineCopy, LINECOPY);
DeclareSciCallV0(LineReverse, LINEREVERSE);
DeclareSciCallV1(LinesSplit, LINESSPLIT, int, pix);
DeclareSciCallV0(LinesJoin, LINESJOIN);
DeclareSciCallV0(EditToggleOverType, EDITTOGGLEOVERTYPE);

DeclareSciCallV2(AssignCmdKey, ASSIGNCMDKEY, size_t, key, unsigned, cmd);
DeclareSciCallV1(ClearCmdKey, CLEARCMDKEY, size_t, keyDefinition);
DeclareSciCallV0(ClearAllCmdKeys, CLEARALLCMDKEYS);

//  Commands
DeclareSciCallV0(LineDuplicate, LINEDUPLICATE);
DeclareSciCallV0(SelectionDuplicate, SELECTIONDUPLICATE);
DeclareSciCallV0(LineTranspose, LINETRANSPOSE);
DeclareSciCallV0(MoveSelectedLinesUp, MOVESELECTEDLINESUP);
DeclareSciCallV0(MoveSelectedLinesDown, MOVESELECTEDLINESDOWN);
DeclareSciCallV0(CutAllowLine, CUTALLOWLINE);
DeclareSciCallR2(FindText, FINDTEXT, DocPos, int, searchFlags, struct Sci_TextToFind*, ft);
DeclareSciCallR2(FindTextFull, FINDTEXTFULL, DocPos, int, flags, struct Sci_TextToFindFull*, text);
DeclareSciCallV0(SearchAnchor, SEARCHANCHOR);
DeclareSciCallR2(SearchNext, SEARCHNEXT, DocPos, int, searchFlags, const char*, text);
DeclareSciCallR2(SearchPrev, SEARCHPREV, DocPos, int, searchFlags, const char*, text);

// Operations
DeclareSciCallV0(Cut, CUT);
DeclareSciCallV0(Copy, COPY);
DeclareSciCallV0(Paste, PASTE);
DeclareSciCallV0(Clear, CLEAR);
DeclareSciCallV0(ClearAll, CLEARALL);
DeclareSciCallV2(CopyRange, COPYRANGE, DocPos, start, DocPos, end);
DeclareSciCallV0(Cancel, CANCEL);
DeclareSciCallV0(CopyAllowLine, COPYALLOWLINE);
DeclareSciCallV2(CopyText, COPYTEXT, DocPos, length, const char*, text);
DeclareSciCallV2(AddStyledText, ADDSTYLEDTEXT, DocPos, length, const char*, c);
DeclareSciCallR0(GetLength, GETLENGTH, DocPos);
DeclareSciCallR1(GetStyleAt, GETSTYLEAT, int, DocPos, pos);
DeclareSciCallR01(GetStyledText, GETSTYLEDTEXT, DocPos, struct Sci_TextRange*, tr);
DeclareSciCallR01(GetStyledTextFull, GETSTYLEDTEXTFULL, DocPos, struct Sci_TextRangeFull*, tr);
DeclareSciCallR01(GetTextRange, GETTEXTRANGE, DocPos, struct Sci_TextRange*, tr);
DeclareSciCallR2(GetText, GETTEXT, DocPos, DocPos, length, const char*, text); // NULL: w/o terminating '\0' (SCI v515)
DeclareSciCallR01(GetTextRangeFull, GETTEXTRANGEFULL, DocPos, struct Sci_TextRangeFull*, textrange);
DeclareSciCallV01(SetText, SETTEXT, const char*, text);
DeclareSciCallV0(Null, NULL);
DeclareSciCallV0(UpperCase, UPPERCASE);
DeclareSciCallV0(LowerCase, LOWERCASE);
DeclareSciCallV2(ReplaceRectangular, REPLACERECTANGULAR, DocPos, length, const char *, text);
DeclareSciCallV01(SetCopySeparator, SETCOPYSEPARATOR, const char*, sep);
DeclareSciCallR01(GetCopySeparator, GETCOPYSEPARATOR, int, char*, separator);
DeclareSciCallV2(SetStylingEx, SETSTYLINGEX, DocPos, length, const char*, styles);
DeclareSciCallR0(GetSearchFlags, GETSEARCHFLAGS, int);
DeclareSciCallV1(SetTargetStart, SETTARGETSTART, DocPos, start);
DeclareSciCallV1(SetTargetEnd, SETTARGETEND, DocPos, end);
DeclareSciCallV1(SetTargetStartVirtualSpace, SETTARGETSTARTVIRTUALSPACE, DocPos, space);
DeclareSciCallR0(GetTargetStartVirtualSpace, GETTARGETSTARTVIRTUALSPACE, DocPos);
DeclareSciCallV1(SetTargetEndVirtualSpace, SETTARGETENDVIRTUALSPACE, DocPos, space);
DeclareSciCallR0(GetTargetEndVirtualSpace, GETTARGETENDVIRTUALSPACE, DocPos);
DeclareSciCallR2(IsRangeWord, ISRANGEWORD, bool, DocPos, start, DocPos, end);
DeclareSciCallR2(CountCodeUnits, COUNTCODEUNITS, DocPos, DocPos, start, DocPos, end);
DeclareSciCallR2(PositionRelativeCodeUnits, POSITIONRELATIVECODEUNITS, DocPos, DocPos, pos, DocPos, relative);
DeclareSciCallR1(TextHeight, TEXTHEIGHT, int, DocLn, line);
DeclareSciCallR2(GetTag, GETTAG, int, int, tagNumber, char*, tagValue);
DeclareSciCallR2(ChangeLexerState, CHANGELEXERSTATE, int, DocPos, start, DocPos, end);
DeclareSciCallR0(GetGapPosition, GETGAPPOSITION, DocPos);
// Line Character Index
DeclareSciCallR0(GetLineCharacterIndex, GETLINECHARACTERINDEX, int);
DeclareSciCallV1(AllocateLineCharacterIndex, ALLOCATELINECHARACTERINDEX, int, lineCharacterIndex);
DeclareSciCallV1(ReleaseLineCharacterIndex, RELEASELINECHARACTERINDEX, int, lineCharacterIndex);
DeclareSciCallR2(LineFromIndexPosition, LINEFROMINDEXPOSITION, DocLn, DocPos, pos, int, lineCharacterIndex);
DeclareSciCallR2(IndexPositionFromLine, INDEXPOSITIONFROMLINE, DocPos, DocLn, line, int, lineCharacterIndex);
// Encoding
DeclareSciCallV1(SetLengthForEncode, SETLENGTHFORENCODE, DocPos, bytes);
DeclareSciCallR2(EncodedFromUTF8, ENCODEDFROMUTF8, DocPos, const char*, utf8, char*, encoded);

//DeclareSciCallR01(TargetAsUTF8, TARGETASUTF8, DocPos, const char*, text);  // WideCharToMultiByteEx(Encoding_SciCP);
// SCI_ENCODEDFROMUTF8 - no need, internal CP is UTF8 always (fixed const for Notepad3);

DeclareSciCallV2(SetSel, SETSEL, DocPos, anchorPos, DocPos, currentPos);
DeclareSciCallV0(SelectAll, SELECTALL);
DeclareSciCallR01(GetSelText, GETSELTEXT, size_t, const char*, text); // NULL: w/o terminating '\0' (SCI v515)
DeclareSciCallV01(ReplaceSel, REPLACESEL, const char*, text);
DeclareSciCallV2(InsertText, INSERTTEXT, DocPos, position, const char*, text);
DeclareSciCallV2(AppendText, APPENDTEXT, DocPos, length, const char*, text);
DeclareSciCallV0(SetSavePoint, SETSAVEPOINT);

DeclareSciCallR0(GetTargetStart, GETTARGETSTART, DocPos);
DeclareSciCallR0(GetTargetEnd, GETTARGETEND, DocPos);
//DeclareSciCallR01(GetTargetText, GETTARGETTEXT, sptr_t, const unsigned char*, text);
DeclareSciCallV0(TargetFromSelection, TARGETFROMSELECTION);
DeclareSciCallV0(TargetWholeDocument, TARGETWHOLEDOCUMENT);
DeclareSciCallV2(SetTargetRange, SETTARGETRANGE, DocPos, start, DocPos, end);
DeclareSciCallR2(ReplaceTarget, REPLACETARGET, DocPos, DocPos, length, const char*, text);
DeclareSciCallR2(ReplaceTargetMinimal, REPLACETARGETMINIMAL, DocPos, DocPos, length, const char*, text);
DeclareSciCallR2(ReplaceTargetRe, REPLACETARGETRE, DocPos, DocPos, length, const char*, text);
DeclareSciCallV2(AddText, ADDTEXT, DocPos, length, const char*, text);
DeclareSciCallV1(SetSearchFlags, SETSEARCHFLAGS, int, flags);
DeclareSciCallR2(SearchInTarget, SEARCHINTARGET, DocPos, DocPos, length, const char*, text);
DeclareSciCallV2(DeleteRange, DELETERANGE, DocPos, start, DocPos, length);

DeclareSciCallV2(ChangeInsertion, CHANGEINSERTION, DocPos, length, const char*, text);

DeclareSciCallV1(SetAnchor, SETANCHOR, DocPos, position);
DeclareSciCallV1(SetCurrentPos, SETCURRENTPOS, DocPos, position);

DeclareSciCallV1(GotoPos, GOTOPOS, DocPos, position);
DeclareSciCallV1(GotoLine, GOTOLINE, DocLn, line);
DeclareSciCallR1(PositionBefore, POSITIONBEFORE, DocPos, DocPos, position);
DeclareSciCallR1(PositionAfter, POSITIONAFTER, DocPos, DocPos, position);
DeclareSciCallR1(GetCharAt, GETCHARAT, char, DocPos, position);
DeclareSciCallR0(GetEOLMode, GETEOLMODE, int);
DeclareSciCallV1(SetEOLMode, SETEOLMODE, int, eolmode);
DeclareSciCallV1(ConvertEOLs, CONVERTEOLS, int, eolmode);

DeclareSciCallR0(GetExtraAscent, GETEXTRAASCENT, int);
DeclareSciCallV1(SetExtraAscent, SETEXTRAASCENT, int, exascent);
DeclareSciCallR0(GetExtraDescent, GETEXTRADESCENT, int);
DeclareSciCallV1(SetExtraDescent, SETEXTRADESCENT, int, exdescent);

DeclareSciCallV0(SetCharsDefault, SETCHARSDEFAULT);
DeclareSciCallV01(SetWordChars, SETWORDCHARS, const char*, chrs);
DeclareSciCallV01(SetWhitespaceChars, SETWHITESPACECHARS, const char*, chrs);
DeclareSciCallV01(SetPunctuationChars, SETPUNCTUATIONCHARS, const char*, chrs);

DeclareSciCallR0(GetLineCount, GETLINECOUNT, DocLn);
DeclareSciCallR0(GetTextLength, GETTEXTLENGTH, DocPos);
DeclareSciCallR1(LineLength, LINELENGTH, DocPos, DocLn, line);
DeclareSciCallR1(LineFromPosition, LINEFROMPOSITION, DocLn, DocPos, position);
DeclareSciCallR1(PositionFromLine, POSITIONFROMLINE, DocPos, DocLn, line);
DeclareSciCallR1(GetLineEndPosition, GETLINEENDPOSITION, DocPos, DocLn, line);
DeclareSciCallR1(GetColumn, GETCOLUMN, DocPos, DocPos, position);
DeclareSciCallR2(FindColumn, FINDCOLUMN, DocPos, DocLn, line, DocPos, column);
DeclareSciCallR2(CountCharacters, COUNTCHARACTERS, DocPos, DocPos, startpos, DocPos, endpos);
DeclareSciCallR2(PositionRelative, POSITIONRELATIVE, DocPos, DocPos, startpos, DocPos, relative);

DeclareSciCallR0(GetDirectPointer, GETDIRECTPOINTER, const char *const);
DeclareSciCallR0(GetCharacterPointer, GETCHARACTERPOINTER, const char *const);
DeclareSciCallR2(GetRangePointer, GETRANGEPOINTER, char *const, DocPos, start, DocPos, length);

DeclareSciCallR2(GetLine, GETLINE, DocPos, DocLn, line, const char*, text);
DeclareSciCallR2(GetCurLine, GETCURLINE, DocPos, unsigned int, length, const char*, text); // NULL: w/o terminating '\0' (SCI v515)

// change history
DeclareSciCallV1(SetChangeHistory, SETCHANGEHISTORY, int, changeHistory);
DeclareSciCallR0(GetChangeHistory, GETCHANGEHISTORY, int);

//=============================================================================

inline DocPos SciCall_GetLine_Safe(DocLn iLine, char* pTxtBuf)
{
    DocPos const iLen = SciCall_GetLine(iLine, pTxtBuf);
    if (pTxtBuf) {
        pTxtBuf[iLen] = '\0';
    }
    return (iLen + 1);
}


//=============================================================================
//
//  CallTip and AutoComplete
//

DeclareSciCallV1(CallTipSetFore, CALLTIPSETFORE, COLORREF, colour);
DeclareSciCallV1(CallTipSetForeHlt, CALLTIPSETFOREHLT, COLORREF, colour);
DeclareSciCallV1(CallTipSetBack, CALLTIPSETBACK, COLORREF, colour);
DeclareSciCallV1(CallTipSetPosition, CALLTIPSETPOSITION, bool, above);
DeclareSciCallR0(CallTipPosStart, CALLTIPPOSSTART, DocPos);
DeclareSciCallV1(CallTipSetPosStart, CALLTIPSETPOSSTART, DocPos, posStart);
DeclareSciCallV2(CallTipShow, CALLTIPSHOW, DocPos, position, const char*, text);
DeclareSciCallV2(CallTipSetHlt, CALLTIPSETHLT, int, beg, int, end);
DeclareSciCallR0(CallTipActive, CALLTIPACTIVE, bool);
DeclareSciCallV0(CallTipCancel, CALLTIPCANCEL);
DeclareSciCallV1(CallTipUseStyle, CALLTIPUSESTYLE, int, tabsize);
DeclareSciCallV1(SetMouseDWellTime, SETMOUSEDWELLTIME, int, millisec);
DeclareSciCallR0(GetMouseDwellTime, GETMOUSEDWELLTIME, int);

DeclareSciCallR0(AutoCActive, AUTOCACTIVE, bool);
DeclareSciCallV0(AutoCComplete, AUTOCCOMPLETE);
DeclareSciCallV0(AutoCCancel, AUTOCCANCEL);
DeclareSciCallR0(AutoCPosStart, AUTOCPOSSTART, DocPos);
DeclareSciCallV01(AutoCStops, AUTOCSTOPS, const char*, characterSet);
DeclareSciCallV1(AutoCSetSeparator, AUTOCSETSEPARATOR, char, separator);
DeclareSciCallR0(AutoCGetSeparator, AUTOCGETSEPARATOR, int);
DeclareSciCallV01(AutoCSelect, AUTOCSELECT, const char*, select);
DeclareSciCallV1(AutoCSetStyle, AUTOCSETSTYLE, int, style);
DeclareSciCallR0(AutoCGetStyle, AUTOCGETSTYLE, int);
DeclareSciCallV1(AutoCSetCancelAtStart, AUTOCSETCANCELATSTART, bool, cancel);
DeclareSciCallR0(AutoCGetCancelAtStart, AUTOCGETCANCELATSTART, bool);
DeclareSciCallV01(AutoCSetFillups, AUTOCSETFILLUPS, const char*, text);
DeclareSciCallV1(AutoCSetChooseSingle, AUTOCSETCHOOSESINGLE, bool, flag);
DeclareSciCallR0(AutoCGetChooseSingle, AUTOCGETCHOOSESINGLE, bool);
DeclareSciCallV1(AutoCSetIgnoreCase, AUTOCSETIGNORECASE, bool, flag);
DeclareSciCallR0(AutoCGetIgnoreCase, AUTOCGETIGNORECASE, bool);
DeclareSciCallV1(AutoCSetAutoHide, AUTOCSETAUTOHIDE, bool, autoHide);
DeclareSciCallR0(AutoCGetAutoHide, AUTOCGETAUTOHIDE, bool);
DeclareSciCallV1(AutoCSetDropRestOfWord, AUTOCSETDROPRESTOFWORD, bool, dropRestOfWord);
DeclareSciCallR0(AutoCGetDropRestOfWord, AUTOCGETDROPRESTOFWORD, bool);
DeclareSciCallV1(AutoCSetTypeSeparator, AUTOCSETTYPESEPARATOR, int, separatorCharacter);
DeclareSciCallR0(AutoCGetTypeSeparator, AUTOCGETTYPESEPARATOR, int);
DeclareSciCallV1(AutoCSetMaxWidth, AUTOCSETMAXWIDTH, int, characterCount);
DeclareSciCallR0(AutoCGetMaxWidth, AUTOCGETMAXWIDTH, int);
DeclareSciCallV1(AutoCSetMaxHeight, AUTOCSETMAXHEIGHT, int, rowCount);
DeclareSciCallR0(AutoCGetMaxHeight, AUTOCGETMAXHEIGHT, int);
DeclareSciCallV1(AutoCSetOptions, AUTOCSETOPTIONS, int, options);
DeclareSciCallR0(AutoCGetOptions, AUTOCGETOPTIONS, int);
DeclareSciCallV1(AutoCSetCaseInsensitiveBehaviour, AUTOCSETCASEINSENSITIVEBEHAVIOUR, int, options);
DeclareSciCallR0(AutoCGetCaseInsensitiveBehaviour, AUTOCGETCASEINSENSITIVEBEHAVIOUR, int);
DeclareSciCallR0(AutoCGetOrder, AUTOCGETORDER, int);
DeclareSciCallV1(AutoCSetOrder, AUTOCSETORDER, int, options);
DeclareSciCallV1(AutoCSetMulti, AUTOCSETMULTI, int, options);
DeclareSciCallR0(AutoCGetMulti, AUTOCGETMULTI, int);
DeclareSciCallR0(AutoCGetCurrent, AUTOCGETCURRENT, int);
DeclareSciCallR01(AutoCGetCurrentText, AUTOCGETCURRENTTEXT, int, char*, text);
DeclareSciCallV1(AutoCSetImageScale, AUTOCSETIMAGESCALE, int, scalePercent);
DeclareSciCallR0(AutoCGetImageScale, AUTOCGETIMAGESCALE, int);
DeclareSciCallV2(AutoCShow, AUTOCSHOW, DocPos, len, const char*, list);
DeclareSciCallV2(UserListShow, USERLISTSHOW, int, listType, const char*, itemList);
DeclareSciCallV2(RegisterImage, REGISTERIMAGE, int, type, const char*, xpmData);
DeclareSciCallV2(RegisterRGBAImage, REGISTERRGBAIMAGE, int, type, const char*, pixels);
DeclareSciCallV1(RGBAImageSetWidth, RGBAIMAGESETWIDTH, int, width);
DeclareSciCallV1(RGBAImageSetHeight, RGBAIMAGESETHEIGHT, int, height);
DeclareSciCallV1(RGBAImageSetScale, RGBAIMAGESETSCALE, int, scalePercent);
DeclareSciCallV0(ClearRegisteredImages, CLEARREGISTEREDIMAGES);


//=============================================================================
//
//  Scrolling and automatic scrolling
//
DeclareSciCallV0(ScrollToStart, SCROLLTOSTART);
DeclareSciCallV0(ScrollToEnd, SCROLLTOEND);
DeclareSciCallV0(ScrollCaret, SCROLLCARET);
DeclareSciCallV0(ChooseCaretX, CHOOSECARETX);
DeclareSciCallV2(LineScroll, LINESCROLL, DocPos, columns, DocLn, lines);
DeclareSciCallV2(ScrollRange, SCROLLRANGE, DocPos, secondaryPos, DocPos, primaryPos);
DeclareSciCallR0(GetScrollWidth, GETSCROLLWIDTH, int);
DeclareSciCallV1(SetScrollWidth, SETSCROLLWIDTH, int, width);
DeclareSciCallR0(GetScrollWidthTracking, GETSCROLLWIDTHTRACKING, bool);
DeclareSciCallV1(SetScrollWidthTracking, SETSCROLLWIDTHTRACKING, bool, srwt);

DeclareSciCallR0(GetEndAtLastLine, GETENDATLASTLINE, bool);
DeclareSciCallV1(SetEndAtLastLine, SETENDATLASTLINE, bool, flag);
DeclareSciCallR0(GetXOffset, GETXOFFSET, int);
DeclareSciCallV1(SetXOffset, SETXOFFSET, int, offset);
DeclareSciCallV2(SetVisiblePolicy, SETVISIBLEPOLICY, int, flags, DocLn, lines);
DeclareSciCallV0(MoveCaretInsideView, MOVECARETINSIDEVIEW);

DeclareSciCallV0(VerticalCentreCaret, VERTICALCENTRECARET);
DeclareSciCallV2(ShowLines, SHOWLINES, DocLn, lnStart, DocLn, lnEnd);
DeclareSciCallV2(HideLines, HIDELINES, DocLn, lineStart, DocLn, lineEnd);
DeclareSciCallR0(GetAllLinesVisible, GETALLLINESVISIBLE, bool);
DeclareSciCallR0(LinesOnScreen, LINESONSCREEN, DocLn);
DeclareSciCallR1(GetLineVisible, GETLINEVISIBLE, bool, DocLn, line);
DeclareSciCallR0(GetFirstVisibleLine, GETFIRSTVISIBLELINE, DocLn);
DeclareSciCallV1(SetFirstVisibleLine, SETFIRSTVISIBLELINE, DocLn, line);
DeclareSciCallV2(ScrollVertical, SCROLLVERTICAL, DocLn, docLine, DocLn, subLine);
DeclareSciCallR1(VisibleFromDocLine, VISIBLEFROMDOCLINE, DocLn, DocLn, line);
DeclareSciCallR1(DocLineFromVisible, DOCLINEFROMVISIBLE, DocLn, DocLn, line);

DeclareSciCallR0(GetHScrollBar, GETHSCROLLBAR, bool);
DeclareSciCallV1(SetHScrollbar, SETHSCROLLBAR, bool, visible);
DeclareSciCallR0(GetVScrollBar, GETVSCROLLBAR, bool);
DeclareSciCallV1(SetVScrollbar, SETVSCROLLBAR, bool, visible);


//=============================================================================
//
//  Folding
//
DeclareSciCallV2(SetFoldLevel, SETFOLDLEVEL, DocLn, line, int, level);
DeclareSciCallR1(GetFoldLevel, GETFOLDLEVEL, int, DocLn, line);
DeclareSciCallV1(SetFoldFlags, SETFOLDFLAGS, int, flags);
DeclareSciCallV1(FoldDisplayTextSetStyle, FOLDDISPLAYTEXTSETSTYLE, int, flags);
DeclareSciCallR1(GetFoldParent, GETFOLDPARENT, DocLn, DocLn, line);
DeclareSciCallR2(GetLastChild, GETLASTCHILD, DocLn, DocLn, line, int, level);
DeclareSciCallV2(SetFoldExpanded, SETFOLDEXPANDED, DocLn, line, bool, expanded);
DeclareSciCallR1(GetFoldExpanded, GETFOLDEXPANDED, bool, DocLn, line);
DeclareSciCallV1(ToggleFold, TOGGLEFOLD, DocLn, line);
DeclareSciCallV2(ToggleFoldShowText, TOGGLEFOLDSHOWTEXT, DocLn, line, const char*, text);
DeclareSciCallV2(FoldLine, FOLDLINE, DocLn, line, int, action);
DeclareSciCallV2(FoldChildren, FOLDCHILDREN, DocLn, line, int, action);
DeclareSciCallV2(ExpandChildren, EXPANDCHILDREN, DocLn, line, int, level);
DeclareSciCallV1(FoldAll, FOLDALL, int, flags);
DeclareSciCallR0(FoldDisplayTextGetStyle, FOLDDISPLAYTEXTGETSTYLE, int);
DeclareSciCallR1(ContractedFoldNext, CONTRACTEDFOLDNEXT, DocLn, DocLn, lineStart);
DeclareSciCallR01(GetDefaultFoldDisplayText, GETDEFAULTFOLDDISPLAYTEXT, int, char*, text);
DeclareSciCallV1(EnsureVisible, ENSUREVISIBLE, DocLn, line);
DeclareSciCallV1(EnsureVisibleEnforcePolicy, ENSUREVISIBLEENFORCEPOLICY, DocLn, line);


//=============================================================================
//
//  Line State (parser internals)
//
//~DeclareSciCallV2(SetLineState, SETLINESTATE, DocLn, line, int, state);
//~DeclareSciCallR1(GetLineState, GETLINESTATE, int, DocLn, line);
//~DeclareSciCallR0(GetMaxLineState, GETMAXLINESTATE, DocLn);


//=============================================================================
//
//  Style definition
//
DeclareSciCallR0(GetLexer, GETLEXER, int);
DeclareSciCallV01(SetILexer, SETILEXER, void*, lexerPtr); // ILexer5*

DeclareSciCallR0(GetIdleStyling, GETIDLESTYLING, int);
DeclareSciCallV1(SetIdleStyling, SETIDLESTYLING, int, idlestyle);

DeclareSciCallV0(StyleClearAll, STYLECLEARALL);
DeclareSciCallV0(ClearDocumentStyle, CLEARDOCUMENTSTYLE);
DeclareSciCallV0(StyleResetDefault, STYLERESETDEFAULT);
DeclareSciCallV2(StyleSetVisible, STYLESETVISIBLE, int, style, bool, visible);
DeclareSciCallV2(StyleSetChangeable, STYLESETCHANGEABLE, int, style, bool, ro);
DeclareSciCallR1(StyleGetFore, STYLEGETFORE, COLORREF, int, style);
DeclareSciCallV2(StyleSetFore, STYLESETFORE, int, style, COLORREF, rgb);
DeclareSciCallR1(StyleGetBack, STYLEGETBACK, COLORREF, int, style);
DeclareSciCallV2(StyleSetBack, STYLESETBACK, int, style, COLORREF, rgb);
DeclareSciCallR1(GetStyleIndexAt, GETSTYLEINDEXAT, int, DocPos, position);
DeclareSciCallV2(SetStyling, SETSTYLING, DocPos, length, int, style);
DeclareSciCallV1(StartStyling, STARTSTYLING, DocPos, position);
DeclareSciCallR0(GetEndStyled, GETENDSTYLED, DocPos);

DeclareSciCallR1(StyleGetBold, STYLEGETBOLD, bool, int, style);
DeclareSciCallR1(StyleGetItalic, STYLEGETITALIC, bool, int, style);
DeclareSciCallR1(StyleGetSize, STYLEGETSIZE, int, int, style);
DeclareSciCallV2(StyleSetSize, STYLESETSIZE, int, style, int, sizePoints);
DeclareSciCallV2(StyleSetSizeFractional, STYLESETSIZEFRACTIONAL, int, style, int, sizeHundredthPoints);
DeclareSciCallR1(StyleGetSizeFractional, STYLEGETSIZEFRACTIONAL, int, int, style);
DeclareSciCallR1(StyleGetWeight, STYLEGETWEIGHT, int, int, style);
DeclareSciCallV2(StyleSetEOLFilled, STYLESETEOLFILLED, int, style, bool, eolFilled);
DeclareSciCallR1(StyleGetEOLFilled, STYLEGETEOLFILLED, bool, int, style);
DeclareSciCallV2(StyleSetUnderline, STYLESETUNDERLINE, int, style, bool, underline);
DeclareSciCallR1(StyleGetUnderline, STYLEGETUNDERLINE, bool, int, style);
DeclareSciCallV2(StyleSetCase, STYLESETCASE, int, style, int, caseVisible);
DeclareSciCallR1(StyleGetCase, STYLEGETCASE, int, int, style);
DeclareSciCallR1(StyleGetCharacterSet, STYLEGETCHARACTERSET, int, int, style);
DeclareSciCallR1(StyleGetVisible, STYLEGETVISIBLE, bool, int, style);
DeclareSciCallR1(StyleGetChangeable, STYLEGETCHANGEABLE, bool, int, style);
DeclareSciCallR1(StyleGetHotspot, STYLEGETHOTSPOT, bool, int, style);
DeclareSciCallV2(StyleSetHotspot, STYLESETHOTSPOT, int, style, bool, hotspot);
DeclareSciCallV2(StyleSetCheckMonospaced, STYLESETCHECKMONOSPACED, int, style, bool, checkMonospaced);
DeclareSciCallR1(StyleGetCheckMonospaced, STYLEGETCHECKMONOSPACED, bool, int, style);
DeclareSciCallV2(StyleSetStretch, STYLESETSTRETCH, int, style, int, stretch);
DeclareSciCallR1(StyleGetStretch, STYLEGETSTRETCH, int, int, style);
DeclareSciCallV2(StyleSetInvisibleRepresentation, STYLESETINVISIBLEREPRESENTATION, int, style, const char*, representation);
DeclareSciCallR2(StyleGetInvisibleRepresentation, STYLEGETINVISIBLEREPRESENTATION, int, int, style, char*, representation);
DeclareSciCallV2(StyleSetStrike, STYLESETSTRIKE, int, style, bool, strike);
DeclareSciCallR1(StyleGetStrike, STYLEGETSTRIKE, bool, int, style);

DeclareSciCallV2(SetHotspotActiveFore, SETHOTSPOTACTIVEFORE, bool, useSetting, COLORREF, fore);
DeclareSciCallR0(GetHotspotActiveFore, GETHOTSPOTACTIVEFORE, COLORREF);
DeclareSciCallV2(SetHotspotActiveBack, SETHOTSPOTACTIVEBACK, bool, useSetting, COLORREF, back);
DeclareSciCallR0(GetHotspotActiveBack, GETHOTSPOTACTIVEBACK, COLORREF);
DeclareSciCallV1(SetHotspotActiveUnderline, SETHOTSPOTACTIVEUNDERLINE, bool, underline);
DeclareSciCallR0(GetHotspotActiveUnderline, GETHOTSPOTACTIVEUNDERLINE, bool);
DeclareSciCallV1(SetHotspotSingleLine, SETHOTSPOTSINGLELINE, bool, singleline);
DeclareSciCallR0(GetHotspotSingleLine, GETHOTSPOTSINGLELINE, bool);

DeclareSciCallR0(GetViewWS, GETVIEWWS, int);
DeclareSciCallV1(SetViewWS, SETVIEWWS, int, wspc);
DeclareSciCallR0(GetViewEOL, GETVIEWEOL, bool);
DeclareSciCallV1(SetViewEOL, SETVIEWEOL, bool, eols);

DeclareSciCallR2(StyleGetFont, STYLEGETFONT, int, int, style, char*, fontname);
DeclareSciCallV2(StyleSetFont, STYLESETFONT, int, style, const char*, fontname);
DeclareSciCallV2(StyleSetWeight, STYLESETWEIGHT, int, style, int, weight);
DeclareSciCallV2(StyleSetBold, STYLESETBOLD, int, style, bool, bold);
DeclareSciCallV2(StyleSetItalic, STYLESETITALIC, int, style, bool, italic);
DeclareSciCallV1(SetFontQuality, SETFONTQUALITY, int, qual);
DeclareSciCallV01(SetFontLocale, SETFONTLOCALE, const char*, localeName);

DeclareSciCallV1(SetSelEOLFilled, SETSELEOLFILLED, bool, filled);
DeclareSciCallV1(SetWhiteSpaceSize, SETWHITESPACESIZE, int, size);
DeclareSciCallR0(GetWhiteSpaceSize, GETWHITESPACESIZE, int);


//=============================================================================
//
// Indentation Guides and Wraping
//
DeclareSciCallV1(SetWrapMode, SETWRAPMODE, int, mode);
DeclareSciCallR0(GetWrapMode, GETWRAPMODE, int);
DeclareSciCallV1(SetWrapIndentMode, SETWRAPINDENTMODE, int, mode);
DeclareSciCallR0(GetWrapIndentMode, GETWRAPINDENTMODE, int);
DeclareSciCallR0(GetWrapStartIndent, GETWRAPSTARTINDENT, int);
DeclareSciCallV1(SetWrapStartIndent, SETWRAPSTARTINDENT, int, mode);
DeclareSciCallR0(GetWrapVisualFlags, GETWRAPVISUALFLAGS, int);
DeclareSciCallV1(SetWrapVisualFlags, SETWRAPVISUALFLAGS, int, opts);
DeclareSciCallR0(GetWrapVisualFlagsLocation, GETWRAPVISUALFLAGSLOCATION, int);
DeclareSciCallV1(SetWrapVisualFlagsLocation, SETWRAPVISUALFLAGSLOCATION, int, opts);
DeclareSciCallR1(WrapCount, WRAPCOUNT, DocLn, DocLn, line);
DeclareSciCallR0(GetIndentationGuides, GETINDENTATIONGUIDES, int);

DeclareSciCallV1(SetEdgeMode, SETEDGEMODE, int, mode);
DeclareSciCallR0(GetEdgeMode, GETEDGEMODE, int);
DeclareSciCallV1(SetEdgeColumn, SETEDGECOLUMN, int, column);
DeclareSciCallR0(GetEdgeColumn, GETEDGECOLUMN, int);
DeclareSciCallV1(SetEdgeColour, SETEDGECOLOUR, int, colour);
DeclareSciCallR0(GetEdgeColour, GETEDGECOLOUR, int);
DeclareSciCallV2(MultiEdgeAddLine, MULTIEDGEADDLINE, int, column, int, colour);
DeclareSciCallV0(MultiEdgeClearAll, MULTIEDGECLEARALL);
DeclareSciCallR1(GetMultiEdgeColumn, GETMULTIEDGECOLUMN, DocPos, int, which);

DeclareSciCallV1(SetTabWidth, SETTABWIDTH, int, width);
DeclareSciCallR0(GetTabWidth, GETTABWIDTH, int);

DeclareSciCallV1(SetIndent, SETINDENT, int, width);
DeclareSciCallR0(GetIndent, GETINDENT, int);
DeclareSciCallV1(SetUseTabs, SETUSETABS, bool, use);
DeclareSciCallR0(GetUseTabs, GETUSETABS, bool);
DeclareSciCallV1(SetTabIndents, SETTABINDENTS, bool, indents);
DeclareSciCallR0(GetTabIndents, GETTABINDENTS, bool);
DeclareSciCallV1(SetBackSpaceUnIndents, SETBACKSPACEUNINDENTS, bool, unindents);
DeclareSciCallR0(GetBackSpaceUnIndents, GETBACKSPACEUNINDENTS, bool);

DeclareSciCallV2(SetLineIndentation, SETLINEINDENTATION, DocLn, line, DocPos, pos);
DeclareSciCallR1(GetLineIndentation, GETLINEINDENTATION, int, DocLn, line);
DeclareSciCallR1(GetLineIndentPosition, GETLINEINDENTPOSITION, DocPos, DocLn, line);

DeclareSciCallV1(SetIndentationGuides, SETINDENTATIONGUIDES, int, iview);
DeclareSciCallV1(SetHighLightGuide, SETHIGHLIGHTGUIDE, int, column);
DeclareSciCallR0(GetHighlightGuide, GETHIGHLIGHTGUIDE, DocPos);
// Tab Stops
DeclareSciCallV1(ClearTabStops, CLEARTABSTOPS, DocLn, line);
DeclareSciCallV2(AddTabStop, ADDTABSTOP, DocLn, line, int, x);
DeclareSciCallR2(GetNextTabStop, GETNEXTTABSTOP, int, DocLn, line, int, x);

DeclareSciCallR1(BraceMatch, BRACEMATCH, DocPos, DocPos, pos);
DeclareSciCallR2(BraceMatchNext, BRACEMATCHNEXT, DocPos, DocPos, pos, DocPos, posStart);
DeclareSciCallV2(BraceHighLight, BRACEHIGHLIGHT, DocPos, pos1, DocPos, pos2);
DeclareSciCallV1(BraceBadLight, BRACEBADLIGHT, DocPos, pos);
DeclareSciCallV2(BraceHighLightIndicator, BRACEHIGHLIGHTINDICATOR, bool, use, int, indic);
DeclareSciCallV2(BraceBadLightIndicator, BRACEBADLIGHTINDICATOR, bool, use, int, indic);


//=============================================================================
//
//  Margins
//
DeclareSciCallR0(GetMarginOptions, GETMARGINOPTIONS, int);
DeclareSciCallV1(SetMarginOptions, SETMARGINOPTIONS, int, options);
DeclareSciCallR1(GetMarginTypeN, GETMARGINTYPEN, int, int, margin);
DeclareSciCallV2(SetMarginTypeN, SETMARGINTYPEN, uintptr_t, margin, int, type);
DeclareSciCallR1(GetMarginWidthN, GETMARGINWIDTHN, int, uintptr_t, margin);
DeclareSciCallV2(SetMarginWidthN, SETMARGINWIDTHN, uintptr_t, margin, int, pixelWidth);
DeclareSciCallR1(GetMarginMaskN, GETMARGINMASKN, int, int, margin);
DeclareSciCallV2(SetMarginMaskN, SETMARGINMASKN, uintptr_t, margin, int, mask);
DeclareSciCallR1(GetMarginSensitiveN, GETMARGINSENSITIVEN, bool, int, margin);
DeclareSciCallV2(SetMarginSensitiveN, SETMARGINSENSITIVEN, uintptr_t, margin, bool, sensitive);
DeclareSciCallR1(GetMarginCursorN, GETMARGINCURSORN, int, int, margin);
DeclareSciCallV2(SetMarginCursorN, SETMARGINCURSORN, uintptr_t, margin, int, cursor);
DeclareSciCallR1(GetMarginBackN, GETMARGINBACKN, COLORREF, int, margin);
DeclareSciCallV2(SetMarginBackN, SETMARGINBACKN, uintptr_t, margin, COLORREF, colour);
DeclareSciCallV2(SetFoldMarginColour, SETFOLDMARGINCOLOUR, bool, useSetting, COLORREF, colour);
DeclareSciCallV2(SetFoldMarginHiColour, SETFOLDMARGINHICOLOUR, bool, useSetting, COLORREF, colour);
DeclareSciCallV01(SetDefaultFoldDisplayText, SETDEFAULTFOLDDISPLAYTEXT, const char*, text);
DeclareSciCallR0(GetMarginLeft, GETMARGINLEFT, int);
DeclareSciCallV01(SetMarginLeft, SETMARGINLEFT, int, width);
DeclareSciCallR0(GetMarginRight, GETMARGINRIGHT, int);
DeclareSciCallV01(SetMarginRight, SETMARGINRIGHT, int, width);

DeclareSciCallR2(TextWidth, TEXTWIDTH, int, int, styleNumber, const char *, text);

//=============================================================================
//
//  Markers
//
DeclareSciCallR1(MarkerGet, MARKERGET, int, DocLn, line);
DeclareSciCallV2(MarkerDefine, MARKERDEFINE, int, markerID, int, markerSymbols);
DeclareSciCallV2(MarkerSetFore, MARKERSETFORE, int, markerID, COLORREF, colour);
DeclareSciCallV2(MarkerSetForeTranslucent, MARKERSETFORETRANSLUCENT, int, markerID, COLORALPHAREF, colouralpha);
DeclareSciCallV2(MarkerSetBack, MARKERSETBACK, int, markerID, COLORREF, colour);
DeclareSciCallV2(MarkerSetBackTranslucent, MARKERSETBACKTRANSLUCENT, int, markerID, COLORALPHAREF, colouralpha);
DeclareSciCallV2(MarkerSetBackSelected, MARKERSETBACKSELECTED, int, markerID, COLORREF, colour);
DeclareSciCallV2(MarkerSetBackSelectedTranslucent, MARKERSETBACKSELECTEDTRANSLUCENT, int, markerID, COLORALPHAREF, colouralpha);
DeclareSciCallV2(MarkerSetStrokeWidth, MARKERSETSTROKEWIDTH, int, markerID, int, hundredths);
DeclareSciCallV1(MarkerEnableHighlight, MARKERENABLEHIGHLIGHT, bool, flag);
DeclareSciCallR1(MarkerGetLayer, MARKERGETLAYER, int, int, markerNumber);
DeclareSciCallV2(MarkerSetLayer, MARKERSETLAYER, int, markerID, int, layer);
DeclareSciCallV2(MarkerSetAlpha, MARKERSETALPHA, int, markerID, int, alpha);
DeclareSciCallR1(MarkerSymbolDefined, MARKERSYMBOLDEFINED, int, int, markerNumber);
DeclareSciCallR1(MarkerLineFromHandle, MARKERLINEFROMHANDLE, DocLn, int, markerHandle);
DeclareSciCallV1(MarkerDeleteHandle, MARKERDELETEHANDLE, int, markerHandle);
DeclareSciCallV2(MarkerDefinePixmap, MARKERDEFINEPIXMAP, int, markerNumber, const char*, pixmap);
DeclareSciCallV2(MarkerDefineRGBAImage, MARKERDEFINERGBAIMAGE, int, markerNumber, const char*, pixels);
DeclareSciCallR2(MarkerAdd, MARKERADD, int, DocLn, line, int, markerID);
DeclareSciCallV2(MarkerAddSet, MARKERADDSET, DocLn, line, int, markerMask);
DeclareSciCallV2(MarkerDelete, MARKERDELETE, DocLn, line, int, markerID);
DeclareSciCallV1(MarkerDeleteAll, MARKERDELETEALL, int, markerID);
DeclareSciCallR2(MarkerNext, MARKERNEXT, DocLn, DocLn, start, int, markerMask);
DeclareSciCallR2(MarkerPrevious, MARKERPREVIOUS, DocLn, DocLn, start, int, markerMask);
DeclareSciCallR2(MarkerNumberFromLine, MARKERNUMBERFROMLINE, int, DocLn, line, int, which);
DeclareSciCallR2(MarkerHandleFromLine, MARKERHANDLEFROMLINE, int, DocLn, line, int, which);

//=============================================================================
//
//  Indicators
//
DeclareSciCallV2(IndicSetStyle, INDICSETSTYLE, int, indicID, int, style);
DeclareSciCallR1(IndicGetStyle, INDICGETSTYLE, int, int, indicID);
DeclareSciCallV2(IndicSetFore, INDICSETFORE, int, indicID, COLORREF, colour);
DeclareSciCallR1(IndicGetFore, INDICGETFORE, COLORREF, int, indicID);
DeclareSciCallV2(IndicSetStrokeWidth, INDICSETSTROKEWIDTH, int, indicID, int, hundredths);
DeclareSciCallR1(IndicGetStrokeWidth, INDICGETSTROKEWIDTH, int, int, indicID);
DeclareSciCallV2(IndicSetUnder, INDICSETUNDER, int, indicID, bool, under);
DeclareSciCallR1(IndicGetUnder, INDICGETUNDER, bool, int, indicID);
DeclareSciCallV2(IndicSetHoverStyle, INDICSETHOVERSTYLE, int, indicID, int, style);
DeclareSciCallR1(IndicGetHoverStyle, INDICGETHOVERSTYLE, int, int, indicator);
DeclareSciCallV2(IndicSetHoverFore, INDICSETHOVERFORE, int, indicID, COLORREF, colour);
DeclareSciCallR1(IndicGetHoverFore, INDICGETHOVERFORE, COLORREF, int, indicator);
DeclareSciCallV2(IndicSetAlpha, INDICSETALPHA, int, indicID, int, alpha);
DeclareSciCallR1(IndicGetAlpha, INDICGETALPHA, int, int, indicator);
DeclareSciCallV2(IndicSetOutlineAlpha, INDICSETOUTLINEALPHA, int, indicID, int, alpha);
DeclareSciCallR1(IndicGetOutlineAlpha, INDICGETOUTLINEALPHA, int, int, indicator);
DeclareSciCallV2(IndicSetFlags, INDICSETFLAGS, int, indicator, int, flags);
DeclareSciCallR1(IndicGetFlags, INDICGETFLAGS, int, int, indicator);
DeclareSciCallV1(SetIndicatorCurrent, SETINDICATORCURRENT, int, indicID);
DeclareSciCallR0(GetIndicatorCurrent, GETINDICATORCURRENT, int);
DeclareSciCallV1(SetIndicatorValue, SETINDICATORVALUE, int, value);
DeclareSciCallR0(GetIndicatorValue, GETINDICATORVALUE, int);
DeclareSciCallV2(IndicatorFillRange, INDICATORFILLRANGE, DocPos, position, DocPos, length);
DeclareSciCallV2(IndicatorClearRange, INDICATORCLEARRANGE, DocPos, position, DocPos, length);
DeclareSciCallR1(IndicatorAllOnFor, INDICATORALLONFOR, int, DocPos, pos);
DeclareSciCallR2(IndicatorValueAt, INDICATORVALUEAT, int, int, indicID, DocPos, position);
DeclareSciCallR2(IndicatorStart, INDICATORSTART, int, int, indicID, DocPos, position);
DeclareSciCallR2(IndicatorEnd, INDICATOREND, int, int, indicID, DocPos, position);
// macOS Find Indicator
DeclareSciCallV2(FindIndicatorShow, FINDINDICATORSHOW, DocPos, start, DocPos, end);
DeclareSciCallV2(FindIndicatorFlash, FINDINDICATORFLASH, DocPos, start, DocPos, end);
DeclareSciCallV0(FindIndicatorHide, FINDINDICATORHIDE);


//=============================================================================
//
//  Lexer
//
DeclareSciCallV2(SetProperty, SETPROPERTY, const char*, key, const char*, value);
DeclareSciCallR2(GetProperty, GETPROPERTY, int, const char*, key, char*, value);
DeclareSciCallR2(GetPropertyExpanded, GETPROPERTYEXPANDED, int, const char*, key, char*, value);
DeclareSciCallR2(GetPropertyInt, GETPROPERTYINT, int, const char*, key, int, defaultValue);
DeclareSciCallV2(SetKeywords, SETKEYWORDS, int, keywordset, const char*, keywords);
DeclareSciCallV2(Colourise, COLOURISE, DocPos, startPos, DocPos, endPos);
DeclareSciCallR01(GetLexerLanguage, GETLEXERLANGUAGE, int, char*, language);
DeclareSciCallR2(PrivateLexerCall, PRIVATELEXERCALL, sptr_t, int, operation, sptr_t, pointer);
DeclareSciCallR01(PropertyNames, PROPERTYNAMES, int, char*, names);
DeclareSciCallR1(PropertyType, PROPERTYTYPE, int, const char*, name);
DeclareSciCallR2(DescribeProperty, DESCRIBEPROPERTY, int, const char*, name, char*, description);
DeclareSciCallR01(DescribeKeyWordSets, DESCRIBEKEYWORDSETS, int, char*, descriptions);
DeclareSciCallR0(GetLineEndTypesSupported, GETLINEENDTYPESSUPPORTED, int);


//=============================================================================
//
//  Cursor, Caret
//
DeclareSciCallV1(SetCursor, SETCURSOR, int, flags);

DeclareSciCallR0(GetCaretStyle, GETCARETSTYLE, int);
DeclareSciCallV1(SetCaretStyle, SETCARETSTYLE, int, style);
DeclareSciCallR0(GetCaretWidth, GETCARETWIDTH, int);
DeclareSciCallV1(SetCaretWidth, SETCARETWIDTH, int, pixel);
DeclareSciCallR0(GetCaretPeriod, GETCARETPERIOD, int);
DeclareSciCallV1(SetCaretPeriod, SETCARETPERIOD, int, msec);
DeclareSciCallV1(SetAdditionalCaretsBlink, SETADDITIONALCARETSBLINK, bool, flag);
DeclareSciCallV1(SetAdditionalCaretsVisible, SETADDITIONALCARETSVISIBLE, bool, flag);

//=============================================================================
//
//  Undo/Redo Stack
//
DeclareSciCallV0(EmptyUndoBuffer, EMPTYUNDOBUFFER);
DeclareSciCallV0(BeginUndoAction, BEGINUNDOACTION);
DeclareSciCallV2(AddUndoAction, ADDUNDOACTION, int, token, int, flags);
DeclareSciCallV0(EndUndoAction, ENDUNDOACTION);
DeclareSciCallR0(GetUndoCollection, GETUNDOCOLLECTION, bool);
DeclareSciCallV1(SetUndoCollection, SETUNDOCOLLECTION, bool, bCollectUndo);
DeclareSciCallR0(GetUndoSequence, GETUNDOSEQUENCE, int);
DeclareSciCallR0(GetUndoActions, GETUNDOACTIONS, int);
DeclareSciCallV1(SetUndoSavePoint, SETUNDOSAVEPOINT, int, action);
DeclareSciCallR0(GetUndoSavePoint, GETUNDOSAVEPOINT, int);
DeclareSciCallV1(SetUndoDetach, SETUNDODETACH, int, action);
DeclareSciCallR0(GetUndoDetach, GETUNDODETACH, int);
DeclareSciCallV1(SetUndoTentative, SETUNDOTENTATIVE, int, action);
DeclareSciCallR0(GetUndoTentative, GETUNDOTENTATIVE, int);
DeclareSciCallV1(SetUndoCurrent, SETUNDOCURRENT, int, action);
DeclareSciCallR0(GetUndoCurrent, GETUNDOCURRENT, int);
DeclareSciCallV2(PushUndoActionType, PUSHUNDOACTIONTYPE, int, type, DocPos, pos);
DeclareSciCallV2(ChangeLastUndoActionText, CHANGELASTUNDOACTIONTEXT, DocPos, length, const char*, text);
DeclareSciCallR1(GetUndoActionType, GETUNDOACTIONTYPE, int, int, action);
DeclareSciCallR1(GetUndoActionPosition, GETUNDOACTIONPOSITION, DocPos, int, action);
DeclareSciCallR2(GetUndoActionText, GETUNDOACTIONTEXT, int, int, action, char*, text);
DeclareSciCallV1(SetUndoSelectionHistory, SETUNDOSELECTIONHISTORY, int, option);
DeclareSciCallR0(GetUndoSelectionHistory, GETUNDOSELECTIONHISTORY, int);


//=============================================================================
//
//  IME
//
DeclareSciCallR0(GetIMEInteraction, GETIMEINTERACTION, int);
DeclareSciCallV1(SetIMEInteraction, SETIMEINTERACTION, int, interact);
DeclareSciCallR0(IsIMEOpen, ISIMEOPEN, bool);
DeclareSciCallR0(IsIMEModeCJK, ISIMEMODECJK, bool);


//=============================================================================
//
//  Annotations
//
DeclareSciCallV2(AnnotationSetText, ANNOTATIONSETTEXT, DocLn, line, const char*, text);
DeclareSciCallR2(AnnotationGetText, ANNOTATIONGETTEXT, int, DocLn, line, char*, text);
DeclareSciCallV2(AnnotationSetStyle, ANNOTATIONSETSTYLE, DocLn, line, int, style);
DeclareSciCallR1(AnnotationGetStyle, ANNOTATIONGETSTYLE, int, DocLn, line);
DeclareSciCallV2(AnnotationSetStyles, ANNOTATIONSETSTYLES, DocLn, line, const char*, styles);
DeclareSciCallR2(AnnotationGetStyles, ANNOTATIONGETSTYLES, int, DocLn, line, char*, styles);
DeclareSciCallR1(AnnotationGetLines, ANNOTATIONGETLINES, int, DocLn, line);
DeclareSciCallV0(AnnotationClearAll, ANNOTATIONCLEARALL);
DeclareSciCallV1(AnnotationSetVisible, ANNOTATIONSETVISIBLE, int, visible);
DeclareSciCallR0(AnnotationGetVisible, ANNOTATIONGETVISIBLE, int);
DeclareSciCallV1(AnnotationSetStyleOffset, ANNOTATIONSETSTYLEOFFSET, int, style);
DeclareSciCallR0(AnnotationGetStyleOffset, ANNOTATIONGETSTYLEOFFSET, int);


//=============================================================================
//
//  EOL Annotations
//
DeclareSciCallV2(EOLAnnotationSetText, EOLANNOTATIONSETTEXT, DocLn, line, const char*, text);
DeclareSciCallR2(EOLAnnotationGetText, EOLANNOTATIONGETTEXT, int, DocLn, line, char*, text);
DeclareSciCallV2(EOLAnnotationSetStyle, EOLANNOTATIONSETSTYLE, DocLn, line, int, style);
DeclareSciCallR1(EOLAnnotationGetStyle, EOLANNOTATIONGETSTYLE, int, DocLn, line);
DeclareSciCallV0(EOLAnnotationClearAll, EOLANNOTATIONCLEARALL);
DeclareSciCallV1(EOLAnnotationSetVisible, EOLANNOTATIONSETVISIBLE, int, visible);
DeclareSciCallR0(EOLAnnotationGetVisible, EOLANNOTATIONGETVISIBLE, int);
DeclareSciCallV1(EOLAnnotationSetStyleOffset, EOLANNOTATIONSETSTYLEOFFSET, int, style);
DeclareSciCallR0(EOLAnnotationGetStyleOffset, EOLANNOTATIONGETSTYLEOFFSET, int);


//=============================================================================
//
//  Margin Text
//
DeclareSciCallV2(MarginSetText, MARGINSETTEXT, DocLn, line, const char*, text);
DeclareSciCallR2(MarginGetText, MARGINGETTEXT, int, DocLn, line, char*, text);
DeclareSciCallV2(MarginSetStyle, MARGINSETSTYLE, DocLn, line, int, style);
DeclareSciCallR1(MarginGetStyle, MARGINGETSTYLE, int, DocLn, line);
DeclareSciCallV2(MarginSetStyles, MARGINSETSTYLES, DocLn, line, const char*, styles);
DeclareSciCallR2(MarginGetStyles, MARGINGETSTYLES, int, DocLn, line, char*, styles);
DeclareSciCallV0(MarginTextClearAll, MARGINTEXTCLEARALL);
DeclareSciCallV1(MarginSetStyleOffset, MARGINSETSTYLEOFFSET, int, style);
DeclareSciCallR0(MarginGetStyleOffset, MARGINGETSTYLEOFFSET, int);


//=============================================================================
//
//  Substyles
//
DeclareSciCallR2(AllocateSubStyles, ALLOCATESUBSTYLES, int, int, styleBase, int, numberStyles);
DeclareSciCallR1(GetSubStylesStart, GETSUBSTYLESSTART, int, int, styleBase);
DeclareSciCallR1(GetSubStylesLength, GETSUBSTYLESLENGTH, int, int, styleBase);
DeclareSciCallV0(FreeSubStyles, FREESUBSTYLES);
DeclareSciCallV2(SetIdentifiers, SETIDENTIFIERS, int, style, const char*, identifiers);
DeclareSciCallR0(DistanceToSecondaryStyles, DISTANCETOSECONDARYSTYLES, int);
DeclareSciCallR01(GetSubStyleBases, GETSUBSTYLEBASES, int, char*, styles);
DeclareSciCallR1(GetStyleFromSubStyle, GETSTYLEFROMSUBSTYLE, int, int, subStyle);
DeclareSciCallR1(GetPrimaryStyleFromStyle, GETPRIMARYSTYLEFROMSTYLE, int, int, style);
DeclareSciCallR0(GetNamedStyles, GETNAMEDSTYLES, int);
DeclareSciCallR2(NameOfStyle, NAMEOFSTYLE, int, int, style, char*, name);
DeclareSciCallR2(TagsOfStyle, TAGSOFSTYLE, int, int, style, char*, tags);
DeclareSciCallR2(DescriptionOfStyle, DESCRIPTIONOFSTYLE, int, int, style, char*, description);
DeclareSciCallV0(ReleaseAllExtendedStyles, RELEASEALLEXTENDEDSTYLES);
DeclareSciCallR1(AllocateExtendedStyles, ALLOCATEEXTENDEDSTYLES, int, int, numberStyles);


//=============================================================================
//
//  Printing
//
DeclareSciCallV1(SetPrintMagnification, SETPRINTMAGNIFICATION, int, magnification);
DeclareSciCallR0(GetPrintMagnification, GETPRINTMAGNIFICATION, int);
DeclareSciCallV1(SetPrintColourMode, SETPRINTCOLOURMODE, int, mode);
DeclareSciCallR0(GetPrintColourMode, GETPRINTCOLOURMODE, int);
DeclareSciCallV1(SetPrintWrapMode, SETPRINTWRAPMODE, int, wrapMode);
DeclareSciCallR0(GetPrintWrapMode, GETPRINTWRAPMODE, int);
DeclareSciCallR2(FormatRange, FORMATRANGE, DocPos, bool, draw, struct Sci_RangeToFormat*, fr);
DeclareSciCallR2(FormatRangeFull, FORMATRANGEFULL, DocPos, bool, draw, struct Sci_RangeToFormatFull*, fr);


//=============================================================================
//
//  Recording
//
DeclareSciCallV0(StartRecord, STARTRECORD);
DeclareSciCallV0(StopRecord, STOPRECORD);


//=============================================================================
//
//  Drag and Drop
//
DeclareSciCallR0(GetDragDropEnabled, GETDRAGDROPENABLED, bool);
DeclareSciCallV1(SetDragDropEnabled, SETDRAGDROPENABLED, bool, dragDropEnabled);


//=============================================================================
//
//  Utilities
//
DeclareSciCallR0(IsSelectionEmpty, GETSELECTIONEMPTY, bool);
DeclareSciCallR0(IsSelectionRectangle, SELECTIONISRECTANGLE, bool);

#define Sci_CallTipCancelEx() { SciCall_CallTipCancel(); SciCall_CallTipSetPosition(false); }

#define Sci_IsDocEmpty() (SciCall_GetTextLength() <= 0LL)

#define Sci_IsThinSelection() (SciCall_GetSelectionMode() == SC_SEL_THIN)
#define Sci_IsStreamSelection() (SciCall_GetSelectionMode() == SC_SEL_STREAM)
#define Sci_IsSingleSelection() (SciCall_GetSelections() == 1)
#define Sci_IsMultiSelection() ((SciCall_GetSelections() > 1) && !SciCall_IsSelectionRectangle())
#define Sci_IsMultiOrRectangleSelection() ((SciCall_GetSelections() > 1) || SciCall_IsSelectionRectangle())

#define Sci_GetLineStartPosition(position) SciCall_PositionFromLine(SciCall_LineFromPosition(position))
#define Sci_GetLineEndPosition(position) SciCall_GetLineEndPosition(SciCall_LineFromPosition(position))

#define Sci_IsCompleteLineSelected() ((Sci_GetLineStartPosition(SciCall_GetSelectionStart()) == SciCall_GetSelectionStart()) && \
                                      (Sci_GetLineEndPosition(SciCall_GetSelectionEnd()) == SciCall_GetSelectionEnd()))

#define Sci_IsSelectionSingleLine() (SciCall_LineFromPosition(SciCall_GetSelectionEnd()) == SciCall_LineFromPosition(SciCall_GetSelectionStart())) 
#define Sci_IsSelectionMultiLine() ((SciCall_LineFromPosition(SciCall_GetSelectionEnd()) - SciCall_LineFromPosition(SciCall_GetSelectionStart())) > 1)

#define Sci_IsPosInSelection(position) ((position >= SciCall_GetSelectionStart()) && (position <= SciCall_GetSelectionEnd()))

#define Sci_IsForwardSelection() (SciCall_GetAnchor() <= SciCall_GetCurrentPos())

#define Sci_HaveUndoRedoHistory() (SciCall_CanUndo() || SciCall_CanRedo())

#define Sci_GetCurrentLineNumber() SciCall_LineFromPosition(SciCall_GetCurrentPos())
#define Sci_GetCurrentColumnNumber() SciCall_GetColumn(SciCall_GetCurrentPos())
#define Sci_GetAnchorLineNumber() SciCall_LineFromPosition(SciCall_GetAnchor())
#define Sci_GetLastDocLineNumber() (SciCall_GetLineCount() - 1)
#define Sci_InLastLine() (SciCall_GetLineCount() == (Sci_GetCurrentLineNumber() + 1))

#define Sci_GetCurrChar() SciCall_GetCharAt(SciCall_GetCurrentPos())
#define Sci_GetNextChar() SciCall_GetCharAt(SciCall_PositionAfter(SciCall_GetCurrentPos()))

// length of line w/o line-end chars (full use SciCall_LineLength()
#define Sci_GetNetLineLength(line) (SciCall_GetLineEndPosition(line) - SciCall_PositionFromLine(line))

//~#define Sci_GetDocEndPosition() SciCall_GetTextLength()
#define Sci_GetDocEndPosition() SciCall_PositionAfter(SciCall_GetTextLength() - 1)

#define Sci_ClampAlpha(alpha) clampi((alpha), SC_ALPHA_TRANSPARENT, SC_ALPHA_OPAQUE) //~SC_ALPHA_NOALPHA

// ----------------------------------------------------------------------------

__forceinline bool Sci_IsPosValid(const DocPos pos)
{
    return (pos == ((pos > 0) ? SciCall_PositionAfter(SciCall_PositionBefore(pos)) : pos));
}
// ----------------------------------------------------------------------------

// max. line length in range (incl. line-breaks)
inline DocPos Sci_GetRangeMaxLineLength(DocLn iBeginLine, DocLn iEndLine)
{
    DocPos iMaxLineLen = 0;
    for (DocLn iLine = iBeginLine; iLine <= iEndLine; ++iLine) {
        DocPos const iLnLen = SciCall_LineLength(iLine);
        if (iLnLen > iMaxLineLen) {
            iMaxLineLen = iLnLen;
        }
    }
    return iMaxLineLen;
}
// ----------------------------------------------------------------------------

inline bool Sci_EnsureVisibleSelection()
{
    bool        bMoveViewToCaret = false;
    DocLn const iCurrentLn = Sci_GetCurrentLineNumber();
    DocLn const iAnchorLn = SciCall_LineFromPosition(SciCall_GetAnchor());
    if (!SciCall_GetLineVisible(iAnchorLn)) {
        if (iAnchorLn == iCurrentLn) {
            SciCall_EnsureVisibleEnforcePolicy(iAnchorLn);
            bMoveViewToCaret = true;
        }
        else {
            SciCall_EnsureVisible(iAnchorLn);
        }
    }
    if ((iAnchorLn != iCurrentLn) && !SciCall_GetLineVisible(iCurrentLn)) {
        SciCall_EnsureVisibleEnforcePolicy(iCurrentLn);
        bMoveViewToCaret = true;
    }
    return bMoveViewToCaret;
}
// ----------------------------------------------------------------------------

inline void Sci_ScrollSelectionToView()
{
    if (Sci_EnsureVisibleSelection()) {
        SciCall_ScrollToEnd(); // (!) jump at top-slope not bottom-slope
    }
    //~SciCall_ScrollRange(SciCall_GetAnchor(), SciCall_GetCurrentPos());
    SciCall_ScrollRange(SciCall_GetCurrentPos(), SciCall_GetAnchor());
}
// ----------------------------------------------------------------------------

// respect VSlop settings
inline void Sci_GotoPosChooseCaret(const DocPos pos)
{
    SciCall_GotoPos(pos);
    Sci_ScrollSelectionToView();
    SciCall_ChooseCaretX();
}
// ----------------------------------------------------------------------------

__forceinline void Sci_ScrollChooseCaret()
{
    SciCall_ScrollCaret();
    SciCall_ChooseCaretX();
}
// ----------------------------------------------------------------------------

inline void Sci_ScrollToLine(const DocLn line)
{
    if (!SciCall_GetLineVisible(line)) {
        SciCall_EnsureVisible(line);
    }
    SciCall_ScrollRange(SciCall_GetLineEndPosition(line), SciCall_PositionFromLine(line));
}
// ----------------------------------------------------------------------------

#define Sci_ScrollToCurrentLine() Sci_ScrollToLine(Sci_GetCurrentLineNumber())

// ----------------------------------------------------------------------------

inline void Sci_SetStreamSelection(DocPos iSelStart, DocPos iSelEnd, bool bStraightSel)
{
    SciCall_SetSelectionStart(iSelStart);
    SciCall_SetSelectionEnd(iSelEnd);
    if (!bStraightSel) {
        SciCall_SwapMainAnchorCaret();
    }
    SciCall_ChooseCaretX();
}
// ----------------------------------------------------------------------------

__forceinline void Sci_SetCaretScrollDocEnd()
{
    //~SciCall_ScrollToEnd();
    SciCall_GotoPos(Sci_GetDocEndPosition());
    SciCall_ScrollCaret(); // enforce visible slop policy
}
// ----------------------------------------------------------------------------

__forceinline void Sci_RedrawScrollbars()
{
    SciCall_SetHScrollbar(false);
    SciCall_SetHScrollbar(true);
    SciCall_SetVScrollbar(false);
    SciCall_SetVScrollbar(true);
}
// ----------------------------------------------------------------------------


//  if iRangeEnd == -1 : apply style from iRangeStart to document end
#define Sci_ColouriseAll() SciCall_Colourise(0, -1)

#define Sci_DisableMouseDWellNotification()  SciCall_SetMouseDWellTime(SC_TIME_FOREVER)

// ----------------------------------------------------------------------------

inline void Sci_GetSelectionTextN(char* pBuffer, size_t count)
{
    size_t const selLen = SciCall_GetSelText(NULL);
    StringCchCopyNA(pBuffer, count, SciCall_GetRangePointer(SciCall_GetSelectionStart(), selLen), selLen);
}

// ----------------------------------------------------------------------------

#define Sci_GetEOLLen() ((SciCall_GetEOLMode() == SC_EOL_CRLF) ? 2 : 1)

inline int Sci_GetCurrentEOL_A(LPCH eol)
{
    switch (SciCall_GetEOLMode()) {
    case SC_EOL_CRLF:
        if (eol) {
            eol[0] = '\r';
            eol[1] = '\n';
            eol[2] = '\0';
        }
        return 2;
    case SC_EOL_CR:
        if (eol) {
            eol[0] = '\r';
            eol[1] = '\0';
        }
        return 1;
    case SC_EOL_LF:
        if (eol) {
            eol[0] = '\n';
            eol[1] = '\0';
        }
        return 1;
    default:
        return 0;
    }
}
// ----------------------------------------------------------------------------

inline int Sci_GetCurrentEOL_W(LPWCH eol)
{
    switch (SciCall_GetEOLMode()) {
    case SC_EOL_CRLF:
        if (eol) {
            eol[0] = L'\r';
            eol[1] = L'\n';
            eol[2] = L'\0';
        }
        return 2;
    case SC_EOL_CR:
        if (eol) {
            eol[0] = L'\r';
            eol[1] = L'\0';
        }
        return 1;
    case SC_EOL_LF:
        if (eol) {
            eol[0] = L'\n';
            eol[1] = L'\0';
        }
        return 1;
    default:
        return 0;
    }
}
// ----------------------------------------------------------------------------
 
inline void Sci_SetWrapModeEx(const int wrapmode) {
    if (wrapmode != SciCall_GetWrapMode()) {
        SciCall_SetWrapMode(wrapmode);
    }
}

// ----------------------------------------------------------------------------

inline DocPos Sci_GetSelectionStartEx()
{
    if (!Sci_IsMultiSelection()) {
        return SciCall_GetSelectionStart();
    }
    DocPosU const nsel = SciCall_GetSelections();
    DocPos selStart = Sci_GetDocEndPosition() + 1;
    for (DocPosU i = 0; i < nsel; ++i) {
        DocPos const iStart = SciCall_GetSelectionNStart(i);
        if (iStart < selStart) {
            selStart = iStart;
        }
    }
    return selStart;
}
// ----------------------------------------------------------------------------

inline DocPos Sci_GetSelectionEndEx()
{
    if (!Sci_IsMultiSelection()) {
        return SciCall_GetSelectionEnd();
    }
    DocPosU const nsel = SciCall_GetSelections();
    DocPos selEnd = 0;
    for (DocPosU i = 0; i < nsel; ++i) {
        DocPos const iEnd = SciCall_GetSelectionNEnd(i);
        if (iEnd > selEnd) {
            selEnd = iEnd;
        }
    }
    return selEnd;
}
// ----------------------------------------------------------------------------

__forceinline DocPos Sci_ReplaceTargetTestChgHist(const DocPos length, const char* text)
{
    return SciCall_GetChangeHistory() ? SciCall_ReplaceTargetMinimal(length, text) : SciCall_ReplaceTarget(length, text);
}
// ----------------------------------------------------------------------------

inline DocPos Sci_ReplaceTargetEx(const int mode, const DocPos length, const char* text)
{
    switch (mode) {
    case SCI_REPLACETARGETRE:
        return SciCall_ReplaceTargetRe(length, text);
    case SCI_REPLACETARGETMINIMAL:
        return SciCall_ReplaceTargetMinimal(length, text);
    case SCI_REPLACETARGET:
    default:
        return SciCall_ReplaceTarget(length, text);
    }
}
// ----------------------------------------------------------------------------

inline LRESULT Sci_ForceNotifyUpdateUI(HWND hwnd, uptr_t idc)
{
    struct SCNotification scn = { 0 };
    scn.nmhdr.hwndFrom = g_hwndEditWindow;
    scn.nmhdr.idFrom = idc;
    scn.nmhdr.code = SCN_UPDATEUI;
    scn.updated = SC_UPDATE_CONTENT;
    return SendMessageW(hwnd, WM_NOTIFY, idc, (LPARAM)&scn);
}
// ----------------------------------------------------------------------------

//=============================================================================

#endif //_NP3_SCICALL_H_
