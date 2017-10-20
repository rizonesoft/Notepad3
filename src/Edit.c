﻿/******************************************************************************
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
#include "SciCall.h"
#include "../crypto/crypto.h"
#include "helpers.h"
#include "edit.h"

#ifndef LCMAP_TITLECASE
#define LCMAP_TITLECASE  0x00000300  // Title Case Letters bit mask
#endif

#define DEFAULT_SCROLL_WIDTH 4096    // 4K

extern HWND  hwndMain;
extern HWND  hwndEdit;
extern HINSTANCE g_hInstance;
//extern LPMALLOC  g_lpMalloc;
extern DWORD dwLastIOError;
extern UINT cpLastFind;
extern BOOL bReplaceInitialized;
extern BOOL bUseOldStyleBraceMatching;

static EDITFINDREPLACE efrSave;
static BOOL bSwitchedFindReplace = FALSE;

static int xFindReplaceDlgSave;
static int yFindReplaceDlgSave;
extern int xFindReplaceDlg;
extern int yFindReplaceDlg;

extern int iDefaultEOLMode;
extern int iLineEndings[3];
extern BOOL bFixLineEndings;
extern BOOL bAutoStripBlanks;

// Default Codepage and Character Set
extern int iDefaultEncoding;
extern int iDefaultCharSet;
extern BOOL bLoadASCIIasUTF8;
extern BOOL bLoadNFOasOEM;

extern BOOL bAccelWordNavigation;
extern BOOL bVirtualSpaceInRectSelection;

extern int iMarkOccurrences;
extern int iMarkOccurrencesCount;
extern int iMarkOccurrencesMaxCount;

extern NP2ENCODING mEncoding[];

#define DELIM_BUFFER 258
char DelimChars[DELIM_BUFFER] = { '\0' };
char DelimCharsAccel[DELIM_BUFFER] = { '\0' };
char WordCharsDefault[DELIM_BUFFER] = { '\0' };
char WhiteSpaceCharsDefault[DELIM_BUFFER] = { '\0' };
char PunctuationCharsDefault[DELIM_BUFFER] = { '\0' };
char WordCharsAccelerated[DELIM_BUFFER] = { '\0' };
char WhiteSpaceCharsAccelerated[DELIM_BUFFER] = { '\0' };
char PunctuationCharsAccelerated[1] = { '\0' }; // empty!

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


//=============================================================================
//
//  EditCreate()
//
HWND EditCreate(HWND hwndParent)
{

  HWND hwnd;

  hwnd = CreateWindowEx(
           WS_EX_CLIENTEDGE,
           L"Scintilla",
           NULL,
           WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
           0,0,0,0,
           hwndParent,
           (HMENU)IDC_EDIT,
           g_hInstance,
           NULL);

  Encoding_Current(iDefaultEncoding);
  Encoding_SciSetCodePage(hwnd,iDefaultEncoding);
  SendMessage(hwnd,SCI_SETEOLMODE,SC_EOL_CRLF,0);
  SendMessage(hwnd,SCI_SETPASTECONVERTENDINGS,TRUE,0);
  SendMessage(hwnd,SCI_SETMODEVENTMASK,/*SC_MODEVENTMASKALL*/SC_MOD_INSERTTEXT|SC_MOD_DELETETEXT|SC_MOD_CONTAINER,0);
  SendMessage(hwnd,SCI_USEPOPUP,FALSE,0);
  SendMessage(hwnd,SCI_SETSCROLLWIDTH, DEFAULT_SCROLL_WIDTH,0);
  SendMessage(hwnd,SCI_SETSCROLLWIDTHTRACKING,TRUE,0);
  SendMessage(hwnd,SCI_SETENDATLASTLINE,TRUE,0);
  SendMessage(hwnd,SCI_SETCARETLINEVISIBLEALWAYS,TRUE,0);
  SendMessage(hwnd,SCI_SETCARETSTICKY,SC_CARETSTICKY_OFF,0);
  //SendMessage(hwnd,SCI_SETCARETSTICKY,SC_CARETSTICKY_WHITESPACE,0);
  SendMessage(hwnd,SCI_SETXCARETPOLICY,CARET_SLOP|CARET_EVEN,50);
  SendMessage(hwnd,SCI_SETYCARETPOLICY,CARET_EVEN,0);
  SendMessage(hwnd,SCI_SETMOUSESELECTIONRECTANGULARSWITCH,TRUE,0);
  SendMessage(hwnd,SCI_SETMULTIPLESELECTION,FALSE,0);
  SendMessage(hwnd,SCI_SETADDITIONALSELECTIONTYPING,FALSE,0);
  SendMessage(hwnd,SCI_SETVIRTUALSPACEOPTIONS,
    (bVirtualSpaceInRectSelection ? (SCVS_RECTANGULARSELECTION | SCVS_USERACCESSIBLE | SCVS_NOWRAPLINESTART) : SCVS_NONE),0);
  SendMessage(hwnd,SCI_SETADDITIONALCARETSBLINK,FALSE,0);
  SendMessage(hwnd,SCI_SETADDITIONALCARETSVISIBLE,FALSE,0);
  SendMessage(hwnd,SCI_SETMOUSEWHEELCAPTURES,FALSE,0);
    
  SendMessage(hwnd,SCI_ASSIGNCMDKEY,(SCK_NEXT + (SCMOD_CTRL << 16)),SCI_PARADOWN);
  SendMessage(hwnd,SCI_ASSIGNCMDKEY,(SCK_PRIOR + (SCMOD_CTRL << 16)),SCI_PARAUP);
  SendMessage(hwnd,SCI_ASSIGNCMDKEY,(SCK_NEXT + ((SCMOD_CTRL | SCMOD_SHIFT) << 16)),SCI_PARADOWNEXTEND);
  SendMessage(hwnd,SCI_ASSIGNCMDKEY,(SCK_PRIOR + ((SCMOD_CTRL | SCMOD_SHIFT) << 16)),SCI_PARAUPEXTEND);
  SendMessage(hwnd,SCI_ASSIGNCMDKEY,(SCK_HOME + (0 << 16)),SCI_VCHOMEWRAP);
  SendMessage(hwnd,SCI_ASSIGNCMDKEY,(SCK_END + (0 << 16)),SCI_LINEENDWRAP);
  SendMessage(hwnd,SCI_ASSIGNCMDKEY,(SCK_HOME + (SCMOD_SHIFT << 16)),SCI_VCHOMEWRAPEXTEND);
  SendMessage(hwnd,SCI_ASSIGNCMDKEY,(SCK_END + (SCMOD_SHIFT << 16)),SCI_LINEENDWRAPEXTEND);

  // set indicator styles
  SendMessage(hwnd, SCI_INDICSETOUTLINEALPHA, INDIC_NP3_MARK_OCCURANCE, 220);
  SendMessage(hwnd, SCI_INDICSETALPHA, INDIC_NP3_MARK_OCCURANCE, 100);
  SendMessage(hwnd, SCI_INDICSETFORE, INDIC_NP3_MARK_OCCURANCE, 0xff << ((iMarkOccurrences - 1) << 3));
  SendMessage(hwnd, SCI_INDICSETSTYLE, INDIC_NP3_MARK_OCCURANCE, INDIC_ROUNDBOX);

  SendMessage(hwnd, SCI_INDICSETOUTLINEALPHA, INDIC_NP3_BAD_BRACE, 220);
  SendMessage(hwnd, SCI_INDICSETALPHA,INDIC_NP3_MATCH_BRACE, 120);
  SendMessage(hwnd, SCI_INDICSETFORE,INDIC_NP3_MATCH_BRACE, 0xff << (1 << 3)); // overriden by style
  SendMessage(hwnd, SCI_INDICSETSTYLE,INDIC_NP3_MATCH_BRACE, INDIC_FULLBOX);

  SendMessage(hwnd, SCI_INDICSETOUTLINEALPHA, INDIC_NP3_BAD_BRACE, 220);
  SendMessage(hwnd, SCI_INDICSETALPHA, INDIC_NP3_BAD_BRACE, 120);
  SendMessage(hwnd, SCI_INDICSETFORE, INDIC_NP3_BAD_BRACE, 0xff ); // overriden by style
  SendMessage(hwnd, SCI_INDICSETSTYLE, INDIC_NP3_BAD_BRACE, INDIC_FULLBOX);
  

  // word delimiter handling
  EditInitWordDelimiter(hwnd);
  EditSetAccelWordNav(hwnd,bAccelWordNavigation);

  // Init default values for printing
  EditPrintInit();

  //SciInitThemes(hwnd);

  return(hwnd);

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
  SendMessage(hwnd, SCI_GETWHITESPACECHARS,0,(LPARAM)WhiteSpaceCharsDefault);
  SendMessage(hwnd, SCI_GETPUNCTUATIONCHARS,0,(LPARAM)PunctuationCharsDefault);

  // default word delimiter chars are whitespace & punctuation & line ends
  const char* lineEnds = "\r\n";
  StringCchCopyA(DelimChars,COUNTOF(DelimChars),WhiteSpaceCharsDefault);
  StringCchCatA(DelimChars,COUNTOF(DelimChars),PunctuationCharsDefault);
  StringCchCatA(DelimChars,COUNTOF(DelimChars), lineEnds);

  // 2nd get user settings
  WCHAR buffer[DELIM_BUFFER] = { L'\0' };
  ZeroMemory(buffer, DELIM_BUFFER * sizeof(WCHAR));

  IniGetString(L"Settings2",L"ExtendedWhiteSpaceChars",L"",buffer,COUNTOF(buffer));
  char whitesp[DELIM_BUFFER] = { '\0' };
  if (StringCchLen(buffer) > 0) {
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

  if (SendMessage(hwnd,SCI_GETREADONLY,0,0))
    SendMessage(hwnd,SCI_SETREADONLY,FALSE,0);

  SendMessage(hwnd,SCI_CANCEL,0,0);
  SendMessage(hwnd,SCI_SETUNDOCOLLECTION,0,0);
  UndoRedoSelectionMap(-1,NULL);
  SendMessage(hwnd,SCI_CLEARALL,0,0);
  SendMessage(hwnd,SCI_MARKERDELETEALL,(WPARAM)-1,0);
  SendMessage(hwnd,SCI_SETSCROLLWIDTH, DEFAULT_SCROLL_WIDTH,0);
  SendMessage(hwnd,SCI_SETXOFFSET,0,0);

  FileVars_Apply(hwnd,&fvCurFile);

  if (cbText > 0)
    SendMessage(hwnd,SCI_ADDTEXT,cbText,(LPARAM)lpstrText);

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
BOOL EditConvertText(HWND hwnd,int encSource,int encDest,BOOL bSetSavePoint)
{
  struct Sci_TextRange tr = { { 0, -1 }, NULL };
  int length, cbText, cbwText;
  char *pchText;
  WCHAR *pwchText;

  if (encSource == encDest)
    return(TRUE);

  if (!(Encoding_IsValid(encSource) && Encoding_IsValid(encDest)))
    return(FALSE);

  length = (int)SendMessage(hwnd,SCI_GETLENGTH,0,0);

  if (length == 0) 
  {
    SendMessage(hwnd,SCI_CANCEL,0,0);
    SendMessage(hwnd,SCI_SETUNDOCOLLECTION,0,0);
    UndoRedoSelectionMap(-1,NULL);
    SendMessage(hwnd,SCI_CLEARALL,0,0);
    SendMessage(hwnd,SCI_MARKERDELETEALL,(WPARAM)-1,0);
    Encoding_SciSetCodePage(hwnd,encDest);
    SendMessage(hwnd,SCI_SETUNDOCOLLECTION,(WPARAM)1,0);
    //SendMessage(hwnd,EM_EMPTYUNDOBUFFER,0,0); // deprecated
    SendMessage(hwnd,SCI_GOTOPOS,0,0);
    SendMessage(hwnd,SCI_CHOOSECARETX,0,0);

    if (bSetSavePoint)
      SendMessage(hwnd,SCI_SETSAVEPOINT,0,0);
  }
  else {

    const int chLen = length * 5 + 1;
    pchText = GlobalAlloc(GPTR,chLen);

    tr.lpstrText = pchText;
    SendMessage(hwnd,SCI_GETTEXTRANGE,0,(LPARAM)&tr);

    const int wchLen = length * 3 + 1;
    pwchText = GlobalAlloc(GPTR,wchLen);

    // MultiBytes(Sci) -> WideChar(destination) -> Sci(MultiByte)
    //UINT cpSci = mEncoding[encSource].uCodePage;
    UINT cpSci = Encoding_SciGetCodePage(hwnd); // fixed Scintilla internal 
    UINT cpDst = mEncoding[encDest].uCodePage;
    cbwText = MultiByteToWideChar(cpDst,0,pchText,length,pwchText,wchLen);
    cbText = WideCharToMultiByte(cpSci,0,pwchText,cbwText,pchText,chLen,NULL,NULL);

    SendMessage(hwnd,SCI_CANCEL,0,0);
    SendMessage(hwnd,SCI_SETUNDOCOLLECTION,0,0);
    UndoRedoSelectionMap(-1,NULL);
    SendMessage(hwnd,SCI_CLEARALL,0,0);
    SendMessage(hwnd,SCI_MARKERDELETEALL,(WPARAM)-1,0);
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
  
    if (SendMessage(hwnd, SCI_GETLENGTH, 0, 0) == 0) {

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
        BeginWaitCursor();
        BOOL result = EditConvertText(hwnd,iCurrentEncoding,iNewEncoding,FALSE);
        EndWaitCursor();
        return result;
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

  UINT codepage = mEncoding[Encoding_Current(CPI_GET)].uCodePage;

  if ((codepage == CP_UTF7) || (codepage == CP_UTF8))
    return FALSE;

  DWORD dwFlags = WC_NO_BEST_FIT_CHARS | WC_COMPOSITECHECK | WC_DEFAULTCHAR;
  BOOL useNullParams = (mEncoding[Encoding_Current(CPI_GET)].uFlags & NCP_MBCS) ? TRUE : FALSE;

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
  HANDLE hmem;
  WCHAR *pwch;
  char  *pmch;
  char  *ptmp;
  int    wlen,mlen,mlen2;

  if (!IsClipboardFormatAvailable(CF_UNICODETEXT) || !OpenClipboard(GetParent(hwnd))) {
    char* pEmpty = StrDupA("");
    return (pEmpty);
  }

  // get clipboard
  hmem = GetClipboardData(CF_UNICODETEXT);
  pwch = GlobalLock(hmem);
  wlen = lstrlenW(pwch);

  if (bCheckEncoding && EditIsRecodingNeeded(pwch,wlen)) 
  {
    int iPos = (int)SendMessage(hwnd,SCI_GETCURRENTPOS,0,0);
    int iAnchor = (int)SendMessage(hwnd,SCI_GETANCHOR,0,0);

    // switch encoding to universal UTF-8 codepage
    SendMessage(hwndMain,WM_COMMAND,(WPARAM)MAKELONG(IDM_ENCODING_UTF8,1),0);

    // restore and adjust selection
    if (iPos > iAnchor) {
      SendMessage(hwnd,SCI_SETSEL,(WPARAM)iAnchor,(LPARAM)iPos);
    }
    else {
      SendMessage(hwnd,SCI_SETSEL,(WPARAM)iPos,(LPARAM)iAnchor);
    }
    EditFixPositions(hwnd);
  }

  // translate to SCI editor component codepage (default: UTF-8)
  UINT codepage = Encoding_SciGetCodePage(hwnd);

  mlen = WideCharToMultiByte(codepage,0,pwch,wlen,NULL,0,NULL,NULL);
  pmch = LocalAlloc(LPTR,mlen + 1);
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
    ptmp = LocalAlloc(LPTR,mlen * 2 + 2);
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
      mlen2 = (int)(d - ptmp);

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
//  EditCopyAppend()
//
BOOL EditCopyAppend(HWND hwnd)
{
  HANDLE hOld;
  WCHAR  *pszOld;

  HANDLE hNew;
  WCHAR  *pszNew;

  char  *pszText;
  int   cchTextW;
  WCHAR *pszTextW;

  WCHAR *pszSep = L"\r\n\r\n";

  UINT  uCodePage;

  int iCurPos;
  int iAnchorPos;

  if (!IsClipboardFormatAvailable(CF_UNICODETEXT)) {
    SendMessage(hwnd,SCI_COPY,0,0);
    return(TRUE);
  }

  iCurPos    = (int)SendMessage(hwnd,SCI_GETCURRENTPOS,0,0);
  iAnchorPos = (int)SendMessage(hwnd,SCI_GETANCHOR,0,0);

  if (iCurPos != iAnchorPos) {

    if (SC_SEL_RECTANGLE == SendMessage(hwnd, SCI_GETSELECTIONMODE, 0, 0)) {
      MsgBox(MBWARN, IDS_SELRECT);
      return(FALSE);
    }
    else {
      int iSelLength = (int)SendMessage(hwnd, SCI_GETSELTEXT, 0, 0);
      pszText = LocalAlloc(LPTR, iSelLength);
      (int)SendMessage(hwnd, SCI_GETSELTEXT, 0, (LPARAM)pszText);
    }
  }
  else {
    int cchText = (int)SendMessage(hwnd,SCI_GETLENGTH,0,0);
    pszText = LocalAlloc(LPTR,cchText + 1);
    SendMessage(hwnd,SCI_GETTEXT,(int)LocalSize(pszText),(LPARAM)pszText);
  }

  uCodePage = Encoding_SciGetCodePage(hwnd);
  cchTextW = MultiByteToWideChar(uCodePage,0,pszText,-1,NULL,0);
  if (cchTextW > 0) {
    int lenTxt = (lstrlen(pszSep) + cchTextW + 1);
    pszTextW = LocalAlloc(LPTR,sizeof(WCHAR)*lenTxt);
    StringCchCopy(pszTextW,lenTxt,pszSep);
    MultiByteToWideChar(uCodePage,0,pszText,-1,StrEnd(pszTextW),lenTxt);
  }
  else {
    pszTextW = L"";
  }

  LocalFree(pszText);

  if (!OpenClipboard(GetParent(hwnd))) {
    LocalFree(pszTextW);
    return(FALSE);
  }

  hOld   = GetClipboardData(CF_UNICODETEXT);
  pszOld = GlobalLock(hOld);

  int sizeNew = (lstrlen(pszOld) + lstrlen(pszTextW) + 1);
  hNew = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,sizeof(WCHAR) * sizeNew);
  pszNew = GlobalLock(hNew);
  
  StringCchCopy(pszNew,sizeNew,pszOld);
  StringCchCat(pszNew,sizeNew,pszTextW);

  GlobalUnlock(hNew);
  GlobalUnlock(hOld);

  EmptyClipboard();
  SetClipboardData(CF_UNICODETEXT,hNew);
  CloseClipboard();

  return(TRUE);
}


//=============================================================================
//
//  EditDetectEOLMode() - moved here to handle Unicode files correctly
//
int EditDetectEOLMode(HWND hwnd,char* lpData,DWORD cbData)
{
  int iEOLMode = iLineEndings[iDefaultEOLMode];
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
       BOOL bSkipEncodingDetection,
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
    Encoding_Source(CPI_NONE);
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
      Encoding_Source(CPI_NONE);
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
      Encoding_Source(CPI_NONE);
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
    Encoding_Source(CPI_NONE);
    Encoding_SrcWeak(CPI_NONE);
    return FALSE;
  }

  DWORD cbData = 0L;
  BOOL bReadSuccess = ReadAndDecryptFile(hwnd, hFile, (DWORD)GlobalSize(lpData) - 2, &lpData, &cbData);
  dwLastIOError = GetLastError();
  CloseHandle(hFile);

  if (!bReadSuccess) {
    GlobalFree(lpData);
    Encoding_Source(CPI_NONE);
    Encoding_SrcWeak(CPI_NONE);
    return FALSE;
  }

  BOOL bPreferOEM = FALSE;
  if (bLoadNFOasOEM)
  {
    if (lpszExt && !(StringCchCompareIX(lpszExt,L".nfo") && StringCchCompareIX(lpszExt,L".diz")))
      bPreferOEM = TRUE;
  }

  int _iPrefEncoding = (bPreferOEM) ? g_DOSEncoding : iDefaultEncoding;
  if (Encoding_IsValid(Encoding_SrcWeak(CPI_GET)))
    _iPrefEncoding = Encoding_SrcWeak(CPI_GET);

  BOOL bBOM = FALSE;
  BOOL bReverse = FALSE;

  const int iSrcEnc = Encoding_Source(CPI_GET);

  if (cbData == 0) {
    FileVars_Init(NULL,0,&fvCurFile);
    *iEOLMode = iLineEndings[iDefaultEOLMode];
    if (iSrcEnc == CPI_NONE) {
      if (bLoadASCIIasUTF8 && !bPreferOEM)
        *iEncoding = CPI_UTF8;
      else
        *iEncoding = _iPrefEncoding;
    }
    else
      *iEncoding = iSrcEnc;

    Encoding_SciSetCodePage(hwnd,*iEncoding);
    EditSetNewText(hwnd,"",0);
    SendMessage(hwnd,SCI_SETEOLMODE,iLineEndings[iDefaultEOLMode],0);
    GlobalFree(lpData);
  }

  else if (!bSkipEncodingDetection && 
      (iSrcEnc == CPI_NONE    || iSrcEnc == CPI_UNICODE   || iSrcEnc == CPI_UNICODEBE) &&
      (iSrcEnc == CPI_UNICODE || iSrcEnc == CPI_UNICODEBE || IsUnicode(lpData,cbData,&bBOM,&bReverse)) &&
      (iSrcEnc == CPI_UNICODE || iSrcEnc == CPI_UNICODEBE || !IsUTF8Signature(lpData))) // check for UTF-8 signature
  {
    char* lpDataUTF8;

    if (iSrcEnc == CPI_UNICODE) {
      bBOM = (*((UNALIGNED PWCHAR)lpData) == 0xFEFF);
      bReverse = FALSE;
    }
    else if (iSrcEnc == CPI_UNICODEBE)
      bBOM = (*((UNALIGNED PWCHAR)lpData) == 0xFFFE);

    if (iSrcEnc == CPI_UNICODEBE || bReverse) {
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
      Encoding_Source(CPI_NONE);
      Encoding_SrcWeak(CPI_NONE);
      return FALSE;
    }
  }

  else {
    FileVars_Init(lpData,cbData,&fvCurFile);
    if (!bSkipEncodingDetection && (iSrcEnc == CPI_NONE || iSrcEnc == CPI_UTF8 || iSrcEnc == CPI_UTF8SIGN) &&
            ((IsUTF8Signature(lpData) ||
              FileVars_IsUTF8(&fvCurFile) ||
              (iSrcEnc == CPI_UTF8 || iSrcEnc == CPI_UTF8SIGN) ||
              (IsUTF8(lpData,cbData) &&
              (((UTF8_mbslen_bytes(UTF8StringStart(lpData)) - 1 !=
                UTF8_mbslen(UTF8StringStart(lpData),IsUTF8Signature(lpData) ? cbData-3 : cbData)) ||
                (!bPreferOEM && (
                  mEncoding[_iPrefEncoding].uFlags & NCP_UTF8 ||
                  bLoadASCIIasUTF8))))))) && !(FileVars_IsNonUTF8(&fvCurFile) &&
                  (iSrcEnc != CPI_UTF8 && iSrcEnc != CPI_UTF8SIGN)))
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

    else {
      if (iSrcEnc != CPI_NONE)
        *iEncoding = iSrcEnc;
      else {
        *iEncoding = FileVars_GetEncoding(&fvCurFile);
        if (*iEncoding == CPI_NONE) {
          if (fvCurFile.mask & FV_ENCODING)
            *iEncoding = CPI_ANSI_DEFAULT;
          else {
            if (Encoding_SrcWeak(CPI_GET) == CPI_NONE)
              *iEncoding = _iPrefEncoding;
            else if (mEncoding[Encoding_SrcWeak(CPI_GET)].uFlags & NCP_INTERNAL)
              *iEncoding = iDefaultEncoding;
            else
              *iEncoding = _iPrefEncoding;
          }
        }
      }

      if (((mEncoding[*iEncoding].uCodePage != CP_UTF7) && (mEncoding[*iEncoding].uFlags & NCP_8BIT)) ||
          ((mEncoding[*iEncoding].uCodePage == CP_UTF7) && IsUTF7(lpData,cbData))) {

        UINT uCodePage  = mEncoding[*iEncoding].uCodePage;

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
            Encoding_Source(CPI_NONE);
            Encoding_SrcWeak(CPI_NONE);
            return FALSE;
          }
        }
        else {
          GlobalFree(lpDataWide);
          GlobalFree(lpData);
          Encoding_Source(CPI_NONE);
          Encoding_SrcWeak(CPI_NONE);
          return FALSE;
        }
      }
      else {
        *iEncoding = Encoding_IsValid(iSrcEnc) ? iSrcEnc : iDefaultEncoding;
        Encoding_SciSetCodePage(hwnd,*iEncoding);
        EditSetNewText(hwnd,"",0);
        EditSetNewText(hwnd,lpData,cbData);
        *iEOLMode = EditDetectEOLMode(hwnd,lpData,cbData);
        GlobalFree(lpData);
      }
    }
  }

  Encoding_Source(CPI_NONE);
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
    EditStripTrailingBlanks(hwnd,TRUE);

  // get text
  cbData = (int)SendMessage(hwnd,SCI_GETLENGTH,0,0);
  lpData = GlobalAlloc(GPTR, cbData + 4); //fix: +bom
  SendMessage(hwnd,SCI_GETTEXT,GlobalSize(lpData),(LPARAM)lpData);

  if (cbData == 0) {
    bWriteSuccess = SetEndOfFile(hFile);
    dwLastIOError = GetLastError();
  }

  else {

  // FIXME: move checks in front of disk file access
  /*if ((mEncoding[iEncoding].uFlags & NCP_UNICODE) == 0 && (mEncoding[iEncoding].uFlags & NCP_UTF8_SIGN) == 0) {
      BOOL bEncodingMismatch = TRUE;
      FILEVARS fv;
      FileVars_Init(lpData,cbData,&fv);
      if (fv.mask & FV_ENCODING) {
        int iAltEncoding;
        if (FileVars_IsValidEncoding(&fv)) {
          iAltEncoding = FileVars_GetEncoding(&fv);
          if (iAltEncoding == iEncoding)
            bEncodingMismatch = FALSE;
          else if ((mEncoding[iAltEncoding].uFlags & NCP_UTF8) && (mEncoding[iEncoding].uFlags & NCP_UTF8))
            bEncodingMismatch = FALSE;
        }
        if (bEncodingMismatch) {
          Encoding_GetLabel(iAltEncoding);
          Encoding_GetLabel(iEncoding);
          InfoBox(0,L"MsgEncodingMismatch",IDS_ENCODINGMISMATCH,
            mEncoding[iAltEncoding].wchLabel,
            mEncoding[iEncoding].wchLabel);
        }
      }
    }*/

    if (mEncoding[iEncoding].uFlags & NCP_UNICODE)
    {
      LPWSTR lpDataWide;
      int    cbDataWide;

      const char* bom = "\xFF\xFE";
      int bomoffset = 0;

      SetEndOfFile(hFile);

       lpDataWide = GlobalAlloc(GPTR, cbData * 2 + 16);

      if (mEncoding[iEncoding].uFlags & NCP_UNICODE_BOM) {
        CopyMemory((char*)lpDataWide, bom, 2);
        bomoffset = 1;
      }
      cbDataWide = bomoffset + MultiByteToWideChar(Encoding_SciGetCodePage(hwnd), 0, lpData, cbData, &lpDataWide[bomoffset], (int)GlobalSize(lpDataWide) / sizeof(WCHAR) - bomoffset);
      if (mEncoding[iEncoding].uFlags & NCP_UNICODE_REVERSE) {
        _swab((char*)lpDataWide, (char*)lpDataWide, cbDataWide * sizeof(WCHAR));
      }
      bWriteSuccess = EncryptAndWriteFile(hwnd, hFile, (BYTE*)lpDataWide, cbDataWide * sizeof(WCHAR), &dwBytesWritten);
      dwLastIOError = GetLastError();

      GlobalFree(lpDataWide);
      GlobalFree(lpData);
    }

    else if (mEncoding[iEncoding].uFlags & NCP_UTF8)
    {
      SetEndOfFile(hFile);

      if (mEncoding[iEncoding].uFlags & NCP_UTF8_SIGN) {
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

    else if (mEncoding[iEncoding].uFlags & NCP_8BIT) {

      BOOL bCancelDataLoss = FALSE;
      UINT uCodePage = mEncoding[iEncoding].uCodePage;

      LPWSTR lpDataWide = GlobalAlloc(GPTR,cbData * 2 + 16);
      int    cbDataWide = MultiByteToWideChar(Encoding_SciGetCodePage(hwnd),0,lpData,cbData,lpDataWide,(int)GlobalSize(lpDataWide)/sizeof(WCHAR));

      if (mEncoding[iEncoding].uFlags & NCP_MBCS) {
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
  int cchTextW;
  int iCurPos;
  int iAnchorPos;
  UINT cpEdit;
  int i;
  BOOL bChanged = FALSE;

  iCurPos    = (int)SendMessage(hwnd,SCI_GETCURRENTPOS,0,0);
  iAnchorPos = (int)SendMessage(hwnd,SCI_GETANCHOR,0,0);

  if (iCurPos != iAnchorPos)
  {
    if (SC_SEL_RECTANGLE != SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0))
    {
      int iSelStart  = (int)SendMessage(hwnd,SCI_GETSELECTIONSTART,0,0);
      int iSelEnd    = (int)SendMessage(hwnd,SCI_GETSELECTIONEND,0,0);
      int iSelLength = (int)SendMessage(hwnd,SCI_GETSELTEXT,0,0);

      char*  pszText  = GlobalAlloc(GPTR,iSelLength);
      LPWSTR pszTextW = GlobalAlloc(GPTR,(iSelLength*sizeof(WCHAR)));

      if (pszText == NULL || pszTextW == NULL) {
        GlobalFree(pszText);
        GlobalFree(pszTextW);
        return;
      }

      SendMessage(hwnd,SCI_GETSELTEXT,0,(LPARAM)pszText);

      cpEdit = Encoding_SciGetCodePage(hwnd);
      cchTextW = MultiByteToWideChar(cpEdit,0,pszText,iSelLength,pszTextW,(int)GlobalSize(pszTextW)/sizeof(WCHAR));

      for (i = 0; i < cchTextW; i++) {
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

        SendMessage(hwnd,SCI_BEGINUNDOACTION,0,0);
        SendMessage(hwnd,SCI_CLEAR,0,0);
        SendMessage(hwnd,SCI_ADDTEXT,(WPARAM)(iSelEnd - iSelStart),(LPARAM)pszText);
        SendMessage(hwnd,SCI_SETSEL,(WPARAM)iAnchorPos,(LPARAM)iCurPos);
        SendMessage(hwnd,SCI_ENDUNDOACTION,0,0);
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
  int cchTextW;
  int iCurPos;
  int iAnchorPos;
  UINT cpEdit;

  iCurPos    = (int)SendMessage(hwnd,SCI_GETCURRENTPOS,0,0);
  iAnchorPos = (int)SendMessage(hwnd,SCI_GETANCHOR,0,0);

  if (iCurPos != iAnchorPos)
  {
    if (SC_SEL_RECTANGLE != SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0))
    {
      int iSelStart = (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
      int iSelEnd = (int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0);
      int iSelLength = (int)SendMessage(hwnd,SCI_GETSELTEXT,0,0);

      char*  pszText  = GlobalAlloc(GPTR,iSelLength);
      LPWSTR pszTextW = GlobalAlloc(GPTR,(iSelLength*sizeof(WCHAR)));

      if (pszText == NULL || pszTextW == NULL) {
        GlobalFree(pszText);
        GlobalFree(pszTextW);
        return;
      }

      SendMessage(hwnd,SCI_GETSELTEXT,0,(LPARAM)pszText);

      cpEdit = Encoding_SciGetCodePage(hwnd);
      cchTextW = MultiByteToWideChar(cpEdit,0,pszText,iSelLength,pszTextW,iSelLength);

      BOOL bChanged = FALSE;

      LPWSTR pszMappedW = LocalAlloc(LPTR,GlobalSize(pszTextW));
      // first make lower case, before applying TitleCase
      if (LCMapString(LOCALE_SYSTEM_DEFAULT,LCMAP_LINGUISTIC_CASING | LCMAP_LOWERCASE,
                      pszTextW,cchTextW,pszMappedW,iSelLength)) {
        if (LCMapString(LOCALE_SYSTEM_DEFAULT,LCMAP_TITLECASE,
                        pszMappedW,cchTextW,pszTextW,iSelLength)) {
          bChanged = TRUE;
        }
      }
      LocalFree(pszMappedW);

      if (bChanged) {

        WideCharToMultiByte(cpEdit,0,pszTextW,cchTextW,pszText,(int)GlobalSize(pszText),NULL,NULL);

        SendMessage(hwnd,SCI_BEGINUNDOACTION,0,0);
        SendMessage(hwnd,SCI_CLEAR,0,0);
        SendMessage(hwnd,SCI_ADDTEXT,(WPARAM)(iSelEnd - iSelStart),(LPARAM)pszText);
        SendMessage(hwnd,SCI_SETSEL,(WPARAM)iAnchorPos,(LPARAM)iCurPos);
        SendMessage(hwnd,SCI_ENDUNDOACTION,0,0);
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
  int cchTextW;
  int iCurPos;
  int iAnchorPos;
  UINT cpEdit;
  int i;
  BOOL bNewSentence = TRUE;
  BOOL bChanged = FALSE;

  iCurPos    = (int)SendMessage(hwnd,SCI_GETCURRENTPOS,0,0);
  iAnchorPos = (int)SendMessage(hwnd,SCI_GETANCHOR,0,0);

  if (iCurPos != iAnchorPos)
  {
    if (SC_SEL_RECTANGLE != SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0))
    {
      int iSelStart  = (int)SendMessage(hwnd,SCI_GETSELECTIONSTART,0,0);
      int iSelEnd    = (int)SendMessage(hwnd,SCI_GETSELECTIONEND,0,0);
      int iSelLength = (int)SendMessage(hwnd,SCI_GETSELTEXT,0,0);

      char*  pszText  = GlobalAlloc(GPTR,iSelLength);
      LPWSTR pszTextW = GlobalAlloc(GPTR,(iSelLength*sizeof(WCHAR)));

      if (pszText == NULL || pszTextW == NULL) {
        GlobalFree(pszText);
        GlobalFree(pszTextW);
        return;
      }

      SendMessage(hwnd,SCI_GETSELTEXT,0,(LPARAM)pszText);

      cpEdit = Encoding_SciGetCodePage(hwnd);
      cchTextW = MultiByteToWideChar(cpEdit,0,pszText,iSelLength,pszTextW,(int)GlobalSize(pszTextW)/sizeof(WCHAR));

      for (i = 0; i < cchTextW; i++) {
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

        SendMessage(hwnd,SCI_BEGINUNDOACTION,0,0);
        SendMessage(hwnd,SCI_CLEAR,0,0);
        SendMessage(hwnd,SCI_ADDTEXT,(WPARAM)(iSelEnd - iSelStart),(LPARAM)pszText);
        SendMessage(hwnd,SCI_SETSEL,(WPARAM)iAnchorPos,(LPARAM)iCurPos);
        SendMessage(hwnd,SCI_ENDUNDOACTION,0,0);
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
  int iCurPos;
  int iAnchorPos;
  UINT cpEdit;

  iCurPos    = (int)SendMessage(hwnd,SCI_GETCURRENTPOS,0,0);
  iAnchorPos = (int)SendMessage(hwnd,SCI_GETANCHOR,0,0);

  if (iCurPos != iAnchorPos)
  {
    if (SC_SEL_RECTANGLE != SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0))
    {
      char*  pszText;
      LPWSTR pszTextW;

      DWORD  cchEscaped;
      char*  pszEscaped;
      DWORD  cchEscapedW;
      LPWSTR pszEscapedW;

      int iSelLength = (int)SendMessage(hwnd, SCI_GETSELTEXT, 0, 0);

      pszText = LocalAlloc(LPTR,iSelLength);
      if (pszText == NULL) {
        return;
      }

      pszTextW = LocalAlloc(LPTR,(iSelLength*sizeof(WCHAR)));
      if (pszTextW == NULL) {
        LocalFree(pszText);
        return;
      }

      SendMessage(hwnd,SCI_GETSELTEXT,0,(LPARAM)pszText);
      cpEdit = Encoding_SciGetCodePage(hwnd);
      /*int cchTextW =*/ MultiByteToWideChar(cpEdit,0,pszText,iSelLength,pszTextW,(int)LocalSize(pszTextW)/sizeof(WCHAR));

      pszEscaped = LocalAlloc(LPTR,LocalSize(pszText) * 3);
      if (pszEscaped == NULL) {
        LocalFree(pszText);
        LocalFree(pszTextW);
        return;
      }

      pszEscapedW = LocalAlloc(LPTR,LocalSize(pszTextW) * 3);
      if (pszEscapedW == NULL) {
        LocalFree(pszText);
        LocalFree(pszTextW);
        LocalFree(pszEscaped);
        return;
      }

      cchEscapedW = (int)LocalSize(pszEscapedW) / sizeof(WCHAR);
      UrlEscape(pszTextW,pszEscapedW,&cchEscapedW,URL_ESCAPE_SEGMENT_ONLY);

      cchEscaped = WideCharToMultiByte(cpEdit,0,pszEscapedW,cchEscapedW,pszEscaped,(int)LocalSize(pszEscaped),NULL,NULL);

      if (iCurPos < iAnchorPos)
        iAnchorPos = iCurPos + cchEscaped;
      else
        iCurPos = iAnchorPos + cchEscaped;

      SendMessage(hwnd,SCI_BEGINUNDOACTION,0,0);
      SendMessage(hwnd,SCI_CLEAR,0,0);
      SendMessage(hwnd,SCI_ADDTEXT,(WPARAM)cchEscaped,(LPARAM)pszEscaped);
      SendMessage(hwnd,SCI_SETSEL,(WPARAM)iAnchorPos,(LPARAM)iCurPos);
      SendMessage(hwnd,SCI_ENDUNDOACTION,0,0);

      LocalFree(pszText);
      LocalFree(pszTextW);
      LocalFree(pszEscaped);
      LocalFree(pszEscapedW);
    }
    else
      MsgBox(MBWARN,IDS_SELRECT);
  }
}


//=============================================================================
//
//  EditURLDecode()
//
void EditURLDecode(HWND hwnd)
{
  int iCurPos;
  int iAnchorPos;
  UINT cpEdit;

  iCurPos    = (int)SendMessage(hwnd,SCI_GETCURRENTPOS,0,0);
  iAnchorPos = (int)SendMessage(hwnd,SCI_GETANCHOR,0,0);

  if (iCurPos != iAnchorPos)
  {
    if (SC_SEL_RECTANGLE != SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0))
    {
      char*  pszText;
      LPWSTR pszTextW;

      DWORD  cchUnescaped;
      char*  pszUnescaped;
      DWORD  cchUnescapedW;
      LPWSTR pszUnescapedW;

      int iSelLength = (int)SendMessage(hwnd,SCI_GETSELTEXT,0,0);

      pszText = LocalAlloc(LPTR,iSelLength);
      if (pszText == NULL) {
        return;
      }

      pszTextW = LocalAlloc(LPTR,(iSelLength*sizeof(WCHAR)));
      if (pszTextW == NULL) {
        LocalFree(pszText);
        return;
      }

      SendMessage(hwnd,SCI_GETSELTEXT,0,(LPARAM)pszText);
      cpEdit = Encoding_SciGetCodePage(hwnd);
      /*int cchTextW =*/ MultiByteToWideChar(cpEdit,0,pszText,iSelLength,pszTextW,(int)LocalSize(pszTextW)/sizeof(WCHAR));

      pszUnescaped = LocalAlloc(LPTR,LocalSize(pszText) * 3);
      if (pszUnescaped == NULL) {
        LocalFree(pszText);
        LocalFree(pszTextW);
        return;
      }

      pszUnescapedW = LocalAlloc(LPTR,LocalSize(pszTextW) * 3);
      if (pszUnescapedW == NULL) {
        LocalFree(pszText);
        LocalFree(pszTextW);
        LocalFree(pszUnescaped);
        return;
      }

      cchUnescapedW = (int)LocalSize(pszUnescapedW) / sizeof(WCHAR);
      UrlUnescape(pszTextW,pszUnescapedW,&cchUnescapedW,0);

      cchUnescaped = WideCharToMultiByte(cpEdit,0,pszUnescapedW,cchUnescapedW,pszUnescaped,(int)LocalSize(pszUnescaped),NULL,NULL);

      if (iCurPos < iAnchorPos)
        iAnchorPos = iCurPos + cchUnescaped;
      else
        iCurPos = iAnchorPos + cchUnescaped;

      SendMessage(hwnd,SCI_BEGINUNDOACTION,0,0);
      SendMessage(hwnd,SCI_CLEAR,0,0);
      SendMessage(hwnd,SCI_ADDTEXT,(WPARAM)cchUnescaped,(LPARAM)pszUnescaped);
      SendMessage(hwnd,SCI_SETSEL,(WPARAM)iAnchorPos,(LPARAM)iCurPos);
      SendMessage(hwnd,SCI_ENDUNDOACTION,0,0);

      LocalFree(pszText);
      LocalFree(pszTextW);
      LocalFree(pszUnescaped);
      LocalFree(pszUnescapedW);
    }
    else
      MsgBox(MBWARN,IDS_SELRECT);
  }
}


//=============================================================================
//
//  EditEscapeCChars()
//
void EditEscapeCChars(HWND hwnd) {

  if (SendMessage(hwnd,SCI_GETSELECTIONEND,0,0) - SendMessage(hwnd,SCI_GETSELECTIONSTART,0,0))
  {
    if (SC_SEL_RECTANGLE != SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0))
    {
      EDITFINDREPLACE efr = { "", "", "", "", 0, 0, 0, 0, 0, 0, 0, NULL };
      efr.hwnd = hwnd;

      SendMessage(hwnd,SCI_BEGINUNDOACTION,0,0);

      StringCchCopyA(efr.szFind,FNDRPL_BUFFER,"\\");
      StringCchCopyA(efr.szReplace,FNDRPL_BUFFER,"\\\\");
      EditReplaceAllInSelection(hwnd,&efr,FALSE);

      StringCchCopyA(efr.szFind,FNDRPL_BUFFER,"\"");
      StringCchCopyA(efr.szReplace,FNDRPL_BUFFER,"\\\"");
      EditReplaceAllInSelection(hwnd,&efr,FALSE);

      StringCchCopyA(efr.szFind,FNDRPL_BUFFER,"\'");
      StringCchCopyA(efr.szReplace,FNDRPL_BUFFER,"\\\'");
      EditReplaceAllInSelection(hwnd,&efr,FALSE);

      SendMessage(hwnd,SCI_ENDUNDOACTION,0,0);
    }
    else
      MsgBox(MBWARN,IDS_SELRECT);
  }
}


//=============================================================================
//
//  EditUnescapeCChars()
//
void EditUnescapeCChars(HWND hwnd) {

  if (SendMessage(hwnd,SCI_GETSELECTIONEND,0,0) - SendMessage(hwnd,SCI_GETSELECTIONSTART,0,0))
  {
    if (SC_SEL_RECTANGLE != SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0))
    {
      EDITFINDREPLACE efr = { "", "", "", "", 0, 0, 0, 0, 0, 0, 0, NULL };
      efr.hwnd = hwnd;

      SendMessage(hwnd,SCI_BEGINUNDOACTION,0,0);

      StringCchCopyA(efr.szFind,FNDRPL_BUFFER,"\\\\");
      StringCchCopyA(efr.szReplace,FNDRPL_BUFFER,"\\");
      EditReplaceAllInSelection(hwnd,&efr,FALSE);

      StringCchCopyA(efr.szFind,FNDRPL_BUFFER,"\\\"");
      StringCchCopyA(efr.szReplace,FNDRPL_BUFFER,"\"");
      EditReplaceAllInSelection(hwnd,&efr,FALSE);

      StringCchCopyA(efr.szFind,FNDRPL_BUFFER,"\\\'");
      StringCchCopyA(efr.szReplace,FNDRPL_BUFFER,"\'");
      EditReplaceAllInSelection(hwnd,&efr,FALSE);

      SendMessage(hwnd,SCI_ENDUNDOACTION,0,0);
    }
    else
      MsgBox(MBWARN,IDS_SELRECT);
  }
}


//=============================================================================
//
//  EditChar2Hex()
//
void EditChar2Hex(HWND hwnd) {

  if (SC_SEL_RECTANGLE != SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0)) {

    int iSelStart = (int)SendMessage(hwnd,SCI_GETSELECTIONSTART,0,0);
    int iSelEnd = (int)SendMessage(hwnd,SCI_GETSELECTIONEND,0,0);

    if (SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iSelStart,0) ==
        SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iSelEnd,0) &&
        iSelEnd == (int)SendMessage(hwnd,SCI_POSITIONAFTER,(WPARAM)iSelStart,0)) {

      char  ch[32] = { '\0' };
      WCHAR wch[32] = { L'\0' };

      SendMessage(hwnd,SCI_GETSELTEXT,0,(LPARAM)ch);

      if (ch[0] == 0)
        StringCchCopyA(ch,COUNTOF(ch),"\\x00");

      else {
        UINT cp = Encoding_SciGetCodePage(hwnd);
        MultiByteToWideCharStrg(cp,ch,wch);
        if (wch[0] <= 0xFF)
          StringCchPrintfA(ch,COUNTOF(ch),"\\x%02X",wch[0] & 0xFF);
        else
          StringCchPrintfA(ch,COUNTOF(ch),"\\u%04X",wch[0]);
      }

      SendMessage(hwnd,SCI_REPLACESEL,0,(LPARAM)ch);
      SendMessage(hwnd,SCI_SETSEL,iSelStart,iSelStart+ StringCchLenA(ch));
    }
  }
  else
    MsgBox(MBWARN,IDS_SELRECT);
}


//=============================================================================
//
//  EditHex2Char()
//
void EditHex2Char(HWND hwnd) {

  if (SC_SEL_RECTANGLE != SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0)) {

    char ch[32] = { L'\0' };
    int  i;
    BOOL bTrySelExpand = FALSE;

    int iSelStart = (int)SendMessage(hwnd,SCI_GETSELECTIONSTART,0,0);
    int iSelEnd   = (int)SendMessage(hwnd,SCI_GETSELECTIONEND,0,0);

    if (iSelEnd - iSelStart) {

      if (SendMessage(hwnd,SCI_GETSELTEXT,0,0) <= COUNTOF(ch)) {

        SendMessage(hwnd,SCI_GETSELTEXT,0,(LPARAM)ch);
        ch[31] = '\0';

        if (StrChrIA(ch,' ') || StrChrIA(ch,'\t') || StrChrIA(ch,'\r') || StrChrIA(ch,'\n') || StrChrIA(ch,'-'))
          return;

        if (StrCmpNIA(ch,"\\x",2) == 0 || StrCmpNIA(ch,"\\u",2) == 0) {
          ch[0] = '0';
          ch[1] = 'x';
        }

        else if (StrChrIA("xu",ch[0])) {
          ch[0] = '0';
          bTrySelExpand = TRUE;
        }

        if (sscanf_s(ch,"%x",&i) == 1) {
          int cch;
          if (i == 0) {
            ch[0] = 0;
            cch = 1;
          }
          else {
            UINT  cp = Encoding_SciGetCodePage(hwnd);
            WCHAR wch[4];
            StringCchPrintf(wch,COUNTOF(wch),L"%lc",(WCHAR)i);
            cch = WideCharToMultiByteStrg(cp,wch,ch) - 1;
            if (bTrySelExpand && (char)SendMessage(hwnd,SCI_GETCHARAT,(WPARAM)iSelStart-1,0) == '\\') {
              iSelStart--;
            }
          }
          SendMessage(hwnd,SCI_SETTARGETSTART,(WPARAM)iSelStart,0);
          SendMessage(hwnd,SCI_SETTARGETEND,(WPARAM)iSelEnd,0);
          SendMessage(hwnd,SCI_REPLACETARGET,(WPARAM)cch,(LPARAM)ch);
          SendMessage(hwnd,SCI_SETSEL,iSelStart,iSelStart+cch);
        }
      }
    }
  }
  else
    MsgBox(MBWARN,IDS_SELRECT);
}


//=============================================================================
//
//  EditModifyNumber()
//
void EditModifyNumber(HWND hwnd,BOOL bIncrease) {

  if (SC_SEL_RECTANGLE != SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0)) {

    int iSelStart = (int)SendMessage(hwnd,SCI_GETSELECTIONSTART,0,0);
    int iSelEnd   = (int)SendMessage(hwnd,SCI_GETSELECTIONEND,0,0);

    if (iSelEnd - iSelStart) {

      char chFormat[32] = { '\0' };
      char chNumber[32] = { '\0' };
      int  iNumber;
      int  iWidth;

      if (SendMessage(hwnd,SCI_GETSELTEXT,0,0) <= COUNTOF(chNumber)) {
        SendMessage(hwnd,SCI_GETSELTEXT,0,(LPARAM)chNumber);
        chNumber[31] = '\0';

        if (StrChrIA(chNumber,'-'))
          return;

        if (!StrChrIA(chNumber, 'x') && sscanf_s(chNumber, "%d", &iNumber) == 1) {
          iWidth = StringCchLenA(chNumber);
          if (iNumber >= 0) {
            if (bIncrease && iNumber < INT_MAX)
              iNumber++;
            if (!bIncrease && iNumber > 0)
              iNumber--;
            StringCchPrintfA(chFormat,COUNTOF(chFormat),"%%0%ii",iWidth);
            StringCchPrintfA(chNumber,COUNTOF(chNumber),chFormat,iNumber);
            SendMessage(hwnd,SCI_REPLACESEL,0,(LPARAM)chNumber);
            SendMessage(hwnd,SCI_SETSEL,iSelStart,iSelStart+StringCchLenA(chNumber));
          }
        }
        else if (sscanf_s(chNumber, "%x", &iNumber) == 1) {
          int i;
          BOOL bUppercase = FALSE;
          iWidth = StringCchLenA(chNumber) - 2;
          if (iNumber >= 0) {
            if (bIncrease && iNumber < INT_MAX)
              iNumber++;
            if (!bIncrease && iNumber > 0)
              iNumber--;
            for (i = StringCchLenA(chNumber) -1 ; i >= 0; i--) {
              if (IsCharLowerA(chNumber[i]))
                break;
              else if (IsCharUpper(chNumber[i])) {
                bUppercase = TRUE;
                break;
              }
            }
            if (bUppercase)
              StringCchPrintfA(chFormat,COUNTOF(chFormat),"%%#0%iX",iWidth);
            else
              StringCchPrintfA(chFormat,COUNTOF(chFormat),"%%#0%ix",iWidth);
            StringCchPrintfA(chNumber,COUNTOF(chNumber),chFormat,iNumber);
            SendMessage(hwnd,SCI_REPLACESEL,0,(LPARAM)chNumber);
            SendMessage(hwnd,SCI_SETSEL,iSelStart,iSelStart+StringCchLenA(chNumber));
          }
        }
      }
    }
  }
  else
    MsgBox(MBWARN,IDS_SELRECT);
}


//=============================================================================
//
//  EditTabsToSpaces()
//
void EditTabsToSpaces(HWND hwnd,int nTabWidth,BOOL bOnlyIndentingWS)
{
  char* pszText;
  LPWSTR pszTextW;
  int cchTextW;
  int iTextW;
  LPWSTR pszConvW;
  int cchConvW;
  int cchConvM;
  int i,j;
  int iLine;
  int iCurPos;
  int iAnchorPos;
  int iSelStart;
  int iSelEnd;
  int iSelCount;
  UINT cpEdit;
  struct Sci_TextRange tr;
  BOOL bIsLineStart = TRUE;
  BOOL bModified = FALSE;

  if (SC_SEL_RECTANGLE == SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0)) {
    MsgBox(MBWARN,IDS_SELRECT);
    return;
  }

  iCurPos    = (int)SendMessage(hwnd,SCI_GETCURRENTPOS,0,0);
  iAnchorPos = (int)SendMessage(hwnd,SCI_GETANCHOR,0,0);

  if (iCurPos == iAnchorPos)
    return;

  iSelStart = (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
  iLine = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iSelStart, 0);
  iSelStart = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLine, 0);   // rebase selection to start of line
  iSelEnd = (int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0);
  iSelCount = iSelEnd - iSelStart;

  pszText = GlobalAlloc(GPTR, iSelCount + 2);
  if (pszText == NULL)
    return;

  pszTextW = GlobalAlloc(GPTR, (iSelCount + 2) * sizeof(WCHAR));
  if (pszTextW == NULL)
  {
    GlobalFree(pszText);
    return;
  }

  tr.chrg.cpMin = iSelStart;
  tr.chrg.cpMax = iSelEnd;
  tr.lpstrText = pszText;
  SendMessage(hwnd,SCI_GETTEXTRANGE,0,(LPARAM)&tr);

  cpEdit = Encoding_SciGetCodePage(hwnd);
  cchTextW = MultiByteToWideChar(cpEdit,0,pszText,iSelCount,pszTextW,(int)GlobalSize(pszTextW)/sizeof(WCHAR));
  GlobalFree(pszText);

  pszConvW = GlobalAlloc(GPTR,cchTextW*sizeof(WCHAR)*nTabWidth+2);
  if (pszConvW == NULL) {
    GlobalFree(pszTextW);
    return;
  }

  cchConvW = 0;

  // Contributed by Homam
  // Thank you very much!
  i=0;
  for (iTextW = 0; iTextW < cchTextW; iTextW++)
  {
    WCHAR w = pszTextW[iTextW];
    if (w == L'\t' && (!bOnlyIndentingWS || bIsLineStart)) {
      for (j = 0; j < nTabWidth - i % nTabWidth; j++)
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
    pszText = GlobalAlloc(GPTR,cchConvW * 3);

    cchConvM = WideCharToMultiByte(cpEdit,0,pszConvW,cchConvW,pszText,(int)GlobalSize(pszText),NULL,NULL);
    GlobalFree(pszConvW);

    if (iAnchorPos > iCurPos) {
      iCurPos = iSelStart;
      iAnchorPos = iSelStart + cchConvM;
    }
    else {
      iAnchorPos = iSelStart;
      iCurPos = iSelStart + cchConvM;
    }

    SendMessage(hwnd,SCI_BEGINUNDOACTION,0,0);
    SendMessage(hwnd,SCI_SETTARGETSTART,(WPARAM)iSelStart,0);
    SendMessage(hwnd,SCI_SETTARGETEND,(WPARAM)iSelEnd,0);
    SendMessage(hwnd,SCI_REPLACETARGET,(WPARAM)cchConvM,(LPARAM)pszText);
    //SendMessage(hwnd,SCI_CLEAR,0,0);
    //SendMessage(hwnd,SCI_ADDTEXT,(WPARAM)cchConvW,(LPARAM)pszText);
    SendMessage(hwnd,SCI_SETSEL,(WPARAM)iAnchorPos,(LPARAM)iCurPos);
    SendMessage(hwnd,SCI_ENDUNDOACTION,0,0);

    GlobalFree(pszText);
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
  char* pszText;
  LPWSTR pszTextW;
  int cchTextW;
  int iTextW;
  LPWSTR pszConvW;
  int cchConvW;
  int cchConvM;
  int i,j,t;
  int iLine;
  int iCurPos;
  int iAnchorPos;
  int iSelStart;
  int iSelEnd;
  int iSelCount;
  UINT cpEdit;
  struct Sci_TextRange tr;
  WCHAR space[256] = { L'\0' };
  BOOL bIsLineStart = TRUE;
  BOOL bModified = FALSE;

  if (SC_SEL_RECTANGLE == SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0)) {
    MsgBox(MBWARN,IDS_SELRECT);
    return;
  }

  iCurPos    = (int)SendMessage(hwnd,SCI_GETCURRENTPOS,0,0);
  iAnchorPos = (int)SendMessage(hwnd,SCI_GETANCHOR,0,0);

  if (iCurPos == iAnchorPos)
    return;

  iSelStart = (int)SendMessage(hwnd, SCI_GETSELECTIONSTART, 0, 0);
  iLine = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, (WPARAM)iSelStart, 0);
  iSelStart = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLine, 0);  // rebase selection to start of line
  iSelEnd = (int)SendMessage(hwnd, SCI_GETSELECTIONEND, 0, 0);
  iSelCount = iSelEnd - iSelStart;

  pszText = GlobalAlloc(GPTR, iSelCount + 2);
  if (pszText == NULL)
    return;

  pszTextW = GlobalAlloc(GPTR, (iSelCount + 2) * sizeof(WCHAR));
  if (pszTextW == NULL)
  {
    GlobalFree(pszText);
    return;
  }

  tr.chrg.cpMin = iSelStart;
  tr.chrg.cpMax = iSelEnd;
  tr.lpstrText = pszText;
  SendMessage(hwnd,SCI_GETTEXTRANGE,0,(LPARAM)&tr);

  cpEdit = Encoding_SciGetCodePage(hwnd);
  cchTextW = MultiByteToWideChar(cpEdit,0,pszText,iSelCount,pszTextW,(int)GlobalSize(pszTextW)/sizeof(WCHAR));
  GlobalFree(pszText);

  pszConvW = GlobalAlloc(GPTR,cchTextW*sizeof(WCHAR)+2);
  if (pszConvW == NULL) {
    GlobalFree(pszTextW);
    return;
  }

  cchConvW = 0;

  // Contributed by Homam
  // Thank you very much!
  i = j = 0;
  for (iTextW = 0; iTextW < cchTextW; iTextW++)
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
        for (t = 0; t < j; t++)
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
    for (t = 0; t < j; t++)
      pszConvW[cchConvW++] = space[t];
    }

  GlobalFree(pszTextW);

  if (bModified || cchConvW != cchTextW) {
    pszText = GlobalAlloc(GPTR,cchConvW * 3);

    cchConvM = WideCharToMultiByte(cpEdit,0,pszConvW,cchConvW,pszText,(int)GlobalSize(pszText),NULL,NULL);
    GlobalFree(pszConvW);

    if (iAnchorPos > iCurPos) {
      iCurPos = iSelStart;
      iAnchorPos = iSelStart + cchConvM;
    }
    else {
      iAnchorPos = iSelStart;
      iCurPos = iSelStart + cchConvM;
    }

    SendMessage(hwnd,SCI_BEGINUNDOACTION,0,0);
    SendMessage(hwnd,SCI_SETTARGETSTART,(WPARAM)iSelStart,0);
    SendMessage(hwnd,SCI_SETTARGETEND,(WPARAM)iSelEnd,0);
    SendMessage(hwnd,SCI_REPLACETARGET,(WPARAM)cchConvM,(LPARAM)pszText);
    //SendMessage(hwnd,SCI_CLEAR,0,0);
    //SendMessage(hwnd,SCI_ADDTEXT,(WPARAM)cchConvW,(LPARAM)pszText);
    SendMessage(hwnd,SCI_SETSEL,(WPARAM)iAnchorPos,(LPARAM)iCurPos);
    SendMessage(hwnd,SCI_ENDUNDOACTION,0,0);

    GlobalFree(pszText);
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

  int iCurPos = (int)SendMessage(hwnd,SCI_GETCURRENTPOS,0,0);
  int iAnchorPos = (int)SendMessage(hwnd,SCI_GETANCHOR,0,0);
  int iCurLine = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iCurPos,0);
  int iAnchorLine = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iAnchorPos,0);

  if (iCurLine == iAnchorLine) {

    int iLineCurPos = iCurPos - (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iCurLine,0);
    int iLineAnchorPos = iAnchorPos - (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iAnchorLine,0);
    if (iCurLine > 0) {

      SendMessage(hwnd,SCI_BEGINUNDOACTION,0,0);
      SendMessage(hwnd,SCI_LINETRANSPOSE,0,0);
      SendMessage(hwnd,SCI_SETSEL,
        (WPARAM)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iAnchorLine-1,0)+iLineAnchorPos,
        (LPARAM)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iCurLine-1,0)+iLineCurPos);
      SendMessage(hwnd,SCI_CHOOSECARETX,0,0);
      SendMessage(hwnd,SCI_ENDUNDOACTION,0,0);
    }
  }
  else if (SC_SEL_RECTANGLE != SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0)) {

    int iLineSrc = min(iCurLine,iAnchorLine) -1;
    if (iLineSrc >= 0) {

      DWORD cLine;
      char *pLine;
      int iLineSrcStart;
      int iLineSrcEnd;
      int iLineDest;
      int iLineDestStart;

      cLine = (int)SendMessage(hwnd,SCI_GETLINE,(WPARAM)iLineSrc,0);
      pLine = LocalAlloc(LPTR,cLine+1);
      SendMessage(hwnd,SCI_GETLINE,(WPARAM)iLineSrc,(LPARAM)pLine);

      iLineSrcStart = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineSrc,0);
      iLineSrcEnd = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineSrc+1,0);

      iLineDest = max(iCurLine,iAnchorLine);
      if (max(iCurPos,iAnchorPos) <= SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineDest,0)) {
        if (iLineDest >= 1)
          iLineDest--;
      }

      SendMessage(hwnd,SCI_BEGINUNDOACTION,0,0);

      SendMessage(hwnd,SCI_SETTARGETSTART,(WPARAM)iLineSrcStart,0);
      SendMessage(hwnd,SCI_SETTARGETEND,(WPARAM)iLineSrcEnd,0);
      SendMessage(hwnd,SCI_REPLACETARGET,0,(LPARAM)"");

      iLineDestStart = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineDest,0);

      SendMessage(hwnd,SCI_INSERTTEXT,(WPARAM)iLineDestStart,(LPARAM)pLine);

      LocalFree(pLine);

      if (iLineDest == SendMessage(hwnd,SCI_GETLINECOUNT,0,0) -1) {

        char chaEOL[] = "\r\n";
        int iEOLMode = (int)SendMessage(hwnd,SCI_GETEOLMODE,0,0);
        if (iEOLMode == SC_EOL_CR)
          chaEOL[1] = 0;
        else if (iEOLMode == SC_EOL_LF) {
          chaEOL[0] = '\n';
          chaEOL[1] = 0;
        }

        SendMessage(hwnd,SCI_INSERTTEXT,(WPARAM)iLineDestStart,(LPARAM)chaEOL);
        SendMessage(hwnd,SCI_SETTARGETSTART,(WPARAM)
          SendMessage(hwnd,SCI_GETLINEENDPOSITION,(WPARAM)iLineDest,0),0);
        SendMessage(hwnd,SCI_SETTARGETEND,(WPARAM)
          SendMessage(hwnd,SCI_GETLENGTH,0,0),0);
        SendMessage(hwnd,SCI_REPLACETARGET,0,(LPARAM)"");
      }

      if (iCurPos < iAnchorPos) {
        iCurPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iCurLine-1,0);
        iAnchorPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineDest,0);
      }
      else {
        iAnchorPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iAnchorLine-1,0);
        iCurPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineDest,0);
      }
      SendMessage(hwnd,SCI_SETSEL,(WPARAM)iAnchorPos,(LPARAM)iCurPos);

      SendMessage(hwnd,SCI_ENDUNDOACTION,0,0);
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

  int iCurPos = (int)SendMessage(hwnd,SCI_GETCURRENTPOS,0,0);
  int iAnchorPos = (int)SendMessage(hwnd,SCI_GETANCHOR,0,0);
  int iCurLine = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iCurPos,0);
  int iAnchorLine = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iAnchorPos,0);

  if (iCurLine == iAnchorLine) {

    int iLineCurPos = iCurPos - (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iCurLine,0);
    int iLineAnchorPos = iAnchorPos - (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iAnchorLine,0);
    if (iCurLine < SendMessage(hwnd,SCI_GETLINECOUNT,0,0) - 1) {
      SendMessage(hwnd,SCI_BEGINUNDOACTION,0,0);
      SendMessage(hwnd,SCI_GOTOLINE,(WPARAM)iCurLine+1,0);
      SendMessage(hwnd,SCI_LINETRANSPOSE,0,0);
      SendMessage(hwnd,SCI_SETSEL,
        (WPARAM)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iAnchorLine+1,0)+iLineAnchorPos,
        (LPARAM)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iCurLine+1,0)+iLineCurPos);
      SendMessage(hwnd,SCI_CHOOSECARETX,0,0);
      SendMessage(hwnd,SCI_ENDUNDOACTION,0,0);
    }
  }
  else if (SC_SEL_RECTANGLE != SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0)) {

    int iLineSrc = max(iCurLine,iAnchorLine) +1;
    if (max(iCurPos,iAnchorPos) <= SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineSrc-1,0)) {
      if (iLineSrc >= 1)
        iLineSrc--;
    }

    if (iLineSrc <= SendMessage(hwnd,SCI_GETLINECOUNT,0,0) -1) {

      DWORD cLine;
      char *pLine;
      int iLineSrcStart;
      int iLineSrcEnd;
      int iLineDest;
      int iLineDestStart;

      BOOL bLastLine = (iLineSrc == SendMessage(hwnd,SCI_GETLINECOUNT,0,0) -1);

      if (bLastLine &&
        (SendMessage(hwnd,SCI_GETLINEENDPOSITION,(WPARAM)iLineSrc,0) -
        SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineSrc,0) == 0) &&
        (SendMessage(hwnd,SCI_GETLINEENDPOSITION,(WPARAM)iLineSrc-1,0) -
        SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineSrc-1,0) == 0))
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

      cLine = (int)SendMessage(hwnd,SCI_GETLINE,(WPARAM)iLineSrc,0);
      pLine = LocalAlloc(LPTR,cLine+3);
      SendMessage(hwnd,SCI_GETLINE,(WPARAM)iLineSrc,(LPARAM)pLine);

      iLineSrcStart = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineSrc,0);
      iLineSrcEnd = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineSrc+1,0);

      iLineDest = min(iCurLine,iAnchorLine);

      SendMessage(hwnd,SCI_BEGINUNDOACTION,0,0);

      SendMessage(hwnd,SCI_SETTARGETSTART,(WPARAM)iLineSrcStart,0);
      SendMessage(hwnd,SCI_SETTARGETEND,(WPARAM)iLineSrcEnd,0);
      SendMessage(hwnd,SCI_REPLACETARGET,0,(LPARAM)"");

      iLineDestStart = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineDest,0);

      SendMessage(hwnd,SCI_INSERTTEXT,(WPARAM)iLineDestStart,(LPARAM)pLine);

      if (bLastLine) {
        SendMessage(hwnd,SCI_SETTARGETSTART,(WPARAM)
          SendMessage(hwnd,SCI_GETLINEENDPOSITION,(WPARAM)
            SendMessage(hwnd,SCI_GETLINECOUNT,0,0)-2,0),0);
        SendMessage(hwnd,SCI_SETTARGETEND,(WPARAM)
          SendMessage(hwnd,SCI_GETLENGTH,0,0),0);
        SendMessage(hwnd,SCI_REPLACETARGET,0,(LPARAM)"");
      }

      LocalFree(pLine);

      if (iCurPos < iAnchorPos) {
        iCurPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iCurLine+1,0);
        iAnchorPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineSrc+1,0);
      }
      else {
        iAnchorPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iAnchorLine+1,0);
        iCurPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineSrc+1,0);
      }
      SendMessage(hwnd,SCI_SETSEL,(WPARAM)iAnchorPos,(LPARAM)iCurPos);

      SendMessage(hwnd,SCI_ENDUNDOACTION,0,0);
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
  char  mszPrefix1[256*3] = { '\0' };
  char  mszPrefix2[256*3] = { '\0' };
  BOOL  bPrefixNum = FALSE;
  int   iPrefixNum = 0;
  int   iPrefixNumWidth = 1;
  char *pszPrefixNumPad = "";
  char  mszAppend1[256*3] = { '\0' };
  char  mszAppend2[256*3] = { '\0' };
  BOOL  bAppendNum = FALSE;
  int   iAppendNum = 0;
  int   iAppendNumWidth = 1;
  char *pszAppendNumPad = "";

  int iSelStart = (int)SendMessage(hwnd,SCI_GETSELECTIONSTART,0,0);
  int iSelEnd   = (int)SendMessage(hwnd,SCI_GETSELECTIONEND,0,0);

  //if (iSelStart == iSelEnd) {
  //  iSelStart = 0;
  //  iSelEnd   = SendMessage(hwnd,SCI_GETLENGTH,0,0);
  //}

  UINT mbcp = Encoding_SciGetCodePage(hwnd);

  if (lstrlen(pwszPrefix))
    WideCharToMultiByteStrg(mbcp,pwszPrefix,mszPrefix1);
  if (lstrlen(pwszAppend))
    WideCharToMultiByteStrg(mbcp,pwszAppend,mszAppend1);

  if (SC_SEL_RECTANGLE != SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0))
  {
    char *p;
    int  i;

    int iLine;

    int iLineStart = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iSelStart,0);
    int iLineEnd   = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iSelEnd,0);

    //if (iSelStart > SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineStart,0))
    //  iLineStart++;

    if (iSelEnd <= SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineEnd,0))
    {
      if (iLineEnd - iLineStart >= 1)
        iLineEnd--;
    }

    if (StringCchLenA(mszPrefix1)) {

      p = StrStrA(mszPrefix1, "$(");
      while (!bPrefixNum && p) {

        if (StrCmpNA(p,"$(I)",CSTRLEN("$(I)")) == 0) {
          *p = 0;
          StringCchCopyA(mszPrefix2,COUNTOF(mszPrefix2),p + CSTRLEN("$(I)"));
          bPrefixNum = TRUE;
          iPrefixNum = 0;
          for (i = iLineEnd - iLineStart; i >= 10; i = i / 10)
            iPrefixNumWidth++;
          pszPrefixNumPad = "";
        }

        else if (StrCmpNA(p,"$(0I)",CSTRLEN("$(0I)")) == 0) {
          *p = 0;
          StringCchCopyA(mszPrefix2,COUNTOF(mszPrefix2),p + CSTRLEN("$(0I)"));
          bPrefixNum = TRUE;
          iPrefixNum = 0;
          for (i = iLineEnd - iLineStart; i >= 10; i = i / 10)
            iPrefixNumWidth++;
          pszPrefixNumPad = "0";
        }

        else if (StrCmpNA(p,"$(N)",CSTRLEN("$(N)")) == 0) {
          *p = 0;
          StringCchCopyA(mszPrefix2,COUNTOF(mszPrefix2),p + CSTRLEN("$(N)"));
          bPrefixNum = TRUE;
          iPrefixNum = 1;
          for (i = iLineEnd - iLineStart + 1; i >= 10; i = i / 10)
            iPrefixNumWidth++;
          pszPrefixNumPad = "";
        }

        else if (StrCmpNA(p,"$(0N)",CSTRLEN("$(0N)")) == 0) {
          *p = 0;
          StringCchCopyA(mszPrefix2,COUNTOF(mszPrefix2),p + CSTRLEN("$(0N)"));
          bPrefixNum = TRUE;
          iPrefixNum = 1;
          for (i = iLineEnd - iLineStart + 1; i >= 10; i = i / 10)
            iPrefixNumWidth++;
          pszPrefixNumPad = "0";
        }

        else if (StrCmpNA(p,"$(L)",CSTRLEN("$(L)")) == 0) {
          *p = 0;
          StringCchCopyA(mszPrefix2,COUNTOF(mszPrefix2),p + CSTRLEN("$(L)"));
          bPrefixNum = TRUE;
          iPrefixNum = iLineStart+1;
          for (i = iLineEnd + 1; i >= 10; i = i / 10)
            iPrefixNumWidth++;
          pszPrefixNumPad = "";
        }

        else if (StrCmpNA(p,"$(0L)",CSTRLEN("$(0L)")) == 0) {
          *p = 0;
          StringCchCopyA(mszPrefix2,COUNTOF(mszPrefix2),p + CSTRLEN("$(0L)"));
          bPrefixNum = TRUE;
          iPrefixNum = iLineStart+1;
          for (i = iLineEnd + 1; i >= 10; i = i / 10)
            iPrefixNumWidth++;
          pszPrefixNumPad = "0";
        }
        p += CSTRLEN("$(");
        p = StrStrA(p, "$("); // next
      }
    }

    if (StringCchLenA(mszAppend1)) {

      p = StrStrA(mszAppend1, "$(");
      while (!bAppendNum && p) {

        if (StrCmpNA(p,"$(I)",CSTRLEN("$(I)")) == 0) {
          *p = 0;
          StringCchCopyA(mszAppend2,COUNTOF(mszAppend2),p + CSTRLEN("$(I)"));
          bAppendNum = TRUE;
          iAppendNum = 0;
          for (i = iLineEnd - iLineStart; i >= 10; i = i / 10)
            iAppendNumWidth++;
          pszAppendNumPad = "";
        }

        else if (StrCmpNA(p,"$(0I)",CSTRLEN("$(0I)")) == 0) {
          *p = 0;
          StringCchCopyA(mszAppend2,COUNTOF(mszAppend2),p + CSTRLEN("$(0I)"));
          bAppendNum = TRUE;
          iAppendNum = 0;
          for (i = iLineEnd - iLineStart; i >= 10; i = i / 10)
            iAppendNumWidth++;
          pszAppendNumPad = "0";
        }

        else if (StrCmpNA(p,"$(N)",CSTRLEN("$(N)")) == 0) {
          *p = 0;
          StringCchCopyA(mszAppend2,COUNTOF(mszAppend2),p + CSTRLEN("$(N)"));
          bAppendNum = TRUE;
          iAppendNum = 1;
          for (i = iLineEnd - iLineStart + 1; i >= 10; i = i / 10)
            iAppendNumWidth++;
          pszAppendNumPad = "";
        }

        else if (StrCmpNA(p,"$(0N)",CSTRLEN("$(0N)")) == 0) {
          *p = 0;
          StringCchCopyA(mszAppend2,COUNTOF(mszAppend2),p + CSTRLEN("$(0N)"));
          bAppendNum = TRUE;
          iAppendNum = 1;
          for (i = iLineEnd - iLineStart + 1; i >= 10; i = i / 10)
            iAppendNumWidth++;
          pszAppendNumPad = "0";
        }

        else if (StrCmpNA(p,"$(L)",CSTRLEN("$(L)")) == 0) {
          *p = 0;
          StringCchCopyA(mszAppend2,COUNTOF(mszAppend2),p + CSTRLEN("$(L)"));
          bAppendNum = TRUE;
          iAppendNum = iLineStart+1;
          for (i = iLineEnd + 1; i >= 10; i = i / 10)
            iAppendNumWidth++;
          pszAppendNumPad = "";
        }

        else if (StrCmpNA(p,"$(0L)",CSTRLEN("$(0L)")) == 0) {
          *p = 0;
          StringCchCopyA(mszAppend2,COUNTOF(mszAppend2),p + CSTRLEN("$(0L)"));
          bAppendNum = TRUE;
          iAppendNum = iLineStart+1;
          for (i = iLineEnd + 1; i >= 10; i = i / 10)
            iAppendNumWidth++;
          pszAppendNumPad = "0";
        }
        p += CSTRLEN("$(");
        p = StrStrA(p, "$("); // next
      }
    }

    SendMessage(hwnd,SCI_BEGINUNDOACTION,0,0);

    for (iLine = iLineStart; iLine <= iLineEnd; iLine++)
    {
      int iPos;

      if (lstrlen(pwszPrefix)) {

        char mszInsert[512*3];
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
        iPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLine,0);
        SendMessage(hwnd,SCI_SETTARGETSTART,(WPARAM)iPos,0);
        SendMessage(hwnd,SCI_SETTARGETEND,(WPARAM)iPos,0);
        SendMessage(hwnd,SCI_REPLACETARGET,(WPARAM)StringCchLenA(mszInsert),(LPARAM)mszInsert);
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
        iPos = (int)SendMessage(hwnd,SCI_GETLINEENDPOSITION,(WPARAM)iLine,0);
        SendMessage(hwnd,SCI_SETTARGETSTART,(WPARAM)iPos,0);
        SendMessage(hwnd,SCI_SETTARGETEND,(WPARAM)iPos,0);
        SendMessage(hwnd,SCI_REPLACETARGET,(WPARAM)StringCchLenA(mszInsert),(LPARAM)mszInsert);
      }
    }
    SendMessage(hwnd,SCI_ENDUNDOACTION,0,0);

    //// Fix selection
    //if (iSelStart != iSelEnd && SendMessage(hwnd,SCI_GETTARGETEND,0,0) > SendMessage(hwnd,SCI_GETSELECTIONEND,0,0))
    //{
    //  int iCurPos = SendMessage(hwnd,SCI_GETCURRENTPOS,0,0);
    //  int iAnchorPos = SendMessage(hwnd,SCI_GETANCHOR,0,0);
    //  if (iCurPos > iAnchorPos)
    //    iCurPos = SendMessage(hwnd,SCI_GETTARGETEND,0,0);
    //  else
    //    iAnchorPos = SendMessage(hwnd,SCI_GETTARGETEND,0,0);
    //  SendMessage(hwnd,SCI_SETSEL,(WPARAM)iAnchorPos,(LPARAM)iCurPos);
    //}

    // extend selection to start of first line
    // the above code is not required when last line has been excluded
    if (iSelStart != iSelEnd)
    {
      int iCurPos = (int)SendMessage(hwnd,SCI_GETCURRENTPOS,0,0);
      int iAnchorPos = (int)SendMessage(hwnd,SCI_GETANCHOR,0,0);
      if (iCurPos < iAnchorPos) {
        iCurPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineStart,0);
        iAnchorPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineEnd+1,0);
      }
      else {
        iAnchorPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineStart,0);
        iCurPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineEnd+1,0);
      }
      SendMessage(hwnd,SCI_SETSEL,(WPARAM)iAnchorPos,(LPARAM)iCurPos);
    }

  }
  else
    MsgBox(MBWARN,IDS_SELRECT);
}


//=============================================================================
//
//  EditAlignText()
//
void EditAlignText(HWND hwnd,int nMode)
{
  #define BUFSIZE_ALIGN 1024

  BOOL  bModified = FALSE;

  int iSelStart  = (int)SendMessage(hwnd,SCI_GETSELECTIONSTART,0,0);
  int iSelEnd    = (int)SendMessage(hwnd,SCI_GETSELECTIONEND,0,0);
  int iCurPos    = (int)SendMessage(hwnd,SCI_GETCURRENTPOS,0,0);
  int iAnchorPos = (int)SendMessage(hwnd,SCI_GETANCHOR,0,0);

  if (SC_SEL_RECTANGLE != SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0))
  {
    int iLine;
    int iMinIndent = BUFSIZE_ALIGN;
    int iMaxLength = 0;

    int iLineStart = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iSelStart,0);
    int iLineEnd   = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iSelEnd,0);

    if (iSelEnd <= SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineEnd,0))
    {
      if (iLineEnd - iLineStart >= 1)
        iLineEnd--;
    }

    for (iLine = iLineStart; iLine <= iLineEnd; iLine++) {

      int iLineEndPos    = (int)SendMessage(hwnd,SCI_GETLINEENDPOSITION,(WPARAM)iLine,0);
      int iLineIndentPos = (int)SendMessage(hwnd,SCI_GETLINEINDENTPOSITION,(WPARAM)iLine,0);

      if (iLineIndentPos != iLineEndPos) {

        int iIndentCol = (int)SendMessage(hwnd,SCI_GETLINEINDENTATION,(WPARAM)iLine,0);
        int iEndCol;
        char ch;
        int iTail;

        iTail = iLineEndPos-1;
        ch = (char)SendMessage(hwnd,SCI_GETCHARAT,(WPARAM)iTail,0);
        while (iTail >= iLineStart && (ch == ' ' || ch == '\t'))
        {
          iTail--;
          ch = (char)SendMessage(hwnd,SCI_GETCHARAT,(WPARAM)iTail,0);
          iLineEndPos--;
        }
        iEndCol = (int)SendMessage(hwnd,SCI_GETCOLUMN,(WPARAM)iLineEndPos,0);

        iMinIndent = min(iMinIndent,iIndentCol);
        iMaxLength = max(iMaxLength,iEndCol);
      }
    }

    UINT mbcp = Encoding_SciGetCodePage(hwnd);

    if (iMaxLength < BUFSIZE_ALIGN) {

      for (iLine = iLineStart; iLine <= iLineEnd; iLine++)
      {
        int iIndentPos = (int)SendMessage(hwnd,SCI_GETLINEINDENTPOSITION,(WPARAM)iLine,0);
        int iEndPos    = (int)SendMessage(hwnd,SCI_GETLINEENDPOSITION,(WPARAM)iLine,0);

        if (iIndentPos == iEndPos && iEndPos > 0) {

          if (!bModified) {
            SendMessage(hwnd,SCI_BEGINUNDOACTION,0,0);
            bModified = TRUE;
          }

          SendMessage(hwnd,SCI_SETTARGETSTART,(WPARAM)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLine,0),0);
          SendMessage(hwnd,SCI_SETTARGETEND,(WPARAM)iEndPos,0);
          SendMessage(hwnd,SCI_REPLACETARGET,0,(LPARAM)"");
        }

        else {

          char  tchLineBuf[BUFSIZE_ALIGN*3] = { '\0' };
          WCHAR wchLineBuf[BUFSIZE_ALIGN*3] = L"";
          WCHAR *pWords[BUFSIZE_ALIGN*3/2];
          WCHAR *p = wchLineBuf;

          int iWords = 0;
          int iWordsLength = 0;
          int cchLine = (int)SendMessage(hwnd,SCI_GETLINE,(WPARAM)iLine,(LPARAM)tchLineBuf);

          if (!bModified) {
            SendMessage(hwnd,SCI_BEGINUNDOACTION,0,0);
            bModified = TRUE;
          }

          MultiByteToWideChar(mbcp,0,tchLineBuf,cchLine,wchLineBuf,COUNTOF(wchLineBuf));
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

                if (SendMessage(hwnd,SCI_GETLINECOUNT,0,0) <= iLine+1)
                  bNextLineIsBlank = TRUE;

                else {

                  int iLineEndPos    = (int)SendMessage(hwnd,SCI_GETLINEENDPOSITION,(WPARAM)iLine+1,0);
                  int iLineIndentPos = (int)SendMessage(hwnd,SCI_GETLINEINDENTPOSITION,(WPARAM)iLine+1,0);

                  if (iLineIndentPos == iLineEndPos)
                    bNextLineIsBlank = TRUE;
                }
              }

              if ((nMode == ALIGN_JUSTIFY || nMode == ALIGN_JUSTIFY_EX) &&
                  iWords > 1 && iWordsLength >= 2 &&
                  ((nMode != ALIGN_JUSTIFY_EX || !bNextLineIsBlank || iLineStart == iLineEnd) ||
                  (bNextLineIsBlank && iWordsLength > (iMaxLength - iMinIndent) * 0.75))) {

                int iGaps = iWords - 1;
                int iSpacesPerGap = (iMaxLength - iMinIndent - iWordsLength) / iGaps;
                int iExtraSpaces = (iMaxLength - iMinIndent - iWordsLength) % iGaps;
                int i,j;
                int iPos;

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
                  StringCchCat(p,(length - StringCchLen(wchNewLineBuf)),pWords[i]);
                  p = StrEnd(p);
                }

                WideCharToMultiByteStrg(mbcp,wchNewLineBuf,tchLineBuf);

                iPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLine,0);
                SendMessage(hwnd,SCI_SETTARGETSTART,(WPARAM)iPos,0);
                iPos = (int)SendMessage(hwnd,SCI_GETLINEENDPOSITION,(WPARAM)iLine,0);
                SendMessage(hwnd,SCI_SETTARGETEND,(WPARAM)iPos,0);
                SendMessage(hwnd,SCI_REPLACETARGET,(WPARAM)StringCchLenA(tchLineBuf),(LPARAM)tchLineBuf);

                SendMessage(hwnd,SCI_SETLINEINDENTATION,(WPARAM)iLine,(LPARAM)iMinIndent);
              }
              else {

                int i;
                int iPos;

                WCHAR wchNewLineBuf[BUFSIZE_ALIGN] = { L'\0' };
                StringCchCopy(wchNewLineBuf,COUNTOF(wchNewLineBuf),pWords[0]);
                p = StrEnd(wchNewLineBuf);

                for (i = 1; i < iWords; i++) {
                  *p++ = L' ';
                  *p = 0;
                  StringCchCat(p,(COUNTOF(wchNewLineBuf) - StringCchLen(wchNewLineBuf)),pWords[i]);
                  p = StrEnd(p);
                }

                WideCharToMultiByteStrg(mbcp,wchNewLineBuf,tchLineBuf);

                iPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLine,0);
                SendMessage(hwnd,SCI_SETTARGETSTART,(WPARAM)iPos,0);
                iPos = (int)SendMessage(hwnd,SCI_GETLINEENDPOSITION,(WPARAM)iLine,0);
                SendMessage(hwnd,SCI_SETTARGETEND,(WPARAM)iPos,0);
                SendMessage(hwnd,SCI_REPLACETARGET,(WPARAM)StringCchLenA(tchLineBuf),(LPARAM)tchLineBuf);

                SendMessage(hwnd,SCI_SETLINEINDENTATION,(WPARAM)iLine,(LPARAM)iMinIndent);
              }
            }
            else {

              int iExtraSpaces = iMaxLength - iMinIndent - iWordsLength - iWords + 1;
              int iOddSpaces   = iExtraSpaces % 2;
              int i;
              int iPos;

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
                StringCchCat(p,(COUNTOF(wchNewLineBuf) - StringCchLen(wchNewLineBuf)),pWords[i]);
                if (i < iWords - 1)
                  StringCchCat(p,(COUNTOF(wchNewLineBuf) - StringCchLen(wchNewLineBuf)),L" ");
                if (nMode == ALIGN_CENTER && iWords > 1 && iOddSpaces > 0 && i + 1 >= iWords / 2) {
                  StringCchCat(p,(COUNTOF(wchNewLineBuf) - StringCchLen(wchNewLineBuf)),L" ");
                  iOddSpaces--;
                }
                p = StrEnd(p);
              }

              WideCharToMultiByteStrg(mbcp,wchNewLineBuf,tchLineBuf);

              if (nMode == ALIGN_RIGHT || nMode == ALIGN_CENTER) {
                SendMessage(hwnd,SCI_SETLINEINDENTATION,(WPARAM)iLine,(LPARAM)iMinIndent);
                iPos = (int)SendMessage(hwnd,SCI_GETLINEINDENTPOSITION,(WPARAM)iLine,0);
              }
              else
                iPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLine,0);
              SendMessage(hwnd,SCI_SETTARGETSTART,(WPARAM)iPos,0);
              iPos = (int)SendMessage(hwnd,SCI_GETLINEENDPOSITION,(WPARAM)iLine,0);
              SendMessage(hwnd,SCI_SETTARGETEND,(WPARAM)iPos,0);
              SendMessage(hwnd,SCI_REPLACETARGET,(WPARAM)StringCchLenA(tchLineBuf),(LPARAM)tchLineBuf);

              if (nMode == ALIGN_LEFT)
                SendMessage(hwnd,SCI_SETLINEINDENTATION,(WPARAM)iLine,(LPARAM)iMinIndent);
            }
          }
        }
      }
      if (bModified)
        SendMessage(hwnd,SCI_ENDUNDOACTION,0,0);
    }
    else
      MsgBox(MBINFO,IDS_BUFFERTOOSMALL);

    if (iCurPos < iAnchorPos) {
      iCurPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineStart,0);
      iAnchorPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineEnd+1,0);
    }
    else {
      iAnchorPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineStart,0);
      iCurPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineEnd+1,0);
    }
    SendMessage(hwnd,SCI_SETSEL,(WPARAM)iAnchorPos,(LPARAM)iCurPos);

  }
  else
    MsgBox(MBWARN,IDS_SELRECT);
}


//=============================================================================
//
//  EditEncloseSelection()
//
void EditEncloseSelection(HWND hwnd,LPCWSTR pwszOpen,LPCWSTR pwszClose)
{
  char  mszOpen[256*3] = { '\0' };
  char  mszClose[256*3] = { '\0' };

  int iSelStart = (int)SendMessage(hwnd,SCI_GETSELECTIONSTART,0,0);
  int iSelEnd   = (int)SendMessage(hwnd,SCI_GETSELECTIONEND,0,0);

  UINT mbcp = Encoding_SciGetCodePage(hwnd);

  if (lstrlen(pwszOpen))
    WideCharToMultiByteStrg(mbcp,pwszOpen,mszOpen);
  if (lstrlen(pwszClose))
    WideCharToMultiByteStrg(mbcp,pwszClose,mszClose);

  if (SC_SEL_RECTANGLE != SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0))
  {

    SendMessage(hwnd,SCI_BEGINUNDOACTION,0,0);

    if (StringCchLenA(mszOpen)) {
      SendMessage(hwnd,SCI_SETTARGETSTART,(WPARAM)iSelStart,0);
      SendMessage(hwnd,SCI_SETTARGETEND,(WPARAM)iSelStart,0);
      SendMessage(hwnd,SCI_REPLACETARGET,(WPARAM)StringCchLenA(mszOpen),(LPARAM)mszOpen);
    }

    if (StringCchLenA(mszClose)) {
      SendMessage(hwnd,SCI_SETTARGETSTART,(WPARAM)iSelEnd + StringCchLenA(mszOpen),0);
      SendMessage(hwnd,SCI_SETTARGETEND,(WPARAM)iSelEnd + StringCchLenA(mszOpen),0);
      SendMessage(hwnd,SCI_REPLACETARGET,(WPARAM)StringCchLenA(mszClose),(LPARAM)mszClose);
    }

    SendMessage(hwnd,SCI_ENDUNDOACTION,0,0);

    // Fix selection
    if (iSelStart == iSelEnd)
      SendMessage(hwnd,SCI_SETSEL,(WPARAM)iSelStart + StringCchLenA(mszOpen),(WPARAM)iSelStart + StringCchLenA(mszOpen));

    else {
      int iCurPos = (int)SendMessage(hwnd,SCI_GETCURRENTPOS,0,0);
      int iAnchorPos = (int)SendMessage(hwnd,SCI_GETANCHOR,0,0);
      if (iCurPos < iAnchorPos) {
        iCurPos = iSelStart + StringCchLenA(mszOpen);
        iAnchorPos = iSelEnd + StringCchLenA(mszOpen);
      }
      else {
        iAnchorPos = iSelStart + StringCchLenA(mszOpen);
        iCurPos = iSelEnd + StringCchLenA(mszOpen);
      }
      SendMessage(hwnd,SCI_SETSEL,(WPARAM)iAnchorPos,(LPARAM)iCurPos);
    }

  }
  else
    MsgBox(MBWARN,IDS_SELRECT);
}


//=============================================================================
//
//  EditToggleLineComments()
//
void EditToggleLineComments(HWND hwnd,LPCWSTR pwszComment,BOOL bInsertAtStart)
{
  char  mszComment[256*3] = { '\0' };
  int   cchComment;
  int   iAction = 0;

  int iSelStart = (int)SendMessage(hwnd,SCI_GETSELECTIONSTART,0,0);
  int iSelEnd   = (int)SendMessage(hwnd,SCI_GETSELECTIONEND,0,0);
  int iCurPos   = (int)SendMessage(hwnd,SCI_GETCURRENTPOS,0,0);

  UINT mbcp = Encoding_SciGetCodePage(hwnd);

  if (lstrlen(pwszComment))
    WideCharToMultiByte(mbcp,0,pwszComment,-1,mszComment,COUNTOF(mszComment),NULL,NULL);
  cchComment = StringCchLenA(mszComment);

  if (SC_SEL_RECTANGLE != SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0) && cchComment)
  {
    int iLine;
    int iCommentCol = 0;

    int iLineStart = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iSelStart,0);
    int iLineEnd   = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iSelEnd,0);

    if (iSelEnd <= SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineEnd,0))
    {
      if (iLineEnd - iLineStart >= 1)
        iLineEnd--;
    }

    if (!bInsertAtStart) {
      iCommentCol = 1024;
      for (iLine = iLineStart; iLine <= iLineEnd; iLine++) {
        int iLineEndPos = (int)SendMessage(hwnd,SCI_GETLINEENDPOSITION,(WPARAM)iLine,0);
        int iLineIndentPos = (int)SendMessage(hwnd,SCI_GETLINEINDENTPOSITION,(WPARAM)iLine,0);
        if (iLineIndentPos != iLineEndPos) {
          int iIndentColumn = (int)SendMessage(hwnd,SCI_GETCOLUMN,(WPARAM)iLineIndentPos,0);
          iCommentCol = min(iCommentCol,iIndentColumn);
        }
      }
    }

    SendMessage(hwnd,SCI_BEGINUNDOACTION,0,0);

    for (iLine = iLineStart; iLine <= iLineEnd; iLine++)
    {
      int iCommentPos;
      int iIndentPos = (int)SendMessage(hwnd,SCI_GETLINEINDENTPOSITION,(WPARAM)iLine,0);
      char tchBuf[32] = { L'\0' };
      struct Sci_TextRange tr;

      if (iIndentPos == SendMessage(hwnd,SCI_GETLINEENDPOSITION,(WPARAM)iLine,0))
        continue;

      tr.chrg.cpMin = iIndentPos;
      tr.chrg.cpMax = tr.chrg.cpMin + min(31,cchComment);
      tr.lpstrText = tchBuf;
      SendMessage(hwnd,SCI_GETTEXTRANGE,0,(LPARAM)&tr);

      if (StrCmpNIA(tchBuf,mszComment,cchComment) == 0) {
        switch (iAction) {
          case 0:
            iAction = 2;
          case 2:
            SendMessage(hwnd,SCI_SETTARGETSTART,(WPARAM)iIndentPos,0);
            SendMessage(hwnd,SCI_SETTARGETEND,(WPARAM)iIndentPos+cchComment,0);
            SendMessage(hwnd,SCI_REPLACETARGET,0,(LPARAM)"");
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
            iCommentPos = (int)SendMessage(hwnd,SCI_FINDCOLUMN,(WPARAM)iLine,(LPARAM)iCommentCol);
            SendMessage(hwnd,SCI_INSERTTEXT,(WPARAM)iCommentPos,(LPARAM)mszComment);
            break;
          case 2:
            break;
        }
      }
    }
    SendMessage(hwnd,SCI_ENDUNDOACTION,0,0);

    if (iSelStart != iSelEnd)
    {
      int iAnchorPos;
      if (iCurPos == iSelStart) {
        iCurPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineStart,0);
        iAnchorPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineEnd+1,0);
      }
      else {
        iAnchorPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineStart,0);
        iCurPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineEnd+1,0);
      }
      SendMessage(hwnd,SCI_SETSEL,(WPARAM)iAnchorPos,(LPARAM)iCurPos);
    }

  }
  else
    MsgBox(MBWARN,IDS_SELRECT);
}


//=============================================================================
//
//  EditPadWithSpaces()
//
void EditPadWithSpaces(HWND hwnd,BOOL bSkipEmpty,BOOL bNoUndoGroup)
{
  char *pmszPadStr;
  int iMaxColumn = 0;
  int iLine = 0;
  BOOL bIsRectangular = FALSE;
  BOOL bReducedSelection = FALSE;

  int iSelStart = 0;
  int iSelEnd = 0;

  int iLineStart = 0;
  int iLineEnd = 0;

  int iRcCurLine = 0;
  int iRcAnchorLine = 0;
  int iRcCurCol = 0;
  int iRcAnchorCol = 0;

  if (SC_SEL_RECTANGLE != SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0)) {

    iSelStart = (int)SendMessage(hwnd,SCI_GETSELECTIONSTART,0,0);
    iSelEnd   = (int)SendMessage(hwnd,SCI_GETSELECTIONEND,0,0);

    iLineStart = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iSelStart,0);
    iLineEnd   = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iSelEnd,0);

    if (iLineStart == iLineEnd) {
      iLineStart = 0;
      iLineEnd = (int)SendMessage(hwnd,SCI_GETLINECOUNT,0,0) -1;
    }

    else {
      if (iSelEnd <= SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineEnd,0)) {
        if (iLineEnd - iLineStart >= 1) {
          iLineEnd--;
          bReducedSelection = TRUE;
        }
      }
    }

    for (iLine = iLineStart; iLine <= iLineEnd; iLine++) {
      int iPos = (int)SendMessage(hwnd,SCI_GETLINEENDPOSITION,(WPARAM)iLine,0);
      iMaxColumn = max(iMaxColumn,(int)SendMessage(hwnd,SCI_GETCOLUMN,(WPARAM)iPos,0));
    }
  }
  else {

    int iCurPos = (int)SendMessage(hwnd,SCI_GETCURRENTPOS,0,0);
    int iAnchorPos = (int)SendMessage(hwnd,SCI_GETANCHOR,0,0);

    iRcCurLine = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iCurPos,0);
    iRcAnchorLine = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iAnchorPos,0);

    iRcCurCol = (int)SendMessage(hwnd,SCI_GETCOLUMN,(WPARAM)iCurPos,0);
    //iRcCurCol += (int)SendMessage(hwnd, SCI_GETSELECTIONNCARETVIRTUALSPACE, 0, 0);

    iRcAnchorCol = (int)SendMessage(hwnd,SCI_GETCOLUMN,(WPARAM)iAnchorPos,0);
    //iRcAnchorCol += (int)SendMessage(hwnd, SCI_GETSELECTIONNANCHORVIRTUALSPACE, 0, 0);

    bIsRectangular = TRUE;

    iLineStart = 0;
    iLineEnd = (int)SendMessage(hwnd,SCI_GETLINECOUNT,0,0) -1;

    for (iLine = iLineStart; iLine <= iLineEnd; iLine++) {

      int iPos = (int)SendMessage(hwnd,SCI_GETLINESELENDPOSITION,(WPARAM)iLine,0);
      if (iPos != INVALID_POSITION) {
        int iCol = (int)SendMessage(hwnd, SCI_GETCOLUMN, (WPARAM)iPos, 0);
        //iCol += (int)SendMessage(hwndEdit, SCI_GETSELECTIONNCARETVIRTUALSPACE, 0, 0);
        iMaxColumn = max(iMaxColumn, iCol);
      }
    }
  }

  pmszPadStr = LocalAlloc(LPTR,(iMaxColumn + 1 )* sizeof(char));
  if (pmszPadStr) {

    FillMemory(pmszPadStr,LocalSize(pmszPadStr),' ');

    if (!bNoUndoGroup)
      SendMessage(hwnd,SCI_BEGINUNDOACTION,0,0);

    for (iLine = iLineStart; iLine <= iLineEnd; iLine++) {

      int iPos;
      int iPadLen;
      int iLineSelEndPos;

      iLineSelEndPos = (int)SendMessage(hwnd,SCI_GETLINESELENDPOSITION,(WPARAM)iLine,0);
      if (bIsRectangular && INVALID_POSITION == iLineSelEndPos)
        continue;

      iPos = (int)SendMessage(hwnd,SCI_GETLINEENDPOSITION,(WPARAM)iLine,0);
      if (bIsRectangular && iPos > iLineSelEndPos)
        continue;
      if (bSkipEmpty && (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLine,0) >= iPos)
        continue;

      int iCol = (int)SendMessage(hwnd, SCI_GETCOLUMN, (WPARAM)iPos, 0);
      //iCol += (int)SendMessage(hwnd, SCI_GETSELECTIONNCARETVIRTUALSPACE, 0, 0);
      iPadLen = iMaxColumn - iCol;

      SendMessage(hwnd,SCI_SETTARGETSTART,(WPARAM)iPos,0);
      SendMessage(hwnd,SCI_SETTARGETEND,(WPARAM)iPos,0);
      SendMessage(hwnd,SCI_REPLACETARGET,(WPARAM)iPadLen,(LPARAM)pmszPadStr);
    }

    if (pmszPadStr)
      LocalFree(pmszPadStr);

    if (!bNoUndoGroup)
      SendMessage(hwnd,SCI_ENDUNDOACTION,0,0);
  }

  if (!bIsRectangular &&
      SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iSelStart,0) !=
      SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iSelEnd,0))
  {
    int iCurPos = (int)SendMessage(hwnd,SCI_GETCURRENTPOS,0,0);
    int iAnchorPos = (int)SendMessage(hwnd,SCI_GETANCHOR,0,0);
    if (iCurPos < iAnchorPos) {
      iCurPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineStart,0);
      if (!bReducedSelection)
        iAnchorPos = (int)SendMessage(hwnd,SCI_GETLINEENDPOSITION,(WPARAM)iLineEnd,0);
      else
        iAnchorPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineEnd+1,0);
    }
    else {
      iAnchorPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineStart,0);
      if (!bReducedSelection)
        iCurPos = (int)SendMessage(hwnd,SCI_GETLINEENDPOSITION,(WPARAM)iLineEnd,0);
      else
        iCurPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineEnd+1,0);
    }
    SendMessage(hwnd,SCI_SETSEL,(WPARAM)iAnchorPos,(LPARAM)iCurPos);
  }

  else if (bIsRectangular) {
    int iCurPos = (int)SendMessage(hwnd,SCI_FINDCOLUMN,(WPARAM)iRcCurLine,(LPARAM)iRcCurCol);
    int iAnchorPos = (int)SendMessage(hwnd,SCI_FINDCOLUMN,(WPARAM)iRcAnchorLine,(LPARAM)iRcAnchorCol);
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
  int iSelStart = (int)SendMessage(hwnd,SCI_GETSELECTIONSTART,0,0);
  int iSelEnd   = (int)SendMessage(hwnd,SCI_GETSELECTIONEND,0,0);

  if (iSelStart == iSelEnd) {
    iSelStart = 0;
    iSelEnd   = (int)SendMessage(hwnd,SCI_GETLENGTH,0,0);
  }

  if (SC_SEL_RECTANGLE != SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0))
  {
    int iLine;

    int iLineStart = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iSelStart,0);
    int iLineEnd   = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iSelEnd,0);

    if (iSelStart > SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineStart,0))
      iLineStart++;

    if (iSelEnd <= SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineEnd,0))
      iLineEnd--;

    SendMessage(hwnd,SCI_BEGINUNDOACTION,0,0);

    for (iLine = iLineStart; iLine <= iLineEnd; iLine++)
    {
      int iPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLine,0);
      if (SendMessage(hwnd,SCI_GETLINEENDPOSITION,(WPARAM)iLine,0)- iPos > 0)
      {
        SendMessage(hwnd,SCI_SETTARGETSTART,(WPARAM)iPos,0);
        SendMessage(hwnd,SCI_SETTARGETEND,
          (WPARAM)SendMessage(hwnd,SCI_POSITIONAFTER,(WPARAM)iPos,0),0);
        SendMessage(hwnd,SCI_REPLACETARGET,0,(LPARAM)"");
      }
    }
    SendMessage(hwnd,SCI_ENDUNDOACTION,0,0);
  }
  else
    MsgBox(MBWARN,IDS_SELRECT);
}


//=============================================================================
//
//  EditStripLastCharacter()
//
void EditStripLastCharacter(HWND hwnd)
{
  int iSelStart = (int)SendMessage(hwnd,SCI_GETSELECTIONSTART,0,0);
  int iSelEnd   = (int)SendMessage(hwnd,SCI_GETSELECTIONEND,0,0);

  if (iSelStart == iSelEnd) {
    iSelStart = 0;
    iSelEnd   = (int)SendMessage(hwnd,SCI_GETLENGTH,0,0);
  }

  if (SC_SEL_RECTANGLE != SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0))
  {
    int iLine;

    int iLineStart = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iSelStart,0);
    int iLineEnd   = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iSelEnd,0);

    if (iSelStart >= SendMessage(hwnd,SCI_GETLINEENDPOSITION,(WPARAM)iLineStart,0))
      iLineStart++;

    if (iSelEnd < SendMessage(hwnd,SCI_GETLINEENDPOSITION,(WPARAM)iLineEnd,0))
      iLineEnd--;

    SendMessage(hwnd,SCI_BEGINUNDOACTION,0,0);

    for (iLine = iLineStart; iLine <= iLineEnd; iLine++)
    {
      int iStartPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLine,0);
      int iEndPos   = (int)SendMessage(hwnd,SCI_GETLINEENDPOSITION,(WPARAM)iLine,0);
      if (iEndPos - iStartPos > 0)
      {
        SendMessage(hwnd,SCI_SETTARGETSTART,
          (WPARAM)SendMessage(hwnd,SCI_POSITIONBEFORE,(WPARAM)iEndPos,0),0);
        SendMessage(hwnd,SCI_SETTARGETEND,(WPARAM)iEndPos,0);
        SendMessage(hwnd,SCI_REPLACETARGET,0,(LPARAM)"");
      }
    }
    SendMessage(hwnd,SCI_ENDUNDOACTION,0,0);
  }
  else
    MsgBox(MBWARN,IDS_SELRECT);
}


//=============================================================================
//
//  EditStripTrailingBlanks()
//
void EditStripTrailingBlanks(HWND hwnd,BOOL bIgnoreSelection)
{
  // Check if there is any selection... simply use a regular expression replace!
  if (!bIgnoreSelection &&
    (SendMessage(hwnd,SCI_GETSELECTIONEND,0,0) - SendMessage(hwnd,SCI_GETSELECTIONSTART,0,0) != 0))
  {
    if (SC_SEL_RECTANGLE != SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0))
    {
      EDITFINDREPLACE efrTrim = { "[ \t]+$", "", "", "", SCFIND_REGEXP, 0, 0, 0, 0, 0, 0, NULL };
      efrTrim.hwnd = hwnd;

      EditReplaceAllInSelection(hwnd,&efrTrim,FALSE);
    }
    else
      MsgBox(MBWARN,IDS_SELRECT);
  }
  // Code from SciTE...
  else
  {
    int line;
    int maxLines;
    int lineStart;
    int lineEnd;
    int i;
    char ch;

    SendMessage(hwnd,SCI_BEGINUNDOACTION,0,0);
    maxLines = (int)SendMessage(hwnd,SCI_GETLINECOUNT,0,0);
    for (line = 0; line < maxLines; line++)
    {
      lineStart = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,line,0);
      lineEnd = (int)SendMessage(hwnd,SCI_GETLINEENDPOSITION,line,0);
      i = lineEnd-1;
      ch = (char)SendMessage(hwnd,SCI_GETCHARAT,i,0);
      while ((i >= lineStart) && ((ch == ' ') || (ch == '\t')))
      {
        i--;
        ch = (char)SendMessage(hwnd,SCI_GETCHARAT,i,0);
      }
      if (i < (lineEnd-1))
      {
        SendMessage(hwnd,SCI_SETTARGETSTART,i + 1,0);
        SendMessage(hwnd,SCI_SETTARGETEND,lineEnd,0);
        SendMessage(hwnd,SCI_REPLACETARGET,0,(LPARAM)"");
      }
    }
    SendMessage(hwnd,SCI_ENDUNDOACTION,0,0);
  }
}


//=============================================================================
//
//  EditCompressSpaces()
//
void EditCompressSpaces(HWND hwnd)
{
  if (SC_SEL_RECTANGLE != SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0))
  {
    int iSelStart  = (int)SendMessage(hwnd,SCI_GETSELECTIONSTART,0,0);
    int iSelEnd    = (int)SendMessage(hwnd,SCI_GETSELECTIONEND,0,0);
    int iCurPos    = (int)SendMessage(hwnd,SCI_GETCURRENTPOS,0,0);
    int iAnchorPos = (int)SendMessage(hwnd,SCI_GETANCHOR,0,0);
    int iLineStart = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iSelStart,0);
    int iLineEnd   = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iSelEnd,0);
    int iLength    = (int)SendMessage(hwnd,SCI_GETLENGTH,0,0);

    char* pszIn;
    char* pszOut;
    BOOL bIsLineStart, bIsLineEnd;
    BOOL bModified = FALSE;

    if (iSelStart != iSelEnd) {
      int cch = (int)SendMessage(hwnd,SCI_GETSELTEXT,0,0);
      pszIn = LocalAlloc(LPTR,cch);
      pszOut = LocalAlloc(LPTR,cch);
      SendMessage(hwnd,SCI_GETSELTEXT,0,(LPARAM)pszIn);
      bIsLineStart =
        (iSelStart == SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineStart,0));
      bIsLineEnd =
        (iSelEnd == SendMessage(hwnd,SCI_GETLINEENDPOSITION,(WPARAM)iLineEnd,0));
    }
    else {
      int cch = iLength + 1;
      pszIn = LocalAlloc(GPTR,cch);
      pszOut = LocalAlloc(GPTR,cch);
      SendMessage(hwnd,SCI_GETTEXT,(WPARAM)cch,(LPARAM)pszIn);
      bIsLineStart = TRUE;
      bIsLineEnd   = TRUE;
    }

    if (pszIn && pszOut) {
      char *ci, *co = pszOut;
      for (ci = pszIn; *ci; ci++) {
        if (*ci == ' ' || *ci == '\t') {
          if (*ci == '\t')
            bModified = TRUE;
          while (*(ci+1) == ' ' || *(ci+1) == '\t') {
            ci++;
            bModified = TRUE;
          }
          if (!bIsLineStart && (*(ci+1) != '\n' && *(ci+1) != '\r'))
            *co++ = ' ';
          else
            bModified = TRUE;
        }
        else {
          if (*ci == '\n' || *ci == '\r')
            bIsLineStart = TRUE;
          else
            bIsLineStart = FALSE;
          *co++ = *ci;
        }
      }
      if (bIsLineEnd && co > pszOut && *(co-1) == ' ') {
        *--co = 0;
        bModified = TRUE;
      }

      if (bModified) {
        if (iSelStart != iSelEnd)
          SendMessage(hwnd,SCI_TARGETFROMSELECTION,0,0);
        else {
          SendMessage(hwnd,SCI_SETTARGETSTART,0,0);
          SendMessage(hwnd,SCI_SETTARGETEND,(WPARAM)iLength,0);
        }
        SendMessage(hwnd,SCI_BEGINUNDOACTION,0,0);
        SendMessage(hwnd,SCI_REPLACETARGET,(WPARAM)-1,(LPARAM)pszOut);
        if (iCurPos > iAnchorPos) {
          iCurPos    = (int)SendMessage(hwnd,SCI_GETTARGETEND,0,0);
          iAnchorPos = (int)SendMessage(hwnd,SCI_GETTARGETSTART,0,0);
          SendMessage(hwnd,SCI_SETSEL,(WPARAM)iAnchorPos,(LPARAM)iCurPos);
        }
        else if (iCurPos < iAnchorPos) {
          iCurPos    = (int)SendMessage(hwnd,SCI_GETTARGETSTART,0,0);
          iAnchorPos = (int)SendMessage(hwnd,SCI_GETTARGETEND,0,0);
          SendMessage(hwnd,SCI_SETSEL,(WPARAM)iAnchorPos,(LPARAM)iCurPos);
        }
        SendMessage(hwnd,SCI_ENDUNDOACTION,0,0);
      }
    }
    if (pszIn)
      LocalFree(pszIn);
    if (pszOut)
      LocalFree(pszOut);
  }
  else
    MsgBox(MBWARN,IDS_SELRECT);
}


//=============================================================================
//
//  EditRemoveBlankLines()
//
void EditRemoveBlankLines(HWND hwnd,BOOL bMerge)
{
  int iSelStart = (int)SendMessage(hwnd,SCI_GETSELECTIONSTART,0,0);
  int iSelEnd   = (int)SendMessage(hwnd,SCI_GETSELECTIONEND,0,0);

  if (iSelStart == iSelEnd) {
    iSelStart = 0;
    iSelEnd   = (int)SendMessage(hwnd,SCI_GETLENGTH,0,0);
  }

  if (SC_SEL_RECTANGLE != SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0))
  {
    int iLine;

    int iLineStart = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iSelStart,0);
    int iLineEnd   = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iSelEnd,0);

    if (iSelStart > SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineStart,0))
      iLineStart++;

    if (iSelEnd <= SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineEnd,0) &&
        iLineEnd != SendMessage(hwnd,SCI_GETLINECOUNT,0,0)-1)
      iLineEnd--;

    SendMessage(hwnd,SCI_BEGINUNDOACTION,0,0);

    for (iLine = iLineStart; iLine <= iLineEnd; )
    {
      int nBlanks = 0;
      while (iLine + nBlanks <= iLineEnd &&
              SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLine + nBlanks,0) ==
              SendMessage(hwnd,SCI_GETLINEENDPOSITION,(WPARAM)iLine + nBlanks,0))
        nBlanks++;

      if (nBlanks == 0 || (nBlanks == 1 && bMerge))
        iLine += nBlanks + 1;

      else {
        int iTargetStart;
        int iTargetEnd;

        if (bMerge)
          nBlanks--;
        iTargetStart = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLine,0);
        iTargetEnd   = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLine + nBlanks,0);
        SendMessage(hwnd,SCI_SETTARGETSTART,(WPARAM)iTargetStart,0);
        SendMessage(hwnd,SCI_SETTARGETEND,(WPARAM)iTargetEnd,0);
        SendMessage(hwnd,SCI_REPLACETARGET,0,(LPARAM)"");
        if (bMerge)
          iLine++;
        iLineEnd -= nBlanks;
      }
    }
    SendMessage(hwnd,SCI_ENDUNDOACTION,0,0);
  }
  else
    MsgBox(MBWARN,IDS_SELRECT);
}


//=============================================================================
//
//  EditWrapToColumn()
//
void EditWrapToColumn(HWND hwnd,int nColumn/*,int nTabWidth*/)
{
  char* pszText;
  LPWSTR pszTextW;
  int cchTextW;
  int iTextW;
  LPWSTR pszConvW;
  int cchConvW;
  int cchConvM;
  int iLineLength;
  int iLine;
  int iCurPos;
  int iAnchorPos;
  int iSelStart;
  int iSelEnd;
  int iSelCount;
  UINT cpEdit;
  struct Sci_TextRange tr;
  int   cEOLMode;
  WCHAR wszEOL[] = L"\r\n";
  int   cchEOL = 2;
  BOOL bModified = FALSE;

  if (SC_SEL_RECTANGLE == SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0)) {
    MsgBox(MBWARN,IDS_SELRECT);
    return;
  }

  iCurPos    = (int)SendMessage(hwnd,SCI_GETCURRENTPOS,0,0);
  iAnchorPos = (int)SendMessage(hwnd,SCI_GETANCHOR,0,0);

  if (iCurPos == iAnchorPos)
    return;

  iSelStart = (int)SendMessage(hwnd,SCI_GETSELECTIONSTART,0,0);
  iLine = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iSelStart,0);
  iSelStart = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLine,0);
  iSelEnd   = (int)SendMessage(hwnd,SCI_GETSELECTIONEND,0,0);
  iSelCount = iSelEnd - iSelStart;

  pszText = GlobalAlloc(GPTR,iSelCount+2);
  if (pszText == NULL)
    return;

  pszTextW = GlobalAlloc(GPTR,(iSelCount+2)*sizeof(WCHAR));
  if (pszTextW == NULL) {
    GlobalFree(pszText);
    return;
  }

  tr.chrg.cpMin = iSelStart;
  tr.chrg.cpMax = iSelEnd;
  tr.lpstrText = pszText;
  SendMessage(hwnd,SCI_GETTEXTRANGE,0,(LPARAM)&tr);

  cpEdit = Encoding_SciGetCodePage(hwnd);
  cchTextW = MultiByteToWideChar(cpEdit,0,pszText,iSelCount,pszTextW,(int)(GlobalSize(pszTextW)/sizeof(WCHAR)));
  GlobalFree(pszText);

  pszConvW = GlobalAlloc(GPTR,cchTextW*sizeof(WCHAR)*3+2);
  if (pszConvW == NULL) {
    GlobalFree(pszTextW);
    return;
  }

  cEOLMode = (int)SendMessage(hwnd,SCI_GETEOLMODE,0,0);
  if (cEOLMode == SC_EOL_CR)
    cchEOL = 1;
  else if (cEOLMode == SC_EOL_LF) {
    cchEOL = 1; wszEOL[0] = L'\n';
  }

  cchConvW = 0;
  iLineLength = 0;

#define W_DELIMITER  L"!\"#$%&'()*+,-./:;<=>?@[\\]^`{|}~"  // underscore counted as part of word
#define ISDELIMITER(wc) StrChr(W_DELIMITER,wc)
#define ISWHITE(wc) StrChr(L" \t\f",wc)
#define ISWORDEND(wc) (/*ISDELIMITER(wc) ||*/ StrChr(L" \t\f\r\n\v",wc))

  for (iTextW = 0; iTextW < cchTextW; iTextW++)
  {
    WCHAR w;
    w = pszTextW[iTextW];

    //if (ISDELIMITER(w))
    //{
    //  int iNextWordLen = 0;
    //  WCHAR w2 = pszTextW[iTextW + 1];

    //  if (iLineLength + iNextWordLen + 1 > nColumn) {
    //    pszConvW[cchConvW++] = wszEOL[0];
    //    if (cchEOL > 1)
    //      pszConvW[cchConvW++] = wszEOL[1];
    //    iLineLength = 0;
    //    bModified = TRUE;
    //  }

    //  while (w2 != L'\0' && !ISWORDEND(w2)) {
    //    iNextWordLen++;
    //    w2 = pszTextW[iTextW + iNextWordLen + 1];
    //  }

    //  if (ISDELIMITER(w2) && iNextWordLen > 0) // delimiters go with the word
    //    iNextWordLen++;

    //  pszConvW[cchConvW++] = w;
    //  iLineLength++;

    //  if (iNextWordLen > 0)
    //  {
    //    if (iLineLength + iNextWordLen + 1 > nColumn) {
    //      pszConvW[cchConvW++] = wszEOL[0];
    //      if (cchEOL > 1)
    //        pszConvW[cchConvW++] = wszEOL[1];
    //      iLineLength = 0;
    //      bModified = TRUE;
    //    }
    //  }
    //}

    if (ISWHITE(w))
    {
      int iNextWordLen = 0;
      WCHAR w2;

      while (pszTextW[iTextW+1] == L' ' || pszTextW[iTextW+1] == L'\t') {
        iTextW++;
        bModified = TRUE;
      } // Modified: left out some whitespaces

      w2 = pszTextW[iTextW + 1];

      while (w2 != L'\0' && !ISWORDEND(w2)) {
        iNextWordLen++;
        w2 = pszTextW[iTextW + iNextWordLen + 1];
      }

      //if (ISDELIMITER(w2) /*&& iNextWordLen > 0*/) // delimiters go with the word
      //  iNextWordLen++;

      if (iNextWordLen > 0)
      {
        if (iLineLength + iNextWordLen + 1 > nColumn) {
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

  if (bModified) {
    pszText = GlobalAlloc(GPTR,cchConvW * 3);

    cchConvM = WideCharToMultiByte(cpEdit,0,pszConvW,cchConvW,pszText,(int)GlobalSize(pszText),NULL,NULL);
    GlobalFree(pszConvW);

    if (iAnchorPos > iCurPos) {
      iCurPos = iSelStart;
      iAnchorPos = iSelStart + cchConvM;
    }
    else {
      iAnchorPos = iSelStart;
      iCurPos = iSelStart + cchConvM;
    }

    SendMessage(hwnd,SCI_BEGINUNDOACTION,0,0);
    SendMessage(hwnd,SCI_SETTARGETSTART,(WPARAM)iSelStart,0);
    SendMessage(hwnd,SCI_SETTARGETEND,(WPARAM)iSelEnd,0);
    SendMessage(hwnd,SCI_REPLACETARGET,(WPARAM)cchConvM,(LPARAM)pszText);
    //SendMessage(hwnd,SCI_CLEAR,0,0);
    //SendMessage(hwnd,SCI_ADDTEXT,(WPARAM)cchConvW,(LPARAM)pszText);
    SendMessage(hwnd,SCI_SETSEL,(WPARAM)iAnchorPos,(LPARAM)iCurPos);
    SendMessage(hwnd,SCI_ENDUNDOACTION,0,0);

    GlobalFree(pszText);
  }

  else
    GlobalFree(pszConvW);
}


//=============================================================================
//
//  EditJoinLinesEx()
//
void EditJoinLinesEx(HWND hwnd)
{
  char* pszText;
  char* pszJoin;
  int   cchJoin = 0;
  int i;
  int iLine;
  int iCurPos;
  int iAnchorPos;
  int iSelStart;
  int iSelEnd;
  int iSelCount;
  struct Sci_TextRange tr;
  int  cEOLMode;
  char szEOL[] = "\r\n";
  int  cchEOL = 2;
  BOOL bModified = FALSE;

  if (SC_SEL_RECTANGLE == SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0)) {
    MsgBox(MBWARN,IDS_SELRECT);
    return;
  }

  iCurPos    = (int)SendMessage(hwnd,SCI_GETCURRENTPOS,0,0);
  iAnchorPos = (int)SendMessage(hwnd,SCI_GETANCHOR,0,0);

  if (iCurPos == iAnchorPos)
    return;

  iSelStart = (int)SendMessage(hwnd,SCI_GETSELECTIONSTART,0,0);
  iLine = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iSelStart,0);
  iSelStart = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLine,0);
  iSelEnd   = (int)SendMessage(hwnd,SCI_GETSELECTIONEND,0,0);
  iSelCount = iSelEnd - iSelStart;

  pszText = LocalAlloc(LPTR,iSelCount+4);
  if (pszText == NULL)
    return;

  pszJoin = LocalAlloc(LPTR,LocalSize(pszText));
  if (pszJoin == NULL) {
    LocalFree(pszText);
    return;
  }

  tr.chrg.cpMin = iSelStart;
  tr.chrg.cpMax = iSelEnd;
  tr.lpstrText = pszText;
  SendMessage(hwnd,SCI_GETTEXTRANGE,0,(LPARAM)&tr);

  cEOLMode = (int)SendMessage(hwnd,SCI_GETEOLMODE,0,0);
  if (cEOLMode == SC_EOL_CR)
    cchEOL = 1;
  else if (cEOLMode == SC_EOL_LF) {
    cchEOL = 1;
    szEOL[0] = '\n';
  }

  cchJoin = 0;
  for (i = 0; i < iSelCount; i++)
  {
    if (pszText[i] == '\r' || pszText[i] == '\n') {
      if (pszText[i] == '\r' && pszText[i+1] == '\n')
        i++;
      if (!StrChrA("\r\n",pszText[i+1]) && pszText[i+1] != 0) {
        pszJoin[cchJoin++] = ' ';
        bModified = TRUE;
      }
      else {
        while (StrChrA("\r\n",pszText[i+1])) {
          i++;
          bModified = TRUE;
        }
        if (pszText[i+1] != 0) {
          pszJoin[cchJoin++] = szEOL[0];
          if (cchEOL > 1)
            pszJoin[cchJoin++] = szEOL[1];
          if (cchJoin > cchEOL) {
            pszJoin[cchJoin++] = szEOL[0];
            if (cchEOL > 1)
              pszJoin[cchJoin++] = szEOL[1];
          }
        }
      }
    }
    else {
      pszJoin[cchJoin++] = pszText[i];
    }
  }

  LocalFree(pszText);

  if (bModified) {
    if (iAnchorPos > iCurPos) {
      iCurPos = iSelStart;
      iAnchorPos = iSelStart + cchJoin;
    }
    else {
      iAnchorPos = iSelStart;
      iCurPos = iSelStart + cchJoin;
    }

    SendMessage(hwnd,SCI_BEGINUNDOACTION,0,0);
    SendMessage(hwnd,SCI_SETTARGETSTART,(WPARAM)iSelStart,0);
    SendMessage(hwnd,SCI_SETTARGETEND,(WPARAM)iSelEnd,0);
    SendMessage(hwnd,SCI_REPLACETARGET,(WPARAM)cchJoin,(LPARAM)pszJoin);
    SendMessage(hwnd,SCI_SETSEL,(WPARAM)iAnchorPos,(LPARAM)iCurPos);
    SendMessage(hwnd,SCI_ENDUNDOACTION,0,0);
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

void EditSortLines(HWND hwnd,int iSortFlags)
{
  int iCurPos = 0;
  int iAnchorPos = 0;
  int iSelStart = 0;
  int iSelEnd = 0;
  int iLineStart = 0;
  int iLineEnd = 0;
  int iLineCount = 0;

  BOOL bIsRectangular = FALSE;
  int iRcCurLine = 0;
  int iRcAnchorLine = 0;
  int iRcCurCol = 0;
  int iRcAnchorCol = 0;

  int  iLine = 0;
  int  cchTotal = 0;
  int  ichlMax  = 3;

  SORTLINE *pLines = NULL;
  char  *pmszResult = NULL;
  char  *pmszBuf = NULL;

  UINT uCodePage = 0;
  DWORD cEOLMode = 0L;
  char mszEOL[] = "\r\n";

  UINT iTabWidth = 0;
  UINT iSortColumn = 0;

  BOOL bLastDup = FALSE;
  FNSTRCMP pfnStrCmp;

  pfnStrCmpLogicalW = GetProcAddress(GetModuleHandle(L"shlwapi"),"StrCmpLogicalW");
  pfnStrCmp = (iSortFlags & SORT_NOCASE) ? StrCmpIW : StrCmpW;

  iCurPos    = (int)SendMessage(hwnd,SCI_GETCURRENTPOS,0,0);
  iAnchorPos = (int)SendMessage(hwnd,SCI_GETANCHOR,0,0);

  if (iCurPos == iAnchorPos)
    return;

  if (SC_SEL_RECTANGLE == SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0)) {

    iRcCurLine = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iCurPos,0);
    iRcAnchorLine = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iAnchorPos,0);

    iRcCurCol = (int)SendMessage(hwnd,SCI_GETCOLUMN,(WPARAM)iCurPos,0);
    //iRcCurCol += (int)SendMessage(hwnd, SCI_GETSELECTIONNCARETVIRTUALSPACE, 0, 0);

    iRcAnchorCol = (int)SendMessage(hwnd, SCI_GETCOLUMN, (WPARAM)iAnchorPos, 0);
    //iRcAnchorCol += (int)SendMessage(hwnd, SCI_GETSELECTIONNANCHORVIRTUALSPACE, 0, 0);

    bIsRectangular = TRUE;

    iLineStart = min(iRcCurLine,iRcAnchorLine);
    iLineEnd   = max(iRcCurLine,iRcAnchorLine);

    iSortColumn = min(iRcCurCol,iRcAnchorCol);
  }

  else {

    iSelStart = (int)SendMessage(hwnd,SCI_GETSELECTIONSTART,0,0);
    iSelEnd   = (int)SendMessage(hwnd,SCI_GETSELECTIONEND,0,0);

    iLine = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iSelStart,0);
    iSelStart = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLine,0);

    iLineStart = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iSelStart,0);
    iLineEnd   = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iSelEnd,0);

    if (iSelEnd <= SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineEnd,0))
      iLineEnd--;

    iSortColumn = (UINT)SendMessage(hwnd,SCI_GETCOLUMN,(WPARAM)SendMessage(hwnd,SCI_GETCURRENTPOS,0,0),0);
    //iSortColumn += (int)SendMessage(hwnd, SCI_GETSELECTIONNCARETVIRTUALSPACE, 0, 0);
  }

  iLineCount = iLineEnd - iLineStart +1;
  if (iLineCount < 2)
    return;

  uCodePage = Encoding_SciGetCodePage(hwnd);
  cEOLMode = (DWORD)SendMessage(hwnd,SCI_GETEOLMODE,0,0);
  if (cEOLMode == SC_EOL_CR) {
    mszEOL[1] = 0;
  }
  else if (cEOLMode == SC_EOL_LF) {
    mszEOL[0] = '\n';
    mszEOL[1] = 0;
  }

  iTabWidth = (UINT)SendMessage(hwnd,SCI_GETTABWIDTH,0,0);

  SendMessage(hwnd,SCI_BEGINUNDOACTION,0,0);
  if (bIsRectangular)
    EditPadWithSpaces(hwnd,!(iSortFlags & SORT_SHUFFLE),TRUE);

  pLines = LocalAlloc(LPTR,sizeof(SORTLINE) * iLineCount);
  int i = 0;
  for (iLine = iLineStart; iLine <= iLineEnd; iLine++) {

    char *pmsz;
    int cchw;
    int cchm = (int)SendMessage(hwnd,SCI_GETLINE,(WPARAM)iLine,0);

    pmsz = LocalAlloc(LPTR,cchm+1);
    SendMessage(hwnd,SCI_GETLINE,(WPARAM)iLine,(LPARAM)pmsz);
    StrTrimA(pmsz,"\r\n");
    cchTotal += cchm;
    ichlMax = max(ichlMax,cchm);

    cchw = MultiByteToWideChar(uCodePage,0,pmsz,-1,NULL,0) -1;
    if (cchw > 0) {
      UINT col = 0, tabs = iTabWidth;
      pLines[i].pwszLine = LocalAlloc(LPTR,sizeof(WCHAR) * (cchw +1));
      MultiByteToWideChar(uCodePage,0,pmsz,-1,pLines[i].pwszLine,(int)LocalSize(pLines[i].pwszLine)/sizeof(WCHAR));
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
      qsort(pLines,iLineCount,sizeof(SORTLINE),CmpLogicalRev);
    else
      qsort(pLines,iLineCount,sizeof(SORTLINE),CmpStdRev);
  }
  else if (iSortFlags & SORT_SHUFFLE) {
    srand((UINT)GetTickCount());
    for (i = iLineCount-1; i > 0; i--) {
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
      qsort(pLines,iLineCount,sizeof(SORTLINE),CmpLogical);
    else
      qsort(pLines,iLineCount,sizeof(SORTLINE),CmpStd);
  }

  int lenRes = cchTotal + 2 * iLineCount + 1;
  pmszResult = LocalAlloc(LPTR,lenRes);
  pmszBuf    = LocalAlloc(LPTR,ichlMax+1);

  for (i = 0; i < iLineCount; i++) {
    BOOL bDropLine = FALSE;
    if (pLines[i].pwszLine && ((iSortFlags & SORT_SHUFFLE) || lstrlen(pLines[i].pwszLine))) {
      if (!(iSortFlags & SORT_SHUFFLE)) {
        if (iSortFlags & SORT_MERGEDUP || iSortFlags & SORT_UNIQDUP || iSortFlags & SORT_UNIQUNIQ) {
          if (i < iLineCount-1) {
            if (pfnStrCmp(pLines[i].pwszLine,pLines[i+1].pwszLine) == 0) {
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
        WideCharToMultiByte(uCodePage,0,pLines[i].pwszLine,-1,pmszBuf,(int)LocalSize(pmszBuf),NULL,NULL);
        StringCchCatA(pmszResult,lenRes,pmszBuf);
        StringCchCatA(pmszResult,lenRes,mszEOL);
      }
    }
  }

  LocalFree(pmszBuf);

  for (i = 0; i < iLineCount; i++) {
    if (pLines[i].pwszLine)
      LocalFree(pLines[i].pwszLine);
  }
  LocalFree(pLines);

  if (!bIsRectangular) {
    if (iAnchorPos > iCurPos) {
      iCurPos = iSelStart;
      iAnchorPos = iSelStart + _StringCchLenNA(pmszResult,lenRes);
    }
    else {
      iAnchorPos = iSelStart;
      iCurPos = iSelStart + _StringCchLenNA(pmszResult,lenRes);
    }
  }

  SendMessage(hwnd,SCI_SETTARGETSTART,(WPARAM)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineStart,0),0);
  SendMessage(hwnd,SCI_SETTARGETEND,(WPARAM)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLineEnd+1,0),0);
  SendMessage(hwnd,SCI_REPLACETARGET,(WPARAM)_StringCchLenNA(pmszResult,lenRes),(LPARAM)pmszResult);
  SendMessage(hwnd,SCI_ENDUNDOACTION,0,0);

  LocalFree(pmszResult);

  if (!bIsRectangular)
    SendMessage(hwnd,SCI_SETSEL,(WPARAM)iAnchorPos,(LPARAM)iCurPos);

  else {
    int iTargetStart = (int)SendMessage(hwnd,SCI_GETTARGETSTART,0,0);
    int iTargetEnd   = (int)SendMessage(hwnd,SCI_GETTARGETEND,0,0);
    SendMessage(hwnd,SCI_CLEARSELECTIONS,0,0);
    if (iTargetStart != iTargetEnd) {
      iTargetEnd -= (int)strlen(mszEOL);
      if (iRcAnchorLine > iRcCurLine) {
        iCurPos = (int)SendMessage(hwnd,SCI_FINDCOLUMN,
          (WPARAM)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iTargetStart,0),(LPARAM)iRcCurCol);
        iAnchorPos = (int)SendMessage(hwnd,SCI_FINDCOLUMN,
          (WPARAM)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iTargetEnd,0),(LPARAM)iRcAnchorCol);
      }
      else {
        iCurPos = (int)SendMessage(hwnd,SCI_FINDCOLUMN,
          (WPARAM)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iTargetEnd,0),(LPARAM)iRcCurCol);
        iAnchorPos = (int)SendMessage(hwnd,SCI_FINDCOLUMN,
          (WPARAM)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iTargetStart,0),(LPARAM)iRcAnchorCol);
      }
      if (iCurPos != iAnchorPos) {
        SendMessage(hwnd,SCI_SETRECTANGULARSELECTIONCARET,(WPARAM)iCurPos,0);
        SendMessage(hwnd,SCI_SETRECTANGULARSELECTIONANCHOR,(WPARAM)iAnchorPos,0);
      }
      else
        SendMessage(hwnd,SCI_SETSEL,(WPARAM)iTargetStart,(LPARAM)iTargetStart);
    }
    else
      SendMessage(hwnd,SCI_SETSEL,(WPARAM)iTargetStart,(LPARAM)iTargetStart);
  }
}


//=============================================================================
//
//  EditJumpTo()
//
void EditJumpTo(HWND hwnd,int iNewLine,int iNewCol)
{
  int iMaxLine = (int)SendMessage(hwnd,SCI_GETLINECOUNT,0,0);

  // Jumpt to end with line set to -1
  if (iNewLine == -1) {
    SendMessage(hwnd,SCI_DOCUMENTEND,0,0);
    return;
  }

  // Line maximum is iMaxLine
  iNewLine = min(iNewLine,iMaxLine);

  // Column minimum is 1
  iNewCol = max(iNewCol,1);

  if (iNewLine > 0 && iNewLine <= iMaxLine && iNewCol > 0)
  {
    int iNewPos  = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iNewLine-1,0);
    int iLineEndPos = (int)SendMessage(hwnd,SCI_GETLINEENDPOSITION,(WPARAM)iNewLine-1,0);

    while (iNewCol-1 > SendMessage(hwnd,SCI_GETCOLUMN,(WPARAM)iNewPos,0))
    {
      if (iNewPos >= iLineEndPos)
        break;

      iNewPos = (int)SendMessage(hwnd,SCI_POSITIONAFTER,(WPARAM)iNewPos,0);
    }

    iNewPos = min(iNewPos,iLineEndPos);

    EditSelectEx(hwnd,-1,iNewPos); // SCI_GOTOPOS(pos) is equivalent to SCI_SETSEL(-1, pos)
    SendMessage(hwnd,SCI_CHOOSECARETX,0,0);
  }
}


//=============================================================================
//
//  EditSelectEx()
//
void EditSelectEx(HWND hwnd,int iAnchorPos,int iCurrentPos)
{
  int iNewLine = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iCurrentPos,0);
  int iAnchorLine = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iAnchorPos,0);

  // Ensure that the first and last lines of a selection are always unfolded
  // This needs to be done *before* the SCI_SETSEL message
  SciCall_EnsureVisible(iAnchorLine);
  if (iAnchorLine != iNewLine)
    SciCall_EnsureVisible(iNewLine);

  SendMessage(hwnd,SCI_SETXCARETPOLICY,CARET_SLOP|CARET_STRICT|CARET_EVEN,50);
  SendMessage(hwnd,SCI_SETYCARETPOLICY,CARET_SLOP|CARET_STRICT|CARET_EVEN,5);
  SendMessage(hwnd,SCI_SETSEL,iAnchorPos,iCurrentPos);
  SendMessage(hwnd,SCI_SETXCARETPOLICY,CARET_SLOP|CARET_EVEN,50);
  SendMessage(hwnd,SCI_SETYCARETPOLICY,CARET_EVEN,0);
}


//=============================================================================
//
//  EditFixPositions()
//
void EditFixPositions(HWND hwnd)
{
  int iMaxPos = (int)SendMessage(hwnd,SCI_GETLENGTH,0,0);
  int iCurrentPos = (int)SendMessage(hwnd,SCI_GETCURRENTPOS,0,0);
  int iAnchorPos = (int)SendMessage(hwnd,SCI_GETANCHOR,0,0);

  if (iCurrentPos > 0 && iCurrentPos < iMaxPos) {
    int iNewPos = (int)SendMessage(hwnd,SCI_POSITIONAFTER,
      (WPARAM)(int)SendMessage(hwnd,SCI_POSITIONBEFORE,(WPARAM)iCurrentPos,0),0);
    if (iNewPos != iCurrentPos) {
      SendMessage(hwnd,SCI_SETCURRENTPOS,(WPARAM)iNewPos,0);
      iCurrentPos = iNewPos;
    }
  }

  if (iAnchorPos != iCurrentPos && iAnchorPos > 0 && iAnchorPos < iMaxPos) {
    int iNewPos = (int)SendMessage(hwnd,SCI_POSITIONAFTER,(WPARAM)
                    (int)SendMessage(hwnd,SCI_POSITIONBEFORE,(WPARAM)iAnchorPos,0),0);
    if (iNewPos != iAnchorPos)
      SendMessage(hwnd,SCI_SETANCHOR,(WPARAM)iNewPos,0);
  }
}


//=============================================================================
//
//  EditEnsureSelectionVisible()
//
void EditEnsureSelectionVisible(HWND hwnd)
{
  int iAnchorPos = (int)SendMessage(hwnd,SCI_GETANCHOR,0,0);
  int iCurrentPos = (int)SendMessage(hwnd,SCI_GETCURRENTPOS,0,0);
  SendMessage(hwnd,SCI_ENSUREVISIBLE,
    (WPARAM)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iAnchorPos,0),0);
  if (iAnchorPos != iCurrentPos) {
    SendMessage(hwnd,SCI_ENSUREVISIBLE,
      (WPARAM)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iCurrentPos,0),0);
  }
  EditSelectEx(hwnd,iAnchorPos,iCurrentPos);
}


//=============================================================================
//
//  EditGetExcerpt()
//
void EditGetExcerpt(HWND hwnd,LPWSTR lpszExcerpt,DWORD cchExcerpt)
{
  WCHAR tch[256] = { L'\0' };
  WCHAR *p;
  DWORD cch = 0;
  UINT cpEdit;
  struct Sci_TextRange tr;
  char*  pszText;
  LPWSTR pszTextW;

  int iCurPos    = (int)SendMessage(hwnd,SCI_GETCURRENTPOS,0,0);
  int iAnchorPos = (int)SendMessage(hwnd,SCI_GETANCHOR,0,0);

  if (iCurPos == iAnchorPos || SC_SEL_RECTANGLE == SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0)) {
    StringCchCopy(lpszExcerpt,cchExcerpt,L"");
    return;
  }

  /*if (iCurPos != iAnchorPos && SC_SEL_RECTANGLE != SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0)) {*/

    tr.chrg.cpMin = (int)SendMessage(hwnd,SCI_GETSELECTIONSTART,0,0);
    tr.chrg.cpMax = min((int)SendMessage(hwnd,SCI_GETSELECTIONEND,0,0),(LONG)(tr.chrg.cpMin + COUNTOF(tch)));
  /*}
  else {

    int iLine = SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)iCurPos,0);
    tr.chrg.cpMin = SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLine,0);
    tr.chrg.cpMax = min(SendMessage(hwnd,SCI_GETLINEENDPOSITION,(WPARAM)iLine,0),(LONG)(tr.chrg.cpMin + COUNTOF(tch)));
  }*/

  tr.chrg.cpMax = min((int)SendMessage(hwnd,SCI_GETLENGTH,0,0),tr.chrg.cpMax);

  pszText  = LocalAlloc(LPTR,(tr.chrg.cpMax - tr.chrg.cpMin)+2);
  pszTextW = LocalAlloc(LPTR,((tr.chrg.cpMax - tr.chrg.cpMin)*2)+2);

  if (pszText && pszTextW) {

    tr.lpstrText = pszText;
    SendMessage(hwnd,SCI_GETTEXTRANGE,0,(LPARAM)&tr);
    cpEdit = Encoding_SciGetCodePage(hwnd);
    MultiByteToWideChar(cpEdit,0,pszText,tr.chrg.cpMax - tr.chrg.cpMin,pszTextW,(int)GlobalSize(pszTextW)/sizeof(WCHAR));

    for (p = pszTextW; *p && cch < COUNTOF(tch)-1; p++) {
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
//  EditFindReplaceDlgProcW()
//
INT_PTR CALLBACK EditFindReplaceDlgProcW(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  LPEDITFINDREPLACE lpefr;
  int i;
  WCHAR tch[512+32];
  BOOL bCloseDlg;
  BOOL bIsFindDlg;

  static UINT uCPEdit;

  switch(umsg)
  {

    case WM_INITDIALOG:
      {
        static BOOL bFirstTime = TRUE;

        WCHAR tch2[FNDRPL_BUFFER] = { L'\0' };
        HMENU hmenu;

        SetWindowLongPtr(hwnd,DWLP_USER,(LONG_PTR)lParam);
        lpefr = (LPEDITFINDREPLACE)lParam;

        // Get the current code page for Unicode conversion
        uCPEdit = Encoding_SciGetCodePage(hwnd);

        // Load MRUs
        for (i = 0; i < MRU_Enum(mruFind,0,NULL,0); i++) {
          MRU_Enum(mruFind,i,tch2,COUNTOF(tch2));
          SendDlgItemMessage(hwnd,IDC_FINDTEXT,CB_ADDSTRING,0,(LPARAM)tch2);
        }
        for (i = 0; i < MRU_Enum(mruReplace,0,NULL,0); i++) {
          MRU_Enum(mruReplace,i,tch2,COUNTOF(tch2));
          SendDlgItemMessage(hwnd,IDC_REPLACETEXT,CB_ADDSTRING,0,(LPARAM)tch2);
        }

        if (!bSwitchedFindReplace)
        {
          char *lpszSelection = NULL;

          int cchSelection = (int)SendMessage(lpefr->hwnd,SCI_GETSELECTIONEND,0,0) -
                             (int)SendMessage(lpefr->hwnd,SCI_GETSELECTIONSTART,0,0);

          if ((0 < cchSelection) && (cchSelection < FNDRPL_BUFFER)) {
            cchSelection = (int)SendMessage(lpefr->hwnd,SCI_GETSELTEXT,0,0);
            lpszSelection = GlobalAlloc(GPTR,cchSelection + 2);
            SendMessage(lpefr->hwnd,SCI_GETSELTEXT,0,(LPARAM)lpszSelection);
          }
          else if (cchSelection == 0) {
            // nothing is selected in the editor:
            // if first time you bring up find/replace dialog, copy content from clipboard to find box
            if (bFirstTime)
            {
              char* pClip = EditGetClipboardText(hwnd,FALSE,NULL,NULL);
              if (pClip) {
                int len = lstrlenA(pClip);
                if (len > 0 && len < FNDRPL_BUFFER) {
                  lpszSelection = GlobalAlloc(GPTR,len + 2);
                  StringCchCopyNA(lpszSelection,len + 2,pClip,len);
                }
                LocalFree(pClip);
              }
            }
            bFirstTime = FALSE;
          }
          if (lpszSelection) {
            // Check lpszSelection and truncate bad chars (CR,LF,TAB)
            char* lpsz = StrChrA(lpszSelection,13);
            if (lpsz) *lpsz = '\0';

            lpsz = StrChrA(lpszSelection,10);
            if (lpsz) *lpsz = '\0';

            lpsz = StrChrA(lpszSelection,9);
            if (lpsz) *lpsz = '\0';

            SetDlgItemTextA2W(uCPEdit,hwnd,IDC_FINDTEXT,lpszSelection);
            GlobalFree(lpszSelection);
          }
          else {
            MRU_Enum(mruFind,0,tch2,COUNTOF(tch2));
            SetDlgItemText(hwnd,IDC_FINDTEXT,tch2);
          }
        }

        SendDlgItemMessage(hwnd,IDC_FINDTEXT,CB_LIMITTEXT,FNDRPL_BUFFER,0);
        SendDlgItemMessage(hwnd,IDC_FINDTEXT,CB_SETEXTENDEDUI,TRUE,0);

        if (!GetWindowTextLengthW(GetDlgItem(hwnd,IDC_FINDTEXT)))
          SetDlgItemTextA2W(CP_UTF8,hwnd,IDC_FINDTEXT,lpefr->szFindUTF8);

        if (GetDlgItem(hwnd,IDC_REPLACETEXT))
        {
          SendDlgItemMessage(hwnd,IDC_REPLACETEXT,CB_LIMITTEXT,FNDRPL_BUFFER,0);
          SendDlgItemMessage(hwnd,IDC_REPLACETEXT,CB_SETEXTENDEDUI,TRUE,0);
          SetDlgItemTextA2W(CP_UTF8,hwnd,IDC_REPLACETEXT,lpefr->szReplaceUTF8);
        }

        if (lpefr->fuFlags & SCFIND_MATCHCASE)
          CheckDlgButton(hwnd,IDC_FINDCASE,BST_CHECKED);

        if (lpefr->fuFlags & SCFIND_WHOLEWORD)
          CheckDlgButton(hwnd,IDC_FINDWORD,BST_CHECKED);

        if (lpefr->fuFlags & SCFIND_WORDSTART)
          CheckDlgButton(hwnd,IDC_FINDSTART,BST_CHECKED);

        if (lpefr->fuFlags & SCFIND_REGEXP)
          CheckDlgButton(hwnd,IDC_FINDREGEXP,BST_CHECKED);

        if (lpefr->bTransformBS)
          CheckDlgButton(hwnd,IDC_FINDTRANSFORMBS,BST_CHECKED);

        if (lpefr->bWildcardSearch)
        {
            CheckDlgButton(hwnd,IDC_WILDCARDSEARCH,BST_CHECKED);
            CheckDlgButton(hwnd,IDC_FINDREGEXP,BST_UNCHECKED);
        }

        if (lpefr->bNoFindWrap)
          CheckDlgButton(hwnd,IDC_NOWRAP,BST_CHECKED);

        if (GetDlgItem(hwnd,IDC_REPLACE)) {
          if (bSwitchedFindReplace) {
            if (lpefr->bFindClose)
              CheckDlgButton(hwnd,IDC_FINDCLOSE,BST_CHECKED);
          }
          else {
            if (lpefr->bReplaceClose)
              CheckDlgButton(hwnd,IDC_FINDCLOSE,BST_CHECKED);
          }
        }
        else {
          if (bSwitchedFindReplace) {
            if (lpefr->bReplaceClose)
              CheckDlgButton(hwnd,IDC_FINDCLOSE,BST_CHECKED);
          }
          else {
            if (lpefr->bFindClose)
              CheckDlgButton(hwnd,IDC_FINDCLOSE,BST_CHECKED);
          }
        }

        if (!bSwitchedFindReplace) {
          if (xFindReplaceDlg == 0 || yFindReplaceDlg == 0)
            CenterDlgInParent(hwnd);
          else
            SetDlgPos(hwnd,xFindReplaceDlg,yFindReplaceDlg);
        }

        else {
          SetDlgPos(hwnd,xFindReplaceDlgSave,yFindReplaceDlgSave);
          bSwitchedFindReplace = FALSE;
          CopyMemory(lpefr,&efrSave,sizeof(EDITFINDREPLACE));
        }

        hmenu = GetSystemMenu(hwnd,FALSE);
        GetString(SC_SAVEPOS,tch2,COUNTOF(tch2));
        InsertMenu(hmenu,0,MF_BYPOSITION|MF_STRING|MF_ENABLED,SC_SAVEPOS,tch2);
        GetString(SC_RESETPOS,tch2,COUNTOF(tch2));
        InsertMenu(hmenu,1,MF_BYPOSITION|MF_STRING|MF_ENABLED,SC_RESETPOS,tch2);
        InsertMenu(hmenu,2,MF_BYPOSITION|MF_SEPARATOR,0,NULL);
      }
      return TRUE;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {

        case IDC_FINDTEXT:
        case IDC_REPLACETEXT:
          {
            BOOL bEnable = (GetWindowTextLengthW(GetDlgItem(hwnd,IDC_FINDTEXT)) ||
                            CB_ERR != SendDlgItemMessage(hwnd,IDC_FINDTEXT,CB_GETCURSEL,0,0));

            EnableWindow(GetDlgItem(hwnd,IDOK),bEnable);
            EnableWindow(GetDlgItem(hwnd,IDC_FINDPREV),bEnable);
            EnableWindow(GetDlgItem(hwnd,IDC_REPLACE),bEnable);
            EnableWindow(GetDlgItem(hwnd,IDC_REPLACEALL),bEnable);
            EnableWindow(GetDlgItem(hwnd,IDC_REPLACEINSEL),bEnable);

            if (HIWORD(wParam) == CBN_CLOSEUP) {
              LONG lSelEnd;
              SendDlgItemMessage(hwnd,LOWORD(wParam),CB_GETEDITSEL,0,(LPARAM)&lSelEnd);
              SendDlgItemMessage(hwnd,LOWORD(wParam),CB_SETEDITSEL,0,MAKELPARAM(lSelEnd,lSelEnd));
            }
          }
          break;

        case IDC_FINDREGEXP:
          if (IsDlgButtonChecked(hwnd,IDC_FINDREGEXP) == BST_CHECKED)
          {
            CheckDlgButton(hwnd,IDC_FINDTRANSFORMBS,BST_UNCHECKED);
            CheckDlgButton(hwnd,IDC_WILDCARDSEARCH,BST_UNCHECKED); // Can not use wildcard search together with regexp
          }
          break;

        case IDC_FINDTRANSFORMBS:
          if (IsDlgButtonChecked(hwnd,IDC_FINDTRANSFORMBS) == BST_CHECKED)
            CheckDlgButton(hwnd,IDC_FINDREGEXP,BST_UNCHECKED);
          break;

        // handle wildcard search checkbox
        case IDC_WILDCARDSEARCH:
            CheckDlgButton(hwnd,IDC_FINDREGEXP,BST_UNCHECKED);
          //if (IsDlgButtonChecked(hwnd,IDC_FINDWILDCARDS) == BST_CHECKED)
          //  CheckDlgButton(hwnd,IDC_FINDREGEXP,BST_UNCHECKED);
          break;

        case IDOK:
        case IDC_FINDPREV:
        case IDC_REPLACE:
        case IDC_REPLACEALL:
        case IDC_REPLACEINSEL:
        case IDACC_SELTONEXT:
        case IDACC_SELTOPREV:
        case IDMSG_SWITCHTOFIND:
        case IDMSG_SWITCHTOREPLACE:

          lpefr = (LPEDITFINDREPLACE)GetWindowLongPtr(hwnd,DWLP_USER);

          bIsFindDlg = (GetDlgItem(hwnd,IDC_REPLACE) == NULL);

          if ((bIsFindDlg && LOWORD(wParam) == IDMSG_SWITCHTOREPLACE ||
              !bIsFindDlg && LOWORD(wParam) == IDMSG_SWITCHTOFIND)) {
            GetDlgPos(hwnd,&xFindReplaceDlgSave,&yFindReplaceDlgSave);
            bSwitchedFindReplace = TRUE;
            CopyMemory(&efrSave,lpefr,sizeof(EDITFINDREPLACE));
          }

          // Get current code page for Unicode conversion
          uCPEdit = Encoding_SciGetCodePage(hwnd);
          cpLastFind = uCPEdit;

          if (!bSwitchedFindReplace &&
              !GetDlgItemTextA2W(uCPEdit,hwnd,IDC_FINDTEXT,lpefr->szFind,COUNTOF(lpefr->szFind))) {

            EnableWindow(GetDlgItem(hwnd,IDOK),FALSE);
            EnableWindow(GetDlgItem(hwnd,IDC_FINDPREV),FALSE);
            EnableWindow(GetDlgItem(hwnd,IDC_REPLACE),FALSE);
            EnableWindow(GetDlgItem(hwnd,IDC_REPLACEALL),FALSE);
            EnableWindow(GetDlgItem(hwnd,IDC_REPLACEINSEL),FALSE);
            return TRUE;
          }

          if (GetDlgItem(hwnd,IDC_REPLACETEXT))
            GetDlgItemTextA2W(uCPEdit,hwnd,IDC_REPLACETEXT,lpefr->szReplace,COUNTOF(lpefr->szReplace));

          lpefr->bWildcardSearch = (IsDlgButtonChecked(hwnd,IDC_WILDCARDSEARCH) == BST_CHECKED) ? TRUE : FALSE;

          lpefr->fuFlags = 0;

          if (IsDlgButtonChecked(hwnd,IDC_FINDCASE) == BST_CHECKED)
            lpefr->fuFlags |= SCFIND_MATCHCASE;

          if (IsDlgButtonChecked(hwnd,IDC_FINDWORD) == BST_CHECKED)
            lpefr->fuFlags |= SCFIND_WHOLEWORD;

          if (IsDlgButtonChecked(hwnd,IDC_FINDSTART) == BST_CHECKED)
            lpefr->fuFlags |= SCFIND_WORDSTART;

          if (IsDlgButtonChecked(hwnd,IDC_FINDREGEXP) == BST_CHECKED)
            lpefr->fuFlags |= SCFIND_REGEXP | SCFIND_POSIX;

          lpefr->bTransformBS =
            (IsDlgButtonChecked(hwnd,IDC_FINDTRANSFORMBS) == BST_CHECKED) ? TRUE : FALSE;

          lpefr->bNoFindWrap = (IsDlgButtonChecked(hwnd,IDC_NOWRAP) == BST_CHECKED) ? TRUE : FALSE;

          if (bIsFindDlg) {
            lpefr->bFindClose = (IsDlgButtonChecked(hwnd,IDC_FINDCLOSE) == BST_CHECKED) ? TRUE : FALSE;
          }
          else {
            lpefr->bReplaceClose = (IsDlgButtonChecked(hwnd,IDC_FINDCLOSE) == BST_CHECKED) ? TRUE : FALSE;
          }

          if (!bSwitchedFindReplace) {
            // Save MRUs
            if (StringCchLenA(lpefr->szFind)) {
              if (GetDlgItemTextA2W(CP_UTF8,hwnd,IDC_FINDTEXT,lpefr->szFindUTF8,COUNTOF(lpefr->szFindUTF8))) {
                GetDlgItemText(hwnd,IDC_FINDTEXT,tch,COUNTOF(tch));
                MRU_Add(mruFind,tch,0,0);
              }
            }
            if (StringCchLenA(lpefr->szReplace)) {
              if (GetDlgItemTextA2W(CP_UTF8,hwnd,IDC_REPLACETEXT,lpefr->szReplaceUTF8,COUNTOF(lpefr->szReplaceUTF8))) {
                GetDlgItemText(hwnd,IDC_REPLACETEXT,tch,COUNTOF(tch));
                MRU_Add(mruReplace,tch,0,0);
              }
            }
            else
              StringCchCopyA(lpefr->szReplaceUTF8,COUNTOF(lpefr->szReplaceUTF8),"");
          }
          else {
            GetDlgItemTextA2W(CP_UTF8,hwnd,IDC_FINDTEXT,lpefr->szFindUTF8,COUNTOF(lpefr->szFindUTF8));
            if (!GetDlgItemTextA2W(CP_UTF8,hwnd,IDC_REPLACETEXT,lpefr->szReplaceUTF8,COUNTOF(lpefr->szReplaceUTF8)))
              StringCchCopyA(lpefr->szReplaceUTF8,COUNTOF(lpefr->szReplaceUTF8),"");
          }

          // Reload MRUs
          SendDlgItemMessage(hwnd,IDC_FINDTEXT,CB_RESETCONTENT,0,0);
          SendDlgItemMessage(hwnd,IDC_REPLACETEXT,CB_RESETCONTENT,0,0);

          for (i = 0; i < MRU_Enum(mruFind,0,NULL,0); i++) {
            MRU_Enum(mruFind,i,tch,COUNTOF(tch));
            SendDlgItemMessage(hwnd,IDC_FINDTEXT,CB_ADDSTRING,0,(LPARAM)tch);
          }
          for (i = 0; i < MRU_Enum(mruReplace,0,NULL,0); i++) {
            MRU_Enum(mruReplace,i,tch,COUNTOF(tch));
            SendDlgItemMessage(hwnd,IDC_REPLACETEXT,CB_ADDSTRING,0,(LPARAM)tch);
          }

          SetDlgItemTextA2W(CP_UTF8,hwnd,IDC_FINDTEXT,lpefr->szFindUTF8);
          SetDlgItemTextA2W(CP_UTF8,hwnd,IDC_REPLACETEXT,lpefr->szReplaceUTF8);

          if (!bSwitchedFindReplace)
            SendMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)(GetFocus()),1);

          if (bIsFindDlg) {
            bCloseDlg = lpefr->bFindClose;
          }
          else {
            if (LOWORD(wParam) == IDOK)
              bCloseDlg = FALSE;
            else
              bCloseDlg = lpefr->bReplaceClose;
          }

          if (bCloseDlg) {
            //EndDialog(hwnd,LOWORD(wParam));
            DestroyWindow(hwnd);
          }

          switch (LOWORD(wParam))
          {
            case IDOK: // find next
            case IDACC_SELTONEXT:
              if (!bIsFindDlg)
                bReplaceInitialized = TRUE;
              EditFindNext(lpefr->hwnd,lpefr,LOWORD(wParam)==IDACC_SELTONEXT||HIBYTE(GetKeyState(VK_SHIFT)));
              break;

            case IDC_FINDPREV: // find previous
            case IDACC_SELTOPREV:
              if (!bIsFindDlg)
                bReplaceInitialized = TRUE;
              EditFindPrev(lpefr->hwnd,lpefr,LOWORD(wParam)==IDACC_SELTOPREV||HIBYTE(GetKeyState(VK_SHIFT)));
              break;

            case IDC_REPLACE:
              bReplaceInitialized = TRUE;
              EditReplace(lpefr->hwnd,lpefr);
              break;

            case IDC_REPLACEALL:
              bReplaceInitialized = TRUE;
              EditReplaceAll(lpefr->hwnd,lpefr,TRUE);
              break;

            case IDC_REPLACEINSEL:
              bReplaceInitialized = TRUE;
              EditReplaceAllInSelection(lpefr->hwnd,lpefr,TRUE);
              break;
          }

          // Wildcard search will enable regexp, so I turn it off again otherwise it will be on in the gui
          if( lpefr->bWildcardSearch  &&  (lpefr->fuFlags & SCFIND_REGEXP) ) lpefr->fuFlags ^= SCFIND_REGEXP;

          break;


        case IDCANCEL:
          //EndDialog(hwnd,IDCANCEL);
          DestroyWindow(hwnd);
          break;

        case IDACC_FIND:
          PostMessage(GetParent(hwnd),WM_COMMAND,MAKELONG(IDM_EDIT_FIND,1),0);
          break;

        case IDACC_REPLACE:
          PostMessage(GetParent(hwnd),WM_COMMAND,MAKELONG(IDM_EDIT_REPLACE,1),0);
          break;

        case IDACC_SAVEPOS:
          GetDlgPos(hwnd,&xFindReplaceDlg,&yFindReplaceDlg);
          break;

        case IDACC_RESETPOS:
          CenterDlgInParent(hwnd);
          xFindReplaceDlg = yFindReplaceDlg = 0;
          break;

        case IDACC_FINDNEXT:
          PostMessage(hwnd,WM_COMMAND,MAKELONG(IDOK,1),0);
          break;

        case IDACC_FINDPREV:
          PostMessage(hwnd,WM_COMMAND,MAKELONG(IDC_FINDPREV,1),0);
          break;

        case IDACC_REPLACENEXT:
          if (GetDlgItem(hwnd,IDC_REPLACE) != NULL)
            PostMessage(hwnd,WM_COMMAND,MAKELONG(IDC_REPLACE,1),0);
          break;

        case IDACC_SAVEFIND:
          SendMessage(hwndMain,WM_COMMAND,MAKELONG(IDM_EDIT_SAVEFIND,1),0);
          lpefr = (LPEDITFINDREPLACE)GetWindowLongPtr(hwnd,DWLP_USER);
          SetDlgItemTextA2W(CP_UTF8,hwnd,IDC_FINDTEXT,lpefr->szFindUTF8);
          CheckDlgButton(hwnd,IDC_FINDREGEXP,BST_UNCHECKED);
          CheckDlgButton(hwnd,IDC_FINDTRANSFORMBS,BST_UNCHECKED);
          PostMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)(GetDlgItem(hwnd,IDC_FINDTEXT)),1);
          break;

      }

      return TRUE;


    case WM_SYSCOMMAND:
      if (wParam == SC_SAVEPOS){
        PostMessage(hwnd,WM_COMMAND,MAKELONG(IDACC_SAVEPOS,0),0);
        return TRUE;
      }
      else if (wParam == SC_RESETPOS){
        PostMessage(hwnd,WM_COMMAND,MAKELONG(IDACC_RESETPOS,0),0);
        return TRUE;
      }
      else
        return FALSE;


    case WM_NOTIFY:
      {
        LPNMHDR pnmhdr = (LPNMHDR)lParam;
        switch (pnmhdr->code) {

          case NM_CLICK:
          case NM_RETURN:
            if (pnmhdr->idFrom == IDC_TOGGLEFINDREPLACE) {
              if (GetDlgItem(hwnd,IDC_REPLACE))
                PostMessage(GetParent(hwnd),WM_COMMAND,MAKELONG(IDM_EDIT_FIND,1),0);
              else
                PostMessage(GetParent(hwnd),WM_COMMAND,MAKELONG(IDM_EDIT_REPLACE,1),0);
            }
            // Display help messages in the find/replace windows
            else if (pnmhdr->idFrom == IDC_BACKSLASHHELP)
            {
                MsgBox(MBINFO,IDS_BACKSLASHHELP);
            }
            else if (pnmhdr->idFrom == IDC_REGEXPHELP)
            {
                MsgBox(MBINFO,IDS_REGEXPHELP);
            }
            else if (pnmhdr->idFrom == IDC_WILDCARDHELP)
            {
                MsgBox(MBINFO,IDS_WILDCARDHELP);
            }
            break;
        }
      }
      break;

  }

  return FALSE;

}


//=============================================================================
//
//  EditFindReplaceDlg()
//
HWND EditFindReplaceDlg(HWND hwnd,LPCEDITFINDREPLACE lpefr,BOOL bReplace)
{

  HWND hDlg;

  lpefr->hwnd = hwnd;

  hDlg = CreateThemedDialogParam(g_hInstance,
            (bReplace) ? MAKEINTRESOURCEW(IDD_REPLACE) : MAKEINTRESOURCEW(IDD_FIND),
            GetParent(hwnd),
            EditFindReplaceDlgProcW,
            (LPARAM) lpefr);

  ShowWindow(hDlg,SW_SHOW);

  return hDlg;

}


// Wildcard search uses the regexp engine to perform a simple search with * ? as wildcards 
// instead of more advanced and user-unfriendly regexp syntax
void EscapeWildcards(char* szFind2, LPCEDITFINDREPLACE lpefr)
{
    char szWildcardEscaped[FNDRPL_BUFFER] = { '\0' };
    int iSource = 0;
    int iDest = 0;

    lpefr->fuFlags |= SCFIND_REGEXP;

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

    StringCchCopyNA(szFind2,FNDRPL_BUFFER,szWildcardEscaped,COUNTOF(szWildcardEscaped));
}


//=============================================================================
//
//  EditFindNext()
//
BOOL EditFindNext(HWND hwnd,LPCEDITFINDREPLACE lpefr,BOOL fExtendSelection)
{

  struct Sci_TextToFind ttf;
  int iPos;
  int iSelPos, iSelAnchor;
  char szFind2[FNDRPL_BUFFER];
  BOOL bSuppressNotFound = FALSE;

  if (!StringCchLenA(lpefr->szFind))
    return /*EditFindReplaceDlg(hwnd,lpefr,FALSE)*/FALSE;

  StringCchCopyNA(szFind2,COUNTOF(szFind2),lpefr->szFind,COUNTOF(lpefr->szFind));
  if (lpefr->bTransformBS)
    TransformBackslashes(szFind2,(lpefr->fuFlags & SCFIND_REGEXP),Encoding_SciGetCodePage(hwnd));

  if (StringCchLenA(szFind2) == 0)
  {
    InfoBox(0,L"MsgNotFound",IDS_NOTFOUND);
    return FALSE;
  }

  if( lpefr->bWildcardSearch ) EscapeWildcards( szFind2 , lpefr );

  iSelPos = (int)SendMessage(hwnd,SCI_GETCURRENTPOS,0,0);
  iSelAnchor = (int)SendMessage(hwnd,SCI_GETANCHOR,0,0);

  ZeroMemory(&ttf,sizeof(ttf));

  ttf.chrg.cpMin = (int)SendMessage(hwnd,SCI_GETSELECTIONEND,0,0);
  ttf.chrg.cpMax = (int)SendMessage(hwnd,SCI_GETLENGTH,0,0);
  ttf.lpstrText = szFind2;

  iPos = (int)SendMessage(hwnd,SCI_FINDTEXT,lpefr->fuFlags,(LPARAM)&ttf);

  if (iPos == -1 && ttf.chrg.cpMin > 0 && !lpefr->bNoFindWrap && !fExtendSelection) {
    if (IDOK == InfoBox(MBOKCANCEL,L"MsgFindWrap1",IDS_FIND_WRAPFW)) {
      ttf.chrg.cpMin = 0;
      iPos = (int)SendMessage(hwnd,SCI_FINDTEXT,lpefr->fuFlags,(LPARAM)&ttf);
    }
    else
      bSuppressNotFound = TRUE;
  }

  if (iPos == -1)
  {
    // notfound
    if (!bSuppressNotFound)
      InfoBox(0,L"MsgNotFound",IDS_NOTFOUND);
    return FALSE;
  }

  if (!fExtendSelection)
    EditSelectEx(hwnd,ttf.chrgText.cpMin,ttf.chrgText.cpMax);
  else
    EditSelectEx(hwnd,min(iSelAnchor,iSelPos),ttf.chrgText.cpMax);

  return TRUE;

}


//=============================================================================
//
//  EditFindPrev()
//
BOOL EditFindPrev(HWND hwnd,LPCEDITFINDREPLACE lpefr,BOOL fExtendSelection)
{

  struct Sci_TextToFind ttf;
  int iPos;
  int iSelPos, iSelAnchor;
  int iLength;
  char szFind2[FNDRPL_BUFFER];
  BOOL bSuppressNotFound = FALSE;

  if (!StringCchLenA(lpefr->szFind))
    return /*EditFindReplaceDlg(hwnd,lpefr,FALSE)*/FALSE;

  StringCchCopyNA(szFind2,COUNTOF(szFind2),lpefr->szFind,COUNTOF(lpefr->szFind));
  if (lpefr->bTransformBS)
    TransformBackslashes(szFind2,(lpefr->fuFlags & SCFIND_REGEXP),Encoding_SciGetCodePage(hwnd));

  if (StringCchLenA(szFind2) == 0)
  {
    InfoBox(0,L"MsgNotFound",IDS_NOTFOUND);
    return FALSE;
  }

  if( lpefr->bWildcardSearch ) EscapeWildcards( szFind2 , lpefr );

  iSelPos = (int)SendMessage(hwnd,SCI_GETCURRENTPOS,0,0);
  iSelAnchor = (int)SendMessage(hwnd,SCI_GETANCHOR,0,0);

  ZeroMemory(&ttf,sizeof(ttf));

  ttf.chrg.cpMin = max(0,(int)SendMessage(hwnd,SCI_GETSELECTIONSTART,0,0));
  ttf.chrg.cpMax = 0;
  ttf.lpstrText = szFind2;

  iPos = (int)SendMessage(hwnd,SCI_FINDTEXT,lpefr->fuFlags,(LPARAM)&ttf);

  iLength = (int)SendMessage(hwnd,SCI_GETLENGTH,0,0);
  if (iPos == -1 && ttf.chrg.cpMin < iLength && !lpefr->bNoFindWrap && !fExtendSelection) {
    if (IDOK == InfoBox(MBOKCANCEL,L"MsgFindWrap2",IDS_FIND_WRAPRE)) {
      ttf.chrg.cpMin = iLength;
      iPos = (int)SendMessage(hwnd,SCI_FINDTEXT,lpefr->fuFlags,(LPARAM)&ttf);
    }
    else
      bSuppressNotFound = TRUE;
  }

  if (iPos == -1)
  {
    // notfound
    if (!bSuppressNotFound)
      InfoBox(0,L"MsgNotFound",IDS_NOTFOUND);
    return FALSE;
  }

  if (!fExtendSelection)
    EditSelectEx(hwnd,ttf.chrgText.cpMin,ttf.chrgText.cpMax);
  else
    EditSelectEx(hwnd,max(iSelPos,iSelAnchor),ttf.chrgText.cpMin);

  return TRUE;

}


//=============================================================================
//
//  EditReplace()
//
BOOL EditReplace(HWND hwnd,LPCEDITFINDREPLACE lpefr)
{

  struct Sci_TextToFind ttf;
  int iPos;
  int iSelStart;
  int iSelEnd;
  int iReplaceMsg = (lpefr->fuFlags & SCFIND_REGEXP) ? SCI_REPLACETARGETRE : SCI_REPLACETARGET;
  char szFind2[FNDRPL_BUFFER];
  char *pszReplace2;
  BOOL bSuppressNotFound = FALSE;

  if (!StringCchLenA(lpefr->szFind))
    return /*EditFindReplaceDlg(hwnd,lpefr,TRUE)*/FALSE;

  StringCchCopyNA(szFind2,COUNTOF(szFind2),lpefr->szFind,COUNTOF(lpefr->szFind));
  if (lpefr->bTransformBS)
    TransformBackslashes(szFind2,(lpefr->fuFlags & SCFIND_REGEXP),Encoding_SciGetCodePage(hwnd));

  if (StringCchLenA(szFind2) == 0)
  {
    InfoBox(0,L"MsgNotFound",IDS_NOTFOUND);
    return FALSE;
  }

  if( lpefr->bWildcardSearch ) EscapeWildcards( szFind2 , lpefr );

  if (StringCchCompareNA(lpefr->szReplace,FNDRPL_BUFFER,"^c",-1) == 0) {
    iReplaceMsg = SCI_REPLACETARGET;
    pszReplace2 = EditGetClipboardText(hwnd,TRUE,NULL,NULL);
  }
  else {
    //lstrcpyA(szReplace2,lpefr->szReplace);
    pszReplace2 = StrDupA(lpefr->szReplace);
    if (!pszReplace2)
      pszReplace2 = StrDupA("");
    if (lpefr->bTransformBS)
      TransformBackslashes(pszReplace2,(lpefr->fuFlags & SCFIND_REGEXP),Encoding_SciGetCodePage(hwnd));
  }

  if (!pszReplace2)
    return FALSE; // recoding canceled

  iSelStart = (int)SendMessage(hwnd,SCI_GETSELECTIONSTART,0,0);
  iSelEnd = (int)SendMessage(hwnd,SCI_GETSELECTIONEND,0,0);

  ZeroMemory(&ttf,sizeof(ttf));
  ttf.chrg.cpMin = (int)SendMessage(hwnd,SCI_GETSELECTIONSTART,0,0); // Start!
  ttf.chrg.cpMax = (int)SendMessage(hwnd,SCI_GETLENGTH,0,0);
  ttf.lpstrText = szFind2;

  iPos = (int)SendMessage(hwnd,SCI_FINDTEXT,lpefr->fuFlags,(LPARAM)&ttf);

  if (iPos == -1 && ttf.chrg.cpMin > 0 && !lpefr->bNoFindWrap) {
    if (IDOK == InfoBox(MBOKCANCEL,L"MsgFindWrap1",IDS_FIND_WRAPFW)) {
      ttf.chrg.cpMin = 0;
      iPos = (int)SendMessage(hwnd,SCI_FINDTEXT,lpefr->fuFlags,(LPARAM)&ttf);
    }
    else
      bSuppressNotFound = TRUE;
  }

  if (iPos == -1)
  {
    // notfound
    if (!bSuppressNotFound)
      InfoBox(0,L"MsgNotFound",IDS_NOTFOUND);
    LocalFree(pszReplace2);
    return FALSE;
  }

  if (iSelStart != ttf.chrgText.cpMin || iSelEnd != ttf.chrgText.cpMax) {
    EditSelectEx(hwnd,ttf.chrgText.cpMin,ttf.chrgText.cpMax);
    LocalFree(pszReplace2);
    return FALSE;
  }

  SendMessage(hwnd,SCI_SETTARGETSTART,ttf.chrgText.cpMin,0);
  SendMessage(hwnd,SCI_SETTARGETEND,ttf.chrgText.cpMax,0);
  SendMessage(hwnd,iReplaceMsg,(WPARAM)-1,(LPARAM)pszReplace2);

  ttf.chrg.cpMin = (int)SendMessage(hwnd,SCI_GETTARGETEND,0,0);
  ttf.chrg.cpMax = (int)SendMessage(hwnd,SCI_GETLENGTH,0,0);

  iPos = (int)SendMessage(hwnd,SCI_FINDTEXT,lpefr->fuFlags,(LPARAM)&ttf);

  bSuppressNotFound = FALSE;
  if (iPos == -1 && ttf.chrg.cpMin > 0 && !lpefr->bNoFindWrap) {
    if (IDOK == InfoBox(MBOKCANCEL,L"MsgFindWrap1",IDS_FIND_WRAPFW)) {
      ttf.chrg.cpMin = 0;
      iPos = (int)SendMessage(hwnd,SCI_FINDTEXT,lpefr->fuFlags,(LPARAM)&ttf);
    }
    else
      bSuppressNotFound = TRUE;
  }

  if (iPos != -1)
    EditSelectEx(hwnd,ttf.chrgText.cpMin,ttf.chrgText.cpMax);

  else {
    EditSelectEx(hwnd,
      (int)SendMessage(hwnd,SCI_GETTARGETEND,0,0),
      (int)SendMessage(hwnd,SCI_GETTARGETEND,0,0));
    if (!bSuppressNotFound)
      InfoBox(0,L"MsgNotFound",IDS_NOTFOUND);
  }

  LocalFree(pszReplace2);
  return TRUE;

}

//=============================================================================
//
//  CompleteWord()
//  Auto-complete words (by Aleksandar Lekov)
//
struct WLIST {
  char* word;
  struct WLIST* next;
};

void CompleteWord(HWND hwnd, BOOL autoInsert) 
{
  const char* NON_WORD = bAccelWordNavigation ? DelimCharsAccel : DelimChars;

  int iCurrentPos = (int)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);
  int iLine = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, iCurrentPos, 0);
  int iCurrentLinePos = iCurrentPos - (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, (WPARAM)iLine, 0);
  int iStartWordPos = iCurrentLinePos;
  char *pLine;
  char *pRoot;
  char *pWord;
  int iNumWords = 0;
  int iRootLen = 0;
  int iDocLen;
  int iPosFind;
  struct Sci_TextRange tr = { { 0, -1 }, NULL };
  struct Sci_TextToFind ft = {{0, 0}, 0, {0, 0}};
  BOOL bWordAllNumbers = TRUE;
  struct WLIST* lListHead = NULL;
  int iWListSize = 0;

  pLine = LocalAlloc(LPTR, (int)SendMessage(hwnd, SCI_GETLINE, (WPARAM)iLine, 0) + 1);
  SendMessage(hwnd, SCI_GETLINE, (WPARAM)iLine, (LPARAM)pLine);

  while (iStartWordPos > 0 && !StrChrIA(NON_WORD, pLine[iStartWordPos - 1])) {
    iStartWordPos--;
    if (pLine[iStartWordPos] < '0' || pLine[iStartWordPos] > '9') {
      bWordAllNumbers = FALSE;
    }
  }

  if (iStartWordPos == iCurrentLinePos || bWordAllNumbers || iCurrentLinePos - iStartWordPos < 2) {
    LocalFree(pLine);
    return;
  }

  int cnt = iCurrentLinePos - iStartWordPos;
  pRoot = LocalAlloc(LPTR,cnt+1);
  StringCchCopyNA(pRoot,cnt+1,pLine + iStartWordPos,cnt);
  LocalFree(pLine);
  iRootLen = _StringCchLenNA(pRoot,cnt+1);

  iDocLen = (int)SendMessage(hwnd, SCI_GETLENGTH, 0, 0);

  ft.lpstrText = pRoot;
  ft.chrg.cpMax = iDocLen;

  iPosFind = (int)SendMessage(hwnd, SCI_FINDTEXT, SCFIND_WORDSTART, (LPARAM)&ft);

  while (iPosFind >= 0 && iPosFind < iDocLen) {
    int wordLength;
    int wordEnd = iPosFind + iRootLen;

    if (iPosFind != iCurrentPos - iRootLen) {
      while (wordEnd < iDocLen && !StrChrIA(NON_WORD, (char)SendMessage(hwnd, SCI_GETCHARAT, (WPARAM)wordEnd, 0)))
        wordEnd++;

      wordLength = wordEnd - iPosFind;
      if (wordLength > iRootLen) {
        struct WLIST* p = lListHead;
        struct WLIST* t = NULL;
        //int lastCmp = 0;
        BOOL found = FALSE;

        pWord = LocalAlloc(LPTR,wordLength + 1);

        tr.lpstrText = pWord;
        tr.chrg.cpMin = iPosFind;
        tr.chrg.cpMax = wordEnd;
        SendMessage(hwnd, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);

        while(p) {
          int cmp = StringCchCompareNA(pWord,wordLength + 1, p->word,-1);
          if (cmp == 0) {
            found = TRUE;
            break;
          } else if (cmp < 0) {
            break;
          }
          t = p;
          p = p->next;
        }
        if (!found) {
          struct WLIST* el = (struct WLIST*)LocalAlloc(LPTR, sizeof(struct WLIST));
          el->word = LocalAlloc(LPTR,wordLength + 1);
          StringCchCopyA(el->word,wordLength + 1,pWord);
          el->next = p;
          if (t) {
            t->next = el;
          } else {
            lListHead = el;
          }

          iNumWords++;
          iWListSize += _StringCchLenNA(pWord,wordLength + 1) + 1;
        }
        LocalFree(pWord);
      }
    }
    ft.chrg.cpMin = wordEnd;
    iPosFind = (int)SendMessage(hwnd, SCI_FINDTEXT, SCFIND_WORDSTART, (LPARAM)&ft);
  }

  if (iNumWords > 0) {
    char *pList;
    struct WLIST* p = lListHead;
    struct WLIST* t;

    pList = LocalAlloc(LPTR, iWListSize + 1);
    while (p) {
      StringCchCatA(pList,iWListSize + 1," ");
      StringCchCatA(pList,iWListSize + 1,p->word);
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

  LocalFree(pRoot);
}


BOOL __fastcall EditHighlightIfBrace(HWND hwnd, int iPos)
{
  if (iPos < 0) {
    // clear indicator
    SendMessage(hwnd, SCI_BRACEBADLIGHT, (WPARAM)INVALID_POSITION, 0);
    SendMessage(hwnd, SCI_SETHIGHLIGHTGUIDE, 0, 0);
    if (!bUseOldStyleBraceMatching)
      SendMessage(hwnd, SCI_BRACEBADLIGHTINDICATOR, 0, INDIC_NP3_BAD_BRACE);
    return TRUE;
  }
  char c = (char)SendMessage(hwnd, SCI_GETCHARAT, iPos, 0);
  if (StrChrA("()[]{}", c)) {
    int iBrace2 = (int)SendMessage(hwnd, SCI_BRACEMATCH, iPos, 0);
    if (iBrace2 != -1) {
      int col1 = (int)SendMessage(hwnd, SCI_GETCOLUMN, iPos, 0);
      int col2 = (int)SendMessage(hwnd, SCI_GETCOLUMN, iBrace2, 0);
      SendMessage(hwnd, SCI_BRACEHIGHLIGHT, iPos, iBrace2);
      SendMessage(hwnd, SCI_SETHIGHLIGHTGUIDE, min(col1, col2), 0);
      if (!bUseOldStyleBraceMatching)
        SendMessage(hwnd, SCI_BRACEHIGHLIGHTINDICATOR, 1, INDIC_NP3_MATCH_BRACE);
    }
    else {
      SendMessage(hwnd, SCI_BRACEBADLIGHT, iPos, 0);
      SendMessage(hwnd, SCI_SETHIGHLIGHTGUIDE, 0, 0);
      if (!bUseOldStyleBraceMatching)
        SendMessage(hwnd, SCI_BRACEBADLIGHTINDICATOR, 1, INDIC_NP3_BAD_BRACE);
    }
    return TRUE;
  }
  return FALSE;
}



//=============================================================================
//
//  EditMatchBrace()
//
void EditMatchBrace(HWND hwnd)
{
  int iEndStyled = (int)SendMessage(hwnd, SCI_GETENDSTYLED, 0, 0);
  if (iEndStyled < (int)SendMessage(hwnd, SCI_GETLENGTH, 0, 0)) {
    int iLine = (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, iEndStyled, 0);
    int iEndStyled2 = (int)SendMessage(hwnd, SCI_POSITIONFROMLINE, iLine, 0);
    SendMessage(hwnd, SCI_COLOURISE, iEndStyled2, -1);
  }

  int iPos = (int)SendMessage(hwnd, SCI_GETCURRENTPOS, 0, 0);

  if (!EditHighlightIfBrace(hwnd, iPos)) {
    // try one before
    iPos = (int)SendMessage(hwnd, SCI_POSITIONBEFORE, iPos, 0);
    if (!EditHighlightIfBrace(hwnd, iPos)) {
      // clear mark
      EditHighlightIfBrace(hwnd, -1);
    }
  }
}



//=============================================================================
//
//  EditMarkAll()
//  Mark all occurrences of the text currently selected (by Aleksandar Lekov)
//
void EditMarkAll(HWND hwnd, BOOL bMarkOccurrencesMatchCase, BOOL bMarkOccurrencesMatchWords)
{
  struct Sci_TextToFind ttf;
  int iPos;
  char *pszText;
  int iTextLen;
  int iSelStart;
  int iSelEnd;
  int iSelLength;
  int iSelCount;

  // feature is off
  if (!iMarkOccurrences) {
    iMarkOccurrencesCount = -1;
    UpdateStatusbar();
    return;
  }


  iTextLen = (int)SendMessage(hwnd,SCI_GETLENGTH,0,0);

  // get current selection
  iSelStart = (int)SendMessage(hwnd,SCI_GETSELECTIONSTART,0,0);
  iSelEnd = (int)SendMessage(hwnd,SCI_GETSELECTIONEND,0,0);
  iSelLength = (int)SendMessage(hwnd,SCI_GETSELTEXT,0,0);
  iSelCount = iSelEnd - iSelStart;

  // clear existing marker indicators
  SendMessage(hwnd, SCI_SETINDICATORCURRENT, INDIC_NP3_MARK_OCCURANCE, 0);
  SendMessage(hwnd, SCI_INDICATORCLEARRANGE, 0, iTextLen);

  // if nothing selected or multiple lines are selected exit
  if ((iSelCount == 0) ||
      (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, iSelStart, 0) !=
      (int)SendMessage(hwnd, SCI_LINEFROMPOSITION, iSelEnd, 0))
    return;


  pszText = LocalAlloc(LPTR,iSelLength);
  (int)SendMessage(hwnd,SCI_GETSELTEXT,0,(LPARAM)pszText);


  // exit if selection is not a word and Match whole words only is enabled
  if (bMarkOccurrencesMatchWords)
  {
    int iSelStart2 = 0;
    const char* delims = (bAccelWordNavigation ? DelimCharsAccel : DelimChars);
    while ((iSelStart2 <= iSelCount) && pszText[iSelStart2])
    {
      if (StrChrIA(delims,pszText[iSelStart2]))
      {
        LocalFree(pszText);
        return;
      }
      iSelStart2++;
    }
  }

  ZeroMemory(&ttf,sizeof(ttf));

  ttf.chrg.cpMin = 0;
  ttf.chrg.cpMax = iTextLen;
  ttf.lpstrText = pszText;

  iMarkOccurrencesCount = 0;
  while ((iPos = (int)SendMessage(hwnd, SCI_FINDTEXT,
      (bMarkOccurrencesMatchCase ? SCFIND_MATCHCASE : 0) | (bMarkOccurrencesMatchWords ? SCFIND_WHOLEWORD : 0),
      (LPARAM)&ttf)) != -1
      && (++iMarkOccurrencesCount < iMarkOccurrencesMaxCount))
  {
    // mark this match
    SendMessage(hwnd, SCI_INDICATORFILLRANGE, iPos, iSelCount);
    ttf.chrg.cpMin = ttf.chrgText.cpMin + iSelCount;
    if (ttf.chrg.cpMin == ttf.chrg.cpMax)
      break;
  }
  LocalFree(pszText);

  UpdateStatusbar();
  iMarkOccurrencesCount = 0;

  return;
}


//=============================================================================
//
//  EditReplaceAll()
//
BOOL EditReplaceAll(HWND hwnd,LPCEDITFINDREPLACE lpefr,BOOL bShowInfo)
{

  struct Sci_TextToFind ttf;
  int iPos;
  int iCount = 0;
  int iReplaceMsg = (lpefr->fuFlags & SCFIND_REGEXP) ? SCI_REPLACETARGETRE : SCI_REPLACETARGET;
  char szFind2[FNDRPL_BUFFER];
  char *pszReplace2;
  BOOL bRegexStartOfLine;
  BOOL bRegexStartOrEndOfLine;

  if (!StringCchLenA(lpefr->szFind))
    return /*EditFindReplaceDlg(hwnd,lpefr,TRUE)*/FALSE;

  // Show wait cursor...
  BeginWaitCursor();

  StringCchCopyNA(szFind2,COUNTOF(szFind2),lpefr->szFind,COUNTOF(lpefr->szFind));
  if (lpefr->bTransformBS)
    TransformBackslashes(szFind2,(lpefr->fuFlags & SCFIND_REGEXP),Encoding_SciGetCodePage(hwnd));

  if (StringCchLenA(szFind2) == 0)
  {
    InfoBox(0,L"MsgNotFound",IDS_NOTFOUND);
    return FALSE;
  }

  if( lpefr->bWildcardSearch ) EscapeWildcards( szFind2 , lpefr );

  bRegexStartOfLine =
    (szFind2[0] == '^');
  bRegexStartOrEndOfLine =
    (lpefr->fuFlags & SCFIND_REGEXP &&
      (!StringCchCompareNA(szFind2,FNDRPL_BUFFER,"$",-1) ||
       !StringCchCompareNA(szFind2,FNDRPL_BUFFER,"^",-1) ||
       !StringCchCompareNA(szFind2,FNDRPL_BUFFER,"^$",-1)));

  if (StringCchCompareNA(lpefr->szReplace,FNDRPL_BUFFER,"^c",-1) == 0) {
    iReplaceMsg = SCI_REPLACETARGET;
    pszReplace2 = EditGetClipboardText(hwnd,TRUE,NULL,NULL);
  }
  else {
    //lstrcpyA(szReplace2,lpefr->szReplace);
    pszReplace2 = StrDupA(lpefr->szReplace);
    if (!pszReplace2)
      pszReplace2 = StrDupA("");
    if (lpefr->bTransformBS)
      TransformBackslashes(pszReplace2,(lpefr->fuFlags & SCFIND_REGEXP),Encoding_SciGetCodePage(hwnd));
  }

  if (!pszReplace2)
    return FALSE; // recoding canceled

  ZeroMemory(&ttf,sizeof(ttf));
  ttf.chrg.cpMin = 0;
  ttf.chrg.cpMax = (int)SendMessage(hwnd,SCI_GETLENGTH,0,0);
  ttf.lpstrText = szFind2;

  while ((iPos = (int)SendMessage(hwnd,SCI_FINDTEXT,lpefr->fuFlags,(LPARAM)&ttf)) != -1)
  {
    int iReplacedLen;
    //char ch;

    if (iCount == 0 && bRegexStartOrEndOfLine) {
      if (0 == SendMessage(hwnd,SCI_GETLINEENDPOSITION,0,0)) {
        iPos = 0;
        ttf.chrgText.cpMin = 0;
        ttf.chrgText.cpMax = 0;
      }
    }

    if (++iCount == 1)
      SendMessage(hwnd,SCI_BEGINUNDOACTION,0,0);

    SendMessage(hwnd,SCI_SETTARGETSTART,ttf.chrgText.cpMin,0);
    SendMessage(hwnd,SCI_SETTARGETEND,ttf.chrgText.cpMax,0);
    iReplacedLen = (int)SendMessage(hwnd,iReplaceMsg,(WPARAM)-1,(LPARAM)pszReplace2);

    ttf.chrg.cpMin = ttf.chrgText.cpMin + iReplacedLen;
    ttf.chrg.cpMax = (int)SendMessage(hwnd,SCI_GETLENGTH,0,0);

    if (ttf.chrg.cpMin == ttf.chrg.cpMax)
      break;

    //ch = (char)SendMessage(hwnd,SCI_GETCHARAT,SendMessage(hwnd,SCI_GETTARGETEND,0,0),0);

    if (/*ch == '\r' || ch == '\n' || iReplacedLen == 0 || */
        ttf.chrgText.cpMin == ttf.chrgText.cpMax &&
        !(bRegexStartOrEndOfLine && iReplacedLen > 0))
      ttf.chrg.cpMin = (int)SendMessage(hwnd,SCI_POSITIONAFTER,ttf.chrg.cpMin,0);
    if (bRegexStartOfLine) {
      int iLine = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)ttf.chrg.cpMin,0);
      int ilPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLine,0);
      if (ilPos == ttf.chrg.cpMin)
        ttf.chrg.cpMin = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLine+1,0);
      if (ttf.chrg.cpMin == ttf.chrg.cpMax)
        break;
    }
  }

  if (iCount)
    SendMessage(hwnd,SCI_ENDUNDOACTION,0,0);

  // Remove wait cursor
  EndWaitCursor();

  if (bShowInfo) {
    if (iCount > 0)
      InfoBox(0,L"MsgReplaceCount",IDS_REPLCOUNT,iCount);
    else
      InfoBox(0,L"MsgNotFound",IDS_NOTFOUND);
  }

  LocalFree(pszReplace2);
  return TRUE;

}


//=============================================================================
//
//  EditReplaceAllInSelection()
//
BOOL EditReplaceAllInSelection(HWND hwnd,LPCEDITFINDREPLACE lpefr,BOOL bShowInfo)
{

  struct Sci_TextToFind ttf;
  int iPos;
  int iCount = 0;
  int iReplaceMsg = (lpefr->fuFlags & SCFIND_REGEXP) ? SCI_REPLACETARGETRE : SCI_REPLACETARGET;
  BOOL fCancel = FALSE;
  char szFind2[FNDRPL_BUFFER];
  char *pszReplace2;
  BOOL bRegexStartOfLine;
  BOOL bRegexStartOrEndOfLine;

  if (SC_SEL_RECTANGLE == SendMessage(hwnd,SCI_GETSELECTIONMODE,0,0))
  {
    MsgBox(MBWARN,IDS_SELRECT);
    return FALSE;
  }

  if (!StringCchLenA(lpefr->szFind))
    return /*EditFindReplaceDlg(hwnd,lpefr,TRUE)*/FALSE;

  // Show wait cursor...
  BeginWaitCursor();

  StringCchCopyNA(szFind2,COUNTOF(szFind2),lpefr->szFind,COUNTOF(lpefr->szFind));
  if (lpefr->bTransformBS)
    TransformBackslashes(szFind2,(lpefr->fuFlags & SCFIND_REGEXP),Encoding_SciGetCodePage(hwnd));

  if (StringCchLenA(szFind2) == 0)
  {
    InfoBox(0,L"MsgNotFound",IDS_NOTFOUND);
    return FALSE;
  }

  if( lpefr->bWildcardSearch ) EscapeWildcards( szFind2 , lpefr );

  bRegexStartOfLine = (szFind2[0] == '^');
  bRegexStartOrEndOfLine = (lpefr->fuFlags & SCFIND_REGEXP &&
      (!StringCchCompareNA(szFind2,FNDRPL_BUFFER,"$",-1) || 
       !StringCchCompareNA(szFind2,FNDRPL_BUFFER,"^",-1) ||
       !StringCchCompareNA(szFind2,FNDRPL_BUFFER,"^$",-1)));

  if (StringCchCompareNA(lpefr->szReplace,FNDRPL_BUFFER,"^c",-1) == 0) {
    iReplaceMsg = SCI_REPLACETARGET;
    pszReplace2 = EditGetClipboardText(hwnd,TRUE,NULL,NULL);
  }
  else {
    //lstrcpyA(szReplace2,lpefr->szReplace);
    pszReplace2 = StrDupA(lpefr->szReplace);
    if (!pszReplace2)
      pszReplace2 = StrDupA("");
    if (lpefr->bTransformBS)
      TransformBackslashes(pszReplace2,(lpefr->fuFlags & SCFIND_REGEXP),Encoding_SciGetCodePage(hwnd));
  }

  if (!pszReplace2)
    return FALSE; // recoding canceled

  ZeroMemory(&ttf,sizeof(ttf));
  ttf.chrg.cpMin = (int)SendMessage(hwnd,SCI_GETSELECTIONSTART,0,0);
  ttf.chrg.cpMax = (int)SendMessage(hwnd,SCI_GETLENGTH,0,0);
  ttf.lpstrText = szFind2;

  while ((iPos = (int)SendMessage(hwnd,SCI_FINDTEXT,lpefr->fuFlags,(LPARAM)&ttf)) != -1 && !fCancel)
  {
    if (ttf.chrgText.cpMin >= SendMessage(hwnd,SCI_GETSELECTIONSTART,0,0) &&
        ttf.chrgText.cpMax <= SendMessage(hwnd,SCI_GETSELECTIONEND,0,0))
    {
      int iReplacedLen;
      //char ch;

      if (ttf.chrg.cpMin == 0 && iCount == 0 && bRegexStartOrEndOfLine) {
        if (0 == SendMessage(hwnd,SCI_GETLINEENDPOSITION,0,0)) {
          iPos = 0;
          ttf.chrgText.cpMin = 0;
          ttf.chrgText.cpMax = 0;
        }
      }

      if (++iCount == 1)
        SendMessage(hwnd,SCI_BEGINUNDOACTION,0,0);

      SendMessage(hwnd,SCI_SETTARGETSTART,ttf.chrgText.cpMin,0);
      SendMessage(hwnd,SCI_SETTARGETEND,ttf.chrgText.cpMax,0);
      iReplacedLen = (int)SendMessage(hwnd,iReplaceMsg,(WPARAM)-1,(LPARAM)pszReplace2);

      ttf.chrg.cpMin = ttf.chrgText.cpMin + iReplacedLen;
      ttf.chrg.cpMax = (int)SendMessage(hwnd,SCI_GETLENGTH,0,0);

      if (ttf.chrg.cpMin == ttf.chrg.cpMax)
        fCancel = TRUE;

      //ch = (char)SendMessage(hwnd,SCI_GETCHARAT,SendMessage(hwnd,SCI_GETTARGETEND,0,0),0);

      if (/*ch == '\r' || ch == '\n' || iReplacedLen == 0 || */
          ttf.chrgText.cpMin == ttf.chrgText.cpMax &&
          !(bRegexStartOrEndOfLine && iReplacedLen > 0))
        ttf.chrg.cpMin = (int)SendMessage(hwnd,SCI_POSITIONAFTER,ttf.chrg.cpMin,0);
      if (bRegexStartOfLine) {
        int iLine = (int)SendMessage(hwnd,SCI_LINEFROMPOSITION,(WPARAM)ttf.chrg.cpMin,0);
        int ilPos = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLine,0);
        if (ilPos == ttf.chrg.cpMin)
          ttf.chrg.cpMin = (int)SendMessage(hwnd,SCI_POSITIONFROMLINE,(WPARAM)iLine+1,0);
        if (ttf.chrg.cpMin == ttf.chrg.cpMax)
          break;
      }
    }

    else
      // gone across selection, cancel
      fCancel = TRUE;
  }

  if (iCount) {

    if (SendMessage(hwnd,SCI_GETSELECTIONEND,0,0) <
        SendMessage(hwnd,SCI_GETTARGETEND,0,0)) {

      int iAnchorPos = (int)SendMessage(hwnd,SCI_GETANCHOR,0,0);
      int iCurrentPos = (int)SendMessage(hwnd,SCI_GETCURRENTPOS,0,0);

      if (iAnchorPos > iCurrentPos)
        iAnchorPos = (int)SendMessage(hwnd,SCI_GETTARGETEND,0,0);
      else
        iCurrentPos = (int)SendMessage(hwnd,SCI_GETTARGETEND,0,0);

      EditSelectEx(hwnd,iAnchorPos,iCurrentPos);
    }

    SendMessage(hwnd,SCI_ENDUNDOACTION,0,0);
  }

  // Remove wait cursor
  EndWaitCursor();

  if (bShowInfo) {
    if (iCount > 0)
      InfoBox(0,L"MsgReplaceCount",IDS_REPLCOUNT,iCount);
    else
      InfoBox(0,L"MsgNotFound",IDS_NOTFOUND);
  }

  LocalFree(pszReplace2);
  return TRUE;

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

        int iCurLine = (int)SendMessage(hwndEdit,SCI_LINEFROMPOSITION,
                         SendMessage(hwndEdit,SCI_GETCURRENTPOS,0,0),0)+1;

        SetDlgItemInt(hwnd,IDC_LINENUM,iCurLine,FALSE);
        SendDlgItemMessage(hwnd,IDC_LINENUM,EM_LIMITTEXT,15,0);

        SendDlgItemMessage(hwnd,IDC_COLNUM,EM_LIMITTEXT,15,0);

        CenterDlgInParent(hwnd);

      }
      return TRUE;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {

        case IDOK: {

          BOOL fTranslated;
          BOOL fTranslated2;

          int iNewCol;

          int iNewLine = (int)GetDlgItemInt(hwnd,IDC_LINENUM,&fTranslated,FALSE);
          int iMaxLine = (int)SendMessage(hwndEdit,SCI_GETLINECOUNT,0,0);

          if (SendDlgItemMessage(hwnd,IDC_COLNUM,WM_GETTEXTLENGTH,0,0) > 0)
            iNewCol = GetDlgItemInt(hwnd,IDC_COLNUM,&fTranslated2,FALSE);
          else {
            iNewCol = 1;
            fTranslated2 = TRUE;
          }

          if (!fTranslated || !fTranslated2)
          {
            PostMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)(GetDlgItem(hwnd,(!fTranslated) ? IDC_LINENUM : IDC_COLNUM)),1);
            return TRUE;
          }

          if (iNewLine > 0 && iNewLine <= iMaxLine && iNewCol > 0)
          {
            EditJumpTo(hwndEdit,iNewLine,iNewCol);
            EndDialog(hwnd,IDOK);
          }

          else
            PostMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)(GetDlgItem(hwnd,(!(iNewLine > 0 && iNewLine <= iMaxLine)) ? IDC_LINENUM : IDC_COLNUM)),1);

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
            SetTextColor(hdc,RGB(0,0,255));
          SelectObject(hdc,/*dwId == id_hover?*/hFontHover/*:hFontNormal*/);
          return(LONG_PTR)GetSysColorBrush(COLOR_BTNFACE);
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
              int  cchIns = 2;
              BOOL bClear = TRUE;

              GetDlgItemTextW(hwnd,100,wchBuf,256);
              if (StringCchLen(wchBuf) >= 3) {

                if (wchBuf[0] == L'<') {

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
          EnableWindow(GetDlgItem(hwnd,103),FALSE);
          EnableWindow(GetDlgItem(hwnd,104),FALSE);
          EnableWindow(GetDlgItem(hwnd,105),FALSE);
          EnableWindow(GetDlgItem(hwnd,106),FALSE);
          EnableWindow(GetDlgItem(hwnd,107),FALSE);
        }
        else
          CheckRadioButton(hwnd,100,102,100);
        if (*piSortFlags & SORT_MERGEDUP)
          CheckDlgButton(hwnd,103,BST_CHECKED);
        if (*piSortFlags & SORT_UNIQDUP) {
          CheckDlgButton(hwnd,104,BST_CHECKED);
          EnableWindow(GetDlgItem(hwnd,103),FALSE);
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
          EnableWindow(GetDlgItem(hwnd,107),FALSE);
          bEnableLogicalSort = FALSE;
        }
        if (SC_SEL_RECTANGLE != SendMessage(hwndEdit,SCI_GETSELECTIONMODE,0,0)) {
          *piSortFlags &= ~SORT_COLUMN;
          EnableWindow(GetDlgItem(hwnd,108),FALSE);
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
          EnableWindow(GetDlgItem(hwnd,103),IsDlgButtonChecked(hwnd,105) != BST_CHECKED);
          EnableWindow(GetDlgItem(hwnd,104),TRUE);
          EnableWindow(GetDlgItem(hwnd,105),TRUE);
          EnableWindow(GetDlgItem(hwnd,106),TRUE);
          EnableWindow(GetDlgItem(hwnd,107),bEnableLogicalSort);
          break;
        case 102:
          EnableWindow(GetDlgItem(hwnd,103),FALSE);
          EnableWindow(GetDlgItem(hwnd,104),FALSE);
          EnableWindow(GetDlgItem(hwnd,105),FALSE);
          EnableWindow(GetDlgItem(hwnd,106),FALSE);
          EnableWindow(GetDlgItem(hwnd,107),FALSE);
          break;
        case 104:
          EnableWindow(GetDlgItem(hwnd,103),IsDlgButtonChecked(hwnd,104) != BST_CHECKED);
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
//  FileVars_Init()
//
extern BOOL bNoEncodingTags;
extern int fNoFileVariables;

BOOL FileVars_Init(char *lpData,DWORD cbData,LPFILEVARS lpfv) {

  int i;
  char tch[LARGE_BUFFER];
  BOOL bDisableFileVariables = FALSE;

  ZeroMemory(lpfv,sizeof(FILEVARS));
  if ((fNoFileVariables && bNoEncodingTags) || !lpData || !cbData)
    return(TRUE);

  StringCchCopyNA(tch,COUNTOF(tch),lpData,min(cbData + 1,COUNTOF(tch)));

  if (!fNoFileVariables) {
    if (FileVars_ParseInt(tch,"enable-local-variables",&i) && (!i))
      bDisableFileVariables = TRUE;

    if (!bDisableFileVariables) {

      if (FileVars_ParseInt(tch,"tab-width",&i)) {
        lpfv->iTabWidth = max(min(i,256),1);
        lpfv->mask |= FV_TABWIDTH;
      }

      if (FileVars_ParseInt(tch,"c-basic-indent",&i)) {
        lpfv->iIndentWidth = max(min(i,256),0);
        lpfv->mask |= FV_INDENTWIDTH;
      }

      if (FileVars_ParseInt(tch,"indent-tabs-mode",&i)) {
        lpfv->bTabsAsSpaces = (i) ? FALSE : TRUE;
        lpfv->mask |= FV_TABSASSPACES;
      }

      if (FileVars_ParseInt(tch,"c-tab-always-indent",&i)) {
        lpfv->bTabIndents = (i) ? TRUE : FALSE;
        lpfv->mask |= FV_TABINDENTS;
      }

      if (FileVars_ParseInt(tch,"truncate-lines",&i)) {
        lpfv->fWordWrap = (i) ? FALSE : TRUE;
        lpfv->mask |= FV_WORDWRAP;
      }

      if (FileVars_ParseInt(tch,"fill-column",&i)) {
        lpfv->iLongLinesLimit = max(min(i,4096),0);
        lpfv->mask |= FV_LONGLINESLIMIT;
      }
    }
  }

  if (!IsUTF8Signature(lpData) && !bNoEncodingTags && !bDisableFileVariables) {

    if (FileVars_ParseStr(tch,"encoding",lpfv->tchEncoding,COUNTOF(lpfv->tchEncoding)))
      lpfv->mask |= FV_ENCODING;
    else if (FileVars_ParseStr(tch,"charset",lpfv->tchEncoding,COUNTOF(lpfv->tchEncoding)))
      lpfv->mask |= FV_ENCODING;
    else if (FileVars_ParseStr(tch,"coding",lpfv->tchEncoding,COUNTOF(lpfv->tchEncoding)))
      lpfv->mask |= FV_ENCODING;
  }

  if (!fNoFileVariables && !bDisableFileVariables) {
    if (FileVars_ParseStr(tch,"mode",lpfv->tchMode,COUNTOF(lpfv->tchMode)))
      lpfv->mask |= FV_MODE;
  }

  if (lpfv->mask == 0 && cbData > COUNTOF(tch)) {

    StringCchCopyNA(tch,COUNTOF(tch),lpData + cbData - COUNTOF(tch) + 1,COUNTOF(tch));

    if (!fNoFileVariables) {
      if (FileVars_ParseInt(tch,"enable-local-variables",&i) && (!i))
        bDisableFileVariables = TRUE;

      if (!bDisableFileVariables) {

        if (FileVars_ParseInt(tch,"tab-width",&i)) {
          lpfv->iTabWidth = max(min(i,256),1);
          lpfv->mask |= FV_TABWIDTH;
        }

        if (FileVars_ParseInt(tch,"c-basic-indent",&i)) {
          lpfv->iIndentWidth = max(min(i,256),0);
          lpfv->mask |= FV_INDENTWIDTH;
        }

        if (FileVars_ParseInt(tch,"indent-tabs-mode",&i)) {
          lpfv->bTabsAsSpaces = (i) ? FALSE : TRUE;
          lpfv->mask |= FV_TABSASSPACES;
        }

        if (FileVars_ParseInt(tch,"c-tab-always-indent",&i)) {
          lpfv->bTabIndents = (i) ? TRUE : FALSE;
          lpfv->mask |= FV_TABINDENTS;
        }

        if (FileVars_ParseInt(tch,"truncate-lines",&i)) {
          lpfv->fWordWrap = (i) ? FALSE : TRUE;
          lpfv->mask |= FV_WORDWRAP;
        }

        if (FileVars_ParseInt(tch,"fill-column",&i)) {
          lpfv->iLongLinesLimit = max(min(i,4096),0);
          lpfv->mask |= FV_LONGLINESLIMIT;
        }
      }
    }

    if (!IsUTF8Signature(lpData) && !bNoEncodingTags && !bDisableFileVariables) {

      if (FileVars_ParseStr(tch,"encoding",lpfv->tchEncoding,COUNTOF(lpfv->tchEncoding)))
        lpfv->mask |= FV_ENCODING;
      else if (FileVars_ParseStr(tch,"charset",lpfv->tchEncoding,COUNTOF(lpfv->tchEncoding)))
        lpfv->mask |= FV_ENCODING;
      else if (FileVars_ParseStr(tch,"coding",lpfv->tchEncoding,COUNTOF(lpfv->tchEncoding)))
        lpfv->mask |= FV_ENCODING;
    }

    if (!fNoFileVariables && !bDisableFileVariables) {
      if (FileVars_ParseStr(tch,"mode",lpfv->tchMode,COUNTOF(lpfv->tchMode)))
        lpfv->mask |= FV_MODE;
    }
  }

  if (lpfv->mask & FV_ENCODING)
    lpfv->iEncoding = Encoding_MatchA(lpfv->tchEncoding);

  return(TRUE);
}


//=============================================================================
//
//  FileVars_Apply()
//
extern int iTabWidth;
extern int iTabWidthG;
extern int iIndentWidth;
extern int iIndentWidthG;
extern BOOL bTabsAsSpaces;
extern BOOL bTabsAsSpacesG;
extern BOOL bTabIndents;
extern BOOL bTabIndentsG;
extern BOOL bWordWrap;
extern BOOL bWordWrapG;
extern int iWordWrapMode;
extern int iLongLinesLimit;
extern int iLongLinesLimitG;
extern int iWrapCol;

BOOL FileVars_Apply(HWND hwnd,LPFILEVARS lpfv) {

  if (lpfv->mask & FV_TABWIDTH)
    iTabWidth = lpfv->iTabWidth;
  else
    iTabWidth = iTabWidthG;
  SendMessage(hwnd,SCI_SETTABWIDTH,iTabWidth,0);

  if (lpfv->mask & FV_INDENTWIDTH)
    iIndentWidth = lpfv->iIndentWidth;
  else if (lpfv->mask & FV_TABWIDTH)
    iIndentWidth = 0;
  else
    iIndentWidth = iIndentWidthG;
  SendMessage(hwnd,SCI_SETINDENT,iIndentWidth,0);

  if (lpfv->mask & FV_TABSASSPACES)
    bTabsAsSpaces = lpfv->bTabsAsSpaces;
  else
    bTabsAsSpaces = bTabsAsSpacesG;
  SendMessage(hwnd,SCI_SETUSETABS,!bTabsAsSpaces,0);

  if (lpfv->mask & FV_TABINDENTS)
    bTabIndents = lpfv->bTabIndents;
  else
    bTabIndents = bTabIndentsG;
  SendMessage(hwndEdit,SCI_SETTABINDENTS,bTabIndents,0);

  if (lpfv->mask & FV_WORDWRAP)
    bWordWrap = lpfv->fWordWrap;
  else
    bWordWrap = bWordWrapG;
  if (!bWordWrap)
    SendMessage(hwndEdit,SCI_SETWRAPMODE,SC_WRAP_NONE,0);
  else
    SendMessage(hwndEdit,SCI_SETWRAPMODE,(iWordWrapMode == 0) ? SC_WRAP_WORD : SC_WRAP_CHAR,0);

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

  char tch[32] = { L'\0' };
  char chPrev;
  char *pvEnd;
  int  itok;

  char *pvStart = StrStrIA(pszData, pszName);
  while (pvStart) {
    chPrev = (pvStart > pszData) ? *(pvStart-1) : 0;
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

    StringCchCopyNA(tch,COUNTOF(tch),pvStart,COUNTOF(tch));

    pvEnd = tch;
    while (*pvEnd && IsCharAlphaNumericA(*pvEnd))
      pvEnd++;
    *pvEnd = 0;
    StrTrimA(tch," \t:=\"'");

    itok = sscanf_s(tch,"%i",piValue);
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

  char tch[32] = { L'\0' };
  char chPrev;
  char *pvEnd;
  BOOL bQuoted = FALSE;

  char *pvStart = StrStrIA(pszData, pszName);
  while (pvStart) {
    chPrev = (pvStart > pszData) ? *(pvStart-1) : 0;
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

    while (*pvStart && StrChrIA(":=\"' \t",*pvStart)) {
      if (*pvStart == '\'' || *pvStart == '"')
        bQuoted = TRUE;
      pvStart++;
    }
    StringCchCopyNA(tch,COUNTOF(tch),pvStart,COUNTOF(tch));

    pvEnd = tch;
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
    if (StringCchLenA(lpfv->tchEncoding) &&
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
    if ((mEncoding[lpfv->iEncoding].uFlags & NCP_INTERNAL) ||
         IsValidCodePage(mEncoding[lpfv->iEncoding].uCodePage) &&
         GetCPInfo(mEncoding[lpfv->iEncoding].uCodePage,&cpi)) {
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
