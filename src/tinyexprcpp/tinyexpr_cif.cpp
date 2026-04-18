/* encoding: UTF-8
 *
 * C Interface for TinyExpr++ (tinyexpr_cif)
 *
 * Implementation of the C-compatible wrapper around TinyExpr++.
 *
 * NOTE: This file intentionally does NOT include tinyexpr_cif.h to avoid
 * name conflicts between the C-interface types (te_variable struct,
 * te_expr opaque type) and the C++ TinyExpr++ types (te_variable class,
 * te_expr class). Function signatures are kept in sync manually.
 * This is a standard C/C++ interop pattern.
 */

#include "tinyexpr.h"  // C++ TinyExpr++ header

#include <algorithm>
#include <climits>
#include <clocale>
#include <cmath>
#include <limits>
#include <set>
#include <string>

// ---------------------------------------------------------------------------
// Internal types
// ---------------------------------------------------------------------------

#ifdef _WIN64
typedef __int64 te_int_t;
#else
typedef int     te_int_t;
#endif

// Mirror of the C te_variable struct from tinyexpr_cif.h
struct te_c_variable {
    const char   *name;
    const double *address;
};

// The opaque handle returned to C callers as te_expr*.
// Contains a te_parser that holds the compiled expression and variable bindings.
struct te_cif_handle {
    te_parser parser;
};

// ---------------------------------------------------------------------------
// Compatibility: old tinyexpr C library provided log(x) = log10(x).
// TinyExpr++ removed log(); users must use ln() or log10() explicitly.
// Register "log" as a custom function so existing expressions keep working.
// ---------------------------------------------------------------------------
static te_type te_compat_log(te_type x) { return std::log10(x); }

static void te_cif_add_compat_functions(te_parser &parser)
{
    parser.add_variable_or_function({ "log", te_compat_log });
}

// ---------------------------------------------------------------------------
// Locale-aware separator configuration.
// Queries the C runtime locale (set by Notepad3's SetMuiLocaleAll) to
// determine the decimal point character.  If the locale uses comma (','),
// configure TinyExpr++ to match: decimal = ',' , list = ';'.
// Otherwise keep the US defaults: decimal = '.' , list = ','.
// ---------------------------------------------------------------------------
static void te_cif_configure_separators(te_parser &parser)
{
    const struct lconv *lc = localeconv();
    if (lc && lc->decimal_point && lc->decimal_point[0] == ',') {
        parser.set_decimal_separator(',');
        parser.set_list_separator(';');
    }
    // else: keep defaults ('.' and ',')
}

// ---------------------------------------------------------------------------
// Helper: map TinyExpr++ error state to old 1-based error position
// Old convention: 0 = success, >= 1 = 1-based error position
// ---------------------------------------------------------------------------
static te_int_t map_error(const te_parser &parser)
{
    if (parser.success()) {
        return 0;
    }
    int64_t pos = parser.get_last_error_position();
    if (pos < 0) {
        return 1; // unknown error position — report position 1
    }
    return static_cast<te_int_t>(pos + 1); // convert 0-based to 1-based
}

// ---------------------------------------------------------------------------
// Exported C interface functions
// ---------------------------------------------------------------------------

extern "C" {

// Parses, evaluates, and returns the result. Returns NaN on error.
double te_interp(const char *expression, te_int_t *error)
{
    if (!expression || !*expression) {
        if (error) {
            *error = 1;
        }
        return std::numeric_limits<double>::quiet_NaN();
    }

    te_parser parser;
    te_cif_add_compat_functions(parser);
    te_cif_configure_separators(parser);
    double result;
    try {
        result = parser.evaluate(expression);
    }
    catch (...) {
        if (error) {
            *error = 1;
        }
        return std::numeric_limits<double>::quiet_NaN();
    }

    if (error) {
        *error = map_error(parser);
    }
    return result;
}

// Compiles an expression with bound variables. Returns NULL on error.
// The 'variables' parameter points to an array of te_c_variable structs
// (ABI-compatible with the te_variable declared in tinyexpr_cif.h).
void *te_compile(const char *expression, const void *variables, int var_count, te_int_t *error)
{
    if (!expression || !*expression) {
        if (error) {
            *error = 1;
        }
        return nullptr;
    }

    auto *ctx = new (std::nothrow) te_cif_handle();
    if (!ctx) {
        if (error) {
            *error = 1;
        }
        return nullptr;
    }

    try {
        // Register compatibility functions and configure locale separators
        te_cif_add_compat_functions(ctx->parser);
        te_cif_configure_separators(ctx->parser);

        // Convert C variable array to C++ te_variable set
        if (variables && var_count > 0) {
            const auto *cvars = static_cast<const te_c_variable *>(variables);
            std::set<te_variable> cppVars;
            for (int i = 0; i < var_count; ++i) {
                if (cvars[i].name && cvars[i].address) {
                    cppVars.insert({ std::string(cvars[i].name), cvars[i].address });
                }
            }
            ctx->parser.set_variables_and_functions(std::move(cppVars));
        }

        // Compile and evaluate once to check for errors
        (void)ctx->parser.evaluate(expression);

        if (!ctx->parser.success()) {
            if (error) {
                *error = map_error(ctx->parser);
            }
            delete ctx;
            return nullptr;
        }
    }
    catch (...) {
        if (error) {
            *error = 1;
        }
        delete ctx;
        return nullptr;
    }

    if (error) {
        *error = 0;
    }
    return ctx;
}

// Evaluates a compiled expression. Variable values are re-read via their pointers.
double te_eval(void *n)
{
    if (!n) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    auto *ctx = static_cast<te_cif_handle *>(n);
    try {
        return ctx->parser.evaluate();
    }
    catch (...) {
        return std::numeric_limits<double>::quiet_NaN();
    }
}

// Frees the compiled expression. Safe to call on NULL.
void te_free(void *n)
{
    if (n) {
        delete static_cast<te_cif_handle *>(n);
    }
}

} // extern "C"
