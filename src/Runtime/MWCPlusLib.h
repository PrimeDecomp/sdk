#ifndef _MWCPLUSLIB
#define _MWCPLUSLIB

#include <SDKVersion.h>

#if SDK_VERSION >= SDK_VERSION_G2ME01_00
#include <stddef.h>
#endif
#include <types.h>
#if SDK_VERSION < SDK_VERSION_G2ME01_00
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
