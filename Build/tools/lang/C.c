// C99 http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1256.pdf
// C11 http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf
// https://en.cppreference.com/w/c
// https://en.cppreference.com/w/c/header

_Alignas alignas
_Alignof alignof
_Atomic
_Generic
_Noreturn noreturn
_Static_assert static_assert
_Thread_local thread_local
break
case const continue
default do
else enum extern
for
goto
if
inline
register restrict return
sizeof static struct switch
typedef
union
void volatile
while

_Bool bool _Complex complex _Imaginary imaginary
auto char double float int long short signed unsigned

asm
fortran
// Predefined identifiers
__func__
// Preprocessing directives
#if
#elif
#else
#endif
defined
#ifdef
#endif
#ifndef
#endif
#define
#undef
#include
#line
#error
#pragma
_Pragma()

// Visual C++ Predefined Macros
// https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
// GCC Predefined Macros
// https://gcc.gnu.org/onlinedocs/cpp/Predefined-Macros.html
// echo | clang -dM -E -
// echo | gcc -dM -E -
// https://software.intel.com/en-us/cpp-compiler-developer-guide-and-reference-additional-predefined-macros
// https://sourceforge.net/p/predef/wiki/Compilers/

#include <assert.h>
{	// Diagnostics
	NDEBUG
	void assert(scalar expression);
	static_assert()			// C11
	_Static_assert()		// C11
}

#include <complex.h>
{	// Complex arithmetic
	complex
	_Complex
	_Complex_I
	imaginary
	_Imaginary_I
	_Imaginary
	#pragma STDC CX_LIMITED_RANGE on-off-switch

	// Trigonometric functions
	double complex cacos(double complex z);
	float complex cacosf(float complex z);
	long double complex cacosl(long double complex z);
	double complex casin(double complex z);
	float complex casinf(float complex z);
	long double complex casinl(long double complex z);
	double complex catan(double complex z);
	float complex catanf(float complex z);
	long double complex catanl(long double complex z);
	double complex ccos(double complex z);
	float complex ccosf(float complex z);
	long double complex ccosl(long double complex z);
	double complex csin(double complex z);
	float complex csinf(float complex z);
	long double complex csinl(long double complex z);
	double complex ctan(double complex z);
	float complex ctanf(float complex z);
	long double complex ctanl(long double complex z);
	// Hyperbolic functions
	double complex cacosh(double complex z);
	float complex cacoshf(float complex z);
	long double complex cacoshl(long double complex z);
	double complex casinh(double complex z);
	float complex casinhf(float complex z);
	long double complex casinhl(long double complex z);
	double complex catanh(double complex z);
	float complex catanhf(float complex z);
	long double complex catanhl(long double complex z);
	double complex ccosh(double complex z);
	float complex ccoshf(float complex z);
	long double complex ccoshl(long double complex z);
	double complex csinh(double complex z);
	float complex csinhf(float complex z);
	long double complex csinhl(long double complex z);
	double complex ctanh(double complex z);
	float complex ctanhf(float complex z);
	long double complex ctanhl(long double complex z);
	// Exponential and logarithmic functions
	double complex cexp(double complex z);
	float complex cexpf(float complex z);
	long double complex cexpl(long double complex z);
	double complex clog(double complex z);
	float complex clogf(float complex z);
	long double complex clogl(long double complex z);
	// Power and absolute-value functions
	double cabs(double complex z);
	float cabsf(float complex z);
	long double cabsl(long double complex z);
	double complex cpow(double complex x, double complex y);
	float complex cpowf(float complex x, float complex y);
	long double complex cpowl(long double complex x, long double complex y);
	double complex csqrt(double complex z);
	float complex csqrtf(float complex z);
	long double complex csqrtl(long double complex z);
	// Manipulation functions
	double carg(double complex z);
	float cargf(float complex z);
	long double cargl(long double complex z);
	double cimag(double complex z);
	float cimagf(float complex z);
	long double cimagl(long double complex z);
	double complex CMPLX(double x, double y);
	float complex CMPLXF(float x, float y);
	long double complex CMPLXL(long double x, long double y);
	double complex conj(double complex z);
	float complex conjf(float complex z);
	long double complex conjl(long double complex z);
	double complex cproj(double complex z);
	float complex cprojf(float complex z);
	long double complex cprojl(long double complex z);
	double creal(double complex z);
	float crealf(float complex z);
	long double creall(long double complex z);
}

#include <ctype.h>
{	// Character handling
	// Character classification functions
	int isalnum(int c);
	int isalpha(int c);
	int isblank(int c);
	int iscntrl(int c);
	int isdigit(int c);
	int isgraph(int c);
	int islower(int c);
	int isprint(int c);
	int ispunct(int c);
	int isspace(int c);
	int isupper(int c);
	int isxdigit(int c);
	// Character case mapping functions
	int tolower(int c);
	int toupper(int c);
}

#include <errno.h>
{	// Errors
	EDOM
	EILSEQ
	ERANGE
	errno
}

#include <fenv.h>
{	// Floating-point environment
	fenv_t
	fexcept_t
	FE_DIVBYZERO
	FE_INEXACT
	FE_INVALID
	FE_OVERFLOW
	FE_UNDERFLOW
	FE_ALL_EXCEPT
	FE_DOWNWARD
	FE_TONEAREST
	FE_TOWARDZERO
	FE_UPWARD
	FE_DFL_ENV
	#pragma STDC FENV_ACCESS on-off-switch

	// Floating-point exceptions
	int feclearexcept(int excepts);
	int fegetexceptflag(fexcept_t *flagp, int excepts);
	int feraiseexcept(int excepts);
	int fesetexceptflag(const fexcept_t *flagp, int excepts);
	int fetestexcept(int excepts);
	// Rounding
	int fegetround(void);
	int fesetround(int round);
	// Environment
	int fegetenv(fenv_t *envp);
	int feholdexcept(fenv_t *envp);
	int fesetenv(const fenv_t *envp);
	int feupdateenv(const fenv_t *envp);
}

#include <float.h>
{	// Characteristics of floating types
	FLT_ROUNDS
	FLT_EVAL_METHOD
	FLT_HAS_SUBNORM
	DBL_HAS_SUBNORM
	LDBL_HAS_SUBNORM
	FLT_RADIX
	FLT_MANT_DIG
	DBL_MANT_DIG
	LDBL_MANT_DIG
	FLT_DECIMAL_DIG
	DBL_DECIMAL_DIG
	LDBL_DECIMAL_DIG
	DECIMAL_DIG
	FLT_DIG
	DBL_DIG
	LDBL_DIG
	FLT_MIN_EXP
	DBL_MIN_EXP
	LDBL_MIN_EXP
	FLT_MIN_10_EXP
	DBL_MIN_10_EXP
	LDBL_MIN_10_EXP
	FLT_MAX_EXP
	DBL_MAX_EXP
	LDBL_MAX_EXP
	FLT_MAX_10_EXP
	DBL_MAX_10_EXP
	LDBL_MAX_10_EXP
	FLT_MAX
	DBL_MAX
	LDBL_MAX
	FLT_EPSILON
	DBL_EPSILON
	LDBL_EPSILON
	FLT_MIN
	DBL_MIN
	LDBL_MIN
	FLT_TRUE_MIN
	DBL_TRUE_MIN
	LDBL_TRUE_MIN
}

#include <inttypes.h>
{	// Format conversion of integer types
	imaxdiv_t
	// Macros for format specifiers
	// The fprintf macros for signed integers
	PRId8
	PRId16
	PRId32
	PRId64
	PRIdLEAST8
	PRIdLEAST16
	PRIdLEAST32
	PRIdLEAST64
	PRIdFAST8
	PRIdFAST16
	PRIdFAST32
	PRIdFAST64
	PRIdMAX
	PRIdPTR
	PRIi8
	PRIi16
	PRIi32
	PRIi64
	PRIiLEAST8
	PRIiLEAST16
	PRIiLEAST32
	PRIiLEAST64
	PRIiFAST8
	PRIiFAST16
	PRIiFAST32
	PRIiFAST64
	PRIiMAX
	PRIiPTR
	// The fprintf macros for unsigned integers
	PRIo8
	PRIo16
	PRIo32
	PRIo64
	PRIoLEAST8
	PRIoLEAST16
	PRIoLEAST32
	PRIoLEAST64
	PRIoFAST8
	PRIoFAST16
	PRIoFAST32
	PRIoFAST64
	PRIoMAX
	PRIoPTR
	PRIu8
	PRIu16
	PRIu32
	PRIu64
	PRIuLEAST8
	PRIuLEAST16
	PRIuLEAST32
	PRIuLEAST64
	PRIuFAST8
	PRIuFAST16
	PRIuFAST32
	PRIuFAST64
	PRIuMAX
	PRIuPTR
	PRIx8
	PRIx16
	PRIx32
	PRIx64
	PRIxLEAST8
	PRIxLEAST16
	PRIxLEAST32
	PRIxLEAST64
	PRIxFAST8
	PRIxFAST16
	PRIxFAST32
	PRIxFAST64
	PRIxMAX
	PRIxPTR
	PRIX8
	PRIX16
	PRIX32
	PRIX64
	PRIXLEAST8
	PRIXLEAST16
	PRIXLEAST32
	PRIXLEAST64
	PRIXFAST8
	PRIXFAST16
	PRIXFAST32
	PRIXFAST64
	PRIXMAX
	PRIXPTR
	// The fscanf macros for signed integers
	SCNd8
	SCNd16
	SCNd32
	SCNd64
	SCNdLEAST8
	SCNdLEAST16
	SCNdLEAST32
	SCNdLEAST64
	SCNdFAST8
	SCNdFAST16
	SCNdFAST32
	SCNdFAST64
	SCNdMAX
	SCNdPTR
	SCNi8
	SCNi16
	SCNi32
	SCNi64
	SCNiLEAST8
	SCNiLEAST16
	SCNiLEAST32
	SCNiLEAST64
	SCNiFAST8
	SCNiFAST16
	SCNiFAST32
	SCNiFAST64
	SCNiMAX
	SCNiPTR
	// The fscanf macros for unsigned integers
	SCNo8
	SCNo16
	SCNo32
	SCNo64
	SCNoLEAST8
	SCNoLEAST16
	SCNoLEAST32
	SCNoLEAST64
	SCNoFAST8
	SCNoFAST16
	SCNoFAST32
	SCNoFAST64
	SCNoMAX
	SCNoPTR
	SCNu8
	SCNu16
	SCNu32
	SCNu64
	SCNuLEAST8
	SCNuLEAST16
	SCNuLEAST32
	SCNuLEAST64
	SCNuFAST8
	SCNuFAST16
	SCNuFAST32
	SCNuFAST64
	SCNuMAX
	SCNuPTR
	SCNx8
	SCNx16
	SCNx32
	SCNx64
	SCNxLEAST8
	SCNxLEAST16
	SCNxLEAST32
	SCNxLEAST64
	SCNxFAST8
	SCNxFAST16
	SCNxFAST32
	SCNxFAST64
	SCNxMAX
	SCNxPTR
	// Functions for greatest-width integer types
	intmax_t imaxabs(intmax_t j);
	imaxdiv_t imaxdiv(intmax_t numer, intmax_t denom);
	intmax_t strtoimax(const char * restrict nptr, char ** restrict endptr, int base);
	uintmax_t strtoumax(const char * restrict nptr, char ** restrict endptr, int base);
	intmax_t wcstoimax(const wchar_t * restrict nptr, wchar_t ** restrict endptr, int base);
	uintmax_t wcstoumax(const wchar_t * restrict nptr, wchar_t ** restrict endptr, int base);
}

#include <iso646.h>
{	// Alternative spellings
	and			&&
	and_eq		&=
	bitand		&
	bitor		|
	compl		~
	not			!
	not_eq		!=
	or			||
	or_eq		|=
	xor			^
	xor_eq		^=
}

#include <limits.h>
{	// Sizes of integer types
	CHAR_BIT
	SCHAR_MIN
	SCHAR_MAX
	UCHAR_MAX
	CHAR_MIN
	CHAR_MAX
	MB_LEN_MAX
	SHRT_MIN
	SHRT_MAX
	USHRT_MAX
	INT_MIN
	INT_MAX
	UINT_MAX
	LONG_MIN
	LONG_MAX
	ULONG_MAX
	LLONG_MIN
	LLONG_MAX
	ULLONG_MAX
}

#include <locale.h>
{	// Localization
	struct lconv;
	NULL
	LC_ALL
	LC_COLLATE
	LC_CTYPE
	LC_MONETARY
	LC_NUMERIC
	LC_TIME

	char *setlocale(int category, const char *locale);
	struct lconv *localeconv(void);
}

#include <math.h>
{	// Mathematics
	float_t
	double_t
	HUGE_VAL
	HUGE_VALF
	HUGE_VALL
	INFINITY
	NAN
	FP_INFINITE
	FP_NAN
	FP_NORMAL
	FP_SUBNORMAL
	FP_ZERO
	FP_FAST_FMA
	FP_FAST_FMAF
	FP_FAST_FMAL
	FP_FAST_FMAF
	FP_FAST_FMAL
	FP_ILOGB0
	FP_ILOGBNAN
	MATH_ERRNO
	MATH_ERREXCEPT
	math_errhandling
	#pragma STDC FP_CONTRACT on-off-switch

	// Classification macros
	int fpclassify(real-floating x);
	int isfinite(real-floating x);
	int isinf(real-floating x);
	int isnan(real-floating x);
	int isnormal(real-floating x);
	int signbit(real-floating x);
	// Trigonometric functions
	double acos(double x);
	float acosf(float x);
	long double acosl(long double x);
	double asin(double x);
	float asinf(float x);
	long double asinl(long double x);
	double atan(double x);
	float atanf(float x);
	long double atanl(long double x);
	double atan2(double y, double x);
	float atan2f(float y, float x);
	long double atan2l(long double y, long double x);
	double cos(double x);
	float cosf(float x);
	long double cosl(long double x);
	double sin(double x);
	float sinf(float x);
	long double sinl(long double x);
	double tan(double x);
	float tanf(float x);
	long double tanl(long double x);
	// Hyperbolic functions
	double acosh(double x);
	float acoshf(float x);
	long double acoshl(long double x);
	double asinh(double x);
	float asinhf(float x);
	long double asinhl(long double x);
	double atanh(double x);
	float atanhf(float x);
	long double atanhl(long double x);
	double cosh(double x);
	float coshf(float x);
	long double coshl(long double x);
	double sinh(double x);
	float sinhf(float x);
	long double sinhl(long double x);
	double tanh(double x);
	float tanhf(float x);
	long double tanhl(long double x);
	// Exponential and logarithmic functions
	double exp(double x);
	float expf(float x);
	long double expl(long double x);
	double exp2(double x);
	float exp2f(float x);
	long double exp2l(long double x);
	double expm1(double x);
	float expm1f(float x);
	long double expm1l(long double x);
	double frexp(double value, int *exp);
	float frexpf(float value, int *exp);
	long double frexpl(long double value, int *exp);
	int ilogb(double x);
	int ilogbf(float x);
	int ilogbl(long double x);
	double ldexp(double x, int exp);
	float ldexpf(float x, int exp);
	long double ldexpl(long double x, int exp);
	double log(double x);
	float logf(float x);
	long double logl(long double x);
	double log10(double x);
	float log10f(float x);
	long double log10l(long double x);
	double log1p(double x);
	float log1pf(float x);
	long double log1pl(long double x);
	double log2(double x);
	float log2f(float x);
	long double log2l(long double x);
	double logb(double x);
	float logbf(float x);
	long double logbl(long double x);
	double modf(double value, double *iptr);
	float modff(float value, float *iptr);
	long double modfl(long double value, long double *iptr);
	double scalbn(double x, int n);
	float scalbnf(float x, int n);
	long double scalbnl(long double x, int n);
	double scalbln(double x, long int n);
	float scalblnf(float x, long int n);
	long double scalblnl(long double x, long int n);
	// Power and absolute-value functions
	double cbrt(double x);
	float cbrtf(float x);
	long double cbrtl(long double x);
	double fabs(double x);
	float fabsf(float x);
	long double fabsl(long double x);
	double hypot(double x, double y);
	float hypotf(float x, float y);
	long double hypotl(long double x, long double y);
	double pow(double x, double y);
	float powf(float x, float y);
	long double powl(long double x, long double y);
	double sqrt(double x);
	float sqrtf(float x);
	long double sqrtl(long double x);
	// Error and gamma functions
	double erfc(double x);
	float erfcf(float x);
	long double erfcl(long double x);
	double lgamma(double x);
	float lgammaf(float x);
	long double lgammal(long double x);
	double tgamma(double x);
	float tgammaf(float x);
	long double tgammal(long double x);
	// Nearest integer functions
	double ceil(double x);
	float ceilf(float x);
	long double ceill(long double x);
	double floor(double x);
	float floorf(float x);
	long double floorl(long double x);
	double nearbyint(double x);
	float nearbyintf(float x);
	long double nearbyintl(long double x);
	double rint(double x);
	float rintf(float x);
	long double rintl(long double x);
	long int lrint(double x);
	long int lrintf(float x);
	long int lrintl(long double x);
	long long int llrint(double x);
	long long int llrintf(float x);
	long long int llrintl(long double x);
	double round(double x);
	float roundf(float x);
	long double roundl(long double x);
	long int lround(double x);
	long int lroundf(float x);
	long int lroundl(long double x);
	long long int llround(double x);
	long long int llroundf(float x);
	long long int llroundl(long double x);
	double trunc(double x);
	float truncf(float x);
	long double truncl(long double x);
	// Remainder functions
	double fmod(double x, double y);
	float fmodf(float x, float y);
	long double fmodl(long double x, long double y);
	double remainder(double x, double y);
	float remainderf(float x, float y);
	long double remainderl(long double x, long double y);
	double remquo(double x, double y, int *quo);
	float remquof(float x, float y, int *quo);
	long double remquol(long double x, long double y, int *quo);
	// Manipulation functions
	double copysign(double x, double y);
	float copysignf(float x, float y);
	long double copysignl(long double x, long double y);
	double nan(const char *tagp);
	float nanf(const char *tagp);
	long double nanl(const char *tagp);
	double nextafter(double x, double y);
	float nextafterf(float x, float y);
	long double nextafterl(long double x, long double y);
	double nexttoward(double x, long double y);
	float nexttowardf(float x, long double y);
	long double nexttowardl(long double x, long double y);
	// Maximum, minimum, and positive difference functions
	double fdim(double x, double y);
	float fdimf(float x, float y);
	long double fdiml(long double x, long double y);
	double fmax(double x, double y);
	float fmaxf(float x, float y);
	long double fmaxl(long double x, long double y);
	double fmin(double x, double y);
	float fminf(float x, float y);
	long double fminl(long double x, long double y);
	// Floating multiply-add
	double fma(double x, double y, double z);
	float fmaf(float x, float y, float z);
	long double fmal(long double x, long double y, long double z);
	// Comparison macros
	int isgreater(real-floating x, real-floating y);
	int isgreaterequal(real-floating x, real-floating y);
	int isless(real-floating x, real-floating y);
	int islessequal(real-floating x, real-floating y);
	int islessgreater(real-floating x, real-floating y);
	int isunordered(real-floating x, real-floating y);
}

#include <setjmp.h>
{	// Nonlocal jumps
	jmp_buf

	int setjmp(jmp_buf env);
	_Noreturn void longjmp(jmp_buf env, int val);
}

#include <signal.h>
{	// Signal handling
	sig_atomic_t
	SIG_DFL
	SIG_ERR
	SIG_IGN
	SIGABRT
	SIGFPE
	SIGILL
	SIGINT
	SIGSEGV
	SIGTERM

	void (*signal(int sig, void (*func)(int)))(int);
	int raise(int sig);
}

#include <stdalign.h>
{	// Alignment
	alignas
	_Alignof
}

#include <stdarg.h>
{	// Variable arguments
	va_list

	type va_arg(va_list ap, type);
	void va_copy(va_list dest, va_list src);
	void va_end(va_list ap);
	void va_start(va_list ap, parmN);
}

#include <stdatomic.h>
{	// Atomics
	ATOMIC_BOOL_LOCK_FREE
	ATOMIC_CHAR_LOCK_FREE
	ATOMIC_CHAR16_T_LOCK_FREE
	ATOMIC_CHAR32_T_LOCK_FREE
	ATOMIC_WCHAR_T_LOCK_FREE
	ATOMIC_SHORT_LOCK_FREE
	ATOMIC_INT_LOCK_FREE
	ATOMIC_LONG_LOCK_FREE
	ATOMIC_LLONG_LOCK_FREE
	ATOMIC_POINTER_LOCK_FREE

	ATOMIC_FLAG_INIT
	memory_order
	atomic_flag

	// Initialization
	#define ATOMIC_VAR_INIT(C value)
	void atomic_init(volatile A *obj, C value);
	// Order and consistency
	memory_order_relaxed
	memory_order_consume
	memory_order_acquire
	memory_order_release
	memory_order_acq_rel
	memory_order_seq_cst
	type kill_dependency(type y);
	// Fences
	void atomic_thread_fence(memory_order order);
	void atomic_signal_fence(memory_order order);
	// Lock-free property
	_Bool atomic_is_lock_free(const volatile A *obj);
	// Atomic integer types
	atomic_bool
	atomic_char
	atomic_schar
	atomic_uchar
	atomic_short
	atomic_ushort
	atomic_int
	atomic_uint
	atomic_long
	atomic_ulong
	atomic_llong
	atomic_ullong
	atomic_char16_t
	atomic_char32_t
	atomic_wchar_t
	atomic_int_least8_t
	atomic_uint_least8_t
	atomic_int_least16_t
	atomic_uint_least16_t
	atomic_int_least32_t
	atomic_uint_least32_t
	atomic_int_least64_t
	atomic_uint_least64_t
	atomic_int_fast8_t
	atomic_uint_fast8_t
	atomic_int_fast16_t
	atomic_uint_fast16_t
	atomic_int_fast32_t
	atomic_uint_fast32_t
	atomic_int_fast64_t
	atomic_uint_fast64_t
	atomic_intptr_t
	atomic_uintptr_t
	atomic_size_t
	atomic_ptrdiff_t
	atomic_intmax_t
	atomic_uintmax_t
	// Operations on atomic types
	void atomic_store(volatile A *object, C desired);
	void atomic_store_explicit(volatile A *object, C desired, memory_order order);
	C atomic_load(volatile A *object);
	C atomic_load_explicit(volatile A *object, memory_order order);
	C atomic_exchange(volatile A *object, C desired);
	C atomic_exchange_explicit(volatile A *object, C desired, memory_order order);
	_Bool atomic_compare_exchange_strong(volatile A *object, C *expected, C desired);
	_Bool atomic_compare_exchange_strong_explicit(volatile A *object, C *expected, C desired, memory_order success, memory_order failure);
	_Bool atomic_compare_exchange_weak(volatile A *object, C *expected, C desired);
	_Bool atomic_compare_exchange_weak_explicit(volatile A *object, C *expected, C desired, memory_order success, memory_order failure);
	C atomic_fetch_add(volatile A *object, M operand);
	C atomic_fetch_add_explicit(volatile A *object, M operand, memory_order order);
	C atomic_fetch_sub(volatile A *object, M operand);
	C atomic_fetch_sub_explicit(volatile A *object, M operand, memory_order order);
	C atomic_fetch_or(volatile A *object, M operand);
	C atomic_fetch_or_explicit(volatile A *object, M operand, memory_order order);
	C atomic_fetch_xor(volatile A *object, M operand);
	C atomic_fetch_xor_explicit(volatile A *object, M operand, memory_order order);
	C atomic_fetch_and(volatile A *object, M operand);
	C atomic_fetch_and_explicit(volatile A *object, M operand, memory_order order);
	// Atomic flag type and operations
	atomic_flag guard = ATOMIC_FLAG_INIT;
	_Bool atomic_flag_test_and_set(volatile atomic_flag *object);
	_Bool atomic_flag_test_and_set_explicit(volatile atomic_flag *object, memory_order order);
	void atomic_flag_clear(volatile atomic_flag *object);
	void atomic_flag_clear_explicit(volatile atomic_flag *object, memory_order order);
}

#include <stdbool.h>
{	// Boolean type and values
	bool
	_Bool
	true
	false
}

#include <stddef.h>
{	// Common definitions
	ptrdiff_t
	size_t
	max_align_t
	wchar_t
	NULL
	offsetof(type, member-designator)
}

#include <stdint.h>
{	// Integer types
	int8_t
	int16_t
	int32_t
	int64_t
	uint8_t
	uint16_t
	uint32_t
	uint64_t
	int_least8_t
	int_least16_t
	int_least32_t
	int_least64_t
	uint_least8_t
	uint_least16_t
	uint_least32_t
	uint_least64_t
	int_fast8_t
	int_fast16_t
	int_fast32_t
	int_fast64_t
	uint_fast8_t
	uint_fast16_t
	uint_fast32_t
	uint_fast64_t
	intptr_t
	uintptr_t
	intmax_t
	uintmax_t
	INT8_MIN
	INT16_MIN
	INT32_MIN
	INT64_MIN
	INT8_MAX
	INT16_MAX
	INT32_MAX
	INT64_MAX
	UINT8_MAX
	UINT16_MAX
	UINT32_MAX
	UINT64_MAX
	INT_FAST8_MIN
	INT_FAST16_MIN
	INT_FAST32_MIN
	INT_FAST64_MIN
	INT_FAST8_MAX
	INT_FAST16_MAX
	INT_FAST32_MAX
	INT_FAST64_MAX
	UINT_FAST8_MAX
	UINT_FAST16_MAX
	UINT_FAST32_MAX
	UINT_FAST64_MAX
	INTPTR_MIN
	INTPTR_MAX
	UINTPTR_MAX
	INTMAX_MIN
	INTMAX_MAX
	UINTMAX_MAX
	PTRDIFF_MIN
	PTRDIFF_MAX
	SIG_ATOMIC_MIN
	SIG_ATOMIC_MAX
	SIZE_MAX
	WCHAR_MIN
	WCHAR_MAX
	WINT_MIN
	WINT_MAX
	INT8_C(value)
	INT16_C(value)
	INT32_C(value)
	INT64_C(value)
	UINT8_C(value)
	UINT16_C(value)
	UINT32_C(value)
	UINT64_C(value)
	INTMAX_C(value)
	UINTMAX_C(value)
}

#include <stdio.h>
{	// Input/output
	FILE
	fpos_t

	_IOFBF
	_IOLBF
	_IONBF
	BUFSIZ
	EOF
	FOPEN_MAX
	FILENAME_MAX
	L_tmpnam
	SEEK_CUR
	SEEK_END
	SEEK_SET
	TMP_MAX
	stderr
	stdin
	stdout
	// Operations on files
	int remove(const char *filename);
	int rename(const char *old, const char *new);
	FILE *tmpfile(void);
	char *tmpnam(char *s);
	int fclose(FILE *stream);
	int fflush(FILE *stream);
	FILE *fopen(const char * restrict filename, const char * restrict mode);
	FILE *freopen(const char * restrict filename, const char * restrict mode, FILE * restrict stream);
	void setbuf(FILE * restrict stream, char * restrict buf);
	int setvbuf(FILE * restrict stream, char * restrict buf, int mode, size_t size);
	// Formatted input/output functions
	int fprintf(FILE * restrict stream, const char * restrict format, ...);
	int fscanf(FILE * restrict stream, const char * restrict format, ...);
	int printf(const char * restrict format, ...);
	int scanf(const char * restrict format, ...);
	int snprintf(char * restrict s, size_t n, const char * restrict format, ...);
	int sprintf(char * restrict s, const char * restrict format, ...);
	int sscanf(const char * restrict s, const char * restrict format, ...);
	int vfprintf(FILE * restrict stream, const char * restrict format, va_list arg);
	int vfscanf(FILE * restrict stream, const char * restrict format, va_list arg);
	int vprintf(const char * restrict format, va_list arg);
	int vscanf(const char * restrict format, va_list arg);
	int vsnprintf(char * restrict s, size_t n, const char * restrict format, va_list arg);
	int vsprintf(char * restrict s, const char * restrict format, va_list arg);
	int vsscanf(const char * restrict s, const char * restrict format, va_list arg);
	// Character input/output functions
	int fgetc(FILE *stream);
	char *fgets(char * restrict s, int n, FILE * restrict stream);
	int fputc(int c, FILE *stream);
	int fputs(const char * restrict s, FILE * restrict stream);
	int getc(FILE *stream);
	int getchar(void);
	int putc(int c, FILE *stream);
	int putchar(int c);
	int puts(const char *s);
	int ungetc(int c, FILE *stream);
	// Direct input/output functions
	size_t fread(void * restrict ptr, size_t size, size_t nmemb, FILE * restrict stream);
	size_t fwrite(const void * restrict ptr, size_t size, size_t nmemb, FILE * restrict stream);
	// File positioning functions
	int fgetpos(FILE * restrict stream, fpos_t * restrict pos);
	int fseek(FILE *stream, long int offset, int whence);
	int fsetpos(FILE *stream, const fpos_t *pos);
	long int ftell(FILE *stream);
	void rewind(FILE *stream);
	// Error-handling functions
	void clearerr(FILE *stream);
	int feof(FILE *stream);
	int ferror(FILE *stream);
	void perror(const char *s);
}

#include <stdlib.h>
{	// General utilities
	div_t
	ldiv_t
	lldiv_t
	EXIT_FAILURE
	EXIT_SUCCESS
	RAND_MAX
	MB_CUR_MAX
	// Numeric conversion functions
	double atof(const char *nptr);
	int atoi(const char *nptr);
	long int atol(const char *nptr);
	long long int atoll(const char *nptr);
	double strtod(const char * restrict nptr, char ** restrict endptr);
	float strtof(const char * restrict nptr, char ** restrict endptr);
	long double strtold(const char * restrict nptr, char ** restrict endptr);
	long int strtol(const char * restrict nptr, char ** restrict endptr, int base);
	long long int strtoll(const char * restrict nptr, char ** restrict endptr, int base);
	unsigned long int strtoul(const char * restrict nptr, char ** restrict endptr, int base);
	unsigned long long int strtoull(const char * restrict nptr, char ** restrict endptr, int base);
	// Pseudo-random sequence generation functions
	int rand(void);
	void srand(unsigned int seed);
	// Memory management functions
	void *aligned_alloc(size_t alignment, size_t size)
	void *calloc(size_t nmemb, size_t size);
	void free(void *ptr);
	void *malloc(size_t size);
	void *realloc(void *ptr, size_t size);
	// Communication with the environment
	_Noreturn void abort(void);
	int atexit(void (*func)(void));
	int at_quick_exit(void (*func)(void));
	_Noreturn void exit(int status);
	_Noreturn void _Exit(int status);
	char *getenv(const char *name);
	_Noreturn void quick_exit(int status);
	int system(const char *string);
	// Searching and sorting utilities
	void *bsearch(const void *key, const void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));
	void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));
	// Integer arithmetic functions
	int abs(int j);
	long int labs(long int j);
	long long int llabs(long long int j);
	div_t div(int numer, int denom);
	ldiv_t ldiv(long int numer, long int denom);
	lldiv_t lldiv(long long int numer, long long int denom);
	// Multibyte/wide character conversion functions
	int mblen(const char *s, size_t n);
	int mbtowc(wchar_t * restrict pwc, const char * restrict s, size_t n);
	int wctomb(char *s, wchar_t wc);
	// Multibyte/wide string conversion functions
	size_t mbstowcs(wchar_t * restrict pwcs, const char * restrict s, size_t n);
	size_t wcstombs(char * restrict s, const wchar_t * restrict pwcs, size_t n);
}

#include <stdnoreturn.h>
{	// _Noreturn
	noreturn
	_Noreturn
}

#include <string.h>
{	// String handling
	void *memcpy(void * restrict s1, const void * restrict s2, size_t n);
	void *memmove(void *s1, const void *s2, size_t n);
	char *strcpy(char * restrict s1, const char * restrict s2);
	char *strncpy(char * restrict s1, const char * restrict s2, size_t n);
	// Concatenation functions
	char *strcat(char * restrict s1, const char * restrict s2);
	char *strncat(char * restrict s1, const char * restrict s2, size_t n);
	// Comparison functions
	int memcmp(const void *s1, const void *s2, size_t n);
	int strcmp(const char *s1, const char *s2);
	int strcoll(const char *s1, const char *s2);
	int strncmp(const char *s1, const char *s2, size_t n);
	size_t strxfrm(char * restrict s1, const char * restrict s2, size_t n);
	// Search functions
	void *memchr(const void *s, int c, size_t n);
	char *strchr(const char *s, int c);
	size_t strcspn(const char *s1, const char *s2);
	char *strpbrk(const char *s1, const char *s2);
	char *strrchr(const char *s, int c);
	size_t strspn(const char *s1, const char *s2);
	char *strstr(const char *s1, const char *s2);
	char *strtok(char * restrict s1, const char * restrict s2);
	// Miscellaneous functions
	void *memset(void *s, int c, size_t n);
	char *strerror(int errnum);
	size_t strlen(const char *s);
}

#include <tgmath.h>
// Type-generic math

#include <threads.h>
{	// Threads
	thread_local
	_Thread_local
	ONCE_FLAG_INIT
	TSS_DTOR_ITERATIONS

	cnd_t
	thrd_t
	tss_t
	mtx_t
	tss_dtor_t
	thrd_start_t
	once_flag

	mtx_plain
	mtx_recursive
	mtx_timed
	thrd_timedout
	thrd_success
	thrd_busy
	thrd_error
	thrd_nomem

	// Initialization functions
	void call_once(once_flag *flag, void (*func)(void));
	// Condition variable functions
	int cnd_broadcast(cnd_t *cond);
	void cnd_destroy(cnd_t *cond);
	int cnd_init(cnd_t *cond);
	int cnd_signal(cnd_t *cond);
	int cnd_timedwait(cnd_t *restrict cond, mtx_t *restrict mtx, const struct timespec *restrict ts);
	int cnd_wait(cnd_t *cond, mtx_t *mtx);
	// Mutex functions
	void mtx_destroy(mtx_t *mtx);
	int mtx_init(mtx_t *mtx, int type);
	int mtx_lock(mtx_t *mtx);
	int mtx_timedlock(mtx_t *restrict mtx, const struct timespec *restrict ts);
	int mtx_trylock(mtx_t *mtx);
	int mtx_unlock(mtx_t *mtx);
	// Thread functions
	int thrd_create(thrd_t *thr, thrd_start_t func, void *arg);
	thrd_t thrd_current(void);
	int thrd_detach(thrd_t thr);
	int thrd_equal(thrd_t thr0, thrd_t thr1);
	_Noreturn void thrd_exit(int res);
	int thrd_join(thrd_t thr, int *res);
	int thrd_sleep(const struct timespec *duration, struct timespec *remaining);
	void thrd_yield(void);
	// Thread-specific storage functions
	int tss_create(tss_t *key, tss_dtor_t dtor);
	void tss_delete(tss_t key);
	void *tss_get(tss_t key);
	int tss_set(tss_t key, void *val);
}

#include <time.h>
{	// Date and time
	CLOCKS_PER_SEC
	TIME_UTC
	clock_t
	time_t
	struct timespec;
	struct tm;

	// Time manipulation functions
	clock_t clock(void);
	double difftime(time_t time1, time_t time0);
	time_t mktime(struct tm *timeptr);
	time_t time(time_t *timer);
	int timespec_get(struct timespec *ts, int base);
	// Time conversion functions
	char *asctime(const struct tm *timeptr);
	char *ctime(const time_t *timer);
	struct tm *gmtime(const time_t *timer);
	struct tm *localtime(const time_t *timer);
	size_t strftime(char * restrict s, size_t maxsize, const char * restrict format, const struct tm * restrict timeptr);
}

#include <uchar.h>
{	// Unicode utilities
	char16_t
	char32_t

	size_t mbrtoc16(char16_t * restrict pc16, const char * restrict s, size_t n, mbstate_t * restrict ps);
	size_t c16rtomb(char * restrict s, char16_t c16, mbstate_t * restrict ps);
	size_t mbrtoc32(char32_t * restrict pc32, const char * restrict s, size_t n, mbstate_t * restrict ps);
	size_t c32rtomb(char * restrict s, char32_t c32, mbstate_t * restrict ps);
}

#include <wchar.h>
{	// Extended multibyte and wide character utilities
	mbstate_t
	wint_t
	WCHAR_MIN
	WCHAR_MAX
	WEOF

	// Formatted wide character input/output functions
	int fwprintf(FILE * restrict stream, const wchar_t * restrict format, ...);
	int fwscanf(FILE * restrict stream, const wchar_t * restrict format, ...);
	int swprintf(wchar_t * restrict s, size_t n, const wchar_t * restrict format, ...);
	int swscanf(const wchar_t * restrict s, const wchar_t * restrict format, ...);
	int vfwprintf(FILE * restrict stream, const wchar_t * restrict format, va_list arg);
	int vfwscanf(FILE * restrict stream, const wchar_t * restrict format, va_list arg);
	int vswprintf(wchar_t * restrict s, size_t n, const wchar_t * restrict format, va_list arg);
	int vswscanf(const wchar_t * restrict s, const wchar_t * restrict format, va_list arg);
	int vwprintf(const wchar_t * restrict format, va_list arg);
	int vwscanf(const wchar_t * restrict format, va_list arg);
	int wprintf(const wchar_t * restrict format, ...);
	int wscanf(const wchar_t * restrict format, ...);
	// Wide character input/output functions
	wint_t fgetwc(FILE *stream);
	wchar_t *fgetws(wchar_t * restrict s, int n, FILE * restrict stream);
	wint_t fputwc(wchar_t c, FILE *stream);
	int fputws(const wchar_t * restrict s, FILE * restrict stream);
	int fwide(FILE *stream, int mode);
	wint_t getwc(FILE *stream);
	wint_t getwchar(void);
	wint_t putwc(wchar_t c, FILE *stream);
	wint_t putwchar(wchar_t c);
	wint_t ungetwc(wint_t c, FILE *stream);
	// General wide string utilities
	double wcstod(const wchar_t * restrict nptr, wchar_t ** restrict endptr);
	float wcstof(const wchar_t * restrict nptr, wchar_t ** restrict endptr);
	long double wcstold(const wchar_t * restrict nptr, wchar_t ** restrict endptr);
	long int wcstol(const wchar_t * restrict nptr, wchar_t ** restrict endptr, int base);
	long long int wcstoll(const wchar_t * restrict nptr, wchar_t ** restrict endptr, int base);
	unsigned long int wcstoul(const wchar_t * restrict nptr, wchar_t ** restrict endptr, int base);
	unsigned long long int wcstoull(const wchar_t * restrict nptr, wchar_t ** restrict endptr, int base);
	wchar_t *wcscpy(wchar_t * restrict s1, const wchar_t * restrict s2);
	wchar_t *wcsncpy(wchar_t * restrict s1, const wchar_t * restrict s2, size_t n);
	wchar_t *wmemcpy(wchar_t * restrict s1, const wchar_t * restrict s2, size_t n);
	wchar_t *wmemmove(wchar_t *s1, const wchar_t *s2, size_t n);
	wchar_t *wcscat(wchar_t * restrict s1, const wchar_t * restrict s2);
	wchar_t *wcsncat(wchar_t * restrict s1, const wchar_t * restrict s2, size_t n);
	int wcscmp(const wchar_t *s1, const wchar_t *s2);
	int wcscoll(const wchar_t *s1, const wchar_t *s2);
	int wcsncmp(const wchar_t *s1, const wchar_t *s2, size_t n);
	size_t wcsxfrm(wchar_t * restrict s1, const wchar_t * restrict s2, size_t n);
	int wmemcmp(const wchar_t *s1, const wchar_t *s2, size_t n);
	wchar_t *wcschr(const wchar_t *s, wchar_t c);
	size_t wcscspn(const wchar_t *s1, const wchar_t *s2);
	wchar_t *wcspbrk(const wchar_t *s1, const wchar_t *s2);
	wchar_t *wcsrchr(const wchar_t *s, wchar_t c);
	size_t wcsspn(const wchar_t *s1, const wchar_t *s2);
	wchar_t *wcsstr(const wchar_t *s1, const wchar_t *s2);
	wchar_t *wcstok(wchar_t * restrict s1, const wchar_t * restrict s2, wchar_t ** restrict ptr);
	wchar_t *wmemchr(const wchar_t *s, wchar_t c, size_t n);
	size_t wcslen(const wchar_t *s);
	wchar_t *wmemset(wchar_t *s, wchar_t c, size_t n);
	// Wide character time conversion functions
	size_t wcsftime(wchar_t * restrict s, size_t maxsize, const wchar_t * restrict format, const struct tm * restrict timeptr);
	// Extended multibyte/wide character conversion utilities
	wint_t btowc(int c);
	int wctob(wint_t c);
	int mbsinit(const mbstate_t *ps);
	size_t mbrlen(const char * restrict s, size_t n, mbstate_t * restrict ps);
	size_t mbrtowc(wchar_t * restrict pwc, const char * restrict s, size_t n, mbstate_t * restrict ps);
	size_t wcrtomb(char * restrict s, wchar_t wc, mbstate_t * restrict ps);
	size_t mbsrtowcs(wchar_t * restrict dst, const char ** restrict src, size_t len, mbstate_t * restrict ps);
	size_t wcsrtombs(char * restrict dst, const wchar_t ** restrict src, size_t len, mbstate_t * restrict ps);
}

#include <wctype.h>
{	//Wide character classification functions
	wctrans_t
	wctype_t

	int iswalnum(wint_t wc);
	int iswalpha(wint_t wc);
	int iswblank(wint_t wc);
	int iswcntrl(wint_t wc);
	int iswdigit(wint_t wc);
	int iswgraph(wint_t wc);
	int iswlower(wint_t wc);
	int iswprint(wint_t wc);
	int iswpunct(wint_t wc);
	int iswspace(wint_t wc);
	int iswupper(wint_t wc);
	int iswxdigit(wint_t wc);
	int iswctype(wint_t wc, wctype_t desc);
	wctype_t wctype(const char *property);
	// Wide character case mapping utilities
	wint_t towlower(wint_t wc);
	wint_t towupper(wint_t wc);
	wint_t towctrans(wint_t wc, wctrans_t desc);
	wctrans_t wctrans(const char *property);
}
