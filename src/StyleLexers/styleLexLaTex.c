#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_LaTex = EMPTY_KEYWORDLIST;

EDITLEXER lexLATEX =
{
    SCLEX_LATEX, "latex", IDS_LEX_LATEX, L"LaTeX Files", L"tex; latex; sty; texi; texinfo; txi", L"",
    &KeyWords_LaTex, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_L_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {MULTI_STYLE(SCE_L_COMMAND,SCE_L_SHORTCMD,SCE_L_CMDOPT,0)}, IDS_LEX_STR_Cmd, L"Command", L"fore:#0000FF", L"" },
        { {MULTI_STYLE(SCE_L_COMMENT,SCE_L_COMMENT2,0,0)}, IDS_LEX_STR_Comment, L"Comment", L"fore:#008000", L"" },
        { {MULTI_STYLE(SCE_L_MATH,SCE_L_MATH2,0,0)}, IDS_LEX_STR_Math, L"Math", L"fore:#FF0000", L"" },
        { {SCE_L_SPECIAL}, IDS_LEX_STR_63306, L"Special Char", L"fore:#AAAA00", L"" },
        { {MULTI_STYLE(SCE_L_TAG,SCE_L_TAG2,0,0)}, IDS_LEX_STR_Tag, L"Tag", L"fore:#0000FF", L"" },
        { {SCE_L_VERBATIM}, IDS_LEX_STR_63307, L"Verbatim Segment", L"fore:#666666", L"" },
        { {SCE_L_ERROR}, IDS_LEX_STR_Error, L"Error", L"fore:#FFFF00; back:#A00000", L"" },
        EDITLEXER_SENTINEL
    }
};
