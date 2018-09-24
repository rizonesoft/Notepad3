#include "StyleLexers.h"

// ----------------------------------------------------------------------------

static int LexFunction(LexFunctionType type, int value)
{
  static bool bStyleChanged = false;

  switch (type) {
  case FCT_SETTING_CHANGE:
    if (value < 0)
      return (bStyleChanged ? 1 : 0);
    else {
      bStyleChanged = (value > 0);
      return 1;
    }

  default:
    break;
  }
  return 0;
};

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_MARKDOWN = EMPTY_KEYWORDLIST;


EDITLEXER lexMARKDOWN = { 
SCLEX_MARKDOWN, IDS_LEX_MARKDOWN, L"Markdown", L"md; markdown; mdown; mkdn; mkd", L"", 
&LexFunction, // static
&KeyWords_MARKDOWN, {
    { STYLE_DEFAULT, IDS_LEX_STR_63126, L"Default", L"", L"" },
    //{ SCE_MARKDOWN_DEFAULT, IDS_LEX_STR_63126, L"Default", L"", L"" },
    { SCE_MARKDOWN_LINE_BEGIN, IDS_LEX_STR_63317, L"Line Begin", L"", L"" },
    { MULTI_STYLE(SCE_MARKDOWN_STRONG1,SCE_MARKDOWN_STRONG2,0,0), IDS_LEX_STR_63318, L"Strong", L"bold", L"" },
    { MULTI_STYLE(SCE_MARKDOWN_EM1,SCE_MARKDOWN_EM2,0,0), IDS_LEX_STR_63319, L"Emphasis", L"italic", L"" },
    { SCE_MARKDOWN_HEADER1, IDS_LEX_STR_63320, L"Header 1", L"fore:#FF0088; bold", L"" },
    { SCE_MARKDOWN_HEADER2, IDS_LEX_STR_63321, L"Header 2", L"fore:#FF0088; bold", L"" },
    { SCE_MARKDOWN_HEADER3, IDS_LEX_STR_63322, L"Header 3", L"fore:#FF0088; bold", L"" },
    { SCE_MARKDOWN_HEADER4, IDS_LEX_STR_63323, L"Header 4", L"fore:#FF0088; bold", L"" },
    { SCE_MARKDOWN_HEADER5, IDS_LEX_STR_63324, L"Header 5", L"fore:#FF0088; bold", L"" },
    { SCE_MARKDOWN_HEADER6, IDS_LEX_STR_63325, L"Header 6", L"fore:#FF0088; bold", L"" },
    { SCE_MARKDOWN_PRECHAR, IDS_LEX_STR_63326, L"Pre Char", L"fore:#00007F", L"" },
    { SCE_MARKDOWN_ULIST_ITEM, IDS_LEX_STR_63327, L"Unordered List", L"fore:#0080FF; bold", L"" },
    { SCE_MARKDOWN_OLIST_ITEM, IDS_LEX_STR_63268, L"Ordered List", L"fore:#0080FF; bold", L"" },
    { SCE_MARKDOWN_BLOCKQUOTE, IDS_LEX_STR_63328, L"Block Quote", L"fore:#00007F", L"" },
    { SCE_MARKDOWN_STRIKEOUT, IDS_LEX_STR_63329, L"Strikeout", L"", L"" },
    { SCE_MARKDOWN_HRULE, IDS_LEX_STR_63330, L"Horizontal Rule", L"bold", L"" },
    { SCE_MARKDOWN_LINK, IDS_LEX_STR_63331, L"Link", L"fore:#0000FF", L"" },
    { MULTI_STYLE(SCE_MARKDOWN_CODE,SCE_MARKDOWN_CODE2,SCE_MARKDOWN_CODEBK,0), IDS_LEX_STR_63332, L"Code", L"fore:#00007F; back:#EBEBEB", L"" },
    EDITLEXER_SENTINEL } };
