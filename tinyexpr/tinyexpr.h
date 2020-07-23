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
  typedef __int64          te_xint_t;
#else
  typedef int              te_xint_t;
#endif

typedef struct te_expr {
    int type;
    union {double value; const double *bound; const void *function;};
#if defined(TINYEXPR_USE_STATIC_MEMORY)
    void *parameters[TINYEXPR_MAX_PARAMETERS];
#else
    void *parameters[1];
#endif
} te_expr;


enum {
    TE_VARIABLE = 0,

    TE_FUNCTION0 = 8, TE_FUNCTION1, TE_FUNCTION2, TE_FUNCTION3,
    TE_FUNCTION4, TE_FUNCTION5, TE_FUNCTION6, TE_FUNCTION7,

    TE_CLOSURE0 = 16, TE_CLOSURE1, TE_CLOSURE2, TE_CLOSURE3,
    TE_CLOSURE4, TE_CLOSURE5, TE_CLOSURE6, TE_CLOSURE7,

    TE_FLAG_PURE = 32
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
void te_expr_memory_usage(unsigned int *count, unsigned int *count_max, unsigned int *free_error_count);
#endif

/* Parses the input expression, evaluates it, and frees it. */
/* Returns NaN on error. */
double te_interp(const char *expression, te_xint_t* error);

/* Parses the input expression and binds variables. */
/* Returns NULL on error. */
te_expr *te_compile(const char *expression, const te_variable *variables, int var_count, te_xint_t* error);

/* Evaluates the expression. */
double te_eval(const te_expr *n);

/* Prints debugging information on the syntax tree. */
void te_print(const te_expr *n);

/* Frees the expression. */
/* This is safe to call on NULL pointers. */
void te_free(te_expr *n);


#ifdef __cplusplus
}
#endif

#endif /*__TINYEXPR_H__*/
