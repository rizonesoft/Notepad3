# Examples

The following are examples demonstrating how to use TinyExpr++.

## Example 1

```cpp
#include "tinyexpr.h"
#include <iostream>

int main(int argc, char *argv[])
    {
    te_parser tep;
    const char *c = "sqrt(5^2+7^2+11^2+(8-2)^2)";
    double r = tep.evaluate(c);
    std::cout << "The expression:\n\t" <<
        c << "\nevaluates to:\n\t" << r << "\n";
    return EXIT_SUCCESS;
    }
```

## Example 2: Binding Custom Variables

```cpp
#include "tinyexpr.h"
#include <iostream>
#include <iomanip>

int main(int argc, char* argv[])
    {
    if (argc < 2)
        {
        std::cout << "Usage: example \"expression\"\n";
        return EXIT_SUCCESS;
        }

    const char* expression = argv[1];
    std::cout << "Evaluating:\n\t" << expression << "\n";

    /* This shows an example where the variables
       x and y are bound at eval-time. */
    double x{ 0 }, y{ 0 };
    // Store variable names and pointers.
    te_parser tep;
    tep.set_variables_and_functions({ {"x", &x}, {"y", &y} });

    /* This will compile the expression and check for errors. */
    if (tep.compile(expression))
        {
        /* The variables can be changed here, and eval can be called as many
         * times as you like. This is fairly efficient because the parsing has
         * already been done. */
        x = 3; y = 4;
        const double r = tep.evaluate();
        std::cout << "Result:\n\t" << r << "\n";
        }
    else
        {
        /* Show the user where the error is at. */
        std::cout << "\t " << std::setfill(' ') <<
            std::setw(tep.get_last_error_position()) << '^' <<
            "\tError near here\n";
        }

    return EXIT_SUCCESS;
    }
```

## Example 3: Calling a Free Function

```cpp
#include "tinyexpr.h"
#include <iostream>
#include <iomanip>

/* An example of calling a free function. */
double my_sum(double a, double b)
    {
    std::cout << "Called C function with " <<
        a << " and " << b << ".\n";
    return a + b;
    }

int main(int argc, char *argv[])
    {
    const char *expression = "mysum(5, 6)";
    std::cout << "Evaluating:\n\t" << expression << "\n";

    te_parser tep;
    tep.set_variables_and_functions({{"mysum", my_sum}});

    if (tep.compile(expression))
        {
        const double r = tep.evaluate();
        std::cout << "Result:\n\t" << r << "\n";
        }
    else
        {
        /* Show the user where the error is at. */
        std::cout << "\t " << std::setfill(' ') <<
            std::setw(tep.get_last_error_position()) << '^' <<
            "\tError near here\n";
        }

    return EXIT_SUCCESS;
    }
```

## Example 4: Non-US Formatted Formulas

```cpp
#include "tinyexpr.h"
#include <iostream>
#include <iomanip>
#include <locale>
#include <clocale>

int main(int argc, char *argv[])
    {
    /* Set locale to German.
       This string is platform dependent. The following works on Windows,
       consult your platform's documentation for more details.*/
    setlocale(LC_ALL, "de-DE");
    std::locale::global(std::locale("de-DE"));

    /* After setting your locale to German, functions like strtod() will fail
       with values like "3.14" because it expects "3,14" instead.
       To fix this, we will tell the parser to use "," as the decimal separator
       and ";" as list argument separator.*/

    const char *expression = "pow(2,2; 2)"; // instead of "pow(2.2, 2)"
    std::cout << "Evaluating:\n\t" << expression << "\n";

    te_parser tep;
    tep.set_decimal_separator(',');
    tep.set_list_separator(';');

    if (tep.compile(expression))
        {
        const double r = tep.evaluate();
        std::cout << "Result:\n\t" << r << "\n";
        }
    else
        {
        /* Show the user where the error is at. */
        std::cout << "\t " << std::setfill(' ') <<
            std::setw(tep.get_last_error_position()) << '^' <<
            "\tError near here\n";
        }

    return EXIT_SUCCESS;
    }
```

## Example 5: Binding to Custom Classes

A class derived from `te_expr` can be bound to custom functions. This enables you to
have full access to an object (via these functions) when parsing an expression.

The following demonstrates creating a `te_expr`-derived class which contains an array of values:

```cpp
class te_expr_array : public te_expr
    {
public:
    explicit te_expr_array(const variable_flags type) noexcept :
        te_expr(type) {}
    std::array<double, 5> m_data = { 5, 6, 7, 8, 9 };
    };
```

Next, create two functions that can accept this object and perform
actions on it. (Note that proper error handling is not shown for brevity.):

```cpp
// Returns the value of a cell from the object's data.
double cell(const te_expr* context, double a)
    {
    auto* c = dynamic_cast<const te_expr_array*>(context);
    return static_cast<double>(c->m_data[static_cast<size_t>(a)]);
    }

// Returns the max value of the object's data.
double cell_max(const te_expr* context)
    {
    auto* c = dynamic_cast<const te_expr_array*>(context);
    return static_cast<double>(
        *std::max_element(c->m_data.cbegin(), c->m_data.cend()));
    }
```

Finally, create an instance of the class and connect the custom functions to it,
while also adding them to the parser:

```cpp
te_expr_array teArray{ TE_DEFAULT };

te_parser tep;
tep.set_variables_and_functions(
    {
        {"cell", cell, TE_DEFAULT, &teArray},
        {"cellmax", cell_max, TE_DEFAULT, &teArray}
    });

// change the object's data and evaluate their summation
// (will be 30)
teArray.m_data = { 6, 7, 8, 5, 4 };
auto result = tep.evaluate("SUM(CELL 0, CELL 1, CELL 2, CELL 3, CELL 4)");

// call the other function, getting the object's max value
// (will be 8)
result = tep.evaluate("CellMax()");
```
