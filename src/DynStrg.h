/****************************************************************/
#pragma once

/**************************************************/
/*             Declared in WINNT.H                */
/*                                                */
/*  Provides bottom line type safety in function  */
/*  calls instead of using void* pointer          */
/**************************************************/
#ifndef DECLARE_HANDLE
#define DECLARE_HANDLE(name) \
    struct name##__ {        \
        int unused;          \
    };                       \
    typedef struct name##__ *name
#endif

#define STRAPI __stdcall


__forceinline size_t StrlenW(const wchar_t* p)
{
    return (!p) ? 0 : wcslen(p);
}
// ----------------------------------------------------------------------------


/**************************************************/
/*                                                */
/*          DYNAMIC WIDE CHAR C STRING            */
/*                                                */
/**************************************************/
DECLARE_HANDLE(HSTRINGW);

#define STRINGW_INVALID_IDX  ((size_t)-1)

HSTRINGW STRAPI          StrgCreate();
void STRAPI              StrgDestroy(HSTRINGW hstr);

int STRAPI               StrgIsEmpty(const HSTRINGW hstr);
size_t STRAPI            StrgGetLength(const HSTRINGW hstr);
size_t STRAPI            StrgGetAllocLength(const HSTRINGW hstr);

void STRAPI              StrgFreeExtra(HSTRINGW hstr);
void STRAPI              StrgEmpty(HSTRINGW hstr);

int STRAPI               StrgSet(HSTRINGW hstr, const wchar_t* str);
const wchar_t* STRAPI    StrgGet(const HSTRINGW hstr);
void STRAPI              StrgSetAt(HSTRINGW hstr, const size_t index, const wchar_t ch);
wchar_t STRAPI           StrgGetAt(const HSTRINGW hstr, const size_t index);
void STRAPI              StrgCat(HSTRINGW hstr, const wchar_t* str); /* concatenate */
size_t STRAPI            StrgInsert(HSTRINGW hstr, const size_t index, const wchar_t* str);
size_t STRAPI            StrgInsertCh(HSTRINGW hstr, size_t index, const wchar_t c);
size_t STRAPI            StrgReplace(HSTRINGW hstr, const wchar_t *pOld, const wchar_t *pNew);
size_t STRAPI            StrgRemove(HSTRINGW hstr, const wchar_t *str);
size_t STRAPI            StrgReplaceCh(HSTRINGW hstr, const wchar_t chOld, const wchar_t chNew);
size_t STRAPI            StrgRemoveCh(HSTRINGW hstr, const wchar_t chRemove);
size_t STRAPI            StrgDelete(HSTRINGW hstr, const size_t index, size_t count);

HSTRINGW STRAPI          StrgCopy(const HSTRINGW hstr);

void STRAPI              StrgToUpper(HSTRINGW hstr);
void STRAPI              StrgToLower(HSTRINGW hstr);
void STRAPI              StrgReverse(HSTRINGW hstr);

void STRAPI              StrgTrimRight(HSTRINGW hstr, const wchar_t wch);
void STRAPI              StrgTrimLeft(HSTRINGW hstr, const wchar_t wch);
void STRAPI              StrgTrim(HSTRINGW hstr, const wchar_t wch);

size_t STRAPI            StrgFind(const HSTRINGW hstr, const wchar_t* sub, const size_t start);
size_t STRAPI            StrgFindCh(const HSTRINGW hstr, const wchar_t wch, const size_t start);
size_t STRAPI            StrgReverseFind(const HSTRINGW hstr, wchar_t wch);
size_t STRAPI            StrgFindOneOf(const HSTRINGW hstr, const wchar_t* wchar_t_set);

HSTRINGW STRAPI          StrgMid(HSTRINGW hstr, const size_t start, size_t count);
HSTRINGW STRAPI          StrgLeft(HSTRINGW hstr, const size_t count);
HSTRINGW STRAPI          StrgRight(HSTRINGW hstr, const size_t count);

void STRAPI              StrgFormat(HSTRINGW hstr, const wchar_t* fmt, ...);


// use for PathLib Only !
#ifdef NP3_PATH_LIB_IMPLEMENTATION

wchar_t* STRAPI StrgWriteAccessBuf(HSTRINGW hstr, size_t min_len);
void STRAPI StrgSanitize(HSTRINGW hstr);  // correct strg length after buffer access

#endif
