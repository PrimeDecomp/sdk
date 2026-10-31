#ifdef PRIME1
#include "GameVersions.h"

#include "dolphin/ai.h"
#endif
#include "dolphin/dvd.h"
#include "dolphin/os.h"
#include "dolphin/os/OSBootInfo.h"
#ifdef PRIME1
#include "dolphin/os/OSContext.h"
#include "dolphin/os/OSInterrupt.h"
#include "dolphin/types.h"
#endif

#ifdef PRIME1
typedef struct ApploaderHeader {
  // total size: 0x20
  char date[16];  // offset 0x0, size 0x10
  u32 entry;      // offset 0x10, size 0x4
  u32 size;       // offset 0x14, size 0x4
  u32 rebootSize; // offset 0x18, size 0x4
  u32 reserved2;  // offset 0x1C, size 0x4
} ApploaderHeader;
#else
extern OSExecParams __OSRebootParams;
void __OSBootDol(u32 doloffset, u32 restartCode, const char** argv);
#endif

#ifdef PRIME1
ATTRIBUTE_ALIGN_DECL(32, static ApploaderHeader Header);
#else
#if VERSION >= VERSION_R32J01
extern u32 __OSNextPartitionType;
vu32 __OSLaunchPartitionType : (OS_BASE_CACHED | 0x3194);
#endif
#endif

#ifdef PRIME1
extern void *__OSSavedRegionStart;
extern void *__OSSavedRegionEnd;
#else
static void* SaveStart = NULL;
static void* SaveEnd = NULL;
#endif

#ifdef PRIME1
static void *SaveStart = NULL;
static void *SaveEnd = NULL;
#else
void __OSReboot(u32 resetCode, u32 bootDol) {
#if VERSION < VERSION_R32J01
  OSContext exceptionContext;
#endif
  const char* argvToPass;
#endif

#ifdef PRIME1
volatile u8 OS_REBOOT_BOOL : 0x800030e2;

extern u32 BOOT_REGION_START AT_ADDRESS(0x812FDFF0);
extern u32 BOOT_REGION_END AT_ADDRESS(0x812FDFEC);
extern u32 UNK_HOT_RESET1 AT_ADDRESS(0x817ffff8);
extern u32 UNK_HOT_RESET2 AT_ADDRESS(0x817ffffc);

static volatile BOOL Prepared = FALSE;

extern void __OSDoHotReset(s32 arg0);
extern void __DVDPrepareResetAsync(DVDCBCallback callback);
extern BOOL DVDCheckDisk(void);
extern BOOL DVDReadAbsAsyncForBS(DVDCommandBlock *block, void *addr, s32 length, s32 offset,
                                 DVDCBCallback callback);

asm void Run(void *entrypoint) {
  // clang-format off
  nofralloc
  sync
  isync
  mtlr r3
  blr
  // clang-format on
#else
  OSDisableInterrupts();
  OSSetArenaLo((void*)0x81280000);
  OSSetArenaHi((void*)0x812f0000);
#if VERSION < VERSION_R32J01
  OSClearContext(&exceptionContext);
  OSSetCurrentContext(&exceptionContext);
#endif
  argvToPass = NULL;
#if VERSION >= VERSION_R32J01
  __OSNextPartitionType = __OSLaunchPartitionType;
#endif
  __OSBootDol(bootDol, resetCode | 0x80000000, &argvToPass);
#endif
}

#ifdef PRIME1
static void Callback(s32 result, DVDCommandBlock *block) { Prepared = TRUE; }

#if VERSION >= VERSION_GM8E_02
extern BOOL __OSIsGcam;
extern void DVDResume(void);
extern BOOL DVDReadAbsAsyncPrio(DVDCommandBlock *block, void *addr, s32 length, s32 offset,
                                DVDCBCallback callback, s32 prio);

static BOOL IsStreamEnabled(void) {
  if (DVDGetCurrentDiskID()->streaming) {
    return TRUE;
  }
  return FALSE;
}

void __OSReboot(u32 resetCode, u32 bootDol) {
  OSContext exceptionContext;
  DVDCommandBlock dvdCmd;
  DVDCommandBlock dvdCmd2;
  DVDCommandBlock dvdCmd3;
  u32 numBytes;
  u32 offset;
  OSTime start;

  OSDisableInterrupts();
  UNK_HOT_RESET2 = 0;
  UNK_HOT_RESET1 = 0;
  OS_REBOOT_BOOL = 1;
  BOOT_REGION_START = (u32)SaveStart;
  BOOT_REGION_END = (u32)SaveEnd;
  OSClearContext(&exceptionContext);
  OSSetCurrentContext(&exceptionContext);
  DVDInit();
  DVDSetAutoInvalidation(TRUE);
  DVDResume();
  Prepared = FALSE;
  __DVDPrepareResetAsync(Callback);

  __OSMaskInterrupts(~0x1F);
  __OSUnmaskInterrupts(0x400);
  OSEnableInterrupts();

  start = OSGetTime();
  while (Prepared != TRUE) {
    if (!DVDCheckDisk() || OSGetTime() - start > OSSecondsToTicks(1)) {
      __OSDoHotReset(UNK_HOT_RESET2);
    }
  }

  if (!__OSIsGcam && IsStreamEnabled()) {
    AISetStreamVolLeft(0);
    AISetStreamVolRight(0);
    DVDCancelStreamAsync(&dvdCmd, NULL);
    start = OSGetTime();
    while (DVDGetCommandBlockStatus(&dvdCmd)) {
      if (!DVDCheckDisk() || OSGetTime() - start > OSSecondsToTicks(1)) {
        __OSDoHotReset(UNK_HOT_RESET2);
      }
    }
    AISetStreamPlayState(0);
  }

  DVDReadAbsAsyncPrio(&dvdCmd2, &Header, 32, 0x2440, NULL, 0);
  start = OSGetTime();
  while (DVDGetCommandBlockStatus(&dvdCmd2)) {
    if (!DVDCheckDisk() || OSGetTime() - start > OSSecondsToTicks(1)) {
      __OSDoHotReset(UNK_HOT_RESET2);
    }
  }

  offset = Header.size + 0x20;
  numBytes = OSRoundUp32B(Header.rebootSize);
  DVDReadAbsAsyncPrio(&dvdCmd3, (void *)OS_BOOTROM_ADDR, numBytes, offset + 0x2440, NULL, 0);
  start = OSGetTime();
  while (DVDGetCommandBlockStatus(&dvdCmd3)) {
    if (!DVDCheckDisk() || OSGetTime() - start > OSSecondsToTicks(1)) {
      __OSDoHotReset(UNK_HOT_RESET2);
    }
  }

  ICInvalidateRange((void *)OS_BOOTROM_ADDR, numBytes);
  OSDisableInterrupts();
  ICFlashInvalidate();
  Run((void *)OS_BOOTROM_ADDR);
}
#else
static inline void ReadApploader(void *addr, long length, long offset) {
  DVDCommandBlock block;

  while (!Prepared) {
  }

  DVDReadAbsAsyncForBS(&block, addr, length, offset + 0x2440, NULL);
  while (1) {
    switch (block.state) {
    case 0:
      return;
    case 1:
      break;
    case -1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
      __OSDoHotReset(UNK_HOT_RESET2);
      break;
    default:
      break;
    }
  }
}

void __OSReboot(u32 resetCode, u32 bootDol) {
  OSContext exceptionContext;
  u32 numBytes;
  u32 offset;

  OSDisableInterrupts();
  UNK_HOT_RESET2 = 0;
  UNK_HOT_RESET1 = 0;
  OS_REBOOT_BOOL = 1;
  BOOT_REGION_START = (u32)SaveStart;
  BOOT_REGION_END = (u32)SaveEnd;
  OSClearContext(&exceptionContext);
  OSSetCurrentContext(&exceptionContext);
  DVDInit();
  DVDSetAutoInvalidation(TRUE);
  __DVDPrepareResetAsync(Callback);
  if (!DVDCheckDisk()) {
    __OSDoHotReset(UNK_HOT_RESET2);
  }

  __OSMaskInterrupts(~0x1F);
  __OSUnmaskInterrupts(0x400);
  OSEnableInterrupts();

  ReadApploader(&Header, 32, 0);
  offset = Header.size + 0x20;
  numBytes = OSRoundUp32B(Header.rebootSize);
  ReadApploader((void *)OS_BOOTROM_ADDR, numBytes, offset);

  ICInvalidateRange((void *)OS_BOOTROM_ADDR, numBytes);
  OSDisableInterrupts();
  ICFlashInvalidate();
  Run((void *)OS_BOOTROM_ADDR);
}
#endif

void OSSetSaveRegion(void *start, void *end) {
#else
void OSSetSaveRegion(void* start, void* end) {
#endif
  SaveStart = start;
  SaveEnd = end;
}

#ifdef PRIME1
void OSGetSaveRegion(void **start, void **end) {
#else
void OSGetSaveRegion(void** start, void** end) {
#endif
  *start = SaveStart;
  *end = SaveEnd;
}

#ifdef PRIME1
void OSGetSavedRegion(void **start, void **end) {
  *start = __OSSavedRegionStart;
  *end = __OSSavedRegionEnd;
#else
void OSGetSavedRegion(void** start, void** end) {
  *start = __OSRebootParams.regionStart;
  *end = __OSRebootParams.regionEnd;
#endif
}
