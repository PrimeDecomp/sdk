#ifndef _STDINT_H_
#define _STDINT_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long int uintptr_t;
typedef long int intptr_t;

#define UINT32_MAX 0xffffffffUL
#define UINTPTR_MAX 0xffffffffUL
#define WCHAR_MAX 0xffffU

#ifdef __cplusplus
}
#endif

#endif
