/*
 * TINYEXPR - Tiny recursive descent parser and evaluation engine in C
 *
 * Copyright (c) 2015, 2016 Lewis Van Winkle
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

#include <stdio.h>
#include <time.h>
#include <math.h>
#include "tinyexpr.h"



#define loops 10000



typedef double (*function1)(double);

void bench(const char *expr, function1 func) {
    int i, j;
    volatile double d;
    double tmp;
    clock_t start;

    te_variable lk = {"a", &tmp};

    printf("Expression: %s\n", expr);

    printf("native ");
    start = clock();
    d = 0;
    for (j = 0; j < loops; ++j)
        for (i = 0; i < loops; ++i) {
            tmp = i;
            d += func(tmp);
        }
    const int nelapsed = (clock() - start) * 1000 / CLOCKS_PER_SEC;

    /*Million floats per second input.*/
    printf(" %.5g", d);
    if (nelapsed)
        printf("\t%5dms\t%5dmfps\n", nelapsed, loops * loops / nelapsed / 1000);
    else
        printf("\tinf\n");




    printf("interp ");
    te_expr *n = te_compile(expr, &lk, 1, 0);
    start = clock();
    d = 0;
    for (j = 0; j < loops; ++j)
        for (i = 0; i < loops; ++i) {
            tmp = i;
            d += te_eval(n);
        }
    const int eelapsed = (clock() - start) * 1000 / CLOCKS_PER_SEC;
    te_free(n);

    /*Million floats per second input.*/
    printf(" %.5g", d);
    if (eelapsed)
        printf("\t%5dms\t%5dmfps\n", eelapsed, loops * loops / eelapsed / 1000);
    else
        printf("\tinf\n");


    printf("%.2f%% longer\n", (((double)eelapsed / nelapsed) - 1.0) * 100.0);


    printf("\n");
}


double a5(double a) {
    return a+5;
}

double a52(double a) {
    return (a+5)*2;
}

double a10(double a) {
    return a+(5*2);
}

double as(double a) {
    return sqrt(pow(a, 1.5) + pow(a, 2.5));
}

double al(double a) {
    return (1/(a+1)+2/(a+2)+3/(a+3));
}

int main(int argc, char *argv[])
{

    bench("sqrt(a^1.5+a^2.5)", as);
    bench("a+5", a5);
    bench("a+(5*2)", a10);
    bench("(a+5)*2", a52);
    bench("(1/(a+1)+2/(a+2)+3/(a+3))", al);

    return 0;
}
