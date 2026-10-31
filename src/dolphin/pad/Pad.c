#ifdef PRIME1
#include <dolphin/pad.h>
#include <dolphin/si.h>
#include <dolphin/sipriv.h>
#else
#include "dolphin/pad.h"
#include "dolphin/os.h"
#include "dolphin/sipriv.h"
#include "dolphin/vifuncs.h"
#include "stddef.h"
#include "string.h"
#endif

#ifdef PRIME1
const char* __PADVersion = "<< Dolphin SDK - PAD\trelease build: Sep  5 2002 05:34:02 (0x2301) >>";
#else
u8 __gUnknown800030E3 : (OS_BASE_CACHED | 0x30E3);
u16 __OSWirelessPadFixMode : (OS_BASE_CACHED | 0x30E0);
#if VERSION < VERSION_R32J01
#if DEBUG
const char* __PADVersion = "<< Dolphin SDK - PAD\tdebug build: Apr  5 2004 03:56:05 (0x2301) >>";
#else
const char* __PADVersion = "<< Dolphin SDK - PAD\trelease build: Apr  5 2004 04:14:49 (0x2301) >>";
#endif
#endif
#endif

#ifdef PRIME1
u8 UnkVal : (OS_BASE_CACHED | 0x30e3);
u16 __OSWirelessPadFixMode : (OS_BASE_CACHED | 0x30E0);
#else
#define PAD_ALL                                                                                    \
  (PAD_BUTTON_LEFT | PAD_BUTTON_RIGHT | PAD_BUTTON_DOWN | PAD_BUTTON_UP | PAD_TRIGGER_Z |          \
   PAD_TRIGGER_R | PAD_TRIGGER_L | PAD_BUTTON_A | PAD_BUTTON_B | PAD_BUTTON_X | PAD_BUTTON_Y |     \
   PAD_BUTTON_MENU | 0x2000 | 0x0080)
#endif

#ifdef PRIME1
static void PADTypeAndStatusCallback(s32 chan, u32 type);
static void PADOriginCallback(s32 chan, u32 error, OSContext* context);
static void PADProbeCallback(s32 chan, u32 error, OSContext* context);
static void SPEC0_MakeStatus(s32 chan, PADStatus* status, u32 data[2]);
static void SPEC1_MakeStatus(s32 chan, PADStatus* status, u32 data[2]);
static void SPEC2_MakeStatus(s32 chan, PADStatus* status, u32 data[2]);
static void PADTypeAndStatusCallback(s32 chan, u32 type);

static void PADOriginCallback(s32 chan, u32 error, OSContext* context);
static void PADProbeCallback(s32 chan, u32 error, OSContext* context);

static void SPEC0_MakeStatus(s32 chan, PADStatus* status, u32 data[2]);
static void SPEC1_MakeStatus(s32 chan, PADStatus* status, u32 data[2]);
static void SPEC2_MakeStatus(s32 chan, PADStatus* status, u32 data[2]);
#else
static s32 ResettingChan = 0x20;
static u32 XPatchBits = PAD_CHAN0_BIT | PAD_CHAN1_BIT | PAD_CHAN2_BIT | PAD_CHAN3_BIT;
static u32 AnalogMode = 0x300;
static u32 Spec = PAD_SPEC_5;
#endif

static BOOL Initialized;
static u32 EnabledBits;
static u32 ResettingBits;
#ifdef PRIME1
static s32 ResettingChan = 32;
#endif
static u32 RecalibrateBits;
static u32 WaitingBits;
static u32 CheckingBits;
static u32 PendingBits;
#ifndef PRIME1
static u32 BarrelBits;
#endif

#ifdef PRIME1
static u32 XPatchBits = PAD_CHAN0_BIT | PAD_CHAN1_BIT | PAD_CHAN2_BIT | PAD_CHAN3_BIT;

static u32 AnalogMode = 0x00000300u;
#else
#if VERSION >= VERSION_R32J01
static PADStatus Origin[4];
static u32 Type[4];
#else
static u32 Type[4];
static PADStatus Origin[4];
#endif
#endif

u32 __PADSpec;
#ifdef PRIME1
static u32 Spec = 5;
static void (*MakeStatus)(s32, PADStatus*, u32[2]) = SPEC2_MakeStatus;
#endif

#ifdef PRIME1
static u32 Type[SI_MAX_CHAN];
static PADStatus Origin[SI_MAX_CHAN];
#else
// prototypes
static void PADTypeAndStatusCallback(s32 chan, u32 type);
static u16 GetWirelessID(s32 chan);
static void SetWirelessID(s32 chan, u16 id);
static void DoReset();
static void PADEnable(s32 chan);
static void ProbeWireless(s32 chan);
static void PADProbeCallback(s32 chan, u32 error, OSContext* context);
static void PADDisable(s32 chan);
static void UpdateOrigin(s32 chan);
static void PADOriginCallback(s32 chan, u32 error, OSContext* context);
static void PADFixCallback(s32 unused, u32 error, struct OSContext* context);
static void PADResetCallback(s32 unused, u32 error, struct OSContext* context);
static void PADReceiveCheckCallback(s32 chan, u32 error);
static void SPEC0_MakeStatus(s32 chan, PADStatus* status, u32 data[2]);
static void SPEC1_MakeStatus(s32 chan, PADStatus* status, u32 data[2]);
static s8 ClampS8(s8 var, s8 org);
static u8 ClampU8(u8 var, u8 org);
static void SPEC2_MakeStatus(s32 chan, PADStatus* status, u32 data[2]);
static BOOL OnReset(BOOL f);
void __PADDisableXPatch(void);
BOOL __PADDisableRumble(BOOL disable);
#endif

#ifdef PRIME1
static u32 CmdReadOrigin = 0x41 << 24;
static u32 CmdCalibrate = 0x42 << 24;
static u32 CmdProbeDevice[SI_MAX_CHAN];
#else
typedef void (*SPECCallback)(s32, PADStatus*, u32*);
static SPECCallback MakeStatus = SPEC2_MakeStatus;
#endif

#ifdef PRIME1
static BOOL OnReset(BOOL final);
#else
static u32 CmdTypeAndStatus;
static u32 CmdReadOrigin = 0x41000000;
static u32 CmdCalibrate = 0x42000000;
static u32 CmdProbeDevice[4];
#if VERSION >= VERSION_R32J01
static PADStatus Prev[4];
#endif
#endif

#ifdef PRIME1
static OSResetFunctionInfo ResetFunctionInfo = {OnReset, 127};

static void (*SamplingCallback)(void);
#else
static OSResetFunctionInfo ResetFunctionInfo = {
    OnReset,
    127,
    NULL,
    NULL,
};
#endif

static void PADEnable(s32 chan) {
  u32 cmd;
  u32 chanBit;
  u32 data[2];

  chanBit = PAD_CHAN0_BIT >> chan;
  EnabledBits |= chanBit;
#ifdef PRIME1
  SIGetResponse(chan, data);
  cmd = (0x40 << 16) | AnalogMode;
#else
  SIGetResponse(chan, &data);
  cmd = (AnalogMode | 0x400000);
#endif
  SISetCommand(chan, cmd);
  SIEnablePolling(EnabledBits);
}

static void PADDisable(s32 chan) {
  BOOL enabled;
  u32 chanBit;

  enabled = OSDisableInterrupts();
  chanBit = PAD_CHAN0_BIT >> chan;
  SIDisablePolling(chanBit);
  EnabledBits &= ~chanBit;
  WaitingBits &= ~chanBit;
  CheckingBits &= ~chanBit;
  PendingBits &= ~chanBit;
#ifndef PRIME1
  BarrelBits &= ~chanBit;
#endif
  OSSetWirelessID(chan, 0);
  OSRestoreInterrupts(enabled);
}

#ifdef PRIME1
static void DoReset(void) {
#else
static void DoReset() {
#endif
  u32 chanBit;

  ResettingChan = __cntlzw(ResettingBits);
#ifdef PRIME1
  if (ResettingChan == 32) {
    return;
#else
  if (ResettingChan != 32) {
    ASSERT(0 <= ResettingChan && ResettingChan < SI_MAX_CHAN);
    chanBit = (PAD_CHAN0_BIT >> ResettingChan);
    ResettingBits &= ~chanBit;

    memset(&Origin[ResettingChan], 0, sizeof(PADStatus));
    SIGetTypeAsync(ResettingChan, PADTypeAndStatusCallback);
#endif
  }
#ifdef PRIME1

  chanBit = PAD_CHAN0_BIT >> ResettingChan;
  ResettingBits &= ~chanBit;

  memset(&Origin[ResettingChan], 0, sizeof(PADStatus));
  SIGetTypeAsync(ResettingChan, PADTypeAndStatusCallback);
#endif
}

static void UpdateOrigin(s32 chan) {
  PADStatus* origin;
  u32 chanBit = PAD_CHAN0_BIT >> chan;

  origin = &Origin[chan];
  switch (AnalogMode & 0x00000700u) {
  case 0x00000000u:
  case 0x00000500u:
  case 0x00000600u:
  case 0x00000700u:
    origin->triggerLeft &= ~15;
    origin->triggerRight &= ~15;
    origin->analogA &= ~15;
    origin->analogB &= ~15;
    break;
  case 0x00000100u:
    origin->substickX &= ~15;
    origin->substickY &= ~15;
    origin->analogA &= ~15;
    origin->analogB &= ~15;
    break;
  case 0x00000200u:
    origin->substickX &= ~15;
    origin->substickY &= ~15;
    origin->triggerLeft &= ~15;
    origin->triggerRight &= ~15;
    break;
  case 0x00000300u:
    break;
  case 0x00000400u:
    break;
  }

  origin->stickX -= 128;
  origin->stickY -= 128;
  origin->substickX -= 128;
  origin->substickY -= 128;

  if (XPatchBits & chanBit) {
#ifdef PRIME1
    if (64 < origin->stickX && (SIGetType(chan) & 0xffff0000) == SI_GC_CONTROLLER) {
#else
    if (64 < origin->stickX && (SIGetType(chan) & 0xFFFF0000) == SI_GC_CONTROLLER) {
#endif
      origin->stickX = 0;
    }
  }
}

static void PADOriginCallback(s32 chan, u32 error, OSContext* context) {
#ifndef PRIME1
  ASSERT(0 <= ResettingChan && ResettingChan < SI_MAX_CHAN);
  ASSERT(chan == ResettingChan);

#endif
  if (!(error &
        (SI_ERROR_UNDER_RUN | SI_ERROR_OVER_RUN | SI_ERROR_NO_RESPONSE | SI_ERROR_COLLISION))) {
    UpdateOrigin(ResettingChan);
    PADEnable(ResettingChan);
  }
  DoReset();
}

static void PADOriginUpdateCallback(s32 chan, u32 error, OSContext* context) {
#ifdef PRIME1

  if (!(EnabledBits & (PAD_CHAN0_BIT >> chan))) {
#else
  ASSERT(0 <= chan && chan < SI_MAX_CHAN);
  if (!(EnabledBits & (PAD_CHAN0_BIT >> chan)))
#endif
    return;
#ifdef PRIME1
  }

#endif
  if (!(error &
#ifdef PRIME1
        (SI_ERROR_UNDER_RUN | SI_ERROR_OVER_RUN | SI_ERROR_NO_RESPONSE | SI_ERROR_COLLISION))) {
#else
        (SI_ERROR_UNDER_RUN | SI_ERROR_OVER_RUN | SI_ERROR_NO_RESPONSE | SI_ERROR_COLLISION)))
#endif
    UpdateOrigin(chan);
#ifdef PRIME1
  }

#endif
  if (error & SI_ERROR_NO_RESPONSE) {
    PADDisable(chan);
  }
}

static void PADProbeCallback(s32 chan, u32 error, OSContext* context) {
#ifndef PRIME1
  u32 type;
  ASSERT(0 <= ResettingChan && ResettingChan < SI_MAX_CHAN);
  ASSERT(chan == ResettingChan);
  ASSERT((Type[chan] & SI_WIRELESS_CONT_MASK) == SI_WIRELESS_CONT &&
         !(Type[chan] & SI_WIRELESS_LITE));

#endif
  if (!(error &
        (SI_ERROR_UNDER_RUN | SI_ERROR_OVER_RUN | SI_ERROR_NO_RESPONSE | SI_ERROR_COLLISION))) {
    PADEnable(ResettingChan);
    WaitingBits |= PAD_CHAN0_BIT >> ResettingChan;
  }
  DoReset();
}

static void PADTypeAndStatusCallback(s32 chan, u32 type) {
  u32 chanBit;
  u32 recalibrate;
  BOOL rc = TRUE;
  u32 error;
#ifndef PRIME1

  ASSERT(0 <= ResettingChan && ResettingChan < SI_MAX_CHAN);
  ASSERT(chan == ResettingChan);

#endif
  chanBit = PAD_CHAN0_BIT >> ResettingChan;
  error = type & 0xFF;
#ifndef PRIME1
  ASSERT(!(error & SI_ERROR_BUSY));

#endif
  recalibrate = RecalibrateBits & chanBit;
  RecalibrateBits &= ~chanBit;

  if (error &
      (SI_ERROR_UNDER_RUN | SI_ERROR_OVER_RUN | SI_ERROR_NO_RESPONSE | SI_ERROR_COLLISION)) {
    DoReset();
    return;
  }

  type &= ~0xFF;
  Type[ResettingChan] = type;

  if ((type & SI_TYPE_MASK) != SI_TYPE_GC || !(type & SI_GC_STANDARD)) {
    DoReset();
    return;
  }

  if (Spec < PAD_SPEC_2) {
    PADEnable(ResettingChan);
    DoReset();
    return;
  }

  if (!(type & SI_GC_WIRELESS) || (type & SI_WIRELESS_IR)) {
    if (recalibrate) {
      rc = SITransfer(ResettingChan, &CmdCalibrate, 3, &Origin[ResettingChan], 10,
                      PADOriginCallback, 0);
    } else {
      rc = SITransfer(ResettingChan, &CmdReadOrigin, 1, &Origin[ResettingChan], 10,
                      PADOriginCallback, 0);
    }
  } else if ((type & SI_WIRELESS_FIX_ID) && (type & SI_WIRELESS_CONT_MASK) == SI_WIRELESS_CONT &&
             !(type & SI_WIRELESS_LITE)) {
    if (type & SI_WIRELESS_RECEIVED) {
      rc = SITransfer(ResettingChan, &CmdReadOrigin, 1, &Origin[ResettingChan], 10,
                      PADOriginCallback, 0);
    } else {
      rc = SITransfer(ResettingChan, &CmdProbeDevice[ResettingChan], 3, &Origin[ResettingChan], 8,
                      PADProbeCallback, 0);
    }
  }
  if (!rc) {
    PendingBits |= chanBit;
    DoReset();
    return;
  }
}

static void PADReceiveCheckCallback(s32 chan, u32 type) {
  u32 error;
  u32 chanBit;

  chanBit = PAD_CHAN0_BIT >> chan;
#ifdef PRIME1
  if (!(EnabledBits & chanBit)) {
    return;
  }
#endif

#ifdef PRIME1
  error = type & 0xFF;
  type &= ~0xFF;
#else
  if (EnabledBits & chanBit) {
    error = type & 0xFF;
    type &= ~0xFF;
#endif

    WaitingBits &= ~chanBit;
    CheckingBits &= ~chanBit;

#ifdef PRIME1
  if (!(error &
        (SI_ERROR_UNDER_RUN | SI_ERROR_OVER_RUN | SI_ERROR_NO_RESPONSE | SI_ERROR_COLLISION)) &&
      (type & SI_GC_WIRELESS) && (type & SI_WIRELESS_FIX_ID) && (type & SI_WIRELESS_RECEIVED) &&
      !(type & SI_WIRELESS_IR) && (type & SI_WIRELESS_CONT_MASK) == SI_WIRELESS_CONT &&
      !(type & SI_WIRELESS_LITE)) {
    SITransfer(chan, &CmdReadOrigin, 1, &Origin[chan], 10, PADOriginUpdateCallback, 0);
  } else {
    PADDisable(chan);
#else
    if (!(error &
          (SI_ERROR_UNDER_RUN | SI_ERROR_OVER_RUN | SI_ERROR_NO_RESPONSE | SI_ERROR_COLLISION)) &&
        (type & SI_GC_WIRELESS) && (type & SI_WIRELESS_FIX_ID) && (type & SI_WIRELESS_RECEIVED) &&
        !(type & SI_WIRELESS_IR) && (type & SI_WIRELESS_CONT_MASK) == SI_WIRELESS_CONT &&
        !(type & SI_WIRELESS_LITE)) {
      SITransfer(chan, &CmdReadOrigin, 1, &Origin[chan], 10, PADOriginUpdateCallback, 0);
    } else {
      PADDisable(chan);
    }
#endif
  }
}

#ifdef PRIME1
BOOL PADReset(u32 mask) {
#else
int PADReset(u32 mask) {
#endif
  BOOL enabled;
#ifdef PRIME1
  u32 diableBits;
#else
  u32 disableBits;

  ASSERTMSG(!(mask & 0x0FFFFFFF), "PADReset(): invalid mask");
#endif

  enabled = OSDisableInterrupts();
  mask |= PendingBits;
  PendingBits = 0;
  mask &= ~(WaitingBits | CheckingBits);
  ResettingBits |= mask;
#ifdef PRIME1
  diableBits = ResettingBits & EnabledBits;
#else
  disableBits = ResettingBits & EnabledBits;
#endif
  EnabledBits &= ~mask;
#ifndef PRIME1
  BarrelBits &= ~mask;
#endif

#ifdef PRIME1
  if (Spec == PAD_SPEC_4) {
#else
  if (Spec == 4) {
#endif
    RecalibrateBits |= mask;
  }

#ifdef PRIME1
  SIDisablePolling(diableBits);
#else
  SIDisablePolling(disableBits);
#endif

#ifdef PRIME1
  if (ResettingChan == 32) {
#else
  if (ResettingChan == 0x20) {
#endif
    DoReset();
  }
  OSRestoreInterrupts(enabled);
#ifdef PRIME1
  return TRUE;
#else
  return 1;
#endif
}

BOOL PADRecalibrate(u32 mask) {
  BOOL enabled;
  u32 disableBits;

#ifndef PRIME1
  ASSERTMSG(!(mask & 0x0FFFFFFF), "PADReset(): invalid mask");
#endif
  enabled = OSDisableInterrupts();

  mask |= PendingBits;
  PendingBits = 0;
  mask &= ~(WaitingBits | CheckingBits);
  ResettingBits |= mask;
  disableBits = ResettingBits & EnabledBits;
  EnabledBits &= ~mask;
#ifndef PRIME1
  BarrelBits &= ~mask;
#endif

#ifdef PRIME1
  if (!(UnkVal & 0x40)) {
#else
  if (!(__gUnknown800030E3 & 0x40)) {
#endif
    RecalibrateBits |= mask;
  }

  SIDisablePolling(disableBits);
#ifdef PRIME1
  if (ResettingChan == 32) {
#else
  if (ResettingChan == 32)
#endif
    DoReset();
#ifdef PRIME1
  }
#else

#endif
  OSRestoreInterrupts(enabled);
#ifdef PRIME1
  return TRUE;
#else
  return 1;
#endif
}

BOOL PADInit() {
  s32 chan;
  if (Initialized) {
#ifdef PRIME1
    return TRUE;
#else
    return 1;
#endif
  }

#ifdef PRIME1
  OSRegisterVersion(__PADVersion);
#else
#if VERSION < VERSION_R32J01
  OSRegisterVersion(__PADVersion);
#endif
#endif

#ifdef PRIME1
  if (__PADSpec) {
#else
  if (__PADSpec)
#endif
    PADSetSpec(__PADSpec);
#ifdef PRIME1
  }
#endif

  Initialized = TRUE;

  if (__PADFixBits != 0) {
    OSTime time = OSGetTime();
    __OSWirelessPadFixMode = (u16)((((time) & 0xffff) + ((time >> 16) & 0xffff) +
                                    ((time >> 32) & 0xffff) + ((time >> 48) & 0xffff)) &
                                   0x3fffu);
    RecalibrateBits = PAD_CHAN0_BIT | PAD_CHAN1_BIT | PAD_CHAN2_BIT | PAD_CHAN3_BIT;
  }

  for (chan = 0; chan < SI_MAX_CHAN; ++chan) {
    CmdProbeDevice[chan] = (0x4D << 24) | (chan << 22) | ((__OSWirelessPadFixMode & 0x3fffu) << 8);
  }

  SIRefreshSamplingRate();
  OSRegisterResetFunction(&ResetFunctionInfo);

  return PADReset(PAD_CHAN0_BIT | PAD_CHAN1_BIT | PAD_CHAN2_BIT | PAD_CHAN3_BIT);
}

#ifdef PRIME1
#define offsetof(type, memb) ((u32) & ((type*)0)->memb)

#endif
u32 PADRead(PADStatus* status) {
  BOOL enabled;
  s32 chan;
  u32 data[2];
  u32 chanBit;
  u32 sr;
  int chanShift;
  u32 motor;

  enabled = OSDisableInterrupts();
#ifndef PRIME1
  motor = 0;
#endif

#ifdef PRIME1
  motor = 0;
  for (chan = 0; chan < SI_MAX_CHAN; chan++, status++) {
#else
  for (chan = 0; chan < 4; chan++, status++) {
#endif
    chanBit = PAD_CHAN0_BIT >> chan;
    chanShift = 8 * (SI_MAX_CHAN - 1 - chan);

    if (PendingBits & chanBit) {
      PADReset(0);
      status->err = PAD_ERR_NOT_READY;
      memset(status, 0, offsetof(PADStatus, err));
#ifdef PRIME1
      continue;
    }

    if ((ResettingBits & chanBit) || ResettingChan == chan) {
#else
    } else if ((ResettingBits & chanBit) || ResettingChan == chan) {
#endif
      status->err = PAD_ERR_NOT_READY;
      memset(status, 0, offsetof(PADStatus, err));
#ifdef PRIME1
      continue;
    }

    if (!(EnabledBits & chanBit)) {
      status->err = (s8)PAD_ERR_NO_CONTROLLER;
#else
    } else if (!(EnabledBits & chanBit)) {
      status->err = PAD_ERR_NO_CONTROLLER;
#endif
      memset(status, 0, offsetof(PADStatus, err));
#ifdef PRIME1
      continue;
    }

    if (SIIsChanBusy(chan)) {
#else
    } else if (SIIsChanBusy(chan)) {
#endif
      status->err = PAD_ERR_TRANSFER;
      memset(status, 0, offsetof(PADStatus, err));
#ifdef PRIME1
      continue;
#else
    } else {
      sr = SIGetStatus(chan);
      if (sr & SI_ERROR_NO_RESPONSE) {
        SIGetResponse(chan, data);

        if (WaitingBits & chanBit) {
          status->err = PAD_ERR_NONE;
          memset(status, 0, offsetof(PADStatus, err));

          if (!(CheckingBits & chanBit)) {
            CheckingBits |= chanBit;
            SIGetTypeAsync(chan, PADReceiveCheckCallback);
          }
        } else {
          PADDisable(chan);
          status->err = PAD_ERR_NO_CONTROLLER;
          memset(status, 0, offsetof(PADStatus, err));
        }
      } else {
        if (!(SIGetType(chan) & SI_GC_NOMOTOR)) {
          motor |= chanBit;
        }

        if (!SIGetResponse(chan, &data)) {
          status->err = PAD_ERR_TRANSFER;
          memset(status, 0, offsetof(PADStatus, err));
        } else if (data[0] & 0x80000000) {
          status->err = PAD_ERR_TRANSFER;
          memset(status, 0, offsetof(PADStatus, err));
        } else {
          MakeStatus(chan, status, data);

#if VERSION >= VERSION_R32J01
          // Any noticeable stick, trigger or button change counts as user activity.
          {
            s32 threshold;

            if ((Type[chan] & 0xFFFF0000) == SI_GC_CONTROLLER &&
                ((status->button & 0x0080) ^ 0x0080)) {
              threshold = 10;
            } else {
              threshold = 3;
            }

            if (__abs(__abs(status->stickX) - __abs(Prev[chan].stickX)) >= threshold ||
                __abs(__abs(status->stickY) - __abs(Prev[chan].stickY)) >= threshold ||
                __abs(__abs(status->substickX) - __abs(Prev[chan].substickX)) >= threshold ||
                __abs(__abs(status->substickY) - __abs(Prev[chan].substickY)) >= threshold ||
                __abs(__abs(status->triggerLeft) - __abs(Prev[chan].triggerLeft)) >= threshold ||
                __abs(__abs(status->triggerRight) - __abs(Prev[chan].triggerRight)) >= threshold ||
                status->button != Prev[chan].button) {
              __VIResetSIIdle();
            }
            memcpy(&Prev[chan], status, sizeof(PADStatus));
          }
#endif

          // Check and clear PAD_ORIGIN bit
          if (status->button & 0x2000) {
            status->err = PAD_ERR_TRANSFER;
            memset(status, 0, offsetof(PADStatus, err));

            // Get origin. It is okay if the following transfer fails
            // since the PAD_ORIGIN bit remains until the read origin
            // command complete.
            SITransfer(chan, &CmdReadOrigin, 1, &Origin[chan], 10, PADOriginUpdateCallback, 0);
          } else {
            status->err = PAD_ERR_NONE;

            // Clear PAD_INTERFERE bit
            status->button &= ~0x0080;
          }
        }
      }
#endif
    }
#ifdef PRIME1

    sr = SIGetStatus(chan);
    if (sr & SI_ERROR_NO_RESPONSE) {
      SIGetResponse(chan, data);

      if (WaitingBits & chanBit) {
        status->err = (s8)PAD_ERR_NONE;
        memset(status, 0, offsetof(PADStatus, err));

        if (!(CheckingBits & chanBit)) {
          CheckingBits |= chanBit;
          SIGetTypeAsync(chan, PADReceiveCheckCallback);
        }
        continue;
      }

      PADDisable(chan);

      status->err = (s8)PAD_ERR_NO_CONTROLLER;
      memset(status, 0, offsetof(PADStatus, err));
      continue;
    }

    if (!(SIGetType(chan) & SI_GC_NOMOTOR)) {
      motor |= chanBit;
    }

    if (!SIGetResponse(chan, data)) {
      status->err = PAD_ERR_TRANSFER;
      memset(status, 0, offsetof(PADStatus, err));
      continue;
    }

    if (data[0] & 0x80000000) {
      status->err = PAD_ERR_TRANSFER;
      memset(status, 0, offsetof(PADStatus, err));
      continue;
    }

    MakeStatus(chan, status, data);

    // Check and clear PAD_ORIGIN bit
    if (status->button & 0x2000) {
      status->err = PAD_ERR_TRANSFER;
      memset(status, 0, offsetof(PADStatus, err));

      // Get origin. It is okay if the following transfer fails
      // since the PAD_ORIGIN bit remains until the read origin
      // command complete.
      SITransfer(chan, &CmdReadOrigin, 1, &Origin[chan], 10, PADOriginUpdateCallback, 0);
      continue;
    }

    status->err = PAD_ERR_NONE;

    // Clear PAD_INTERFERE bit
    status->button &= ~0x0080;
#endif
  }

  OSRestoreInterrupts(enabled);
  return motor;
}
#ifndef PRIME1

typedef struct XY {
  u8 line;
  u8 count;
} XY;

void PADSetSamplingRate(u32 msec) { SISetSamplingRate(msec); }

#if DEBUG
void __PADTestSamplingRate(u32 tvmode) { __SITestSamplingRate(tvmode); }
#endif
#endif

void PADControlAllMotors(const u32* commandArray) {
  BOOL enabled;
  int chan;
  u32 command;
  BOOL commit;
  u32 chanBit;

  enabled = OSDisableInterrupts();
  commit = FALSE;
  for (chan = 0; chan < SI_MAX_CHAN; chan++, commandArray++) {
    chanBit = PAD_CHAN0_BIT >> chan;
#ifdef PRIME1
    if ((EnabledBits & chanBit) && !(SIGetType(chan) & SI_GC_NOMOTOR)) {
#else
    if ((EnabledBits & chanBit) && !(SIGetType(chan) & 0x20000000)) {
#endif
      command = *commandArray;
#ifdef PRIME1
      if (Spec < PAD_SPEC_2 && command == PAD_MOTOR_STOP_HARD) {
#else
      ASSERTMSG(!(command & 0xFFFFFFFC), "PADControlAllMotors(): invalid command");
      if (Spec < PAD_SPEC_2 && command == PAD_MOTOR_STOP_HARD)
#endif
        command = PAD_MOTOR_STOP;
#ifdef PRIME1
      }

#else
      if (__gUnknown800030E3 & 0x20)
        command = PAD_MOTOR_STOP;
#endif
      SISetCommand(chan, (0x40 << 16) | AnalogMode | (command & (0x00000001 | 0x00000002)));
      commit = TRUE;
    }
  }
#ifdef PRIME1
  if (commit) {
#else

  if (commit)
#endif
    SITransferCommands();
#ifdef PRIME1
  }
#else

#endif
  OSRestoreInterrupts(enabled);
}

void PADControlMotor(s32 chan, u32 command) {
  BOOL enabled;
  u32 chanBit;

#ifndef PRIME1
  ASSERTMSG(!(command & 0xFFFFFFFC), "PADControlMotor(): invalid command");

#endif
  enabled = OSDisableInterrupts();
  chanBit = PAD_CHAN0_BIT >> chan;
  if ((EnabledBits & chanBit) && !(SIGetType(chan) & SI_GC_NOMOTOR)) {
#ifdef PRIME1
    if (Spec < PAD_SPEC_2 && command == PAD_MOTOR_STOP_HARD) {
#else
    if (Spec < PAD_SPEC_2 && command == PAD_MOTOR_STOP_HARD)
#endif
      command = PAD_MOTOR_STOP;
#ifdef PRIME1
    }

#else
    if (__gUnknown800030E3 & 0x20)
      command = PAD_MOTOR_STOP;
#endif
    SISetCommand(chan, (0x40 << 16) | AnalogMode | (command & (0x00000001 | 0x00000002)));
    SITransferCommands();
  }
  OSRestoreInterrupts(enabled);
}

void PADSetSpec(u32 spec) {
#ifndef PRIME1
  ASSERT(!Initialized);
#endif
  __PADSpec = 0;
  switch (spec) {
  case PAD_SPEC_0:
    MakeStatus = SPEC0_MakeStatus;
    break;
  case PAD_SPEC_1:
    MakeStatus = SPEC1_MakeStatus;
    break;
  case PAD_SPEC_2:
  case PAD_SPEC_3:
  case PAD_SPEC_4:
  case PAD_SPEC_5:
    MakeStatus = SPEC2_MakeStatus;
    break;
  }
  Spec = spec;
}

u32 PADGetSpec(void) { return Spec; }

static void SPEC0_MakeStatus(s32 chan, PADStatus* status, u32 data[2]) {
  status->button = 0;
  status->button |= ((data[0] >> 16) & 0x0008) ? PAD_BUTTON_A : 0;
  status->button |= ((data[0] >> 16) & 0x0020) ? PAD_BUTTON_B : 0;
  status->button |= ((data[0] >> 16) & 0x0100) ? PAD_BUTTON_X : 0;
  status->button |= ((data[0] >> 16) & 0x0001) ? PAD_BUTTON_Y : 0;
  status->button |= ((data[0] >> 16) & 0x0010) ? PAD_BUTTON_START : 0;
  status->stickX = (s8)(data[1] >> 16);
  status->stickY = (s8)(data[1] >> 24);
  status->substickX = (s8)(data[1]);
  status->substickY = (s8)(data[1] >> 8);
  status->triggerLeft = (u8)(data[0] >> 8);
  status->triggerRight = (u8)data[0];
  status->analogA = 0;
  status->analogB = 0;
#ifdef PRIME1
  if (170 <= status->triggerLeft) {
#else
  if (170 <= status->triggerLeft)
#endif
    status->button |= PAD_TRIGGER_L;
#ifdef PRIME1
  }
  if (170 <= status->triggerRight) {
#else
  if (170 <= status->triggerRight)
#endif
    status->button |= PAD_TRIGGER_R;
#ifdef PRIME1
  }
#endif
  status->stickX -= 128;
  status->stickY -= 128;
  status->substickX -= 128;
  status->substickY -= 128;
}

static void SPEC1_MakeStatus(s32 chan, PADStatus* status, u32 data[2]) {
  status->button = 0;
  status->button |= ((data[0] >> 16) & 0x0080) ? PAD_BUTTON_A : 0;
  status->button |= ((data[0] >> 16) & 0x0100) ? PAD_BUTTON_B : 0;
  status->button |= ((data[0] >> 16) & 0x0020) ? PAD_BUTTON_X : 0;
  status->button |= ((data[0] >> 16) & 0x0010) ? PAD_BUTTON_Y : 0;
  status->button |= ((data[0] >> 16) & 0x0200) ? PAD_BUTTON_START : 0;

  status->stickX = (s8)(data[1] >> 16);
  status->stickY = (s8)(data[1] >> 24);
  status->substickX = (s8)(data[1]);
  status->substickY = (s8)(data[1] >> 8);

  status->triggerLeft = (u8)(data[0] >> 8);
  status->triggerRight = (u8)data[0];

  status->analogA = 0;
  status->analogB = 0;

#ifdef PRIME1
  if (170 <= status->triggerLeft) {
#else
  if (170 <= status->triggerLeft)
#endif
    status->button |= PAD_TRIGGER_L;
#ifdef PRIME1
  }
  if (170 <= status->triggerRight) {
#else
  if (170 <= status->triggerRight)
#endif
    status->button |= PAD_TRIGGER_R;
#ifdef PRIME1
  }
#endif

  status->stickX -= 128;
  status->stickY -= 128;
  status->substickX -= 128;
  status->substickY -= 128;
}

static s8 ClampS8(s8 var, s8 org) {
  if (0 < org) {
    s8 min = (s8)(-128 + org);
#ifdef PRIME1
    if (var < min) {
#else
    if (var < min)
#endif
      var = min;
#ifdef PRIME1
    }
#endif
  } else if (org < 0) {
    s8 max = (s8)(127 + org);
#ifdef PRIME1
    if (max < var) {
#else
    if (max < var)
#endif
      var = max;
#ifdef PRIME1
    }
#endif
  }
  return var -= org;
}

static u8 ClampU8(u8 var, u8 org) {
#ifdef PRIME1
  if (var < org) {
#else
  if (var < org)
#endif
    var = org;
#ifdef PRIME1
  }
#endif
  return var -= org;
}
#ifdef PRIME1

#define PAD_ALL                                                                                    \
  (PAD_BUTTON_LEFT | PAD_BUTTON_RIGHT | PAD_BUTTON_DOWN | PAD_BUTTON_UP | PAD_TRIGGER_Z |          \
   PAD_TRIGGER_R | PAD_TRIGGER_L | PAD_BUTTON_A | PAD_BUTTON_B | PAD_BUTTON_X | PAD_BUTTON_Y |     \
   PAD_BUTTON_MENU | 0x2000 | 0x0080)
#endif

static void SPEC2_MakeStatus(s32 chan, PADStatus* status, u32 data[2]) {
  PADStatus* origin;

  status->button = (u16)((data[0] >> 16) & PAD_ALL);
  status->stickX = (s8)(data[0] >> 8);
  status->stickY = (s8)(data[0]);

  switch (AnalogMode & 0x00000700) {
  case 0x00000000:
  case 0x00000500:
  case 0x00000600:
  case 0x00000700:
    status->substickX = (s8)(data[1] >> 24);
    status->substickY = (s8)(data[1] >> 16);
    status->triggerLeft = (u8)(((data[1] >> 12) & 0x0f) << 4);
    status->triggerRight = (u8)(((data[1] >> 8) & 0x0f) << 4);
    status->analogA = (u8)(((data[1] >> 4) & 0x0f) << 4);
    status->analogB = (u8)(((data[1] >> 0) & 0x0f) << 4);
    break;
  case 0x00000100:
    status->substickX = (s8)(((data[1] >> 28) & 0x0f) << 4);
    status->substickY = (s8)(((data[1] >> 24) & 0x0f) << 4);
    status->triggerLeft = (u8)(data[1] >> 16);
    status->triggerRight = (u8)(data[1] >> 8);
    status->analogA = (u8)(((data[1] >> 4) & 0x0f) << 4);
    status->analogB = (u8)(((data[1] >> 0) & 0x0f) << 4);
    break;
  case 0x00000200:
    status->substickX = (s8)(((data[1] >> 28) & 0x0f) << 4);
    status->substickY = (s8)(((data[1] >> 24) & 0x0f) << 4);
    status->triggerLeft = (u8)(((data[1] >> 20) & 0x0f) << 4);
    status->triggerRight = (u8)(((data[1] >> 16) & 0x0f) << 4);
    status->analogA = (u8)(data[1] >> 8);
    status->analogB = (u8)(data[1] >> 0);
    break;
  case 0x00000300:
    status->substickX = (s8)(data[1] >> 24);
    status->substickY = (s8)(data[1] >> 16);
    status->triggerLeft = (u8)(data[1] >> 8);
    status->triggerRight = (u8)(data[1] >> 0);
    status->analogA = 0;
    status->analogB = 0;
    break;
  case 0x00000400:
    status->substickX = (s8)(data[1] >> 24);
    status->substickY = (s8)(data[1] >> 16);
    status->triggerLeft = 0;
    status->triggerRight = 0;
    status->analogA = (u8)(data[1] >> 8);
    status->analogB = (u8)(data[1] >> 0);
    break;
  }

  status->stickX -= 128;
  status->stickY -= 128;
  status->substickX -= 128;
  status->substickY -= 128;

#ifndef PRIME1
  if (((Type[chan] & (0xFFFF0000)) == SI_GC_CONTROLLER) && ((status->button & 0x80) ^ 0x80)) {
    BarrelBits |= (PAD_CHAN0_BIT >> chan);
    status->stickX = 0;
    status->stickY = 0;
    status->substickX = 0;
    status->substickY = 0;
    return;
  } else {
    BarrelBits &= ~(PAD_CHAN0_BIT >> chan);
  }

#endif
  origin = &Origin[chan];
  status->stickX = ClampS8(status->stickX, origin->stickX);
  status->stickY = ClampS8(status->stickY, origin->stickY);
  status->substickX = ClampS8(status->substickX, origin->substickX);
  status->substickY = ClampS8(status->substickY, origin->substickY);
  status->triggerLeft = ClampU8(status->triggerLeft, origin->triggerLeft);
  status->triggerRight = ClampU8(status->triggerRight, origin->triggerRight);
}

#ifdef PRIME1
BOOL PADGetType(s32 chan, u32* type) {
#else
int PADGetType(s32 chan, u32* type) {
#endif
  u32 chanBit;

  *type = SIGetType(chan);
  chanBit = PAD_CHAN0_BIT >> chan;
#ifdef PRIME1
  if ((ResettingBits & chanBit) || ResettingChan == chan || !(EnabledBits & chanBit)) {
    return FALSE;
#else
  if (ResettingBits & chanBit || ResettingChan == chan || !(EnabledBits & chanBit)) {
    return 0;
#endif
  }
#ifdef PRIME1
  return TRUE;
#else
  return 1;
#endif
}

#ifdef PRIME1
BOOL PADSync(void) { return ResettingBits == 0 && ResettingChan == 32 && !SIBusy(); }
#else
BOOL PADSync(void) { return ResettingBits == 0 && (s32)ResettingChan == 32 && !SIBusy(); }
#endif

void PADSetAnalogMode(u32 mode) {
  BOOL enabled;
  u32 mask;
#ifndef PRIME1

  ASSERTMSG((mode < 8), "PADSetAnalogMode(): invalid mode");
#endif

  enabled = OSDisableInterrupts();
  AnalogMode = mode << 8;
  mask = EnabledBits;

  EnabledBits &= ~mask;
  WaitingBits &= ~mask;
  CheckingBits &= ~mask;

  SIDisablePolling(mask);
  OSRestoreInterrupts(enabled);
}

#ifdef PRIME1
static BOOL OnReset(BOOL f) {
#else
static void (*SamplingCallback)();

static BOOL OnReset(BOOL final) {
  BOOL sync;
#endif
  static BOOL recalibrated = FALSE;
#ifdef PRIME1
  BOOL sync;
#endif

#ifdef PRIME1
  if (SamplingCallback) {
#else
  if (SamplingCallback)
#endif
    PADSetSamplingCallback(NULL);
#ifdef PRIME1
  }
#endif

#ifdef PRIME1
  if (!f) {
#else
  if (!final) {
#endif
    sync = PADSync();
    if (!recalibrated && sync) {
      recalibrated = PADRecalibrate(PAD_CHAN0_BIT | PAD_CHAN1_BIT | PAD_CHAN2_BIT | PAD_CHAN3_BIT);
      return FALSE;
    }
    return sync;
#ifdef PRIME1
  } else {
#else
  } else
#endif
    recalibrated = FALSE;
#ifdef PRIME1
  }
#endif

  return TRUE;
}

void __PADDisableXPatch(void) { XPatchBits = 0; }

static void SamplingHandler(__OSInterrupt interrupt, OSContext* context) {
  OSContext exceptionContext;

  if (SamplingCallback) {
    OSClearContext(&exceptionContext);
    OSSetCurrentContext(&exceptionContext);
    SamplingCallback();
    OSClearContext(&exceptionContext);
    OSSetCurrentContext(context);
  }
}

PADSamplingCallback PADSetSamplingCallback(PADSamplingCallback callback) {
  PADSamplingCallback prev;

  prev = SamplingCallback;
  SamplingCallback = callback;
  if (callback) {
    SIRegisterPollingHandler(SamplingHandler);
  } else {
    SIUnregisterPollingHandler(SamplingHandler);
  }
  return prev;
}

BOOL __PADDisableRecalibration(BOOL disable) {
  BOOL enabled;
  BOOL prev;

  enabled = OSDisableInterrupts();
#ifdef PRIME1
  prev = (UnkVal & 0x40) ? TRUE : FALSE;
  UnkVal &= (u8)~0x40;
#else
  prev = (__gUnknown800030E3 & 0x40) ? TRUE : FALSE;
  __gUnknown800030E3 &= ~0x40;
#endif
  if (disable) {
#ifdef PRIME1
    UnkVal |= 0x40;
#else
    __gUnknown800030E3 |= 0x40;
  }

  OSRestoreInterrupts(enabled);
  return prev;
}

BOOL __PADDisableRumble(BOOL disable) {
  BOOL enabled;
  BOOL prev;

  enabled = OSDisableInterrupts();
  prev = (__gUnknown800030E3 & 0x20) ? TRUE : FALSE;
  __gUnknown800030E3 &= ~0x20;
  if (disable) {
    __gUnknown800030E3 |= 0x20;
#endif
  }
  OSRestoreInterrupts(enabled);
  return prev;
}

#ifndef PRIME1
BOOL PADIsBarrel(s32 chan) {
  if (chan < 0 || chan >= 4) {
    return FALSE;
  }

  if (BarrelBits & (PAD_CHAN0_BIT >> chan)) {
    return TRUE;
  }

  return FALSE;
}

#endif