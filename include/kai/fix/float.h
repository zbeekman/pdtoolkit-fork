/**
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-2001, Intel Corp. All rights reserved.
 **/

#ifndef __KAI_FLOAT_H
#define __KAI_FLOAT_H

#ifndef _crayx1
/* Section 2.2.4.2.2 of ANSI X3.159-1989 */
#define FLT_RADIX 2
#define FLT_MANT_DIG 24
#define FLT_EPSILON 1.19209290E-07F
#define FLT_DIG 6
#define FLT_MIN_EXP (-125)
#define FLT_MIN 1.17549435E-38F
#define FLT_MAX 3.40282347E+38F
#define FLT_MIN_10_EXP (-37)
#define FLT_MAX_EXP 128
#define FLT_MAX_10_EXP 38
#define FLT_ROUNDS 1

#ifdef __APPLE__
/*#ifdef __i386__ */
#define __FLT_MIN__ FLT_MIN
#define __DBL_MIN__ DBL_MIN
#define __LDBL_MIN__ LDBL_MIN
/* #endif */ /* i386 */
#endif /* APPLE */

#define DBL_MANT_DIG 53
#define DBL_EPSILON 2.2204460492503131E-16
#define DBL_DIG 15
#define DBL_MIN_EXP (-1021)
#define DBL_MIN 2.2250738585072014E-308
#define DBL_MIN_10_EXP (-307)
#define DBL_MAX_EXP 1024
#define DBL_MAX 1.7976931348623157e+308
#define DBL_MAX_10_EXP 308

/* some platforms do not support long doubles */
/*
#define LDBL_MANT_DIG 64
#define LDBL_EPSILON 1.0842e-19L
#define LDBL_DIG 18
#define LDBL_MIN_EXP (-16381)
#define LDBL_MIN 3.362103143112093506262677817322E-4932L
#define LDBL_MIN_10_EXP (-4931)
#define LDBL_MAX_EXP 16384
#define LDBL_MAX 1.189731495357231765021263853031E+4932L
#define LDBL_MAX_10_EXP 4932
*/
#define LDBL_MANT_DIG DBL_MANT_DIG
#define LDBL_EPSILON DBL_EPSILON
#define LDBL_DIG DBL_DIG
#define LDBL_MIN_EXP DBL_MIN_EXP
#define LDBL_MIN DBL_MIN
#define LDBL_MIN_10_EXP DBL_MIN_10_EXP
#define LDBL_MAX_EXP DBL_MAX_EXP
#define LDBL_MAX DBL_MAX
#define LDBL_MAX_10_EXP DBL_MAX_10_EXP

#ifdef _CRAYT3E
#include "/usr/include/float.h"
#endif /* _CRAYT3E */

#else /* _crayx1 */

#include "/usr/include/float.h"
#undef FLT_MIN
#define FLT_MIN 1E-5
#endif /* _crayx1 */

#endif /* __KAI_FLOAT_H */
