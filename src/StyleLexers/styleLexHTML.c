#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_HTML =
{
// HTML elements and attributes
    "!doctype ^aria- ^data- a abbr accept accept-charset accesskey acronym action address align alink alt and "
    "applet archive area article aside async audio autocomplete autofocus autoplay axis b background base "
    "basefont bb bdi bdo bgcolor big blockquote body border bordercolor br buffered button canvas caption "
    "cellpadding cellspacing center challenge char charoff charset checkbox checked cite class classid clear "
    "code codebase codetype col colgroup color cols colspan command compact content contenteditable "
    "contextmenu controls coords crossorigin data datafld dataformatas datagrid datalist datapagesize datasrc "
    "datetime dd declare default defer del details dfn dialog dir dirname disabled div dl download draggable "
    "dropzone dt em embed enctype event eventsource face fieldset figcaption figure file font footer for form "
    "formaction formenctype formmethod formnovalidate formtarget frame frameborder frameset h1 h2 h3 h4 h5 h6 "
    "head header headers height hgroup hidden high hr href hreflang hspace html http-equiv i icon id iframe "
    "image img input ins integrity isindex ismap itemprop itemscope itemtype kbd keygen keytype kind label "
    "lang language leftmargin legend li link list longdesc loop low main manifest map marginheight marginwidth "
    "mark max maxlength media mediagroup menu menuitem meta meter method min multiple muted name nav noframes "
    "nohref noresize noscript noshade novalidate nowrap object ol onabort onafterprint onbeforeprint "
    "onbeforeunload onblur oncancel oncanplay oncanplaythrough onchange onclick onclose oncontextmenu "
    "oncuechange ondblclick ondrag ondragend ondragenter ondragleave ondragover ondragstart ondrop "
    "ondurationchange onemptied onended onerror onfocus onformchange onforminput onhashchange oninput "
    "oninvalid onkeydown onkeypress onkeyup onload onloadeddata onloadedmetadata onloadstart onmessage "
    "onmousedown onmousemove onmouseout onmouseover onmouseup onmousewheel onoffline ononline onpagehide "
    "onpageshow onpause onplay onplaying onpopstate onprogress onratechange onreadystatechange onredo onreset "
    "onresize onscroll onseeked onseeking onselect onshow onstalled onstorage onsubmit onsuspend ontimeupdate "
    "onundo onunload onvolumechange onwaiting open optgroup optimum option output p param password pattern "
    "picture ping placeholder poster pre prefix preload profile progress prompt property pubdate public q "
    "radio radiogroup readonly rel required reset rev reversed role rows rowspan rp rt ruby rules s samp "
    "sandbox scheme scope scoped script scrolling seamless section select selected shape size sizes slot small "
    "source span spellcheck src srcdoc srclang standby start step strike strong style sub submit summary sup "
    "tabindex table target tbody td template text textarea tfoot th thead time title topmargin tr track "
    "translate tt type typemustmatch u ul usemap valign value valuetype var version video vlink vspace wbr "
    "width wrap xml xmlns",
// JavaScript keywords (sync with lexJS::KeyWords_JS)
    NP3_LEXER_JS_KEYWORD_LIST,
// VBScript keywords
    NP3_LEXER_VB_KEYWORD_LIST,
// Python keywords
    "",
// PHP keywords
    "__callstatic __class__ __compiler_halt_offset__ __dir__ __file__ __function__ __get __halt_compiler __isset "
    "__line__ __method__ __namespace__ __set __sleep __trait__ __unset __wakeup abstract and argc argv array "
    "as break callable case catch cfunction class clone closure const continue declare default define die "
    "directory do e_all e_compile_error e_compile_warning e_core_error e_core_warning e_deprecated e_error "
    "e_fatal e_notice e_parse e_strict e_user_deprecated e_user_error e_user_notice e_user_warning e_warning "
    "echo else elseif empty enddeclare endfor endforeach endif endswitch endwhile eval exception exit extends "
    "false final for foreach function global goto http_cookie_vars http_env_vars http_get_vars http_post_files "
    "http_post_vars http_server_vars if implements include include_once instanceof insteadof interface isset "
    "list namespace new not null old_function or parent php_self print private protected public require "
    "require_once return static stdclass switch this throw trait true try unset use var virtual while xor",
// SGML and DTD keywords
    "",
// CSS properties (for embedded <style> highlighting)
    "above absolute activeborder activecaption always appworkspace aqua armenian ascent auto avoid azimuth "
    "background background-attachment background-color background-image background-position background-repeat "
    "background-size baseline bbox behind below bidi-override black blink block blue bold bolder border "
    "border-bottom border-bottom-color border-bottom-left-radius border-bottom-right-radius border-bottom-style "
    "border-bottom-width border-collapse border-color border-left border-left-color border-left-style "
    "border-left-width border-radius border-right border-right-color border-right-style border-right-width "
    "border-spacing border-style border-top border-top-color border-top-left-radius border-top-right-radius "
    "border-top-style border-top-width border-width both bottom box-shadow buttonface buttonhighlight buttonshadow "
    "buttontext cap-height capitalize caption caption-side captiontext center center-left center-right centerline "
    "child circle cjk-ideographic clear clip close-quote code collapse color column-count column-gap column-rule "
    "column-rule-color column-rule-style column-rule-width column-width columns compact condensed content "
    "counter-increment counter-reset crop cross crosshair cue cue-after cue-before cursor dashed decimal "
    "decimal-leading-zero default definition-src descent digits direction disc display dotted double e-resize "
    "elevation embed empty-cells expanded extra-condensed extra-expanded far-left far-right fast faster female "
    "fixed float font font-family font-size font-size-adjust font-stretch font-style font-variant font-weight "
    "fuchsia georgian gray graytext green groove hebrew height help hidden hide high higher highlight "
    "highlighttext hiragana hiragana-iroha icon inactiveborder inactivecaption inactivecaptiontext infobackground "
    "infotext inherit inline inline-table inset inside italic justify katakana katakana-iroha landscape large "
    "larger left left-side leftwards letter-spacing level lime line-height line-through list-item list-style "
    "list-style-image list-style-position list-style-type loud low lower lower-alpha lower-greek lower-latin "
    "lower-roman lowercase ltr male margin margin-bottom margin-left margin-right margin-top marker marker-offset "
    "marks maroon mathline max-height max-width medium menu menutext message-box middle min-height min-width mix "
    "move n-resize narrower navy ne-resize no-close-quote no-open-quote no-repeat no-wrap none normal nw-resize "
    "oblique olive once opacity open-quote orphans outline outline-color outline-style outline-width outset "
    "outside overflow overline padding padding-bottom padding-left padding-right padding-top page page-break-after "
    "page-break-before page-break-inside panose-1 pause pause-after pause-before pitch pitch-range play-during "
    "pointer portrait position pre purple quotes red relative repeat repeat-x repeat-y resize richness ridge right "
    "right-side rightwards rtl run-in s-resize scroll scrollbar se-resize semi-condensed semi-expanded separate "
    "show silent silver size slope slow slower small small-caps small-caption smaller soft solid speak "
    "speak-header speak-numeral speak-punctuation speech-rate spell-out square src static status-bar stemh stemv "
    "stress sub super sw-resize table table-caption table-cell table-column table-column-group table-footer-group "
    "table-header-group table-layout table-row table-row-group teal text text-align text-bottom text-decoration "
    "text-indent text-shadow text-top text-transform thick thin threeddarkshadow threedface threedhighlight "
    "threedlightshadow threedshadow top topline transparent ultra-condensed ultra-expanded underline unicode-bidi "
    "unicode-range units-per-em upper-alpha upper-latin upper-roman uppercase vertical-align visibility visible "
    "voice-family volume w-resize wait white white-space wider widows width widths window windowframe windowtext "
    "word-spacing word-wrap x-fast x-height x-high x-large x-loud x-low x-slow x-small x-soft xx-large xx-small "
    "yellow z-index",
    NULL,
};


EDITLEXER lexHTML =
{
    SCLEX_HTML, "hypertext", IDS_LEX_WEB_SRC, L"Web Source Code", L"html; htm; asp; aspx; shtml; htd; xhtml; php; php3; phtml; htt; cfm; tpl; dtd; hta; htc; jsp; mht; jd", L"",
    &KeyWords_HTML, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {MULTI_STYLE(SCE_H_TAG,SCE_H_TAGEND,0,0)}, IDS_LEX_STR_63136, L"HTML Tag", L"fore:#648000", L"" },
        { {SCE_H_TAGUNKNOWN}, IDS_LEX_STR_63137, L"HTML Unknown Tag", L"fore:#C80000; back:#FFFF80", L"" },
        { {SCE_H_ATTRIBUTE}, IDS_LEX_STR_63138, L"HTML Attribute", L"fore:#FF4000", L"" },
        { {SCE_H_ATTRIBUTEUNKNOWN}, IDS_LEX_STR_63139, L"HTML Unknown Attribute", L"fore:#C80000; back:#FFFF80", L"" },
        { {SCE_H_VALUE}, IDS_LEX_STR_63140, L"HTML Value", L"fore:#3A6EA5", L"" },
        { {MULTI_STYLE(SCE_H_DOUBLESTRING,SCE_H_SINGLESTRING,0,0)}, IDS_LEX_STR_63141, L"HTML String", L"fore:#3A6EA5", L"" },
        { {SCE_H_OTHER}, IDS_LEX_STR_63142, L"HTML Other Inside Tag", L"fore:#3A6EA5", L"" },
        { {MULTI_STYLE(SCE_H_COMMENT,SCE_H_XCCOMMENT,0,0)}, IDS_LEX_STR_63143, L"HTML Comment", L"fore:#646464", L"" },
        { {SCE_H_ENTITY}, IDS_LEX_STR_63144, L"HTML Entity", L"fore:#B000B0", L"" },
        { {SCE_H_DEFAULT}, IDS_LEX_STR_63256, L"HTML Element Text", L"", L"" },
        { {MULTI_STYLE(SCE_H_XMLSTART,SCE_H_XMLEND,0,0)}, IDS_LEX_STR_63145, L"XML Identifier", L"bold; fore:#881280", L"" },
        { {SCE_H_SGML_DEFAULT}, IDS_LEX_STR_63237, L"SGML", L"fore:#881280", L"" },
        { {SCE_H_CDATA}, IDS_LEX_STR_63147, L"CDATA", L"fore:#646464", L"" },
        { {MULTI_STYLE(SCE_H_ASP,SCE_H_ASPAT,0,0)}, IDS_LEX_STR_63146, L"ASP Start Tag", L"bold; fore:#000080", L"" },
        //{ {SCE_H_SCRIPT}, L"Script", L"", L"" },
        { {SCE_H_QUESTION}, IDS_LEX_STR_63148, L"PHP Start Tag", L"bold; fore:#000080", L"" },
        { {SCE_HPHP_DEFAULT}, IDS_LEX_STR_63149, L"PHP Default", L"", L"" },
        { {MULTI_STYLE(SCE_HPHP_COMMENT,SCE_HPHP_COMMENTLINE,0,0)}, IDS_LEX_STR_63157, L"PHP Comment", L"fore:#FF8000", L"" },
        { {SCE_HPHP_WORD}, IDS_LEX_STR_63152, L"PHP Keyword", L"bold; fore:#A46000", L"" },
        { {SCE_HPHP_HSTRING}, IDS_LEX_STR_63150, L"PHP String", L"fore:#008000", L"" },
        { {SCE_HPHP_SIMPLESTRING}, IDS_LEX_STR_63151, L"PHP Simple String", L"fore:#008000", L"" },
        { {SCE_HPHP_NUMBER}, IDS_LEX_STR_63153, L"PHP Number", L"fore:#FF0000", L"" },
        { {SCE_HPHP_OPERATOR}, IDS_LEX_STR_63158, L"PHP Operator", L"fore:#B000B0", L"" },
        { {SCE_HPHP_VARIABLE}, IDS_LEX_STR_63154, L"PHP Variable", L"italic; fore:#000080", L"" },
        { {SCE_HPHP_HSTRING_VARIABLE}, IDS_LEX_STR_63155, L"PHP String Variable", L"italic; fore:#000080", L"" },
        { {SCE_HPHP_COMPLEX_VARIABLE}, IDS_LEX_STR_63156, L"PHP Complex Variable", L"italic; fore:#000080", L"" },
        { {MULTI_STYLE(SCE_HJ_DEFAULT,SCE_HJ_START,0,0)}, IDS_LEX_STR_63159, L"JS Default", L"", L"" },
        { {MULTI_STYLE(SCE_HJ_COMMENT,SCE_HJ_COMMENTLINE,SCE_HJ_COMMENTDOC,0)}, IDS_LEX_STR_63160, L"JS Comment", L"fore:#646464", L"" },
        { {SCE_HJ_KEYWORD}, IDS_LEX_STR_63163, L"JS Keyword", L"bold; fore:#A46000", L"" },
        { {SCE_HJ_WORD}, IDS_LEX_STR_63162, L"JS Identifier", L"", L"" },
        { {MULTI_STYLE(SCE_HJ_DOUBLESTRING,SCE_HJ_SINGLESTRING,SCE_HJ_STRINGEOL,SCE_HJ_TEMPLATELITERAL)}, IDS_LEX_STR_63164, L"JS String", L"fore:#008000", L"" },
        { {SCE_HJ_REGEX}, IDS_LEX_STR_63166, L"JS Regex", L"fore:#006633; back:#FFF1A8", L"" },
        { {SCE_HJ_NUMBER}, IDS_LEX_STR_63161, L"JS Number", L"fore:#FF0000", L"" },
        { {SCE_HJ_SYMBOLS}, IDS_LEX_STR_63165, L"JS Symbols", L"fore:#B000B0", L"" },
        { {MULTI_STYLE(SCE_HJA_DEFAULT,SCE_HJA_START,0,0)}, IDS_LEX_STR_63167, L"ASP JS Default", L"", L"" },
        { {MULTI_STYLE(SCE_HJA_COMMENT,SCE_HJA_COMMENTLINE,SCE_HJA_COMMENTDOC,0)}, IDS_LEX_STR_63168, L"ASP JS Comment", L"fore:#646464", L"" },
        { {SCE_HJA_KEYWORD}, IDS_LEX_STR_63171, L"ASP JS Keyword", L"bold; fore:#A46000", L"" },
        { {SCE_HJA_WORD}, IDS_LEX_STR_63170, L"ASP JS Identifier", L"", L"" },
        { {MULTI_STYLE(SCE_HJA_DOUBLESTRING,SCE_HJA_SINGLESTRING,SCE_HJA_STRINGEOL,SCE_HJA_TEMPLATELITERAL)}, IDS_LEX_STR_63172, L"ASP JS String", L"fore:#008000", L"" },
        { {SCE_HJA_REGEX}, IDS_LEX_STR_63174, L"ASP JS Regex", L"fore:#006633; back:#FFF1A8", L"" },
        { {SCE_HJA_NUMBER}, IDS_LEX_STR_63169, L"ASP JS Number", L"fore:#FF0000", L"" },
        { {SCE_HJA_SYMBOLS}, IDS_LEX_STR_63173, L"ASP JS Symbols", L"fore:#B000B0", L"" },
        { {MULTI_STYLE(SCE_HB_DEFAULT,SCE_HB_START,0,0)}, IDS_LEX_STR_63175, L"VBS Default", L"", L"" },
        { {SCE_HB_COMMENTLINE}, IDS_LEX_STR_63176, L"VBS Comment", L"fore:#646464", L"" },
        { {SCE_HB_WORD}, IDS_LEX_STR_63178, L"VBS Keyword", L"bold; fore:#B000B0", L"" },
        { {SCE_HB_IDENTIFIER}, IDS_LEX_STR_63180, L"VBS Identifier", L"", L"" },
        { {MULTI_STYLE(SCE_HB_STRING,SCE_HB_STRINGEOL,0,0)}, IDS_LEX_STR_63179, L"VBS String", L"fore:#008000", L"" },
        { {SCE_HB_NUMBER}, IDS_LEX_STR_63177, L"VBS Number", L"fore:#FF0000", L"" },
        { {MULTI_STYLE(SCE_HBA_DEFAULT,SCE_HBA_START,0,0)}, IDS_LEX_STR_63181, L"ASP VBS Default", L"", L"" },
        { {SCE_HBA_COMMENTLINE}, IDS_LEX_STR_63182, L"ASP VBS Comment", L"fore:#646464", L"" },
        { {SCE_HBA_WORD}, IDS_LEX_STR_63184, L"ASP VBS Keyword", L"bold; fore:#B000B0", L"" },
        { {SCE_HBA_IDENTIFIER}, IDS_LEX_STR_63186, L"ASP VBS Identifier", L"", L"" },
        { {MULTI_STYLE(SCE_HBA_STRING,SCE_HBA_STRINGEOL,0,0)}, IDS_LEX_STR_63185, L"ASP VBS String", L"fore:#008000", L"" },
        { {SCE_HBA_NUMBER}, IDS_LEX_STR_63183, L"ASP VBS Number", L"fore:#FF0000", L"" },
        { {SCE_HCSS_DEFAULT}, IDS_LEX_STR_CSSDefault, L"CSS Default", L"", L"" },
        { {SCE_HCSS_COMMENT}, IDS_LEX_STR_CSSComment, L"CSS Comment", L"fore:#646464", L"" },
        { {MULTI_STYLE(SCE_HCSS_SELECTOR,SCE_HCSS_PSEUDOCLASS,0,0)}, IDS_LEX_STR_CSSSelector, L"CSS Selector", L"bold; fore:#0A246A", L"" },
        { {MULTI_STYLE(SCE_HCSS_CLASS,SCE_HCSS_ID,0,0)}, IDS_LEX_STR_CSSClassID, L"CSS Class/ID", L"fore:#648000", L"" },
        { {SCE_HCSS_PROPERTY}, IDS_LEX_STR_CSSProperty, L"CSS Property", L"fore:#FF4000", L"" },
        { {MULTI_STYLE(SCE_HCSS_VALUE,SCE_HCSS_NUMBER,0,0)}, IDS_LEX_STR_CSSValue, L"CSS Value", L"fore:#3A6EA5", L"" },
        { {MULTI_STYLE(SCE_HCSS_STRING,SCE_HCSS_IMPORTANT,0,0)}, IDS_LEX_STR_CSSString, L"CSS String", L"fore:#008000", L"" },
        { {MULTI_STYLE(SCE_HCSS_OPERATOR,SCE_HCSS_DIRECTIVE,0,0)}, IDS_LEX_STR_CSSDirOp, L"CSS Directive/Operator", L"fore:#B000B0", L"" },
        EDITLEXER_SENTINEL
    }
};

