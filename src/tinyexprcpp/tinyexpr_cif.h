/* encoding: UTF-8
 * TinyExpr CPP : C-Interface
 */

#ifndef __TE_C_INTERFACE_H__
#define __TE_C_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN64
  typedef __int64          te_int_t;
  #define TE_XINT_FMT      "%lli" 
#else
  typedef int              te_int_t;
  #define TE_XINT_FMT      "%i"
#endif


/* Parses the input expression, evaluates it, and frees it. */
/* Returns NaN on error. */
double te_interp(const char * expression, te_int_t * error);

typedef struct te_variable_c {
    const char* name;
    const double* address;
} te_variable_c;

/* Parses the input expression and binds variables. */
/* Returns NULL on error. */
double te_evaluate(const char* expression, const te_variable_c variables[], int var_count, te_int_t* error);


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

#endif /*__TE_C_INTERFACE_H__*/
