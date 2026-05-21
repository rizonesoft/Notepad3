/* encoding: UTF-8
 *
 * C Interface for TinyExpr++ (tinyexpr_cif)
 *
 * Provides a C-compatible API wrapping the C++ TinyExpr++ library,
 * matching the function signatures of the original TinyExpr C library.
 *
 * This header is intended to be included by C source files.
 * The implementation (tinyexpr_cif.cpp) uses TinyExpr++ internally.
 */

#ifndef TINYEXPR_CIF_H
#define TINYEXPR_CIF_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN64
  typedef __int64          te_int_t;
  #define TE_INT_FMT      "%lli"
#else
  typedef int              te_int_t;
  #define TE_INT_FMT      "%i"
#endif

/* Opaque handle for a compiled expression (returned by te_compile). */
typedef struct te_expr te_expr;

/* Variable binding for te_compile.
 * Only name and address fields are used. */
typedef struct te_variable {
    const char   *name;
    const double *address;
} te_variable;


/* Parses the input expression, evaluates it, and frees it.
 * Returns NaN on error.
 * *error is set to 0 on success, or the 1-based position of the
 * parse error on failure. */
double te_interp(const char *expression, te_int_t *error);

/* Parses, evaluates, and returns a cooked string representation.
 *   - "true"  / "false"  if the expression is "logical" AND the result
 *     is finite and exactly 1.0 or 0.0.
 *   - Numeric formatting otherwise, matching Notepad3's TinyExprToStringA:
 *     near-integer values (fractional part below 1e-15 and magnitude
 *     under 1e21) use `%.21g`, all other (and non-finite) values use
 *     `%.15g`, which snprintf renders as "nan" / "inf" / "-inf" for
 *     NaN / +Inf / -Inf respectively.
 *
 * "Logical" detection is lexical at parenthesis depth 0: the expression
 * is logical if it contains one of `==`, `=`, `!=`, `<>`, `<=`, `>=`,
 * `<`, `>`, `&&`, `||`, leading `!`, the bare keywords `true`/`false`,
 * or an outermost call to AND, OR, NOT, ISERR, ISERROR, ISNA, ISNAN,
 * ISEVEN, or ISODD (case-insensitive). IF / IFS are intentionally NOT
 * treated as logical since they return arbitrary user-supplied values.
 *
 * The returned pointer is to a thread-local internal buffer; it remains
 * valid until the next call from the same thread.
 *
 * *error follows the same convention as te_interp(): 0 on success,
 * 1-based parse error position on failure. */
const char *te_interp_str(const char *expression, te_int_t *error);

/* Returns 1 if `expression` lexically looks like a logical/comparison
 * expression at parenthesis depth 0 (one fully-enclosing outer pair is
 * stripped first). Considered logical when it contains one of `==`, `=`,
 * `!=`, `<>`, `<=`, `>=`, `<`, `>`, `&&`, `||`, leading `!`, the bare
 * keywords `true` / `false`, or an outermost call to AND, OR, NOT,
 * ISERR, ISERROR, ISNA, ISNAN, ISEVEN, ISODD (case-insensitive).
 * Returns 0 otherwise. Does NOT evaluate the expression. */
int te_is_logical_expr(const char *expression);

/* Parses the input expression and binds variables.
 * Returns NULL on error.
 * *error is set to 0 on success, or the 1-based error position on failure. */
te_expr *te_compile(const char *expression, const te_variable *variables, int var_count, te_int_t *error);

/* Evaluates a previously compiled expression.
 * Variable bindings are re-read on each call. */
double te_eval(te_expr *n);

/* Frees the compiled expression.
 * Safe to call on NULL pointers. */
void te_free(te_expr *n);

/* Returns 1 if the parser's internal numeric type can represent uint64_t
 * without loss of precision, 0 otherwise.
 * (For the default `double` te_type the answer is 0; bitwise ops are
 * therefore limited to ~52 bits in practice.) */
int te_supports_64bit(void);


/* ---- NP3-specific utility functions (inline) ---- */

/* ANSI codepage of te operators */
inline unsigned te_cp(void) { return 1252U; }

/* Invalid default char for conversion */
inline unsigned te_invalid_chr(void) { return (unsigned)'#'; }

/* Check if character is a digit */
inline unsigned te_is_num(const char * const pch) { return (pch && (*pch > 47) && (*pch < 58)); }

/* Check for operator or special character */
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
    case ';':
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
#endif

#endif /* TINYEXPR_CIF_H */
