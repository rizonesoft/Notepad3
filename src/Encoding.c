/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Encoding.c                                                                  *
*   Handling and Helpers for File Encoding                                    *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*	                                                                            *
*	                                                                            *
*                                                                             *
*                                                  (c) Rizonesoft 2015-2018   *
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
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>
#include <stdlib.h>

#include "../uthash/utarray.h"

#include "scintilla.h"
#include "helpers.h"
#include "resource.h"
#include "encoding.h"


extern HMODULE   g_hLngResContainer;

//=============================================================================
//
//  Encoding Helper Functions
//

int g_DOSEncoding = CPI_NONE;
bool g_bForceCompEncDetection = false;

// Supported Encodings
WCHAR wchANSI[16] = { L'\0' };
WCHAR wchOEM[16] = { L'\0' };

// ============================================================================


int Encoding_Current(int iEncoding) 
{
  static int CurrentEncoding = CPI_NONE;

  if (iEncoding >= 0) {
    if (Encoding_IsValid(iEncoding))
      CurrentEncoding = iEncoding;
    else
      CurrentEncoding = CPI_UTF8;
  }
  return CurrentEncoding;
}
// ============================================================================


int Encoding_SrcCmdLn(int iSrcEncoding) 
{
  static int SourceEncoding = CPI_NONE;

  if (iSrcEncoding >= 0) {
    if (Encoding_IsValid(iSrcEncoding))
      SourceEncoding = iSrcEncoding;
    else
      SourceEncoding = CPI_ANSI_DEFAULT;
  }
  else if (iSrcEncoding == CPI_NONE) {
    SourceEncoding = CPI_NONE;
  }
  return SourceEncoding;
}
// ============================================================================


int  Encoding_SrcWeak(int iSrcWeakEnc) 
{
  static int SourceWeakEncoding = CPI_NONE;

  if (iSrcWeakEnc >= 0) {
    if (Encoding_IsValid(iSrcWeakEnc))
      SourceWeakEncoding = iSrcWeakEnc;
    else
      SourceWeakEncoding = CPI_ANSI_DEFAULT;
  }
  else if (iSrcWeakEnc == CPI_NONE) {
    SourceWeakEncoding = CPI_NONE;
  }
  return SourceWeakEncoding;
}
// ============================================================================


bool Encoding_HasChanged(int iOriginalEncoding) {
  static int OriginalEncoding = CPI_NONE;

  if (iOriginalEncoding >= CPI_NONE) {
    OriginalEncoding = iOriginalEncoding;
  }
  return (bool)(OriginalEncoding != Encoding_Current(CPI_GET));
}
// ============================================================================

void Encoding_InitDefaults()
{
  const UINT uCodePageMBCS[20] = {
    42, // (Symbol)
    50220,50221,50222,50225,50227,50229, // (Chinese, Japanese, Korean) 
    54936, // (GB18030)
    57002,57003,57004,57005,57006,57007,57008,57009,57010,57011, // (ISCII)
    65000, // (UTF-7)
    65001  // (UTF-8)
  };

  ChangeEncodingCodePage(CPI_ANSI_DEFAULT, GetACP()); // set ANSI system CP
  StringCchPrintf(wchANSI, COUNTOF(wchANSI), L" (CP-%u)", g_Encodings[CPI_ANSI_DEFAULT].uCodePage);

  for (int i = CPI_UTF7 + 1; i < Encoding_CountOf(); ++i) {
    if (Encoding_IsValid(i) && (g_Encodings[i].uCodePage == g_Encodings[CPI_ANSI_DEFAULT].uCodePage)) {
      g_Encodings[i].uFlags |= NCP_ANSI;
      if (g_Encodings[i].uFlags & NCP_EXTERNAL_8BIT)
        g_Encodings[CPI_ANSI_DEFAULT].uFlags |= NCP_EXTERNAL_8BIT;
      break;
    }
  }

  ChangeEncodingCodePage(CPI_OEM, GetOEMCP()); // set OEM system CP
  StringCchPrintf(wchOEM, COUNTOF(wchOEM), L" (CP-%u)", g_Encodings[CPI_OEM].uCodePage);

  for (int i = CPI_UTF7 + 1; i < Encoding_CountOf(); ++i) {
    if (Encoding_IsValid(i) && (g_Encodings[i].uCodePage == g_Encodings[CPI_OEM].uCodePage)) {
      g_Encodings[i].uFlags |= NCP_OEM;
      if (g_Encodings[i].uFlags & NCP_EXTERNAL_8BIT)
        g_Encodings[CPI_OEM].uFlags |= NCP_EXTERNAL_8BIT;
      break;
    }
  }

  // multi byte character sets
  for (int i = 0; i < Encoding_CountOf(); ++i) {
    for (int k = 0; k < COUNTOF(uCodePageMBCS); k++) {
      if (g_Encodings[i].uCodePage == uCodePageMBCS[k]) {
        g_Encodings[i].uFlags |= NCP_MBCS;
      }
    }
  }

  g_DOSEncoding = CPI_OEM;
  // Try to set the DOS encoding to DOS-437 if the default OEMCP is not DOS-437
  if (g_Encodings[g_DOSEncoding].uCodePage != 437) {
    for (int i = CPI_UTF7 + 1; i < Encoding_CountOf(); ++i) {
      if (Encoding_IsValid(i) && (g_Encodings[i].uCodePage == 437)) {
        g_DOSEncoding = i;
        break;
      }
    }
  }

}
// ============================================================================


int Encoding_MapIniSetting(bool bLoad, int iSetting) {
  if (bLoad) {
    switch (iSetting) {
    case -1: return CPI_NONE;
    case  0: return CPI_ANSI_DEFAULT;
    case  1: return CPI_UNICODEBOM;
    case  2: return CPI_UNICODEBEBOM;
    case  3: return CPI_UTF8;
    case  4: return CPI_UTF8SIGN;
    case  5: return CPI_OEM;
    case  6: return CPI_UNICODE;
    case  7: return CPI_UNICODEBE;
    case  8: return CPI_UTF7;
    default: {
      for (int i = CPI_UTF7 + 1; i < Encoding_CountOf(); i++) {
        if ((g_Encodings[i].uCodePage == (UINT)iSetting) && Encoding_IsValid(i))
          return(i);
      }
      return CPI_ANSI_DEFAULT;
    }
    }
  }
  else {
    switch (iSetting) {
    case CPI_NONE:         return -1;
    case CPI_ANSI_DEFAULT: return  0;
    case CPI_UNICODEBOM:   return  1;
    case CPI_UNICODEBEBOM: return  2;
    case CPI_UTF8:         return  3;
    case CPI_UTF8SIGN:     return  4;
    case CPI_OEM:          return  5;
    case CPI_UNICODE:      return  6;
    case CPI_UNICODEBE:    return  7;
    case CPI_UTF7:         return  8;
    default: {
      if (Encoding_IsValid(iSetting))
        return(g_Encodings[iSetting].uCodePage);
      else
        return CPI_ANSI_DEFAULT;
    }
    }
  }
}
// ============================================================================


int Encoding_MapUnicode(int iUni) {

  if (iUni == CPI_UNICODEBOM)
    return CPI_UNICODE;
  else if (iUni == CPI_UNICODEBEBOM)
    return CPI_UNICODEBE;
  else if (iUni == CPI_UTF8SIGN)
    return CPI_UTF8;
  else
    return iUni;
}
// ============================================================================


void Encoding_SetLabel(int iEncoding) {
  if (g_Encodings[iEncoding].wchLabel[0] == L'\0') {
    WCHAR wch1[128] = { L'\0' };
    WCHAR wch2[128] = { L'\0' };
    GetLngString(g_Encodings[iEncoding].idsName, wch1, COUNTOF(wch1));
    WCHAR *pwsz = StrChr(wch1, L';');
    if (pwsz) {
      pwsz = StrChr(CharNext(pwsz), L';');
      if (pwsz) {
        pwsz = CharNext(pwsz);
      }
    }
    if (!pwsz)
      pwsz = wch1;

    StringCchCopyN(wch2, COUNTOF(wch2), pwsz, COUNTOF(wch1));

    if (Encoding_IsANSI(iEncoding))
      StringCchCatN(wch2, COUNTOF(wch2), wchANSI, COUNTOF(wchANSI));
    else if (Encoding_IsOEM(iEncoding))
      StringCchCatN(wch2, COUNTOF(wch2), wchOEM, COUNTOF(wchOEM));

    StringCchCopyN(g_Encodings[iEncoding].wchLabel, COUNTOF(g_Encodings[iEncoding].wchLabel),
      wch2, COUNTOF(g_Encodings[iEncoding].wchLabel));
  }
}
// ============================================================================


int Encoding_MatchW(LPCWSTR pwszTest) {
  char tchTest[256] = { '\0' };
  WideCharToMultiByteStrg(CP_ACP, pwszTest, tchTest);
  return(Encoding_MatchA(tchTest));
}
// ============================================================================


int Encoding_MatchA(char *pchTest) {
  char  chTest[256] = { '\0' };
  char *pchSrc = pchTest;
  char *pchDst = chTest;
  *pchDst++ = ',';
  while (*pchSrc) {
    if (IsCharAlphaNumericA(*pchSrc))
      *pchDst++ = *CharLowerA(pchSrc);
    pchSrc++;
  }
  *pchDst++ = ',';
  *pchDst = 0;
  for (int i = 0; i < Encoding_CountOf(); i++) {
    if (StrStrIA(g_Encodings[i].pszParseNames, chTest)) {
      CPINFO cpi;
      if ((g_Encodings[i].uFlags & NCP_INTERNAL) ||
        IsValidCodePage(g_Encodings[i].uCodePage) &&
        GetCPInfo(g_Encodings[i].uCodePage, &cpi))
        return(i);
      else
        return(-1);
    }
  }
  return(-1);
}
// ============================================================================


int Encoding_GetByCodePage(UINT cp) {
  for (int i = 0; i < Encoding_CountOf(); i++) {
    if (cp == g_Encodings[i].uCodePage) {
      return i;
    }
  }
  return CPI_ANSI_DEFAULT;
}
// ============================================================================


bool Encoding_IsValid(int iTestEncoding) {
  CPINFO cpi;
  if ((iTestEncoding >= 0) && (iTestEncoding < Encoding_CountOf())) {
    if ((g_Encodings[iTestEncoding].uFlags & NCP_INTERNAL) ||
      IsValidCodePage(g_Encodings[iTestEncoding].uCodePage) &&
      GetCPInfo(g_Encodings[iTestEncoding].uCodePage, &cpi)) {
      return(true);
    }
  }
  return(false);
}
// ============================================================================


typedef struct _ee {
  int    id;
  WCHAR  wch[256];
} ENCODINGENTRY, *PENCODINGENTRY;

int CmpEncoding(const void *s1, const void *s2) {
  return StrCmp(((PENCODINGENTRY)s1)->wch, ((PENCODINGENTRY)s2)->wch);
}
// ============================================================================


void Encoding_AddToListView(HWND hwnd, int idSel, bool bRecodeOnly) {
  int i;
  int iSelItem = -1;
  LVITEM lvi;
  WCHAR wchBuf[256] = { L'\0' };

  PENCODINGENTRY pEE = LocalAlloc(LPTR, Encoding_CountOf() * sizeof(ENCODINGENTRY));
  for (i = 0; i < Encoding_CountOf(); i++) {
    pEE[i].id = i;
    GetLngString(g_Encodings[i].idsName, pEE[i].wch, COUNTOF(pEE[i].wch));
  }
  qsort(pEE, Encoding_CountOf(), sizeof(ENCODINGENTRY), CmpEncoding);

  ZeroMemory(&lvi, sizeof(LVITEM));
  lvi.mask = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE;
  lvi.pszText = wchBuf;

  for (i = 0; i < Encoding_CountOf(); i++) {

    int id = pEE[i].id;
    if (!bRecodeOnly || (g_Encodings[id].uFlags & NCP_RECODE)) {

      lvi.iItem = ListView_GetItemCount(hwnd);

      WCHAR *pwsz = StrChr(pEE[i].wch, L';');
      if (pwsz) {
        StringCchCopyN(wchBuf, COUNTOF(wchBuf), CharNext(pwsz), COUNTOF(wchBuf));
        pwsz = StrChr(wchBuf, L';');
        if (pwsz)
          *pwsz = 0;
      }
      else
        StringCchCopyN(wchBuf, COUNTOF(wchBuf), pEE[i].wch, COUNTOF(wchBuf));

      if (Encoding_IsANSI(id))
        StringCchCatN(wchBuf, COUNTOF(wchBuf), wchANSI, COUNTOF(wchANSI));
      else if (Encoding_IsOEM(id))
        StringCchCatN(wchBuf, COUNTOF(wchBuf), wchOEM, COUNTOF(wchOEM));

      if (Encoding_IsValid(id))
        lvi.iImage = 0;
      else
        lvi.iImage = 1;

      lvi.lParam = (LPARAM)id;
      ListView_InsertItem(hwnd, &lvi);

      if (idSel == id)
        iSelItem = lvi.iItem;
    }
  }

  LocalFree(pEE);

  if (iSelItem != -1) {
    ListView_SetItemState(hwnd, iSelItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    ListView_EnsureVisible(hwnd, iSelItem, false);
  }
  else {
    ListView_SetItemState(hwnd, 0, LVIS_FOCUSED, LVIS_FOCUSED);
    ListView_EnsureVisible(hwnd, 0, false);
  }
}
// ============================================================================


bool Encoding_GetFromListView(HWND hwnd, int *pidEncoding) {
  LVITEM lvi;

  lvi.iItem = ListView_GetNextItem(hwnd, -1, LVNI_ALL | LVNI_SELECTED);
  lvi.iSubItem = 0;
  lvi.mask = LVIF_PARAM;

  if (ListView_GetItem(hwnd, &lvi)) {
    if (Encoding_IsValid((int)lvi.lParam))
      *pidEncoding = (int)lvi.lParam;
    else
      *pidEncoding = -1;

    return (true);
  }
  return(false);
}
// ============================================================================


void Encoding_AddToComboboxEx(HWND hwnd, int idSel, bool bRecodeOnly) {
  int i;
  int iSelItem = -1;
  COMBOBOXEXITEM cbei;
  WCHAR wchBuf[256] = { L'\0' };

  PENCODINGENTRY pEE = LocalAlloc(LPTR, Encoding_CountOf() * sizeof(ENCODINGENTRY));
  for (i = 0; i < Encoding_CountOf(); i++) {
    pEE[i].id = i;
    GetLngString(g_Encodings[i].idsName, pEE[i].wch, COUNTOF(pEE[i].wch));
  }
  qsort(pEE, Encoding_CountOf(), sizeof(ENCODINGENTRY), CmpEncoding);

  ZeroMemory(&cbei, sizeof(COMBOBOXEXITEM));
  cbei.mask = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_LPARAM;
  cbei.pszText = wchBuf;
  cbei.cchTextMax = COUNTOF(wchBuf);
  cbei.iImage = 0;
  cbei.iSelectedImage = 0;

  for (i = 0; i < Encoding_CountOf(); i++) {

    int id = pEE[i].id;
    if (!bRecodeOnly || (g_Encodings[id].uFlags & NCP_RECODE)) {

      cbei.iItem = SendMessage(hwnd, CB_GETCOUNT, 0, 0);

      WCHAR *pwsz = StrChr(pEE[i].wch, L';');
      if (pwsz) {
        StringCchCopyN(wchBuf, COUNTOF(wchBuf), CharNext(pwsz), COUNTOF(wchBuf));
        pwsz = StrChr(wchBuf, L';');
        if (pwsz)
          *pwsz = 0;
      }
      else
        StringCchCopyN(wchBuf, COUNTOF(wchBuf), pEE[i].wch, COUNTOF(wchBuf));

      if (Encoding_IsANSI(id))
        StringCchCatN(wchBuf, COUNTOF(wchBuf), wchANSI, COUNTOF(wchANSI));
      else if (id == CPI_OEM)
        StringCchCatN(wchBuf, COUNTOF(wchBuf), wchOEM, COUNTOF(wchOEM));

      cbei.iImage = (Encoding_IsValid(id) ? 0 : 1);

      cbei.lParam = (LPARAM)id;
      SendMessage(hwnd, CBEM_INSERTITEM, 0, (LPARAM)&cbei);

      if (idSel == id)
        iSelItem = (int)cbei.iItem;
    }
  }

  LocalFree(pEE);

  if (iSelItem != -1)
    SendMessage(hwnd, CB_SETCURSEL, (WPARAM)iSelItem, 0);
}
// ============================================================================


bool Encoding_GetFromComboboxEx(HWND hwnd, int *pidEncoding) {
  COMBOBOXEXITEM cbei;

  cbei.iItem = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
  cbei.mask = CBEIF_LPARAM;

  if (SendMessage(hwnd, CBEM_GETITEM, 0, (LPARAM)&cbei)) {
    if (Encoding_IsValid((int)cbei.lParam))
      *pidEncoding = (int)cbei.lParam;
    else
      *pidEncoding = -1;

    return (true);
  }
  return(false);
}
// ============================================================================


UINT Encoding_GetCodePage(int iEncoding) {
  return (iEncoding >= 0) ? g_Encodings[iEncoding].uCodePage : CP_ACP;
}
// ============================================================================

bool Encoding_IsDefault(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_DEFAULT) : false;
}
// ============================================================================

bool Encoding_IsANSI(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_ANSI) : false;
}
// ============================================================================

bool Encoding_IsOEM(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_OEM) : false;
}
// ============================================================================

bool Encoding_IsUTF8(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_UTF8) : false;
}
// ============================================================================

bool Encoding_IsUTF8_SIGN(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_UTF8_SIGN) : false;
}
// ============================================================================

bool Encoding_IsMBCS(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_MBCS) : false;
}
// ============================================================================

bool Encoding_IsUNICODE(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_UNICODE) : false;
}
// ============================================================================

bool Encoding_IsUNICODE_BOM(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_UNICODE_BOM) : false;
}
// ============================================================================

bool Encoding_IsUNICODE_REVERSE(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_UNICODE_REVERSE) : false;
}
// ============================================================================


bool Encoding_IsINTERNAL(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_INTERNAL) : false;
}
// ============================================================================

bool Encoding_IsEXTERNAL_8BIT(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_EXTERNAL_8BIT) : false;
}
// ============================================================================

bool Encoding_IsRECODE(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_RECODE) : false;
}
// ============================================================================


void Encoding_SetDefaultFlag(int iEncoding) {
  if (iEncoding >= 0)
    g_Encodings[iEncoding].uFlags |= NCP_DEFAULT;
}
// ============================================================================


const WCHAR* Encoding_GetLabel(int iEncoding) {
  return (iEncoding >= 0) ? g_Encodings[iEncoding].wchLabel : NULL;
}
// ============================================================================

const char* Encoding_GetParseNames(int iEncoding) {
  return (iEncoding >= 0) ? g_Encodings[iEncoding].pszParseNames : NULL;
}
// ============================================================================


bool IsUnicode(const char* pBuffer, size_t cb, bool* lpbBOM, bool* lpbReverse) {
  int i = 0xFFFF;

  bool bIsTextUnicode;

  bool bHasBOM;
  bool bHasRBOM;

  if (!pBuffer || cb < 2)
    return false;

  bIsTextUnicode = IsTextUnicode(pBuffer, (int)cb, &i);

  bHasBOM = (*((UNALIGNED PWCHAR)pBuffer) == 0xFEFF);
  bHasRBOM = (*((UNALIGNED PWCHAR)pBuffer) == 0xFFFE);

  if (i == 0xFFFF) // i doesn't seem to have been modified ...
    i = 0;

  if (bIsTextUnicode || bHasBOM || bHasRBOM ||
    ((i & (IS_TEXT_UNICODE_UNICODE_MASK | IS_TEXT_UNICODE_REVERSE_MASK)) &&
      !((i & IS_TEXT_UNICODE_UNICODE_MASK) && (i & IS_TEXT_UNICODE_REVERSE_MASK)) &&
      !(i & IS_TEXT_UNICODE_ODD_LENGTH) &&
      !(i & IS_TEXT_UNICODE_ILLEGAL_CHARS && !(i & IS_TEXT_UNICODE_REVERSE_SIGNATURE)) &&
      !((i & IS_TEXT_UNICODE_REVERSE_MASK) == IS_TEXT_UNICODE_REVERSE_STATISTICS))) {

    if (lpbBOM)
      *lpbBOM = (bHasBOM || bHasRBOM ||
      (i & (IS_TEXT_UNICODE_SIGNATURE | IS_TEXT_UNICODE_REVERSE_SIGNATURE)))
      ? true : false;

    if (lpbReverse)
      *lpbReverse = (bHasRBOM || (i & IS_TEXT_UNICODE_REVERSE_MASK)) ? true : false;

    return true;
  }

  else

    return false;
}
// ============================================================================


bool IsUTF7(const char* pTest, size_t nLength) {
  const char *pt = pTest;

  for (size_t i = 0; i < nLength; i++) {
    if (*pt & 0x80 || !*pt)
      return false;
    pt++;
  }

  return true;
}
// ============================================================================


#undef _OLD_UTF8_VALIDATOR_
//#define _OLD_UTF8_VALIDATOR_ 1
#ifdef _OLD_UTF8_VALIDATOR_

bool IsUTF8(const char* pTest, size_t nLength)
{
  static int byte_class_table[256] = {
    /*       00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  */
    /* 00 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 10 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 20 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 30 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 40 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 50 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 60 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 70 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 80 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 90 */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    /* A0 */ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    /* B0 */ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    /* C0 */ 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    /* D0 */ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    /* E0 */ 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 7, 7,
    /* F0 */ 9,10,10,10,11, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4
    /*       00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  */ };

  /* state table */
  typedef enum {
    kSTART = 0, kA, kB, kC, kD, kE, kF, kG, kERROR, kNumOfStates
  } utf8_state;

  static utf8_state state_table[] = {
    /*                            kSTART, kA,     kB,     kC,     kD,     kE,     kF,     kG,     kERROR */
    /* 0x00-0x7F: 0            */ kSTART, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0x80-0x8F: 1            */ kERROR, kSTART, kA,     kERROR, kA,     kB,     kERROR, kB,     kERROR,
    /* 0x90-0x9f: 2            */ kERROR, kSTART, kA,     kERROR, kA,     kB,     kB,     kERROR, kERROR,
    /* 0xa0-0xbf: 3            */ kERROR, kSTART, kA,     kA,     kERROR, kB,     kB,     kERROR, kERROR,
    /* 0xc0-0xc1, 0xf5-0xff: 4 */ kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0xc2-0xdf: 5            */ kA,     kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0xe0: 6                 */ kC,     kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0xe1-0xec, 0xee-0xef: 7 */ kB,     kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0xed: 8                 */ kD,     kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0xf0: 9                 */ kF,     kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0xf1-0xf3: 10           */ kE,     kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0xf4: 11                */ kG,     kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR };

#define BYTE_CLASS(b) (byte_class_table[(unsigned char)b])
#define NEXT_STATE(b,cur) (state_table[(BYTE_CLASS(b) * kNumOfStates) + (cur)])

  utf8_state current = kSTART;

  const char* pt = pTest;
  size_t len = nLength;

  for (size_t i = 0; i < len; i++, pt++) {

    current = NEXT_STATE(*pt, current);
    if (kERROR == current)
      break;
  }

  return (current == kSTART) ? true : false;
}

// ============================================================================

/* byte length of UTF-8 sequence based on value of first byte.
for UTF-16 (21-bit space), max. code length is 4, so we only need to look
at 4 upper bits.
*/
static const size_t utf8_lengths[16] =
{
  1,1,1,1,1,1,1,1,        /* 0000 to 0111 : 1 byte (plain ASCII) */
  0,0,0,0,                /* 1000 to 1011 : not valid */
  2,2,                    /* 1100, 1101 : 2 bytes */
  3,                      /* 1110 : 3 bytes */
  4                       /* 1111 : 4 bytes */
};

// ----------------------------------------------------------------------------

/*++
Function :
UTF8_mbslen_bytes [INTERNAL]

Calculates the byte size of a NULL-terminated UTF-8 string.

Parameters :
char *utf8_string : string to examine

Return value :
size (in bytes) of a NULL-terminated UTF-8 string.
-1 if invalid NULL-terminated UTF-8 string
--*/
size_t UTF8_mbslen_bytes(LPCSTR utf8_string)
{
  size_t length = 0;
  size_t code_size;
  BYTE byte;

  while (*utf8_string)
  {
    byte = (BYTE)*utf8_string;

    if ((byte <= 0xF7) && (0 != (code_size = utf8_lengths[byte >> 4]))) {
      length += code_size;
      utf8_string += code_size;
    }
    else {
      /* we got an invalid byte value but need to count it,
      it will be later ignored during the string conversion */
      //WARN("invalid first byte value 0x%02X in UTF-8 sequence!\n",byte);
      length++;
      utf8_string++;
    }
  }
  length++; /* include NULL terminator */
  return length;
}
// ----------------------------------------------------------------------------

/*++
Function :
UTF8_mbslen [INTERNAL]

Calculates the character size of a NULL-terminated UTF-8 string.

Parameters :
char *utf8_string : string to examine
int byte_length : byte size of string

Return value :
size (in characters) of a UTF-8 string.
-1 if invalid UTF-8 string
--*/
size_t UTF8_mbslen(LPCSTR utf8_string, size_t byte_length)
{
  size_t wchar_length = 0;
  size_t code_size;
  BYTE byte;

  while (byte_length > 0) {
    byte = (BYTE)*utf8_string;

    /* UTF-16 can't encode 5-byte and 6-byte sequences, so maximum value
    for first byte is 11110111. Use lookup table to determine sequence
    length based on upper 4 bits of first byte */
    if ((byte <= 0xF7) && (0 != (code_size = utf8_lengths[byte >> 4]))) {
      /* 1 sequence == 1 character */
      wchar_length++;

      if (code_size == 4)
        wchar_length++;

      utf8_string += code_size;        /* increment pointer */
      byte_length -= code_size;   /* decrement counter*/
    }
    else {
      /*
      unlike UTF8_mbslen_bytes, we ignore the invalid characters.
      we only report the number of valid characters we have encountered
      to match the Windows behavior.
      */
      //WARN("invalid byte 0x%02X in UTF-8 sequence, skipping it!\n", byte);
      utf8_string++;
      byte_length--;
    }
  }
  return wchar_length;
}
// ----------------------------------------------------------------------------

bool UTF8_ContainsInvalidChars(LPCSTR utf8_string, size_t byte_length)
{
  return ((UTF8_mbslen_bytes(UTF8StringStart(utf8_string)) - 1) !=
    UTF8_mbslen(UTF8StringStart(utf8_string), IsUTF8Signature(utf8_string) ? (byte_length - 3) : byte_length));
}


// ============================================================================
#else  // new UTF-8 validator
// ============================================================================


// Copyright (c) 2008-2010 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.


enum {
  UTF8_ACCEPT = 0,
  UTF8_REJECT = 12,
  UTF8_NOTEST = 113
};

static UINT s_State = UTF8_NOTEST;

bool IsUTF8(const char* pTest, size_t nLength)
{
  static const unsigned char utf8_dfa[] = {
    // The first part of the table maps bytes to character classes that
    // to reduce the size of the transition table and create bitmasks.
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
     7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
     8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,

    // The second part is a transition table that maps a combination
    // of a state of the automaton and a character class to a state.
     0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
    12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
    12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
    12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
    12,36,12,12,12,12,12,12,12,12,12,12,
  };

  const unsigned char *pt = (const unsigned char *)pTest;
  const unsigned char *end = pt + nLength;

  s_State = UTF8_ACCEPT;
  while (pt < end && *pt) {
    s_State = utf8_dfa[256 + s_State + utf8_dfa[*pt++]];
    if (s_State == UTF8_REJECT) {
      return false;
    }
  }
  return (s_State == UTF8_ACCEPT);
}

// ----------------------------------------------------------------------------

bool UTF8_ContainsInvalidChars(LPCSTR utf8_string, size_t byte_length)
{
  bool result = true;
  if (s_State != UTF8_NOTEST) {
    result = (s_State == UTF8_REJECT);
  }
  else {
    result = IsUTF8(utf8_string, byte_length);
  }
  s_State = UTF8_NOTEST; // reset: old way, call IsUTF8() before 
  return result;
}


// ----------------------------------------------------------------------------

#endif

// ============================================================================




