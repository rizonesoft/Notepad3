#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_BAT =
{
    "arp assoc attrib bcdedit bootcfg break cacls call cd change chcp chdir chkdsk chkntfs choice cipher "
    "cleanmgr cls cmd cmdkey color com comp compact con convert copy country ctty date defined defrag del dir "
    "disableextensions diskcomp diskcopy diskpart do doskey driverquery echo echo. else enabledelayedexpansion "
    "enableextensions endlocal equ erase errorlevel exist exit expand fc find findstr for forfiles format "
    "fsutil ftp ftype geq goto goto:eof gpresult gpupdate graftabl gtr help icacls if in ipconfig kill label leq "
    "loadfix loadhigh logman logoff lpt lss md mem mkdir mklink mode more move msg msiexe nbtstat neq net "
    "netsh netstat not nslookup nul openfiles path pathping pause perfmon popd powercfg print prompt pushd rd "
    "recover reg regedit regsvr32 rem ren rename replace rmdir robocopy route runas rundll32 sc schtasks "
    "sclist set setlocal sfc shift shutdown sort start subst systeminfo taskkill tasklist time timeout title "
    "tracert tree type typeperf ver verify vol wmic xcopy",
    NULL,
};


EDITLEXER lexBAT =
{
    SCLEX_BATCH, "batch", IDS_LEX_BATCH, L"Batch Files", L"bat; cmd", L"",
    &KeyWords_BAT, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_BAT_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {SCE_BAT_COMMENT}, IDS_LEX_STR_Comment, L"Comment", L"fore:#008000", L"" },
        { {SCE_BAT_WORD}, IDS_LEX_STR_Keyword, L"Keyword", L"bold; fore:#0A246A", L"" },
        { {SCE_BAT_IDENTIFIER}, IDS_LEX_STR_Identifier, L"Identifier", L"fore:#003CE6; back:#FFF1A8", L"" },
        { {SCE_BAT_OPERATOR}, IDS_LEX_STR_Operator, L"Operator", L"fore:#B000B0", L"" },
        { {MULTI_STYLE(SCE_BAT_COMMAND,SCE_BAT_HIDE,0,0)}, IDS_LEX_STR_Cmd, L"Command", L"bold", L"" },
        { {SCE_BAT_LABEL}, IDS_LEX_STR_Label, L"Label", L"fore:#C80000; back:#F4F4F4; eolfilled", L"" },
        EDITLEXER_SENTINEL
    }
};
