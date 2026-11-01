#include <SDKVersion.h>
#include "dolphin/OSRtcPriv.h"
#include "dolphin/dvd.h"
#include "dolphin/os/OSBootInfo.h"
#include "dolphin/os.h"
#include "dolphin/vi.h"
#include "dolphin/hw_regs.h"

volatile u8 DAT_800030e2 : 0x800030e2;
typedef struct Unk {
  u8 pad[0x24];
  u32 resetCode;
} Unk;
volatile Unk DAT_cc003000 : 0xcc003000;

typedef struct Unk2 {
  u16 _0;
  u16 _2;
} Unk2;

volatile Unk2 DAT_cc002000 : 0xcc002000;

typedef struct OSResetQueue {
  OSResetFunctionInfo* first;
  OSResetFunctionInfo* last;
} OSResetQueue;

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
OSResetQueue ResetFunctionQueue;
#else
static OSResetQueue ResetFunctionQueue;
static u32 bootThisDol;
extern OSExecParams __OSRebootParams;
#endif

void OSRegisterResetFunction(OSResetFunctionInfo* func) {
  OSResetFunctionInfo* tmp;
  OSResetFunctionInfo* iter;

  for (iter = ResetFunctionQueue.first; iter && iter->priority <= func->priority; iter = iter->next)
    ;

  if (iter == NULL) {
    tmp = ResetFunctionQueue.last;
    if (tmp == NULL) {
      ResetFunctionQueue.first = func;
    } else {
      tmp->next = func;
    }
    func->prev = tmp;
    func->next = NULL;
    ResetFunctionQueue.last = func;
    return;
  }

  func->next = iter;
  tmp = iter->prev;
  iter->prev = func;
  func->prev = tmp;
  if (tmp == NULL) {
    ResetFunctionQueue.first = func;
    return;
  }
  tmp->next = func;
}

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
BOOL __OSCallResetFunctions(u32 arg0) {
  OSResetFunctionInfo* iter;
  s32 retCode = 0;
#else
BOOL __OSCallResetFunctions(BOOL final) {
  OSResetFunctionInfo* info;
  s32 err;
  u32 priority;
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  for (iter = ResetFunctionQueue.first; iter != NULL; iter = iter->next) {
    retCode |= !iter->func(arg0);
#else
  priority = 0;
  err = 0;

  for (info = ResetFunctionQueue.first; info != NULL;) {
    if (err != 0 && priority != info->priority)
      break;
    err |= !info->func(final);
    priority = info->priority;
    info = info->next;
#endif
  }
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  retCode |= !__OSSyncSram();
  if (retCode) {
#else

  err |= !__OSSyncSram();
  if (err) {
#endif
    return 0;
  }
  return 1;
}

asm void Reset(register s32 resetCode) {
  // clang-format off
    nofralloc
    b lbl_8038315C
lbl_80383140:
    mfspr r8, HID0
    ori r8, r8, 8
    mtspr HID0, r8
    isync 
    sync
    nop 
    b lbl_80383160
lbl_8038315C:
    b lbl_8038317C
lbl_80383160:
    mftb r5, 268
lbl_80383164:
    mftb r6, 268
    subf r7, r5, r6
    cmplwi r7, 0x1124
    blt lbl_80383164
    nop 
    b lbl_80383180
lbl_8038317C:
    b lbl_8038319C
lbl_80383180:
    lis r8, 0xCC003000@h
    ori r8, r8, 0xCC003000@l
    li r4, 3
    stw r4, 0x24(r8)
    stw r3, 0x24(r8)
    nop 
    b lbl_803831A0
lbl_8038319C:
    b lbl_803831A8
lbl_803831A0:
    nop 
    b lbl_803831A0
lbl_803831A8:
    b lbl_80383140
  // clang-format on
}

OSThreadQueue __OSActiveThreadQueue : (OS_BASE_CACHED | 0x00DC);

static void KillThreads(void) {
  OSThread* thread;
  OSThread* next;

  for (thread = __OSActiveThreadQueue.head; thread; thread = next) {
    next = thread->linkActive.next;
    switch (thread->state) {
    case 1:
    case 4:
      OSCancelThread(thread);
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
      break;
#else
      continue;
#endif
    default:
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
      break;
#else
      continue;
#endif
    }
  }
}

void __OSDoHotReset(s32 arg0) {
  OSDisableInterrupts();
  __VIRegs[1] = 0;
  ICFlashInvalidate();
  Reset(arg0 * 8);
}

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
void OSResetSystem(int reset, u32 resetCode, BOOL forceMenu) {
#else
static void __OSShutdownDevices(BOOL doRecal) {
#endif
  BOOL rc;
  BOOL disableRecalibration;
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  u32 unk[3];
  OSDisableScheduler();
#endif
  __OSStopAudioSystem();

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  if (reset == OS_RESET_SHUTDOWN) {
#else
  if (!doRecal) {
#endif
    disableRecalibration = __PADDisableRecalibration(TRUE);
  }

  while (!__OSCallResetFunctions(FALSE))
    ;
#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2003_04
  while (!__OSSyncSram())
    ;

  OSDisableInterrupts();

  rc = __OSCallResetFunctions(TRUE);

  LCDisable();
  if (!doRecal) {
    __PADDisableRecalibration(disableRecalibration);
  }

  KillThreads();
}

void OSResetSystem(int reset, u32 resetCode, BOOL forceMenu) {
  OSSram* sram;

  OSDisableScheduler();
#endif

  if (reset == OS_RESET_HOTRESET && forceMenu) {
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
    OSSram* sram;

#endif
    sram = __OSLockSram();
    sram->flags |= 0x40;
    __OSUnlockSram(TRUE);

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
    while (!__OSSyncSram())
      ;
#else
    resetCode = 0;
#endif
  }
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  OSDisableInterrupts();
  __OSCallResetFunctions(TRUE);
  LCDisable();
#else

  if (reset == OS_RESET_SHUTDOWN ||
      (reset == OS_RESET_RESTART && (bootThisDol || resetCode + 0x3fff0000 == 0))) {
    __OSShutdownDevices(FALSE);
  } else {
    __OSShutdownDevices(TRUE);
  }

#endif
  if (reset == OS_RESET_HOTRESET) {
    __OSDoHotReset(resetCode);
  } else if (reset == OS_RESET_RESTART) {
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
    KillThreads();
#else
    if (forceMenu == TRUE) {
      OSReport("OSResetSystem(): You can't specify TRUE to forceMenu if you restart. Ignored\n");
    }
#endif
    OSEnableScheduler();
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
    __OSReboot(resetCode, forceMenu);
#else
    __OSReboot(resetCode, bootThisDol);
#endif
  }
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  KillThreads();
#endif
  memset(OSPhysicalToCached(0x40), 0, 0xcc - 0x40);
  memset(OSPhysicalToCached(0xd4), 0, 0xe8 - 0xd4);
  memset(OSPhysicalToCached(0xf4), 0, 0xf8 - 0xf4);
  memset(OSPhysicalToCached(0x3000), 0, 0xc0);
  memset(OSPhysicalToCached(0x30c8), 0, 0xd4 - 0xc8);
  memset(OSPhysicalToCached(0x30e2), 0, 1);
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02

  __PADDisableRecalibration(disableRecalibration);
#endif
}

u32 OSGetResetCode(void) {
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  if (DAT_800030e2 != 0) {
    return 0x80000000;
  }
  return ((DAT_cc003000.resetCode & ~7) >> 3);
#else
  u32 resetCode;
  if (__OSRebootParams.valid)
    resetCode = 0x80000000 | __OSRebootParams.restartCode;
  else
    resetCode = (__PIRegs[9] & 0xFFFFFFF8) / 8;

  return resetCode;
#endif
}

#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2003_04
u32 OSSetBootDol(u32 dolOffset) {
  u32 oldDol;

  oldDol = bootThisDol;
  bootThisDol = dolOffset;
  return oldDol;
}

#endif
