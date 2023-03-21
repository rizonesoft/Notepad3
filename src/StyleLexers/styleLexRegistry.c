#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_Registry = EMPTY_KEYWORDLIST;

EDITLEXER lexRegistry =
{
    SCLEX_REGISTRY, "registry", IDS_LEX_REG_FILES, L"Registry Files", L"reg", L"",
    &KeyWords_Registry, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_REG_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {SCE_REG_COMMENT}, IDS_LEX_STR_Comment, L"Comment", L"fore:#008800", L"" },
        { {SCE_REG_VALUENAME}, IDS_LEX_STR_63285, L"Value Name", L"", L"" },
        { {SCE_REG_STRING}, IDS_LEX_STR_String, L"String", L"fore:#99004D", L"" },
        { {SCE_REG_VALUETYPE}, IDS_LEX_STR_63286, L"Value Type", L"bold; fore:#00007F", L"" },
        { {SCE_REG_HEXDIGIT}, IDS_LEX_STR_Hex, L"Hex", L"fore:#7F0B0C", L"" },
        { {SCE_REG_ADDEDKEY}, IDS_LEX_STR_AddKey, L"Added Key", L"bold; fore:#000000; back:#FF8040; eolfilled", L"" }, //fore:#530155
        { {SCE_REG_DELETEDKEY}, IDS_LEX_STR_DelKey, L"Deleted Key", L"fore:#FF0000", L"" },
        { {SCE_REG_ESCAPED}, IDS_LEX_STR_Esc, L"Escaped", L"bold; fore:#7D8187", L"" },
        { {SCE_REG_STRING_GUID}, IDS_LEX_STR_GUIDStrg, L"GUID String", L"fore:#C58D25", L"" },
        { {SCE_REG_KEYPATH_GUID}, IDS_LEX_STR_63291, L"GUID in Key Path", L"fore:#009F9F", L"" },
        { {SCE_REG_PARAMETER}, IDS_LEX_STR_Param, L"Parameter", L"fore:#0B6561", L"" },
        { {SCE_REG_OPERATOR}, IDS_LEX_STR_Operator, L"Operator", L"bold", L"" },
        EDITLEXER_SENTINEL
    }
};

