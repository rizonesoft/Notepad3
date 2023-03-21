#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_PAS =
{
    "absolute abstract alias and array as asm assembler begin break case cdecl class const constructor continue "
    "cppdecl default destructor dispose div do downto else end end. except exit export exports external false "
    "far far16 file finalization finally for forward function goto if implementation in index inherited "
    "initialization inline interface is label library local message mod name near new nil nostackframe not "
    "object of oldfpccall on operator or out overload override packed pascal private procedure program "
    "property protected public published raise read record register reintroduce repeat resourcestring safecall "
    "self set shl shr softfloat stdcall stored string then threadvar to true try type unit until uses var "
    "virtual while with write xor",
    NULL,
};


EDITLEXER lexPAS =
{
    SCLEX_PASCAL, "pascal", IDS_LEX_PASCAL_SRC, L"Pascal Source Code", L"pas; dpr; dpk; dfm; pp; lfm; lpr; fpd", L"",
    &KeyWords_PAS, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_PAS_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {MULTI_STYLE(SCE_PAS_COMMENT,SCE_PAS_COMMENT2,SCE_PAS_COMMENTLINE,0)}, IDS_LEX_STR_Comment, L"Comment", L"fore:#646464", L"" },
        { {SCE_PAS_WORD}, IDS_LEX_STR_Keyword, L"Keyword", L"bold; fore:#800080", L"" },
        { {SCE_PAS_IDENTIFIER}, IDS_LEX_STR_Identifier, L"Identifier", L"", L"" },
        { {MULTI_STYLE(SCE_PAS_STRING,SCE_PAS_CHARACTER,SCE_PAS_STRINGEOL,0)}, IDS_LEX_STR_String, L"String", L"fore:#008000", L"" },
        { {MULTI_STYLE(SCE_PAS_NUMBER,SCE_PAS_HEXNUMBER,0,0)}, IDS_LEX_STR_Number, L"Number", L"fore:#FF0000", L"" },
        { {SCE_PAS_OPERATOR}, IDS_LEX_STR_Operator, L"Operator", L"bold", L"" },
        { {SCE_PAS_ASM}, IDS_LEX_STR_63205, L"Inline Asm", L"fore:#0000FF", L"" },
        { {MULTI_STYLE(SCE_PAS_PREPROCESSOR,SCE_PAS_PREPROCESSOR2,0,0)}, IDS_LEX_STR_PreProc, L"Preprocessor", L"fore:#FF00FF", L"" },
        EDITLEXER_SENTINEL
    }
};

