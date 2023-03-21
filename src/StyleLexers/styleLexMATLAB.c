#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_MATLAB =
{
    "break case catch continue else elseif end for function global if otherwise persistent return "
    "switch try while",
    NULL,
};


EDITLEXER lexMATLAB =
{
    SCLEX_MATLAB, "matlab", IDS_LEX_MATLAB, L"MATLAB", L"matlab; m; sce; sci", L"",
    &KeyWords_MATLAB, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_MATLAB_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {SCE_MATLAB_COMMENT}, IDS_LEX_STR_Comment, L"Comment", L"fore:#008000", L"" },
        { {SCE_MATLAB_COMMAND}, IDS_LEX_STR_Cmd, L"Command", L"bold", L"" },
        { {SCE_MATLAB_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#FF8000", L"" },
        { {SCE_MATLAB_KEYWORD}, IDS_LEX_STR_Keyword, L"Keyword", L"bold; fore:#00007F", L"" },
        { {MULTI_STYLE(SCE_MATLAB_STRING,SCE_MATLAB_DOUBLEQUOTESTRING,0,0)}, IDS_LEX_STR_String, L"String", L"fore:#7F007F", L"" },
        { {SCE_MATLAB_OPERATOR}, IDS_LEX_STR_Operator, L"Operator", L"", L"" },
        { {SCE_MATLAB_IDENTIFIER}, IDS_LEX_STR_Identifier, L"Identifier", L"", L"" },
        EDITLEXER_SENTINEL
    }
};
