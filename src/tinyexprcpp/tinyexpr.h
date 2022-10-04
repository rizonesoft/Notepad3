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

/*
 * @@@: Used to mark changes from original code according to ZLib license
 */

#ifndef __TINYEXPR_PLUS_PLUS_H__
#define __TINYEXPR_PLUS_PLUS_H__

// @@@: >>>
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1
#endif
// <<< @@@:

#include <vector>
#include <variant>
#include <string>
#include <limits>
#include <initializer_list>
#include <algorithm>
#include <random>
#include <string_view>
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <cassert>
#include <cctype>
#include <set>

class te_expr;

// regular functions
using te_fun0 = double (*)();
using te_fun1 = double (*)(double);
using te_fun2 = double (*)(double, double);
using te_fun3 = double (*)(double, double, double);
using te_fun4 = double (*)(double, double, double, double);
using te_fun5 = double (*)(double, double, double, double, double);
using te_fun6 = double (*)(double, double, double, double, double, double);
using te_fun7 = double (*)(double, double, double, double, double, double, double);
// context functions (where te_variable passes a client's te_expr as the first argument)
using te_confun0 = double (*)(const te_expr*);
using te_confun1 = double (*)(const te_expr*, double);
using te_confun2 = double (*)(const te_expr*, double, double);
using te_confun3 = double (*)(const te_expr*, double, double, double);
using te_confun4 = double (*)(const te_expr*, double, double, double, double);
using te_confun5 = double (*)(const te_expr*, double, double, double, double, double);
using te_confun6 = double (*)(const te_expr*, double, double, double, double, double, double);
using te_confun7 = double (*)(const te_expr*, double, double, double, double, double, double, double);

// do not change the ordering of these, the indices are used to determine the value type of a te_variable
using variant_type = std::variant<double, const double*, // indices 0-1
    te_fun0, te_fun1, te_fun2, te_fun3, te_fun4, te_fun5, te_fun6, te_fun7, // indices 2-9
    te_confun0, te_confun1, te_confun2, te_confun3, te_confun4, te_confun5, te_confun6, te_confun7>; //indices 10-17

/// A variable's flags, effecting how it is evaluated.
/// @note This is a bitmask, so flags can be OR'ed.
enum variable_flags
    {
    // note that because this is a bitmask, don't declare it as an enum class, just a C-style enum
    TE_DEFAULT = 0, // don't do anything special when evaluating
    TE_PURE = 1 << 1, // don't update when simple evaluation is ran (i.e., only updated when expression is compiled)
    TE_VARIADIC = 1 << 2 // function that can take 1-7 argument (unused arguments are set to NaN).
    };

// turns off const in various places for debug builds.
#ifdef NDEBUG
    #define TE_RELEASE_CONST const
#else
    #define TE_RELEASE_CONST
#endif

// Case insensitive comparison for char strings.
class case_insensitive_char_traits
    {
public:
    using char_type = char;
    using int_type = int;
    using off_type = std::streamoff;
    using pos_type = std::streampos;
    using state_type = std::mbstate_t;

    static bool eq_int_type(const int_type& i1, const int_type& i2) noexcept
        { return tolower(static_cast<char_type>(i1)) == tolower(static_cast<char_type>(i2)); }
    static constexpr int_type eof() noexcept
        { return static_cast<int_type>(EOF); }
    static constexpr int_type not_eof(const int_type& i) noexcept
        {
        // EOF is negative, so 0 != EOF
        return (i == static_cast<int_type>(EOF)) ? 0 : 1;
        }
    static constexpr char_type to_char_type(const int_type& i) noexcept
        { return static_cast<char_type>(i); }
    static constexpr int_type to_int_type(const char_type& c) noexcept
        { return static_cast<unsigned char>(c); }

    inline static size_t length(const char_type* s) noexcept
        { return std::strlen(s); }

    inline static void assign(char_type& dst, const char_type src) noexcept
        { dst = src; }
    inline static char_type* assign(char_type* dst, size_t n, char_type c) noexcept
        { return static_cast<char_type*>(std::memset(dst, c, n)); }

    inline static char_type* move(char_type* dst, const char_type* src, size_t n) noexcept
        { return static_cast<char_type*>(std::memmove(dst, src, n)); }
    inline static char_type* copy(char_type* strDest, const char_type* strSource, size_t count) noexcept
        { return std::strncpy(strDest, strSource, count); }

    inline static bool eq(const char_type& first, const char_type& second) noexcept
        {
        return (tolower(first) == tolower(second) );
        }

    inline static bool lt(const char_type& first, const char_type& second) noexcept
        {
        return (tolower(first) < tolower(second) );
        }

    static char_type tolower(const char_type& ch) noexcept
        {
        return static_cast<char_type>(std::tolower(static_cast<unsigned char>(ch)));
        }

    static int compare(const char_type* s1, const char_type* s2, size_t n) noexcept
        {
        assert(s1); assert(s2);
        if (s1 == nullptr)
            { return -1; }
        else if (s2 == nullptr)
            { return 1; }
        for (size_t i = 0; i < n; ++i)
            {
            if (!eq(s1[i], s2[i]) )
                {
                return lt(s1[i], s2[i]) ? -1 : 1;
                }
            }
        return 0;
        }

    static const char_type* find(const char_type* s1, size_t n,
                                 const char_type ch) noexcept
        {
        assert(s1);
        if (s1 == nullptr)
            { return nullptr; }
        for (size_t i = 0; i < n; ++i)
            {
            if (eq(s1[i], ch) )
                {
                return s1+i;
                }
            }
        return nullptr;
        }

    static const char_type* find(const char_type* s1, size_t n1,
                                 const char_type* s2, size_t n2) noexcept
        {
        assert(n1 && n2);
        assert(s1); assert(s2);
        if (s1 == nullptr || s2 == nullptr || (n2 > n1))
            { return nullptr; }
        size_t j = 1;
        for (size_t i = 0; i < n1; i+=j)
            {
            //if the first character of the substring matches then start comparing
            if (eq(s1[i], s2[0]) )
                {
                //if only looking for one character then return
                if (n2 == 1)
                    {
                    return s1+i;
                    }
                //already know the first chars match, so start at next one
                for (j = 1; j < n2; ++j)
                    {
                    if (!eq(s1[i+j], s2[j]) )
                        {
                        break;
                        }
                    }
                //if every character matched then return it
                if (n2 == j)
                    {
                    return s1+i;
                    }
                }
            }
        return nullptr;
        }
    };

/// A compiled expression.
/// Can also be an additional object that can be passed to te_confun0-te_confun7 functions via a te_variable.
class te_expr
    {
public:
    te_expr(const variable_flags type, const variant_type& value) noexcept : m_type(type), m_value(value) {}
    explicit te_expr(const variable_flags type) noexcept : m_type(type) {}
    te_expr() noexcept {}
    te_expr(const te_expr&) = delete;
    te_expr& operator=(const te_expr&) = delete;
    virtual ~te_expr() {}
    /// The type that m_value represents.
    variable_flags m_type{ TE_DEFAULT };
    /// The double constant, double pointer, or function to bind to.
    variant_type m_value;
    /// Additional parameters.
    std::vector<te_expr*> m_parameters{ nullptr };
    };

/// Custom variable or function that can be added to a te_parser.
class te_variable
    {
public:
    /// The name as it would appear in a formula.
    std::basic_string<char, case_insensitive_char_traits> m_name;
    /// The double constant, double pointer, or function to bind the name to.
    variant_type m_value;
    /// The type that m_value represents.
    variable_flags m_type{ TE_DEFAULT };
    /// If m_value is a function pointer of type te_confun0-te_confun7, then
    /// this is passed to that function when called. This is useful for passing
    /// an object which manages additional data to your functions.
    te_expr* m_context{ nullptr };
    // @@@: c'tor
    te_variable(std::basic_string<char, case_insensitive_char_traits> name, variant_type value, variable_flags type)
        : m_name(name), m_value(value), m_type(type), m_context(nullptr) {}
};

/// Math formula parser.
class te_parser
    {
public:
    te_parser() noexcept {}
    te_parser(const te_parser&) = delete;
    te_parser(te_parser&&) = delete;
    te_parser& operator=(const te_parser&) = delete;
    te_parser& operator=(te_parser&&) = delete;
    ~te_parser() { te_free(m_compiledExpression); }
    /** Parses the input @c expression.
        @param expression The formula to compile.
        @returns Whether the expression compiled or not.
        @sa success().*/
    bool compile(const char* expression);
    /** Evaluates expression passed to compile() previuosly and returns its result.
        @returns The result, or NaN on error.*/
    [[nodiscard]] double evaluate();
    /** Compiles and evaluates an expression and returns its result.
        @param expression The formula to compile and evaluate.
        @returns The result, or NaN on error.*/
    [[nodiscard]] double evaluate(const char* expression);
    /// @returns The last call to evaluate()'s result (which will be NaN on error).
    [[nodiscard]] double get_result() const noexcept
        { return m_result; }
    /// @returns Whether the last call to compile() was successful.
    /// @sa get_last_error_position().
    [[nodiscard]] bool success() const noexcept
        { return m_parseSuccess; }
    /// @returns The zero-based index into the last parsed expression where the parse failed, or -1 if no error occurred.
    /// @note Call success() to see if the last parse succeeded or not.
    [[nodiscard]] int64_t get_last_error_position() const noexcept
        { return m_errorPos; }

    /// Sets the list of custom variables and functions.
    /// @param vars The list of variables and functions.
    void set_vars(const std::vector<te_variable>& vars)
        {
        m_vars = vars;
        std::sort(m_vars.begin(), m_vars.end(),
            [](const auto& lhv, const auto& rhv) noexcept { return lhv.m_name < rhv.m_name; });
        }
    /// @returns The list of custom variables and functions.
    [[nodiscard]] const std::vector<te_variable>& get_vars() const noexcept
        { return m_vars; }

    /// @returns The decimal separator used for nunmbers.
    [[nodiscard]] char get_decimal_separator() const noexcept
        { return m_decimalSeparator; }
    /// Sets the decimal separator used for nunmbers.
    /// @param sep The decimal separator.
    void set_decimal_separator(const char sep)  noexcept
        { m_decimalSeparator = sep; }

    /// Changes a constant variable's value.
    /// @param name The name of the (constant) variable.
    /// @param value The new value to set the constant to.
    /// @returns True if the constant variable was found and updates, false otherwise.
    bool set_constant(const char* name, const double value)
        {
        auto cvar = find_variable(name);
        if (cvar == get_vars().end() || !is_constant(cvar->m_value))
            { return false; }
        else
            {
            cvar->m_value = value;
            // if previously compiled, then re-compile since this constant would have been optimized
            if (m_expression.length())
                { compile(m_expression.c_str()); }
            return true;
            }
        }
    /// Retrieves a constant variable's value.
    /// @param name The name of the (constant) variable.
    /// @returns The value of the constant variable if found, NaN otherwise.
    double get_constant(const char* name) const
        {
        auto cvar = find_variable(name);
        try
            {
            return (cvar == get_vars().end() || !is_constant(cvar->m_value)) ?
                std::numeric_limits<double>::quiet_NaN() :
                std::get<double>(cvar->m_value);
            }
        catch (std::bad_variant_access const& ex)
            {
            printf(ex.what());
            return std::numeric_limits<double>::quiet_NaN();
            }
        }

    /// @returns The separator used between function arguments.
    [[nodiscard]] char get_list_separator() const noexcept
        { return m_listSeparator; }
    /// Sets the separator used between function arguments.
    /// @param sep The list separator.
    void set_list_separator(const char sep)  noexcept
        { m_listSeparator = sep; }
    
    /// @returns True if @c name is a function that had been used in the last parsed formula.
    /// @param name The name of the function.
    /// @sa compile() and evaluate().
    [[nodiscard]] bool is_function_used(const char* name) const
        { return m_usedFunctions.find(std::basic_string<char, case_insensitive_char_traits>(name)) != m_usedFunctions.cend(); }
    /// @returns True if @c name is a variable that had been used in the last parsed formula.
    /// @param name The name of the variable.
    /// @sa compile() and evaluate().
    [[nodiscard]] bool is_variable_used(const char* name) const
        { return m_usedVars.find(std::basic_string<char, case_insensitive_char_traits>(name)) != m_usedVars.cend(); }
private:
    /// @returns The list of custom variables and functions.
    [[nodiscard]] std::vector<te_variable>& get_vars() noexcept
        { return m_vars; }
    /// @returns An iterator to the custom variable or function with the given @c name, or end of get_vars() if not found.
    [[nodiscard]] std::vector<te_variable>::iterator find_variable(const char* name)
        {
        if (!name) return m_vars.end();
        // debug sanity check
        assert(std::is_sorted(m_vars.cbegin(), m_vars.cend(),
            [](const auto& lhv, const auto& rhv) noexcept { return lhv.m_name < rhv.m_name; }));

        const auto foundPos = std::lower_bound(m_vars.begin(), m_vars.end(),
            std::basic_string_view<char, case_insensitive_char_traits>(name),
            [](const auto& var, const auto& sv) noexcept { return var.m_name < sv; });
        // did it find an exact match?
        return (foundPos != m_vars.end() && foundPos->m_name.compare(0, foundPos->m_name.length(), name) == 0) ? foundPos : m_vars.end();
        }
    /// @returns An iterator to the custom variable or function with the given @c name, or end of get_vars() if not found.
    [[nodiscard]] std::vector<te_variable>::const_iterator find_variable(const char* name) const
        {
        if (!name) return m_vars.cend();
        // debug sanity check
        assert(std::is_sorted(m_vars.cbegin(), m_vars.cend(),
            [](const auto& lhv, const auto& rhv) noexcept { return lhv.m_name < rhv.m_name; }));

        const auto foundPos = std::lower_bound(m_vars.cbegin(), m_vars.cend(),
            std::basic_string_view<char, case_insensitive_char_traits>(name),
            [](const auto& var, const auto& sv) noexcept { return var.m_name < sv; });
        // did it find an exact match?
        return (foundPos != m_vars.cend() && foundPos->m_name.compare(0, foundPos->m_name.length(), name) == 0) ? foundPos : m_vars.cend();
        }
    [[nodiscard]] constexpr static auto is_pure(const variable_flags type)
        { return (((type)&TE_PURE) != 0); }
    [[nodiscard]] constexpr static auto is_variadic(const variable_flags type)
        { return (((type)&TE_VARIADIC) != 0); }
    /// @returns Number of parameters that a function/variable takes.
    [[nodiscard]] inline static auto get_arity(const variant_type& var) noexcept
        {
        return (var.index() == 0 || var.index() == 1) ? 0 :
            (is_function0(var) || is_closure0(var)) ? 0 :
            (is_function1(var) || is_closure1(var)) ? 1 :
            (is_function2(var) || is_closure2(var)) ? 2 :
            (is_function3(var) || is_closure3(var)) ? 3 :
            (is_function4(var) || is_closure4(var)) ? 4 :
            (is_function5(var) || is_closure5(var)) ? 5 :
            (is_function6(var) || is_closure6(var)) ? 6 :
            (is_function7(var) || is_closure7(var)) ? 7 :
            0;
        }
    [[nodiscard]] constexpr static bool is_constant(const variant_type& var) noexcept
        { return var.index() == 0; }
    [[nodiscard]] constexpr static double get_constant(const variant_type& var)
        {
        assert(std::holds_alternative<double>(var));
        return std::get<0>(var);
        }
    [[nodiscard]] constexpr static bool is_variable(const variant_type& var) noexcept
        { return var.index() == 1; }
    [[nodiscard]] constexpr static const double* get_variable(const variant_type& var)
        {
        assert(std::holds_alternative<const double*>(var));
        return std::get<1>(var);
        }
    [[nodiscard]] constexpr static bool is_function(const variant_type& var) noexcept
        { return (var.index() >= 2 && var.index() <= 9); }
    [[nodiscard]] constexpr static bool is_function0(const variant_type& var) noexcept
        { return var.index() == 2; }
    [[nodiscard]] constexpr static te_fun0 get_function0(const variant_type& var)
        {
        assert(std::holds_alternative<te_fun0>(var));
        return std::get<2>(var);
        }
    [[nodiscard]] constexpr static bool is_function1(const variant_type& var) noexcept
        { return var.index() == 3; }
    [[nodiscard]] constexpr static te_fun1 get_function1(const variant_type& var)
        {
        assert(std::holds_alternative<te_fun1>(var));
        return std::get<3>(var);
        }
    [[nodiscard]] constexpr static bool is_function2(const variant_type& var) noexcept
        { return var.index() == 4; }
    [[nodiscard]] constexpr static te_fun2 get_function2(const variant_type& var)
        {
        assert(std::holds_alternative<te_fun2>(var));
        return std::get<4>(var);
        }
    [[nodiscard]] constexpr static bool is_function3(const variant_type& var) noexcept
        { return var.index() == 5; }
    [[nodiscard]] constexpr static te_fun3 get_function3(const variant_type& var)
        {
        assert(std::holds_alternative<te_fun3>(var));
        return std::get<5>(var);
        }
    [[nodiscard]] constexpr static bool is_function4(const variant_type& var) noexcept
        { return var.index() == 6; }
    [[nodiscard]] constexpr static te_fun4 get_function4(const variant_type& var)
        {
        assert(std::holds_alternative<te_fun4>(var));
        return std::get<6>(var);
        }
    [[nodiscard]] constexpr static bool is_function5(const variant_type& var) noexcept
        { return var.index() == 7; }
    [[nodiscard]] constexpr static te_fun5 get_function5(const variant_type& var)
        {
        assert(std::holds_alternative<te_fun5>(var));
        return std::get<7>(var);
        }
    [[nodiscard]] constexpr static bool is_function6(const variant_type& var) noexcept
        { return var.index() == 8; }
    [[nodiscard]] constexpr static te_fun6 get_function6(const variant_type& var)
        {
        assert(std::holds_alternative<te_fun6>(var));
        return std::get<8>(var);
        }
    [[nodiscard]] constexpr static bool is_function7(const variant_type& var) noexcept
        { return var.index() == 9; }
    [[nodiscard]] constexpr static te_fun7 get_function7(const variant_type& var)
        {
        assert(std::holds_alternative<te_fun7>(var));
        return std::get<9>(var);
        }
    [[nodiscard]] constexpr static bool is_closure(const variant_type& var) noexcept
        { return (var.index() >= 10 && var.index() <= 17); }
    [[nodiscard]] constexpr static bool is_closure0(const variant_type& var) noexcept
        { return var.index() == 10; }
    [[nodiscard]] constexpr static te_confun0 get_closure0(const variant_type& var)
        {
        assert(std::holds_alternative<te_confun0>(var));
        return std::get<10>(var);
        }
    [[nodiscard]] constexpr static bool is_closure1(const variant_type& var) noexcept
        { return var.index() == 11; }
    [[nodiscard]] constexpr static te_confun1 get_closure1(const variant_type& var)
        {
        assert(std::holds_alternative<te_confun1>(var));
        return std::get<11>(var);
        }
    [[nodiscard]] constexpr static bool is_closure2(const variant_type& var) noexcept
        { return var.index() == 12; }
    [[nodiscard]] constexpr static te_confun2 get_closure2(const variant_type& var)
        {
        assert(std::holds_alternative<te_confun2>(var));
        return std::get<12>(var);
        }
    [[nodiscard]] constexpr static bool is_closure3(const variant_type& var) noexcept
        { return var.index() == 13; }
    [[nodiscard]] constexpr static te_confun3 get_closure3(const variant_type& var)
        {
        assert(std::holds_alternative<te_confun3>(var));
        return std::get<13>(var);
        }
    [[nodiscard]] constexpr static bool is_closure4(const variant_type& var) noexcept
        { return var.index() == 14; }
    [[nodiscard]] constexpr static te_confun4 get_closure4(const variant_type& var)
        {
        assert(std::holds_alternative<te_confun4>(var));
        return std::get<14>(var);
        }
    [[nodiscard]] constexpr static bool is_closure5(const variant_type& var) noexcept
        { return var.index() == 15; }
    [[nodiscard]] constexpr static te_confun5 get_closure5(const variant_type& var)
        {
        assert(std::holds_alternative<te_confun5>(var));
        return std::get<15>(var);
        }
    [[nodiscard]] constexpr static bool is_closure6(const variant_type& var) noexcept
        { return var.index() == 16; }
    [[nodiscard]] constexpr static te_confun6 get_closure6(const variant_type& var)
        {
        assert(std::holds_alternative<te_confun6>(var));
        return std::get<16>(var);
        }
    [[nodiscard]] constexpr static bool is_closure7(const variant_type& var) noexcept
        { return var.index() == 17; }
    [[nodiscard]] constexpr static te_confun7 get_closure7(const variant_type& var)
        {
        assert(std::holds_alternative<te_confun7>(var));
        return std::get<17>(var);
        }

    struct state
        {
        enum class token_type
            {
            TOK_NULL, TOK_ERROR, TOK_END, TOK_SEP, TOK_OPEN,
            TOK_CLOSE, TOK_NUMBER, TOK_VARIABLE, TOK_FUNCTION, TOK_INFIX
            };
        state(const char* expression, variable_flags varType,
            TE_RELEASE_CONST std::vector<te_variable>& vars) :
            m_start(expression), m_next(expression),
            m_varType(varType), m_lookup(vars)
            {}
        const char* m_start{ nullptr };
        const char* m_next{ nullptr };
        token_type m_type{ token_type::TOK_NULL };
        variable_flags m_varType{ TE_DEFAULT };
        variant_type m_value;
        te_expr* context{ nullptr };

        TE_RELEASE_CONST std::vector<te_variable>& m_lookup;
        };
    [[nodiscard]] static te_expr* new_expr(const variable_flags type, const variant_type& value, const std::initializer_list<te_expr*> parameters);
    [[nodiscard]] static constexpr bool is_letter(const char c) noexcept
        { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
    /* Parses the input expression and binds variables.
       @returns NULL on error. */
    [[nodiscard]] te_expr* te_compile(const char* expression, TE_RELEASE_CONST std::vector<te_variable>& variables);
    /* Evaluates the expression. */
    [[nodiscard]] static double te_eval(const te_expr* n);
    /* Frees the expression. */
    /* This is safe to call on NULL pointers. */
    static void te_free(te_expr* n);
    static void te_free_parameters(te_expr* n);
    static void optimize(te_expr* n);
    [[nodiscard]] static auto find_builtin(const char* name, const size_t len)
        {
        // debug sanity check
        assert(std::is_sorted(m_functions.cbegin(), m_functions.cend(),
            [](const auto& lhv, const auto& rhv) noexcept { return lhv.m_name < rhv.m_name; }));

        const auto foundPos = std::lower_bound(m_functions.cbegin(), m_functions.cend(),
            std::basic_string_view<char, case_insensitive_char_traits>(name, len),
            [](const auto& var, const auto& sv) noexcept { return var.m_name < sv; });
        // did it find an exact match?
        return (foundPos != m_functions.cend() && foundPos->m_name.compare(0, foundPos->m_name.length(), name, len) == 0) ? foundPos : m_functions.cend();
        }

    [[nodiscard]] static auto find_lookup(TE_RELEASE_CONST state* s,
                                          const char* name, const size_t len)
        {
        // debug sanity checks
        assert(std::is_sorted(s->m_lookup.cbegin(), s->m_lookup.cend(),
            [](const auto& lhv, const auto& rhv) noexcept { return lhv.m_name < rhv.m_name; }));
#ifndef NDEBUG
        // see if there are any duplicates in the vars/functions
        if (s->m_lookup.size())
            {
            std::sort(s->m_lookup.begin(), s->m_lookup.end(),
                [](const auto& lhv, const auto& rhv) noexcept { return lhv.m_name < rhv.m_name; });
            auto uniqueEnd = std::adjacent_find(s->m_lookup.begin(), s->m_lookup.end(),
                [](auto& lhv, auto& rhv) noexcept { return lhv.m_name == rhv.m_name; });

            if (uniqueEnd != s->m_lookup.end())
                {
                fprintf(stderr, ("\n'" + uniqueEnd->m_name + "' was entered in the custom variable/function list twice!\n").c_str());
                assert(0 && "Custom variable/function list twice!");
                }
            }
#endif

        const auto foundPos = std::lower_bound(s->m_lookup.cbegin(), s->m_lookup.cend(),
            std::basic_string_view<char, case_insensitive_char_traits>(name, len),
            [](const auto& var, const auto& sv) noexcept { return var.m_name < sv; });
        // did it find an exact match?
        return (foundPos != s->m_lookup.cend() && foundPos->m_name.compare(0, foundPos->m_name.length(), name, len) == 0) ? foundPos : s->m_lookup.cend();
        }

#ifndef NDEBUG
    /* Prints debugging information on the syntax tree. */
    static void te_print(const te_expr* n, int depth);
#endif

    void next_token(state* s);
    [[nodiscard]] te_expr* base(state* s);
    [[nodiscard]] te_expr* power(state* s);
    [[nodiscard]] te_expr* factor(state* s);
    [[nodiscard]] te_expr* term(state* s);
    [[nodiscard]] te_expr* expr(state* s);
    [[nodiscard]] te_expr* list(state* s);

    std::string m_expression;
    te_expr* m_compiledExpression{ nullptr };

    std::vector<te_variable>::const_iterator m_currentVar;
    bool m_varFound{ false };
    std::set<std::basic_string<char, case_insensitive_char_traits>> m_usedFunctions;
    std::set<std::basic_string<char, case_insensitive_char_traits>> m_usedVars;

    static const std::vector<te_variable> m_functions;
    std::vector<te_variable> m_vars;

    bool m_parseSuccess{ false };
    int64_t m_errorPos{ 0 };
    double m_result{ std::numeric_limits<double>::quiet_NaN() };
    char m_decimalSeparator{ '.' };
    char m_listSeparator{ ',' };
    };

#endif // __TINYEXPR_PLUS_PLUS_H__
