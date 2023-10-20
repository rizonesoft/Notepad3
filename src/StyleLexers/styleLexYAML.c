#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_YAML = { "false n no off on true y yes",
                              NULL,
                            };

EDITLEXER lexYAML =
{
    SCLEX_YAML, "yaml", IDS_LEX_YAML, L"YAML", L"yaml; yml", L"",
    &KeyWords_YAML, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_YAML_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {SCE_YAML_COMMENT}, IDS_LEX_STR_Comment, L"Comment", L"fore:#008800", L"" },
        { {SCE_YAML_IDENTIFIER}, IDS_LEX_STR_Identifier, L"Identifier", L"bold; fore:#0A246A", L"" },
        { {SCE_YAML_KEYWORD}, IDS_LEX_STR_Keyword, L"Keyword", L"fore:#880088", L"" },
        { {SCE_YAML_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#FF8000", L"" },
        { {SCE_YAML_REFERENCE}, IDS_LEX_STR_Ref, L"Reference", L"fore:#008888", L"" },
        { {SCE_YAML_DOCUMENT}, IDS_LEX_STR_Doc, L"Document", L"bold; fore:#FFFFFF; back:#000088; eolfilled", L"" },
        { {SCE_YAML_TEXT}, IDS_LEX_STR_Txt, L"Text", L"fore:#404040", L"" },
        { {SCE_YAML_ERROR}, IDS_LEX_STR_Error, L"Error", L"bold; italic; fore:#FFFFFF; back:#FF0000; eolfilled", L"" },
        { {SCE_YAML_OPERATOR}, IDS_LEX_STR_Operator, L"Operator", L"fore:#333366", L"" },
        EDITLEXER_SENTINEL
    }
};
