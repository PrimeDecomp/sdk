#include <revolution/wpad_2005.h>
#include <dolphin/os.h>
#include <dolphin/si.h>
#include <dolphin/sipriv.h>
#include <string.h>

const char* __WPADVersion =
    "<< Revolution SDK - WPAD\trelease build: Nov 25 2005 09:26:23 (0x2301) >>";

static u8 s_txBuf[4][3];
static u8 s_report0[4][4];
static u8 s_report1[4][4];
static u8 s_rxBuf0[128] ATTRIBUTE_ALIGN(32);
static u8 s_rxBuf1[128] ATTRIBUTE_ALIGN(32);
static BOOL s_coreMotor_flag[4];
static WPADCallback s_userCallback[4];
static void* s_samplingBufs_ptr[4];
static u32 s_bufLatestIndex[4];
static u32 s_bufLength[4];
static u32 s_dataFormat[4];
static PADStatus s_dolOrigin[4];
static WPADFSStatus s_fsOrigin[4];
BOOL g_ext_flag[4];

static u32 wpadGetType(s32 chan);
static void wpadCopyOut(void);
static void wpadSISamplingCb(__OSInterrupt interrupt, OSContext* context);
static void wpadSITransferCommands(u8 count);
static s8 wpadErrSequence(s32 chan);
static void wpadConvert(s32 chan, void* status);
static void wpadConvertCoreFormat(s32 chan, WPADStatus* status);
static void wpadConvertFSFormat(s32 chan, WPADFSStatus* status);
static void wpadConvertDolphinFormat(s32 chan, PADStatus* status);
static void wpadFixDPDValue(DPDObject* obj);
static s8 wpadCalibrateStick(s8 value, s8 origin);
static u8 wpadCalibrateTrigger(u8 value, u8 origin);

void WPADInit(void) {
  static BOOL s_initialized_flag;
  s32 chan;
  u8 response[8];
  u32 command;
  if (!s_initialized_flag) {
    for (chan = 0; chan < SI_MAX_CHAN; ++chan) {
      s_userCallback[chan] = NULL;
      s_samplingBufs_ptr[chan] = NULL;
      s_bufLatestIndex[chan] = 0;
      s_bufLength[chan] = 0;
      s_report0[chan][3] = SI_ERROR_NO_RESPONSE;
      s_report1[chan][3] = SI_ERROR_NO_RESPONSE;
      s_dataFormat[chan] = WPAD_FMT_CORE;
    }
    SIInit();
    for (chan = 0, command = 0x00400000; chan < SI_MAX_CHAN; ++chan) {
      SIGetResponse(chan, response);
      SISetCommand(chan, command);
    }
    SISetSamplingRate(1);
    SIRegisterPollingHandler(wpadSISamplingCb);
    SIEnablePolling(SI_CHAN0_BIT | SI_CHAN1_BIT | SI_CHAN2_BIT | SI_CHAN3_BIT);
    OSRegisterVersion(__WPADVersion);
    s_initialized_flag = TRUE;
  }
}

void WPADSetDataFormat(s32 chan, u32 fmt) { s_dataFormat[chan] = fmt; }

void WPADControlMotor(s32 chan, u32 command) {
  if (command == WPAD_MOTOR_STOP) {
    s_coreMotor_flag[chan] = FALSE;
  } else if (command == WPAD_MOTOR_RUMBLE) {
    s_coreMotor_flag[chan] = TRUE;
  }
}

void WPADSetAutoSamplingBuf(s32 chan, void* buf, u32 length) {
  u32 i;
  if (buf != NULL) {
    for (i = 0; i < length; ++i) {
      switch (s_dataFormat[chan]) {
      case WPAD_FMT_CORE:
        ((WPADStatus*)buf)[i].err = WPAD_ERR_INVALID;
        break;
      case WPAD_FMT_FREESTYLE:
        ((WPADFSStatus*)buf)[i].err = WPAD_ERR_INVALID;
        break;
      case WPAD_FMT_DOLPHIN:
        ((PADStatus*)buf)[i].err = WPAD_ERR_INVALID;
        break;
      }
    }
    s_bufLatestIndex[chan] = -1;
    s_bufLength[chan] = length;
  }
  s_samplingBufs_ptr[chan] = buf;
}

u32 WPADGetLatestIndexInBuf(s32 chan) {
  if (s_bufLatestIndex[chan] == (u32)-1) {
    return 0;
  }
  return s_bufLatestIndex[chan];
}

void WPADRead(s32 chan, void* status) { wpadConvert(chan, status); }

s32 WPADProbe(s32 chan, u32* type) {
  s32 err = wpadErrSequence(chan);
  *type = wpadGetType(chan);
  return err;
}

// Guessed name
s32 WPADRecalibrate(s32 chan) {
  wpadConvertFSFormat(chan, &s_fsOrigin[chan]);
  wpadConvertDolphinFormat(chan, &s_dolOrigin[chan]);
  return 1;
}

static u32 wpadGetType(s32 chan) {
  u8 type, err;
  u32 result;
  BOOL enabled = OSDisableInterrupts();
  type = s_report1[chan][1];
  err = s_report1[chan][3];
  OSRestoreInterrupts(enabled);
  if (err == 0) {
    switch (type) {
    case 0:
      result = WPAD_DEV_DOLPHIN;
      break;
    case 0xf0:
    case 0xf1:
      if ((s_rxBuf1[chan * 32 + 5] >> 3) & 1) {
        result = WPAD_DEV_FREESTYLE;
      } else {
        result = WPAD_DEV_CORE;
      }
      break;
    default:
      result = WPAD_DEV_UNKNOWN;
      break;
    }
    return result;
  }
  return 0;
}

static void wpadCopyOut(void) {
  s32 chan;
  for (chan = 0; chan < SI_MAX_CHAN; ++chan) {
    if (s_samplingBufs_ptr[chan] != NULL) {
      ++s_bufLatestIndex[chan];
      if (s_bufLatestIndex[chan] >= s_bufLength[chan]) {
        s_bufLatestIndex[chan] = 0;
      }
      switch (s_dataFormat[chan]) {
      case WPAD_FMT_CORE:
        wpadConvert(chan, &((WPADStatus*)s_samplingBufs_ptr[chan])[s_bufLatestIndex[chan]]);
        break;
      case WPAD_FMT_FREESTYLE:
        wpadConvert(chan, &((WPADFSStatus*)s_samplingBufs_ptr[chan])[s_bufLatestIndex[chan]]);
        break;
      case WPAD_FMT_DOLPHIN:
        wpadConvert(chan, &((PADStatus*)s_samplingBufs_ptr[chan])[s_bufLatestIndex[chan]]);
        break;
      }
    }
    if (s_userCallback[chan]) {
      s_userCallback[chan](chan);
    }
  }
}

static void wpadSISamplingCb(__OSInterrupt interrupt, OSContext* context) {
#pragma unused(interrupt, context)
  static u8 s_cnt;
  static BOOL s_ready_flag;
  s32 chan;
  u8 response[8];
  u32 type;
  if (s_cnt == 0 && s_ready_flag) {
    BOOL enabled = OSDisableInterrupts();
    memcpy(s_rxBuf1, s_rxBuf0, sizeof(s_rxBuf1));
    memcpy(s_report1, s_report0, sizeof(s_report1));
    wpadCopyOut();
    OSRestoreInterrupts(enabled);
  }
  for (chan = 0; chan < SI_MAX_CHAN; ++chan) {
    BOOL enabled = OSDisableInterrupts();
    s_report0[chan][3] = SIGetStatus(chan);
    if (!SIGetResponse(chan, response)) {
      s_report0[chan][3] |= SI_ERROR_BUSY;
    } else {
      if (s_dataFormat[chan] != WPAD_FMT_DOLPHIN) {
        switch (response[0] & 0x30) {
        case 0x00:
          memcpy(s_rxBuf0 + chan * 32, response, 8);
          break;
        case 0x10:
          memcpy(s_rxBuf0 + chan * 32 + 8, response, 8);
          break;
        case 0x20:
          memcpy(s_rxBuf0 + chan * 32 + 16, response, 8);
          break;
        case 0x30:
          memcpy(s_rxBuf0 + chan * 32 + 24, response, 8);
          type = SIGetType(chan);
          s_report0[chan][0] = type >> 24;
          s_report0[chan][1] = type >> 16;
          s_report0[chan][2] = type >> 8;
          s_report0[chan][3] = type;
          break;
        }
      } else {
        memcpy(&s_rxBuf0[chan * 32], response, 8);
      }
      wpadSITransferCommands(s_cnt);
      OSRestoreInterrupts(enabled);
    }
  }
  if (!s_ready_flag && s_cnt == 3) {
    s_ready_flag = TRUE;
  }
  s_cnt = (s_cnt + 1) % 4;
}

static void wpadSITransferCommands(u8 count) {
  s32 chan;
  u32 motor, extension;
  u32 commands[4];
  for (chan = 0; chan < SI_MAX_CHAN; ++chan) {
    commands[chan] = 0x00400000;
  }
  for (chan = 0; chan < SI_MAX_CHAN; ++chan) {
    motor = s_coreMotor_flag[chan] ? 1 : 0;
    extension = g_ext_flag[chan] ? 4 : 0;
    commands[chan] = (commands[chan] & 0xffffff00) | ((u8)motor | (u8)extension);
    commands[chan] = (commands[chan] & 0xffff00ff) | ((count + 1) << 8);
    if (s_dataFormat[chan] == WPAD_FMT_DOLPHIN) {
      SISetCommand(chan, 0x00400300);
    } else {
      SISetCommand(chan, commands[chan]);
    }
    SITransferCommands();
  }
}

static s8 wpadErrSequence(s32 chan) {
  s8 err;
  u8 status;
  BOOL enabled = OSDisableInterrupts();
  status = s_report1[chan][3];
  if (status == 0) {
    err = WPAD_ERR_NONE;
  } else if (status & SI_ERROR_NO_RESPONSE) {
    err = WPAD_ERR_NO_CONTROLLER;
  } else if (status & (SI_ERROR_UNDER_RUN | SI_ERROR_OVER_RUN | SI_ERROR_COLLISION)) {
    err = WPAD_ERR_TRANSFER;
  } else if (status & (SI_ERROR_WRST | SI_ERROR_RDST | SI_ERROR_BUSY)) {
    err = WPAD_ERR_BUSY;
  } else {
    err = WPAD_ERR_TRANSFER;
  }
  OSRestoreInterrupts(enabled);
  return err;
}

static void wpadConvert(s32 chan, void* status) {
  WPADFSStatus* fs;
  PADStatus* dol;
  switch (s_dataFormat[chan]) {
  case WPAD_FMT_CORE:
    wpadConvertCoreFormat(chan, status);
    break;
  case WPAD_FMT_FREESTYLE:
    fs = status;
    wpadConvertFSFormat(chan, fs);
    fs->fsStickX = wpadCalibrateStick(fs->fsStickX, s_fsOrigin[chan].fsStickX);
    fs->fsStickY = wpadCalibrateStick(fs->fsStickY, s_fsOrigin[chan].fsStickY);
    break;
  case WPAD_FMT_DOLPHIN:
    dol = status;
    wpadConvertDolphinFormat(chan, dol);
    dol->stickX = wpadCalibrateStick(dol->stickX, s_dolOrigin[chan].stickX);
    dol->stickY = wpadCalibrateStick(dol->stickY, s_dolOrigin[chan].stickY);
    dol->substickX = wpadCalibrateStick(dol->substickX, s_dolOrigin[chan].substickX);
    dol->substickY = wpadCalibrateStick(dol->substickY, s_dolOrigin[chan].substickY);
    dol->triggerLeft = wpadCalibrateTrigger(dol->triggerLeft, s_dolOrigin[chan].triggerLeft);
    dol->triggerRight = wpadCalibrateTrigger(dol->triggerRight, s_dolOrigin[chan].triggerRight);
    break;
  }
}

static void wpadConvertCoreFormat(s32 chan, WPADStatus* status) {
  static BOOL s_enabled_flag[4];
  BOOL enabled = OSDisableInterrupts();
  u32 buttons;
  u32 button_map[11][3] = {
    {4, 0x80, 0x1},
    {4, 0x40, 0x2},
    {12, 0x80, 0x20},
    {12, 0x40, 0x10},
    {28, 0x80, 0x4},
    {28, 0x40, 0x8},
    {29, 0x80, 0x100},
    {29, 0x40, 0x200},
    {29, 0x20, 0x400},
    {29, 0x10, 0x800},
    {29, 0x8, 0x1000},
  };
  s32 i;
  status->button = 0;
  for (i = 0; i < 11; ++i) {
    buttons = s_rxBuf1[chan * 32 + button_map[i][0]];
    if (buttons & button_map[i][1]) {
      status->button |= button_map[i][2];
    }
  }
  status->accX = (((s_rxBuf1[chan * 32 + 5] & 0x30) >> 4) | (s_rxBuf1[chan * 32 + 6] << 2)) - 511;
  status->accY = (((s_rxBuf1[chan * 32 + 5] >> 6) & 3) | (s_rxBuf1[chan * 32 + 7] << 2)) - 511;
  status->accZ = (((s_rxBuf1[chan * 32 + 13] >> 4) & 3) | (s_rxBuf1[chan * 32 + 14] << 2)) - 511;
  for (i = 0; i < 4; ++i) {
    status->obj[i].x = ((s_rxBuf1[chan * 32 + i * 8] & 3) << 8) | s_rxBuf1[chan * 32 + i * 8 + 1];
    status->obj[i].y = 767 - (((s_rxBuf1[chan * 32 + i * 8] << 6) & 0x300) | s_rxBuf1[chan * 32 + i * 8 + 2]);
    status->obj[i].size = s_rxBuf1[chan * 32 + i * 8 + 3] | ((s_rxBuf1[chan * 32 + i * 8 + 4] & 0x3f) << 8);
    status->obj[i].traceId = s_rxBuf1[chan * 32 + i * 8 + 5] & 7;
  }
  wpadFixDPDValue(status->obj);
  status->dev = wpadGetType(chan);
  status->err = wpadErrSequence(chan);
  OSRestoreInterrupts(enabled);
}

static void wpadConvertFSFormat(s32 chan, WPADFSStatus* status) {
  BOOL enabled = OSDisableInterrupts();
  u32 buttons;
  u32 button_map[13][3] = {
    {4, 0x80, 0x1},
    {4, 0x40, 0x2},
    {12, 0x80, 0x20},
    {12, 0x40, 0x10},
    {20, 0x40, 0x4000},
    {20, 0x80, 0x2000},
    {28, 0x80, 0x4},
    {28, 0x40, 0x8},
    {29, 0x80, 0x100},
    {29, 0x40, 0x200},
    {29, 0x20, 0x400},
    {29, 0x10, 0x800},
    {29, 0x8, 0x1000},
  };
  s32 i;
  status->button = 0;
  for (i = 0; i < 13; ++i) {
    buttons = s_rxBuf1[chan * 32 + button_map[i][0]];
    if (buttons & button_map[i][1]) {
      status->button |= button_map[i][2];
    }
  }
  status->accX = (((s_rxBuf1[chan * 32 + 5] & 0x30) >> 4) | (s_rxBuf1[chan * 32 + 6] << 2)) - 511;
  status->accY = (((s_rxBuf1[chan * 32 + 5] >> 6) & 3) | (s_rxBuf1[chan * 32 + 7] << 2)) - 511;
  status->accZ = (((s_rxBuf1[chan * 32 + 13] >> 4) & 3) | (s_rxBuf1[chan * 32 + 14] << 2)) - 511;
  for (i = 0; i < 4; ++i) {
    status->obj[i].x = ((s_rxBuf1[chan * 32 + i * 8] & 3) << 8) | s_rxBuf1[chan * 32 + i * 8 + 1];
    status->obj[i].y = 767 - (((s_rxBuf1[chan * 32 + i * 8] << 6) & 0x300) | s_rxBuf1[chan * 32 + i * 8 + 2]);
    status->obj[i].size = s_rxBuf1[chan * 32 + i * 8 + 3] | ((s_rxBuf1[chan * 32 + i * 8 + 4] & 0x3f) << 8);
    status->obj[i].traceId = s_rxBuf1[chan * 32 + i * 8 + 5] & 7;
  }
  wpadFixDPDValue(status->obj);
  status->dev = wpadGetType(chan);
  status->fsAccX = (((s_rxBuf1[chan * 32 + 13] >> 6) & 3) | (s_rxBuf1[chan * 32 + 15] << 2)) - 511;
  status->fsAccY = (((s_rxBuf1[chan * 32 + 21] >> 4) & 3) | (s_rxBuf1[chan * 32 + 22] << 2)) - 511;
  status->fsAccZ = (((s_rxBuf1[chan * 32 + 21] >> 6) & 3) | (s_rxBuf1[chan * 32 + 23] << 2)) - 511;
  status->fsStickX = s_rxBuf1[chan * 32 + 30] - 127;
  status->fsStickY = s_rxBuf1[chan * 32 + 31] - 127;
  status->err = wpadErrSequence(chan);
  OSRestoreInterrupts(enabled);
}

static void wpadConvertDolphinFormat(s32 chan, PADStatus* status) {
  static BOOL s_enabled_flag[4];
  s32 offset = chan * 32;
  u32 button_map[12][3] = {
    {0, 0x1, 0x100},
    {0, 0x2, 0x200},
    {0, 0x4, 0x400},
    {0, 0x8, 0x800},
    {0, 0x10, 0x1000},
    {1, 0x1, 0x1},
    {1, 0x2, 0x2},
    {1, 0x4, 0x4},
    {1, 0x8, 0x8},
    {1, 0x10, 0x10},
    {1, 0x20, 0x20},
    {1, 0x40, 0x40},
  };
  s32 i;
  BOOL enabled;
  status->button = 0;
  enabled = OSDisableInterrupts();
  for (i = 0; i < 12; ++i) {
    if (s_rxBuf1[offset + button_map[i][0]] & button_map[i][1]) {
      status->button |= button_map[i][2];
    }
  }
  status->stickX = s_rxBuf1[offset + 2] - 128;
  status->stickY = s_rxBuf1[offset + 3] - 128;
  status->substickX = s_rxBuf1[offset + 4] - 128;
  status->substickY = s_rxBuf1[offset + 5] - 128;
  status->triggerLeft = s_rxBuf1[offset + 6];
  status->triggerRight = s_rxBuf1[offset + 7];
  status->err = wpadErrSequence(chan);
  OSRestoreInterrupts(enabled);
}

static void wpadFixDPDValue(DPDObject* obj) {
  s32 i;
  for (i = 0; i < 4; ++i, ++obj) {
    if (obj->size > 80) {
      obj->x = 0;
      obj->y = 767;
      obj->size = 0;
      obj->traceId = 0;
    }
  }
}

static s8 wpadCalibrateStick(s8 value, s8 origin) {
  s32 difference = value - origin;
  if (difference < -128) {
    return -128;
  } else if (difference > 127) {
    return 127;
  } else {
    return difference;
  }
}

static u8 wpadCalibrateTrigger(u8 value, u8 origin) {
  if (value < origin) {
    return 0;
  } else {
    return value - origin;
  }
}
