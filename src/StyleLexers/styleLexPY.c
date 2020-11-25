#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_PY =
{
    "False None True and as assert break class continue def del elif else except exec finally for from global if "
    "import in is lambda nonlocal not or pass print raise return try while with yield",
    NULL,
};


EDITLEXER lexPY =
{
    SCLEX_PYTHON, "python", IDS_LEX_PYTHON, L"Python Script", L"py; pyw; pyx; pxd; pxi; boo; empy; cobra; gs", L"",
    &KeyWords_PY, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
        //{ {SCE_P_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
        { {MULTI_STYLE(SCE_P_COMMENTLINE,SCE_P_COMMENTBLOCK,0,0)}, IDS_LEX_STR_63127, L"Comment", L"fore:#880000", L"" },
        { {SCE_P_WORD}, IDS_LEX_STR_63128, L"Keyword", L"fore:#000088", L"" },
        { {SCE_P_IDENTIFIER}, IDS_LEX_STR_63129, L"Identifier", L"", L"" },
        { {MULTI_STYLE(SCE_P_STRING,SCE_P_STRINGEOL,0,0)}, IDS_LEX_STR_63211, L"String Double Quoted", L"fore:#008800", L"" },
        { {SCE_P_CHARACTER}, IDS_LEX_STR_63212, L"String Single Quoted", L"fore:#008800", L"" },
        { {SCE_P_TRIPLEDOUBLE}, IDS_LEX_STR_63244, L"String Triple Double Quotes", L"fore:#008800", L"" },
        { {SCE_P_TRIPLE}, IDS_LEX_STR_63245, L"String Triple Single Quotes", L"fore:#008800", L"" },
        { {SCE_P_NUMBER}, IDS_LEX_STR_63130, L"Number", L"fore:#FF4000", L"" },
        { {SCE_P_OPERATOR}, IDS_LEX_STR_63132, L"Operator", L"bold; fore:#666600", L"" },
        { {SCE_P_DEFNAME}, IDS_LEX_STR_63247, L"Function Name", L"fore:#660066", L"" },
        { {SCE_P_CLASSNAME}, IDS_LEX_STR_63246, L"Class Name", L"fore:#660066", L"" },
        EDITLEXER_SENTINEL
    }
};
