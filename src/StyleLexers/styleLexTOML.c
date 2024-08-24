#include "StyleLexers.h"

// ----------------------------------------------------------------------------

//KEYWORDLIST KeyWords_TOML = EMPTY_KEYWORDLIST;
KEYWORDLIST KeyWords_TOML =
{
    "+inf +nan -inf -nan false inf nan true", // Keyword
    NULL,
};


EDITLEXER lexTOML =
{
    SCLEX_TOML, "TOML", IDS_LEX_TOML_CFG, L"TOML Config", L"toml", L"",
    &KeyWords_TOML,{
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_TOML_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {SCE_TOML_IDENTIFIER}, IDS_LEX_STR_Identifier, L"Identifier", L"", L"" },
        { {SCE_TOML_KEYWORD}, IDS_LEX_STR_Keyword, L"Keyword", L"bold; fore:#FF0080", L"" },
        { {SCE_TOML_COMMENT}, IDS_LEX_STR_Comment, L"Comment", L"fore:#008000", L"" },
        { {SCE_TOML_KEY}, IDS_LEX_STR_Key, L"Key", L"bold; fore:#5E608F", L"" },
        { {SCE_TOML_OPERATOR}, IDS_LEX_STR_Operator, L"Operator", L"fore:#B000B0", L"" },
        { {SCE_TOML_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#0000E0", L"" },
        { {SCE_TOML_DATETIME }, IDS_LEX_STR_DateTime, L"Date-Time", L"fore:#950095", L"" },
        { {SCE_TOML_TABLE}, IDS_LEX_STR_Table, L"Table", L"bold; fore:#FF8000", L"" },
        { {MULTI_STYLE(SCE_TOML_STRING_SQ, SCE_TOML_TRIPLE_STRING_SQ,0,0)}, IDS_LEX_STR_63212, L"String Single Quoted", L"italic; fore:#606060", L"" },
        { {MULTI_STYLE(SCE_TOML_STRING_DQ, SCE_TOML_TRIPLE_STRING_DQ,0,0)}, IDS_LEX_STR_63211, L"String Double Quoted", L"italic; fore:#606060", L"" },
        { {SCE_TOML_ESCAPECHAR}, IDS_LEX_STR_Esc, L"Escaped", L"fore:#202020", L"" },
        { {SCE_TOML_ERROR}, IDS_LEX_STR_63252, L"Parsing Error", L"fore:#FFFF00; back:#A00000; eolfilled", L"" },
        EDITLEXER_SENTINEL
    }
};
