#include <SDKVersion.h>
#include <dolphin/card.h>
#include <dolphin/dsp.h>
#include <dolphin/dvd.h>
#include <dolphin/os.h>

#include <dolphin/CARDPriv.h>

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
const char* __CARDVersion =
    "<< Dolphin SDK - CARD\trelease build: Sep  5 2002 05:35:20 (0x2301) >>";
#else
#ifndef _DEBUG
const char* __CARDVersion =
    "<< Dolphin SDK - CARD\trelease build: Apr  5 2004 04:15:35 (0x2301) >>";
#else
const char* __CARDVersion = "<< Dolphin SDK - CARD\tdebug build: Apr  5 2004 03:56:53 (0x2301) >>";
#endif

#if _DEBUG
u32 __CARDFreq = EXI_FREQ_16M;
#endif
#endif

CARDControl __CARDBlock[2];
DVDDiskID __CARDDiskNone;
static u16 __CARDEncode;
#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04
static u16 __CARDFastMode;
#endif

s32 __CARDReadStatus(s32 chan, u8* status);
s32 __CARDClearStatus(s32 chan);
void __CARDSetDiskID(const DVDDiskID* id);
static s32 Retry(s32 chan);

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
BOOL OnReset(BOOL f);
static OSResetFunctionInfo ResetFunctionInfo = {OnReset, 127};
#else
static BOOL OnReset(BOOL f);
static OSResetFunctionInfo ResetFunctionInfo = {
    OnReset,
    OS_RESET_PRIO_CARD,
};
#endif

void __CARDDefaultApiCallback(s32 chan, s32 result) {}
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
void __CARDSyncCallback(s32 chan) {
  CARDControl* card = &__CARDBlock[chan];
#endif

#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04
void __CARDSyncCallback(s32 chan, s32 result) {
  CARDControl* card;
  card = &__CARDBlock[chan];
#endif
  OSWakeupThread(&card->threadQueue);
}

#line 228
void __CARDExtHandler(s32 chan, OSContext* context) {
  CARDControl* card;
  CARDCallback callback;

#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04
  ASSERT(0 <= chan && chan < 2);
#endif
  card = &__CARDBlock[chan];
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  if (card->attached) {
    card->attached = FALSE;
    EXISetExiCallback(chan, 0);
    OSCancelAlarm(&card->alarm);
    callback = card->exiCallback;
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
    if (callback) {
      card->exiCallback = 0;
      callback(chan, CARD_RESULT_NOCARD);
    }
#else
  if (!card->attached) {
    return;
  }
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
    if (card->result != CARD_RESULT_BUSY) {
      card->result = CARD_RESULT_NOCARD;
    }
#else
  ASSERT(card->txCallback == 0);
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
    callback = card->extCallback;
    if (callback && CARD_MAX_MOUNT_STEP <= card->mountStep) {
      card->extCallback = 0;
      callback(chan, CARD_RESULT_NOCARD);
    }
#else
  card->attached = FALSE;
  EXISetExiCallback(chan, 0);
  OSCancelAlarm(&card->alarm);
  callback = card->exiCallback;

  if (callback) {
    card->exiCallback = 0;
    callback(chan, CARD_RESULT_NOCARD);
  }

  if (card->result != CARD_RESULT_BUSY) {
    card->result = CARD_RESULT_NOCARD;
  }

  callback = card->extCallback;
  if (callback && CARD_MAX_MOUNT_STEP <= card->mountStep) {
    card->extCallback = 0;
    callback(chan, CARD_RESULT_NOCARD);
#endif
  }
}

#line 277
void __CARDExiHandler(s32 chan, OSContext* context) {
  CARDControl* card;
  CARDCallback callback;
  u8 status;
  s32 result;
#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04

  ASSERT(0 <= chan && chan < 2);
#endif

  card = &__CARDBlock[chan];

  OSCancelAlarm(&card->alarm);

  if (!card->attached) {
    return;
  }

  if (!EXILock(chan, 0, 0)) {
    result = CARD_RESULT_FATAL_ERROR;
    goto fatal;
  }

  if ((result = __CARDReadStatus(chan, &status)) < 0 || (result = __CARDClearStatus(chan)) < 0) {
    goto error;
  }

  if ((result = (status & 0x18) ? CARD_RESULT_IOERROR : CARD_RESULT_READY) == CARD_RESULT_IOERROR &&
      --card->retry > 0) {
    result = Retry(chan);
    if (result >= 0) {
      return;
    }
    goto fatal;
  }

error:
  EXIUnlock(chan);

fatal:
  callback = card->exiCallback;
  if (callback) {
    card->exiCallback = 0;
    callback(chan, result);
  }
}

#line 359
void __CARDTxHandler(s32 chan, OSContext* context) {
  CARDControl* card;
  CARDCallback callback;
  BOOL err;
#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04

  ASSERT(0 <= chan && chan < 2);
#endif

  card = &__CARDBlock[chan];
  err = !EXIDeselect(chan);
  EXIUnlock(chan);
  callback = card->txCallback;
  if (callback) {
    card->txCallback = 0;
    callback(chan, (!err && EXIProbe(chan)) ? CARD_RESULT_READY : CARD_RESULT_NOCARD);
  }
}

#line 407
void __CARDUnlockedHandler(s32 chan, OSContext* context) {
  CARDControl* card;
  CARDCallback callback;

#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04
  ASSERT(0 <= chan && chan < 2);
#endif
  card = &__CARDBlock[chan];
  callback = card->unlockCallback;
  if (callback) {
    card->unlockCallback = 0;
    callback(chan, EXIProbe(chan) ? CARD_RESULT_UNLOCKED : CARD_RESULT_NOCARD);
  }
}

#line 427
s32 __CARDEnableInterrupt(s32 chan, BOOL enable) {
  BOOL err;
  u32 cmd;

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  if (!EXISelect(chan, 0, 4)) {
#else
  ASSERT(0 <= chan && chan < 2);
#ifdef _DEBUG
  if (!EXISelect(chan, 0, __CARDFreq)) {
#else
  if (!EXISelect(chan, 0, 4)) {
#endif
#endif
    return CARD_RESULT_NOCARD;
  }
  cmd = enable ? 0x81010000 : 0x81000000;
  err = FALSE;
  err |= !EXIImm(chan, &cmd, 2, 1, NULL);
  err |= !EXISync(chan);
  err |= !EXIDeselect(chan);
  return err ? CARD_RESULT_NOCARD : CARD_RESULT_READY;
}

#line 446
s32 __CARDReadStatus(s32 chan, u8* status) {
  BOOL err;
  u32 cmd;

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  if (!EXISelect(chan, 0, 4)) {
#else
  ASSERT(0 <= chan && chan < 2);
#ifdef _DEBUG
  if (!EXISelect(chan, 0, __CARDFreq)) {
#else
  if (!EXISelect(chan, 0, 4)) {
#endif
#endif
    return CARD_RESULT_NOCARD;
  }

  cmd = 0x83000000;
  err = FALSE;
  err |= !EXIImm(chan, &cmd, 2, 1, NULL);
  err |= !EXISync(chan);
  err |= !EXIImm(chan, status, 1, 0, NULL);
  err |= !EXISync(chan);
  err |= !EXIDeselect(chan);
  return err ? CARD_RESULT_NOCARD : CARD_RESULT_READY;
}

#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04
#line 467
s32 __CARDReadVendorID(s32 chan, u16* id) {
  BOOL err;
  u32 cmd;

  ASSERT(0 <= chan && chan < 2);
#ifdef _DEBUG
  if (!EXISelect(chan, 0, __CARDFreq)) {
#else
  if (!EXISelect(chan, 0, 4)) {
#endif
    return CARD_RESULT_NOCARD;
  }

  cmd = 0x85000000;
  err = FALSE;
  err |= !EXIImm(chan, &cmd, 2, EXI_WRITE, NULL);
  err |= !EXISync(chan);
  err |= !EXIImm(chan, id, 2, EXI_READ, NULL);
  err |= !EXISync(chan);
  err |= !EXIDeselect(chan);
  return err ? CARD_RESULT_NOCARD : CARD_RESULT_READY;
}

#line 488
#endif
s32 __CARDClearStatus(s32 chan) {
  BOOL err;
  u32 cmd;

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  if (!EXISelect(chan, 0, 4)) {
#else
#ifdef _DEBUG
  if (!EXISelect(chan, 0, __CARDFreq)) {
#else
  if (!EXISelect(chan, 0, 4)) {
#endif
#endif
    return CARD_RESULT_NOCARD;
  }

  cmd = 0x89000000;
  err = FALSE;
  err |= !EXIImm(chan, &cmd, 1, 1, 0);
  err |= !EXISync(chan);
  err |= !EXIDeselect(chan);

  return err ? CARD_RESULT_NOCARD : CARD_RESULT_READY;
}

#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04
//
s32 __CARDSleep(s32 chan) {
  BOOL err;
  u32 cmd;

  ASSERT(0 <= chan && chan < 2);
#ifdef _DEBUG
  if (!EXISelect(chan, 0, __CARDFreq)) {
#else
  if (!EXISelect(chan, 0, 4)) {
#endif
    return CARD_RESULT_NOCARD;
  }

  cmd = 0x88000000;
  err = FALSE;
  err |= !EXIImm(chan, &cmd, 1, EXI_WRITE, NULL);
  err |= !EXISync(chan);
  err |= !EXIDeselect(chan);
  return err ? CARD_RESULT_NOCARD : CARD_RESULT_READY;
}

#line 526
s32 __CARDWakeup(s32 chan) {
  BOOL err;
  u32 cmd;

  ASSERT(0 <= chan && chan < 2);
#ifdef _DEBUG
  if (!EXISelect(chan, 0, __CARDFreq)) {
#else
  if (!EXISelect(chan, 0, 4)) {
#endif
    return CARD_RESULT_NOCARD;
  }

  cmd = 0x87000000;
  err = FALSE;
  err |= !EXIImm(chan, &cmd, 1, EXI_WRITE, NULL);
  err |= !EXISync(chan);
  err |= !EXIDeselect(chan);
  return err ? CARD_RESULT_NOCARD : CARD_RESULT_READY;
}

#line 564
#endif
static void TimeoutHandler(OSAlarm* alarm, OSContext* context) {
  s32 chan;
  CARDControl* card;
  CARDCallback callback;
  for (chan = 0; chan < 2; ++chan) {
    card = &__CARDBlock[chan];
    if (alarm == &card->alarm) {
      break;
    }
  }
#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04

  //
  //
  ASSERT(0 <= chan && chan < 2);
#endif

  if (!card->attached) {
    return;
  }

  EXISetExiCallback(chan, NULL);
  callback = card->exiCallback;
  if (callback) {
    card->exiCallback = 0;
    callback(chan, CARD_RESULT_IOERROR);
  }
}

static void SetupTimeoutAlarm(CARDControl* card) {
  OSCancelAlarm(&card->alarm);
  switch (card->cmd[0]) {
  case 0xF2:
    OSSetAlarm(&card->alarm, OSMillisecondsToTicks(100), TimeoutHandler);
    break;
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  case 0xF3:
#else
  case 0xF4:
    if (CARD_PAGE_SIZE < card->pageSize) {
      OSSetAlarm(&card->alarm, OSSecondsToTicks((OSTime)2) * (card->cBlock / 64), TimeoutHandler);
      break;
    }
  case 0xF1:
    OSSetAlarm(&card->alarm, OSSecondsToTicks((OSTime)2) * (card->sectorSize / (8 * 1024)),
               TimeoutHandler);
#endif
    break;
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  case 0xF4:
  case 0xF1:
    OSSetAlarm(&card->alarm, OSSecondsToTicks((OSTime)2) * (card->sectorSize / 0x2000),
               TimeoutHandler);
#else
  default:
#endif
    break;
  }
}

static s32 Retry(s32 chan) {
  CARDControl* card;
#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04

#line 654
  ASSERT(0 <= chan && chan < 2);

#endif
  card = &__CARDBlock[chan];

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  if (!EXISelect(chan, 0, 4)) {
#else
#ifdef _DEBUG
  if (!EXISelect(chan, 0, __CARDFreq)) {
#else
  if (!EXISelect(chan, 0, 4)) {
#endif
#endif
    EXIUnlock(chan);
    return CARD_RESULT_NOCARD;
  }

  SetupTimeoutAlarm(card);

  if (!EXIImmEx(chan, card->cmd, card->cmdlen, 1)) {
    EXIDeselect(chan);
    EXIUnlock(chan);
    return CARD_RESULT_NOCARD;
  }

  if (card->cmd[0] == 0x52 &&
      !EXIImmEx(chan, (u8*)card->workArea + sizeof(CARDID), card->latency, 1)) {
    EXIDeselect(chan);
    EXIUnlock(chan);
    return CARD_RESULT_NOCARD;
  }

  if (card->mode == 0xffffffff) {
    EXIDeselect(chan);
    EXIUnlock(chan);
    return CARD_RESULT_READY;
  }

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  if (!EXIDma(chan, card->buffer, (s32)((card->cmd[0] == 0x52) ? 512 : 128), card->mode,
#else
  if (!EXIDma(chan, card->buffer, (s32)((card->cmd[0] == 0x52) ? 512 : card->pageSize), card->mode,
#endif
              __CARDTxHandler)) {
    EXIDeselect(chan);
    EXIUnlock(chan);
    return CARD_RESULT_NOCARD;
  }

  return CARD_RESULT_READY;
}

#line 714
static void UnlockedCallback(s32 chan, s32 result) {
  CARDCallback callback;
  CARDControl* card;

#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04
  ASSERT(0 <= chan && chan < 2);
#endif
  card = &__CARDBlock[chan];
  if (result >= 0) {
    card->unlockCallback = UnlockedCallback;
    if (!EXILock(chan, 0, __CARDUnlockedHandler)) {
      result = CARD_RESULT_READY;
    } else {
      card->unlockCallback = 0;
      result = Retry(chan);
    }
  }

  if (result < 0) {
    switch (card->cmd[0]) {
    case 0x52:
      callback = card->txCallback;
      if (callback) {
        card->txCallback = 0;
        callback(chan, result);
      }

      break;
    case 0xF2:
    case 0xF4:
    case 0xF1:
      callback = card->exiCallback;
      if (callback) {
        card->exiCallback = 0;
        callback(chan, result);
      }
      break;
    }
  }
}

#line 777
static s32 __CARDStart(s32 chan, CARDCallback txCallback, CARDCallback exiCallback) {
  BOOL enabled;
  CARDControl* card;
  s32 result;

  enabled = OSDisableInterrupts();

#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04
  ASSERT(0 <= chan && chan < 2);
#endif
  card = &__CARDBlock[chan];
  if (!card->attached) {
    result = CARD_RESULT_NOCARD;
  } else {

    if (txCallback) {
      card->txCallback = txCallback;
    }
    if (exiCallback) {
      card->exiCallback = exiCallback;
    }
    card->unlockCallback = UnlockedCallback;
    if (!EXILock(chan, 0, __CARDUnlockedHandler)) {
      result = CARD_RESULT_BUSY;
    } else {
      card->unlockCallback = 0;

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
      if (!EXISelect(chan, 0, 4)) {
#else
#ifdef _DEBUG
      if (!EXISelect(chan, 0, __CARDFreq)) {
#else
      if (!EXISelect(chan, 0, 4)) {
#endif
#endif
        EXIUnlock(chan);
        result = CARD_RESULT_NOCARD;
      } else {
        SetupTimeoutAlarm(card);
        result = CARD_RESULT_READY;
      }
    }
  }

  OSRestoreInterrupts(enabled);
  return result;
}

#define AD1(x) ((u8)(((x) >> 17) & 0x7f))
#define AD1EX(x) ((u8)(AD1(x) | 0x80));
#define AD2(x) ((u8)(((x) >> 9) & 0xff))
#define AD3(x) ((u8)(((x) >> 7) & 0x03))
#define BA(x) ((u8)((x)&0x7f))

#line 842
s32 __CARDReadSegment(s32 chan, CARDCallback callback) {
  CARDControl* card;
  s32 result;
#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04
  // clang-format off
  ASSERT(0 <= chan && chan < 2);
  card = &__CARDBlock[chan];
  ASSERT(card->addr % CARD_SEG_SIZE == 0);
  ASSERT(card->addr < (u32) card->size * 1024 * 1024 / 8);
  // clang-format on
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  card = &__CARDBlock[chan];
#endif
  card->cmd[0] = 0x52;
  card->cmd[1] = AD1(card->addr);
  card->cmd[2] = AD2(card->addr);
  card->cmd[3] = AD3(card->addr);
  card->cmd[4] = BA(card->addr);
  card->cmdlen = 5;
  card->mode = 0;
  card->retry = 0;

  result = __CARDStart(chan, callback, 0);
  if (result == CARD_RESULT_BUSY) {
    result = CARD_RESULT_READY;
  } else if (result >= 0) {
    if (!EXIImmEx(chan, card->cmd, card->cmdlen, 1) ||
        !EXIImmEx(chan, (u8*)card->workArea + sizeof(CARDID), card->latency,
                  1) || // XXX use DMA if possible
        !EXIDma(chan, card->buffer, 512, card->mode, __CARDTxHandler)) {
      card->txCallback = 0;
      EXIDeselect(chan);
      EXIUnlock(chan);
      result = CARD_RESULT_NOCARD;
    } else {
      result = CARD_RESULT_READY;
    }
  }
  return result;
}

#line 899
s32 __CARDWritePage(s32 chan, CARDCallback callback) {
  CARDControl* card;
  s32 result;
#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04
  // clang-format off
  ASSERT(0 <= chan && chan < 2);
  card = &__CARDBlock[chan];
  ASSERT(card->addr % card->pageSize == 0);
  ASSERT(card->addr < (u32) card->size * 1024 * 1024 / 8);
  // clang-format on
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  card = &__CARDBlock[chan];
#endif
  card->cmd[0] = 0xF2;
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  card->cmd[1] = AD1(card->addr);
#else
  if (CARD_PAGE_SIZE < card->pageSize) {
    card->cmd[1] = AD1EX(card->addr);
  } else {
    card->cmd[1] = AD1(card->addr);
  }
#endif
  card->cmd[2] = AD2(card->addr);
  card->cmd[3] = AD3(card->addr);
  card->cmd[4] = BA(card->addr);
  card->cmdlen = 5;
  card->mode = 1;
  card->retry = 3;

  result = __CARDStart(chan, 0, callback);
  if (result == CARD_RESULT_BUSY) {
    result = CARD_RESULT_READY;
  } else if (result >= 0) {
    if (!EXIImmEx(chan, card->cmd, card->cmdlen, 1) ||
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
        !EXIDma(chan, card->buffer, 128, card->mode, __CARDTxHandler)) {
#else
        !EXIDma(chan, card->buffer, card->pageSize, card->mode, __CARDTxHandler)) {
#endif
      card->exiCallback = 0;
      EXIDeselect(chan);
      EXIUnlock(chan);
      result = CARD_RESULT_NOCARD;
    } else {
      result = CARD_RESULT_READY;
    }
  }
  return result;
}

#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04
#line 958
s32 __CARDErase(s32 chan, CARDCallback callback) {
  CARDControl* card;
  s32 result;

  ASSERT(0 <= chan && chan < 2);
  card = &__CARDBlock[chan];
  card->cmd[0] = 0xF4;
  card->cmd[1] = 0;
  card->cmd[2] = 0;
  card->cmdlen = 3;
  card->mode = -1;
  card->retry = 3;

  result = __CARDStart(chan, 0, callback);
  if (result == CARD_RESULT_BUSY) {
    result = CARD_RESULT_READY;
  } else if (0 <= result) {
    if (!EXIImmEx(chan, card->cmd, card->cmdlen, EXI_WRITE)) {
      result = CARD_RESULT_NOCARD;
      card->exiCallback = 0;
    } else {
      result = CARD_RESULT_READY;
    }
    EXIDeselect(chan);
    EXIUnlock(chan);
  }
  return result;
}

#line 1006
#endif
s32 __CARDEraseSector(s32 chan, u32 addr, CARDCallback callback) {
  CARDControl* card;
  s32 result;
#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04
  // clang-format off
  ASSERT(0 <= chan && chan < 2);
  card = &__CARDBlock[chan];
  ASSERT(addr % card->sectorSize == 0);
  ASSERT(addr < (u32) card->size * 1024 * 1024 / 8);
  // clang-format on
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  card = &__CARDBlock[chan];
#else
  if (CARD_PAGE_SIZE < card->pageSize) {
    if (callback) {
      callback(chan, CARD_RESULT_READY);
    }
    return CARD_RESULT_READY;
  }

#endif
  card->cmd[0] = 0xF1;
  card->cmd[1] = AD1(addr);
  card->cmd[2] = AD2(addr);
  card->cmdlen = 3;
  card->mode = -1;
  card->retry = 3;

  result = __CARDStart(chan, 0, callback);

  if (result == CARD_RESULT_BUSY) {
    result = CARD_RESULT_READY;
  } else if (result >= 0) {
    if (!EXIImmEx(chan, card->cmd, card->cmdlen, 1)) {
#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04
      result = CARD_RESULT_NOCARD;
#endif
      card->exiCallback = NULL;
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
      result = CARD_RESULT_NOCARD;
#endif
    } else {
      result = CARD_RESULT_READY;
    }

    EXIDeselect(chan);
    EXIUnlock(chan);
  }
  return result;
}

void CARDInit(void) {
  int chan;

  if (__CARDBlock[0].diskID && __CARDBlock[1].diskID) {
    return;
  }

  __CARDEncode = OSGetFontEncode();

  OSRegisterVersion(__CARDVersion);

  DSPInit();
  OSInitAlarm();

  for (chan = 0; chan < 2; ++chan) {
    CARDControl* card = &__CARDBlock[chan];

    card->result = CARD_RESULT_NOCARD;
    OSInitThreadQueue(&card->threadQueue);
    OSCreateAlarm(&card->alarm);
  }
  __CARDSetDiskID((DVDDiskID*)OSPhysicalToCached(0x0));

  OSRegisterResetFunction(&ResetFunctionInfo);
}

u16 __CARDGetFontEncode() { return __CARDEncode; }

#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04
u16 __CARDSetFontEncode(u16 encode) {
  u16 prev;

  prev = __CARDEncode;
  switch (encode) {
  case OS_FONT_ENCODE_ANSI:
  case OS_FONT_ENCODE_SJIS:
    __CARDEncode = encode;
    break;
  }
  return prev;
}

#endif
void __CARDSetDiskID(const DVDDiskID* id) {
  __CARDBlock[0].diskID = id ? id : &__CARDDiskNone;
  __CARDBlock[1].diskID = id ? id : &__CARDDiskNone;
#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04
}

#line 1167
const DVDDiskID* CARDGetDiskID(s32 chan) {
  ASSERT(0 <= chan && chan < 2);
  return __CARDBlock[chan].diskID;
}

#line 1185
s32 CARDSetDiskID(s32 chan, const DVDDiskID* diskID) {
  BOOL enabled;
  CARDControl* card = &__CARDBlock[chan];

  ASSERT(0 <= chan && chan < 2);
  enabled = OSDisableInterrupts();
  if (card->result == CARD_RESULT_BUSY) {
    return CARD_RESULT_BUSY;
  }
  card->diskID = diskID ? diskID : (DVDDiskID*)OSPhysicalToCached(0);
  OSRestoreInterrupts(enabled);
  return CARD_RESULT_READY;
#endif
}

s32 __CARDGetControlBlock(s32 chan, CARDControl** pcard) {
  BOOL enabled;
  s32 result;
  CARDControl* card;

  card = &__CARDBlock[chan];
  if (chan < 0 || chan >= 2 || card->diskID == NULL) {
    return CARD_RESULT_FATAL_ERROR;
  }

  enabled = OSDisableInterrupts();
  if (!card->attached) {
    result = CARD_RESULT_NOCARD;
  } else if (card->result == CARD_RESULT_BUSY) {
    result = CARD_RESULT_BUSY;
  } else {
    card->result = CARD_RESULT_BUSY;
    result = CARD_RESULT_READY;
    card->apiCallback = 0;
    *pcard = card;
  }
  OSRestoreInterrupts(enabled);
  return result;
}

#line 1256
s32 __CARDPutControlBlock(CARDControl* card, s32 result) {
  BOOL enabled;

#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04
  ASSERT(result != CARD_RESULT_BUSY);
#endif
  enabled = OSDisableInterrupts();
  if (card->attached) {
    card->result = result;
  } else if (card->result == CARD_RESULT_BUSY) {
    card->result = result;
  }
  OSRestoreInterrupts(enabled);
  return result;
}

#line 1289
s32 CARDGetResultCode(s32 chan) {
  CARDControl* card;
#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04

  ASSERT(0 <= chan && chan < 2);
#endif
  if (chan < 0 || chan >= 2) {
    return CARD_RESULT_FATAL_ERROR;
  }
  card = &__CARDBlock[chan];
  return card->result;
}

s32 CARDFreeBlocks(s32 chan, s32* byteNotUsed, s32* filesNotUsed) {
  CARDControl* card;
  s32 result;
  u16* fat;
  CARDDir* dir;
  CARDDir* ent;
  u16 fileNo;

  result = __CARDGetControlBlock(chan, &card);
  if (result < 0) {
    return result;
  }

  fat = __CARDGetFatBlock(card);
  dir = __CARDGetDirBlock(card);
  if (fat == 0 || dir == 0) {
    return __CARDPutControlBlock(card, CARD_RESULT_BROKEN);
  }

  if (byteNotUsed) {
    *byteNotUsed = (s32)(card->sectorSize * fat[CARD_FAT_FREEBLOCKS]);
  }

  if (filesNotUsed) {
    *filesNotUsed = 0;
    for (fileNo = 0; fileNo < CARD_MAX_FILE; fileNo++) {
      ent = &dir[fileNo];
      if (ent->fileName[0] == 0xff) {
        ++*filesNotUsed;
      }
    }
  }

  return __CARDPutControlBlock(card, CARD_RESULT_READY);
}

s32 CARDGetEncoding(s32 chan, u16* encode) {
  CARDControl* card;
  CARDID* id;
  s32 result;

  result = __CARDGetControlBlock(chan, &card);
  if (result < 0) {
    return result;
  }
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  id = card->workArea;
#else

  id = (CARDID*)card->workArea;
#endif
  *encode = id->encode;
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  return __CARDPutControlBlock(card, 0);
#else

  return __CARDPutControlBlock(card, CARD_RESULT_READY);
#endif
}

s32 CARDGetMemSize(s32 chan, u16* size) {
  CARDControl* card;
  s32 result;

  result = __CARDGetControlBlock(chan, &card);
  if (result < 0) {
    return result;
  }
  *size = card->size;
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  return __CARDPutControlBlock(card, 0);
#else

  return __CARDPutControlBlock(card, CARD_RESULT_READY);
#endif
}

s32 CARDGetSectorSize(s32 chan, u32* size) {
  CARDControl* card;
  s32 result;

  result = __CARDGetControlBlock(chan, &card);
  if (result < 0) {
    return result;
  }
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  *size = card->sectorSize;
  return __CARDPutControlBlock(card, 0);
#else

  *size = (u32)card->sectorSize;

  return __CARDPutControlBlock(card, CARD_RESULT_READY);
#endif
}

s32 __CARDSync(s32 chan) {
  CARDControl* card;
  s32 result;
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  s32 enabled;
#else
  BOOL enabled;
#endif

  card = &__CARDBlock[chan];
  enabled = OSDisableInterrupts();
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  while ((result = CARDGetResultCode(chan)) == -1) {
#else
  while ((result = CARDGetResultCode(chan)) == CARD_RESULT_BUSY) {
#endif
    OSSleepThread(&card->threadQueue);
  }
  OSRestoreInterrupts(enabled);
  return result;
}

static BOOL OnReset(BOOL f) {
  if (!f) {
    if (CARDUnmount(0) == CARD_RESULT_BUSY || CARDUnmount(1) == CARD_RESULT_BUSY) {
      return FALSE;
    }
  }

  return TRUE;
}

#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04
BOOL CARDSetFastMode(BOOL enable) {
  u16 prev;

  prev = __CARDFastMode;
  __CARDFastMode = (u16)(enable ? 1 : 0);
  return prev ? TRUE : FALSE;
}

BOOL CARDGetFastMode(void) { return __CARDFastMode ? TRUE : FALSE; }

s32 CARDGetCurrentMode(s32 chan, u32* mode) {
  CARDControl* card;
  s32 result;

  result = __CARDGetControlBlock(chan, &card);
  if (result < 0) {
    return result;
  }

  switch (card->pageSize) {
  case CARD_SEG_SIZE:
    *mode = CARD_MODE_FAST;
    break;
  default:
  case CARD_PAGE_SIZE:
    *mode = CARD_MODE_NORMAL;
    break;
  }

  return __CARDPutControlBlock(card, CARD_RESULT_READY);
}

#endif
