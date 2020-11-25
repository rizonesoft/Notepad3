#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_CSS =
{
    "align-content align-items align-self alignment-adjust alignment-baseline animation animation-delay "
    "animation-direction animation-duration animation-fill-mode animation-iteration-count animation-name "
    "animation-play-state animation-timing-function appearance ascent azimuth backface-visibility background "
    "background-attachment background-blend-mode background-break background-clip background-color "
    "background-image background-origin background-position background-repeat background-size baseline "
    "baseline-shift bbox binding bleed bookmark-label bookmark-level bookmark-state bookmark-target border "
    "border-bottom border-bottom-color border-bottom-left-radius border-bottom-right-radius "
    "border-bottom-style border-bottom-width border-collapse border-color border-image border-image-outset "
    "border-image-repeat border-image-slice border-image-source border-image-width border-left "
    "border-left-color border-left-style border-left-width border-length border-radius border-right "
    "border-right-color border-right-style border-right-width border-spacing border-style border-top "
    "border-top-color border-top-left-radius border-top-right-radius border-top-style border-top-width "
    "border-width bottom box-align box-decoration-break box-direction box-flex box-flex-group box-lines "
    "box-ordinal-group box-orient box-pack box-shadow box-sizing break-after break-before break-inside "
    "cap-height caption-side centerline change-bar change-bar-class change-bar-offset change-bar-side clear "
    "clip clip-path clip-rule color color-profile column-count column-fill column-gap column-rule "
    "column-rule-color column-rule-style column-rule-width column-span column-width columns content "
    "counter-increment counter-reset crop cue cue-after cue-before cursor definition-src descent direction "
    "display dominant-baseline drop-initial-after-adjust drop-initial-after-align drop-initial-before-adjust "
    "drop-initial-before-align drop-initial-size drop-initial-value elevation empty-cells fill fit "
    "fit-position flex flex-basis flex-direction flex-flow flex-grow flex-shrink flex-wrap float float-offset "
    "flow-from flow-into font font-family font-feature-settings font-kerning font-size font-size-adjust "
    "font-stretch font-style font-synthesis font-variant font-weight grid-columns grid-rows "
    "hanging-punctuation height hyphenate-after hyphenate-before hyphenate-character hyphenate-limit-chars "
    "hyphenate-limit-last hyphenate-limit-zone hyphenate-lines hyphenate-resource hyphens icon "
    "image-orientation image-resolution ime-mode inline-box-align insert-position interpret-as justify-content "
    "left letter-spacing line-height line-stacking line-stacking-ruby line-stacking-shift "
    "line-stacking-strategy list-style list-style-image list-style-position list-style-type make-element "
    "margin margin-bottom margin-left margin-right margin-top mark mark-after mark-before marker-offset marks "
    "marquee-direction marquee-play-count marquee-speed marquee-style mask mask-type mathline max-height "
    "max-width media min-height min-width move-to nav-down nav-index nav-left nav-right nav-up object-fit "
    "object-position opacity order orphans outline outline-color outline-offset outline-style outline-width "
    "overflow overflow-style overflow-wrap overflow-x overflow-y padding padding-bottom padding-left "
    "padding-right padding-top page page-break-after page-break-before page-break-inside page-policy panose-1 "
    "pause pause-after pause-before perspective perspective-origin phonemes pitch pitch-range play-during "
    "pointer-events position presentation-level prototype prototype-insert-policy prototype-insert-position "
    "punctuation-trim quotes region-overflow rendering-intent resize rest rest-after rest-before richness "
    "right rotation rotation-point ruby-align ruby-overhang ruby-position ruby-span shape-image-threshold "
    "shape-inside shape-outside size slope speak speak-header speak-numeral speak-punctuation speech-rate src "
    "stemh stemv stress string-set tab-size table-layout target target-name target-new target-position "
    "text-align text-align-last text-decoration text-decoration-color text-decoration-line "
    "text-decoration-style text-emphasis text-height text-indent text-justify text-outline text-overflow "
    "text-rendering text-replace text-shadow text-transform text-underline-position text-wrap top topline "
    "transform transform-origin transform-style transition transition-delay transition-duration "
    "transition-property transition-timing-function unicode-bidi unicode-range units-per-em vertical-align "
    "visibility voice-balance voice-duration voice-family voice-pitch voice-pitch-range voice-rate "
    "voice-stress voice-volume volume white-space white-space-collapse widows width widths will-change "
    "word-break word-spacing word-wrap wrap wrap-flow wrap-margin wrap-padding wrap-through writing-mode "
    "x-height z-index",
    "active after before checked choices default disabled empty enabled first first-child first-letter "
    "first-line first-of-type focus hover in-range indeterminate invalid lang last-child last-of-type left "
    "link not nth-child nth-last-child nth-last-of-type nth-of-type only-child only-of-type optional "
    "out-of-range read-only read-write repeat-index repeat-item required right root target valid visited",
    "", "",
// Word List Descriptions
    "after before first-letter first-line selection",
    "^-moz- ^-ms- ^-o- ^-webkit-",
    "^-moz- ^-ms- ^-o- ^-webkit-",
    "^-moz- ^-ms- ^-o- ^-webkit-",
    NULL,
};


EDITLEXER lexCSS =
{
    SCLEX_CSS, "css", IDS_LEX_CSS_STYLE, L"CSS Style Sheets", L"css; less; hss; sass; scss", L"",
    &KeyWords_CSS, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
        //{ {SCE_CSS_DEFAULT}, IDS_LEX_STR_63126, L"CSS Default", L"", L"" },
        { {SCE_CSS_COMMENT}, IDS_LEX_STR_63127, L"Comment", L"fore:#646464", L"" },
        { {SCE_CSS_TAG}, IDS_LEX_STR_63136, L"HTML Tag", L"bold; fore:#0A246A", L"" },
        { {SCE_CSS_CLASS}, IDS_LEX_STR_63194, L"Tag-Class", L"fore:#648000", L"" },
        { {SCE_CSS_ID}, IDS_LEX_STR_63195, L"Tag-ID", L"fore:#648000", L"" },
        { {SCE_CSS_ATTRIBUTE}, IDS_LEX_STR_63196, L"Tag-Attribute", L"italic; fore:#648000", L"" },
        { {MULTI_STYLE(SCE_CSS_PSEUDOCLASS,SCE_CSS_EXTENDED_PSEUDOCLASS,0,0)}, IDS_LEX_STR_63197, L"Pseudo-Class", L"fore:#B000B0", L"" },
        { {MULTI_STYLE(SCE_CSS_PSEUDOELEMENT,SCE_CSS_EXTENDED_PSEUDOELEMENT,0,0)}, IDS_LEX_STR_63302, L"Pseudo-Element", L"fore:#B00050", L"" },
        { {MULTI_STYLE(SCE_CSS_IDENTIFIER,SCE_CSS_IDENTIFIER2,SCE_CSS_IDENTIFIER3,SCE_CSS_EXTENDED_IDENTIFIER)}, IDS_LEX_STR_63199, L"CSS Property", L"fore:#FF4000", L"" },
        { {MULTI_STYLE(SCE_CSS_DOUBLESTRING,SCE_CSS_SINGLESTRING,0,0)}, IDS_LEX_STR_63131, L"String", L"fore:#008000", L"" },
        { {SCE_CSS_VALUE}, IDS_LEX_STR_63201, L"Value", L"fore:#3A6EA5", L"" },
        { {SCE_CSS_OPERATOR}, IDS_LEX_STR_63132, L"Operator", L"fore:#B000B0", L"" },
        { {SCE_CSS_IMPORTANT}, IDS_LEX_STR_63202, L"Important", L"bold; fore:#C80000", L"" },
        { {SCE_CSS_DIRECTIVE}, IDS_LEX_STR_63203, L"Directive", L"bold; fore:#000000; back:#FFF1A8", L"" },
        { {SCE_CSS_MEDIA}, IDS_LEX_STR_63303, L"Media", L"bold; fore:#0A246A", L"" },
        { {SCE_CSS_VARIABLE}, IDS_LEX_STR_63249, L"Variable", L"bold; fore:#FF4000", L"" },
        { {SCE_CSS_UNKNOWN_PSEUDOCLASS}, IDS_LEX_STR_63198, L"Unknown Pseudo-Class", L"fore:#C80000; back:#FFFF80", L"" },
        { {SCE_CSS_UNKNOWN_IDENTIFIER}, IDS_LEX_STR_63200, L"Unknown Property", L"fore:#C80000; back:#FFFF80", L"" },
        EDITLEXER_SENTINEL
    }
};


