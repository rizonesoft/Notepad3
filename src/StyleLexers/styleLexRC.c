#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_RC =
{
    // Primary keywords
    "ACCELERATORS ALT AUTO3STATE AUTOCHECKBOX AUTORADIOBUTTON BEGIN BITMAP BLOCK BUTTON CAPTION CHARACTERISTICS "
    "CHECKBOX CLASS COMBOBOX CONTROL CTEXT CURSOR DEFPUSHBUTTON DIALOG DIALOGEX DISCARDABLE EDITTEXT END "
    "EXSTYLE FONT GROUPBOX ICON LANGUAGE LISTBOX LTEXT MENU MENUEX MENUITEM MESSAGETABLE POPUP PUSHBUTTON "
    "RADIOBUTTON RCDATA RTEXT SCROLLBAR SEPARATOR SHIFT STATE3 STRINGTABLE STYLE TEXTINCLUDE VALUE VERSION "
    "VERSIONINFO VIRTKEY",
    // Secondary keywords
    "",
    // Documentation comment keywords
    "addindex addtogroup anchor arg attention author b brief bug c class code copyright date def defgroup deprecated dontinclude "
    "e em endcode endhtmlonly endif endlatexonly endlink endverbatim enum example exception f$ f[f] file"
    "hideinitializer htmlinclude htmlonly if image include ingroup internal invariant interface latexonly li line link "
    "mainpage name namespace nosubgrouping note overload p page par param param[in] param[out] post pre "
    "ref relates remarks return retval sa section see showinitializer since skip skipline struct subsection "
    "test throw throws todo typedef union until var verbatim verbinclude version warning weakgroup",
    // Global classes and typedefs
    "",
    // Preprocessor definitions
    "",
    // Task marker and error marker keywords
    "BUG FIXME HACK NOTE TBD TODO UNDONE XXX @@@",
    NULL,
};


EDITLEXER lexRC = {
    SCLEX_CPP, "cpp", IDS_LEX_RESOURCE_SCR, L"Resource Script", L"rc; rc2; rct; rh; dlg; lang", L"",
    &KeyWords_RC, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_C_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {SCE_C_IDENTIFIER}, IDS_LEX_STR_Identifier, L"Identifier", L"", L"" },
        { {MULTI_STYLE(SCE_C_COMMENT,SCE_C_COMMENTLINE,0,0)}, IDS_LEX_STR_Comment, L"Comment", L"fore:#008000", L"" },
        { {SCE_C_WORD}, IDS_LEX_STR_Keyword, L"Keyword", L"bold; fore:#0A246A", L"" },
        //{ {SCE_C_WORD2}, IDS_LEX_STR_Keyword2nd, L"Keyword 2nd", L"bold; fore:#A46000", L"" },
        //{ {SCE_C_GLOBALCLASS}, IDS_LEX_STR_63258, L"Typedefs/Classes", L"bold; italic; fore:#800000", L"" },
        { {SCE_C_STRING}, IDS_LEX_STR_String, L"String", L"fore:#008000", L"" },
        //{ {SCE_C_REGEX}, IDS_LEX_STR_RegEx, L"Regex", L"fore:#006633; back:#FFF1A8", L"" },
        { {SCE_C_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#FF0000", L"" },
        { {SCE_C_OPERATOR}, IDS_LEX_STR_Operator, L"Operator", L"fore:#0A246A", L"" },
        { {MULTI_STYLE(SCE_C_PREPROCESSOR,SCE_C_PREPROCESSORCOMMENT,SCE_C_PREPROCESSORCOMMENTDOC,0)}, IDS_LEX_STR_PreProc, L"Preprocessor", L"fore:#FF8000", L"" },
        //{ {MULTI_STYLE(SCE_C_VERBATIM, SCE_C_TRIPLEVERBATIM,0,0)}, IDS_LEX_STR_VerbStrg, L"Verbatim", L"fore:#B000B0", L"" },
        { {MULTI_STYLE(SCE_C_COMMENTDOC,SCE_C_COMMENTLINEDOC,0,0)}, IDS_LEX_STR_63259, L"Comment Doc", L"fore:#808080", L"" },
        { {SCE_C_COMMENTDOCKEYWORD}, IDS_LEX_STR_63371, L"Comment Doc Word", L"bold; fore:#808080", L"" },
        { {SCE_C_COMMENTDOCKEYWORDERROR}, IDS_LEX_STR_63374, L"Comment Doc Error", L"italic; fore:#800000", L"" },
        { {SCE_C_TASKMARKER}, IDS_LEX_STR_63373, L"Task Marker", L"bold; fore:#208080", L"" },
        //{ {SCE_C_UUID}, L"UUID", L"", L"" },
        //{ {SCE_C_GLOBALCLASS}, L"Global Class", L"", L"" },
        EDITLEXER_SENTINEL
    }
};

