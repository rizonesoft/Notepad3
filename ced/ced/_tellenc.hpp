/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* EncTables.hpp                                                               *
*   Encoding detector tables                                     *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2017   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

#pragma once
#ifndef _NP3_ENCTABLES_H_
#define _NP3_ENCTABLES_H_

/******************************************************************************
*                                                                             *
*                                                                             *
* tellenc                                                                     *
*                                                                             *
*                                                                             *
* Copyright (C) 2006-2016 Wu Yongwei <wuyongwei@gmail.com>                    *
*                                                                             *
* This software is provided 'as-is', without any express or implied           *
* warranty.  In no event will the authors be held liable for any              *
* damages arising from the use of this software.                              *
*                                                                             *
* Permission is granted to anyone to use this software for any purpose,       *
* including commercial applications, and to alter it and redistribute         *
* it freely, subject to the following restrictions:                           *
*                                                                             *
* 1. The origin of this software must not be misrepresented; you must         *
*    not claim that you wrote the original software.  If you use this         *
*    software in a product, an acknowledgement in the product                 *
*    documentation would be appreciated but is not required.                  *
* 2. Altered source versions must be plainly marked as such, and must         *
*    not be misrepresented as being the original software.                    *
* 3. This notice may not be removed or altered from any source                *
*    distribution.                                                            *
*                                                                             *
*                                                                             *
* The latest version of this software should be available at:                 *
*      <URL:https://github.com/adah1972/tellenc>                              *
*                                                                             *
*                                                                             *
*******************************************************************************/


typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

const uint32_t limitDblHiByte4ANSI = 5; // [%] of max. double HighByte in file to be assumed as ANSI

typedef struct {
  int         encID;     // index of encoding related to table "g_Encodings" (above)
  uint16_t    dbyte;     // encoding typically double byte sequence 
  const char* encoding;  // for human readability only
} freq_analysis_data_t;


static freq_analysis_data_t freq_analysis_data[] = 
{
{ 19, 0x9a74, "windows-1250" },         // "št" (Czech)
{ 19, 0xe865, "windows-1250" },         // "če" (Czech)
{ 19, 0xf865, "windows-1250" },         // "ře" (Czech)
{ 19, 0xe167, "windows-1250" },         // "ág" (Hungarian)
{ 19, 0xe96c, "windows-1250" },         // "él" (Hungarian)
{ 19, 0xb36f, "windows-1250" },         // "ło" (Polish)
{ 19, 0xea7a, "windows-1250" },         // "ęz" (Polish)
{ 19, 0xf377, "windows-1250" },         // "ów" (Polish)
{ 19, 0x9d20, "windows-1250" },         // "ť " (Slovak)
{ 19, 0xfa9d, "windows-1250" },         // "úť" (Slovak)
{ 19, 0x9e69, "windows-1250" },         // "ži" (Slovenian)
{ 19, 0xe869, "windows-1250" },         // "či" (Slovenian)
  
{ 30, 0xe820, "windows-1251" },         // "и " (Cyrillic)
{ 30, 0xe3ee, "windows-1251" },         // "го" (Cyrillic)
{ 30, 0xeaee, "windows-1251" },         // "ко" (Cyrillic)
{ 30, 0xf1ea, "windows-1251" },         // "ск" (Cyrillic)
{ 30, 0xf1f2, "windows-1251" },         // "ст" (Cyrillic)

{ 67, 0xe020, "windows-1252" },         // "à " (French)
{ 67, 0xe920, "windows-1252" },         // "é " (French)
{ 67, 0xe963, "windows-1252" },         // "éc" (French)
{ 67, 0xe965, "windows-1252" },         // "ée" (French)
{ 67, 0xe972, "windows-1252" },         // "ér" (French)
{ 67, 0xe4e4, "windows-1252" },         // "ää" (Finnish)
{ 67, 0xe474, "windows-1252" },         // "ät" (German)
{ 67, 0xfc72, "windows-1252" },         // "ür" (German)
{ 67, 0xed6e, "windows-1252" },         // "ín" (Spanish)
{ 67, 0xf36e, "windows-1252" },         // "ón" (Spanish)
{ 52, 0x8220, "cp437" },                // "é " (French)
{ 52, 0x8263, "cp437" },                // "éc" (French)
{ 52, 0x8265, "cp437" },                // "ée" (French)
{ 52, 0x8272, "cp437" },                // "ér" (French)
{ 52, 0x8520, "cp437" },                // "à " (French)
{ 52, 0x8172, "cp437" },                // "ür" (German)
{ 52, 0x8474, "cp437" },                // "ät" (German)
{ 52, 0xc4c4, "cp437" },                // "──"
{ 52, 0xcdcd, "cp437" },                // "══"
{ 52, 0xdbdb, "cp437" },                // "██"
{ 20, 0xa1a1, "gbk" },                  // "　"
{ 20, 0xa1a2, "gbk" },                  // "、"
{ 20, 0xa1a3, "gbk" },                  // "。"
{ 20, 0xa1a4, "gbk" },                  // "·"
{ 20, 0xa1b6, "gbk" },                  // "《"
{ 20, 0xa1b7, "gbk" },                  // "》"
{ 20, 0xa3ac, "gbk" },                  // "，"
{ 20, 0xa3ba, "gbk" },                  // "："
{ 20, 0xb5c4, "gbk" },                  // "的"
{ 20, 0xc1cb, "gbk" },                  // "了"
{ 20, 0xd2bb, "gbk" },                  // "一"
{ 20, 0xcac7, "gbk" },                  // "是"
{ 20, 0xb2bb, "gbk" },                  // "不"
{ 20, 0xb8f6, "gbk" },                  // "个"
{ 20, 0xc8cb, "gbk" },                  // "人"
{ 20, 0xd5e2, "gbk" },                  // "这"
{ 20, 0xd3d0, "gbk" },                  // "有"
{ 20, 0xced2, "gbk" },                  // "我"
{ 20, 0xc4e3, "gbk" },                  // "你"
{ 20, 0xcbfb, "gbk" },                  // "他"
{ 20, 0xcbfd, "gbk" },                  // "她"
{ 20, 0xc9cf, "gbk" },                  // "上"
{ 20, 0xbfb4, "gbk" },                  // "看"
{ 20, 0xd6ae, "gbk" },                  // "之"
{ 20, 0xbbb9, "gbk" },                  // "还"
{ 20, 0xbfc9, "gbk" },                  // "可"
{ 20, 0xbaf3, "gbk" },                  // "后"
{ 20, 0xd6d0, "gbk" },                  // "中"
{ 20, 0xd0d0, "gbk" },                  // "行"
{ 20, 0xb1d2, "gbk" },                  // "币"
{ 20, 0xb3f6, "gbk" },                  // "出"
{ 20, 0xb7d1, "gbk" },                  // "费"
{ 20, 0xb8d0, "gbk" },                  // "感"
{ 20, 0xbef5, "gbk" },                  // "觉"
{ 20, 0xc4ea, "gbk" },                  // "年"
{ 20, 0xd4c2, "gbk" },                  // "月"
{ 20, 0xc8d5, "gbk" },                  // "日"
{ 22, 0xa140, "big5" },                 // "　"
{ 22, 0xa141, "big5" },                 // "，"
{ 22, 0xa143, "big5" },                 // "。"
{ 22, 0xa147, "big5" },                 // "："
{ 22, 0xaaba, "big5" },                 // "的"
{ 22, 0xa446, "big5" },                 // "了"
{ 22, 0xa440, "big5" },                 // "一"
{ 22, 0xac4f, "big5" },                 // "是"
{ 22, 0xa4a3, "big5" },                 // "不"
{ 22, 0xa448, "big5" },                 // "人"
{ 22, 0xa7da, "big5" },                 // "我"
{ 22, 0xa741, "big5" },                 // "你"
{ 22, 0xa54c, "big5" },                 // "他"
{ 22, 0xa66f, "big5" },                 // "她"
{ 22, 0xadd3, "big5" },                 // "個"
{ 22, 0xa457, "big5" },                 // "上"
{ 22, 0xa662, "big5" },                 // "在"
{ 22, 0xbba1, "big5" },                 // "說"
{ 22, 0xa65e, "big5" },                 // "回"
{ 46, 0x8140, "sjis" },                 // "　"
{ 46, 0x8141, "sjis" },                 // "、"
{ 46, 0x8142, "sjis" },                 // "。"
{ 46, 0x8145, "sjis" },                 // "・"
{ 46, 0x8146, "sjis" },                 // "："
{ 46, 0x815b, "sjis" },                 // "ー"
{ 46, 0x82b5, "sjis" },                 // "し"
{ 46, 0x82bd, "sjis" },                 // "た"
{ 46, 0x82c8, "sjis" },                 // "な"
{ 46, 0x82c9, "sjis" },                 // "に"
{ 46, 0x82cc, "sjis" },                 // "の"
{ 46, 0x82dc, "sjis" },                 // "ま"
{ 46, 0x82f0, "sjis" },                 // "を"
{ 46, 0x8367, "sjis" },                 // "ト"
{ 46, 0x8393, "sjis" },                 // "ン"
{ 46, 0x89ef, "sjis" },                 // "会"
{ 46, 0x906c, "sjis" },                 // "人"
{ 46, 0x9094, "sjis" },                 // "数"
{ 46, 0x93fa, "sjis" },                 // "日"
{ 46, 0x95f1, "sjis" },                 // "報"
{ 73, 0xa1bc, "euc-jp" },               // "ー"
{ 73, 0xa4bf, "euc-jp" },               // "た"
{ 73, 0xa4ca, "euc-jp" },               // "な"
{ 73, 0xa4cb, "euc-jp" },               // "に"
{ 73, 0xa4ce, "euc-jp" },               // "の"
{ 73, 0xa4de, "euc-jp" },               // "ま"
{ 73, 0xa4f2, "euc-jp" },               // "を"
{ 73, 0xa5c8, "euc-jp" },               // "ト"
{ 73, 0xa5f3, "euc-jp" },               // "ン"
{ 73, 0xb2f1, "euc-jp" },               // "会"
{ 73, 0xbfcd, "euc-jp" },               // "人"
{ 73, 0xbff4, "euc-jp" },               // "数"
{ 73, 0xc6fc, "euc-jp" },               // "日"
{ 73, 0xcaf3, "euc-jp" },               // "報"
{ 74, 0xc0cc, "euc-kr" },               // "이"
{ 74, 0xb0fa, "euc-kr" },               // "과"
{ 74, 0xb1e2, "euc-kr" },               // "기"
{ 74, 0xb4c2, "euc-kr" },               // "는"
{ 74, 0xb7ce, "euc-kr" },               // "로"
{ 74, 0xb1db, "euc-kr" },               // "글"
{ 74, 0xc5e4, "euc-kr" },               // "토"
{ 74, 0xc1a4, "euc-kr" },               // "정"
{ 27, 0xc920, "koi8-r" },               // "и "
{ 27, 0xc7cf, "koi8-r" },               // "го"
{ 27, 0xcbcf, "koi8-r" },               // "ко"
{ 27, 0xd3cb, "koi8-r" },               // "ск"
{ 27, 0xd3d4, "koi8-r" },               // "ст"
{ 28, 0xa6a7, "koi8-u" },               // "ії"
{ 28, 0xa6ce, "koi8-u" },               // "ін"
{ 28, 0xa6d7, "koi8-u" },               // "ів"
{ 28, 0xa7ce, "koi8-u" },               // "їн"
{ 28, 0xd0cf, "koi8-u" },               // "по"
{ 28, 0xd4c9, "koi8-u" },               // "ти"
};
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

typedef struct _dbyte_cnt_t
{
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
    if (last_ch != EOF) {
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


// init tellenc code page detection
init_utf8_validation_char_table();


// ============================================================================
// ============================================================================
//
// END  OF  "TELLENC"  PART
//
// ============================================================================
// ============================================================================


#endif //_NP3_ENCTABLES_H_
