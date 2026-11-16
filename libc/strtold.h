#ifndef _STRTOLD_H_
#define _STRTOLD_H_

#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

long double __strtold(int max_width, int (*read_proc)(void*, int, int), void* read_arg,
                     int* chars_scanned, int* overflow);
// Guessed name
long double __wcstold(int max_width, wint_t (*read_proc)(void*, wint_t, int), void* read_arg,
                     int* chars_scanned, int* overflow);

#ifdef __cplusplus
}
#endif

#endif
