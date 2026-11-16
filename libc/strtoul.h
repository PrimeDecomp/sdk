#ifndef _STRTOUL_H_
#define _STRTOUL_H_

#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

unsigned long __strtoul(int base, int max_width, int (*read_proc)(void*, int, int),
                       void* read_arg, int* chars_scanned, int* negative, int* overflow);
unsigned long long __strtoull(int base, int max_width, int (*read_proc)(void*, int, int),
                             void* read_arg, int* chars_scanned, int* negative, int* overflow);
unsigned long __wcstoul(int base, int max_width, wint_t (*read_proc)(void*, wint_t, int),
                       void* read_arg, int* chars_scanned, int* negative, int* overflow);

#ifdef __cplusplus
}
#endif

#endif
