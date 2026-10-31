#ifndef _DOLPHIN_VITYPES
#define _DOLPHIN_VITYPES

#include "GameVersions.h"

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PRIME1
#define VI_INTERLACE 0
#define VI_NON_INTERLACE 1
#define VI_PROGRESSIVE 2
#define VI_3D 3
#else
typedef void (*VIRetraceCallback)(u32 retraceCount);
#endif

#ifdef PRIME1
#define VI_NTSC 0
#define VI_PAL 1
#define VI_MPAL 2
#define VI_DEBUG 3
#define VI_DEBUG_PAL 4
#define VI_EURGB60 5
#define VI_GCA 6

#define VI_TVMODE(format, interlace) (((format) << 2) + (interlace))

typedef enum {
  // NTSC
  VI_TVMODE_NTSC_INT = VI_TVMODE(VI_NTSC, VI_INTERLACE),
  VI_TVMODE_NTSC_DS = VI_TVMODE(VI_NTSC, VI_NON_INTERLACE),
  VI_TVMODE_NTSC_PROG = VI_TVMODE(VI_NTSC, VI_PROGRESSIVE),
  VI_TVMODE_NTSC_3D = VI_TVMODE(VI_NTSC, VI_3D),
  // PAL
  VI_TVMODE_PAL_INT = VI_TVMODE(VI_PAL, VI_INTERLACE),
  VI_TVMODE_PAL_DS = VI_TVMODE(VI_PAL, VI_NON_INTERLACE),

  // MPAL
  VI_TVMODE_MPAL_INT = VI_TVMODE(VI_MPAL, VI_INTERLACE),
  VI_TVMODE_MPAL_DS = VI_TVMODE(VI_MPAL, VI_NON_INTERLACE),

  // Debug
  VI_TVMODE_DEBUG_INT = VI_TVMODE(VI_DEBUG, VI_INTERLACE),

  // Debug PAL
  VI_TVMODE_DEBUG_PAL_INT = VI_TVMODE(VI_DEBUG_PAL, VI_INTERLACE),
  VI_TVMODE_DEBUG_PAL_DS = VI_TVMODE(VI_DEBUG_PAL, VI_NON_INTERLACE),

  // EU RGB60
  VI_TVMODE_EURGB60_INT = VI_TVMODE(VI_EURGB60, VI_INTERLACE),
  VI_TVMODE_EURGB60_DS = VI_TVMODE(VI_EURGB60, VI_NON_INTERLACE),

  // GCA
  VI_TVMODE_GCA_INT = VI_TVMODE(VI_GCA, VI_INTERLACE),
  VI_TVMODE_GCA_DS = VI_TVMODE(VI_GCA, VI_NON_INTERLACE),
  VI_TVMODE_GCA_PROG = VI_TVMODE(VI_GCA, VI_PROGRESSIVE),
} VITVMode;

typedef enum {
  VI_XFBMODE_SF = 0,
  VI_XFBMODE_DF = 1,
} VIXFBMode;

typedef void (*VIPositionCallback)(s16 x, s16 y);
typedef void (*VIRetraceCallback)(u32 retraceCount);

// Structure to use with timing in vi.c (size 0x28).
typedef struct VITimingInfo {
  u8 equ;           // _00
  u16 acv;          // _02
  u16 prbOdd;       // _04
  u16 prbEven;      // _06
  u16 psbOdd;       // _08
  u16 psbEven;      // _0A
  u8 bs1;           // _0C
  u8 bs2;           // _0D
  u8 bs3;           // _0E
  u8 bs4;           // _0F
  u16 be1;          // _10
  u16 be2;          // _12
  u16 be3;          // _14
  u16 be4;          // _16
  u16 numHalfLines; // _18
  u16 hlw;          // _1A
  u8 hsy;           // _1C
  u8 hcs;           // _1D
  u8 hce;           // _1E
  u8 hbe640;        // _1F
  u16 hbs640;       // _20
  u8 hbeCCIR656;    // _24
  u16 hbsCCIR656;   // _26
} VITimingInfo;

// Structure to use with HorVer in vi.c (size 0x58).
typedef struct VIPositionInfo {
  u16 dispPosX;         // _00
  u16 dispPosY;         // _02
  u16 dispSizeX;        // _04
  u16 dispSizeY;        // _06
  u16 adjDispPosX;      // _08
  u16 adjDispPosY;      // _0A
  u16 adjDispSizeY;     // _0C
  u16 adjPanPosY;       // _0E
  u16 adjPanSizeY;      // _10
  u16 fbSizeX;          // _12
  u16 fbSizeY;          // _14
  u16 panPosX;          // _16
  u16 panPosY;          // _18
  u16 panSizeX;         // _1A
  u16 panSizeY;         // _1C
  VIXFBMode xfbMode;    // _20
  u32 nonInter;         // _24
  u32 tv;               // _28
  u8 wordPerLine;       // _2C
  u8 std;               // _2D
  u8 wpl;               // _2E
  u32 bufAddr;          // _30
  u32 tfbb;             // _34
  u32 bfbb;             // _38
  u8 xof;               // _3C
  BOOL isBlack;         // _40
  BOOL is3D;            // _44
  u32 rbufAddr;         // _48
  u32 rtfbb;            // _4C
  u32 rbfbb;            // _50
  VITimingInfo* timing; // _54
} VIPositionInfo;
#else
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
#endif

#ifdef __cplusplus
}
#endif

#endif // _DOLPHIN_VITYPES
