#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_D =
{
// Primary keywords and identifiers
    "abstract alias align asm assert auto body break case cast catch class const continue debug default "
    "delegate  delete deprecated do else enum export extern final finally for foreach foreach_reverse function "
    "goto if import  in inout interface invariant is lazy mixin module new out override package pragma private "
    "protected public  return scope static struct super switch synchronized template this throw try typedef "
    "typeid typeof union  unittest version volatile while with",
// Secondary keywords and identifiers
    "false null true",
// Documentation comment keywords  (doxygen)
    "a addindex addtogroup anchor arg attention author b brief bug c class code date def defgroup deprecated "
    "dontinclude e em endcode endhtmlonly endif endlatexonly endlink endverbatim enum example exception f$ "
    "f[f] file fn hideinitializer htmlinclude htmlonly if image include ingroup interface internal invariant "
    "latexonly li line link mainpage name namespace nosubgrouping note overload p page par param post pre ref "
    "relates remarks return retval sa section see showinitializer since skip skipline struct subsection test "
    "throw todo typedef union until var verbatim verbinclude version warning weakgroup",
// Type definitions and aliases
    "bool byte cdouble cent cfloat char creal dchar double float idouble ifloat int ireal long real short ubyte "
    "ucent uint ulong ushort void wchar",
// Keywords 5
    "",
// Keywords 6
    "",
// Keywords 7
    "",
// ---
    NULL,
};


EDITLEXER lexD =
{
    SCLEX_D, "d", IDS_LEX_D_SRC, L"D Source Code", L"d; dd; di", L"",
    &KeyWords_D, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_D_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {MULTI_STYLE(SCE_D_COMMENT,SCE_D_COMMENTLINE,SCE_D_COMMENTNESTED,0)}, IDS_LEX_STR_Comment, L"Comment", L"fore:#008000", L"" },
        { {SCE_D_COMMENTDOC}, IDS_LEX_STR_63259, L"Comment Doc", L"fore:#0040A0", L"" },
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
        EDITLEXER_SENTINEL
    }
};
