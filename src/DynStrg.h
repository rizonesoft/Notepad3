// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* DynStrg.h                                                                   *
*   Definitions for dynamic wide char string handling                         *
*   Based on code from                                                        *
*   https://www.codeproject.com/Articles/1259074/C-Language-Dynamic-String    *
*   by steveb (MIT license)                                                   *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2025   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#pragma once

#include "TypeDefs.h"

#define STRAPI __stdcall

// ----------------------------------------------------------------------------

/**************************************************/
/*                                                */
/*          DYNAMIC WIDE CHAR C STRING            */
/*                                                */
/**************************************************/

#define STRINGW_MAX_URL_LENGTH   INTERNET_MAX_URL_LENGTH 
#define STRINGW_INVALID_IDX      ((size_t)-1)

__forceinline size_t     StrlenW(LPCWSTR p) { return (!p) ? 0 : wcslen(p); }

HSTRINGW STRAPI          StrgCreate(LPCWSTR str);
void STRAPI              StrgDestroy(HSTRINGW hstr);

LPCWSTR STRAPI           StrgGet(const HSTRINGW hstr);
int STRAPI               StrgIsEmpty(const HSTRINGW hstr);
inline int STRAPI        StrgIsNotEmpty(const HSTRINGW hstr) { return !StrgIsEmpty(hstr); };
size_t STRAPI            StrgGetLength(const HSTRINGW hstr);
size_t STRAPI            StrgGetAllocLength(const HSTRINGW hstr);

void STRAPI              StrgFree(HSTRINGW hstr);        // NULL PTR 
void STRAPI              StrgFreeExtra(HSTRINGW hstr, size_t keep_length); // shrink not below keep_len
void STRAPI              StrgEmpty(const HSTRINGW hstr, bool truncate); // -> L""  

int STRAPI               StrgReset(HSTRINGW hstr, LPCWSTR str);
void STRAPI              StrgSetAt(HSTRINGW hstr, const size_t index, const wchar_t ch);
wchar_t STRAPI           StrgGetAt(const HSTRINGW hstr, const size_t index);
HSTRINGW STRAPI          StrgCopy(const HSTRINGW hstr);
void STRAPI              StrgSwap(HSTRINGW hstr1, HSTRINGW hstr2); // ensure not NULL
void STRAPI              StrgCat(HSTRINGW hstr, LPCWSTR str); /* concatenate */
size_t STRAPI            StrgInsert(HSTRINGW hstr, const size_t index, LPCWSTR str);
size_t STRAPI            StrgInsertCh(HSTRINGW hstr, size_t index, const wchar_t c);
size_t STRAPI            StrgReplace(HSTRINGW hstr, const wchar_t *pOld, const wchar_t *pNew);
size_t STRAPI            StrgRemove(HSTRINGW hstr, const wchar_t *str);
size_t STRAPI            StrgReplaceCh(HSTRINGW hstr, const wchar_t chOld, const wchar_t chNew);
size_t STRAPI            StrgRemoveCh(HSTRINGW hstr, const wchar_t chRemove);
size_t STRAPI            StrgDelete(HSTRINGW hstr, const size_t index, size_t count);

int STRAPI               StrgGetAsUTF8(const HSTRINGW hstr, char* chStrg, int cch);
int STRAPI               StrgResetFromUTF8(HSTRINGW hstr, const char* str);
inline int STRAPI        StrgLengthAsUTF8(const HSTRINGW hstr) { return StrgGetAsUTF8(hstr, NULL, 0) - 1; };

void STRAPI              StrgToUpper(HSTRINGW hstr);
void STRAPI              StrgToLower(HSTRINGW hstr);
void STRAPI              StrgReverse(HSTRINGW hstr);

void STRAPI              StrgTrimRight(HSTRINGW hstr, const wchar_t wch);
void STRAPI              StrgTrimLeft(HSTRINGW hstr, const wchar_t wch);
void STRAPI              StrgTrim(HSTRINGW hstr, const wchar_t wch);

size_t STRAPI            StrgFind(const HSTRINGW hstr, LPCWSTR sub, const size_t start);
size_t STRAPI            StrgFindCh(const HSTRINGW hstr, const wchar_t wch, const size_t start);
size_t STRAPI            StrgReverseFind(const HSTRINGW hstr, wchar_t wch);
size_t STRAPI            StrgFindOneOf(const HSTRINGW hstr, LPCWSTR wchar_t_set);

HSTRINGW STRAPI          StrgMid(HSTRINGW hstr, const size_t start, size_t count);
HSTRINGW STRAPI          StrgLeft(HSTRINGW hstr, const size_t count);
HSTRINGW STRAPI          StrgRight(HSTRINGW hstr, const size_t count);

void STRAPI              StrgFormat(HSTRINGW hstr, LPCWSTR fmt, ...);

// use together (after external access) - may get consistency issues if not (!)
LPWSTR STRAPI StrgWriteAccessBuf(HSTRINGW hstr, size_t min_len); // min_len = 0   for not resizing buffer
void STRAPI   StrgSanitize(HSTRINGW hstr);  // correct string length after buffer access
