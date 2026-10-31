#ifndef _REVOLUTION_TYPES
#define _REVOLUTION_TYPES

// The Wii SDK was built with a pointer-typed NULL in C sources.
#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void*)0)
#endif
#endif

#include <dolphin/types.h>

#define ROUND_UP(x, align) (((x) + (align) - 1) & (-(align)))
#define ROUND_UP_PTR(x, align) ((void*)((((u32)(x)) + (align) - 1) & (~((align) - 1))))
#define FOURCC(c0, c1, c2, c3) ((u32)(((c0) & 0xFF) << 24 | ((c1) & 0xFF) << 16 | ((c2) & 0xFF) << 8 | ((c3) & 0xFF)))

#endif // _REVOLUTION_TYPES
