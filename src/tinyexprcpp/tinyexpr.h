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
 *
 * Copyright (c) 2020-2026 Blake Madden
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

#ifndef __TINYEXPR_PLUS_PLUS_H__
#define __TINYEXPR_PLUS_PLUS_H__

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <functional>
#include <initializer_list>
#include <limits>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>
#if __has_include(<bit>)
    #include <bit>
#endif
// type_traits to get n_args.
#include <tuple>
#include <type_traits>

constexpr int TINYEXPR_CPP_MAJOR_VERSION = 1;
constexpr int TINYEXPR_CPP_MINOR_VERSION = 1;
constexpr int TINYEXPR_CPP_PATCH_VERSION = 0;
constexpr int TINYEXPR_CPP_TWEAK_VERSION = 0;
#define TINYEXPR_CPP_COPYRIGHT                                                                     \
    "TinyExpr: Copyright (c) 2015-2020 Lewis Van Winkle\n"                                         \
    "TinyExpr++: Copyright (c) 2020-2026 Blake Madden"

class te_parser;

#if defined(TE_RAND_SEED) && defined(TE_RAND_SEED_TIME)
    #error TE_RAND_SEED and TE_RAND_SEED_TIME compile options cannot be combined. Only one random number generator seeding method can be specified.
#endif

#if defined(TE_FLOAT) && defined(TE_LONG_DOUBLE)
    #error TE_FLOAT and TE_LONG_DOUBLE compile options cannot be combined. Only one data type can be specified.
#endif

/// @brief Define this to use @c float instead of @c double for the parser's data type.
#ifdef TE_FLOAT
/// @brief The parameter and return type for parser and its functions.
using te_type = float;
#elif defined(TE_LONG_DOUBLE)
using te_type = long double;
#else
/// @brief The parameter and return type for parser and its functions.
using te_type = double;
#endif

#if defined(TE_FLOAT) && defined(TE_BITWISE_OPERATORS)
    #error TE_FLOAT and TE_BITWISE_OPERATORS compile options cannot be combined. TE_FLOAT will not support bitwise operations.
#endif

class te_expr;
// clang-format off
// regular functions
using te_fun0  = te_type (*)();
using te_fun1  = te_type (*)(te_type);
using te_fun2  = te_type (*)(te_type, te_type);
using te_fun3  = te_type (*)(te_type, te_type, te_type);
using te_fun4  = te_type (*)(te_type, te_type, te_type, te_type);
using te_fun5  = te_type (*)(te_type, te_type, te_type, te_type, te_type);
using te_fun6  = te_type (*)(te_type, te_type, te_type, te_type, te_type, te_type);
using te_fun7  = te_type (*)(te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_fun8  = te_type (*)(te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_fun9  = te_type (*)(te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_fun10 = te_type (*)(te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_fun11 = te_type (*)(te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_fun12 = te_type (*)(te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_fun13 = te_type (*)(te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_fun14 = te_type (*)(te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_fun15 = te_type (*)(te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_fun16 = te_type (*)(te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_fun17 = te_type (*)(te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_fun18 = te_type (*)(te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_fun19 = te_type (*)(te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_fun20 = te_type (*)(te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_fun21 = te_type (*)(te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_fun22 = te_type (*)(te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_fun23 = te_type (*)(te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_fun24 = te_type (*)(te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
// context functions (where te_variable passes a client's te_expr as the first argument)
using te_confun0  = te_type (*)(const te_expr*);
using te_confun1  = te_type (*)(const te_expr*, te_type);
using te_confun2  = te_type (*)(const te_expr*, te_type, te_type);
using te_confun3  = te_type (*)(const te_expr*, te_type, te_type, te_type);
using te_confun4  = te_type (*)(const te_expr*, te_type, te_type, te_type, te_type);
using te_confun5  = te_type (*)(const te_expr*, te_type, te_type, te_type, te_type, te_type);
using te_confun6  = te_type (*)(const te_expr*, te_type, te_type, te_type, te_type, te_type, te_type);
using te_confun7  = te_type (*)(const te_expr*, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_confun8  = te_type (*)(const te_expr*, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_confun9  = te_type (*)(const te_expr*, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_confun10 = te_type (*)(const te_expr*, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_confun11 = te_type (*)(const te_expr*, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_confun12 = te_type (*)(const te_expr*, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_confun13 = te_type (*)(const te_expr*, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_confun14 = te_type (*)(const te_expr*, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_confun15 = te_type (*)(const te_expr*, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_confun16 = te_type (*)(const te_expr*, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_confun17 = te_type (*)(const te_expr*, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_confun18 = te_type (*)(const te_expr*, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_confun19 = te_type (*)(const te_expr*, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_confun20 = te_type (*)(const te_expr*, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_confun21 = te_type (*)(const te_expr*, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_confun22 = te_type (*)(const te_expr*, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_confun23 = te_type (*)(const te_expr*, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
using te_confun24 = te_type (*)(const te_expr*, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type, te_type);
// clang-format on
template<typename FuncType>
struct te_fun_traits;

// Specialization for function pointers
template<typename ReturnType, typename... Args>
struct te_fun_traits<ReturnType (*)(Args...)>
    {
    using arg_tuple = std::tuple<Args...>;
    constexpr static size_t arity = std::tuple_size<arg_tuple>::value;
    };

// Specialization for function types
template<typename ReturnType, typename... Args>
struct te_fun_traits<ReturnType(Args...)>
    {
    using arg_tuple = std::tuple<Args...>;
    constexpr static size_t arity = std::tuple_size<arg_tuple>::value;
    };

// Helper variable template for easier access
template<typename FuncType>
constexpr size_t te_function_arity = te_fun_traits<FuncType>::arity;

template<typename T>
constexpr bool te_is_constant_v = std::is_same_v<T, te_type>;

template<typename T>
constexpr bool te_is_variable_v = std::is_same_v<T, const te_type*>;

template<typename FuncType>
struct te_is_closure
    {
    constexpr static bool value{ false };
    };

// Specialization for function pointers
template<typename ReturnType, typename... Args>
struct te_is_closure<ReturnType (*)(const te_expr*, Args...)>
    {
    constexpr static bool value{ true };
    };

// Specialization for function types
template<typename ReturnType, typename... Args>
struct te_is_closure<ReturnType(const te_expr*, Args...)>
    {
    constexpr static bool value{ true };
    };

template<typename T>
constexpr bool te_is_closure_v = te_is_closure<T>::value;

template<typename T>
constexpr bool te_is_function_v =
    !te_is_constant_v<T> && !te_is_variable_v<T> && !te_is_closure_v<T>;

// functions for unknown symbol resolution
using te_usr_noop = std::function<void()>;
using te_usr_fun0 = std::function<te_type(std::string_view)>;
using te_usr_fun1 = std::function<te_type(std::string_view, std::string&)>;

using te_usr_variant_type = std::variant<te_usr_noop, te_usr_fun0, te_usr_fun1>;

// do not change the ordering of these, the indices are used to determine
// the value type of te_variable
using te_variant_type =
    std::variant<te_type, const te_type*, // indices 0-1
                 te_fun0, te_fun1, te_fun2, te_fun3, te_fun4, te_fun5, te_fun6, te_fun7, te_fun8,
                 te_fun9, te_fun10, te_fun11, te_fun12, te_fun13, te_fun14, te_fun15, te_fun16,
                 te_fun17, te_fun18, te_fun19, te_fun20, te_fun21, te_fun22, te_fun23, te_fun24,
                 te_confun0, te_confun1, te_confun2, te_confun3, te_confun4, te_confun5, te_confun6,
                 te_confun7, te_confun8, te_confun9, te_confun10, te_confun11, te_confun12,
                 te_confun13, te_confun14, te_confun15, te_confun16, te_confun17, te_confun18,
                 te_confun19, te_confun20, te_confun21, te_confun22, te_confun23, te_confun24>;

/// @brief A variable's flags, effecting how it is evaluated.
/// @note This is a bitmask, so flags (TE_PURE and TE_VARIADIC) can be OR'ed.
/// @internal Note that because this is a bitmask, don't declare it as an enum class,
///     just a C-style enum.
enum te_variable_flags
    {
    /// @brief Don't do anything special when evaluating.
    TE_DEFAULT = 0,
    /// @brief Don't update when simple evaluation is run
    ///     (i.e., only updated when expression is compiled).
    TE_PURE = (1 << 0),
    /// @brief Function that can take 1-24 argument (unused arguments are set to NaN).
    TE_VARIADIC = (1 << 1)
    };

/// @private
class te_string_less
    {
  public:
    [[nodiscard]]
    bool operator()(const std::string& lhv, const std::string& rhv) const
        {
        const auto minStrLen = std::min(lhv.length(), rhv.length());
        for (size_t i = 0; i < minStrLen; ++i)
            {
            const auto lhCh = tolower(lhv[i]);
            const auto rhCh = tolower(rhv[i]);
            if (lhCh == rhCh)
                {
                continue;
                }
            return (lhCh < rhCh);
            }
        return (lhv.length() < rhv.length());
        }

    // We can assume that we are only dealing with a-z, A-Z, 0-9, ., or _,
    // so use a branchless tolower.
    [[nodiscard]]
    constexpr static char tolower(const char chr) noexcept
        {
        return chr + (32 * (chr >= 'A' && chr <= 'Z'));
        }
    };

/// @brief A compiled expression.
/// @details Can also be an additional object that can be passed to
///     te_confun0-te_confun24 functions via a te_variable.
class te_expr
    {
  public:
    te_expr(const te_variable_flags type, const te_variant_type& value) noexcept
        : m_type(type), m_value(value)
        {
        }

    explicit te_expr(const te_variable_flags type) noexcept : m_type(type) {}

    /// @private
    te_expr() noexcept = default;

    /// @private
    te_expr(const te_expr&) = delete;
    /// @private
    te_expr& operator=(const te_expr&) = delete;

    /// @private
    virtual ~te_expr() = default;

    /// @brief The type that m_value represents.
    te_variable_flags m_type{ TE_DEFAULT };
    /// @brief The te_type constant, te_type pointer, or function to bind to.
    te_variant_type m_value{ static_cast<te_type>(0.0) };
    /// @brief Additional parameters.
    std::vector<te_expr*> m_parameters{ nullptr };
    };

/// @brief Custom variable or function that can be added to a te_parser.
class te_variable
    {
  public:
    /// @private
    using name_type = std::string;

    /// @private
    [[nodiscard]]
    bool operator<(const te_variable& that) const
        {
        return te_string_less{}(m_name, that.m_name);
        }

    /// @brief The name as it would appear in a formula.
    name_type m_name;
    /// @brief The te_type constant, te_type pointer, or function to bind the name to.
    te_variant_type m_value;
    /// @brief The type that m_value represents.
    te_variable_flags m_type{ TE_DEFAULT };
    /// If @c m_value is a function pointer of type `te_confun0`-`te_confun24`, then
    /// this is passed to that function when called. This is useful for passing
    /// an object which manages additional data to your functions.
    te_expr* m_context{ nullptr };
    };

/// @brief Math formula parser.
class te_parser
    {
  public:
    /// @private
    te_parser() = default;

    /// @private
    // cppcheck-suppress uninitMemberVar
    te_parser(const te_parser& that)
        : m_customFuncsAndVars(that.m_customFuncsAndVars),
          m_unknownSymbolResolve(that.m_unknownSymbolResolve),
          m_keepResolvedVariables(that.m_keepResolvedVariables),
          m_decimalSeparator(that.m_decimalSeparator), m_listSeparator(that.m_listSeparator),
          m_expression(that.m_expression)
        {
        try
            {
            if (!m_expression.empty())
                {
                [[maybe_unused]]
                const auto retVal = evaluate(m_expression);
                }
            }
        catch (const std::exception& expt)
            {
            m_parseSuccess = false;
            m_result = te_nan;
            m_lastErrorMessage = expt.what();
            }
        }

    /// @private
    te_parser& operator=(const te_parser& that)
        {
        m_customFuncsAndVars = that.m_customFuncsAndVars;
        m_unknownSymbolResolve = that.m_unknownSymbolResolve;
        m_keepResolvedVariables = that.m_keepResolvedVariables;
        m_decimalSeparator = that.m_decimalSeparator;
        m_listSeparator = that.m_listSeparator;
        m_expression = that.m_expression;

        // re-run the expression that was copied over
        reset_state();

        try
            {
            if (!m_expression.empty())
                {
                [[maybe_unused]]
                const auto retVal = evaluate(m_expression);
                }
            }
        catch (const std::exception& expt)
            {
            m_parseSuccess = false;
            m_result = te_nan;
            m_lastErrorMessage = expt.what();
            }

        return *this;
        }

    /// @private
    ~te_parser() { te_free(m_compiledExpression); }

    /// @brief NaN (not-a-number) constant to indicate an invalid value.
    constexpr static auto te_nan = std::numeric_limits<te_type>::quiet_NaN();
    /// @brief No position, which is what get_last_error_position() returns
    ///     when there was no parsing error.
    constexpr static int64_t npos = -1;
    /// @private
    // (2^48)-1
    constexpr static double MAX_BITOPS_VAL{ 281474976710655 }; // NOLINT

    /// @returns @c true if the parser's internal type can hold `uint32_t` without truncation.
    [[nodiscard]]
    constexpr static bool supports_32bit() noexcept
        {
        return std::numeric_limits<te_type>::digits >= std::numeric_limits<uint32_t>::digits;
        }

    /// @returns @c true if the parser's internal type can hold `uint64_t` without truncation.
    [[nodiscard]]
    constexpr static bool supports_64bit() noexcept
        {
        return std::numeric_limits<te_type>::digits >= std::numeric_limits<uint64_t>::digits;
        }

    /// @returns The bits available in the internal data type.\n
    ///     This will affect the largest integer size that can be used in bitwise operations.
    [[nodiscard]]
    constexpr static int get_max_integer_bitness() noexcept
        {
        return std::numeric_limits<te_type>::digits;
        }

    /// @returns The largest integer value that the parser can handle without truncation.
    [[nodiscard]]
    static te_type get_max_integer()
        {
#ifdef TE_FLOAT
        const te_type maxBit = std::ldexp(1, FLT_MANT_DIG - 1);
#elif defined(TE_LONG_DOUBLE)
        const te_type maxBit = std::ldexp(1, LDBL_MANT_DIG - 1);
#else
        const te_type maxBit = std::ldexp(1, DBL_MANT_DIG - 1);
#endif
        return maxBit + (maxBit - 1);
        }

    /** @brief Parses the input @c expression.
        @param expression The formula to compile.
        @returns Whether the expression compiled or not. (This can be checked
            by calling success() afterward as well.)
        @sa success().
        @note Returns NaN if division or modulus by zero occurs.
        @throws std::runtime_error Throws an exception in the case of arithmetic overflows
            (e.g., `1 << 64` would cause an overflow).*/
    bool compile(std::string_view expression);
    /** @brief Evaluates expression passed to compile() previously and returns its result.
        @returns The result, or NaN on error.
        @throws std::runtime_error Throws an exception in the case of arithmetic overflows
            (e.g., `1 << 64` would cause an overflow).*/
    [[nodiscard]]
    te_type evaluate();
    /** @brief Compiles and evaluates an expression and returns its result.
        @param expression The formula to compile and evaluate.
        @returns The result, or NaN on error.
        @note Returns NaN if division or modulus by zero occurs.
        @throws std::runtime_error Throws an exception in the case of arithmetic overflows
            (e.g., `1 << 64` would cause an overflow).*/
    [[nodiscard]]
    te_type evaluate(std::string_view expression);

    /// @returns The last call to evaluate()'s result (which will be NaN on error).
    [[nodiscard]]
    te_type get_result() const noexcept
        {
        return m_result;
        }

    /// @private
    [[nodiscard]]
    te_type get_result() const volatile noexcept
        {
        return m_result;
        }

    /// @returns Whether the last call to compile() was successful.
    /// @sa get_last_error_position().
    [[nodiscard]]
    bool success() const noexcept
        {
        return m_parseSuccess;
        }

    [[nodiscard]]
    bool success() const volatile noexcept
        {
        return m_parseSuccess;
        }

    /// @returns The zero-based index into the last parsed expression where the parse failed,
    ///     or te_parser::npos if no error occurred.
    /// @note Call success() to see if the last parse succeeded or not.
    [[nodiscard]]
    int64_t get_last_error_position() const noexcept
        {
        return m_errorPos;
        }

    /// @private
    [[nodiscard]]
    int64_t get_last_error_position() const volatile noexcept
        {
        return m_errorPos;
        }

    /// @returns Any error message from the last parse.
    [[nodiscard]]
    const std::string& get_last_error_message() const noexcept
        {
        return m_lastErrorMessage;
        }

    /// @brief Sets the list of custom variables and functions.
    /// @param vars The list of variables and functions.
    /// @note Valid variable and function names must begin with a letter from a-z (A-Z),
    ///     followed by additional English letters, numbers, periods, or underscores.
    /// @throws std::runtime_error Throws an exception if an illegal character is found
    ///     in any variable name.
    void set_variables_and_functions(std::set<te_variable> vars)
        {
        for (const auto& var : vars)
            {
            validate_name(var);
            }
        m_customFuncsAndVars = std::move(vars);
        }

    /// @brief Adds a custom variable or function.
    /// @param var The variable/function to add.
    /// @note Prefer using set_variables_and_functions() as it will be more optimal
    ///     (fewer sorts will need to be performed).
    /// @throws std::runtime_error Throws an exception if an illegal character is found
    ///     in the variable name.
    void add_variable_or_function(te_variable var)
        {
        validate_name(var);
        m_customFuncsAndVars.insert(std::move(var));
        }

    /// @brief Removes a custom variable or function.
    /// @param var The variable/function to remove (by name).
    void remove_variable_or_function(te_variable::name_type var)
        {
        const auto foundVar = m_customFuncsAndVars.find(
            te_variable{ std::move(var), static_cast<te_type>(0.0), TE_DEFAULT, nullptr });
        if (foundVar != m_customFuncsAndVars.cend())
            {
            m_customFuncsAndVars.erase(foundVar);
            }
        }

#ifndef TE_NO_BOOKKEEPING
    /// @brief Removes any custom variables and functions that weren't used in the last compilation.
    /// @details This can be useful if the parser is preloaded with a large number of
    ///     variables and functions that needs to be pruned after the first expression is parsed.
    /// @warning After calling this, any custom variables and functions that weren't found
    ///     in the previously parsed expression will no longer be available.
    void remove_unused_variables_and_functions()
        {
        for (auto funcIter = m_customFuncsAndVars.cbegin(); funcIter != m_customFuncsAndVars.cend();
             /* in loop*/)
            {
            funcIter = (is_function_used(funcIter->m_name) || is_variable_used(funcIter->m_name)) ?
                           ++funcIter :
                           m_customFuncsAndVars.erase(funcIter);
            }
        }
#endif

    /** @brief Sets a custom function to resolve unknown symbols in an expression.
        @param usr The function to use to resolve unknown symbols.
        @param keepResolvedVariables @c true to cache any resolved variables into the parser.
            This means that they will not need to be resolved again on subsequent
            calls to evaluate().\n
            Pass @c false to this if you wish to re-resolve any previously resolved
            variables on later evaluations.
            This can be useful for when a resolved variable's value is
            volatile and needs to be re-resolved on every use.*/
    void set_unknown_symbol_resolver(te_usr_variant_type usr,
                                     const bool keepResolvedVariables = true)
        {
        m_unknownSymbolResolve = std::move(usr);
        m_keepResolvedVariables = keepResolvedVariables;
        }

    /// @private
    [[nodiscard]]
    const std::set<te_variable>& get_variables_and_functions() const noexcept
        {
        return m_customFuncsAndVars;
        }

    /// @returns The list of custom variables and functions.
    [[nodiscard]]
    std::set<te_variable>& get_variables_and_functions() noexcept
        {
        return m_customFuncsAndVars;
        }

    /// @returns The decimal separator used for numbers.
    [[nodiscard]]
    char get_decimal_separator() const noexcept
        {
        return m_decimalSeparator;
        }

    /// @private
    [[nodiscard]]
    char get_decimal_separator() const volatile noexcept
        {
        return m_decimalSeparator;
        }

    /// @brief Sets the decimal separator used for numbers.
    /// @param sep The decimal separator.
    /// @throws std::runtime_error Throws an exception if an illegal character is used.
    void set_decimal_separator(const char sep)
        {
        if (sep != ',' && sep != '.')
            {
            throw std::runtime_error("Decimal separator must be either a '.' or ','.");
            }
        m_decimalSeparator = sep;
        }

    /// @private
    void set_decimal_separator(const char sep) volatile
        {
        if (sep != ',' && sep != '.')
            {
            throw std::runtime_error("Decimal separator must be either a '.' or ','.");
            }
        m_decimalSeparator = sep;
        }

    /// @brief Sets a constant variable's value.
    /// @param name The name of the (constant) variable.
    /// @param value The new value to set the constant to.
    /// @note If the constant variable hasn't been added yet (via set_variables_and_functions()),
    ///     then this will add it.\n
    ///     If a variable with the provided name is found but is not a constant,
    ///     then this will be ignored.
    void set_constant(const std::string_view name, const te_type value)
        {
        auto cvar = find_variable_or_function(name);
        if (cvar == get_variables_and_functions().end())
            {
            add_variable_or_function({ te_variable::name_type{ name }, value });
            }
        else if (is_constant(cvar->m_value))
            {
            auto nh = get_variables_and_functions().extract(cvar);
            nh.value().m_value = value;
            get_variables_and_functions().insert(std::move(nh));
            // if previously compiled, then re-compile since this
            // constant would have been optimized
            if (!m_expression.empty())
                {
                compile(m_expression);
                }
            }
        }

    /// @brief Retrieves a constant variable's value.
    /// @param name The name of the (constant) variable.
    /// @returns The value of the constant variable if found, NaN otherwise.
    [[nodiscard]]
    te_type get_constant(const std::string_view name) const
        {
        auto cvar = find_variable_or_function(name);
        if (cvar == get_variables_and_functions().cend() || !is_constant(cvar->m_value))
            {
            return te_nan;
            }
        if (const auto* const val = std::get_if<te_type>(&cvar->m_value); val != nullptr)
            {
            return *val;
            }

        return te_nan;
        }

    /// @returns The separator used between function arguments.
    [[nodiscard]]
    char get_list_separator() const noexcept
        {
        return m_listSeparator;
        }

    /// @private
    [[nodiscard]]
    char get_list_separator() const volatile noexcept
        {
        return m_listSeparator;
        }

    /// @brief Sets the separator used between function arguments.
    /// @param sep The list separator.
    /// @throws std::runtime_error Throws an exception if an illegal character is used.
    void set_list_separator(const char sep)
        {
        if (sep != ',' && sep != ';')
            {
            throw std::runtime_error("List separator must be either a ',' or ';'.");
            }
        m_listSeparator = sep;
        }

    /// @private
    void set_list_separator(const char sep) volatile
        {
        if (sep != ',' && sep != ';')
            {
            throw std::runtime_error("List separator must be either a ',' or ';'.");
            }
        m_listSeparator = sep;
        }
#ifndef TE_NO_BOOKKEEPING
    /// @returns @c true if @c name is a function that had been used in the last parsed formula.
    /// @param name The name of the function.
    /// @sa compile() and evaluate().
    [[nodiscard]]
    bool is_function_used(const std::string_view name) const
        {
        return m_usedFunctions.contains(te_variable::name_type{ name });
        }

    /// @returns @c true if @c name is a variable that had been used in the last parsed formula.
    /// @param name The name of the variable.
    /// @sa compile() and evaluate().
    [[nodiscard]]
    bool is_variable_used(const std::string_view name) const
        {
        return m_usedVars.contains(te_variable::name_type{ name });
        }
#endif
    /// @returns A report of all available functions and variables.
    [[nodiscard]]
    std::string list_available_functions_and_variables();

    /// @returns The last formula passed to the parser.
    /// @note Comments will be stripped from the original expression.
    [[nodiscard]]
    const std::string& get_expression() const noexcept
        {
        return m_expression;
        }

    /// @brief Helper function to convert a number into boolean.
    /// @details This takes NaN into account, returning @c for NaN.
    /// @param val The value to examine.
    /// @returns @c true if the value is non-zero and also valid
    ///     (not NaN or infinite).
    /// @private
    [[nodiscard]]
    static bool number_to_bool(te_type val)
        {
        return std::isfinite(val) ? static_cast<bool>(val) : false;
        }

    /// @returns Information about how the parser is configured, its capabilities, etc.
    [[nodiscard]]
    static std::string info();

  private:
    /// Reset everything from previous call.
    void reset_state()
        {
        m_errorPos = te_parser::npos;
        m_lastErrorMessage.clear();
        m_result = te_nan;
        m_parseSuccess = false;
        te_free(m_compiledExpression);
        m_compiledExpression = nullptr;
        m_currentVar = m_functions.cend();
        m_varFound = false;
#ifndef TE_NO_BOOKKEEPING
        m_usedFunctions.clear();
        m_usedVars.clear();
#endif
        m_resolvedVariables.clear();
        }

    /// @brief Resets any resolved variables from USR if not being cached.
    void reset_usr_resolved_if_necessary()
        {
        if (!m_keepResolvedVariables && !m_resolvedVariables.empty())
            {
            for (const auto& resolvedVar : m_resolvedVariables)
                {
                remove_variable_or_function(resolvedVar);
                }
            m_resolvedVariables.clear();
            }
        }

    /// @brief Gets the compiled expression, which will be the optimized version
    ///     of the original expression.
    /// @returns The compiled expression.
    [[nodiscard]]
    const te_expr* get_compiled_expression() const noexcept
        {
        return m_compiledExpression;
        }

    /// @private
    [[nodiscard]]
    const te_expr* get_compiled_expression() const volatile noexcept
        {
        return m_compiledExpression;
        }

    /// @brief Validates that a variable only contains legal characters
    ///     (and has a valid length).
    /// @param var The variable to validate.
    /// @throws std::runtime_error Throws an exception if an illegal character is found.
    static void validate_name(const te_variable& var)
        {
        if (var.m_name.empty())
            {
            throw std::runtime_error("Variable name is empty.");
            }
        if (!is_letter(var.m_name[0]) && var.m_name[0] != '_')
            {
            throw std::runtime_error(
                std::string("Variable name must begin with a letter from a-z or _: ") + var.m_name);
            }
        const auto varCharPos =
            std::find_if(var.m_name.cbegin(), var.m_name.cend(),
                         [](const auto chr) noexcept { return !is_name_char_valid(chr); });
        if (varCharPos != var.m_name.cend())
            {
            throw std::runtime_error(std::string("Invalid character in variable name: ") +
                                     var.m_name);
            }
        }

    /// @returns @c true if character is valid for a function or variable name.
    /// @param chr The character to review.
    [[nodiscard]]
    constexpr static bool is_name_char_valid(const char chr) noexcept
        {
        return (is_letter(chr) || (chr >= '0' && chr <= '9') || (chr == '_') || (chr == '.'));
        }

    /// @returns An iterator to the custom variable or function with the given @c name,
    ///     or end of get_variables_and_functions() if not found.
    /// @param name The name of the function or variable to search for.
    [[nodiscard]]
    std::set<te_variable>::iterator find_variable_or_function(const std::string_view name)
        {
        if (name.empty())
            {
            return m_customFuncsAndVars.end();
            }

        return m_customFuncsAndVars.find(te_variable{
            te_variable::name_type{ name }, static_cast<te_type>(0.0), TE_DEFAULT, nullptr });
        }

    /// @returns An iterator to the custom variable or function with the given @c name,
    ///     or end of get_variables_and_functions() if not found.
    /// @param name The name of the function or variable to search for.
    [[nodiscard]]
    std::set<te_variable>::const_iterator
    find_variable_or_function(const std::string_view name) const
        {
        if (name.empty())
            {
            return m_customFuncsAndVars.cend();
            }

        return m_customFuncsAndVars.find(te_variable{
            te_variable::name_type{ name }, static_cast<te_type>(0.0), TE_DEFAULT, nullptr });
        }

    [[nodiscard]]
    constexpr static auto is_pure(const te_variable_flags type)
        {
        return (((type)&TE_PURE) != 0);
        }

    [[nodiscard]]
    constexpr static auto is_variadic(const te_variable_flags type)
        {
        return (((type)&TE_VARIADIC) != 0);
        }

    /// @returns Number of parameters that a function/variable takes.
    [[nodiscard]]
    static auto get_arity(const te_variant_type& var)
        {
        return std::visit(
            [](const auto& var0) -> size_t
            {
                using T = std::decay_t<decltype(var0)>;
                if constexpr (te_is_constant_v<T> || te_is_variable_v<T>)
                    {
                    return 0;
                    }
                else if constexpr (te_is_closure_v<T>)
                    {
                    return te_function_arity<T> - 1; // minus the first ctx variable.
                    }
                else
                    { // te_is_function_v
                    return te_function_arity<T>;
                    }
            },
            var);
        }

    [[nodiscard]]
    constexpr static bool is_constant(const te_variant_type& var) noexcept
        {
        return var.index() == 0;
        }

    [[nodiscard]]
    constexpr static te_type get_constant(const te_variant_type& var)
        {
        assert(std::holds_alternative<te_type>(var));
        return std::get<0>(var);
        }

    [[nodiscard]]
    constexpr static bool is_variable(const te_variant_type& var) noexcept
        {
        return var.index() == 1;
        }

    [[nodiscard]]
    constexpr static const te_type* get_variable(const te_variant_type& var)
        {
        assert(std::holds_alternative<const te_type*>(var));
        return std::get<1>(var);
        }

    [[nodiscard]]
    constexpr static bool is_function(const te_variant_type& var)
        {
        return std::visit(
            [](const auto& var0) -> bool
            {
                using T = std::decay_t<decltype(var0)>;
                return te_is_function_v<T>;
            },
            var);
        }

#define TE_DEF_FUNCTION(n)                                                                         \
    [[nodiscard]]                                                                                  \
    constexpr static bool is_function##n(const te_variant_type& var) noexcept                      \
        {                                                                                          \
        return std::holds_alternative<te_fun##n>(var);                                             \
        }                                                                                          \
    [[nodiscard]]                                                                                  \
    constexpr static te_fun##n get_function##n(const te_variant_type& var)                         \
        {                                                                                          \
        assert(std::holds_alternative<te_fun##n>(var));                                            \
        return std::get<te_fun##n>(var);                                                           \
        }
    TE_DEF_FUNCTION(0);
    TE_DEF_FUNCTION(1);
    TE_DEF_FUNCTION(2);
#undef TE_DEF_FUNCTION

    [[nodiscard]]
    constexpr static bool is_closure(const te_variant_type& var)
        {
        return std::visit(
            [](const auto& var0) -> bool
            {
                using T = std::decay_t<decltype(var0)>;
                return te_is_closure<T>::value;
            },
            var);
        }

#define TE_DEF_CLOSURE(n)                                                                          \
    [[nodiscard]]                                                                                  \
    constexpr static bool is_closure##n(const te_variant_type& var) noexcept                       \
        {                                                                                          \
        return std::holds_alternative<te_confun##n>(var);                                          \
        }                                                                                          \
    [[nodiscard]]                                                                                  \
    constexpr static te_confun##n get_closure##n(const te_variant_type& var)                       \
        {                                                                                          \
        assert(std::holds_alternative<te_confun##n>(var));                                         \
        return std::get<te_confun##n>(var);                                                        \
        }
    TE_DEF_CLOSURE(0);
    TE_DEF_CLOSURE(1);
    TE_DEF_CLOSURE(2);
#undef TE_DEF_CLOSURE

    struct state
        {
        enum class token_type
            {
            TOK_NULL,
            TOK_ERROR,
            TOK_END,
            TOK_SEP,
            TOK_OPEN,
            TOK_CLOSE,
            TOK_NUMBER,
            TOK_VARIABLE,
            TOK_FUNCTION,
            TOK_INFIX
            };

        state(const char* expression, te_variable_flags varType, std::set<te_variable>& vars)
            : m_start(expression), m_next(expression), m_varType(varType), m_lookup(vars)
            {
            }

        const char* m_start{ nullptr };
        const char* m_next{ nullptr };
        token_type m_type{ token_type::TOK_NULL };
        te_variable_flags m_varType{ TE_DEFAULT };
        te_variant_type m_value;
        te_expr* context{ nullptr };

        std::set<te_variable>& m_lookup;
        };

    [[nodiscard]]
    static te_expr* new_expr(const te_variable_flags type, te_variant_type value,
                             const std::initializer_list<te_expr*>& parameters)
        {
        auto* ret = new te_expr{ type, value };
        ret->m_parameters.resize(
            std::max<size_t>(std::max<size_t>(parameters.size(), get_arity(ret->m_value)) +
                                 (is_closure(ret->m_value) ? 1 : 0),
                             0));
        if (parameters.size() != 0U)
            {
            std::ranges::copy(parameters, ret->m_parameters.begin());
            }
        return ret;
        }

    [[nodiscard]]
    static te_expr* new_expr(const te_variable_flags type, te_variant_type value)
        {
        auto* ret = new te_expr{ type, value };
        ret->m_parameters.resize(static_cast<size_t>(get_arity(ret->m_value)) +
                                 (is_closure(ret->m_value) ? 1 : 0));
        return ret;
        }

    [[nodiscard]]
    constexpr static bool is_letter(const char chr) noexcept
        {
        return (chr >= 'a' && chr <= 'z') || (chr >= 'A' && chr <= 'Z');
        }

    /** @brief Parses the input expression and binds variables.
        @param expression The formula to parse.
        @param variables The collection of custom functions and
            variables to add to the parser.
        @returns null on error.*/
    [[nodiscard]]
    te_expr* te_compile(std::string_view expression, std::set<te_variable>& variables);
    /* Evaluates the expression. */
    [[nodiscard]]
    static te_type te_eval(const te_expr* texp);

    /* Frees the expression. */
    /* This is safe to call on null pointers. */
    static void te_free(te_expr* texp)
        {
        if (texp == nullptr)
            {
            return;
            }
        te_free_parameters(texp);
        delete texp;
        }

    static void te_free_parameters(te_expr* texp);
    static void optimize(te_expr* texp);

    [[nodiscard]]
    static auto find_builtin(const std::string_view name)
        {
        return m_functions.find(te_variable{ te_variable::name_type{ name },
                                             static_cast<te_type>(0.0), TE_DEFAULT, nullptr });
        }

    [[nodiscard]]
    static auto find_lookup(state* ste, const std::string_view name)
        {
        return ste->m_lookup.find(te_variable{ te_variable::name_type{ name },
                                               static_cast<te_type>(0.0), TE_DEFAULT, nullptr });
        }

    void next_token(state* theState);
    [[nodiscard]]
    te_expr* base(state* theState);
    [[nodiscard]]
    te_expr* power(state* theState);
    [[nodiscard]]
    te_expr* factor(state* theState);
    [[nodiscard]]
    te_expr* term(state* theState);
    [[nodiscard]]
    te_expr* expr_level1(state* theState);
    [[nodiscard]]
    te_expr* expr_level2(state* theState);
    [[nodiscard]]
    te_expr* expr_level3(state* theState);
    [[nodiscard]]
    te_expr* expr_level4(state* theState);
    [[nodiscard]]
    te_expr* expr_level5(state* theState);
    [[nodiscard]]
    te_expr* expr_level6(state* theState);
    [[nodiscard]]
    te_expr* expr_level7(state* theState);
    [[nodiscard]]
    te_expr* expr_level8(state* theState);
    [[nodiscard]]
    te_expr* expr_level9(state* theState);
    [[nodiscard]]
    te_expr* list(state* theState);

    // built-in functions
    static const std::set<te_variable> m_functions;

    // customizable settings
    std::set<te_variable> m_customFuncsAndVars;

    te_usr_variant_type m_unknownSymbolResolve{ te_usr_noop{} };

    bool m_keepResolvedVariables{ true };

    char m_decimalSeparator{ '.' };
    char m_listSeparator{ ',' };

    // state information
    std::string m_expression;
    te_expr* m_compiledExpression{ nullptr };

    bool m_parseSuccess{ false };
    int64_t m_errorPos{ 0 };
    std::string m_lastErrorMessage;
    te_type m_result{ te_nan };

    std::set<te_variable::name_type> m_resolvedVariables;

    std::set<te_variable>::const_iterator m_currentVar;
    bool m_varFound{ false };
#ifndef TE_NO_BOOKKEEPING
    std::set<te_variable::name_type, te_string_less> m_usedFunctions;
    std::set<te_variable::name_type, te_string_less> m_usedVars;
#endif
    };

#endif // __TINYEXPR_PLUS_PLUS_H__
