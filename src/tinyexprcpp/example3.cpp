#include "tinyexpr.h"
#include <stdio.h>


/* An example of calling a C function. */
double my_sum(double a, double b) {
    printf("Called C function with %f and %f.\n", a, b);
    return a + b;
}


int main(int argc, char *argv[])
{
    const char *expression = "mysum(5, 6)";
    printf("Evaluating:\n\t%s\n", expression);

    te_parser tep;
    tep.set_vars({{"mysum", my_sum}});

    if (tep.compile(expression)) {
        const double r = tep.evaluate();
        printf("Result:\n\t%f\n", r);
    } else {
        /* Show the user where the error is at. */
        printf("\t%*s^\nError near here", tep.get_last_error_position(), "");
    }


    return 0;
}
