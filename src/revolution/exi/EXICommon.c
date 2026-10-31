#include <revolution/exi.h>

#define EXI_FREQ_16M 4

static u32 __EXISwap32(u32 val) {
  return val >> 24 & 0x000000FF | val >> 8 & 0x0000FF00 | val << 8 & 0x00FF0000 |
         val << 24 & 0xFF000000;
}

const u32 __EXIFreq = EXI_FREQ_16M;

BOOL EXIWriteReg(s32 chan, u32 dev, u32 cmd, void *buf, s32 len) {
  BOOL error = FALSE;
  u32 write_val;

  switch (len) {
  case 1:
    write_val = *(u8 *)buf << 24;
    break;
  case 2:
    write_val = *(u16 *)buf << 24 | (*(u16 *)buf & 0xFF00) << 8;
    break;
  default:
    write_val = __EXISwap32(*(u32 *)buf);
    break;
  }

  error |= !EXILock(chan, dev, NULL);
  if (error) {
    return FALSE;
  }

  error |= !EXISelect(chan, dev, EXI_FREQ_16M);
  if (error) {
    EXIUnlock(chan);
    return FALSE;
  }

  error |= !EXIImm(chan, &cmd, sizeof(cmd), 1, NULL);
  error |= !EXISync(chan);
  error |= !EXIImm(chan, &write_val, sizeof(write_val), 1, NULL);
  error |= !EXISync(chan);
  error |= !EXIDeselect(chan);
  error |= !EXIUnlock(chan);

  return error == FALSE;
}
