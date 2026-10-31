#include <dolphin/ai.h>
#include <dolphin/hw_regs.h>
#include <dolphin/os.h>

const char *__AIVersion = "<< RVL_SDK - AI \trelease build: Aug  8 2007 01:58:12 (0x4199_60831) >>";

static AIDCallback __AID_Callback;
static u8 *__CallbackStack;
static u8 *__OldStack;

static OSTime bound_32KHz;
static OSTime bound_48KHz;
static OSTime min_wait;
static OSTime max_wait;
static OSTime buffer;

static volatile BOOL __AID_Active;
static volatile BOOL __AI_init_flag;

static void __AIDHandler(s16 interrupt, OSContext *context);
static void __AICallbackStackSwitch(register AIDCallback cb);
static void __AI_SRC_INIT(void);

AIDCallback AIRegisterDMACallback(AIDCallback callback) {
  BOOL enabled;
  AIDCallback old;

  old = __AID_Callback;
  enabled = OSDisableInterrupts();
  __AID_Callback = callback;
  OSRestoreInterrupts(enabled);
  return old;
}

void AIInitDMA(u32 addr, u32 length) {
  BOOL enabled = OSDisableInterrupts();

  __DSPRegs[24] = (u16)((__DSPRegs[24] & ~0x1FFF) | (addr >> 16));
  __DSPRegs[25] = (u16)((__DSPRegs[25] & ~0xFFE0) | (addr & 0xFFFF));
  __DSPRegs[27] = (u16)((__DSPRegs[27] & ~0x7FFF) | (u16)((length >> 5) & 0xFFFF));
  OSRestoreInterrupts(enabled);
}

void AIStartDMA(void) { __DSPRegs[27] |= 0x8000; }

void AIStopDMA(void) { __DSPRegs[27] &= ~0x8000; }

u32 AIGetDMABytesLeft(void) { return (u32)((__DSPRegs[29] & 0x7FFF) << 5); }

u32 AIGetDMAStartAddr(void) {
  return (u32)((__DSPRegs[24] & 0x1FFF) << 16) | (__DSPRegs[25] & 0xFFE0);
}

u32 AIGetDMALength(void) { return (u32)((__DSPRegs[27] & 0x7FFF) << 5); }

BOOL AICheckInit(void) { return __AI_init_flag; }

void AISetDSPSampleRate(u32 rate) {
  BOOL enabled;

  if (rate == AIGetDSPSampleRate()) {
    return;
  }

  __AIRegs[0] &= ~0x40;
  if (rate == 0) {
    enabled = OSDisableInterrupts();
    __AI_SRC_INIT();
    __AIRegs[0] |= 0x40;
    OSRestoreInterrupts(enabled);
  }
}

u32 AIGetDSPSampleRate(void) { return ((__AIRegs[0] >> 6) & 1) ^ 1; }

void AIInit(u8 *stack) {
  if (__AI_init_flag == TRUE) {
    return;
  }

  OSRegisterVersion(__AIVersion);
  bound_32KHz = OSNanosecondsToTicks(31524);
  bound_48KHz = OSNanosecondsToTicks(42024);
  min_wait = OSNanosecondsToTicks(42000);
  max_wait = OSNanosecondsToTicks(63000);
  buffer = OSNanosecondsToTicks(3000);

  __AIRegs[0] &= ~0x15;
  __AIRegs[1] = 0;
  __AIRegs[3] = 0;
  __AIRegs[0] = (__AIRegs[0] & ~0x20) | 0x20;
  AISetDSPSampleRate(0);
  __AID_Callback = NULL;
  __CallbackStack = stack;
  __OSSetInterruptHandler(5, __AIDHandler);
  __OSUnmaskInterrupts(0x04000000);
  __AI_init_flag = TRUE;
}

void AIReset(void) { __AI_init_flag = FALSE; }

static void __AIDHandler(s16 interrupt, OSContext *context) {
  OSContext tempContext;
  __DSPRegs[5] = (__DSPRegs[5] & ~0xA0) | 8;
  OSClearContext(&tempContext);
  OSSetCurrentContext(&tempContext);
  if (__AID_Callback && !__AID_Active) {
    __AID_Active = TRUE;
    if (__CallbackStack) {
      __AICallbackStackSwitch(__AID_Callback);
    } else {
      __AID_Callback();
    }

    __AID_Active = FALSE;
  }

  OSClearContext(&tempContext);
  OSSetCurrentContext(context);
}

// clang-format off
static asm void __AICallbackStackSwitch(register AIDCallback cb) {
  fralloc

  lis r5, __OldStack@ha
  addi r5, r5, __OldStack@l
  stw r1, 0(r5)

  lis r5, __CallbackStack@ha
  addi r5, r5, __CallbackStack@l
  lwz r1,0(r5)

  subi r1, r1, 8
  mtlr cb
  blrl

  lis r5, __OldStack@ha
  addi r5, r5, __OldStack@l
  lwz r1,0(r5)

  frfree

  blr
}
// clang-format on

static void __AI_SRC_INIT(void) {
  OSTime rise32 = 0;
  OSTime rise48 = 0;
  OSTime diff = 0;
  OSTime unused1 = 0;
  OSTime temp = 0;
  u32 temp0 = 0;
  u32 temp1 = 0;
  u32 done = 0;
  u32 walking = 0;
  u32 unused2 = 0;
  u32 initCnt = 0;

  walking = 0;
  initCnt = 0;
  temp = 0;

  while (!done) {
    __AIRegs[0] = (__AIRegs[0] & ~0x20) | 0x20;
    __AIRegs[0] &= ~2;
    __AIRegs[0] = (__AIRegs[0] & ~1) | 1;

    temp0 = __AIRegs[2] & ~0x80000000;

    while (temp0 == (__AIRegs[2] & ~0x80000000))
      ;
    rise32 = OSGetTime();

    __AIRegs[0] = (__AIRegs[0] & ~2) | 2;
    __AIRegs[0] = (__AIRegs[0] & ~1) | 1;

    temp1 = __AIRegs[2] & ~0x80000000;
    while (temp1 == (__AIRegs[2] & ~0x80000000))
      ;

    rise48 = OSGetTime();

    diff = rise48 - rise32;
    __AIRegs[0] &= ~2;
    __AIRegs[0] &= ~1;

    if (diff < (bound_32KHz - buffer)) {
      temp = min_wait;
      done = 1;
      ++initCnt;
    } else if (diff >= (bound_32KHz + buffer) && diff < (bound_48KHz - buffer)) {
      temp = max_wait;
      done = 1;
      ++initCnt;
    } else {
      done = 0;
      walking = 1;
      ++initCnt;
    }
  }

  while ((rise48 + temp) > OSGetTime())
    ;
}
