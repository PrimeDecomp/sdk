#ifndef _FLOAT_H_
#define _FLOAT_H_

#include <SDKVersion.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int __float_max[];

// The constant on Metroid Prime's Dolphin SDKs (no __float_max) and on the Revolution SDKs; the
// Dolphin SDKs of Echoes and the Corruption prototype read the __float_max global.
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02 || SDK_VERSION >= SDK_VERSION_RVL_2007_06
#define FLT_MAX 3.402823466e+38f
#else
#define FLT_MAX (*(float*)__float_max)
#endif
#define FLT_EPSILON 1.192092896e-07f
#define FLT_MIN 1.175494351e-38f

#define DBL_EPSILON 1.1920929e-07

#ifdef __cplusplus
}
#endif

#define FLT_DIG 6
#define DBL_DIG 6
#define DBL_MIN (*(double*)__double_min)
#define DBL_MAX (*(double*)__double_max)
#define DBL_MANT_DIG 53
#define LDBL_MAX (*(long double*)__extended_max)
#define LDBL_EPSILON (*(long double*)__extended_epsilon)
#define LDBL_MIN (*(long double*)__extended_min)

#endif
