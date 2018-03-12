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

#include <windows.h>
#include <commctrl.h>
#include <stdlib.h>

#include "../uthash/utarray.h"

#include "scintilla.h"
#include "helpers.h"
#include "encoding.h"


extern HINSTANCE g_hInstance;
extern BOOL bLoadASCIIasUTF8;

//=============================================================================

typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

const uint32_t limitDblHiByte4ANSI = 5; // [%] of max. double HighByte in file to be assumed as ANSI

#include "EncTables.hpp"

//=============================================================================
//
//  Encoding Helper Functions
//

int g_DOSEncoding;

// Supported Encodings
WCHAR wchANSI[16] = { L'\0' };
WCHAR wchOEM[16] = { L'\0' };


int Encoding_CountOf() {
  return COUNTOF(g_Encodings);
}
// ============================================================================


int Encoding_Current(int iEncoding) {
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


int Encoding_SrcCmdLn(int iSrcEncoding) {
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


int  Encoding_SrcWeak(int iSrcWeakEnc) {
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


BOOL Encoding_HasChanged(int iOriginalEncoding) {
  static int OriginalEncoding = CPI_NONE;

  if (iOriginalEncoding >= CPI_NONE) {
    OriginalEncoding = iOriginalEncoding;
  }
  return (BOOL)(OriginalEncoding != Encoding_Current(CPI_GET));
}
// ============================================================================


// ============================================================================
// ============================================================================

/*
* Mostly taken from "tellenc"
* Program to detect the encoding of text.  It currently supports ASCII,
* UTF-8, UTF-16/32 (little-endian or big-endian), Latin1, Windows-1252,
* CP437, GB2312, GBK, Big5, and SJIS, among others.
*
* Copyright (C) 2006-2016 Wu Yongwei <wuyongwei@gmail.com>
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any
* damages arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute
* it freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must
*    not claim that you wrote the original software.  If you use this
*    software in a product, an acknowledgment in the product
*    documentation would be appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must
*    not be misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source
*    distribution.
*
*
* The latest version of this software should be available at:
*      <URL:https://github.com/adah1972/tellenc>
*
*/

typedef enum _UTF8_ValidationState
{
  UTF8_INVALID,
  UTF8_1,
  UTF8_2,
  UTF8_3,
  UTF8_4,
  UTF8_TAIL
} UTF8_ValidationState;

#define MAX_CHAR 256

typedef struct _dbyte_cnt_t {
  uint16_t dblByte;
  uint32_t count;
} dbyte_cnt_t;

int __fastcall check_freq_dbyte(uint16_t dbyte)
{
  for (size_t i = 0; i < (sizeof freq_analysis_data / sizeof(freq_analysis_data_t)); ++i) {
    if (dbyte == freq_analysis_data[i].dbyte) {
      return freq_analysis_data[i].encID;
    }
  }
  return CPI_NONE;
}
// ============================================================================

// --------------------------------------------------------------
// arg dbyte_cnt_map must be sorted (high count first)
//
int __fastcall search_freq_dbytes(const UT_array* dbyte_cnt_map)
{
  size_t max_comp_cnt = 10;
  size_t cnt = 0;

  for (dbyte_cnt_t* p = (dbyte_cnt_t*)utarray_front(dbyte_cnt_map);
       (p != NULL) && (++cnt <= max_comp_cnt);
       p = (dbyte_cnt_t*)utarray_next(dbyte_cnt_map, p)) {

    const int enc = check_freq_dbyte(p->dblByte);

    if (enc > CPI_NONE) {
      return enc;
    }
  }
  return CPI_NONE;
}
// ============================================================================


static UTF8_ValidationState utf8_char_table[MAX_CHAR];

void init_utf8_validation_char_table()
{
  int ch = 0;
  utf8_char_table[ch] = UTF8_INVALID;
  ++ch;
  for (; ch <= 0x7f; ++ch) {
    utf8_char_table[ch] = UTF8_1;
  }
  for (; ch <= 0xbf; ++ch) {
    utf8_char_table[ch] = UTF8_TAIL;
  }
  for (; ch <= 0xc1; ++ch) {
    utf8_char_table[ch] = UTF8_INVALID;
  }
  for (; ch <= 0xdf; ++ch) {
    utf8_char_table[ch] = UTF8_2;
  }
  for (; ch <= 0xef; ++ch) {
    utf8_char_table[ch] = UTF8_3;
  }
  for (; ch <= 0xf4; ++ch) {
    utf8_char_table[ch] = UTF8_4;
  }
  for (; ch <= 0xff; ++ch) {
    utf8_char_table[ch] = UTF8_INVALID;
  }
}
// ============================================================================


void __fastcall init_sbyte_char_count(dbyte_cnt_t sbyte_char_cnt[])
{
  for (size_t ch = 0; ch < MAX_CHAR; ++ch) {
    sbyte_char_cnt[ch].dblByte = (uint16_t)ch;
    sbyte_char_cnt[ch].count = 0;
  }
}
// ============================================================================

static const unsigned char NON_TEXT_CHARS[] = { 0, 26, 127, 255 };

__forceinline bool is_non_text(char ch)
{
  for (size_t i = 0; i < sizeof(NON_TEXT_CHARS); ++i) {
    if (ch == NON_TEXT_CHARS[i]) {
      return true;
    }
  }
  return false;
}
// ============================================================================


__forceinline dbyte_cnt_t* find_dbyte_count(const UT_array* const dbyte_cnt_map, const uint16_t dbyte)
{
  for (dbyte_cnt_t* p = (dbyte_cnt_t*)utarray_front(dbyte_cnt_map);
    (p != NULL);
       p = (dbyte_cnt_t*)utarray_next(dbyte_cnt_map, p)) {

    if (p->dblByte == dbyte)
      return p;
  }
  return NULL;
}
// ============================================================================


static int ascending_count(const void *lhs, const void *rhs)
{
  const uint32_t lcnt = ((dbyte_cnt_t*)lhs)->count;
  const uint32_t rcnt = ((dbyte_cnt_t*)rhs)->count;
  return (lcnt - rcnt); // ascending order
}

static int descending_count(const void *lhs, const void *rhs)
{
  const uint32_t lcnt = ((dbyte_cnt_t*)lhs)->count;
  const uint32_t rcnt = ((dbyte_cnt_t*)rhs)->count;
  return (rcnt - lcnt); // descending order
}


int __fastcall check_ucs_bom(const char* const buffer, const size_t len)
{
  const struct bom_t
  {
    const int encoding;
    const char* bom;
    size_t bom_len;
  } boms[] = 
  {
    { CPI_UCS4BE,       "\x00\x00\xFE\xFF",  4 },
    { CPI_UCS4,         "\xFF\xFE\x00\x00",  4 },
    { CPI_UTF8SIGN,     "\xEF\xBB\xBF",      3 },
    { CPI_UNICODEBEBOM, "\xFE\xFF",          2 },
    { CPI_UNICODEBOM,   "\xFF\xFE",          2 },
    { -1  ,             NULL,                0 }
  };
  for (size_t i = 0; (boms[i].encoding >= 0); ++i) {
    if ((len >= boms[i].bom_len) && (memcmp(buffer, boms[i].bom, boms[i].bom_len) == 0)) {
      return boms[i].encoding;
    }
  }
  return CPI_NONE;
}


// ============================================================================

//typedef pair<uint16_t, uint32_t>  char_count_t;
//typedef map<uint16_t, uint32_t>   char_count_map_t;
//typedef vector<char_count_t>      char_count_vec_t;

static const char NUL = '\0';
static const char DOS_EOF = '\x1A';
static const int EVEN = 0;
static const int ODD = 1;

static size_t nul_count_byte[2];
static size_t nul_count_word[2];


int Encoding_Analyze(const char* const buffer, const size_t len)
{
  bool is_binary = false;
  bool is_valid_utf8 = true;
  bool is_valid_latin1 = true;
  uint32_t dbyte_cnt = 0;
  uint32_t dbyte_hihi_cnt = 0;

  UT_icd dbyte_count_icd = { sizeof(dbyte_cnt_t), NULL, NULL, NULL };
  UT_array* dbyte_count_map = NULL;

  int iEncoding = check_ucs_bom(buffer, len);

  if (iEncoding != CPI_NONE)
    return iEncoding;

  utarray_new(dbyte_count_map, &dbyte_count_icd);
  utarray_reserve(dbyte_count_map, MAX_CHAR);

  //~dbyte_cnt_t sbyte_char_count[MAX_CHAR];
  //~init_sbyte_char_count(sbyte_char_count);

  int last_ch = EOF;
  UTF8_ValidationState utf8_valid_state = UTF8_1;

  for (size_t pos = 0; pos < len; ++pos) {

    const unsigned char ch = buffer[pos];
    //~ ++(sbyte_char_count[ch].count);

    // Check for binary data (including UTF-16/32)
    if (is_non_text(ch)) {
      if (!is_binary && !(ch == DOS_EOF && pos == len - 1)) {
        is_binary = true;
      }
      if (ch == NUL) {
        // Count for NULs in even- and odd-number bytes
        nul_count_byte[pos & 1]++;
        if (pos & 1) {
          if (buffer[pos - 1] == NUL) {
            // Count for NULs in even- and odd-number words
            nul_count_word[(pos / 2) & 1]++;
          }
        }
      }
    }

    // Check for UTF-8 validity
    if (is_valid_utf8) {
      switch (utf8_char_table[ch]) {
      case UTF8_INVALID:
        is_valid_utf8 = false;
        break;
      case UTF8_1:
        if (utf8_valid_state != UTF8_1) {
          is_valid_utf8 = false;
        }
        break;
      case UTF8_2:
        if (utf8_valid_state != UTF8_1) {
          is_valid_utf8 = false;
        }
        else {
          utf8_valid_state = UTF8_2;
        }
        break;
      case UTF8_3:
        if (utf8_valid_state != UTF8_1) {
          is_valid_utf8 = false;
        }
        else {
          utf8_valid_state = UTF8_3;
        }
        break;
      case UTF8_4:
        if (utf8_valid_state != UTF8_1) {
          is_valid_utf8 = false;
        }
        else {
          utf8_valid_state = UTF8_4;
        }
        break;
      case UTF8_TAIL:
        if (utf8_valid_state > UTF8_1) {
          utf8_valid_state--;
        }
        else {
          is_valid_utf8 = false;
        }
        break;
      }
    }

    // Check whether non-Latin1 characters appear
    if (is_valid_latin1) {
      if (ch >= 0x80 && ch < 0xa0) {
        is_valid_latin1 = false;
      }
    }
    
    // Construct double-bytes and count
    if (last_ch != EOF)
    {
      dbyte_cnt_t dbyte_item = { 0, 1 };
      dbyte_item.dblByte = (uint16_t)((last_ch << 8) + ch);

      dbyte_cnt_t* item = find_dbyte_count(dbyte_count_map, dbyte_item.dblByte);
      if (item == NULL)
        utarray_push_back(dbyte_count_map, &dbyte_item);
      else
        ++(item->count);

      ++dbyte_cnt;
      if ((last_ch > 0xa0) && (ch > 0xa0)) {
        ++dbyte_hihi_cnt;
      }
      //last_ch = EOF;
    }

    if (ch >= 0x80)
      last_ch = ch;
    else
      last_ch = EOF;

  } // for

  if (!is_valid_utf8 && is_binary) {
    // Heuristics for UTF-16/32
    if (nul_count_byte[EVEN] > 4 &&
      (nul_count_byte[ODD] == 0 ||
       nul_count_byte[EVEN] / nul_count_byte[ODD] > 20)) {
      iEncoding = CPI_UNICODEBE;
    }
    else if (nul_count_byte[ODD] > 4 &&
      (nul_count_byte[EVEN] == 0 ||
       nul_count_byte[ODD] / nul_count_byte[EVEN] > 20)) {
      iEncoding = CPI_UNICODE;
    }
    else if (nul_count_word[EVEN] > 4 &&
      (nul_count_word[ODD] == 0 ||
       nul_count_word[EVEN] / nul_count_word[ODD] > 20)) {
      iEncoding = CPI_UCS4BE;   // utf-32 is not a built-in encoding for Notepad3
    }
    else if (nul_count_word[ODD] > 4 &&
      (nul_count_word[EVEN] == 0 ||
       nul_count_word[ODD] / nul_count_word[EVEN] > 20)) {
      iEncoding = CPI_UCS4; // utf-32le is not a built-in encoding for Notepad3
    }
  }
  else if (dbyte_cnt == 0) {
    // No characters outside the scope of ASCII
    iEncoding = bLoadASCIIasUTF8 ? CPI_UTF8 : CPI_ANSI_DEFAULT;
  }
  else if (is_valid_utf8) {
    // Only valid UTF-8 sequences
    iEncoding = CPI_UTF8;
  }

  if (iEncoding == CPI_NONE)  // still unknown ?
  {
    // Get the character counts in descending order
    //~qsort((void*)sbyte_char_count, MAX_CHAR, sizeof(dbyte_cnt_t), descending_count);

    // Get the double-byte counts in descending order
    utarray_sort(dbyte_count_map, descending_count);

    const int probEncoding = search_freq_dbytes(dbyte_count_map);

    if (probEncoding != CPI_NONE) {
      iEncoding = probEncoding;
    }
    else if (((dbyte_hihi_cnt * 100) / ++dbyte_cnt) < limitDblHiByte4ANSI) {
      // mostly a low-byte follows a high-byte
      iEncoding = bLoadASCIIasUTF8 ? CPI_UTF8 : CPI_ANSI_DEFAULT;
    }
  }

  utarray_clear(dbyte_count_map);
  utarray_free(dbyte_count_map);

  return iEncoding;
}
// ============================================================================


// ============================================================================
// ============================================================================
//
// END  OF  "TELLENC"  PART
//
// ============================================================================
// ============================================================================


void Encoding_InitDefaults()

{
  // init tellenc code page detection
  init_utf8_validation_char_table();

  const UINT uCodePageMBCS[20] = {
    42, // (Symbol)
    50220,50221,50222,50225,50227,50229, // (Chinese, Japanese, Korean) 
    54936, // (GB18030)
    57002,57003,57004,57005,57006,57007,57008,57009,57010,57011, // (ISCII)
    65000, // (UTF-7)
    65001  // (UTF-8)
  };

  g_Encodings[CPI_ANSI_DEFAULT].uCodePage = GetACP(); // set ANSI system CP
  StringCchPrintf(wchANSI, COUNTOF(wchANSI), L" (CP-%u)", g_Encodings[CPI_ANSI_DEFAULT].uCodePage);

  for (int i = CPI_UTF7 + 1; i < COUNTOF(g_Encodings); ++i) {
    if (Encoding_IsValid(i) && (g_Encodings[i].uCodePage == g_Encodings[CPI_ANSI_DEFAULT].uCodePage)) {
      g_Encodings[i].uFlags |= NCP_ANSI;
      if (g_Encodings[i].uFlags & NCP_EXTERNAL_8BIT)
        g_Encodings[CPI_ANSI_DEFAULT].uFlags |= NCP_EXTERNAL_8BIT;
      break;
    }
  }

  g_Encodings[CPI_OEM].uCodePage = GetOEMCP();
  StringCchPrintf(wchOEM, COUNTOF(wchOEM), L" (CP-%u)", g_Encodings[CPI_OEM].uCodePage);

  for (int i = CPI_UTF7 + 1; i < COUNTOF(g_Encodings); ++i) {
    if (Encoding_IsValid(i) && (g_Encodings[i].uCodePage == g_Encodings[CPI_OEM].uCodePage)) {
      g_Encodings[i].uFlags |= NCP_OEM;
      if (g_Encodings[i].uFlags & NCP_EXTERNAL_8BIT)
        g_Encodings[CPI_OEM].uFlags |= NCP_EXTERNAL_8BIT;
      break;
    }
  }

  // multi byte character sets
  for (int i = 0; i < COUNTOF(g_Encodings); ++i) {
    for (int k = 0; k < COUNTOF(uCodePageMBCS); k++) {
      if (g_Encodings[i].uCodePage == uCodePageMBCS[k]) {
        g_Encodings[i].uFlags |= NCP_MBCS;
      }
    }
  }

  g_DOSEncoding = CPI_OEM;
  // Try to set the DOS encoding to DOS-437 if the default OEMCP is not DOS-437
  if (g_Encodings[g_DOSEncoding].uCodePage != 437) {
    for (int i = CPI_UTF7 + 1; i < COUNTOF(g_Encodings); ++i) {
      if (Encoding_IsValid(i) && (g_Encodings[i].uCodePage == 437)) {
        g_DOSEncoding = i;
        break;
      }
    }
  }

}
// ============================================================================


int Encoding_MapIniSetting(BOOL bLoad, int iSetting) {
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
      for (int i = CPI_UTF7 + 1; i < COUNTOF(g_Encodings); i++) {
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
    GetString(g_Encodings[iEncoding].idsName, wch1, COUNTOF(wch1));
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
  for (int i = 0; i < COUNTOF(g_Encodings); i++) {
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
  for (int i = 0; i < COUNTOF(g_Encodings); i++) {
    if (cp == g_Encodings[i].uCodePage) {
      return i;
    }
  }
  return CPI_ANSI_DEFAULT;
}
// ============================================================================


BOOL Encoding_IsValid(int iTestEncoding) {
  CPINFO cpi;
  if ((iTestEncoding >= 0) && (iTestEncoding < COUNTOF(g_Encodings))) {
    if ((g_Encodings[iTestEncoding].uFlags & NCP_INTERNAL) ||
      IsValidCodePage(g_Encodings[iTestEncoding].uCodePage) &&
      GetCPInfo(g_Encodings[iTestEncoding].uCodePage, &cpi)) {
      return(TRUE);
    }
  }
  return(FALSE);
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


void Encoding_AddToListView(HWND hwnd, int idSel, BOOL bRecodeOnly) {
  int i;
  int iSelItem = -1;
  LVITEM lvi;
  WCHAR wchBuf[256] = { L'\0' };

  PENCODINGENTRY pEE = LocalAlloc(LPTR, COUNTOF(g_Encodings) * sizeof(ENCODINGENTRY));
  for (i = 0; i < COUNTOF(g_Encodings); i++) {
    pEE[i].id = i;
    GetString(g_Encodings[i].idsName, pEE[i].wch, COUNTOF(pEE[i].wch));
  }
  qsort(pEE, COUNTOF(g_Encodings), sizeof(ENCODINGENTRY), CmpEncoding);

  ZeroMemory(&lvi, sizeof(LVITEM));
  lvi.mask = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE;
  lvi.pszText = wchBuf;

  for (i = 0; i < COUNTOF(g_Encodings); i++) {

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
    ListView_EnsureVisible(hwnd, iSelItem, FALSE);
  }
  else {
    ListView_SetItemState(hwnd, 0, LVIS_FOCUSED, LVIS_FOCUSED);
    ListView_EnsureVisible(hwnd, 0, FALSE);
  }
}
// ============================================================================


BOOL Encoding_GetFromListView(HWND hwnd, int *pidEncoding) {
  LVITEM lvi;

  lvi.iItem = ListView_GetNextItem(hwnd, -1, LVNI_ALL | LVNI_SELECTED);
  lvi.iSubItem = 0;
  lvi.mask = LVIF_PARAM;

  if (ListView_GetItem(hwnd, &lvi)) {
    if (Encoding_IsValid((int)lvi.lParam))
      *pidEncoding = (int)lvi.lParam;
    else
      *pidEncoding = -1;

    return (TRUE);
  }
  return(FALSE);
}
// ============================================================================


void Encoding_AddToComboboxEx(HWND hwnd, int idSel, BOOL bRecodeOnly) {
  int i;
  int iSelItem = -1;
  COMBOBOXEXITEM cbei;
  WCHAR wchBuf[256] = { L'\0' };

  PENCODINGENTRY pEE = LocalAlloc(LPTR, COUNTOF(g_Encodings) * sizeof(ENCODINGENTRY));
  for (i = 0; i < COUNTOF(g_Encodings); i++) {
    pEE[i].id = i;
    GetString(g_Encodings[i].idsName, pEE[i].wch, COUNTOF(pEE[i].wch));
  }
  qsort(pEE, COUNTOF(g_Encodings), sizeof(ENCODINGENTRY), CmpEncoding);

  ZeroMemory(&cbei, sizeof(COMBOBOXEXITEM));
  cbei.mask = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_LPARAM;
  cbei.pszText = wchBuf;
  cbei.cchTextMax = COUNTOF(wchBuf);
  cbei.iImage = 0;
  cbei.iSelectedImage = 0;

  for (i = 0; i < COUNTOF(g_Encodings); i++) {

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


BOOL Encoding_GetFromComboboxEx(HWND hwnd, int *pidEncoding) {
  COMBOBOXEXITEM cbei;

  cbei.iItem = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
  cbei.mask = CBEIF_LPARAM;

  if (SendMessage(hwnd, CBEM_GETITEM, 0, (LPARAM)&cbei)) {
    if (Encoding_IsValid((int)cbei.lParam))
      *pidEncoding = (int)cbei.lParam;
    else
      *pidEncoding = -1;

    return (TRUE);
  }
  return(FALSE);
}
// ============================================================================


UINT Encoding_GetCodePage(int iEncoding) {
  return (iEncoding >= 0) ? g_Encodings[iEncoding].uCodePage : CP_ACP;
}
// ============================================================================

BOOL Encoding_IsDefault(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_DEFAULT) : FALSE;
}
// ============================================================================

BOOL Encoding_IsANSI(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_ANSI) : FALSE;
}
// ============================================================================

BOOL Encoding_IsOEM(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_OEM) : FALSE;
}
// ============================================================================

BOOL Encoding_IsUTF8(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_UTF8) : FALSE;
}
// ============================================================================

BOOL Encoding_IsUTF8_SIGN(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_UTF8_SIGN) : FALSE;
}
// ============================================================================

BOOL Encoding_IsMBCS(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_MBCS) : FALSE;
}
// ============================================================================

BOOL Encoding_IsUNICODE(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_UNICODE) : FALSE;
}
// ============================================================================

BOOL Encoding_IsUNICODE_BOM(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_UNICODE_BOM) : FALSE;
}
// ============================================================================

BOOL Encoding_IsUNICODE_REVERSE(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_UNICODE_REVERSE) : FALSE;
}
// ============================================================================


BOOL Encoding_IsINTERNAL(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_INTERNAL) : FALSE;
}
// ============================================================================

BOOL Encoding_IsEXTERNAL_8BIT(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_EXTERNAL_8BIT) : FALSE;
}
// ============================================================================

BOOL Encoding_IsRECODE(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_RECODE) : FALSE;
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




UINT Encoding_SciGetCodePage(HWND hwnd) {
  UNUSED(hwnd);
  return CP_UTF8;
  // remove internal support for Chinese, Japan, Korean DBCS  use UTF-8 instead
  /*
  int cp = (UINT)SendMessage(hwnd,SCI_GETCODEPAGE,0,0);
  if (cp == 932 || cp == 936 || cp == 949 || cp == 950) {
  return cp;
  }
  return (cp == 0) ? CP_ACP : CP_UTF8;
  */
}
// ============================================================================


int Encoding_SciMappedCodePage(int iEncoding) {
  UNUSED(iEncoding);
  return SC_CP_UTF8;
  // remove internal support for Chinese, Japan, Korean DBCS  use UTF-8 instead
  /*
  if (Encoding_IsValid(iEncoding)) {
  // check for Chinese, Japan, Korean DBCS code pages and switch accordingly
  int cp = (int)g_Encodings[iEncoding].uCodePage;
  if (cp == 932 || cp == 936 || cp == 949 || cp == 950) {
  return cp;
  }
  }
  */
}
// ============================================================================


void Encoding_SciSetCodePage(HWND hwnd, int iEncoding) {
  int cp = Encoding_SciMappedCodePage(iEncoding);
  SendMessage(hwnd, SCI_SETCODEPAGE, (WPARAM)cp, 0);
  // charsets can be changed via styles schema
  /*
  int charset = SC_CHARSET_ANSI;
  switch (cp) {
  case 932:
  charset = SC_CHARSET_SHIFTJIS;
  break;
  case 936:
  charset = SC_CHARSET_GB2312;
  break;
  case 949:
  charset = SC_CHARSET_HANGUL;
  break;
  case 950:
  charset = SC_CHARSET_CHINESEBIG5;
  break;
  default:
  charset = g_iDefaultCharSet;
  break;
  }
  SendMessage(hwnd,SCI_STYLESETCHARACTERSET,(WPARAM)STYLE_DEFAULT,(LPARAM)charset);
  */
}
// ============================================================================


BOOL IsUnicode(const char* pBuffer, int cb, LPBOOL lpbBOM, LPBOOL lpbReverse) {
  int i = 0xFFFF;

  BOOL bIsTextUnicode;

  BOOL bHasBOM;
  BOOL bHasRBOM;

  if (!pBuffer || cb < 2)
    return FALSE;

  bIsTextUnicode = IsTextUnicode(pBuffer, cb, &i);

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
      ? TRUE : FALSE;

    if (lpbReverse)
      *lpbReverse = (bHasRBOM || (i & IS_TEXT_UNICODE_REVERSE_MASK)) ? TRUE : FALSE;

    return TRUE;
  }

  else

    return FALSE;
}
// ============================================================================


BOOL IsUTF8(const char* pTest, int nLength)
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
  int i;

  const char* pt = pTest;
  int len = nLength;

  for (i = 0; i < len; i++, pt++) {

    current = NEXT_STATE(*pt, current);
    if (kERROR == current)
      break;
  }

  return (current == kSTART) ? true : false;
}
// ============================================================================



BOOL IsUTF7(const char* pTest, int nLength) {
  int i;
  const char *pt = pTest;

  for (i = 0; i < nLength; i++) {
    if (*pt & 0x80 || !*pt)
      return FALSE;
    pt++;
  }

  return TRUE;
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
// ============================================================================


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
// ============================================================================


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
// ============================================================================



bool UTF8_ContainsInvalidChars(LPCSTR utf8_string, size_t byte_length)
{
  return ((UTF8_mbslen_bytes(UTF8StringStart(utf8_string)) - 1) != 
           UTF8_mbslen(UTF8StringStart(utf8_string), IsUTF8Signature(utf8_string) ? (byte_length - 3) : byte_length));
}
// ============================================================================

