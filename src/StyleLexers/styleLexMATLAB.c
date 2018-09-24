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
KEYWORDLIST KeyWords_MATLAB = {
"break case catch continue else elseif end for function global if otherwise "
"persistent return switch try while ",
"", "", "", "", "", "", "", "" };


EDITLEXER lexMATLAB = { 
SCLEX_MATLAB, IDS_LEX_MATLAB, L"MATLAB", L"matlab", L"", 
&LexFunction, // static
&KeyWords_MATLAB, {
    { STYLE_DEFAULT, IDS_LEX_STR_63126, L"Default", L"", L"" },
    //{ SCE_MATLAB_DEFAULT, IDS_LEX_STR_63126, L"Default", L"", L"" },
    { SCE_MATLAB_COMMENT, IDS_LEX_STR_63127, L"Comment", L"fore:#008000", L"" },
    { SCE_MATLAB_COMMAND, IDS_LEX_STR_63236, L"Command", L"bold", L"" },
    { SCE_MATLAB_NUMBER, IDS_LEX_STR_63130, L"Number", L"fore:#FF8000", L"" },
    { SCE_MATLAB_KEYWORD, IDS_LEX_STR_63128, L"Keyword", L"fore:#00007F; bold", L"" },
    { MULTI_STYLE(SCE_MATLAB_STRING,SCE_MATLAB_DOUBLEQUOTESTRING,0,0), IDS_LEX_STR_63131, L"String", L"fore:#7F007F", L"" },
    { SCE_MATLAB_OPERATOR, IDS_LEX_STR_63132, L"Operator", L"", L"" },
    { SCE_MATLAB_IDENTIFIER, IDS_LEX_STR_63129, L"Identifier", L"", L"" },
    EDITLEXER_SENTINEL } };
