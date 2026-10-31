#include <dolphin/os.h>

#define EXI_MAGIC 0xa5ff005a

static BOOL Enabled = FALSE;
static BOOL BarnacleEnabled = FALSE;

static u32 Dev = 0;
static s32 Chan = 0;

static BOOL ProbeBarnacle(s32 chan, u32 dev, u32 *id) {
  BOOL error;
  u32 cmd;

  if (chan != 2 && dev == 0) {
    if (!EXIAttach(chan, NULL)) {
      return FALSE;
    }
  }

  error = !EXILock(chan, dev, NULL);
  if (error) {
    goto _detach;
  }

  error = !EXISelect(chan, dev, EXI_FREQ_1M);
  if (error) {
    goto _unlock;
  }

  cmd = 0x20011300;
  error = !EXIImm(chan, &cmd, sizeof(u32), EXI_WRITE, NULL);
  error |= !EXISync(chan);

  error |= !EXIImm(chan, id, sizeof(u32), EXI_READ, NULL);
  error |= !EXISync(chan);
  error |= !EXIDeselect(chan);

_unlock:
  EXIUnlock(chan);

_detach:
  if (chan != 2 && dev == 0) {
    EXIDetach(chan);
  }

  if (error) {
    return FALSE;
  }

  return *id != 0xFFFFFFFF;
}

void __OSEnableBarnacle(s32 chan, u32 dev) {
  u32 id;

  if (!EXIGetID(chan, dev, &id)) {
    return;
  }

  switch (id) {
  case 0x00000004:
  case 0x00000008:
  case 0x00000010:
  case 0x00000020:
  case 0x01010000:
  case 0x01020000:
  case 0x02020000:
  case 0x03010000:
  case 0x04020100:
  case 0x04020200:
  case 0x04020300:
  case 0x04040404:
  case 0x04060000:
  case 0x04120000:
  case 0x04130000:
  case 0x04220000:
  case 0x80000004:
  case 0x80000008:
  case 0x80000010:
  case 0x80000020:
  case 0xFFFFFFFF:
    break;
  default:
    if (ProbeBarnacle(chan, dev, &id)) {
      Chan = chan;
      Dev = dev;
      BarnacleEnabled = EXI_MAGIC;
      Enabled = EXI_MAGIC;
    }
    break;
  }
}
