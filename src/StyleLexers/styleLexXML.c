#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_XML = EMPTY_KEYWORDLIST;


EDITLEXER lexXML =
{
    SCLEX_XML, "xml", IDS_LEX_XML_DOC, L"XML Document", L"xml; xsl; rss; svg; xul; xsd; xslt; axl; rdf; xaml; vcproj; ffs_gui; nzb; resx; plist; xrc; fbp; manifest", L"",
    &KeyWords_XML, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {MULTI_STYLE(SCE_H_TAG,SCE_H_TAGUNKNOWN,SCE_H_TAGEND,0)}, IDS_LEX_STR_63187, L"XML Tag", L"fore:#881280", L"" },
        { {MULTI_STYLE(SCE_H_ATTRIBUTE,SCE_H_ATTRIBUTEUNKNOWN,0,0)}, IDS_LEX_STR_63188, L"XML Attribute", L"fore:#994500", L"" },
        { {SCE_H_VALUE}, IDS_LEX_STR_63189, L"XML Value", L"fore:#1A1AA6", L"" },
        { {MULTI_STYLE(SCE_H_DOUBLESTRING,SCE_H_SINGLESTRING,0,0)}, IDS_LEX_STR_63190, L"XML String", L"fore:#1A1AA6", L"" },
        { {SCE_H_OTHER}, IDS_LEX_STR_63191, L"XML Other Inside Tag", L"fore:#1A1AA6", L"" },
        { {MULTI_STYLE(SCE_H_COMMENT,SCE_H_XCCOMMENT,0,0)}, IDS_LEX_STR_63192, L"XML Comment", L"fore:#646464", L"" },
        { {SCE_H_ENTITY}, IDS_LEX_STR_63193, L"XML Entity", L"fore:#B000B0", L"" },
        { {SCE_H_DEFAULT}, IDS_LEX_STR_63257, L"XML Element Text", L"", L"" },
        { {MULTI_STYLE(SCE_H_XMLSTART,SCE_H_XMLEND,0,0)}, IDS_LEX_STR_63145, L"XML Identifier", L"bold; fore:#881280", L"" },
        { {SCE_H_SGML_DEFAULT}, IDS_LEX_STR_63237, L"SGML", L"fore:#881280", L"" },
        { {SCE_H_CDATA}, IDS_LEX_STR_63147, L"CDATA", L"fore:#646464", L"" },
        EDITLEXER_SENTINEL
    }
};

