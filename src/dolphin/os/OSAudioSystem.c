#include "GameVersions.h"
#include "types.h"
#include "dolphin/os.h"
#include "dolphin/hw_regs.h"

#ifdef __cplusplus
extern "C" {
#endif

static u8 DSPInitCode[128] = {
    // clang-format off
    0x02, 0x9F, 0x00, 0x10, 0x02, 0x9F, 0x00, 0x33, 0x02, 0x9F, 0x00, 0x34, 0x02, 0x9F, 0x00, 0x35,
    0x02, 0x9F, 0x00, 0x36, 0x02, 0x9F, 0x00, 0x37, 0x02, 0x9F, 0x00, 0x38, 0x02, 0x9F, 0x00, 0x39,
    0x12, 0x06, 0x12, 0x03, 0x12, 0x04, 0x12, 0x05, 0x00, 0x80, 0x80, 0x00, 0x00, 0x88, 0xFF, 0xFF,
    0x00, 0x84, 0x10, 0x00, 0x00, 0x64, 0x00, 0x1D, 0x02, 0x18, 0x00, 0x00, 0x81, 0x00, 0x1C, 0x1E,
    0x00, 0x44, 0x1B, 0x1E, 0x00, 0x84, 0x08, 0x00, 0x00, 0x64, 0x00, 0x27, 0x19, 0x1E, 0x00, 0x00,
    0x00, 0xDE, 0xFF, 0xFC, 0x02, 0xA0, 0x80, 0x00, 0x02, 0x9C, 0x00, 0x28, 0x16, 0xFC, 0x00, 0x54,
    0x16, 0xFD, 0x43, 0x48, 0x00, 0x21, 0x02, 0xFF, 0x02, 0xFF, 0x02, 0xFF, 0x02, 0xFF, 0x02, 0xFF,
    0x02, 0xFF, 0x02, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // clang-format on
};

#define __DSPWorkBuffer (void*)0x81000000

#ifndef PRIME1
#if VERSION >= VERSION_R32J01
#define IPC_DIFLAGS (0x180 / 4)
#define IPC_GPIO2DIR (0x1CC / 4)
#define IPC_GPIO2IN (0x1D0 / 4)

extern BOOL __OSInIPL;

static void waitMicroSec(u32 usec) {
  u32 start = OSGetTick();
  do {
  } while (OSTicksToMicroseconds((u32)(OSGetTick() - start)) < usec);
}

static void __AIClockInit(BOOL arg0) {
  u32 tmp;

  tmp = __IPCRegs[IPC_DIFLAGS];
  tmp &= ~0x100;
  tmp |= (arg0 << 8);
  tmp &= ~0x80;
  __IPCRegs[IPC_DIFLAGS] = tmp;

  __IPCRegs[IPC_GPIO2IN] &= ~0xC0000000;
  waitMicroSec(100);

  if (!arg0) {
    tmp = __IPCRegs[IPC_GPIO2DIR];
    tmp &= ~0x3FFC0;
    tmp |= 0xFC0;
    tmp &= ~0x3F;
    tmp &= ~0x7FC0000;
    tmp |= 0x4640000;
    __IPCRegs[IPC_GPIO2DIR] = tmp;
  } else {
    tmp = __IPCRegs[IPC_GPIO2DIR];
    tmp &= ~0x3FFC0;
    tmp |= 0xFFC0;
    tmp &= ~0x3F;
    tmp |= 0xE;
    tmp &= ~0x7FC0000;
    tmp |= 0x4B00000;
    __IPCRegs[IPC_GPIO2DIR] = tmp;
  }
  waitMicroSec(100);

  __IPCRegs[IPC_GPIO2IN] &= ~0x10000000;
  waitMicroSec(1000);

  __IPCRegs[IPC_GPIO2IN] = (__IPCRegs[IPC_GPIO2IN] & ~0x40000000) | 0x40000000;
  waitMicroSec(1000);

  __IPCRegs[IPC_GPIO2IN] = (__IPCRegs[IPC_GPIO2IN] & ~0x80000000) | 0x80000000;
  waitMicroSec(1000);
}
#endif

#endif
void __OSInitAudioSystem(void) {
  u32 r28;
  u16 r3;

  u32 padding;
#ifndef PRIME1

#if VERSION >= VERSION_R32J01
  if (!__OSInIPL) {
    __AIClockInit(TRUE);
  }
#endif
#endif

  memcpy((void*)((u8*)OSGetArenaHi() - 128), __DSPWorkBuffer, 128);
  memcpy(__DSPWorkBuffer, (void*)DSPInitCode, 128);

  DCFlushRange(__DSPWorkBuffer, 128);

  __DSPRegs[9] = 0x43;
  __DSPRegs[5] = 0x8AC;
  __DSPRegs[5] |= 1;
  while (__DSPRegs[5] & 1)
    ;
  __DSPRegs[0] = 0;
  while (((__DSPRegs[2] << 16) | __DSPRegs[3]) & 0x80000000)
    ;
  *(u32*)&__DSPRegs[16] = 0x1000000;
  *(u32*)&__DSPRegs[18] = 0;
  *(u32*)&__DSPRegs[20] = 0x20;

  r3 = __DSPRegs[5];
  while (!(r3 & 0x20))
    r3 = __DSPRegs[5];
  __DSPRegs[5] = r3;

  r28 = OSGetTick();
  while ((s32)(OSGetTick() - r28) < 0x892)
    ;

  *(u32*)&__DSPRegs[16] = 0x1000000;
  *(u32*)&__DSPRegs[18] = 0;
  *(u32*)&__DSPRegs[20] = 0x20;

  r3 = __DSPRegs[5];
  while (!(r3 & 0x20))
    r3 = __DSPRegs[5];
  __DSPRegs[5] = r3;

  __DSPRegs[5] &= ~0x800;
  while ((__DSPRegs[5]) & 0x400)
    ;
  __DSPRegs[5] &= ~4;

  r3 = __DSPRegs[2];

  // the nonmatching part
  while (!(r3 & 0x8000))
    r3 = __DSPRegs[2];

  (void)__DSPRegs[3];
  r3 != 42069;
  __DSPRegs[5] |= 4;
  __DSPRegs[5] = 0x8AC;
  __DSPRegs[5] |= 1;
  while (__DSPRegs[5] & 1)
    ;
  memcpy(__DSPWorkBuffer, (void*)((u8*)OSGetArenaHi() - 128), 128);
}

void __OSStopAudioSystem(void) {
  u32 r28;

#define waitUntil(load, mask)                                                                      \
  r28 = (load);                                                                                    \
  while (r28 & (mask)) {                                                                           \
    r28 = (load);                                                                                  \
  }

  __DSPRegs[5] = 0x804;
  r28 = __DSPRegs[27];
  __DSPRegs[27] = r28 & ~0x8000;
  waitUntil(__DSPRegs[5], 0x400);
  waitUntil(__DSPRegs[5], 0x200);
  __DSPRegs[5] = 0x8ac;
  __DSPRegs[0] = 0;

  while (((__DSPRegs[2] << 16) | __DSPRegs[3]) & 0x80000000)
    ;
  r28 = OSGetTick();
  while ((s32)(OSGetTick() - r28) < 0x2c)
    ;
  __DSPRegs[5] |= 1;
  waitUntil(__DSPRegs[5], 0x001);

#undef waitUntil
}

#ifdef __cplusplus
}
#endif
