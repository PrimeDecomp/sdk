#include <SDKVersion.h>
#include "dolphin/dsp.h"
#include "dolphin/os.h"

#include "dolphin/hw_regs.h"

#ifdef __cplusplus
extern "C" {
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
static const char* __DSPVersion =
    "<< Dolphin SDK - DSP\trelease build: Sep  5 2002 05:35:13 (0x2301) >>";
#else
#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
const char* __DSPVersion = "<< RVL_SDK - DSP \trelease build: Aug  8 2007 01:59:06 (0x4199_60831) >>";
#define DSP_BUILD_DATE "Aug  8 2007"
#define DSP_BUILD_TIME "01:59:06"
#else
static const char* __DSPVersion =
    "<< Dolphin SDK - DSP\trelease build: Apr  5 2004 04:15:32 (0x2301) >>";
#define DSP_BUILD_DATE "Apr  5 2004"
#define DSP_BUILD_TIME "04:15:32"
#endif
#endif

static s32 __DSP_init_flag = 0;
extern DSPTaskInfo* __DSP_tmp_task;
extern DSPTaskInfo* __DSP_last_task;
extern DSPTaskInfo* __DSP_first_task;
extern DSPTaskInfo* __DSP_curr_task;
extern DSPTaskInfo* __DSP_rude_task;
extern BOOL __DSP_rude_task_pending;

extern void __DSPHandler(__OSInterrupt, OSContext*);
extern void __DSP_debug_printf(const char* fmt, ...);
extern void __DSP_boot_task(DSPTaskInfo* task);

u32 DSPCheckMailToDSP(void) { return (__DSPRegs[0] >> 0xF) & 1; }

u32 DSPCheckMailFromDSP(void) { return (__DSPRegs[2] >> 0xF) & 1; }

u32 DSPReadMailFromDSP() {
  u16 reg1;
  u16 reg2;
  reg1 = __DSPRegs[2];
  reg2 = __DSPRegs[3];
  return reg1 << 16 | reg2;
}

void DSPSendMailToDSP(u32 mail) {
  __DSPRegs[0] = mail >> 16;
  __DSPRegs[1] = mail;
}

void DSPInit(void) {
  u32 oldInt;
  u16 reg;
#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  __DSP_debug_printf("DSPInit(): Build Date: %s %s\n", "Sep  5 2002", "05:35:13");
#else
  __DSP_debug_printf("DSPInit(): Build Date: %s %s\n", DSP_BUILD_DATE, DSP_BUILD_TIME);
#endif

  if (__DSP_init_flag == 1) {
    return;
  }
  OSRegisterVersion(__DSPVersion);
  oldInt = OSDisableInterrupts();
  __OSSetInterruptHandler(7, __DSPHandler);
  __OSUnmaskInterrupts(0x1000000);
  reg = __DSPRegs[5];
  reg = (reg & ~0xA8) | 0x800;
  __DSPRegs[5] = reg;
  reg = __DSPRegs[5];
  reg = reg & ~0xAC;
  __DSPRegs[5] = reg;
  __DSP_tmp_task = 0;
  __DSP_curr_task = 0;
  __DSP_last_task = 0;
  __DSP_first_task = 0;
  __DSP_init_flag = 1;
  OSRestoreInterrupts(oldInt);
}

void DSPReset(void) {
  u16 reg;
  u32 oldInt;
  oldInt = OSDisableInterrupts();
  reg = __DSPRegs[5];
  __DSPRegs[5] = (reg & ~0xA8) | 0x801;
  __DSP_init_flag = 0;
  OSRestoreInterrupts(oldInt);
}

void DSPHalt(void) {
  u16 reg;
  u32 oldInt;
  oldInt = OSDisableInterrupts();
  reg = __DSPRegs[5];
  __DSPRegs[5] = (reg & ~0xA8) | 4;
  OSRestoreInterrupts(oldInt);
}

u32 DSPGetDMAStatus(void) { return __DSPRegs[5] & 0x200; }

#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04
#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
BOOL DSPCheckInit(void) { return __DSP_init_flag; }
#endif

#endif
DSPTaskInfo* DSPAddTask(DSPTaskInfo* task) {
  u32 oldInt;
  oldInt = OSDisableInterrupts();
  __DSP_insert_task(task);
  task->state = 0;
  task->flags = 1;
  OSRestoreInterrupts(oldInt);
  if (task == __DSP_first_task) {
    __DSP_boot_task(task);
  }

  return task;
}

#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04
DSPTaskInfo* DSPCancelTask(DSPTaskInfo* task) {
  u32 oldInt;
  oldInt = OSDisableInterrupts();
  task->flags |= DSP_TASK_FLAG_CANCEL;
  OSRestoreInterrupts(oldInt);
  return task;
}

#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
DSPTaskInfo* DSPAssertTask(DSPTaskInfo* task) {
  u32 oldInt;
  u16 reg;
  oldInt = OSDisableInterrupts();
  if (__DSP_curr_task == task) {
    __DSP_rude_task_pending = TRUE;
    __DSP_rude_task = task;
    OSRestoreInterrupts(oldInt);
    return task;
  }
  if (task->priority < __DSP_curr_task->priority) {
    __DSP_rude_task_pending = TRUE;
    __DSP_rude_task = task;
    if (__DSP_curr_task->state == DSP_TASK_STATE_RUN) {
      u32 oldInt2 = OSDisableInterrupts();
      reg = __DSPRegs[5];
      __DSPRegs[5] = (reg & ~0xA8) | 2;
      OSRestoreInterrupts(oldInt2);
    }
    OSRestoreInterrupts(oldInt);
    return task;
  }
  OSRestoreInterrupts(oldInt);
  return NULL;
}
#endif

#endif
#ifdef __cplusplus
}
#endif
