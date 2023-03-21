#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_TCL =
{
// TCL Keywords
    "Platform-specific after append array auto_execok auto_import auto_load auto_load_index auto_qualify beep "
    "bgerror binary break case catch cd clock close concat continue dde default echo else elseif encoding eof "
    "error eval exec exit expr fblocked fconfigure fcopy file fileevent flush for foreach format gets glob "
    "global history http if incr info interp join lappend lindex linsert list llength load loadTk lrange "
    "lreplace lsearch lset lsort memory msgcat namespace open package pid pkg::create pkg_mkIndex proc puts "
    "pwd re_syntax read regexp registry regsub rename resource return scan seek set socket source split string "
    "subst switch tclLog tclMacPkgSearch tclPkgSetup tclPkgUnknown tell time trace unknown unset update "
    "uplevel upvar variable vwait while",
// TK Keywords
    "Inter-client bell bind bindtags bitmap button canvas checkbutton clipboard colors console cursors destroy "
    "entry event focus font frame grab grid image keysyms label labelframe listbox lower menu menubutton "
    "message option options pack panedwindow photo place radiobutton raise scale scrollbar selection send "
    "spinbox text tk tk_chooseColor tk_chooseDirectory tk_dialog tk_focusNext tk_getOpenFile tk_messageBox "
    "tk_optionMenu tk_popup tk_setPalette tkerror tkvars tkwait toplevel winfo wish wm",
// iTCL Keywords
    "@scope body class code common component configbody constructor define destructor hull import inherit itcl "
    "itk itk_component itk_initialize itk_interior itk_option iwidgets keep method private protected public",
    NULL,
};


#define SCE_TCL__MULTI_COMMENT      MULTI_STYLE(SCE_TCL_COMMENT,SCE_TCL_COMMENTLINE,SCE_TCL_COMMENT_BOX,SCE_TCL_BLOCK_COMMENT)
#define SCE_TCL__MULTI_KEYWORD      MULTI_STYLE(SCE_TCL_WORD,SCE_TCL_WORD2,SCE_TCL_WORD3,SCE_TCL_WORD_IN_QUOTE)
#define SCE_TCL__MULTI_SUBSTITUTION MULTI_STYLE(SCE_TCL_SUBSTITUTION,SCE_TCL_SUB_BRACE,0,0)


EDITLEXER lexTCL =
{
    SCLEX_TCL, "tcl", IDS_LEX_TCL, L"Tcl Script", L"tcl; itcl; tm", L"",
    &KeyWords_TCL, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_TCL_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {SCE_TCL__MULTI_COMMENT}, IDS_LEX_STR_Comment, L"Comment", L"fore:#008000", L"" },
        { {SCE_TCL__MULTI_KEYWORD}, IDS_LEX_STR_Keyword, L"Keyword", L"fore:#0000FF", L"" },
        { {SCE_TCL_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#008080", L"" },
        { {SCE_TCL_IN_QUOTE}, IDS_LEX_STR_String, L"String", L"fore:#008080", L"" },
        { {SCE_TCL_OPERATOR}, IDS_LEX_STR_Operator, L"Operator", L"", L"" },
        { {SCE_TCL_IDENTIFIER}, IDS_LEX_STR_Identifier, L"Identifier", L"fore:#800080", L"" },
        { {SCE_TCL__MULTI_SUBSTITUTION}, IDS_LEX_STR_Subst, L"Substitution", L"fore:#CC0000", L"" },
        { {SCE_TCL_MODIFIER}, IDS_LEX_STR_Modf, L"Modifier", L"fore:#FF00FF", L"" },
        EDITLEXER_SENTINEL
    }
};

