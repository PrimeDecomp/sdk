#ifndef _DOLPHIN_VIFUNCS
#define _DOLPHIN_VIFUNCS
#ifndef PRIME1

#include "GameVersions.h"
#endif

#include <dolphin/types.h>

#include <dolphin/vitypes.h>

#ifdef __cplusplus
extern "C" {
#endif

u32 VIGetNextField(void);
VIRetraceCallback VISetPreRetraceCallback(VIRetraceCallback callback);
VIRetraceCallback VISetPostRetraceCallback(VIRetraceCallback callback);
#ifdef PRIME1
void __VIGetCurrentPosition(s16* x, s16* y);
#endif
u32 VIGetDTVStatus(void);
#ifndef PRIME1

#if VERSION >= VERSION_R32J01
BOOL VIEnableDimming(BOOL enable);
VITimeToDIM VISetTimeToDimming(VITimeToDIM time);
BOOL VIResetDimmingCount(void);
BOOL __VIResetRFIdle(void);
BOOL __VIResetDev0Idle(void);
BOOL __VIResetSIIdle(void);
BOOL VIEnableDVDStopMotor(BOOL enable);
u32 VIGetCurrentLine(void);
u32 VIGetScanMode(void);

void __VISetYUVSEL(VIBool outsel);
void __VISetTiming(VITimingMode timing);
void __VISetFilter4EURGB60(VIBool enable);
void __VISetVBICtrl(VIBool cgms, VIBool wss, VIBool cc);
void __VISetCGMS(void);
void __VISetWSS(void);
void VISetWSS(u8 gp1, u8 gp2, u8 gp3, u8 gp4);
void __VISetClosedCaption(void);
void VISetClosedCaption(u8 cc1, u8 cc2, u8 cc3, u8 cc4);
void __VISetMacrovisionImm(VIMacroVisionObj* mparam);
void __VISetMacrovision(void);
void __VISetGammaImm(VIGammaObj* gamma);
void __VISetGamma(void);
void __VISetTrapFilter(void);
void __VISetTrapFilterImm(VIBool filter);
void VISetTrapFilter(VIBool filter);
void __VISetRGBOverDrive(void);
void VISetRGBOverDrive(VIOverDrive level);
void VISetRGBModeImm(void);
void __VISetVolume(u8 volumeL, u8 volumeR);
void __VISetRGBModeImm(void);
void __VISetRevolutionModeSimple(void);

s32 __VISendI2CData(u8 slaveAddr, u8* pData, s32 nBytes);
void WaitMicroTime(s32 usec);
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif // _DOLPHIN_VIFUNCS
