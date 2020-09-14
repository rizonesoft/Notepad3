// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Edit.c                                                                      *
*   Text File Editing Helper Stuff                                            *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2020   *
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
#include <time.h>

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

static WCHAR W_DelimChars[ANSI_CHAR_BUFFER] = { L'\0' };
static WCHAR W_DelimCharsAccel[ANSI_CHAR_BUFFER] = { L'\0' };
static WCHAR W_WhiteSpaceCharsDefault[ANSI_CHAR_BUFFER] = { L'\0' };
static WCHAR W_WhiteSpaceCharsAccelerated[ANSI_CHAR_BUFFER] = { L'\0' };

static char AutoCompleteFillUpChars[64] = { '\0' };
static bool s_ACFillUpCharsHaveNewLn = false;

// Default Codepage and Character Set
#define W_AUTOC_WORD_ANSI1252 L"#$%&@0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyzÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýþÿ"
static char AutoCompleteWordCharSet[ANSI_CHAR_BUFFER] = { L'\0' };

// Is the character a white space char?
#define IsWhiteSpace(ch)  StrChrA(WhiteSpaceCharsDefault, (ch))
#define IsAccelWhiteSpace(ch)  StrChrA(WhiteSpaceCharsAccelerated, (ch))
#define IsWhiteSpaceW(wch)  StrChrW(W_WhiteSpaceCharsDefault, (wch))
#define IsAccelWhiteSpaceW(wch)  StrChrW(W_WhiteSpaceCharsAccelerated, (wch))

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
  SORT_LEXICOGRAPH = 0x400,
  SORT_COLUMN      = 0x800 
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
//  EditReplaceSelection()
//
void EditReplaceSelection(const char* text, bool bForceSel)
{
  _BEGIN_UNDO_ACTION_;
  bool const bSelWasEmpty = SciCall_IsSelectionEmpty();
  DocPos const posSelBeg = SciCall_GetSelectionStart();
  SciCall_ReplaceSel(text);
  if (bForceSel || !bSelWasEmpty) {
    SciCall_SetSel(posSelBeg, SciCall_GetCurrentPos());
  }
  _END_UNDO_ACTION_;
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
    WideCharToMultiByte(Encoding_SciCP, 0, Settings2.ExtendedWhiteSpaceChars, -1, whitesp, (int)COUNTOF(whitesp), NULL, NULL);
  }

  // 3rd set accelerated arrays

  // init with default
  StringCchCopyA(WhiteSpaceCharsAccelerated, COUNTOF(WhiteSpaceCharsAccelerated), WhiteSpaceCharsDefault);

  // add only 7-bit-ASCII chars to accelerated whitespace list
  size_t const wsplen = StringCchLenA(whitesp, ANSI_CHAR_BUFFER);
  for (size_t i = 0; i < wsplen; i++) {
    if (whitesp[i] & 0x7F) {
      if (!StrChrA(WhiteSpaceCharsAccelerated, whitesp[i])) {
        StringCchCatNA(WhiteSpaceCharsAccelerated, COUNTOF(WhiteSpaceCharsAccelerated), &(whitesp[i]), 1);
      }
    }
  }

  // construct word char array
  StringCchCopyA(WordCharsAccelerated, COUNTOF(WordCharsAccelerated), WordCharsDefault); // init
  // add punctuation chars not listed in white-space array
  size_t const pcdlen = StringCchLenA(PunctuationCharsDefault, ANSI_CHAR_BUFFER);
  for (size_t i = 0; i < pcdlen; i++) {
    if (!StrChrA(WhiteSpaceCharsAccelerated, PunctuationCharsDefault[i])) {
      StringCchCatNA(WordCharsAccelerated, COUNTOF(WordCharsAccelerated), &(PunctuationCharsDefault[i]), 1);
    }
  }

  // construct accelerated delimiters
  StringCchCopyA(DelimCharsAccel, COUNTOF(DelimCharsAccel), WhiteSpaceCharsDefault);
  StringCchCatA(DelimCharsAccel, COUNTOF(DelimCharsAccel), lineEnds);

  if (StrIsNotEmpty(Settings2.AutoCompleteFillUpChars))
  {
    WideCharToMultiByte(Encoding_SciCP, 0, Settings2.AutoCompleteFillUpChars, -1, AutoCompleteFillUpChars, (int)COUNTOF(AutoCompleteFillUpChars), NULL, NULL);
    UnSlashA(AutoCompleteFillUpChars, Encoding_SciCP);

    s_ACFillUpCharsHaveNewLn = false;
    int i = 0;
    while (AutoCompleteFillUpChars[i]) {
      if ((AutoCompleteFillUpChars[i] == '\r') || (AutoCompleteFillUpChars[i] == '\n')) {
        s_ACFillUpCharsHaveNewLn = true;
        break;
      }
      ++i;
    }
  }

  if (StrIsNotEmpty(Settings2.AutoCompleteWordCharSet))
  {
    WideCharToMultiByte(Encoding_SciCP, 0, Settings2.AutoCompleteWordCharSet, -1, AutoCompleteWordCharSet, (int)COUNTOF(AutoCompleteWordCharSet), NULL, NULL);
    Globals.bUseLimitedAutoCCharSet = true;
  } else {
    WideCharToMultiByte(Encoding_SciCP, 0, W_AUTOC_WORD_ANSI1252, -1, AutoCompleteWordCharSet, (int)COUNTOF(AutoCompleteWordCharSet), NULL, NULL);
    Globals.bUseLimitedAutoCCharSet = false;
  }

  // construct wide char arrays
  MultiByteToWideChar(Encoding_SciCP, 0, DelimChars, -1, W_DelimChars, (int)COUNTOF(W_DelimChars));
  MultiByteToWideChar(Encoding_SciCP, 0, DelimCharsAccel, -1, W_DelimCharsAccel, (int)COUNTOF(W_DelimCharsAccel));
  MultiByteToWideChar(Encoding_SciCP, 0, WhiteSpaceCharsDefault, -1, W_WhiteSpaceCharsDefault, (int)COUNTOF(W_WhiteSpaceCharsDefault));
  MultiByteToWideChar(Encoding_SciCP, 0, WhiteSpaceCharsAccelerated, -1, W_WhiteSpaceCharsAccelerated, (int)COUNTOF(W_WhiteSpaceCharsAccelerated));
}


//=============================================================================
//
//  EditSetNewText()
//
extern bool bFreezeAppTitle;

void EditSetNewText(HWND hwnd, const char* lpstrText, DocPosU lenText, bool bClearUndoHistory)
{
  if (!lpstrText) { lenText = 0; }

  bFreezeAppTitle = true;

  // clear markers, flags and positions
  if (FocusedView.HideNonMatchedLines) { EditToggleView(hwnd); }
  if (bClearUndoHistory) {
    UndoRedoRecordingStop();
  }
  _IGNORE_NOTIFY_CHANGE_;
  SciCall_Cancel();
  if (SciCall_GetReadOnly()) { SciCall_SetReadOnly(false); }
  EditClearAllBookMarks(hwnd);
  EditClearAllOccurrenceMarkers(hwnd);
  SciCall_SetScrollWidth(1);
  SciCall_SetXOffset(0);
  _OBSERVE_NOTIFY_CHANGE_;

  FileVars_Apply(&Globals.fvCurFile);

  _IGNORE_NOTIFY_CHANGE_;
  EditSetDocumentBuffer(lpstrText, lenText);
  _OBSERVE_NOTIFY_CHANGE_;

  Sci_GotoPosChooseCaret(0);

  if (bClearUndoHistory) {
    UndoRedoRecordingStart();
  }

  bFreezeAppTitle = false;
}



//=============================================================================
//
//  EditConvertText()
//
bool EditConvertText(HWND hwnd, cpi_enc_t encSource, cpi_enc_t encDest)
{
  if ((encSource == encDest) || (Encoding_SciCP == encDest)) {
    return false;
  }
  if (!(Encoding_IsValid(encSource) && Encoding_IsValid(encDest))) {
    return false;
  }

  DocPos const length = SciCall_GetTextLength();

  if (length <= 0) {
    EditSetNewText(hwnd, "", 0, true);
    return false;
  }

  const DocPos chBufSize = length * 5 + 2;
  char*        pchText   = AllocMem(chBufSize, HEAP_ZERO_MEMORY);

  struct Sci_TextRange tr = {{0, -1}, NULL};
  tr.lpstrText            = pchText;
  DocPos const rlength    = SciCall_GetTextRange(&tr);

  const DocPos wchBufSize = rlength * 3 + 2;
  WCHAR*       pwchText   = AllocMem(wchBufSize, HEAP_ZERO_MEMORY);

  // MultiBytes(Sci) -> WideChar(destination) -> Sci(MultiByte)
  const UINT cpDst = Encoding_GetCodePage(encDest);

  // get text as wide char
  ptrdiff_t cbwText = MultiByteToWideCharEx(Encoding_SciCP, 0, pchText, length, pwchText, wchBufSize);
  // convert wide char to destination multibyte
  ptrdiff_t cbText = WideCharToMultiByteEx(cpDst, 0, pwchText, cbwText, pchText, chBufSize, NULL, NULL);
  // re-code to wide char
  cbwText = MultiByteToWideCharEx(cpDst, 0, pchText, cbText, pwchText, wchBufSize);
  // convert to Scintilla format
  cbText = WideCharToMultiByteEx(Encoding_SciCP, 0, pwchText, cbwText, pchText, chBufSize, NULL, NULL);

  pchText[cbText]     = '\0';
  pchText[cbText + 1] = '\0';

  FreeMem(pwchText);

  EditSetNewText(hwnd, pchText, cbText, true);

  FreeMem(pchText);

  Encoding_Current(encDest);

  return true;
}


//=============================================================================
//
//  EditSetNewEncoding()
//
bool EditSetNewEncoding(HWND hwnd, cpi_enc_t iNewEncoding, bool bSupressWarning)
{
  cpi_enc_t iCurrentEncoding = Encoding_GetCurrent();

  if (iCurrentEncoding != iNewEncoding) {

    // suppress recoding message for certain encodings
    UINT const currentCP = Encoding_GetCodePage(iCurrentEncoding); 
    UINT const targetCP  = Encoding_GetCodePage(iNewEncoding);
    if (((currentCP == 936) && (targetCP == 54936)) || ((currentCP == 54936) && (targetCP == 936))) {
      bSupressWarning = true;
    }

    if (Sci_IsDocEmpty()) 
    {
      bool const doNewEncoding = (Sci_HaveUndoRedoHistory() && !bSupressWarning) ?
        (InfoBoxLng(MB_YESNO, L"MsgConv2", IDS_MUI_ASK_ENCODING2) == IDYES) : true;

      if (doNewEncoding) {
        return EditConvertText(hwnd, iCurrentEncoding, iNewEncoding);
      }
    }
    else {

      if (!bSupressWarning) {
        bool const bIsCurANSI   = Encoding_IsANSI(iCurrentEncoding);
        bool const bIsTargetUTF = Encoding_IsUTF8(iNewEncoding) || Encoding_IsUNICODE(iNewEncoding);
        bSupressWarning         = bIsCurANSI && bIsTargetUTF;
      }

      bool const doNewEncoding = (!bSupressWarning) ? (InfoBoxLng(MB_YESNO, L"MsgConv1", IDS_MUI_ASK_ENCODING) == IDYES) : true;

      if (doNewEncoding) {
        return EditConvertText(hwnd, iCurrentEncoding, iNewEncoding);
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

  UINT codepage = Encoding_GetCodePage(Encoding_GetCurrent());

  if ((codepage == CP_UTF7) || (codepage == CP_UTF8))
    return false;

  DWORD dwFlags = WC_NO_BEST_FIT_CHARS | WC_COMPOSITECHECK | WC_DEFAULTCHAR;
  bool  useNullParams = Encoding_IsMBCS(Encoding_GetCurrent()) ? true : false;

  BOOL bDefaultCharsUsed = FALSE;
  ptrdiff_t cch = 0;
  if (useNullParams)
    cch = WideCharToMultiByteEx(codepage, 0, pszText, cchLen, NULL, 0, NULL, NULL);
  else
    cch = WideCharToMultiByteEx(codepage, dwFlags, pszText, cchLen, NULL, 0, NULL, &bDefaultCharsUsed);

  if (useNullParams && (cch == 0)) {
    if (GetLastError() != ERROR_NO_UNICODE_TRANSLATION)
      cch = cchLen; // don't care
  }

  bool bSuccess = ((cch >= cchLen) && (cch != 0xFFFD)) ? true : false;
  
  return (!bSuccess || bDefaultCharsUsed);
}



//=============================================================================
//
//  EditGetSelectedText()
//
size_t EditGetSelectedText(LPWSTR pwchBuffer, size_t wchLength)
{
  if (!pwchBuffer || (wchLength == 0)) { return 0; }
  size_t const selSize = SciCall_GetSelText(NULL);
  if (1 < selSize) {
    char* pszText = AllocMem(selSize, HEAP_ZERO_MEMORY);
    if (pszText) {
      SciCall_GetSelText(pszText);
      size_t const length = (size_t)MultiByteToWideChar(Encoding_SciCP, 0, pszText, -1, pwchBuffer, (int)wchLength);
      FreeMem(pszText);
      return length;
    }
  }
  if (wchLength > 0) {
    pwchBuffer[0] = L'\0';
    return selSize;
  }
  return 0;
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
  ptrdiff_t mlen = 0;
  if (wlen > 0) {
    mlen = WideCharToMultiByteEx(Encoding_SciCP, 0, pwch, wlen, NULL, 0, NULL, NULL);
    pmch = (char*)AllocMem(mlen + 1, HEAP_ZERO_MEMORY);
    if (pmch && mlen != 0) {
      ptrdiff_t const cnt = WideCharToMultiByteEx(Encoding_SciCP, 0, pwch, wlen, pmch, SizeOfMem(pmch), NULL, NULL);
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
        StringCchCopyA(pmch, SizeOfMem(pmch), ptmp);
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
//  EditGetClipboardW()
//
void EditGetClipboardW(LPWSTR pwchBuffer, size_t wchLength) 
{
  if (!IsClipboardFormatAvailable(CF_UNICODETEXT) || !OpenClipboard(Globals.hwndMain)) { return; }

  HANDLE const hmem = GetClipboardData(CF_UNICODETEXT);
  if (hmem) {
    const WCHAR* const pwch = GlobalLock(hmem);
    if (pwch) {
      StringCchCopyW(pwchBuffer, wchLength, pwch);
    }
    GlobalUnlock(hmem);
  }
  CloseClipboard();
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
  ptrdiff_t const cchTextW = MultiByteToWideCharEx(Encoding_SciCP, 0, pszText, cchText, NULL, 0);
  if (cchTextW > 1) {
    pszTextW = AllocMem((cchTextW + 1) * sizeof(WCHAR), HEAP_ZERO_MEMORY);
    if (pszTextW) {
      MultiByteToWideCharEx(Encoding_SciCP, 0, pszText, cchText, pszTextW, cchTextW + 1);
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
  if (!OpenClipboard(GetParent(hwnd))) { return false; }
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

  _BEGIN_UNDO_ACTION_;

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
  FreeMem(pszText);

  _END_UNDO_ACTION_;

  if (!Sci_IsMultiOrRectangleSelection())
  {
    //~_BEGIN_UNDO_ACTION_;
    if (iCurPos < iAnchorPos) {
      EditSetSelectionEx(iCurPos + clipLen, iCurPos, -1, -1);
    }
    else {
      EditSetSelectionEx(iAnchorPos, iAnchorPos + clipLen, -1, -1);
  }
    //~_END_UNDO_ACTION_;
  }
  else {
    // TODO: restore rectangular selection in case of swap clipboard 
  }

  FreeMem(pClip);
  return true;
}


//=============================================================================
//
//  EditCopyRangeAppend()
//
bool EditCopyRangeAppend(HWND hwnd, DocPos posBegin, DocPos posEnd, bool bAppend)
{
  if (posBegin > posEnd) {
    swapos(&posBegin, &posEnd);
  }
  DocPos const length = (posEnd - posBegin);
  if (length == 0) { return true; }

  const char* const pszText = SciCall_GetRangePointer(posBegin, length);

  WCHAR* pszTextW = NULL;
  ptrdiff_t cchTextW = 0;
  if (pszText && *pszText) {
    cchTextW = MultiByteToWideCharEx(Encoding_SciCP, 0, pszText, length, NULL, 0);
    if (cchTextW > 0) {
      pszTextW = AllocMem((cchTextW + 1) * sizeof(WCHAR), HEAP_ZERO_MEMORY);
      if (pszTextW) {
        MultiByteToWideCharEx(Encoding_SciCP, 0, pszText, length, pszTextW, cchTextW + 1);
        pszTextW[cchTextW] = L'\0';
      }
    }
  }

  bool res = false;
  HWND const hwndParent = GetParent(hwnd);

  if (!bAppend) {
    res = SetClipboardTextW(hwndParent, pszTextW, cchTextW);
    FreeMem(pszTextW);
    return res;
  }

  // --- Append to Clipboard ---

  if (!OpenClipboard(hwndParent)) {
    FreeMem(pszTextW);
    return res;
  }

  HANDLE const hOld   = GetClipboardData(CF_UNICODETEXT);
  const WCHAR* pszOld = GlobalLock(hOld);

  WCHAR pszSep[3] = { L'\0' };
  Sci_GetCurrentEOL_W(pszSep);

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
    StringCchCat(pszNewTextW, cchNewText + 1, pszTextW);
    res = SetClipboardTextW(hwndParent, pszNewTextW, cchNewText);
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
void EditDetectEOLMode(LPCSTR lpData, size_t cbData, EditFileIOStatus* const status)
{
  if (!lpData || (cbData == 0)) { return; }

  /* '\r' and '\n' is not reused (e.g. as trailing byte in DBCS) by any known encoding,
  it's safe to check whole data byte by byte.*/

  DocLn lineCountCRLF = 0;
  DocLn lineCountCR = 0;
  DocLn lineCountLF = 0;

  // tools/GenerateTable.py
  static const uint8_t eol_table[16] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 0, 0, // 00 - 0F
  };

  const uint8_t* ptr = (const uint8_t*)lpData;
  // No NULL-terminated requirement for *ptr == '\n'
  const uint8_t* const end = (const uint8_t*)lpData + cbData - 1;

  do {
    // skip to line end
    uint8_t ch;
    uint8_t type = 0;
    while (ptr < end && ((ch = *ptr++) > '\r' || (type = eol_table[ch]) == 0)) {} // nop
    switch (type) {
      case 1: //'\n'
        ++lineCountLF;
        break;
      case 2: //'\r'
        if (*ptr == '\n') {
          ++ptr;
          ++lineCountCRLF;
        }
        else {
          ++lineCountCR;
        }
        break;
    }
  } while (ptr < end);

  if (ptr == end) {
    switch (*ptr) {
      case '\n':
        ++lineCountLF;
        break;
      case '\r':
        ++lineCountCR;
        break;
    }
  }

  // values must kept in same order as SC_EOL_CRLF(0), SC_EOL_CR(1), SC_EOL_LF(2)
  DocLn const linesMax = max_ln(max_ln(lineCountCRLF, lineCountCR), lineCountLF);
  DocLn linesCount[3] = { 0, 0, 0 };
  linesCount[SC_EOL_CRLF] = lineCountCRLF;
  linesCount[SC_EOL_CR] = lineCountCR;
  linesCount[SC_EOL_LF] = lineCountLF;

  int iEOLMode = status->iEOLMode;
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

  status->bInconsistentEOLs = 1 < ((!!lineCountCRLF) + (!!lineCountCR) + (!!lineCountLF));
  status->eolCount[SC_EOL_CRLF] = lineCountCRLF;
  status->eolCount[SC_EOL_CR] = lineCountCR;
  status->eolCount[SC_EOL_LF] = lineCountLF;
}


//=============================================================================
//
// EditIndentationCount() - check indentation consistency
//
void EditIndentationStatistic(HWND hwnd, EditFileIOStatus* const status)
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

  if (Flags.bLargeFileLoaded) { return; }

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
        if ((indentWidth > 0) && (subSpcCnt >= indentWidth)) {
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
      if ((tabWidth > 0) && (lineIndentDepth % tabWidth)) {
        ++(status->indentCount[I_TAB_MOD_X]);
      }
      if ((indentWidth > 0) && (lineIndentDepth % indentWidth)) {
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
  EditFileIOStatus* const status)
{
  status->iEncoding = Settings.DefaultEncoding;
  status->bUnicodeErr = false;
  status->bUnknownExt = false;
  status->bEncryptedRaw = false;
  Flags.bLargeFileLoaded = false;

  HANDLE hFile = CreateFile(pszFile,
    GENERIC_READ,
    FILE_SHARE_READ|FILE_SHARE_WRITE,
    NULL,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    NULL);

  Globals.dwLastError = GetLastError();

  if (hFile == INVALID_HANDLE_VALUE) {
    Encoding_Forced(CPI_NONE);
    return false;
  }

  // calculate buffer size and limits

  LARGE_INTEGER liFileSize = { 0, 0 };
  bool const okay = GetFileSizeEx(hFile, &liFileSize);
  //DWORD const fileSizeMB = (DWORD)liFileSize.HighPart * (DWORD_MAX >> 20) + (liFileSize.LowPart >> 20);
  
  bool const bLargerThan2GB = okay && ((liFileSize.HighPart > 0) || (liFileSize.LowPart >= (DWORD)INT32_MAX));

  if (!okay || bLargerThan2GB) {
    if (!okay) {
      Globals.dwLastError = GetLastError();
      CloseHandle(hFile);
      return false;
    }
    else {
#ifdef _WIN64
      // can only handle ASCII/UTF-8 of this size
      Encoding_Forced(CPI_UTF8);
      // @@@ TODO: Scintilla can't handle files larger than 4GB :-( yet (2020-02-25)
      bool const bFileTooBig = (liFileSize.HighPart > 0); // > DWORD_MAX 
#else
      bool const bFileTooBig = true; // _WIN32: file size < 2GB only
#endif
      if (bFileTooBig) {
        // refuse to handle file in 32-bit
        WCHAR sizeStr[64] = { L'\0' };
        StrFormatByteSize((LONGLONG)liFileSize.QuadPart, sizeStr, COUNTOF(sizeStr));
        InfoBoxLng(MB_ICONERROR, NULL, IDS_MUI_ERR_FILE_TOO_LARGE, sizeStr);
        CloseHandle(hFile);
        Encoding_Forced(CPI_NONE);
        Flags.bLargeFileLoaded = true;
        return false;
      }
    }
  }

  size_t const fileSize = (size_t)liFileSize.QuadPart;

  // Check if a warning message should be displayed for large files
  size_t const fileSizeWarning = (size_t)Settings2.FileLoadWarningMB << 20;
  if ((fileSizeWarning != 0ULL) && (fileSizeWarning <= fileSize)) 
  {
    WCHAR sizeStr[64] = { L'\0' };
    StrFormatByteSize((LONGLONG)liFileSize.QuadPart, sizeStr, COUNTOF(sizeStr));
    WCHAR sizeWarnStr[64] = { L'\0' };
    StrFormatByteSize((LONGLONG)fileSizeWarning, sizeWarnStr, COUNTOF(sizeWarnStr));
    if (InfoBoxLng(MB_YESNO, L"MsgFileSizeWarning", IDS_MUI_WARN_LOAD_BIG_FILE, sizeStr, sizeWarnStr) != IDYES) {
      CloseHandle(hFile);
      Encoding_Forced(CPI_NONE);
      return false;
    }
    Flags.bLargeFileLoaded = true;
  }

  // check for unknown file/extension
  status->bUnknownExt = false;
  if (!Style_HasLexerForExt(pszFile)) {
    INT_PTR const answer = InfoBoxLng(MB_YESNO, L"MsgFileUnknownExt", IDS_MUI_WARN_UNKNOWN_EXT, PathFindFileName(pszFile));
    if (!((IDOK == answer) || (IDYES == answer))) {
      CloseHandle(hFile);
      Encoding_Forced(CPI_NONE);
      status->bUnknownExt = true;
      return false;
    }
  }
  
  // new document text buffer
  char* lpData = AllocMem(fileSize + 2ULL, HEAP_ZERO_MEMORY);
  if (!lpData)
  {
    Globals.dwLastError = GetLastError();
    CloseHandle(hFile);
    Encoding_Forced(CPI_NONE);
    Flags.bLargeFileLoaded = true;
    return false;
  }

  size_t cbData = 0LL;
  int const readFlag = ReadAndDecryptFile(hwnd, hFile, fileSize, (void**)&lpData, &cbData);
  Globals.dwLastError = GetLastError();
  CloseHandle(hFile);

  if (cbData == 0) {
    FileVars_GetFromData(NULL, 0, &Globals.fvCurFile); // init-reset
    status->iEOLMode = Settings.DefaultEOLMode;
    EditSetNewText(hwnd, "", 0, bClearUndoHistory);
    SciCall_SetEOLMode(Settings.DefaultEOLMode);
    Encoding_Forced(CPI_NONE);
    FreeMem(lpData);
    return true;
  }

  bool bReadSuccess = ((readFlag & DECRYPT_FATAL_ERROR) || (readFlag & DECRYPT_FREAD_FAILED)) ? false : true;
  
  if ((readFlag & DECRYPT_CANCELED_NO_PASS) || (readFlag & DECRYPT_WRONG_PASS))
  {
    bReadSuccess = (InfoBoxLng(MB_OKCANCEL, L"MsgNoOrWrongPassphrase", IDS_MUI_NOPASS) == IDOK);
    if (!bReadSuccess) {
      Encoding_Forced(CPI_NONE);
      FreeMem(lpData);
      return true;
    }
    else {
      status->bEncryptedRaw =  true;
    }
  }
  if (!bReadSuccess) {
    Encoding_Forced(CPI_NONE);
    FreeMem(lpData);
    return false;
  }

  // force very large file to be ASCII/UTF-8 (!) - Scintilla can't handle it otherwise
  if (bLargerThan2GB) 
  {
    bool const bIsUTF8Sig = IsUTF8Signature(lpData);
    Encoding_Forced(bIsUTF8Sig ? CPI_UTF8SIGN : CPI_UTF8);
    
    FileVars_GetFromData(NULL, 0, &Globals.fvCurFile); // init-reset
    status->iEncoding = Encoding_Forced(CPI_GET);
    status->iEOLMode = Settings.DefaultEOLMode;

    if (bIsUTF8Sig) {
      EditSetNewText(hwnd, UTF8StringStart(lpData), cbData - 3, bClearUndoHistory);
    }
    else {
      EditSetNewText(hwnd, lpData, cbData, bClearUndoHistory);
    }

    SciCall_SetEOLMode(Settings.DefaultEOLMode);

    FreeMem(lpData);
    return true;
  }

  // --------------------------------------------------------------------------

  ENC_DET_T const encDetection = Encoding_DetectEncoding(pszFile, lpData, cbData, 
                                                         Settings.UseDefaultForFileEncoding ? Settings.DefaultEncoding : CPI_PREFERRED_ENCODING,
                                                         bSkipUTFDetection, bSkipANSICPDetection, bForceEncDetection);

  #define IS_ENC_ENFORCED() (!Encoding_IsNONE(encDetection.forcedEncoding))

  // --------------------------------------------------------------------------

  if (Flags.bDevDebugMode) {
#if TRUE
    SetAdditionalTitleInfo(Encoding_GetTitleInfo());
#else
    DocPos const iPos = SciCall_PositionFromLine(SciCall_GetFirstVisibleLine());
    int const iXOff = SciCall_GetXOffset();
    SciCall_SetXOffset(0);
    SciCall_CallTipShow(iPos, Encoding_GetTitleInfoA());
    SciCall_SetXOffset(iXOff);
    Globals.CallTipType = CT_ENC_INFO;
#endif

    if (IS_ENC_ENFORCED()) {
      WCHAR wchBuf[128] = { L'\0' };
      StringCchPrintf(wchBuf, COUNTOF(wchBuf), L"ForcedEncoding='%s'", g_Encodings[encDetection.forcedEncoding].wchLabel);
      SetAdditionalTitleInfo(wchBuf);
    }

    if (!Encoding_IsNONE(encDetection.fileVarEncoding) && FileVars_IsValidEncoding(&Globals.fvCurFile)) {
      WCHAR wchBuf[128] = { L'\0' };
      StringCchPrintf(wchBuf, COUNTOF(wchBuf), L" - FilEncTag='%s'",
        g_Encodings[FileVars_GetEncoding(&Globals.fvCurFile)].wchLabel);
      AppendAdditionalTitleInfo(wchBuf);
    }

    WCHAR wcBuf[128] = { L'\0' };
    StringCchPrintf(wcBuf, ARRAYSIZE(wcBuf), L" - OS-CP='%s'", g_Encodings[CPI_ANSI_DEFAULT].wchLabel);
    AppendAdditionalTitleInfo(wcBuf);
  }
  
  // --------------------------------------------------------------------------
  // ===  UNICODE  ( UTF-16LE / UTF-16BE ) ===
  // --------------------------------------------------------------------------

  bool const bIsUnicodeDetected = !IS_ENC_ENFORCED() && Encoding_IsUNICODE(encDetection.unicodeAnalysis);

  if (Encoding_IsUNICODE(encDetection.Encoding) || bIsUnicodeDetected)
  {
    // ----------------------------------------------------------------------
    status->iEncoding = encDetection.bHasBOM ? (encDetection.bIsReverse ? CPI_UNICODEBEBOM : CPI_UNICODEBOM) : 
                                               (encDetection.bIsReverse ? CPI_UNICODEBE    : CPI_UNICODE);
    // ----------------------------------------------------------------------

    if (encDetection.bIsReverse) { SwabEx(lpData, lpData, cbData); }

    char* const lpDataUTF8 = AllocMem((cbData * 3) + 2, HEAP_ZERO_MEMORY);

    ptrdiff_t convCnt = WideCharToMultiByteEx(Encoding_SciCP, 0, (encDetection.bHasBOM ? (LPWSTR)lpData + 1 : (LPWSTR)lpData),
      (encDetection.bHasBOM ? (cbData / sizeof(WCHAR)) : (cbData / sizeof(WCHAR) + 1)), lpDataUTF8, SizeOfMem(lpDataUTF8), NULL, NULL);

    if (convCnt == 0) {
      convCnt = WideCharToMultiByteEx(CP_ACP, 0, (encDetection.bHasBOM ? (LPWSTR)lpData + 1 : (LPWSTR)lpData),
        -1, lpDataUTF8, SizeOfMem(lpDataUTF8), NULL, NULL);
      status->bUnicodeErr = true;
    }

    FileVars_GetFromData(lpDataUTF8, convCnt - 1, &Globals.fvCurFile);
    EditSetNewText(hwnd, lpDataUTF8, convCnt - 1, bClearUndoHistory);
    EditDetectEOLMode(lpDataUTF8, convCnt - 1, status);
    FreeMem(lpDataUTF8);

  }
  else  // ===  ALL OTHERS  ===
  {
    // ===  UTF-8 ? ===
    bool const bValidUTF8 = encDetection.bValidUTF8;
    bool const bForcedUTF8 = Encoding_IsUTF8(encDetection.forcedEncoding);// ~ don't || encDetection.bIsUTF8Sig here !
    bool const bAnalysisUTF8 = Encoding_IsUTF8(encDetection.Encoding);

    bool const bRejectUTF8 = (IS_ENC_ENFORCED() && !bForcedUTF8) || !bValidUTF8 || (!encDetection.bIsUTF8Sig && bSkipUTFDetection);
    bool const bIsCP_UTF7 = (Encoding_GetCodePage(encDetection.Encoding) == CP_UTF7);

    if (bForcedUTF8 || (!bRejectUTF8 && (encDetection.bIsUTF8Sig || bAnalysisUTF8)))
    {
      if (encDetection.bIsUTF8Sig) {
        EditSetNewText(hwnd, UTF8StringStart(lpData), cbData - 3, bClearUndoHistory);
        status->iEncoding = CPI_UTF8SIGN;
        EditDetectEOLMode(UTF8StringStart(lpData), cbData - 3, status);
      }
      else {
        EditSetNewText(hwnd, lpData, cbData, bClearUndoHistory);
        status->iEncoding = CPI_UTF8;
        EditDetectEOLMode(lpData, cbData, status);
      }
    }
    else if (!IS_ENC_ENFORCED() && (bIsCP_UTF7 || encDetection.bIs7BitASCII))
    {
      // load UTF-7/ASCII(7-bit) as ANSI/UTF-8
      EditSetNewText(hwnd, lpData, cbData, bClearUndoHistory);
      status->iEncoding = (Settings.LoadASCIIasUTF8 ? CPI_UTF8 : CPI_ANSI_DEFAULT);
      EditDetectEOLMode(lpData, cbData, status);
    }
    else { // ===  ALL OTHER NON UTF-8 ===

      status->iEncoding = encDetection.Encoding;
      UINT const uCodePage = Encoding_GetCodePage(encDetection.Encoding);

      if (Encoding_IsEXTERNAL_8BIT(status->iEncoding)) 
      {
        LPWSTR lpDataWide = AllocMem(cbData * 2 + 16, HEAP_ZERO_MEMORY);

        ptrdiff_t const cbDataWide = MultiByteToWideCharEx(uCodePage, 0, lpData, cbData, lpDataWide, (SizeOfMem(lpDataWide) / sizeof(WCHAR)));
        if (cbDataWide != 0)
        {
          FreeMem(lpData);
          lpData = AllocMem(cbDataWide * 3 + 16, HEAP_ZERO_MEMORY);

          cbData = WideCharToMultiByteEx(Encoding_SciCP, 0, lpDataWide, cbDataWide, lpData, SizeOfMem(lpData), NULL, NULL);
          if (cbData != 0) {
            EditSetNewText(hwnd, lpData, cbData, bClearUndoHistory);
            EditDetectEOLMode(lpData, cbData, status);
            FreeMem(lpDataWide);
          }
          else {
            Encoding_Forced(CPI_NONE);
            FreeMem(lpDataWide);
            FreeMem(lpData);
            return false;
          }
        }
        else {
          Encoding_Forced(CPI_NONE);
          FreeMem(lpDataWide);
          FreeMem(lpData);
          return false;
        }
      }
      else {
        EditSetNewText(hwnd, lpData, cbData, bClearUndoHistory);
        EditDetectEOLMode(lpData, cbData, status);
      }
    }
  }

  SciCall_SetCharacterCategoryOptimization(Encoding_IsCJK(encDetection.analyzedEncoding) ? 0x10000 : 0x1000);

  Encoding_Forced(CPI_NONE);

  FreeMem(lpData);

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
       bool bSaveCopy,
       bool bPreserveTimeStamp)
{
  bool bWriteSuccess = false;
  status->bCancelDataLoss = false;

  HANDLE hFile = CreateFile(pszFile,
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

  if (hFile == INVALID_HANDLE_VALUE) {
    return false;
  }

  //FILETIME createTime;
  //FILETIME laccessTime;
  FILETIME modTime;
  //if (!GetFileTime(hFile, &createTime, &laccessTime, &modTime)) {
  if (!GetFileTime(hFile, NULL, NULL, &modTime)) {
    return false;
  }

  // ensure consistent line endings
  if (Settings.FixLineEndings) {
    EditEnsureConsistentLineEndings(hwnd);
  }

  // strip trailing blanks
  if (Settings.FixTrailingBlanks) {
    EditStripLastCharacter(hwnd, true, true);
  }

  // get text length in bytes
  DocPos const cbData = SciCall_GetTextLength();
  size_t bytesWritten = 0ULL;

  // files larger than 2GB will be forced stored as ASCII/UTF-8
  if (cbData >= (DocPos)INT32_MAX) {
    Encoding_Current(CPI_UTF8);
    Encoding_Forced(CPI_UTF8);
    status->iEncoding = Encoding_Forced(CPI_GET);
  }

  if ((cbData <= 0) || (cbData >= DWORD_MAX)) {
    bWriteSuccess = SetEndOfFile(hFile) && (cbData < DWORD_MAX);
    Globals.dwLastError = GetLastError();
  }
  else {

    if (Encoding_IsUTF8(status->iEncoding))
    {
      const char* bom = NULL;
      DocPos bomoffset = 0;
      if (Encoding_IsUTF8_SIGN(status->iEncoding)) {
        bom = "\xEF\xBB\xBF";
        bomoffset = 3; // in char
      }

      SetEndOfFile(hFile);
     
      if (IsEncryptionRequired() && (cbData < ((DocPos)INT32_MAX - 1 - bomoffset)))
      {
        char* const lpData = AllocMem(cbData + 1 + bomoffset, HEAP_ZERO_MEMORY); //fix: +bom
        if (lpData) {
          if (bom) {
            CopyMemory(lpData, bom, bomoffset);
          }
          SciCall_GetText((cbData + 1), &lpData[bomoffset]);
          bWriteSuccess = EncryptAndWriteFile(hwnd, hFile, (BYTE*)lpData, (size_t)(cbData + bomoffset), &bytesWritten);
          Globals.dwLastError = GetLastError();
          FreeMem(lpData);
        }
        else { Globals.dwLastError = GetLastError(); }
      }
      else { // raw data handling of UTF-8 or >2GB file size
        DWORD dwBytesWritten = 0;
        if (bom) {
          WriteFile(hFile, bom, (DWORD)bomoffset, &dwBytesWritten, NULL);
        }
        bWriteSuccess = WriteFileXL(hFile, SciCall_GetCharacterPointer(), cbData, &bytesWritten);
        bytesWritten += (size_t)dwBytesWritten;
      }
    }

    else if (Encoding_IsUNICODE(status->iEncoding))  // UTF-16LE/BE_(BOM)
    {
      const char* bom = NULL;
      DocPos bomoffset = 0;
      if (Encoding_IsUNICODE_BOM(status->iEncoding)) {
        bom = "\xFF\xFE";
        bomoffset = 1; // in wide-char
      }

      SetEndOfFile(hFile);

      LPWSTR const lpDataWide = AllocMem((cbData+1+bomoffset) * 2, HEAP_ZERO_MEMORY);
      if (lpDataWide) {
        if (bom) {
          CopyMemory((char*)lpDataWide, bom, bomoffset * 2);
          bomoffset = 1;
        }
        ptrdiff_t const cbDataWide = bomoffset +
          MultiByteToWideCharEx(Encoding_SciCP, 0, SciCall_GetCharacterPointer(), cbData,
            &lpDataWide[bomoffset], ((SizeOfMem(lpDataWide) / sizeof(WCHAR)) - bomoffset));

        if (Encoding_IsUNICODE_REVERSE(status->iEncoding)) {
          SwabEx((char*)lpDataWide, (char*)lpDataWide, cbDataWide * sizeof(WCHAR));
        }
        bWriteSuccess = EncryptAndWriteFile(hwnd, hFile, (BYTE*)lpDataWide, cbDataWide * sizeof(WCHAR), &bytesWritten);
        Globals.dwLastError = GetLastError();
        FreeMem(lpDataWide);
      }
      else { Globals.dwLastError = GetLastError(); }
    }

    else if (Encoding_IsEXTERNAL_8BIT(status->iEncoding)) 
    {
      BOOL bCancelDataLoss = FALSE;
      UINT const uCodePage = Encoding_GetCodePage(status->iEncoding);
      bool const isUTF_7_or_8 = ((uCodePage == CPI_UTF7) || (uCodePage == CPI_UTF8));

      LPWSTR const lpDataWide = AllocMem((cbData+1) * 2, HEAP_ZERO_MEMORY);
      if (lpDataWide) {
        size_t const cbDataWide = (size_t)MultiByteToWideCharEx(Encoding_SciCP, 0, SciCall_GetCharacterPointer(), cbData,
                                                                lpDataWide, (SizeOfMem(lpDataWide) / sizeof(WCHAR))); 

        // dry conversion run
        size_t const cbSizeNeeded = (size_t)WideCharToMultiByteEx(uCodePage, 0, lpDataWide, cbDataWide, NULL, 0, NULL, NULL);
        size_t const cbDataNew = max(cbSizeNeeded, cbDataWide);

        char* const lpData = AllocMem(cbDataNew + 1, HEAP_ZERO_MEMORY);
        if (lpData) 
        {
          size_t cbDataConverted = 0ULL;
          if (Encoding_IsMBCS(status->iEncoding)) {
            cbDataConverted = (size_t)WideCharToMultiByteEx(uCodePage, 0, lpDataWide, cbDataWide,
                                                            lpData, SizeOfMem(lpData), NULL, NULL);
          }
          else {
            cbDataConverted = (size_t)WideCharToMultiByteEx(uCodePage, WC_NO_BEST_FIT_CHARS, lpDataWide, cbDataWide,
                                                            lpData, SizeOfMem(lpData), NULL, isUTF_7_or_8 ? NULL : &bCancelDataLoss);
          }

          FreeMem(lpDataWide);

          if (!bCancelDataLoss || InfoBoxLng(MB_OKCANCEL, L"MsgConv3", IDS_MUI_ERR_UNICODE2) == IDOK) {
            SetEndOfFile(hFile);
            if (cbDataConverted != 0) {
              bWriteSuccess = EncryptAndWriteFile(hwnd, hFile, (BYTE*)lpData, cbDataConverted, &bytesWritten);
              Globals.dwLastError = GetLastError();
            }
          }
          else {
            bWriteSuccess = false;
            status->bCancelDataLoss = true;
          }
          FreeMem(lpData);
        }
        else { Globals.dwLastError = GetLastError(); }
      }
      else { Globals.dwLastError = GetLastError(); }
    }

    else 
    {
      if (IsEncryptionRequired())
      {
        char* const lpData = AllocMem(cbData + 1, HEAP_ZERO_MEMORY);
        if (lpData) {
          SciCall_GetText((cbData + 1), lpData);
          SetEndOfFile(hFile);
          bWriteSuccess = EncryptAndWriteFile(hwnd, hFile, (BYTE*)lpData, (DWORD)cbData, &bytesWritten);
          Globals.dwLastError = GetLastError();
          FreeMem(lpData);
        }
        else { Globals.dwLastError = GetLastError(); }
      }
      else {
        SetEndOfFile(hFile);
        bWriteSuccess = WriteFileXL(hFile, SciCall_GetCharacterPointer(), cbData, &bytesWritten);
      }
    }
  }

  if (bPreserveTimeStamp) {
    SetFileTime(hFile, NULL, NULL, &modTime);
  }
  CloseHandle(hFile);

  if (bWriteSuccess && !bSaveCopy) {
    SetSavePoint();
  }
  return bWriteSuccess;
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
    if (Sci_IsMultiOrRectangleSelection()) {
      InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECTORMULTI);
      return;
    }

    const DocPos iSelStart = SciCall_GetSelectionStart();
    const DocPos iSelEnd = SciCall_GetSelectionEnd();
    const DocPos iSelSize = SciCall_GetSelText(NULL);

    LPWSTR pszTextW = AllocMem(iSelSize * sizeof(WCHAR), HEAP_ZERO_MEMORY);
    if (pszTextW) {

      size_t const cchTextW = EditGetSelectedText(pszTextW, iSelSize);

      bool bChanged = false;
      for (size_t i = 0; i < cchTextW; i++) {
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
        char* pszText = AllocMem(iSelSize, HEAP_ZERO_MEMORY);
        WideCharToMultiByteEx(Encoding_SciCP, 0, pszTextW, cchTextW, pszText, iSelSize, NULL, NULL);
        _BEGIN_UNDO_ACTION_;
        SciCall_Clear();
        SciCall_AddText((iSelEnd - iSelStart), pszText);
        SciCall_SetSel(iAnchorPos, iCurPos);
        _END_UNDO_ACTION_;
        FreeMem(pszText);
      }
      FreeMem(pszTextW);
    }
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
    if (Sci_IsMultiOrRectangleSelection()) {
      InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECTORMULTI);
      return;
    }
    const DocPos iSelStart = SciCall_GetSelectionStart();
    const DocPos iSelEnd = SciCall_GetSelectionEnd();
    const DocPos iSelSize = SciCall_GetSelText(NULL);

    LPWSTR pszTextW = AllocMem((iSelSize * sizeof(WCHAR)), HEAP_ZERO_MEMORY);

    if (pszTextW == NULL) {
      FreeMem(pszTextW);
      return;
    }

    size_t const cchTextW = EditGetSelectedText(pszTextW, iSelSize);

    bool bChanged = false;
    LPWSTR pszMappedW = AllocMem(SizeOfMem(pszTextW), HEAP_ZERO_MEMORY);
    if (pszMappedW) {
      // first make lower case, before applying TitleCase
      if (LCMapString(LOCALE_SYSTEM_DEFAULT, (LCMAP_LINGUISTIC_CASING | LCMAP_LOWERCASE), pszTextW, (int)cchTextW, pszMappedW, (int)iSelSize)) 
      {
        if (LCMapString(LOCALE_SYSTEM_DEFAULT, LCMAP_TITLECASE, pszMappedW, (int)cchTextW, pszTextW, (int)iSelSize)) {
          bChanged = true;
        }
      }
      FreeMem(pszMappedW);
    }

    if (bChanged) {
      char* pszText = AllocMem(iSelSize, HEAP_ZERO_MEMORY);
      WideCharToMultiByteEx(Encoding_SciCP, 0, pszTextW, cchTextW, pszText, iSelSize, NULL, NULL);
      _BEGIN_UNDO_ACTION_;
      SciCall_Clear();
      SciCall_AddText((iSelEnd - iSelStart), pszText);
      SciCall_SetSel(iAnchorPos, iCurPos);
      _END_UNDO_ACTION_;
      FreeMem(pszText);
    }
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
    if (Sci_IsMultiOrRectangleSelection()) {
      InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECTORMULTI);
      return;
    }
    const DocPos iSelStart = SciCall_GetSelectionStart();
    const DocPos iSelEnd = SciCall_GetSelectionEnd();
    const DocPos iSelSize = SciCall_GetSelText(NULL);

    LPWSTR pszTextW = AllocMem((iSelSize * sizeof(WCHAR)), HEAP_ZERO_MEMORY);

    if (pszTextW == NULL) {
      FreeMem(pszTextW);
      return;
    }

    size_t const cchTextW = EditGetSelectedText(pszTextW, iSelSize);

    bool bChanged = false;
    bool bNewSentence = true;
    for (size_t i = 0; i < cchTextW; i++) {
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
      char* pszText = AllocMem(iSelSize, HEAP_ZERO_MEMORY);
      WideCharToMultiByteEx(Encoding_SciCP, 0, pszTextW, cchTextW, pszText, iSelSize, NULL, NULL);
      _BEGIN_UNDO_ACTION_;
      SciCall_Clear();
      SciCall_AddText((iSelEnd - iSelStart), pszText);
      SciCall_SetSel(iAnchorPos, iCurPos);
      _END_UNDO_ACTION_;
      FreeMem(pszText);
    }
    FreeMem(pszTextW);
  }
}



//=============================================================================
//
//  EditURLEncode()
//
void EditURLEncode()
{
  if (SciCall_IsSelectionEmpty()) { return; }

  if (Sci_IsMultiOrRectangleSelection()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECTORMULTI);
    return;
  }

  DocPos const iCurPos = SciCall_GetCurrentPos();
  DocPos const iAnchorPos = SciCall_GetAnchor();
  DocPos const iSelSize = SciCall_GetSelText(NULL) - 1;

  const char* const pszText = (const char*)SciCall_GetRangePointer(min_p(iCurPos, iAnchorPos), iSelSize);

  WCHAR szTextW[INTERNET_MAX_URL_LENGTH+1];
  ptrdiff_t const cchTextW = MultiByteToWideChar(Encoding_SciCP, 0, pszText, (int)iSelSize, szTextW, INTERNET_MAX_URL_LENGTH);
  szTextW[cchTextW] = L'\0';
  StrTrim(szTextW, L" \r\n\t");

  size_t const cchEscaped = iSelSize * 3 + 1;
  char* pszEscaped = (char*)AllocMem(cchEscaped, HEAP_ZERO_MEMORY);
  if (pszEscaped == NULL) {
    return;
  }

  LPWSTR const pszEscapedW = (LPWSTR)AllocMem(cchEscaped * sizeof(WCHAR), HEAP_ZERO_MEMORY);
  if (pszEscapedW == NULL) {
    FreeMem(pszEscaped);
    return;
  }

  DWORD cchEscapedW = (DWORD)cchEscaped;

  UrlEscapeEx(szTextW, pszEscapedW, &cchEscapedW, true);

  ptrdiff_t const cchEscapedEnc = WideCharToMultiByte(Encoding_SciCP, 0, pszEscapedW, cchEscapedW,
                                                      pszEscaped, (int)cchEscaped, NULL, NULL);

  DocPos const saveTargetBeg = SciCall_GetTargetStart();
  DocPos const saveTargetEnd = SciCall_GetTargetEnd();

  _BEGIN_UNDO_ACTION_;

  if (iCurPos < iAnchorPos) {
    SciCall_SetTargetRange(iCurPos, iAnchorPos);
  }
  else {
    SciCall_SetTargetRange(iAnchorPos, iCurPos);
  }
  SciCall_ReplaceTarget(cchEscapedEnc, pszEscaped);
 
  SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore

  if (iCurPos < iAnchorPos)
    EditSetSelectionEx(iCurPos + cchEscapedEnc, iCurPos, -1, -1);
  else
    EditSetSelectionEx(iAnchorPos, iAnchorPos + cchEscapedEnc, -1, -1);

  _END_UNDO_ACTION_;

  FreeMem(pszEscaped);
  FreeMem(pszEscapedW);
}


//=============================================================================
//
//  EditURLDecode()
//
void EditURLDecode()
{
  if (SciCall_IsSelectionEmpty()) { return; }

  if (Sci_IsMultiOrRectangleSelection()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECTORMULTI);
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

  /*ptrdiff_t cchTextW =*/ MultiByteToWideCharEx(Encoding_SciCP, 0, pszText, (iSelSize-1), pszTextW, iSelSize);

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

  int const cchUnescapedDec = WideCharToMultiByte(Encoding_SciCP, 0, pszUnescapedW, cchUnescapedW,
                                                          pszUnescaped, (int)cchUnescaped, NULL, NULL);

  DocPos const saveTargetBeg = SciCall_GetTargetStart();
  DocPos const saveTargetEnd = SciCall_GetTargetEnd();

  _BEGIN_UNDO_ACTION_;

  if (iCurPos < iAnchorPos) {
    SciCall_SetTargetRange(iCurPos, iAnchorPos);
  }
  else {
    SciCall_SetTargetRange(iAnchorPos, iCurPos);
  }
  SciCall_ReplaceTarget(cchUnescapedDec, pszUnescaped);
  SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore

  if (iCurPos < iAnchorPos) {
    EditSetSelectionEx(iCurPos + cchUnescapedDec, iCurPos, -1, -1);
  }
  else {
    EditSetSelectionEx(iAnchorPos, iAnchorPos + cchUnescapedDec, -1, -1);
  }

  _END_UNDO_ACTION_;

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
    if (Sci_IsMultiOrRectangleSelection()) {
      InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECTORMULTI);
      return;
    }

    EDITFINDREPLACE efr = INIT_EFR_DATA;
    efr.hwnd = hwnd;

    _BEGIN_UNDO_ACTION_;

    StringCchCopyA(efr.szFind,COUNTOF(efr.szFind), "\\");
    StringCchCopyA(efr.szReplace, COUNTOF(efr.szReplace), "\\\\");
    EditReplaceAllInSelection(hwnd,&efr,false);

    StringCchCopyA(efr.szFind, COUNTOF(efr.szFind), "\"");
    StringCchCopyA(efr.szReplace, COUNTOF(efr.szReplace), "\\\"");
    EditReplaceAllInSelection(hwnd,&efr,false);

    StringCchCopyA(efr.szFind, COUNTOF(efr.szFind), "\'");
    StringCchCopyA(efr.szReplace, COUNTOF(efr.szReplace), "\\\'");
    EditReplaceAllInSelection(hwnd,&efr,false);
    
    _END_UNDO_ACTION_;
  }
}


//=============================================================================
//
//  EditUnescapeCChars()
//
void EditUnescapeCChars(HWND hwnd) {

  if (!SciCall_IsSelectionEmpty())
  {
    if (Sci_IsMultiOrRectangleSelection()) {
      InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECTORMULTI);
      return;
    }

    EDITFINDREPLACE efr = INIT_EFR_DATA;
    efr.hwnd = hwnd;

    _BEGIN_UNDO_ACTION_;

    StringCchCopyA(efr.szFind,FNDRPL_BUFFER,"\\\\");
    StringCchCopyA(efr.szReplace,FNDRPL_BUFFER,"\\");
    EditReplaceAllInSelection(hwnd,&efr,false);

    StringCchCopyA(efr.szFind,FNDRPL_BUFFER,"\\\"");
    StringCchCopyA(efr.szReplace,FNDRPL_BUFFER,"\"");
    EditReplaceAllInSelection(hwnd,&efr,false);

    StringCchCopyA(efr.szFind,FNDRPL_BUFFER,"\\\'");
    StringCchCopyA(efr.szReplace,FNDRPL_BUFFER,"\'");
    EditReplaceAllInSelection(hwnd,&efr,false);

    _END_UNDO_ACTION_;
  }
}


//=============================================================================
//
// EditChar2Hex()
//
void EditChar2Hex(HWND hwnd) 
{
  UNUSED(hwnd);

  if (Sci_IsMultiOrRectangleSelection()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECTORMULTI);
    return;
  }

  bool const bSelEmpty = SciCall_IsSelectionEmpty();

  DocPos const iAnchorPos = bSelEmpty ? SciCall_GetCurrentPos() : SciCall_GetAnchor();
  DocPos const iCurPos = bSelEmpty ? SciCall_PositionAfter(iAnchorPos) : SciCall_GetCurrentPos();
  if (iAnchorPos == iCurPos) { return; }

  if (bSelEmpty) { SciCall_SetSelection(iCurPos, iAnchorPos); }
  DocPos const count = Sci_GetSelTextLength();

  char const uesc = 'u';
  //???char const uesc = (LEXER == CSHARP) ? 'x' : 'u';  // '\xn[n][n][n]' - variable length version
  //switch (Style_GetCurrentLexerPtr()->lexerID)
  //{
  //  case SCLEX_CPP: 
  //    uesc = 'x';
  //  default:
  //    break;
  //}

  size_t const alloc = count * (2 + MAX_ESCAPE_HEX_DIGIT) + 1;
  char* ch = (char*)AllocMem(alloc, HEAP_ZERO_MEMORY);
  WCHAR* wch = (WCHAR*)AllocMem(alloc * sizeof(WCHAR), HEAP_ZERO_MEMORY);

  SciCall_GetSelText(ch);
  int const nchars = (DocPos)MultiByteToWideChar(Encoding_SciCP, 0, ch, -1, wch, (int)alloc) - 1; // '\0'
  memset(ch, 0, alloc);

  for (int i = 0, j = 0; i < nchars; ++i) 
  {
    if (wch[i] <= 0xFF) {
      StringCchPrintfA(&ch[j], (alloc - j), "\\x%02X", (wch[i] & 0xFF));  // \xhh
      j += 4;
    }
    else {
      StringCchPrintfA(ch + j, (alloc - j), "\\%c%04X", uesc, wch[i]);  // \uhhhh \xhhhh
      j += 6;
    }
  }

  _BEGIN_UNDO_ACTION_;
    
  SciCall_ReplaceSel(ch);

  DocPos const iReplLen = (DocPos)StringCchLenA(ch, alloc);

  if (!bSelEmpty) {
    if (iCurPos < iAnchorPos) {
      EditSetSelectionEx(iCurPos + iReplLen, iCurPos, -1, -1);
    }
    else if (iCurPos > iAnchorPos) {
      EditSetSelectionEx(iAnchorPos, iAnchorPos + iReplLen, -1, -1);
    }
    else { // empty selection
      EditSetSelectionEx(iCurPos + iReplLen, iCurPos + iReplLen, -1, -1);
    }
  }

  _END_UNDO_ACTION_;

  FreeMem(ch);
  FreeMem(wch);
}

//=============================================================================
//
// EditHex2Char()
//
void EditHex2Char(HWND hwnd) 
{
  UNUSED(hwnd);

  if (SciCall_IsSelectionEmpty()) {
    return;
  }
  if (Sci_IsMultiOrRectangleSelection()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECTORMULTI);
    return;
  }

  DocPos const iCurPos = SciCall_GetCurrentPos();
  DocPos const iAnchorPos = SciCall_GetAnchor();
  DocPos const count = Sci_GetSelTextLength();
  if (count <= 0) { return; }

  size_t const alloc = count * (2 + MAX_ESCAPE_HEX_DIGIT) + 1;
  char* ch = (char*)AllocMem(alloc, HEAP_ZERO_MEMORY);

  SciCall_GetSelText(ch);

  int const cch = Hex2Char(ch, (int)alloc);

  _BEGIN_UNDO_ACTION_;
  SciCall_ReplaceSel(ch);
  if (iCurPos < iAnchorPos) {
    EditSetSelectionEx(iCurPos + cch, iCurPos, -1, -1);
  }
  else {
    EditSetSelectionEx(iAnchorPos, iAnchorPos + cch, -1, -1);
  }
  _END_UNDO_ACTION_;

  FreeMem(ch);
}


//=============================================================================
//
//  EditFindMatchingBrace()
//
void EditFindMatchingBrace()
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
    Sci_GotoPosChooseCaret(iMatchingBracePos);
  }
}


//=============================================================================
//
//  EditSelectToMatchingBrace()
//
void EditSelectToMatchingBrace()
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
    _BEGIN_UNDO_ACTION_;
    if (bIsAfter) {
      EditSetSelectionEx(iCurPos, iMatchingBracePos, -1, -1);
    }
    else {
      EditSetSelectionEx(iCurPos, SciCall_PositionAfter(iMatchingBracePos), -1, -1);
  }
    _END_UNDO_ACTION_;
  }
}


//=============================================================================
//
//  EditModifyNumber()
//
void EditModifyNumber(HWND hwnd,bool bIncrease) {

  if (Sci_IsMultiOrRectangleSelection()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECTORMULTI);
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
        EditReplaceSelection(chNumber, false);
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
        EditReplaceSelection(chNumber, false);
      }
    }
  }
  UNUSED(hwnd);
}


//=============================================================================
//
//  _GetDateFormatProc() - date format information provided by the EnumDateFormatsExEx()
//
static unsigned int  _DateFmtIdx = 0;

static BOOL CALLBACK _GetDateFormatProc(LPWSTR lpDateFormatString, CALID CalendarID, LPARAM lParam)
{
  UNUSED(CalendarID);
  static unsigned int count = 0;

  LPWSTR const pwchFind = (LPWSTR)lParam;

  if (StrIsEmpty(pwchFind)) {
    count = 0; // begin
    StringCchCopy(pwchFind, SMALL_BUFFER, lpDateFormatString); // default
    if (count == _DateFmtIdx) { return FALSE; } // found
  }
  else if (count == _DateFmtIdx) {
    StringCchCopy(pwchFind, SMALL_BUFFER, lpDateFormatString);
    return FALSE; // found
  }
  ++count;
  return TRUE;
}



//=============================================================================
//
//  _GetCurrentDateTimeString()
//
static void _GetCurrentDateTimeString(LPWSTR pwchDateTimeStrg, size_t cchBufLen, bool bShortFmt)
{
  SYSTEMTIME st;
  GetLocalTime(&st);

  const WCHAR* const confFormat = bShortFmt ? Settings2.DateTimeFormat : Settings2.DateTimeLongFormat;

  if (StrIsNotEmpty(pwchDateTimeStrg) || StrIsNotEmpty(confFormat))
  {
    WCHAR wchTemplate[MIDSZ_BUFFER] = {L'\0'};
    StringCchCopyW(wchTemplate, COUNTOF(wchTemplate), StrIsNotEmpty(pwchDateTimeStrg) ? pwchDateTimeStrg : confFormat);

    struct tm sst;
    sst.tm_isdst = -1;
    sst.tm_sec = (int)st.wSecond;
    sst.tm_min = (int)st.wMinute;
    sst.tm_hour = (int)st.wHour;
    sst.tm_mday = (int)st.wDay;
    sst.tm_mon = (int)st.wMonth - 1;
    sst.tm_year = (int)st.wYear - 1900;
    sst.tm_wday = (int)st.wDayOfWeek;
    mktime(&sst);
    size_t const cnt = wcsftime(pwchDateTimeStrg, cchBufLen, wchTemplate, &sst);
    if (cnt == 0) {
      StringCchCopy(pwchDateTimeStrg, cchBufLen, wchTemplate);
    }
  }
  else // use configured DateTime Format
  {
    WCHAR wchFormat[SMALL_BUFFER] = { L'\0' };
    _DateFmtIdx = 0;
    EnumDateFormatsExEx(_GetDateFormatProc, Settings2.PreferredLanguageLocaleName, (bShortFmt ? DATE_SHORTDATE : DATE_LONGDATE), (LPARAM)wchFormat);

    WCHAR wchDate[SMALL_BUFFER] = { L'\0' };
    GetDateFormatEx(Settings2.PreferredLanguageLocaleName, DATE_AUTOLAYOUT, &st, wchFormat, wchDate, COUNTOF(wchDate), NULL);

    WCHAR wchTime[SMALL_BUFFER] = { L'\0' };
    GetTimeFormatEx(Settings2.PreferredLanguageLocaleName, TIME_NOSECONDS, &st, NULL, wchTime, COUNTOF(wchTime));

    StringCchPrintf(pwchDateTimeStrg, cchBufLen, L"%s %s", wchTime, wchDate);
  }
}

static void _GetCurrentTimeStamp(LPWSTR pwchDateTimeStrg, size_t cchBufLen, bool bShortFmt)
{
  if (StrIsEmpty(pwchDateTimeStrg)) {
    // '%s' is not allowd pattern of wcsftime(), so it must be string format
    PCWSTR p = StrStr(Settings2.TimeStampFormat, L"%s");
    if (p && !StrStr(p + 2, L"%s")) {
      WCHAR wchDateTime[SMALL_BUFFER] = {L'\0'};
      _GetCurrentDateTimeString(wchDateTime, COUNTOF(wchDateTime), bShortFmt);
      StringCchPrintfW(pwchDateTimeStrg, cchBufLen, Settings2.TimeStampFormat, wchDateTime);
      return;
    }
    // use configuration
    StringCchCopyW(pwchDateTimeStrg, cchBufLen, Settings2.TimeStampFormat);
  }
  _GetCurrentDateTimeString(pwchDateTimeStrg, cchBufLen, bShortFmt);
}


//=============================================================================
//
//  EditInsertDateTimeStrg()
//


void EditInsertDateTimeStrg(bool bShortFmt, bool bTimestampFmt)
{
  //~~~_BEGIN_UNDO_ACTION_;

  WCHAR wchDateTime[SMALL_BUFFER] = { L'\0' };
  char  chTimeStamp[MIDSZ_BUFFER] = {'\0'};

  if (bTimestampFmt) {
    _GetCurrentTimeStamp(wchDateTime, COUNTOF(wchDateTime), bShortFmt);
  }
  else {
    StringCchCopyW(wchDateTime, COUNTOF(wchDateTime), bShortFmt ? Settings2.DateTimeFormat : Settings2.DateTimeLongFormat);
    _GetCurrentDateTimeString(wchDateTime, COUNTOF(wchDateTime), bShortFmt);
  }
  WideCharToMultiByte(Encoding_SciCP, 0, wchDateTime, -1, chTimeStamp, COUNTOF(chTimeStamp), NULL, NULL);
  EditReplaceSelection(chTimeStamp, false);

  //~~~_END_UNDO_ACTION_;
}


//=============================================================================
//
//  EditUpdateTimestamps()
//
void EditUpdateTimestamps()
{
  WCHAR wchReplaceStrg[MIDSZ_BUFFER] = { L'\0' };
  _GetCurrentTimeStamp(wchReplaceStrg, COUNTOF(wchReplaceStrg), true); // DateTimeFormat

  EDITFINDREPLACE efrTS_L = INIT_EFR_DATA;
  efrTS_L.hwnd = Globals.hwndEdit;
  efrTS_L.fuFlags = (SCFIND_REGEXP | SCFIND_POSIX);
  WideCharToMultiByte(Encoding_SciCP, 0, Settings2.TimeStampRegEx, -1, efrTS_L.szFind, COUNTOF(efrTS_L.szFind), NULL, NULL);
  WideCharToMultiByte(Encoding_SciCP, 0, wchReplaceStrg, -1, efrTS_L.szReplace, COUNTOF(efrTS_L.szReplace), NULL, NULL);

  if (!SciCall_IsSelectionEmpty())
  {
    EditReplaceAllInSelection(Globals.hwndEdit, &efrTS_L, true);
  }
  else {
    EditReplaceAll(Globals.hwndEdit, &efrTS_L, true);
  }
}


//=============================================================================
//
//  EditTabsToSpaces()
//
void EditTabsToSpaces(int nTabWidth,bool bOnlyIndentingWS)
{
  if (SciCall_IsSelectionEmpty()) { return; } // no selection

  if (Sci_IsMultiOrRectangleSelection()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECTORMULTI);
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

  ptrdiff_t const cchTextW = MultiByteToWideCharEx(Encoding_SciCP,0,pszText,iSelCount,pszTextW,iSelCount+1);

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

    ptrdiff_t cchConvM = WideCharToMultiByteEx(Encoding_SciCP,0,pszConvW,cchConvW,
                                       pszText2,SizeOfMem(pszText2),NULL,NULL);

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
    _BEGIN_UNDO_ACTION_;
    SciCall_SetTargetRange(iSelStart, iSelEnd);
    SciCall_ReplaceTarget(cchConvM, pszText2);
    SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore
    EditSetSelectionEx(iAnchorPos, iCurPos, -1, -1);
    _END_UNDO_ACTION_;
    FreeMem(pszText2);
  }
  FreeMem(pszConvW);
}


//=============================================================================
//
//  EditSpacesToTabs()
//
void EditSpacesToTabs(int nTabWidth,bool bOnlyIndentingWS)
{
  if (SciCall_IsSelectionEmpty()) { return; } // no selection

  if (Sci_IsMultiOrRectangleSelection()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECTORMULTI);
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

  ptrdiff_t const cchTextW = MultiByteToWideCharEx(Encoding_SciCP,0,pszText,iSelCount,pszTextW,iSelCount+1);

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

    ptrdiff_t cchConvM = WideCharToMultiByteEx(Encoding_SciCP,0,pszConvW,cchConvW,
                                       pszText2,SizeOfMem(pszText2),NULL,NULL);

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
    _BEGIN_UNDO_ACTION_;
    SciCall_SetTargetRange(iSelStart, iSelEnd);
    SciCall_ReplaceTarget(cchConvM, pszText2);
    SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore
    EditSetSelectionEx(iAnchorPos, iCurPos, -1, -1);
    _END_UNDO_ACTION_;
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
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECTORMULTI);
    return;
  }

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

    _BEGIN_UNDO_ACTION_;

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

    _END_UNDO_ACTION_;
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
//  EditSetCaretToSelectionStart()
//
bool EditSetCaretToSelectionStart()
{
  DocPos const c = SciCall_GetSelectionNCaret(0) + SciCall_GetSelectionNCaretVirtualSpace(0);
  DocPos const s = SciCall_GetSelectionNStart(0) + SciCall_GetSelectionNStartVirtualSpace(0);
  bool const bSwap = (c != s);
  if (bSwap) {
    size_t const n = SciCall_GetSelections();
    for (size_t i = 0; i < n; ++i) {
      SciCall_SwapMainAnchorCaret();
      SciCall_RotateSelection();
    }
  }
  return bSwap;
}

//=============================================================================
//
//  EditSetCaretToSelectionEnd()
//
bool EditSetCaretToSelectionEnd()
{
  DocPos const c = SciCall_GetSelectionNCaret(0) + SciCall_GetSelectionNCaretVirtualSpace(0);
  DocPos const e = SciCall_GetSelectionNEnd(0) + SciCall_GetSelectionNEndVirtualSpace(0);
  bool const bSwap = (c != e);
  if (bSwap) {
    size_t const n = SciCall_GetSelections();
    for (size_t i = 0; i < n; ++i) {
      SciCall_SwapMainAnchorCaret();
      SciCall_RotateSelection();
    }
  }
  return bSwap;
}


//=============================================================================
//
//  EditModifyLines()
//
void EditModifyLines(LPCWSTR pwszPrefix, LPCWSTR pwszAppend)
{
  if (Sci_IsMultiOrRectangleSelection()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECTORMULTI);
    return;
  }

  char  mszPrefix1[256 * 3] = { '\0' };
  char  mszAppend1[256 * 3] = { '\0' };

  DocPos iSelStart = SciCall_GetSelectionStart();
  DocPos iSelEnd = SciCall_GetSelectionEnd();

  if (StrIsNotEmpty(pwszPrefix)) { WideCharToMultiByteEx(Encoding_SciCP, 0, pwszPrefix, -1, mszPrefix1, COUNTOF(mszPrefix1), NULL, NULL); }
  if (StrIsNotEmpty(pwszAppend)) { WideCharToMultiByteEx(Encoding_SciCP, 0, pwszAppend, -1, mszAppend1, COUNTOF(mszAppend1), NULL, NULL); }

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
  char  pszPrefixNumPad[2] = { '\0', '\0' };
  char  pszAppendNumPad[2] = { '\0', '\0' };
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
        pszPrefixNumPad[0] = '\0';
      }

      else if (StrCmpNA(p, "$(0I)", CSTRLEN("$(0I)")) == 0) {
        *p = 0;
        StringCchCopyA(mszPrefix2, COUNTOF(mszPrefix2), p + CSTRLEN("$(0I)"));
        bPrefixNum = true;
        iPrefixNum = 0;
        for (DocLn i = iLineEnd - iLineStart; i >= 10; i = i / 10)
          iPrefixNumWidth++;
        pszPrefixNumPad[0] = '0';
      }

      else if (StrCmpNA(p, "$(N)", CSTRLEN("$(N)")) == 0) {
        *p = 0;
        StringCchCopyA(mszPrefix2, COUNTOF(mszPrefix2), p + CSTRLEN("$(N)"));
        bPrefixNum = true;
        iPrefixNum = 1;
        for (DocLn i = iLineEnd - iLineStart + 1; i >= 10; i = i / 10)
          iPrefixNumWidth++;
        pszPrefixNumPad[0] = '\0';
      }

      else if (StrCmpNA(p, "$(0N)", CSTRLEN("$(0N)")) == 0) {
        *p = 0;
        StringCchCopyA(mszPrefix2, COUNTOF(mszPrefix2), p + CSTRLEN("$(0N)"));
        bPrefixNum = true;
        iPrefixNum = 1;
        for (DocLn i = iLineEnd - iLineStart + 1; i >= 10; i = i / 10)
          iPrefixNumWidth++;
        pszPrefixNumPad[0] = '0';
      }

      else if (StrCmpNA(p, "$(L)", CSTRLEN("$(L)")) == 0) {
        *p = 0;
        StringCchCopyA(mszPrefix2, COUNTOF(mszPrefix2), p + CSTRLEN("$(L)"));
        bPrefixNum = true;
        iPrefixNum = iLineStart + 1;
        for (DocLn i = iLineEnd + 1; i >= 10; i = i / 10)
          iPrefixNumWidth++;
        pszPrefixNumPad[0] = '\0';
      }

      else if (StrCmpNA(p, "$(0L)", CSTRLEN("$(0L)")) == 0) {
        *p = 0;
        StringCchCopyA(mszPrefix2, COUNTOF(mszPrefix2), p + CSTRLEN("$(0L)"));
        bPrefixNum = true;
        iPrefixNum = iLineStart + 1;
        for (DocLn i = iLineEnd + 1; i >= 10; i = i / 10)
          iPrefixNumWidth++;
        pszPrefixNumPad[0] = '0';
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
        pszAppendNumPad[0] = '\0';
      }

      else if (StrCmpNA(p, "$(0I)", CSTRLEN("$(0I)")) == 0) {
        *p = 0;
        StringCchCopyA(mszAppend2, COUNTOF(mszAppend2), p + CSTRLEN("$(0I)"));
        bAppendNum = true;
        iAppendNum = 0;
        for (DocLn i = iLineEnd - iLineStart; i >= 10; i = i / 10)
          iAppendNumWidth++;
        pszAppendNumPad[0] = '0';
      }

      else if (StrCmpNA(p, "$(N)", CSTRLEN("$(N)")) == 0) {
        *p = 0;
        StringCchCopyA(mszAppend2, COUNTOF(mszAppend2), p + CSTRLEN("$(N)"));
        bAppendNum = true;
        iAppendNum = 1;
        for (DocLn i = iLineEnd - iLineStart + 1; i >= 10; i = i / 10)
          iAppendNumWidth++;
        pszAppendNumPad[0] = '\0';
      }

      else if (StrCmpNA(p, "$(0N)", CSTRLEN("$(0N)")) == 0) {
        *p = 0;
        StringCchCopyA(mszAppend2, COUNTOF(mszAppend2), p + CSTRLEN("$(0N)"));
        bAppendNum = true;
        iAppendNum = 1;
        for (DocLn i = iLineEnd - iLineStart + 1; i >= 10; i = i / 10)
          iAppendNumWidth++;
        pszAppendNumPad[0] = '0';
      }

      else if (StrCmpNA(p, "$(L)", CSTRLEN("$(L)")) == 0) {
        *p = 0;
        StringCchCopyA(mszAppend2, COUNTOF(mszAppend2), p + CSTRLEN("$(L)"));
        bAppendNum = true;
        iAppendNum = iLineStart + 1;
        for (DocLn i = iLineEnd + 1; i >= 10; i = i / 10)
          iAppendNumWidth++;
        pszAppendNumPad[0] = '\0';
      }

      else if (StrCmpNA(p, "$(0L)", CSTRLEN("$(0L)")) == 0) {
        *p = 0;
        StringCchCopyA(mszAppend2, COUNTOF(mszAppend2), p + CSTRLEN("$(0L)"));
        bAppendNum = true;
        iAppendNum = iLineStart + 1;
        for (DocLn i = iLineEnd + 1; i >= 10; i = i / 10)
          iAppendNumWidth++;
        pszAppendNumPad[0] = '0';
      }
      p += CSTRLEN("$(");
      p = StrStrA(p, "$("); // next
    }
  }

  _BEGIN_UNDO_ACTION_;

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
    EditSetSelectionEx(iAnchorPos, iCurPos, -1, -1);
  }

  _END_UNDO_ACTION_;
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
  bool const bSingleLine = Sci_IsSelectionSingleLine();

  bool const _bTabIndents = SciCall_GetTabIndents();
  bool const _bBSpUnindents = SciCall_GetBackSpaceUnIndents();

  DocPos iDiffCurrent = 0;
  DocPos iDiffAnchor = 0;
  bool bFixStart = false;

  _BEGIN_UNDO_ACTION_;

  if (bSingleLine) {
    if (bFormatIndentation) {
      SciCall_VCHome();
      if (SciCall_PositionFromLine(iCurLine) == SciCall_GetCurrentPos()) {
        SciCall_VCHome();
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
    SciCall_Tab();
    if (bFormatIndentation) {
      SciCall_SetTabIndents(_bTabIndents);
    }
  }
  else  // SCI_BACKTAB
  {
    SciCall_SetBackSpaceUnIndents(bFormatIndentation ? true : _bBSpUnindents);
    SciCall_BackTab();
    if (bFormatIndentation) {
      SciCall_SetBackSpaceUnIndents(_bBSpUnindents);
    }
  }

  if (!bForceAll) {
    if (bSingleLine) {
      if (bFormatIndentation) {
        EditSetSelectionEx(SciCall_GetCurrentPos() + iDiffCurrent + (iAnchorPos - iCurPos), SciCall_GetCurrentPos() + iDiffCurrent, -1, -1);
      }
    }
    else {  // on multiline indentation, anchor and current positions are moved to line begin resp. end
      if (bFixStart) {
        if (iCurPos < iAnchorPos)
          iDiffCurrent = SciCall_LineLength(iCurLine) - Sci_GetEOLLen();
        else
          iDiffAnchor = SciCall_LineLength(iAnchorLine) - Sci_GetEOLLen();
      }
      EditSetSelectionEx(SciCall_GetLineEndPosition(iAnchorLine) - iDiffAnchor, SciCall_GetLineEndPosition(iCurLine) - iDiffCurrent, -1, -1);
    }
  }
  else {
    Sci_GotoPosChooseCaret(iInitialPos);
  }

  _END_UNDO_ACTION_;
}


//=============================================================================
//
//  EditAlignText()
//
void EditAlignText(int nMode)
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
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECTORMULTI);
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

  _BEGIN_UNDO_ACTION_;

  if (chNewLineBuf && wchLineBuf && wchNewLineBuf) {

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
        DocPos const cchLine = SciCall_LineLength(iLine);
        DocPos const cwch = (DocPos)MultiByteToWideCharEx(Encoding_SciCP, 0,
                                             SciCall_GetRangePointer(iStartPos, cchLine),
                                             cchLine, wchLineBuf, iBufCount);
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
              p = (WCHAR*)StrEnd(wchNewLineBuf, iBufCount);

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
                p = (WCHAR*)StrEnd(p, 0);
              }
            }
            else {
              StringCchCopy(wchNewLineBuf, iBufCount, pWords[0]);
              p = (WCHAR*)StrEnd(wchNewLineBuf, iBufCount);

              for (int i = 1; i < iWords; i++) {
                *p++ = L' ';
                *p = 0;
                StringCchCat(p, (iBufCount - StringCchLenW(wchNewLineBuf, iBufCount)), pWords[i]);
                p = (WCHAR*)StrEnd(p, 0);
              }
            }

            ptrdiff_t const cch = WideCharToMultiByteEx(Encoding_SciCP, 0, wchNewLineBuf, -1, chNewLineBuf, (int)iBufCount, NULL, NULL) - 1;

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
              p = (WCHAR*)StrEnd(p, 0);
            }

            ptrdiff_t const cch = WideCharToMultiByteEx(Encoding_SciCP, 0, wchNewLineBuf, -1,
                                                        chNewLineBuf, iBufCount, NULL, NULL) - 1;

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
  EditSetSelectionEx(iAnchorPos, iCurPos, -1, -1);

  _END_UNDO_ACTION_;
}



//=============================================================================
//
//  EditEncloseSelection()
//
void EditEncloseSelection(LPCWSTR pwszOpen, LPCWSTR pwszClose)
{
  if (Sci_IsMultiOrRectangleSelection()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECTORMULTI);
    return;
  }

  char  mszOpen[256 * 3] = { '\0' };
  char  mszClose[256 * 3] = { '\0' };

  const DocPos iCurPos = SciCall_GetCurrentPos();
  const DocPos iAnchorPos = SciCall_GetAnchor();
  const DocPos iSelStart = SciCall_GetSelectionStart();
  const DocPos iSelEnd = SciCall_GetSelectionEnd();

  if (StrIsNotEmpty(pwszOpen)) { WideCharToMultiByteEx(Encoding_SciCP, 0, pwszOpen, -1, mszOpen, COUNTOF(mszOpen), NULL, NULL); }
  if (StrIsNotEmpty(pwszClose)) { WideCharToMultiByteEx(Encoding_SciCP, 0, pwszClose, -1, mszClose, COUNTOF(mszClose), NULL, NULL); }

  DocPos const iLenOpen = (DocPos)StringCchLenA(mszOpen, COUNTOF(mszOpen));
  DocPos const iLenClose = (DocPos)StringCchLenA(mszClose, COUNTOF(mszClose));

  DocPos const saveTargetBeg = SciCall_GetTargetStart();
  DocPos const saveTargetEnd = SciCall_GetTargetEnd();

  _BEGIN_UNDO_ACTION_;

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
  EditSetSelectionEx(iAnchorPos + iLenOpen, iCurPos + iLenOpen, -1, -1);

  _END_UNDO_ACTION_;
}


//=============================================================================
//
//  EditToggleLineCommentsSimple()
//
void EditToggleLineCommentsSimple(HWND hwnd, LPCWSTR pwszComment, bool bInsertAtStart)
{
  UNUSED(hwnd);
  const DocPos iCurPos = SciCall_GetCurrentPos();
  const DocPos iAnchorPos = SciCall_GetAnchor();
  DocPos const iSelStart = SciCall_GetSelectionStart();
  DocPos const iSelEnd = SciCall_GetSelectionEnd();

  const DocPos iSelBegCol = SciCall_GetColumn(iSelStart);


  char mszPrefix[32 * 3] = { '\0' };
  char mszComment[96 * 3] = { '\0' };

  if (StrIsNotEmpty(pwszComment)) {
    char mszPostfix[64 * 3] = { '\0' };
    WideCharToMultiByteEx(Encoding_SciCP, 0, pwszComment, -1, mszPrefix, COUNTOF(mszPrefix), NULL, NULL);
    if (StrIsNotEmpty(Settings2.LineCommentPostfixStrg)) {
      WideCharToMultiByteEx(Encoding_SciCP, 0, Settings2.LineCommentPostfixStrg, -1, mszPostfix, COUNTOF(mszPostfix), NULL, NULL);
    }
    StringCchCopyA(mszComment, COUNTOF(mszComment), mszPrefix);
    StringCchCatA(mszComment, COUNTOF(mszComment), mszPostfix);
  }

  DocPos const cchPrefix = (DocPos)StringCchLenA(mszPrefix, COUNTOF(mszPrefix));
  DocPos const cchComment = (DocPos)StringCchLenA(mszComment, COUNTOF(mszComment));

  if (cchComment == 0) { return; }

  if (Sci_IsMultiOrRectangleSelection()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECTORMULTI);
    return;
  }

  const DocLn iLineStart = SciCall_LineFromPosition(iSelStart);
  DocLn iLineEnd = SciCall_LineFromPosition(iSelEnd);

  // don't consider (last) line where caret is before 1st column
  if (iSelEnd <= SciCall_PositionFromLine(iLineEnd)) {
    if ((iLineEnd - iLineStart) >= 1) { // except it is the only one
      --iLineEnd;
    }
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

  DocPos const saveTargetBeg = SciCall_GetTargetStart();
  DocPos const saveTargetEnd = SciCall_GetTargetEnd();

  int iAction = 0;

  _BEGIN_UNDO_ACTION_;

  for (DocLn iLine = iLineStart; iLine <= iLineEnd; ++iLine)
  {
    DocPos const iIndentPos = SciCall_GetLineIndentPosition(iLine);

    if (iIndentPos == SciCall_GetLineEndPosition(iLine)) {
      // don't set comment char on "empty" (white-space only) lines
      //~iAction = 1;
      continue;
    }

    const char* tchBuf = SciCall_GetRangePointer(iIndentPos, cchComment + 1);
    if (StrCmpNA(tchBuf, mszComment, (int)cchComment) == 0) 
    {
      // remove comment chars incl. Postfix
      DocPos const iSelPos = iIndentPos + cchComment;
      switch (iAction) {
        case 0:
          iAction = 2;
        case 2:
          SciCall_SetTargetRange(iIndentPos, iSelPos);
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
    else if (StrCmpNA(tchBuf, mszPrefix, (int)cchPrefix) == 0)
    {
      // remove pure comment chars
      DocPos const iSelPos = iIndentPos + cchPrefix;
      switch (iAction) {
        case 0:
          iAction = 2;
        case 2:
          SciCall_SetTargetRange(iIndentPos, iSelPos);
          SciCall_ReplaceTarget(0, "");
          iSelEndOffset -= cchPrefix;
          if (iLine == iLineStart) {
            iSelStartOffset = (iSelStart == SciCall_PositionFromLine(iLine)) ? 0 : (0 - cchPrefix);
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
          DocPos const iPos = SciCall_FindColumn(iLine, iCommentCol);
          SciCall_InsertText(iPos, mszComment);
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

  if (iCurPos < iAnchorPos) {
    EditSetSelectionEx(iAnchorPos + iSelEndOffset, iCurPos + iSelStartOffset, -1, -1);
  }
  else if (iCurPos > iAnchorPos) {
    EditSetSelectionEx(iAnchorPos + iSelStartOffset, iCurPos + iSelEndOffset, -1, -1);
  }
  else {
    EditSetSelectionEx(iAnchorPos + iSelStartOffset, iCurPos + iSelStartOffset, -1, -1);
  }

  _END_UNDO_ACTION_;
}


//=============================================================================
//
//  EditToggleLineCommentsExtended()
//
void EditToggleLineCommentsExtended(HWND hwnd, LPCWSTR pwszComment, bool bInsertAtStart)
{
  UNUSED(hwnd);
  DocPos const iSelStart = Sci_GetSelectionStartEx();
  DocPos const iSelEnd = Sci_GetSelectionEndEx();

  char mszComment[32 * 3] = { '\0' };

  if (StrIsNotEmpty(pwszComment)) {
    WideCharToMultiByteEx(Encoding_SciCP, 0, pwszComment, -1, mszComment, COUNTOF(mszComment), NULL, NULL);
  }
  DocPos const cchComment = (DocPos)StringCchLenA(mszComment, COUNTOF(mszComment));

  if (cchComment == 0) { return; }

  const DocLn iLineStart = SciCall_LineFromPosition(iSelStart);
  DocLn iLineEnd = SciCall_LineFromPosition(iSelEnd);

  if (!Sci_IsMultiOrRectangleSelection()) {
    // don't consider (last) line where caret is before 1st column
    if (iSelEnd <= SciCall_PositionFromLine(iLineEnd)) {
      if ((iLineEnd - iLineStart) >= 1) { // except it is the only one
        --iLineEnd;
      }
    }
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

  DocPos const saveTargetBeg = SciCall_GetTargetStart();
  DocPos const saveTargetEnd = SciCall_GetTargetEnd();

  int iAction = 0;

  UT_icd docpos_icd = { sizeof(DocPos), NULL, NULL, NULL };
  UT_array* sel_positions = NULL;
  utarray_new(sel_positions, &docpos_icd);
  utarray_reserve(sel_positions, (int)(iLineEnd - iLineStart + 1));

  _BEGIN_UNDO_ACTION_;

  for (DocLn iLine = iLineStart; iLine <= iLineEnd; ++iLine)
  {
    DocPos const iIndentPos = SciCall_GetLineIndentPosition(iLine);

    if (iIndentPos == SciCall_GetLineEndPosition(iLine)) {
      // don't set comment char on "empty" (white-space only) lines
      //~iAction = 1;
      continue;
    }

    const char* tchBuf = SciCall_GetRangePointer(iIndentPos, cchComment + 1);
    if (StrCmpNIA(tchBuf, mszComment, (int)cchComment) == 0) 
    {
      // remove comment chars
      DocPos const iSelPos = iIndentPos + cchComment;
      switch (iAction) {
        case 0:
          iAction = 2;
        case 2:
          SciCall_SetTargetRange(iIndentPos, iSelPos);
          SciCall_ReplaceTarget(0, "");
          utarray_push_back(sel_positions, &iIndentPos);
          break;
        case 1:
          utarray_push_back(sel_positions, &iSelPos);
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
          DocPos const iPos = SciCall_FindColumn(iLine, iCommentCol);
          SciCall_InsertText(iPos, mszComment);
          DocPos const iSelPos = iIndentPos + cchComment;
          utarray_push_back(sel_positions, &iSelPos);
        }
        break;
        case 2:
          break;
      }
    }
  }

  SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore

  // cppcheck-suppress nullPointerArithmetic
  DocPos* p = (DocPos*)utarray_next(sel_positions, NULL);
  if (p) { SciCall_SetSelection(*p, *p); }
  while (p) {
    p = (DocPos*)utarray_next(sel_positions, p);
    if (p) { SciCall_AddSelection(*p, *p); }
  }
  utarray_free(sel_positions);
  
  _END_UNDO_ACTION_;
}


//=============================================================================
//
//  EditToggleLineComments()
//
void EditToggleLineComments(HWND hwnd, LPCWSTR pwszComment, bool bInsertAtStart)
{
  if (Settings.EditLineCommentBlock) {
    EditToggleLineCommentsExtended(hwnd, pwszComment, bInsertAtStart);
  }
  else {
    EditToggleLineCommentsSimple(hwnd, pwszComment, bInsertAtStart);
  }
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

  _IGNORE_NOTIFY_CHANGE_;

  int const token = (!bNoUndoGroup ? BeginUndoAction() : -1);
  __try {

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
        EditSetSelectionEx(selAnchorMainPos + pVspAVec[0], selCaretMainPos + iSpcCount, 0, 0);
      else
        EditSetSelectionEx(selAnchorMainPos + pVspAVec[0], selCaretMainPos + pVspCVec[iLineCount - 1] + iSpcCount - pVspAVec[iLineCount - 1], 0, 0);
    }
    else {
      if (bSelLeft2Right)
        EditSetSelectionEx(selAnchorMainPos + pVspAVec[0] + iSpcCount - pVspCVec[0], selCaretMainPos + pVspCVec[iLineCount - 1], 0, 0);
      else
        EditSetSelectionEx(selAnchorMainPos + iSpcCount, selCaretMainPos + pVspCVec[iLineCount - 1], 0, 0);
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
    }
      if (iStartLine < iEndLine) {
    DocPos iMaxColumn = 0;
    for (DocLn iLine = iStartLine; iLine <= iEndLine; ++iLine) {
      iMaxColumn = max_p(iMaxColumn, SciCall_GetColumn(SciCall_GetLineEndPosition(iLine)));
    }
        if (iMaxColumn > 0) {
    const DocPos iSpcCount = _AppendSpaces(hwnd, iStartLine, iEndLine, iMaxColumn, bSkipEmpty);
          if (iCurPos < iAnchorPos) {
      EditSetSelectionEx(iAnchorPos + iSpcCount, iCurPos, -1, -1);
          }
          else {
      EditSetSelectionEx(iAnchorPos, iCurPos + iSpcCount, -1, -1);
  }
        }
      }
    }
  }
  __finally {
    if (token >= 0) { EndUndoAction(token); }
  }
  _OBSERVE_NOTIFY_CHANGE_;
}


//=============================================================================
//
//  EditStripFirstCharacter()
//
void EditStripFirstCharacter(HWND hwnd)
{
  UNUSED(hwnd);

  if (SciCall_IsSelectionEmpty()) { return; }

  DocPos const iSelStart = SciCall_IsSelectionEmpty() ? 0 : SciCall_GetSelectionStart();
  DocPos const iSelEnd = SciCall_IsSelectionEmpty() ? Sci_GetDocEndPosition() : SciCall_GetSelectionEnd();
  DocLn const iLineStart = SciCall_LineFromPosition(iSelStart);
  DocLn const iLineEnd = SciCall_LineFromPosition(iSelEnd);

  DocPos const saveTargetBeg = SciCall_GetTargetStart();
  DocPos const saveTargetEnd = SciCall_GetTargetEnd();

  _BEGIN_UNDO_ACTION_;

  if (SciCall_IsSelectionRectangle()) 
  {
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
            StringCchCopyNA(lineBuffer, SizeOfMem(lineBuffer), SciCall_GetRangePointer(nextPos, len + 1), len);
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
  else if (Sci_IsMultiSelection()) {
    EditSetCaretToSelectionStart();
    SciCall_CharLeft();   // -> thin multi-selection at begin
    SciCall_CharRight();  // -> no SCI_DEL, so use SCI_DELBACK:
    SciCall_DeleteBack();
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

  _END_UNDO_ACTION_;
}


//=============================================================================
//
//  EditStripLastCharacter()
//
void EditStripLastCharacter(HWND hwnd, bool bIgnoreSelection, bool bTrailingBlanksOnly)
{
  UNUSED(hwnd);

  if (SciCall_IsSelectionEmpty() && !(bIgnoreSelection || bTrailingBlanksOnly)) { return; }

  DocPos const iSelStart = (SciCall_IsSelectionEmpty() || bIgnoreSelection) ? 0 : SciCall_GetSelectionStart();
  DocPos const iSelEnd = (SciCall_IsSelectionEmpty() || bIgnoreSelection) ? Sci_GetDocEndPosition() : SciCall_GetSelectionEnd();
  DocLn const iLineStart = SciCall_LineFromPosition(iSelStart);
  DocLn const iLineEnd = SciCall_LineFromPosition(iSelEnd);

  DocPos const saveTargetBeg = SciCall_GetTargetStart();
  DocPos const saveTargetEnd = SciCall_GetTargetEnd();

  _BEGIN_UNDO_ACTION_;

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
              StringCchCopyNA(lineBuffer, SizeOfMem(lineBuffer), SciCall_GetRangePointer(selTargetStart, len + 1), len);
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

  _END_UNDO_ACTION_;
}


//=============================================================================
//
//  EditCompressBlanks()
//
void EditCompressBlanks()
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

    _BEGIN_UNDO_ACTION_;

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

    _END_UNDO_ACTION_;

  }
  else if (Sci_IsMultiSelection()) {
    // @@@ not implemented
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELMULTI);
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

        _BEGIN_UNDO_ACTION_;

        if (!SciCall_IsSelectionEmpty()) {
          SciCall_TargetFromSelection();
        }
        else {
          SciCall_TargetWholeDocument();
        }
        SciCall_ReplaceTarget(-1, pszOut);

        SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore

        DocPos const iNewLen = (DocPos)StringCchLenA(pszOut, SizeOfMem(pszOut));

        if (iCurPos < iAnchorPos) {
          EditSetSelectionEx(iCurPos + iNewLen, iCurPos, -1, -1);
        }
        else if (iCurPos > iAnchorPos) {
          EditSetSelectionEx(iAnchorPos, iAnchorPos + iNewLen, -1, -1);
        }
        else { // empty selection
          DocPos iNewPos = iCurPos;
          if (iCurPos > 0) {
            iNewPos = SciCall_PositionBefore(SciCall_PositionAfter(iCurPos - remWSuntilCaretPos));
          }
          EditSetSelectionEx(iNewPos, iNewPos, -1, -1);
        }

        _END_UNDO_ACTION_;
      }
    }
    FreeMem(pszOut);
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
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECTORMULTI);
    return;
  }

  const DocPos iSelStart = (SciCall_IsSelectionEmpty() ? 0 : SciCall_GetSelectionStart());
  const DocPos iSelEnd = (SciCall_IsSelectionEmpty() ? Sci_GetDocEndPosition() : SciCall_GetSelectionEnd());

  DocLn iBegLine = SciCall_LineFromPosition(iSelStart);
  DocLn iEndLine = SciCall_LineFromPosition(iSelEnd);

  if (iSelStart > SciCall_PositionFromLine(iBegLine)) { ++iBegLine; }
  if ((iSelEnd <= SciCall_PositionFromLine(iEndLine)) && (iEndLine != SciCall_GetLineCount() - 1)) { --iEndLine; }

  DocPos const saveTargetBeg = SciCall_GetTargetStart();
  DocPos const saveTargetEnd = SciCall_GetTargetEnd();

  _BEGIN_UNDO_ACTION_;

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

  _END_UNDO_ACTION_;

  SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore

}


//=============================================================================
//
//  EditRemoveDuplicateLines()
//
void EditRemoveDuplicateLines(HWND hwnd, bool bRemoveEmptyLines)
{
  UNUSED(hwnd);

  if (Sci_IsMultiOrRectangleSelection()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECTORMULTI);
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
  
  DocPos const saveTargetBeg = SciCall_GetTargetStart();
  DocPos const saveTargetEnd = SciCall_GetTargetEnd();

  _BEGIN_UNDO_ACTION_;

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

  _END_UNDO_ACTION_;
}


//=============================================================================
//
//  EditFocusMarkedLinesCmd()
//
void EditFocusMarkedLinesCmd(HWND hwnd, bool bCopy, bool bDelete)
{
    if (!(bCopy || bDelete)) { return; } // nothing todo

    DocLn const curLn = Sci_GetCurrentLineNumber();
    int const bitmask = SciCall_MarkerGet(curLn) & OCCURRENCE_MARKER_BITMASK();

    if (!bitmask) {
        return;
    }

    if (bCopy) {
        EditClearClipboard(hwnd);
    }

    _IGNORE_NOTIFY_CHANGE_;
    SciCall_BeginUndoAction();

    DocLn line = -1;
    do {
        line = SciCall_MarkerNext(line + 1, bitmask);
        if (line >= 0) {
            int const lnmask = SciCall_MarkerGet(line) & OCCURRENCE_MARKER_BITMASK();
            if (lnmask == bitmask) { // fit all markers
                if (bCopy) {
                    DocPos const lnBeg = SciCall_PositionFromLine(line);
                    //DocPos const lnEnd = lnBeg + SciCall_LineLength(line); // incl line-breaks
                    DocPos const lnEnd = SciCall_GetLineEndPosition(line); // w/o line-breaks
                    EditCopyRangeAppend(hwnd, lnBeg, lnEnd, true);
                }
                if (bDelete) {
                    SciCall_GotoLine(line);
                    SciCall_MarkerDelete(line, -1);
                    SciCall_LineDelete();
                    --line;
                }
            }
        }
    } while (line >= 0);

    SciCall_EndUndoAction();
    _OBSERVE_NOTIFY_CHANGE_;

    SciCall_GotoLine(min_ln(curLn, Sci_GetLastDocLineNumber()));
}


//=============================================================================
//
//  EditWrapToColumn()
//
void EditWrapToColumn(DocPosU nColumn)
{
  DocPosU const tabWidth = SciCall_GetTabWidth();
  nColumn = clamppu(nColumn, tabWidth, LONG_LINES_MARKER_LIMIT);

  DocPosU iCurPos = SciCall_GetCurrentPos();
  DocPosU iAnchorPos = SciCall_GetAnchor();

  DocPosU iSelStart = 0;
  DocPosU iSelEnd = Sci_GetDocEndPosition();
  DocPosU iSelCount = iSelEnd;

  if (!SciCall_IsSelectionEmpty()) {
    iSelStart = SciCall_GetSelectionStart();
    DocLn iLine = SciCall_LineFromPosition(iSelStart);
    iSelStart = SciCall_PositionFromLine(iLine);   // re-base selection to start of line
    iSelEnd = SciCall_GetSelectionEnd();
    iSelCount = (iSelEnd - iSelStart);
  }

  char* pszText = SciCall_GetRangePointer(iSelStart, iSelCount);

  LPWSTR pszTextW = AllocMem((iSelCount+1)*sizeof(WCHAR), HEAP_ZERO_MEMORY);
  if (pszTextW == NULL) {
    return;
  }

  DocPosU const cchTextW = (DocPosU)MultiByteToWideCharEx(Encoding_SciCP,0,pszText,iSelCount,
                                                          pszTextW,(SizeOfMem(pszTextW)/sizeof(WCHAR)));

  WCHAR wszEOL[3] = { L'\0' };
  int const cchEOL = Sci_GetCurrentEOL_W(wszEOL);

  DocPosU const convSize = (cchTextW * sizeof(WCHAR) * 3) + (cchEOL * (iSelCount/nColumn + 1)) + 1;
  LPWSTR pszConvW = AllocMem(convSize, HEAP_ZERO_MEMORY);
  if (pszConvW == NULL) {
    FreeMem(pszTextW);
    return;
  }

  // --------------------------------------------------------------------------
  //#define W_DELIMITER  L"!\"#$%&'()*+,-./:;<=>?@[\\]^`{|}~"  // underscore counted as part of word
  const WCHAR* const W_DELIMITER  = Settings.AccelWordNavigation ? W_DelimCharsAccel : W_DelimChars;
  #define ISDELIMITER(wc) (!(wc) || StrChrW(W_DELIMITER,(wc)))
  //#define ISWHITE(wc) StrChr(L" \t\f",wc)
  const WCHAR* const W_WHITESPACE = Settings.AccelWordNavigation ? W_WhiteSpaceCharsAccelerated : W_WhiteSpaceCharsDefault;
  #define ISWHITE(wc) (!(wc) || StrChrW(W_WHITESPACE,(wc)))
  #define ISLINEBREAK(wc) (!(wc) || ((wc) == wszEOL[0]) || ((wc) == wszEOL[1]))
  #define ISWORDCHAR(wc) (!ISWHITE(wc) && !ISLINEBREAK(wc) && !ISDELIMITER(wc))
  #define ISTAB(wc) ((wc) == L'\t')
  // --------------------------------------------------------------------------

  DocPos iCaretShift = 0;
  bool bModified = false;

  DocPosU  cchConvW = 0;
  DocPosU  iLineLength = 0;
    
  for (DocPosU iTextW = 0; iTextW < cchTextW; ++iTextW)
  {
    WCHAR w = pszTextW[iTextW];

    // read complete words
    while (ISWORDCHAR(w) && (iTextW < cchTextW)) {
      pszConvW[cchConvW++] = w;
      ++iLineLength;
      w = pszTextW[++iTextW];
    }

    // read delimiter until column limit
    while (!ISWORDCHAR(w) && (iTextW < cchTextW)) {
      if (ISLINEBREAK(w)) {
        if (w != L'\0') {
          pszConvW[cchConvW++] = w;
        }
        iLineLength = 0;
      }
      else if (iLineLength >= nColumn) {
        pszConvW[cchConvW++] = wszEOL[0];
        if (cchEOL > 1) {
          pszConvW[cchConvW++] = wszEOL[1];
        }
        if (cchConvW <= iCurPos) { iCaretShift += cchEOL; }
        bModified = true;
        pszConvW[cchConvW++] = w;
        iLineLength = ISTAB(w) ? tabWidth : 1;
      }
      else {
        pszConvW[cchConvW++] = w;
        iLineLength += ISTAB(w) ? tabWidth : 1;
      }
      w = pszTextW[++iTextW];
    }

    // does next word exceeds column limit ?
    DocPosU iNextWordLen = 1;
    DocPosU iNextW = iTextW;
    WCHAR w2 = pszTextW[iNextW];
    while (ISWORDCHAR(w2) && (iNextW < cchTextW)) {
      w2 = pszTextW[++iNextW];
      ++iNextWordLen;
    }
    if (w != L'\0') {
      if ((iLineLength + iNextWordLen) >= nColumn) {
        pszConvW[cchConvW++] = wszEOL[0];
        if (cchEOL > 1) {
          pszConvW[cchConvW++] = wszEOL[1];
        }
        if (cchConvW <= iCurPos) { iCaretShift += cchEOL; }
        iLineLength = 0;
        bModified = true;
      }
      pszConvW[cchConvW++] = w;
      iLineLength += ISTAB(w) ? tabWidth : 1;
    }

  }
  FreeMem(pszTextW);

  if (bModified) 
  {
    pszText = AllocMem(cchConvW * 3, HEAP_ZERO_MEMORY);
    if (pszText) 
    {
      DocPosU const cchConvM = WideCharToMultiByteEx(Encoding_SciCP, 0, pszConvW, cchConvW,
                                                     pszText, SizeOfMem(pszText), NULL, NULL);

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

      _BEGIN_UNDO_ACTION_;
      DocPos const saveTargetBeg = SciCall_GetTargetStart();
      DocPos const saveTargetEnd = SciCall_GetTargetEnd();
      SciCall_SetTargetRange(iSelStart, iSelEnd);
      SciCall_ReplaceTarget(cchConvM, pszText);
      SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore
      EditSetSelectionEx(iAnchorPos, iCurPos, -1, -1);
      _END_UNDO_ACTION_;
      FreeMem(pszText);
    }
  }
  FreeMem(pszConvW);
}


#if FALSE
//=============================================================================
//
//  EditWrapToColumnForce()
//
void EditWrapToColumnForce(HWND hwnd, DocPosU nColumn/*,int nTabWidth*/)
{
  UNUSED(hwnd);

  if (Sci_IsMultiOrRectangleSelection()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECTORMULTI);
    return;
  }

  size_t const size = (size_t)nColumn + 1LL;
  char const spc = ' ';
  char* const pTxt = (char* const)AllocMem(size + 1, HEAP_ZERO_MEMORY);
  memset(pTxt, spc, size);
  int const width_pix = SciCall_TextWidth(STYLE_DEFAULT, pTxt);
  FreeMem(pTxt);

  DocPos const saveTargetBeg = SciCall_GetTargetStart();
  DocPos const saveTargetEnd = SciCall_GetTargetEnd();

  _BEGIN_UNDO_ACTION_;
  if (SciCall_IsSelectionEmpty()) { SciCall_TargetWholeDocument(); } else { SciCall_TargetFromSelection(); }
  SciCall_LinesSplit(width_pix);
  SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore
  _END_UNDO_ACTION_;
}
#endif


//=============================================================================
//
//  EditSplitLines()
//
void EditSplitLines(HWND hwnd)
{
  UNUSED(hwnd);
  DocPos const saveTargetBeg = SciCall_GetTargetStart();
  DocPos const saveTargetEnd = SciCall_GetTargetEnd();

  _BEGIN_UNDO_ACTION_;
  SciCall_TargetFromSelection();
  SciCall_LinesSplit(0);
  SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore
  _END_UNDO_ACTION_;
}


//=============================================================================
//
//  EditJoinLinesEx()
//
//  Customized version of  SCI_LINESJOIN  (w/o using TARGET transaction)
//
void EditJoinLinesEx(bool bPreserveParagraphs, bool bCRLF2Space)
{
  bool bModified = false;

  if (SciCall_IsSelectionEmpty()) { return; }

  if (Sci_IsMultiOrRectangleSelection()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECTORMULTI);
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

  char szEOL[3] = { '\0' };
  int const cchEOL = Sci_GetCurrentEOL_A(szEOL);

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

    _BEGIN_UNDO_ACTION_;
    DocPos const saveTargetBeg = SciCall_GetTargetStart();
    DocPos const saveTargetEnd = SciCall_GetTargetEnd();
    SciCall_SetTargetRange(iSelStart, iSelEnd);
    SciCall_ReplaceTarget(cchJoin, pszJoin);
    SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore
    EditSetSelectionEx(iAnchorPos, iCurPos, -1, -1);
    _END_UNDO_ACTION_;
  }
  FreeMem(pszJoin);
}


//=============================================================================
//
//  EditSortLines()
//
typedef struct _SORTLINE {
  wchar_t* pwszLine;
  wchar_t* pwszSortEntry;
} SORTLINE;

typedef int (*FNSTRCMP)(const wchar_t*, const wchar_t*);
typedef int (*FNSTRLOGCMP)(const wchar_t*, const wchar_t*);

// ----------------------------------------------------------------------------

int CmpStd(const void *s1, const void *s2) {
  //~StrCmp()
  int const cmp =      wcscoll_s(((SORTLINE*)s1)->pwszSortEntry, ((SORTLINE*)s2)->pwszSortEntry);
  return (cmp) ? cmp : wcscoll_s(((SORTLINE*)s1)->pwszLine, ((SORTLINE*)s2)->pwszLine);
}

int CmpStdRev(const void* s1, const void* s2) { return -1 * CmpStd(s1, s2); }


int CmpStdI(const void* s1, const void* s2) {
  //~StrCmpI()
  int const cmp =      wcsicoll_s(((SORTLINE*)s1)->pwszSortEntry, ((SORTLINE*)s2)->pwszSortEntry);
  return (cmp) ? cmp : wcsicoll_s(((SORTLINE*)s1)->pwszLine, ((SORTLINE*)s2)->pwszLine);
}

int CmpStdIRev(const void* s1, const void* s2) { return -1 * CmpStdI(s1, s2); }

// ----------------------------------------------------------------------------

int CmpLexicographical(const void *s1, const void *s2) {
  int const cmp =      wcscmp_s(((SORTLINE*)s1)->pwszSortEntry, ((SORTLINE*)s2)->pwszSortEntry);
  return (cmp) ? cmp : wcscmp_s(((SORTLINE*)s1)->pwszLine, ((SORTLINE*)s2)->pwszLine);
  }

//int CmpLexicographicalI(const void* s1, const void* s2) {
//  int const cmp = _wcsicmp(((SORTLINE*)s1)->pwszSortEntry, ((SORTLINE*)s2)->pwszSortEntry);
//  return (cmp) ? cmp : _wcsicmp(((SORTLINE*)s1)->pwszLine, ((SORTLINE*)s2)->pwszLine);
//}

int CmpLexicographicalRev(const void* s1, const void* s2) { return -1 * CmpLexicographical(s1, s2); }

//int CmpLexicographicalIRev(const void* s1, const void* s2) { return -1 * CmpLexicographicalI(s1, s2); }

// non inlined for function pointer
static int _wcscmp_s(const wchar_t* s1, const wchar_t* s2) { return wcscmp_s(s1, s2); }
static int _wcscoll_s(const wchar_t* s1, const wchar_t* s2) { return wcscoll_s(s1, s2); }
static int _wcsicmp_s(const wchar_t* s1, const wchar_t* s2) { return wcsicmp_s(s1, s2); }
static int _wcsicoll_s(const wchar_t* s1, const wchar_t* s2) { return wcsicoll_s(s1, s2); }

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

int CmpStdLogical(const void *s1, const void *s2) {
  if (StrIsNotEmpty(s1) && StrIsNotEmpty(s2))
  {
    int cmp = StrCmpLogicalW(((SORTLINE*)s1)->pwszSortEntry, ((SORTLINE*)s2)->pwszSortEntry);
    if (cmp == 0) {
      cmp = StrCmpLogicalW(((SORTLINE*)s1)->pwszLine, ((SORTLINE*)s2)->pwszLine);
    }
    return (cmp) ? cmp : CmpStd(s1, s2);
  }
  else {
    return (StrIsNotEmpty(s1) ? 1 : (StrIsNotEmpty(s2) ? -1 : 0));
  }
}

int CmpStdLogicalRev(const void* s1, const void* s2) { return -1 * CmpStdLogical(s1, s2); }

// ----------------------------------------------------------------------------

void EditSortLines(HWND hwnd, int iSortFlags)
{
  if (SciCall_IsSelectionEmpty()) { return; } // no selection
  
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

  char mszEOL[3] = { '\0' };
  Sci_GetCurrentEOL_A(mszEOL);

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

  int ichlMax = 3;
  DocPos cchTotal = 0;
  DocLn iZeroLenLineCount = 0;
  for (DocLn i = 0, iLn = iLineStart; iLn <= iLineEnd; ++iLn, ++i) {

    int const cchm = (int)SciCall_LineLength(iLn);
    cchTotal += cchm;
    ichlMax = max_i(ichlMax, cchm);

    SciCall_GetLine_Safe(iLn, pmsz);

    if (iSortFlags & SORT_REMWSPACELN) {
      StrTrimA(pmsz, "\t\v \r\n"); // try clean line
      if (StrIsEmptyA(pmsz)) {
        // white-space only - remove
        continue;
      }
    }
    StrTrimA(pmsz, "\r\n"); // ignore line-breaks 

    int const cchw = MultiByteToWideChar(Encoding_SciCP, 0, pmsz, -1, NULL, 0);
    if (cchw > 1) {
      int tabs = _iTabWidth;
      ptrdiff_t const lnLen = (sizeof(WCHAR) * cchw);
      pLines[i].pwszLine = AllocMem(lnLen, HEAP_ZERO_MEMORY);
      MultiByteToWideChar(Encoding_SciCP, 0, pmsz, -1, pLines[i].pwszLine, cchw);
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

  if (iSortFlags & SORT_ASCENDING) {
    if (iSortFlags & SORT_NOCASE)
      qsort(pLines, iLineCount, sizeof(SORTLINE), CmpStdI);
    else if (iSortFlags & SORT_LOGICAL)
      qsort(pLines, iLineCount, sizeof(SORTLINE), CmpStdLogical);
    else if (iSortFlags & SORT_LEXICOGRAPH)
      qsort(pLines, iLineCount, sizeof(SORTLINE), CmpLexicographical);
    else
      qsort(pLines, iLineCount, sizeof(SORTLINE), CmpStd);
  }
  else if (iSortFlags & SORT_DESCENDING) {
    if (iSortFlags & SORT_NOCASE)
      qsort(pLines, iLineCount, sizeof(SORTLINE), CmpStdIRev);
    else if (iSortFlags & SORT_LOGICAL)
      qsort(pLines, iLineCount, sizeof(SORTLINE), CmpStdLogicalRev);
    else if (iSortFlags & SORT_LEXICOGRAPH)
      qsort(pLines, iLineCount, sizeof(SORTLINE), CmpLexicographicalRev);
    else
      qsort(pLines, iLineCount, sizeof(SORTLINE), CmpStdRev);
  }
  else /*if (iSortFlags & SORT_SHUFFLE)*/ {
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

  DocLn const lenRes = cchTotal + (2 * iLineCount) + 1;
  char* pmszResult = AllocMem(lenRes, HEAP_ZERO_MEMORY);
  char* pmszResOffset = pmszResult;
  char* pmszBuf = AllocMem(ichlMax + 1, HEAP_ZERO_MEMORY);

  FNSTRCMP const pFctStrCmp = (iSortFlags & SORT_NOCASE) ? ((iSortFlags & SORT_LEXICOGRAPH) ? _wcsicmp_s : _wcsicoll_s) :
                                                           ((iSortFlags & SORT_LEXICOGRAPH) ? _wcscmp_s  : _wcscoll_s);

  bool bLastDup = false;
  for (DocLn i = 0; i < iLineCount; ++i) {
    if (pLines[i].pwszLine && ((iSortFlags & SORT_SHUFFLE) || StrIsNotEmpty(pLines[i].pwszLine))) {
      bool bDropLine = false;
      if (!(iSortFlags & SORT_SHUFFLE)) {
        if (iSortFlags & SORT_MERGEDUP || iSortFlags & SORT_UNIQDUP || iSortFlags & SORT_UNIQUNIQ) {
          if (i < (iLineCount - 1)) {
            if (pFctStrCmp(pLines[i].pwszLine, pLines[i + 1].pwszLine) == 0) {
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
        WideCharToMultiByte(Encoding_SciCP, 0, pLines[i].pwszLine, -1, pmszBuf, (ichlMax + 1), NULL, NULL);
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
    FreeMem(pLines[i].pwszLine);
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
  
  _BEGIN_UNDO_ACTION_;

  //SciCall_SetTargetRange(SciCall_PositionFromLine(iLineStart), SciCall_PositionFromLine(iLineEnd + 1));
  SciCall_SetTargetRange(SciCall_PositionFromLine(iLineStart), SciCall_GetLineEndPosition(iLineEnd));
  SciCall_ReplaceTarget(-1, pmszResult);
  SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore
  FreeMem(pmszResult);

  if (bIsMultiSel) {
    EditSetSelectionEx(iAnchorPos, iCurPos, iAnchorPosVS, iCurPosVS);
  }
  else {
    EditSetSelectionEx(iAnchorPos, iCurPos, -1, -1);
  }

  _END_UNDO_ACTION_;
}


//=============================================================================
//
//  EditSetSelectionEx()
//
void EditSetSelectionEx(DocPos iAnchorPos, DocPos iCurrentPos, DocPos vSpcAnchor, DocPos vSpcCurrent)
{
  //~~~_BEGIN_UNDO_ACTION_;~~~

  if ((iAnchorPos < 0) && (iCurrentPos < 0)) {
    SciCall_SelectAll();
  }
  else {
    if (iAnchorPos < 0) {
      iAnchorPos = 0;
    }
    if (iCurrentPos < 0) {
      iCurrentPos = Sci_GetDocEndPosition();
    }

    DocLn const iCurrentLine = SciCall_LineFromPosition(iCurrentPos);
    DocLn const iAnchorLine = SciCall_LineFromPosition(iAnchorPos);

    // Ensure that the first and last lines of a selection are always unfolded
    // This needs to be done *before* the SCI_SETSEL message
    SciCall_EnsureVisible(iAnchorLine);
    if (iAnchorLine != iCurrentLine) { SciCall_EnsureVisible(iCurrentLine); }

    if ((vSpcAnchor >= 0) && (vSpcCurrent >= 0)) {
      SciCall_SetRectangularSelectionAnchor(iAnchorPos);
      if (vSpcAnchor > 0) {
        SciCall_SetRectangularSelectionAnchorVirtualSpace(vSpcAnchor);
      }
      SciCall_SetRectangularSelectionCaret(iCurrentPos);
      if (vSpcCurrent > 0) {
        SciCall_SetRectangularSelectionCaretVirtualSpace(vSpcCurrent);
      }
      SciCall_ScrollCaret();
    }
    else {
      SciCall_SetSel(iAnchorPos, iCurrentPos);  // scrolls into view
    }
    SciCall_ChooseCaretX();
  }
  
  //~~~_END_UNDO_ACTION_;~~~
}


//=============================================================================
//
//  EditEnsureConsistentLineEndings()
//
void EditEnsureConsistentLineEndings(HWND hwnd)
{
  SciCall_ConvertEOLs(SciCall_GetEOLMode());
  Globals.bDocHasInconsistentEOLs = false;
  EditFixPositions(hwnd);
}


//=============================================================================
//
//  EditEnsureSelectionVisible()
//
void EditEnsureSelectionVisible()
{
  // Ensure that the first and last lines of a selection are always unfolded
  DocLn const iCurrentLine = SciCall_LineFromPosition(SciCall_GetCurrentPos());
  DocLn const iAnchorLine = SciCall_LineFromPosition(SciCall_GetAnchor());
  if (iAnchorLine != iCurrentLine) { SciCall_EnsureVisible(iAnchorLine); } 
  SciCall_EnsureVisible(iCurrentLine);
  SciCall_ScrollCaret();
}


//=============================================================================
//
//  EditJumpTo()
//
void EditJumpTo(DocLn iNewLine, DocPos iNewCol)
{
  // Line maximum is iMaxLine - 1 (doc line count starts with 0)
  DocLn const iMaxLine = SciCall_GetLineCount() - 1;

  // jump to end with line set to -1
  if ((iNewLine < 0) || (iNewLine > iMaxLine)) {
    SciCall_DocumentEnd();
    return;
  }
  if (iNewLine == 0) { iNewLine = 1; }

  iNewLine = (min_ln(iNewLine, iMaxLine) - 1);
  DocPos const iLineEndPos = SciCall_GetLineEndPosition(iNewLine);
  
  // Column minimum is 1
  DocPos const colOffset = Globals.bZeroBasedColumnIndex ? 0 : 1;
  iNewCol = clampp((iNewCol - colOffset), 0, iLineEndPos);
  const DocPos iNewPos = SciCall_FindColumn(iNewLine, iNewCol);

  Sci_GotoPosChooseCaret(iNewPos);
}


//=============================================================================
//
//  EditFixPositions()
//
void EditFixPositions()
{
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
    MultiByteToWideCharEx(Encoding_SciCP,0,pszText,rlen,pszTextW,len);

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

  FreeMem(pszText);
  FreeMem(pszTextW);
}


//=============================================================================
//
//  _SetSearchFlags()
//
static void  _SetSearchFlags(HWND hwnd, LPEDITFINDREPLACE lpefr)
{
  if (lpefr) 
  {
    if (hwnd) 
    {
      char szBuf[FNDRPL_BUFFER] = { '\0' };
      bool bIsFindDlg = (GetDlgItem(Globals.hwndDlgFindReplace, IDC_REPLACE) == NULL);

      GetDlgItemTextW2MB(hwnd, IDC_FINDTEXT, szBuf, COUNTOF(szBuf));
      if (StringCchCompareXA(szBuf, lpefr->szFind) != 0) {
        StringCchCopyA(lpefr->szFind, COUNTOF(lpefr->szFind), szBuf);
        lpefr->bStateChanged = true;
      }

      GetDlgItemTextW2MB(hwnd, IDC_REPLACETEXT, szBuf, COUNTOF(szBuf));
      if (StringCchCompareXA(szBuf, lpefr->szReplace) != 0) {
        StringCchCopyA(lpefr->szReplace, COUNTOF(lpefr->szReplace), szBuf);
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

      bIsFlagSet = ((lpefr->fuFlags & SCFIND_REGEXP) != 0);
      if (IsButtonChecked(hwnd, IDC_FINDREGEXP)) {
        if (!bIsFlagSet) {
          lpefr->fuFlags |= SCFIND_REGEXP;
          lpefr->bStateChanged = true;
        }
      }
      else {
        if (bIsFlagSet) {
          lpefr->fuFlags &= ~SCFIND_REGEXP;
          lpefr->bStateChanged = true;
        }
      }

      bIsFlagSet = ((lpefr->fuFlags & SCFIND_DOT_MATCH_ALL) != 0);
      if (IsButtonChecked(hwnd, IDC_DOT_MATCH_ALL)) {
        if (!bIsFlagSet) {
          lpefr->fuFlags |= SCFIND_DOT_MATCH_ALL;
          lpefr->bStateChanged = true;
        }
      }
      else {
        if (bIsFlagSet) {
          lpefr->fuFlags &= ~SCFIND_DOT_MATCH_ALL;
          lpefr->bStateChanged = true;
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

      bIsFlagSet = lpefr->bOverlappingFind;
      if (IsButtonChecked(hwnd, IDC_FIND_OVERLAPPING)) {
        if (!bIsFlagSet) {
          lpefr->bOverlappingFind = true;
          lpefr->bStateChanged = false; // no effect on state
        }
      }
      else {
        if (bIsFlagSet) {
          lpefr->bOverlappingFind = false;
          lpefr->bStateChanged = false; // no effect on state
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
    } // if hwnd
  }
}


// Wildcard search uses the regexp engine to perform a simple search with * ? as wildcards 
// instead of more advanced and user-unfriendly regexp syntax
// for speed, we only need POSIX syntax here
static void  _EscapeWildcards(char* szFind2, size_t cch, LPCEDITFINDREPLACE lpefr)
{
  char *const szWildcardEscaped = (char *)AllocMem((cch<<1) + 1, HEAP_ZERO_MEMORY);
  if (szWildcardEscaped)
  {
    size_t iSource = 0;
    size_t iDest = 0;

    lpefr->fuFlags |= SCFIND_REGEXP;

    while ((iSource < cch) && (szFind2[iSource] != '\0'))
    {
      char c = szFind2[iSource];
      if (c == '*') {
        szWildcardEscaped[iDest++] = '.';
      } else if (c == '?') {
        c = '.';
      } else {
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
            c == '\\') {
          szWildcardEscaped[iDest++] = '\\';
        }
      }
      szWildcardEscaped[iDest++] = c;
      ++iSource;
    }

    StringCchCopyNA(szFind2, cch, szWildcardEscaped, SizeOfMem(szWildcardEscaped));

    FreeMem(szWildcardEscaped);
  }
}


//=============================================================================
//
//  _EditGetFindStrg()
//
static size_t _EditGetFindStrg(HWND hwnd, LPCEDITFINDREPLACE lpefr, LPSTR szFind, size_t cchCnt) {
  UNUSED(hwnd);
  if (!lpefr) { return 0; }
  if (!StrIsEmptyA(lpefr->szFind)) {
    StringCchCopyNA(szFind, cchCnt, lpefr->szFind, COUNTOF(lpefr->szFind));
  }
  else {
    CopyFindPatternMB(szFind, cchCnt);
    StringCchCopyNA(lpefr->szFind, COUNTOF(lpefr->szFind), szFind, cchCnt);
  }
  if (StrIsEmptyA(lpefr->szFind)) {
    return 0;
  }

  bool const bIsRegEx = (lpefr->fuFlags & SCFIND_REGEXP);
  if (lpefr->bTransformBS || bIsRegEx) {
    TransformBackslashes(szFind, bIsRegEx, Encoding_SciCP, NULL);
  }
  if (!StrIsEmptyA(szFind) && (lpefr->bWildcardSearch)) {
    _EscapeWildcards(szFind, cchCnt, lpefr);
  }

  return StringCchLenA(szFind, cchCnt);
}


//=============================================================================
//
//  _FindInTarget()
//
static DocPos  _FindInTarget(LPCSTR szFind, DocPos length, int sFlags, 
                             DocPos* start, DocPos* end, bool bForceNext, FR_UPD_MODES fMode)
{
  DocPos _start = *start;
  DocPos _end = *end;
  bool const bFindPrev = (_start > _end);
  DocPos iPos = 0;

  DocPos const saveTargetBeg = SciCall_GetTargetStart();
  DocPos const saveTargetEnd = SciCall_GetTargetEnd();

  SciCall_SetSearchFlags(sFlags);
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

static RegExResult_t _FindHasMatch(HWND hwnd, LPCEDITFINDREPLACE lpefr, DocPos iStartPos, bool bMarkAll, bool bFirstMatchOnly)
{
  char szFind[FNDRPL_BUFFER] = { '\0' };
  DocPos const slen = _EditGetFindStrg(hwnd, lpefr, szFind, COUNTOF(szFind));
  if (slen == 0) {
    return NO_MATCH;
  }
  int const sFlags = (int)(lpefr->fuFlags);

  DocPos const iStart = bFirstMatchOnly ? iStartPos : 0;
  DocPos const iTextEnd = Sci_GetDocEndPosition();

  DocPos start = iStart;
  DocPos end = iTextEnd;
  DocPos const iPos = _FindInTarget(szFind, slen, sFlags, &start, &end, false, FRMOD_IGNORE);

  if (bFirstMatchOnly && !Globals.bReplaceInitialized) {
    if (IsWindow(Globals.hwndDlgFindReplace) && (GetForegroundWindow() == Globals.hwndDlgFindReplace)) {
      if (iPos >= 0) {
        SciCall_SetSel(start, end);
      } else {
        SciCall_ScrollCaret();
      }
    }
  } else // mark all matches
  {
    if (bMarkAll) {
      EditClearAllOccurrenceMarkers(hwnd);
      if (iPos >= 0) {
        EditMarkAll(szFind, (int)(lpefr->fuFlags), 0, iTextEnd, false);
        if (FocusedView.HideNonMatchedLines) {
          EditFoldMarkedLineRange(lpefr->hwnd, true);
        }
      } else {
        if (FocusedView.HideNonMatchedLines) {
          EditFoldMarkedLineRange(lpefr->hwnd, false);
        }
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

static bool s_SaveMarkOccurrences = false;
static bool s_SaveMarkMatchVisible = false;
static bool s_SaveTFBackSlashes = false;

//=============================================================================
//
//  EditBoxForPasteFixes()
//
static LRESULT CALLBACK EditBoxForPasteFixes(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                                             UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
  LPEDITFINDREPLACE pefrData = (LPEDITFINDREPLACE)dwRefData;

  if (pefrData)
  {
    switch (uMsg)
    {
      case WM_PASTE:
        {
          WCHAR wchBuf[FNDRPL_BUFFER] = { L'\0' }; // tmp working buffer
          EditGetClipboardW(wchBuf, COUNTOF(wchBuf));
          SendMessage(hwnd, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)wchBuf);
        }
        return TRUE;

      //case WM_LBUTTONDOWN:
      //  SendMessage(hwnd, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)L"X");
      //  return TRUE;

      case WM_NCDESTROY:
        RemoveWindowSubclass(hwnd, EditBoxForPasteFixes, uIdSubclass);
        break;

      default:
        break;
    }
  }
  return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}


//=============================================================================
//
//  EditFindReplaceDlgProc()
//
extern int    s_flagMatchText;

static INT_PTR CALLBACK EditFindReplaceDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  static WCHAR s_tchBuf[FNDRPL_BUFFER] = { L'\0' }; // tmp working buffer
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
      // clear cmd line stuff
      s_flagMatchText = 0;
      sg_pefrData = NULL;

      // the global static Find/Replace data structure
      SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
      SetDialogIconNP3(hwnd);

      Globals.hwndDlgFindReplace = hwnd;

      SetTimer(hwnd, IDT_TIMER_MRKALL, USER_TIMER_MINIMUM, MQ_ExecuteNext);

      s_InitialSearchStart = SciCall_GetSelectionStart();
      s_InitialAnchorPos = SciCall_GetAnchor();
      s_InitialCaretPos = SciCall_GetCurrentPos();
      s_InitialTopLine = SciCall_GetFirstVisibleLine();

      EditSetCaretToSelectionStart(); // avoid search text selection jumps to next match (before ResizeDlg_InitX())

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
      
      COMBOBOXINFO cbInfoF = { sizeof(COMBOBOXINFO) };
      GetComboBoxInfo(GetDlgItem(hwnd, IDC_FINDTEXT), &cbInfoF);
      if (cbInfoF.hwndItem) {
        SetWindowSubclass(cbInfoF.hwndItem, EditBoxForPasteFixes, 0, (DWORD_PTR)sg_pefrData);
        SHAutoComplete(cbInfoF.hwndItem, SHACF_FILESYS_ONLY | SHACF_AUTOAPPEND_FORCE_OFF | SHACF_AUTOSUGGEST_FORCE_OFF);
      }

      if (!GetWindowTextLengthW(GetDlgItem(hwnd, IDC_FINDTEXT))) {
        if (!StrIsEmptyA(sg_pefrData->szFind)) {
          SetDlgItemTextMB2W(hwnd, IDC_FINDTEXT, sg_pefrData->szFind);
        }
      }

      if (GetDlgItem(hwnd, IDC_REPLACETEXT))
      {
        SendDlgItemMessage(hwnd, IDC_REPLACETEXT, CB_LIMITTEXT, FNDRPL_BUFFER, 0);
        SendDlgItemMessage(hwnd, IDC_REPLACETEXT, CB_SETEXTENDEDUI, true, 0);

        COMBOBOXINFO cbInfoR = { sizeof(COMBOBOXINFO) };
        GetComboBoxInfo(GetDlgItem(hwnd, IDC_REPLACETEXT), &cbInfoR);
        if (cbInfoR.hwndItem) {
          SetWindowSubclass(cbInfoR.hwndItem, EditBoxForPasteFixes, 0, 0);
          SHAutoComplete(cbInfoR.hwndItem, SHACF_FILESYS_ONLY | SHACF_AUTOAPPEND_FORCE_OFF | SHACF_AUTOSUGGEST_FORCE_OFF);
        }

        if (!StrIsEmptyA(sg_pefrData->szReplace)) {
          SetDlgItemTextMB2W(hwnd, IDC_REPLACETEXT, sg_pefrData->szReplace);
        }
      }

      bool const bRegEx = (sg_pefrData->fuFlags & SCFIND_REGEXP) != 0;
      bool const bDotMatchAll = (sg_pefrData->fuFlags & SCFIND_DOT_MATCH_ALL) != 0;
      s_SaveTFBackSlashes = sg_pefrData->bTransformBS;

      if (sg_pefrData->bTransformBS || bRegEx) {
        CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, BST_CHECKED);
      }
      if (bDotMatchAll) {
        CheckDlgButton(hwnd, IDC_DOT_MATCH_ALL, BST_CHECKED);
      }
      if (bRegEx) {
        CheckDlgButton(hwnd, IDC_FINDREGEXP, BST_CHECKED);
        sg_pefrData->bWildcardSearch = false;
        DialogEnableControl(hwnd, IDC_FINDTRANSFORMBS, false);
      }
      else {
        DialogEnableControl(hwnd, IDC_DOT_MATCH_ALL, false);
      }

      if (sg_pefrData->bWildcardSearch) {
        CheckDlgButton(hwnd, IDC_WILDCARDSEARCH, BST_CHECKED);
        CheckDlgButton(hwnd, IDC_FINDREGEXP, BST_UNCHECKED);
        DialogEnableControl(hwnd, IDC_DOT_MATCH_ALL, false);
        // transform BS handled by regex (wildcard search based on):
        CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, BST_CHECKED);
        DialogEnableControl(hwnd, IDC_FINDTRANSFORMBS, false);
      }

      CheckDlgButton(hwnd, IDC_FIND_OVERLAPPING, sg_pefrData->bOverlappingFind ? BST_CHECKED : BST_UNCHECKED);

      if (sg_pefrData->bMarkOccurences) {
        CheckDlgButton(hwnd, IDC_ALL_OCCURRENCES, BST_CHECKED);
      } else {
        EditClearAllOccurrenceMarkers(sg_pefrData->hwnd);
        Globals.iMarkOccurrencesCount = 0;
      }

      if (sg_pefrData->fuFlags & SCFIND_MATCHCASE) {
        CheckDlgButton(hwnd, IDC_FINDCASE, BST_CHECKED);
      }
      if (sg_pefrData->fuFlags & SCFIND_WHOLEWORD) {
        CheckDlgButton(hwnd, IDC_FINDWORD, BST_CHECKED);
      }
      if (sg_pefrData->fuFlags & SCFIND_WORDSTART) {
        CheckDlgButton(hwnd, IDC_FINDSTART, BST_CHECKED);
      }
      if (sg_pefrData->bNoFindWrap) {
        CheckDlgButton(hwnd, IDC_NOWRAP, BST_CHECKED);
      }

      if (GetDlgItem(hwnd, IDC_REPLACE)) {
        if (s_bSwitchedFindReplace) {
          if (sg_pefrData->bFindClose) {
            CheckDlgButton(hwnd, IDC_FINDCLOSE, BST_CHECKED);
          }
        }
        else {
          if (sg_pefrData->bReplaceClose) {
            CheckDlgButton(hwnd, IDC_FINDCLOSE, BST_CHECKED);
          }
        }
      }
      else {
        if (s_bSwitchedFindReplace) {
          if (sg_pefrData->bReplaceClose) {
            CheckDlgButton(hwnd, IDC_FINDCLOSE, BST_CHECKED);
          }
        }
        else {
          if (sg_pefrData->bFindClose) {
            CheckDlgButton(hwnd, IDC_FINDCLOSE, BST_CHECKED);
          }
        }
      }

      CheckDlgButton(hwnd, IDC_TRANSPARENT, SetBtn(Settings.FindReplaceTransparentMode));

      if (!s_bSwitchedFindReplace) {
        if (Settings.FindReplaceDlgPosX == CW_USEDEFAULT || Settings.FindReplaceDlgPosY == CW_USEDEFAULT)
          CenterDlgInParent(hwnd, NULL);
        else
          SetDlgPos(hwnd, Settings.FindReplaceDlgPosX, Settings.FindReplaceDlgPosY);
      }
      else {
        SetDlgPos(hwnd, s_xFindReplaceDlgSave, s_yFindReplaceDlgSave);
        s_bSwitchedFindReplace = false;
        CopyMemory(sg_pefrData, &s_efrSave, sizeof(EDITFINDREPLACE));
      }

      WCHAR wchMenuBuf[80] = {L'\0'};
      HMENU hmenu = GetSystemMenu(hwnd, false);

      GetLngString(IDS_MUI_SAVEPOS, wchMenuBuf, COUNTOF(wchMenuBuf));
      InsertMenu(hmenu, 0, MF_BYPOSITION | MF_STRING | MF_ENABLED, IDS_MUI_SAVEPOS, wchMenuBuf);
      GetLngString(IDS_MUI_RESETPOS, wchMenuBuf, COUNTOF(wchMenuBuf));
      InsertMenu(hmenu, 1, MF_BYPOSITION | MF_STRING | MF_ENABLED, IDS_MUI_RESETPOS, wchMenuBuf);
      InsertMenu(hmenu, 2, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
      GetLngString(IDS_MUI_CLEAR_FIND_HISTORY, wchMenuBuf, COUNTOF(wchMenuBuf));
      InsertMenu(hmenu, 3, MF_BYPOSITION | MF_STRING | MF_ENABLED, IDS_MUI_CLEAR_FIND_HISTORY, wchMenuBuf);
      GetLngString(IDS_MUI_CLEAR_REPL_HISTORY, wchMenuBuf, COUNTOF(wchMenuBuf));
      InsertMenu(hmenu, 4, MF_BYPOSITION | MF_STRING | MF_ENABLED, IDS_MUI_CLEAR_REPL_HISTORY, wchMenuBuf);
      InsertMenu(hmenu, 5, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);


      hBrushRed = CreateSolidBrush(rgbRedColorRef);
      hBrushGreen = CreateSolidBrush(rgbGreenColorRef);
      hBrushBlue = CreateSolidBrush(rgbBlueColorRef);
      
      s_fwrdMatch = s_anyMatch = NO_MATCH;

      _SetSearchFlags(hwnd, sg_pefrData); // sync
      sg_pefrData->bStateChanged = true;  // force update

      DialogEnableControl(hwnd, IDC_TOGGLE_VISIBILITY, sg_pefrData->bMarkOccurences);
    }
    return !0; // (!) further processing

  case WM_ENABLE:
    // modal child dialog should disable main window too
    EnableWindow(Globals.hwndMain, (BOOL)wParam);
    return !0;

  case WM_DESTROY:
    {
      _SetSearchFlags(hwnd, sg_pefrData); // sync
      Settings.EFR_Data = *sg_pefrData;   // remember options

      if (!s_bSwitchedFindReplace)
      {
        if (s_anyMatch == MATCH) {
          // Save MRUs
          if (!StrIsEmptyA(sg_pefrData->szFind)) {
            if (GetDlgItemText(hwnd, IDC_FINDTEXT, s_tchBuf, COUNTOF(s_tchBuf))) {
              MRU_Add(Globals.pMRUfind, s_tchBuf, 0, -1, -1, NULL);
              SetFindPattern(s_tchBuf);
            }
          }
        }

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
          Globals.iMarkOccurrencesCount = 0;
        }

        if (s_InitialTopLine >= 0) {
          SciCall_SetFirstVisibleLine(s_InitialTopLine);
          s_InitialTopLine = -1;  // reset
        }
        else {
          if (s_fwrdMatch == NO_MATCH) {
            EditSetSelectionEx(s_InitialAnchorPos, s_InitialCaretPos, -1, -1);
          }
          else {
            EditEnsureSelectionVisible();
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
      sg_pefrData = NULL;
      Globals.hwndDlgFindReplace = NULL;
    }
    return 0;


    case WM_DPICHANGED:
      {
        DPI_T dpi;
        dpi.x = LOWORD(wParam);
        dpi.y = HIWORD(wParam);
        UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, &dpi);
      }
      return !0; // further processing


  case WM_ACTIVATE:
    {
      if (!sg_pefrData) { return false; }

      switch (LOWORD(wParam))
      {
        case WA_INACTIVE:
          SetWindowTransparentMode(hwnd, Settings.FindReplaceTransparentMode, Settings2.FindReplaceOpacityLevel);
          break;

        case WA_CLICKACTIVE:
          // mouse click activation
        case WA_ACTIVE:
          SetWindowTransparentMode(hwnd, false, 100);

          // selection changed ?
          if (s_InitialTopLine < 0) {
            s_InitialAnchorPos = SciCall_GetAnchor();
            s_InitialCaretPos = SciCall_GetCurrentPos();
            s_InitialTopLine = SciCall_GetFirstVisibleLine();
            s_InitialSearchStart = s_InitialCaretPos;
            s_fwrdMatch = NO_MATCH;
          }

          if (!SciCall_IsSelectionEmpty()) {
            EditEnsureSelectionVisible();
          }

          bool const bEnableReplInSel = !(SciCall_IsSelectionEmpty() || Sci_IsMultiOrRectangleSelection());
          DialogEnableControl(hwnd, IDC_REPLACEINSEL, bEnableReplInSel);

          _DelayMarkAll(hwnd, 50, s_InitialSearchStart);

          break;

        default:
          break;
      }
    }
    return false;


    case WM_COMMAND:
    {
      if (!sg_pefrData) { return false; }

      switch (LOWORD(wParam))
      {

      case IDC_DOC_MODIFIED:
        s_InitialSearchStart = SciCall_GetSelectionStart();
        s_InitialTopLine = -1;  // reset
        sg_pefrData->bStateChanged = true;
        break;

      case IDC_FINDTEXT:
      case IDC_REPLACETEXT:
      {
        bool bEditChange = (HIWORD(wParam) == CBN_EDITCHANGE);

        if (Globals.bFindReplCopySelOrClip)
        {
          char* lpszSelection = NULL;
          s_tchBuf[0] = L'\0';

          DocPos const cchSelection = SciCall_GetSelText(NULL);
          if ((1 < cchSelection) && (LOWORD(wParam) != IDC_REPLACETEXT)) {
            lpszSelection = AllocMem(cchSelection + 1, HEAP_ZERO_MEMORY);
            SciCall_GetSelText(lpszSelection);
          }
          else { // (cchSelection <= 1)
            // nothing is selected in the editor:
            // if first time you bring up find/replace dialog,
            // use most recent search pattern to find box
            GetFindPattern(s_tchBuf, COUNTOF(s_tchBuf));
            if (s_tchBuf[0] == L'\0') {
              MRU_Enum(Globals.pMRUfind, 0, s_tchBuf, COUNTOF(s_tchBuf));
            }
            // no recent find pattern: copy content clipboard to find box
            if (s_tchBuf[0] == L'\0') {
              char *const pClip = EditGetClipboardText(Globals.hwndEdit, false, NULL, NULL);
              if (pClip) {
                size_t const len = StringCchLenA(pClip, 0);
                if (len) {
                  lpszSelection = AllocMem(len + 1, HEAP_ZERO_MEMORY);
                  StringCchCopyA(lpszSelection, SizeOfMem(lpszSelection), pClip);
                }
                FreeMem(pClip);
              }
            }
          }

          if (lpszSelection) {
            SetDlgItemTextMB2W(hwnd, IDC_FINDTEXT, lpszSelection);
            FreeMem(lpszSelection);
            lpszSelection = NULL;
            bEditChange = true;
          }
          else {
            if (s_tchBuf[0] == L'\0') {
              GetFindPattern(s_tchBuf, COUNTOF(s_tchBuf));
            }
            if (s_tchBuf[0] == L'\0') {
              MRU_Enum(Globals.pMRUfind, 0, s_tchBuf, COUNTOF(s_tchBuf));
            }
            SetDlgItemText(hwnd, IDC_FINDTEXT, s_tchBuf);
            bEditChange = true;
          }

          s_InitialTopLine = -1;  // reset
          s_anyMatch = s_fwrdMatch = NO_MATCH;
          Globals.bFindReplCopySelOrClip = false;

        } // Globals.bFindReplCopySelOrClip

        if (!bEditChange) { break; }

        bool const bEnableF = (GetWindowTextLengthW(GetDlgItem(hwnd, IDC_FINDTEXT)) ||
          CB_ERR != SendDlgItemMessage(hwnd, IDC_FINDTEXT, CB_GETCURSEL, 0, 0));

        bool const bEnableR = (GetWindowTextLengthW(GetDlgItem(hwnd, IDC_REPLACETEXT)) ||
          CB_ERR != SendDlgItemMessage(hwnd, IDC_REPLACETEXT, CB_GETCURSEL, 0, 0));

        bool const bEnableIS = !(SciCall_IsSelectionEmpty() || Sci_IsMultiOrRectangleSelection());

        DialogEnableControl(hwnd, IDOK, bEnableF);
        DialogEnableControl(hwnd, IDC_FINDPREV, bEnableF);
        DialogEnableControl(hwnd, IDC_REPLACE, bEnableF);
        DialogEnableControl(hwnd, IDC_REPLACEALL, bEnableF);
        DialogEnableControl(hwnd, IDC_REPLACEINSEL, bEnableF && bEnableIS);
        DialogEnableControl(hwnd, IDC_SWAPSTRG, bEnableF || bEnableR);

        if (!bEnableF) { s_anyMatch = s_fwrdMatch = NO_MATCH; }

        if (HIWORD(wParam) == CBN_CLOSEUP) {
          LONG lSelEnd;
          SendDlgItemMessage(hwnd, LOWORD(wParam), CB_GETEDITSEL, 0, (LPARAM)&lSelEnd);
          SendDlgItemMessage(hwnd, LOWORD(wParam), CB_SETEDITSEL, 0, MAKELPARAM(lSelEnd, lSelEnd));
        }

        _SetSearchFlags(hwnd, sg_pefrData);

        if (StrIsEmptyA(sg_pefrData->szFind)) {
          SetFindPattern(L"");
          SciCall_SetSel(s_InitialSearchStart, s_InitialSearchStart);
        }

        _DelayMarkAll(hwnd, 50, s_InitialSearchStart);

      }
      break;

      case IDT_TIMER_MAIN_MRKALL:
        {
          if (sg_pefrData->bMarkOccurences) {
            static char s_lastFind[FNDRPL_BUFFER] = { L'\0' };
            if (sg_pefrData->bStateChanged || (StringCchCompareXA(s_lastFind, sg_pefrData->szFind) != 0)) {
              _IGNORE_NOTIFY_CHANGE_;
              EditClearAllOccurrenceMarkers(sg_pefrData->hwnd);
              StringCchCopyA(s_lastFind, COUNTOF(s_lastFind), sg_pefrData->szFind);
              RegExResult_t match = _FindHasMatch(sg_pefrData->hwnd, sg_pefrData, 0, (sg_pefrData->bMarkOccurences), false);
              s_anyMatch = match;
              // we have to set Sci's regex instance to first find (have substitution in place)
              DocPos const iStartPos = (DocPos)lParam;
              if (!GetDlgItem(hwnd, IDC_REPLACE) || !Sci_IsSelectionMultiLine()) {
                s_fwrdMatch = _FindHasMatch(sg_pefrData->hwnd, sg_pefrData, iStartPos, false, true);
              }
              else {
                s_fwrdMatch = match;
              }
              InvalidateRect(GetDlgItem(hwnd, IDC_FINDTEXT), NULL, TRUE);

              if (match != MATCH) {
                EditClearAllOccurrenceMarkers(sg_pefrData->hwnd);
                if (s_InitialTopLine >= 0) {
                  SciCall_SetFirstVisibleLine(s_InitialTopLine);
                }
                else {
                  EditSetSelectionEx(s_InitialAnchorPos, s_InitialCaretPos, -1, -1);
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
            DialogEnableControl(hwnd, IDC_TOGGLE_VISIBILITY, true);
            _DelayMarkAll(hwnd, 50, s_InitialSearchStart);
          }
          else {  // switched OFF
            DialogEnableControl(hwnd, IDC_TOGGLE_VISIBILITY, false);
            if (FocusedView.HideNonMatchedLines) {
              EditToggleView(sg_pefrData->hwnd);
            }
            EditClearAllOccurrenceMarkers(sg_pefrData->hwnd);
            Globals.iMarkOccurrencesCount = 0;
            InvalidateRect(GetDlgItem(hwnd, IDC_FINDTEXT), NULL, TRUE);
          }
        }
        break;


      case IDC_TOGGLE_VISIBILITY:
        if (sg_pefrData) {
          EditToggleView(sg_pefrData->hwnd);
          if (!FocusedView.HideNonMatchedLines) {
            sg_pefrData->bStateChanged = true;
            s_InitialTopLine = -1;  // reset
            EditClearAllOccurrenceMarkers(sg_pefrData->hwnd);
            _DelayMarkAll(hwnd, 50, s_InitialSearchStart);
          }
        }
        break;


      case IDC_FINDREGEXP:
        if (IsButtonChecked(hwnd, IDC_FINDREGEXP))
        {
          DialogEnableControl(hwnd, IDC_DOT_MATCH_ALL, true);
          CheckDlgButton(hwnd, IDC_WILDCARDSEARCH, BST_UNCHECKED); // Can not use wildcard search together with regexp
          CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, BST_CHECKED); // transform BS handled by regex
          DialogEnableControl(hwnd, IDC_FINDTRANSFORMBS, false);
        }
        else { // unchecked
          DialogEnableControl(hwnd, IDC_DOT_MATCH_ALL, false);
          DialogEnableControl(hwnd, IDC_FINDTRANSFORMBS, true);
          CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, SetBtn(s_SaveTFBackSlashes));
        }
        _SetSearchFlags(hwnd, sg_pefrData);
        _DelayMarkAll(hwnd, 50, s_InitialSearchStart);
        break;

      case IDC_DOT_MATCH_ALL:
        _SetSearchFlags(hwnd, sg_pefrData);
        _DelayMarkAll(hwnd, 50, s_InitialSearchStart);
        break;

      case IDC_WILDCARDSEARCH:
        if (IsButtonChecked(hwnd, IDC_WILDCARDSEARCH))
        {
          CheckDlgButton(hwnd, IDC_FINDREGEXP, BST_UNCHECKED);
          DialogEnableControl(hwnd, IDC_DOT_MATCH_ALL, false);
          CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, SetBtn(s_SaveTFBackSlashes));
          // transform BS handled by regex (wildcard search based on):
          CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, BST_CHECKED);
          DialogEnableControl(hwnd, IDC_FINDTRANSFORMBS, false);
        }
        else { // unchecked
          CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, SetBtn(s_SaveTFBackSlashes));
          DialogEnableControl(hwnd, IDC_FINDTRANSFORMBS, true);
        }
        _SetSearchFlags(hwnd, sg_pefrData);
        _DelayMarkAll(hwnd, 50, s_InitialSearchStart);
        break;

      case IDC_FIND_OVERLAPPING:
        _SetSearchFlags(hwnd, sg_pefrData);
        _DelayMarkAll(hwnd, 50, s_InitialSearchStart);
        break;

      case IDC_FINDTRANSFORMBS:
        {
          s_SaveTFBackSlashes = IsButtonChecked(hwnd, IDC_FINDTRANSFORMBS);
          _SetSearchFlags(hwnd, sg_pefrData);
          _DelayMarkAll(hwnd, 50, s_InitialSearchStart);
        }
        break;

        case IDC_FINDCASE:
        _SetSearchFlags(hwnd, sg_pefrData);
        _DelayMarkAll(hwnd, 50, s_InitialSearchStart);
        break;

      case IDC_FINDWORD:
        _SetSearchFlags(hwnd, sg_pefrData);
        _DelayMarkAll(hwnd, 50, s_InitialSearchStart);
        break;

      case IDC_FINDSTART:
        _SetSearchFlags(hwnd, sg_pefrData);
        _DelayMarkAll(hwnd, 50, s_InitialSearchStart);
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
        bool bIsFindDlg = (GetDlgItem(hwnd, IDC_REPLACE) == NULL);

        if ((bIsFindDlg && LOWORD(wParam) == IDMSG_SWITCHTOREPLACE) ||
           (!bIsFindDlg && LOWORD(wParam) == IDMSG_SWITCHTOFIND)) {
          GetDlgPos(hwnd, &s_xFindReplaceDlgSave, &s_yFindReplaceDlgSave);
          s_bSwitchedFindReplace = true;
          CopyMemory(&s_efrSave, sg_pefrData, sizeof(EDITFINDREPLACE));
        }

        if (!s_bSwitchedFindReplace &&
          !GetDlgItemTextW2MB(hwnd, IDC_FINDTEXT, sg_pefrData->szFind, COUNTOF(sg_pefrData->szFind))) {
          DialogEnableControl(hwnd, IDOK, false);
          DialogEnableControl(hwnd, IDC_FINDPREV, false);
          DialogEnableControl(hwnd, IDC_REPLACE, false);
          DialogEnableControl(hwnd, IDC_REPLACEALL, false);
          DialogEnableControl(hwnd, IDC_REPLACEINSEL, false);
          if (!GetDlgItemTextW2MB(hwnd, IDC_REPLACETEXT, sg_pefrData->szReplace, COUNTOF(sg_pefrData->szReplace)))
            DialogEnableControl(hwnd, IDC_SWAPSTRG, false);
          return true;
        }

        _SetSearchFlags(hwnd, sg_pefrData);

        if (!s_bSwitchedFindReplace) {
          // Save MRUs
          if (!StrIsEmptyA(sg_pefrData->szFind)) {
            MultiByteToWideChar(Encoding_SciCP, 0, sg_pefrData->szFind, -1, s_tchBuf, (int)COUNTOF(s_tchBuf));
            MRU_Add(Globals.pMRUfind, s_tchBuf, 0, -1, -1, NULL);
            SetFindPattern(s_tchBuf);
          }
          if (!StrIsEmptyA(sg_pefrData->szReplace)) {
            MultiByteToWideChar(Encoding_SciCP, 0, sg_pefrData->szReplace, -1, s_tchBuf, (int)COUNTOF(s_tchBuf));
            MRU_Add(Globals.pMRUreplace, s_tchBuf, 0, -1, -1, NULL);
          }
        }

        // Reload MRUs
        SendDlgItemMessage(hwnd, IDC_FINDTEXT, CB_RESETCONTENT, 0, 0);
        SendDlgItemMessage(hwnd, IDC_REPLACETEXT, CB_RESETCONTENT, 0, 0);

        for (int i = 0; i < MRU_Count(Globals.pMRUfind); i++) {
          MRU_Enum(Globals.pMRUfind, i, s_tchBuf, COUNTOF(s_tchBuf));
          SendDlgItemMessage(hwnd, IDC_FINDTEXT, CB_ADDSTRING, 0, (LPARAM)s_tchBuf);
        }
        for (int i = 0; i < MRU_Count(Globals.pMRUreplace); i++) {
          MRU_Enum(Globals.pMRUreplace, i, s_tchBuf, COUNTOF(s_tchBuf));
          SendDlgItemMessage(hwnd, IDC_REPLACETEXT, CB_ADDSTRING, 0, (LPARAM)s_tchBuf);
        }

        SetDlgItemTextMB2W(hwnd, IDC_FINDTEXT, sg_pefrData->szFind);
        SetDlgItemTextMB2W(hwnd, IDC_REPLACETEXT, sg_pefrData->szReplace);

        if (!s_bSwitchedFindReplace) {
          SendMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetFocus()), 1);
        }

        switch (LOWORD(wParam))
        {
          case IDOK: // find next
          case IDACC_SELTONEXT:
            if (!bIsFindDlg) { Globals.bReplaceInitialized = true; }
            EditFindNext(sg_pefrData->hwnd, sg_pefrData, (LOWORD(wParam) == IDACC_SELTONEXT), IsKeyDown(VK_F3));
            s_InitialSearchStart = SciCall_GetSelectionStart();
            s_InitialAnchorPos = SciCall_GetAnchor();
            s_InitialCaretPos = SciCall_GetCurrentPos();
            s_InitialTopLine = -1;  // reset
            break;

          case IDC_FINDPREV: // find previous
          case IDACC_SELTOPREV:
            if (!bIsFindDlg) { Globals.bReplaceInitialized = true; }
            EditFindPrev(sg_pefrData->hwnd, sg_pefrData, (LOWORD(wParam) == IDACC_SELTOPREV), IsKeyDown(VK_F3));
            s_InitialSearchStart = SciCall_GetSelectionStart();
            s_InitialAnchorPos = SciCall_GetAnchor();
            s_InitialCaretPos = SciCall_GetCurrentPos();
            s_InitialTopLine = -1;  // reset
            break;

          case IDC_REPLACE:
            {
              Globals.bReplaceInitialized = true;
              EditReplace(sg_pefrData->hwnd, sg_pefrData);
            }
            break;

          case IDC_REPLACEALL:
            Globals.bReplaceInitialized = true;
            EditReplaceAll(sg_pefrData->hwnd, sg_pefrData, true);
            break;

          case IDC_REPLACEINSEL:
            if (!SciCall_IsSelectionEmpty()) {
              Globals.bReplaceInitialized = true;
              EditReplaceAllInSelection(sg_pefrData->hwnd, sg_pefrData, true);
            }
            break;
        }

        if (bIsFindDlg && (sg_pefrData->bFindClose)) {
          //~EndDialog(hwnd, LOWORD(wParam)); ~ not running own message loop
          DestroyWindow(hwnd);
        }
        else if ((LOWORD(wParam) != IDOK) && sg_pefrData->bReplaceClose) {
          //~EndDialog(hwnd, LOWORD(wParam)); ~ not running own message loop
          DestroyWindow(hwnd);
        }
      }
      _DelayMarkAll(hwnd, 50, s_InitialSearchStart);
      break;


      case IDCANCEL:
        //~EndDialog(hwnd, IDCANCEL); ~ not running own message loop
        DestroyWindow(hwnd);
        break;

      case IDC_FINDESCCTRLCHR:
      case IDC_REPLESCCTRLCHR:
        {
          static bool toggle = true;
          UINT const ctrl_id = (LOWORD(wParam) == IDC_FINDESCCTRLCHR) ? IDC_FINDTEXT : IDC_REPLACETEXT;
          GetDlgItemTextW(hwnd, ctrl_id, s_tchBuf, COUNTOF(s_tchBuf));
          size_t const len1 = StringCchLen(s_tchBuf, 0);
          if (toggle) {
            WCHAR trf[FNDRPL_BUFFER] = { L'\0' };
            size_t const len2 = SlashCtrlW(trf, COUNTOF(trf), s_tchBuf);
            if (len1 == len2) { UnSlashCtrlW(trf); }
            SetDlgItemTextW(hwnd, ctrl_id, trf);
          } else {
            size_t const len2 = UnSlashCtrlW(s_tchBuf);
            if (len1 != len2) {
              SetDlgItemTextW(hwnd, ctrl_id, s_tchBuf);
            } else {
              WCHAR trf[FNDRPL_BUFFER] = { L'\0' };
              SlashCtrlW(trf, COUNTOF(trf), s_tchBuf);
              SetDlgItemTextW(hwnd, ctrl_id, trf);
            }
          }
          toggle = !toggle;
        }
        break;

      case IDC_SWAPSTRG:
      {
        WCHAR* wszFind = s_tchBuf;
        WCHAR wszRepl[FNDRPL_BUFFER] = { L'\0' };
        GetDlgItemTextW(hwnd, IDC_FINDTEXT, wszFind, COUNTOF(s_tchBuf));
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
        CenterDlgInParent(hwnd, NULL);
        Settings.FindReplaceDlgPosX = Settings.FindReplaceDlgPosY = CW_USEDEFAULT;
        break;

      case IDACC_CLEAR_FIND_HISTORY:
        MRU_Empty(Globals.pMRUfind, true);
        if (Globals.bCanSaveIniFile) {
          MRU_Save(Globals.pMRUfind);
        }
        while ((int)SendDlgItemMessage(hwnd, IDC_FINDTEXT, CB_DELETESTRING, 0, 0) > 0) {};
        break;

      case IDACC_CLEAR_REPL_HISTORY:
        MRU_Empty(Globals.pMRUreplace, true);
        if (Globals.bCanSaveIniFile) {
          MRU_Save(Globals.pMRUreplace);
        }
        while ((int)SendDlgItemMessage(hwnd, IDC_REPLACETEXT, CB_DELETESTRING, 0, 0) > 0) {};
        break;

      case IDACC_FINDNEXT:
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
        CheckDlgButton(hwnd, IDC_FIND_OVERLAPPING, BST_UNCHECKED);
        CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, BST_UNCHECKED);
        PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_FINDTEXT)), 1);
        break;

      case IDACC_VIEWSCHEMECONFIG:
        PostWMCommand(GetParent(hwnd), IDM_VIEW_SCHEMECONFIG);
        break;

      default:
        return !0;
      }

    } // WM_COMMAND:
    return !0;


    case WM_SYSCOMMAND:
      if (wParam == IDS_MUI_SAVEPOS) {
        PostWMCommand(hwnd, IDACC_SAVEPOS);
        return !0;
      }
      else if (wParam == IDS_MUI_RESETPOS) {
        PostWMCommand(hwnd, IDACC_RESETPOS);
        return !0;
      }
      else if (wParam == IDS_MUI_CLEAR_FIND_HISTORY) {
        PostWMCommand(hwnd, IDACC_CLEAR_FIND_HISTORY);
        return !0;
      }
      else if (wParam == IDS_MUI_CLEAR_REPL_HISTORY) {
        PostWMCommand(hwnd, IDACC_CLEAR_REPL_HISTORY);
        return !0;
      }
      break;

    case WM_NOTIFY:
      {
        LPNMHDR pnmhdr = (LPNMHDR)lParam;
        switch (pnmhdr->code) 
        {
        case NM_CLICK:
        case NM_RETURN:
          switch (pnmhdr->idFrom) {
          case IDC_TOGGLEFINDREPLACE:
            if (GetDlgItem(hwnd, IDC_REPLACE)) {
              PostWMCommand(GetParent(hwnd), IDM_EDIT_FIND);
            } else {
              PostWMCommand(GetParent(hwnd), IDM_EDIT_REPLACE);
            }
            break;
          case IDC_FINDESCCTRLCHR:
            SendWMCommand(hwnd, IDC_FINDESCCTRLCHR);
            break;
          case IDC_REPLESCCTRLCHR:
            SendWMCommand(hwnd, IDC_REPLESCCTRLCHR);
            break;
          case IDC_BACKSLASHHELP:
            // Display help messages in the find/replace windows
            MessageBoxLng(MB_ICONINFORMATION, IDS_MUI_BACKSLASHHELP);
            break;
          case IDC_REGEXPHELP:
            MessageBoxLng(MB_ICONINFORMATION, IDS_MUI_REGEXPHELP);
            break;
          case IDC_WILDCARDHELP:
            MessageBoxLng(MB_ICONINFORMATION, IDS_MUI_WILDCARDHELP);
            break;
          default:
            break;
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

  return 0; // message handled
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
            EditFindReplaceDlgProc,
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

  bool bSuppressNotFound = false;

  char szFind[FNDRPL_BUFFER];
  DocPos const slen = _EditGetFindStrg(hwnd, lpefr, szFind, COUNTOF(szFind));
  if (slen <= 0) { return false; }
  int const sFlags = (int)(lpefr->fuFlags);

  if (bFocusWnd) {
    SetFocus(hwnd);
  }
  DocPos const iDocEndPos = Sci_GetDocEndPosition();
  DocPos start = 0;
  if (lpefr->bOverlappingFind) {
    EditSetCaretToSelectionStart();
    start = SciCall_PositionAfter(SciCall_GetSelectionStart());
  }
  else {
    EditSetCaretToSelectionEnd();
    start = SciCall_GetSelectionEnd();
  }
  DocPos end = iDocEndPos;

  if (start >= end) {
    if (IDOK == InfoBoxLng(MB_OKCANCEL, L"MsgFindWrap1", IDS_MUI_FIND_WRAPFW)) {
      end = min_p(start, iDocEndPos);  start = 0;
    }
    else {
      bSuppressNotFound = true;
    }
  }

  CancelCallTip();

  DocPos iPos = _FindInTarget(szFind, slen, sFlags, &start, &end, true, FRMOD_NORM);

  if ((iPos < -1) && (lpefr->fuFlags & SCFIND_REGEXP)) {
    InfoBoxLng(MB_ICONWARNING, L"MsgInvalidRegex", IDS_MUI_REGEX_INVALID);
    bSuppressNotFound = true;
  }
  else if ((iPos < 0) && (start > 0) && !bExtendSelection) 
  {
    UpdateStatusbar(false);
    if (!lpefr->bNoFindWrap && !bSuppressNotFound) {
      if (IDOK == InfoBoxLng(MB_OKCANCEL, L"MsgFindWrap2", IDS_MUI_FIND_WRAPFW)) 
      {
        end = min_p(start, iDocEndPos);  start = 0;

        iPos = _FindInTarget(szFind, slen, sFlags, &start, &end, false, FRMOD_WRAPED);

        if ((iPos < -1) && (lpefr->fuFlags & SCFIND_REGEXP)) {
          InfoBoxLng(MB_ICONWARNING, L"MsgInvalidRegex2", IDS_MUI_REGEX_INVALID);
          bSuppressNotFound = true;
        }
      }
      else {
        bSuppressNotFound = true;
      }
    }
  }

  if (iPos < 0) {
    if (!bSuppressNotFound) {
      InfoBoxLng(MB_OK, L"MsgNotFound", IDS_MUI_NOTFOUND);
    }
#ifdef _DEBUG
    WCHAR fnd[256];
    WCHAR msg[256];
    MultiByteToWideChar(CP_UTF8, 0, szFind, -1, fnd, (int)COUNTOF(fnd));
    StringCchPrintf(msg, COUNTOF(msg), L"Suchbegriff:'%s'", fnd);
    MsgBoxLastError(msg, 0);
#endif
    return false;
  }

  if (bExtendSelection) {
    DocPos const iSelPos = SciCall_GetCurrentPos();
    DocPos const iSelAnchor = SciCall_GetAnchor();
    EditSetSelectionEx(min_p(iSelAnchor, iSelPos), end, -1, -1);
  }
  else {
    EditSetSelectionEx(iPos, end, -1, -1);
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

  if (bFocusWnd) {
    SetFocus(hwnd);
  }
  DocPos const slen = _EditGetFindStrg(hwnd, lpefr, szFind, COUNTOF(szFind));
  if (slen <= 0) { return false; }
  int const sFlags = (int)(lpefr->fuFlags);

  DocPos const iDocEndPos = Sci_GetDocEndPosition();

  DocPos start = 0;
  if (lpefr->bOverlappingFind) {
    EditSetCaretToSelectionEnd();
    start = SciCall_PositionBefore(SciCall_GetSelectionEnd());
  }
  else {
    EditSetCaretToSelectionStart();
    start = SciCall_GetSelectionStart();
  }
  DocPos end = 0;

  if (start <= end) {
    if (IDOK == InfoBoxLng(MB_OKCANCEL, L"MsgFindWrap1", IDS_MUI_FIND_WRAPFW)) {
      end = max_p(start, 0);  start = iDocEndPos;
    }
    else
      bSuppressNotFound = true;
  }

  CancelCallTip();

  DocPos iPos = _FindInTarget(szFind, slen, sFlags, &start, &end, true, FRMOD_NORM);

  if ((iPos < -1) && (sFlags & SCFIND_REGEXP)) 
  {
    InfoBoxLng(MB_ICONWARNING, L"MsgInvalidRegex", IDS_MUI_REGEX_INVALID);
    bSuppressNotFound = true;
  }
  else if ((iPos < 0) && (start <= iDocEndPos) &&  !bExtendSelection) 
  {
    UpdateStatusbar(false);
    if (!lpefr->bNoFindWrap && !bSuppressNotFound) 
    {
      if (IDOK == InfoBoxLng(MB_OKCANCEL, L"MsgFindWrap2", IDS_MUI_FIND_WRAPRE)) 
      {
        end = max_p(start, 0);  start = iDocEndPos;

        iPos = _FindInTarget(szFind, slen, sFlags, &start, &end, false, FRMOD_WRAPED);

        if ((iPos < -1) && (sFlags & SCFIND_REGEXP)) {
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
    EditSetSelectionEx(max_p(iSelPos, iSelAnchor), iPos, -1, -1);
  }
  else {
    EditSetSelectionEx(end, iPos, -1, -1);
  }

  if (start == end) {
    EditShowZeroLengthCallTip(hwnd, iPos);
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
    EditMarkAll(NULL, searchFlags, iPosStart, iPosEnd, false);
  }
  else {
    EditMarkAll(NULL, searchFlags, 0, Sci_GetDocEndPosition(), false);
  }
  
  _OBSERVE_NOTIFY_CHANGE_;
}


//=============================================================================
//
//  EditSelectionMultiSelectAll()
//
void EditSelectionMultiSelectAll()
{
  if (SciCall_GetSelText(NULL) > 1)
  {
    DocPos const saveTargetBeg = SciCall_GetTargetStart();
    DocPos const saveTargetEnd = SciCall_GetTargetEnd();

    SciCall_TargetWholeDocument();

    SciCall_SetSearchFlags(GetMarkAllOccSearchFlags());

    SciCall_MultipleSelectAddEach();

    SciCall_SetMainSelection(0);
    if (SciCall_GetSelectionNAnchor(0) > SciCall_GetSelectionNCaret(0)) {
      SciCall_SwapMainAnchorCaret();
    }

    SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore

    EditEnsureSelectionVisible();
  }
}


//=============================================================================
//
//  EditSelectionMultiSelectAllEx()
//
void EditSelectionMultiSelectAllEx(EDITFINDREPLACE edFndRpl)
{
  if (IsWindow(Globals.hwndDlgFindReplace)) {
    _SetSearchFlags(Globals.hwndDlgFindReplace, &edFndRpl);
  }
  else {
    edFndRpl.fuFlags = GetMarkAllOccSearchFlags();
  }

  _IGNORE_NOTIFY_CHANGE_;
  EditMarkAll(edFndRpl.szFind, edFndRpl.fuFlags, 0, Sci_GetDocEndPosition(), true);
  _OBSERVE_NOTIFY_CHANGE_;
}


//=============================================================================
//
//  _GetReplaceString()
//
static char* _GetReplaceString(HWND hwnd, LPCEDITFINDREPLACE lpefr, int* iReplaceMsg)
{
  char* pszReplace = NULL; // replace text of arbitrary size
  if (StringCchCompareNIA(lpefr->szReplace, COUNTOF(lpefr->szReplace), "^c", 2) == 0) {
    *iReplaceMsg = SCI_REPLACETARGET;
    pszReplace = EditGetClipboardText(hwnd, true, NULL, NULL);
  }
  else {
    size_t const cch = StringCchLenA(lpefr->szReplace, COUNTOF(lpefr->szReplace));
    pszReplace = (char*)AllocMem(cch + 1, HEAP_ZERO_MEMORY);
    if (pszReplace) {
      StringCchCopyA(pszReplace, SizeOfMem(pszReplace), lpefr->szReplace);
      bool const bIsRegEx = (lpefr->fuFlags & SCFIND_REGEXP);
      if (lpefr->bTransformBS || bIsRegEx) {
        TransformBackslashes(pszReplace, bIsRegEx, Encoding_SciCP, iReplaceMsg);
      }
    }
  }
  return pszReplace; // move ownership
}


//=============================================================================
//
//  EditReplace()
//
bool EditReplace(HWND hwnd, LPCEDITFINDREPLACE lpefr) 
{
  int iReplaceMsg = SCI_REPLACETARGET;
  char* pszReplace = _GetReplaceString(hwnd, lpefr, &iReplaceMsg);
  if (!pszReplace) {
    return false; // recoding of clipboard canceled
  }
  DocPos const selBeg = SciCall_GetSelectionStart();
  DocPos const selEnd = SciCall_GetSelectionEnd();

  // redo find to get group ranges filled
  DocPos start = (SciCall_IsSelectionEmpty() ? SciCall_GetCurrentPos() : selBeg);
  DocPos end = Sci_GetDocEndPosition();
  DocPos _start = start;
  Globals.iReplacedOccurrences = 0;

  char szFind[FNDRPL_BUFFER];
  DocPos const slen = _EditGetFindStrg(hwnd, lpefr, szFind, COUNTOF(szFind));
  int const sFlags = (int)(lpefr->fuFlags);
  DocPos const iPos = _FindInTarget(szFind, slen, sFlags, &start, &end, false, FRMOD_NORM);

  // w/o selection, replacement string is put into current position
  // but this maybe not intended here
  if (SciCall_IsSelectionEmpty()) {
    if ((iPos < 0) || (_start != start) || (_start != end)) {
      // empty-replace was not intended
      FreeMem(pszReplace);
      if (iPos < 0) {
        return EditFindNext(hwnd, lpefr, false, false);
      }
      EditSetSelectionEx(start, end, -1, -1);
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

  _BEGIN_UNDO_ACTION_;

  SciCall_TargetFromSelection();
  Sci_ReplaceTarget(iReplaceMsg, -1, pszReplace);
  // move caret behind replacement
  SciCall_SetCurrentPos(SciCall_GetTargetEnd());
  Globals.iReplacedOccurrences = 1;

  SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); //restore
  FreeMem(pszReplace);

  _END_UNDO_ACTION_;

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
  int iCount = 0;

  if (iStartPos > iEndPos) { swapos(&iStartPos, &iEndPos); }

  char szFind[FNDRPL_BUFFER];
  size_t const slen = _EditGetFindStrg(hwnd, lpefr, szFind, COUNTOF(szFind));
  if (slen <= 0) { return 0; }
  int const sFlags = (int)(lpefr->fuFlags);

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

  DocPos iPos = _FindInTarget(szFind, slen, sFlags, &start, &end, false, FRMOD_NORM);

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
      iPos = _FindInTarget(szFind, slen, sFlags, &start, &end, ((posPair.end - posPair.beg) == 0), FRMOD_IGNORE);
    else
      iPos = -1;
  } 
  
  iCount = utarray_len(ReplPosUTArray);
  if (iCount <= 0) { FreeMem(pszReplace); return 0; }

  // ===  iterate over findings and replace strings  ===
  DocPos searchStart = iStartPos;
  DocPos totalReplLength = 0;

  _BEGIN_UNDO_ACTION_;

  for (ReplPos_t* pPosPair = (ReplPos_t*)utarray_front(ReplPosUTArray);
                  pPosPair != NULL;
                  pPosPair = (ReplPos_t*)utarray_next(ReplPosUTArray, pPosPair)) {

    start = searchStart;
    end = iEndPos + totalReplLength;

    if (iReplaceMsg == SCI_REPLACETARGETRE) 
    {
      // redo find to get group ranges filled
      /*iPos = */_FindInTarget(szFind, slen, sFlags, &start, &end, false, FRMOD_IGNORE);
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

  _END_UNDO_ACTION_;

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

  BeginWaitCursor(true,L"Replace all...")
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
  if (Sci_IsMultiOrRectangleSelection()) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECTORMULTI);
    return false;
  }

  const DocPos start = SciCall_GetSelectionStart();
  const DocPos end = SciCall_GetSelectionEnd();
  const DocPos currPos = SciCall_GetCurrentPos();
  const DocPos anchorPos = SciCall_GetAnchor();
  DocPos enlargement = 0;

  Globals.iReplacedOccurrences = EditReplaceAllInRange(hwnd, lpefr, start, end, &enlargement);

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

  SciCall_MarkerDeleteAll(MARKER_NP3_OCCURRENCE);

  _OBSERVE_NOTIFY_CHANGE_;
}


//=============================================================================
//
//  EditClearAllBookMarks()
//
void EditClearAllBookMarks(HWND hwnd)
{
    UNUSED(hwnd);
    int const bitmask = OCCURRENCE_MARKER_BITMASK() & ~(1 << MARKER_NP3_BOOKMARK);
    DocLn const line = SciCall_MarkerNext(0, bitmask);
    if (line >= 0) {
        // 1st press: clear all occurrences marker
        for (int m = MARKER_NP3_1; m < MARKER_NP3_BOOKMARK; ++m) {
            SciCall_MarkerDeleteAll(m);
        }
    } else {
        // if no occurrences marker found
        SciCall_MarkerDeleteAll(MARKER_NP3_BOOKMARK);
    }
}


//=============================================================================
//
//  EditToggleView()
//
void EditToggleView(HWND hwnd)
{
  if (Settings.FocusViewMarkerMode & FVMM_FOLD) {
    BeginWaitCursor(true, L"Toggle View...");

    FocusedView.HideNonMatchedLines = !FocusedView.HideNonMatchedLines; // toggle

    if (FocusedView.HideNonMatchedLines) {
      EditFoldMarkedLineRange(hwnd, true);
      if (Settings.FocusViewMarkerMode & (FVMM_MARGIN | FVMM_LN_BACKGR)) {
        EditBookMarkLineRange(hwnd);
      }
    } else {
      EditFoldMarkedLineRange(hwnd, false);
    }

    SciCall_SetReadOnly(FocusedView.HideNonMatchedLines);
    SciCall_ScrollCaret();

    EndWaitCursor();
  }
  else if (Settings.FocusViewMarkerMode & (FVMM_MARGIN | FVMM_LN_BACKGR))
  {
    EditBookMarkLineRange(hwnd);
  }
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
  flags |= (bRegEx ? SCFIND_REGEXP : 0);
  flags |= (bWordStart ? SCFIND_WORDSTART : 0);
  flags |= (bMatchWords ? SCFIND_WHOLEWORD : 0);
  flags |= (bMatchCase ? SCFIND_MATCHCASE : 0);
  flags |= (bDotMatchAll ? SCFIND_DOT_MATCH_ALL : 0);
  return flags;
}


//=============================================================================
//
//  EditMarkAll()
//  Mark all occurrences of the matching text in range (by Aleksandar Lekov)
//
void EditMarkAll(char* pszFind, int sFlags, DocPos rangeStart, DocPos rangeEnd, bool bMultiSel)
{
  char txtBuffer[FNDRPL_BUFFER] = { '\0' };
  char* pszText = (pszFind != NULL) ? pszFind : txtBuffer;

  DocPos iFindLength = 0;

  if (StrIsEmptyA(pszText))
  {
    if (SciCall_IsSelectionEmpty()) {
      // nothing selected, get word under caret if flagged
      if (Settings.MarkOccurrencesCurrentWord && (sFlags & SCFIND_WHOLEWORD))
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
      if (sFlags & SCFIND_WHOLEWORD) {
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

    if (bMultiSel) { SciCall_ClearSelections(); }

    DocPos const iTextEnd = Sci_GetDocEndPosition();
    rangeStart = max_p(0, rangeStart);
    rangeEnd = min_p(rangeEnd, iTextEnd);

    DocPos start = rangeStart;
    DocPos end = rangeEnd;

    DocPos iPos = (DocPos)-1;

    DocPosU count = 0;
    do {

      iPos = _FindInTarget(pszText, iFindLength, sFlags, &start, &end, (start == iPos), FRMOD_IGNORE);

      if (iPos < 0) {
        break; // not found
      }

      if (bMultiSel) {
        if (count) {
          SciCall_AddSelection(end, iPos);
        }
        else {
          SciCall_SetSelection(end, iPos);
        }
      }
      else {
        // mark this match if not done before
        SciCall_SetIndicatorCurrent(INDIC_NP3_MARK_OCCURANCE);
        SciCall_IndicatorFillRange(iPos, (end - start));
        SciCall_MarkerAdd(SciCall_LineFromPosition(iPos), MARKER_NP3_OCCURRENCE);
      }
      start = end;
      end = rangeEnd;
      ++count;

    } while (start < end);
    
    Globals.iMarkOccurrencesCount = count;
  }
}


//=============================================================================
//
//  EditCheckNewLineInACFillUps()
//
bool EditCheckNewLineInACFillUps()
{
  return s_ACFillUpCharsHaveNewLn;
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

  DocPos const iMinWdChCnt = autoInsert ? 0 : 2;  // min number of typed chars before AutoC

  char const* const pchAllowdWordChars = 
    ((Globals.bIsCJKInputCodePage || Globals.bUseLimitedAutoCCharSet) ? AutoCompleteWordCharSet :
    (Settings.AccelWordNavigation ? WordCharsAccelerated : WordCharsDefault));
  
  SciCall_SetWordChars(pchAllowdWordChars);

  DocPos const iDocEndPos = Sci_GetDocEndPosition();
  DocPos const iCurrentPos = SciCall_GetCurrentPos();
  DocPos const iCol = SciCall_GetColumn(iCurrentPos);
  DocPos const iPosBefore = SciCall_PositionBefore(iCurrentPos);
  DocPos const iWordStartPos = SciCall_WordStartPosition(iPosBefore, true);

  if (((iPosBefore - iWordStartPos) < iMinWdChCnt) || (iCol < iMinWdChCnt) || ((iCurrentPos - iWordStartPos) < iMinWdChCnt)) {
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
  if (!autoInsert && bWordAllNumbers) {
    EditSetAccelWordNav(hwnd, Settings.AccelWordNavigation);
    return true;
  }

  char pRoot[_MAX_AUTOC_WORD_LEN];
  DocPos const iRootLen = (iCurrentPos - iWordStartPos);
  StringCchCopyNA(pRoot, COUNTOF(pRoot), SciCall_GetRangePointer(iWordStartPos, iRootLen), (size_t)iRootLen);
  if ((iRootLen <= 0) || StrIsEmptyA(pRoot)) { return true; } // nothing to find

  int iNumWords = 0;
  size_t iWListSize = 0;

  PWLIST pListHead = NULL;

  if (Settings.AutoCompleteWords || (autoInsert && !Settings.AutoCLexerKeyWords))
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
    FreeMem(pwlNewWord); pwlNewWord = NULL;
  }
  // --------------------------------------------------------------------------
  if (Settings.AutoCLexerKeyWords || (autoInsert && !Settings.AutoCompleteWords))
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
    FreeMem(pwlNewWord); pwlNewWord = NULL;
  }

  // --------------------------------------------------------------------------

  if (iNumWords > 0) 
  {
    const char* const sep = " ";
    SciCall_AutoCCancel();
    SciCall_ClearRegisteredImages();

    SciCall_AutoCSetSeperator(sep[0]);
    SciCall_AutoCSetIgnoreCase(true);
    //~SciCall_AutoCSetCaseInsensitiveBehaviour(SC_CASEINSENSITIVEBEHAVIOUR_IGNORECASE);
    SciCall_AutoCSetChooseSingle(autoInsert);
    SciCall_AutoCSetOrder(SC_ORDER_PERFORMSORT); // already sorted
    SciCall_AutoCSetFillups(AutoCompleteFillUpChars);

    ++iWListSize; // zero termination
    char* const pList = AllocMem(iWListSize + 1, HEAP_ZERO_MEMORY);
    if (pList) {
      PWLIST pTmp = NULL;
      PWLIST pWLItem = NULL;
      LL_FOREACH_SAFE(pListHead, pWLItem, pTmp) {
        if (pWLItem->word[0]) {
          StringCchCatA(pList, SizeOfMem(pList), sep);
          StringCchCatA(pList, SizeOfMem(pList), pWLItem->word);
        }
        LL_DELETE(pListHead, pWLItem);
        FreeMem(pWLItem);
      }
      SciCall_AutoCShow(iRootLen, &pList[1]); // skip first sep
      FreeMem(pList);
    }
  }

  EditSetAccelWordNav(hwnd, Settings.AccelWordNavigation);
  return true;
}


//=============================================================================
//
//  EditDoStyling()
//
void EditDoVisibleStyling()
{
  DocLn const iStartLine = SciCall_DocLineFromVisible(SciCall_GetFirstVisibleLine());
  DocLn const iEndLine = min_ln((iStartLine + SciCall_LinesOnScreen()), (SciCall_GetLineCount() - 1));
  EditDoStyling(SciCall_PositionFromLine(iStartLine), SciCall_GetLineEndPosition(iEndLine));
}

//=============================================================================
//
//  EditDoStyling()
//
void EditDoStyling(DocPos iStartPos, DocPos iEndPos)
{
  static bool guard = false;  // protect against recursion by notification event SCN_STYLENEEDED

  if (Flags.bLargeFileLoaded) { return; }

  if (!guard)
  {
    guard = true;
    if (iStartPos < 0) {
      iStartPos = SciCall_GetEndStyled();
    }
    else {
      iStartPos = SciCall_PositionFromLine(SciCall_LineFromPosition(iStartPos));
    }

    if (iEndPos < 0) {
      Sci_ApplyLexerStyle(iStartPos, -1);
    }
    else {
      iEndPos = SciCall_GetLineEndPosition(SciCall_LineFromPosition(iEndPos));
      if (iStartPos < iEndPos) {
        Sci_ApplyLexerStyle(iStartPos, iEndPos);
      }
    }
    guard = false;
  }
}


//=============================================================================
//
//  EditUpdateVisibleIndicators()
// 
void EditUpdateVisibleIndicators()
{
  DocLn const iStartLine = SciCall_DocLineFromVisible(SciCall_GetFirstVisibleLine());
  DocLn const iEndLine = min_ln((iStartLine + SciCall_LinesOnScreen()), (SciCall_GetLineCount() - 1));
  EditUpdateIndicators(SciCall_PositionFromLine(iStartLine), SciCall_GetLineEndPosition(iEndLine), false);
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

static void _UpdateIndicators(const int indicator, const int indicator2nd, 
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

    DocPos const start_m = start;
    DocPos const end_m   = end;
    DocPos const iPos = _FindInTarget(regExpr, iRegExLen, SCFIND_REGEXP, &start, &end, false, FRMOD_IGNORE);

    if (iPos < 0) {
      // not found
      _ClearIndicatorInRange(indicator, indicator2nd, start_m, end_m);
      break;
    }
    DocPos const mlen = end - start;
    if ((mlen <= 0) || (end > endPos)) {
      // wrong match
      _ClearIndicatorInRange(indicator, indicator2nd, start_m, end_m);
      break; // wrong match
    }

    _ClearIndicatorInRange(indicator, indicator2nd, start_m, end);

    SciCall_SetIndicatorCurrent(indicator);
    SciCall_IndicatorFillRange(start, mlen);
    if (indicator2nd >= 0) {
      SciCall_SetIndicatorCurrent(indicator2nd);
      SciCall_IndicatorFillRange(start, mlen);
    }

    // next occurrence
    start = SciCall_PositionAfter(end);
    end = endPos;

  } while (start < end);
  
}

//=============================================================================
//
//  EditUpdateIndicators()
//  - Find and mark all URL hot-spots
//  - Find and mark all COLOR refs (#RRGGBB)
//
void EditUpdateIndicators(DocPos startPos, DocPos endPos, bool bClearOnly)
{
  if (bClearOnly) {
    _ClearIndicatorInRange(INDIC_NP3_HYPERLINK, INDIC_NP3_HYPERLINK_U, startPos, endPos);
    _ClearIndicatorInRange(INDIC_NP3_COLOR_DEF, INDIC_NP3_COLOR_DEF_T, startPos, endPos);
    _ClearIndicatorInRange(INDIC_NP3_UNICODE_POINT, -1, startPos, endPos);
    return;
  }
  if (Settings.HyperlinkHotspot)
  {
    // https://mathiasbynens.be/demo/url-regex : @stephenhay
    //static const char* pUrlRegEx = "\\b(?:(?:https?|ftp|file)://|www\\.|ftp\\.)[^\\s/$.?#].[^\\s]*";

    static const char* const pUrlRegEx = "\\b(?:(?:https?|ftp|file)://|www\\.|ftp\\.)"
      "(?:\\([-a-z\\u00a1-\\uffff0-9+&@#/%=~_|$?!:,.]*\\)|[-a-z\\u00a1-\\uffff0-9+&@#/%=~_|$?!:,.])*"
      "(?:\\([-a-z\\u00a1-\\uffff0-9+&@#/%=~_|$?!:,.]*\\)|[a-z\\u00a1-\\uffff0-9+&@#/%=~_|$])";

    _UpdateIndicators(INDIC_NP3_HYPERLINK, INDIC_NP3_HYPERLINK_U, pUrlRegEx, startPos, endPos);
  }
  else {
    _ClearIndicatorInRange(INDIC_NP3_HYPERLINK, INDIC_NP3_HYPERLINK_U, startPos, endPos);
  }
  
  if (IsColorDefHotspotEnabled()) 
  {
    static const char* const pColorRegEx = "#([0-9a-fA-F]){8}|#([0-9a-fA-F]){6}"; // ARGB, RGBA, RGB
    static const char* const pColorRegEx_A = "#([0-9a-fA-F]){8}"; // no RGB search (BGRA)
    if (Settings.ColorDefHotspot < 3) {
      _UpdateIndicators(INDIC_NP3_COLOR_DEF, -1, pColorRegEx, startPos, endPos);
    }
    else {
      _UpdateIndicators(INDIC_NP3_COLOR_DEF, -1, pColorRegEx_A, startPos, endPos);
    }
  }
  else {
    _ClearIndicatorInRange(INDIC_NP3_COLOR_DEF, INDIC_NP3_COLOR_DEF_T, startPos, endPos);
  }

  if (Settings.HighlightUnicodePoints) 
  {
    static const char* const pUnicodeRegEx = "(\\\\[uU|xX]([0-9a-fA-F]){4}|\\\\[xX]([0-9a-fA-F]){2})+";
    _UpdateIndicators(INDIC_NP3_UNICODE_POINT, -1, pUnicodeRegEx, startPos, endPos);
  }
  else {
    _ClearIndicatorInRange(INDIC_NP3_UNICODE_POINT, -1, startPos, endPos);
  }

  EditDoStyling(startPos, endPos);
}


//=============================================================================
//
//  EditFoldMarkedLineRange()
//
void EditFoldMarkedLineRange(HWND hwnd, bool bHideLines)
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
        FocusedView.ShowCodeFolding      = true;
        Style_SetFoldingFocusedView();
        Style_SetFolding(hwnd, true);

        int const baseLevel = SC_FOLDLEVELBASE;

        DocLn const iStartLine = 0;
        DocLn const iEndLine   = SciCall_GetLineCount() - 1;

        // 1st line
        int level = baseLevel;
        SciCall_SetFoldLevel(iStartLine, SC_FOLDLEVELHEADERFLAG | level++); // visible in any case

        int const bitmask = (1 << MARKER_NP3_OCCURRENCE);
        DocLn     markerLine = SciCall_MarkerNext(iStartLine + 1, bitmask);
        
        for (DocLn line = iStartLine + 1; line <= iEndLine; ++line)
        {
            if (line == markerLine) { // visible
                level = baseLevel;
                SciCall_SetFoldLevel(line, SC_FOLDLEVELHEADERFLAG | level++);
                markerLine = SciCall_MarkerNext(line + 1, bitmask); // next
            }
            else { // hide line
                SciCall_SetFoldLevel(line, SC_FOLDLEVELWHITEFLAG | level);
            }
        }
        SciCall_FoldAll(FOLD);
    }
}


//=============================================================================
//
//  EditBookMarkLineRange()
//
void EditBookMarkLineRange(HWND hwnd)
{
    UNUSED(hwnd);
    // get next free bookmark
    int marker;
    for (marker = MARKER_NP3_1; marker < MARKER_NP3_BOOKMARK; ++marker) { // all(!)
      if (SciCall_MarkerNext(0, (1 << marker)) < 0) {
        break; // found unused
      }
    }
    if (marker >= MARKER_NP3_BOOKMARK) {
      InfoBoxLng(MB_ICONWARNING, L"OutOfOccurrenceMarkers", IDS_MUI_OUT_OFF_OCCMRK);
      return;
    }

    DocLn line = -1;
    int const bitmask = (1 << MARKER_NP3_OCCURRENCE);
    do {
        line = SciCall_MarkerNext(line + 1, bitmask);
        if (line >= 0) {
            SciCall_MarkerAdd(line, marker);
        }
    } while (line >= 0);
}


//=============================================================================
//
//  EditDeleteMarkerInSelection()
//
void EditDeleteMarkerInSelection()
{
  if (Sci_IsStreamSelection() && !SciCall_IsSelectionEmpty())
  {
    DocPos const posSelBeg = SciCall_GetSelectionStart();
    DocPos const posSelEnd = SciCall_GetSelectionEnd();
    DocLn const lnBeg = SciCall_LineFromPosition(posSelBeg);
    DocLn const lnEnd = SciCall_LineFromPosition(posSelEnd);
    DocLn const lnDelBeg = (posSelBeg <= SciCall_PositionFromLine(lnBeg)) ? lnBeg : lnBeg + 1;
    DocLn const lnDelEnd = (posSelEnd  > SciCall_GetLineEndPosition(lnEnd)) ? lnEnd : lnEnd - 1;
    for (DocLn ln = lnDelBeg; ln <= lnDelEnd; ++ln) {
      SciCall_MarkerDelete(ln, -1);
    }
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

        SetDialogIconNP3(hwnd);

        DocLn const iCurLine = SciCall_LineFromPosition(SciCall_GetCurrentPos())+1;
        DocLn const iMaxLnNum = SciCall_GetLineCount();
        DocPos const iCurColumn = SciCall_GetColumn(SciCall_GetCurrentPos()) + 1;
        DocPos const iLineEndPos = Sci_GetNetLineLength(iCurLine);

        FormatLngStringW(wchLineCaption, COUNTOF(wchLineCaption), IDS_MUI_GOTO_LINE, 
          (int)clampp(iMaxLnNum, 0, INT_MAX));
        FormatLngStringW(wchColumnCaption, COUNTOF(wchColumnCaption), IDS_MUI_GOTO_COLUMN, 
          (int)clampp(max_p(iLineEndPos, (DocPos)Settings.LongLinesLimit), 0, INT_MAX));
        SetDlgItemText(hwnd, IDC_LINE_TEXT, wchLineCaption);
        SetDlgItemText(hwnd, IDC_COLUMN_TEXT, wchColumnCaption);

        SetDlgItemInt(hwnd, IDC_LINENUM, (int)clampp(iCurLine, 0, INT_MAX), false);
        SetDlgItemInt(hwnd, IDC_COLNUM, (int)clampp(iCurColumn, 0, INT_MAX), false);
        SendDlgItemMessage(hwnd,IDC_LINENUM,EM_LIMITTEXT,80,0);
        SendDlgItemMessage(hwnd,IDC_COLNUM,EM_LIMITTEXT,80,0);
        CenterDlgInParent(hwnd, NULL);
      }
      return true;


    case WM_DPICHANGED:
      {
        DPI_T dpi;
        dpi.x = LOWORD(wParam);
        dpi.y = HIWORD(wParam);
        UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, &dpi);
      }
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

          intptr_t iExprError    = 0;
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
            EditJumpTo(iNewLine, iNewCol);
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

  static unsigned id_hover = 0;
  static unsigned id_capture = 0;

  //static HFONT   hFontNormal = NULL;
  static HFONT   hFontHover = NULL;
  static HCURSOR hCursorNormal;
  static HCURSOR hCursorHover;

  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        id_hover = 0;
        id_capture = 0;

        SetDialogIconNP3(hwnd);

        HFONT const hFont = (HFONT)SendDlgItemMessage(hwnd, 200, WM_GETFONT, 0, 0);
        if (hFont) {
          LOGFONT lf;
          GetObject(hFont, sizeof(LOGFONT), &lf);
          lf.lfUnderline = true;
          //lf.lfWeight    = FW_BOLD;
          if (hFontHover) {
            DeleteObject(hFontHover);
          }
          hFontHover = CreateFontIndirectW(&lf);
        }
        
        hCursorNormal = LoadCursor(NULL, IDC_ARROW);
        hCursorHover = LoadCursor(NULL,IDC_HAND);
        if (!hCursorHover) {
          hCursorHover = LoadCursor(Globals.hInstance, IDC_ARROW);
        }
        pdata = (PMODLINESDATA)lParam;
        SetDlgItemTextW(hwnd,100,pdata->pwsz1);
        SendDlgItemMessage(hwnd,100,EM_LIMITTEXT,255,0);
        SetDlgItemTextW(hwnd,101,pdata->pwsz2);
        SendDlgItemMessage(hwnd,101,EM_LIMITTEXT,255,0);
        CenterDlgInParent(hwnd, NULL);
      }
      return true;

    case WM_DPICHANGED:
      {
        DPI_T dpi;
        dpi.x = LOWORD(wParam);
        dpi.y = HIWORD(wParam);

        HFONT const hFont = (HFONT)SendDlgItemMessage(hwnd, 200, WM_GETFONT, 0, 0);
        if (hFont) {
          LOGFONT lf;
          GetObject(hFont, sizeof(LOGFONT), &lf);
          lf.lfUnderline = true;
          //lf.lfWeight    = FW_BOLD;
          if (hFontHover) {
            DeleteObject(hFontHover);
          }
          hFontHover = CreateFontIndirectW(&lf);
        }

        UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, &dpi);
      }
      return !0;

    case WM_DESTROY:
      //DeleteObject(hFontNormal);
      DeleteObject(hFontHover);
      return 0;

    case WM_NCACTIVATE:
      if (!(bool)wParam) {
        if (id_hover != 0) {
          //int _id_hover = id_hover;
          id_hover = 0;
          id_capture = 0;
        }
      }
      return 0;

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
          //SelectObject(hdc, (dwId == id_hover) ? hFontHover : hFontNormal);
          SelectObject(hdc, hFontHover);
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
            else {
              id_hover = 0;
            }
          }
          else {
            id_hover = 0;
          }
          SetCursor((id_hover != 0) ? hCursorHover : hCursorNormal);
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
        }
        SetCursor((id_hover != 0) ? hCursorHover : hCursorNormal);
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
        SetCursor((id_hover != 0) ? hCursorHover : hCursorNormal);
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
      return !0;
  }
  return 0;
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
        SetDialogIconNP3(hwnd);
        CheckRadioButton(hwnd,100,104,*piAlignMode+100);
        CenterDlgInParent(hwnd, NULL);
      }
      return true;

    case WM_DPICHANGED:
      UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, NULL);
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
        SetDialogIconNP3(hwnd);
        SendDlgItemMessage(hwnd,100,EM_LIMITTEXT,255,0);
        SetDlgItemTextW(hwnd,100,pdata->pwsz1);
        SendDlgItemMessage(hwnd,101,EM_LIMITTEXT,255,0);
        SetDlgItemTextW(hwnd,101,pdata->pwsz2);
        CenterDlgInParent(hwnd, NULL);
      }
      return true;

    case WM_DPICHANGED:
      UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, NULL);
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
//            102 Times
//
typedef struct _tagsdata {
  LPWSTR pwsz1;
  LPWSTR pwsz2;
  UINT   repeat;
} TAGSDATA, *PTAGSDATA;


static INT_PTR CALLBACK EditInsertTagDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  static PTAGSDATA pdata;
  static WCHAR wchOpenTagStrg[256] = { L'\0' };
  static WCHAR wchCloseTagStrg[256] = { L'\0' };

  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        pdata = (PTAGSDATA)lParam;
        SetDialogIconNP3(hwnd);
        if (!wchOpenTagStrg[0]) { StringCchCopy(wchOpenTagStrg, COUNTOF(wchOpenTagStrg), L"<tag>"); }
        if (!wchCloseTagStrg[0]) { StringCchCopy(wchCloseTagStrg, COUNTOF(wchCloseTagStrg), L"</tag>"); }
        SendDlgItemMessage(hwnd,100,EM_LIMITTEXT, COUNTOF(wchOpenTagStrg)-1,0);
        SetDlgItemTextW(hwnd,100, wchOpenTagStrg);
        SendDlgItemMessage(hwnd,101,EM_LIMITTEXT, COUNTOF(wchCloseTagStrg)-1,0);
        SetDlgItemTextW(hwnd,101, wchCloseTagStrg);
        pdata->repeat = 1;
        SetDlgItemInt(hwnd, 102, pdata->repeat, FALSE);
        SetFocus(GetDlgItem(hwnd,100));
        PostMessage(GetDlgItem(hwnd,100),EM_SETSEL,1,(int)(StringCchLen(wchOpenTagStrg,0)-1));
        CenterDlgInParent(hwnd, NULL);
      }
      return false;

    case WM_DPICHANGED:
      UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, NULL);
      return true;

    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
        case 100: {
            if (HIWORD(wParam) == EN_CHANGE) 
            {
              bool bClear = true;
              GetDlgItemTextW(hwnd,100,wchOpenTagStrg, COUNTOF(wchOpenTagStrg));
              if (StringCchLenW(wchOpenTagStrg,COUNTOF(wchOpenTagStrg)) >= 3) {

                if (wchOpenTagStrg[0] == L'<')
                {
                  WCHAR wchIns[COUNTOF(wchCloseTagStrg)] = { L'\0' };
                  StringCchCopy(wchIns, COUNTOF(wchIns), L"</");
                  int  cchIns = 2;
                  const WCHAR* pwCur = &wchOpenTagStrg[1];
                  while (
                    *pwCur &&
                    *pwCur != L'<' &&
                    *pwCur != L'>' &&
                    *pwCur != L' ' &&
                    *pwCur != L'\t' &&
                    (StrChr(L":_-.",*pwCur) || IsCharAlphaNumericW(*pwCur)))

                    wchIns[cchIns++] = *pwCur++;

                  while (*pwCur && *pwCur != L'>') { pwCur++; }

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
                        StringCchCompareXI(wchIns,L"</meta>")) 
                    {
                        SetDlgItemTextW(hwnd,101, wchIns);
                        bClear = false;
                    }
                  }
                }
              }
              if (bClear) {
                SetDlgItemTextW(hwnd, 101, L"");
              }
            }
          }
          break;
        case IDOK: {
            GetDlgItemTextW(hwnd, 100, wchOpenTagStrg, COUNTOF(wchOpenTagStrg));
            GetDlgItemTextW(hwnd, 101, wchCloseTagStrg, COUNTOF(wchCloseTagStrg));
            StringCchCopy(pdata->pwsz1, 256, wchOpenTagStrg);
            StringCchCopy(pdata->pwsz2, 256, wchCloseTagStrg);
            BOOL fTranslated = FALSE;
            UINT const iTimes = GetDlgItemInt(hwnd, 102, &fTranslated, FALSE);
            if (fTranslated) {
              pdata->repeat = clampu(iTimes, 1, UINT_MAX);
            }
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
bool EditInsertTagDlg(HWND hwnd,LPWSTR pwszOpen,LPWSTR pwszClose, UINT* pRepeat)
{

  INT_PTR iResult;
  TAGSDATA data;
  data.pwsz1 = pwszOpen;  data.pwsz2 = pwszClose;  data.repeat = 1;
  
  iResult = ThemedDialogBoxParam(
              Globals.hLngResContainer,
              MAKEINTRESOURCEW(IDD_MUI_INSERTTAG),
              hwnd,
              EditInsertTagDlgProc,
              (LPARAM)&data);

  if (iResult == IDOK) {
    *pRepeat = data.repeat;
    return true;
  }
  return false;
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

        SetDialogIconNP3(hwnd);

        if (*piSortFlags & SORT_DESCENDING) {
          CheckRadioButton(hwnd, 100, 102, 101);
        }
        else if (*piSortFlags & SORT_SHUFFLE) {
          CheckRadioButton(hwnd,100,102,102);
          DialogEnableControl(hwnd,103,false);
          DialogEnableControl(hwnd,104,false);
          DialogEnableControl(hwnd,105,false);
          DialogEnableControl(hwnd,106,false);
          DialogEnableControl(hwnd,107,false);
          DialogEnableControl(hwnd,108,false);
          DialogEnableControl(hwnd,109,false);
          DialogEnableControl(hwnd,110,false);
          DialogEnableControl(hwnd,111,false);
        }
        else {
          CheckRadioButton(hwnd, 100, 102, 100);
        }
        if (*piSortFlags & SORT_MERGEDUP) {
          CheckDlgButton(hwnd, 103, BST_CHECKED);
        }
        if (*piSortFlags & SORT_UNIQDUP) {
          CheckDlgButton(hwnd, 104, BST_CHECKED);
          DialogEnableControl(hwnd, 103, false);
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
          DialogEnableControl(hwnd, 106, false);
        }
        
        CheckRadioButton(hwnd, 108, 111, 108);
        
        if (*piSortFlags & SORT_NOCASE) {
          CheckRadioButton(hwnd, 108, 111, 109);
        }
        else if (*piSortFlags & SORT_LOGICAL) {
          CheckRadioButton(hwnd, 108, 111, 110);
        }
        else if (*piSortFlags & SORT_LEXICOGRAPH) {
          CheckRadioButton(hwnd, 108, 111, 111);
        }
        if (!Sci_IsMultiOrRectangleSelection()) {
          *piSortFlags &= ~SORT_COLUMN;
          DialogEnableControl(hwnd,112,false);
        }
        else {
          *piSortFlags |= SORT_COLUMN;
          CheckDlgButton(hwnd,112,BST_CHECKED);
        }
        CenterDlgInParent(hwnd, NULL);
      }
      return true;

    case WM_DPICHANGED:
      UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, NULL);
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
              *piSortFlags &= ~SORT_NOCASE;
            if (IsButtonChecked(hwnd,109))
              *piSortFlags |= SORT_NOCASE;
            if (IsButtonChecked(hwnd,110))
              *piSortFlags |= SORT_LOGICAL;
            if (IsButtonChecked(hwnd,111))
              *piSortFlags |= SORT_LEXICOGRAPH;
            if (IsButtonChecked(hwnd,112))
              *piSortFlags |= SORT_COLUMN;
            EndDialog(hwnd,IDOK);
          }
          break;

        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;

        case 100:
        case 101:
          DialogEnableControl(hwnd,103, IsButtonUnchecked(hwnd,105));
          DialogEnableControl(hwnd,104,true);
          DialogEnableControl(hwnd,105,true);
          DialogEnableControl(hwnd,106,true);
          DialogEnableControl(hwnd,107,true);
          DialogEnableControl(hwnd,108,true);
          DialogEnableControl(hwnd,109,true);
          DialogEnableControl(hwnd,110,true);
          DialogEnableControl(hwnd,111,true);
          break;
        case 102:
          DialogEnableControl(hwnd,103,false);
          DialogEnableControl(hwnd,104,false);
          DialogEnableControl(hwnd,105,false);
          DialogEnableControl(hwnd,106,false);
          DialogEnableControl(hwnd,107,false);
          DialogEnableControl(hwnd,108,false);
          DialogEnableControl(hwnd,109,false);
          DialogEnableControl(hwnd,110,false);
          DialogEnableControl(hwnd,111,false);
          break;
        case 104:
          DialogEnableControl(hwnd,103,IsButtonUnchecked(hwnd,104));
          break;
        case 107:
          if (IsButtonChecked(hwnd, 107)) {
            CheckDlgButton(hwnd, 106, BST_CHECKED);
            DialogEnableControl(hwnd, 106, false);
          }
          else {
            DialogEnableControl(hwnd, 106, true);
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
  else {
    SciCall_SetCharsDefault();
  }
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
  UNUSED(hwnd);
  WCHAR tchLine[32];
  StringCchCopyW(pszBookMarks, cchLength, L"");
  int const bitmask = (1 << MARKER_NP3_BOOKMARK);
  DocLn iLine = -1;
  do {
    iLine = SciCall_MarkerNext(iLine + 1, bitmask);
    if (iLine >= 0) {
      StringCchPrintfW(tchLine, COUNTOF(tchLine), DOCPOSFMTW L";", iLine);
      StringCchCatW(pszBookMarks, cchLength, tchLine);
    }
  } while (iLine >= 0);

  StrTrim(pszBookMarks, L";");
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

  DocLn const iLineMax = SciCall_GetLineCount() - 1;

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
//  EditBookmarkNext()
//
void EditBookmarkNext(HWND hwnd, const DocLn iLine)
{
  UNUSED(hwnd);
  int bitmask = SciCall_MarkerGet(iLine) & OCCURRENCE_MARKER_BITMASK();
  if (!bitmask) {
    bitmask = (1 << MARKER_NP3_BOOKMARK);
  }
  DocLn iNextLine = SciCall_MarkerNext(iLine + 1, bitmask);
  if (iNextLine == (DocLn)-1) {
    iNextLine = SciCall_MarkerNext(0, bitmask); // wrap around
  }
  if (iNextLine == (DocLn)-1) {
    bitmask = OCCURRENCE_MARKER_BITMASK();
    iNextLine = SciCall_MarkerNext(iLine + 1, bitmask); // find any bookmark
  }
  if (iNextLine == (DocLn)-1) {
    iNextLine = SciCall_MarkerNext(0, bitmask); // wrap around
  }

  if (iNextLine != (DocLn)-1) {
    SciCall_GotoLine(iNextLine);
  }
}

//=============================================================================
//
//  EditBookmarkPrevious()
//
void EditBookmarkPrevious(HWND hwnd, const DocLn iLine)
{
  UNUSED(hwnd);
  int bitmask = SciCall_MarkerGet(iLine) & OCCURRENCE_MARKER_BITMASK();
  if (!bitmask) {
    bitmask = (1 << MARKER_NP3_BOOKMARK);
  }
  DocLn iPrevLine = SciCall_MarkerPrevious(max_ln(0, iLine - 1), bitmask);
  if (iPrevLine == (DocLn)-1) {
    iPrevLine = SciCall_MarkerPrevious(SciCall_GetLineCount(), bitmask); // wrap around
  }
  if (iPrevLine == (DocLn)-1) {
    bitmask = OCCURRENCE_MARKER_BITMASK();
    iPrevLine = SciCall_MarkerPrevious(max_ln(0, iLine - 1), bitmask); //find any bookmark
  }
  if (iPrevLine == (DocLn)-1) {
    iPrevLine = SciCall_MarkerPrevious(SciCall_GetLineCount(), bitmask); // wrap around
  }

  if (iPrevLine != (DocLn)-1) {
    SciCall_GotoLine(iPrevLine);
  } 
}


//=============================================================================
//
//  EditBookmarkToggle()
//
void EditBookmarkToggle(HWND hwnd, const DocLn ln, const int modifiers) {
  UNUSED(hwnd);
  int const bitmask = SciCall_MarkerGet(ln) & OCCURRENCE_MARKER_BITMASK();
  if (!bitmask) {
    SciCall_MarkerAdd(ln, MARKER_NP3_BOOKMARK); // set
  } else if (bitmask & (1 << MARKER_NP3_BOOKMARK)) {
    SciCall_MarkerDelete(ln, MARKER_NP3_BOOKMARK); // unset
  } else {
    for (int m = MARKER_NP3_1; m < MARKER_NP3_BOOKMARK; ++m) {
      if (bitmask & (1 << m)) {
        SciCall_MarkerDeleteAll(m);
      }
    }
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
      if (fToggled) { EditEnsureSelectionVisible(); }
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
      prev.dwTickCount = GetTickCount();
      return;
    }
  }

  EditFoldPerformAction(ln, mode, SNIFF);

  if (fGotoFoldPoint) {
    EditJumpTo(ln + 1, 0);
  }
}


void EditFoldCmdKey(FOLD_MOVE move, FOLD_ACTION action)
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
          EditJumpTo(ln + 1, 0);
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
          EditJumpTo(ln + 1, 0);
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
