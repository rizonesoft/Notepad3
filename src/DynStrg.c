/****************************************************************/

#define VC_EXTRALEAN 1
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

#include <stdlib.h>
#include <tchar.h>
#include <assert.h>
//#include <ctype.h>
#include <stdarg.h>
#include <heapapi.h>
#include <strsafe.h>

#include "DynStrg.h"

#define STRINGW_MAX_CCH   STRSAFE_MAX_CCH

const DWORD STR_CCH_FLAGS = 0UL | STRSAFE_FILL_BEHIND_NULL | STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE;

typedef struct tagSTRINGW
{
    wchar_t*  data;
    size_t    data_length;
    size_t    alloc_length;

} STRINGW;

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

__forceinline BOOL FreeMemStrg(LPVOID lpMemory) {
    return (lpMemory ? HeapFree(s_hndlProcessHeap, 0, lpMemory) : TRUE);
}

__forceinline size_t SizeOfMemStrg(LPCVOID lpMemory) {
    return (lpMemory ? HeapSize(s_hndlProcessHeap, 0, lpMemory) : 0);
}


/**************************************************/
/*                                                */
/*          PRIVATE API                           */
/*                                                */
/**************************************************/

__forceinline STRINGW* STRAPI ToWString(HSTRINGW hstr)
{
    if (!hstr)
        return NULL;
    return (STRINGW*)hstr;
}

__forceinline size_t STRAPI StrlenW(const wchar_t *p) { 
    return (!p) ? 0 : wcslen(p); 
}

inline static void * STRAPI AllocBuffer(size_t len, size_t char_size, BOOL bZeroMem) {
    if (!s_hndlProcessHeap) {
        s_hndlProcessHeap = GetProcessHeap();
    }
    return AllocMemStrg(len * char_size, bZeroMem ? HEAP_ZERO_MEMORY : 0);
}

inline static void * STRAPI ReAllocBuffer(void* pdata, size_t len, size_t char_size, BOOL bZeroMem) {
    if (!s_hndlProcessHeap) {
        s_hndlProcessHeap = GetProcessHeap();
    }
    return ReAllocMemStrg(pdata, len * char_size, bZeroMem ? HEAP_ZERO_MEMORY : 0);
}

inline static void STRAPI FreeBuffer(wchar_t * pstr) {
    if (!s_hndlProcessHeap) {
        s_hndlProcessHeap = GetProcessHeap();
    }
    FreeMemStrg(pstr);
}

inline static void STRAPI FreeBufferW(STRINGW* pstr) {
    if (!pstr->data) {
        return;
    }
    FreeBuffer(pstr->data);
    pstr->data = NULL;
    pstr->alloc_length = 0;
    pstr->data_length = 0;
}

static void STRAPI AllocW(STRINGW* pstr, size_t len)
{
    if (len == 0)
        FreeBufferW(pstr);
    else
    {
        size_t const alloc_len = len + 1;
        if (!pstr->data) {
            pstr->data = AllocBuffer(alloc_len, sizeof(wchar_t), FALSE);
        } else if (len >= pstr->alloc_length) {
            pstr->data = ReAllocBuffer(pstr->data, alloc_len, sizeof(wchar_t), FALSE);
        }
        if (pstr->data) // init
        {
            pstr->alloc_length = alloc_len;
            pstr->data_length = 0;
            pstr->data[len] = L'\0';
            pstr->data[0] = L'\0';
        }
    }
}

static void STRAPI AllocCopyW(STRINGW* pstr, STRINGW* pDest, size_t copy_len, size_t copy_index, size_t extra_len)
{
    size_t new_len = copy_len + extra_len;
    if (new_len > 0)
    {
        AllocW(pDest, new_len);
        wchar_t * endptr = NULL;
        StringCchCopyNExW(pDest->data, pDest->alloc_length, pstr->data + copy_index, copy_len, &endptr, NULL, STR_CCH_FLAGS);
        pDest->data_length = (size_t)(endptr - pDest->data);
    }
}

static void STRAPI CopyW(STRINGW *pstr, size_t len, const wchar_t *p) {
    if (pstr->data) {
        wchar_t *endptr = NULL;
        StringCchCopyNExW(pstr->data, pstr->alloc_length, p, len, &endptr, NULL, STR_CCH_FLAGS);
        pstr->data_length = (size_t)(endptr - pstr->data);
    }
}

static void STRAPI SetCopyW(STRINGW* pstr, size_t len, const wchar_t* p)
{
    AllocW(pstr, len);
    CopyW(pstr, len, p);
}

static wchar_t* STRAPI CopyOldDataW(STRINGW* pstr, size_t* outLen)
{
    size_t old_len = StrlenW(pstr->data);
    wchar_t* ptr = AllocBuffer(old_len + 1, sizeof(wchar_t), FALSE);
    if (ptr)
    {
        wchar_t *endptr = NULL;
        StringCchCopyNExW(ptr, old_len + 1, pstr->data, old_len, &endptr, NULL, STR_CCH_FLAGS);
        *outLen = (size_t)(endptr - pstr->data);
    }
    return ptr;
}

static void STRAPI FreeUnusedData(STRINGW *pstr)
{
    if ((pstr->data_length + 1) != pstr->alloc_length) {
        size_t old_len = 0;
        wchar_t *p = CopyOldDataW(pstr, &old_len);
        if (!p)
            return;

        FreeBufferW(pstr);
        pstr->data = p;
        pstr->alloc_length = old_len + 1;
        pstr->data_length = old_len;
    }
}

static void STRAPI CopyConcatW(STRINGW *pstr, size_t len1, const wchar_t *p1, size_t len2, const wchar_t *p2)
{
    size_t const new_len = len1 + len2;
    if (new_len != 0)
    {
        AllocW(pstr, new_len);
        StringCchCopyNW(pstr->data, pstr->alloc_length, p1, len1);
        StringCchCatNW(pstr->data, pstr->alloc_length, p2, len2);
        pstr->data_length = new_len;
    }
}

static void STRAPI ConcatW(STRINGW* pstr, size_t len, const wchar_t* p)
{
    if (len == 0)
        return;

    size_t const new_len = pstr->data_length + len;
    if (new_len >= pstr->alloc_length)
    {
        /* need to grow buffer */
        size_t old_len = 0;
        wchar_t *pOld = CopyOldDataW(pstr, &old_len);
        if (!pOld)
            return;

        FreeBufferW(pstr);
        CopyConcatW(pstr, old_len, pOld, len, p);
        FreeBuffer(pOld);
    }
    else
    {
        /* buffer is sufficiently long enough */
        StringCchCatNW(pstr->data, pstr->alloc_length, p, len);
        pstr->data_length = new_len;
    }
}


static void STRAPI FormatW(HSTRINGW hstr, const wchar_t* fmt, va_list args)
{
    STRINGW* pstr = ToWString(hstr);
    if (!pstr)
        return;

    va_list prev_list = args;

    size_t max_len = 0;
    const wchar_t * p;
    for (p = fmt; *p != L'\0'; p = _wcsinc(p)) {
        size_t item_len = 0, width = 0, prec = 0, modif = 0;
        if (*p != L'%' || *(p = _wcsinc(p)) == L'%') {
            ++max_len;
            continue;
        }
        item_len = 0;
        width = 0;
        for (; *p != L'\0'; p = _wcsinc(p)) {
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
            for (; *p != L'\0' && isdigit(*p); p = _wcsinc(p))
                ;
        }
        assert(width >= 0);

        prec = 0;
        if (*p == L'.') {
            p = _wcsinc(p);

            if (*p == L'*') {
                prec = va_arg(args, int);
                p = _wcsinc(p);
            } else {
                prec = _ttoi(p);
                for (; *p != L'\0' && isdigit(*p); p = _wcsinc(p))
                    ;
            }
            assert(prec >= 0);
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
            wchar_t *next_arg = va_arg(args, wchar_t *);
            if (next_arg == NULL)
                item_len = 6;
            else {
                item_len = wcslen(next_arg);
                item_len = max(1, item_len);
            }
        } break;

        case L's' | 0x20000:
        case L'S' | 0x20000: {
            wchar_t *next_arg = va_arg(args, wchar_t *);
            if (next_arg == NULL)
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
                assert(0); /* unknown format */
            }
        }
        max_len += item_len;
    }

    AllocW(pstr, max_len);

    wchar_t* endptr = NULL;
    StringCchVPrintfExW(pstr->data, pstr->alloc_length, &endptr, NULL, STR_CCH_FLAGS, fmt, prev_list);
    pstr->data_length = (size_t)(endptr - pstr->data);
    va_end(prev_list);
}


/**************************************************/
/*                                                */
/*              PUBLIC API                        */
/*                                                */
/**************************************************/

HSTRINGW STRAPI StrgCreate()
{
    STRINGW *pstr = AllocBuffer(sizeof(STRINGW), 1, TRUE);
    if (!pstr)
        return NULL;
    return (HSTRINGW)pstr;
}


//~void STRAPI StrgReserveBuffer(HSTRINGW hstr, size_t len) {
//~
//~    STRINGW* ptr = ToWString(hstr);
//~    if (!ptr)
//~        return;
//~    AllocW(ptr, len);
//~}


void STRAPI StrgDestroy(HSTRINGW hstr)
{
    STRINGW* pstr = ToWString(hstr);
    if (!pstr)
        return;
    FreeBufferW(pstr);
    FreeBuffer((wchar_t*)pstr);
}


int STRAPI StrgSet(HSTRINGW hstr, const wchar_t* str)
{
    STRINGW* pstr = ToWString(hstr);
    if (!pstr)
        return -1;
    SetCopyW(pstr, StrlenW(str), str);
    return  (pstr->data == NULL) ? 0 : 1; 
}


const wchar_t* STRAPI StrgGet(HSTRINGW hstr)
{
    STRINGW* pstr = ToWString(hstr);
    if (!pstr)
        return NULL;
    return pstr->data;
}


size_t STRAPI StrgGetLength(HSTRINGW hstr)
{
    STRINGW* pstr = ToWString(hstr);
    if (!pstr)
        return 0;
    return pstr->data_length;
}


size_t STRAPI StrgGetAllocLength(HSTRINGW hstr)
{
    STRINGW* pstr = ToWString(hstr);
    if (!pstr)
        return 0;
    return pstr->alloc_length;
}


int STRAPI StrgIsEmpty(HSTRINGW hstr) 
{ 
    return StrgGetLength(hstr) == 0;
}


void STRAPI StrgFreeExtra(HSTRINGW hstr)
{
    STRINGW* pstr = ToWString(hstr);
    if (!pstr)
        return;
    FreeUnusedData(pstr);
}


void STRAPI StrgEmpty(HSTRINGW hstr)
{
    STRINGW* pstr = ToWString(hstr);
    if (!pstr)
        return;
    FreeBufferW(pstr);
}


void STRAPI StrgSetAt(HSTRINGW hstr, size_t index, wchar_t ch)
{
    STRINGW* pstr = ToWString(hstr);
    if (!pstr)
        return;
    if (index >= pstr->data_length)
    {
        assert(0);/* buffer too small */
        return;
    }
    pstr->data[index] = ch;
}


wchar_t STRAPI StrgGetAt(HSTRINGW hstr, size_t index)
{
    STRINGW* pstr = ToWString(hstr);
    if (!pstr)
        return L'\0';
    if (index >= pstr->data_length)
    {
        assert(0);/* buffer too small */
        return L'\0';
    }
    return pstr->data[index];
}


void STRAPI StrgCat(HSTRINGW hstr, const wchar_t* str)
{
    STRINGW* pstr = ToWString(hstr);
    if (!pstr)
        return;
    ConcatW(pstr, StrlenW(str), str);
}


size_t STRAPI StrgInsert(HSTRINGW hstr, size_t index, const wchar_t* str)
{
    STRINGW* pstr = ToWString(hstr);
    if (!pstr)
        return STRINGW_INVALID_SIZE;

    size_t const ins_len = StrlenW(str);
    size_t new_len = pstr->data_length;

    if (ins_len > 0)
    {
        if (index > new_len)
            index = new_len;

        new_len += ins_len;

        if (pstr->alloc_length <= new_len)
        {
            size_t old_len = 0;
            wchar_t *pOld = CopyOldDataW(pstr, &old_len);
            FreeBufferW(pstr);
            SetCopyW(pstr, new_len, pOld);
            FreeBuffer(pOld);
        }
        memcpy(pstr->data + index + ins_len, pstr->data + index, (new_len - index - ins_len + 1) * sizeof(wchar_t));
        memcpy(pstr->data + index, str, ins_len * sizeof(wchar_t));
        pstr->data_length = new_len;
    }

    return new_len;
}


size_t STRAPI StrgInsertCh(HSTRINGW hstr, size_t index, wchar_t c)
{
    STRINGW *pstr = ToWString(hstr);
    if (!pstr)
        return 0;

    size_t const new_len = pstr->data_length + 1;
    if (index >= new_len)
        index = new_len - 1;

    if (pstr->alloc_length <= new_len)
    {
        size_t old_len = 0;
        wchar_t *pOld = CopyOldDataW(pstr, &old_len);
        FreeBufferW(pstr);
        SetCopyW(pstr, new_len, pOld);
        FreeBuffer(pOld);
    }
    memcpy(pstr->data + index + 1, pstr->data + index, (new_len - index) * sizeof(wchar_t));
    pstr->data[index] = c;
    pstr->data_length = new_len;
    return new_len;
}


size_t STRAPI StrgReplace(HSTRINGW hstr, const wchar_t* pOld, const wchar_t* pNew)
{
    STRINGW* pstr = ToWString(hstr);
    if (!pstr)
        return 0;

    size_t const src_len = StrlenW(pOld);
    if (src_len == 0)
        return 0;
    
    size_t const repl_len = StrlenW(pNew);
    wchar_t * start = pstr->data;
    wchar_t * end = pstr->data + pstr->data_length;
    wchar_t * target = NULL;
    
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

        if (pstr->alloc_length <= new_len)
        {
            wchar_t *pOldData = CopyOldDataW(pstr, &old_len);
            FreeBufferW(pstr);
            SetCopyW(pstr, new_len, pOld);
            FreeBuffer(pOldData);
        }
        start = pstr->data;
        end = pstr->data + pstr->data_length;

        while (start < end)
        {
            while((target = wcsstr(start, pOld)) != NULL)
            {
                size_t bal = old_len - (target - pstr->data + src_len);
                memmove(target + repl_len, target + src_len, bal * sizeof(wchar_t));
                memcpy(target, pNew, repl_len * sizeof(wchar_t));
                start = target + repl_len;
                start[bal] = L'\0';
                old_len += (repl_len - src_len);
            }
            start += wcslen(start) + 1;
        }
        pstr->data_length = new_len;
    }

    return count;
}


size_t STRAPI StrgRemove(HSTRINGW hstr, const wchar_t *str)
{
    return StrgReplace(hstr, str, L"");
}


size_t STRAPI StrgReplaceCh(HSTRINGW hstr, wchar_t chOld, wchar_t chNew)
{
    STRINGW* pstr = ToWString(hstr);
    if (!pstr)
        return 0;

    size_t count = 0;
    if (chOld != chNew)
    {
        wchar_t* p = pstr->data;
        wchar_t* end = p + pstr->data_length;
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


size_t STRAPI StrgRemoveCh(HSTRINGW hstr, wchar_t chRemove)
{
    STRINGW* pstr = ToWString(hstr);
    if (!pstr)
        return 0;

    wchar_t * source = pstr->data;
    wchar_t * dest = pstr->data;
    wchar_t * end = pstr->data + pstr->data_length;

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
    *dest = L'\0';
    count = (int)(ptrdiff_t)(source - dest);
    pstr->data_length -= count;

    return count;
}


size_t STRAPI StrgDelete(HSTRINGW hstr, size_t index, size_t count)
{
    STRINGW* pstr = ToWString(hstr);
    if (!pstr)
        return 0;

    size_t const new_len = pstr->data_length;
    if (count > 0 && index < new_len)
    {
        size_t copy = new_len - (index + count) + 1;
        memcpy(pstr->data + index, pstr->data + index + count, copy * sizeof(wchar_t));
        pstr->data_length = new_len - count;
    }

    return new_len;
}


void STRAPI StrgToUpper(HSTRINGW hstr)
{
    STRINGW* pstr = ToWString(hstr);
    if (!pstr)
        return;
    _wcsupr_s(pstr->data, pstr->data_length);
}


void STRAPI StrgToLower(HSTRINGW hstr)
{
    STRINGW* pstr = ToWString(hstr);
    if (!pstr)
        return;
    _wcslwr_s(pstr->data, pstr->data_length);
}


void STRAPI StrgReverse(HSTRINGW hstr)
{
    STRINGW* pstr = ToWString(hstr);
    if (!pstr)
        return;
    _wcsrev(pstr->data);
}


void STRAPI StrgTrimRight(HSTRINGW hstr)
{
    STRINGW* pstr = ToWString(hstr);
    if (!pstr)
        return;

    wchar_t * start = pstr->data;
    wchar_t * end = NULL;

    while (*start != L'\0')
    {
        if (isspace(*start))
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
        *end = L'\0';
        pstr->data_length = end - pstr->data;
    }
}


void STRAPI StrgTrimLeft(HSTRINGW hstr)
{
    STRINGW* pstr = ToWString(hstr);
    if (!pstr)
        return;

    wchar_t * start = pstr->data;

    while (isspace(*start))
        start++;

    if (start != pstr->data)
    {
        size_t data_length = pstr->data_length - (start - pstr->data);
        memmove(pstr->data, start, (data_length + 1) * sizeof(wchar_t));
        pstr->data_length = data_length;
    }
}


void STRAPI StrgTrim(HSTRINGW hstr)
{
    StrgTrimRight(hstr);
    StrgTrimLeft(hstr);
}


size_t STRAPI StrgFind(HSTRINGW hstr, const wchar_t* sub, size_t start)
{
    STRINGW* pstr = ToWString(hstr);
    if (!pstr)
        return STRINGW_INVALID_SIZE;

    if (start > pstr->data_length)
        return STRINGW_INVALID_SIZE;

    wchar_t * str = wcsstr(pstr->data + start, sub);

    return (str == NULL) ? STRINGW_INVALID_SIZE : (size_t)(str - pstr->data);
}


size_t STRAPI StrgFindCh(HSTRINGW hstr, wchar_t ch, size_t start)
{
    STRINGW* pstr = ToWString(hstr);
    if (!pstr)
        return STRINGW_INVALID_SIZE;

    if (start >= pstr->data_length)
        return STRINGW_INVALID_SIZE;

    wchar_t * p = wcschr(pstr->data + start, ch);

    return (p == NULL) ? STRINGW_INVALID_SIZE : (size_t)(p - pstr->data);
}


size_t STRAPI StrgReverseFind(HSTRINGW hstr, wchar_t ch)
{
    STRINGW* pstr = ToWString(hstr);
    if (!pstr)
        return STRINGW_INVALID_SIZE;

    wchar_t * p = wcsrchr(pstr->data, ch);

    return (p == NULL) ? STRINGW_INVALID_SIZE : (size_t)(p - pstr->data);
}


size_t STRAPI StrgFindOneOf(HSTRINGW hstr, const wchar_t* char_set)
{
    STRINGW* pstr = ToWString(hstr);
    if (!pstr)
        return STRINGW_INVALID_SIZE;

    wchar_t * p = wcspbrk(pstr->data, char_set);
    return (p == NULL) ? STRINGW_INVALID_SIZE : (size_t)(p - pstr->data);
}


HSTRINGW STRAPI StrgCopy(HSTRINGW hstr)
{
    STRINGW* pstr = ToWString(hstr);
    if (!pstr)
        return NULL;

    HSTRINGW hCopy = StrgCreate();
    STRINGW* pCopy = ToWString(hCopy);

    SetCopyW(pCopy, pstr->data_length, pstr->data);
    return hCopy;
}


HSTRINGW STRAPI StrgMid(HSTRINGW hstr, size_t start, size_t count)
{
    STRINGW* pstr = ToWString(hstr);
    if (!pstr)
        return NULL;

    if (start + count > pstr->data_length)
        count = pstr->data_length - start;
    if (start > pstr->data_length)
        count = 0;

    assert(start + count <= pstr->data_length);

    HSTRINGW hCopy = StrgCreate();
    STRINGW* pCopy = ToWString(hCopy);

    if (start == 0 && start + count == pstr->data_length) {
        SetCopyW(pCopy, pstr->data_length, pstr->data);
    } 
    else {
        AllocCopyW(pstr, pCopy, count, start, 0);
    }
    return hCopy;
}


HSTRINGW STRAPI StrgLeft(HSTRINGW hstr, size_t count)
{
    STRINGW* pstr = ToWString(hstr);
    if (!pstr)
        return NULL;

    HSTRINGW hCopy = StrgCreate();
    STRINGW* pCopy = ToWString(hCopy);

    if (count >= pstr->data_length) {
        SetCopyW(pCopy, pstr->data_length, pstr->data);
    } else {
        AllocCopyW(pstr, pCopy, count, 0, 0);
    }
    return hCopy;
}


HSTRINGW STRAPI StrgRight(HSTRINGW hstr, size_t count)
{
    STRINGW* pstr = ToWString(hstr);
    if (!pstr)
        return NULL;

    HSTRINGW hCopy = StrgCreate();
    STRINGW* pCopy = ToWString(hCopy);

    if (count >= pstr->data_length) {
        SetCopyW(pCopy, pstr->data_length, pstr->data);
    } else {
        AllocCopyW(pstr, pCopy, count, pstr->data_length - count, 0);
    }
    return hCopy;
}


void STRAPI StrgFormat(HSTRINGW hstr, const wchar_t* fmt, ...)
{
    STRINGW *pstr = ToWString(hstr);
    if (!pstr)
        return;
    va_list args;
    va_start(args, fmt);
    FormatW(hstr, fmt, args);
    va_end(args);
}
