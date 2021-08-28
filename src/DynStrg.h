/****************************************************************/
#pragma once

/**************************************************/
/*             Declared in WINNT.H                */
/*                                                */
/*  Provides bottom line type safety in function  */
/*  calls instead of using void* pointer          */
/**************************************************/
#ifndef DECLARE_HANDLE
#define DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
#endif

#define STRAPI __stdcall

/**************************************************/
/*                                                */
/*          DYNAMIC WIDE CHAR C STRING            */
/*                                                */
/**************************************************/
DECLARE_HANDLE(HSTRINGW);

#define STRINGW_INVALID_SIZE  ((size_t)-1)

HSTRINGW STRAPI          StrgCreate();
//~void STRAPI              StrgReserveBuffer(HSTRINGW hstr, size_t len);
void STRAPI              StrgDestroy(HSTRINGW hstr);

size_t STRAPI            StrgGetLength(HSTRINGW hstr);
size_t STRAPI            StrgGetAllocLength(HSTRINGW hstr);
int STRAPI               StrgIsEmpty(HSTRINGW hstr);

void STRAPI              StrgFreeExtra(HSTRINGW hstr);
void STRAPI              StrgEmpty(HSTRINGW hstr);

int STRAPI               StrgSet(HSTRINGW hstr, const wchar_t* str);
const wchar_t* STRAPI    StrgGet(HSTRINGW hstr);
void STRAPI              StrgSetAt(HSTRINGW hstr, size_t index, wchar_t ch);
wchar_t STRAPI           StrgGetAt(HSTRINGW hstr, size_t index);
void STRAPI              StrgCat(HSTRINGW hstr, const wchar_t* str); /* concatenate */
size_t STRAPI            StrgInsert(HSTRINGW hstr, size_t index, const wchar_t* str);
size_t STRAPI            StrgInsertCh(HSTRINGW hstr, size_t index, wchar_t c);
size_t STRAPI            StrgReplace(HSTRINGW hstr, const wchar_t *pOld, const wchar_t *pNew);
size_t STRAPI            StrgRemove(HSTRINGW hstr, const wchar_t *str);
size_t STRAPI            StrgReplaceCh(HSTRINGW hstr, wchar_t chOld, wchar_t chNew);
size_t STRAPI            StrgRemoveCh(HSTRINGW hstr, wchar_t chRemove);
size_t STRAPI            StrgDelete(HSTRINGW hstr, size_t index, size_t count);

HSTRINGW STRAPI          StrgCopy(HSTRINGW hstr);

void STRAPI              StrgToUpper(HSTRINGW hstr);
void STRAPI              StrgToLower(HSTRINGW hstr);
void STRAPI              StrgReverse(HSTRINGW hstr);

void STRAPI              StrgTrimRight(HSTRINGW hstr);
void STRAPI              StrgTrimLeft(HSTRINGW hstr);
void STRAPI              StrgTrim(HSTRINGW hstr);

size_t STRAPI            StrgFind(HSTRINGW hstr, const wchar_t* sub, size_t start);
size_t STRAPI            StrgFindCh(HSTRINGW hstr, wchar_t wch, size_t start);
size_t STRAPI            StrgReverseFind(HSTRINGW hstr, wchar_t wch);
size_t STRAPI            StrgFindOneOf(HSTRINGW hstr, const wchar_t* wchar_t_set);

HSTRINGW STRAPI          StrgMid(HSTRINGW hstr, size_t start, size_t count);
HSTRINGW STRAPI          StrgLeft(HSTRINGW hstr, size_t count);
HSTRINGW STRAPI          StrgRight(HSTRINGW hstr, size_t count);

void STRAPI              StrgFormat(HSTRINGW hstr, const wchar_t* fmt, ...);
