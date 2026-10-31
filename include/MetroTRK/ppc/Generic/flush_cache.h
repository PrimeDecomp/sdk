#ifndef PPC_GENERIC_FLUSH_CACHE_H
#define PPC_GENERIC_FLUSH_CACHE_H

#include <types.h>

#ifdef __cplusplus
extern "C" {
#endif

void TRK_flush_cache(void* addr, int size);

#ifdef __cplusplus
}
#endif

#endif /* PPC_GENERIC_FLUSH_CACHE_H */
