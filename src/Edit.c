/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Edit.c                                                                      *
*   Text File Editing Helper Stuff                                            *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2016   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

#if !defined(WINVER)
#define WINVER 0x601  /*_WIN32_WINNT_WIN7*/
#endif
#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x601  /*_WIN32_WINNT_WIN7*/
#endif
#if !defined(NTDDI_VERSION)
#define NTDDI_VERSION 0x06010000  /*NTDDI_WIN7*/
#endif

#define VC_EXTRALEAN 1
#include <windows.h>

#include <shlwapi.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "scintilla.h"
#include "scilexer.h"
#include "notepad3.h"
#include "styles.h"
#include "dialogs.h"
#include "resource.h"
#include "../crypto/crypto.h"
#include "../uthash/utarray.h"
//#include "../uthash/utstring.h"
#include "helpers.h"
#include "encoding.h"

#include "SciCall.h"

#include "edit.h"


#ifndef LCMAP_TITLECASE
#define LCMAP_TITLECASE  0x00000300  // Title Case Letters bit mask
#endif

// find free bits in scintilla.h SCFIND_ defines
#define SCFIND_NP3_REGEX (SCFIND_REGEXP | SCFIND_POSIX)

extern HWND  g_hwndMain;
extern HWND  g_hwndStatus;
extern HWND  g_hwndDlgFindReplace;
extern WININFO g_WinInfo;

extern HINSTANCE g_hInstance;
//extern LPMALLOC  g_lpMalloc;

extern DWORD dwLastIOError;
extern UINT cpLastFind;
extern BOOL bReplaceInitialized;
extern BOOL bUseOldStyleBraceMatching;
extern BOOL bUseDefaultForFileEncoding;
extern BOOL bFindReplCopySelOrClip;

static EDITFINDREPLACE efrSave;
static BOOL bSwitchedFindReplace = FALSE;

extern int xFindReplaceDlg;
extern int yFindReplaceDlg;
static int xFindReplaceDlgSave;
static int yFindReplaceDlgSave;

extern int g_iDefaultEOLMode;
extern int iLineEndings[3];
extern BOOL bFixLineEndings;
extern BOOL bAutoStripBlanks;

// Default Codepage and Character Set
extern int g_iDefaultNewFileEncoding;
extern int g_iDefaultCharSet;
extern BOOL bLoadASCIIasUTF8;
extern BOOL bLoadNFOasOEM;

extern BOOL bAccelWordNavigation;

extern int  iReplacedOccurrences;
extern int  iMarkOccurrences;
extern int  iMarkOccurrencesCount;
extern int  iMarkOccurrencesMaxCount;
extern BOOL bMarkOccurrencesMatchVisible;
extern BOOL g_bCodeFoldingAvailable;
extern BOOL g_bShowCodeFolding;

extern BOOL g_bTabsAsSpaces;
extern BOOL g_bTabIndents;
extern int  g_iTabWidth;
extern int  g_iIndentWidth;


#define DELIM_BUFFER 258
static char DelimChars[DELIM_BUFFER] = { '\0' };
static char DelimCharsAccel[DELIM_BUFFER] = { '\0' };
static char WordCharsDefault[DELIM_BUFFER] = { '\0' };
static char WhiteSpaceCharsDefault[DELIM_BUFFER] = { '\0' };
static char PunctuationCharsDefault[DELIM_BUFFER] = { '\0' };
static char WordCharsAccelerated[DELIM_BUFFER] = { '\0' };
static char WhiteSpaceCharsAccelerated[DELIM_BUFFER] = { '\0' };
static char PunctuationCharsAccelerated[1] = { '\0' }; // empty!

//static WCHAR W_DelimChars[DELIM_BUFFER] = { L'\0' };
//static WCHAR W_DelimCharsAccel[DELIM_BUFFER] = { L'\0' };
//static WCHAR W_WhiteSpaceCharsDefault[DELIM_BUFFER] = { L'\0' };
//static WCHAR W_WhiteSpaceCharsAccelerated[DELIM_BUFFER] = { L'\0' };

enum AlignMask {
  ALIGN_LEFT = 0,
  ALIGN_RIGHT = 1,
  ALIGN_CENTER = 2,
  ALIGN_JUSTIFY = 3,
  ALIGN_JUSTIFY_EX = 4
};

enum SortOrderMask {
  SORT_ASCENDING = 0,
  SORT_DESCENDING = 1,
  SORT_SHUFFLE = 2,
  SORT_MERGEDUP = 4,
  SORT_UNIQDUP = 8,
  SORT_UNIQUNIQ = 16,
  SORT_NOCASE = 32,
  SORT_LOGICAL = 64,
  SORT_COLUMN = 128
};


extern LPMRULIST mruFind;
extern LPMRULIST mruReplace;

extern BOOL bMarkOccurrencesCurrentWord;
extern BOOL bMarkOccurrencesMatchCase;
extern BOOL bMarkOccurrencesMatchWords;

// Timer bitfield
static volatile LONG g_lTargetTransactionBits = 0;
#define TIMER_BIT_MARK_OCC 1L
#define BLOCK_BIT_TARGET_TRANSACTION 2L
#define TEST_AND_SET(B)  InterlockedBitTestAndSet(&g_lTargetTransactionBits, B)
#define TEST_AND_RESET(B)  InterlockedBitTestAndReset(&g_lTargetTransactionBits, B)


//=============================================================================
//
//  EditEnterTargetTransaction(), EditLeaveTargetTransaction()
//
void EditEnterTargetTransaction() {
  (void)TEST_AND_SET(BLOCK_BIT_TARGET_TRANSACTION);
}

void EditLeaveTargetTransaction() {
  (void)TEST_AND_RESET(BLOCK_BIT_TARGET_TRANSACTION);
}

BOOL EditIsInTargetTransaction() {
  if (TEST_AND_RESET(BLOCK_BIT_TARGET_TRANSACTION)) {
    (void)TEST_AND_SET(BLOCK_BIT_TARGET_TRANSACTION);
    return TRUE;
  }
  return FALSE;
}


//=============================================================================
//
//  EditSetWordDelimiter()
//
void EditInitWordDelimiter(HWND hwnd)
{
  ZeroMemory(WordCharsDefault, COUNTOF(WordCharsDefault));
  ZeroMemory(WhiteSpaceCharsDefault, COUNTOF(WhiteSpaceCharsDefault));
  ZeroMemory(PunctuationCharsDefault, COUNTOF(PunctuationCharsDefault));
  ZeroMemory(WordCharsAccelerated, COUNTOF(WordCharsAccelerated));
  ZeroMemory(WhiteSpaceCharsAccelerated, COUNTOF(WhiteSpaceCharsAccelerated));
  //ZeroMemory(PunctuationCharsAccelerated, COUNTOF(PunctuationCharsAccelerated)); // empty!

  // 1st get/set defaults
  SendMessage(hwnd, SCI_GETWORDCHARS, 0, (LPARAM)WordCharsDefault);
  SendMessage(hwnd, SCI_GETWHITESPACECHARS, 0, (LPARAM)WhiteSpaceCharsDefault);
  SendMessage(hwnd, SCI_GETPUNCTUATIONCHARS, 0, (LPARAM)PunctuationCharsDefault);

  // default word delimiter chars are whitespace & punctuation & line ends
  const char* lineEnds = "\r\n";
  StringCchCopyA(DelimChars, COUNTOF(DelimChars), WhiteSpaceCharsDefault);
  StringCchCatA(DelimChars, COUNTOF(DelimChars), PunctuationCharsDefault);
  StringCchCatA(DelimChars, COUNTOF(DelimChars), lineEnds);

  // 2nd get user settings
  WCHAR buffer[DELIM_BUFFER] = { L'\0' };
  ZeroMemory(buffer, DELIM_BUFFER * sizeof(WCHAR));

  IniGetString(L"Settings2", L"ExtendedWhiteSpaceChars", L"", buffer, COUNTOF(buffer));
  char whitesp[DELIM_BUFFER] = { '\0' };
  if (StringCchLen(buffer, COUNTOF(buffer)) > 0) {
    WideCharToMultiByteStrg(CP_ACP, buffer, whitesp);
  }

  // 3rd set accelerated arrays

  // init with default
  StringCchCopyA(WhiteSpaceCharsAccelerated, COUNTOF(WhiteSpaceCharsAccelerated), WhiteSpaceCharsDefault);

  // add only 7-bit-ASCII chars to accelerated whitespace list
  for (size_t i = 0; i < strlen(whitesp); i++) {
    if (whitesp[i] & 0x7F) {
      if (!StrChrA(WhiteSpaceCharsAccelerated, whitesp[i])) {
        StringCchCatNA(WhiteSpaceCharsAccelerated, COUNTOF(WhiteSpaceCharsAccelerated), &(whitesp[i]), 1);
      }
    }
  }

  // construct word char array
  StringCchCopyA(WordCharsAccelerated, COUNTOF(WordCharsAccelerated), WordCharsDefault); // init
  // add punctuation chars not listed in white-space array
  for (size_t i = 0; i < strlen(PunctuationCharsDefault); i++) {
    if (!StrChrA(WhiteSpaceCharsAccelerated, PunctuationCharsDefault[i])) {
      StringCchCatNA(WordCharsAccelerated, COUNTOF(WordCharsAccelerated), &(PunctuationCharsDefault[i]), 1);
    }
  }

  // construct accelerated delimiters
  StringCchCopyA(DelimCharsAccel, COUNTOF(DelimCharsAccel), WhiteSpaceCharsDefault);
  StringCchCatA(DelimCharsAccel, COUNTOF(DelimCharsAccel), lineEnds);

  // constuct wide char arrays
  //MultiByteToWideChar(CP_UTF8, 0, DelimChars, -1, W_DelimChars, COUNTOF(W_DelimChars));
  //MultiByteToWideChar(CP_UTF8, 0, DelimCharsAccel, -1, W_DelimCharsAccel, COUNTOF(W_DelimCharsAccel));
  //MultiByteToWideChar(CP_UTF8, 0, WhiteSpaceCharsDefault, -1, W_WhiteSpaceCharsDefault, COUNTOF(W_WhiteSpaceCharsDefault));
  //MultiByteToWideChar(CP_UTF8, 0, WhiteSpaceCharsAccelerated, -1, W_WhiteSpaceCharsAccelerated, COUNTOF(W_WhiteSpaceCharsAccelerated));

}



//=============================================================================
//
//  EditSetNewText()
//
extern BOOL bFreezeAppTitle;
extern FILEVARS fvCurFile;

void EditSetNewText(HWND hwnd,char* lpstrText,DWORD cbText)
{
  bFreezeAppTitle = TRUE;

  if (SendMessage(hwnd, SCI_GETREADONLY, 0, 0)) {
    SendMessage(hwnd, SCI_SETREADONLY, FALSE, 0);
  }
  SendMessage(hwnd,SCI_CANCEL,0,0);
  SendMessage(hwnd,SCI_SETUNDOCOLLECTION,0,0);
  UndoRedoActionMap(-1,NULL);
  SendMessage(hwnd,SCI_CLEARALL,0,0);
  SendMessage(hwnd,SCI_MARKERDELETEALL,(WPARAM)MARKER_NP3_BOOKMARK,0);
  SendMessage(hwnd,SCI_SETSCROLLWIDTH, DEFAULT_SCROLL_WIDTH,0);
  SendMessage(hwnd,SCI_SETXOFFSET,0,0);

  FileVars_Apply(hwnd,&fvCurFile);

  if (cbText > 0) {
    SendMessage(hwnd, SCI_ADDTEXT, cbText, (LPARAM)lpstrText);
  }
  SendMessage(hwnd,SCI_SETUNDOCOLLECTION,1,0);
  //SendMessage(hwnd,EM_EMPTYUNDOBUFFER,0,0); // deprecated
  SendMessage(hwnd,SCI_SETSAVEPOINT,0,0);
  SendMessage(hwnd,SCI_GOTOPOS,0,0);
  SendMessage(hwnd,SCI_CHOOSECARETX,0,0);

  bFreezeAppTitle = FALSE;
}


//=============================================================================
//
//  EditConvertText()
//
BOOL EditConvertText(HWND hwnd, int encSource, int encDest, BOOL bSetSavePoint)
{
  if (encSource == encDest)
    return(TRUE);

  if (!(Encoding_IsValid(encSource) && Encoding_IsValid(encDest)))
    return(FALSE);

  DocPos length = SciCall_GetTextLength();

  if (length == 0)
  {
    SendMessage(hwnd,SCI_CANCEL,0,0);
    SendMessage(hwnd,SCI_SETUNDOCOLLECTION,0,0);
    UndoRedoActionMap(-1,NULL);
    SendMessage(hwnd,SCI_CLEARALL,0,0);
    SendMessage(hwnd,SCI_MARKERDELETEALL,(WPARAM)MARKER_NP3_BOOKMARK,0);
    Encoding_SciSetCodePage(hwnd,encDest);
    SendMessage(hwnd,SCI_SETUNDOCOLLECTION,(WPARAM)1,0);
    SendMessage(hwnd,SCI_GOTOPOS,0,0);
    SendMessage(hwnd,SCI_CHOOSECARETX,0,0);

    if (bSetSavePoint)
      SendMessage(hwnd,SCI_SETSAVEPOINT,0,0);
  }
  else {

    const DocPos chBufSize = length * 5 + 2;
    char* pchText = GlobalAlloc(GPTR,chBufSize);

    struct Sci_TextRange tr = { { 0, -1 }, NULL };
    tr.lpstrText = pchText;
    SendMessage(hwnd,SCI_GETTEXTRANGE,0,(LPARAM)&tr);

    const DocPos wchBufSize = length * 3 + 2;
    WCHAR* pwchText = GlobalAlloc(GPTR,wchBufSize);

    // MultiBytes(Sci) -> WideChar(destination) -> Sci(MultiByte)
    //UINT cpSci = Encoding_GetCodePage(encSource);
    UINT cpSci = Encoding_SciGetCodePage(hwnd); // fixed Scintilla internal (UTF-8)
    UINT cpDst = Encoding_GetCodePage(encDest);
    
    // get text as wide char
    int cbwText = MultiByteToWideChar(cpSci,0, pchText, (int)length, pwchText, (int)wchBufSize);
    // convert wide char to destination multibyte
    int cbText = WideCharToMultiByte(cpDst, 0, pwchText, cbwText, pchText, (int)chBufSize, NULL, NULL);
    // re-code to wide char
    cbwText = MultiByteToWideChar(cpDst, 0, pchText, cbText, pwchText, (int)wchBufSize);
    // convert to Scintilla format
    cbText = WideCharToMultiByte(cpSci, 0, pwchText, cbwText, pchText, (int)chBufSize, NULL, NULL);
    pchText[cbText] = '\0';
    pchText[cbText+1] = '\0';

    SendMessage(hwnd,SCI_CANCEL,0,0);
    SendMessage(hwnd,SCI_SETUNDOCOLLECTION,0,0);
    UndoRedoActionMap(-1,NULL);
    SendMessage(hwnd,SCI_CLEARALL,0,0);
    SendMessage(hwnd,SCI_MARKERDELETEALL,(WPARAM)MARKER_NP3_BOOKMARK,0);
    Encoding_SciSetCodePage(hwnd,encDest);
    SendMessage(hwnd,SCI_ADDTEXT,cbText,(LPARAM)pchText);
    SendMessage(hwnd,SCI_SETUNDOCOLLECTION,(WPARAM)1,0);
    SendMessage(hwnd,SCI_GOTOPOS,0,0);
    SendMessage(hwnd,SCI_CHOOSECARETX,0,0);

    GlobalFree(pchText);
    GlobalFree(pwchText);

  }
  return(TRUE);
}


//=============================================================================
//
//  EditSetNewEncoding()
//
BOOL EditSetNewEncoding(HWND hwnd,int iNewEncoding,BOOL bNoUI,BOOL bSetSavePoint) {

  int iCurrentEncoding = Encoding_Current(CPI_GET);

  if (iCurrentEncoding != iNewEncoding) {

    // conversion between arbitrary encodings may lead to unexpected results
    //BOOL bOneEncodingIsANSI = (Encoding_IsANSI(iCurrentEncoding) || Encoding_IsANSI(iNewEncoding));
    //BOOL bBothEncodingsAreANSI = (Encoding_IsANSI(iCurrentEncoding) && Encoding_IsANSI(iNewEncoding));
    //if (!bOneEncodingIsANSI || bBothEncodingsAreANSI) {
      // ~ return TRUE; // this would imply a successful conversion - it is not !
      //return FALSE; // commented out ? : allow conversion between arbitrary encodings
    //}
  
    if (SciCall_GetTextLength() == 0) {

      BOOL bIsEmptyUndoHistory = (SendMessage(hwnd, SCI_CANUNDO, 0, 0) == 0 && SendMessage(hwnd, SCI_CANREDO, 0, 0) == 0);

      BOOL doNewEncoding = (!bIsEmptyUndoHistory && !bNoUI) ?
        (InfoBox(MBYESNO, L"MsgConv2", IDS_ASK_ENCODING2) == IDYES) : TRUE;

      if (doNewEncoding) {
        return EditConvertText(hwnd,iCurrentEncoding,iNewEncoding,bSetSavePoint);
      }
    }
    else {
      
      BOOL doNewEncoding = (!bNoUI) ? (InfoBox(MBYESNO, L"MsgConv1", IDS_ASK_ENCODING) == IDYES) : TRUE;

      if (doNewEncoding) {
        return EditConvertText(hwnd,iCurrentEncoding,iNewEncoding,FALSE);
      }
    }
  } 
  return FALSE;
}

//=============================================================================
//
//  EditIsRecodingNeeded()
//
BOOL EditIsRecodingNeeded(WCHAR* pszText, int cchLen)
{
  if ((pszText == NULL) || (cchLen < 1))
    return FALSE;

  UINT codepage = Encoding_GetCodePage(Encoding_Current(CPI_GET));

  if ((codepage == CP_UTF7) || (codepage == CP_UTF8))
    return FALSE;

  DWORD dwFlags = WC_NO_BEST_FIT_CHARS | WC_COMPOSITECHECK | WC_DEFAULTCHAR;
  BOOL useNullParams = Encoding_IsMBCS(Encoding_Current(CPI_GET)) ? TRUE : FALSE;

  BOOL bDefaultCharsUsed = FALSE;
  int cch = 0;
  if (useNullParams)
    cch = WideCharToMultiByte(codepage, 0, pszText, cchLen, NULL, 0, NULL, NULL);
  else
    cch = WideCharToMultiByte(codepage, dwFlags, pszText, cchLen, NULL, 0, NULL, &bDefaultCharsUsed);

  if (useNullParams && (cch == 0)) {
    if (GetLastError() != ERROR_NO_UNICODE_TRANSLATION)
      cch = cchLen; // don't care
  }

  BOOL bSuccess = ((cch >= cchLen) && (cch != (int)0xFFFD)) ? TRUE : FALSE;
  
  return (!bSuccess || bDefaultCharsUsed);
}


//=============================================================================
//
//  EditGetClipboardText()
//
char* EditGetClipboardText(HWND hwnd,BOOL bCheckEncoding,int* pLineCount,int* pLenLastLn) {

  if (!IsClipboardFormatAvailable(CF_UNICODETEXT) || !OpenClipboard(GetParent(hwnd))) {
    char* pEmpty = StrDupA("");
    return (pEmpty);
  }

  // get clipboard
  HANDLE hmem = GetClipboardData(CF_UNICODETEXT);
  WCHAR* pwch = GlobalLock(hmem);
  int wlen = lstrlenW(pwch);

  if (bCheckEncoding && EditIsRecodingNeeded(pwch,wlen)) 
  {
    const DocPos iPos = SciCall_GetCurrentPos();
    const DocPos iAnchor = SciCall_GetAnchor();

    // switch encoding to universal UTF-8 codepage
    SendMessage(g_hwndMain,WM_COMMAND,(WPARAM)MAKELONG(IDM_ENCODING_UTF8,1),0);

    // restore and adjust selection
    if (iPos > iAnchor) {
      SciCall_SetSel(iAnchor, iPos);
    }
    else {
      SciCall_SetSel(iPos, iAnchor);
    }
    EditFixPositions(hwnd);
  }

  // translate to SCI editor component codepage (default: UTF-8)
  UINT codepage = Encoding_SciGetCodePage(hwnd);

  int mlen = WideCharToMultiByte(codepage,0,pwch,wlen,NULL,0,NULL,NULL);
  char* pmch = LocalAlloc(LPTR,mlen + 1);
  if (pmch && mlen != 0) {
    int cnt = WideCharToMultiByte(codepage,0,pwch,wlen,pmch,mlen + 1,NULL,NULL);
    if (cnt == 0)
      return (pmch);
  }
  else 
    return (pmch);

  int lineCount = 0;
  int lenLastLine = 0;
  if ((BOOL)SendMessage(hwnd,SCI_GETPASTECONVERTENDINGS,0,0)) {
    char* ptmp = LocalAlloc(LPTR,mlen * 2 + 2);
    if (ptmp) {
      char *s = pmch;
      char *d = ptmp;
      int eolmode = (int)SendMessage(hwnd,SCI_GETEOLMODE,0,0);
      for (int i = 0; (i <= mlen) && (*s != '\0'); ++i, ++lenLastLine) {
        if (*s == '\n' || *s == '\r') {
          if (eolmode == SC_EOL_CR) {
            *d++ = '\r';
          }
          else if (eolmode == SC_EOL_LF) {
            *d++ = '\n';
          }
          else { // eolmode == SC_EOL_CRLF
            *d++ = '\r';
            *d++ = '\n';
          }
          if ((*s == '\r') && (i + 1 < mlen) && (*(s + 1) == '\n')) {
            i++;
            s++;
          }
          s++;
          ++lineCount;
          lenLastLine = 0;
        }
        else {
          *d++ = *s++;
        }
      }
      *d = '\0';
      int mlen2 = (int)(d - ptmp);

      LocalFree(pmch);
      pmch = LocalAlloc(LPTR,mlen2 + 1);
      StringCchCopyA(pmch,mlen2 + 1,ptmp);
      LocalFree(ptmp);
    }
  }
  else {
    // count lines only
    char *s = pmch;
    for (int i = 0; (i <= mlen) && (*s != '\0'); ++i, ++lenLastLine) {
      if (*s == '\n' || *s == '\r') {
        if ((*s == '\r') && (i + 1 < mlen) && (*(s + 1) == '\n')) {
          i++;
          s++;
        }
        s++;
        ++lineCount;
        lenLastLine = 0;
      }
    }
  }

  GlobalUnlock(hmem);
  CloseClipboard();

  if (pLineCount)
    *pLineCount = lineCount;

  if (pLenLastLn)
    *pLenLastLn = lenLastLine;

  return (pmch);
}


//=============================================================================
//
//  EditSetClipboardText()
//
BOOL EditSetClipboardText(HWND hwnd, const char* pszText)
{
  if (!IsClipboardFormatAvailable(CF_UNICODETEXT)) {
    SciCall_CopyText((DocPos)strlen(pszText), pszText);
    return TRUE;
  }

  WCHAR* pszTextW = NULL;
  UINT uCodePage = Encoding_SciGetCodePage(hwnd);
  int cchTextW = MultiByteToWideChar(uCodePage, 0, pszText, -1, NULL, 0) + 1;
  if (cchTextW > 1) {
    pszTextW = LocalAlloc(LPTR, sizeof(WCHAR)*cchTextW);
    MultiByteToWideChar(uCodePage, 0, pszText, -1, pszTextW, cchTextW);
  }

  if (pszTextW) {
    SetClipboardTextW(GetParent(hwnd), pszTextW);
    LocalFree(pszTextW);
    return TRUE;
  }
  return FALSE;
}


//=============================================================================
//
//  EditClearClipboard()
//
BOOL EditClearClipboard(HWND hwnd)
{
  if (!IsClipboardFormatAvailable(CF_UNICODETEXT)) {
    SciCall_CopyText(0, "");
    return TRUE;
  }
  if (!OpenClipboard(GetParent(hwnd))) {
    return FALSE;
  }
  EmptyClipboard();
  CloseClipboard();
  return TRUE;
}


//=============================================================================
//
//  EditPaste2RectSel()
//
void EditPaste2RectSel(HWND hwnd, char* pText)
{
  if (!SciCall_IsSelectionRectangle()) { return; }

  const DocPos length = lstrlenA(pText); // '\0' terminated

  IgnoreNotifyChangeEvent();
  EditEnterTargetTransaction();

  const DocPosU selCount = (DocPosU)SendMessage(hwnd, SCI_GETSELECTIONS, 0, 0);

  char* pTextLine = pText;
  // remove line-break from last line
  if (*pTextLine != '\0') { StrTrimA(pTextLine, "\r\n"); }

  for (DocPosU s = 0; s < selCount; ++s) {
    // get lines from clip
    char *ln = pTextLine;
    int lnLen = 0;
    while (*ln != '\0') {
      if (s < (selCount - 1)) {
        if (*ln == '\n' || *ln == '\r') {
          if ((*ln == '\r') && (*(ln + 1) == '\n')) { ++ln; }
          ++ln; // next line
          break;
        }
        else { ++ln; ++lnLen; }
      }
      else { ++ln; ++lnLen; } // last line
    }

    const DocPos selCaretPos = (DocPos)SendMessage(hwnd, SCI_GETSELECTIONNCARET, (WPARAM)s, 0);
    const DocPos selAnchorPos = (DocPos)SendMessage(hwnd, SCI_GETSELECTIONNANCHOR, (WPARAM)s, 0);

    DocPos virtualSpaceLen = 0;
    DocPos selTargetStart = 0;
    DocPos selTargetEnd = 0;
    if (selCaretPos < selAnchorPos) {
      selTargetStart = selCaretPos;
      selTargetEnd = selAnchorPos;
      virtualSpaceLen = (DocPos)SendMessage(hwnd, SCI_GETSELECTIONNCARETVIRTUALSPACE, (WPARAM)s, 0);
    }
    else {
      selTargetStart = selAnchorPos;
      selTargetEnd = selCaretPos;
      virtualSpaceLen = (DocPos)SendMessage(hwnd, SCI_GETSELECTIONNANCHORVIRTUALSPACE, (WPARAM)s, 0);
    }

    if (virtualSpaceLen > 0) {
      char* pPadStr = LocalAlloc(LPTR, (virtualSpaceLen + length + 1) * sizeof(char));
      if (pPadStr) {
        SIZE_T size = LocalSize(pPadStr) - sizeof(char);
        FillMemory(pPadStr, virtualSpaceLen, ' ');
        pPadStr[virtualSpaceLen] = '\0';
        StringCchCatNA(pPadStr, size, pTextLine, lnLen);
        SciCall_SetTargetRange(selTargetStart, selTargetEnd);
        SciCall_ReplaceTarget(lstrlenA(pPadStr), pPadStr);
        LocalFree(pPadStr);
      }
      else {
        SciCall_SetTargetRange(selTargetStart, selTargetEnd);
        SciCall_ReplaceTarget(lnLen, pTextLine);
      }
    }
    else // no virtual space to pad
    {
      SciCall_SetTargetRange(selTargetStart, selTargetEnd);
      SciCall_ReplaceTarget(lnLen, pTextLine);
    }

    SendMessage(hwnd, SCI_SETSELECTIONNCARET, (WPARAM)s, (LPARAM)selTargetStart);
    SendMessage(hwnd, SCI_SETSELECTIONNANCHOR, (WPARAM)s, (LPARAM)selTargetStart);
    if (virtualSpaceLen > 0) {
      SendMessage(hwnd, SCI_SETSELECTIONNCARETVIRTUALSPACE, (WPARAM)s, (LPARAM)virtualSpaceLen);
      SendMessage(hwnd, SCI_SETSELECTIONNANCHORVIRTUALSPACE, (WPARAM)s, (LPARAM)virtualSpaceLen);
    }

    if (*ln != '\0') {
      pTextLine = ln; // next clip line
    }
    //else: rest of rect single selections are filled with last line
  
  } // for()

  EditLeaveTargetTransaction();
  ObserveNotifyChangeEvent();
}


//=============================================================================
//
//  EditPasteClipboard()
//
BOOL EditPasteClipboard(HWND hwnd, BOOL bSwapClipBoard, BOOL bSkipUnicodeCheck)
{
  int lineCount = 0;
  int lenLastLine = 0;

  char* pClip = EditGetClipboardText(hwnd, !bSkipUnicodeCheck, &lineCount, &lenLastLine);
  if (!pClip) {
    return FALSE; // recoding canceled
  }
  const DocPos clipLen = lstrlenA(pClip);

  const DocPos iCurPos = SciCall_GetCurrentPos();
  const DocPos iAnchorPos = SciCall_GetAnchor();

  if (SciCall_IsSelectionEmpty() || (lineCount <= 1)) 
  {
    IgnoreNotifyChangeEvent();

    if (SciCall_IsSelectionEmpty()) // SC_SEL_THIN
    {
      SciCall_Paste();
      if (bSwapClipBoard) { 
        EditClearClipboard(hwnd);
        EditSelectEx(hwnd, iAnchorPos, SciCall_GetCurrentPos());
      }
    }
    else {
      char* pszText = LocalAlloc(LPTR, SciCall_GetSelText(NULL));
      SciCall_GetSelText(pszText);
      if (clipLen == 0) { SciCall_Clear(); } else { SciCall_Paste(); }
      if (bSwapClipBoard) {
        EditSetClipboardText(hwnd, pszText);
        if (iCurPos < iAnchorPos)
          EditSelectEx(hwnd, SciCall_GetCurrentPos(), iCurPos);
        else
          EditSelectEx(hwnd, iAnchorPos, SciCall_GetCurrentPos());
      }
      else {
        if (iCurPos < iAnchorPos)
          EditSelectEx(hwnd, iCurPos, iCurPos);
      }
      LocalFree(pszText);
    }
    ObserveNotifyChangeEvent();
  }
  else {
    if (SciCall_IsSelectionRectangle()) 
    {
      if (bSwapClipBoard) { SciCall_Copy(); }
      EditPaste2RectSel(hwnd, pClip);
      // TODO: restore selection in case of swap clipboard 
    }
    else // Selection: SC_SEL_STREAM, SC_SEL_LINES
    {
      IgnoreNotifyChangeEvent();
      if (bSwapClipBoard) {
        SciCall_Copy();
        SciCall_ReplaceSel(pClip);
        if (iCurPos < iAnchorPos)
          EditSelectEx(hwnd, iCurPos + clipLen, iCurPos);
        else
          EditSelectEx(hwnd, iAnchorPos, iAnchorPos + clipLen);
      }
      else {
        SciCall_ReplaceSel(pClip);
        if (iCurPos < iAnchorPos)
          EditSelectEx(hwnd, iCurPos, iCurPos);
      }
      ObserveNotifyChangeEvent();
    }
  }
  LocalFree(pClip);
  return TRUE;
}


//=============================================================================
//
//  EditCopyAppend()
//
BOOL EditCopyAppend(HWND hwnd, BOOL bAppend)
{
  DocPos iCurPos = SciCall_GetCurrentPos();
  DocPos iAnchorPos = SciCall_GetAnchor();

  char* pszText = NULL;
  if (iCurPos != iAnchorPos) {
    if (SciCall_IsSelectionRectangle()) {
      MsgBox(MBWARN, IDS_SELRECT);
      return FALSE;
    }
    else {
      pszText = LocalAlloc(LPTR, SciCall_GetSelText(NULL));
      SciCall_GetSelText(pszText);
    }
  }
  else {
    DocPos cchText = SciCall_GetTextLength();
    pszText = LocalAlloc(LPTR,cchText + 1);
    SciCall_GetTextFromBegin((DocPos)LocalSize(pszText), pszText);
  }
  WCHAR* pszTextW = NULL;
  UINT uCodePage = Encoding_SciGetCodePage(hwnd);
  int cchTextW = MultiByteToWideChar(uCodePage,0,pszText,-1,NULL,0);
  if (cchTextW > 0) {
    int lenTxt = (cchTextW + 1);
    pszTextW = LocalAlloc(LPTR,sizeof(WCHAR)*lenTxt);
    MultiByteToWideChar(uCodePage,0,pszText,-1,pszTextW,lenTxt);
  }
  
  if (pszText)
    LocalFree(pszText);

  if (!bAppend) {
    BOOL res = (BOOL)SetClipboardTextW(GetParent(hwnd), pszTextW);
    LocalFree(pszTextW);
    return res;
  }

  // --- Append to Clipboard ---

  if (!OpenClipboard(GetParent(hwnd))) {
    LocalFree(pszTextW);
    return FALSE;
  }

  HANDLE hOld   = GetClipboardData(CF_UNICODETEXT);
  WCHAR* pszOld = GlobalLock(hOld);

  int sizeNew   = lstrlen(pszOld) + lstrlen(pszTextW) + 1;

  const  WCHAR *pszSep = L"\r\n";
  sizeNew += (int)lstrlen(pszSep);

  // Copy Clip
  WCHAR* pszNewTextW = LocalAlloc(LPTR, sizeof(WCHAR) * sizeNew);
  
  if (pszOld)
    StringCchCopy(pszNewTextW, sizeNew, pszOld);

  GlobalUnlock(hOld);
  CloseClipboard();


  // Add New
  StringCchCat(pszNewTextW, sizeNew, pszSep);
  StringCchCat(pszNewTextW, sizeNew, pszTextW);

  BOOL res = (BOOL)SetClipboardTextW(GetParent(hwnd), pszNewTextW);

  LocalFree(pszNewTextW);
  return res;
}


//=============================================================================
//
//  EditDetectEOLMode() - moved here to handle Unicode files correctly
//
int EditDetectEOLMode(HWND hwnd,char* lpData,DWORD cbData)
{
  int iEOLMode = iLineEndings[g_iDefaultEOLMode];
  char *cp = (char*)lpData;

  if (!cp)
    return (iEOLMode);

  while (*cp && (*cp != '\x0D' && *cp != '\x0A')) cp++;

  if (*cp == '\x0D' && *(cp+1) == '\x0A')
    iEOLMode = SC_EOL_CRLF;
  else if (*cp == '\x0D' && *(cp+1) != '\x0A')
    iEOLMode = SC_EOL_CR;
  else if (*cp == '\x0A')
    iEOLMode = SC_EOL_LF;

  UNUSED(hwnd);
  UNUSED(cbData);

  return (iEOLMode);
}



//=============================================================================
//
//  EditLoadFile()
//
BOOL EditLoadFile(
       HWND hwnd,
       LPCWSTR pszFile,
       BOOL bSkipUTFDetection,
       BOOL bSkipANSICPDetection,
       int* iEncoding,
       int* iEOLMode,
       BOOL *pbUnicodeErr,
       BOOL *pbFileTooBig,
       BOOL *pbUnkownExt)
{
  if (pbUnicodeErr)
    *pbUnicodeErr = FALSE;
  if (pbFileTooBig)
    *pbFileTooBig = FALSE;
  if (pbUnkownExt)
    *pbUnkownExt  = FALSE;

  HANDLE hFile = CreateFile(pszFile,
                            GENERIC_READ,
                            FILE_SHARE_READ|FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);
  dwLastIOError = GetLastError();

  if (hFile == INVALID_HANDLE_VALUE) {
    Encoding_SrcCmdLn(CPI_NONE);
    Encoding_SrcWeak(CPI_NONE);
    return FALSE;
  }

  // calculate buffer limit
  DWORD dwFileSize = GetFileSize(hFile,NULL);
  DWORD dwBufSize = dwFileSize + 16;

  // check for unknown extension
  LPWSTR lpszExt = PathFindExtension(pszFile);
  if (!Style_HasLexerForExt(lpszExt)) {
    if (InfoBox(MBYESNO,L"MsgFileUnknownExt",IDS_WARN_UNKNOWN_EXT,lpszExt) != IDYES) {
      CloseHandle(hFile);
      if (pbUnkownExt)
        *pbUnkownExt = TRUE;
      Encoding_SrcCmdLn(CPI_NONE);
      Encoding_SrcWeak(CPI_NONE);
      return FALSE;
    }
  }

  // Check if a warning message should be displayed for large files
  DWORD dwFileSizeLimit = IniGetInt(L"Settings2",L"FileLoadWarningMB",1);
  if (dwFileSizeLimit != 0 && dwFileSizeLimit * 1024 * 1024 < dwFileSize) {
    if (InfoBox(MBYESNO,L"MsgFileSizeWarning",IDS_WARN_LOAD_BIG_FILE) != IDYES) {
      CloseHandle(hFile);
      if (pbFileTooBig)
        *pbFileTooBig = TRUE;
      Encoding_SrcCmdLn(CPI_NONE);
      Encoding_SrcWeak(CPI_NONE);
      return FALSE;
    }
  }

  char* lpData = GlobalAlloc(GPTR,dwBufSize);

  dwLastIOError = GetLastError();
  if (!lpData)
  {
    CloseHandle(hFile);
    if (pbFileTooBig)
      *pbFileTooBig = FALSE;
    Encoding_SrcCmdLn(CPI_NONE);
    Encoding_SrcWeak(CPI_NONE);
    return FALSE;
  }

  DWORD cbData = 0L;
  BOOL bReadSuccess = ReadAndDecryptFile(hwnd, hFile, (DWORD)GlobalSize(lpData) - 2, &lpData, &cbData);
  dwLastIOError = GetLastError();
  CloseHandle(hFile);

  if (!bReadSuccess) {
    GlobalFree(lpData);
    Encoding_SrcCmdLn(CPI_NONE);
    Encoding_SrcWeak(CPI_NONE);
    return FALSE;
  }

  BOOL bPreferOEM = FALSE;
  if (bLoadNFOasOEM)
  {
    if (lpszExt && !(StringCchCompareIX(lpszExt,L".nfo") && StringCchCompareIX(lpszExt,L".diz")))
      bPreferOEM = TRUE;
  }

  const int iForcedEncoding = Encoding_SrcCmdLn(CPI_GET);
  const int iFileEncWeak = Encoding_SrcWeak(CPI_GET);
  
  const size_t cbNbytes4Analysis = (cbData < 200000L) ? cbData : 200000L;
  bool bIsReliable = false;
  const int iAnalyzedEncoding = bSkipANSICPDetection ? CPI_NONE : Encoding_Analyze(lpData, cbNbytes4Analysis, &bIsReliable);

  // choose best encoding guess
  int iPreferedEncoding = (bPreferOEM) ? g_DOSEncoding : (bUseDefaultForFileEncoding ? g_iDefaultNewFileEncoding : CPI_ANSI_DEFAULT);

  if (iForcedEncoding != CPI_NONE)
    iPreferedEncoding = iForcedEncoding;
  else if (iFileEncWeak != CPI_NONE)
    iPreferedEncoding = iFileEncWeak;
  else if (Encoding_IsUNICODE(iAnalyzedEncoding) && !bSkipUTFDetection)
    iPreferedEncoding = iAnalyzedEncoding;
  else if (iAnalyzedEncoding != CPI_NONE)
    iPreferedEncoding = iAnalyzedEncoding;


  BOOL bBOM = FALSE;
  BOOL bReverse = FALSE;

  if (cbData == 0) {
    FileVars_Init(NULL,0,&fvCurFile);
    *iEOLMode = iLineEndings[g_iDefaultEOLMode];
    if (iForcedEncoding == CPI_NONE) {
      if (bLoadASCIIasUTF8 && !bPreferOEM)
        *iEncoding = CPI_UTF8;
      else
        *iEncoding = iPreferedEncoding;
    }
    else
      *iEncoding = iForcedEncoding;

    Encoding_SciSetCodePage(hwnd,*iEncoding);
    EditSetNewText(hwnd,"",0);
    SendMessage(hwnd,SCI_SETEOLMODE,iLineEndings[g_iDefaultEOLMode],0);
    GlobalFree(lpData);
  }
  // ===  UNICODE  ===
  else if (!bSkipUTFDetection &&  //TODO: use Encoding_IsUNICODE(iAnalyzedEncoding) here ???
      (Encoding_IsUNICODE(iForcedEncoding) || (iForcedEncoding == CPI_NONE)) &&
      (Encoding_IsUNICODE(iForcedEncoding) || IsUnicode(lpData,cbData,&bBOM,&bReverse)) &&
      (Encoding_IsUNICODE(iForcedEncoding) || !IsUTF8Signature(lpData))) // check for UTF-8 signature
  {
    char* lpDataUTF8;

    if (iForcedEncoding == CPI_UNICODE) {
      bBOM = (*((UNALIGNED PWCHAR)lpData) == 0xFEFF);
      bReverse = FALSE;
    }
    else if (iForcedEncoding == CPI_UNICODEBE)
      bBOM = (*((UNALIGNED PWCHAR)lpData) == 0xFFFE);

    if (iForcedEncoding == CPI_UNICODEBE || bReverse) {
      _swab(lpData,lpData,cbData);
      if (bBOM)
        *iEncoding = CPI_UNICODEBEBOM;
      else
        *iEncoding = CPI_UNICODEBE;
    }
    else {
      if (bBOM)
        *iEncoding = CPI_UNICODEBOM;
      else
        *iEncoding = CPI_UNICODE;
    }
    Encoding_SciSetCodePage(hwnd,*iEncoding);

    lpDataUTF8 = GlobalAlloc(GPTR,(cbData * 3) + 2);

    DWORD convCnt = (DWORD)WideCharToMultiByte(Encoding_SciGetCodePage(hwnd),0,(bBOM) ? (LPWSTR)lpData + 1 : (LPWSTR)lpData,
              (bBOM) ? (cbData)/sizeof(WCHAR) : cbData/sizeof(WCHAR) + 1,lpDataUTF8,(int)GlobalSize(lpDataUTF8),NULL,NULL);

    if (convCnt == 0) {
      if (pbUnicodeErr)
        *pbUnicodeErr = TRUE;
      convCnt = (DWORD)WideCharToMultiByte(CP_ACP,0,(bBOM) ? (LPWSTR)lpData + 1 : (LPWSTR)lpData,
                (-1),lpDataUTF8,(int)GlobalSize(lpDataUTF8),NULL,NULL);
    }

    if (convCnt != 0) {
      GlobalFree(lpData);
      Encoding_SciSetCodePage(hwnd,*iEncoding);
      EditSetNewText(hwnd,"",0);
      FileVars_Init(lpDataUTF8,convCnt - 1,&fvCurFile);
      EditSetNewText(hwnd,lpDataUTF8,convCnt - 1);
      *iEOLMode = EditDetectEOLMode(hwnd,lpDataUTF8,convCnt - 1);
      GlobalFree(lpDataUTF8);
    }
    else {
      GlobalFree(lpDataUTF8);
      GlobalFree(lpData);
      Encoding_SrcCmdLn(CPI_NONE);
      Encoding_SrcWeak(CPI_NONE);
      return FALSE;
    }
  }

  else { // ===  ALL OTHERS  ===

    FileVars_Init(lpData,cbData,&fvCurFile);

    // ===  UTF-8  ===
    if (!bSkipUTFDetection && (Encoding_IsNONE(iForcedEncoding) || Encoding_IsUTF8(iForcedEncoding)) &&
      ((IsUTF8Signature(lpData) || 
        FileVars_IsUTF8(&fvCurFile) || 
        (Encoding_IsUTF8(iForcedEncoding) || 
         Encoding_IsUTF8(iAnalyzedEncoding) ||
         (!bPreferOEM && bLoadASCIIasUTF8) ||  // from menu "Reload As UTF-8"
         (IsUTF8(lpData,cbData) && ((UTF8_ContainsInvalidChars(lpData, cbData) ||
         (!bPreferOEM && (Encoding_IsUTF8(iPreferedEncoding) || bLoadASCIIasUTF8))))))) && 
       !(FileVars_IsNonUTF8(&fvCurFile) && !Encoding_IsUTF8(iForcedEncoding))))
    {
      Encoding_SciSetCodePage(hwnd,CPI_UTF8);
      EditSetNewText(hwnd,"",0);
      if (IsUTF8Signature(lpData)) {
        EditSetNewText(hwnd,UTF8StringStart(lpData),cbData-3);
        *iEncoding = CPI_UTF8SIGN;
        *iEOLMode = EditDetectEOLMode(hwnd,UTF8StringStart(lpData),cbData-3);
      }
      else {
        EditSetNewText(hwnd,lpData,cbData);
        *iEncoding = CPI_UTF8;
        *iEOLMode = EditDetectEOLMode(hwnd,lpData,cbData);
      }
      GlobalFree(lpData);
    }

    else { // ===  ALL OTHER  ===

      if (!Encoding_IsNONE(iForcedEncoding))
        *iEncoding = iForcedEncoding;
      else {
        *iEncoding = FileVars_GetEncoding(&fvCurFile);
        if (Encoding_IsNONE(*iEncoding)) {
          if (fvCurFile.mask & FV_ENCODING)
            *iEncoding = CPI_ANSI_DEFAULT;
          else {
            *iEncoding = iPreferedEncoding;
          }
        }
      }

      if (((Encoding_GetCodePage(*iEncoding) != CP_UTF7) && Encoding_IsEXTERNAL_8BIT(*iEncoding)) ||
          ((Encoding_GetCodePage(*iEncoding) == CP_UTF7) && IsUTF7(lpData,cbData))) {

        UINT uCodePage = Encoding_GetCodePage(*iEncoding);

        LPWSTR lpDataWide = GlobalAlloc(GPTR,cbData * 2 + 16);
        int cbDataWide = MultiByteToWideChar(uCodePage,0,lpData,cbData,lpDataWide,(int)GlobalSize(lpDataWide)/sizeof(WCHAR));
        if (cbDataWide != 0) 
        {
          GlobalFree(lpData);
          lpData = GlobalAlloc(GPTR,cbDataWide * 3 + 16);

          Encoding_SciSetCodePage(hwnd,*iEncoding);
          cbData = WideCharToMultiByte(Encoding_SciGetCodePage(hwnd),0,lpDataWide,cbDataWide,lpData,(int)GlobalSize(lpData),NULL,NULL);
          if (cbData != 0) {
            GlobalFree(lpDataWide);
            EditSetNewText(hwnd,"",0);
            EditSetNewText(hwnd,lpData,cbData);
            *iEOLMode = EditDetectEOLMode(hwnd,lpData,cbData);
            GlobalFree(lpData);
          }
          else {
            GlobalFree(lpDataWide);
            GlobalFree(lpData);
            Encoding_SrcCmdLn(CPI_NONE);
            Encoding_SrcWeak(CPI_NONE);
            return FALSE;
          }
        }
        else {
          GlobalFree(lpDataWide);
          GlobalFree(lpData);
          Encoding_SrcCmdLn(CPI_NONE);
          Encoding_SrcWeak(CPI_NONE);
          return FALSE;
        }
      }
      else {
        *iEncoding = Encoding_IsValid(iForcedEncoding) ? iForcedEncoding : iPreferedEncoding;
        Encoding_SciSetCodePage(hwnd,*iEncoding);
        EditSetNewText(hwnd,"",0);
        EditSetNewText(hwnd,lpData,cbData);
        *iEOLMode = EditDetectEOLMode(hwnd,lpData,cbData);
        GlobalFree(lpData);
      }
    }
  }

  Encoding_SrcCmdLn(CPI_NONE);
  Encoding_SrcWeak(CPI_NONE);
  return TRUE;

}


//=============================================================================
//
//  EditSaveFile()
//
BOOL EditSaveFile(
       HWND hwnd,
       LPCWSTR pszFile,
       int iEncoding,
       BOOL *pbCancelDataLoss,
       BOOL bSaveCopy)
{

  HANDLE hFile;
  BOOL   bWriteSuccess;

  char* lpData;
  DWORD cbData;
  DWORD dwBytesWritten;

  *pbCancelDataLoss = FALSE;

  hFile = CreateFile(pszFile,
                     GENERIC_WRITE,
                     FILE_SHARE_READ|FILE_SHARE_WRITE,
                     NULL,
                     OPEN_ALWAYS,
                     FILE_ATTRIBUTE_NORMAL,
                     NULL);
  dwLastIOError = GetLastError();

  // failure could be due to missing attributes (2k/XP)
  if (hFile == INVALID_HANDLE_VALUE)
  {
    DWORD dwAttributes = GetFileAttributes(pszFile);
    if (dwAttributes != INVALID_FILE_ATTRIBUTES)
    {
      dwAttributes = dwAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
      hFile = CreateFile(pszFile,
                        GENERIC_WRITE,
                        FILE_SHARE_READ|FILE_SHARE_WRITE,
                        NULL,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL | dwAttributes,
                        NULL);
      dwLastIOError = GetLastError();
    }
  }

  if (hFile == INVALID_HANDLE_VALUE)
    return FALSE;

  // ensure consistent line endings
  if (bFixLineEndings) {
    SendMessage(hwnd,SCI_CONVERTEOLS,SendMessage(hwnd,SCI_GETEOLMODE,0,0),0);
    EditFixPositions(hwnd);
  }

  // strip trailing blanks
  if (bAutoStripBlanks)
    EditStripLastCharacter(hwnd, TRUE, TRUE);

  // get text
  cbData = (DWORD)SciCall_GetTextLength();
  lpData = GlobalAlloc(GPTR, cbData + 4); //fix: +bom
  SendMessage(hwnd,SCI_GETTEXT,GlobalSize(lpData),(LPARAM)lpData);

  if (cbData == 0) {
    bWriteSuccess = SetEndOfFile(hFile);
    dwLastIOError = GetLastError();
  }

  else {

  // FIXME: move checks in front of disk file access
  /*if ((g_Encodings[iEncoding].uFlags & NCP_UNICODE) == 0 && (g_Encodings[iEncoding].uFlags & NCP_UTF8_SIGN) == 0) {
      BOOL bEncodingMismatch = TRUE;
      FILEVARS fv;
      FileVars_Init(lpData,cbData,&fv);
      if (fv.mask & FV_ENCODING) {
        int iAltEncoding;
        if (FileVars_IsValidEncoding(&fv)) {
          iAltEncoding = FileVars_GetEncoding(&fv);
          if (iAltEncoding == iEncoding)
            bEncodingMismatch = FALSE;
          else if ((g_Encodings[iAltEncoding].uFlags & NCP_UTF8) && (g_Encodings[iEncoding].uFlags & NCP_UTF8))
            bEncodingMismatch = FALSE;
        }
        if (bEncodingMismatch) {
          Encoding_SetLabel(iAltEncoding);
          Encoding_SetLabel(iEncoding);
          InfoBox(0,L"MsgEncodingMismatch",IDS_ENCODINGMISMATCH,
            g_Encodings[iAltEncoding].wchLabel,
            g_Encodings[iEncoding].wchLabel);
        }
      }
    }*/

    if (Encoding_IsUNICODE(iEncoding))
    {
      SetEndOfFile(hFile);

      LPWSTR lpDataWide = GlobalAlloc(GPTR, cbData * 2 + 16);
      int bomoffset = 0;
      if (Encoding_IsUNICODE_BOM(iEncoding)) {
        const char* bom = "\xFF\xFE";
        CopyMemory((char*)lpDataWide, bom, 2);
        bomoffset = 1;
      }
      int cbDataWide = bomoffset + MultiByteToWideChar(Encoding_SciGetCodePage(hwnd), 0, lpData, cbData, &lpDataWide[bomoffset], (int)GlobalSize(lpDataWide) / sizeof(WCHAR) - bomoffset);
      if (Encoding_IsUNICODE_REVERSE(iEncoding)) {
        _swab((char*)lpDataWide, (char*)lpDataWide, cbDataWide * sizeof(WCHAR));
      }
      bWriteSuccess = EncryptAndWriteFile(hwnd, hFile, (BYTE*)lpDataWide, cbDataWide * sizeof(WCHAR), &dwBytesWritten);
      dwLastIOError = GetLastError();

      GlobalFree(lpDataWide);
      GlobalFree(lpData);
    }

    else if (Encoding_IsUTF8(iEncoding))
    {
      SetEndOfFile(hFile);

      if (Encoding_IsUTF8_SIGN(iEncoding)) {
        const char* bom = "\xEF\xBB\xBF";
        DWORD bomoffset = 3;
        MoveMemory(&lpData[bomoffset], lpData, cbData);
        CopyMemory(lpData, bom, bomoffset);
        cbData += bomoffset;
    }
      //bWriteSuccess = WriteFile(hFile,lpData,cbData,&dwBytesWritten,NULL);
      bWriteSuccess = EncryptAndWriteFile(hwnd, hFile, (BYTE*)lpData, cbData, &dwBytesWritten);
      dwLastIOError = GetLastError();

      GlobalFree(lpData);
    }

    else if (Encoding_IsEXTERNAL_8BIT(iEncoding)) {

      BOOL bCancelDataLoss = FALSE;
      UINT uCodePage = Encoding_GetCodePage(iEncoding);

      LPWSTR lpDataWide = GlobalAlloc(GPTR,cbData * 2 + 16);
      int    cbDataWide = MultiByteToWideChar(Encoding_SciGetCodePage(hwnd),0,lpData,cbData,lpDataWide,(int)GlobalSize(lpDataWide)/sizeof(WCHAR));

      if (Encoding_IsMBCS(iEncoding)) {
        GlobalFree(lpData);
        lpData = GlobalAlloc(GPTR, GlobalSize(lpDataWide) * 2); // need more space
        cbData = WideCharToMultiByte(uCodePage, 0, lpDataWide, cbDataWide, lpData, (int)GlobalSize(lpData), NULL, NULL);
      }
      else {
        ZeroMemory(lpData, GlobalSize(lpData));
        cbData = WideCharToMultiByte(uCodePage,WC_NO_BEST_FIT_CHARS,lpDataWide,cbDataWide,lpData,(int)GlobalSize(lpData),NULL,&bCancelDataLoss);
        if (!bCancelDataLoss) {
          cbData = WideCharToMultiByte(uCodePage,0,lpDataWide,cbDataWide,lpData,(int)GlobalSize(lpData),NULL,NULL);
          bCancelDataLoss = FALSE;
        }
      }
      GlobalFree(lpDataWide);

      if (!bCancelDataLoss || InfoBox(MBOKCANCEL,L"MsgConv3",IDS_ERR_UNICODE2) == IDOK) {
        SetEndOfFile(hFile);
        bWriteSuccess = EncryptAndWriteFile(hwnd, hFile, (BYTE*)lpData, cbData, &dwBytesWritten);
        dwLastIOError = GetLastError();
      }
      else {
        bWriteSuccess = FALSE;
        *pbCancelDataLoss = TRUE;
      }

      GlobalFree(lpData);
    }

    else {
      SetEndOfFile(hFile);
      bWriteSuccess = EncryptAndWriteFile(hwnd, hFile, (BYTE*)lpData, cbData, &dwBytesWritten);
      dwLastIOError = GetLastError();
      GlobalFree(lpData);
    }
  }

  CloseHandle(hFile);

  if (bWriteSuccess)
  {
    if (!bSaveCopy)
      SendMessage(hwnd,SCI_SETSAVEPOINT,0,0);

    return TRUE;
  }

  else
    return FALSE;

}


//=============================================================================
//
//  EditInvertCase()
//
void EditInvertCase(HWND hwnd)
{
  const DocPos iCurPos = SciCall_GetCurrentPos(); 
  const DocPos iAnchorPos = SciCall_GetAnchor();

  if (iCurPos != iAnchorPos)
  {
    if (!SciCall_IsSelectionRectangle())
    {
      const DocPos iSelStart = SciCall_GetSelectionStart();
      const DocPos iSelEnd = SciCall_GetSelectionEnd();
      const DocPos iSelLength = SciCall_GetSelText(NULL);

      char*  pszText  = GlobalAlloc(GPTR,iSelLength);
      LPWSTR pszTextW = GlobalAlloc(GPTR,(iSelLength*sizeof(WCHAR)));

      if (pszText == NULL || pszTextW == NULL) {
        GlobalFree(pszText);
        GlobalFree(pszTextW);
        return;
      }
      SciCall_GetSelText(pszText);

      UINT cpEdit = Encoding_SciGetCodePage(hwnd);
      int cchTextW = MultiByteToWideChar(cpEdit,0,pszText,(int)(iSelLength-1),pszTextW,(int)iSelLength);

      BOOL bChanged = FALSE;
      for (int i = 0; i < cchTextW; i++) {
        if (IsCharUpperW(pszTextW[i])) {
          pszTextW[i] = LOWORD(CharLowerW((LPWSTR)(LONG_PTR)MAKELONG(pszTextW[i],0)));
          bChanged = TRUE;
        }
        else if (IsCharLowerW(pszTextW[i])) {
          pszTextW[i] = LOWORD(CharUpperW((LPWSTR)(LONG_PTR)MAKELONG(pszTextW[i],0)));
          bChanged = TRUE;
        }
      }

      if (bChanged) {

        WideCharToMultiByte(cpEdit,0,pszTextW,cchTextW,pszText,(int)GlobalSize(pszText),NULL,NULL);

        SciCall_Clear();
        SciCall_AddText((iSelEnd - iSelStart), pszText);
        SciCall_SetSel(iAnchorPos, iCurPos);
      }

      GlobalFree(pszText);
      GlobalFree(pszTextW);
    }
    else
      MsgBox(MBWARN,IDS_SELRECT);
  }
}


//=============================================================================
//
//  EditTitleCase()
//
void EditTitleCase(HWND hwnd)
{
  const DocPos iCurPos = SciCall_GetCurrentPos();
  const DocPos iAnchorPos = SciCall_GetAnchor();

  if (iCurPos != iAnchorPos)
  {
    if (!SciCall_IsSelectionRectangle())
    {
      const DocPos iSelStart = SciCall_GetSelectionStart();
      const DocPos iSelEnd = SciCall_GetSelectionEnd();
      const DocPos iSelLength = SciCall_GetSelText(NULL);

      char*  pszText  = GlobalAlloc(GPTR,iSelLength);
      LPWSTR pszTextW = GlobalAlloc(GPTR,(iSelLength*sizeof(WCHAR)));

      if (pszText == NULL || pszTextW == NULL) {
        GlobalFree(pszText);
        GlobalFree(pszTextW);
        return;
      }
      SciCall_GetSelText(pszText);

      UINT cpEdit = Encoding_SciGetCodePage(hwnd);
      int cchTextW = MultiByteToWideChar(cpEdit,0,pszText,(int)(iSelLength-1),pszTextW,(int)iSelLength);

      BOOL bChanged = FALSE;
      LPWSTR pszMappedW = LocalAlloc(LPTR,GlobalSize(pszTextW));
      // first make lower case, before applying TitleCase
      if (LCMapString(LOCALE_SYSTEM_DEFAULT,(LCMAP_LINGUISTIC_CASING | LCMAP_LOWERCASE), pszTextW,cchTextW,pszMappedW,(int)iSelLength)) 
      {
        if (LCMapString(LOCALE_SYSTEM_DEFAULT,LCMAP_TITLECASE,pszMappedW,cchTextW,pszTextW,(int)iSelLength)) {
          bChanged = TRUE;
        }
      }
      LocalFree(pszMappedW);

      if (bChanged) {

        WideCharToMultiByte(cpEdit,0,pszTextW,cchTextW,pszText,(int)GlobalSize(pszText),NULL,NULL);

        SciCall_Clear();
        SciCall_AddText((iSelEnd - iSelStart), pszText);
        SciCall_SetSel(iAnchorPos, iCurPos);
      }

      GlobalFree(pszText);
      GlobalFree(pszTextW);
    }
    else
      MsgBox(MBWARN,IDS_SELRECT);
  }
}


//=============================================================================
//
//  EditSentenceCase()
//
void EditSentenceCase(HWND hwnd)
{
  const DocPos iCurPos = SciCall_GetCurrentPos();
  const DocPos iAnchorPos = SciCall_GetAnchor();

  if (iCurPos != iAnchorPos)
  {
    if (!SciCall_IsSelectionRectangle())
    {
      const DocPos iSelStart = SciCall_GetSelectionStart();
      const DocPos iSelEnd = SciCall_GetSelectionEnd();
      const DocPos iSelLength = SciCall_GetSelText(NULL);

      char*  pszText  = GlobalAlloc(GPTR,iSelLength);
      LPWSTR pszTextW = GlobalAlloc(GPTR,(iSelLength*sizeof(WCHAR)));

      if (pszText == NULL || pszTextW == NULL) {
        GlobalFree(pszText);
        GlobalFree(pszTextW);
        return;
      }
      SciCall_GetSelText(pszText);

      UINT cpEdit  = Encoding_SciGetCodePage(hwnd);
      int cchTextW = MultiByteToWideChar(cpEdit,0,pszText,(int)(iSelLength-1),pszTextW,(int)iSelLength);

      BOOL bChanged = FALSE;
      BOOL bNewSentence = TRUE;
      for (int i = 0; i < cchTextW; i++) {
        if (StrChr(L".;!?\r\n",pszTextW[i])) {
          bNewSentence = TRUE;
        }
        else {
          if (IsCharAlphaNumericW(pszTextW[i])) {
            if (bNewSentence) {
              if (IsCharLowerW(pszTextW[i])) {
                pszTextW[i] = LOWORD(CharUpperW((LPWSTR)(LONG_PTR)MAKELONG(pszTextW[i],0)));
                bChanged = TRUE;
              }
              bNewSentence = FALSE;
            }
            else {
              if (IsCharUpperW(pszTextW[i])) {
                pszTextW[i] = LOWORD(CharLowerW((LPWSTR)(LONG_PTR)MAKELONG(pszTextW[i],0)));
                bChanged = TRUE;
              }
            }
          }
        }
      }

      if (bChanged) {

        WideCharToMultiByte(cpEdit,0,pszTextW,cchTextW,pszText,(int)GlobalSize(pszText),NULL,NULL);

        SciCall_Clear();
        SciCall_AddText((iSelEnd - iSelStart), pszText);
        SciCall_SetSel(iAnchorPos, iCurPos);
      }

      GlobalFree(pszText);
      GlobalFree(pszTextW);
    }
    else
      MsgBox(MBWARN,IDS_SELRECT);
  }
}


//=============================================================================
//
//  EditURLEncode()
//
void EditURLEncode(HWND hwnd)
{
  if (SciCall_IsSelectionEmpty()) { return; }

  if (SciCall_IsSelectionRectangle()) {
    MsgBox(MBWARN, IDS_SELRECT);
    return;
  }

  const DocPos iCurPos = SciCall_GetCurrentPos();
  const DocPos iAnchorPos = SciCall_GetAnchor();
  const DocPos iSelLength = SciCall_GetSelText(NULL);

  const char* pszText = (const char*)SciCall_GetRangePointer(min(iCurPos, iAnchorPos), iSelLength);

  LPWSTR pszTextW = LocalAlloc(LPTR, (iSelLength * sizeof(WCHAR)));
  if (pszTextW == NULL) {
    return;
  }

  UINT cpEdit = Encoding_SciGetCodePage(hwnd);
  /*int cchTextW =*/ MultiByteToWideChar(cpEdit, 0, pszText, (int)(iSelLength-1), pszTextW, (int)iSelLength);

  char* pszEscaped = LocalAlloc(LPTR, iSelLength * 3);
  if (pszEscaped == NULL) {
    LocalFree(pszTextW);
    return;
  }

  LPWSTR pszEscapedW = LocalAlloc(LPTR, LocalSize(pszTextW) * 3);
  if (pszEscapedW == NULL) {
    LocalFree(pszTextW);
    LocalFree(pszEscaped);
    return;
  }

  DWORD cchEscapedW = (int)LocalSize(pszEscapedW) / sizeof(WCHAR);
  UrlEscape(pszTextW, pszEscapedW, &cchEscapedW, URL_ESCAPE_SEGMENT_ONLY | URL_ESCAPE_PERCENT | URL_ESCAPE_AS_UTF8);

  DWORD cchEscaped = WideCharToMultiByte(cpEdit, 0, pszEscapedW, cchEscapedW, pszEscaped, (int)LocalSize(pszEscaped), NULL, NULL);

  EditEnterTargetTransaction();
  if (iCurPos < iAnchorPos)
    SciCall_SetTargetRange(iCurPos, iAnchorPos);
  else
    SciCall_SetTargetRange(iAnchorPos, iCurPos);

  SciCall_ReplaceTarget(cchEscaped, pszEscaped);
  EditLeaveTargetTransaction();
  

  if (iCurPos < iAnchorPos)
    EditSelectEx(hwnd, iCurPos + cchEscaped, iCurPos);
  else
    EditSelectEx(hwnd, iAnchorPos, iAnchorPos + cchEscaped);

  LocalFree(pszTextW);
  LocalFree(pszEscaped);
  LocalFree(pszEscapedW);
}


//=============================================================================
//
//  EditURLDecode()
//
void EditURLDecode(HWND hwnd)
{
  if (SciCall_IsSelectionEmpty()) { return; }

  if (SciCall_IsSelectionRectangle()) {
    MsgBox(MBWARN, IDS_SELRECT);
    return;
  }

  const DocPos iCurPos = SciCall_GetCurrentPos();
  const DocPos iAnchorPos = SciCall_GetAnchor();
  const DocPos iSelLength = SciCall_GetSelText(NULL);

  const char* pszText = (const char*)SciCall_GetRangePointer(min(iCurPos, iAnchorPos), iSelLength);

  LPWSTR pszTextW = LocalAlloc(LPTR, (iSelLength * sizeof(WCHAR)));
  if (pszTextW == NULL) {
    return;
  }

  UINT cpEdit = Encoding_SciGetCodePage(hwnd);
  /*int cchTextW =*/ MultiByteToWideChar(cpEdit, 0, pszText, (int)(iSelLength-1), pszTextW, (int)iSelLength);

  char* pszUnescaped = LocalAlloc(LPTR, iSelLength * 3);
  if (pszUnescaped == NULL) {
    LocalFree(pszTextW);
    return;
  }

  LPWSTR pszUnescapedW = LocalAlloc(LPTR, LocalSize(pszTextW) * 3);
  if (pszUnescapedW == NULL) {
    LocalFree(pszTextW);
    LocalFree(pszUnescaped);
    return;
  }

  DWORD cchUnescapedW = (int)LocalSize(pszUnescapedW) / sizeof(WCHAR);

  UrlUnescapeEx(pszTextW, pszUnescapedW, &cchUnescapedW);

  DWORD cchUnescaped = WideCharToMultiByte(cpEdit, 0, pszUnescapedW, cchUnescapedW, pszUnescaped, (int)LocalSize(pszUnescaped), NULL, NULL);

  EditEnterTargetTransaction();
  if (iCurPos < iAnchorPos)
    SciCall_SetTargetRange(iCurPos, iAnchorPos);
  else
    SciCall_SetTargetRange(iAnchorPos, iCurPos);

  SciCall_ReplaceTarget(cchUnescaped, pszUnescaped);
  EditLeaveTargetTransaction();

  if (iCurPos < iAnchorPos)
    EditSelectEx(hwnd, iCurPos + cchUnescaped, iCurPos);
  else 
    EditSelectEx(hwnd, iAnchorPos, iAnchorPos + cchUnescaped);

  LocalFree(pszTextW);
  LocalFree(pszUnescaped);
  LocalFree(pszUnescapedW);

}


//=============================================================================
//
//  EditEscapeCChars()
//
void EditEscapeCChars(HWND hwnd) {

  if (!SciCall_IsSelectionEmpty())
  {
    if (SciCall_IsSelectionRectangle())
    {
      MsgBox(MBWARN, IDS_SELRECT);
      return;
    }

    EDITFINDREPLACE efr = EFR_INIT_DATA;
    efr.hwnd = hwnd;

    StringCchCopyA(efr.szFind,FNDRPL_BUFFER,"\\");
    StringCchCopyA(efr.szReplace,FNDRPL_BUFFER,"\\\\");
    EditReplaceAllInSelection(hwnd,&efr,FALSE);

    StringCchCopyA(efr.szFind,FNDRPL_BUFFER,"\"");
    StringCchCopyA(efr.szReplace,FNDRPL_BUFFER,"\\\"");
    EditReplaceAllInSelection(hwnd,&efr,FALSE);

    StringCchCopyA(efr.szFind,FNDRPL_BUFFER,"\'");
    StringCchCopyA(efr.szReplace,FNDRPL_BUFFER,"\\\'");
    EditReplaceAllInSelection(hwnd,&efr,FALSE);
  }
}


//=============================================================================
//
//  EditUnescapeCChars()
//
void EditUnescapeCChars(HWND hwnd) {

  if (!SciCall_IsSelectionEmpty())
  {
    if (SciCall_IsSelectionRectangle())
    {
      MsgBox(MBWARN, IDS_SELRECT);
      return;
    }

    EDITFINDREPLACE efr = EFR_INIT_DATA;
    efr.hwnd = hwnd;

    StringCchCopyA(efr.szFind,FNDRPL_BUFFER,"\\\\");
    StringCchCopyA(efr.szReplace,FNDRPL_BUFFER,"\\");
    EditReplaceAllInSelection(hwnd,&efr,FALSE);

    StringCchCopyA(efr.szFind,FNDRPL_BUFFER,"\\\"");
    StringCchCopyA(efr.szReplace,FNDRPL_BUFFER,"\"");
    EditReplaceAllInSelection(hwnd,&efr,FALSE);

    StringCchCopyA(efr.szFind,FNDRPL_BUFFER,"\\\'");
    StringCchCopyA(efr.szReplace,FNDRPL_BUFFER,"\'");
    EditReplaceAllInSelection(hwnd,&efr,FALSE);
  }
}


//=============================================================================
//
//  EditChar2Hex()
//
void EditChar2Hex(HWND hwnd) {

  if (SciCall_IsSelectionRectangle()) {
    MsgBox(MBWARN, IDS_SELRECT);
    return;
  }

  const DocPos iCurPos = SciCall_GetCurrentPos();
  const DocPos iAnchorPos = SciCall_GetAnchor();
  const DocPos iSelStart = SciCall_GetSelectionStart();
  DocPos iSelEnd = SciCall_GetSelectionEnd();

  if (iCurPos == iAnchorPos) {
    iSelEnd = SciCall_PositionAfter(iCurPos);
  }

  char  ch[32] = { '\0' };
  WCHAR wch[32] = { L'\0' };

  EditSelectEx(hwnd, iSelStart, iSelEnd);
  
  //TODO: iterate over complete selection?
  if (SciCall_GetSelText(NULL) <= COUNTOF(ch)) {
    SciCall_GetSelText(ch);
  }

  if (ch[0] == '\0') {
    StringCchCopyA(ch, COUNTOF(ch), "\\x00");
  }
  else {
    UINT cp = Encoding_SciGetCodePage(hwnd);
    MultiByteToWideCharStrg(cp, ch, wch);
    if (wch[0] <= 0xFF)
      StringCchPrintfA(ch, COUNTOF(ch), "\\x%02X", wch[0] & 0xFF);
    else
      StringCchPrintfA(ch, COUNTOF(ch), "\\u%04X", wch[0]);
  }
  SendMessage(hwnd, SCI_REPLACESEL, 0, (LPARAM)ch);

  const DocPos iReplLen = StringCchLenA(ch, COUNTOF(ch));

  if (iCurPos < iAnchorPos) {
    EditSelectEx(hwnd, iCurPos + iReplLen, iCurPos);
  }
  else if (iCurPos > iAnchorPos) {
    EditSelectEx(hwnd, iAnchorPos, iAnchorPos + iReplLen);
  }
  else { // empty selection
    EditSelectEx(hwnd, iCurPos + iReplLen, iCurPos + iReplLen);
  }
}


//=============================================================================
//
//  EditHex2Char()
//
void EditHex2Char(HWND hwnd) 
{
  if (SciCall_IsSelectionEmpty()) { return; }
    
  if (SciCall_IsSelectionRectangle()) {
    MsgBox(MBWARN, IDS_SELRECT);
    return;
  }

  DocPos iCurPos = SciCall_GetCurrentPos();
  DocPos iAnchorPos = SciCall_GetAnchor();
  DocPos iSelStart = SciCall_GetSelectionStart();
  const DocPos iSelEnd = SciCall_GetSelectionEnd();

  char ch[32] = { L'\0' };
  if (SciCall_GetSelText(NULL) <= COUNTOF(ch))
  {
    BOOL bTrySelExpand = FALSE;

    SciCall_GetSelText(ch);

    if (StrChrIA(ch, ' ') || StrChrIA(ch, '\t') || StrChrIA(ch, '\r') || StrChrIA(ch, '\n') || StrChrIA(ch, '-')) {
      return;
    }

    if (StrCmpNIA(ch, "\\x", 2) == 0 || StrCmpNIA(ch, "\\u", 2) == 0) {
      ch[0] = '0';
      ch[1] = 'x';
    }
    else if (StrChrIA("xu", ch[0])) {
      ch[0] = '0';
      bTrySelExpand = TRUE;
    }
    else
      return;

    int i = 0;
    if (sscanf_s(ch, "%x", &i) == 1) {
      int cch = 0;
      if (i == 0) {
        ch[0] = 0;
        cch = 1;
      }
      else {
        WCHAR wch[8] = { L'\0' };
        UINT  cp = Encoding_SciGetCodePage(hwnd);
        StringCchPrintfW(wch, COUNTOF(wch), L"%lc", (WCHAR)i);
        cch = WideCharToMultiByteStrg(cp, wch, ch) - 1;

        if (bTrySelExpand && (char)SendMessage(hwnd, SCI_GETCHARAT, (WPARAM)iSelStart - 1, 0) == '\\') {
          --iSelStart;
          if (iCurPos < iAnchorPos) { --iCurPos; } else { --iAnchorPos; }
        }
      }
      EditSelectEx(hwnd, iSelStart, iSelEnd);
      SendMessage(hwnd, SCI_REPLACESEL, 0, (LPARAM)ch);

      if (iCurPos < iAnchorPos)
        EditSelectEx(hwnd, iCurPos + cch, iCurPos);
      else
        EditSelectEx(hwnd, iAnchorPos, iAnchorPos + cch);

    }
  }
}


//=============================================================================
//
//  EditFindMatchingBrace()
//
void EditFindMatchingBrace(HWND hwnd)
{
  BOOL bIsAfter = FALSE;
  DocPos iMatchingBracePos = (DocPos)-1;
  const DocPos iCurPos = SciCall_GetCurrentPos();
  const char c = SciCall_GetCharAt(iCurPos);
  if (StrChrA("()[]{}", c)) {
    iMatchingBracePos = (DocPos)SendMessage(hwnd, SCI_BRACEMATCH, iCurPos, 0);
  }
  else { // Try one before
    const DocPos iPosBefore = SciCall_PositionBefore(iCurPos);
    const char cb = SciCall_GetCharAt(iPosBefore);
    if (StrChrA("()[]{}", cb)) {
      iMatchingBracePos = (DocPos)SendMessage(hwnd, SCI_BRACEMATCH, iPosBefore, 0);
    }
    bIsAfter = TRUE;
  }
  if (iMatchingBracePos != (DocPos)-1) {
    iMatchingBracePos = bIsAfter ? iMatchingBracePos : SciCall_PositionAfter(iMatchingBracePos);
    EditSelectEx(hwnd, iMatchingBracePos, iMatchingBracePos);
  }
}


//=============================================================================
//
//  EditSelectToMatchingBrace()
//
void EditSelectToMatchingBrace(HWND hwnd)
{
  BOOL bIsAfter = FALSE;
  DocPos iMatchingBracePos = -1;
  const DocPos iCurPos = SciCall_GetCurrentPos();
  const char c = SciCall_GetCharAt(iCurPos);
  if (StrChrA("()[]{}", c)) {
    iMatchingBracePos = (DocPos)SendMessage(hwnd, SCI_BRACEMATCH, iCurPos, 0);
  }
  else { // Try one before
    const DocPos iPosBefore = SciCall_PositionBefore(iCurPos);
    const char cb = SciCall_GetCharAt(iPosBefore);
    if (StrChrA("()[]{}", cb)) {
      iMatchingBracePos = (DocPos)SendMessage(hwnd, SCI_BRACEMATCH, iPosBefore, 0);
    }
    bIsAfter = TRUE;
  }
  if (iMatchingBracePos != (DocPos)-1) {
    if (bIsAfter)
      EditSelectEx(hwnd, iCurPos, iMatchingBracePos);
    else
      EditSelectEx(hwnd, iCurPos, SciCall_PositionAfter(iMatchingBracePos));
  }
}


//=============================================================================
//
//  EditModifyNumber()
//
void EditModifyNumber(HWND hwnd,BOOL bIncrease) {

  if (SciCall_IsSelectionRectangle()) {
    MsgBox(MBWARN, IDS_SELRECT);
    return;
  }

  const DocPos iSelStart = SciCall_GetSelectionStart();
  const DocPos iSelEnd = SciCall_GetSelectionEnd();

  if ((iSelEnd - iSelStart) > 0) {
    char chNumber[32] = { '\0' };
    if (SciCall_GetSelText(NULL) <= COUNTOF(chNumber)) 
    {
      SciCall_GetSelText(chNumber);

      if (StrChrIA(chNumber, '-'))
        return;

      int iNumber;
      int iWidth;
      char chFormat[32] = { '\0' };
      if (!StrChrIA(chNumber, 'x') && sscanf_s(chNumber, "%d", &iNumber) == 1) {
        iWidth = StringCchLenA(chNumber, COUNTOF(chNumber));
        if (iNumber >= 0) {
          if (bIncrease && iNumber < INT_MAX)
            iNumber++;
          if (!bIncrease && iNumber > 0)
            iNumber--;

          StringCchPrintfA(chFormat, COUNTOF(chFormat), "%%0%ii", iWidth);
          StringCchPrintfA(chNumber, COUNTOF(chNumber), chFormat, iNumber);
          SciCall_ReplaceSel(chNumber);
          SciCall_SetSel(iSelStart, iSelStart + StringCchLenA(chNumber, COUNTOF(chNumber)));
        }
      }
      else if (sscanf_s(chNumber, "%x", &iNumber) == 1) {
        BOOL bUppercase = FALSE;
        iWidth = StringCchLenA(chNumber, COUNTOF(chNumber)) - 2;
        if (iNumber >= 0) {
          if (bIncrease && iNumber < INT_MAX)
            iNumber++;
          if (!bIncrease && iNumber > 0)
            iNumber--;
          for (int i = StringCchLenA(chNumber, COUNTOF(chNumber)) - 1; i >= 0; i--) {
            if (IsCharLowerA(chNumber[i]))
              break;
            else if (IsCharUpper(chNumber[i])) {
              bUppercase = TRUE;
              break;
            }
          }
          if (bUppercase)
            StringCchPrintfA(chFormat, COUNTOF(chFormat), "%%#0%iX", iWidth);
          else
            StringCchPrintfA(chFormat, COUNTOF(chFormat), "%%#0%ix", iWidth);

          StringCchPrintfA(chNumber, COUNTOF(chNumber), chFormat, iNumber);
          SciCall_ReplaceSel(chNumber);
          SciCall_SetSel(iSelStart, iSelStart + StringCchLenA(chNumber, COUNTOF(chNumber)));
        }
      }
    }
  }
  UNUSED(hwnd);
}


//=============================================================================
//
//  EditTabsToSpaces()
//
void EditTabsToSpaces(HWND hwnd,int nTabWidth,BOOL bOnlyIndentingWS)
{
  if (SciCall_IsSelectionEmpty()) { return; } // no selection

  if (SciCall_IsSelectionRectangle()) {
    MsgBox(MBWARN,IDS_SELRECT);
    return;
  }

  DocPos iCurPos    = SciCall_GetCurrentPos();
  DocPos iAnchorPos = SciCall_GetAnchor();

  DocPos iSelStart = SciCall_GetSelectionStart();
  //DocLn iLine = SciCall_LineFromPosition(iSelStart);
  //iSelStart = SciCall_PositionFromLine(iLine);   // re-base selection to start of line
  DocPos iSelEnd = SciCall_GetSelectionEnd();
  DocPos iSelCount = (iSelEnd - iSelStart);


  const char* pszText = SciCall_GetRangePointer(iSelStart, iSelCount);

  LPWSTR pszTextW = GlobalAlloc(GPTR, (iSelCount + 1) * sizeof(WCHAR));
  if (pszTextW == NULL) { return; }

  UINT cpEdit = Encoding_SciGetCodePage(hwnd);
  int cchTextW = MultiByteToWideChar(cpEdit,0,pszText,(int)iSelCount,pszTextW,(int)iSelCount+1);

  LPWSTR pszConvW = GlobalAlloc(GPTR,cchTextW*sizeof(WCHAR)*nTabWidth+2);
  if (pszConvW == NULL) {
    GlobalFree(pszTextW);
    return;
  }

  int cchConvW = 0;

  // Contributed by Homam
  // Thank you very much!
  int i = 0;
  BOOL bIsLineStart = TRUE;
  BOOL bModified = FALSE;
  for (int iTextW = 0; iTextW < cchTextW; iTextW++)
  {
    WCHAR w = pszTextW[iTextW];
    if (w == L'\t' && (!bOnlyIndentingWS || bIsLineStart)) {
      for (int j = 0; j < nTabWidth - i % nTabWidth; j++)
        pszConvW[cchConvW++] = L' ';
      i = 0;
      bModified = TRUE;
    }
    else {
      i++;
      if (w == L'\n' || w == L'\r') {
        i = 0;
        bIsLineStart = TRUE;
      }
      else if (w != L' ')
        bIsLineStart = FALSE;
      pszConvW[cchConvW++] = w;
    }
  }

  GlobalFree(pszTextW);

  if (bModified) {
    char* pszText2 = GlobalAlloc(GPTR,cchConvW*3);

    int cchConvM = WideCharToMultiByte(cpEdit,0,pszConvW,cchConvW,pszText2,(int)GlobalSize(pszText2),NULL,NULL);
    GlobalFree(pszConvW);

    if (iCurPos < iAnchorPos) {
      iCurPos = iSelStart;
      iAnchorPos = iSelStart + cchConvM;
    }
    else {
      iAnchorPos = iSelStart;
      iCurPos = iSelStart + cchConvM;
    }

    EditEnterTargetTransaction();
    SendMessage(hwnd, SCI_SETTARGETRANGE, iSelStart, iSelEnd);
    SendMessage(hwnd, SCI_REPLACETARGET, (WPARAM)cchConvM, (LPARAM)pszText2);
    EditLeaveTargetTransaction();

    EditSelectEx(hwnd, iAnchorPos, iCurPos);

    GlobalFree(pszText2);
  }
  else
    GlobalFree(pszConvW);
}


//=============================================================================
//
//  EditSpacesToTabs()
//
void EditSpacesToTabs(HWND hwnd,int nTabWidth,BOOL bOnlyIndentingWS)
{
  if (SciCall_IsSelectionEmpty()) { return; } // no selection

  if (SciCall_IsSelectionRectangle()) {
    MsgBox(MBWARN, IDS_SELRECT);
    return;
  }

  DocPos iCurPos = SciCall_GetCurrentPos();
  DocPos iAnchorPos = SciCall_GetAnchor();

  DocPos iSelStart = SciCall_GetSelectionStart();
  //DocLn iLine = SciCall_LineFromPosition(iSelStart);
  //iSelStart = SciCall_PositionFromLine(iLine);   // re-base selection to start of line
  DocPos iSelEnd = SciCall_GetSelectionEnd();
  DocPos iSelCount = (iSelEnd - iSelStart);

  const char* pszText = SciCall_GetRangePointer(iSelStart, iSelCount);

  LPWSTR pszTextW = GlobalAlloc(GPTR, (iSelCount + 1) * sizeof(WCHAR));
  if (pszTextW == NULL)
  {
    return;
  }

  UINT cpEdit = Encoding_SciGetCodePage(hwnd);
  int cchTextW = MultiByteToWideChar(cpEdit,0,pszText,(int)iSelCount,pszTextW,(int)iSelCount+1);

  LPWSTR pszConvW = GlobalAlloc(GPTR,cchTextW*sizeof(WCHAR)+2);
  if (pszConvW == NULL) {
    GlobalFree(pszTextW);
    return;
  }

  int cchConvW = 0;

  // Contributed by Homam
  // Thank you very much!
  int i = 0;
  int j = 0;
  BOOL bIsLineStart = TRUE;
  BOOL bModified = FALSE;
  WCHAR space[256] = { L'\0' };
  for (int iTextW = 0; iTextW < cchTextW; iTextW++)
  {
    WCHAR w = pszTextW[iTextW];
    if ((w == L' ' || w == L'\t') && (!bOnlyIndentingWS || bIsLineStart)) {
      space[j++] = w;
      if (j == nTabWidth - i % nTabWidth || w == L'\t') {
        if (j > 1 || pszTextW[iTextW+1] == L' ' || pszTextW[iTextW+1] == L'\t')
          pszConvW[cchConvW++] = L'\t';
        else
          pszConvW[cchConvW++] = w;
        i = j = 0;
        bModified = bModified || (w != pszConvW[cchConvW-1]);
      }
    }
    else {
      i += j + 1;
      if (j > 0) {
        //space[j] = '\0';
        for (int t = 0; t < j; t++)
          pszConvW[cchConvW++] = space[t];
        j = 0;
      }
      if (w == L'\n' || w == L'\r') {
        i = 0;
        bIsLineStart = TRUE;
      }
      else
        bIsLineStart = FALSE;
      pszConvW[cchConvW++] = w;
    }
  }
  if (j > 0) {
    for (int t = 0; t < j; t++)
      pszConvW[cchConvW++] = space[t];
    }

  GlobalFree(pszTextW);

  if (bModified || cchConvW != cchTextW) {
    char* pszText2 = GlobalAlloc(GPTR,cchConvW * 3);

    int cchConvM = WideCharToMultiByte(cpEdit,0,pszConvW,cchConvW,pszText2,(int)GlobalSize(pszText2),NULL,NULL);
    GlobalFree(pszConvW);

    if (iAnchorPos > iCurPos) {
      iCurPos = iSelStart;
      iAnchorPos = iSelStart + cchConvM;
    }
    else {
      iAnchorPos = iSelStart;
      iCurPos = iSelStart + cchConvM;
    }

    EditEnterTargetTransaction();
    SendMessage(hwnd, SCI_SETTARGETRANGE, iSelStart, iSelEnd);
    SendMessage(hwnd, SCI_REPLACETARGET, (WPARAM)cchConvM, (LPARAM)pszText2);
    EditLeaveTargetTransaction();

    EditSelectEx(hwnd, iAnchorPos, iCurPos);

    GlobalFree(pszText2);
  }
  else
    GlobalFree(pszConvW);
}


//=============================================================================
//
//  EditMoveUp()
//
void EditMoveUp(HWND hwnd)
{
  
  DocPos iCurPos = SciCall_GetCurrentPos();
  DocPos iAnchorPos = SciCall_GetAnchor();
  const DocLn iCurLine = SciCall_LineFromPosition(iCurPos);
  const DocLn iAnchorLine = SciCall_LineFromPosition(iAnchorPos);

  if (iCurLine == iAnchorLine) 
  {
    const DocPos iLineCurPos = iCurPos - SciCall_PositionFromLine(iCurLine);
    const DocPos iLineAnchorPos = iAnchorPos - SciCall_PositionFromLine(iAnchorLine);
    if (iCurLine > 0) {
      SendMessage(hwnd,SCI_LINETRANSPOSE,0,0);
      SciCall_SetSel(SciCall_PositionFromLine(iAnchorLine - 1) + iLineAnchorPos,
                     SciCall_PositionFromLine(iCurLine - 1) + iLineCurPos);
      SendMessage(hwnd,SCI_CHOOSECARETX,0,0);
    }
  }
  else if (!SciCall_IsSelectionRectangle()) {

    const DocLn iLineSrc = min(iCurLine,iAnchorLine) - 1;
    if (iLineSrc >= 0) {

      const DocPos cLine = SciCall_GetLine(iLineSrc, NULL);
      char* pLine = LocalAlloc(LPTR,cLine+1);
      /*cLine=*/SciCall_GetLine(iLineSrc, pLine);

      const DocPos iLineSrcStart = SciCall_PositionFromLine(iLineSrc);
      const DocPos iLineSrcEnd = SciCall_PositionFromLine(iLineSrc + 1);
      
      DocLn iLineDest = max(iCurLine,iAnchorLine);
      if (max(iCurPos,iAnchorPos) <= SciCall_PositionFromLine(iLineDest)) {
        if (iLineDest >= 1)
          --iLineDest;
      }

      EditEnterTargetTransaction();

      SciCall_SetTargetRange(iLineSrcStart, iLineSrcEnd);
      SciCall_ReplaceTarget(0,"");

      const DocPos iLineDestStart = SciCall_PositionFromLine(iLineDest);
      SciCall_InsertText(iLineDestStart, pLine);

      LocalFree(pLine);

      if (iLineDest == (SciCall_GetLineCount() - 1)) 
      {
        char chaEOL[] = "\r\n";
        int iEOLMode = (int)SendMessage(hwnd,SCI_GETEOLMODE,0,0);
        if (iEOLMode == SC_EOL_CR)
          chaEOL[1] = 0;
        else if (iEOLMode == SC_EOL_LF) {
          chaEOL[0] = '\n';
          chaEOL[1] = 0;
        }
        SciCall_InsertText(iLineDestStart, chaEOL);
        SciCall_SetTargetRange(SciCall_GetLineEndPosition(iLineDest), SciCall_GetTextLength());
        SciCall_ReplaceTarget(0, "");
      }

      EditLeaveTargetTransaction();

      if (iCurPos < iAnchorPos) {
        iCurPos = SciCall_PositionFromLine(iCurLine - 1);
        iAnchorPos = SciCall_PositionFromLine(iLineDest);
      }
      else {
        iAnchorPos = SciCall_PositionFromLine(iAnchorLine - 1);
        iCurPos = SciCall_PositionFromLine(iLineDest);
      }
      EditSelectEx(hwnd, iAnchorPos, iCurPos);
    }
  }
  else
    MsgBox(MBWARN,IDS_SELRECT);
}


//=============================================================================
//
//  EditMoveDown()
//
void EditMoveDown(HWND hwnd)
{
  DocPos iCurPos = SciCall_GetCurrentPos();
  DocPos iAnchorPos = SciCall_GetAnchor();
  DocLn iCurLine = SciCall_LineFromPosition(iCurPos);
  DocLn iAnchorLine = SciCall_LineFromPosition(iAnchorPos);

  if (iCurLine == iAnchorLine) 
  {
    DocPos iLineCurPos = iCurPos - SciCall_PositionFromLine(iCurLine);
    DocPos iLineAnchorPos = iAnchorPos - SciCall_PositionFromLine(iAnchorLine);
    if (iCurLine < (SciCall_GetLineCount() - 1)) {
      SciCall_GotoLine(iCurLine + 1);
      SendMessage(hwnd, SCI_LINETRANSPOSE, 0, 0);
      SciCall_SetSel(SciCall_PositionFromLine(iAnchorLine + 1) + iLineAnchorPos,
                     SciCall_PositionFromLine(iCurLine + 1) + iLineCurPos);
      SendMessage(hwnd, SCI_CHOOSECARETX, 0, 0);
    }
  }
  else if (!SciCall_IsSelectionRectangle())
  {
    DocLn iLineSrc = max(iCurLine,iAnchorLine) + 1;
    if (max(iCurPos,iAnchorPos) <= SciCall_PositionFromLine(iLineSrc - 1)) {
      if (iLineSrc >= 1)
        --iLineSrc;
    }

    if (iLineSrc <= (DocLn)SendMessage(hwnd,SCI_GETLINECOUNT,0,0) - 1) {

      const BOOL bLastLine = (iLineSrc == (SciCall_GetLineCount() - 1));

      if (bLastLine &&
          (SciCall_GetLineEndPosition(iLineSrc) - SciCall_PositionFromLine(iLineSrc) == 0) &&
          (SciCall_GetLineEndPosition(iLineSrc-1) - SciCall_PositionFromLine(iLineSrc-1) == 0))
        return;

      if (bLastLine) {
        char chaEOL[] = "\r\n";
        int iEOLMode = (int)SendMessage(hwnd,SCI_GETEOLMODE,0,0);
        if (iEOLMode == SC_EOL_CR)
          chaEOL[1] = 0;
        else if (iEOLMode == SC_EOL_LF) {
          chaEOL[0] = '\n';
          chaEOL[1] = 0;
        }
        SendMessage(hwnd,SCI_APPENDTEXT,(WPARAM)strlen(chaEOL),(LPARAM)chaEOL);
      }

      const DocPos cLine = SciCall_GetLine(iLineSrc, NULL);
      char* pLine = LocalAlloc(LPTR,cLine+1);
      SciCall_GetLine(iLineSrc, pLine);

      const DocPos iLineSrcStart = SciCall_PositionFromLine(iLineSrc);
      const DocPos iLineSrcEnd = SciCall_PositionFromLine(iLineSrc + 1);
      const DocLn  iLineDest = min(iCurLine,iAnchorLine);

      EditEnterTargetTransaction();

      SciCall_SetTargetRange(iLineSrcStart, iLineSrcEnd);
      SciCall_ReplaceTarget(0, "");

      const DocPos iLineDestStart = SciCall_PositionFromLine(iLineDest);
      SciCall_InsertText(iLineDestStart, pLine);

      if (bLastLine) {
        SciCall_SetTargetRange(SciCall_GetLineEndPosition(SciCall_GetLineCount() - 2), SciCall_GetTextLength());
        SciCall_ReplaceTarget(0, "");
      }

      EditLeaveTargetTransaction();

      LocalFree(pLine);

      if (iCurPos < iAnchorPos) {
        iCurPos = SciCall_PositionFromLine(iCurLine + 1);
        iAnchorPos = SciCall_PositionFromLine(iLineSrc + 1);
      }
      else {
        iAnchorPos = SciCall_PositionFromLine(iAnchorLine + 1);
        iCurPos = SciCall_PositionFromLine(iLineSrc + 1);
      }
      EditSelectEx(hwnd, iAnchorPos, iCurPos);
    }
  }
  else
    MsgBox(MBWARN,IDS_SELRECT);
}


//=============================================================================
//
//  EditModifyLines()
//
void EditModifyLines(HWND hwnd,LPCWSTR pwszPrefix,LPCWSTR pwszAppend)
{
  BOOL  bAppendNum = FALSE;
  char  mszPrefix1[256*3] = { '\0' };
  char  mszAppend1[256*3] = { '\0' };

  DocPos iSelStart = SciCall_GetSelectionStart();
  DocPos iSelEnd = SciCall_GetSelectionEnd();

  UINT mbcp = Encoding_SciGetCodePage(hwnd);

  if (lstrlen(pwszPrefix))
    WideCharToMultiByteStrg(mbcp,pwszPrefix,mszPrefix1);
  if (lstrlen(pwszAppend))
    WideCharToMultiByteStrg(mbcp,pwszAppend,mszAppend1);

  if (!SciCall_IsSelectionRectangle())
  {
    DocLn iLine;

    DocLn iLineStart = SciCall_LineFromPosition(iSelStart);
    DocLn iLineEnd   = SciCall_LineFromPosition(iSelEnd);

    //if (iSelStart > SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineStart,0))
    //  iLineStart++;
    
    if (iSelEnd <= SciCall_PositionFromLine(iLineEnd))
    {
      if ((iLineEnd - iLineStart) >= 1)
        --iLineEnd;
    }

    BOOL  bPrefixNum = FALSE;
    DocLn iPrefixNum = 0;
    int   iPrefixNumWidth = 1;
    DocLn iAppendNum = 0;
    int   iAppendNumWidth = 1;
    char* pszPrefixNumPad = "";
    char* pszAppendNumPad = "";
    char  mszPrefix2[256*3] = { '\0' };
    char  mszAppend2[256*3] = { '\0' };

    if (StringCchLenA(mszPrefix1,COUNTOF(mszPrefix1))) {


      char* p = StrStrA(mszPrefix1, "$(");
      while (!bPrefixNum && p) {

        if (StrCmpNA(p,"$(I)",CSTRLEN("$(I)")) == 0) {
          *p = 0;
          StringCchCopyA(mszPrefix2,COUNTOF(mszPrefix2),p + CSTRLEN("$(I)"));
          bPrefixNum = TRUE;
          iPrefixNum = 0;
          for (DocLn i = iLineEnd - iLineStart; i >= 10; i = i / 10)
            iPrefixNumWidth++;
          pszPrefixNumPad = "";
        }

        else if (StrCmpNA(p,"$(0I)",CSTRLEN("$(0I)")) == 0) {
          *p = 0;
          StringCchCopyA(mszPrefix2,COUNTOF(mszPrefix2),p + CSTRLEN("$(0I)"));
          bPrefixNum = TRUE;
          iPrefixNum = 0;
          for (DocLn i = iLineEnd - iLineStart; i >= 10; i = i / 10)
            iPrefixNumWidth++;
          pszPrefixNumPad = "0";
        }

        else if (StrCmpNA(p,"$(N)",CSTRLEN("$(N)")) == 0) {
          *p = 0;
          StringCchCopyA(mszPrefix2,COUNTOF(mszPrefix2),p + CSTRLEN("$(N)"));
          bPrefixNum = TRUE;
          iPrefixNum = 1;
          for (DocLn i = iLineEnd - iLineStart + 1; i >= 10; i = i / 10)
            iPrefixNumWidth++;
          pszPrefixNumPad = "";
        }

        else if (StrCmpNA(p,"$(0N)",CSTRLEN("$(0N)")) == 0) {
          *p = 0;
          StringCchCopyA(mszPrefix2,COUNTOF(mszPrefix2),p + CSTRLEN("$(0N)"));
          bPrefixNum = TRUE;
          iPrefixNum = 1;
          for (DocLn i = iLineEnd - iLineStart + 1; i >= 10; i = i / 10)
            iPrefixNumWidth++;
          pszPrefixNumPad = "0";
        }

        else if (StrCmpNA(p,"$(L)",CSTRLEN("$(L)")) == 0) {
          *p = 0;
          StringCchCopyA(mszPrefix2,COUNTOF(mszPrefix2),p + CSTRLEN("$(L)"));
          bPrefixNum = TRUE;
          iPrefixNum = iLineStart+1;
          for (DocLn i = iLineEnd + 1; i >= 10; i = i / 10)
            iPrefixNumWidth++;
          pszPrefixNumPad = "";
        }

        else if (StrCmpNA(p,"$(0L)",CSTRLEN("$(0L)")) == 0) {
          *p = 0;
          StringCchCopyA(mszPrefix2,COUNTOF(mszPrefix2),p + CSTRLEN("$(0L)"));
          bPrefixNum = TRUE;
          iPrefixNum = iLineStart+1;
          for (DocLn i = iLineEnd + 1; i >= 10; i = i / 10)
            iPrefixNumWidth++;
          pszPrefixNumPad = "0";
        }
        p += CSTRLEN("$(");
        p = StrStrA(p, "$("); // next
      }
    }

    if (StringCchLenA(mszAppend1,COUNTOF(mszAppend1))) {

      char* p = StrStrA(mszAppend1, "$(");
      while (!bAppendNum && p) {

        if (StrCmpNA(p,"$(I)",CSTRLEN("$(I)")) == 0) {
          *p = 0;
          StringCchCopyA(mszAppend2,COUNTOF(mszAppend2),p + CSTRLEN("$(I)"));
          bAppendNum = TRUE;
          iAppendNum = 0;
          for (DocLn i = iLineEnd - iLineStart; i >= 10; i = i / 10)
            iAppendNumWidth++;
          pszAppendNumPad = "";
        }

        else if (StrCmpNA(p,"$(0I)",CSTRLEN("$(0I)")) == 0) {
          *p = 0;
          StringCchCopyA(mszAppend2,COUNTOF(mszAppend2),p + CSTRLEN("$(0I)"));
          bAppendNum = TRUE;
          iAppendNum = 0;
          for (DocLn i = iLineEnd - iLineStart; i >= 10; i = i / 10)
            iAppendNumWidth++;
          pszAppendNumPad = "0";
        }

        else if (StrCmpNA(p,"$(N)",CSTRLEN("$(N)")) == 0) {
          *p = 0;
          StringCchCopyA(mszAppend2,COUNTOF(mszAppend2),p + CSTRLEN("$(N)"));
          bAppendNum = TRUE;
          iAppendNum = 1;
          for (DocLn i = iLineEnd - iLineStart + 1; i >= 10; i = i / 10)
            iAppendNumWidth++;
          pszAppendNumPad = "";
        }

        else if (StrCmpNA(p,"$(0N)",CSTRLEN("$(0N)")) == 0) {
          *p = 0;
          StringCchCopyA(mszAppend2,COUNTOF(mszAppend2),p + CSTRLEN("$(0N)"));
          bAppendNum = TRUE;
          iAppendNum = 1;
          for (DocLn i = iLineEnd - iLineStart + 1; i >= 10; i = i / 10)
            iAppendNumWidth++;
          pszAppendNumPad = "0";
        }

        else if (StrCmpNA(p,"$(L)",CSTRLEN("$(L)")) == 0) {
          *p = 0;
          StringCchCopyA(mszAppend2,COUNTOF(mszAppend2),p + CSTRLEN("$(L)"));
          bAppendNum = TRUE;
          iAppendNum = iLineStart+1;
          for (DocLn i = iLineEnd + 1; i >= 10; i = i / 10)
            iAppendNumWidth++;
          pszAppendNumPad = "";
        }

        else if (StrCmpNA(p,"$(0L)",CSTRLEN("$(0L)")) == 0) {
          *p = 0;
          StringCchCopyA(mszAppend2,COUNTOF(mszAppend2),p + CSTRLEN("$(0L)"));
          bAppendNum = TRUE;
          iAppendNum = iLineStart+1;
          for (DocLn i = iLineEnd + 1; i >= 10; i = i / 10)
            iAppendNumWidth++;
          pszAppendNumPad = "0";
        }
        p += CSTRLEN("$(");
        p = StrStrA(p, "$("); // next
      }
    }

    IgnoreNotifyChangeEvent();
    EditEnterTargetTransaction();

    for (iLine = iLineStart; iLine <= iLineEnd; iLine++)
    {
      DocPos iPos;

      if (lstrlen(pwszPrefix)) {

        char mszInsert[512*3] = { '\0' };
        StringCchCopyA(mszInsert,COUNTOF(mszInsert),mszPrefix1);

        if (bPrefixNum) {
          char tchFmt[64] = { '\0' };
          char tchNum[64] = { '\0' };
          StringCchPrintfA(tchFmt,COUNTOF(tchFmt),"%%%s%ii",pszPrefixNumPad,iPrefixNumWidth);
          StringCchPrintfA(tchNum,COUNTOF(tchNum),tchFmt,iPrefixNum);
          StringCchCatA(mszInsert,COUNTOF(mszInsert),tchNum);
          StringCchCatA(mszInsert,COUNTOF(mszInsert),mszPrefix2);
          iPrefixNum++;
        }
        iPos = SciCall_PositionFromLine(iLine);
        SendMessage(hwnd, SCI_SETTARGETRANGE, iPos, iPos);
        SendMessage(hwnd, SCI_REPLACETARGET, (WPARAM)-1, (LPARAM)mszInsert);
      }

      if (lstrlen(pwszAppend)) {

        char mszInsert[512*3] = { '\0' };
        StringCchCopyA(mszInsert,COUNTOF(mszInsert),mszAppend1);

        if (bAppendNum) {
          char tchFmt[64] = { '\0' };
          char tchNum[64] = { '\0' };
          StringCchPrintfA(tchFmt,COUNTOF(tchFmt),"%%%s%ii",pszAppendNumPad,iAppendNumWidth);
          StringCchPrintfA(tchNum,COUNTOF(tchNum),tchFmt,iAppendNum);
          StringCchCatA(mszInsert,COUNTOF(mszInsert),tchNum);
          StringCchCatA(mszInsert,COUNTOF(mszInsert),mszAppend2);
          iAppendNum++;
        }
        iPos = SciCall_GetLineEndPosition(iLine);
        SendMessage(hwnd, SCI_SETTARGETRANGE, iPos, iPos);
        SendMessage(hwnd, SCI_REPLACETARGET, (WPARAM)-1, (LPARAM)mszInsert);
      }
    }

    EditLeaveTargetTransaction();
    ObserveNotifyChangeEvent();

    // extend selection to start of first line
    // the above code is not required when last line has been excluded
    if (iSelStart != iSelEnd)
    {
      DocPos iCurPos = SciCall_GetCurrentPos();
      DocPos iAnchorPos = SciCall_GetAnchor();
      if (iCurPos < iAnchorPos) {
        iCurPos = SciCall_PositionFromLine(iLineStart);
        iAnchorPos = SciCall_PositionFromLine(iLineEnd + 1);
      }
      else {
        iAnchorPos = SciCall_PositionFromLine(iLineStart);
        iCurPos = SciCall_PositionFromLine(iLineEnd + 1);
      }
      EditSelectEx(hwnd, iAnchorPos, iCurPos);
    }
  }
  else
    MsgBox(MBWARN,IDS_SELRECT);
}


//=============================================================================
//
//  EditIndentBlock()
//
void EditIndentBlock(HWND hwnd, int cmd, BOOL bFormatIndentation)
{
  if ((cmd != SCI_TAB) && (cmd != SCI_BACKTAB)) {
    SendMessage(hwnd, cmd, 0, 0);
    return;
  }

  if (SciCall_IsSelectionRectangle()) 
  {
    //if (cmd == SCI_TAB) {
    //  if (g_bTabsAsSpaces) {
    //    int size = (bFormatIndentation ? g_iIndentWidth : g_iTabWidth);
    //    char* pPadStr = LocalAlloc(LPTR, size + 1);
    //    FillMemory(pPadStr, size, ' ');
    //    EditPaste2RectSel(hwnd, pPadStr);
    //    LocalFree(pPadStr);
    //  }
    //  else {
    //    EditPaste2RectSel(hwnd, "\t");
    //  }
    //  return;
    //}
    // better idea:  EditPaste2RectSel(hwnd, pPadStr, pText); pText==NULL => copy single sel

    //TODO: workaround for rectangular selection: make stream selection
    EditSelectEx(hwnd, SciCall_GetAnchor(), SciCall_GetCurrentPos());
  }

  const DocPos iCurPos = SciCall_GetCurrentPos();
  const DocPos iAnchorPos = SciCall_GetAnchor();
  //const DocPos iSelStart = SciCall_GetSelectionStart();
  //const DocPos iSelEnd = SciCall_GetSelectionEnd();
  const DocLn iCurLine = SciCall_LineFromPosition(iCurPos);
  const DocLn iAnchorLine = SciCall_LineFromPosition(iAnchorPos);
  const BOOL bSingleLine = IsSingleLineSelection();

  const BOOL _bTabIndents = (BOOL)SendMessage(hwnd, SCI_GETTABINDENTS, 0, 0);
  const BOOL _bBSpUnindents = (BOOL)SendMessage(hwnd, SCI_GETBACKSPACEUNINDENTS, 0, 0);

  DocPos iDiffCurrent = 0;
  DocPos iDiffAnchor = 0;
  BOOL bFixStart = FALSE;

  if (bSingleLine) {
    if (bFormatIndentation) {
      SendMessage(hwnd, SCI_VCHOME, 0, 0);
      if (SciCall_PositionFromLine(iCurLine) == SciCall_GetCurrentPos()) {
        SendMessage(hwnd, SCI_VCHOME, 0, 0);
      }
      iDiffCurrent = (iCurPos - SciCall_GetCurrentPos());
    }
  }
  else {
    iDiffCurrent = (SciCall_GetLineEndPosition(iCurLine) - iCurPos);
    iDiffAnchor = (SciCall_GetLineEndPosition(iAnchorLine) - iAnchorPos);
    if (iCurPos < iAnchorPos)
      bFixStart = (SciCall_PositionFromLine(iCurLine) == SciCall_GetCurrentPos());
    else
      bFixStart = (SciCall_PositionFromLine(iAnchorLine) == SciCall_GetAnchor());
  }

  if (cmd == SCI_TAB) 
  {
    SendMessage(hwnd, SCI_SETTABINDENTS, (bFormatIndentation ? TRUE : _bTabIndents), 0);
    SendMessage(hwnd, SCI_TAB, 0, 0);
    if (bFormatIndentation)
      SendMessage(hwnd, SCI_SETTABINDENTS, _bTabIndents, 0);
  }
  else  // SCI_BACKTAB
  {
    //if (SciCall_PositionFromLine(iCurLine) != SciCall_GetSelectionStart()) 
    SendMessage(hwnd, SCI_SETBACKSPACEUNINDENTS, (bFormatIndentation ? TRUE : _bBSpUnindents), 0);
    SendMessage(hwnd, SCI_BACKTAB, 0, 0);
    if (bFormatIndentation)
      SendMessage(hwnd, SCI_SETBACKSPACEUNINDENTS, _bBSpUnindents, 0);
  }

  if (bSingleLine) {
    if (bFormatIndentation)
      EditSelectEx(hwnd, SciCall_GetCurrentPos() + iDiffCurrent + (iAnchorPos - iCurPos), SciCall_GetCurrentPos() + iDiffCurrent);
  }
  else {  // on multiline indentation, anchor and current positions are moved to line begin resp. end
    if (bFixStart) {
      if (iCurPos < iAnchorPos)
        iDiffCurrent = SciCall_LineLength(iCurLine) - GetEOLLen();
      else
        iDiffAnchor = SciCall_LineLength(iAnchorLine) - GetEOLLen();
    }
    EditSelectEx(hwnd, SciCall_GetLineEndPosition(iAnchorLine) - iDiffAnchor, SciCall_GetLineEndPosition(iCurLine) - iDiffCurrent);
  }
}


//=============================================================================
//
//  EditAlignText()
//
void EditAlignText(HWND hwnd,int nMode)
{
  #define BUFSIZE_ALIGN 1024

  BOOL  bModified = FALSE;

  const DocPos iSelStart  = SciCall_GetSelectionStart();
  const DocPos iSelEnd    = SciCall_GetSelectionEnd();
  DocPos iCurPos    = SciCall_GetCurrentPos();
  DocPos iAnchorPos = SciCall_GetAnchor();

  if (!SciCall_IsSelectionRectangle())
  {
    DocLn iLine;
    DocPos iMinIndent = BUFSIZE_ALIGN;
    DocPos iMaxLength = 0;

    DocLn iLineStart = SciCall_LineFromPosition(iSelStart);
    DocLn iLineEnd   = SciCall_LineFromPosition(iSelEnd);
    
    if (iSelEnd <= SciCall_PositionFromLine(iLineEnd))
    {
      if ((iLineEnd - iLineStart) >= 1)
        --iLineEnd;
    }

    for (iLine = iLineStart; iLine <= iLineEnd; iLine++) {

      DocPos iLineEndPos    = SciCall_GetLineEndPosition(iLine);
      const DocPos iLineIndentPos = SciCall_GetLineIndentPosition(iLine);

      if (iLineIndentPos != iLineEndPos) 
      {
        const DocPos iIndentCol = (DocPos)SendMessage(hwnd,SCI_GETLINEINDENTATION,(WPARAM)iLine,0);
        DocPos iTail;

        iTail = iLineEndPos-1;
        char ch = (char)SendMessage(hwnd,SCI_GETCHARAT,(WPARAM)iTail,0);
        while (iTail >= iLineStart && (ch == ' ' || ch == '\t'))
        {
          --iTail;
          ch = (char)SendMessage(hwnd,SCI_GETCHARAT,(WPARAM)iTail,0);
          --iLineEndPos;
        }
        const DocPos iEndCol = SciCall_GetColumn(iLineEndPos);

        iMinIndent = min(iMinIndent,iIndentCol);
        iMaxLength = max(iMaxLength,iEndCol);
      }
    }

    UINT mbcp = Encoding_SciGetCodePage(hwnd);

    if (iMaxLength < BUFSIZE_ALIGN) {

      IgnoreNotifyChangeEvent();
      EditEnterTargetTransaction();

      for (iLine = iLineStart; iLine <= iLineEnd; iLine++)
      {
        DocPos iEndPos = SciCall_GetLineEndPosition(iLine);
        DocPos iIndentPos = SciCall_GetLineIndentPosition(iLine);

        if ((iIndentPos == iEndPos) && (iEndPos > 0)) {

          if (!bModified) {
            SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
            bModified = TRUE;
          }
          SendMessage(hwnd, SCI_SETTARGETRANGE, SciCall_PositionFromLine(iLine), iEndPos);
          SendMessage(hwnd, SCI_REPLACETARGET, 0, (LPARAM)"");
        }

        else {

          char  tchLineBuf[BUFSIZE_ALIGN*3] = { '\0' };
          WCHAR wchLineBuf[BUFSIZE_ALIGN*3] = L"";
          WCHAR *pWords[BUFSIZE_ALIGN*3/2];
          WCHAR *p = wchLineBuf;

          int iWords = 0;
          int iWordsLength = 0;
          DocPos cchLine = SciCall_GetLine(iLine, tchLineBuf);

          if (!bModified) {
            SendMessage(hwnd, SCI_BEGINUNDOACTION, 0, 0);
            bModified = TRUE;
          }

          MultiByteToWideChar(mbcp,0,tchLineBuf,(int)cchLine,wchLineBuf,COUNTOF(wchLineBuf));
          StrTrim(wchLineBuf,L"\r\n\t ");

          while (*p) {
            if (*p != L' ' && *p != L'\t') {
              pWords[iWords++] = p++;
              iWordsLength++;
              while (*p && *p != L' ' && *p != L'\t') {
                p++;
                iWordsLength++;
              }
            }
            else
              *p++ = 0;
          }

          if (iWords > 0) {

            if (nMode == ALIGN_JUSTIFY || nMode == ALIGN_JUSTIFY_EX) {

              BOOL bNextLineIsBlank = FALSE;
              if (nMode == ALIGN_JUSTIFY_EX) {

                if (SciCall_GetLineCount() <= iLine+1)
                  bNextLineIsBlank = TRUE;

                else {
                  
                  DocPos iLineEndPos    = SciCall_GetLineEndPosition(iLine + 1);
                  DocPos iLineIndentPos = SciCall_GetLineIndentPosition(iLine + 1);

                  if (iLineIndentPos == iLineEndPos)
                    bNextLineIsBlank = TRUE;
                }
              }

              if ((nMode == ALIGN_JUSTIFY || nMode == ALIGN_JUSTIFY_EX) &&
                  iWords > 1 && iWordsLength >= 2 &&
                  ((nMode != ALIGN_JUSTIFY_EX || !bNextLineIsBlank || iLineStart == iLineEnd) ||
                  (bNextLineIsBlank && iWordsLength > (iMaxLength - iMinIndent) * 0.75))) {

                int iGaps = iWords - 1;
                DocPos iSpacesPerGap = (iMaxLength - iMinIndent - iWordsLength) / iGaps;
                DocPos iExtraSpaces = (iMaxLength - iMinIndent - iWordsLength) % iGaps;
                int i,j;

                WCHAR wchNewLineBuf[BUFSIZE_ALIGN * 3] = { L'\0' };
                int length = BUFSIZE_ALIGN * 3;
                StringCchCopy(wchNewLineBuf,COUNTOF(wchNewLineBuf),pWords[0]);
                p = StrEnd(wchNewLineBuf);

                for (i = 1; i < iWords; i++) {
                  for (j = 0; j < iSpacesPerGap; j++) {
                    *p++ = L' ';
                    *p = 0;
                  }
                  if (i > iGaps - iExtraSpaces) {
                    *p++ = L' ';
                    *p = 0;
                  }
                  StringCchCat(p,(length - StringCchLenW(wchNewLineBuf,COUNTOF(wchNewLineBuf))),pWords[i]);
                  p = StrEnd(p);
                }

                int cch = WideCharToMultiByteStrg(mbcp,wchNewLineBuf,tchLineBuf) - 1;

                SendMessage(hwnd, SCI_SETTARGETRANGE, SciCall_PositionFromLine(iLine), SciCall_GetLineEndPosition(iLine));
                SendMessage(hwnd, SCI_REPLACETARGET, (WPARAM)cch, (LPARAM)tchLineBuf);

                SendMessage(hwnd,SCI_SETLINEINDENTATION,(WPARAM)iLine,(LPARAM)iMinIndent);
              }
              else {

                WCHAR wchNewLineBuf[BUFSIZE_ALIGN] = { L'\0' };
                StringCchCopy(wchNewLineBuf,COUNTOF(wchNewLineBuf),pWords[0]);
                p = StrEnd(wchNewLineBuf);

                for (int i = 1; i < iWords; i++) {
                  *p++ = L' ';
                  *p = 0;
                  StringCchCat(p,(COUNTOF(wchNewLineBuf) - StringCchLenW(wchNewLineBuf,COUNTOF(wchNewLineBuf))),pWords[i]);
                  p = StrEnd(p);
                }

                int cch = WideCharToMultiByteStrg(mbcp,wchNewLineBuf,tchLineBuf) - 1;

                SendMessage(hwnd, SCI_SETTARGETRANGE, SciCall_PositionFromLine(iLine), SciCall_GetLineEndPosition(iLine));
                SendMessage(hwnd, SCI_REPLACETARGET, (WPARAM)cch, (LPARAM)tchLineBuf);

                SendMessage(hwnd, SCI_SETLINEINDENTATION, (WPARAM)iLine, (LPARAM)iMinIndent);
              }
            }
            else {

              DocPos iExtraSpaces = iMaxLength - iMinIndent - iWordsLength - iWords + 1;
              DocPos iOddSpaces   = iExtraSpaces % 2;
              int i;
              DocPos iPos;

              WCHAR wchNewLineBuf[BUFSIZE_ALIGN*3] = L"";
              p = wchNewLineBuf;

              if (nMode == ALIGN_RIGHT) {
                for (i = 0; i < iExtraSpaces; i++)
                  *p++ = L' ';
                *p = 0;
              }
              if (nMode == ALIGN_CENTER) {
                for (i = 1; i < iExtraSpaces - iOddSpaces; i+=2)
                  *p++ = L' ';
                *p = 0;
              }
              for (i = 0; i < iWords; i++) {
                StringCchCat(p,(COUNTOF(wchNewLineBuf) - StringCchLenW(wchNewLineBuf,COUNTOF(wchNewLineBuf))),pWords[i]);
                if (i < iWords - 1)
                  StringCchCat(p,(COUNTOF(wchNewLineBuf) - StringCchLenW(wchNewLineBuf,COUNTOF(wchNewLineBuf))),L" ");
                if (nMode == ALIGN_CENTER && iWords > 1 && iOddSpaces > 0 && i + 1 >= iWords / 2) {
                  StringCchCat(p,(COUNTOF(wchNewLineBuf) - StringCchLenW(wchNewLineBuf,COUNTOF(wchNewLineBuf))),L" ");
                  iOddSpaces--;
                }
                p = StrEnd(p);
              }

              int cch = WideCharToMultiByteStrg(mbcp,wchNewLineBuf,tchLineBuf) - 1;

              if (nMode == ALIGN_RIGHT || nMode == ALIGN_CENTER) {
                SendMessage(hwnd,SCI_SETLINEINDENTATION,(WPARAM)iLine,(LPARAM)iMinIndent);
                iPos = SciCall_GetLineIndentPosition(iLine);
              }
              else
                iPos = SciCall_PositionFromLine(iLine);
              
              SendMessage(hwnd, SCI_SETTARGETRANGE, iPos, SciCall_GetLineEndPosition(iLine));
              SendMessage(hwnd, SCI_REPLACETARGET, (WPARAM)cch, (LPARAM)tchLineBuf);

              if (nMode == ALIGN_LEFT)
                SendMessage(hwnd, SCI_SETLINEINDENTATION, (WPARAM)iLine, (LPARAM)iMinIndent);
            }
          }
        }
      }
      EditLeaveTargetTransaction();
      ObserveNotifyChangeEvent();
    }
    else
      MsgBox(MBINFO, IDS_BUFFERTOOSMALL);

      if (bModified) {
        SendMessage(hwnd, SCI_ENDUNDOACTION, 0, 0);
      }

      if (iCurPos < iAnchorPos) {
        iCurPos = SciCall_PositionFromLine(iLineStart);
        iAnchorPos = SciCall_PositionFromLine(iLineEnd + 1);
      }
      else {
        iAnchorPos = SciCall_PositionFromLine(iLineStart);
        iCurPos = SciCall_PositionFromLine(iLineEnd + 1);
      }
      EditSelectEx(hwnd, iAnchorPos, iCurPos);
  }
  else
    MsgBox(MBWARN, IDS_SELRECT);
}



//=============================================================================
//
//  EditEncloseSelection()
//
void EditEncloseSelection(HWND hwnd, LPCWSTR pwszOpen, LPCWSTR pwszClose)
{
  if (SciCall_IsSelectionRectangle()) {
    MsgBox(MBWARN, IDS_SELRECT);
    return;
  }

  char  mszOpen[256 * 3] = { '\0' };
  char  mszClose[256 * 3] = { '\0' };

  const DocPos iCurPos = SciCall_GetCurrentPos();
  const DocPos iAnchorPos = SciCall_GetAnchor();
  const DocPos iSelStart = SciCall_GetSelectionStart();
  const DocPos iSelEnd = SciCall_GetSelectionEnd();

  UINT mbcp = Encoding_SciGetCodePage(hwnd);

  if (lstrlen(pwszOpen))
    WideCharToMultiByteStrg(mbcp, pwszOpen, mszOpen);
  if (lstrlen(pwszClose))
    WideCharToMultiByteStrg(mbcp, pwszClose, mszClose);

  const DocPos iLenOpen = StringCchLenA(mszOpen, COUNTOF(mszOpen));
  const DocPos iLenClose = StringCchLenA(mszClose, COUNTOF(mszClose));

  EditEnterTargetTransaction();

  if (iLenOpen > 0) {
    SendMessage(hwnd, SCI_SETTARGETRANGE, iSelStart, iSelStart);
    SendMessage(hwnd, SCI_REPLACETARGET, (WPARAM)-1, (LPARAM)mszOpen);
  }

  if (iLenClose > 0) {
    SendMessage(hwnd, SCI_SETTARGETRANGE, iSelEnd + iLenOpen, iSelEnd + iLenOpen);
    SendMessage(hwnd, SCI_REPLACETARGET, (WPARAM)-1, (LPARAM)mszClose);
  }

  EditLeaveTargetTransaction();

  // Fix selection
  EditSelectEx(hwnd, iAnchorPos + iLenOpen, iCurPos + iLenOpen);
}


//=============================================================================
//
//  EditToggleLineComments()
//
void EditToggleLineComments(HWND hwnd, LPCWSTR pwszComment, BOOL bInsertAtStart)
{
  const DocPos iCurPos = SciCall_GetCurrentPos();
  const DocPos iAnchorPos = SciCall_GetAnchor();
  const DocPos iSelStart = SciCall_GetSelectionStart();
  const DocPos iSelEnd = SciCall_GetSelectionEnd();

  const DocPos iSelBegCol = SciCall_GetColumn(iSelStart);

  char  mszComment[256 * 3] = { '\0' };

  if (lstrlen(pwszComment)) {
    UINT mbcp = Encoding_SciGetCodePage(hwnd);
    WideCharToMultiByte(mbcp, 0, pwszComment, -1, mszComment, COUNTOF(mszComment), NULL, NULL);
  }
  const DocPos cchComment = StringCchLenA(mszComment, COUNTOF(mszComment));

  if (SciCall_IsSelectionRectangle() || (cchComment == 0)) {
    MsgBox(MBWARN, IDS_SELRECT);
    return;
  }

  const DocLn iLineStart = SciCall_LineFromPosition(iSelStart);
  DocLn iLineEnd = SciCall_LineFromPosition(iSelEnd);

  if (iSelEnd <= SciCall_PositionFromLine(iLineEnd)) {
    if ((iLineEnd - iLineStart) >= 1)
      --iLineEnd;
  }

  DocPos iSelStartOffset = cchComment;
  DocPos iSelEndOffset = 0;

  DocPos iCommentCol = 0;
  if (!bInsertAtStart) {
    iCommentCol = 1024;
    for (DocLn iLine = iLineStart; iLine <= iLineEnd; iLine++)
    {
      const DocPos iLineEndPos = SciCall_GetLineEndPosition(iLine);
      const DocPos iLineIndentPos = SciCall_GetLineIndentPosition(iLine);
      if (iLineIndentPos != iLineEndPos) {
        const DocPos iIndentColumn = SciCall_GetColumn(iLineIndentPos);
        iCommentCol = min(iCommentCol, iIndentColumn);
      }
    }
  }

  IgnoreNotifyChangeEvent();
  EditEnterTargetTransaction();

  int iAction = 0;

  for (DocLn iLine = iLineStart; iLine <= iLineEnd; iLine++)
  {
    const DocPos iIndentPos = SciCall_GetLineIndentPosition(iLine);
    if (iIndentPos == SciCall_GetLineEndPosition(iLine))  continue;

    char tchBuf[32] = { L'\0' };
    struct Sci_TextRange tr = { { 0, 0 }, NULL };
    tr.chrg.cpMin = (DocPosCR)iIndentPos;
    tr.chrg.cpMax = tr.chrg.cpMin + min(31, (DocPosCR)cchComment);
    tr.lpstrText = tchBuf;
    SendMessage(hwnd, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);

    if (StrCmpNIA(tchBuf, mszComment, (int)cchComment) == 0) {
      switch (iAction) {
      case 0:
        iAction = 2;
      case 2:
        SendMessage(hwnd, SCI_SETTARGETRANGE, iIndentPos, iIndentPos + cchComment);
        SendMessage(hwnd, SCI_REPLACETARGET, 0, (LPARAM)"");
        iSelEndOffset -= cchComment;
        if (iLine == iLineStart) {
          iSelStartOffset = (iSelStart == SciCall_PositionFromLine(iLine)) ? 0 : (0 - cchComment);
        }
        break;
      case 1:
        break;
      }
    }
    else {
      switch (iAction) {
      case 0:
        iAction = 1;
      case 1:
        {
          const DocPos iCommentPos = (DocPos)SendMessage(hwnd, SCI_FINDCOLUMN, (WPARAM)iLine, (LPARAM)iCommentCol);
          SendMessage(hwnd, SCI_INSERTTEXT, (WPARAM)iCommentPos, (LPARAM)mszComment);
          iSelEndOffset += cchComment;
          if (iLine == iLineStart) { 
            iSelStartOffset = (iCommentCol >= iSelBegCol) ? 0 : cchComment;
          }
        }
        break;
      case 2:
        break;
      }
    }
  }

  EditLeaveTargetTransaction();
  ObserveNotifyChangeEvent();

  if (iCurPos < iAnchorPos)
    EditSelectEx(hwnd, iAnchorPos + iSelEndOffset, iCurPos + iSelStartOffset);
  else if (iCurPos > iAnchorPos)
    EditSelectEx(hwnd, iAnchorPos + iSelStartOffset, iCurPos + iSelEndOffset);
  else
    EditSelectEx(hwnd, iAnchorPos + iSelStartOffset, iCurPos + iSelStartOffset);
  
}


//=============================================================================
//
//  EditPadWithSpaces()
//
void EditPadWithSpaces(HWND hwnd,BOOL bSkipEmpty,BOOL bNoUndoGroup)
{
  char *pmszPadStr;
  DocPos iMaxColumn = 0;
  DocLn iLine = 0;
  BOOL bIsRectangular = FALSE;
  BOOL bReducedSelection = FALSE;
  int token = -1;

  DocPos iSelStart = 0;
  DocPos iSelEnd = 0;

  DocLn iLineStart = 0;
  DocLn iLineEnd = 0;

  DocLn iRcCurLine = 0;
  DocLn iRcAnchorLine = 0;
  DocPos iRcCurCol = 0;
  DocPos iRcAnchorCol = 0;

  if (!SciCall_IsSelectionRectangle()) {

    iSelStart = SciCall_GetSelectionStart();
    iSelEnd   = SciCall_GetSelectionEnd();

    iLineStart = SciCall_LineFromPosition(iSelStart);
    iLineEnd   = SciCall_LineFromPosition(iSelEnd);

    if (iLineStart == iLineEnd) {
      iLineStart = 0;
      iLineEnd = SciCall_GetLineCount() - 1;
    }

    else {
      if (iSelEnd <= SciCall_PositionFromLine(iLineEnd)) {
        if ((iLineEnd - iLineStart) >= 1) {
          --iLineEnd;
          bReducedSelection = TRUE;
        }
      }
    }

    for (iLine = iLineStart; iLine <= iLineEnd; iLine++) {
      const DocPos iPos = SciCall_GetLineEndPosition(iLine);
      iMaxColumn = max(iMaxColumn, SciCall_GetColumn(iPos));
    }
  }
  else {

    const DocPos iCurPos = SciCall_GetCurrentPos();
    const DocPos iAnchorPos = SciCall_GetAnchor();

    iRcCurLine = SciCall_LineFromPosition(iCurPos);
    iRcAnchorLine = SciCall_LineFromPosition(iAnchorPos);
    
    iRcCurCol = SciCall_GetColumn(iCurPos);
    iRcAnchorCol = SciCall_GetColumn(iAnchorPos);

    bIsRectangular = TRUE;

    iLineStart = 0;
    iLineEnd = SciCall_GetLineCount() - 1;

    for (iLine = iLineStart; iLine <= iLineEnd; iLine++) {
      DocPos iPos = SciCall_GetLineSelEndPosition(iLine);
      if (iPos != (DocPos)(INVALID_POSITION)) {
        DocPos iCol = SciCall_GetColumn(iPos);
        iMaxColumn = max(iMaxColumn, iCol);
      }
    }
  }

  pmszPadStr = LocalAlloc(LPTR, (iMaxColumn + 2) * sizeof(char));

  if (pmszPadStr) {

    SIZE_T size = LocalSize(pmszPadStr) - sizeof(char);

    FillMemory(pmszPadStr, size, ' ');
    pmszPadStr[size] = '\0';

    if (!bNoUndoGroup) {
      token = BeginUndoAction();
    }
    IgnoreNotifyChangeEvent();
    EditEnterTargetTransaction();

    for (iLine = iLineStart; iLine <= iLineEnd; iLine++) {

      const DocPos iLineSelEndPos = SciCall_GetLineSelEndPosition(iLine);
      if (bIsRectangular && ((DocPos)(INVALID_POSITION) == iLineSelEndPos))
        continue;
      
      const DocPos iPos = SciCall_GetLineEndPosition(iLine);
      if (bIsRectangular && iPos > iLineSelEndPos)
        continue;

      if (bSkipEmpty && (SciCall_PositionFromLine(iLine) >= iPos))
        continue;

      const DocPos iCol = SciCall_GetColumn(iPos);
      //iCol += (DocPos)SendMessage(hwnd, SCI_GETSELECTIONNCARETVIRTUALSPACE, 0, 0);
      const DocPos iPadLen = iMaxColumn - iCol;
      pmszPadStr[iPadLen] = '\0';

      SendMessage(hwnd, SCI_SETTARGETRANGE, iPos, iPos);
      SendMessage(hwnd, SCI_REPLACETARGET, (WPARAM)-1, (LPARAM)pmszPadStr);

      pmszPadStr[iPadLen] = ' ';
    }

    LocalFree(pmszPadStr);

    EditLeaveTargetTransaction();
    ObserveNotifyChangeEvent();
  }

  if (!bNoUndoGroup && (token >= 0)) {
    EndUndoAction(token);
  }

  if (!bIsRectangular && (SciCall_LineFromPosition(iSelStart) != SciCall_LineFromPosition(iSelEnd)))
  {
    DocPos iCurPos = SciCall_GetCurrentPos();
    DocPos iAnchorPos = SciCall_GetAnchor();
    if (iCurPos < iAnchorPos) {
      iCurPos = SciCall_PositionFromLine(iLineStart);
      if (!bReducedSelection)
        iAnchorPos = SciCall_GetLineEndPosition(iLineEnd);
      else
        iAnchorPos = SciCall_PositionFromLine(iLineEnd + 1);
    }
    else {
      iAnchorPos = SciCall_PositionFromLine(iLineStart);
      if (!bReducedSelection)
        iCurPos = SciCall_GetLineEndPosition(iLineEnd);
      else
        iCurPos = SciCall_PositionFromLine(iLineEnd + 1);
    }
    EditSelectEx(hwnd, iAnchorPos, iCurPos);
  }
  else if (bIsRectangular) 
  {
    const DocPos iCurPos = (DocPos)SendMessage(hwnd,SCI_FINDCOLUMN,(WPARAM)iRcCurLine,(LPARAM)iRcCurCol);
    const DocPos iAnchorPos = (DocPos)SendMessage(hwnd,SCI_FINDCOLUMN,(WPARAM)iRcAnchorLine,(LPARAM)iRcAnchorCol);
    SendMessage(hwnd,SCI_SETRECTANGULARSELECTIONCARET,(WPARAM)iCurPos,0);
    SendMessage(hwnd,SCI_SETRECTANGULARSELECTIONANCHOR,(WPARAM)iAnchorPos,0);
  }
}



//=============================================================================
//
//  EditStripFirstCharacter()
//
void EditStripFirstCharacter(HWND hwnd)
{
  DocPos iSelStart = 0;
  DocPos iSelEnd = 0;

  if (SciCall_IsSelectionEmpty()) 
  {
    iSelEnd = SciCall_GetTextLength();
  }
  else {
    if (SciCall_IsSelectionRectangle()) {
      MsgBox(MBWARN, IDS_SELRECT);
      return;
    }
    iSelStart = SciCall_GetSelectionStart();
    iSelEnd = SciCall_GetSelectionEnd();
  }

  const DocLn iLineStart = SciCall_LineFromPosition(iSelStart);
  const DocLn iLineEnd = SciCall_LineFromPosition(iSelEnd);

  IgnoreNotifyChangeEvent();
  EditEnterTargetTransaction();

  for (DocLn iLine = iLineStart; iLine <= iLineEnd; ++iLine) {
    const DocPos iPos = SciCall_PositionFromLine(iLine);
    if (iPos < SciCall_GetLineEndPosition(iLine)) {
      SendMessage(hwnd, SCI_SETTARGETRANGE, (WPARAM)iPos, (LPARAM)SciCall_PositionAfter(iPos));
      SendMessage(hwnd, SCI_REPLACETARGET, 0, (LPARAM)"");
    }
  }

  EditLeaveTargetTransaction();
  ObserveNotifyChangeEvent();
}


//=============================================================================
//
//  EditStripLastCharacter()
//
void EditStripLastCharacter(HWND hwnd, BOOL bIgnoreSelection, BOOL bTrailingBlanksOnly)
{
  DocPos iSelStart = 0;
  DocPos iSelEnd = 0;

  if (SciCall_IsSelectionEmpty() || bIgnoreSelection) {
    iSelEnd = SciCall_GetTextLength();
  }
  else {
    if (SciCall_IsSelectionRectangle()) {
      MsgBox(MBWARN, IDS_SELRECT);
      return;
    }
    iSelStart = SciCall_GetSelectionStart();
    iSelEnd = SciCall_GetSelectionEnd();
  }

  const DocLn iLineStart = SciCall_LineFromPosition(iSelStart);
  const DocLn iLineEnd = SciCall_LineFromPosition(iSelEnd);

  IgnoreNotifyChangeEvent();
  EditEnterTargetTransaction();

  for (DocLn iLine = iLineStart; iLine <= iLineEnd; ++iLine) 
  {
    const DocPos iStartPos = SciCall_PositionFromLine(iLine);
    const DocPos iEndPos = SciCall_GetLineEndPosition(iLine);

    if (bTrailingBlanksOnly)
    {
      DocPos i = iEndPos;
      char ch = '\0';
      do {
        ch = SciCall_GetCharAt(--i);
      } while ((i >= iStartPos) && ((ch == ' ') || (ch == '\t')));
      if ((++i) < iEndPos) {
        SciCall_SetTargetRange(i, iEndPos);
        SciCall_ReplaceTarget(0, "");
      }
    }
    else { // any char at line end
      if (iStartPos < iEndPos) {
        SciCall_SetTargetRange(SciCall_PositionBefore(iEndPos), iEndPos);
        SciCall_ReplaceTarget(0, "");
      }

    }
  }

  EditLeaveTargetTransaction();
  ObserveNotifyChangeEvent();
  UNUSED(hwnd);
}


//=============================================================================
//
//  EditCompressSpaces()
//
void EditCompressSpaces(HWND hwnd)
{
  if (SciCall_IsSelectionRectangle()) {
    MsgBox(MBWARN, IDS_SELRECT);
    return;
  }

  const DocPos iCurPos      = SciCall_GetCurrentPos();
  const DocPos iAnchorPos   = SciCall_GetAnchor();
  const DocPos iSelStartPos = SciCall_GetSelectionStart();
  const DocPos iSelEndPos   = SciCall_GetSelectionEnd();
  const DocPos iSelLength   = (iSelEndPos - iSelStartPos);

  const DocLn iLineStart   = SciCall_LineFromPosition(iSelStartPos);
  const DocLn iLineEnd     = SciCall_LineFromPosition(iSelEndPos);
  const DocPos iTxtLength   = SciCall_GetTextLength();
  const BOOL bIsSelEmpty = SciCall_IsSelectionEmpty();

  const char* pszIn = NULL;  
  char* pszOut = NULL;
  BOOL bIsLineStart = TRUE;
  BOOL bIsLineEnd = TRUE;

  BOOL bModified = FALSE;
  
  DocPos cch = 0;
  if (bIsSelEmpty) {
    pszIn = (const char*)SciCall_GetCharacterPointer();
    cch = iTxtLength;
    pszOut = LocalAlloc(GPTR, cch+1);
  }
  else {
    pszIn = (const char*)SciCall_GetRangePointer(iSelStartPos, iSelLength);
    cch = SciCall_GetSelText(NULL) - 1;
    pszOut = LocalAlloc(LPTR,cch+1);
    bIsLineStart = (iSelStartPos == SciCall_PositionFromLine(iLineStart));
    bIsLineEnd = (iSelEndPos == SciCall_GetLineEndPosition(iLineEnd));
  }
  
  if (pszIn && pszOut) {
    char* co = (char*)pszOut;
    DocPos remWSuntilCaretPos = 0;
    for (int i = 0; i < cch; ++i) {
      if (pszIn[i] == ' ' || pszIn[i] == '\t') {
        if (pszIn[i] == '\t') { bModified = TRUE; }
        while (pszIn[i+1] == ' ' || pszIn[i+1] == '\t') {
          if (bIsSelEmpty && (i < iSelStartPos)) { ++remWSuntilCaretPos; }
          ++i;
          bModified = TRUE;
        }
        if (!bIsLineStart && ((pszIn[i + 1] != '\n') && (pszIn[i + 1] != '\r'))) {
          *co++ = ' ';
        }
        else {
          bModified = TRUE;
        }
      }
      else {
        bIsLineStart = (pszIn[i] == '\n' || pszIn[i] == '\r') ? TRUE : FALSE;
        *co++ = pszIn[i];
      }
    }

    if (bIsLineEnd && (co > pszOut) && (*(co-1) == ' ')) {
      if (bIsSelEmpty && ((cch-1) < iSelStartPos)) { --remWSuntilCaretPos; }
      *--co = '\0';
      bModified = TRUE;
    }

    if (bModified) {

      EditEnterTargetTransaction();

      if (!SciCall_IsSelectionEmpty()) {
        SendMessage(hwnd, SCI_TARGETFROMSELECTION, 0, 0);
      }
      else {
        SendMessage(hwnd, SCI_SETTARGETRANGE, 0, iTxtLength);
      }
      SendMessage(hwnd, SCI_REPLACETARGET, (WPARAM)-1, (LPARAM)pszOut);

      EditLeaveTargetTransaction();

      const DocPos iNewLen = StringCchLenA(pszOut, LocalSize(pszOut));

      if (iCurPos < iAnchorPos) {
        EditSelectEx(hwnd, iCurPos + iNewLen, iCurPos);
      }
      else if (iCurPos > iAnchorPos) {
        EditSelectEx(hwnd, iAnchorPos, iAnchorPos + iNewLen);
      }
      else { // empty selection
        DocPos iNewPos = iCurPos;
        if (iCurPos > 0) {
          iNewPos = SciCall_PositionBefore(SciCall_PositionAfter(iCurPos - remWSuntilCaretPos));
        }
        EditSelectEx(hwnd, iNewPos, iNewPos);
      }
    }
  }
  if (pszOut) { LocalFree(pszOut); }
}


//=============================================================================
//
//  EditRemoveBlankLines()
//
void EditRemoveBlankLines(HWND hwnd,BOOL bMerge)
{
  DocPos iSelStart = SciCall_GetSelectionStart();
  DocPos iSelEnd   = SciCall_GetSelectionEnd();

  if (iSelStart == iSelEnd) {
    iSelStart = 0;
    iSelEnd   = SciCall_GetTextLength();
  }

  if (SciCall_IsSelectionRectangle()) {
    MsgBox(MBWARN, IDS_SELRECT);
    return;
  }

  DocLn iLineStart = SciCall_LineFromPosition(iSelStart);
  DocLn iLineEnd   = SciCall_LineFromPosition(iSelEnd);

  if (iSelStart > SciCall_PositionFromLine(iLineStart)) { ++iLineStart; }
  if ((iSelEnd <= SciCall_PositionFromLine(iLineEnd)) && (iLineEnd != SciCall_GetLineCount() - 1)) { --iLineEnd; }

  EditEnterTargetTransaction();

  for (DocLn iLine = iLineStart; iLine <= iLineEnd; )
  {
    int nBlanks = 0;
    while (((iLine + nBlanks) <= iLineEnd) &&
      (SciCall_PositionFromLine(iLine + nBlanks) == SciCall_GetLineEndPosition(iLine + nBlanks))) {
      ++nBlanks;
    }
    if ((nBlanks == 0) || ((nBlanks == 1) && bMerge)) {
      iLine += (nBlanks + 1);
    }
    else {
      if (bMerge) { --nBlanks; }

      SendMessage(hwnd, SCI_SETTARGETRANGE, SciCall_PositionFromLine(iLine), SciCall_PositionFromLine(iLine + nBlanks));
      SendMessage(hwnd, SCI_REPLACETARGET, 0, (LPARAM)"");

      if (bMerge) { ++iLine; }
      iLineEnd -= nBlanks;
    }
  }

  EditLeaveTargetTransaction();
}


//=============================================================================
//
//  EditWrapToColumn()
//
void EditWrapToColumn(HWND hwnd,int nColumn/*,int nTabWidth*/)
{
  if (SciCall_IsSelectionRectangle()) {
    MsgBox(MBWARN,IDS_SELRECT);
    return;
  }

  DocPos iCurPos = SciCall_GetCurrentPos();
  DocPos iAnchorPos = SciCall_GetAnchor();

  DocPos iSelStart = 0;
  DocPos iSelEnd = SciCall_GetTextLength();
  DocPos iSelCount = SciCall_GetTextLength();

  if (!SciCall_IsSelectionEmpty()) {
    iSelStart = SciCall_GetSelectionStart();
    DocLn iLine = SciCall_LineFromPosition(iSelStart);
    iSelStart = SciCall_PositionFromLine(iLine);   // re-base selection to start of line
    iSelEnd = SciCall_GetSelectionEnd();
    iSelCount = (iSelEnd - iSelStart);
  }

  char* pszText = (char*)SciCall_GetRangePointer(iSelStart, iSelCount);

  LPWSTR pszTextW = GlobalAlloc(GPTR,(iSelCount+2)*sizeof(WCHAR));
  if (pszTextW == NULL) {
    return;
  }
  UINT cpEdit = Encoding_SciGetCodePage(hwnd);
  int cchTextW = MultiByteToWideChar(cpEdit,0,pszText,(int)iSelCount,pszTextW,(int)(GlobalSize(pszTextW)/sizeof(WCHAR)));

  LPWSTR pszConvW = GlobalAlloc(GPTR,cchTextW*sizeof(WCHAR)*3+2);
  if (pszConvW == NULL) {
    GlobalFree(pszTextW);
    return;
  }

  int cchEOL = 2;
  WCHAR wszEOL[] = L"\r\n";
  int cEOLMode = (int)SendMessage(hwnd,SCI_GETEOLMODE,0,0);
  if (cEOLMode == SC_EOL_CR)
    cchEOL = 1;
  else if (cEOLMode == SC_EOL_LF) {
    cchEOL = 1; wszEOL[0] = L'\n';
  }

  int cchConvW = 0;
  int iLineLength = 0;

  //#define W_DELIMITER  L"!\"#$%&'()*+,-./:;<=>?@[\\]^`{|}~"  // underscore counted as part of word
  //WCHAR* W_DELIMITER  = bAccelWordNavigation ? W_DelimCharsAccel : W_DelimChars;
  //#define ISDELIMITER(wc) StrChr(W_DELIMITER,wc)

  //WCHAR* W_WHITESPACE = bAccelWordNavigation ? W_WhiteSpaceCharsAccelerated : W_WhiteSpaceCharsDefault;
  //#define ISWHITE(wc) StrChr(W_WHITESPACE,wc)
  #define ISWHITE(wc) StrChr(L" \t\f",wc)

  //#define ISWORDEND(wc) (ISDELIMITER(wc) || ISWHITE(wc))
  #define ISWORDEND(wc) StrChr(L" \t\f\r\n\v",wc)
  
  int iCaretShift = 0;
  BOOL bModified = FALSE;

  for (int iTextW = 0; iTextW < cchTextW; iTextW++)
  {
    WCHAR w = pszTextW[iTextW];

    if (ISWHITE(w))
    {
      int iNextWordLen = 0;

      while (pszTextW[iTextW+1] == L' ' || pszTextW[iTextW+1] == L'\t') {
        ++iTextW;
        bModified = TRUE;
      }

      WCHAR w2 = pszTextW[iTextW + 1];

      while (w2 != L'\0' && !ISWORDEND(w2)) {
        iNextWordLen++;
        w2 = pszTextW[iTextW + iNextWordLen + 1];
      }

      //if (ISDELIMITER(w2) /*&& iNextWordLen > 0*/) // delimiters go with the word
      //  iNextWordLen++;

      if (iNextWordLen > 0)
      {
        if (iLineLength + iNextWordLen + 1 > nColumn) {
          if (cchConvW <= iCurPos) { ++iCaretShift; };
          pszConvW[cchConvW++] = wszEOL[0];
          if (cchEOL > 1)
            pszConvW[cchConvW++] = wszEOL[1];
          iLineLength = 0;
          bModified = TRUE;
        }
        else {
          if (iLineLength > 0) {
            pszConvW[cchConvW++] = L' ';
            iLineLength++;
          }
        }
      }
    }
    else
    {
      pszConvW[cchConvW++] = w;
      if (w == L'\r' || w == L'\n') {
        iLineLength = 0;
      }
      else {
        iLineLength++;
      }
    }
  }
  GlobalFree(pszTextW);

  if (bModified) 
  {
    pszText = GlobalAlloc(GPTR, cchConvW * 3);
    if (pszText) 
    {
      int cchConvM = WideCharToMultiByte(cpEdit, 0, pszConvW, cchConvW, pszText, (int)GlobalSize(pszText), NULL, NULL);

      if (iCurPos < iAnchorPos) {
        iAnchorPos = iSelStart + cchConvM;
      }
      else if (iCurPos > iAnchorPos) {
        iCurPos = iSelStart + cchConvM;
      }
      else {
        iCurPos += iCaretShift;
        iAnchorPos = iCurPos;
      }

      EditEnterTargetTransaction();
      SendMessage(hwnd, SCI_SETTARGETRANGE, iSelStart, iSelEnd);
      SendMessage(hwnd, SCI_REPLACETARGET, (WPARAM)cchConvM, (LPARAM)pszText);
      EditLeaveTargetTransaction();

      GlobalFree(pszText);

      EditSelectEx(hwnd, iAnchorPos, iCurPos);
    }
  }
  GlobalFree(pszConvW);
}


//=============================================================================
//
//  EditJoinLinesEx()
//
//  Customized version of  SCI_LINESJOIN  (w/o using TARGET transaction)
//
//   ~EditEnterTargetTransaction();
//   ~SciCall_TargetFromSelection();
//   ~SendMessage(g_hwndEdit, SCI_LINESJOIN, 0, 0);
//   ~EditLeaveTargetTransaction();
//
void EditJoinLinesEx(HWND hwnd, BOOL bPreserveParagraphs, BOOL bCRLF2Space)
{
  BOOL bModified = FALSE;

  if (SciCall_IsSelectionEmpty())
    return;

  if (SciCall_IsSelectionRectangle()) {
    MsgBox(MBWARN,IDS_SELRECT);
    return;
  }

  DocPos iCurPos    = SciCall_GetCurrentPos();
  DocPos iAnchorPos = SciCall_GetAnchor();

  DocPos iSelStart = SciCall_GetSelectionStart();
  DocPos iSelEnd = SciCall_GetSelectionEnd();
  DocPos iSelLength = (iSelEnd - iSelStart);

  char* pszText = (char*)SciCall_GetRangePointer(iSelStart, iSelLength);

  char* pszJoin = LocalAlloc(LPTR, iSelLength+1);
  if (pszJoin == NULL) {
    return;
  }

  char szEOL[] = "\r\n";
  int  cchEOL = 2;
  switch ((int)SendMessage(hwnd, SCI_GETEOLMODE, 0, 0)) 
  {
    case SC_EOL_LF:
      szEOL[0] = '\n';
      szEOL[1] = '\0';
      cchEOL = 1;
      break;
    case SC_EOL_CR:
      szEOL[1] = '\0';
      cchEOL = 1;
      break;
    case SC_EOL_CRLF:
    default:
      break;
  }

  DocPos cchJoin = (DocPos)-1;
  for (int i = 0; i < iSelLength; ++i)
  {
    if ((pszText[i] == '\r') || (pszText[i] == '\n')) 
    {
      if ((pszText[i+1] == '\r') || (pszText[i+1] == '\n')) { ++i;  }

      int j = ++i;
      while (StrChrA("\r\n", pszText[j])) { ++j; }  // swallow all next line-breaks
   
      if ((i < j) && (j < iSelLength) && pszText[j] && bPreserveParagraphs)
      {
        for (int k = 0; k < cchEOL; ++k) { pszJoin[++cchJoin] = szEOL[k]; }
        if (bCRLF2Space) {
          for (int k = 0; k < cchEOL; ++k) { pszJoin[++cchJoin] = szEOL[k]; }
        }
      }
      else if ((j < iSelLength) && pszText[j] && bCRLF2Space) 
      { 
        pszJoin[++cchJoin] = ' '; 
      }
      i = j;
      bModified = TRUE;
    }
    if (i < iSelLength) {
      pszJoin[++cchJoin] = pszText[i]; // copy char
    }
  }
  ++cchJoin; // start at -1 

  if (bModified) {
    if (iAnchorPos > iCurPos) {
      iCurPos = iSelStart;
      iAnchorPos = iSelStart + cchJoin;
    }
    else {
      iAnchorPos = iSelStart;
      iCurPos = iSelStart + cchJoin;
    }

    EditEnterTargetTransaction();
    SendMessage(hwnd, SCI_SETTARGETRANGE, iSelStart, iSelEnd);
    SendMessage(hwnd, SCI_REPLACETARGET, (WPARAM)cchJoin, (LPARAM)pszJoin);
    EditLeaveTargetTransaction();

    EditSelectEx(hwnd, iAnchorPos, iCurPos);
  }
  LocalFree(pszJoin);
}


//=============================================================================
//
//  EditSortLines()
//
typedef struct _SORTLINE {
  WCHAR *pwszLine;
  WCHAR *pwszSortEntry;
} SORTLINE;

static FARPROC pfnStrCmpLogicalW;
typedef int (__stdcall *FNSTRCMP)(LPCWSTR,LPCWSTR);

int CmpStd(const void *s1, const void *s2) {
  int cmp = StrCmp(((SORTLINE*)s1)->pwszSortEntry,((SORTLINE*)s2)->pwszSortEntry);
  return (cmp) ? cmp : StrCmp(((SORTLINE*)s1)->pwszLine,((SORTLINE*)s2)->pwszLine);
}

int CmpStdRev(const void *s1, const void *s2) {
  int cmp = -1 * StrCmp(((SORTLINE*)s1)->pwszSortEntry,((SORTLINE*)s2)->pwszSortEntry);
  return (cmp) ? cmp :  -1 * StrCmp(((SORTLINE*)s1)->pwszLine,((SORTLINE*)s2)->pwszLine);
}

int CmpLogical(const void *s1, const void *s2) {
  int cmp = (int)pfnStrCmpLogicalW(((SORTLINE*)s1)->pwszSortEntry,((SORTLINE*)s2)->pwszSortEntry);
  if (cmp == 0)
    cmp = (int)pfnStrCmpLogicalW(((SORTLINE*)s1)->pwszLine,((SORTLINE*)s2)->pwszLine);
  if (cmp)
    return cmp;
  else {
    cmp = StrCmp(((SORTLINE*)s1)->pwszSortEntry,((SORTLINE*)s2)->pwszSortEntry);
    return (cmp) ? cmp : StrCmp(((SORTLINE*)s1)->pwszLine,((SORTLINE*)s2)->pwszLine);
  }
}

int CmpLogicalRev(const void *s1, const void *s2) {
  int cmp = -1 * (int)pfnStrCmpLogicalW(((SORTLINE*)s1)->pwszSortEntry,((SORTLINE*)s2)->pwszSortEntry);
  if (cmp == 0)
    cmp = -1 * (int)pfnStrCmpLogicalW(((SORTLINE*)s1)->pwszLine,((SORTLINE*)s2)->pwszLine);
  if (cmp)
    return cmp;
  else {
    cmp = -1 * StrCmp(((SORTLINE*)s1)->pwszSortEntry,((SORTLINE*)s2)->pwszSortEntry);
    return (cmp) ? cmp : -1 * StrCmp(((SORTLINE*)s1)->pwszLine,((SORTLINE*)s2)->pwszLine);
  }
}


void EditSortLines(HWND hwnd, int iSortFlags)
{
  BOOL bIsRectangular = FALSE;

  DocPos iCurPos = 0;
  DocPos iAnchorPos = 0;
  DocPos iCurPosVS = 0;
  DocPos iAnchorPosVS = 0;
  DocPos iSelStart = 0;
  DocPos iSelEnd = 0;
  DocLn iLineStart = 0;
  DocLn iLineEnd = 0;
  DocPos iSortColumn = 0;

  DocLn  iLine = 0;
  DocPos cchTotal = 0;
  DocPos ichlMax = 3;

  SORTLINE *pLines = NULL;
  char  *pmszResult = NULL;
  char  *pmszBuf = NULL;

  UINT uCodePage = 0;
  DWORD cEOLMode = 0L;
  char mszEOL[] = "\r\n";

  int iTabWidth = 0;

  BOOL bLastDup = FALSE;
  FNSTRCMP pfnStrCmp;

  if ((BOOL)SendMessage(hwnd, SCI_GETSELECTIONEMPTY, 0, 0))
    return; // no selection

  pfnStrCmpLogicalW = GetProcAddress(GetModuleHandle(L"shlwapi"), "StrCmpLogicalW");
  pfnStrCmp = (iSortFlags & SORT_NOCASE) ? StrCmpIW : StrCmpW;

  if (SciCall_IsSelectionRectangle()) {

    bIsRectangular = TRUE;

    iCurPos = (DocPos)SendMessage(hwnd, SCI_GETRECTANGULARSELECTIONCARET, 0, 0);
    iAnchorPos = (DocPos)SendMessage(hwnd, SCI_GETRECTANGULARSELECTIONANCHOR, 0, 0);

    iCurPosVS = (DocPos)SendMessage(hwnd, SCI_GETRECTANGULARSELECTIONCARETVIRTUALSPACE, 0, 0);
    iAnchorPosVS = (DocPos)SendMessage(hwnd, SCI_GETRECTANGULARSELECTIONANCHORVIRTUALSPACE, 0, 0);

    iSelStart = min(iCurPos, iAnchorPos); // (DocPos)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
    iSelEnd = max(iCurPos, iAnchorPos); // (DocPos)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0);

    DocLn iRcCurLine = (DocLn)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iCurPos, 0);
    DocLn iRcAnchorLine = (DocLn)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iAnchorPos, 0);

    DocPos iRcCurCol = (DocPos)SendMessage(hwnd, SCI_GETCOLUMN, (WPARAM)iCurPos, 0);
    DocPos iRcAnchorCol = (DocPos)SendMessage(hwnd, SCI_GETCOLUMN, (WPARAM)iAnchorPos, 0);

    iLineStart = min(iRcCurLine, iRcAnchorLine);
    iLineEnd = max(iRcCurLine, iRcAnchorLine);

    iSortColumn = min(iRcCurCol, iRcAnchorCol);
  }
  else { // stream selection

    iCurPos = SciCall_GetCurrentPos();
    iAnchorPos = SciCall_GetAnchor();

    iSelStart = SciCall_GetSelectionStart(); // min(iCurPos, iAnchorPos)
    iSelEnd = SciCall_GetSelectionEnd();     // max(iCurPos, iAnchorPos)
 
    iLine = SciCall_LineFromPosition(iSelStart);
    iSelStart = SciCall_PositionFromLine(iLine);
    iLineStart = SciCall_LineFromPosition(iSelStart);
    iLineEnd = SciCall_LineFromPosition(iSelEnd);

    if (iSelEnd <= SciCall_PositionFromLine(iLineEnd)) { --iLineEnd; }

    iSortColumn = (UINT)SciCall_GetColumn(iCurPos);
  }

  DocLn iLineCount = iLineEnd - iLineStart + 1;
  if (iLineCount < 2)
    return;

  uCodePage = Encoding_SciGetCodePage(hwnd);
  cEOLMode = (DWORD)SendMessage(hwnd, SCI_GETEOLMODE, 0, 0);
  if (cEOLMode == SC_EOL_CR) {
    mszEOL[1] = 0;
  }
  else if (cEOLMode == SC_EOL_LF) {
    mszEOL[0] = '\n';
    mszEOL[1] = 0;
  }

  iTabWidth = (int)SendMessage(hwnd, SCI_GETTABWIDTH, 0, 0);

  if (bIsRectangular)
    EditPadWithSpaces(hwnd, !(iSortFlags & SORT_SHUFFLE), TRUE);

  pLines = LocalAlloc(LPTR, sizeof(SORTLINE) * iLineCount);
  DocLn i = 0;
  for (iLine = iLineStart; iLine <= iLineEnd; iLine++) {

    const DocPos cchm = SciCall_GetLine(iLine, NULL);

    char* pmsz = LocalAlloc(LPTR, cchm + 1);
    SciCall_GetLine(iLine, pmsz);

    StrTrimA(pmsz, "\r\n");
    cchTotal += cchm;
    ichlMax = max(ichlMax, cchm);

    int cchw = MultiByteToWideChar(uCodePage, 0, pmsz, -1, NULL, 0) - 1;
    if (cchw > 0) {
      int col = 0, tabs = iTabWidth;
      pLines[i].pwszLine = LocalAlloc(LPTR, sizeof(WCHAR) * (cchw + 1));
      MultiByteToWideChar(uCodePage, 0, pmsz, -1, pLines[i].pwszLine, (int)LocalSize(pLines[i].pwszLine) / sizeof(WCHAR));
      pLines[i].pwszSortEntry = pLines[i].pwszLine;
      if (iSortFlags & SORT_COLUMN) {
        while (*(pLines[i].pwszSortEntry)) {
          if (*(pLines[i].pwszSortEntry) == L'\t') {
            if (col + tabs <= iSortColumn) {
              col += tabs;
              tabs = iTabWidth;
              pLines[i].pwszSortEntry = CharNext(pLines[i].pwszSortEntry);
            }
            else
              break;
          }
          else if (col < iSortColumn) {
            col++;
            if (--tabs == 0)
              tabs = iTabWidth;
            pLines[i].pwszSortEntry = CharNext(pLines[i].pwszSortEntry);
          }
          else
            break;
        }
      }
    }
    else {
      pLines[i].pwszLine = StrDup(L"");
      pLines[i].pwszSortEntry = pLines[i].pwszLine;
    }
    LocalFree(pmsz);
    i++;
  }

  if (iSortFlags & SORT_DESCENDING) {
    if (iSortFlags & SORT_LOGICAL && pfnStrCmpLogicalW)
      qsort(pLines, iLineCount, sizeof(SORTLINE), CmpLogicalRev);
    else
      qsort(pLines, iLineCount, sizeof(SORTLINE), CmpStdRev);
  }
  else if (iSortFlags & SORT_SHUFFLE) {
    srand((UINT)GetTickCount());
    for (i = iLineCount - 1; i > 0; i--) {
      int j = rand() % i;
      SORTLINE sLine;
      sLine.pwszLine = pLines[i].pwszLine;
      sLine.pwszSortEntry = pLines[i].pwszSortEntry;
      pLines[i] = pLines[j];
      pLines[j].pwszLine = sLine.pwszLine;
      pLines[j].pwszSortEntry = sLine.pwszSortEntry;
    }
  }
  else {
    if (iSortFlags & SORT_LOGICAL && pfnStrCmpLogicalW)
      qsort(pLines, iLineCount, sizeof(SORTLINE), CmpLogical);
    else
      qsort(pLines, iLineCount, sizeof(SORTLINE), CmpStd);
  }

  DocLn lenRes = cchTotal + 2 * iLineCount + 1;
  pmszResult = LocalAlloc(LPTR, lenRes);
  pmszBuf = LocalAlloc(LPTR, ichlMax + 1);

  for (i = 0; i < iLineCount; i++) {
    BOOL bDropLine = FALSE;
    if (pLines[i].pwszLine && ((iSortFlags & SORT_SHUFFLE) || lstrlen(pLines[i].pwszLine))) {
      if (!(iSortFlags & SORT_SHUFFLE)) {
        if (iSortFlags & SORT_MERGEDUP || iSortFlags & SORT_UNIQDUP || iSortFlags & SORT_UNIQUNIQ) {
          if (i < iLineCount - 1) {
            if (pfnStrCmp(pLines[i].pwszLine, pLines[i + 1].pwszLine) == 0) {
              bLastDup = TRUE;
              bDropLine = (iSortFlags & SORT_MERGEDUP || iSortFlags & SORT_UNIQDUP);
            }
            else {
              bDropLine = (!bLastDup && (iSortFlags & SORT_UNIQUNIQ)) || (bLastDup && (iSortFlags & SORT_UNIQDUP));
              bLastDup = FALSE;
            }
          }
          else {
            bDropLine = (!bLastDup && (iSortFlags & SORT_UNIQUNIQ)) || (bLastDup && (iSortFlags & SORT_UNIQDUP));
            bLastDup = FALSE;
          }
        }
      }
      if (!bDropLine) {
        WideCharToMultiByte(uCodePage, 0, pLines[i].pwszLine, -1, pmszBuf, (int)LocalSize(pmszBuf), NULL, NULL);
        StringCchCatA(pmszResult, lenRes, pmszBuf);
        StringCchCatA(pmszResult, lenRes, mszEOL);
      }
    }
  }

  LocalFree(pmszBuf);

  for (i = 0; i < iLineCount; i++) {
    if (pLines[i].pwszLine)
      LocalFree(pLines[i].pwszLine);
  }
  LocalFree(pLines);

  int iResultLength = StringCchLenA(pmszResult, lenRes);
  if (!bIsRectangular) {
    if (iAnchorPos > iCurPos) {
      iCurPos = iSelStart;
      iAnchorPos = iSelStart + iResultLength;
    }
    else {
      iAnchorPos = iSelStart;
      iCurPos = iSelStart + iResultLength;
    }
  }
  EditEnterTargetTransaction();

  SendMessage(hwnd, SCI_SETTARGETRANGE, SciCall_PositionFromLine(iLineStart), SciCall_PositionFromLine(iLineEnd + 1));
  SendMessage(hwnd, SCI_REPLACETARGET, (WPARAM)-1, (LPARAM)pmszResult);

  EditLeaveTargetTransaction();

  LocalFree(pmszResult);

  if (bIsRectangular) {
    SendMessage(hwnd, SCI_SETRECTANGULARSELECTIONANCHOR, (WPARAM)iAnchorPos, 0);
    SendMessage(hwnd, SCI_SETRECTANGULARSELECTIONCARET, (WPARAM)iCurPos, 0);
    SendMessage(hwnd, SCI_SETRECTANGULARSELECTIONANCHORVIRTUALSPACE, (WPARAM)iAnchorPosVS, 0);
    SendMessage(hwnd, SCI_SETRECTANGULARSELECTIONCARETVIRTUALSPACE, (WPARAM)iCurPosVS, 0);
  }
  else {
    EditSelectEx(hwnd, iAnchorPos, iCurPos);
  }

}


//=============================================================================
//
//  EditSelectEx()
//
void EditSelectEx(HWND hwnd, DocPos iAnchorPos, DocPos iCurrentPos)
{
  if ((iAnchorPos < 0) && (iCurrentPos < 0)) {
    SciCall_SelectAll();
  }
  else if (iAnchorPos < 0) {
    iAnchorPos = 0;
  }
  if (iCurrentPos < 0) {
    iCurrentPos =  SciCall_GetTextLength();
  }

  const DocLn iNewLine = SciCall_LineFromPosition(iCurrentPos);
  const DocLn iAnchorLine = SciCall_LineFromPosition(iAnchorPos);

  // Ensure that the first and last lines of a selection are always unfolded
  // This needs to be done *before* the SCI_SETSEL message
  SciCall_EnsureVisible(iAnchorLine);
  if (iAnchorLine != iNewLine) {  SciCall_EnsureVisible(iNewLine);  }
  
  SciCall_SetSel(iAnchorPos, iCurrentPos);

  SciCall_ScrollRange(iAnchorPos, iCurrentPos);

  // remember x-pos for moving caret vertically
  SciCall_ChooseCaretX();

  UpdateToolbar();
  UpdateStatusbar();
  UNUSED(hwnd);
}


//=============================================================================
//
//  EditJumpTo()
//
void EditJumpTo(HWND hwnd, DocLn iNewLine, DocPos iNewCol)
{
  // jump to end with line set to -1
  if (iNewLine < 0) {
    SendMessage(hwnd, SCI_DOCUMENTEND, 0, 0);
    return;
  }

  const DocLn iMaxLine = SciCall_GetLineCount();

  // Line maximum is iMaxLine - 1 (doc line count starts with 0)
  iNewLine = (min(iNewLine, iMaxLine) - 1);
  const DocPos iLineEndPos = SciCall_GetLineEndPosition(iNewLine);

  // Column minimum is 1
  iNewCol = max(0, min((iNewCol - 1), iLineEndPos));

  const DocPos iNewPos = (DocPos)SendMessage(hwnd, SCI_FINDCOLUMN, (WPARAM)iNewLine, (LPARAM)iNewCol);

  EditSelectEx(hwnd, iNewPos, iNewPos); // <= SCI_GOTOPOS(pos)

}


//=============================================================================
//
//  EditFixPositions()
//
void EditFixPositions(HWND hwnd)
{
  DocPos iMaxPos     = SciCall_GetTextLength();
  DocPos iCurrentPos = SciCall_GetCurrentPos();
  DocPos iAnchorPos  = SciCall_GetAnchor();

  if ((iCurrentPos > 0) && (iCurrentPos < iMaxPos)) 
  {
    DocPos iNewPos = SciCall_PositionAfter( SciCall_PositionBefore(iCurrentPos) );

    if (iNewPos != iCurrentPos) {
      SciCall_SetCurrentPos(iNewPos);
      iCurrentPos = iNewPos;
    }
  }

  if ((iAnchorPos != iCurrentPos) && (iAnchorPos > 0) && (iAnchorPos < iMaxPos)) 
  {
    DocPos iNewPos = SciCall_PositionAfter(SciCall_PositionBefore(iAnchorPos));
    if (iNewPos != iAnchorPos) { 
      SciCall_SetAnchor(iNewPos); 
    }
  }
  UNUSED(hwnd);
}


//=============================================================================
//
//  EditEnsureSelectionVisible()
//
void EditEnsureSelectionVisible(HWND hwnd)
{
  const DocPos iAnchorPos = SciCall_GetAnchor();
  const DocPos iCurrentPos = SciCall_GetCurrentPos();
  EditSelectEx(hwnd, iAnchorPos, iCurrentPos);
  UNUSED(hwnd);
}


//=============================================================================
//
//  EditGetExcerpt()
//
void EditGetExcerpt(HWND hwnd,LPWSTR lpszExcerpt,DWORD cchExcerpt)
{
  const DocPos iCurPos = SciCall_GetCurrentPos();
  const DocPos iAnchorPos = SciCall_GetAnchor();

  if (iCurPos == iAnchorPos || SciCall_IsSelectionRectangle()) {
    StringCchCopy(lpszExcerpt,cchExcerpt,L"");
    return;
  }

  WCHAR tch[256] = { L'\0' };
  struct Sci_TextRange tr = { { 0, 0 }, NULL };
  /*if (iCurPos != iAnchorPos && !SciCall_IsSelectionRectangle()) {*/
  tr.chrg.cpMin = (DocPosCR)SciCall_GetSelectionStart();
  tr.chrg.cpMax = min((tr.chrg.cpMin + (DocPosCR)COUNTOF(tch)), (DocPosCR)SciCall_GetSelectionEnd());
  /*}
  else {
    int iLine = SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iCurPos,0);
    tr.chrg.cpMin = SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLine,0);
    tr.chrg.cpMax = min(SendMessage(hwnd,SCI_GETLINEENDPOSITION,(WPARAM)iLine,0),(LONG)(tr.chrg.cpMin + COUNTOF(tchBuf2)));
  }*/
  tr.chrg.cpMax = min(tr.chrg.cpMax, (DocPosCR)SciCall_GetTextLength());

  char*  pszText  = LocalAlloc(LPTR,(tr.chrg.cpMax - tr.chrg.cpMin)+2);
  LPWSTR pszTextW = LocalAlloc(LPTR,((tr.chrg.cpMax - tr.chrg.cpMin)*2)+2);

  DWORD cch = 0;
  if (pszText && pszTextW) 
  {
    tr.lpstrText = pszText;
    SendMessage(hwnd,SCI_GETTEXTRANGE,0,(LPARAM)&tr);
    UINT cpEdit = Encoding_SciGetCodePage(hwnd);
    MultiByteToWideChar(cpEdit,0,pszText,tr.chrg.cpMax - tr.chrg.cpMin,pszTextW,(int)GlobalSize(pszTextW)/sizeof(WCHAR));

    for (WCHAR* p = pszTextW; *p && cch < COUNTOF(tch)-1; p++) {
      if (*p == L'\r' || *p == L'\n' || *p == L'\t' || *p == L' ') {
        tch[cch++] = L' ';
        while (*(p+1) == L'\r' || *(p+1) == L'\n' || *(p+1) == L'\t' || *(p+1) == L' ')
          p++;
      }
      else
        tch[cch++] = *p;
    }
    tch[cch++] = L'\0';
    StrTrim(tch,L" ");
  }

  if (cch == 1)
    StringCchCopy(tch,COUNTOF(tch),L" ... ");

  if (cch > cchExcerpt) {
    tch[cchExcerpt-2] = L'.';
    tch[cchExcerpt-3] = L'.';
    tch[cchExcerpt-4] = L'.';
  }
  StringCchCopyN(lpszExcerpt,cchExcerpt,tch,cchExcerpt);

  if (pszText)
    LocalFree(pszText);
  if (pszTextW)
    LocalFree(pszTextW);
}



//=============================================================================
//
//  EditSetSearchFlags()
//
void __fastcall EditSetSearchFlags(HWND hwnd, LPEDITFINDREPLACE lpefr)
{
  UINT uCPEdit = Encoding_SciGetCodePage(g_hwndEdit);

  GetDlgItemTextW2A(uCPEdit, hwnd, IDC_FINDTEXT, lpefr->szFind, COUNTOF(lpefr->szFind));

  if (GetDlgItem(hwnd, IDC_REPLACETEXT)) {
    GetDlgItemTextW2A(uCPEdit, hwnd, IDC_REPLACETEXT, lpefr->szReplace, COUNTOF(lpefr->szReplace));
  }

  lpefr->fuFlags = 0; // clear all

  if (IsDlgButtonChecked(hwnd, IDC_FINDCASE) == BST_CHECKED) {
    lpefr->fuFlags |= SCFIND_MATCHCASE;
  }
  if (IsDlgButtonChecked(hwnd, IDC_FINDWORD) == BST_CHECKED) {
    lpefr->fuFlags |= SCFIND_WHOLEWORD;
  }
  if (IsDlgButtonChecked(hwnd, IDC_FINDSTART) == BST_CHECKED) {
    lpefr->fuFlags |= SCFIND_WORDSTART;
  }
  if (IsDlgButtonChecked(hwnd, IDC_FINDREGEXP) == BST_CHECKED) {
    lpefr->fuFlags |= SCFIND_NP3_REGEX;
    if (IsDlgButtonChecked(hwnd, IDC_DOT_MATCH_ALL) == BST_CHECKED) {
      lpefr->bDotMatchAll = TRUE;
      lpefr->fuFlags |= SCFIND_DOT_MATCH_ALL;
    }
  }
  if (IsDlgButtonChecked(hwnd, IDC_WILDCARDSEARCH) == BST_CHECKED) {
    lpefr->bWildcardSearch = TRUE;
    lpefr->fuFlags |= SCFIND_NP3_REGEX;
    lpefr->fuFlags &= ~(SCFIND_DOT_MATCH_ALL);
  }

  lpefr->bTransformBS = (IsDlgButtonChecked(hwnd, IDC_FINDTRANSFORMBS) == BST_CHECKED) ? TRUE : FALSE;

  lpefr->bMarkOccurences = (IsDlgButtonChecked(hwnd, IDC_ALL_OCCURRENCES) == BST_CHECKED) ? TRUE : FALSE;

  lpefr->bNoFindWrap = (IsDlgButtonChecked(hwnd, IDC_NOWRAP) == BST_CHECKED) ? TRUE : FALSE;
}


// Wildcard search uses the regexp engine to perform a simple search with * ? as wildcards 
// instead of more advanced and user-unfriendly regexp syntax
// for speed, we only need POSIX syntax here
void __fastcall EscapeWildcards(char* szFind2, LPCEDITFINDREPLACE lpefr)
{
  char szWildcardEscaped[FNDRPL_BUFFER] = { '\0' };
  int iSource = 0;
  int iDest = 0;

  lpefr->fuFlags |= SCFIND_NP3_REGEX;

  while (szFind2[iSource] != '\0')
  {
    char c = szFind2[iSource];
    if (c == '*')
    {
      szWildcardEscaped[iDest++] = '.';
    }
    else if (c == '?')
    {
      c = '.';
    }
    else
    {
      if (c == '^' ||
        c == '$' ||
        c == '(' ||
        c == ')' ||
        c == '[' ||
        c == ']' ||
        c == '{' ||
        c == '}' ||
        c == '.' ||
        c == '+' ||
        c == '|' ||
        c == '\\')
      {
        szWildcardEscaped[iDest++] = '\\';
      }
    }
    szWildcardEscaped[iDest++] = c;
    iSource++;
  }

  szWildcardEscaped[iDest] = '\0';

  StringCchCopyNA(szFind2, FNDRPL_BUFFER, szWildcardEscaped, COUNTOF(szWildcardEscaped));
}


//=============================================================================
//
//  EditGetFindStrg()
//
int __fastcall EditGetFindStrg(HWND hwnd, LPCEDITFINDREPLACE lpefr, LPSTR szFind, int cchCnt)
{
  if (!StringCchLenA(lpefr->szFind, COUNTOF(lpefr->szFind))) { return 0; }

  StringCchCopyA(szFind, cchCnt, lpefr->szFind);

  BOOL bIsRegEx = (lpefr->fuFlags & SCFIND_REGEXP);
  if (lpefr->bTransformBS || bIsRegEx) {
    TransformBackslashes(szFind, bIsRegEx, Encoding_SciGetCodePage(hwnd), NULL);
  }
  if (StringCchLenA(szFind, FNDRPL_BUFFER) > 0) {
    if (lpefr->bWildcardSearch)
      EscapeWildcards(szFind, lpefr);
  }
  
  return StringCchLenA(szFind, FNDRPL_BUFFER);
}




//=============================================================================
//
//  EditFindInTarget()
//
DocPos __fastcall EditFindInTarget(HWND hwnd, LPCSTR szFind, DocPos length, int flags, DocPos* start, DocPos* end, BOOL bForceNext)
{
  DocPos _start = *start;
  DocPos _end = *end;
  BOOL bFindPrev = (_start > _end);

  EditEnterTargetTransaction();

  SendMessage(hwnd, SCI_SETSEARCHFLAGS, flags, 0);
  SendMessage(hwnd, SCI_SETTARGETRANGE, _start, _end);
  DocPos iPos = (DocPos)SendMessage(hwnd, SCI_SEARCHINTARGET, length, (LPARAM)szFind);
  //  handle next in case of zero-length-matches (regex) !
  if (iPos == _start) {
    DocPos nend = (DocPos)SendMessage(hwnd, SCI_GETTARGETEND, 0, 0);
    if ((_start == nend) && bForceNext)
    {
      DocPos newStart = (int)(bFindPrev ?
        SendMessage(hwnd, SCI_POSITIONBEFORE, _start, 0) :
        SendMessage(hwnd, SCI_POSITIONAFTER, _start, 0));
      if (newStart != _start) {
        //_start = newStart;
        SendMessage(hwnd, SCI_SETTARGETRANGE, newStart, _end);
        iPos = (DocPos)SendMessage(hwnd, SCI_SEARCHINTARGET, length, (LPARAM)szFind);
      }
      else {
        iPos = (DocPos)-1; // already at document begin or end => not found
      }
    }
  }
  if (iPos >= 0) {
    // found in range, set begin and end of finding
    *start = (DocPos)SendMessage(hwnd, SCI_GETTARGETSTART, 0, 0);
    *end = (DocPos)SendMessage(hwnd, SCI_GETTARGETEND, 0, 0);
  }

  EditLeaveTargetTransaction();

  return iPos;
}


//=============================================================================
//
//  EditCheckRegex()
//
typedef enum { MATCH = 0, NO_MATCH = 1, INVALID = 2 } RegExResult_t;

RegExResult_t __fastcall EditFindHasMatch(HWND hwnd, LPCEDITFINDREPLACE lpefr, BOOL bMarkAll, BOOL bFirstMatchOnly)
{
  char szFind[FNDRPL_BUFFER];
  DocPos slen = EditGetFindStrg(hwnd, lpefr, szFind, COUNTOF(szFind));

  const DocPos iStart = bFirstMatchOnly ? SciCall_GetSelectionStart() : 0;
  const DocPos iTextLength = SciCall_GetTextLength();

  DocPos start = iStart;
  DocPos end   = iTextLength;
  DocPos iPos  = EditFindInTarget(hwnd, szFind, slen, (int)(lpefr->fuFlags), &start, &end, FALSE);

  if (!bFirstMatchOnly) 
  {
    if (bMarkAll && (iPos >= 0)) {
      EditClearAllMarks(hwnd, (DocPos)0, iTextLength);
      EditMarkAll(hwnd, szFind, (int)(lpefr->fuFlags), (DocPos)0, iTextLength, FALSE, FALSE);
    }
  }
  return ((iPos >= 0) ? MATCH : ((iPos == (DocPos)-1) ? NO_MATCH : INVALID));
}



//=============================================================================
//
//  EditFindReplaceDlgProcW()
//
static void __fastcall EditSetTimerMarkAll(HWND hwnd, int delay)
{
  if (delay < USER_TIMER_MINIMUM) {
    TEST_AND_RESET(TIMER_BIT_MARK_OCC);
    KillTimer(hwnd, IDT_TIMER_MRKALL);
    SendMessage(hwnd, WM_COMMAND, MAKELONG(IDC_MARKALL_OCC, 1), 0);
    return;
  }
  TEST_AND_SET(TIMER_BIT_MARK_OCC);
  SetTimer(hwnd, IDT_TIMER_MRKALL, delay, NULL);
}


//=============================================================================
//
//  EditFindReplaceDlgProcW()
//
static char g_lastFind[FNDRPL_BUFFER] = { L'\0' };

INT_PTR CALLBACK EditFindReplaceDlgProcW(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  static LPEDITFINDREPLACE lpefr = NULL;

  static RegExResult_t regexMatch = INVALID;

  static BOOL bFlagsChanged = TRUE;

  static COLORREF rgbRed = RGB(255, 170, 170);
  static COLORREF rgbGreen = RGB(170, 255, 170);
  static COLORREF rgbBlue = RGB(170, 200, 255);
  static HBRUSH hBrushRed;
  static HBRUSH hBrushGreen;
  static HBRUSH hBrushBlue;

  static int  iSaveMarkOcc = -1;
  static BOOL bSaveOccVisible = FALSE;
  static BOOL bSaveTFBackSlashes = FALSE;

  WCHAR tchBuf[FNDRPL_BUFFER] = { L'\0' };

  switch(umsg)
  {
    case WM_INITDIALOG:
    {
      iReplacedOccurrences = 0;

      SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
      lpefr = (LPEDITFINDREPLACE)lParam;

      if (lpefr->bMarkOccurences) {
        iSaveMarkOcc = iMarkOccurrences;
        EnableCmd(GetMenu(g_hwndMain), IDM_VIEW_MARKOCCUR_ONOFF, FALSE);
        iMarkOccurrences = 0;
        bSaveOccVisible = bMarkOccurrencesMatchVisible;
        EnableCmd(GetMenu(g_hwndMain), IDM_VIEW_MARKOCCUR_VISIBLE, FALSE);
        bMarkOccurrencesMatchVisible = FALSE;
        CheckDlgButton(hwnd, IDC_ALL_OCCURRENCES, BST_CHECKED);
      }
      else {
        iSaveMarkOcc = -1;
        bSaveOccVisible = bMarkOccurrencesMatchVisible;
        CheckDlgButton(hwnd, IDC_ALL_OCCURRENCES, BST_UNCHECKED);
        EditClearAllMarks(g_hwndEdit, 0, -1);
      }

      //const WORD wTabSpacing = (WORD)SendMessage(lpefr->hwnd, SCI_GETTABWIDTH, 0, 0);;  // dialog box units
      //SendDlgItemMessage(hwnd, IDC_FINDTEXT, EM_SETTABSTOPS, 1, (LPARAM)&wTabSpacing);

      // Load MRUs
      for (int i = 0; i < MRU_Enum(mruFind, 0, NULL, 0); i++) {
        MRU_Enum(mruFind, i, tchBuf, COUNTOF(tchBuf));
        SendDlgItemMessage(hwnd, IDC_FINDTEXT, CB_ADDSTRING, 0, (LPARAM)tchBuf);
      }
      for (int i = 0; i < MRU_Enum(mruReplace, 0, NULL, 0); i++) {
        MRU_Enum(mruReplace, i, tchBuf, COUNTOF(tchBuf));
        SendDlgItemMessage(hwnd, IDC_REPLACETEXT, CB_ADDSTRING, 0, (LPARAM)tchBuf);
      }

      SendDlgItemMessage(hwnd, IDC_FINDTEXT, CB_LIMITTEXT, FNDRPL_BUFFER, 0);
      SendDlgItemMessage(hwnd, IDC_FINDTEXT, CB_SETEXTENDEDUI, TRUE, 0);

      if (!GetWindowTextLengthW(GetDlgItem(hwnd, IDC_FINDTEXT)))
        SetDlgItemTextA2W(CP_UTF8, hwnd, IDC_FINDTEXT, lpefr->szFindUTF8);

      if (GetDlgItem(hwnd, IDC_REPLACETEXT))
      {
        SendDlgItemMessage(hwnd, IDC_REPLACETEXT, CB_LIMITTEXT, FNDRPL_BUFFER, 0);
        SendDlgItemMessage(hwnd, IDC_REPLACETEXT, CB_SETEXTENDEDUI, TRUE, 0);
        SetDlgItemTextA2W(CP_UTF8, hwnd, IDC_REPLACETEXT, lpefr->szReplaceUTF8);
      }

      if (lpefr->fuFlags & SCFIND_MATCHCASE)
        CheckDlgButton(hwnd, IDC_FINDCASE, BST_CHECKED);

      if (lpefr->fuFlags & SCFIND_WHOLEWORD)
        CheckDlgButton(hwnd, IDC_FINDWORD, BST_CHECKED);

      if (lpefr->fuFlags & SCFIND_WORDSTART)
        CheckDlgButton(hwnd, IDC_FINDSTART, BST_CHECKED);

      if (lpefr->bTransformBS) {
        bSaveTFBackSlashes = lpefr->bTransformBS;
        CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, BST_CHECKED);
      }
      else
        bSaveTFBackSlashes = FALSE;

      if (lpefr->fuFlags & SCFIND_REGEXP) {
        CheckDlgButton(hwnd, IDC_FINDREGEXP, BST_CHECKED);
        CheckDlgButton(hwnd, IDC_WILDCARDSEARCH, BST_UNCHECKED);
        DialogEnableWindow(hwnd, IDC_DOT_MATCH_ALL, TRUE);
      }

      if (lpefr->bDotMatchAll) {
        CheckDlgButton(hwnd, IDC_DOT_MATCH_ALL, BST_CHECKED);
      }

      if (lpefr->bWildcardSearch) {
        CheckDlgButton(hwnd, IDC_FINDREGEXP, BST_UNCHECKED);
        CheckDlgButton(hwnd, IDC_WILDCARDSEARCH, BST_CHECKED);
        DialogEnableWindow(hwnd, IDC_DOT_MATCH_ALL, FALSE);
      }

      if (lpefr->bMarkOccurences) {
        CheckDlgButton(hwnd, IDC_ALL_OCCURRENCES, BST_CHECKED);
      }

      if (lpefr->fuFlags & SCFIND_REGEXP) {
        CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, BST_CHECKED);
        DialogEnableWindow(hwnd, IDC_FINDTRANSFORMBS, FALSE);
      }
      else {
        DialogEnableWindow(hwnd, IDC_DOT_MATCH_ALL, FALSE);
      }

      if (lpefr->bNoFindWrap) {
        CheckDlgButton(hwnd, IDC_NOWRAP, BST_CHECKED);
      }

      if (GetDlgItem(hwnd, IDC_REPLACE)) {
        if (bSwitchedFindReplace) {
          if (lpefr->bFindClose)
            CheckDlgButton(hwnd, IDC_FINDCLOSE, BST_CHECKED);
        }
        else {
          if (lpefr->bReplaceClose)
            CheckDlgButton(hwnd, IDC_FINDCLOSE, BST_CHECKED);
        }
      }
      else {
        if (bSwitchedFindReplace) {
          if (lpefr->bReplaceClose)
            CheckDlgButton(hwnd, IDC_FINDCLOSE, BST_CHECKED);
        }
        else {
          if (lpefr->bFindClose)
            CheckDlgButton(hwnd, IDC_FINDCLOSE, BST_CHECKED);
        }
      }

      if (!bSwitchedFindReplace) {
        if (xFindReplaceDlg == 0 || yFindReplaceDlg == 0)
          CenterDlgInParent(hwnd);
        else
          SetDlgPos(hwnd, xFindReplaceDlg, yFindReplaceDlg);
      }
      else {
        SetDlgPos(hwnd, xFindReplaceDlgSave, yFindReplaceDlgSave);
        bSwitchedFindReplace = FALSE;
        CopyMemory(lpefr, &efrSave, sizeof(EDITFINDREPLACE));
      }

      HMENU hmenu = GetSystemMenu(hwnd, FALSE);
      GetString(IDS_SAVEPOS, tchBuf, COUNTOF(tchBuf));
      InsertMenu(hmenu, 0, MF_BYPOSITION | MF_STRING | MF_ENABLED, IDS_SAVEPOS, tchBuf);
      GetString(IDS_RESETPOS, tchBuf, COUNTOF(tchBuf));
      InsertMenu(hmenu, 1, MF_BYPOSITION | MF_STRING | MF_ENABLED, IDS_RESETPOS, tchBuf);
      InsertMenu(hmenu, 2, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);

      hBrushRed = CreateSolidBrush(rgbRed);
      hBrushGreen = CreateSolidBrush(rgbGreen);
      hBrushBlue = CreateSolidBrush(rgbBlue);

      EditSetSearchFlags(hwnd, lpefr);
      bFlagsChanged = TRUE;
      EditSetTimerMarkAll(hwnd, 50);
    }
    return TRUE;


    case WM_DESTROY:
      {
        DeleteObject(hBrushRed);
        DeleteObject(hBrushGreen);
        DeleteObject(hBrushBlue);
        KillTimer(hwnd, IDT_TIMER_MRKALL);

        if (iSaveMarkOcc >= 0) {
          EnableCmd(GetMenu(g_hwndMain), IDM_VIEW_MARKOCCUR_ONOFF, TRUE);
          if (iSaveMarkOcc != 0) {
            SendMessage(g_hwndMain, WM_COMMAND, (WPARAM)MAKELONG(IDM_VIEW_MARKOCCUR_ONOFF, 1), 0);
          }
        }
        bMarkOccurrencesMatchVisible = bSaveOccVisible;
        EnableCmd(GetMenu(g_hwndMain), IDM_VIEW_MARKOCCUR_VISIBLE, bMarkOccurrencesMatchVisible);

        iReplacedOccurrences = 0;
      }
      return FALSE;


    case WM_TIMER:
      {
        if (LOWORD(wParam) == IDT_TIMER_MRKALL)
        {
          if (TEST_AND_RESET(TIMER_BIT_MARK_OCC)) {
            PostMessage(hwnd, WM_COMMAND, MAKELONG(IDC_MARKALL_OCC, 1), 0);
            KillTimer(hwnd, IDT_TIMER_MRKALL);
          }
          return TRUE;
        }
      }
      return FALSE;


    case WM_ACTIVATE:
      {
        DialogEnableWindow(hwnd, IDC_REPLACEINSEL, !SciCall_IsSelectionEmpty());
      
        lpefr = (LPEDITFINDREPLACE)GetWindowLongPtr(hwnd, DWLP_USER);
        if (lpefr->bMarkOccurences) {
          bFlagsChanged = TRUE;
          EditSetTimerMarkAll(hwnd,50);
        }
        //if (LOWORD(wParam) == WA_INACTIVE) {
        //  bFindReplCopySelOrClip = TRUE;
        //}
      }
      return FALSE;


    case WM_COMMAND:
    {
      lpefr = (LPEDITFINDREPLACE)GetWindowLongPtr(hwnd, DWLP_USER);

      switch (LOWORD(wParam))
      {
      case IDC_FINDTEXT:
      case IDC_REPLACETEXT:
      {
        if (bFindReplCopySelOrClip)
        {
          char *lpszSelection = NULL;
          tchBuf[0] = L'\0';

          DocPos cchSelection = (DocPos)SendMessage(lpefr->hwnd, SCI_GETSELTEXT, 0, (LPARAM)NULL);
          if ((1 < cchSelection) && (cchSelection < FNDRPL_BUFFER)) {
            lpszSelection = GlobalAlloc(GPTR, cchSelection);
            SendMessage(lpefr->hwnd, SCI_GETSELTEXT, 0, (LPARAM)lpszSelection);
          }
          else if (cchSelection <= 1) {
            // nothing is selected in the editor:
            // if first time you bring up find/replace dialog, 
            // copy content from File-MRU or clipboard to find box
            GetFindPattern(tchBuf, FNDRPL_BUFFER);
            if (tchBuf[0] == L'\0') {
              // empty string in File-MRU list
              char* pClip = EditGetClipboardText(hwnd, FALSE, NULL, NULL);
              if (pClip) {
                int len = lstrlenA(pClip);
                if (len > 0 && len < FNDRPL_BUFFER) {
                  lpszSelection = GlobalAlloc(GPTR, len + 1);
                  StringCchCopyNA(lpszSelection, len + 1, pClip, len);
                }
                LocalFree(pClip);
              }
            }
          }

          if (lpszSelection) {
            // Check lpszSelection and truncate bad chars (CR,LF,VT)
            char* lpsz = StrChrA(lpszSelection, 13);
            if (lpsz) *lpsz = '\0';

            lpsz = StrChrA(lpszSelection, 10);
            if (lpsz) *lpsz = '\0';

            lpsz = StrChrA(lpszSelection, 11);
            if (lpsz) *lpsz = '\0';

            SetDlgItemTextA2W(Encoding_SciGetCodePage(g_hwndEdit), hwnd, IDC_FINDTEXT, lpszSelection);
            GlobalFree(lpszSelection);
          }
          else {
            if (tchBuf[0] == L'\0') {
              MRU_Enum(mruFind, 0, tchBuf, COUNTOF(tchBuf));
            }
            SetDlgItemText(hwnd, IDC_FINDTEXT, tchBuf);
          }
          bFindReplCopySelOrClip = FALSE;
        }

        BOOL bEnableF = (GetWindowTextLengthW(GetDlgItem(hwnd, IDC_FINDTEXT)) ||
          CB_ERR != SendDlgItemMessage(hwnd, IDC_FINDTEXT, CB_GETCURSEL, 0, 0));

        BOOL bEnableR = (GetWindowTextLengthW(GetDlgItem(hwnd, IDC_REPLACETEXT)) ||
          CB_ERR != SendDlgItemMessage(hwnd, IDC_REPLACETEXT, CB_GETCURSEL, 0, 0));

        BOOL bEnableIS = !(BOOL)SendMessage(g_hwndEdit, SCI_GETSELECTIONEMPTY, 0, 0);

        DialogEnableWindow(hwnd, IDOK, bEnableF);
        DialogEnableWindow(hwnd, IDC_FINDPREV, bEnableF);
        DialogEnableWindow(hwnd, IDC_REPLACE, bEnableF);
        DialogEnableWindow(hwnd, IDC_REPLACEALL, bEnableF);
        DialogEnableWindow(hwnd, IDC_REPLACEINSEL, bEnableF && bEnableIS);
        DialogEnableWindow(hwnd, IDC_SWAPSTRG, bEnableF || bEnableR);

        if (HIWORD(wParam) == CBN_CLOSEUP) {
          LONG lSelEnd;
          SendDlgItemMessage(hwnd, LOWORD(wParam), CB_GETEDITSEL, 0, (LPARAM)&lSelEnd);
          SendDlgItemMessage(hwnd, LOWORD(wParam), CB_SETEDITSEL, 0, MAKELPARAM(lSelEnd, lSelEnd));
        }
        bFlagsChanged = TRUE;
        EditSetTimerMarkAll(hwnd,50);
      }
      break;


      case IDC_ALL_OCCURRENCES:
        {
          if (IsDlgButtonChecked(hwnd, IDC_ALL_OCCURRENCES) == BST_CHECKED) 
          {
            lpefr->bMarkOccurences = TRUE;
            iSaveMarkOcc = iMarkOccurrences;
            EnableCmd(GetMenu(g_hwndMain), IDM_VIEW_MARKOCCUR_ONOFF, FALSE);
            iMarkOccurrences = 0;
            bSaveOccVisible = bMarkOccurrencesMatchVisible;
            EnableCmd(GetMenu(g_hwndMain), IDM_VIEW_MARKOCCUR_VISIBLE, FALSE);
            bMarkOccurrencesMatchVisible = FALSE;
          }
          else {                         // switched OFF
            lpefr->bMarkOccurences = FALSE;
            if (iSaveMarkOcc >= 0) {
              EnableCmd(GetMenu(g_hwndMain), IDM_VIEW_MARKOCCUR_ONOFF, TRUE);
              if (iSaveMarkOcc != 0) {
                SendMessage(g_hwndMain, WM_COMMAND, (WPARAM)MAKELONG(IDM_VIEW_MARKOCCUR_ONOFF, 1), 0);
              }
            }
            iSaveMarkOcc = -1;
            bMarkOccurrencesMatchVisible = bSaveOccVisible;
            EnableCmd(GetMenu(g_hwndMain), IDM_VIEW_MARKOCCUR_VISIBLE, bMarkOccurrencesMatchVisible);
            bSaveOccVisible = FALSE;
            EditClearAllMarks(g_hwndEdit, 0, -1);
            InvalidateRect(GetDlgItem(hwnd, IDC_FINDTEXT), NULL, TRUE);
          }
          bFlagsChanged = TRUE;
          EditSetTimerMarkAll(hwnd,0);
        }
        break;

      // called on timer trigger
      case IDC_MARKALL_OCC:
        {
          iMarkOccurrencesCount = 0;
          EditSetSearchFlags(hwnd, lpefr);
          if (lpefr->bMarkOccurences) {
            if (bFlagsChanged || (StringCchCompareXA(g_lastFind, lpefr->szFind) != 0)) {
              StringCchCopyA(g_lastFind, COUNTOF(g_lastFind), lpefr->szFind);
              RegExResult_t match = EditFindHasMatch(g_hwndEdit, lpefr, (iSaveMarkOcc > 0), FALSE);
              if (regexMatch != match) {
                regexMatch = match;
              }
              // we have to set Sci's regex instance to first find (have substitution in place)
              EditFindHasMatch(g_hwndEdit, lpefr, FALSE, TRUE);
              bFlagsChanged = FALSE;
              InvalidateRect(GetDlgItem(hwnd, IDC_FINDTEXT), NULL, TRUE);
            }
            UpdateToolbar();
            UpdateStatusbar();
          }
        }
        break;


      case IDC_FINDREGEXP:
        if (IsDlgButtonChecked(hwnd, IDC_FINDREGEXP) == BST_CHECKED)
        {
          lpefr->fuFlags |= SCFIND_NP3_REGEX;

          DialogEnableWindow(hwnd, IDC_DOT_MATCH_ALL, TRUE);

          if (lpefr->bDotMatchAll) {
            lpefr->fuFlags |= SCFIND_DOT_MATCH_ALL;
          }
          else {
            lpefr->fuFlags &= ~(SCFIND_DOT_MATCH_ALL);
          }

          CheckDlgButton(hwnd, IDC_WILDCARDSEARCH, BST_UNCHECKED); // Can not use wildcard search together with regexp
          lpefr->bWildcardSearch = FALSE;

          CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, BST_CHECKED); // transform BS handled by regex
          DialogEnableWindow(hwnd, IDC_FINDTRANSFORMBS, FALSE);
        }
        else { // unchecked

          lpefr->fuFlags &= ~(SCFIND_NP3_REGEX);
          lpefr->fuFlags &= ~(SCFIND_DOT_MATCH_ALL);

          DialogEnableWindow(hwnd, IDC_DOT_MATCH_ALL, FALSE);

          DialogEnableWindow(hwnd, IDC_FINDTRANSFORMBS, TRUE);
          lpefr->bTransformBS = bSaveTFBackSlashes;
          CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, (lpefr->bTransformBS) ? BST_CHECKED : BST_UNCHECKED);
        }
        bFlagsChanged = TRUE;
        EditSetTimerMarkAll(hwnd,0);
        break;

      case IDC_DOT_MATCH_ALL:
        if (IsDlgButtonChecked(hwnd, IDC_DOT_MATCH_ALL) == BST_CHECKED) {
          lpefr->bDotMatchAll = TRUE;
          lpefr->fuFlags |= SCFIND_DOT_MATCH_ALL;
        }
        else {
          lpefr->bDotMatchAll = FALSE;
          lpefr->fuFlags &= ~(SCFIND_DOT_MATCH_ALL);
        }
        bFlagsChanged = TRUE;
        EditSetTimerMarkAll(hwnd,0);
        break;

      case IDC_WILDCARDSEARCH:
        if (IsDlgButtonChecked(hwnd, IDC_WILDCARDSEARCH) == BST_CHECKED)
        {
          lpefr->bWildcardSearch = TRUE;
          lpefr->fuFlags |= SCFIND_NP3_REGEX;
          lpefr->fuFlags &= ~(SCFIND_DOT_MATCH_ALL);

          CheckDlgButton(hwnd, IDC_FINDREGEXP, BST_UNCHECKED);
          DialogEnableWindow(hwnd, IDC_DOT_MATCH_ALL, FALSE);

          CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, BST_CHECKED);  // transform BS handled by regex
          DialogEnableWindow(hwnd, IDC_FINDTRANSFORMBS, FALSE);
        }
        else { // unchecked

          lpefr->bWildcardSearch = FALSE;
          lpefr->fuFlags &= ~(SCFIND_NP3_REGEX);
          lpefr->fuFlags &= ~(SCFIND_DOT_MATCH_ALL);

          DialogEnableWindow(hwnd, IDC_FINDTRANSFORMBS, TRUE);
          lpefr->bTransformBS = bSaveTFBackSlashes;
          CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, (lpefr->bTransformBS) ? BST_CHECKED : BST_UNCHECKED);
        }
        bFlagsChanged = TRUE;
        EditSetTimerMarkAll(hwnd,0);
        break;

      case IDC_FINDTRANSFORMBS:
        if (IsDlgButtonChecked(hwnd, IDC_FINDTRANSFORMBS) == BST_CHECKED) {
          lpefr->bTransformBS = TRUE;
          bSaveTFBackSlashes = TRUE;
        }
        else {
          lpefr->bTransformBS = FALSE;
          bSaveTFBackSlashes = FALSE;
        }
        bFlagsChanged = TRUE;
        EditSetTimerMarkAll(hwnd,0);
        break;

      case IDC_FINDCASE:
        bFlagsChanged = TRUE;
        EditSetTimerMarkAll(hwnd,0);
        break;

      case IDC_FINDWORD:
        bFlagsChanged = TRUE;
        EditSetTimerMarkAll(hwnd,0);
        break;

      case IDC_FINDSTART:
        bFlagsChanged = TRUE;
        EditSetTimerMarkAll(hwnd,0);
        break;


      case IDC_REPLACE:
      case IDC_REPLACEALL:
      case IDC_REPLACEINSEL:
        iReplacedOccurrences = 0;
      case IDOK:
      case IDC_FINDPREV:
      case IDACC_SELTONEXT:
      case IDACC_SELTOPREV:
      case IDMSG_SWITCHTOFIND:
      case IDMSG_SWITCHTOREPLACE:
      {
        BOOL bIsFindDlg = (GetDlgItem(g_hwndDlgFindReplace, IDC_REPLACE) == NULL);

        if ((bIsFindDlg && LOWORD(wParam) == IDMSG_SWITCHTOREPLACE ||
          !bIsFindDlg && LOWORD(wParam) == IDMSG_SWITCHTOFIND)) {
          GetDlgPos(hwnd, &xFindReplaceDlgSave, &yFindReplaceDlgSave);
          bSwitchedFindReplace = TRUE;
          CopyMemory(&efrSave, lpefr, sizeof(EDITFINDREPLACE));
        }

        // Get current code page for Unicode conversion
        UINT uCPEdit = Encoding_SciGetCodePage(hwnd);
        cpLastFind = uCPEdit;

        if (!bSwitchedFindReplace &&
          !GetDlgItemTextW2A(uCPEdit, hwnd, IDC_FINDTEXT, lpefr->szFind, COUNTOF(lpefr->szFind))) {
          DialogEnableWindow(hwnd, IDOK, FALSE);
          DialogEnableWindow(hwnd, IDC_FINDPREV, FALSE);
          DialogEnableWindow(hwnd, IDC_REPLACE, FALSE);
          DialogEnableWindow(hwnd, IDC_REPLACEALL, FALSE);
          DialogEnableWindow(hwnd, IDC_REPLACEINSEL, FALSE);
          if (!GetDlgItemTextW2A(uCPEdit, hwnd, IDC_REPLACETEXT, lpefr->szReplace, COUNTOF(lpefr->szReplace)))
            DialogEnableWindow(hwnd, IDC_SWAPSTRG, FALSE);
          return TRUE;
        }

        EditSetSearchFlags(hwnd, lpefr);

        if (bIsFindDlg) {
          lpefr->bFindClose = (IsDlgButtonChecked(hwnd, IDC_FINDCLOSE) == BST_CHECKED) ? TRUE : FALSE;
        }
        else {
          lpefr->bReplaceClose = (IsDlgButtonChecked(hwnd, IDC_FINDCLOSE) == BST_CHECKED) ? TRUE : FALSE;
        }

        WCHAR tchBuf2[FNDRPL_BUFFER] = { L'\0' };

        if (!bSwitchedFindReplace) {
          // Save MRUs
          if (StringCchLenA(lpefr->szFind, COUNTOF(lpefr->szFind))) {
            if (GetDlgItemTextW2A(CP_UTF8, hwnd, IDC_FINDTEXT, lpefr->szFindUTF8, COUNTOF(lpefr->szFindUTF8))) {
              GetDlgItemText(hwnd, IDC_FINDTEXT, tchBuf2, COUNTOF(tchBuf2));
              MRU_Add(mruFind, tchBuf2, 0, 0, NULL, NULL);
              SetFindPattern(tchBuf2);
            }
          }
          if (StringCchLenA(lpefr->szReplace, COUNTOF(lpefr->szReplace))) {
            if (GetDlgItemTextW2A(CP_UTF8, hwnd, IDC_REPLACETEXT, lpefr->szReplaceUTF8, COUNTOF(lpefr->szReplaceUTF8))) {
              GetDlgItemText(hwnd, IDC_REPLACETEXT, tchBuf2, COUNTOF(tchBuf2));
              MRU_Add(mruReplace, tchBuf2, 0, 0, NULL, NULL);
            }
          }
          else
            StringCchCopyA(lpefr->szReplaceUTF8, COUNTOF(lpefr->szReplaceUTF8), "");
        }
        else {
          GetDlgItemTextW2A(CP_UTF8, hwnd, IDC_FINDTEXT, lpefr->szFindUTF8, COUNTOF(lpefr->szFindUTF8));
          if (!GetDlgItemTextW2A(CP_UTF8, hwnd, IDC_REPLACETEXT, lpefr->szReplaceUTF8, COUNTOF(lpefr->szReplaceUTF8)))
            StringCchCopyA(lpefr->szReplaceUTF8, COUNTOF(lpefr->szReplaceUTF8), "");
        }

        // Reload MRUs
        SendDlgItemMessage(hwnd, IDC_FINDTEXT, CB_RESETCONTENT, 0, 0);
        SendDlgItemMessage(hwnd, IDC_REPLACETEXT, CB_RESETCONTENT, 0, 0);

        for (int i = 0; i < MRU_Enum(mruFind, 0, NULL, 0); i++) {
          MRU_Enum(mruFind, i, tchBuf2, COUNTOF(tchBuf2));
          SendDlgItemMessage(hwnd, IDC_FINDTEXT, CB_ADDSTRING, 0, (LPARAM)tchBuf2);
        }
        for (int i = 0; i < MRU_Enum(mruReplace, 0, NULL, 0); i++) {
          MRU_Enum(mruReplace, i, tchBuf2, COUNTOF(tchBuf2));
          SendDlgItemMessage(hwnd, IDC_REPLACETEXT, CB_ADDSTRING, 0, (LPARAM)tchBuf2);
        }

        SetDlgItemTextA2W(CP_UTF8, hwnd, IDC_FINDTEXT, lpefr->szFindUTF8);
        SetDlgItemTextA2W(CP_UTF8, hwnd, IDC_REPLACETEXT, lpefr->szReplaceUTF8);

        if (!bSwitchedFindReplace)
          SendMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetFocus()), 1);

        BOOL bCloseDlg = FALSE;
        if (bIsFindDlg) {
          bCloseDlg = lpefr->bFindClose;
        }
        else if (LOWORD(wParam) != IDOK) {
          bCloseDlg = lpefr->bReplaceClose;
        }

        if (bCloseDlg) {
          //EndDialog(hwnd,LOWORD(wParam));
          DestroyWindow(hwnd);
        }

        switch (LOWORD(wParam)) {
        case IDOK: // find next
        case IDACC_SELTONEXT:
          if (!bIsFindDlg)
            bReplaceInitialized = TRUE;
          EditFindNext(lpefr->hwnd, lpefr, (LOWORD(wParam) == IDACC_SELTONEXT), HIBYTE(GetKeyState(VK_F3)));
          break;

        case IDC_FINDPREV: // find previous
        case IDACC_SELTOPREV:
          if (!bIsFindDlg)
            bReplaceInitialized = TRUE;
          EditFindPrev(lpefr->hwnd, lpefr, (LOWORD(wParam) == IDACC_SELTOPREV), HIBYTE(GetKeyState(VK_F3)));
          break;

        case IDC_REPLACE:
          {
            bReplaceInitialized = TRUE;
            int token = BeginUndoAction();
            EditReplace(lpefr->hwnd, lpefr);
            EndUndoAction(token);
          }
          break;

        case IDC_REPLACEALL:
          bReplaceInitialized = TRUE;
          EditReplaceAll(lpefr->hwnd, lpefr, TRUE);
          break;

        case IDC_REPLACEINSEL:
          if (!SciCall_IsSelectionEmpty()) {
            bReplaceInitialized = TRUE;
            EditReplaceAllInSelection(lpefr->hwnd, lpefr, TRUE);
          }
          break;
        }
      }
      bFlagsChanged = TRUE;
      EditSetTimerMarkAll(hwnd,50);
      break;


      case IDCANCEL:
        //EndDialog(hwnd,IDCANCEL);
        DestroyWindow(hwnd);
        break;

      case IDC_SWAPSTRG:
      {
        WCHAR wszFind[FNDRPL_BUFFER] = { L'\0' };
        WCHAR wszRepl[FNDRPL_BUFFER] = { L'\0' };
        GetDlgItemTextW(hwnd, IDC_FINDTEXT, wszFind, COUNTOF(wszFind));
        GetDlgItemTextW(hwnd, IDC_REPLACETEXT, wszRepl, COUNTOF(wszRepl));
        SetDlgItemTextW(hwnd, IDC_FINDTEXT, wszRepl);
        SetDlgItemTextW(hwnd, IDC_REPLACETEXT, wszFind);
        bFlagsChanged = TRUE;
        EditSetTimerMarkAll(hwnd,50);
      }
      break;

      case IDACC_FIND:
        PostMessage(GetParent(hwnd), WM_COMMAND, MAKELONG(IDM_EDIT_FIND, 1), 0);
        break;

      case IDACC_REPLACE:
        PostMessage(GetParent(hwnd), WM_COMMAND, MAKELONG(IDM_EDIT_REPLACE, 1), 0);
        break;

      case IDACC_SAVEPOS:
        GetDlgPos(hwnd, &xFindReplaceDlg, &yFindReplaceDlg);
        break;

      case IDACC_RESETPOS:
        CenterDlgInParent(hwnd);
        xFindReplaceDlg = yFindReplaceDlg = 0;
        break;

      case IDACC_FINDNEXT:
        //SetFocus(g_hwndMain);
        //SetForegroundWindow(g_hwndMain);
        PostMessage(hwnd, WM_COMMAND, MAKELONG(IDOK, 1), 0);
        break;

      case IDACC_FINDPREV:
        //SetFocus(g_hwndMain);
        //SetForegroundWindow(g_hwndMain);
        PostMessage(hwnd, WM_COMMAND, MAKELONG(IDC_FINDPREV, 1), 0);
        break;

      case IDACC_REPLACENEXT:
        if (GetDlgItem(hwnd, IDC_REPLACE) != NULL)
          PostMessage(hwnd, WM_COMMAND, MAKELONG(IDC_REPLACE, 1), 0);
        break;

      case IDACC_SAVEFIND:
        SendMessage(g_hwndMain, WM_COMMAND, MAKELONG(IDM_EDIT_SAVEFIND, 1), 0);
        SetDlgItemTextA2W(CP_UTF8, hwnd, IDC_FINDTEXT, lpefr->szFindUTF8);
        CheckDlgButton(hwnd, IDC_FINDREGEXP, BST_UNCHECKED);
        CheckDlgButton(hwnd, IDC_DOT_MATCH_ALL, BST_UNCHECKED);
        CheckDlgButton(hwnd, IDC_WILDCARDSEARCH, BST_UNCHECKED);
        CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, BST_UNCHECKED);
        PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_FINDTEXT)), 1);
        break;

      case IDACC_VIEWSCHEMECONFIG:
        PostMessage(GetParent(hwnd), WM_COMMAND, MAKELONG(IDM_VIEW_SCHEMECONFIG, 1), 0);
        break;

      default:
        //return FALSE; ???
        break;
      }

    } // WM_COMMAND:
    return TRUE;


    case WM_SYSCOMMAND:
      if (wParam == IDS_SAVEPOS) {
        PostMessage(hwnd, WM_COMMAND, MAKELONG(IDACC_SAVEPOS, 0), 0);
        return TRUE;
      }
      else if (wParam == IDS_RESETPOS) {
        PostMessage(hwnd, WM_COMMAND, MAKELONG(IDACC_RESETPOS, 0), 0);
        return TRUE;
      }
      else
        return FALSE;


    case WM_NOTIFY:
      {
        LPNMHDR pnmhdr = (LPNMHDR)lParam;
        switch (pnmhdr->code) 
        {
        case NM_CLICK:
        case NM_RETURN:
          if (pnmhdr->idFrom == IDC_TOGGLEFINDREPLACE) {
            if (GetDlgItem(hwnd, IDC_REPLACE))
              PostMessage(GetParent(hwnd), WM_COMMAND, MAKELONG(IDM_EDIT_FIND, 1), 0);
            else
              PostMessage(GetParent(hwnd), WM_COMMAND, MAKELONG(IDM_EDIT_REPLACE, 1), 0);
          }
          // Display help messages in the find/replace windows
          else if (pnmhdr->idFrom == IDC_BACKSLASHHELP) {
            MsgBox(MBINFO, IDS_BACKSLASHHELP);
          }
          else if (pnmhdr->idFrom == IDC_REGEXPHELP) {
            MsgBox(MBINFO, IDS_REGEXPHELP);
          }
          else if (pnmhdr->idFrom == IDC_WILDCARDHELP) {
            MsgBox(MBINFO, IDS_WILDCARDHELP);
          }
          break;

        default:
          return FALSE;
        }
      }
      break;

    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
      {
        lpefr = (LPEDITFINDREPLACE)GetWindowLongPtr(hwnd, DWLP_USER);
        if (lpefr->bMarkOccurences)
        {
          HWND hCheck = (HWND)lParam;
          HDC hDC = (HDC)wParam;

          HWND hComboBox = GetDlgItem(hwnd, IDC_FINDTEXT);
          COMBOBOXINFO ci = { sizeof(COMBOBOXINFO) };
          GetComboBoxInfo(hComboBox, &ci);

          //if (hCheck == ci.hwndItem || hCheck == ci.hwndList)
          if (hCheck == ci.hwndItem) {
            SetBkMode(hDC, TRANSPARENT);
            INT_PTR hBrush;
            switch (regexMatch) {
            case MATCH:
              //SetTextColor(hDC, green);
              SetBkColor(hDC, rgbGreen);
              hBrush = (INT_PTR)hBrushGreen;
              break;
            case NO_MATCH:
              //SetTextColor(hDC, blue);
              SetBkColor(hDC, rgbBlue);
              hBrush = (INT_PTR)hBrushBlue;
              break;
            case INVALID:
            default:
              //SetTextColor(hDC, red);
              SetBkColor(hDC, rgbRed);
              hBrush = (INT_PTR)hBrushRed;
              break;
            }
            return hBrush;
          }
        }
      }
      return DefWindowProc(hwnd, umsg, wParam, lParam);

    default:
      break;

  } // switch(umsg)

  return FALSE;
}


//=============================================================================
//
//  EditFindReplaceDlg()
//
HWND EditFindReplaceDlg(HWND hwnd,LPCEDITFINDREPLACE lpefr,BOOL bReplace)
{
  lpefr->hwnd = hwnd;
  HWND hDlg = CreateThemedDialogParam(g_hInstance,
            (bReplace) ? MAKEINTRESOURCEW(IDD_REPLACE) : MAKEINTRESOURCEW(IDD_FIND),
            GetParent(hwnd),
            EditFindReplaceDlgProcW,
            (LPARAM) lpefr);

  ShowWindow(hDlg,SW_SHOW);

  return hDlg;
}


//=============================================================================
//
//  EditFindNext()
//
BOOL EditFindNext(HWND hwnd, LPCEDITFINDREPLACE lpefr, BOOL bExtendSelection, BOOL bFocusWnd) {

  char szFind[FNDRPL_BUFFER];
  BOOL bSuppressNotFound = FALSE;

  DocPos slen = EditGetFindStrg(hwnd, lpefr, szFind, COUNTOF(szFind));
  if (slen <= 0)
    return FALSE;

  if (bFocusWnd)
    SetFocus(hwnd);

  DocPos iTextLength = SciCall_GetTextLength();

  DocPos start = SciCall_GetCurrentPos();
  DocPos end = iTextLength;

  if (start >= end) {
    if (IDOK == InfoBox(MBOKCANCEL, L"MsgFindWrap1", IDS_FIND_WRAPFW)) {
      end = min(start, iTextLength);  start = 0;
    }
    else
      bSuppressNotFound = TRUE;
  }

  DocPos iPos = EditFindInTarget(hwnd, szFind, slen, (int)(lpefr->fuFlags), &start, &end, TRUE);

  if ((iPos < -1) && (lpefr->fuFlags & SCFIND_REGEXP)) {
    InfoBox(MBWARN, L"MsgInvalidRegex", IDS_REGEX_INVALID);
    bSuppressNotFound = TRUE;
  }
  else if ((iPos < 0) && (start > 0) && !lpefr->bNoFindWrap && !bExtendSelection && !bSuppressNotFound) 
  {
    if (IDOK == InfoBox(MBOKCANCEL, L"MsgFindWrap2", IDS_FIND_WRAPFW)) 
    {
      end = min(start, iTextLength);  start = 0;

      iPos = EditFindInTarget(hwnd, szFind, slen, (int)(lpefr->fuFlags), &start, &end, FALSE);

      if ((iPos < -1) && (lpefr->fuFlags & SCFIND_REGEXP)) {
        InfoBox(MBWARN, L"MsgInvalidRegex2", IDS_REGEX_INVALID);
        bSuppressNotFound = TRUE;
      }
    }
    else
      bSuppressNotFound = TRUE;
  }

  if (iPos < 0) {
    if (!bSuppressNotFound)
      InfoBox(0, L"MsgNotFound", IDS_NOTFOUND);
    return FALSE;
  }

  if (bExtendSelection) {
    DocPos iSelPos = SciCall_GetCurrentPos();
    DocPos iSelAnchor = SciCall_GetAnchor();
    EditSelectEx(hwnd, min(iSelAnchor, iSelPos), end);
  }
  else {
    EditSelectEx(hwnd, start, end);
  }
  return TRUE;
}


//=============================================================================
//
//  EditFindPrev()
//
BOOL EditFindPrev(HWND hwnd, LPCEDITFINDREPLACE lpefr, BOOL bExtendSelection, BOOL bFocusWnd) {

  char szFind[FNDRPL_BUFFER];
  BOOL bSuppressNotFound = FALSE;

  if (bFocusWnd)
    SetFocus(hwnd);

  DocPos slen = EditGetFindStrg(hwnd, lpefr, szFind, COUNTOF(szFind));
  if (slen <= 0)
    return FALSE;

  const DocPos iTextLength = SciCall_GetTextLength();

  DocPos start = SciCall_GetCurrentPos();
  DocPos end = 0;

  if (start <= end) {
    if (IDOK == InfoBox(MBOKCANCEL, L"MsgFindWrap1", IDS_FIND_WRAPFW)) {
      end = start;  start = iTextLength;
    }
    else
      bSuppressNotFound = TRUE;
  }

  DocPos iPos = EditFindInTarget(hwnd, szFind, slen, (int)(lpefr->fuFlags), &start, &end, TRUE);

  if ((iPos < -1) && (lpefr->fuFlags & SCFIND_REGEXP)) 
  {
    InfoBox(MBWARN, L"MsgInvalidRegex", IDS_REGEX_INVALID);
    bSuppressNotFound = TRUE;
  }
  else if ((iPos < 0) && (start <= iTextLength) && !lpefr->bNoFindWrap && !bExtendSelection && !bSuppressNotFound) 
  {
    if (IDOK == InfoBox(MBOKCANCEL, L"MsgFindWrap2", IDS_FIND_WRAPRE)) 
    {
      end = start;  start = iTextLength;

      iPos = EditFindInTarget(hwnd, szFind, slen, (int)(lpefr->fuFlags), &start, &end, FALSE);

      if ((iPos < -1) && (lpefr->fuFlags & SCFIND_REGEXP)) {
        InfoBox(MBWARN, L"MsgInvalidRegex2", IDS_REGEX_INVALID);
        bSuppressNotFound = TRUE;
      }
    }
    else
      bSuppressNotFound = TRUE;
  }

  if (iPos < 0) {
    if (!bSuppressNotFound)
      InfoBox(0, L"MsgNotFound", IDS_NOTFOUND);
    return FALSE;
  }

  if (bExtendSelection) {
    DocPos iSelPos = SciCall_GetCurrentPos();
    DocPos iSelAnchor = SciCall_GetAnchor();
    EditSelectEx(hwnd, max(iSelPos, iSelAnchor), start);
  }
  else {
    EditSelectEx(hwnd, end, start);
  }
  return TRUE;
}


//=============================================================================
//
//  EditMarkAllOccurrences()
// 
void EditMarkAllOccurrences()
{
  if (iMarkOccurrences != 0) {
    
    if (EditIsInTargetTransaction()) { return; }  // do not block, next event occurs for sure

    if (bMarkOccurrencesMatchVisible)
    {
      // get visible lines for update
      DocLn iFirstVisibleLine = SciCall_DocLineFromVisible(SciCall_GetFirstVisibleLine());

      DocLn iStartLine = max(0, (iFirstVisibleLine - SciCall_LinesOnScreen()));
      DocLn iEndLine = min((iFirstVisibleLine + (SciCall_LinesOnScreen() << 1)), (SciCall_GetLineCount() - 1));

      DocPos iPosStart = SciCall_PositionFromLine(iStartLine);
      DocPos iPosEnd = SciCall_GetLineEndPosition(iEndLine);

      // !!! don't clear all marks, else this method is re-called
      // !!! on UpdateUI notification on drawing indicator mark
      EditMarkAll(g_hwndEdit, NULL, bMarkOccurrencesCurrentWord, iPosStart, iPosEnd, bMarkOccurrencesMatchCase, bMarkOccurrencesMatchWords);
    }
    else {
      EditMarkAll(g_hwndEdit, NULL, bMarkOccurrencesCurrentWord, 0, SciCall_GetTextLength(), bMarkOccurrencesMatchCase, bMarkOccurrencesMatchWords);
      UpdateStatusbar();
    }
    EditLeaveTargetTransaction();
  }
  else {
    iMarkOccurrencesCount = 0;
  }
}


//=============================================================================
//
//  EditUpdateVisibleUrlHotspot()
// 
void EditUpdateVisibleUrlHotspot(BOOL bEnabled)
{
  if (bEnabled)
  {
    if (EditIsInTargetTransaction()) { return; }  // do not block, next event occurs for sure

    // get visible lines for update
    DocLn iFirstVisibleLine = SciCall_DocLineFromVisible(SciCall_GetFirstVisibleLine());

    DocLn iStartLine = max(0, (iFirstVisibleLine - SciCall_LinesOnScreen()));
    DocLn iEndLine = min((iFirstVisibleLine + (SciCall_LinesOnScreen() << 1)), (SciCall_GetLineCount() - 1));

    DocPos iPosStart = SciCall_PositionFromLine(iStartLine);
    DocPos iPosEnd = SciCall_GetLineEndPosition(iEndLine);

    EditUpdateUrlHotspots(g_hwndEdit, iPosStart, iPosEnd, bEnabled);

    EditLeaveTargetTransaction();
  }
}


//=============================================================================
//
//  EditGetReplaceString()
//
char* __fastcall EditGetReplaceString(HWND hwnd, LPCEDITFINDREPLACE lpefr, int* iReplaceMsg)
{
  char* pszReplace = NULL; // replace text of arbitrary size
  if (StringCchCompareINA(lpefr->szReplace, FNDRPL_BUFFER, "^c", -1) == 0) {
    *iReplaceMsg = SCI_REPLACETARGET;
    pszReplace = EditGetClipboardText(hwnd, TRUE, NULL, NULL);
  }
  else {
    pszReplace = StrDupA(lpefr->szReplace);
    if (!pszReplace) {
      pszReplace = StrDupA("");
    }
    BOOL bIsRegEx = (lpefr->fuFlags & SCFIND_REGEXP);
    if (lpefr->bTransformBS || bIsRegEx) {
      TransformBackslashes(pszReplace, bIsRegEx, Encoding_SciGetCodePage(hwnd), iReplaceMsg);
    }
  }
  return pszReplace;
}


//=============================================================================
//
//  EditReplace()
//
BOOL EditReplace(HWND hwnd, LPCEDITFINDREPLACE lpefr) {

  int iReplaceMsg = SCI_REPLACETARGET;
  char* pszReplace = EditGetReplaceString(hwnd, lpefr, &iReplaceMsg);
  if (!pszReplace)
    return FALSE; // recoding of clipboard canceled

  // redo find to get group ranges filled
  DocPos start = (SciCall_IsSelectionEmpty() ? SciCall_GetCurrentPos() : SciCall_GetSelectionStart());
  DocPos end = SciCall_GetTextLength();
  DocPos _start = start;
  iReplacedOccurrences = 0;

  const DocPos iPos = EditFindInTarget(hwnd, lpefr->szFind, StringCchLenA(lpefr->szFind, FNDRPL_BUFFER),  (int)(lpefr->fuFlags), &start, &end, FALSE);

  // w/o selection, replacement string is put into current position
  // but this maybe not intended here
  if (SciCall_IsSelectionEmpty()) {
    if ((iPos < 0) || (_start != start) || (_start != end)) {
      // empty-replace was not intended
      LocalFree(pszReplace);
      if (iPos < 0)
        return EditFindNext(hwnd, lpefr, FALSE, FALSE);
      else {
        EditSelectEx(hwnd, start, end);
        return TRUE;
      }
    }
  }
  iReplacedOccurrences = 1;

  EditEnterTargetTransaction();

  SciCall_TargetFromSelection();
  SendMessage(hwnd, iReplaceMsg, (WPARAM)-1, (LPARAM)pszReplace);

  // move caret behind replacement

  const DocPos after = SciCall_GetTargetEnd();
  SciCall_SetSel(after, after);

  EditLeaveTargetTransaction();

  LocalFree(pszReplace);

  return EditFindNext(hwnd, lpefr, FALSE, FALSE);
}



//=============================================================================
//
//  EditReplaceAllInRange()
//

typedef struct _replPos
{
  DocPos beg;
  DocPos end;
}
ReplPos_t;

static UT_icd ReplPos_icd = { sizeof(ReplPos_t), NULL, NULL, NULL };

// -------------------------------------------------------------------------------------------------------

int EditReplaceAllInRange(HWND hwnd, LPCEDITFINDREPLACE lpefr, DocPos iStartPos, DocPos iEndPos, DocPos* enlargement)
{
  char szFind[FNDRPL_BUFFER];

  if (iStartPos > iEndPos) { swapos(&iStartPos, &iEndPos); }

  int slen = EditGetFindStrg(hwnd, lpefr, szFind, COUNTOF(szFind));
  if (slen <= 0) { return 0; }

  int iReplaceMsg = SCI_REPLACETARGET;
  char* pszReplace = EditGetReplaceString(hwnd, lpefr, &iReplaceMsg);
  if (!pszReplace) {
    return -1; // recoding of clipboard canceled
  }

  UT_array* ReplPosUTArray = NULL;
  utarray_new(ReplPosUTArray, &ReplPos_icd);
  utarray_reserve(ReplPosUTArray, (2 * SciCall_GetLineCount()) );
  
  DocPos start = iStartPos;
  DocPos end = iEndPos;

  DocPos iPos = EditFindInTarget(hwnd, szFind, slen, (int)(lpefr->fuFlags), &start, &end, FALSE);

  if ((iPos < -1) && (lpefr->fuFlags & SCFIND_REGEXP)) {
    InfoBox(MBWARN, L"MsgInvalidRegex", IDS_REGEX_INVALID);
  }

  // ===  build array of matches for later replacements  ===

  ReplPos_t posPair = { 0, 0 };

  while ((iPos >= 0) && (start <= iEndPos))
  {
    posPair.beg = start;
    posPair.end = end;
    utarray_push_back(ReplPosUTArray, &posPair);

    start = end;
    end = iEndPos;

    if (start <= iEndPos)
      iPos = EditFindInTarget(hwnd, szFind, slen, (int)(lpefr->fuFlags), &start, &end, ((posPair.end - posPair.beg) == 0));
    else
      iPos = -1;
  } 
  
  int iCount = utarray_len(ReplPosUTArray);

  // ===  iterate over findings and replace strings  ===
  IgnoreNotifyChangeEvent();

  DocPos offset = 0;
  for (ReplPos_t* pPosPair = (ReplPos_t*)utarray_front(ReplPosUTArray);
                  pPosPair != NULL;
                  pPosPair = (ReplPos_t*)utarray_next(ReplPosUTArray, pPosPair)) {

    // redo find to get group ranges filled
    start = pPosPair->beg + offset;
    end = iEndPos + offset;

    iPos = EditFindInTarget(hwnd, szFind, slen, (int)(lpefr->fuFlags), &start, &end, FALSE);

    EditEnterTargetTransaction();

    SciCall_SetTargetRange(start, end);

    offset += ((DocPos)SendMessage(hwnd, iReplaceMsg, (WPARAM)-1, (LPARAM)pszReplace) - pPosPair->end + pPosPair->beg);

    EditLeaveTargetTransaction();
  }

  ObserveNotifyChangeEvent();

  utarray_clear(ReplPosUTArray);
  utarray_free(ReplPosUTArray);
  LocalFree(pszReplace);

  *enlargement = offset;

  return iCount;
}


//=============================================================================
//
//  EditReplaceAll()
//
BOOL EditReplaceAll(HWND hwnd, LPCEDITFINDREPLACE lpefr, BOOL bShowInfo)
{
  const DocPos start = 0;
  const DocPos end = SciCall_GetTextLength();
  DocPos enlargement = 0;

  BeginWaitCursor(NULL);

  int token = BeginUndoAction();

  iReplacedOccurrences = EditReplaceAllInRange(hwnd, lpefr, start, end, &enlargement);

  EndUndoAction(token);

  EndWaitCursor();

  if (bShowInfo) {
    if (iReplacedOccurrences > 0)
      InfoBox(0, L"MsgReplaceCount", IDS_REPLCOUNT, iReplacedOccurrences);
    else
      InfoBox(0, L"MsgNotFound", IDS_NOTFOUND);
  }

  return (iReplacedOccurrences > 0) ? TRUE : FALSE;
}


//=============================================================================
//
//  EditReplaceAllInSelection()
//
BOOL EditReplaceAllInSelection(HWND hwnd, LPCEDITFINDREPLACE lpefr, BOOL bShowInfo)
{
  if (SciCall_IsSelectionRectangle()) {
    MsgBox(MBWARN, IDS_SELRECT);
    return FALSE;
  }

  const DocPos start = SciCall_GetSelectionStart();
  const DocPos end = SciCall_GetSelectionEnd();
  const DocPos currPos = SciCall_GetCurrentPos();
  const DocPos anchorPos = SciCall_GetAnchor();
  DocPos enlargement = 0;
  bool bWaitCursor = false;

  if ((end - start) > (512 * 512)) {
    BeginWaitCursor(NULL);
    bWaitCursor = true;
  }

  int token = BeginUndoAction();

  iReplacedOccurrences = EditReplaceAllInRange(hwnd, lpefr, start, end, &enlargement);

  if (bWaitCursor) {
    EndWaitCursor();
  }

  if (iReplacedOccurrences <= 0) {
    EndUndoAction(token);
    return FALSE;
  }

  if (currPos < anchorPos)
    SciCall_SetSel(anchorPos + enlargement, currPos);
  else
    SciCall_SetSel(anchorPos, currPos + enlargement);

  EndUndoAction(token);

  if (bShowInfo) {
    if (iReplacedOccurrences > 0)
      InfoBox(0, L"MsgReplaceCount", IDS_REPLCOUNT, iReplacedOccurrences);
    else
      InfoBox(0, L"MsgNotFound", IDS_NOTFOUND);
  }

  return (iReplacedOccurrences > 0) ? TRUE : FALSE;
}


//=============================================================================
//
//  EditClearAllMarks()
//
void EditClearAllMarks(HWND hwnd, DocPos iRangeStart, DocPos iRangeEnd)
{
  if (iRangeEnd <= 0) {
    iRangeEnd = SciCall_GetTextLength();
  }
  if (iRangeStart > iRangeEnd) {
    swapos(&iRangeStart, &iRangeEnd);
  }
  SendMessage(hwnd, SCI_SETINDICATORCURRENT, INDIC_NP3_MARK_OCCURANCE, 0);
  SendMessage(hwnd, SCI_INDICATORCLEARRANGE, iRangeStart, iRangeEnd);
}


//=============================================================================
//
//  EditMarkAll()
//  Mark all occurrences of the matching text in range (by Aleksandar Lekov)
//
void EditMarkAll(HWND hwnd, char* pszFind, int flags, DocPos rangeStart, DocPos rangeEnd, BOOL bMatchCase, BOOL bMatchWords)
{
  char* pszText = NULL;
  char txtBuffer[HUGE_BUFFER] = { '\0' };

  DocPos iFindLength = 0;

  if (pszFind != NULL)
    pszText = pszFind;
  else
    pszText = txtBuffer;

  if (pszFind == NULL) {

    if (SciCall_IsSelectionEmpty()) {
      if (flags) { // nothing selected, get word under caret if flagged
        DocPos iCurrPos = SciCall_GetCurrentPos();
        DocPos iWordStart = (DocPos)SendMessage(hwnd, SCI_WORDSTARTPOSITION, iCurrPos, (LPARAM)1);
        DocPos iWordEnd = (DocPos)SendMessage(hwnd, SCI_WORDENDPOSITION, iCurrPos, (LPARAM)1);
        iFindLength = (iWordEnd - iWordStart);
        StringCchCopyNA(pszText, HUGE_BUFFER, SciCall_GetRangePointer(iWordStart, iFindLength), iFindLength);
      }
      else {
        return; // no selection and no word mark chosen
      }
    }
    else { // selection found

      if (flags) { return; } // no current word matching if we have a selection 

      // get current selection
      DocPos iSelStart = SciCall_GetSelectionStart();
      DocPos iSelEnd = SciCall_GetSelectionEnd();
      DocPos iSelCount = (iSelEnd - iSelStart);

      // if multiple lines are selected exit
      
      if ((SciCall_LineFromPosition(iSelStart) != SciCall_LineFromPosition(iSelEnd)) || (iSelCount >= HUGE_BUFFER)) {
        return;
      }
      
      iFindLength = SciCall_GetSelText(pszText) - 1;

      // exit if selection is not a word and Match whole words only is enabled
      if (bMatchWords) {
        DocPos iSelStart2 = 0;
        const char* delims = (bAccelWordNavigation ? DelimCharsAccel : DelimChars);
        while ((iSelStart2 <= iSelCount) && pszText[iSelStart2]) {
          if (StrChrIA(delims, pszText[iSelStart2])) {
            return;
          }
          iSelStart2++;
        }
      }
    }
    // set additional flags
    flags = flags ? SCFIND_WHOLEWORD : 0; // match current word under caret ?
    flags |= (bMatchWords) ? SCFIND_WHOLEWORD : 0;
    flags |= (bMatchCase ? SCFIND_MATCHCASE : 0);
  }
  else {
    iFindLength = StringCchLenA(pszFind, FNDRPL_BUFFER);
  }

  if (iFindLength > 0) {

    const DocPos iTextLength = SciCall_GetTextLength();
    rangeStart = max(0, rangeStart);
    rangeEnd = min(rangeEnd, iTextLength);

    DocPos start = rangeStart;
    DocPos end = rangeEnd;


    iMarkOccurrencesCount = 0;
    SendMessage(hwnd, SCI_SETINDICATORCURRENT, INDIC_NP3_MARK_OCCURANCE, 0);

    DocPos iPos = (DocPos)-1;
    do {

      iPos = EditFindInTarget(hwnd, pszText, iFindLength, flags, &start, &end, (start == iPos));

      if (iPos < 0)
        break; // not found

      //// mark this match if not done before
      SciCall_IndicatorFillRange(iPos, (end - start));

      start = end;
      end = rangeEnd;

    } while ((++iMarkOccurrencesCount < iMarkOccurrencesMaxCount) && (start < end));
  }
}


//=============================================================================
//
//  EditCompleteWord()
//  Auto-complete words (by Aleksandar Lekov)
//
struct WLIST {
  char* word;
  struct WLIST* next;
};

void EditCompleteWord(HWND hwnd, BOOL autoInsert) 
{
  const char* NON_WORD = bAccelWordNavigation ? DelimCharsAccel : DelimChars;

  const DocPos iCurrentPos = SciCall_GetCurrentPos();
  const DocLn iLine = SciCall_LineFromPosition(iCurrentPos);
  const DocPos iLineStart = SciCall_PositionFromLine(iLine);
  const DocPos iCurrentLinePos = iCurrentPos - iLineStart;

  DocPos iLineLen = SciCall_GetLine(iLine, NULL);
  const char* pLine = SciCall_GetRangePointer(iLineStart, iLineLen);

  BOOL bWordAllNumbers = TRUE;
  DocPos iStartWordPos = iCurrentLinePos;
  while (iStartWordPos > 0 && !StrChrIA(NON_WORD, pLine[iStartWordPos - 1])) {
    iStartWordPos--;
    if (pLine[iStartWordPos] < '0' || pLine[iStartWordPos] > '9') {
      bWordAllNumbers = FALSE;
    }
  }

  if (iStartWordPos == iCurrentLinePos || bWordAllNumbers || iCurrentLinePos - iStartWordPos < 2) {
    return;
  }

  char pRoot[256];
  DocPosCR iRootLen = (DocPosCR)(iCurrentLinePos - iStartWordPos);
  StringCchCopyNA(pRoot, COUNTOF(pRoot), pLine + iStartWordPos, (size_t)iRootLen);

  const DocPosCR iDocLen = (DocPosCR)SciCall_GetTextLength();
  struct Sci_TextToFind ft = { { 0, 0 }, 0, { 0, 0 } };
  ft.lpstrText = pRoot;
  ft.chrg.cpMax = iDocLen;

  DocPos iPosFind = (DocPos)SendMessage(hwnd, SCI_FINDTEXT, SCFIND_WORDSTART, (LPARAM)&ft);

  int iNumWords = 0;
  DocPos iWListSize = 0;
  struct WLIST* lListHead = NULL;

  char pWord[1024];
  while ((iPosFind >= 0) && (iPosFind < iDocLen)) 
  {
    DocPos wordLength;
    DocPos wordEnd = (DocPosCR)(iPosFind + iRootLen);

    if (iPosFind != iCurrentPos - iRootLen) 
    {
      while ((wordEnd < iDocLen) && !StrChrIA(NON_WORD, SciCall_GetCharAt(wordEnd))) { ++wordEnd; }

      wordLength = wordEnd - iPosFind;
      if (wordLength > iRootLen) {
        struct WLIST* p = lListHead;
        struct WLIST* t = NULL;
        BOOL found = FALSE;

        StringCchCopyNA(pWord, COUNTOF(pWord), SciCall_GetRangePointer(iPosFind, wordLength), wordLength);

        while (p) {
          int cmp = lstrcmpA(pWord, p->word);
          if (!cmp) {
            found = TRUE;
            break;
          }
          else if (cmp < 0) {
            break;
          }
          t = p;
          p = p->next;
        }
        if (!found) {
          struct WLIST* el = (struct WLIST*)LocalAlloc(LPTR, sizeof(struct WLIST));
          const DocPos wSize = (wordEnd - iPosFind) + 1;
          el->word = LocalAlloc(LPTR, wSize+1);
          StringCchCopyA(el->word, wSize+1, pWord);
          el->next = p;
          if (t) {
            t->next = el;
          }
          else {
            lListHead = el;
          }
          ++iNumWords;
          iWListSize += wSize;
        }
      }
    }
    ft.chrg.cpMin = (DocPosCR)wordEnd;
    iPosFind = (DocPos)SendMessage(hwnd, SCI_FINDTEXT, SCFIND_WORDSTART, (LPARAM)&ft);
  }

  if (iNumWords > 0) {
    char *pList;
    struct WLIST* p = lListHead;
    struct WLIST* t;

    pList = LocalAlloc(LPTR, iWListSize + 1);
    while (p) {
      lstrcatA(pList, " ");
      lstrcatA(pList, p->word);
      LocalFree(p->word);
      t = p;
      p = p->next;
      LocalFree(t);
    }

    SendMessage(hwnd, SCI_AUTOCSETIGNORECASE, 1, 0);
    SendMessage(hwnd, SCI_AUTOCSETSEPARATOR, ' ', 0);
    SendMessage(hwnd, SCI_AUTOCSETFILLUPS, 0, (LPARAM)"\t\n\r");
    SendMessage(hwnd, SCI_AUTOCSETCHOOSESINGLE, autoInsert, 0);
    SendMessage(hwnd, SCI_AUTOCSHOW, iRootLen, (LPARAM)(pList + 1));
    LocalFree(pList);
  }

//  LocalFree(pRoot);
}



//=============================================================================
//
//  EditUpdateUrlHotspots()
//  Find and mark all URL hot-spots
//
void EditUpdateUrlHotspots(HWND hwnd, DocPos startPos, DocPos endPos, BOOL bActiveHotspot)
{
  if (endPos < startPos) {
    swapos(&startPos, &endPos);
  }

  // 1st apply current lexer style
  EditFinalizeStyling(hwnd,startPos);

  const char* pszUrlRegEx = "\\b(?:(?:https?|ftp|file)://|www\\.|ftp\\.)"
    "(?:\\([-A-Z0-9+&@#/%=~_|$?!:,.]*\\)|[-A-Z0-9+&@#/%=~_|$?!:,.])*"
    "(?:\\([-A-Z0-9+&@#/%=~_|$?!:,.]*\\)|[A-Z0-9+&@#/%=~_|$])";

  const int iRegExLen = (int)strlen(pszUrlRegEx);

  if (startPos < 0) { // current line only
    DocPos currPos = SciCall_GetCurrentPos();
    DocLn lineNo = SciCall_LineFromPosition(currPos);
    startPos = SciCall_PositionFromLine(lineNo);
    endPos = SciCall_GetLineEndPosition(lineNo);
  }
  if (endPos == startPos)
    return;

  DocPos start = startPos;
  DocPos end = endPos;
  int iStyle = bActiveHotspot ? Style_GetHotspotStyleID() : STYLE_DEFAULT;
  
  do {
    DocPos iPos = EditFindInTarget(hwnd, pszUrlRegEx, iRegExLen, SCFIND_NP3_REGEX, &start, &end, FALSE);

    if (iPos < 0)
      break; // not found

    DocPos mlen = end - start;
    if ((mlen <= 0) || ((iPos + mlen) > endPos))
      break; // wrong match

    // mark this match
    SciCall_StartStyling(iPos);
    SciCall_SetStyling((DocPosCR)mlen, iStyle);

    // next occurrence
    start = end;
    end = endPos;

  } while (start < end);


  if (bActiveHotspot) 
    SciCall_StartStyling(endPos);
  else
    SciCall_StartStyling(startPos);
}


//=============================================================================
//
//  EditHighlightIfBrace()
//
BOOL __fastcall EditHighlightIfBrace(HWND hwnd, DocPos iPos) 
{
  if (iPos < 0) {
    // clear indicator
    SendMessage(hwnd, SCI_BRACEBADLIGHT, (WPARAM)INVALID_POSITION, 0);
    SendMessage(hwnd, SCI_SETHIGHLIGHTGUIDE, 0, 0);
    if (!bUseOldStyleBraceMatching)
      SendMessage(hwnd, SCI_BRACEBADLIGHTINDICATOR, 0, INDIC_NP3_BAD_BRACE);
    return TRUE;
  }

  char c = SciCall_GetCharAt(iPos);
  
  if (StrChrA("()[]{}", c)) {
    DocPos iBrace2 = (DocPos)SendMessage(hwnd, SCI_BRACEMATCH, iPos, 0);
    if (iBrace2 != -1) {
      DocPos col1 = SciCall_GetColumn(iPos);
      DocPos col2 = SciCall_GetColumn(iBrace2);
      SendMessage(hwnd, SCI_BRACEHIGHLIGHT, iPos, iBrace2);
      SendMessage(hwnd, SCI_SETHIGHLIGHTGUIDE, min(col1, col2), 0);
      if (!bUseOldStyleBraceMatching) {
        SendMessage(hwnd, SCI_BRACEHIGHLIGHTINDICATOR, 1, INDIC_NP3_MATCH_BRACE);
      }
    }
    else {
      SendMessage(hwnd, SCI_BRACEBADLIGHT, iPos, 0);
      SendMessage(hwnd, SCI_SETHIGHLIGHTGUIDE, 0, 0);
      if (!bUseOldStyleBraceMatching) {
        SendMessage(hwnd, SCI_BRACEBADLIGHTINDICATOR, 1, INDIC_NP3_BAD_BRACE);
      }
    }
    return TRUE;
  }
  return FALSE;
}


//=============================================================================
//
//  EditApplyLexerStyle()
//
void EditApplyLexerStyle(HWND hwnd, DocPos iRangeStart, DocPos iRangeEnd)
{
  SendMessage(hwnd, SCI_COLOURISE, (WPARAM)iRangeStart, (LPARAM)iRangeEnd);
}


//=============================================================================
//
//  EditFinalizeStyling()
//
void EditFinalizeStyling(HWND hwnd, DocPos iEndPos)
{
  const int iEndStyled = SciCall_GetEndStyled();

  if ((iEndPos < 0) || (iEndStyled < iEndPos))
  {
    const DocLn iLineEndStyled = SciCall_LineFromPosition(iEndStyled);
    const DocPos iStartStyling = SciCall_PositionFromLine(iLineEndStyled);
    EditApplyLexerStyle(hwnd, iStartStyling, iEndPos);
  }
}


//=============================================================================
//
//  EditMatchBrace()
//
void EditMatchBrace(HWND hwnd) 
{
  DocPos iPos = SciCall_GetCurrentPos();

  EditFinalizeStyling(hwnd, iPos);

  if (!EditHighlightIfBrace(hwnd, iPos)) {
    // try one before
    iPos = SciCall_PositionBefore(iPos);
    if (!EditHighlightIfBrace(hwnd, iPos)) {
      // clear mark
      EditHighlightIfBrace(hwnd, -1);
    }
  }
}



//=============================================================================
//
//  EditLinenumDlgProc()
//
INT_PTR CALLBACK EditLinenumDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        DocLn iCurLine = SciCall_LineFromPosition(SciCall_GetCurrentPos())+1;
        DocPos iCurColumn = SciCall_GetColumn(SciCall_GetCurrentPos()) + 1;

        SetDlgItemInt(hwnd, IDC_LINENUM, (UINT)iCurLine, FALSE);
        SetDlgItemInt(hwnd, IDC_COLNUM, (UINT)iCurColumn, FALSE);
        SendDlgItemMessage(hwnd,IDC_LINENUM,EM_LIMITTEXT,15,0);
        SendDlgItemMessage(hwnd,IDC_COLNUM,EM_LIMITTEXT,15,0);
        CenterDlgInParent(hwnd);
      }
      return TRUE;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {
        case IDOK: 
        {
          BOOL fTranslated = TRUE;
          int iNewLine = (int)GetDlgItemInt(hwnd,IDC_LINENUM,&fTranslated,FALSE);

          DocLn iMaxLine = (DocLn)SendMessage(g_hwndEdit,SCI_GETLINECOUNT,0,0);

          DocPos iNewCol = 1;
          BOOL fTranslated2 = TRUE;
          if (SendDlgItemMessage(hwnd, IDC_COLNUM, WM_GETTEXTLENGTH, 0, 0) > 0) {
            iNewCol = GetDlgItemInt(hwnd, IDC_COLNUM, &fTranslated2, FALSE);
          }

          if (!fTranslated || !fTranslated2)
          {
            PostMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)(GetDlgItem(hwnd,(!fTranslated) ? IDC_LINENUM : IDC_COLNUM)),1);
            return TRUE;
          }

          if ((iNewLine > 0) && (iNewLine <= iMaxLine) && (iNewCol > 0))
          {
            EditJumpTo(g_hwndEdit,iNewLine,iNewCol);
            EndDialog(hwnd,IDOK);
          }
          else {
            PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, (!((iNewLine > 0) && (iNewLine <= iMaxLine))) ? IDC_LINENUM : IDC_COLNUM)), 1);
          }
        }
        break;

        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;

      }
      return TRUE;

  }

  UNUSED(lParam);

  return FALSE;
}


//=============================================================================
//
//  EditLinenumDlg()
//
BOOL EditLinenumDlg(HWND hwnd)
{

  if (IDOK == ThemedDialogBoxParam(g_hInstance,MAKEINTRESOURCE(IDD_LINENUM),
                             GetParent(hwnd),EditLinenumDlgProc,(LPARAM)hwnd))
    return TRUE;

  else
    return FALSE;

}


//=============================================================================
//
//  EditModifyLinesDlg()
//
//  Controls: 100 Input
//            101 Input
//
typedef struct _modlinesdata {
  LPWSTR pwsz1;
  LPWSTR pwsz2;
} MODLINESDATA, *PMODLINESDATA;


INT_PTR CALLBACK EditModifyLinesDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  static PMODLINESDATA pdata;

  static int id_hover;
  static int id_capture;

  static HFONT hFontNormal;
  static HFONT hFontHover;

  static HCURSOR hCursorNormal;
  static HCURSOR hCursorHover;

  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        LOGFONT lf;

        id_hover = 0;
        id_capture = 0;

        if (NULL == (hFontNormal = (HFONT)SendDlgItemMessage(hwnd,200,WM_GETFONT,0,0)))
          hFontNormal = GetStockObject(DEFAULT_GUI_FONT);
        GetObject(hFontNormal,sizeof(LOGFONT),&lf);
        lf.lfUnderline = TRUE;
        hFontHover = CreateFontIndirect(&lf);

        hCursorNormal = LoadCursor(NULL,IDC_ARROW);
        hCursorHover = LoadCursor(NULL,IDC_HAND);
        if (!hCursorHover)
          hCursorHover = LoadCursor(g_hInstance, IDC_ARROW);

        pdata = (PMODLINESDATA)lParam;
        SetDlgItemTextW(hwnd,100,pdata->pwsz1);
        SendDlgItemMessage(hwnd,100,EM_LIMITTEXT,255,0);
        SetDlgItemTextW(hwnd,101,pdata->pwsz2);
        SendDlgItemMessage(hwnd,101,EM_LIMITTEXT,255,0);
        CenterDlgInParent(hwnd);
      }
      return TRUE;

    case WM_DESTROY:
      DeleteObject(hFontHover);
      return FALSE;

    case WM_NCACTIVATE:
      if (!(BOOL)wParam) {
        if (id_hover != 0) {
          //int _id_hover = id_hover;
          id_hover = 0;
          id_capture = 0;
          //InvalidateRect(GetDlgItem(hwnd,id_hover),NULL,FALSE);
        }
      }
      return FALSE;

    case WM_CTLCOLORSTATIC:
      {
        DWORD dwId = GetWindowLong((HWND)lParam,GWL_ID);
        HDC hdc = (HDC)wParam;

        if (dwId >= 200 && dwId <= 205) {
          SetBkMode(hdc,TRANSPARENT);
          if (GetSysColorBrush(COLOR_HOTLIGHT))
            SetTextColor(hdc,GetSysColor(COLOR_HOTLIGHT));
          else
            SetTextColor(hdc,RGB(0, 0, 0xFF));
          SelectObject(hdc,/*dwId == id_hover?*/hFontHover/*:hFontNormal*/);
          return (INT_PTR)GetSysColorBrush(COLOR_BTNFACE);
        }
      }
      break;

    case WM_MOUSEMOVE:
      {
        POINT pt;
        pt.x = LOWORD(lParam);  pt.y = HIWORD(lParam);
        HWND hwndHover = ChildWindowFromPoint(hwnd,pt);
        DWORD dwId = (DWORD)GetWindowLong(hwndHover,GWL_ID);

        if (GetActiveWindow() == hwnd) {
          if (dwId >= 200 && dwId <= 205) {
            if (id_capture == (int)dwId || id_capture == 0) {
              if (id_hover != id_capture || id_hover == 0) {
                id_hover = (int)dwId;
                //InvalidateRect(GetDlgItem(hwnd,dwId),NULL,FALSE);
              }
            }
            else if (id_hover != 0) {
              //int _id_hover = id_hover;
              id_hover = 0;
              //InvalidateRect(GetDlgItem(hwnd,_id_hover),NULL,FALSE);
            }
          }
          else if (id_hover != 0) {
            //int _id_hover = id_hover;
            id_hover = 0;
            //InvalidateRect(GetDlgItem(hwnd,_id_hover),NULL,FALSE);
          }
          SetCursor(id_hover != 0 ? hCursorHover : hCursorNormal);
        }
      }
      break;

    case WM_LBUTTONDOWN:
      {
        POINT pt;
        pt.x = LOWORD(lParam);  pt.y = HIWORD(lParam);
        HWND hwndHover = ChildWindowFromPoint(hwnd,pt);
        DWORD dwId = GetWindowLong(hwndHover,GWL_ID);

        if (dwId >= 200 && dwId <= 205) {
          GetCapture();
          id_hover = dwId;
          id_capture = dwId;
          //InvalidateRect(GetDlgItem(hwnd,dwId),NULL,FALSE);
        }
        SetCursor(id_hover != 0?hCursorHover:hCursorNormal);
      }
      break;

    case WM_LBUTTONUP:
      {
        POINT pt;
        pt.x = LOWORD(lParam);  pt.y = HIWORD(lParam);
        //HWND hwndHover = ChildWindowFromPoint(hwnd,pt);
        //DWORD dwId = GetWindowLong(hwndHover,GWL_ID);
        if (id_capture != 0) {
          ReleaseCapture();
          if (id_hover == id_capture) {
            int id_focus = GetWindowLong(GetFocus(),GWL_ID);
            if (id_focus == 100 || id_focus == 101) {
              WCHAR wch[8];
              GetDlgItemText(hwnd,id_capture,wch,COUNTOF(wch));
              SendDlgItemMessage(hwnd,id_focus,EM_SETSEL,(WPARAM)0,(LPARAM)-1);
              SendDlgItemMessage(hwnd,id_focus,EM_REPLACESEL,(WPARAM)TRUE,(LPARAM)wch);
              PostMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)(GetFocus()),1);
            }
          }
          id_capture = 0;
        }
        SetCursor(id_hover != 0?hCursorHover:hCursorNormal);
      }
      break;

    case WM_CANCELMODE:
      if (id_capture != 0) {
        ReleaseCapture();
        id_hover = 0;
        id_capture = 0;
        SetCursor(hCursorNormal);
      }
      break;
    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
        case IDOK: {
            GetDlgItemTextW(hwnd,100,pdata->pwsz1,256);
            GetDlgItemTextW(hwnd,101,pdata->pwsz2,256);
            EndDialog(hwnd,IDOK);
          }
          break;
        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;
      }
      return TRUE;
  }
  return FALSE;
}


//=============================================================================
//
//  EditModifyLinesDlg()
//
BOOL EditModifyLinesDlg(HWND hwnd,LPWSTR pwsz1,LPWSTR pwsz2)
{

  INT_PTR iResult;
  MODLINESDATA data;
  data.pwsz1 = pwsz1;  data.pwsz2 = pwsz2;

  iResult = ThemedDialogBoxParam(
              g_hInstance,
              MAKEINTRESOURCEW(IDD_MODIFYLINES),
              hwnd,
              EditModifyLinesDlgProc,
              (LPARAM)&data);

  return (iResult == IDOK) ? TRUE : FALSE;

}


//=============================================================================
//
//  EditAlignDlgProc()
//
//  Controls: 100 Radio Button
//            101 Radio Button
//            102 Radio Button
//            103 Radio Button
//            104 Radio Button
//
INT_PTR CALLBACK EditAlignDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  static int *piAlignMode;
  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        piAlignMode = (int*)lParam;
        CheckRadioButton(hwnd,100,104,*piAlignMode+100);
        CenterDlgInParent(hwnd);
      }
      return TRUE;
    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
        case IDOK: {
            *piAlignMode = 0;
            if (IsDlgButtonChecked(hwnd,100) == BST_CHECKED)
              *piAlignMode = ALIGN_LEFT;
            else if (IsDlgButtonChecked(hwnd,101) == BST_CHECKED)
              *piAlignMode = ALIGN_RIGHT;
            else if (IsDlgButtonChecked(hwnd,102) == BST_CHECKED)
              *piAlignMode = ALIGN_CENTER;
            else if (IsDlgButtonChecked(hwnd,103) == BST_CHECKED)
              *piAlignMode = ALIGN_JUSTIFY;
            else if (IsDlgButtonChecked(hwnd,104) == BST_CHECKED)
              *piAlignMode = ALIGN_JUSTIFY_EX;
            EndDialog(hwnd,IDOK);
          }
          break;
        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;
      }
      return TRUE;
  }
  return FALSE;
}


//=============================================================================
//
//  EditAlignDlg()
//
BOOL EditAlignDlg(HWND hwnd,int *piAlignMode)
{

  INT_PTR iResult;

  iResult = ThemedDialogBoxParam(
              g_hInstance,
              MAKEINTRESOURCEW(IDD_ALIGN),
              hwnd,
              EditAlignDlgProc,
              (LPARAM)piAlignMode);

  return (iResult == IDOK) ? TRUE : FALSE;

}


//=============================================================================
//
//  EditEncloseSelectionDlgProc()
//
//  Controls: 100 Input
//            101 Input
//
typedef struct _encloseselectiondata {
  LPWSTR pwsz1;
  LPWSTR pwsz2;
} ENCLOSESELDATA, *PENCLOSESELDATA;


INT_PTR CALLBACK EditEncloseSelectionDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  static PENCLOSESELDATA pdata;
  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        pdata = (PENCLOSESELDATA)lParam;
        SendDlgItemMessage(hwnd,100,EM_LIMITTEXT,255,0);
        SetDlgItemTextW(hwnd,100,pdata->pwsz1);
        SendDlgItemMessage(hwnd,101,EM_LIMITTEXT,255,0);
        SetDlgItemTextW(hwnd,101,pdata->pwsz2);
        CenterDlgInParent(hwnd);
      }
      return TRUE;
    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
        case IDOK: {
            GetDlgItemTextW(hwnd,100,pdata->pwsz1,256);
            GetDlgItemTextW(hwnd,101,pdata->pwsz2,256);
            EndDialog(hwnd,IDOK);
          }
          break;
        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;
      }
      return TRUE;
  }
  return FALSE;
}


//=============================================================================
//
//  EditEncloseSelectionDlg()
//
BOOL EditEncloseSelectionDlg(HWND hwnd,LPWSTR pwszOpen,LPWSTR pwszClose)
{

  INT_PTR iResult;
  ENCLOSESELDATA data;
  data.pwsz1 = pwszOpen;  data.pwsz2 = pwszClose;

  iResult = ThemedDialogBoxParam(
              g_hInstance,
              MAKEINTRESOURCEW(IDD_ENCLOSESELECTION),
              hwnd,
              EditEncloseSelectionDlgProc,
              (LPARAM)&data);

  return (iResult == IDOK) ? TRUE : FALSE;

}


//=============================================================================
//
//  EditInsertTagDlgProc()
//
//  Controls: 100 Input
//            101 Input
//
typedef struct _tagsdata {
  LPWSTR pwsz1;
  LPWSTR pwsz2;
} TAGSDATA, *PTAGSDATA;


INT_PTR CALLBACK EditInsertTagDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  static PTAGSDATA pdata;
  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        pdata = (PTAGSDATA)lParam;
        SendDlgItemMessage(hwnd,100,EM_LIMITTEXT,254,0);
        SetDlgItemTextW(hwnd,100,L"<tag>");
        SendDlgItemMessage(hwnd,101,EM_LIMITTEXT,255,0);
        SetDlgItemTextW(hwnd,101,L"</tag>");
        SetFocus(GetDlgItem(hwnd,100));
        PostMessage(GetDlgItem(hwnd,100),EM_SETSEL,1,4);
        CenterDlgInParent(hwnd);
      }
      return FALSE;
    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
        case 100: {
            if (HIWORD(wParam) == EN_CHANGE) {

              WCHAR wchBuf[256] = { L'\0' };
              WCHAR wchIns[256] = L"</";
              BOOL bClear = TRUE;

              GetDlgItemTextW(hwnd,100,wchBuf,256);
              if (StringCchLenW(wchBuf,COUNTOF(wchBuf)) >= 3) {

                if (wchBuf[0] == L'<') 
                {
                  int  cchIns = 2;
                  const WCHAR* pwCur = &wchBuf[1];
                  while (
                    *pwCur &&
                    *pwCur != L'<' &&
                    *pwCur != L'>' &&
                    *pwCur != L' ' &&
                    *pwCur != L'\t' &&
                    (StrChr(L":_-.",*pwCur) || IsCharAlphaNumericW(*pwCur)))

                      wchIns[cchIns++] = *pwCur++;

                  while (
                    *pwCur &&
                    *pwCur != L'>')

                      pwCur++;

                  if (*pwCur == L'>' && *(pwCur-1) != L'/') {
                    wchIns[cchIns++] = L'>';
                    wchIns[cchIns] = L'\0';

                    if (cchIns > 3 &&
                        StringCchCompareIN(wchIns,COUNTOF(wchIns),L"</base>",-1) &&
                        StringCchCompareIN(wchIns,COUNTOF(wchIns),L"</bgsound>",-1) &&
                        StringCchCompareIN(wchIns,COUNTOF(wchIns),L"</br>",-1) &&
                        StringCchCompareIN(wchIns,COUNTOF(wchIns),L"</embed>",-1) &&
                        StringCchCompareIN(wchIns,COUNTOF(wchIns),L"</hr>",-1) &&
                        StringCchCompareIN(wchIns,COUNTOF(wchIns),L"</img>",-1) &&
                        StringCchCompareIN(wchIns,COUNTOF(wchIns),L"</input>",-1) &&
                        StringCchCompareIN(wchIns,COUNTOF(wchIns),L"</link>",-1) &&
                        StringCchCompareIN(wchIns,COUNTOF(wchIns),L"</meta>",-1)) {

                        SetDlgItemTextW(hwnd,101,wchIns);
                        bClear = FALSE;
                    }
                  }
                }
              }
              if (bClear)
                SetDlgItemTextW(hwnd,101,L"");
            }
          }
          break;
        case IDOK: {
            GetDlgItemTextW(hwnd,100,pdata->pwsz1,256);
            GetDlgItemTextW(hwnd,101,pdata->pwsz2,256);
            EndDialog(hwnd,IDOK);
          }
          break;
        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;
      }
      return TRUE;
  }
  return FALSE;
}


//=============================================================================
//
//  EditInsertTagDlg()
//
BOOL EditInsertTagDlg(HWND hwnd,LPWSTR pwszOpen,LPWSTR pwszClose)
{

  INT_PTR iResult;
  TAGSDATA data;
  data.pwsz1 = pwszOpen;  data.pwsz2 = pwszClose;
  
  iResult = ThemedDialogBoxParam(
              g_hInstance,
              MAKEINTRESOURCEW(IDD_INSERTTAG),
              hwnd,
              EditInsertTagDlgProc,
              (LPARAM)&data);

  return (iResult == IDOK) ? TRUE : FALSE;

}


//=============================================================================
//
//  EditSortDlgProc()
//
//  Controls: 100-102 Radio Button
//            103-108 Check Box
//
INT_PTR CALLBACK EditSortDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  static int *piSortFlags;
  static BOOL bEnableLogicalSort;

  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        piSortFlags = (int*)lParam;
        if (*piSortFlags & SORT_DESCENDING)
          CheckRadioButton(hwnd,100,102,101);
        else if (*piSortFlags & SORT_SHUFFLE) {
          CheckRadioButton(hwnd,100,102,102);
          DialogEnableWindow(hwnd,103,FALSE);
          DialogEnableWindow(hwnd,104,FALSE);
          DialogEnableWindow(hwnd,105,FALSE);
          DialogEnableWindow(hwnd,106,FALSE);
          DialogEnableWindow(hwnd,107,FALSE);
        }
        else
          CheckRadioButton(hwnd,100,102,100);
        if (*piSortFlags & SORT_MERGEDUP)
          CheckDlgButton(hwnd,103,BST_CHECKED);
        if (*piSortFlags & SORT_UNIQDUP) {
          CheckDlgButton(hwnd,104,BST_CHECKED);
          DialogEnableWindow(hwnd,103,FALSE);
        }
        if (*piSortFlags & SORT_UNIQUNIQ)
          CheckDlgButton(hwnd,105,BST_CHECKED);
        if (*piSortFlags & SORT_NOCASE)
          CheckDlgButton(hwnd,106,BST_CHECKED);
        if (GetProcAddress(GetModuleHandle(L"shlwapi"),"StrCmpLogicalW")) {
          if (*piSortFlags & SORT_LOGICAL)
            CheckDlgButton(hwnd,107,BST_CHECKED);
          bEnableLogicalSort = TRUE;
        }
        else {
          DialogEnableWindow(hwnd,107,FALSE);
          bEnableLogicalSort = FALSE;
        }
        if (!SciCall_IsSelectionRectangle()) {
          *piSortFlags &= ~SORT_COLUMN;
          DialogEnableWindow(hwnd,108,FALSE);
        }
        else {
          *piSortFlags |= SORT_COLUMN;
          CheckDlgButton(hwnd,108,BST_CHECKED);
        }
        CenterDlgInParent(hwnd);
      }
      return TRUE;
    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
        case IDOK: {
            *piSortFlags = 0;
            if (IsDlgButtonChecked(hwnd,101) == BST_CHECKED)
              *piSortFlags |= SORT_DESCENDING;
            if (IsDlgButtonChecked(hwnd,102) == BST_CHECKED)
              *piSortFlags |= SORT_SHUFFLE;
            if (IsDlgButtonChecked(hwnd,103) == BST_CHECKED)
              *piSortFlags |= SORT_MERGEDUP;
            if (IsDlgButtonChecked(hwnd,104) == BST_CHECKED)
              *piSortFlags |= SORT_UNIQDUP;
            if (IsDlgButtonChecked(hwnd,105) == BST_CHECKED)
              *piSortFlags |= SORT_UNIQUNIQ;
            if (IsDlgButtonChecked(hwnd,106) == BST_CHECKED)
              *piSortFlags |= SORT_NOCASE;
            if (IsDlgButtonChecked(hwnd,107) == BST_CHECKED)
              *piSortFlags |= SORT_LOGICAL;
            if (IsDlgButtonChecked(hwnd,108) == BST_CHECKED)
              *piSortFlags |= SORT_COLUMN;
            EndDialog(hwnd,IDOK);
          }
          break;
        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;
        case 100:
        case 101:
          DialogEnableWindow(hwnd,103,IsDlgButtonChecked(hwnd,105) != BST_CHECKED);
          DialogEnableWindow(hwnd,104,TRUE);
          DialogEnableWindow(hwnd,105,TRUE);
          DialogEnableWindow(hwnd,106,TRUE);
          DialogEnableWindow(hwnd,107,bEnableLogicalSort);
          break;
        case 102:
          DialogEnableWindow(hwnd,103,FALSE);
          DialogEnableWindow(hwnd,104,FALSE);
          DialogEnableWindow(hwnd,105,FALSE);
          DialogEnableWindow(hwnd,106,FALSE);
          DialogEnableWindow(hwnd,107,FALSE);
          break;
        case 104:
          DialogEnableWindow(hwnd,103,IsDlgButtonChecked(hwnd,104) != BST_CHECKED);
          break;
      }
      return TRUE;
  }
  return FALSE;
}


//=============================================================================
//
//  EditSortDlg()
//
BOOL EditSortDlg(HWND hwnd,int *piSortFlags)
{

  INT_PTR iResult;

  iResult = ThemedDialogBoxParam(
              g_hInstance,
              MAKEINTRESOURCEW(IDD_SORT),
              hwnd,
              EditSortDlgProc,
              (LPARAM)piSortFlags);

  return (iResult == IDOK) ? TRUE : FALSE;

}


//=============================================================================
//
//  EditSortDlg()
//
void EditSetAccelWordNav(HWND hwnd,BOOL bAccelWordNav)
{
  bAccelWordNavigation = bAccelWordNav;

  if (bAccelWordNavigation) {
    SendMessage(hwnd, SCI_SETWORDCHARS, 0, (LPARAM)WordCharsAccelerated);
    SendMessage(hwnd, SCI_SETWHITESPACECHARS, 0,(LPARAM)WhiteSpaceCharsAccelerated);
    SendMessage(hwnd, SCI_SETPUNCTUATIONCHARS,0,(LPARAM)PunctuationCharsAccelerated);
  }
  else
    SendMessage(hwnd, SCI_SETCHARSDEFAULT, 0, 0);
}


//=============================================================================
//
//  EditGetBookmarkList()
//
void  EditGetBookmarkList(HWND hwnd, LPWSTR pszBookMarks, int cchLength)
{
  WCHAR tchLine[32];
  StringCchCopyW(pszBookMarks, cchLength, L"");
  int bitmask = (1 << MARKER_NP3_BOOKMARK);
  DocLn iLine = -1;
  do {
    iLine = (DocLn)SendMessage(hwnd, SCI_MARKERNEXT, iLine + 1, bitmask);
    if (iLine >= 0) {
      StringCchPrintfW(tchLine, COUNTOF(tchLine), L"%i;", iLine);
      StringCchCatW(pszBookMarks, cchLength, tchLine);
    }
  } while (iLine >= 0);

  StrTrimW(pszBookMarks, L";");
}


//=============================================================================
//
//  EditSetBookmarkList()
//
void  EditSetBookmarkList(HWND hwnd, LPCWSTR pszBookMarks)
{
  WCHAR lnNum[32];
  const WCHAR* p1 = pszBookMarks;
  if (!p1) return;

  const DocLn iLineMax = SciCall_GetLineCount() - 1;

  while (*p1) {
    const WCHAR* p2 = StrChr(p1, L';');
    if (!p2)
      p2 = StrEnd(p1);
    StringCchCopyNW(lnNum, COUNTOF(lnNum), p1, min((int)(p2 - p1), 16));
    int iLine = 0;
    if (swscanf_s(lnNum, L"%i", &iLine) == 1) {
      if (iLine <= iLineMax) {
        SendMessage(hwnd, SCI_MARKERADD, iLine, MARKER_NP3_BOOKMARK);
      }
    }
    p1 = (*p2) ? (p2 + 1) : p2;
  }
}


//=============================================================================
//
//  SetFileVars()
//
extern BOOL bNoEncodingTags;
extern int flagNoFileVariables;

void __fastcall SetFileVars(char* lpData, char* tch, LPFILEVARS lpfv)
{
  int i;
  BOOL bDisableFileVar = FALSE;

  if (!flagNoFileVariables) {

    if (FileVars_ParseInt(tch, "enable-local-variables", &i) && (!i))
      bDisableFileVar = TRUE;

    if (!bDisableFileVar) {

      if (FileVars_ParseInt(tch, "tab-width", &i)) {
        lpfv->iTabWidth = max(min(i, 256), 1);
        lpfv->mask |= FV_TABWIDTH;
      }

      if (FileVars_ParseInt(tch, "c-basic-indent", &i)) {
        lpfv->iIndentWidth = max(min(i, 256), 0);
        lpfv->mask |= FV_INDENTWIDTH;
      }

      if (FileVars_ParseInt(tch, "indent-tabs-mode", &i)) {
        lpfv->bTabsAsSpaces = (i) ? FALSE : TRUE;
        lpfv->mask |= FV_TABSASSPACES;
      }

      if (FileVars_ParseInt(tch, "c-tab-always-indent", &i)) {
        lpfv->bTabIndents = (i) ? TRUE : FALSE;
        lpfv->mask |= FV_TABINDENTS;
      }

      if (FileVars_ParseInt(tch, "truncate-lines", &i)) {
        lpfv->fWordWrap = (i) ? FALSE : TRUE;
        lpfv->mask |= FV_WORDWRAP;
      }

      if (FileVars_ParseInt(tch, "fill-column", &i)) {
        lpfv->iLongLinesLimit = max(min(i, 4096), 0);
        lpfv->mask |= FV_LONGLINESLIMIT;
      }
    }
  }

  if (!IsUTF8Signature(lpData) && !bNoEncodingTags && !bDisableFileVar) {

    if (FileVars_ParseStr(tch, "encoding", lpfv->tchEncoding, COUNTOF(lpfv->tchEncoding)))
      lpfv->mask |= FV_ENCODING;
    else if (FileVars_ParseStr(tch, "charset", lpfv->tchEncoding, COUNTOF(lpfv->tchEncoding)))
      lpfv->mask |= FV_ENCODING;
    else if (FileVars_ParseStr(tch, "coding", lpfv->tchEncoding, COUNTOF(lpfv->tchEncoding)))
      lpfv->mask |= FV_ENCODING;
  }

  if (!flagNoFileVariables && !bDisableFileVar) {
    if (FileVars_ParseStr(tch, "mode", lpfv->tchMode, COUNTOF(lpfv->tchMode)))
      lpfv->mask |= FV_MODE;
  }
}

//=============================================================================
//
//  FileVars_Init()
//

BOOL FileVars_Init(char *lpData, DWORD cbData, LPFILEVARS lpfv) {

  char tch[LARGE_BUFFER];

  ZeroMemory(lpfv,sizeof(FILEVARS));
  if ((flagNoFileVariables && bNoEncodingTags) || !lpData || !cbData)
    return TRUE;

  StringCchCopyNA(tch,COUNTOF(tch),lpData,min(cbData + 1,COUNTOF(tch)));
  SetFileVars(lpData, tch, lpfv);

  if (lpfv->mask == 0 && cbData > COUNTOF(tch)) {
    StringCchCopyNA(tch,COUNTOF(tch),lpData + cbData - COUNTOF(tch) + 1,COUNTOF(tch));
    SetFileVars(lpData, tch, lpfv);
  }

  if (lpfv->mask & FV_ENCODING)
    lpfv->iEncoding = Encoding_MatchA(lpfv->tchEncoding);

  return TRUE;
}


//=============================================================================
//
//  FileVars_Apply()
//
extern BOOL bTabsAsSpacesG;
extern BOOL bTabIndentsG;
extern int iTabWidthG;
extern int iIndentWidthG;
extern BOOL bWordWrap;
extern BOOL bWordWrapG;
extern int iWordWrapMode;
extern int iLongLinesLimit;
extern int iLongLinesLimitG;
extern int iWrapCol;

BOOL FileVars_Apply(HWND hwnd,LPFILEVARS lpfv) {

  if (lpfv->mask & FV_TABWIDTH)
    g_iTabWidth = lpfv->iTabWidth;
  else
    g_iTabWidth = iTabWidthG;
  SendMessage(hwnd,SCI_SETTABWIDTH,g_iTabWidth,0);

  if (lpfv->mask & FV_INDENTWIDTH)
    g_iIndentWidth = lpfv->iIndentWidth;
  else if (lpfv->mask & FV_TABWIDTH)
    g_iIndentWidth = 0;
  else
    g_iIndentWidth = iIndentWidthG;
  SendMessage(hwnd,SCI_SETINDENT,g_iIndentWidth,0);

  if (lpfv->mask & FV_TABSASSPACES)
    g_bTabsAsSpaces = lpfv->bTabsAsSpaces;
  else
    g_bTabsAsSpaces = bTabsAsSpacesG;
  SendMessage(hwnd,SCI_SETUSETABS,!g_bTabsAsSpaces,0);

  if (lpfv->mask & FV_TABINDENTS)
    g_bTabIndents = lpfv->bTabIndents;
  else
    g_bTabIndents = bTabIndentsG;
  SendMessage(g_hwndEdit,SCI_SETTABINDENTS,g_bTabIndents,0);

  if (lpfv->mask & FV_WORDWRAP)
    bWordWrap = lpfv->fWordWrap;
  else
    bWordWrap = bWordWrapG;

  if (!bWordWrap)
    SendMessage(g_hwndEdit,SCI_SETWRAPMODE,SC_WRAP_NONE,0);
  else
    SendMessage(g_hwndEdit,SCI_SETWRAPMODE,(iWordWrapMode == 0) ? SC_WRAP_WHITESPACE : SC_WRAP_CHAR,0);

  if (lpfv->mask & FV_LONGLINESLIMIT)
    iLongLinesLimit = lpfv->iLongLinesLimit;
  else
    iLongLinesLimit = iLongLinesLimitG;
  SendMessage(hwnd,SCI_SETEDGECOLUMN,iLongLinesLimit,0);

  iWrapCol = 0;

  return(TRUE);
}


//=============================================================================
//
//  FileVars_ParseInt()
//
BOOL FileVars_ParseInt(char* pszData,char* pszName,int* piValue) {

  char *pvStart = StrStrIA(pszData, pszName);
  while (pvStart) {
    char chPrev = (pvStart > pszData) ? *(pvStart-1) : 0;
    if (!IsCharAlphaNumericA(chPrev) && chPrev != '-' && chPrev != '_') {
      pvStart += lstrlenA(pszName);
      while (*pvStart == ' ')
        pvStart++;
      if (*pvStart == ':' || *pvStart == '=')
        break;
    }
    else
      pvStart += lstrlenA(pszName);

    pvStart = StrStrIA(pvStart, pszName); // next
  }

  if (pvStart) {

    while (*pvStart && StrChrIA(":=\"' \t",*pvStart))
      pvStart++;

    char tch[32] = { L'\0' };
    StringCchCopyNA(tch,COUNTOF(tch),pvStart,COUNTOF(tch));

    char* pvEnd = tch;
    while (*pvEnd && IsCharAlphaNumericA(*pvEnd))
      pvEnd++;
    *pvEnd = 0;
    StrTrimA(tch," \t:=\"'");

    int itok = sscanf_s(tch,"%i",piValue);
    if (itok == 1)
      return(TRUE);

    if (tch[0] == 't') {
      *piValue = 1;
      return(TRUE);
    }

    if (tch[0] == 'n' || tch[0] == 'f') {
      *piValue = 0;
      return(TRUE);
    }
  }
  return(FALSE);
}


//=============================================================================
//
//  FileVars_ParseStr()
//
BOOL FileVars_ParseStr(char* pszData,char* pszName,char* pszValue,int cchValue) {

  char *pvStart = StrStrIA(pszData, pszName);
  while (pvStart) {
    char chPrev = (pvStart > pszData) ? *(pvStart-1) : 0;
    if (!IsCharAlphaNumericA(chPrev) && chPrev != '-' && chPrev != '_') {
      pvStart += lstrlenA(pszName);
      while (*pvStart == ' ')
        pvStart++;
      if (*pvStart == ':' || *pvStart == '=')
        break;
    }
    else
      pvStart += lstrlenA(pszName);

    pvStart = StrStrIA(pvStart, pszName);  // next
  }

  if (pvStart) {

    BOOL bQuoted = FALSE;
    while (*pvStart && StrChrIA(":=\"' \t",*pvStart)) {
      if (*pvStart == '\'' || *pvStart == '"')
        bQuoted = TRUE;
      pvStart++;
    }

    char tch[32] = { L'\0' };
    StringCchCopyNA(tch,COUNTOF(tch),pvStart,COUNTOF(tch));

    char* pvEnd = tch;
    while (*pvEnd && (IsCharAlphaNumericA(*pvEnd) || StrChrIA("+-/_",*pvEnd) || (bQuoted && *pvEnd == ' ')))
      pvEnd++;
    *pvEnd = 0;
    StrTrimA(tch," \t:=\"'");

    StringCchCopyNA(pszValue,cchValue,tch,COUNTOF(tch));

    return(TRUE);
  }
  return(FALSE);
}


//=============================================================================
//
//  FileVars_IsUTF8()
//
BOOL FileVars_IsUTF8(LPFILEVARS lpfv) {
  if (lpfv->mask & FV_ENCODING) {
    if (StringCchCompareINA(lpfv->tchEncoding,COUNTOF(lpfv->tchEncoding),"utf-8",-1) == 0 ||
        StringCchCompareINA(lpfv->tchEncoding,COUNTOF(lpfv->tchEncoding),"utf8",-1) == 0)
      return(TRUE);
  }
  return(FALSE);
}


//=============================================================================
//
//  FileVars_IsNonUTF8()
//
BOOL FileVars_IsNonUTF8(LPFILEVARS lpfv) {
  if (lpfv->mask & FV_ENCODING) {
    if (StringCchLenA(lpfv->tchEncoding,COUNTOF(lpfv->tchEncoding)) &&
        StringCchCompareINA(lpfv->tchEncoding,COUNTOF(lpfv->tchEncoding),"utf-8",-1) != 0 &&
        StringCchCompareINA(lpfv->tchEncoding,COUNTOF(lpfv->tchEncoding),"utf8",-1) != 0)
      return(TRUE);
  }
  return(FALSE);
}


//=============================================================================
//
//  FileVars_IsValidEncoding()
//
BOOL FileVars_IsValidEncoding(LPFILEVARS lpfv) {
  CPINFO cpi;
  if (lpfv->mask & FV_ENCODING &&
      lpfv->iEncoding >= 0 &&
      lpfv->iEncoding < Encoding_CountOf()) {
    if ((Encoding_IsINTERNAL(lpfv->iEncoding)) ||
         IsValidCodePage(Encoding_GetCodePage(lpfv->iEncoding)) &&
         GetCPInfo(Encoding_GetCodePage(lpfv->iEncoding),&cpi)) {
      return(TRUE);
    }
  }
  return(FALSE);
}

//=============================================================================
//
//  FileVars_GetEncoding()
//
int FileVars_GetEncoding(LPFILEVARS lpfv) {
  if (lpfv->mask & FV_ENCODING)
    return(lpfv->iEncoding);
  else
    return(-1);
}


//==============================================================================
//
//  Folding Functions
//
//
#define FOLD_CHILDREN SCMOD_CTRL
#define FOLD_SIBLINGS SCMOD_SHIFT

BOOL __stdcall FoldToggleNode(DocLn ln, FOLD_ACTION action)
{
  const BOOL fExpanded = SciCall_GetFoldExpanded(ln);

  if ((action == FOLD && fExpanded) || (action == EXPAND && !fExpanded))
  {
    SciCall_ToggleFold(ln);
    return TRUE;
  }
  else if (action == SNIFF)
  {
    SciCall_ToggleFold(ln);
    return TRUE;
  }
  return FALSE;
}


void __stdcall EditFoldPerformAction(DocLn ln, int mode, FOLD_ACTION action)
{
  if (action == SNIFF) {
    action = SciCall_GetFoldExpanded(ln) ? FOLD : EXPAND;
  }
  if (mode & (FOLD_CHILDREN | FOLD_SIBLINGS))
  {
    // ln/lvNode: line and level of the source of this fold action
    DocLn lnNode = ln;
    int lvNode = SciCall_GetFoldLevel(lnNode) & SC_FOLDLEVELNUMBERMASK;
    DocLn lnTotal = SciCall_GetLineCount();

    // lvStop: the level over which we should not cross
    int lvStop = lvNode;

    if (mode & FOLD_SIBLINGS)
    {
      ln = SciCall_GetFoldParent(lnNode) + 1;  // -1 + 1 = 0 if no parent
      --lvStop;
    }

    for (; ln < lnTotal; ++ln)
    {
      int lv = SciCall_GetFoldLevel(ln);
      BOOL fHeader = lv & SC_FOLDLEVELHEADERFLAG;
      lv &= SC_FOLDLEVELNUMBERMASK;

      if (lv < lvStop || (lv == lvStop && fHeader && ln != lnNode))
        return;
      else if (fHeader && (lv == lvNode || (lv > lvNode && mode & FOLD_CHILDREN)))
        FoldToggleNode(ln, action);
    }
  }
  else {
    FoldToggleNode(ln, action);
  }
}


void EditFoldToggleAll(FOLD_ACTION action)
{
  static FOLD_ACTION sLastAction = EXPAND;

  BOOL fToggled = FALSE;

  DocLn lnTotal = SciCall_GetLineCount();

  if (action == SNIFF)
  {
    int cntFolded = 0;
    int cntExpanded = 0;
    for (int ln = 0; ln < lnTotal; ++ln)
    {
      if (SciCall_GetFoldLevel(ln) & SC_FOLDLEVELHEADERFLAG)
      {
        if (SciCall_GetFoldExpanded(ln))
          ++cntExpanded;
        else
          ++cntFolded;
      }
    }
    if (cntFolded == cntExpanded)
      action = (sLastAction == FOLD) ? EXPAND : FOLD;
    else
      action = (cntFolded < cntExpanded) ? FOLD : EXPAND;
  }

  for (int ln = 0; ln < lnTotal; ++ln)
  {
    if (SciCall_GetFoldLevel(ln) & SC_FOLDLEVELHEADERFLAG)
    {
      if (FoldToggleNode(ln, action)) { fToggled = TRUE; }
    }
  }
  if (fToggled) { SciCall_ScrollCaret(); }
}


void EditFoldClick(DocLn ln, int mode)
{
  static struct {
    DocLn ln;
    int mode;
    DWORD dwTickCount;
  } prev;

  BOOL fGotoFoldPoint = mode & FOLD_SIBLINGS;

  if (!(SciCall_GetFoldLevel(ln) & SC_FOLDLEVELHEADERFLAG))
  {
    // Not a fold point: need to look for a double-click
    if (prev.ln == ln && prev.mode == mode &&
      GetTickCount() - prev.dwTickCount <= GetDoubleClickTime())
    {
      prev.ln = (DocLn)-1;  // Prevent re-triggering on a triple-click

      ln = SciCall_GetFoldParent(ln);

      if (ln >= 0 && SciCall_GetFoldExpanded(ln))
        fGotoFoldPoint = TRUE;
      else
        return;
    }
    else
    {
      // Save the info needed to match this click with the next click
      prev.ln = ln;
      prev.mode = mode;
      prev.dwTickCount = GetTickCount();
      return;
    }
  }

  EditFoldPerformAction(ln, mode, SNIFF);

  if (fGotoFoldPoint) {
    EditJumpTo(g_hwndEdit, ln + 1, 0);
  }
}


void EditFoldAltArrow(FOLD_MOVE move, FOLD_ACTION action)
{
  if (g_bCodeFoldingAvailable && g_bShowCodeFolding)
  {
    DocLn ln = SciCall_LineFromPosition(SciCall_GetCurrentPos());

    // Jump to the next visible fold point
    if (move == DOWN)
    {
      DocLn lnTotal = SciCall_GetLineCount();
      for (ln = ln + 1; ln < lnTotal; ++ln)
      {
        if ((SciCall_GetFoldLevel(ln) & SC_FOLDLEVELHEADERFLAG) && SciCall_GetLineVisible(ln))
        {
          EditJumpTo(g_hwndEdit, ln + 1, 0);
          return;
        }
      }
    }
    else if (move == UP) // Jump to the previous visible fold point
    {
      for (ln = ln - 1; ln >= 0; --ln)
      {
        if ((SciCall_GetFoldLevel(ln) & SC_FOLDLEVELHEADERFLAG) && SciCall_GetLineVisible(ln))
        {
          EditJumpTo(g_hwndEdit, ln + 1, 0);
          return;
        }
      }
    }

    // Perform a fold/unfold operation
    if (SciCall_GetFoldLevel(ln) & SC_FOLDLEVELHEADERFLAG)
    {
      if (action != SNIFF) {
        FoldToggleNode(ln, action);
      }
    }
  }
}



//=============================================================================
//
//  SciInitThemes()
//
//WNDPROC pfnSciWndProc = NULL;
//
//FARPROC pfnOpenThemeData = NULL;
//FARPROC pfnCloseThemeData = NULL;
//FARPROC pfnDrawThemeBackground = NULL;
//FARPROC pfnGetThemeBackgroundContentRect = NULL;
//FARPROC pfnIsThemeActive = NULL;
//FARPROC pfnDrawThemeParentBackground = NULL;
//FARPROC pfnIsThemeBackgroundPartiallyTransparent = NULL;
//
//BOOL bThemesPresent = FALSE;
//extern BOOL bIsAppThemed;
//extern HMODULE hModUxTheme;
//
//void SciInitThemes(HWND hwnd)
//{
//  if (hModUxTheme) {
//
//    pfnOpenThemeData = GetProcAddress(hModUxTheme,"OpenThemeData");
//    pfnCloseThemeData = GetProcAddress(hModUxTheme,"CloseThemeData");
//    pfnDrawThemeBackground = GetProcAddress(hModUxTheme,"DrawThemeBackground");
//    pfnGetThemeBackgroundContentRect = GetProcAddress(hModUxTheme,"GetThemeBackgroundContentRect");
//    pfnIsThemeActive = GetProcAddress(hModUxTheme,"IsThemeActive");
//    pfnDrawThemeParentBackground = GetProcAddress(hModUxTheme,"DrawThemeParentBackground");
//    pfnIsThemeBackgroundPartiallyTransparent = GetProcAddress(hModUxTheme,"IsThemeBackgroundPartiallyTransparent");
//
//    pfnSciWndProc = (WNDPROC)SetWindowLongPtrW(hwnd,GWLP_WNDPROC,(LONG_PTR)&SciThemedWndProc);
//    bThemesPresent = TRUE;
//  }
//}
//
//
////=============================================================================
////
////  SciThemedWndProc()
////
//LRESULT CALLBACK SciThemedWndProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
//{
//  static RECT rcContent;
//
//  if (umsg == WM_NCCALCSIZE) {
//    if (wParam) {
//      LRESULT lresult = CallWindowProcW(pfnSciWndProc,hwnd,WM_NCCALCSIZE,wParam,lParam);
//      NCCALCSIZE_PARAMS *csp = (NCCALCSIZE_PARAMS*)lParam;
//
//      if (bThemesPresent && bIsAppThemed) {
//        HANDLE hTheme = (HANDLE)pfnOpenThemeData(hwnd,L"edit");
//        if(hTheme) {
//          BOOL bSuccess = FALSE;
//          RECT rcClient;
//
//          if(pfnGetThemeBackgroundContentRect(
//              hTheme,NULL,/*EP_EDITTEXT*/1,/*ETS_NORMAL*/1,&csp->rgrc[0],&rcClient) == S_OK) {
//            InflateRect(&rcClient,-1,-1);
//
//            rcContent.left = rcClient.left-csp->rgrc[0].left;
//            rcContent.top = rcClient.top-csp->rgrc[0].top;
//            rcContent.right = csp->rgrc[0].right-rcClient.right;
//            rcContent.bottom = csp->rgrc[0].bottom-rcClient.bottom;
//
//            CopyRect(&csp->rgrc[0],&rcClient);
//            bSuccess = TRUE;
//          }
//          pfnCloseThemeData(hTheme);
//
//          if (bSuccess)
//            return WVR_REDRAW;
//        }
//      }
//      return lresult;
//    }
//  }
//
//  else if (umsg == WM_NCPAINT) {
//    LRESULT lresult = CallWindowProcW(pfnSciWndProc,hwnd,WM_NCPAINT,wParam,lParam);
//    if(bThemesPresent && bIsAppThemed) {
//
//      HANDLE hTheme = (HANDLE)pfnOpenThemeData(hwnd,L"edit");
//      if(hTheme) {
//        RECT rcBorder;
//        RECT rcClient;
//        int nState;
//
//        HDC hdc = GetWindowDC(hwnd);
//
//        GetWindowRect(hwnd,&rcBorder);
//        OffsetRect(&rcBorder,-rcBorder.left,-rcBorder.top);
//
//        CopyRect(&rcClient,&rcBorder);
//        rcClient.left += rcContent.left;
//        rcClient.top += rcContent.top;
//        rcClient.right -= rcContent.right;
//        rcClient.bottom -= rcContent.bottom;
//
//        ExcludeClipRect(hdc,rcClient.left,rcClient.top,rcClient.right,rcClient.bottom);
//
//        if(pfnIsThemeBackgroundPartiallyTransparent(hTheme,/*EP_EDITTEXT*/1,/*ETS_NORMAL*/1))
//          pfnDrawThemeParentBackground(hwnd,hdc,&rcBorder);
//
//        /*
//        ETS_NORMAL = 1
//        ETS_HOT = 2
//        ETS_SELECTED = 3
//        ETS_DISABLED = 4
//        ETS_FOCUSED = 5
//        ETS_READONLY = 6
//        ETS_ASSIST = 7
//        */
//
//        if(!IsWindowEnabled(hwnd))
//          nState = /*ETS_DISABLED*/4;
//        else if (GetFocus() == hwnd)
//          nState = /*ETS_FOCUSED*/5;
//        else if(SendMessage(hwnd,SCI_GETREADONLY,0,0))
//          nState = /*ETS_READONLY*/6;
//        else
//          nState = /*ETS_NORMAL*/1;
//
//        pfnDrawThemeBackground(hTheme,hdc,/*EP_EDITTEXT*/1,nState,&rcBorder,NULL);
//        pfnCloseThemeData(hTheme);
//
//        ReleaseDC(hwnd,hdc);
//        return 0;
//      }
//    }
//    return lresult;
//  }
//
//  return CallWindowProcW(pfnSciWndProc,hwnd,umsg,wParam,lParam);
//}



///   End of Edit.c   \\\
