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
#include <array>
#include <cctype>
#include <climits>
#include <clocale>
#include <cmath>
#include <cstdio>
#include <cstring>
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
// Pre-pass: rewrite C-style binary literals `0b<bits>` (or `0B<bits>`) to their
// decimal equivalent, since TinyExpr++ tokenizes via std::strtod which only
// accepts `0x` (hex) and decimal/scientific. Hex is left alone (parser handles
// it). Octal is not rewritten — TinyExpr++ has no concept of octal literals
// either, but binary is the only base we currently emit on output, so input
// parity here matters most.
//
// Detection rule: `0b` is recognized as a binary literal only when it begins
// at a token-start position (start-of-string, after whitespace, an operator,
// `(`, `,`, `;`, etc.). Inside a variable name (e.g. `var0b1`) it is left as-is.
// ---------------------------------------------------------------------------
static std::string te_cif_rewrite_binary_literals(const char *expression)
{
    std::string out;
    if (!expression) {
        return out;
    }
    out.reserve(std::strlen(expression));

    bool numStart = true; // expression start counts as a token boundary
    const unsigned char *p = reinterpret_cast<const unsigned char *>(expression);
    while (*p) {
        if (numStart && p[0] == '0' && (p[1] == 'b' || p[1] == 'B') &&
            (p[2] == '0' || p[2] == '1')) {
            const unsigned char *digits = p + 2;
            const unsigned char *end    = digits;
            unsigned long long   value  = 0;
            while (*end == '0' || *end == '1') {
                if (end - digits < 64) { // silently clamp at 64 bits
                    value = (value << 1) | static_cast<unsigned long long>(*end - '0');
                }
                ++end;
            }
            char buf[32];
            std::snprintf(buf, sizeof(buf), "%llu", value);
            out.append(buf);
            p        = end;
            numStart = false;
            continue;
        }
        unsigned char const c = *p;
        out.push_back(static_cast<char>(c));
        switch (c) {
        case '+': case '-': case '*': case '/': case '%': case '^':
        case '(': case ',': case ';':
        case '<': case '>': case '=': case '!':
        case '|': case '&': case ':':
        case ' ': case '\t': case '\n': case '\r':
            numStart = true;
            break;
        default:
            numStart = false;
            break;
        }
        ++p;
    }
    return out;
}

// ---------------------------------------------------------------------------
// Boolean-expression detection (for te_interp_str)
//
// An expression is classified as "logical" when, at parenthesis depth 0,
// it contains any of:
//   - relational / equality / logical operators:
//     `==` `=` `!=` `<>` `<=` `>=` `<` `>` `&&` `||` `!`  (unary or `!=`)
//   - the bare keywords `true` / `false`
//   - an outermost call to AND, OR, NOT, ISERR, ISERROR, ISNA, ISNAN,
//     ISEVEN, ISODD
// IF / IFS are intentionally excluded - they return arbitrary user values
// (`IF(a>b, 5, 10)` is not a boolean expression even though `a>b` is).
//
// Block / line comments are skipped. Bit-shift `<<`, `>>` and bit-rotate
// `<<<`, `>>>` are explicitly consumed without triggering classification.
// ---------------------------------------------------------------------------
namespace {

constexpr std::array<const char *, 9> kLogicalFunctions = {
    "and",    "or",      "not",
    "iserr",  "iserror", "isna",  "isnan",
    "iseven", "isodd",
};

inline bool te_is_ident_char(unsigned char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '_' || c == '.';
}

inline char te_ascii_tolower(char c)
{
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c;
}

// Case-insensitive ASCII match of `kw` (lowercase) against expr[pos..].
// Returns the matched length (> 0) on success and only if the byte just
// past the match is NOT an identifier continuation. Returns 0 on no match.
size_t te_match_ci(const std::string &expr, size_t pos, const char *kw)
{
    size_t i = 0;
    while (kw[i]) {
        if (pos + i >= expr.size() || te_ascii_tolower(expr[pos + i]) != kw[i]) {
            return 0;
        }
        ++i;
    }
    if (pos + i < expr.size() &&
        te_is_ident_char(static_cast<unsigned char>(expr[pos + i]))) {
        return 0;
    }
    return i;
}

bool te_is_logical_keyword_at(const std::string &expr, size_t pos)
{
    return te_match_ci(expr, pos, "true") > 0 || te_match_ci(expr, pos, "false") > 0;
}

bool te_is_logical_func_at(const std::string &expr, size_t pos)
{
    for (const char *name : kLogicalFunctions) {
        size_t const consumed = te_match_ci(expr, pos, name);
        if (consumed == 0) {
            continue;
        }
        size_t after = pos + consumed;
        while (after < expr.size() && (expr[after] == ' ' || expr[after] == '\t')) {
            ++after;
        }
        if (after < expr.size() && expr[after] == '(') {
            return true;
        }
    }
    return false;
}

// Strip whitespace and any fully-enclosing outer parenthesis pairs so that
// `(1 == 1)` is classified the same as `1 == 1`.
std::string te_strip_outer_parens(std::string s)
{
    auto trim = [](std::string &t) {
        size_t a = 0;
        while (a < t.size() && std::isspace(static_cast<unsigned char>(t[a]))) ++a;
        size_t b = t.size();
        while (b > a && std::isspace(static_cast<unsigned char>(t[b - 1]))) --b;
        t.assign(t, a, b - a);
    };
    trim(s);
    while (s.size() >= 2 && s.front() == '(' && s.back() == ')') {
        // Find the close-paren that balances the leading '('. If it isn't
        // at the very end, the outer parens don't fully wrap (e.g.
        // "(a)+(b)") - or the parens are unbalanced - so stop.
        int    depth    = 0;
        size_t closePos = std::string::npos;
        for (size_t i = 0; i < s.size(); ++i) {
            if (s[i] == '(') {
                ++depth;
            }
            else if (s[i] == ')' && --depth == 0) {
                closePos = i;
                break;
            }
        }
        if (closePos != s.size() - 1) {
            break;
        }
        s.assign(s, 1, s.size() - 2);
        trim(s);
    }
    return s;
}

bool te_expression_is_logical(const std::string &raw)
{
    const std::string expr = te_strip_outer_parens(raw);
    int               depth       = 0;
    bool              after_ident = false;
    const size_t      n           = expr.size();
    for (size_t i = 0; i < n; ++i) {
        const char c   = expr[i];
        const char nxt = (i + 1 < n) ? expr[i + 1] : '\0';

        // Skip C/C++ comments (mirrors TinyExpr++ parser behavior).
        if (c == '/' && nxt == '*') {
            size_t e = expr.find("*/", i + 2);
            i        = (e == std::string::npos) ? n - 1 : e + 1;
            after_ident = false;
            continue;
        }
        if (c == '/' && nxt == '/') {
            size_t e = expr.find_first_of("\r\n", i + 2);
            i        = (e == std::string::npos) ? n - 1 : e - 1;
            after_ident = false;
            continue;
        }

        if (c == '(') {
            ++depth;
            after_ident = false;
            continue;
        }
        if (c == ')') {
            if (depth > 0) {
                --depth;
            }
            after_ident = false;
            continue;
        }

        if (depth == 0) {
            // `<<`, `>>` (shift) and `<<<`, `>>>` (rotate) are NOT boolean
            // producers; consume and keep scanning. Lone `<` / `>` and the
            // `<=`, `>=`, `<>` variants DO classify as logical and fall
            // through to the return below.
            if ((c == '<' || c == '>') && c == nxt) {
                ++i;
                if (i + 1 < n && expr[i + 1] == c) {
                    ++i;
                }
                after_ident = false;
                continue;
            }

            // Relational / equality / logical operators.
            if (c == '=' || c == '<' || c == '>' || c == '!') {
                return true;
            }
            if ((c == '&' && nxt == '&') || (c == '|' && nxt == '|')) {
                return true;
            }

            // Identifier start: check for boolean keyword / boolean function call.
            if (!after_ident && te_is_ident_char(static_cast<unsigned char>(c)) &&
                !(c >= '0' && c <= '9')) {
                if (te_is_logical_keyword_at(expr, i) ||
                    te_is_logical_func_at(expr, i)) {
                    return true;
                }
            }
        }

        after_ident = te_is_ident_char(static_cast<unsigned char>(c));
    }
    return false;
}

// Mirrors Notepad3's TinyExprToStringA: integer-like values (fractional part
// below 1e-15 and magnitude under 1e21) use `%.21g`; everything else -
// including non-finite NaN / Inf / -Inf - falls through to `%.15g`, which
// snprintf renders as "nan" / "inf" / "-inf". Hex / binary output modes are
// UI-level concerns and remain in TinyExprToStringA proper.
void te_format_number(char *buf, size_t bufSize, double v)
{
    double       intpart  = 0.0;
    double const fracpart = std::modf(v, &intpart);
    if (std::fabs(fracpart) < 1.0E-15 && std::fabs(intpart) < 1.0E+21) {
        std::snprintf(buf, bufSize, "%.21g", intpart);
    }
    else {
        std::snprintf(buf, bufSize, "%.15g", v);
    }
}

} // anonymous namespace

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

    try {
        te_parser parser;
        te_cif_add_compat_functions(parser);
        te_cif_configure_separators(parser);
        std::string const rewritten = te_cif_rewrite_binary_literals(expression);
        double const      result    = parser.evaluate(rewritten);
        if (error) {
            *error = map_error(parser);
        }
        return result;
    }
    catch (...) {
        if (error) {
            *error = 1;
        }
        return std::numeric_limits<double>::quiet_NaN();
    }
}

// Evaluates expression and returns a cooked string.
// Returns "true" / "false" when the source is lexically logical AND the
// result is finite and exactly 1.0 / 0.0; otherwise returns a numeric
// formatting (or "nan" / "inf" / "-inf").
const char *te_interp_str(const char *expression, te_int_t *error)
{
    static thread_local char buf[64];
    constexpr double         kNaN = std::numeric_limits<double>::quiet_NaN();

    if (!expression || !*expression) {
        if (error) {
            *error = 1;
        }
        te_format_number(buf, sizeof(buf), kNaN);
        return buf;
    }

    try {
        te_parser parser;
        te_cif_add_compat_functions(parser);
        te_cif_configure_separators(parser);
        std::string const rewritten = te_cif_rewrite_binary_literals(expression);
        double const      result    = parser.evaluate(rewritten);
        if (error) {
            *error = map_error(parser);
        }

        if (parser.success() && std::isfinite(result) &&
            (result == 0.0 || result == 1.0) &&
            te_expression_is_logical(rewritten)) {
            std::snprintf(buf, sizeof(buf), "%s", result == 1.0 ? "true" : "false");
            return buf;
        }

        te_format_number(buf, sizeof(buf), result);
        return buf;
    }
    catch (...) {
        if (error) {
            *error = 1;
        }
        te_format_number(buf, sizeof(buf), kNaN);
        return buf;
    }
}

// Lexical-only predicate: does this expression look logical?
// See header for the full set of detected operators / keywords / functions.
int te_is_logical_expr(const char *expression)
{
    if (!expression || !*expression) {
        return 0;
    }
    try {
        return te_expression_is_logical(std::string(expression)) ? 1 : 0;
    }
    catch (...) {
        return 0;
    }
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

        // Compile and evaluate once to check for errors. Rewrite 0b… literals first.
        std::string const rewritten = te_cif_rewrite_binary_literals(expression);
        (void)ctx->parser.evaluate(rewritten);

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

// Reports whether the parser's te_type can represent uint64_t losslessly.
// te_parser::supports_64bit() is constexpr; the optimizer will inline the constant.
int te_supports_64bit(void)
{
    return te_parser::supports_64bit() ? 1 : 0;
}

} // extern "C"
