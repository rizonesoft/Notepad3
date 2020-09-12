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

// TODO: remove warnings
#pragma warning( disable : 4201 4204 )

/* COMPILE TIME OPTIONS */

/* Exponentiation associativity:
For a^b^c = (a^b)^c and -a^b = (-a)^b do nothing.
For a^b^c = a^(b^c) and -a^b = -(a^b) uncomment the next line.*/
/* #define TE_POW_FROM_RIGHT */

/* Logarithms
For log = base 10 log do nothing
For log = natural log uncomment the next line. */
/* #define TE_NAT_LOG */

#include "tinyexpr.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <stdbool.h>

#ifndef NAN
#define NAN (0.0/0.0)
#endif

#ifndef INFINITY
#define INFINITY (1.0/0.0)
#endif


enum {
    TOK_NULL = TE_CLOSURE7+1, TOK_ERROR, TOK_END, TOK_SEP,
    TOK_OPEN, TOK_CLOSE, TOK_NUMBER, TOK_VARIABLE, TOK_INFIX
};


enum {TE_CONSTANT = 1};


typedef struct state {
    const char *start;
    const char *next;
    int type;
    union {double value; const double *bound; const void *function;};
    void *context;

    const te_variable *lookup;
    int lookup_len;
} state;

#if defined(TINYEXPR_USE_STATIC_MEMORY)
    static te_expr te_expr_array[TINYEXPR_MAX_EXPRESSIONS] = {0};
    static bool te_expr_isAllocated[TINYEXPR_MAX_EXPRESSIONS] = {false};
#if defined(TINYEXPR_UNIT_TEST)
    static unsigned int te_expr_count = 0, te_expr_count_max = 0, te_expr_free_error_count = 0;
#endif
#endif

#define TYPE_MASK(TYPE) ((TYPE)&0x0000001F)

#define IS_PURE(TYPE) (((TYPE) & TE_FLAG_PURE) != 0)
#define IS_FUNCTION(TYPE) (((TYPE) & TE_FUNCTION0) != 0)
#define IS_CLOSURE(TYPE) (((TYPE) & TE_CLOSURE0) != 0)
#define ARITY(TYPE) ( ((TYPE) & (TE_FUNCTION0 | TE_CLOSURE0)) ? ((TYPE) & 0x00000007) : 0 )
#define NEW_EXPR(type, ...) new_expr((type), (const te_expr*[]){__VA_ARGS__})

#if defined(TINYEXPR_USE_STATIC_MEMORY) && defined(TINYEXPR_UNIT_TEST)
void te_expr_clean_up(void)
{
    /* Clear static memory array. */
    memset(te_expr_array, 0, sizeof(te_expr)*TINYEXPR_MAX_EXPRESSIONS);
    /* Clear allocation indication array. */
    for (int i = 0; i < TINYEXPR_MAX_EXPRESSIONS; i++) {
        te_expr_isAllocated[i] = false;
    }
    /* Clear counters. */
    te_expr_count = 0;
    te_expr_count_max = 0;
    te_expr_free_error_count = 0;
}

void te_expr_memory_usage(unsigned int *count, unsigned int *count_max, unsigned int *free_error_count)
{
    *count = te_expr_count;
    *count_max = te_expr_count_max;
    *free_error_count = te_expr_free_error_count;
}
#endif

static inline bool check_is_equal_function_pointer_1d(const void* pointer, double (*function)(double))
{
  if (pointer == (const void*)function) {
    return true;
  }
  return false;
}

static inline bool check_is_equal_function_pointer_2d(const void* pointer, double (*function)(double, double))
{
    if (pointer == (const void*)function) {
        return true;
    }
    return false;
}

static inline const void* get_function_pointer_1d(double (*function)(double))
{
  return (const void*)function;
}

static inline const void* get_function_pointer_2d(double (*function)(double, double))
{
    return (const void*)function;
}


static te_expr *new_expr(const int type, const te_expr *parameters[]) {
    const int arity = ARITY(type);
#if defined(TINYEXPR_USE_STATIC_MEMORY)
    te_expr *ret = NULL;
    for (int i = 0; i < TINYEXPR_MAX_EXPRESSIONS; i++) {
        if (!te_expr_isAllocated[i]) {
            ret = &te_expr_array[i];
            te_expr_isAllocated[i] = true;
#if defined(TINYEXPR_UNIT_TEST)
            te_expr_count++;
            if (te_expr_count > te_expr_count_max) {
                te_expr_count_max = te_expr_count;
            }
#endif
            break;
        }
    }
    if (ret == NULL) {
        return NULL;
    }
    memset(ret, 0, sizeof(te_expr));
#else
    const int psize = sizeof(void*) * arity;
    const int size = (sizeof(te_expr) - sizeof(void*)) + psize + (IS_CLOSURE(type) ? sizeof(void*) : 0);
    te_expr *ret = malloc(size);
    memset(ret, 0, size);
#endif
    if (arity && parameters) {
#if defined(TINYEXPR_USE_STATIC_MEMORY)
        memcpy(ret->parameters, parameters, sizeof(void*)*TINYEXPR_MAX_PARAMETERS);
#else
        memcpy(ret->parameters, parameters, psize);
#endif
    }
    ret->type = type;
    ret->bound = 0;
    return ret;
}


void te_free_parameters(te_expr *n) {
    if (!n) return;
    switch (TYPE_MASK(n->type)) {
        case TE_FUNCTION7: case TE_CLOSURE7: te_free(n->parameters[6]);     /* Falls through. */
        case TE_FUNCTION6: case TE_CLOSURE6: te_free(n->parameters[5]);     /* Falls through. */
        case TE_FUNCTION5: case TE_CLOSURE5: te_free(n->parameters[4]);     /* Falls through. */
        case TE_FUNCTION4: case TE_CLOSURE4: te_free(n->parameters[3]);     /* Falls through. */
        case TE_FUNCTION3: case TE_CLOSURE3: te_free(n->parameters[2]);     /* Falls through. */
        case TE_FUNCTION2: case TE_CLOSURE2: te_free(n->parameters[1]);     /* Falls through. */
        case TE_FUNCTION1: case TE_CLOSURE1: te_free(n->parameters[0]);
    }
}

static void te_free_once(te_expr *n) {
#if defined(TINYEXPR_USE_STATIC_MEMORY)
    #if defined(TINYEXPR_UNIT_TEST)
        bool isFreed = false;
    #endif
        for (int i = 0; i < TINYEXPR_MAX_EXPRESSIONS; i++) {
            if (n == &te_expr_array[i]) {
                te_expr_isAllocated[i] = false;
    #if defined(TINYEXPR_UNIT_TEST)
                te_expr_count--;
                isFreed = true;
    #endif
                break;
            }
        }
    #if defined(TINYEXPR_UNIT_TEST)
        if (!isFreed) {
            if ((te_expr_free_error_count + 1) > 0) {
                te_expr_free_error_count++;
            }
        }
    #endif
        n = NULL;
#else
    free(n);
#endif
}

void te_free(te_expr *n) {
    if (!n) return;
    te_free_parameters(n);
    te_free_once(n);
}


static double pi(void) {return 3.14159265358979323846;}
static double e(void) {return 2.71828182845904523536;}
static double fac(double a) {/* simplest version of fac */
    if (a < 0.0)
        return NAN;
    if (a > UINT_MAX)
        return INFINITY;
    unsigned int ua = (unsigned int)(a);
    unsigned long int result = 1, i;
    for (i = 1; i <= ua; i++) {
        if (i > ULONG_MAX / result)
            return INFINITY;
        result *= i;
    }
    return (double)result;
}
static double ncr(double n, double r) {
    if (n < 0.0 || r < 0.0 || n < r) return NAN;
    if (n > UINT_MAX || r > UINT_MAX) return INFINITY;
    unsigned long int un = (unsigned int)(n), ur = (unsigned int)(r), i;
    unsigned long int result = 1;
    if (ur > un / 2) ur = un - ur;
    for (i = 1; i <= ur; i++) {
        if (result > ULONG_MAX / (un - ur + i))
            return INFINITY;
        result *= un - ur + i;
        result /= i;
    }
    return result;
}

static double add(double a, double b) { return a + b; }
static double sub(double a, double b) { return a - b; }
static double mul(double a, double b) { return a * b; }
static double divide(double a, double b) { return a / b; }
static double negate(double a) { return -a; }
static double comma(double a, double b) { (void)a; return b; }
static double percent(double a) { return a / 100.0; }

static double greater(double a, double b) { return a > b; }
static double greater_eq(double a, double b) { return a >= b; }
static double lower(double a, double b) { return a < b; }
static double lower_eq(double a, double b) { return a <= b; }
static double equal(double a, double b) { return a == b; }
static double not_equal(double a, double b) { return a != b; }
static double logical_and(double a, double b) { return a != 0.0 && b != 0.0; }
static double logical_or(double a, double b) { return a != 0.0 || b != 0.0; }
static double logical_not(double a) { return a == 0.0; }
static double logical_notnot(double a) { return a != 0.0; }
static double negate_logical_not(double a) { return -(a == 0.0); }
static double negate_logical_notnot(double a) { return -(a != 0.0); }

static double npr(double n, double r) { return ncr(n, r) * fac(r); }
static double ceilx(double n) { return (double)ceil(n); }
static double floorx(double n) { return (double)floor(n); }

#ifndef COUNTOF
#define COUNTOF(A) (sizeof(A)/sizeof((A)[0]))
#endif

static const te_variable functions[] = {
    /* must be in alphabetical order */
    {"abs",     (const void*)fabs,        TE_FUNCTION1 | TE_FLAG_PURE, 0},
    {"acos",    (const void*)acos,        TE_FUNCTION1 | TE_FLAG_PURE, 0},
    {"add",     (const void*)add,         TE_FUNCTION2 | TE_FLAG_PURE, 0},
    {"asin",    (const void*)asin,        TE_FUNCTION1 | TE_FLAG_PURE, 0},
    {"atan",    (const void*)atan,        TE_FUNCTION1 | TE_FLAG_PURE, 0},
    {"atan2",   (const void*)atan2,       TE_FUNCTION2 | TE_FLAG_PURE, 0},
    {"ceil",    (const void*)ceilx,       TE_FUNCTION1 | TE_FLAG_PURE, 0},
    {"comma",   (const void*)comma,       TE_FUNCTION2 | TE_FLAG_PURE, 0},
    {"cos",     (const void*)cos,         TE_FUNCTION1 | TE_FLAG_PURE, 0},
    {"cosh",    (const void*)cosh,        TE_FUNCTION1 | TE_FLAG_PURE, 0},
    {"div",     (const void*)divide,      TE_FUNCTION2 | TE_FLAG_PURE, 0},
    {"e",       (const void*)e,           TE_FUNCTION0 | TE_FLAG_PURE, 0},
    {"exp",     (const void*)exp,         TE_FUNCTION1 | TE_FLAG_PURE, 0},
    {"fac",     (const void*)fac,         TE_FUNCTION1 | TE_FLAG_PURE, 0},
    {"floor",   (const void*)floorx,      TE_FUNCTION1 | TE_FLAG_PURE, 0},
    {"ln",      (const void*)log,         TE_FUNCTION1 | TE_FLAG_PURE, 0},
#ifdef TE_NAT_LOG
    {"log",     (const void*)log,         TE_FUNCTION1 | TE_FLAG_PURE, 0},
#else
    {"log",     (const void*)log10,       TE_FUNCTION1 | TE_FLAG_PURE, 0},
#endif
    {"log10",   (const void*)log10,       TE_FUNCTION1 | TE_FLAG_PURE, 0},
    {"mod",     (const void*)fmod,        TE_FUNCTION2 | TE_FLAG_PURE, 0},
    {"mul",     (const void*)mul,         TE_FUNCTION2 | TE_FLAG_PURE, 0},
    {"ncr",     (const void*)ncr,         TE_FUNCTION2 | TE_FLAG_PURE, 0},
    {"neg",     (const void*)negate,      TE_FUNCTION1 | TE_FLAG_PURE, 0},
    {"npr",     (const void*)npr,         TE_FUNCTION2 | TE_FLAG_PURE, 0},
    {"percent", (const void*)percent,     TE_FUNCTION1 | TE_FLAG_PURE, 0},
    {"pi",      (const void*)pi,          TE_FUNCTION0 | TE_FLAG_PURE, 0},
    {"pow",     (const void*)pow,         TE_FUNCTION2 | TE_FLAG_PURE, 0},
    {"sin",     (const void*)sin,         TE_FUNCTION1 | TE_FLAG_PURE, 0},
    {"sinh",    (const void*)sinh,        TE_FUNCTION1 | TE_FLAG_PURE, 0},
    {"sqrt",    (const void*)sqrt,        TE_FUNCTION1 | TE_FLAG_PURE, 0},
    {"sub",     (const void*)sub,         TE_FUNCTION2 | TE_FLAG_PURE, 0},
    {"tan",     (const void*)tan,         TE_FUNCTION1 | TE_FLAG_PURE, 0},
    {"tanh",    (const void*)tanh,        TE_FUNCTION1 | TE_FLAG_PURE, 0},
    {0, 0, 0, 0}
};


static const te_variable *find_builtin(const char *name, size_t len) {
    int imin = 0;
    int imax = COUNTOF(functions) - 2;

    /*Binary search.*/
    while (imax >= imin) {
        const int i = (imin + ((imax-imin)/2));
        int c = strncmp(name, functions[i].name, len);
        if (!c) c = '\0' - functions[i].name[len];
        if (c == 0) {
            return functions + i;
        } else if (c > 0) {
            imin = i + 1;
        } else {
            imax = i - 1;
        }
    }
    return 0;
}

static const te_variable *find_lookup(const state *s, const char *name, size_t len) {
    int iters;
    const te_variable *var;
    if (!s->lookup) return 0;

    for (var = s->lookup, iters = s->lookup_len; iters; ++var, --iters) {
        if (strncmp(name, var->name, len) == 0 && var->name[len] == '\0') {
            return var;
        }
    }
    return 0;
}

void next_token(state *s) {
    s->type = TOK_NULL;

    do {

        if (!*s->next){
            s->type = TOK_END;
            return;
        }

        /* Try reading a number. */
        if ((s->next[0] >= '0' && s->next[0] <= '9') || s->next[0] == '.') {
            s->value = strtod(s->next, (char**)&s->next);
            s->type = TOK_NUMBER;
        } else {
            /* Look for a variable or builtin function call. */
            if (s->next[0] >= 'a' && s->next[0] <= 'z') {
                const char *start = s->next;
                while ((s->next[0] >= 'a' && s->next[0] <= 'z') || (s->next[0] >= '0' && s->next[0] <= '9') || (s->next[0] == '_')) s->next++;

                const te_variable *var = find_lookup(s, start, s->next - start);
                if (!var) var = find_builtin(start, s->next - start);

                if (!var) {
                    s->type = TOK_ERROR;
                } else {
                    switch(TYPE_MASK(var->type))
                    {
                        case TE_VARIABLE:
                            s->type = TOK_VARIABLE;
                            s->bound = var->address;
                            break;

                        case TE_CLOSURE0: case TE_CLOSURE1: case TE_CLOSURE2: case TE_CLOSURE3:         /* Falls through. */
                        case TE_CLOSURE4: case TE_CLOSURE5: case TE_CLOSURE6: case TE_CLOSURE7:         /* Falls through. */
                            s->context = var->context;                                                  /* Falls through. */

                        case TE_FUNCTION0: case TE_FUNCTION1: case TE_FUNCTION2: case TE_FUNCTION3:     /* Falls through. */
                        case TE_FUNCTION4: case TE_FUNCTION5: case TE_FUNCTION6: case TE_FUNCTION7:     /* Falls through. */
                            s->type = var->type;
                            s->function = var->address;
                            break;
                    }
                }

            } else {
                /* Look for an operator or special character. */
                switch (s->next++[0]) {
                    case '+': s->type = TOK_INFIX; s->function = get_function_pointer_2d(add); break;
                    case '-': s->type = TOK_INFIX; s->function = get_function_pointer_2d(sub); break;
                    case '*': s->type = TOK_INFIX; s->function = get_function_pointer_2d(mul); break;
                    case -41: s->type = TOK_INFIX; s->function = get_function_pointer_2d(mul); break;    // '×' CP-1252
                    case '/': s->type = TOK_INFIX; s->function = get_function_pointer_2d(divide); break;
                    case ':': s->type = TOK_INFIX; s->function = get_function_pointer_2d(divide); break;
                    case -9: s->type = TOK_INFIX; s->function = get_function_pointer_2d(divide); break;    // '÷' CP-1252
                    case '^': s->type = TOK_INFIX; s->function = get_function_pointer_2d(pow); break;
                    case '%':
                        if (s->next++[0] == '%') {
                          s->type = TOK_INFIX; s->function = get_function_pointer_1d(percent);
                        }
                        else {
                          s->next--;
                          s->type = TOK_INFIX; s->function = get_function_pointer_2d(fmod);
                        }
                        break;
                    case '!':
                        if (s->next++[0] == '=') {
                          s->type = TOK_INFIX; s->function = get_function_pointer_2d(not_equal);
                        }
                        else {
                          s->next--;
                          if (s->next++[0] == '*') {
                            s->type = TOK_INFIX; s->function = get_function_pointer_1d(fac);
                          }
                          else {
                            s->next--;
                            s->type = TOK_INFIX; s->function = get_function_pointer_1d(logical_not);
                          }
                        }
                        break;
                    case '=':
                        if (s->next++[0] == '=') {
                            s->type = TOK_INFIX; s->function = get_function_pointer_2d(equal);
                        } else {
                            s->type = TOK_ERROR;
                        }
                        break;
                    case '<':
                        if (s->next++[0] == '=') {
                            s->type = TOK_INFIX; s->function = get_function_pointer_2d(lower_eq);
                        } else {
                            s->next--;
                            s->type = TOK_INFIX; s->function = get_function_pointer_2d(lower);
                        }
                        break;
                    case '>':
                        if (s->next++[0] == '=') {
                            s->type = TOK_INFIX; s->function = get_function_pointer_2d(greater_eq);
                        } else {
                            s->next--;
                            s->type = TOK_INFIX; s->function = get_function_pointer_2d(greater);
                        }
                        break;
                    case '&':
                        if (s->next++[0] == '&') {
                            s->type = TOK_INFIX; s->function = get_function_pointer_2d(logical_and);
                        } else {
                            s->type = TOK_ERROR;
                        }
                        break;
                    case '|':
                        if (s->next++[0] == '|') {
                            s->type = TOK_INFIX; s->function = get_function_pointer_2d(logical_or);
                        } else {
                            s->type = TOK_ERROR;
                        }
                        break;
                    case '(': s->type = TOK_OPEN; break;
                    case ')': s->type = TOK_CLOSE; break;
                    case ',': s->type = TOK_SEP; break;
                    case ' ': case '\t': case '\n': case '\r': break;
                    default: s->type = TOK_ERROR; break;
                }
            }
        }
    } while (s->type == TOK_NULL);
}


static te_expr *list(state *s);
static te_expr *expr(state *s);
static te_expr *power(state *s);

static te_expr *base(state *s) {
    /* <base>      =    <constant> | <variable> | <constant> { "%%" | "!*" } | <function-0> {"(" ")"} | <function-1> <power> | <function-X> "(" <expr> {"," <expr>} ")" | "(" <list> ")" */
    te_expr *ret = NULL;
    int arity = 0;

    switch (TYPE_MASK(s->type)) {
        case TOK_NUMBER:
            ret = new_expr(TE_CONSTANT, 0);
            if (ret) {
                ret->value = s->value;
                next_token(s);
                if (s->type == TOK_INFIX && (
                     check_is_equal_function_pointer_1d(s->function, percent) ||
                     check_is_equal_function_pointer_1d(s->function, fac))) {
                    te_expr* ret_new = NEW_EXPR(TE_FUNCTION1, ret);
                    if (ret_new) {
                      ret = ret_new;
                      ret->function = s->function;
                      next_token(s);
                    }
                }
            }
            break;

        case TOK_VARIABLE:
            ret = new_expr(TE_VARIABLE, 0);
            if (ret) {
                ret->bound = s->bound;
                next_token(s);
            }
            break;

        case TE_FUNCTION0:
        case TE_CLOSURE0:
            ret = new_expr(s->type, 0);
            if (ret) {
                ret->function = s->function;
                if (IS_CLOSURE(s->type)) ret->parameters[0] = s->context;
                next_token(s);
                if (s->type == TOK_OPEN) {
                    next_token(s);
                    if (s->type != TOK_CLOSE) {
                        s->type = TOK_ERROR;
                    } else {
                        next_token(s);
                    }
                }
            }
            break;

        case TE_FUNCTION1:
        case TE_CLOSURE1:
            ret = new_expr(s->type, 0);
            if (ret) {
                ret->function = s->function;
                if (IS_CLOSURE(s->type)) ret->parameters[1] = s->context;
                next_token(s);
                ret->parameters[0] = power(s);
            }
            break;

        case TE_FUNCTION2: case TE_FUNCTION3: case TE_FUNCTION4:
        case TE_FUNCTION5: case TE_FUNCTION6: case TE_FUNCTION7:
        case TE_CLOSURE2: case TE_CLOSURE3: case TE_CLOSURE4:
        case TE_CLOSURE5: case TE_CLOSURE6: case TE_CLOSURE7:
            arity = ARITY(s->type);

            ret = new_expr(s->type, 0);
            if (ret) {
                ret->function = s->function;
                if (IS_CLOSURE(s->type)) ret->parameters[arity] = s->context;
                next_token(s);

                if (s->type != TOK_OPEN) {
                    s->type = TOK_ERROR;
                } else {
                    int i;
                    for(i = 0; i < arity; i++) {
                        next_token(s);
                        ret->parameters[i] = expr(s);
                        if(s->type != TOK_SEP) {
                            break;
                        }
                    }
                    if(s->type != TOK_CLOSE || i != arity - 1) {
                        s->type = TOK_ERROR;
                    } else {
                        next_token(s);
                    }
                }
            }
            break;

        case TOK_OPEN:
            next_token(s);
            ret = list(s);
            if (ret) {
                if (s->type != TOK_CLOSE) {
                    s->type = TOK_ERROR;
                } else {
                    next_token(s);
                }
            }
            break;

        default:
            ret = new_expr(0, 0);
            if (ret) {
                s->type = TOK_ERROR;
                ret->value = NAN;
            }
            break;
    }

    return ret;
}


static te_expr *power(state *s) {
    /* <power>     =    {("-" | "+" | "!")} <base> */
    int sign = 1;
    while (s->type == TOK_INFIX && (check_is_equal_function_pointer_2d(s->function, add) || check_is_equal_function_pointer_2d(s->function, sub))) {
        if (check_is_equal_function_pointer_2d(s->function, sub)) sign = -sign;
        next_token(s);
    }

    int logical = 0;
    while (s->type == TOK_INFIX && (
      check_is_equal_function_pointer_2d(s->function, add) ||
      check_is_equal_function_pointer_2d(s->function, sub) ||
      check_is_equal_function_pointer_1d(s->function, logical_not)))
    {
        if (check_is_equal_function_pointer_1d(s->function, logical_not)) {
            if (logical == 0) {
                logical = -1;
            } else {
                logical = -logical;
            }
        }
        next_token(s);
    }

    te_expr *ret = base(s);

    if (ret && (sign == 1)) {
        if (logical == 0) {
            return ret;
        }
        else if (logical == -1) {
            te_expr *ret_new = NEW_EXPR(TE_FUNCTION1 | TE_FLAG_PURE, ret);
            if (ret_new) {
                ret = ret_new;
                ret->function = get_function_pointer_1d(logical_not);
            } else {
                te_free(ret);
                return NULL;
            }
        } else {
            te_expr *ret_new = NEW_EXPR(TE_FUNCTION1 | TE_FLAG_PURE, ret);
            if (ret_new) {
                ret = ret_new;
                ret->function = get_function_pointer_1d(logical_notnot);
            } else {
                te_free(ret);
                return NULL;
            }
        }
    }
    else if (ret && (sign != 1)) {
        if (logical == 0) {
            te_expr *ret_new = NEW_EXPR(TE_FUNCTION1 | TE_FLAG_PURE, ret);
            if (ret_new) {
                ret = ret_new;
                ret->function = get_function_pointer_1d(negate);
            } else {
                te_free(ret);
                return NULL;
            }
        } else if (logical == -1) {
            te_expr *ret_new = NEW_EXPR(TE_FUNCTION1 | TE_FLAG_PURE, ret);
            if (ret_new) {
                ret = ret_new;
                ret->function = get_function_pointer_1d(negate_logical_not);
            } else {
                te_free(ret);
                return NULL;
            }
        } else {
            te_expr *ret_new = NEW_EXPR(TE_FUNCTION1 | TE_FLAG_PURE, ret);
            if (ret_new) {
                ret = ret_new;
                ret->function = get_function_pointer_1d(negate_logical_notnot);
            } else {
                te_free(ret);
                return NULL;
            }
        }
    }
    return ret;
}

#ifdef TE_POW_FROM_RIGHT
static te_expr *factor(state *s) {
    /* <factor>    =    <power> {"^" <power>} */
    te_expr *ret = power(s);
    if (!ret) {
        return NULL;
    }

    const void *left_function = NULL;
    te_expr *insertion = NULL;

    if (ret->type == (TE_FUNCTION1 | TE_FLAG_PURE) && (
        check_is_equal_function_pointer_2d(ret->function, negate) ||
        check_is_equal_function_pointer_1d(ret->function, logical_not) ||
        check_is_equal_function_pointer_1d(ret->function, logical_notnot) ||
        check_is_equal_function_pointer_1d(ret->function, negate_logical_not) ||
        check_is_equal_function_pointer_1d(ret->function, negate_logical_notnot)) {
        
        left_function = ret->function;
        te_expr *se = ret->parameters[0];
        te_free_once(ret); /* Free only the top expression as ret's parameters were just used. */
        ret = se;
    }

    while (s->type == TOK_INFIX && check_is_equal_function_pointer_2d(s->function, pow)) {
        const void* t = s->function;
        next_token(s);

        te_expr *param = power(s);
        if (!param) {
            te_free(insertion);
            te_free(ret);
            return NULL;
        }

        if (insertion) {
            /* Make exponentiation go right-to-left. */
            te_expr *insert = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, insertion->parameters[1], param);
            if (insert) {
                insert->function = t;
                insertion->parameters[1] = insert;
                insertion = insert;
            } else {
                te_free(param);
                te_free(insertion);
                te_free(ret);
                return NULL;
            }
        } else {
            te_expr *ret_new = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, ret, param);
            if (ret_new) {
                ret = ret_new;
                ret->function = t;
                insertion = ret;
            } else {
                te_free(param);
                te_free(insertion);
                te_free(ret);
                return NULL;
            }
        }
    }

    if (left_function) {
        te_expr *ret_new = NEW_EXPR(TE_FUNCTION1 | TE_FLAG_PURE, ret);
        if (ret_new) {
            ret = ret_new;
            ret->function = left_function;
        } else {
            te_free(insertion);
            te_free(ret);
            return NULL;
        }
    }

    return ret;
}
#else
static te_expr *factor(state *s) {
    /* <factor>    =    <power> {"^" <power>} */
    te_expr *ret = power(s);
    if (!ret) {
        return NULL;
    }

    while (s->type == TOK_INFIX && check_is_equal_function_pointer_2d(s->function, pow)) {
        const void* t = s->function;
        next_token(s);
        te_expr *param = power(s);
        if (!param) {
            te_free(ret);
            return NULL;
        }
        te_expr *ret_new = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, ret, param);
        if (ret_new) {
            ret = ret_new;
            ret->function = t;
        } else {
            te_free(param);
            te_free(ret);
            return NULL;
        }
    }

    return ret;
}
#endif



static te_expr *term(state *s) {
    /* <term>      =    <factor> {("*" | "/" | "%") <factor>} */
    te_expr *ret = factor(s);
    if (!ret) {
        return NULL;
    }

    while (s->type == TOK_INFIX &&
          (check_is_equal_function_pointer_2d(s->function, mul)    ||
           check_is_equal_function_pointer_2d(s->function, divide) ||
           check_is_equal_function_pointer_2d(s->function, fmod)))
    {
        const void* t = s->function;
        next_token(s);
        te_expr *param = factor(s);
        if (!param) {
            te_free(ret);
            return NULL;
        }
        te_expr *ret_new = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, ret, param);
        if (ret_new) {
            ret = ret_new;
            ret->function = t;
        } else {
            te_free(param);
            te_free(ret);
            return NULL;
        }
    }

    return ret;
}


static te_expr *sum_expr(state *s) {
    /* <expr>      =    <term> {("+" | "-") <term>} */
    te_expr *ret = term(s);
    if (!ret) {
        return NULL;
    }

    while (s->type == TOK_INFIX && (
           check_is_equal_function_pointer_2d(s->function, add) || 
           check_is_equal_function_pointer_2d(s->function, sub))) {
        const void* t = s->function;
        next_token(s);
        te_expr *param = term(s);
        if (!param) {
            te_free(ret);
            return NULL;
        }
        te_expr *ret_new = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, ret, param);
        if (ret_new) {
            ret = ret_new;
            ret->function = t;
        } else {
            te_free(param);
            te_free(ret);
            return NULL;
        }
    }
    return ret;
}


static te_expr *test_expr(state *s) {
    /* <expr>      =    <sum_expr> {(">" | ">=" | "<" | "<=" | "==" | "!=") <sum_expr>} */
    te_expr *ret = sum_expr(s);
    if (!ret) {
        return NULL;
    }

    while (s->type == TOK_INFIX && (
            check_is_equal_function_pointer_2d(s->function, greater) ||
            check_is_equal_function_pointer_2d(s->function, greater_eq) ||
            check_is_equal_function_pointer_2d(s->function, lower) ||
            check_is_equal_function_pointer_2d(s->function, lower_eq) ||
            check_is_equal_function_pointer_2d(s->function, equal) ||
            check_is_equal_function_pointer_2d(s->function, not_equal))) {
        const void* t = s->function;
        next_token(s);
        te_expr *param = sum_expr(s);
        if (!param) {
            te_free(ret);
            return NULL;
        }
        te_expr *ret_new = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, ret, param);
        if (ret_new) {
            ret = ret_new;
            ret->function = t;
        } else {
            te_free(param);
            te_free(ret);
            return NULL;
        }
    }
    return ret;
}


static te_expr *expr(state *s) {
    /* <expr>      =    <test_expr> {("&&" | "||") <test_expr>} */
    te_expr *ret = test_expr(s);
    if (!ret) {
        return NULL;
    }

    while (s->type == TOK_INFIX && (
            check_is_equal_function_pointer_2d(s->function, logical_and) ||
            check_is_equal_function_pointer_2d(s->function, logical_or))) {
        const void* t = s->function;
        next_token(s);
        te_expr *param = test_expr(s);
        if (!param) {
            te_free(ret);
            return NULL;
        }
        te_expr *ret_new = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, ret, param);
        if (ret_new) {
            ret = ret_new;
            ret->function = t;
        } else {
            te_free(param);
            te_free(ret);
            return NULL;
        }
    }
    return ret;
}


static te_expr *list(state *s) {
    /* <list>      =    <expr> {"," <expr>} */
    te_expr *ret = expr(s);
    if (!ret) {
        return NULL;
    }

    while (s->type == TOK_SEP) {
        next_token(s);
        te_expr *param = expr(s);
        if (!param) {
            te_free(ret);
            return NULL;
        }
        te_expr *ret_new = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, ret, param);
        if (ret_new) {
            ret = ret_new;
            ret->function = get_function_pointer_2d(comma);
        } else {
            te_free(param);
            te_free(ret);
            return NULL;
        }
    }

    return ret;
}


#define TE_FUN(...) ((double(*)(__VA_ARGS__))n->function)
#define M(e) te_eval(n->parameters[e])


double te_eval(const te_expr *n) {
    if (!n) return NAN;

    switch(TYPE_MASK(n->type)) {
        case TE_CONSTANT: return n->value;
        case TE_VARIABLE: return *n->bound;

        case TE_FUNCTION0: case TE_FUNCTION1: case TE_FUNCTION2: case TE_FUNCTION3:
        case TE_FUNCTION4: case TE_FUNCTION5: case TE_FUNCTION6: case TE_FUNCTION7:
            switch(ARITY(n->type)) {
                case 0: return TE_FUN(void)();
                case 1: return TE_FUN(double)(M(0));
                case 2: return TE_FUN(double, double)(M(0), M(1));
                case 3: return TE_FUN(double, double, double)(M(0), M(1), M(2));
                case 4: return TE_FUN(double, double, double, double)(M(0), M(1), M(2), M(3));
                case 5: return TE_FUN(double, double, double, double, double)(M(0), M(1), M(2), M(3), M(4));
                case 6: return TE_FUN(double, double, double, double, double, double)(M(0), M(1), M(2), M(3), M(4), M(5));
                case 7: return TE_FUN(double, double, double, double, double, double, double)(M(0), M(1), M(2), M(3), M(4), M(5), M(6));
                default: return NAN;
            }

        case TE_CLOSURE0: case TE_CLOSURE1: case TE_CLOSURE2: case TE_CLOSURE3:
        case TE_CLOSURE4: case TE_CLOSURE5: case TE_CLOSURE6: case TE_CLOSURE7:
            switch(ARITY(n->type)) {
                case 0: return TE_FUN(void*)(n->parameters[0]);
                case 1: return TE_FUN(void*, double)(n->parameters[1], M(0));
                case 2: return TE_FUN(void*, double, double)(n->parameters[2], M(0), M(1));
                case 3: return TE_FUN(void*, double, double, double)(n->parameters[3], M(0), M(1), M(2));
                case 4: return TE_FUN(void*, double, double, double, double)(n->parameters[4], M(0), M(1), M(2), M(3));
                case 5: return TE_FUN(void*, double, double, double, double, double)(n->parameters[5], M(0), M(1), M(2), M(3), M(4));
                case 6: return TE_FUN(void*, double, double, double, double, double, double)(n->parameters[6], M(0), M(1), M(2), M(3), M(4), M(5));
                case 7: return TE_FUN(void*, double, double, double, double, double, double, double)(n->parameters[7], M(0), M(1), M(2), M(3), M(4), M(5), M(6));
                default: return NAN;
            }

        default: return NAN;
    }

}

#undef TE_FUN
#undef M

static void optimize(te_expr *n) {
    if (!n) return;

    /* Evaluates as much as possible. */
    if (n->type == TE_CONSTANT) return;
    if (n->type == TE_VARIABLE) return;

    /* Only optimize out functions flagged as pure. */
    if (IS_PURE(n->type)) {
        const int arity = ARITY(n->type);
        bool known = true;
        int i;
        for (i = 0; i < arity; ++i) {
            optimize(n->parameters[i]);
            if (((te_expr*)(n->parameters[i]))->type != TE_CONSTANT) {
                known = false;
            }
        }
        if (known) {
            const double value = te_eval(n);
            te_free_parameters(n);
            n->type = TE_CONSTANT;
            n->value = value;
        }
    }
}


te_expr *te_compile(const char *expression, const te_variable *variables, int var_count, te_xint_t *error) {
    state s;
    s.start = s.next = expression;
    s.lookup = variables;
    s.lookup_len = var_count;

    next_token(&s);
    te_expr *root = list(&s);
    if (root) {
        if (s.type != TOK_END) {
            te_free(root);
            if (error) {
                *error = (s.next - s.start);
                if (*error == 0) *error = 1;
            }
            return NULL;
        } else {
            optimize(root);
            if (error) *error = 0;
            return root;
        }
    } else {
        /* Failed due to memory allocation. */
        if (error) {
            *error = 1;
        }
        return NULL;
    }
}


double te_interp(const char *expression, te_xint_t* error) {
    te_expr *n = te_compile(expression, 0, 0, error);
    double ret = NAN;
    if (n) {
        ret = te_eval(n);
        te_free(n);
    }
    return ret;
}

static void pn (const te_expr *n, int depth) {
    int i, arity;
    printf("%*s", depth, "");

    switch(TYPE_MASK(n->type)) {
    case TE_CONSTANT: printf("%f\n", n->value); break;
    case TE_VARIABLE: printf("bound %p\n", n->bound); break;

    case TE_FUNCTION0: case TE_FUNCTION1: case TE_FUNCTION2: case TE_FUNCTION3:
    case TE_FUNCTION4: case TE_FUNCTION5: case TE_FUNCTION6: case TE_FUNCTION7:
    case TE_CLOSURE0: case TE_CLOSURE1: case TE_CLOSURE2: case TE_CLOSURE3:
    case TE_CLOSURE4: case TE_CLOSURE5: case TE_CLOSURE6: case TE_CLOSURE7:
         arity = ARITY(n->type);
         printf("f%d", arity);
         for(i = 0; i < arity; i++) {
             printf(" %p", n->parameters[i]);
         }
         printf("\n");
         for(i = 0; i < arity; i++) {
             pn(n->parameters[i], depth + 1);
         }
         break;
    }
}


void te_print(const te_expr *n) {
    pn(n, 0);
}
