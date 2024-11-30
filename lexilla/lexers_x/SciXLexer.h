// encoding: UTF-8
#pragma once

#include "SciLexer.h"    // Scintilla/Lexilla Lexer defines

#define SCLEX_AHK         200
#define SCLEX_CSV         201
#define SCLEX_KOTLIN      202
#define SCLEX_SYSVERILOG  203

// -----------------------------------------------------------------------------
// !!!!! ADD  Lexer Linkage in:  scintilla\src\Catalogue.cxx  !!!!!
// -----------------------------------------------------------------------------

#define SCE_AHK_DEFAULT 0
#define SCE_AHK_COMMENTLINE 1
#define SCE_AHK_COMMENTBLOCK 2
#define SCE_AHK_ESCAPE 3
#define SCE_AHK_SYNOPERATOR 4
#define SCE_AHK_EXPOPERATOR 5
#define SCE_AHK_STRING 6
#define SCE_AHK_NUMBER 7
#define SCE_AHK_IDENTIFIER 8
#define SCE_AHK_VARREF 9
#define SCE_AHK_LABEL 10
#define SCE_AHK_WORD_CF 11
#define SCE_AHK_WORD_CMD 12
#define SCE_AHK_WORD_FN 13
#define SCE_AHK_WORD_DIR 14
#define SCE_AHK_WORD_KB 15
#define SCE_AHK_WORD_VAR 16
#define SCE_AHK_WORD_SP 17
#define SCE_AHK_WORD_UD 18
#define SCE_AHK_VARREFKW 19
#define SCE_AHK_ERROR 20


#define SCE_CSV_DEFAULT 0
#define SCE_CSV_COLUMN_0 1
#define SCE_CSV_COLUMN_1 2
#define SCE_CSV_COLUMN_2 3
#define SCE_CSV_COLUMN_3 4
#define SCE_CSV_COLUMN_4 5
#define SCE_CSV_COLUMN_5 6
#define SCE_CSV_COLUMN_6 7
#define SCE_CSV_COLUMN_7 8
#define SCE_CSV_COLUMN_8 9
#define SCE_CSV_COLUMN_9 10


#define SCE_KOTLIN_DEFAULT 0
#define SCE_KOTLIN_COMMENTLINE 1
#define SCE_KOTLIN_COMMENTLINEDOC 2
#define SCE_KOTLIN_COMMENTBLOCK 3
#define SCE_KOTLIN_COMMENTBLOCKDOC 4
#define SCE_KOTLIN_COMMENTDOCWORD 5
#define SCE_KOTLIN_TASKMARKER 6
#define SCE_KOTLIN_NUMBER 7
#define SCE_KOTLIN_OPERATOR 8
#define SCE_KOTLIN_OPERATOR2 9
#define SCE_KOTLIN_CHARACTER 10
#define SCE_KOTLIN_STRING 11
#define SCE_KOTLIN_RAWSTRING 12
#define SCE_KOTLIN_ESCAPECHAR 13
#define SCE_KOTLIN_VARIABLE 14
#define SCE_KOTLIN_LABEL 15
#define SCE_KOTLIN_IDENTIFIER 16
#define SCE_KOTLIN_ANNOTATION 17
#define SCE_KOTLIN_BACKTICKS 18
#define SCE_KOTLIN_WORD 21
#define SCE_KOTLIN_CLASS 22
#define SCE_KOTLIN_INTERFACE 23
#define SCE_KOTLIN_ENUM 24
#define SCE_KOTLIN_FUNCTION 25
#define SCE_KOTLIN_FUNCTION_DEFINITION 26


//#define SCE_YAML_DEFAULT 0
//#define SCE_YAML_COMMENT 1
//#define SCE_YAML_IDENTIFIER 2
//#define SCE_YAML_KEYWORD 3
//#define SCE_YAML_NUMBER 4
//#define SCE_YAML_REFERENCE 5
//#define SCE_YAML_DOCUMENT 6
//#define SCE_YAML_TEXT 7
//#define SCE_YAML_ERROR 8
//#define SCE_YAML_OPERATOR 9
//#define SCE_YAML_DIRECTIVE 10
//#define SCE_YAML_STRING1 11
//#define SCE_YAML_STRING2 12
//#define SCE_YAML_ESCAPECHAR 13
//#define SCE_YAML_KEY 14
//#define SCE_YAML_BLOCK_SCALAR 15
//#define SCE_YAML_TAG 16
//#define SCE_YAML_VERBATIM_TAG 17
//#define SCE_YAML_DATETIME 18
//#define SCE_YAML_INDENTED_TEXT 19
//
