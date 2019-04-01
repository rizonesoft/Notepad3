/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Encoding.h                                                                  *
*   General helper functions                                                  *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*	Parts taken from SciTE, (c) Neil Hodgson                                  *
*	MinimizeToTray, (c) 2000 Matthew Ellis                                    *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2019   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

#pragma once
#ifndef _NP3_ENCODING_H_
#define _NP3_ENCODING_H_

#include "TypeDefs.h"

#define NCP_DEFAULT            1
#define NCP_UTF8               2
#define NCP_UTF8_SIGN          4
#define NCP_UNICODE            8
#define NCP_UNICODE_REVERSE   16
#define NCP_UNICODE_BOM       32
#define NCP_ANSI              64
#define NCP_OEM              128
#define NCP_MBCS             256
#define NCP_ASCII_7BIT       512
#define NCP_INTERNAL         (NCP_DEFAULT|NCP_UTF8|NCP_UTF8_SIGN|NCP_UNICODE|NCP_UNICODE_REVERSE|NCP_UNICODE_BOM|NCP_ANSI|NCP_OEM|NCP_MBCS|NCP_ASCII_7BIT)
#define NCP_EXTERNAL_8BIT   1024
#define NCP_RECODE          2048

#define CPI_ASCII_7BIT       (-3)
#define CPI_GET              (-2)
#define CPI_NONE             (-1)
// following IDs must match with : NP2ENCODING g_Encodings[]
#define CPI_ANSI_DEFAULT       0
#define CPI_OEM                1
#define CPI_UNICODEBOM         2
#define CPI_UNICODEBEBOM       3
#define CPI_UNICODE            4
#define CPI_UNICODEBE          5
#define CPI_UTF8               6
#define CPI_UTF8SIGN           7
#define CPI_UTF7               8

#define CPI_UTF32       CPI_NONE // invalid
#define CPI_UTF32BE     CPI_NONE // invalid
#define CPI_UCS4       CPI_UTF32 // invalid
#define CPI_UCS4BE   CPI_UTF32BE // invalid

#define Encoding_IsNONE(enc) ((enc) == CPI_NONE)

typedef struct _np2encoding {

  UINT        uFlags;
  UINT        uCodePage;
  const char* pszParseNames;
  int         idsName;
  WCHAR       wchLabel[64];

} NP2ENCODING;

cpi_enc_t  Encoding_Current(cpi_enc_t iEncoding);            // getter/setter
cpi_enc_t  Encoding_SrcCmdLn(cpi_enc_t iSrcEncoding);        // getter/setter
cpi_enc_t  Encoding_SrcWeak(cpi_enc_t iSrcWeakEnc);          // getter/setter
bool       Encoding_HasChanged(cpi_enc_t iOriginalEncoding); // query/setter
           
void       Encoding_InitDefaults();
int        Encoding_MapIniSetting(bool, int iSetting);

cpi_enc_t  Encoding_MapUnicode(cpi_enc_t iUni);
void       Encoding_SetLabel(cpi_enc_t iEncoding);
cpi_enc_t  Encoding_MatchW(LPCWSTR pwszTest);
cpi_enc_t  Encoding_MatchA(const char* pchTest);
bool       Encoding_IsValid(cpi_enc_t iTestEncoding);
cpi_enc_t  Encoding_GetByCodePage(const UINT codepage);
void       Encoding_AddToListView(HWND hwnd, cpi_enc_t idSel, bool);
bool       Encoding_GetFromListView(HWND hwnd, cpi_enc_t* pidEncoding);
void       Encoding_AddToComboboxEx(HWND hwnd, cpi_enc_t idSel, bool);
bool       Encoding_GetFromComboboxEx(HWND hwnd, cpi_enc_t* pidEncoding);
           
UINT       Encoding_GetCodePage(const cpi_enc_t iEncoding);
           
bool       Encoding_IsDefault(const cpi_enc_t iEncoding);
bool       Encoding_IsASCII(const cpi_enc_t iEncoding);
bool       Encoding_IsANSI(const cpi_enc_t iEncoding);
bool       Encoding_IsOEM(const cpi_enc_t iEncoding);
bool       Encoding_IsUTF8(const cpi_enc_t iEncoding);
bool       Encoding_IsUTF8_SIGN(const cpi_enc_t iEncoding);
bool       Encoding_IsMBCS(const cpi_enc_t iEncoding);
bool       Encoding_IsCJK(const cpi_enc_t iEncoding);
bool       Encoding_IsUNICODE(const cpi_enc_t iEncoding);
bool       Encoding_IsUNICODE_BOM(const cpi_enc_t iEncoding);
bool       Encoding_IsUNICODE_REVERSE(const cpi_enc_t iEncoding);
bool       Encoding_IsINTERNAL(const cpi_enc_t iEncoding);
bool       Encoding_IsEXTERNAL_8BIT(const cpi_enc_t iEncoding);
bool       Encoding_IsRECODE(const cpi_enc_t iEncoding);

// Scintilla related
#define Encoding_SciCP  CP_UTF8

void Encoding_SetDefaultFlag(const cpi_enc_t iEncoding);
const WCHAR* Encoding_GetLabel(const cpi_enc_t iEncoding);
const char* Encoding_GetParseNames(const cpi_enc_t iEncoding);
void Encoding_Get1stParseName(const cpi_enc_t iEncoding, const char* buffer, size_t cch);

bool Has_UTF16_LE_BOM(const char* pBuf, size_t cnt);
bool Has_UTF16_BE_BOM(const char* pBuf, size_t cnt);

inline bool IsUTF8Signature(const char* p) {
  return ((p[0] == '\xEF') && (p[1] == '\xBB') && (p[2] == '\xBF'));
}
#define UTF8StringStart(p) (IsUTF8Signature(p)) ? ((p)+3) : (p)

bool IsValidUTF7(const char* pTest, size_t nLength);
bool IsValidUTF8(const char* pTest, size_t nLength);
bool IsValidUnicode(const char* pBuffer, const size_t len, bool* lpbBOM, bool* lpbReverse);

//////////////////////////////////////////////////////
// Google's   CED       "Compact Encoding Detection" 
// Mozilla's  UCHARDET  "Universal Charset Detection"
//////////////////////////////////////////////////////

extern NP2ENCODING g_Encodings[];

cpi_enc_t Encoding_CountOf();

void ChangeEncodingCodePage(const cpi_enc_t cpi, UINT newCP);

inline bool Encoding_IsValidIdx(const cpi_enc_t cpi)
{
  return ((cpi >= 0) && (cpi < Encoding_CountOf()));
}

// 932 Shift-JIS, 936 GBK, 949 UHC, 950 Big5, 951 Big5-hkscs, 1361 Johab
inline bool IsDBCSCodePage(UINT cp) {
  return ((cp == 932) || (cp == 936) || (cp == 949) || (cp == 950) || (cp == 951) || (cp == 1361));
}

cpi_enc_t Encoding_AnalyzeText(const char* const text, const size_t len, float* confidence_io, const cpi_enc_t encodingHint);

const char*  Encoding_GetTitleInfoA();
const WCHAR* Encoding_GetTitleInfoW();

// --------------------------------------------------------------------------------------------------------------------------------

#endif //_NP3_ENCODING_H_
