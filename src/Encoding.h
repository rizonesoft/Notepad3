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
*                                                  (c) Rizonesoft 2008-2018   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

#pragma once
#ifndef _NP3_ENCODING_H_
#define _NP3_ENCODING_H_

extern int g_DOSEncoding;
extern bool g_bForceCompEncDetection;

#define NCP_DEFAULT            1
#define NCP_UTF8               2
#define NCP_UTF8_SIGN          4
#define NCP_UNICODE            8
#define NCP_UNICODE_REVERSE   16
#define NCP_UNICODE_BOM       32
#define NCP_ANSI              64
#define NCP_OEM              128
#define NCP_MBCS             256
#define NCP_INTERNAL         (NCP_DEFAULT|NCP_UTF8|NCP_UTF8_SIGN|NCP_UNICODE|NCP_UNICODE_REVERSE|NCP_UNICODE_BOM|NCP_ANSI|NCP_OEM|NCP_MBCS)
#define NCP_EXTERNAL_8BIT    512
#define NCP_RECODE          1024

#define CED_NO_MAPPING       (-4)
#define CPI_ASCII_7BIT       (-3)
#define CPI_GET              (-2)
#define CPI_NONE             (-1)
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

//~#define PREFERRED_DAFAULT_ENCODING  CPI_ANSI_DEFAULT
#define PREFERRED_DEFAULT_ENCODING  CPI_UTF8

typedef struct _np2encoding {
  UINT    uFlags;
  UINT    uCodePage;
  const char* pszParseNames;
  int     idsName;
  int     iCEDEncoding;
  WCHAR   wchLabel[64];
} NP2ENCODING;

int  Encoding_CountOf();
int  Encoding_Current(int iEncoding);            // getter/setter
int  Encoding_SrcCmdLn(int iSrcEncoding);        // getter/setter
int  Encoding_SrcWeak(int iSrcWeakEnc);          // getter/setter
bool Encoding_HasChanged(int iOriginalEncoding); // query/setter

void Encoding_InitDefaults();
int  Encoding_MapIniSetting(bool, int iSetting);
int  Encoding_MapUnicode(int iUni);
void Encoding_SetLabel(int iEncoding);
int  Encoding_MatchW(LPCWSTR pwszTest);
int  Encoding_MatchA(char* pchTest);
bool Encoding_IsValid(int iTestEncoding);
int  Encoding_GetByCodePage(UINT cp);
void Encoding_AddToListView(HWND hwnd, int idSel, bool);
bool Encoding_GetFromListView(HWND hwnd, int * pidEncoding);
void Encoding_AddToComboboxEx(HWND hwnd, int idSel, bool);
bool Encoding_GetFromComboboxEx(HWND hwnd, int * pidEncoding);

UINT Encoding_GetCodePage(int iEncoding);

bool Encoding_IsDefault(int iEncoding);
bool Encoding_IsANSI(int iEncoding);
bool Encoding_IsOEM(int iEncoding);
bool Encoding_IsUTF8(int iEncoding);
bool Encoding_IsUTF8_SIGN(int iEncoding);
bool Encoding_IsMBCS(int iEncoding);
bool Encoding_IsUNICODE(int iEncoding);
bool Encoding_IsUNICODE_BOM(int iEncoding);
bool Encoding_IsUNICODE_REVERSE(int iEncoding);
bool Encoding_IsINTERNAL(int iEncoding);
bool Encoding_IsEXTERNAL_8BIT(int iEncoding);
bool Encoding_IsRECODE(int iEncoding);

// Scintilla related
#define Encoding_SciCP  CP_UTF8

void Encoding_SetDefaultFlag(int iEncoding);
const WCHAR* Encoding_GetLabel(int iEncoding);
const char* Encoding_GetParseNames(int iEncoding);

bool Has_UTF16_LE_BOM(const char* pBuf, int cnt);
bool Has_UTF16_BE_BOM(const char* pBuf, int cnt);

inline bool IsUTF8Signature(const char* p) {
  return ((p[0] == '\xEF') && (p[1] == '\xBB') && (p[2] == '\xBF'));
}
#define UTF8StringStart(p) (IsUTF8Signature(p)) ? ((p)+3) : (p)

bool IsValidUnicode(const char* pBuffer, size_t cb, bool*, bool*);
bool IsValidUTF7(const char* pTest, size_t nLength);
bool IsValidUTF8(const char* pTest, size_t nLength);

// Google's "Compact Encoding Detection" 
extern NP2ENCODING g_Encodings[];
void ChangeEncodingCodePage(int cpi, UINT newCP);
int Encoding_Analyze(const char* text, size_t len, int encodingHint, bool* pIsReliable);

// 932 Shift-JIS, 936 GBK, 949 UHC, 950 Big5, 1361 Johab
inline bool IsDBCSCodePage(UINT cp) {
  return ((cp == 932) || (cp == 936) || (cp == 949) || (cp == 950) || (cp == 1361));
}

// --------------------------------------------------------------------------------------------------------------------------------

#endif //_NP3_ENCODING_H_
