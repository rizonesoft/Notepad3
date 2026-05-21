/*
 * TinyExpr++ Expression Test File for Notepad3
 * =============================================
 *
 * How to use:
 *   1. Open this file in Notepad3.
 *   2. Enable Settings -> "Evaluate TinyExpr on Selection".
 *   3. For any test line: place the caret IMMEDIATELY after '=?' and press
 *      ENTER (the trigger replaces '<expr>=?' inline with the result), OR
 *      select the expression and read the status-bar TinyExpr field.
 *   4. Compare against the expected value shown in the trailing comment.
 *
 * Notes on output format (post-change):
 *   * Numeric values use '%.15g' (decimal) and '%.21g' (integer-like, when
 *     fractional part is below 1.0e-15 and magnitude is below 1.0e+21).
 *   * Non-finite values render as 'nan' / 'inf' / '-inf' (lowercase).
 *   * Boolean-detected expressions render as 'true' / 'false':
 *       - Lexical hit at parenthesis depth 0 on one of:
 *         == = != <> <= >= < > && || !  (or top-level call to
 *         AND / OR / NOT / ISERR / ISERROR / ISNA / ISNAN / ISEVEN / ISODD,
 *         or the bare keywords true / false)
 *       AND
 *       - Evaluated result is finite and exactly 0.0 or 1.0.
 *   * Hex / binary status-bar output modes apply to NUMERIC results only;
 *     boolean results override the mode and always show as 'true' / 'false'.
 */
 
//  Document-level commentary uses '//' or '/* ... */' C++-style
//  comments, which the TinyExpr++ parser also recognizes and strips.
//  Each test line is an expression terminated by '=?' (the inline-
//  evaluation trigger) followed by '// <expected-value>' so the
//  intended result stays visible after the trigger replaces '=?'.
//  
//  Opens in Notepad3 with the C/C++ lexer for syntax highlighting.


// ============================================================
// 1. Basic arithmetic
// ============================================================

1+1=?                          // 2
10-3=?                         // 7
6*7=?                          // 42
20/4=?                         // 5
10%3=?                         // 1   (modulus, not percent)
2^10=?                         // 1024
2**10=?                        // 1024 (alternative power syntax)
-5+3=?                         // -2
+5-3=?                         // 2
1+2+3+4+5=?                    // 15


// ============================================================
// 2. Operator precedence
// ============================================================

5+5+5/2=?                      // 12.5  (division before addition)
(5+5+5)/2=?                    // 7.5
2+5^2=?                        // 27    (exponentiation before addition)
(2+5)^2=?                      // 49
2*3+4=?                        // 10
2*(3+4)=?                      // 14
~5=?                           // -6    (bitwise NOT - requires TE_BITWISE_OPERATORS)


// ============================================================
// 3. Number formats
// ============================================================

42=?                           // 42
3.14=?                         // 3.14
.5=?                           // 0.5
0x1F=?                         // 31              (hexadecimal)
0xFF=?                         // 255
0xFFFF=?                       // 65535
0b101010=?                     // 42              (binary - NP3 extension)
0b11111111=?                   // 255
1e3=?                          // 1000            (scientific)
2.5e-2=?                       // 0.025
1.5e10=?                       // 15000000000


// ============================================================
// 4. Basic math functions
// ============================================================

ABS(-5)=?                      // 5
ABS(7)=?                       // 7
CEIL(2.3)=?                    // 3
CEIL(-2.3)=?                   // -2
FLOOR(2.7)=?                   // 2
FLOOR(-2.7)=?                  // -3
ROUND(3.456, 2)=?              // 3.46
ROUND(2.5, 0)=?                // 3
ROUND(-2.5, 0)=?               // -3
TRUNC(3.7)=?                   // 3
TRUNC(-3.7)=?                  // -3
SIGN(-7)=?                     // -1
SIGN(7)=?                      // 1
SIGN(0)=?                      // 0
CLAMP(15, 0, 10)=?             // 10
CLAMP(-5, 0, 10)=?             // 0
CLAMP(5, 0, 10)=?              // 5
EVEN(3)=?                      // 4
EVEN(-3)=?                     // -4
ODD(4)=?                       // 5


// ============================================================
// 5. Powers and roots
// ============================================================

SQRT(16)=?                     // 4
SQRT(2)=?                      // ~1.4142135623731
SQRT(0)=?                      // 0
POW(2, 10)=?                   // 1024
POW(2, 0)=?                    // 1
POW(0, 0)=?                    // 1
POWER(3, 4)=?                  // 81
EXP(0)=?                       // 1
EXP(1)=?                       // ~2.71828182845905


// ============================================================
// 6. Logarithms
// ============================================================

LN(E)=?                        // 1
LN(1)=?                        // 0
LOG10(1000)=?                  // 3
LOG10(1)=?                     // 0
LOG10(100000)=?                // 5
LOG(100)=?                     // 2     (LOG = LOG10 for compatibility)
log(1000)=?                    // 3     (lowercase 'log' - NP3 compat shim)


// ============================================================
// 7. Trigonometry (angles in radians)
// ============================================================

SIN(0)=?                       // 0
SIN(PI/2)=?                    // 1
COS(0)=?                       // 1
COS(PI)=?                      // -1
TAN(0)=?                       // 0
ASIN(1)=?                      // ~1.5707963267949   (= PI/2)
ACOS(1)=?                      // 0
ATAN(1)=?                      // ~0.7853981633974   (= PI/4)
ATAN2(1, 1)=?                  // ~0.7853981633974   (= PI/4)
ATAN2(1, 0)=?                  // ~1.5707963267949   (= PI/2)
SINH(0)=?                      // 0
COSH(0)=?                      // 1


// ============================================================
// 8. Statistics (variadic - up to 24 args)
// ============================================================

SUM(1, 2, 3)=?                 // 6
SUM(1, 2, 3, 4, 5)=?           // 15
SUM(1.5, 2.5, 3)=?             // 7
AVERAGE(2, 4, 6)=?             // 4
AVERAGE(1, 2, 3, 4, 5)=?       // 3
MIN(3, 1, 2)=?                 // 1
MIN(-5, -10, -2)=?             // -10
MAX(3, 1, 2)=?                 // 3
MAX(-5, -10, -2)=?             // -2


// ============================================================
// 9. Combinatorics
// ============================================================

FAC(0)=?                       // 1
FAC(5)=?                       // 120
FACT(6)=?                      // 720
COMBIN(5, 2)=?                 // 10
COMBIN(10, 3)=?                // 120
NCR(5, 2)=?                    // 10              (alias for COMBIN)
PERMUT(5, 2)=?                 // 20
PERMUT(10, 3)=?                // 720
NPR(5, 2)=?                    // 20              (alias for PERMUT)
TGAMMA(5)=?                    // 24              (= 4!)
GAMMA(6)=?                     // 120             (= 5!)


// ============================================================
// 10. Constants
// ============================================================

PI=?                           // 3.14159265358979
E=?                            // 2.71828182845905
TRUE=?                         // 1
FALSE=?                        // 0
NAN=?                          // nan


// ============================================================
// 11. NEW: Boolean detection - relational / equality operators
//     Result renders as 'true' / 'false' (lowercase).
// ============================================================

1==1=?                         // true
1==2=?                         // false
1=1=?                          // true            (lone '=' parses as '==')
1=2=?                          // false           (lone '=' parses as '==')
1+1=2+2=?                      // false           ((1+1) == (2+2) -> 2 == 4)
1+1=2=?                        // true            (parser-side, the surprising bit)
1!=2=?                         // true
1!=1=?                         // false
1<>2=?                         // true            (<> is alternative inequality)
1<>1=?                         // false
5<10=?                         // true
5>10=?                         // false
5<=5=?                         // true
5>=5=?                         // true
5<5=?                          // false
5>5=?                          // false


// ============================================================
// 12. NEW: Boolean detection - logical operators / functions
// ============================================================

1 && 1=?                       // true
1 && 0=?                       // false
0 && 0=?                       // false
1 || 0=?                       // true
0 || 0=?                       // false
!0=?                           // true
!1=?                           // false
AND(1, 1, 1)=?                 // true
AND(1, 0, 1)=?                 // false
OR(0, 0, 1)=?                  // true
OR(0, 0, 0)=?                  // false
NOT(0)=?                       // true
NOT(1)=?                       // false
TRUE && TRUE=?                 // true
FALSE || TRUE=?                // true


// ============================================================
// 13. Boolean detection - the depth-0 rule
//     Operators inside parens DON'T count - except a single fully-
//     enclosing outer pair is stripped first.
// ============================================================

(1==1)=?                       // true            (outer parens stripped)
((1==1))=?                     // true            (recursive stripping)
1+(1==1)=?                     // 2               (== is inside parens; result not 0/1)
(1==1)*(2==2)=?                // 1               (numeric; no depth-0 op)
(1==1)+(0==1)=?                // 1               (numeric; no depth-0 op)


// ============================================================
// 14. Conditionals - NOT detected as boolean
//     IF / IFS return arbitrary user-supplied branches; they are
//     intentionally NOT in the predicate list, so results render
//     as numeric values even when the condition is logical.
// ============================================================

IF(1>0, 100, 200)=?            // 100
IF(1<0, 100, 200)=?            // 200
IF(1==1, 42, 99)=?             // 42
IF(AND(5>1, 5<10), 1, 0)=?     // 1               (numeric; outer is IF, not AND)
IFS(0, 1, 1, 2)=?              // 2
IFS(90>=90, 4, 90>=80, 3, 90>=70, 2, 1, 1)=?      // 4


// ============================================================
// 15. Error checking
// ============================================================

NA()=?                         // nan
ISERR(NAN)=?                   // true
ISERR(5)=?                     // false
ISERROR(0/0)=?                 // true
ISNA(NAN)=?                    // true
ISNAN(1.5)=?                   // false
ISEVEN(4)=?                    // true
ISEVEN(3)=?                    // false
ISEVEN(0)=?                    // true
ISODD(5)=?                     // true
ISODD(4)=?                     // false


// ============================================================
// 16. Bitwise functions
// ============================================================

BITAND(0xF0, 0x3C)=?           // 48              (= 0x30)
BITAND(0b1100, 0b1010)=?       // 8               (= 0b1000)
BITOR(0xF0, 0x0F)=?            // 255             (= 0xFF)
BITOR(0b1100, 0b0011)=?        // 15
BITXOR(0xFF, 0xAA)=?           // 85              (= 0x55)
BITXOR(0b1010, 0b1010)=?       // 0
BITNOT(0)=?                    // 4294967295      (32-bit ~0 = 0xFFFFFFFF) - bit-width depends on build
BITLSHIFT(1, 4)=?              // 16
BITLSHIFT(1, 20)=?             // 1048576         (= 1 MiB)
BITRSHIFT(256, 4)=?            // 16
BITRSHIFT(0xFF00, 8)=?         // 255


// ============================================================
// 17. Bit-shift / bit-rotate operators
// ============================================================

1<<4=?                         // 16
256>>4=?                       // 16
0xFF<<8=?                      // 65280           (= 0xFF00)
0xFF00>>8=?                    // 255             (= 0xFF)


// ============================================================
// 18. Precision and rounding (new %.15g format)
// ============================================================

0.1+0.2=?                      // 0.3                       (IEEE-754 surprise hidden by %.15g)
1/3=?                          // 0.333333333333333
2/3=?                          // 0.666666666666667
100/2.54=?                     // 39.3700787401575          (cm to inches)
2*PI=?                         // 6.28318530717959
2*PI*6.371e6=?                 // 40030173.5921478          (Earth's circumference, meters)
1.0000000001=?                 // 1.0000000001              (preserved: > 1e-15 cutoff)
1.0000000000000001=?           // 1                         (clipped: <= 1e-15)
2^53=?                         // 9007199254740992          (largest exact integer in double)
2^60=?                         // 1152921504606846976       (still exact via %.21g)


// ============================================================
// 19. Non-finite results
// ============================================================

0/0=?                          // nan
1/0=?                          // inf
-1/0=?                         // -inf
SQRT(-1)=?                     // nan
LN(0)=?                        // -inf
LN(-1)=?                       // nan
NAN+1=?                        // nan
NAN==NAN=?                     // nan             (comparison involving NaN -> NaN, NOT 'false')
NAN==NAN || TRUE=?             // true            (short-circuit through TRUE keyword)


// ============================================================
// 20. Comments inside expressions
// ============================================================

(3 + 4) /* this is a block comment */ * 2=?       // 14
5 + /* inline */ 3=?                              // 8
2 /* multi
   line */ + 3=?                                  // 5
// Note: line-style '//' comments at the END of the line are tricky -
// the '=?' trigger doesn't know about comments, so put '=?' BEFORE
// any '//' annotation (which is the convention used throughout this file).


// ============================================================
// 21. Compound real-world expressions
// ============================================================

256*1024=?                     // 262144                    (1 MiB in bytes)
SQRT(3^2 + 4^2)=?              // 5                         (Pythagorean)
(98.6 - 32) * 5/9=?            // 37                        (F -> C)
72 * 0.0254=?                  // 1.8288                    (inches -> meters)
2^16 - 1=?                     // 65535                     (uint16 max)
2^32 - 1=?                     // 4294967295                (uint32 max)
IF(5>3, MAX(1,2), MIN(3,4))=?  // 2
SUM(1,2,3) + AVERAGE(4,6)=?    // 11                        (= 6 + 5)
ABS(SIN(PI))<1e-10=?           // true                      (PI is approximate -> SIN(PI) is tiny non-zero)


// ============================================================
// 22. Hex / binary output modes
//
// These tests illustrate how the status-bar TinyExpr field changes
// numeric formatting based on its mode (double-click the status
// field to cycle: Decimal -> Hex -> Binary).  Boolean results
// OVERRIDE the mode and always show as 'true' / 'false'.
// ============================================================

255=?                          // dec: 255      hex: 0xFF       bin: 0b11111111
4096=?                         // dec: 4096     hex: 0x1000     bin: 0b1000000000000
-1=?                           // dec: -1       hex: 0xFFFFFFFF bin: 0b11111111111111111111111111111111
1==1=?                         // true   (mode ignored)
1+1=2+2=?                      // false  (mode ignored)


// ============================================================
// End of test file.
// ============================================================
