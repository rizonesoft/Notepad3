#include "StyleLexers.h"

// ----------------------------------------------------------------------------

// https://github.com/nim-lang/Nim/blob/devel/doc/keywords.txt

KEYWORDLIST KeyWords_Nim =
{
    "addr and as asm bind block break case cast concept const continue converter defer discard distinct div do "
    "elif else end enum except export finally for from func if import in include interface is isnot iterator "
    "let macro method mixin mod nil not notin object of or out proc ptr raise ref return shl shr static "
    "template try tuple type using var when while xor yield",
    NULL,
};



EDITLEXER lexNim =
{
    SCLEX_NIM, "nim", IDS_LEX_NIM_SRC, L"Nim Source Code", L"nim; nimrod", L"",
    &KeyWords_Nim,{
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_NIM_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {MULTI_STYLE(SCE_NIM_COMMENT,SCE_NIM_COMMENTDOC,SCE_NIM_COMMENTLINE,SCE_NIM_COMMENTLINEDOC)}, IDS_LEX_STR_Comment, L"Comment", L"italic; fore:#484A86", L"" },
        { {SCE_NIM_WORD}, IDS_LEX_STR_Keyword, L"Keyword", L"bold; fore:#5E8F60", L"" },
        { {SCE_NIM_IDENTIFIER}, IDS_LEX_STR_Identifier, L"Identifier", L"", L"" },
        { {MULTI_STYLE(SCE_NIM_STRING,SCE_NIM_STRINGEOL,0,0)}, IDS_LEX_STR_63211, L"String Double Quoted", L"fore:#A4255B", L"" },
        { {SCE_NIM_CHARACTER}, IDS_LEX_STR_63212, L"String Single Quoted", L"fore:#A4255B", L"" },
        { {SCE_NIM_TRIPLEDOUBLE}, IDS_LEX_STR_63244, L"String Triple Double Quotes", L"fore:#A4255B", L"" },
        { {SCE_NIM_TRIPLE}, IDS_LEX_STR_63245, L"String Triple Single Quotes", L"fore:#A4255B", L"" },
        { {SCE_NIM_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#252DBE", L"" },
        { {SCE_NIM_OPERATOR}, IDS_LEX_STR_Operator, L"Operator", L"bold; fore:#4B4B4B", L"" },
        { {SCE_NIM_FUNCNAME}, IDS_LEX_STR_FctName, L"Function name", L"fore:#4B4B4B", L"" },
        { {SCE_NIM_NUMERROR}, IDS_LEX_STR_63252, L"Parsing Error", L"italic; fore:#FFFF00; back:#A00000", L"" },
        //{ {SCE_NIM_BACKTICKS}, IDS_LEX_STR_Scalar, L"Back Ticks", L"fore:#660066", L"" },
        EDITLEXER_SENTINEL
    }
};
