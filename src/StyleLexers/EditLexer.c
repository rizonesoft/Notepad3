
#include <assert.h>
#include "Helpers.h"
#include "lexers_x/SciXLexer.h"

#include "SciCall.h"
#include "EditLexer.h"


void Lexer_GetStreamCommentStrgs(LPWSTR beg_out, LPWSTR end_out, size_t maxlen)
{
    if (beg_out && end_out && maxlen) {

        switch (SciCall_GetLexer()) {
        case SCLEX_AU3:
            StringCchCopy(beg_out, maxlen, L"#cs");
            StringCchCopy(end_out, maxlen, L"#ce");
            break;
        case SCLEX_AVS:
        case SCLEX_CPP:
        case SCLEX_CSS:
        case SCLEX_D: // L"/+", L"+/" could also be used instead
        case SCLEX_DART:
        case SCLEX_JSON:
        case SCLEX_KOTLIN:
        case SCLEX_NSIS:
        case SCLEX_PHPSCRIPT:
        case SCLEX_RUST:
        case SCLEX_SQL:
        case SCLEX_VHDL:
            StringCchCopy(beg_out, maxlen, L"/*");
            StringCchCopy(end_out, maxlen, L"*/");
            break;
        case SCLEX_HTML: {
            int const cStyleBeg = SciCall_GetStyleIndexAt(Sci_GetLineStartPosition(SciCall_GetSelectionStart()));
            int const cStyleEnd = SciCall_GetStyleIndexAt(SciCall_GetSelectionEnd());
            if (((min_i(cStyleBeg, cStyleEnd) >= SCE_HPHP_DEFAULT) && (max_i(cStyleBeg, cStyleEnd) <= SCE_HPHP_OPERATOR)) ||
                ((min_i(cStyleBeg, cStyleEnd) >= SCE_HJ_START) && (max_i(cStyleBeg, cStyleEnd) <= SCE_HJA_REGEX))) {
                StringCchCopy(beg_out, maxlen, L"/*");
                StringCchCopy(end_out, maxlen, L"*/");
                break;
            }
        }
        // [[fallthrough]] // -> XML
        case SCLEX_XML:
            StringCchCopy(beg_out, maxlen, L"<!--");
            StringCchCopy(end_out, maxlen, L"-->");
            break;
        case SCLEX_INNOSETUP:
        case SCLEX_PASCAL:
            StringCchCopy(beg_out, maxlen, L"{");
            StringCchCopy(end_out, maxlen, L"}");
            break;
        case SCLEX_LUA:
            StringCchCopy(beg_out, maxlen, L"--[[");
            StringCchCopy(end_out, maxlen, L"]]");
            break;
        case SCLEX_COFFEESCRIPT:
            StringCchCopy(beg_out, maxlen, L"###");
            StringCchCopy(end_out, maxlen, L"###");
            break;
        case SCLEX_MATLAB:
            StringCchCopy(beg_out, maxlen, L"%{");
            StringCchCopy(end_out, maxlen, L"%}");
            break;
        // ------------------
        case SCLEX_CONTAINER:
            assert("SciCall_GetLexer() UNDEFINED!" && 0);
        // ------------------
        case SCLEX_NULL:
        case SCLEX_AHK:
        case SCLEX_ASM:
        case SCLEX_BASH:
        case SCLEX_BATCH:
        case SCLEX_CMAKE:
        case SCLEX_CONF:
        case SCLEX_DIFF:
        case SCLEX_LATEX:
        case SCLEX_MAKEFILE:
        case SCLEX_MARKDOWN:
        case SCLEX_NIM:
        case SCLEX_PERL:
        case SCLEX_POWERSHELL:
        case SCLEX_PROPERTIES:
        case SCLEX_PYTHON:
        case SCLEX_R:
        case SCLEX_REGISTRY:
        case SCLEX_RUBY:
        case SCLEX_TCL:
        case SCLEX_TOML:
        case SCLEX_VB:
        case SCLEX_VBSCRIPT:
        case SCLEX_YAML:
        default:
            StringCchCopy(beg_out, maxlen, L"");
            StringCchCopy(end_out, maxlen, L"");
            break;
        }
    }
}


bool Lexer_GetLineCommentStrg(LPWSTR pre_out, size_t maxlen)
{
    if (pre_out && maxlen) {

        switch (SciCall_GetLexer()) {
        case SCLEX_CPP:
        case SCLEX_D:
        case SCLEX_DART:
        case SCLEX_JSON:
        case SCLEX_KOTLIN:
        case SCLEX_PASCAL:
        case SCLEX_RUST:
            StringCchCopy(pre_out, maxlen, L"//");
            return false;
        case SCLEX_VB:
        case SCLEX_VBSCRIPT:
            StringCchCopy(pre_out, maxlen, L"'");
            return false;
        case SCLEX_AVS:
        case SCLEX_BASH:
        case SCLEX_CMAKE:
        case SCLEX_COFFEESCRIPT:
        case SCLEX_CONF:
        case SCLEX_MAKEFILE:
        case SCLEX_NIM:
        case SCLEX_PERL:
        case SCLEX_PHPSCRIPT:
        case SCLEX_POWERSHELL:
        case SCLEX_PYTHON:
        case SCLEX_R:
        case SCLEX_RUBY:
        case SCLEX_TCL:
        case SCLEX_TOML:
        case SCLEX_YAML:
            StringCchCopy(pre_out, maxlen, L"#");
            return true;
        case SCLEX_AHK:
        case SCLEX_ASM:
        case SCLEX_AU3:
        case SCLEX_INNOSETUP:
        case SCLEX_NSIS: // "#" could also be used instead
        case SCLEX_PROPERTIES:
        case SCLEX_REGISTRY:
            StringCchCopy(pre_out, maxlen, L";");
            return true;
        case SCLEX_LUA:
        case SCLEX_SQL:
        case SCLEX_VHDL:
            StringCchCopy(pre_out, maxlen, L"--");
            return true;
        case SCLEX_BATCH: // "::" could also be used instead
            StringCchCopy(pre_out, maxlen, L"rem ");
            return true;
        case SCLEX_LATEX:
        case SCLEX_MATLAB:
            StringCchCopy(pre_out, maxlen, L"%");
            return true;
        case SCLEX_FORTRAN:
        case SCLEX_F77:
            StringCchCopy(pre_out, maxlen, L"!");
            return true;
        // ------------------
        case SCLEX_CONTAINER:
            assert("SciCall_GetLexer() UNDEFINED!" && 0);
        // ------------------
        case SCLEX_NULL:
        case SCLEX_CSS:
        case SCLEX_DIFF:
        case SCLEX_MARKDOWN:
        case SCLEX_HTML: {
            int const cStyleBeg = SciCall_GetStyleIndexAt(Sci_GetLineStartPosition(SciCall_GetSelectionStart()));
            int const cStyleEnd = SciCall_GetStyleIndexAt(SciCall_GetSelectionEnd());
            if (((min_i(cStyleBeg, cStyleEnd) >= SCE_HPHP_DEFAULT) && (max_i(cStyleBeg, cStyleEnd) <= SCE_HPHP_OPERATOR)) || (min_i(cStyleBeg, cStyleEnd) == SCE_HPHP_COMPLEX_VARIABLE) ||
                ((min_i(cStyleBeg, cStyleEnd) >= SCE_HJ_START) && (max_i(cStyleBeg, cStyleEnd) <= SCE_HJA_REGEX))) {
                StringCchCopy(pre_out, maxlen, L"//");
                return false;
            }
            if (((min_i(cStyleBeg, cStyleEnd) >= SCE_HP_START) && (max_i(cStyleBeg, cStyleEnd) <= SCE_HP_IDENTIFIER)) ||
                ((min_i(cStyleBeg, cStyleEnd) >= SCE_HPA_START) && (max_i(cStyleBeg, cStyleEnd) <= SCE_HPA_IDENTIFIER))) {
                StringCchCopy(pre_out, maxlen, L"#");
                return false;
            }
        }
        // [[fallthrough]] // -> XML
        case SCLEX_XML:
        default:
            StringCchCopy(pre_out, maxlen, L"");
            break;
        }
    }
    return false;
}


//=============================================================================
//
//  Lexer_SetFoldingAvailability()
//
void Lexer_SetFoldingAvailability(PEDITLEXER pLexer) {
    switch (pLexer->lexerID) {
    case SCLEX_NULL:
    case SCLEX_CONTAINER:
    case SCLEX_BATCH:
    case SCLEX_CONF:
    case SCLEX_MAKEFILE:
    case SCLEX_MARKDOWN:
        FocusedView.CodeFoldingAvailable = false;
        break;
    default:
        FocusedView.CodeFoldingAvailable = true;
        break;
    }
}

//=============================================================================
//
//  Lexer_SetFoldingProperties()
//
void Lexer_SetFoldingProperties(bool active) {
    if (active) {
        SciCall_SetProperty("fold", "1");
        SciCall_SetProperty("fold.comment", "1");
        SciCall_SetProperty("fold.compact", "0");
        SciCall_SetProperty("fold.foldsyntaxbased", "1");

        SciCall_SetProperty("fold.html", "1");
        SciCall_SetProperty("fold.preprocessor", "1");
        SciCall_SetProperty("fold.cpp.comment.explicit", "0");
    } else {
        SciCall_SetProperty("fold", "0");
    }
}

//=============================================================================
//
//  Lexer_SetFoldingFocusedView()
//
void Lexer_SetFoldingFocusedView() {
    SciCall_SetProperty("fold", "0"); // disable folding by lexers
    SciCall_SetProperty("fold.comment", "1");
    SciCall_SetProperty("fold.compact", "0");
}

//=============================================================================
//
//  void Lexer_SetLexerSpecificProperties()
//
void Lexer_SetLexerSpecificProperties(const int lexerId) {
    switch (lexerId) {
    case SCLEX_CPP:
        SciCall_SetProperty("styling.within.preprocessor", "1");
        SciCall_SetProperty("lexer.cpp.track.preprocessor", "0");
        SciCall_SetProperty("lexer.cpp.update.preprocessor", "0");
        break;

    case SCLEX_PASCAL:
        SciCall_SetProperty("lexer.pascal.smart.highlighting", "1");
        break;

    case SCLEX_SQL:
        SciCall_SetProperty("fold.sql.at.else", "1");
        //SciCall_SetProperty("fold.sql.only.begin", "1");
        SciCall_SetProperty("sql.backslash.escapes", "1");
        SciCall_SetProperty("lexer.sql.backticks.identifier", "1");
        SciCall_SetProperty("lexer.sql.numbersign.comment", Settings2.LexerSQLNumberSignAsComment ? "1" : "0");
        //SciCall_SetProperty("lexer.sql.allow.dotted.word", "0");
        break;

    case SCLEX_MARKDOWN:
        SciCall_SetProperty("lexer.markdown.header.eolfill", "1");
        break;

    case SCLEX_NSIS:
        SciCall_SetProperty("nsis.ignorecase", "1");
        break;

    case SCLEX_CSS:
        SciCall_SetProperty("lexer.css.scss.language", "1");
        SciCall_SetProperty("lexer.css.less.language", "1");
        break;

    case SCLEX_JSON:
        SciCall_SetProperty("lexer.json.allow.comments", "1");
        SciCall_SetProperty("lexer.json.escape.sequence", "1");
        break;

    case SCLEX_PYTHON:
        SciCall_SetProperty("tab.timmy.whinge.level", "1");
        SciCall_SetProperty("lexer.python.strings.f", "1");
        break;

    case SCLEX_VERILOG:
    case SCLEX_SYSVERILOG:
        SciCall_SetProperty("lexer.verilog.track.preprocessor", "1");
        SciCall_SetProperty("lexer.verilog.update.preprocessor", "1");
        SciCall_SetProperty("lexer.verilog.portstyling", "1");
        SciCall_SetProperty("lexer.verilog.allupperkeywords", "1");
        break;

    case SCLEX_XML:
        SciCall_SetProperty("lexer.xml.allow.scripts", "1");
        break;

    default:
        break;
    }
}
