#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_NULL = EMPTY_KEYWORDLIST;

EDITLEXER lexStandard = { 
SCLEX_NULL, IDS_LEX_DEF_TXT, L"Common Base", L"", L"", 
&KeyWords_NULL, {
    /*  0 */ { {STYLE_DEFAULT}, IDS_LEX_STD_STYLE, L"Default Style", L"font:Default", L"" },
    /*  1 */ { {STYLE_LINENUMBER}, IDS_LEX_STD_MARGIN, L"Margins and Line Numbers", L"size:-2; fore:#008080", L"" },
    /*  2 */ { {STYLE_BRACELIGHT}, IDS_LEX_STD_BRACE, L"Matching Braces (Indicator)", L"fore:#00FF40; alpha:80; alpha2:80; indic_roundbox", L"" },
    /*  3 */ { {STYLE_BRACEBAD}, IDS_LEX_STD_BRACE_FAIL, L"Matching Braces Error (Indicator)", L"fore:#FF0080; alpha:140; alpha2:140; indic_roundbox", L"" },
    /*  4 */ { {STYLE_CONTROLCHAR}, IDS_LEX_STD_CTRL_CHAR, L"Control Characters (Font)", L"size:-1", L"" },
    /*  5 */ { {STYLE_INDENTGUIDE}, IDS_LEX_STD_INDENT, L"Indentation Guide (Color)", L"fore:#A0A0A0", L"" },
    /*  6 */ { {_STYLE_GETSTYLEID(STY_SEL_TXT)}, IDS_LEX_STD_SEL, L"Selected Text (Colors)", L"fore:#FF4000; back:#4040FF; eolfilled; alpha:80", L"" },
    /*  7 */ { {_STYLE_GETSTYLEID(STY_WHITESPACE)}, IDS_LEX_STD_WSPC, L"Whitespace (Colors, Size 0-12)", L"fore:#FF4000", L"" },
    /*  8 */ { {_STYLE_GETSTYLEID(STY_CUR_LN)}, IDS_LEX_STD_LN_BACKGR, L"Current Line Background (Color)", L"size:2; fore:#A0A0A0; back:#FFFF00; alpha:50", L"" },
    /*  9 */ { {_STYLE_GETSTYLEID(STY_CARET)}, IDS_LEX_STD_CARET, L"Caret (Color, Size 1-3)", L"", L"" },
    /* 10 */ { {_STYLE_GETSTYLEID(STY_LONG_LN_MRK)}, IDS_LEX_STD_LONG_LN, L"Long Line Marker (Colors)", L"fore:#FFC000", L"" },
    /* 11 */ { {_STYLE_GETSTYLEID(STY_X_LN_SPACE)}, IDS_LEX_STD_X_SPC, L"Extra Line Spacing (Size)", L"size:2", L"" },
    /* 12 */ { {_STYLE_GETSTYLEID(STY_BOOK_MARK)}, IDS_LEX_STD_BKMRK, L"Bookmarks and Folding (Colors, Size)", L"size:+2; fore:#000000; back:#00DC00; alpha:100", L"" },
    /* 13 */ { {_STYLE_GETSTYLEID(STY_MARK_OCC)}, IDS_LEX_STR_63262, L"Mark Occurrences (Indicator)", L"fore:#3399FF; alpha:60; alpha2:60; indic_roundbox", L"" },
    /* 14 */ { {_STYLE_GETSTYLEID(STY_URL_HOTSPOT)}, IDS_LEX_STR_63264, L"Hyperlink Hotspots", L"fore:#0000FF; back:#0000BF; indic_plain", L"" },
    /* 15 */ { {_STYLE_GETSTYLEID(STY_UNICODE_HOTSPOT)}, IDS_LEX_STR_63367, L"Unicode-Point Hover", L"fore:#00FA00; alpha:60; alpha2:180; indic_compositionthick", L""},
    /* 16 */ { {_STYLE_GETSTYLEID(STY_MULTI_EDIT)}, IDS_LEX_STR_63354, L"Multi Edit Indicator", L"fore:#FFA000; alpha:60; alpha2:180; indic_roundbox", L"" },
    /* 17 */ { {_STYLE_GETSTYLEID(STY_IME_COLOR)}, IDS_LEX_STR_63352, L"Inline-IME Color", L"fore:#00AA00", L"" },
             EDITLEXER_SENTINEL } };


EDITLEXER lexStandard2nd = { 
SCLEX_NULL, IDS_LEX_STR_63266, L"2nd Common Base", L"", L"", 
&KeyWords_NULL,{
    /*  0 */ { {STYLE_DEFAULT}, IDS_LEX_2ND_STYLE, L"2nd Default Style", L"font:Courier New", L"" },
    /*  1 */ { {STYLE_LINENUMBER}, IDS_LEX_2ND_MARGIN, L"2nd Margins and Line Numbers", L"font:Courier New; size:-2; fore:#008080", L"" },
    /*  2 */ { {STYLE_BRACELIGHT}, IDS_LEX_2ND_BRACE, L"2nd Matching Braces (Indicator)", L"fore:#00FF40; alpha:80; alpha2:220; indic_roundbox", L"" },
    /*  3 */ { {STYLE_BRACEBAD}, IDS_LEX_2ND_BRACE_FAIL, L"2nd Matching Braces Error (Indicator)", L"fore:#FF0080; alpha:140; alpha2:220; indic_roundbox", L"" },
    /*  4 */ { {STYLE_CONTROLCHAR}, IDS_LEX_2ND_CTRL_CHAR, L"2nd Control Characters (Font)", L"size:-1", L"" },
    /*  5 */ { {STYLE_INDENTGUIDE}, IDS_LEX_2ND_INDENT, L"2nd Indentation Guide (Color)", L"fore:#A0A0A0", L"" },
    /*  6 */ { {_STYLE_GETSTYLEID(STY_SEL_TXT)}, IDS_LEX_2ND_SEL, L"2nd Selected Text (Colors)", L"fore:#FF4000; eolfilled", L"" },
    /*  7 */ { {_STYLE_GETSTYLEID(STY_WHITESPACE)}, IDS_LEX_2ND_WSPC, L"2nd Whitespace (Colors, Size 0-12)", L"fore:#FF4000", L"" },
    /*  8 */ { {_STYLE_GETSTYLEID(STY_CUR_LN)}, IDS_LEX_2ND_LN_BACKGR, L"2nd Current Line Background (Color)", L"size:2; fore:#0000B0; back:#FFFF00; alpha:50", L"" },
    /*  9 */ { {_STYLE_GETSTYLEID(STY_CARET)}, IDS_LEX_2ND_CARET, L"2nd Caret (Color, Size 1-3)", L"", L"" },
    /* 10 */ { {_STYLE_GETSTYLEID(STY_LONG_LN_MRK)}, IDS_LEX_2ND_LONG_LN, L"2nd Long Line Marker (Colors)", L"fore:#FFC000", L"" },
    /* 11 */ { {_STYLE_GETSTYLEID(STY_X_LN_SPACE)}, IDS_LEX_2ND_X_SPC, L"2nd Extra Line Spacing (Size)", L"", L"" },
    /* 12 */ { {_STYLE_GETSTYLEID(STY_BOOK_MARK)}, IDS_LEX_2ND_BKMRK, L"2nd Bookmarks and Folding (Colors, Size)", L"size:+2; fore:#000000; back:#00DC00; charset:2; case:U; alpha:100", L"" },
    /* 13 */ { {_STYLE_GETSTYLEID(STY_MARK_OCC)}, IDS_LEX_STR_63263, L"2nd Mark Occurrences (Indicator)", L"fore:#0000FF; alpha:60; alpha2:60; indic_box", L"" },
    /* 14 */ { {_STYLE_GETSTYLEID(STY_URL_HOTSPOT)}, IDS_LEX_STR_63265, L"2nd Hyperlink Hotspots", L"fore:#00D000; back:#009C00; alpha:180; indic_compositionthin", L"" },
    /* 15 */ {{_STYLE_GETSTYLEID(STY_UNICODE_HOTSPOT)}, IDS_LEX_STR_63368, L"2nd Unicode-Point Hover", L"fore:#0000FA; alpha:60; alpha2:180; indic_compositionthick", L""},
    /* 16 */ {{_STYLE_GETSTYLEID(STY_MULTI_EDIT)}, IDS_LEX_STR_63355, L"2nd Multi Edit Indicator", L"fore:#00A5FF; indic_box", L""},
    /* 17 */ { {_STYLE_GETSTYLEID(STY_IME_COLOR)}, IDS_LEX_STR_63353, L"2nd Inline-IME Color", L"fore:#FF0000", L"" },
             EDITLEXER_SENTINEL } };


EDITLEXER lexTEXT = {
SCLEX_NULL, IDS_LEX_TEXT_FILES, L"Text Files", L"txt; text; tmp; log; asc; doc; wtx", L"",
&KeyWords_NULL,{
    { {STYLE_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
    { {STYLE_LINENUMBER}, IDS_LEX_STD_MARGIN, L"Margins and Line Numbers", L"font:Lucida Console; size:-2", L"" },
    { {SCI_SETEXTRAASCENT + SCI_SETEXTRADESCENT}, IDS_LEX_STD_X_SPC, L"Extra Line Spacing (Size)", L"size:-1", L"" },
    EDITLEXER_SENTINEL } };


EDITLEXER lexANSI = { 
SCLEX_NULL, IDS_LEX_ANSI_ART, L"ANSI Art", L"nfo; diz; \\^Readme$", L"", 
&KeyWords_NULL,{
    { {STYLE_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"font:Lucida Console; size:11; thin; smoothing:none", L"" },
    { {STYLE_LINENUMBER}, IDS_LEX_STD_MARGIN, L"Margins and Line Numbers", L"font:Lucida Console; size:-2", L"" },
    { {STYLE_BRACELIGHT}, IDS_LEX_STD_BRACE, L"Matching Braces", L"", L"" },
    { {STYLE_BRACEBAD}, IDS_LEX_STD_BRACE_FAIL, L"Matching Braces Error", L"", L"" },
    { {SCI_SETEXTRAASCENT + SCI_SETEXTRADESCENT}, IDS_LEX_STD_X_SPC, L"Extra Line Spacing (Size)", L"size:-1", L"" },
    EDITLEXER_SENTINEL } };

