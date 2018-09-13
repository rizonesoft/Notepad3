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


typedef struct _np2encoding {
  UINT    uFlags;
  UINT    uCodePage;
  char*   pszParseNames;
  int     idsName;
  int     iCEDEncoding;
  WCHAR   wchLabel[64];
} NP2ENCODING;

int  Encoding_CountOf();
int  Encoding_Current(int);    // getter/setter
int  Encoding_SrcCmdLn(int);     // getter/setter
int  Encoding_SrcWeak(int);    // getter/setter
bool Encoding_HasChanged(int); // query/setter

void Encoding_InitDefaults();
int  Encoding_MapIniSetting(bool, int);
int  Encoding_MapUnicode(int);
void Encoding_SetLabel(int);
int  Encoding_MatchW(LPCWSTR);
int  Encoding_MatchA(char*);
bool Encoding_IsValid(int);
int  Encoding_GetByCodePage(UINT);
void Encoding_AddToListView(HWND, int, bool);
bool Encoding_GetFromListView(HWND, int *);
void Encoding_AddToComboboxEx(HWND, int, bool);
bool Encoding_GetFromComboboxEx(HWND, int *);

UINT Encoding_GetCodePage(int);

bool Encoding_IsDefault(int);
bool Encoding_IsANSI(int);
bool Encoding_IsOEM(int);
bool Encoding_IsUTF8(int);
bool Encoding_IsUTF8_SIGN(int);
bool Encoding_IsMBCS(int);
bool Encoding_IsUNICODE(int);
bool Encoding_IsUNICODE_BOM(int);
bool Encoding_IsUNICODE_REVERSE(int);
bool Encoding_IsINTERNAL(int);
bool Encoding_IsEXTERNAL_8BIT(int);
bool Encoding_IsRECODE(int);

// Scintilla related
#define Encoding_SciCP  CP_UTF8

void Encoding_SetDefaultFlag(int);
const WCHAR* Encoding_GetLabel(int);
const char* Encoding_GetParseNames(int);

bool Has_UTF16_LE_BOM(const char* pBuf, int cnt);
bool Has_UTF16_BE_BOM(const char* pBuf, int cnt);

inline bool IsUTF8Signature(const char* p) {
  return ((p[0] == '\xEF') && (p[1] == '\xBB') && (p[2] == '\xBF'));
}
#define UTF8StringStart(p) (IsUTF8Signature(p)) ? ((p)+3) : (p)

bool IsValidUnicode(const char*, size_t, bool*, bool*);
bool IsValidUTF7(const char*, size_t);
bool IsValidUTF8(const char*, size_t);

// Google's "Compact Encoding Detection" 
extern NP2ENCODING g_Encodings[];
int Encoding_CountOf();
void ChangeEncodingCodePage(int cpi, UINT newCP);
int Encoding_Analyze(const char* const text, const size_t len, const int encodingHint, bool* isReliable);

// --------------------------------------------------------------------------------------------------------------------------------

#endif //_NP3_ENCODING_H_
