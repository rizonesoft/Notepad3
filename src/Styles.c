/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Styles.c                                                                    *
*   Scintilla Style Management                                                *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*	Mostly taken from SciTE, (c) Neil Hodgson                                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2016   *
*                                                 http://www.rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#if !defined(WINVER)
#define WINVER 0x601  /*_WIN32_WINNT_WIN7*/
#endif
#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x601  /*_WIN32_WINNT_WIN7*/
#endif
#if !defined(NTDDI_VERSION)
#define NTDDI_VERSION 0x06010000  /*NTDDI_WIN7*/
#endif
#define VC_EXTRALEAN 1

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <stdio.h>
#include "scintilla.h"
#include "scilexer.h"
#include "notepad3.h"
#include "edit.h"
#include "dialogs.h"
#include "resource.h"
#include "SciCall.h"
#include "helpers.h"
#include "styles.h"


extern HINSTANCE g_hInstance;

extern int iSciFontQuality;
extern const int FontQuality[4];

extern int  iMarkOccurrences;
extern BOOL bUseOldStyleBraceMatching;

#define MULTI_STYLE(a,b,c,d) ((a)|(b<<8)|(c<<16)|(d<<24))


static int iBaseFontSize = 10;


KEYWORDLIST KeyWords_NULL = {
"", "", "", "", "", "", "", "", "" };


EDITLEXER lexDefault =   { SCLEX_NULL, 63000, L"Default Text", L"txt; text; wtx; log; asc; doc", L"", &KeyWords_NULL, {
                /*  0 */ { STYLE_DEFAULT, 63100, L"Default Style", L"font:Default; size:10", L"" },
                /*  1 */ { STYLE_LINENUMBER, 63101, L"Margins and Line Numbers", L"size:-2; fore:#FF0000", L"" },
                /*  2 */ { STYLE_BRACELIGHT, 63102, L"Matching Braces", L"size:+1; bold; fore:#FF0000", L"" },
                /*  3 */ { STYLE_BRACEBAD, 63103, L"Matching Braces Error", L"size:+1; bold; fore:#000080", L"" },
                /*  4 */ { STYLE_CONTROLCHAR, 63104, L"Control Characters (Font)", L"size:-1", L"" },
                /*  5 */ { STYLE_INDENTGUIDE, 63105, L"Indentation Guide (Color)", L"fore:#A0A0A0", L"" },
                /*  6 */ { SCI_SETSELFORE+SCI_SETSELBACK, 63106, L"Selected Text (Colors)", L"back:#0A246A; eolfilled; alpha:95", L"" },
                /*  7 */ { SCI_SETWHITESPACEFORE+SCI_SETWHITESPACEBACK+SCI_SETWHITESPACESIZE, 63107, L"Whitespace (Colors, Size 0-5)", L"fore:#FF4000", L"" },
                /*  8 */ { SCI_SETCARETLINEBACK, 63108, L"Current Line Background (Color)", L"back:#FFFF00; alpha:50", L"" },
                /*  9 */ { SCI_SETCARETFORE+SCI_SETCARETWIDTH, 63109, L"Caret (Color, Size 1-3)", L"", L"" },
                /* 10 */ { SCI_SETEDGECOLOUR, 63110, L"Long Line Marker (Colors)", L"fore:#FFC000", L"" },
                /* 11 */ { SCI_SETEXTRAASCENT+SCI_SETEXTRADESCENT, 63111, L"Extra Line Spacing (Size)", L"size:2", L"" },
                /* 12 */ { SCI_MARKERSETBACK+SCI_MARKERSETALPHA, 63124, L"Book Marks (Colors)", L"back:#00FF00; alpha:20", L"" },
                /* 13 */ { SCI_MARKERSETBACK+SCI_MARKERSETALPHA, 63262, L"Mark Occurrences (Colors)", L"", L"" },
                /* 14 */ { SCI_SETHOTSPOTACTIVEFORE, 63264, L"Hyperlink Hotspots", L"italics; fore:#0000FF", L"" },
                
                /* 15 */ { STYLE_DEFAULT, 63112, L"2nd Default Style", L"font:Courier New; size:10", L"" },
                /* 16 */ { STYLE_LINENUMBER, 63113, L"2nd Margins and Line Numbers", L"font:Tahoma; size:-2; fore:#FF0000", L"" },
                /* 17 */ { STYLE_BRACELIGHT, 63114, L"2nd Matching Braces", L"bold; fore:#FF0000", L"" },
                /* 18 */ { STYLE_BRACEBAD, 63115, L"2nd Matching Braces Error", L"bold; fore:#000080", L"" },
                /* 19 */ { STYLE_CONTROLCHAR, 63116, L"2nd Control Characters (Font)", L"size:-1", L"" },
                /* 20 */ { STYLE_INDENTGUIDE, 63117, L"2nd Indentation Guide (Color)", L"fore:#A0A0A0", L"" },
                /* 21 */ { SCI_SETSELFORE + SCI_SETSELBACK, 63118, L"2nd Selected Text (Colors)", L"eolfilled", L"" },
                /* 22 */ { SCI_SETWHITESPACEFORE + SCI_SETWHITESPACEBACK + SCI_SETWHITESPACESIZE, 63119, L"2nd Whitespace (Colors, Size 0-5)", L"fore:#FF4000", L"" },
                /* 23 */ { SCI_SETCARETLINEBACK, 63120, L"2nd Current Line Background (Color)", L"back:#FFFF00; alpha:50", L"" },
                /* 24 */ { SCI_SETCARETFORE + SCI_SETCARETWIDTH, 63121, L"2nd Caret (Color, Size 1-3)", L"", L"" },
                /* 25 */ { SCI_SETEDGECOLOUR, 63122, L"2nd Long Line Marker (Colors)", L"fore:#FFC000", L"" },
                /* 26 */ { SCI_SETEXTRAASCENT + SCI_SETEXTRADESCENT, 63123, L"2nd Extra Line Spacing (Size)", L"", L"" },
                /* 27 */ { SCI_MARKERSETBACK+SCI_MARKERSETALPHA, 63125, L"2nd Book Marks (Colors)", L"back:#00FF00; alpha:20", L"" },
                /* 28 */ { SCI_MARKERSETBACK+SCI_MARKERSETALPHA, 63263, L"2nd Mark Occurrences (Colors)", L"fore:#0x00FF00; alpha:100; alpha2:100; indic_box", L"" },
                /* 29 */ { SCI_SETHOTSPOTACTIVEFORE, 63265, L"2nd Hyperlink Hotspots", L"bold; fore:#FF0000", L"" },

                         { -1, 00000, L"", L"", L"" } } };

enum LexDefaultStyles {
  STY_DEFAULT = 0,
  STY_MARGIN = 1,
  STY_BRACE_OK = 2,
  STY_BRACE_BAD = 3,
  STY_CTRL_CHR = 4,
  STY_INDENT_GUIDE = 5,
  STY_SEL_TXT = 6,
  STY_WHITESPACE = 7,
  STY_CUR_LN_BCK = 8,
  STY_CARET = 9,
  STY_LONG_LN_MRK = 10,
  STY_X_LN_SPACE = 11,
  STY_BOOK_MARK = 12,
  STY_MARK_OCC = 13,
  STY_URL_HOTSPOT = 14,

  STY_CNT_LAST = 15  // STY_2ND_XXX = STY_XXX + STY_CNT_LAST 
};


KEYWORDLIST KeyWords_HTML = {
"!doctype ^aria- ^data- a abbr accept accept-charset accesskey acronym action address align alink "
"alt and applet archive area article aside async audio autocomplete autofocus autoplay axis b "
"background base basefont bb bdi bdo bgcolor big blockquote body border bordercolor br buffered button "
"canvas caption cellpadding cellspacing center challenge char charoff charset checkbox checked "
"cite class classid clear code codebase codetype col colgroup color cols colspan command compact "
"content contenteditable contextmenu controls coords crossorigin data datafld dataformatas datagrid "
"datalist datapagesize datasrc datetime dd declare default defer del details dfn dialog dir dirname "
"disabled div dl download draggable dropzone dt em embed enctype event eventsource face fieldset "
"figcaption figure file font footer for form formaction formenctype formmethod formnovalidate "
"formtarget frame frameborder frameset h1 h2 h3 h4 h5 h6 head header headers height hgroup hidden "
"high hr href hreflang hspace html http-equiv i icon id iframe image img input ins integrity isindex "
"ismap itemprop itemscope itemtype kbd keygen keytype kind label lang language leftmargin legend li link "
"list longdesc loop low main manifest map marginheight marginwidth mark max maxlength media mediagroup "
"menu menuitem meta meter method min multiple muted name nav noframes nohref noresize noscript noshade "
"novalidate nowrap object ol onabort onafterprint onbeforeprint onbeforeunload onblur oncancel oncanplay "
"oncanplaythrough onchange onclick onclose oncontextmenu oncuechange ondblclick ondrag ondragend ondragenter "
"ondragleave ondragover ondragstart ondrop ondurationchange onemptied onended onerror onfocus onformchange "
"onforminput onhashchange oninput oninvalid onkeydown onkeypress onkeyup onload onloadeddata onloadedmetadata "
"onloadstart onmessage onmousedown onmousemove onmouseout onmouseover onmouseup onmousewheel "
"onoffline ononline onpagehide onpageshow onpause onplay onplaying onpopstate onprogress "
"onratechange onreadystatechange onredo onreset onresize onscroll onseeked onseeking onselect "
"onshow onstalled onstorage onsubmit onsuspend ontimeupdate onundo onunload onvolumechange "
"onwaiting open optgroup optimum option output p param password pattern ping placeholder poster "
"pre prefix preload profile progress prompt property pubdate public q radio radiogroup readonly rel "
"required reset rev reversed role rows rowspan rp rt ruby rules s samp sandbox scheme scope scoped script "
"scrolling seamless section select selected shape size sizes small source span spellcheck src "
"srcdoc srclang standby start step strike strong style sub submit summary sup tabindex table "
"target tbody td text textarea tfoot th thead time title topmargin tr track translate tt type "
"typemustmatch u ul usemap valign value valuetype var version video vlink vspace wbr width wrap xml "
"xmlns",
"abstract boolean break byte case catch char class const continue debugger default delete do "
"double else enum export extends false final finally float for function goto if implements "
"import in instanceof int interface long native new null package private protected public "
"return short static super switch synchronized this throw throws transient true try typeof var "
"void volatile while with",
"alias and as attribute begin boolean byref byte byval call case class compare const continue "
"currency date declare dim do double each else elseif empty end enum eqv erase error event exit "
"explicit false for friend function get global gosub goto if imp implement in integer is let lib "
"load long loop lset me mid mod module new next not nothing null object on option optional or "
"preserve private property public raiseevent redim rem resume return rset select set single "
"static stop string sub then to true type unload until variant wend while with withevents xor",
"",
"__callstatic __class__ __compiler_halt_offset__ __dir__ __file__ __function__ __get __halt_compiler "
"__isset __line__ __method__ __namespace__ __set __sleep __trait__ __unset __wakeup "
"abstract and argc argv array as break callable case catch cfunction class clone closure const continue "
"declare default define die directory do e_all e_compile_error e_compile_warning e_core_error e_core_warning "
"e_deprecated e_error e_fatal e_notice e_parse e_strict e_user_deprecated e_user_error e_user_notice "
"e_user_warning e_warning echo else elseif empty enddeclare endfor endforeach endif endswitch endwhile "
"eval exception exit extends false final for foreach function global goto http_cookie_vars http_env_vars "
"http_get_vars http_post_files http_post_vars http_server_vars if implements include include_once "
"instanceof insteadof interface isset list namespace new not null old_function or parent php_self "
"print private protected public require require_once return static stdclass switch this throw trait "
"true try unset use var virtual while xor",
"", "", "", "" };


EDITLEXER lexHTML = { SCLEX_HTML, 63001, L"Web Source Code", L"html; htm; asp; aspx; shtml; htd; xhtml; php; php3; phtml; htt; cfm; tpl; dtd; hta; htc", L"", &KeyWords_HTML, {
                      { STYLE_DEFAULT, 63126, L"Default", L"", L"" },
                      { MULTI_STYLE(SCE_H_TAG,SCE_H_TAGEND,0,0), 63136, L"HTML Tag", L"fore:#648000", L"" },
                      { SCE_H_TAGUNKNOWN, 63137, L"HTML Unknown Tag", L"fore:#C80000; back:#FFFF80", L"" },
                      { SCE_H_ATTRIBUTE, 63138, L"HTML Attribute", L"fore:#FF4000", L"" },
                      { SCE_H_ATTRIBUTEUNKNOWN, 63139, L"HTML Unknown Attribute", L"fore:#C80000; back:#FFFF80", L"" },
                      { SCE_H_VALUE, 63140, L"HTML Value", L"fore:#3A6EA5", L"" },
                      { MULTI_STYLE(SCE_H_DOUBLESTRING,SCE_H_SINGLESTRING,0,0), 63141, L"HTML String", L"fore:#3A6EA5", L"" },
                      { SCE_H_OTHER, 63142, L"HTML Other Inside Tag", L"fore:#3A6EA5", L"" },
                      { MULTI_STYLE(SCE_H_COMMENT,SCE_H_XCCOMMENT,0,0), 63143, L"HTML Comment", L"fore:#646464", L"" },
                      { SCE_H_ENTITY, 63144, L"HTML Entity", L"fore:#B000B0", L"" },
                      { SCE_H_DEFAULT, 63256, L"HTML Element Text", L"", L"" },
                      { MULTI_STYLE(SCE_H_XMLSTART,SCE_H_XMLEND,0,0), 63145, L"XML Identifier", L"bold; fore:#881280", L"" },
                      { SCE_H_SGML_DEFAULT, 63237, L"SGML", L"fore:#881280", L"" },
                      { SCE_H_CDATA, 63147, L"CDATA", L"fore:#646464", L"" },
                      { MULTI_STYLE(SCE_H_ASP,SCE_H_ASPAT,0,0), 63146, L"ASP Start Tag", L"bold; fore:#000080", L"" },
                      //{ SCE_H_SCRIPT, L"Script", L"", L"" },
                      { SCE_H_QUESTION, 63148, L"PHP Start Tag", L"bold; fore:#000080", L"" },
                      { SCE_HPHP_DEFAULT, 63149, L"PHP Default", L"", L"" },
                      { MULTI_STYLE(SCE_HPHP_COMMENT,SCE_HPHP_COMMENTLINE,0,0), 63157, L"PHP Comment", L"fore:#FF8000", L"" },
                      { SCE_HPHP_WORD, 63152, L"PHP Keyword", L"bold; fore:#A46000", L"" },
                      { SCE_HPHP_HSTRING, 63150, L"PHP String", L"fore:#008000", L"" },
                      { SCE_HPHP_SIMPLESTRING, 63151, L"PHP Simple String", L"fore:#008000", L"" },
                      { SCE_HPHP_NUMBER, 63153, L"PHP Number", L"fore:#FF0000", L"" },
                      { SCE_HPHP_OPERATOR, 63158, L"PHP Operator", L"fore:#B000B0", L"" },
                      { SCE_HPHP_VARIABLE, 63154, L"PHP Variable", L"italic; fore:#000080", L"" },
                      { SCE_HPHP_HSTRING_VARIABLE, 63155, L"PHP String Variable", L"italic; fore:#000080", L"" },
                      { SCE_HPHP_COMPLEX_VARIABLE, 63156, L"PHP Complex Variable", L"italic; fore:#000080", L"" },
                      { MULTI_STYLE(SCE_HJ_DEFAULT,SCE_HJ_START,0,0), 63159, L"JS Default", L"", L"" },
                      { MULTI_STYLE(SCE_HJ_COMMENT,SCE_HJ_COMMENTLINE,SCE_HJ_COMMENTDOC,0), 63160, L"JS Comment", L"fore:#646464", L"" },
                      { SCE_HJ_KEYWORD, 63163, L"JS Keyword", L"bold; fore:#A46000", L"" },
                      { SCE_HJ_WORD, 63162, L"JS Identifier", L"", L"" },
                      { MULTI_STYLE(SCE_HJ_DOUBLESTRING,SCE_HJ_SINGLESTRING,SCE_HJ_STRINGEOL,0), 63164, L"JS String", L"fore:#008000", L"" },
                      { SCE_HJ_REGEX, 63166, L"JS Regex", L"fore:#006633; back:#FFF1A8", L"" },
                      { SCE_HJ_NUMBER, 63161, L"JS Number", L"fore:#FF0000", L"" },
                      { SCE_HJ_SYMBOLS, 63165, L"JS Symbols", L"fore:#B000B0", L"" },
                      { MULTI_STYLE(SCE_HJA_DEFAULT,SCE_HJA_START,0,0), 63167, L"ASP JS Default", L"", L"" },
                      { MULTI_STYLE(SCE_HJA_COMMENT,SCE_HJA_COMMENTLINE,SCE_HJA_COMMENTDOC,0), 63168, L"ASP JS Comment", L"fore:#646464", L"" },
                      { SCE_HJA_KEYWORD, 63171, L"ASP JS Keyword", L"bold; fore:#A46000", L"" },
                      { SCE_HJA_WORD, 63170, L"ASP JS Identifier", L"", L"" },
                      { MULTI_STYLE(SCE_HJA_DOUBLESTRING,SCE_HJA_SINGLESTRING,SCE_HJA_STRINGEOL,0), 63172, L"ASP JS String", L"fore:#008000", L"" },
                      { SCE_HJA_REGEX, 63174, L"ASP JS Regex", L"fore:#006633; back:#FFF1A8", L"" },
                      { SCE_HJA_NUMBER, 63169, L"ASP JS Number", L"fore:#FF0000", L"" },
                      { SCE_HJA_SYMBOLS, 63173, L"ASP JS Symbols", L"fore:#B000B0", L"" },
                      { MULTI_STYLE(SCE_HB_DEFAULT,SCE_HB_START,0,0), 63175, L"VBS Default", L"", L"" },
                      { SCE_HB_COMMENTLINE, 63176, L"VBS Comment", L"fore:#646464", L"" },
                      { SCE_HB_WORD, 63178, L"VBS Keyword", L"bold; fore:#B000B0", L"" },
                      { SCE_HB_IDENTIFIER, 63180, L"VBS Identifier", L"", L"" },
                      { MULTI_STYLE(SCE_HB_STRING,SCE_HB_STRINGEOL,0,0), 63179, L"VBS String", L"fore:#008000", L"" },
                      { SCE_HB_NUMBER, 63177, L"VBS Number", L"fore:#FF0000", L"" },
                      { MULTI_STYLE(SCE_HBA_DEFAULT,SCE_HBA_START,0,0), 63181, L"ASP VBS Default", L"", L"" },
                      { SCE_HBA_COMMENTLINE, 63182, L"ASP VBS Comment", L"fore:#646464", L"" },
                      { SCE_HBA_WORD, 63184, L"ASP VBS Keyword", L"bold; fore:#B000B0", L"" },
                      { SCE_HBA_IDENTIFIER, 63186, L"ASP VBS Identifier", L"", L"" },
                      { MULTI_STYLE(SCE_HBA_STRING,SCE_HBA_STRINGEOL,0,0), 63185, L"ASP VBS String", L"fore:#008000", L"" },
                      { SCE_HBA_NUMBER, 63183, L"ASP VBS Number", L"fore:#FF0000", L"" },
                      //{ SCE_HP_START, L"Phyton Start", L"", L"" },
                      //{ SCE_HP_DEFAULT, L"Phyton Default", L"", L"" },
                      //{ SCE_HP_COMMENTLINE, L"Phyton Comment Line", L"", L"" },
                      //{ SCE_HP_NUMBER, L"Phyton Number", L"", L"" },
                      //{ SCE_HP_STRING, L"Phyton String", L"", L"" },
                      //{ SCE_HP_CHARACTER, L"Phyton Character", L"", L"" },
                      //{ SCE_HP_WORD, L"Phyton Keyword", L"", L"" },
                      //{ SCE_HP_TRIPLE, L"Phyton Triple", L"", L"" },
                      //{ SCE_HP_TRIPLEDOUBLE, L"Phyton Triple Double", L"", L"" },
                      //{ SCE_HP_CLASSNAME, L"Phyton Class Name", L"", L"" },
                      //{ SCE_HP_DEFNAME, L"Phyton Def Name", L"", L"" },
                      //{ SCE_HP_OPERATOR, L"Phyton Operator", L"", L"" },
                      //{ SCE_HP_IDENTIFIER, L"Phyton Identifier", L"", L"" },
                      //{ SCE_HPA_START, L"ASP Phyton Start", L"", L"" },
                      //{ SCE_HPA_DEFAULT, L"ASP Phyton Default", L"", L"" },
                      //{ SCE_HPA_COMMENTLINE, L"ASP Phyton Comment Line", L"", L"" },
                      //{ SCE_HPA_NUMBER, L"ASP Phyton Number", L"", L"" },
                      //{ SCE_HPA_STRING, L"ASP Phyton String", L"", L"" },
                      //{ SCE_HPA_CHARACTER, L"ASP Phyton Character", L"", L"" },
                      //{ SCE_HPA_WORD, L"ASP Phyton Keyword", L"", L"" },
                      //{ SCE_HPA_TRIPLE, L"ASP Phyton Triple", L"", L"" },
                      //{ SCE_HPA_TRIPLEDOUBLE, L"ASP Phyton Triple Double", L"", L"" },
                      //{ SCE_HPA_CLASSNAME, L"ASP Phyton Class Name", L"", L"" },
                      //{ SCE_HPA_DEFNAME, L"ASP Phyton Def Name", L"", L"" },
                      //{ SCE_HPA_OPERATOR, L"ASP Phyton Operator", L"", L"" },
                      //{ SCE_HPA_IDENTIFIER, L"ASP Phyton Identifier", L"", L"" },
                      { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_XML = {
"", "", "", "", "", "", "", "", "" };


EDITLEXER lexXML = { SCLEX_XML, 63002, L"XML Document", L"xml; xsl; rss; svg; xul; xsd; xslt; axl; rdf; xaml; vcproj", L"", &KeyWords_XML, {
                     { STYLE_DEFAULT, 63126, L"Default", L"", L"" },
                     { MULTI_STYLE(SCE_H_TAG,SCE_H_TAGUNKNOWN,SCE_H_TAGEND,0), 63187, L"XML Tag", L"fore:#881280", L"" },
                     { MULTI_STYLE(SCE_H_ATTRIBUTE,SCE_H_ATTRIBUTEUNKNOWN,0,0), 63188, L"XML Attribute", L"fore:#994500", L"" },
                     { SCE_H_VALUE, 63189, L"XML Value", L"fore:#1A1AA6", L"" },
                     { MULTI_STYLE(SCE_H_DOUBLESTRING,SCE_H_SINGLESTRING,0,0), 63190, L"XML String", L"fore:#1A1AA6", L"" },
                     { SCE_H_OTHER, 63191, L"XML Other Inside Tag", L"fore:#1A1AA6", L"" },
                     { MULTI_STYLE(SCE_H_COMMENT,SCE_H_XCCOMMENT,0,0), 63192, L"XML Comment", L"fore:#646464", L"" },
                     { SCE_H_ENTITY, 63193, L"XML Entity", L"fore:#B000B0", L"" },
                     { SCE_H_DEFAULT, 63257, L"XML Element Text", L"", L"" },
                     { MULTI_STYLE(SCE_H_XMLSTART,SCE_H_XMLEND,0,0), 63145, L"XML Identifier", L"bold; fore:#881280", L"" },
                     { SCE_H_SGML_DEFAULT, 63237, L"SGML", L"fore:#881280", L"" },
                     { SCE_H_CDATA, 63147, L"CDATA", L"fore:#646464", L"" },
                     { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_CSS = {
"align-content align-items align-self alignment-adjust alignment-baseline animation animation-delay "
"animation-direction animation-duration animation-fill-mode animation-iteration-count animation-name "
"animation-play-state animation-timing-function appearance ascent azimuth backface-visibility "
"background background-attachment background-blend-mode background-break background-clip background-color "
"background-image background-origin background-position background-repeat background-size "
"baseline baseline-shift bbox binding bleed bookmark-label bookmark-level bookmark-state "
"bookmark-target border border-bottom border-bottom-color border-bottom-left-radius "
"border-bottom-right-radius border-bottom-style border-bottom-width border-collapse border-color "
"border-image border-image-outset border-image-repeat border-image-slice border-image-source "
"border-image-width border-left border-left-color border-left-style border-left-width "
"border-length border-radius border-right border-right-color border-right-style "
"border-right-width border-spacing border-style border-top border-top-color "
"border-top-left-radius border-top-right-radius border-top-style border-top-width border-width "
"bottom box-align box-decoration-break box-direction box-flex box-flex-group box-lines "
"box-ordinal-group box-orient box-pack box-shadow box-sizing break-after break-before "
"break-inside cap-height caption-side centerline change-bar change-bar-class change-bar-offset "
"change-bar-side clear clip clip-path clip-rule color color-profile column-count column-fill column-gap "
"column-rule column-rule-color column-rule-style column-rule-width columns column-span column-width "
"content counter-increment counter-reset crop cue cue-after cue-before cursor definition-src descent "
"direction display dominant-baseline drop-initial-after-adjust drop-initial-after-align "
"drop-initial-before-adjust drop-initial-before-align drop-initial-size drop-initial-value "
"elevation empty-cells fill fit fit-position flex flex-basis flex-direction flex-flow flex-grow flex-shrink "
"flex-wrap float float-offset flow-from flow-into font font-family font-feature-settings font-kerning font-size "
"font-size-adjust font-stretch font-style font-synthesis font-variant font-weight grid-columns grid-rows "
"hanging-punctuation height hyphenate-after hyphenate-before hyphenate-character hyphenate-limit-chars "
"hyphenate-limit-last hyphenate-limit-zone hyphenate-lines hyphenate-resource hyphens icon image-orientation "
"image-resolution ime-mode inline-box-align insert-position interpret-as justify-content left letter-spacing "
"line-height line-stacking line-stacking-ruby line-stacking-shift line-stacking-strategy list-style "
"list-style-image list-style-position list-style-type make-element margin margin-bottom margin-left "
"margin-right margin-top mark mark-after mark-before marker-offset marks marquee-direction marquee-play-count "
"marquee-speed marquee-style mask mask-type mathline max-height max-width media min-height min-width "
"move-to nav-down nav-index nav-left nav-right nav-up object-fit object-position opacity order orphans "
"outline outline-color outline-offset outline-style outline-width overflow overflow-style overflow-wrap "
"overflow-x overflow-y padding padding-bottom padding-left padding-right padding-top page page-break-after "
"page-break-before page-break-inside page-policy panose-1 pause pause-after pause-before perspective "
"perspective-origin phonemes pitch pitch-range play-during pointer-events position presentation-level prototype "
"prototype-insert-policy prototype-insert-position punctuation-trim quotes region-overflow "
"rendering-intent resize rest rest-after rest-before richness right rotation rotation-point ruby-align "
"ruby-overhang ruby-position ruby-span shape-image-threshold shape-inside shape-outside size slope speak "
"speak-header speak-numeral speak-punctuation speech-rate src stemh stemv stress string-set tab-size table-layout "
"target target-name target-new target-position text-align text-align-last text-decoration text-decoration-color "
"text-decoration-line text-decoration-style text-emphasis text-height text-indent text-justify text-outline "
"text-overflow text-rendering text-replace text-shadow text-transform text-underline-position text-wrap top topline "
"transform transform-origin transform-style transition transition-delay transition-duration transition-property "
"transition-timing-function unicode-bidi unicode-range units-per-em vertical-align visibility "
"voice-balance voice-duration voice-family voice-pitch voice-pitch-range voice-rate voice-stress "
"voice-volume volume white-space white-space-collapse widows width widths will-change word-break word-spacing "
"word-wrap wrap wrap-flow wrap-margin wrap-padding wrap-through writing-mode x-height z-index",
"active after before checked choices default disabled empty enabled first first-child first-letter "
"first-line first-of-type focus hover indeterminate in-range invalid lang last-child last-of-type left "
"link not nth-child nth-last-child nth-last-of-type nth-of-type only-child only-of-type optional "
"out-of-range read-only read-write repeat-index repeat-item required right root target valid visited",
"", "",
"after before first-letter first-line selection",
"^-moz- ^-ms- ^-o- ^-webkit-",
"^-moz- ^-ms- ^-o- ^-webkit-",
"^-moz- ^-ms- ^-o- ^-webkit-",
"" };


EDITLEXER lexCSS = { SCLEX_CSS, 63003, L"CSS Style Sheets", L"css; less; sass; scss", L"", &KeyWords_CSS, {
                     { SCE_CSS_DEFAULT, 63126, L"Default", L"", L"" },
                     { SCE_CSS_COMMENT, 63127, L"Comment", L"fore:#646464", L"" },
                     { SCE_CSS_TAG, 63136, L"HTML Tag", L"bold; fore:#0A246A", L"" },
                     { SCE_CSS_CLASS, 63194, L"Tag-Class", L"fore:#648000", L"" },
                     { SCE_CSS_ID, 63195, L"Tag-Id", L"fore:#648000", L"" },
                     { SCE_CSS_ATTRIBUTE, 63196, L"Tag-Attribute", L"italic; fore:#648000", L"" },
                     { MULTI_STYLE(SCE_CSS_PSEUDOCLASS,SCE_CSS_EXTENDED_PSEUDOCLASS,0,0), 63197, L"Pseudo-class", L"fore:#B000B0", L"" },
                     { MULTI_STYLE(SCE_CSS_PSEUDOELEMENT,SCE_CSS_EXTENDED_PSEUDOELEMENT,0,0), 63361, L"Pseudo-element", L"fore:#B00050", L"" },
                     { MULTI_STYLE(SCE_CSS_IDENTIFIER,SCE_CSS_IDENTIFIER2,SCE_CSS_IDENTIFIER3,SCE_CSS_EXTENDED_IDENTIFIER), 63199, L"CSS Property", L"fore:#FF4000", L"" },
                     { MULTI_STYLE(SCE_CSS_DOUBLESTRING,SCE_CSS_SINGLESTRING,0,0), 63131, L"String", L"fore:#008000", L"" },
                     { SCE_CSS_VALUE, 63201, L"Value", L"fore:#3A6EA5", L"" },
                     { SCE_CSS_OPERATOR, 63132, L"Operator", L"fore:#B000B0", L"" },
                     { SCE_CSS_IMPORTANT, 63202, L"Important", L"bold; fore:#C80000", L"" },
                     { SCE_CSS_DIRECTIVE, 63203, L"Directive", L"bold; fore:#000000; back:#FFF1A8", L"" },
                     { SCE_CSS_MEDIA, 63382, L"Media", L"bold; fore:#0A246A", L"" },
                     { SCE_CSS_VARIABLE, 63249, L"Variable", L"bold; fore:#FF4000", L"" },
                     { SCE_CSS_UNKNOWN_PSEUDOCLASS, 63198, L"Unknown Pseudo-class", L"fore:#C80000; back:#FFFF80", L"" },
                     { SCE_CSS_UNKNOWN_IDENTIFIER, 63200, L"Unknown Property", L"fore:#C80000; back:#FFFF80", L"" },
                     { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_CPP = {
  // Primary keywords
  "alignas auto bool break case catch char char16_t char32_t class const constexpr const_cast "
  "continue decltype default delete do double dynamic_cast else enum explicit export extern false float "
  "for friend goto if inline int long mutable namespace new noexcept nullptr operator "
  "private protected public register reinterpret_cast restrict return short signed sizeof static "
  "static_assert static_cast struct switch template this thread_local throw true try typedef typeid typename "
  "union unsigned using virtual void volatile wchar_t while "
  "alignof defined naked noreturn",
  // Secondary keywords
  "asm __abstract __alignof __asm __assume __based __box __cdecl __declspec __delegate __event "
  "__except __except__try __fastcall __finally __forceinline __gc __hook __identifier "
  "__if_exists __if_not_exists __inline __interface __leave "
  "__multiple_inheritance __nogc __noop __pin __property __raise "
  "__sealed __single_inheritance __stdcall __super __try __try_cast __unhook __uuidof __value "
  "__virtual_inheritance",
  // Documentation comment keywords
  "",
  // Global classes and typedefs
  "complex imaginary int8_t int16_t int32_t int64_t intptr_t intmax_t ptrdiff_t size_t "
  "uint8_t uint16_t uint32_t uint64_t uintptr_t uintmax_t"
  "__int16 __int32 __int64 __int8 __m128 __m128d __m128i __m64 __wchar_t "
  "_Alignas _Alignof _Atomic _Bool _Complex _Generic _Imaginary _Noreturn _Pragma _Static_assert _Thread_local",
  // Preprocessor definitions
  "DEBUG NDEBUG UNICODE _DEBUG _UNICODE _MSC_VER", 
  // Task marker and error marker keywords
  "",
  "", 
  "", 
  "" 
};

EDITLEXER lexCPP = { SCLEX_CPP, 63004, L"C/C++ Source Code", L"c; cpp; cxx; cc; h; hpp; hxx; hh; m; mm; idl; inl; odl", L"", &KeyWords_CPP, {
                     { SCE_C_DEFAULT, 63126, L"Default", L"", L"" },
                     { SCE_C_IDENTIFIER, 63129, L"Identifier", L"", L"" },
                     { SCE_C_COMMENT, 63127, L"Comment", L"fore:#008000", L"" },
                     { SCE_C_WORD, 63128, L"Keyword", L"bold; fore:#0A246A", L"" },
                     { SCE_C_WORD2, 63260, L"Keyword 2nd", L"bold; italic; fore:#3C6CDD", L"" },
                     { SCE_C_GLOBALCLASS, 63258, L"Typedefs/Classes", L"bold; italic; fore:#800000", L"" },
                     { MULTI_STYLE(SCE_C_STRING,SCE_C_CHARACTER,SCE_C_STRINGEOL,SCE_C_VERBATIM), 63131, L"String", L"fore:#008000", L"" },
                     { SCE_C_NUMBER, 63130, L"Number", L"fore:#FF0000", L"" },
                     { SCE_C_OPERATOR, 63132, L"Operator", L"fore:#B000B0", L"" },
                     { MULTI_STYLE(SCE_C_PREPROCESSOR,SCE_C_PREPROCESSORCOMMENT,SCE_C_PREPROCESSORCOMMENTDOC,0), 63133, L"Preprocessor", L"fore:#FF8000", L"" },
                     //{ SCE_C_UUID, L"UUID", L"", L"" },
                     //{ SCE_C_REGEX, L"Regex", L"", L"" },
                     { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_CS = {
"abstract add alias as ascending async await base bool break by byte case catch char checked "
"class const continue decimal default delegate descending do double dynamic else "
"enum equals event explicit extern false finally fixed float for foreach from get "
"global goto group if implicit in int interface internal into is join lock let long "
"namespace new null object on operator orderby out override params partial private "
"protected public readonly ref remove return sbyte sealed select set short sizeof "
"stackalloc static string struct switch this throw true try typeof uint ulong "
"unchecked unsafe ushort using value var virtual void volatile where while yield",
"", "",
"AccessViolationException Action ActivationContext Activator AggregateException AppDomain "
"AppDomainInitializer AppDomainManager AppDomainManagerInitializationOptions AppDomainSetup "
"AppDomainUnloadedException ApplicationException ApplicationId ApplicationIdentity ArgIterator "
"ArgumentException ArgumentNullException ArgumentOutOfRangeException ArithmeticException Array "
"ArrayList ArraySegment ArrayTypeMismatchException AssemblyLoadEventArgs "
"AssemblyLoadEventHandler AsyncCallback Attribute AttributeTargets AttributeUsage "
"AttributeUsageAttribute BadImageFormatException Base64FormattingOptions BinaryReader "
"BinaryWriter BitArray BitConverter BlockingCollection Boolean Buffer BufferedStream "
"Byte CannotUnloadAppDomainException CaseInsensitiveComparer CaseInsensitiveHashCodeProvider "
"Char CharEnumerator CLSCompliant CLSCompliantAttribute CollectionBase CollectionDataContract "
"CollectionDataContractAttribute Color Comparer Comparison ConcurrentBag ConcurrentDictionary "
"ConcurrentQueue ConcurrentStack ConformanceLevel Console ConsoleCancelEventArgs "
"ConsoleCancelEventHandler ConsoleColor ConsoleKey ConsoleKeyInfo ConsoleModifiers "
"ConsoleSpecialKey ContextBoundObject ContextMarshalException ContextStatic "
"ContextStaticAttribute ContractNamespace ContractNamespaceAttribute Convert Converter "
"CrossAppDomainDelegate DataContract DataContractAttribute DataContractResolver "
"DataContractSerializer DataMember DataMemberAttribute DataMisalignedException DateTime "
"DateTimeKind DateTimeOffset DayOfWeek DBNull Decimal Delegate Dictionary DictionaryBase "
"DictionaryEntry Directory DirectoryInfo DirectoryNotFoundException DivideByZeroException "
"DllNotFoundException Double DriveInfo DriveNotFoundException DriveType DtdProcessing "
"DuplicateWaitObjectException EndOfStreamException EntityHandling EntryPointNotFoundException "
"Enum EnumMember EnumMemberAttribute Environment EnvironmentVariableTarget EqualityComparer "
"ErrorEventArgs ErrorEventHandler EventArgs EventHandler Exception ExecutionEngineException "
"ExportOptions ExtensionDataObject FieldAccessException File FileAccess FileAttributes "
"FileFormatException FileInfo FileLoadException FileMode FileNotFoundException FileOptions "
"FileShare FileStream FileStyleUriParser FileSystemEventArgs FileSystemEventHandler "
"FileSystemInfo FileSystemWatcher Flags FlagsAttribute FormatException Formatter "
"FormatterConverter FormatterServices Formatting FtpStyleUriParser Func GC GCCollectionMode "
"GCNotificationStatus GenericUriParser GenericUriParserOptions GopherStyleUriParser Guid "
"HandleInheritability HashSet Hashtable HttpStyleUriParser IAppDomainSetup IAsyncResult "
"ICloneable ICollection IComparable IComparer IConvertible ICustomFormatter "
"IDataContractSurrogate IDeserializationCallback IDictionary IDictionaryEnumerator IDisposable "
"IEnumerable IEnumerator IEqualityComparer IEquatable IExtensibleDataObject IFormatProvider "
"IFormattable IFormatter IFormatterConverter IFragmentCapableXmlDictionaryWriter "
"IgnoreDataMember IgnoreDataMemberAttribute IHashCodeProvider IHasXmlNode IList ImportOptions "
"IndexOutOfRangeException InsufficientExecutionStackException InsufficientMemoryException Int16 "
"Int32 Int64 InternalBufferOverflowException IntPtr InvalidCastException "
"InvalidDataContractException InvalidDataException InvalidOperationException "
"InvalidProgramException InvalidTimeZoneException IObjectReference IObservable IObserver "
"IODescription IODescriptionAttribute IOException IProducerConsumerCollection "
"ISafeSerializationData ISerializable ISerializationSurrogate IServiceProvider ISet "
"IStreamProvider IStructuralComparable IStructuralEquatable ISurrogateSelector "
"IXmlBinaryReaderInitializer IXmlBinaryWriterInitializer IXmlDictionary IXmlLineInfo "
"IXmlMtomReaderInitializer IXmlMtomWriterInitializer IXmlNamespaceResolver IXmlSchemaInfo "
"IXmlTextReaderInitializer IXmlTextWriterInitializer IXPathNavigable IXsltContextFunction "
"IXsltContextVariable KeyedByTypeCollection KeyNotFoundException KeyValuePair KnownType "
"KnownTypeAttribute Lazy LdapStyleUriParser LinkedList LinkedListNode List LoaderOptimization "
"LoaderOptimizationAttribute LoadOptions LocalDataStoreSlot MarshalByRefObject Math "
"MemberAccessException MemoryStream MethodAccessException MidpointRounding "
"MissingFieldException MissingMemberException MissingMethodException ModuleHandle MTAThread "
"MTAThreadAttribute MulticastDelegate MulticastNotSupportedException NamespaceHandling "
"NameTable NetDataContractSerializer NetPipeStyleUriParser NetTcpStyleUriParser "
"NewLineHandling NewsStyleUriParser NonSerialized NonSerializedAttribute "
"NotFiniteNumberException NotifyFilters NotImplementedException NotSupportedException Nullable "
"NullReferenceException Object ObjectDisposedException ObjectIDGenerator ObjectManager Obsolete "
"ObsoleteAttribute OnDeserialized OnDeserializedAttribute OnDeserializing "
"OnDeserializingAttribute OnSerialized OnSerializedAttribute OnSerializing "
"OnSerializingAttribute OnXmlDictionaryReaderClose OperatingSystem OperationCanceledException "
"OptionalField OptionalFieldAttribute OrderablePartitioner OutOfMemoryException "
"OverflowException ParamArray ParamArrayAttribute Partitioner Path PathTooLongException "
"PipeException PlatformID PlatformNotSupportedException Predicate Queue Random RankException "
"ReaderOptions ReadOnlyCollectionBase ReadState RenamedEventArgs RenamedEventHandler "
"ResolveEventArgs ResolveEventHandler RuntimeArgumentHandle RuntimeFieldHandle "
"RuntimeMethodHandle RuntimeTypeHandle SafeSerializationEventArgs SaveOptions SByte "
"SearchOption SeekOrigin Serializable SerializableAttribute SerializationBinder "
"SerializationEntry SerializationException SerializationInfo SerializationInfoEnumerator "
"SerializationObjectManager Single SortedDictionary SortedList SortedSet Stack "
"StackOverflowException STAThread STAThreadAttribute Stream StreamingContext "
"StreamingContextStates StreamReader StreamWriter String StringBuilder StringComparer StringComparison "
"StringReader StringSplitOptions StringWriter StructuralComparisons SurrogateSelector "
"SynchronizedCollection SynchronizedKeyedCollection SynchronizedReadOnlyCollection "
"SystemException TextReader TextWriter ThreadStatic ThreadStaticAttribute TimeoutException "
"TimeSpan TimeZone TimeZoneInfo TimeZoneNotFoundException Tuple Type TypeAccessException "
"TypeCode TypedReference TypeInitializationException TypeLoadException TypeUnloadedException "
"UInt16 UInt32 UInt64 UIntPtr UnauthorizedAccessException UnhandledExceptionEventArgs "
"UnhandledExceptionEventHandler UniqueId UnmanagedMemoryAccessor UnmanagedMemoryStream Uri "
"UriBuilder UriComponents UriFormat UriFormatException UriHostNameType UriIdnScope UriKind "
"UriParser UriPartial UriTemplate UriTemplateEquivalenceComparer UriTemplateMatch "
"UriTemplateMatchException UriTemplateTable UriTypeConverter ValidationEventArgs "
"ValidationEventHandler ValidationType ValueType Version Void WaitForChangedResult "
"WatcherChangeTypes WeakReference WhitespaceHandling WriteState XAttribute XCData XComment "
"XContainer XDeclaration XDocument XDocumentType XElement XmlAtomicValue XmlAttribute "
"XmlAttributeCollection XmlBinaryReaderSession XmlBinaryWriterSession XmlCaseOrder "
"XmlCDataSection XmlCharacterData XmlComment XmlConvert XmlDataDocument XmlDataType "
"XmlDateTimeSerializationMode XmlDeclaration XmlDictionary XmlDictionaryReader "
"XmlDictionaryReaderQuotas XmlDictionaryString XmlDictionaryWriter XmlDocument "
"XmlDocumentFragment XmlDocumentType XmlElement XmlEntity XmlEntityReference XmlException "
"XmlImplementation XmlLinkedNode XmlNamedNodeMap XmlNamespaceManager XmlNamespaceScope "
"XmlNameTable XmlNode XmlNodeChangedAction XmlNodeChangedEventArgs XmlNodeChangedEventHandler "
"XmlNodeList XmlNodeOrder XmlNodeReader XmlNodeType XmlNotation XmlObjectSerializer "
"XmlOutputMethod XmlParserContext XmlProcessingInstruction XmlQualifiedName XmlReader "
"XmlReaderSettings XmlResolver XmlSchema XmlSchemaAll XmlSchemaAnnotated XmlSchemaAnnotation "
"XmlSchemaAny XmlSchemaAnyAttribute XmlSchemaAppInfo XmlSchemaAttribute XmlSchemaAttributeGroup "
"XmlSchemaAttributeGroupRef XmlSchemaChoice XmlSchemaCollection XmlSchemaCollectionEnumerator "
"XmlSchemaCompilationSettings XmlSchemaComplexContent XmlSchemaComplexContentExtension "
"XmlSchemaComplexContentRestriction XmlSchemaComplexType XmlSchemaContent XmlSchemaContentModel "
"XmlSchemaContentProcessing XmlSchemaContentType XmlSchemaDatatype XmlSchemaDatatypeVariety "
"XmlSchemaDerivationMethod XmlSchemaDocumentation XmlSchemaElement XmlSchemaEnumerationFacet "
"XmlSchemaException XmlSchemaExternal XmlSchemaFacet XmlSchemaForm XmlSchemaFractionDigitsFacet "
"XmlSchemaGroup XmlSchemaGroupBase XmlSchemaGroupRef XmlSchemaIdentityConstraint "
"XmlSchemaImport XmlSchemaInclude XmlSchemaInference XmlSchemaInference.InferenceOption "
"XmlSchemaInferenceException XmlSchemaInfo XmlSchemaKey XmlSchemaKeyref XmlSchemaLengthFacet "
"XmlSchemaMaxExclusiveFacet XmlSchemaMaxInclusiveFacet XmlSchemaMaxLengthFacet "
"XmlSchemaMinExclusiveFacet XmlSchemaMinInclusiveFacet XmlSchemaMinLengthFacet "
"XmlSchemaNotation XmlSchemaNumericFacet XmlSchemaObject XmlSchemaObjectCollection "
"XmlSchemaObjectEnumerator XmlSchemaObjectTable XmlSchemaParticle XmlSchemaPatternFacet "
"XmlSchemaRedefine XmlSchemaSequence XmlSchemaSet XmlSchemaSimpleContent "
"XmlSchemaSimpleContentExtension XmlSchemaSimpleContentRestriction XmlSchemaSimpleType "
"XmlSchemaSimpleTypeContent XmlSchemaSimpleTypeList XmlSchemaSimpleTypeRestriction "
"XmlSchemaSimpleTypeUnion XmlSchemaTotalDigitsFacet XmlSchemaType XmlSchemaUnique "
"XmlSchemaUse XmlSchemaValidationException XmlSchemaValidationFlags XmlSchemaValidator "
"XmlSchemaValidity XmlSchemaWhiteSpaceFacet XmlSchemaXPath XmlSecureResolver "
"XmlSerializableServices XmlSeverityType XmlSignificantWhitespace XmlSortOrder XmlSpace "
"XmlText XmlTextReader XmlTextWriter XmlTokenizedType XmlTypeCode XmlUrlResolver "
"XmlValidatingReader XmlValueGetter XmlWhitespace XmlWriter XmlWriterSettings XName "
"XNamespace XNode XNodeDocumentOrderComparer XNodeEqualityComparer XObject XObjectChange "
"XObjectChangeEventArgs XPathDocument XPathException XPathExpression XPathItem "
"XPathNamespaceScope XPathNavigator XPathNodeIterator XPathNodeType XPathQueryGenerator "
"XPathResultType XProcessingInstruction XsdDataContractExporter XsdDataContractImporter "
"XslCompiledTransform XsltArgumentList XsltCompileException XsltContext XsltException "
"XsltMessageEncounteredEventArgs XsltMessageEncounteredEventHandler XslTransform XsltSettings "
"XStreamingElement XText",
"", "", "", "", "" };


EDITLEXER lexCS = { SCLEX_CPP, 63005, L"C# Source Code", L"cs", L"", &KeyWords_CS, {
                    { SCE_C_DEFAULT, 63126, L"Default", L"", L"" },
                    { SCE_C_COMMENT, 63127, L"Comment", L"fore:#008000", L"" },
                    { SCE_C_WORD, 63128, L"Keyword", L"bold; fore:#804000", L"" },
                    { SCE_C_IDENTIFIER, 63129, L"Identifier", L"", L"" },
                    { MULTI_STYLE(SCE_C_STRING,SCE_C_CHARACTER,SCE_C_STRINGEOL,0), 63131, L"String", L"fore:#008000", L"" },
                    { SCE_C_VERBATIM, 63134, L"Verbatim String", L"fore:#008000", L"" },
                    { SCE_C_NUMBER, 63130, L"Number", L"fore:#FF0000", L"" },
                    { SCE_C_OPERATOR, 63132, L"Operator", L"fore:#B000B0", L"" },
                    { SCE_C_PREPROCESSOR, 63133, L"Preprocessor", L"fore:#FF8000", L"" },
                    //{ SCE_C_UUID, L"UUID", L"", L"" },
                    //{ SCE_C_REGEX, L"Regex", L"", L"" },
                    //{ SCE_C_WORD2, L"Word 2", L"", L"" },
                    { SCE_C_GLOBALCLASS, 63337, L"Global Class", L"fore:#2B91AF", L"" },
                    { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_RC = {
"ACCELERATORS ALT AUTO3STATE AUTOCHECKBOX AUTORADIOBUTTON "
"BEGIN BITMAP BLOCK BUTTON CAPTION CHARACTERISTICS CHECKBOX "
"CLASS COMBOBOX CONTROL CTEXT CURSOR DEFPUSHBUTTON DIALOG "
"DIALOGEX DISCARDABLE EDITTEXT END EXSTYLE FONT GROUPBOX "
"ICON LANGUAGE LISTBOX LTEXT MENU MENUEX MENUITEM "
"MESSAGETABLE POPUP PUSHBUTTON RADIOBUTTON RCDATA RTEXT "
"SCROLLBAR SEPARATOR SHIFT STATE3 STRINGTABLE STYLE "
"TEXTINCLUDE VALUE VERSION VERSIONINFO VIRTKEY",
"", "", "", "", "", "", "", "" };


EDITLEXER lexRC = { SCLEX_CPP, 63006, L"Resource Script", L"rc; rc2; rct; rh; r; dlg", L"", &KeyWords_RC, {
                    { SCE_C_DEFAULT, 63126, L"Default", L"", L"" },
                    { SCE_C_COMMENT, 63127, L"Comment", L"fore:#008000", L"" },
                    { SCE_C_WORD, 63128, L"Keyword", L"bold; fore:#0A246A", L"" },
                    { SCE_C_IDENTIFIER, 63129, L"Identifier", L"", L"" },
                    { MULTI_STYLE(SCE_C_STRING,SCE_C_CHARACTER,SCE_C_STRINGEOL,SCE_C_VERBATIM), 63131, L"String", L"fore:#008000", L"" },
                    { SCE_C_NUMBER, 63130, L"Number", L"fore:#FF0000", L"" },
                    { SCE_C_OPERATOR, 63132, L"Operator", L"fore:#0A246A", L"" },
                    { SCE_C_PREPROCESSOR, 63133, L"Preprocessor", L"fore:#FF8000", L"" },
                    //{ SCE_C_UUID, L"UUID", L"", L"" },
                    //{ SCE_C_REGEX, L"Regex", L"", L"" },
                    //{ SCE_C_WORD2, L"Word 2", L"", L"" },
                    //{ SCE_C_GLOBALCLASS, L"Global Class", L"", L"" },
                    { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_MAK = {
"", "", "", "", "", "", "", "", "" };


EDITLEXER lexMAK = { SCLEX_MAKEFILE, 63007, L"Makefiles", L"mak; make; mk; dsp; msc; msvc", L"", &KeyWords_MAK, {
                     { SCE_MAKE_DEFAULT, 63126, L"Default", L"fore:#0A246A", L"" },
                     { SCE_MAKE_COMMENT, 63127, L"Comment", L"fore:#008000", L"" },
                     { MULTI_STYLE(SCE_MAKE_IDENTIFIER,SCE_MAKE_IDEOL,0,0), 63129, L"Identifier", L"fore:#003CE6", L"" },
                     { SCE_MAKE_OPERATOR, 63132, L"Operator", L"", L"" },
                     { SCE_MAKE_TARGET, 63204, L"Target", L"fore:#003CE6; back:#FFC000", L"" },
                     { SCE_MAKE_PREPROCESSOR, 63133, L"Preprocessor", L"fore:#FF8000", L"" },
                     { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_VBS = {
"alias and as attribute begin boolean byref byte byval call case class compare const continue "
"currency date declare dim do double each else elseif empty end enum eqv erase error event exit "
"explicit false for friend function get global gosub goto if imp implement in integer is let lib "
"load long loop lset me mid mod module new next not nothing null object on option optional or "
"preserve private property public raiseevent redim rem resume return rset select set single "
"static stop string sub then to true type unload until variant wend while with withevents xor",
"", "", "", "", "", "", "", "" };


EDITLEXER lexVBS = { SCLEX_VBSCRIPT, 63008, L"VBScript", L"vbs; dsm", L"", &KeyWords_VBS, {
                    { SCE_B_DEFAULT, 63126, L"Default", L"", L"" },
                    { SCE_B_COMMENT, 63127, L"Comment", L"fore:#808080", L"" },
                    { SCE_B_KEYWORD, 63128, L"Keyword", L"bold; fore:#B000B0", L"" },
                    { SCE_B_IDENTIFIER, 63129, L"Identifier", L"", L"" },
                    { MULTI_STYLE(SCE_B_STRING,SCE_B_STRINGEOL,0,0), 63131, L"String", L"fore:#008000", L"" },
                    { SCE_B_NUMBER, 63130, L"Number", L"fore:#FF0000", L"" },
                    { SCE_B_OPERATOR, 63132, L"Operator", L"", L"" },
                    //{ SCE_B_PREPROCESSOR, 63133, L"Preprocessor", L"fore:#FF9C00", L"" },
                    //{ SCE_B_CONSTANT, L"Constant", L"", L"" },
                    //{ SCE_B_DATE, L"Date", L"", L"" },
                    //{ SCE_B_KEYWORD2, L"Keyword 2", L"", L"" },
                    //{ SCE_B_KEYWORD3, L"Keyword 3", L"", L"" },
                    //{ SCE_B_KEYWORD4, L"Keyword 4", L"", L"" },
                    //{ SCE_B_ASM, L"Inline Asm", L"fore:#FF8000", L"" },
                    { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_VB = {
"addhandler addressof alias and andalso ansi any as assembly auto boolean byref byte byval call "
"case catch cbool cbyte cchar cdate cdbl cdec char cint class clng cobj compare const cshort csng "
"cstr ctype date decimal declare default delegate dim directcast do double each else elseif end "
"enum erase error event exit explicit externalsource false finally for friend function get "
"gettype gosub goto handles if implements imports in inherits integer interface is let lib like "
"long loop me mid mod module mustinherit mustoverride mybase myclass namespace new next not "
"nothing notinheritable notoverridable object on option optional or orelse overloads overridable "
"overrides paramarray preserve private property protected public raiseevent randomize readonly "
"redim rem removehandler resume return select set shadows shared short single static step stop "
"strict string structure sub synclock then throw to true try typeof unicode until variant when "
"while with withevents writeonly xor",
"", "", "", "", "", "", "", "" };


EDITLEXER lexVB = { SCLEX_VB, 63009, L"Visual Basic", L"vb; bas; frm; cls; ctl; pag; dsr; dob", L"", &KeyWords_VB, {
                    { SCE_B_DEFAULT, 63126, L"Default", L"", L"" },
                    { SCE_B_COMMENT, 63127, L"Comment", L"fore:#808080", L"" },
                    { SCE_B_KEYWORD, 63128, L"Keyword", L"bold; fore:#B000B0", L"" },
                    { SCE_B_IDENTIFIER, 63129, L"Identifier", L"", L"" },
                    { MULTI_STYLE(SCE_B_STRING,SCE_B_STRINGEOL,0,0), 63131, L"String", L"fore:#008000", L"" },
                    { MULTI_STYLE(SCE_B_NUMBER,SCE_B_DATE,0,0), 63130, L"Number", L"fore:#FF0000", L"" },
                    { SCE_B_OPERATOR, 63132, L"Operator", L"", L"" },
                    { SCE_B_PREPROCESSOR, 63133, L"Preprocessor", L"fore:#FF9C00", L"" },
                    //{ SCE_B_CONSTANT, L"Constant", L"", L"" },
                    //{ SCE_B_KEYWORD2, L"Keyword 2", L"", L"" },
                    //{ SCE_B_KEYWORD3, L"Keyword 3", L"", L"" },
                    //{ SCE_B_KEYWORD4, L"Keyword 4", L"", L"" },
                    //{ SCE_B_ASM, L"Inline Asm", L"fore:#FF8000", L"" },
                    { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_JS = {
"abstract boolean break byte case catch char class const continue debugger default delete do "
"double else enum export extends false final finally float for function goto if implements "
"import in instanceof int interface let long native new null package private protected public "
"return short static super switch synchronized this throw throws transient true try typeof var "
"void volatile while with",
"", "", "", "", "", "", "", "" };


EDITLEXER lexJS = { SCLEX_CPP, 63010, L"JavaScript", L"js; jse; jsm; as", L"", &KeyWords_JS, {
                    { SCE_C_DEFAULT, 63126, L"Default", L"", L"" },
                    { SCE_C_COMMENT, 63127, L"Comment", L"fore:#646464", L"" },
                    { SCE_C_WORD, 63128, L"Keyword", L"bold; fore:#A46000", L"" },
                    { SCE_C_IDENTIFIER, 63129, L"Identifier", L"", L"" },
                    { MULTI_STYLE(SCE_C_STRING,SCE_C_CHARACTER,SCE_C_STRINGEOL,SCE_C_VERBATIM), 63131, L"String", L"fore:#008000", L"" },
                    { SCE_C_REGEX, 63135, L"Regex", L"fore:#006633; back:#FFF1A8", L"" },
                    { SCE_C_NUMBER, 63130, L"Number", L"fore:#FF0000", L"" },
                    { SCE_C_OPERATOR, 63132, L"Operator", L"fore:#B000B0", L"" },
                    //{ SCE_C_UUID, L"UUID", L"", L"" },
                    //{ SCE_C_PREPROCESSOR, L"Preprocessor", L"fore:#FF8000", L"" },
                    //{ SCE_C_WORD2, L"Word 2", L"", L"" },
                    //{ SCE_C_GLOBALCLASS, L"Global Class", L"", L"" },
                    { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_JSON = {
"false true null",
"@id @context @type @value @language @container @list @set @reverse @index @base @vocab @graph",
"", "", "", "", "", "", "" };


EDITLEXER lexJSON = { SCLEX_JSON, 63382, L"JSON", L"json; eslintrc; jshintrc; jsonld", L"", &KeyWords_JSON, {
                    { SCE_C_DEFAULT, 63126, L"Default", L"", L"" },
                    { SCE_C_COMMENT, 63127, L"Comment", L"fore:#646464", L"" },
                    { SCE_C_WORD, 63128, L"Keyword", L"bold; fore:#A46000", L"" },
                    { SCE_C_IDENTIFIER, 63129, L"Identifier", L"", L"" },
                    { SCE_JSON_STRING, 63131, L"String", L"fore:#008000", L"" },
                    { SCE_C_REGEX, 63135, L"Regex", L"fore:#006633; back:#FFF1A8", L"" },
                    { SCE_JSON_NUMBER, 63130, L"Number", L"fore:#FF0000", L"" },
                    { SCE_C_OPERATOR, 63132, L"Operator", L"fore:#B000B0", L"" },
                    { -1, 00000, L"", L"", L"" } } };

/*
# String
style.json.2=fore:#7F0000
# Unclosed string       SCE_JSON_STRINGEOL
style.json.3=fore:#FFFFFF,back:#FF0000,eolfilled
# Property name         SCE_JSON_PROPERTYNAME
style.json.4=fore:#880AE8
# Escape sequence       SCE_JSON_ESCAPESEQUENCE
style.json.5=fore:#0B982E
# Line comment          SCE_JSON_LINECOMMENT
style.json.6=fore:#05BBAE,italics
# Block comment         SCE_JSON_BLOCKCOMMENT
style.json.7=$(style.json.6)
# Operator              SCE_JSON_OPERATOR
style.json.8=fore:#18644A
# URL/IRI               SCE_JSON_URI
style.json.9=fore:#0000FF
# JSON-LD compact IRI   SCE_JSON_COMPACTIRI
style.json.10=fore:#D137C1
# JSON keyword          SCE_JSON_KEYWORD
style.json.11=fore:#0BCEA7,bold
# JSON-LD keyword       SCE_JSON_LDKEYWORD
style.json.12=fore:#EC2806
# Parsing error         SCE_JSON_ERROR
style.json.13=fore:#FFFFFF,back:#FF0000
*/

KEYWORDLIST KeyWords_JAVA = {
"@interface abstract assert boolean break byte case catch char class const "
"continue default do double else enum extends final finally float for future "
"generic goto if implements import inner instanceof int interface long "
"native new null outer package private protected public rest return "
"short static super switch synchronized this throw throws transient try "
"var void volatile while "
"@Deprecated @Documented @FlaskyTest @Inherited @JavascriptInterface "
"@LargeTest @MediumTest @Override @Retention "
"@SmallTest @Smoke @Supress @SupressLint @SupressWarnings @Target @TargetApi "
"@TestTarget @TestTargetClass @UiThreadTest",
"", "", "", "", "", "", "", "" };


EDITLEXER lexJAVA = { SCLEX_CPP, 63011, L"Java Source Code", L"java", L"", &KeyWords_JAVA, {
                      { SCE_C_DEFAULT, 63126, L"Default", L"", L"" },
                      { SCE_C_COMMENT, 63127, L"Comment", L"fore:#646464", L"" },
                      { SCE_C_WORD, 63128, L"Keyword", L"bold; fore:#A46000", L"" },
                      { SCE_C_IDENTIFIER, 63129, L"Identifier", L"", L"" },
                      { MULTI_STYLE(SCE_C_STRING,SCE_C_CHARACTER,SCE_C_STRINGEOL,SCE_C_VERBATIM), 63131, L"String", L"fore:#008000", L"" },
                      { SCE_C_REGEX, 63135, L"Regex", L"fore:#006633; back:#FFF1A8", L"" },
                      { SCE_C_NUMBER, 63130, L"Number", L"fore:#FF0000", L"" },
                      { SCE_C_OPERATOR, 63132, L"Operator", L"fore:#B000B0", L"" },
                      //{ SCE_C_UUID, L"UUID", L"", L"" },
                      //{ SCE_C_PREPROCESSOR, L"Preprocessor", L"fore:#FF8000", L"" },
                      //{ SCE_C_WORD2, L"Word 2", L"", L"" },
                      //{ SCE_C_GLOBALCLASS, L"Global Class", L"", L"" },
                      { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_PAS = {
"absolute abstract alias and array as asm assembler begin break case cdecl class const constructor continue cppdecl default "
"destructor dispose div do downto else end end. except exit export exports external false far far16 file finalization finally for "
"forward function goto if implementation in index inherited initialization inline interface is label library local message mod "
"name near new nil nostackframe not object of oldfpccall on operator or out overload override packed pascal private procedure "
"program property protected public published raise read record register reintroduce repeat resourcestring safecall self set shl "
"shr softfloat stdcall stored string then threadvar to true try type unit until uses var virtual while with write xor",
"", "", "", "", "", "", "", "" };


EDITLEXER lexPAS = { SCLEX_PASCAL, 63012, L"Pascal Source Code", L"pas; dpr; dpk; dfm; inc; pp", L"", &KeyWords_PAS, {
                     { SCE_PAS_DEFAULT, 63126, L"Default", L"", L"" },
                     { MULTI_STYLE(SCE_PAS_COMMENT,SCE_PAS_COMMENT2,SCE_PAS_COMMENTLINE,0), 63127, L"Comment", L"fore:#646464", L"" },
                     { SCE_PAS_WORD, 63128, L"Keyword", L"bold; fore:#800080", L"" },
                     { SCE_PAS_IDENTIFIER, 63129, L"Identifier", L"", L"" },
                     { MULTI_STYLE(SCE_PAS_STRING,SCE_PAS_CHARACTER,SCE_PAS_STRINGEOL,0), 63131, L"String", L"fore:#008000", L"" },
                     { MULTI_STYLE(SCE_PAS_NUMBER,SCE_PAS_HEXNUMBER,0,0), 63130, L"Number", L"fore:#FF0000", L"" },
                     { SCE_PAS_OPERATOR, 63132, L"Operator", L"bold", L"" },
                     { SCE_PAS_ASM, 63205, L"Inline Asm", L"fore:#0000FF", L"" },
                     { MULTI_STYLE(SCE_PAS_PREPROCESSOR,SCE_PAS_PREPROCESSOR2,0,0), 63133, L"Preprocessor", L"fore:#FF00FF", L"" },
                     { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_ASM = {
"aaa aad aam aas adc add and arpl bound bsf bsr bswap bt btc btr bts call cbw cdq cflush clc cld "
"cli clts cmc cmova cmovae cmovb cmovbe cmovc cmove cmovg cmovge cmovl cmovle cmovna cmovnae "
"cmovnb cmovnbe cmovnc cmovne cmovng cmovnge cmovnl cmovnle cmovno cmovnp cmovns cmovnz cmovo "
"cmovp cmovpe cmovpo cmovs cmovz cmp cmps cmpsb cmpsd cmpsq cmpsw cmpxchg cmpxchg486 cmpxchg8b "
"cpuid cwd cwde daa das dec div emms enter esc femms hlt ibts icebp idiv imul in inc ins insb "
"insd insw int int01 int03 int1 int3 into invd invlpg iret iretd iretdf iretf iretw ja jae jb jbe "
"jc jcxz je jecxz jg jge jl jle jmp jna jnae jnb jnbe jnc jne jng jnge jnl jnle jno jnp jns jnz "
"jo jp jpe jpo js jz lahf lar lds lea leave les lfs lgdt lgs lidt lldt lmsw loadall loadall286 "
"lock lods lodsb lodsd lodsq lodsw loop loopd loope looped loopew loopne loopned loopnew loopnz "
"loopnzd loopnzw loopw loopz loopzd loopzw lsl lss ltr mov movs movsb movsd movsq movsw movsx "
"movsxd movzx mul neg nop not or out outs outsb outsd outsw pop popa popad popaw popf popfd popfw "
"push pusha pushad pushaw pushd pushf pushfd pushfw pushw rcl rcr rdmsr rdpmc rdshr rdtsc rep "
"repe repne repnz repz ret retf retn rol ror rsdc rsldt rsm rsts sahf sal salc sar sbb scas scasb "
"scasd scasq scasw seta setae setb setbe setc sete setg setge setl setle setna setnae setnb "
"setnbe setnc setne setng setnge setnl setnle setno setnp setns setnz seto setp setpe setpo sets "
"setz sgdt shl shld shr shrd sidt sldt smi smint smintold smsw stc std sti stos stosb stosd stosq "
"stosw str sub svdc svldt svts syscall sysenter sysexit sysret test ud0 ud1 ud2 umov verr verw "
"wait wbinvd wrmsr wrshr xadd xbts xchg xlat xlatb xor",
"f2xm1 fabs fadd faddp fbld fbstp fchs fclex fcmovb fcmovbe fcmove fcmovnb fcmovnbe fcmovne "
"fcmovnu fcmovu fcom fcomi fcomip fcomp fcompp fcos fdecstp fdisi fdiv fdivp fdivr fdivrp feni "
"ffree ffreep fiadd ficom ficomp fidiv fidivr fild fimul fincstp finit fist fistp fisub fisubr "
"fld fld1 fldcw fldenv fldenvd fldenvw fldl2e fldl2t fldlg2 fldln2 fldpi fldz fmul fmulp fnclex "
"fndisi fneni fninit fnop fnsave fnsaved fnsavew fnstcw fnstenv fnstenvd fnstenvw fnstsw fpatan "
"fprem fprem1 fptan frndint frstor frstord frstorw fsave fsaved fsavew fscale fsetpm fsin fsincos "
"fsqrt fst fstcw fstenv fstenvd fstenvw fstp fstsw fsub fsubp fsubr fsubrp ftst fucom fucomp "
"fucompp fwait fxam fxch fxtract fyl2x fyl2xp1",
"ah al ax bh bl bp bx ch cl cr0 cr2 cr3 cr4 cs cx dh di dl dr0 dr1 dr2 dr3 dr6 dr7 ds dx eax ebp "
"ebx ecx edi edx eip es esi esp fs gs mm0 mm1 mm2 mm3 mm4 mm5 mm6 mm7 r10 r10b r10d r10w r11 r11b "
"r11d r11w r12 r12b r12d r12w r13 r13b r13d r13w r14 r14b r14d r14w r15 r15b r15d r15w r8 r8b r8d "
"r8w r9 r9b r9d r9w rax rbp rbx rcx rdi rdx rip rsi rsp si sp ss st st0 st1 st2 st3 st4 st5 st6 "
"st7 tr3 tr4 tr5 tr6 tr7 xmm0 xmm1 xmm10 xmm11 xmm12 xmm13 xmm14 xmm15 xmm2 xmm3 xmm4 xmm5 xmm6 "
"xmm7 xmm8 xmm9 ymm0 ymm1 ymm10 ymm11 ymm12 ymm13 ymm14 ymm15 ymm2 ymm3 ymm4 ymm5 ymm6 ymm7 ymm8 "
"ymm9",
"%arg %assign %define %elif %elifctk %elifdef %elifid %elifidn %elifidni %elifmacro %elifnctk "
"%elifndef %elifnid %elifnidn %elifnidni %elifnmacro %elifnnum %elifnstr %elifnum %elifstr %else "
"%endif %endmacro %endrep %error %exitrep %iassign %idefine %if %ifctk %ifdef %ifid %ifidn "
"%ifidni %ifmacro %ifnctk %ifndef %ifnid %ifnidn %ifnidni %ifnmacro %ifnnum %ifnstr %ifnum %ifstr "
"%imacro %include %line %local %macro %out %pop %push %rep %repl %rotate %stacksize %strlen "
"%substr %undef %xdefine %xidefine .186 .286 .286c .286p .287 .386 .386c .386p .387 .486 .486p "
".8086 .8087 .alpha .break .code .const .continue .cref .data .data? .dosseg .else .elseif .endif "
".endw .err .err1 .err2 .errb .errdef .errdif .errdifi .erre .erridn .erridni .errnb .errndef "
".errnz .exit .fardata .fardata? .if .lall .lfcond .list .listall .listif .listmacro "
".listmacroall .model .msfloat .no87 .nocref .nolist .nolistif .nolistmacro .radix .repeat .sall "
".seq .sfcond .stack .startup .tfcond .type .until .untilcxz .while .xall .xcref .xlist absolute "
"alias align alignb assume at bits catstr comm comment common cpu db dd df dosseg dq dt dup dw "
"echo else elseif elseif1 elseif2 elseifb elseifdef elseifdif elseifdifi elseife elseifidn "
"elseifidni elseifnb elseifndef end endif endm endp ends endstruc eq equ even exitm export extern "
"externdef extrn for forc ge global goto group gt high highword iend if if1 if2 ifb ifdef ifdif "
"ifdifi ife ifidn ifidni ifnb ifndef import incbin include includelib instr invoke irp irpc "
"istruc label le length lengthof local low lowword lroffset lt macro mask mod name ne offset "
"opattr option org page popcontext proc proto ptr public purge pushcontext record repeat rept "
"resb resd resq rest resw section seg segment short size sizeof sizestr struc struct substr "
"subtitle subttl textequ this times title type typedef union use16 use32 while width",
"$ $$ %0 %1 %2 %3 %4 %5 %6 %7 %8 %9 .bss .data .text ? @b @f a16 a32 abs addr all assumes at "
"basic byte c carry? casemap common compact cpu dotname dword emulator epilogue error export "
"expr16 expr32 far far16 far32 farstack flat forceframe fortran fword huge language large listing "
"ljmp loadds m510 medium memory near near16 near32 nearstack nodotname noemulator nokeyword "
"noljmp nom510 none nonunique nooldmacros nooldstructs noreadonly noscoped nosignextend nosplit "
"nothing notpublic o16 o32 oldmacros oldstructs os_dos overflow? para parity? pascal private "
"prologue qword radix readonly real10 real4 real8 req sbyte scoped sdword seq setif2 sign? small "
"smallstack stdcall sword syscall tbyte tiny use16 use32 uses vararg word wrt zero?",
"addpd addps addsd addss andnpd andnps andpd andps blendpd blendps blendvpd blendvps cmpeqpd "
"cmpeqps cmpeqsd cmpeqss cmplepd cmpleps cmplesd cmpless cmpltpd cmpltps cmpltsd cmpltss cmpnepd "
"cmpneps cmpnesd cmpness cmpnlepd cmpnleps cmpnlesd cmpnless cmpnltpd cmpnltps cmpnltsd cmpnltss "
"cmpordpd cmpordps cmpordsd cmpordss cmpunordpd cmpunordps cmpunordsd cmpunordss comisd comiss "
"crc32 cvtdq2pd cvtdq2ps cvtpd2dq cvtpd2pi cvtpd2ps cvtpi2pd cvtpi2ps cvtps2dq cvtps2pd cvtps2pi "
"cvtsd2si cvtsd2ss cvtsi2sd cvtsi2ss cvtss2sd cvtss2si cvttpd2dq cvttpd2pi cvttps2dq cvttps2pi "
"cvttsd2si cvttss2si divpd divps divsd divss dppd dpps extractps fxrstor fxsave insertps ldmxscr "
"lfence maskmovdq maskmovdqu maxpd maxps maxss mfence minpd minps minsd minss movapd movaps movd "
"movdq2q movdqa movdqu movhlps movhpd movhps movlhps movlpd movlps movmskpd movmskps movntdq "
"movntdqa movnti movntpd movntps movntq movq movq2dq movsd movss movupd movups mpsadbw mulpd "
"mulps mulsd mulss orpd orps packssdw packsswb packusdw packuswb paddb paddd paddq paddsb paddsiw "
"paddsw paddusb paddusw paddw pand pandn pause paveb pavgb pavgusb pavgw paxsd pblendvb pblendw "
"pcmpeqb pcmpeqd pcmpeqq pcmpeqw pcmpestri pcmpestrm pcmpgtb pcmpgtd pcmpgtq pcmpgtw pcmpistri "
"pcmpistrm pdistib pextrb pextrd pextrq pextrw pf2id pf2iw pfacc pfadd pfcmpeq pfcmpge pfcmpgt "
"pfmax pfmin pfmul pfnacc pfpnacc pfrcp pfrcpit1 pfrcpit2 pfrsqit1 pfrsqrt pfsub pfsubr "
"phminposuw pi2fd pinsrb pinsrd pinsrq pinsrw pmachriw pmaddwd pmagw pmaxsb pmaxsd pmaxsw pmaxub "
"pmaxud pmaxuw pminsb pminsd pminsw pminub pminud pminuw pmovmskb pmovsxbd pmovsxbq pmovsxbw "
"pmovsxdq pmovsxwd pmovsxwq pmovzxbd pmovzxbq pmovzxbw pmovzxdq pmovzxwd pmovzxwq pmuldq pmulhriw "
"pmulhrwa pmulhrwc pmulhuw pmulhw pmulld pmullw pmuludq pmvgezb pmvlzb pmvnzb pmvzb popcnt por "
"prefetch prefetchnta prefetcht0 prefetcht1 prefetcht2 prefetchw psadbw pshufd pshufhw pshuflw "
"pshufw pslld pslldq psllq psllw psrad psraw psrld psrldq psrlq psrlw psubb psubd psubq psubsb "
"psubsiw psubsw psubusb psubusw psubw pswapd ptest punpckhbw punpckhdq punpckhqdq punpckhwd "
"punpcklbw punpckldq punpcklqdq punpcklwd pxor rcpps rcpss roundpd roundps roundsd roundss "
"rsqrtps rsqrtss sfence shufpd shufps sqrtpd sqrtps sqrtsd sqrtss stmxcsr subpd subps subsd subss "
"ucomisd ucomiss unpckhpd unpckhps unpcklpd unpcklps xorpd xorps",
"", "", "" };


EDITLEXER lexASM = { SCLEX_ASM, 63013, L"Assembly Script", L"asm", L"", &KeyWords_ASM, {
                     { SCE_ASM_DEFAULT, 63126, L"Default", L"", L"" },
                     { MULTI_STYLE(SCE_ASM_COMMENT,SCE_ASM_COMMENTBLOCK,0,0), 63127, L"Comment", L"fore:#008000", L"" },
                     { SCE_ASM_IDENTIFIER, 63129, L"Identifier", L"", L"" },
                     { MULTI_STYLE(SCE_ASM_STRING,SCE_ASM_CHARACTER,SCE_ASM_STRINGEOL,0), 63131, L"String", L"fore:#008000", L"" },
                     { SCE_ASM_NUMBER, 63130, L"Number", L"fore:#FF0000", L"" },
                     { SCE_ASM_OPERATOR, 63132, L"Operator", L"fore:#0A246A", L"" },
                     { SCE_ASM_CPUINSTRUCTION, 63206, L"CPU Instruction", L"fore:#0A246A", L"" },
                     { SCE_ASM_MATHINSTRUCTION, 63207, L"FPU Instruction", L"fore:#0A246A", L"" },
                     { SCE_ASM_EXTINSTRUCTION, 63210, L"Extended Instruction", L"fore:#0A246A", L"" },
                     { SCE_ASM_DIRECTIVE, 63203, L"Directive", L"fore:#0A246A", L"" },
                     { SCE_ASM_DIRECTIVEOPERAND, 63209, L"Directive Operand", L"fore:#0A246A", L"" },
                     { SCE_ASM_REGISTER, 63208, L"Register", L"fore:#FF8000", L"" },
                     { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_PL = {
"__DATA__ __END__ __FILE__ __LINE__ __PACKAGE__ abs accept alarm and atan2 AUTOLOAD BEGIN "
"bind binmode bless break caller chdir CHECK chmod chomp chop chown chr chroot close closedir "
"cmp connect continue CORE cos crypt dbmclose dbmopen default defined delete DESTROY die do "
"dump each else elsif END endgrent endhostent endnetent endprotoent endpwent endservent eof "
"eq EQ eval exec exists exit exp fcntl fileno flock for foreach fork format formline ge GE "
"getc getgrent getgrgid getgrnam gethostbyaddr gethostbyname gethostent getlogin "
"getnetbyaddr getnetbyname getnetent getpeername getpgrp getppid getpriority getprotobyname "
"getprotobynumber getprotoent getpwent getpwnam getpwuid getservbyname getservbyport "
"getservent getsockname getsockopt given glob gmtime goto grep gt GT hex if index INIT int "
"ioctl join keys kill last lc lcfirst le LE length link listen local localtime lock log "
"lstat lt LT map mkdir msgctl msgget msgrcv msgsnd my ne NE next no not NULL oct open "
"opendir or ord our pack package pipe pop pos print printf prototype push qu quotemeta rand "
"read readdir readline readlink readpipe recv redo ref rename require reset return reverse "
"rewinddir rindex rmdir say scalar seek seekdir select semctl semget semop send setgrent "
"sethostent setnetent setpgrp setpriority setprotoent setpwent setservent setsockopt shift "
"shmctl shmget shmread shmwrite shutdown sin sleep socket socketpair sort splice split "
"sprintf sqrt srand stat state study sub substr symlink syscall sysopen sysread sysseek "
"system syswrite tell telldir tie tied time times truncate uc ucfirst umask undef UNITCHECK "
"unless unlink unpack unshift untie until use utime values vec wait waitpid wantarray warn "
"when while write xor",
"", "", "", "", "", "", "", "" };


EDITLEXER lexPL = { SCLEX_PERL, 63014, L"Perl Script", L"pl; pm; cgi; pod", L"", &KeyWords_PL, {
                    { SCE_PL_DEFAULT, 63126, L"Default", L"", L"" },
                    { SCE_PL_COMMENTLINE, 63127, L"Comment", L"fore:#646464", L"" },
                    { SCE_PL_WORD, 63128, L"Keyword", L"bold; fore:#804000", L"" },
                    { SCE_PL_IDENTIFIER, 63129, L"Identifier", L"", L"" },
                    { SCE_PL_STRING, 63211, L"String double quoted", L"fore:#008000", L"" },
                    { SCE_PL_CHARACTER, 63212, L"String single quoted", L"fore:#008000", L"" },
                    { SCE_PL_NUMBER, 63130, L"Number", L"fore:#FF0000", L"" },
                    { SCE_PL_OPERATOR, 63132, L"Operator", L"bold", L"" },
                    { SCE_PL_SCALAR, 63215, L"Scalar $var", L"fore:#0A246A", L"" },
                    { SCE_PL_ARRAY, 63216, L"Array @var", L"fore:#003CE6", L"" },
                    { SCE_PL_HASH, 63217, L"Hash %var", L"fore:#B000B0", L"" },
                    { SCE_PL_SYMBOLTABLE, 63218, L"Symbol table *var", L"fore:#3A6EA5", L"" },
                    { SCE_PL_REGEX, 63219, L"Regex /re/ or m{re}", L"fore:#006633; back:#FFF1A8", L"" },
                    { SCE_PL_REGSUBST, 63220, L"Substitution s/re/ore/", L"fore:#006633; back:#FFF1A8", L"" },
                    { SCE_PL_BACKTICKS, 63221, L"Back ticks", L"fore:#E24000; back:#FFF1A8", L"" },
                    { SCE_PL_HERE_DELIM, 63223, L"Here-doc (delimiter)", L"fore:#648000", L"" },
                    { SCE_PL_HERE_Q, 63224, L"Here-doc (single quoted, q)", L"fore:#648000", L"" },
                    { SCE_PL_HERE_QQ, 63225, L"Here-doc (double quoted, qq)", L"fore:#648000", L"" },
                    { SCE_PL_HERE_QX, 63226, L"Here-doc (back ticks, qx)", L"fore:#E24000; back:#FFF1A8", L"" },
                    { SCE_PL_STRING_Q, 63227, L"Single quoted string (generic, q)", L"fore:#008000", L"" },
                    { SCE_PL_STRING_QQ, 63228, L"Double quoted string (qq)", L"fore:#008000", L"" },
                    { SCE_PL_STRING_QX, 63229, L"Back ticks (qx)", L"fore:#E24000; back:#FFF1A8", L"" },
                    { SCE_PL_STRING_QR, 63230, L"Regex (qr)", L"fore:#006633; back:#FFF1A8", L"" },
                    { SCE_PL_STRING_QW, 63231, L"Array (qw)", L"fore:#003CE6", L"" },
                    { SCE_PL_SUB_PROTOTYPE, 63253, L"Prototype", L"fore:#800080; back:#FFE2FF", L"" },
                    { SCE_PL_FORMAT_IDENT, 63254, L"Format identifier", L"bold; fore:#648000; back:#FFF1A8", L"" },
                    { SCE_PL_FORMAT, 63255, L"Format body", L"fore:#648000; back:#FFF1A8", L"" },
                    { SCE_PL_POD, 63213, L"POD (common)", L"fore:#A46000; back:#FFFFC0; eolfilled", L"" },
                    { SCE_PL_POD_VERB, 63214, L"POD (verbatim)", L"fore:#A46000; back:#FFFFC0; eolfilled", L"" },
                    { SCE_PL_DATASECTION, 63222, L"Data section", L"fore:#A46000; back:#FFFFC0; eolfilled", L"" },
                    { SCE_PL_ERROR, 63252, L"Parsing error", L"fore:#C80000; back:#FFFF80", L"" },
                    //{ SCE_PL_PUNCTUATION, L"Symbols / punctuation (not used)", L"", L"" },
                    //{ SCE_PL_PREPROCESSOR, L"Preprocessor (not used)", L"", L"" },
                    //{ SCE_PL_LONGQUOTE, L"Long quote (qq, qr, qw, qx) (not used)", L"", L"" },
                    { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_PROPS = {
"", "", "", "", "", "", "", "", "" };


EDITLEXER lexPROPS = { SCLEX_PROPERTIES, 63015, L"Configuration Files", L"ini; inf; cfg; properties; oem; sif; url; sed; theme", L"", &KeyWords_PROPS, {
                     { SCE_PROPS_DEFAULT, 63126, L"Default", L"", L"" },
                     { SCE_PROPS_COMMENT, 63127, L"Comment", L"fore:#008000", L"" },
                     { SCE_PROPS_SECTION, 63232, L"Section", L"fore:#000000; back:#FF8040; bold; eolfilled", L"" },
                     { SCE_PROPS_ASSIGNMENT, 63233, L"Assignment", L"fore:#FF0000", L"" },
                     { SCE_PROPS_DEFVAL, 63234, L"Default Value", L"fore:#FF0000", L"" },
                     { -1, 00000, L"", L"", L"" } } };


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


EDITLEXER lexBAT = { SCLEX_BATCH, 63016, L"Batch Files", L"bat; cmd", L"", &KeyWords_BAT, {
                     { SCE_BAT_DEFAULT, 63126, L"Default", L"", L"" },
                     { SCE_BAT_COMMENT, 63127, L"Comment", L"fore:#008000", L"" },
                     { SCE_BAT_WORD, 63128, L"Keyword", L"bold; fore:#0A246A", L"" },
                     { SCE_BAT_IDENTIFIER, 63129, L"Identifier", L"fore:#003CE6; back:#FFF1A8", L"" },
                     { SCE_BAT_OPERATOR, 63132, L"Operator", L"fore:#B000B0", L"" },
                     { MULTI_STYLE(SCE_BAT_COMMAND,SCE_BAT_HIDE,0,0), 63236, L"Command", L"bold", L"" },
                     { SCE_BAT_LABEL, 63235, L"Label", L"fore:#C80000; back:#F4F4F4; eolfilled", L"" },
                     { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_DIFF = {
"", "", "", "", "", "", "", "", "" };


EDITLEXER lexDIFF = { SCLEX_DIFF, 63017, L"Diff Files", L"diff; patch", L"", &KeyWords_DIFF, {
                      { SCE_DIFF_DEFAULT, 63126, L"Default", L"", L"" },
                      { SCE_DIFF_COMMENT, 63127, L"Comment", L"fore:#008000", L"" },
                      { SCE_DIFF_COMMAND, 63236, L"Command", L"bold; fore:#0A246A", L"" },
                      { SCE_DIFF_HEADER, 63238, L"Source and Destination", L"fore:#C80000; back:#FFF1A8; eolfilled", L"" },
                      { SCE_DIFF_POSITION, 63239, L"Position Setting", L"fore:#0000FF", L"" },
                      { SCE_DIFF_ADDED, 63240, L"Line Addition", L"fore:#002000; back:#80FF80; eolfilled", L"" },
                      { SCE_DIFF_DELETED, 63241, L"Line Removal", L"fore:#200000; back:#FF8080; eolfilled", L"" },
                      { SCE_DIFF_CHANGED, 63242, L"Line Change", L"fore:#000020; back:#8080FF; eolfilled", L"" },
                      { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_SQL = {
"abort accessible action add after all alter analyze and as asc asensitive attach autoincrement "
"before begin between bigint binary bit blob both by call cascade case cast change char character "
"check collate column commit condition conflict constraint continue convert create cross current_date "
"current_time current_timestamp current_user cursor database databases date day_hour day_microsecond "
"day_minute day_second dec decimal declare default deferrable deferred delayed delete desc describe "
"detach deterministic distinct distinctrow div double drop dual each else elseif enclosed end enum "
"escape escaped except exclusive exists exit explain fail false fetch float float4 float8 for force "
"foreign from full fulltext glob grant group having high_priority hour_microsecond hour_minute "
"hour_second if ignore immediate in index infile initially inner inout insensitive insert instead int "
"int1 int2 int3 int4 int8 integer intersect interval into is isnull iterate join key keys kill "
"leading leave left like limit linear lines load localtime localtimestamp lock long longblob longtext "
"loop low_priority master_ssl_verify_server_cert match merge mediumblob mediumint mediumtext middleint "
"minute_microsecond minute_second mod modifies natural no no_write_to_binlog not notnull null numeric "
"of offset on optimize option optionally or order out outer outfile plan pragma precision primary "
"procedure purge query raise range read read_only read_write reads real references regexp reindex "
"release rename repeat replace require restrict return revoke right rlike rollback row rowid schema "
"schemas second_microsecond select sensitive separator set show smallint spatial specific sql "
"sql_big_result sql_calc_found_rows sql_small_result sqlexception sqlstate sqlwarning ssl starting "
"straight_join table temp temporary terminated text then time timestamp tinyblob tinyint tinytext to "
"trailing transaction trigger true undo union unique unlock unsigned update usage use using utc_date "
"utc_time utc_timestamp vacuum values varbinary varchar varcharacter varying view virtual when where "
"while with write xor year_month zerofill",
"", "", "", "", "", "", "", "" };


EDITLEXER lexSQL = { SCLEX_SQL, 63018, L"SQL Query", L"sql", L"", &KeyWords_SQL, {
                     { SCE_SQL_DEFAULT, 63126, L"Default", L"", L"" },
                     { SCE_SQL_COMMENT, 63127, L"Comment", L"fore:#505050", L"" },
                     { SCE_SQL_WORD, 63128, L"Keyword", L"bold; fore:#800080", L"" },
                     { MULTI_STYLE(SCE_SQL_STRING,SCE_SQL_CHARACTER,0,0), 63131, L"String", L"fore:#008000; back:#FFF1A8", L"" },
                     { SCE_SQL_IDENTIFIER, 63129, L"Identifier", L"fore:#800080", L"" },
                     { SCE_SQL_QUOTEDIDENTIFIER, 63243, L"Quoted Identifier", L"fore:#800080; back:#FFCCFF", L"" },
                     { SCE_SQL_NUMBER, 63130, L"Number", L"fore:#FF0000", L"" },
                     { SCE_SQL_OPERATOR, 63132, L"Operator", L"bold; fore:#800080", L"" },
                     { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_PY = {
"and as assert break class continue def del elif else except "
"exec False finally for from global if import in is lambda None "
"nonlocal not or pass print raise return True try while with yield",
"", "", "", "", "", "", "", "" };


EDITLEXER lexPY = { SCLEX_PYTHON, 63019, L"Python Script", L"py; pyw", L"", &KeyWords_PY, {
                    { SCE_P_DEFAULT, 63126, L"Default", L"", L"" },
                    { MULTI_STYLE(SCE_P_COMMENTLINE,SCE_P_COMMENTBLOCK,0,0), 63127, L"Comment", L"fore:#880000", L"" },
                    { SCE_P_WORD, 63128, L"Keyword", L"fore:#000088", L"" },
                    { SCE_P_IDENTIFIER, 63129, L"Identifier", L"", L"" },
                    { MULTI_STYLE(SCE_P_STRING,SCE_P_STRINGEOL,0,0), 63211, L"String double quoted", L"fore:#008800", L"" },
                    { SCE_P_CHARACTER, 63212, L"String single quoted", L"fore:#008800", L"" },
                    { SCE_P_TRIPLEDOUBLE, 63244, L"String triple double quotes", L"fore:#008800", L"" },
                    { SCE_P_TRIPLE, 63245, L"String triple single quotes", L"fore:#008800", L"" },
                    { SCE_P_NUMBER, 63130, L"Number", L"fore:#FF4000", L"" },
                    { SCE_P_OPERATOR, 63132, L"Operator", L"bold; fore:#666600", L"" },
                    { SCE_P_DEFNAME, 63247, L"Function name", L"fore:#660066", L"" },
                    { SCE_P_CLASSNAME, 63246, L"Class name", L"fore:#660066", L"" },
                    { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_CONF = {
"acceptfilter acceptmutex acceptpathinfo accessconfig accessfilename action addalt addaltbyencoding "
"addaltbytype addcharset adddefaultcharset adddescription addencoding addhandler addicon addiconbyencoding "
"addiconbytype addinputfilter addlanguage addmodule addmoduleinfo addoutputfilter addoutputfilterbytype "
"addtype agentlog alias aliasmatch all allow allowconnect allowencodedslashes allowmethods allowoverride "
"allowoverridelist anonymous anonymous_authoritative anonymous_logemail anonymous_mustgiveemail "
"anonymous_nouserid anonymous_verifyemail assignuserid asyncrequestworkerfactor authauthoritative "
"authbasicauthoritative authbasicfake authbasicprovider authbasicusedigestalgorithm authdbauthoritative "
"authdbduserpwquery authdbduserrealmquery authdbgroupfile authdbmauthoritative authdbmgroupfile "
"authdbmtype authdbmuserfile authdbuserfile authdigestalgorithm authdigestdomain authdigestfile "
"authdigestgroupfile authdigestnccheck authdigestnonceformat authdigestnoncelifetime authdigestprovider "
"authdigestqop authdigestshmemsize authformauthoritative authformbody authformdisablenostore authformfakebasicauth "
"authformlocation authformloginrequiredlocation authformloginsuccesslocation authformlogoutlocation authformmethod "
"authformmimetype authformpassword authformprovider authformsitepassphrase authformsize authformusername "
"authgroupfile authldapauthoritative authldapauthorizeprefix authldapbindauthoritative authldapbinddn "
"authldapbindpassword authldapcharsetconfig authldapcompareasuser authldapcomparednonserver "
"authldapdereferencealiases authldapenabled authldapfrontpagehack authldapgroupattribute "
"authldapgroupattributeisdn authldapinitialbindasuser authldapinitialbindpattern authldapmaxsubgroupdepth "
"authldapremoteuserattribute authldapremoteuserisdn authldapsearchasuser authldapsubgroupattribute "
"authldapsubgroupclass authldapurl authmerging authname authncachecontext authncacheenable authncacheprovidefor "
"authncachesocache authncachetimeout authnprovideralias authnzfcgicheckauthnprovider authnzfcgidefineprovider "
"authtype authuserfile authzdbdlogintoreferer authzdbdquery authzdbdredirectquery authzdbmtype "
"authzsendforbiddenonfailure balancergrowth balancerinherit balancermember balancerpersist bindaddress "
"browsermatch browsermatchnocase bs2000account bufferedlogs buffersize cachedefaultexpire cachedetailheader "
"cachedirlength cachedirlevels cachedisable cacheenable cacheexpirycheck cachefile cacheforcecompletion "
"cachegcclean cachegcdaily cachegcinterval cachegcmemusage cachegcunused cacheheader cacheignorecachecontrol "
"cacheignoreheaders cacheignorenolastmod cacheignorequerystring cacheignoreurlsessionidentifiers "
"cachekeybaseurl cachelastmodifiedfactor cachelock cachelockmaxage cachelockpath cachemaxexpire "
"cachemaxfilesize cacheminexpire cacheminfilesize cachenegotiateddocs cachequickhandler cachereadsize "
"cachereadtime cacheroot cachesize cachesocache cachesocachemaxsize cachesocachemaxtime cachesocachemintime "
"cachesocachereadsize cachesocachereadtime cachestaleonerror cachestoreexpired cachestorenostore cachestoreprivate "
"cachetimemargin cgidscripttimeout cgimapextension cgipassauth cgivar charsetdefault charsetoptions charsetsourceenc "
"checkcaseonly checkspelling childperuserid chrootdir clearmodulelist contentdigest cookiedomain cookieexpires "
"cookielog cookiename cookiestyle cookietracking coredumpdirectory customlog dav davdepthinfinity davgenericlockdb "
"davlockdb davmintimeout dbdexptime dbdinitsql dbdkeep dbdmax dbdmin dbdparams dbdpersist dbdpreparesql dbdriver "
"defaulticon defaultlanguage defaultruntimedir defaulttype define deflatebuffersize deflatecompressionlevel "
"deflatefilternote deflateinflatelimitrequestbody deflateinflateratioburst deflateinflateratiolimit deflatememlevel "
"deflatewindowsize deny directory directorycheckhandler directoryindex directoryindexredirect directorymatch "
"directoryslash documentroot dtraceprivileges dumpioinput dumpiooutput else elseif enableexceptionhook enablemmap "
"enablesendfile error errordocument errorlog errorlogformat example expiresactive expiresbytype expiresdefault "
"extendedstatus extfilterdefine extfilteroptions fallbackresource fancyindexing fileetag files filesmatch "
"filterchain filterdeclare filterprotocol filterprovider filtertrace forcelanguagepriority forcetype forensiclog "
"from globallog gracefulshutdowntimeout group h2direct h2maxsessionstreams h2maxworkeridleseconds h2maxworkers "
"h2minworkers h2moderntlsonly h2push h2pushdiarysize h2pushpriority h2serializeheaders h2sessionextrafiles "
"h2streammaxmemsize h2tlscooldownsecs h2tlswarmupsize h2upgrade h2windowsize header headername heartbeataddress "
"heartbeatlisten heartbeatmaxservers heartbeatstorage hostnamelookups identitycheck identitychecktimeout "
"if ifdefine ifmodule ifversion imapbase imapdefault imapmenu include includeoptional indexheadinsert "
"indexignore indexignorereset indexoptions indexorderdefault indexstylesheet inputsed isapiappendlogtoerrors "
"isapiappendlogtoquery isapicachefile isapifakeasync isapilognotsupported isapireadaheadbuffer keepalive "
"keepalivetimeout keptbodysize languagepriority ldapcacheentries ldapcachettl ldapconnectionpoolttl "
"ldapconnectiontimeout ldaplibrarydebug ldapopcacheentries ldapopcachettl ldapreferralhoplimit ldapreferrals "
"ldapretries ldapretrydelay ldapsharedcachefile ldapsharedcachesize ldaptimeout ldaptrustedca ldaptrustedcatype "
"ldaptrustedclientcert ldaptrustedglobalcert ldaptrustedmode ldapverifyservercert limit limitexcept "
"limitinternalrecursion limitrequestbody limitrequestfields limitrequestfieldsize limitrequestline "
"limitxmlrequestbody listen listenbacklog listencoresbucketsratio loadfile loadmodule location "
"locationmatch lockfile logformat logiotrackttfb loglevel logmessage luaauthzprovider luacodecache "
"luahookaccesschecker luahookauthchecker luahookcheckuserid luahookfixups luahookinsertfilter luahooklog "
"luahookmaptostorage luahooktranslatename luahooktypechecker luainherit luainputfilter luamaphandler "
"luaoutputfilter luapackagecpath luapackagepath luaquickhandler luaroot luascope macro maxclients "
"maxconnectionsperchild maxkeepaliverequests maxmemfree maxrangeoverlaps maxrangereversals maxranges "
"maxrequestsperchild maxrequestsperthread maxrequestworkers maxspareservers maxsparethreads maxthreads "
"maxthreadsperchild mcachemaxobjectcount mcachemaxobjectsize mcachemaxstreamingbuffer mcacheminobjectsize "
"mcacheremovalalgorithm mcachesize memcacheconnttl mergetrailers metadir metafiles metasuffix mimemagicfile "
"minspareservers minsparethreads mmapfile modemstandard modmimeusepathinfo multiviewsmatch mutex namevirtualhost "
"nocache noproxy numservers nwssltrustedcerts nwsslupgradeable options order outputsed passenv pidfile port "
"privilegesmode protocol protocolecho protocols protocolshonororder proxy proxyaddheaders proxybadheader "
"proxyblock proxydomain proxyerroroverride proxyexpressdbmfile proxyexpressdbmtype proxyexpressenable "
"proxyftpdircharset proxyftpescapewildcards proxyftplistonwildcard proxyhcexpr proxyhctemplate proxyhctpsize "
"proxyhtmlbufsize proxyhtmlcharsetout proxyhtmldoctype proxyhtmlenable proxyhtmlevents proxyhtmlextended "
"proxyhtmlfixups proxyhtmlinterp proxyhtmllinks proxyhtmlmeta proxyhtmlstripcomments proxyhtmlurlmap "
"proxyiobuffersize proxymatch proxymaxforwards proxypass proxypassinherit proxypassinterpolateenv "
"proxypassmatch proxypassreverse proxypassreversecookiedomain proxypassreversecookiepath proxypreservehost "
"proxyreceivebuffersize proxyremote proxyremotematch proxyrequests proxyscgiinternalredirect proxyscgisendfile "
"proxyset proxysourceaddress proxystatus proxytimeout proxyvia qsc qualifyredirecturl readmename "
"receivebuffersize redirect redirectmatch redirectpermanent redirecttemp refererignore refererlog "
"reflectorheader remoteipheader remoteipinternalproxy remoteipinternalproxylist remoteipproxiesheader "
"remoteiptrustedproxy remoteiptrustedproxylist removecharset removeencoding removehandler removeinputfilter "
"removelanguage removeoutputfilter removetype requestheader requestreadtimeout require requireall "
"requireany requirenone resourceconfig rewritebase rewritecond rewriteengine rewritelock rewritelog "
"rewriteloglevel rewritemap rewriteoptions rewriterule rlimitcpu rlimitmem rlimitnproc satisfy "
"scoreboardfile script scriptalias scriptaliasmatch scriptinterpretersource scriptlog scriptlogbuffer "
"scriptloglength scriptsock securelisten seerequesttail sendbuffersize serveradmin serveralias serverlimit "
"servername serverpath serverroot serversignature servertokens servertype session sessioncookiename "
"sessioncookiename2 sessioncookieremove sessioncryptocipher sessioncryptodriver sessioncryptopassphrase "
"sessioncryptopassphrasefile sessiondbdcookiename sessiondbdcookiename2 sessiondbdcookieremove "
"sessiondbddeletelabel sessiondbdinsertlabel sessiondbdperuser sessiondbdselectlabel sessiondbdupdatelabel "
"sessionenv sessionexclude sessionheader sessioninclude sessionmaxage setenv setenvif setenvifexpr "
"setenvifnocase sethandler setinputfilter setoutputfilter singlelisten ssiendtag ssierrormsg ssietag "
"ssilastmodified ssilegacyexprparser ssistarttag ssitimeformat ssiundefinedecho sslcacertificatefile "
"sslcacertificatepath sslcadnrequestfile sslcadnrequestpath sslcarevocationcheck sslcarevocationfile "
"sslcarevocationpath sslcertificatechainfile sslcertificatefile sslcertificatekeyfile sslciphersuite "
"sslcompression sslcryptodevice sslengine sslfips sslhonorcipherorder sslinsecurerenegotiation sslmutex "
"sslocspdefaultresponder sslocspenable sslocspoverrideresponder sslocspproxyurl sslocsprespondertimeout "
"sslocspresponsemaxage sslocspresponsetimeskew sslocspuserequestnonce sslopensslconfcmd ssloptions "
"sslpassphrasedialog sslprotocol sslproxycacertificatefile sslproxycacertificatepath sslproxycarevocationcheck "
"sslproxycarevocationfile sslproxycarevocationpath sslproxycheckpeercn sslproxycheckpeerexpire "
"sslproxycheckpeername sslproxyciphersuite sslproxyengine sslproxymachinecertificatechainfile "
"sslproxymachinecertificatefile sslproxymachinecertificatepath sslproxyprotocol sslproxyverify "
"sslproxyverifydepth sslrandomseed sslrenegbuffersize sslrequire sslrequiressl sslsessioncache "
"sslsessioncachetimeout sslsessionticketkeyfile sslsessiontickets sslsrpunknownuserseed "
"sslsrpverifierfile sslstaplingcache sslstaplingerrorcachetimeout sslstaplingfaketrylater "
"sslstaplingforceurl sslstaplingrespondertimeout sslstaplingresponsemaxage sslstaplingresponsetimeskew "
"sslstaplingreturnrespondererrors sslstaplingstandardcachetimeout sslstrictsnivhostcheck sslusername "
"sslusestapling sslverifyclient sslverifydepth startservers startthreads substitute substituteinheritbefore "
"substitutemaxlinelength suexec suexecusergroup threadlimit threadsperchild threadstacksize timeout "
"traceenable transferlog typesconfig undefine undefmacro unsetenv use usecanonicalname usecanonicalphysicalport "
"user userdir vhostcgimode vhostcgiprivs vhostgroup vhostprivs vhostsecure vhostuser virtualdocumentroot "
"virtualdocumentrootip virtualhost virtualscriptalias virtualscriptaliasip watchdoginterval win32disableacceptex "
"xbithack xml2encalias xml2encdefault xml2startparse",
"", //"on off standalone inetd force-response-1.0 downgrade-1.0 nokeepalive indexes includes followsymlinks none x-compress x-gzip",
"", "", "", "", "", "", "" };


EDITLEXER lexCONF = { SCLEX_CONF, 63020, L"Apache Config Files", L"conf; htaccess", L"", &KeyWords_CONF, {
                      { SCE_CONF_DEFAULT, 63126, L"Default", L"", L"" },
                      { SCE_CONF_COMMENT, 63127, L"Comment", L"fore:#648000", L"" },
                      { SCE_CONF_STRING, 63131, L"String", L"fore:#B000B0", L"" },
                      { SCE_CONF_NUMBER, 63130, L"Number", L"fore:#FF4000", L"" },
                      { SCE_CONF_DIRECTIVE, 63203, L"Directive", L"fore:#003CE6", L"" },
                      { SCE_CONF_IP, 63248, L"IP Address", L"bold; fore:#FF4000", L"" },
// Not used by lexer  { SCE_CONF_IDENTIFIER, L"Identifier", L"", L"" },
// Lexer is buggy     { SCE_CONF_OPERATOR, L"Operator", L"", L"" },
// Lexer is buggy     { SCE_CONF_PARAMETER, L"Runtime Directive Parameter", L"", L"" },
// Lexer is buggy     { SCE_CONF_EXTENSION, L"Extension", L"", L"" },
                      { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_PS = {
"begin break catch continue data do dynamicparam else elseif end exit filter finally for foreach "
"from function if in local param private process return switch throw trap try until where while",
"add-computer add-content add-history add-member add-pssnapin add-type checkpoint-computer "
"clear-content clear-eventlog clear-history clear-host clear-item clear-itemproperty "
"clear-variable compare-object complete-transaction connect-wsman convertfrom-csv "
"convertfrom-securestring convertfrom-stringdata convert-path convertto-csv convertto-html "
"convertto-securestring convertto-xml copy-item copy-itemproperty debug-process "
"disable-computerrestore disable-psbreakpoint disable-psremoting disable-pssessionconfiguration "
"disable-wsmancredssp disconnect-wsman enable-computerrestore enable-psbreakpoint "
"enable-psremoting enable-pssessionconfiguration enable-wsmancredssp enter-pssession "
"exit-pssession export-alias export-clixml export-console export-counter export-csv "
"export-formatdata export-modulemember export-pssession foreach-object format-custom format-list "
"format-table format-wide get-acl get-alias get-authenticodesignature get-childitem get-command "
"get-computerrestorepoint get-content get-counter get-credential get-culture get-date get-event "
"get-eventlog get-eventsubscriber get-executionpolicy get-formatdata get-help get-history "
"get-host get-hotfix get-item get-itemproperty get-job get-location get-member get-module "
"get-pfxcertificate get-process get-psbreakpoint get-pscallstack get-psdrive get-psprovider "
"get-pssession get-pssessionconfiguration get-pssnapin get-random get-service get-tracesource "
"get-transaction get-uiculture get-unique get-variable get-verb get-winevent get-wmiobject "
"get-wsmancredssp get-wsmaninstance group-object import-alias import-clixml import-counter "
"import-csv import-localizeddata import-module import-pssession invoke-command invoke-expression "
"invoke-history invoke-item invoke-restmethod invoke-webrequest invoke-wmimethod "
"invoke-wsmanaction join-path limit-eventlog measure-command measure-object move-item "
"move-itemproperty new-alias new-event new-eventlog new-item new-itemproperty new-module "
"new-modulemanifest new-object new-psdrive new-pssession new-pssessionoption new-service "
"new-timespan new-variable new-webserviceproxy new-wsmaninstance new-wsmansessionoption "
"out-default out-file out-gridview out-host out-null out-printer out-string pop-location "
"push-location read-host receive-job register-engineevent register-objectevent "
"register-pssessionconfiguration register-wmievent remove-computer remove-event remove-eventlog "
"remove-item remove-itemproperty remove-job remove-module remove-psbreakpoint remove-psdrive "
"remove-pssession remove-pssnapin remove-variable remove-wmiobject remove-wsmaninstance "
"rename-item rename-itemproperty reset-computermachinepassword resolve-path restart-computer "
"restart-service restore-computer resume-service select-object select-string select-xml "
"send-mailmessage set-acl set-alias set-authenticodesignature set-content set-date "
"set-executionpolicy set-item set-itemproperty set-location set-psbreakpoint set-psdebug "
"set-pssessionconfiguration set-service set-strictmode set-tracesource set-variable "
"set-wmiinstance set-wsmaninstance set-wsmanquickconfig show-eventlog sort-object split-path "
"start-job start-process start-service start-sleep start-transaction start-transcript "
"stop-computer stop-job stop-process stop-service stop-transcript suspend-service tee-object "
"test-computersecurechannel test-connection test-modulemanifest test-path test-wsman "
"trace-command undo-transaction unregister-event unregister-pssessionconfiguration "
"update-formatdata update-list update-typedata use-transaction wait-event wait-job wait-process "
"where-object write-debug write-error write-eventlog write-host write-output write-progress "
"write-verbose write-warning",
"ac asnp cat cd chdir clc clear clhy cli clp cls clv compare copy cp cpi cpp cvpa dbp del diff "
"dir ebp echo epal epcsv epsn erase etsn exsn fc fl foreach ft fw gal gbp gc gci gcm gcs gdr ghy "
"gi gjb gl gm gmo gp gps group gsn gsnp gsv gu gv gwmi h help history icm iex ihy ii ipal ipcsv "
"ipmo ipsn ise iwmi kill lp ls man md measure mi mkdir more mount move mp mv nal ndr ni nmo nsn "
"nv ogv oh popd ps pushd pwd r rbp rcjb rd rdr ren ri rjb rm rmdir rmo rni rnp rp rsn rsnp rv "
"rvpa rwmi sajb sal saps sasv sbp sc select set si sl sleep sort sp spjb spps spsv start sv swmi "
"tee type where wjb write",
"importsystemmodules prompt psedit tabexpansion",
"", "", "", "", "" };


EDITLEXER lexPS = { SCLEX_POWERSHELL, 63021, L"PowerShell Script", L"ps1; psd1; psm1", L"", &KeyWords_PS, {
                    { SCE_POWERSHELL_DEFAULT, 63126, L"Default", L"", L"" },
                    { MULTI_STYLE(SCE_POWERSHELL_COMMENT,SCE_POWERSHELL_COMMENTSTREAM,0,0), 63127, L"Comment", L"fore:#646464", L"" },
                    { SCE_POWERSHELL_KEYWORD, 63128, L"Keyword", L"bold; fore:#804000", L"" },
                    { SCE_POWERSHELL_IDENTIFIER, 63129, L"Identifier", L"", L"" },
                    { MULTI_STYLE(SCE_POWERSHELL_STRING,SCE_POWERSHELL_CHARACTER,0,0), 63131, L"String", L"fore:#008000", L"" },
                    { SCE_POWERSHELL_NUMBER, 63130, L"Number", L"fore:#FF0000", L"" },
                    { SCE_POWERSHELL_OPERATOR, 63132, L"Operator", L"bold", L"" },
                    { SCE_POWERSHELL_VARIABLE, 63249, L"Variable", L"fore:#0A246A", L"" },
                    { MULTI_STYLE(SCE_POWERSHELL_CMDLET,SCE_POWERSHELL_FUNCTION,0,0), 63250, L"Cmdlet", L"fore:#804000; back:#FFF1A8", L"" },
                    { SCE_POWERSHELL_ALIAS, 63251, L"Alias", L"bold; fore:#0A246A", L"" },
                    { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_NSIS = {
"!addincludedir !addplugindir !appendfile !cd !define !delfile !echo !else !endif !error "
"!execute !finalize !getdllversion !if !ifdef !ifmacrodef !ifmacrondef !ifndef !include !insertmacro !macro "
"!macroend !macroundef !makensis !packhdr !searchparse !searchreplace !system !tempfile !undef !verbose !warning "
".onguiend .onguiinit .oninit .oninstfailed .oninstsuccess .onmouseoversection .onrebootfailed .onselchange "
".onuserabort .onverifyinstdir un.onguiend un.onguiinit un.oninit un.onrebootfailed un.onuninstfailed un.onuninstsuccess "
"un.onuserabort abort addbrandingimage addsize allowrootdirinstall allowskipfiles autoclosewindow "
"bannertrimpath bgfont bggradient brandingtext bringtofront call callinstdll caption changeui checkbitmap "
"clearerrors completedtext componenttext copyfiles crccheck createdirectory createfont createshortcut "
"delete deleteinisec deleteinistr deleteregkey deleteregvalue detailprint detailsbuttontext dirstate dirtext "
"dirvar dirverify enablewindow enumregkey enumregvalue exch exec execshell execwait expandenvstrings "
"file filebufsize fileclose fileerrortext fileexists fileopen fileread filereadbyte filereadutf16le filereadword "
"fileseek filewrite filewritebyte filewriteutf16le filewriteword findclose findfirst findnext findproc "
"findwindow flushini getcurinsttype getcurrentaddress getdlgitem getdllversion getdllversionlocal "
"geterrorlevel getfiletime getfiletimelocal getfontname getfontnamelocal getfontversion getfontversionlocal "
"getfullpathname getfunctionaddress getinstdirerror getlabeladdress gettempfilename goto hidewindow icon "
"ifabort iferrors iffileexists ifrebootflag ifsilent initpluginsdir installbuttontext installcolors installdir "
"installdirregkey instprogressflags insttype insttypegettext insttypesettext intcmp intcmpu intfmt intop "
"iswindow langstring licensebkcolor licensedata licenseforceselection licenselangstring licensetext "
"loadlanguagefile lockwindow logset logtext manifestsupportedos messagebox miscbuttontext name nop outfile page "
"pagecallbacks pop push quit readenvstr readinistr readregdword readregstr reboot regdll rename requestexecutionlevel "
"reservefile return rmdir searchpath sectiongetflags sectiongetinsttypes sectiongetsize sectiongettext sectionin "
"sectionsetflags sectionsetinsttypes sectionsetsize sectionsettext sendmessage setautoclose setbrandingimage "
"setcompress setcompressionlevel setcompressor setcompressordictsize setctlcolors setcurinsttype "
"setdatablockoptimize setdatesave setdetailsprint setdetailsview seterrorlevel seterrors setfileattributes "
"setfont setoutpath setoverwrite setpluginunload setrebootflag setregview setshellvarcontext setsilent "
"showinstdetails showuninstdetails showwindow silentinstall silentuninstall sleep spacetexts strcmp strcmps "
"strcpy strlen subcaption unicode uninstallbuttontext uninstallcaption uninstallicon uninstallsubcaption uninstalltext "
"uninstpage unregdll var viaddversionkey vifileversion viproductversion windowicon writeinistr writeregbin "
"writeregdword writeregexpandstr writeregstr writeuninstaller xpstyle",
"${nsisdir} $0 $1 $2 $3 $4 $5 $6 $7 $8 $9 $r0 $r1 $r2 $r3 $r4 $r5 $r6 $r7 $r8 $r9 $instdir $outdir $cmdline "
"$language $programfiles $programfiles32 $programfiles64 $commonfiles $commonfiles32 $commonfiles64 "
"$desktop $exedir $exefile $exepath $windir $sysdir $temp $startmenu $smprograms $smstartup $quicklaunch "
"$documents $sendto $recent $favorites $music $pictures $videos $nethood $fonts $templates $appdata "
"$localappdata $printhood $internet_cache $cookies $history $profile $admintools $resources $resources_localized "
"$cdburn_area $hwndparent $pluginsdir ${__date__} ${__file__} ${__function__} ${__global__} ${__line__} "
"${__pageex__} ${__section__} ${__time__} ${__timestamp__} ${__uninstall__}",
"alt charset colored control cur date end global ignorecase leave shift smooth utcdate sw_hide sw_showmaximized "
"sw_showminimized sw_shownormal archive auto oname rebootok nonfatal ifempty nounload filesonly short mb_ok "
"mb_okcancel mb_abortretryignore mb_retrycancel mb_yesno mb_yesnocancel mb_iconexclamation mb_iconinformation "
"mb_iconquestion mb_iconstop mb_usericon mb_topmost mb_setforeground mb_right mb_rtlreading mb_defbutton1 "
"mb_defbutton2 mb_defbutton3 mb_defbutton4 idabort idcancel idignore idno idok idretry idyes sd current all "
"timeout imgid resizetofit listonly textonly both branding hkcr hkey_classes_root hklm hkey_local_machine hkcu "
"hkey_current_user hku hkey_users hkcc hkey_current_config hkdd hkey_dyn_data hkpd hkey_performance_data shctx "
"shell_context left right top bottom true false on off italic underline strike trimleft trimright trimcenter "
"idd_license idd_dir idd_selcom idd_inst idd_instfiles idd_uninst idd_verify force windows nocustom customstring "
"componentsonlyoncustom gray none user highest admin lang hide show nevershow normal silent silentlog solid final "
"zlib bzip2 lzma try ifnewer ifdiff lastused manual alwaysoff normal file_attribute_normal file_attribute_archive "
"hidden file_attribute_hidden offline file_attribute_offline readonly file_attribute_readonly system "
"file_attribute_system temporary file_attribute_temporary custom license components directory instfiles "
"uninstconfirm 32 64 enablecancel noworkingdir plugin rawnl winvista win7 win8 win8.1 win10",
"", "", "", "", "", "" };


EDITLEXER lexNSIS = { SCLEX_NSIS, 63284, L"NSIS Script", L"nsi; nsh", L"", &KeyWords_NSIS, {
                      { SCE_NSIS_DEFAULT, 63126, L"Default", L"", L"" },
                      { MULTI_STYLE(SCE_NSIS_COMMENT,SCE_NSIS_COMMENTBOX,0,0), 63127, L"Comment", L"fore:#008000", L"" },
                      { MULTI_STYLE(SCE_NSIS_STRINGDQ,SCE_NSIS_STRINGLQ,SCE_NSIS_STRINGRQ,0), 63131, L"String", L"fore:#666666; back:#EEEEEE", L"" },
                      { SCE_NSIS_FUNCTION, 63277, L"Function", L"fore:#0033CC", L"" },
                      { SCE_NSIS_VARIABLE, 63249, L"Variable", L"fore:#CC3300", L"" },
                      { SCE_NSIS_STRINGVAR, 63285, L"Variable within String", L"fore:#CC3300; back:#EEEEEE", L"" },
                      { SCE_NSIS_NUMBER, 63130, L"Number", L"fore:#FF0000", L"" },
                      { SCE_NSIS_LABEL, 63286, L"Constant", L"fore:#FF9900", L"" },
                      { SCE_NSIS_SECTIONDEF, 63232, L"Section", L"fore:#0033CC", L"" },
                      { SCE_NSIS_SUBSECTIONDEF, 63287, L"Sub Section", L"fore:#0033CC", L"" },
                      { SCE_NSIS_SECTIONGROUP, 63288, L"Section Group", L"fore:#0033CC", L"" },
                      { SCE_NSIS_FUNCTIONDEF, 63289, L"Function Definition", L"fore:#0033CC", L"" },
                      { SCE_NSIS_PAGEEX, 63290, L"PageEx", L"fore:#0033CC", L"" },
                      { SCE_NSIS_IFDEFINEDEF, 63291, L"If Definition", L"fore:#0033CC", L"" },
                      { SCE_NSIS_MACRODEF, 63292, L"Macro Definition", L"fore:#0033CC", L"" },
                      //{ SCE_NSIS_USERDEFINED, L"User Defined", L"", L"" },
                      { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_INNO = {
"code components custommessages dirs files icons ini installdelete langoptions languages messages "
"registry run setup types tasks uninstalldelete uninstallrun _istool",
"allowcancelduringinstall allownetworkdrive allownoicons allowrootdirectory allowuncpath alwaysrestart "
"alwaysshowcomponentslist alwaysshowdironreadypage alwaysshowgrouponreadypage alwaysusepersonalgroup appcomments "
"appcontact appcopyright appenddefaultdirname appenddefaultgroupname appid appmodifypath appmutex appname apppublisher "
"apppublisherurl appreadmefile appsupportphone appsupporturl appupdatesurl appvername appversion architecturesallowed "
"architecturesinstallin64bitmode backcolor backcolor2 backcolordirection backsolid beveledlabel changesassociations "
"changesenvironment closeapplications closeapplicationsfilter compression compressionthreads copyrightfontname "
"copyrightfontsize createappdir createuninstallregkey defaultdirname defaultgroupname defaultuserinfoname "
"defaultuserinfoorg defaultuserinfoserial dialogfontname dialogfontsize direxistswarning disabledirpage "
"disablefinishedpage disableprogramgrouppage disablereadymemo disablereadypage disablestartupprompt "
"disablewelcomepage diskclustersize diskslicesize diskspanning enabledirdoesntexistwarning encryption "
"extradiskspacerequired flatcomponentslist infoafterfile infobeforefile internalcompresslevel languagedetectionmethod "
"languagecodepage languageid languagename licensefile lzmaalgorithm lzmablocksize lzmadictionarysize lzmamatchfinder "
"lzmanumblockthreads lzmanumfastbytes lzmauseseparateprocess mergeduplicatefiles minversion onlybelowversion "
"outputbasefilename outputdir outputmanifestfile password privilegesrequired reservebytes restartapplications "
"restartifneededbyrun righttoleft setupiconfile setuplogging setupmutex showcomponentsizes showlanguagedialog showtaskstreelines "
"showundisplayablelanguages signeduninstaller signeduninstallerdir signtool signtoolretrycount slicesperdisk solidcompression "
"sourcedir strongassemblyname timestamprounding timestampsinutc titlefontname titlefontsize touchdate touchtime uninstallable "
"uninstalldisplayicon uninstalldisplayname uninstallfilesdir uninstalldisplaysize uninstalllogmode uninstallrestartcomputer "
"updateuninstalllogappname usepreviousappdir usepreviousgroup usepreviouslanguage useprevioussetuptype useprevioustasks "
"verb versioninfoproductname useprevioususerinfo userinfopage usesetupldr versioninfocompany versioninfocopyright "
"versioninfodescription versioninfoproductversion versioninfotextversion versioninfoversion versioninfoproducttextversion "
"welcomefontname welcomefontsize windowshowcaption windowstartmaximized windowresizable windowvisible wizardimagealphaformat "
"wizardimagebackcolor wizardimagefile wizardimagestretch wizardsmallimagefile",
"appusermodelid afterinstall attribs beforeinstall check comment components copymode description destdir destname excludes "
"extradiskspacerequired filename flags fontinstall groupdescription hotkey infoafterfile infobeforefile iconfilename "
"iconindex key languages licensefile messagesfile minversion name onlybelowversion parameters permissions root runonceid "
"section source statusmsg string subkey tasks terminalservicesaware type types valuedata valuename valuetype workingdir",
"append define dim else emit elif endif endsub error expr file for if ifdef ifexist ifndef ifnexist include insert pragma "
"sub undef",
"and begin break case const continue do downto else end except finally for function "
"if not of or procedure repeat then to try type until uses var while with",
"", "", "", "" };


EDITLEXER lexINNO = { SCLEX_INNOSETUP, 63293, L"Inno Setup Script", L"iss; isl; islu", L"", &KeyWords_INNO, {
                      { SCE_INNO_DEFAULT, 63126, L"Default", L"", L"" },
                      { SCE_INNO_COMMENT, 63127, L"Comment", L"fore:#008000", L"" },
                      { SCE_INNO_KEYWORD, 63128, L"Keyword", L"fore:#0000FF", L"" },
                      { SCE_INNO_PARAMETER, 63294, L"Parameter", L"fore:#0000FF", L"" },
                      { SCE_INNO_SECTION, 63232, L"Section", L"fore:#000080; bold", L"" },
                      { SCE_INNO_PREPROC, 63133, L"Preprocessor", L"fore:#CC0000", L"" },
                      { SCE_INNO_INLINE_EXPANSION, 63295, L"Inline Expansion", L"fore:#800080", L"" },
                      { SCE_INNO_COMMENT_PASCAL, 63296, L"Pascal Comment", L"fore:#008000", L"" },
                      { SCE_INNO_KEYWORD_PASCAL, 63297, L"Pascal Keyword", L"fore:#0000FF", L"" },
                      { MULTI_STYLE(SCE_INNO_STRING_DOUBLE,SCE_INNO_STRING_SINGLE,0,0), 63131, L"String", L"", L"" },
                      //{ SCE_INNO_IDENTIFIER, 63129, L"Identifier", L"", L"" },
                      //{ SCE_INNO_KEYWORD_USER, L"User Defined", L"", L"" },
                      { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_RUBY = {
"__FILE__ __LINE__ alias and begin break case class def defined? do else elsif end ensure "
"false for in if module next nil not or redo rescue retry return self super then true "
"undef unless until when while yield",
"", "", "", "", "", "", "", "" };

EDITLEXER lexRUBY = { SCLEX_RUBY, 63304, L"Ruby Script", L"rb; ruby; rbw; rake; rjs; Rakefile; gemspec", L"", &KeyWords_RUBY, {
                    { SCE_RB_DEFAULT, 63126, L"Default", L"", L"" },
                    { MULTI_STYLE(SCE_RB_COMMENTLINE,SCE_P_COMMENTBLOCK,0,0), 63127, L"Comment", L"fore:#008000", L"" },
                    { SCE_RB_WORD, 63128, L"Keyword", L"fore:#00007F", L"" },
                    { SCE_RB_IDENTIFIER, 63129, L"Identifier", L"", L"" },
                    { SCE_RB_NUMBER, 63130, L"Number", L"fore:#008080", L"" },
                    { SCE_RB_OPERATOR, 63132, L"Operator", L"", L"" },
                    { MULTI_STYLE(SCE_RB_STRING,SCE_RB_CHARACTER,SCE_P_STRINGEOL,0), 63131, L"String", L"fore:#FF8000", L"" },
                    { SCE_RB_CLASSNAME, 63246, L"Class name", L"fore:#0000FF", L"" },
                    { SCE_RB_DEFNAME, 63247, L"Function name", L"fore:#007F7F", L"" },
                    { SCE_RB_POD, 63314, L"POD", L"fore:#004000; back:#C0FFC0; eolfilled", L"" },
                    { SCE_RB_REGEX, 63315, L"Regex", L"fore:#000000; back:#A0FFA0", L"" },
                    { SCE_RB_SYMBOL, 63316, L"Symbol", L"fore:#C0A030", L"" },
                    { SCE_RB_MODULE_NAME, 63317, L"Module name", L"fore:#A000A0", L"" },
                    { SCE_RB_INSTANCE_VAR, 63318, L"Instance Var", L"fore:#B00080", L"" },
                    { SCE_RB_CLASS_VAR, 63319, L"Class Var", L"fore:#8000B0", L"" },
                    { SCE_RB_DATASECTION, 63320, L"Data Section", L"fore:#600000; back:#FFF0D8; eolfilled", L"" },
                    { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_LUA = {
"and break do else elseif end false for function goto if "
"in local nil not or repeat return then true until while",
// Basic Functions
"_VERSION assert collectgarbage dofile error gcinfo loadfile loadstring print rawget rawset "
"require tonumber tostring type unpack _ALERT _ERRORMESSAGE _INPUT _PROMPT _OUTPUT _STDERR "
"_STDIN _STDOUT call dostring foreach foreachi getn globals newtype sort tinsert tremove "
"_G getfenv getmetatable ipairs loadlib next pairs pcall rawequal setfenv setmetatable xpcall "
"string table math coroutine io os debug load module select",
// String Manipulation, Table Manipulation, Mathematical Functions
"abs acos asin atan atan2 ceil cos deg exp floor format frexp gsub ldexp log log10 max min "
"mod rad random randomseed sin sqrt strbyte strchar strfind strlen strlower strrep strsub strupper tan "
"string.byte string.char string.dump string.find string.len string.lower string.rep string.sub string.upper "
"string.format string.gfind string.gsub table.concat table.foreach table.foreachi table.getn table.sort "
"table.insert table.remove table.setn math.abs math.acos math.asin math.atan math.atan2 math.ceil math.cos "
"math.deg math.exp math.floor math.frexp math.ldexp math.log math.log10 math.max math.min math.mod "
"math.pi math.pow math.rad math.random math.randomseed math.sin math.sqrt math.tan string.gmatch "
"string.match string.reverse table.maxn math.cosh math.fmod math.modf math.sinh math.tanh math.huge",
// Input and Output Facilities & System Facilities Coroutine Manipulation,
//Input and Output Facilities, System Facilities (coroutine & io & os)
"openfile closefile readfrom writeto appendto remove rename flush seek tmpfile tmpname read "
"write clock date difftime execute exit getenv setlocale time coroutine.create coroutine.resume "
"coroutine.status coroutine.wrap coroutine.yield io.close io.flush io.input io.lines io.open io.output "
"io.read io.tmpfile io.type io.write io.stdin io.stdout io.stderr os.clock os.date os.difftime "
"os.execute os.exit os.getenv os.remove os.rename os.setlocale os.time os.tmpname coroutine.running "
"package.cpath package.loaded package.loadlib package.path package.preload package.seeall io.popen",
"", "", "", "", "" };


EDITLEXER lexLUA = { SCLEX_LUA, 63298, L"Lua Script", L"lua", L"", &KeyWords_LUA, {
                    { SCE_LUA_DEFAULT, 63126, L"Default", L"", L"" },
                    { MULTI_STYLE(SCE_LUA_COMMENT,SCE_LUA_COMMENTLINE,SCE_LUA_COMMENTDOC,0), 63127, L"Comment", L"fore:#008000", L"" },
                    { SCE_LUA_NUMBER, 63130, L"Number", L"fore:#008080", L"" },
                    { SCE_LUA_WORD, 63128, L"Keyword", L"fore:#00007F", L"" },
                    { SCE_LUA_WORD2, 63321, L"Basic Functions", L"fore:#00007F", L"" },
                    { SCE_LUA_WORD3, 63322, L"String, Table & Math Functions", L"fore:#00007F", L"" },
                    { SCE_LUA_WORD4, 63323, L"Input, Output & System Facilities", L"fore:#00007F", L"" },
                    { MULTI_STYLE(SCE_LUA_STRING,SCE_LUA_STRINGEOL,SCE_LUA_CHARACTER,0), 63131, L"String", L"fore:#B000B0", L"" },
                    { SCE_LUA_LITERALSTRING, 63302, L"Literal String", L"fore:#B000B0", L"" },
                    { SCE_LUA_PREPROCESSOR, 63133, L"Preprocessor", L"fore:#FF8000", L"" },
                    { SCE_LUA_OPERATOR, 63132, L"Operator", L"", L"" },
                    { SCE_LUA_IDENTIFIER, 63129, L"Identifier", L"", L"" },
                    { SCE_LUA_LABEL, 63235, L"Label", L"fore:#808000", L"" },
                    { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_BASH = {
"alias ar asa awk banner basename bash bc bdiff break bunzip2 bzip2 cal calendar case cat "
"cc cd chmod cksum clear cmp col comm compress continue cp cpio crypt csplit ctags cut date "
"dc dd declare deroff dev df diff diff3 dircmp dirname do done du echo ed egrep elif else "
"env esac eval ex exec exit expand export expr false fc fgrep fi file find fmt fold for function "
"functions getconf getopt getopts grep gres hash head help history iconv id if in integer "
"jobs join kill local lc let line ln logname look ls m4 mail mailx make man mkdir more mt mv "
"newgrp nl nm nohup ntps od pack paste patch pathchk pax pcat perl pg pr print printf ps pwd "
"read readonly red return rev rm rmdir sed select set sh shift size sleep sort spell split "
"start stop strings strip stty sum suspend sync tail tar tee test then time times touch tr "
"trap true tsort tty type typeset ulimit umask unalias uname uncompress unexpand uniq unpack "
"unset until uudecode uuencode vi vim vpax wait wc whence which while who wpaste wstart xargs "
"zcat chgrp chown chroot dir dircolors factor groups hostid install link md5sum mkfifo mknod "
"nice pinky printenv ptx readlink seq sha1sum shred stat su tac unlink users vdir whoami yes",
"", "", "", "", "", "", "", "" };


EDITLEXER lexBASH = { SCLEX_BASH, 63259, L"Shell Script", L"sh", L"", &KeyWords_BASH, {
                      { SCE_SH_DEFAULT, 63126, L"Default", L"", L"" },
                      { SCE_SH_ERROR, 63261, L"Error", L"", L"" },
                      { SCE_SH_COMMENTLINE, 63127, L"Comment", L"fore:#008000", L"" },
                      { SCE_SH_NUMBER, 63130, L"Number", L"fore:#008080", L"" },
                      { SCE_SH_WORD, 63128, L"Keyword", L"fore:#0000FF", L"" },
                      { SCE_SH_STRING, 63211, L"String double quoted", L"fore:#008080", L"" },
                      { SCE_SH_CHARACTER, 63212, L"String single quoted", L"fore:#800080", L"" },
                      { SCE_SH_OPERATOR, 63132, L"Operator", L"", L"" },
                      { SCE_SH_IDENTIFIER, 63129, L"Identifier", L"", L"" },
                      { SCE_SH_SCALAR, 63268, L"Scalar", L"fore:#808000", L"" },
                      { SCE_SH_PARAM, 63269, L"Parameter expansion", L"fore:#808000; back:#FFFF99", L"" },
                      { SCE_SH_BACKTICKS, 63270, L"Back Ticks", L"fore:#FF0080", L"" },
                      { SCE_SH_HERE_DELIM, 63271, L"Here-doc (Delimiter)", L"", L"" },
                      { SCE_SH_HERE_Q, 63272, L"Here-doc (Single quoted, q)", L"fore:#008080", L"" },
                      { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_TCL = {
// TCL Keywords
"after append array auto_execok auto_import auto_load auto_load_index auto_qualify beep "
"bgerror binary break case catch cd clock close concat continue dde default echo else "
"elseif encoding eof error eval exec exit expr fblocked fconfigure fcopy file fileevent "
"flush for foreach format gets glob global history http if incr info interp join lappend "
"lindex linsert list llength load loadTk lrange lreplace lsearch lset lsort memory msgcat "
"namespace open package pid pkg::create pkg_mkIndex Platform-specific proc puts pwd "
"re_syntax read regexp registry regsub rename resource return scan seek set socket source "
"split string subst switch tclLog tclMacPkgSearch tclPkgSetup tclPkgUnknown tell time trace "
"unknown unset update uplevel upvar variable vwait while",
// TK Keywords
"bell bind bindtags bitmap button canvas checkbutton clipboard colors console cursors "
"destroy entry event focus font frame grab grid image Inter-client keysyms label labelframe "
"listbox lower menu menubutton message option options pack panedwindow photo place "
"radiobutton raise scale scrollbar selection send spinbox text tk tk_chooseColor "
"tk_chooseDirectory tk_dialog tk_focusNext tk_getOpenFile tk_messageBox tk_optionMenu "
"tk_popup tk_setPalette tkerror tkvars tkwait toplevel winfo wish wm",
// iTCL Keywords
"@scope body class code common component configbody constructor define destructor hull "
"import inherit itcl itk itk_component itk_initialize itk_interior itk_option iwidgets keep "
"method private protected public",
"", "", "", "", "", "" };


#define SCE_TCL__MULTI_COMMENT      MULTI_STYLE(SCE_TCL_COMMENT,SCE_TCL_COMMENTLINE,SCE_TCL_COMMENT_BOX,SCE_TCL_BLOCK_COMMENT)
#define SCE_TCL__MULTI_KEYWORD      MULTI_STYLE(SCE_TCL_WORD,SCE_TCL_WORD2,SCE_TCL_WORD3,SCE_TCL_WORD_IN_QUOTE)
#define SCE_TCL__MULTI_SUBSTITUTION MULTI_STYLE(SCE_TCL_SUBSTITUTION,SCE_TCL_SUB_BRACE,0,0)


EDITLEXER lexTCL = { SCLEX_TCL, 63273, L"Tcl Script", L"tcl; itcl", L"", &KeyWords_TCL, {
                     { SCE_TCL_DEFAULT, 63126, L"Default", L"", L"" },
                     { SCE_TCL__MULTI_COMMENT, 63127, L"Comment", L"fore:#008000", L"" },
                     { SCE_TCL__MULTI_KEYWORD, 63128, L"Keyword", L"fore:#0000FF", L"" },
                     { SCE_TCL_NUMBER, 63130, L"Number", L"fore:#008080", L"" },
                     { SCE_TCL_IN_QUOTE, 63131, L"String", L"fore:#008080", L"" },
                     { SCE_TCL_OPERATOR, 63132, L"Operator", L"", L"" },
                     { SCE_TCL_IDENTIFIER, 63129, L"Identifier", L"fore:#800080", L"" },
                     { SCE_TCL__MULTI_SUBSTITUTION, 63274, L"Substitution", L"fore:#CC0000", L"" },
                     { SCE_TCL_MODIFIER, 63275, L"Modifier", L"fore:#FF00FF", L"" },
                     { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_AU3 = {
"and byref case const continuecase continueloop default dim do else elseif endfunc endif "
"endselect endswitch endwith enum exit exitloop false for func global if in local next not "
"or redim return select static step switch then to true until wend while with",
"abs acos adlibregister adlibunregister asc ascw asin assign atan autoitsetoption autoitwingettitle "
"autoitwinsettitle beep binary binarylen binarymid binarytostring bitand bitnot bitor bitrotate "
"bitshift bitxor blockinput break call cdtray ceiling chr chrw clipget clipput consoleread "
"consolewrite consolewriteerror controlclick controlcommand controldisable controlenable "
"controlfocus controlgetfocus controlgethandle controlgetpos controlgettext controlhide "
"controllistview controlmove controlsend controlsettext controlshow controltreeview cos dec "
"dircopy dircreate dirgetsize dirmove dirremove dllcall dllcalladdress dllcallbackfree dllcallbackgetptr "
"dllcallbackregister dllclose dllopen dllstructcreate dllstructgetdata dllstructgetptr "
"dllstructgetsize dllstructsetdata drivegetdrive drivegetfilesystem drivegetlabel drivegetserial "
"drivegettype drivemapadd drivemapdel drivemapget drivesetlabel drivespacefree drivespacetotal "
"drivestatus envget envset envupdate eval execute exp filechangedir fileclose filecopy "
"filecreatentfslink filecreateshortcut filedelete fileexists filefindfirstfile filefindnextfile "
"fileflush filegetattrib filegetencoding filegetlongname filegetpos filegetshortcut filegetshortname "
"filegetsize filegettime filegetversion fileinstall filemove fileopen fileopendialog fileread "
"filereadline filerecycle filerecycleempty filesavedialog fileselectfolder filesetattrib filesetpos "
"filesettime filewrite filewriteline floor ftpsetproxy guicreate guictrlcreateavi guictrlcreatebutton "
"guictrlcreatecheckbox guictrlcreatecombo guictrlcreatecontextmenu guictrlcreatedate guictrlcreatedummy "
"guictrlcreateedit guictrlcreategraphic guictrlcreategroup guictrlcreateicon guictrlcreateinput "
"guictrlcreatelabel guictrlcreatelist guictrlcreatelistview guictrlcreatelistviewitem guictrlcreatemenu "
"guictrlcreatemenuitem guictrlcreatemonthcal guictrlcreateobj guictrlcreatepic guictrlcreateprogress "
"guictrlcreateradio guictrlcreateslider guictrlcreatetab guictrlcreatetabitem guictrlcreatetreeview "
"guictrlcreatetreeviewitem guictrlcreateupdown guictrldelete guictrlgethandle guictrlgetstate "
"guictrlread guictrlrecvmsg guictrlregisterlistviewsort guictrlsendmsg guictrlsendtodummy "
"guictrlsetbkcolor guictrlsetcolor guictrlsetcursor guictrlsetdata guictrlsetdefbkcolor "
"guictrlsetdefcolor guictrlsetfont guictrlsetgraphic guictrlsetimage guictrlsetlimit guictrlsetonevent "
"guictrlsetpos guictrlsetresizing guictrlsetstate guictrlsetstyle guictrlsettip guidelete "
"guigetcursorinfo guigetmsg guigetstyle guiregistermsg guisetaccelerators guisetbkcolor guisetcoord "
"guisetcursor guisetfont guisethelp guiseticon guisetonevent guisetstate guisetstyle guistartgroup "
"guiswitch hex hotkeyset httpsetproxy httpsetuseragent hwnd inetclose inetget inetgetinfo inetgetsize "
"inetread inidelete iniread inireadsection inireadsectionnames inirenamesection iniwrite iniwritesection "
"inputbox int isadmin isarray isbinary isbool isdeclared isdllstruct isfloat ishwnd isint iskeyword "
"isnumber isobj isptr isstring log memgetstats mod mouseclick mouseclickdrag mousedown mousegetcursor "
"mousegetpos mousemove mouseup mousewheel msgbox number objcreate objcreateinterface objevent objevent "
"objget objname onautoitexitregister onautoitexitunregister opt ping pixelchecksum pixelgetcolor "
"pixelsearch pluginclose pluginopen processclose processexists processgetstats processlist "
"processsetpriority processwait processwaitclose progressoff progresson progressset ptr random regdelete "
"regenumkey regenumval regread regwrite round run runas runaswait runwait send sendkeepactive "
"seterror setextended shellexecute shellexecutewait shutdown sin sleep soundplay soundsetwavevolume "
"splashimageon splashoff splashtexton sqrt srandom statusbargettext stderrread stdinwrite "
"stdioclose stdoutread string stringaddcr stringcompare stringformat stringfromasciiarray stringinstr "
"stringisalnum stringisalpha stringisascii stringisdigit stringisfloat stringisint stringislower "
"stringisspace stringisupper stringisxdigit stringleft stringlen stringlower stringmid "
"stringregexp stringregexpreplace stringreplace stringright stringsplit stringstripcr stringstripws "
"stringtoasciiarray stringtobinary stringtrimleft stringtrimright stringupper tan tcpaccept "
"tcpclosesocket tcpconnect tcplisten tcpnametoip tcprecv tcpsend tcpshutdown tcpstartup "
"timerdiff timerinit tooltip traycreateitem traycreatemenu traygetmsg trayitemdelete "
"trayitemgethandle trayitemgetstate trayitemgettext trayitemsetonevent trayitemsetstate "
"trayitemsettext traysetclick trayseticon traysetonevent traysetpauseicon traysetstate "
"traysettooltip traytip ubound udpbind udpclosesocket udpopen udprecv udpsend udpshutdown "
"udpstartup vargettype winactivate winactive winclose winexists winflash wingetcaretpos "
"wingetclasslist wingetclientsize wingethandle wingetpos wingetprocess wingetstate "
"wingettext wingettitle winkill winlist winmenuselectitem winminimizeall winminimizeallundo "
"winmove winsetontop winsetstate winsettitle winsettrans winwait winwaitactive winwaitclose "
"winwaitnotactive",
"@appdatacommondir @appdatadir @autoitexe @autoitpid @autoitunicode @autoitversion @autoitx64 "
"@com_eventobj @commonfilesdir @compiled @computername @comspec @cpuarch @cr @crlf @desktopcommondir "
"@desktopdepth @desktopdir @desktopheight @desktoprefresh @desktopwidth @documentscommondir "
"@error @exitcode @exitmethod @extended @favoritescommondir @favoritesdir @gui_ctrlhandle "
"@gui_ctrlid @gui_dragfile @gui_dragid @gui_dropid @gui_winhandle @homedrive @homepath @homeshare "
"@hotkeypressed @hour @inetgetactive @inetgetbytesread @ipaddress1 @ipaddress2 @ipaddress3 "
"@ipaddress4 @kblayout @lf @logondnsdomain @logondomain @logonserver @mday @min @mon @msec @muilang "
"@mydocumentsdir @numparams @osarch @osbuild @oslang @osservicepack @ostype @osversion @programfilesdir "
"@programscommondir @programsdir @scriptdir @scriptfullpath @scriptlinenumber @scriptname @sec "
"@startmenucommondir @startmenudir @startupcommondir @startupdir @sw_disable @sw_enable @sw_hide @sw_lock "
"@sw_maximize @sw_minimize @sw_restore @sw_show @sw_showdefault @sw_showmaximized @sw_showminimized "
"@sw_showminnoactive @sw_showna @sw_shownoactivate @sw_shownormal @sw_unlock @systemdir @tab @tempdir "
"@tray_id @trayiconflashing @trayiconvisible @username @userprofiledir @wday @windowsdir "
"@workingdir @yday @year",
"{!} {#} {^} {{} {}} {+} {alt} {altdown} {altup} {appskey} "
"{asc} {backspace} {break} {browser_back} {browser_favorites} {browser_forward} {browser_home} "
"{browser_refresh} {browser_search} {browser_stop} {bs} {capslock} {ctrldown} {ctrlup} "
"{del} {delete} {down} {end} {enter} {esc} {escape} {f1} {f10} {f11} {f12} {f2} {f3} "
"{f4} {f5} {f6} {f7} {f8} {f9} {home} {ins} {insert} {lalt} {launch_app1} {launch_app2} "
"{launch_mail} {launch_media} {lctrl} {left} {lshift} {lwin} {lwindown} {lwinup} {media_next} "
"{media_play_pause} {media_prev} {media_stop} {numlock} {numpad0} {numpad1} {numpad2} "
"{numpad3} {numpad4} {numpad5} {numpad6} {numpad7} {numpad8} {numpad9} {numpadadd} "
"{numpaddiv} {numpaddot} {numpadenter} {numpadmult} {numpadsub} {pause} {pgdn} {pgup} "
"{printscreen} {ralt} {rctrl} {right} {rshift} {rwin} {rwindown} {rwinup} {scrolllock} "
"{shiftdown} {shiftup} {sleep} {space} {tab} {up} {volume_down} {volume_mute} {volume_up}",
"#ce #comments-end #comments-start #cs #include #include-once #noautoit3execute #notrayicon "
"#onautoitstartregister #requireadmin",
"#autoit3wrapper_au3check_parameters #autoit3wrapper_au3check_stop_onwarning "
"#autoit3wrapper_change2cui #autoit3wrapper_compression #autoit3wrapper_cvswrapper_parameters "
"#autoit3wrapper_icon #autoit3wrapper_outfile #autoit3wrapper_outfile_type #autoit3wrapper_plugin_funcs "
"#autoit3wrapper_res_comment #autoit3wrapper_res_description #autoit3wrapper_res_field "
"#autoit3wrapper_res_file_add #autoit3wrapper_res_fileversion #autoit3wrapper_res_fileversion_autoincrement "
"#autoit3wrapper_res_icon_add #autoit3wrapper_res_language #autoit3wrapper_res_legalcopyright "
"#autoit3wrapper_res_requestedexecutionlevel #autoit3wrapper_res_savesource #autoit3wrapper_run_after "
"#autoit3wrapper_run_au3check #autoit3wrapper_run_before #autoit3wrapper_run_cvswrapper "
"#autoit3wrapper_run_debug_mode #autoit3wrapper_run_obfuscator #autoit3wrapper_run_tidy "
"#autoit3wrapper_tidy_stop_onerror #autoit3wrapper_useansi #autoit3wrapper_useupx "
"#autoit3wrapper_usex64 #autoit3wrapper_version #endregion #forceref #obfuscator_ignore_funcs "
"#obfuscator_ignore_variables #obfuscator_parameters #region #tidy_parameters",
"", // Reserved for expand
"_arrayadd _arraybinarysearch _arraycombinations _arrayconcatenate _arraydelete _arraydisplay _arrayfindall "
"_arrayinsert _arraymax _arraymaxindex _arraymin _arrayminindex _arraypermute _arraypop _arraypush "
"_arrayreverse _arraysearch _arraysort _arrayswap _arraytoclip _arraytostring _arraytrim _arrayunique _assert "
"_choosecolor _choosefont _clipboard_changechain _clipboard_close _clipboard _countformats _clipboard_empty "
"_clipboard_enumformats _clipboard_formatstr _clipboard_getdata _clipboard_getdataex _clipboard_getformatname "
"_clipboard_getopenwindow _clipboard_getowner _clipboard_getpriorityformat _clipboard_getsequencenumber "
"_clipboard_getviewer _clipboard_isformatavailable _clipboard_open _clipboard_registerformat "
"_clipboard_setdata _clipboard_setdataex _clipboard_setviewer _clipputfile _colorconverthsltorgb "
"_colorconvertrgbtohsl _colorgetblue _colorgetcolorref _colorgetgreen _colorgetred _colorgetrgb "
"_colorsetcolorref _colorsetrgb _crypt_decryptdata _crypt_decryptfile _crypt_derivekey _crypt_destroykey "
"_crypt_encryptdata _crypt_encryptfile _crypt_hashdata _crypt_hashfile _crypt_shutdown _crypt_startup "
"_date_time_comparefiletime _date_time_dosdatetimetoarray _date_time_dosdatetimetofiletime "
"_date_time_dosdatetimetostr _date_time_dosdatetoarray _date_time_dosdatetostr _date_time_dostimetoarray "
"_date_time_dostimetostr _date_time_encodefiletime _date_time_encodesystemtime _date_time_filetimetoarray "
"_date_time_filetimetodosdatetime _date_time_filetimetolocalfiletime _date_time_filetimetostr "
"_date_time_filetimetosystemtime _date_time_getfiletime _date_time_getlocaltime _date_time_getsystemtime "
"_date_time_getsystemtimeadjustment _date_time_getsystemtimeasfiletime _date_time_getsystemtimes "
"_date_time_gettickcount _date_time_gettimezoneinformation _date_time_localfiletimetofiletime "
"_date_time_setfiletime _date_time_setlocaltime _date_time_setsystemtime _date_time_setsystemtimeadjustment "
"_date_time_settimezoneinformation _date_time_systemtimetoarray _date_time_systemtimetodatestr "
"_date_time_systemtimetodatetimestr _date_time_systemtimetofiletime _date_time_systemtimetotimestr "
"_date_time_systemtimetotzspecificlocaltime _date_time_tzspecificlocaltimetosystemtime _dateadd "
"_datedayofweek _datedaysinmonth _datediff _dateisleapyear _dateisvalid _datetimeformat _datetimesplit "
"_datetodayofweek _datetodayofweekiso _datetodayvalue _datetomonth _dayvaluetodate _debugbugreportenv "
"_debugout _debugreport _debugreportex _debugreportvar _debugsetup _degree _eventlog__backup _eventlog__clear "
"_eventlog__close _eventlog__count _eventlog__deregistersource _eventlog__full _eventlog__notify "
"_eventlog__oldest _eventlog__open _eventlog__openbackup _eventlog__read _eventlog__registersource "
"_eventlog__report _excelbookattach _excelbookclose _excelbooknew _excelbookopen _excelbooksave "
"_excelbooksaveas _excelcolumndelete _excelcolumninsert _excelfontsetproperties _excelhorizontalalignset "
"_excelhyperlinkinsert _excelnumberformat _excelreadarray _excelreadcell _excelreadsheettoarray "
"_excelrowdelete _excelrowinsert _excelsheetactivate _excelsheetaddnew _excelsheetdelete _excelsheetlist "
"_excelsheetmove _excelsheetnameget _excelsheetnameset _excelwritearray _excelwritecell _excelwriteformula "
"_excelwritesheetfromarray _filecountlines _filecreate _filelisttoarray _fileprint _filereadtoarray "
"_filewritefromarray _filewritelog _filewritetoline _ftp_close _ftp_command _ftp_connect "
"_ftp_decodeinternetstatus _ftp_dircreate _ftp_dirdelete _ftp_dirgetcurrent _ftp_dirputcontents "
"_ftp_dirsetcurrent _ftp_fileclose _ftp_filedelete _ftp_fileget _ftp_filegetsize _ftp_fileopen _ftp_fileput "
"_ftp_fileread _ftp_filerename _ftp_filetimelohitostr _ftp_findfileclose _ftp_findfilefirst _ftp_findfilenext "
"_ftp_getlastresponseinfo _ftp_listtoarray _ftp_listtoarray2d _ftp_listtoarrayex _ftp_open "
"_ftp_progressdownload _ftp_progressupload _ftp_setstatuscallback _gdiplus_arrowcapcreate "
"_gdiplus_arrowcapdispose _gdiplus_arrowcapgetfillstate _gdiplus_arrowcapgetheight "
"_gdiplus_arrowcapgetmiddleinset _gdiplus_arrowcapgetwidth _gdiplus_arrowcapsetfillstate "
"_gdiplus_arrowcapsetheight _gdiplus_arrowcapsetmiddleinset _gdiplus_arrowcapsetwidth "
"_gdiplus_bitmapclonearea _gdiplus_bitmapcreatefromfile _gdiplus_bitmapcreatefromgraphics "
"_gdiplus_bitmapcreatefromhbitmap _gdiplus_bitmapcreatehbitmapfrombitmap _gdiplus_bitmapdispose "
"_gdiplus_bitmaplockbits _gdiplus_bitmapunlockbits _gdiplus_brushclone _gdiplus_brushcreatesolid "
"_gdiplus_brushdispose _gdiplus_brushgetsolidcolor _gdiplus_brushgettype _gdiplus_brushsetsolidcolor "
"_gdiplus_customlinecapdispose _gdiplus_decoders _gdiplus_decodersgetcount _gdiplus_decodersgetsize "
"_gdiplus_drawimagepoints _gdiplus_encoders _gdiplus_encodersgetclsid _gdiplus_encodersgetcount "
"_gdiplus_encodersgetparamlist _gdiplus_encodersgetparamlistsize _gdiplus_encodersgetsize _gdiplus_fontcreate "
"_gdiplus_fontdispose _gdiplus_fontfamilycreate _gdiplus_fontfamilydispose _gdiplus_graphicsclear "
"_gdiplus_graphicscreatefromhdc _gdiplus_graphicscreatefromhwnd _gdiplus_graphicsdispose "
"_gdiplus_graphicsdrawarc _gdiplus_graphicsdrawbezier _gdiplus_graphicsdrawclosedcurve "
"_gdiplus_graphicsdrawcurve _gdiplus_graphicsdrawellipse _gdiplus_graphicsdrawimage "
"_gdiplus_graphicsdrawimagerect _gdiplus_graphicsdrawimagerectrect _gdiplus_graphicsdrawline "
"_gdiplus_graphicsdrawpie _gdiplus_graphicsdrawpolygon _gdiplus_graphicsdrawrect _gdiplus_graphicsdrawstring "
"_gdiplus_graphicsdrawstringex _gdiplus_graphicsfillclosedcurve _gdiplus_graphicsfillellipse "
"_gdiplus_graphicsfillpie _gdiplus_graphicsfillpolygon _gdiplus_graphicsfillrect _gdiplus_graphicsgetdc "
"_gdiplus_graphicsgetsmoothingmode _gdiplus_graphicsmeasurestring _gdiplus_graphicsreleasedc "
"_gdiplus_graphicssetsmoothingmode _gdiplus_graphicssettransform _gdiplus_imagedispose _gdiplus_imagegetflags "
"_gdiplus_imagegetgraphicscontext _gdiplus_imagegetheight _gdiplus_imagegethorizontalresolution "
"_gdiplus_imagegetpixelformat _gdiplus_imagegetrawformat _gdiplus_imagegettype "
"_gdiplus_imagegetverticalresolution _gdiplus_imagegetwidth _gdiplus_imageloadfromfile "
"_gdiplus_imagesavetofile _gdiplus_imagesavetofileex _gdiplus_matrixcreate _gdiplus_matrixdispose "
"_gdiplus_matrixrotate _gdiplus_matrixscale _gdiplus_matrixtranslate _gdiplus_paramadd _gdiplus_paraminit "
"_gdiplus_pencreate _gdiplus_pendispose _gdiplus_pengetalignment _gdiplus_pengetcolor "
"_gdiplus_pengetcustomendcap _gdiplus_pengetdashcap _gdiplus_pengetdashstyle _gdiplus_pengetendcap "
"_gdiplus_pengetwidth _gdiplus_pensetalignment _gdiplus_pensetcolor _gdiplus_pensetcustomendcap "
"_gdiplus_pensetdashcap _gdiplus_pensetdashstyle _gdiplus_pensetendcap _gdiplus_pensetwidth "
"_gdiplus_rectfcreate _gdiplus_shutdown _gdiplus_startup _gdiplus_stringformatcreate "
"_gdiplus_stringformatdispose _gdiplus_stringformatsetalign _getip _guictrlavi_close _guictrlavi_create "
"_guictrlavi_destroy _guictrlavi_isplaying _guictrlavi_open _guictrlavi_openex _guictrlavi_play "
"_guictrlavi_seek _guictrlavi_show _guictrlavi_stop _guictrlbutton_click _guictrlbutton_create "
"_guictrlbutton_destroy _guictrlbutton_enable _guictrlbutton_getcheck _guictrlbutton_getfocus "
"_guictrlbutton_getidealsize _guictrlbutton_getimage _guictrlbutton_getimagelist _guictrlbutton_getnote "
"_guictrlbutton_getnotelength _guictrlbutton_getsplitinfo _guictrlbutton_getstate _guictrlbutton_gettext "
"_guictrlbutton_gettextmargin _guictrlbutton_setcheck _guictrlbutton_setdontclick _guictrlbutton_setfocus "
"_guictrlbutton_setimage _guictrlbutton_setimagelist _guictrlbutton_setnote _guictrlbutton_setshield "
"_guictrlbutton_setsize _guictrlbutton_setsplitinfo _guictrlbutton_setstate _guictrlbutton_setstyle "
"_guictrlbutton_settext _guictrlbutton_settextmargin _guictrlbutton_show _guictrlcombobox_adddir "
"_guictrlcombobox_addstring _guictrlcombobox_autocomplete _guictrlcombobox_beginupdate "
"_guictrlcombobox_create _guictrlcombobox_deletestring _guictrlcombobox_destroy _guictrlcombobox_endupdate "
"_guictrlcombobox_findstring _guictrlcombobox_findstringexact _guictrlcombobox_getcomboboxinfo "
"_guictrlcombobox_getcount _guictrlcombobox_getcuebanner _guictrlcombobox_getcursel "
"_guictrlcombobox_getdroppedcontrolrect _guictrlcombobox_getdroppedcontrolrectex "
"_guictrlcombobox_getdroppedstate _guictrlcombobox_getdroppedwidth _guictrlcombobox_geteditsel "
"_guictrlcombobox_getedittext _guictrlcombobox_getextendedui _guictrlcombobox_gethorizontalextent "
"_guictrlcombobox_getitemheight _guictrlcombobox_getlbtext _guictrlcombobox_getlbtextlen "
"_guictrlcombobox_getlist _guictrlcombobox_getlistarray _guictrlcombobox_getlocale "
"_guictrlcombobox_getlocalecountry _guictrlcombobox_getlocalelang _guictrlcombobox_getlocaleprimlang "
"_guictrlcombobox_getlocalesublang _guictrlcombobox_getminvisible _guictrlcombobox_gettopindex "
"_guictrlcombobox_initstorage _guictrlcombobox_insertstring _guictrlcombobox_limittext "
"_guictrlcombobox_replaceeditsel _guictrlcombobox_resetcontent _guictrlcombobox_selectstring "
"_guictrlcombobox_setcuebanner _guictrlcombobox_setcursel _guictrlcombobox_setdroppedwidth "
"_guictrlcombobox_seteditsel _guictrlcombobox_setedittext _guictrlcombobox_setextendedui "
"_guictrlcombobox_sethorizontalextent _guictrlcombobox_setitemheight _guictrlcombobox_setminvisible "
"_guictrlcombobox_settopindex _guictrlcombobox_showdropdown _guictrlcomboboxex_adddir "
"_guictrlcomboboxex_addstring _guictrlcomboboxex_beginupdate _guictrlcomboboxex_create "
"_guictrlcomboboxex_createsolidbitmap _guictrlcomboboxex_deletestring _guictrlcomboboxex_destroy "
"_guictrlcomboboxex_endupdate _guictrlcomboboxex_findstringexact _guictrlcomboboxex_getcomboboxinfo "
"_guictrlcomboboxex_getcombocontrol _guictrlcomboboxex_getcount _guictrlcomboboxex_getcursel "
"_guictrlcomboboxex_getdroppedcontrolrect _guictrlcomboboxex_getdroppedcontrolrectex "
"_guictrlcomboboxex_getdroppedstate _guictrlcomboboxex_getdroppedwidth _guictrlcomboboxex_geteditcontrol "
"_guictrlcomboboxex_geteditsel _guictrlcomboboxex_getedittext _guictrlcomboboxex_getextendedstyle "
"_guictrlcomboboxex_getextendedui _guictrlcomboboxex_getimagelist _guictrlcomboboxex_getitem "
"_guictrlcomboboxex_getitemex _guictrlcomboboxex_getitemheight _guictrlcomboboxex_getitemimage "
"_guictrlcomboboxex_getitemindent _guictrlcomboboxex_getitemoverlayimage _guictrlcomboboxex_getitemparam "
"_guictrlcomboboxex_getitemselectedimage _guictrlcomboboxex_getitemtext _guictrlcomboboxex_getitemtextlen "
"_guictrlcomboboxex_getlist _guictrlcomboboxex_getlistarray _guictrlcomboboxex_getlocale "
"_guictrlcomboboxex_getlocalecountry _guictrlcomboboxex_getlocalelang _guictrlcomboboxex_getlocaleprimlang "
"_guictrlcomboboxex_getlocalesublang _guictrlcomboboxex_getminvisible _guictrlcomboboxex_gettopindex "
"_guictrlcomboboxex_getunicode _guictrlcomboboxex_initstorage _guictrlcomboboxex_insertstring "
"_guictrlcomboboxex_limittext _guictrlcomboboxex_replaceeditsel _guictrlcomboboxex_resetcontent "
"_guictrlcomboboxex_setcursel _guictrlcomboboxex_setdroppedwidth _guictrlcomboboxex_seteditsel "
"_guictrlcomboboxex_setedittext _guictrlcomboboxex_setextendedstyle _guictrlcomboboxex_setextendedui "
"_guictrlcomboboxex_setimagelist _guictrlcomboboxex_setitem _guictrlcomboboxex_setitemex "
"_guictrlcomboboxex_setitemheight _guictrlcomboboxex_setitemimage _guictrlcomboboxex_setitemindent "
"_guictrlcomboboxex_setitemoverlayimage _guictrlcomboboxex_setitemparam "
"_guictrlcomboboxex_setitemselectedimage _guictrlcomboboxex_setminvisible _guictrlcomboboxex_settopindex "
"_guictrlcomboboxex_setunicode _guictrlcomboboxex_showdropdown _guictrldtp_create _guictrldtp_destroy "
"_guictrldtp_getmccolor _guictrldtp_getmcfont _guictrldtp_getmonthcal _guictrldtp_getrange "
"_guictrldtp_getrangeex _guictrldtp_getsystemtime _guictrldtp_getsystemtimeex _guictrldtp_setformat "
"_guictrldtp_setmccolor _guictrldtp_setmcfont _guictrldtp_setrange _guictrldtp_setrangeex "
"_guictrldtp_setsystemtime _guictrldtp_setsystemtimeex _guictrledit_appendtext _guictrledit_beginupdate "
"_guictrledit_canundo _guictrledit_charfrompos _guictrledit_create _guictrledit_destroy "
"_guictrledit_emptyundobuffer _guictrledit_endupdate _guictrledit_find _guictrledit_fmtlines "
"_guictrledit_getfirstvisibleline _guictrledit_getlimittext _guictrledit_getline _guictrledit_getlinecount "
"_guictrledit_getmargins _guictrledit_getmodify _guictrledit_getpasswordchar _guictrledit_getrect "
"_guictrledit_getrectex _guictrledit_getsel _guictrledit_gettext _guictrledit_gettextlen "
"_guictrledit_hideballoontip _guictrledit_inserttext _guictrledit_linefromchar _guictrledit_lineindex "
"_guictrledit_linelength _guictrledit_linescroll _guictrledit_posfromchar _guictrledit_replacesel "
"_guictrledit_scroll _guictrledit_setlimittext _guictrledit_setmargins _guictrledit_setmodify "
"_guictrledit_setpasswordchar _guictrledit_setreadonly _guictrledit_setrect _guictrledit_setrectex "
"_guictrledit_setrectnp _guictrledit_setrectnpex _guictrledit_setsel _guictrledit_settabstops "
"_guictrledit_settext _guictrledit_showballoontip _guictrledit_undo _guictrlheader_additem "
"_guictrlheader_clearfilter _guictrlheader_clearfilterall _guictrlheader_create "
"_guictrlheader_createdragimage _guictrlheader_deleteitem _guictrlheader_destroy _guictrlheader_editfilter "
"_guictrlheader_getbitmapmargin _guictrlheader_getimagelist _guictrlheader_getitem "
"_guictrlheader_getitemalign _guictrlheader_getitembitmap _guictrlheader_getitemcount "
"_guictrlheader_getitemdisplay _guictrlheader_getitemflags _guictrlheader_getitemformat "
"_guictrlheader_getitemimage _guictrlheader_getitemorder _guictrlheader_getitemparam "
"_guictrlheader_getitemrect _guictrlheader_getitemrectex _guictrlheader_getitemtext "
"_guictrlheader_getitemwidth _guictrlheader_getorderarray _guictrlheader_getunicodeformat "
"_guictrlheader_hittest _guictrlheader_insertitem _guictrlheader_layout _guictrlheader_ordertoindex "
"_guictrlheader_setbitmapmargin _guictrlheader_setfilterchangetimeout _guictrlheader_sethotdivider "
"_guictrlheader_setimagelist _guictrlheader_setitem _guictrlheader_setitemalign "
"_guictrlheader_setitembitmap _guictrlheader_setitemdisplay _guictrlheader_setitemflags "
"_guictrlheader_setitemformat _guictrlheader_setitemimage _guictrlheader_setitemorder "
"_guictrlheader_setitemparam _guictrlheader_setitemtext _guictrlheader_setitemwidth "
"_guictrlheader_setorderarray _guictrlheader_setunicodeformat _guictrlipaddress_clearaddress "
"_guictrlipaddress_create _guictrlipaddress_destroy _guictrlipaddress_get _guictrlipaddress_getarray "
"_guictrlipaddress_getex _guictrlipaddress_isblank _guictrlipaddress_set _guictrlipaddress_setarray "
"_guictrlipaddress_setex _guictrlipaddress_setfocus _guictrlipaddress_setfont _guictrlipaddress_setrange "
"_guictrlipaddress_showhide _guictrllistbox_addfile _guictrllistbox_addstring _guictrllistbox_beginupdate "
"_guictrllistbox_clickitem _guictrllistbox_create _guictrllistbox_deletestring _guictrllistbox_destroy "
"_guictrllistbox_dir _guictrllistbox_endupdate _guictrllistbox_findintext _guictrllistbox_findstring "
"_guictrllistbox_getanchorindex _guictrllistbox_getcaretindex _guictrllistbox_getcount "
"_guictrllistbox_getcursel _guictrllistbox_gethorizontalextent _guictrllistbox_getitemdata "
"_guictrllistbox_getitemheight _guictrllistbox_getitemrect _guictrllistbox_getitemrectex "
"_guictrllistbox_getlistboxinfo _guictrllistbox_getlocale _guictrllistbox_getlocalecountry "
"_guictrllistbox_getlocalelang _guictrllistbox_getlocaleprimlang _guictrllistbox_getlocalesublang "
"_guictrllistbox_getsel _guictrllistbox_getselcount _guictrllistbox_getselitems "
"_guictrllistbox_getselitemstext _guictrllistbox_gettext _guictrllistbox_gettextlen "
"_guictrllistbox_gettopindex _guictrllistbox_initstorage _guictrllistbox_insertstring "
"_guictrllistbox_itemfrompoint _guictrllistbox_replacestring _guictrllistbox_resetcontent "
"_guictrllistbox_selectstring _guictrllistbox_selitemrange _guictrllistbox_selitemrangeex "
"_guictrllistbox_setanchorindex _guictrllistbox_setcaretindex _guictrllistbox_setcolumnwidth "
"_guictrllistbox_setcursel _guictrllistbox_sethorizontalextent _guictrllistbox_setitemdata "
"_guictrllistbox_setitemheight _guictrllistbox_setlocale _guictrllistbox_setsel _guictrllistbox_settabstops "
"_guictrllistbox_settopindex _guictrllistbox_sort _guictrllistbox_swapstring _guictrllistbox_updatehscroll "
"_guictrllistview_addarray _guictrllistview_addcolumn _guictrllistview_additem _guictrllistview_addsubitem "
"_guictrllistview_approximateviewheight _guictrllistview_approximateviewrect "
"_guictrllistview_approximateviewwidth _guictrllistview_arrange _guictrllistview_beginupdate "
"_guictrllistview_canceleditlabel _guictrllistview_clickitem _guictrllistview_copyitems "
"_guictrllistview_create _guictrllistview_createdragimage _guictrllistview_createsolidbitmap "
"_guictrllistview_deleteallitems _guictrllistview_deletecolumn _guictrllistview_deleteitem "
"_guictrllistview_deleteitemsselected _guictrllistview_destroy _guictrllistview_drawdragimage "
"_guictrllistview_editlabel _guictrllistview_enablegroupview _guictrllistview_endupdate "
"_guictrllistview_ensurevisible _guictrllistview_findintext _guictrllistview_finditem "
"_guictrllistview_findnearest _guictrllistview_findparam _guictrllistview_findtext "
"_guictrllistview_getbkcolor _guictrllistview_getbkimage _guictrllistview_getcallbackmask "
"_guictrllistview_getcolumn _guictrllistview_getcolumncount _guictrllistview_getcolumnorder "
"_guictrllistview_getcolumnorderarray _guictrllistview_getcolumnwidth _guictrllistview_getcounterpage "
"_guictrllistview_geteditcontrol _guictrllistview_getextendedlistviewstyle _guictrllistview_getfocusedgroup "
"_guictrllistview_getgroupcount _guictrllistview_getgroupinfo _guictrllistview_getgroupinfobyindex "
"_guictrllistview_getgrouprect _guictrllistview_getgroupviewenabled _guictrllistview_getheader "
"_guictrllistview_gethotcursor _guictrllistview_gethotitem _guictrllistview_gethovertime "
"_guictrllistview_getimagelist _guictrllistview_getisearchstring _guictrllistview_getitem "
"_guictrllistview_getitemchecked _guictrllistview_getitemcount _guictrllistview_getitemcut "
"_guictrllistview_getitemdrophilited _guictrllistview_getitemex _guictrllistview_getitemfocused "
"_guictrllistview_getitemgroupid _guictrllistview_getitemimage _guictrllistview_getitemindent "
"_guictrllistview_getitemparam _guictrllistview_getitemposition _guictrllistview_getitempositionx "
"_guictrllistview_getitempositiony _guictrllistview_getitemrect _guictrllistview_getitemrectex "
"_guictrllistview_getitemselected _guictrllistview_getitemspacing _guictrllistview_getitemspacingx "
"_guictrllistview_getitemspacingy _guictrllistview_getitemstate _guictrllistview_getitemstateimage "
"_guictrllistview_getitemtext _guictrllistview_getitemtextarray _guictrllistview_getitemtextstring "
"_guictrllistview_getnextitem _guictrllistview_getnumberofworkareas _guictrllistview_getorigin "
"_guictrllistview_getoriginx _guictrllistview_getoriginy _guictrllistview_getoutlinecolor "
"_guictrllistview_getselectedcolumn _guictrllistview_getselectedcount _guictrllistview_getselectedindices "
"_guictrllistview_getselectionmark _guictrllistview_getstringwidth _guictrllistview_getsubitemrect "
"_guictrllistview_gettextbkcolor _guictrllistview_gettextcolor _guictrllistview_gettooltips "
"_guictrllistview_gettopindex _guictrllistview_getunicodeformat _guictrllistview_getview "
"_guictrllistview_getviewdetails _guictrllistview_getviewlarge _guictrllistview_getviewlist "
"_guictrllistview_getviewrect _guictrllistview_getviewsmall _guictrllistview_getviewtile "
"_guictrllistview_hidecolumn _guictrllistview_hittest _guictrllistview_insertcolumn "
"_guictrllistview_insertgroup _guictrllistview_insertitem _guictrllistview_justifycolumn "
"_guictrllistview_mapidtoindex _guictrllistview_mapindextoid _guictrllistview_redrawitems "
"_guictrllistview_registersortcallback _guictrllistview_removeallgroups _guictrllistview_removegroup "
"_guictrllistview_scroll _guictrllistview_setbkcolor _guictrllistview_setbkimage "
"_guictrllistview_setcallbackmask _guictrllistview_setcolumn _guictrllistview_setcolumnorder "
"_guictrllistview_setcolumnorderarray _guictrllistview_setcolumnwidth "
"_guictrllistview_setextendedlistviewstyle _guictrllistview_setgroupinfo _guictrllistview_sethotitem "
"_guictrllistview_sethovertime _guictrllistview_seticonspacing _guictrllistview_setimagelist "
"_guictrllistview_setitem _guictrllistview_setitemchecked _guictrllistview_setitemcount "
"_guictrllistview_setitemcut _guictrllistview_setitemdrophilited _guictrllistview_setitemex "
"_guictrllistview_setitemfocused _guictrllistview_setitemgroupid _guictrllistview_setitemimage "
"_guictrllistview_setitemindent _guictrllistview_setitemparam _guictrllistview_setitemposition "
"_guictrllistview_setitemposition32 _guictrllistview_setitemselected _guictrllistview_setitemstate "
"_guictrllistview_setitemstateimage _guictrllistview_setitemtext _guictrllistview_setoutlinecolor "
"_guictrllistview_setselectedcolumn _guictrllistview_setselectionmark _guictrllistview_settextbkcolor "
"_guictrllistview_settextcolor _guictrllistview_settooltips _guictrllistview_setunicodeformat "
"_guictrllistview_setview _guictrllistview_setworkareas _guictrllistview_simplesort "
"_guictrllistview_sortitems _guictrllistview_subitemhittest _guictrllistview_unregistersortcallback "
"_guictrlmenu_addmenuitem _guictrlmenu_appendmenu _guictrlmenu_checkmenuitem _guictrlmenu_checkradioitem "
"_guictrlmenu_createmenu _guictrlmenu_createpopup _guictrlmenu_deletemenu _guictrlmenu_destroymenu "
"_guictrlmenu_drawmenubar _guictrlmenu_enablemenuitem _guictrlmenu_finditem _guictrlmenu_findparent "
"_guictrlmenu_getitembmp _guictrlmenu_getitembmpchecked _guictrlmenu_getitembmpunchecked "
"_guictrlmenu_getitemchecked _guictrlmenu_getitemcount _guictrlmenu_getitemdata _guictrlmenu_getitemdefault "
"_guictrlmenu_getitemdisabled _guictrlmenu_getitemenabled _guictrlmenu_getitemgrayed "
"_guictrlmenu_getitemhighlighted _guictrlmenu_getitemid _guictrlmenu_getiteminfo _guictrlmenu_getitemrect "
"_guictrlmenu_getitemrectex _guictrlmenu_getitemstate _guictrlmenu_getitemstateex "
"_guictrlmenu_getitemsubmenu _guictrlmenu_getitemtext _guictrlmenu_getitemtype _guictrlmenu_getmenu "
"_guictrlmenu_getmenubackground _guictrlmenu_getmenubarinfo _guictrlmenu_getmenucontexthelpid "
"_guictrlmenu_getmenudata _guictrlmenu_getmenudefaultitem _guictrlmenu_getmenuheight "
"_guictrlmenu_getmenuinfo _guictrlmenu_getmenustyle _guictrlmenu_getsystemmenu _guictrlmenu_insertmenuitem "
"_guictrlmenu_insertmenuitemex _guictrlmenu_ismenu _guictrlmenu_loadmenu _guictrlmenu_mapaccelerator "
"_guictrlmenu_menuitemfrompoint _guictrlmenu_removemenu _guictrlmenu_setitembitmaps _guictrlmenu_setitembmp "
"_guictrlmenu_setitembmpchecked _guictrlmenu_setitembmpunchecked _guictrlmenu_setitemchecked "
"_guictrlmenu_setitemdata _guictrlmenu_setitemdefault _guictrlmenu_setitemdisabled "
"_guictrlmenu_setitemenabled _guictrlmenu_setitemgrayed _guictrlmenu_setitemhighlighted "
"_guictrlmenu_setitemid _guictrlmenu_setiteminfo _guictrlmenu_setitemstate _guictrlmenu_setitemsubmenu "
"_guictrlmenu_setitemtext _guictrlmenu_setitemtype _guictrlmenu_setmenu _guictrlmenu_setmenubackground "
"_guictrlmenu_setmenucontexthelpid _guictrlmenu_setmenudata _guictrlmenu_setmenudefaultitem "
"_guictrlmenu_setmenuheight _guictrlmenu_setmenuinfo _guictrlmenu_setmenustyle _guictrlmenu_trackpopupmenu "
"_guictrlmonthcal_create _guictrlmonthcal_destroy _guictrlmonthcal_getcalendarborder "
"_guictrlmonthcal_getcalendarcount _guictrlmonthcal_getcolor _guictrlmonthcal_getcolorarray "
"_guictrlmonthcal_getcursel _guictrlmonthcal_getcurselstr _guictrlmonthcal_getfirstdow "
"_guictrlmonthcal_getfirstdowstr _guictrlmonthcal_getmaxselcount _guictrlmonthcal_getmaxtodaywidth "
"_guictrlmonthcal_getminreqheight _guictrlmonthcal_getminreqrect _guictrlmonthcal_getminreqrectarray "
"_guictrlmonthcal_getminreqwidth _guictrlmonthcal_getmonthdelta _guictrlmonthcal_getmonthrange "
"_guictrlmonthcal_getmonthrangemax _guictrlmonthcal_getmonthrangemaxstr _guictrlmonthcal_getmonthrangemin "
"_guictrlmonthcal_getmonthrangeminstr _guictrlmonthcal_getmonthrangespan _guictrlmonthcal_getrange "
"_guictrlmonthcal_getrangemax _guictrlmonthcal_getrangemaxstr _guictrlmonthcal_getrangemin "
"_guictrlmonthcal_getrangeminstr _guictrlmonthcal_getselrange _guictrlmonthcal_getselrangemax "
"_guictrlmonthcal_getselrangemaxstr _guictrlmonthcal_getselrangemin _guictrlmonthcal_getselrangeminstr "
"_guictrlmonthcal_gettoday _guictrlmonthcal_gettodaystr _guictrlmonthcal_getunicodeformat "
"_guictrlmonthcal_hittest _guictrlmonthcal_setcalendarborder _guictrlmonthcal_setcolor "
"_guictrlmonthcal_setcursel _guictrlmonthcal_setdaystate _guictrlmonthcal_setfirstdow "
"_guictrlmonthcal_setmaxselcount _guictrlmonthcal_setmonthdelta _guictrlmonthcal_setrange "
"_guictrlmonthcal_setselrange _guictrlmonthcal_settoday _guictrlmonthcal_setunicodeformat "
"_guictrlrebar_addband _guictrlrebar_addtoolbarband _guictrlrebar_begindrag _guictrlrebar_create "
"_guictrlrebar_deleteband _guictrlrebar_destroy _guictrlrebar_dragmove _guictrlrebar_enddrag "
"_guictrlrebar_getbandbackcolor _guictrlrebar_getbandborders _guictrlrebar_getbandbordersex "
"_guictrlrebar_getbandchildhandle _guictrlrebar_getbandchildsize _guictrlrebar_getbandcount "
"_guictrlrebar_getbandforecolor _guictrlrebar_getbandheadersize _guictrlrebar_getbandid "
"_guictrlrebar_getbandidealsize _guictrlrebar_getbandlength _guictrlrebar_getbandlparam "
"_guictrlrebar_getbandmargins _guictrlrebar_getbandmarginsex _guictrlrebar_getbandrect "
"_guictrlrebar_getbandrectex _guictrlrebar_getbandstyle _guictrlrebar_getbandstylebreak "
"_guictrlrebar_getbandstylechildedge _guictrlrebar_getbandstylefixedbmp _guictrlrebar_getbandstylefixedsize "
"_guictrlrebar_getbandstylegripperalways _guictrlrebar_getbandstylehidden "
"_guictrlrebar_getbandstylehidetitle _guictrlrebar_getbandstylenogripper _guictrlrebar_getbandstyletopalign "
"_guictrlrebar_getbandstyleusechevron _guictrlrebar_getbandstylevariableheight _guictrlrebar_getbandtext "
"_guictrlrebar_getbarheight _guictrlrebar_getbarinfo _guictrlrebar_getbkcolor _guictrlrebar_getcolorscheme "
"_guictrlrebar_getrowcount _guictrlrebar_getrowheight _guictrlrebar_gettextcolor _guictrlrebar_gettooltips "
"_guictrlrebar_getunicodeformat _guictrlrebar_hittest _guictrlrebar_idtoindex _guictrlrebar_maximizeband "
"_guictrlrebar_minimizeband _guictrlrebar_moveband _guictrlrebar_setbandbackcolor "
"_guictrlrebar_setbandforecolor _guictrlrebar_setbandheadersize _guictrlrebar_setbandid "
"_guictrlrebar_setbandidealsize _guictrlrebar_setbandlength _guictrlrebar_setbandlparam "
"_guictrlrebar_setbandstyle _guictrlrebar_setbandstylebreak _guictrlrebar_setbandstylechildedge "
"_guictrlrebar_setbandstylefixedbmp _guictrlrebar_setbandstylefixedsize "
"_guictrlrebar_setbandstylegripperalways _guictrlrebar_setbandstylehidden "
"_guictrlrebar_setbandstylehidetitle _guictrlrebar_setbandstylenogripper _guictrlrebar_setbandstyletopalign "
"_guictrlrebar_setbandstyleusechevron _guictrlrebar_setbandstylevariableheight _guictrlrebar_setbandtext "
"_guictrlrebar_setbarinfo _guictrlrebar_setbkcolor _guictrlrebar_setcolorscheme _guictrlrebar_settextcolor "
"_guictrlrebar_settooltips _guictrlrebar_setunicodeformat _guictrlrebar_showband "
"_guictrlrichedit_appendtext _guictrlrichedit_autodetecturl _guictrlrichedit_canpaste "
"_guictrlrichedit_canpastespecial _guictrlrichedit_canredo _guictrlrichedit_canundo "
"_guictrlrichedit_changefontsize _guictrlrichedit_copy _guictrlrichedit_create _guictrlrichedit_cut "
"_guictrlrichedit_deselect _guictrlrichedit_destroy _guictrlrichedit_emptyundobuffer "
"_guictrlrichedit_findtext _guictrlrichedit_findtextinrange _guictrlrichedit_getbkcolor "
"_guictrlrichedit_getcharattributes _guictrlrichedit_getcharbkcolor _guictrlrichedit_getcharcolor "
"_guictrlrichedit_getcharposfromxy _guictrlrichedit_getcharposofnextword "
"_guictrlrichedit_getcharposofpreviousword _guictrlrichedit_getcharwordbreakinfo "
"_guictrlrichedit_getfirstcharposonline _guictrlrichedit_getfont _guictrlrichedit_getlinecount "
"_guictrlrichedit_getlinelength _guictrlrichedit_getlinenumberfromcharpos _guictrlrichedit_getnextredo "
"_guictrlrichedit_getnextundo _guictrlrichedit_getnumberoffirstvisibleline "
"_guictrlrichedit_getparaalignment _guictrlrichedit_getparaattributes _guictrlrichedit_getparaborder "
"_guictrlrichedit_getparaindents _guictrlrichedit_getparanumbering _guictrlrichedit_getparashading "
"_guictrlrichedit_getparaspacing _guictrlrichedit_getparatabstops _guictrlrichedit_getpasswordchar "
"_guictrlrichedit_getrect _guictrlrichedit_getscrollpos _guictrlrichedit_getsel _guictrlrichedit_getselaa "
"_guictrlrichedit_getseltext _guictrlrichedit_getspaceunit _guictrlrichedit_gettext "
"_guictrlrichedit_gettextinline _guictrlrichedit_gettextinrange _guictrlrichedit_gettextlength "
"_guictrlrichedit_getversion _guictrlrichedit_getxyfromcharpos _guictrlrichedit_getzoom "
"_guictrlrichedit_gotocharpos _guictrlrichedit_hideselection _guictrlrichedit_inserttext "
"_guictrlrichedit_ismodified _guictrlrichedit_istextselected _guictrlrichedit_paste "
"_guictrlrichedit_pastespecial _guictrlrichedit_pauseredraw _guictrlrichedit_redo "
"_guictrlrichedit_replacetext _guictrlrichedit_resumeredraw _guictrlrichedit_scrolllineorpage "
"_guictrlrichedit_scrolllines _guictrlrichedit_scrolltocaret _guictrlrichedit_setbkcolor "
"_guictrlrichedit_setcharattributes _guictrlrichedit_setcharbkcolor _guictrlrichedit_setcharcolor "
"_guictrlrichedit_seteventmask _guictrlrichedit_setfont _guictrlrichedit_setlimitontext "
"_guictrlrichedit_setmodified _guictrlrichedit_setparaalignment _guictrlrichedit_setparaattributes "
"_guictrlrichedit_setparaborder _guictrlrichedit_setparaindents _guictrlrichedit_setparanumbering "
"_guictrlrichedit_setparashading _guictrlrichedit_setparaspacing _guictrlrichedit_setparatabstops "
"_guictrlrichedit_setpasswordchar _guictrlrichedit_setreadonly _guictrlrichedit_setrect "
"_guictrlrichedit_setscrollpos _guictrlrichedit_setsel _guictrlrichedit_setspaceunit "
"_guictrlrichedit_settabstops _guictrlrichedit_settext _guictrlrichedit_setundolimit "
"_guictrlrichedit_setzoom _guictrlrichedit_streamfromfile _guictrlrichedit_streamfromvar "
"_guictrlrichedit_streamtofile _guictrlrichedit_streamtovar _guictrlrichedit_undo _guictrlslider_clearsel "
"_guictrlslider_cleartics _guictrlslider_create _guictrlslider_destroy _guictrlslider_getbuddy "
"_guictrlslider_getchannelrect _guictrlslider_getchannelrectex _guictrlslider_getlinesize "
"_guictrlslider_getlogicaltics _guictrlslider_getnumtics _guictrlslider_getpagesize _guictrlslider_getpos "
"_guictrlslider_getrange _guictrlslider_getrangemax _guictrlslider_getrangemin _guictrlslider_getsel "
"_guictrlslider_getselend _guictrlslider_getselstart _guictrlslider_getthumblength "
"_guictrlslider_getthumbrect _guictrlslider_getthumbrectex _guictrlslider_gettic _guictrlslider_getticpos "
"_guictrlslider_gettooltips _guictrlslider_getunicodeformat _guictrlslider_setbuddy "
"_guictrlslider_setlinesize _guictrlslider_setpagesize _guictrlslider_setpos _guictrlslider_setrange "
"_guictrlslider_setrangemax _guictrlslider_setrangemin _guictrlslider_setsel _guictrlslider_setselend "
"_guictrlslider_setselstart _guictrlslider_setthumblength _guictrlslider_settic _guictrlslider_setticfreq "
"_guictrlslider_settipside _guictrlslider_settooltips _guictrlslider_setunicodeformat "
"_guictrlstatusbar_create _guictrlstatusbar_destroy _guictrlstatusbar_embedcontrol "
"_guictrlstatusbar_getborders _guictrlstatusbar_getbordershorz _guictrlstatusbar_getbordersrect "
"_guictrlstatusbar_getbordersvert _guictrlstatusbar_getcount _guictrlstatusbar_getheight "
"_guictrlstatusbar_geticon _guictrlstatusbar_getparts _guictrlstatusbar_getrect _guictrlstatusbar_getrectex "
"_guictrlstatusbar_gettext _guictrlstatusbar_gettextflags _guictrlstatusbar_gettextlength "
"_guictrlstatusbar_gettextlengthex _guictrlstatusbar_gettiptext _guictrlstatusbar_getunicodeformat "
"_guictrlstatusbar_getwidth _guictrlstatusbar_issimple _guictrlstatusbar_resize "
"_guictrlstatusbar_setbkcolor _guictrlstatusbar_seticon _guictrlstatusbar_setminheight "
"_guictrlstatusbar_setparts _guictrlstatusbar_setsimple _guictrlstatusbar_settext "
"_guictrlstatusbar_settiptext _guictrlstatusbar_setunicodeformat _guictrlstatusbar_showhide "
"_guictrltab_activatetab _guictrltab_clicktab _guictrltab_create _guictrltab_deleteallitems "
"_guictrltab_deleteitem _guictrltab_deselectall _guictrltab_destroy _guictrltab_findtab "
"_guictrltab_getcurfocus _guictrltab_getcursel _guictrltab_getdisplayrect _guictrltab_getdisplayrectex "
"_guictrltab_getextendedstyle _guictrltab_getimagelist _guictrltab_getitem _guictrltab_getitemcount "
"_guictrltab_getitemimage _guictrltab_getitemparam _guictrltab_getitemrect _guictrltab_getitemrectex "
"_guictrltab_getitemstate _guictrltab_getitemtext _guictrltab_getrowcount _guictrltab_gettooltips "
"_guictrltab_getunicodeformat _guictrltab_highlightitem _guictrltab_hittest _guictrltab_insertitem "
"_guictrltab_removeimage _guictrltab_setcurfocus _guictrltab_setcursel _guictrltab_setextendedstyle "
"_guictrltab_setimagelist _guictrltab_setitem _guictrltab_setitemimage _guictrltab_setitemparam "
"_guictrltab_setitemsize _guictrltab_setitemstate _guictrltab_setitemtext _guictrltab_setmintabwidth "
"_guictrltab_setpadding _guictrltab_settooltips _guictrltab_setunicodeformat _guictrltoolbar_addbitmap "
"_guictrltoolbar_addbutton _guictrltoolbar_addbuttonsep _guictrltoolbar_addstring "
"_guictrltoolbar_buttoncount _guictrltoolbar_checkbutton _guictrltoolbar_clickaccel "
"_guictrltoolbar_clickbutton _guictrltoolbar_clickindex _guictrltoolbar_commandtoindex "
"_guictrltoolbar_create _guictrltoolbar_customize _guictrltoolbar_deletebutton _guictrltoolbar_destroy "
"_guictrltoolbar_enablebutton _guictrltoolbar_findtoolbar _guictrltoolbar_getanchorhighlight "
"_guictrltoolbar_getbitmapflags _guictrltoolbar_getbuttonbitmap _guictrltoolbar_getbuttoninfo "
"_guictrltoolbar_getbuttoninfoex _guictrltoolbar_getbuttonparam _guictrltoolbar_getbuttonrect "
"_guictrltoolbar_getbuttonrectex _guictrltoolbar_getbuttonsize _guictrltoolbar_getbuttonstate "
"_guictrltoolbar_getbuttonstyle _guictrltoolbar_getbuttontext _guictrltoolbar_getcolorscheme "
"_guictrltoolbar_getdisabledimagelist _guictrltoolbar_getextendedstyle _guictrltoolbar_gethotimagelist "
"_guictrltoolbar_gethotitem _guictrltoolbar_getimagelist _guictrltoolbar_getinsertmark "
"_guictrltoolbar_getinsertmarkcolor _guictrltoolbar_getmaxsize _guictrltoolbar_getmetrics "
"_guictrltoolbar_getpadding _guictrltoolbar_getrows _guictrltoolbar_getstring _guictrltoolbar_getstyle "
"_guictrltoolbar_getstylealtdrag _guictrltoolbar_getstylecustomerase _guictrltoolbar_getstyleflat "
"_guictrltoolbar_getstylelist _guictrltoolbar_getstyleregisterdrop _guictrltoolbar_getstyletooltips "
"_guictrltoolbar_getstyletransparent _guictrltoolbar_getstylewrapable _guictrltoolbar_gettextrows "
"_guictrltoolbar_gettooltips _guictrltoolbar_getunicodeformat _guictrltoolbar_hidebutton "
"_guictrltoolbar_highlightbutton _guictrltoolbar_hittest _guictrltoolbar_indextocommand "
"_guictrltoolbar_insertbutton _guictrltoolbar_insertmarkhittest _guictrltoolbar_isbuttonchecked "
"_guictrltoolbar_isbuttonenabled _guictrltoolbar_isbuttonhidden _guictrltoolbar_isbuttonhighlighted "
"_guictrltoolbar_isbuttonindeterminate _guictrltoolbar_isbuttonpressed _guictrltoolbar_loadbitmap "
"_guictrltoolbar_loadimages _guictrltoolbar_mapaccelerator _guictrltoolbar_movebutton "
"_guictrltoolbar_pressbutton _guictrltoolbar_setanchorhighlight _guictrltoolbar_setbitmapsize "
"_guictrltoolbar_setbuttonbitmap _guictrltoolbar_setbuttoninfo _guictrltoolbar_setbuttoninfoex "
"_guictrltoolbar_setbuttonparam _guictrltoolbar_setbuttonsize _guictrltoolbar_setbuttonstate "
"_guictrltoolbar_setbuttonstyle _guictrltoolbar_setbuttontext _guictrltoolbar_setbuttonwidth "
"_guictrltoolbar_setcmdid _guictrltoolbar_setcolorscheme _guictrltoolbar_setdisabledimagelist "
"_guictrltoolbar_setdrawtextflags _guictrltoolbar_setextendedstyle _guictrltoolbar_sethotimagelist "
"_guictrltoolbar_sethotitem _guictrltoolbar_setimagelist _guictrltoolbar_setindent "
"_guictrltoolbar_setindeterminate _guictrltoolbar_setinsertmark _guictrltoolbar_setinsertmarkcolor "
"_guictrltoolbar_setmaxtextrows _guictrltoolbar_setmetrics _guictrltoolbar_setpadding "
"_guictrltoolbar_setparent _guictrltoolbar_setrows _guictrltoolbar_setstyle _guictrltoolbar_setstylealtdrag "
"_guictrltoolbar_setstylecustomerase _guictrltoolbar_setstyleflat _guictrltoolbar_setstylelist "
"_guictrltoolbar_setstyleregisterdrop _guictrltoolbar_setstyletooltips _guictrltoolbar_setstyletransparent "
"_guictrltoolbar_setstylewrapable _guictrltoolbar_settooltips _guictrltoolbar_setunicodeformat "
"_guictrltoolbar_setwindowtheme _guictrltreeview_add _guictrltreeview_addchild "
"_guictrltreeview_addchildfirst _guictrltreeview_addfirst _guictrltreeview_beginupdate "
"_guictrltreeview_clickitem _guictrltreeview_create _guictrltreeview_createdragimage "
"_guictrltreeview_createsolidbitmap _guictrltreeview_delete _guictrltreeview_deleteall "
"_guictrltreeview_deletechildren _guictrltreeview_destroy _guictrltreeview_displayrect "
"_guictrltreeview_displayrectex _guictrltreeview_edittext _guictrltreeview_endedit "
"_guictrltreeview_endupdate _guictrltreeview_ensurevisible _guictrltreeview_expand "
"_guictrltreeview_expandedonce _guictrltreeview_finditem _guictrltreeview_finditemex "
"_guictrltreeview_getbkcolor _guictrltreeview_getbold _guictrltreeview_getchecked "
"_guictrltreeview_getchildcount _guictrltreeview_getchildren _guictrltreeview_getcount "
"_guictrltreeview_getcut _guictrltreeview_getdroptarget _guictrltreeview_geteditcontrol "
"_guictrltreeview_getexpanded _guictrltreeview_getfirstchild _guictrltreeview_getfirstitem "
"_guictrltreeview_getfirstvisible _guictrltreeview_getfocused _guictrltreeview_getheight "
"_guictrltreeview_getimageindex _guictrltreeview_getimagelisticonhandle _guictrltreeview_getindent "
"_guictrltreeview_getinsertmarkcolor _guictrltreeview_getisearchstring _guictrltreeview_getitembyindex "
"_guictrltreeview_getitemhandle _guictrltreeview_getitemparam _guictrltreeview_getlastchild "
"_guictrltreeview_getlinecolor _guictrltreeview_getnext _guictrltreeview_getnextchild "
"_guictrltreeview_getnextsibling _guictrltreeview_getnextvisible _guictrltreeview_getnormalimagelist "
"_guictrltreeview_getparenthandle _guictrltreeview_getparentparam _guictrltreeview_getprev "
"_guictrltreeview_getprevchild _guictrltreeview_getprevsibling _guictrltreeview_getprevvisible "
"_guictrltreeview_getscrolltime _guictrltreeview_getselected _guictrltreeview_getselectedimageindex "
"_guictrltreeview_getselection _guictrltreeview_getsiblingcount _guictrltreeview_getstate "
"_guictrltreeview_getstateimageindex _guictrltreeview_getstateimagelist _guictrltreeview_gettext "
"_guictrltreeview_gettextcolor _guictrltreeview_gettooltips _guictrltreeview_gettree "
"_guictrltreeview_getunicodeformat _guictrltreeview_getvisible _guictrltreeview_getvisiblecount "
"_guictrltreeview_hittest _guictrltreeview_hittestex _guictrltreeview_hittestitem _guictrltreeview_index "
"_guictrltreeview_insertitem _guictrltreeview_isfirstitem _guictrltreeview_isparent _guictrltreeview_level "
"_guictrltreeview_selectitem _guictrltreeview_selectitembyindex _guictrltreeview_setbkcolor "
"_guictrltreeview_setbold _guictrltreeview_setchecked _guictrltreeview_setcheckedbyindex "
"_guictrltreeview_setchildren _guictrltreeview_setcut _guictrltreeview_setdroptarget "
"_guictrltreeview_setfocused _guictrltreeview_setheight _guictrltreeview_seticon "
"_guictrltreeview_setimageindex _guictrltreeview_setindent _guictrltreeview_setinsertmark "
"_guictrltreeview_setinsertmarkcolor _guictrltreeview_setitemheight _guictrltreeview_setitemparam "
"_guictrltreeview_setlinecolor _guictrltreeview_setnormalimagelist _guictrltreeview_setscrolltime "
"_guictrltreeview_setselected _guictrltreeview_setselectedimageindex _guictrltreeview_setstate "
"_guictrltreeview_setstateimageindex _guictrltreeview_setstateimagelist _guictrltreeview_settext "
"_guictrltreeview_settextcolor _guictrltreeview_settooltips _guictrltreeview_setunicodeformat "
"_guictrltreeview_sort _guiimagelist_add _guiimagelist_addbitmap _guiimagelist_addicon "
"_guiimagelist_addmasked _guiimagelist_begindrag _guiimagelist_copy _guiimagelist_create "
"_guiimagelist_destroy _guiimagelist_destroyicon _guiimagelist_dragenter _guiimagelist_dragleave "
"_guiimagelist_dragmove _guiimagelist_draw _guiimagelist_drawex _guiimagelist_duplicate "
"_guiimagelist_enddrag _guiimagelist_getbkcolor _guiimagelist_geticon _guiimagelist_geticonheight "
"_guiimagelist_geticonsize _guiimagelist_geticonsizeex _guiimagelist_geticonwidth "
"_guiimagelist_getimagecount _guiimagelist_getimageinfoex _guiimagelist_remove _guiimagelist_replaceicon "
"_guiimagelist_setbkcolor _guiimagelist_seticonsize _guiimagelist_setimagecount _guiimagelist_swap "
"_guiscrollbars_enablescrollbar _guiscrollbars_getscrollbarinfoex _guiscrollbars_getscrollbarrect "
"_guiscrollbars_getscrollbarrgstate _guiscrollbars_getscrollbarxylinebutton "
"_guiscrollbars_getscrollbarxythumbbottom _guiscrollbars_getscrollbarxythumbtop "
"_guiscrollbars_getscrollinfo _guiscrollbars_getscrollinfoex _guiscrollbars_getscrollinfomax "
"_guiscrollbars_getscrollinfomin _guiscrollbars_getscrollinfopage _guiscrollbars_getscrollinfopos "
"_guiscrollbars_getscrollinfotrackpos _guiscrollbars_getscrollpos _guiscrollbars_getscrollrange "
"_guiscrollbars_init _guiscrollbars_scrollwindow _guiscrollbars_setscrollinfo "
"_guiscrollbars_setscrollinfomax _guiscrollbars_setscrollinfomin _guiscrollbars_setscrollinfopage "
"_guiscrollbars_setscrollinfopos _guiscrollbars_setscrollrange _guiscrollbars_showscrollbar "
"_guitooltip_activate _guitooltip_addtool _guitooltip_adjustrect _guitooltip_bitstottf _guitooltip_create "
"_guitooltip_deltool _guitooltip_destroy _guitooltip_enumtools _guitooltip_getbubbleheight "
"_guitooltip_getbubblesize _guitooltip_getbubblewidth _guitooltip_getcurrenttool _guitooltip_getdelaytime "
"_guitooltip_getmargin _guitooltip_getmarginex _guitooltip_getmaxtipwidth _guitooltip_gettext "
"_guitooltip_gettipbkcolor _guitooltip_gettiptextcolor _guitooltip_gettitlebitmap _guitooltip_gettitletext "
"_guitooltip_gettoolcount _guitooltip_gettoolinfo _guitooltip_hittest _guitooltip_newtoolrect "
"_guitooltip_pop _guitooltip_popup _guitooltip_setdelaytime _guitooltip_setmargin "
"_guitooltip_setmaxtipwidth _guitooltip_settipbkcolor _guitooltip_settiptextcolor _guitooltip_settitle "
"_guitooltip_settoolinfo _guitooltip_setwindowtheme _guitooltip_toolexists _guitooltip_tooltoarray "
"_guitooltip_trackactivate _guitooltip_trackposition _guitooltip_ttftobits _guitooltip_update "
"_guitooltip_updatetiptext _hextostring _ie_example _ie_introduction _ie_versioninfo _ieaction _ieattach "
"_iebodyreadhtml _iebodyreadtext _iebodywritehtml _iecreate _iecreateembedded _iedocgetobj _iedocinserthtml "
"_iedocinserttext _iedocreadhtml _iedocwritehtml _ieerrorhandlerderegister _ieerrorhandlerregister "
"_ieerrornotify _ieformelementcheckboxselect _ieformelementgetcollection _ieformelementgetobjbyname "
"_ieformelementgetvalue _ieformelementoptionselect _ieformelementradioselect _ieformelementsetvalue "
"_ieformgetcollection _ieformgetobjbyname _ieformimageclick _ieformreset _ieformsubmit "
"_ieframegetcollection _ieframegetobjbyname _iegetobjbyid _iegetobjbyname _ieheadinserteventscript "
"_ieimgclick _ieimggetcollection _ieisframeset _ielinkclickbyindex _ielinkclickbytext _ielinkgetcollection "
"_ieloadwait _ieloadwaittimeout _ienavigate _iepropertyget _iepropertyset _iequit _ietablegetcollection "
"_ietablewritetoarray _ietagnameallgetcollection _ietagnamegetcollection _iif _inetexplorercapable "
"_inetgetsource _inetmail _inetsmtpmail _ispressed _mathcheckdiv _max _memglobalalloc _memglobalfree "
"_memgloballock _memglobalsize _memglobalunlock _memmovememory _memvirtualalloc _memvirtualallocex "
"_memvirtualfree _memvirtualfreeex _min _mousetrap _namedpipes_callnamedpipe _namedpipes_connectnamedpipe "
"_namedpipes_createnamedpipe _namedpipes_createpipe _namedpipes_disconnectnamedpipe "
"_namedpipes_getnamedpipehandlestate _namedpipes_getnamedpipeinfo _namedpipes_peeknamedpipe "
"_namedpipes_setnamedpipehandlestate _namedpipes_transactnamedpipe _namedpipes_waitnamedpipe "
"_net_share_connectionenum _net_share_fileclose _net_share_fileenum _net_share_filegetinfo "
"_net_share_permstr _net_share_resourcestr _net_share_sessiondel _net_share_sessionenum "
"_net_share_sessiongetinfo _net_share_shareadd _net_share_sharecheck _net_share_sharedel "
"_net_share_shareenum _net_share_sharegetinfo _net_share_sharesetinfo _net_share_statisticsgetsvr "
"_net_share_statisticsgetwrk _now _nowcalc _nowcalcdate _nowdate _nowtime _pathfull _pathgetrelative "
"_pathmake _pathsplit _processgetname _processgetpriority _radian _replacestringinfile _rundos "
"_screencapture_capture _screencapture_capturewnd _screencapture_saveimage _screencapture_setbmpformat "
"_screencapture_setjpgquality _screencapture_settifcolordepth _screencapture_settifcompression "
"_security__adjusttokenprivileges _security__createprocesswithtoken _security__duplicatetokenex "
"_security__getaccountsid _security__getlengthsid _security__gettokeninformation _security__impersonateself "
"_security__isvalidsid _security__lookupaccountname _security__lookupaccountsid "
"_security__lookupprivilegevalue _security__openprocesstoken _security__openthreadtoken "
"_security__openthreadtokenex _security__setprivilege _security__settokeninformation "
"_security__sidtostringsid _security__sidtypestr _security__stringsidtosid _sendmessage _sendmessagea "
"_setdate _settime _singleton _soundclose _soundlength _soundopen _soundpause _soundplay _soundpos "
"_soundresume _soundseek _soundstatus _soundstop _sqlite_changes _sqlite_close _sqlite_display2dresult "
"_sqlite_encode _sqlite_errcode _sqlite_errmsg _sqlite_escape _sqlite_exec _sqlite_fastencode "
"_sqlite_fastescape _sqlite_fetchdata _sqlite_fetchnames _sqlite_gettable _sqlite_gettable2d "
"_sqlite_lastinsertrowid _sqlite_libversion _sqlite_open _sqlite_query _sqlite_queryfinalize "
"_sqlite_queryreset _sqlite_querysinglerow _sqlite_safemode _sqlite_settimeout _sqlite_shutdown "
"_sqlite_sqliteexe _sqlite_startup _sqlite_totalchanges _stringbetween _stringencrypt _stringexplode "
"_stringinsert _stringproper _stringrepeat _stringreverse _stringtohex _tcpiptoname _tempfile _tickstotime "
"_timer_diff _timer_getidletime _timer_gettimerid _timer_init _timer_killalltimers _timer_killtimer "
"_timer_settimer _timetoticks _versioncompare _viclose _viexeccommand _vifindgpib _vigpibbusreset _vigtl "
"_viinteractivecontrol _viopen _visetattribute _visettimeout _weeknumberiso _winapi_attachconsole "
"_winapi_attachthreadinput _winapi_beep _winapi_bitblt _winapi_callnexthookex _winapi_callwindowproc "
"_winapi_clienttoscreen _winapi_closehandle _winapi_combinergn _winapi_commdlgextendederror "
"_winapi_copyicon _winapi_createbitmap _winapi_createcompatiblebitmap _winapi_createcompatibledc "
"_winapi_createevent _winapi_createfile _winapi_createfont _winapi_createfontindirect _winapi_createpen "
"_winapi_createprocess _winapi_createrectrgn _winapi_createroundrectrgn _winapi_createsolidbitmap "
"_winapi_createsolidbrush _winapi_createwindowex _winapi_defwindowproc _winapi_deletedc "
"_winapi_deleteobject _winapi_destroyicon _winapi_destroywindow _winapi_drawedge _winapi_drawframecontrol "
"_winapi_drawicon _winapi_drawiconex _winapi_drawline _winapi_drawtext _winapi_duplicatehandle "
"_winapi_enablewindow _winapi_enumdisplaydevices _winapi_enumwindows _winapi_enumwindowspopup "
"_winapi_enumwindowstop _winapi_expandenvironmentstrings _winapi_extracticonex _winapi_fatalappexit "
"_winapi_fillrect _winapi_findexecutable _winapi_findwindow _winapi_flashwindow _winapi_flashwindowex "
"_winapi_floattoint _winapi_flushfilebuffers _winapi_formatmessage _winapi_framerect _winapi_freelibrary "
"_winapi_getancestor _winapi_getasynckeystate _winapi_getbkmode _winapi_getclassname "
"_winapi_getclientheight _winapi_getclientrect _winapi_getclientwidth _winapi_getcurrentprocess "
"_winapi_getcurrentprocessid _winapi_getcurrentthread _winapi_getcurrentthreadid _winapi_getcursorinfo "
"_winapi_getdc _winapi_getdesktopwindow _winapi_getdevicecaps _winapi_getdibits _winapi_getdlgctrlid "
"_winapi_getdlgitem _winapi_getfilesizeex _winapi_getfocus _winapi_getforegroundwindow "
"_winapi_getguiresources _winapi_geticoninfo _winapi_getlasterror _winapi_getlasterrormessage "
"_winapi_getlayeredwindowattributes _winapi_getmodulehandle _winapi_getmousepos _winapi_getmouseposx "
"_winapi_getmouseposy _winapi_getobject _winapi_getopenfilename _winapi_getoverlappedresult "
"_winapi_getparent _winapi_getprocessaffinitymask _winapi_getsavefilename _winapi_getstdhandle "
"_winapi_getstockobject _winapi_getsyscolor _winapi_getsyscolorbrush _winapi_getsystemmetrics "
"_winapi_gettextextentpoint32 _winapi_gettextmetrics _winapi_getwindow _winapi_getwindowdc "
"_winapi_getwindowheight _winapi_getwindowlong _winapi_getwindowplacement _winapi_getwindowrect "
"_winapi_getwindowrgn _winapi_getwindowtext _winapi_getwindowthreadprocessid _winapi_getwindowwidth "
"_winapi_getxyfrompoint _winapi_globalmemorystatus _winapi_guidfromstring _winapi_guidfromstringex "
"_winapi_hiword _winapi_inprocess _winapi_inttofloat _winapi_invalidaterect _winapi_isclassname "
"_winapi_iswindow _winapi_iswindowvisible _winapi_lineto _winapi_loadbitmap _winapi_loadimage "
"_winapi_loadlibrary _winapi_loadlibraryex _winapi_loadshell32icon _winapi_loadstring _winapi_localfree "
"_winapi_loword _winapi_makelangid _winapi_makelcid _winapi_makelong _winapi_makeqword _winapi_messagebeep "
"_winapi_mouse_event _winapi_moveto _winapi_movewindow _winapi_msgbox _winapi_muldiv "
"_winapi_multibytetowidechar _winapi_multibytetowidecharex _winapi_openprocess _winapi_pathfindonpath "
"_winapi_pointfromrect _winapi_postmessage _winapi_primarylangid _winapi_ptinrect _winapi_readfile "
"_winapi_readprocessmemory _winapi_rectisempty _winapi_redrawwindow _winapi_registerwindowmessage "
"_winapi_releasecapture _winapi_releasedc _winapi_screentoclient _winapi_selectobject _winapi_setbkcolor "
"_winapi_setbkmode _winapi_setcapture _winapi_setcursor _winapi_setdefaultprinter _winapi_setdibits "
"_winapi_setendoffile _winapi_setevent _winapi_setfilepointer _winapi_setfocus _winapi_setfont "
"_winapi_sethandleinformation _winapi_setlasterror _winapi_setlayeredwindowattributes _winapi_setparent "
"_winapi_setprocessaffinitymask _winapi_setsyscolors _winapi_settextcolor _winapi_setwindowlong "
"_winapi_setwindowplacement _winapi_setwindowpos _winapi_setwindowrgn _winapi_setwindowshookex "
"_winapi_setwindowtext _winapi_showcursor _winapi_showerror _winapi_showmsg _winapi_showwindow "
"_winapi_stringfromguid _winapi_stringlena _winapi_stringlenw _winapi_sublangid "
"_winapi_systemparametersinfo _winapi_twipsperpixelx _winapi_twipsperpixely _winapi_unhookwindowshookex "
"_winapi_updatelayeredwindow _winapi_updatewindow _winapi_waitforinputidle _winapi_waitformultipleobjects "
"_winapi_waitforsingleobject _winapi_widechartomultibyte _winapi_windowfrompoint _winapi_writeconsole "
"_winapi_writefile _winapi_writeprocessmemory _winnet_addconnection _winnet_addconnection2 "
"_winnet_addconnection3 _winnet_cancelconnection _winnet_cancelconnection2 _winnet_closeenum "
"_winnet_connectiondialog _winnet_connectiondialog1 _winnet_disconnectdialog _winnet_disconnectdialog1 "
"_winnet_enumresource _winnet_getconnection _winnet_getconnectionperformance _winnet_getlasterror "
"_winnet_getnetworkinformation _winnet_getprovidername _winnet_getresourceinformation "
"_winnet_getresourceparent _winnet_getuniversalname _winnet_getuser _winnet_openenum "
"_winnet_restoreconnection _winnet_useconnection _word_versioninfo _wordattach _wordcreate _worddocadd "
"_worddocaddlink _worddocaddpicture _worddocclose _worddocfindreplace _worddocgetcollection "
"_worddoclinkgetcollection _worddocopen _worddocprint _worddocpropertyget _worddocpropertyset _worddocsave "
"_worddocsaveas _worderrorhandlerderegister _worderrorhandlerregister _worderrornotify _wordmacrorun "
"_wordpropertyget _wordpropertyset _wordquit",
"" };


EDITLEXER lexAU3 = { SCLEX_AU3, 63276, L"AutoIt3 Script", L"au3", L"", &KeyWords_AU3, {
                     { SCE_AU3_DEFAULT, 63126, L"Default", L"", L"" },
                     { MULTI_STYLE(SCE_AU3_COMMENT,SCE_AU3_COMMENTBLOCK,0,0), 63127, L"Comment", L"fore:#008000", L"" },
                     { SCE_AU3_NUMBER, 63130, L"Number", L"fore:#008080", L"" },
                     { SCE_AU3_FUNCTION, 63277, L"Function", L"fore:#0000FF", L"" },
                     { SCE_AU3_UDF, 63360, L"User-Defined Function", L"fore:#0000FF", L"" },
                     { SCE_AU3_KEYWORD, 63128, L"Keyword", L"fore:#0000FF", L"" },
                     { SCE_AU3_MACRO, 63278, L"Macro", L"fore:#0080FF", L"" },
                     { SCE_AU3_STRING, 63131, L"String", L"fore:#008080", L"" },
                     { SCE_AU3_OPERATOR, 63132, L"Operator", L"fore:#C000C0", L"" },
                     { SCE_AU3_VARIABLE, 63249, L"Variable", L"fore:#808000", L"" },
                     { SCE_AU3_SENT, 63279, L"Send Key", L"fore:#FF0000", L"" },
                     { SCE_AU3_PREPROCESSOR, 63133, L"Preprocessor", L"fore:#FF8000", L"" },
                     { SCE_AU3_SPECIAL, 63280, L"Special", L"fore:#FF8000", L"" },
                     { -1, 00000, L"", L"", L"" } } };


EDITLEXER lexLATEX = { SCLEX_LATEX, 63281, L"LaTeX Files", L"tex; latex; sty", L"", &KeyWords_NULL, {
                       { SCE_L_DEFAULT, 63126, L"Default", L"", L"" },
                       { MULTI_STYLE(SCE_L_COMMAND,SCE_L_SHORTCMD,SCE_L_CMDOPT,0), 63236, L"Command", L"fore:#0000FF", L"" },
                       { MULTI_STYLE(SCE_L_COMMENT,SCE_L_COMMENT2,0,0), 63127, L"Comment", L"fore:#008000", L"" },
                       { MULTI_STYLE(SCE_L_MATH,SCE_L_MATH2,0,0), 63283, L"Math", L"fore:#FF0000", L"" },
                       { SCE_L_SPECIAL, 63330, L"Special Char", L"fore:#AAAA00", L"" },
                       { MULTI_STYLE(SCE_L_TAG,SCE_L_TAG2,0,0), 63282, L"Tag", L"fore:#0000FF", L"" },
                       { SCE_L_VERBATIM, 63331, L"Verbatim Segment", L"fore:#666666", L"" },
                       { -1, 00000, L"", L"", L"" } } };


EDITLEXER lexANSI = { SCLEX_NULL, 63258, L"ANSI Art", L"nfo; diz", L"", &KeyWords_NULL, {
                      { STYLE_DEFAULT, 63126, L"Default", L"font:Lucida Console", L"" },
                      { STYLE_LINENUMBER, 63101, L"Margins and Line Numbers", L"font:Lucida Console; size:-2", L"" },
                      { STYLE_BRACELIGHT, 63102, L"Matching Braces", L"size:+0", L"" },
                      { STYLE_BRACEBAD, 63103, L"Matching Braces Error", L"size:+0", L"" },
                      { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_AHK = {
"break continue else exit exitapp gosub goto if ifequal ifexist ifgreater ifgreaterorequal "
"ifinstring ifless iflessorequal ifmsgbox ifnotequal ifnotexist ifnotinstring ifwinactive "
"ifwinexist ifwinnotactive ifwinnotexist loop onexit pause repeat return settimer sleep "
"suspend static global local var byref while until for class try catch throw",
"autotrim blockinput clipwait control controlclick controlfocus controlget controlgetfocus "
"controlgetpos controlgettext controlmove controlsend controlsendraw controlsettext coordmode "
"critical detecthiddentext detecthiddenwindows drive driveget drivespacefree edit endrepeat "
"envadd envdiv envget envmult envset envsub envupdate fileappend filecopy filecopydir filecreatedir "
"filecreateshortcut filedelete filegetattrib filegetshortcut filegetsize filegettime filegetversion "
"fileinstall filemove filemovedir fileread filereadline filerecycle filerecycleempty fileremovedir "
"fileselectfile fileselectfolder filesetattrib filesettime formattime getkeystate groupactivate "
"groupadd groupclose groupdeactivategui guicontrol guicontrolget hideautoitwin hotkey imagesearch "
"inidelete iniread iniwrite input inputbox keyhistory keywait listhotkeys listlines listvars menu "
"mouseclick mouseclickdrag mousegetpos mousemove msgbox outputdebug pixelgetcolor pixelsearch "
"postmessage process progress random regdelete regread regwrite reload run runas runwait send "
"sendevent sendinput sendmessage sendmode sendplay sendraw setbatchlines setcapslockstate "
"setcontroldelay setdefaultmousespeed setenv setformat setkeydelay setmousedelay setnumlockstate "
"setscrolllockstate setstorecapslockmode settitlematchmode setwindelay setworkingdir shutdown sort "
"soundbeep soundget soundgetwavevolume soundplay soundset soundsetwavevolume splashimage splashtextoff "
"splashtexton splitpath statusbargettext statusbarwait stringcasesense stringgetpos stringleft stringlen "
"stringlower stringmid stringreplace stringright stringsplit stringtrimleft stringtrimright stringupper "
"sysget thread tooltip transform traytip urldownloadtofile winactivate winactivatebottom winclose winget "
"wingetactivestats wingetactivetitle wingetclass wingetpos wingettext wingettitle winhide winkill "
"winmaximize winmenuselectitem winminimize winminimizeall winminimizeallundo winmove winrestore winset "
"winsettitle winshow winwait winwaitactive winwaitclose winwaitnotactive fileencoding",
"abs acos asc asin atan ceil chr cos dllcall exp fileexist floor getkeystate numget numput "
"registercallback il_add il_create il_destroy instr islabel isfunc ln log lv_add lv_delete "
"lv_deletecol lv_getcount lv_getnext lv_gettext lv_insert lv_insertcol lv_modify lv_modifycol "
"lv_setimagelist mod onmessage round regexmatch regexreplace sb_seticon sb_setparts sb_settext "
"sin sqrt strlen substr tan tv_add tv_delete tv_getchild tv_getcount tv_getnext tv_get tv_getparent "
"tv_getprev tv_getselection tv_gettext tv_modify tv_setimagelist varsetcapacity winactive winexist "
"trim ltrim rtrim fileopen strget strput object array isobject objinsert objremove objminindex "
"objmaxindex objsetcapacity objgetcapacity objgetaddress objnewenum objaddref objrelease objhaskey "
"objclone _insert _remove _minindex _maxindex _setcapacity _getcapacity _getaddress _newenum _addref "
"_release _haskey _clone comobjcreate comobjget comobjconnect comobjerror comobjactive comobjenwrap "
"comobjunwrap comobjparameter comobjmissing comobjtype comobjvalue comobjarray comobjquery comobjflags "
"func getkeyname getkeyvk getkeysc isbyref exception",
"allowsamelinecomments clipboardtimeout commentflag errorstdout escapechar hotkeyinterval "
"hotkeymodifiertimeout hotstring if iftimeout ifwinactive ifwinexist include includeagain "
"installkeybdhook installmousehook keyhistory ltrim maxhotkeysperinterval maxmem maxthreads "
"maxthreadsbuffer maxthreadsperhotkey menumaskkey noenv notrayicon persistent singleinstance "
"usehook warn winactivateforce",
"shift lshift rshift alt lalt ralt control lcontrol rcontrol ctrl lctrl rctrl lwin rwin appskey "
"altdown altup shiftdown shiftup ctrldown ctrlup lwindown lwinup rwindown rwinup lbutton rbutton "
"mbutton wheelup wheeldown xbutton1 xbutton2 joy1 joy2 joy3 joy4 joy5 joy6 joy7 joy8 joy9 joy10 "
"joy11 joy12 joy13 joy14 joy15 joy16 joy17 joy18 joy19 joy20 joy21 joy22 joy23 joy24 joy25 joy26 "
"joy27 joy28 joy29 joy30 joy31 joy32 joyx joyy joyz joyr joyu joyv joypov joyname joybuttons "
"joyaxes joyinfo space tab enter escape esc backspace bs delete del insert ins pgup pgdn home end "
"up down left right printscreen ctrlbreak pause scrolllock capslock numlock numpad0 numpad1 numpad2 "
"numpad3 numpad4 numpad5 numpad6 numpad7 numpad8 numpad9 numpadmult numpadadd numpadsub numpaddiv "
"numpaddot numpaddel numpadins numpadclear numpadup numpaddown numpadleft numpadright numpadhome "
"numpadend numpadpgup numpadpgdn numpadenter f1 f2 f3 f4 f5 f6 f7 f8 f9 f10 f11 f12 f13 f14 f15 "
"f16 f17 f18 f19 f20 f21 f22 f23 f24 browser_back browser_forward browser_refresh browser_stop "
"browser_search browser_favorites browser_home volume_mute volume_down volume_up media_next "
"media_prev media_stop media_play_pause launch_mail launch_media launch_app1 launch_app2 blind "
"click raw wheelleft wheelright",
"a_ahkpath a_ahkversion a_appdata a_appdatacommon a_autotrim a_batchlines a_caretx a_carety "
"a_computername a_controldelay a_cursor a_dd a_ddd a_dddd a_defaultmousespeed a_desktop "
"a_desktopcommon a_detecthiddentext a_detecthiddenwindows a_endchar a_eventinfo a_exitreason "
"a_formatfloat a_formatinteger a_gui a_guievent a_guicontrol a_guicontrolevent a_guiheight "
"a_guiwidth a_guix a_guiy a_hour a_iconfile a_iconhidden a_iconnumber a_icontip a_index "
"a_ipaddress1 a_ipaddress2 a_ipaddress3 a_ipaddress4 a_isadmin a_iscompiled a_issuspended "
"a_keydelay a_language a_lasterror a_linefile a_linenumber a_loopfield a_loopfileattrib "
"a_loopfiledir a_loopfileext a_loopfilefullpath a_loopfilelongpath a_loopfilename "
"a_loopfileshortname a_loopfileshortpath a_loopfilesize a_loopfilesizekb a_loopfilesizemb "
"a_loopfiletimeaccessed a_loopfiletimecreated a_loopfiletimemodified a_loopreadline a_loopregkey "
"a_loopregname a_loopregsubkey a_loopregtimemodified a_loopregtype a_mday a_min a_mm a_mmm "
"a_mmmm a_mon a_mousedelay a_msec a_mydocuments a_now a_nowutc a_numbatchlines a_ostype "
"a_osversion a_priorhotkey a_programfiles a_programs a_programscommon a_screenheight "
"a_screenwidth a_scriptdir a_scriptfullpath a_scriptname a_sec a_space a_startmenu "
"a_startmenucommon a_startup a_startupcommon a_stringcasesense a_tab a_temp a_thishotkey "
"a_thismenu a_thismenuitem a_thismenuitempos a_tickcount a_timeidle a_timeidlephysical "
"a_timesincepriorhotkey a_timesincethishotkey a_titlematchmode a_titlematchmodespeed "
"a_username a_wday a_windelay a_windir a_workingdir a_yday a_year a_yweek a_yyyy "
"clipboard clipboardall comspec errorlevel programfiles true false a_thisfunc a_thislabel "
"a_ispaused a_iscritical a_isunicode a_ptrsize a_scripthwnd a_priorkey",
"ltrim rtrim join ahk_id ahk_pid ahk_class ahk_group ahk_exe processname processpath minmax "
"controllist statuscd filesystem setlabel alwaysontop mainwindow nomainwindow useerrorlevel "
"altsubmit hscroll vscroll imagelist wantctrla wantf2 vis visfirst wantreturn backgroundtrans "
"minimizebox maximizebox sysmenu toolwindow exstyle check3 checkedgray readonly notab lastfound "
"lastfoundexist alttab shiftalttab alttabmenu alttabandmenu alttabmenudismiss controllisthwnd "
"hwnd deref pow bitnot bitand bitor bitxor bitshiftleft bitshiftright sendandmouse mousemove "
"mousemoveoff hkey_local_machine hkey_users hkey_current_user hkey_classes_root hkey_current_config "
"hklm hku hkcu hkcr hkcc reg_sz reg_expand_sz reg_multi_sz reg_dword reg_qword reg_binary reg_link "
"reg_resource_list reg_full_resource_descriptor reg_resource_requirements_list reg_dword_big_endian "
"regex pixel mouse screen relative rgb low belownormal normal abovenormal high realtime between "
"contains in is integer float number digit xdigit integerfast floatfast alpha upper lower alnum "
"time date not or and topmost top bottom transparent transcolor redraw region id idlast count "
"list capacity eject lock unlock label serial type status seconds minutes hours days read parse "
"logoff close error single shutdown menu exit reload tray add rename check uncheck togglecheck "
"enable disable toggleenable default nodefault standard nostandard color delete deleteall icon "
"noicon tip click show edit progress hotkey text picture pic groupbox button checkbox radio "
"dropdownlist ddl combobox statusbar treeview listbox listview datetime monthcal updown slider "
"tab tab2 activex iconsmall tile report sortdesc nosort nosorthdr grid hdr autosize range xm ym "
"ys xs xp yp font resize owner submit nohide minimize maximize restore noactivate na cancel "
"destroy center margin owndialogs guiescape guiclose guisize guicontextmenu guidropfiles tabstop "
"section wrap border top bottom buttons expand first lines number uppercase lowercase limit "
"password multi group background bold italic strike underline norm theme caption delimiter flash "
"style checked password hidden left right center section move focus hide choose choosestring text "
"pos enabled disabled visible notimers interrupt priority waitclose unicode tocodepage fromcodepage "
"yes no ok cancel abort retry ignore force on off all send wanttab monitorcount monitorprimary "
"monitorname monitorworkarea pid this base extends __get __set __call __delete __new new "
"useunsetlocal useunsetglobal useenv localsameasglobal",
"", "" };


EDITLEXER lexAHK = { SCLEX_AHK, 63305, L"AutoHotkey Script", L"ahk; ia; scriptlet", L"", &KeyWords_AHK, {
                     { SCE_AHK_DEFAULT, 63126, L"Default", L"", L"" },
                     { MULTI_STYLE(SCE_AHK_COMMENTLINE,SCE_AHK_COMMENTBLOCK,0,0), 63127, L"Comment", L"fore:#008000", L"" },
                     { SCE_AHK_ESCAPE, 63306, L"Escape", L"fore:#FF8000", L"" },
                     { SCE_AHK_SYNOPERATOR, 63307, L"Syntax Operator", L"fore:#7F200F", L"" },
                     { SCE_AHK_EXPOPERATOR, 63308, L"Expression operator", L"fore:#FF4F00", L"" },
                     { SCE_AHK_STRING, 63131, L"String", L"fore:#404040", L"" },
                     { SCE_AHK_NUMBER, 63130, L"Number", L"fore:#2F4F7F", L"" },
                     { SCE_AHK_IDENTIFIER, 63129, L"Identifier", L"fore:#CF2F0F", L"" },
                     { SCE_AHK_VARREF, 63309, L"Variable dereferencing", L"fore:#CF2F0F; back:#E4FFE4", L"" },
                     { SCE_AHK_LABEL, 63235, L"Label", L"fore:#000000; back:#FFFFA1", L"" },
                     { SCE_AHK_WORD_CF, 63310, L"Flow of control", L"fore:#480048; bold", L"" },
                     { SCE_AHK_WORD_CMD, 63236, L"Command", L"fore:#004080", L"" },
                     { SCE_AHK_WORD_FN, 63277, L"Function", L"fore:#0F707F; italics", L"" },
                     { SCE_AHK_WORD_DIR, 63203, L"Directive", L"fore:#F04020; italics", L"" },
                     { SCE_AHK_WORD_KB, 63311, L"Keys & buttons", L"fore:#FF00FF; bold", L"" },
                     { SCE_AHK_WORD_VAR, 63312, L"Built-in Variables", L"fore:#CF00CF; italics", L"" },
                     { SCE_AHK_WORD_SP, 63280, L"Special", L"fore:#0000FF; italics", L"" },
                     //{ SCE_AHK_WORD_UD, 63106, L"User Defined", L"fore:#800020", L"" },
                     { SCE_AHK_VARREFKW, 63313, L"Variable keyword", L"fore:#CF00CF; italics; back:#F9F9FF", L"" },
                     { SCE_AHK_ERROR, 63261, L"Error", L"back:#FFC0C0", L"" },
                     { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_CMAKE = {
"add_custom_command add_custom_target add_definitions add_dependencies add_executable add_library "
"add_subdirectory add_test aux_source_directory build_command build_name cmake_minimum_required "
"configure_file create_test_sourcelist else elseif enable_language enable_testing endforeach endif "
"endmacro endwhile exec_program execute_process export_library_dependencies file find_file find_library "
"find_package find_path find_program fltk_wrap_ui foreach get_cmake_property get_directory_property "
"get_filename_component get_source_file_property get_target_property get_test_property if include "
"include_directories include_external_msproject include_regular_expression install install_files "
"install_programs install_targets link_directories link_libraries list load_cache load_command "
"macro make_directory mark_as_advanced math message option output_required_files project qt_wrap_cpp "
"qt_wrap_ui remove remove_definitions separate_arguments set set_directory_properties set_source_files_properties "
"set_target_properties set_tests_properties site_name source_group string subdir_depends subdirs "
"target_link_libraries try_compile try_run use_mangled_mesa utility_source variable_requires vtk_make_instantiator "
"vtk_wrap_java vtk_wrap_python vtk_wrap_tcl while write_file",
"ABSOLUTE ABSTRACT ADDITIONAL_MAKE_CLEAN_FILES ALL AND APPEND ARGS ASCII BEFORE CACHE CACHE_VARIABLES "
"CLEAR COMMAND COMMANDS COMMAND_NAME COMMENT COMPARE COMPILE_FLAGS COPYONLY DEFINED DEFINE_SYMBOL "
"DEPENDS DOC EQUAL ESCAPE_QUOTES EXCLUDE EXCLUDE_FROM_ALL EXISTS EXPORT_MACRO EXT EXTRA_INCLUDE "
"FATAL_ERROR FILE FILES FORCE FUNCTION GENERATED GLOB GLOB_RECURSE GREATER GROUP_SIZE HEADER_FILE_ONLY "
"HEADER_LOCATION IMMEDIATE INCLUDES INCLUDE_DIRECTORIES INCLUDE_INTERNALS INCLUDE_REGULAR_EXPRESSION "
"LESS LINK_DIRECTORIES LINK_FLAGS LOCATION MACOSX_BUNDLE MACROS MAIN_DEPENDENCY MAKE_DIRECTORY MATCH "
"MATCHALL MATCHES MODULE NAME NAME_WE NOT NOTEQUAL NO_SYSTEM_PATH OBJECT_DEPENDS OPTIONAL OR OUTPUT "
"OUTPUT_VARIABLE PATH PATHS POST_BUILD POST_INSTALL_SCRIPT PREFIX PREORDER PRE_BUILD PRE_INSTALL_SCRIPT "
"PRE_LINK PROGRAM PROGRAM_ARGS PROPERTIES QUIET RANGE READ REGEX REGULAR_EXPRESSION REPLACE REQUIRED "
"RETURN_VALUE RUNTIME_DIRECTORY SEND_ERROR SHARED SOURCES STATIC STATUS STREQUAL STRGREATER STRLESS "
"SUFFIX TARGET TOLOWER TOUPPER VAR VARIABLES VERSION WIN32 WRAP_EXCLUDE WRITE APPLE MINGW MSYS CYGWIN "
"BORLAND WATCOM MSVC MSVC_IDE MSVC60 MSVC70 MSVC71 MSVC80 CMAKE_COMPILER_2005 OFF ON",
"", "", "", "", "", "", "" };


EDITLEXER lexCmake = { SCLEX_CMAKE, 63324, L"Cmake Script", L"cmake; ctest", L"", &KeyWords_CMAKE, {
                       { SCE_CMAKE_DEFAULT, 63126, L"Default", L"", L"" },
                       { SCE_CMAKE_COMMENT, 63127, L"Comment", L"fore:#008000", L"" },
                       { MULTI_STYLE(SCE_CMAKE_STRINGDQ,SCE_CMAKE_STRINGLQ,SCE_CMAKE_STRINGRQ,0), 63131, L"String", L"back:#EEEEEE; fore:#7F007F", L"" },
                       { SCE_CMAKE_COMMANDS, 63277, L"Function", L"fore:#00007F", L"" },
                       { SCE_CMAKE_PARAMETERS, 63294, L"Parameter", L"fore:#7F200F", L"" },
                       { SCE_CMAKE_VARIABLE, 63249, L"Variable", L"fore:#CC3300", L"" },
                       { SCE_CMAKE_WHILEDEF, 63325, L"While def", L"fore:#00007F", L"" },
                       { SCE_CMAKE_FOREACHDEF, 63326, L"For each def", L"fore:#00007F", L"" },
                       { SCE_CMAKE_IFDEFINEDEF, 63327, L"If def", L"fore:#00007F", L"" },
                       { SCE_CMAKE_MACRODEF, 63328, L"Macro def", L"fore:#00007F", L"" },
                       { SCE_CMAKE_STRINGVAR, 63329, L"Variable within string", L"back:#EEEEEE; fore:#CC3300", L"" },
                       { SCE_CMAKE_NUMBER, 63130, L"Number", L"fore:#008080", L"" },
                       //{ SCE_CMAKE_USERDEFINED, 63106, L"User Defined", L"fore:#800020", L"" },
                       { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_AVS = {
"true false return global",
"addborders alignedsplice amplify amplifydb animate applyrange assumebff assumefieldbased assumefps "
"assumeframebased assumesamplerate assumescaledfps assumetff audiodub audiodubex avifilesource "
"avisource bicubicresize bilinearresize blackmanresize blackness blankclip blur bob cache changefps "
"colorbars colorkeymask coloryuv compare complementparity conditionalfilter conditionalreader "
"convertaudio convertaudioto16bit convertaudioto24bit convertaudioto32bit convertaudioto8bit "
"convertaudiotofloat convertbacktoyuy2 convertfps converttobackyuy2 converttomono converttorgb "
"converttorgb24 converttorgb32 converttoy8 converttoyv16 converttoyv24 converttoyv411 converttoyuy2 "
"converttoyv12 crop cropbottom delayaudio deleteframe dissolve distributor doubleweave duplicateframe "
"ensurevbrmp3sync fadein fadein0 fadein2 fadeio fadeio0 fadeio2 fadeout fadeout0 fadeout2 fixbrokenchromaupsampling "
"fixluminance fliphorizontal flipvertical frameevaluate freezeframe gaussresize generalconvolution "
"getchannel getchannels getmtmode getparity grayscale greyscale histogram horizontalreduceby2 "
"imagereader imagesource imagewriter info interleave internalcache internalcachemt invert killaudio "
"killvideo lanczos4resize lanczosresize layer letterbox levels limiter loop mask maskhs max merge "
"mergeargb mergechannels mergechroma mergeluma mergergb messageclip min mixaudio monotostereo normalize "
"null opendmlsource overlay peculiarblend pointresize pulldown reduceby2 resampleaudio resetmask reverse "
"rgbadjust scriptclip segmentedavisource segmenteddirectshowsource selecteven selectevery selectodd "
"selectrangeevery separatefields setmtmode sharpen showalpha showblue showfiveversions showframenumber "
"showgreen showred showsmpte showtime sincresize skewrows spatialsoften spline16resize spline36resize "
"spline64resize ssrc stackhorizontal stackvertical subtitle subtract supereq swapfields swapuv "
"temporalsoften timestretch tone trim turn180 turnleft turnright tweak unalignedsplice utoy utoy8 "
"version verticalreduceby2 vtoy vtoy8 wavsource weave writefile writefileend writefileif writefilestart "
"ytouv",
"addgrain addgrainc agc_hdragc analyzelogo animeivtc asharp audiograph autocrop autoyuy2 avsrecursion "
"awarpsharp bassaudiosource bicublinresize bifrost binarize blendfields blindpp blockbuster bordercontrol "
"cfielddiff cframediff chromashift cnr2 colormatrix combmask contra convolution3d convolution3dyv12 "
"dctfilter ddcc deblendlogo deblock deblock_qed decimate decomb dedup deen deflate degrainmedian depan "
"depanestimate depaninterleave depanscenes depanstabilize descratch despot dfttest dgbob dgdecode_mpeg2source "
"dgsource directshowsource distancefunction dss2 dup dupmc edeen edgemask ediupsizer eedi2 eedi3 eedi3_rpow2 "
"expand faerydust fastbicubicresize fastbilinearresize fastediupsizer dedgemask fdecimate ffaudiosource "
"ffdshow ffindex ffmpegsource ffmpegsource2 fft3dfilter fft3dgpu ffvideosource fielddeinterlace fielddiff "
"fillmargins fity2uv fity2u fity2v fitu2y fitv2y fluxsmooth fluxsmoothst fluxsmootht framediff framenumber "
"frfun3b frfun7 gicocu golddust gradfun2db grapesmoother greedyhma grid guavacomb hqdn3d hybridfupp "
"hysteresymask ibob improvesceneswitch inflate inpand inpaintlogo interframe interlacedresize "
"interlacedwarpedresize interleaved2planar iscombed iscombedt iscombedtivtc kerneldeint leakkernelbob "
"leakkerneldeint limitedsharpen limitedsharpenfaster logic lsfmod lumafilter lumayv12 manalyse "
"maskeddeinterlace maskedmerge maskedmix mblockfps mcompensate mctemporaldenoise mctemporaldenoisepp "
"mdegrain1 mdegrain2 mdegrain3 mdepan medianblur mergehints mflow mflowblur mflowfps mflowinter minblur "
"mipsmooth mmask moderatesharpen monitorfilter motionmask mpasource mpeg2source mrecalculate mscdetection "
"msharpen mshow msmooth msu_fieldshiftfixer msu_frc msuper mt mt_adddiff mt_average mt_binarize mt_circle "
"mt_clamp mt_convolution mt_deflate mt_diamond mt_edge mt_ellipse mt_expand mt_freeellipse mt_freelosange "
"mt_freerectangle mt_hysteresis mt_infix mt_inflate mt_inpand mt_invert mt_logic mt_losange mt_lut mt_lutf "
"mt_luts mt_lutspa mt_lutsx mt_lutxy mt_lutxyz mt_makediff mt_mappedblur mt_merge mt_motion mt_polish "
"mt_rectangle mt_square mti mtsource multidecimate mvanalyse mvblockfps mvchangecompensate mvcompensate "
"mvdegrain1 mvdegrain2 mvdegrain3 mvdenoise mvdepan mvflow mvflowblur mvflowfps mvflowfps2 mvflowinter "
"mvincrease mvmask mvrecalculate mvscdetection mvshow nicac3source nicdtssource niclpcmsource nicmpasource "
"nicmpg123source nnedi nnedi2 nnedi2_rpow2 nnedi3 nnedi3_rpow2 nomosmooth overlaymask peachsmoother pixiedust "
"planar2interleaved qtgmc qtinput rawavsource rawsource reduceflicker reinterpolate411 removedirt removedust "
"removegrain removegrainhd removetemporalgrain repair requestlinear reversefielddominance rgb3dlut rgdeinterlace "
"rgsdeinterlace rgblut rotate sangnom seesaw sharpen2 showchannels showcombedtivtc smartdecimate smartdeinterlace "
"smdegrain smoothdeinterlace smoothuv soothess soxfilter spacedust sshiq ssim ssiq stmedianfilter t3dlut tanisotropic "
"tbilateral tcanny tcomb tcombmask tcpserver tcpsource tdecimate tdeint tedgemask telecide temporalcleaner "
"temporalrepair temporalsmoother tfieldblank tfm tisophote tivtc tmaskblank tmaskedmerge tmaskedmerge3 tmm "
"tmonitor tnlmeans tomsmocomp toon textsub ttempsmooth ttempsmoothf tunsharp unblock uncomb undot unfilter "
"unsharpmask vaguedenoiser variableblur verticalcleaner videoscope vinverse vobsub vqmcalc warpedresize warpsharp "
"xsharpen yadif yadifmod yuy2lut yv12convolution yv12interlacedreduceby2 yv12interlacedselecttopfields "
"yv12layer yv12lut yv12lutxy yv12substract yv12torgb24 yv12toyuy2",
"abs apply assert bool ceil chr clip continueddenominator continuednumerator cos default defined eval "
"averagechromau averagechromav averageluma chromaudifference chromavdifference lumadifference "
"exist exp findstr float floor frac hexvalue import int isbool isclip isfloat isint isstring lcase leftstr "
"load_stdcall_plugin loadcplugin loadplugin loadvfapiplugin loadvirtualdubplugin log midstr muldiv nop "
"opt_allowfloataudio opt_avipadscanlines opt_dwchannelmask opt_usewaveextensible opt_vdubplanarhack "
"pi pow rand revstr rightstr round scriptdir scriptfile scriptname select setmemorymax setplanarlegacyalignment "
"rgbdifference rgbdifferencefromprevious rgbdifferencetonext udifferencefromprevious udifferencetonext "
"setworkingdir sign sin spline sqrt string strlen time ucase undefined value versionnumber versionstring "
"uplanemax uplanemedian uplanemin uplaneminmaxdifference vdifferencefromprevious vdifferencetonext "
"vplanemax vplanemedian vplanemin vplaneminmaxdifference ydifferencefromprevious ydifferencetonext "
"yplanemax yplanemedian yplanemin yplaneminmaxdifference",
"audiobits audiochannels audiolength audiolengthf audiorate framecount framerate frameratedenominator "
"frameratenumerator getleftchannel getrightchannel hasaudio hasvideo height isaudiofloat isaudioint "
"isfieldbased isframebased isinterleaved isplanar isrgb isrgb24 isrgb32 isyuv isyuy2 isyv12 width",
"", "", "", "" };


EDITLEXER lexAVS = { SCLEX_AVS, 63332, L"AviSynth Script", L"avs; avsi", L"", &KeyWords_AVS, {
                     { SCE_AVS_DEFAULT, 63126, L"Default", L"", L"" },
                     { MULTI_STYLE(SCE_AVS_COMMENTLINE,SCE_AVS_COMMENTBLOCK,SCE_AVS_COMMENTBLOCKN,0), 63127, L"Comment", L"fore:#008000", L"" },
                     { SCE_AVS_OPERATOR, 63132, L"Operator", L"", L"" },
                     { MULTI_STYLE(SCE_AVS_STRING,SCE_AVS_TRIPLESTRING,0,0), 63131, L"String", L"fore:#7F007F", L"" },
                     { SCE_AVS_NUMBER, 63130, L"Number", L"fore:#007F7F", L"" },
                     { SCE_AVS_KEYWORD, 63128, L"Keyword", L"fore:#00007F; bold", L"" },
                     { SCE_AVS_FILTER, 63333, L"Filter", L"fore:#00007F; bold", L"" },
                     { SCE_AVS_PLUGIN, 63334, L"Plugin", L"fore:#0080C0; bold", L"" },
                     { SCE_AVS_FUNCTION, 63277, L"Function", L"fore:#007F7F", L"" },
                     { SCE_AVS_CLIPPROP, 63335, L"Clip property", L"fore:#00007F", L"" },
                     //{ SCE_AVS_USERDFN, 63106, L"User Defined", L"fore:#8000FF", L"" },
                     { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_MARKDOWN = {
"", "", "", "", "", "", "", "", "" };


EDITLEXER lexMARKDOWN = { SCLEX_MARKDOWN, 63336, L"Markdown", L"md; markdown; mdown; mkdn; mkd", L"", &KeyWords_MARKDOWN, {
                          { SCE_MARKDOWN_DEFAULT, 63126, L"Default", L"", L"" },
                          { SCE_MARKDOWN_LINE_BEGIN, 63338, L"Line Begin", L"", L"" },
                          { MULTI_STYLE(SCE_MARKDOWN_STRONG1,SCE_MARKDOWN_STRONG2,0,0), 63339, L"Strong", L"bold", L"" },
                          { MULTI_STYLE(SCE_MARKDOWN_EM1,SCE_MARKDOWN_EM2,0,0), 63340, L"Emphasis", L"italic", L"" },
                          { SCE_MARKDOWN_HEADER1, 63341, L"Header 1", L"fore:#FF0088; bold", L"" },
                          { SCE_MARKDOWN_HEADER2, 63342, L"Header 2", L"fore:#FF0088; bold", L"" },
                          { SCE_MARKDOWN_HEADER3, 63343, L"Header 3", L"fore:#FF0088; bold", L"" },
                          { SCE_MARKDOWN_HEADER4, 63344, L"Header 4", L"fore:#FF0088; bold", L"" },
                          { SCE_MARKDOWN_HEADER5, 63345, L"Header 5", L"fore:#FF0088; bold", L"" },
                          { SCE_MARKDOWN_HEADER6, 63346, L"Header 6", L"fore:#FF0088; bold", L"" },
                          { SCE_MARKDOWN_PRECHAR, 63347, L"Pre char", L"fore:#00007F", L"" },
                          { SCE_MARKDOWN_ULIST_ITEM, 63348, L"Unordered list", L"fore:#0080FF; bold", L"" },
                          { SCE_MARKDOWN_OLIST_ITEM, 63349, L"Ordered list", L"fore:#0080FF; bold", L"" },
                          { SCE_MARKDOWN_BLOCKQUOTE, 63350, L"Block quote", L"fore:#00007F", L"" },
                          { SCE_MARKDOWN_STRIKEOUT, 63351, L"Strikeout", L"", L"" },
                          { SCE_MARKDOWN_HRULE, 63352, L"Horizontal rule", L"bold", L"" },
                          { SCE_MARKDOWN_LINK, 63353, L"Link", L"fore:#0000FF", L"" },
                          { MULTI_STYLE(SCE_MARKDOWN_CODE,SCE_MARKDOWN_CODE2,SCE_MARKDOWN_CODEBK,0), 63354, L"Code", L"fore:#00007F; back:#EBEBEB", L"" },
                          { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_YAML = {
"y n yes no on off true false", "", "", "", "", "", "", "", "" };

EDITLEXER lexYAML = { SCLEX_YAML, 63355, L"YAML", L"yaml; yml", L"", &KeyWords_YAML, {
                      { SCE_YAML_DEFAULT, 63126, L"Default", L"", L"" },
                      { SCE_YAML_COMMENT, 63127, L"Comment", L"fore:#008800", L"" },
                      { SCE_YAML_IDENTIFIER, 63129, L"Identifier", L"bold; fore:#0A246A", L"" },
                      { SCE_YAML_KEYWORD, 63128, L"Keyword", L"fore:#880088", L"" },
                      { SCE_YAML_NUMBER, 63130, L"Number", L"fore:#FF8000", L"" },
                      { SCE_YAML_REFERENCE, 63356, L"Reference", L"fore:#008888", L"" },
                      { SCE_YAML_DOCUMENT, 63357, L"Document", L"fore:#FFFFFF; bold; back:#000088; eolfilled", L"" },
                      { SCE_YAML_TEXT, 63358, L"Text", L"fore:#404040", L"" },
                      { SCE_YAML_ERROR, 63359, L"Error", L"fore:#FFFFFF; bold; italic; back:#FF0000; eolfilled", L"" },
                      { SCE_YAML_OPERATOR, 63132, L"Operator", L"fore:#333366", L"" },
                      { -1, 00000, L"", L"", L"" } } };

KEYWORDLIST KeyWords_VHDL = {
"access after alias all architecture array assert attribute begin block body buffer bus case component configuration "
"constant disconnect downto else elsif end entity exit file for function generate generic group guarded if impure in "
"inertial inout is label library linkage literal loop map new next null of on open others out package port postponed "
"procedure process pure range record register reject report return select severity shared signal subtype then "
"to transport type unaffected units until use variable wait when while with",
"abs and mod nand nor not or rem rol ror sla sll sra srl xnor xor",
"left right low high ascending image value pos val succ pred leftof rightof base range reverse_range length delayed stable "
"quiet transaction event active last_event last_active last_value driving driving_value simple_name path_name instance_name",
"now readline read writeline write endfile resolved to_bit to_bitvector to_stdulogic to_stdlogicvector to_stdulogicvector "
"to_x01 to_x01z to_UX01 rising_edge falling_edge is_x shift_left shift_right rotate_left rotate_right resize to_integer "
"to_unsigned to_signed std_match to_01",
"std ieee work standard textio std_logic_1164 std_logic_arith std_logic_misc std_logic_signed std_logic_textio std_logic_unsigned "
"numeric_bit numeric_std math_complex math_real vital_primitives vital_timing",
"boolean bit character severity_level integer real time delay_length natural positive string bit_vector file_open_kind "
"file_open_status line text side width std_ulogic std_ulogic_vector std_logic std_logic_vector X01 X01Z UX01 UX01Z unsigned signed",
"", "", "" };

EDITLEXER lexVHDL = { SCLEX_VHDL, 63370, L"VHDL", L"vhdl; vhd", L"", &KeyWords_VHDL, {
                       { SCE_VHDL_DEFAULT, 63126, L"Default", L"", L"" },
                       { MULTI_STYLE(SCE_VHDL_COMMENTLINEBANG, SCE_VHDL_COMMENT, SCE_VHDL_BLOCK_COMMENT, 0), 63127, L"Comment", L"fore:#008800", L"" },
                       { SCE_VHDL_NUMBER, 63130, L"Number", L"fore:#FF0000", L"" },
                       { MULTI_STYLE(SCE_VHDL_STRING, SCE_VHDL_STRINGEOL, 0, 0), 63131, L"String", L"fore:#008000", L"" },
                       { SCE_VHDL_OPERATOR, 63132, L"Operator", L"fore:#B000B0", L"" },
                       { SCE_VHDL_IDENTIFIER, 63129, L"Identifier", L"", L"" },
                       { SCE_VHDL_KEYWORD, 63128, L"Keyword", L"bold; fore:#0A246A", L"" },
                       { SCE_VHDL_STDOPERATOR, 63371, L"Standard operator", L"bold; fore:#0A246A", L"" },
                       { SCE_VHDL_ATTRIBUTE, 63372, L"Attribute", L"", L"" },
                       { SCE_VHDL_STDFUNCTION, 63373, L"Standard function", L"", L"" },
                       { SCE_VHDL_STDPACKAGE, 63374, L"Standard package", L"", L"" },
                       { SCE_VHDL_STDTYPE, 63375, L"Standard type", L"fore:#FF8000", L"" },
                       { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_Registry = {
"", "", "", "", "", "", "", "", "" };

EDITLEXER lexRegistry = { SCLEX_REGISTRY, 63370, L"Windows Registry Files", L"reg", L"", &KeyWords_Registry, {
                       { SCE_REG_DEFAULT, 63126, L"Default", L"", L"" },
                       { SCE_REG_COMMENT, 63127, L"Comment", L"fore:#008800", L"" },
                       { SCE_REG_VALUENAME, 63376, L"Value Name", L"", L"" },
                       { MULTI_STYLE(SCE_REG_STRING,SCE_REG_STRING_GUID,0,0), 63131, L"String", L"fore:#008000", L"" },
                       { SCE_REG_VALUETYPE, 63377, L"Value Type", L"bold; fore:#00007F", L"" },
                       { SCE_REG_HEXDIGIT, 63378, L"Hex", L"fore:#7F0B0C", L"" },
                       { SCE_REG_ADDEDKEY, 63379, L"Added Key", L"fore:#000000; back:#FF8040; bold; eolfilled", L"" }, //fore:#530155
                       { SCE_REG_DELETEDKEY, 63380, L"Deleted Key", L"fore:#FF0000", L"" },
                       { SCE_REG_ESCAPED, 63381, L"Escaped", L"bold; fore:#7D8187", L"" },
                       { SCE_REG_KEYPATH_GUID, 63382, L"GUID in Key Path", L"fore:#7B5F15", L"" },
                       { SCE_REG_PARAMETER, 63294, L"Parameter", L"fore:#0B6561", L"" },
                       { SCE_REG_OPERATOR, 63132, L"Operator", L"bold", L"" },
                       { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_COFFEESCRIPT = {
"", "", "", "", "", "", "", "", "" };


EDITLEXER lexCOFFEESCRIPT = { SCLEX_COFFEESCRIPT, 63362, L"Coffeescript", L"coffee; Cakefile", L"", &KeyWords_COFFEESCRIPT, {
                       { SCE_COFFEESCRIPT_DEFAULT, 63126, L"Default", L"", L"" },
                       { MULTI_STYLE(SCE_COFFEESCRIPT_COMMENT,SCE_COFFEESCRIPT_COMMENTLINE,SCE_COFFEESCRIPT_COMMENTDOC,SCE_COFFEESCRIPT_COMMENTBLOCK), 63127, L"Comment", L"fore:#646464", L"" },
                       { MULTI_STYLE(SCE_COFFEESCRIPT_STRING,SCE_COFFEESCRIPT_STRINGEOL,SCE_COFFEESCRIPT_STRINGRAW,0), 63131, L"String", L"fore:#008000", L"" },
                       { SCE_COFFEESCRIPT_PREPROCESSOR, 63133, L"Preprocessor", L"fore:#FF8000", L"" },
                       { SCE_COFFEESCRIPT_IDENTIFIER, 63129, L"Identifier", L"bold; fore:#0A246A", L"" },
                       { SCE_COFFEESCRIPT_OPERATOR, 63132, L"Operator", L"fore:#B000B0", L"" },
                       { SCE_COFFEESCRIPT_NUMBER, 63130, L"Number", L"fore:#FF0000", L"" },
                       //{ SCE_COFFEESCRIPT_CHARACTER, 63376, L"Character", L"", L"" },
                       { MULTI_STYLE(SCE_COFFEESCRIPT_REGEX,SCE_COFFEESCRIPT_VERBOSE_REGEX,SCE_COFFEESCRIPT_VERBOSE_REGEX_COMMENT,0), 63315, L"Regex", L"fore:#006633; back:#FFF1A8", L"" },
                       { SCE_COFFEESCRIPT_GLOBALCLASS, 63378, L"Global Class", L"", L"" },
                       //{ MULTI_STYLE(SCE_COFFEESCRIPT_COMMENTLINEDOC,SCE_COFFEESCRIPT_COMMENTDOCKEYWORD,SCE_COFFEESCRIPT_COMMENTDOCKEYWORDERROR,0), 63379, L"Comment line", L"fore:#646464", L"" },
                       { MULTI_STYLE(SCE_COFFEESCRIPT_WORD,SCE_COFFEESCRIPT_WORD2,0,0), 63380, L"Word", L"", L"" },
                       { MULTI_STYLE(SCE_COFFEESCRIPT_VERBATIM,SCE_COFFEESCRIPT_TRIPLEVERBATIM,0,0), 63381, L"Verbatim", L"", L"" },
                       { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_MATLAB = {
"break case catch continue else elseif end for function global if otherwise "
"persistent return switch try while",
"", "", "", "", "", "", "", "" };


EDITLEXER lexMATLAB = { SCLEX_MATLAB, 63360, L"MATLAB", L"matlab", L"", &KeyWords_MATLAB, {
                        { SCE_MATLAB_DEFAULT, 63126, L"Default", L"", L"" },
                        { SCE_MATLAB_COMMENT, 63127, L"Comment", L"fore:#008000", L"" },
                        { SCE_MATLAB_COMMAND, 63236, L"Command", L"bold", L"" },
                        { SCE_MATLAB_NUMBER, 63130, L"Number", L"fore:#FF8000", L"" },
                        { SCE_MATLAB_KEYWORD, 63128, L"Keyword", L"fore:#00007F; bold", L"" },
                        { MULTI_STYLE(SCE_MATLAB_STRING,SCE_MATLAB_DOUBLEQUOTESTRING,0,0), 63131, L"String", L"fore:#7F007F", L"" },
                        { SCE_MATLAB_OPERATOR, 63132, L"Operator", L"", L"" },
                        { SCE_MATLAB_IDENTIFIER, 63129, L"Identifier", L"", L"" },
                        { -1, 00000, L"", L"", L"" } } };



KEYWORDLIST KeyWords_D = {
  // Primary keywords and identifiers
  "abstract alias align asm assert auto body break case cast catch class const continue "
  "debug default delegate delete deprecated do else enum export extern final finally for foreach foreach_reverse function "
  "goto if import in inout interface invariant is lazy mixin module new out override "
  "package pragma private protected public return scope static struct super switch synchronized "
  "template this throw try typedef typeid typeof union unittest version volatile while with",
  // Secondary keywords and identifiers
  "false null true",
  // Documentation comment keywords  (doxygen)
  "a addindex addtogroup anchor arg attention author b brief bug c class code date def defgroup deprecated dontinclude "
  "e em endcode endhtmlonly endif endlatexonly endlink endverbatim enum example exception f$ f[f] file fn hideinitializer htmlinclude htmlonly "
  "if image include ingroup internal invariant interface latexonly li line link mainpage name namespace nosubgrouping note overload "
  "p page par param post pre ref relates remarks return retval sa section see showinitializer since skip skipline struct subsection "
  "test throw todo typedef union until var verbatim verbinclude version warning weakgroup",
  // Type definitions and aliases
  "bool byte cdouble cent cfloat char creal dchar double float idouble ifloat int ireal long real short ubyte ucent uint ulong ushort void wchar",
  // Keywords 5
  "",
  // Keywords 6
  "",
  // Keywords 7
  "",
  // ---
  ""
};


EDITLEXER lexD = { SCLEX_D, 63022, L"D Source Code", L"d; dd; di", L"", &KeyWords_D, {
                   { SCE_D_DEFAULT, 63126, L"Default", L"", L"" },
                   { MULTI_STYLE(SCE_D_COMMENT,SCE_D_COMMENTLINE,SCE_D_COMMENTNESTED,0), 63127, L"Comment", L"fore:#008000", L"" },
                   { SCE_D_COMMENTDOC, 63259, L"Comment Doc", L"fore:#040A0", L"" },
                   { SCE_D_NUMBER, 63130, L"Number", L"fore:#FF0000", L"" },
                   { SCE_D_WORD, 63128, L"Keyword", L"bold; fore:#0A246A", L"" },
                   { SCE_D_WORD2, 63260, L"Keyword 2nd", L"bold; fore:#0A246A", L"" },
                   //{ SCE_D_WORD3, 63128, L"Keyword 3", L"bold; fore:#0A246A", L"" },
                   //{ SCE_D_WORD5, 63128, L"Keyword 5", L"bold; fore:#0A246A", L"" },
                   //{ SCE_D_WORD6, 63128, L"Keyword 6", L"bold; fore:#0A246A", L"" },
                   //{ SCE_D_WORD7, 63128, L"Keyword 7", L"bold; fore:#0A246A", L"" },
                   { SCE_D_TYPEDEF, 63258, L"Typedef", L"italic; fore:#0A246A", L"" },
                   { MULTI_STYLE(SCE_D_STRING,SCE_D_CHARACTER,SCE_D_STRINGEOL,0), 63131, L"String", L"italic; fore:#3C6CDD", L"" },
                   { SCE_D_OPERATOR, 63132, L"Operator", L"fore:#B000B0", L"" },
                   { SCE_D_IDENTIFIER, 63129, L"Identifier", L"", L"" },
                   //{ SCE_D_COMMENTLINEDOC, L"Default", L"", L"" },
                   //{ SCE_D_COMMENTDOCKEYWORD, L"Default", L"", L"" },
                   //{ SCE_D_STRINGB, L"Default", L"", L"" },
                   //{ SCE_D_STRINGR, L"Default", L"", L"" },
                   { -1, 00000, L"", L"", L"" } } };


KEYWORDLIST KeyWords_Go = {
  // Primary keywords and identifiers
  "break default func interface select case defer go map struct chan else goto package switch const fallthrough if range type "
  "continue for import return var",
  // Secondary keywords and identifiers
  "nil true false",
  // Documentation comment keywords  (doxygen)
  "",
  // Type definitions and aliases
  "bool int int8 int16 int32 int64 byte uint uint8 uint16 uint32 uint64 uintptr float float32 float64 string",
  // Keywords 5
  "",
  // Keywords 6
  "",
  // Keywords 7
  "",
  // ---
  ""
};


EDITLEXER lexGo = { SCLEX_D, 63023, L"Go Source Code", L"go", L"", &KeyWords_Go,{
                    { SCE_D_DEFAULT, 63126, L"Default", L"", L"" },
                    { MULTI_STYLE(SCE_D_COMMENT,SCE_D_COMMENTLINE,SCE_D_COMMENTNESTED,0), 63127, L"Comment", L"fore:#008000", L"" },
                    //{ SCE_D_COMMENTDOC, 63259, L"Comment Doc", L"fore:#040A0", L"" },
                    { SCE_D_NUMBER, 63130, L"Number", L"fore:#FF0000", L"" },
                    { SCE_D_WORD, 63128, L"Keyword", L"bold; fore:#0A246A", L"" },
                    { SCE_D_WORD2, 63260, L"Keyword 2nd", L"bold; fore:#0A246A", L"" },
                    //{ SCE_D_WORD3, 63128, L"Keyword 3", L"bold; fore:#0A246A", L"" },
                    //{ SCE_D_WORD5, 63128, L"Keyword 5", L"bold; fore:#0A246A", L"" },
                    //{ SCE_D_WORD6, 63128, L"Keyword 6", L"bold; fore:#0A246A", L"" },
                    //{ SCE_D_WORD7, 63128, L"Keyword 7", L"bold; fore:#0A246A", L"" },
                    { SCE_D_TYPEDEF, 63258, L"Typedef", L"italic; fore:#0A246A", L"" },
                    { MULTI_STYLE(SCE_D_STRING,SCE_D_CHARACTER,SCE_D_STRINGEOL,0), 63131, L"String", L"italic; fore:#3C6CDD", L"" },
                    { SCE_D_OPERATOR, 63132, L"Operator", L"fore:#B000B0", L"" },
                    { SCE_D_IDENTIFIER, 63129, L"Identifier", L"", L"" },
                    //{ SCE_D_COMMENTLINEDOC, L"Default", L"", L"" },
                    //{ SCE_D_COMMENTDOCKEYWORD, L"Default", L"", L"" },
                    //{ SCE_D_STRINGB, L"Default", L"", L"" },
                    //{ SCE_D_STRINGR, L"Default", L"", L"" },
                    //C++: { MULTI_STYLE(SCE_C_PREPROCESSOR,SCE_C_PREPROCESSORCOMMENT,SCE_C_PREPROCESSORCOMMENTDOC,0), 63133, L"Preprocessor", L"fore:#FF8000", L"" },
                    { -1, 00000, L"", L"", L"" } } };



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

  ""
};


EDITLEXER lexAwk = { SCLEX_PYTHON,  63024, L"Awk Script", L"awk", L"", &KeyWords_Awk,{
  { SCE_P_DEFAULT, 63126, L"Default", L"", L"" },
  { SCE_P_WORD, 63128, L"Keyword", L"bold; fore:#0000A0", L"" },
  { SCE_P_WORD2, 63260, L"Keyword 2nd", L"bold; italic; fore:#6666FF", L"" },
  { SCE_P_IDENTIFIER, 63129, L"Identifier", L"", L"" },
  { MULTI_STYLE(SCE_P_COMMENTLINE,SCE_P_COMMENTBLOCK,0,0), 63127, L"Comment", L"fore:#808080", L"" },
  { MULTI_STYLE(SCE_P_STRING,SCE_P_STRINGEOL,SCE_P_CHARACTER,0), 63131, L"String", L"fore:#008000", L"" },
  { SCE_P_NUMBER, 63130, L"Number", L"fore:#C04000", L"" },
  { SCE_P_OPERATOR, 63132, L"Operator", L"fore:#B000B0", L"" },
  { -1, 00000, L"", L"", L"" } } };


// This array holds all the lexers...
// Don't forget to change the number of the lexer for HTML and XML
// in Notepad2.c ParseCommandLine() if you change this array!
PEDITLEXER pLexArray[NUMLEXERS] =
{
  &lexDefault,
  &lexANSI,
  &lexAHK,
  &lexASM,
  &lexAU3,
  &lexAVS,
  &lexAwk,
  &lexBASH,
  &lexBAT,
  &lexCOFFEESCRIPT,
  &lexCONF, //Apache Config Scripts
  &lexCPP,
  &lexCS,
  &lexCSS,
  &lexCmake,
  &lexD,
  &lexDIFF,
  &lexGo,
  &lexHTML,
  &lexINNO,
  &lexJAVA,
  &lexJS,
  &lexJSON,
  &lexLATEX,
  &lexLUA,
  &lexMAK,
  &lexMARKDOWN,
  &lexMATLAB,
  &lexNSIS,
  &lexPAS,
  &lexPL,
  &lexPROPS,
  &lexPS,
  &lexPY,
  &lexRC,
  &lexRUBY,
  &lexRegistry,
  &lexSQL,
  &lexTCL,
  &lexVB,
  &lexVBS,
  &lexVHDL,
  &lexXML,
  &lexYAML
};


// Currently used lexer
PEDITLEXER pLexCurrent = &lexDefault;
COLORREF crCustom[16];
BOOL bUse2ndDefaultStyle;
BOOL fStylesModified = FALSE;
BOOL fWarnedNoIniFile = FALSE;
int iDefaultLexer;
BOOL bAutoSelect;
int cxStyleSelectDlg;
int cyStyleSelectDlg;

extern int  iDefaultCharSet;
extern BOOL bHiliteCurrentLine;
extern BOOL bHyperlinkHotspot;
extern BOOL bShowSelectionMargin;




//=============================================================================
//
//  Style_RgbAlpha()
//
int __fastcall Style_RgbAlpha(int rgbFore, int rgbBack, int alpha)
{
  return (int)RGB(\
    (0xFF - alpha) * (int)GetRValue(rgbBack) / 0xFF + alpha * (int)GetRValue(rgbFore) / 0xFF, \
    (0xFF - alpha) * (int)GetGValue(rgbBack) / 0xFF + alpha * (int)GetGValue(rgbFore) / 0xFF, \
    (0xFF - alpha) * (int)GetBValue(rgbBack) / 0xFF + alpha * (int)GetBValue(rgbFore) / 0xFF);
}


//=============================================================================
//
//  Style_Load()
//
void Style_Load()
{
  int i,iLexer;
  WCHAR tch[32] = { L'\0' };;
  WCHAR *pIniSection = LocalAlloc(LPTR,sizeof(WCHAR)*32*1024);
  int   cchIniSection = (int)LocalSize(pIniSection)/sizeof(WCHAR);

  // Custom colors
  crCustom [0] = RGB(0x00,0x00,0x00);
  crCustom [1] = RGB(0x0A,0x24,0x6A);
  crCustom [2] = RGB(0x3A,0x6E,0xA5);
  crCustom [3] = RGB(0x00,0x3C,0xE6);
  crCustom [4] = RGB(0x00,0x66,0x33);
  crCustom [5] = RGB(0x60,0x80,0x20);
  crCustom [6] = RGB(0x64,0x80,0x00);
  crCustom [7] = RGB(0xA4,0x60,0x00);
  crCustom [8] = RGB(0xFF,0xFF,0xFF);
  crCustom [9] = RGB(0xFF,0xFF,0xE2);
  crCustom[10] = RGB(0xFF,0xF1,0xA8);
  crCustom[11] = RGB(0xFF,0xC0,0x00);
  crCustom[12] = RGB(0xFF,0x40,0x00);
  crCustom[13] = RGB(0xC8,0x00,0x00);
  crCustom[14] = RGB(0xB0,0x00,0xB0);
  crCustom[15] = RGB(0xB2,0x8B,0x40);

  LoadIniSection(L"Custom Colors",pIniSection,cchIniSection);
  for (i = 0; i < 16; i++) {
    WCHAR wch[32] = { L'\0' };
    StringCchPrintf(tch,COUNTOF(tch),L"%02i",i+1);
    if (IniSectionGetString(pIniSection,tch,L"",wch,COUNTOF(wch))) {
      if (wch[0] == L'#') {
        int irgb;
        int itok = swscanf_s(CharNext(wch),L"%x",&irgb);
        if (itok == 1)
          crCustom[i] = RGB((irgb&0xFF0000) >> 16,(irgb&0xFF00) >> 8,irgb&0xFF);
      }
    }
  }

  LoadIniSection(L"Styles",pIniSection,cchIniSection);
  // 2nd default
  bUse2ndDefaultStyle = (IniSectionGetInt(pIniSection,L"Use2ndDefaultStyle",0)) ? TRUE : FALSE;

  // default scheme
  iDefaultLexer = IniSectionGetInt(pIniSection,L"DefaultScheme",0);
  iDefaultLexer = min(max(iDefaultLexer,0),COUNTOF(pLexArray)-1);

  // auto select
  bAutoSelect = (IniSectionGetInt(pIniSection,L"AutoSelect",1)) ? 1 : 0;

  // scheme select dlg dimensions
  cxStyleSelectDlg = IniSectionGetInt(pIniSection,L"SelectDlgSizeX",304);
  cxStyleSelectDlg = max(cxStyleSelectDlg,0);

  cyStyleSelectDlg = IniSectionGetInt(pIniSection,L"SelectDlgSizeY",0);
  cyStyleSelectDlg = max(cyStyleSelectDlg,324);

  for (iLexer = 0; iLexer < COUNTOF(pLexArray); iLexer++) {
    LoadIniSection(pLexArray[iLexer]->pszName,pIniSection,cchIniSection);
    IniSectionGetString(pIniSection, L"FileNameExtensions", pLexArray[iLexer]->pszDefExt,
      pLexArray[iLexer]->szExtensions, COUNTOF(pLexArray[iLexer]->szExtensions));
    i = 0;
    while (pLexArray[iLexer]->Styles[i].iStyle != -1) {
      IniSectionGetString(pIniSection,pLexArray[iLexer]->Styles[i].pszName,
        pLexArray[iLexer]->Styles[i].pszDefault,
        pLexArray[iLexer]->Styles[i].szValue,
        COUNTOF(pLexArray[iLexer]->Styles[i].szValue));
      i++;
    }
  }
  LocalFree(pIniSection);
}


//=============================================================================
//
//  Style_Save()
//
void Style_Save()
{
  int i,iLexer;
  WCHAR tch[32] = { L'\0' };;
  WCHAR *pIniSection = LocalAlloc(LPTR,sizeof(WCHAR)*32*1024);
  //int   cchIniSection = (int)LocalSize(pIniSection)/sizeof(WCHAR);

  // Custom colors
  for (i = 0; i < 16; i++) {
    WCHAR wch[32] = { L'\0' };
    StringCchPrintf(tch,COUNTOF(tch),L"%02i",i+1);
    StringCchPrintf(wch,COUNTOF(wch),L"#%02X%02X%02X",
      (int)GetRValue(crCustom[i]),(int)GetGValue(crCustom[i]),(int)GetBValue(crCustom[i]));
    IniSectionSetString(pIniSection,tch,wch);
  }
  SaveIniSection(L"Custom Colors",pIniSection);
  ZeroMemory(pIniSection,LocalSize(pIniSection));

  // auto select
  IniSectionSetInt(pIniSection,L"Use2ndDefaultStyle",bUse2ndDefaultStyle);

  // default scheme
  IniSectionSetInt(pIniSection,L"DefaultScheme",iDefaultLexer);

  // auto select
  IniSectionSetInt(pIniSection,L"AutoSelect",bAutoSelect);

  // scheme select dlg dimensions
  IniSectionSetInt(pIniSection,L"SelectDlgSizeX",cxStyleSelectDlg);
  IniSectionSetInt(pIniSection,L"SelectDlgSizeY",cyStyleSelectDlg);

  SaveIniSection(L"Styles",pIniSection);
  ZeroMemory(pIniSection,LocalSize(pIniSection));

  if (!fStylesModified) {
    LocalFree(pIniSection);
    return;
  }
  
  for (iLexer = 0; iLexer < COUNTOF(pLexArray); iLexer++) {
    IniSectionSetString(pIniSection,L"FileNameExtensions",pLexArray[iLexer]->szExtensions);
    i = 0;
    while (pLexArray[iLexer]->Styles[i].iStyle != -1) {
      IniSectionSetString(pIniSection,pLexArray[iLexer]->Styles[i].pszName,pLexArray[iLexer]->Styles[i].szValue);
      i++;
    }

    SaveIniSection(pLexArray[iLexer]->pszName,pIniSection);
    ZeroMemory(pIniSection,LocalSize(pIniSection));
  }
  LocalFree(pIniSection);
}


//=============================================================================
//
//  Style_Import()
//
BOOL Style_Import(HWND hwnd)
{
  WCHAR szFile[MAX_PATH * 2] = { L'\0' };
  WCHAR szFilter[256] = { L'\0' };
  OPENFILENAME ofn;

  ZeroMemory(&ofn,sizeof(OPENFILENAME));
  GetString(IDS_FILTER_INI,szFilter,COUNTOF(szFilter));
  PrepareFilterStr(szFilter);

  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFilter = szFilter;
  ofn.lpstrFile = szFile;
  ofn.lpstrDefExt = L"ini";
  ofn.nMaxFile = COUNTOF(szFile);
  ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT
            | OFN_PATHMUSTEXIST | OFN_SHAREAWARE /*| OFN_NODEREFERENCELINKS*/;

  if (GetOpenFileName(&ofn)) {

    int i,iLexer;
    WCHAR *pIniSection = LocalAlloc(LPTR,sizeof(WCHAR)*32*1024);
    int   cchIniSection = (int)LocalSize(pIniSection)/sizeof(WCHAR);

    for (iLexer = 0; iLexer < COUNTOF(pLexArray); iLexer++) {
      if (GetPrivateProfileSection(pLexArray[iLexer]->pszName,pIniSection,cchIniSection,szFile)) {
        IniSectionGetString(pIniSection, L"FileNameExtensions", pLexArray[iLexer]->pszDefExt,
          pLexArray[iLexer]->szExtensions, COUNTOF(pLexArray[iLexer]->szExtensions));
        i = 0;
        while (pLexArray[iLexer]->Styles[i].iStyle != -1) {
          IniSectionGetString(pIniSection,pLexArray[iLexer]->Styles[i].pszName,
            pLexArray[iLexer]->Styles[i].pszDefault,
            pLexArray[iLexer]->Styles[i].szValue,
            COUNTOF(pLexArray[iLexer]->Styles[i].szValue));
          i++;
        }
      }
    }
    LocalFree(pIniSection);
    return(TRUE);
  }
  return(FALSE);
}

//=============================================================================
//
//  Style_Export()
//
BOOL Style_Export(HWND hwnd)
{
  WCHAR szFile[MAX_PATH * 2] = { L'\0' };
  WCHAR szFilter[256] = { L'\0' };
  OPENFILENAME ofn;
  DWORD dwError = ERROR_SUCCESS;

  ZeroMemory(&ofn,sizeof(OPENFILENAME));
  GetString(IDS_FILTER_INI,szFilter,COUNTOF(szFilter));
  PrepareFilterStr(szFilter);

  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFilter = szFilter;
  ofn.lpstrFile = szFile;
  ofn.lpstrDefExt = L"ini";
  ofn.nMaxFile = COUNTOF(szFile);
  ofn.Flags = /*OFN_FILEMUSTEXIST |*/ OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT
            | OFN_PATHMUSTEXIST | OFN_SHAREAWARE /*| OFN_NODEREFERENCELINKS*/ | OFN_OVERWRITEPROMPT;

  if (GetSaveFileName(&ofn)) {

    WCHAR *pIniSection = LocalAlloc(LPTR,sizeof(WCHAR)*32*1024);
    //int   cchIniSection = (int)LocalSize(pIniSection)/sizeof(WCHAR);

    for (int iLexer = 0; iLexer < COUNTOF(pLexArray); iLexer++) {
      IniSectionSetString(pIniSection,L"FileNameExtensions",pLexArray[iLexer]->szExtensions);
      int i = 0;
      while (pLexArray[iLexer]->Styles[i].iStyle != -1) {
        IniSectionSetString(pIniSection,pLexArray[iLexer]->Styles[i].pszName,pLexArray[iLexer]->Styles[i].szValue);
        i++;
      }
      if (!WritePrivateProfileSection(pLexArray[iLexer]->pszName,pIniSection,szFile))
        dwError = GetLastError();
      ZeroMemory(pIniSection,LocalSize(pIniSection));
    }
    LocalFree(pIniSection);

    if (dwError != ERROR_SUCCESS) {
      MsgBox(MBINFO,IDS_EXPORT_FAIL,szFile);
    }
    return(TRUE);
  }
  return(FALSE);
}


//=============================================================================
//
//  Style_SetLexer()
//
void Style_SetLexer(HWND hwnd, PEDITLEXER pLexNew) {
  int rgb;
  int iValue;
  int iIdx;
  WCHAR wchFontName[64] = { '\0' };
  WCHAR wchCaretStyle[64] = { L'\0' };

  // Select default if NULL is specified
  if (!pLexNew)
    pLexNew = pLexArray[iDefaultLexer];

  // Lexer
  SendMessage(hwnd, SCI_SETLEXER, pLexNew->iLexer, 0);

  // deprecated since SCI 3.4.2
  //int iStyleBits = (int)SendMessage(hwnd,SCI_GETSTYLEBITSNEEDED,0,0);
  //SendMessage(hwnd,SCI_SETSTYLEBITS,(WPARAM)iStyleBits,0);

  if (pLexNew->iLexer == SCLEX_XML)
    SendMessage(hwnd, SCI_SETPROPERTY, (WPARAM)"lexer.xml.allow.scripts", (LPARAM)"1");
  if (pLexNew->iLexer == SCLEX_CPP) {
    SendMessage(hwnd, SCI_SETPROPERTY, (WPARAM)"styling.within.preprocessor", (LPARAM)"1");
    SendMessage(hwnd, SCI_SETPROPERTY, (WPARAM)"lexer.cpp.track.preprocessor", (LPARAM)"0");
    SendMessage(hwnd, SCI_SETPROPERTY, (WPARAM)"lexer.cpp.update.preprocessor", (LPARAM)"0");
  }
  else if (pLexNew->iLexer == SCLEX_PASCAL)
    SendMessage(hwnd, SCI_SETPROPERTY, (WPARAM)"lexer.pascal.smart.highlighting", (LPARAM)"1");
  else if (pLexNew->iLexer == SCLEX_SQL) {
    SendMessage(hwnd, SCI_SETPROPERTY, (WPARAM)"sql.backslash.escapes", (LPARAM)"1");
    SendMessage(hwnd, SCI_SETPROPERTY, (WPARAM)"lexer.sql.backticks.identifier", (LPARAM)"1");
    SendMessage(hwnd, SCI_SETPROPERTY, (WPARAM)"lexer.sql.numbersign.comment", (LPARAM)"1");
  }
  else if (pLexNew->iLexer == SCLEX_NSIS)
    SciCall_SetProperty("nsis.ignorecase", "1");
  else if (pLexNew->iLexer == SCLEX_CSS) {
    SciCall_SetProperty("lexer.css.scss.language", "1");
    SciCall_SetProperty("lexer.css.less.language", "1");
  }
  else if (pLexNew->iLexer == SCLEX_JSON) {
    SciCall_SetProperty("json.allow.comments", "1");
    SciCall_SetProperty("json.escape.sequence", "1");
  }

  // Code folding
  SciCall_SetProperty("fold", "1");
  SciCall_SetProperty("fold.compact", "0");
  SciCall_SetProperty("fold.comment", "1");
  SciCall_SetProperty("fold.html", "1");
  SciCall_SetProperty("fold.preprocessor", "1");
  SciCall_SetProperty("fold.cpp.comment.explicit", "0");

  // Add KeyWord Lists
  for (int i = 0; i < (KEYWORDSET_MAX + 1); i++)
    SendMessage(hwnd, SCI_SETKEYWORDS, i, (LPARAM)pLexNew->pKeyWords->pszKeyWords[i]);

  // Use 2nd default style
  iIdx = (bUse2ndDefaultStyle) ? STY_CNT_LAST : 0;
  int iDefaultStyle = lexDefault.Styles[STY_DEFAULT + iIdx].iStyle;
  WCHAR* wchDefaultStyleStrg = lexDefault.Styles[STY_DEFAULT + iIdx].szValue;

  // Clear
  SendMessage(hwnd, SCI_CLEARDOCUMENTSTYLE, 0, 0);

  // Idle Styling (very large text)
  //SendMessage(hwnd, SCI_SETIDLESTYLING, SC_IDLESTYLING_ALL, 0);  

  // Default Values are always set
  SendMessage(hwnd, SCI_STYLERESETDEFAULT, 0, 0);


  if (!Style_StrGetColor(TRUE, wchDefaultStyleStrg, &iValue))
    SendMessage(hwnd, SCI_STYLESETFORE, STYLE_DEFAULT, (LPARAM)GetSysColor(COLOR_WINDOWTEXT));   // default text color
  if (!Style_StrGetColor(FALSE, wchDefaultStyleStrg, &iValue))
    SendMessage(hwnd, SCI_STYLESETBACK, STYLE_DEFAULT, (LPARAM)GetSysColor(COLOR_WINDOW));       // default window color

  // Auto-select codepage according to charset
  //~Style_SetACPfromCharSet(hwnd);

  // ---- Font & More  ---
  Style_SetFontQuality(hwnd, wchDefaultStyleStrg);

  if (!Style_StrGetFont(wchDefaultStyleStrg, wchFontName, COUNTOF(wchFontName))) 
  {
    char chFontName[32] = { '\0' };
    if (IsFontAvailable(L"Consolas"))
      StringCchCopyA(chFontName, COUNTOF(chFontName), "Consolas");
    else
      StringCchCopyA(chFontName, COUNTOF(chFontName), "Lucida Console");

    SendMessage(hwnd, SCI_STYLESETFONT, iDefaultStyle, (LPARAM)chFontName);
  }
  iBaseFontSize = 10;
  if (!Style_StrGetSize(wchDefaultStyleStrg, &iBaseFontSize)) {
    SendMessage(hwnd, SCI_STYLESETSIZE, iDefaultStyle, (LPARAM)iBaseFontSize);  // base size
  }
  SendMessage(hwnd, SCI_STYLESETWEIGHT, iDefaultStyle, (LPARAM)FW_NORMAL);
  SendMessage(hwnd, SCI_STYLESETITALIC, iDefaultStyle, (LPARAM)FALSE);
  SendMessage(hwnd, SCI_STYLESETEOLFILLED, iDefaultStyle, (LPARAM)FALSE);
  SendMessage(hwnd, SCI_STYLESETCHARACTERSET, iDefaultStyle, (LPARAM)DEFAULT_CHARSET);

  // apply default style
  Style_SetStyles(hwnd, iDefaultStyle, wchDefaultStyleStrg);
  if (StringCchLenW(pLexNew->Styles[STY_DEFAULT].szValue, COUNTOF(pLexNew->Styles[STY_DEFAULT].szValue)) > 0) {
    // override with Lexer's specific defaults
    Style_SetStyles(hwnd, iDefaultStyle, pLexNew->Styles[STY_DEFAULT].szValue);
  }

  SendMessage(hwnd, SCI_STYLESETHOTSPOT, STYLE_DEFAULT, (LPARAM)FALSE);       // default hotspot off

  // Re-Set to just defined default style (STYLE_DEFAULT)
  SendMessage(hwnd, SCI_STYLECLEARALL, 0, 0);

  // --------------------------------------------------------------------------

  Style_SetStyles(hwnd, lexDefault.Styles[STY_MARGIN + iIdx].iStyle, lexDefault.Styles[STY_MARGIN + iIdx].szValue); // linenumber

  if (bUseOldStyleBraceMatching) {
    Style_SetStyles(hwnd, lexDefault.Styles[STY_BRACE_OK + iIdx].iStyle, lexDefault.Styles[STY_BRACE_OK + iIdx].szValue); // brace light
  }
  else {
    if (Style_StrGetColor(TRUE, lexDefault.Styles[STY_BRACE_OK + iIdx].szValue, &iValue))
      SendMessage(hwnd, SCI_INDICSETFORE, INDIC_NP3_MATCH_BRACE, iValue);
    if (Style_StrGetAlpha(lexDefault.Styles[STY_BRACE_OK + iIdx].szValue, &iValue, TRUE))
      SendMessage(hwnd, SCI_INDICSETALPHA, INDIC_NP3_MATCH_BRACE, iValue);
    if (Style_StrGetAlpha(lexDefault.Styles[STY_BRACE_OK + iIdx].szValue, &iValue, FALSE))
      SendMessage(hwnd, SCI_INDICSETOUTLINEALPHA, INDIC_NP3_MATCH_BRACE, iValue);
  }
  if (bUseOldStyleBraceMatching) {
    Style_SetStyles(hwnd, lexDefault.Styles[STY_BRACE_BAD + iIdx].iStyle, lexDefault.Styles[STY_BRACE_BAD + iIdx].szValue); // brace bad
  }
  else {
    if (Style_StrGetColor(TRUE, lexDefault.Styles[STY_BRACE_BAD + iIdx].szValue, &iValue))
      SendMessage(hwnd, SCI_INDICSETFORE, INDIC_NP3_BAD_BRACE, iValue);
    if (Style_StrGetAlpha(lexDefault.Styles[STY_BRACE_BAD + iIdx].szValue, &iValue, TRUE))
      SendMessage(hwnd, SCI_INDICSETALPHA, INDIC_NP3_BAD_BRACE, iValue);
    if (Style_StrGetAlpha(lexDefault.Styles[STY_BRACE_BAD + iIdx].szValue, &iValue, FALSE))
      SendMessage(hwnd, SCI_INDICSETOUTLINEALPHA, INDIC_NP3_BAD_BRACE, iValue);
  }

  // Occurrences Marker

  if (!Style_StrGetColor(TRUE, lexDefault.Styles[STY_MARK_OCC + iIdx].szValue, &iValue)) 
  {
    WCHAR* sty = L"";
    switch (iMarkOccurrences) {
    case 1:
      sty = L"fore:0xFF0000";
      iValue = RGB(0xFF, 0x00, 0x00);
      break;
    case 2:
      sty = L"fore:0x00FF00";
      iValue = RGB(0x00, 0xFF, 0x00);
      break;
    case 3:
    default:
      sty = L"fore:0x0000FF";
      iValue = RGB(0x00, 0xFF, 0x00);
      break;
    }
    StringCchCopyW(lexDefault.Styles[STY_MARK_OCC + iIdx].szValue, COUNTOF(lexDefault.Styles[0].szValue), sty);
  }
  SendMessage(hwnd, SCI_INDICSETFORE, INDIC_NP3_MARK_OCCURANCE, iValue);

  if (!Style_StrGetAlpha(lexDefault.Styles[STY_MARK_OCC + iIdx].szValue, &iValue, TRUE)) {
    iValue = 100;
    StringCchCatW(lexDefault.Styles[STY_MARK_OCC + iIdx].szValue, COUNTOF(lexDefault.Styles[0].szValue), L"; alpha:100");
  }   
  SendMessage(hwnd, SCI_INDICSETALPHA, INDIC_NP3_MARK_OCCURANCE, iValue);

  if (!Style_StrGetAlpha(lexDefault.Styles[STY_MARK_OCC + iIdx].szValue, &iValue, FALSE)) {
    iValue = 100;
    StringCchCatW(lexDefault.Styles[STY_MARK_OCC + iIdx].szValue, COUNTOF(lexDefault.Styles[0].szValue), L"; alpha2:100");
  }
  SendMessage(hwnd, SCI_INDICSETOUTLINEALPHA, INDIC_NP3_MARK_OCCURANCE, iValue);

  iValue = -1; // need for retrieval
  if (!Style_GetIndicatorType(lexDefault.Styles[STY_MARK_OCC + iIdx].szValue, COUNTOF(lexDefault.Styles[STY_MARK_OCC + iIdx].szValue), &iValue)) {
    // got default, get string
    StringCchCatW(lexDefault.Styles[STY_MARK_OCC + iIdx].szValue, COUNTOF(lexDefault.Styles[0].szValue), L"; ");
    Style_GetIndicatorType(wchCaretStyle, COUNTOF(wchCaretStyle), &iValue);
    StringCchCatW(lexDefault.Styles[STY_MARK_OCC + iIdx].szValue, COUNTOF(lexDefault.Styles[0].szValue), wchCaretStyle);
  }
  SendMessage(hwnd, SCI_INDICSETSTYLE, INDIC_NP3_MARK_OCCURANCE, iValue);


  // More default values...

  if (pLexNew != &lexANSI)
    Style_SetStyles(hwnd, lexDefault.Styles[STY_CTRL_CHR + iIdx].iStyle, lexDefault.Styles[STY_CTRL_CHR + iIdx].szValue); // control char

  Style_SetStyles(hwnd, lexDefault.Styles[STY_INDENT_GUIDE + iIdx].iStyle, lexDefault.Styles[STY_INDENT_GUIDE + iIdx].szValue); // indent guide

  if (Style_StrGetColor(TRUE, lexDefault.Styles[STY_SEL_TXT + iIdx].szValue, &rgb)) { // selection fore
    SendMessage(hwnd, SCI_SETSELFORE, TRUE, rgb);
    SendMessage(hwnd, SCI_SETADDITIONALSELFORE, rgb, 0);
  }
  else {
    SendMessage(hwnd, SCI_SETSELFORE, 0, 0);
    SendMessage(hwnd, SCI_SETADDITIONALSELFORE, 0, 0);
  }

  if (Style_StrGetColor(FALSE, lexDefault.Styles[STY_SEL_TXT + iIdx].szValue, &iValue)) { // selection back
    SendMessage(hwnd, SCI_SETSELBACK, TRUE, iValue);
    SendMessage(hwnd, SCI_SETADDITIONALSELBACK, iValue, 0);
  }
  else {
    SendMessage(hwnd, SCI_SETSELBACK, TRUE, RGB(0xC0, 0xC0, 0xC0)); // use a default value...
    SendMessage(hwnd, SCI_SETADDITIONALSELBACK, RGB(0xC0, 0xC0, 0xC0), 0);
  }

  if (Style_StrGetAlpha(lexDefault.Styles[STY_SEL_TXT + iIdx].szValue, &iValue, TRUE)) { // selection alpha
    SendMessage(hwnd, SCI_SETSELALPHA, iValue, 0);
    SendMessage(hwnd, SCI_SETADDITIONALSELALPHA, iValue, 0);
  }
  else {
    SendMessage(hwnd, SCI_SETSELALPHA, SC_ALPHA_NOALPHA, 0);
    SendMessage(hwnd, SCI_SETADDITIONALSELALPHA, SC_ALPHA_NOALPHA, 0);
  }

  if (StrStrI(lexDefault.Styles[STY_SEL_TXT + iIdx].szValue, L"eolfilled")) // selection eolfilled
    SendMessage(hwnd, SCI_SETSELEOLFILLED, 1, 0);
  else
    SendMessage(hwnd, SCI_SETSELEOLFILLED, 0, 0);

  if (Style_StrGetColor(TRUE, lexDefault.Styles[STY_WHITESPACE + iIdx].szValue, &rgb)) // whitespace fore
    SendMessage(hwnd, SCI_SETWHITESPACEFORE, TRUE, rgb);
  else
    SendMessage(hwnd, SCI_SETWHITESPACEFORE, 0, 0);

  if (Style_StrGetColor(FALSE, lexDefault.Styles[STY_WHITESPACE + iIdx].szValue, &rgb)) // whitespace back
    SendMessage(hwnd, SCI_SETWHITESPACEBACK, TRUE, rgb);
  else
    SendMessage(hwnd, SCI_SETWHITESPACEBACK, 0, 0);    // use a default value...

  // whitespace dot size
  iValue = 1;
  if (Style_StrGetSize(lexDefault.Styles[STY_WHITESPACE + iIdx].szValue, &iValue)) {

    WCHAR tch[32] = { L'\0' };
    WCHAR wchStyle[BUFSIZE_STYLE_VALUE];
    StringCchCopyN(wchStyle, COUNTOF(wchStyle), lexDefault.Styles[STY_WHITESPACE + iIdx].szValue, COUNTOF(lexDefault.Styles[STY_WHITESPACE + iIdx].szValue));

    iValue = max(min(iValue, 5), 0);
    StringCchPrintf(lexDefault.Styles[STY_WHITESPACE + iIdx].szValue, COUNTOF(lexDefault.Styles[STY_WHITESPACE + iIdx].szValue), L"size:%i", iValue);

    if (Style_StrGetColor(TRUE, wchStyle, &rgb)) {
      StringCchPrintf(tch, COUNTOF(tch), L"; fore:#%02X%02X%02X",
        (int)GetRValue(rgb),
        (int)GetGValue(rgb),
        (int)GetBValue(rgb));
      StringCchCat(lexDefault.Styles[STY_WHITESPACE + iIdx].szValue, COUNTOF(lexDefault.Styles[STY_WHITESPACE + iIdx].szValue), tch);
    }

    if (Style_StrGetColor(FALSE, wchStyle, &rgb)) {
      StringCchPrintf(tch, COUNTOF(tch), L"; back:#%02X%02X%02X",
        (int)GetRValue(rgb),
        (int)GetGValue(rgb),
        (int)GetBValue(rgb));
      StringCchCat(lexDefault.Styles[STY_WHITESPACE + iIdx].szValue, COUNTOF(lexDefault.Styles[STY_WHITESPACE + iIdx].szValue), tch);
    }
  }
  SendMessage(hwnd, SCI_SETWHITESPACESIZE, iValue, 0);

  // current line background
  Style_SetCurrentLineBackground(hwnd, bHiliteCurrentLine);

  // bookmark line or marker
  Style_SetCurrentMargin(hwnd, bShowSelectionMargin);


  // caret style and width
  if (StrStr(lexDefault.Styles[STY_CARET + iIdx].szValue,L"block")) {
    SendMessage(hwnd,SCI_SETCARETSTYLE,CARETSTYLE_BLOCK,0);
    StringCchCopy(wchCaretStyle,COUNTOF(wchCaretStyle),L"block");
  }
  else {
    WCHAR wch[32] = { L'\0' };
    iValue = 1;
    if (Style_StrGetSize(lexDefault.Styles[STY_CARET + iIdx].szValue,&iValue)) {
      iValue = max(min(iValue,3),1);
      StringCchPrintf(wch,COUNTOF(wch),L"size:%i",iValue);
      StringCchCat(wchCaretStyle,COUNTOF(wchCaretStyle),wch);
    }
    SendMessage(hwnd,SCI_SETCARETSTYLE,CARETSTYLE_LINE,0);
    SendMessage(hwnd,SCI_SETCARETWIDTH,iValue,0);
  }
  if (StrStr(lexDefault.Styles[STY_CARET + iIdx].szValue,L"noblink")) {
    SendMessage(hwnd,SCI_SETCARETPERIOD,(WPARAM)0,0);
    if (StringCchLenW(wchCaretStyle,COUNTOF(wchCaretStyle)))
      StringCchCat(wchCaretStyle,COUNTOF(wchCaretStyle),L"; ");
    StringCchCat(wchCaretStyle,COUNTOF(wchCaretStyle),L"noblink");
  }
  else
    SendMessage(hwnd,SCI_SETCARETPERIOD,(WPARAM)GetCaretBlinkTime(),0);

  // caret fore
  if (!Style_StrGetColor(TRUE,lexDefault.Styles[STY_CARET + iIdx].szValue,&rgb))
    rgb = GetSysColor(COLOR_WINDOWTEXT);
  else {
    WCHAR wch[32] = { L'\0' };
    StringCchPrintf(wch,COUNTOF(wch),L"fore:#%02X%02X%02X",
      (int)GetRValue(rgb),
      (int)GetGValue(rgb),
      (int)GetBValue(rgb));
    if (StringCchLenW(wchCaretStyle,COUNTOF(wchCaretStyle)))
      StringCchCat(wchCaretStyle,COUNTOF(wchCaretStyle),L"; ");
    StringCchCat(wchCaretStyle,COUNTOF(wchCaretStyle),wch);
  }
  if (!VerifyContrast(rgb,(COLORREF)SendMessage(hwnd,SCI_STYLEGETBACK,0,0)))
    rgb = (int)SendMessage(hwnd,SCI_STYLEGETFORE,0,0);
  SendMessage(hwnd,SCI_SETCARETFORE,rgb,0);
  SendMessage(hwnd,SCI_SETADDITIONALCARETFORE,rgb,0);
  StringCchCopy(lexDefault.Styles[STY_CARET + iIdx].szValue,COUNTOF(lexDefault.Styles[STY_CARET + iIdx].szValue),wchCaretStyle);

  if (SendMessage(hwnd,SCI_GETEDGEMODE,0,0) == EDGE_LINE) {
    if (Style_StrGetColor(TRUE,lexDefault.Styles[STY_LONG_LN_MRK + iIdx].szValue,&rgb)) // edge fore
      SendMessage(hwnd,SCI_SETEDGECOLOUR,rgb,0);
    else
      SendMessage(hwnd,SCI_SETEDGECOLOUR,GetSysColor(COLOR_3DLIGHT),0);
  }
  else {
    if (Style_StrGetColor(FALSE,lexDefault.Styles[STY_LONG_LN_MRK + iIdx].szValue,&rgb)) // edge back
      SendMessage(hwnd,SCI_SETEDGECOLOUR,rgb,0);
    else
      SendMessage(hwnd,SCI_SETEDGECOLOUR,GetSysColor(COLOR_3DLIGHT),0);
  }

  // Extra Line Spacing
  if (Style_StrGetSize(lexDefault.Styles[STY_X_LN_SPACE + iIdx].szValue,&iValue) && (pLexNew != &lexANSI)) {
    int iAscent = 0;
    int iDescent = 0;
    int iValAdj = min(max(iValue,0),64);
    if (iValAdj != iValue)
      StringCchPrintf(lexDefault.Styles[STY_X_LN_SPACE + iIdx].szValue, COUNTOF(lexDefault.Styles[STY_X_LN_SPACE + iIdx].szValue), L"size:%i", iValAdj);

    if ((iValAdj % 2) != 0) {
      iAscent++;
      iValAdj--;
    }
    iAscent += (iValAdj / 2);
    iDescent += (iValAdj / 2);
    SendMessage(hwnd,SCI_SETEXTRAASCENT,(WPARAM)iAscent,0);
    SendMessage(hwnd,SCI_SETEXTRADESCENT,(WPARAM)iDescent,0);
  }
  else {
    SendMessage(hwnd,SCI_SETEXTRAASCENT,0,0);
    SendMessage(hwnd,SCI_SETEXTRADESCENT,0,0);
    //StringCchPrintf(lexDefault.Styles[STY_X_LN_SPACE + iIdx].szValue, COUNTOF(lexDefault.Styles[STY_X_LN_SPACE + iIdx].szValue), L"size:%i", 0);
  }

  { // set folding style; braces are for scoping only
    static const int iMarkerIDs[] =
    {
      SC_MARKNUM_FOLDEROPEN,
      SC_MARKNUM_FOLDER,
      SC_MARKNUM_FOLDERSUB,
      SC_MARKNUM_FOLDERTAIL,
      SC_MARKNUM_FOLDEREND,
      SC_MARKNUM_FOLDEROPENMID,
      SC_MARKNUM_FOLDERMIDTAIL
    };

    COLORREF clrFore = SciCall_StyleGetFore(STYLE_DEFAULT);
    COLORREF clrBack = SciCall_StyleGetBack(STYLE_DEFAULT);

    SciCall_SetFoldMarginColour(TRUE, clrBack);
    SciCall_SetFoldMarginHiColour(TRUE, clrBack);

    // Set marker color to the average of clrFore and clrBack
    clrFore = (((clrFore & 0xFF0000) + (clrBack & 0xFF0000)) >> 1 & 0xFF0000) |
              (((clrFore & 0x00FF00) + (clrBack & 0x00FF00)) >> 1 & 0x00FF00) |
              (((clrFore & 0x0000FF) + (clrBack & 0x0000FF)) >> 1 & 0x0000FF);

    // Rounding hack for pure white against pure black
    if (clrFore == 0x7F7F7F) clrFore = 0x808080;

    for (int i = 0; i < COUNTOF(iMarkerIDs); ++i)
    {
      SciCall_MarkerSetBack(iMarkerIDs[i], clrFore);
      SciCall_MarkerSetFore(iMarkerIDs[i], clrBack);
    }
  } // end set folding style

  if (SendMessage(hwnd,SCI_GETINDENTATIONGUIDES,0,0) != SC_IV_NONE)
    Style_SetIndentGuides(hwnd,TRUE);


  if (pLexNew->iLexer != SCLEX_NULL || pLexNew == &lexANSI)
  {
    int j;
    int i = 1; // don't re-apply lexers default style
    while (pLexNew->Styles[i].iStyle != -1) 
    {
      for (j = 0; j < 4 && (pLexNew->Styles[i].iStyle8[j] != 0 || j == 0); ++j) {
        Style_SetStyles(hwnd, pLexNew->Styles[i].iStyle8[j], pLexNew->Styles[i].szValue);
      }

      if (pLexNew->iLexer == SCLEX_HTML && pLexNew->Styles[i].iStyle8[0] == SCE_HPHP_DEFAULT) {
        int iRelated[] = { SCE_HPHP_COMMENT, SCE_HPHP_COMMENTLINE, SCE_HPHP_WORD, SCE_HPHP_HSTRING, SCE_HPHP_SIMPLESTRING, SCE_HPHP_NUMBER,
                           SCE_HPHP_OPERATOR, SCE_HPHP_VARIABLE, SCE_HPHP_HSTRING_VARIABLE, SCE_HPHP_COMPLEX_VARIABLE };
        for (j = 0; j < COUNTOF(iRelated); j++)
          Style_SetStyles(hwnd,iRelated[j],pLexNew->Styles[i].szValue);
      }

      if (pLexNew->iLexer == SCLEX_HTML && pLexNew->Styles[i].iStyle8[0] == SCE_HJ_DEFAULT) {
        int iRelated[] = { SCE_HJ_COMMENT, SCE_HJ_COMMENTLINE, SCE_HJ_COMMENTDOC, SCE_HJ_KEYWORD, SCE_HJ_WORD, SCE_HJ_DOUBLESTRING,
                           SCE_HJ_SINGLESTRING, SCE_HJ_STRINGEOL, SCE_HJ_REGEX, SCE_HJ_NUMBER, SCE_HJ_SYMBOLS };
        for (j = 0; j < COUNTOF(iRelated); j++)
          Style_SetStyles(hwnd,iRelated[j],pLexNew->Styles[i].szValue);
      }

      if (pLexNew->iLexer == SCLEX_HTML && pLexNew->Styles[i].iStyle8[0] == SCE_HJA_DEFAULT) {
        int iRelated[] = { SCE_HJA_COMMENT, SCE_HJA_COMMENTLINE, SCE_HJA_COMMENTDOC, SCE_HJA_KEYWORD, SCE_HJA_WORD, SCE_HJA_DOUBLESTRING,
                           SCE_HJA_SINGLESTRING, SCE_HJA_STRINGEOL, SCE_HJA_REGEX, SCE_HJA_NUMBER, SCE_HJA_SYMBOLS };
        for (j = 0; j < COUNTOF(iRelated); j++)
          Style_SetStyles(hwnd,iRelated[j],pLexNew->Styles[i].szValue);
      }

      if (pLexNew->iLexer == SCLEX_HTML && pLexNew->Styles[i].iStyle8[0] == SCE_HB_DEFAULT) {
        int iRelated[] = { SCE_HB_COMMENTLINE, SCE_HB_WORD, SCE_HB_IDENTIFIER, SCE_HB_STRING, SCE_HB_STRINGEOL, SCE_HB_NUMBER };
        for (j = 0; j < COUNTOF(iRelated); j++)
          Style_SetStyles(hwnd,iRelated[j],pLexNew->Styles[i].szValue);
      }

      if (pLexNew->iLexer == SCLEX_HTML && pLexNew->Styles[i].iStyle8[0] == SCE_HBA_DEFAULT) {
        int iRelated[] = { SCE_HBA_COMMENTLINE, SCE_HBA_WORD, SCE_HBA_IDENTIFIER, SCE_HBA_STRING, SCE_HBA_STRINGEOL, SCE_HBA_NUMBER };
        for (j = 0; j < COUNTOF(iRelated); j++)
          Style_SetStyles(hwnd,iRelated[j],pLexNew->Styles[i].szValue);
      }

      if ((pLexNew->iLexer == SCLEX_HTML || pLexNew->iLexer == SCLEX_XML) && pLexNew->Styles[i].iStyle8[0] == SCE_H_SGML_DEFAULT) {
        int iRelated[] = { SCE_H_SGML_COMMAND, SCE_H_SGML_1ST_PARAM, SCE_H_SGML_DOUBLESTRING, SCE_H_SGML_SIMPLESTRING, SCE_H_SGML_ERROR,
                           SCE_H_SGML_SPECIAL, SCE_H_SGML_ENTITY, SCE_H_SGML_COMMENT, SCE_H_SGML_1ST_PARAM_COMMENT, SCE_H_SGML_BLOCK_DEFAULT };
        for (j = 0; j < COUNTOF(iRelated); j++)
          Style_SetStyles(hwnd,iRelated[j],pLexNew->Styles[i].szValue);
      }

      if ((pLexNew->iLexer == SCLEX_HTML || pLexNew->iLexer == SCLEX_XML) && pLexNew->Styles[i].iStyle8[0] == SCE_H_CDATA) {
        int iRelated[] = { SCE_HP_START, SCE_HP_DEFAULT, SCE_HP_COMMENTLINE, SCE_HP_NUMBER, SCE_HP_STRING,
                           SCE_HP_CHARACTER, SCE_HP_WORD, SCE_HP_TRIPLE, SCE_HP_TRIPLEDOUBLE, SCE_HP_CLASSNAME,
                           SCE_HP_DEFNAME, SCE_HP_OPERATOR, SCE_HP_IDENTIFIER, SCE_HPA_START, SCE_HPA_DEFAULT,
                           SCE_HPA_COMMENTLINE, SCE_HPA_NUMBER, SCE_HPA_STRING, SCE_HPA_CHARACTER, SCE_HPA_WORD,
                           SCE_HPA_TRIPLE, SCE_HPA_TRIPLEDOUBLE, SCE_HPA_CLASSNAME, SCE_HPA_DEFNAME, SCE_HPA_OPERATOR,
                           SCE_HPA_IDENTIFIER };
        for (j = 0; j < COUNTOF(iRelated); j++)
          Style_SetStyles(hwnd,iRelated[j],pLexNew->Styles[i].szValue);
      }

      if (pLexNew->iLexer == SCLEX_XML && pLexNew->Styles[i].iStyle8[0] == SCE_H_CDATA) {
        int iRelated[] = { SCE_H_SCRIPT, SCE_H_ASP, SCE_H_ASPAT, SCE_H_QUESTION,
                           SCE_HPHP_DEFAULT, SCE_HPHP_COMMENT, SCE_HPHP_COMMENTLINE, SCE_HPHP_WORD, SCE_HPHP_HSTRING,
                           SCE_HPHP_SIMPLESTRING, SCE_HPHP_NUMBER, SCE_HPHP_OPERATOR, SCE_HPHP_VARIABLE,
                           SCE_HPHP_HSTRING_VARIABLE, SCE_HPHP_COMPLEX_VARIABLE, SCE_HJ_START, SCE_HJ_DEFAULT,
                           SCE_HJ_COMMENT, SCE_HJ_COMMENTLINE, SCE_HJ_COMMENTDOC, SCE_HJ_KEYWORD, SCE_HJ_WORD,
                           SCE_HJ_DOUBLESTRING, SCE_HJ_SINGLESTRING, SCE_HJ_STRINGEOL, SCE_HJ_REGEX, SCE_HJ_NUMBER,
                           SCE_HJ_SYMBOLS, SCE_HJA_START, SCE_HJA_DEFAULT, SCE_HJA_COMMENT, SCE_HJA_COMMENTLINE,
                           SCE_HJA_COMMENTDOC, SCE_HJA_KEYWORD, SCE_HJA_WORD, SCE_HJA_DOUBLESTRING, SCE_HJA_SINGLESTRING,
                           SCE_HJA_STRINGEOL, SCE_HJA_REGEX, SCE_HJA_NUMBER, SCE_HJA_SYMBOLS, SCE_HB_START, SCE_HB_DEFAULT,
                           SCE_HB_COMMENTLINE, SCE_HB_WORD, SCE_HB_IDENTIFIER, SCE_HB_STRING, SCE_HB_STRINGEOL,
                           SCE_HB_NUMBER, SCE_HBA_START, SCE_HBA_DEFAULT, SCE_HBA_COMMENTLINE, SCE_HBA_WORD,
                           SCE_HBA_IDENTIFIER, SCE_HBA_STRING, SCE_HBA_STRINGEOL, SCE_HBA_NUMBER, SCE_HP_START,
                           SCE_HP_DEFAULT, SCE_HP_COMMENTLINE, SCE_HP_NUMBER, SCE_HP_STRING, SCE_HP_CHARACTER, SCE_HP_WORD,
                           SCE_HP_TRIPLE, SCE_HP_TRIPLEDOUBLE, SCE_HP_CLASSNAME, SCE_HP_DEFNAME, SCE_HP_OPERATOR,
                           SCE_HP_IDENTIFIER, SCE_HPA_START, SCE_HPA_DEFAULT, SCE_HPA_COMMENTLINE, SCE_HPA_NUMBER,
                           SCE_HPA_STRING, SCE_HPA_CHARACTER, SCE_HPA_WORD, SCE_HPA_TRIPLE, SCE_HPA_TRIPLEDOUBLE,
                           SCE_HPA_CLASSNAME, SCE_HPA_DEFNAME, SCE_HPA_OPERATOR, SCE_HPA_IDENTIFIER };
        for (j = 0; j < COUNTOF(iRelated); j++)
          Style_SetStyles(hwnd,iRelated[j],pLexNew->Styles[i].szValue);
      }

      if (pLexNew->iLexer == SCLEX_CPP && pLexNew->Styles[i].iStyle8[0] == SCE_C_COMMENT) {
        int iRelated[] = { SCE_C_COMMENTLINE, SCE_C_COMMENTDOC, SCE_C_COMMENTLINEDOC, SCE_C_COMMENTDOCKEYWORD, SCE_C_COMMENTDOCKEYWORDERROR };
        for (j = 0; j < COUNTOF(iRelated); j++)
          Style_SetStyles(hwnd,iRelated[j],pLexNew->Styles[i].szValue);
      }

      if (pLexNew -> iLexer == SCLEX_SQL && pLexNew->Styles[i].iStyle8[0] == SCE_SQL_COMMENT) {
        int iRelated[] = { SCE_SQL_COMMENTLINE, SCE_SQL_COMMENTDOC, SCE_SQL_COMMENTLINEDOC, SCE_SQL_COMMENTDOCKEYWORD, SCE_SQL_COMMENTDOCKEYWORDERROR };
        for (j = 0; j < COUNTOF(iRelated); j++)
          Style_SetStyles(hwnd,iRelated[j],pLexNew->Styles[i].szValue);
      }
      i++;
    }
  }

  // apply lexer styles
  Style_SetUrlHotSpot(hwnd, TRUE);
  SendMessage(hwnd, SCI_COLOURISE, 0, (LPARAM)-1);

  // update UI for hotspots
  Style_SetUrlHotSpot(hwnd, bHyperlinkHotspot);
  EditUpdateUrlHotspots(hwnd, 0, SciCall_GetTextLength(), bHyperlinkHotspot);
  
  // Save current lexer
  pLexCurrent = pLexNew;

}


//=============================================================================
//
//  Style_GetHotspotStyleID()
//
int Style_GetHotspotStyleID()
{
  return (bUse2ndDefaultStyle ? (STYLE_LASTPREDEFINED + STY_URL_HOTSPOT + STY_CNT_LAST) : (STYLE_LASTPREDEFINED + STY_URL_HOTSPOT));
}


//=============================================================================
//
//  Style_SetUrlHotSpot()
//
void Style_SetUrlHotSpot(HWND hwnd, BOOL bHotSpot)
{
  // Use 2nd default style ?
  int iIdx = (bUse2ndDefaultStyle) ? STY_CNT_LAST : 0;

  // Hot Spot settings
  const int iStyleHotSpot = Style_GetHotspotStyleID();

  if (bHotSpot)
  {
    const WCHAR* lpszStyleHotSpot = lexDefault.Styles[STY_URL_HOTSPOT + iIdx].szValue;

    SendMessage(hwnd, SCI_STYLESETHOTSPOT, iStyleHotSpot, (LPARAM)TRUE);
    SendMessage(hwnd, SCI_SETHOTSPOTSINGLELINE, TRUE, 0);

    // Font
    Style_SetStyles(hwnd, iStyleHotSpot, lpszStyleHotSpot);

    //if (StrStrI(lpszStyleHotSpot, L"underline") != NULL)
    //  SendMessage(hwnd, SCI_SETHOTSPOTACTIVEUNDERLINE, TRUE, 0);
    //else
    //  SendMessage(hwnd, SCI_SETHOTSPOTACTIVEUNDERLINE, FALSE, 0);
    SendMessage(hwnd, SCI_SETHOTSPOTACTIVEUNDERLINE, TRUE, 0);

    int rgb = 0;
    // Fore
    if (Style_StrGetColor(TRUE, lpszStyleHotSpot, &rgb)) {
      int inactiveFG = (int)((rgb * 75 + 50) / 100);
      SendMessage(hwnd, SCI_STYLESETFORE, iStyleHotSpot, (LPARAM)inactiveFG);
      SendMessage(hwnd, SCI_SETHOTSPOTACTIVEFORE, TRUE, (LPARAM)rgb);
    }
    // Back
    if (Style_StrGetColor(FALSE, lpszStyleHotSpot, &rgb)) {
      SendMessage(hwnd, SCI_STYLESETBACK, iStyleHotSpot, (LPARAM)rgb);
      SendMessage(hwnd, SCI_SETHOTSPOTACTIVEBACK, TRUE, (LPARAM)rgb);
    }
  }
  else
    SendMessage(hwnd, SCI_STYLESETHOTSPOT, iStyleHotSpot, (LPARAM)FALSE);

}


//=============================================================================
//
//  Style_SetLongLineColors()
//
void Style_SetLongLineColors(HWND hwnd)
{
  int rgb;

  // Use 2nd default style
  int iIdx = (bUse2ndDefaultStyle) ? STY_CNT_LAST : 0;

  if (SendMessage(hwnd,SCI_GETEDGEMODE,0,0) == EDGE_LINE) {
    if (Style_StrGetColor(TRUE,lexDefault.Styles[STY_LONG_LN_MRK + iIdx].szValue,&rgb)) // edge fore
      SendMessage(hwnd,SCI_SETEDGECOLOUR,rgb,0);
    else
      SendMessage(hwnd,SCI_SETEDGECOLOUR,GetSysColor(COLOR_3DLIGHT),0);
  }
  else {
    if (Style_StrGetColor(FALSE,lexDefault.Styles[STY_LONG_LN_MRK + iIdx].szValue,&rgb)) // edge back
      SendMessage(hwnd,SCI_SETEDGECOLOUR,rgb,0);
    else
      SendMessage(hwnd,SCI_SETEDGECOLOUR,GetSysColor(COLOR_3DLIGHT),0);
  }
}


//=============================================================================
//
//  Style_SetCurrentLineBackground()
//
void Style_SetCurrentLineBackground(HWND hwnd, BOOL bHiLitCurrLn)
{
  if (bHiLitCurrLn) {
    
    // Use 2nd default style ?
    int iIdx = (bUse2ndDefaultStyle) ? STY_CNT_LAST : 0;

    int rgb = 0;
    if (Style_StrGetColor(FALSE,lexDefault.Styles[STY_CUR_LN_BCK + iIdx].szValue,&rgb)) // caret line back
    {
      SendMessage(hwnd,SCI_SETCARETLINEVISIBLE,TRUE,0);
      SendMessage(hwnd,SCI_SETCARETLINEBACK,rgb,0);

      int alpha = 0;
      if (Style_StrGetAlpha(lexDefault.Styles[STY_CUR_LN_BCK + iIdx].szValue, &alpha, TRUE))
        SendMessage(hwnd,SCI_SETCARETLINEBACKALPHA,alpha,0);
      else
        SendMessage(hwnd,SCI_SETCARETLINEBACKALPHA,SC_ALPHA_NOALPHA,0);
    }
    else
      SendMessage(hwnd,SCI_SETCARETLINEVISIBLE,FALSE,0);
  }
  else
    SendMessage(hwnd,SCI_SETCARETLINEVISIBLE,FALSE,0);
}


//=============================================================================
//
//  Style_SetCurrentMargin()
//
void Style_SetCurrentMargin(HWND hwnd, BOOL bShowSelMargin) {

  int iIdx = (bUse2ndDefaultStyle) ? STY_CNT_LAST : 0;

  int alpha = 20;
  Style_StrGetAlpha(lexDefault.Styles[STY_BOOK_MARK + iIdx].szValue, &alpha, TRUE);

  int rgbFore = RGB(0xFF, 0, 0); // red
  Style_StrGetColor(TRUE, lexDefault.Styles[STY_BOOK_MARK + iIdx].szValue, &rgbFore);

  int rgbBack = RGB(0, 0xFF, 0); // green
  Style_StrGetColor(FALSE, lexDefault.Styles[STY_BOOK_MARK + iIdx].szValue, &rgbBack);

  // adjust background color by alpha in case of show margin
  if (bShowSelMargin) {
    //int bckgrnd = (int)SendMessage(hwnd, SCI_GETMARGINBACKN, MARGIN_NP3_BOOKMRK, 0);
    int bckgrnd = RGB(0xF0, 0xF0, 0xF0);
    Style_StrGetColor(FALSE, lexDefault.Styles[STY_MARGIN + iIdx].szValue, &bckgrnd);
    rgbBack = Style_RgbAlpha(rgbBack, bckgrnd, min(0xFF, alpha + 50));
  }
  SendMessage(hwnd, SCI_MARKERSETFORE, MARKER_NP3_BOOKMARK, rgbFore);
  SendMessage(hwnd, SCI_MARKERSETBACK, MARKER_NP3_BOOKMARK, rgbBack);
  SendMessage(hwnd, SCI_MARKERSETALPHA, MARKER_NP3_BOOKMARK, alpha);

  // Depending on if the margin is visible or not, choose different bookmark indication
  if (bShowSelMargin) {
    SendMessage(hwnd, SCI_MARKERDEFINE, MARKER_NP3_BOOKMARK, SC_MARK_BOOKMARK);
  }
  else {
    SendMessage(hwnd, SCI_MARKERDEFINE, MARKER_NP3_BOOKMARK, SC_MARK_BACKGROUND);
  }
  //SendMessage(hwndEdit, SCI_SETMARGINWIDTHN, 2, 0);
  SendMessage(hwnd, SCI_SETMARGINWIDTHN, MARGIN_NP3_BOOKMRK, (bShowSelMargin) ? 16 : 0);
}


//=============================================================================
//
//  Style_SniffShebang()
//
PEDITLEXER __fastcall Style_SniffShebang(char *pchText)
{
  if (StrCmpNA(pchText,"#!",2) == 0) {
    char *pch = pchText + 2;
    while (*pch == ' ' || *pch == '\t')
      pch++;
    while (*pch && *pch != ' ' && *pch != '\t' && *pch != '\r' && *pch != '\n')
      pch++;
    if ((pch - pchText) >= 3 && StrCmpNA(pch-3,"env",3) == 0) {
      while (*pch == ' ')
        pch++;
      while (*pch && *pch != ' ' && *pch != '\t' && *pch != '\r' && *pch != '\n')
        pch++;
    }
    if ((pch - pchText) >= 3 && StrCmpNIA(pch-3,"php",3) == 0)
      return(&lexHTML);
    else if ((pch - pchText) >= 4 && StrCmpNIA(pch-4,"perl",4) == 0)
      return(&lexPL);
    else if ((pch - pchText) >= 6 && StrCmpNIA(pch-6,"python",6) == 0)
      return(&lexPY);
    else if ((pch - pchText) >= 3 && StrCmpNA(pch-3,"tcl",3) == 0)
      return(&lexTCL);
    else if ((pch - pchText) >= 4 && StrCmpNA(pch-4,"wish",4) == 0)
      return(&lexTCL);
    else if ((pch - pchText) >= 5 && StrCmpNA(pch-5,"tclsh",5) == 0)
      return(&lexTCL);
    else if ((pch - pchText) >= 2 && StrCmpNA(pch-2,"sh",2) == 0)
      return(&lexBASH);
    else if ((pch - pchText) >= 4 && StrCmpNA(pch-4,"ruby",4) == 0)
      return(&lexRUBY);
    else if ((pch - pchText) >= 4 && StrCmpNA(pch-4,"node",4) == 0)
      return(&lexJS);
  }

  return(NULL);
}


//=============================================================================
//
//  Style_MatchLexer()
//
PEDITLEXER __fastcall Style_MatchLexer(LPCWSTR lpszMatch,BOOL bCheckNames) {
  int i;
  WCHAR  tch[COUNTOF(pLexArray[0]->szExtensions)] = { L'\0' };
  WCHAR  *p1,*p2;

  if (!bCheckNames) {

    for (i = 0; i < COUNTOF(pLexArray); i++) {
      ZeroMemory(tch,sizeof(WCHAR)*COUNTOF(tch));
      StringCchCopy(tch,COUNTOF(tch),pLexArray[i]->szExtensions);
      p1 = tch;
      while (*p1) {
        p2 = StrChr(p1,L';');
        if (p2)
          *p2 = L'\0';
        else
          p2 = StrEnd(p1);
        StrTrim(p1,L" .");
        if (StringCchCompareIX(p1,lpszMatch) == 0)
          return(pLexArray[i]);
        p1 = p2 + 1;
      }
    }
  }

  else {

    int cch = lstrlen(lpszMatch);
    if (cch >= 3) {

      for (i = 0; i < COUNTOF(pLexArray); i++) {
        if (StrCmpNI(pLexArray[i]->pszName,lpszMatch,cch) == 0)
          return(pLexArray[i]);
      }
    }
  }
  return(NULL);
}


//=============================================================================
//
//  Style_HasLexerForExt()
//
BOOL Style_HasLexerForExt(LPCWSTR lpszExt)
{
  if (lpszExt && (*lpszExt == L'.')) ++lpszExt;
  return (lpszExt && Style_MatchLexer(lpszExt,FALSE)) ? TRUE : FALSE;
}


//=============================================================================
//
//  Style_SetLexerFromFile()
//
extern int fNoHTMLGuess;
extern int fNoCGIGuess;
extern FILEVARS fvCurFile;

void Style_SetLexerFromFile(HWND hwnd,LPCWSTR lpszFile)
{
  LPWSTR lpszExt = PathFindExtension(lpszFile);
  BOOL  bFound = FALSE;
  PEDITLEXER pLexNew = pLexArray[iDefaultLexer];
  PEDITLEXER pLexSniffed;

  if ((fvCurFile.mask & FV_MODE) && fvCurFile.tchMode[0]) {

    WCHAR wchMode[32] = { L'\0' };
    PEDITLEXER pLexMode;
    UINT cp = Encoding_SciGetCodePage(hwnd);
    MultiByteToWideCharStrg(cp,fvCurFile.tchMode,wchMode);

    if (!fNoCGIGuess && (StringCchCompareIN(wchMode,COUNTOF(wchMode),L"cgi",-1) == 0 || 
                         StringCchCompareIN(wchMode,COUNTOF(wchMode),L"fcgi",-1) == 0)) {
      char tchText[256] = { L'\0' };
      SendMessage(hwnd,SCI_GETTEXT,(WPARAM)COUNTOF(tchText)-1,(LPARAM)tchText);
      StrTrimA(tchText," \t\n\r");
      pLexSniffed = Style_SniffShebang(tchText);
      if (pLexSniffed) {
        if (Encoding_Current(CPI_GET) != g_DOSEncoding || pLexSniffed != &lexDefault || (
          StringCchCompareIX(lpszExt,L"nfo") && StringCchCompareIX(lpszExt,L"diz"))) {
          // Although .nfo and .diz were removed from the default lexer's
          // default extensions list, they may still presist in the user's INI
          pLexNew = pLexSniffed;
          bFound = TRUE;
        }
      }
    }

    if (!bFound) {
      pLexMode = Style_MatchLexer(wchMode, FALSE);
      if (pLexMode) {
        pLexNew = pLexMode;
        bFound = TRUE;
      }
      else {
        pLexMode = Style_MatchLexer(wchMode, TRUE);
        if (pLexMode) {
          pLexNew = pLexMode;
          bFound = TRUE;
        }
      }
    }
  }

  if (!bFound && bAutoSelect && /* bAutoSelect == FALSE skips lexer search */
      (lpszFile && StringCchLen(lpszFile,MAX_PATH) > 0 && *lpszExt)) {

    if (*lpszExt == L'.') ++lpszExt;

    if (!fNoCGIGuess && (StringCchCompareIX(lpszExt,L"cgi") == 0 || StringCchCompareIX(lpszExt,L"fcgi") == 0)) {
      char tchText[256] = { L'\0' };
      SendMessage(hwnd,SCI_GETTEXT,(WPARAM)COUNTOF(tchText)-1,(LPARAM)tchText);
      StrTrimA(tchText," \t\n\r");
      pLexSniffed = Style_SniffShebang(tchText);
      if (pLexSniffed) {
        pLexNew = pLexSniffed;
        bFound = TRUE;
      }
    }

    if (!bFound && StringCchCompareIX(PathFindFileName(lpszFile),L"cmakelists.txt") == 0) {
      pLexNew = &lexCmake;
      bFound = TRUE;
    }

    // check associated extensions
    if (!bFound) {
      pLexSniffed = Style_MatchLexer(lpszExt, FALSE);
      if (pLexSniffed) {
        pLexNew = pLexSniffed;
        bFound = TRUE;
      }
    }
  }

  if (!bFound && bAutoSelect &&
      StringCchCompareIX(PathFindFileName(lpszFile),L"makefile") == 0) {
    pLexNew = &lexMAK;
    bFound = TRUE;
  }

  if (!bFound && bAutoSelect &&
      StringCchCompareIX(PathFindFileName(lpszFile),L"rakefile") == 0) {
    pLexNew = &lexRUBY;
    bFound = TRUE;
  }

  if (!bFound && bAutoSelect &&
      StringCchCompareIX(PathFindFileName(lpszFile),L"mozconfig") == 0) {
    pLexNew = &lexBASH;
    bFound = TRUE;
  }

  if (!bFound && bAutoSelect && (!fNoHTMLGuess || !fNoCGIGuess)) {
    char tchText[512];
    SendMessage(hwnd,SCI_GETTEXT,(WPARAM)COUNTOF(tchText)-1,(LPARAM)tchText);
    StrTrimA(tchText," \t\n\r");
    if (!fNoCGIGuess) {
      if (tchText[0] == '<') {
        if (StrStrIA(tchText, "<html"))
          pLexNew = &lexHTML;
        else
          pLexNew = &lexXML;
        bFound = TRUE;
      }
      else {
        pLexSniffed = Style_SniffShebang(tchText);
        if (pLexSniffed) {
          pLexNew = pLexSniffed;
          bFound = TRUE;
        }
      }
    }
  }

  if (!bFound && Encoding_Current(CPI_GET) == g_DOSEncoding)
    pLexNew = &lexANSI;

  // Apply the new lexer
  Style_SetLexer(hwnd,pLexNew);
}


//=============================================================================
//
//  Style_SetLexerFromName()
//
void Style_SetLexerFromName(HWND hwnd,LPCWSTR lpszFile,LPCWSTR lpszName)
{
  PEDITLEXER pLexNew = Style_MatchLexer(lpszName, FALSE);
  if (pLexNew)
    Style_SetLexer(hwnd,pLexNew);
  else {
    pLexNew = Style_MatchLexer(lpszName, TRUE);
    if (pLexNew)
      Style_SetLexer(hwnd, pLexNew);
    else
      Style_SetLexerFromFile(hwnd, lpszFile);
  }
}


//=============================================================================
//
//  Style_SetDefaultLexer()
//
void Style_SetDefaultLexer(HWND hwnd)
{
  Style_SetLexer(hwnd,pLexArray[0]);
}


//=============================================================================
//
//  Style_SetHTMLLexer()
//
void Style_SetHTMLLexer(HWND hwnd)
{
  Style_SetLexer(hwnd,Style_MatchLexer(L"Web Source Code",TRUE));
}


//=============================================================================
//
//  Style_SetXMLLexer()
//
void Style_SetXMLLexer(HWND hwnd)
{
  Style_SetLexer(hwnd,Style_MatchLexer(L"XML Document",TRUE));
}


//=============================================================================
//
//  Style_SetLexerFromID()
//
void Style_SetLexerFromID(HWND hwnd,int id)
{
  if (id >= 0 && id < COUNTOF(pLexArray)) {
    Style_SetLexer(hwnd,pLexArray[id]);
  }
}


//=============================================================================
//
//  Style_ToggleUse2ndDefault()
//
void Style_ToggleUse2ndDefault(HWND hwnd)
{
  bUse2ndDefaultStyle = (bUse2ndDefaultStyle) ? FALSE : TRUE;
  Style_SetLexer(hwnd,pLexCurrent);
}


//=============================================================================
//
//  Style_SetDefaultFont()
//
void Style_SetDefaultFont(HWND hwnd)
{
  const int iIdx = (bUse2ndDefaultStyle) ? STY_CNT_LAST : 0;
  if (Style_SelectFont(hwnd, lexDefault.Styles[STY_DEFAULT + iIdx].szValue,
        COUNTOF(lexDefault.Styles[STY_DEFAULT + iIdx].szValue), TRUE)) 
  {
    fStylesModified = TRUE;
    // clear lexers default - lexer inherits global default then
    pLexCurrent->Styles[STY_DEFAULT].szValue[0] = L'\0';
    Style_SetLexer(hwnd,pLexCurrent);
  }
}


//=============================================================================
//
//  Style_GetUse2ndDefault()
//
BOOL Style_GetUse2ndDefault(HWND hwnd)
{
  UNUSED(hwnd);
  return (bUse2ndDefaultStyle);
}


//=============================================================================
//
//  Style_SetIndentGuides()
//
extern int flagSimpleIndentGuides;

void Style_SetIndentGuides(HWND hwnd,BOOL bShow)
{
  int iIndentView = SC_IV_NONE;
  if (bShow) {
    if (!flagSimpleIndentGuides) {
      if (SendMessage(hwnd,SCI_GETLEXER,0,0) == SCLEX_PYTHON)
        iIndentView = SC_IV_LOOKFORWARD;
      else
        iIndentView = SC_IV_LOOKBOTH;
    }
    else
      iIndentView = SC_IV_REAL;
  }
  SendMessage(hwnd,SCI_SETINDENTATIONGUIDES,iIndentView,0);
}


//=============================================================================
//
//  Style_GetFileOpenDlgFilter()
//
extern WCHAR tchFileDlgFilters[5*1024];

BOOL Style_GetOpenDlgFilterStr(LPWSTR lpszFilter,int cchFilter)
{
  if (StringCchLenW(tchFileDlgFilters,COUNTOF(tchFileDlgFilters)) == 0)
    GetString(IDS_FILTER_ALL,lpszFilter,cchFilter);
  else {
    StringCchCopyN(lpszFilter,cchFilter,tchFileDlgFilters,cchFilter - 2);
    StringCchCat(lpszFilter,cchFilter,L"||");
  }
  PrepareFilterStr(lpszFilter);
  return TRUE;
}


//=============================================================================
//
//  Style_StrGetFont()
//
BOOL Style_StrGetFont(LPCWSTR lpszStyle,LPWSTR lpszFont,int cchFont)
{
  WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
  WCHAR *p = StrStrI(lpszStyle, L"font:");
  if (p)
  {
    StringCchCopy(tch,COUNTOF(tch),p + CSTRLEN(L"font:"));
    p = StrChr(tch, L';');
    if (p)
      *p = L'\0';
    TrimString(tch);

    if (StringCchCompareIN(tch,COUNTOF(tch),L"Default",-1) == 0)
    {
      if (IsFontAvailable(L"Consolas"))
        StringCchCopyN(lpszFont,cchFont,L"Consolas",cchFont);
      else
        StringCchCopyN(lpszFont,cchFont,L"Lucida Console",cchFont);
    }
    else
    {
      StringCchCopyN(lpszFont,cchFont,tch,cchFont);
    }

    return TRUE;
  }
  return FALSE;
}


//=============================================================================
//
//  Style_StrGetFontQuality()
//
BOOL Style_StrGetFontQuality(LPCWSTR lpszStyle,LPWSTR lpszQuality,int cchQuality)
{
  WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
  WCHAR *p = StrStrI(lpszStyle, L"smoothing:");
  if (p)
  {
    StringCchCopy(tch,COUNTOF(tch),p + CSTRLEN(L"smoothing:"));
    p = StrChr(tch, L';');
    if (p)
      *p = L'\0';
    TrimString(tch);
    if (StringCchCompareIN(tch,COUNTOF(tch),L"none",-1) == 0 ||
        StringCchCompareIN(tch,COUNTOF(tch),L"standard",-1) == 0 ||
        StringCchCompareIN(tch,COUNTOF(tch),L"cleartype",-1) == 0 ||
        StringCchCompareIN(tch,COUNTOF(tch),L"default",-1) == 0) 
    {
      StringCchCopyN(lpszQuality,cchQuality,tch,COUNTOF(tch));
      return TRUE;
    }
  }
  return FALSE;
}


//=============================================================================
//
//  Style_StrGetCharSet()
//
BOOL Style_StrGetCharSet(LPCWSTR lpszStyle,int* i)
{
  WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
  WCHAR *p = StrStrI(lpszStyle, L"charset:");
  if (p)
  {
    StringCchCopy(tch,COUNTOF(tch),p + CSTRLEN(L"charset:"));
    p = StrChr(tch, L';');
    if (p)
      *p = L'\0';
    TrimString(tch);
    int iValue = 0;
    int itok = swscanf_s(tch,L"%i",&iValue);
    if (itok == 1)
    {
      *i = iValue;
      return TRUE;
    }
  }
  return FALSE;
}


//=============================================================================
//
//  Style_StrGetSize()
//
BOOL Style_StrGetSize(LPCWSTR lpszStyle,int* i)
{
  WCHAR *p = StrStrI(lpszStyle, L"size:");
  if (p)
  {
    int iSign = 0;
    WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
    StringCchCopy(tch,COUNTOF(tch),p + CSTRLEN(L"size:"));
    if (tch[0] == L'+')
    {
      iSign = 1;
      tch[0] = L' ';
    }
    else if (tch[0] == L'-')
    {
      iSign = -1;
      tch[0] = L' ';
    }
    p = StrChr(tch, L';');
    if (p)
      *p = L'\0';
    TrimString(tch);
    int iValue = 0;
    int itok = swscanf_s(tch,L"%i",&iValue);
    if (itok == 1)
    {
      if (iSign == 0)
        *i = iValue;
      else
        *i = max(0,iBaseFontSize + iValue * iSign); // size must be +
      return TRUE;
    }
  }
  return FALSE;
}


//=============================================================================
//
//  Style_StrGetSizeStr()
//
BOOL Style_StrGetSizeStr(LPCWSTR lpszStyle,LPWSTR lpszSize,int cchSize)
{
  WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
  WCHAR *p = StrStrI(lpszStyle, L"size:");
  if (p)
  {
    StringCchCopy(tch,COUNTOF(tch),p + CSTRLEN(L"size:"));
    p = StrChr(tch, L';');
    if (p)
      *p = L'\0';
    TrimString(tch);
    StringCchCopyN(lpszSize,cchSize,tch,COUNTOF(tch));
    return TRUE;
  }
  return FALSE;
}


//=============================================================================
//
//  Style_StrGetColor()
//
BOOL Style_StrGetColor(BOOL bFore,LPCWSTR lpszStyle,int *rgb)
{
  WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
  WCHAR *pItem = (bFore) ? L"fore:" : L"back:";

  WCHAR *p = StrStrI(lpszStyle, pItem);
  if (p)
  {
    StringCchCopy(tch,COUNTOF(tch),p + lstrlen(pItem));
    if (tch[0] == L'#')
      tch[0] = L' ';
    p = StrChr(tch, L';');
    if (p)
      *p = L'\0';
    TrimString(tch);
    int iValue = 0;
    int itok = swscanf_s(tch,L"%x",&iValue);
    if (itok == 1)
    {
      *rgb = RGB((iValue&0xFF0000) >> 16,(iValue&0xFF00) >> 8,iValue&0xFF);
      return TRUE;
    }
  }
  return FALSE;
}


//=============================================================================
//
//  Style_StrGetAlpha()
//
BOOL Style_StrGetAlpha(LPCWSTR lpszStyle, int* i, BOOL bAlpha1st) 
{
  const WCHAR* strAlpha = bAlpha1st ? L"alpha:" : L"alpha2:";

  WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
  WCHAR* p = StrStrI(lpszStyle, strAlpha);
  if (p) {
    StringCchCopy(tch, COUNTOF(tch), p + lstrlen(strAlpha));
    p = StrChr(tch, L';');
    if (p)
      *p = L'\0';
    TrimString(tch);
    int iValue = 0;
    int itok = swscanf_s(tch, L"%i", &iValue);
    if (itok == 1) {
      *i = min(max(SC_ALPHA_TRANSPARENT, iValue), SC_ALPHA_OPAQUE);
      return TRUE;
    }
  }
  return FALSE;
}


//=============================================================================
//
//  Style_StrGetCase()
//
BOOL Style_StrGetCase(LPCWSTR lpszStyle,int* i)
{
  WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
  WCHAR *p = StrStrI(lpszStyle, L"case:");
  if (p)
  {
    StringCchCopy(tch,COUNTOF(tch),p + CSTRLEN(L"case:"));
    p = StrChr(tch, L';');
    if (p)
      *p = L'\0';
    TrimString(tch);
    if (tch[0] == L'u' || tch[0] == L'U') {
      *i = SC_CASE_UPPER;
      return TRUE;
    }
    else if (tch[0] == L'l' || tch[0] == L'L') {
      *i = SC_CASE_LOWER;
      return TRUE;
  }
  }
  return FALSE;
}


//=============================================================================
//
//  Style_GetIndicatorType()
//

static WCHAR* IndicatorTypes[20] = {
  L"indic_plain",
  L"indic_squiggle",
  L"indic_tt",
  L"indic_diagonal",
  L"indic_strike",
  L"indic_hidden",
  L"indic_box",
  L"indic_roundbox",
  L"indic_straightbox",
  L"indic_dash",
  L"indic_dots",
  L"indic_squigglelow",
  L"indic_dotbox",
  L"indic_squigglepixmap",
  L"indic_compositionthick",
  L"indic_compositionthin",
  L"indic_fullbox",
  L"indic_textfore",
  L"indic_point",
  L"indic_pointcharacter"
};

BOOL Style_GetIndicatorType(LPWSTR lpszStyle, int cchSize, int* idx)
{
  if (*idx < 0) { // retrieve indicator style from string
    for (int i = 0; i < COUNTOF(IndicatorTypes); i++) {
      if (StrStrI(lpszStyle, IndicatorTypes[i])) {
        *idx = i;
        return TRUE;
      }
    }
    *idx = INDIC_ROUNDBOX; // default
  }
  else {  // get indicator string from index

    if (*idx < COUNTOF(IndicatorTypes)) 
    {
      StringCchCopy(lpszStyle, cchSize, IndicatorTypes[*idx]);
      return TRUE;
    }
    StringCchCopy(lpszStyle, cchSize, IndicatorTypes[INDIC_ROUNDBOX]); // default
  }
  return FALSE;
}


//=============================================================================
//
//  Style_SelectFont()
//
BOOL Style_SelectFont(HWND hwnd,LPWSTR lpszStyle,int cchStyle,BOOL bDefaultStyle)
{
  CHOOSEFONT cf;
  LOGFONT lf;
  WCHAR szNewStyle[BUFSIZE_STYLE_VALUE] = { L'\0' };
  int  iValue;
  WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
  HDC hdc;

  ZeroMemory(&cf,sizeof(CHOOSEFONT));
  ZeroMemory(&lf,sizeof(LOGFONT));

  // Map lpszStyle to LOGFONT
  if (Style_StrGetFont(lpszStyle,tch,COUNTOF(tch)))
    StringCchCopyN(lf.lfFaceName,COUNTOF(lf.lfFaceName),tch,COUNTOF(lf.lfFaceName));
  if (Style_StrGetCharSet(lpszStyle,&iValue))
    lf.lfCharSet = (BYTE)iValue;
  if (Style_StrGetSize(lpszStyle,&iValue)) {
    hdc = GetDC(hwnd);
    lf.lfHeight = -MulDiv(iValue,GetDeviceCaps(hdc,LOGPIXELSY),72);
    ReleaseDC(hwnd,hdc);
  }

  if (StrStrI(lpszStyle, L"thin"))
    lf.lfWeight = FW_THIN;
  else if (StrStrI(lpszStyle, L"extralight"))
    lf.lfWeight = FW_EXTRALIGHT;
  else if (StrStrI(lpszStyle, L"light"))
    lf.lfWeight = FW_LIGHT;
  else if (StrStrI(lpszStyle, L"normal"))
    lf.lfWeight = FW_NORMAL;
  else if (StrStrI(lpszStyle, L"medium"))
    lf.lfWeight = FW_MEDIUM;
  else if (StrStrI(lpszStyle, L"semibold"))
    lf.lfWeight = FW_SEMIBOLD;
  else if (StrStrI(lpszStyle, L"bold"))
    lf.lfWeight = FW_BOLD;
  else if (StrStrI(lpszStyle, L"extrabold"))
    lf.lfWeight = FW_EXTRABOLD;
  else if (StrStrI(lpszStyle, L"heavy"))
    lf.lfWeight = FW_HEAVY;
  else
    lf.lfWeight = FW_NORMAL;

  lf.lfItalic = (StrStrI(lpszStyle,L"italic")) ? TRUE : FALSE;

  COLORREF color = 0L;
  if (Style_StrGetColor(TRUE, lpszStyle, &iValue)) {
    color = RGB(GetRValue(iValue),GetGValue(iValue),GetBValue(iValue));
  }

  // Init cf
  cf.lStructSize = sizeof(CHOOSEFONT);
  cf.hwndOwner = hwnd;
  cf.rgbColors = color;
  cf.lpLogFont = &lf;

  cf.Flags = CF_INITTOLOGFONTSTRUCT /*| CF_EFFECTS | CF_NOSCRIPTSEL*/ | CF_SCREENFONTS | CF_FORCEFONTEXIST;
  if (HIBYTE(GetKeyState(VK_SHIFT)))
    cf.Flags |= CF_FIXEDPITCHONLY;

  if (!ChooseFont(&cf) || !StringCchLenW(lf.lfFaceName,COUNTOF(lf.lfFaceName)))
    return FALSE;

  // Map back to lpszStyle
  StringCchCopy(szNewStyle,COUNTOF(szNewStyle),L"font:");
  StringCchCat(szNewStyle,COUNTOF(szNewStyle),lf.lfFaceName);
  if (Style_StrGetFontQuality(lpszStyle,tch,COUNTOF(tch)))
  {
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),L"; smoothing:");
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),tch);
  }
  if (bDefaultStyle &&
      lf.lfCharSet != DEFAULT_CHARSET &&
      lf.lfCharSet != ANSI_CHARSET &&
      lf.lfCharSet != iDefaultCharSet) {
    StringCchPrintf(tch,COUNTOF(tch),L"; charset:%i",lf.lfCharSet);
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),tch);
  }
  StringCchPrintf(tch,COUNTOF(tch),L"; size:%i",cf.iPointSize/10);
  StringCchCat(szNewStyle,COUNTOF(szNewStyle),tch);
  
  switch (lf.lfWeight) {
  case FW_THIN:
    StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; thin");
    break;
  case FW_EXTRALIGHT:
    StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; extralight");
    break;
  case FW_LIGHT:
    StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; light");
    break;
  case FW_MEDIUM:
    StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; medium");
    break;
  case FW_SEMIBOLD:
    StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; semibold");
    break;
  case FW_BOLD:
    StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; bold");
    break;
  case FW_EXTRABOLD:
    StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; extrabold");
    break;
  case FW_HEAVY:
    StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; heavy");
    break;
  case FW_NORMAL:
  default:
    //~StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; normal");
    if (cf.nFontType & BOLD_FONTTYPE)
      StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; bold");
    break;
  }

  if ((cf.nFontType & ITALIC_FONTTYPE) || lf.lfItalic)
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),L"; italic");

  if (StrStrI(lpszStyle,L"underline"))
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),L"; underline");

  // save colors
  if (color != 0)
  {
    StringCchPrintf(tch,COUNTOF(tch),L"; fore:#%02X%02X%02X",
      (int)GetRValue(cf.rgbColors),
      (int)GetGValue(cf.rgbColors),
      (int)GetBValue(cf.rgbColors));
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),tch);
  }
  if (Style_StrGetColor(FALSE,lpszStyle,&iValue))
  {
    StringCchPrintf(tch,COUNTOF(tch),L"; back:#%02X%02X%02X",
      (int)GetRValue(iValue),
      (int)GetGValue(iValue),
      (int)GetBValue(iValue));
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),tch);
  }

  if (StrStrI(lpszStyle,L"eolfilled"))
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),L"; eolfilled");

  if (Style_StrGetCase(lpszStyle,&iValue)) {
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),L"; case:");
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),(iValue == SC_CASE_UPPER) ? L"u" : L"");
  }

  if (Style_StrGetAlpha(lpszStyle, &iValue, TRUE)) {
    StringCchPrintf(tch,COUNTOF(tch),L"; alpha:%i",iValue);
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),tch);
  }
  if (Style_StrGetAlpha(lpszStyle, &iValue, FALSE)) {
    StringCchPrintf(tch, COUNTOF(tch), L"; alpha2:%i", iValue);
    StringCchCat(szNewStyle, COUNTOF(szNewStyle), tch);
  }

  iValue = -1;
  if (Style_GetIndicatorType(lpszStyle, cchStyle, &iValue)) {
    StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; ");
    Style_GetIndicatorType(tch, COUNTOF(tch), &iValue);
    StringCchCat(szNewStyle, COUNTOF(szNewStyle), tch);
  }

  StrTrim(szNewStyle, L" ;");
  StringCchCopyN(lpszStyle,cchStyle,szNewStyle,COUNTOF(szNewStyle));
  return TRUE;
}


//=============================================================================
//
//  Style_SelectColor()
//
BOOL Style_SelectColor(HWND hwnd,BOOL bFore,LPWSTR lpszStyle,int cchStyle)
{
  CHOOSECOLOR cc;
  WCHAR szNewStyle[BUFSIZE_STYLE_VALUE] = { L'\0' };
  int  iRGBResult;
  int  iValue;
  WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };

  ZeroMemory(&cc,sizeof(CHOOSECOLOR));

  iRGBResult = (bFore) ? GetSysColor(COLOR_WINDOWTEXT) : GetSysColor(COLOR_WINDOW);
  Style_StrGetColor(bFore,lpszStyle,&iRGBResult);

  cc.lStructSize = sizeof(CHOOSECOLOR);
  cc.hwndOwner = hwnd;
  cc.rgbResult = iRGBResult;
  cc.lpCustColors = crCustom;
  cc.Flags = CC_FULLOPEN | CC_RGBINIT | CC_SOLIDCOLOR;

  if (!ChooseColor(&cc))
    return FALSE;

  iRGBResult = cc.rgbResult;

  // Rebuild style string
  StringCchCopy(szNewStyle,COUNTOF(szNewStyle),L"");

  if (Style_StrGetFont(lpszStyle,tch,COUNTOF(tch)))
  {
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),L"font:");
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),tch);
  }
  if (Style_StrGetFontQuality(lpszStyle,tch,COUNTOF(tch)))
  {
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),L"; smoothing:");
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),tch);
  }
  if (Style_StrGetCharSet(lpszStyle,&iValue))
  {
    StringCchPrintf(tch,COUNTOF(tch),L"; charset:%i",iValue);
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),tch);
  }
  if (Style_StrGetSizeStr(lpszStyle,tch,COUNTOF(tch)))
  {
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),L"; size:");
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),tch);
  }

  if (StrStrI(lpszStyle, L"thin"))
    StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; thin");
  else if (StrStrI(lpszStyle, L"extralight"))
    StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; extralight");
  else if (StrStrI(lpszStyle, L"light"))
    StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; light");
  else if (StrStrI(lpszStyle, L"normal"))
    StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; normal");
  else if (StrStrI(lpszStyle, L"medium"))
    StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; medium");
  else if (StrStrI(lpszStyle, L"semibold"))
    StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; semibold");
  else if (StrStrI(lpszStyle, L"bold"))
    StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; bold");
  else if (StrStrI(lpszStyle, L"extrabold"))
    StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; extrabold");
  else if (StrStrI(lpszStyle, L"heavy"))
    StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; heavy");
  //else
  //  StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; normal");

  if (StrStrI(lpszStyle,L"italic"))
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),L"; italic");

  if (StrStrI(lpszStyle,L"underline"))
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),L"; underline");

  if (bFore)
  {
    StringCchPrintf(tch,COUNTOF(tch),L"; fore:#%02X%02X%02X",
      (int)GetRValue(iRGBResult),
      (int)GetGValue(iRGBResult),
      (int)GetBValue(iRGBResult));
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),tch);
    if (Style_StrGetColor(FALSE,lpszStyle,&iValue))
    {
      StringCchPrintf(tch,COUNTOF(tch),L"; back:#%02X%02X%02X",
        (int)GetRValue(iValue),
        (int)GetGValue(iValue),
        (int)GetBValue(iValue));
      StringCchCat(szNewStyle,COUNTOF(szNewStyle),tch);
    }
  }
  else
  {
    if (Style_StrGetColor(TRUE,lpszStyle,&iValue))
    {
      StringCchPrintf(tch,COUNTOF(tch),L"; fore:#%02X%02X%02X; ",
        (int)GetRValue(iValue),
        (int)GetGValue(iValue),
        (int)GetBValue(iValue));
      StringCchCat(szNewStyle,COUNTOF(szNewStyle),tch);
    }
    StringCchPrintf(tch,COUNTOF(tch),L"; back:#%02X%02X%02X",
      (int)GetRValue(iRGBResult),
      (int)GetGValue(iRGBResult),
      (int)GetBValue(iRGBResult));
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),tch);
  }

  if (StrStrI(lpszStyle,L"eolfilled"))
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),L"; eolfilled");

  if (Style_StrGetCase(lpszStyle,&iValue)) {
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),L"; case");
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),(iValue == SC_CASE_UPPER) ? L"u" : L"");
  }

  if (Style_StrGetAlpha(lpszStyle, &iValue, TRUE)) {
    StringCchPrintf(tch,COUNTOF(tch),L"; alpha:%i",iValue);
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),tch);
  }
  if (Style_StrGetAlpha(lpszStyle, &iValue, FALSE)) {
    StringCchPrintf(tch, COUNTOF(tch), L"; alpha2:%i", iValue);
    StringCchCat(szNewStyle, COUNTOF(szNewStyle), tch);
  }

  if (StrStrI(lpszStyle,L"block"))
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),L"; block");

  if (StrStrI(lpszStyle,L"noblink"))
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),L"; noblink");

  iValue = -1;
  if (Style_GetIndicatorType(lpszStyle, cchStyle, &iValue)) {
    StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; ");
    Style_GetIndicatorType(tch, COUNTOF(tch), &iValue);
    StringCchCat(szNewStyle, COUNTOF(szNewStyle), tch);
  }

  StrTrim(szNewStyle, L" ;");
  StringCchCopyN(lpszStyle,cchStyle,szNewStyle,cchStyle);
  return TRUE;
}


//=============================================================================
//
//  Style_SetStyles()
//
void Style_SetStyles(HWND hwnd,int iStyle,LPCWSTR lpszStyle)
{
  WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
  int  iValue;

  // Font
  char chFont[64] = { '\0' };
  if (Style_StrGetFont(lpszStyle, tch, COUNTOF(tch))) {
    WideCharToMultiByteStrg(CP_UTF8, tch, chFont);
    SendMessage(hwnd, SCI_STYLESETFONT, iStyle, (LPARAM)chFont);
  }

  // Size
  if (Style_StrGetSize(lpszStyle, &iValue)) {
    SendMessage(hwnd, SCI_STYLESETSIZE, iStyle, (LPARAM)iValue);
    //or Fractional
    //SendMessage(hwnd, SCI_STYLESETSIZEFRACTIONAL, iStyle, (LPARAM)(iValue * SC_FONT_SIZE_MULTIPLIER));
  }

  // Fore
  if (Style_StrGetColor(TRUE,lpszStyle,&iValue))
    SendMessage(hwnd,SCI_STYLESETFORE,iStyle,(LPARAM)iValue);

  // Back
  if (Style_StrGetColor(FALSE,lpszStyle,&iValue))
    SendMessage(hwnd,SCI_STYLESETBACK,iStyle,(LPARAM)iValue);

  //// Bold
  //if (StrStrI(lpszStyle,L"bold") != NULL)
  //  SendMessage(hwnd,SCI_STYLESETBOLD,iStyle,(LPARAM)TRUE);

  // Weight
  if (StrStrI(lpszStyle, L"thin"))
    SendMessage(hwnd, SCI_STYLESETWEIGHT, iStyle, (LPARAM)FW_THIN);
  else if (StrStrI(lpszStyle, L"extralight"))
    SendMessage(hwnd, SCI_STYLESETWEIGHT, iStyle, (LPARAM)FW_EXTRALIGHT);
  else if (StrStrI(lpszStyle, L"light"))
    SendMessage(hwnd, SCI_STYLESETWEIGHT, iStyle, (LPARAM)FW_LIGHT);
  else if (StrStrI(lpszStyle, L"normal"))
    SendMessage(hwnd, SCI_STYLESETWEIGHT, iStyle, (LPARAM)FW_NORMAL);
  else if (StrStrI(lpszStyle, L"medium"))
    SendMessage(hwnd, SCI_STYLESETWEIGHT, iStyle, (LPARAM)FW_MEDIUM);
  else if (StrStrI(lpszStyle, L"semibold"))
    SendMessage(hwnd, SCI_STYLESETWEIGHT, iStyle, (LPARAM)FW_SEMIBOLD);
  else if (StrStrI(lpszStyle, L"bold"))
    SendMessage(hwnd, SCI_STYLESETWEIGHT, iStyle, (LPARAM)FW_BOLD);
  else if (StrStrI(lpszStyle, L"extrabold"))
    SendMessage(hwnd, SCI_STYLESETWEIGHT, iStyle, (LPARAM)FW_EXTRABOLD);
  else if (StrStrI(lpszStyle, L"heavy"))
    SendMessage(hwnd, SCI_STYLESETWEIGHT, iStyle, (LPARAM)FW_HEAVY);

  // Italic
  if (StrStrI(lpszStyle,L"italic") != NULL)
    SendMessage(hwnd,SCI_STYLESETITALIC,iStyle,(LPARAM)TRUE);

  // Underline
  if (StrStrI(lpszStyle,L"underline") != NULL)
    SendMessage(hwnd,SCI_STYLESETUNDERLINE,iStyle,(LPARAM)TRUE);

  // EOL Filled
  if (StrStrI(lpszStyle,L"eolfilled") != NULL)
    SendMessage(hwnd,SCI_STYLESETEOLFILLED,iStyle,(LPARAM)TRUE);

  // Case
  if (Style_StrGetCase(lpszStyle,&iValue))
    SendMessage(hwnd,SCI_STYLESETCASE,iStyle,(LPARAM)iValue);

  // Character Set
  if (Style_StrGetCharSet(lpszStyle,&iValue))
    SendMessage(hwnd,SCI_STYLESETCHARACTERSET,iStyle,(LPARAM)iValue);
}


//=============================================================================
//
//  Style_SetFontQuality()
//
void Style_SetFontQuality(HWND hwnd,LPCWSTR lpszStyle) {

  WPARAM wQuality = (WPARAM)FontQuality[iSciFontQuality];
  WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };

  if (Style_StrGetFontQuality(lpszStyle,tch,COUNTOF(tch))) {
    if (StringCchCompareIN(tch,COUNTOF(tch),L"default",-1) == 0)
      wQuality = SC_EFF_QUALITY_DEFAULT;
    else if (StringCchCompareIN(tch,COUNTOF(tch),L"none",-1) == 0)
      wQuality = SC_EFF_QUALITY_NON_ANTIALIASED;
    else if (StringCchCompareIN(tch,COUNTOF(tch),L"standard",-1) == 0)
      wQuality = SC_EFF_QUALITY_ANTIALIASED;
    else if (StringCchCompareIN(tch,COUNTOF(tch),L"cleartype",-1) == 0)
      wQuality = SC_EFF_QUALITY_LCD_OPTIMIZED;
  }
  else if (wQuality == SC_EFF_QUALITY_DEFAULT) { 
    // undefined, use general settings, except for special fonts
    if (Style_StrGetFont(lpszStyle,tch,COUNTOF(tch))) {
      if (StringCchCompareIN(tch,COUNTOF(tch),L"Calibri",-1) == 0 ||
          StringCchCompareIN(tch,COUNTOF(tch),L"Cambria",-1) == 0 ||
          StringCchCompareIN(tch,COUNTOF(tch),L"Candara",-1) == 0 ||
          StringCchCompareIN(tch,COUNTOF(tch),L"Consolas",-1) == 0 ||
          StringCchCompareIN(tch,COUNTOF(tch),L"Constantia",-1) == 0 ||
          StringCchCompareIN(tch,COUNTOF(tch),L"Corbel",-1) == 0 ||
          StringCchCompareIN(tch,COUNTOF(tch),L"Segoe UI",-1) == 0 ||
          StringCchCompareIN(tch,COUNTOF(tch),L"Source Code Pro",-1) == 0)
        wQuality = SC_EFF_QUALITY_LCD_OPTIMIZED;
    }
  }
  SendMessage(hwnd,SCI_SETFONTQUALITY,wQuality,0);
}


//=============================================================================
//
//  Style_GetCurrentLexerName()
//
void Style_GetCurrentLexerName(LPWSTR lpszName,int cchName)
{
  if (!GetString(pLexCurrent->rid,lpszName,cchName))
    StringCchCopyN(lpszName,cchName,pLexCurrent->pszName,cchName);
}


//=============================================================================
//
//  Style_GetLexerIconId()
//
int Style_GetLexerIconId(PEDITLEXER plex)
{
  WCHAR pszFile[MAX_PATH + BUFZIZE_STYLE_EXTENTIONS];

  WCHAR *pszExtensions;
  if (StringCchLenW(plex->szExtensions,COUNTOF(plex->szExtensions)))
    pszExtensions = plex->szExtensions;
  else
    pszExtensions = plex->pszDefExt;

  StringCchCopy(pszFile,COUNTOF(pszFile),L"*.");
  StringCchCat(pszFile,COUNTOF(pszFile),pszExtensions);

  WCHAR *p = StrChr(pszFile, L';');
  if (p) { *p = L'\0'; }

  // check for ; at beginning
  if (StringCchLen(pszFile, COUNTOF(pszFile)) < 3)
    StringCchCat(pszFile, COUNTOF(pszFile),L"txt");

  SHFILEINFO shfi;
  ZeroMemory(&shfi,sizeof(SHFILEINFO));

  SHGetFileInfo(pszFile,FILE_ATTRIBUTE_NORMAL,&shfi,sizeof(SHFILEINFO),
    SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES);

  return (shfi.iIcon);
}


//=============================================================================
//
//  Style_AddLexerToTreeView()
//
HTREEITEM Style_AddLexerToTreeView(HWND hwnd,PEDITLEXER plex)
{
  int i = 0;
  WCHAR tch[MIDSZ_BUFFER] = { L'\0' };

  HTREEITEM hTreeNode;

  TVINSERTSTRUCT tvis;
  ZeroMemory(&tvis,sizeof(TVINSERTSTRUCT));

  tvis.hInsertAfter = TVI_LAST;

  tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
  if (GetString(plex->rid,tch,COUNTOF(tch)))
    tvis.item.pszText = tch;
  else
    tvis.item.pszText = plex->pszName;
  tvis.item.iImage = Style_GetLexerIconId(plex);
  tvis.item.iSelectedImage = tvis.item.iImage;
  tvis.item.lParam = (LPARAM)plex;

  hTreeNode = TreeView_InsertItem(hwnd,&tvis);

  tvis.hParent = hTreeNode;

  tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
  //tvis.item.iImage = -1;
  //tvis.item.iSelectedImage = -1;

  while (plex->Styles[i].iStyle != -1) {

    if (GetString(plex->Styles[i].rid,tch,COUNTOF(tch)))
      tvis.item.pszText = tch;
    else
      tvis.item.pszText = plex->Styles[i].pszName;
    tvis.item.lParam = (LPARAM)(&plex->Styles[i]);
    TreeView_InsertItem(hwnd,&tvis);
    i++;
  }

  return hTreeNode;
}


//=============================================================================
//
//  Style_AddLexerToListView()
//
void Style_AddLexerToListView(HWND hwnd,PEDITLEXER plex)
{
  WCHAR tch[MIDSZ_BUFFER] = { L'\0' };
  LVITEM lvi;
  ZeroMemory(&lvi,sizeof(LVITEM));

  lvi.mask = LVIF_IMAGE | LVIF_PARAM | LVIF_TEXT;
  lvi.iItem = ListView_GetItemCount(hwnd);
  if (GetString(plex->rid,tch,COUNTOF(tch)))
    lvi.pszText = tch;
  else
    lvi.pszText = plex->pszName;
  lvi.iImage = Style_GetLexerIconId(plex);
  lvi.lParam = (LPARAM)plex;

  ListView_InsertItem(hwnd,&lvi);
}


//=============================================================================
//
//  Style_ConfigDlgProc()
//
INT_PTR CALLBACK Style_ConfigDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  static HWND hwndTV;
  static BOOL fDragging;
  static PEDITLEXER pCurrentLexer;
  static PEDITSTYLE pCurrentStyle;
  static HFONT hFontTitle;
  static HBRUSH hbrFore;
  static HBRUSH hbrBack;

  switch(umsg)
  {

    case WM_INITDIALOG:
      {
        int i;
        SHFILEINFO shfi;
        LOGFONT lf;
        HTREEITEM currentLex = NULL;
        int found = 0;

        hwndTV = GetDlgItem(hwnd,IDC_STYLELIST);
        fDragging = FALSE;

        TreeView_SetImageList(hwndTV,
          (HIMAGELIST)SHGetFileInfo(L"C:\\",0,&shfi,sizeof(SHFILEINFO),
          SHGFI_SMALLICON | SHGFI_SYSICONINDEX),TVSIL_NORMAL);

        // Add lexers
        for (i = 0; i < COUNTOF(pLexArray); i++)
        {
          if (!found && (StringCchCompareX(pLexArray[i]->pszName,pLexCurrent->pszName) == 0))
          {
              found = 1;
              currentLex = Style_AddLexerToTreeView(hwndTV,pLexArray[i]);
          }
          else
              Style_AddLexerToTreeView(hwndTV,pLexArray[i]);
        }

        pCurrentStyle = NULL;

        //SetExplorerTheme(hwndTV);
        //TreeView_Expand(hwndTV,TreeView_GetRoot(hwndTV),TVE_EXPAND);
        TreeView_Select(hwndTV,currentLex,TVGN_CARET);

        SendDlgItemMessage(hwnd,IDC_STYLEEDIT,EM_LIMITTEXT, max(BUFSIZE_STYLE_VALUE, BUFZIZE_STYLE_EXTENTIONS)-1,0);

        MakeBitmapButton(hwnd,IDC_PREVSTYLE,g_hInstance,IDB_PREV);
        MakeBitmapButton(hwnd,IDC_NEXTSTYLE,g_hInstance,IDB_NEXT);

        // Setup title font
        if (hFontTitle)
          DeleteObject(hFontTitle);
        if (NULL == (hFontTitle = (HFONT)SendDlgItemMessage(hwnd,IDC_TITLE,WM_GETFONT,0,0)))
          hFontTitle = GetStockObject(DEFAULT_GUI_FONT);
        GetObject(hFontTitle,sizeof(LOGFONT),&lf);
        lf.lfHeight += lf.lfHeight / 5;
        lf.lfWeight = FW_BOLD;
        hFontTitle = CreateFontIndirect(&lf);
        SendDlgItemMessage(hwnd,IDC_TITLE,WM_SETFONT,(WPARAM)hFontTitle,TRUE);

        CenterDlgInParent(hwnd);
      }
      return TRUE;


    case WM_DESTROY:
      DeleteBitmapButton(hwnd,IDC_STYLEFORE);
      DeleteBitmapButton(hwnd,IDC_STYLEBACK);
      DeleteBitmapButton(hwnd,IDC_PREVSTYLE);
      DeleteBitmapButton(hwnd,IDC_NEXTSTYLE);
      return FALSE;


    case WM_NOTIFY:

      if (((LPNMHDR)(lParam))->idFrom == IDC_STYLELIST)
      {
        LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW)lParam;

        switch (lpnmtv->hdr.code)
        {

          case TVN_SELCHANGED:
            {
              if (pCurrentStyle)
                GetDlgItemText(hwnd,IDC_STYLEEDIT,pCurrentStyle->szValue,COUNTOF(pCurrentStyle->szValue));

              else if (pCurrentLexer) {
                WCHAR szBuf[BUFZIZE_STYLE_EXTENTIONS] = { L'\0' };
                if (GetDlgItemText(hwnd, IDC_STYLEEDIT, szBuf, COUNTOF(szBuf))) 
                  StringCchCopy(pCurrentLexer->szExtensions,COUNTOF(pCurrentLexer->szExtensions),szBuf);
              }

              // a lexer has been selected
              if (!TreeView_GetParent(hwndTV,lpnmtv->itemNew.hItem))
              {
                WCHAR wch[max(BUFSIZE_STYLE_VALUE, BUFZIZE_STYLE_EXTENTIONS)] = { L'\0' };

                GetDlgItemText(hwnd,IDC_STYLELABELS,wch,COUNTOF(wch));
                if (StrChr(wch,L'|')) *StrChr(wch,L'|') = L'\0';

                pCurrentStyle = 0;
                pCurrentLexer = (PEDITLEXER)lpnmtv->itemNew.lParam;
                if (pCurrentLexer)
                {
                  SetDlgItemText(hwnd,IDC_STYLELABEL,wch);
                  DialogEnableWindow(hwnd,IDC_STYLEEDIT,TRUE);
                  DialogEnableWindow(hwnd,IDC_STYLEFONT,FALSE);
                  DialogEnableWindow(hwnd,IDC_STYLEFORE,FALSE);
                  DialogEnableWindow(hwnd,IDC_STYLEBACK,FALSE);
                  DialogEnableWindow(hwnd,IDC_STYLEDEFAULT,TRUE);
                  //DialogEnableWindow(hwnd,IDC_STYLEBOLD,FALSE);
                  //DialogEnableWindow(hwnd,IDC_STYLEITALIC,FALSE);
                  //DialogEnableWindow(hwnd,IDC_STYLEUNDERLINE,FALSE);
                  //DialogEnableWindow(hwnd,IDC_STYLEEOLFILLED,FALSE);
                  //CheckDlgButton(hwnd,IDC_STYLEBOLD,BST_UNCHECKED);
                  //CheckDlgButton(hwnd,IDC_STYLEITALIC,BST_UNCHECKED);
                  //CheckDlgButton(hwnd,IDC_STYLEUNDERLINE,BST_UNCHECKED);
                  //CheckDlgButton(hwnd,IDC_STYLEEOLFILLED,BST_UNCHECKED);
                  SetDlgItemText(hwnd,IDC_STYLEEDIT,pCurrentLexer->szExtensions);
                }
                else
                {
                  SetDlgItemText(hwnd,IDC_STYLELABEL,L"");
                  DialogEnableWindow(hwnd,IDC_STYLEEDIT,FALSE);
                  DialogEnableWindow(hwnd,IDC_STYLEFONT,FALSE);
                  DialogEnableWindow(hwnd,IDC_STYLEFORE,FALSE);
                  DialogEnableWindow(hwnd,IDC_STYLEBACK,FALSE);
                  DialogEnableWindow(hwnd,IDC_STYLEDEFAULT,FALSE);
                  //DialogEnableWindow(hwnd,IDC_STYLEBOLD,FALSE);
                  //DialogEnableWindow(hwnd,IDC_STYLEITALIC,FALSE);
                  //DialogEnableWindow(hwnd,IDC_STYLEUNDERLINE,FALSE);
                  //DialogEnableWindow(hwnd,IDC_STYLEEOLFILLED,FALSE);
                  //CheckDlgButton(hwnd,IDC_STYLEBOLD,BST_UNCHECKED);
                  //CheckDlgButton(hwnd,IDC_STYLEITALIC,BST_UNCHECKED);
                  //CheckDlgButton(hwnd,IDC_STYLEUNDERLINE,BST_UNCHECKED);
                  //CheckDlgButton(hwnd,IDC_STYLEEOLFILLED,BST_UNCHECKED);
                  SetDlgItemText(hwnd,IDC_STYLEEDIT,L"");
                }
              }

              // a style has been selected
              else
              {
                WCHAR wch[BUFSIZE_STYLE_VALUE] = { L'\0' };

                GetDlgItemText(hwnd,IDC_STYLELABELS,wch,COUNTOF(wch));
                if (StrChr(wch,L'|')) *StrChr(wch,L'|') = L'\0';


                pCurrentLexer = 0;
                pCurrentStyle = (PEDITSTYLE)lpnmtv->itemNew.lParam;
                if (pCurrentStyle)
                {
                  SetDlgItemText(hwnd,IDC_STYLELABEL,StrEnd(wch)+1);
                  DialogEnableWindow(hwnd,IDC_STYLEEDIT,TRUE);
                  DialogEnableWindow(hwnd,IDC_STYLEFONT,TRUE);
                  DialogEnableWindow(hwnd,IDC_STYLEFORE,TRUE);
                  DialogEnableWindow(hwnd,IDC_STYLEBACK,TRUE);
                  DialogEnableWindow(hwnd,IDC_STYLEDEFAULT,TRUE);
                  SetDlgItemText(hwnd,IDC_STYLEEDIT,pCurrentStyle->szValue);
                }
                else
                {
                  SetDlgItemText(hwnd,IDC_STYLELABEL,L"");
                  DialogEnableWindow(hwnd,IDC_STYLEEDIT,FALSE);
                  DialogEnableWindow(hwnd,IDC_STYLEFONT,FALSE);
                  DialogEnableWindow(hwnd,IDC_STYLEFORE,FALSE);
                  DialogEnableWindow(hwnd,IDC_STYLEBACK,FALSE);
                  DialogEnableWindow(hwnd,IDC_STYLEDEFAULT,FALSE);
                  SetDlgItemText(hwnd,IDC_STYLEEDIT,L"");
                }
              }
            }
            break;

          case TVN_BEGINDRAG:
            {
              //HIMAGELIST himl;

              //if (pCurrentStyle)
              //  GetDlgItemText(hwnd,IDC_STYLEEDIT,pCurrentStyle->szValue,COUNTOF(pCurrentStyle->szValue));
              TreeView_Select(hwndTV,lpnmtv->itemNew.hItem,TVGN_CARET);

              //himl = TreeView_CreateDragImage(hwndTV,lpnmtv->itemNew.hItem);
              //ImageList_BeginDrag(himl,0,0,0);
              //ImageList_DragEnter(hwndTV,lpnmtv->ptDrag.x,lpnmtv->ptDrag.y);
              if (pCurrentStyle)
                DestroyCursor(SetCursor(LoadCursor(g_hInstance,MAKEINTRESOURCE(IDC_COPY))));
              else
                DestroyCursor(SetCursor(LoadCursor(NULL,IDC_NO)));

              SetCapture(hwnd);
              fDragging = TRUE;
            }

        }
      }

      break;


    case WM_MOUSEMOVE:
      {
        HTREEITEM htiTarget;
        TVHITTESTINFO tvht;

        if (fDragging && pCurrentStyle)
        {
          LONG xCur = LOWORD(lParam);
          LONG yCur = HIWORD(lParam);

          //ImageList_DragMove(xCur,yCur);
          //ImageList_DragShowNolock(FALSE);

          tvht.pt.x = xCur;
          tvht.pt.y = yCur;

          //ClientToScreen(hwnd,&tvht.pt);
          //ScreenToClient(hwndTV,&tvht.pt);
          MapWindowPoints(hwnd,hwndTV,&tvht.pt,1);

          if ((htiTarget = TreeView_HitTest(hwndTV,&tvht)) != NULL &&
               TreeView_GetParent(hwndTV,htiTarget) != NULL)
          {
            TreeView_SelectDropTarget(hwndTV,htiTarget);
            //TreeView_Expand(hwndTV,htiTarget,TVE_EXPAND);
            TreeView_EnsureVisible(hwndTV,htiTarget);
          }
          else
            TreeView_SelectDropTarget(hwndTV,NULL);

          //ImageList_DragShowNolock(TRUE);
        }
      }
      break;


    case WM_LBUTTONUP:
      {
        if (fDragging)
        {
          HTREEITEM htiTarget;

          //ImageList_EndDrag();
          htiTarget = TreeView_GetDropHilight(hwndTV);
          if (htiTarget)
          {
            WCHAR tchCopy[max(BUFSIZE_STYLE_VALUE, BUFZIZE_STYLE_EXTENTIONS)] = { L'\0' };
            TreeView_SelectDropTarget(hwndTV,NULL);
            GetDlgItemText(hwnd,IDC_STYLEEDIT,tchCopy,COUNTOF(tchCopy));
            TreeView_Select(hwndTV,htiTarget,TVGN_CARET);

            // after select, this is new current item
            if (pCurrentStyle)
            {
              StringCchCopy(pCurrentStyle->szValue,COUNTOF(pCurrentStyle->szValue),tchCopy);
              SetDlgItemText(hwnd,IDC_STYLEEDIT,tchCopy);
            }
          }
          ReleaseCapture();
          DestroyCursor(SetCursor(LoadCursor(NULL,IDC_ARROW)));
          fDragging = FALSE;
        }
      }
      break;


    case WM_CANCELMODE:
      {
        if (fDragging)
        {
          //ImageList_EndDrag();
          TreeView_SelectDropTarget(hwndTV,NULL);
          ReleaseCapture();
          DestroyCursor(SetCursor(LoadCursor(NULL,IDC_ARROW)));
          fDragging = FALSE;
        }
      }
      break;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {

        case IDC_PREVSTYLE:
          if (TreeView_GetSelection(hwndTV))
            TreeView_Select(hwndTV,TreeView_GetPrevVisible(hwndTV,
              TreeView_GetSelection(hwndTV)),TVGN_CARET);
          PostMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)(GetDlgItem(hwnd,IDC_STYLEEDIT)),1);
          break;

        case IDC_NEXTSTYLE:
          if (TreeView_GetSelection(hwndTV))
            TreeView_Select(hwndTV,TreeView_GetNextVisible(hwndTV,
              TreeView_GetSelection(hwndTV)),TVGN_CARET);
          PostMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)(GetDlgItem(hwnd,IDC_STYLEEDIT)),1);
          break;

        case IDC_STYLEFONT:
          if (pCurrentStyle)
          {
            WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
            GetDlgItemText(hwnd,IDC_STYLEEDIT,tch,COUNTOF(tch));
            if (Style_SelectFont(hwnd,tch,COUNTOF(tch),
                                 StringCchCompareIX(pCurrentStyle->pszName,L"Default Style") == 0 ||
                                 StringCchCompareIX(pCurrentStyle->pszName,L"2nd Default Style") == 0)) {
              SetDlgItemText(hwnd,IDC_STYLEEDIT,tch);
            }
          }
          PostMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)(GetDlgItem(hwnd,IDC_STYLEEDIT)),1);
          break;

        case IDC_STYLEFORE:
          if (pCurrentStyle)
          {
            WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
            GetDlgItemText(hwnd,IDC_STYLEEDIT,tch,COUNTOF(tch));
            if (Style_SelectColor(hwnd,TRUE,tch,COUNTOF(tch))) {
              SetDlgItemText(hwnd,IDC_STYLEEDIT,tch);
            }
          }
          PostMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)(GetDlgItem(hwnd,IDC_STYLEEDIT)),1);
          break;

        case IDC_STYLEBACK:
          if (pCurrentStyle)
          {
            WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
            GetDlgItemText(hwnd,IDC_STYLEEDIT,tch,COUNTOF(tch));
            if (Style_SelectColor(hwnd,FALSE,tch,COUNTOF(tch))) {
              SetDlgItemText(hwnd,IDC_STYLEEDIT,tch);
            }
          }
          PostMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)(GetDlgItem(hwnd,IDC_STYLEEDIT)),1);
          break;

        case IDC_STYLEDEFAULT:
          if (pCurrentStyle)
          {
            StringCchCopy(pCurrentStyle->szValue,COUNTOF(pCurrentStyle->szValue),pCurrentStyle->pszDefault);
            SetDlgItemText(hwnd,IDC_STYLEEDIT,pCurrentStyle->szValue);
          }
          else if (pCurrentLexer) {
            StringCchCopy(pCurrentLexer->szExtensions,COUNTOF(pCurrentLexer->szExtensions),pCurrentLexer->pszDefExt);
            SetDlgItemText(hwnd,IDC_STYLEEDIT,pCurrentLexer->szExtensions);
          }
          PostMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)(GetDlgItem(hwnd,IDC_STYLEEDIT)),1);
          break;

        case IDC_STYLEEDIT:
          {
            if (HIWORD(wParam) == EN_CHANGE) {
              int cr;
              WCHAR tch[max(BUFSIZE_STYLE_VALUE, BUFZIZE_STYLE_EXTENTIONS)] = { L'\0' };

              GetDlgItemText(hwnd,IDC_STYLEEDIT,tch,COUNTOF(tch));

              cr = -1;
              Style_StrGetColor(TRUE,tch,&cr);
              MakeColorPickButton(hwnd,IDC_STYLEFORE,g_hInstance,cr);

              cr = -1;
              Style_StrGetColor(FALSE,tch,&cr);
              MakeColorPickButton(hwnd,IDC_STYLEBACK,g_hInstance,cr);
            }
          }
          break;

        case IDC_IMPORT:
          {
            hwndTV = GetDlgItem(hwnd,IDC_STYLELIST);

            if (pCurrentStyle)
              GetDlgItemText(hwnd,IDC_STYLEEDIT,pCurrentStyle->szValue,COUNTOF(pCurrentStyle->szValue));
            else if (pCurrentLexer) {
              if (!GetDlgItemText(hwnd,IDC_STYLEEDIT,pCurrentLexer->szExtensions,COUNTOF(pCurrentLexer->szExtensions)))
                StringCchCopy(pCurrentLexer->szExtensions,COUNTOF(pCurrentLexer->szExtensions),pCurrentLexer->pszDefExt);
            }

            if (Style_Import(hwnd)) {

              if (pCurrentStyle)
                SetDlgItemText(hwnd,IDC_STYLEEDIT,pCurrentStyle->szValue);
              else if (pCurrentLexer)
                SetDlgItemText(hwnd,IDC_STYLEEDIT,pCurrentLexer->szExtensions);

              TreeView_Select(hwndTV,TreeView_GetRoot(hwndTV),TVGN_CARET);
            }
          }
          break;

        case IDC_EXPORT:
          {
            if (pCurrentStyle)
              GetDlgItemText(hwnd,IDC_STYLEEDIT,pCurrentStyle->szValue,COUNTOF(pCurrentStyle->szValue));
            else if (pCurrentLexer) {
              if (!GetDlgItemText(hwnd,IDC_STYLEEDIT,pCurrentLexer->szExtensions,COUNTOF(pCurrentLexer->szExtensions)))
                StringCchCopy(pCurrentLexer->szExtensions,COUNTOF(pCurrentLexer->szExtensions),pCurrentLexer->pszDefExt);
            }

            Style_Export(hwnd);
          }
          break;

        case IDC_PREVIEW:
          {
            // Hack from outside
            extern HWND hwndEdit;

            if (pCurrentStyle)
              GetDlgItemText(hwnd,IDC_STYLEEDIT,pCurrentStyle->szValue,COUNTOF(pCurrentStyle->szValue));
            else if (pCurrentLexer) {
              if (!GetDlgItemText(hwnd,IDC_STYLEEDIT,pCurrentLexer->szExtensions,COUNTOF(pCurrentLexer->szExtensions)))
                StringCchCopy(pCurrentLexer->szExtensions,COUNTOF(pCurrentLexer->szExtensions),pCurrentLexer->pszDefExt);
            }
            Style_SetLexer(hwndEdit,pLexCurrent);
            UpdateLineNumberWidth();
          }
          break;

        case IDOK:
          if (pCurrentStyle)
            GetDlgItemText(hwnd,IDC_STYLEEDIT,pCurrentStyle->szValue,COUNTOF(pCurrentStyle->szValue));
          else if (pCurrentLexer) {
            if (!GetDlgItemText(hwnd,IDC_STYLEEDIT,pCurrentLexer->szExtensions,COUNTOF(pCurrentLexer->szExtensions)))
              StringCchCopy(pCurrentLexer->szExtensions,COUNTOF(pCurrentLexer->szExtensions),pCurrentLexer->pszDefExt);
          }
          EndDialog(hwnd,IDOK);
          break;

        case IDCANCEL:
          if (fDragging)
            SendMessage(hwnd,WM_CANCELMODE,0,0);
          else
            EndDialog(hwnd,IDCANCEL);
          break;

      }

      return TRUE;

  }

  return FALSE;

}


//=============================================================================
//
//  Style_ConfigDlg()
//
void Style_ConfigDlg(HWND hwnd)
{

  WCHAR* StyleBackup[NUMLEXERS * MAX_NUM_OF_STYLES_PER_LEXER];
  int c,cItems,i,iLexer;

  // Backup Styles
  c = 0;
  for (iLexer = 0; iLexer < COUNTOF(pLexArray); iLexer++) {
    StyleBackup[c++] = StrDup(pLexArray[iLexer]->szExtensions);
    i = 0;
    while (pLexArray[iLexer]->Styles[i].iStyle != -1) {
      StyleBackup[c++] = StrDup(pLexArray[iLexer]->Styles[i].szValue);
      i++;
    }
  }
  cItems = c;

  if (IDCANCEL == ThemedDialogBoxParam(g_hInstance,
                    MAKEINTRESOURCE(IDD_STYLECONFIG),
                    GetParent(hwnd),Style_ConfigDlgProc,
                    (LPARAM)&StyleBackup))
  {
    // Restore Styles
    c = 0;
    for (iLexer = 0; iLexer < COUNTOF(pLexArray); iLexer++) {
      StringCchCopy(pLexArray[iLexer]->szExtensions,COUNTOF(pLexArray[iLexer]->szExtensions),StyleBackup[c++]);
      i = 0;
      while (pLexArray[iLexer]->Styles[i].iStyle != -1) {
        StringCchCopy(pLexArray[iLexer]->Styles[i].szValue,COUNTOF(pLexArray[iLexer]->Styles[i].szValue),StyleBackup[c++]);
        i++;
      }
    }
  }
  else {
    fStylesModified = TRUE;
    if (!fWarnedNoIniFile && (StringCchLenW(szIniFile,COUNTOF(szIniFile)) == 0)) {
      MsgBox(MBWARN,IDS_SETTINGSNOTSAVED);
      fWarnedNoIniFile = TRUE;
    }
  }

  for (c = 0; c < cItems; c++) {
    LocalFree(StyleBackup[c]);
  }
  // Apply new (or previous) Styles
  Style_SetLexer(hwnd,pLexCurrent);
}


//=============================================================================
//
//  Style_SelectLexerDlgProc()
//
INT_PTR CALLBACK Style_SelectLexerDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  static int cxClient;
  static int cyClient;
  static int mmiPtMaxY;
  static int mmiPtMinX;

  static HWND hwndLV;
  static int  iInternalDefault;

  switch(umsg)
  {

    case WM_INITDIALOG:
      {
        int i;
        int lvItems;
        LVITEM lvi;
        SHFILEINFO shfi;
        LVCOLUMN lvc = { LVCF_FMT|LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0 };

        RECT rc;
        WCHAR tch[MAX_PATH] = { L'\0' };
        int cGrip;

        GetClientRect(hwnd,&rc);
        cxClient = rc.right - rc.left;
        cyClient = rc.bottom - rc.top;

        AdjustWindowRectEx(&rc,GetWindowLong(hwnd,GWL_STYLE)|WS_THICKFRAME,FALSE,0);
        mmiPtMinX = rc.right-rc.left;
        mmiPtMaxY = rc.bottom-rc.top;

        if (cxStyleSelectDlg < (rc.right-rc.left))
          cxStyleSelectDlg = rc.right-rc.left;
        if (cyStyleSelectDlg < (rc.bottom-rc.top))
          cyStyleSelectDlg = rc.bottom-rc.top;
        SetWindowPos(hwnd,NULL,rc.left,rc.top,cxStyleSelectDlg,cyStyleSelectDlg,SWP_NOZORDER);

        SetWindowLongPtr(hwnd,GWL_STYLE,GetWindowLongPtr(hwnd,GWL_STYLE)|WS_THICKFRAME);
        SetWindowPos(hwnd,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);

        GetMenuString(GetSystemMenu(GetParent(hwnd),FALSE),SC_SIZE,tch,COUNTOF(tch),MF_BYCOMMAND);
        InsertMenu(GetSystemMenu(hwnd,FALSE),SC_CLOSE,MF_BYCOMMAND|MF_STRING|MF_ENABLED,SC_SIZE,tch);
        InsertMenu(GetSystemMenu(hwnd,FALSE),SC_CLOSE,MF_BYCOMMAND|MF_SEPARATOR,0,NULL);

        SetWindowLongPtr(GetDlgItem(hwnd,IDC_RESIZEGRIP3),GWL_STYLE,
          GetWindowLongPtr(GetDlgItem(hwnd,IDC_RESIZEGRIP3),GWL_STYLE)|SBS_SIZEGRIP|WS_CLIPSIBLINGS);

        cGrip = GetSystemMetrics(SM_CXHTHUMB);
        SetWindowPos(GetDlgItem(hwnd,IDC_RESIZEGRIP3),NULL,cxClient-cGrip,
                     cyClient-cGrip,cGrip,cGrip,SWP_NOZORDER);

        hwndLV = GetDlgItem(hwnd,IDC_STYLELIST);

        ListView_SetImageList(hwndLV,
          (HIMAGELIST)SHGetFileInfo(L"C:\\",0,&shfi,sizeof(SHFILEINFO),SHGFI_SMALLICON | SHGFI_SYSICONINDEX),
          LVSIL_SMALL);

        ListView_SetImageList(hwndLV,
          (HIMAGELIST)SHGetFileInfo(L"C:\\",0,&shfi,sizeof(SHFILEINFO),SHGFI_LARGEICON | SHGFI_SYSICONINDEX),
          LVSIL_NORMAL);

        //SetExplorerTheme(hwndLV);
        ListView_SetExtendedListViewStyle(hwndLV,/*LVS_EX_FULLROWSELECT|*/LVS_EX_DOUBLEBUFFER|LVS_EX_LABELTIP);
        ListView_InsertColumn(hwndLV,0,&lvc);

        // Add lexers
        for (i = 0; i < COUNTOF(pLexArray); i++) {
          Style_AddLexerToListView(hwndLV, pLexArray[i]);
        }
        ListView_SetColumnWidth(hwndLV,0,LVSCW_AUTOSIZE_USEHEADER);

        // Select current lexer
        lvItems = ListView_GetItemCount(hwndLV);
        lvi.mask = LVIF_PARAM;
        for (i = 0; i < lvItems; i++) {
          lvi.iItem = i;
          ListView_GetItem(hwndLV,&lvi);;
          if (StringCchCompareX(((PEDITLEXER)lvi.lParam)->pszName,pLexCurrent->pszName) == 0) {
            ListView_SetItemState(hwndLV,i,LVIS_FOCUSED|LVIS_SELECTED,LVIS_FOCUSED|LVIS_SELECTED);
            ListView_EnsureVisible(hwndLV,i,FALSE);
            if (iDefaultLexer == i) {
              CheckDlgButton(hwnd,IDC_DEFAULTSCHEME,BST_CHECKED);
            }
          }
        }

        iInternalDefault = iDefaultLexer;

        if (bAutoSelect)
          CheckDlgButton(hwnd,IDC_AUTOSELECT,BST_CHECKED);

        CenterDlgInParent(hwnd);
      }
      return TRUE;


    case WM_DESTROY:
      {
        RECT rc;

        GetWindowRect(hwnd,&rc);
        cxStyleSelectDlg = rc.right-rc.left;
        cyStyleSelectDlg = rc.bottom-rc.top;
      }
      return FALSE;


    case WM_SIZE:
      {
        RECT rc;

        int dxClient = LOWORD(lParam) - cxClient;
        int dyClient = HIWORD(lParam) - cyClient;
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);

        GetWindowRect(GetDlgItem(hwnd,IDC_RESIZEGRIP3),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDC_RESIZEGRIP3),NULL,rc.left+dxClient,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDC_RESIZEGRIP3),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDOK),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDOK),NULL,rc.left+dxClient,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDOK),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDCANCEL),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDCANCEL),NULL,rc.left+dxClient,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDCANCEL),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDC_STYLELIST),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDC_STYLELIST),NULL,0,0,rc.right-rc.left+dxClient,rc.bottom-rc.top+dyClient,SWP_NOZORDER|SWP_NOMOVE);
        ListView_SetColumnWidth(GetDlgItem(hwnd,IDC_STYLELIST),0,LVSCW_AUTOSIZE_USEHEADER);
        InvalidateRect(GetDlgItem(hwnd,IDC_STYLELIST),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDC_AUTOSELECT),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDC_AUTOSELECT),NULL,rc.left,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDC_AUTOSELECT),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDC_DEFAULTSCHEME),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDC_DEFAULTSCHEME),NULL,rc.left,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDC_DEFAULTSCHEME),NULL,TRUE);
      }
      return TRUE;


    case WM_GETMINMAXINFO:
      {
        LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
        lpmmi->ptMinTrackSize.x = mmiPtMinX;
        lpmmi->ptMinTrackSize.y = mmiPtMaxY;
        //lpmmi->ptMaxTrackSize.y = mmiPtMaxY;
      }
      return TRUE;


    case WM_NOTIFY: {
      if (((LPNMHDR)(lParam))->idFrom == IDC_STYLELIST) {

      switch (((LPNMHDR)(lParam))->code) {

        case NM_DBLCLK:
          SendMessage(hwnd,WM_COMMAND,MAKELONG(IDOK,1),0);
          break;

        case LVN_ITEMCHANGED:
        case LVN_DELETEITEM: {
          int i = ListView_GetNextItem(hwndLV,-1,LVNI_ALL | LVNI_SELECTED);
          if (iInternalDefault == i)
            CheckDlgButton(hwnd,IDC_DEFAULTSCHEME,BST_CHECKED);
          else
            CheckDlgButton(hwnd,IDC_DEFAULTSCHEME,BST_UNCHECKED);
          DialogEnableWindow(hwnd,IDC_DEFAULTSCHEME,i != -1);
          DialogEnableWindow(hwnd,IDOK,i != -1);
          }
          break;
        }
      }
    }

      return TRUE;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {

        case IDC_DEFAULTSCHEME:
          if (IsDlgButtonChecked(hwnd,IDC_DEFAULTSCHEME) == BST_CHECKED)
            iInternalDefault = ListView_GetNextItem(hwndLV,-1,LVNI_ALL | LVNI_SELECTED);
          else
            iInternalDefault = 0;
          break;


        case IDOK:
          {
            LVITEM lvi;

            lvi.mask = LVIF_PARAM;
            lvi.iItem = ListView_GetNextItem(hwndLV,-1,LVNI_ALL | LVNI_SELECTED);
            if (ListView_GetItem(hwndLV,&lvi)) {
              pLexCurrent = (PEDITLEXER)lvi.lParam;
              iDefaultLexer = iInternalDefault;
              bAutoSelect = (IsDlgButtonChecked(hwnd,IDC_AUTOSELECT) == BST_CHECKED) ? 1 : 0;
              EndDialog(hwnd,IDOK);
            }
          }
          break;


        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;

      }

      return TRUE;

  }

  return FALSE;

}


//=============================================================================
//
//  Style_SelectLexerDlg()
//
void Style_SelectLexerDlg(HWND hwnd)
{
  if (IDOK == ThemedDialogBoxParam(g_hInstance,
                MAKEINTRESOURCE(IDD_STYLESELECT),
                GetParent(hwnd),Style_SelectLexerDlgProc,0))

    Style_SetLexer(hwnd,pLexCurrent);
}

// End of Styles.c
