#ifndef _DOLPHIN_OSARENA
#define _DOLPHIN_OSARENA

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

void* OSGetArenaHi(void);
void* OSGetArenaLo(void);
void OSSetArenaHi(void* addr);
void OSSetArenaLo(void* addr);
void* OSAllocFromArenaLo(u32 size, u32 align);

void* OSGetMEM1ArenaHi(void);
void* OSGetMEM2ArenaHi(void);
void* OSGetMEM1ArenaLo(void);
void* OSGetMEM2ArenaLo(void);
void OSSetMEM1ArenaHi(void* addr);
void OSSetMEM2ArenaHi(void* addr);
void OSSetMEM1ArenaLo(void* addr);
void OSSetMEM2ArenaLo(void* addr);
void* OSAllocFromMEM2ArenaLo(u32 size, u32 align);

#ifdef __cplusplus
}
#endif

#endif // _DOLPHIN_OSARENA
