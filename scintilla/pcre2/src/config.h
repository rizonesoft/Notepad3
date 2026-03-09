/* config.h for PCRE2 - Notepad3/Scintilla integration (MSVC/Windows)
   Generated from config.h.generic for static compilation with MSVC.
   PCRE2 10.47 */

/* PCRE2 is compiled as a static library integrated into scintilla.lib */

/* Standard MSVC headers available */
#define HAVE_ASSERT_H 1
#define HAVE_LIMITS_H 1
#define HAVE_MEMMOVE 1
#define HAVE_STDINT_H 1
#define HAVE_STDIO_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRERROR 1
#define HAVE_STRING_H 1
#define HAVE_WINDOWS_H 1
#define HAVE_WCHAR_H 1

/* MSVC provides __assume() */
#define HAVE_BUILTIN_ASSUME 1

/* \R matches any Unicode line ending by default (not just CR/LF/CRLF) */
/* #undef BSR_ANYCRLF */

/* Internal link size (2 = patterns up to 65535 code units) */
#ifndef LINK_SIZE
#define LINK_SIZE 2
#endif

/* Match limits to prevent catastrophic backtracking */
#ifndef MATCH_LIMIT
#define MATCH_LIMIT 10000000
#endif

#ifndef MATCH_LIMIT_DEPTH
#define MATCH_LIMIT_DEPTH MATCH_LIMIT
#endif

/* Heap limit in KiB */
#ifndef HEAP_LIMIT
#define HEAP_LIMIT 20000000
#endif

/* Named group limits */
#ifndef MAX_NAME_COUNT
#define MAX_NAME_COUNT 10000
#endif

#ifndef MAX_NAME_SIZE
#define MAX_NAME_SIZE 128
#endif

/* Variable-length lookbehind max */
#ifndef MAX_VARLOOKBEHIND
#define MAX_VARLOOKBEHIND 255
#endif

/* Default newline: LF (2). Notepad3 handles EOL modes at runtime. */
#ifndef NEWLINE_DEFAULT
#define NEWLINE_DEFAULT 2
#endif

/* Parentheses nesting limit */
#ifndef PARENS_NEST_LIMIT
#define PARENS_NEST_LIMIT 250
#endif

/* Package info */
#define PACKAGE "pcre2"
#define PACKAGE_BUGREPORT ""
#define PACKAGE_NAME "PCRE2"
#define PACKAGE_STRING "PCRE2 10.47"
#define PACKAGE_TARNAME "pcre2"
#define PACKAGE_URL ""
#define PACKAGE_VERSION "10.47"
#define VERSION "10.47"

/* Static export (no DLL export decorations) */
#define PCRE2_EXPORT

/* Enable 8-bit PCRE2 library (UTF-8) */
#define SUPPORT_PCRE2_8 1

/* Enable Unicode and UTF support */
#define SUPPORT_UNICODE 1

/* Enable JIT compilation for performance */
#define SUPPORT_JIT 1

/* Not needed for our use case */
/* #undef SUPPORT_PCRE2_16 */
/* #undef SUPPORT_PCRE2_32 */
/* #undef EBCDIC */
/* #undef NEVER_BACKSLASH_C */
/* #undef PCRE2_DEBUG */
/* #undef SUPPORT_VALGRIND */
