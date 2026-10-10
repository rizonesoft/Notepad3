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
    "itk itk_component itk_initialize itk_interior itk_option iwidgets keep method private protected public "
    "tkButtonAutoInvoke tkButtonDown tkButtonEnter tkButtonInvoke tkButtonLeave tkButtonUp tkCancelRepeat tkCheckRadioDown "
    "tkCheckRadioEnter tkCheckRadioInvoke tkColorDialog tkColorDialog_BuildDialog tkColorDialog_CancelCmd tkColorDialog_Config "
    "tkColorDialog_CreateSelector tkColorDialog_DrawColorScale tkColorDialog_EnterColorBar tkColorDialog_HandleRGBEntry "
    "tkColorDialog_HandleSelEntry tkColorDialog_InitValues tkColorDialog_LeaveColorBar tkColorDialog_MoveSelector "
    "tkColorDialog_OkCmd tkColorDialog_RedrawColorBars tkColorDialog_RedrawFinalColor tkColorDialog_ReleaseMouse "
    "tkColorDialog_ResizeColorBars tkColorDialog_RgbToX tkColorDialog_SetRGBValue tkColorDialog_StartMove tkColorDialog_XToRgb "
    "tkConsoleAbout tkConsoleBind tkConsoleExit tkConsoleHistory tkConsoleInit tkConsoleInsert tkConsoleInvoke tkConsoleOutput "
    "tkConsolePrompt tkConsoleSource tkDarken tkEntryAutoScan tkEntryBackspace tkEntryButton1 tkEntryClosestGap tkEntryGetSelection "
    "tkEntryInsert tkEntryKeySelect tkEntryMouseSelect tkEntryNextWord tkEntryPaste tkEntryPreviousWord tkEntrySeeInsert tkEntrySetCursor "
    "tkEntryTranspose tkEventMotifBindings tkFDGetFileTypes tkFirstMenu tkFocusGroup_BindIn tkFocusGroup_BindOut tkFocusGroup_Create "
    "tkFocusGroup_Destroy tkFocusGroup_In tkFocusGroup_Out tkFocusOK tkGenerateMenuSelect tkIconList tkIconList_Add tkIconList_Arrange "
    "tkIconList_AutoScan tkIconList_Btn1 tkIconList_Config tkIconList_Create tkIconList_CtrlBtn1 tkIconList_Curselection "
    "tkIconList_DeleteAll tkIconList_Double1 tkIconList_DrawSelection tkIconList_FocusIn tkIconList_FocusOut tkIconList_Get "
    "tkIconList_Goto tkIconList_Index tkIconList_Invoke tkIconList_KeyPress tkIconList_Leave1 tkIconList_LeftRight tkIconList_Motion1 "
    "tkIconList_Reset tkIconList_ReturnKey tkIconList_See tkIconList_Select tkIconList_Selection tkIconList_ShiftBtn1 tkIconList_UpDown "
    "tkListbox tkListboxAutoScan tkListboxBeginExtend tkListboxBeginSelect tkListboxBeginToggle tkListboxCancel tkListboxDataExtend "
    "tkListboxExtendUpDown tkListboxKeyAccel_Goto tkListboxKeyAccel_Key tkListboxKeyAccel_Reset tkListboxKeyAccel_Set tkListboxKeyAccel_Unset "
    "tkListboxMotion tkListboxSelectAll tkListboxUpDown tkMbButtonUp tkMbEnter tkMbLeave tkMbMotion tkMbPost tkMenuButtonDown "
    "tkMenuDownArrow tkMenuDup tkMenuEscape tkMenuFind tkMenuFindName tkMenuFirstEntry tkMenuInvoke tkMenuLeave tkMenuLeftArrow "
    "tkMenuMotion tkMenuNextEntry tkMenuNextMenu tkMenuRightArrow tkMenuUnpost tkMenuUpArrow tkMessageBox tkMotifFDialog "
    "tkMotifFDialog_ActivateDList tkMotifFDialog_ActivateFEnt tkMotifFDialog_ActivateFList tkMotifFDialog_ActivateSEnt "
    "tkMotifFDialog_BrowseDList tkMotifFDialog_BrowseFList tkMotifFDialog_BuildUI tkMotifFDialog_CancelCmd tkMotifFDialog_Config "
    "tkMotifFDialog_Create tkMotifFDialog_FileTypes tkMotifFDialog_FilterCmd tkMotifFDialog_InterpFilter tkMotifFDialog_LoadFiles "
    "tkMotifFDialog_MakeSList tkMotifFDialog_OkCmd tkMotifFDialog_SetFilter tkMotifFDialog_SetListMode tkMotifFDialog_Update "
    "tkPostOverPoint tkRecolorTree tkRestoreOldGrab tkSaveGrabInfo tkScaleActivate tkScaleButton2Down tkScaleButtonDown tkScaleControlPress "
    "tkScaleDrag tkScaleEndDrag tkScaleIncrement tkScreenChanged tkScrollButton2Down tkScrollButtonDown tkScrollButtonDrag tkScrollButtonUp "
    "tkScrollByPages tkScrollByUnits tkScrollDrag tkScrollEndDrag tkScrollSelect tkScrollStartDrag tkScrollToPos tkScrollTopBottom tkTabToWindow "
    "tkTearOffMenu tkTextAutoScan tkTextButton1 tkTextClosestGap tkTextInsert tkTextKeyExtend tkTextKeySelect tkTextNextPara tkTextNextPos "
    "tkTextNextWord tkTextPaste tkTextPrevPara tkTextPrevPos tkTextPrevWord tkTextResetAnchor tkTextScrollPages tkTextSelectTo tkTextSetCursor "
    "tkTextTranspose tkTextUpDownLine tkTraverseToMenu tkTraverseWithinMenu tk_bisque tk_focusFollowsMouse tk_focusPrev tk_getSaveFile tk_textCopy tk_textCut tk_textPaste",
    // User slot 3
    "",
    // User slot 4
    "",
    // User slot 5
    "",
    // User slot 6
    "",
    // User slot 7
    "",
    // User slot 8
    "",
    NULL,
};


#define SCE_TCL__MULTI_COMMENT      MULTI_STYLE(SCE_TCL_COMMENT,SCE_TCL_COMMENTLINE,SCE_TCL_COMMENT_BOX,SCE_TCL_BLOCK_COMMENT)
#define SCE_TCL__MULTI_KEYWORD      MULTI_STYLE(SCE_TCL_WORD,SCE_TCL_WORD2,SCE_TCL_WORD3,SCE_TCL_WORD_IN_QUOTE)
#define SCE_TCL__MULTI_SUBSTITUTION MULTI_STYLE(SCE_TCL_SUBSTITUTION,SCE_TCL_SUB_BRACE,0,0)


EDITLEXER lexTCL =
{
    SCLEX_TCL, "tcl", IDS_LEX_TCL, L"Tcl Script", L"tcl; itcl; tm; ts; tsx", L"",
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
        { {SCE_TCL_EXPAND}, IDS_LEX_STR_TclExpand, L"Expansion {*}", L"bold; fore:#FF8000", L"" },
        { {MULTI_STYLE(SCE_TCL_WORD4,SCE_TCL_WORD5,SCE_TCL_WORD6,SCE_TCL_WORD7)}, IDS_LEX_STR_Keyword47, L"Keyword 4-7", L"fore:#0080FF", L"" },
        { {SCE_TCL_WORD8}, IDS_LEX_STR_Keyword8, L"Keyword 8", L"fore:#0080FF", L"" },
        //{ {SCE_TCL_WORD}, IDS_LEX_STR_Word, L"Keyword 1", L"", L"" },
        //{ {SCE_TCL_WORD2}, IDS_LEX_STR_Word, L"Keyword 2", L"", L"" },
        //{ {SCE_TCL_WORD3}, IDS_LEX_STR_Word, L"Keyword 3", L"", L"" },
        EDITLEXER_SENTINEL
    }
};

