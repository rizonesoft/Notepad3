#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_Go =
{
// Primary keywords and identifiers
    "break case chan const continue default defer else fallthrough for func go goto if import interface map "
    "package range return select struct switch type var",
// Secondary keywords and identifiers
    "false nil true",
// Documentation comment keywords  (doxygen)
    "",
// Type definitions and aliases
    "bool byte float float32 float64 int int16 int32 int64 int8 string uint uint16 uint32 uint64 uint8 uintptr",
// Keywords 5
    "",
// Keywords 6
    "",
// Keywords 7
    "",
// ---
    NULL,
};


EDITLEXER lexGo =
{
    SCLEX_D, "d", IDS_LEX_GO_SRC, L"Go Source Code", L"go", L"",
    &KeyWords_Go,{
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_D_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {MULTI_STYLE(SCE_D_COMMENT,SCE_D_COMMENTLINE,SCE_D_COMMENTNESTED,0)}, IDS_LEX_STR_Comment, L"Comment", L"fore:#008000", L"" },
        //{ {SCE_D_COMMENTDOC}, IDS_LEX_STR_63259, L"Comment Doc", L"fore:#040A0", L"" },
        { {SCE_D_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#FF0000", L"" },
        { {SCE_D_WORD}, IDS_LEX_STR_Keyword, L"Keyword", L"bold; fore:#0A246A", L"" },
        { {SCE_D_WORD}, IDS_LEX_STR_Keyword2nd, L"Keyword 2nd", L"bold; fore:#0A246A", L"" },
        //{ {SCE_D_WORD}, IDS_LEX_STR_Keyword, L"Keyword 3", L"bold; fore:#0A246A", L"" },
        //{ {SCE_D_WORD}, IDS_LEX_STR_Keyword, L"Keyword 5", L"bold; fore:#0A246A", L"" },
        //{ {SCE_D_WORD}, IDS_LEX_STR_Keyword, L"Keyword 6", L"bold; fore:#0A246A", L"" },
        //{ {SCE_D_WORD}, IDS_LEX_STR_Keyword, L"Keyword 7", L"bold; fore:#0A246A", L"" },
        { {SCE_D_TYPEDEF}, IDS_LEX_STR_63258, L"Typedef", L"italic; fore:#0A246A", L"" },
        { {MULTI_STYLE(SCE_D_STRING,SCE_D_CHARACTER,SCE_D_STRINGEOL,0)}, IDS_LEX_STR_String, L"String", L"italic; fore:#3C6CDD", L"" },
        { {SCE_D_OPERATOR}, IDS_LEX_STR_Operator, L"Operator", L"fore:#B000B0", L"" },
        { {SCE_D_IDENTIFIER}, IDS_LEX_STR_Identifier, L"Identifier", L"", L"" },
        //{ {SCE_D_COMMENTLINEDOC}, L"Default", L"", L"" },
        //{ {SCE_D_COMMENTDOCKEYWORD}, L"Default", L"", L"" },
        //{ {SCE_D_STRINGB}, L"Default", L"", L"" },
        //{ {SCE_D_STRINGR}, L"Default", L"", L"" },
        //C++: { MULTI_STYLE(SCE_C_PREPROCESSOR,SCE_C_PREPROCESSORCOMMENT,SCE_C_PREPROCESSORCOMMENTDOC,0), IDS_LEX_STR_PreProc, L"Preprocessor", L"fore:#FF8000", L"" },
        EDITLEXER_SENTINEL
    }
};
