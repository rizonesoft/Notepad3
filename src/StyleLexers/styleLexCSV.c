#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_CSV = EMPTY_KEYWORDLIST;

EDITLEXER lexCSV =
{
    SCLEX_CSV, "CSV", IDS_LEX_PRISM_CSV, L"CSV Prism", L"csv", L"",
    & KeyWords_CSV,{
        //SCE_CSV_DEFAULT
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {STYLE_LINENUMBER}, IDS_LEX_STD_MARGIN, L"Margins and Line Numbers", L"", L"" },
        { {SCI_SETEXTRAASCENT + SCI_SETEXTRADESCENT}, IDS_LEX_STD_X_SPC, L"Extra Line Spacing (Size)", L"size:-1", L"" },
        { {SCE_CSV_COLUMN_0}, IDS_LEX_CSV_COL_0, L"Column 0", L"fore:#9400D3", L"" },
        { {SCE_CSV_COLUMN_1}, IDS_LEX_CSV_COL_1, L"Column 1", L"fore:#1C01AF", L"" },
        { {SCE_CSV_COLUMN_2}, IDS_LEX_CSV_COL_2, L"Column 2", L"fore:#0162F3", L"" },
        { {SCE_CSV_COLUMN_3}, IDS_LEX_CSV_COL_3, L"Column 3", L"fore:#28A4FF", L"" },
        { {SCE_CSV_COLUMN_4}, IDS_LEX_CSV_COL_4, L"Column 4", L"fore:#01C2C2", L"" },
        { {SCE_CSV_COLUMN_5}, IDS_LEX_CSV_COL_5, L"Column 5", L"fore:#00D530", L"" },
        { {SCE_CSV_COLUMN_6}, IDS_LEX_CSV_COL_6, L"Column 6", L"fore:#80D500", L"" },
        { {SCE_CSV_COLUMN_7}, IDS_LEX_CSV_COL_7, L"Column 7", L"fore:#D3E401", L"" },
        { {SCE_CSV_COLUMN_8}, IDS_LEX_CSV_COL_8, L"Column 8", L"fore:#FE9901", L"" },
        { {SCE_CSV_COLUMN_9}, IDS_LEX_CSV_COL_9, L"Column 9", L"fore:#D90000", L"" },

        EDITLEXER_SENTINEL
    }
};

