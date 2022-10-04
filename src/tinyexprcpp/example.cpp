#include "tinyexpr.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    te_parser tep;
    const char *c = "sqrt(5^2+7^2+11^2+(8-2)^2)";
    double r = tep.evaluate(c);
    printf("The expression:\n\t%s\nevaluates to:\n\t%f\n", c, r);
    return 0;
}
