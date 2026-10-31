#ifndef OSSTATETM_H
#define OSSTATETM_H

#include <dolphin/os.h>
#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*OSPowerCallback)(void);

OSPowerCallback OSSetPowerCallback(OSPowerCallback callback);
BOOL OSGetResetButtonState(void);
int __OSInitSTM(void);
void __OSShutdownToSBY(void);
void __OSHotReset(void);
int __OSSetVIForceDimming(BOOL isEnabled, u32 yShift, u32 xShift);
s32 __OSSetIdleLEDMode(u32);
s32 __OSUnRegisterStateEvent(void);

#ifdef __cplusplus
}
#endif

#endif // OSSTATETM_H