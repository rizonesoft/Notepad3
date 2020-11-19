#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_RC =
{
    "ACCELERATORS ALT AUTO3STATE AUTOCHECKBOX AUTORADIOBUTTON BEGIN BITMAP BLOCK BUTTON CAPTION CHARACTERISTICS "
    "CHECKBOX CLASS COMBOBOX CONTROL CTEXT CURSOR DEFPUSHBUTTON DIALOG DIALOGEX DISCARDABLE EDITTEXT END "
    "EXSTYLE FONT GROUPBOX ICON LANGUAGE LISTBOX LTEXT MENU MENUEX MENUITEM MESSAGETABLE POPUP PUSHBUTTON "
    "RADIOBUTTON RCDATA RTEXT SCROLLBAR SEPARATOR SHIFT STATE3 STRINGTABLE STYLE TEXTINCLUDE VALUE VERSION "
    "VERSIONINFO VIRTKEY",
    NULL,
};


EDITLEXER lexRC =
{
    SCLEX_CPP, "cpp", IDS_LEX_RESOURCE_SCR, L"Resource Script", L"rc; rc2; rct; rh; dlg; lang", L"",
    &KeyWords_RC, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
        //{ {SCE_C_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
        { {SCE_C_COMMENT}, IDS_LEX_STR_63127, L"Comment", L"fore:#008000", L"" },
        { {SCE_C_WORD}, IDS_LEX_STR_63128, L"Keyword", L"bold; fore:#0A246A", L"" },
        { {SCE_C_IDENTIFIER}, IDS_LEX_STR_63129, L"Identifier", L"", L"" },
        { {MULTI_STYLE(SCE_C_STRING,SCE_C_CHARACTER,SCE_C_STRINGEOL,SCE_C_VERBATIM)}, IDS_LEX_STR_63131, L"String", L"fore:#008000", L"" },
        { {SCE_C_NUMBER}, IDS_LEX_STR_63130, L"Number", L"fore:#FF0000", L"" },
        { {SCE_C_OPERATOR}, IDS_LEX_STR_63132, L"Operator", L"fore:#0A246A", L"" },
        { {SCE_C_PREPROCESSOR}, IDS_LEX_STR_63133, L"Preprocessor", L"fore:#FF8000", L"" },
        //{ {SCE_C_UUID}, L"UUID", L"", L"" },
        //{ {SCE_C_REGEX}, L"Regex", L"", L"" },
        //{ {SCE_C_WORD2}, L"Word 2", L"", L"" },
        //{ {SCE_C_GLOBALCLASS}, L"Global Class", L"", L"" },
        EDITLEXER_SENTINEL
    }
};
