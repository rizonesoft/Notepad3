#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_Abaqus = EMPTY_KEYWORDLIST;

EDITLEXER lexAbaqus =
{
    SCLEX_ABAQUS, "abaqus", IDS_LEX_ABAQUS, L"ABAQUS", L"inp", L"",
    &KeyWords_Abaqus, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_ABAQUS_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {MULTI_STYLE(SCE_ABAQUS_COMMENT,SCE_ABAQUS_COMMENTBLOCK,0,0)}, IDS_LEX_STR_Comment, L"Comment", L"fore:#008800", L"" },
        { {SCE_ABAQUS_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#FF8000", L"" },
        { {SCE_ABAQUS_STRING}, IDS_LEX_STR_String, L"String", L"fore:#808080", L"" },
        { {SCE_ABAQUS_OPERATOR}, IDS_LEX_STR_Operator, L"Operator", L"bold; fore:#FF8000", L"" },
        { {SCE_ABAQUS_STARCOMMAND}, IDS_LEX_STR_StarCmd, L"Star Command", L"bold; fore:#0A246A", L"" },
        { {SCE_ABAQUS_ARGUMENT}, IDS_LEX_STR_Argument, L"Argument", L"fore:#4747B0", L"" },
        { {SCE_ABAQUS_PROCESSOR}, IDS_LEX_STR_Processor, L"Processor", L"fore:#FF0000", L"" },
        { {SCE_ABAQUS_COMMAND}, IDS_LEX_STR_Cmd, L"Command", L"bold; fore:#003CE6", L"" },
        { {SCE_ABAQUS_SLASHCOMMAND}, IDS_LEX_STR_SlashCmd, L"Slash Command", L"bold; fore:#003CE6", L"" },
        { {SCE_ABAQUS_WORD}, IDS_LEX_STR_Word, L"Word", L"fore:#000000", L"" },
        { {SCE_ABAQUS_FUNCTION}, IDS_LEX_STR_Function, L"Function", L"fore:#A46000", L"" },
        EDITLEXER_SENTINEL
    }
};
