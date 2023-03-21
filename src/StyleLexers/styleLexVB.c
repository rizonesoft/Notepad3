#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_VB =
{
    NP3_LEXER_VB_KEYWORD_LIST,
    NULL,
};


EDITLEXER lexVB =
{
    SCLEX_VB, "vb", IDS_LEX_VIS_BAS, L"Visual Basic", L"vb; bas; frm; cls; ctl; pag; dsr; dob", L"",
    &KeyWords_VB, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_B_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {SCE_B_COMMENT}, IDS_LEX_STR_Comment, L"Comment", L"fore:#808080", L"" },
        { {SCE_B_KEYWORD}, IDS_LEX_STR_Keyword, L"Keyword", L"bold; fore:#B000B0", L"" },
        { {SCE_B_IDENTIFIER}, IDS_LEX_STR_Identifier, L"Identifier", L"", L"" },
        { {MULTI_STYLE(SCE_B_STRING,SCE_B_STRINGEOL,0,0)}, IDS_LEX_STR_String, L"String", L"fore:#008000", L"" },
        { {MULTI_STYLE(SCE_B_NUMBER,SCE_B_DATE,0,0)}, IDS_LEX_STR_Number, L"Number", L"fore:#FF0000", L"" },
        { {SCE_B_OPERATOR}, IDS_LEX_STR_Operator, L"Operator", L"", L"" },
        { {SCE_B_PREPROCESSOR}, IDS_LEX_STR_PreProc, L"Preprocessor", L"fore:#FF9C00", L"" },
        //{ {SCE_B_CONSTANT}, L"Constant", L"", L"" },
        //{ {SCE_B_KEYWORD2}, L"Keyword 2", L"", L"" },
        //{ {SCE_B_KEYWORD3}, L"Keyword 3", L"", L"" },
        //{ {SCE_B_KEYWORD4}, L"Keyword 4", L"", L"" },
        //{ {SCE_B_ASM}, L"Inline Asm", L"fore:#FF8000", L"" },
        EDITLEXER_SENTINEL
    }
};
