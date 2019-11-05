#include "tinyexpr.h"
#include <stdio.h>


/* An example of calling a C function. */
double my_sum(double a, double b) {
    printf("Called C function with %f and %f.\n", a, b);
    return a + b;
}


int main(int argc, char *argv[])
{
    te_variable vars[] = {
        {"mysum", my_sum, TE_FUNCTION2}
    };

    const char *expression = "mysum(5, 6)";
    printf("Evaluating:\n\t%s\n", expression);

    int err;
    te_expr *n = te_compile(expression, vars, 1, &err);

    if (n) {
        const double r = te_eval(n);
        printf("Result:\n\t%f\n", r);
        te_free(n);
    } else {
        /* Show the user where the error is at. */
        printf("\t%*s^\nError near here", err-1, "");
    }


    return 0;
}
