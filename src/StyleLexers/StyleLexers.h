#ifndef _STYLE_LEXERS_H_
#define _STYLE_LEXERS_H_

// ----------------------------------------------------------------------------

#include <assert.h>

#include "resource.h"
#include "Scintilla.h"
#include "lexers_x/SciXLexer.h"
#include "EditLexer.h"

// ----------------------------------------------------------------------------


#define MULTI_STYLE(a,b,c,d) ((a)|(b<<8)|(c<<16)|(d<<24))

#define EMPTY_KEYWORDLIST { NULL, }

#define EDITLEXER_SENTINEL { {(-1)}, (00000), L"", L"", L"" }

// ----------------------------------------------------------------------------

#define LEX_FUNCTION_BODY(type, value)                      \
                                                            \
  static __int64 iStyleChanged = 0LL;                       \
                                                            \
  assert(((value) > -63) && ((value) < 63));                \
                                                            \
  switch (type) {                                           \
  case FCT_SETTING_CHANGE:                                  \
    if ((value) == 0) {                                     \
      return iStyleChanged;                                 \
    }                                                       \
    else if ((value) > 0) {                                 \
      iStyleChanged |= (((__int64)1) << (value));           \
    }                                                       \
    else {  /* (value) < 0 */                               \
      iStyleChanged &= ~(((__int64)1) << (0 - (value)));    \
    }                                                       \
    break;                                                  \
                                                            \
  default:                                                  \
    break;                                                  \
  }                                                         \

// ----------------------------------------------------------------------------

// clamp
inline int clampi(int x, int lower, int upper)
{
    return (x < lower) ? lower : ((x > upper) ? upper : x);
}

// ----------------------------------------------------------------------------

#endif // _STYLE_LEXERS_H_
