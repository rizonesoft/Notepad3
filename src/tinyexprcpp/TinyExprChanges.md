The following are changes from the original TinyExpr C library:

- Compiles as C++20 code.
- `te_*` functions are now wrapped in a `te_parser` class.
- `te_interp()`, `te_compile()`, and `te_eval()` have been replaced with `te_parser::compile()`, `te_parser::evaluate()`, and `te_parser::set_variables_and_functions()`.
  `set_variables_and_functions()` sets your list of custom functions and variables. `compile()` compiles and optimizes an expression.
  Finally, `evaluate()` will use the already compiled expression and return its result.
  `evaluate()` also has an overload that compiles and evaluates an expression in one call.
- Variable/function types (e.g., `TE_FUNCTION0`) have been removed; types are now deduced by the compiler.
  The available flags for variables and functions are now just combinations of `TE_DEFAULT`, `TE_PURE`, and `TE_VARIADIC`.
- Formula parsing is now case insensitive.
- Added support for variadic functions (can accept 1-24 arguments); enabled through the `TE_VARIADIC` flag.
  (Refer to the `AVERAGE()` function in `tinyexpr.cpp` for an example.)
- Added support for parsing formulas in non-US format (e.g., `pow(2,2; 2)` instead of `pow(2.2, 2)`). Useful for when the program's locale is non-English.
  (Refer to [Example 4](Examples.md) for a demonstration.)
- `te_expr` is now a derivable base class. This means that you can derive from `te_expr`, add new fields to that derived class (e.g., arrays, strings, even other classes)
  and then use a custom class as an argument to the various function types that accept a `te_expr*` parameter. The function that you connect can then `dynamic_cast<>`
  this argument and use its custom fields, thus greatly enhancing the functionality for these types of functions.
  (See below for example.)
- Added exception support, where exceptions are thrown for situations like providing invalid separators. Calls to `compile` and `evaluate` should be wrapped in `try`...`catch` blocks.
- Memory management is handled by the `te_parser` class (you no longer need to call `te_free`). Also, replaced `malloc/free` with `new/delete`.
- Stricter type safety; uses `std::variant` (instead of unions) that support `double`, `const double*`,
  and 16 specific function signatures (that will work with lambdas or function pointers).
  Also uses `std::initializer` lists (instead of various pointer operations).
- Separate enums are now used between `te_expr` and `state`'s types and are more strongly typed.
- Added support for C and C++ style comments (`//` and `/**/`).
- `compile()` and `evaluate()` now accept `std::string_view`s, meaning that these functions can accept either `char*` or `std::string` arguments.
- Added support for `volatile`.
- Custom functions and variables can now contain periods in their names.
- Custom functions and variables can now start with an underscore.
- Custom functions and variables can now be removed.
- Added support for custom handlers to resolve unknown variables.
- Added new built-in functions:
  - `and`: returns true (i.e., non-zero) if all conditions are true (accepts 1-24 arguments).
  - `average`: returns the mean for a range of values (accepts 1-24 arguments).
  - `bitand`: bitwise AND.
  - `bitlrotate`: bitwise left rotate. Versions of this are available for 8-, 16-, 32-, and 64-bit integers (if supported by the platform).
  - `bitlshift`: left shift.
     Negative shift amount arguments (similar to *Excel*) are supported.
  - `bitnot`: bitwise NOT. Versions of this are available for 8-, 16-, 32-, and 64-bit integers (if supported by the platform).
  - `bitor`: bitwise OR.
  - `bitrrotate`: bitwise right rotate. Versions of this are available for 8-, 16-, 32-, and 64-bit integers (if supported by the platform).
  - `bitrshift`: right shift.
     Negative shift amount arguments (similar to *Excel*) are supported.
  - `bitxor`: bitwise XOR.
  - `cot`: returns the cotangent of an angle.
  - `combin`: alias for `ncr()`, like the *Excel* function.
  - `clamp`: constrains a value to a range.
  - `cumprinc`: returns the cumulative principal paid on a loan between two periods.
  - `cumipmt`: returns the cumulative interest paid on a loan between two periods.
  - `db`: returns the depreciation of an asset for a specified period using the fixed-declining balance method.
  - `effect`: returns the effective annual interest rate, provided the nominal annual interest rate and the number of compounding periods per year.
  - `even`: returns a value rounded up to the nearest even integer.
  - `fact`: alias for `fac()`, like the *Excel* function.
  - `false`: returns `false` (i.e., `0`) in a boolean expression.
  - `fv`: returns the future value of an investment.
  - `ipmt`: returns the interest portion of a payment for a specified period of an investment.
  - `iserr`: returns true if an expression evaluates to NaN.
  - `iserror`: alias for `iserr`.
  - `iseven`: returns true if a number is even, false if odd.
  - `isna`: alias for `iserr`.
  - `isnan`: alias for `iserr`.
  - `isodd`: returns true if a number is odd, false if even.
  - `if`: if a value is true (i.e., non-zero), then returns the second argument; otherwise, returns the third argument.
  - `ifs`: checks up to three conditions, returning the value corresponding to the first met condition.
  - `max`: returns the maximum of a range of values (accepts 1-24 arguments).
  - `maxint`: returns the largest integer value that the parser can store.
  - `min`: returns the minimum of a range of values (accepts 1-24 arguments).
  - `mod`: returns remainder from a division.
  - `na`: returns `NaN` (i.e., Not-a-Number) in a boolean expression.
  - `nan`: alias for `na`.
  - `nominal`: returns the nominal annual interest rate, provided the effective rate and the number of compounding periods per year.
  - `nper`: returns the number of periods for an investment.
  - `odd`: returns a value rounded up to the nearest odd integer.
  - `or`: returns true (i.e., non-zero) if any condition is true (accepts 1-24 arguments).
  - `not`: returns logical negation of value.
  - `permut`: alias for `npr()`, like the *Excel* function.
  - `power`: alias for `pow()`, like the *Excel* function.
  - `pmt`: returns the periodic payment for an investment or loan based on a constant interest rate, a fixed number of periods, and a present value (like the *Excel* function).
  - `ppmt`: returns the principal portion of a payment for a specified period of an investment.
  - `pv`: returns the present value of an investment, like the *Excel* function.
  - `rand`: returns random number between `0` and `1`.
     Note that this implementation uses the Mersenne Twister (`mt19937`) to generate random numbers.
  - `round`: returns a number, rounded to a given decimal point.
     (Decimal point is optional and defaults to `0`.)
     Negative number-of-digits arguments (similar to *Excel*) is supported.
  - `sign`: returns the sign of a number: `1` if positive, `-1` if negative, `0` if zero.
  - `sum`: returns the sum of a list of values (accepts 1-24 arguments).
  - `sqr`: returns a number squared.
  - `tgamma`: returns gamma function of a specified value.
  - `trunc`: returns the integer part of a number.
  - `true`: returns `true` (i.e., `1`) in a boolean expression.
- Added new operators:
  - `&`    logical AND.
  - `|`    logical OR.
  - `&&`   logical AND.
  - `||`   logical OR.
  - `=`    equal to.
  - `==`   equal to.
  - `<>`   not equal to.
  - `!=`   not equal to.
  - `<`    less than.
  - `<=`   less than or equal to.
  - `>`    greater than.
  - `>=`   greater than or equal to.
  - `<<`   left shift operator.
  - `>>`   right shift operator.
  - `<<<`  left (`uint64_t`) rotation operator.
  - `>>>`  right (`uint64_t`) rotation operator.
  - `**`   exponentiation (alias for `^`).
  - `~`    bitwise NOT.
- `round` now supports negative number of digit arguments, similar to *Excel*.
  For example, `ROUND(-50.55,-2)` will yield `-100`.
- Custom variables and functions are now stored in a `std::set`
  (which can be easily accessed and updated via the new `get_variables_and_functions()/set_variables_and_functions()` functions).
- Added `is_function_used()` and `is_variable_used()` functions to see if a specific function or variable was used in the last parsed formula.
- Added `set_constant()` function to find and update the value of a constant (custom) variable by name.
  (In this context, a constant is a variable mapped to a double value in the parser, rather than mapped to a runtime variable.)
- Added `get_constant()` function to return the value of a constant (custom) variable by name.
- Added `TE_FLOAT` preprocessor flag to use `float` instead of `double` for the parser's data type.
- Added `TE_LONG_DOUBLE` preprocessor flag to use `long double` instead of `double` for the parser's data type.
- Binary search (i.e., `std::set`) is now used to look up custom variables and functions (small optimization).
- You no longer need to specify the number of arguments for custom functions; it will deduce that for you.
- The position of an error when evaluating an expression is now managed by the `te_parser` class and accessible via `get_last_error_position()`.
- Some parsing errors can provide error messages available via `get_last_error_message()`.
- The position of aforementioned error is now 0-indexed (not 1-indexed); `te_parser::npos` indicates that there was no error.
- Added `success()` function to indicate if the last parse succeeded or not.
- Added `get_result()` function to get result from the last call to `evaluate` or `compile`.
- Now uses `std::numeric_limits` for math constants (instead of macro constants).
- Replaced C-style casts with `static_cast<>`.
- Replaced all macros with `constexpr`s and lambdas.
- Replaced custom binary search used for built-in function searching; `std::set` is used now.
- Now uses `nullptr` (instead of `0`).
- All data fields are now initialized.
- Added [Doxygen](https://github.com/doxygen/doxygen) comments.
- Removed `te_print()` debug function.
- Added `list_available_functions_and_variables()` function to display all available built-in and custom functions and variables.
- Added `get_expression()` function to get the last formula used.
- Added `[[nodiscard]]` attributes to improve compile-time warnings.
- Added `constexpr` and `noexcept` for C++ optimization.
