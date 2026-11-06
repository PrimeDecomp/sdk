#include <SDKVersion.h>
#include <dolphin/os/OSArena.h>

#define ROUND(n, a) (((u32)(n) + (a)-1) & ~((a)-1))
#define TRUNC(n, a) (((u32)(n)) & ~((a)-1))

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2004_04
void* __OSArenaHi;
void* __OSArenaLo = (void*)-1;

void* OSGetArenaHi(void) { return __OSArenaHi; }

void* OSGetArenaLo(void) { return __OSArenaLo; }

void OSSetArenaHi(void* addr) { __OSArenaHi = addr; }

void OSSetArenaLo(void* addr) { __OSArenaLo = addr; }
#else
static void* __OSArenaLo = (void*)-1;
static void* s_mem2ArenaLo = (void*)-1;
static void* __OSArenaHi = NULL;
static void* s_mem2ArenaHi = NULL;

void* OSGetMEM1ArenaHi(void) { return __OSArenaHi; }

void* OSGetMEM2ArenaHi(void) { return s_mem2ArenaHi; }

void* OSGetArenaHi(void) { return OSGetMEM1ArenaHi(); }

void* OSGetMEM1ArenaLo(void) { return __OSArenaLo; }

void* OSGetMEM2ArenaLo(void) { return s_mem2ArenaLo; }

void* OSGetArenaLo(void) { return OSGetMEM1ArenaLo(); }

void OSSetMEM1ArenaHi(void* addr) { __OSArenaHi = addr; }

void OSSetMEM2ArenaHi(void* addr) { s_mem2ArenaHi = addr; }

void OSSetArenaHi(void* addr) { OSSetMEM1ArenaHi(addr); }

void OSSetMEM1ArenaLo(void* addr) { __OSArenaLo = addr; }

void OSSetMEM2ArenaLo(void* addr) { s_mem2ArenaLo = addr; }

void OSSetArenaLo(void* addr) { OSSetMEM1ArenaLo(addr); }
#endif

void* OSAllocFromArenaLo(u32 size, u32 align) {
  void* ptr;
  u8* arenaLo;

  ptr = OSGetArenaLo();
  arenaLo = ptr = (void*)ROUND(ptr, align);
  arenaLo += size;
  arenaLo = (u8*)ROUND(arenaLo, align);
  OSSetArenaLo(arenaLo);
  return ptr;
}

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
void* OSAllocFromArenaHi(u32 size, u32 align) {
  void* ptr;
  u8* arenaHi;

  arenaHi = OSGetArenaHi();
  arenaHi = (u8*)TRUNC(arenaHi, align);
  arenaHi -= size;
  arenaHi = ptr = (void*)TRUNC(arenaHi, align);
  OSSetArenaHi(arenaHi);
  return ptr;
}
#elif SDK_VERSION <= SDK_VERSION_DOLPHIN_2004_04

void* OSAllocFromArenaHi(u32 size, u32 align) {
  void* ptr;
  u8* arenaHi;

  arenaHi = OSGetArenaHi();
  arenaHi = (u8*)TRUNC(arenaHi, align);
  arenaHi -= size;
  arenaHi = ptr = (void*)TRUNC(arenaHi, align);
  OSSetArenaHi(arenaHi);
  return ptr;
}
#else
void* OSAllocFromMEM2ArenaLo(u32 size, u32 align) {
  void* ptr;
  u8* arenaLo;

  ptr = OSGetMEM2ArenaLo();
  arenaLo = ptr = (void*)ROUND(ptr, align);
  arenaLo += size;
  arenaLo = (u8*)ROUND(arenaLo, align);
  OSSetMEM2ArenaLo(arenaLo);
  return ptr;
}

#endif
