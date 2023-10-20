#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_RUBY =
{
    "__FILE__ __LINE__ alias and begin break case class def defined? do else elsif end ensure false for if in "
    "module next nil not or redo rescue retry return self super then true undef unless until when while yield",
    NULL,
};

EDITLEXER lexRUBY =
{
    SCLEX_RUBY, "ruby", IDS_LEX_RUBY, L"Ruby Script", L"rb; ruby; rbw; rake; rjs; rakefile; gemspec; podspec; \\^Rakefile$; \\^Podfile$", L"",
    &KeyWords_RUBY, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_RB_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {MULTI_STYLE(SCE_RB_COMMENTLINE,SCE_P_COMMENTBLOCK,0,0)}, IDS_LEX_STR_Comment, L"Comment", L"fore:#008000", L"" },
        { {SCE_RB_WORD}, IDS_LEX_STR_Keyword, L"Keyword", L"fore:#00007F", L"" },
        { {SCE_RB_IDENTIFIER}, IDS_LEX_STR_Identifier, L"Identifier", L"", L"" },
        { {SCE_RB_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#008080", L"" },
        { {SCE_RB_OPERATOR}, IDS_LEX_STR_Operator, L"Operator", L"", L"" },
        { {MULTI_STYLE(SCE_RB_STRING,SCE_RB_CHARACTER,SCE_P_STRINGEOL,0)}, IDS_LEX_STR_String, L"String", L"fore:#FF8000", L"" },
        { {SCE_RB_CLASSNAME}, IDS_LEX_STR_ClassName, L"Class Name", L"fore:#0000FF", L"" },
        { {SCE_RB_DEFNAME}, IDS_LEX_STR_FctName, L"Function Name", L"fore:#007F7F", L"" },
        { {SCE_RB_POD}, IDS_LEX_STR_POD, L"POD", L"fore:#004000; back:#C0FFC0; eolfilled", L"" },
        { {SCE_RB_REGEX}, IDS_LEX_STR_RegEx, L"Regex", L"fore:#000000; back:#A0FFA0", L"" },
        { {SCE_RB_GLOBAL}, IDS_LEX_STR_Global, L"Global", L"fore:#EA4D00", L"" },
        { {SCE_RB_SYMBOL}, IDS_LEX_STR_Symbol, L"Symbol", L"fore:#C0A030", L"" },
        { {SCE_RB_MODULE_NAME}, IDS_LEX_STR_63294, L"Module Name", L"fore:#A000A0", L"" },
        { {SCE_RB_INSTANCE_VAR}, IDS_LEX_STR_63295, L"Instance Var", L"fore:#B00080", L"" },
        { {SCE_RB_CLASS_VAR}, IDS_LEX_STR_63296, L"Class Var", L"fore:#8000B0", L"" },
        { {SCE_RB_DATASECTION}, IDS_LEX_STR_63222, L"Data Section", L"fore:#600000; back:#FFF0D8; eolfilled", L"" },
        { {SCE_RB_ERROR}, IDS_LEX_STR_Error, L"Error", L"fore:#FFFF00; back:#A00000; eolfilled", L"" },
        EDITLEXER_SENTINEL
    }
};

//SCE_RB_BACKTICKS=18
//SCE_RB_HERE_DELIM=20
//SCE_RB_HERE_Q=21
//SCE_RB_HERE_QQ=22
//SCE_RB_HERE_QX=23
//SCE_RB_STRING_Q=24
//SCE_RB_STRING_QQ=25
//SCE_RB_STRING_QX=26
//SCE_RB_STRING_QR=27
//SCE_RB_STRING_QW=28
//SCE_RB_WORD_DEMOTED=29
//SCE_RB_STDIN=30
//SCE_RB_STDOUT=31
//SCE_RB_STDERR=40
//SCE_RB_STRING_W=41
//SCE_RB_STRING_I=42
//SCE_RB_STRING_QI=43
//SCE_RB_STRING_QS=44
//SCE_RB_UPPER_BOUND=45
