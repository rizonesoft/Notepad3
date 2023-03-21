#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_VBS =
{
    "alias and as attribute begin boolean byref byte byval call case class compare const continue currency date "
    "declare dim do double each else elseif empty end enum eqv erase error event exit explicit false for "
    "friend function get global gosub goto if imp implement in integer is let lib load long loop lset me mid "
    "mod module new next not nothing null object on option optional or preserve private property public "
    "raiseevent redim rem resume return rset select set single static stop string sub then to true type unload "
    "until variant wend while with withevents xor",
    NULL,
};


EDITLEXER lexVBS =
{
    SCLEX_VBSCRIPT, "vbscript", IDS_LEX_VB_SCR, L"VBScript", L"vbs; dsm", L"",
    &KeyWords_VBS, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_B_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {SCE_B_COMMENT}, IDS_LEX_STR_Comment, L"Comment", L"fore:#808080", L"" },
        { {SCE_B_KEYWORD}, IDS_LEX_STR_Keyword, L"Keyword", L"bold; fore:#B000B0", L"" },
        { {SCE_B_IDENTIFIER}, IDS_LEX_STR_Identifier, L"Identifier", L"", L"" },
        { {MULTI_STYLE(SCE_B_STRING,SCE_B_STRINGEOL,0,0)}, IDS_LEX_STR_String, L"String", L"fore:#008000", L"" },
        { {SCE_B_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#FF0000", L"" },
        { {SCE_B_OPERATOR}, IDS_LEX_STR_Operator, L"Operator", L"", L"" },
        //{ {SCE_B_PREPROCESSOR}, IDS_LEX_STR_PreProc, L"Preprocessor", L"fore:#FF9C00", L"" },
        //{ {SCE_B_CONSTANT}, L"Constant", L"", L"" },
        //{ {SCE_B_DATE}, L"Date", L"", L"" },
        //{ {SCE_B_KEYWORD2}, L"Keyword 2", L"", L"" },
        //{ {SCE_B_KEYWORD3}, L"Keyword 3", L"", L"" },
        //{ {SCE_B_KEYWORD4}, L"Keyword 4", L"", L"" },
        //{ {SCE_B_ASM}, L"Inline Asm", L"fore:#FF8000", L"" },
        EDITLEXER_SENTINEL
    }
};
