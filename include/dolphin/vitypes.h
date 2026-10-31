#ifndef _DOLPHIN_VITYPES
#define _DOLPHIN_VITYPES

#include "GameVersions.h"

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*VIRetraceCallback)(u32 retraceCount);

#if VERSION >= VERSION_R32J01
#define VI_TVMODE_PAL_PROG ((VITVMode)6)

typedef u8 VIBool;
#define VI_FALSE ((VIBool)0)
#define VI_TRUE ((VIBool)1)
#define VI_DISABLE ((VIBool)0)
#define VI_ENABLE ((VIBool)1)

typedef enum { VI_DM_DEFAULT = 0, VI_DM_10M, VI_DM_15M } VITimeToDIM;

typedef enum { VI_TMG_GAME = 0, VI_TMG_DVD = 1 } VITimingMode;

typedef enum { VI_VMODE_NTSC = 0, VI_VMODE_MPAL = 1, VI_VMODE_PAL = 2, VI_VMODE_RGB = 3 } VIVideo;

typedef enum {
  VI_GM_0_1 = 1,
  VI_GM_0_2,
  VI_GM_0_3,
  VI_GM_0_4,
  VI_GM_0_5,
  VI_GM_0_6,
  VI_GM_0_7,
  VI_GM_0_8,
  VI_GM_0_9,
  VI_GM_1_0,
  VI_GM_1_1,
  VI_GM_1_2,
  VI_GM_1_3,
  VI_GM_1_4,
  VI_GM_1_5,
  VI_GM_1_6,
  VI_GM_1_7,
  VI_GM_1_8,
  VI_GM_1_9,
  VI_GM_2_0,
  VI_GM_2_1,
  VI_GM_2_2,
  VI_GM_2_3,
  VI_GM_2_4,
  VI_GM_2_5,
  VI_GM_2_6,
  VI_GM_2_7,
  VI_GM_2_8,
  VI_GM_2_9,
  VI_GM_3_0
} VIGamma;

typedef struct VIGammaObj {
  u16 a[6];
  u8 yin[7];
  u16 yout[7];
} VIGammaObj;

typedef enum { VI_ACP_OFF = 1, VI_ACP_TYPE1 = 2, VI_ACP_TYPE2 = 3, VI_ACP_TYPE3 = 4 } VIACPType;

typedef enum {
  VI_ODV_L1 = 0,
  VI_ODV_L2 = 1,
  VI_ODV_L3 = 2,
  VI_ODV_L4 = 3,
  VI_ODV_L5 = 4,
  VI_ODV_L6 = 5
} VIOverDrive;

typedef struct VIMacroVisionObj {
  u8 m[26];
} VIMacroVisionObj;
#endif

#ifdef __cplusplus
}
#endif

#endif // _DOLPHIN_VITYPES
