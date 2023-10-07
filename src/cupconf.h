/*
** $Id: cupconf.h $
** Configuration file for Cup
** See Copyright Notice in cup.h
*/


#ifndef cupconf_h
#define cupconf_h

#include <limits.h>
#include <stddef.h>


/*
** ===================================================================
** General Configuration File for Cup
**
** Some definitions here can be changed externally, through the compiler
** (e.g., with '-D' options): They are commented out or protected
** by '#if !defined' guards. However, several other definitions
** should be changed directly here, either because they affect the
** Cup ABI (by making the changes here, you ensure that all software
** connected to Cup, such as C libraries, will be compiled with the same
** configuration); or because they are seldom changed.
**
** Search for "@@" to find all configurable definitions.
** ===================================================================
*/


/*
** {====================================================================
** System Configuration: macros to adapt (if needed) Cup to some
** particular platform, for instance restricting it to C89.
** =====================================================================
*/

/*
@@ CUP_USE_C89 controls the use of non-ISO-C89 features.
** Define it if you want Cup to avoid the use of a few C99 features
** or Windows-specific features on Windows.
*/
/* #define CUP_USE_C89 */


/*
** By default, Cup on Windows use (some) specific Windows features
*/
#if !defined(CUP_USE_C89) && defined(_WIN32) && !defined(_WIN32_WCE)
#define CUP_USE_WINDOWS  /* enable Cupodies for regular Windows */
#endif


#if defined(CUP_USE_WINDOWS)
#define CUP_DL_DLL	/* enable support for DLL */
#define CUP_USE_C89	/* broadly, Windows is C89 */
#endif


#if defined(CUP_USE_LINUX)
#define CUP_USE_POSIX
#define CUP_USE_DLOPEN		/* needs an extra library: -ldl */
#endif


#if defined(CUP_USE_MACOSX)
#define CUP_USE_POSIX
#define CUP_USE_DLOPEN		/* MacOS does not need -ldl */
#endif


/*
@@ CUPI_IS32INT is true iff 'int' has (at least) 32 bits.
*/
#define CUPI_IS32INT	((UINT_MAX >> 30) >= 3)

/* }================================================================== */



/*
** {==================================================================
** Configuration for Number types. These options should not be
** set externally, because any other code connected to Cup must
** use the same configuration.
** ===================================================================
*/

/*
@@ CUP_INT_TYPE defines the type for Cup integers.
@@ CUP_FLOAT_TYPE defines the type for Cup floats.
** Cup should work fine with any mix of these options supported
** by your C compiler. The usual configurations are 64-bit integers
** and 'double' (the default), 32-bit integers and 'float' (for
** restricted platforms), and 'long'/'double' (for C compilers not
** compliant with C99, which may not have support for 'long long').
*/

/* predefined options for CUP_INT_TYPE */
#define CUP_INT_INT		1
#define CUP_INT_LONG		2
#define CUP_INT_LONGLONG	3

/* predefined options for CUP_FLOAT_TYPE */
#define CUP_FLOAT_FLOAT		1
#define CUP_FLOAT_DOUBLE	2
#define CUP_FLOAT_LONGDOUBLE	3


/* Default configuration ('long long' and 'double', for 64-bit Cup) */
#define CUP_INT_DEFAULT		CUP_INT_LONGLONG
#define CUP_FLOAT_DEFAULT	CUP_FLOAT_DOUBLE


/*
@@ CUP_32BITS enables Cup with 32-bit integers and 32-bit floats.
*/
#define CUP_32BITS	0


/*
@@ CUP_C89_NUMBERS ensures that Cup uses the largest types available for
** C89 ('long' and 'double'); Windows always has '__int64', so it does
** not need to use this case.
*/
#if defined(CUP_USE_C89) && !defined(CUP_USE_WINDOWS)
#define CUP_C89_NUMBERS		1
#else
#define CUP_C89_NUMBERS		0
#endif


#if CUP_32BITS		/* { */
/*
** 32-bit integers and 'float'
*/
#if CUPI_IS32INT  /* use 'int' if big enough */
#define CUP_INT_TYPE	CUP_INT_INT
#else  /* otherwise use 'long' */
#define CUP_INT_TYPE	CUP_INT_LONG
#endif
#define CUP_FLOAT_TYPE	CUP_FLOAT_FLOAT

#elif CUP_C89_NUMBERS	/* }{ */
/*
** largest types available for C89 ('long' and 'double')
*/
#define CUP_INT_TYPE	CUP_INT_LONG
#define CUP_FLOAT_TYPE	CUP_FLOAT_DOUBLE

#else		/* }{ */
/* use defaults */

#define CUP_INT_TYPE	CUP_INT_DEFAULT
#define CUP_FLOAT_TYPE	CUP_FLOAT_DEFAULT

#endif				/* } */


/* }================================================================== */



/*
** {==================================================================
** Configuration for Paths.
** ===================================================================
*/

/*
** CUP_PATH_SEP is the character that separates templates in a path.
** CUP_PATH_MARK is the string that marks the substitution points in a
** template.
** CUP_EXEC_DIR in a Windows path is replaced by the executable's
** directory.
*/
#define CUP_PATH_SEP            ";"
#define CUP_PATH_MARK           "?"
#define CUP_EXEC_DIR            "!"


/*
@@ CUP_PATH_DEFAULT is the default path that Cup uses to look for
** Cup libraries.
@@ CUP_CPATH_DEFAULT is the default path that Cup uses to look for
** C libraries.
** CHANGE them if your machine has a non-conventional directory
** hierarchy or if you want to install your libraries in
** non-conventional directories.
*/

#define CUP_VDIR	CUP_VERSION_MAJOR "." CUP_VERSION_MINOR
#if defined(_WIN32)	/* { */
/*
** In Windows, any exclamation mark ('!') in the path is replaced by the
** path of the directory of the executable file of the current process.
*/
#define CUP_LDIR	"!\\cup\\"
#define CUP_CDIR	"!\\"
#define CUP_SHRDIR	"!\\..\\share\\cup\\" CUP_VDIR "\\"

#if !defined(CUP_PATH_DEFAULT)
#define CUP_PATH_DEFAULT  \
		CUP_LDIR"?.cup;"  CUP_LDIR"?\\init.cup;" \
		CUP_CDIR"?.cup;"  CUP_CDIR"?\\init.cup;" \
		CUP_SHRDIR"?.cup;" CUP_SHRDIR"?\\init.cup;" \
		".\\?.cup;" ".\\?\\init.cup"
#endif

#if !defined(CUP_CPATH_DEFAULT)
#define CUP_CPATH_DEFAULT \
		CUP_CDIR"?.dll;" \
		CUP_CDIR"..\\lib\\cup\\" CUP_VDIR "\\?.dll;" \
		CUP_CDIR"loadall.dll;" ".\\?.dll"
#endif

#else			/* }{ */

#define CUP_ROOT	"/usr/local/"
#define CUP_LDIR	CUP_ROOT "share/cup/" CUP_VDIR "/"
#define CUP_CDIR	CUP_ROOT "lib/cup/" CUP_VDIR "/"

#if !defined(CUP_PATH_DEFAULT)
#define CUP_PATH_DEFAULT  \
		CUP_LDIR"?.cup;"  CUP_LDIR"?/init.cup;" \
		CUP_CDIR"?.cup;"  CUP_CDIR"?/init.cup;" \
		"./?.cup;" "./?/init.cup"
#endif

#if !defined(CUP_CPATH_DEFAULT)
#define CUP_CPATH_DEFAULT \
		CUP_CDIR"?.so;" CUP_CDIR"loadall.so;" "./?.so"
#endif

#endif			/* } */


/*
@@ CUP_DIRSEP is the directory separator (for submodules).
** CHANGE it if your machine does not use "/" as the directory separator
** and is not Windows. (On Windows Cup automatically uses "\".)
*/
#if !defined(CUP_DIRSEP)

#if defined(_WIN32)
#define CUP_DIRSEP	"\\"
#else
#define CUP_DIRSEP	"/"
#endif

#endif

/* }================================================================== */


/*
** {==================================================================
** Marks for exported symbols in the C code
** ===================================================================
*/

/*
@@ CUP_API is a mark for all core API functions.
@@ CUPLIB_API is a mark for all auxiliary library functions.
@@ CUPMOD_API is a mark for all standard library opening functions.
** CHANGE them if you need to define those functions in some special way.
** For instance, if you want to create one Windows DLL with the core and
** the libraries, you may want to use the following definition (define
** CUP_BUILD_AS_DLL to get it).
*/
#if defined(CUP_BUILD_AS_DLL)	/* { */

#if defined(CUP_CORE) || defined(CUP_LIB)	/* { */
#define CUP_API __declspec(dllexport)
#else						/* }{ */
#define CUP_API __declspec(dllimport)
#endif						/* } */

#else				/* }{ */

#define CUP_API		extern

#endif				/* } */


/*
** More often than not the libs Cup together with the core.
*/
#define CUPLIB_API	CUP_API
#define CUPMOD_API	CUP_API


/*
@@ CUPI_FUNC is a mark for all extern functions that are not to be
** exported to outside modules.
@@ CUPI_DDEF and CUPI_DDEC are marks for all extern (const) variables,
** none of which to be exported to outside modules (CUPI_DDEF for
** definitions and CUPI_DDEC for declarations).
** CHANGE them if you need to mark them in some special way. Elf/gcc
** (versions 3.2 and later) mark them as "hidden" to optimize access
** when Cup is compiled as a shared library. Not all elf targets support
** this attribute. Unfortunately, gcc does not offer a way to check
** whether the target offers that support, and those without support
** give a warning about it. To avoid these warnings, change to the
** default definition.
*/
#if defined(__GNUC__) && ((__GNUC__*100 + __GNUC_MINOR__) >= 302) && \
    defined(__ELF__)		/* { */
#define CUPI_FUNC	__attribute__((visibility("internal"))) extern
#else				/* }{ */
#define CUPI_FUNC	extern
#endif				/* } */

#define CUPI_DDEC(dec)	CUPI_FUNC dec
#define CUPI_DDEF	/* empty */

/* }================================================================== */


/*
** {==================================================================
** Compatibility with previous versions
** ===================================================================
*/

/*
@@ CUP_COMPAT_5_3 controls other macros for compatibility with Cup 5.3.
** You can define it to get all options, or change specific options
** to fit your specific needs.
*/
#if defined(CUP_COMPAT_5_3)	/* { */

/*
@@ CUP_COMPAT_MATHLIB controls the presence of several deprecated
** functions in the mathematical library.
** (These functions were already officially removed in 5.3;
** nevertheless they are still available here.)
*/
#define CUP_COMPAT_MATHLIB

/*
@@ CUP_COMPAT_APIINTCASTS controls the presence of macros for
** manipulating other integer types (cup_pushunsigned, cup_tounsigned,
** cupL_checkint, cupL_checklong, etc.)
** (These macros were also officially removed in 5.3, but they are still
** available here.)
*/
#define CUP_COMPAT_APIINTCASTS


/*
@@ CUP_COMPAT_LT_LE controls the emulation of the '__le' metamethod
** using '__lt'.
*/
#define CUP_COMPAT_LT_LE


/*
@@ The following macros supply trivial compatibility for some
** changes in the API. The macros themselves document how to
** change your code to avoid using them.
** (Once more, these macros were officially removed in 5.3, but they are
** still available here.)
*/
#define cup_strlen(L,i)		cup_rawlen(L, (i))

#define cup_objlen(L,i)		cup_rawlen(L, (i))

#define cup_equal(L,idx1,idx2)		cup_compare(L,(idx1),(idx2),CUP_OPEQ)
#define cup_lessthan(L,idx1,idx2)	cup_compare(L,(idx1),(idx2),CUP_OPLT)

#endif				/* } */

/* }================================================================== */



/*
** {==================================================================
** Configuration for Numbers (low-level part).
** Change these definitions if no predefined CUP_FLOAT_* / CUP_INT_*
** satisfy your needs.
** ===================================================================
*/

/*
@@ CUPI_UACNUMBER is the result of a 'default argument promotion'
@@ over a floating number.
@@ l_floatatt(x) corrects float attribute 'x' to the proper float type
** by prefixing it with one of FLT/DBL/LDBL.
@@ CUP_NUMBER_FRMLEN is the length modifier for writing floats.
@@ CUP_NUMBER_FMT is the format for writing floats.
@@ cup_number2str converts a float to a string.
@@ l_mathop allows the addition of an 'l' or 'f' to all math operations.
@@ l_floor takes the floor of a float.
@@ cup_str2number converts a decimal numeral to a number.
*/


/* The following definitions are Cupod for most cases here */

#define l_floor(x)		(l_mathop(floor)(x))

#define cup_number2str(s,sz,n)  \
	l_sprintf((s), sz, CUP_NUMBER_FMT, (CUPI_UACNUMBER)(n))

/*
@@ cup_numbertointeger converts a float number with an integral value
** to an integer, or returns 0 if float is not within the range of
** a cup_Integer.  (The range comparisons are tricky because of
** rounding. The tests here assume a two-complement representation,
** where MININTEGER always has an exact representation as a float;
** MAXINTEGER may not have one, and therefore its conversion to float
** may have an ill-defined value.)
*/
#define cup_numbertointeger(n,p) \
  ((n) >= (CUP_NUMBER)(CUP_MININTEGER) && \
   (n) < -(CUP_NUMBER)(CUP_MININTEGER) && \
      (*(p) = (CUP_INTEGER)(n), 1))


/* now the variable definitions */

#if CUP_FLOAT_TYPE == CUP_FLOAT_FLOAT		/* { single float */

#define CUP_NUMBER	float

#define l_floatatt(n)		(FLT_##n)

#define CUPI_UACNUMBER	double

#define CUP_NUMBER_FRMLEN	""
#define CUP_NUMBER_FMT		"%.7g"

#define l_mathop(op)		op##f

#define cup_str2number(s,p)	strtof((s), (p))


#elif CUP_FLOAT_TYPE == CUP_FLOAT_LONGDOUBLE	/* }{ long double */

#define CUP_NUMBER	long double

#define l_floatatt(n)		(LDBL_##n)

#define CUPI_UACNUMBER	long double

#define CUP_NUMBER_FRMLEN	"L"
#define CUP_NUMBER_FMT		"%.19Lg"

#define l_mathop(op)		op##l

#define cup_str2number(s,p)	strtold((s), (p))

#elif CUP_FLOAT_TYPE == CUP_FLOAT_DOUBLE	/* }{ double */

#define CUP_NUMBER	double

#define l_floatatt(n)		(DBL_##n)

#define CUPI_UACNUMBER	double

#define CUP_NUMBER_FRMLEN	""
#define CUP_NUMBER_FMT		"%.14g"

#define l_mathop(op)		op

#define cup_str2number(s,p)	strtod((s), (p))

#else						/* }{ */

#error "numeric float type not defined"

#endif					/* } */



/*
@@ CUP_UNSIGNED is the unsigned version of CUP_INTEGER.
@@ CUPI_UACINT is the result of a 'default argument promotion'
@@ over a CUP_INTEGER.
@@ CUP_INTEGER_FRMLEN is the length modifier for reading/writing integers.
@@ CUP_INTEGER_FMT is the format for writing integers.
@@ CUP_MAXINTEGER is the maximum value for a CUP_INTEGER.
@@ CUP_MININTEGER is the minimum value for a CUP_INTEGER.
@@ CUP_MAXUNSIGNED is the maximum value for a CUP_UNSIGNED.
@@ cup_integer2str converts an integer to a string.
*/


/* The following definitions are Cupod for most cases here */

#define CUP_INTEGER_FMT		"%" CUP_INTEGER_FRMLEN "d"

#define CUPI_UACINT		CUP_INTEGER

#define cup_integer2str(s,sz,n)  \
	l_sprintf((s), sz, CUP_INTEGER_FMT, (CUPI_UACINT)(n))

/*
** use CUPI_UACINT here to avoid problems with promotions (which
** can turn a comparison between unsigneds into a signed comparison)
*/
#define CUP_UNSIGNED		unsigned CUPI_UACINT


/* now the variable definitions */

#if CUP_INT_TYPE == CUP_INT_INT		/* { int */

#define CUP_INTEGER		int
#define CUP_INTEGER_FRMLEN	""

#define CUP_MAXINTEGER		INT_MAX
#define CUP_MININTEGER		INT_MIN

#define CUP_MAXUNSIGNED		UINT_MAX

#elif CUP_INT_TYPE == CUP_INT_LONG	/* }{ long */

#define CUP_INTEGER		long
#define CUP_INTEGER_FRMLEN	"l"

#define CUP_MAXINTEGER		LONG_MAX
#define CUP_MININTEGER		LONG_MIN

#define CUP_MAXUNSIGNED		ULONG_MAX

#elif CUP_INT_TYPE == CUP_INT_LONGLONG	/* }{ long long */

/* use presence of macro LLONG_MAX as proxy for C99 compliance */
#if defined(LLONG_MAX)		/* { */
/* use ISO C99 stuff */

#define CUP_INTEGER		long long
#define CUP_INTEGER_FRMLEN	"ll"

#define CUP_MAXINTEGER		LLONG_MAX
#define CUP_MININTEGER		LLONG_MIN

#define CUP_MAXUNSIGNED		ULLONG_MAX

#elif defined(CUP_USE_WINDOWS) /* }{ */
/* in Windows, can use specific Windows types */

#define CUP_INTEGER		__int64
#define CUP_INTEGER_FRMLEN	"I64"

#define CUP_MAXINTEGER		_I64_MAX
#define CUP_MININTEGER		_I64_MIN

#define CUP_MAXUNSIGNED		_UI64_MAX

#else				/* }{ */

#error "Compiler does not support 'long long'. Use option '-DCUP_32BITS' \
  or '-DCUP_C89_NUMBERS' (see file 'cupconf.h' for details)"

#endif				/* } */

#else				/* }{ */

#error "numeric integer type not defined"

#endif				/* } */

/* }================================================================== */


/*
** {==================================================================
** Dependencies with C99 and other C details
** ===================================================================
*/

/*
@@ l_sprintf is equivalent to 'snprintf' or 'sprintf' in C89.
** (All uses in Cup have only one format item.)
*/
#if !defined(CUP_USE_C89)
#define l_sprintf(s,sz,f,i)	snprintf(s,sz,f,i)
#else
#define l_sprintf(s,sz,f,i)	((void)(sz), sprintf(s,f,i))
#endif


/*
@@ cup_strx2number converts a hexadecimal numeral to a number.
** In C99, 'strtod' does that conversion. Otherwise, you can
** leave 'cup_strx2number' undefined and Cup will provide its own
** implementation.
*/
#if !defined(CUP_USE_C89)
#define cup_strx2number(s,p)		cup_str2number(s,p)
#endif


/*
@@ cup_pointer2str converts a pointer to a readable string in a
** non-specified way.
*/
#define cup_pointer2str(buff,sz,p)	l_sprintf(buff,sz,"%p",p)


/*
@@ cup_number2strx converts a float to a hexadecimal numeral.
** In C99, 'sprintf' (with format specifiers '%a'/'%A') does that.
** Otherwise, you can leave 'cup_number2strx' undefined and Cup will
** provide its own implementation.
*/
#if !defined(CUP_USE_C89)
#define cup_number2strx(L,b,sz,f,n)  \
	((void)L, l_sprintf(b,sz,f,(CUPI_UACNUMBER)(n)))
#endif


/*
** 'strtof' and 'opf' variants for math functions are not valid in
** C89. Otherwise, the macro 'HUGE_VALF' is a Cupod proxy for testing the
** availability of these variants. ('math.h' is already included in
** all files that use these macros.)
*/
#if defined(CUP_USE_C89) || (defined(HUGE_VAL) && !defined(HUGE_VALF))
#undef l_mathop  /* variants not available */
#undef cup_str2number
#define l_mathop(op)		(cup_Number)op  /* no variant */
#define cup_str2number(s,p)	((cup_Number)strtod((s), (p)))
#endif


/*
@@ CUP_KCONTEXT is the type of the context ('ctx') for continuation
** functions.  It must be a numerical type; Cup will use 'intptr_t' if
** available, otherwise it will use 'ptrdiff_t' (the nearest thing to
** 'intptr_t' in C89)
*/
#define CUP_KCONTEXT	ptrdiff_t

#if !defined(CUP_USE_C89) && defined(__STDC_VERSION__) && \
    __STDC_VERSION__ >= 199901L
#include <stdint.h>
#if defined(INTPTR_MAX)  /* even in C99 this type is optional */
#undef CUP_KCONTEXT
#define CUP_KCONTEXT	intptr_t
#endif
#endif


/*
@@ cup_getlocaledecpoint gets the locale "radix character" (decimal point).
** Change that if you do not want to use C locales. (Code using this
** macro must include the header 'locale.h'.)
*/
#if !defined(cup_getlocaledecpoint)
#define cup_getlocaledecpoint()		(localeconv()->decimal_point[0])
#endif


/*
** macros to improve jump prediction, used mostly for error handling
** and debug facilities. (Some macros in the Cup API use these macros.
** Define CUP_NOBUILTIN if you do not want '__builtin_expect' in your
** code.)
*/
#if !defined(cupi_likely)

#if defined(__GNUC__) && !defined(CUP_NOBUILTIN)
#define cupi_likely(x)		(__builtin_expect(((x) != 0), 1))
#define cupi_unlikely(x)	(__builtin_expect(((x) != 0), 0))
#else
#define cupi_likely(x)		(x)
#define cupi_unlikely(x)	(x)
#endif

#endif


#if defined(CUP_CORE) || defined(CUP_LIB)
/* shorter names for Cup's own use */
#define l_likely(x)	cupi_likely(x)
#define l_unlikely(x)	cupi_unlikely(x)
#endif



/* }================================================================== */


/*
** {==================================================================
** Language Variations
** =====================================================================
*/

/*
@@ CUP_NOCVTN2S/CUP_NOCVTS2N control how Cup performs some
** coercions. Define CUP_NOCVTN2S to turn off automatic coercion from
** numbers to strings. Define CUP_NOCVTS2N to turn off automatic
** coercion from strings to numbers.
*/
/* #define CUP_NOCVTN2S */
/* #define CUP_NOCVTS2N */


/*
@@ CUP_USE_APICHECK turns on several consistency checks on the C API.
** Define it as a help when debugging C code.
*/
#if defined(CUP_USE_APICHECK)
#include <assert.h>
#define cupi_apicheck(l,e)	assert(e)
#endif

/* }================================================================== */


/*
** {==================================================================
** Macros that affect the API and must be stable (that is, must be the
** same when you compile Cup and when you compile code that links to
** Cup).
** =====================================================================
*/

/*
@@ CUPI_MAXSTACK limits the size of the Cup stack.
** CHANGE it if you need a different limit. This limit is arbitrary;
** its only purpose is to stop Cup from consuming unlimited stack
** space (and to reserve some numbers for pseudo-indices).
** (It must fit into max(size_t)/32.)
*/
#if CUPI_IS32INT
#define CUPI_MAXSTACK		1000000
#else
#define CUPI_MAXSTACK		15000
#endif


/*
@@ CUP_EXTRASPACE defines the size of a raw memory area associated with
** a Cup state with very fast access.
** CHANGE it if you need a different size.
*/
#define CUP_EXTRASPACE		(sizeof(void *))


/*
@@ CUP_IDSIZE gives the maximum size for the description of the source
@@ of a function in debug information.
** CHANGE it if you want a different size.
*/
#define CUP_IDSIZE	60


/*
@@ CUPL_BUFFERSIZE is the buffer size used by the lauxlib buffer system.
*/
#define CUPL_BUFFERSIZE   ((int)(16 * sizeof(void*) * sizeof(cup_Number)))


/*
@@ CUPI_MAXALIGN defines fields that, when used in a union, ensure
** maximum alignment for the other items in that union.
*/
#define CUPI_MAXALIGN  cup_Number n; double u; void *s; cup_Integer i; long l

/* }================================================================== */





/* =================================================================== */

/*
** Local configuration. You can use this space to add your redefinitions
** without modifying the main part of the file.
*/





#endif
