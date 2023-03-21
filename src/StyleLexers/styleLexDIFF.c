#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_DIFF = EMPTY_KEYWORDLIST;


EDITLEXER lexDIFF =
{
    SCLEX_DIFF, "diff", IDS_LEX_DIFF, L"Diff Files", L"diff; patch", L"",
    &KeyWords_DIFF, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_DIFF_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {SCE_DIFF_COMMENT}, IDS_LEX_STR_Comment, L"Comment", L"fore:#008000", L"" },
        { {SCE_DIFF_COMMAND}, IDS_LEX_STR_Cmd, L"Command", L"bold; fore:#0A246A", L"" },
        { {SCE_DIFF_HEADER}, IDS_LEX_STR_63238, L"Source and Destination", L"fore:#C80000; back:#FFF1A8; eolfilled", L"" },
        { {SCE_DIFF_POSITION}, IDS_LEX_STR_63239, L"Position Setting", L"fore:#0000FF", L"" },
        { {MULTI_STYLE(SCE_DIFF_ADDED, SCE_DIFF_PATCH_ADD, SCE_DIFF_REMOVED_PATCH_ADD, 0)}, IDS_LEX_STR_63240, L"Line Addition", L"fore:#002000; back:#80FF80; eolfilled", L"" },
        { {MULTI_STYLE(SCE_DIFF_DELETED, SCE_DIFF_PATCH_DELETE, SCE_DIFF_REMOVED_PATCH_DELETE, 0)}, IDS_LEX_STR_63241, L"Line Removal", L"fore:#200000; back:#FF8080; eolfilled", L"" },
        { {SCE_DIFF_CHANGED}, IDS_LEX_STR_63242, L"Line Change", L"fore:#000020; back:#8080FF; eolfilled", L"" },
        EDITLEXER_SENTINEL
    }
};
