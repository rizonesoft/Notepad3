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

KEYWORDLIST KeyWords_Awk = {
  // Keywords
  "break case continue default do else exit function for if in next return switch while "
  "@include delete nextfile print printf BEGIN BEGINFILE END "
  "atan2 cos exp int log rand sin sqrt srand asort asorti gensub gsub index "
  "length match patsplit split sprintf strtonum sub substr tolower toupper close "
  "fflush system mktime strftime systime and compl lshift rshift xor "
  "isarray bindtextdomain dcgettext dcngettext",

  // Highlighted identifiers (Keywords 2nd)
  "ARGC ARGIND ARGV FILENAME FNR FS NF NR OFMT OFS ORS RLENGTH RS RSTART SUBSEP TEXTDOMAIN "
  "BINMODE CONVFMT FIELDWIDTHS FPAT IGNORECASE LINT TEXTDOMAiN ENVIRON ERRNO PROCINFO RT",

  "", "", "", "", "", "" ,""
};


EDITLEXER lexAwk = { 
SCLEX_PYTHON,  IDS_LEX_AWK_SCR, L"Awk Script", L"awk", L"", 
&LexFunction, // static
&KeyWords_Awk,{
    { STYLE_DEFAULT, IDS_LEX_STR_63126, L"Default", L"", L"" },
    //{ SCE_P_DEFAULT, IDS_LEX_STR_63126, L"Default", L"", L"" },
    { SCE_P_WORD, IDS_LEX_STR_63128, L"Keyword", L"bold; fore:#0000A0", L"" },
    { SCE_P_WORD2, IDS_LEX_STR_63260, L"Keyword 2nd", L"bold; italic; fore:#6666FF", L"" },
    { SCE_P_IDENTIFIER, IDS_LEX_STR_63129, L"Identifier", L"", L"" },
    { MULTI_STYLE(SCE_P_COMMENTLINE,SCE_P_COMMENTBLOCK,0,0), IDS_LEX_STR_63127, L"Comment", L"fore:#808080", L"" },
    { MULTI_STYLE(SCE_P_STRING,SCE_P_STRINGEOL,SCE_P_CHARACTER,0), IDS_LEX_STR_63131, L"String", L"fore:#008000", L"" },
    { SCE_P_NUMBER, IDS_LEX_STR_63130, L"Number", L"fore:#C04000", L"" },
    { SCE_P_OPERATOR, IDS_LEX_STR_63132, L"Operator", L"fore:#B000B0", L"" },
    EDITLEXER_SENTINEL } };
