#ifndef __BUG_FLOAT
#define __BUG_FLOAT

#include </usr/include/float.h>


#if defined(sparc) || defined(__sparc) || defined(__ppc)

/*
#undef	LDBL_MANT_DIG
#undef	LDBL_EPSILON
#undef	LDBL_DIG
#undef	LDBL_MIN_EXP
#undef	LDBL_MIN
#undef	LDBL_MIN_10_EXP
#undef	LDBL_MAX_EXP
#undef	LDBL_MAX
#undef	LDBL_MAX_10_EXP
#define	LDBL_MANT_DIG	53
#define	LDBL_EPSILON	2.2204460492503131E-16
#define	LDBL_DIG		15
#define	LDBL_MIN_EXP	(-1021)
#define	LDBL_MIN		2.2250738585072014E-308
#define	LDBL_MIN_10_EXP	(-307)
#define	LDBL_MAX_EXP	(+1024)
#define	LDBL_MAX		1.7976931348623157E+308
#define	LDBL_MAX_10_EXP	(+308)
*/
#define fabsl(x) fabs((double) x)
#define sinl(x) sin((double) x)
#define cosl(x) cos((double) x)
#define tanl(x) tan((double) x)
#define expl(x) exp((double) x)
#define logl(x) log((double) x)
#define log10l(x) log10((double) x)
#define sqrtl(x) sqrt((double) x)
#define asinl(x) asin((double) x)
#define acosl(x) acos((double) x)
#define atanl(x) atan((double) x)
#define sinhl(x) sinh((double) x)
#define coshl(x) cosh((double) x)
#define tanhl(x) tanh((double) x)
#define powl(x,y) pow((double) x, (double) y)
#define atan2l(x,y) atan2((double) x, (double) y)

#endif

#endif /* __BUG_FLOAT */



