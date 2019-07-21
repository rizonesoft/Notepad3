/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Edit.c                                                                      *
*   Text File Editing Helper Stuff                                            *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2019   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

#include "Helpers.h"

#include <shlwapi.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <shellapi.h>

#include "Styles.h"
#include "Dialogs.h"
#include "resource.h"
#include "../crypto/crypto.h"
#include "../uthash/utarray.h"
#include "../uthash/utlist.h"
//#include "../uthash/utstring.h"
#include "../tinyexpr/tinyexpr.h"
#include "Encoding.h"
#include "MuiLanguage.h"
#include "Notepad3.h"
#include "Config/Config.h"

#include "SciCall.h"
#include "SciLexer.h"

#include "Edit.h"


#ifndef LCMAP_TITLECASE
#define LCMAP_TITLECASE  0x00000300  // Title Case Letters bit mask
#endif

// find free bits in scintilla.h SCFIND_ defines
#define SCFIND_NP3_REGEX (SCFIND_REGEXP | SCFIND_POSIX)

static EDITFINDREPLACE s_efrSave;
static bool s_bSwitchedFindReplace = false;

static int s_xFindReplaceDlgSave;
static int s_yFindReplaceDlgSave;

static char DelimChars[ANSI_CHAR_BUFFER] = { '\0' };
static char DelimCharsAccel[ANSI_CHAR_BUFFER] = { '\0' };
static char WordCharsDefault[ANSI_CHAR_BUFFER] = { '\0' };
static char WhiteSpaceCharsDefault[ANSI_CHAR_BUFFER] = { '\0' };
static char PunctuationCharsDefault[ANSI_CHAR_BUFFER] = { '\0' };
static char WordCharsAccelerated[ANSI_CHAR_BUFFER] = { '\0' };
static char WhiteSpaceCharsAccelerated[ANSI_CHAR_BUFFER] = { '\0' };
static char PunctuationCharsAccelerated[1] = { '\0' }; // empty!

// Default Codepage and Character Set
#define W_AUTOC_WORD_ANSI1252 L"#$%&@0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyzÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýþÿ"
static char AutoCompleteWordCharSet[ANSI_CHAR_BUFFER] = { L'\0' };

//static WCHAR W_DelimChars[ANSI_CAHR_BUFFER] = { L'\0' };
//static WCHAR W_DelimCharsAccel[ANSI_CAHR_BUFFER] = { L'\0' };
//static WCHAR W_WhiteSpaceCharsDefault[ANSI_CAHR_BUFFER] = { L'\0' };
//static WCHAR W_WhiteSpaceCharsAccelerated[ANSI_CAHR_BUFFER] = { L'\0' };


// Is the character a white space char?
#define IsWhiteSpace(ch)  StrChrA(WhiteSpaceCharsDefault, (ch))
#define IsAccelWhiteSpace(ch)  StrChrA(WhiteSpaceCharsAccelerated, (ch))


enum AlignMask {
  ALIGN_LEFT = 0,
  ALIGN_RIGHT = 1,
  ALIGN_CENTER = 2,
  ALIGN_JUSTIFY = 3,
  ALIGN_JUSTIFY_EX = 4
};

enum SortOrderMask {
  SORT_ASCENDING   = 0x001,
  SORT_DESCENDING  = 0x002,
  SORT_SHUFFLE     = 0x004,
  SORT_MERGEDUP    = 0x008,
  SORT_UNIQDUP     = 0x010,
  SORT_UNIQUNIQ    = 0x020,
  SORT_REMZEROLEN  = 0x040,
  SORT_REMWSPACELN = 0x080,
  SORT_NOCASE      = 0x100,
  SORT_LOGICAL     = 0x200,
  SORT_COLUMN      = 0x400 
};


//=============================================================================
//
//  Delay Message Queue Handling  (TODO: MultiThreading)
//

static CmdMessageQueue_t* MessageQueue = NULL;

// ----------------------------------------------------------------------------

static int msgcmp(void* mqc1, void* mqc2)
{
  CmdMessageQueue_t* const pMQC1 = (CmdMessageQueue_t*)mqc1;
  CmdMessageQueue_t* const pMQC2 = (CmdMessageQueue_t*)mqc2;

  if ((pMQC1->hwnd == pMQC2->hwnd)
      && (pMQC1->cmd == pMQC2->cmd)
      && (pMQC1->wparam == pMQC2->wparam)
      && (pMQC1->lparam == pMQC2->lparam)) {
    return 0;
  }
  return 1;
}
// ----------------------------------------------------------------------------

#define _MQ_ms(T) ((T) / USER_TIMER_MINIMUM)

static void  _MQ_AppendCmd(CmdMessageQueue_t* const pMsgQCmd, int cycles)
{
  CmdMessageQueue_t* pmqc = NULL;
  DL_SEARCH(MessageQueue, pmqc, pMsgQCmd, msgcmp);

  if (!pmqc) { // NOT found
    pmqc = AllocMem(sizeof(CmdMessageQueue_t), HEAP_ZERO_MEMORY);
    pmqc->hwnd = pMsgQCmd->hwnd;
    pmqc->cmd = pMsgQCmd->cmd;
    pmqc->wparam = pMsgQCmd->wparam;
    pmqc->lparam = pMsgQCmd->lparam;
    pmqc->delay = cycles;
    DL_APPEND(MessageQueue, pmqc);
  }

  if (cycles < 2) {
    pmqc->delay = -1; // execute now (do not use PostMessage() here)
    SendMessage(pMsgQCmd->hwnd, pMsgQCmd->cmd, pMsgQCmd->wparam, pMsgQCmd->lparam);
  }
  else {
    pmqc->delay = (pmqc->delay + cycles) / 2; // increase delay
  }
}
// ----------------------------------------------------------------------------

/* unused yet
static void  _MQ_RemoveCmd(CmdMessageQueue_t* const pMsgQCmd)
{
  CmdMessageQueue_t* pmqc = NULL;

  DL_FOREACH(MessageQueue, pmqc)
  {
    if ((pMsgQCmd->hwnd == pmqc->hwnd)
      && (pMsgQCmd->cmd == pmqc->cmd)
      && (pMsgQCmd->wparam == pmqc->wparam))
    {
      pmqc->delay = -1;
    }
  }
}
// ----------------------------------------------------------------------------
*/

// ----------------------------------------------------------------------------
//
// called by Timer(IDT_TIMER_MRKALL)
//
static void CALLBACK MQ_ExecuteNext(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
  UNUSED(hwnd);    // must be main wnd
  UNUSED(uMsg);    // must be WM_TIMER
  UNUSED(idEvent); // must be IDT_TIMER_MRKALL
  UNUSED(dwTime);  // This is the value returned by the GetTickCount function

  CmdMessageQueue_t* pmqc;

  DL_FOREACH(MessageQueue, pmqc)
  {
    if (pmqc->delay == 0) {
      pmqc->delay = -1;
      SendMessage(pmqc->hwnd, pmqc->cmd, pmqc->wparam, pmqc->lparam);
    }
    else if (pmqc->delay >= 0) {
      pmqc->delay -= 1;
    }
  }
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

  char whitesp[ANSI_CHAR_BUFFER*2] = { '\0' };
  if (StrIsNotEmpty(Settings2.ExtendedWhiteSpaceChars)) {
    WideCharToMultiByte(Encoding_SciCP, 0, Settings2.ExtendedWhiteSpaceChars, -1, whitesp, COUNTOF(whitesp), NULL, NULL);
  }

  // 3rd set accelerated arrays

  // init with default
  StringCchCopyA(WhiteSpaceCharsAccelerated, COUNTOF(WhiteSpaceCharsAccelerated), WhiteSpaceCharsDefault);

  // add only 7-bit-ASCII chars to accelerated whitespace list
  for (size_t i = 0; i < StringCchLenA(whitesp, ANSI_CHAR_BUFFER); i++) {
    if (whitesp[i] & 0x7F) {
      if (!StrChrA(WhiteSpaceCharsAccelerated, whitesp[i])) {
        StringCchCatNA(WhiteSpaceCharsAccelerated, COUNTOF(WhiteSpaceCharsAccelerated), &(whitesp[i]), 1);
      }
    }
  }

  // construct word char array
  StringCchCopyA(WordCharsAccelerated, COUNTOF(WordCharsAccelerated), WordCharsDefault); // init
  // add punctuation chars not listed in white-space array
  for (size_t i = 0; i < StringCchLenA(PunctuationCharsDefault, ANSI_CHAR_BUFFER); i++) {
    if (!StrChrA(WhiteSpaceCharsAccelerated, PunctuationCharsDefault[i])) {
      StringCchCatNA(WordCharsAccelerated, COUNTOF(WordCharsAccelerated), &(PunctuationCharsDefault[i]), 1);
    }
  }

  // construct accelerated delimiters
  StringCchCopyA(DelimCharsAccel, COUNTOF(DelimCharsAccel), WhiteSpaceCharsDefault);
  StringCchCatA(DelimCharsAccel, COUNTOF(DelimCharsAccel), lineEnds);

  if (StrIsNotEmpty(Settings2.AutoCompleteWordCharSet))
  {
    WideCharToMultiByte(Encoding_SciCP, 0, Settings2.AutoCompleteWordCharSet, -1, AutoCompleteWordCharSet, COUNTOF(AutoCompleteWordCharSet), NULL, NULL);
    Globals.bUseLimitedAutoCCharSet = true;
  } else {
    WideCharToMultiByte(Encoding_SciCP, 0, W_AUTOC_WORD_ANSI1252, -1, AutoCompleteWordCharSet, COUNTOF(AutoCompleteWordCharSet), NULL, NULL);
    Globals.bUseLimitedAutoCCharSet = false;
  }

  // construct wide char arrays
  //MultiByteToWideChar(Encoding_SciCP, 0, DelimChars, -1, W_DelimChars, COUNTOF(W_DelimChars));
  //MultiByteToWideChar(Encoding_SciCP, 0, DelimCharsAccel, -1, W_DelimCharsAccel, COUNTOF(W_DelimCharsAccel));
  //MultiByteToWideChar(Encoding_SciCP, 0, WhiteSpaceCharsDefault, -1, W_WhiteSpaceCharsDefault, COUNTOF(W_WhiteSpaceCharsDefault));
  //MultiByteToWideChar(Encoding_SciCP, 0, WhiteSpaceCharsAccelerated, -1, W_WhiteSpaceCharsAccelerated, COUNTOF(W_WhiteSpaceCharsAccelerated));
}


//=============================================================================
//
//  EditSetNewText()
//
extern bool bFreezeAppTitle;

void EditSetNewText(HWND hwnd, const char* lpstrText, DocPos lenText, bool bClearUndoHistory)
{
  bFreezeAppTitle = true;

  // clear markers, flags and positions
  if (bClearUndoHistory) { UndoRedoRecordingStop(); }
  if (FocusedView.HideNonMatchedLines) { EditToggleView(hwnd); }
  _IGNORE_NOTIFY_CHANGE_;
  SciCall_Cancel();
  if (SciCall_GetReadOnly()) { SciCall_SetReadOnly(false); }
  SciCall_MarkerDeleteAll(MARKER_NP3_BOOKMARK);
  EditClearAllOccurrenceMarkers(hwnd);
  SciCall_SetScrollWidth(1);
  SciCall_SetXOffset(0);
  _OBSERVE_NOTIFY_CHANGE_;

  FileVars_Apply(&Globals.fvCurFile);

  // set new text
  if (lenText > 0) {
    DocPos const saveTargetBeg = SciCall_GetTargetStart();
    DocPos const saveTargetEnd = SciCall_GetTargetEnd();
    SciCall_TargetWholeDocument();
    SciCall_ReplaceTarget(lenText, lpstrText);
    SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore
  }
  else {
    SciCall_ClearAll();
  }
  SciCall_GotoPos(0);
  SciCall_ChooseCaretX();

  if (bClearUndoHistory) {
    SciCall_SetSavePoint();
    UndoRedoRecordingStart();
  }

  bFreezeAppTitle = false;
}


//=============================================================================
//
//  EditConvertText()
//
bool EditConvertText(HWND hwnd, cpi_enc_t encSource, cpi_enc_t encDest, bool bSetSavePoint)
{
  if ((encSource == encDest) || (Encoding_SciCP == encDest)) {
    if (bSetSavePoint) {
      SciCall_SetSavePoint();
    }
    return true; 
  }

  if (!(Encoding_IsValid(encSource) && Encoding_IsValid(encDest))) { return false; }

  DocPos const length = SciCall_GetTextLength();

  if (length <= 0)
  {
    EditSetNewText(hwnd, "", 0, bSetSavePoint);
  }
  else {

    const DocPos chBufSize = length * 5 + 2;
    char* pchText = AllocMem(chBufSize,HEAP_ZERO_MEMORY);

    struct Sci_TextRange tr = { { 0, -1 }, NULL };
    tr.lpstrText = pchText;
    DocPos const rlength = SciCall_GetTextRange(&tr);

    const DocPos wchBufSize = rlength * 3 + 2;
    WCHAR* pwchText = AllocMem(wchBufSize, HEAP_ZERO_MEMORY);

    // MultiBytes(Sci) -> WideChar(destination) -> Sci(MultiByte)
    const UINT cpDst = Encoding_GetCodePage(encDest);
    
    // get text as wide char
    int cbwText = MultiByteToWideChar(Encoding_SciCP,0, pchText, (MBWC_DocPos_Cast)length, pwchText, (MBWC_DocPos_Cast)wchBufSize);
    // convert wide char to destination multibyte
    int cbText = WideCharToMultiByte(cpDst, 0, pwchText, cbwText, pchText, (MBWC_DocPos_Cast)chBufSize, NULL, NULL);
    // re-code to wide char
    cbwText = MultiByteToWideChar(cpDst, 0, pchText, cbText, pwchText, (MBWC_DocPos_Cast)wchBufSize);
    // convert to Scintilla format
    cbText = WideCharToMultiByte(Encoding_SciCP, 0, pwchText, cbwText, pchText, (MBWC_DocPos_Cast)chBufSize, NULL, NULL);

    pchText[cbText] = '\0';
    pchText[cbText+1] = '\0';

    FreeMem(pwchText);

    EditSetNewText(hwnd, pchText, cbText, bSetSavePoint);

    FreeMem(pchText);
  }
  return true;
}


//=============================================================================
//
//  EditSetNewEncoding()
//
bool EditSetNewEncoding(HWND hwnd, cpi_enc_t iNewEncoding, bool bNoUI)
{
  cpi_enc_t iCurrentEncoding = Encoding_Current(CPI_GET);

  if (iCurrentEncoding != iNewEncoding) {

    // conversion between arbitrary encodings may lead to unexpected results
    //bool bOneEncodingIsANSI = (Encoding_IsANSI(iCurrentEncoding) || Encoding_IsANSI(iNewEncoding));
    //bool bBothEncodingsAreANSI = (Encoding_IsANSI(iCurrentEncoding) && Encoding_IsANSI(iNewEncoding));
    //if (!bOneEncodingIsANSI || bBothEncodingsAreANSI) {
      // ~ return true; // this would imply a successful conversion - it is not !
      //return false; // commented out ? : allow conversion between arbitrary encodings
    //}

    // suppress recoding message for certain encodings
    if ((Encoding_GetCodePage(iCurrentEncoding) == 936) && (Encoding_GetCodePage(iNewEncoding) == 54936)) {
      bNoUI = true;
    }
    // and vice versa ???

    if (SciCall_GetTextLength() <= 0) 
    {
      bool const doNewEncoding = (Sci_HaveUndoRedoHistory() && !bNoUI) ?
        (InfoBoxLng(MB_YESNO, L"MsgConv2", IDS_MUI_ASK_ENCODING2) == IDYES) : true;

      if (doNewEncoding) {
        return EditConvertText(hwnd, iCurrentEncoding, iNewEncoding, true);
      }
    }
    else {

      bool const doNewEncoding = (!bNoUI) ? (InfoBoxLng(MB_YESNO, L"MsgConv1", IDS_MUI_ASK_ENCODING) == IDYES) : true;

      if (doNewEncoding) {
        return EditConvertText(hwnd, iCurrentEncoding, iNewEncoding, true);
      }
    }
  }
  return false;
}


//=============================================================================
//
//  EditIsRecodingNeeded()
//
bool EditIsRecodingNeeded(WCHAR* pszText, int cchLen)
{
  if ((pszText == NULL) || (cchLen < 1))
    return false;

  UINT codepage = Encoding_GetCodePage(Encoding_Current(CPI_GET));

  if ((codepage == CP_UTF7) || (codepage == CP_UTF8))
    return false;

  DWORD dwFlags = WC_NO_BEST_FIT_CHARS | WC_COMPOSITECHECK | WC_DEFAULTCHAR;
  bool useNullParams = Encoding_IsMBCS(Encoding_Current(CPI_GET)) ? true : false;

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

  bool bSuccess = ((cch >= cchLen) && (cch != 0xFFFD)) ? true : false;
  
  return (!bSuccess || bDefaultCharsUsed);
}


//=============================================================================
//
//  EditGetClipboardText()
//
char* EditGetClipboardText(HWND hwnd, bool bCheckEncoding, int* pLineCount, int* pLenLastLn) 
{
  if (!IsClipboardFormatAvailable(CF_UNICODETEXT) || !OpenClipboard(GetParent(hwnd))) {
    char* pEmpty = AllocMem(1, HEAP_ZERO_MEMORY);
    return pEmpty;
  }

  // get clipboard
  HANDLE hmem = GetClipboardData(CF_UNICODETEXT);
  WCHAR* pwch = GlobalLock(hmem);
  int const wlen = (int)StringCchLenW(pwch,0);

  if (bCheckEncoding && EditIsRecodingNeeded(pwch,wlen)) 
  {
    const DocPos iPos = SciCall_GetCurrentPos();
    const DocPos iAnchor = SciCall_GetAnchor();

    // switch encoding to universal UTF-8 codepage
    SendWMCommand(Globals.hwndMain, IDM_ENCODING_UTF8);

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
  char* pmch = NULL;
  int mlen = 0;
  if (wlen > 0) {
    mlen = WideCharToMultiByte(Encoding_SciCP, 0, pwch, wlen, NULL, 0, NULL, NULL);
    pmch = (char*)AllocMem(mlen + 1, HEAP_ZERO_MEMORY);
    if (pmch && mlen != 0) {
      int const cnt = WideCharToMultiByte(Encoding_SciCP, 0, pwch, wlen, pmch, mlen + 1, NULL, NULL);
      if (cnt == 0)
        return pmch;
    }
    else
      return pmch;
  }
  else {
    pmch = AllocMem(1, HEAP_ZERO_MEMORY);
    return pmch;
  }
  int lineCount = 0;
  int lenLastLine = 0;

  if ((bool)SendMessage(hwnd,SCI_GETPASTECONVERTENDINGS,0,0)) 
  {
    char* ptmp = (char*)AllocMem((mlen+1)*2, HEAP_ZERO_MEMORY);
    if (ptmp) {
      char *s = pmch;
      char *d = ptmp;
      int eolmode = SciCall_GetEOLMode();
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

      FreeMem(pmch);
      pmch = AllocMem(mlen2 + 1, HEAP_ZERO_MEMORY);
      if (pmch) {
        StringCchCopyA(pmch, mlen2 + 1, ptmp);
        FreeMem(ptmp);
      }
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

  return pmch;
}


//=============================================================================
//
//  EditSetClipboardText()
//
bool EditSetClipboardText(HWND hwnd, const char* pszText, size_t cchText)
{
  if (!IsClipboardFormatAvailable(CF_UNICODETEXT)) {
    SciCall_CopyText((DocPos)cchText, pszText);
    return true;
  }

  WCHAR* pszTextW = NULL;
  int const cchTextW = MultiByteToWideChar(Encoding_SciCP, 0, pszText, (MBWC_DocPos_Cast)cchText, NULL, 0);
  if (cchTextW > 1) {
    pszTextW = AllocMem((cchTextW + 1) * sizeof(WCHAR), HEAP_ZERO_MEMORY);
    if (pszTextW) {
      MultiByteToWideChar(Encoding_SciCP, 0, pszText, (MBWC_DocPos_Cast)cchText, pszTextW, cchTextW);
      pszTextW[cchTextW] = L'\0';
    }
  }

  if (pszTextW) {
    SetClipboardTextW(GetParent(hwnd), pszTextW, cchTextW);
    FreeMem(pszTextW);
    return true;
  }
  return false;
}


//=============================================================================
//
//  EditClearClipboard()
//
bool EditClearClipboard(HWND hwnd)
{
  if (!IsClipboardFormatAvailable(CF_UNICODETEXT)) {
    SciCall_CopyText(0, "");
    return true;
  }
  if (!OpenClipboard(GetParent(hwnd))) {
    return false;
  }
  EmptyClipboard();
  CloseClipboard();
  return true;
}


//=============================================================================
//
//  EditSwapClipboard()
//
bool EditSwapClipboard(HWND hwnd, bool bSkipUnicodeCheck)
{
  int lineCount = 0;
  int lenLastLine = 0;
  char* const pClip = EditGetClipboardText(hwnd, !bSkipUnicodeCheck, &lineCount, &lenLastLine);
  if (!pClip) {
    return false; // recoding canceled
  }
  DocPos const clipLen = (DocPos)StringCchLenA(pClip,0);

  DocPos const iCurPos = SciCall_GetCurrentPos();
  DocPos const iAnchorPos = SciCall_GetAnchor();
  bool const bIsMultiSel = Sci_IsMultiOrRectangleSelection();

  char* pszText = NULL;
  size_t const size = SciCall_GetSelText(NULL);
  if (size > 0) {
    pszText = AllocMem(size, HEAP_ZERO_MEMORY);
    SciCall_GetSelText(pszText);
    SciCall_Paste();  //~SciCall_ReplaceSel(pClip);
    EditSetClipboardText(hwnd, pszText, (size - 1));
  }
  else {
    SciCall_Paste();  //~SciCall_ReplaceSel(pClip);
    SciCall_Clear();
  }
  if (pszText) { FreeMem(pszText); }

  if (!bIsMultiSel) {
    // TODO(rkotten): check for Rectangular Clipboard and skip selection restore
    if (iCurPos < iAnchorPos)
      EditSetSelectionEx(hwnd, iCurPos + clipLen, iCurPos, -1, -1);
    else
      EditSetSelectionEx(hwnd, iAnchorPos, iAnchorPos + clipLen, -1, -1);
  }
  else {
    // TODO(rkotten): restore rectangular selection in case of swap clipboard 
  }

  FreeMem(pClip);
  return true;
}


//=============================================================================
//
//  EditCopyAppend()
//
bool EditCopyAppend(HWND hwnd, bool bAppend)
{
  bool res = false;

  DocPos const iSelStart = SciCall_GetSelectionStart();
  DocPos const iSelEnd = SciCall_GetSelectionEnd();

  char* pszText = NULL;
  DocPos length = 0;

  if (iSelStart != iSelEnd) {
    if (Sci_IsMultiOrRectangleSelection()) {
      InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECT);
      return res;
    }
    length = (iSelEnd - iSelStart);
    pszText = SciCall_GetRangePointer(iSelStart, length);
  }
  else {
    length = SciCall_GetTextLength();
    pszText = SciCall_GetRangePointer(0, length);
  }
  if (length <= 0) {
    res = true;  // nothing to copy or append
    return res;
  }

  WCHAR* pszTextW = NULL;
  int cchTextW = 0;
  if (pszText && *pszText) {
    cchTextW = MultiByteToWideChar(Encoding_SciCP, 0, pszText, (MBWC_DocPos_Cast)length, NULL, 0);
    if (cchTextW > 0) {
      pszTextW = AllocMem((cchTextW + 1) * sizeof(WCHAR), HEAP_ZERO_MEMORY);
      if (pszTextW) {
        MultiByteToWideChar(Encoding_SciCP, 0, pszText, (MBWC_DocPos_Cast)length, pszTextW, cchTextW);
        pszTextW[cchTextW] = L'\0';
      }
    }
  }

  if (!bAppend) {
    res = SetClipboardTextW(GetParent(hwnd), pszTextW, cchTextW);
    FreeMem(pszTextW);
    return res;
  }

  // --- Append to Clipboard ---

  if (!OpenClipboard(GetParent(hwnd))) {
    FreeMem(pszTextW);
    return res;
  }

  HANDLE const hOld   = GetClipboardData(CF_UNICODETEXT);
  const WCHAR* pszOld = GlobalLock(hOld);

  int const _eol_mode = SciCall_GetEOLMode();
  const WCHAR *pszSep = ((_eol_mode == SC_EOL_CRLF) ? L"\r\n" : ((_eol_mode == SC_EOL_CR) ? L"\r" : L"\n"));

  size_t cchNewText = cchTextW;
  if (pszOld && *pszOld) {
    cchNewText += StringCchLen(pszOld, 0) + StringCchLen(pszSep, 0);
  }

  // Copy Clip & add line break
  WCHAR* pszNewTextW = AllocMem((cchNewText + 1) * sizeof(WCHAR), HEAP_ZERO_MEMORY);
  if (pszOld && *pszOld && pszNewTextW) {
    StringCchCopy(pszNewTextW, cchNewText + 1, pszOld);
    StringCchCat(pszNewTextW, cchNewText + 1, pszSep);
  }
  GlobalUnlock(hOld);
  CloseClipboard();

  // Add New
  if (pszTextW && *pszTextW && pszNewTextW) {
    StringCchCat(pszNewTextW, cchNewText+1, pszTextW);
    res = SetClipboardTextW(GetParent(hwnd), pszNewTextW, cchNewText);
  }

  FreeMem(pszTextW);
  FreeMem(pszNewTextW);
  return res;
}


//=============================================================================
//
// EditDetectEOLMode() - moved here to handle Unicode files correctly
// by zufuliu (https://github.com/zufuliu/notepad2)
//
void EditDetectEOLMode(LPCSTR lpData, DWORD cbData, EditFileIOStatus* status)
{
  int iEOLMode = Settings.DefaultEOLMode;

  if (cbData == 0) {
    status->iEOLMode = iEOLMode;
    return;
  }

  DocLn linesCount[3] = { 0, 0, 0 };


  // tools/GenerateTable.py
  static const UINT8 eol_table[16] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 0, 0, // 00 - 0F
  };

  const UINT8* ptr = (const UINT8*)lpData;
  const UINT8* const end = ptr + cbData;
  do {
    // skip to line end
    UINT ch;
    UINT type = 0;
    while (ptr < end && ((ch = *ptr++) > '\r' || (type = eol_table[ch]) == 0)) { /* nop */ }
    switch (type) {
    case 1: //'\n'
      ++linesCount[SC_EOL_LF];
      break;
    case 2: //'\r'
      if (*ptr == '\n') {
        ++ptr;
        ++linesCount[SC_EOL_CRLF];
      }
      else {
        ++linesCount[SC_EOL_CR];
      }
      break;
    }
  } while (ptr < end);

  
  DocLn const linesMax = max_ln(max_ln(linesCount[0], linesCount[1]), linesCount[2]);

  if (linesMax != linesCount[iEOLMode]) 
  {
    if (linesMax == linesCount[SC_EOL_CRLF]) {
      iEOLMode = SC_EOL_CRLF;
    }
    else if (linesMax == linesCount[SC_EOL_CR]) {
      iEOLMode = SC_EOL_CR;
    }
    else {
      iEOLMode = SC_EOL_LF;
    }
  }

  status->iEOLMode = iEOLMode;
  status->bInconsistentEOLs = ((!!linesCount[0]) + (!!linesCount[1]) + (!!linesCount[2])) > 1;
  status->eolCount[SC_EOL_CRLF] = linesCount[SC_EOL_CRLF];
  status->eolCount[SC_EOL_CR] = linesCount[SC_EOL_CR];
  status->eolCount[SC_EOL_LF] = linesCount[SC_EOL_LF];
}


//=============================================================================
//
// EditIndentationCount() - check indentation consistency
//
void EditIndentationStatistic(HWND hwnd, EditFileIOStatus* status)
{
  UNUSED(hwnd);

  int const tabWidth = Globals.fvCurFile.iTabWidth;
  int const indentWidth = Globals.fvCurFile.iIndentWidth;
  DocLn const lineCount = SciCall_GetLineCount();

  status->indentCount[I_TAB_LN] = 0;
  status->indentCount[I_SPC_LN] = 0;
  status->indentCount[I_MIX_LN] = 0;
  status->indentCount[I_TAB_MOD_X] = 0;
  status->indentCount[I_SPC_MOD_X] = 0;

  for (DocLn line = 0; line < lineCount; ++line) 
  {
    DocPos const lineStartPos = SciCall_PositionFromLine(line);
    DocPos const lineIndentBeg = SciCall_GetLineIndentPosition(line);
    DocPos const lineIndentDepth = SciCall_GetLineIndentation(line);

    int tabCount = 0;
    int blankCount = 0;
    int subSpcCnt = 0;
    for (DocPos pos = lineStartPos; pos < lineIndentBeg; ++pos) {
      char const ch = SciCall_GetCharAt(pos);
      switch (ch) {
      case 0x09: // tab
        ++tabCount;
        break;
      case 0x20: // space
        ++subSpcCnt;
        if (subSpcCnt >= indentWidth) {
          ++blankCount;
          subSpcCnt = 0;
        }
        break;
      default:
        break;
      }
    }

    // analyze
    if (tabCount || blankCount) {
      if (lineIndentDepth % tabWidth) {
        ++(status->indentCount[I_TAB_MOD_X]);
      }
      if (lineIndentDepth % indentWidth) {
        ++(status->indentCount[I_SPC_MOD_X]);
      }
    }
    if (tabCount && blankCount) {
      ++(status->indentCount[I_MIX_LN]);
    }
    else if (tabCount) {
      ++(status->indentCount[I_TAB_LN]);
    }
    else if (blankCount) {
      ++(status->indentCount[I_SPC_LN]);
    }
  }
}


//=============================================================================
//
//  EditLoadFile()
//
bool EditLoadFile(
  HWND hwnd,
  LPWSTR pszFile,
  bool bSkipUTFDetection,
  bool bSkipANSICPDetection,
  bool bForceEncDetection,
  bool bClearUndoHistory,
  EditFileIOStatus* status)
{
  status->bUnicodeErr = false;
  status->bFileTooBig = false;
  status->bUnknownExt = false;
  status->bEncryptedRaw = false;

  HANDLE hFile = CreateFile(pszFile,
    GENERIC_READ,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
    NULL,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    NULL);

  Globals.dwLastError = GetLastError();

  if (hFile == INVALID_HANDLE_VALUE) {
    Encoding_SrcCmdLn(CPI_NONE);
    Encoding_SrcWeak(CPI_NONE);
    return false;
  }

  // calculate buffer limit
  DWORD dwFileSize = GetFileSize(hFile, NULL);
  DWORD dwBufSize = dwFileSize + 16;

  // check for unknown file/extension
  if (!Style_HasLexerForExt(pszFile)) {
    status->bUnknownExt = true;
    INT_PTR const answer = InfoBoxLng(MB_YESNO, L"MsgFileUnknownExt", IDS_MUI_WARN_UNKNOWN_EXT, PathFindFileName(pszFile));
    if (!((IDOK == answer) || (IDYES == answer))) {
      CloseHandle(hFile);
      status->bUnknownExt = true;
      Encoding_SrcCmdLn(CPI_NONE);
      Encoding_SrcWeak(CPI_NONE);
      return false;
    }
  }
  else {
    status->bUnknownExt = true;
  }

  // Check if a warning message should be displayed for large files
  DWORD dwFileSizeLimit = Settings2.FileLoadWarningMB;
  if ((dwFileSizeLimit != 0) && ((dwFileSizeLimit * 1024 * 1024) < dwFileSize)) {
    if (InfoBoxLng(MB_YESNO, L"MsgFileSizeWarning", IDS_MUI_WARN_LOAD_BIG_FILE) != IDYES) {
      CloseHandle(hFile);
      status->bFileTooBig = true;
      Encoding_SrcCmdLn(CPI_NONE);
      Encoding_SrcWeak(CPI_NONE);
      return false;
    }
  }

  char* lpData = AllocMem(dwBufSize, HEAP_ZERO_MEMORY);

  Globals.dwLastError = GetLastError();
  if (!lpData)
  {
    CloseHandle(hFile);
    status->bFileTooBig = true;
    Encoding_SrcCmdLn(CPI_NONE);
    Encoding_SrcWeak(CPI_NONE);
    return false;
  }

  DWORD cbData = 0L;
  int const readFlag = ReadAndDecryptFile(hwnd, hFile, dwBufSize, (void**)&lpData, &cbData);
  Globals.dwLastError = GetLastError();
  CloseHandle(hFile);

  bool bReadSuccess = ((readFlag & DECRYPT_FATAL_ERROR) || (readFlag & DECRYPT_FREAD_FAILED)) ? false : true;
  // ((readFlag == DECRYPT_SUCCESS) || (readFlag & DECRYPT_NO_ENCRYPTION)) => true;
  if ((readFlag & DECRYPT_CANCELED_NO_PASS) || (readFlag & DECRYPT_WRONG_PASS))
  {
    bReadSuccess = (InfoBoxLng(MB_OKCANCEL, L"MsgNoOrWrongPassphrase", IDS_MUI_NOPASS) == IDOK);
    if (!bReadSuccess) {
      FreeMem(lpData);
      return true;
    }
    else {
      status->bEncryptedRaw =  true;
    }
  }

  if (!bReadSuccess) {
    FreeMem(lpData);
    Encoding_SrcCmdLn(CPI_NONE);
    Encoding_SrcWeak(CPI_NONE);
    return false;
  }

  // --------------------------------------------------------------------------
  // Encoding Detection
  // --------------------------------------------------------------------------

  // assume current code-page or default encoding (if forced)
  cpi_enc_t const iAnalyzeFallback = Settings.UseDefaultForFileEncoding ? Settings.DefaultEncoding : CPI_ANSI_DEFAULT;

  // --- 1st check for force encodings ---
  LPCWSTR lpszExt = PathFindExtension(pszFile);
  bool const bNfoDizDetected = (lpszExt && !(StringCchCompareXI(lpszExt, L".nfo") && StringCchCompareXI(lpszExt, L".diz")));

  cpi_enc_t iForcedEncoding = Globals.bForceReLoadAsUTF8 ? CPI_UTF8 :
    ((Settings.LoadNFOasOEM && bNfoDizDetected) ? Globals.DOSEncoding : Encoding_SrcCmdLn(CPI_GET));

  // --- 2nd Use Encoding Analysis if applicable

  size_t const cbNbytes4Analysis = (cbData < 200000L) ? cbData : 200000L;

  float confidence = 0.0f;
  cpi_enc_t iAnalyzedEncoding = iForcedEncoding;

  if (Encoding_IsNONE(iForcedEncoding) || bForceEncDetection)
  {
    iAnalyzedEncoding = Encoding_AnalyzeText(lpData, cbNbytes4Analysis, &confidence, iAnalyzeFallback);

    if (Flags.bDevDebugMode) {
#if 1
      SetAdditionalTitleInfo(Encoding_GetTitleInfoW());
#else
      DocPos const iPos = SciCall_PositionFromLine(SciCall_GetFirstVisibleLine());
      int const iXOff = SciCall_GetXOffset();
      SciCall_SetXOffset(0);
      SciCall_CallTipShow(iPos, Encoding_GetTitleInfoA());
      SciCall_SetXOffset(iXOff);
      Globals.CallTipType = CT_ENC_INFO;
#endif
    }
  }
  else {
    if (Flags.bDevDebugMode) {
      WCHAR wchBuf[128] = { L'\0' };
      StringCchPrintf(wchBuf, COUNTOF(wchBuf), L"ForcedEncoding='%s'", g_Encodings[iForcedEncoding].wchLabel);
      SetAdditionalTitleInfo(wchBuf);
    }
  }

  // ------------------------------------------------------

  if (!bForceEncDetection)
  {
    bool const bIsUnicode = Encoding_IsUTF8(iAnalyzedEncoding) || Encoding_IsUNICODE(iAnalyzedEncoding);

    if (iAnalyzedEncoding == CPI_NONE) 
    {
      iAnalyzedEncoding = iAnalyzeFallback;
      confidence = Settings2.AnalyzeReliableConfidenceLevel;
    }
    else if (iAnalyzedEncoding == CPI_ASCII_7BIT) {
      iAnalyzedEncoding = Settings.LoadASCIIasUTF8 ? CPI_UTF8 : CPI_ANSI_DEFAULT;
      confidence = Settings2.AnalyzeReliableConfidenceLevel;
    }
    else {
      if ((bSkipUTFDetection && bIsUnicode) || (bSkipANSICPDetection && !bIsUnicode)) {
        iAnalyzedEncoding = CPI_NONE;
        confidence = 0.0;
      }
    }
  }

  bool const bIsReliable = (confidence >= Settings2.AnalyzeReliableConfidenceLevel);

  // --------------------------------------------------------------------------

  if (bForceEncDetection && !Encoding_IsNONE(iAnalyzedEncoding)) 
  {
    iForcedEncoding = iAnalyzedEncoding;  // no bIsReliable check (forced unreliable detection)
  }
  // --------------------------------------------------------------------------

  // --- 3rd Unicode Checks

  bool const bIsForced = !Encoding_IsNONE(iForcedEncoding);
  bool const bIsUnicodeForced = Encoding_IsUNICODE(iForcedEncoding);

  // choose best encoding guess
  cpi_enc_t const iFileEncWeak = Encoding_SrcWeak(CPI_GET);

  // set Preferred Encoding
  cpi_enc_t iPreferredEncoding = Settings.LoadASCIIasUTF8 ? CPI_UTF8 : CPI_ANSI_DEFAULT;

  if (bIsForced) {
    iPreferredEncoding = iForcedEncoding;
  }
  else if (!Encoding_IsNONE(iFileEncWeak)) {
    iPreferredEncoding = iFileEncWeak;
  }
  else if (!Encoding_IsNONE(iAnalyzedEncoding) && (bIsReliable || !Settings.UseReliableCEDonly)) {
    iPreferredEncoding = iAnalyzedEncoding;
  }
  else if (Encoding_IsNONE(iPreferredEncoding)) {
    iPreferredEncoding = Settings.LoadASCIIasUTF8 ? CPI_UTF8 : CPI_ANSI_DEFAULT;
  }

  // --------------------------------------------------------------------------

  bool const bIsUTF8Sig = ((cbData >= 3) ? IsUTF8Signature(lpData) : false);

  bool bBOM = false;
  bool bReverse = false;
  bool const bIsUnicodeAnalyzed = ((Encoding_IsUNICODE(iAnalyzedEncoding) && bIsReliable) && !bIsForced && !bSkipUTFDetection && !bIsUTF8Sig);

  cpi_enc_t const encUnicode = bSkipUTFDetection ? CPI_NONE : GetUnicodeEncoding(lpData, cbData, &bBOM, &bReverse);

  if (cbData == 0) {
    FileVars_Init(NULL, 0, &Globals.fvCurFile);
    status->iEOLMode = Settings.DefaultEOLMode;
    status->iEncoding = bIsForced ? iForcedEncoding : (Settings.LoadASCIIasUTF8 ? CPI_UTF8 : iPreferredEncoding);
    EditSetNewText(hwnd, "", 0, bClearUndoHistory);
    SciCall_SetEOLMode(Settings.DefaultEOLMode);
    FreeMem(lpData);
  }
  else if (bIsUnicodeForced || (!bIsForced && (bIsUnicodeAnalyzed || !Encoding_IsNONE(encUnicode))))
  {
    // ===  UNICODE  ===
    if (Encoding_IsNONE(encUnicode)) 
    {
      bool const bBOM_LE = Has_UTF16_LE_BOM(lpData, cbData);
      bool const bBOM_BE = Has_UTF16_BE_BOM(lpData, cbData);

      if ((iForcedEncoding == CPI_UNICODE) || bBOM_LE) {
        bBOM = bBOM_LE;
        bReverse = false;
      }
      else if ((iForcedEncoding == CPI_UNICODEBE) || bBOM_BE) {
        bBOM = bBOM_BE;
        bReverse = true;
      }
    }

    if (bReverse)
    {
      _swab(lpData, lpData, cbData);
      status->iEncoding = (bBOM ? CPI_UNICODEBEBOM : CPI_UNICODEBE);
    }
    else {
      status->iEncoding = (bBOM ? CPI_UNICODEBOM : CPI_UNICODE);
    }

    char* lpDataUTF8 = AllocMem((cbData * 3) + 2, HEAP_ZERO_MEMORY);

    DWORD convCnt = (DWORD)WideCharToMultiByte(Encoding_SciCP, 0, (bBOM) ? (LPWSTR)lpData + 1 : (LPWSTR)lpData,
      (bBOM) ? (cbData / sizeof(WCHAR)) : (cbData / sizeof(WCHAR) + 1), lpDataUTF8, (MBWC_DocPos_Cast)SizeOfMem(lpDataUTF8), NULL, NULL);

    if (convCnt == 0) {
      status->bUnicodeErr = true;
      convCnt = (DWORD)WideCharToMultiByte(CP_ACP, 0, (bBOM) ? (LPWSTR)lpData + 1 : (LPWSTR)lpData,
        (-1), lpDataUTF8, (MBWC_DocPos_Cast)SizeOfMem(lpDataUTF8), NULL, NULL);
    }

    if (convCnt != 0) {
      FreeMem(lpData);
      FileVars_Init(lpDataUTF8, convCnt - 1, &Globals.fvCurFile);
      EditSetNewText(hwnd, lpDataUTF8, convCnt - 1, bClearUndoHistory);
      EditDetectEOLMode(lpDataUTF8, convCnt - 1, status);
      FreeMem(lpDataUTF8);
    }
    else {
      FreeMem(lpDataUTF8);
      FreeMem(lpData);
      Encoding_SrcCmdLn(CPI_NONE);
      Encoding_SrcWeak(CPI_NONE);
      return false;
    }
  }

  else { // ===  ALL OTHERS  ===

    FileVars_Init(lpData, cbData, &Globals.fvCurFile);

    if (Flags.bDevDebugMode) {
      if (FileVars_IsValidEncoding(&Globals.fvCurFile)) {
        WCHAR wchBuf[128] = { L'\0' };
        StringCchPrintf(wchBuf, COUNTOF(wchBuf), L"FileVarEncoding='%s'", 
          g_Encodings[FileVars_GetEncoding(&Globals.fvCurFile)].wchLabel);
        SetAdditionalTitleInfo(wchBuf);
      }
    }

    // ===  UTF-8  ===
    bool const bValidUTF8 = IsValidUTF8(lpData, cbData);
    bool const bForcedUTF8 = Encoding_IsUTF8(iForcedEncoding) || (FileVars_IsUTF8(&Globals.fvCurFile) && !Settings.NoEncodingTags);
    bool const bAnalysisUTF8 = Encoding_IsUTF8(iAnalyzedEncoding) && bIsReliable;
    bool const bSoftHintUTF8 = Encoding_IsUTF8(iAnalyzedEncoding) && Encoding_IsUTF8(iPreferredEncoding); // non-reliable analysis = soft-hint

    bool const bRejectUTF8 = !bValidUTF8 || (!bIsUTF8Sig && bSkipUTFDetection);

    if (bForcedUTF8 || (!bRejectUTF8 && (bIsUTF8Sig || bAnalysisUTF8 || bSoftHintUTF8))) // soft-hint = prefer UTF-8
    {
      if (bIsUTF8Sig) {
        EditSetNewText(hwnd, UTF8StringStart(lpData), cbData - 3, bClearUndoHistory);
        status->iEncoding = CPI_UTF8SIGN;
        EditDetectEOLMode(UTF8StringStart(lpData), cbData - 3, status);
      }
      else {
        EditSetNewText(hwnd, lpData, cbData, bClearUndoHistory);
        status->iEncoding = CPI_UTF8;
        EditDetectEOLMode(lpData, cbData, status);
      }
      FreeMem(lpData);
    }

    else { // ===  ALL OTHER  ===

      if (bIsForced)
        status->iEncoding = iForcedEncoding;

      else {
        status->iEncoding = FileVars_GetEncoding(&Globals.fvCurFile);
        if (Encoding_IsNONE(status->iEncoding))
        {
          status->iEncoding = ((Globals.fvCurFile.mask & FV_ENCODING) ? CPI_ANSI_DEFAULT : iPreferredEncoding);
        }
      }

      if (((Encoding_GetCodePage(status->iEncoding) != CP_UTF7) && Encoding_IsEXTERNAL_8BIT(status->iEncoding)) ||
        ((Encoding_GetCodePage(status->iEncoding) == CP_UTF7) && IsValidUTF7(lpData, cbData))) {

        UINT uCodePage = Encoding_GetCodePage(status->iEncoding);

        LPWSTR lpDataWide = AllocMem(cbData * 2 + 16, HEAP_ZERO_MEMORY);
        int const cbDataWide = MultiByteToWideChar(uCodePage, 0, lpData, cbData, lpDataWide, (MBWC_DocPos_Cast)(SizeOfMem(lpDataWide) / sizeof(WCHAR)));
        if (cbDataWide != 0)
        {
          FreeMem(lpData);
          lpData = AllocMem(cbDataWide * 3 + 16, HEAP_ZERO_MEMORY);

          cbData = WideCharToMultiByte(Encoding_SciCP, 0, lpDataWide, cbDataWide, lpData, (MBWC_DocPos_Cast)SizeOfMem(lpData), NULL, NULL);
          if (cbData != 0) {
            FreeMem(lpDataWide);
            EditSetNewText(hwnd, lpData, cbData, bClearUndoHistory);
            EditDetectEOLMode(lpData, cbData, status);
            FreeMem(lpData);
          }
          else {
            FreeMem(lpDataWide);
            FreeMem(lpData);
            Encoding_SrcCmdLn(CPI_NONE);
            Encoding_SrcWeak(CPI_NONE);
            return false;
          }
        }
        else {
          FreeMem(lpDataWide);
          FreeMem(lpData);
          Encoding_SrcCmdLn(CPI_NONE);
          Encoding_SrcWeak(CPI_NONE);
          return false;
        }
      }
      else {
        status->iEncoding = Encoding_IsValid(iForcedEncoding) ? iForcedEncoding : iPreferredEncoding;
        EditSetNewText(hwnd, lpData, cbData, bClearUndoHistory);
        EditDetectEOLMode(lpData, cbData, status);
        FreeMem(lpData);
      }
    }
  }

  Encoding_SrcCmdLn(CPI_NONE);
  Encoding_SrcWeak(CPI_NONE);

  SciCall_SetCharacterCategoryOptimization(Encoding_IsCJK(iAnalyzedEncoding) ? 0x10000 : 0x1000);

  if (Flags.bDevDebugMode) {
    WCHAR wcBuf[128] = { L'\0' };
    StringCchPrintf(wcBuf, ARRAYSIZE(wcBuf), L"  OS-CP='%s'", g_Encodings[CPI_ANSI_DEFAULT].wchLabel);
    AppendAdditionalTitleInfo(wcBuf);
  }

  return true;
}


//=============================================================================
//
//  EditSaveFile()
//
bool EditSaveFile(
       HWND hwnd,
       LPCWSTR pszFile,
       EditFileIOStatus* status,
       bool bSaveCopy)
{

  HANDLE hFile;
  bool   bWriteSuccess;

  char* lpData;
  DWORD dwBytesWritten;

  status->bCancelDataLoss = false;

  hFile = CreateFile(pszFile,
                     GENERIC_WRITE,
                     FILE_SHARE_READ|FILE_SHARE_WRITE,
                     NULL,
                     OPEN_ALWAYS,
                     FILE_ATTRIBUTE_NORMAL,
                     NULL);
  Globals.dwLastError = GetLastError();

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
      Globals.dwLastError = GetLastError();
    }
  }

  if (hFile == INVALID_HANDLE_VALUE)
    return false;

  // ensure consistent line endings
  if (Settings.FixLineEndings) {
    EditEnsureConsistentLineEndings(hwnd);
  }

  // strip trailing blanks
  if (Settings.FixTrailingBlanks) {
    EditStripLastCharacter(hwnd, true, true);
  }

  // get text
  DocPos cbData = SciCall_GetTextLength();

  if (cbData <= 0) {
    bWriteSuccess = SetEndOfFile(hFile);
    Globals.dwLastError = GetLastError();
  }
  else {

    lpData = AllocMem(cbData + 4, HEAP_ZERO_MEMORY); //fix: +bom
    cbData = SciCall_GetText((cbData+1), lpData);

    // FIXME: move checks in front of disk file access
    // Msg if file tag encoding does not correspond to BOM
    /*if ((g_Encodings[iEncoding].uFlags & NCP_UNICODE) == 0 && (g_Encodings[iEncoding].uFlags & NCP_UTF8_SIGN) == 0) {
      bool bEncodingMismatch = true;
      FILEVARS fv;
      FileVars_Init(lpData,cbData,&fv);
      if (fv.mask & FV_ENCODING) {
        int iAltEncoding;
        if (FileVars_IsValidEncoding(&fv)) {
          iAltEncoding = FileVars_GetEncoding(&fv);
          if (iAltEncoding == iEncoding)
            bEncodingMismatch = false;
          else if ((g_Encodings[iAltEncoding].uFlags & NCP_UTF8) && (g_Encodings[iEncoding].uFlags & NCP_UTF8))
            bEncodingMismatch = false;
        }
        if (bEncodingMismatch) {
          InfoBoxLng(MB_OK,L"MsgEncodingMismatch",IDS_MUI_ENCODINGMISMATCH,
            g_Encodings[iAltEncoding].wchLabel,
            g_Encodings[iEncoding].wchLabel);
        }
      }
    }*/

    if (Encoding_IsUNICODE(status->iEncoding))  // UTF-16LE/BE_(BOM)
    {
      SetEndOfFile(hFile);

      LPWSTR lpDataWide = AllocMem((cbData+1) * 2 + 2, HEAP_ZERO_MEMORY);
      int bomoffset = 0;
      if (Encoding_IsUNICODE_BOM(status->iEncoding)) {
        const char* bom = "\xFF\xFE";
        CopyMemory((char*)lpDataWide, bom, 2);
        bomoffset = 1;
      }
      int const cbDataWide = bomoffset + 
        MultiByteToWideChar(Encoding_SciCP, 0, lpData, (MBWC_DocPos_Cast)cbData, &lpDataWide[bomoffset],
        (MBWC_DocPos_Cast)((SizeOfMem(lpDataWide) / sizeof(WCHAR)) - bomoffset));
      if (Encoding_IsUNICODE_REVERSE(status->iEncoding)) {
        _swab((char*)lpDataWide, (char*)lpDataWide, cbDataWide * sizeof(WCHAR));
      }
      bWriteSuccess = EncryptAndWriteFile(hwnd, hFile, (BYTE*)lpDataWide, cbDataWide * sizeof(WCHAR), &dwBytesWritten);
      Globals.dwLastError = GetLastError();

      FreeMem(lpDataWide);
      FreeMem(lpData);
    }

    else if (Encoding_IsUTF8(status->iEncoding))
    {
      SetEndOfFile(hFile);

      DocPos bomoffset = 0;
      if (Encoding_IsUTF8_SIGN(status->iEncoding)) {
        const char* bom = "\xEF\xBB\xBF";
        bomoffset = 3;
        MoveMemory(&lpData[bomoffset], lpData, cbData);
        CopyMemory(lpData, bom, bomoffset);
      }
      //bWriteSuccess = WriteFile(hFile,lpData,cbData,&dwBytesWritten,NULL);
      bWriteSuccess = EncryptAndWriteFile(hwnd, hFile, (BYTE*)lpData, (DWORD)(cbData + bomoffset), &dwBytesWritten);
      Globals.dwLastError = GetLastError();

      FreeMem(lpData);
    }

    else if (Encoding_IsEXTERNAL_8BIT(status->iEncoding)) {

      BOOL bCancelDataLoss = FALSE;
      UINT uCodePage = Encoding_GetCodePage(status->iEncoding);

      LPWSTR lpDataWide = AllocMem((cbData+1) * 2, HEAP_ZERO_MEMORY);
      int const cbDataWide = MultiByteToWideChar(Encoding_SciCP,0,lpData, (MBWC_DocPos_Cast)cbData,
                                                 lpDataWide, (MBWC_DocPos_Cast)(SizeOfMem(lpDataWide)/sizeof(WCHAR)));

      int cbDataNew = 0;
      if (Encoding_IsMBCS(status->iEncoding)) {
        FreeMem(lpData);
        lpData = AllocMem(SizeOfMem(lpDataWide) * 2, HEAP_ZERO_MEMORY); // need more space
        cbDataNew = WideCharToMultiByte(uCodePage, 0, lpDataWide, cbDataWide,
                                     lpData, (MBWC_DocPos_Cast)SizeOfMem(lpData), NULL, NULL);
      }
      else {
        ZeroMemory(lpData, SizeOfMem(lpData));
        cbDataNew = WideCharToMultiByte(uCodePage,WC_NO_BEST_FIT_CHARS,lpDataWide,cbDataWide,
                                     lpData,(MBWC_DocPos_Cast)SizeOfMem(lpData),NULL,&bCancelDataLoss);
        if (!bCancelDataLoss) {
          cbDataNew = WideCharToMultiByte(uCodePage,0,lpDataWide,cbDataWide,
                                          lpData,(MBWC_DocPos_Cast)SizeOfMem(lpData),NULL,NULL);
          bCancelDataLoss = FALSE;
        }
      }
      FreeMem(lpDataWide);

      if (!bCancelDataLoss || InfoBoxLng(MB_OKCANCEL,L"MsgConv3",IDS_MUI_ERR_UNICODE2) == IDOK) {
        SetEndOfFile(hFile);
        bWriteSuccess = EncryptAndWriteFile(hwnd, hFile, (BYTE*)lpData, cbDataNew, &dwBytesWritten);
        Globals.dwLastError = GetLastError();
      }
      else {
        bWriteSuccess = false;
        status->bCancelDataLoss = true;
      }
      FreeMem(lpData);
    }

    else {
      SetEndOfFile(hFile);
      bWriteSuccess = EncryptAndWriteFile(hwnd, hFile, (BYTE*)lpData, (DWORD)cbData, &dwBytesWritten);
      Globals.dwLastError = GetLastError();
      FreeMem(lpData);
    }
  }

  CloseHandle(hFile);

  if (bWriteSuccess) {
    if (!bSaveCopy) {
      SciCall_SetSavePoint();
    }
    return true;
  }
  return false;
}


//=============================================================================
//
//  EditInvertCase()
//
void EditInvertCase(HWND hwnd)
{
  UNUSED(hwnd);
  const DocPos iCurPos = SciCall_GetCurrentPos(); 
  const DocPos iAnchorPos = SciCall_GetAnchor();

  if (iCurPos != iAnchorPos)
  {
    if (!Sci_IsMultiOrRectangleSelection())
    {
      InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECT);
      return;
    }
    const DocPos iSelStart = SciCall_GetSelectionStart();
    const DocPos iSelEnd = SciCall_GetSelectionEnd();
    const DocPos iSelSize = SciCall_GetSelText(NULL);

    LPSTR pszText = AllocMem(iSelSize, HEAP_ZERO_MEMORY);
    LPWSTR pszTextW = AllocMem(iSelSize * sizeof(WCHAR), HEAP_ZERO_MEMORY);
    if (!pszText || !pszTextW) {
      FreeMem(pszText);
      FreeMem(pszTextW);
      return;
    }

    SciCall_GetSelText(pszText);

    int const cchTextW = MultiByteToWideChar(Encoding_SciCP, 0, pszText, (MBWC_DocPos_Cast)(iSelSize - 1),
      pszTextW, (MBWC_DocPos_Cast)iSelSize);

    bool bChanged = false;
    for (int i = 0; i < cchTextW; i++) {
      if (IsCharUpperW(pszTextW[i])) {
        pszTextW[i] = LOWORD(CharLowerW((LPWSTR)(LONG_PTR)MAKELONG(pszTextW[i], 0)));
        bChanged = true;
      }
      else if (IsCharLowerW(pszTextW[i])) {
        pszTextW[i] = LOWORD(CharUpperW((LPWSTR)(LONG_PTR)MAKELONG(pszTextW[i], 0)));
        bChanged = true;
      }
    }

    if (bChanged) {
      WideCharToMultiByte(Encoding_SciCP, 0, pszTextW, cchTextW,
        pszText, (MBWC_DocPos_Cast)SizeOfMem(pszText), NULL, NULL);
      SciCall_Clear();
      SciCall_AddText((iSelEnd - iSelStart), pszText);
      SciCall_SetSel(iAnchorPos, iCurPos);
    }
    FreeMem(pszText);
    FreeMem(pszTextW);
  }
}


//=============================================================================
//
//  EditTitleCase()
//
void EditTitleCase(HWND hwnd)
{
  UNUSED(hwnd);
  const DocPos iCurPos = SciCall_GetCurrentPos();
  const DocPos iAnchorPos = SciCall_GetAnchor();

  if (iCurPos != iAnchorPos)
  {
    if (!Sci_IsMultiOrRectangleSelection())
    {
      InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECT);
      return;
    }
    const DocPos iSelStart = SciCall_GetSelectionStart();
    const DocPos iSelEnd = SciCall_GetSelectionEnd();
    const DocPos iSelSize = SciCall_GetSelText(NULL);

    char* pszText = AllocMem(iSelSize, HEAP_ZERO_MEMORY);
    LPWSTR pszTextW = AllocMem((iSelSize * sizeof(WCHAR)), HEAP_ZERO_MEMORY);

    if (pszText == NULL || pszTextW == NULL) {
      FreeMem(pszText);
      FreeMem(pszTextW);
      return;
    }
    SciCall_GetSelText(pszText);

    int const cchTextW = MultiByteToWideChar(Encoding_SciCP, 0, pszText, (MBWC_DocPos_Cast)(iSelSize - 1),
      pszTextW, (MBWC_DocPos_Cast)iSelSize);

    bool bChanged = false;
    LPWSTR pszMappedW = AllocMem(SizeOfMem(pszTextW), HEAP_ZERO_MEMORY);
    if (pszMappedW) {
      // first make lower case, before applying TitleCase
      if (LCMapString(LOCALE_SYSTEM_DEFAULT, (LCMAP_LINGUISTIC_CASING | LCMAP_LOWERCASE), pszTextW, cchTextW, pszMappedW, (MBWC_DocPos_Cast)iSelSize)) {
        if (LCMapString(LOCALE_SYSTEM_DEFAULT, LCMAP_TITLECASE, pszMappedW, cchTextW, pszTextW, (MBWC_DocPos_Cast)iSelSize)) {
          bChanged = true;
        }
      }
      FreeMem(pszMappedW);
    }

    if (bChanged) {

      WideCharToMultiByte(Encoding_SciCP, 0, pszTextW, cchTextW,
        pszText, (MBWC_DocPos_Cast)SizeOfMem(pszText), NULL, NULL);

      SciCall_Clear();
      SciCall_AddText((iSelEnd - iSelStart), pszText);
      SciCall_SetSel(iAnchorPos, iCurPos);
    }

    FreeMem(pszText);
    FreeMem(pszTextW);
  }
}

//=============================================================================
//
//  EditSentenceCase()
//
void EditSentenceCase(HWND hwnd)
{
  UNUSED(hwnd);
  const DocPos iCurPos = SciCall_GetCurrentPos();
  const DocPos iAnchorPos = SciCall_GetAnchor();

  if (iCurPos != iAnchorPos)
  {
    if (!Sci_IsMultiOrRectangleSelection())
    {
      InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECT);
      return;
    }
    const DocPos iSelStart = SciCall_GetSelectionStart();
    const DocPos iSelEnd = SciCall_GetSelectionEnd();
    const DocPos iSelSize = SciCall_GetSelText(NULL);

    char* pszText = AllocMem(iSelSize, HEAP_ZERO_MEMORY);
    LPWSTR pszTextW = AllocMem((iSelSize * sizeof(WCHAR)), HEAP_ZERO_MEMORY);

    if (pszText == NULL || pszTextW == NULL) {
      FreeMem(pszText);
      FreeMem(pszTextW);
      return;
    }
    SciCall_GetSelText(pszText);

    int cchTextW = MultiByteToWideChar(Encoding_SciCP, 0, pszText, (MBWC_DocPos_Cast)(iSelSize - 1), pszTextW, (MBWC_DocPos_Cast)iSelSize);

    bool bChanged = false;
    bool bNewSentence = true;
    for (int i = 0; i < cchTextW; i++) {
      if (StrChr(L".;!?\r\n", pszTextW[i])) {
        bNewSentence = true;
      }
      else {
        if (IsCharAlphaNumericW(pszTextW[i])) {
          if (bNewSentence) {
            if (IsCharLowerW(pszTextW[i])) {
              pszTextW[i] = LOWORD(CharUpperW((LPWSTR)(LONG_PTR)MAKELONG(pszTextW[i], 0)));
              bChanged = true;
            }
            bNewSentence = false;
          }
          else {
            if (IsCharUpperW(pszTextW[i])) {
              pszTextW[i] = LOWORD(CharLowerW((LPWSTR)(LONG_PTR)MAKELONG(pszTextW[i], 0)));
              bChanged = true;
            }
          }
        }
      }
    }

    if (bChanged) {

      WideCharToMultiByte(Encoding_SciCP, 0, pszTextW, cchTextW,
        pszText, (MBWC_DocPos_Cast)SizeOfMem(pszText), NULL, NULL);

      SciCall_Clear();
      SciCall_AddText((iSelEnd - iSelStart), pszText);
      SciCall_SetSel(iAnchorPos, iCurPos);
    }

    FreeMem(pszText);
    FreeMem(pszTextW);
  }
}



//=============================================================================
//
//  EditURLEncode()
//
void EditURLEncode(HWND hwnd)
{
  if (SciCall_IsSelectionEmpty()) { return; }

  if (Sci_IsMultiOrRectangleSelection()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECT);
    return;
  }

  DocPos const iCurPos = SciCall_GetCurrentPos();
  DocPos const iAnchorPos = SciCall_GetAnchor();
  DocPos const iSelSize = SciCall_GetSelText(NULL);

  const char* pszText = (const char*)SciCall_GetRangePointer(min_p(iCurPos, iAnchorPos), iSelSize);

  LPWSTR pszTextW = AllocMem(iSelSize * sizeof(WCHAR), HEAP_ZERO_MEMORY);
  if (pszTextW == NULL) {
    return;
  }

  /*int cchTextW =*/ MultiByteToWideChar(Encoding_SciCP, 0, pszText, (MBWC_DocPos_Cast)(iSelSize-1),
                                         pszTextW, (MBWC_DocPos_Cast)iSelSize);

  size_t const cchEscaped = iSelSize * 3;
  char* pszEscaped = (char*)AllocMem(cchEscaped, HEAP_ZERO_MEMORY);
  if (pszEscaped == NULL) {
    FreeMem(pszTextW);
    return;
  }

  LPWSTR pszEscapedW = (LPWSTR)AllocMem(cchEscaped * sizeof(WCHAR), HEAP_ZERO_MEMORY);
  if (pszEscapedW == NULL) {
    FreeMem(pszTextW);
    FreeMem(pszEscaped);
    return;
  }

  DWORD cchEscapedW = (DWORD)cchEscaped;
  UrlEscape(pszTextW, pszEscapedW, &cchEscapedW, URL_ESCAPE_SEGMENT_ONLY | URL_ESCAPE_PERCENT | URL_ESCAPE_AS_UTF8);

  DWORD const cchEscapedEnc = WideCharToMultiByte(Encoding_SciCP, 0, pszEscapedW, cchEscapedW, 
                                                  pszEscaped, (MBWC_DocPos_Cast)cchEscaped, NULL, NULL);

  DocPos const saveTargetBeg = SciCall_GetTargetStart();
  DocPos const saveTargetEnd = SciCall_GetTargetEnd();

  if (iCurPos < iAnchorPos)
    SciCall_SetTargetRange(iCurPos, iAnchorPos);
  else
    SciCall_SetTargetRange(iAnchorPos, iCurPos);

  SciCall_ReplaceTarget(cchEscapedEnc, pszEscaped);
 
  SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore


  if (iCurPos < iAnchorPos)
    EditSetSelectionEx(hwnd, iCurPos + cchEscapedEnc, iCurPos, -1, -1);
  else
    EditSetSelectionEx(hwnd, iAnchorPos, iAnchorPos + cchEscapedEnc, -1, -1);

  FreeMem(pszTextW);
  FreeMem(pszEscaped);
  FreeMem(pszEscapedW);
}


//=============================================================================
//
//  EditURLDecode()
//
void EditURLDecode(HWND hwnd)
{
  if (SciCall_IsSelectionEmpty()) { return; }

  if (Sci_IsMultiOrRectangleSelection()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECT);
    return;
  }

  DocPos const iCurPos = SciCall_GetCurrentPos();
  DocPos const iAnchorPos = SciCall_GetAnchor();
  DocPos const iSelSize = SciCall_GetSelText(NULL);

  const char* pszText = SciCall_GetRangePointer(min_p(iCurPos, iAnchorPos), iSelSize);

  LPWSTR pszTextW = AllocMem(iSelSize * sizeof(WCHAR), HEAP_ZERO_MEMORY);
  if (pszTextW == NULL) {
    return;
  }

  /*int cchTextW =*/ MultiByteToWideChar(Encoding_SciCP, 0, pszText, (MBWC_DocPos_Cast)(iSelSize-1),
                                         pszTextW, (MBWC_DocPos_Cast)iSelSize);

  size_t const cchUnescaped = iSelSize * 3;
  char* pszUnescaped = (char*)AllocMem(cchUnescaped, HEAP_ZERO_MEMORY);
  if (pszUnescaped == NULL) {
    FreeMem(pszTextW);
    return;
  }

  LPWSTR pszUnescapedW = (LPWSTR)AllocMem(cchUnescaped * sizeof(WCHAR), HEAP_ZERO_MEMORY);
  if (pszUnescapedW == NULL) {
    FreeMem(pszTextW);
    FreeMem(pszUnescaped);
    return;
  }

  DWORD cchUnescapedW = (DWORD)cchUnescaped;
  UrlUnescapeEx(pszTextW, pszUnescapedW, &cchUnescapedW);

  DWORD const cchUnescapedDec = WideCharToMultiByte(Encoding_SciCP, 0, pszUnescapedW, cchUnescapedW,
                                                    pszUnescaped, (MBWC_DocPos_Cast)cchUnescaped, NULL, NULL);

  DocPos const saveTargetBeg = SciCall_GetTargetStart();
  DocPos const saveTargetEnd = SciCall_GetTargetEnd();

  if (iCurPos < iAnchorPos)
    SciCall_SetTargetRange(iCurPos, iAnchorPos);
  else
    SciCall_SetTargetRange(iAnchorPos, iCurPos);

  SciCall_ReplaceTarget(cchUnescapedDec, pszUnescaped);

  SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore

  if (iCurPos < iAnchorPos)
    EditSetSelectionEx(hwnd, iCurPos + cchUnescapedDec, iCurPos, -1, -1);
  else 
    EditSetSelectionEx(hwnd, iAnchorPos, iAnchorPos + cchUnescapedDec, -1, -1);

  FreeMem(pszTextW);
  FreeMem(pszUnescaped);
  FreeMem(pszUnescapedW);

}


//=============================================================================
//
//  EditEscapeCChars()
//
void EditEscapeCChars(HWND hwnd) {

  if (!SciCall_IsSelectionEmpty())
  {
    if (Sci_IsMultiOrRectangleSelection())
    {
      InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECT);
      return;
    }

    EDITFINDREPLACE efr = EFR_INIT_DATA;
    efr.hwnd = hwnd;

    StringCchCopyA(efr.szFind,FNDRPL_BUFFER,"\\");
    StringCchCopyA(efr.szReplace,FNDRPL_BUFFER,"\\\\");
    EditReplaceAllInSelection(hwnd,&efr,false);

    StringCchCopyA(efr.szFind,FNDRPL_BUFFER,"\"");
    StringCchCopyA(efr.szReplace,FNDRPL_BUFFER,"\\\"");
    EditReplaceAllInSelection(hwnd,&efr,false);

    StringCchCopyA(efr.szFind,FNDRPL_BUFFER,"\'");
    StringCchCopyA(efr.szReplace,FNDRPL_BUFFER,"\\\'");
    EditReplaceAllInSelection(hwnd,&efr,false);
  }
}


//=============================================================================
//
//  EditUnescapeCChars()
//
void EditUnescapeCChars(HWND hwnd) {

  if (!SciCall_IsSelectionEmpty())
  {
    if (Sci_IsMultiOrRectangleSelection())
    {
      InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECT);
      return;
    }

    EDITFINDREPLACE efr = EFR_INIT_DATA;
    efr.hwnd = hwnd;

    StringCchCopyA(efr.szFind,FNDRPL_BUFFER,"\\\\");
    StringCchCopyA(efr.szReplace,FNDRPL_BUFFER,"\\");
    EditReplaceAllInSelection(hwnd,&efr,false);

    StringCchCopyA(efr.szFind,FNDRPL_BUFFER,"\\\"");
    StringCchCopyA(efr.szReplace,FNDRPL_BUFFER,"\"");
    EditReplaceAllInSelection(hwnd,&efr,false);

    StringCchCopyA(efr.szFind,FNDRPL_BUFFER,"\\\'");
    StringCchCopyA(efr.szReplace,FNDRPL_BUFFER,"\'");
    EditReplaceAllInSelection(hwnd,&efr,false);
  }
}


//=============================================================================
//
//  EditChar2Hex()
//
void EditChar2Hex(HWND hwnd) {

  if (Sci_IsMultiOrRectangleSelection()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECT);
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

  EditSetSelectionEx(hwnd, iSelStart, iSelEnd, -1, -1);
  
  // TODO(rkotten): iterate over complete selection?
  if (SciCall_GetSelText(NULL) <= COUNTOF(ch)) {
    SciCall_GetSelText(ch);
  }

  if (ch[0] == '\0') {
    StringCchCopyA(ch, COUNTOF(ch), "\\x00");
  }
  else {
    WCHAR wch[32] = { L'\0' };
    MultiByteToWideChar(Encoding_SciCP, 0, ch, -1, wch, COUNTOF(wch));
    if (wch[0] <= 0xFF)
      StringCchPrintfA(ch, COUNTOF(ch), "\\x%02X", wch[0] & 0xFF);
    else
      StringCchPrintfA(ch, COUNTOF(ch), "\\u%04X", wch[0]);
  }
  SendMessage(hwnd, SCI_REPLACESEL, 0, (LPARAM)ch);

  DocPos const iReplLen = (DocPos)StringCchLenA(ch, COUNTOF(ch));

  if (iCurPos < iAnchorPos) {
    EditSetSelectionEx(hwnd, iCurPos + iReplLen, iCurPos, -1, -1);
  }
  else if (iCurPos > iAnchorPos) {
    EditSetSelectionEx(hwnd, iAnchorPos, iAnchorPos + iReplLen, -1, -1);
  }
  else { // empty selection
    EditSetSelectionEx(hwnd, iCurPos + iReplLen, iCurPos + iReplLen, -1, -1);
  }
}


//=============================================================================
//
//  EditHex2Char()
//
void EditHex2Char(HWND hwnd) 
{
  if (SciCall_IsSelectionEmpty()) { return; }
    
  if (Sci_IsMultiOrRectangleSelection()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECT);
    return;
  }

  DocPos iCurPos = SciCall_GetCurrentPos();
  DocPos iAnchorPos = SciCall_GetAnchor();
  DocPos iSelStart = SciCall_GetSelectionStart();
  const DocPos iSelEnd = SciCall_GetSelectionEnd();

  char ch[32] = { L'\0' };
  if (SciCall_GetSelText(NULL) <= COUNTOF(ch))
  {
    bool bTrySelExpand = false;

    SciCall_GetSelText(ch);

    if (StrChrIA(ch, ' ') || StrChrIA(ch, '\t') || StrChrIA(ch, '\r') || StrChrIA(ch, '\n') || StrChrIA(ch, '-')) {
      return;
    }

    if (StrCmpNIA(ch, "0x", 2) == 0 || StrCmpNIA(ch, "\\x", 2) == 0 || StrCmpNIA(ch, "\\u", 2) == 0) {
      ch[0] = '0';
      ch[1] = 'x';
    }
    else if (StrChrIA("xu", ch[0])) {
      ch[0] = '0';
      bTrySelExpand = true;
    }
    else
      return;

    unsigned int i = 0;
    if (sscanf_s(ch, "%x", &i) == 1) {
      int cch = 0;
      if (i == 0) {
        ch[0] = 0;
        cch = 1;
      }
      else {
        WCHAR wch[8] = { L'\0' };
        StringCchPrintfW(wch, COUNTOF(wch), L"%lc", (WCHAR)i);
        cch = WideCharToMultiByte(Encoding_SciCP, 0, wch, -1, ch, COUNTOF(ch), NULL, NULL) - 1;

        if (bTrySelExpand && (SciCall_GetCharAt(iSelStart - 1) == '\\')) {
          --iSelStart;
          if (iCurPos < iAnchorPos) { --iCurPos; } else { --iAnchorPos; }
        }
      }
      EditSetSelectionEx(hwnd, iSelStart, iSelEnd, -1, -1);
      SciCall_ReplaceSel(ch);
      if (iCurPos < iAnchorPos)
        EditSetSelectionEx(hwnd, iCurPos + cch, iCurPos, -1, -1);
      else
        EditSetSelectionEx(hwnd, iAnchorPos, iAnchorPos + cch, -1, -1);

    }
  }
}


//=============================================================================
//
//  EditFindMatchingBrace()
//
void EditFindMatchingBrace(HWND hwnd)
{
  bool bIsAfter = false;
  DocPos iMatchingBracePos = (DocPos)-1;
  const DocPos iCurPos = SciCall_GetCurrentPos();
  const char c = SciCall_GetCharAt(iCurPos);
  if (StrChrA(NP3_BRACES_TO_MATCH, c)) {
    iMatchingBracePos = SciCall_BraceMatch(iCurPos);
  }
  else { // Try one before
    const DocPos iPosBefore = SciCall_PositionBefore(iCurPos);
    const char cb = SciCall_GetCharAt(iPosBefore);
    if (StrChrA(NP3_BRACES_TO_MATCH, cb)) {
      iMatchingBracePos = SciCall_BraceMatch(iPosBefore);
    }
    bIsAfter = true;
  }
  if (iMatchingBracePos != (DocPos)-1) {
    iMatchingBracePos = bIsAfter ? iMatchingBracePos : SciCall_PositionAfter(iMatchingBracePos);
    EditSetSelectionEx(hwnd, iMatchingBracePos, iMatchingBracePos, -1, -1);
  }
}


//=============================================================================
//
//  EditSelectToMatchingBrace()
//
void EditSelectToMatchingBrace(HWND hwnd)
{
  bool bIsAfter = false;
  DocPos iMatchingBracePos = -1;
  const DocPos iCurPos = SciCall_GetCurrentPos();
  const char c = SciCall_GetCharAt(iCurPos);
  if (StrChrA(NP3_BRACES_TO_MATCH, c)) {
    iMatchingBracePos = SciCall_BraceMatch(iCurPos);
  }
  else { // Try one before
    const DocPos iPosBefore = SciCall_PositionBefore(iCurPos);
    const char cb = SciCall_GetCharAt(iPosBefore);
    if (StrChrA(NP3_BRACES_TO_MATCH, cb)) {
      iMatchingBracePos = SciCall_BraceMatch(iPosBefore);
    }
    bIsAfter = true;
  }
  if (iMatchingBracePos != (DocPos)-1) {
    if (bIsAfter)
      EditSetSelectionEx(hwnd, iCurPos, iMatchingBracePos, -1, -1);
    else
      EditSetSelectionEx(hwnd, iCurPos, SciCall_PositionAfter(iMatchingBracePos), -1, -1);
  }
}


//=============================================================================
//
//  EditModifyNumber()
//
void EditModifyNumber(HWND hwnd,bool bIncrease) {

  if (Sci_IsMultiOrRectangleSelection()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECT);
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

      unsigned int iNumber;
      int iWidth;
      char chFormat[32] = { '\0' };
      if (!StrChrIA(chNumber, 'x') && sscanf_s(chNumber, "%ui", &iNumber) == 1) {
        iWidth = (int)StringCchLenA(chNumber, COUNTOF(chNumber));
        if (bIncrease && (iNumber < UINT_MAX))
          iNumber++;
        if (!bIncrease && (iNumber > 0))
          iNumber--;

        StringCchPrintfA(chFormat, COUNTOF(chFormat), "%%0%ii", iWidth);
        StringCchPrintfA(chNumber, COUNTOF(chNumber), chFormat, iNumber);
        SciCall_ReplaceSel(chNumber);
        SciCall_SetSel(iSelStart, iSelStart + (DocPos)StringCchLenA(chNumber, COUNTOF(chNumber)));
      }
      else if (sscanf_s(chNumber, "%x", &iNumber) == 1) 
      {
        iWidth = (int)StringCchLenA(chNumber, COUNTOF(chNumber)) - 2;
        if (bIncrease && iNumber < UINT_MAX)
          iNumber++;
        if (!bIncrease && iNumber > 0)
          iNumber--;
        bool bUppercase = false;
        for (int i = (int)StringCchLenA(chNumber, COUNTOF(chNumber)) - 1; i >= 0; i--) {
          if (IsCharLowerA(chNumber[i])) {
            break;
          }
          if (IsCharUpperA(chNumber[i])) {
            bUppercase = true;
            break;
          }
        }
        if (bUppercase)
          StringCchPrintfA(chFormat, COUNTOF(chFormat), "%%#0%iX", iWidth);
        else
          StringCchPrintfA(chFormat, COUNTOF(chFormat), "%%#0%ix", iWidth);

        StringCchPrintfA(chNumber, COUNTOF(chNumber), chFormat, iNumber);
        SciCall_ReplaceSel(chNumber);
        SciCall_SetSel(iSelStart, iSelStart + (DocPos)StringCchLenA(chNumber, COUNTOF(chNumber)));
      }
    }
  }
  UNUSED(hwnd);
}


//=============================================================================
//
//  EditTabsToSpaces()
//
void EditTabsToSpaces(HWND hwnd,int nTabWidth,bool bOnlyIndentingWS)
{
  if (SciCall_IsSelectionEmpty()) { return; } // no selection

  if (Sci_IsMultiOrRectangleSelection()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECT);
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

  LPWSTR pszTextW = AllocMem((iSelCount + 1) * sizeof(WCHAR), HEAP_ZERO_MEMORY);
  if (pszTextW == NULL) { return; }

  int cchTextW = MultiByteToWideChar(Encoding_SciCP,0,pszText,(MBWC_DocPos_Cast)iSelCount,pszTextW,(MBWC_DocPos_Cast)iSelCount+1);

  LPWSTR pszConvW = AllocMem(cchTextW*sizeof(WCHAR)*nTabWidth+2, HEAP_ZERO_MEMORY);
  if (pszConvW == NULL) {
    FreeMem(pszTextW);
    return;
  }

  int cchConvW = 0;

  // Contributed by Homam
  // Thank you very much!
  int i = 0;
  bool bIsLineStart = true;
  bool bModified = false;
  for (int iTextW = 0; iTextW < cchTextW; iTextW++)
  {
    WCHAR w = pszTextW[iTextW];
    if (w == L'\t' && (!bOnlyIndentingWS || bIsLineStart)) {
      for (int j = 0; j < nTabWidth - i % nTabWidth; j++)
        pszConvW[cchConvW++] = L' ';
      i = 0;
      bModified = true;
    }
    else {
      i++;
      if (w == L'\n' || w == L'\r') {
        i = 0;
        bIsLineStart = true;
      }
      else if (w != L' ')
        bIsLineStart = false;
      pszConvW[cchConvW++] = w;
    }
  }

  FreeMem(pszTextW);

  if (bModified) {
    char* pszText2 = AllocMem(cchConvW*3, HEAP_ZERO_MEMORY);

    int cchConvM = WideCharToMultiByte(Encoding_SciCP,0,pszConvW,cchConvW,
                                       pszText2,(MBWC_DocPos_Cast)SizeOfMem(pszText2),NULL,NULL);

    if (iCurPos < iAnchorPos) {
      iCurPos = iSelStart;
      iAnchorPos = iSelStart + cchConvM;
    }
    else {
      iAnchorPos = iSelStart;
      iCurPos = iSelStart + cchConvM;
    }

    DocPos const saveTargetBeg = SciCall_GetTargetStart();
    DocPos const saveTargetEnd = SciCall_GetTargetEnd();
    SciCall_SetTargetRange(iSelStart, iSelEnd);
    SciCall_ReplaceTarget(cchConvM, pszText2);
    SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore

    EditSetSelectionEx(hwnd, iAnchorPos, iCurPos, -1, -1);

    FreeMem(pszText2);
  }

  FreeMem(pszConvW);
}


//=============================================================================
//
//  EditSpacesToTabs()
//
void EditSpacesToTabs(HWND hwnd,int nTabWidth,bool bOnlyIndentingWS)
{
  if (SciCall_IsSelectionEmpty()) { return; } // no selection

  if (Sci_IsMultiOrRectangleSelection()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECT);
    return;
  }

  DocPos iCurPos = SciCall_GetCurrentPos();
  DocPos iAnchorPos = SciCall_GetAnchor();

  DocPos const iSelStart = SciCall_GetSelectionStart();
  //DocLn iLine = SciCall_LineFromPosition(iSelStart);
  //iSelStart = SciCall_PositionFromLine(iLine);   // re-base selection to start of line
  DocPos const iSelEnd = SciCall_GetSelectionEnd();
  DocPos const iSelCount = (iSelEnd - iSelStart);

  const char* pszText = SciCall_GetRangePointer(iSelStart, iSelCount);

  LPWSTR pszTextW = AllocMem((iSelCount + 1) * sizeof(WCHAR), HEAP_ZERO_MEMORY);
  if (pszTextW == NULL)
  {
    return;
  }

  int cchTextW = MultiByteToWideChar(Encoding_SciCP,0,pszText,(MBWC_DocPos_Cast)iSelCount,pszTextW,(MBWC_DocPos_Cast)iSelCount+1);

  LPWSTR pszConvW = AllocMem(cchTextW*sizeof(WCHAR)+2, HEAP_ZERO_MEMORY);
  if (pszConvW == NULL) {
    FreeMem(pszTextW);
    return;
  }

  int cchConvW = 0;

  // Contributed by Homam
  // Thank you very much!
  int i = 0;
  int j = 0;
  bool bIsLineStart = true;
  bool bModified = false;
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
        bIsLineStart = true;
      }
      else
        bIsLineStart = false;
      pszConvW[cchConvW++] = w;
    }
  }
  if (j > 0) {
    for (int t = 0; t < j; t++)
      pszConvW[cchConvW++] = space[t];
    }

  FreeMem(pszTextW);

  if (bModified || cchConvW != cchTextW) {
    char* pszText2 = AllocMem(cchConvW * 3, HEAP_ZERO_MEMORY);

    int cchConvM = WideCharToMultiByte(Encoding_SciCP,0,pszConvW,cchConvW,
                                       pszText2,(MBWC_DocPos_Cast)SizeOfMem(pszText2),NULL,NULL);

    if (iAnchorPos > iCurPos) {
      iCurPos = iSelStart;
      iAnchorPos = iSelStart + cchConvM;
    }
    else {
      iAnchorPos = iSelStart;
      iCurPos = iSelStart + cchConvM;
    }

    DocPos const saveTargetBeg = SciCall_GetTargetStart();
    DocPos const saveTargetEnd = SciCall_GetTargetEnd();
    SciCall_SetTargetRange(iSelStart, iSelEnd);
    SciCall_ReplaceTarget(cchConvM, pszText2);
    SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore

    EditSetSelectionEx(hwnd, iAnchorPos, iCurPos, -1, -1);

    FreeMem(pszText2);
  }

  FreeMem(pszConvW);
}



//=============================================================================
//
//  _EditMoveLines()
//
static void  _EditMoveLines(bool bMoveUp)
{
  if (Sci_IsMultiOrRectangleSelection()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECT);
  }
  else {

    DocPos const iSelBeg = SciCall_GetSelectionStart();
    DocPos const iSelEnd = SciCall_GetSelectionEnd();
    DocLn  const iBegLine = SciCall_LineFromPosition(iSelBeg);
    DocLn  const iEndLine = SciCall_LineFromPosition(iSelEnd);

    DocLn lastLine = Sci_GetLastDocLineNumber();

    if (Sci_GetNetLineLength(lastLine) == 0) { --lastLine; }

    bool const bCanMove = bMoveUp ? (iBegLine > 0) : (iEndLine < lastLine);
    if (bCanMove) {

      bool const bForwardSelection = Sci_IsForwardSelection();
      int const direction = (bMoveUp ? -1 : 1);

      DocPos const iBegChCount = SciCall_CountCharacters(SciCall_PositionFromLine(iBegLine), iSelBeg);
      DocPos const iEndChCount = SciCall_CountCharacters(SciCall_PositionFromLine(iEndLine), iSelEnd);

      if (bMoveUp)
        SciCall_MoveSelectedLinesUp();
      else
        SciCall_MoveSelectedLinesDown();

      DocPos const iNewSelBeg = SciCall_PositionRelative(SciCall_PositionFromLine(iBegLine + direction), iBegChCount);
      DocPos const iNewSelEnd = SciCall_PositionRelative(SciCall_PositionFromLine(iEndLine + direction), iEndChCount);

      if (bForwardSelection)
        SciCall_SetSel(iNewSelBeg, iNewSelEnd);
      else
        SciCall_SetSel(iNewSelEnd, iNewSelBeg);
    }
  }
}


//=============================================================================
//
//  EditMoveUp()
//
void EditMoveUp(HWND hwnd)
{
  UNUSED(hwnd);
  _EditMoveLines(true);
}


//=============================================================================
//
//  EditMoveDown()
//
void EditMoveDown(HWND hwnd)
{
  UNUSED(hwnd);
  _EditMoveLines(false);
}


//=============================================================================
//
//  EditJumpToSelectionStart()
//
void EditJumpToSelectionStart(HWND hwnd)
{
  UNUSED(hwnd);
  if (!Sci_IsMultiSelection()) {
    if (SciCall_GetCurrentPos() != SciCall_GetSelectionStart()) {
      SciCall_SwapMainAnchorCaret();
    }
  }
}

//=============================================================================
//
//  EditJumpToSelectionEnd()
//
void EditJumpToSelectionEnd(HWND hwnd)
{
  UNUSED(hwnd);
  if (!Sci_IsMultiSelection()) {
    if (SciCall_GetCurrentPos() != SciCall_GetSelectionEnd()) {
      SciCall_SwapMainAnchorCaret();
    }
  }
}


//=============================================================================
//
//  EditModifyLines()
//
void EditModifyLines(HWND hwnd, LPCWSTR pwszPrefix, LPCWSTR pwszAppend)
{
  if (!Sci_IsMultiOrRectangleSelection())
  {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECT);
    return;
  }

  char  mszPrefix1[256 * 3] = { '\0' };
  char  mszAppend1[256 * 3] = { '\0' };

  DocPos iSelStart = SciCall_GetSelectionStart();
  DocPos iSelEnd = SciCall_GetSelectionEnd();

  if (StrIsNotEmpty(pwszPrefix)) { WideCharToMultiByte(Encoding_SciCP, 0, pwszPrefix, -1, mszPrefix1, COUNTOF(mszPrefix1), NULL, NULL); }
  if (StrIsNotEmpty(pwszAppend)) { WideCharToMultiByte(Encoding_SciCP, 0, pwszAppend, -1, mszAppend1, COUNTOF(mszAppend1), NULL, NULL); }

  DocLn iLine;

  DocLn iLineStart = SciCall_LineFromPosition(iSelStart);
  DocLn iLineEnd = SciCall_LineFromPosition(iSelEnd);

  //if (iSelStart > SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineStart,0))
  //  iLineStart++;

  if (iSelEnd <= SciCall_PositionFromLine(iLineEnd))
  {
    if ((iLineEnd - iLineStart) >= 1)
      --iLineEnd;
  }

  bool  bPrefixNum = false;
  DocLn iPrefixNum = 0;
  int   iPrefixNumWidth = 1;
  DocLn iAppendNum = 0;
  int   iAppendNumWidth = 1;
  char* pszPrefixNumPad = "";
  char* pszAppendNumPad = "";
  char  mszPrefix2[256 * 3] = { '\0' };
  char  mszAppend2[256 * 3] = { '\0' };

  if (!StrIsEmptyA(mszPrefix1))
  {
    char* p = StrStrA(mszPrefix1, "$(");
    while (!bPrefixNum && p) {

      if (StrCmpNA(p, "$(I)", CSTRLEN("$(I)")) == 0) {
        *p = 0;
        StringCchCopyA(mszPrefix2, COUNTOF(mszPrefix2), p + CSTRLEN("$(I)"));
        bPrefixNum = true;
        iPrefixNum = 0;
        for (DocLn i = iLineEnd - iLineStart; i >= 10; i = i / 10)
          iPrefixNumWidth++;
        pszPrefixNumPad = "";
      }

      else if (StrCmpNA(p, "$(0I)", CSTRLEN("$(0I)")) == 0) {
        *p = 0;
        StringCchCopyA(mszPrefix2, COUNTOF(mszPrefix2), p + CSTRLEN("$(0I)"));
        bPrefixNum = true;
        iPrefixNum = 0;
        for (DocLn i = iLineEnd - iLineStart; i >= 10; i = i / 10)
          iPrefixNumWidth++;
        pszPrefixNumPad = "0";
      }

      else if (StrCmpNA(p, "$(N)", CSTRLEN("$(N)")) == 0) {
        *p = 0;
        StringCchCopyA(mszPrefix2, COUNTOF(mszPrefix2), p + CSTRLEN("$(N)"));
        bPrefixNum = true;
        iPrefixNum = 1;
        for (DocLn i = iLineEnd - iLineStart + 1; i >= 10; i = i / 10)
          iPrefixNumWidth++;
        pszPrefixNumPad = "";
      }

      else if (StrCmpNA(p, "$(0N)", CSTRLEN("$(0N)")) == 0) {
        *p = 0;
        StringCchCopyA(mszPrefix2, COUNTOF(mszPrefix2), p + CSTRLEN("$(0N)"));
        bPrefixNum = true;
        iPrefixNum = 1;
        for (DocLn i = iLineEnd - iLineStart + 1; i >= 10; i = i / 10)
          iPrefixNumWidth++;
        pszPrefixNumPad = "0";
      }

      else if (StrCmpNA(p, "$(L)", CSTRLEN("$(L)")) == 0) {
        *p = 0;
        StringCchCopyA(mszPrefix2, COUNTOF(mszPrefix2), p + CSTRLEN("$(L)"));
        bPrefixNum = true;
        iPrefixNum = iLineStart + 1;
        for (DocLn i = iLineEnd + 1; i >= 10; i = i / 10)
          iPrefixNumWidth++;
        pszPrefixNumPad = "";
      }

      else if (StrCmpNA(p, "$(0L)", CSTRLEN("$(0L)")) == 0) {
        *p = 0;
        StringCchCopyA(mszPrefix2, COUNTOF(mszPrefix2), p + CSTRLEN("$(0L)"));
        bPrefixNum = true;
        iPrefixNum = iLineStart + 1;
        for (DocLn i = iLineEnd + 1; i >= 10; i = i / 10)
          iPrefixNumWidth++;
        pszPrefixNumPad = "0";
      }
      p += CSTRLEN("$(");
      p = StrStrA(p, "$("); // next
    }
  }

  bool  bAppendNum = false;

  if (!StrIsEmptyA(mszAppend1))
  {
    char* p = StrStrA(mszAppend1, "$(");
    while (!bAppendNum && p) {

      if (StrCmpNA(p, "$(I)", CSTRLEN("$(I)")) == 0) {
        *p = 0;
        StringCchCopyA(mszAppend2, COUNTOF(mszAppend2), p + CSTRLEN("$(I)"));
        bAppendNum = true;
        iAppendNum = 0;
        for (DocLn i = iLineEnd - iLineStart; i >= 10; i = i / 10)
          iAppendNumWidth++;
        pszAppendNumPad = "";
      }

      else if (StrCmpNA(p, "$(0I)", CSTRLEN("$(0I)")) == 0) {
        *p = 0;
        StringCchCopyA(mszAppend2, COUNTOF(mszAppend2), p + CSTRLEN("$(0I)"));
        bAppendNum = true;
        iAppendNum = 0;
        for (DocLn i = iLineEnd - iLineStart; i >= 10; i = i / 10)
          iAppendNumWidth++;
        pszAppendNumPad = "0";
      }

      else if (StrCmpNA(p, "$(N)", CSTRLEN("$(N)")) == 0) {
        *p = 0;
        StringCchCopyA(mszAppend2, COUNTOF(mszAppend2), p + CSTRLEN("$(N)"));
        bAppendNum = true;
        iAppendNum = 1;
        for (DocLn i = iLineEnd - iLineStart + 1; i >= 10; i = i / 10)
          iAppendNumWidth++;
        pszAppendNumPad = "";
      }

      else if (StrCmpNA(p, "$(0N)", CSTRLEN("$(0N)")) == 0) {
        *p = 0;
        StringCchCopyA(mszAppend2, COUNTOF(mszAppend2), p + CSTRLEN("$(0N)"));
        bAppendNum = true;
        iAppendNum = 1;
        for (DocLn i = iLineEnd - iLineStart + 1; i >= 10; i = i / 10)
          iAppendNumWidth++;
        pszAppendNumPad = "0";
      }

      else if (StrCmpNA(p, "$(L)", CSTRLEN("$(L)")) == 0) {
        *p = 0;
        StringCchCopyA(mszAppend2, COUNTOF(mszAppend2), p + CSTRLEN("$(L)"));
        bAppendNum = true;
        iAppendNum = iLineStart + 1;
        for (DocLn i = iLineEnd + 1; i >= 10; i = i / 10)
          iAppendNumWidth++;
        pszAppendNumPad = "";
      }

      else if (StrCmpNA(p, "$(0L)", CSTRLEN("$(0L)")) == 0) {
        *p = 0;
        StringCchCopyA(mszAppend2, COUNTOF(mszAppend2), p + CSTRLEN("$(0L)"));
        bAppendNum = true;
        iAppendNum = iLineStart + 1;
        for (DocLn i = iLineEnd + 1; i >= 10; i = i / 10)
          iAppendNumWidth++;
        pszAppendNumPad = "0";
      }
      p += CSTRLEN("$(");
      p = StrStrA(p, "$("); // next
    }
  }

  _IGNORE_NOTIFY_CHANGE_;

  DocPos const saveTargetBeg = SciCall_GetTargetStart();
  DocPos const saveTargetEnd = SciCall_GetTargetEnd();

  for (iLine = iLineStart; iLine <= iLineEnd; iLine++)
  {
    DocPos iPos;

    if (StrIsNotEmpty(pwszPrefix)) {

      char mszInsert[512 * 3] = { '\0' };
      StringCchCopyA(mszInsert, COUNTOF(mszInsert), mszPrefix1);

      if (bPrefixNum) {
        char tchFmt[64] = { '\0' };
        char tchNum[64] = { '\0' };
        StringCchPrintfA(tchFmt, COUNTOF(tchFmt), "%%%s%ii", pszPrefixNumPad, iPrefixNumWidth);
        StringCchPrintfA(tchNum, COUNTOF(tchNum), tchFmt, iPrefixNum);
        StringCchCatA(mszInsert, COUNTOF(mszInsert), tchNum);
        StringCchCatA(mszInsert, COUNTOF(mszInsert), mszPrefix2);
        iPrefixNum++;
      }
      iPos = SciCall_PositionFromLine(iLine);
      SciCall_SetTargetRange(iPos, iPos);
      SciCall_ReplaceTarget(-1, mszInsert);
    }

    if (StrIsNotEmpty(pwszAppend)) {

      char mszInsert[512 * 3] = { '\0' };
      StringCchCopyA(mszInsert, COUNTOF(mszInsert), mszAppend1);

      if (bAppendNum) {
        char tchFmt[64] = { '\0' };
        char tchNum[64] = { '\0' };
        StringCchPrintfA(tchFmt, COUNTOF(tchFmt), "%%%s%ii", pszAppendNumPad, iAppendNumWidth);
        StringCchPrintfA(tchNum, COUNTOF(tchNum), tchFmt, iAppendNum);
        StringCchCatA(mszInsert, COUNTOF(mszInsert), tchNum);
        StringCchCatA(mszInsert, COUNTOF(mszInsert), mszAppend2);
        iAppendNum++;
      }
      iPos = SciCall_GetLineEndPosition(iLine);
      SciCall_SetTargetRange(iPos, iPos);
      SciCall_ReplaceTarget(-1, mszInsert);
    }
  }

  SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore
  _OBSERVE_NOTIFY_CHANGE_;

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
    EditSetSelectionEx(hwnd, iAnchorPos, iCurPos, -1, -1);
  }

}


//=============================================================================
//
//  EditIndentBlock()
//
void EditIndentBlock(HWND hwnd, int cmd, bool bFormatIndentation, bool bForceAll)
{
  if ((cmd != SCI_TAB) && (cmd != SCI_BACKTAB)) {
    SendMessage(hwnd, cmd, 0, 0);
    return;
  }
  if (!bForceAll && Sci_IsMultiOrRectangleSelection())
  {
    SendMessage(hwnd, cmd, 0, 0);
    return;
  }

  DocPos const iInitialPos = SciCall_GetCurrentPos();
  if (bForceAll) { SciCall_SelectAll(); }

  DocPos const iCurPos = SciCall_GetCurrentPos();
  DocPos const iAnchorPos = SciCall_GetAnchor();

  DocLn const iCurLine = SciCall_LineFromPosition(iCurPos);
  DocLn const iAnchorLine = SciCall_LineFromPosition(iAnchorPos);
  bool const bSingleLine = Sci_IsSingleLineSelection();

  bool const _bTabIndents = SciCall_GetTabIndents();
  bool const _bBSpUnindents = SciCall_GetBackSpaceUnIndents();

  DocPos iDiffCurrent = 0;
  DocPos iDiffAnchor = 0;
  bool bFixStart = false;

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
    SciCall_SetTabIndents(bFormatIndentation ? true : _bTabIndents);
    SendMessage(hwnd, SCI_TAB, 0, 0);
    if (bFormatIndentation) {
      SciCall_SetTabIndents(_bTabIndents);
    }
  }
  else  // SCI_BACKTAB
  {
    SciCall_SetBackSpaceUnIndents(bFormatIndentation ? true : _bBSpUnindents);
    SendMessage(hwnd, SCI_BACKTAB, 0, 0);
    if (bFormatIndentation) {
      SciCall_SetBackSpaceUnIndents(_bBSpUnindents);
    }
  }

  if (!bForceAll) {
    if (bSingleLine) {
      if (bFormatIndentation) {
        EditSetSelectionEx(hwnd, SciCall_GetCurrentPos() + iDiffCurrent + (iAnchorPos - iCurPos), SciCall_GetCurrentPos() + iDiffCurrent, -1, -1);
      }
    }
    else {  // on multiline indentation, anchor and current positions are moved to line begin resp. end
      if (bFixStart) {
        if (iCurPos < iAnchorPos)
          iDiffCurrent = SciCall_LineLength(iCurLine) - Sci_GetEOLLen();
        else
          iDiffAnchor = SciCall_LineLength(iAnchorLine) - Sci_GetEOLLen();
      }
      EditSetSelectionEx(hwnd, SciCall_GetLineEndPosition(iAnchorLine) - iDiffAnchor, SciCall_GetLineEndPosition(iCurLine) - iDiffCurrent, -1, -1);
    }
  }
  else {
    EditSetSelectionEx(hwnd, iInitialPos, iInitialPos, -1, -1);
  }
}


//=============================================================================
//
//  EditAlignText()
//
void EditAlignText(HWND hwnd, int nMode)
{
  DocPos iCurPos = SciCall_GetCurrentPos();
  DocPos iAnchorPos = SciCall_GetAnchor();

  DocPos const iSelStart = SciCall_GetSelectionStart();
  DocPos const iSelEnd = SciCall_GetSelectionEnd();

  DocLn const iLineStart = SciCall_LineFromPosition(iSelStart);
  DocLn const _lnend = SciCall_LineFromPosition(iSelEnd);
  DocLn const iLineEnd = (iSelEnd <= SciCall_PositionFromLine(_lnend)) ? (_lnend - 1) : _lnend;

  DocPos const iCurCol = SciCall_GetColumn(iCurPos);
  DocPos const iAnchorCol = SciCall_GetColumn(iAnchorPos);

  if (Sci_IsMultiOrRectangleSelection()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECT);
    return;
  }
  if (iLineEnd <= iLineStart) { return; }

  int iMinIndent = INT_MAX;
  DocPos iMaxLength = 0;

  for (DocLn iLine = iLineStart; iLine <= iLineEnd; iLine++) {

    DocPos iLineEndPos = SciCall_GetLineEndPosition(iLine);
    const DocPos iLineIndentPos = SciCall_GetLineIndentPosition(iLine);

    if (iLineIndentPos != iLineEndPos) {
      int const iIndentCol = SciCall_GetLineIndentation(iLine);
      DocPos iTail = iLineEndPos - 1;
      char ch = SciCall_GetCharAt(iTail);
      while (iTail >= iLineStart && (ch == ' ' || ch == '\t')) {
        --iTail;
        ch = SciCall_GetCharAt(iTail);
        --iLineEndPos;
      }
      const DocPos iEndCol = SciCall_GetColumn(iLineEndPos);

      iMinIndent = min_i(iMinIndent, iIndentCol);
      iMaxLength = max_p(iMaxLength, iEndCol);
    }
  }

  size_t const iBufCount = (iMaxLength + 3) * 3;
  char* chNewLineBuf = AllocMem(iBufCount, HEAP_ZERO_MEMORY);
  WCHAR* wchLineBuf = AllocMem(iBufCount * sizeof(WCHAR), HEAP_ZERO_MEMORY);
  WCHAR* wchNewLineBuf = AllocMem(iBufCount * sizeof(WCHAR), HEAP_ZERO_MEMORY);
  PWCHAR* pWords = (PWCHAR*)AllocMem(iBufCount * sizeof(PWCHAR), HEAP_ZERO_MEMORY);

  if (chNewLineBuf && wchLineBuf && wchNewLineBuf) {

    _IGNORE_NOTIFY_CHANGE_;
    DocPos const saveTargetBeg = SciCall_GetTargetStart();
    DocPos const saveTargetEnd = SciCall_GetTargetEnd();

    for (DocLn iLine = iLineStart; iLine <= iLineEnd; iLine++) {
      DocPos const iStartPos = SciCall_PositionFromLine(iLine);
      DocPos const iEndPos = SciCall_GetLineEndPosition(iLine);
      DocPos const iIndentPos = SciCall_GetLineIndentPosition(iLine);

      if ((iIndentPos == iEndPos) && (iEndPos > 0)) {
        SciCall_SetTargetRange(iStartPos, iEndPos);
        SciCall_ReplaceTarget(0, "");
      }
      else {
        int iWords = 0;
        int iWordsLength = 0;
        int const cchLine = (MBWC_DocPos_Cast)SciCall_LineLength(iLine);
        int const cwch = MultiByteToWideChar(Encoding_SciCP, 0,
                                             SciCall_GetRangePointer(iStartPos, cchLine),
                                             cchLine, wchLineBuf, (MBWC_DocPos_Cast)iBufCount);
        wchLineBuf[cwch] = L'\0';
        StrTrim(wchLineBuf, L"\r\n\t ");

        WCHAR* p = wchLineBuf;
        while (*p) {
          if ((*p != L' ') && (*p != L'\t')) {
            pWords[iWords++] = p++;
            iWordsLength++;
            while (*p && (*p != L' ') && (*p != L'\t')) {
              p++;
              iWordsLength++;
            }
          }
          else
            *p++ = L'\0';
        }

        if (iWords > 0) {

          if (nMode == ALIGN_JUSTIFY || nMode == ALIGN_JUSTIFY_EX) {

            bool bNextLineIsBlank = false;
            if (nMode == ALIGN_JUSTIFY_EX) {
              if (SciCall_GetLineCount() <= iLine + 1) {
                bNextLineIsBlank = true;
              }
              else {
                DocPos const iLineEndPos = SciCall_GetLineEndPosition(iLine + 1);
                DocPos const iLineIndentPos = SciCall_GetLineIndentPosition(iLine + 1);
                if (iLineIndentPos == iLineEndPos) {
                  bNextLineIsBlank = true;
                }
              }
            }

            if ((nMode == ALIGN_JUSTIFY || nMode == ALIGN_JUSTIFY_EX) &&
                iWords > 1 && iWordsLength >= 2 &&
                ((nMode != ALIGN_JUSTIFY_EX || !bNextLineIsBlank || iLineStart == iLineEnd) ||
                (bNextLineIsBlank && iWordsLength > (iMaxLength - iMinIndent) * 0.75))) {
              int iGaps = iWords - 1;
              DocPos const iSpacesPerGap = (iMaxLength - iMinIndent - iWordsLength) / iGaps;
              DocPos const iExtraSpaces = (iMaxLength - iMinIndent - iWordsLength) % iGaps;

              DocPos const length = iMaxLength * 3;
              StringCchCopy(wchNewLineBuf, iBufCount, pWords[0]);
              p = StrEnd(wchNewLineBuf, iBufCount);

              for (int i = 1; i < iWords; i++) {
                for (int j = 0; j < iSpacesPerGap; j++) {
                  *p++ = L' ';
                  *p = 0;
                }
                if (i > iGaps - iExtraSpaces) {
                  *p++ = L' ';
                  *p = 0;
                }
                StringCchCat(p, (length - StringCchLenW(wchNewLineBuf, iBufCount)), pWords[i]);
                p = StrEnd(p, 0);
              }
            }
            else {
              StringCchCopy(wchNewLineBuf, iBufCount, pWords[0]);
              p = StrEnd(wchNewLineBuf, iBufCount);

              for (int i = 1; i < iWords; i++) {
                *p++ = L' ';
                *p = 0;
                StringCchCat(p, (iBufCount - StringCchLenW(wchNewLineBuf, iBufCount)), pWords[i]);
                p = StrEnd(p, 0);
              }
            }


            int const cch = WideCharToMultiByte(Encoding_SciCP, 0, wchNewLineBuf, -1, chNewLineBuf, (int)iBufCount, NULL, NULL) - 1;
            SciCall_SetTargetRange(SciCall_PositionFromLine(iLine), SciCall_GetLineEndPosition(iLine));
            SciCall_ReplaceTarget(cch, chNewLineBuf);
            SciCall_SetLineIndentation(iLine, iMinIndent);
          }
          else {
            chNewLineBuf[0] = '\0';
            wchNewLineBuf[0] = L'\0';
            p = wchNewLineBuf;

            DocPos const iExtraSpaces = iMaxLength - iMinIndent - iWordsLength - iWords + 1;
            if (nMode == ALIGN_RIGHT) {
              for (int i = 0; i < iExtraSpaces; i++)
                *p++ = L' ';
              *p = 0;
            }

            DocPos iOddSpaces = iExtraSpaces % 2;
            if (nMode == ALIGN_CENTER) {
              for (int i = 1; i < iExtraSpaces - iOddSpaces; i += 2)
                *p++ = L' ';
              *p = 0;
            }
            for (int i = 0; i < iWords; i++) {
              StringCchCat(p, (iBufCount - StringCchLenW(wchNewLineBuf, iBufCount)), pWords[i]);
              if (i < iWords - 1)
                StringCchCat(p, (iBufCount - StringCchLenW(wchNewLineBuf, iBufCount)), L" ");
              if (nMode == ALIGN_CENTER && iWords > 1 && iOddSpaces > 0 && i + 1 >= iWords / 2) {
                StringCchCat(p, (iBufCount - StringCchLenW(wchNewLineBuf, iBufCount)), L" ");
                iOddSpaces--;
              }
              p = StrEnd(p, 0);
            }

            int const cch = WideCharToMultiByte(Encoding_SciCP, 0, wchNewLineBuf, -1,
                                                chNewLineBuf, (MBWC_DocPos_Cast)iBufCount, NULL, NULL) - 1;

            if (cch >= 0) {
              DocPos iPos = 0;
              if (nMode == ALIGN_RIGHT || nMode == ALIGN_CENTER) {
                SciCall_SetLineIndentation(iLine, iMinIndent);
                iPos = SciCall_GetLineIndentPosition(iLine);
              }
              else {
                iPos = SciCall_PositionFromLine(iLine);
              }
              SciCall_SetTargetRange(iPos, SciCall_GetLineEndPosition(iLine));
              SciCall_ReplaceTarget(cch, chNewLineBuf);

              if (nMode == ALIGN_LEFT) {
                SciCall_SetLineIndentation(iLine, iMinIndent);
              }
            }
          }
        }
      }
    }
    SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore
    _OBSERVE_NOTIFY_CHANGE_;

    FreeMem(pWords);
    FreeMem(wchNewLineBuf);
    FreeMem(wchLineBuf);
    FreeMem(chNewLineBuf);
  }
  else {
    InfoBoxLng(MB_ICONERROR, NULL, IDS_MUI_BUFFERTOOSMALL);
  }

  if (iAnchorPos > iCurPos) {
    iCurPos = SciCall_FindColumn(iLineStart, iCurCol);
    iAnchorPos = SciCall_FindColumn(_lnend, iAnchorCol);
  }
  else {
    iAnchorPos = SciCall_FindColumn(iLineStart, iAnchorCol);
    iCurPos = SciCall_FindColumn(_lnend, iCurCol);
  }

  EditSetSelectionEx(hwnd, iAnchorPos, iCurPos, -1, -1);
}



//=============================================================================
//
//  EditEncloseSelection()
//
void EditEncloseSelection(HWND hwnd, LPCWSTR pwszOpen, LPCWSTR pwszClose)
{
  if (Sci_IsMultiOrRectangleSelection()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECT);
    return;
  }

  char  mszOpen[256 * 3] = { '\0' };
  char  mszClose[256 * 3] = { '\0' };

  const DocPos iCurPos = SciCall_GetCurrentPos();
  const DocPos iAnchorPos = SciCall_GetAnchor();
  const DocPos iSelStart = SciCall_GetSelectionStart();
  const DocPos iSelEnd = SciCall_GetSelectionEnd();

  if (StrIsNotEmpty(pwszOpen)) { WideCharToMultiByte(Encoding_SciCP, 0, pwszOpen, -1, mszOpen, COUNTOF(mszOpen), NULL, NULL); }
  if (StrIsNotEmpty(pwszClose)) { WideCharToMultiByte(Encoding_SciCP, 0, pwszClose, -1, mszClose, COUNTOF(mszClose), NULL, NULL); }

  DocPos const iLenOpen = (DocPos)StringCchLenA(mszOpen, COUNTOF(mszOpen));
  DocPos const iLenClose = (DocPos)StringCchLenA(mszClose, COUNTOF(mszClose));

  DocPos const saveTargetBeg = SciCall_GetTargetStart();
  DocPos const saveTargetEnd = SciCall_GetTargetEnd();

  if (iLenOpen > 0) {
    SciCall_SetTargetRange(iSelStart, iSelStart);
    SciCall_ReplaceTarget(-1, mszOpen);
  }

  if (iLenClose > 0) {
    SciCall_SetTargetRange(iSelEnd + iLenOpen, iSelEnd + iLenOpen);
    SciCall_ReplaceTarget(-1, mszClose);
  }

  SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore

  // Fix selection
  EditSetSelectionEx(hwnd, iAnchorPos + iLenOpen, iCurPos + iLenOpen, -1, -1);
}


//=============================================================================
//
//  EditToggleLineComments()
//
void EditToggleLineComments(HWND hwnd, LPCWSTR pwszComment, bool bInsertAtStart)
{
  const DocPos iCurPos = SciCall_GetCurrentPos();
  const DocPos iAnchorPos = SciCall_GetAnchor();
  const DocPos iSelStart = SciCall_GetSelectionStart();
  const DocPos iSelEnd = SciCall_GetSelectionEnd();

  const DocPos iSelBegCol = SciCall_GetColumn(iSelStart);

  char mszComment[32 * 3] = { '\0' };

  if (StrIsNotEmpty(pwszComment)) {
    WideCharToMultiByte(Encoding_SciCP, 0, pwszComment, -1, mszComment, COUNTOF(mszComment), NULL, NULL);
  }
  DocPos const cchComment = (DocPos)StringCchLenA(mszComment, COUNTOF(mszComment));

  if (cchComment == 0) { return; }

  if (Sci_IsMultiOrRectangleSelection()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECT);
    return;
  }

  const DocLn iLineStart = SciCall_LineFromPosition(iSelStart);
  DocLn iLineEnd = SciCall_LineFromPosition(iSelEnd);

  if (iSelEnd <= SciCall_PositionFromLine(iLineEnd)) {
    if ((iLineEnd - iLineStart) >= 1)
      --iLineEnd;
  }

  DocPos iCommentCol = 0;

  if (!bInsertAtStart) {
    iCommentCol = (DocPos)INT_MAX;
    for (DocLn iLine = iLineStart; iLine <= iLineEnd; iLine++)
    {
      const DocPos iLineEndPos = SciCall_GetLineEndPosition(iLine);
      const DocPos iLineIndentPos = SciCall_GetLineIndentPosition(iLine);
      if (iLineIndentPos != iLineEndPos) {
        const DocPos iIndentColumn = SciCall_GetColumn(iLineIndentPos);
        iCommentCol = min_p(iCommentCol, iIndentColumn);
      }
    }
  }

  DocPos iSelStartOffset = (iCommentCol >= iSelBegCol) ? 0 : cchComment;
  DocPos iSelEndOffset = 0;


  _IGNORE_NOTIFY_CHANGE_;
  DocPos const saveTargetBeg = SciCall_GetTargetStart();
  DocPos const saveTargetEnd = SciCall_GetTargetEnd();

  int iAction = 0;

  for (DocLn iLine = iLineStart; iLine <= iLineEnd; iLine++)
  {
    const DocPos iIndentPos = SciCall_GetLineIndentPosition(iLine);

    if (iIndentPos == SciCall_GetLineEndPosition(iLine)) {
      // don't set comment char on "empty" (white-space only) lines
      //~iAction = 1;
      continue;
    }

    const char* tchBuf = SciCall_GetRangePointer(iIndentPos, cchComment + 1);
    if (StrCmpNIA(tchBuf, mszComment, (int)cchComment) == 0) 
    {
      // remove comment chars
      switch (iAction) {
      case 0:
        iAction = 2;
      case 2:
        SciCall_SetTargetRange(iIndentPos, iIndentPos + cchComment);
        SciCall_ReplaceTarget(0, "");
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
      // set comment chars at indent pos
      switch (iAction) {
      case 0:
        iAction = 1;
      case 1:
        {
          SciCall_InsertText(SciCall_FindColumn(iLine, iCommentCol), mszComment);
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

  SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore
  _OBSERVE_NOTIFY_CHANGE_;

  if (iCurPos < iAnchorPos)
    EditSetSelectionEx(hwnd, iAnchorPos + iSelEndOffset, iCurPos + iSelStartOffset, -1, -1);
  else if (iCurPos > iAnchorPos)
    EditSetSelectionEx(hwnd, iAnchorPos + iSelStartOffset, iCurPos + iSelEndOffset, -1, -1);
  else
    EditSetSelectionEx(hwnd, iAnchorPos + iSelStartOffset, iCurPos + iSelStartOffset, -1, -1);
}


//=============================================================================
//
//  _AppendSpaces()
//
static DocPos  _AppendSpaces(HWND hwnd, DocLn iLineStart, DocLn iLineEnd, DocPos iMaxColumn, bool bSkipEmpty)
{
  UNUSED(hwnd);

  size_t size = (size_t)iMaxColumn;
  char* pmszPadStr = AllocMem(size + 1, HEAP_ZERO_MEMORY);
  FillMemory(pmszPadStr, size, ' ');

  DocPos spcCount = 0;

  _IGNORE_NOTIFY_CHANGE_;
  DocPos const saveTargetBeg = SciCall_GetTargetStart();
  DocPos const saveTargetEnd = SciCall_GetTargetEnd();

  for (DocLn iLine = iLineStart; iLine <= iLineEnd; ++iLine) {

    // insertion position is at end of line
    const DocPos iPos = SciCall_GetLineEndPosition(iLine); 
    const DocPos iCol = SciCall_GetColumn(iPos);

    if (iCol >= iMaxColumn) { continue; }
    if (bSkipEmpty && (iPos <= SciCall_PositionFromLine(iLine))) { continue; }

    const DocPos iPadLen = (iMaxColumn - iCol);

    pmszPadStr[iPadLen] = '\0'; // slice

    SciCall_SetTargetRange(iPos, iPos);
    SciCall_ReplaceTarget(-1, pmszPadStr); // pad

    pmszPadStr[iPadLen] = ' '; // reset
    spcCount += iPadLen;
  }

  SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore
  _OBSERVE_NOTIFY_CHANGE_;

  FreeMem(pmszPadStr);
  
  return spcCount;
}

//=============================================================================
//
//  EditPadWithSpaces()
//
void EditPadWithSpaces(HWND hwnd, bool bSkipEmpty, bool bNoUndoGroup)
{
  if (SciCall_IsSelectionEmpty()) { return; }

  int const token = (!bNoUndoGroup ? BeginUndoAction() : -1);

  if (Sci_IsMultiOrRectangleSelection() && !SciCall_IsSelectionEmpty())
  {
    DocPos const selAnchorMainPos = SciCall_GetRectangularSelectionAnchor();
    DocPos const selCaretMainPos = SciCall_GetRectangularSelectionCaret();

    DocPos const iAnchorColumn = SciCall_GetColumn(SciCall_GetSelectionNAnchor(0)) + SciCall_GetSelectionNAnchorVirtualSpace(0);
    DocPos const iCaretColumn = SciCall_GetColumn(SciCall_GetSelectionNCaret(0)) + SciCall_GetSelectionNCaretVirtualSpace(0);
    bool const bSelLeft2Right = (iAnchorColumn <= iCaretColumn);

    DocLn iRcAnchorLine = SciCall_LineFromPosition(selAnchorMainPos);
    DocLn iRcCaretLine = SciCall_LineFromPosition(selCaretMainPos);
    DocLn const iLineCount = abs_p(iRcCaretLine - iRcAnchorLine) + 1;

    // lots of spaces
    DocPos const spBufSize = max_p(iAnchorColumn, selCaretMainPos);
    char* pSpaceBuffer = (char*)AllocMem((spBufSize + 1) * sizeof(char), HEAP_ZERO_MEMORY);
    FillMemory(pSpaceBuffer, spBufSize * sizeof(char), ' ');

    DocPos* pVspAVec = (DocPos*)AllocMem(iLineCount * sizeof(DocPos), HEAP_ZERO_MEMORY);
    DocPos* pVspCVec = (DocPos*)AllocMem(iLineCount * sizeof(DocPos), HEAP_ZERO_MEMORY);

    for (DocLn i = 0; i < iLineCount; ++i) {
      pVspAVec[i] = SciCall_GetSelectionNAnchorVirtualSpace(i); 
      pVspCVec[i] = SciCall_GetSelectionNCaretVirtualSpace(i);
    }

    DocPosU i = 0;
    DocPos iSpcCount = 0;
    DocLn const iLnIncr = (iRcAnchorLine <= iRcCaretLine) ? (DocLn)+1 : (DocLn)-1;
    DocLn iLine = iRcAnchorLine - iLnIncr;
    do {
      iLine += iLnIncr;
      DocPos const iInsPos = SciCall_GetLineEndPosition(iLine);
      DocPos const cntVSp = bSelLeft2Right ? pVspCVec[i++] : pVspAVec[i++];
      bool const bSkip = (bSkipEmpty && (iInsPos <= SciCall_PositionFromLine(iLine)));

      if ((cntVSp > 0) && !bSkip) {
        pSpaceBuffer[cntVSp] = '\0';
        SciCall_InsertText(iInsPos, pSpaceBuffer);
        pSpaceBuffer[cntVSp] = ' ';
        iSpcCount += cntVSp;
      }
    } while (iLine != iRcCaretLine);

    FreeMem(pSpaceBuffer);

    if (iRcAnchorLine <= iRcCaretLine) {
      if (bSelLeft2Right)
        EditSetSelectionEx(hwnd, selAnchorMainPos + pVspAVec[0], selCaretMainPos + iSpcCount, 0, 0);
      else
        EditSetSelectionEx(hwnd, selAnchorMainPos + pVspAVec[0], selCaretMainPos + pVspCVec[iLineCount - 1] + iSpcCount - pVspAVec[iLineCount - 1], 0, 0);
    }
    else {
      if (bSelLeft2Right)
        EditSetSelectionEx(hwnd, selAnchorMainPos + pVspAVec[0] + iSpcCount - pVspCVec[0], selCaretMainPos + pVspCVec[iLineCount - 1], 0, 0);
      else
        EditSetSelectionEx(hwnd, selAnchorMainPos + iSpcCount, selCaretMainPos + pVspCVec[iLineCount - 1], 0, 0);
    }

    FreeMem(pVspCVec);
    FreeMem(pVspAVec);
  }
  else  // SC_SEL_LINES | SC_SEL_STREAM
  {
    const DocPos iCurPos = SciCall_GetCurrentPos();
    const DocPos iAnchorPos = SciCall_GetAnchor();

    const DocPos iSelStart = SciCall_GetSelectionStart();
    const DocPos iSelEnd = SciCall_GetSelectionEnd();

    DocLn iStartLine = 0;
    DocLn iEndLine = SciCall_GetLineCount() - 1;

    if (iSelStart != iSelEnd) {
      iStartLine = SciCall_LineFromPosition(iSelStart);
      iEndLine = SciCall_LineFromPosition(iSelEnd);
      if (iSelEnd < SciCall_GetLineEndPosition(iEndLine)) { --iEndLine; }
      if (iEndLine <= iStartLine) { return; }
    }

    DocPos iMaxColumn = 0;
    for (DocLn iLine = iStartLine; iLine <= iEndLine; ++iLine) {
      iMaxColumn = max_p(iMaxColumn, SciCall_GetColumn(SciCall_GetLineEndPosition(iLine)));
    }
    if (iMaxColumn <= 0) { return; }

    const DocPos iSpcCount = _AppendSpaces(hwnd, iStartLine, iEndLine, iMaxColumn, bSkipEmpty);

    if (iCurPos < iAnchorPos)
      EditSetSelectionEx(hwnd, iAnchorPos + iSpcCount, iCurPos, -1, -1);
    else
      EditSetSelectionEx(hwnd, iAnchorPos, iCurPos + iSpcCount, -1, -1);
  }

  if (token >= 0) { EndUndoAction(token); }
}


//=============================================================================
//
//  EditStripFirstCharacter()
//
void EditStripFirstCharacter(HWND hwnd)
{
  UNUSED(hwnd);

  DocPos const iSelStart = SciCall_IsSelectionEmpty() ? 0 : SciCall_GetSelectionStart();
  DocPos const iSelEnd = SciCall_IsSelectionEmpty() ? Sci_GetDocEndPosition() : SciCall_GetSelectionEnd();
  DocLn const iLineStart = SciCall_LineFromPosition(iSelStart);
  DocLn const iLineEnd = SciCall_LineFromPosition(iSelEnd);

  _IGNORE_NOTIFY_CHANGE_;
  DocPos const saveTargetBeg = SciCall_GetTargetStart();
  DocPos const saveTargetEnd = SciCall_GetTargetEnd();

  if (SciCall_IsSelectionRectangle()) 
  {
    if (SciCall_IsSelectionEmpty()) {
      SciCall_Clear();
    }
    else {
      const DocPos selAnchorMainPos = SciCall_GetRectangularSelectionAnchor();
      const DocPos selCaretMainPos = SciCall_GetRectangularSelectionCaret();
      const DocPos vSpcAnchorMainPos = SciCall_GetRectangularSelectionAnchorVirtualSpace();
      const DocPos vSpcCaretMainPos = SciCall_GetRectangularSelectionCaretVirtualSpace();

      DocPos iMaxLineLen = Sci_GetRangeMaxLineLength(iLineStart, iLineEnd);
      char* lineBuffer = AllocMem(iMaxLineLen + 1, HEAP_ZERO_MEMORY);
      DocPos remCount = 0;
      if (lineBuffer) {
        DocPosU const selCount = SciCall_GetSelections();
        for (DocPosU s = 0; s < selCount; ++s) 
        {
          DocPos const selTargetStart = SciCall_GetSelectionNStart(s);
          DocPos const selTargetEnd = SciCall_GetSelectionNEnd(s);
          DocPos const nextPos = SciCall_PositionAfter(selTargetStart);
          DocPos const len = (selTargetEnd - nextPos);
          if (len > 0) {
            StringCchCopyNA(lineBuffer, iMaxLineLen, SciCall_GetRangePointer(nextPos, len + 1), len);
            SciCall_SetTargetRange(selTargetStart, selTargetEnd);
            SciCall_ReplaceTarget(len, lineBuffer);
          }
          remCount += (nextPos - selTargetStart);
        } // for()
        FreeMem(lineBuffer);
      }
      
      SciCall_SetRectangularSelectionAnchor(selAnchorMainPos);
      if (vSpcAnchorMainPos > 0)
        SciCall_SetRectangularSelectionAnchorVirtualSpace(vSpcAnchorMainPos);

      SciCall_SetRectangularSelectionCaret(selCaretMainPos - remCount);
      if (vSpcCaretMainPos > 0)
        SciCall_SetRectangularSelectionCaretVirtualSpace(vSpcCaretMainPos);
    }
  }
  else if (Sci_IsMultiSelection()) {
    // not implemented
  }
  else // SC_SEL_LINES | SC_SEL_STREAM
  {
    for (DocLn iLine = iLineStart; iLine <= iLineEnd; ++iLine) {
      const DocPos iPos = SciCall_PositionFromLine(iLine);
      if (iPos < SciCall_GetLineEndPosition(iLine)) {
        SciCall_SetTargetRange(iPos, SciCall_PositionAfter(iPos));
        SciCall_ReplaceTarget(0, "");
      }
    }
  }
  SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore
  _OBSERVE_NOTIFY_CHANGE_;
}


//=============================================================================
//
//  EditStripLastCharacter()
//
void EditStripLastCharacter(HWND hwnd, bool bIgnoreSelection, bool bTrailingBlanksOnly)
{
  UNUSED(hwnd);

  DocPos const iSelStart = (SciCall_IsSelectionEmpty() || bIgnoreSelection) ? 0 : SciCall_GetSelectionStart();
  DocPos const iSelEnd = (SciCall_IsSelectionEmpty() || bIgnoreSelection) ? Sci_GetDocEndPosition() : SciCall_GetSelectionEnd();
  DocLn const iLineStart = SciCall_LineFromPosition(iSelStart);
  DocLn const iLineEnd = SciCall_LineFromPosition(iSelEnd);

  _IGNORE_NOTIFY_CHANGE_;
  DocPos const saveTargetBeg = SciCall_GetTargetStart();
  DocPos const saveTargetEnd = SciCall_GetTargetEnd();

  if (Sci_IsMultiOrRectangleSelection() && !bIgnoreSelection)
  {
    if (SciCall_IsSelectionEmpty()) {
      SciCall_Clear();
    }
    else {
      const DocPos selAnchorMainPos = SciCall_GetRectangularSelectionAnchor();
      const DocPos selCaretMainPos = SciCall_GetRectangularSelectionCaret();
      const DocPos vSpcAnchorMainPos = SciCall_GetRectangularSelectionAnchorVirtualSpace();
      const DocPos vSpcCaretMainPos = SciCall_GetRectangularSelectionCaretVirtualSpace();

      DocPos iMaxLineLen = Sci_GetRangeMaxLineLength(iLineStart, iLineEnd);
      char* lineBuffer = AllocMem(iMaxLineLen + 1, HEAP_ZERO_MEMORY);
      DocPos remCount = 0;
      if (lineBuffer) {
        const DocPosU selCount = SciCall_GetSelections();
        for (DocPosU s = 0; s < selCount; ++s) 
        {
          DocPos const selTargetStart = SciCall_GetSelectionNStart(s);
          DocPos const selTargetEnd = SciCall_GetSelectionNEnd(s);

          DocPos diff = 0;
          DocPos len = 0;
          if (bTrailingBlanksOnly) {
            len = (selTargetEnd - selTargetStart);
            if (len > 0) {
              StringCchCopyNA(lineBuffer, iMaxLineLen, SciCall_GetRangePointer(selTargetStart, len + 1), len);
              DocPos end = (DocPos)StrCSpnA(lineBuffer, "\r\n");
              DocPos i = end;
              while (--i >= 0) {
                const char ch = lineBuffer[i];
                if (IsBlankChar(ch)) {
                  lineBuffer[i] = '\0';
                }
                else
                  break;
              }
              while (end < len) {
                lineBuffer[++i] = lineBuffer[end++];  // add "\r\n" if any
              }
              diff = len - (++i);
              SciCall_SetTargetRange(selTargetStart, selTargetEnd);
              SciCall_ReplaceTarget(-1, lineBuffer);
            }
          }
          else {
            DocPos const prevPos = SciCall_PositionBefore(selTargetEnd);
            diff = (selTargetEnd - prevPos);
            len = (prevPos - selTargetStart);
            if (len > 0) {
              StringCchCopyNA(lineBuffer, iMaxLineLen, SciCall_GetRangePointer(selTargetStart, len + 1), len);
              SciCall_SetTargetRange(selTargetStart, selTargetEnd);
              SciCall_ReplaceTarget(len, lineBuffer);
            }
          }
          remCount += diff;
        } // for()
        FreeMem(lineBuffer);
      }

      SciCall_SetRectangularSelectionAnchor(selAnchorMainPos);
      if (vSpcAnchorMainPos > 0)
        SciCall_SetRectangularSelectionAnchorVirtualSpace(vSpcAnchorMainPos);

      SciCall_SetRectangularSelectionCaret(selCaretMainPos - remCount);
      if (vSpcCaretMainPos > 0)
        SciCall_SetRectangularSelectionCaretVirtualSpace(vSpcCaretMainPos);
    }
  }
  else  // SC_SEL_LINES | SC_SEL_STREAM
  {
    for (DocLn iLine = iLineStart; iLine <= iLineEnd; ++iLine)
    {
      DocPos const iStartPos = SciCall_PositionFromLine(iLine);
      DocPos const iEndPos = SciCall_GetLineEndPosition(iLine);

      if (bTrailingBlanksOnly)
      {
        DocPos i = iEndPos;
        char ch;
        do {
          ch = SciCall_GetCharAt(--i);
        } while ((i >= iStartPos) && IsBlankChar(ch));
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
  }
  SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore
  _OBSERVE_NOTIFY_CHANGE_;
}


//=============================================================================
//
//  EditCompressSpaces()
//
void EditCompressBlanks(HWND hwnd)
{
  const bool bIsSelEmpty = SciCall_IsSelectionEmpty();

  const DocPos iSelStartPos = SciCall_GetSelectionStart();
  const DocPos iSelEndPos = SciCall_GetSelectionEnd();
  const DocLn iLineStart = SciCall_LineFromPosition(iSelStartPos);
  const DocLn iLineEnd = SciCall_LineFromPosition(iSelEndPos);

  if (SciCall_IsSelectionRectangle())
  {
    if (bIsSelEmpty) { return; }

    const DocPos selAnchorMainPos = SciCall_GetRectangularSelectionAnchor();
    const DocPos selCaretMainPos = SciCall_GetRectangularSelectionCaret();
    const DocPos vSpcAnchorMainPos = SciCall_GetRectangularSelectionAnchorVirtualSpace();
    const DocPos vSpcCaretMainPos = SciCall_GetRectangularSelectionCaretVirtualSpace();

    DocPos iMaxLineLen = Sci_GetRangeMaxLineLength(iLineStart, iLineEnd);
    char* lineBuffer = AllocMem(iMaxLineLen + 1, HEAP_ZERO_MEMORY);
    DocPos remCount = 0;
    if (lineBuffer) {
      const DocPosU selCount = SciCall_GetSelections();
      for (DocPosU s = 0; s < selCount; ++s) {
        const DocPos selCaretPos = SciCall_GetSelectionNCaret(s);
        const DocPos selAnchorPos = SciCall_GetSelectionNAnchor(s);
        //const DocPos vSpcCaretPos = SciCall_GetSelectionNCaretVirtualSpace(s);
        //const DocPos vSpcAnchorPos = SciCall_GetSelectionNAnchorVirtualSpace(s);

        const DocPos selTargetStart = (selAnchorPos < selCaretPos) ? selAnchorPos : selCaretPos;
        const DocPos selTargetEnd = (selAnchorPos < selCaretPos) ? selCaretPos : selAnchorPos;
        //const DocPos vSpcLength = (selAnchorPos < selCaretPos) ? (vSpcCaretPos - vSpcAnchorPos) : (vSpcAnchorPos - vSpcCaretPos);

        DocPos diff = 0;
        DocPos const len = (selTargetEnd - selTargetStart);
        if (len >= 0) {
          char* pText = SciCall_GetRangePointer(selTargetStart, len + 1);
          const char* pEnd = (pText + len);
          DocPos i = 0;
          while (pText < pEnd) {
            const char ch = *pText++;
            if (IsBlankChar(ch)) {
              lineBuffer[i++] = ' ';
              while (IsBlankChar(*pText)) { ++pText; }
            }
            else { lineBuffer[i++] = ch; }
          }
          lineBuffer[i] = '\0';
          diff = len - i;
          SciCall_SetTargetRange(selTargetStart, selTargetEnd);
          SciCall_ReplaceTarget(-1, lineBuffer);
        }
        remCount += diff;
      } // for()
      FreeMem(lineBuffer);
    }

    SciCall_SetRectangularSelectionAnchor(selAnchorMainPos);
    if (vSpcAnchorMainPos > 0) {
      SciCall_SetRectangularSelectionAnchorVirtualSpace(vSpcAnchorMainPos);
    }
    SciCall_SetRectangularSelectionCaret(selCaretMainPos - remCount);
    if (vSpcCaretMainPos > 0) {
      SciCall_SetRectangularSelectionCaretVirtualSpace(vSpcCaretMainPos);
    }
  }
  else if (Sci_IsMultiSelection()) {
    // @@@ not implemented
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECT);
  }
  else   // SC_SEL_LINES | SC_SEL_STREAM
  {
    const DocPos iCurPos = SciCall_GetCurrentPos();
    const DocPos iAnchorPos = SciCall_GetAnchor();
    const DocPos iSelLength = (iSelEndPos - iSelStartPos);

    bool bIsLineStart = true;
    bool bIsLineEnd = true;

    const char* pszIn = NULL;
    char* pszOut = NULL;
    DocPos cch = 0;
    if (bIsSelEmpty) {
      pszIn = (const char*)SciCall_GetCharacterPointer();
      cch = SciCall_GetTextLength();
      pszOut = AllocMem(cch + 1, HEAP_ZERO_MEMORY);
    }
    else {
      pszIn = (const char*)SciCall_GetRangePointer(iSelStartPos, iSelLength + 1);
      cch = SciCall_GetSelText(NULL) - 1;
      pszOut = AllocMem(cch + 1, HEAP_ZERO_MEMORY);
      bIsLineStart = (iSelStartPos == SciCall_PositionFromLine(iLineStart));
      bIsLineEnd = (iSelEndPos == SciCall_GetLineEndPosition(iLineEnd));
    }

    if (pszIn && pszOut) {
      bool bModified = false;
      char* co = pszOut;
      DocPos remWSuntilCaretPos = 0;
      for (int i = 0; i < cch; ++i) {
        if (IsBlankChar(pszIn[i])) {
          if (pszIn[i] == '\t') { bModified = true; }
          while (IsBlankChar(pszIn[i + 1])) {
            if (bIsSelEmpty && (i < iSelStartPos)) { ++remWSuntilCaretPos; }
            ++i;
            bModified = true;
          }
          if (!bIsLineStart && ((pszIn[i + 1] != '\n') && (pszIn[i + 1] != '\r'))) {
            *co++ = ' ';
          }
          else {
            bModified = true;
          }
        }
        else {
          bIsLineStart = (pszIn[i] == '\n' || pszIn[i] == '\r') ? true : false;
          *co++ = pszIn[i];
        }
      }

      if (bIsLineEnd && (co > pszOut) && (*(co - 1) == ' ')) {
        if (bIsSelEmpty && ((cch - 1) < iSelStartPos)) { --remWSuntilCaretPos; }
        *--co = '\0';
        bModified = true;
      }

      if (bModified) {

        DocPos const saveTargetBeg = SciCall_GetTargetStart();
        DocPos const saveTargetEnd = SciCall_GetTargetEnd();

        if (!SciCall_IsSelectionEmpty()) {
          SciCall_TargetFromSelection();
        }
        else {
          SciCall_TargetWholeDocument();
        }
        SciCall_ReplaceTarget(-1, pszOut);

        SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore

        DocPos const iNewLen = (DocPos)StringCchLenA(pszOut, cch + 1);

        if (iCurPos < iAnchorPos) {
          EditSetSelectionEx(hwnd, iCurPos + iNewLen, iCurPos, -1, -1);
        }
        else if (iCurPos > iAnchorPos) {
          EditSetSelectionEx(hwnd, iAnchorPos, iAnchorPos + iNewLen, -1, -1);
        }
        else { // empty selection
          DocPos iNewPos = iCurPos;
          if (iCurPos > 0) {
            iNewPos = SciCall_PositionBefore(SciCall_PositionAfter(iCurPos - remWSuntilCaretPos));
          }
          EditSetSelectionEx(hwnd, iNewPos, iNewPos, -1, -1);
        }
      }
    }
    if (pszOut) { FreeMem(pszOut); }
  }
}


//=============================================================================
//
//  EditRemoveBlankLines()
//
void EditRemoveBlankLines(HWND hwnd, bool bMerge, bool bRemoveWhiteSpace)
{
  UNUSED(hwnd);

  if (Sci_IsMultiOrRectangleSelection()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECT);
    return;
  }

  const DocPos iSelStart = (SciCall_IsSelectionEmpty() ? 0 : SciCall_GetSelectionStart());
  const DocPos iSelEnd = (SciCall_IsSelectionEmpty() ? Sci_GetDocEndPosition() : SciCall_GetSelectionEnd());

  DocLn iBegLine = SciCall_LineFromPosition(iSelStart);
  DocLn iEndLine = SciCall_LineFromPosition(iSelEnd);

  if (iSelStart > SciCall_PositionFromLine(iBegLine)) { ++iBegLine; }
  if ((iSelEnd <= SciCall_PositionFromLine(iEndLine)) && (iEndLine != SciCall_GetLineCount() - 1)) { --iEndLine; }

  _IGNORE_NOTIFY_CHANGE_;
  DocPos const saveTargetBeg = SciCall_GetTargetStart();
  DocPos const saveTargetEnd = SciCall_GetTargetEnd();

  for (DocLn iLine = iBegLine; iLine <= iEndLine; )
  {
    DocLn nBlanks = 0;
    bool bSpcOnly = true;
    while (((iLine + nBlanks) <= iEndLine) && bSpcOnly) 
    {
      bSpcOnly = false;
      const DocPos posLnBeg = SciCall_PositionFromLine(iLine + nBlanks);
      const DocPos posLnEnd = SciCall_GetLineEndPosition(iLine + nBlanks);
      const DocPos iLnLength = (posLnEnd - posLnBeg);

      if (iLnLength == 0) {
        ++nBlanks;
        bSpcOnly = true;
      }
      else if (bRemoveWhiteSpace) {
        const char* pLine = SciCall_GetRangePointer(posLnBeg, iLnLength);
        DocPos i = 0;
        for (; i < iLnLength; ++i) {
          if (!IsBlankChar(pLine[i])) {
            break;
          }
        }
        if (i >= iLnLength) {
          ++nBlanks;
          bSpcOnly = true;
        }
      }
    }
    if ((nBlanks == 0) || ((nBlanks == 1) && bMerge)) {
      iLine += (nBlanks + 1);
    }
    else {
      if (bMerge) { --nBlanks; }

      SciCall_SetTargetRange(SciCall_PositionFromLine(iLine), SciCall_PositionFromLine(iLine + nBlanks));
      SciCall_ReplaceTarget(0, "");

      if (bMerge) { ++iLine; }
      iEndLine -= nBlanks;
    }
  }
  SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore
  _OBSERVE_NOTIFY_CHANGE_;
}


//=============================================================================
//
//  EditRemoveDuplicateLines()
//
void EditRemoveDuplicateLines(HWND hwnd, bool bRemoveEmptyLines)
{
  UNUSED(hwnd);

  if (Sci_IsMultiOrRectangleSelection()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECT);
    return;
  }
  
  DocPos const iSelStart = SciCall_GetSelectionStart();
  DocPos const iSelEnd = SciCall_GetSelectionEnd();

  DocLn iStartLine = 0;
  DocLn iEndLine = 0;
  if (iSelStart != iSelEnd) {
    iStartLine = SciCall_LineFromPosition(iSelStart);
    if (iSelStart > SciCall_PositionFromLine(iStartLine)) { ++iStartLine; }
    iEndLine = SciCall_LineFromPosition(iSelEnd);
    if (iSelEnd <= SciCall_PositionFromLine(iEndLine)) { --iEndLine; }
  }
  else {
    iEndLine = Sci_GetLastDocLineNumber();
  }

  if ((iEndLine - iStartLine) <= 1) { return; }
  
  _IGNORE_NOTIFY_CHANGE_;
  DocPos const saveTargetBeg = SciCall_GetTargetStart();
  DocPos const saveTargetEnd = SciCall_GetTargetEnd();

  for (DocLn iCurLine = iStartLine; iCurLine < iEndLine; ++iCurLine)
  {
    DocPos const iCurLnLen = Sci_GetNetLineLength(iCurLine);
    DocPos const iBegCurLine = SciCall_PositionFromLine(iCurLine);
    const char* const pCurrentLine = SciCall_GetRangePointer(iBegCurLine, iCurLnLen + 1);

    if (bRemoveEmptyLines || (iCurLnLen > 0)) 
    {
      DocLn iPrevLine = iCurLine;

      for (DocLn iCompareLine = iCurLine + 1; iCompareLine <= iEndLine; ++iCompareLine) 
      {
        DocPos const iCmpLnLen = Sci_GetNetLineLength(iCompareLine);
        if (bRemoveEmptyLines || (iCmpLnLen > 0)) 
        {
          DocPos const iBegCmpLine = SciCall_PositionFromLine(iCompareLine);
          const char* const pCompareLine = SciCall_GetRangePointer(iBegCmpLine, iCmpLnLen + 1);

          if (iCurLnLen == iCmpLnLen) 
          {
            if (StringCchCompareNA(pCurrentLine, iCurLnLen, pCompareLine, iCmpLnLen) == 0) 
            {
              SciCall_SetTargetRange(SciCall_GetLineEndPosition(iPrevLine), SciCall_GetLineEndPosition(iCompareLine));
              SciCall_ReplaceTarget(0, "");
              --iCompareLine; // proactive preventing progress to avoid comparison line skip
              --iEndLine;
            }
          }
        } // empty 
        iPrevLine = iCompareLine;
      }
    } // empty
  }

  SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore
  _OBSERVE_NOTIFY_CHANGE_;
}


//=============================================================================
//
//  EditWrapToColumn()
//
void EditWrapToColumn(HWND hwnd,DocPos nColumn/*,int nTabWidth*/)
{
  if (Sci_IsMultiOrRectangleSelection()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECT);
    return;
  }

  DocPos iCurPos = SciCall_GetCurrentPos();
  DocPos iAnchorPos = SciCall_GetAnchor();

  DocPos iSelStart = 0;
  DocPos iSelEnd = Sci_GetDocEndPosition();
  DocPos iSelCount = iSelEnd;

  if (!SciCall_IsSelectionEmpty()) {
    iSelStart = SciCall_GetSelectionStart();
    DocLn iLine = SciCall_LineFromPosition(iSelStart);
    iSelStart = SciCall_PositionFromLine(iLine);   // re-base selection to start of line
    iSelEnd = SciCall_GetSelectionEnd();
    iSelCount = (iSelEnd - iSelStart);
  }

  char* pszText = SciCall_GetRangePointer(iSelStart, iSelCount);

  LPWSTR pszTextW = AllocMem((iSelCount+2)*sizeof(WCHAR), HEAP_ZERO_MEMORY);
  if (pszTextW == NULL) {
    return;
  }

  int cchTextW = MultiByteToWideChar(Encoding_SciCP,0,pszText,(MBWC_DocPos_Cast)iSelCount,
                                     pszTextW,(MBWC_DocPos_Cast)(SizeOfMem(pszTextW)/sizeof(WCHAR)));

  LPWSTR pszConvW = AllocMem(cchTextW*sizeof(WCHAR)*3+2, HEAP_ZERO_MEMORY);
  if (pszConvW == NULL) {
    FreeMem(pszTextW);
    return;
  }

  int cchEOL = 2;
  WCHAR wszEOL[] = L"\r\n";
  int const cEOLMode = SciCall_GetEOLMode();
  if (cEOLMode == SC_EOL_CR)
    cchEOL = 1;
  else if (cEOLMode == SC_EOL_LF) {
    cchEOL = 1; wszEOL[0] = L'\n';
  }

  int cchConvW = 0;
  DocPos iLineLength = 0;

  //#define W_DELIMITER  L"!\"#$%&'()*+,-./:;<=>?@[\\]^`{|}~"  // underscore counted as part of word
  //WCHAR* W_DELIMITER  = Settings.AccelWordNavigation ? W_DelimCharsAccel : W_DelimChars;
  //#define ISDELIMITER(wc) StrChr(W_DELIMITER,wc)

  //WCHAR* W_WHITESPACE = Settings.AccelWordNavigation ? W_WhiteSpaceCharsAccelerated : W_WhiteSpaceCharsDefault;
  //#define ISWHITE(wc) StrChr(W_WHITESPACE,wc)
  #define ISWHITE(wc) StrChr(L" \t\f",wc)

  //#define ISWORDEND(wc) (ISDELIMITER(wc) || ISWHITE(wc))
  #define ISWORDEND(wc) StrChr(L" \t\f\r\n\v",wc)
  
  DocPos iCaretShift = 0;
  bool bModified = false;

  for (int iTextW = 0; iTextW < cchTextW; iTextW++)
  {
    WCHAR w = pszTextW[iTextW];

    if (ISWHITE(w))
    {
      DocPos iNextWordLen = 0;

      while (pszTextW[iTextW+1] == L' ' || pszTextW[iTextW+1] == L'\t') {
        ++iTextW;
        bModified = true;
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
          bModified = true;
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
  FreeMem(pszTextW);

  if (bModified) 
  {
    pszText = AllocMem(cchConvW * 3, HEAP_ZERO_MEMORY);
    if (pszText) 
    {
      int const cchConvM = WideCharToMultiByte(Encoding_SciCP, 0, pszConvW, cchConvW, 
                                               pszText, (MBWC_DocPos_Cast)SizeOfMem(pszText), NULL, NULL);

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

      DocPos const saveTargetBeg = SciCall_GetTargetStart();
      DocPos const saveTargetEnd = SciCall_GetTargetEnd();
      SciCall_SetTargetRange(iSelStart, iSelEnd);
      SciCall_ReplaceTarget(cchConvM, pszText);
      SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore

      FreeMem(pszText);

      EditSetSelectionEx(hwnd, iAnchorPos, iCurPos, -1, -1);
    }
  }
  FreeMem(pszConvW);
}



//=============================================================================
//
//  EditSplitLines()
//
void EditSplitLines(HWND hwnd)
{
  UNUSED(hwnd);
  DocPos const saveTargetBeg = SciCall_GetTargetStart();
  DocPos const saveTargetEnd = SciCall_GetTargetEnd();
  SciCall_TargetFromSelection();
  SciCall_LinesSplit();
  SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore
}


//=============================================================================
//
//  EditJoinLinesEx()
//
//  Customized version of  SCI_LINESJOIN  (w/o using TARGET transaction)
//
void EditJoinLinesEx(HWND hwnd, bool bPreserveParagraphs, bool bCRLF2Space)
{
  bool bModified = false;

  if (SciCall_IsSelectionEmpty()) { return; }

  if (Sci_IsMultiOrRectangleSelection()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECT);
    return;
  }

  DocPos const iSelStart = SciCall_GetSelectionStart();
  DocPos const iSelEnd = SciCall_GetSelectionEnd();
  DocPos const iSelLength = (iSelEnd - iSelStart);
  DocPos iCurPos = SciCall_GetCurrentPos();
  DocPos iAnchorPos = SciCall_GetAnchor();

  DocPos cchJoin = (DocPos)-1;

  char* pszText = SciCall_GetRangePointer(iSelStart, iSelLength);

  char* pszJoin = AllocMem(iSelLength + 1, HEAP_ZERO_MEMORY);
  if (pszJoin == NULL) {
    return;
  }

  char szEOL[] = "\r\n";
  int  cchEOL = 2;
  switch (SciCall_GetEOLMode())
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

  for (int i = 0; i < iSelLength; ++i)
  {
    int j = i;
    // try to swallow next line-breaks
    while (StrChrA("\r\n", pszText[j])) { ++j; }

    if (i < j) {
      // swallowed!
      if (((j - i) >= 2*cchEOL) && bPreserveParagraphs) {
        for (int k = 0; k < 2*cchEOL; ++k) { pszJoin[++cchJoin] = szEOL[k % cchEOL]; }
      }
      else if (bCRLF2Space) {
        pszJoin[++cchJoin] = ' ';
      }
      i = j;
      bModified = true;
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

    DocPos const saveTargetBeg = SciCall_GetTargetStart();
    DocPos const saveTargetEnd = SciCall_GetTargetEnd();
    SciCall_SetTargetRange(iSelStart, iSelEnd);
    SciCall_ReplaceTarget(cchJoin, pszJoin);
    SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore

    EditSetSelectionEx(hwnd, iAnchorPos, iCurPos, -1, -1);
  }

  if (pszJoin) { FreeMem(pszJoin); }
}


//=============================================================================
//
//  EditSortLines()
//
typedef struct _SORTLINE {
  WCHAR *pwszLine;
  WCHAR *pwszSortEntry;
} SORTLINE;

typedef int (__stdcall * FNSTRCMP)(LPCWSTR,LPCWSTR);
typedef int (__stdcall * FNSTRLOGCMP)(LPCWSTR, LPCWSTR);

int CmpStd(const void *s1, const void *s2) {
  int cmp = StrCmp(((SORTLINE*)s1)->pwszSortEntry,((SORTLINE*)s2)->pwszSortEntry);
  return (cmp) ? cmp : StrCmp(((SORTLINE*)s1)->pwszLine,((SORTLINE*)s2)->pwszLine);
}

int CmpStdRev(const void *s1, const void *s2) {
  int cmp = -1 * StrCmp(((SORTLINE*)s1)->pwszSortEntry,((SORTLINE*)s2)->pwszSortEntry);
  return (cmp) ? cmp :  -1 * StrCmp(((SORTLINE*)s1)->pwszLine,((SORTLINE*)s2)->pwszLine);
}

int CmpLogical(const void *s1, const void *s2) {
  int cmp = StrCmpLogicalW(((SORTLINE*)s1)->pwszSortEntry,((SORTLINE*)s2)->pwszSortEntry);
  if (cmp == 0) {
    cmp = StrCmpLogicalW(((SORTLINE*)s1)->pwszLine, ((SORTLINE*)s2)->pwszLine);
  }
  if (cmp) {
    return cmp;
  }
  cmp = StrCmp(((SORTLINE*)s1)->pwszSortEntry,((SORTLINE*)s2)->pwszSortEntry);
  return (cmp) ? cmp : StrCmp(((SORTLINE*)s1)->pwszLine,((SORTLINE*)s2)->pwszLine);
}

int CmpLogicalRev(const void *s1, const void *s2) {
  int cmp = -1 * StrCmpLogicalW(((SORTLINE*)s1)->pwszSortEntry,((SORTLINE*)s2)->pwszSortEntry);
  if (cmp == 0) {
    cmp = -1 * StrCmpLogicalW(((SORTLINE*)s1)->pwszLine, ((SORTLINE*)s2)->pwszLine);
  }
  if (cmp) {
    return cmp;
  }
  cmp = -1 * StrCmp(((SORTLINE*)s1)->pwszSortEntry,((SORTLINE*)s2)->pwszSortEntry);
  return (cmp) ? cmp : -1 * StrCmp(((SORTLINE*)s1)->pwszLine,((SORTLINE*)s2)->pwszLine);
}


void EditSortLines(HWND hwnd, int iSortFlags)
{

  if (SciCall_IsSelectionEmpty()) { return; } // no selection
  
  FNSTRCMP pfnStrCmp = (iSortFlags & SORT_NOCASE) ? StrCmpIW : StrCmpW;

  bool const bIsMultiSel = Sci_IsMultiOrRectangleSelection();

  DocPos const iSelStart = SciCall_GetSelectionStart(); //iSelStart = SciCall_PositionFromLine(iLine);
  DocPos const iSelEnd = SciCall_GetSelectionEnd();
  //DocLn const iLine = SciCall_LineFromPosition(iSelStart);

  DocPos iCurPos = bIsMultiSel ? SciCall_GetRectangularSelectionCaret() : SciCall_GetCurrentPos();
  DocPos iAnchorPos = bIsMultiSel ? SciCall_GetRectangularSelectionAnchor() : SciCall_GetAnchor();
  DocPos iCurPosVS = bIsMultiSel ? SciCall_GetRectangularSelectionCaretVirtualSpace() : 0;
  DocPos iAnchorPosVS = bIsMultiSel ? SciCall_GetRectangularSelectionAnchorVirtualSpace() : 0;

  DocLn const iRcCurLine = bIsMultiSel ? SciCall_LineFromPosition(iCurPos) : 0;
  DocLn const iRcAnchorLine = bIsMultiSel ? SciCall_LineFromPosition(iAnchorPos) : 0;

  DocPos const iCurCol = SciCall_GetColumn(iCurPos);
  DocPos const iAnchorCol = SciCall_GetColumn(iAnchorPos);
  DocLn const iSortColumn = bIsMultiSel ? min_p(iCurCol, iAnchorCol) : (UINT)SciCall_GetColumn(iCurPos);

  DocLn const iLineStart = bIsMultiSel ? min_ln(iRcCurLine, iRcAnchorLine) : SciCall_LineFromPosition(iSelStart);
  DocLn const _lnend = bIsMultiSel ? max_ln(iRcCurLine, iRcAnchorLine) : SciCall_LineFromPosition(iSelEnd);
  DocLn const iLineEnd = (iSelEnd <= SciCall_PositionFromLine(_lnend)) ? (_lnend - 1) : _lnend;
  if (iLineEnd <= iLineStart) { return; }

  DocLn const iLineCount = iLineEnd - iLineStart + 1;

  int const cEOLMode = SciCall_GetEOLMode();
  char mszEOL[] = "\r\n";
  if (cEOLMode == SC_EOL_CR) {
    mszEOL[1] = '\0';
  }
  else if (cEOLMode == SC_EOL_LF) {
    mszEOL[0] = '\n';
    mszEOL[1] = '\0';
  }

  int const _iTabWidth = SciCall_GetTabWidth();

  if (bIsMultiSel)
  {
    EditPadWithSpaces(hwnd, !(iSortFlags & SORT_SHUFFLE), true);
    // changed rectangular selection
    iCurPos = SciCall_GetRectangularSelectionCaret();
    iAnchorPos = SciCall_GetRectangularSelectionAnchor();
    iCurPosVS = SciCall_GetRectangularSelectionCaretVirtualSpace();
    iAnchorPosVS = SciCall_GetRectangularSelectionAnchorVirtualSpace();
  }

  SORTLINE* pLines = AllocMem(sizeof(SORTLINE) * iLineCount, HEAP_ZERO_MEMORY);
  if (!pLines) { return; }

  DocPos iMaxLineLen = Sci_GetRangeMaxLineLength(iLineStart, iLineEnd);
  char* pmsz = AllocMem(iMaxLineLen + 1, HEAP_ZERO_MEMORY);

  DocPos cchTotal = 0;
  DocPos ichlMax = 3;
  DocLn iZeroLenLineCount = 0;
  for (DocLn i = 0, iLn = iLineStart; iLn <= iLineEnd; ++iLn, ++i) {

    DocPos const cchm = SciCall_LineLength(iLn);
    cchTotal += cchm;
    ichlMax = max_p(ichlMax, cchm);

    SciCall_GetLine_Safe(iLn, pmsz);

    if (iSortFlags & SORT_REMWSPACELN) {
      StrTrimA(pmsz, "\t\v \r\n"); // try clean line
      if (StringCchLenA(pmsz, cchm) == 0) {
        // white-space only - remove
        continue;
      }
    }
    StrTrimA(pmsz, "\r\n"); // ignore line-breaks 

    int const cchw = MultiByteToWideChar(Encoding_SciCP, 0, pmsz, -1, NULL, 0) - 1;
    if (cchw > 0) {
      int tabs = _iTabWidth;
      int const lnLen = (MBWC_DocPos_Cast)(sizeof(WCHAR) * (cchw + 1));
      pLines[i].pwszLine = AllocMem(lnLen, HEAP_ZERO_MEMORY);
      MultiByteToWideChar(Encoding_SciCP, 0, pmsz, -1, pLines[i].pwszLine, lnLen / (int)sizeof(WCHAR));
      pLines[i].pwszSortEntry = pLines[i].pwszLine;
      if (iSortFlags & SORT_COLUMN) {
        int col = 0;
        while (*(pLines[i].pwszSortEntry)) {
          if (*(pLines[i].pwszSortEntry) == L'\t') {
            if (col + tabs <= iSortColumn) {
              col += tabs;
              tabs = _iTabWidth;
              pLines[i].pwszSortEntry = CharNext(pLines[i].pwszSortEntry);
            }
            else
              break;
          }
          else if (col < iSortColumn) {
            col++;
            if (--tabs == 0)
              tabs = _iTabWidth;
            pLines[i].pwszSortEntry = CharNext(pLines[i].pwszSortEntry);
          }
          else
            break;
        }
      }
    }
    else {
      ++iZeroLenLineCount;
    }
  }
  FreeMem(pmsz);

  if (iSortFlags & SORT_DESCENDING) {
    if (iSortFlags & SORT_LOGICAL)
      qsort(pLines, iLineCount, sizeof(SORTLINE), CmpLogicalRev);
    else
      qsort(pLines, iLineCount, sizeof(SORTLINE), CmpStdRev);
  }
  else if (iSortFlags & SORT_SHUFFLE) {
    srand((UINT)GetTickCount());
    for (DocLn i = (iLineCount - 1); i > 0; --i) {
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
    if ((iSortFlags & SORT_LOGICAL))
      qsort(pLines, iLineCount, sizeof(SORTLINE), CmpLogical);
    else
      qsort(pLines, iLineCount, sizeof(SORTLINE), CmpStd);
  }

  DocLn const lenRes = cchTotal + (2 * iLineCount) + 1;
  char* pmszResult = AllocMem(lenRes, HEAP_ZERO_MEMORY);
  char* pmszResOffset = pmszResult;
  char* pmszBuf = AllocMem(ichlMax + 1, HEAP_ZERO_MEMORY);

  bool bLastDup = false;
  for (DocLn i = 0; i < iLineCount; ++i) {
    if (pLines[i].pwszLine && ((iSortFlags & SORT_SHUFFLE) || StrIsNotEmpty(pLines[i].pwszLine))) {
      bool bDropLine = false;
      if (!(iSortFlags & SORT_SHUFFLE)) {
        if (iSortFlags & SORT_MERGEDUP || iSortFlags & SORT_UNIQDUP || iSortFlags & SORT_UNIQUNIQ) {
          if (i < (iLineCount - 1)) {
            if (pfnStrCmp(pLines[i].pwszLine, pLines[i + 1].pwszLine) == 0) {
              bLastDup = true;
              bDropLine = (iSortFlags & SORT_MERGEDUP || iSortFlags & SORT_UNIQDUP);
            }
            else {
              bDropLine = (!bLastDup && (iSortFlags & SORT_UNIQUNIQ)) || (bLastDup && (iSortFlags & SORT_UNIQDUP));
              bLastDup = false;
            }
          }
          else {
            bDropLine = (!bLastDup && (iSortFlags & SORT_UNIQUNIQ)) || (bLastDup && (iSortFlags & SORT_UNIQDUP));
            bLastDup = false;
          }
        }
      }
      if (!bDropLine) {
        WideCharToMultiByte(Encoding_SciCP, 0, pLines[i].pwszLine, -1, 
                            pmszBuf, (MBWC_DocPos_Cast)(ichlMax + 1), NULL, NULL);
        StringCchCatA(pmszResult, lenRes, pmszBuf);
        StringCchCatA(pmszResult, lenRes, mszEOL);
      }
    }
  }
  FreeMem(pmszBuf);

  // Handle empty (no whitespace or other char) lines (always at the end)
  if (!(iSortFlags & SORT_UNIQDUP) || (iZeroLenLineCount == 0)) {
    StrTrimA(pmszResOffset, "\r\n"); // trim end only
  }
  if (((iSortFlags & SORT_UNIQDUP) && (iZeroLenLineCount > 1)) || (iSortFlags & SORT_MERGEDUP)) {
    iZeroLenLineCount = 1; // removes duplicate empty lines
  }
  if (!(iSortFlags & SORT_REMZEROLEN)) {
    for (DocLn i = 0; i < iZeroLenLineCount; ++i) {
      StringCchCatA(pmszResult, lenRes, mszEOL);
    }
  }

  for (DocLn i = 0; i < iLineCount; ++i) {
    if (pLines[i].pwszLine) { FreeMem(pLines[i].pwszLine); }
  }
  FreeMem(pLines);

  //DocPos const iResultLength = (DocPos)StringCchLenA(pmszResult, lenRes) + ((cEOLMode == SC_EOL_CRLF) ? 2 : 1);
  if (!bIsMultiSel) {
    if (iAnchorPos > iCurPos) {
      iCurPos = SciCall_FindColumn(iLineStart, iCurCol);
      iAnchorPos = SciCall_FindColumn(_lnend, iAnchorCol);
    }
    else {
      iAnchorPos = SciCall_FindColumn(iLineStart, iAnchorCol);
      iCurPos = SciCall_FindColumn(_lnend, iCurCol);
    }
  }

  DocPos const saveTargetBeg = SciCall_GetTargetStart();
  DocPos const saveTargetEnd = SciCall_GetTargetEnd();
//SciCall_SetTargetRange(SciCall_PositionFromLine(iLineStart), SciCall_PositionFromLine(iLineEnd + 1));
  SciCall_SetTargetRange(SciCall_PositionFromLine(iLineStart), SciCall_GetLineEndPosition(iLineEnd));
  SciCall_ReplaceTarget(-1, pmszResult);
  SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore
  FreeMem(pmszResult);

  if (bIsMultiSel) {
    EditSetSelectionEx(hwnd, iAnchorPos, iCurPos, iAnchorPosVS, iCurPosVS);
  }
  else {
    EditSetSelectionEx(hwnd, iAnchorPos, iCurPos, -1, -1);
  }
}


//=============================================================================
//
//  EditSetSelectionEx()
//
void EditSetSelectionEx(HWND hwnd, DocPos iAnchorPos, DocPos iCurrentPos, DocPos vSpcAnchor, DocPos vSpcCurrent)
{
  UNUSED(hwnd);

  if ((iAnchorPos < 0) && (iCurrentPos < 0)) {
    SciCall_SelectAll();
  }
  else if (iAnchorPos < 0) {
    iAnchorPos = 0;
  }
  if (iCurrentPos < 0) {
    iCurrentPos = Sci_GetDocEndPosition();
  }

  DocLn const iNewLine = SciCall_LineFromPosition(iCurrentPos);
  DocLn const iAnchorLine = SciCall_LineFromPosition(iAnchorPos);

  // Ensure that the first and last lines of a selection are always unfolded
  // This needs to be done *before* the SCI_SETSEL message
  SciCall_EnsureVisible(iAnchorLine);
  if (iAnchorLine != iNewLine) {  SciCall_EnsureVisible(iNewLine);  }

  if ((vSpcAnchor >= 0) && (vSpcCurrent >= 0)) {
    SciCall_SetRectangularSelectionAnchor(iAnchorPos);
    if (vSpcAnchor > 0) {
      SciCall_SetRectangularSelectionAnchorVirtualSpace(vSpcAnchor);
    }
    SciCall_SetRectangularSelectionCaret(iCurrentPos);
    if (vSpcCurrent > 0) {
      SciCall_SetRectangularSelectionCaretVirtualSpace(vSpcCurrent);
    }
    SciCall_ScrollRange(iAnchorPos, iCurrentPos);
  }
  else {
    SciCall_SetSel(iAnchorPos, iCurrentPos);  // scrolls into view
  }
  // remember x-pos for moving caret vertically
  SciCall_ChooseCaretX();

  UpdateToolbar();
  UpdateStatusbar(false);
}


//=============================================================================
//
//  EditEnsureSelectionVisible()
//
void EditEnsureSelectionVisible(HWND hwnd)
{
  DocPos iAnchorPos = 0;
  DocPos iCurrentPos = 0;
  DocPos iAnchorPosVS = -1;
  DocPos iCurPosVS = -1;

  if (SciCall_IsSelectionRectangle()) 
  {
    iAnchorPos = SciCall_GetRectangularSelectionAnchor();
    iCurrentPos = SciCall_GetRectangularSelectionCaret();
    iAnchorPosVS = SciCall_GetRectangularSelectionAnchorVirtualSpace();
    iCurPosVS = SciCall_GetRectangularSelectionCaretVirtualSpace();
  }
  else if (Sci_IsMultiSelection()) {
    iAnchorPos = SciCall_GetSelectionNAnchor(0);
    iCurrentPos = SciCall_GetSelectionNCaret(0);
    iAnchorPosVS = SciCall_GetSelectionNAnchorVirtualSpace(0);
    iCurPosVS = SciCall_GetSelectionNCaretVirtualSpace(0);
  }
  else {
    iAnchorPos = SciCall_GetAnchor();
    iCurrentPos = SciCall_GetCurrentPos();
  }
  EditSetSelectionEx(hwnd, iAnchorPos, iCurrentPos, iAnchorPosVS, iCurPosVS);
}


//=============================================================================
//
//  EditEnsureConsistentLineEndings()
//
void EditEnsureConsistentLineEndings(HWND hwnd)
{
  Globals.bDocHasInconsistentEOLs = false;
  SciCall_ConvertEOLs(SciCall_GetEOLMode());
  EditFixPositions(hwnd);
}


//=============================================================================
//
//  EditScrollTo()
//
void EditScrollTo(HWND hwnd, DocLn iScrollToLine, int iSlop)
{
  UNUSED(hwnd);

  const int iXoff = SciCall_GetXOffset();
  const DocLn iLinesOnScreen = SciCall_LinesOnScreen();
  const DocLn iSlopLines = ((iSlop < 0) || (iSlop >= iLinesOnScreen)) ? (iLinesOnScreen/2) : iSlop;

  SciCall_SetVisiblePolicy((VISIBLE_SLOP | VISIBLE_STRICT), iSlopLines);
  SciCall_EnsureVisibleEnforcePolicy(iScrollToLine);
  SciCall_SetXOffset(iXoff);
}


//=============================================================================
//
//  EditJumpTo()
//
void EditJumpTo(HWND hwnd, DocLn iNewLine, DocPos iNewCol)
{
  // jump to end with line set to -1
  if (iNewLine < 0) {
    SciCall_DocumentEnd();
    return;
  }
  const DocLn iMaxLine = SciCall_GetLineCount();
  // Line maximum is iMaxLine - 1 (doc line count starts with 0)
  iNewLine = (min_ln(iNewLine, iMaxLine) - 1);
  const DocPos iLineEndPos = SciCall_GetLineEndPosition(iNewLine);
  
  // Column minimum is 1
  DocPos const colOffset = Globals.bZeroBasedColumnIndex ? 0 : 1;
  iNewCol = max_p(0, min_p((iNewCol - colOffset), iLineEndPos));
  const DocPos iNewPos = SciCall_FindColumn(iNewLine, iNewCol);

  SciCall_GotoPos(iNewPos);
  EditScrollTo(hwnd, iNewLine, -1);

  // remember x-pos for moving caret vertically
  SciCall_ChooseCaretX();
}



//=============================================================================
//
//  EditGetCurrentDocView()
//
const DOCVIEWPOS_T EditGetCurrentDocView(HWND hwnd)
{
  UNUSED(hwnd);
  DOCVIEWPOS_T docView = INIT_DOCVIEWPOS;
  docView.iCurPos = Sci_IsMultiOrRectangleSelection() ? SciCall_GetSelectionNCaret(0) : SciCall_GetCurrentPos();
  docView.iAnchorPos = Sci_IsMultiOrRectangleSelection() ? SciCall_GetSelectionNAnchor(SciCall_GetSelections() - 1) : SciCall_GetAnchor();
  //docView.vSpcCaretPos = SciCall_IsSelectionRectangle() ? SciCall_GetRectangularSelectionCaretVirtualSpace() : -1;
  //docView.vSpcAnchorPos = SciCall_IsSelectionRectangle() ? SciCall_GetRectangularSelectionAnchorVirtualSpace() : -1;
  docView.iCurrLine = SciCall_LineFromPosition(docView.iCurPos);
  docView.iCurColumn = SciCall_GetColumn(docView.iCurPos);
  docView.iVisTopLine = SciCall_GetFirstVisibleLine();
  docView.iDocTopLine = SciCall_DocLineFromVisible(docView.iVisTopLine);
  docView.iXOffset = SciCall_GetXOffset();
  docView.bIsTail = (docView.iCurPos == docView.iAnchorPos) && (docView.iCurrLine >= (SciCall_GetLineCount() - 1));
  return docView;
}


//=============================================================================
//
//  EditSetDocView()
//
void EditSetDocView(HWND hwnd, const DOCVIEWPOS_T docView)
{
  EditJumpTo(hwnd, docView.iCurrLine + 1, docView.iCurColumn + 1);
  DocLn const iNewTopLine = SciCall_GetFirstVisibleLine();
  SciCall_EnsureVisible(iNewTopLine);
  SciCall_LineScroll(0, docView.iVisTopLine - iNewTopLine);
  SciCall_SetXOffset(docView.iXOffset);
}


//=============================================================================
//
//  EditFixPositions()
//
void EditFixPositions(HWND hwnd)
{
  UNUSED(hwnd);

  DocPos const iCurrentPos = SciCall_GetCurrentPos();
  DocPos const iAnchorPos = SciCall_GetAnchor();
  DocPos const iMaxPos = Sci_GetDocEndPosition();
  
  DocPos iNewPos = iCurrentPos;

  if ((iCurrentPos > 0) && (iCurrentPos <= iMaxPos)) 
  {
    iNewPos = SciCall_PositionAfter(SciCall_PositionBefore(iCurrentPos));

    if (iNewPos != iCurrentPos) {
      SciCall_SetCurrentPos(iNewPos);
    }
  }

  if ((iAnchorPos != iNewPos) && (iAnchorPos > 0) && (iAnchorPos <= iMaxPos))
  {
    iNewPos = SciCall_PositionAfter(SciCall_PositionBefore(iAnchorPos));
    if (iNewPos != iAnchorPos) { 
      SciCall_SetAnchor(iNewPos); 
    }
  }
}


//=============================================================================
//
//  EditGetExcerpt()
//
void EditGetExcerpt(HWND hwnd,LPWSTR lpszExcerpt,DWORD cchExcerpt)
{
  UNUSED(hwnd);

  const DocPos iCurPos = SciCall_GetCurrentPos();
  const DocPos iAnchorPos = SciCall_GetAnchor();

  if ((iCurPos == iAnchorPos) || Sci_IsMultiOrRectangleSelection()) {
    StringCchCopy(lpszExcerpt,cchExcerpt,L"");
    return;
  }

  WCHAR tch[256] = { L'\0' };
  struct Sci_TextRange tr = { { 0, 0 }, NULL };
  /*if (iCurPos != iAnchorPos && !Sci_IsMultiOrRectangleSelection()) {*/
  tr.chrg.cpMin = (DocPosCR)SciCall_GetSelectionStart();
  tr.chrg.cpMax = min_cr((tr.chrg.cpMin + (DocPosCR)COUNTOF(tch)), (DocPosCR)SciCall_GetSelectionEnd());
  /*}
  else {
    int iLine = SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iCurPos,0);
    tr.chrg.cpMin = SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLine,0);
    tr.chrg.cpMax = min_cr(SendMessage(hwnd,SCI_GETLINEENDPOSITION,(WPARAM)iLine,0),(LONG)(tr.chrg.cpMin + COUNTOF(tchBuf2)));
  }*/
  tr.chrg.cpMax = min_cr(tr.chrg.cpMax, (DocPosCR)Sci_GetDocEndPosition());

  size_t const len = (tr.chrg.cpMax - tr.chrg.cpMin);
  char*  pszText  = AllocMem(len+1, HEAP_ZERO_MEMORY);
  LPWSTR pszTextW = AllocMem((len+1) * sizeof(WCHAR), HEAP_ZERO_MEMORY);

  DWORD cch = 0;
  if (pszText && pszTextW) 
  {
    tr.lpstrText = pszText;
    DocPos const rlen = SciCall_GetTextRange(&tr);
    MultiByteToWideChar(Encoding_SciCP,0,pszText,(MBWC_DocPos_Cast)rlen,pszTextW,(MBWC_DocPos_Cast)len);

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

  if ((cch > cchExcerpt) && (cchExcerpt >= 4)) {
    tch[cchExcerpt-2] = L'.';
    tch[cchExcerpt-3] = L'.';
    tch[cchExcerpt-4] = L'.';
  }
  StringCchCopyN(lpszExcerpt,cchExcerpt,tch,cchExcerpt);

  if (pszText) { FreeMem(pszText); }
  if (pszTextW) { FreeMem(pszTextW); }
}


//=============================================================================
//
//  _SetSearchFlags()
//
static void  _SetSearchFlags(HWND hwnd, LPEDITFINDREPLACE lpefr)
{
  char szBuf[FNDRPL_BUFFER];

  bool bIsFindDlg = (GetDlgItem(Globals.hwndDlgFindReplace, IDC_REPLACE) == NULL);

  GetDlgItemTextW2MB(hwnd, IDC_FINDTEXT, szBuf, COUNTOF(szBuf));
  if (StringCchCompareXA(szBuf, lpefr->szFind) != 0) {
    StringCchCopyNA(lpefr->szFind, COUNTOF(lpefr->szFind), szBuf, COUNTOF(szBuf));
    lpefr->bStateChanged = true;
  }

  GetDlgItemTextW2MB(hwnd, IDC_REPLACETEXT, szBuf, COUNTOF(szBuf));
  if (StringCchCompareXA(szBuf, lpefr->szReplace) != 0) {
    StringCchCopyNA(lpefr->szReplace, COUNTOF(lpefr->szReplace), szBuf, COUNTOF(szBuf));
    lpefr->bStateChanged = true;
  }

  bool bIsFlagSet = ((lpefr->fuFlags & SCFIND_MATCHCASE) != 0);
  if (IsButtonChecked(hwnd, IDC_FINDCASE)) {
    if (!bIsFlagSet) {
      lpefr->fuFlags |= SCFIND_MATCHCASE;
      lpefr->bStateChanged = true;
    }
  }
  else {
    if (bIsFlagSet) {
      lpefr->fuFlags &= ~(SCFIND_MATCHCASE);
      lpefr->bStateChanged = true;
    }
  }

  bIsFlagSet = ((lpefr->fuFlags & SCFIND_WHOLEWORD) != 0);
  if (IsButtonChecked(hwnd, IDC_FINDWORD)) {
    if (!bIsFlagSet) {
      lpefr->fuFlags |= SCFIND_WHOLEWORD;
      lpefr->bStateChanged = true;
    }
  }
  else {
    if (bIsFlagSet) {
      lpefr->fuFlags &= ~(SCFIND_WHOLEWORD);
      lpefr->bStateChanged = true;
    }
  }

  bIsFlagSet = ((lpefr->fuFlags & SCFIND_WORDSTART) != 0);
  if (IsButtonChecked(hwnd, IDC_FINDSTART)) {
    if (!bIsFlagSet) {
      lpefr->fuFlags |= SCFIND_WORDSTART;
      lpefr->bStateChanged = true;
    }
  }
  else {
    if (bIsFlagSet) {
      lpefr->fuFlags &= ~(SCFIND_WORDSTART);
      lpefr->bStateChanged = true;
    }
  }

  bIsFlagSet = ((lpefr->fuFlags & SCFIND_NP3_REGEX) != 0);
  if (IsButtonChecked(hwnd, IDC_FINDREGEXP)) {
    if (!bIsFlagSet) {
      lpefr->fuFlags |= SCFIND_NP3_REGEX;
      lpefr->bStateChanged = true;
    }
  }
  else {
    if (bIsFlagSet) {
      lpefr->fuFlags &= ~(SCFIND_NP3_REGEX);
      lpefr->bStateChanged = true;
    }
  }
  if (bIsFlagSet) // check "dot match all" too
  {
    bIsFlagSet = ((lpefr->fuFlags & SCFIND_DOT_MATCH_ALL) != 0);
    if (IsButtonChecked(hwnd, IDC_DOT_MATCH_ALL)) {
      if (!bIsFlagSet) {
        lpefr->fuFlags |= SCFIND_DOT_MATCH_ALL;
        lpefr->bStateChanged = true;
      }
    }
    else {
      if (bIsFlagSet) {
        lpefr->fuFlags &= ~(SCFIND_DOT_MATCH_ALL);
        lpefr->bStateChanged = true;
      }
    }
  }

  bIsFlagSet = lpefr->bWildcardSearch;
  if (IsButtonChecked(hwnd, IDC_WILDCARDSEARCH)) {
    if (!bIsFlagSet) {
      lpefr->bWildcardSearch = true;
      lpefr->bStateChanged = true;
    }
  }
  else {
    if (bIsFlagSet) {
      lpefr->bWildcardSearch = false;
      lpefr->bStateChanged = true;
    }
  }
  if (bIsFlagSet) // special setting for wildcardsearch
  {
    bIsFlagSet = ((lpefr->fuFlags & SCFIND_NP3_REGEX) != 0);
    if (!bIsFlagSet) {
      lpefr->fuFlags |= SCFIND_NP3_REGEX;
      lpefr->bStateChanged = true;
    }

    bIsFlagSet = ((lpefr->fuFlags & SCFIND_DOT_MATCH_ALL) != 0);
    if (bIsFlagSet) {
      lpefr->fuFlags &= ~(SCFIND_DOT_MATCH_ALL);
      lpefr->bStateChanged = true;
    }
  }

  bIsFlagSet = lpefr->bTransformBS;
  if (IsButtonChecked(hwnd, IDC_FINDTRANSFORMBS)) {
    if (!bIsFlagSet) {
      lpefr->bTransformBS = true;
      lpefr->bStateChanged = true;
    }
  }
  else {
    if (bIsFlagSet) {
      lpefr->bTransformBS = false;
      lpefr->bStateChanged = true;
    }
  }

  bIsFlagSet = lpefr->bNoFindWrap;
  if (IsButtonChecked(hwnd, IDC_NOWRAP)) {
    if (!bIsFlagSet) {
      lpefr->bNoFindWrap = true;
      lpefr->bStateChanged = true;
    }
  }
  else {
    if (bIsFlagSet) {
      lpefr->bNoFindWrap = false;
      lpefr->bStateChanged = true;
    }
  }

  bIsFlagSet = lpefr->bMarkOccurences;
  if (IsButtonChecked(hwnd, IDC_ALL_OCCURRENCES)) {
    if (!bIsFlagSet) {
      lpefr->bMarkOccurences = true;
      lpefr->bStateChanged = true;
    }
  }
  else {
    if (bIsFlagSet) {
      lpefr->bMarkOccurences = false;
      lpefr->bStateChanged = true;
    }
  }

  if (bIsFindDlg) 
  {
    bIsFlagSet = lpefr->bFindClose;
    if (IsButtonChecked(hwnd, IDC_FINDCLOSE)) {
      if (!bIsFlagSet) {
        lpefr->bFindClose = true;
        lpefr->bStateChanged = true;
      }
    }
    else {
      if (bIsFlagSet) {
        lpefr->bFindClose = false;
        lpefr->bStateChanged = true;
      }
    }
  }
  else // replace close
  {
    bIsFlagSet = lpefr->bReplaceClose;
    if (IsButtonChecked(hwnd, IDC_FINDCLOSE)) {
      if (!bIsFlagSet) {
        lpefr->bReplaceClose = true;
        lpefr->bStateChanged = true;
      }
    }
    else {
      if (bIsFlagSet) {
        lpefr->bReplaceClose = false;
        lpefr->bStateChanged = true;
      }
    }

  }
}


// Wildcard search uses the regexp engine to perform a simple search with * ? as wildcards 
// instead of more advanced and user-unfriendly regexp syntax
// for speed, we only need POSIX syntax here
static void  _EscapeWildcards(char* szFind2, LPCEDITFINDREPLACE lpefr)
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
//  _EditGetFindStrg()
//
static int  _EditGetFindStrg(HWND hwnd, LPCEDITFINDREPLACE lpefr, LPSTR szFind, int cchCnt)
{
  UNUSED(hwnd);
  if (StringCchLenA(lpefr->szFind, COUNTOF(lpefr->szFind))) {
    StringCchCopyA(szFind, cchCnt, lpefr->szFind);
  }
  else {
    GetFindPatternMB(szFind, cchCnt);
    StringCchCopyA(lpefr->szFind, COUNTOF(lpefr->szFind), szFind);
  }
  if (!StringCchLenA(szFind, cchCnt)) { return 0; }

  bool bIsRegEx = (lpefr->fuFlags & SCFIND_REGEXP);
  if (lpefr->bTransformBS || bIsRegEx) {
    TransformBackslashes(szFind, bIsRegEx, Encoding_SciCP, NULL);
  }
  if (StringCchLenA(szFind, FNDRPL_BUFFER) > 0) {
    if (lpefr->bWildcardSearch)
      _EscapeWildcards(szFind, lpefr);
  }
  
  return (int)StringCchLenA(szFind, FNDRPL_BUFFER);
}




//=============================================================================
//
//  _FindInTarget()
//
static DocPos  _FindInTarget(HWND hwnd, LPCSTR szFind, DocPos length, int flags, 
                             DocPos* start, DocPos* end, bool bForceNext, FR_UPD_MODES fMode)
{
  UNUSED(hwnd);

  DocPos _start = *start;
  DocPos _end = *end;
  bool const bFindPrev = (_start > _end);
  DocPos iPos = 0;

  DocPos const saveTargetBeg = SciCall_GetTargetStart();
  DocPos const saveTargetEnd = SciCall_GetTargetEnd();

  SciCall_SetSearchFlags(flags);
  SciCall_SetTargetRange(_start, _end);
  iPos = SciCall_SearchInTarget(length, szFind);
  //  handle next in case of zero-length-matches (regex) !
  if (iPos == _start) {
    DocPos const nend = SciCall_GetTargetEnd();
    if ((_start == nend) && bForceNext)
    {
      DocPos const _new_start = (bFindPrev ? SciCall_PositionBefore(_start) : SciCall_PositionAfter(_start));
      bool const bProceed = (bFindPrev ? (_new_start >= _end) : (_new_start <= _end));
      if ((_new_start != _start) && bProceed) {
        SciCall_SetTargetRange(_new_start, _end);
        iPos = SciCall_SearchInTarget(length, szFind);
      }
      else {
        iPos = (DocPos)-1; // already at document begin or end => not found
      }
    }
  }
  if (iPos >= 0) {
    if (fMode != FRMOD_IGNORE) {
      Globals.FindReplaceMatchFoundState = bFindPrev ? 
        ((fMode == FRMOD_WRAPED) ? PRV_WRP_FND : PRV_FND) : 
        ((fMode == FRMOD_WRAPED) ? NXT_WRP_FND : NXT_FND);
    }
    // found in range, set begin and end of finding
    *start = SciCall_GetTargetStart();
    *end = SciCall_GetTargetEnd();
  }
  else {
    if (fMode != FRMOD_IGNORE) {
      Globals.FindReplaceMatchFoundState = (fMode != FRMOD_WRAPED) ? (bFindPrev ? PRV_NOT_FND : NXT_NOT_FND) : FND_NOP;
    }
  }

  SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore

  return iPos;
}


//=============================================================================
//
//  _FindHasMatch()
//
typedef enum { MATCH = 0, NO_MATCH = 1, INVALID = 2 } RegExResult_t;

static RegExResult_t  _FindHasMatch(HWND hwnd, LPCEDITFINDREPLACE lpefr, DocPos iStartPos, bool bMarkAll, bool bFirstMatchOnly)
{
  char szFind[FNDRPL_BUFFER];
  DocPos slen = _EditGetFindStrg(hwnd, lpefr, szFind, COUNTOF(szFind));
  if (slen == 0) { return NO_MATCH; }

  const DocPos iStart = bFirstMatchOnly ? iStartPos : 0;
  const DocPos iTextEnd = Sci_GetDocEndPosition();

  DocPos start = iStart;
  DocPos end   = iTextEnd;
  const DocPos iPos  = _FindInTarget(hwnd, szFind, slen, (int)(lpefr->fuFlags), &start, &end, false, FRMOD_IGNORE);

  if (bFirstMatchOnly && !Globals.bReplaceInitialized) {
    if (GetForegroundWindow() == Globals.hwndDlgFindReplace) {
      if (iPos >= 0) {
        SciCall_SetSel(start, end);
        SciCall_ScrollRange(iPos, iPos);
      }
      else {
        SciCall_ScrollCaret();
      }
    }
  }
  else // mark all matches
  {
    if (bMarkAll) {
      EditClearAllOccurrenceMarkers(hwnd);
      if (iPos >= 0) {
        EditMarkAll(hwnd, szFind, (int)(lpefr->fuFlags), 0, iTextEnd);
        if (FocusedView.HideNonMatchedLines) { EditHideNotMarkedLineRange(lpefr->hwnd, true); }
      }
      else {
        if (FocusedView.HideNonMatchedLines) { EditHideNotMarkedLineRange(lpefr->hwnd, false); }
      }
    }
  }
  return ((iPos >= 0) ? MATCH : ((iPos == (DocPos)(-1)) ? NO_MATCH : INVALID));
}


//=============================================================================
//
//  _DelayMarkAll()
//  
//
static void  _DelayMarkAll(HWND hwnd, int delay, DocPos iStartPos)
{
  static CmdMessageQueue_t mqc = MQ_WM_CMD_INIT(IDT_TIMER_MAIN_MRKALL, 0);
  mqc.hwnd = hwnd;
  mqc.lparam = (LPARAM)iStartPos;

  _MQ_AppendCmd(&mqc, (UINT)(delay <= 0 ? 0 : _MQ_ms(delay)));
}


//=============================================================================
//
//  EditFindReplaceDlgProcW()
//
static char s_lastFind[FNDRPL_BUFFER] = { L'\0' };
static WCHAR s_tchBuf[FNDRPL_BUFFER] = { L'\0' };
static WCHAR s_tchBuf2[FNDRPL_BUFFER] = { L'\0' };

static bool s_SaveMarkOccurrences = false;
static bool s_SaveMarkMatchVisible = false;
static bool s_SaveTFBackSlashes = false;

static INT_PTR CALLBACK EditFindReplaceDlgProcW(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  static LPEDITFINDREPLACE sg_pefrData = NULL;
  static DocPos s_InitialSearchStart = 0;
  static DocPos s_InitialAnchorPos = 0;
  static DocPos s_InitialCaretPos = 0;
  static DocLn  s_InitialTopLine = -1;

  static RegExResult_t s_anyMatch = NO_MATCH;
  static RegExResult_t s_fwrdMatch = NO_MATCH;

  static HBRUSH hBrushRed;
  static HBRUSH hBrushGreen;
  static HBRUSH hBrushBlue;

  switch (umsg)
  {
  case WM_INITDIALOG:
    {
      // the global static Find/Replace data structure
      SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
      if (Globals.hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIcon); }
      ResizeDlg_InitX(hwnd, Settings.FindReplaceDlgSizeX, IDC_RESIZEGRIP);

      //sg_pefrData = (LPEDITFINDREPLACE)lParam;
      sg_pefrData = (LPEDITFINDREPLACE)GetWindowLongPtr(hwnd, DWLP_USER);

      Globals.iReplacedOccurrences = 0;
      Globals.FindReplaceMatchFoundState = FND_NOP;

      s_SaveMarkOccurrences = s_bSwitchedFindReplace ? s_SaveMarkOccurrences : Settings.MarkOccurrences;
      s_SaveMarkMatchVisible = s_bSwitchedFindReplace ? s_SaveMarkMatchVisible : Settings.MarkOccurrencesMatchVisible;
      // switch off normal mark occurrences
      Settings.MarkOccurrences = false;
      Settings.MarkOccurrencesMatchVisible = false;
      EnableCmd(GetMenu(Globals.hwndMain), IDM_VIEW_MARKOCCUR_ONOFF, false);

      // Load MRUs
      for (int i = 0; i < MRU_Count(Globals.pMRUfind); i++) {
        MRU_Enum(Globals.pMRUfind, i, s_tchBuf, COUNTOF(s_tchBuf));
        SendDlgItemMessage(hwnd, IDC_FINDTEXT, CB_ADDSTRING, 0, (LPARAM)s_tchBuf);
      }
      for (int i = 0; i < MRU_Count(Globals.pMRUreplace); i++) {
        MRU_Enum(Globals.pMRUreplace, i, s_tchBuf, COUNTOF(s_tchBuf));
        SendDlgItemMessage(hwnd, IDC_REPLACETEXT, CB_ADDSTRING, 0, (LPARAM)s_tchBuf);
      }

      SendDlgItemMessage(hwnd, IDC_FINDTEXT, CB_LIMITTEXT, FNDRPL_BUFFER, 0);
      SendDlgItemMessage(hwnd, IDC_FINDTEXT, CB_SETEXTENDEDUI, true, 0);
      //const HWND hwndItem = (HWND)SendDlgItemMessage(hwnd, IDC_FINDTEXT, CBEM_GETEDITCONTROL, 0, 0);
      COMBOBOXINFO infoF = { sizeof(COMBOBOXINFO) };
      GetComboBoxInfo(GetDlgItem(hwnd, IDC_FINDTEXT), &infoF);
      //SHAutoComplete(infoF.hwndItem, SHACF_DEFAULT);
      if (infoF.hwndItem) {
        SHAutoComplete(infoF.hwndItem, SHACF_FILESYS_ONLY | SHACF_AUTOAPPEND_FORCE_OFF | SHACF_AUTOSUGGEST_FORCE_OFF);
      }
      if (!GetWindowTextLengthW(GetDlgItem(hwnd, IDC_FINDTEXT))) {
        SetDlgItemTextMB2W(hwnd, IDC_FINDTEXT, sg_pefrData->szFind);
      }
      if (GetDlgItem(hwnd, IDC_REPLACETEXT))
      {
        SendDlgItemMessage(hwnd, IDC_REPLACETEXT, CB_LIMITTEXT, FNDRPL_BUFFER, 0);
        SendDlgItemMessage(hwnd, IDC_REPLACETEXT, CB_SETEXTENDEDUI, true, 0);
        //const HWND hwndItem = (HWND)SendDlgItemMessage(hwnd, IDC_REPLACETEXT, CBEM_GETEDITCONTROL, 0, 0);
        COMBOBOXINFO infoR = { sizeof(COMBOBOXINFO) };
        GetComboBoxInfo(GetDlgItem(hwnd, IDC_REPLACETEXT), &infoR);
        //SHAutoComplete(infoR.hwndItem, SHACF_DEFAULT);
        SHAutoComplete(infoR.hwndItem, SHACF_FILESYS_ONLY | SHACF_AUTOAPPEND_FORCE_OFF | SHACF_AUTOSUGGEST_FORCE_OFF);
        
        SetDlgItemTextMB2W(hwnd, IDC_REPLACETEXT, sg_pefrData->szReplace);
      }

      if (sg_pefrData->fuFlags & SCFIND_MATCHCASE)
        CheckDlgButton(hwnd, IDC_FINDCASE, BST_CHECKED);

      if (sg_pefrData->fuFlags & SCFIND_WHOLEWORD)
        CheckDlgButton(hwnd, IDC_FINDWORD, BST_CHECKED);

      if (sg_pefrData->fuFlags & SCFIND_WORDSTART)
        CheckDlgButton(hwnd, IDC_FINDSTART, BST_CHECKED);

      if (sg_pefrData->bTransformBS) {
        CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, BST_CHECKED);
      }
      s_SaveTFBackSlashes = sg_pefrData->bTransformBS;

      if (sg_pefrData->fuFlags & SCFIND_REGEXP) {
        CheckDlgButton(hwnd, IDC_FINDREGEXP, BST_CHECKED);
        CheckDlgButton(hwnd, IDC_WILDCARDSEARCH, BST_UNCHECKED);
        DialogEnableWindow(hwnd, IDC_DOT_MATCH_ALL, true);
      }

      if (sg_pefrData->bDotMatchAll) {
        CheckDlgButton(hwnd, IDC_DOT_MATCH_ALL, BST_CHECKED);
      }

      if (sg_pefrData->bWildcardSearch) {
        CheckDlgButton(hwnd, IDC_FINDREGEXP, BST_UNCHECKED);
        CheckDlgButton(hwnd, IDC_WILDCARDSEARCH, BST_CHECKED);
        DialogEnableWindow(hwnd, IDC_DOT_MATCH_ALL, false);
      }

      if (sg_pefrData->bMarkOccurences) {
        CheckDlgButton(hwnd, IDC_ALL_OCCURRENCES, BST_CHECKED);
      }
      else {
        CheckDlgButton(hwnd, IDC_ALL_OCCURRENCES, BST_UNCHECKED);
        EditClearAllOccurrenceMarkers(sg_pefrData->hwnd);
        Globals.iMarkOccurrencesCount = (DocPos)-1;
      }

      if (sg_pefrData->fuFlags & SCFIND_REGEXP) {
        CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, BST_CHECKED);
        DialogEnableWindow(hwnd, IDC_FINDTRANSFORMBS, false);
      }
      else {
        DialogEnableWindow(hwnd, IDC_DOT_MATCH_ALL, false);
      }

      if (sg_pefrData->bNoFindWrap) {
        CheckDlgButton(hwnd, IDC_NOWRAP, BST_CHECKED);
      }

      if (GetDlgItem(hwnd, IDC_REPLACE)) {
        if (s_bSwitchedFindReplace) {
          if (sg_pefrData->bFindClose)
            CheckDlgButton(hwnd, IDC_FINDCLOSE, BST_CHECKED);
        }
        else {
          if (sg_pefrData->bReplaceClose)
            CheckDlgButton(hwnd, IDC_FINDCLOSE, BST_CHECKED);
        }
      }
      else {
        if (s_bSwitchedFindReplace) {
          if (sg_pefrData->bReplaceClose)
            CheckDlgButton(hwnd, IDC_FINDCLOSE, BST_CHECKED);
        }
        else {
          if (sg_pefrData->bFindClose)
            CheckDlgButton(hwnd, IDC_FINDCLOSE, BST_CHECKED);
        }
      }

      CheckDlgButton(hwnd, IDC_TRANSPARENT, SetBtn(Settings.FindReplaceTransparentMode));

      if (!s_bSwitchedFindReplace) {
        if (Settings.FindReplaceDlgPosX == CW_USEDEFAULT || Settings.FindReplaceDlgPosY == CW_USEDEFAULT)
          CenterDlgInParent(hwnd);
        else
          SetDlgPos(hwnd, Settings.FindReplaceDlgPosX, Settings.FindReplaceDlgPosY);
      }
      else {
        SetDlgPos(hwnd, s_xFindReplaceDlgSave, s_yFindReplaceDlgSave);
        s_bSwitchedFindReplace = false;
        CopyMemory(sg_pefrData, &s_efrSave, sizeof(EDITFINDREPLACE));
      }
      _SetSearchFlags(hwnd, sg_pefrData); // sync
      s_fwrdMatch = NO_MATCH;
      s_anyMatch = (Sci_IsMultiOrRectangleSelection() || SciCall_IsSelectionEmpty() ? NO_MATCH : MATCH);

      HMENU hmenu = GetSystemMenu(hwnd, false);
      GetLngString(IDS_MUI_SAVEPOS, s_tchBuf, COUNTOF(s_tchBuf));
      InsertMenu(hmenu, 0, MF_BYPOSITION | MF_STRING | MF_ENABLED, IDS_MUI_SAVEPOS, s_tchBuf);
      GetLngString(IDS_MUI_RESETPOS, s_tchBuf, COUNTOF(s_tchBuf));
      InsertMenu(hmenu, 1, MF_BYPOSITION | MF_STRING | MF_ENABLED, IDS_MUI_RESETPOS, s_tchBuf);
      InsertMenu(hmenu, 2, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);

      hBrushRed = CreateSolidBrush(rgbRedColorRef);
      hBrushGreen = CreateSolidBrush(rgbGreenColorRef);
      hBrushBlue = CreateSolidBrush(rgbBlueColorRef);
      
      s_InitialAnchorPos = SciCall_GetAnchor();
      s_InitialCaretPos = SciCall_GetCurrentPos();
      s_InitialTopLine = SciCall_GetFirstVisibleLine();

      // find first occurrence of clip-board text
      //if (!Sci_IsMultiOrRectangleSelection() && SciCall_IsSelectionEmpty()) {
      //  PostWMCommand(hwnd, IDOK);
      //}

      SetTimer(hwnd, IDT_TIMER_MRKALL, USER_TIMER_MINIMUM, MQ_ExecuteNext);

      _DelayMarkAll(hwnd, 0, s_InitialSearchStart);
    }
    return true;


  case WM_DPICHANGED:
    UpdateWindowLayoutForDPI(hwnd, 0, 0, 0, 0);
    return true;
  
  
  case WM_DESTROY:
      {
        if (!s_bSwitchedFindReplace)
        {
          if (s_anyMatch == MATCH) {
            // Save MRUs
            if (StringCchLenA(sg_pefrData->szFind, COUNTOF(sg_pefrData->szFind))) {
              if (GetDlgItemText(hwnd, IDC_FINDTEXT, s_tchBuf, COUNTOF(s_tchBuf))) {
                MRU_Add(Globals.pMRUfind, s_tchBuf, 0, 0, NULL);
                SetFindPattern(s_tchBuf);
              }
            }
          }
          sg_pefrData->szFind[0] = '\0';

          Globals.iReplacedOccurrences = 0;
          Globals.FindReplaceMatchFoundState = FND_NOP;

          Settings.MarkOccurrences = s_SaveMarkOccurrences;
          Settings.MarkOccurrencesMatchVisible = s_SaveMarkMatchVisible;
          EnableCmd(GetMenu(Globals.hwndMain), IDM_VIEW_MARKOCCUR_ONOFF, true);

          if (FocusedView.HideNonMatchedLines) {
            EditToggleView(sg_pefrData->hwnd);
          }

          if (IsMarkOccurrencesEnabled()) {
            MarkAllOccurrences(50, true);
          }
          else {
            EditClearAllOccurrenceMarkers(sg_pefrData->hwnd);
            Globals.iMarkOccurrencesCount = (DocPos)-1;
          }

          if (s_InitialTopLine >= 0) { 
            SciCall_SetFirstVisibleLine(s_InitialTopLine); 
            s_InitialTopLine = -1;  // reset
          }
          else {
            if (s_fwrdMatch == NO_MATCH) {
              EditSetSelectionEx(sg_pefrData->hwnd, s_InitialAnchorPos, s_InitialCaretPos, -1, -1);
            }
            else {
              EditEnsureSelectionVisible(sg_pefrData->hwnd);
            }
          }

          CmdMessageQueue_t* pmqc = NULL;
          CmdMessageQueue_t* dummy;
          DL_FOREACH_SAFE(MessageQueue, pmqc, dummy)
          {
            DL_DELETE(MessageQueue, pmqc);
            FreeMem(pmqc);
          }
        }

        KillTimer(hwnd, IDT_TIMER_MRKALL);
        DeleteObject(hBrushRed);
        DeleteObject(hBrushGreen);
        DeleteObject(hBrushBlue);
        ResizeDlg_Destroy(hwnd, &Settings.FindReplaceDlgSizeX, NULL);
      }
      return false;


  case WM_SIZE: {
      int dx;
      bool const isReplace = (GetDlgItem(hwnd, IDC_REPLACETEXT) != NULL);
      ResizeDlg_Size(hwnd, lParam, &dx, NULL);
      HDWP hdwp = BeginDeferWindowPos(isReplace ? 15 : 12);
      hdwp = DeferCtlPos(hdwp, hwnd, IDC_RESIZEGRIP, dx, 0, SWP_NOSIZE);
      hdwp = DeferCtlPos(hdwp, hwnd, IDOK, dx, 0, SWP_NOSIZE);
      hdwp = DeferCtlPos(hdwp, hwnd, IDCANCEL, dx, 0, SWP_NOSIZE);
      hdwp = DeferCtlPos(hdwp, hwnd, IDC_FINDTEXT, dx, 0, SWP_NOMOVE);
      hdwp = DeferCtlPos(hdwp, hwnd, IDC_FINDPREV, dx, 0, SWP_NOSIZE);
      hdwp = DeferCtlPos(hdwp, hwnd, IDC_TOGGLE_VISIBILITY, dx, 0, SWP_NOSIZE);
      hdwp = DeferCtlPos(hdwp, hwnd, IDS_FR_STATUS_TEXT, dx, 0, SWP_NOMOVE);
      if (isReplace) {
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_REPLACETEXT, dx, 0, SWP_NOMOVE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_SWAPSTRG, dx, 0, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_REPLACE, dx, 0, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_REPLACEINSEL, dx, 0, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_REPLACEALL, dx, 0, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_REPLACEINSEL, dx, 0, SWP_NOSIZE);
      }
      EndDeferWindowPos(hdwp);
    }
    return true;


  case WM_GETMINMAXINFO:
    ResizeDlg_GetMinMaxInfo(hwnd, lParam);
    return true;
  

  case WM_ACTIVATE:
      {
        switch (LOWORD(wParam)) 
        {
        case WA_INACTIVE:
          SetWindowTransparentMode(hwnd, Settings.FindReplaceTransparentMode, Settings2.FindReplaceOpacityLevel);
          break;

        case WA_CLICKACTIVE:
          // mouse click activation
        case WA_ACTIVE:
          SetWindowTransparentMode(hwnd, false, 100);

          s_fwrdMatch = NO_MATCH;
          s_InitialSearchStart = SciCall_GetSelectionStart();

          if (s_InitialTopLine < 0) {
            s_InitialAnchorPos = SciCall_GetAnchor();
            s_InitialCaretPos = SciCall_GetCurrentPos();
            s_InitialTopLine = SciCall_GetFirstVisibleLine();
          }

          if (!SciCall_IsSelectionEmpty()) {
            EditEnsureSelectionVisible(Globals.hwndEdit);
          }

          bool const bEnableReplInSel = !(SciCall_IsSelectionEmpty() || Sci_IsMultiOrRectangleSelection());
          DialogEnableWindow(hwnd, IDC_REPLACEINSEL, bEnableReplInSel);

          _DelayMarkAll(hwnd, 50, s_InitialSearchStart);

          break;

        default:
          break;
        }
      }
      return false;


    case WM_COMMAND:
    {
      switch (LOWORD(wParam))
      {

      case IDC_DOC_MODIFIED:
        sg_pefrData->bStateChanged = true;
        s_InitialSearchStart = SciCall_GetSelectionStart();
        s_InitialTopLine = -1;  // reset
        break;

      case IDC_FINDTEXT:
      case IDC_REPLACETEXT:
      {
        static char szFind[FNDRPL_BUFFER] = { '\0' };
        static char szCmpBuf[FNDRPL_BUFFER] = { '\0' };

        if (Globals.bFindReplCopySelOrClip)
        {
          char *lpszSelection = NULL;
          s_tchBuf[0] = L'\0';

          DocPos const cchSelection = SciCall_GetSelText(NULL);
          if ((1 < cchSelection) && !(GetDlgItem(hwnd, IDC_REPLACE) && Sci_IsMultiLineSelection())) {
            lpszSelection = AllocMem(cchSelection, HEAP_ZERO_MEMORY);
            SciCall_GetSelText(lpszSelection);
          }
          else { // (cchSelection <= 1)
            // nothing is selected in the editor:
            // if first time you bring up find/replace dialog,
            // use most recent search pattern to find box
            GetFindPattern(s_tchBuf, FNDRPL_BUFFER);
            if (s_tchBuf[0] == L'\0') {
              MRU_Enum(Globals.pMRUfind, 0, s_tchBuf, COUNTOF(s_tchBuf));
            }
            // no recent find pattern: copy content clipboard to find box
            if (s_tchBuf[0] == L'\0') {
              char* pClip = EditGetClipboardText(Globals.hwndEdit, false, NULL, NULL);
              if (pClip) {
                size_t const len = StringCchLenA(pClip, 0);
                if (len) {
                  lpszSelection = AllocMem(len + 1, HEAP_ZERO_MEMORY);
                  StringCchCopyNA(lpszSelection, len + 1, pClip, len);
                }
                FreeMem(pClip);
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

            StringCchCopyNA(szFind, FNDRPL_BUFFER, lpszSelection, SizeOfMem(lpszSelection));

            SetDlgItemTextMB2W(hwnd, IDC_FINDTEXT, szFind);
            FreeMem(lpszSelection);
          }
          else {
            if (s_tchBuf[0] == L'\0') {
              GetFindPattern(s_tchBuf, FNDRPL_BUFFER);
            }
            // cppcheck-suppress duplicateCondition  // s_tchBuf may have changed
            if (s_tchBuf[0] == L'\0') {
              MRU_Enum(Globals.pMRUfind, 0, s_tchBuf, COUNTOF(s_tchBuf));
            }
            SetDlgItemText(hwnd, IDC_FINDTEXT, s_tchBuf);
            GetDlgItemTextW2MB(hwnd, IDC_FINDTEXT, szFind, FNDRPL_BUFFER);
          }
          Globals.bFindReplCopySelOrClip = false;
          s_anyMatch = s_fwrdMatch = NO_MATCH;
        }

        GetDlgItemTextW2MB(hwnd, IDC_FINDTEXT, szCmpBuf, FNDRPL_BUFFER);
        if ((StringCchCompareXA(szCmpBuf, szFind) != 0)) {
          s_InitialTopLine = -1;  // reset
          StringCchCopyNA(szFind, FNDRPL_BUFFER, szCmpBuf, FNDRPL_BUFFER);
        }

        bool const bEnableF = (GetWindowTextLengthW(GetDlgItem(hwnd, IDC_FINDTEXT)) ||
          CB_ERR != SendDlgItemMessage(hwnd, IDC_FINDTEXT, CB_GETCURSEL, 0, 0));

        bool const bEnableR = (GetWindowTextLengthW(GetDlgItem(hwnd, IDC_REPLACETEXT)) ||
          CB_ERR != SendDlgItemMessage(hwnd, IDC_REPLACETEXT, CB_GETCURSEL, 0, 0));

        bool const bEnableIS = !(SciCall_IsSelectionEmpty() || Sci_IsMultiOrRectangleSelection());

        DialogEnableWindow(hwnd, IDOK, bEnableF);
        DialogEnableWindow(hwnd, IDC_FINDPREV, bEnableF);
        DialogEnableWindow(hwnd, IDC_REPLACE, bEnableF);
        DialogEnableWindow(hwnd, IDC_REPLACEALL, bEnableF);
        DialogEnableWindow(hwnd, IDC_REPLACEINSEL, bEnableF && bEnableIS);
        DialogEnableWindow(hwnd, IDC_SWAPSTRG, bEnableF || bEnableR);

        if (!bEnableF) { s_anyMatch = s_fwrdMatch = NO_MATCH; }

        if (HIWORD(wParam) == CBN_CLOSEUP) {
          LONG lSelEnd;
          SendDlgItemMessage(hwnd, LOWORD(wParam), CB_GETEDITSEL, 0, (LPARAM)&lSelEnd);
          SendDlgItemMessage(hwnd, LOWORD(wParam), CB_SETEDITSEL, 0, MAKELPARAM(lSelEnd, lSelEnd));
        }
        if (HIWORD(wParam) == CBN_EDITCHANGE) {
          _SetSearchFlags(hwnd, sg_pefrData);
          _DelayMarkAll(hwnd, 50, s_InitialSearchStart);
        }
      }
      break;

      case IDT_TIMER_MAIN_MRKALL:
        if (sg_pefrData) {
          _SetSearchFlags(hwnd, sg_pefrData);
          if (sg_pefrData->bMarkOccurences) {
            if (sg_pefrData->bStateChanged || (StringCchCompareXA(s_lastFind, sg_pefrData->szFind) != 0)) {
              _IGNORE_NOTIFY_CHANGE_;
              EditClearAllOccurrenceMarkers(sg_pefrData->hwnd);
              StringCchCopyA(s_lastFind, COUNTOF(s_lastFind), sg_pefrData->szFind);
              RegExResult_t match = _FindHasMatch(sg_pefrData->hwnd, sg_pefrData, 0, (sg_pefrData->bMarkOccurences), false);
              if (s_anyMatch != match) { s_anyMatch = match; }
              // we have to set Sci's regex instance to first find (have substitution in place)
              DocPos const iStartPos = (DocPos)lParam;
              if (!GetDlgItem(hwnd, IDC_REPLACE) || !Sci_IsMultiLineSelection()) {
                s_fwrdMatch = _FindHasMatch(sg_pefrData->hwnd, sg_pefrData, iStartPos, false, true);
              }
              else {
                s_fwrdMatch = match;
              }
              InvalidateRect(GetDlgItem(hwnd, IDC_FINDTEXT), NULL, true);
              if (match != MATCH) {
                EditClearAllOccurrenceMarkers(sg_pefrData->hwnd);
                if (s_InitialTopLine >= 0) {
                  SciCall_SetFirstVisibleLine(s_InitialTopLine);
                }
                else {
                  EditSetSelectionEx(sg_pefrData->hwnd, s_InitialAnchorPos, s_InitialCaretPos, -1, -1);
                }
                if (FocusedView.HideNonMatchedLines) {
                  EditToggleView(sg_pefrData->hwnd);
                }
                MarkAllOccurrences(4, true);
              }
              _OBSERVE_NOTIFY_CHANGE_;
            }
          }
          else if (sg_pefrData->bStateChanged) {
            if (FocusedView.HideNonMatchedLines) {
              SendWMCommand(hwnd, IDC_TOGGLE_VISIBILITY);
            }
            else {
              EditClearAllOccurrenceMarkers(sg_pefrData->hwnd);
            }
          }
          sg_pefrData->bStateChanged = false;
        }
        return false;


      case IDC_ALL_OCCURRENCES:
        {
          _SetSearchFlags(hwnd, sg_pefrData);

          if (IsButtonChecked(hwnd, IDC_ALL_OCCURRENCES))
          {
            DialogEnableWindow(hwnd, IDC_TOGGLE_VISIBILITY, true);
            _DelayMarkAll(hwnd, 0, s_InitialSearchStart);
          }
          else {  // switched OFF
            DialogEnableWindow(hwnd, IDC_TOGGLE_VISIBILITY, false);
            if (FocusedView.HideNonMatchedLines) {
              EditToggleView(sg_pefrData->hwnd);
            }
            EditClearAllOccurrenceMarkers(sg_pefrData->hwnd);
            Globals.iMarkOccurrencesCount = (DocPos)-1;
            InvalidateRect(GetDlgItem(hwnd, IDC_FINDTEXT), NULL, true);
          }
        }
        break;


      case IDC_TOGGLE_VISIBILITY:
        EditToggleView(sg_pefrData->hwnd);
        if (!FocusedView.HideNonMatchedLines) {
          sg_pefrData->bStateChanged = true;
          s_InitialTopLine = -1;  // reset
          EditClearAllOccurrenceMarkers(sg_pefrData->hwnd);
          _DelayMarkAll(hwnd, 0, s_InitialSearchStart);
        }
        break;


      case IDC_FINDREGEXP:
        if (IsButtonChecked(hwnd, IDC_FINDREGEXP))
        {
          DialogEnableWindow(hwnd, IDC_DOT_MATCH_ALL, true);
          CheckDlgButton(hwnd, IDC_WILDCARDSEARCH, BST_UNCHECKED); // Can not use wildcard search together with regexp
          CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, SetBtn(s_SaveTFBackSlashes));
          CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, BST_CHECKED); // transform BS handled by regex
          DialogEnableWindow(hwnd, IDC_FINDTRANSFORMBS, false);
        }
        else { // unchecked
          DialogEnableWindow(hwnd, IDC_DOT_MATCH_ALL, false);
          DialogEnableWindow(hwnd, IDC_FINDTRANSFORMBS, true);
          CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, SetBtn(s_SaveTFBackSlashes));
        }
        _SetSearchFlags(hwnd, sg_pefrData);
        _DelayMarkAll(hwnd, 0, s_InitialSearchStart);
        break;

      case IDC_DOT_MATCH_ALL:
        _SetSearchFlags(hwnd, sg_pefrData);
        _DelayMarkAll(hwnd, 0, s_InitialSearchStart);
        break;

      case IDC_WILDCARDSEARCH:
        if (IsButtonChecked(hwnd, IDC_WILDCARDSEARCH))
        {
          CheckDlgButton(hwnd, IDC_FINDREGEXP, BST_UNCHECKED);
          DialogEnableWindow(hwnd, IDC_DOT_MATCH_ALL, false);
          CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, SetBtn(s_SaveTFBackSlashes));
          CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, BST_CHECKED);  // transform BS handled by regex
          DialogEnableWindow(hwnd, IDC_FINDTRANSFORMBS, false);
        }
        else { // unchecked
          DialogEnableWindow(hwnd, IDC_FINDTRANSFORMBS, true);
          CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, SetBtn(s_SaveTFBackSlashes));
        }
        _SetSearchFlags(hwnd, sg_pefrData);
        _DelayMarkAll(hwnd, 0, s_InitialSearchStart);
        break;

      case IDC_FINDTRANSFORMBS:
        s_SaveTFBackSlashes = IsButtonChecked(hwnd, IDC_FINDTRANSFORMBS);
        _SetSearchFlags(hwnd, sg_pefrData);
        _DelayMarkAll(hwnd, 0, s_InitialSearchStart);
        break;

      case IDC_FINDCASE:
        _SetSearchFlags(hwnd, sg_pefrData);
        _DelayMarkAll(hwnd, 0, s_InitialSearchStart);
        break;

      case IDC_FINDWORD:
        _SetSearchFlags(hwnd, sg_pefrData);
        _DelayMarkAll(hwnd, 0, s_InitialSearchStart);
        break;

      case IDC_FINDSTART:
        _SetSearchFlags(hwnd, sg_pefrData);
        _DelayMarkAll(hwnd, 0, s_InitialSearchStart);
        break;

      case IDC_TRANSPARENT:
        Settings.FindReplaceTransparentMode = IsButtonChecked(hwnd, IDC_TRANSPARENT);
        break;

      case IDC_REPLACE:
      case IDC_REPLACEALL:
      case IDC_REPLACEINSEL:
        Globals.iReplacedOccurrences = 0;
      case IDOK:
      case IDC_FINDPREV:
      case IDACC_SELTONEXT:
      case IDACC_SELTOPREV:
      case IDMSG_SWITCHTOFIND:
      case IDMSG_SWITCHTOREPLACE:
      {
        bool bIsFindDlg = (GetDlgItem(Globals.hwndDlgFindReplace, IDC_REPLACE) == NULL);

        if ((bIsFindDlg && LOWORD(wParam) == IDMSG_SWITCHTOREPLACE) ||
           (!bIsFindDlg && LOWORD(wParam) == IDMSG_SWITCHTOFIND)) {
          GetDlgPos(hwnd, &s_xFindReplaceDlgSave, &s_yFindReplaceDlgSave);
          s_bSwitchedFindReplace = true;
          CopyMemory(&s_efrSave, sg_pefrData, sizeof(EDITFINDREPLACE));
        }

        if (!s_bSwitchedFindReplace &&
          !GetDlgItemTextW2MB(hwnd, IDC_FINDTEXT, sg_pefrData->szFind, COUNTOF(sg_pefrData->szFind))) {
          DialogEnableWindow(hwnd, IDOK, false);
          DialogEnableWindow(hwnd, IDC_FINDPREV, false);
          DialogEnableWindow(hwnd, IDC_REPLACE, false);
          DialogEnableWindow(hwnd, IDC_REPLACEALL, false);
          DialogEnableWindow(hwnd, IDC_REPLACEINSEL, false);
          if (!GetDlgItemTextW2MB(hwnd, IDC_REPLACETEXT, sg_pefrData->szReplace, COUNTOF(sg_pefrData->szReplace)))
            DialogEnableWindow(hwnd, IDC_SWAPSTRG, false);
          return true;
        }

        _SetSearchFlags(hwnd, sg_pefrData);

        if (!s_bSwitchedFindReplace) {
          // Save MRUs
          if (StringCchLenA(sg_pefrData->szFind, COUNTOF(sg_pefrData->szFind))) {
            if (GetDlgItemText(hwnd, IDC_FINDTEXT, s_tchBuf2, COUNTOF(s_tchBuf2))) {
              MRU_Add(Globals.pMRUfind, s_tchBuf2, 0, 0, NULL);
              SetFindPattern(s_tchBuf2);
            }
          }
          if (StringCchLenA(sg_pefrData->szReplace, COUNTOF(sg_pefrData->szReplace))) {
            if (GetDlgItemText(hwnd, IDC_REPLACETEXT, s_tchBuf2, COUNTOF(s_tchBuf2))) {
              MRU_Add(Globals.pMRUreplace, s_tchBuf2, 0, 0, NULL);
            }
          }
        }

        // Reload MRUs
        SendDlgItemMessage(hwnd, IDC_FINDTEXT, CB_RESETCONTENT, 0, 0);
        SendDlgItemMessage(hwnd, IDC_REPLACETEXT, CB_RESETCONTENT, 0, 0);

        for (int i = 0; i < MRU_Count(Globals.pMRUfind); i++) {
          MRU_Enum(Globals.pMRUfind, i, s_tchBuf2, COUNTOF(s_tchBuf2));
          SendDlgItemMessage(hwnd, IDC_FINDTEXT, CB_ADDSTRING, 0, (LPARAM)s_tchBuf2);
        }
        for (int i = 0; i < MRU_Count(Globals.pMRUreplace); i++) {
          MRU_Enum(Globals.pMRUreplace, i, s_tchBuf2, COUNTOF(s_tchBuf2));
          SendDlgItemMessage(hwnd, IDC_REPLACETEXT, CB_ADDSTRING, 0, (LPARAM)s_tchBuf2);
        }

        SetDlgItemTextMB2W(hwnd, IDC_FINDTEXT, sg_pefrData->szFind);
        SetDlgItemTextMB2W(hwnd, IDC_REPLACETEXT, sg_pefrData->szReplace);

        if (!s_bSwitchedFindReplace)
          SendMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetFocus()), 1);

        bool bCloseDlg = false;
        if (bIsFindDlg) {
          bCloseDlg = sg_pefrData->bFindClose;
        }
        else if (LOWORD(wParam) != IDOK) {
          bCloseDlg = sg_pefrData->bReplaceClose;
        }

        if (bCloseDlg) {
          //EndDialog(hwnd,LOWORD(wParam));
          DestroyWindow(hwnd);
        }

        switch (LOWORD(wParam)) {
        case IDOK: // find next
        case IDACC_SELTONEXT:
          if (!bIsFindDlg) { Globals.bReplaceInitialized = true; }
          if (!SciCall_IsSelectionEmpty()) { EditJumpToSelectionEnd(sg_pefrData->hwnd); }
          EditFindNext(sg_pefrData->hwnd, sg_pefrData, (LOWORD(wParam) == IDACC_SELTONEXT), IsKeyDown(VK_F3));
          s_InitialSearchStart = SciCall_GetSelectionStart();
          s_InitialAnchorPos = SciCall_GetAnchor();
          s_InitialCaretPos = SciCall_GetCurrentPos();
          s_InitialTopLine = -1;  // reset
          break;

        case IDC_FINDPREV: // find previous
        case IDACC_SELTOPREV:
          if (!bIsFindDlg) { Globals.bReplaceInitialized = true; }
          if (!SciCall_IsSelectionEmpty()) { EditJumpToSelectionStart(sg_pefrData->hwnd);  }
          EditFindPrev(sg_pefrData->hwnd, sg_pefrData, (LOWORD(wParam) == IDACC_SELTOPREV), IsKeyDown(VK_F3));
          s_InitialSearchStart = SciCall_GetSelectionStart();
          s_InitialAnchorPos = SciCall_GetAnchor();
          s_InitialCaretPos = SciCall_GetCurrentPos();
          s_InitialTopLine = -1;  // reset
          break;

        case IDC_REPLACE:
          {
            Globals.bReplaceInitialized = true;
            _BEGIN_UNDO_ACTION_
            EditReplace(sg_pefrData->hwnd, sg_pefrData);
            _END_UNDO_ACTION_
          }
          break;

        case IDC_REPLACEALL:
          Globals.bReplaceInitialized = true;
          _BEGIN_UNDO_ACTION_
          EditReplaceAll(sg_pefrData->hwnd, sg_pefrData, true);
          _END_UNDO_ACTION_
          break;

        case IDC_REPLACEINSEL:
          if (!SciCall_IsSelectionEmpty()) {
            Globals.bReplaceInitialized = true;
            EditReplaceAllInSelection(sg_pefrData->hwnd, sg_pefrData, true);
          }
          break;
        }
      }
      _DelayMarkAll(hwnd, 50, s_InitialSearchStart);
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
        Globals.FindReplaceMatchFoundState = FND_NOP;
        _SetSearchFlags(hwnd, sg_pefrData);
        _DelayMarkAll(hwnd, 50, s_InitialSearchStart);
      }
      break;

      case IDACC_FIND:
        PostWMCommand(GetParent(hwnd), IDM_EDIT_FIND);
        break;

      case IDACC_REPLACE:
        PostWMCommand(GetParent(hwnd), IDM_EDIT_REPLACE);
        break;

      case IDACC_SAVEPOS:
        GetDlgPos(hwnd, &Settings.FindReplaceDlgPosX, &Settings.FindReplaceDlgPosY);
        break;

      case IDACC_RESETPOS:
        CenterDlgInParent(hwnd);
        Settings.FindReplaceDlgPosX = Settings.FindReplaceDlgPosY = CW_USEDEFAULT;
        break;

      case IDACC_FINDNEXT:
        //SetFocus(Globals.hwndMain);
        //SetForegroundWindow(Globals.hwndMain);
        PostWMCommand(hwnd, IDOK);
        break;

      case IDACC_FINDPREV:
        //SetFocus(Globals.hwndMain);
        //SetForegroundWindow(Globals.hwndMain);
        PostWMCommand(hwnd, IDC_FINDPREV);
        break;

      case IDACC_REPLACENEXT:
        if (GetDlgItem(hwnd, IDC_REPLACE) != NULL) {
          PostWMCommand(hwnd, IDC_REPLACE);
        }
        break;

      case IDACC_SAVEFIND:
        Globals.FindReplaceMatchFoundState = FND_NOP;
        SendWMCommand(Globals.hwndMain, IDM_EDIT_SAVEFIND);
        SetDlgItemTextMB2W(hwnd, IDC_FINDTEXT, sg_pefrData->szFind);
        CheckDlgButton(hwnd, IDC_FINDREGEXP, BST_UNCHECKED);
        CheckDlgButton(hwnd, IDC_DOT_MATCH_ALL, BST_UNCHECKED);
        CheckDlgButton(hwnd, IDC_WILDCARDSEARCH, BST_UNCHECKED);
        CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, BST_UNCHECKED);
        PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_FINDTEXT)), 1);
        break;

      case IDACC_VIEWSCHEMECONFIG:
        PostWMCommand(GetParent(hwnd), IDM_VIEW_SCHEMECONFIG);
        break;

      default:
        //return false; ???
        break;
      }

    } // WM_COMMAND:
    return true;


    case WM_SYSCOMMAND:
      if (wParam == IDS_MUI_SAVEPOS) {
        PostWMCommand(hwnd, IDACC_SAVEPOS);
        return true;
      }
      else if (wParam == IDS_MUI_RESETPOS) {
        PostWMCommand(hwnd, IDACC_RESETPOS);
        return true;
      }
      else
        return false;


    case WM_NOTIFY:
      {
        LPNMHDR pnmhdr = (LPNMHDR)lParam;
        switch (pnmhdr->code) 
        {
        case NM_CLICK:
        case NM_RETURN:
          if (pnmhdr->idFrom == IDC_TOGGLEFINDREPLACE) {
            if (GetDlgItem(hwnd, IDC_REPLACE)) {
              PostWMCommand(GetParent(hwnd), IDM_EDIT_FIND);
            }
            else {
              PostWMCommand(GetParent(hwnd), IDM_EDIT_REPLACE);
            }
          }
          // Display help messages in the find/replace windows
          else if (pnmhdr->idFrom == IDC_BACKSLASHHELP) {
            MessageBoxLng(MB_ICONINFORMATION, IDS_MUI_BACKSLASHHELP);
          }
          else if (pnmhdr->idFrom == IDC_REGEXPHELP) {
            MessageBoxLng(MB_ICONINFORMATION, IDS_MUI_REGEXPHELP);
          }
          else if (pnmhdr->idFrom == IDC_WILDCARDHELP) {
            MessageBoxLng(MB_ICONINFORMATION, IDS_MUI_WILDCARDHELP);
          }
          break;

        default:
          return false;
        }
      }
      break;

    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
      {
        if (sg_pefrData->bMarkOccurences)
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
            switch (s_anyMatch) {
            case MATCH:
              //SetTextColor(hDC, green);
              SetBkColor(hDC, rgbGreenColorRef);
              hBrush = (INT_PTR)hBrushGreen;
              break;
            case NO_MATCH:
              //SetTextColor(hDC, blue);
              SetBkColor(hDC, rgbBlueColorRef);
              hBrush = (INT_PTR)hBrushBlue;
              break;
            case INVALID:
            default:
              //SetTextColor(hDC, red);
              SetBkColor(hDC, rgbRedColorRef);
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

  return false;
}


//=============================================================================
//
//  EditFindReplaceDlg()
//
HWND EditFindReplaceDlg(HWND hwnd,LPCEDITFINDREPLACE lpefr,bool bReplace)
{
  (void)CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_SPEED_OVER_MEMORY);

  lpefr->hwnd = hwnd;
  HWND hDlg = CreateThemedDialogParam(Globals.hLngResContainer,
            (bReplace) ? MAKEINTRESOURCEW(IDD_MUI_REPLACE) : MAKEINTRESOURCEW(IDD_MUI_FIND),
            GetParent(hwnd),
            EditFindReplaceDlgProcW,
            (LPARAM) lpefr);

  if (hDlg != INVALID_HANDLE_VALUE) {
    ShowWindow(hDlg, SW_SHOW);
  }
  CoUninitialize();
  return hDlg;
}


//=============================================================================
//
//  EditFindNext()
//
bool EditFindNext(HWND hwnd, LPCEDITFINDREPLACE lpefr, bool bExtendSelection, bool bFocusWnd) {

  char szFind[FNDRPL_BUFFER];
  bool bSuppressNotFound = false;

  DocPos slen = _EditGetFindStrg(hwnd, lpefr, szFind, COUNTOF(szFind));
  if (slen <= 0)
    return false;

  if (bFocusWnd)
    SetFocus(hwnd);

  DocPos const iDocEndPos = Sci_GetDocEndPosition();

  DocPos start = SciCall_GetCurrentPos();
  DocPos end = iDocEndPos;

  if (start >= end) {
    if (IDOK == InfoBoxLng(MB_OKCANCEL, L"MsgFindWrap1", IDS_MUI_FIND_WRAPFW)) {
      end = min_p(start, iDocEndPos);  start = 0;
    }
    else
      bSuppressNotFound = true;
  }

  CancelCallTip();

  DocPos iPos = _FindInTarget(hwnd, szFind, slen, (int)(lpefr->fuFlags), &start, &end, true, FRMOD_NORM);

  if ((iPos < -1) && (lpefr->fuFlags & SCFIND_REGEXP)) {
    InfoBoxLng(MB_ICONWARNING, L"MsgInvalidRegex", IDS_MUI_REGEX_INVALID);
    bSuppressNotFound = true;
  }
  else if ((iPos < 0) && (start > 0) && !bExtendSelection) 
  {
    UpdateStatusbar(false);
    if (!lpefr->bNoFindWrap && !bSuppressNotFound) {
      if (IDOK == InfoBoxLng(MB_OKCANCEL, L"MsgFindWrap2", IDS_MUI_FIND_WRAPFW)) {
        end = min_p(start, iDocEndPos);  start = 0;

        iPos = _FindInTarget(hwnd, szFind, slen, (int)(lpefr->fuFlags), &start, &end, false, FRMOD_WRAPED);

        if ((iPos < -1) && (lpefr->fuFlags & SCFIND_REGEXP)) {
          InfoBoxLng(MB_ICONWARNING, L"MsgInvalidRegex2", IDS_MUI_REGEX_INVALID);
          bSuppressNotFound = true;
        }
      }
      else
        bSuppressNotFound = true;
    }
  }

  if (iPos < 0) {
    if (!bSuppressNotFound) {
      InfoBoxLng(MB_OK, L"MsgNotFound", IDS_MUI_NOTFOUND);
    }
    return false;
  }

  if (bExtendSelection) {
    DocPos iSelPos = SciCall_GetCurrentPos();
    DocPos iSelAnchor = SciCall_GetAnchor();
    EditSetSelectionEx(hwnd, min_p(iSelAnchor, iSelPos), end, -1, -1);
  }
  else {
    EditSetSelectionEx(hwnd, start, end, -1, -1);
  }

  if (start == end) {
    EditShowZeroLengthCallTip(hwnd, start);
  }
  
  return true;
}


//=============================================================================
//
//  EditFindPrev()
//
bool EditFindPrev(HWND hwnd, LPCEDITFINDREPLACE lpefr, bool bExtendSelection, bool bFocusWnd) {

  char szFind[FNDRPL_BUFFER];
  bool bSuppressNotFound = false;

  if (bFocusWnd)
    SetFocus(hwnd);

  DocPos slen = _EditGetFindStrg(hwnd, lpefr, szFind, COUNTOF(szFind));
  if (slen <= 0)
    return false;

  DocPos const iDocEndPos = Sci_GetDocEndPosition();
  DocPos start = SciCall_GetCurrentPos();
  DocPos end = 0;

  if (start <= end) {
    if (IDOK == InfoBoxLng(MB_OKCANCEL, L"MsgFindWrap1", IDS_MUI_FIND_WRAPFW)) {
      end = start;  start = iDocEndPos;
    }
    else
      bSuppressNotFound = true;
  }

  CancelCallTip();

  DocPos iPos = _FindInTarget(hwnd, szFind, slen, (int)(lpefr->fuFlags), &start, &end, true, FRMOD_NORM);

  if ((iPos < -1) && (lpefr->fuFlags & SCFIND_REGEXP)) 
  {
    InfoBoxLng(MB_ICONWARNING, L"MsgInvalidRegex", IDS_MUI_REGEX_INVALID);
    bSuppressNotFound = true;
  }
  else if ((iPos < 0) && (start <= iDocEndPos) &&  !bExtendSelection) 
  {
    UpdateStatusbar(false);
    if (!lpefr->bNoFindWrap && !bSuppressNotFound) 
    {
      if (IDOK == InfoBoxLng(MB_OKCANCEL, L"MsgFindWrap2", IDS_MUI_FIND_WRAPRE)) {
        end = start;  start = iDocEndPos;

        iPos = _FindInTarget(hwnd, szFind, slen, (int)(lpefr->fuFlags), &start, &end, false, FRMOD_WRAPED);

        if ((iPos < -1) && (lpefr->fuFlags & SCFIND_REGEXP)) {
          InfoBoxLng(MB_ICONWARNING, L"MsgInvalidRegex2", IDS_MUI_REGEX_INVALID);
          bSuppressNotFound = true;
        }
      }
      else
        bSuppressNotFound = true;
    }
  }

  if (iPos < 0) {
    if (!bSuppressNotFound) {
      InfoBoxLng(MB_OK, L"MsgNotFound", IDS_MUI_NOTFOUND);
    }
    return false;
  }

  if (bExtendSelection) {
    DocPos const iSelPos = SciCall_GetCurrentPos();
    DocPos const iSelAnchor = SciCall_GetAnchor();
    EditSetSelectionEx(hwnd, max_p(iSelPos, iSelAnchor), start, -1, -1);
  }
  else {
    EditSetSelectionEx(hwnd, end, start, -1, -1);
  }

  if (start == end) {
    EditShowZeroLengthCallTip(hwnd, start);
  }
  return true;
}


//=============================================================================
//
//  EditMarkAllOccurrences()
// 
void EditMarkAllOccurrences(HWND hwnd, bool bForceClear)
{
  if (bForceClear) {
    EditClearAllOccurrenceMarkers(hwnd);
  }
  
  if (!IsMarkOccurrencesEnabled()) {
    EditClearAllOccurrenceMarkers(hwnd);
    return;
  }

  bool const bWaitCursor = (Globals.iMarkOccurrencesCount > 4000) ? true : false;
  if (bWaitCursor) { BeginWaitCursor(NULL); }

  int const searchFlags = GetMarkAllOccSearchFlags();

  _IGNORE_NOTIFY_CHANGE_;

  if (Settings.MarkOccurrencesMatchVisible) {

    // get visible lines for update
    DocLn const iStartLine = SciCall_DocLineFromVisible(SciCall_GetFirstVisibleLine());
    DocLn const iEndLine = min_ln((iStartLine + SciCall_LinesOnScreen()), (SciCall_GetLineCount() - 1));
    DocPos const iPosStart = SciCall_PositionFromLine(iStartLine);
    DocPos const iPosEnd = SciCall_GetLineEndPosition(iEndLine);

    // !!! don't clear all marks, else this method is re-called
    // !!! on UpdateUI notification on drawing indicator mark
    EditMarkAll(hwnd, NULL, searchFlags, iPosStart, iPosEnd);
  }
  else {
    EditMarkAll(hwnd, NULL, searchFlags, 0, Sci_GetDocEndPosition());
  }
  
  _OBSERVE_NOTIFY_CHANGE_;

  if (bWaitCursor) { EndWaitCursor(); } 
}


//=============================================================================
//
//  EditSelectionMultiSelectAll()
//
void EditSelectionMultiSelectAll()
{
  DocPos const iSelSize = SciCall_GetSelText(NULL);
  if ((iSelSize > 1))
  {
    char* pszText = AllocMem(iSelSize, HEAP_ZERO_MEMORY);
    if (NULL == pszText) {
      FreeMem(pszText);
      return;
    }
    SciCall_GetSelText(pszText);

    int const searchFlags = IsMarkOccurrencesEnabled() ? GetMarkAllOccSearchFlags() : 
                            EditAddSearchFlags(0, false, false, true, false, false);

    SciCall_SetSearchFlags(searchFlags);

    DocPos const saveTargetBeg = SciCall_GetTargetStart();
    DocPos const saveTargetEnd = SciCall_GetTargetEnd();

    if (IsMarkOccurrencesEnabled() && Settings.MarkOccurrencesMatchVisible) 
    {
      // get visible lines for update
      DocLn const iStartLine = SciCall_DocLineFromVisible(SciCall_GetFirstVisibleLine());
      DocLn const iEndLine = min_ln((iStartLine + SciCall_LinesOnScreen()), (SciCall_GetLineCount() - 1));
      SciCall_SetTargetRange(SciCall_PositionFromLine(iStartLine), SciCall_GetLineEndPosition(iEndLine));
    }
    else {
      SciCall_TargetWholeDocument();
    }
    SciCall_MultipleSelectAddEach();
    SciCall_SetMainSelection(0);
    SciCall_ScrollRange(SciCall_GetSelectionNAnchor(0), SciCall_GetSelectionNCaret(0));
    SciCall_ChooseCaretX();

    SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore

    FreeMem(pszText);
  }
}


//=============================================================================
//
//  _GetReplaceString()
//
static char*  _GetReplaceString(HWND hwnd, LPCEDITFINDREPLACE lpefr, int* iReplaceMsg)
{
  char* pszReplace = NULL; // replace text of arbitrary size
  if (StringCchCompareNIA(lpefr->szReplace, FNDRPL_BUFFER, "^c", 2) == 0) {
    *iReplaceMsg = SCI_REPLACETARGET;
    pszReplace = EditGetClipboardText(hwnd, true, NULL, NULL);
  }
  else {
    size_t const size = StringCchLenA(lpefr->szReplace,0) + 1;
    pszReplace = (char*)AllocMem(size, HEAP_ZERO_MEMORY);
    StringCchCopyA(pszReplace, size, lpefr->szReplace);
    bool bIsRegEx = (lpefr->fuFlags & SCFIND_REGEXP);
    if (lpefr->bTransformBS || bIsRegEx) {
      TransformBackslashes(pszReplace, bIsRegEx, Encoding_SciCP, iReplaceMsg);
    }
  }
  return pszReplace;
}


//=============================================================================
//
//  EditReplace()
//
bool EditReplace(HWND hwnd, LPCEDITFINDREPLACE lpefr) {

  int iReplaceMsg = SCI_REPLACETARGET;
  char* pszReplace = _GetReplaceString(hwnd, lpefr, &iReplaceMsg);
  if (!pszReplace)
    return false; // recoding of clipboard canceled

  DocPos const selBeg = SciCall_GetSelectionStart();
  DocPos const selEnd = SciCall_GetSelectionEnd();

  // redo find to get group ranges filled
  DocPos start = (SciCall_IsSelectionEmpty() ? SciCall_GetCurrentPos() : selBeg);
  DocPos end = Sci_GetDocEndPosition();
  DocPos _start = start;
  Globals.iReplacedOccurrences = 0;

  DocPos const findLen = (DocPos)StringCchLenA(lpefr->szFind, 0);
  DocPos const iPos = _FindInTarget(hwnd, lpefr->szFind, findLen, (int)(lpefr->fuFlags), &start, &end, false, FRMOD_NORM);

  // w/o selection, replacement string is put into current position
  // but this maybe not intended here
  if (SciCall_IsSelectionEmpty()) {
    if ((iPos < 0) || (_start != start) || (_start != end)) {
      // empty-replace was not intended
      FreeMem(pszReplace);
      if (iPos < 0) {
        return EditFindNext(hwnd, lpefr, false, false);
      }
      EditSetSelectionEx(hwnd, start, end, -1, -1);
      return true;
    }
  }
  // if selection is is not equal current find, set selection
  else if ((selBeg != start) || (selEnd != end)) {
    FreeMem(pszReplace);
    SciCall_SetCurrentPos(selBeg);
    return EditFindNext(hwnd, lpefr, false, false);
  }

  DocPos const saveTargetBeg = SciCall_GetTargetStart();
  DocPos const saveTargetEnd = SciCall_GetTargetEnd();

  SciCall_TargetFromSelection();
  Sci_ReplaceTarget(iReplaceMsg, -1, pszReplace);
  // move caret behind replacement
  SciCall_SetCurrentPos(SciCall_GetTargetEnd());
  Globals.iReplacedOccurrences = 1;

  SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore
  FreeMem(pszReplace);

  return EditFindNext(hwnd, lpefr, false, false);
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

  int slen = _EditGetFindStrg(hwnd, lpefr, szFind, COUNTOF(szFind));
  if (slen <= 0) { return 0; }

  // SCI_REPLACETARGET or SCI_REPLACETARGETRE
  int iReplaceMsg = SCI_REPLACETARGET;
  char* pszReplace = _GetReplaceString(hwnd, lpefr, &iReplaceMsg);
  if (!pszReplace) {
    return -1; // recoding of clipboard canceled
  }

  UT_array* ReplPosUTArray = NULL;
  utarray_new(ReplPosUTArray, &ReplPos_icd);
  utarray_reserve(ReplPosUTArray, (2 * SciCall_GetLineCount()) );
  
  DocPos start = iStartPos;
  DocPos end = iEndPos;

  DocPos iPos = _FindInTarget(hwnd, szFind, slen, (int)(lpefr->fuFlags), &start, &end, false, FRMOD_NORM);

  if ((iPos < -1) && (lpefr->fuFlags & SCFIND_REGEXP)) {
    InfoBoxLng(MB_ICONWARNING, L"MsgInvalidRegex", IDS_MUI_REGEX_INVALID);
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
      iPos = _FindInTarget(hwnd, szFind, slen, (int)(lpefr->fuFlags), &start, &end, ((posPair.end - posPair.beg) == 0), FRMOD_IGNORE);
    else
      iPos = -1;
  } 
  
  int const iCount = utarray_len(ReplPosUTArray);
  if (iCount <= 0) { FreeMem(pszReplace); return 0; }

  // ===  iterate over findings and replace strings  ===
  DocPos searchStart = iStartPos;
  DocPos totalReplLength = 0;

  _IGNORE_NOTIFY_CHANGE_;

  for (ReplPos_t* pPosPair = (ReplPos_t*)utarray_front(ReplPosUTArray);
                  pPosPair != NULL;
                  pPosPair = (ReplPos_t*)utarray_next(ReplPosUTArray, pPosPair)) {

    start = searchStart;
    end = iEndPos + totalReplLength;

    if (iReplaceMsg == SCI_REPLACETARGETRE) 
    {
      // redo find to get group ranges filled
      /*iPos = */_FindInTarget(hwnd, szFind, slen, (int)(lpefr->fuFlags), &start, &end, false, FRMOD_IGNORE);
    }
    else {
      start = pPosPair->beg + totalReplLength;
      end = pPosPair->end + totalReplLength;
    }
    DocPos const saveTargetBeg = SciCall_GetTargetStart();
    DocPos const saveTargetEnd = SciCall_GetTargetEnd();

    SciCall_SetTargetRange(start, end);
    DocPos const replLen = Sci_ReplaceTarget(iReplaceMsg, -1, pszReplace);
    totalReplLength += replLen + pPosPair->beg - pPosPair->end;
    searchStart = (start != end) ? SciCall_GetTargetEnd() : SciCall_GetTargetEnd() + 1; // zero-find-length

    SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore
  }

  _OBSERVE_NOTIFY_CHANGE_;

  utarray_clear(ReplPosUTArray);
  utarray_free(ReplPosUTArray);
  FreeMem(pszReplace);

  *enlargement = totalReplLength;

  return iCount;
}


//=============================================================================
//
//  EditReplaceAll()
//
bool EditReplaceAll(HWND hwnd, LPCEDITFINDREPLACE lpefr, bool bShowInfo)
{
  DocPos const start = 0;
  DocPos const end = Sci_GetDocEndPosition();
  DocPos enlargement = 0;

  BeginWaitCursor(NULL);
  Globals.iReplacedOccurrences = EditReplaceAllInRange(hwnd, lpefr, start, end, &enlargement);
  EndWaitCursor();

  if (bShowInfo) {
    if (Globals.iReplacedOccurrences > 0) {
      InfoBoxLng(MB_OK, L"MsgReplaceCount", IDS_MUI_REPLCOUNT, Globals.iReplacedOccurrences);
    }
    else {
      InfoBoxLng(MB_OK, L"MsgNotFound", IDS_MUI_NOTFOUND);
    }
  }

  return (Globals.iReplacedOccurrences > 0) ? true : false;
}


//=============================================================================
//
//  EditReplaceAllInSelection()
//
bool EditReplaceAllInSelection(HWND hwnd, LPCEDITFINDREPLACE lpefr, bool bShowInfo)
{
  if (SciCall_IsSelectionRectangle()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECT);
    return false;
  }

  const DocPos start = SciCall_GetSelectionStart();
  const DocPos end = SciCall_GetSelectionEnd();
  const DocPos currPos = SciCall_GetCurrentPos();
  const DocPos anchorPos = SciCall_GetAnchor();
  DocPos enlargement = 0;

  _BEGIN_UNDO_ACTION_

  bool const bWaitCursor = ((end - start) > (512 * 512)) ? true : false;
  if (bWaitCursor) { BeginWaitCursor(NULL); }
  Globals.iReplacedOccurrences = EditReplaceAllInRange(hwnd, lpefr, start, end, &enlargement);
  if (bWaitCursor) { EndWaitCursor(); }

  if (Globals.iReplacedOccurrences > 0) 
  {
    if (currPos < anchorPos)
      SciCall_SetSel(anchorPos + enlargement, currPos);
    else
      SciCall_SetSel(anchorPos, currPos + enlargement);

    if (bShowInfo) {
      if (Globals.iReplacedOccurrences > 0) {
        InfoBoxLng(MB_OK, L"MsgReplaceCount", IDS_MUI_REPLCOUNT, Globals.iReplacedOccurrences);
      }
      else {
        InfoBoxLng(MB_OK, L"MsgNotFound", IDS_MUI_NOTFOUND);
      }
    }
  }
  _END_UNDO_ACTION_

  return (Globals.iReplacedOccurrences > 0) ? true : false;
}



//=============================================================================
//
//  EditClearAllOccurrenceMarkers()
//
void EditClearAllOccurrenceMarkers(HWND hwnd)
{
  UNUSED(hwnd);
  Globals.iMarkOccurrencesCount = 0;

  _IGNORE_NOTIFY_CHANGE_;

  SciCall_SetIndicatorCurrent(INDIC_NP3_MARK_OCCURANCE);
  SciCall_IndicatorClearRange(0, Sci_GetDocEndPosition());
  SciCall_SetIndicatorCurrent(INDIC_NP3_FOCUS_VIEW);
  SciCall_IndicatorClearRange(0, Sci_GetDocEndPosition());

  _OBSERVE_NOTIFY_CHANGE_;
}


//=============================================================================
//
//  EditToggleView()
//
void EditToggleView(HWND hwnd)
{
  bool const bWaitCursor = ((Globals.iMarkOccurrencesCount > 1000) || (SciCall_GetLineCount() > 2000)) ? true : false;
  if (bWaitCursor) { BeginWaitCursor(NULL); }

  FocusedView.HideNonMatchedLines = !FocusedView.HideNonMatchedLines; // toggle

  EditHideNotMarkedLineRange(hwnd, FocusedView.HideNonMatchedLines);

  if (FocusedView.HideNonMatchedLines) {
    EditScrollTo(hwnd, 0, false);
    SciCall_SetReadOnly(true);
  }
  else {
    EditScrollTo(hwnd, Sci_GetCurrentLineNumber(), true);
    SciCall_SetReadOnly(false);
  }

  if (bWaitCursor) { EndWaitCursor(); }
}


//=============================================================================
//
//  EditSelectWordAtPos()
//
void EditSelectWordAtPos(const DocPos iPos, const bool bForceWord)
{
  DocPos iWordStart = SciCall_WordStartPosition(iPos, true);
  DocPos iWordEnd = SciCall_WordEndPosition(iPos, true);

  if ((iWordStart == iWordEnd) && bForceWord) // we are in whitespace salad...
  {
    iWordStart = SciCall_WordEndPosition(iPos, false);
    iWordEnd = SciCall_WordEndPosition(iWordStart, true);
    if (iWordStart != iWordEnd) {
      SciCall_SetSelection(iWordEnd, iWordStart);
    }
  }
  else {
    SciCall_SetSelection(iWordEnd, iWordStart);
  }
}


//=============================================================================
//
//  EditAddSearchFlags()
//
int EditAddSearchFlags(int flags, bool bRegEx, bool bWordStart, bool bMatchCase, bool bMatchWords, bool bDotMatchAll)
{
  flags |= (bRegEx) ? SCFIND_REGEXP : 0;
  flags |= (bWordStart) ? SCFIND_WORDSTART : 0;
  flags |= (bMatchWords) ? SCFIND_WHOLEWORD : 0;
  flags |= (bMatchCase ? SCFIND_MATCHCASE : 0);
  flags |= (bDotMatchAll ? SCFIND_DOT_MATCH_ALL : 0);
  return flags;
}


//=============================================================================
//
//  EditMarkAll()
//  Mark all occurrences of the matching text in range (by Aleksandar Lekov)
//
void EditMarkAll(HWND hwnd, char* pszFind, int flags, DocPos rangeStart, DocPos rangeEnd)
{
  char txtBuffer[XHUGE_BUFFER] = { '\0' };
  char* pszText = (pszFind != NULL) ? pszFind : txtBuffer;

  DocPos iFindLength = 0;

  if (StrIsEmptyA(pszText))
  {
    if (SciCall_IsSelectionEmpty()) {
      // nothing selected, get word under caret if flagged
      if (Settings.MarkOccurrencesCurrentWord && (flags & SCFIND_WHOLEWORD))
      {
        DocPos const iCurPos = SciCall_GetCurrentPos();
        DocPos iWordStart = SciCall_WordStartPosition(iCurPos, true);
        DocPos iWordEnd = SciCall_WordEndPosition(iCurPos, true);
        if (iWordStart == iWordEnd) { return; }
        iFindLength = (iWordEnd - iWordStart);
        StringCchCopyNA(txtBuffer, COUNTOF(txtBuffer), SciCall_GetRangePointer(iWordStart, iFindLength), iFindLength);
      }
      else {
        return; // no pattern, no selection and no word mark chosen
      }
    }
    else // we have a selection
    {
      if (Sci_IsMultiSelection()) { return; }

      // get current selection
      DocPos const iSelStart = SciCall_GetSelectionStart();
      DocPos const iSelEnd = SciCall_GetSelectionEnd();
      DocPos const iSelCount = (iSelEnd - iSelStart);

      // if multiple lines are selected exit
      if ((SciCall_LineFromPosition(iSelStart) != SciCall_LineFromPosition(iSelEnd)) || (iSelCount >= COUNTOF(txtBuffer))) {
        return;
      }

      iFindLength = SciCall_GetSelText(pszText) - 1;

      // exit if selection is not a word and Match whole words only is enabled
      if (flags & SCFIND_WHOLEWORD) {
        DocPos iSelStart2 = 0;
        const char* delims = (Settings.AccelWordNavigation ? DelimCharsAccel : DelimChars);
        while ((iSelStart2 <= iSelCount) && pszText[iSelStart2]) {
          if (StrChrIA(delims, pszText[iSelStart2])) {
            return;
          }
          ++iSelStart2;
        }
      }
    }
  }
  else {
    iFindLength = (DocPos)StringCchLenA(pszFind, FNDRPL_BUFFER);
  }

  if (iFindLength > 0) {

    DocPos const iTextEnd = Sci_GetDocEndPosition();
    rangeStart = max_p(0, rangeStart);
    rangeEnd = min_p(rangeEnd, iTextEnd);

    DocPos start = rangeStart;
    DocPos end = rangeEnd;

    DocPos iPos = (DocPos)-1;
    do {

      iPos = _FindInTarget(hwnd, pszText, iFindLength, flags, &start, &end, (start == iPos), FRMOD_IGNORE);

      if (iPos < 0) {
        break; // not found
      }
      // mark this match if not done before
      SciCall_SetIndicatorCurrent(INDIC_NP3_MARK_OCCURANCE);
      SciCall_IndicatorFillRange(iPos, (end - start));
      SciCall_SetIndicatorCurrent(INDIC_NP3_FOCUS_VIEW);
      SciCall_IndicatorFillRange(Sci_GetLineStartPosition(iPos), 1);

      start = end;
      end = rangeEnd;

    } while ((++Globals.iMarkOccurrencesCount < (DocPos)Settings2.MarkOccurrencesMaxCount) && (start < end));
  }
}


//=============================================================================
//
//  EditAutoCompleteWord()
//  Auto-complete words (by Aleksandar Lekov)
//

#define _MAX_AUTOC_WORD_LEN 240

typedef struct WLIST {
  struct WLIST* next;
  char word[_MAX_AUTOC_WORD_LEN];
} WLIST, *PWLIST;


static int  wordcmp(PWLIST a, PWLIST b) {
  return StringCchCompareXA(a->word, b->word);
}

/* unused yet
static int  wordcmpi(PWLIST a, PWLIST b) {
  return StringCchCompareXIA(a->word, b->word);
}
*/

// ----------------------------------------------

static const char*  _strNextLexKeyWord(const char* strg, const char* const wdroot, DocPosCR* pwdlen)
{
  char const sep = ' ';
  bool found = false;
  const char* start = strg;
  do {
    start = StrStrIA(start, wdroot);
    if (start) {
      if ((start == strg) || (start[-1] == sep)) { // word begin
        found = true;
        break;
      }
      ++start;
    }
  } while (start && *start);

  if (found) {
    DocPosCR len = *pwdlen;
    while (start[len] && (start[len] != sep)) { ++len; }
    *pwdlen = len;
  }
  else {
    *pwdlen = 0;
  }
  return (found ? start : NULL);
}

// ----------------------------------------------

bool EditAutoCompleteWord(HWND hwnd, bool autoInsert)
{
  if (SciCall_IsIMEModeCJK()) {
    SciCall_AutoCCancel();
    return false;
  }

  DocPos const iMinWdChCnt = 2;  // min number of typed chars before AutoC

  char const* const pchAllowdWordChars = 
    ((Globals.bIsCJKInputCodePage || Globals.bUseLimitedAutoCCharSet) ? AutoCompleteWordCharSet :
    (Settings.AccelWordNavigation ? WordCharsAccelerated : WordCharsDefault));
  
  SciCall_SetWordChars(pchAllowdWordChars);

  DocPos const iDocEndPos = Sci_GetDocEndPosition();
  DocPos const iCurrentPos = SciCall_GetCurrentPos();
  DocPos const iCol = SciCall_GetColumn(iCurrentPos);
  DocPos const iPosBefore = SciCall_PositionBefore(iCurrentPos);
  DocPos const iWordStartPos = SciCall_WordStartPosition(iPosBefore, true);

  if ((iWordStartPos == iPosBefore) || (iCol < iMinWdChCnt) || ((iCurrentPos - iWordStartPos) < iMinWdChCnt)) {
    EditSetAccelWordNav(hwnd, Settings.AccelWordNavigation);
    return true;
  }

  DocPos iPos = iWordStartPos;
  bool bWordAllNumbers = true;
  while ((iPos < iCurrentPos) && bWordAllNumbers && (iPos <= iDocEndPos)) {
    char const ch = SciCall_GetCharAt(iPos);
    if (ch < '0' || ch > '9') {
      bWordAllNumbers = false;
    }
    iPos = SciCall_PositionAfter(iPos);
  }
  if (bWordAllNumbers) {
    EditSetAccelWordNav(hwnd, Settings.AccelWordNavigation);
    return true;
  }


  char pRoot[_MAX_AUTOC_WORD_LEN];
  DocPos const iRootLen = (iCurrentPos - iWordStartPos);
  StringCchCopyNA(pRoot, COUNTOF(pRoot), SciCall_GetRangePointer(iWordStartPos, iRootLen), (size_t)iRootLen);

  int iNumWords = 0;
  size_t iWListSize = 0;

  PWLIST pListHead = NULL;

  if (Settings.AutoCompleteWords)
  {
    struct Sci_TextToFind ft = { { 0, 0 }, 0, { 0, 0 } };
    ft.lpstrText = pRoot;
    ft.chrg.cpMax = (DocPosCR)iDocEndPos;

    DocPos iPosFind = SciCall_FindText(SCFIND_WORDSTART, &ft);
    PWLIST pwlNewWord = NULL;

    while ((iPosFind >= 0) && ((iPosFind + iRootLen) < iDocEndPos))
    {
      DocPos const iWordEndPos = SciCall_WordEndPosition(iPosFind + iRootLen, true);

      if (iPosFind != (iCurrentPos - iRootLen))
      {
        DocPos const wordLength = (iWordEndPos - iPosFind);
        if (wordLength > iRootLen)
        {
          if (!pwlNewWord) { pwlNewWord = (PWLIST)AllocMem(sizeof(WLIST), HEAP_ZERO_MEMORY); }
          if (pwlNewWord)
          {
            StringCchCopyNA(pwlNewWord->word, _MAX_AUTOC_WORD_LEN, SciCall_GetRangePointer(iPosFind, wordLength), wordLength);

            PWLIST pPrev = NULL;
            PWLIST pWLItem = NULL;
            LL_SEARCH_ORDERED(pListHead, pPrev, pWLItem, pwlNewWord, wordcmp);
            if (!pWLItem) { // not found
              //LL_INSERT_INORDER(pListHead, pwlNewWord, wordcmpi);
              LL_APPEND_ELEM(pListHead, pPrev, pwlNewWord);
              ++iNumWords;
              iWListSize += (wordLength + 1);
              pwlNewWord = NULL; // alloc new
            }
          }
        }
      }
      
      ft.chrg.cpMin = (DocPosCR)iWordEndPos;
      iPosFind = SciCall_FindText(SCFIND_WORDSTART, &ft);
    }
    if (pwlNewWord) { FreeMem(pwlNewWord); pwlNewWord = NULL; }
  }
  // --------------------------------------------------------------------------
  if (Settings.AutoCLexerKeyWords)
  // --------------------------------------------------------------------------
  {
    PKEYWORDLIST const pKeyWordList = Style_GetCurrentLexerPtr()->pKeyWords;

    PWLIST pwlNewWord = NULL;
    for (int i = 0; i <= KEYWORDSET_MAX; ++i) {
      const char* word = pKeyWordList->pszKeyWords[i];
      do {
        DocPosCR wlen = (DocPosCR)iRootLen;
        word = _strNextLexKeyWord(word, pRoot, &wlen);
        if (word) {
          if (wlen > iRootLen) {
            if (!pwlNewWord) { pwlNewWord = (PWLIST)AllocMem(sizeof(WLIST), HEAP_ZERO_MEMORY); }
            if (pwlNewWord)
            {
              StringCchCopyNA(pwlNewWord->word, _MAX_AUTOC_WORD_LEN, word, wlen);

              PWLIST pPrev = NULL;
              PWLIST pWLItem = NULL;
              LL_SEARCH_ORDERED(pListHead, pPrev, pWLItem, pwlNewWord, wordcmp);
              if (!pWLItem) { // not found
                //LL_INSERT_INORDER(pListHead, pwlNewWord, wordcmpi);
                LL_APPEND_ELEM(pListHead, pPrev, pwlNewWord);
                ++iNumWords;
                iWListSize += (wlen + 1);
                pwlNewWord = NULL; // alloc new
              }
            }
          }
          word += (wlen ? wlen : 1);
        }
      } while (word && word[0]);
    }
    if (pwlNewWord) { FreeMem(pwlNewWord); pwlNewWord = NULL; }
  }

  // --------------------------------------------------------------------------

  if (iNumWords > 0) 
  {
    const char* const sep = " ";
    SciCall_AutoCCancel();
    // cppcheck-suppress constArgument
    SciCall_AutoCSetSeperator(sep[0]);
    SciCall_AutoCSetIgnoreCase(true);
    //SendMessage(hwnd, SCI_AUTOCSETCASEINSENSITIVEBEHAVIOUR, (WPARAM)SC_CASEINSENSITIVEBEHAVIOUR_IGNORECASE, 0);
    SciCall_AutoCSetChooseSingle(autoInsert);
    //SciCall_AutoCSetOrder(SC_ORDER_PERFORMSORT); // already sorted
    SciCall_AutoCSetFillups("\t\n\r");
    //SciCall_AutoCSetFillups(Settings.AccelWordNavigation ? WhiteSpaceCharsDefault : WhiteSpaceCharsAccelerated);

    ++iWListSize; // zero termination
    char* const pList = AllocMem(iWListSize, HEAP_ZERO_MEMORY);
    if (pList) {
      PWLIST pTmp = NULL;
      PWLIST pWLItem = NULL;
      LL_FOREACH_SAFE(pListHead, pWLItem, pTmp) {
        if (pWLItem->word[0]) {
          StringCchCatA(pList, iWListSize, sep);
          StringCchCatA(pList, iWListSize, pWLItem->word);
        }
        LL_DELETE(pListHead, pWLItem);
        FreeMem(pWLItem);
      }
      
      SciCall_AutoCShow(iRootLen, (pList + 1));
      FreeMem(pList);
    }
  }

  EditSetAccelWordNav(hwnd, Settings.AccelWordNavigation);
  return true;
}


//=============================================================================
//
//  EditFinalizeStyling()
//
void EditFinalizeStyling(HWND hwnd, DocPos iEndPos)
{
  UNUSED(hwnd);
  DocPos const startPos = SciCall_PositionFromLine(SciCall_LineFromPosition(SciCall_GetEndStyled()));
  DocPos const endPos = (iEndPos < 0) ? (DocPos)-1 : SciCall_GetLineEndPosition(SciCall_LineFromPosition(iEndPos));
  if (startPos < endPos) {
    Sci_ApplyLexerStyle(startPos, endPos);
  }
}


//=============================================================================
//
//  EditUpdateIndicators()
//  Find and mark all COLOR refs (#RRGGBB)
//
static void _ClearIndicatorInRange(const int indicator, const int indicator2nd,
                                   const DocPos startPos, const DocPos endPos)
{
  SciCall_SetIndicatorCurrent(indicator);
  SciCall_IndicatorClearRange(startPos, endPos - startPos);
  if (indicator2nd >= 0) {
    SciCall_SetIndicatorCurrent(indicator2nd);
    SciCall_IndicatorClearRange(startPos, endPos - startPos);
  }
}

static void _UpdateIndicators(HWND hwnd, const int indicator, const int indicator2nd, 
                              const char* regExpr, DocPos startPos, DocPos endPos)
{
  if (endPos < 0) {
    endPos = Sci_GetDocEndPosition();
  }
  else if (endPos < startPos) {
    swapos(&startPos, &endPos);
  }
  if (startPos < 0) { // current line only
    DocLn const lineNo = SciCall_LineFromPosition(SciCall_GetCurrentPos());
    startPos = SciCall_PositionFromLine(lineNo);
    endPos = SciCall_GetLineEndPosition(lineNo);
  }
  else if (endPos == startPos) { return; }

  // --------------------------------------------------------------------------

  int const iRegExLen = (int)StringCchLenA(regExpr, 0);

  DocPos start = startPos;
  DocPos end = endPos;
  do {

    DocPos const _start = start;
    DocPos const _end   = end;
    DocPos const iPos = _FindInTarget(hwnd, regExpr, iRegExLen, SCFIND_NP3_REGEX, &start, &end, false, FRMOD_IGNORE);

    if (iPos < 0) {
      // not found
      _ClearIndicatorInRange(indicator, indicator2nd, _start, _end);
      break;
    }
    DocPos const mlen = end - start;
    if ((mlen <= 0) || (end > endPos)) {
      // wrong match
      _ClearIndicatorInRange(indicator, indicator2nd, _start, _end);
      break; // wrong match
    }

    _ClearIndicatorInRange(indicator, indicator2nd, _start, end);

    SciCall_SetIndicatorCurrent(indicator);
    SciCall_IndicatorFillRange(start, mlen);
    if (indicator2nd >= 0) {
      SciCall_SetIndicatorCurrent(indicator2nd);
      SciCall_IndicatorFillRange(start, mlen);
    }

    // next occurrence
    start = end + 1;
    end = endPos;

  } while (start < end);

}

//=============================================================================
//
//  EditUpdateIndicators()
//  - Find and mark all URL hot-spots
//  - Find and mark all COLOR refs (#RRGGBB)
//
void EditUpdateIndicators(HWND hwnd, DocPos startPos, DocPos endPos, bool bClearOnly)
{
  if (bClearOnly) {
    _ClearIndicatorInRange(INDIC_NP3_HYPERLINK, INDIC_NP3_HYPERLINK_U, startPos, endPos);
    _ClearIndicatorInRange(INDIC_NP3_COLOR_DEF, INDIC_NP3_COLOR_DWELL, startPos, endPos);
    return;
  }

  if (Settings.HyperlinkHotspot) 
  {
    static const char* pUrlRegEx = "\\b(?:(?:https?|ftp|file)://|www\\.|ftp\\.)"
      "(?:\\([-A-Z0-9+&@#/%=~_|$?!:,.]*\\)|[-A-Z0-9+&@#/%=~_|$?!:,.])*"
      "(?:\\([-A-Z0-9+&@#/%=~_|$?!:,.]*\\)|[A-Z0-9+&@#/%=~_|$])";
    _UpdateIndicators(hwnd, INDIC_NP3_HYPERLINK, INDIC_NP3_HYPERLINK_U, pUrlRegEx, startPos, endPos);
  }
  
  if (Settings.ColorDefHotspot) 
  {
    static const char* pColorRegEx = "#([0-9a-fA-F]){6}";
    _UpdateIndicators(hwnd, INDIC_NP3_COLOR_DEF, -1, pColorRegEx, startPos, endPos);
  }
}


//=============================================================================
//
//  EditHideNotMarkedLineRange()
//
void EditHideNotMarkedLineRange(HWND hwnd, bool bHideLines)
{
  if (!bHideLines) {
    // reset
    SciCall_FoldAll(EXPAND);
    Style_SetFoldingAvailability(Style_GetCurrentLexerPtr());
    FocusedView.ShowCodeFolding = Settings.ShowCodeFolding;
    Style_SetFoldingProperties(FocusedView.CodeFoldingAvailable);
    Style_SetFolding(hwnd, FocusedView.CodeFoldingAvailable && FocusedView.ShowCodeFolding);
    Sci_ApplyLexerStyle(0, -1);
    EditMarkAllOccurrences(hwnd, true);
  }
  else // =====   fold lines without marker   =====
  {
    // prepare hidden (folding) settings
    FocusedView.CodeFoldingAvailable = true;
    FocusedView.ShowCodeFolding = true;
    Style_SetFoldingFocusedView();
    Style_SetFolding(hwnd, true);

    DocLn const iStartLine = 0;
    DocLn const iEndLine = SciCall_GetLineCount() - 1;

    int const baseLevel = SC_FOLDLEVELBASE;

    // 1st line
    int level = baseLevel;
    SciCall_SetFoldLevel(iStartLine, SC_FOLDLEVELHEADERFLAG | level++); // visible in any case

    for (DocLn iLine = iStartLine + 1; iLine <= iEndLine; ++iLine)
    {
      DocPos const begOfLine = SciCall_PositionFromLine(iLine);

      if (begOfLine == SciCall_IndicatorStart(INDIC_NP3_FOCUS_VIEW, begOfLine)) // visible
      {
        level = baseLevel;
        SciCall_SetFoldLevel(iLine, SC_FOLDLEVELHEADERFLAG | level++);
      }
      else // hide line
      {
        SciCall_SetFoldLevel(iLine, SC_FOLDLEVELWHITEFLAG | level);
      }
    }
    SciCall_FoldAll(FOLD);
  }
}


//=============================================================================
//
//  _HighlightIfBrace()
//
static bool _HighlightIfBrace(const HWND hwnd, const DocPos iPos)
{
  UNUSED(hwnd);
  if (iPos < 0) {
    // clear indicator
    SciCall_BraceBadLight(INVALID_POSITION);
    SciCall_SetHighLightGuide(0);
    return true;
  }

  char const c = SciCall_GetCharAt(iPos);
  
  if (StrChrA(NP3_BRACES_TO_MATCH, c)) {
    DocPos const iBrace2 = SciCall_BraceMatch(iPos);
    if (iBrace2 != (DocPos)-1) {
      int const col1 = (int)SciCall_GetColumn(iPos);
      int const col2 = (int)SciCall_GetColumn(iBrace2);
      SciCall_BraceHighLight(iPos, iBrace2);
      SciCall_SetHighLightGuide(min_i(col1, col2));
    }
    else {
      SciCall_BraceBadLight(iPos);
      SciCall_SetHighLightGuide(0);
    }
    return true;
  }
  return false;
}


//=============================================================================
//
//  EditMatchBrace()
//
void EditMatchBrace(HWND hwnd) 
{
  DocPos iPos = SciCall_GetCurrentPos();
  if (!_HighlightIfBrace(hwnd, iPos)) {
    // try one before
    iPos = SciCall_PositionBefore(iPos);
    if (!_HighlightIfBrace(hwnd, iPos)) {
      // clear mark
      _HighlightIfBrace(hwnd, -1);
    }
  }
}



//=============================================================================
//
//  EditLinenumDlgProc()
//
static INT_PTR CALLBACK EditLinenumDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  UNUSED(lParam);

  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        WCHAR wchLineCaption[96];
        WCHAR wchColumnCaption[96];

        if (Globals.hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIcon); }

        DocLn const iCurLine = SciCall_LineFromPosition(SciCall_GetCurrentPos())+1;
        DocLn const iMaxLnNum = SciCall_GetLineCount();
        DocPos const iCurColumn = SciCall_GetColumn(SciCall_GetCurrentPos()) + 1;
        DocPos const iLineEndPos = Sci_GetNetLineLength(iCurLine);

        FormatLngStringW(wchLineCaption, COUNTOF(wchLineCaption), IDS_MUI_GOTO_LINE, 
          (int)clampp(iMaxLnNum, 0, INT_MAX));
        FormatLngStringW(wchColumnCaption, COUNTOF(wchColumnCaption), IDS_MUI_GOTO_COLUMN, 
          (int)clampp(max_p(iLineEndPos, (DocPos)Globals.fvCurFile.iLongLinesLimit), 0, INT_MAX));
        SetDlgItemText(hwnd, IDC_LINE_TEXT, wchLineCaption);
        SetDlgItemText(hwnd, IDC_COLUMN_TEXT, wchColumnCaption);

        SetDlgItemInt(hwnd, IDC_LINENUM, (int)clampp(iCurLine, 0, INT_MAX), false);
        SetDlgItemInt(hwnd, IDC_COLNUM, (int)clampp(iCurColumn, 0, INT_MAX), false);
        SendDlgItemMessage(hwnd,IDC_LINENUM,EM_LIMITTEXT,80,0);
        SendDlgItemMessage(hwnd,IDC_COLNUM,EM_LIMITTEXT,80,0);
        CenterDlgInParent(hwnd);
      }
      return true;


    case WM_DPICHANGED:
      UpdateWindowLayoutForDPI(hwnd, 0, 0, 0, 0);
      return true;


    case WM_COMMAND:
      {
        switch (LOWORD(wParam))
        {
        case IDOK:
        {
          DocLn const iMaxLnNum = SciCall_GetLineCount();

          //~BOOL fTranslated = TRUE;
          //~DocLn iNewLine = (DocLn)GetDlgItemInt(hwnd, IDC_LINENUM, &fTranslated, FALSE);

          int iExprError = 0;
          bool bLnTranslated = true;
          DocLn iNewLine = 0;
          if (SendDlgItemMessage(hwnd, IDC_LINENUM, WM_GETTEXTLENGTH, 0, 0) > 0) 
          {
            char chLineNumber[96];
            GetDlgItemTextA(hwnd, IDC_LINENUM, chLineNumber, COUNTOF(chLineNumber));
            iNewLine = (DocLn)te_interp(chLineNumber, &iExprError);
            if (iExprError > 1) {
              chLineNumber[iExprError-1] = '\0';
              iNewLine = (DocLn)te_interp(chLineNumber, &iExprError);
            }
            bLnTranslated = (iExprError == 0);
          }

          bool bColTranslated = true;
          DocPos iNewCol = 1;
          if (SendDlgItemMessage(hwnd, IDC_COLNUM, WM_GETTEXTLENGTH, 0, 0) > 0) 
          {
            char chColumnNumber[96];
            GetDlgItemTextA(hwnd, IDC_COLNUM, chColumnNumber, COUNTOF(chColumnNumber));
            iNewCol = (DocPos)te_interp(chColumnNumber, &iExprError);
            if (iExprError > 1) {
              chColumnNumber[iExprError-1] = '\0';
              iNewLine = (DocLn)te_interp(chColumnNumber, &iExprError);
            }
            bColTranslated = (iExprError == 0);
          }

          if (!bLnTranslated || !bColTranslated)
          {
            PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, (!bLnTranslated) ? IDC_LINENUM : IDC_COLNUM)), 1);
            return true;
          }

          if ((iNewLine > 0) && (iNewLine <= iMaxLnNum) && (iNewCol > 0))
          {
            EditJumpTo(Globals.hwndEdit, iNewLine, iNewCol);
            EndDialog(hwnd, IDOK);
          }
          else {
            PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, (!((iNewLine > 0) && (iNewLine <= iMaxLnNum))) ? IDC_LINENUM : IDC_COLNUM)), 1);
          }
        }
        break;

        case IDCANCEL:
          EndDialog(hwnd, IDCANCEL);
          break;

        }
      }
      return true;
  }
  return false;
}


//=============================================================================
//
//  EditLinenumDlg()
//
bool EditLinenumDlg(HWND hwnd)
{
  if (IDOK == ThemedDialogBoxParam(Globals.hLngResContainer, MAKEINTRESOURCE(IDD_MUI_LINENUM),
                                   GetParent(hwnd), EditLinenumDlgProc, (LPARAM)hwnd)) {
    return true;
  }
  return false;
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


static INT_PTR CALLBACK EditModifyLinesDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  static PMODLINESDATA pdata;

  static int id_hover;
  static int id_capture;

  static HCURSOR hCursorNormal;
  static HCURSOR hCursorHover;

  switch(umsg)
  {
    static HFONT hFontHover;

    case WM_INITDIALOG:
      {
        id_hover = 0;
        id_capture = 0;

        if (Globals.hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIcon); }

        static HFONT hFontNormal;
        if (NULL == (hFontNormal = (HFONT)SendDlgItemMessage(hwnd, 200, WM_GETFONT, 0, 0))) {
          hFontNormal = GetStockObject(DEFAULT_GUI_FONT);
        }
        LOGFONT lf;
        GetObject(hFontNormal,sizeof(LOGFONT),&lf);
        lf.lfUnderline = true;
        hFontHover = CreateFontIndirect(&lf);

        hCursorNormal = LoadCursor(NULL,IDC_ARROW);
        hCursorHover = LoadCursor(NULL,IDC_HAND);
        if (!hCursorHover)
          hCursorHover = LoadCursor(Globals.hInstance, IDC_ARROW);

        pdata = (PMODLINESDATA)lParam;
        SetDlgItemTextW(hwnd,100,pdata->pwsz1);
        SendDlgItemMessage(hwnd,100,EM_LIMITTEXT,255,0);
        SetDlgItemTextW(hwnd,101,pdata->pwsz2);
        SendDlgItemMessage(hwnd,101,EM_LIMITTEXT,255,0);
        CenterDlgInParent(hwnd);
      }
      return true;

    case WM_DPICHANGED:
      UpdateWindowLayoutForDPI(hwnd, 0, 0, 0, 0);
      return true;

    case WM_DESTROY:
      DeleteObject(hFontHover);
      return false;

    case WM_NCACTIVATE:
      if (!(bool)wParam) {
        if (id_hover != 0) {
          //int _id_hover = id_hover;
          id_hover = 0;
          id_capture = 0;
          //InvalidateRect(GetDlgItem(hwnd,id_hover),NULL,false);
        }
      }
      return false;

    case WM_CTLCOLORSTATIC:
      {
        DWORD dwId = GetWindowLong((HWND)lParam,GWL_ID);
        HDC hdc = (HDC)wParam;

        if (dwId >= 200 && dwId <= 205) {
          SetBkMode(hdc,TRANSPARENT);
          if (GetSysColorBrush(COLOR_HOTLIGHT)) {
            SetTextColor(hdc, GetSysColor(COLOR_HOTLIGHT));
          }
          else {
            SetTextColor(hdc, RGB(0, 0, 0xFF));
          }
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
              }
            }
            else if (id_hover != 0) {
              id_hover = 0;
            }
          }
          else if (id_hover != 0) {
            id_hover = 0;
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
          //InvalidateRect(GetDlgItem(hwnd,dwId),NULL,false);
        }
        SetCursor(id_hover != 0?hCursorHover:hCursorNormal);
      }
      break;

    case WM_LBUTTONUP:
      {
        //POINT pt;
        //pt.x = LOWORD(lParam);  pt.y = HIWORD(lParam);
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
              SendDlgItemMessage(hwnd,id_focus,EM_REPLACESEL,(WPARAM)true,(LPARAM)wch);
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
      return true;
  }
  return false;
}


//=============================================================================
//
//  EditModifyLinesDlg()
//
bool EditModifyLinesDlg(HWND hwnd,LPWSTR pwsz1,LPWSTR pwsz2)
{

  INT_PTR iResult;
  MODLINESDATA data;
  data.pwsz1 = pwsz1;  data.pwsz2 = pwsz2;

  iResult = ThemedDialogBoxParam(
              Globals.hLngResContainer,
              MAKEINTRESOURCEW(IDD_MUI_MODIFYLINES),
              hwnd,
              EditModifyLinesDlgProc,
              (LPARAM)&data);

  return (iResult == IDOK) ? true : false;

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
static INT_PTR CALLBACK EditAlignDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  static int *piAlignMode;
  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        piAlignMode = (int*)lParam;
        if (Globals.hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIcon); }
        CheckRadioButton(hwnd,100,104,*piAlignMode+100);
        CenterDlgInParent(hwnd);
      }
      return true;

    case WM_DPICHANGED:
      UpdateWindowLayoutForDPI(hwnd, 0, 0, 0, 0);
      return true;

    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
        case IDOK: {
            *piAlignMode = 0;
            if (IsButtonChecked(hwnd,100))
              *piAlignMode = ALIGN_LEFT;
            else if (IsButtonChecked(hwnd,101))
              *piAlignMode = ALIGN_RIGHT;
            else if (IsButtonChecked(hwnd,102))
              *piAlignMode = ALIGN_CENTER;
            else if (IsButtonChecked(hwnd,103))
              *piAlignMode = ALIGN_JUSTIFY;
            else if (IsButtonChecked(hwnd,104))
              *piAlignMode = ALIGN_JUSTIFY_EX;
            EndDialog(hwnd,IDOK);
          }
          break;

        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;
      }
      return true;
  }
  return false;
}


//=============================================================================
//
//  EditAlignDlg()
//
bool EditAlignDlg(HWND hwnd,int *piAlignMode)
{

  INT_PTR iResult;

  iResult = ThemedDialogBoxParam(
              Globals.hLngResContainer,
              MAKEINTRESOURCEW(IDD_MUI_ALIGN),
              hwnd,
              EditAlignDlgProc,
              (LPARAM)piAlignMode);

  return (iResult == IDOK) ? true : false;

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


static INT_PTR CALLBACK EditEncloseSelectionDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  static PENCLOSESELDATA pdata;
  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        pdata = (PENCLOSESELDATA)lParam;
        if (Globals.hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIcon); }
        SendDlgItemMessage(hwnd,100,EM_LIMITTEXT,255,0);
        SetDlgItemTextW(hwnd,100,pdata->pwsz1);
        SendDlgItemMessage(hwnd,101,EM_LIMITTEXT,255,0);
        SetDlgItemTextW(hwnd,101,pdata->pwsz2);
        CenterDlgInParent(hwnd);
      }
      return true;

    case WM_DPICHANGED:
      UpdateWindowLayoutForDPI(hwnd, 0, 0, 0, 0);
      return true;

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
      return true;
  }
  return false;
}


//=============================================================================
//
//  EditEncloseSelectionDlg()
//
bool EditEncloseSelectionDlg(HWND hwnd,LPWSTR pwszOpen,LPWSTR pwszClose)
{

  INT_PTR iResult;
  ENCLOSESELDATA data;
  data.pwsz1 = pwszOpen;  data.pwsz2 = pwszClose;

  iResult = ThemedDialogBoxParam(
              Globals.hLngResContainer,
              MAKEINTRESOURCEW(IDD_MUI_ENCLOSESELECTION),
              hwnd,
              EditEncloseSelectionDlgProc,
              (LPARAM)&data);

  return (iResult == IDOK) ? true : false;

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


static INT_PTR CALLBACK EditInsertTagDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  static PTAGSDATA pdata;
  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        pdata = (PTAGSDATA)lParam;
        if (Globals.hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIcon); }
        SendDlgItemMessage(hwnd,100,EM_LIMITTEXT,254,0);
        SetDlgItemTextW(hwnd,100,L"<tag>");
        SendDlgItemMessage(hwnd,101,EM_LIMITTEXT,255,0);
        SetDlgItemTextW(hwnd,101,L"</tag>");
        SetFocus(GetDlgItem(hwnd,100));
        PostMessage(GetDlgItem(hwnd,100),EM_SETSEL,1,4);
        CenterDlgInParent(hwnd);
      }
      return false;

    case WM_DPICHANGED:
      UpdateWindowLayoutForDPI(hwnd, 0, 0, 0, 0);
      return true;

    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
        case 100: {
            if (HIWORD(wParam) == EN_CHANGE) 
            {
              bool bClear = true;
              WCHAR wchBuf[256] = { L'\0' };
              GetDlgItemTextW(hwnd,100,wchBuf,256);
              if (StringCchLenW(wchBuf,COUNTOF(wchBuf)) >= 3) {

                if (wchBuf[0] == L'<') 
                {
                  WCHAR wchIns[256] = L"</";
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
                        StringCchCompareXI(wchIns,L"</base>") &&
                        StringCchCompareXI(wchIns,L"</bgsound>") &&
                        StringCchCompareXI(wchIns,L"</br>") &&
                        StringCchCompareXI(wchIns,L"</embed>") &&
                        StringCchCompareXI(wchIns,L"</hr>") &&
                        StringCchCompareXI(wchIns,L"</img>") &&
                        StringCchCompareXI(wchIns,L"</input>") &&
                        StringCchCompareXI(wchIns,L"</link>") &&
                        StringCchCompareXI(wchIns,L"</meta>")) {

                        SetDlgItemTextW(hwnd,101,wchIns);
                        bClear = false;
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
      return true;
  }
  return false;
}


//=============================================================================
//
//  EditInsertTagDlg()
//
bool EditInsertTagDlg(HWND hwnd,LPWSTR pwszOpen,LPWSTR pwszClose)
{

  INT_PTR iResult;
  TAGSDATA data;
  data.pwsz1 = pwszOpen;  data.pwsz2 = pwszClose;
  
  iResult = ThemedDialogBoxParam(
              Globals.hLngResContainer,
              MAKEINTRESOURCEW(IDD_MUI_INSERTTAG),
              hwnd,
              EditInsertTagDlgProc,
              (LPARAM)&data);

  return (iResult == IDOK) ? true : false;

}


//=============================================================================
//
//  EditSortDlgProc()
//
//  Controls: 100-102 Radio Button
//            103-109 Check Box
//
static INT_PTR CALLBACK EditSortDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  static int* piSortFlags;
  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        piSortFlags = (int*)lParam;

        if (*piSortFlags == 0) {
          *piSortFlags = SORT_ASCENDING | SORT_REMZEROLEN;
        }

        if (Globals.hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIcon); }

        if (*piSortFlags & SORT_DESCENDING) {
          CheckRadioButton(hwnd, 100, 102, 101);
        }
        else if (*piSortFlags & SORT_SHUFFLE) {
          CheckRadioButton(hwnd,100,102,102);
          DialogEnableWindow(hwnd,103,false);
          DialogEnableWindow(hwnd,104,false);
          DialogEnableWindow(hwnd,105,false);
          DialogEnableWindow(hwnd,106,false);
          DialogEnableWindow(hwnd,107,false);
          DialogEnableWindow(hwnd,108,false);
          DialogEnableWindow(hwnd,109,false);
        }
        else {
          CheckRadioButton(hwnd, 100, 102, 100);
        }
        if (*piSortFlags & SORT_MERGEDUP) {
          CheckDlgButton(hwnd, 103, BST_CHECKED);
        }
        if (*piSortFlags & SORT_UNIQDUP) {
          CheckDlgButton(hwnd, 104, BST_CHECKED);
          DialogEnableWindow(hwnd, 103, false);
        }
        if (*piSortFlags & SORT_UNIQUNIQ) {
          CheckDlgButton(hwnd, 105, BST_CHECKED);
        }
        if (*piSortFlags & SORT_REMZEROLEN) {
          CheckDlgButton(hwnd, 106, BST_CHECKED);
        }
        if (*piSortFlags & SORT_REMWSPACELN) {
          CheckDlgButton(hwnd, 107, BST_CHECKED);
          CheckDlgButton(hwnd, 106, BST_CHECKED);
          DialogEnableWindow(hwnd, 106, false);
        }
        if (*piSortFlags & SORT_NOCASE) {
          CheckDlgButton(hwnd, 108, BST_CHECKED);
        }
        if (*piSortFlags & SORT_LOGICAL) {
          CheckDlgButton(hwnd, 109, BST_CHECKED);
        }
        if (!Sci_IsMultiOrRectangleSelection()) {
          *piSortFlags &= ~SORT_COLUMN;
          DialogEnableWindow(hwnd,110,false);
        }
        else {
          *piSortFlags |= SORT_COLUMN;
          CheckDlgButton(hwnd,110,BST_CHECKED);
        }
        CenterDlgInParent(hwnd);
      }
      return true;

    case WM_DPICHANGED:
      UpdateWindowLayoutForDPI(hwnd, 0, 0, 0, 0);
      return true;

    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
        case IDOK: {
            *piSortFlags = 0;
            if (IsButtonChecked(hwnd, 100))
              *piSortFlags |= SORT_ASCENDING;
            if (IsButtonChecked(hwnd,101))
              *piSortFlags |= SORT_DESCENDING;
            if (IsButtonChecked(hwnd,102))
              *piSortFlags |= SORT_SHUFFLE;
            if (IsButtonChecked(hwnd,103))
              *piSortFlags |= SORT_MERGEDUP;
            if (IsButtonChecked(hwnd,104))
              *piSortFlags |= SORT_UNIQDUP;
            if (IsButtonChecked(hwnd,105))
              *piSortFlags |= SORT_UNIQUNIQ;
            if (IsButtonChecked(hwnd,106))
              *piSortFlags |= SORT_REMZEROLEN;
            if (IsButtonChecked(hwnd,107))
              *piSortFlags |= SORT_REMWSPACELN;
            if (IsButtonChecked(hwnd,108))
              *piSortFlags |= SORT_NOCASE;
            if (IsButtonChecked(hwnd,109))
              *piSortFlags |= SORT_LOGICAL;
            if (IsButtonChecked(hwnd,110))
              *piSortFlags |= SORT_COLUMN;
            EndDialog(hwnd,IDOK);
          }
          break;

        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;

        case 100:
        case 101:
          DialogEnableWindow(hwnd,103, IsButtonUnchecked(hwnd,105));
          DialogEnableWindow(hwnd,104,true);
          DialogEnableWindow(hwnd,105,true);
          DialogEnableWindow(hwnd,106,true);
          DialogEnableWindow(hwnd,107,true);
          DialogEnableWindow(hwnd,108,true);
          DialogEnableWindow(hwnd,109,true);
          break;
        case 102:
          DialogEnableWindow(hwnd,103,false);
          DialogEnableWindow(hwnd,104,false);
          DialogEnableWindow(hwnd,105,false);
          DialogEnableWindow(hwnd,106,false);
          DialogEnableWindow(hwnd,107,false);
          DialogEnableWindow(hwnd,108,false);
          DialogEnableWindow(hwnd,109,false);
          break;
        case 104:
          DialogEnableWindow(hwnd,103,IsButtonUnchecked(hwnd,104));
          break;
        case 107:
          if (IsButtonChecked(hwnd, 107)) {
            CheckDlgButton(hwnd, 106, BST_CHECKED);
            DialogEnableWindow(hwnd, 106, false);
          }
          else {
            DialogEnableWindow(hwnd, 106, true);
          }
          break;
        default:
          break;
      }
      return true;
  }
  return false;
}


//=============================================================================
//
//  EditSortDlg()
//
bool EditSortDlg(HWND hwnd,int* piSortFlags)
{

  INT_PTR iResult;

  iResult = ThemedDialogBoxParam(
              Globals.hLngResContainer,
              MAKEINTRESOURCEW(IDD_MUI_SORT),
              hwnd,
              EditSortDlgProc,
              (LPARAM)piSortFlags);

  return (iResult == IDOK) ? true : false;

}


//=============================================================================
//
//  EditSetAccelWordNav()
//
void EditSetAccelWordNav(HWND hwnd,bool bAccelWordNav)
{
  UNUSED(hwnd);
  Settings.AccelWordNavigation = bAccelWordNav;
  if (Settings.AccelWordNavigation) {
    SciCall_SetWordChars(WordCharsAccelerated);
    SciCall_SetWhitespaceChars(WhiteSpaceCharsAccelerated);
    SciCall_SetPunctuationChars(PunctuationCharsAccelerated);
  }
  else
    SciCall_SetCharsDefault();
}


//=============================================================================
//
//  EditShowZeroLengthCallTip()
//
static char s_chZeroLenCT[80] = { '\0' };

void EditShowZeroLengthCallTip(HWND hwnd, DocPos iPosition)
{
  UNUSED(hwnd);
  if (s_chZeroLenCT[0] == '\0') {
    GetLngStringW2MB(IDS_MUI_ZERO_LEN_MATCH, s_chZeroLenCT, COUNTOF(s_chZeroLenCT));
  }
  SciCall_CallTipShow(iPosition, s_chZeroLenCT);
  Globals.CallTipType = CT_ZEROLEN_MATCH;
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
      StringCchPrintfW(tchLine, COUNTOF(tchLine), L"%td;", (long long)iLine);
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
  UNUSED(hwnd);
  WCHAR lnNum[32];
  const WCHAR* p1 = pszBookMarks;
  if (!p1) return;

  const DocLn iLineMax = SciCall_GetLineCount() - 1;

  while (*p1) {
    const WCHAR* p2 = StrChr(p1, L';');
    if (!p2)
      p2 = StrEnd(p1,0);
    StringCchCopyNW(lnNum, COUNTOF(lnNum), p1, min_s((size_t)(p2 - p1), 16));
    DocLn iLine = 0;
    if (swscanf_s(lnNum, DOCPOSFMTW, &iLine) == 1) {
      if (iLine <= iLineMax) {
        SciCall_MarkerAdd(iLine, MARKER_NP3_BOOKMARK);
      }
    }
    p1 = (*p2) ? (p2 + 1) : p2;
  }
}


//=============================================================================
//
//  _SetFileVars()
//
static void _SetFileVars(char* buffer, size_t cch, LPFILEVARS lpfv)
{
  bool bDisableFileVar = false;

  if (!Flags.NoFileVariables) 
  {
    int i;
    if (FileVars_ParseInt(buffer, "enable-local-variables", &i) && (!i)) {
      bDisableFileVar = true;
    }
    if (!bDisableFileVar) {

      if (FileVars_ParseInt(buffer, "tab-width", &i)) {
        lpfv->iTabWidth = clampi(i, 1, 256);
        lpfv->mask |= FV_TABWIDTH;
      }

      if (FileVars_ParseInt(buffer, "c-basic-indent", &i)) {
        lpfv->iIndentWidth = clampi(i, 0, 256);
        lpfv->mask |= FV_INDENTWIDTH;
      }

      if (FileVars_ParseInt(buffer, "indent-tabs-mode", &i)) {
        lpfv->bTabsAsSpaces = (i) ? false : true;
        lpfv->mask |= FV_TABSASSPACES;
      }

      if (FileVars_ParseInt(buffer, "c-tab-always-indent", &i)) {
        lpfv->bTabIndents = (i) ? true : false;
        lpfv->mask |= FV_TABINDENTS;
      }

      if (FileVars_ParseInt(buffer, "truncate-lines", &i)) {
        lpfv->bWordWrap = (i) ? false : true;
        lpfv->mask |= FV_WORDWRAP;
      }

      if (FileVars_ParseInt(buffer, "fill-column", &i)) {
        lpfv->iLongLinesLimit = clampi(i, 0, LONG_LINES_MARKER_LIMIT);
        lpfv->mask |= FV_LONGLINESLIMIT;
      }
    }
  }

  // Unicode Sig
  bool const bHasSignature = IsUTF8Signature(buffer) || Has_UTF16_LE_BOM(buffer, cch) || Has_UTF16_BE_BOM(buffer, cch);

  if (!bHasSignature && !Settings.NoEncodingTags && !bDisableFileVar) {

    if (FileVars_ParseStr(buffer, "encoding", lpfv->tchEncoding, COUNTOF(lpfv->tchEncoding)))
      lpfv->mask |= FV_ENCODING;
    else if (FileVars_ParseStr(buffer, "charset", lpfv->tchEncoding, COUNTOF(lpfv->tchEncoding)))
      lpfv->mask |= FV_ENCODING;
    else if (FileVars_ParseStr(buffer, "coding", lpfv->tchEncoding, COUNTOF(lpfv->tchEncoding)))
      lpfv->mask |= FV_ENCODING;
  }
  if (lpfv->mask & FV_ENCODING) {
    lpfv->iEncoding = Encoding_MatchA(lpfv->tchEncoding);
  }

  if (!Flags.NoFileVariables && !bDisableFileVar) {
    if (FileVars_ParseStr(buffer, "mode", lpfv->tchMode, COUNTOF(lpfv->tchMode)))
      lpfv->mask |= FV_MODE;
  }
}

//=============================================================================
//
//  FileVars_Init()
//
bool FileVars_Init(char* lpData, DWORD cbData, LPFILEVARS lpfv) 
{
  ZeroMemory(lpfv, sizeof(FILEVARS));
  lpfv->bTabIndents     = Settings.TabIndents;
  lpfv->bTabsAsSpaces   = Settings.TabsAsSpaces;
  lpfv->bWordWrap       = Settings.WordWrap;
  lpfv->iTabWidth       = Settings.TabWidth;
  lpfv->iIndentWidth    = Settings.IndentWidth;
  lpfv->iLongLinesLimit = Settings.LongLinesLimit;
  lpfv->iEncoding       = Settings.DefaultEncoding;

  if ((Flags.NoFileVariables && Settings.NoEncodingTags) || !lpData || !cbData) {
    return true;
  }

  char tmpbuf[LARGE_BUFFER];
  size_t const cch = min_s(cbData + 1, COUNTOF(tmpbuf));

  StringCchCopyNA(tmpbuf, COUNTOF(tmpbuf), lpData, cch);
  _SetFileVars(tmpbuf, cch, lpfv);

  // if no file vars found, look at EOF
  if ((lpfv->mask == 0) && (cbData > COUNTOF(tmpbuf))) {
    StringCchCopyNA(tmpbuf, COUNTOF(tmpbuf), lpData + cbData - COUNTOF(tmpbuf) + 1, COUNTOF(tmpbuf));
    _SetFileVars(tmpbuf, cch, lpfv);
  }

  return true;
}


//=============================================================================
//
//  FileVars_Apply()
//
bool FileVars_Apply(LPFILEVARS lpfv) {

  int const _iTabWidth = (lpfv->mask & FV_TABWIDTH) ? lpfv->iTabWidth : Settings.TabWidth;
  SciCall_SetTabWidth(_iTabWidth);

  int const _iIndentWidth = (lpfv->mask & FV_INDENTWIDTH) ? lpfv->iIndentWidth : ((lpfv->mask & FV_TABWIDTH) ? 0 : Settings.IndentWidth);
  SciCall_SetIndent(_iIndentWidth);

  bool const _bTabsAsSpaces = (lpfv->mask & FV_TABSASSPACES) ? lpfv->bTabsAsSpaces : Settings.TabsAsSpaces;
  SciCall_SetUseTabs(!_bTabsAsSpaces);

  bool const _bTabIndents = (lpfv->mask & FV_TABINDENTS) ? lpfv->bTabIndents : Settings.TabIndents;
  SciCall_SetTabIndents(_bTabIndents);
  SciCall_SetBackSpaceUnIndents(Settings.BackspaceUnindents);

  bool const _bWordWrap = (lpfv->mask & FV_WORDWRAP) ? lpfv->bWordWrap : Settings.WordWrap;
  int const  _iWrapMode = _bWordWrap ? ((Settings.WordWrapMode == 0) ? SC_WRAP_WHITESPACE : SC_WRAP_CHAR) : SC_WRAP_NONE;
  SciCall_SetWrapMode(_iWrapMode);

  int const _iLongLinesLimit = (lpfv->mask & FV_LONGLINESLIMIT) ? lpfv->iLongLinesLimit : Settings.LongLinesLimit;
  SciCall_SetEdgeColumn(_iLongLinesLimit);
  Globals.iWrapCol = _iLongLinesLimit;

  return true;
}


//=============================================================================
//
//  FileVars_ParseInt()
//
bool FileVars_ParseInt(char* pszData,char* pszName,int* piValue) {

  char *pvStart = StrStrIA(pszData, pszName);
  while (pvStart) {
    char chPrev = (pvStart > pszData) ? *(pvStart-1) : 0;
    if (!IsCharAlphaNumericA(chPrev) && chPrev != '-' && chPrev != '_') {
      pvStart += StringCchLenA(pszName,0);
      while (*pvStart == ' ') {
        pvStart++;
      }
      if (*pvStart == ':' || *pvStart == '=') { break; }
    }
    else {
      pvStart += StringCchLenA(pszName, 0);
    }
    pvStart = StrStrIA(pvStart, pszName); // next
  }

  if (pvStart) {

    while (*pvStart && StrChrIA(":=\"' \t", *pvStart)) {
      pvStart++;
    }
    char tch[32] = { L'\0' };
    StringCchCopyNA(tch,COUNTOF(tch),pvStart,COUNTOF(tch));

    char* pvEnd = tch;
    while (*pvEnd && IsCharAlphaNumericA(*pvEnd)) {
      pvEnd++;
    }
    *pvEnd = 0;
    StrTrimA(tch," \t:=\"'");

    int itok = sscanf_s(tch,"%i",piValue);
    if (itok == 1) {
      return true;
    }
    if (tch[0] == 't') {
      *piValue = 1;
      return true;
    }
    if (tch[0] == 'n' || tch[0] == 'f') {
      *piValue = 0;
      return true;
    }
  }
  return false;
}


//=============================================================================
//
//  FileVars_ParseStr()
//
bool FileVars_ParseStr(char* pszData,char* pszName,char* pszValue,int cchValue) {

  char *pvStart = StrStrIA(pszData, pszName);
  while (pvStart) {
    char chPrev = (pvStart > pszData) ? *(pvStart-1) : 0;
    if (!IsCharAlphaNumericA(chPrev) && chPrev != '-' && chPrev != '_') {
      pvStart += StringCchLenA(pszName,0);
      while (*pvStart == ' ') {
        pvStart++;
      }
      if (*pvStart == ':' || *pvStart == '=') {
        break;
      }
    }
    else {
      pvStart += StringCchLenA(pszName, 0);
    }
    pvStart = StrStrIA(pvStart, pszName);  // next
  }

  if (pvStart) {

    bool bQuoted = false;
    while (*pvStart && StrChrIA(":=\"' \t",*pvStart)) {
      if (*pvStart == '\'' || *pvStart == '"')
        bQuoted = true;
      pvStart++;
    }

    char tch[32] = { L'\0' };
    StringCchCopyNA(tch,COUNTOF(tch),pvStart,COUNTOF(tch));

    char* pvEnd = tch;
    while (*pvEnd && (IsCharAlphaNumericA(*pvEnd) || StrChrIA("+-/_", *pvEnd) || (bQuoted && *pvEnd == ' '))) {
      pvEnd++;
    }
    *pvEnd = 0;

    StrTrimA(tch," \t:=\"'");

    StringCchCopyNA(pszValue,cchValue,tch,COUNTOF(tch));

    return true;
  }
  return false;
}


//=============================================================================
//
//  FileVars_IsUTF8()
//
bool FileVars_IsUTF8(LPFILEVARS lpfv) {
  if (lpfv->mask & FV_ENCODING) {
    if (StringCchCompareNIA(lpfv->tchEncoding,COUNTOF(lpfv->tchEncoding),"utf-8",CSTRLEN("utf-8")) == 0 ||
        StringCchCompareNIA(lpfv->tchEncoding,COUNTOF(lpfv->tchEncoding),"utf8", CSTRLEN("utf8")) == 0)
      return true;
  }
  return false;
}


//=============================================================================
//
//  FileVars_IsValidEncoding()
//
bool FileVars_IsValidEncoding(LPFILEVARS lpfv) {
  CPINFO cpi;
  if (lpfv->mask & FV_ENCODING && Encoding_IsValidIdx(lpfv->iEncoding)) {
    if ((Encoding_IsINTERNAL(lpfv->iEncoding)) ||
         (IsValidCodePage(Encoding_GetCodePage(lpfv->iEncoding)) &&
          GetCPInfo(Encoding_GetCodePage(lpfv->iEncoding),&cpi))) {
      return true;
    }
  }
  return false;
}


//=============================================================================
//
//  FileVars_GetEncoding()
//
cpi_enc_t FileVars_GetEncoding(LPFILEVARS lpfv) 
{
  if (lpfv->mask & FV_ENCODING) {
    return(lpfv->iEncoding);
  }
  return CPI_NONE;
}


//=============================================================================
//
//  EditBookmarkClick()
//
void  EditBookmarkClick(const DocLn ln, const int modifiers)
{
  UNUSED(modifiers);

  int const bitmask = SciCall_MarkerGet(ln);

  if (bitmask & (1 << MARKER_NP3_BOOKMARK))
  {
    SciCall_MarkerDelete(ln, MARKER_NP3_BOOKMARK); // unset
  }
  else {
    SciCall_MarkerAdd(ln, MARKER_NP3_BOOKMARK);    // set
  }

  if (modifiers & SCMOD_ALT) {
    SciCall_GotoLine(ln);
  }
}


//==============================================================================
//
//  Folding Functions
//
//
#define FOLD_CHILDREN SCMOD_CTRL
#define FOLD_SIBLINGS SCMOD_SHIFT

inline bool _FoldToggleNode(const DocLn ln, const FOLD_ACTION action)
{
  bool const fExpanded = SciCall_GetFoldExpanded(ln);
  if ((action == SNIFF) || ((action == FOLD) && fExpanded) || ((action == EXPAND) && !fExpanded))
  {
    SciCall_ToggleFold(ln);
    return true;
  }
  return false;
}


void EditFoldPerformAction(DocLn ln, int mode, FOLD_ACTION action)
{
  bool fToggled = false;
  if (action == SNIFF) {
    action = SciCall_GetFoldExpanded(ln) ? FOLD : EXPAND;
  }
  if (mode & (FOLD_CHILDREN | FOLD_SIBLINGS))
  {
    // ln/lvNode: line and level of the source of this fold action
    DocLn const lnNode = ln;
    int const lvNode = SciCall_GetFoldLevel(lnNode) & SC_FOLDLEVELNUMBERMASK;
    DocLn const lnTotal = SciCall_GetLineCount();

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
      bool fHeader = lv & SC_FOLDLEVELHEADERFLAG;
      lv &= SC_FOLDLEVELNUMBERMASK;

      if (lv < lvStop || (lv == lvStop && fHeader && ln != lnNode)) {
        return;
      }
      if (fHeader && (lv == lvNode || (lv > lvNode && mode & FOLD_CHILDREN))) {
        fToggled |= _FoldToggleNode(ln, action);
      }
    }
  }
  else {
    fToggled = _FoldToggleNode(ln, action);
  }
}


void EditToggleFolds(FOLD_ACTION action, bool bForceAll)
{
  DocLn const iBegLn = SciCall_LineFromPosition(SciCall_GetSelectionStart());
  DocLn const iEndLn = SciCall_LineFromPosition(SciCall_GetSelectionEnd());

  if (bForceAll) {
    SciCall_FoldAll(action);
  }
  else // in selection
  {
    if (iBegLn == iEndLn) {
      // single line
      DocLn const ln = (SciCall_GetFoldLevel(iBegLn) & SC_FOLDLEVELHEADERFLAG) ? iBegLn : SciCall_GetFoldParent(iBegLn);
      if (_FoldToggleNode(ln, action)) { SciCall_ScrollCaret(); }
    }
    else {
      // selection range spans at least two lines
      bool fToggled = bForceAll;
      for (DocLn ln = iBegLn; ln <= iEndLn; ++ln) {
        if (SciCall_GetFoldLevel(ln) & SC_FOLDLEVELHEADERFLAG) {
          fToggled |= _FoldToggleNode(ln, action);
        }
      }
      if (fToggled) { EditEnsureSelectionVisible(Globals.hwndEdit); }
    }
  }
}


void EditFoldClick(DocLn ln, int mode)
{
  static struct {
    DocLn ln;
    int mode;
    DWORD dwTickCount;
  } prev;

  bool fGotoFoldPoint = mode & FOLD_SIBLINGS;

  if (!(SciCall_GetFoldLevel(ln) & SC_FOLDLEVELHEADERFLAG))
  {
    // Not a fold point: need to look for a double-click
    if (prev.ln == ln && prev.mode == mode &&
      GetTickCount() - prev.dwTickCount <= GetDoubleClickTime())
    {
      prev.ln = (DocLn)-1;  // Prevent re-triggering on a triple-click

      ln = SciCall_GetFoldParent(ln);

      if (ln >= 0 && SciCall_GetFoldExpanded(ln))
        fGotoFoldPoint = true;
      else
        return;
    }
    else
    {
      // Save the info needed to match this click with the next click
      prev.ln = ln;
      prev.mode = mode;
      // cppcheck-suppress unreadVariable
      prev.dwTickCount = GetTickCount();
      return;
    }
  }

  EditFoldPerformAction(ln, mode, SNIFF);

  if (fGotoFoldPoint) {
    EditJumpTo(Globals.hwndEdit, ln + 1, 0);
  }
}


void EditFoldAltArrow(FOLD_MOVE move, FOLD_ACTION action)
{
  if (FocusedView.CodeFoldingAvailable && FocusedView.ShowCodeFolding)
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
          EditJumpTo(Globals.hwndEdit, ln + 1, 0);
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
          EditJumpTo(Globals.hwndEdit, ln + 1, 0);
          return;
        }
      }
    }

    // Perform a fold/unfold operation
    DocLn const iBegLn = SciCall_LineFromPosition(SciCall_GetSelectionStart());
    DocLn const iEndLn = SciCall_LineFromPosition(SciCall_GetSelectionEnd());

    // selection range must span at least two lines for action
    if (iBegLn != iEndLn) {
      EditToggleFolds(action, false);
    }
    else {
      ln = (SciCall_GetFoldLevel(iBegLn) & SC_FOLDLEVELHEADERFLAG) ? iBegLn : SciCall_GetFoldParent(iBegLn);
      if (action != SNIFF) {
        if (_FoldToggleNode(ln, action)) { SciCall_ScrollCaret(); }
      }
    }
  }
}

///   End of Edit.c   ///
