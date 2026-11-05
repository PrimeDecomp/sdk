#include <revolution/ax.h>
#include <revolution/os.h>

#if SDK_VERSION < SDK_VERSION_RVL_2008_01
const char *__AXVersion = "<< RVL_SDK - AX \trelease build: Dec 18 2006 15:43:48 (0x4199_60831) >>";
#else
const char *__AXVersion = "<< RVL_SDK - AX \trelease build: Aug  8 2007 01:58:18 (0x4199_60831) >>";
#endif

static BOOL __init = FALSE;

void AXInit(void) { AXInitEx(0); }

void AXInitEx(u32 mode) {
  if (!__init) {
    OSRegisterVersion(__AXVersion);

    __AXAllocInit();
    __AXVPBInit();
    __AXSPBInit();
    __AXAuxInit();
    __AXClInit();
    __AXOutInit(mode);

    __init = TRUE;
  }
}

BOOL AXIsInit(void) { return __init; }
