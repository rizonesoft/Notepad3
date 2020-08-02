// encoding: UTF-8
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
*                                                  (c) Rizonesoft 2008-2020   *
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

#define CPI_ASCII_7BIT       (-7)
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

//#define CPI_PREFERRED_ENCODING  CPI_ANSI_DEFAULT
#define CPI_PREFERRED_ENCODING  CPI_UTF8

typedef struct _np2encoding {

  UINT        uFlags;
  UINT        uCodePage;
  const char* pszParseNames;
  int         idsName;
  WCHAR       wchLabel[64];

} NP2ENCODING;

cpi_enc_t  Encoding_Current(cpi_enc_t iEncoding);         // getter/setter
cpi_enc_t  Encoding_Forced(cpi_enc_t iEncoding);          // getter/setter
cpi_enc_t  Encoding_SrcWeak(cpi_enc_t iSrcWeakEnc);       // getter/setter
inline cpi_enc_t const Encoding_GetCurrent() { return Encoding_Current(CPI_GET); }

void       Encoding_InitDefaults();
int        Encoding_MapIniSetting(bool, int iSetting);

void       Encoding_SetLabel(cpi_enc_t iEncoding);
cpi_enc_t  Encoding_MatchW(LPCWSTR pwszTest);
cpi_enc_t  Encoding_MatchA(const char* pchTest);
bool       Encoding_IsValid(cpi_enc_t iTestEncoding);
cpi_enc_t  Encoding_GetByCodePage(const UINT codepage);
cpi_enc_t  Encoding_MapSignature(cpi_enc_t iUni);
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
bool       Encoding_IsUTF8_NO_SIGN(const cpi_enc_t iEncoding);
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
int Encoding_GetNameA(const cpi_enc_t iEncoding, char* buffer, size_t cch);
int Encoding_GetNameW(const cpi_enc_t iEncoding, LPWSTR buffer, size_t cwch);

bool Has_UTF16_LE_BOM(const char* pBuf, size_t cnt);
bool Has_UTF16_BE_BOM(const char* pBuf, size_t cnt);

inline bool IsUTF8Signature(const char* p) {
  return ((p[0] == '\xEF') && (p[1] == '\xBB') && (p[2] == '\xBF'));
}
#define UTF8StringStart(p) (IsUTF8Signature(p)) ? ((p)+3) : (p)

bool IsValidUTF7(const char* pTest, size_t nLength);
bool IsValidUTF8(const char* pTest, size_t nLength);


//////////////////////////////////////////////////////
// Google's   CED       "Compact Encoding Detection" 
// Mozilla's  UCHARDET  "Universal Charset Detection"
//////////////////////////////////////////////////////

extern NP2ENCODING g_Encodings[];

cpi_enc_t Encoding_CountOf();

void ChangeEncodingCodePage(const cpi_enc_t cpi, UINT newCP);

inline bool Encoding_IsValidIdx(const cpi_enc_t cpi)
{
  return ((cpi > CPI_NONE) && (cpi < Encoding_CountOf()));
}

// 932 Shift-JIS, 936 GBK, 949 UHC, 950 Big5, 951 Big5-hkscs, 1361 Johab
inline bool IsDBCSCodePage(UINT cp) {
  return ((cp == 932) || (cp == 936) || (cp == 949) || (cp == 950) || (cp == 951) || (cp == 1361));
}

// ----------------------------------------------------------------------------

#define FV_TABWIDTH        1
#define FV_INDENTWIDTH     2
#define FV_TABSASSPACES    4
#define FV_TABINDENTS      8
#define FV_WORDWRAP       16
#define FV_LONGLINESLIMIT 32
#define FV_ENCODING       64
#define FV_MODE          128

bool       FileVars_GetFromData(const char* lpData, size_t cbData, LPFILEVARS lpfv);
bool       FileVars_Apply(LPFILEVARS lpfv);
bool       FileVars_ParseInt(char* pszData, char* pszName, int* piValue);
bool       FileVars_ParseStr(char* pszData, char* pszName, char* pszValue, int cchValue);
bool       FileVars_IsUTF8(LPFILEVARS lpfv);
bool       FileVars_IsValidEncoding(LPFILEVARS lpfv);
cpi_enc_t  FileVars_GetEncoding(LPFILEVARS lpfv);

// ----------------------------------------------------------------------------

typedef struct _enc_det_t
{
  cpi_enc_t Encoding; // final detection result
  // statistic:
  cpi_enc_t forcedEncoding;
  cpi_enc_t fileVarEncoding;
  cpi_enc_t analyzedEncoding;
  cpi_enc_t unicodeAnalysis;
  float     confidence;
  // flags:
  bool bIsAnalysisReliable;
  bool bIs7BitASCII;
  bool bHasBOM;
  bool bIsReverse;
  bool bIsUTF8Sig;
  bool bValidUTF8;

  char encodingStrg[64];
  
} ENC_DET_T;

#define INIT_ENC_DET_T  { CPI_NONE, CPI_NONE, CPI_NONE, CPI_NONE, CPI_NONE, 0.0f, false, false, false, false, false, false, "" }


ENC_DET_T Encoding_DetectEncoding(LPWSTR pszFile, const char* lpData, const size_t cbData,
                                  cpi_enc_t iAnalyzeHint,
                                  bool bSkipUTFDetection, bool bSkipANSICPDetection, bool bForceEncDetection);

// ----------------------------------------------------------------------------

const WCHAR* Encoding_GetTitleInfo();
//const char*  Encoding_GetTitleInfoA();

// --------------------------------------------------------------------------------------------------------------------------------

#endif //_NP3_ENCODING_H_
