// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Encoding.c                                                                  *
*   Handling and Helpers for File Encoding                                    *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                                             *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2020   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

#include "Helpers.h"

#include <shellapi.h>
#include <commctrl.h>
#include <stdlib.h>
#include <assert.h>

#include "../uthash/utarray.h"

#include "resource.h"
#include "Encoding.h"
#include "MuiLanguage.h"

#include "Scintilla.h"


// ============================================================================

// Supported Encodings
WCHAR wchANSI[16] = { L'\0' };
WCHAR wchOEM[16] = { L'\0' };

// ============================================================================

cpi_enc_t Encoding_Current(cpi_enc_t iEncoding)
{
  static cpi_enc_t CurrentEncoding = CPI_NONE;

  if (iEncoding >= CPI_NONE) {
    if (Encoding_IsValid(iEncoding)) {
      CurrentEncoding = iEncoding;
    }
    else {
      CurrentEncoding = CPI_PREFERRED_ENCODING;
    }
  }
  return CurrentEncoding;
}
// ============================================================================


cpi_enc_t Encoding_Forced(cpi_enc_t iEncoding)
{
  static cpi_enc_t SourceEncoding = CPI_NONE;

  if (iEncoding >= 0) {
    if (Encoding_IsValid(iEncoding))
      SourceEncoding = iEncoding;
    else
      SourceEncoding = CPI_ANSI_DEFAULT;
  }
  else if (iEncoding == CPI_NONE) {
    SourceEncoding = CPI_NONE;
  }
  return SourceEncoding;
}
// ============================================================================


cpi_enc_t  Encoding_SrcWeak(cpi_enc_t iSrcWeakEnc)
{
  static cpi_enc_t SourceWeakEncoding = CPI_NONE;

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

  UINT const ansiCP = CodePageFromCharSet(ANSI_CHARSET);
  ChangeEncodingCodePage(CPI_ANSI_DEFAULT, ansiCP); // set ANSI system CP ()
  assert(g_Encodings[CPI_ANSI_DEFAULT].uCodePage == ansiCP);
  StringCchPrintf(wchANSI, COUNTOF(wchANSI), L" (CP-%u)", ansiCP);

  Globals.bIsCJKInputCodePage = IsDBCSCodePage(Scintilla_InputCodePage());

  for (cpi_enc_t i = CPI_UTF7 + 1; i < Encoding_CountOf(); ++i) {
    if (Encoding_IsValid(i) && (g_Encodings[i].uCodePage == g_Encodings[CPI_ANSI_DEFAULT].uCodePage)) {
      g_Encodings[i].uFlags |= NCP_ANSI;
      if (g_Encodings[i].uFlags & NCP_EXTERNAL_8BIT)
        g_Encodings[CPI_ANSI_DEFAULT].uFlags |= NCP_EXTERNAL_8BIT;
      break;
    }
  }

  ChangeEncodingCodePage(CPI_OEM, GetOEMCP()); // set OEM system CP
  StringCchPrintf(wchOEM, COUNTOF(wchOEM), L" (CP-%u)", g_Encodings[CPI_OEM].uCodePage);

  for (cpi_enc_t i = CPI_UTF7 + 1; i < Encoding_CountOf(); ++i) {
    if (Encoding_IsValid(i) && (g_Encodings[i].uCodePage == g_Encodings[CPI_OEM].uCodePage)) {
      g_Encodings[i].uFlags |= NCP_OEM;
      if (g_Encodings[i].uFlags & NCP_EXTERNAL_8BIT)
        g_Encodings[CPI_OEM].uFlags |= NCP_EXTERNAL_8BIT;
      break;
    }
  }

  // multi byte character sets
  for (cpi_enc_t i = 0; i < Encoding_CountOf(); ++i) {
    for (int k = 0; k < COUNTOF(uCodePageMBCS); k++) {
      if (g_Encodings[i].uCodePage == uCodePageMBCS[k]) {
        g_Encodings[i].uFlags |= NCP_MBCS;
      }
    }
  }

  Globals.DOSEncoding = CPI_OEM;
  // Try to set the DOS encoding to DOS-437 if the default OEMCP is not DOS-437
  if (g_Encodings[Globals.DOSEncoding].uCodePage != 437) {
    for (cpi_enc_t cpi = CPI_UTF7 + 1; cpi < Encoding_CountOf(); ++cpi) {
      if (Encoding_IsValid(cpi) && (g_Encodings[cpi].uCodePage == 437)) {
        Globals.DOSEncoding = cpi;
        break;
      }
    }
  }

}
// ============================================================================


int Encoding_MapIniSetting(bool bLoad, int iSetting)
{
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
      for (cpi_enc_t i = CPI_UTF7 + 1; i < Encoding_CountOf(); i++) {
        if ((g_Encodings[i].uCodePage == (UINT)iSetting) && Encoding_IsValid(i))
          return (int)i;
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
    default:
      if (Encoding_IsValid((cpi_enc_t)iSetting)) {
        return (int)g_Encodings[iSetting].uCodePage;
      }
      return CPI_ANSI_DEFAULT;
    }
  }
}
// ============================================================================


cpi_enc_t Encoding_MapSignature(cpi_enc_t iUni)
{
  if (iUni == CPI_UTF8SIGN) {
    return CPI_UTF8;
  }
  if (iUni == CPI_UNICODEBOM) {
    return CPI_UNICODE;
  }
  if (iUni == CPI_UNICODEBEBOM) {
    return CPI_UNICODEBE;
  }
  return iUni;
}
// ============================================================================


void Encoding_SetLabel(cpi_enc_t iEncoding)
{
  WCHAR wch1[128] = { L'\0' };
  GetLngString(g_Encodings[iEncoding].idsName, wch1, COUNTOF(wch1));

  // point to correct label in list
  WCHAR* pwsz = StrChr(wch1, L';');
  if (pwsz) {
    pwsz = StrChr(CharNext(pwsz), L';');
    if (pwsz) {
      pwsz = CharNext(pwsz);
    }
  }
  if (!pwsz)
    pwsz = wch1;

  WCHAR wch2[128] = { L'\0' };
  StringCchCopyN(wch2, COUNTOF(wch2), pwsz, COUNTOF(wch1));

  if (Encoding_IsANSI(iEncoding)) {
    StringCchCatN(wch2, COUNTOF(wch2), wchANSI, COUNTOF(wchANSI));
  }
  else if (Encoding_IsOEM(iEncoding)) {
    StringCchCatN(wch2, COUNTOF(wch2), wchOEM, COUNTOF(wchOEM));
  }

  StringCchCopyN(g_Encodings[iEncoding].wchLabel, COUNTOF(g_Encodings[iEncoding].wchLabel), wch2, COUNTOF(wch2));
}
// ============================================================================


cpi_enc_t Encoding_MatchW(LPCWSTR pwszTest)
{
  char tchTest[256] = { '\0' };
  WideCharToMultiByteEx(CP_ACP, 0, pwszTest, -1, tchTest, COUNTOF(tchTest), NULL, NULL);
  return Encoding_MatchA(tchTest);
}
// ============================================================================


cpi_enc_t Encoding_MatchA(const char *pchTest)
{
  char chTestLC[256];
  chTestLC[0] = ',';
  chTestLC[1] = '\0';
  StringCchCatA(chTestLC, 256, pchTest);
  CharLowerA(chTestLC);
  StringCchCatA(chTestLC, 256, ","); // parsing incl. comma
  for (cpi_enc_t cpiEncId = 0; cpiEncId < Encoding_CountOf(); cpiEncId++) {
    if (StrStrIA(g_Encodings[cpiEncId].pszParseNames, chTestLC)) {
      CPINFO cpi;
      if ((g_Encodings[cpiEncId].uFlags & NCP_INTERNAL) ||
        (IsValidCodePage(g_Encodings[cpiEncId].uCodePage) &&
         GetCPInfo(g_Encodings[cpiEncId].uCodePage, &cpi))) {
        return cpiEncId;
      }
      return CPI_NONE;
    }
  }
  return CPI_NONE;
}
// ============================================================================



cpi_enc_t Encoding_GetByCodePage(const UINT codepage) 
{
  for (cpi_enc_t cpi = 0; cpi < Encoding_CountOf(); cpi++) {
    if (codepage == g_Encodings[cpi].uCodePage) {
      return cpi;
    }
  }
  return CPI_ANSI_DEFAULT;
}
// ============================================================================


bool Encoding_IsValid(cpi_enc_t iTestEncoding) {
  CPINFO cpi;
  if (Encoding_IsValidIdx(iTestEncoding)) {
    if ((g_Encodings[iTestEncoding].uFlags & NCP_INTERNAL) ||
      (IsValidCodePage(g_Encodings[iTestEncoding].uCodePage) &&
       GetCPInfo(g_Encodings[iTestEncoding].uCodePage, &cpi))) {
      return true;
    }
  }
  return false;
}
// ============================================================================


typedef struct _ee {
  cpi_enc_t id;
  WCHAR     wch[256];
} 
ENCODINGENTRY, *PENCODINGENTRY;

int CmpEncoding(const void *s1, const void *s2) {
  return wcscmp_s(((const PENCODINGENTRY)s1)->wch, ((const PENCODINGENTRY)s2)->wch);
}
// ============================================================================


void Encoding_AddToListView(HWND hwnd, cpi_enc_t idSel, bool bRecodeOnly) {
  int iSelItem = -1;
  LVITEM lvi;
  WCHAR wchBuf[256] = { L'\0' };

  PENCODINGENTRY pEE = AllocMem(Encoding_CountOf() * sizeof(ENCODINGENTRY), HEAP_ZERO_MEMORY);
  if (pEE) {
    for (cpi_enc_t i = 0; i < Encoding_CountOf(); i++) {
      pEE[i].id = i;
      GetLngString(g_Encodings[i].idsName, pEE[i].wch, COUNTOF(pEE[i].wch));
    }
    qsort(pEE, Encoding_CountOf(), sizeof(ENCODINGENTRY), CmpEncoding);

    ZeroMemory(&lvi, sizeof(LVITEM));
    lvi.mask = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE;
    lvi.pszText = wchBuf;

    for (int i = 0; i < Encoding_CountOf(); i++) {

      cpi_enc_t id = pEE[i].id;
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
    FreeMem(pEE);
  }
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


bool Encoding_GetFromListView(HWND hwnd, cpi_enc_t* pidEncoding) 
{
  LVITEM lvi;
  ZeroMemory(&lvi, sizeof(LVITEM));
  lvi.iItem = ListView_GetNextItem(hwnd, -1, LVNI_ALL | LVNI_SELECTED);
  lvi.iSubItem = 0;
  lvi.mask = LVIF_PARAM;

  if (ListView_GetItem(hwnd, &lvi)) {
    if (Encoding_IsValid((cpi_enc_t)lvi.lParam)) {
      *pidEncoding = (cpi_enc_t)lvi.lParam;
    }
    else {
      *pidEncoding = CPI_NONE;
    }
    return true;
  }
  return false;
}
// ============================================================================


void Encoding_AddToComboboxEx(HWND hwnd, cpi_enc_t idSel, bool bRecodeOnly)
{
  int iSelItem = -1;
  COMBOBOXEXITEM cbei;
  WCHAR wchBuf[256] = { L'\0' };

  PENCODINGENTRY pEE = AllocMem(Encoding_CountOf() * sizeof(ENCODINGENTRY), HEAP_ZERO_MEMORY);
  if (pEE) {
    for (cpi_enc_t i = 0; i < Encoding_CountOf(); i++) {
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

    for (int i = 0; i < Encoding_CountOf(); i++) {

      cpi_enc_t id = pEE[i].id;
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
    FreeMem(pEE);
  }
  if (iSelItem != -1) {
    SendMessage(hwnd, CB_SETCURSEL, (WPARAM)iSelItem, 0);
  }
}
// ============================================================================


bool Encoding_GetFromComboboxEx(HWND hwnd, cpi_enc_t* pidEncoding) 
{
  COMBOBOXEXITEM cbei;
  ZeroMemory(&cbei, sizeof(COMBOBOXEXITEM));
  cbei.iItem = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
  cbei.mask = CBEIF_LPARAM;

  if (SendMessage(hwnd, CBEM_GETITEM, 0, (LPARAM)&cbei)) {
    if (Encoding_IsValid((cpi_enc_t)cbei.lParam)) {
      *pidEncoding = (cpi_enc_t)cbei.lParam;
    }
    else {
      *pidEncoding = -1;
    }
    return true;
  }
  return false;
}
// ============================================================================


UINT Encoding_GetCodePage(const cpi_enc_t iEncoding) {
  return (iEncoding >= 0) ? g_Encodings[iEncoding].uCodePage : CP_ACP;
}
// ============================================================================

bool Encoding_IsDefault(const cpi_enc_t iEncoding) {
  return  (iEncoding >= 0) ? ((g_Encodings[iEncoding].uFlags & NCP_DEFAULT) != 0) : (iEncoding == CPI_ASCII_7BIT);
}
// ============================================================================

bool Encoding_IsASCII(const cpi_enc_t iEncoding) {
  return  (iEncoding >= 0) ? ((g_Encodings[iEncoding].uFlags & NCP_ASCII_7BIT) != 0) : (iEncoding == CPI_ASCII_7BIT);
}
// ============================================================================

bool Encoding_IsANSI(const cpi_enc_t iEncoding) {
  return  (iEncoding >= 0) ? ((g_Encodings[iEncoding].uFlags & NCP_ANSI) != 0) : (iEncoding == CPI_ASCII_7BIT);
}
// ============================================================================

bool Encoding_IsOEM(const cpi_enc_t iEncoding) {
  return  (iEncoding >= 0) ? ((g_Encodings[iEncoding].uFlags & NCP_OEM) != 0) : (iEncoding == CPI_ASCII_7BIT);
}
// ============================================================================

bool Encoding_IsUTF8(const cpi_enc_t iEncoding) {
  return  (iEncoding >= 0) ? ((g_Encodings[iEncoding].uFlags & NCP_UTF8) != 0) : (iEncoding == CPI_ASCII_7BIT);
}
// ============================================================================

bool Encoding_IsUTF8_SIGN(const cpi_enc_t iEncoding) {
  return  (iEncoding >= 0) ? ((g_Encodings[iEncoding].uFlags & NCP_UTF8_SIGN) != 0) : false;
}
// ============================================================================

bool Encoding_IsUTF8_NO_SIGN(const cpi_enc_t iEncoding) {
  return  (Encoding_IsUTF8(iEncoding) && !Encoding_IsUTF8_SIGN(iEncoding));
}
// ============================================================================

bool Encoding_IsMBCS(const cpi_enc_t iEncoding) {
  return  (iEncoding >= 0) ? ((g_Encodings[iEncoding].uFlags & NCP_MBCS) != 0) : false;
}
// ============================================================================

bool Encoding_IsCJK(const cpi_enc_t iEncoding) {
  UINT const codePage = Encoding_GetCodePage(iEncoding);
  switch (codePage) 
  {
  case 932:
  case 936:
  case 949:
  case 950:
  case 951:
  case 1361:
  case 10001:
  case 10002:
  case 10003:
  case 10008:
  case 20000:
  case 20932:
  case 20936:
  case 50220:
  case 50225:
  case 51949:
  case 52936:
  case 54936:
    return true;
  default:
    break;
  }
  return false;
}
// ============================================================================

bool Encoding_IsUNICODE(const cpi_enc_t iEncoding) {
  return  (iEncoding >= 0) ? ((g_Encodings[iEncoding].uFlags & NCP_UNICODE) != 0) : false;
}
// ============================================================================

bool Encoding_IsUNICODE_BOM(const cpi_enc_t iEncoding) {
  return  (iEncoding >= 0) ? ((g_Encodings[iEncoding].uFlags & NCP_UNICODE_BOM) != 0) : false;
}
// ============================================================================

bool Encoding_IsUNICODE_REVERSE(const cpi_enc_t iEncoding) {
  return  (iEncoding >= 0) ? ((g_Encodings[iEncoding].uFlags & NCP_UNICODE_REVERSE) != 0) : false;
}
// ============================================================================


bool Encoding_IsINTERNAL(const cpi_enc_t iEncoding) {
  return  (iEncoding >= 0) ? ((g_Encodings[iEncoding].uFlags & NCP_INTERNAL) != 0) : false;
}
// ============================================================================

bool Encoding_IsEXTERNAL_8BIT(const cpi_enc_t iEncoding) {
  return  (iEncoding >= 0) ? ((g_Encodings[iEncoding].uFlags & NCP_EXTERNAL_8BIT) != 0) : false;
}
// ============================================================================

bool Encoding_IsRECODE(const cpi_enc_t iEncoding) {
  return  (iEncoding >= 0) ? ((g_Encodings[iEncoding].uFlags & NCP_RECODE) != 0) : false;
}
// ============================================================================


void Encoding_SetDefaultFlag(const cpi_enc_t iEncoding) {
  if (iEncoding >= 0) {
    g_Encodings[iEncoding].uFlags |= NCP_DEFAULT;
  }
}
// ============================================================================


const WCHAR* Encoding_GetLabel(const cpi_enc_t iEncoding) {
  return (iEncoding >= 0) ? g_Encodings[iEncoding].wchLabel : NULL;
}
// ============================================================================


const char* Encoding_GetParseNames(const cpi_enc_t iEncoding) {
  return (iEncoding >= 0) ? g_Encodings[iEncoding].pszParseNames : NULL;
}
// ============================================================================


int Encoding_GetNameA(const cpi_enc_t iEncoding, char* buffer, size_t cch) {
  if (iEncoding >= 0) {
    const char* p = Encoding_GetParseNames(iEncoding);
    if (p && *p) {
      ++p;
      const char* q = StrChrA(p, ',');
      if (q && *q) {
        StringCchCopyNA(buffer, cch, p, (q - p));
        return (int)min_s((q - p), cch);
      }
    }
  }
  return 0;
}
// ============================================================================


int Encoding_GetNameW(const cpi_enc_t iEncoding, LPWSTR buffer, size_t cwch)
{
  char tmpbuffer[256] = { '\0' };
  Encoding_GetNameA(iEncoding, tmpbuffer, 256);
  return (int)MultiByteToWideCharEx(Encoding_SciCP, 0, tmpbuffer, -1, buffer, cwch);
}
// ============================================================================


bool Has_UTF16_LE_BOM(const char* pBuf, size_t cnt)
{
  int iTest = IS_TEXT_UNICODE_SIGNATURE;
  /*bool const ok =*/ (void)IsTextUnicode(pBuf, clampi((int)cnt, 0, 4), &iTest);
  //~return (ok && ((iTest & IS_TEXT_UNICODE_SIGNATURE) != 0));
  return ((iTest & IS_TEXT_UNICODE_SIGNATURE) != 0); // don't rely on result ok
}
// ----------------------------------------------------------------------------

bool Has_UTF16_BE_BOM(const char* pBuf, size_t cnt)
{
  int iTest = IS_TEXT_UNICODE_REVERSE_SIGNATURE;
  /*bool const ok =*/ (void)IsTextUnicode(pBuf, clampi((int)cnt, 0, 4), &iTest);
  //~return (ok && ((iTest & IS_TEXT_UNICODE_REVERSE_SIGNATURE) != 0));
  return ((iTest & IS_TEXT_UNICODE_REVERSE_SIGNATURE) != 0); // don't rely on result ok
}
// ============================================================================


bool IsValidUTF7(const char* pTest, size_t nLength) 
{
  if (!pTest) { return false; }

  char const *pt = pTest;
  for (size_t i = 0; i < nLength; ++i) {
    if ((*pt & 0x80) || !*pt) { return false; }
    ++pt;
  }
  return true;
}
// ============================================================================


#undef _OLD_UTF8_VALIDATOR_
//#define _OLD_UTF8_VALIDATOR_ 1
#ifdef _OLD_UTF8_VALIDATOR_

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
size_t  UTF8_mbslen_bytes(LPCSTR utf8_string)
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
size_t  UTF8_mbslen(LPCSTR utf8_string, size_t byte_length)
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

bool  UTF8_ContainsInvalidChars(LPCSTR utf8_string, size_t byte_length)
{
  return ((UTF8_mbslen_bytes(UTF8StringStart(utf8_string)) - 1) !=
    UTF8_mbslen(UTF8StringStart(utf8_string), IsUTF8Signature(utf8_string) ? (byte_length - 3) : byte_length));
}

// ----------------------------------------------------------------------------


bool IsValidUTF8(const char* pTest, size_t nLength)
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

  return (current == kSTART) && !UTF8_ContainsInvalidChars(pTest, nLength);
}


// ============================================================================
#else  // new UTF-8 validator
// ============================================================================


// Copyright (c) 2008-2010 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

bool IsValidUTF8(const char* pTest, size_t nLength)
{
  enum {
    UTF8_ACCEPT = 0,
    UTF8_REJECT = 12
  };

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

  UINT state = UTF8_ACCEPT;
  while (pt < end && *pt) {
    state = utf8_dfa[256 + state + utf8_dfa[*pt++]];
    if (state == UTF8_REJECT) {
      return false;
    }
  }
  return (state == UTF8_ACCEPT);
}

// ----------------------------------------------------------------------------

#endif

// ============================================================================




