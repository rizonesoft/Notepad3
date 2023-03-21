#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_MAK = EMPTY_KEYWORDLIST;

EDITLEXER lexMAK =
{
    SCLEX_MAKEFILE, "makefile", IDS_LEX_MAKEFILES, L"Makefiles", L"mak; make; mk; dsp; msc; msvc; am; pro; pri; gmk; ninja; dsw; \\^Makefile$; \\^Kbuild$", L"",
    &KeyWords_MAK, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_MAKE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {SCE_MAKE_COMMENT}, IDS_LEX_STR_Comment, L"Comment", L"fore:#008000", L"" },
        { {MULTI_STYLE(SCE_MAKE_IDENTIFIER,SCE_MAKE_IDEOL,0,0)}, IDS_LEX_STR_Identifier, L"Identifier", L"fore:#003CE6", L"" },
        { {SCE_MAKE_OPERATOR}, IDS_LEX_STR_Operator, L"Operator", L"", L"" },
        { {SCE_MAKE_TARGET}, IDS_LEX_STR_Target, L"Target", L"fore:#003CE6; back:#FFC000", L"" },
        { {SCE_MAKE_PREPROCESSOR}, IDS_LEX_STR_PreProc, L"Preprocessor", L"fore:#FF8000", L"" },
        EDITLEXER_SENTINEL
    }
};
