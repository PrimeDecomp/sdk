#ifndef PF_SERVICE_H
#define PF_SERVICE_H

#include <revolution/types.h>

#define SWAP16(x) ((((x) >> 8) & 0xFF) | (((x) << 8) & 0xFF00))
#define SWAP32(x) (((x) >> 24) | (((x) >> 8) & 0xFF00) | (((x) << 8) & 0xFF0000) | ((x) << 24))

u16 VFiPF_GET_LE_U16(const u8* buf);
void VFiPF_LE16_TO_U16_STR(u8* sSrc, u32 num);

#endif // PF_SERVICE_H
