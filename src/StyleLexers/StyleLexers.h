#ifndef _STYLE_LEXERS_H_
#define _STYLE_LEXERS_H_

// ----------------------------------------------------------------------------

#include <stdbool.h>

#include "Scintilla.h"
#include "SciLexer.h"
#include "../sciXlexers/SciXLexer.h"

#include "resource.h"

#include "EditLexer.h"

// ----------------------------------------------------------------------------


#define MULTI_STYLE(a,b,c,d) ((a)|(b<<8)|(c<<16)|(d<<24))

#define EMPTY_KEYWORDLIST { "", "", "", "", "", "", "", "", "" }

#define EDITLEXER_SENTINEL { -1, 00000, L"", L"", L"" }


// clamp
inline int clampi(int x, int lower, int upper) { return (x < lower) ? lower : ((x > upper) ? upper : x); }

// ----------------------------------------------------------------------------

#endif // _STYLE_LEXERS_H_
