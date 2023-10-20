#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_CSS =
{
    // 1 - CSS1 Properties
    "aqua auto background background-attachment background-color background-image background-position background-repeat baseline "
    "black blink block blue bold bolder border border-bottom border-bottom-width border-color border-left border-left-width "
    "border-right border-right-width border-style border-top border-top-width border-width both bottom capitalize center circle "
    "clear color dashed decimal disc display dotted double fixed float font font-family font-size font-style font-variant "
    "font-weight fuchsia gray green groove height inline inset inside italic justify large larger left letter-spacing lime "
    "line-height line-through list-item list-style list-style-image list-style-position list-style-type lower-alpha lower-roman "
    "lowercase margin margin-bottom margin-left margin-right margin-top maroon medium medium middle navy no-repeat no-wrap none "
    "normal oblique olive outset outside overline padding padding-bottom padding-left padding-right padding-top pre purple red "
    "repeat repeat-x repeat-y ridge right scroll silver small small-caps smaller solid square sub super teal text-align "
    "text-bottom text-decoration text-indent text-top text-transform thick thin top transparent underline upper-alpha upper-roman "
    "uppercase vertical-align white white-space width word-spacing x-large x-small xx-large xx-small yellow",

    // 2 - Pseudo-classes or more precisely single colon ':' selectors
    "link active visited "
    "first-child focus hover lang left right first "
    "empty enabled disabled checked not root target only-child last-child nth-child nth-last-child "
    "first-of-type last-of-type nth-of-type nth-last-of-type only-of-type valid invalid required optional "
    "first-letter first-line before after",

    // 3 - CSS2 Properties
    "above absolute ActiveBorder ActiveCaption always AppWorkspace armenian ascent avoid azimuth Background baseline bbox behind "
    "below bidi-override border-bottom-color border-bottom-style border-collapse border-color border-left-color border-left-style "
    "border-right-color border-right-style border-spacing border-style border-top-color border-top-style bottom ButtonFace "
    "ButtonHighlight ButtonShadow ButtonText cap-height caption caption-side CaptionText center-left center-right centerline child "
    "cjk-ideographic clip close-quote code collapse compact condensed content continuous, counter-increment counter-reset crop "
    "cross crosshair cue cue-after cue-before cursor decimal-leading-zero default definition-src descent digits direction e-resize "
    "elevation embed empty-cells expanded extra-condensed extra-expanded far-left far-right fast faster female fixed "
    "font-size-adjust font-stretch georgian GrayText hebrew help hidden hide high higher Highlight HighlightText hiragana "
    "hiragana-iroha icon InactiveBorder InactiveCaption InactiveCaptionText InfoBackground InfoText inherit inline-table katakana "
    "katakana-iroha landscape left left-side leftwards level loud low lower lower-greek lower-latin ltr male marker marker-offset "
    "marks mathline max-height max-width medium medium menu Menu MenuText message-box min-height min-width mix move n-resize "
    "narrower ne-resize no-close-quote no-open-quote nw-resize once open-quote orphans outline outline-color outline-style "
    "outline-width overflow page page-break-after page-break-before page-break-inside panose-1 pause pause-after pause-before "
    "pitch pitch-range play-during pointer portrait position quotes relative richness right right-side rightwards rtl run-in "
    "s-resize scroll Scrollbar se-resize semi-condensed semi-expanded separate show silent size slope slow slower small-caption "
    "soft speak speak-header speak-numeral speak-punctuation speech-rate spell-out src static status-bar stemh stemv stress "
    "sw-resize table table-caption table-cell table-column table-column-group table-footer-group table-header-group table-layout "
    "table-row table-row-group text text-shadow ThreeDDarkShadow ThreeDFace ThreeDHighlight ThreeDLightShadow ThreeDShadow top "
    "topline ultra-condensed ultra-expanded unicode-bidi unicode-range units-per-em upper-latin visibility visible voice-family "
    "volume w-resize wait wider widows widths Window WindowFrame WindowText x-fast x-height x-high x-loud x-low x-slow x-soft "
    "z-index",

    // 4 - CSS3 Properties
    "background-size border-radius border-top-right-radius border-bottom-right-radius border-bottom-left-radius border-top-left-radius "
    "box-shadow columns column-width column-count column-rule column-gap column-rule-color column-rule-style column-rule-width "
    "resize opacity word-wrap",

    // 5 - Pseudo-elements or more precisely double colon '::' selectors
    "first-letter first-line before after selection",

    // 6 - Browser-Specific CSS Properties (properties and pseudos starting with -moz- etc.)
    "^-moz- ^-webkit- ^-o- ^-ms- filter",

    // 7 - Browser-Specific Pseudo-classes
    "indeterminate default ^-moz- ^-webkit- ^-o- ^-ms-",

    // 8 - Browser-Specific Pseudo-elements
    "^-moz- ^-webkit- ^-o- ^-ms-",

    NULL,
};


EDITLEXER lexCSS =
{
    SCLEX_CSS, "css", IDS_LEX_CSS_STYLE, L"CSS Style Sheets", L"css; less; hss; sass; scss", L"",
    &KeyWords_CSS, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_CSS_DEFAULT}, IDS_LEX_STR_Default, L"CSS Default", L"", L"" },
        { {SCE_CSS_COMMENT}, IDS_LEX_STR_Comment, L"Comment", L"fore:#646464", L"" },
        { {SCE_CSS_TAG}, IDS_LEX_STR_63136, L"HTML Tag", L"bold; fore:#0A246A", L"" },
        { {SCE_CSS_CLASS}, IDS_LEX_STR_63194, L"Tag-Class", L"fore:#648000", L"" },
        { {SCE_CSS_ID}, IDS_LEX_STR_63195, L"Tag-ID", L"fore:#648000", L"" },
        { {SCE_CSS_ATTRIBUTE}, IDS_LEX_STR_63196, L"Tag-Attribute", L"italic; fore:#648000", L"" },
        { {MULTI_STYLE(SCE_CSS_PSEUDOCLASS,SCE_CSS_EXTENDED_PSEUDOCLASS,0,0)}, IDS_LEX_STR_63197, L"Pseudo-Class", L"fore:#B000B0", L"" },
        { {MULTI_STYLE(SCE_CSS_PSEUDOELEMENT,SCE_CSS_EXTENDED_PSEUDOELEMENT,0,0)}, IDS_LEX_STR_63302, L"Pseudo-Element", L"fore:#B00050", L"" },
        { {MULTI_STYLE(SCE_CSS_IDENTIFIER,SCE_CSS_IDENTIFIER2,SCE_CSS_IDENTIFIER3,SCE_CSS_EXTENDED_IDENTIFIER)}, IDS_LEX_STR_63199, L"CSS Property", L"fore:#FF4000", L"" },
        { {MULTI_STYLE(SCE_CSS_DOUBLESTRING,SCE_CSS_SINGLESTRING,0,0)}, IDS_LEX_STR_String, L"String", L"fore:#008000", L"" },
        { {SCE_CSS_VALUE}, IDS_LEX_STR_Value, L"Value", L"fore:#3A6EA5", L"" },
        { {SCE_CSS_OPERATOR}, IDS_LEX_STR_Operator, L"Operator", L"fore:#B000B0", L"" },
        { {SCE_CSS_IMPORTANT}, IDS_LEX_STR_63202, L"Important", L"bold; fore:#C80000", L"" },
        { {SCE_CSS_DIRECTIVE}, IDS_LEX_STR_63203, L"Directive", L"bold; fore:#000000; back:#FFF1A8", L"" },
        { {SCE_CSS_GROUP_RULE}, IDS_LEX_STR_Media, L"Media", L"bold; fore:#0A246A", L"" },
        { {SCE_CSS_VARIABLE}, IDS_LEX_STR_Var, L"Variable", L"bold; fore:#FF4000", L"" },
        { {SCE_CSS_UNKNOWN_PSEUDOCLASS}, IDS_LEX_STR_63198, L"Unknown Pseudo-Class", L"fore:#C80000; back:#FFFF80", L"" },
        { {SCE_CSS_UNKNOWN_IDENTIFIER}, IDS_LEX_STR_63200, L"Unknown Property", L"fore:#C80000; back:#FFFF80", L"" },
        EDITLEXER_SENTINEL
    }
};
