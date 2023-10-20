// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* DynStrg.c                                                                   *
*   Implementation for dynamic wide char string handling                      *
*   Based on code from                                                        *
*   https://www.codeproject.com/Articles/1259074/C-Language-Dynamic-String    *
*   by steveb (MIT license)                                                   *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2023   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

#define NOMINMAX
#define VC_EXTRALEAN 1
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

#include <stdlib.h>
#include <tchar.h>
#include <assert.h>
#include <stdarg.h>
#include <heapapi.h>
#include <strsafe.h>
#include <stringapiset.h>

#include "DynStrg.h"

#define STRINGW_MAX_CCH   STRSAFE_MAX_CCH

const wchar_t WCHR_NULL = L'\0';

typedef struct tagSTRINGW
{
    LPWSTR  data;
    size_t  data_length;
    size_t  alloc_length;

} STRINGW;

// ----------------------------------------------------------------------------

// min/max
#define _min_(x, y) (((x) > (y)) ? (y) : (x))
#define _RETCMPMIN_             \
    {                           \
        return (x > y) ? y : x; \
    }
    __forceinline size_t min_s(const size_t x, const size_t y) _RETCMPMIN_

#define _max_(x, y) (((x) < (y)) ? (y) : (x))
#define _RETCMPMAX_             \
    {                           \
        return (x < y) ? y : x; \
    }
    __forceinline size_t max_s(const size_t x, const size_t y) _RETCMPMAX_

// ----------------------------------------------------------------------------

/**************************************************/
/*                                                */
/*          HEAP ALLOC                           */
/*                                                */
/**************************************************/

// direct heap allocation
#if (defined(_DEBUG) || defined(DEBUG)) && !defined(NDEBUG)
#define DEFAULT_ALLOC_FLAGS (HEAP_GENERATE_EXCEPTIONS)
#else
#define DEFAULT_ALLOC_FLAGS (0)
#endif

static HANDLE s_hndlProcessHeap = NULL;

__forceinline LPVOID AllocMemStrg(size_t numBytes, DWORD dwFlags) {
    return HeapAlloc(s_hndlProcessHeap, (dwFlags | DEFAULT_ALLOC_FLAGS), numBytes);
}

__forceinline LPVOID ReAllocMemStrg(LPVOID lpMem, size_t numBytes, DWORD dwFlags) {
    return HeapReAlloc(s_hndlProcessHeap, (dwFlags | DEFAULT_ALLOC_FLAGS), lpMem, numBytes);
}

__forceinline bool FreeMemStrg(LPVOID lpMemory) {
    return (lpMemory ? HeapFree(s_hndlProcessHeap, 0, lpMemory) : true);
}

__forceinline size_t SizeOfMemStrg(LPCVOID lpMemory) {
    return (lpMemory ? HeapSize(s_hndlProcessHeap, 0, lpMemory) : 0);
}

// ----------------------------------------------------------------------------


/**************************************************/
/*                                                */
/*          PRIVATE API                           */
/*                                                */
/**************************************************/

__forceinline size_t limit_len(const size_t len) { return (((len) < STRINGW_MAX_CCH) ? max_s((8 / sizeof(wchar_t)), len) : (STRINGW_MAX_CCH - 1)); }

__forceinline STRINGW* ToWStrg(HSTRINGW hstr) { return (STRINGW*)hstr; }

inline static void * AllocBuffer(const size_t len) {
    if (!s_hndlProcessHeap) {
        s_hndlProcessHeap = GetProcessHeap();
    }
    return AllocMemStrg(limit_len(len) * sizeof(wchar_t), HEAP_ZERO_MEMORY);
}
// ----------------------------------------------------------------------------

inline static void * ReAllocBuffer(void* pdata, const size_t len, bool bInPlace) {
    if (!s_hndlProcessHeap) {
        s_hndlProcessHeap = GetProcessHeap();
    }
    DWORD const dwFlags = HEAP_ZERO_MEMORY | (bInPlace ? HEAP_REALLOC_IN_PLACE_ONLY : 0);
    return ReAllocMemStrg(pdata, limit_len(len) * sizeof(wchar_t), dwFlags);
}
// ----------------------------------------------------------------------------

#define LengthOfBuffer(PBUF) (SizeOfMemStrg(PBUF) / sizeof(wchar_t))

// ----------------------------------------------------------------------------

inline static void FreeBuffer(LPWSTR pstr) {
    if (!s_hndlProcessHeap) {
        s_hndlProcessHeap = GetProcessHeap();
    }
    FreeMemStrg(pstr);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

inline static void FreeBufferW(STRINGW* pstr) {
    if (!pstr || !pstr->data) {
        return;
    }
    FreeBuffer(pstr->data);
    pstr->data = NULL;
    pstr->alloc_length = 0;
    pstr->data_length = 0;
}
// ----------------------------------------------------------------------------

static void ReAllocW(STRINGW* pstr, size_t len)
{
    len = limit_len(len);
    size_t const alloc_len = len + 1;
    if (!pstr->data) {
        pstr->data = AllocBuffer(alloc_len);
        if (pstr->data) { // init
            pstr->alloc_length = LengthOfBuffer(pstr->data);
            assert("inconsistent data" && (alloc_len == pstr->alloc_length));
            pstr->data_length = 0;
        }
        else {
            pstr->alloc_length = 0;
            pstr->data_length = 0;
        }
    }
    else if (pstr->alloc_length < alloc_len) {
        pstr->data = ReAllocBuffer(pstr->data, alloc_len, false);
        pstr->alloc_length = LengthOfBuffer(pstr->data);
        assert("inconsistent data 1" && (alloc_len == pstr->alloc_length));
        /// original memory block is moved, so data_length is not touched
        assert("inconsistent data 2" && (alloc_len > pstr->data_length));
        pstr->data[pstr->data_length] = WCHR_NULL; // ensure terminating zero
    }
    else {
        ZeroMemory(&(pstr->data[pstr->data_length]), (pstr->alloc_length - pstr->data_length) * sizeof(wchar_t));
    }
}
// ----------------------------------------------------------------------------

static void AllocCopyW(STRINGW* pstr, STRINGW* pDest, size_t copy_len, size_t copy_index, size_t extra_len)
{
    size_t new_len = copy_len + extra_len;
    if (0 < new_len)
    {
        ReAllocW(pDest, new_len);
        StringCchCopyNW(pDest->data, pDest->alloc_length, (pstr->data + copy_index), copy_len);
        pDest->data_length = StrlenW(pstr->data);
    }
}
// ----------------------------------------------------------------------------

static void SetCopyW(STRINGW* pstr, size_t len, LPCWSTR p)
{
    ReAllocW(pstr, len);
    if (pstr->data) {
        StringCchCopyNW(pstr->data, pstr->alloc_length, p, len);
        pstr->data_length = StrlenW(pstr->data);
    }
}
// ----------------------------------------------------------------------------

#if 0
//~ replaced by ReAllocW()

static LPWSTR CopyOldDataW(STRINGW* pstr, size_t* outLen)
{
    size_t const old_siz = StrlenW(pstr->data) + 1;
    LPWSTR const ptr = AllocBuffer(old_siz);
    if (ptr) {
        StringCchCopyW(ptr, old_siz, pstr->data ? pstr->data : L"");
        *outLen = wcslen(ptr);
    }
    return ptr; // transfer ownership
}
#endif
// ----------------------------------------------------------------------------


static void FreeUnusedData(STRINGW* pstr, size_t keep_length)
{
    size_t const new_alloc_len = max_s(keep_length + 1, pstr->data_length + 1);
    if ((pstr->alloc_length > new_alloc_len) ) {
        pstr->data = ReAllocBuffer(pstr->data, new_alloc_len, false);
        pstr->alloc_length = LengthOfBuffer(pstr->data);
        pstr->data_length = StrlenW(pstr->data);
    }
    // else: not reserve memory up to keep_length if shorter
}
// ----------------------------------------------------------------------------

static void CopyConcatW(STRINGW *pstr, size_t len1, LPCWSTR p1, size_t len2, LPCWSTR p2)
{
    size_t const new_len = len1 + len2;
    if (0 < new_len) {
        ReAllocW(pstr, new_len);
        StringCchCopyNW(pstr->data, pstr->alloc_length, p1, len1);
        StringCchCatNW(pstr->data, pstr->alloc_length, p2, len2);
        pstr->data_length = StrlenW(pstr->data);
    }
}
// ----------------------------------------------------------------------------

static void ConcatW(STRINGW* pstr, size_t len, LPCWSTR p)
{
    if (len == 0)
        return;

    size_t const new_len = pstr->data_length + len;
    if (pstr->alloc_length <= new_len) {
        ReAllocW(pstr, new_len); // copies old data
    }
    StringCchCatNW(pstr->data, pstr->alloc_length, p, len);
    pstr->data_length = StrlenW(pstr->data);
}
// ----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable : 6269)

static void FormatW(STRINGW* pstr, LPCWSTR fmt, va_list args)
{
    va_list orig_list = args;

    size_t max_len = 0;
    LPCWSTR  p;

    for (p = fmt; *p != WCHR_NULL; p = _wcsinc(p)) {
        size_t item_len = 0, width = 0, prec = 0, modif = 0;
        if (*p != L'%' || *(p = _wcsinc(p)) == L'%') {
            ++max_len;
            continue;
        }
        item_len = 0;
        width = 0;
        for (; *p != WCHR_NULL; p = _wcsinc(p)) {
            if (*p == L'#')
                max_len += 2; /* L'0x'*/
            else if (*p == L'*')
                width = va_arg(args, int);
            else if (*p == L'-' || *p == L'+' || *p == L'0' ||
                     *p == L' ')
                ;
            else
                break;
        }
        if (width == 0) {
            width = _wtoi(p);
            for (; *p != WCHR_NULL && isdigit(*p); p = _wcsinc(p))
                ;
        }
        assert("negative width" && (width >= 0));

        prec = 0;
        if (*p == L'.') {
            p = _wcsinc(p);

            if (*p == L'*') {
                prec = va_arg(args, int);
                p = _wcsinc(p);
            } else {
                prec = _ttoi(p);
                for (; *p != WCHR_NULL && isdigit(*p); p = _wcsinc(p))
                    ;
            }
            assert("negative prec" && (prec >= 0));
        }

        modif = 0;
        if (wcsncmp(p, L"I64", 3) == 0) {
            p += 3;
            modif = 0x40000;
        } else {
            switch (*p) {
            case L'h':
                modif = 0x10000;
                p = _wcsinc(p);
                break;
            case L'l':
                modif = 0x20000;
                p = _wcsinc(p);
                break;

            case L'F':
            case L'N':
            case L'L':
                p = _wcsinc(p);
                break;
            }
        }

        switch (*p | modif) {
        case L'c' | 0x20000:
        case L'C' | 0x20000:
            item_len = 2;
            va_arg(args, wchar_t);
            break;

        case L's':
        case L'S': {
            LPWSTR const next_arg = va_arg(args, LPWSTR);
            if (!next_arg)
                item_len = 6;
            else {
                item_len = wcslen(next_arg);
                item_len = max(1, item_len);
            }
        } break;

        case L's' | 0x20000:
        case L'S' | 0x20000: {
            LPWSTR const next_arg = va_arg(args, LPWSTR);
            if (!next_arg)
                item_len = 6;
            else {
                item_len = wcslen(next_arg);
                item_len = max(1, item_len);
            }
        } break;
        }

        if (item_len != 0) {
            if (prec != 0)
                item_len = min(item_len, prec);
            item_len = max(item_len, width);
        } else {
            switch (*p) {
            case L'd':
            case L'i':
            case L'u':
            case L'x':
            case L'X':
            case L'o':
                if (modif & 0x40000)
                    va_arg(args, __int64);
                else
                    va_arg(args, int);
                item_len = 32;
                item_len = max(item_len, width + prec);
                break;

            case L'e':
            case L'f':
            case L'g':
            case L'G':
                va_arg(args, double);
                item_len = 128;
                item_len = max(item_len, width + prec);
                break;

            case L'p':
                va_arg(args, void *);
                item_len = 32;
                item_len = max(item_len, width + prec);
                break;

            case L'n':
                va_arg(args, int *);
                break;

            default:
                assert("unknown format" && 0);
                break;
            }
        }
        max_len += item_len;
    }

    ReAllocW(pstr, max_len);

    StringCchVPrintfW(pstr->data, pstr->alloc_length, fmt, orig_list);
    pstr->data_length = StrlenW(pstr->data);

    va_end(orig_list);
}
#pragma warning(pop)
// ----------------------------------------------------------------------------


/**************************************************/
/*                                                */
/*              PUBLIC API                        */
/*                                                */
/**************************************************/

HSTRINGW STRAPI StrgCreate(LPCWSTR str)
{
    STRINGW *pstr = AllocBuffer(sizeof(STRINGW));
    if (!pstr)
        return NULL;
    if (str)
        SetCopyW(pstr, StrlenW(str), str);
    else
        pstr->data = NULL;

    return (HSTRINGW)pstr;
}
// ----------------------------------------------------------------------------


void STRAPI StrgDestroy(HSTRINGW hstr)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr)
        return;
    FreeBufferW(pstr);
    FreeBuffer((LPWSTR)pstr);
}
// ----------------------------------------------------------------------------


int STRAPI StrgReset(HSTRINGW hstr, LPCWSTR str)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr)
        return -1;
    SetCopyW(pstr, StrlenW(str), str);
    return  (pstr->data == NULL) ? 0 : 1; 
}
// ----------------------------------------------------------------------------


LPCWSTR STRAPI StrgGet(const HSTRINGW hstr)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr)
        return NULL;
    return pstr->data;
}
// ----------------------------------------------------------------------------


int STRAPI StrgIsEmpty(const HSTRINGW hstr)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr)
        return !0;

    int const res = (!pstr->data || ((pstr->data)[0] == WCHR_NULL)) ? !0 : 0;

    assert("inconsistent data" && (pstr->data_length != (size_t)res));
    return res;
}
// ----------------------------------------------------------------------------


size_t STRAPI StrgGetLength(const HSTRINGW hstr)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr)
        return 0;
    return pstr->data_length;
}
// ----------------------------------------------------------------------------


size_t STRAPI StrgGetAllocLength(const HSTRINGW hstr)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr)
        return 0;
    return pstr->alloc_length;
}
// ----------------------------------------------------------------------------


void STRAPI StrgFree(HSTRINGW hstr)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr)
        return;
    FreeBufferW(pstr);
}
// ----------------------------------------------------------------------------


void STRAPI StrgFreeExtra(HSTRINGW hstr, size_t keep_length)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr)
        return;
    FreeUnusedData(pstr, keep_length);
}
// ----------------------------------------------------------------------------


void STRAPI StrgEmpty(const HSTRINGW hstr, bool truncate)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr)
        return;
    if (!pstr->data) {
        ReAllocW(pstr, 0);
        return;
    }
    (pstr->data)[0] = WCHR_NULL;
    pstr->data_length = 0;
    if (truncate) {
        FreeUnusedData(pstr, 0);
    }
}
// ----------------------------------------------------------------------------


void STRAPI StrgSetAt(HSTRINGW hstr, const size_t index, const wchar_t ch)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr)
        return;
    if (!(pstr->data)) {
        ReAllocW(pstr, 0);
    }
    if (index >= pstr->data_length)
    {
        assert("buffer too small" && 0);
        return;
    }
    if (pstr->data)
        (pstr->data)[index] = ch;
}
// ----------------------------------------------------------------------------


wchar_t STRAPI StrgGetAt(const HSTRINGW hstr, const size_t index)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr)
        return WCHR_NULL;
    if (!pstr->data) {
        ReAllocW(pstr, 0);
    }
    if (index >= pstr->data_length)
    {
        assert("buffer too small" && 0);
        return WCHR_NULL;
    }
    return (pstr->data) ? pstr->data[index] : WCHR_NULL;
}
// ----------------------------------------------------------------------------


HSTRINGW STRAPI StrgCopy(const HSTRINGW hstr)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr)
        return NULL;
    return StrgCreate(StrgGet(hstr));
}
// ----------------------------------------------------------------------------


void STRAPI StrgSwap(HSTRINGW hstr1, HSTRINGW hstr2)
{
    STRINGW* pstr1 = ToWStrg(hstr1);
    STRINGW* pstr2 = ToWStrg(hstr2);
    assert("swap nil" && ((pstr1 != NULL) && (pstr2 != NULL)));
    if (!pstr1 || !pstr2)
        return;
    
    LPWSTR const ptmp_data = pstr1->data;
    size_t const tmp_data_len = pstr1->data_length;
    size_t const tmp_alloc_len = pstr1->alloc_length;

    pstr1->data = pstr2->data;
    pstr1->data_length = pstr2->data_length;
    pstr1->alloc_length = pstr2->alloc_length;

    pstr2->data = ptmp_data;
    pstr2->data_length = tmp_data_len;
    pstr2->alloc_length = tmp_alloc_len;
}
// ----------------------------------------------------------------------------


void STRAPI StrgCat(HSTRINGW hstr, LPCWSTR str)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr)
        return;
    ConcatW(pstr, StrlenW(str), str);
}
// ----------------------------------------------------------------------------


size_t STRAPI StrgInsert(HSTRINGW hstr, size_t index, LPCWSTR str)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr)
        return STRINGW_INVALID_IDX;
    if (!pstr->data) {
        ReAllocW(pstr, 0);
    }

    size_t const ins_len = StrlenW(str);
    size_t new_len = pstr->data_length;

    if (ins_len > 0)
    {
        if (index > new_len) {
            index = new_len;
        }
        new_len += ins_len;

        if (pstr->alloc_length <= new_len) {
            ReAllocW(pstr, new_len);
        }
        wmemmove_s((pstr->data + index + ins_len), (pstr->alloc_length - index - ins_len),
                   (pstr->data + index), (new_len - index - ins_len + 1));
        wmemcpy_s((pstr->data + index), (pstr->alloc_length - index), str, ins_len);
        pstr->data_length = StrlenW(pstr->data);
    }
    return new_len;
}
// ----------------------------------------------------------------------------


size_t STRAPI StrgInsertCh(HSTRINGW hstr, size_t index, const wchar_t c)
{
    STRINGW *pstr = ToWStrg(hstr);
    if (!pstr)
        return 0;
    if (!pstr->data) {
        ReAllocW(pstr, 0);
    }
    size_t const new_len = pstr->data_length + 1;
    if (index >= new_len)
        index = new_len - 1;

    if (pstr->alloc_length <= new_len) {
        ReAllocW(pstr, new_len);
    }
    wmemmove_s((pstr->data + index + 1), (pstr->alloc_length - index - 1),
               (pstr->data + index), (new_len - index));
    if (pstr->data)
        pstr->data[index] = c;
    pstr->data_length = StrlenW(pstr->data);
    return new_len;
}
// ----------------------------------------------------------------------------


size_t STRAPI StrgReplace(HSTRINGW hstr, LPCWSTR pOld, LPCWSTR pNew)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr)
        return 0;
    if (!pstr->data) {
        ReAllocW(pstr, 0);
    }

    size_t const src_len = StrlenW(pOld);
    if (src_len == 0)
        return 0;
    
    size_t const repl_len = StrlenW(pNew);
    LPWSTR start = pstr->data;
    LPWSTR end = pstr->data + pstr->data_length;
    LPWSTR target = NULL;
    
    size_t count = 0;
    while (start < end)
    {
        while ((target = wcsstr(start, pOld)) != NULL)
        {
            count++;
            start = target + src_len;
        }
        start += wcslen(start) + 1;
    }

    if (count > 0)
    {
        size_t old_len = pstr->data_length;
        size_t const new_len =  old_len + (repl_len - src_len) * count;

        if (pstr->alloc_length <= new_len) {
            ReAllocW(pstr, new_len);
        }
        start = pstr->data;
        end = pstr->data + pstr->data_length;

        while (start < end)
        {
            while((target = wcsstr(start, pOld)) != NULL)
            {
                size_t bal = old_len - (target - pstr->data + src_len);
                wmemmove_s(target + repl_len, (pstr->alloc_length - (target - pstr->data) - repl_len), target + src_len, bal);
                wmemcpy_s(target, (pstr->alloc_length - (target - pstr->data)), pNew, repl_len);
                start = target + repl_len;
                start[bal] = WCHR_NULL;
                old_len += (repl_len - src_len);
            }
            start += wcslen(start) + 1;
        }
        pstr->data_length = StrlenW(pstr->data);
    }

    return count;
}
// ----------------------------------------------------------------------------


size_t STRAPI StrgRemove(HSTRINGW hstr, LPCWSTR str)
{
    return StrgReplace(hstr, str, L"");
}
// ----------------------------------------------------------------------------


size_t STRAPI StrgReplaceCh(HSTRINGW hstr, const wchar_t chOld, const wchar_t chNew)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr)
        return 0;
    if (!pstr->data) {
        ReAllocW(pstr, 0);
    }

    size_t count = 0;
    if (chOld != chNew)
    {
        LPWSTR p = pstr->data;
        LPWSTR end = p + pstr->data_length;
        while (p < end)
        {
            if (*p == chOld)
            {
                *p = chNew;
                count++;
            }
            p++;
        }
    }
    return count;
}
// ----------------------------------------------------------------------------


size_t STRAPI StrgRemoveCh(HSTRINGW hstr, const wchar_t chRemove)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr)
        return 0;
    if (!pstr->data) {
        ReAllocW(pstr, 0);
    }

    LPWSTR  source = pstr->data;
    LPWSTR  dest = pstr->data;
    LPWSTR  end = pstr->data + pstr->data_length;

    size_t count = 0;
    while (source < end)
    {
        if (*source != chRemove)
        {
            *dest = *source;
            dest++;
        }
        source++;
    }
    if (dest)
        *dest = WCHR_NULL;
    count = (int)(ptrdiff_t)(source - dest);
    pstr->data_length -= count;

    return count;
}
// ----------------------------------------------------------------------------


size_t STRAPI StrgDelete(HSTRINGW hstr, const size_t index, size_t count)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr)
        return 0;
    if (!pstr->data) {
        ReAllocW(pstr, 0);
    }

    size_t const len = pstr->data_length;
    if (count > 0 && index < len)
    {
        count = min_s(count, (len - index));
        size_t copy = len - (index + count) + 1;
        wmemmove_s((pstr->data + index), (pstr->alloc_length - index),
                   (pstr->data + index + count), copy);
        pstr->data_length = len - count;
    }
    return len;
}
// ----------------------------------------------------------------------------


int STRAPI StrgGetAsUTF8(const HSTRINGW hstr, char* chStrg, int cch)
{
    return WideCharToMultiByte(CP_UTF8, 0, StrgGet(hstr), -1, chStrg, cch, NULL, NULL);
}
// ----------------------------------------------------------------------------


int STRAPI StrgResetFromUTF8(HSTRINGW hstr, const char* str)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr || !str)
        return -1;
    int const len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0) + 1;
    ReAllocW(pstr, len);
    int const res = MultiByteToWideChar(CP_UTF8, 0, str, -1, pstr->data, (int)pstr->alloc_length);
    pstr->data_length = StrlenW(pstr->data);
    return res;
}
// ----------------------------------------------------------------------------


void STRAPI StrgToUpper(HSTRINGW hstr)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr)
        return;
    if (!pstr->data) {
        ReAllocW(pstr, 0);
    }
    if (pstr->data)
        _wcsupr_s(pstr->data, pstr->data_length);
}
// ----------------------------------------------------------------------------


void STRAPI StrgToLower(HSTRINGW hstr)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr)
        return;
    if (!pstr->data) {
        ReAllocW(pstr, 0);
    }
    if (pstr->data)
        _wcslwr_s(pstr->data, pstr->data_length);
}
// ----------------------------------------------------------------------------


void STRAPI StrgReverse(HSTRINGW hstr)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr)
        return;
    if (!pstr->data) {
        ReAllocW(pstr, 0);
    }
    _wcsrev(pstr->data);
}
// ----------------------------------------------------------------------------


void STRAPI StrgTrimRight(HSTRINGW hstr, const wchar_t wch)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr)
        return;
    if (!pstr->data) {
        ReAllocW(pstr, 0);
    }

    LPWSTR  start = pstr->data;
    LPWSTR  end = NULL;

    while (start && (*start != WCHR_NULL))
    {
        if (isspace(*start) || (wch ? (*start == wch) : 0))
        {
            if (end == NULL)
                end = start;
        }
        else
            end = NULL;
        start++;
    }

    if (end != NULL)
    {
        *end = WCHR_NULL;
        pstr->data_length = end - pstr->data;
    }
}
// ----------------------------------------------------------------------------


void STRAPI StrgTrimLeft(HSTRINGW hstr, const wchar_t wch)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr)
        return;
    if (!pstr->data) {
        ReAllocW(pstr, 0);
    }

    LPWSTR  start = pstr->data;

    while (start && (isspace(*start) || (wch ? (*start == wch) : 0)))
        start++;

    if (start != pstr->data)
    {
        size_t data_length = pstr->data_length - (start - pstr->data);
        wmemmove_s(pstr->data, pstr->alloc_length, start, (data_length + 1));
        pstr->data_length = data_length;
    }
}
// ----------------------------------------------------------------------------


void STRAPI StrgTrim(HSTRINGW hstr, const wchar_t wch)
{
    StrgTrimRight(hstr, wch);
    StrgTrimLeft(hstr, wch);
}
// ----------------------------------------------------------------------------


size_t STRAPI StrgFind(const HSTRINGW hstr, LPCWSTR sub, const size_t start)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr || !pstr->data)
        return STRINGW_INVALID_IDX;

    if (start >= pstr->data_length)
        return STRINGW_INVALID_IDX;

    LPWSTR  str = wcsstr(pstr->data + start, sub);

    return (str == NULL) ? STRINGW_INVALID_IDX : (size_t)(str - pstr->data);
}
// ----------------------------------------------------------------------------


size_t STRAPI StrgFindCh(const HSTRINGW hstr, const wchar_t ch, const size_t start)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr || !pstr->data)
        return STRINGW_INVALID_IDX;

    if (start >= pstr->data_length)
        return STRINGW_INVALID_IDX;

    LPWSTR  p = wcschr(pstr->data + start, ch);

    return (p == NULL) ? STRINGW_INVALID_IDX : (size_t)(p - pstr->data);
}
// ----------------------------------------------------------------------------


size_t STRAPI StrgReverseFind(const HSTRINGW hstr, wchar_t ch)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr || !pstr->data)
        return STRINGW_INVALID_IDX;

    LPWSTR  p = wcsrchr(pstr->data, ch);

    return (p == NULL) ? STRINGW_INVALID_IDX : (size_t)(p - pstr->data);
}
// ----------------------------------------------------------------------------


size_t STRAPI StrgFindOneOf(const HSTRINGW hstr, LPCWSTR char_set)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr || !pstr->data)
        return STRINGW_INVALID_IDX;

    LPWSTR  p = wcspbrk(pstr->data, char_set);
    return (p == NULL) ? STRINGW_INVALID_IDX : (size_t)(p - pstr->data);
}
// ----------------------------------------------------------------------------


HSTRINGW STRAPI StrgMid(HSTRINGW hstr, const size_t start, size_t count)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr)
        return NULL;
    if (!pstr->data) {
        ReAllocW(pstr, 0);
    }

    if (start + count > pstr->data_length)
        count = pstr->data_length - start;
    if (start > pstr->data_length)
        count = 0;

    assert("snap too large" && (start + count <= pstr->data_length));

    HSTRINGW hCopy = StrgCreate(NULL);
    STRINGW* pCopy = ToWStrg(hCopy);

    if (start == 0 && start + count == pstr->data_length) {
        SetCopyW(pCopy, pstr->data_length, pstr->data);
    } 
    else {
        AllocCopyW(pstr, pCopy, count, start, 0);
    }
    return hCopy;
}
// ----------------------------------------------------------------------------


HSTRINGW STRAPI StrgLeft(HSTRINGW hstr, const size_t count)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr)
        return NULL;
    if (!pstr->data) {
        ReAllocW(pstr, 0);
    }

    HSTRINGW hCopy = StrgCreate(NULL);
    STRINGW* pCopy = ToWStrg(hCopy);

    if (count >= pstr->data_length) {
        SetCopyW(pCopy, pstr->data_length, pstr->data);
    } else {
        AllocCopyW(pstr, pCopy, count, 0, 0);
    }
    return hCopy;
}
// ----------------------------------------------------------------------------


HSTRINGW STRAPI StrgRight(HSTRINGW hstr, const size_t count)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr)
        return NULL;
    if (!pstr->data) {
        ReAllocW(pstr, 0);
    }

    HSTRINGW hCopy = StrgCreate(NULL);
    STRINGW* pCopy = ToWStrg(hCopy);

    if (count >= pstr->data_length) {
        SetCopyW(pCopy, pstr->data_length, pstr->data);
    } else {
        AllocCopyW(pstr, pCopy, count, pstr->data_length - count, 0);
    }
    return hCopy;
}
// ----------------------------------------------------------------------------


void STRAPI StrgFormat(HSTRINGW hstr, LPCWSTR fmt, ...)
{
    STRINGW* const pstr = ToWStrg(hstr);
    if (!pstr)
        return;
    va_list args;
    va_start(args, fmt);
    FormatW(pstr, fmt, args);
    va_end(args);
}
// ----------------------------------------------------------------------------

// ############################################################################

LPWSTR STRAPI StrgWriteAccessBuf(HSTRINGW hstr, size_t min_len)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr)
        return NULL;

    if (pstr->alloc_length <= min_len) {
        ReAllocW(pstr, min_len);
    }
    return pstr->data;
}
// ----------------------------------------------------------------------------

void STRAPI StrgSanitize(HSTRINGW hstr)
{
    STRINGW* pstr = ToWStrg(hstr);
    if (!pstr)
        return;
    if (!pstr->data) {
        ReAllocW(pstr, 0);
    }
    // ensure buffer limits
    size_t const buflen = LengthOfBuffer(pstr->data);
    if (pstr->data && (buflen > 0))
        pstr->data[buflen - 1] = WCHR_NULL;
    pstr->alloc_length = buflen;
    pstr->data_length = StrlenW(pstr->data);
}
// --------------------------------------------------------------------------

// ############################################################################
