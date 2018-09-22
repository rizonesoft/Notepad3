#ifndef _STYLE_LEXERS_H_
#define _STYLE_LEXERS_H_

// ----------------------------------------------------------------------------

#include "Scintilla.h"
#include "SciLexer.h"
#include "../sciXlexers/SciXLexer.h"

#include "resource.h"

#include "EditLexer.h"

// ----------------------------------------------------------------------------


#define MULTI_STYLE(a,b,c,d) ((a)|(b<<8)|(c<<16)|(d<<24))
#define EDITLEXER_SENTINEL { -1, 00000, L"", L"", L"" }

// ----------------------------------------------------------------------------

#endif // _STYLE_LEXERS_H_
