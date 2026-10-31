#ifndef _REVOLUTION_NDEVEXI2AD_ODEMU
#define _REVOLUTION_NDEVEXI2AD_ODEMU

#include <revolution/types.h>

static u32 ODEMUGenMailData(u32 ofs, u32 size) {
  return (ofs & 0xFF) << 16 | 0x1F000000 | size & 0x1FFF;
}

static u32 ODEMUGetPage(u32 mail) { return (mail & 0xFF0000) >> 16; }

static u32 ODEMUGetPc2NngcOffset(u32 mail) {
  if (!(ODEMUGetPage(mail) & 1)) {
    return 0;
  }
  return 0x800;
}

static BOOL ODEMUIsValidMail(u32 mail) { return (mail & 0x1F000000) == 0x1F000000; }

static u32 ODEMUGetSize(u32 mail) { return mail & 0x1FFF; }

#endif // _REVOLUTION_NDEVEXI2AD_ODEMU
