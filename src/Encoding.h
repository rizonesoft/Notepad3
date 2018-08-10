/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Encoding.h                                                                  *
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

#define CED_NO_MAPPING       (-3)
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

bool IsUnicode(const char*, int, bool*, bool*);
bool IsUTF8(const char*, int);
bool IsUTF7(const char*, int);

#define IsUTF8Signature(p) ((*((p)+0) == '\xEF' && *((p)+1) == '\xBB' && *((p)+2) == '\xBF'))
#define UTF8StringStart(p) (IsUTF8Signature(p)) ? ((p)+3) : (p)

size_t UTF8_mbslen_bytes(LPCSTR utf8_string);
size_t UTF8_mbslen(LPCSTR utf8_string, size_t byte_length);
bool UTF8_ContainsInvalidChars(LPCSTR utf8_string, size_t byte_length);

// Google's "Compact Encoding Detection" 
extern NP2ENCODING g_Encodings[];
int Encoding_CountOf();
void ChangeEncodingCodePage(int cpi, UINT newCP);
int Encoding_Analyze(const char* const text, const size_t len, const int encodingHint, bool* isReliable);

// --------------------------------------------------------------------------------------------------------------------------------

#endif //_NP3_ENCODING_H_
