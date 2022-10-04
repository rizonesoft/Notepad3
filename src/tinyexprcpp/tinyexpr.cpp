// SPDX-License-Identifier: Zlib
/*
 * TINYEXPR - Tiny recursive descent parser and evaluation engine in C
 *
 * Copyright (c) 2015-2020 Lewis Van Winkle
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

/*
 * TINYEXPR++ - Tiny recursive descent parser and evaluation engine in C++
 * Copyright (c) 2020-2021 Blake Madden
 * 
 * C++ version of the TinyExpr library.
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

// builtin functions
[[nodiscard]] constexpr static double _equal(double a, double b) noexcept { return (a == b) ? 1 : 0; }
[[nodiscard]] constexpr static double _not_equal(double a, double b) noexcept { return (a != b) ? 1 : 0; }
[[nodiscard]] constexpr static double _less_than(double a, double b) noexcept { return (a < b) ? 1 : 0; }
[[nodiscard]] constexpr static double _less_than_equal_to(double a, double b) noexcept { return (a <= b) ? 1 : 0; }
[[nodiscard]] constexpr static double _greater_than(double a, double b) noexcept { return (a > b) ? 1 : 0; }
[[nodiscard]] constexpr static double _greater_than_equal_to(double a, double b) noexcept { return (a >= b) ? 1 : 0; }
[[nodiscard]] constexpr static double _and(double a, double b) noexcept { return (a && b) ? 1 : 0; }
[[nodiscard]] constexpr static double _or(double a, double b) noexcept { return (a || b) ? 1 : 0; }
[[nodiscard]] constexpr static double _not(double a) noexcept { return !a; }
[[nodiscard]] constexpr static double pi() noexcept { return 3.14159265358979323846; }
[[nodiscard]] constexpr static double e() noexcept { return 2.71828182845904523536; }
[[nodiscard]] static double fac(double a) noexcept {/* simplest version of factorial */
    if (a < 0.0 || std::isnan(a))
        return std::numeric_limits<double>::quiet_NaN();
    if (a > std::numeric_limits<unsigned int>::max())
        return std::numeric_limits<double>::infinity();
    const auto ua = static_cast<size_t>(a);
    unsigned long int result{ 1 }, i{ 1 };
    for (i = 1; i <= ua; i++) {
        if (i > std::numeric_limits<unsigned long>::max() / result)
            return std::numeric_limits<double>::infinity();
        result *= i;
    }
    return static_cast<double>(result);
}
[[nodiscard]] static double random()
    {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distr(0.0, 1.0);
    return distr(gen);
    }
[[nodiscard]] constexpr static double divide(double a, double b)
    {
    if (b == 0)
        { throw std::exception("Division by zero."); }
    return a / b;
    }
[[nodiscard]] static double modulus(double a, double b)
    {
    if (b == 0)
        { throw std::exception("Modulus by zero."); }
    return std::fmod(a,b);
    }
[[nodiscard]] static double sum(double v1, double v2, double v3, double v4,
                                double v5, double v6, double v7) noexcept
    {
    return (std::isnan(v1) ? 0 : v1) +
        (std::isnan(v2) ? 0 : v2) +
        (std::isnan(v3) ? 0 : v3) +
        (std::isnan(v4) ? 0 : v4) +
        (std::isnan(v5) ? 0 : v5) +
        (std::isnan(v6) ? 0 : v6) +
        (std::isnan(v7) ? 0 : v7);
    }
[[nodiscard]] static double average(double v1, double v2, double v3, double v4,
                                    double v5, double v6, double v7) noexcept
    {
    const auto validN = (std::isnan(v1) ? 0 : 1) +
        (std::isnan(v2) ? 0 : 1) +
        (std::isnan(v3) ? 0 : 1) +
        (std::isnan(v4) ? 0 : 1) +
        (std::isnan(v5) ? 0 : 1) +
        (std::isnan(v6) ? 0 : 1) +
        (std::isnan(v7) ? 0 : 1);
    const auto total = sum(v1, v2, v3, v4, v5, v6, v7);
    return divide(total, validN);
    }
[[nodiscard]] static double _round(double val, double decimal_places) noexcept
    {
    const size_t decimalPlaces{ std::isnan(decimal_places) ? 0 : static_cast<size_t>(decimal_places) };
    const size_t decimalPostition = (decimalPlaces == 0) ? 0 :
        (decimalPlaces == 1) ? 10 : (decimalPlaces == 2) ? 100 :
        (decimalPlaces == 3) ? 1000 : (decimalPlaces == 4) ? 10000 :
        (decimalPlaces == 5) ? 100000 : (decimalPlaces >= 6) ? 1000000 :
        10;

    if (val < 0)
        {
        return (decimalPostition == 0) ? std::ceil(val -0.5f) :
            std::ceil((val * decimalPostition) - 0.5f)/decimalPostition;
        }
    else
        {
        return (decimalPostition == 0) ? std::floor(val +0.5f) :
            std::floor((val * decimalPostition) + 0.5f)/decimalPostition;
        }
    }
// Combinations (without repetition)
[[nodiscard]] static double ncr(double n, double r) noexcept {
    if (n < 0.0 || r < 0.0 || n < r || std::isnan(n) || std::isnan(r)) return std::numeric_limits<double>::quiet_NaN();
    if (n > std::numeric_limits<unsigned int>::max() || r > std::numeric_limits<unsigned int>::max()) return std::numeric_limits<double>::infinity();
    const unsigned long int un = static_cast<unsigned int>(n);
    unsigned long int ur = static_cast<unsigned int>(r);
    unsigned long int result = 1;
    if (ur > un / 2) ur = un - ur;
    for (decltype(ur) i = 1; i <= ur; i++) {
        if (result > std::numeric_limits<unsigned long>::max() / (un - ur + i))
            return std::numeric_limits<double>::infinity();
        result *= un - ur + i;
        result /= i;
    }
    return static_cast<double>(result);
}
// Permutations (without repetition)
[[nodiscard]] static double npr(double n, double r) noexcept { return ncr(n, r) * fac(r); }
[[nodiscard]] constexpr static double add(double a, double b) noexcept { return a + b; }
[[nodiscard]] constexpr static double sub(double a, double b) noexcept { return a - b; }
[[nodiscard]] constexpr static double mul(double a, double b) noexcept { return a * b; }
[[nodiscard]] constexpr static double _sqr(double a) noexcept { return a*a; }
[[nodiscard]] static double _max_maybe_nan(double v1, double v2_maybe_nan) noexcept
    { return std::max(v1, std::isnan(v2_maybe_nan) ? v1 : v2_maybe_nan); }
[[nodiscard]] static double _max(double v1, double v2, double v3, double v4,
                                 double v5, double v6, double v7) noexcept
    {
    // assumes that at least v1 is a number, rest can be NaN
    auto maxVal = _max_maybe_nan(v1, v2);
    maxVal = _max_maybe_nan(maxVal, v3);
    maxVal = _max_maybe_nan(maxVal, v4);
    maxVal = _max_maybe_nan(maxVal, v5);
    maxVal = _max_maybe_nan(maxVal, v6);
    return _max_maybe_nan(maxVal, v7);
    }
[[nodiscard]] static double _min_maybe_nan(double v1, double v2_maybe_nan) noexcept
    { return std::min(v1, std::isnan(v2_maybe_nan) ? v1 : v2_maybe_nan); }
[[nodiscard]] static double _min(double v1, double v2, double v3, double v4,
                                 double v5, double v6, double v7) noexcept
    {
    // assumes that at least v1 is legit, rest can be NaN
    auto minVal = _min_maybe_nan(v1, v2);
    minVal = _min_maybe_nan(minVal, v3);
    minVal = _min_maybe_nan(minVal, v4);
    minVal = _min_maybe_nan(minVal, v5);
    minVal = _min_maybe_nan(minVal, v6);
    return _min_maybe_nan(minVal, v7);
    }
[[nodiscard]] static double _and_maybe_nan(double v1, double v2_maybe_nan) noexcept
    { return std::isnan(v2_maybe_nan) ? v1 : (v1 && v2_maybe_nan); }
[[nodiscard]] static double _and_variadic(double v1, double v2, double v3, double v4,
                                          double v5, double v6, double v7) noexcept
    {
    // assumes that at least v1 is legit, rest can be NaN
    auto andVal = _and_maybe_nan(v1, v2);
    andVal = _and_maybe_nan(andVal, v3);
    andVal = _and_maybe_nan(andVal, v4);
    andVal = _and_maybe_nan(andVal, v5);
    andVal = _and_maybe_nan(andVal, v6);
    return _and_maybe_nan(andVal, v7);
    }
[[nodiscard]] static double _or_maybe_nan(double v1, double v2_maybe_nan) noexcept
    { return std::isnan(v2_maybe_nan) ? v1 : (v1 || v2_maybe_nan); }
[[nodiscard]] static double _or_variadic(double v1, double v2, double v3, double v4,
                                          double v5, double v6, double v7) noexcept
    {
    // assumes that at least v1 is legit, rest can be NaN
    auto orVal = _or_maybe_nan(v1, v2);
    orVal = _or_maybe_nan(orVal, v3);
    orVal = _or_maybe_nan(orVal, v4);
    orVal = _or_maybe_nan(orVal, v5);
    orVal = _or_maybe_nan(orVal, v6);
    return _or_maybe_nan(orVal, v7);
    }
[[nodiscard]] constexpr static double _if(double a, double b, double c) noexcept
    { return (a != 0.0) ? b : c; }
// cotangent
[[nodiscard]] static double cot(double a) noexcept
    {
    if (a == 0.0)
        { return std::numeric_limits<double>::quiet_NaN(); }
    return 1 / std::tan(a);
    }
[[nodiscard]] constexpr static double sign(double a) noexcept
    { return (a < 0.0) ? -1 : (a > 0.0) ? 1 : 0; }
[[nodiscard]] constexpr static double negate(double a) noexcept { return -a; }
[[nodiscard]] constexpr static double comma([[maybe_unused]] double a, double b) noexcept { return b; }

te_expr* te_parser::new_expr(const variable_flags type, const variant_type& value, const std::initializer_list<te_expr*> parameters)
    {
    const auto arity = get_arity(value);
    te_expr* ret = new te_expr;
    ret->m_parameters.resize(
        std::max<size_t>(
            std::max<size_t>(parameters.size(), arity) + (is_closure(value) ? 1 : 0),
            1)
        );
    if (parameters.size())
        { std::copy(parameters.begin(), parameters.end(), ret->m_parameters.begin()); }
    ret->m_type = type;
    ret->m_value = 0;
    return ret;
    }

void te_parser::te_free_parameters(te_expr *n)
    {
    if (!n) return;
    if (is_closure(n->m_value))
        {
        // last param is the context object, we don't manage that here
        for (auto param = n->m_parameters.begin(); param != n->m_parameters.end() - 1; ++param)
            {
            te_free(*param);
            *param = nullptr;
            }
        }
    else if (is_function(n->m_value))
        {
        for (auto param = n->m_parameters.begin(); param != n->m_parameters.end(); ++param)
            {
            te_free(*param);
            *param = nullptr;
            }
        }
    }

void te_parser::te_free(te_expr *n)
    {
    if (!n) return;
    te_free_parameters(n);
    delete n;
    }

const std::vector<te_variable> te_parser::m_functions = {
    /* must be in alphabetical order */
    {"abs",   static_cast<te_fun1>(std::fabs),  TE_PURE},
    {"acos",  static_cast<te_fun1>(std::acos),  TE_PURE},
    {"and",  static_cast<te_fun7>(_and_variadic), static_cast<variable_flags>(TE_PURE|TE_VARIADIC)}, // variadic, accepts 1-7 arguments
    {"asin",  static_cast<te_fun1>(std::asin),  TE_PURE},
    {"atan",  static_cast<te_fun1>(std::atan),  TE_PURE},
    {"atan2", static_cast<te_fun2>(std::atan2), TE_PURE},
    {"average",  static_cast<te_fun7>(average), static_cast<variable_flags>(TE_PURE|TE_VARIADIC)},
    {"ceil",  static_cast<te_fun1>(std::ceil),  TE_PURE},
    {"clamp",  static_cast<te_fun3>(
        [](const double num, const double start, const double end)
            {
            if (start > end)
                { throw std::exception("Error in CLAMP: start of range cannot be larger than end of range."); }
            return std::clamp<double>(num, start, end);
            }),
        TE_PURE},
    {"combin",   static_cast<te_fun2>(ncr),   TE_PURE},
    {"cos",   static_cast<te_fun1>(std::cos),   TE_PURE},
    {"cosh",  static_cast<te_fun1>(std::cosh),  TE_PURE},
    {"cot",  static_cast<te_fun1>(cot),  TE_PURE},
    {"e",     static_cast<te_fun0>(e),     TE_PURE},
    {"exp",   static_cast<te_fun1>(std::exp),   TE_PURE},
    {"fac",   static_cast<te_fun1>(fac),   TE_PURE},
    {"fact",   static_cast<te_fun1>(fac),   TE_PURE},
    {"floor", static_cast<te_fun1>(std::floor), TE_PURE},
    {"if", static_cast<te_fun3>(_if), TE_PURE},
    {"ln",    static_cast<te_fun1>(std::log),   TE_PURE},
#ifdef TE_NAT_LOG
    {"log",   static_cast<te_fun1>(std::log),   TE_PURE},
#else
    {"log",   static_cast<te_fun1>(std::log10), TE_PURE},
#endif
    {"log10", static_cast<te_fun1>(std::log10), TE_PURE},
    {"max",  static_cast<te_fun7>(_max),  static_cast<variable_flags>(TE_PURE|TE_VARIADIC)},
    {"min",  static_cast<te_fun7>(_min),  static_cast<variable_flags>(TE_PURE|TE_VARIADIC)},
    {"mod",  static_cast<te_fun2>(modulus),  TE_PURE},
    {"ncr",   static_cast<te_fun2>(ncr),   TE_PURE},
    {"not",  static_cast<te_fun1>(_not),  TE_PURE},
    {"npr",   static_cast<te_fun2>(npr),   TE_PURE},
    {"or",  static_cast<te_fun7>(_or_variadic),  static_cast<variable_flags>(TE_PURE|TE_VARIADIC)},
    {"permut",   static_cast<te_fun2>(npr),   TE_PURE},
    {"pi",    static_cast<te_fun0>(pi),    TE_PURE},
    {"pow",   static_cast<te_fun2>(std::pow),   TE_PURE},
    {"power",/* Excel alias*/   static_cast<te_fun2>(std::pow),   TE_PURE},
    {"rand",    static_cast<te_fun0>(random),    TE_PURE},
    {"round",   static_cast<te_fun2>(_round),   static_cast<variable_flags>(TE_PURE|TE_VARIADIC)},
    {"sign",   static_cast<te_fun1>(sign),   TE_PURE},
    {"sin",   static_cast<te_fun1>(std::sin),   TE_PURE},
    {"sinh",  static_cast<te_fun1>(std::sinh),  TE_PURE},
    {"sqr",  static_cast<te_fun1>(_sqr),  TE_PURE},
    {"sqrt",  static_cast<te_fun1>(std::sqrt),  TE_PURE},
    {"sum",  static_cast<te_fun7>(sum),  static_cast<variable_flags>(TE_PURE|TE_VARIADIC)},
    {"tan",   static_cast<te_fun1>(std::tan),   TE_PURE},
    {"tanh",  static_cast<te_fun1>(std::tanh),  TE_PURE},
    {"trunc",  static_cast<te_fun1>(std::trunc),  TE_PURE}
};

void te_parser::next_token(te_parser::state *s)
    {
    assert(s);
    if (!s)
        { return; }

    s->m_type = te_parser::state::token_type::TOK_NULL;

    do
        {
        if (!*s->m_next)
            {
            s->m_type = te_parser::state::token_type::TOK_END;
            return;
            }

        /* Try reading a number. */
        if ((s->m_next[0] >= '0' && s->m_next[0] <= '9') || s->m_next[0] == get_decimal_separator())
            {
            char* nEnd{ nullptr };
            s->m_value = std::strtod(s->m_next, &nEnd);
            s->m_next = nEnd;
            s->m_type = te_parser::state::token_type::TOK_NUMBER;
            }
        else
            {
            /* Look for a variable or builtin function call. */
            if (is_letter(s->m_next[0]))
                {
                const char* start = s->m_next;
                while (is_letter(s->m_next[0]) || (s->m_next[0] >= '0' && s->m_next[0] <= '9') || (s->m_next[0] == '_')) s->m_next++;

                m_varFound = false;
                m_currentVar = find_lookup(s, start, s->m_next - start);
                if (m_currentVar != s->m_lookup.cend())
                    { m_varFound = true; }
                else
                    {
                    m_currentVar = find_builtin(start, s->m_next - start);
                    if (m_currentVar != m_functions.cend())
                        { m_varFound = true; }
                    }

                if (!m_varFound)
                    {
                    s->m_type = te_parser::state::token_type::TOK_ERROR;
                    }
                else
                    {
                    // keep track of what's been used in the formula
                    if (is_function(m_currentVar->m_value) || is_closure(m_currentVar->m_value))
                        { m_usedFunctions.insert(m_currentVar->m_name); }
                    else
                        { m_usedVars.insert(m_currentVar->m_name); }

                    if (is_constant(m_currentVar->m_value))
                        {
                        s->m_type = te_parser::state::token_type::TOK_NUMBER;
                        s->m_value = m_currentVar->m_value;
                        }
                    else if (is_variable(m_currentVar->m_value))
                        {
                        s->m_type = te_parser::state::token_type::TOK_VARIABLE;
                        s->m_value = m_currentVar->m_value;
                        }
                    else if (is_closure(m_currentVar->m_value))
                        {
                        s->context = m_currentVar->m_context;
                        s->m_type = te_parser::state::token_type::TOK_FUNCTION;
                        s->m_varType = m_currentVar->m_type;
                        s->m_value = m_currentVar->m_value;
                        }
                    else if (is_function(m_currentVar->m_value))
                        {
                        s->m_type = te_parser::state::token_type::TOK_FUNCTION;
                        s->m_varType = m_currentVar->m_type;
                        s->m_value = m_currentVar->m_value;
                        }
                    }
                }
            else
                {
                /* Look for an operator or special character. */
                const auto tok = s->m_next++[0];
                if (tok == '+') { s->m_type = te_parser::state::token_type::TOK_INFIX; s->m_value = add; }
                else if (tok == '-') { s->m_type = te_parser::state::token_type::TOK_INFIX; s->m_value = sub; }
                else if (tok == '*') { s->m_type = te_parser::state::token_type::TOK_INFIX; s->m_value = mul; }
                else if (tok == '/') { s->m_type = te_parser::state::token_type::TOK_INFIX; s->m_value = divide; }
                else if (tok == '^') { s->m_type = te_parser::state::token_type::TOK_INFIX; s->m_value = static_cast<te_fun2>(std::pow); }
                else if (tok == '%') { s->m_type = te_parser::state::token_type::TOK_INFIX; s->m_value = modulus; }
                else if (tok == '(') { s->m_type = te_parser::state::token_type::TOK_OPEN; }
                else if (tok == ')') { s->m_type = te_parser::state::token_type::TOK_CLOSE; }
                else if (tok == get_list_separator()) { s->m_type = te_parser::state::token_type::TOK_SEP; }
                // logical operators
                else if (tok == '=') { s->m_type = te_parser::state::token_type::TOK_INFIX; s->m_value = static_cast<te_fun2>(_equal); }
                else if (tok == '<' && s->m_next[0] == '>') { s->m_type = te_parser::state::token_type::TOK_INFIX; s->m_value = static_cast<te_fun2>(_not_equal); ++s->m_next; }
                else if (tok == '<' && s->m_next[0] == '=') { s->m_type = te_parser::state::token_type::TOK_INFIX; s->m_value = static_cast<te_fun2>(_less_than_equal_to); ++s->m_next; }
                else if (tok == '<') { s->m_type = te_parser::state::token_type::TOK_INFIX; s->m_value = static_cast<te_fun2>(_less_than); }
                else if (tok == '>' && s->m_next[0] == '=') { s->m_type = te_parser::state::token_type::TOK_INFIX; s->m_value = static_cast<te_fun2>(_greater_than_equal_to); ++s->m_next; }
                else if (tok == '>') { s->m_type = te_parser::state::token_type::TOK_INFIX; s->m_value = static_cast<te_fun2>(_greater_than); }
                else if (tok == '&') { s->m_type = te_parser::state::token_type::TOK_INFIX; s->m_value = static_cast<te_fun2>(_and); }
                else if (tok == '|') { s->m_type = te_parser::state::token_type::TOK_INFIX; s->m_value = static_cast<te_fun2>(_or); }
                else if (tok == ' ' || tok == '\t' || tok == '\n' || tok == '\r') { /*noop*/ }
                else { s->m_type = te_parser::state::token_type::TOK_ERROR; }
                }
            }
        } while (s->m_type == te_parser::state::token_type::TOK_NULL);
    }

te_expr* te_parser::base(te_parser::state *s)
    {
    /* <base>      =    <constant> | <variable> | <function-0> {"(" ")"} | <function-1> <power> | <function-X> "(" <expr> {"," <expr>} ")" | "(" <list> ")" */
    te_expr* ret{ nullptr };
    int arity{ 0 };

    if (s->m_type == te_parser::state::token_type::TOK_OPEN)
        {
        next_token(s);
        ret = list(s);
        if (s->m_type != te_parser::state::token_type::TOK_CLOSE)
            { s->m_type = te_parser::state::token_type::TOK_ERROR; }
        else
            { next_token(s); }
        }
    else if (s->m_type == te_parser::state::token_type::TOK_NUMBER)
        {
        ret = new_expr(TE_DEFAULT, s->m_value, {});
        ret->m_value = s->m_value;
        next_token(s);
        }
    else if (s->m_type == te_parser::state::token_type::TOK_VARIABLE)
        {
        ret = new_expr(TE_DEFAULT, s->m_value, {});
        ret->m_value = s->m_value;
        next_token(s);
        }
    else if (s->m_type == te_parser::state::token_type::TOK_NULL ||
        s->m_type == te_parser::state::token_type::TOK_ERROR ||
        s->m_type == te_parser::state::token_type::TOK_END ||
        s->m_type == te_parser::state::token_type::TOK_SEP ||
        s->m_type == te_parser::state::token_type::TOK_CLOSE ||
        s->m_type == te_parser::state::token_type::TOK_INFIX)
        {
        ret = new_expr(TE_DEFAULT, variant_type(static_cast<double>(0.0)), {});
        s->m_type = te_parser::state::token_type::TOK_ERROR;
        ret->m_value = std::numeric_limits<double>::quiet_NaN();
        }
    else if (is_function0(s->m_value) || is_closure0(s->m_value))
        {
        ret = new_expr(s->m_varType, s->m_value, {});
        ret->m_value = s->m_value;
        if (is_closure(s->m_value)) ret->m_parameters[0] = s->context;
        next_token(s);
        if (s->m_type == te_parser::state::token_type::TOK_OPEN)
            {
            next_token(s);
            if (s->m_type != te_parser::state::token_type::TOK_CLOSE)
                { s->m_type = te_parser::state::token_type::TOK_ERROR; }
            else
                { next_token(s); }
            }
        }
    else if (is_function1(s->m_value) || is_closure1(s->m_value))
        {
        ret = new_expr(s->m_varType, s->m_value, { nullptr });
        ret->m_value = s->m_value;
        if (is_closure(s->m_value)) ret->m_parameters[1] = s->context;
        next_token(s);
        ret->m_parameters[0] = power(s);
        }
    else if (is_function2(s->m_value) || is_closure2(s->m_value) ||
        is_function3(s->m_value) || is_closure3(s->m_value) ||
        is_function4(s->m_value) || is_closure4(s->m_value) ||
        is_function5(s->m_value) || is_closure5(s->m_value) ||
        is_function6(s->m_value) || is_closure6(s->m_value) ||
        is_function7(s->m_value) || is_closure7(s->m_value))
        {
        arity = get_arity(s->m_value);

        ret = new_expr(s->m_varType, s->m_value, {});
        ret->m_value = s->m_value;
        if (is_closure(s->m_value)) ret->m_parameters[arity] = s->context;
        next_token(s);

        if (s->m_type != te_parser::state::token_type::TOK_OPEN)
            { s->m_type = te_parser::state::token_type::TOK_ERROR; }
        else
            {
            int i{ 0 };
            // If there are vars or other functions in the parameters, keep track of the original
            // opening function; that is what we will do our variadic check on.
            const auto varValid{ m_varFound };
            const auto openingVar = m_currentVar;
            // load any parameters
            for(i = 0; i < arity; i++)
                {
                next_token(s);
                ret->m_parameters[i] = expr(s);
                if(s->m_type != te_parser::state::token_type::TOK_SEP)
                    { break; }
                }
            if (s->m_type == te_parser::state::token_type::TOK_CLOSE && i != arity - 1 &&
                varValid && is_variadic(openingVar->m_type))
                { next_token(s); }
            else if(s->m_type != te_parser::state::token_type::TOK_CLOSE || i != arity - 1)
                { s->m_type = te_parser::state::token_type::TOK_ERROR; }
            else
                { next_token(s); }
            }
        }

    return ret;
    }

te_expr* te_parser::power(te_parser::state *s) {
    /* <power>     =    {("-" | "+" | "&" | "|")} <base> */
    int Sign{ 1 };
    while (s->m_type == te_parser::state::token_type::TOK_INFIX &&
        is_function2(s->m_value) &&
           (get_function2(s->m_value) == add || get_function2(s->m_value) == sub ||
            get_function2(s->m_value) == _and || get_function2(s->m_value) == _or ||
            get_function2(s->m_value) == _equal ||
            get_function2(s->m_value) == _not_equal ||
            get_function2(s->m_value) == _less_than ||
            get_function2(s->m_value) == _less_than_equal_to ||
            get_function2(s->m_value) == _greater_than ||
            get_function2(s->m_value) == _greater_than_equal_to))
        {
        if (get_function2(s->m_value) == sub) Sign = -Sign;
        next_token(s);
        }

    te_expr* ret{ nullptr };

    if (Sign == 1) {
        ret = base(s);
    } else {
        ret = new_expr(TE_PURE, variant_type(negate), { base(s) });
        ret->m_value = negate;
    }

    return ret;
}

#ifdef TE_POW_FROM_RIGHT
te_expr *factor(te_parser::state *s) {
    /* <factor>    =    <power> {"^" <power>} */
    te_expr* ret = power(s);

    int neg = 0;

    if (ret->m_type == (TE_PURE) &&
        is_function1(ret->m_value) &&
        get_function1(ret->m_value) == negate) {
        te_expr *se = ret->m_parameters[0];
        free(ret);
        ret = se;
        neg = 1;
    }

    te_expr *insertion = 0;
    while (s->m_type == te_parser::state::token_type::TOK_INFIX &&
        is_function2(ret->m_value) &&
           (get_function2(ret->m_value) == static_cast<te_fun2>(std::pow))) {
        const te_fun2 t = get_function2(s->m_value);
        next_token(s);

        if (insertion) {
            /* Make exponentiation go right-to-left. */
            te_expr* insert = new_expr(TE_PURE, t, { insertion->m_parameters[1], power(s) });
            insert->m_value = t;
            insertion->m_parameters[1] = insert;
            insertion = insert;
        } else {
            ret = new_expr(TE_PURE, t, { ret, power(s) });
            ret->m_value = t;
            insertion = ret;
        }
    }

    if (neg) {
        ret = new_expr(TE_PURE, variant_type(negate), { ret });
        ret->m_value = negate;
    }

    return ret;
}
#else
te_expr* te_parser::factor(te_parser::state *s) {
    /* <factor>    =    <power> {"^" <power>} */
    te_expr* ret = power(s);
    while (s->m_type == te_parser::state::token_type::TOK_INFIX &&
        is_function2(s->m_value) &&
        (get_function2(s->m_value) == static_cast<te_fun2>(std::pow))) {
        const te_fun2 t = get_function2(s->m_value);
        next_token(s);
        ret = new_expr(TE_PURE, t, { ret, power(s) });
        ret->m_value = t;
    }

    return ret;
}
#endif

te_expr* te_parser::term(te_parser::state *s) {
    /* <term>      =    <factor> {("*" | "/" | "%") <factor>} */
    te_expr* ret = factor(s);
    while (s->m_type == te_parser::state::token_type::TOK_INFIX &&
        is_function2(s->m_value) &&
        (get_function2(s->m_value) == mul || get_function2(s->m_value) == divide ||
         get_function2(s->m_value) == modulus)) {
        const te_fun2 t = get_function2(s->m_value);
        next_token(s);
        ret = new_expr(TE_PURE, t, { ret, factor(s) });
        ret->m_value = t;
    }

    return ret;
}

te_expr* te_parser::expr(te_parser::state *s) {
    /* <expr>      =    <term> {("+" | "-" | "&" | "|") <term>} */
    te_expr* ret = term(s);

    while (s->m_type == te_parser::state::token_type::TOK_INFIX &&
        is_function2(s->m_value) &&
        (get_function2(s->m_value) == add ||
         get_function2(s->m_value) == _and ||
         get_function2(s->m_value) == _or ||
         get_function2(s->m_value) == _equal ||
         get_function2(s->m_value) == _not_equal ||
         get_function2(s->m_value) == _less_than ||
         get_function2(s->m_value) == _less_than_equal_to ||
         get_function2(s->m_value) == _greater_than ||
         get_function2(s->m_value) == _greater_than_equal_to ||
         get_function2(s->m_value) == sub))
        {
        const te_fun2 t = get_function2(s->m_value);
        next_token(s);
        ret = new_expr(TE_PURE, t, { ret, term(s) });
        ret->m_value = t;
        }

    return ret;
}

te_expr* te_parser::list(te_parser::state *s) {
    /* <list>      =    <expr> {"," <expr>} */
    te_expr* ret = expr(s);

    while (s->m_type == te_parser::state::token_type::TOK_SEP) {
        next_token(s);
        ret = new_expr(TE_PURE, variant_type(comma), { ret, expr(s) });
        ret->m_value = comma;
    }

    return ret;
}

double te_parser::te_eval(const te_expr *n)
    {
    if (!n) return std::numeric_limits<double>::quiet_NaN();

    const auto M = [&n = std::as_const(n)](const size_t e) constexpr noexcept
        {
        return (e < n->m_parameters.size()) ? te_eval(n->m_parameters[e]) :
            std::numeric_limits<double>::quiet_NaN();
        };

    if (is_constant(n->m_value))
        { return get_constant(n->m_value); }
    else if (is_variable(n->m_value))
        { return *(get_variable(n->m_value)); }
    else if (is_function0(n->m_value))
        { return get_function0(n->m_value)(); }
    else if (is_function1(n->m_value))
        { return get_function1(n->m_value)( M(0) ); }
    else if (is_function2(n->m_value))
        { return get_function2(n->m_value)(M(0), M(1)); }
    else if (is_function3(n->m_value))
        { return get_function3(n->m_value)(M(0), M(1), M(2)); }
    else if (is_function4(n->m_value))
        { return get_function4(n->m_value)(M(0), M(1), M(2), M(3)); }
    else if (is_function5(n->m_value))
        { return get_function5(n->m_value)(M(0), M(1), M(2), M(3), M(4)); }
    else if (is_function6(n->m_value))
        { return get_function6(n->m_value)(M(0), M(1), M(2), M(3), M(4), M(5)); }
    else if (is_function7(n->m_value))
        { return get_function7(n->m_value)(M(0), M(1), M(2), M(3), M(4), M(5), M(6)); }
    else if (is_closure0(n->m_value))
        { return get_closure0(n->m_value)(n->m_parameters[0]); }
    else if (is_closure1(n->m_value))
        { return get_closure1(n->m_value)(n->m_parameters[1], M(0)); }
    else if (is_closure2(n->m_value))
        { return get_closure2(n->m_value)(n->m_parameters[2], M(0), M(1)); }
    else if (is_closure3(n->m_value))
        { return get_closure3(n->m_value)(n->m_parameters[3], M(0), M(1), M(2)); }
    else if (is_closure4(n->m_value))
        { return get_closure4(n->m_value)(n->m_parameters[4], M(0), M(1), M(2), M(3)); }
    else if (is_closure5(n->m_value))
        { return get_closure5(n->m_value)(n->m_parameters[5], M(0), M(1), M(2), M(3), M(4)); }
    else if (is_closure6(n->m_value))
        { return get_closure6(n->m_value)(n->m_parameters[6], M(0), M(1), M(2), M(3), M(4), M(5)); }
    else if (is_closure7(n->m_value))
        { return get_closure7(n->m_value)(n->m_parameters[7], M(0), M(1), M(2), M(3), M(4), M(5), M(6)); }
    else
        { return std::numeric_limits<double>::quiet_NaN(); }
    }

void te_parser::optimize(te_expr *n)
    {
    if (!n) return;
    /* Evaluates as much as possible. */
    if (is_constant(n->m_value) || is_variable(n->m_value)) return;

    /* Only optimize out functions flagged as pure. */
    if (is_pure(n->m_type))
        {
        const int arity = get_arity(n->m_value);
        bool known{ true };
        for (int i = 0; i < arity; ++i)
            {
            if (!n->m_parameters[i])
                { break; }
            optimize(n->m_parameters[i]);
            if (!is_constant(n->m_parameters[i]->m_value))
                { known = false; }
            }
        if (known)
            {
            const double value = te_eval(n);
            te_free_parameters(n);
            n->m_type = TE_DEFAULT;
            n->m_value = value;
            }
        }
    }

te_expr* te_parser::te_compile(const char* expression, TE_RELEASE_CONST std::vector<te_variable>& variables)
    {
    state s(expression, TE_DEFAULT, variables);

    next_token(&s);
    te_expr* root = list(&s);

    if (s.m_type != te_parser::state::token_type::TOK_END)
        {
        te_free(root);
        m_errorPos = (s.m_next - s.m_start);
        if (m_errorPos > 0) --m_errorPos;
        return nullptr;
        }
    else
        {
        optimize(root);
        m_errorPos = -1;
        return root;
        }
    }

bool te_parser::compile(const char* expression)
    {
    assert(expression && "compile() should not be called with null!");
    // reset everything from previous call
    m_errorPos = -1;
    m_result = std::numeric_limits<double>::quiet_NaN();
    m_parseSuccess = false;
    te_free(m_compiledExpression);
    m_compiledExpression = nullptr;
    m_currentVar = m_functions.cend();
    m_varFound = false;
    m_usedFunctions.clear();
    m_usedVars.clear();
    if (!expression)
        {
        m_expression.clear();
        return std::numeric_limits<double>::quiet_NaN();
        }
    m_expression.assign(expression);

    m_compiledExpression = te_compile(expression, get_vars());
    m_parseSuccess = (m_compiledExpression) ? true : false;
    return m_parseSuccess;
    }

double te_parser::evaluate()
    {
    m_result = (m_compiledExpression) ? te_eval(m_compiledExpression) : std::numeric_limits<double>::quiet_NaN();
    return m_result;
    }

double te_parser::evaluate(const char* expression)
    {
    if (compile(expression))
        { return evaluate(); }
    else
        { return std::numeric_limits<double>::quiet_NaN(); }
    }

#ifndef NDEBUG
void te_parser::te_print(const te_expr *n, int depth)
    {
    printf("%*s", depth, "");

    if (is_function(n->m_value) || is_closure(n->m_value))
        {
        int arity = get_arity(n->m_value);
        printf("f%d", arity);
        for (int i = 0; i < arity; i++)
            { printf(" %p", n->m_parameters[i]); }
        printf("\n");
        for (int i = 0; i < arity; i++)
            { te_print(n->m_parameters[i], depth + 1); }
        }
    else if (is_constant(n->m_value))
        { printf("%f\n", get_constant(n->m_value)); }
    else if (is_variable(n->m_value))
        { printf("bound %p\n", get_variable(n->m_value)); }
    }
#endif
