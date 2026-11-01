#include <SDKVersion.h>
#include "string.h"
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
#include "dolphin/base/PPCArch.h"
#else
#include "dolphin/PPCArch.h"
#endif
#include "dolphin/os.h"

void __OSSystemCallVectorStart();
void __OSSystemCallVectorEnd();
static asm void SystemCallVector() {
  nofralloc
entry __OSSystemCallVectorStart
  mfspr r9, HID0
  ori r10, r9, 8
  mtspr HID0, r10
  isync
    sync
  mtspr HID0, r9

  rfi

entry __OSSystemCallVectorEnd
  nop
}

void __OSInitSystemCall() {
  void* addr = OSPhysicalToCached(0x00C00);
  memcpy(addr, __OSSystemCallVectorStart, (size_t)__OSSystemCallVectorEnd - (size_t)__OSSystemCallVectorStart);
  DCFlushRangeNoSync(addr, 0x100);
  __sync();
  ICInvalidateRange(addr, 0x100);
}
