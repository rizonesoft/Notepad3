/* encoding: UTF-8
 * TINYEXPR - Tiny recursive descent parser and evaluation engine in C
 *
 * Copyright (c) 2015-2018 Lewis Van Winkle
 *
 * http://CodePlea.com
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgement in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#ifndef __TINYEXPR_H__
#define __TINYEXPR_H__

// CFG:
#undef TINYEXPR_USE_STATIC_MEMORY

#ifdef __cplusplus
extern "C" {
#endif

#if defined(TINYEXPR_USE_STATIC_MEMORY)
    #if !defined(TINYEXPR_MAX_EXPRESSIONS)
        #define TINYEXPR_MAX_EXPRESSIONS    64
    #endif

    #define TINYEXPR_MAX_PARAMETERS         8
#endif

#ifdef _WIN64
  typedef __int64          te_int_t;
  #define TE_INT_FMT      "%lli" 
#else
  typedef int              te_int_t;
  #define TE_INT_FMT      "%i"
#endif

#pragma warning(push)
#pragma warning(disable:4201)

  typedef struct te_expr {
    int type;
    int fmt;
    union {double value; const double * bound; const void * function;};
#if defined(TINYEXPR_USE_STATIC_MEMORY)
    void *parameters[TINYEXPR_MAX_PARAMETERS];
#else
    void *parameters[1];
#endif
} te_expr;

#pragma warning(pop)

enum {
    TE_VARIABLE = 0,

    TE_FUNCTION0 = 8, TE_FUNCTION1, TE_FUNCTION2, TE_FUNCTION3,
    TE_FUNCTION4, TE_FUNCTION5, TE_FUNCTION6, TE_FUNCTION7,

    TE_CLOSURE0 = 16, TE_CLOSURE1, TE_CLOSURE2, TE_CLOSURE3,
    TE_CLOSURE4, TE_CLOSURE5, TE_CLOSURE6, TE_CLOSURE7,

    TOK_NUM_STD = 32, TOK_NUM_HEX, TOK_NUM_BIN, TOK_NUM_OCT,

    TE_FLAG_PURE = 64
};

typedef struct te_variable {
    const char *name;
    const void *address;
    int type;
    void *context;
} te_variable;


/* Static memory unit test supporting functions. */
#if defined(TINYEXPR_USE_STATIC_MEMORY) && defined(TINYEXPR_UNIT_TEST)
/* Cleans internal static memory and supporting variables. */
void te_expr_clean_up(void);
/* Returns memory usage for static memory test. */
void te_expr_memory_usage(unsigned int *count, unsigned int * count_max, unsigned int * free_error_count);
#endif

/* Parses the input expression, evaluates it, and frees it. */
/* Returns NaN on error. */
double te_interp(const char *expression, te_int_t* error, int* fmt);

/* Parses the input expression and binds variables. */
/* Returns NULL on error. */
te_expr *te_compile(const char * expression, const te_variable * variables, int var_count, te_int_t * error);

/* Evaluates the expression. */
double te_eval(const te_expr *n);

/* Prints debugging information on the syntax tree. */
void te_print(const te_expr *n);

/* Frees the expression. */
/* This is safe to call on NULL pointers. */
void te_free(te_expr *n);

/* ANSI codepage of te operators */
inline unsigned te_cp() { return 1252U; }

/* invalid default char for conversion */
inline unsigned te_invalid_chr() { return '#'; }

/* invalid default char for conversion */
inline unsigned te_is_num(const char * const pch) { return (pch && (*pch > 47) && (*pch < 58)); }

/* check for operator or special character. */
inline int te_is_op(const char * const expr) {
    if (!expr)
        return !0;
    switch (*expr) {
    case '\0':
    case '+':
    case '-':
    case '*':
    case -41:
    case '/':
    case ':':
    case -9:
    case '^':
    case '%':
    case '!':
    case '<':
    case '>':
    case '(':
    case ')':
    case ',':
    case ' ':
    case '\t':
    case '\n':
    case '\r':
        return !0;
    case '=':
        if (expr[1] == '=')
            return !0;
        break;
    case '&':
        if (expr[1] == '&')
            return !0;
        break;
    case '|':
        if (expr[1] == '|')
            return !0;
        break;
    default:
        break;        
    }
    return 0;
}


#ifdef __cplusplus
}
#else
#endif

#endif /*__TINYEXPR_H__*/
