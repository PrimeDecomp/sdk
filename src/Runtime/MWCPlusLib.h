#ifndef _MWCPLUSLIB
#define _MWCPLUSLIB

#ifndef PRIME1
#include <stddef.h>
#endif
#include <types.h>
#ifdef PRIME1
#include <stddef.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
void* __copy(char* to, char* from, size_t size);

#ifdef __cplusplus
}
#endif

#endif // _MWCPLUSLIB
