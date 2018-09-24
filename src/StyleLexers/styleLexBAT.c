#include "StyleLexers.h"

// ----------------------------------------------------------------------------

static __int64 LexFunction(LexFunctionType type, int value)
{
  static __int64 iStyleChanged = 0LL;

  switch (type)
  {
  case FCT_SETTING_CHANGE:
    if (value == 0) {
      return iStyleChanged;
    }
    else if (value > 0) {
      iStyleChanged |= (((__int64)1) << value);
    }
    else {  // value < 0
      iStyleChanged &= ~(((__int64)1) << (0 - value));
    }
    break;

  default:
    break;
  }
  return (__int64)0;
};

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_BAT = {
"arp assoc attrib bcdedit bootcfg break cacls call cd change chcp chdir chkdsk chkntfs choice cipher "
"cleanmgr cls cmd cmdkey color com comp compact con convert copy country ctty date defined defrag del "
"dir disableextensions diskcomp diskcopy diskpart do doskey driverquery echo echo. else enableextensions "
"enabledelayedexpansion endlocal equ erase errorlevel exist exit expand fc find findstr for forfiles format "
"fsutil ftp ftype geq goto gpresult gpupdate graftabl gtr help icacls if in ipconfig kill label leq loadfix "
"loadhigh logman logoff lpt lss md mem mkdir mklink mode more move msg msiexe nbtstat neq net netstat netsh "
"not nslookup nul openfiles path pathping pause perfmon popd powercfg print prompt pushd rd recover reg regedit "
"regsvr32 rem ren rename replace rmdir robocopy route runas rundll32 sc schtasks sclist set setlocal sfc shift "
"shutdown sort start subst systeminfo taskkill tasklist time timeout title tracert tree type typeperf ver verify "
"vol wmic xcopy",
"", "", "", "", "", "", "", "" };


EDITLEXER lexBAT = { 
SCLEX_BATCH, IDS_LEX_BATCH, L"Batch Files", L"bat; cmd", L"", 
&LexFunction, // static
&KeyWords_BAT, {
    { STYLE_DEFAULT, IDS_LEX_STR_63126, L"Default", L"", L"" },
    //{ SCE_BAT_DEFAULT, IDS_LEX_STR_63126, L"Default", L"", L"" },
    { SCE_BAT_COMMENT, IDS_LEX_STR_63127, L"Comment", L"fore:#008000", L"" },
    { SCE_BAT_WORD, IDS_LEX_STR_63128, L"Keyword", L"bold; fore:#0A246A", L"" },
    { SCE_BAT_IDENTIFIER, IDS_LEX_STR_63129, L"Identifier", L"fore:#003CE6; back:#FFF1A8", L"" },
    { SCE_BAT_OPERATOR, IDS_LEX_STR_63132, L"Operator", L"fore:#B000B0", L"" },
    { MULTI_STYLE(SCE_BAT_COMMAND,SCE_BAT_HIDE,0,0), IDS_LEX_STR_63236, L"Command", L"bold", L"" },
    { SCE_BAT_LABEL, IDS_LEX_STR_63235, L"Label", L"fore:#C80000; back:#F4F4F4; eolfilled", L"" },
    EDITLEXER_SENTINEL } };
