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

#include "tinyexpr.h"
#include <stdio.h>
#include "minctest.h"
#include <array>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>


typedef struct {
    const char *expr;
    double answer;
} test_case;

typedef struct {
    const char *expr1;
    const char *expr2;
} test_equ;



void test_results() {
    test_case cases[] = {
        {"1", 1},
        {"1 ", 1},
        {"(1)", 1},

        {"pi", 3.14159},
        {"atan(1)*4 - pi", 0},
        {"e", 2.71828},

        {"2+1", 2+1},
        {"(((2+(1))))", 2+1},
        {"3+2", 3+2},

        {"3+2+4", 3+2+4},
        {"(3+2)+4", 3+2+4},
        {"3+(2+4)", 3+2+4},
        {"(3+2+4)", 3+2+4},

        {"3*2*4", 3*2*4},
        {"(3*2)*4", 3*2*4},
        {"3*(2*4)", 3*2*4},
        {"(3*2*4)", 3*2*4},

        {"3-2-4", 3-2-4},
        {"(3-2)-4", (3-2)-4},
        {"3-(2-4)", 3-(2-4)},
        {"(3-2-4)", 3-2-4},

        {"3/2/4", 3.0/2.0/4.0},
        {"(3/2)/4", (3.0/2.0)/4.0},
        {"3/(2/4)", 3.0/(2.0/4.0)},
        {"(3/2/4)", 3.0/2.0/4.0},

        {"(3*2/4)", 3.0*2.0/4.0},
        {"(3/2*4)", 3.0/2.0*4.0},
        {"3*(2/4)", 3.0*(2.0/4.0)},

        {"asin sin .5", 0.5},
        {"sin asin .5", 0.5},
        {"ln exp .5", 0.5},
        {"exp ln .5", 0.5},

        {"asin sin-.5", -0.5},
        {"asin sin-0.5", -0.5},
        {"asin sin -0.5", -0.5},
        {"asin (sin -0.5)", -0.5},
        {"asin (sin (-0.5))", -0.5},
        {"asin sin (-0.5)", -0.5},
        {"(asin sin (-0.5))", -0.5},

        {"log10 1000", 3},
        {"log10 1e3", 3},
        {"log10 1000", 3},
        {"log10 1e3", 3},
        {"log10(1000)", 3},
        {"log10(1e3)", 3},
        {"log10 1.0e3", 3},
        {"10^5*5e-5", 5},

#ifdef TE_NAT_LOG
        {"log 1000", 6.9078},
        {"log e", 1},
        {"log (e^10)", 10},
#else
        {"log 1000", 3},
#endif

        {"ln (e^10)", 10},
        {"100^.5+1", 11},
        {"100 ^.5+1", 11},
        {"100^+.5+1", 11},
        {"100^--.5+1", 11},
        {"100^---+-++---++-+-+-.5+1", 11},

        {"100^-.5+1", 1.1},
        {"100^---.5+1", 1.1},
        {"100^+---.5+1", 1.1},
        {"1e2^+---.5e0+1e0", 1.1},
        {"--(1e2^(+(-(-(-.5e0))))+1e0)", 1.1},

        {"sqrt 100 + 7", 17},
        {"sqrt 100 * 7", 70},
        {"sqrt (100 * 100)", 100},

        {"1,2", 2},
        {"1,2+1", 3},
        {"1+1,2+2,2+1", 3},
        {"1,2,3", 3},
        {"(1,2),3", 3},
        {"1,(2,3)", 3},
        {"-(1,(2,3))", -3},

        {"2^2", 4},
        {"pow(2,2)", 4},

        {"atan2(1,1)", 0.7854},
        {"atan2(1,2)", 0.4636},
        {"atan2(2,1)", 1.1071},
        {"atan2(3,4)", 0.6435},
        {"atan2(3+3,4*2)", 0.6435},
        {"atan2(3+3,(4*2))", 0.6435},
        {"atan2((3+3),4*2)", 0.6435},
        {"atan2((3+3),(4*2))", 0.6435},
        {"max(9, 7)", 9 },
        {"min(9, 7)", 7 },
        { "mod(12, 10)", 2 },
        { "sign(-7.9)", -1 },
        { "sign(7.9)", 1 },
        { "sign(0)", 0 },
        { "trunc(9.57878423)", 9 },
        { "trunc(9.3)", 9 },
        // variadic functions
        { "round(9.57878423, 0)", 10 },
        { "round(9.57878423)", 10 },
        { "round(pow(2,2))", 4 }, // non-variadic function inside of variadic
        { "round(9.57878423, 1)", 9.6 },
        { "round(9.57878423, 2)", 9.58 },
        { "round(9.57878423, 3)", 9.579 },
        { "sum(9)", 9 },
        { "sum(9,9)", 18 },
        { "sum(9,9,9)", 27 },
        { "sum(9,9,9,9)", 36 },
        { "sum(9,9,9,9,9)", 45 },
        { "sum(9,9,9,9,9,9)", 54 },
        { "sum(pow(3,2),sum(3,3,3),9,pow(3,2),6+3,9,9)", 63 },
        { "pow(3,2)+sum(pow(3,2),sum(3,3,3),9,pow(3,2),6+3,9,9)", 72 },
        { "pow(2, sum(2,2))", 16 },
        { "average(1)", 1 },
        { "average(1,2)", 1.5 },
        { "average(1,2,3)", 2 },
        { "average(1,2,3,4)", 2.5 },
        { "average(1,2,3,4,5)", 3 },
        { "average(1,2,3,4,5,6)", 3.5 },
        { "average(1,2,3,4,5,6,7)", 4 },
        // logical
        { "if(1, 9, 7)", 9 },
        { "if(0, 9, 7)", 7 },
        { "and(0.0, 5)", 0 },
        { "and(0.0, 0)", 0 },
        { "AND(-1, 5)", 1 },
        { "AND(1, 1)", 1 },
        { "or(-1, 0.0)", 1 },
        { "or(0.0, 5)", 1 },
        { "or(0.0, 0)", 0 },
        { "OR(-1, 5)", 1 },
        { "OR(1, 1)", 1 },
        { "not(-1)", 0 },
        { "not(0.0)", 1 },
        { "NOT(0)", 1 },
        { "NOT(5)", 0 },
        // operators
        { "0.0 & 5", 0 },
        { "0.0 & 0", 0 },
        { "-1 & 5", 1 },
        { "1 & 1", 1 },
        { "0.0 | 5", 1 },
        { "0.0 | 0", 0 },
        { "-1 | 5", 1 },
        { "1 | 1", 1 },
        { "-1 | 0.0", 1 },

    };


    int i;
    te_parser tep;
    for (i = 0; i < sizeof(cases) / sizeof(test_case); ++i) {
        const char *expr = cases[i].expr;
        const double answer = cases[i].answer;

        
        const double ev = tep.evaluate(expr);
        lok(tep.get_last_error_position() == -1);
        lfequal(ev, answer);

        if (tep.get_last_error_position() != -1) {
            printf("FAILED: %s (%d)\n", expr, tep.get_last_error_position());
        }
    }
}


void test_syntax() {
    test_case errors[] = {
        {"", 0},
        {"1+", 1},
        {"1)", 1},
        {"(1", 1},
        {"1**1", 2},
        {"1*2(+4", 3},
        {"1*2(1+4", 3},
        {"a+5", 0},
        {"A+5", 0},
        {"aA+5", 1},
        {"1^^5", 2},
        {"1**5", 2},
        {"sin(cos5", 7},
        {"average()", 8}, // needs at least 1 arg
        {"sum()", 4}, // needs at least 1 arg
    };


    int i;
    for (i = 0; i < sizeof(errors) / sizeof(test_case); ++i) {
        const char *expr = errors[i].expr;
        const int e = (int)errors[i].answer;

        
        te_parser tep;
        const double r = tep.evaluate(expr);
        lequal(tep.get_last_error_position(), e);
        lok(r != r);

        auto a = tep.evaluate(expr);
        lequal(tep.get_last_error_position(), e);
        lok(!tep.success());

        if (tep.get_last_error_position() != e) {
            printf("FAILED: %s\n", expr);
        }

        const double k = tep.evaluate(expr);
        lok(k != k);
    }
}


void test_nans() {

    const char *nans[] = {
        "0/0",
        "1%0",
        "1%(1%0)",
        "(1%0)%1",
        "fac(-1)",
        "ncr(2, 4)",
        "ncr(-2, 4)",
        "ncr(2, -4)",
        "npr(2, 4)",
        "npr(-2, 4)",
        "npr(2, -4)",
    };

    int i;
    for (i = 0; i < sizeof(nans) / sizeof(const char *); ++i) {
        const char *expr = nans[i];

        
        te_parser tep;
        const double r = tep.evaluate(expr);
        lequal(tep.get_last_error_position(), -1);
        lok(r != r);

        const double c = tep.evaluate();
        lok(tep.success());
        lequal(tep.get_last_error_position(), -1);
        lok(c != c);
    }
}


void test_infs() {

    const char *infs[] = {
            "1/0",
            "log(0)",
            "pow(2,10000000)",
            "fac(300)",
            "ncr(300,100)",
            "ncr(300000,100)",
            "ncr(300000,100)*8",
            "npr(3,2)*ncr(300000,100)",
            "npr(100,90)",
            "npr(30,25)",
    };

    int i;
    for (i = 0; i < sizeof(infs) / sizeof(const char *); ++i) {
        const char *expr = infs[i];

        
        te_parser tep;
        const double r = tep.evaluate(expr);
        lequal(tep.get_last_error_position(), -1);
        lok(r == r + 1);

        const double c = tep.evaluate();
        lok(tep.success());
        lequal(tep.get_last_error_position(), -1);
        lok(c == c + 1);
    }
}


void test_variables() {

    double x, y, test;

    te_parser tep;
    tep.set_vars({{"x", &x}, {"y", &y}, {"te_st", &test}});

    auto expr1 = tep.evaluate("cos x + sin y");
    lok(tep.success());
    lok(tep.get_last_error_position() == -1);

    auto expr2 = tep.evaluate("x+x+x-y");
    lok(tep.success());
    lok(tep.get_last_error_position() == -1);

    auto expr3 = tep.evaluate("x*y^3");
    lok(tep.success());
    lok(tep.get_last_error_position() == -1);

    auto expr4 = tep.evaluate("te_st+5");
    lok(tep.success());
    lok(tep.get_last_error_position() == -1);

    for (y = 2; y < 3; ++y) {
        for (x = 0; x < 5; ++x) {
            double ev;

            ev = tep.evaluate("cos x + sin y");
            lfequal(ev, cos(x) + sin(y));

            ev = tep.evaluate("x+x+x-y");
            lfequal(ev, x+x+x-y);

            ev = tep.evaluate("x*y^3");
            lfequal(ev, x*y*y*y);

            test = x;
            ev = tep.evaluate("te_st+5");
            lfequal(ev, x+5);
        }
    }

    auto a = tep.evaluate("xx*y^3");
    lok(!tep.success());
    lok(tep.get_last_error_position() != -1);

    a = tep.evaluate("tes");
    lok(!tep.success());
    lok(tep.get_last_error_position() != -1);

    a = tep.evaluate("sinn x");
    lok(!tep.success());
    lok(tep.get_last_error_position() != -1);

    a = tep.evaluate("si x");
    lok(!tep.success());
    lok(tep.get_last_error_position() != -1);
}



#define cross_check(a, b) do {\
    if ((b)!=(b)) break;\
    auto c = tep.evaluate((a));\
    lfequal(c, (b));\
    lok(tep.get_last_error_position() == -1);\
}while(0)

void test_functions() {

    double x, y;

    te_parser tep;
    tep.set_vars({ {"x", &x}, {"y", &y} });

    for (x = -5; x < 5; x += .2) {
        cross_check("abs x", fabs(x));
        cross_check("acos x", acos(x));
        cross_check("asin x", asin(x));
        cross_check("atan x", atan(x));
        cross_check("ceil x", ceil(x));
        cross_check("cos x", cos(x));
        cross_check("cosh x", cosh(x));
        cross_check("exp x", exp(x));
        cross_check("floor x", floor(x));
        cross_check("ln x", log(x));
        cross_check("log10 x", log10(x));
        cross_check("sin x", sin(x));
        cross_check("sinh x", sinh(x));
        cross_check("sqrt x", sqrt(x));
        cross_check("tan x", tan(x));
        cross_check("tanh x", tanh(x));

        for (y = -2; y < 2; y += .2) {
            if (fabs(x) < 0.01) break;
            cross_check("atan2(x,y)", atan2(x, y));
            cross_check("pow(x,y)", pow(x, y));
        }
    }

    // ucased formulas
    for (x = -5; x < 5; x += .2) {
        cross_check("ABS x", fabs(x));
        cross_check("ACOS x", acos(x));
        cross_check("ASIN x", asin(x));
        cross_check("ATAN x", atan(x));
        cross_check("CEIL x", ceil(x));
        cross_check("COS x", cos(x));
        cross_check("COSH x", cosh(x));
        cross_check("EXP X", exp(x));
        cross_check("FLOOR x", floor(x));
        cross_check("LN x", log(x));
        cross_check("LOG10 x", log10(x));
        cross_check("SIN x", sin(x));
        cross_check("SINH x", sinh(x));
        cross_check("SQRT x", sqrt(x));
        cross_check("TAN x", tan(x));
        cross_check("TANH x", tanh(x));

        for (y = -2; y < 2; y += .2) {
            if (fabs(x) < 0.01) break;
            cross_check("ATAN2(x,y)", atan2(x, y));
            cross_check("POW(x,y)", pow(x, y));
        }
    }
}


double sum0() {
    return 6;
}
double sum1(double a) {
    return a * 2;
}
double sum2(double a, double b) {
    return a + b;
}
double sum3(double a, double b, double c) {
    return a + b + c;
}
double sum4(double a, double b, double c, double d) {
    return a + b + c + d;
}
double sum5(double a, double b, double c, double d, double e) {
    return a + b + c + d + e;
}
double sum6(double a, double b, double c, double d, double e, double f) {
    return a + b + c + d + e + f;
}
double sum7(double a, double b, double c, double d, double e, double f, double g) {
    return a + b + c + d + e + f + g;
}


void test_dynamic() {

    double x, f;
    std::vector<te_variable> lookup = {
        {"x", &x},
        {"f", &f},
        {"sum0", sum0},
        {"sum1", sum1},
        {"sum2", sum2},
        {"sum3", sum3},
        {"sum4", sum4},
        {"sum5", sum5},
        {"sum6", sum6},
        {"sum7", sum7},
    };

    test_case cases[] = {
        {"x", 2},
        {"f+x", 7},
        {"x+x", 4},
        {"x+f", 7},
        {"f+f", 10},
        {"f+sum0", 11},
        {"sum0+sum0", 12},
        {"sum0()+sum0", 12},
        {"sum0+sum0()", 12},
        {"sum0()+(0)+sum0()", 12},
        {"sum1 sum0", 12},
        {"sum1(sum0)", 12},
        {"sum1 f", 10},
        {"sum1 x", 4},
        {"sum2 (sum0, x)", 8},
        {"sum3 (sum0, x, 2)", 10},
        {"sum2(2,3)", 5},
        {"sum3(2,3,4)", 9},
        {"sum4(2,3,4,5)", 14},
        {"sum5(2,3,4,5,6)", 20},
        {"sum6(2,3,4,5,6,7)", 27},
        {"sum7(2,3,4,5,6,7,8)", 35},
    };

    x = 2;
    f = 5;

    int i;
    te_parser tep;
    tep.set_vars(lookup);
    for (i = 0; i < sizeof(cases) / sizeof(test_case); ++i) {
        const char *expr = cases[i].expr;
        const double answer = cases[i].answer;

        
        auto c = tep.evaluate(expr);
        lok(tep.success());
        lfequal(c, answer);
    }
}


double clo0(const te_expr* context) {
    if (context) return *(std::get<const double*>(context->m_value)) + 6;
    return 6;
}
double clo1(const te_expr* context, double a) {
    if (context) return *(std::get<const double*>(context->m_value)) + a * 2;
    return a * 2;
}
double clo2(const te_expr* context, double a, double b) {
    if (context) return *(std::get<const double*>(context->m_value)) + a + b;
    return a + b;
}

class te_expr_array : public te_expr
    {
public:
    te_expr_array(const variable_flags type) noexcept : te_expr(type) {}
    std::array<double, 5> m_data = { 5,6,7,8,9 };
    };

double cell(const te_expr* context, double a) {
    auto *c = dynamic_cast<const te_expr_array*>(context);
    return c->m_data[(int)a];
}

void test_closure() {

    double extra{ 0 };
    
    te_expr te{ TE_DEFAULT, &extra };
    te_expr_array teArray{ TE_DEFAULT };
    
    std::vector<te_variable> lookup = {
        {"c0", clo0, TE_DEFAULT, &te},
        {"c1", clo1, TE_DEFAULT, &te},
        {"c2", clo2, TE_DEFAULT, &te},
        {"cell", cell, TE_DEFAULT, &teArray},
    };

    test_case cases[] = {
        {"c0", 6},
        {"c1 4", 8},
        {"c2 (10, 20)", 30},
    };

    int i;
    te_parser tep;
    tep.set_vars(lookup);
    for (i = 0; i < sizeof(cases) / sizeof(test_case); ++i) {
        const char *expr = cases[i].expr;
        const double answer = cases[i].answer;

        auto a = tep.evaluate(expr);
        lok(tep.success());

        extra = 0;
        auto c = tep.evaluate();
        lfequal(c, answer + extra);

        extra = 10;
        c = tep.evaluate();
        lfequal(c, answer + extra);
    }


    test_case cases2[] = {
        {"cell 0", 5},
        {"cell 1", 6},
        {"cell 0 + cell 1", 11},
        {"cell 1 * cell 3 + cell 4", 57},
    };

    for (i = 0; i < sizeof(cases2) / sizeof(test_case); ++i) {
        const char *expr = cases2[i].expr;
        const double answer = cases2[i].answer;

        auto c = tep.evaluate(expr);
        lok(tep.success());
        lfequal(c, answer);
    }
}

void test_optimize() {

    test_case cases[] = {
        {"5+5", 10},
        {"pow(2,2)", 4},
        {"sqrt 100", 10},
        {"pi * 2", 6.2832},
    };

    int i;
    te_parser tep;
    for (i = 0; i < sizeof(cases) / sizeof(test_case); ++i) {
        const char *expr = cases[i].expr;
        const double answer = cases[i].answer;

        auto c = tep.evaluate(expr);
        lok(tep.success());

        lfequal(c, answer);
    }
}

void test_pow() {
#ifdef TE_POW_FROM_RIGHT
    test_equ cases[] = {
        {"2^3^4", "2^(3^4)"},
        {"-2^2", "-(2^2)"},
        {"--2^2", "(2^2)"},
        {"---2^2", "-(2^2)"},
        {"-(2*1)^2", "-(2^2)"},
        {"-2^2", "-4"},
        {"2^1.1^1.2^1.3", "2^(1.1^(1.2^1.3))"},
        {"-a^b", "-(a^b)"},
        {"-a^-b", "-(a^-b)"},
        {"1^0", "1"},
        {"(1)^0", "1"},
        {"-(2)^2", "-(2^2)"}
        /* TODO POW FROM RIGHT IS STILL BUGGY
        {"(-2)^2", "4"},
        {"(-1)^0", "1"},
        {"(-5)^0", "1"},
        {"-2^-3^-4", "-(2^(-(3^-4)))"}*/
    };
#else
    test_equ cases[] = {
        {"2^3^4", "(2^3)^4"},
        {"-2^2", "(-2)^2"},
        {"(-2)^2", "4"},
        {"--2^2", "2^2"},
        {"---2^2", "(-2)^2"},
        {"-2^2", "4"},
        {"2^1.1^1.2^1.3", "((2^1.1)^1.2)^1.3"},
        {"-a^b", "(-a)^b"},
        {"-a^-b", "(-a)^(-b)"},
        {"1^0", "1"},
        {"(1)^0", "1"},
        {"(-1)^0", "1"},
        {"(-5)^0", "1"},
        {"-2^-3^-4", "((-2)^(-3))^(-4)"}
    };
#endif

    double a = 2, b = 3;

    std::vector<te_variable> lookup = {
        {"a", &a},
        {"b", &b}
    };

    int i;
    te_parser tep;
    tep.set_vars(lookup);
    for (i = 0; i < sizeof(cases) / sizeof(test_equ); ++i) {
        const char *expr1 = cases[i].expr1;
        const char *expr2 = cases[i].expr2;

        double r1 = tep.evaluate(expr1);
        lok(tep.success());
        double r2 = tep.evaluate(expr2);
        lok(tep.success());

        fflush(stdout);
        const int olfail = lfails;
        lfequal(r1, r2);
        if (olfail != lfails) {
            printf("Failed expression: %s <> %s\n", expr1, expr2);
        }
    }

}

void test_combinatorics() {
    test_case cases[] = {
            {"fac(0)", 1},
            {"fac(0.2)", 1},
            {"fac(1)", 1},
            {"fac(2)", 2},
            {"fac(3)", 6},
            {"fac(4.8)", 24},
            {"fac(10)", 3628800},

            {"ncr(0,0)", 1},
            {"ncr(10,1)", 10},
            {"ncr(10,0)", 1},
            {"ncr(10,10)", 1},
            {"ncr(16,7)", 11440},
            {"ncr(16,9)", 11440},
            {"ncr(100,95)", 75287520},

            {"npr(0,0)", 1},
            {"npr(10,1)", 10},
            {"npr(10,0)", 1},
            {"npr(10,10)", 3628800},
            {"npr(20,5)", 1860480},
            {"npr(100,4)", 94109400},
    };


    int i;
    te_parser tep;
    for (i = 0; i < sizeof(cases) / sizeof(test_case); ++i) {
        const char *expr = cases[i].expr;
        const double answer = cases[i].answer;

        const double ev = tep.evaluate(expr);
        lok(tep.get_last_error_position() == -1);
        lfequal(ev, answer);

        if (tep.get_last_error_position() != -1) {
            printf("FAILED: %s (%d)\n", expr, tep.get_last_error_position());
        }
    }
}


int main(int argc, char *argv[])
{
    lrun("Results", test_results);
    lrun("Syntax", test_syntax);
    lrun("NaNs", test_nans);
    lrun("INFs", test_infs);
    lrun("Variables", test_variables);
    lrun("Functions", test_functions);
    lrun("Dynamic", test_dynamic);
    lrun("Closure", test_closure);
    lrun("Optimize", test_optimize);
    lrun("Pow", test_pow);
    lrun("Combinatorics", test_combinatorics);
    lresults();

    _CrtDumpMemoryLeaks();

    return lfails != 0;
}
