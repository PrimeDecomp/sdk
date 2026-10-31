#ifndef PRIME1
#include "GameVersions.h"
#endif
#include <dolphin/os.h>

#define TRUNC(n, a) (((u32)(n)) & ~((a)-1))
#define ROUND(n, a) (((u32)(n) + (a)-1) & ~((a)-1))

vu16 __MEMRegs[64] : 0xCC004000;
extern OSErrorHandler __OSErrorTable[16];

#ifdef PRIME1
static BOOL OnReset(BOOL final);

static OSResetFunctionInfo ResetFunctionInfo = {
    OnReset,
    127,
};

static BOOL OnReset(BOOL final) {
  if (final != FALSE) {
    __MEMRegs[8] = 0xFF;
    __OSMaskInterrupts(0xf0000000);
  }
  return TRUE;
}

u32 OSGetPhysicalMemSize() { return *(u32*)(OSPhysicalToCached(0x0028)); }

u32 OSGetConsoleSimulatedMemSize() { return *(u32*)(OSPhysicalToCached(0x00F0)); }
#else
#if VERSION >= VERSION_R32J01
static BOOL OnShutdown(BOOL final, u32 event);

static OSShutdownFunctionInfo ShutdownFunctionInfo = {
    OnShutdown,
    127,
};

u32 OSGetPhysicalMem1Size() { return *(u32*)(OSPhysicalToCached(0x3100)); }

u32 OSGetPhysicalMem2Size() { return *(u32*)(OSPhysicalToCached(0x3118)); }

u32 OSGetConsoleSimulatedMem1Size() { return *(u32*)(OSPhysicalToCached(0x3104)); }

u32 OSGetConsoleSimulatedMem2Size() { return *(u32*)(OSPhysicalToCached(0x311C)); }

static BOOL OnShutdown(BOOL final, u32 event) {
  if (final != FALSE) {
    __MEMRegs[8] = 0xFF;
    __OSMaskInterrupts(0xf0000000);
  }
  return TRUE;
}
#else
static BOOL OnReset(BOOL final);

static OSResetFunctionInfo ResetFunctionInfo = {
    OnReset,
    127,
};

static BOOL OnReset(BOOL final) {
  if (final != FALSE) {
    __MEMRegs[8] = 0xFF;
    __OSMaskInterrupts(0xf0000000);
  }
  return TRUE;
}

u32 OSGetPhysicalMemSize() { return *(u32*)(OSPhysicalToCached(0x0028)); }

u32 OSGetConsoleSimulatedMemSize() { return *(u32*)(OSPhysicalToCached(0x00F0)); }
#endif
#endif

static void MEMIntrruptHandler(__OSInterrupt interrupt, OSContext* context) {
  u32 addr;
  u32 cause;

  cause = __MEMRegs[0xf];
  addr = (((u32)__MEMRegs[0x12] & 0x3ff) << 16) | __MEMRegs[0x11];
  __MEMRegs[0x10] = 0;

  if (__OSErrorTable[OS_ERROR_PROTECTION]) {
    __OSErrorTable[OS_ERROR_PROTECTION](OS_ERROR_PROTECTION, context, cause, addr);
    return;
  }

  __OSUnhandledException(OS_ERROR_PROTECTION, context, cause, addr);
}

void OSProtectRange(u32 chan, void* addr, u32 nBytes, u32 control) {
  BOOL enabled;
  u32 start;
  u32 end;
  u16 reg;
  if (4 <= chan) {
    return;
  }

  control &= OS_PROTECT_CONTROL_RDWR;

  end = (u32)addr + nBytes;
  start = TRUNC(addr, 1u << 10);
  end = ROUND(end, 1u << 10);

  DCFlushRange((void*)start, end - start);

  enabled = OSDisableInterrupts();

  __OSMaskInterrupts(OS_INTERRUPTMASK(__OS_INTERRUPT_MEM_0 + chan));

  __MEMRegs[0 + 2 * chan] = (u16)(start >> 10);
  __MEMRegs[1 + 2 * chan] = (u16)(end >> 10);

  reg = __MEMRegs[8];
  reg &= ~(OS_PROTECT_CONTROL_RDWR << 2 * chan);
  reg |= control << 2 * chan;
  __MEMRegs[8] = reg;

  if (control != OS_PROTECT_CONTROL_RDWR) {
    __OSUnmaskInterrupts(OS_INTERRUPTMASK(__OS_INTERRUPT_MEM_0 + chan));
  }

  OSRestoreInterrupts(enabled);
}

static asm void Config24MB() {
  // clang-format off
  nofralloc

  addi    r7,r0,0

  addis   r4,r0,0x00000002@ha
  addi    r4,r4,0x00000002@l
  addis   r3,r0,0x800001ff@ha
  addi    r3,r3,0x800001ff@l

  addis   r6,r0,0x01000002@ha
  addi    r6,r6,0x01000002@l
  addis   r5,r0,0x810000ff@ha
  addi    r5,r5,0x810000ff@l

  isync

  mtspr   dbat0u,r7
  mtspr   dbat0l,r4
  mtspr   dbat0u,r3
  isync

  mtspr   ibat0u,r7
  mtspr   ibat0l,r4
  mtspr   ibat0u,r3
  isync

  mtspr   dbat2u,r7
  mtspr   dbat2l,r6
  mtspr   dbat2u,r5
  isync

  mtspr   ibat2u,r7
  mtspr   ibat2l,r6
  mtspr   ibat2u,r5
  isync

  mfmsr   r3
  ori     r3, r3, 0x30
  mtsrr1  r3

  mflr    r3
  mtsrr0  r3
  rfi
  // clang-format on
}

static asm void Config48MB() {
  // clang-format off
  nofralloc

  addi    r7,r0,0x0000

  addis   r4,r0,0x00000002@ha
  addi    r4,r4,0x00000002@l
  addis   r3,r0,0x800003ff@ha
  addi    r3,r3,0x800003ff@l

  addis   r6,r0,0x02000002@ha
  addi    r6,r6,0x02000002@l
  addis   r5,r0,0x820001ff@ha
  addi    r5,r5,0x820001ff@l

  isync

  mtspr   dbat0u,r7
  mtspr   dbat0l,r4
  mtspr   dbat0u,r3
  isync

  mtspr   ibat0u,r7
  mtspr   ibat0l,r4
  mtspr   ibat0u,r3
  isync

  mtspr   dbat2u,r7
  mtspr   dbat2l,r6
  mtspr   dbat2u,r5
  isync

  mtspr   ibat2u,r7
  mtspr   ibat2l,r6
  mtspr   ibat2u,r5
  isync

  mfmsr   r3
  ori     r3, r3, 0x30
  mtsrr1  r3

  mflr    r3
  mtsrr0  r3
  rfi
  // clang-format on
}

#ifndef PRIME1
#if VERSION >= VERSION_R32J01
static asm void ConfigMEM2_52MB() {
    // clang-format off
    nofralloc

    li r7, 0
    lis r4, 0x10000002@ha
    addi r4, r4, 0x10000002@l
    lis r3, 0x900003FF@ha
    addi r3, r3, 0x900003FF@l
    lis r6, 0x1000002A@ha
    addi r6, r6, 0x1000002A@l
    lis r5, 0xD00007FF@ha
    addi r5, r5, 0xD00007FF@l
    isync
    mtspr 0x238, r7
    mtspr 0x239, r4
    mtspr 0x238, r3
    isync
    mtspr 0x230, r7
    mtspr 0x231, r4
    mtspr 0x230, r3
    isync
    mtspr 0x23a, r7
    mtspr 0x23b, r6
    mtspr 0x23a, r5
    isync
    mtspr 0x232, r7
    mtspr 0x233, r7
    isync
    lis r4, 0x12000002@ha
    addi r4, r4, 0x12000002@l
    lis r3, 0x920001FF@ha
    addi r3, r3, 0x920001FF@l
    lis r6, 0x13000002@ha
    addi r6, r6, 0x13000002@l
    lis r5, 0x9300007F@ha
    addi r5, r5, 0x9300007F@l
    isync
    mtspr 0x23c, r7
    mtspr 0x23d, r4
    mtspr 0x23c, r3
    isync
    mtspr 0x234, r7
    mtspr 0x235, r4
    mtspr 0x234, r3
    isync
    mtspr 0x23e, r7
    mtspr 0x23f, r6
    mtspr 0x23e, r5
    isync
    mtspr 0x236, r7
    mtspr 0x237, r6
    mtspr 0x236, r5
    isync
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    mflr r3
    mtspr 0x1a, r3
    rfi
    // clang-format on
}

static asm void ConfigMEM2_56MB() {
    // clang-format off
    nofralloc

    li r7, 0
    lis r4, 0x10000002@ha
    addi r4, r4, 0x10000002@l
    lis r3, 0x900003FF@ha
    addi r3, r3, 0x900003FF@l
    lis r6, 0x1000002A@ha
    addi r6, r6, 0x1000002A@l
    lis r5, 0xD00007FF@ha
    addi r5, r5, 0xD00007FF@l
    isync
    mtspr 0x238, r7
    mtspr 0x239, r4
    mtspr 0x238, r3
    isync
    mtspr 0x230, r7
    mtspr 0x231, r4
    mtspr 0x230, r3
    isync
    mtspr 0x23a, r7
    mtspr 0x23b, r6
    mtspr 0x23a, r5
    isync
    mtspr 0x232, r7
    mtspr 0x233, r7
    isync
    lis r4, 0x12000002@ha
    addi r4, r4, 0x12000002@l
    lis r3, 0x920001FF@ha
    addi r3, r3, 0x920001FF@l
    lis r6, 0x13000002@ha
    addi r6, r6, 0x13000002@l
    lis r5, 0x930000FF@ha
    addi r5, r5, 0x930000FF@l
    isync
    mtspr 0x23c, r7
    mtspr 0x23d, r4
    mtspr 0x23c, r3
    isync
    mtspr 0x234, r7
    mtspr 0x235, r4
    mtspr 0x234, r3
    isync
    mtspr 0x23e, r7
    mtspr 0x23f, r6
    mtspr 0x23e, r5
    isync
    mtspr 0x236, r7
    mtspr 0x237, r6
    mtspr 0x236, r5
    isync
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    mflr r3
    mtspr 0x1a, r3
    rfi
    // clang-format on
}

static asm void ConfigMEM2_64MB() {
    // clang-format off
    nofralloc

    li r7, 0
    lis r4, 0x10000002@ha
    addi r4, r4, 0x10000002@l
    lis r3, 0x900007FF@ha
    addi r3, r3, 0x900007FF@l
    lis r6, 0x1000002A@ha
    addi r6, r6, 0x1000002A@l
    lis r5, 0xD00007FF@ha
    addi r5, r5, 0xD00007FF@l
    isync
    mtspr 0x238, r7
    mtspr 0x239, r4
    mtspr 0x238, r3
    isync
    mtspr 0x230, r7
    mtspr 0x231, r4
    mtspr 0x230, r3
    isync
    mtspr 0x23a, r7
    mtspr 0x23b, r6
    mtspr 0x23a, r5
    isync
    mtspr 0x232, r7
    mtspr 0x233, r7
    isync
    mtspr 0x234, r7
    mtspr 0x235, r7
    isync
    mtspr 0x236, r7
    mtspr 0x237, r7
    isync
    mtspr 0x23c, r7
    mtspr 0x23d, r7
    isync
    mtspr 0x23e, r7
    mtspr 0x23f, r7
    isync
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    mflr r3
    mtspr 0x1a, r3
    rfi
    // clang-format on
}

static asm void ConfigMEM2_112MB() {
    // clang-format off
    nofralloc

    li r7, 0
    lis r4, 0x10000002@ha
    addi r4, r4, 0x10000002@l
    lis r3, 0x900007FF@ha
    addi r3, r3, 0x900007FF@l
    lis r6, 0x1000002A@ha
    addi r6, r6, 0x1000002A@l
    lis r5, 0xD0000FFF@ha
    addi r5, r5, 0xD0000FFF@l
    isync
    mtspr 0x238, r7
    mtspr 0x239, r4
    mtspr 0x238, r3
    isync
    mtspr 0x230, r7
    mtspr 0x231, r4
    mtspr 0x230, r3
    isync
    mtspr 0x23a, r7
    mtspr 0x23b, r6
    mtspr 0x23a, r5
    isync
    mtspr 0x232, r7
    mtspr 0x233, r7
    isync
    lis r4, 0x14000002@ha
    addi r4, r4, 0x14000002@l
    lis r3, 0x940003FF@ha
    addi r3, r3, 0x940003FF@l
    lis r6, 0x16000002@ha
    addi r6, r6, 0x16000002@l
    lis r5, 0x960001FF@ha
    addi r5, r5, 0x960001FF@l
    isync
    mtspr 0x23c, r7
    mtspr 0x23d, r4
    mtspr 0x23c, r3
    isync
    mtspr 0x234, r7
    mtspr 0x235, r4
    mtspr 0x234, r3
    isync
    mtspr 0x23e, r7
    mtspr 0x23f, r6
    mtspr 0x23e, r5
    isync
    mtspr 0x236, r7
    mtspr 0x237, r6
    mtspr 0x236, r5
    isync
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    mflr r3
    mtspr 0x1a, r3
    rfi
    // clang-format on
}

static asm void ConfigMEM2_128MB() {
    // clang-format off
    nofralloc

    li r7, 0
    lis r4, 0x10000002@ha
    addi r4, r4, 0x10000002@l
    lis r3, 0x90000FFF@ha
    addi r3, r3, 0x90000FFF@l
    lis r6, 0x1000002A@ha
    addi r6, r6, 0x1000002A@l
    lis r5, 0xD0000FFF@ha
    addi r5, r5, 0xD0000FFF@l
    isync
    mtspr 0x238, r7
    mtspr 0x239, r4
    mtspr 0x238, r3
    isync
    mtspr 0x230, r7
    mtspr 0x231, r4
    mtspr 0x230, r3
    isync
    mtspr 0x23a, r7
    mtspr 0x23b, r6
    mtspr 0x23a, r5
    isync
    mtspr 0x232, r7
    mtspr 0x233, r7
    isync
    mtspr 0x234, r7
    mtspr 0x235, r7
    isync
    mtspr 0x236, r7
    mtspr 0x237, r7
    isync
    mtspr 0x23c, r7
    mtspr 0x23d, r7
    isync
    mtspr 0x23e, r7
    mtspr 0x23f, r7
    isync
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    mflr r3
    mtspr 0x1a, r3
    rfi
    // clang-format on
}

static asm void ConfigMEM_ES1_0() {
    // clang-format off
    nofralloc

    li r7, 0
    lis r4, 0x00000002@ha
    addi r4, r4, 0x00000002@l
    lis r3, 0x80000FFF@ha
    addi r3, r3, 0x80000FFF@l
    isync
    mtdbatu 0, r7
    mtdbatl 0, r4
    mtdbatu 0, r3
    isync
    mtibatu 0, r7
    mtibatl 0, r4
    mtibatu 0, r3
    isync
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    mflr r3
    mtspr 0x1a, r3
    rfi
    // clang-format on
}

#endif

#endif
static asm void RealMode(register u32 addr) {
  // clang-format off
  nofralloc
  clrlwi r3, r3, 2
  mtsrr0 r3
  mfmsr r3
  rlwinm r3, r3, 0, 28, 25
  mtsrr1 r3
  rfi
  // clang-format on
}

#ifdef PRIME1
void __OSInitMemoryProtection() {
  u32 padding[8];
  u32 simulatedSize;
  BOOL enabled;
  simulatedSize = OSGetConsoleSimulatedMemSize();
  enabled = OSDisableInterrupts();
  if (simulatedSize <= 0x1800000) {
    RealMode((u32)&Config24MB);
  } else if (simulatedSize <= 0x3000000) {
    RealMode((u32)&Config48MB);
  }

  __MEMRegs[16] = 0;
  __MEMRegs[8] = 0xFF;

  __OSMaskInterrupts(OS_INTERRUPTMASK_MEM_0 | OS_INTERRUPTMASK_MEM_1 | OS_INTERRUPTMASK_MEM_2 |
                     OS_INTERRUPTMASK_MEM_3);
  __OSSetInterruptHandler(__OS_INTERRUPT_MEM_0, MEMIntrruptHandler);
  __OSSetInterruptHandler(__OS_INTERRUPT_MEM_1, MEMIntrruptHandler);
  __OSSetInterruptHandler(__OS_INTERRUPT_MEM_2, MEMIntrruptHandler);
  __OSSetInterruptHandler(__OS_INTERRUPT_MEM_3, MEMIntrruptHandler);
  __OSSetInterruptHandler(__OS_INTERRUPT_MEM_ADDRESS, MEMIntrruptHandler);
  OSRegisterResetFunction(&ResetFunctionInfo);

  if (OSGetConsoleSimulatedMemSize() < OSGetPhysicalMemSize() &&
      OSGetConsoleSimulatedMemSize() == 0x1800000) {
    __MEMRegs[20] = 2;
  }

  __OSUnmaskInterrupts(OS_INTERRUPTMASK_MEM_ADDRESS);
  OSRestoreInterrupts(enabled);
}
#else
#if VERSION >= VERSION_R32J01
static void BATConfig(void) {
  u32 mem1sim;
  u32 mem1phys;
  void* mem2end;

  if (*(u32*)OSPhysicalToCached(0x3138) == 0) {
    if (OSGetPhysicalMem1Size == 0) {
      RealMode((u32)&ConfigMEM_ES1_0);
      return;
    }
  }

  mem1sim = OSGetConsoleSimulatedMem1Size();
  mem1phys = OSGetPhysicalMem1Size();
  if (OSGetConsoleSimulatedMem1Size() < mem1phys && mem1sim == 0x1800000) {
    DCInvalidateRange((void*)0x81800000, 0x1800000);
    __MEMRegs[20] = 2;
  }

  if (mem1sim <= 0x1800000) {
    RealMode((u32)&Config24MB);
  } else if (mem1sim <= 0x3000000) {
    RealMode((u32)&Config48MB);
  }

  mem2end = *(void**)OSPhysicalToCached(0x3120);
  if (OSGetConsoleSimulatedMem2Size() <= 0x4000000) {
    if (mem2end <= (void*)0x93400000) {
      RealMode((u32)&ConfigMEM2_52MB);
    } else if ((char*)mem2end <= (char*)0x93400000 + 0x400000) {
      RealMode((u32)&ConfigMEM2_56MB);
    } else {
      RealMode((u32)&ConfigMEM2_64MB);
    }
  } else if (OSGetConsoleSimulatedMem2Size() <= 0x8000000) {
    if (mem2end <= (void*)0x97000000) {
      RealMode((u32)&ConfigMEM2_112MB);
    } else {
      RealMode((u32)&ConfigMEM2_128MB);
    }
  }
}

void __OSInitMemoryProtection() {
  BOOL enabled = OSDisableInterrupts();

  __MEMRegs[16] = 0;
  __MEMRegs[8] = 0xFF;

  __OSMaskInterrupts(OS_INTERRUPTMASK_MEM_0 | OS_INTERRUPTMASK_MEM_1 | OS_INTERRUPTMASK_MEM_2 |
                     OS_INTERRUPTMASK_MEM_3);
  __OSSetInterruptHandler(__OS_INTERRUPT_MEM_0, MEMIntrruptHandler);
  __OSSetInterruptHandler(__OS_INTERRUPT_MEM_1, MEMIntrruptHandler);
  __OSSetInterruptHandler(__OS_INTERRUPT_MEM_2, MEMIntrruptHandler);
  __OSSetInterruptHandler(__OS_INTERRUPT_MEM_3, MEMIntrruptHandler);
  __OSSetInterruptHandler(__OS_INTERRUPT_MEM_ADDRESS, MEMIntrruptHandler);
  OSRegisterShutdownFunction(&ShutdownFunctionInfo);

  BATConfig();
  __OSUnmaskInterrupts(OS_INTERRUPTMASK_MEM_ADDRESS);
  OSRestoreInterrupts(enabled);
}
#else
void __OSInitMemoryProtection() {
  u32 padding[9];
  u32 temp;
  BOOL enabled;
  u32 size;

  size = OSGetConsoleSimulatedMemSize();
  enabled = OSDisableInterrupts();

  __MEMRegs[16] = 0;
  __MEMRegs[8] = 0xFF;

  __OSMaskInterrupts(OS_INTERRUPTMASK_MEM_0 | OS_INTERRUPTMASK_MEM_1 | OS_INTERRUPTMASK_MEM_2 |
                     OS_INTERRUPTMASK_MEM_3);
  __OSSetInterruptHandler(__OS_INTERRUPT_MEM_0, MEMIntrruptHandler);
  __OSSetInterruptHandler(__OS_INTERRUPT_MEM_1, MEMIntrruptHandler);
  __OSSetInterruptHandler(__OS_INTERRUPT_MEM_2, MEMIntrruptHandler);
  __OSSetInterruptHandler(__OS_INTERRUPT_MEM_3, MEMIntrruptHandler);
  __OSSetInterruptHandler(__OS_INTERRUPT_MEM_ADDRESS, MEMIntrruptHandler);
  OSRegisterResetFunction(&ResetFunctionInfo);

  temp = OSGetConsoleSimulatedMemSize();
  if (temp < OSGetPhysicalMemSize() && temp == 0x1800000) {
    DCInvalidateRange((void*)0x81800000, 0x1800000);
    __MEMRegs[20] = 2;
  }

  if (size <= 0x1800000) {
    RealMode((u32)&Config24MB);
  } else if (size <= 0x3000000) {
    RealMode((u32)&Config48MB);
  }

  __OSUnmaskInterrupts(OS_INTERRUPTMASK_MEM_ADDRESS);
  OSRestoreInterrupts(enabled);
}
#endif
#endif
