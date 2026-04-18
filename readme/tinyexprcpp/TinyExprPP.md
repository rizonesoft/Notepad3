# TinyExpr++ — Expression Evaluator in Notepad3

Notepad3 includes [TinyExpr++](https://blake-madden.github.io/tinyexpr-plusplus/), a math expression parser and evaluator. It is used in several places throughout the application to let you write and evaluate math formulas directly.

## Where Expressions Are Used

### 1. Inline Expression Evaluation (`=?`)

Type a math expression followed by `=?` (or `=` then press **Enter**) to evaluate it in-place.

**Enable:** Menu → Settings → *Evaluate TinyExpr on Selection*

**Example:**
```
(3+4)*5=?
```
The `=?` and the expression are replaced with the result: `35`

This also works with selected text — select a math expression, and its result appears in the **status bar**.

### 2. Status Bar Expression Display

When *Evaluate TinyExpr on Selection* is enabled, selecting any text containing a valid expression shows its computed value in the status bar's TinyExpr field.

**Multi-selection / rectangular selection** is supported — selected values are concatenated and evaluated as a single expression.

### 3. Go to Line / Column Dialog

The **Go to Line** dialog (Ctrl+G) accepts expressions, not just numbers:

| Input | Result |
|-------|--------|
| `100` | Goes to line 100 |
| `50+25` | Goes to line 75 |
| `2^10` | Goes to line 1024 |

The same applies to the **Column** field.

### 4. Line Number Expressions (Modify Lines)

The **Modify Lines** dialog (Alt+M) supports expressions in its prefix/append patterns using the `${expression}` syntax. Three variables are available:

| Variable | Description |
|----------|-------------|
| `L` | Current 1-based line number in the file |
| `I` | Zero-based iteration index (0, 1, 2, …) |
| `N` | One-based iteration counter (1, 2, 3, …) |

**Examples:**

| Pattern | Lines 5–8 produce |
|---------|--------------------|
| `${N}` | 1, 2, 3, 4 |
| `${L}` | 5, 6, 7, 8 |
| `${N*10}` | 10, 20, 30, 40 |
| `${L^2}` | 25, 36, 49, 64 |
| `${0N}` | 1, 2, 3, 4 (zero-padded) |

### 5. Auto-Save Interval

The auto-save interval field in Settings accepts expressions (e.g., `60*5` for 5 minutes).

### 6. Dark Mode Contrast Setting

The Dark Mode highlight contrast value in the Customize Schemes dialog accepts expressions.

---

## Expression Syntax

### Numbers

| Format | Example | Value |
|--------|---------|-------|
| Integer | `42` | 42 |
| Decimal | `3.14` | 3.14 |
| Leading dot | `.5` | 0.5 |
| Hexadecimal | `0x1F` | 31 |
| Scientific | `1e3` | 1000 |
| Negative scientific | `2.5e-2` | 0.025 |

### Operators (by Precedence)

From highest to lowest precedence:

| Precedence | Operators | Description |
|:----------:|-----------|-------------|
| 1 | `( )` | Grouping |
| 2 | `+` `-` `~` | Unary plus, minus, bitwise NOT |
| 3 | `^` `**` | Exponentiation |
| 4 | `*` `/` `%` | Multiply, divide, modulus |
| 5 | `+` `-` | Add, subtract |
| 6 | `<<` `>>` `<<<` `>>>` | Bit shift, bit rotate |
| 7 | `<` `>` `<=` `>=` | Relational comparison |
| 8 | `=` `==` `<>` `!=` | Equality / inequality |
| 9 | `&&` | Logical AND |
| 10 | `\|\|` | Logical OR |

> **Note:** `%` is the **modulus** operator (remainder), not a percentage sign.

**Examples:**
```
5+5+5/2       → 12.5   (division before addition)
(5+5+5)/2     → 7.5    (parentheses override precedence)
2+5^2         → 27     (exponentiation before addition)
(2+5)^2       → 49
10 % 3        → 1      (remainder of 10÷3)
```

### Constants

| Constant | Value |
|----------|-------|
| `PI` | 3.14159265358979… |
| `E` | 2.71828182845905… |
| `TRUE` | 1 |
| `FALSE` | 0 |
| `NAN` | Not-a-Number (invalid) |

### Comments

C/C++ style comments are supported inside expressions:

```
(3 + 4) /* this is a comment */ * 2    → 14
5 + 3   // rest of line is ignored     → 8
```

---

## Function Reference

All function names are **case-insensitive** (`sin`, `SIN`, `Sin` all work).

### Basic Math

| Function | Description | Example |
|----------|-------------|---------|
| `ABS(x)` | Absolute value | `ABS(-5)` → `5` |
| `CEIL(x)` | Round up to integer | `CEIL(2.3)` → `3` |
| `FLOOR(x)` | Round down to integer | `FLOOR(2.7)` → `2` |
| `ROUND(x, n)` | Round to *n* decimal places | `ROUND(3.456, 2)` → `3.46` |
| `TRUNC(x)` | Discard fractional part | `TRUNC(3.7)` → `3` |
| `SIGN(x)` | Sign: 1, 0, or −1 | `SIGN(-7)` → `-1` |
| `CLAMP(x, lo, hi)` | Constrain to range | `CLAMP(15, 0, 10)` → `10` |
| `EVEN(x)` | Round up to nearest even | `EVEN(3)` → `4` |
| `ODD(x)` | Round up to nearest odd | `ODD(4)` → `5` |

### Powers & Roots

| Function | Description | Example |
|----------|-------------|---------|
| `SQRT(x)` | Square root | `SQRT(16)` → `4` |
| `POW(x, y)` | *x* raised to *y* | `POW(2, 10)` → `1024` |
| `POWER(x, y)` | Alias for `POW` | |
| `EXP(x)` | *e* raised to *x* | `EXP(1)` → `2.71828…` |

### Logarithms

| Function | Description | Example |
|----------|-------------|---------|
| `LN(x)` | Natural log (base *e*) | `LN(E)` → `1` |
| `LOG10(x)` | Common log (base 10) | `LOG10(1000)` → `3` |
| `LOG(x)` | Same as `LOG10` (compatibility) | `LOG(100)` → `2` |

### Trigonometry

| Function | Description |
|----------|-------------|
| `SIN(x)` | Sine (radians) |
| `COS(x)` | Cosine (radians) |
| `TAN(x)` | Tangent |
| `COT(x)` | Cotangent |
| `ASIN(x)` | Arcsine → radians |
| `ACOS(x)` | Arccosine → radians |
| `ATAN(x)` | Arctangent → radians |
| `ATAN2(y, x)` | Arctangent of *y/x* → radians |
| `SINH(x)` | Hyperbolic sine |
| `COSH(x)` | Hyperbolic cosine |

### Statistics

| Function | Description | Example |
|----------|-------------|---------|
| `SUM(a, b, …)` | Sum of values (up to 24 args) | `SUM(1,2,3)` → `6` |
| `AVERAGE(a, b, …)` | Mean of values (up to 24 args) | `AVERAGE(2,4,6)` → `4` |
| `MIN(a, b, …)` | Smallest value (up to 24 args) | `MIN(3,1,2)` → `1` |
| `MAX(a, b, …)` | Largest value (up to 24 args) | `MAX(3,1,2)` → `3` |

### Combinatorics

| Function | Description | Example |
|----------|-------------|---------|
| `FAC(n)` / `FACT(n)` | Factorial (*n!*) | `FAC(5)` → `120` |
| `COMBIN(n, k)` / `NCR(n, k)` | Combinations | `COMBIN(5,2)` → `10` |
| `PERMUT(n, k)` / `NPR(n, k)` | Permutations | `PERMUT(5,2)` → `20` |
| `TGAMMA(x)` / `GAMMA(x)` | Gamma function | |

### Logic

| Function | Description | Example |
|----------|-------------|---------|
| `IF(cond, yes, no)` | Conditional | `IF(1>2, 10, 20)` → `20` |
| `IFS(c1,v1, c2,v2, …)` | Multi-condition (up to 12 pairs) | `IFS(0,1, 1,2)` → `2` |
| `AND(a, b, …)` | All true? (up to 24 args) | `AND(1,1,0)` → `0` |
| `OR(a, b, …)` | Any true? (up to 24 args) | `OR(0,0,1)` → `1` |
| `NOT(x)` | Logical negation | `NOT(0)` → `1` |

### Error Checking

| Function | Description |
|----------|-------------|
| `NA()` / `NAN` | Returns NaN (invalid value) |
| `ISERR(x)` / `ISERROR(x)` | 1 if *x* is NaN, else 0 |
| `ISNA(x)` / `ISNAN(x)` | Alias for `ISERR` |
| `ISEVEN(x)` | 1 if *x* is even |
| `ISODD(x)` | 1 if *x* is odd |

### Bitwise Operations

| Function | Description |
|----------|-------------|
| `BITAND(a, b)` | Bitwise AND |
| `BITOR(a, b)` | Bitwise OR |
| `BITXOR(a, b)` | Bitwise XOR |
| `BITNOT(x)` | Bitwise NOT (auto-selects 32 or 64 bit) |
| `BITLSHIFT(x, n)` | Left shift by *n* |
| `BITRSHIFT(x, n)` | Right shift by *n* |
| `RAND()` | Random number between 0 and 1 |

### Financial Functions

| Function | Description |
|----------|-------------|
| `FV(rate, nper, pmt, [pv], [type])` | Future value |
| `PV(rate, nper, pmt, [fv], [type])` | Present value |
| `PMT(rate, nper, pv, [fv], [type])` | Periodic payment |
| `NPER(rate, pmt, pv, [fv], [type])` | Number of periods |
| `IPMT(rate, per, nper, pv, [fv], [type])` | Interest portion |
| `PPMT(rate, per, nper, pv, [fv], [type])` | Principal portion |

> Optional parameters `[…]` default to 0. The `type` parameter: `0` = end of period, `1` = beginning.

---

## Practical Examples

### Quick Calculations

```
256*1024=?                → 262144
SQRT(3^2 + 4^2)=?        → 5
2*PI*6.371e6=?            → 40030173.592…  (Earth's circumference in meters)
```

### Unit Conversions

```
72 * 0.0254=?             → 1.8288   (72 inches to meters)
100 / 2.54=?              → 39.3701  (100 cm to inches)
(98.6 - 32) * 5/9=?      → 37       (Fahrenheit to Celsius)
```

### Programming Helpers

```
0xFF=?                    → 255
2^16 - 1=?               → 65535
BITLSHIFT(1, 20)=?       → 1048576  (1 MB in bytes)
BITAND(0xF0, 0x3C)=?     → 48
```

### Line Numbering (Modify Lines Dialog)

| Goal | Prefix pattern |
|------|---------------|
| Simple line numbers | `${N}. ` |
| Multiples of 10 | `${N*10}: ` |
| Even numbers only | `${N*2}: ` |
| Squared sequence | `${N^2} ` |
| Zero-padded (001, 002…) | `${0N}. ` |
| Reverse from 100 | `${100-I} ` |
| Starting from line number | `${L}: ` |

### Conditional Logic

```
IF(5>3, 100, 200)=?                         → 100
IF(AND(10>=5, 10<=15), 1, 0)=?              → 1   (range check)
IFS(90>=90, 4, 90>=80, 3, 90>=70, 2, 1,1)=? → 4   (grading)
```

---

## Locale-Aware Number Formatting

TinyExpr++ in Notepad3 automatically adapts to the **decimal separator** of the active UI language. When Notepad3 is set to a locale that uses a comma as the decimal mark (e.g., German, French, Spanish), the expression engine switches accordingly:

| | English (en-US) | German (de-DE) |
|---|---|---|
| **Decimal separator** | `.` (dot) | `,` (comma) |
| **Function argument separator** | `,` (comma) | `;` (semicolon) |
| **Number example** | `3.14` | `3,14` |
| **Function call** | `SUM(1.5, 2)` | `SUM(1,5; 2)` |
| **Inline evaluation** | `1/3=?` → `0.33333333` | `1/3=?` → `0,33333333` |

### Examples by Locale

**English (dot decimal):**
```
ROUND(PI, 4)=?           → 3.1416
SUM(1.5, 2.5, 3)=?       → 7
IF(2.5 > 1, 10, 20)=?    → 10
```

**German / French / Spanish (comma decimal):**
```
ROUND(PI; 4)=?            → 3,1416
SUM(1,5; 2,5; 3)=?        → 7
IF(2,5 > 1; 10; 20)=?     → 10
```

> **Tip:** The separator style follows the Notepad3 UI language, which is set in *Settings → Preferred Language*. It does not change when editing files in different encodings.

---

## Notes

- All function names are **case-insensitive**.
- `LOG(x)` is provided for backward compatibility and evaluates as `LOG10(x)`.
- The `%` operator computes the **modulus** (remainder), not a percentage.
- Expressions support **C/C++ style comments** (`/* … */` and `// …`).
- Invalid operations (division by zero, `SQRT(-1)`) return **NaN**.

For the full TinyExpr++ reference, see the [TinyExpr++ documentation](https://blake-madden.github.io/tinyexpr-plusplus/).
