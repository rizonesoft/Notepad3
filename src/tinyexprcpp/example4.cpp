#include "tinyexpr.h"
#include <cstdio>
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
    printf("Evaluating:\n\t%s\n", expression);

    te_parser tep;
    tep.set_decimal_separator(',');
    tep.set_list_separator(';');

    if (tep.compile(expression)) {
        const double r = tep.evaluate(); printf("Result:\n\t%f\n", r);
    } else {
        /* Show the user where the error is at. */
        printf("\t%*s^\nError near here", tep.get_last_error_position(), "");
    }

    return 0;
}
