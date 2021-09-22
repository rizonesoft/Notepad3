/*
Copyright (c) 2008-2021, Troy D. Hanson   http://troydhanson.github.com/uthash/
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* a dynamic string implementation using macros
 */
#ifndef UTSTRING_H
#define UTSTRING_H

#define UTSTRING_VERSION 2.3.0

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stringapiset.h>

#ifdef __GNUC__
#define UTSTRING_UNUSED __attribute__((__unused__))
#else
#define UTSTRING_UNUSED inline
#endif

#ifdef oom
#error "The name of macro 'oom' has been changed to 'utstring_oom'. Please update your code."
#define utstring_oom() oom()
#endif

#ifndef utstring_oom
#define utstring_oom() exit(-1)
#endif

typedef struct {
    char *d;  /* pointer to allocated buffer */
    size_t n; /* allocated capacity */
    size_t i; /* index of first unused byte */
} UT_string;

#define utstring_reserve(s,amt)                            \
do {                                                       \
  if (((s)->n - (s)->i) < (size_t)(amt)) {                 \
    char *utstring_tmp = (char*)realloc(                   \
      (s)->d, (s)->n + (amt));                             \
    if (!utstring_tmp) {                                   \
      utstring_oom();                                      \
    }                                                      \
    (s)->d = utstring_tmp;                                 \
    (s)->n += (amt);                                       \
  }                                                        \
} while(0)

#define utstring_sanitize(s)                               \
do {                                                       \
  (s)->i = strlen((s)->d);                                 \
} while(0)

#define utstring_done(s)                                   \
do {                                                       \
  if ((s)->d != NULL) free((s)->d);                        \
  (s)->d = NULL; (s)->n = 0; (s)->i = 0;                   \
} while(0)

#define utstring_init(s)                                   \
do {                                                       \
  utstring_done(s);                                        \
  utstring_reserve(s,128);                                 \
  (s)->d[0] = '\0';                                        \
} while(0)

#define utstring_free(s)                                   \
do {                                                       \
  utstring_done(s);                                        \
  free(s);                                                 \
} while(0)

#define utstring_new(s)                                    \
do {                                                       \
  (s) = (UT_string*)malloc(sizeof(UT_string));             \
  if (!(s)) {                                              \
    utstring_oom();                                        \
  }                                                        \
  (s)->d = NULL;                                           \
  utstring_init(s);                                        \
} while(0)

#define utstring_clear(s)                                  \
do {                                                       \
  (s)->d[0] = '\0';                                        \
  (s)->i = 0;                                              \
} while(0)

#define utstring_renew(s)                                  \
do {                                                       \
   if (s) {                                                \
     utstring_clear(s);                                    \
   } else {                                                \
     utstring_new(s);                                      \
   }                                                       \
} while(0)

#define utstring_bincpy(s,b,l)                             \
do {                                                       \
  utstring_reserve((s),(l)+1);                             \
  if (l) memcpy(&((s)->d[(s)->i]), b, l);                  \
  (s)->i += (l);                                           \
  (s)->d[(s)->i]='\0';                                     \
} while(0)

#define utstring_concat(dst,src)                                   \
do {                                                               \
  utstring_reserve((dst),((src)->i)+1);                            \
  if ((src)->i) memcpy(&((dst)->d[(dst)->i]), (src)->d, (src)->i); \
  (dst)->i += (src)->i;                                            \
  (dst)->d[(dst)->i]='\0';                                         \
} while(0)

#define utstring_is_empty(s) (!((s)->d) || ((s)->d[0] == '\0'))

#define utstring_len(s) ((s)->i)

#define utstring_body(s) ((s)->d)

#define utstring_alloc_len(s) ((s)->n)


UTSTRING_UNUSED static void utstring_printf_va(UT_string *s, const char *fmt, va_list ap) {
   ptrdiff_t n;
   va_list cp;
   for (;;) {
#ifdef _WIN32
      cp = ap;
#else
      va_copy(cp, ap);
#endif
      n = (ptrdiff_t)vsnprintf(&s->d[s->i], s->n - s->i, fmt, cp);
      va_end(cp);

      if ((n > -1) && (n < (ptrdiff_t)(s->n - s->i))) {
        s->i += n;
        return;
      }

      /* Else try again with more space. */
      if (n > -1) utstring_reserve(s,n+1); /* exact */
      else utstring_reserve(s,(s->n)*2);   /* 2x */
   }
}
#ifdef __GNUC__
/* support printf format checking (2=the format string, 3=start of varargs) */
static void utstring_printf(UT_string *s, const char *fmt, ...)
  __attribute__ (( format( printf, 2, 3) ));
#endif
UTSTRING_UNUSED static void utstring_printf(UT_string *s, const char *fmt, ...) {
   va_list ap;
   va_start(ap,fmt);
   utstring_printf_va(s,fmt,ap);
   va_end(ap);
}

/*******************************************************************************
 * begin substring search functions                                            *
 ******************************************************************************/
/* Build KMP table from left to right. */
UTSTRING_UNUSED static void _utstring_BuildTable(
    const char *P_Needle,
    size_t P_NeedleLen,
    ptrdiff_t*  P_KMP_Table)
{
    ptrdiff_t i, j;

    i = 0;
    j = i - 1;
    P_KMP_Table[i] = j;
    while (i < (ptrdiff_t)P_NeedleLen)
    {
        while ( (j > -1) && (P_Needle[i] != P_Needle[j]) )
        {
           j = P_KMP_Table[j];
        }
        i++;
        j++;
        if (i < (ptrdiff_t)P_NeedleLen)
        {
            if (P_Needle[i] == P_Needle[j])
            {
                P_KMP_Table[i] = P_KMP_Table[j];
            }
            else
            {
                P_KMP_Table[i] = j;
            }
        }
        else
        {
            P_KMP_Table[i] = j;
        }
    }

    return;
}


/* Build KMP table from right to left. */
UTSTRING_UNUSED static void _utstring_BuildTableR(
    const char *P_Needle,
    size_t P_NeedleLen,
    ptrdiff_t*  P_KMP_Table)
{
    ptrdiff_t i, j;

    i = P_NeedleLen - 1;
    j = i + 1;
    P_KMP_Table[i + 1] = j;
    while (i >= 0)
    {
        while ((j < (ptrdiff_t)P_NeedleLen) && (P_Needle[i] != P_Needle[j]))
        {
           j = P_KMP_Table[j + 1];
        }
        i--;
        j--;
        if (i >= 0)
        {
            if (P_Needle[i] == P_Needle[j])
            {
                P_KMP_Table[i + 1] = P_KMP_Table[j + 1];
            }
            else
            {
                P_KMP_Table[i + 1] = j;
            }
        }
        else
        {
            P_KMP_Table[i + 1] = j;
        }
    }

    return;
}


/* Search data from left to right. ( Multiple search mode. ) */
UTSTRING_UNUSED static ptrdiff_t _utstring_find(
    const char *P_Haystack,
    size_t P_HaystackLen,
    const char *P_Needle,
    size_t P_NeedleLen,
    ptrdiff_t*  P_KMP_Table)
{
    ptrdiff_t i, j;
    ptrdiff_t V_FindPosition = -1;

    /* Search from left to right. */
    i = j = 0;
    while ((j < (ptrdiff_t)P_HaystackLen) && (((P_HaystackLen - j) + i) >= P_NeedleLen))
    {
        while ( (i > -1) && (P_Needle[i] != P_Haystack[j]) )
        {
            i = P_KMP_Table[i];
        }
        i++;
        j++;
        if (i >= (ptrdiff_t)P_NeedleLen)
        {
            /* Found. */
            V_FindPosition = j - i;
            break;
        }
    }

    return V_FindPosition;
}


/* Search data from right to left. ( Multiple search mode. ) */
UTSTRING_UNUSED static ptrdiff_t _utstring_findR(
    const char *P_Haystack,
    size_t P_HaystackLen,
    const char *P_Needle,
    size_t P_NeedleLen,
    ptrdiff_t*  P_KMP_Table)
{
    ptrdiff_t i, j;
    ptrdiff_t V_FindPosition = -1;

    /* Search from right to left. */
    j = (P_HaystackLen - 1);
    i = (P_NeedleLen - 1);
    while ( (j >= 0) && (j >= i) )
    {
        while ((i < (ptrdiff_t)P_NeedleLen) && (P_Needle[i] != P_Haystack[j]))
        {
            i = P_KMP_Table[i + 1];
        }
        i--;
        j--;
        if (i < 0)
        {
            /* Found. */
            V_FindPosition = j + 1;
            break;
        }
    }

    return V_FindPosition;
}


/* Search data from left to right. ( One time search mode. ) */
UTSTRING_UNUSED static ptrdiff_t utstring_find(
    UT_string *s,
    ptrdiff_t P_StartPosition, /* Start from 0. -1 means last position. */
    const char *P_Needle,
    size_t P_NeedleLen)
{
    ptrdiff_t V_StartPosition;
    ptrdiff_t V_HaystackLen;
    ptrdiff_t* V_KMP_Table;
    ptrdiff_t V_FindPosition = -1;

    if (P_StartPosition < 0)
    {
        V_StartPosition = s->i + P_StartPosition;
    }
    else
    {
        V_StartPosition = P_StartPosition;
    }
    V_HaystackLen = s->i - V_StartPosition;
    if ((V_HaystackLen >= (ptrdiff_t)P_NeedleLen) && (P_NeedleLen > 0))
    {
        V_KMP_Table = (ptrdiff_t*)malloc(sizeof(ptrdiff_t) * (P_NeedleLen + 1));
        if (V_KMP_Table != NULL)
        {
            _utstring_BuildTable(P_Needle, P_NeedleLen, V_KMP_Table);

            V_FindPosition = _utstring_find(s->d + V_StartPosition,
                                            V_HaystackLen,
                                            P_Needle,
                                            P_NeedleLen,
                                            V_KMP_Table);
            if (V_FindPosition >= 0)
            {
                V_FindPosition += V_StartPosition;
            }

            free(V_KMP_Table);
        }
    }

    return V_FindPosition;
}


/* Search data from right to left. ( One time search mode. ) */
UTSTRING_UNUSED static ptrdiff_t utstring_findR(
    UT_string *s,
    ptrdiff_t P_StartPosition, /* Start from 0. -1 means last position. */
    const char *P_Needle,
    size_t P_NeedleLen)
{
    ptrdiff_t V_StartPosition;
    ptrdiff_t V_HaystackLen;
    ptrdiff_t* V_KMP_Table;
    ptrdiff_t  V_FindPosition = -1;

    if (P_StartPosition < 0)
    {
        V_StartPosition = s->i + P_StartPosition;
    }
    else
    {
        V_StartPosition = P_StartPosition;
    }
    V_HaystackLen = V_StartPosition + 1;
    if ((V_HaystackLen >= (ptrdiff_t)P_NeedleLen) && (P_NeedleLen > 0))
    {
        V_KMP_Table = (ptrdiff_t*)malloc(sizeof(ptrdiff_t) * (P_NeedleLen + 1));
        if (V_KMP_Table != NULL)
        {
            _utstring_BuildTableR(P_Needle, P_NeedleLen, V_KMP_Table);

            V_FindPosition = _utstring_findR(s->d,
                                             V_HaystackLen,
                                             P_Needle,
                                             P_NeedleLen,
                                             V_KMP_Table);

            free(V_KMP_Table);
        }
    }

    return V_FindPosition;
}
/*******************************************************************************
 * end substring search functions                                              *
 ******************************************************************************/


UTSTRING_UNUSED static void utstring_setw(UT_string* s, const wchar_t* wch)
{
    int const len = WideCharToMultiByte(CP_UTF8, 0, wch, -1, NULL, 0, NULL, NULL);
    utstring_clear(s);
    if ((s)->n < len) { utstring_reserve(s, len - (s)->n); }
    WideCharToMultiByte(CP_UTF8, 0, wch, -1, (s)->d, len, NULL, NULL);
    (s)->i = strlen((s)->d);
}


UTSTRING_UNUSED static size_t utstring_getw(UT_string* s, wchar_t* wch, size_t count)
{
    return (size_t) MultiByteToWideChar(CP_UTF8, 0, utstring_body(s), -1, wch, (int)count);
}


#endif /* UTSTRING_H */
