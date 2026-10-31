#ifndef _REVOLUTION_FLIPPER
#define _REVOLUTION_FLIPPER

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// The IPC mailbox registers and the access control registers share one block.
#ifdef __MWERKS__
vu32 __IPCRegs[4] : 0xCD000000;
vu32 __ACRRegs[89] : 0xCD000000;
#else
#define __IPCRegs ((vu32*)0xCD000000)
#define __ACRRegs ((vu32*)0xCD000000)
#endif

static inline void ACRWriteReg(u32 offset, u32 val) { __ACRRegs[offset >> 2] = val; }

static inline u32 ACRReadReg(u32 offset) { return __ACRRegs[offset >> 2]; }

#ifdef __cplusplus
}
#endif

#endif // _REVOLUTION_FLIPPER
