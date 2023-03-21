#include "StyleLexers.h"

// ----------------------------------------------------------------------------

//KEYWORDLIST KeyWords_KiX = EMPTY_KEYWORDLIST;
KEYWORDLIST KeyWords_KiX =
{
    // keywords
    "beep big break call case cd cls color cookie1 copy debug del dim display do each else endif endselect exit "
    "flushkb for function get gets global go gosub goto if include loop md move next "
    "password play quit rd redim return run select set setl setm settime shell sleep small until use while",
    // functions
    "abs addkey addprinterconnection addprogramgroup addprogramitem asc ascan at backupeventlog box "
    "cdbl chr cin cleareventlog close comparefiletimes createobject cstr "
    "dectohex delkey delprinterconnection delprogramgroup delprogramitem deltree delvalue dir "
    "enumgroup enumipinfo enumkey enumlocalgroup enumvalue execute exist existkey expandenviromentvars "
    "fix formatnumber freefilehandle getdiskspace getfileattr getfilesize getfiletime getfileversion getobject "
    "iif ingroup instr instrrev int isdeclared join kbhit keyexist lcase left len loadhive loadkey logevent logoff ltrim "
    "memorysize messagebox open readline readprofilestring readtype readvalue redirectoutput right rnd round rtrim "
    "savekey sendkeys sendmessage setascii setconsole setdefaultprinter setfileattr setfocus setoption setsystemstate settitle showprogramgroup shutdown sidtoname srnd "
    "trim ubound ucase unloadhive val vartype vartypename writeline writeprofilestring writevalue",
    // macros
    "address build color comment cpu crlf csd curdir date day domain dos error fullname homedir homedrive homeshr hostname "
    "inwin ipaddress kix lanroot ldomain ldrive lm logonmode longhomedir lserver maxpwage mdayno mhz monthno month msecs "
    "onwow64 pid primarygroup priv productsuite producttype pwage ras result rserver "
    "scriptdir scriptexe scriptname serror sid site startdir syslang ticks time tssession userid userlang wdayno wksta wuserid ydayno year",
    NULL,
};


EDITLEXER lexKiX =
{
    SCLEX_KIX, "kix", IDS_LEX_KIX_SCR, L"KiXtart Script", L"kix", L"",
    &KeyWords_KiX, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_KIX_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {SCE_KIX_KEYWORD}, IDS_LEX_STR_Keyword, L"Keyword", L"bold; fore:#265CFF", L"" },
        { {SCE_KIX_FUNCTIONS}, IDS_LEX_STR_63277, L"Function", L"fore:#9B009B", L"" },
        { {SCE_KIX_MACRO}, IDS_LEX_STR_63280, L"Macro", L"fore:#FFC000", L"" },
        { {MULTI_STYLE(SCE_KIX_COMMENT, SCE_KIX_COMMENTSTREAM,0,0)}, IDS_LEX_STR_Comment, L"Comment", L"fore:#008000", L"" },
        { {MULTI_STYLE(SCE_KIX_STRING1, SCE_KIX_STRING2,0,0)}, IDS_LEX_STR_String, L"String", L"italic; fore:#8F8F8F", L"" },
        { {SCE_KIX_OPERATOR}, IDS_LEX_STR_Operator, L"Operator", L"fore:#C80000", L"" },
        { {SCE_KIX_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#009797", L"" },
        { {SCE_KIX_VAR}, IDS_LEX_STR_Var, L"Variable", L"fore:#9E4D2A", L"" },
        { {SCE_KIX_IDENTIFIER}, IDS_LEX_STR_Identifier, L"Identifier", L"", L"" },
        EDITLEXER_SENTINEL
    }
};
