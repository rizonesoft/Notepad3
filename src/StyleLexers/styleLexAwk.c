#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_Awk =
{
// Keywords
    "@include BEGIN BEGINFILE END and asort asorti atan2 bindtextdomain break case close compl continue cos "
    "dcgettext dcngettext default delete do else exit exp fflush for function gensub gsub if in index int "
    "isarray length log lshift match mktime next nextfile patsplit print printf rand return rshift sin split "
    "sprintf sqrt srand strftime strtonum sub substr switch system systime tolower toupper while xor",
// Highlighted identifiers (Keywords 2nd)
    "ARGC ARGIND ARGV BINMODE CONVFMT ENVIRON ERRNO FIELDWIDTHS FILENAME FNR FPAT FS IGNORECASE LINT NF NR "
    "OFMT OFS ORS PROCINFO RLENGTH RS RSTART RT SUBSEP TEXTDOMAIN",
    NULL,
};


EDITLEXER lexAwk =
{
    SCLEX_PYTHON, "python", IDS_LEX_AWK_SCR, L"Awk Script", L"awk", L"",
    &KeyWords_Awk,{
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_P_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {SCE_P_WORD}, IDS_LEX_STR_Keyword, L"Keyword", L"bold; fore:#0000A0", L"" },
        { {SCE_P_WORD}, IDS_LEX_STR_Keyword2nd, L"Keyword 2nd", L"bold; italic; fore:#6666FF", L"" },
        { {SCE_P_IDENTIFIER}, IDS_LEX_STR_Identifier, L"Identifier", L"", L"" },
        { {MULTI_STYLE(SCE_P_COMMENTLINE,SCE_P_COMMENTBLOCK,0,0)}, IDS_LEX_STR_Comment, L"Comment", L"fore:#808080", L"" },
        { {MULTI_STYLE(SCE_P_STRING,SCE_P_STRINGEOL,SCE_P_CHARACTER,0)}, IDS_LEX_STR_String, L"String", L"fore:#008000", L"" },
        { {SCE_P_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#C04000", L"" },
        { {SCE_P_OPERATOR}, IDS_LEX_STR_Operator, L"Operator", L"fore:#B000B0", L"" },
        EDITLEXER_SENTINEL
    }
};
