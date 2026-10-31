#include "dolphin/dvd.h"
#include "dolphin/os.h"
#include "dolphin/os/OSBootInfo.h"

extern OSExecParams __OSRebootParams;
void __OSBootDol(u32 doloffset, u32 restartCode, const char** argv);

#if VERSION >= VERSION_R32J01
extern u32 __OSNextPartitionType;
vu32 __OSLaunchPartitionType : (OS_BASE_CACHED | 0x3194);
#endif

static void* SaveStart = NULL;
static void* SaveEnd = NULL;

void __OSReboot(u32 resetCode, u32 bootDol) {
#if VERSION < VERSION_R32J01
  OSContext exceptionContext;
#endif
  const char* argvToPass;

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
}

void OSSetSaveRegion(void* start, void* end) {
  SaveStart = start;
  SaveEnd = end;
}

void OSGetSaveRegion(void** start, void** end) {
  *start = SaveStart;
  *end = SaveEnd;
}

void OSGetSavedRegion(void** start, void** end) {
  *start = __OSRebootParams.regionStart;
  *end = __OSRebootParams.regionEnd;
}
