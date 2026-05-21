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

#### Output modes — decimal · hexadecimal · binary

The TinyExpr status field can display the result in three integer-result formats. The active mode applies to every subsequent evaluation, and is **process-local** — it resets to *decimal* on the next Notepad3 launch.

| Mode | Example output | Notes |
|------|----------------|-------|
| Decimal *(default)* | `15`, `3.14`, `Inf` | Existing format; integers use up to 21 significant digits, floats `%.8G`. |
| Hexadecimal | `0xF`, `0xFF`, `0xFFFFFFFF` | Uppercase, no padding. |
| Binary | `0b1111`, `0b11111111` | Lowercase `b` prefix, no padding. |

In hex / binary, fractional results are rounded to the nearest integer (round-half-away-from-zero). Negative values are shown in **two's-complement** (`-1` → `0xFFFFFFFF` / 32 ones). Values outside the supported integer range, `NaN`, and `Inf` fall back to the decimal `%.8G` representation for that single evaluation.

**Effective bit width** is chosen at runtime via `te_supports_64bit()` — currently 32 bits on the standard MSVC build (where the parser's `double` has a 53-bit mantissa). The display widens automatically to 64 bits if the parser is ever rebuilt with a 64-bit-precise numeric type.

**Empty selection placeholder** also reflects the active mode: `--`, `0x--`, or `0b--`.

**Parse error indicator** carries the same prefix: a syntax error at column N is shown as `^[N]` (decimal), `0x^[N]` (hex), or `0b^[N]` (binary).

#### Status-bar gestures on the TinyExpr field

| Gesture | Action |
|---------|--------|
| **Single left-click** | Copies the currently displayed result (in the active mode) to the clipboard. The copy is debounced by the system double-click interval, so a follow-up double-click cancels the copy. |
| **Double-click** | Cycles output mode: *Decimal → Hexadecimal → Binary → Decimal …* The status field refreshes immediately. |
| **Right-click** | Standard status-bar context menu (unchanged). |

#### Binary input

Expressions may also **use** binary literals `0b…` (or `0B…`), in addition to the always-supported decimal, scientific, and hex (`0x…`) literals. They round-trip the binary output mode:

```
0b101010=?            → 42
0xFF + 0b1=?          → 256
BITAND(0b1100, 0b1010)=?  → 8
```

Binary literals are recognized only at token-start positions, so identifiers like `var0b1` are not affected.

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
| Binary | `0b101010` | 42 |
| Scientific | `1e3` | 1000 |
| Negative scientific | `2.5e-2` | 0.025 |

> Binary literals (`0b…` / `0B…`) are a Notepad3 extension on top of TinyExpr++ — they are rewritten to decimal before the parser sees them.

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

> **Important — a single `=` inside an expression means equality, not assignment.**
> TinyExpr++ has no assignment operator, so within an expression a lone `=` is
> treated exactly like `==`. That means:
> ```
> 1+1=2+2    → 0     (parses as (1+1) == (2+2), i.e. 2 == 4 → false)
> 1+1=2      → 1     (parses as (1+1) == 2,     i.e. 2 == 2 → true)
> ```
> Use `==` explicitly when you want the equality intent to be obvious, and
> parenthesize freely. Note the distinction from the inline-evaluation
> *suffix* `=?` and the "type `=` then press **Enter**" trigger described
> above — those are UI markers that ask Notepad3 to evaluate the expression,
> not part of the expression itself. A *leading* `=` at the very start is
> stripped by the parser to tolerate spreadsheet-style input like
> `=SUM(1,2)`; equality semantics only apply to a `=` that appears between
> two operands.

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
72 * 0.0254=?             → 1.8288             (72 inches to meters)
100 / 2.54=?              → 39.3700787401575   (100 cm to inches)
(98.6 - 32) * 5/9=?      → 37                  (Fahrenheit to Celsius)
```

### Programming Helpers

```
0xFF=?                    → 255
0b101010=?                → 42
2^16 - 1=?               → 65535
BITLSHIFT(1, 20)=?       → 1048576  (1 MB in bytes)
BITAND(0xF0, 0x3C)=?     → 48
BITAND(0b1100, 0b1010)=? → 8
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
| **Inline evaluation** | `1/3=?` → `0.333333333333333` | `1/3=?` → `0,333333333333333` |

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

## C API: Boolean-Aware Evaluation (developer reference)

The C wrapper around TinyExpr++ exposes an evaluator that renders results
of boolean-looking expressions as the words `true` / `false`, so callers
don't have to invent their own classification scheme:

```c
#include "tinyexpr_cif.h"

const char *te_interp_str(const char *expression, te_int_t *error);
```

The function evaluates the expression once and returns a thread-local
internal buffer. The returned pointer remains valid until the next call
from the same thread; `*error` follows the same convention as `te_interp()`
(`0` on success, 1-based parse-error position on failure).

| Returned string | When |
|-----------------|------|
| `"true"` / `"false"` | Expression is lexically logical **and** evaluates to exactly `1.0` / `0.0`. |
| `"nan"`, `"inf"`, `"-inf"` | Result is non-finite (e.g., `0/0`, `LN(0)`, comparison involving `NaN`). |
| Integer-like (`%.21g`) | Finite value whose fractional part is below `1e-15` and whose magnitude is below `1e21`. |
| Decimal (`%.15g`) | All other finite results. |

The numeric formatting mirrors Notepad3's `TinyExprToStringA` exactly, so values returned by `te_interp_str()` match what the status bar / `=?` inline-replacement would render. Hex / binary output modes are UI-level concerns and remain in `TinyExprToStringA` proper.

### When is an expression classified as logical?

Classification requires **both** of the following to hold:

1. **Lexical hit** at parenthesis depth 0 (one fully-enclosing outer
   pair is stripped first, so `(1==1)` is treated like `1==1`):
   - a relational / equality / logical operator: `==`, `=`, `!=`, `<>`,
     `<=`, `>=`, `<`, `>`, `&&`, `||`, leading `!`
   - the bare keywords `true` / `false`
   - an outermost call to `AND`, `OR`, `NOT`, `ISERR`, `ISERROR`,
     `ISNA`, `ISNAN`, `ISEVEN`, or `ISODD` (case-insensitive)
2. **Value hit**: the finite evaluation result is exactly `0.0` or `1.0`.

Bit-shift (`<<`, `>>`) and bit-rotate (`<<<`, `>>>`) are consumed by the
scanner without triggering classification. Block comments (`/* … */`) and
line comments (`// …`) are skipped, mirroring the parser.

`IF` / `IFS` are intentionally **not** in the predicate list — they return
arbitrary user-supplied values, so `IF(a>b, 5, 10)` is numeric, not
boolean.

### Examples

| Expression | Returns | Reason |
|------------|---------|--------|
| `1+1=2+2` | `"false"` | Parses as `(1+1) == (2+2)`; lone `=` is equality. |
| `1+1=2` | `"true"` | Same path; `2 == 2` is true. |
| `(1==1)` | `"true"` | Outer parens stripped before the lexical scan. |
| `ISEVEN(4)` | `"true"` | Top-level call to a predicate function. |
| `1 && 0` | `"false"` | Logical AND at depth 0. |
| `!0` | `"true"` | Unary logical-NOT. |
| `IF(1>2, 100, 200)` | `"200"` | `IF` is not a predicate; `>` is inside parens. |
| `1 + (1==1)` | `"2"` | `==` is inside parens, and the result isn't 0/1. |
| `(1==1) * (2==2)` | `"1"` | No comparison at depth 0; result is arithmetic. |
| `0/0 == 1` | `"nan"` | Non-finite result bypasses classification. |
| `2*PI` | `"6.28318530717959"` | No logical operator; `%.15g` format. |

> **Why both checks?** A purely value-based test would mis-label `1+0` as
> a boolean. A purely lexical test would mis-label `IF(a>b, 5, 10)`
> (which evaluates to `5` or `10`, not `0`/`1`). The intersection is much
> closer to user intent. `te_interp()` and `te_compile()` remain
> unchanged for callers that prefer to handle classification themselves.

---

## Notes

- All function names are **case-insensitive**.
- `LOG(x)` is provided for backward compatibility and evaluates as `LOG10(x)`.
- The `%` operator computes the **modulus** (remainder), not a percentage.
- Expressions support **C/C++ style comments** (`/* … */` and `// …`).
- Invalid operations (division by zero, `SQRT(-1)`) return **NaN**.

For the full TinyExpr++ reference, see the [TinyExpr++ documentation](https://blake-madden.github.io/tinyexpr-plusplus/).
