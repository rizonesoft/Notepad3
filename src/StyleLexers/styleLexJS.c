#include "StyleLexers.h"

// ----------------------------------------------------------------------------

// JavaScript keywords (sync with lexHTML::KeyWords_HTML for embedded JS)
KEYWORDLIST KeyWords_JS =
{
    NP3_LEXER_JS_KEYWORD_LIST,
    NULL,
};


EDITLEXER lexJS =
{
    SCLEX_CPP, "cpp", IDS_LEX_J_SCR, L"JavaScript", L"js; jse; jsm; as; mjs; qs", L"",
    &KeyWords_JS, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
        //{ {SCE_C_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
        { {SCE_C_COMMENT}, IDS_LEX_STR_63127, L"Comment", L"fore:#646464", L"" },
        { {SCE_C_WORD}, IDS_LEX_STR_63128, L"Keyword", L"bold; fore:#A46000", L"" },
        { {SCE_C_IDENTIFIER}, IDS_LEX_STR_63129, L"Identifier", L"", L"" },
        { {MULTI_STYLE(SCE_C_STRING,SCE_C_CHARACTER,SCE_C_STRINGEOL,SCE_C_VERBATIM)}, IDS_LEX_STR_63131, L"String", L"fore:#008000", L"" },
        { {SCE_C_REGEX}, IDS_LEX_STR_63135, L"Regex", L"fore:#006633; back:#FFF1A8", L"" },
        { {SCE_C_NUMBER}, IDS_LEX_STR_63130, L"Number", L"fore:#FF0000", L"" },
        { {SCE_C_OPERATOR}, IDS_LEX_STR_63132, L"Operator", L"fore:#B000B0", L"" },
        //{ {SCE_C_UUID}, L"UUID", L"", L"" },
        //{ {SCE_C_PREPROCESSOR}, L"Preprocessor", L"fore:#FF8000", L"" },
        //{ {SCE_C_WORD}, L"Word 2", L"", L"" },
        //{ {SCE_C_GLOBALCLASS}, L"Global Class", L"", L"" },
        EDITLEXER_SENTINEL
    }
};

