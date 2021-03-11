// Lexilla lexer library
/** @file Lexilla.h
 ** Lexilla definitions for dynamic and static linking.
 ** For C++, more features and type safety are available with the LexillaAccess module.
 **/
// Copyright 2020 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

// Define the default Lexilla shared library name for each platform
#if _WIN32
#define LEXILLA_LIB "lexilla"
#define LEXILLA_EXTENSION ".dll"
#else
#define LEXILLA_LIB "liblexilla"
#if defined(__APPLE__)
#define LEXILLA_EXTENSION ".dylib"
#else
#define LEXILLA_EXTENSION ".so"
#endif
#endif

// On Win32 use the stdcall calling convention otherwise use the standard calling convention
#if _WIN32
#define LEXILLA_CALL __stdcall
#else
#define LEXILLA_CALL
#endif

#ifdef __cplusplus
// Must have already included ILexer.h to have Scintilla::ILexer5 defined.
using Scintilla::ILexer5;
#else
typedef void ILexer5;
#endif

typedef ILexer5 *(*LexerFactoryFunction)();

#ifdef __cplusplus
namespace Lexilla {
#endif

typedef int (LEXILLA_CALL *GetLexerCountFn)();
typedef void (LEXILLA_CALL *GetLexerNameFn)(unsigned int Index, char *name, int buflength);
typedef LexerFactoryFunction(LEXILLA_CALL *GetLexerFactoryFn)(unsigned int Index);
typedef ILexer5*(LEXILLA_CALL *CreateLexerFn)(const char *name);
typedef const char *(LEXILLA_CALL *GetLibraryPropertyNamesFn)();
typedef void(LEXILLA_CALL *SetLibraryPropertyFn)(const char *key, const char *value);

#ifdef __cplusplus
}
#endif

#define LEXILLA_GETLEXERCOUNT "GetLexerCount"
#define LEXILLA_GETLEXERNAME "GetLexerName"
#define LEXILLA_GETLEXERFACTORY "GetLexerFactory"
#define LEXILLA_CREATELEXER "CreateLexer"
#define LEXILLA_GETLIBRARYPROPERTYNAMES "GetLibraryPropertyNames"
#define LEXILLA_SETLIBRARYPROPERTY "SetLibraryProperty"

// Static linking prototypes

#ifdef __cplusplus
extern "C" {
#endif

ILexer5 * LEXILLA_CALL CreateLexer(const char *name);
int LEXILLA_CALL GetLexerCount();
void LEXILLA_CALL GetLexerName(unsigned int index, char *name, int buflength);
LexerFactoryFunction LEXILLA_CALL GetLexerFactory(unsigned int index);
const char * LEXILLA_CALL GetLibraryPropertyNames();
void LEXILLA_CALL SetLibraryProperty(const char *key, const char *value);

#ifdef __cplusplus
}
#endif
